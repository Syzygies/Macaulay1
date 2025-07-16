// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include "shared.h"
#include "cmds.h"
#include "vars.h"
#include "ring.h"
#include "gmatrix.h"
#include "monitor.h"
#include "input.h"
#include "parse.h"
#include "human_io.h"
#include "qring.h"
#include "poly.h"
#include "standard.h"
#include "cmdnames.h"
#include "set.h"
#include "shell.h"   // for outfile, cmd_lookup
#include "charp.h"   // for charac
#include "plist.h"   // for mod_init, dl_copy, mat_copy
#include "stdcmds.h" // for gm_std, put_std
#include "generic.h" // for variable type constants

#include "gm_dlist.h"  // for dlDisplay, dl_max, dl_min
#include "rvar.h"      // for getVars
#include "parsepoly.h" // for getPoly

// Constants
constexpr int IDSIZE = 50;
// MAINVAR and PARTVAR now come from shared.h
// Variable type constants now come from generic.h

// Inline functions to replace macros
static inline gmatrix VAR_MODULE(variable *p)
{
    return var_get_module(p);
}

static inline ring VAR_RING(variable *p)
{
    return var_get_ring(p);
}

static inline variable *VREF(int n)
{
    return vref(n);
}

static inline int DREF(dlist dl, int i)
{
    return dlist_ref(&dl, i);
}

// NOTE: gm_change() is called but not defined anywhere - may need investigation
// FORWARD DECLARATION REMOVED - function never used, possibly dead code

// GET_* inline functions to replace macros
static inline bool get_mod(gmatrix *g, int argc, char **argv, int i)
{
    if (i >= argc)
        return false;
    *g = vget_mod(argv[i]);
    return (*g != NULL);
}

static inline bool get_vring(variable **p, int argc, char **argv, int i)
{
    if (i >= argc)
        return false;
    *p = vget_ring(argv[i]);
    return (*p != NULL);
}

static inline bool get_ring_arg(ring *R, int argc, char **argv, int i)
{
    if (i >= argc)
        return false;
    *R = get_ring(argv[i]);
    return (*R != NULL);
}

static inline bool new_ring(variable **p, int argc, char **argv, int i)
{
    if (i >= argc)
        return false;
    *p = make_var(argv[i], MAINVAR, VRING, NULL);
    return (*p != NULL);
}

static inline bool new_mod(variable **p, int argc, char **argv, int i)
{
    if (i >= argc)
        return false;
    *p = make_var(argv[i], MAINVAR, VMODULE, current_ring);
    return (*p != NULL);
}

static inline bool new_int(variable **p, int argc, char **argv, int i)
{
    if (i >= argc)
        return false;
    *p = make_var(argv[i], MAINVAR, VINT, NULL);
    return (*p != NULL);
}

static inline bool new_savring(variable **p, char *s)
{
    *p = make_var(s, MAINVAR, VRING, NULL);
    return (*p != NULL);
}

static inline bool new_savmod(variable **p, char *s)
{
    *p = make_var(s, MAINVAR, VMODULE, current_ring);
    return (*p != NULL);
}

// All macros have been converted to inline functions above

// vget_vmod  and  vget_mod  find the variable "name"  and returns
// a pointer to or its value respectively, if "name" refers to a
// module.  If not, NULL is returned.
// v_getrgmod is same as vget_mod, except that it returns NULL if
// the base ring of this var is NOT the current ring.

variable *vgetmod(const char *name) // doesn't set base ring
{
    variable *p;

    p = find_var(name);
    if (p == NULL)
        return (NULL);
    if (p->b_alias != NULL)
        p = p->b_alias;
    if (!is_a_module(p))
    {
        prerror("; variable isn't a module\n");
        return (NULL);
    }
    return (p);
}

variable *vget_vmod(const char *name) // sets base ring
{
    variable *p;

    p = find_var(name);
    if (p == NULL)
        return (NULL);
    if (p->b_alias != NULL)
        p = p->b_alias;
    if (!is_a_module(p))
    {
        prerror("; variable isn't a module\n");
        return (NULL);
    }
    else
    {
        vrg_install(p->b_ring);
        return (p);
    }
}

gmatrix vget_mod(const char *name)
{
    variable *p;

    p = vget_vmod(name);
    if (p == NULL)
        return (NULL);
    else
        return (VAR_MODULE(p));
}

gmatrix vget_rgmod(const char *name)
{
    variable *p;

    p = vgetmod(name);
    if (p == NULL)
        return (NULL);
    if (p->b_ring != current_ring)
    {
        prerror("; variable %s has different or incorrect base ring\n", name);
        return (NULL);
    }
    return (VAR_MODULE(p));
}

// vget_ring: finds the variable with the given name.  If it is not a ring
// then its base ring (if any) is returned.  In any case, NULL is returned
// if there is any error (and an error message is generated).

variable *vget_ring(const char *name)
{
    variable *p;

    p = find_var(name);
    if (p == NULL)
        return (NULL);
    if (p->type != VRING)
    {
        p = p->b_ring;
        if (p == NULL)
        {
            prerror("; variable %s has no base ring\n", name);
            return (NULL);
        }
    }
    debug_assert("; internal error: bad base ring", p->type == VRING);
    vrg_install(p);
    return (p);
}

ring get_ring(const char *name)
{
    variable *p;

    p = vget_ring(name);
    return (VAR_RING(p));
}

// each time a ring is created (ring_cmd, ringsum_cmd,
// the following routine should be called: it creates two new variables:
// 1. an ideal with all the variables in the ring, named same as the ring
// itself.
// 2. a 0 by 0 matrix, named ring'zero, where "ring" is the name of the given
// ring.
// Thus, the ring no longer has its own name.
// This routine acts on the current ring: it should be called after
// vrg_install is done.

int do_ring_vars(void)
{
    variable *p, *q;
    char s[100];

    if (current_ring == NULL)
        return 1;
    sprintf(s, "%s'zero", current_ring->name);
    p = make_var(s, MAINVAR, VMODULE, current_ring);
    set_value(p, (ADDRESS)mod_init());
    q = make_var(current_ring->name, MAINVAR, VMODULE, current_ring);
    set_value(q, (ADDRESS)gm_vars());
    return 1;
}

int ring_cmd(int argc, char *argv[])
{
    variable *p;
    char s[IDSIZE];
    ring R;

    if (argc != 2)
    {
        printnew("ring <result ring>\n");
        return 0;
    }
    strcpy(s, argv[1]);
    R = rgScan();
    p = make_var(s, MAINVAR, VRING, NULL);
    if (p == NULL)
        return 0;
    set_value(p, (ADDRESS)R);
    vrg_install(p);
    do_ring_vars();
    return 1;
}

int ringsum_cmd(int argc, char *argv[])
{
    variable *RR1, *RR2;
    ring R1, R2;
    variable *p, *q;

    if (argc != 4)
    {
        printnew("ring_sum <ring1> <ring2> <result ring>\n");
        return 0;
    }
    if (!get_vring(&RR1, argc, argv, 1))
        return 0;
    if (!get_vring(&RR2, argc, argv, 2))
        return 0;
    R1 = VAR_RING(RR1);
    R2 = VAR_RING(RR2);
    if (!new_ring(&p, argc, argv, 3))
        return 0;
    // we first create the ring without any quotient ideals (if any)
    // only after a variable has been created do we deal with this.
    // this is because we need a ring where to put the result quotient ideal
    set_value(p, (ADDRESS)rgSum(R1, R2));
    vrg_install(p);
    if ((isQRing(RR1)) || (isQRing(RR2)))
    {
        q = make_var(argv[3], MAINVAR, VRING, current_ring);
        if (q == NULL)
            return 0;
        set_value(q, (ADDRESS)qrgSum(RR1, RR2, p));
        vrg_install(q); // reinstall with quotient ideal in
    }
    do_ring_vars();
    return 1;
}

int setring_cmd(int argc, char *argv[])
{
    variable *p;

    if (argc != 2)
    {
        printnew("setring <new current ring>\n");
        return 0;
    }
    p = vget_ring(argv[1]);
    if (p == NULL)
        return 0;
    vrg_install(p);
    return 1;
}

int pring_cmd(int argc, char *argv[])
{
    variable *p;

    if (argc == 1)
    {
        newline();
        if (current_ring != NULL)
        {
            print("current ring is %s\n", current_ring->name);
            rgDisplay(outfile, VAR_RING(current_ring));
        }
        else
            print("no current ring\n");
        return 1;
    }
    p = vget_ring(argv[1]);
    if (p == NULL)
        return 0;
    newline();
    print("ring %s\n", p->name);
    rgDisplay(outfile, VAR_RING(p));
    return 1;
}

int ringcol_cmd(int argc, char *argv[])
{
    gmatrix M;
    char **vars;
    variable *p;
    char s[IDSIZE];
    ring R;

    if (argc != 3)
    {
        printnew("ring_from_cols <matrix> <result ring>\n");
        return 0;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    if (ncols(M) == 0)
    {
        prerror("; number of columns must be positive\n");
        return 0;
    }
    strcpy(s, argv[2]);
    vars = getVars(ncols(M));
    if (vars == NULL)
        return 0;
    R = rgFromDegrees(&M->deggens, vars);
    if (!new_savring(&p, s))
        return 0;
    set_value(p, (ADDRESS)R);
    vrg_install(p);
    do_ring_vars();
    return 1;
}

int ringrow_cmd(int argc, char *argv[])
{
    gmatrix M;
    char **vars;
    variable *p;
    char s[IDSIZE];
    ring R;

    if (argc != 3)
    {
        printnew("ring_from_rows <matrix> <result ring>\n");
        return 0;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    if (nrows(M) == 0)
    {
        prerror("; number of rows must be positive\n");
        return 0;
    }
    strcpy(s, argv[2]);
    vars = getVars(nrows(M));
    if (vars == NULL)
        return 0;
    R = rgFromDegrees(&M->degrees, vars);
    if (!new_ring(&p, argc, argv, 2))
        return 0;
    set_value(p, (ADDRESS)R);
    vrg_install(p);
    do_ring_vars();
    return 1;
}

int mat_cmd(int argc, char *argv[])
{
    variable *p;
    int plev;
    gmatrix M;
    char s[IDSIZE];

    if ((argc < 2) || (argc > 3))
    {
        printnew("mat <result matrix> [optional: file name]\n");
        return 0;
    }
    if (current_ring == NULL)
    {
        prerror("; no ring defined yet\n");
        return 0;
    }
    if (argc == 3)
    {
        plev = prlevel; // store previous value
        if (open_tour(argv[2]) == 0)
            return 0;
        prlevel = 1;
    }
    strcpy(s, argv[1]);
    M = mat_scan();
    if (!new_savmod(&p, s))
        return 0;
    set_value(p, (ADDRESS)M);
    if (argc == 3)
        prlevel = plev;
    return 1;
}

int sparse_cmd(int argc, char *argv[])
{
    variable *p;
    int plev;
    char s[IDSIZE];
    gmatrix M;

    if ((argc < 2) || (argc > 3))
    {
        printnew("sparse <result matrix> [optional: file name]\n");
        return 0;
    }
    if (current_ring == NULL)
    {
        prerror("; no ring defined yet\n");
        return 0;
    }
    if (argc == 3)
    {
        plev = prlevel; // store previous value
        if (open_tour(argv[2]) == 0)
            return 0;
        prlevel = 1;
    }
    strcpy(s, argv[1]);
    M = sparse_scan();
    if (!new_savmod(&p, s))
        return 0;
    set_value(p, (ADDRESS)M);
    if (argc == 3)
        prlevel = plev;
    return 1;
}

int ideal_cmd(int argc, char *argv[])
{
    variable *p;
    gmatrix M;
    char s[IDSIZE];

    if (argc != 2)
    {
        printnew("ideal <resulting matrix>\n");
        return 0;
    }
    if (current_ring == NULL)
    {
        prerror("; no ring defined yet\n");
        return 0;
    }
    strcpy(s, argv[1]);
    M = ideal_scan();
    if (!new_savmod(&p, s))
        return 0;
    set_value(p, (ADDRESS)M);
    return 1;
}

int poly_cmd(int argc, char *argv[])
{
    variable *p;
    gmatrix f;
    char s[IDSIZE];

    if (argc < 3)
    {
        printnew("poly <result> <polynomial>\n");
        return 0;
    }
    if (current_ring == NULL)
    {
        prerror("; no ring defined yet\n");
        return 0;
    }

    strcpy(s, argv[1]);

    f = poly_scan(argc - 2, argv + 2);

    if (!new_savmod(&p, s))
        return 0;
    set_value(p, (ADDRESS)f);
    return 1;
}

int cpx_cmd(int argc, char *argv[])
{
    variable *p;
    gmatrix M;
    char s[20];
    int num;

    if (argc != 2)
    {
        printnew("cpx <resulting complex>\n");
        return 0;
    }

    if (current_ring == NULL)
    {
        prerror("; no ring defined yet\n");
        return 0;
    }
    p = make_var(argv[1], MAINVAR, VCOMPLEX, current_ring);
    if (p == NULL)
        return 0;
    num = 0;
    while ((M = mat_scan()) != NULL)
    {
        print("\n");
        sprintf(s, "%d", num);
        num++;
        p = make_var(s, PARTVAR, VMODULE, current_ring);
        set_value(p, (ADDRESS)M);
    }
    return 1;
}

int putstd_cmd(int argc, char *argv[])
{
    gmatrix M;
    variable *p;

    if ((argc < 2) || (argc > 3))
    {
        printnew("putstd <standard basis> [optional: result matrix]\n");
        return 0;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    if (argc == 2)
        put_std(outfile, M, USESTD);
    else
    {
        if (!new_mod(&p, argc, argv, 2))
            return 0;
        set_value(p, (ADDRESS)gm_std(M, USESTD));
    }
    return 1;
}

int putchange_cmd(int argc, char *argv[])
{
    gmatrix M;
    variable *p;

    if ((argc < 2) || (argc > 3))
    {
        printnew("putchange <standard basis> [optional: result matrix]\n");
        return 0;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    if (argc == 2)
        put_std(outfile, M, USECHANGE);
    else
    {
        if (!new_mod(&p, argc, argv, 2))
            return 0;
        set_value(p, (ADDRESS)gm_std(M, USECHANGE));
    }
    return 1;
}

int rowdegs_cmd(int argc, char *argv[])
{
    gmatrix M, N;
    variable *p;

    if ((argc < 2) || (argc > 3))
    {
        printnew("row_degs <matrix> [row degrees]\n");
        return 0;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    if (argc == 3)
    {
        if (!new_mod(&p, argc, argv, 2))
            return 0;
        N = mod_init();
        dl_copy(&M->degrees, &N->degrees);
        set_value(p, (ADDRESS)N);
    }
    else
    {
        fnewline(outfile);
        dlDisplay(outfile, &(M->degrees), linesize);
    }
    return 1;
}

int coldegs_cmd(int argc, char *argv[])
{
    gmatrix M, N;
    variable *p;

    if ((argc < 2) || (argc > 3))
    {
        printnew("col_degs <matrix> [column degrees]\n");
        return 0;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    if (argc == 3)
    {
        if (!new_mod(&p, argc, argv, 2))
            return 0;
        N = mod_init();
        dl_copy(&M->deggens, &N->degrees);
        set_value(p, (ADDRESS)N);
    }
    else
    {
        fnewline(outfile);
        dlDisplay(outfile, &(M->deggens), linesize);
    }
    return 1;
}

int putmat_cmd(int argc, char *argv[])
{
    gmatrix M;

    if (argc != 2)
    {
        printnew("putmat <matrix>\n");
        return 0;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    putmat(outfile, M);
    return 1;
}

int pres_cmd(int argc, char *argv[])
{
    gmatrix M;
    variable *p;
    int v;

    if (argc != 2)
    {
        printnew("pres <complex> \n");
        return 0;
    }
    p = find_var(argv[1]);
    if (p == NULL)
        return 0;
    v = p->var_num;
    p = VREF(v);
    do
    {
        if (is_a_module(p))
        {
            if (p->name[0] != '0')
            { // index of variable is zero
                M = VAR_MODULE(p);
                if (ncols(M) == 0)
                    break;
                vrg_install(p->b_ring);
                fnewline(outfile);
                fprint(outfile, "\n");
                fnewline(outfile);
                fprint(outfile, "----------------------------------\n\n");
                pl_pprint(outfile, &(M->gens), nrows(M), linesize);
            }
        }
        v++;
        if (v > last_var)
            break;
        p = VREF(v);
    } while (p->exists == PARTVAR);
    fnewline(outfile);
    fprint(outfile, "\n");
    fnewline(outfile);
    fprint(outfile, "----------------------------------\n\n");
    return 1;
}

int copy_cmd(int argc, char *argv[])
{
    gmatrix Mold;
    variable *newv;

    if (argc != 3)
    {
        printnew("copy <existing matrix> <copy of matrix>\n");
        return 0;
    }
    if (!get_mod(&Mold, argc, argv, 1))
        return 0;
    if (!new_mod(&newv, argc, argv, 2))
        return 0;
    set_value(newv, (ADDRESS)mat_copy(Mold));
    return 1;
}

int numinfo_cmd(int argc, char *argv[])
{
    gmatrix M;
    variable *p;
    int v;

    if (argc != 2)
    {
        printnew("numinfo <res>\n");
        return 0;
    }
    p = find_var(argv[1]);
    if (p == NULL)
        return 0;
    v = p->var_num;
    fnewline(outfile);
    fprint(outfile, "name    #rows #cols #standard  degrees\n");
    do
    {
        if (is_a_module(p))
        {
            M = VAR_MODULE(p);
            if (ncols(M) == 0)
                break;
            fnewline(outfile);
            fprint(outfile, "%-8s", p->name);
            fprint(outfile, "%-6d", nrows(M));
            fprint(outfile, "%-6d", ncols(M));
            fprint(outfile, "%-11d", M->nstandard);
            int display_width = linesize - 31;
            if (display_width < 10)
                display_width = 10;
            dlDisplay(outfile, &(M->deggens), display_width);
        }
        v++;
        if (v > last_var)
            break;
        p = VREF(v);
    } while (p->exists == PARTVAR);
    return 1;
}

int setdegs_cmd(int argc, char *argv[])
{
    gmatrix M;

    if (argc != 2)
    {
        printnew("setdegs <matrix>\n");
        return 0;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    setdegs(M);
    return 1;
}

int setcoldegs_cmd(int argc, char *argv[])
{
    gmatrix M;

    if (argc != 2)
    {
        printnew("setcoldegs <matrix>\n");
        return 0;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    setdeggens(M);
    return 1;
}

int dshift_cmd(int argc, char *argv[])
{
    gmatrix M;
    int i, n;

    if (argc != 3)
    {
        printnew("dshift <matrix> <degree to shift by>\n");
        return 0;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    n = getInt(argv[2]);
    for (i = 1; i <= length(&M->degrees); i++)
        dlist_set(&M->degrees, i, DREF(M->degrees, i) + n);
    for (i = 1; i <= length(&M->deggens); i++)
        dlist_set(&M->deggens, i, DREF(M->deggens, i) + n);
    return 1;
}

int int_cmd(int argc, char *argv[])
{
    variable *p;
    int n;

    if (argc != 3)
    {
        printnew("int <name> <new value>\n");
        return 0;
    }
    n = parseInt(argv + 2);
    if (!new_int(&p, argc, argv, 1))
        return 0;
    var_set_int(p, n);
    return 1;
}

int doIntCmd(char *name, int val, boolean setval, const char *mess)
{
    variable *p;

    if (setval)
    {
        p = make_var(name, MAINVAR, VINT, NULL);
        if (p == NULL)
            return 0;
        var_set_int(p, val);
    }
    else
    {
        newline();
        print("%s : %d\n", mess, val);
    }
    return 1;
}

int nrows_cmd(int argc, char *argv[])
{
    gmatrix M;
    int n;

    if ((argc < 2) || (argc > 3))
    {
        printnew("nrows <matrix> [result integer]\n");
        return 0;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    n = nrows(M);
    doIntCmd(argv[2], n, (argc == 3), "number of rows");
    return 1;
}

int ncols_cmd(int argc, char *argv[])
{
    gmatrix M;
    int n;

    if ((argc < 2) || (argc > 3))
    {
        printnew("ncols <matrix> [result integer]\n");
        return 0;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    n = ncols(M);
    doIntCmd(argv[2], n, (argc == 3), "number of columns");
    return 1;
}

int nvars_cmd(int argc, char *argv[])
{
    ring R;
    int n;

    if ((argc < 2) || (argc > 3))
    {
        printnew("nvars <ring> [result integer]\n");
        return 0;
    }
    if (!get_ring_arg(&R, argc, argv, 1))
        return 0;
    n = numvars;
    doIntCmd(argv[2], n, (argc == 3), "number of variables in ring");
    return 1;
}

int charac_cmd(int argc, char *argv[])
{
    ring R;
    int n;

    if ((argc < 2) || (argc > 3))
    {
        printnew("characteristic <ring> [result integer]\n");
        return 0;
    }
    if (!get_ring_arg(&R, argc, argv, 1))
        return 0;
    n = charac;
    doIntCmd(argv[2], n, (argc == 3), "characteristic of ring");
    return 1;
}

int rowdeg_cmd(int argc, char *argv[])
{
    gmatrix M;
    int n, r;

    if ((argc < 3) || (argc > 4))
    {
        printnew("row_degree <matrix> <row> [result integer]\n");
        return 0;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    r = getInt(argv[2]);
    if ((r <= 0) || (r > nrows(M)))
        n = 0;
    else
        n = DREF(M->degrees, r);

    doIntCmd(argv[3], n, (argc == 4), "row degree");
    return 1;
}

int coldeg_cmd(int argc, char *argv[])
{
    gmatrix M;
    int n, c;

    if ((argc < 3) || (argc > 4))
    {
        printnew("col_degree <matrix> <column> [result integer]\n");
        return 0;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    c = getInt(argv[2]);
    if ((c <= 0) || (c > ncols(M)))
        n = 0;
    else
        n = DREF(M->deggens, c);

    doIntCmd(argv[3], n, (argc == 4), "column degree");
    return 1;
}

int max_cmd(int argc, char *argv[])
{
    gmatrix M;

    if ((argc < 2) || (argc > 3))
    {
        printnew("max <matrix> [result maximum row degree]\n");
        return 0;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    doIntCmd(argv[2], dl_max(&M->degrees), (argc == 3), "maximum row degree");
    return 1;
}

int min_cmd(int argc, char *argv[])
{
    gmatrix M;

    if ((argc < 2) || (argc > 3))
    {
        printnew("min <matrix> [result minimum row degree]\n");
        return 0;
    }
    if (!get_mod(&M, argc, argv, 1))
        return 0;
    doIntCmd(argv[2], dl_min(&M->degrees), (argc == 3), "minimum row degree");
    return 1;
}

int iszero_cmd(int argc, char *argv[])
{
    poly f;

    if (argc != 3)
    {
        printnew("is_zero <poly> <result integer: 1 if zero, else 0>\n");
        return 0;
    }
    f = getPoly(argv[1], 1);
    doIntCmd(argv[2], f == NULL, (argc == 3), "");
    return 1;
}

// cmd_rec cmd_list[] is already declared in cmds_.h

int args_cmd(int argc, char *argv[])
{
    int i, args;
    char *p;

    if (argc == 2 || argc == 3)
    {
        for (p = argv[1]; *p != '\0'; ++p)
            if (*p == '-')
                *p = '_';
        i = cmd_lookup(argv[1], 1);
        if (i >= 0)
        {
            if (argc == 2)
            {
                printnew("%d\n", cmd_list[i].args);
            }
            else // argc == 3
            {
                if (sscanf(argv[2], "%d", &args) && args >= 0)
                    cmd_list[i].args = args;
                else
                    prerror("; invalid argument to args: %s\n", argv[2]);
            }
        }
        else if (i == -1)
        {
            prerror("; command not found: %s\n", argv[1]);
        }
    }
    else
    {
        printnew("args <command name>        (to inspect)\n");
        printnew("args <command name> <int>  (to modify)\n");
    }
    return 1;
}
