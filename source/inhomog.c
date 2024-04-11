/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"
#include "ddefs.h"

// void IHorig_gens (gmatrix M, int intval, variable *B);
// boolean /* always returns TRUE */ IHcalc_standard (gmatrix M, variable *B);
void IHsend_off (gmatrix M, variable *B, poly h, poly hrep);
void IHins_elem (gmatrix M, poly h, poly hrep);
// void IHmn_adjoin (gmatrix M, arrow head, expterm nexp, char *bag);
// void IHmn_lcm (arrow head, arrow *newh, expterm m, arrow mloc);
void IHmo_insert (gmatrix M, term m1, mn_standard bag);
// boolean IHmo_next_pair (gmatrix M, mn_standard *i, mn_standard *j);

extern arrow monnewhead();
extern arrow monadv();
extern arrow Rideal;
extern char *pairStash;

void IHorig_gens (gmatrix M, int intval, variable *B)
{
    int i;
    poly f, k, h, hrep;

    for (i=1; i<=length(&M->gens); i++) {
        f = p_copy(PREF(M->gens, i));
        division(M, &f, &hrep, &h);
        if ((intval < 0) OR (intval >= i)) {
            k = e_sub_i(i);
            p_add(&hrep, &k);
        }
        IHsend_off(M, B, h, hrep);
    }
}

boolean /* always returns TRUE */ IHcalc_standard (gmatrix M, variable *B)
/* output "box" */
{
    mn_standard i, j, p;
    poly h, hrep, f, rep;
    boolean notdone;

    if (ncols(M) > 0)
        prflush(".");
    mo_reset(M, &i, &j);
    do {
        notdone = FALSE;
        while (IHmo_next_pair(M, &i, &j)) {
            calc_s_pair(M, i, j, &h, &hrep);
            IHsend_off(M, B, h, hrep);
            notdone = TRUE;
        }
        while (M->stdbasis ISNT NULL) { /* stdbasis is actually scrap heap */
            p = M->stdbasis;
            f = p->standard;
            hrep = p->change;
            M->stdbasis = p->next;
            free_slug(std_stash, p);
            division(M, &f, &rep, &h);
            p_sub(&hrep, &rep);
            IHsend_off(M, B, h, hrep);
            notdone = TRUE;
        }
    } while (notdone);
    return(TRUE);
}

void IHsend_off (gmatrix M, variable *B, poly h, poly hrep)
{
    if (h ISNT NULL) {
        IHins_elem(M, h, hrep);
        if (verbose > 0) prflush("m");
    } else if (hrep ISNT NULL) {
        send_poly(B, hrep, 0);
        if (verbose > 0) prflush("s");
    } else if (verbose > 0) prflush("o");
    intr_shell();
}

void IHins_elem (gmatrix M, poly h, poly hrep)
{
    mn_standard i;

    make2_monic(&h, &hrep);
    i = (mn_standard) get_slug(std_stash);
    i->standard = h;
    i->change = hrep;
    i->ismin = (char) TRUE;
    i->next = NULL;
    i->degree = 0;
    IHmo_insert(M, INITIAL(h), i);
}

void IHmn_adjoin (gmatrix M, arrow head, expterm nexp, char *bag)
/* put deleted baggage on M->stdbasis */
/* insert this guy */
{
    arrow p, new;
    mn_standard q;

    monreset(head, FOW);
    monsearch(head, FOW, nexp);
    moninsert(head, nexp);
    M->nstandard++;
    M->modtype = MISTD;
    new = head->umh.mloc;
    new->uld.ldc.ci = bag;
    monreset(head, BAK);
    while (TRUE) {
        monsearch(head, BAK, nexp);
        p = head->umh.mloc;
        if (new == p) break;
        else {
            q = (mn_standard) p->uld.ldc.ci;
            q->next = M->stdbasis;
            q->ismin = (char) FALSE;
            M->nstandard--;
            M->stdbasis = q;
            mondelete(head);
        }
    }
}

void IHmn_lcm (arrow head, arrow *newh, expterm m, arrow mloc)
/* monomial ideal */
/* (possibly) new monom ideal of l.c.m.'s of head w. "m" */
/* a monomial, which if it exists in "head", should be ignored here */
{
    arrow p, newhead;
    int i;
    expterm nexp;

    if ((head != NULL) AND (head != head->uld.lda[FOW])) {
        head->umh.mloc = head;
        head->umh.mii = 0;
        if (*newh == NULL)
            *newh = monnewhead (numvars);
        newhead = *newh;
        while ((p = monadv (head,TRUE)) != head) {
            if (p == mloc) continue;
            /*for (i=head->umh.mimin; i<numvars; ++i)*/
            for (i=0; i<numvars; ++i)
                nexp[i] = MAX(m[i],p->umn.mexp[i]);
            if (!monadjoin (newhead,nexp,NULL))
                newhead->umh.mloc->uld.ldc.ci = p->uld.ldc.ci;
        }
    }
}

void IHmo_insert (gmatrix M, term m1, mn_standard bag)
{
    expterm m;
    int c, len, i;
    mn_table *a;
    mn_syzes *b;
    arrow *pp, head, newh, p, newelem;
    mn_pair *q, **r;

    a = &M->montab;
    b = &M->monsyz;
    c = tm_component(m1);
    sToExp(m1, m);
    len = length(a);
    if (len < c) for (i=len+1; i<=c; ++i)
            *((char **) ins_array(a)) = NULL;
    pp = (arrow *) ref(a,c);
    if (*pp == NULL) *pp = monnewhead(numvars);
    head = *pp;

    IHmn_adjoin(M, head, m, bag);
    newh = NULL;
    newelem = head->umh.mloc;
    if (M->nstandard > 1)
        IHmn_lcm(head, &newh, m, newelem);
    IHmn_lcm(Rideal, &newh, m, newelem);
    if (newh != NULL) {
        if (length(b) == 0) {
            r = (mn_pair **) ins_array(b);
            *r = NULL;
        } else
            r = (mn_pair **) ref(b,1);
        newh->umh.mloc = newh;
        while ((p=monadv(newh,TRUE))!=newh) {
            q = (mn_pair *) get_slug(pairStash);
            q->id1 = (char *) bag;
            q->id2 = p->uld.ldc.ci;
            q->mpp = *r;
            *r = q;
        }
        monrefund (newh);
    }
}

boolean IHmo_next_pair (gmatrix M, mn_standard *i, mn_standard *j)
{
    mn_syzes *b;
    mn_pair **p, *q;

    b = &M->monsyz;
    if (length(b) IS 0) return(FALSE);
    p = (mn_pair **) ref(b, 1);
    while ((q = *p) != NULL) {
        *i = (mn_standard) q->id1;
        *j = (mn_standard) q->id2;
        *p = q->mpp;
        free_slug(pairStash, q);
        if (NOT (((*i)->ismin) AND ((*j)->ismin)))
            continue;
        return(TRUE);
    }
    return(FALSE);
}
