// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include "shared.h"
#include "ring.h"
#include "charp.h"      // For init_charp(), charac
#include "hilb.h"       // For mn_ring(), mn_unring(), headStash, monStash, parenStash
#include "input.h"      // For prinput(), get_line(), get_int(), get_defint()
#include "mem.h"        // For gimmy(), mkPolyStash(), endof_stash()
#include "monitor.h"    // For print(), prerror(), newline(), fnewline(), fprint()
#include "qring.h"      // For qrgInstall(), qrgKill(), qrgDisplay()
#include "rvar.h"       // For getVarRel(), getVars()
#include "set.h"        // For maxdegree
#include "smallterm.h"  // For symmCreate(), blockList
#include "gm_dlist.h" /* For dl_concat, dl_addto, dl_min, dlVarConsume, dlConsume, dlScan,
                         dlSpecSet, varUse */
#include "plist.h"     // For dl_copy, dl_kill, dl_lohi
#include "stash.h"     // For gimmy, endof_stash
#include "human_io.h"  // For get_int, get_defint

// Functions not in headers - need forward declarations
extern void dl_set(dlist* dl, int i, int val); // from gm_dlist.c but not exported

// Inline functions for list operations
static inline void dl_init(dlist* dl)
{
    dlist_init(dl);
}

static inline void dl_insert(dlist* dl, int val)
{
    dlist_insert(dl, val);
}

int numvars;  // number of variables in current ring
int nblocks;  // number of variable blocks in current ring
symmBlock* blocks; /* array(0..nblocks-1) of info for each block in
                      current ring */
int compLoc; // = -1 if comp doesn't exist in terms, else is location
term zerodegs;
char** varnames;  // names of vars of current ring, 0..numvars-1
dlist rgDegs;     // degrees of vars , 1..numvars
char* pStash;     // polynomial stash for this ring

void init_rings(void)
{
    blockList = NULL;
}

int nvars(ring R)
{
    return (R->nvars);
}

int getRingVar(ring R, char* s)
{
    return (getVarRel(R->varnames, R->nvars, s));
}

char*varName(ring R, int i) // variables indexed from 0..nvars(R)-1
{
    if ((i < 0) || (i >= R->nvars))
        return (NULL);
    return (R->varnames[i]);
}

int varWeight(ring R, int i)
{
    if ((i < 0) || (i >= R->nvars))
        return (0); // Changed from NULL to 0 since function returns int
    return (dlist_ref(&R->degs, i + 1));
}

void rgInstall(ring R)
{
    init_charp(R->charac);
    numvars = R->nvars;
    nblocks = R->nblocks;
    blocks = R->symmalg;
    compLoc = R->compLoc;
    zerodegs = R->zerodegs;
    varnames = R->varnames;
    rgDegs = R->degs;
    pStash = R->pStash;
    headStash = R->headStash;
    monStash = R->monStash;
    parenStash = R->parenStash;
    qrgInstall(R->Rideal);
}

void rgKill(ring R)
{
    if (R == NULL)
        return;
    if (R->Rideal == NULL)
    { // no quotient ring here
        if (R->pStash != NULL)
            endof_stash(R->pStash);
        dl_kill(&R->degs);
        dl_kill(&R->monorder);
        dl_kill(&R->wtfcns);
        mn_unring(R);
    }
    else
    {
        qrgKill(R->Rideal, R->Rstd);
    }
}

ring rgCreate(int characteristic, char** vnames, dlist* degs, dlist* monorder, dlist* wtfcns)
{
    register ring result;
    register int i, j, n;
    int* d;
    int loc, wtloc;

    result = (ring)(void*)gimmy(sizeof(struct ringrec));

    result->Rideal = NULL;
    result->Rstd = NULL;

    result->charac = characteristic;
    result->varnames = vnames;
    dl_copy(degs, &result->degs);
    dl_copy(monorder, &result->monorder);
    dl_copy(wtfcns, &result->wtfcns);
    result->nvars = length(degs);
    result->nblocks = length(monorder);
    result->compLoc = -1; // if nec., it is set below

    result->pStash = mkPolyStash(result->nblocks);
    result->symmalg =
        (symmBlock*)(void*)gimmy((unsigned)(result->nblocks + 1) * (unsigned)sizeof(symmBlock));
    result->symmalg[result->nblocks] = NULL;

    loc = 1;
    wtloc = 1;
    for (i = 0; i < result->nblocks; i++)
    {
        n = dlist_ref(monorder, i + 1);
        if (n == RCOMP)
        {
            result->compLoc = i;
            result->symmalg[i] = (symmBlock)(void*)gimmy(sizeof(struct symblock));
            result->symmalg[i]->rtyp = RCOMP;
        }
        else if (n == RWTFCN)
        {
            d = (int*)(void*)gimmy((unsigned)result->nvars * (unsigned)sizeof(int));
            for (j = 0; j < result->nvars; j++, wtloc++)
                d[j] = dlist_ref(wtfcns, wtloc);
            result->symmalg[i] = (symmBlock)(void*)gimmy(sizeof(struct symblock));
            result->symmalg[i]->rtyp = RWTFCN;
            result->symmalg[i]->degs = d;
        }
        else
        {
            d = (int*)(void*)gimmy((unsigned)(n + 1) * (unsigned)sizeof(int));
            for (j = 0; j < n; j++, loc++)
                d[j] = dlist_ref(degs, loc);
            d[n] = 1;
            result->symmalg[i] = symmCreate(n, d, vnames);
            vnames += n;
        }
    }
    result->zerodegs = (term)(void*)gimmy((unsigned)sizeof(smallmon) * (unsigned)result->nblocks);
    for (i = 0; i < result->nblocks; i++)
        result->zerodegs[i] = 0;
    mn_ring(result); // sets head, mon, paren stashes
    return (result);
}

// getMonOrder obtains from the user the monomial order (a "dlist")

// A monomial order consists of a sequence of numbers, with 3 special
// characters also allowed
// - a number list, as usual using dlConsume
// - "w" (without the quotes), stands for a weight function
// - "c" stands for the location of the module "component", if any.
// - "-" stands for: no component will be stored.  This one MUST appear
// as the first entry, or not at all.

void getMonOrder(dlist* monorder, int nvars)
{
    register int nv, i, this, nblks;
    boolean done, hascomp, needscomp;
    int argc;
    char** argv;

    dl_init(monorder);
    do
    {
        done = TRUE;
        nv = 0;
        dl_kill(monorder);  // in case this is second time around
        dl_init(monorder);  // make it an int array again
        hascomp = FALSE;

        prinput("monomial order (if not rev. lex.)  ");
        dlSpecSet(varUse); // allow "c" and "w" in monomial order
        get_line(&argc, &argv);
        if ((argc >= 1) && (argv[0][0] == '-'))
        {
            needscomp = FALSE;
            argc--;
            argv++;
        }
        else
            needscomp = TRUE;
        dlVarConsume(monorder, argc, argv);
        dlSpecSet(NULL);
        nblks = length(monorder);
        for (i = 1; i <= nblks; i++)
        {
            this = dlist_ref(monorder, i);
            if (this == RCOMP)
            {
                if (hascomp == TRUE)
                {
                    prerror("; only one component allowed, try again\n");
                    done = FALSE;
                    break;
                }
                else
                    hascomp = TRUE;
            }
            else if (this != RWTFCN)
            {
                if (this > 0)
                    nv += this;
                else
                {
                    prerror("; negative numbers not allowed, try again\n");
                    done = FALSE;
                    break;
                }
            }
        }
        if (nv > nvars)
        {
            prerror("; too many variables specified, try again\n");
            done = FALSE;
        }
    } while (!done);
    if (nvars > nv)
        dl_insert(monorder, nvars - nv);
    if (needscomp && (!hascomp))
        dl_insert(monorder, RCOMP);
}

void getWtFcn(int n, dlist* wtfcns, dlist* degs, char** vars)
{
    int argc, elimOrder, i, len, first;
    int a, b, k;
    char** argv;

    first = length(wtfcns);
    len = length(degs);
    do
    {
        prinput("weight vector #%d                   ", n);
        get_line(&argc, &argv);
        if (argc == 0)
            continue;
        elimOrder = getVarRel(vars, len, argv[0]);
        if (elimOrder >= 0)
        {
            for (i = 0; i <= elimOrder; i++)
                dl_insert(wtfcns, dlist_ref(degs, i + 1));
            for (; i < len; i++)
                dl_insert(wtfcns, 0);
        }
        else
            dlConsume(wtfcns, argc, argv, first + len, 0);
    } while (argc == 0);

    // we now check to see if this wt fcn is negative, if so:
    // up it by a multiple of "degs"

    k = 0;
    for (i = 1; i <= length(degs); i++)
    {
        a = dlist_ref(wtfcns, first + i);
        b = dlist_ref(degs, i);
        if (a + b * k >= 0)
            continue;
        k = (-a - 1) / b + 1; // with this k, k = least number s.t. a+bk>=0
    }
    if (k > 0)
    {
        for (i = 1; i <= length(degs); i++)
        {
            int temp = dlist_ref(wtfcns, first + i) + k * dlist_ref(degs, i);
            dl_set(wtfcns, first + i, temp);
        }
    }
}

static int n_bigprimes = 20,
           bigprimes[] = { 32749, 32719, 32717, 32713, 32707, 32693, 32687, 32653, 32647, 32633,
                           32621, 32611, 32609, 32603, 32587, 32579, 32573, 32569, 32563, 32561 };

static int n_lilprimes = 26, lilprimes[] = { 2,  3,  5,  7,  11, 13, 17, 19, 23, 29, 31, 37, 41,
                                             43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101 };

ring rgScan(void)
{
    int chrc, nwtfcns, nv;
    int i, m, n;
    dlist monorder, degs, degs2, wtfcns;
    ring R;
    char** vars;

    prinput("characteristic (if not %5d)      ", LARGE_PRIME);
    chrc = get_defint("", LARGE_PRIME);
    if (chrc > 0 && chrc < lilprimes[n_lilprimes - 1])
    {
        for (i = 0; i < n_lilprimes; ++i)
            if (chrc == lilprimes[i])
                break;
        if (i == n_lilprimes)
            chrc = 0;
    } // trust 3 digit primes for now...
    if (chrc < 0 && chrc >= -n_bigprimes)
        chrc = bigprimes[-chrc - 1];
    else if (chrc <= 0)
    {
        prerror("; characteristic must be a positive prime...\n");
        prerror("; setting characteristic to %d\n", LARGE_PRIME);
        chrc = LARGE_PRIME;
    }

    prinput("number of variables                ");
    nv = get_int("");
    if ((nv <= 0) || (nv > NVARS))
    {
        prerror("; need between 1 and %d variables\n", NVARS);
        return (NULL);
    }

    vars = getVars(nv);
    if (vars == NULL)
        return (NULL);

    prinput("variable weights (if not all 1)    ");
    dl_init(&degs);
    dl_init(&degs2);
    dlScan(&degs2, nv, 1);
    n = dl_min(&degs2);
    if (n <= 0)
        m = -n + 1; // make every weight positive
    else
        m = 0; // i.e. don't change weights
    dl_addto(&degs, m, &degs2);
    dl_kill(&degs2);
    for (i = 1; i <= length(&degs); i++)
        if (dlist_ref(&degs, i) > maxdegree)
        {
            {
                int temp = maxdegree;
                dl_set(&degs, i, temp);
            }
            prerror("; weights must be <= %d\n", maxdegree);
        }
    // get monomial order now

    getMonOrder(&monorder, nv);
    nwtfcns = 0;
    dl_init(&wtfcns);
    for (i = 1; i <= length(&monorder); i++)
        if (dlist_ref(&monorder, i) == RWTFCN)
        {
            nwtfcns++;
            getWtFcn(nwtfcns, &wtfcns, &degs, vars);
        }

    R = rgCreate(chrc, vars, &degs, &monorder, &wtfcns);
    dl_kill(&monorder);
    dl_kill(&degs);
    dl_kill(&wtfcns);

    newline();
    print("  largest degree of a monomial        : ");
    for (i = 0; i < R->nblocks; i++)
        if (R->symmalg[i]->rtyp == RSYMM)
            print("%d ", R->symmalg[i]->maxdeg);
    print("\n");
    return (R);
}

void rgDisplay(FILE* fil, ring R)
{
    int i, j, loc;
    int nsymm;

    if (R == NULL)
        return;

    fnewline(fil);
    fprint(fil, "characteristic           : %d\n", R->charac);
    fnewline(fil);
    fprint(fil, "number of variables      : %d\n", R->nvars);

    nsymm = 0;
    for (i = 0; i < R->nblocks; i++)
    {
        switch (R->symmalg[i]->rtyp)
        {
        case RSYMM:
            nsymm++;
            break;
        case RWTFCN:
            break;
        }
    }

    if (nsymm == 1)
    {
        fnewline(fil);
        fprint(fil, "variables                : ");
        for (i = 0; i < R->nvars; i++)
            fprint(fil, "%s", R->varnames[i]);
        fprint(fil, "\n");

        fnewline(fil);
        fprint(fil, "weights                  : ");
        for (i = 1; i <= R->nvars; i++)
            fprint(fil, "%d ", dlist_ref(&R->degs, i));
        fprint(fil, "\n");
    }
    else
    {
        loc = 0;
        for (i = 0; i < R->nblocks; i++)
        {
            if (R->symmalg[i]->rtyp == RSYMM)
            {
                fnewline(fil);
                fprint(fil, "%2d variables for block %d : ", R->symmalg[i]->nvars, i);
                for (j = 0; j < R->symmalg[i]->nvars; j++, loc++)
                    fprint(fil, "%s", R->varnames[loc]);
                fprint(fil, "\n");
            }
        }

        loc = 1;
        for (i = 0; i < R->nblocks; i++)
        {
            if (R->symmalg[i]->rtyp == RSYMM)
            {
                fnewline(fil);
                fprint(fil, "weights for block %d      : ", i);
                for (j = 1; j <= R->symmalg[i]->nvars; j++, loc++)
                    fprint(fil, "%d ", dlist_ref(&R->degs, loc));
                fprint(fil, "\n");
            }
        }
    }

    for (i = 0; i < R->nblocks; i++)
    {
        if (R->symmalg[i]->rtyp == RWTFCN)
        {
            fnewline(fil);
            fprint(fil, "weight vector block %d    : ", i);
            for (j = 0; j < R->nvars; j++)
                fprint(fil, "%d ", R->symmalg[i]->degs[j]);
            fprint(fil, "\n");
        }
    }

    fnewline(fil);
    fprint(fil, "monomial order           : ");
    for (i = 0; i < R->nblocks; i++)
    {
        int typ = R->symmalg[i]->rtyp;
        if (typ == RCOMP)
            fprint(fil, "c ");
        else if (typ == RWTFCN)
            fprint(fil, "w ");
        else
            fprint(fil, "%d ", R->symmalg[i]->nvars);
    }
    fprint(fil, "\n");

    fnewline(fil);
    fprint(fil, "top degree of a monomial : ");
    for (i = 0; i < R->nblocks; i++)
        if (R->symmalg[i]->rtyp == RSYMM)
            fprint(fil, "%d ", R->symmalg[i]->maxdeg);
    fprint(fil, "\n");

    if (R->compLoc == -1)
    {
        fnewline(fil);
        fprint(fil, "maximum number of rows in any matrix is : 1\n");
    }

    if (R->Rideal != NULL)
        qrgDisplay(fil, R);
}

void rgPut(FILE* fil, ring R)
{
    // This function appears to be a stub in the original
    (void)fil;
    (void)R;
}

ring rgSum(ring R1, ring R2)
{
    int chrc;
    char** vnames;
    dlist degs, monorder, wtfcns;
    int i, j, n, nvars, nleft;
    char** s, ** t;
    ring R;

    if ((R1 == NULL) || (R2 == NULL))
        return (NULL);
    chrc = R1->charac;
    nvars = R1->nvars + R2->nvars;

    vnames = (char**)(void*)gimmy((unsigned)nvars * (unsigned)sizeof(char*));
    s = R1->varnames;
    t = vnames;
    for (i = 0; i < R1->nvars; i++)
        *t++ = *s++;
    s = R2->varnames;
    for (i = 0; i < R2->nvars; i++)
        *t++ = *s++;

    dl_init(&degs);
    dl_concat(&degs, &R1->degs, &R2->degs);

    dl_init(&monorder);
    for (i = 1; i <= length(&R1->monorder); i++)
    {
        n = dlist_ref(&R1->monorder, i);
        if (n != RCOMP)
            dl_insert(&monorder, n);
    }
    for (i = 1; i <= length(&R2->monorder); i++)
    {
        n = dlist_ref(&R2->monorder, i);
        dl_insert(&monorder, n);
    }

    // bug fix 8/2/94 by John P. Dalbec
    dl_init(&wtfcns);
    // nleft = R1->nvars - 1;
    nleft = R1->nvars;
    for (i = 1; i <= length(&R1->wtfcns); i++)
    {
        dl_insert(&wtfcns, dlist_ref(&R1->wtfcns, i));
        nleft--;
        if (nleft == 0)
        {
            for (j = 1; j <= R2->nvars; j++)
                dl_insert(&wtfcns, 0);
            // nleft = R1->nvars - 1;
            nleft = R1->nvars;
        }
    }
    nleft = 0;
    for (i = 1; i <= length(&R2->wtfcns); i++)
    {
        if (nleft == 0)
        {
            for (j = 1; j <= R1->nvars; j++)
                dl_insert(&wtfcns, 0);
            // nleft = R2->nvars - 1;
            nleft = R2->nvars;
        }
        dl_insert(&wtfcns, dlist_ref(&R2->wtfcns, i));
        nleft--;
    }

    R = rgCreate(chrc, vnames, &degs, &monorder, &wtfcns);
    dl_kill(&degs);
    dl_kill(&monorder);
    dl_kill(&wtfcns);
    return (R);
}

ring rgFromDegrees(dlist* degs, char** vars)
{
    ring result;
    dlist correctDegs, monorder, wtfcns;
    int n, lo, hi;

    dl_lohi(degs, &lo, &hi);
    dl_init(&correctDegs);
    if (lo <= 0)
        n = -lo + 1;
    else
        n = 0;
    dl_addto(&correctDegs, n, degs);

    dl_init(&monorder);
    dl_insert(&monorder, length(degs));
    dl_insert(&monorder, RCOMP);
    dl_init(&wtfcns);

    result = rgCreate(charac, vars, &correctDegs, &monorder, &wtfcns);

    dl_kill(&correctDegs);
    dl_kill(&monorder);
    dl_kill(&wtfcns);
    return (result);
}
