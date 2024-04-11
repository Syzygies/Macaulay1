/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"

// poly change1_vars (gmatrix rmap, variable *R, variable *S, poly f);
// poly change_vars (gmatrix rmap, variable *R, variable *S, poly f);
// gmatrix mat_apply (gmatrix rmap, variable *R, variable *S, gmatrix M);
// gmatrix rmap_scan (variable *R, variable *S);
// void rmap_pprint (gmatrix rmap, variable *R, variable *S);
// gmatrix imap (variable *R, variable *S, boolean ones);
// void edit_rmap (gmatrix rmap, variable *R, variable *S);
// void rmap_cmd (int argc, char *argv[]);
// void imap_cmd (int argc, char *argv[]);
// void ev_cmd (int argc, char *argv[]);
// void toring_cmd (int argc, char *argv[]);
// void fetch_cmd (int argc, char *argv[]);
// void modrmap_cmd (int argc, char *argv[]);
// void pmap_cmd (int argc, char *argv[]);

extern poly p_xjei();
rmap_kill() {}

/*
 * Evaluates the monomial "f" via a map of polynomial rings
 * using the ideal "rmap" (over ring "S"), which defines a ring map
 * from R ---> S, by taking i th variable of R to the i th column
 * of "rmap".  If "rmap" has too few columns, it is (conceptually) padded
 * with zeros.
 *
 * Assumptions: ring(rmap) = S
 *      ring(f) = R.
 */

poly change1_vars (gmatrix rmap, variable *R, variable *S, poly f)
{
    field fcoef, rcoef;
    int fcomp, top, i, j, a;
    poly g, rpoly, rp, temp;
    expterm fexp, rexp, exp;

    /* first grab information from "f" */

    vrg_install(R);
    fcoef = f->coef;
    sToExp(INITIAL(f), fexp);
    fcomp = get_comp(f);
    top = MIN(ncols(rmap), numvars);
    for (i=top; i<numvars; i++)
        if (fexp[i] > 0) {
            vrg_install(S); /* on exit, ring S must be current ring */
            return(NULL);
        }

    /* now set up ring we will work in, and rcoef,rexp,rpoly */

    vrg_install(S);
    rcoef = normalize(fcoef);  /* in case characteristic has changed */
    rpoly = e_sub_i(fcomp);
    for (i=0; i<numvars; i++)
        rexp[i] = 0;

    /* now loop through and set up result */

    for (i=0; i<top; i++) {
        if ((a=fexp[i]) IS 0)
            continue;
        g = PREF(rmap->gens, i+1);
        if (g IS NULL) {
            p_kill(&rpoly);
            return(NULL);
        } else if (g->next IS NULL) {
            sToExp(INITIAL(g), exp);
            for (j=1; j<=a; j++)
                fd_mult(rcoef, g->coef, &rcoef);
            for (j=0; j<numvars; j++)
                rexp[j] += a*exp[j];
        } else {
            /* else g has at least two terms */
            for (j=1; j<=a; j++) {
                temp = p_mult(g, rpoly);
                p_kill(&rpoly);
                rpoly = temp;
            }
        }
        if ((rcoef IS 0) OR (rpoly IS NULL)) {
            p_kill(&rpoly);
            return(NULL);
        }
    }

    /* finally, piece together rcoef, rexp, and rpoly */

    rp = rpoly;
    while (rp ISNT NULL) {
        fd_mult(rcoef, rp->coef, &rp->coef);
        sToExp(INITIAL(rp), exp);
        for (j=0; j<numvars; j++)
            exp[j] += rexp[j];
        expToS(exp, fcomp, INITIAL(rp));
        rp = rp->next;
    }
    qrgReduce(&rpoly); /* in case we are in a quotient ring */
    return(rpoly);
}

poly change_vars (gmatrix rmap, variable *R, variable *S, poly f)
{
    poly result, temp;

    result = NULL;
    while (f ISNT NULL) {
        temp = change1_vars(rmap, R, S, f);
        p_add(&result, &temp);
        f = f->next;
    }
    return(result);
}

gmatrix mat_apply (gmatrix rmap, variable *R, variable *S, gmatrix M)
{
    int i;
    poly f;
    gmatrix result;

    vrg_install(S);
    result = mod_init(); /* in ring S */
    dl_copy(&M->degrees, &result->degrees);
    for (i=1; i<=ncols(M); i++) {
        f = change_vars(rmap, R, S, PREF(M->gens, i));
        gmInsert(result, f);
    }
    return(result);
}

/*------------------------------------------------------------*/

gmatrix rmap_scan (variable *R, variable *S)
{
    gmatrix result;
    int i, n;
    ring RR;

    RR = VAR_RING(R);
    n = nvars(RR);

    vrg_install(S);
    result = mod_init();
    dl_new(&result->degrees, 1);
    for (i=0; i<n; i++) {
        prinput("%s --->", varName(RR, i));
        gmInsert(result, rdPoly(1));
    }
    return(result);
}

void rmap_pprint (gmatrix rmap, variable *R, variable *S)
/* ring(rmap)=S */
{
    ring RR;
    int i, n;

    fnewline(outfile);
    fprint(outfile, "map : %s ---> %s\n", R->name, S->name);
    vrg_install(S);
    RR = VAR_RING(R);
    n = nvars(RR);
    for (i=0; i<n; i++) {
        fnewline(outfile);
        fprint(outfile, "%s |--> ", varName(RR, i));
        if (i >= ncols(rmap))
            fprint(outfile, "0");
        else
            p_pprint(outfile, PREF(rmap->gens, i+1), 1);
        fprint(outfile, "\n");
    }
}

gmatrix imap (variable *R, variable *S, boolean ones)
{
    ring RR, SS;
    gmatrix result;
    int v, w, n;

    vrg_install(S);
    RR = VAR_RING(R);
    SS = VAR_RING(S);
    n = nvars(RR);
    result = mod_init();
    dl_new(&result->degrees, 1);
    for (v=0; v<n; v++) {
        w = getVar(varName(RR, v));
        if (w IS -1) {
            if (ones)
                gmInsert(result, e_sub_i(1));
            else
                gmInsert(result, NULL);
        } else
            gmInsert(result, p_xjei(w, 1));
    }
    return(result);
}

void edit_rmap (gmatrix rmap, variable *R, variable *S)
{
    int argc, v;
    char **argv;
    poly f;
    poly rdPolyStr();

    vrg_install(S);
    while (TRUE) {
        prinput("variable, new image");
        get_line(&argc, &argv);
        if (argc < 2) break;
        v = 1 + getRingVar(VAR_RING(R), argv[0]);
        if (v IS 0) {
            prerror("; variable %s not defined\n", argv[0]);
            continue;
        }
        f = rdPolyStr(argc-1, argv+1, 1);
        while (v > ncols(rmap))  /* in case ideal "rmap" not long enough */
            gmInsert(rmap, NULL);
        p_kill(&PREF(rmap->gens, v));
        PREF(rmap->gens, v) = f;
    }
}

/*--------------------------------------------------------------*/

void rmap_cmd (int argc, char *argv[])
{
    variable *R, *S, *p;
    char s[IDSIZE];
    gmatrix M;

    if (argc ISNT 4) {
        printnew("rmap <result> <R> <S>\n");
        return;
    }
    GET_VRING(R, 2);
    GET_VRING(S, 3);
    strcpy(s, argv[1]);
    M = rmap_scan(R, S);
    NEW_savMOD(p, s);  /* result is ideal in ring S */
    set_value(p, M);
}

void imap_cmd (int argc, char *argv[])
{
    variable *R, *S;
    variable *p;
    gmatrix f;
    char s[IDSIZE];

    if ((argc < 4) OR (argc > 5)) {
        printnew("imap <new ring map> <R> <S> [ones, default=zeros]\n");
        return;
    }
    GET_VRING(R, 2);
    GET_VRING(S, 3);
    strcpy(s, argv[1]);
    f = imap(R, S, (argc IS 5));
    edit_rmap(f, R, S);
    NEW_savMOD(p, s); /* result is ideal in ring S */
    set_value(p, f);
}

void ev_cmd (int argc, char *argv[])
{
    gmatrix M, rmap;
    variable *p, *R, *S;

    if (argc ISNT 4) {
        printnew("ev <ideal> <matrix> <changed matrix>\n");
        return;
    }
    GET_MOD(M, 2);
    GET_CRING(R);

    GET_MOD(rmap, 1);
    GET_CRING(S);

    NEW_MOD(p, 3);  /* will be over ring S */
    set_value(p, mat_apply(rmap, R, S, M));
}
/*
void toring_cmd (int argc, char *argv[])
{
    variable *S, *R, *p;
    gmatrix g, M;
    char s[IDSIZE];

    if ((argc < 3) OR (argc > 4)) {
    printnew("to-ring <matrix> <result matrix> [ones, default=zeros]\n");
    return;
    }
    GET_CRING(S);
    GET_MOD(M, 1);
    GET_CRING(R);
    vrg_install(S);
    strcpy(s, argv[2]);
    g = imap(R, S, (argc IS 4));
    edit_rmap(g, R, S);
    NEW_savMOD(p, s);
    set_value(p, mat_apply(g, R, S, M));
    mod_kill(g);
}
*/

void fetch_cmd (int argc, char *argv[])
{
    variable *S, *R, *p;
    gmatrix g, M;
    char s[IDSIZE];

    if ((argc < 3) OR (argc > 4)) {
        printnew("fetch <matrix> <result matrix> [ones, default=zeros]\n");
        return;
    }
    GET_CRING(S);
    GET_MOD(M, 1);
    GET_CRING(R);
    vrg_install(S);
    strcpy(s, argv[2]);
    g = imap(R, S, (argc IS 4));
    NEW_savMOD(p, s);  /* matrix will be over ring S */
    set_value(p, mat_apply(g, R, S, M));
    mod_kill(g);
}

void modrmap_cmd (int argc, char *argv[])
{
    variable *R, *S;
    gmatrix g;

    if ((argc < 2) OR (argc > 3)) {
        printnew("edit_map <ideal> [source ring]\n");
        return;
    }

    if (argc IS 2)
        GET_CRING(R)
        else
            GET_VRING(R, 2);

    GET_MOD(g, 1);
    GET_CRING(S);

    edit_rmap(g, R, S);
}

void pmap_cmd (int argc, char *argv[])
{
    variable *R, *S;
    gmatrix g;

    if ((argc < 2) OR (argc > 3)) {
        printnew("pmap <ideal> [source ring]\n");
        return;
    }
    if (argc IS 2)
        GET_CRING(R)
        else
            GET_VRING(R, 2);

    GET_MOD(g, 1);
    GET_CRING(S);

    rmap_pprint(g, R, S);
}
