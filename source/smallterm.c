// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <setjmp.h> // For jmp_buf type required by parse.h
#include <limits.h> // For UINT_MAX
#include "shared.h"
#include "smallterm.h"
#include "mem.h"     // For memory management
#include "stash.h"   // For gimmy() and ungimmy()
#include "monitor.h" // For print() and prerror()
#include "shell.h"   // For to_shell()
#include "set.h"     // For maxdegree global variable

// Simple decrement operation - kept as direct code due to register variables

typedef long offset; // for register vars to be added to ptrs

symmBlock blockList; // list of all symmBlocks created

// findBlock is called to determine if any of the monomial tables
// so far constructed can be reused.  This is the case if the same number
// of variables, with the same number of weights already has been computed.
// HOWEVER, the top degree's must match up: otherwise a new table must
// be recomputed.

symmBlock findBlock(int n, int *degs)
{
    register symmBlock p = blockList;
    register int i;
    register boolean found;

    while (p != NULL)
    {
        if (p->defaultmax == maxdegree)
        {
            if (n == p->nvars)
            {
                found = true;
                for (i = 0; i < n; i++)
                    if (p->degs[i] != degs[i])
                    {
                        found = false;
                        break;
                    }
                if (found == true)
                    return (p);
            }
        }
        p = p->nextBlock;
    }
    return (NULL);
}

symmBlock symmCreate(int n, int *degs, char **varnames)
{
    symmBlock glo, p;
    smallmon *scaf, big, x, y;
    register smallmon **cols;
    int top, d;
    register int i, j, k;

    glo = (symmBlock)(void *)gimmy(sizeof(struct symblock));

    p = findBlock(n, degs);
    if (p != NULL)
    {
        *glo = *p;
        glo->names = varnames;
        glo->degree = 1;
        return (glo);
    }
    else
    {
        glo->nextBlock = blockList;
        blockList = glo;
    }
    glo->rtyp = RSYMM;
    glo->nvars = n;
    glo->degs = degs;
    glo->names = varnames;
    glo->defaultmax = maxdegree;

    glo->n1 = n - 1;
    glo->degree = 1;
    glo->w1 = TRUE;
    for (i = 0; i < n; i++)
        if (degs[i] != 1)
            glo->w1 = FALSE;
    if (glo->w1)
    {
        // Use union to convert function pointers safely
        union
        {
            void (*stob)(symmBlock, smallmon, int *);
            void (*generic)(void);
        } stob_cvt = {.stob = stob1};
        union
        {
            smallmon (*madd)(symmBlock, smallmon, int *);
            smallmon (*generic)(void);
        } madd_cvt = {.madd = madd1};
        glo->stob = stob_cvt.generic;
        glo->madd = madd_cvt.generic;
    }
    else
    {
        // Use union to convert function pointers safely
        union
        {
            void (*stob)(symmBlock, smallmon, int *);
            void (*generic)(void);
        } stob_cvt = {.stob = stob2};
        union
        {
            smallmon (*madd)(symmBlock, smallmon, int *);
            smallmon (*generic)(void);
        } madd_cvt = {.madd = madd2};
        glo->stob = stob_cvt.generic;
        glo->madd = madd_cvt.generic;
    }
    // Use union to convert function pointer safely
    union
    {
        smallmon (*btos)(symmBlock, int *);
        smallmon (*generic)(void);
    } btos_cvt = {.btos = btos};
    glo->btos = btos_cvt.generic;

    big = ~0UL;
    top = maxdegree + 1; // maxdegree = "set" variable
                         // Check for overflow before allocation
    size_t scaf_size = (size_t)top * sizeof(smallmon);
    if (scaf_size > UINT_MAX)
    {
        prerror("; smallterm: allocation size overflow\n");
        return NULL;
    }
    scaf = (smallmon *)(void *)gimmy((unsigned)scaf_size);
    for (k = 0; k < top; ++k)
        scaf[k] = 1;
    for (i = 0; i < n; ++i)
    {
        d = degs[i];
        for (j = 0; j < d; ++j)
        {
            x = scaf[j];
            for (k = j + d; k < top; k += d)
            {
                y = scaf[k];
                if (big - y <= x)
                    top = k;
                else
                {
                    x += y;
                    scaf[k] = x;
                }
            }
        }
    }
    glo->maxval = scaf[top - 1] - 1;
    ungimmy((char *)scaf);
    glo->maxdeg = top - 1;
    glo->over = top;

    // Check for overflow before allocation
    size_t table_size = (size_t)(1 + n * top) * sizeof(smallmon);
    size_t cols_size = (size_t)n * sizeof(smallmon *);
    if (table_size > UINT_MAX || cols_size > UINT_MAX)
    {
        prerror("; smallterm: allocation size overflow\n");
        return NULL;
    }
    glo->table = (smallmon *)(void *)gimmy((unsigned)table_size);
    cols = (smallmon **)(void *)gimmy((unsigned)cols_size);
    glo->cols = cols;
    for (i = 0; i < n; ++i)
        cols[i] = glo->table + i * top;
    d = degs[0];
    for (k = 1; k < top; ++k)
        cols[0][k] = 0;
    for (k = 0; k < top; k += d)
        cols[0][k] = 1;
    for (i = 1; i < n; ++i)
    {
        d = degs[i];
        for (j = 0; j < d; ++j)
        {
            x = cols[i - 1][j];
            cols[i][j] = x;
            for (k = j + d; k < top; k += d)
            {
                x += cols[i - 1][k];
                cols[i][k] = x;
            }
        }
    }
    for (i = 0; i < n; ++i)
    {
        d = degs[i + 1];
        for (j = 0; j < d; ++j)
        {
            x = 0;
            for (k = j; k < top; k += d)
            {
                y = cols[i][k];
                cols[i][k] = x;
                x += y;
            }
        }
    }
    cols[n - 1][top] = big;
    return (glo);
}

void prSymmBlock(symmBlock symm)
{
    int i, j;

    print("\nsymm block:\n");
    print("#vars = %d, w1=%d, maxdeg=%d, over=%d, n1=%d, degree=%d\n", symm->nvars, symm->w1,
          symm->maxdeg, symm->over, symm->n1, symm->degree);
    print("degrees = ");
    for (i = 0; i <= symm->nvars; i++)
        print("%d ", symm->degs[i]);
    print("\n");

    for (i = 0; i < symm->nvars; i++)
    {
        for (j = 0; j <= symm->maxdeg; j++)
            print("%lu ", symm->cols[i][j]);
        print("\n");
    }
    print("maxval = %lu\n", symm->maxval);
}

smallmon btos(symmBlock glo, int *exp)
{
    register smallmon v, **t;
    register int i, *e;
    register offset n1;

    v = 0;
    e = exp;
    t = glo->cols;
    n1 = glo->n1;
    if (e[n1] > glo->maxdeg)
    {
        prerror("; degree bound exceeded in monomial operation\n");
        to_shell();
    }
    for (i = 0; i <= n1; ++i)
        v += (*t++)[*e++];
    return v;
}

void stob1(symmBlock glo, smallmon val, int *exp)
{
    register int d, i, ov, *e;
    register smallmon v, *t;

    v = val;
    debug_assert("smallmon.c stob1: value exceeds maxval", v <= glo->maxval);
    d = glo->degree;
    i = glo->n1;
    ov = glo->over;
    e = exp + i;
    t = glo->cols[i] + d;
    if (*t <= v)
    {
        do
            ++d, ++t;
        while (*t <= v);
        glo->degree = d;
        --d, --t;
    }
    else if (--d, *--t > v)
    {
        do
            --d, --t;
        while (*t > v);
        glo->degree = d + 1;
    }
    v -= *t;
    *e-- = d;
    if (i != 0)
        do
        {
            t -= ov;
            if (*t > v)
                do
                    --d, --t;
                while (*t > v);
            v -= *t;
            *e-- = d;
        } while (--i != 0);
}

smallmon madd1(symmBlock glo, smallmon val, int *exp)
{
    register int d, i, *e;
    register smallmon v, s, *t;

    v = val;
    debug_assert("smallmon.c madd1: value exceeds maxval", v <= glo->maxval);
    d = glo->degree;
    s = 0;
    i = glo->n1;
    e = exp + i;
    t = glo->cols[i] + d;
    if (*t <= v)
    {
        do
            ++d, ++t;
        while (*t <= v);
        glo->degree = d;
        --d, --t;
    }
    else if (--d, *--t > v)
    {
        do
            --d, --t;
        while (*t > v);
        glo->degree = d + 1;
    }
    if (d + *e > glo->maxdeg)
    {
        prerror("; degree bound exceeded in monomial operation\n");
        to_shell();
    }
    v -= *t;
    s += t[*e--];
    if (i != 0)
        do
        {
            t -= glo->over;
            if (*t > v)
                do
                    --d, --t;
                while (*t > v);
            v -= *t;
            s += t[*e--];
        } while (--i != 0);
    return s;
}

void stob2(symmBlock glo, smallmon val, int *exp)
{
    register int d, *dg, *e;
    register smallmon v, *t;
    int i;

    v = val;
    debug_assert("smallmon.c stob2: value exceeds maxval", v <= glo->maxval);
    d = glo->degree;
    {
        register offset n1 = glo->n1;
        dg = glo->degs + n1;
        e = exp + n1;
        t = glo->cols[n1] + d;
        i = (int)n1;
    }
    if (*t <= v)
    {
        do
            ++d, ++t;
        while (*t <= v);
        glo->degree = d;
        --d, --t;
    }
    else if (--d, *--t > v)
    {
        do
            --d, --t;
        while (*t > v);
        glo->degree = d + 1;
    }
    v -= *t;
    *e-- = d;
    if (i != 0)
        do
        {
            t -= glo->over;
            if (*t > v)
            {
                register int up = *dg;
                do
                    d -= up, t -= up;
                while (*t > v);
            }
            v -= *t;
            *e-- = d;
            --dg;
        } while (--i != 0);
}

smallmon madd2(symmBlock glo, smallmon val, int *exp)
{
    register int d, *dg, *e;
    register smallmon v, *t;
    smallmon s;
    int i;

    v = val;
    debug_assert("smallmon.c madd2: value exceeds maxval", v <= glo->maxval);
    d = glo->degree;
    s = 0;
    {
        register offset n1 = glo->n1;
        dg = glo->degs + n1;
        e = exp + n1;
        t = glo->cols[n1] + d;
        i = (int)n1;
    }
    if (*t <= v)
    {
        do
            ++d, ++t;
        while (*t <= v);
        glo->degree = d;
        --d, --t;
    }
    else if (--d, *--t > v)
    {
        do
            --d, --t;
        while (*t > v);
        glo->degree = d + 1;
    }
    if (d + *e > glo->maxdeg)
    {
        prerror("; degree bound exceeded in monomial operation\n");
        to_shell();
    }
    v -= *t;
    s += t[*e--];
    if (i != 0)
        do
        {
            t -= glo->over;
            if (*t > v)
            {
                register int up = *dg;
                do
                    d -= up, t -= up;
                while (*t > v);
            }
            v -= *t;
            s += t[*e--];
            --dg;
        } while (--i != 0);
    return s;
}
