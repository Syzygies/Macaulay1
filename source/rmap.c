// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <stdio.h>
#include <string.h>
#include "shared.h"
#include "rmap.h"
#include "array.h"     // For ref(), array access
#include "charp.h"     // For fd_mult(), normalize()
#include "cmds.h"      // For vget_mod(), vget_ring()
#include "plist.h"     // For dl_copy(), mod_init(), mod_kill()
#include "gm_poly.h"   // For p_xjei(), p_kill(), p_add(), p_mult()
#include "human_io.h"  // For p_pprint()
#include "gmatrix.h"   // For gmInsert(), ncols()
#include "input.h"     // For prinput(), get_line()
#include "parsepoly.h" // For rdPoly(), rdPolyStr()
#include "monitor.h"   // For prerror(), printnew(), fnewline(), fprint()
#include "poly.h"      // For e_sub_i()
#include "qring.h"     // For qrgReduce()
#include "ring.h"      // For vrg_install(), nvars(), varName(), getRingVar()
#include "shell.h"     // For outfile
#include "term.h"      // For sToExp(), expToS(), get_comp()
#include "vars.h"      // For make_var(), set_value()
#include "generic.h"   // For VMODULE constant
#include "gm_dlist.h"  // For dl_new()
#include "rvar.h"      // For getVar()

// VAR_RING inline function - gets ring from variable
static inline ring var_ring(variable *v)
{
    return (ring)(v)->value;
}

// Constants - MAINVAR comes from vars.h, VMODULE from generic.h

// IDSIZE constant
constexpr int IDSIZE = 20;

void rmap_kill(void)
{
}

// Evaluates the monomial "f" via a map of polynomial rings
// using the ideal "rmap" (over ring "S"), which defines a ring map
// from R ---> S, by taking i th variable of R to the i th column
// of "rmap".  If "rmap" has too few columns, it is (conceptually) padded
// with zeros.

// Assumptions: ring(rmap) = S
// ring(f) = R.

poly change1_vars(gmatrix rmap, variable *R, variable *S, poly f)
{
    field fcoef, rcoef;
    int fcomp, top, i, j, a;
    poly g, rpoly, rp, temp;
    expterm fexp, rexp, exp;

    // first grab information from "f"

    vrg_install(R);
    fcoef = f->coef;
    sToExp(poly_initial(f), fexp);
    fcomp = get_comp(f);
    top = min_int(ncols(rmap), numvars);
    for (i = top; i < numvars; i++)
        if (fexp[i] > 0)
        {
            vrg_install(S); // on exit, ring S must be current ring
            return (NULL);
        }

    // now set up ring we will work in, and rcoef,rexp,rpoly

    vrg_install(S);
    rcoef = normalize(fcoef); // in case characteristic has changed
    rpoly = e_sub_i(fcomp);
    for (i = 0; i < numvars; i++)
        rexp[i] = 0;

    // now loop through and set up result

    for (i = 0; i < top; i++)
    {
        if ((a = fexp[i]) == 0)
            continue;
        g = plist_ref(&rmap->gens, i + 1);
        if (g == NULL)
        {
            p_kill(&rpoly);
            return (NULL);
        }
        else if (g->next == NULL)
        {
            sToExp(poly_initial(g), exp);
            for (j = 1; j <= a; j++)
                fd_mult(rcoef, g->coef, &rcoef);
            for (j = 0; j < numvars; j++)
                rexp[j] += a * exp[j];
        }
        else
        {
            // else g has at least two terms
            for (j = 1; j <= a; j++)
            {
                temp = p_mult(g, rpoly);
                p_kill(&rpoly);
                rpoly = temp;
            }
        }
        if ((rcoef == 0) || (rpoly == NULL))
        {
            p_kill(&rpoly);
            return (NULL);
        }
    }

    // finally, piece together rcoef, rexp, and rpoly

    rp = rpoly;
    while (rp != NULL)
    {
        fd_mult(rcoef, rp->coef, &rp->coef);
        sToExp(poly_initial(rp), exp);
        for (j = 0; j < numvars; j++)
            exp[j] += rexp[j];
        expToS(exp, fcomp, poly_initial(rp));
        rp = rp->next;
    }
    qrgReduce(&rpoly); // in case we are in a quotient ring
    return (rpoly);
}

poly change_vars(gmatrix rmap, variable *R, variable *S, poly f)
{
    poly result, temp;

    result = NULL;
    while (f != NULL)
    {
        temp = change1_vars(rmap, R, S, f);
        p_add(&result, &temp);
        f = f->next;
    }
    return (result);
}

gmatrix mat_apply(gmatrix rmap, variable *R, variable *S, gmatrix M)
{
    register int i;
    register poly f;
    register gmatrix result;

    vrg_install(S);
    result = mod_init(); // in ring S
    dl_copy(&M->degrees, &result->degrees);
    for (i = 1; i <= ncols(M); i++)
    {
        f = change_vars(rmap, R, S, plist_ref(&M->gens, i));
        gmInsert(result, f);
    }
    return (result);
}

gmatrix rmap_scan(variable *R, variable *S)
{
    gmatrix result;
    int i, n;
    ring RR;

    RR = var_ring(R);
    n = nvars(RR);

    vrg_install(S);
    result = mod_init();
    dl_new(&result->degrees, 1);
    for (i = 0; i < n; i++)
    {
        prinput("%s --->", varName(RR, i));
        gmInsert(result, rdPoly(1));
    }
    return (result);
}

void rmap_pprint(gmatrix rmap, variable *R, variable *S)
{
    ring RR;
    int i, n;

    fnewline(outfile);
    fprint(outfile, "map : %s ---> %s\n", R->name, S->name);
    vrg_install(S);
    RR = var_ring(R);
    n = nvars(RR);
    for (i = 0; i < n; i++)
    {
        fnewline(outfile);
        fprint(outfile, "%s |--> ", varName(RR, i));
        if (i >= ncols(rmap))
            fprint(outfile, "0");
        else
            p_pprint(outfile, plist_ref(&rmap->gens, i + 1), 1);
        fprint(outfile, "\n");
    }
}

gmatrix imap(variable *R, variable *S, boolean ones)
{
    ring RR;
    gmatrix result;
    int v, w, n;

    vrg_install(S);
    RR = var_ring(R);
    n = nvars(RR);
    result = mod_init();
    dl_new(&result->degrees, 1);
    for (v = 0; v < n; v++)
    {
        w = getVar(varName(RR, v));
        if (w == -1)
        {
            if (ones)
                gmInsert(result, e_sub_i(1));
            else
                gmInsert(result, NULL);
        }
        else
            gmInsert(result, p_xjei(w, 1));
    }
    return (result);
}

void edit_rmap(gmatrix rmap, variable *R, variable *S)
{
    int argc, v;
    char **argv;
    poly f;

    vrg_install(S);
    while (true)
    {
        prinput("variable, new image");
        get_line(&argc, &argv);
        if (argc < 2)
            break;
        v = 1 + getRingVar(var_ring(R), argv[0]);
        if (v == 0)
        {
            prerror("; variable %s not defined\n", argv[0]);
            continue;
        }
        f = rdPolyStr(argc - 1, argv + 1, 1);
        while (v > ncols(rmap)) // in case ideal "rmap" not long enough
            gmInsert(rmap, NULL);
        poly *p = plist_ref_ptr(&rmap->gens, v);
        p_kill(p);
        *p = f;
    }
}

void rmap_cmd(int argc, char *argv[])
{
    variable *R, *S, *p;
    char s[IDSIZE];
    gmatrix M;

    if (argc != 4)
    {
        printnew("rmap <result> <R> <S>\n");
        return;
    }
    if ((R = vget_ring(argv[2])) == NULL)
        return;
    if ((S = vget_ring(argv[3])) == NULL)
        return;
    strcpy(s, argv[1]);
    M = rmap_scan(R, S);
    p = make_var(s, MAINVAR, VMODULE, S);
    if (p == NULL)
        return;
    set_value(p, M);
}

void imap_cmd(int argc, char *argv[])
{
    variable *R, *S;
    variable *p;
    gmatrix f;
    char s[IDSIZE];

    if ((argc < 4) || (argc > 5))
    {
        printnew("imap <new ring map> <R> <S> [ones, default=zeros]\n");
        return;
    }
    if ((R = vget_ring(argv[2])) == NULL)
        return;
    if ((S = vget_ring(argv[3])) == NULL)
        return;
    strcpy(s, argv[1]);
    f = imap(R, S, (argc == 5));
    edit_rmap(f, R, S);
    p = make_var(s, MAINVAR, VMODULE, S);
    if (p == NULL)
        return;
    set_value(p, f);
}

void ev_cmd(int argc, char *argv[])
{
    gmatrix M, rmap;
    variable *p, *R, *S;

    if (argc != 4)
    {
        printnew("ev <ideal> <matrix> <changed matrix>\n");
        return;
    }

    // Get matrix M and its ring
    if ((M = vget_mod(argv[2])) == NULL)
        return;
    R = current_ring ? vget_ring("") : NULL; // Ring of M
    if (!R)
        return;

    // Get rmap and its ring
    if ((rmap = vget_mod(argv[1])) == NULL)
        return;
    S = current_ring ? vget_ring("") : NULL; // Ring of rmap
    if (!S)
        return;

    // Create result in ring S
    if (((p) = make_var(argv[3], MAINVAR, VMODULE, S)) == NULL)
        return;
    set_value(p, mat_apply(rmap, R, S, M));
}

// toring_cmd(argc, argv)
// int argc ;
// char *argv[] ;
// {
// variable *S, *R, *p ;
// gmatrix g, M ;
// char s[IDSIZE] ;

// if ((argc < 3) OR (argc > 4)) {
// printnew("to-ring <matrix> <result matrix> [ones, default=zeros]\n");
// return ;
// }
// GET_CRING(S) ;
// GET_MOD(M, 1) ;
// GET_CRING(R) ;
// vrg_install(S) ;
// strcpy(s, argv[2]) ;
// g = imap(R, S, (argc == 4)) ;
// edit_rmap(g, R, S) ;
// NEW_savMOD(p, s) ;
// set_value(p, mat_apply(g, R, S, M)) ;
// mod_kill(g) ;
// }

void fetch_cmd(int argc, char *argv[])
{
    variable *S, *R, *p;
    gmatrix g, M;
    char s[IDSIZE];

    if ((argc < 3) || (argc > 4))
    {
        printnew("fetch <matrix> <result matrix> [ones, default=zeros]\n");
        return;
    }

    // Save current ring as S
    S = current_ring ? vget_ring("") : NULL;
    if (!S)
        return;

    // Get matrix M and its ring
    if ((M = vget_mod(argv[1])) == NULL)
        return;
    R = current_ring ? vget_ring("") : NULL; // Ring of M
    if (!R)
        return;

    // Switch back to original ring S
    vrg_install(S);
    strcpy(s, argv[2]);
    g = imap(R, S, (argc == 4));

    // Create result in ring S
    if (((p) = make_var(s, MAINVAR, VMODULE, S)) == NULL)
        return;
    set_value(p, mat_apply(g, R, S, M));
    mod_kill(g);
}

void modrmap_cmd(int argc, char *argv[])
{
    variable *R, *S;
    gmatrix g;

    if ((argc < 2) || (argc > 3))
    {
        printnew("edit_map <ideal> [source ring]\n");
        return;
    }

    if (argc == 2)
    {
        R = current_ring ? vget_ring("") : NULL;
        if (!R)
            return;
    }
    else if ((R = vget_ring(argv[2])) == NULL)
        return;

    if ((g = vget_mod(argv[1])) == NULL)
        return;
    S = current_ring ? vget_ring("") : NULL; // Ring of g
    if (!S)
        return;

    edit_rmap(g, R, S);
}

void pmap_cmd(int argc, char *argv[])
{
    variable *R, *S;
    gmatrix g;

    if ((argc < 2) || (argc > 3))
    {
        printnew("pmap <ideal> [source ring]\n");
        return;
    }
    if (argc == 2)
    {
        R = current_ring ? vget_ring("") : NULL;
        if (!R)
            return;
    }
    else if ((R = vget_ring(argv[2])) == NULL)
        return;

    if ((g = vget_mod(argv[1])) == NULL)
        return;
    S = current_ring ? vget_ring("") : NULL; // Ring of g
    if (!S)
        return;

    rmap_pprint(g, R, S);
}
