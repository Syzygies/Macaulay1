// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "getdet.h"
#include <stdio.h>
#include <setjmp.h>

#include "comb_rec.h"
#include "tables.h"
#include "gmatrix.h"
#include "poly.h"
#include "plist.h"
#include "vars.h"
#include "cmds.h"
#include "parse.h"
#include "monitor.h"

// Note: Original header mentioned "two implementations" - meaning unclear
int Temp; // keep this with swapint

#ifdef OPCOUNT
unsigned multcount = 0, addcount = 0;
#endif

// GET_MOD inline function implementation
bool get_mod(gmatrix* g, int argc, char** argv, int i)
{
    if (i >= argc)
        return false;
    *g = vget_mod(argv[i]);
    return (*g != NULL);
}

// poly calc_det
// calculate the minor with rows r[0] through r[p-1] and
// columns c[0] through c[p-1] by expansion and cofactors

poly calc_det(gmatrix M, int* r, int* c, int p)
{
    int negate = 0;
    int i;
    poly result;
    int temp;
    poly* prevresult; // lookup to see if we've done this before
    poly ptemp;
    poly ptemp2;

    // NOTE - base case will occur at p==1 when prevresult will be
    // found in the table

    if (*(prevresult = stored_value(r, c, p)) /* get a pointer to where
                                                 the result belongs */
        != NO_ENTRY)
        return (*prevresult); /* this one has already been
                                 done */

    // pull out each column and put it at the beginning, then calculate
    // the remaining minor

    if ((ptemp2 = matrixsub(*c, *r)) == NULL)
        result = NULL;
    else
        result = p_mult(ptemp2, calc_det(M, r + 1, c + 1, p - 1));

#ifdef OPCOUNT
    multcount++;
#endif

    for (i = 1; i < p; i++)
    {
        negate = !negate;

        swapints(&c[0], &c[i]);

        if ((ptemp2 = matrixsub(*c, *r)) != NULL)
        {
            ptemp = p_mult(ptemp2, calc_det(M, r + 1, c + 1, p - 1));

            if (negate)
                p_sub(&result, &ptemp);
            else
                p_add(&result, &ptemp);

#ifdef OPCOUNT
            multcount++;
            addcount++;
#endif
        }
    }

    // pulling out the columns has disordered c. Fix it

    temp = c[0];
    for (i = 0; i < p - 1; i++)
        c[i] = c[i + 1];
    c[p - 1] = temp;

    // stick the newly calculated number in the table at the previously
    // looked up spot

    *prevresult = result;

    return result;
}

gmatrix determinants(gmatrix M, int s)
{
    int x, y;
    struct comb_rec rowlist, collist;
    gmatrix result;

    x = ncols(M);
    y = nrows(M);

    // Bounds checking: minor size must not exceed matrix dimensions
    if (s > x || s > y)
    {
        prerror("; getdet: minor size %d exceeds matrix dimensions %dx%d\n", s, y, x);
        return mod_init();
    }
    if (s <= 0)
    {
        prerror("; getdet: minor size must be positive\n");
        return mod_init();
    }

    result = mod_init();
    dlist_insert(&result->degrees, 0);

    // Note: C23 version of init_combs and init_comb_recs return void
    // Original returned 0 on error, but C23 version will exit on malloc failure
    init_combs(max_int(x, y));
    init_comb_recs(s);
    if (init_tables(M, x, y, s) == 0)
    {
        printf("not enough memory to use this command\n");
        free_comb_recs();
        free_combs();
        return result;
    }

    get_comb_rec(&rowlist, s);
    get_comb_rec(&collist, s);
    init_comb_rec(&rowlist);

    do
    {
        init_comb_rec(&collist);
        do
        {
            gmInsert(result, p_copy(calc_det(M, rowlist.t, collist.t, s)));
        } while (next_comb(collist.t, &collist.sign, collist.length, x));
    } while (next_comb(rowlist.t, &rowlist.sign, rowlist.length, y));

    free_comb_rec(&rowlist);
    free_comb_rec(&collist);

    free_tables();
    free_comb_recs();
    free_combs();

#ifdef OPCOUNT
    fprintf(stderr, "Used %u adds and %u multiplies.\n", addcount, multcount);
#endif

    return result;
}

void determinants_cmd(int argc, char* argv[])
{
    gmatrix g;
    int n;
    variable* p;

    if (argc != 4)
    {
        printnew("determinants <matrix> <p> <result>\n");
        return;
    }

    if (!get_mod(&g, argc, argv, 1))
        return;

    n = getInt(argv[2]);

    if ((n <= 0) || (n > nrows(g)) || (n > ncols(g)))
    {
        prerror("; all size %d minors are zero\n", n);
        return;
    }

    if (((p) = make_var(argv[3], MAINVAR, VMODULE, (variable*)current_ring)) == NULL)
        return;
    set_value(p, determinants(g, n));
}
