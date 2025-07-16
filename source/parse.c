// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <ctype.h>
#include <string.h>
#include <setjmp.h>
#include <stdint.h>
#include "shared.h"
#include "parse.h"
#include "vars.h"
#include "monitor.h"

// Token constants
constexpr int LXEOI = 0;     // end of input
constexpr int LXINT = 1;     // integer
constexpr int LXPOLY = 2;    // polynomial
constexpr int LXLT = 3;      // <
constexpr int LXLE = 4;      // <=
constexpr int LXGT = 5;      // >
constexpr int LXGE = 6;      // >=
constexpr int LXEQ = 7;      // ==
constexpr int LXNE = 8;      // !=
constexpr int LXOR = 9;      // ||
constexpr int LXAND = 10;    // &&
constexpr int LXPLUS = 11;   // +
constexpr int LXMINUS = 12;  // -
constexpr int LXMULT = 13;   // *
constexpr int LXDIV = 14;    // /
constexpr int LXMOD = 15;    // % (& in original)
constexpr int LXEXP = 16;    // ^ or **
constexpr int LXLP = 17;     // (
constexpr int LXRP = 18;     // )
constexpr int LXUMINUS = 19; // unary -
constexpr int LXUPLUS = 20;  // unary +
constexpr int LXNOT = 21;    // !

// Stack constants
constexpr int NSTACK = 50;
constexpr int NARGS = 2;

// Stack structure
typedef struct stack_struct
{
    int tos;
    void *vals[NSTACK];
} stack;

// these mean: $int pol  < <= > >= = !=  || &&  + - * / & ^  ( ) u- u+ NOT
int fprec[] = {0, 11, 11, 4, 4, 4, 4, 4, 4, 3, 3, 6, 6, 8, 8, 8, 8, 1, 13, 8, 8, 8};
int gprec[] = {0, 10, 10, 3, 3, 3, 3, 3, 3, 2, 2, 5, 5, 7, 7, 7, 9, 11, 1, 10, 10, 10};

int nargs[] = {0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 1, 0, 1};

// variables used in parsing

static int thistok;    // one of LXEOI, ..., LXUMINUS
static int thisval;    // if thistok = LXINT, this is the integer that it is
static int parenlevel; // should never be negative.  End value should be 0

static stack opStack;  // operator stack
static stack valStack; // value stack

char *beginStr;    // begin of input string: used to display errors
jmp_buf jmpparse;  // used to leave parsing once error is found
int amParsing = 0; // =1 if beginStr, jmpparse are active

// stack routines

boolean emptyStack(stack *st)
{
    return (st->tos == -1);
}

void initStack(stack *st)
{
    st->tos = -1;
}

void *pop(stack *st)
{
    register void *val;

    val = st->vals[st->tos];
    st->tos--;
    return val;
}

void push(stack *st, void *val)
{
    st->tos++;
    st->vals[st->tos] = val;
}

void *tos(stack *st)
{
    return st->vals[st->tos];
}

// getNext -- get next token from input string "parseStr"

static boolean isunary(int tok)
{
    return ((tok != LXRP) && (tok != LXINT) && (tok != LXPOLY));
}

int collectInt(char c, char **str)
{
    int n;

    n = c - '0';
    while (isdigit(c = *(*str)++))
        n = 10 * n + c - '0';
    (*str)--;
    return n;
}

static int get2(char **str, char ch, int tok1, int tok2)
{
    char c;

    c = *(*str)++;
    if (c == ch)
        return tok1;
    else
    {
        (*str)--;
        return tok2;
    }
}

static boolean isBinOp(int tok)
{
    return ((tok >= LXPLUS) && (tok <= LXEXP));
}

static boolean isOperator(int tok)
{
    return (isBinOp(tok) || (tok == LXUMINUS));
}

void parseErr(char *end, const char *mess)
{
    char c;

    c = *end;
    *end = '\0';
    prerror("; %s at: %s\n", mess, beginStr);
    *end = c;
    longjmp(jmpparse, -1);
}

static boolean checkError(int lasttok, int tok, char *end)
{
    if (((lasttok == LXINT) || (lasttok == LXRP)) && ((tok == LXINT) || (tok == LXLP)))
    {
        parseErr(end, "missing operator");
        return TRUE;
    }
    if (((isOperator(lasttok)) || (lasttok == LXLP)) && ((isBinOp(tok)) || (tok == LXRP)))
    {
        parseErr(end, "missing operand");
        return TRUE;
    }
    return FALSE;
}

static void getNext(char **str)
{
    char c;
    int lasttok;
    char ident[100];

    lasttok = thistok;
    c = *(*str)++;

    if (c == '*')
    {
        if (**str == '*')
        {
            (*str)++;
            thistok = LXEXP;
        }
        else
            thistok = LXMULT;
    }
    else if (c == '/')
        thistok = LXDIV;
    else if (c == '|')
        thistok = get2(str, '|', LXOR, LXEOI);
    else if (c == '&')
        thistok = get2(str, '&', LXAND, LXMOD);
    else if (c == '^')
        thistok = LXEXP;
    else if (c == '(')
    {
        parenlevel++;
        thistok = LXLP;
    }
    else if (c == ')')
    {
        if (parenlevel == 0)
        {
            thistok = LXEOI;
            (*str)--;
        }
        else
        {
            parenlevel--;
            thistok = LXRP;
        }
    }
    else if (c == '+')
    {
        if (isunary(thistok))
            thistok = LXUPLUS;
        else
            thistok = LXPLUS;
    }
    else if (c == '-')
    {
        if (isunary(thistok))
            thistok = LXUMINUS;
        else
            thistok = LXMINUS;
    }
    else if (c == '>')
        thistok = get2(str, '=', LXGE, LXGT);
    else if (c == '<')
        thistok = get2(str, '=', LXLE, LXLT);
    else if (c == '=')
        thistok = get2(str, '=', LXEQ, LXEQ);
    else if (c == '!')
        thistok = get2(str, '=', LXNE, LXNOT);
    else if (isdigit(c))
    {
        thisval = collectInt(c, str);
        thistok = LXINT;
    }
    else if (canStartVar(c))
    {
        (*str)--;
        getIdentifier(str, ident);
        thistok = LXINT;
        thisval = getIdVal(ident);
    }
    else
    {
        (*str)--;
        thistok = LXEOI;
    }
    checkError(lasttok, thistok, *str);
}

// doAction -- take action depending on operator

static void doAction(int op, char *parseStr)
{
    int i, n, val;
    int args[NARGS];

    n = nargs[op];
    for (i = 0; i < n; i++)
        if (emptyStack(&valStack))
        {
            parseErr(parseStr, "too few operands");
            args[i] = 0;
        }
        else
            args[i] = (int)(intptr_t)pop(&valStack);

    switch (op)
    {
    case LXEOI:
    case LXLP:
    case LXRP:
    case LXUPLUS:
        break;
    case LXEQ:
        push(&valStack, (void *)(intptr_t)(args[1] == args[0]));
        break;
    case LXOR:
        push(&valStack, (void *)(intptr_t)(args[1] || args[0]));
        break;
    case LXAND:
        push(&valStack, (void *)(intptr_t)(args[1] && args[0]));
        break;
    case LXNE:
        push(&valStack, (void *)(intptr_t)(args[1] != args[0]));
        break;
    case LXGT:
        push(&valStack, (void *)(intptr_t)(args[1] > args[0]));
        break;
    case LXGE:
        push(&valStack, (void *)(intptr_t)(args[1] >= args[0]));
        break;
    case LXLT:
        push(&valStack, (void *)(intptr_t)(args[1] < args[0]));
        break;
    case LXLE:
        push(&valStack, (void *)(intptr_t)(args[1] <= args[0]));
        break;
    case LXNOT:
        push(&valStack, (void *)(intptr_t)(!args[0]));
        break;
    case LXPLUS:
        push(&valStack, (void *)(intptr_t)(args[1] + args[0]));
        break;
    case LXMINUS:
        push(&valStack, (void *)(intptr_t)(args[1] - args[0]));
        break;
    case LXMULT:
        push(&valStack, (void *)(intptr_t)(args[1] * args[0]));
        break;
    case LXDIV:
        if (args[0] == 0)
        {
            prerror("; attempt to divide by zero\n");
            push(&valStack, (void *)(intptr_t)0);
        }
        else
            push(&valStack, (void *)(intptr_t)(args[1] / args[0]));
        break;
    case LXMOD:
        if (args[0] == 0)
        {
            prerror("; attempt to divide by zero\n");
            push(&valStack, (void *)(intptr_t)args[1]);
        }
        else
            push(&valStack, (void *)(intptr_t)(args[1] % args[0]));
        break;
    case LXEXP:
        val = 1;
        for (i = 1; i <= args[0]; i++)
            val *= args[1];
        push(&valStack, (void *)(intptr_t)val);
        break;
    case LXUMINUS:
        push(&valStack, (void *)(intptr_t)(-args[0]));
        break;
    default:
        prerror(";internal error: shouldn't get here\n");
    }
}

// eatInt -- use operator precedence parsing to compute integer value

// "str" is modified to point directly after integer computed.

int eatInt(char **str)
{
    int a, lastop;

    parenlevel = 0;
    thistok = LXEOI;
    initStack(&opStack);
    push(&opStack, (void *)(intptr_t)LXEOI);
    initStack(&valStack);

    beginStr = *str; // used for error messages
    getNext(str);
    while (TRUE)
    {
        a = (int)(intptr_t)tos(&opStack);
        if ((thistok == LXEOI) && (a == LXEOI))
        {
            if (emptyStack(&valStack))
            {
                prerror("; missing operand\n");
                return 0;
            }
            else
                return (int)(intptr_t)pop(&valStack);
        }
        if (thistok == LXINT)
        {
            push(&valStack, (void *)(intptr_t)thisval);
            getNext(str);
        }
        else if (fprec[a] <= gprec[thistok])
        {
            push(&opStack, (void *)(intptr_t)thistok);
            getNext(str);
        }
        else
        {
            do
            {
                lastop = (int)(intptr_t)pop(&opStack);
                doAction(lastop, *str);
            } while (fprec[(int)(intptr_t)tos(&opStack)] >= gprec[lastop]);
        }
    }
}

int parseInt(char **str)
{
    int n;

    if (amParsing == 0)
    {
        amParsing = 1;
        beginStr = *str;
        if (setjmp(jmpparse))
        {
            amParsing = 0;
            return 0;
        }
        n = eatInt(str);
        amParsing = 0;
        return n;
    }
    n = eatInt(str);
    return n;
}

int getInt(const char *s)
{
    // Make a local copy to avoid cast-qual warning
    char buf[256];
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    char *str = buf;
    return parseInt(&str);
}

int readInt(char **str)
{
    int n;

    n = parseInt(str);
    if (**str != '\0')
        prerror("; premature end of expression\n");
    return n;
}
