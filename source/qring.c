/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "style.h"
#include "iring.h"
#include "vars.h"
#include "ddefs.h"

// void qrgInsert (arrow head, mn_standard *rstd, poly f);
// void qrgAddIdeal (arrow head, mn_standard *rstd, gmatrix M);
// void qrgAddMonoms (arrow head, mn_standard *rstd, arrow rideal);
// void qrgToRing (arrow head, mn_standard *rstd, arrow rideal, variable *R, variable *S);
// ring qrgSum (variable *R1, variable *R2, variable *S);
// ring qrgMake (gmatrix M);
// boolean isQRing (variable *p);
// void qrgInstall (arrow r);
// void qrgKill (arrow r, mn_standard rstd);
// void qrgDisplay (FILE *fil, ring R);
// void qrgReduce (poly *f);
// void qring_cmd (int argc, char *argv[]);
// gmatrix qrgPresent (ring R, mn_standard rstd);
// void presentring_cmd (int argc, char *argv[]);

arrow Rideal;  /* ideal which is being "modded out" */

extern arrow monnewhead();

void qrgInsert (arrow head, mn_standard *rstd, poly f)
{
    expterm exp;
    mn_standard q;

    q = (mn_standard) get_slug(std_stash);
    q->standard = f;
    q->change = NULL;
    q->next = *rstd;
    if (f ISNT NULL)
        q->degree = tm_degree(INITIAL(f));
    else
        q->degree = 0;
    (*rstd) = q;

    sToExp(INITIAL(f), exp);
    monreset(head, FOW);
    monsearch(head, FOW, exp);
    moninsert(head, exp);
    head->umh.mloc->uld.ldc.ci = (char *) q;
}

void qrgAddIdeal (arrow head, mn_standard *rstd, gmatrix M)
{
    modgen mg;
    poly f,g;

    stdFirst(M, &mg, USESTD);
    while ((f=stdNext(&mg)) ISNT NULL) {
        f = p_copy(f);
        /* f must be monic if it is to be in a std basis */
        make1_monic(&f);
        for (g=f; g ISNT NULL; g = g->next)
            set_comp(g, 0);
        qrgInsert(head, rstd, f);
    }
}

void qrgAddMonoms (arrow head, mn_standard *rstd, arrow rideal)
{
    arrow p;
    poly f;

    if (rideal IS NULL) return;
    p = rideal;
    while ((p = monnext(p)) ISNT NULL) {
        f = ((mn_standard) p->uld.ldc.ci)->standard;
        f = p_copy(f);
        qrgInsert(head, rstd, f);
    }
}

/* this routine takes every poly of "rideal" (over ring R), maps it to ring
   S, and inserts it into (head, rstd).  It is assumed that rideal remains
   a standard basis over the ring S.
 */

void qrgToRing (arrow head, mn_standard *rstd, arrow rideal, variable *R, variable *S)
{
    arrow p;
    gmatrix map;
    poly f;
    gmatrix imap();
    poly change_vars();

    if (rideal IS NULL) return;
    map = imap(R, S, FALSE);
    p = rideal;
    while ((p = monnext(p)) ISNT NULL) {
        f = change_vars(map, R, S, ((mn_standard) p->uld.ldc.ci)->standard);
        vrg_install(S);  /* just in case state has been left funny */
        qrgInsert(head, rstd, f);
    }
    mod_kill(map);
}

/* this routine takes the Rideal's of R1, and R2, and puts them into S. */

ring qrgSum (variable *R1, variable *R2, variable *S)
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

    result = (ring) gimmy(sizeof(struct ringrec));
    *result = *s;

    result->Rideal = (char *) head;
    result->Rstd   = (char *) rstd;
    return(result);
}

ring qrgMake (gmatrix M)
{
    ring result;
    arrow head;
    mn_standard rstd;

    result = (ring) gimmy(sizeof(struct ringrec));

    *result = *VAR_RING(current_ring);

    head = monnewhead(numvars);
    rstd = NULL;
    stdWarning(M);
    qrgAddIdeal(head, &rstd, M);
    qrgAddMonoms(head, &rstd, Rideal);
    result->Rideal = (char *) head;
    result->Rstd = (char *) rstd;
    return(result);
}

boolean isQRing (variable *p)
{
    ring R;

    R = VAR_RING(p);
    if (R IS NULL) return(FALSE);
    return(R->Rideal ISNT NULL);
}

void qrgInstall (arrow r)
/* actually R->Rideal, some R */
{
    Rideal = r;
}

void qrgKill (arrow r, mn_standard rstd)
{
    monrefund(r);
    std_kill(rstd);
}

void qrgDisplay (FILE *fil, ring R)
{
    mn_standard p;

    fnewline(fil);
    fprint(fil, "quotient ring by ideal:\n");
    p = (mn_standard) R->Rstd;
    while (p ISNT NULL) {
        fnewline(fil);
        p_pprint(fil, p->standard, 0);
        fprint(fil, "\n");
        p = p->next;
    }
}

/*--------------------------------------------------
 * This reduction routine should be called in:
 *  parsePoly (therefore done in getPoly)
 *  p_mult
 *  p_xjei
 *  p_dot
 *  p_homog
 *
 *  Possibly there are other places where reduction should occur.
 *  Notice that reduce and division (standard.c) already to this.
 *--------------------------------------------------*/

void qrgReduce (poly *f)
{
    poly inresult;
    mn_standard i;
    allocterm t;
    extern poly divnode;
    poly g;

    if (Rideal IS NULL) return;
    g = *f;
    inresult = divnode;
    while (g ISNT NULL) {
        if (mn_rdiv(INITIAL(g), &i, t))
            special_sub(&g, LEAD_COEF(g), t, i->standard);
        else {
            inresult->next = g;
            inresult = g;
            g = g->next;
        }
    }
    inresult->next = NULL;
    *f = divnode->next;
}

void qring_cmd (int argc, char *argv[])
{
    gmatrix M;
    variable *p;

    if (argc ISNT 3) {
        printnew("qring <ideal> <new ring>\n");
        return;
    }
    GET_MOD(M, 1);
    p = make_var(argv[2], MAINVAR, VRING, current_ring);
    if (p IS NULL) return;
    set_value(p, qrgMake(M));
    vrg_install(p);
    do_ring_vars();
}

gmatrix qrgPresent (ring R, mn_standard rstd)
{
#pragma unused(R)
    /* the ring R should be installed */

    mn_standard p;
    poly f;
    gmatrix result;
    poly compshift();

    result = mod_init();
    dl_insert(&result->degrees, 0);
    p = rstd;
    while (p ISNT NULL) {
        f = compshift(p->standard, 1);  /* want row # 1 */
        gmInsert(result, f);
        p = p->next;
    }
    return(result);
}

void presentring_cmd (int argc, char *argv[])
{
    variable *R, *p;

    if (argc ISNT 3) {
        printnew("present_ring <quotient ring> <result ideal>\n");
        return;
    }
    GET_VRING(R, 1);
    if (R->b_ring IS NULL) {
        NEW_MOD(p, 2);
        set_value(p, qrgPresent(VAR_RING(R), NULL));
    } else {
        vrg_install(R->b_ring);
        NEW_MOD(p, 2);
        set_value(p, qrgPresent(VAR_RING(R->b_ring), VAR_RING(R)->Rstd));
    }
}
