// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "shared.h"
#include "qring.h"
#include "stash.h"
#include "mem.h" // std_stash

#include "monideal.h"
#include "poly.h"
#include "term.h"
#include "standard.h"
#include "ring.h"
#include "vars.h"
#include "generic.h"
#include "plist.h"
#include "monitor.h"
#include "gm_poly.h"  // compshift, set_comp
#include "gmatrix.h"  // gmInsert
#include "cmds.h"     // do_ring_vars, vget_mod, vget_ring
#include "rmap.h"     // imap, change_vars
#include "stdcmds.h"  // stdFirst, stdNext, stdWarning
#include "hilb.h"     // mn_rdiv, monnewhead, monreset, monsearch, moninsert, monrefund
#include "human_io.h" // p_pprint

// Inline functions to replace macros
static inline term INITIAL(poly f)
{
    return (term)(f->monom);
}

static inline field LEAD_COEF(poly g)
{
    return g->coef;
}

static inline ring VAR_RING(variable *v)
{
    return (ring)(v->value);
}

static inline void dl_insert(dlist *dl, int i)
{
    dlist_insert(dl, i);
}

// Constants - MAINVAR comes from shared.h, VRING and VMODULE come from generic.h
// numvars comes from ring.h (also in shared.h)
// divnode comes from standard.h
// current_ring comes from shared.h
// std_stash comes from mem.h

// Inline functions for command argument handling
static inline bool get_mod(gmatrix *m, int argc, char **argv, int i)
{
    if (i >= argc)
        return false;
    *m = vget_mod(argv[i]);
    return (*m != NULL);
}

static inline bool get_vring(variable **v, int argc, char **argv, int i)
{
    if (i >= argc)
        return false;
    *v = (variable *)vget_ring(argv[i]);
    return (*v != NULL);
}

static inline bool new_vmod(variable **p, int argc, char **argv, int i)
{
    if (i >= argc)
        return false;
    *p = make_var("", 0, VMODULE, NULL);
    return (*p != NULL);
}

arrow Rideal; // ideal which is being "modded out"

void qrgInsert(arrow head, mn_standard *rstd, poly f)
{
    expterm exp;
    mn_standard q;

    q = (mn_standard)(void *)get_slug((struct stash *)std_stash);
    q->standard = f;
    q->change = NULL;
    q->next = *rstd;
    if (f != NULL)
        q->degree = tm_degree(INITIAL(f));
    else
        q->degree = 0;
    (*rstd) = q;

    sToExp(INITIAL(f), exp);
    monreset(head, FOW);
    monsearch(head, FOW, exp);
    moninsert(head, exp);
    head->umh.mloc->uld.ldc.ca = (union all *)q;
}

void qrgAddIdeal(arrow head, mn_standard *rstd, gmatrix M)
{
    modgen mg;
    poly f, g;

    stdFirst(M, &mg, USESTD);
    while ((f = stdNext(&mg)) != NULL)
    {
        f = p_copy(f);
        // f must be monic if it is to be in a std basis
        make1_monic(&f);
        for (g = f; g != NULL; g = g->next)
            set_comp(g, 0);
        qrgInsert(head, rstd, f);
    }
}

void qrgAddMonoms(arrow head, mn_standard *rstd, arrow rideal)
{
    arrow p;
    poly f;

    if (rideal == NULL)
        return;
    p = rideal;
    while ((p = monnext(p)) != NULL)
    {
        f = ((mn_standard)p->uld.ldc.ca)->standard;
        f = p_copy(f);
        qrgInsert(head, rstd, f);
    }
}

// this routine takes every poly of "rideal" (over ring R), maps it to ring
// S, and inserts it into (head, rstd).  It is assumed that rideal remains
// a standard basis over the ring S.

void qrgToRing(arrow head, mn_standard *rstd, arrow rideal, variable *R, variable *S)
{
    arrow p;
    gmatrix map;
    poly f;

    if (rideal == NULL)
        return;
    map = imap(R, S, 0);
    p = rideal;
    while ((p = monnext(p)) != NULL)
    {
        f = change_vars(map, R, S, ((mn_standard)p->uld.ldc.ca)->standard);
        vrg_install(S); // just in case state has been left funny
        qrgInsert(head, rstd, f);
    }
    mod_kill(map);
}

// this routine takes the Rideal's of R1, and R2, and puts them into S.

ring qrgSum(variable *R1, variable *R2, variable *S)
{
    ring r1, r2, s, result;
    arrow head;
    mn_standard rstd;

    vrg_install(S);
    head = monnewhead(numvars);
    rstd = NULL;

    r1 = VAR_RING(R1);
    r2 = VAR_RING(R2);
    s = VAR_RING(S);

    qrgToRing(head, &rstd, r1->Rideal, R1, S);
    qrgToRing(head, &rstd, r2->Rideal, R2, S);

    result = (ring)(void *)gimmy(sizeof(struct ringrec));
    *result = *s;

    result->Rideal = head;
    result->Rstd = rstd;
    return result;
}

ring qrgMake(gmatrix M)
{
    ring result;
    arrow head;
    mn_standard rstd;

    result = (ring)(void *)gimmy(sizeof(struct ringrec));

    *result = *VAR_RING(current_ring);

    head = monnewhead(numvars);
    rstd = NULL;
    stdWarning(M);
    qrgAddIdeal(head, &rstd, M);
    qrgAddMonoms(head, &rstd, Rideal);
    result->Rideal = head;
    result->Rstd = rstd;
    return result;
}

boolean isQRing(variable *p)
{
    ring R;

    R = VAR_RING(p);
    if (R == NULL)
        return 0;
    return R->Rideal != NULL;
}

void qrgInstall(arrow r) // actually R->Rideal, some R
{
    Rideal = r;
}

void qrgKill(arrow r, mn_standard rstd)
{
    monrefund(r);
    std_kill(rstd);
}

void qrgDisplay(FILE *fil, ring R)
{
    mn_standard p;

    fnewline(fil);
    fprint(fil, "quotient ring by ideal:\n");
    p = (mn_standard)R->Rstd;
    while (p != NULL)
    {
        fnewline(fil);
        p_pprint(fil, p->standard, 0);
        fprint(fil, "\n");
        p = p->next;
    }
}

// This reduction routine should be called in:
// parsePoly (therefore done in getPoly)
// p_mult
// p_xjei
// p_dot
// p_homog

// Possibly there are other places where reduction should occur.
// Notice that reduce and division (standard.c) already to this.

void qrgReduce(poly *f)
{
    poly inresult;
    mn_standard i;
    allocterm t;
    poly g, h;

    if (Rideal == NULL)
        return;
    g = *f;
    inresult = divnode;
    while (g != NULL)
    {
        if (mn_rdiv(INITIAL(g), (char **)&i, t))
        {
            special_sub(&g, LEAD_COEF(g), t, i->standard);
        }
        else
        {
            inresult->next = g;
            inresult = g;
            g = g->next;
        }
    }
    inresult->next = NULL;
    *f = divnode->next;
}

void qring_cmd(int argc, char *argv[])
{
    gmatrix M;
    variable *p;

    if (argc != 3)
    {
        printnew("qring <ideal> <new ring>\n");
        return;
    }
    if (!get_mod(&M, argc, argv, 1))
        return;
    // Note: current_ring is ring type in iring.h, but make_var expects variable*
    // In the original code, current_ring was variable* from vars.c
    // This appears to be a naming conflict in the codebase
    p = make_var(argv[2], MAINVAR, VRING, NULL);
    if (p == NULL)
        return;
    set_value(p, qrgMake(M));
    vrg_install(p);
    do_ring_vars();
}

gmatrix qrgPresent(ring R, mn_standard rstd)
{
    (void)R; // unused parameter
             // the ring R should be installed

    mn_standard p;
    poly f;
    gmatrix result;

    result = mod_init();
    dl_insert(&result->degrees, 0);
    p = rstd;
    while (p != NULL)
    {
        f = compshift(p->standard, 1); // want row # 1
        gmInsert(result, f);
        p = p->next;
    }
    return result;
}

void presentring_cmd(int argc, char *argv[])
{
    variable *R, *p;

    if (argc != 3)
    {
        printnew("present_ring <quotient ring> <result ideal>\n");
        return;
    }
    if (!get_vring(&R, argc, argv, 1))
        return;
    if (R->b_ring == NULL)
    {
        if (!new_vmod(&p, argc, argv, 2))
            return;
        set_value(p, qrgPresent(VAR_RING(R), NULL));
    }
    else
    {
        vrg_install(R->b_ring);
        if (!new_vmod(&p, argc, argv, 2))
            return;
        set_value(p, qrgPresent(VAR_RING(R->b_ring), VAR_RING(R)->Rstd));
    }
}
