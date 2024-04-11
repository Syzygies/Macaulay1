/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"

// void h_init (hilbFunc *hf, int deg0);
// void h_addto (hilbFunc *hf, long genfun[], int deg0);
// void h_divAll (hilbFunc *hf);
void h_div (hilbFunc *hf);
// void h_display (hilbFunc *hf);
// long h_genus (hilbFunc *hf);
// void hilbDisplay (hilbFunc *hf, gmatrix M);
// void hilb_cmd (int argc, char *argv[]);
// void numer_tell (arrow head, int plus);
// void hilb_numer_cmd (int argc, char *argv[]);
// poly monom_prod (int exp[], int n, gmatrix rmap);
// void monoms_cmd (int argc, char *argv[]);
void binoms_cmd (int argc, char *argv[]);

extern gmatrix gm_in();
extern htell();

extern int genfun[];
extern int verbose;

void h_init (hilbFunc *hf, int deg0)
{
    int i;

    hf->codim = 0;
    hf->deg0 = deg0;
    hf->degree = 0;
    for (i=0; i<GFSIZE; i++)
        hf->genfun[i] = 0;
}

void h_addto (hilbFunc *hf, long genfun[], int deg0)
/* t^0 in genfun has this degree */
{
    int diff, i;

    /* deg0 >= hf->deg0 ALWAYS, or problems result */
    /* hf->codim SHOULD BE 0 */
    if (deg0 < hf->deg0) {
        prerror("\n;internal error: hilb addto: deg0 too small\n");
        to_shell();
    }
    if (hf->codim != 0) {
        prerror("\n;internal error: hilb addto: codim not zero\n");
        to_shell();
    }

    diff = deg0 - hf->deg0;
    for (i=0; i<GFSIZE-diff; i++) {
        hf->genfun[diff+i] += genfun[i];
        hf->degree += genfun[i];
    }
    while (i<GFSIZE) if (genfun[i++] != 0) {
            prerror("\n;error: hilb addto: degree bound exceeded!\n");
            to_shell();
        }
}

void h_divAll (hilbFunc *hf)
{
    while (hf->degree == 0)
        h_div(hf);
}

void h_div (hilbFunc *hf)
{
    int j, lo;
    lo = hf->codim;
    hf->degree += hf->genfun[GFSIZE-1] *= -1;
    for (j=GFSIZE-2; j>=lo; j--) {
        hf->genfun[j] = hf->genfun[j+1] - hf->genfun[j];
        hf->degree += hf->genfun[j];
    }
    hf->codim++;
}

void h_display (hilbFunc *hf)
{
    int i;

    fprint (outfile, "\n");
    for ( i=hf->codim; i<GFSIZE; ++i)
        if (hf->genfun[i] != 0) {
            fnewline(outfile);
            fprint (outfile, "%7ld t %2d\n",hf->genfun[i],
                    i + hf->deg0 - hf->codim);
        }
    if (hf->degree != 0) {
        fprint (outfile, "\n");
        fnewline(outfile);
        fprint(outfile, "codimension = %d\n", hf->codim);
        fnewline(outfile);
        fprint(outfile, "degree      = %ld\n", hf->degree);
    }
}

long h_genus (hilbFunc *hf)
{
    int i;
    long n, lo, genus;

    genus = 1;
    lo = hf->codim;
    for (i=lo; i<GFSIZE; i++)
        if ((n = hf->genfun[i]) != 0)
            genus += (i-lo-1)*n;
    return(genus);
}

boolean hilb(hf, M) /* returns TRUE if user doesn't interrupt */
hilbFunc *hf;
gmatrix M;
{
    int lodeg, hideg;
    int comp;
    arrow head;
    arrow monhilb();

    dl_lohi(&M->degrees, &lodeg, &hideg);
    h_init(hf, lodeg);
    for (comp=nrows(M); comp>=1; comp--) {
        if (verbose > 0) intflush("[%d] ", comp);
        i_genfun();
        if ((head = monhilb(M, comp, htell)) IS NULL)
            return(FALSE);
        h_addto(hf, genfun, DREF(M->degrees, comp));
        monrefund(head);
    }
    if (verbose > 0) print("\n");
    return(TRUE);
}

void hilbDisplay (hilbFunc *hf, gmatrix M)
{
    if (NOT hilb(hf, M)) return;
    h_display(hf);
    h_divAll(hf);
    h_display(hf);
    fnewline(outfile);
    fprint(outfile, "genus       = %ld\n", h_genus(hf));
}

void hilb_cmd (int argc, char *argv[])
{
    gmatrix M;
    hilbFunc hf;

    if (argc ISNT 2) {
        printnew("hilb <standard basis>\n");
        return;
    }
    GET_MOD(M, 1);
    stdWarning(M);
    hilbDisplay(&hf, M);
}

/*---------------------------------------------*/

static struct numer_rec {
    gmatrix rmap;
    variable *R, *S;
    int comp;
    poly result;
} nu;

void numer_tell (arrow head, int plus)
{
    int i, j, en;
    int *a;
    static unsigned int exp[NVARS];
    poly f, g;
    poly change1_vars();

    /* exp[] is current exponent vector */
    en = head->umh.mn;
    a = head->umh.mloc->umn.mexp;
    for ( i=0, j=numvars-en; i<en; ++i, ++j)
        exp[j] = a[i];

    /* make exp[] into poly f over R */
    vrg_install(nu.R);
    f = p_monom(fd_one);
    expToS(exp, nu.comp, INITIAL(f));

    /* map f to poly g over S */
    g = change1_vars(nu.rmap, nu.R, nu.S, f);

    /* add g to result */
    vrg_install(nu.S);
    if (plus)
        p_add (&nu.result, &g);
    else
        p_sub (&nu.result, &g);
    p_kill (&g);
    vrg_install(nu.R);
    p_kill (&f);

    if (verbose > 1) { /* print monom */
        print("%c (", plus ? '+' : '-');
        for ( i=0; i<numvars; ++i) {
            print("%s%d", i ? ", " : "", exp[i]);
        }
        print(")\n");
    }
} /* 8/26/93 DB */

boolean hilb_numer(M) /* returns TRUE if user doesn't interrupt */
gmatrix M;
{
    int comp;
    arrow head;
    arrow monhilb();
    poly f;

    nu.result = NULL;
    for (comp=nrows(M); comp>=1; comp--) {
        vrg_install(nu.S);
        f = e_sub_i(comp);
        p_add (&nu.result, &f);
        p_kill (&f);

        vrg_install(nu.R);
        nu.comp = comp;
        if (verbose > 0) intflush("[%d] ", comp);
        if ((head = monhilb(M, comp, numer_tell)) IS NULL)
            return(FALSE);
        monrefund(head);
    }
    if (verbose > 0) print("\n");
    return(TRUE);
} /* 8/26/93 DB */

void hilb_numer_cmd (int argc, char *argv[])
{
    gmatrix M, N;
    variable *p;

    if (argc != 4) {
        printnew("hilb_numer <standard basis> <ideal> <result>\n");
        return;
    }

    GET_MOD(M, 1);
    stdWarning(M);
    GET_CRING(nu.R);
    GET_MOD(nu.rmap, 2);
    GET_CRING(nu.S);
    NEW_MOD(p, 3);  /* will be over ring S */
    hilb_numer(M);

    vrg_install(nu.S);
    N = mod_init(); /* in ring S */
    dl_copy(&M->degrees, &N->degrees);
    gmInsert(N, nu.result);
    set_value(p, N);
} /* 8/26/93 DB */

/*---------------------------------------------*/

poly monom_prod (int exp[], int n, gmatrix rmap)
/* cribbed from change1_vars */
{
    poly f, g, h;
    int i, j;

    f = e_sub_i(1);
    for (i=0; i<n; ++i)
        if (exp[i] > 0)
        {
            g = PREF(rmap->gens, i+1);
            for (j=0; j<exp[i]; ++j)
            {
                h = p_mult(g, f);
                p_kill(&f);
                f = h;
            }
        }
    qrgReduce(&f);
    return(f);
} /* 9/9/94 DB */

void monoms_cmd (int argc, char *argv[])
{
    if (argc != 4) {
        printnew("monoms <matrix> <ideal> <result>\n");
        return;
    }

    *argv[0] = '\0';
    binoms_cmd(argc, argv);
} /* 9/9/94 DB */

void binoms_cmd (int argc, char *argv[])
/* This is here because it is cribbed from hilb_numer */
{
    gmatrix M, N, rmap;
    variable *R, *S, *p;
    poly f, g;
    int exp1[NVARS], exp2[NVARS];
    int n, i, j, comp, both;
    field coef;

    if (argc != 4) {
        printnew("binoms <matrix> <ideal> <result>\n");
        return;
    }
    both = *argv[0] != '\0'; /* binoms or monoms? */

    GET_MOD(M, 1);
    GET_CRING(R);
    n = nrows(M);

    GET_MOD(rmap, 2);
    GET_CRING(S);
    if (n > ncols(rmap))
    {
        prerror("; too few elements in <ideal>\n");
        return;
    }

    NEW_MOD(p, 3);  /* will be over ring S */

    vrg_install(S);
    N = mod_init();
    setzerodegs(N, 1);

    vrg_install(R);
    for (i=1; i<=ncols(M); ++i)
    {
        for (j=0; j<n; ++j) exp1[j] = 0;
        f = PREF(M->gens, i);
        while (f ISNT NULL)
        {
            coef = f->coef;
            comp = get_comp(f);
            exp1[comp-1] += coef;
            f = f->next;
        }
        for (j=0; j<n; ++j)
        {
            if (exp1[j] < 0)
            {
                exp2[j] = - exp1[j];
                exp1[j] = 0;
            }
            else exp2[j] = 0;
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
} /* 9/9/94 DB */
