/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "style.h"
#include "parse.h"

// boolean emptyStack (stack *st);
// void initStack (stack *st);
// int pop (stack *st);
// void push (stack *st, int val);
// int tos (stack *st);
// boolean isunary (int thistok);
// int collectInt (char c, char **str);
// int get2 (char **str, char ch, int tok1, int tok2);
// void getNext (char **str);
// boolean isBinOp (int tok);
// boolean isOperator (int tok);
// boolean checkError (int lasttok, int thistok, char *end);
void parseErr (char *end, char *mess);
// void doAction (int op, char *parseStr);
// int eatInt (char **str);
// int parseInt (char **str);
// int getInt (char *s);
// int readInt (char **str);

/* these mean: $int pol  < <= > >= = !=  || &&  + - * / & ^  ( ) u- u+ NOT */
int fprec[] = {0, 11,11, 4,4,4,4,4,4, 3,3, 6,6,8,8,8,8,  1, 13,  8,  8,  8};
int gprec[] = {0, 10,10, 3,3,3,3,3,3, 2,2, 5,5,7,7,7,9, 11,  1, 10, 10, 10};

int nargs[] = {0, 1,1,   2,2,2,2,2,2, 2,2, 2,2,2,2,2,2,  0,  0,  1,  0,  1};

/* variables used in parsing */

int thistok;   /* one of LXEOI, ..., LXUMINUS */
int thisval;   /* if thistok = LXINT, this is the integer that it is */
int parenlevel; /* should never be negative.  End value should be 0 */

stack opStack; /* operator stack */
stack valStack; /* value stack */

char *beginStr; /* begin of input string: used to display errors */
jmp_buf jmpparse; /* used to leave parsing once error is found */
int amParsing = 0; /* =1 if beginStr, jmpparse are active */

/*---------------------------------------------------------------
 *
 *  stack routines
 *
 *---------------------------------------------------------------*/

boolean emptyStack (stack *st)
{
    return(st->tos IS -1);
}

void initStack (stack *st)
{
    st->tos = -1;
}

int pop (stack *st)
{
    int val;

    val = st->vals[st->tos];
    st->tos--;
    return(val);
}

void push (stack *st, int val)
{
    st->tos++;
    st->vals[st->tos] = val;
}

int tos (stack *st)
{
    return(st->vals[st->tos]);
}

/*---------------------------------------------------------------
 *
 *  getNext -- get next token from input string "parseStr"
 *
 *---------------------------------------------------------------*/

boolean isunary (int thistok)
{
    return((thistok ISNT LXRP) AND (thistok ISNT LXINT)
           AND (thistok ISNT LXPOLY));
}

int collectInt (char c, char **str)
{
    int n;

    n = c - '0';
    while (isdigit(c = *(*str)++ ))
        n = 10*n + c - '0';
    (*str)--;
    return(n);
}

int get2 (char **str, char ch, int tok1, int tok2)
{
    char c;

    c = *(*str)++;
    if (c IS ch)
        return(tok1);
    else {
        (*str)--;
        return(tok2);
    }
}

void getNext (char **str)
{
    char c;
    int lasttok;
    char ident[100];

    lasttok = thistok;
    c = *(*str)++;

    if (c IS '*') {
        if (**str IS '*') {
            (*str)++;
            thistok = LXEXP;
        } else
            thistok = LXMULT;
    } else if (c IS '/')
        thistok = LXDIV;
    else if (c IS '|')
        thistok = get2(str, '|', LXOR, LXEOI);
    else if (c IS '&')
        thistok = get2(str, '&', LXAND, LXMOD);
    else if (c IS '^')
        thistok = LXEXP;
    else if (c IS '(') {
        parenlevel++;
        thistok = LXLP;
    } else if (c IS ')') {
        if (parenlevel IS 0) {
            thistok = LXEOI;
            (*str)--;
        } else {
            parenlevel--;
            thistok = LXRP;
        }
    } else if (c IS '+') {
        if (isunary(thistok))
            thistok = LXUPLUS;
        else
            thistok = LXPLUS;
    } else if (c IS '-') {
        if (isunary(thistok))
            thistok = LXUMINUS;
        else
            thistok = LXMINUS;
    } else if (c IS '>')
        thistok = get2(str, '=', LXGE, LXGT);
    else if (c IS '<')
        thistok = get2(str, '=', LXLE, LXLT);
    else if (c IS '=')
        thistok = get2(str, '=', LXEQ, LXEQ);
    else if (c IS '!')
        thistok = get2(str, '=', LXNE, LXNOT);
    else if (isdigit(c)) {
        thisval = collectInt(c, str);
        thistok = LXINT;
    } else if (canStartVar(c)) {
        (*str)--;
        getIdentifier(str, ident);
        thistok = LXINT;
        thisval = getIdVal(ident);
    } else {
        (*str)--;
        thistok = LXEOI;
    }
    checkError(lasttok, thistok, *str);
}

/*---------------------------------------------------------------
 *
 *  parseErr -- display error message
 *
 *---------------------------------------------------------------*/

boolean isBinOp (int tok)
{
    return((tok >= LXPLUS) AND (tok <= LXEXP));
}

boolean isOperator (int tok)
{
    return(isBinOp(tok) OR (tok IS LXUMINUS));
}

boolean checkError (int lasttok, int thistok, char *end)
{
    if (((lasttok IS LXINT) OR (lasttok IS LXRP))
            AND ((thistok IS LXINT) OR (thistok IS LXLP))) {
        parseErr(end, "missing operator");
        return(TRUE);
    }
    if (((isOperator(lasttok)) OR (lasttok IS LXLP))
            AND ((isBinOp(thistok)) OR (thistok IS LXRP))) {
        parseErr(end, "missing operand");
        return(TRUE);
    }
    return(FALSE);
}

void parseErr (char *end, char *mess)
{
    int c;

    c = *end;
    *end = '\0';
    prerror("; %s at: %s\n", mess, beginStr);
    *end = c;
    longjmp(jmpparse, -1);
}

/*---------------------------------------------------------------
 *
 *  doAction -- take action depending on operator
 *
 *---------------------------------------------------------------*/

void doAction (int op, char *parseStr)
{
    int i, n, val;
    int args[NARGS];

    n = nargs[op];
    for (i=0; i<n; i++)
        if (emptyStack(&valStack)) {
            parseErr(parseStr, "too few operands");
            args[i] = 0;
        } else
            args[i] = pop(&valStack);

    switch (op) {
    case LXEOI:
    case LXLP:
    case LXRP:
    case LXUPLUS:
        break;
    case LXEQ:
        push(&valStack, args[1] == args[0]);
        break;
    case LXOR:
        push(&valStack, args[1] || args[0]);
        break;
    case LXAND:
        push(&valStack, args[1] && args[0]);
        break;
    case LXNE:
        push(&valStack, args[1] != args[0]);
        break;
    case LXGT:
        push(&valStack, args[1] > args[0]);
        break;
    case LXGE:
        push(&valStack, args[1] >= args[0]);
        break;
    case LXLT:
        push(&valStack, args[1] < args[0]);
        break;
    case LXLE:
        push(&valStack, args[1] <= args[0]);
        break;
    case LXNOT:
        push(&valStack, NOT args[0]);
        break;
    case LXPLUS:
        push(&valStack, args[1] + args[0]);
        break;
    case LXMINUS:
        push(&valStack, args[1] - args[0]);
        break;
    case LXMULT:
        push(&valStack, args[1] * args[0]);
        break;
    case LXDIV:
        if (args[0] IS 0) {
            prerror("; attempt to divide by zero\n");
            push(&valStack, 0);
        } else
            push(&valStack, args[1] / args[0]);
        break;
    case LXMOD:
        if (args[0] IS 0) {
            prerror("; attempt to divide by zero\n");
            push(&valStack, args[1]);
        } else
            push(&valStack, args[1] % args[0]);
        break;
    case LXEXP:
        val = 1;
        for (i=1; i<=args[0]; i++)
            val *= args[1];
        push(&valStack, val);
        break;
    case LXUMINUS:
        push(&valStack, - args[0]);
        break;
    default:
        prerror(";internal error: shouldn't get here\n");
    }
}

/*---------------------------------------------------------------
 *
 *  eatInt -- use operator precedence parsing to compute integer value
 *
 *  "str" is modified to point directly after integer computed.
 *
 *---------------------------------------------------------------*/

int eatInt (char **str)
{
    int a, lastop;

    parenlevel = 0;
    thistok = LXEOI;
    initStack(&opStack);
    push(&opStack, LXEOI);
    initStack(&valStack);

    beginStr = *str; /* used for error messages */
    getNext(str);
    while (TRUE) {
        a = tos(&opStack);
        if ((thistok IS LXEOI) AND (a IS LXEOI)) {
            if (emptyStack(&valStack)) {
                prerror("; missing operand\n");
                return(0);
            } else
                return(pop(&valStack));
        }
        if (thistok IS LXINT) {
            push(&valStack, thisval);
            getNext(str);
        } else if (fprec[a] <= gprec[thistok]) {
            push(&opStack, thistok);
            getNext(str);
        } else {
            do {
                lastop = pop(&opStack);
                doAction(lastop, *str);
            } while (fprec[tos(&opStack)] >= gprec[lastop]);
        }
    }
}

int parseInt (char **str)
{
    int n;

    if (amParsing == 0) {
        amParsing = 1;
        beginStr = *str;
        if (setjmp(jmpparse)) {
            amParsing = 0;
            return(0);
        }
        n = eatInt(str);
        amParsing = 0;
        return(n);
    }
    n = eatInt(str);
    return(n);
}
/*
int parseInt (char **str)
{
    int n;

    if (setjmp(jmpparse))
      return(0);
    beginStr = *str;
    n = eatInt(str);
    return(n);
}
*/

int getInt (char *s)
{
    return(parseInt(&s));
}

int readInt (char **str)
{
    int n;

    n = parseInt(str);
    if (**str ISNT '\0')
        prerror("; premature end of expression\n");
    return(n);
}
