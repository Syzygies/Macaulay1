// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "boxcmds.h"
#include "vars.h"      // find_var()
#include "cmds.h"      // vget_mod(), vget_vmod()
#include "boxprocs.h"  // All mk* functions and start functions
#include "monitor.h"   // print(), printnew(), prerror()
#include "parse.h"     // getInt()
#include "generic.h"   // Variable type constants including VSTARTER

// Inline functions to replace GET_MOD and GET_VMOD macros
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

int coll_cmd(int argc, char* argv[])
{
    if (argc == 1)
        mkcollect("");
    else
        mkcollect(argv[1]);
    return 0;
}

int trash_cmd(int argc, char* argv[])
{
    if (argc == 1)
        mktrash("");
    else
        mktrash(argv[1]);
    return 0;
}

int mknres_cmd(int argc, char* argv[])
{
    int n, ch;
    int last_ch, first;

    if (argc != 5)
    {
        printnew("mknres <# to make> <# in change> <# in last change>");
        print(" [int. name of first nres]\n");
        return 0;
    }
    n = getInt(argv[1]);
    ch = getInt(argv[2]);
    last_ch = getInt(argv[3]);
    first = getInt(argv[4]);
    if (n <= 0)
    {
        prerror("; not creating any nres vars\n");
        return 0;
    }
    if (first < 0)
    {
        prerror("; first name must be non-negative integer\n");
        return 0;
    }
    if (last_ch < 0)
        last_ch = -1;
    if (ch < 0)
        ch = -1;
    mk_sev_nres(first, n, ch, last_ch);
    return 0;
}

int mkstd_cmd(int argc, char* argv[])
{
    const char* name;

    if (argc > 2)
    {
        printnew("mkstd [name]\n");
        return 0;
    }
    name = (argc == 2 ? argv[1] : "");
    mkstd(name);
    return 0;
}

int mkistd_cmd(int argc, char* argv[])
{
    const char* name;
    gmatrix M;
    int intval;

    if ((argc < 3) || (argc > 4))
    {
        printnew("mkistd <# in change> <matrix> [name]\n");
        return 0;
    }
    name = (argc == 4 ? argv[3] : "");
    intval = getInt(argv[1]);
    if (!get_mod(&M, argv, 2))
        return 0;
    mkistd(name, intval, M);
    return 0;
}

int mkres_cmd(int argc, char* argv[])
{
    int ch;
    gmatrix M;
    const char* name;

    if ((argc < 3) || (argc > 4))
    {
        printnew("mkres <# in change> <module> [name]\n");
        return 0;
    }
    ch = getInt(argv[1]);
    if (!get_mod(&M, argv, 2))
        return 0;
    name = (argc == 4 ? argv[3] : "");
    mkres(name, ch, M);
    return 0;
}

int emit_cmd(int argc, char* argv[])
{
    variable* p;
    const char* name;

    if ((argc < 2) || (argc > 3))
    {
        printnew("mkemit <module> [name]\n");
        return 0;
    }
    if (!get_vmod(&p, argv, 1))
        return 0;
    name = (argc == 3 ? argv[2] : "");
    mkemit(name, p);
    return 0;
}

int stdemit_cmd(int argc, char* argv[])
{
    variable* p;
    const char* name;

    if ((argc < 2) || (argc > 3))
    {
        printnew("mkstdemit <module> [name]\n");
        return 0;
    }
    if (!get_vmod(&p, argv, 1))
        return 0;
    name = (argc == 3 ? argv[2] : "");
    mkstdemit(name, p);
    return 0;
}

int mklift_cmd(int argc, char* argv[])
{
    variable* p;
    const char* name;

    if ((argc < 2) || (argc > 3))
    {
        printnew("mklift <module> [name]\n");
        return 0;
    }
    if (!get_vmod(&p, argv, 1))
        return 0;
    name = (argc == 3 ? argv[2] : "");
    mklift(name, p);
    return 0;
}

int shift_cmd(int argc, char* argv[])
{
    int n;
    const char* name;

    if ((argc < 2) || (argc > 3))
    {
        printnew("mkshift <shiftval> [name]\n");
        return 0;
    }
    name = (argc == 3 ? argv[2] : "");
    n = getInt(argv[1]);
    mkshift(name, n);
    return 0;
}

int merge_cmd(int argc, char* argv[])
{
    const char* name;

    name = (argc == 2 ? argv[1] : "");
    mkmerge(name);
    return 0;
}

int start0_cmd(int argc, char* argv[])
{
    int numstart, maxgen, lastdeg;

    if (argc != 5)
    {
        printnew("mkstart0 <name> <# to start> <maxgen> <lastdeg done>\n");
        return 0;
    }
    numstart = getInt(argv[2]);
    maxgen = getInt(argv[3]);
    lastdeg = getInt(argv[4]);
    mkstarter(argv[1], maxgen, lastdeg, numstart);
    return 0;
}

int conn_cmd(int argc, char* argv[])
{
    variable* invar, * outvar;

    if ((argc < 2) || (argc > 3))
    {
        printnew("connect <in var> [out var]\n");
        return 0;
    }
    invar = find_var(argv[1]);
    if (invar == NULL)
        return 0;
    if (argc == 2)
        outvar = NULL;
    else
        outvar = find_var(argv[2]);
    invar->b_next = outvar;
    return 0;
}

int chcalc_cmd(int argc, char* argv[])
{
    start_rec* s;
    variable* p;

    if ((argc < 2) || (argc > 3))
    {
        printnew("chcalc <computation> [new hi deg]\n");
        return 0;
    }
    p = find_var(argv[1]);
    if (p == NULL)
        return 0;
    if (p->type != VSTARTER)
    {
        prerror("; variable is not a computation\n");
        return 0;
    }
    s = (start_rec*)p->value;
    if ((s->hideg == (s->lastdeg - 1)) && (!s->doalldegs))
    {
        prerror("; computation is not in progress\n");
        return 0;
    }
    st_bounds(p, argc, argv);
    return 0;
}

int calc_cmd(int argc, char* argv[])
{
    variable* p;

    if ((argc < 2) || (argc > 3))
    {
        printnew("calc <computation> [hi degree]\n");
        return 0;
    }
    if ((p = find_var(argv[1])) == NULL)
        return 0;
    if (p->type != VSTARTER)
    {
        prerror("; variable is not a computation\n");
        return 0;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#ifdef __clang__
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
#endif
    st_start(p, argc, argv);
#pragma GCC diagnostic pop
    return 0;
}
