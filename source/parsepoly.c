/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "mtypes.h"
#include "parse.h"

// void pGetNext (char **str);
// int pCheckError (int lasttok, int thistok, char *end);
// void pDoAction (int op, char *parseStr);
// poly eatPoly (char **str, int comp);
// poly parsePoly (char **str, int comp);
// poly readPoly (char **str, int comp);
// poly getPoly (char *s, int comp);
// poly rdPoly (int whichComp);
// poly rdPolyStr (int argc, char **argv, int whichComp);

extern poly p_xjei();
extern poly p_intpoly();
extern poly getPolyId();
extern poly p_div();
extern poly p_mod();

/* variables used in parsing */

/*
 * Only polynomials are on the value stack.  This was done for
 * "simplicity" (in any case, to do this all correct needs a total
 * rewrite).  The only place an integer is needed is in exponentiation
 * Since these are never stacked, the integer value is placed in a
 * single variable: "ithisval".
 */

int pthistok;  /* one of LXEOI, ..., LXUMINUS */
poly pthisval; /* if pthistok = LXPOLY, this is the poly that it is */
int ithisval;  /* integer value, if pthistok = LXINT: ONLY after '^' */

int pComponent; /* row number of resulting polynomial */

stack pOpStack;    /* operator stack */
stack pValStack; /* value stack */

int pParenlevel; /* should never be negative.  End value should be 0 */

/*pGetNext(str)
char **str;
{
    p1GetNext(str);
    print("token is %d\n", pthistok);
}*/

void pGetNext (char **str)
{
    int lasttok, varnum;
    char c;
    char ident[100];

    lasttok = pthistok;
    c = *(*str)++;

    /* insert '*' between terminals */
    if (((c IS '(') OR (c IS '{') OR (validVar(c)))
            AND ((lasttok IS LXINT) OR (lasttok IS LXRP) OR (lasttok IS LXPOLY))) {

        (*str)--;
        pthistok = LXMULT;
        return;
    }

    /* insert '^' if "looks like" exponential */
    if ((isdigit(c)) AND ((lasttok IS LXRP) OR (lasttok IS LXPOLY))) {

        (*str)--;
        pthistok = LXEXP;
        return;
    }

    /* if the last token was '^', then we now expect an integer: get it */

    if (lasttok IS LXEXP) {
        if (c IS '(') {
            ithisval = eatInt(str);
            if (**str ISNT ')')
                parseErr(*str, "missing right parenthesis");
            (*str)++;
        } else if (canStartVar(c)) {
            (*str)--;
            getIdentifier(str, ident);
            ithisval = getIdVal(ident);
        } else if (isdigit(c)) {
            ithisval = collectInt(c, str);
        } else
            parseErr(str, "bad exponent");
        pthistok = LXINT;
        return;
    }

    if (c IS '*') {
        if (**str IS '*') {
            (*str)++;
            pthistok = LXEXP;
        } else
            pthistok = LXMULT;
    } else if (c IS '/')
        pthistok = LXDIV;
    else if (c IS '&')
        pthistok = LXMOD;
    else if (c IS '^')
        pthistok = LXEXP;
    else if (c IS '(') {
        pParenlevel++;
        pthistok = LXLP;
    } else if (c IS ')') {
        if (pParenlevel IS 0) {
            pthistok = LXEOI;
            (*str)--;
        } else {
            pParenlevel--;
            pthistok = LXRP;
        }
    } else if (c IS '+') {
        if (isunary(pthistok))
            pthistok = LXUPLUS;
        else
            pthistok = LXPLUS;
    } else if (c IS '-') {
        if (isunary(pthistok))
            pthistok = LXUMINUS;
        else
            pthistok = LXMINUS;
    } else if (validVar(c)) {
        (*str)--;
        varnum = parseVar(str);
        if (varnum IS -1) {
            prerror("; ring variable not defined\n");
            pthisval = NULL;
        } else
            pthisval = p_xjei(varnum, pComponent);
        pthistok = LXPOLY;
    } else if (isdigit(c)) {
        pthisval = p_intpoly(collectInt(c,str), pComponent);
        pthistok = LXPOLY;
    } else if (canStartVar(c)) {   /* notice this is a limited set */
        (*str)--;
        getIdentifier(str, ident);
        pthisval = getPolyId(ident, pComponent);
        pthistok = LXPOLY;
    } else {
        (*str)--;
        pthistok = LXEOI;
    }
    pCheckError(lasttok, pthistok, *str);
}

int pCheckError (int lasttok, int thistok, char *end)
{
    if (((isOperator(lasttok)) OR (lasttok IS LXLP))
            AND ((isBinOp(thistok)) OR (thistok IS LXRP))) {

        parseErr(end, "missing operand");
        return(TRUE);
    }
    return(FALSE);
}

void pDoAction (int op, char *parseStr)
{
    poly f, g;
    int i, n;
    poly args[NARGS];

    if (op IS LXEXP)
        n = 1;  /* exponential kludge: second argument is in "ithisval" */
    else
        n = nargs[op];
    for (i=0; i<n; i++) {
        if (emptyStack(&pValStack)) {
            parseErr(parseStr, "too few operands");
            args[i] = NULL;
        } else
            args[i] = (poly) pop(&pValStack);
    }

    switch (op) {
    case LXPLUS:
        p_add(args, args+1);
        push(&pValStack, args[0]);
        break;
    case LXMINUS:
        p_sub(args+1, args);
        push(&pValStack, args[1]);
        break;
    case LXMULT:
        f = p_mult(args[0], args[1]);
        push(&pValStack, f);
        p_kill(args);
        p_kill(args+1);
        break;
    case LXDIV:
        f = p_div(args+1, args[0]);
        push(&pValStack, f);
        p_kill(args);
        break;
    case LXMOD:
        f = p_mod(args+1, args[0]);
        push(&pValStack, f);
        p_kill(args);
        break;
    case LXEXP:
        n = ithisval;
        f = e_sub_i(pComponent);
        for (i=1; i<=n; i++) {
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

poly eatPoly (char **str, int comp)
{
    int a, lastop;

    pComponent = comp; /* make this global */

    pParenlevel = 0;
    pthistok = LXEOI;
    initStack(&pOpStack);
    push(&pOpStack, LXEOI);
    initStack(&pValStack);

    pGetNext(str);
    while (TRUE) {
        a = tos(&pOpStack);
        if ((pthistok IS LXEOI) AND (a IS LXEOI)) {
            if (emptyStack(&pValStack)) {
                prerror("; missing operand\n");
                return(NULL);
            } else
                return((poly) pop(&pValStack));
        }
        if (pthistok IS LXPOLY) {
            push(&pValStack, pthisval);
            pGetNext(str);
        } else if (pthistok IS LXINT)
            pGetNext(str);
        else if (fprec[a] <= gprec[pthistok]) {
            push(&pOpStack, pthistok);
            pGetNext(str);
        } else {
            do {
                lastop = pop(&pOpStack);
                pDoAction(lastop, *str);
            } while (fprec[tos(&pOpStack)] >= gprec[lastop]);
        }
    }
}

poly parsePoly (char **str, int comp)
{
    poly f;

    amParsing = 1;
    if (setjmp(jmpparse)) {
        amParsing = 0;
        return(NULL);
    }
    beginStr = *str;
    f = eatPoly(str, comp);
    if (**str ISNT '\0')
        prerror("; premature end of expression\n");
    amParsing = 0;
    return(f);
}

poly readPoly (char **str, int comp)
{
    poly f;

    f = parsePoly(str, comp);
    if (**str ISNT '\0')
        prerror("; premature end of expression\n");
    return(f);
}

poly getPoly (char *s, int comp)
{
    return(parsePoly(&s, comp));
}

poly rdPoly (int whichComp)
{
    poly f, g;
    char *str;

    f = NULL;
    while (get_contstr("", &str)) {
        g = getPoly(str, whichComp);
        p_add(&f, &g);
    }
    g = getPoly(str, whichComp);
    p_add(&f, &g);
    return(f);
}

poly rdPolyStr (int argc, char **argv, int whichComp)
{
    poly f, g;
    char *str;

    f = getPoly(argv[0], whichComp);
    if (argc IS 1) return(f); /* no continuation */
    while (get_contstr("", &str)) {
        g = getPoly(str, whichComp);
        p_add(&f, &g);
    }
    g = getPoly(str, whichComp);
    p_add(&f, &g);
    return(f);
}
