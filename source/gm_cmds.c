// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <string.h>
#include <setjmp.h>

#include "shared.h"
#include "gm_cmds.h"
#include "vars.h"
#include "monitor.h"
#include "gmatrix.h"
#include "plist.h"
#include "gm_dlist.h"
#include "parse.h"
#include "rvar.h"
#include "parsepoly.h"
#include "stdcmds.h"
#include "ring.h"
#include "cmds.h"

// Constants
constexpr int IDSIZE = 50;
// MAINVAR comes from shared.h
constexpr int VMODULE = 4; // Should come from generic.h

// GET/NEW macros inline functions
static inline bool get_mod(gmatrix *g, int argc, char **argv, int i)
{
    if (i >= argc)
        return false;
    *g = vget_mod(argv[i]);
    return (*g != NULL);
}

static inline bool get_rgmod(gmatrix *g, int argc, char **argv, int i)
{
    if (i >= argc)
        return false;
    *g = vget_rgmod(argv[i]);
    return (*g != NULL);
}

static inline bool new_mod(variable **p, int argc, char **argv, int i)
{
    if (i >= argc)
        return false;
    *p = make_var(argv[i], MAINVAR, VMODULE, current_ring);
    return (*p != NULL);
}

static inline bool new_savmod(variable **p, char *s)
{
    *p = make_var(s, MAINVAR, VMODULE, current_ring);
    return (*p != NULL);
}

void add_cmd(int argc, char *argv[])
{
    gmatrix f, g;
    variable *p;

    if (argc != 4)
    {
        printnew("add <matrix> <matrix> <result>\n");
        return;
    }
    if (!get_mod(&f, argc, argv, 1))
        return;
    if (!get_rgmod(&g, argc, argv, 2))
        return;
    if (!new_mod(&p, argc, argv, 3))
        return;
    set_value(p, gm_add(f, g));
}

void mult_cmd(int argc, char *argv[])
{
    gmatrix f, g;
    variable *p;

    if (argc != 4)
    {
        printnew("mult <matrix> <matrix> <result>\n");
        return;
    }
    if (!get_mod(&f, argc, argv, 1))
        return;
    if (!get_rgmod(&g, argc, argv, 2))
        return;
    if (!new_mod(&p, argc, argv, 3))
        return;
    set_value(p, gm_mult(f, g));
}

void smult_cmd(int argc, char *argv[])
{
    gmatrix M;
    poly f;
    variable *p;

    if (argc != 4)
    {
        printnew("smult <matrix> <polynomial> <result matrix>\n");
        return;
    }
    if (!get_mod(&M, argc, argv, 1))
        return;
    f = getPoly(argv[2], 1);
    if (!new_mod(&p, argc, argv, 3))
        return;
    set_value(p, gm_smult(M, f));
}

void sub_cmd(int argc, char *argv[])
{
    gmatrix f, g;
    variable *p;

    if (argc != 4)
    {
        printnew("subtract <matrix> <matrix> <result>\n");
        return;
    }
    if (!get_mod(&f, argc, argv, 1))
        return;
    if (!get_rgmod(&g, argc, argv, 2))
        return;
    if (!new_mod(&p, argc, argv, 3))
        return;
    set_value(p, gm_sub(f, g));
}

void trans_cmd(int argc, char *argv[])
{
    gmatrix f;
    variable *p;

    if (argc != 3)
    {
        printnew("transpose <matrix> <result matrix>\n");
        return;
    }
    if (!get_mod(&f, argc, argv, 1))
        return;
    if (!new_mod(&p, argc, argv, 2))
        return;
    set_value(p, gm_transpose(f));
}

void dsum_cmd(int argc, char *argv[])
{
    gmatrix result, M;
    int i;
    variable *p;

    if (argc < 3)
    {
        printnew("dsum <matrix> ... <matrix> <result>\n");
        return;
    }

    if (!get_mod(&M, argc, argv, 1))
        return;
    // first, check that rings match up:
    for (i = 2; i < argc - 1; i++)
        if (!get_rgmod(&M, argc, argv, i))
            return;

    result = mod_init();
    for (i = 1; i < argc - 1; i++)
    {
        if (!get_rgmod(&M, argc, argv, i))
            return;
        gm_dsum(result, M);
    }
    if (!new_mod(&p, argc, argv, argc - 1))
        return;
    set_value(p, result);
}

void iden_cmd(int argc, char *argv[])
{
    int n;
    variable *p;

    if (argc != 3)
    {
        printnew("iden <size> <result>\n");
        return;
    }
    if (current_ring == NULL)
    {
        prerror("; no current ring is defined\n");
        return;
    }
    n = getInt(argv[1]);
    if (n < 0)
    {
        prerror("; size must be non-negative\n");
        return;
    }
    if (!new_mod(&p, argc, argv, 2))
        return;
    set_value(p, gm_iden(n));
}

void diag_cmd(int argc, char *argv[])
{
    gmatrix M;
    variable *p;

    if (argc != 3)
    {
        printnew("diag <matrix> <result>\n");
        return;
    }
    if (!get_mod(&M, argc, argv, 1))
        return;
    if (!new_mod(&p, argc, argv, 2))
        return;
    set_value(p, gm_diag(M));
}

void submat_cmd(int argc, char *argv[])
{
    gmatrix g;
    variable *p;
    dlist drows, dcols;
    char s[IDSIZE];

    if (argc != 3)
    {
        printnew("submat <matrix> <result submatrix>\n");
        return;
    }
    if (!get_mod(&g, argc, argv, 1))
        return;
    strcpy(s, argv[2]);
    prinput("rows");
    if (!dlVarGet(&drows, 1, nrows(g)))
        return;
    prinput("columns");
    if (!dlVarGet(&dcols, 1, ncols(g)))
        return;
    if (!new_savmod(&p, s))
        return;
    set_value(p, gm_submat(g, &drows, &dcols));
    dl_kill(&drows);
    dl_kill(&dcols);
}

void wedge_cmd(int argc, char *argv[])
{
    gmatrix g;
    int n;
    variable *p;

    if (argc != 4)
    {
        printnew("wedge <matrix> <p> <result>\n");
        return;
    }
    if (!get_mod(&g, argc, argv, 1))
        return;
    n = getInt(argv[2]);
    if ((n <= 0) || (n > nrows(g)) || (n > ncols(g)))
    {
        prerror("; all size %d minors are zero\n", n);
        return;
    }
    if (!new_mod(&p, argc, argv, 3))
        return;
    set_value(p, gm_wedge(g, n));
}

void pfaff_cmd(int argc, char *argv[])
{
    gmatrix g;
    variable *p;

    if (argc != 3)
    {
        printnew("pfaff <skew matrix> <result ideal of 4 by 4 pfaffians>\n");
        return;
    }
    if (!get_mod(&g, argc, argv, 1))
        return;
    if (!new_mod(&p, argc, argv, 2))
        return;
    set_value(p, gm_pfaff4(g));
}

void flatten_cmd(int argc, char *argv[])
{
    gmatrix g;
    variable *p;

    if (argc != 3)
    {
        printnew("flatten <matrix> <result ideal of entries>\n");
        return;
    }
    if (!get_mod(&g, argc, argv, 1))
        return;
    if (!new_mod(&p, argc, argv, 2))
        return;
    set_value(p, gm_flatten(g));
}

void cat_cmd(int argc, char *argv[])
{
    variable *p;
    dlist drows, dcols;
    int firstvar;
    char s[IDSIZE];

    if (argc != 3)
    {
        printnew("cat <first var(0..numvars-1)> <result matrix>\n");
        return;
    }
    strcpy(s, argv[2]);
    firstvar = getVar(argv[1]);
    if (firstvar < 0) // then try seeing if this is an integer
        firstvar = getInt(argv[1]);

    prinput("rows");
    dlVarScan(&drows, 0, numvars - 1);
    prinput("columns");
    dlVarScan(&dcols, 0, numvars - 1);
    if (!new_savmod(&p, s))
        return;
    set_value(p, gm_genMat(&drows, &dcols, firstvar));
}

void jacob_cmd(int argc, char *argv[])
{
    variable *p;
    gmatrix g;
    expterm vars;

    if (argc < 3)
    {
        printnew("jacob <ideal> <resulting jacobian> [variable list]\n");
        return;
    }
    if (!get_mod(&g, argc, argv, 1))
        return;
    if (!new_mod(&p, argc, argv, 2))
        return;
    const char **const_argv = (const char **)(void *)(argv + 3);
    if (!(getVarList(argc - 3, const_argv, vars)))
        return;
    set_value(p, gm_jacobian(g, vars));
}

void concat_cmd(int argc, char *argv[])
{
    gmatrix result, g;
    int i;

    if (argc <= 2)
    {
        printnew("concat <matrix> <matrix2> ... <matrix n>\n");
        return;
    }
    if (!get_mod(&result, argc, argv, 1))
        return;
    for (i = 2; i < argc; i++)
    {
        if (!get_rgmod(&g, argc, argv, i))
            return;
        gm_concat(result, g);
    }
}

void reduce_cmd(int argc, char *argv[])
{
    gmatrix f, g;
    gmatrix red, lift;
    variable *p, *q;

    if ((argc < 4) || (argc > 5))
    {
        printnew("reduce <standard basis> <matrix to reduce> ");
        print("<result reduced matrix> [result lifted matrix]\n");
        return;
    }
    if (!get_mod(&f, argc, argv, 1))
        return;
    if (!get_rgmod(&g, argc, argv, 2))
        return;
    if (nrows(f) < nrows(g))
    {
        prerror("; %s has too many rows\n", argv[2]);
        return;
    }
    if (!new_mod(&p, argc, argv, 3))
        return;
    if (argc == 5)
    {
        if (!new_mod(&q, argc, argv, 4))
            return;
        gm_reduce(f, g, &red, &lift);
        set_value(q, lift);
    }
    else
        gm_reduce(f, g, &red, NULL);
    set_value(p, red);
}

void lift_cmd(int argc, char *argv[])
{
    gmatrix f, g;
    variable *p;

    if (argc != 4)
    {
        printnew("lift <standard basis> <matrix to lift> <result>\n");
        return;
    }
    if (!get_mod(&f, argc, argv, 1))
        return;
    if (!get_rgmod(&g, argc, argv, 2))
        return;
    if (!new_mod(&p, argc, argv, 3))
        return;
    set_value(p, gm_lift(f, g));
}

void trace_cmd(int argc, char *argv[])
{
    gmatrix M;
    variable *p;

    if (argc != 3)
    {
        printnew("trace <matrix> <resulting trace>\n");
        return;
    }
    if (!get_mod(&M, argc, argv, 1))
        return;
    if (!new_mod(&p, argc, argv, 2))
        return;
    set_value(p, gm_trace(M));
}

void tensor_cmd(int argc, char *argv[])
{
    gmatrix M, N;
    variable *p;

    if (argc != 4)
    {
        printnew("tensor <matrix M> <matrix N> <result matrix M.N>\n");
        return;
    }
    if (!get_mod(&M, argc, argv, 1))
        return;
    if (!get_rgmod(&N, argc, argv, 2))
        return;
    if (!new_mod(&p, argc, argv, 3))
        return;
    set_value(p, gm_tensor(M, N));
}

void outer_cmd(int argc, char *argv[])
{
    gmatrix M, N;
    variable *p;

    if (argc != 4)
    {
        printnew("outer <matrix> <matrix> <result matrix>\n");
        return;
    }
    if (!get_mod(&M, argc, argv, 1))
        return;
    if (!get_rgmod(&N, argc, argv, 2))
        return;
    if (!new_mod(&p, argc, argv, 3))
        return;
    set_value(p, gm_outer(M, N));
}

void power_cmd(int argc, char *argv[])
{
    gmatrix M;
    int n;
    variable *p;

    if (argc != 4)
    {
        printnew("power <matrix 1 by n> <int power> <result matrix>\n");
        return;
    }
    if (!get_mod(&M, argc, argv, 1))
        return;
    if (nrows(M) != 1)
    {
        prerror("; there must be only one row in the given matrix\n");
        return;
    }
    n = getInt(argv[2]);
    if (n < 0)
    {
        prerror("; power must be non-negative\n");
        return;
    }
    if (!new_mod(&p, argc, argv, 3))
        return;
    set_value(p, gm_power(M, n));
}

void diff_cmd(int argc, char *argv[])
{
    gmatrix M, N;
    variable *p;
    boolean usecoef;

    if (argc != 4)
    {
        printnew("%s <ideal> <ideal> <result matrix>\n", argv[0]);
        return;
    }
    if (!get_mod(&M, argc, argv, 1))
        return;
    if (!get_mod(&N, argc, argv, 2))
        return;
    if (!new_mod(&p, argc, argv, 3))
        return;
    usecoef = (argv[0][0] == 'd'); /* simple way to choose between
                                       "diff" and "contract" */
    set_value(p, gm_diff(M, N, usecoef));
}

void random_cmd(int argc, char *argv[])
{
    int r, c;
    variable *p;

    if (argc != 4)
    {
        printnew("random <nrows> <ncolumns> <result random matrix>\n");
        return;
    }
    r = getInt(argv[1]);
    c = getInt(argv[2]);
    if ((r < 0) || (c < 0))
    {
        prerror("; need non-negative number of rows and columns\n");
        return;
    }
    if (!new_mod(&p, argc, argv, 3))
        return;
    set_value(p, gm_random(r, c));
}

void compress_cmd(int argc, char *argv[])
{
    gmatrix M;
    variable *p;

    if (argc != 3)
    {
        printnew("compress <matrix> <result matrix>\n");
        return;
    }
    if (!get_mod(&M, argc, argv, 1))
        return;
    if (!new_mod(&p, argc, argv, 2))
        return;
    set_value(p, gm_compress(M));
}
