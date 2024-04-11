/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "mtypes.h"

// void dl_copy (dlist *dl1, dlist *dl2);
// void pl_copy (plist *pl1, plist *pl2);
// void pl_kill (plist *pl);
// void dl_kill (dlist *dl);
// void dl_lohi (dlist *dl, int *lo, int *hi);
// gmatrix mod_init ();
// gmatrix mat_copy (gmatrix M);
// void std_kill (mn_standard p);
// void mod_kill (gmatrix M);
// int degree (gmatrix M, poly f);

void dl_copy (dlist *dl1, dlist *dl2)
{
    int i;

    dl_init(dl2);
    for (i=1; i<=LENGTH(*dl1); i++)
        dl_insert(dl2, DREF(*dl1, i));
}

void pl_copy (plist *pl1, plist *pl2)
{
    int i;

    pl_init(pl2);
    for (i=1; i<=LENGTH(*pl1); i++)
        pl_insert(pl2, p_copy(PREF(*pl1, i)));
}

void pl_kill (plist *pl)
{
    int i;

    for (i=1; i<=LENGTH(*pl); i++)
        p_kill(&PREF(*pl, i));
    free_array(pl);
}

void dl_kill (dlist *dl)
{
    free_array(dl);
}

/* dl_lohi puts into "lo", and "hi", the degree of the lowest and highest
        integer of "dl".
*/

void dl_lohi (dlist *dl, int *lo, int *hi)
{
    int i, d;

    if (length(dl) IS 0) {
        *lo = 0;
        *hi = 0;
        return;
    }
    *lo = DREF(*dl, 1);
    *hi = *lo;
    for (i=2; i<=length(dl); i++) {
        d = DREF(*dl, i);
        if (d < *lo)
            *lo = d;
        else if (d > *hi)
            *hi = d;
    }
}

gmatrix mod_init ()
{
    gmatrix M;

    M = new_mod();
    M->next = NULL;
    M->modtype = MMAT;
    dl_init(&(M->degrees));
    dl_init(&(M->deggens));
    pl_init(&(M->gens));
    M->nstandard = 0;
    M->stdbasis = NULL;
    mo_init(M);
    return(M);
}

gmatrix mat_copy (gmatrix M)
{
    gmatrix result;

    result = new_mod();
    result->next = NULL;
    result->modtype = MMAT;
    dl_copy(&(M->degrees), &(result->degrees));
    dl_copy(&(M->deggens), &(result->deggens));
    pl_copy(&(M->gens), &(result->gens));
    result->nstandard = 0;
    result->stdbasis = NULL;
    mo_init(result);
    return(result);
}

void std_kill (mn_standard p)
{
    mn_standard temp;

    while (p ISNT NULL) {
        temp = p;
        p = p->next;
        p_kill(&temp->standard);
        p_kill(&temp->change);
        free_slug(std_stash, temp);
    }
}

void mod_kill (gmatrix M)
{
    if (M IS NULL) return;
    dl_kill(&(M->degrees));
    dl_kill(&(M->deggens));
    pl_kill(&(M->gens));
    std_kill(M->stdbasis);
    M->nstandard = 0;
    mo_kill(M);
    free_slug(mod_stash, M);
}

int degree (gmatrix M, poly f)
{
    if (f IS NULL) return(0);
    return(tm_degree(INITIAL(f)) + DREF(M->degrees,
                                        tm_component(INITIAL(f))));
}
