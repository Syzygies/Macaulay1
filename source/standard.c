// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <stdio.h>
#include "shared.h"
#include "standard.h"
#include "poly.h"
#include "term.h"
#include "monoms.h"
#include "plist.h"
#include "monitor.h"
#include "shell.h"
#include "stash.h"
#include "gmatrix.h"
#include "charp.h"
#include "vars.h"
#include "array.h"
#include "human_io.h"
#include "mac.h"
#include "betti.h"
#include "set.h"   // autoReduce, showpairs, verbose
#include "mem.h"   // std_stash
#include "ring.h"  // pStash
#include "stats.h" // stdiv, stloop

poly divnode;

// Inline functions to replace macros
static inline term INITIAL(poly f)
{
    return (term)(f->monom);
}

static inline field LEAD_COEF(poly g)
{
    return g->coef;
}

static inline int LENGTH(array a)
{
    return length(&a);
}

static inline gmatrix VAR_MODULE(variable *p)
{
    return (gmatrix)(p->value);
}

static inline poly PREF(plist pl, int i)
{
    return plist_ref(&pl, i);
}

static inline int DREF(dlist dl, int i)
{
    return dlist_ref(&dl, i);
}

static inline void pl_insert(plist *pl, poly f)
{
    plist_insert(pl, f);
}

static inline void dl_insert(dlist *dl, int i)
{
    dlist_insert(dl, i);
}

// TODO: This should be replaced with actual function when vars.h is complete
static inline void send_poly(variable *box, poly f, int deg)
{
    // send_poly stub
    (void)box;
    (void)f;
    (void)deg;
}

// Statistics inline function - no-op unless STATISTICS is defined
#ifdef STATISTICS
static inline void STAT(long *x)
{
    (*x)++;
}

#else
static inline void STAT(long *x)
{
    (void)x; // no-op
}

#endif

void init_division(void)
{
    divnode = p_listhead();
}

void deb_print(poly f, int n)
{
    int i;

    print("[");
    for (i = 1; i < n; i++)
    {
        p_pprint(stdout, f, i);
        print(", ");
    }
    p_pprint(stdout, f, n);
    print("]\n");
}

poly reduce(gmatrix M, poly *f)
{
    mn_standard i;
    allocterm t;
    poly inresult;

    inresult = divnode;
    while (*f != NULL)
    {
        if (mo_find_div(M, INITIAL(*f), (char **)&i, t))
        {
            div_sub(f, LEAD_COEF(*f), t, i->standard);
        }
        else if (autoReduce <= 0)
        {
            inresult->next = *f;
            inresult = *f;
            *f = (*f)->next;
        }
        else
        {
            inresult->next = *f;
            return divnode->next;
        }
    }
    inresult->next = NULL;
    return divnode->next;
}

void division(gmatrix M, poly *f, poly *g, poly *h)
{
    poly inresult;
    mn_standard i;
    allocterm t;

    STAT(&stdiv);
    *g = NULL;
    inresult = divnode;
    while (*f != NULL)
    {
        STAT(&stloop);
        if (mo_find_div(M, INITIAL(*f), (char **)&i, t))
        {
            div_sub(g, LEAD_COEF(*f), t, i->change);
            div_sub(f, LEAD_COEF(*f), t, i->standard);
        }
        else if (autoReduce <= 0)
        {
            inresult->next = *f;
            inresult = *f;
            *f = (*f)->next;
        }
        else
        {
            inresult->next = *f;
            *f = NULL;
            *h = divnode->next;
            return;
        }
    }
    inresult->next = NULL;
    *h = divnode->next;
}

// calc. standard basis routines

boolean calc_standard(gmatrix M, int deg, variable *B)
{
    mn_standard i, j;
    poly h, hrep;

    if (ncols(M) > 0)
    {
        prflush(".");
        if (showpairs > 0)
            spairs_flush(M);
    }
    mo_reset(M, (char **)&i, (char **)&j);
    while (mo_next_pair(M, deg, (char **)&i, (char **)&j))
    {
        calc_s_pair(M, i, j, &h, &hrep);
        send_off(M, deg, B, h, hrep);
    }
    return mo_iscomplete(M, deg);
}

void send_off(gmatrix M, int deg, variable *B, poly h, poly hrep)
{
    if (showapair)
    {
        spairs_flush(M);
        showapair = 0;
    }
    if (h != NULL)
    {
        ins_elem(M, deg, h, hrep);
        if (verbose > 0)
        {
            prflush("m");
            if (showpairs > 2)
                spairs_flush(M);
        }
    }
    else if (hrep != NULL)
    {
        send_poly(B, hrep, deg);
        if (verbose > 0)
        {
            prflush("s");
            if (showpairs > 3)
                spairs_flush(M);
        }
    }
    else if (verbose > 0)
    {
        prflush("o");
        if (showpairs > 3)
            spairs_flush(M);
    }
    intr_shell();
} // 5/18/89 DB 5/6

void calc_s_pair(gmatrix M, mn_standard i, mn_standard j, poly *h, poly *hrep)
{
    allocterm s1, s2;
    poly f, k;

    tm_joinminus(INITIAL(i->standard), INITIAL(j->standard), s1, s2);
    f = mult_sub(s1, i->standard, s2, j->standard);
    k = mult_sub(s1, i->change, s2, j->change);
    division(M, &f, hrep, h);
    p_add(hrep, &k);
}

void ins_elem(gmatrix M, int deg, poly h, poly hrep)
{
    mn_standard i;

    make2_monic(&h, &hrep);
    i = (mn_standard)(void *)get_slug((struct stash *)std_stash);
    i->standard = h;
    i->change = hrep;
    i->next = M->stdbasis;
    i->ismin = TRUE;
    i->degree = deg;
    M->stdbasis = i;
    M->nstandard++;
    M->modtype = MSTD;
    mo_insert(M, INITIAL(h), (char *)i);
    if (autoReduce <= 0)
        auto_reduce(M, deg, h, hrep);
}

void auto_reduce(gmatrix M, int deg, poly h, poly hrep)
{
    mn_standard i;
    field a;

    // assertion: h is monic

    i = M->stdbasis->next; // don't start with current (h,hrep)
    while ((i != NULL) && (i->degree == deg))
    {
        if (occurs_in(i->standard, INITIAL(h), &a))
        {
            if (verbose > 0)
                prflush("a");
            auto_sub(&i->standard, a, h);
            auto_sub(&i->change, a, hrep);
        }
        i = i->next;
    }
}

boolean occurs_in(poly f, term t, field *a)
{
    int comp;

    while (f != NULL)
    {
        comp = tm_compare(INITIAL(f), t);
        if (comp == LT)
            return FALSE;
        if (comp == EQ)
        {
            *a = LEAD_COEF(f);
            return TRUE;
        }
        f = f->next;
    }
    return FALSE;
}

void orig_gens(gmatrix M, int deg, int intval, variable *box)
{
    int i;
    poly f, h, hrep, k;

    for (i = 1; i <= LENGTH(M->gens); i++)
        if (DREF(M->deggens, i) == deg)
        {
            f = p_copy(PREF(M->gens, i));
            division(M, &f, &hrep, &h);
            if ((intval < 0) || (intval >= i))
            {
                k = e_sub_i(i);
                p_add(&hrep, &k);
            }
            send_off(M, deg, box, h, hrep);
        }
}

void ins_generator(variable *box, poly f, int deg)
{
    poly h, hrep;
    int i;
    gmatrix M;

    M = VAR_MODULE(box);
    h = reduce(M, &f);

    if (h == NULL)
        return;
    make1_monic(&h);
    pl_insert(&(M->gens), h);
    i = LENGTH(M->gens);
    dl_insert(&(M->deggens), deg);
    h = p_copy(h);
    if ((box->intval < 0) || (box->intval >= i))
        hrep = e_sub_i(i);
    else
        hrep = NULL;
    ins_elem(M, deg, h, hrep);
}

// added 6/20/91 MES - converted to inline functions 2025

// Helper function to allocate and initialize a polynomial node
static inline poly alloc_poly_node(void)
{
    poly p = (poly)(void *)get_slug((struct stash *)pStash);
    // Initialize monom pointer like p_monom does
    size_t offset = sizeof(struct pol);
    offset = (offset + sizeof(int) - 1) & ~(sizeof(int) - 1);
    p->monom = (char *)p + offset;
    return p;
}

// Process h == NULL case in div_sub
static inline bool check_h_null(poly h, poly f, poly inresult, poly *f1)
{
    if (h == NULL)
    {
        inresult->next = f;
        *f1 = addnode->next;
        return true; // Signal caller to return
    }
    return false;
}

// Process f == NULL case in div_sub
static inline void process_f_null(poly *f, poly *h, poly *inresult, poly *nexth, field a,
                                  bigterm big, poly *f1)
{
    if (*f == NULL)
    {
        while (true)
        {
            (*nexth)->coef = normalize(-(long)a * (long)(*h)->coef);
            (*inresult)->next = *nexth;
            *inresult = *nexth;
            *h = (*h)->next;

            if (check_h_null(*h, *f, *inresult, f1))
                return;

            *nexth = alloc_poly_node();
            tm_add(INITIAL(*h), big, INITIAL(*nexth));
        }
    }
}

// div_sub:  computes f := f - a.t.h
// where a,t,h are not modified
// and  a = field elem,
// t = bigterm
// h = polynomial (in a module)

void div_sub(poly *f1, field a, term t, poly h1)
{
    poly inresult = addnode;
    poly f = *f1;
    poly h = h1;
    poly nexth;
    poly temp;
    bigterm big;

    sToBig(t, big);

    // Check initial h == NULL case
    if (check_h_null(h, f, inresult, f1))
        return;

    // Allocate and initialize nexth
    nexth = alloc_poly_node();
    tm_add(INITIAL(h), big, INITIAL(nexth));

    // Process f == NULL case
    process_f_null(&f, &h, &inresult, &nexth, a, big, f1);
    if (f == NULL)
        return;

    while (true)
    {
        switch (tm_compare(INITIAL(f), INITIAL(nexth)))
        {
        case GT:
            inresult->next = f;
            inresult = f;
            f = f->next;
            process_f_null(&f, &h, &inresult, &nexth, a, big, f1);
            if (f == NULL)
                return;
            break;
        case LT:
            nexth->coef = normalize(-(long)a * (long)h->coef);
            inresult->next = nexth;
            inresult = nexth;
            h = h->next;
            if (check_h_null(h, f, inresult, f1))
                return;
            nexth = alloc_poly_node();
            tm_add(INITIAL(h), big, INITIAL(nexth));
            break;
        case EQ:
            f->coef = normalize((long)f->coef - (long)a * (long)h->coef);
            if (f->coef == 0)
            {
                temp = f;
                f = f->next;
                free_slug((struct stash *)pStash, (struct slug *)temp);
            }
            else
            {
                inresult->next = f;
                inresult = f;
                f = f->next;
            }
            free_slug((struct stash *)pStash, (struct slug *)nexth);
            h = h->next;
            if (check_h_null(h, f, inresult, f1))
                return;
            nexth = alloc_poly_node();
            tm_add(INITIAL(h), big, INITIAL(nexth));
            process_f_null(&f, &h, &inresult, &nexth, a, big, f1);
            if (f == NULL)
                return;
            break;
        }
    }
}

// div_sub for auto reduction

// computes:  f1 := f1 - a*h1

// Process h == NULL case in auto_sub
static inline bool check_h_auto_null(poly h, poly f, poly inresult, poly *f1)
{
    if (h == NULL)
    {
        inresult->next = f;
        *f1 = addnode->next;
        return true; // Signal caller to return
    }
    return false;
}

// Process f == NULL case in auto_sub
static inline void process_f_auto_null(poly *f, poly *h, poly *inresult, field a, poly *f1)
{
    if (*f == NULL)
    {
        while (true)
        {
            poly temp = (poly)(void *)get_slug((struct stash *)pStash);
            temp->coef = normalize(-(long)a * (long)(*h)->coef);
            tm_copy(INITIAL(*h), INITIAL(temp));
            (*inresult)->next = temp;
            *inresult = temp;
            *h = (*h)->next;

            if (check_h_auto_null(*h, *f, *inresult, f1))
                return;
        }
    }
}

void auto_sub(poly *f1, field a, poly h1)
{
    poly inresult = addnode;
    poly f = *f1;
    poly h = h1;
    poly temp;

    // Check initial h == NULL case
    if (check_h_auto_null(h, f, inresult, f1))
        return;

    // Process f == NULL case
    process_f_auto_null(&f, &h, &inresult, a, f1);
    if (f == NULL)
        return;

    while (true)
    {
        switch (tm_compare(INITIAL(f), INITIAL(h)))
        {
        case GT:
            inresult->next = f;
            inresult = f;
            f = f->next;
            process_f_auto_null(&f, &h, &inresult, a, f1);
            if (f == NULL)
                return;
            break;
        case LT:
            temp = (poly)(void *)get_slug((struct stash *)pStash);
            temp->coef = normalize(-(long)a * (long)h->coef);
            tm_copy(INITIAL(h), INITIAL(temp));
            inresult->next = temp;
            inresult = temp;
            h = h->next;
            if (check_h_auto_null(h, f, inresult, f1))
                return;
            break;
        case EQ:
            f->coef = normalize((long)f->coef - (long)a * (long)h->coef);
            if (f->coef == 0)
            {
                temp = f;
                f = f->next;
                free_slug((struct stash *)pStash, (struct slug *)temp);
            }
            else
            {
                inresult->next = f;
                inresult = f;
                f = f->next;
            }
            h = h->next;
            if (check_h_auto_null(h, f, inresult, f1))
                return;
            process_f_auto_null(&f, &h, &inresult, a, f1);
            if (f == NULL)
                return;
            break;
        }
    }
}
