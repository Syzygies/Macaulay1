/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "mtypes.h"

// void pl_new (plist *result, int size);
// void pl_add (plist *result, plist *a, plist *b);
// void pl_sub (plist *result, plist *a, plist *b);
// void pl_transpose (plist *result, plist *a, int ncols);
// void pl_mult (plist *result, plist *a, plist *b, int nrows);
// void pl_dsum (plist *result, plist *a, plist *b, int shiftval);
// void pl_submat (plist *result, plist *a, dlist *drows, dlist *dcols);
// void pl_diag (plist *result, poly f, int size);

extern poly p_dot();
extern poly compshift();
extern poly extract();

void pl_new (plist *result, int size)
{
    int i;

    for (i=1; i<=size; i++)
        pl_insert(result, NULL);
}

void pl_add (plist *result, plist *a, plist *b)
{
    int i, last;
    poly f, g;

    last = MIN(length(a), length(b));
    for (i=1; i<=last; i++) {
        f = p_copy(PREF(*a, i));
        g = p_copy(PREF(*b, i));
        p_add(&f, &g);
        pl_insert(result, f);
    }
}

void pl_sub (plist *result, plist *a, plist *b)
{
    int i, last;
    poly f, g;

    last = MIN(length(a), length(b));
    for (i=1; i<=last; i++) {
        f = p_copy(PREF(*a, i));
        g = p_copy(PREF(*b, i));
        p_sub(&f, &g);
        pl_insert(result, f);
    }
}

void pl_transpose (plist *result, plist *a, int ncols)
{
    int i;

    pl_new(result, ncols);
    for (i=1; i<=length(a); i++)
        getcol(result, PREF(*a, i), i);
}

void pl_mult (plist *result, plist *a, plist *b, int nrows)
{
    plist pl;
    poly f;
    int r, c;

    pl_init(&pl);
    pl_new(result, length(b));
    pl_transpose(&pl, a, nrows);
    for (r=1; r<=nrows; r++)
        for (c=1; c<=length(b); c++) {
            f = p_dot(PREF(pl, r), PREF(*b, c), r);
            p_add(&PREF(*result, c), &f);
        }
    pl_kill(&pl);
}

void pl_dsum (plist *result, plist *a, plist *b, int shiftval)
{
    int i;

    pl_copy(a, result);
    for (i=1; i<=length(b); i++)
        pl_insert(result, compshift(PREF(*b, i), shiftval));
}

void pl_submat (plist *result, plist *a, dlist *drows, dlist *dcols)
{
    int i;

    for (i=1; i<=length(dcols); i++)
        pl_insert(result, extract(PREF(*a, DREF(*dcols,i)), drows));
}

void pl_diag (plist *result, poly f, int size)
{
    int i;
    poly g;

    g = e_sub_i(1);
    for (i=1; i<=size; i++) {
        set_comp(g, i);
        pl_insert(result, p_mult(f, g));
    }
    p_kill(&g);
}
