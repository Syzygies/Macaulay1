/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"

// void fd_wr (FILE *fil, field a, boolean p_one);
// void tm_wr (FILE *fil, term t);
// void p_wr (FILE *fil, poly f, int comp);
// void mat_wr (FILE *fil, gmatrix M);
// void prmat_cmd (int argc, char *argv[]);

int maxterms = 10;
static int nterms; /* number of monomials printed in current poly */
static int nexps;  /* number of parts of current monomial printed */
/* parts include: lead coef, var**exp */
/* lead +, - don't count */

void fd_wr (FILE *fil, field a, boolean p_one)
{
    int n, m;
    boolean p_plus;

    p_plus = (nterms > 0);
    lift(charac, a, &n, &m); /* n = denom., m = numerator */
    if (n IS 0) {
        m = a;
        n = 1;
    } else if (n < 0) {
        n = -n;
        m = -m;
    }
    if (n IS 1) {
        if ((abs(m) ISNT 1) OR p_one)
            nexps++;
        int_pprint(fil, m, p_one, p_plus);
    } else {
        nexps++;
        int_pprint(fil, m, TRUE, p_plus);
        fprint(fil, "/");
        int_pprint(fil, n, FALSE, FALSE);
    }
}

void tm_wr (FILE *fil, term t)
{
    int i, a;
    expterm exp;

    sToExp(t, exp);
    for (i=0; i<numvars; i++) {
        a = exp[i];
        if (a > 0) {
            if (nexps > 0)
                fprint(fil, "*");
            fprint(fil, "%s", varnames[i]);
            nexps++;
        }
        if (a > 1) {
            fprint(fil, "^");
            int_pprint(fil, a, FALSE, FALSE);
        }
    }
}

void p_wr (FILE *fil, poly f, int comp)
{
    boolean z;

    nterms = 0;
    while (f ISNT NULL)
    {
        if (tm_component(INITIAL(f)) IS comp) {
            z = tm_iszero(INITIAL(f));
            if (nterms % maxterms IS (maxterms-1))
                fprint(fil, "\n  ");
            nexps = 0;
            fd_wr(fil,f->coef,z);
            tm_wr(fil,INITIAL(f));
            nterms++;
        }
        f = f->next;
    }
    if (nterms IS 0) fprint(fil, "0");
}

void mat_wr (FILE *fil, gmatrix M)
{
    int i, j;

    fprint(fil,"{\n");
    for (i=1; i<=nrows(M); i++) {
        fprint(fil, "  {");
        for (j=1; j<=ncols(M); j++) {
            p_wr(fil, PREF(M->gens, j), i);
            if (j < ncols(M)) fprint(fil, ",\n    ");
        }
        fprint(fil, "}");
        if (i < nrows(M)) fprint(fil, ",\n");
    }
    fprint(fil,"\n}\n");
}

void prmat_cmd (int argc, char *argv[])
{
    gmatrix M;

    if (argc ISNT 2) {
        printnew("prmat <matrix>\n");
        return;
    }
    GET_MOD(M, 1);
    mat_wr(outfile, M);
}
