/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "mtypes.h"

// void mo_init (gmatrix M);
// void mo_kill (gmatrix M);
// void mo_insert (gmatrix M, term t, int i);
// void mo_rg_insert (term t, int i);
// boolean mo_iscomplete (gmatrix M, int deg);
// void mo_reset (gmatrix M, int *i, int *j);
// void mo_rgreset (int *i, int *j, int jmax);
// boolean mo_next_pair (gmatrix M, int deg, int *i, int *j);
// boolean mo_rg_pair (gmatrix M, int deg, int *i, int *j);
// int tdegree (gmatrix M, term t);
// boolean mo_find_div (gmatrix M, term t, int *i, term s);
// boolean mo_rg_div (plist *pl, term t, int *i, term s);

/* the stuff in this file is temporary, until Dave's monomial package
        is included.  The routines have slightly different interfaces
        than the real ones will have, so it might take a
        little work */

void mo_init (gmatrix M)
{
    mn_init(M);
}

void mo_kill (gmatrix M)
{
    mn_kill(M);
}

void mo_insert (gmatrix M, term t, int i)
{
    mn_insert(M, t, i);
}

void mo_rg_insert (term t, int i)
{
#pragma unused(t,i)
}

boolean mo_iscomplete (gmatrix M, int deg)
{
#pragma unused(deg)
    return(mn_iscomplete(M));
}

void mo_reset (gmatrix M, int *i, int *j)
{
#pragma unused(M,i,j)
}

void mo_rgreset (int *i, int *j, int jmax)
{
#pragma unused(i,j,jmax)
}

boolean mo_next_pair (gmatrix M, int deg, int *i, int *j)
{
    return(mn_next_pair(M, deg, i, j));
}

boolean mo_rg_pair (gmatrix M, int deg, int *i, int *j)
{
#pragma unused(M,deg,i,j)
    return(FALSE); /* for now, never do any such pairs */
}

int tdegree (gmatrix M, term t)
{
    int result;

    result = DREF(M->degrees, tm_component(t)) + tm_degree(t);
    return(result);
}

boolean mo_find_div (gmatrix M, term t, int *i, term s)
{
    return(mn_find_div(&M->montab, t, i, s));
}

boolean mo_rg_div (plist *pl, term t, int *i, term s)
{
#pragma unused(pl,t,i,s)
    return(FALSE);
}
