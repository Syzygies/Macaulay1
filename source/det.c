/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "mtypes.h"

#define NDET    15
typedef poly detmat[NDET][NDET];

// boolean pincr_array (int p, int n, int t[], int *a, int *sign);
// void pinit_array (int p, int n, int t[], int *a, int *sign);
// void parr_pr (int p, int t[], int a);
// void dl_wedge (int p, dlist *dl, dlist *result);
// void pl_wedge (int p, int nrows, int ncols, plist *pl, plist *result);
void todetmat (int p, plist *pl, int rows[], int cols[], detmat MAT);
// void get_pivot (detmat MAT, int p, int *colpivot, poly *pivot);
// poly detmult (poly f1, poly g1, poly f2, poly g2, poly divpoly);
// void gauss (detmat MAT, int r, int p, int colpivot, poly pivot, poly lastpivot);
// poly det (detmat MAT, int size, int sign, int comp);
// poly pfaff4 (gmatrix g, int t[]);

extern poly extr1();
extern poly det();
extern poly p_div();

/*
 *  pincr_array:  takes an array t[1..p], which is filled with
 * integers in 1..n, so that  t[1] < t[2] < ... < t[p].  The integer
 * a = max { i : t[i] < n+i-p }  (largest index which can be incremented)
 * It then modifies t, a  by "incrementing" the array to the next
 * possible fillin of  t  with  1..n.
 *  If no more exist, FALSE is returned, else TRUE is returned.
 * Also: "sign" is changed to reflect the sign of the permutation.
 */

boolean pincr_array (int p, int n, int t[], int *a, int *sign)
{
    int i;
    int aa = *a;

    if (aa IS 0) return(FALSE);
    t[aa]++;
    if (t[aa] IS n-p+aa) {
        aa--;
        *sign = - (*sign);
    } else {
        for (i=1; i<=p-aa; i++)
            t[aa+i] = t[aa] + i;

        /* Modified 7/14/94 John P. Dalbec */
        /* The code up to this point has changed the values of */
        /* (t[aa],t[aa+1],...,t[p]) from (t[aa]-1,n-(p-aa)+1,...,n) */
        /* to (t[aa],t[aa]+1,...,t[aa]+p-aa).  This change decreases the sum */
        /* (t[1]+t[2]+...+t[p]) by ((p-aa)((n-t[aa])-(p-aa))-1), */
        /* so the sign of the permutation changes by the same amount (mod 2). */

        if ((((p-aa) % 2) IS 0) OR (((n-t[aa]) % 2) IS 1))
            *sign = - (*sign);
        aa = p;
    }
    *a = aa;
    return(TRUE);
}

void pinit_array (int p, int n, int t[], int *a, int *sign)
{
    int i;

    *sign = 1;
    for (i=1; i<=p; i++)
        t[i] = i;
    if (p IS n)
        *a = 0;
    else *a = p;
}

void parr_pr (int p, int t[], int a)
{
    int i;

    newline();
    for (i=1; i<=p; i++)
        print("%d	",t[i]);
    print("		a = %d\n", a);
}

/*
 *      dl_wedge
 *  Computes all the p th sums of "dl", putting these into "result".
 */

void dl_wedge (int p, dlist *dl, dlist *result)
{
    int t[NDET];
    int a, i, sum, sign;

    pinit_array(p, length(dl), t, &a, &sign);
    do {
        sum = 0;
        for (i=1; i<=p; i++)
            sum += DREF(*dl, t[i]);
        dl_insert(result, sum);
    } while (pincr_array(p, length(dl), t, &a, &sign));
}

void pl_wedge (int p, int nrows, int ncols, plist *pl, plist *result)
{
    int a, b, r, rowsign, colsign;
    poly f, thiscol;
    int rows[NDET], cols[NDET];
    detmat MAT;
    boolean isintr;

    pinit_array(p, ncols, cols, &a, &colsign);
    isintr = FALSE;
    do {
        pinit_array(p, nrows, rows, &b, &rowsign);
        r = 1;
        thiscol = NULL;
        do {
            if (have_intr())
                isintr = TRUE;
            if (!isintr) {
                if (verbose > 0) prflush(".");
                todetmat(p, pl, rows, cols, MAT);
                f = det(MAT, p, colsign*rowsign, r);
                p_add(&thiscol, &f);
                r++;
            }
        } while (pincr_array(p, nrows, rows, &b, &rowsign));
        pl_insert(result, thiscol);
    } while (pincr_array(p, ncols, cols, &a, &colsign));
}

void todetmat (int p, plist *pl, int rows[], int cols[], detmat MAT)
/* size of result */
{
    int r, c;

    for (r=1; r<=p; r++)
        for (c=1; c<=p; c++)
            MAT[r][c] = extr1(PREF(*pl, cols[c]), rows[r], 0);
}

void get_pivot (detmat MAT, int p, int *colpivot, poly *pivot)
/* column where nonzero pivot elem in row p is found*/
/* the pivot elem. itself, in person */
{
    int c;

    for (c=1; c<=p; c++)
        if (MAT[p][c] ISNT NULL) {
            *colpivot = c;
            *pivot = MAT[p][c];
            return;
        }
    *pivot = NULL;
}

/*
 * detmult --  computes f1.g1 - f2.g2 div (divpoly), and frees
 *  g1.  For the reasoning of this, see "gauss".
 */

poly detmult (poly f1, poly g1, poly f2, poly g2, poly divpoly)
/* acts as pivot */
{
    poly a, b;

    a = p_mult(f1, g1);
    b = p_mult(f2, g2);
    p_sub(&a, &b);
    if (divpoly ISNT NULL)
        a = p_div(&a, divpoly);
    p_kill(&g1);
    return(a);
}

void gauss (detmat MAT, int r, int p, int colpivot, poly pivot, poly lastpivot)
/* row to modify */
/* size of matrix, and also pivot row */
/* column of pivot element */
{
    int c;
    poly f;

    f = MAT[r][colpivot];
    for (c=1; c<colpivot; c++)
        MAT[r][c] = detmult(pivot, MAT[r][c], f, MAT[p][c], lastpivot);
    for (c=colpivot+1; c<=p; c++)
        MAT[r][c-1] = detmult(pivot, MAT[r][c], f, MAT[p][c], lastpivot);
    p_kill(&f);
}

poly det (detmat MAT, int size, int sign, int comp)
/* MAT is a size by size matrix of polynomials */
/* multiply resulting det. by sign = 1, or -1 */
/* put every component of result to be "comp" */
{
    poly pivot, lastpivot;
    poly result;
    int p, colpivot, r, c;

    pivot = NULL;
    lastpivot = NULL;
    for (p=size; p>=2; p--) {
        p_kill(&lastpivot);
        lastpivot = pivot;
        get_pivot(MAT, p, &colpivot, &pivot);
        if (pivot IS NULL) {
            for (r=1; r<=p; r++)
                for (c=1; c<=p; c++)
                    p_kill(&(MAT[r][c]));
            p_kill(&lastpivot);
            return(NULL);
        }

        for (r=1; r<p; r++)
            gauss(MAT, r, p, colpivot, pivot, lastpivot);

        if (((p + colpivot) % 2) IS 1)  /* p+colpivot is odd */
            sign = -sign;
        for (c=1; c<=p; c++)
            if (c ISNT colpivot)
                p_kill(&(MAT[p][c]));
    }
    p_kill(&pivot);
    p_kill(&lastpivot);
    /* at this point MAT[1][1] contains the det, up to sign */
    result = MAT[1][1];
    while (result ISNT NULL) {
        set_comp(result, comp);
        if (sign IS -1) fd_negate(&(result->coef));
        result = result->next;
    }
    return(MAT[1][1]);
}

/*--------- pfaffians ---------------------------------*/

poly pfaff4 (gmatrix g, int t[])
/* size at least 1..4 */
{
    poly f1, f2, result, h;

    f1 = extr1(PREF(g->gens, t[2]), t[1], 1);
    f2 = extr1(PREF(g->gens, t[4]), t[3], 1);
    result = p_mult(f1, f2);
    p_kill(&f1);
    p_kill(&f2);
    f1 = extr1(PREF(g->gens, t[3]), t[1], 1);
    f2 = extr1(PREF(g->gens, t[4]), t[2], 1);
    h = p_mult(f1, f2);
    p_sub(&result, &h);
    p_kill(&f1);
    p_kill(&f2);
    f1 = extr1(PREF(g->gens, t[3]), t[2], 1);
    f2 = extr1(PREF(g->gens, t[4]), t[1], 1);
    h = p_mult(f1, f2);
    p_add(&result, &h);
    p_kill(&f1);
    p_kill(&f2);
    return(result);
}
