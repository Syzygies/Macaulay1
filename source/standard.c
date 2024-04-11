/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"
#include "stats.h"

// void init_division ();
// void deb_print (poly f, int n);
// poly reduce (gmatrix M, poly *f);
// void division (gmatrix M, poly *f, poly *g, poly *h);
// boolean calc_standard (gmatrix M, int deg, variable *B);
void send_off (gmatrix M, int deg, variable *B, poly h, poly hrep);
void calc_s_pair (gmatrix M, mn_standard i, mn_standard j, poly *h, poly *hrep);
void ins_elem (gmatrix M, int deg, poly h, poly hrep);
void auto_reduce (gmatrix M, int deg, poly h, poly hrep);
// boolean occurs_in (poly f, term t, field *a);
// void orig_gens (gmatrix M, int deg, int intval, variable *box);
// void ins_generator (variable *box, poly f, int deg);
void div_sub (poly *f1, field a, term t, poly h1);
void auto_sub (poly *f1, field a, poly h1);

poly divnode;
extern int autoReduce; /* >0 means don't autoreduce */
extern int showpairs;  /* >0 means call spairs_flush */

void init_division ()
{
    divnode = p_listhead();
}

void deb_print (poly f, int n)
{
    int i;

    print("[");
    for (i=1; i<n; i++) {
        p_pprint(stdout, f, i);
        print(", ");
    }
    p_pprint(stdout, f, n);
    print("]\n");
}

poly reduce (gmatrix M, poly *f)
{
    mn_standard i;
    allocterm t;
    poly inresult;

    inresult = divnode;
    while (*f ISNT NULL) {
        if (mo_find_div(M, INITIAL(*f), &i, t)) {
            div_sub(f, LEAD_COEF(*f), t, i->standard);
        } else if (autoReduce <= 0) {
            inresult->next = *f;
            inresult = *f;
            *f = (*f)->next;
        } else {
            inresult->next = *f;
            return(divnode->next);
        }
    }
    inresult->next = NULL;
    return(divnode->next);
}

void division (gmatrix M, poly *f, poly *g, poly *h)
{
    poly inresult;
    mn_standard i;
    allocterm t;

    STAT(stdiv);
    *g = NULL;
    inresult = divnode;
    while (*f ISNT NULL) {
        STAT(stloop);
        if (mo_find_div(M, INITIAL(*f), &i, t)) {
            div_sub(g, LEAD_COEF(*f), t, i->change);
            div_sub(f, LEAD_COEF(*f), t, i->standard);
        } else if (autoReduce <= 0) {
            inresult->next = *f;
            inresult = *f;
            *f = (*f)->next;
        } else {
            inresult->next = *f;
            *f = NULL;
            *h = divnode->next;
            return;
        }
    }
    inresult->next = NULL;
    *h = divnode->next;
}

/*------ calc. standard basis routines -----------------*/

boolean calc_standard (gmatrix M, int deg, variable *B)
{
    mn_standard i, j;
    poly h, hrep;

    if (ncols(M) > 0) {
        prflush(".");
        if (showpairs > 0)
            spairs_flush(M);
    }
    mo_reset(M, &i, &j);
    while (mo_next_pair(M, deg, &i, &j)) {
        calc_s_pair(M, i, j, &h, &hrep);
        send_off(M, deg, B, h, hrep);
    }
    return(mo_iscomplete(M, deg));
}

extern int showapair;

void send_off (gmatrix M, int deg, variable *B, poly h, poly hrep)
{
    if (showapair) {
        spairs_flush(M);
        showapair = 0;
    }
    if (h ISNT NULL) {
        ins_elem(M, deg, h, hrep);
        if (verbose > 0) {
            prflush("m");
            if (showpairs > 2) spairs_flush(M);
        }
    } else if (hrep ISNT NULL) {
        send_poly(B, hrep, deg);
        if (verbose > 0) {
            prflush("s");
            if (showpairs > 3) spairs_flush(M);
        }
    } else if (verbose > 0) {
        prflush("o");
        if (showpairs > 3) spairs_flush(M);
    }
    intr_shell();
} /* 5/18/89 DB 5/6 */

void calc_s_pair (gmatrix M, mn_standard i, mn_standard j, poly *h, poly *hrep)
{
    allocterm s1, s2;
    poly f, k;

    tm_joinminus(INITIAL(i->standard),
                 INITIAL(j->standard),
                 s1, s2);
    f = mult_sub(s1, i->standard,
                 s2, j->standard);
    k = mult_sub(s1, i->change,
                 s2, j->change);
    division(M, &f, hrep, h);
    p_add(hrep, &k);
}

void ins_elem (gmatrix M, int deg, poly h, poly hrep)
{
    mn_standard i;

    make2_monic(&h, &hrep);
    i = (mn_standard) get_slug(std_stash);
    i->standard = h;
    i->change = hrep;
    i->next = M->stdbasis;
    i->ismin = (char) TRUE;
    i->degree = deg;
    M->stdbasis = i;
    M->nstandard++;
    M->modtype = MSTD;
    mo_insert(M, INITIAL(h), i);
    if (autoReduce <= 0) auto_reduce(M, deg, h, hrep);
}

void auto_reduce (gmatrix M, int deg, poly h, poly hrep)
{
    mn_standard i;
    field a;

    /* assertion: h is monic */

    i = M->stdbasis->next; /* don't start with current (h,hrep) */
    while ((i ISNT NULL) AND (i->degree IS deg)) {
        if (occurs_in(i->standard, INITIAL(h), &a)) {
            if (verbose > 0) prflush("a");
            auto_sub(&i->standard, a, h);
            auto_sub(&i->change, a, hrep);
        }
        i = i->next;
    }
}

boolean occurs_in (poly f, term t, field *a)
{
    int comp;

    while (f ISNT NULL) {
        comp = tm_compare(INITIAL(f), t);
        if (comp IS LT) return(FALSE);
        if (comp IS EQ) {
            *a = LEAD_COEF(f);
            return(TRUE);
        }
        f = f->next;
    }
    return(FALSE);
}

/*-----------------------------------------------------------*/

void orig_gens (gmatrix M, int deg, int intval, variable *box)
{
    int i;
    poly f, h, hrep, k;

    for (i=1; i<=LENGTH(M->gens); i++)
        if (DREF(M->deggens, i) IS deg) {
            f = p_copy(PREF(M->gens, i));
            division(M, &f, &hrep, &h);
            if ((intval < 0) OR (intval >= i)) {
                k = e_sub_i(i);
                p_add(&hrep, &k);
            }
            send_off(M, deg, box, h, hrep);
        }
}

void ins_generator (variable *box, poly f, int deg)
{
    poly h, hrep;
    int i;
    gmatrix M;

    M = VAR_MODULE(box);
    h = reduce(M, &f);

    if (h IS NULL) return;
    make1_monic(&h);
    pl_insert(&(M->gens), h);
    i = LENGTH(M->gens);
    dl_insert(&(M->deggens), deg);
    h = p_copy(h);
    if ((box->intval < 0) OR (box->intval >= i))
        hrep = e_sub_i(i);
    else hrep = NULL;
    ins_elem(M, deg, h, hrep);
}

/*----- added 6/20/91 MES --------------------------*/

extern poly addnode;

#define H_NULL  {  if (h IS NULL) { \
             inresult->next = f; \
             *f1 = addnode->next; \
             return; \
           } \
           nexth = (poly) get_slug(pStash); \
           tm_add(INITIAL(h), big, INITIAL(nexth)); \
         }

#define F_NULL  {  if (f IS NULL) { \
             while (TRUE) { \
            nexth->coef = normalize(-a*h->coef); \
            inresult->next = nexth; \
            inresult = nexth; \
            h = h->next; \
            H_NULL \
              } \
           } \
         }

/* div_sub:  computes f := f - a.t.h
 *   where a,t,h are not modified
 *   and  a = field elem,
 *        t = bigterm
 *        h = polynomial (in a module)
 */

void div_sub (poly *f1, field a, term t, poly h1)
{
    poly inresult = addnode;
    poly f = *f1;
    poly h = h1;
    poly nexth;
    poly temp;
    bigterm big;

    sToBig(t, big);
    H_NULL;
    F_NULL;

    while (TRUE) {
        switch (tm_compare(INITIAL(f), INITIAL(nexth))) {
        case GT:
            inresult->next = f;
            inresult = f;
            f = f->next;
            F_NULL;
            break;
        case LT:
            nexth->coef = normalize(-a*h->coef);
            inresult->next = nexth;
            inresult = nexth;
            h = h->next;
            H_NULL;
            break;
        case EQ:
            f->coef = normalize(f->coef - a * h->coef);
            if (f->coef IS 0) {
                temp = f;
                f = f->next;
                free_slug(pStash, temp);
            } else {
                inresult->next = f;
                inresult = f;
                f = f->next;
            }
            free_slug(pStash, nexth);
            h = h->next;
            H_NULL;
            F_NULL;
            break;
        }
    }
}

/*---------------- div_sub for auto reduction ------------------*/

/* computes:  f1 := f1 - a*h1 */

#define H_AUTO_NULL {  if (h IS NULL) { \
             inresult->next = f; \
             *f1 = addnode->next; \
             return; \
           } \
         }

#define F_AUTO_NULL {  if (f IS NULL) { \
             while (TRUE) { \
            temp = (poly) get_slug(pStash); \
            temp->coef = normalize(-a*h->coef); \
            tm_copy(INITIAL(h), INITIAL(temp)); \
            inresult->next = temp; \
            inresult = temp; \
            h = h->next; \
            H_AUTO_NULL \
              } \
           } \
         }

void auto_sub (poly *f1, field a, poly h1)
{
    poly inresult = addnode;
    poly f = *f1;
    poly h = h1;
    poly temp;

    H_AUTO_NULL;
    F_AUTO_NULL;

    while (TRUE) {
        switch (tm_compare(INITIAL(f), INITIAL(h))) {
        case GT:
            inresult->next = f;
            inresult = f;
            f = f->next;
            F_AUTO_NULL;
            break;
        case LT:
            temp = (poly) get_slug(pStash);
            temp->coef = normalize(-a*h->coef);
            tm_copy(INITIAL(h), INITIAL(temp));
            inresult->next = temp;
            inresult = temp;
            h = h->next;
            H_AUTO_NULL;
            break;
        case EQ:
            f->coef = normalize(f->coef - a * h->coef);
            if (f->coef IS 0) {
                temp = f;
                f = f->next;
                free_slug(pStash, temp);
            } else {
                inresult->next = f;
                inresult = f;
                f = f->next;
            }
            h = h->next;
            H_AUTO_NULL;
            F_AUTO_NULL;
            break;
        }
    }
}
