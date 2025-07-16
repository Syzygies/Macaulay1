// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "monoms.h"
#include "hilb.h"
#include "term.h"

// the stuff in this file is temporary, until Dave's monomial package
// is included.  The routines have slightly different interfaces
// than the real ones will have, so it might take a
// little work

void mo_init(gmatrix M)
{
    mn_init(M);
}

void mo_kill(gmatrix M)
{
    mn_kill(M);
}

void mo_insert(gmatrix M, term t, char* i)
{
    mn_insert(M, t, i);
}

void mo_rg_insert(term t, char* i)
{
    // Stub function - does nothing
    (void)t;  // suppress unused parameter warning
    (void)i;
}

boolean mo_iscomplete(gmatrix M, int deg)
{
    (void)deg; // suppress unused parameter warning
    return mn_iscomplete(M);
}

void mo_reset(gmatrix M, char** i, char** j)
{
    // Stub function - does nothing
    (void)M;  // suppress unused parameter warning
    (void)i;
    (void)j;
}

void mo_rgreset(char** i, char** j, int jmax)
{
    // Stub function - does nothing
    (void)i;  // suppress unused parameter warning
    (void)j;
    (void)jmax;
}

boolean mo_next_pair(gmatrix M, int deg, char** i, char** j)
{
    return mn_next_pair(M, deg, i, j);
}

boolean mo_rg_pair(gmatrix M, int deg, char** i, char** j)
{
    (void)M; // suppress unused parameter warning
    (void)deg;
    (void)i;
    (void)j;
    return 0; // for now, never do any such pairs
}

int tdegree(gmatrix M, term t)
{
    int result;

    result = dlist_ref(&(M->degrees), tm_component(t)) + tm_degree(t);
    return result;
}

boolean mo_find_div(gmatrix M, term t, char** i, term s)
{
    return mn_find_div(&M->montab, t, i, s);
}

boolean mo_rg_div(plist* pl, term t, char** i, term s)
{
    (void)pl; // suppress unused parameter warning
    (void)t;
    (void)i;
    (void)s;
    return 0;
}
