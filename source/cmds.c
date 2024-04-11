/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"

// gmatrix vget_mod (char *name);
// gmatrix vget_rgmod (char *name);
// variable *vget_ring (char *name);
// ring get_ring (char *name);
// void do_ring_vars ();
// void ring_cmd (int argc, char *argv[]);
// void ringsum_cmd (int argc, char *argv[]);
// void setring_cmd (int argc, char *argv[]);
// void pring_cmd (int argc, char *argv[]);
// void ringcol_cmd (int argc, char *argv[]);
// void ringrow_cmd (int argc, char *argv[]);
// void mat_cmd (int argc, char *argv[]);
// void sparse_cmd (int argc, char *argv[]);
// void ideal_cmd (int argc, char *argv[]);
// void poly_cmd (int argc, char *argv[]);
// void cpx_cmd (int argc, char *argv[]);
// void putstd_cmd (int argc, char *argv[]);
// void putchange_cmd (int argc, char *argv[]);
// void rowdegs_cmd (int argc, char *argv[]);
// void coldegs_cmd (int argc, char *argv[]);
// void putmat_cmd (int argc, char *argv[]);
// void pres_cmd (int argc, char *argv[]);
// void copy_cmd (int argc, char *argv[]);
// void numinfo_cmd (int argc, char *argv[]);
// void setdegs_cmd (int argc, char *argv[]);
// void setcoldegs_cmd (int argc, char *argv[]);
// void dshift_cmd (int argc, char *argv[]);
// void int_cmd (int argc, char *argv[]);
// void doIntCmd (char *name, int val, boolean setval, char *mess);
// void nrows_cmd (int argc, char *argv[]);
// void ncols_cmd (int argc, char *argv[]);
// void nvars_cmd (int argc, char *argv[]);
// void charac_cmd (int argc, char *argv[]);
// void rowdeg_cmd (int argc, char *argv[]);
// void coldeg_cmd (int argc, char *argv[]);
// void max_cmd (int argc, char *argv[]);
// void min_cmd (int argc, char *argv[]);
// void iszero_cmd (int argc, char *argv[]);
// void args_cmd (int argc, char *argv[]);

extern int prlevel;
extern gmatrix gm_std();
extern gmatrix gm_change();

/*
 *  vget_vmod  and  vget_mod  find the variable "name"  and returns
 *  a pointer to or its value respectively, if "name" refers to a
 *  module.  If not, NULL is returned.
 *  v_getrgmod is same as vget_mod, except that it returns NULL if
 *  the base ring of this var is NOT the current ring.
 */

variable *
vgetmod(name)   /* doesn't set base ring */
char *name;
{
    variable *p;

    p = find_var(name);
    if (p IS NULL) return(NULL);
    if (p->b_alias ISNT NULL)
        p = p->b_alias;
    if (!is_a_module(p)) {
        prerror("; variable isn't a module\n");
        return(NULL);
    }
    return(p);
}

variable *
vget_vmod(name)     /* sets base ring */
char *name;
{
    variable *p;

    p = find_var(name);
    if (p IS NULL) return(NULL);
    if (p->b_alias ISNT NULL)
        p = p->b_alias;
    if (!is_a_module(p)) {
        prerror("; variable isn't a module\n");
        return(NULL);
    } else {
        vrg_install(p->b_ring);
        return(p);
    }
}

gmatrix vget_mod (char *name)
{
    variable *p;

    p = vget_vmod(name);
    if (p IS NULL)
        return(NULL);
    else
        return(VAR_MODULE(p));
}

gmatrix vget_rgmod (char *name)
{
    variable *p;

    p = vgetmod(name);
    if (p IS NULL)
        return(NULL);
    if (p->b_ring ISNT current_ring) {
        prerror("; variable %s has different or incorrect base ring\n",
                name);
        return(NULL);
    }
    return(VAR_MODULE(p));
}

/* vget_ring: finds the variable with the given name.  If it is not a ring
 * then its base ring (if any) is returned.  In any case, NULL is returned
 * if there is any error (and an error message is generated).
 */

variable *vget_ring (char *name)
{
    variable *p;

    p = find_var(name);
    if (p IS NULL) return(NULL);
    if (p->type ISNT VRING) {
        p = p->b_ring;
        if (p IS NULL) {
            prerror("; variable %s has no base ring\n", name);
            return(NULL);
        }
    }
    ASSERT("; internal error: bad base ring", p->type IS VRING);
    vrg_install(p);
    return(p);
}

ring get_ring (char *name)
{
    variable *p;

    p = vget_ring(name);
    return(VAR_RING(p));
}

/* each time a ring is created (ring_cmd, ringsum_cmd,
   the following routine should be called: it creates two new variables:
   1. an ideal with all the variables in the ring, named same as the ring
      itself.
   2. a 0 by 0 matrix, named ring'zero, where "ring" is the name of the given
      ring.
   Thus, the ring no longer has its own name.
    This routine acts on the current ring: it should be called after
    vrg_install is done.
*/

void do_ring_vars ()
{
    variable *p, *q;
    char s[100];

    if (current_ring IS NULL) return;
    sprintf(s, "%s'zero", current_ring->name);
    p = make_var(s, MAINVAR, VMODULE, current_ring);
    set_value(p, mod_init());
    q = make_var(current_ring->name, MAINVAR, VMODULE, current_ring);
    set_value(q, gm_vars());
}

void ring_cmd (int argc, char *argv[])
{
    variable *p;
    char s[IDSIZE];
    ring R;

    if (argc ISNT 2) {
        printnew("ring <result ring>\n");
        return;
    }
    strcpy(s, argv[1]);
    R = rgScan();
    p = make_var(s, MAINVAR, VRING, NULL);
    if (p IS NULL) return;
    set_value(p, R);
    vrg_install(p);
    do_ring_vars();
}

void ringsum_cmd (int argc, char *argv[])
{
    variable *RR1, *RR2;
    ring R1, R2;
    variable *p, *q;

    if (argc ISNT 4) {
        printnew("ring_sum <ring1> <ring2> <result ring>\n");
        return;
    }
    GET_VRING(RR1, 1);
    GET_VRING(RR2, 2);
    R1 = VAR_RING(RR1);
    R2 = VAR_RING(RR2);
    NEW_RING(p, 3);
    /* we first create the ring without any quotient ideals (if any) */
    /* only after a variable has been created do we deal with this. */
    /* this is because we need a ring where to put the result quotient ideal */
    set_value(p, rgSum(R1, R2));
    vrg_install(p);
    if ((isQRing(RR1)) OR (isQRing(RR2))) {
        q = make_var(argv[3], MAINVAR, VRING, current_ring);
        if (q IS NULL) return;
        set_value(q, qrgSum(RR1, RR2, p));
        vrg_install(q); /* reinstall with quotient ideal in */
    }
    do_ring_vars();
}

void setring_cmd (int argc, char *argv[])
{
    variable *p;

    if (argc ISNT 2) {
        printnew("setring <new current ring>\n");
        return;
    }
    p = vget_ring(argv[1]);
    if (p IS NULL) return;
    vrg_install(p);
}

void pring_cmd (int argc, char *argv[])
{
    variable *p;

    if (argc IS 1) {
        newline();
        if (current_ring ISNT NULL) {
            print("current ring is %s\n", current_ring->name);
            rgDisplay(outfile, VAR_RING(current_ring));
        } else print("no current ring\n");
        return;
    }
    p = vget_ring(argv[1]);
    if (p IS NULL) return;
    newline();
    print("ring %s\n", p->name);
    rgDisplay(outfile, VAR_RING(p));
}

void ringcol_cmd (int argc, char *argv[])
{
    gmatrix M;
    char **vars;
    variable *p;
    char **getVars();
    char s[IDSIZE];
    ring R;
    ring rgFromDegrees();

    if (argc ISNT 3) {
        printnew("ring_from_cols <matrix> <result ring>\n");
        return;
    }
    GET_MOD(M, 1);
    if (ncols(M) IS 0) {
        prerror("; number of columns must be positive\n");
        return;
    }
    strcpy(s, argv[2]);
    vars = getVars(ncols(M));
    if (vars IS NULL) return;
    R = rgFromDegrees(&M->deggens, vars);
    NEW_savRING(p, s);
    set_value(p, R);
    vrg_install(p);
    do_ring_vars();
}

void ringrow_cmd (int argc, char *argv[])
{
    gmatrix M;
    char **vars;
    variable *p;
    char **getVars();
    char s[IDSIZE];
    ring R;
    ring rgFromDegrees();

    if (argc ISNT 3) {
        printnew("ring_from_rows <matrix> <result ring>\n");
        return;
    }
    GET_MOD(M, 1);
    if (nrows(M) IS 0) {
        prerror("; number of rows must be positive\n");
        return;
    }
    strcpy(s, argv[2]);
    vars = getVars(nrows(M));
    if (vars IS NULL) return;
    R = rgFromDegrees(&M->degrees, vars);
    NEW_RING(p, 2);
    set_value(p, R);
    vrg_install(p);
    do_ring_vars();
}

void mat_cmd (int argc, char *argv[])
{
    variable *p;
    int plev;
    gmatrix M;
    char s[IDSIZE];

    if ((argc < 2) OR (argc > 3)) {
        printnew("mat <result matrix> [optional: file name]\n");
        return;
    }
    if (current_ring IS NULL) {
        prerror("; no ring defined yet\n");
        return;
    }
    if (argc IS 3) {
        plev = prlevel; /* store previous value */
        if (open_tour(argv[2]) IS 0) return;
        prlevel = 1;
    }
    strcpy(s, argv[1]);
    M = mat_scan();
    NEW_savMOD(p, s);
    set_value(p, M);
    if (argc IS 3) prlevel = plev;
}

void sparse_cmd (int argc, char *argv[])
{
    variable *p;
    int plev;
    char s[IDSIZE];
    gmatrix M;
    gmatrix sparse_scan();

    if ((argc < 2) OR (argc > 3)) {
        printnew("sparse <result matrix> [optional: file name]\n");
        return;
    }
    if (current_ring IS NULL) {
        prerror("; no ring defined yet\n");
        return;
    }
    if (argc IS 3) {
        plev = prlevel; /* store previous value */
        if (open_tour(argv[2]) IS 0) return;
        prlevel = 1;
    }
    strcpy(s, argv[1]);
    M = sparse_scan();
    NEW_savMOD(p, s);
    set_value(p, M);
    if (argc IS 3) prlevel = plev;
}

void ideal_cmd (int argc, char *argv[])
{
    variable *p;
    gmatrix M;
    char s[IDSIZE];

    if (argc ISNT 2) {
        printnew("ideal <resulting matrix>\n");
        return;
    }
    if (current_ring IS NULL) {
        prerror("; no ring defined yet\n");
        return;
    }
    strcpy(s, argv[1]);
    M = ideal_scan();
    NEW_savMOD(p, s);
    set_value(p, M);
}

void poly_cmd (int argc, char *argv[])
{
    variable *p;
    gmatrix f;
    char s[IDSIZE];
    gmatrix poly_scan();

    if (argc < 3) {
        printnew("poly <result> <polynomial>\n");
        return;
    }
    if (current_ring IS NULL) {
        prerror("; no ring defined yet\n");
        return;
    }

    strcpy(s, argv[1]);

    f = poly_scan(argc-2, argv+2);

    NEW_savMOD(p, s);
    set_value(p, f);
}

void cpx_cmd (int argc, char *argv[])
{
    variable *p;
    gmatrix M;
    char s[20];
    int num;

    if (argc ISNT 2) {
        printnew("cpx <resulting complex>\n");
        return;
    }

    if (current_ring IS NULL) {
        prerror("; no ring defined yet\n");
        return;
    }
    p = make_var(argv[1], MAINVAR, VCOMPLEX, current_ring);
    if (p IS NULL) return;
    num = 0;
    while ((M = mat_scan()) ISNT NULL) {
        print("\n");
        sprintf(s, "%d\0", num);
        num++;
        p = make_var(s, PARTVAR, VMODULE, current_ring);
        set_value(p, M);
    }
}

void putstd_cmd (int argc, char *argv[])
{
    gmatrix M;
    variable *p;

    if ((argc < 2) OR (argc > 3)) {
        printnew("putstd <standard basis> [optional: result matrix]\n");
        return;
    }
    GET_MOD(M, 1);
    if (argc IS 2)
        put_std(outfile, M, USESTD);
    else {
        NEW_MOD(p, 2);
        set_value(p, gm_std(M, USESTD));
    }
}

void putchange_cmd (int argc, char *argv[])
{
    gmatrix M;
    variable *p;

    if ((argc < 2) OR (argc > 3)) {
        printnew("putchange <standard basis> [optional: result matrix]\n");
        return;
    }
    GET_MOD(M, 1);
    if (argc IS 2)
        put_std(outfile, M, USECHANGE);
    else {
        NEW_MOD(p, 2);
        set_value(p, gm_std(M, USECHANGE));
    }
}

void rowdegs_cmd (int argc, char *argv[])
{
    gmatrix M, N;
    variable *p;

    if ((argc < 2) OR (argc > 3)) {
        printnew("row_degs <matrix> [row degrees]\n");
        return;
    }
    GET_MOD(M, 1);
    if (argc IS 3) {
        NEW_MOD(p, 2);
        N = mod_init();
        dl_copy(&M->degrees, &N->degrees);
        set_value(p, N);
    } else {
        fnewline(outfile);
        dlDisplay(outfile, &(M->degrees), linesize);
    }
}

void coldegs_cmd (int argc, char *argv[])
{
    gmatrix M, N;
    variable *p;

    if ((argc < 2) OR (argc > 3)) {
        printnew("col_degs <matrix> [column degrees]\n");
        return;
    }
    GET_MOD(M, 1);
    if (argc IS 3) {
        NEW_MOD(p, 2);
        N = mod_init();
        dl_copy(&M->deggens, &N->degrees);
        set_value(p, N);
    } else {
        fnewline(outfile);
        dlDisplay(outfile, &(M->deggens), linesize);
    }
}

void putmat_cmd (int argc, char *argv[])
{
    gmatrix M;

    if (argc ISNT 2) {
        printnew("putmat <matrix>\n");
        return;
    }
    GET_MOD(M, 1);
    putmat(outfile, M);
}

void pres_cmd (int argc, char *argv[])
{
    gmatrix M;
    variable *p;
    int v;

    if (argc ISNT 2) {
        printnew("pres <complex> \n");
        return;
    }
    p = find_var(argv[1]);
    if (p IS NULL) return;
    v = p->var_num;
    p = VREF(v);
    do {
        if (is_a_module(p)) {
            if (p->name[0] ISNT '0') {  /* index of variable is zero */
                M = VAR_MODULE(p);
                if (ncols(M) IS 0) break;
                vrg_install(p->b_ring);
                fnewline(outfile);
                fprint(outfile,"\n");
                fnewline(outfile);
                fprint(outfile, "----------------------------------\n\n");
                pl_pprint(outfile, &(M->gens), nrows(M), linesize);
            }
        }
        v++;
        if (v > last_var) break;
        p = VREF(v);
    } while (p->exists IS PARTVAR);
    fnewline(outfile);
    fprint(outfile,"\n");
    fnewline(outfile);
    fprint(outfile, "----------------------------------\n\n");
}

void copy_cmd (int argc, char *argv[])
{
    gmatrix Mold;
    variable *newv;

    if (argc ISNT 3) {
        printnew("copy <existing matrix> <copy of matrix>\n");
        return;
    }
    GET_MOD(Mold, 1);
    NEW_MOD(newv, 2);
    set_value(newv, mat_copy(Mold));
}

void numinfo_cmd (int argc, char *argv[])
{
    gmatrix M;
    variable *p;
    int v;

    if (argc ISNT 2) {
        printnew("numinfo <res>\n");
        return;
    }
    p = find_var(argv[1]);
    if (p IS NULL) return;
    v = p->var_num;
    fnewline(outfile);
    fprint(outfile,"name    #rows #cols #standard  degrees\n");
    do {
        if (is_a_module(p)) {
            M = VAR_MODULE(p);
            if (ncols(M) IS 0) break;
            fnewline(outfile);
            fprint(outfile, "%-8s", p->name);
            fprint(outfile, "%-6d", nrows(M));
            fprint(outfile, "%-6d", ncols(M));
            fprint(outfile, "%-11d", M->nstandard);
            dlDisplay(outfile, &(M->deggens), MAX(10, linesize-31));
        }
        v++;
        if (v > last_var) break;
        p = VREF(v);
    } while (p->exists IS PARTVAR);
}

void setdegs_cmd (int argc, char *argv[])
{
    gmatrix M;

    if (argc ISNT 2) {
        printnew("setdegs <matrix>\n");
        return;
    }
    GET_MOD(M, 1);
    setdegs(M);
}

void setcoldegs_cmd (int argc, char *argv[])
{
    gmatrix M;

    if (argc ISNT 2) {
        printnew("setcoldegs <matrix>\n");
        return;
    }
    GET_MOD(M, 1);
    setdeggens(M);
}

void dshift_cmd (int argc, char *argv[])
{
    gmatrix M;
    int i, n;

    if (argc ISNT 3) {
        printnew("dshift <matrix> <degree to shift by>\n");
        return;
    }
    GET_MOD(M, 1);
    n = getInt(argv[2]);
    for (i=1; i<=length(&M->degrees); i++)
        DREF(M->degrees, i) += n;
    for (i=1; i<=length(&M->deggens); i++)
        DREF(M->deggens, i) += n;
}

void int_cmd (int argc, char *argv[])
{
    variable *p;
    int n;

    if (argc ISNT 3) {
        printnew("int <name> <new value>\n");
        return;
    }
    n = parseInt(argv+2);
    NEW_INT(p, 1);
    set_value(p, n);
}

void doIntCmd (char *name, int val, boolean setval, char *mess)
/* variable name to set, if setval = TRUE */
/* integer value */
/* TRUE: set value, FALSE: give message */
/* start of message, to use if setval = FALSE */
{
    variable *p;

    if (setval) {
        p = make_var(name, MAINVAR, VINT, NULL);
        if (p IS NULL) return;
        set_value(p, val);
    } else {
        newline();
        print("%s : %d\n", mess, val);
    }
}

void nrows_cmd (int argc, char *argv[])
{
    gmatrix M;
    int n;

    if ((argc < 2) OR (argc > 3)) {
        printnew("nrows <matrix> [result integer]\n");
        return;
    }
    GET_MOD(M, 1);
    n = nrows(M);
    doIntCmd(argv[2], n, (argc IS 3), "number of rows");
}

void ncols_cmd (int argc, char *argv[])
{
    gmatrix M;
    int n;

    if ((argc < 2) OR (argc > 3)) {
        printnew("ncols <matrix> [result integer]\n");
        return;
    }
    GET_MOD(M, 1);
    n = ncols(M);
    doIntCmd(argv[2], n, (argc IS 3), "number of columns");
}

void nvars_cmd (int argc, char *argv[])
{
    ring R;
    int n;

    if ((argc < 2) OR (argc > 3)) {
        printnew("nvars <ring> [result integer]\n");
        return;
    }
    GET_RING(R, 1);
    n = numvars;
    doIntCmd(argv[2], n, (argc IS 3), "number of variables in ring");
}

void charac_cmd (int argc, char *argv[])
{
    ring R;
    int n;

    if ((argc < 2) OR (argc > 3)) {
        printnew("characteristic <ring> [result integer]\n");
        return;
    }
    GET_RING(R, 1);
    n = charac;
    doIntCmd(argv[2], n, (argc IS 3), "characteristic of ring");
}

void rowdeg_cmd (int argc, char *argv[])
{
    gmatrix M;
    int n, r;

    if ((argc < 3) OR (argc > 4)) {
        printnew("row_degree <matrix> <row> [result integer]\n");
        return;
    }
    GET_MOD(M, 1);
    r = getInt(argv[2]);
    if ((r <= 0) OR (r > nrows(M)))
        n = 0;
    else
        n = DREF(M->degrees, r);

    doIntCmd(argv[3], n, (argc IS 4), "row degree");
}

void coldeg_cmd (int argc, char *argv[])
{
    gmatrix M;
    int n, c;

    if ((argc < 3) OR (argc > 4)) {
        printnew("col_degree <matrix> <column> [result integer]\n");
        return;
    }
    GET_MOD(M, 1);
    c = getInt(argv[2]);
    if ((c <= 0) OR (c > ncols(M)))
        n = 0;
    else
        n = DREF(M->deggens, c);

    doIntCmd(argv[3], n, (argc IS 4), "column degree");
}

void max_cmd (int argc, char *argv[])
{
    gmatrix M;

    if ((argc < 2) OR (argc > 3)) {
        printnew("max <matrix> [result maximum row degree]\n");
        return;
    }
    GET_MOD(M, 1);
    doIntCmd(argv[2], dl_max(&M->degrees), (argc IS 3), "maximum row degree");
}

void min_cmd (int argc, char *argv[])
{
    gmatrix M;

    if ((argc < 2) OR (argc > 3)) {
        printnew("min <matrix> [result minimum row degree]\n");
        return;
    }
    GET_MOD(M, 1);
    doIntCmd(argv[2], dl_min(&M->degrees), (argc IS 3), "minimum row degree");
}

void iszero_cmd (int argc, char *argv[])
{
    poly f;

    if (argc ISNT 3) {
        printnew("is_zero <poly> <result integer: 1 if zero, else 0>\n");
        return;
    }
    f = getPoly(argv[1], 1);
    doIntCmd(argv[2], f IS NULL, (argc IS 3), "");
}

extern cmd_rec cmd_list[]; /* table in cmdnames.c describing all commands */

void args_cmd (int argc, char *argv[])
{
    int i, args;
    char *p;

    if (argc == 2 || argc == 3) {
        for (p=argv[1]; *p!='\0'; ++p) if (*p == '-') *p = '_';
        i = cmd_lookup(argv[1], 1);
        if (i >= 0) {
            if (argc == 2) {
                printnew("%d\n", cmd_list[i].args);
            }
            else { /* argc == 3 */
                if (sscanf (argv[2], "%d", &args) && args >= 0)
                    cmd_list[i].args = args;
                else
                    prerror("; invalid argument to args: %s\n", argv[2]);
            }
        }
        else if (i == -1) {
            prerror("; command not found: %s\n", argv[1]);
        }
    }
    else {
        printnew("args <command name>        (to inspect)\n");
        printnew("args <command name> <int>  (to modify)\n");
    }
}
