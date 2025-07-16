// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <ctype.h>
#include <setjmp.h>
#include <stdint.h>
#include "shared.h"
#include "parsepoly.h"
#include "parse.h"    // fprec, gprec, nargs, jmpparse, beginStr, amParsing, token constants
#include "poly.h"     // polynomial operations
#include "monitor.h"  // prerror
#include "rvar.h"     // validVar, parseVar
#include "vars.h"     // canStartVar, getIdentifier, getIdVal, getPolyId
#include "gm_poly.h"  // p_div, p_mod
#include "human_io.h" // get_contstr

// SINGLE SOURCE OF TRUTH VIOLATION - CRITICAL BUG:
// 1. struct stack_struct is multiply defined:
// - Here with 'top' member and NSTACK=100
// - In parse.c with 'tos' member and NSTACK=50
// 2. These incompatible definitions violate single source of truth
// 3. PROPER FIX: Move to parse.h or shared.h with consistent definition
// This is a project-wide issue that cannot be fixed by modifying parsepoly.c alone
constexpr int NSTACK = 100;
struct stack_struct
{
    int top;
    void *vals[NSTACK];
};

// Constants
constexpr int NARGS = 10;

// SINGLE SOURCE OF TRUTH VIOLATION:
// Token constants are defined in parse.c but not exported in parse.h
// These definitions duplicate those in parse.c
// PROPER FIX: Move these constants to parse.h
// This is a project-wide issue that cannot be fixed by modifying parsepoly.c alone
#ifndef LXEOI
constexpr int LXEOI = 0;
constexpr int LXINT = 1;
constexpr int LXPOLY = 2;
constexpr int LXLP = 3;
constexpr int LXRP = 4;
constexpr int LXLB = 5;
constexpr int LXRB = 6;
constexpr int LXPLUS = 7;
constexpr int LXMINUS = 8;
constexpr int LXMULT = 9;
constexpr int LXDIV = 10;
constexpr int LXMOD = 11;
constexpr int LXEXP = 12;
constexpr int LXEQUAL = 13;
constexpr int LXSEMI = 14;
constexpr int LXCOMMA = 15;
constexpr int LXUPLUS = 16;
constexpr int LXUMINUS = 17;
#endif

// Macros for token classification
static inline bool isunary(int tok)
{
    return (tok == LXEOI) || (tok == LXLP) || (tok == LXLB) || (tok >= LXPLUS && tok <= LXEXP) ||
           (tok == LXEQUAL) || (tok == LXSEMI) || (tok == LXCOMMA);
}

static inline bool isOperator(int tok)
{
    return (tok >= LXPLUS && tok <= LXUMINUS);
}

static inline bool isBinOp(int tok)
{
    return (tok >= LXPLUS && tok <= LXEXP);
}

// Note: External globals fprec, gprec, nargs, jmpparse, beginStr, amParsing
// should come from parse.h when included properly

// variables used in parsing

// Only polynomials are on the value stack.  This was done for
// "simplicity" (in any case, to do this all correct needs a total
// rewrite).  The only place an integer is needed is in exponentiation
// Since these are never stacked, the integer value is placed in a
// single variable: "ithisval".

int pthistok;  // one of LXEOI, ..., LXUMINUS
poly pthisval; // if pthistok = LXPOLY, this is the poly that it is
int ithisval;  // integer value, if pthistok = LXINT: ONLY after '^'

int pComponent; // row number of resulting polynomial

stack pOpStack;  // operator stack
stack pValStack; // value stack

int pParenlevel; // should never be negative.  End value should be 0

// pGetNext(str)
// char **str ;
// {
// p1GetNext(str) ;
// print("token is %d\n", pthistok) ;
// }

void pGetNext(char **str)
{
    int lasttok, varnum;
    char c;
    char ident[100];

    lasttok = pthistok;
    c = *(*str)++;

    // insert '*' between terminals
    if (((c == '(') || (c == '{') || (validVar(c))) &&
        ((lasttok == LXINT) || (lasttok == LXRP) || (lasttok == LXPOLY)))
    {

        (*str)--;
        pthistok = LXMULT;
        return;
    }

    // insert '^' if "looks like" exponential
    if ((isdigit(c)) && ((lasttok == LXRP) || (lasttok == LXPOLY)))
    {

        (*str)--;
        pthistok = LXEXP;
        return;
    }

    // if the last token was '^', then we now expect an integer: get it

    if (lasttok == LXEXP)
    {
        if (c == '(')
        {
            ithisval = eatInt(str);
            if (**str != ')')
                parseErr(*str, "missing right parenthesis");
            (*str)++;
        }
        else if (canStartVar(c))
        {
            (*str)--;
            getIdentifier(str, ident);
            ithisval = getIdVal(ident);
        }
        else if (isdigit(c))
        {
            ithisval = collectInt(c, str);
        }
        else
            parseErr(*str, "bad exponent");
        pthistok = LXINT;
        return;
    }

    if (c == '*')
    {
        if (**str == '*')
        {
            (*str)++;
            pthistok = LXEXP;
        }
        else
            pthistok = LXMULT;
    }
    else if (c == '/')
        pthistok = LXDIV;
    else if (c == '&')
        pthistok = LXMOD;
    else if (c == '^')
        pthistok = LXEXP;
    else if (c == '(')
    {
        pParenlevel++;
        pthistok = LXLP;
    }
    else if (c == ')')
    {
        if (pParenlevel == 0)
        {
            pthistok = LXEOI;
            (*str)--;
        }
        else
        {
            pParenlevel--;
            pthistok = LXRP;
        }
    }
    else if (c == '+')
    {
        if (isunary(pthistok))
            pthistok = LXUPLUS;
        else
            pthistok = LXPLUS;
    }
    else if (c == '-')
    {
        if (isunary(pthistok))
            pthistok = LXUMINUS;
        else
            pthistok = LXMINUS;
    }
    else if (validVar(c))
    {
        (*str)--;
        varnum = parseVar(str);
        if (varnum == -1)
        {
            prerror("; ring variable not defined\n");
            pthisval = nullptr;
        }
        else
        {
            pthisval = p_xjei(varnum, pComponent);
        }
        pthistok = LXPOLY;
    }
    else if (isdigit(c))
    {
        pthisval = p_intpoly(collectInt(c, str), pComponent);
        pthistok = LXPOLY;
    }
    else if (canStartVar(c))
    { // notice this is a limited set
        (*str)--;
        getIdentifier(str, ident);
        pthisval = getPolyId(ident, pComponent);
        pthistok = LXPOLY;
    }
    else
    {
        (*str)--;
        pthistok = LXEOI;
    }
    pCheckError(lasttok, pthistok, *str);
}

int pCheckError(int lasttok, int thistok, char *end)
{
    if (((isOperator(lasttok)) || (lasttok == LXLP)) && ((isBinOp(thistok)) || (thistok == LXRP)))
    {

        parseErr(end, "missing operand");
        return (TRUE);
    }
    return (FALSE);
}

void pDoAction(int op, char *parseStr)
{
    poly f, g;
    int i, n;
    poly args[NARGS];

    if (op == LXEXP)
        n = 1; // exponential kludge: second argument is in "ithisval"
    else
        n = nargs[op];
    for (i = 0; i < n; i++)
    {
        if (emptyStack(&pValStack))
        {
            parseErr(parseStr, "too few operands");
            args[i] = NULL;
        }
        else
        {
            args[i] = (poly)pop(&pValStack);
        }
    }

    switch (op)
    {
    case LXPLUS:
        p_add(args, args + 1);
        push(&pValStack, args[0]);
        break;
    case LXMINUS:
        p_sub(args + 1, args);
        push(&pValStack, args[1]);
        break;
    case LXMULT:
        f = p_mult(args[0], args[1]);
        push(&pValStack, f);
        p_kill(args);
        p_kill(args + 1);
        break;
    case LXDIV:
        f = p_div(&args[1], args[0]);
        push(&pValStack, f);
        p_kill(args);
        break;
    case LXMOD:
        f = p_mod(&args[1], args[0]);
        push(&pValStack, f);
        p_kill(args);
        break;
    case LXEXP:
        n = ithisval;
        f = e_sub_i(pComponent);
        for (i = 1; i <= n; i++)
        {
            g = p_mult(f, args[0]);
            p_kill(&f);
            f = g;
        }
        p_kill(args);
        push(&pValStack, f);
        break;
    case LXUMINUS:
        p_negate(args);
        push(&pValStack, args[0]);
        break;
    case LXEOI:
    case LXLP:
    case LXRP:
    case LXUPLUS:
        break;
    default:
        prerror(";internal error: shouldn't get here\n");
    }
}

poly eatPoly(char **str, int comp)
{
    int a, lastop;

    pComponent = comp; // make this global

    pParenlevel = 0;
    pthistok = LXEOI;
    initStack(&pOpStack);
    push(&pOpStack, (void *)(intptr_t)LXEOI);
    initStack(&pValStack);

    pGetNext(str);
    while (TRUE)
    {
        a = (int)(intptr_t)tos(&pOpStack);
        if ((pthistok == LXEOI) && (a == LXEOI))
        {
            if (emptyStack(&pValStack))
            {
                prerror("; missing operand\n");
                return (NULL);
            }
            else
                return ((poly)pop(&pValStack));
        }
        if (pthistok == LXPOLY)
        {
            push(&pValStack, pthisval);
            pGetNext(str);
        }
        else if (pthistok == LXINT)
            pGetNext(str);
        else if (fprec[a] <= gprec[pthistok])
        {
            push(&pOpStack, (void *)(intptr_t)pthistok);
            pGetNext(str);
        }
        else
        {
            do
            {
                lastop = (int)(intptr_t)pop(&pOpStack);
                pDoAction(lastop, *str);
            } while (fprec[(int)(intptr_t)tos(&pOpStack)] >= gprec[lastop]);
        }
    }
}

poly parsePoly(char **str, int comp)
{
    poly f;

    amParsing = 1;
    if (setjmp(jmpparse))
    {
        amParsing = 0;
        return (NULL);
    }
    beginStr = *str;
    f = eatPoly(str, comp);
    if (**str != '\0')
        prerror("; premature end of expression\n");
    amParsing = 0;
    return (f);
}

poly readPoly(char **str, int comp)
{
    poly f;

    f = parsePoly(str, comp);
    if (**str != '\0')
        prerror("; premature end of expression\n");
    return (f);
}

poly getPoly(char *s, int comp)
{
    return (parsePoly(&s, comp));
}

poly rdPoly(int whichComp)
{
    poly f, g;
    char *str;

    f = NULL;
    while (get_contstr("", &str))
    {
        g = getPoly(str, whichComp);
        p_add(&f, &g);
    }
    g = getPoly(str, whichComp);
    p_add(&f, &g);
    return (f);
}

poly rdPolyStr(int argc, char **argv, int whichComp)
{
    poly f, g;
    char *str;

    f = getPoly(argv[0], whichComp);
    if (argc == 1)
        return (f); // no continuation
    while (get_contstr("", &str))
    {
        g = getPoly(str, whichComp);
        p_add(&f, &g);
    }
    g = getPoly(str, whichComp);
    p_add(&f, &g);
    return (f);
}
