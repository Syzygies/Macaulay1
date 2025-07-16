// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "gm_poly.h"
#include "poly.h"
#include "term.h"
#include "charp.h"
#include "array.h"
#include "qring.h"

// INITIAL inline function - accesses monomial part of polynomial
static inline term INITIAL(poly f)
{
    return f->monom;
}

int get_comp(poly f)
{
    return tm_component(INITIAL(f));
}

void set_comp(poly f, int n)
{
    tm_setcomp(INITIAL(f), n);
}

// getcol  takes the polynomial (column) "f", transposes it into a row,
// and adds it to the "r"th row of "a".

void getcol(plist *a, poly f, int r)
{
    poly g;
    int i;

    for (i = 1; i <= length(a); i++)
    {
        g = extr1(f, i, r);
        p_add(plist_ref_ptr(a, i), &g);
    }
}

// dot
// Computes the dot product of the two vectors  f, g.  The result
// is a ring element, but each term in the result is given component
// = comp.

poly p11_mult(poly f, bigterm big, poly g, int comp) // f, g = single monomials, comp = comp. of
{                                                    // resulting product
    register poly result;
    field a;

    fd_mult(f->coef, g->coef, &a);
    result = p_monom(a);
    tm_add(INITIAL(g), big, INITIAL(result));
    set_comp(result, comp);
    return result;
}

poly dot1(poly f, poly g, int comp) // f = monom., g = vector
{
    poly result, temp;
    int which;
    bigterm big;

    result = NULL;
    which = get_comp(f);
    sToBig(INITIAL(f), big);
    while (g != NULL)
    {
        if (which == get_comp(g))
        {
            temp = p11_mult(f, big, g, comp);
            p_add(&result, &temp);
        }
        g = g->next;
    }
    return result;
}

poly p_dot(poly f, poly g, int comp)
{
    poly result, temp;

    result = NULL;
    while (f != NULL)
    {
        temp = dot1(f, g, comp);
        p_add(&result, &temp);
        f = f->next;
    }
    qrgReduce(&result);
    return result;
}

// compshift   "shift components"  This returns the polynomial
// obtained by adding "n" to each component of each monomial of  "f".
// "f" itself is not modified.

poly compshift(poly f, int n)
{
    poly result;

    f = p_copy(f);
    result = f;
    while (f != NULL)
    {
        set_comp(f, n + get_comp(f));
        f = f->next;
    }
    return result;
}

// tensorShift
// changes component "i" to (i-1)*m + j
// Used for computing tensor product of modules. "f" isn't modified.

poly tensorShift(poly f, int m, int j)
{
    poly result;

    f = p_copy(f);
    result = f;
    while (f != NULL)
    {
        set_comp(f, (get_comp(f) - 1) * m + j);
        f = f->next;
    }
    return result;
}

// extract

// returns the poly obtained from "f" by extracting those terms
// with component = a[i], some i (i.e. which occur in "a"), and setting
// their new component to "i".

poly extr1(poly f, int i, int j)
{
    register poly p;

    p = addnode;
    p->next = NULL;
    while (f != NULL)
    {
        if (i == get_comp(f))
        {
            p->next = p_initial(f);
            p = p->next;
            set_comp(p, j);
        }
        f = f->next;
    }
    return addnode->next;
}

poly extract(poly f, dlist *a)
{
    poly result, temp;
    int j;

    result = NULL;
    for (j = 1; j <= length(a); j++)
    {
        temp = extr1(f, dlist_ref(a, j), j);
        p_add(&result, &temp);
    }
    return result;
}

// The following routine allows the changing of an entry of a matrix
// "g" will be destoyed, and each component of g should be "i".

void p_chEntry(poly *f, int i, poly g)
{
    poly p, q;

    p = addnode;
    p->next = *f;
    while (p->next != NULL)
        if (get_comp(p->next) == i)
        {
            q = p->next;
            p->next = q->next;
            q->next = NULL;
            p_kill(&q);
        }
        else
            p = p->next;
    *f = addnode->next;
    p_add(f, &g);
}

void p_divmod(poly *ff, poly g, poly *hdiv, poly *hmod)
{
    poly rdiv, rmod, f, h;
    field a, coef;

    if (g == NULL)
    {
        *hdiv = NULL;
        *hmod = *ff;
        *ff = NULL;
        return;
    }
    rdiv = NULL;
    rmod = NULL;
    f = *ff;
    a = fd_copy(g->coef);
    fd_recip(&a);
    while (f != NULL)
    {
        fd_mult(f->coef, a, &coef);
        h = p_monom(coef);
        if (!tm_divides(INITIAL(f), INITIAL(g), get_comp(f), INITIAL(h)))
        {
            p_kill(&h);
            h = f;
            f = f->next;
            h->next = NULL;
            p_add(&rmod, &h);
        }
        else
        {
            special_sub(&f, coef, INITIAL(h), g);
            p_add(&rdiv, &h);
        }
    }
    *hdiv = rdiv;
    *hmod = rmod;
}

poly p_div(poly *f, poly g) // divides  f  (module elem.) by  g  (ring elem.)
{
    poly hdiv, hmod;

    p_divmod(f, g, &hdiv, &hmod);
    p_kill(&hmod);
    return hdiv;
}

poly p_mod(poly *f, poly g)
{
    poly hdiv, hmod;

    p_divmod(f, g, &hdiv, &hmod);
    p_kill(&hdiv);
    return hmod;
}

poly p_diff(poly f, int var, int comp)
{
    poly result, g;
    int exp;
    field a;

    result = NULL;
    while (f != NULL)
    {
        g = p_monom(fd_zero);
        tm_diff(INITIAL(f), INITIAL(g), var, &exp);
        a = normalize(exp);
        if (a != 0)
        {
            fd_mult(f->coef, a, &(g->coef));
            set_comp(g, comp);
            p_add(&result, &g);
        }
        else
            p_kill(&g);
        f = f->next;
    }
    return result;
}

poly p_contract(poly x, poly f, boolean usecoef, int comp)
{
    poly result, y, g, h;
    field a, b;
    int coefval;

    result = NULL;
    for (y = x; y != NULL; y = y->next)
    {
        for (g = f; g != NULL; g = g->next)
        {
            h = p_monom(fd_zero);
            tm_contract(INITIAL(y), INITIAL(g), INITIAL(h), &coefval);
            a = normalize(coefval);
            if ((usecoef && (a != 0)) || ((!usecoef) && (coefval != 0)))
            {
                if (usecoef)
                    fd_mult(g->coef, a, &b);
                else
                    b = g->coef;
                fd_mult(y->coef, b, &h->coef);
                set_comp(h, comp);
                p_add(&result, &h);
            }
            else
                p_kill(&h);
        }
    }
    return result;
}
