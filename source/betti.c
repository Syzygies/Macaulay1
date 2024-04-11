/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"
#include "ddefs.h"

// int dl_ndeg (dlist *dl, int deg);
// void resLoHi (int v, int *ResLength, int *lo, int *hi, int betti[]);
// void resDeg (int v, int deg, int betti[]);
// void betti (FILE *fil, variable *p);
// void betti_cmd (int argc, char *argv[]);
// long int nMonoms (poly f);
// void pl_lohi (gmatrix M, plist *pl, int *lo, int *hi);
// void std_lohi (gmatrix M, int *lo, int *hi);
// void getNumMonoms (gmatrix M, plist *pl, int deg, long int *ngens, long int *nmonoms);
// void getNstdMonoms (gmatrix M, int deg, long int *ngens, long int *nmonoms);
// void gmSize (gmatrix M);
// void nSPairs (gmatrix M);
// void spairs_cmd (int argc, char *argv[]);
// void spairs_flush (gmatrix M);
// void size_cmd (int argc, char *argv[]);

#define RESSIZE 20

int dl_ndeg (dlist *dl, int deg)
{
    int sum = 0;
    int i;

    for (i=1; i<=length(dl); i++)
        if (DREF(*dl, i) IS deg)
            sum++;
    return(sum);
}

void resLoHi (int v, int *ResLength, int *lo, int *hi, int betti[])
/* var. number of the resolution */
/* result = length of the resolution */
/* result = lowest and highest (slanted) degrees in res. */
/* result = total betti numbers, betti[0..reslen] */
{
    variable *p;
    int reslen, l, h;
    gmatrix M;

    p = VREF(v);
    reslen = -1;
    do {
        if (is_a_module(p)) {
            if (p->name[0] ISNT '0') {  /* index of variable is zero */
                M = VAR_MODULE(p);
                if (reslen IS -1) {
                    reslen = 0;
                    dl_lohi(&M->degrees, lo, hi);
                    betti[0] = length(&M->degrees);
                }
                if (ncols(M) IS 0) break;
                reslen++;
                betti[reslen] = ncols(M);
                dl_lohi(&M->deggens, &l, &h);
                l -= reslen;
                h -= reslen;
                if (l < *lo) *lo = l;
                if (h > *hi) *hi = h;
            }
        }
        v++;
        if (v > last_var) break;
        p = VREF(v);
    } while (p->exists IS PARTVAR);
    *ResLength = reslen;
}

void resDeg (int v, int deg, int betti[])
/* var. number of the resolution */
/* betti[] will be filled up with the # in this (slanted) deg*/
{
    variable *p;
    int reslen;
    gmatrix M;

    p = VREF(v);
    reslen = -1;
    do {
        if (is_a_module(p)) {
            if (p->name[0] ISNT '0') {  /* index of variable is zero */
                M = VAR_MODULE(p);
                if (reslen IS -1) {
                    reslen = 0;
                    betti[0] = dl_ndeg(&M->degrees, deg);
                }
                if (ncols(M) IS 0) break;
                reslen++;
                betti[reslen] = dl_ndeg(&M->deggens, deg+reslen);
            }
        }
        v++;
        if (v > last_var) break;
        p = VREF(v);
    } while (p->exists IS PARTVAR);
}

void betti (FILE *fil, variable *p)
{
    int v, i, deg;
    int lo, hi, reslen;
    int betti[RESSIZE];

    v = p->var_num;
    resLoHi(v, &reslen, &lo, &hi, betti);

    fnewline(fil);
    fprint(fil, "total:  ");
    for (i=0; i<=reslen; i++)
        fprint(fil, "%5d ", betti[i]);
    fprint(fil, "\n");

    fnewline(fil);
    fprint(fil, "--------");
    for (i=0; i<=reslen; i++)
        fprint(fil, "------");
    fprint(fil, "\n");

    for (deg=lo; deg<=hi; deg++) {
        fnewline(fil);
        fprint(fil, "%5d:  ", deg);
        resDeg(v, deg, betti);
        for (i=0; i<=reslen; i++)
            if (betti[i] IS 0)
                fprint(fil, "    - ");
            else
                fprint(fil, "%5d ", betti[i]);
        fprint(fil, "\n");
    }
}

void betti_cmd (int argc, char *argv[])
{
    variable *p;

    if (argc ISNT 2) {
        printnew("betti <res>\n");
        return;
    }
    p = find_var(argv[1]);
    if (p IS NULL) return;
    betti(outfile, p);
}

long int nMonoms (poly f)
{
    long total = 0;

    while (f ISNT NULL) {
        f = f->next;
        total++;
    }
    return(total);
}

void pl_lohi (gmatrix M, plist *pl, int *lo, int *hi)
{
    int i, d;

    if (length(pl) IS 0) {
        *lo = 0;
        *hi = 0;
        return;
    }
    *lo = degree(M, PREF(*pl, 1));
    *hi = *lo;
    for (i=2; i<=length(pl); i++) {
        d = degree(M, PREF(*pl, i));
        if (d < *lo)
            *lo = d;
        else if (d > *hi)
            *hi = d;
    }
}

void std_lohi (gmatrix M, int *lo, int *hi)
/* this routine uses the fact that the elems of M->stdbasis are in decreasing degree order. */
{
    mn_standard p;

    if (M->stdbasis IS NULL) {
        *lo = 0;
        *hi = 0;
        return;
    }
    p = M->stdbasis;
    *hi = p->degree;
    while (p->next ISNT NULL) p = p->next;
    *lo = p->degree;
}

void getNumMonoms (gmatrix M, plist *pl, int deg, long int *ngens, long int *nmonoms)
{
    int j;
    poly f;

    *ngens = 0;
    *nmonoms = 0;
    for (j=1; j<=length(pl); j++) {
        f = PREF(*pl, j);
        if (degree(M, f) IS deg) {
            (*ngens)++;
            *nmonoms += nMonoms(f);
        }
    }
}

void getNstdMonoms (gmatrix M, int deg, long int *ngens, long int *nmonoms)
{
    long ng, nm;
    int d;
    mn_standard p;

    ng = 0;
    nm = 0;
    p = M->stdbasis;
    while ((p ISNT NULL) AND ((d = p->degree) >= deg)) {
        if (d IS deg) {
            ng++;
            nm += nMonoms(p->standard);
        }
        p = p->next;
    }
    *ngens = ng;
    *nmonoms = nm;
}

void gmSize (gmatrix M)
{
    long int totgens, totstd;
    long int ngens, nstd, nmons, nmonstd;
    int lo, lo2, hi, hi2, i;

    totgens = 0;
    totstd = 0;
    dl_lohi(&M->deggens, &lo, &hi);
    std_lohi(M, &lo2, &hi2);
    newline();
    print("degree   #gens    #standard    #monoms in gens    #monoms in std\n");
    if (lo2 < lo) lo = lo2;
    if (hi2 > hi) hi = hi2;
    for (i=lo; i<=hi; i++) {
        getNumMonoms(M, &M->gens, i, &ngens, &nmons);
        getNstdMonoms(M, i, &nstd, &nmonstd);
        totgens += nmons;
        totstd += nmonstd;
        newline();
        print("%6d%8ld%13ld", i, ngens, nstd);
        print("%19ld%18ld\n", nmons, nmonstd);
    }
    newline();
    print("total %8d%13d%19ld%18ld\n", length(&M->gens), M->nstandard,
          totgens, totstd);
}

void nSPairs (gmatrix M)
{
    mn_syzes *a;
    int i, d;
    long total, grand;
    mn_pair *p;

    a = &M->monsyz;
    d = M->mn_lodeg;

    grand = 0;
    newline();
    print("degree #S-pairs\n");
    newline();
    print("------ --------\n");
    for (i=1; i<=length(a); i++) {
        total = 0;
        p = * (mn_pair **) ref(a, i);
        while (p ISNT NULL) {
            total++;
            p = p->mpp;
        }
        grand += total;
        if (total ISNT 0) {
            newline();
            print("%6d %8ld\n", d+i, total);
        }
    }
    newline();
    print("------ --------\n");
    newline();
    print("total  %8ld\n", grand);
}

void spairs_cmd (int argc, char *argv[])
{
    gmatrix M;

    if (argc ISNT 2) {
        printnew("spairs <module>\n");
        return;
    }

    GET_MOD(M, 1);
    nSPairs(M);
}

void spairs_flush (gmatrix M)
{
    mn_syzes *a;
    int i;
    long total;
    mn_pair *p;

    a = &M->monsyz;

    total = 0;
    for (i=1; i<=length(a); i++) {
        p = * (mn_pair **) ref(a, i);
        while (p ISNT NULL) {
            total++;
            p = p->mpp;
        }
    }
    intflush("<%ld>", total);
}

void size_cmd (int argc, char *argv[])
{
    gmatrix M;

    if (argc ISNT 2) {
        printnew("size <module>\n");
        return;
    }

    GET_MOD(M, 1);
    gmSize(M);
}
