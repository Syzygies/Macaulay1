/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"

// void stdWarning (gmatrix M);
// int stdNum (gmatrix M);
// void stdFirst (gmatrix M, modgen *mg, int isstd);
// poly stdNext (modgen *mg);
// gmatrix gm_elim (gmatrix M, int n, boolean doelim);
// void elim_cmd (int argc, char *argv[]);
// void keep_cmd (int argc, char *argv[]);
// gmatrix gm_in (gmatrix M, int n);
// void in_pprint (FILE *fil, term t, int comp);
// void put_in (FILE *fil, gmatrix M);
// void in_cmd (int argc, char *argv[]);
// boolean getVarList (int argc, char *argv[], expterm exp);
// boolean exp_eqcoef (expterm exp, expterm exp2, expterm vars);
// poly p_incoef (poly f, expterm vars);
// gmatrix gm_incoef (gmatrix M, expterm vars);
// void incoef_cmd (int argc, char *argv[]);
// void exp_inpart (expterm exp, expterm vars);
// poly p_inpart (poly f, expterm vars);
// gmatrix gm_stdpart (gmatrix M, expterm vars, boolean initialOnly);
// void stdpart_cmd (int argc, char *argv[]);
// void inpart_cmd (int argc, char *argv[]);
// void ins_stdmin (gmatrix result, gmatrix M, expterm nexp, int thiscomp, expterm vars);
// gmatrix gm_stdmin (gmatrix M, expterm vars);
// void stdmin_cmd (int argc, char *argv[]);
// int p_maxdiv (poly f, int n);
// poly p_vardiv (poly f, int n, int maxpower);
// gmatrix gm_vardiv (gmatrix M, int n, int maxpower);
// void sat_cmd (int argc, char *argv[]);
// gmatrix gm_std (gmatrix M, int isstd);
// void put_std (FILE *fil, gmatrix M, int isstd);
// gmatrix gm_force (gmatrix M, gmatrix Mchange);
// void forcestd_cmd (int argc, char *argv[]);

void stdWarning (gmatrix M)
{
    if (M IS NULL) return;
    if (M->modtype IS MMAT) {
        newline();
        print("warning: no standard basis. Using initial terms of");
        print(" matrix\n");
    }
}

int stdNum (gmatrix M)
{
    if (M IS NULL) return(0);
    if (M->modtype IS MMAT)
        return(ncols(M));
    else
        return(M->nstandard);
}

/* There are really 3 types of generators here:
 *  1. MSTD: standard basis exists.  Each time stdNext is called,
 *      the current element is issued, and then incremented.
 *  2. MISTD: inhomog. standard basis exists.  Each time stdNext
 *      is called, the current element is incremented, and THEN returned.
 *  3. MMAT: neither standard basis type is present.  Each time stdNext
 *      is called, ith entry is returned, and then i is incremented.
 */

void stdFirst (gmatrix M, modgen *mg, int isstd)
/* USESTD: try std basis first, then matrix itself */
/* USECHANGE: try change of basis matrix, then nothing */
/* USEMAT: use matrix generators, even if std basis exists */
{
    mg->misstd = isstd;
    if (M IS NULL)
        mg->mtype = MNOTHING;
    /*else /f ((isstd ISNT USEMAT) AND (M->modtype IS MSTD)) {
    mg->mtype = MSTD;
    mg->mp = M->stdbasis;
    }*/ else if ((isstd ISNT USEMAT) AND (M->modtype ISNT MMAT)) {
        mg->mtype = MISTD;
        mg->mmontab = &M->montab;
        mg->mi = 0;
        mg->mq = NULL;
    } else if (isstd IS USECHANGE) { /* but there isn't one so set to NOTHING*/
        mg->mtype = MNOTHING;
    } else {
        mg->mtype = MMAT;
        mg->mgens = &M->gens;
        mg->mi = 1;
    }
}

poly stdNext (modgen *mg)
{
    poly result;
    arrow head;
    mn_standard q;

    if (mg->mtype IS MNOTHING)
        return(NULL);
    /*if (mg->mtype IS MSTD) {
    if (mg->mp IS NULL) {
        mg->mtype = MNOTHING;
        return(NULL);
    }
    if (mg->misstd IS USESTD)
        result = mg->mp->standard;
    else
        result = mg->mp->change;
    mg->mp = mg->mp->next;
    return(result);
    } else*/ if (mg->mtype ISNT MMAT) {
        if (mg->mq ISNT NULL) {
            mg->mq = monnext(mg->mq);
            if (mg->mq ISNT NULL) {
                q = (mn_standard) mg->mq->umn.ldc.ci;
                if (mg->misstd IS USESTD)
                    result = q->standard;
                else
                    result = q->change;
                return(result);
            }
        }
        mg->mi++;
        for (; mg->mi <= length(mg->mmontab); mg->mi++) {
            head = * (arrow *) ref(mg->mmontab, mg->mi);
            if (head IS NULL) continue;
            monreset(head);
            mg->mq = monnext(head);
            if (mg->mq IS NULL) continue;
            q = (mn_standard) mg->mq->umn.ldc.ci;
            if (mg->misstd IS USESTD)
                return(q->standard);
            else
                return(q->change);
        }
        mg->mtype = MNOTHING;
        return(NULL);
    } else {  /* type = MMAT */
        if (mg->mi <= length(mg->mgens)) {
            result = PREF(*(mg->mgens), mg->mi);
            mg->mi++;
            return(result);
        }
        mg->mtype = MNOTHING;
        return(NULL);
    }
}

/*--- elimination --------------------------------------*/

gmatrix gm_elim (gmatrix M, int n, boolean doelim)
/* TRUE: elim, FALSE: keep */
{
    gmatrix N;
    modgen mg;
    poly f;
    boolean doit;

    N = mod_init();
    dl_copy(&M->degrees, &N->degrees);
    stdWarning(M);
    stdFirst(M, &mg, USESTD);
    while ((f=stdNext(&mg)) ISNT NULL) {
        doit = in_subring(INITIAL(f), n);
        if (NOT doelim)
            doit = NOT doit;
        if (doit) {
            f = p_copy(f);
            gmInsert(N, f);
        }
    }
    return(N);
}

void elim_cmd (int argc, char *argv[])
{
    gmatrix M;
    variable *p;
    int n;

    if ((argc < 3) OR (argc > 4)) {
        printnew("elim <standard basis> <result matrix>");
        print(" [n]\n");
        return;
    }
    GET_MOD(M, 1);
    if (argc IS 3)
        n = 1;
    else
        n = getInt(argv[3]);
    NEW_MOD(p, 2);
    set_value(p, gm_elim(M, n, TRUE));
}

void keep_cmd (int argc, char *argv[])
{
    gmatrix M;
    variable *p;
    int n;

    if ((argc < 3) OR (argc > 4)) {
        printnew("keep <standard basis> <result matrix>");
        print(" [n]\n");
        return;
    }
    GET_MOD(M, 1);
    if (argc IS 3)
        n = 1;
    else
        n = getInt(argv[3]);
    NEW_MOD(p, 2);
    set_value(p, gm_elim(M, n, FALSE));
}
/*--- initial submodules --------------------------------*/

gmatrix gm_in (gmatrix M, int n)
{
    gmatrix result;
    modgen mg;
    poly f;

    result = mod_init();
    dl_copy(&M->degrees, &result->degrees);
    stdWarning(M);
    stdFirst(M, &mg, USESTD);
    while ((f=stdNext(&mg)) ISNT NULL)
        gmInsert(result, p_in(f, n));
    return(result);
}

void in_pprint (FILE *fil, term t, int comp)
{
    if (comp ISNT tm_component(t))
        fprint(fil, "0");
    else
        tm_pprint(fil, t);
}

void put_in (FILE *fil, gmatrix M)
{
    modgen mg;
    poly f;
    int j, r;
    int c;

    r = nrows(M);
    c = stdNum(M);
    fnewline(fil);
    fprint(fil, "%d\n", r);
    fnewline(fil);
    fprint(fil, "%d\n", c);
    stdWarning(M);
    stdFirst(M, &mg, USESTD);
    while ((f=stdNext(&mg)) ISNT NULL) {
        for (j=1; j<=r; j++) {
            fnewline(fil);
            in_pprint(fil, INITIAL(f), j);
            fprint(fil, "\n");
        }
    }
    dlDisplay(fil, &M->degrees, linesize);
}

void in_cmd (int argc, char *argv[])
{
    gmatrix M;
    variable *p;
    int n;

    if ((argc < 2) OR (argc > 4)) {
        printnew("in <standard basis> [optional result matrix] [n]\n");
        return;
    }
    GET_MOD(M, 1);
    if (argc IS 2)
        put_in(outfile, M);
    else {
        if (argc IS 3)
            n = nblocks;
        else
            n = getInt(argv[3]);
        NEW_MOD(p, 2);
        set_value(p, gm_in(M, n));
    }
}
/*--- obtaining a variable list ------------------------------*/

boolean varLoHi(s, exp) /* returns TRUE if no error */
char *s;
expterm exp;
{
    int i, lo, hi;

    lo = parseVar(&s);
    if (*s IS '\0') hi = lo;
    else {
        if (*s IS '.') s++;
        if (*s ISNT '.')
            hi = lo-1;
        else {
            s++;
            hi = parseVar(&s);
        }
    }
    if ((lo < 0) OR (hi < 0) OR (hi < lo)) {
        prerror("; bad variable list\n");
        return(FALSE);
    }
    for (i=lo; i<=hi; i++) exp[i] = 1;
    return(TRUE);
}

boolean getVarList (int argc, char *argv[], expterm exp)
{
    int i;
    boolean flip;

    for (i=0; i<numvars; i++)
        exp[i] = 0;
    if (argc IS 0) {
        getfirstblock(exp);
        return(TRUE);
    }

    if (*argv[0] IS '!') {
        flip = TRUE;
        argc--;
        argv++;
    }
    else flip = FALSE;

    i = 0;
    while (i < argc) {
        if (*argv[i] IS '\\') {
            prinput("more variables");
            get_line(&argc, &argv);
            i = 0;
            continue;
        }
        if (NOT varLoHi(argv[i], exp)) return(FALSE);;
        i++;
    }
    if (flip) {
        for (i=0; i<numvars; i++)
            if (exp[i] IS 0)
                exp[i] = 1;
            else
                exp[i] = 0;
    }
    return(TRUE);
}

/*--- initial coefficients -----------------------------------*/

boolean exp_eqcoef (expterm exp, expterm exp2, expterm vars)
{
    int i;

    for (i=0; i<numvars; i++)
        if (vars[i] ISNT 0) {
            if (exp[i] ISNT exp2[i])
                return(FALSE);
            else
                exp2[i] = 0;
        }
    return(TRUE);
}

poly p_incoef (poly f, expterm vars)
/* returns the initial monomials of f having same "vars" component as initial term of f, except sets the "vars" variable exponents to 0 */
{
    term t;
    expterm exp, exp2;
    poly result, g;

    if (f IS NULL) return(NULL);
    t = (term) INITIAL(f);
    sToExp(t, exp);
    result = NULL;
    while (f ISNT NULL) {
        sToExp(INITIAL(f), exp2);
        if (NOT exp_eqcoef(exp, exp2, vars))
            break;
        g = p_monom(f->coef);
        expToS(exp2, get_comp(f), INITIAL(g));
        f = f->next;
        p_add(&result, &g);
    }
    return(result);
}

gmatrix gm_incoef (gmatrix M, expterm vars)
{
    gmatrix result;
    modgen mg;
    poly f;

    result = mod_init();
    dl_copy(&M->degrees, &result->degrees);
    stdWarning(M);
    stdFirst(M, &mg, USESTD);
    while ((f=stdNext(&mg)) ISNT NULL)
        gmInsert(result, p_incoef(f, vars));
    return(result);
}

void incoef_cmd (int argc, char *argv[])
{
    gmatrix M;
    variable *p;
    expterm vars;

    if (argc < 3)  {
        printnew("incoef <standard basis> <result matrix> [variable list]\n");
        return;
    }
    GET_MOD(M, 1);
    NEW_MOD(p, 2);
    if (NOT(getVarList(argc-3, argv+3, vars))) return;
    set_value(p, gm_incoef(M, vars));
}

/*--- initial ideals for "part" of the variables -------------*/

void exp_inpart (expterm exp, expterm vars)
{
    int i;

    for (i=0; i<=numvars; i++)
        if (vars[i] IS 0)
            exp[i] = 0;
}

poly p_inpart (poly f, expterm vars)
{
    poly result;
    expterm exp;

    if (f IS NULL) return(NULL);
    result = p_monom(f->coef);
    sToExp(INITIAL(f), exp);
    exp_inpart(exp, vars);
    expToS(exp, get_comp(f), INITIAL(result));
    return(result);
}

gmatrix gm_stdpart (gmatrix M, expterm vars, boolean initialOnly)
/* TRUE: only insert initial monomial, FALSE: whole polynomial */
{
    gmatrix result;
    int i;
    arrow p, head;
    modgen mg;
    expterm exp;
    poly f;

    result = mod_init();
    dl_copy(&M->degrees, &result->degrees);
    stdWarning(M);
    for (i=1; i<=nrows(M); i++) {
        head = monnewhead(numvars);
        stdFirst(M, &mg, USESTD);
        while ((f=stdNext(&mg)) ISNT NULL) {
            if (i ISNT get_comp(f)) continue;
            sToExp(INITIAL(f), exp);
            exp_inpart(exp, vars);
            monbagadjoin(head, exp, f);
        }
        p = head;
        while ((p=monnext(p)) ISNT NULL)  {
            f = ((poly) p->uld.ldc.ci);
            if (initialOnly)
                f = p_inpart(f, vars);
            else
                f = p_copy(f);
            gmInsert(result, f);
        }
        monrefund(head);
    }
    return(result);
}

void stdpart_cmd (int argc, char *argv[])
{
    gmatrix M;
    variable *p;
    expterm vars;

    if (argc < 3) {
        printnew("stdpart <standard basis> <result matrix> [variable list]\n");
        return;
    }
    GET_MOD(M, 1);
    NEW_MOD(p, 2);
    if (NOT(getVarList(argc-3, argv+3, vars))) return;
    set_value(p, gm_stdpart(M, vars, FALSE));
}

void inpart_cmd (int argc, char *argv[])
{
    gmatrix M;
    variable *p;
    expterm vars;

    if (argc < 3) {
        printnew("inpart <standard basis> <result matrix> [variable list]\n");
        return;
    }
    GET_MOD(M, 1);
    NEW_MOD(p, 2);
    if (NOT(getVarList(argc-3, argv+3, vars))) return;
    set_value(p, gm_stdpart(M, vars, TRUE));
}

/*--- finding subset of std basis whose in(f) are min. gens of in(I) ---*/

void ins_stdmin (gmatrix result, gmatrix M, expterm nexp, int thiscomp, expterm vars)
{
    modgen mg;
    poly f;
    expterm exp;

    stdFirst(M, &mg, USESTD);
    while ((f=stdNext(&mg)) ISNT NULL) {
        if (thiscomp ISNT get_comp(f)) continue;
        sToExp(INITIAL(f), exp);
        if (exp_eqcoef(nexp, exp, vars))
            gmInsert(result, p_copy(f));
    }
}

gmatrix gm_stdmin (gmatrix M, expterm vars)
{
    gmatrix result;
    int i;
    arrow p, head;
    modgen mg;
    poly f;
    expterm exp;

    result = mod_init();
    dl_copy(&M->degrees, &result->degrees);
    stdWarning(M);
    for (i=1; i<=nrows(M); i++) {
        head = monnewhead(numvars);
        stdFirst(M, &mg, USESTD);
        while ((f=stdNext(&mg)) ISNT NULL) {
            if (i ISNT get_comp(f)) continue;
            sToExp(INITIAL(f), exp);
            exp_inpart(exp, vars);
            monbagadjoin(head, exp, f);
        }
        p = head;
        while ((p=monnext(p)) ISNT NULL)  {
            ins_stdmin(result, M, p->umn.mexp, i, vars);
        }
        monrefund(head);
    }
    return(result);
}

void stdmin_cmd (int argc, char *argv[])
{
    gmatrix M;
    variable *p;
    expterm vars;

    if (argc < 3) {
        printnew(
            "std_minimal <standard basis> <result matrix> [variable list]\n");
        return;
    }
    GET_MOD(M, 1);
    NEW_MOD(p, 2);
    if (NOT(getVarList(argc-3, argv+3, vars))) return;
    set_value(p, gm_stdmin(M, vars));
}

/*--- saturation --------------------------------------------*/

int p_maxdiv (poly f, int n)
/* n=0..numvars-1, which variable to find max division of */
{
    int result;
    expterm exp;

    if (f IS NULL) return(0);
    sToExp(INITIAL(f), exp);
    result = exp[n];
    while ((f=f->next) ISNT NULL) {
        sToExp(INITIAL(f), exp);
        if (exp[n] < result)
            result = exp[n];
        if (result IS 0) return(0);
    }
    return(result);
}

poly p_vardiv (poly f, int n, int maxpower)
/* max. power of variable "n" to divide f by */
{
    expterm exp;
    poly result, g;

    result = NULL;
    while (f ISNT NULL) {
        sToExp(INITIAL(f), exp);
        if (maxpower <= exp[n])
            exp[n] -= maxpower;
        else
            exp[n] = 0;
        g = p_monom(f->coef);
        expToS(exp, get_comp(f), INITIAL(g));
        p_add(&result, &g);
        f = f->next;
    }
    return(result);
}

gmatrix gm_vardiv (gmatrix M, int n, int maxpower)
/* -1 means divide by as much as possible */
{
    gmatrix result;
    int i, max;
    poly f;
    modgen mg;

    result = mod_init();
    dl_copy(&M->degrees, &result->degrees);
    stdWarning(M);
    stdFirst(M, &mg, USESTD);
    max = -1;
    while ((f=stdNext(&mg)) ISNT NULL) {
        i = p_maxdiv(f,n);
        if ((maxpower > 0) AND (i > maxpower))
            i = maxpower;
        if (max IS -1) max = i;
        else max = MAX(max, i);
        gmInsert(result, p_vardiv(f, n, i));
    }

    if (max > 0) {
        newline();
        print("largest division was by %s", varnames[n]);
        if (max > 1) print("%d", max);
        print("\n");
    }
    return(result);
}

void sat_cmd (int argc, char *argv[])
{
    gmatrix M;
    variable *p;
    int maxpower, varnum;

    if ((argc < 3) OR (argc > 5)) {
        printnew("sat <standard basis> <result matrix> <variable> [max power]\n");
        return;
    }
    GET_MOD(M, 1);
    varnum = getVar(argv[3]);
    if (varnum IS -1) {
        prerror("; bad ring variable name\n");
        return;
    }
    NEW_MOD(p, 2);
    if (argc < 5) maxpower = -1;
    else maxpower = getInt(argv[4]);

    set_value(p, gm_vardiv(M, varnum, maxpower));
}

/*--- obtaining std, change ----------------------------------*/

gmatrix gm_std (gmatrix M, int isstd)
/* USESTD: pick off std basis, if none, then matrix */
/* USECHANGE: pick off change of basis matrix, if any */
{
    gmatrix result;
    modgen mg;
    poly f;

    result = mod_init();
    if (isstd IS USESTD)
        dl_copy(&M->degrees, &result->degrees);
    else
        dl_copy(&M->deggens, &result->degrees);
    stdFirst(M, &mg, isstd);
    while ((f=stdNext(&mg)) ISNT NULL)
        gmInsert(result, p_copy(f));
    return(result);
}

void put_std (FILE *fil, gmatrix M, int isstd)
/* USESTD: pick off std basis, if none, then matrix */
/* USECHANGE: pick off change of basis matrix, if any */
{
    int i, j, r;
    modgen mg;
    poly f;

    r = (isstd IS USESTD ? nrows(M) : ncols(M));
    fnewline(fil);
    fprint(fil,"%d\n",r);
    fnewline(fil);
    fprint(fil,"%d\n", stdNum(M));
    stdFirst(M, &mg, isstd);
    for (i=1; i<=stdNum(M); i++) {
        f=stdNext(&mg);
        for (j=1; j<=r; j++) {
            fnewline(fil);
            p_pprint(fil, f, j);
            fprint(fil, "\n");
        }
    }
}

/*--- force standard basis ----------------------*/

gmatrix gm_force (gmatrix M, gmatrix Mchange)
{
    gmatrix result;
    poly f, g;
    mn_standard q;
    modgen mg, mgchange;

    if (M IS NULL) return(NULL);
    if (Mchange ISNT NULL) {
        if (nrows(Mchange) > ncols(M)) {
            prerror("; change of basis matrix has too many rows\n");
            return(NULL);
        }
    }
    result = mod_init();
    if (check_homog(M))
        result->modtype = MSTD;
    else
        result->modtype = MISTD;

    dl_copy(&M->degrees, &result->degrees);
    stdFirst(M, &mg, USEMAT);
    stdFirst(Mchange, &mgchange, USEMAT);
    while ((f=stdNext(&mg)) ISNT NULL) {
        f = p_copy(f);
        gmInsert(result, f);
        g = stdNext(&mgchange);
        f = p_copy(f);
        g = p_copy(g);
        make2_monic(&f, &g);
        q = (mn_standard) get_slug(std_stash);
        q->standard = f;
        q->change = g;
        q->degree = degree(M, f);
        if (result->modtype IS MSTD) {
            q->next = result->stdbasis;
            result->stdbasis = q;
        } else
            q->next = NULL;
        result->nstandard++;
        mn_stdinsert(result, INITIAL(f), q);
    }
    return(result);
}

void forcestd_cmd (int argc, char *argv[])
{
    gmatrix M, Mchange;
    variable *p;

    if ((argc < 3) OR (argc > 4)) {
        printnew("forcestd <matrix> <result standard basis> [change of basis]\n");
        return;
    }
    GET_MOD(M, 1);
    if (argc IS 4) {
        GET_MOD(Mchange, 3);
    } else
        Mchange = NULL;
    NEW_MOD(p, 2);
    set_value(p, gm_force(M, Mchange));
}
