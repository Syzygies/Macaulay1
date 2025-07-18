// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "poly.h"
#include "mem.h"
#include "stash.h"
#include "charp.h"
#include "term.h"
#include "qring.h"
#include "ring.h"
#ifdef STATISTICS
#include "stats.h"
#endif

// initial() inline function is now in shared.h

poly addnode;

poly p_listhead(void)
{
    poly result;
    void *ptr;

    ptr = get_small(sizeof(struct pol));
    result = (poly)ptr;
    return (result);
}

void init_polys(void)
{
    addnode = p_listhead();
}

// WARNING: p_monom does not set the "monom" component of result

poly p_monom(field a)
{
    poly result;
    void *ptr;
    size_t offset;
    term t;
    int i;

    ptr = get_slug((struct stash *)pStash);
    result = (poly)ptr;
    result->coef = fd_copy(a);
    result->next = NULL;

    // Calculate offset for monom pointer
    // The poly struct already contains the coef field, so we just need to skip past the struct
    offset = sizeof(struct pol);
    // CRITICAL: Align to sizeof(int) boundary for term data
    offset = (offset + sizeof(int) - 1) & ~(sizeof(int) - 1);
    result->monom = (char *)ptr + offset;

    // CRITICAL: Zero out the term data to prevent garbage
    t = (term)result->monom;
    for (i = 0; i < nblocks; i++)
        t[i] = 0;

    return (result);
}

poly p_initial(poly f)
{
    poly result;

    result = p_monom(f->coef);
    tm_copy(initial(f), initial(result));
    return (result);
}

void p_kill(poly *f)
{
    register poly temp;
    register poly wf;

    wf = *f;
    while (wf != NULL)
    {
        temp = wf;
        wf = wf->next;
        free_slug((struct stash *)pStash, (struct slug *)temp);
    }
    *f = NULL;
}

poly e_sub_i(int i)
{
    poly result;

    result = p_monom(fd_one);
    term_i(i, initial(result));
    return (result);
}

poly p_intpoly(int n, int comp)
{
    poly result;
    register field a;

    a = normalize(n);
    if (a == 0)
        return (NULL);
    result = p_monom(a);
    term_i(comp, initial(result));
    return (result);
}

poly p_xjei(int j, int i)
{
    poly result;

    result = p_monom(fd_one);
    tm_xjei(j, i, initial(result));
    qrgReduce(&result);
    return (result);
}

poly p_copy(poly f)
{
    poly inresult;

    inresult = addnode;
    addnode->next = NULL; // fix: added 11/3/85 MES
    while (f != NULL)
    {
        inresult->next = p_initial(f);
        f = f->next;
        inresult = inresult->next;
    }
    return (addnode->next);
}

poly p_in(poly f, int n) // returns the initial terms of f which agree on the first
{                        // "n" blocks of monomials (not including component)
    term t;
    poly inresult;

    if (f == NULL)
        return (NULL);
    t = (term)initial(f);
    inresult = addnode;
    addnode->next = NULL;
    while ((f != NULL) && (tm_inEq(t, initial(f), n)))
    {
        inresult->next = p_initial(f);
        f = f->next;
        inresult = inresult->next;
    }
    return (addnode->next);
}

void p_add(poly *f1, poly *g1)
{
    poly inresult, f, g;
    poly temp1, temp2;

    inresult = addnode;
    f = *f1;
    g = *g1;
    *g1 = NULL;

    while (TRUE)
    {
        if (f == NULL)
        {
            inresult->next = g;
            *f1 = addnode->next;
            return;
        }
        else if (g == NULL)
        {
            inresult->next = f;
            *f1 = addnode->next;
            return;
        }
        else
            switch (tm_compare(initial(f), initial(g)))
            {
            case GT:
                inresult->next = f;
                inresult = f;
                f = f->next;
                break;
            case LT:
                inresult->next = g;
                inresult = g;
                g = g->next;
                break;
            case EQ:
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
                else
                {
                    inresult->next = temp1;
                    inresult = temp1;
                }
                break;
            } // case
    }
}

void p_negate(poly *g)
{
    poly temp;

    for (temp = (*g); temp != NULL; temp = temp->next)
        fd_negate(&(temp->coef));
}

void p_sub(poly *f, poly *g)
{
    p_negate(g);
    p_add(f, g);
}

poly p1_mult(field a, term t, poly f) // computes a.t.f
{
    poly inresult;
    field b;
    bigterm big;

    if (f == NULL)
        return (NULL);
    inresult = addnode;
    inresult->next = NULL;
    sToBig(t, big);
    while (f != NULL)
    {
        fd_mult(a, f->coef, &b);
        inresult->next = p_monom(b);
        inresult = inresult->next;
        tm_add(initial(f), big, initial(inresult));
        f = f->next;
    }
    return (addnode->next);
}

poly p_mult(poly f, poly g) // f in ring, g in module
{
    poly result, temp;

    result = NULL;
    while (f != NULL)
    {
        temp = p1_mult(f->coef, initial(f), g);
        p_add(&result, &temp);
        f = f->next;
    }
    qrgReduce(&result);
    return (result);
}

// special routines for computing standard bases

// computes : f = f - a.t.h  (a, t, h are not modified)

void special_sub(poly *f, field a, term t, poly h) // f, h in module; a.t in ring
{
    poly g;

#ifdef STATISTICS
    stspecial++;
#endif
    g = p1_mult(a, t, h);
    p_sub(f, &g);
}

// computes : s1.g1 - s2.g2 , where either si or gi has comp=0, both i

poly mult_sub(term s1, poly g1, term s2, poly g2)
{
    poly h1, h2;

    h1 = p1_mult(fd_one, s1, g1);
    h2 = p1_mult(fd_one, s2, g2);
    p_sub(&h1, &h2);
    return (h1);
}

// make_monic : divides f, frep by lead coef of f

void make1_monic(poly *f)
{
    field a;
    poly p;
    a = fd_copy((*f)->coef);
    fd_recip(&a);
    for (p = (*f); p != NULL; p = p->next)
        fd_mult(a, p->coef, &(p->coef));
}

void make2_monic(poly *f, poly *frep)
{
    field a;
    poly p;

    a = fd_copy((*f)->coef);
    fd_recip(&a);
    for (p = (*f); p != NULL; p = p->next)
        fd_mult(a, p->coef, &(p->coef));
    for (p = (*frep); p != NULL; p = p->next)
        fd_mult(a, p->coef, &(p->coef));
}
