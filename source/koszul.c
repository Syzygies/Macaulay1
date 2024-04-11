/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"

// boolean member (int e, int p, int *t);
// boolean subset (int p1, int *t1, int p2, int *t2);
// void kremove (int p, int *t, int i, int *tresult);
// int binom (int n, int p);
// int loc (int p, int n, int *t);
// int kz_getdeg (gmatrix M, int p, int *tcol);
// void kz_setrowdegs (dlist *dl, gmatrix M, int n, int p);
// poly kosz_row (gmatrix M, int p, int n, int *tcol);
// gmatrix gm_koszul (gmatrix M, int n, int p);
// void koszul_cmd (int argc, char *argv[]);

extern poly p_xjei();
extern poly extr1();

boolean member (int e, int p, int *t)
/* is "e" in the p-subset "t"? */
{
    int i;

    for (i=0; i<p; i++)
        if (e == t[i])
            return(TRUE);
    return(FALSE);
}

/* subset determines whether the p1-subset t1 is contained in the
    p2-subset t2 */
boolean subset (int p1, int *t1, int p2, int *t2)
{
    int i;

    if (p1 > p2) return(FALSE);
    for (i=0; i<p1; i++)
        if (!member(t1[i], p2, t2))
            return(FALSE);
    return(TRUE);
}

/* kremove removes the ith entry of the p-subset t, (i = 1..p)
    and puts the resulting (p-1)-subset into tresult */
void kremove (int p, int *t, int i, int *tresult)
{
    int j;

    tresult++;
    for (j=1; j<i; j++)
        *tresult++ = t[j];
    for (j=i+1; j<=p; j++)
        *tresult++ = t[j];
}

int binom (int n, int p)
/* returns n choose p */
{
    int i, result;

    result = 1;
    for (i=0; i<p; i++)
        result = result*(n-i)/(i+1);
    return(result);
}

int loc (int p, int n, int *t)
{
    int sum, i;

    if (p == 0) return(0);
    sum = 0;
    t++;
    for (i=1; i<=n; i++)
        if (*t == i) {
            t++;
            p--;
            if (p == 0) return(sum);
        } else {
            sum += binom(n-i, p-1);
        }
    return(-237);  /* should never get here */
}

/* kz_getdeg -- adds the degrees of each column of M in tcol.
 * If M is NULL, the degrees of the variables in the current ring are used */

int kz_getdeg (gmatrix M, int p, int *tcol)
/* tcol is an array 1..p */
{
    int sum, i;

    sum = 0;
    if (M != NULL) {
        for (i=1; i<=p; i++)
            sum += DREF(M->deggens, tcol[i]);
    } else {
        for (i=1; i<=p; i++)
            sum += varWeight(VAR_RING(current_ring), tcol[i]-1);
    }
    return(sum);
}

/* kz_setrowdegs -- sets the row degrees of the Koszul matrix correctly.
 * added -- 1/2/91 MES
 */

void kz_setrowdegs (dlist *dl, gmatrix M, int n, int p)
{
    int tcol[100];
    int acol, junk;
    int sum;

    dl_init(dl);
    pinit_array(p-1, n, tcol, &acol, &junk);
    do {
        sum = kz_getdeg(M,p-1,tcol);
        dl_insert(dl, sum);
    } while (pincr_array(p-1,n,tcol,&acol,&junk));
}

poly kosz_row (gmatrix M, int p, int n, int *tcol)
/* if NULL, use first "n" vars in current ring */
{
    poly result, f;
    int sign, j, m;
    int tresult[100];

    result = NULL;
    sign = 1;
    for (j=p; j>0; j--) {
        kremove(p,tcol,j,tresult);
        m = 1 + loc(p-1,n,tresult);
        if (M IS NULL)
            f = p_xjei(tcol[j]-1, m);
        else
            f = extr1(PREF(M->gens, tcol[j]), 1, m);
        if (sign == -1)
            p_sub(&result, &f);
        else
            p_add(&result, &f);
        sign = -sign;
    }
    return(result);
}

gmatrix gm_koszul (gmatrix M, int n, int p)
/* if NULL, use instead first "n" vars in current ring */
{
    int ncols, sum, acol, junk;
    int tcol[100];
    gmatrix result;
    poly f;

    result = mod_init();
    if ((p <= 0) OR (p > n)) return(result);
    kz_setrowdegs(&result->degrees, M, n, p);
    ncols = binom(n, p);
    pinit_array(p,n,tcol,&acol, &junk);
    do {
        f = kosz_row(M, p,n,tcol);
        sum = kz_getdeg(M,p,tcol);
        pl_insert(&result->gens, f);
        dl_insert(&result->deggens, sum);
    } while (pincr_array(p,n,tcol,&acol, &junk));
    return(result);
}

void koszul_cmd (int argc, char *argv[])
{
    int p, n;
    variable *result, *q;
    gmatrix M;
    char ident[100], *t;

    if (argc != 4) {
        printnew("koszul <int n, or matrix> <p> <result matrix>\n");
        return;
    }
    p = getInt(argv[2]);

    /* check to see whether first arg is matrix */
    /* this code is junk -- should be elsewhere */

    if (canStartVar(argv[1][0]) ISNT NULL) {
        t = argv[1];
        getIdentifier(&t, ident);
        q = find_var(ident);
        if (q IS NULL) return;
        if (q->b_alias ISNT NULL)
            q = q->b_alias;
        if (is_a_module(q)) {
            vrg_install(q->b_ring);
            M = VAR_MODULE(q);
            n = ncols(M);
            NEW_MOD(result, 3);
            set_value(result, gm_koszul(M, n, p));
            return;
        }
    }

    /* at this point, first argument should be an integer */

    M = NULL;
    n = getInt(argv[1]);
    n = MIN(n, numvars);
    NEW_MOD(result, 3);
    set_value(result, gm_koszul(M, n, p));
}
