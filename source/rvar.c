// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <ctype.h>
#include <setjmp.h>
#include <string.h>
#include "shared.h"
#include "rvar.h"
#include "human_io.h"
#include "parse.h"
#include "stash.h"
#include "monitor.h"
#include "ring.h"

jmp_buf jmpseq; // used from genVars to stop generating a sequence

constexpr int NINDICES = 10;
constexpr int VSIZE = 100;

// Allowed variables:

// (1) any string enclosed by vertical bars (string cannot include blanks,
// vertical bars, or special characters eaten by input.c
// (2) a single letter (upper and lower case are distinct)
// (3) a single letter followed by [int,int,...,int], where each int is
// any allowed integer expression.
// (4) [int,...,int], this is the same as (3), except the initial letter
// is missing.

// Sequences of variables can be made with "genvars" routine (used in
// creating rings).  A sequence is created by typing two allowed
// variables with a "-" between them (no blanks at all!).  Both variables
// must be of the same type as above (type (1) is not allowed), and have
// the same number of indices.

// Examples:
// |a,b,c|
// a
// a[1,2,3+(4*5)/2]
// [1,-2,3]

// Examples of generators:
// a[1]-b[3]  generates a[1]a[2]a[3]b[1]b[2]b[3]
// a-e        generates abcde

// The following type definition is used for generating
// sequences of variables as well as determining the string after
// evaluating the subscripts of the indeterminate.

// If nindices = -1     This is case (1) above.
// Otherwise nindices >= 0.  In this case nindices is the total number of
// subscripts, (so a[1,2] has 2 indices).
// Also, [1,2] has nindices=2.
// indices[0] is special:
// In cases (2),(3) above, indices[0] is the integer value of the letter.
// In case (4) above, indices[0] = 0.

typedef struct
{
    int nindices;          // -1=string var. >=0 means type (2)-(4) above
    int indices[NINDICES]; // actual indices, 0..nindices
} indet;

static void putPosInt(char **ss, int n)
{
    if (n >= 10)
        putPosInt(ss, n / 10);
    *(*ss)++ = (char)('0' + (n % 10));
}

static void putInt(char **ss, int n)
{
    if (n < 0)
    {
        *(*ss)++ = '-';
        n = -n;
    }
    putPosInt(ss, n);
    **ss = '\0';
}

// getIndexVar -- create string with "print" name of indexed variable

static void getIndexVar(indet *v, char *s)
{
    int i;

    if (v->nindices < 0)
    {
        *s = '\0';
        return;
    }
    else if (v->indices[0] != 0)
        *s++ = (char)v->indices[0];

    if (v->nindices == 0)
    {
        *s = '\0';
        return;
    }
    *s++ = '[';
    for (i = 1; i < v->nindices; i++)
    {
        putInt(&s, v->indices[i]);
        *s++ = ',';
    }
    putInt(&s, v->indices[v->nindices]);
    *s++ = ']';
    *s = '\0';
}

static boolean collectVar(char **str, indet *v, char *s)
{
    char c;

    if (!validVar(**str))
        return (0);

    if (**str == '|')
    {
        v->nindices = -1; // i.e. a string variable
        *s++ = *(*str)++;
        do
        {
            c = *(*str)++;
            *s++ = c;
        } while ((c != '|') && (c != '\0'));
        *s = '\0';
        return (1);
    }

    if (isalpha(**str))
        v->indices[0] = *(*str)++;
    else
        v->indices[0] = 0;

    v->nindices = 0;
    if (**str == '[')
    {
        do
        {
            (*str)++;
            v->nindices++;
            v->indices[v->nindices] = parseInt(str);
        } while (**str == ',');
        if (**str != ']')
        {
            s[0] = '\0';
            return (1);
        }
        else
            (*str)++;
    }
    getIndexVar(v, s);
    return (1);
}

// returns index of variable "s" in "vars", or -1 if not found

static int findVar(char **vars, int n, char *s)
{
    register int v;

    for (v = 0; v < n; v++)
        if (strcmp(vars[v], s) == 0)
            return (v);
    return (-1);
}

// insVar adds "s" as the vars[v] variable in "vars"

static void insVar(char **vars, int *v, char *s)
{
    register char *p;
    register int size;

    if (findVar(vars, *v, s) != -1)
    {
        prerror("; can't define variable %s twice\n", s);
        return;
    }
    size = (int)strlen(s) + 1;
    if ((size % 2) != 0)
        size++;
    p = (char *)gimmy((unsigned)size);
    strcpy(p, s);
    vars[*v] = p;
    (*v)++;
}

static indet *seqv1, *seqv2;
static indet seqv; // notice that this is the actual structure, not a pointer
static char **seqvars;
static int seqmaxvars; // maximum number of variables to be generated
static int *seqnvars;  // number of variables so far generated
static char seqs[100];

static void seqVars(int loc)
{
    int i, first, second, direction;

    if (loc == (1 + seqv1->nindices))
    {
        getIndexVar(&seqv, seqs);
        insVar(seqvars, seqnvars, seqs);
        if (*seqnvars == seqmaxvars)
            longjmp(jmpseq, -1);
        return;
    }
    else
    {
        first = seqv1->indices[loc];
        second = seqv2->indices[loc];
        direction = (first > second);
        for (i = first; ((direction && (i >= second)) || ((!direction) && (i <= second)));
             (direction ? i-- : i++))
        {
            seqv.indices[loc] = i;
            seqVars(loc + 1);
        }
    }
}

static boolean okSeqVars(indet *v1, indet *v2)
{
    if ((v1->nindices != v2->nindices) || (v1->nindices == -1))
        return (0);
    if ((v1->indices[0] == 0) && (v2->indices[0] == 0))
        return (1);
    if ((islower(v1->indices[0])) && (islower(v2->indices[0])))
        return (1);
    if ((isupper(v1->indices[0])) && (isupper(v2->indices[0])))
        return (1);
    return (0);
}

static void doSeqVars(char **vars, int *nvarsSoFar, indet *v1, indet *v2, int nvars)
{
    if (!okSeqVars(v1, v2))
    {
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
        return; /* this is the way we return from seqVars
                   if we generate too many variables */
    seqVars(0);
}

// nextVar -- very insistent about getting a variable

static void nextVar(char **str, indet *v, char *s, int numleft)
{
    while (!collectVar(str, v, s))
    {
        if (numleft == 1)
            prinput("one more variable, please          ");
        else
            prinput("%3d more variables, please         ", numleft);
        *str = get_str("");
    }
}

// Global Routines in this file

// validVar returns TRUE if "c" can begin a valid variable.

boolean validVar(char c)
{
    if ((c == '|') || (c == '[') || (isalpha(c)))
        return (1);
    return (0);
}

int parseVar(char **str)
{
    indet v;
    char s[100];

    if (collectVar(str, &v, s))
    {
        return (findVar(varnames, numvars, s));
    }
    else
        return (-1);
}

int getVar(const char *s)
{
    // Make a local copy to avoid cast-qual warning
    char buf[256];
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    char *str = buf;
    return (parseVar(&str));
}

int getVarRel(char **vnames, int len, char *s)
{
    indet v;
    char t[100];

    if (collectVar(&s, &v, t))
    {
        return (findVar(vnames, len, t));
    }
    else
        return (-1);
}

char **getVars(int nvars)
{
    register char **result;
    char *str;
    char s[VSIZE];
    int nv;
    indet v1, v2;

    result = (char **)(void *)gimmy((unsigned)nvars * (unsigned)sizeof(char *));
    prinput("%3d variables, please              ", nvars);
    str = get_str("");
    nv = 0;

    v1.nindices = -1; // just so we don't start with sequence
    while (nv < nvars)
    {
        nextVar(&str, &v1, s, nvars - nv);
        if (*str == '-')
        {
            str++;
            collectVar(&str, &v2, s);
            doSeqVars(result, &nv, &v1, &v2, nvars);
        }
        else
            insVar(result, &nv, s);
    }
    if (*str != '\0')
    {
        newline();
        print("too many variables given, ignoring %s\n", str);
    }
    return (result);
}
