// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "det.h"
#include "gm_poly.h"
#include "poly.h"
#include "charp.h"
#include "mac.h"
#include "monitor.h"
#include "set.h"

// pincr_array:  takes an array t[1..p], which is filled with
// integers in 1..n, so that  t[1] < t[2] < ... < t[p].  The integer
// a = max { i : t[i] < n+i-p }  (largest index which can be incremented)
// It then modifies t, a  by "incrementing" the array to the next
// possible fillin of  t  with  1..n.
// If no more exist, false is returned, else true is returned.
// Also: "sign" is changed to reflect the sign of the permutation.

boolean pincr_array(int p, int n, int t[], int* a, int* sign)
{
    register int i;
    register int aa = *a;

    if (aa == 0)
        return false;
    t[aa]++;
    if (t[aa] == n - p + aa)
    {
        aa--;
        *sign = -(*sign);
    }
    else
    {
        for (i = 1; i <= p - aa; i++)
            t[aa + i] = t[aa] + i;

        // Modified 7/14/94 John P. Dalbec
        // The code up to this point has changed the values of
        // (t[aa],t[aa+1],...,t[p]) from (t[aa]-1,n-(p-aa)+1,...,n)
        // to (t[aa],t[aa]+1,...,t[aa]+p-aa).  This change decreases the sum
        // (t[1]+t[2]+...+t[p]) by ((p-aa)((n-t[aa])-(p-aa))-1),
        // so the sign of the permutation changes by the same amount (mod 2).

        if ((((p - aa) % 2) == 0) || (((n - t[aa]) % 2) == 1))
            *sign = -(*sign);
        aa = p;
    }
    *a = aa;
    return true;
}

void pinit_array(int p, int n, int t[], int* a, int* sign)
{
    register int i;

    *sign = 1;
    for (i = 1; i <= p; i++)
        t[i] = i;
    if (p == n)
        *a = 0;
    else
        *a = p;
}

void parr_pr(int p, int t[], int a)
{
    register int i;

    newline();
    for (i = 1; i <= p; i++)
        print("%d	", t[i]);
    print("		a = %d\n", a);
}

// dl_wedge
// Computes all the p th sums of "dl", putting these into "result".

void dl_wedge(int p, dlist* dl, dlist* result)
{
    int t[NDET];
    int a, i, sum, sign;

    pinit_array(p, length(dl), t, &a, &sign);
    do
    {
        sum = 0;
        for (i = 1; i <= p; i++)
            sum += dlist_ref(dl, t[i]);
        dlist_insert(result, sum);
    } while (pincr_array(p, length(dl), t, &a, &sign));
}

void pl_wedge(int p, int nrows, int ncols, plist* pl, plist* result)
{
    int a, b, r, rowsign, colsign;
    poly f, thiscol;
    int rows[NDET], cols[NDET];
    detmat MAT;
    boolean isintr;

    pinit_array(p, ncols, cols, &a, &colsign);
    isintr = false;
    do
    {
        pinit_array(p, nrows, rows, &b, &rowsign);
        r = 1;
        thiscol = NULL;
        do
        {
            if (have_intr())
                isintr = true;
            if (!isintr)
            {
                if (verbose > 0)
                    prflush(".");
                todetmat(p, pl, rows, cols, MAT);
                f = det(MAT, p, colsign * rowsign, r);
                p_add(&thiscol, &f);
                r++;
            }
        } while (pincr_array(p, nrows, rows, &b, &rowsign));
        plist_insert(result, thiscol);
    } while (pincr_array(p, ncols, cols, &a, &colsign));
}

void todetmat(int p, plist* pl, int rows[], int cols[], detmat MAT)
{
    register int r, c;

    for (r = 1; r <= p; r++)
        for (c = 1; c <= p; c++)
            MAT[r][c] = extr1(plist_ref(pl, cols[c]), rows[r], 0);
}

void get_pivot(detmat MAT, int p, int* colpivot, poly* pivot)
{
    register int c;

    for (c = 1; c <= p; c++)
        if (MAT[p][c] != NULL)
        {
            *colpivot = c;
            *pivot = MAT[p][c];
            return;
        }
    *pivot = NULL;
}

// detmult --  computes f1.g1 - f2.g2 div (divpoly), and frees
// g1.  For the reasoning of this, see "gauss".

poly detmult(poly f1, poly g1, poly f2, poly g2, poly divpoly)
{
    poly a, b;

    a = p_mult(f1, g1);
    b = p_mult(f2, g2);
    p_sub(&a, &b);
    if (divpoly != NULL)
        a = p_div(&a, divpoly);
    p_kill(&g1);
    return a;
}

void gauss(detmat MAT, int r, int p, int colpivot, poly pivot, poly lastpivot)
{
    register int c;
    poly f;

    f = MAT[r][colpivot];
    for (c = 1; c < colpivot; c++)
        MAT[r][c] = detmult(pivot, MAT[r][c], f, MAT[p][c], lastpivot);
    for (c = colpivot + 1; c <= p; c++)
        MAT[r][c - 1] = detmult(pivot, MAT[r][c], f, MAT[p][c], lastpivot);
    p_kill(&f);
}

poly det(detmat MAT, int size, int sign, int comp)
{
    poly pivot, lastpivot;
    poly result;
    int p, colpivot, r, c;

    pivot = NULL;
    lastpivot = NULL;
    for (p = size; p >= 2; p--)
    {
        p_kill(&lastpivot);
        lastpivot = pivot;
        get_pivot(MAT, p, &colpivot, &pivot);
        if (pivot == NULL)
        {
            for (r = 1; r <= p; r++)
                for (c = 1; c <= p; c++)
                    p_kill(&(MAT[r][c]));
            p_kill(&lastpivot);
            return NULL;
        }

        for (r = 1; r < p; r++)
            gauss(MAT, r, p, colpivot, pivot, lastpivot);

        if (((p + colpivot) % 2) == 1) // p+colpivot is odd
            sign = -sign;
        for (c = 1; c <= p; c++)
            if (c != colpivot)
                p_kill(&(MAT[p][c]));
    }
    p_kill(&pivot);
    p_kill(&lastpivot);
    // at this point MAT[1][1] contains the det, up to sign
    result = MAT[1][1];
    while (result != NULL)
    {
        set_comp(result, comp);
        if (sign == -1)
            fd_negate(&(result->coef));
        result = result->next;
    }
    return MAT[1][1];
}

// pfaffians

poly pfaff4(gmatrix g, int t[])
{
    poly f1, f2, result, h;

    f1 = extr1(plist_ref(&(g->gens), t[2]), t[1], 1);
    f2 = extr1(plist_ref(&(g->gens), t[4]), t[3], 1);
    result = p_mult(f1, f2);
    p_kill(&f1);
    p_kill(&f2);
    f1 = extr1(plist_ref(&(g->gens), t[3]), t[1], 1);
    f2 = extr1(plist_ref(&(g->gens), t[4]), t[2], 1);
    h = p_mult(f1, f2);
    p_sub(&result, &h);
    p_kill(&f1);
    p_kill(&f2);
    f1 = extr1(plist_ref(&(g->gens), t[3]), t[2], 1);
    f2 = extr1(plist_ref(&(g->gens), t[4]), t[1], 1);
    h = p_mult(f1, f2);
    p_add(&result, &h);
    p_kill(&f1);
    p_kill(&f2);
    return result;
}
