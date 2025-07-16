// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <stdio.h>
#include "shared.h"
#include "betti.h"
#include "vars.h"
#include "gmatrix.h"
#include "monitor.h"
#include "plist.h"
#include "cmds.h"
#include "shell.h"

// Constants
constexpr int RESSIZE = 20;
// PARTVAR comes from shared.h

// Inline helper for GET_MOD macro replacement
static inline bool get_mod(gmatrix* g, int argc, char** argv, int i)
{
    if (i >= argc)
        return false;
    *g = vget_mod(argv[i]);
    return (*g != NULL);
}

int dl_ndeg(dlist* dl, int deg)
{
    int sum = 0;
    int i;

    for (i = 1; i <= length(dl); i++)
        if (dlist_ref(dl, i) == deg)
            sum++;
    return sum;
}

void resLoHi(int v, int* ResLength, int* lo, int* hi, int betti[])
{
    variable* p;
    int reslen, l, h;
    gmatrix M;

    p = vref(v);
    reslen = -1;
    do
    {
        if (is_a_module(p))
        {
            if (p->name[0] != '0')
            { // index of variable is zero
                M = var_get_module(p);
                if (reslen == -1)
                {
                    reslen = 0;
                    dl_lohi(&M->degrees, lo, hi);
                    betti[0] = length(&M->degrees);
                }
                if (ncols(M) == 0)
                    break;
                reslen++;
                betti[reslen] = ncols(M);
                dl_lohi(&M->deggens, &l, &h);
                l -= reslen;
                h -= reslen;
                if (l < *lo)
                    *lo = l;
                if (h > *hi)
                    *hi = h;
            }
        }
        v++;
        if (v > last_var)
            break;
        p = vref(v);
    } while (p->exists == PARTVAR);
    *ResLength = reslen;
}

void resDeg(int v, int deg, int betti[])
{
    variable* p;
    int reslen;
    gmatrix M;

    p = vref(v);
    reslen = -1;
    do
    {
        if (is_a_module(p))
        {
            if (p->name[0] != '0')
            { // index of variable is zero
                M = var_get_module(p);
                if (reslen == -1)
                {
                    reslen = 0;
                    betti[0] = dl_ndeg(&M->degrees, deg);
                }
                if (ncols(M) == 0)
                    break;
                reslen++;
                betti[reslen] = dl_ndeg(&M->deggens, deg + reslen);
            }
        }
        v++;
        if (v > last_var)
            break;
        p = vref(v);
    } while (p->exists == PARTVAR);
}

void betti(FILE* fil, variable* p)
{
    int v, i, deg;
    int lo, hi, reslen;
    int betti[RESSIZE];

    v = p->var_num;
    resLoHi(v, &reslen, &lo, &hi, betti);

    fnewline(fil);
    fprint(fil, "total:  ");
    for (i = 0; i <= reslen; i++)
        fprint(fil, "%5d ", betti[i]);
    fprint(fil, "\n");

    fnewline(fil);
    fprint(fil, "--------");
    for (i = 0; i <= reslen; i++)
        fprint(fil, "------");
    fprint(fil, "\n");

    for (deg = lo; deg <= hi; deg++)
    {
        fnewline(fil);
        fprint(fil, "%5d:  ", deg);
        resDeg(v, deg, betti);
        for (i = 0; i <= reslen; i++)
            if (betti[i] == 0)
                fprint(fil, "    - ");
            else
                fprint(fil, "%5d ", betti[i]);
        fprint(fil, "\n");
    }
}

void betti_cmd(int argc, char* argv[])
{
    variable* p;

    if (argc != 2)
    {
        printnew("betti <res>\n");
        return;
    }
    p = find_var(argv[1]);
    if (p == NULL)
        return;
    betti(outfile, p);
}

long int nMonoms(poly f)
{
    register long total = 0;

    while (f != NULL)
    {
        f = f->next;
        total++;
    }
    return total;
}

void pl_lohi(gmatrix M, plist* pl, int* lo, int* hi)
{
    int i, d;

    if (length(pl) == 0)
    {
        *lo = 0;
        *hi = 0;
        return;
    }
    *lo = degree(M, plist_ref(pl, 1));
    *hi = *lo;
    for (i = 2; i <= length(pl); i++)
    {
        d = degree(M, plist_ref(pl, i));
        if (d < *lo)
            *lo = d;
        else if (d > *hi)
            *hi = d;
    }
}

void std_lohi(gmatrix M, int* lo, int* hi)
{
    // this routine uses the fact that the elems of
    // M->stdbasis are in decreasing degree order.
    mn_standard p;

    if (M->stdbasis == NULL)
    {
        *lo = 0;
        *hi = 0;
        return;
    }
    p = M->stdbasis;
    *hi = p->degree;
    while (p->next != NULL)
        p = p->next;
    *lo = p->degree;
}

void getNumMonoms(gmatrix M, plist* pl, int deg, long int* ngens, long int* nmonoms)
{
    register int j;
    register poly f;

    *ngens = 0;
    *nmonoms = 0;
    for (j = 1; j <= length(pl); j++)
    {
        f = plist_ref(pl, j);
        if (degree(M, f) == deg)
        {
            (*ngens)++;
            *nmonoms += nMonoms(f);
        }
    }
}

void getNstdMonoms(gmatrix M, int deg, long int* ngens, long int* nmonoms)
{
    register long ng, nm;
    register int d;
    register mn_standard p;

    ng = 0;
    nm = 0;
    p = M->stdbasis;
    while ((p != NULL) && ((d = p->degree) >= deg))
    {
        if (d == deg)
        {
            ng++;
            nm += nMonoms(p->standard);
        }
        p = p->next;
    }
    *ngens = ng;
    *nmonoms = nm;
}

void gmSize(gmatrix M)
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
    if (lo2 < lo)
        lo = lo2;
    if (hi2 > hi)
        hi = hi2;
    for (i = lo; i <= hi; i++)
    {
        getNumMonoms(M, &M->gens, i, &ngens, &nmons);
        getNstdMonoms(M, i, &nstd, &nmonstd);
        totgens += nmons;
        totstd += nmonstd;
        newline();
        print("%6d%8ld%13ld", i, ngens, nstd);
        print("%19ld%18ld\n", nmons, nmonstd);
    }
    newline();
    print("total %8d%13d%19ld%18ld\n", length(&M->gens), M->nstandard, totgens, totstd);
}

void nSPairs(gmatrix M)
{
    mn_syzes* a;
    int i, d;
    long total, grand;
    mn_pair* p;

    a = &M->monsyz;
    d = M->mn_lodeg;

    grand = 0;
    newline();
    print("degree #S-pairs\n");
    newline();
    print("------ --------\n");
    for (i = 1; i <= length(a); i++)
    {
        total = 0;
        p = *(mn_pair**)ref(a, i);
        while (p != NULL)
        {
            total++;
            p = p->mpp;
        }
        grand += total;
        if (total != 0)
        {
            newline();
            print("%6d %8ld\n", d + i, total);
        }
    }
    newline();
    print("------ --------\n");
    newline();
    print("total  %8ld\n", grand);
}

void spairs_cmd(int argc, char* argv[])
{
    gmatrix M;

    if (argc != 2)
    {
        printnew("spairs <module>\n");
        return;
    }

    if (!get_mod(&M, argc, argv, 1))
    {
        prerror("; module not found\n");
        return;
    }
    nSPairs(M);
}

void spairs_flush(gmatrix M)
{
    mn_syzes* a;
    int i;
    long total;
    mn_pair* p;

    a = &M->monsyz;

    total = 0;
    for (i = 1; i <= length(a); i++)
    {
        p = *(mn_pair**)ref(a, i);
        while (p != NULL)
        {
            total++;
            p = p->mpp;
        }
    }
    intflush("<%ld>", (int)total); // Cast to int for compatibility
}

void size_cmd(int argc, char* argv[])
{
    gmatrix M;

    if (argc != 2)
    {
        printnew("size <module>\n");
        return;
    }

    if (!get_mod(&M, argc, argv, 1))
    {
        prerror("; module not found\n");
        return;
    }
    gmSize(M);
}
