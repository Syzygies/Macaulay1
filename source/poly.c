/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "mtypes.h"
#include "stats.h"

// poly p_listhead ();
// void init_polys ();
// poly p_monom (field a);
// poly p_initial (poly f);
// void p_kill (poly *f);
// poly e_sub_i (int i);
// poly p_intpoly (int n, int comp);
// poly p_xjei (int j, int i);
// poly p_copy (poly f);
// void p_add (poly *f1, poly *g1);
// void p_negate (poly *g);
// void p_sub (poly *f, poly *g);
// poly mult_sub (term s1, poly g1, term s2, poly g2);
// void make1_monic (poly *f);
// void make2_monic (poly *f, poly *frep);

poly addnode;

poly p_listhead ()
{
    poly result;

    result = (poly) get_small(sizeof(poly));
    return(result);
}

void init_polys ()
{
    addnode = p_listhead();
}

/* WARNING: p_monom does not set the "monom" component of result */

poly p_monom (field a)
{
    poly result;

    result = (poly) get_slug(pStash);
    result->coef = fd_copy(a);
    result->next = NULL;
    return(result);
}

poly p_initial (poly f)
{
    poly result;

    result = p_monom(f->coef);
    tm_copy(INITIAL(f), INITIAL(result));
    return(result);
}

void p_kill (poly *f)
{
    poly temp;
    poly wf;

    wf = *f;
    while (wf ISNT NULL) {
        temp = wf;
        wf = wf->next;
        free_slug(pStash, temp);
    }
    *f = NULL;
}

poly e_sub_i (int i)
{
    poly result;

    result = p_monom(fd_one);
    term_i(i, INITIAL(result));
    return(result);
}

poly p_intpoly (int n, int comp)
{
    poly result;
    field a;

    a = normalize(n);
    if (a IS 0) return(NULL);
    result = p_monom(a);
    term_i(comp, INITIAL(result));
    return(result);
}

poly p_xjei (int j, int i)
{
    poly result;

    result = p_monom(fd_one);
    tm_xjei(j, i, INITIAL(result));
    qrgReduce(&result);
    return(result);
}

poly p_copy (poly f)
{
    poly inresult;

    inresult = addnode;
    addnode->next = NULL;  /* fix: added 11/3/85 MES */
    while (f ISNT NULL) {
        inresult->next = p_initial(f);
        f = f->next;
        inresult = inresult->next;
    }
    return(addnode->next);
}

poly p_in(f, n) /* returns the initial terms of f which agree on the first */
poly f;    /* "n" blocks of monomials (not including component) */
int n;
{
    term t;
    poly inresult;

    if (f IS NULL) return(NULL);
    t = (term) INITIAL(f);
    inresult = addnode;
    addnode->next = NULL;
    while ((f ISNT NULL) AND (tm_inEq(t, INITIAL(f), n))) {
        inresult->next = p_initial(f);
        f = f->next;
        inresult = inresult->next;
    }
    return(addnode->next);
}

void p_add (poly *f1, poly *g1)
{
    poly inresult, f, g;
    poly temp1, temp2;

    inresult = addnode;
    f = *f1;
    g = *g1;
    *g1 = NULL;

    while (TRUE) {
        if (f IS NULL) {
            inresult->next = g;
            *f1 = addnode->next;
            return;
        } else if (g IS NULL) {
            inresult->next = f;
            *f1 = addnode->next;
            return;
        } else switch (tm_compare(INITIAL(f), INITIAL(g))) {
            case GT :
                inresult->next = f;
                inresult = f;
                f = f->next;
                break;
            case LT :
                inresult->next = g;
                inresult = g;
                g = g->next;
                break;
            case EQ :
                temp1 = f;
                temp2 = g;
                f = f->next;
                g = g->next;
                temp1->next = NULL;
                temp2->next = NULL;
                fd_add(temp1->coef, temp2->coef, &(temp1->coef));
                p_kill(&temp2);
                if (fd_iszero(temp1->coef))
                    p_kill(&temp1);
                else {
                    inresult->next = temp1;
                    inresult = temp1;
                }
                break;
            } /*case */
    }
}

void p_negate (poly *g)
{
    poly temp;

    for (temp=(*g); temp ISNT NULL; temp = temp->next)
        fd_negate(&(temp->coef));
}

void p_sub (poly *f, poly *g)
{
    p_negate(g);
    p_add(f, g);
}

poly
p1_mult(a, t, f)   /* computes a.t.f */
field a;
term t;
poly f;
{
    poly inresult;
    field b;
    bigterm big;

    if (f IS NULL) return(NULL);
    inresult = addnode;
    inresult->next = NULL;
    sToBig(t, big);
    while (f ISNT NULL) {
        fd_mult(a, f->coef, &b);
        inresult->next = p_monom(b);
        inresult = inresult->next;
        tm_add(INITIAL(f), big, INITIAL(inresult));
        f = f->next;
    }
    return(addnode->next);
}

poly
p_mult(f, g)       /* f in ring, g in module */
poly f, g;
{
    poly result, temp;

    result = NULL;
    while (f ISNT NULL) {
        temp = p1_mult(f->coef, INITIAL(f), g);
        p_add(&result, &temp);
        f = f->next;
    }
    qrgReduce(&result);
    return(result);
}

/* special routines for computing standard bases */

/* computes : f = f - a.t.h  (a, t, h are not modified) */

special_sub(f, a, t, h)        /* f, h in module; a.t in ring */
poly *f;
field a;
term t;
poly h;
{
    poly g;

    STAT(stspecial);
    g = p1_mult(a, t, h);
    p_sub(f, &g);
}

/* computes : s1.g1 - s2.g2 , where either si or gi has comp=0, both i */

poly mult_sub (term s1, poly g1, term s2, poly g2)
{
    poly h1, h2;

    h1 = p1_mult(fd_one, s1, g1);
    h2 = p1_mult(fd_one, s2, g2);
    p_sub(&h1, &h2);
    return(h1);
}

/* make_monic : divides f, frep by lead coef of f */

void make1_monic (poly *f)
{
    field a;
    poly p;
    a = fd_copy((*f)->coef);
    fd_recip(&a);
    for (p=(*f); p ISNT NULL; p = p->next)
        fd_mult(a, p->coef, &(p->coef));
}

void make2_monic (poly *f, poly *frep)
{
    field a;
    poly p;

    a = fd_copy((*f)->coef);
    fd_recip(&a);
    for (p=(*f); p ISNT NULL; p = p->next)
        fd_mult(a, p->coef, &(p->coef));
    for (p=(*frep); p ISNT NULL; p = p->next)
        fd_mult(a, p->coef, &(p->coef));
}
