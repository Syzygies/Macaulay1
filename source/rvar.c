/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "style.h"
#include <setjmp.h>

// void putPosInt (char **ss, int n);
// void putInt (char **ss, int n);
// void getIndexVar (indet *v, char *s);
// boolean collectVar (char **str, indet *v, char *s);
// int findVar (char **vars, int n, char *s);
// void insVar (char **vars, int *v, char *s);
// void seqVars (int loc);
// boolean okSeqVars (indet *v1, indet *v2);
// void doSeqVars (char **vars, int *nvarsSoFar, indet *v1, indet *v2, int nvars);
// void nextVar (char **str, indet *v, char *s, int numleft);
boolean validVar (char c);
// int parseVar (char **str);
// int getVar (char *s);
// int getVarRel (char **varnames, int len, char *s);
// char ** getVars (int nvars);

jmp_buf jmpseq;  /* used from genVars to stop generating a sequene */

extern char *get_str();
extern char *gimmy();

extern char **varnames;
extern int numvars;

#define NINDICES 10
#define VSIZE 100

/*--------------------------------------------------------------
 *
 *  Allowed variables:
 *
 * (1) any string enclosed by vertical bars (string cannot include blanks,
 *  vertical bars, or special characters eaten by input.c
 * (2) a single letter (upper and lower case are distinct)
 * (3) a single letter followed by [int,int,...,int], where each int is
 *  any allowed integer expression.
 * (4) [int,...,int], this is the same as (3), except the initial letter
 *  is missing.
 *
 *  Sequences of variables can be made with "genvars" routine (used in
 *  creating rings).  A sequence is created by typing two allowed
 *  variables with a "-" between them (no blanks at all!).  Both variables
 *  must be of the same type as above (type (1) is not allowed), and have
 *  the same number of indices.
 *
 *  Examples:
 *  |a,b,c|
 *  a
 *  a[1,2,3+(4*5)/2]
 *  [1,-2,3]
 *
 *  Examples of generators:
 *  a[1]-b[3]  generates a[1]a[2]a[3]b[1]b[2]b[3]
 *  a-e        generates abcde
 *
 *--------------------------------------------------------------*/

/*
 * The following type definition is used for generating
 * sequences of variables as well as determining the string after
 * evaluating the subscripts of the indeterminate.
 *
 * If nindices = -1 This is case (1) above.
 * Otherwise nindices >= 0.  In this case nindices is the total number of
 *  subscripts, (so a[1,2] has 2 indices).
 *  Also, [1,2] has nindices=2.
 * indices[0] is special:
 * In cases (2),(3) above, indices[0] is the integer value of the letter.
 * In case (4) above, indices[0] = 0.
 */

typedef struct {
    int nindices;  /* -1=string var. >=0 means type (2)-(4) above */
    int indices[NINDICES]; /* actual indices, 0..nindices */
} indet;

void putPosInt (char **ss, int n)
{
    if (n >= 10)
        putPosInt(ss, n/10);
    *(*ss)++ = '0' + (n%10);
}

void putInt (char **ss, int n)
{
    if (n < 0) {
        *(*ss)++ = '-';
        n = -n;
    }
    putPosInt(ss, n);
    **ss = '\0';
}

/*
 *  getIndexVar -- create string with "print" name of indexed variable
 */

void getIndexVar (indet *v, char *s)
{
    int i;

    if (v->nindices < 0) {
        *s = '\0';
        return;
    } else if (v->indices[0] ISNT 0)
        *s++ = v->indices[0];

    if (v->nindices IS 0) {
        *s = '\0';
        return;
    }
    *s++ = '[';
    for (i=1; i<v->nindices; i++) {
        putInt(&s, v->indices[i]);
        *s++ = ',';
    }
    putInt(&s, v->indices[v->nindices]);
    *s++ = ']';
    *s = '\0';
}

boolean collectVar (char **str, indet *v, char *s)
{
    char c;

    if (NOT(validVar(**str)))
        return(FALSE);

    if (**str IS '|') {
        v->nindices = -1;  /* i.e. a string variable */
        *s++ = *(*str)++;
        do {
            c = *(*str)++;
            *s++ = c;
        } while ((c ISNT '|') AND (c ISNT '\0'));
        if (c IS '\0')
            s[0] = '\0';
        else
            *s = '\0';
        return(TRUE);
    }

    if (isalpha(**str))
        v->indices[0] = *(*str)++;
    else
        v->indices[0] = 0;

    v->nindices = 0;
    if (**str IS '[') {
        do {
            (*str)++;
            v->nindices++;
            v->indices[v->nindices] = parseInt(str);
        } while (**str IS ',');
        if (**str ISNT ']') {
            s[0] = '\0';
            return(TRUE);
        } else
            (*str)++;
    }
    getIndexVar(v, s);
    return(TRUE);
}

/* returns index of variable "s" in "vars", or -1 if not found */

int findVar (char **vars, int n, char *s)
/* array 0..n-1 of variables */
/* desired string to match against a variable */
{
    int v;

    for (v=0; v<n; v++)
        if (strcmp(vars[v], s) IS 0)
            return(v);
    return(-1);
}

/* insVar adds "s" as the vars[v] variable in "vars" */

void insVar (char **vars, int *v, char *s)
/* v is incremented if "s" is added to "vars" */
{
    char *p;
    int size;

    if (findVar(vars, *v, s) ISNT -1) {
        prerror("; can't define variable %s twice\n", s);
        return;
    }
    size = strlen(s)+1;
    if ((size%2) ISNT 0) size++;
    p = (char *) gimmy(size);
    strcpy(p,s);
    vars[*v] = p;
    (*v)++;
}

indet *seqv1, *seqv2;
indet seqv;  /* notice that this is the actual structure, not a pointer */
char **seqvars;
int seqmaxvars; /* maximum number of variables to be generated */
int *seqnvars;  /* number of variables so far generated */
char seqs[100];

void seqVars (int loc)
{
    int i, first, second, direction;

    if (loc IS (1 + seqv1->nindices)) {
        getIndexVar(&seqv, seqs);
        insVar(seqvars, seqnvars, seqs);
        if (*seqnvars IS seqmaxvars)
            longjmp(jmpseq, -1);
        return;
    } else {
        first = seqv1->indices[loc];
        second = seqv2->indices[loc];
        direction = (first > second);
        for (i=first; ((direction AND (i>=second)) OR
                       ((NOT direction) AND (i<=second)));
                (direction ? i-- : i++)) {
            seqv.indices[loc] = i;
            seqVars(loc+1);
        }
    }
}

boolean okSeqVars (indet *v1, indet *v2)
{
    if ((v1->nindices ISNT v2->nindices) OR
            (v1->nindices IS -1))
        return(FALSE);
    if ((v1->indices[0] IS 0) AND (v2->indices[0] IS 0))
        return(TRUE);
    if ((islower(v1->indices[0])) AND (islower(v2->indices[0])))
        return(TRUE);
    if ((isupper(v1->indices[0])) AND (isupper(v2->indices[0])))
        return(TRUE);
    return(FALSE);
}

void doSeqVars (char **vars, int *nvarsSoFar, indet *v1, indet *v2, int nvars)
/* max. number of variables to find */
{
    if (NOT okSeqVars(v1, v2)) {
        prerror("; non-compatible variables can't be sequenced\n");
        return;
    }
    seqvars = vars;
    seqv1 = v1;
    seqv2 = v2;
    seqnvars = nvarsSoFar;
    seqv.nindices = v1->nindices;
    seqmaxvars = nvars;
    if (setjmp(jmpseq))
        return;    /* this is the way we return from seqVars
               if we generate too many variables */
    seqVars(0);
}

/*
 * nextVar -- very insistent about getting a variable
 */

void nextVar (char **str, indet *v, char *s, int numleft)
{
    while (NOT collectVar(str, v, s)) {
        if (numleft IS 1)
            prinput("one more variable, please          ");
        else
            prinput("%3d more variables, please         ", numleft);
        *str = get_str("");
    }
}

/*------------------------------------------------------------------
 *
 *  Global Routines in this file
 *
 *------------------------------------------------------------------*/

/*
 * validVar returns TRUE if "c" can begin a valid variable.
 */

boolean validVar (char c)
{
    if ((c IS '|') OR (c IS '[') OR (isalpha(c)))
        return(TRUE);
    return(FALSE);
}

int parseVar (char **str)
{
    indet v;
    char s[100];

    if (collectVar(str, &v, s)) {
        return(findVar(varnames, numvars, s));
    } else
        return(-1);
}

int getVar (char *s)
{
    return(parseVar(&s));
}

int getVarRel (char **varnames, int len, char *s)
{
    indet v;
    char t[100];

    if (collectVar(&s, &v, t)) {
        return(findVar(varnames, len, t));
    } else
        return(-1);
}

char ** getVars (int nvars)
{
    char **result;
    char *str;
    char s[VSIZE];
    int nv;
    indet v1, v2;

    result = (char **) gimmy(nvars*sizeof(char *));
    prinput("%3d variables, please              ", nvars);
    str = get_str("");
    nv = 0;

    v1.nindices = -1;  /* just so we don't start with sequence */
    while (nv < nvars) {
        nextVar(&str, &v1, s, nvars-nv);
        if (*str IS '-') {
            str++;
            collectVar(&str, &v2, s);
            doSeqVars(result, &nv, &v1, &v2, nvars);
        } else
            insVar(result, &nv, s);
    }
    if (*str ISNT '\0') {
        newline();
        print("too many variables given, ignoring %s\n", str);
    }
    return(result);
}
