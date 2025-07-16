// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "intersect.h"
#include "cmds.h"      // For vget_mod(), vget_rgmod()
#include "gmatrix.h"   // For mod_init(), nrows(), ncols(), gmInsert()
#include "gm_poly.h"   // For compshift()
#include "human_io.h"  // For check_homog()
#include "monitor.h"   // For printnew(), prerror(), intflush()
#include "plist.h"     // For degree()
#include "poly.h"      // For e_sub_i(), p_add()
#include "rescmds.h"   // For syz(), doAutocalc()
#include "set.h"       // For autocalc
#include "vars.h"      // For make_var(), set_value(), current_ring
#include "gm_dlist.h"  // For dl_addto()
#include "generic.h"   // For VMODULE

// Variable type constants from vars.h
// MAINVAR comes from shared.h

gmatrix mkintersect(dlist* dl)
{
    gmatrix result;
    int i;

    result = mod_init();
    dl_copy(dl, &result->deggens);
    for (i = 1; i <= length(dl); i++)
        plist_insert(&result->gens, NULL);
    return result;
}

void addintersect(gmatrix result, gmatrix M, dlist* dl)
{
    int i, shiftval;
    poly f, g;

    shiftval = nrows(result);

    for (i = 1; i <= length(dl); i++)
    {
        dlist_insert(&result->degrees, dlist_ref(dl, i));
        f = plist_ref(&result->gens, i);
        g = e_sub_i(shiftval + i);
        p_add(&f, &g);
        *plist_ref_ptr(&result->gens, i) = f;
    }

    for (i = 1; i <= ncols(M); i++)
        gmInsert(result, compshift(plist_ref(&M->gens, i), shiftval));
}

gmatrix mkquotient(void)
{
    gmatrix result;

    result = mod_init();
    dlist_insert(&result->deggens, 0);
    plist_insert(&result->gens, NULL);
    return result;
}

void addquotient(gmatrix result, gmatrix I, gmatrix J, int n)
{
    int i, j, k, shiftval;
    poly f, g;

    for (i = 1; i <= ncols(J); i++)
    {
        shiftval = nrows(result);
        f = plist_ref(&J->gens, i);
        for (k = 1; k <= n; k++)
        {
            g = compshift(f, shiftval + k - 1);
            p_add(plist_ref_ptr(&result->gens, k), &g);
        }
        dl_addto(&result->degrees, -degree(J, f), &I->degrees);
        for (j = 1; j <= ncols(I); j++)
            gmInsert(result, compshift(plist_ref(&I->gens, j), shiftval));
    }
}

void inter_cmd(int argc, char* argv[])
{
    variable* p;
    gmatrix M, g;
    int i, n;
    dlist* dl;

    if (argc < 3)
    {
        printnew("intersect <mat 1> ... <mat n> <result computation>\n");
        return;
    }
    if ((M = vget_mod(argv[1])) == NULL)
    {
        printnew("Error: invalid module\n");
        return;
    }
    n = nrows(M);
    dl = &M->degrees;
    for (i = 1; i < argc - 1; i++)
    {
        if ((M = vget_rgmod(argv[i])) == NULL)
        {
            printnew("Error: invalid module\n");
            return;
        }
        if (n < nrows(M))
        {
            prerror("; matrices have different numbers of rows\n");
            return;
        }
    }
    g = mkintersect(dl);
    for (i = 1; i < argc - 1; i++)
    {
        if ((M = vget_rgmod(argv[i])) == NULL)
        {
            printnew("Error: invalid module\n");
            return;
        }
        addintersect(g, M, dl);
    }

    // g is now correctly set up

    if (!check_homog(g))
    {
        prerror("; either degrees of rows are not the same for each matrix ");
        prerror("or one of these is not homogeneous\n");
        prerror("; setting ");
        prerror(argv[argc - 1]);
        prerror(" to auxiliary matrix\n");
        p = make_var(argv[argc - 1], MAINVAR, VMODULE, current_ring);
        set_value(p, g);
        return;
    }

    (void)syz(argv[argc - 1], g, n);
    doAutocalc(argv[argc - 1]);
}

boolean isIdeal(gmatrix g)
{
    if (nrows(g) != 1)
        return false;
    if (dlist_ref(&g->degrees, 1) != 0)
        return false;
    return true;
}

void quot_cmd(int argc, char* argv[])
{
    gmatrix I, J, g;
    int n;
    variable* p;

    if (argc != 4)
    {
        printnew("quotient <matrix I> <matrix J> <result computation I:J>\n");
        return;
    }

    if ((I = vget_mod(argv[1])) == NULL)
    {
        printnew("Error: invalid module\n");
        return;
    }
    if ((J = vget_rgmod(argv[2])) == NULL)
    {
        printnew("Error: invalid module\n");
        return;
    }
    if (isIdeal(J))
    {
        n = length(&I->degrees);
        g = mkintersect(&I->degrees);
    }
    else
    {
        n = 1;
        g = mkquotient();
    }
    addquotient(g, I, J, n);

    // p = make_var(argv[3], MAINVAR, VMODULE);
    // set_value(p, g);
    // return;
    // g is now correctly set up

    if (!check_homog(g))
    {
        prerror("; either degrees of rows are not the same for each matrix ");
        prerror("or one of these is not homogeneous\n");
        prerror("; setting ");
        prerror(argv[3]);
        prerror(" to auxiliary matrix\n");
        p = make_var(argv[3], MAINVAR, VMODULE, current_ring);
        set_value(p, g);
        return;
    }

    (void)syz(argv[argc - 1], g, n);
    doAutocalc(argv[argc - 1]);
}
