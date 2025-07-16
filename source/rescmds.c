// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "rescmds.h"
#include <setjmp.h>    // for jmp_buf type used in parse.h
#include "cmds.h"      // vget_mod, vget_vmod, vget_rgmod, printnew
#include "parse.h"     // getInt
#include "human_io.h"  // check_homog
#include "monitor.h"   // prerror
#include "plist.h"     // dl_lohi
#include "gmatrix.h"   // mat_copy, mod_kill, ncols, gmInsert
#include "poly.h"      // p_copy
#include "vars.h"      // find_var, garb_collect, set_alias
#include "boxprocs.h" /* mkstarter, mkshift, mkres, mk_sev_nres, mkemit, mknres, mkstd, mkistd,
                         mktrash, mkcollect, calc */
#include "set.h"   // autocalc, autodegree
#include "ring.h"  // numvars

// Inline functions to replace GET_* macros
static inline bool get_mod(gmatrix* g, char** argv, int i)
{
    *g = vget_mod(argv[i]);
    return (*g != NULL);
}

static inline bool get_vmod(variable** p, char** argv, int i)
{
    *p = vget_vmod(argv[i]);
    return (*p != NULL);
}

static inline bool get_rgmod(gmatrix* g, char** argv, int i)
{
    *g = vget_rgmod(argv[i]);
    return (*g != NULL);
}

boolean hom_check(gmatrix M, char* name)
{
    if (!check_homog(M))
    {
        prerror("; matrix %s isn't homogeneous; either homogenize it\n", name);
        prerror("; using 'homog', or change degrees using 'setdegs'\n");
        return false;
    }
    return true;
}

void doAutocalc(char* s)
{
    variable* box;

    garb_collect();
    box = find_var(s);

    if (autocalc < 0)
        calc(box, true, 0);
    else if (autocalc > 0)
        calc(box, false, autodegree);
}

variable*res(char* name, gmatrix M, int depth)
{
    variable* result;
    int lo, hi;

    dl_lohi(&M->deggens, &lo, &hi);
    result = mkstarter(name, hi - 1, lo - 2, 1);
    mkshift("", 1); // so calc w n gives n-linear strand
    mkres("1", -1, M);
    mk_sev_nres(2, depth - 1, -1, 0);
    return result;
}

variable*nres(char* name, variable* p, int depth)
{
    variable* result;
    int lo, hi;
    gmatrix M;

    M = var_get_module(p);
    dl_lohi(&M->deggens, &lo, &hi);
    result = mkstarter(name, hi, lo - 1, 1);
    mkemit("", p);
    mk_sev_nres(1, depth, -1, 0);
    return result;
}

variable*std(char* name, variable* p)
{
    variable* result, * alias;
    int lo, hi;
    gmatrix M;

    M = var_get_module(p);
    dl_lohi(&M->deggens, &lo, &hi);
    result = mkstarter(name, hi, lo - 1, 1);
    mkemit("", p);
    alias = mkstd("1");
    set_alias(result, alias);
    return result;
}

variable*lift_std(char* name, gmatrix M)
{
    variable* result, * alias;
    int lo, hi;

    dl_lohi(&M->deggens, &lo, &hi);
    result = mkstarter(name, hi, lo - 1, 1);
    alias = mkres("1", -1, M);
    mktrash("");
    set_alias(result, alias);
    return result;
}

variable*syz(char* name, gmatrix M, int ncomps)
{
    variable* result, * alias;
    int lo, hi;

    dl_lohi(&M->deggens, &lo, &hi);
    result = mkstarter(name, hi, lo - 1, 1);
    mkres("0", ncomps, M);
    alias = mknres("1", 0);
    set_alias(result, alias);
    return result;
}

variable*modulo(char* name, gmatrix M, gmatrix N)
{
    gmatrix g;
    int i;
    variable* p;

    g = mat_copy(M);
    for (i = 1; i <= ncols(N); i++)
        gmInsert(g, p_copy(plist_ref(&N->gens, i)));
    if (!check_homog(g))
    {
        prerror("; concatenating both matrices produces a matrix which is not\n");
        prerror("; homogeneous.  Either homogenize, or change degrees using setdegs\n");
        mod_kill(g);
        return nullptr;
    }
    p = syz(name, g, ncols(M));
    mod_kill(g);
    return p;
}

variable*istd(char* name, gmatrix M, int ncomps)
{
    variable* result, * alias;

    result = mkstarter(name, -1, -1, 1);
    alias = mkistd("1", ncomps, M);
    if (ncomps != 0)
        mkcollect("2");
    set_alias(result, alias);
    return result;
}

void res_cmd(int argc, char* argv[])
{
    gmatrix M;
    int maxdepth;

    if ((argc < 3) || (argc > 4))
    {
        printnew("res <module> <computation> [max depth]\n");
        return;
    }
    if (!get_mod(&M, argv, 1))
        return;
    if (argc >= 4)
        maxdepth = getInt(argv[3]);
    else
        maxdepth = max_int(numvars, 2);

    if (!hom_check(M, argv[1]))
        return;
    res(argv[2], M, maxdepth);
    doAutocalc(argv[2]);
}

void nres_cmd(int argc, char* argv[])
{
    variable* p;
    gmatrix M;
    int maxdepth;

    if ((argc < 3) || (argc > 4))
    {
        printnew("nres <module> <computation> [max depth]\n");
        return;
    }
    if (!get_vmod(&p, argv, 1))
        return;
    M = var_get_module(p);
    if (argc >= 4)
        maxdepth = getInt(argv[3]);
    else
        maxdepth = numvars;

    if (!hom_check(M, argv[1]))
        return;
    nres(argv[2], p, maxdepth);
    doAutocalc(argv[2]);
}

void std_cmd(int argc, char* argv[])
{
    variable* p;
    gmatrix M;

    if (argc != 3)
    {
        printnew("std <module> <computation>\n");
        return;
    }
    if (!get_vmod(&p, argv, 1))
        return;
    M = var_get_module(p);
    if (M == nullptr)
        return;
    if (!hom_check(M, argv[1]))
        return;
    std(argv[2], p);
    doAutocalc(argv[2]);
}

void liftstd_cmd(int argc, char* argv[])
{
    gmatrix M;

    if (argc != 3)
    {
        printnew("lift_std <matrix> <computation>\n");
        return;
    }
    if (!get_mod(&M, argv, 1))
        return;
    if (!hom_check(M, argv[1]))
        return;
    lift_std(argv[2], M);
    doAutocalc(argv[2]);
}

void syz_cmd(int argc, char* argv[])
{
    gmatrix M;
    int n;

    if (argc != 4)
    {
        printnew("syz <module> <# components(-1=all,0,...)> <computation>\n");
        return;
    }
    if (!get_mod(&M, argv, 1))
        return;
    n = getInt(argv[2]);
    if (n < 0)
        n = -1;
    if (!hom_check(M, argv[1]))
        return;
    syz(argv[3], M, n);
    doAutocalc(argv[3]);
}

void modulo_cmd(int argc, char* argv[])
{
    variable* box;
    gmatrix M, N;

    if (argc != 4)
    {
        printnew("modulo <matrix> <matrix> <computation>\n");
        return;
    }
    if (!get_mod(&M, argv, 1))
        return;
    if (!get_rgmod(&N, argv, 2))
        return;
    box = modulo(argv[3], M, N);
    if (box == nullptr)
        return;
    doAutocalc(argv[3]);
}

void istd_cmd(int argc, char* argv[])
{
    gmatrix M;
    int n;

    if ((argc < 3) || (argc > 4))
    {
        printnew("istd <matrix> <computation> [# components, default = 0]\n");
        return;
    }
    if (!get_mod(&M, argv, 1))
        return;
    if (argc == 3)
        n = 0;
    else
    {
        n = getInt(argv[3]);
        if (n < 0)
            n = -1;
    }
    istd(argv[2], M, n);
    doAutocalc(argv[2]);
}
