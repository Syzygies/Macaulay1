/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"

// void add_cmd (int argc, char *argv[]);
// void mult_cmd (int argc, char *argv[]);
// void smult_cmd (int argc, char *argv[]);
// void sub_cmd (int argc, char *argv[]);
// void trans_cmd (int argc, char *argv[]);
// void dsum_cmd (int argc, char *argv[]);
// void iden_cmd (int argc, char *argv[]);
// void diag_cmd (int argc, char *argv[]);
// void submat_cmd (int argc, char *argv[]);
// void wedge_cmd (int argc, char *argv[]);
// void pfaff_cmd (int argc, char *argv[]);
// void flatten_cmd (int argc, char *argv[]);
// void cat_cmd (int argc, char *argv[]);
// void jacob_cmd (int argc, char *argv[]);
// void concat_cmd (int argc, char *argv[]);
// void reduce_cmd (int argc, char *argv[]);
// void lift_cmd (int argc, char *argv[]);
// void trace_cmd (int argc, char *argv[]);
// void tensor_cmd (int argc, char *argv[]);
// void outer_cmd (int argc, char *argv[]);
// void power_cmd (int argc, char *argv[]);
// void diff_cmd (int argc, char *argv[]);
// void random_cmd (int argc, char *argv[]);
// void compress_cmd (int argc, char *argv[]);

extern gmatrix gm_add();
extern gmatrix gm_sub();
extern gmatrix gm_mult();
extern gmatrix gm_smult();
extern gmatrix gm_transpose();
extern gmatrix gm_diag();
extern gmatrix gm_submat();
extern gmatrix gm_wedge();
extern gmatrix gm_pfaff4();
extern gmatrix gm_flatten();
extern gmatrix gm_genMat();
extern gmatrix gm_lift();
extern gmatrix gm_jacobian();
extern gmatrix gm_trace();
extern gmatrix gm_tensor();
extern gmatrix gm_outer();
extern gmatrix gm_diff();
extern gmatrix gm_power();
extern gmatrix gm_random();
extern gmatrix gm_compress();

void add_cmd (int argc, char *argv[])
{
    gmatrix f, g;
    variable *p;

    if (argc ISNT 4) {
        printnew("add <matrix> <matrix> <result>\n");
        return;
    }
    GET_MOD(f, 1);
    GET_rgMOD(g, 2);
    NEW_MOD(p, 3);
    set_value(p, gm_add(f, g));
}

void mult_cmd (int argc, char *argv[])
{
    gmatrix f, g;
    variable *p;

    if (argc ISNT 4) {
        printnew("mult <matrix> <matrix> <result>\n");
        return;
    }
    GET_MOD(f, 1);
    GET_rgMOD(g, 2);
    NEW_MOD(p, 3);
    set_value(p, gm_mult(f, g));
}

void smult_cmd (int argc, char *argv[])
{
    gmatrix M;
    poly f;
    variable *p;

    if (argc ISNT 4) {
        printnew("smult <matrix> <polynomial> <result matrix>\n");
        return;
    }
    GET_MOD(M, 1);
    f = getPoly(argv[2], 1);
    NEW_MOD(p, 3);
    set_value(p, gm_smult(M, f));
}

void sub_cmd (int argc, char *argv[])
{
    gmatrix f, g;
    variable *p;

    if (argc ISNT 4) {
        printnew("subtract <matrix> <matrix> <result>\n");
        return;
    }
    GET_MOD(f, 1);
    GET_rgMOD(g, 2);
    NEW_MOD(p, 3);
    set_value(p, gm_sub(f, g));
}

void trans_cmd (int argc, char *argv[])
{
    gmatrix f;
    variable *p;

    if (argc ISNT 3) {
        printnew("transpose <matrix> <result matrix>\n");
        return;
    }
    GET_MOD(f, 1);
    NEW_MOD(p, 2);
    set_value(p, gm_transpose(f));
}

void dsum_cmd (int argc, char *argv[])
{
    gmatrix result, M;
    int i;
    variable *p;

    if (argc < 3) {
        printnew("dsum <matrix> ... <matrix> <result>\n");
        return;
    }

    GET_MOD(M, 1);
    /* first, check that rings match up: */
    for (i=2; i<argc-1; i++)
        GET_rgMOD(M, i);

    result = mod_init();
    for (i=1; i<argc-1; i++) {
        GET_rgMOD(M, i);
        gm_dsum(result, M);
    }
    NEW_MOD(p, argc-1);
    set_value(p, result);
}

void iden_cmd (int argc, char *argv[])
{
    int n;
    variable *p;

    if (argc ISNT 3) {
        printnew("iden <size> <result>\n");
        return;
    }
    if (current_ring IS NULL) {
        prerror("; no current ring is defined\n");
        return;
    }
    n = getInt(argv[1]);
    if (n < 0) {
        prerror("; size must be non-negative\n");
        return;
    }
    NEW_MOD(p, 2);
    set_value(p, gm_iden(n));
}

void diag_cmd (int argc, char *argv[])
{
    gmatrix M;
    variable *p;

    if (argc ISNT 3) {
        printnew("diag <matrix> <result>\n");
        return;
    }
    GET_MOD(M, 1);
    NEW_MOD(p, 2);
    set_value(p, gm_diag(M));
}

void submat_cmd (int argc, char *argv[])
{
    gmatrix g;
    variable *p;
    dlist drows, dcols;
    char s[IDSIZE];

    if (argc ISNT 3) {
        printnew("submat <matrix> <result submatrix>\n");
        return;
    }
    GET_MOD(g, 1);
    strcpy(s, argv[2]);
    prinput("rows");
    if (NOT dlVarGet(&drows, 1, nrows(g))) return;
    prinput("columns");
    if (NOT dlVarGet(&dcols, 1, ncols(g))) return;
    NEW_savMOD(p, s);
    set_value(p, gm_submat(g, &drows, &dcols));
    dl_kill(&drows);
    dl_kill(&dcols);
}

void wedge_cmd (int argc, char *argv[])
{
    gmatrix g;
    int n;
    variable *p;

    if (argc ISNT 4) {
        printnew("wedge <matrix> <p> <result>\n");
        return;
    }
    GET_MOD(g, 1);
    n = getInt(argv[2]);
    if ((n <= 0) OR (n > nrows(g)) OR (n > ncols(g))) {
        prerror("; all size %d minors are zero\n", n);
        return;
    }
    NEW_MOD(p, 3);
    set_value(p, gm_wedge(g, n));
}

void pfaff_cmd (int argc, char *argv[])
{
    gmatrix g;
    variable *p;

    if (argc ISNT 3) {
        printnew("pfaff <skew matrix> <result ideal of 4 by 4 pfaffians>\n");
        return;
    }
    GET_MOD(g, 1);
    NEW_MOD(p, 2);
    set_value(p, gm_pfaff4(g));
}

void flatten_cmd (int argc, char *argv[])
{
    gmatrix g;
    variable *p;

    if (argc ISNT 3) {
        printnew("flatten <matrix> <result ideal of entries>\n");
        return;
    }
    GET_MOD(g, 1);
    NEW_MOD(p, 2);
    set_value(p, gm_flatten(g));
}

void cat_cmd (int argc, char *argv[])
{
    variable *p;
    dlist drows, dcols;
    int firstvar;
    char s[IDSIZE];

    if (argc ISNT 3) {
        printnew("cat <first var(0..numvars-1)> <result matrix>\n");
        return;
    }
    strcpy(s, argv[2]);
    firstvar = getVar(argv[1]);
    if (firstvar < 0)  /* then try seeing if this is an integer */
        firstvar = getInt(argv[1]);

    prinput("rows");
    dlVarScan(&drows, 0, numvars-1);
    prinput("columns");
    dlVarScan(&dcols, 0, numvars-1);
    NEW_savMOD(p, s);
    set_value(p, gm_genMat(&drows, &dcols, firstvar));
}

void jacob_cmd (int argc, char *argv[])
{
    variable *p;
    gmatrix g;
    expterm vars;

    if (argc < 3) {
        printnew("jacob <ideal> <resulting jacobian> [variable list]\n");
        return;
    }
    GET_MOD(g, 1);
    NEW_MOD(p, 2);
    if (NOT(getVarList(argc-3, argv+3, vars))) return;
    set_value(p, gm_jacobian(g, vars));
}

void concat_cmd (int argc, char *argv[])
{
    gmatrix result, g;
    int i;

    if (argc <= 2) {
        printnew("concat <matrix> <matrix2> ... <matrix n>\n");
        return;
    }
    GET_MOD(result, 1);
    for (i=2; i<argc; i++) {
        GET_rgMOD(g, i);
        gm_concat(result, g);
    }
}

void reduce_cmd (int argc, char *argv[])
{
    gmatrix f, g;
    gmatrix red, lift;
    variable *p, *q;

    if ((argc < 4) OR (argc > 5)) {
        printnew("reduce <standard basis> <matrix to reduce> ");
        print("<result reduced matrix> [result lifted matrix]\n");
        return;
    }
    GET_MOD(f, 1);
    GET_rgMOD(g, 2);
    if (nrows(f) < nrows(g)) {
        prerror("; %s has too many rows\n", argv[2]);
        return;
    }
    NEW_MOD(p, 3);
    if (argc IS 5) {
        NEW_MOD(q, 4);
        gm_reduce(f,g,&red, &lift);
        set_value(q,lift);
    } else
        gm_reduce(f,g,&red,NULL);
    set_value(p, red);
}

void lift_cmd (int argc, char *argv[])
{
    gmatrix f, g;
    variable *p;

    if (argc ISNT 4) {
        printnew("lift <standard basis> <matrix to lift> <result>\n");
        return;
    }
    GET_MOD(f, 1);
    GET_rgMOD(g, 2);
    NEW_MOD(p, 3);
    set_value(p, gm_lift(f, g));
}

void trace_cmd (int argc, char *argv[])
{
    gmatrix M;
    variable *p;

    if (argc ISNT 3) {
        printnew("trace <matrix> <resulting trace>\n");
        return;
    }
    GET_MOD(M, 1);
    NEW_MOD(p, 2);
    set_value(p, gm_trace(M));
}

void tensor_cmd (int argc, char *argv[])
{
    gmatrix M, N;
    variable *p;

    if (argc ISNT 4) {
        printnew("tensor <matrix M> <matrix N> <result matrix M.N>\n");
        return;
    }
    GET_MOD(M, 1);
    GET_rgMOD(N, 2);
    NEW_MOD(p, 3);
    set_value(p, gm_tensor(M,N));
}

void outer_cmd (int argc, char *argv[])
{
    gmatrix M, N;
    variable *p;

    if (argc ISNT 4) {
        printnew("outer <matrix> <matrix> <result matrix>\n");
        return;
    }
    GET_MOD(M, 1);
    GET_rgMOD(N, 2);
    NEW_MOD(p, 3);
    set_value(p, gm_outer(M,N));
}

void power_cmd (int argc, char *argv[])
{
    gmatrix M;
    int n;
    variable *p;

    if (argc ISNT 4) {
        printnew("power <matrix 1 by n> <int power> <result matrix>\n");
        return;
    }
    GET_MOD(M, 1);
    if (nrows(M) ISNT 1) {
        prerror("; there must be only one row in the given matrix\n");
        return;
    }
    n = getInt(argv[2]);
    if (n < 0) {
        prerror("; power must be non-negative\n");
        return;
    }
    NEW_MOD(p, 3);
    set_value(p, gm_power(M, n));
}

void diff_cmd (int argc, char *argv[])
{
    gmatrix M, N;
    variable *p;
    boolean usecoef;

    if (argc ISNT 4) {
        printnew("%s <ideal> <ideal> <result matrix>\n", argv[0]);
        return;
    }
    GET_MOD(M, 1);
    GET_MOD(N, 2);
    NEW_MOD(p, 3);
    usecoef = (argv[0][0] == 'd'); /* simple way to choose between
                        "diff" and "contract" */
    set_value(p, gm_diff(M, N, usecoef));
}

void random_cmd (int argc, char *argv[])
{
    int r,c;
    variable *p;

    if (argc ISNT 4) {
        printnew("random <nrows> <ncolumns> <result random matrix>\n");
        return;
    }
    r = getInt(argv[1]);
    c = getInt(argv[2]);
    if ((r < 0) OR (c < 0)) {
        prerror("; need non-negative number of rows and columns\n");
        return;
    }
    NEW_MOD(p, 3);
    set_value(p, gm_random(r,c));
}

void compress_cmd (int argc, char *argv[])
{
    gmatrix M;
    variable *p;

    if (argc ISNT 3) {
        printnew("compress <matrix> <result matrix>\n");
        return;
    }
    GET_MOD(M, 1);
    NEW_MOD(p, 2);
    set_value(p, gm_compress(M));
}
