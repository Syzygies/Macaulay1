/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"

// boolean hom_check (gmatrix M, char *name);
// void doAutocalc (char *s);
// variable *res (char *name, gmatrix M, int depth);
// variable *nres (char *name, variable *p, int depth);
// variable *std (char *name, variable *p);
// variable *lift_std (char *name, gmatrix M);
// variable *syz (char *name, gmatrix M, int ncomps);
// variable *modulo (char *name, gmatrix M, gmatrix N);
// variable *istd (char *name, gmatrix M, int ncomps);
// void res_cmd (int argc, char *argv[]);
// void nres_cmd (int argc, char *argv[]);
// void std_cmd (int argc, char *argv[]);
// void liftstd_cmd (int argc, char *argv[]);
// void syz_cmd (int argc, char *argv[]);
// void modulo_cmd (int argc, char *argv[]);
// void istd_cmd (int argc, char *argv[]);

extern int autocalc;
extern int autodegree;

boolean hom_check (gmatrix M, char *name)
{
    if (!check_homog(M)) {
        prerror("; matrix %s isn't homogeneous; either homogenize it\n",
                name);
        prerror("; using 'homog', or change degrees using 'setdegs'\n");
        return(FALSE);
    }
    return(TRUE);
}

void doAutocalc (char *s)
/* name of computation */
{
    variable *box;

    garb_collect();
    box = find_var(s);

    if (autocalc < 0)
        calc(box, TRUE);
    else if (autocalc > 0)
        calc(box, FALSE, autodegree);
}

variable *res (char *name, gmatrix M, int depth)
{
    variable *result;
    int lo, hi;

    dl_lohi(&M->deggens, &lo, &hi);
    result = mkstarter(name, hi-1, lo-2, 1);
    mkshift("", 1);    /* so calc w n gives n-linear strand */
    mkres("1", -1, M);
    mk_sev_nres(2, depth-1, -1, 0);
    return(result);
}

variable *nres (char *name, variable *p, int depth)
/* a gmatrix var. */
{
    variable *result;
    int lo, hi;
    gmatrix M;

    M = VAR_MODULE(p);
    dl_lohi(&M->deggens, &lo, &hi);
    result = mkstarter(name, hi, lo-1, 1);
    mkemit("", p);
    mk_sev_nres(1, depth, -1, 0);
    return(result);
}

variable *std (char *name, variable *p)
{
    variable *result, *alias;
    int lo, hi;
    gmatrix M;

    M = VAR_MODULE(p);
    dl_lohi(&M->deggens, &lo, &hi);
    result = mkstarter(name, hi, lo-1, 1);
    mkemit("", p);
    alias = mkstd("1");
    set_alias(result, alias);
    return(result);
}

variable *lift_std (char *name, gmatrix M)
{
    variable *result, *alias;
    int lo, hi;

    dl_lohi(&M->deggens, &lo, &hi);
    result = mkstarter(name, hi, lo-1, 1);
    alias = mkres("1", -1, M);
    mktrash("");
    set_alias(result, alias);
    return(result);
}

variable *syz (char *name, gmatrix M, int ncomps)
{
    variable *result, *alias;
    int lo, hi;

    dl_lohi(&M->deggens, &lo, &hi);
    result = mkstarter(name, hi, lo-1, 1);
    mkres("0", ncomps, M);
    alias = mknres("1", 0);
    set_alias(result, alias);
    return(result);
}

variable *modulo (char *name, gmatrix M, gmatrix N)
{
    gmatrix g;
    int i;
    variable *p;

    g = mat_copy(M);
    for (i=1; i<=ncols(N); i++)
        gmInsert(g, p_copy(PREF(N->gens, i)));
    if (!check_homog(g)) {
        prerror("; concatenating both matrices produces a matrix which is not\n");
        prerror("; homogeneous.  Either homogenize, or change degrees using setdegs\n");
        return(NULL);
    }
    p = syz(name, g, ncols(M));
    mod_kill(g);
    return(p);
}

variable *istd (char *name, gmatrix M, int ncomps)
{
    variable *result, *alias;

    result = mkstarter(name, -1, -1, 1);
    alias = mkistd("1", ncomps, M);
    if (ncomps ISNT 0)
        mkcollect("2");
    set_alias(result, alias);
    return(result);
}

void res_cmd (int argc, char *argv[])
{
    gmatrix M;
    int maxdepth;
    variable *box;

    if ((argc < 3) OR (argc > 4)) {
        printnew("res <module> <computation> [max depth]\n");
        return;
    }
    GET_MOD(M, 1);
    if (argc >= 4)
        maxdepth = getInt(argv[3]);
    else maxdepth = MAX(numvars,2);

    if (!hom_check(M, argv[1])) return;
    box = res(argv[2], M, maxdepth);
    doAutocalc(argv[2]);
}

void nres_cmd (int argc, char *argv[])
{
    variable *p, *box;
    gmatrix M;
    int maxdepth;

    if ((argc < 3) OR (argc > 4)) {
        printnew("nres <module> <computation> [max depth]\n");
        return;
    }
    GET_VMOD(p, 1);
    M = VAR_MODULE(p);
    if (argc >= 4)
        maxdepth = getInt(argv[3]);
    else maxdepth = numvars;

    if (!hom_check(M, argv[1])) return;
    box = nres(argv[2], p, maxdepth);
    doAutocalc(argv[2]);
}

void std_cmd (int argc, char *argv[])
{
    variable *p, *box;
    gmatrix M;

    if (argc ISNT 3) {
        printnew("std <module> <computation>\n");
        return;
    }
    GET_VMOD(p, 1);
    M = VAR_MODULE(p);
    if (M IS NULL) return;
    if (!hom_check(M, argv[1])) return;
    box = std(argv[2], p);
    doAutocalc(argv[2]);
}

void liftstd_cmd (int argc, char *argv[])
{
    gmatrix M;

    if (argc ISNT 3) {
        printnew("lift_std <matrix> <computation>\n");
        return;
    }
    GET_MOD(M, 1);
    if (!hom_check(M, argv[1])) return;
    lift_std(argv[2], M);
    doAutocalc(argv[2]);
}

void syz_cmd (int argc, char *argv[])
{
    variable *box;
    gmatrix M;
    int n;

    if (argc ISNT 4) {
        printnew("syz <module> <# components(-1=all,0,...)> <computation>\n");
        return;
    }
    GET_MOD(M, 1);
    n = getInt(argv[2]);
    if (n < 0) n = -1;
    if (!hom_check(M, argv[1])) return;
    box = syz(argv[3], M, n);
    doAutocalc(argv[3]);
}

void modulo_cmd (int argc, char *argv[])
{
    variable *box;
    gmatrix M, N;

    if (argc ISNT 4) {
        printnew("modulo <matrix> <matrix> <computation>\n");
        return;
    }
    GET_MOD(M, 1);
    GET_rgMOD(N, 2);
    box = modulo(argv[3], M, N);
    if (box IS NULL) return;
    doAutocalc(argv[3]);
}

void istd_cmd (int argc, char *argv[])
{
    variable *box;
    gmatrix M;
    int n;

    if ((argc < 3) OR (argc > 4)) {
        printnew("istd <matrix> <computation> [# components, default = 0]\n");
        return;
    }
    GET_MOD(M, 1);
    if (argc IS 3)
        n = 0;
    else {
        n = getInt(argv[3]);
        if (n < 0) n = -1;
    }
    box = istd(argv[2], M, n);
    doAutocalc(argv[2]);
}
