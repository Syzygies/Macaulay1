// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <stdio.h>   // For FILE type
#include <string.h>  // For strlen
#include "shared.h"
#include "term.h"
#include "monitor.h"   // For print(), fprint()
#include "human_io.h"  // For int_pprint(), int_size()
#include "ring.h"      // For blocks, nblocks, compLoc, numvars, varnames, rgDegs

// Suppress pedantic warnings about function pointer conversions
// These conversions were valid in the original K&R C and work correctly
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

// Different formats for monomials

// 1. term (smallterm)
// This is the encoded form of a monomial, length in ints is
// nblocks (for the given ring), composed of three types of slots
// - encoded monomials
// - component (<= 1 per monom)
// - weight function values ( >= 0 per monom)
// The interpretation always comes from the current ring.

// 2. big
// Used in monomial multiplication as one of the parameters.
// Consists of a sequence of ints, broken into slots:
// - component
// - weight function value
// - a set of integers for a given variable block, decoded into
// partial sum format. (i.e. output of stob in smallterm.c)

void prSmall(term t)
{
    int i;

    for (i = 0; i < nblocks; i++)
        print("%ld ", t[i]);
    print("\n");
}

void prBig(register bigterm big)
{
    register symmBlock* sy = blocks;
    register symmBlock this = *sy;
    register int i;

    do
    {
        if (this->rtyp == RSYMM)
        {
            for (i = 0; i < this->nvars; i++)
                print("%lu ", *big++);
        }
        else // either RCOMP or RWTFCN
            print("%ld ", *big++);
        sy++;
        this = *sy;
    } while (this != NULL);
    print("\n");
}

void prExp(register bigterm exp)
{
    register int i;

    for (i = 0; i < numvars; i++)
        print("%d ", exp[i]);
    print("\n");
}

bigterm helpexp;

void expToS(bigterm sexp, int component, register term result)
{
    register symmBlock* sy = blocks;
    register symmBlock this = *sy;
    register int* exp = sexp;
    register int i;
    int n, * deg;

    do
    {
        if (this->rtyp == RSYMM)
        {
            n = this->nvars;
            deg = this->degs;
            helpexp[0] = (*exp++) * (*deg++);
            for (i = 1; i < n; i++)
                helpexp[i] = (*exp++) * (*deg++) + helpexp[i - 1];
            {
                smallmon (*func)(symmBlock, int*) =
                    (smallmon (*)(symmBlock, int*))(void*) this->btos;
                *result = func(this, helpexp);
            }
        }
        else if (this->rtyp == RCOMP)
            *result = (smallmon)component;
        else
        {
            register int sum = 0;
            deg = this->degs;
            for (i = numvars - 1; i >= 0; i--)
                sum += deg[i] * sexp[i];
            *result = (smallmon)sum;
        }
        result++;
        sy++;
        this = *sy;
    } while (this != NULL);
}

void sToExp(term s, bigterm exp)
{
    register symmBlock* sy = blocks;
    register symmBlock this = *sy;
    register int i, * deg, n;

    do
    {
        if (this->rtyp == RSYMM)
        {
            n = this->nvars;
            deg = this->degs;
            {
                void (*func)(symmBlock, smallmon, int*) =
                    (void (*)(symmBlock, smallmon, int*))(void*) this->stob;
                func(this, *s, exp);
            }
            for (i = n - 1; i >= 1; i--)
            {
                exp[i] -= exp[i - 1];
                exp[i] /= deg[i];
            }
            exp[0] /= deg[0];
            exp += n;
        }
        sy++;
        s++;
        this = *sy;
    } while (this != NULL);
}

void sToBig(register term t, register bigterm big)
{
    register symmBlock* sy = blocks;
    register symmBlock this = *sy;

    do
    {
        if (this->rtyp == RSYMM)
        {
            {
                void (*func)(symmBlock, smallmon, int*) =
                    (void (*)(symmBlock, smallmon, int*))(void*) this->stob;
                func(this, *t, big);
            }
            big += this->nvars;
        }
        else // either RCOMP or RWTFCN
            *big++ = (int)*t;
        sy++;
        t++;
        this = *sy;
    } while (this != NULL);
}

void bigToS(register bigterm big, register term t)
{
    register symmBlock* sy = blocks;
    register symmBlock this = *sy;

    do
    {
        if (this->rtyp == RSYMM)
        {
            {
                smallmon (*func)(symmBlock, int*) =
                    (smallmon (*)(symmBlock, int*))(void*) this->btos;
                *t = func(this, big);
            }
            big += this->nvars;
        }
        else // either RCOMP or RWTFCN
            *big++ = (int)*t;
        sy++;
        t++;
        this = *sy;
    } while (this != NULL);
}

void hilbToS(bigterm hexp, int component, register term result)
{
    register symmBlock* sy = blocks;
    register symmBlock this = *sy;
    register int* exp = hexp;
    register int i, j;
    int n, * deg;

    do
    {
        if (this->rtyp == RSYMM)
        {
            n = this->nvars;
            helpexp[0] = exp[n - 1];
            for (j = 1, i = n - 2; i >= 0; i--, j++)
                helpexp[j] = exp[i] + helpexp[j - 1];
            {
                smallmon (*func)(symmBlock, int*) =
                    (smallmon (*)(symmBlock, int*))(void*) this->btos;
                *result = func(this, helpexp);
            }
        }
        else if (this->rtyp == RCOMP)
            *result = (smallmon)component;
        else
        {
            register int sum = 0;
            deg = this->degs;
            for (i = numvars - 1, j = 0; i >= 0; i--, j++)
                sum += deg[j] * hexp[i];
            *result = (smallmon)sum;
        }
        result++;
        sy++;
        this = *sy;
    } while (this != NULL);
}

void sToHilb(term s, bigterm hexp, int* component)
{
    register symmBlock* sy = blocks;
    register symmBlock this = *sy;
    register int i, j, n;
    bigterm
        big; // Note: bigterm is typedef'd as int[NVARS], so this allocates an array on the stack

    do
    {
        if (this->rtyp == RSYMM)
        {
            n = this->nvars;
            {
                void (*func)(symmBlock, smallmon, int*) =
                    (void (*)(symmBlock, smallmon, int*))(void*) this->stob;
                func(this, *s, big);
            }
            hexp[n - 1] = big[0];
            for (j = n - 2, i = 1; j >= 0; i++, j--)
                hexp[j] = big[i] - big[i - 1];
            hexp += n;
        }
        else if (this->rtyp == RCOMP)
            *component = (int)*s;
        sy++;
        s++;
        this = *sy;
    } while (this != NULL);
}

void tm_pprint(FILE* fil, term t)
{
    register int i, a;
    bigterm exp;

    sToExp(t, exp);
    for (i = 0; i < numvars; i++)
    {
        a = exp[i];
        if (a > 0)
            fprint(fil, "%s", varnames[i]);
        if (a > 1)
            int_pprint(fil, a, false, false);
    }
}

int tm_size(term t)
{
    register int i, a, sum;
    bigterm exp;

    sum = 0;
    sToExp(t, exp);
    for (i = 0; i < numvars; i++)
    {
        a = exp[i];
        if (a > 0)
            sum += (int)strlen(varnames[i]);
        if (a > 1)
            sum += int_size(a, false, false);
    }
    return (sum);
}

void tm_add(register term s, register bigterm big, register term result)
{
    register symmBlock* sy = blocks;
    register symmBlock this = *sy;

    do
    {
        if (this->rtyp == RSYMM)
        {
            {
                smallmon (*func)(symmBlock, smallmon, int*) =
                    (smallmon (*)(symmBlock, smallmon, int*))(void*) this->madd;
                *result = func(this, *s, big);
            }
            big += this->nvars;
        }
        else if (this->rtyp == RCOMP)
        {
            *result = (smallmon)max_int((int)*s, *big);
            big++;
        }
        else // WTFCN
            *result = (smallmon)((*big++) + (int)(*s));
        sy++;
        s++;
        result++;
        this = *sy;
    } while (this != NULL);
}

int tm_compare(register term s, register term t)
{
    register int n = nblocks;
    register int i;

    for (i = n; i > 0; i--, s++, t++)
        if (*s > *t)
            return (GT);
        else if (*s < *t)
            return (LT);
    return (EQ);
}

// simple routines

void term_i(int i, term t)
{
    register int j;

    for (j = 0; j < nblocks; j++)
        t[j] = 0;
    if (compLoc >= 0)
        t[compLoc] = (smallmon)i;
}

void tm_xjei(int j, int i, term t)
{
    bigterm exp;
    register int a;

    for (a = 0; a < numvars; a++)
        exp[a] = 0;
    exp[j] = 1;
    expToS(exp, i, t);
}

void tm_copy(term s, term t)
{
    register int i;

    for (i = nblocks; i > 0; i--)
        *t++ = *s++;
}

int tm_component(term t)
{
    if (compLoc >= 0)
        return ((int)t[compLoc]);
    else
        return (1);
}

void tm_setcomp(term t, int c)
{
    if (compLoc >= 0)
        t[compLoc] = (smallmon)c;
}

boolean tm_iszero(term t)
{
    register int i;

    for (i = 0; i < nblocks; i++)
        if ((*t++ != 0) && (compLoc != i))
            return (FALSE);
    return (TRUE);
}

int tm_degree(term t)
{
    bigterm exp;
    register int i, sum;

    sToExp(t, exp);
    sum = 0;
    for (i = 0; i < numvars; i++)
        sum += exp[i] * dlist_ref(&rgDegs, i + 1);
    return (sum);
}

int exp_degree(expterm exp)
{
    register int sum, i;

    sum = 0;
    for (i = 0; i < numvars; i++)
        sum += exp[i] * dlist_ref(&rgDegs, i + 1);
    return (sum);
}

// joins

// joinminus computes the join, J,  of t1 and t2, and returns
// result1 := J - t1
// result2 := J - t2
// the component of resulti is set to ci (i=1,2)

void joinminus(term t1, term t2, int c1, int c2, term result1, term result2)
{
    bigterm exp1, exp2;
    register int i, d;

    sToExp(t1, exp1);
    sToExp(t2, exp2);
    for (i = numvars - 1; i >= 0; i--)
    {
        d = max_int(exp1[i], exp2[i]);
        exp1[i] = d - exp1[i];
        exp2[i] = d - exp2[i];
    }
    expToS(exp1, c1, result1);
    expToS(exp2, c2, result2);
}

// tm_joinminus is same as joinminus, except the components of resulti
// are set to 0.

void tm_joinminus(term t1, term t2, term result1, term result2)
{
    joinminus(t1, t2, 0, 0, result1, result2);
}

// tm_rg_join is same as joinminus, except t1 is thought of as in
// the ring, while t2 is in a module.

void tm_rg_join(term t1, term t2, term result1, term result2)
{
    joinminus(t1, t2, tm_component(t2), 0, result1, result2);
}

// division

// tm_divides returns TRUE if t divides s (ignoring components).
// In this case, result := s - t, with component = c.

boolean tm_divides(term s, term t, int c, term result)
{
    expterm bs, bt;
    register int i, a;

    sToExp(s, bs);
    sToExp(t, bt);
    for (i = 0; i < numvars; i++)
    {
        a = bs[i] - bt[i];
        if (a < 0)
            return (FALSE);
        else
            bs[i] = a;
    }
    expToS(bs, c, result);
    return (TRUE);
}

// other monomial operations

// tm_diff differentiates s by the variable v, with the result going
// into result, and the original exponent of the variable v, into power

void tm_diff(term s, term result, int v, int* power)
{
    bigterm exp;

    sToExp(s, exp);
    *power = exp[v];
    if (*power == 0)
        return;
    exp[v]--;
    expToS(exp, tm_component(s), result);
}

// tm_contract also differentiates: monomial "s" by monomial "x".
// result goes into "result", and the product of the powers goes into
// "coefval".

void tm_contract(term x, term s, term result, int* coefval)
{
    bigterm expx;
    bigterm exps;
    int i, j, a;

    sToExp(s, exps);
    sToExp(x, expx);
    *coefval = 1;
    for (i = 0; i < numvars; i++)
    {
        if (expx[i] > exps[i])
        {
            *coefval = 0;
            return;
        }
        for (j = 1; j <= expx[i]; j++)
        {
            a = exps[i];
            (*coefval) *= a;
            exps[i]--;
        }
    }
    expToS(exps, tm_component(s), result);
}

// in_subring returns TRUE if the first "i" monomial blocks, not including
// components, are 0

boolean in_subring(term t, int i)
{
    register int j;

    if ((compLoc >= 0) && (compLoc < i))
        i++;
    if (i > nblocks)
        i = nblocks;
    for (j = 0; j < i; j++)
        if ((compLoc != j) && (t[j] != 0))
            return (FALSE);
    return (TRUE);
}

// tm_ineq returns TRUE if "s" and "t" have the same value on blocks
// 1..i, otherwise FALSE is returned.  This routine is used to compute
// initial ideals and submodules for possibly not strict monomial orders.

boolean tm_inEq(term s, term t, int i)
{
    register int j;

    if (i > nblocks)
        i = nblocks;
    for (j = 0; j < i; j++)
        if (t[j] != s[j])
            return (FALSE);
    return (TRUE);
}

// this routine sets the first n elements of exp to 1, where n =
// number of variables in the first block

void getfirstblock(expterm exp)
{
    symmBlock* sy = blocks;
    symmBlock this = *sy;
    int i;

    do
    {
        if (this->rtyp == RSYMM)
        {
            for (i = 0; i < this->nvars; i++)
                exp[i] = 1;
            return;
        }
        this = *sy++;
    } while (this != NULL);
}

// Re-enable pedantic warnings
#pragma GCC diagnostic pop
