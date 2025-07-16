// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <string.h> // strcpy
#include "shared.h"
#include "monideal.h"
#include "monoms.h"   // monnewhead, monadv
#include "stdcmds.h"  // stdFirst, stdNext, stdWarning
#include "standard.h" // for other standard operations if needed
#include "poly.h"     // e_sub_i, p_monom, p_add, p_kill
#include "term.h"     // get_comp, sToExp, expToS, exp_degree
#include "gmatrix.h"  // mod_init, nrows, gmInsert
#include "plist.h"    // dl_copy
#include "monitor.h"  // print, printnew, newline, prflush, intflush, prerror
#include "mac.h"      // have_intr
#include "vars.h"     // make_var, set_value, current_ring
#include "stash.h"    // open_stash, get_slug, endof_stash
#include "charp.h"    // fd_one
#include "set.h"      // verbose, maxdegree
#include "ring.h"     // numvars
#include "qring.h"    // Rideal
#include "cmds.h"     // doIntCmd, vget_mod
#include "gm_poly.h"  // get_comp (if not in term.h)
#include "hilb.h"     // monbagadjoin, monrefund, monadjoin, monreset, monsearch (if not in
                      // monoms.h)
#include "human_io.h" // setzerodegs, getVarList, get_defint
#include "generic.h"  // VMODULE

// Note: The following types must be defined before including monideal.h:
// - ass_prime_func typedef
// - cheek structure
// These should be moved to shared.h in final cleanup

// Constants
constexpr int IDSIZE = 50;
// MAINVAR comes from shared.h
// VMODULE comes from generic.h

// Inline functions to replace macros
static inline bool get_mod(gmatrix *g, int argc, char **argv, int i)
{
    if (i >= argc)
        return false;
    *g = vget_mod(argv[i]);
    return (*g != NULL);
}

static inline bool new_mod(variable **p, int argc, char **argv, int i)
{
    if (i >= argc)
        return false;
    *p = make_var(argv[i], MAINVAR, VMODULE, current_ring);
    return (*p != NULL);
}

static inline bool new_savmod(variable **p, char *s)
{
    *p = make_var(s, MAINVAR, VMODULE, current_ring);
    return (*p != NULL);
}

// There are several generators used in this file.  In particular, the
// routine "findAssPrimes" which finds the associated prime ideals to a given
// monomial ideal.

// "gotAss" is the routine which is called after each such prime ideal is found.
// the parameters are: (*gotAss)(codim, exp)  where "codim" is the codimension
// of this prime ideal given by "exp": which has exp[i] == 1 iff variable i is
// in the prime ideal.

ass_prime_func gotAss; // set before using "findAssPrimes"

arrow monnext(arrow p)
{
    while (TRUE)
    {
        p = p->uld.lda[FOW];
        if (p->uld.ldkind == 'h')
            return NULL;
        if (p->uld.ldkind != 'm')
            continue;
        return p;
    }
}

// the following routine creates a monomial "head" ideal from the lead terms
// of the standard basis of M, in row "comp".  The function fcn should expect
// one parameter: an expterm.  It should modify this argument, if desired.

arrow monmake(gmatrix M, int comp, void (*fcn)(expterm))
{
    arrow p, head;
    register int i;
    poly f;
    expterm nexp;
    modgen mg;

    head = monnewhead(numvars);
    stdFirst(M, &mg, USESTD);
    while ((f = stdNext(&mg)) != NULL)
    {
        if (comp != get_comp(f))
            continue;
        sToExp(poly_initial(f), nexp);
        (*fcn)(nexp);
        monbagadjoin(head, nexp, NULL);
    }
    // now add in the stuff from Rideal, if any
    if (Rideal != NULL)
    {
        p = Rideal;
        while ((p = monnext(p)) != NULL)
        {
            for (i = 0; i < numvars; i++)
                nexp[i] = p->umn.mexp[i];
            (*fcn)(nexp);
            monbagadjoin(head, nexp, NULL);
        }
    }
    return head;
}

// the following routine creates a monomial "head" ideal from the lead terms
// of the standard basis of M, in row "comp".  The function fcn
// "tell" will be passed to monadjoin, and is used to find hilbert funcs
// and so on...
// NULL is returned if the user interrupts the computation.

arrow monhilb(gmatrix M, int comp, void (*fcn)(arrow, int))
{
    arrow p, head;
    poly f;
    expterm nexp;
    modgen mg;

    head = monnewhead(numvars);
    stdFirst(M, &mg, USESTD);
    while ((f = stdNext(&mg)) != NULL)
    {
        if (comp != get_comp(f))
            continue;
        if (have_intr())
        {
            monrefund(head);
            print("\n");
            return NULL;
        }
        sToExp(poly_initial(f), nexp);
        monadjoin(head, nexp, fcn);
        if (verbose > 0)
            prflush(".");
    }
    // now add in the stuff from Rideal, if any
    if (Rideal != NULL)
    {
        p = Rideal;
        while ((p = monnext(p)) != NULL)
        {
            if (have_intr())
            {
                monrefund(head);
                print("\n");
                return NULL;
            }
            monadjoin(head, p->umn.mexp, fcn);
            if (verbose > 0)
                prflush(".");
        }
    }
    return head;
}

void tm_radical(expterm nexp)
{
    register int j;

    for (j = 0; j < numvars; j++)
        if (nexp[j] > 0)
            nexp[j] = 1;
}

arrow monradical(gmatrix M, int comp)
{
    return monmake(M, comp, tm_radical);
}

int *mondivExp; /* used in mondivide, mondiv as a parameter to tm_...
                   routines */

void tm_mdivide(expterm nexp)
{
    register int j, k;

    k = 0;
    for (j = 0; j < numvars; j++)
        if (mondivExp[j] == 1)
            nexp[k++] = nexp[j];
    for (; k < numvars; k++)
        nexp[k] = 0;
}

arrow mondivide(gmatrix M, int comp, int *exp)
{
    mondivExp = exp;
    return monmake(M, comp, tm_mdivide);
}

// the following routine "mondiv" is the same as mondivide, except that the
// variables are not "compacted" as they are in mondivide.

void tm_mdiv(expterm nexp)
{
    register int j;

    for (j = 0; j < numvars; j++)
        if (mondivExp[j] != 1)
            nexp[j] = 0;
}

arrow mondiv(gmatrix M, int comp, expterm vars)
{
    mondivExp = vars;
    return monmake(M, comp, tm_mdiv);
}

// redExp ("reduce exp") takes the monomial "t" and the monomial prime ideal
// "exp" (actually "exp" also contains var NE 0 statements too) and returns
// 0    if the monomial "t" is in this ideal.
// 1    if "t" isn't in the ideal but some var can be set to zero.
// -1   if each var in "t" cannot be set to zero.

int redExp(int *t, int *exp)
{
    register boolean isOne;
    register int i;

    isOne = TRUE;
    for (i = 0; i < numvars; i++)
    {
        if (t[i] > 0)
        {
            if (exp[i] == 1)
                return 0;
            if (exp[i] == 0)
                isOne = FALSE;
        }
    }
    if (isOne)
        return -1;
    return 1;
}

int topCodim; // used to determine codimension of an ideal: in  findAssPrimes

void findAssPrimes(arrow p, int *exp1, int codim)
{
    register int i;
    expterm exp;
    int *nexp;

    if (codim > topCodim)
    {
        return;
    }
    for (i = 0; i < numvars; i++)
        exp[i] = exp1[i];
    while (TRUE)
    {
        if (p == NULL)
        {
            (*gotAss)(codim, exp);
            return;
        }
        nexp = p->umn.mexp;
        switch (redExp(nexp, exp))
        {
        case 0:
            p = monnext(p);
            break;
        case -1:
            return;
        case 1:
            for (i = 0; i < numvars; i++)
            {
                if ((nexp[i] > 0) && (exp[i] == 0))
                {
                    exp[i] = 1;
                    findAssPrimes(monnext(p), exp, codim + 1);
                    exp[i] = -1;
                }
            }
            return;
        }
    }
}

void wrAss(int *exp, int codim)
{
    register int i;

    newline();
    print("[%d] ", codim);
    for (i = 0; i < numvars; i++)
        if (exp[i] == 1)
            print("1 ");
        else
            print("0 ");
    prflush("\n");
}

void mondisplay(arrow head)
{
    arrow p;
    int i;
    int *nexp;

    p = head;
    while ((p = monnext(p)) != NULL)
    {
        nexp = p->umn.mexp;
        newline();
        for (i = 0; i < numvars; i++)
            print("%d ", nexp[i]);
        print("\n");
    }
}

// Finding the codimension of a quotient of a free module
// by a monomial submodule.

static void codimGotAss(int codim, int *exp)
{
    if (codim <= topCodim)
    {
        if (verbose > 0)
            wrAss(exp, codim);
        topCodim = codim;
    }
}

int codim(gmatrix M)
{
    int ncomps;
    register int i, j;
    expterm exp;
    arrow head;

    ncomps = nrows(M);
    topCodim = numvars + 1;
    gotAss = codimGotAss;
    for (i = 1; i <= ncomps; i++)
    {
        if (verbose > 0)
        {
            newline();
            print("component %d:\n", i);
        }
        head = monradical(M, i);
        for (j = 0; j < numvars; j++)
            exp[j] = 0;
        findAssPrimes(monnext(head), exp, 0);
        monrefund(head);
    }
    return topCodim;
}

int codim_cmd(int argc, char *argv[])
{
    gmatrix M;
    int cod;

    if ((argc < 2) || (argc > 3))
    {
        printnew("codim <standard basis> [integer result]\n");
        return 1;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    stdWarning(M);
    cod = codim(M);
    doIntCmd(argv[2], cod, (argc == 3), "codimension");
    return 1;
}

// Finding the isolated primes of a quotient of a free
// module by a monomial submodule.

cheek *cheeks;

char *monprimes_stash;
gmatrix monprimes_N;

static void monprimesGotAss(int codim, int *exp)
{
    int i;
    cheek *p;

    p = (cheek *)(void *)get_slug((struct stash *)monprimes_stash);
    p->next = cheeks;
    cheeks = p;
    for (i = 0; i < numvars; i++)
    {
        if (exp[i] == 1)
            p->exp[i] = 1;
        else
            p->exp[i] = 0;
    }
}

void monprimes(gmatrix M)
{
    int ncomps;
    register int i, j;
    expterm exp;
    arrow head;

    ncomps = nrows(M);
    topCodim = numvars + 1;
    gotAss = monprimesGotAss;
    for (i = 1; i <= ncomps; i++)
    {
        head = monradical(M, i);
        for (j = 0; j < numvars; j++)
            exp[j] = 0;
        findAssPrimes(monnext(head), exp, 0);
        monrefund(head);
    }
}

int monprimes_cmd(int argc, char *argv[])
{
    gmatrix M;
    variable *z;
    poly f, g;
    int i;
    cheek *p, *q;

    if (argc != 3)
    {
        printnew("monprimes <standard basis> <result>\n");
        return 1;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    if (!new_mod(&z, argc, argv, 2))
        return 0;
    monprimes_N = mod_init();
    setzerodegs(monprimes_N, numvars);

    monprimes_stash =
        open_stash((int)(sizeof(cheek) + (size_t)(numvars - 1) * sizeof(short)), "cheek");
    cheeks = NULL;

    monprimes(M);
    for (p = cheeks; p != NULL; p = p->next)
    {
        for (q = cheeks; q != NULL; q = q->next)
        {
            if (q == p)
                continue;
            for (i = 0; i < numvars; ++i)
                if (p->exp[i] == 0 && q->exp[i] != 0)
                    break;
            if (i == numvars)
                break; // false prime
        }
        if (q != NULL)
            continue; // false prime

        f = NULL;
        for (i = 0; i < numvars; ++i)
            if (p->exp[i] == 1)
            {
                g = e_sub_i(i + 1);
                p_add(&f, &g);
                p_kill(&g);
            }
        gmInsert(monprimes_N, f);
    }
    endof_stash((struct stash *)monprimes_stash);

    set_value(z, monprimes_N);
    return 1;
}

// Finding the degree of a quotient of a free module by a monomial submodule.

// The following variables are really parameters to the generator "enumMons"

expterm MDmon;  // monomial generator
arrow MDhead;   // monomial header for artinian ring
int MDnvars;    // number of variables for this routine
long int MDdeg; // accumulated degree

long int mondegree(arrow head, int nvars)
{
    register int i;

    for (i = 0; i < nvars; i++)
        MDmon[i] = 0;
    MDhead = head;
    MDnvars = nvars;
    MDdeg = 1;
    enumMons(0);
    return MDdeg;
}

void enumMons(int lastval)
{
    register int i;

    for (i = lastval; i < MDnvars; i++)
    {
        MDmon[i]++;
        monreset(MDhead, FOW);
        if (!monsearch(MDhead, FOW, MDmon))
        {
            MDdeg++;
            enumMons(i);
        }
        MDmon[i]--;
    }
}

// the following variables are really parameters to the routine "degGotAss"
// which is inside the generator function "findAssPrimes".

gmatrix DDmat;     // monomial submodule in question
int DDcodim;       // computed codimension of  DDpl
int DDcomp;        // current component we are trying to find degree of
long int DDdegree; // accumulated degree for each associated prime

static void degGotAss(int codim, int *exp)
{
    arrow head;

    if (codim == DDcodim)
    {
        head = mondivide(DDmat, DDcomp, exp);
        DDdegree += mondegree(head, DDcodim);
        if (verbose > 0)
            intflush("%ld.", (int)DDdegree);
        monrefund(head);
    }
} // mod 24feb89 DB

long int modDegree(gmatrix M, int thisCodim)
{
    int j;
    int ncomps;
    arrow head;
    expterm exp;

    ncomps = nrows(M);
    DDdegree = 0;
    DDmat = M;
    DDcodim = thisCodim;
    gotAss = degGotAss;
    for (DDcomp = 1; DDcomp <= ncomps; DDcomp++)
    {
        head = monradical(M, DDcomp);
        for (j = 0; j < numvars; j++)
            exp[j] = 0;
        findAssPrimes(monnext(head), exp, 0);
        monrefund(head);
    }
    if (verbose > 0)
        prflush("\n");
    return DDdegree;
}

int degree_cmd(int argc, char *argv[])
{
    gmatrix M;
    int cod;
    long int deg;
    int oldVerbose;

    if ((argc < 2) || (argc > 4))
    {
        printnew("degree <standard basis> [integer codim] [integer degree]\n");
        return 1;
    }

    if (!get_mod(&M, argc, argv, 1))
        return 0;
    stdWarning(M);
    oldVerbose = verbose;
    verbose = FALSE;
    cod = codim(M);
    doIntCmd(argv[2], cod, (argc >= 3), "codimension");
    verbose = oldVerbose;
    deg = modDegree(M, cod);
    doIntCmd(argv[3], (int)deg, (argc == 4), "degree     ");
    return 1;
}

// computing a k-basis of a module

// the following variables are global to "accumMons", which does the actual
// work of the "basis" command.  They are set in the gm_basis routine.

gmatrix MAresult; // result matrix where we throw our monomials
arrow MAhead;     // monomial ideal consisting of lead terms of M
int *MAmon;       // monomial being constructed in accumMons
int MAcomp;       // row number of monomial being constructed
int MAcompDeg;    // degree of row "MAcomp"
int MAlo, MAhi;   // lo and hi degrees allowed for monomial
int *MAvars;      // variables allowed

void accumMons(int lastval)
{
    register int i, d, e;
    poly f;

    for (i = lastval; i < numvars; i++)
    {
        if (MAvars[i] != 1)
            continue;
        MAmon[i]++;
        e = exp_degree(MAmon);
        d = MAcompDeg + e;
        if ((d > MAhi) || (e > maxdegree))
        {
            // maxdegree = largest allowed degree
            MAmon[i]--;
            continue;
        }
        monreset(MAhead, FOW);
        if (!monsearch(MAhead, FOW, MAmon))
        {
            if (d >= MAlo)
            {
                f = p_monom(fd_one);
                expToS(MAmon, MAcomp, poly_initial(f));
                gmInsert(MAresult, f);
            }
            accumMons(i);
        }
        MAmon[i]--;
    }
}

gmatrix gm_basis(gmatrix M, expterm vars, int lo, int hi, boolean isbasis)
{
    int i;
    expterm exp;

    MAresult = mod_init();
    dl_copy(&M->degrees, &MAresult->degrees);
    MAlo = lo;
    MAhi = hi;
    MAvars = vars;
    for (MAcomp = 1; MAcomp <= nrows(M); MAcomp++)
    {
        MAcompDeg = dlist_ref(&M->degrees, MAcomp);
        if ((MAcompDeg > hi) && (isbasis))
            continue;
        MAhead = mondiv(M, MAcomp, vars);
        for (i = 0; i < numvars; i++)
            exp[i] = 0;
        MAmon = exp;

        monreset(MAhead, FOW);
        if (!monsearch(MAhead, FOW, MAmon))
            if (MAcompDeg >= lo)
                gmInsert(MAresult, e_sub_i(MAcomp));

        if (MAcompDeg <= hi)
            accumMons(0);
        monrefund(MAhead);
    }
    return MAresult;
}

// basis: finds a k-basis of the module presented by <matrix> (M), assuming
// that the variables are those of the [variable list].  The user is prompted
// for the lowest and highest degrees: A basis is given for the module
// in the range lo <= d <= hi.

int basis_cmd(int argc, char *argv[])
{
    expterm vars;
    gmatrix M;
    variable *p;
    int lo, hi;
    char s[IDSIZE];

    if (argc < 3)
    {
        printnew("k_basis <matrix> <result matrix> [variable list]\n");
        return 1;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    strcpy(s, argv[2]);
    if (!(getVarList(argc - 3, (const char **)(void *)(argv + 3), vars)))
        return 1;
    lo = get_defint("lowest degree (default=no lowest degree) ", -5000);
    hi = get_defint("highest degree (default=no high degree)  ", 5000);
    stdWarning(M);
    if (!new_savmod(&p, s))
        return 0;
    set_value(p, gm_basis(M, vars, lo, hi, TRUE));
    return 1;
}

// gmatrix gm_truncate(gmatrix M, expterm vars, int lo)
// {
// gmatrix result;
// int i;

// result = gm_basis(M, vars, lo, lo, FALSE);
// for (i = 1; i <= nrows(M); i++) {
// if (dlist_ref(&M->degrees, i) > lo)
// gmInsert(result, e_sub_i(i));
// }
// return result;
// }

int truncate_cmd(int argc, char *argv[])
{
    expterm vars;
    gmatrix M;
    variable *p;
    int lo;
    char s[IDSIZE];

    if (argc < 3)
    {
        printnew("truncate <matrix> <result matrix> [variable list]\n");
        return 1;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    strcpy(s, argv[2]);
    if (!(getVarList(argc - 3, (const char **)(void *)(argv + 3), vars)))
        return 1;
    lo = get_defint("lowest degree (default=0) ", 0);
    stdWarning(M);
    if (!new_savmod(&p, s))
        return 0;
    set_value(p, gm_basis(M, vars, lo, lo, FALSE));
    return 1;
}
