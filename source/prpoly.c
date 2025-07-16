// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <stdio.h>
#include <stdlib.h>
#include "shared.h"
#include "prpoly.h"
#include "charp.h"     // For lift(), charac
#include "cmds.h"      // For vget_mod()
#include "gmatrix.h"   // For nrows(), ncols()
#include "human_io.h"  // For int_pprint()
#include "monitor.h"   // For fprint(), printnew()
#include "ring.h"      // For varnames[], numvars
#include "shell.h"     // For outfile
#include "term.h"      // For sToExp(), tm_component(), tm_iszero()

int maxterms = 10;
static int nterms;  // number of monomials printed in current poly
static int nexps;   // number of parts of current monomial printed
                    // parts include: lead coef, var**exp
                    // lead +, - don't count

void fd_wr(FILE* fil, field a, boolean p_one)
{
    int n, m;
    boolean p_plus;

    p_plus = (nterms > 0);
    lift(charac, a, &n, &m); // n = denom., m = numerator
    if (n == 0)
    {
        m = a;
        n = 1;
    }
    else if (n < 0)
    {
        n = -n;
        m = -m;
    }
    if (n == 1)
    {
        if ((abs(m) != 1) || p_one)
            nexps++;
        int_pprint(fil, m, p_one, p_plus);
    }
    else
    {
        nexps++;
        int_pprint(fil, m, true, p_plus);
        fprint(fil, "/");
        int_pprint(fil, n, false, false);
    }
}

void tm_wr(FILE* fil, term t)
{
    register int i, a;
    expterm exp;

    sToExp(t, exp);
    for (i = 0; i < numvars; i++)
    {
        a = exp[i];
        if (a > 0)
        {
            if (nexps > 0)
                fprint(fil, "*");
            fprint(fil, "%s", varnames[i]);
            nexps++;
        }
        if (a > 1)
        {
            fprint(fil, "^");
            int_pprint(fil, a, false, false);
        }
    }
}

void p_wr(FILE* fil, poly f, int comp)
{
    boolean z;

    nterms = 0;
    while (f != NULL)
    {
        if (tm_component(poly_initial(f)) == comp)
        {
            z = tm_iszero(poly_initial(f));
            if (nterms % maxterms == (maxterms - 1))
                fprint(fil, "\n  ");
            nexps = 0;
            fd_wr(fil, f->coef, z);
            tm_wr(fil, poly_initial(f));
            nterms++;
        }
        f = f->next;
    }
    if (nterms == 0)
        fprint(fil, "0");
}

void mat_wr(FILE* fil, gmatrix M)
{
    int i, j;

    fprint(fil, "{\n");
    for (i = 1; i <= nrows(M); i++)
    {
        fprint(fil, "  {");
        for (j = 1; j <= ncols(M); j++)
        {
            p_wr(fil, plist_ref(&M->gens, j), i);
            if (j < ncols(M))
                fprint(fil, ",\n    ");
        }
        fprint(fil, "}");
        if (i < nrows(M))
            fprint(fil, ",\n");
    }
    fprint(fil, "\n}\n");
}

void prmat_cmd(int argc, char* argv[])
{
    gmatrix M;

    if (argc != 2)
    {
        printnew("prmat <matrix>\n");
        return;
    }
    if ((M = vget_mod(argv[1])) == NULL)
        return;
    mat_wr(outfile, M);
}
