/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "style.h"
#include "iring.h"
#include "vars.h"

// void i_tull (int numvars, int d);
// void tull (arrow head, int plus);
// boolean hulb (gmatrix M, int d);
// void hulb_cmd (int argc, char *argv[]);

extern int verbose;
extern int prlevel;
extern int numvars; /* number of vars in current ring */
long tullexp[NVARS];
int deg;
extern int binom();

void i_tull (int numvars, int d)
{
    int i;

    for (i=0; i<numvars; i++)
        tullexp[i] = 0;
    deg = d;
}

void tull (arrow head, int plus)
{
    int i, j, d, e, n, n2, c1, c2, s;
    int *a;

    d = head->umh.mpred;
    n = head->umh.mn;
    n2 = head->umh.mpren;
    a = head->umh.mloc->umn.mexp;
    for ( i=0; i<n; ++i)
        d += a[i];
    if (d > deg) return;
    /* d == degree of new monomial */
    e = deg - d;
    c1 = binom(numvars-1+e, e);
    c2 = (e == 0 ? 0 : binom(numvars-1+e, e-1));
    s = (plus ? -1 : 1); /* yeah, that's right, buster */
    for (i=0, j=0; i<n2; ++i, ++j)
        tullexp[j] += s * (c1*head->umh.mstack[i].mpre + c2);
    for (i=0; i<n; ++i, ++j)
        tullexp[j] += s * (c1*a[i] + c2);
}

boolean hulb (gmatrix M, int d)
{
    /* returns TRUE if user doesn't interrupt */
    arrow head;
    arrow monhilb();

    i_tull(numvars, d);
    if ((head = monhilb(M, 1, tull)) == NULL)
        return(FALSE);
    monrefund(head);
    return(TRUE);
}
/*
boolean hulb (gmatrix M, int d)
{

    expterm nexp;
    poly f;
    arrow head;
    modgen mg;

    i_tull(numvars, d);
    head = monnewhead(numvars);
    stdFirst(M, &mg, USESTD);
    while ((f=stdNext(&mg)) ISNT NULL) {
        if (have_intr()) {
            monrefund(head);
            print("\n");
            return(FALSE);
        }
        sToExp(INITIAL(f), nexp);
        monadjoin(head, nexp, tull);
    }
    monrefund(head);
    return(TRUE);
}
*/

void hulb_cmd (int argc, char *argv[])
{
    gmatrix M;
    int i, d;

    if (argc ISNT 3) {
        print("hulb <standard basis> <deg>\n");
        return;
    }
    GET_MOD(M, 1);
    d = getInt(argv[2]);
    stdWarning(M);
    hulb(M, d);
    for (i=0; i<numvars; i++)
        print("%ld ", tullexp[i]);
    print("\n");
}
