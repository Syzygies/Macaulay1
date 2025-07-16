// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "plist.h"
#include "mem.h"
#include "poly.h"
#include "term.h"
#include "monoms.h"
#include "stash.h"

void dl_copy(dlist *dl1, dlist *dl2)
{
    register int i;

    dlist_init(dl2);
    for (i = 1; i <= array_length((array *)dl1); i++)
        dlist_insert(dl2, dlist_ref(dl1, i));
}

void pl_copy(plist *pl1, plist *pl2)
{
    register int i;

    plist_init(pl2);
    for (i = 1; i <= array_length((array *)pl1); i++)
        plist_insert(pl2, p_copy(plist_ref(pl1, i)));
}

void pl_kill(plist *pl)
{
    int i;
    poly *p;

    for (i = 1; i <= array_length((array *)pl); i++)
    {
        p = (poly *)ref((array *)pl, i);
        p_kill(p);
    }
    free_array((array *)pl);
}

void dl_kill(dlist *dl)
{
    free_array((array *)dl);
}

void dl_lohi(dlist *dl, int *lo, int *hi)
{
    int i, d;

    if (array_length((array *)dl) == 0)
    {
        *lo = 0;
        *hi = 0;
        return;
    }
    *lo = dlist_ref(dl, 1);
    *hi = *lo;
    for (i = 2; i <= array_length((array *)dl); i++)
    {
        d = dlist_ref(dl, i);
        if (d < *lo)
            *lo = d;
        else if (d > *hi)
            *hi = d;
    }
}

gmatrix mod_init(void)
{
    gmatrix M;

    M = new_mod();
    M->next = NULL;
    M->modtype = MMAT;
    dlist_init(&(M->degrees));
    dlist_init(&(M->deggens));
    plist_init(&(M->gens));
    M->nstandard = 0;
    M->stdbasis = NULL;
    mo_init(M);
    return (M);
}

gmatrix mat_copy(gmatrix M)
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
    return (result);
}

void std_kill(mn_standard p)
{
    mn_standard temp;

    while (p != NULL)
    {
        temp = p;
        p = p->next;
        p_kill(&temp->standard);
        p_kill(&temp->change);
        free_slug((struct stash *)std_stash, (struct slug *)temp);
    }
}

void mod_kill(gmatrix M)
{
    if (M == NULL)
        return;
    dl_kill(&(M->degrees));
    dl_kill(&(M->deggens));
    pl_kill(&(M->gens));
    std_kill(M->stdbasis);
    M->nstandard = 0;
    mo_kill(M);
    free_slug((struct stash *)mod_stash, (struct slug *)M);
}

int degree(gmatrix M, poly f)
{
    if (f == NULL)
        return (0);
    return (tm_degree(poly_initial(f)) + dlist_ref(&M->degrees, tm_component(poly_initial(f))));
}
