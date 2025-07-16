// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <stdlib.h> // for random functions
#include "shared.h"
#include "gmatrix.h"
#include "det.h"       // pinit_array, pincr_array, pfaff4
#include "gm_poly.h"   // extr1, p_diff, tensorShift, p_contract, compshift
#include "poly.h"      // p_xjei, p_mult, p_add, p_sub, p_copy, p_kill, p_negate, e_sub_i
#include "plist.h"     // degree, mod_init, dl_* functions
#include "gm_plist.h"  // pl_add, pl_sub, pl_mult, pl_transpose, pl_submat, pl_wedge
#include "gm_dlist.h" /* dl_trans, dl_addto, dl_new, dl_composite, dl_newall, dl_wedge, dl_copy,
                         dl_kill */
#include "qring.h"     // qrgReduce
#include "monitor.h"   // prerror, prflush
#include "term.h"      // tm_degree
#include "standard.h"  // division, reduce
#include "mac.h"       // have_intr
#include "ring.h"      // varWeight
#include "charp.h"     // normalize
#include "vars.h"      // current_ring
#include "set.h"       // verbose

int nrows(gmatrix g)
{
    return array_length(&(g->degrees));
}

int ncols(gmatrix g)
{
    return array_length(&(g->deggens));
}

void gmInsert(gmatrix g, poly f)
{
    int n;

    plist_insert(&g->gens, f);
    if (f == NULL)
        n = 0;
    else
        n = degree(g, f);
    dlist_insert(&g->deggens, n);
}

poly gm_elem(gmatrix g, int i, int j, int comp)
{
    poly f;

    if (g == NULL)
        return NULL;
    if ((nrows(g) < 1) || (ncols(g) < 1))
        return NULL;
    f = plist_ref(&g->gens, j);
    return extr1(f, i, comp);
}

gmatrix gm_add(gmatrix f, gmatrix g)
{
    gmatrix result;

    result = mod_init();
    if (nrows(f) != nrows(g))
    {
        prerror("; matrices have different number of rows\n");
        return result;
    }
    if (ncols(f) != ncols(g))
    {
        prerror("; matrices have different number of columns\n");
        return result;
    }
    pl_add(&(result->gens), &(f->gens), &(g->gens));
    dl_copy(&(f->degrees), &(result->degrees));
    dl_copy(&(f->deggens), &(result->deggens));
    return result;
}

gmatrix gm_sub(gmatrix f, gmatrix g)
{
    gmatrix result;

    result = mod_init();
    if (nrows(f) != nrows(g))
    {
        prerror("; matrices have different number of rows\n");
        return result;
    }
    if (ncols(f) != ncols(g))
    {
        prerror("; matrices have different number of columns\n");
        return result;
    }
    pl_sub(&(result->gens), &(f->gens), &(g->gens));
    dl_copy(&(f->degrees), &(result->degrees));
    dl_copy(&(f->deggens), &(result->deggens));
    return result;
}

gmatrix gm_transpose(gmatrix f)
{
    gmatrix result;

    result = mod_init();
    pl_transpose(&(result->gens), &(f->gens), nrows(f));
    dl_trans(&(result->degrees), &(f->deggens));
    dl_trans(&(result->deggens), &(f->degrees));
    return result;
}

gmatrix gm_mult(gmatrix f, gmatrix g)
{
    gmatrix result;

    result = mod_init();
    if (ncols(f) != nrows(g))
    {
        prerror("; matrices are wrong shape to be multiplied\n");
        return result;
    }
    pl_mult(&(result->gens), &(f->gens), &(g->gens), nrows(f));
    dl_copy(&(f->degrees), &(result->degrees));
    if (ncols(f) > 0)
    {
        dl_addto(&(result->deggens), dlist_ref(&f->deggens, 1) - dlist_ref(&g->degrees, 1),
                 &(g->deggens));
    }
    else
        dl_copy(&g->deggens, &result->deggens);
    return result;
}

gmatrix gm_smult(gmatrix M, poly f)
{
    gmatrix result;
    int i, deg;
    poly g;

    result = mod_init();
    dl_copy(&M->degrees, &result->degrees);
    if (f == NULL)
        deg = 0;
    else
        deg = tm_degree(poly_initial(f));
    dl_addto(&result->deggens, deg, &M->deggens);
    for (i = 1; i <= ncols(M); i++)
    {
        g = plist_ref(&M->gens, i);
        plist_insert(&result->gens, p_mult(f, g));
    }
    return result;
}

void gm_dsum(gmatrix result, gmatrix M)
{
    int n, i;

    n = nrows(result);
    dl_addto(&result->degrees, 0, &M->degrees);
    dl_addto(&result->deggens, 0, &M->deggens);
    for (i = 1; i <= ncols(M); i++)
        plist_insert(&result->gens, compshift(plist_ref(&M->gens, i), n));
}

gmatrix gm_iden(int n)
{
    gmatrix result;
    int i;
    poly f;

    result = mod_init();
    dl_new(&result->degrees, n);
    for (i = 1; i <= n; i++)
    {
        f = e_sub_i(i);
        qrgReduce(&f); // unlikely event that 1 \in quotient ring
        gmInsert(result, f);
    }
    return result;
}

gmatrix gm_diag(gmatrix M)
{
    gmatrix result;
    int thiscomp, r, c, i, j, degf, deg;
    poly f;

    result = mod_init();
    r = nrows(M);
    c = ncols(M);
    dl_new(&result->degrees, r * c);
    thiscomp = 1;
    for (i = 1; i <= c; i++)
    {
        f = plist_ref(&M->gens, i);
        degf = dlist_ref(&M->deggens, i);
        for (j = 1; j <= r; j++)
        {
            deg = degf - dlist_ref(&M->degrees, j);
            dlist_insert(&result->deggens, deg);
            plist_insert(&result->gens, extr1(f, j, thiscomp++));
        }
    }
    return result;
}

gmatrix gm_submat(gmatrix g, dlist* drows, dlist* dcols)
{
    gmatrix result;

    result = mod_init();
    pl_submat(&(result->gens), &(g->gens), drows, dcols);
    dl_composite(&(result->degrees), drows, &(g->degrees));
    dl_composite(&(result->deggens), dcols, &(g->deggens));
    return result;
}

gmatrix gm_wedge(gmatrix g, int p)
{
    gmatrix result;

    result = mod_init();
    dl_wedge(p, &(g->degrees), &(result->degrees));
    dl_wedge(p, &(g->deggens), &(result->deggens));
    pl_wedge(p, nrows(g), ncols(g), &(g->gens), &(result->gens));
    return result;
}

gmatrix gm_pfaff4(gmatrix g)
{
    gmatrix result;
    poly f;
    int t[5]; // zero th spot isnt used
    int a, sign;

    result = mod_init();
    dlist_insert(&(result->degrees), 0);
    pinit_array(4, ncols(g), t, &a, &sign);
    do
    {
        f = pfaff4(g, t);
        plist_insert(&(result->gens), f);
        dlist_insert(&(result->deggens), degree(result, f));
    } while (pincr_array(4, ncols(g), t, &a, &sign));
    return result;
}

gmatrix gm_flatten(gmatrix g)
{
    gmatrix result;
    int r, c;
    poly f, h;

    result = mod_init();
    dlist_insert(&(result->degrees), 0);
    for (c = 1; c <= ncols(g); c++)
    {
        f = plist_ref(&g->gens, c);
        for (r = 1; r <= nrows(g); r++)
        {
            h = extr1(f, r, 1);
            plist_insert(&(result->gens), h);
            dlist_insert(&(result->deggens), degree(result, h));
        }
    }
    return result;
}

gmatrix gm_genMat(dlist* rows, dlist* cols, int firstvar)
{
    gmatrix result;
    register int r, c, maxR, maxC, val;
    int dcol, thisdeg;
    poly f, g;

    result = mod_init();
    maxR = array_length(rows);
    maxC = array_length(cols);
    dl_newall(&(result->degrees), 0, maxR);
    for (c = 1; c <= maxC; c++)
    {
        dcol = dlist_ref(cols, c) + firstvar;
        f = NULL;
        thisdeg = dlist_ref(&rgDegs, 1 + dcol + dlist_ref(rows, 1)); // deg of first var
        dlist_insert(&result->deggens, thisdeg);
        for (r = 1; r <= maxR; r++)
        {
            val = dcol + dlist_ref(rows, r);
            if ((val >= 0) && (val < numvars))
            {
                g = p_xjei(val, r);
                p_add(&f, &g);
            }
        }
        plist_insert(&result->gens, f);
    }
    return result;
}

gmatrix gm_jacobian(gmatrix g, expterm vars)
{
    register gmatrix result;
    register int v, i, e;
    poly f, h;

    if (ncols(g) >= 1)
        e = dlist_ref(&g->degrees, 1); // this is possibly not = 0
    result = mod_init();
    for (i = 1; i <= ncols(g); i++) /* the "e" resets the degrees as if this
                                       were an ideal */
        dlist_insert(&result->degrees, e - degree(g, plist_ref(&g->gens, i)));
    for (v = 0; v < numvars; v++)
    {
        if (vars[v] == 0)
            continue;
        dlist_insert(&result->deggens, -varWeight((ring)current_ring, v));
        f = NULL;
        for (i = 1; i <= ncols(g); i++)
        {
            h = p_diff(plist_ref(&g->gens, i), v, i);
            p_add(&f, &h);
        }
        plist_insert(&result->gens, f);
    }
    return result;
}

// BUG? FIX! why doesn't gm_concat() call mo_init()? -db

void gm_concat(gmatrix result, gmatrix g)
{
    register int i;
    register poly f;

    if (nrows(result) < nrows(g))
    {
        dl_kill(&result->degrees);
        dl_copy(&g->degrees, &result->degrees);
    }
    for (i = 1; i <= ncols(g); i++)
    {
        f = p_copy(plist_ref(&g->gens, i));
        plist_insert(&(result->gens), f);
        dlist_insert(&result->deggens, dlist_ref(&g->deggens, i));
    }
}

gmatrix gm_lift(gmatrix M, gmatrix N)
{
    register int i;
    poly f, g, h;
    gmatrix result;

    result = mod_init();
    dl_copy(&M->deggens, &result->degrees);
    for (i = 1; i <= ncols(N); i++)
    {
        f = p_copy(plist_ref(&N->gens, i));
        division(M, &f, &g, &h);
        if (h != NULL)
            prerror("; column #%d not in module\n", i);
        p_negate(&g);
        gmInsert(result, g);
        if (verbose > 0)
            prflush("x");
        if (have_intr())
            return result;
    }
    return result;
}

void gm_reduce(gmatrix M, gmatrix N, gmatrix* redN, gmatrix* liftN)
{
    register int i;
    poly f, g, h;

    debug_assert("gm_reduce: M must have at least as many rows as N", (nrows(M) >= nrows(N)));
    *redN = mod_init();
    dl_copy(&M->degrees, &(*redN)->degrees);
    if (liftN != NULL)
    {
        *liftN = mod_init();
        dl_copy(&M->deggens, &(*liftN)->degrees);
    }
    for (i = 1; i <= ncols(N); i++)
    {
        f = p_copy(plist_ref(&N->gens, i));
        if (liftN == NULL)
            h = reduce(M, &f);
        else
        {
            division(M, &f, &g, &h);
            p_negate(&g);
            gmInsert(*liftN, g);
        }
        gmInsert(*redN, h);
        if (verbose > 0)
            prflush("x");
        if (have_intr())
            return;
    }
}

gmatrix gm_trace(gmatrix M)
{
    gmatrix result;
    int top, i;
    poly f, g;

    result = mod_init();
    dl_new(&result->degrees, 1);

    if (nrows(M) > ncols(M))
        top = ncols(M);
    else
        top = nrows(M);

    f = NULL;
    for (i = 1; i <= top; i++)
    {
        g = extr1(plist_ref(&M->gens, i), i, 1);
        p_add(&f, &g);
    }
    gmInsert(result, f);
    return result;
}

gmatrix gm_tensor(gmatrix M, gmatrix N)
{
    gmatrix result;
    int m, n, i, j;
    poly f;

    result = mod_init();

    m = nrows(M);
    n = nrows(N);
    for (i = 1; i <= m; i++)
        for (j = 1; j <= n; j++)
            dlist_insert(&result->degrees, dlist_ref(&M->degrees, i) + dlist_ref(&N->degrees, j));

    for (j = 1; j <= m; j++)
        for (i = 1; i <= ncols(N); i++)
        {
            f = plist_ref(&N->gens, i);
            gmInsert(result, compshift(f, (j - 1) * n));
        }
    for (j = 1; j <= n; j++)
        for (i = 1; i <= ncols(M); i++)
        {
            f = plist_ref(&M->gens, i);
            gmInsert(result, tensorShift(f, n, j));
        }
    return result;
}

gmatrix gm_outer(gmatrix M, gmatrix N)
{
    gmatrix result;
    int mr, mc, nr, nc, i, j, jc, ic, jr;
    poly f, g, h, a, b, c;

    mr = nrows(M);
    mc = ncols(M);
    nr = nrows(N);
    nc = ncols(N);
    result = mod_init();
    for (i = 1; i <= mr; i++)
        for (j = 1; j <= nr; j++)
            dlist_insert(&result->degrees, dlist_ref(&M->degrees, i) + dlist_ref(&N->degrees, j));
    for (i = 1; i <= mc; i++)
        for (j = 1; j <= nc; j++)
            dlist_insert(&result->deggens, dlist_ref(&M->deggens, i) + dlist_ref(&N->deggens, j));

    for (jc = 1; jc <= mc; jc++)
    {
        f = extr1(plist_ref(&M->gens, jc), 1, 1);
        for (ic = 1; ic <= nc; ic++)
        {
            g = plist_ref(&N->gens, ic);
            h = p_mult(f, g);

            for (jr = 2; jr <= mr; jr++)
            {
                a = extr1(plist_ref(&M->gens, jc), jr, 1);
                b = p_mult(a, g);
                p_kill(&a);
                c = compshift(b, (jr - 1) * nr);
                p_kill(&b);
                p_add(&h, &c);
            }
            plist_insert(&result->gens, h);
        }
        p_kill(&f);
    }
    return result;
}

void powerp(gmatrix M, gmatrix result, poly f, int lastn, int pow)
{
    int i;
    poly g;

    if (pow == 0)
    {
        gmInsert(result, f);
    }
    else
    {
        for (i = lastn; i <= ncols(M); i++)
        {
            g = p_mult(f, plist_ref(&M->gens, i));
            powerp(M, result, g, i, pow - 1);
        }
        p_kill(&f);
    }
}

gmatrix gm_power(gmatrix M, int n)
{
    gmatrix result;
    poly f;

    result = mod_init();
    dlist_insert(&result->degrees, 0);
    f = e_sub_i(1);
    powerp(M, result, f, 1, n);
    return result;
}

gmatrix gm_diff(gmatrix M, gmatrix N, boolean usecoef)
{
    int i, j;
    gmatrix result;
    poly x, f, g, h;

    result = mod_init();
    dl_copy(&M->deggens, &result->degrees);

    for (j = 1; j <= ncols(N); j++)
    {
        g = plist_ref(&N->gens, j);
        f = NULL;
        for (i = 1; i <= ncols(M); i++)
        {
            x = plist_ref(&M->gens, i);
            h = p_contract(x, g, usecoef, i);
            p_add(&f, &h);
        }
        gmInsert(result, f);
    }
    return result;
}

gmatrix gm_random(int r, int c)
{
    int i, j, n;
    gmatrix result;
    poly f, g;
    field a;

    result = mod_init();
    dl_new(&result->degrees, r);

    for (i = 1; i <= c; i++)
    {
        f = NULL;
        for (j = 1; j <= r; j++)
        {
            n = rand();
            a = normalize(n);
            if (a == 0)
                continue;
            g = e_sub_i(j);
            g->coef = a;
            p_add(&f, &g);
        }
        qrgReduce(&f); // unlikely event that 1 \in quotient ring
        gmInsert(result, f);
    }
    return result;
}

gmatrix gm_compress(gmatrix M)
{
    gmatrix result;
    int i;
    poly f;

    result = mod_init();
    dl_copy(&M->degrees, &result->degrees);

    for (i = 1; i <= ncols(M); i++)
    {
        f = plist_ref(&M->gens, i);
        if (f != NULL)
            gmInsert(result, p_copy(f));
    }
    return result;
}

gmatrix gm_vars(void)
{
    gmatrix result;
    int i;

    result = mod_init();
    dlist_insert(&result->degrees, 0);
    for (i = 0; i < numvars; i++)
        gmInsert(result, p_xjei(i, 1));
    return result;
}
