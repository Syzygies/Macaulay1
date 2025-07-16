// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "inhomog.h"
#include "monitor.h"  // prflush()
#include "mac.h"      // intr_shell()
#include "standard.h" // division(), calc_s_pair(), mo_reset()
#include "gmatrix.h"  // ncols()
#include "poly.h"     // p_copy(), p_add(), p_sub(), make2_monic(), e_sub_i(), poly_initial()
#include "hilb.h"     // monnewhead(), monadv(), monreset(), etc.
#include "term.h"     // sToExp(), tm_component()
#include "stash.h"    // get_slug(), free_slug()
#include "monoms.h"   // mo_reset()
#include "boxprocs.h" // send_poly()
#include "set.h"      // verbose
#include "mem.h"      // std_stash

void IHorig_gens(gmatrix M, int intval, variable *B)
{
    int i;
    poly f, k, h, hrep;

    for (i = 1; i <= length(&M->gens); i++)
    {
        f = p_copy(plist_ref(&M->gens, i));
        division(M, &f, &hrep, &h);
        if ((intval < 0) || (intval >= i))
        {
            k = e_sub_i(i);
            p_add(&hrep, &k);
        }
        IHsend_off(M, B, h, hrep);
    }
}

boolean // always returns TRUE
IHcalc_standard(gmatrix M, variable *B)
{
    mn_standard i, j, p;
    poly h, hrep, f, rep;
    boolean notdone;

    if (ncols(M) > 0)
        prflush(".");
    mo_reset(M, (char **)&i, (char **)&j);
    do
    {
        notdone = FALSE;
        while (IHmo_next_pair(M, &i, &j))
        {
            calc_s_pair(M, i, j, &h, &hrep);
            IHsend_off(M, B, h, hrep);
            notdone = TRUE;
        }
        while (M->stdbasis != NULL)
        { // stdbasis is actually scrap heap
            p = M->stdbasis;
            f = p->standard;
            hrep = p->change;
            M->stdbasis = p->next;
            free_slug((struct stash *)std_stash, (struct slug *)p);
            division(M, &f, &rep, &h);
            p_sub(&hrep, &rep);
            IHsend_off(M, B, h, hrep);
            notdone = TRUE;
        }
    } while (notdone);
    return (TRUE);
}

void IHsend_off(gmatrix M, variable *B, poly h, poly hrep)
{
    if (h != NULL)
    {
        IHins_elem(M, h, hrep);
        if (verbose > 0)
            prflush("m");
    }
    else if (hrep != NULL)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
        send_poly(B, hrep, 0);
#pragma GCC diagnostic pop
        if (verbose > 0)
            prflush("s");
    }
    else if (verbose > 0)
        prflush("o");
    intr_shell();
}

void IHins_elem(gmatrix M, poly h, poly hrep)
{
    mn_standard i;

    make2_monic(&h, &hrep);
    i = (mn_standard)(void *)get_slug((struct stash *)std_stash);
    i->standard = h;
    i->change = hrep;
    i->ismin = (char)TRUE;
    i->next = NULL;
    i->degree = 0;
    IHmo_insert(M, poly_initial(h), i);
}

void IHmn_adjoin(gmatrix M, arrow head, expterm nexp, char *bag)
{
    arrow p, new;
    mn_standard q;

    monreset(head, FOW);
    monsearch(head, FOW, nexp);
    moninsert(head, nexp);
    M->nstandard++;
    M->modtype = MISTD;
    new = head->umh.mloc;
    new->uld.ldc.ca = (union all *)(void *)bag;
    monreset(head, BAK);
    while (TRUE)
    {
        monsearch(head, BAK, nexp);
        p = head->umh.mloc;
        if (new == p)
            break;
        else
        {
            q = (mn_standard)p->uld.ldc.ca;
            q->next = M->stdbasis;
            q->ismin = (char)FALSE;
            M->nstandard--;
            M->stdbasis = q;
            mondelete(head);
        }
    }
}

void IHmn_lcm(arrow head, arrow *newh, expterm m, arrow mloc)
{
    arrow p, newhead;
    int i;
    expterm nexp;

    if ((head != NULL) && (head != head->uld.lda[FOW]))
    {
        head->umh.mloc = head;
        head->umh.mii = 0;
        if (*newh == NULL)
            *newh = monnewhead(numvars);
        newhead = *newh;
        while ((p = monadv(head, TRUE)) != head)
        {
            if (p == mloc)
                continue;
            // for (i=head->umh.mimin; i<numvars; ++i)
            for (i = 0; i < numvars; ++i)
                nexp[i] = max_int(m[i], p->umn.mexp[i]);
            if (!monadjoin(newhead, nexp, NULL))
                newhead->umh.mloc->uld.ldc.ca = p->uld.ldc.ca;
        }
    }
}

void IHmo_insert(gmatrix M, term m1, mn_standard bag)
{
    expterm m;
    int c, len, i;
    array *a;
    array *b;
    arrow *pp, head, newh, p, newelem;
    mn_pair *q, **r;

    a = &M->montab;
    b = &M->monsyz;
    c = tm_component(m1);
    sToExp(m1, m);
    len = length(a);
    if (len < c)
        for (i = len + 1; i <= c; ++i)
            *((char **)ins_array(a)) = NULL;
    pp = (arrow *)ref(a, c);
    if (*pp == NULL)
        *pp = monnewhead(numvars);
    head = *pp;

    IHmn_adjoin(M, head, m, (char *)bag);
    newh = NULL;
    newelem = head->umh.mloc;
    if (M->nstandard > 1)
        IHmn_lcm(head, &newh, m, newelem);
    IHmn_lcm(Rideal, &newh, m, newelem);
    if (newh != NULL)
    {
        if (length(b) == 0)
        {
            r = (mn_pair **)ins_array(b);
            *r = NULL;
        }
        else
            r = (mn_pair **)ref(b, 1);
        newh->umh.mloc = newh;
        while ((p = monadv(newh, TRUE)) != newh)
        {
            q = (mn_pair *)(void *)get_slug((struct stash *)pairStash);
            q->id1 = (char *)bag;
            q->id2 = (char *)p->uld.ldc.ca;
            q->mpp = *r;
            *r = q;
        }
        monrefund(newh);
    }
}

boolean IHmo_next_pair(gmatrix M, mn_standard *i, mn_standard *j)
{
    array *b;
    mn_pair **p, *q;

    b = &M->monsyz;
    if (length(b) == 0)
        return (FALSE);
    p = (mn_pair **)ref(b, 1);
    while ((q = *p) != NULL)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
        *i = (mn_standard)q->id1;
        *j = (mn_standard)q->id2;
#pragma GCC diagnostic pop
        *p = q->mpp;
        free_slug((struct stash *)pairStash, (struct slug *)q);
        if (!(((*i)->ismin) && ((*j)->ismin)))
            continue;
        return (TRUE);
    }
    return (FALSE);
}
