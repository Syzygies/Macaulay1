// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "gm_plist.h"
#include "plist.h"    // pl_kill, pl_copy
#include "poly.h"     // p_copy, p_add, p_sub, p_mult, p_kill, p_dot, e_sub_i, set_comp
#include "gm_poly.h"  // compshift, extract, getcol

void pl_new(plist* result, int size)
{
    int i;

    for (i = 1; i <= size; i++)
        plist_insert(result, NULL);
}

void pl_add(plist* result, plist* a, plist* b)
{
    int i, last;
    poly f, g;

    last = min_int(array_length((array*)a), array_length((array*)b));
    for (i = 1; i <= last; i++)
    {
        f = p_copy(plist_ref(a, i));
        g = p_copy(plist_ref(b, i));
        p_add(&f, &g);
        plist_insert(result, f);
    }
}

void pl_sub(plist* result, plist* a, plist* b)
{
    int i, last;
    poly f, g;

    last = min_int(array_length((array*)a), array_length((array*)b));
    for (i = 1; i <= last; i++)
    {
        f = p_copy(plist_ref(a, i));
        g = p_copy(plist_ref(b, i));
        p_sub(&f, &g);
        plist_insert(result, f);
    }
}

void pl_transpose(plist* result, plist* a, int ncols)
{
    int i;

    pl_new(result, ncols);
    for (i = 1; i <= array_length((array*)a); i++)
        getcol(result, plist_ref(a, i), i);
}

void pl_mult(plist* result, plist* a, plist* b, int nrows)
{
    plist pl;
    poly f;
    int r, c;

    plist_init(&pl);
    pl_new(result, array_length((array*)b));
    pl_transpose(&pl, a, nrows);
    for (r = 1; r <= nrows; r++)
        for (c = 1; c <= array_length((array*)b); c++)
        {
            f = p_dot(plist_ref(&pl, r), plist_ref(b, c), r);
            poly p = plist_ref(result, c);
            p_add(&p, &f);
            plist_set(result, c, p);
        }
    pl_kill(&pl);
}

void pl_dsum(plist* result, plist* a, plist* b, int shiftval)
{
    int i;

    pl_copy(a, result);
    for (i = 1; i <= array_length((array*)b); i++)
        plist_insert(result, compshift(plist_ref(b, i), shiftval));
}

void pl_submat(plist* result, plist* a, dlist* drows, dlist* dcols)
{
    int i;

    for (i = 1; i <= array_length((array*)dcols); i++)
        plist_insert(result, extract(plist_ref(a, dlist_ref(dcols, i)), drows));
}

void pl_diag(plist* result, poly f, int size)
{
    int i;
    poly g;

    g = e_sub_i(1);
    for (i = 1; i <= size; i++)
    {
        set_comp(g, i);
        plist_insert(result, p_mult(f, g));
    }
    p_kill(&g);
}
