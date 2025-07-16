// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "ed.h"
#include "vars.h"
#include "monitor.h"
#include "input.h"
#include "poly.h"
#include "gmatrix.h"
#include "smallterm.h"
#include "charp.h"
#include "cmds.h"
#include "parsepoly.h"
#include "parse.h"
#include "term.h"
#include "gm_poly.h"
#include "ring.h"
#include "plist.h"

// Constants
constexpr int ROW = 1;
constexpr int COL = 2;

// Inline functions to replace macros
static inline gmatrix VAR_MODULE(variable *p)
{
    return var_get_module(p);
}

static inline poly PREF(plist pl, int i)
{
    return plist_ref(&pl, i);
}

static inline int DREF(dlist dl, int i)
{
    return dlist_ref(&dl, i);
}

static inline term INITIAL(poly f)
{
    return poly_initial(f);
}

// GET_VMOD inline function
static inline bool get_vmod(variable **p, int argc, char **argv, int i)
{
    if (i >= argc)
        return false;
    *p = vget_vmod(argv[i]);
    return (*p != NULL);
}

static inline bool GET_VMOD(variable **p, int argc, char **argv, int i)
{
    return get_vmod(p, argc, argv, i);
}

variable *editvar;
gmatrix editmat;
boolean ispfaff;

int edit_cmd(int argc, char *argv[])
{
    if (argc == 1)
        editvar = NULL;
    else
    {
        if (!GET_VMOD(&editvar, argc, argv, 1))
            return 0;
        ispfaff = ((argc == 3) ? true : false);
    }
    return 1;
}

boolean ok_edit(int row_or_col)
{
    if (editvar == NULL)
    {
        prerror("; no matrix to edit\n");
        return (false);
    }
    if ((ispfaff) && (row_or_col == COL))
    {
        prerror("; use row commands for pfaffians\n");
        return (false);
    }
    vrg_install(editvar->b_ring);
    editmat = VAR_MODULE(editvar);
    return (true);
}

boolean rcheck(int n)
{
    if ((n <= 0) || (n > nrows(editmat)))
    {
        prerror("; row %d out of range\n", n);
        return (false);
    }
    return (true);
}

boolean ccheck(int n)
{
    if ((n <= 0) || (n > ncols(editmat)))
    {
        prerror("; column %d out of range\n", n);
        return (false);
    }
    return (true);
}

int pr_cmd(int argc, char *argv[])
{
    int i1, i2;

    if (argc != 3)
    {
        printnew("pr <row 1> <row 2>		(permute rows)\n");
        return 0;
    }
    if (!ok_edit(ROW))
        return 0;
    i1 = getInt(argv[1]);
    i2 = getInt(argv[2]);
    if (!rcheck(i1))
        return 0;
    if (!rcheck(i2))
        return 0;
    gmR_permute(editmat, i1, i2);
    if (ispfaff)
        gmC_permute(editmat, i1, i2);
    return 1;
}

int pc_cmd(int argc, char *argv[])
{
    int i1, i2;

    if (argc != 3)
    {
        printnew("pc <column 1> <column 2>		(permute columns)\n");
        return 0;
    }
    if (!ok_edit(COL))
        return 0;
    i1 = getInt(argv[1]);
    i2 = getInt(argv[2]);
    if (!ccheck(i1))
        return 0;
    if (!ccheck(i2))
        return 0;
    gmC_permute(editmat, i1, i2);
    return 1;
}

int mr_cmd(int argc, char *argv[])
{
    int i;
    poly f;

    if (argc != 3)
    {
        printnew("mr <row> <poly multiplier>		(multiply row)\n");
        return 0;
    }
    if (!ok_edit(ROW))
        return 0;
    i = getInt(argv[1]);
    if (!rcheck(i))
        return 0;
    f = getPoly(argv[2], 1);
    gmR_mult(editmat, i, f);
    if (ispfaff)
        gmC_mult(editmat, i, f);
    p_kill(&f);
    return 1;
}

int mc_cmd(int argc, char *argv[])
{
    int i;
    poly f;

    if (argc != 3)
    {
        printnew("mc <col> <poly multiplier>		(multiply col.)\n");
        return 0;
    }
    if (!ok_edit(COL))
        return 0;
    i = getInt(argv[1]);
    if (!ccheck(i))
        return 0;
    f = getPoly(argv[2], 1);
    gmC_mult(editmat, i, f);
    p_kill(&f);
    return 1;
}

int dr_cmd(int argc, char *argv[])
{
    int i;
    field a;
    poly f;

    if (argc != 3)
    {
        printnew("dr <row> <field elem>		(divide row)\n");
        return 0;
    }
    if (!ok_edit(ROW))
        return 0;
    i = getInt(argv[1]);
    if (!rcheck(i))
        return 0;
    a = (field)getInt(argv[2]);
    if (a == 0)
    {
        prerror("; can't divide by zero\n");
        return 0;
    }
    fd_recip(&a);
    f = p_monom(a);
    tm_copy(zerodegs, INITIAL(f));
    gmR_mult(editmat, i, f);
    if (ispfaff)
        gmC_mult(editmat, i, f);
    p_kill(&f);
    return 1;
}

int dc_cmd(int argc, char *argv[])
{
    int i;
    field a;
    poly f;

    if (argc != 3)
    {
        printnew("dc <col> <field elem>		(divide col.)\n");
        return 0;
    }
    if (!ok_edit(COL))
        return 0;
    i = getInt(argv[1]);
    if (!ccheck(i))
        return 0;
    a = (field)getInt(argv[2]);
    if (a == 0)
    {
        prerror("; can't divide by zero\n");
        return 0;
    }
    fd_recip(&a);
    f = p_monom(a);
    tm_copy(zerodegs, INITIAL(f));
    gmC_mult(editmat, i, f);
    p_kill(&f);
    return 1;
}

int ar_cmd(int argc, char *argv[])
{
    int i1, i2;
    poly f;

    if (argc != 4)
    {
        printnew("ar <row> <poly. multiplier> <row to modify> ");
        print("		(add to row)\n");
        return 0;
    }
    if (!ok_edit(ROW))
        return 0;
    i1 = getInt(argv[1]);
    i2 = getInt(argv[3]);
    if (!rcheck(i1))
        return 0;
    if (!rcheck(i2))
        return 0;
    f = getPoly(argv[2], 1);
    gmR_addto(editmat, i1, f, i2);
    if (ispfaff)
        gmC_addto(editmat, i1, f, i2);
    p_kill(&f);
    return 1;
}

int ac_cmd(int argc, char *argv[])
{
    int i1, i2;
    poly f;

    if (argc != 4)
    {
        printnew("ac <col> <poly. multiplier> <col to modify> ");
        print("		(add to col)\n");
        return 0;
    }
    if (!ok_edit(COL))
        return 0;
    i1 = getInt(argv[1]);
    i2 = getInt(argv[3]);
    if (!ccheck(i1))
        return 0;
    if (!ccheck(i2))
        return 0;
    f = getPoly(argv[2], 1);
    gmC_addto(editmat, i1, f, i2);
    return 1;
}

int ce_cmd(int argc, char *argv[])
{
    poly f, g;
    int i, j;

    if (argc != 4)
    {
        printnew("ce <row> <col> <new poly. entry>		change entry\n");
        return 0;
    }
    if (!ok_edit(ROW))
        return 0;
    i = getInt(argv[1]);
    j = getInt(argv[2]);
    if (!rcheck(i))
        return 0;
    if (!ccheck(j))
        return 0;
    g = getPoly(argv[3], i);
    f = PREF(editmat->gens, j);
    p_chEntry(&f, i, g);
    plist_set(&editmat->gens, j, f);
    return 1;
}

void gmR_permute(gmatrix g, int r1, int r2)
{
    int i;

    for (i = 1; i <= ncols(g); i++)
    {
        poly p = PREF(g->gens, i);
        p_perm(&p, r1, r2);
        plist_set(&g->gens, i, p);
    }
    dl_permute(&(g->degrees), r1, r2);
}

void gmC_permute(gmatrix g, int c1, int c2)
{
    poly temp;

    dl_permute(&(g->deggens), c1, c2);
    temp = PREF(g->gens, c1);
    plist_set(&g->gens, c1, PREF(g->gens, c2));
    plist_set(&g->gens, c2, temp);
}

void dl_permute(dlist *dl, int i1, int i2)
{
    int temp;

    temp = DREF(*dl, i1);
    dlist_set(dl, i1, DREF(*dl, i2));
    dlist_set(dl, i2, temp);
}

void p_perm(poly *f, int i, int j)
{
    poly result, temp;
    register poly ff;
    register int c;

    result = NULL;
    ff = *f;
    while (ff != NULL)
    {
        temp = ff;
        ff = ff->next;
        temp->next = NULL;
        c = get_comp(temp);
        if (c == i)
            set_comp(temp, j);
        else if (c == j)
            set_comp(temp, i);
        p_add(&result, &temp);
    }
    *f = result;
}

void gmR_addto(gmatrix g, int r1, poly f, int r2)
// row r2 of g is set to row r2 + f * row r1
{
    int i;

    for (i = 1; i <= ncols(g); i++)
    {
        poly p = PREF(g->gens, i);
        p_addto(&p, r1, f, r2);
        plist_set(&g->gens, i, p);
    }
}

void gmC_addto(gmatrix g, int c1, poly f, int c2)
{
    poly a;

    a = p_mult(f, PREF(g->gens, c1));
    poly p = PREF(g->gens, c2);
    p_add(&p, &a);
    plist_set(&g->gens, c2, p);
}

void p_addto(poly *pol, int i1, poly f, int i2)
{
    poly result, temp, h;
    register poly p;

    result = NULL;
    p = *pol;
    while (p != NULL)
    {
        temp = p;
        p = p->next;
        temp->next = NULL;
        if (get_comp(temp) == i1)
        {
            set_comp(temp, i2);
            h = p_mult(f, temp);
            p_add(&result, &h);
            set_comp(temp, i1);
        }
        p_add(&result, &temp);
    }
    *pol = result;
}

void gmC_mult(gmatrix g, int i, poly f)
{
    poly k, h;

    k = PREF(g->gens, i);
    h = p_mult(f, k);
    p_kill(&k);
    plist_set(&g->gens, i, h);
    dlist_set(&g->deggens, i, degree(g, h));
}

void gmR_mult(gmatrix g, int i, poly f)
{
    int deg, j;

    deg = degree(g, f);
    dlist_set(&g->degrees, i, DREF(g->degrees, i) - deg);
    for (j = 1; j <= ncols(g); j++)
    {
        poly p = PREF(g->gens, j);
        p_multcomp(&p, i, f);
        plist_set(&g->gens, j, p);
    }
}

void p_multcomp(poly *pol, int i, poly f)
{
    poly result, temp, h;
    register poly p;

    result = NULL;
    p = *pol;
    while (p != NULL)
    {
        temp = p;
        p = p->next;
        temp->next = NULL;
        if (get_comp(temp) == i)
        {
            h = p_mult(f, temp);
            p_add(&result, &h);
        }
        else
            p_add(&result, &temp);
    }
    *pol = result;
}
