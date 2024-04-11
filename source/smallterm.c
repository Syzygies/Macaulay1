/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "style.h"
#include "iring.h"

// symmBlock findBlock (int n, int *degs);
// symmBlock symmCreate (int n, int *degs, char **varnames);
// void prSymmBlock (symmBlock symm);
// smallmon btos (symmBlock glo, int *exp);
// void stob1 (symmBlock glo, smallmon val, int *exp);
// smallmon madd1 (symmBlock glo, smallmon val, int *exp);
// void stob2 (symmBlock glo, smallmon val, int *exp);
// smallmon madd2 (symmBlock glo, smallmon val, int *exp);

#define DECR(A,B) ((A) -= (B))

extern smallmon madd1(), madd2();
extern void stob1(), stob2();
extern smallmon btos();

extern int maxdegree; /* default maximum degree for monomials */

typedef long offset; /* for vars to be added to ptrs */

symmBlock blockList;   /* list of all symmBlocks created */

/* findBlock is called to determine if any of the monomial tables
 * so far constructed can be reused.  This is the case if the same number
 * of variables, with the same number of weights already has been computed.
 *  HOWEVER, the top degree's must match up: otherwise a new table must
 * be recomputed.
 */

symmBlock findBlock (int n, int *degs)
{
    symmBlock p = blockList;
    int i;
    boolean found;

    while (p ISNT NULL)  {
        if (p->defaultmax IS maxdegree) {
            if (n IS p->nvars) {
                found = TRUE;
                for (i=0; i<n; i++)
                    if (p->degs[i] ISNT degs[i]) {
                        found = FALSE;
                        break;
                    }
                if (found IS TRUE) return(p);
            }
        }
        p = p->nextBlock;
    }
    return(NULL);
}

symmBlock symmCreate (int n, int *degs, char **varnames)
/* number of variables */
/* array 0..n, with degs[n]=1 */
/* array 0..n-1 of char. strings */
{
    symmBlock glo, p;
    smallmon *scaf, big, x, y;
    smallmon **cols;
    int top, d;
    int i, j, k;
    char *gimmy();

    glo = (symmBlock) gimmy(sizeof(struct symblock));

    p = findBlock(n, degs);
    if (p ISNT NULL) {
        *glo = *p;
        glo->names = varnames;
        glo->degree = 1;
        return(glo);
    } else {
        glo->nextBlock = blockList;
        blockList = glo;
    }
    glo->rtyp = RSYMM;
    glo->nvars = n;
    glo->degs = degs;
    glo->names = varnames;
    glo->defaultmax = maxdegree;

    glo->n1 = n-1;
    glo->degree = 1;
    glo->w1 = TRUE;
    for (i=0; i<n; i++)
        if (degs[i] ISNT 1)
            glo->w1 = FALSE;
    if (glo->w1) {
        glo->stob = stob1;
        glo->madd = madd1;
    } else {
        glo->stob = stob2;
        glo->madd = madd2;
    }
    glo->btos = btos;

    big = ~0L;
    top = maxdegree+1;  /* maxdegree = "set" variable */
    scaf = (smallmon *) gimmy((top)*sizeof(smallmon));
    for (k=0; k<top; ++k) scaf[k] = 1;
    for (i=0; i<n; ++i) {
        d = degs[i];
        for (j=0; j<d; ++j) {
            x = scaf[j];
            for (k=j+d; k<top; k+=d) {
                y = scaf[k];
                if (big-y <= x) top = k;
                else {
                    x += y;
                    scaf[k] = x;
                }
            }
        }
    }
    glo->maxval = scaf[top-1] - 1;
    ungimmy(scaf);
    glo->maxdeg = top-1;
    glo->over = top;

    glo->table = (smallmon *) gimmy((1+n*top)*sizeof(smallmon));
    cols = (smallmon **) gimmy(n*sizeof(smallmon *));
    glo->cols = cols;
    for (i=0; i<n; ++i) cols[i] = glo->table + i*top;
    d = degs[0];
    for (k=1; k<top; ++k) cols[0][k] = 0;
    for (k=0; k<top; k+=d) cols[0][k] = 1;
    for (i=1; i<n; ++i) {
        d = degs[i];
        for (j=0; j<d; ++j) {
            x = cols[i-1][j];
            cols[i][j] = x;
            for (k=j+d; k<top; k+=d) {
                x += cols[i-1][k];
                cols[i][k] = x;
            }
        }
    }
    for (i=0; i<n; ++i) {
        d = degs[i+1];
        for (j=0; j<d; ++j) {
            x = 0;
            for (k=j; k<top; k+=d) {
                y = cols[i][k];
                cols[i][k] = x;
                x += y;
            }
        }
    }
    cols[n-1][top] = big;
    return(glo);
}

void prSymmBlock (symmBlock symm)
{
    int i, j;

    print("\nsymm block:\n");
    print("#vars = %d, w1=%d, maxdeg=%d, over=%d, n1=%d, degree=%d\n",
          symm->nvars, symm->w1, symm->maxdeg, symm->over, symm->n1,
          symm->degree);
    print("degrees = ");
    for (i=0; i<=symm->nvars; i++)
        print("%d ", symm->degs[i]);
    print("\n");

    for (i=0; i<symm->nvars; i++) {
        for (j=0; j<=symm->maxdeg; j++)
            print("%lu ", symm->cols[i][j]);
        print("\n");
    }
    print("maxval = %lu\n", symm->maxval);
}

smallmon btos (symmBlock glo, int *exp)
{
    smallmon v, **t;
    int i, *e;
    offset n1;

    v = 0;
    e = exp;
    t = glo->cols;
    n1 = glo->n1;
    if (e[n1] > glo->maxdeg) {
        prerror("; degree bound exceeded in monomial operation\n");
        to_shell();
    }
    for (i=0; i<=n1; ++i) v += (*t++)[*e++];
    return v;
}

void stob1 (symmBlock glo, smallmon val, int *exp)
{
    int d, i, ov, *e;
    smallmon v, *t;

    v = val;
    ASSERT("smallmon.c stob1",v <= glo->maxval);
    d = glo->degree;
    i = glo->n1;
    ov = glo->over;
    e = exp + i;
    t = glo->cols[i] + d;
    if (*t <= v) {
        do ++d, ++t;
        while (*t <= v);
        glo->degree = d;
        --d, --t;
    }
    else if (--d, *--t > v) {
        do --d, --t;
        while (*t > v);
        glo->degree = d+1;
    }
    v -= *t;
    *e-- = d;
    if (i != 0) do {
            DECR(t,ov);
            if (*t > v) do --d, --t;
                while (*t > v);
            v -= *t;
            *e-- = d;
        } while (--i != 0);
}

smallmon madd1 (symmBlock glo, smallmon val, int *exp)
{
    int d, i, *e;
    smallmon v, s, *t;

    v = val;
    ASSERT("smallmon.c madd1",v <= glo->maxval);
    d = glo->degree;
    s = 0;
    i = glo->n1;
    e = exp + i;
    t = glo->cols[i] + d;
    if (*t <= v) {
        do ++d, ++t;
        while (*t <= v);
        glo->degree = d;
        --d, --t;
    }
    else if (--d, *--t > v) {
        do --d, --t;
        while (*t > v);
        glo->degree = d+1;
    }
    if (d + *e > glo->maxdeg) {
        prerror("; degree bound exceeded in monomial operation\n");
        to_shell();
    }
    v -= *t;
    s += t[*e--];
    if (i != 0) do {
            DECR(t,glo->over);
            if (*t > v) do --d, --t;
                while (*t > v);
            v -= *t;
            s += t[*e--];
        } while (--i != 0);
    return s;
}

void stob2 (symmBlock glo, smallmon val, int *exp)
{
    int d, *dg, *e;
    smallmon v, *t;
    int i;

    v = val;
    ASSERT("smallmon.c stob2",v <= glo->maxval);
    d = glo->degree;
    {
        offset n1 = glo->n1;
        dg = glo->degs + n1;
        e = exp + n1;
        t = glo->cols[n1] + d;
        i = n1;
    }
    if (*t <= v) {
        do ++d, ++t;
        while (*t <= v);
        glo->degree = d;
        --d, --t;
    }
    else if (--d, *--t > v) {
        do --d, --t;
        while (*t > v);
        glo->degree = d+1;
    }
    v -= *t;
    *e-- = d;
    if (i != 0) do {
            DECR(t,glo->over);
            if (*t > v) {
                offset up = *dg;
                do d-=up, t-=up;
                while (*t > v);
            }
            v -= *t;
            *e-- = d;
            --dg;
        } while (--i != 0);
}

smallmon madd2 (symmBlock glo, smallmon val, int *exp)
{
    int d, *dg, *e;
    smallmon v, *t;
    smallmon s;
    int i;

    v = val;
    ASSERT("smallmon.c madd2",v <= glo->maxval);
    d = glo->degree;
    s = 0;
    {
        offset n1 = glo->n1;
        dg = glo->degs + n1;
        e = exp + n1;
        t = glo->cols[n1] + d;
        i = n1;
    }
    if (*t <= v) {
        do ++d, ++t;
        while (*t <= v);
        glo->degree = d;
        --d, --t;
    }
    else if (--d, *--t > v) {
        do --d, --t;
        while (*t > v);
        glo->degree = d+1;
    }
    if (d + *e > glo->maxdeg) {
        prerror("; degree bound exceeded in monomial operation\n");
        to_shell();
    }
    v -= *t;
    s += t[*e--];
    if (i != 0) do {
            DECR(t,glo->over);
            if (*t > v) {
                offset up = *dg;
                do d-=up, t-=up;
                while (*t > v);
            }
            v -= *t;
            s += t[*e--];
            --dg;
        } while (--i != 0);
    return s;
}
