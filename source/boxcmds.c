/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"

// void coll_cmd (int argc, char *argv[]);
// void trash_cmd (int argc, char *argv[]);
// void mknres_cmd (int argc, char *argv[]);
// void mkstd_cmd (int argc, char *argv[]);
// void mkistd_cmd (int argc, char *argv[]);
// void mkres_cmd (int argc, char *argv[]);
// void emit_cmd (int argc, char *argv[]);
// void stdemit_cmd (int argc, char *argv[]);
// void mklift_cmd (int argc, char *argv[]);
// void shift_cmd (int argc, char *argv[]);
// void merge_cmd (int argc, char *argv[]);
// void start0_cmd (int argc, char *argv[]);
// void conn_cmd (int argc, char *argv[]);
// void chcalc_cmd (int argc, char *argv[]);
// void calc_cmd (int argc, char *argv[]);

void coll_cmd (int argc, char *argv[])
{
    if (argc IS 1) mkcollect("");
    else mkcollect(argv[1]);
}

void trash_cmd (int argc, char *argv[])
{
    if (argc IS 1) mktrash("");
    else mktrash(argv[1]);
}

void mknres_cmd (int argc, char *argv[])
{
    int n, ch;
    int last_ch, first;

    if (argc ISNT 5) {
        printnew("mknres <# to make> <# in change> <# in last change>");
        print(" [int. name of first nres]\n");
        return;
    }
    n = getInt(argv[1]);
    ch = getInt(argv[2]);
    last_ch = getInt(argv[3]);
    first = getInt(argv[4]);
    if (n <= 0) {
        prerror("; not creating any nres vars\n");
        return;
    }
    if (first < 0) {
        prerror("; first name must be non-negative integer\n");
        return;
    }
    if (last_ch < 0) last_ch = -1;
    if (ch < 0) ch = -1;
    mk_sev_nres(first, n, ch, last_ch);
}

void mkstd_cmd (int argc, char *argv[])
{
    char *name;

    if (argc > 2) {
        printnew("mkstd [name]\n");
        return;
    }
    name = (argc IS 2 ? argv[1] : "");
    mkstd(name);
}

void mkistd_cmd (int argc, char *argv[])
{
    char *name;
    gmatrix M;
    int intval;

    if ((argc < 3) OR (argc > 4)) {
        printnew("mkistd <# in change> <matrix> [name]\n");
        return;
    }
    name = (argc IS 4 ? argv[3] : "");
    intval = getInt(argv[1]);
    GET_MOD(M, 3);
    mkistd(name, intval, M);
}

void mkres_cmd (int argc, char *argv[])
{
    int ch;
    gmatrix M;
    char *name;

    if ((argc < 3) OR (argc > 4)) {
        printnew("mkres <# in change> <module> [name]\n");
        return;
    }
    ch = getInt(argv[1]);
    GET_MOD(M, 2);
    name = (argc IS 4 ? argv[3] : "");
    mkres(name, ch, M);
}

void emit_cmd (int argc, char *argv[])
{
    variable *p;
    char *name;

    if ((argc < 2) OR (argc > 3)) {
        printnew("mkemit <module> [name]\n");
        return;
    }
    GET_VMOD(p, 1);
    name = (argc IS 3 ? argv[2] : "");
    mkemit(name, p);
}

void stdemit_cmd (int argc, char *argv[])
{
    variable *p;
    char *name;

    if ((argc < 2) OR (argc > 3)) {
        printnew("mkstdemit <module> [name]\n");
        return;
    }
    GET_VMOD(p, 1);
    name = (argc IS 3 ? argv[2] : "");
    mkstdemit(name,p);
}

void mklift_cmd (int argc, char *argv[])
{
    variable *p;
    char *name;

    if ((argc < 2) OR (argc > 3)) {
        printnew("mklift <module> [name]\n");
        return;
    }
    GET_VMOD(p, 1);
    name = (argc IS 3 ? argv[2] : "");
    mklift(name, p);
}

void shift_cmd (int argc, char *argv[])
{
    int n;
    char *name;

    if ((argc < 2) OR (argc > 3)) {
        printnew("mkshift <shiftval> [name]\n");
        return;
    }
    name = (argc IS 3 ? argv[2] : "");
    n = getInt(argv[1]);
    mkshift(name, n);
}

void merge_cmd (int argc, char *argv[])
{
    char *name;

    name = (argc IS 2 ? argv[1] : "");
    mkmerge(name);
}

void start0_cmd (int argc, char *argv[])
{
    int numstart, maxgen, lastdeg;

    if (argc ISNT 5) {
        printnew("mkstart0 <name> <# to start> <maxgen> <lastdeg done>\n");
        return;
    }
    numstart = getInt(argv[2]);
    maxgen = getInt(argv[3]);
    lastdeg = getInt(argv[4]);
    mkstarter(argv[1], maxgen, lastdeg, numstart);
}

void conn_cmd (int argc, char *argv[])
{
    variable *invar, *outvar;

    if ((argc < 2) OR (argc > 3)) {
        printnew("connect <in var> [out var]\n");
        return;
    }
    invar = find_var(argv[1]);
    if (invar IS NULL) return;
    if (argc IS 2) outvar = NULL;
    else outvar = find_var(argv[2]);
    invar->b_next = outvar;
}

void chcalc_cmd (int argc, char *argv[])
{
    start_rec *s;
    variable *p;

    if ((argc < 2) OR (argc > 3)) {
        printnew("chcalc <computation> [new hi deg]\n");
        return;
    }
    p = find_var(argv[1]);
    if (p IS NULL) return;
    if (p->type ISNT VSTARTER) {
        prerror("; variable is not a computation\n");
        return;
    }
    s = (start_rec *) p->value;
    if ((s->hideg IS (s->lastdeg - 1)) AND (NOT s->doalldegs)) {
        prerror("; computation is not in progress\n");
        return;
    }
    st_bounds(p, argc, argv);
}

void calc_cmd (int argc, char *argv[])
{
    variable *p;

    if ((argc < 2) OR (argc > 3)) {
        printnew("calc <computation> [hi degree]\n");
        return;
    }
    if ((p = find_var(argv[1])) IS NULL)  return;
    if (p->type ISNT VSTARTER) {
        prerror("; variable is not a computation\n");
        return;
    }
    start(p, argc, argv);
}
