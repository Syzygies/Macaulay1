// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <stdio.h>
#include <string.h>
#include "shared.h"
#include "spcmds.h"
#include "array.h"   // For array_length()
#include "cmds.h"    // For vget_mod(), vget_ring()
#include "vars.h"    // For make_var(), set_value()
#include "generic.h" // For VMODULE constant
#include "gm_poly.h" // For p_copy()
#include "gmatrix.h" // For gmInsert()
#include "input.h"   // For get_line()
#include "monitor.h" // For printnew(), prerror()
#include "poly.h"    // For p_monom(), p_add(), p1_mult()
#include "qring.h"   // For qrgReduce()
#include "ring.h"    // For nvars(), varName()
#include "term.h"    // For sToExp(), expToS(), tm_component()
#include "plist.h"   // For dl_copy(), mod_init(), degree()
#include <setjmp.h>  // For jmp_buf in rvar.h
#include "rvar.h"    // For getVar()
#include "stdcmds.h" // For getVarList()
#include "charp.h"   // For fd_one

// Constants - MAINVAR comes from vars.h, VMODULE from generic.h

// coef cmd

boolean moneq(expterm exp, term t, expterm vars) // if return true, sets exponent of each
// var in "vars", of "t" to 0
// if vars[i] != 0, "i" is in "vars"
{
    int i;
    expterm exp2;

    sToExp(t, exp2);
    for (i = 0; i < numvars; i++)
    {
        if (vars[i] == 0)
            continue;
        if (exp[i] != exp2[i])
            return (false);
        else
            exp2[i] = 0;
    }
    expToS(exp2, tm_component(t), t);
    return (true);
}

poly strip_poly(poly *f, expterm vars, poly *hmonom)
{
    int i;
    poly result, temp;
    expterm exp;
    poly inresult;
    // Use a dummy poly struct for list head trick
    struct pol dummy_head;

    if (*f == NULL)
        return (NULL);
    inresult = &dummy_head;
    inresult->next = *f;
    sToExp(poly_initial(*f), exp);
    result = NULL;

    for (i = 0; i < numvars; i++)
        if (vars[i] == 0)
            exp[i] = 0;
    *hmonom = p_monom(fd_one);
    expToS(exp, 1, poly_initial(*hmonom));

    while (inresult->next != NULL)
        if (moneq(exp, poly_initial(inresult->next), vars))
        {
            temp = inresult->next;
            inresult->next = temp->next;
            temp->next = NULL;
            p_add(&result, &temp);
        }
        else
            inresult = inresult->next;
    *f = dummy_head.next;
    return (result);
}

void gm_coef(gmatrix M, expterm vars, gmatrix *Mmonoms, gmatrix *Mcoefs)
{
    poly f, g, hmonom;
    int j;

    *Mmonoms = mod_init();
    dlist_insert(&(*Mmonoms)->degrees, 0);
    *Mcoefs = mod_init();
    dl_copy(&(M->degrees), &((*Mcoefs)->degrees));
    for (j = 1; j <= length(&(M->gens)); j++)
    {
        f = p_copy(plist_ref(&M->gens, j));
        while (f != NULL)
        {
            g = strip_poly(&f, vars, &hmonom);
            gmInsert(*Mmonoms, hmonom);
            gmInsert(*Mcoefs, g);
        }
    }
}

void coef_cmd(int argc, char *argv[])
{
    gmatrix M;
    gmatrix Mmonoms, Mcoefs;
    variable *p, *q;
    expterm vars;

    if (argc <= 3)
    {
        printnew("coef <matrix> <result monoms> <result coefs> [variable list]\n");
        return;
    }
    if ((M = vget_mod(argv[1])) == NULL)
        return;
    p = make_var(argv[2], MAINVAR, VMODULE, current_ring);
    if (p == NULL)
        return;
    q = make_var(argv[3], MAINVAR, VMODULE, current_ring);
    if (q == NULL)
        return;
    if (!getVarList(argc - 4, (const char **)(void *)(argv + 4), vars))
        return;
    gm_coef(M, vars, &Mmonoms, &Mcoefs);
    set_value(p, Mmonoms);
    set_value(q, Mcoefs);
}

poly p_homog(gmatrix M, poly f, int v)
{
    poly g, h, temp, result;
    int d, maxd, a;
    expterm exp;
    allocterm t;

    if (f == NULL)
        return (NULL);
    g = f;
    maxd = degree(M, g);
    while (g != NULL)
    {
        d = degree(M, g);
        if (d > maxd)
            maxd = d;
        g = g->next;
    }

    g = f;
    result = NULL;
    while (g != NULL)
    {
        d = maxd - degree(M, g);
        // the following should be in poly.c or term.c
        for (a = 0; a < numvars; a++)
            exp[a] = 0;
        exp[v] = d;
        expToS(exp, 1, t);
        temp = g;
        g = g->next;
        temp->next = NULL;
        h = p1_mult(fd_one, t, temp);
        temp->next = g;
        p_add(&result, &h);
    }
    qrgReduce(&result);
    return (result);
}

gmatrix gm_homog(gmatrix M, int v) // use this var as homog. variable
{
    gmatrix N; // result
    int j;
    poly f, g;

    N = mod_init();
    dl_copy(&(M->degrees), &(N->degrees));
    for (j = 1; j <= length(&(M->gens)); j++)
    {
        f = plist_ref(&M->gens, j);
        if (f == NULL)
            continue;
        g = p_homog(M, f, v);
        gmInsert(N, g);
        dlist_insert(&(N->deggens), degree(N, g));
    }
    return (N);
}

void homog_cmd(int argc, char *argv[])
{
    gmatrix M;
    variable *p;
    int v;

    if (argc != 4)
    {
        printnew("homog <matrix> <homog variable> <new matrix>\n");
        return;
    }
    if ((M = vget_mod(argv[1])) == NULL)
        return;
    v = getVar(argv[2]);
    if (v == -1)
    {
        prerror("; var %s not defined\n", argv[2]);
        return;
    }
    p = make_var(argv[3], MAINVAR, VMODULE, current_ring);
    if (p == NULL)
        return;
    set_value(p, gm_homog(M, v));
}

// intersections, ideal quotients, and tensor products

// intersect_cmd(argc, argv)
// int argc ;
// char *argv[] ;
// {
// if (argc < 3) {
// printnew("intersect <matrix 1> ... <matrix n> <computation>\n") ;
// return ;
// }
// GET_MOD(M, 1) ;
// r = nrows(M) ;
// mat = gm_intmat(M) ;
// for (i=2; i<argc-1; i++) {
// if ((M = vget_mod(argv[i])) == NULL)
// return;
// if (r != nrows(M)) {
// prerror("; matrix #%d has wrong number of rows\n", i) ;
// mod_kill(M) ;
// return ;
// }
// gm_int2(mat, M) ;
// }

// }
