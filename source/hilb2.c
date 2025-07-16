// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"

// Arrow access structure - should be moved to shared.h in final cleanup
typedef struct
{
    union all *lda[2];
    cargo ldc;
    char ldkind;
    monhead umh;
} arrow_access;

#include "hilb2.h"
#include "hilb.h"     // monrefund, i_genfun, htell, genfun
#include "monideal.h" // monhilb

#include "gmatrix.h"  // nrows, ncols, mod_init, gmInsert
#include "vars.h"     // make_var, variable type constants, current_ring
#include "cmds.h"     // vget_mod
#include "ring.h"     // vrg_install, vget_ring
#include "poly.h"     // e_sub_i, p_mult, p_monom, p_add, p_sub, p_kill
#include "plist.h"    // dl_lohi, dl_copy
#include "term.h"     // expToS
#include "monitor.h"  // fprint, fnewline, print, printnew, intflush, prerror
#include "shell.h"    // outfile, to_shell
#include "charp.h"    // fd_one
#include "set.h"      // verbose
#include "stdcmds.h"  // stdWarning
#include "rmap.h"     // change1_vars
#include "qring.h"    // qrgReduce
#include "human_io.h" // setzerodegs
#include "gm_poly.h"  // get_comp
#include "generic.h"  // VMODULE

// Constants from original macros
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

void h_init(hilbFunc *hf, int deg0)
{
    register int i;

    hf->codim = 0;
    hf->deg0 = deg0;
    hf->degree = 0;
    for (i = 0; i < GFSIZE; i++)
        hf->genfun[i] = 0;
}

void h_addto(hilbFunc *hf, long gf[], int deg0)
{
    register int diff, i;

    // deg0 >= hf->deg0 ALWAYS, or problems result
    // hf->codim SHOULD BE 0
    if (deg0 < hf->deg0)
    {
        prerror("\n;internal error: hilb addto: deg0 too small\n");
        to_shell();
    }
    if (hf->codim != 0)
    {
        prerror("\n;internal error: hilb addto: codim not zero\n");
        to_shell();
    }

    diff = deg0 - hf->deg0;
    for (i = 0; i < GFSIZE - diff; i++)
    {
        hf->genfun[diff + i] += gf[i];
        hf->degree += gf[i];
    }
    while (i < GFSIZE)
        if (gf[i++] != 0)
        {
            prerror("\n;error: hilb addto: degree bound exceeded!\n");
            to_shell();
        }
}

void h_divAll(hilbFunc *hf)
{
    while (hf->degree == 0)
        h_div(hf);
}

void h_div(hilbFunc *hf)
{
    register int j, lo;
    lo = hf->codim;
    hf->degree += hf->genfun[GFSIZE - 1] *= -1;
    for (j = GFSIZE - 2; j >= lo; j--)
    {
        hf->genfun[j] = hf->genfun[j + 1] - hf->genfun[j];
        hf->degree += hf->genfun[j];
    }
    hf->codim++;
}

void h_display(hilbFunc *hf)
{
    register int i;

    fprint(outfile, "\n");
    for (i = hf->codim; i < GFSIZE; ++i)
        if (hf->genfun[i] != 0)
        {
            fnewline(outfile);
            fprint(outfile, "%7ld t %2d\n", hf->genfun[i], i + hf->deg0 - hf->codim);
        }
    if (hf->degree != 0)
    {
        fprint(outfile, "\n");
        fnewline(outfile);
        fprint(outfile, "codimension = %d\n", hf->codim);
        fnewline(outfile);
        fprint(outfile, "degree      = %ld\n", hf->degree);
    }
}

long h_genus(hilbFunc *hf)
{
    int i;
    long n, genus;
    int lo;

    genus = 1;
    lo = hf->codim;
    for (i = lo; i < GFSIZE; i++)
        if ((n = hf->genfun[i]) != 0)
            genus += (i - lo - 1) * n;
    return genus;
}

boolean hilb(hilbFunc *hf, gmatrix M) // returns TRUE if user doesn't interrupt
{
    int lodeg, hideg;
    int comp;
    arrow head;

    dl_lohi(&M->degrees, &lodeg, &hideg);
    h_init(hf, lodeg);
    for (comp = nrows(M); comp >= 1; comp--)
    {
        if (verbose > 0)
            intflush("[%d] ", comp);
        i_genfun();
        if ((head = monhilb(M, comp, htell)) == NULL)
            return FALSE;
        // genfun is int[], cast through void* to avoid alignment warning
        h_addto(hf, (long *)(void *)genfun, dlist_ref(&M->degrees, comp));
        monrefund(head);
    }
    if (verbose > 0)
        print("\n");
    return TRUE;
}

void hilbDisplay(hilbFunc *hf, gmatrix M)
{
    if (!hilb(hf, M))
        return;
    h_display(hf);
    h_divAll(hf);
    h_display(hf);
    fnewline(outfile);
    fprint(outfile, "genus       = %ld\n", h_genus(hf));
}

int hilb_cmd(int argc, char *argv[])
{
    gmatrix M;
    hilbFunc hf;

    if (argc != 2)
    {
        printnew("hilb <standard basis>\n");
        return 0;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    stdWarning(M);
    hilbDisplay(&hf, M);
    return 1;
}

static struct numer_rec
{
    gmatrix rmap;
    variable *R, *S;
    int comp;
    poly result;
} nu;

void numer_tell(arrow head, int plus)
{
    int i, j, en;
    int *a;
    static int exp[NVARS];
    poly f, g;

    // exp[] is current exponent vector
    en = ((arrow_access *)head)->umh.mn;
    a = ((arrow_access *)head)->umh.mloc->umn.mexp;
    for (i = 0, j = numvars - en; i < en; ++i, ++j)
        exp[j] = a[i];

    // make exp[] into poly f over R
    vrg_install(nu.R);
    f = p_monom(fd_one);
    expToS(exp, nu.comp, poly_initial(f));

    // map f to poly g over S
    g = change1_vars(nu.rmap, nu.R, nu.S, f);

    // add g to result
    vrg_install(nu.S);
    if (plus)
        p_add(&nu.result, &g);
    else
        p_sub(&nu.result, &g);
    p_kill(&g);
    vrg_install(nu.R);
    p_kill(&f);

    if (verbose > 1)
    { // print monom
        print("%c (", plus ? '+' : '-');
        for (i = 0; i < numvars; ++i)
        {
            print("%s%d", i ? ", " : "", exp[i]);
        }
        print(")\n");
    }
} // 8/26/93 DB

boolean hilb_numer(gmatrix M) // returns TRUE if user doesn't interrupt
{
    int comp;
    arrow head;
    poly f;

    nu.result = NULL;
    for (comp = nrows(M); comp >= 1; comp--)
    {
        vrg_install(nu.S);
        f = e_sub_i(comp);
        p_add(&nu.result, &f);
        p_kill(&f);

        vrg_install(nu.R);
        nu.comp = comp;
        if (verbose > 0)
            intflush("[%d] ", comp);
        if ((head = monhilb(M, comp, (void (*)(arrow, int))numer_tell)) == NULL)
            return FALSE;
        monrefund(head);
    }
    if (verbose > 0)
        print("\n");
    return TRUE;
} // 8/26/93 DB

int hilb_numer_cmd(int argc, char *argv[])
{
    gmatrix M, N;
    variable *p;

    if (argc != 4)
    {
        printnew("hilb_numer <standard basis> <ideal> <result>\n");
        return 0;
    }

    if (!get_mod(&M, argc, argv, 1))
        return 0;
    stdWarning(M);
    nu.R = vget_ring(NULL); // get current ring
    if (nu.R == NULL)
        return 0;
    if (!get_mod(&nu.rmap, argc, argv, 2))
        return 0;
    nu.S = vget_ring(NULL); // get current ring after rmap
    if (nu.S == NULL)
        return 0;
    if (!new_mod(&p, argc, argv, 3)) // will be over ring S
        return 0;
    hilb_numer(M);

    vrg_install(nu.S);
    N = mod_init(); // in ring S
    dl_copy(&M->degrees, &N->degrees);
    gmInsert(N, nu.result);
    set_value(p, N);
    return 1;
} // 8/26/93 DB

poly monom_prod(int exp[], int n, gmatrix rmap)
// cribbed from change1_vars
{
    poly f, g, h;
    int i, j;

    f = e_sub_i(1);
    for (i = 0; i < n; ++i)
        if (exp[i] > 0)
        {
            g = plist_ref(&rmap->gens, i + 1);
            for (j = 0; j < exp[i]; ++j)
            {
                h = p_mult(g, f);
                p_kill(&f);
                f = h;
            }
        }
    qrgReduce(&f);
    return f;
} // 9/9/94 DB

int monoms_cmd(int argc, char *argv[])
{
    if (argc != 4)
    {
        printnew("monoms <matrix> <ideal> <result>\n");
        return 0;
    }

    *argv[0] = '\0';
    binoms_cmd(argc, argv);
    return 0;
} // 9/9/94 DB

int binoms_cmd(int argc, char *argv[])
// This is here because it is cribbed from hilb_numer
{
    gmatrix M, N, rmap;
    variable *R, *S, *p;
    poly f, g;
    int exp1[NVARS], exp2[NVARS];
    int n, i, j, comp, both;
    field coef;

    if (argc != 4)
    {
        printnew("binoms <matrix> <ideal> <result>\n");
        return 0;
    }
    both = *argv[0] != '\0'; // binoms or monoms?

    if (!get_mod(&M, argc, argv, 1))
        return 0;
    R = vget_ring(NULL); // get current ring
    if (R == NULL)
        return 0;
    n = nrows(M);

    if (!get_mod(&rmap, argc, argv, 2))
        return 0;
    S = vget_ring(NULL); // get current ring after rmap
    if (S == NULL)
        return 0;
    if (n > ncols(rmap))
    {
        prerror("; too few elements in <ideal>\n");
        return 0;
    }

    if (!new_mod(&p, argc, argv, 3)) // will be over ring S
        return 0;

    vrg_install(S);
    N = mod_init();
    setzerodegs(N, 1);

    vrg_install(R);
    for (i = 1; i <= ncols(M); ++i)
    {
        for (j = 0; j < n; ++j)
            exp1[j] = 0;
        f = plist_ref(&M->gens, i);
        while (f != NULL)
        {
            coef = f->coef;
            comp = get_comp(f);
            exp1[comp - 1] += coef;
            f = f->next;
        }
        for (j = 0; j < n; ++j)
        {
            if (exp1[j] < 0)
            {
                exp2[j] = -exp1[j];
                exp1[j] = 0;
            }
            else
                exp2[j] = 0;
        }

        vrg_install(S);
        f = monom_prod(exp1, n, rmap);
        if (both)
        {
            g = monom_prod(exp2, n, rmap);
            p_sub(&f, &g);
            p_kill(&g);
        }
        gmInsert(N, f);
        vrg_install(R);
    }
    vrg_install(S);
    set_value(p, N);
    return 1;
} // 9/9/94 DB
