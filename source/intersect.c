/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"

// gmatrix mkintersect (dlist *dl);
// void addintersect (gmatrix result, gmatrix M, dlist *dl);
// gmatrix mkquotient ();
// void addquotient (gmatrix result, gmatrix I, gmatrix J, int n);
// void inter_cmd (int argc, char *argv[]);
// boolean isIdeal (gmatrix g);
// void quot_cmd (int argc, char *argv[]);

extern int autocalc;
extern poly compshift();

gmatrix mkintersect (dlist *dl)
{
    gmatrix result;
    int i;

    result = mod_init();
    dl_copy(dl, &result->deggens);
    for (i=1; i<=length(dl); i++)
        pl_insert(&result->gens, NULL);
    return(result);
}

void addintersect (gmatrix result, gmatrix M, dlist *dl)
{
    int i, shiftval;
    poly f, g;

    shiftval = nrows(result);

    for (i=1; i<=length(dl); i++) {
        dl_insert(&result->degrees, DREF(*dl, i));
        f = PREF(result->gens, i);
        g = e_sub_i(shiftval+i);
        p_add(&f, &g);
        PREF(result->gens, i) = f;
    }

    for (i=1; i<=ncols(M); i++)
        gmInsert(result, compshift(PREF(M->gens, i), shiftval));
}

gmatrix mkquotient ()
{
    gmatrix result;

    result = mod_init();
    dl_insert(&result->deggens, 0);
    pl_insert(&result->gens, NULL);
    return(result);
}

void addquotient (gmatrix result, gmatrix I, gmatrix J, int n)
/* the number of columns to keep in syz command */
{
    int i, j, k, shiftval;
    poly f, g;

    for (i=1; i<=ncols(J); i++) {
        shiftval = nrows(result);
        f = PREF(J->gens, i);
        for (k=1; k<=n; k++) {
            g = compshift(f, shiftval+k-1);
            p_add(&PREF(result->gens,k), &g);
        }
        dl_addto(&result->degrees, -degree(J, f), &I->degrees);
        for (j=1; j<=ncols(I); j++)
            gmInsert(result, compshift(PREF(I->gens,j), shiftval));
    }
}

void inter_cmd (int argc, char *argv[])
{
    variable *box, *p, *syz();
    gmatrix M, g;
    int i, n;
    dlist *dl;

    if (argc < 3) {
        printnew("intersect <mat 1> ... <mat n> <result computation>\n");
        return;
    }
    GET_MOD(M, 1);
    n = nrows(M);
    dl = &M->degrees;
    for (i=1; i<argc-1; i++) {
        GET_rgMOD(M, i);
        if (n < nrows(M)) {
            prerror("; matrices have different numbers of rows\n");
            return;
        }
    }
    g = mkintersect(dl);
    for (i=1; i<argc-1; i++) {
        GET_rgMOD(M, i);
        addintersect(g, M, dl);
    }

    /* g is now correctly set up */

    if (!check_homog(g)) {
        prerror("; either degrees of rows are not the same for each matrix ");
        prerror("or one of these is not homogeneous\n");
        prerror("; setting %s to auxiliary matrix\n", argv[argc-1]);
        NEW_MOD(p, 3);
        set_value(p, g);
        return;
    }

    box = syz(argv[argc-1], g, n);
    doAutocalc(argv[argc-1]);
}

boolean isIdeal (gmatrix g)
{
    if (nrows(g) ISNT 1) return(FALSE);
    if (DREF(g->degrees, 1) ISNT 0) return(FALSE);
    return(TRUE);
}

void quot_cmd (int argc, char *argv[])
{
    gmatrix I, J, g;
    int n;
    variable *box, *p, *syz();

    if (argc ISNT 4) {
        printnew("quotient <matrix I> <matrix J> <result computation I:J>\n");
        return;
    }

    GET_MOD(I, 1);
    GET_rgMOD(J, 2);
    if (isIdeal(J)) {
        n = length(&I->degrees);
        g = mkintersect(&I->degrees);
    } else {
        n = 1;
        g = mkquotient();
    }
    addquotient(g, I, J, n);

    /*
        NEW_MOD(p, 3);
        set_value(p, g);
        return;
    */
    /* g is now correctly set up */

    if (!check_homog(g)) {
        prerror("; either degrees of rows are not the same for each matrix ");
        prerror("or one of these is not homogeneous\n");
        prerror("; setting %s to auxiliary matrix\n", argv[3]);
        NEW_MOD(p, 3);
        set_value(p, g);
        return;
    }

    box = syz(argv[argc-1], g, n);
    doAutocalc(argv[argc-1]);
}
