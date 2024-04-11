/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"

// void edit_cmd (int argc, char *argv[]);
// boolean ok_edit (int row_or_col);
// boolean rcheck (int n);
// boolean ccheck (int n);
// void pr_cmd (int argc, char *argv[]);
// void pc_cmd (int argc, char *argv[]);
// void mr_cmd (int argc, char *argv[]);
// void mc_cmd (int argc, char *argv[]);
// void dr_cmd (int argc, char *argv[]);
// void dc_cmd (int argc, char *argv[]);
// void ar_cmd (int argc, char *argv[]);
// void ac_cmd (int argc, char *argv[]);
// void ce_cmd (int argc, char *argv[]);
void gmR_permute (gmatrix g, int r1, int r2);
void gmC_permute (gmatrix g, int c1, int c2);
void dl_permute (dlist *dl, int i1, int i2);
void p_perm (poly *f, int i, int j);
void gmR_addto (gmatrix g, int r1, poly f, int r2);
void gmC_addto (gmatrix g, int c1, poly f, int c2);
void p_addto (poly *pol, int i1, poly f, int i2);
void gmC_mult (gmatrix g, int i, poly f);
void gmR_mult (gmatrix g, int i, poly f);
void p_multcomp (poly *pol, int i, poly f);

#define ROW 1
#define COL 2

variable *editvar;
gmatrix editmat;
boolean ispfaff;

void edit_cmd (int argc, char *argv[])
{
    if (argc IS 1)
        editvar = NULL;
    else {
        GET_VMOD(editvar, 1);
        ispfaff = ((argc IS 3) ? TRUE : FALSE);
    }
}

boolean ok_edit (int row_or_col)
{
    if (editvar IS NULL) {
        prerror("; no matrix to edit\n");
        return(FALSE);
    }
    if ((ispfaff) AND (row_or_col IS COL)) {
        prerror("; use row commands for pfaffians\n");
        return(FALSE);
    }
    vrg_install(editvar->b_ring);
    editmat = VAR_MODULE(editvar);
    return(TRUE);
}

boolean rcheck (int n)
{
    if ((n <= 0) OR (n > nrows(editmat))) {
        prerror("; row %d out of range\n", n);
        return(FALSE);
    }
    return(TRUE);
}

boolean ccheck (int n)
{
    if ((n <= 0) OR (n > ncols(editmat))) {
        prerror("; column %d out of range\n", n);
        return(FALSE);
    }
    return(TRUE);
}

void pr_cmd (int argc, char *argv[])
{
    int i1, i2;

    if (argc ISNT 3) {
        printnew("pr <row 1> <row 2>		(permute rows)\n");
        return;
    }
    if (NOT ok_edit(ROW)) return;
    i1 = getInt(argv[1]);
    i2 = getInt(argv[2]);
    if (NOT rcheck(i1)) return;
    if (NOT rcheck(i2)) return;
    gmR_permute(editmat, i1, i2);
    if (ispfaff)
        gmC_permute(editmat, i1, i2);
}

void pc_cmd (int argc, char *argv[])
{
    int i1, i2;

    if (argc ISNT 3) {
        printnew("pc <column 1> <column 2>		(permute columns)\n");
        return;
    }
    if (NOT ok_edit(COL)) return;
    i1 = getInt(argv[1]);
    i2 = getInt(argv[2]);
    if (NOT ccheck(i1)) return;
    if (NOT ccheck(i2)) return;
    gmC_permute(editmat, i1, i2);
}

void mr_cmd (int argc, char *argv[])
{
    int i;
    poly f;

    if (argc ISNT 3) {
        printnew("mr <row> <poly multiplier>		(multiply row)\n");
        return;
    }
    if (NOT ok_edit(ROW)) return;
    i = getInt(argv[1]);
    if (NOT rcheck(i)) return;
    f = getPoly(argv[2], 1);
    gmR_mult(editmat, i, f);
    if (ispfaff)
        gmC_mult(editmat, i, f);
    p_kill(&f);
}

void mc_cmd (int argc, char *argv[])
{
    int i;
    poly f;

    if (argc ISNT 3) {
        printnew("mc <col> <poly multiplier>		(multiply col.)\n");
        return;
    }
    if (NOT ok_edit(COL)) return;
    i = getInt(argv[1]);
    if (NOT ccheck(i)) return;
    f = getPoly(argv[2], 1);
    gmC_mult(editmat, i, f);
    p_kill(&f);
}

void dr_cmd (int argc, char *argv[])
{
    int i;
    field a;
    poly f;

    if (argc ISNT 3) {
        printnew("dr <row> <field elem>		(divide row)\n");
        return;
    }
    if (NOT ok_edit(ROW)) return;
    i = getInt(argv[1]);
    if (NOT rcheck(i)) return;
    a = getInt(argv[2]);
    if (a IS 0) {
        prerror("; can't divide by zero\n");
        return;
    }
    fd_recip(&a);
    f = p_monom(a);
    tm_copy(zerodegs, INITIAL(f));
    gmR_mult(editmat, i, f);
    if (ispfaff)
        gmC_mult(editmat, i, f);
    p_kill(&f);
}

void dc_cmd (int argc, char *argv[])
{
    int i;
    field a;
    poly f;

    if (argc ISNT 3) {
        printnew("dc <col> <field elem>		(divide col.)\n");
        return;
    }
    if (NOT ok_edit(COL)) return;
    i = getInt(argv[1]);
    if (NOT ccheck(i)) return;
    a = getInt(argv[2]);
    if (a IS 0) {
        prerror("; can't divide by zero\n");
        return;
    }
    fd_recip(&a);
    f = p_monom(a);
    tm_copy(zerodegs, INITIAL(f));
    gmC_mult(editmat, i, f);
    p_kill(&f);
}

void ar_cmd (int argc, char *argv[])
{
    int i1, i2;
    poly f;

    if (argc ISNT 4) {
        printnew("ar <row> <poly. multiplier> <row to modify> ");
        print("		(add to row)\n");
        return;
    }
    if (NOT ok_edit(ROW)) return;
    i1 = getInt(argv[1]);
    i2 = getInt(argv[3]);
    if (NOT rcheck(i1)) return;
    if (NOT rcheck(i2)) return;
    f = getPoly(argv[2], 1);
    gmR_addto(editmat, i1, f, i2);
    if (ispfaff)
        gmC_addto(editmat, i1, f, i2);
    p_kill(&f);
}

void ac_cmd (int argc, char *argv[])
{
    int i1, i2;
    poly f;

    if (argc ISNT 4) {
        printnew("ac <col> <poly. multiplier> <col to modify> ");
        print("		(add to col)\n");
        return;
    }
    if (NOT ok_edit(COL)) return;
    i1 = getInt(argv[1]);
    i2 = getInt(argv[3]);
    if (NOT ccheck(i1)) return;
    if (NOT ccheck(i2)) return;
    f = getPoly(argv[2], 1);
    gmC_addto(editmat, i1, f, i2);
}

void ce_cmd (int argc, char *argv[])
{
    poly f, g;
    int i, j;

    if (argc ISNT 4) {
        printnew("ce <row> <col> <new poly. entry>		change entry\n");
        return;
    }
    if (NOT ok_edit(ROW)) return;
    i = getInt(argv[1]);
    j = getInt(argv[2]);
    if (NOT rcheck(i)) return;
    if (NOT ccheck(j)) return;
    g = getPoly(argv[3], i);
    f = PREF(editmat->gens, j);
    p_chEntry(&f, i, g);
    PREF(editmat->gens, j) = f;
}

/*----------------------------------------------------------*/

void gmR_permute (gmatrix g, int r1, int r2)
{
    int i;

    for (i=1; i<=ncols(g); i++)
        p_perm(&(PREF(g->gens, i)), r1, r2);
    dl_permute(&(g->degrees), r1, r2);
}

void gmC_permute (gmatrix g, int c1, int c2)
{
    poly temp;

    dl_permute(&(g->deggens), c1, c2);
    temp = PREF(g->gens, c1);
    PREF(g->gens, c1) = PREF(g->gens, c2);
    PREF(g->gens, c2) = temp;
}

void dl_permute (dlist *dl, int i1, int i2)
{
    int temp;

    temp = DREF(*dl, i1);
    DREF(*dl, i1) = DREF(*dl, i2);
    DREF(*dl, i2) = temp;
}

void p_perm (poly *f, int i, int j)
{
    poly result, temp;
    poly ff;
    int c;

    result = NULL;
    ff = *f;
    while (ff ISNT NULL) {
        temp = ff;
        ff = ff->next;
        temp->next = NULL;
        c = get_comp(temp);
        if (c IS i)
            set_comp(temp, j);
        else if (c IS j)
            set_comp(temp, i);
        p_add(&result, &temp);
    }
    *f = result;
}

/*------------------------------------------------------------*/

void gmR_addto (gmatrix g, int r1, poly f, int r2)
/* row r2 of g is set to row r2 + f * row r1 */
/* not modified */
{
    int i;

    for (i=1; i<=ncols(g); i++)
        p_addto(&(PREF(g->gens, i)), r1, f, r2);
}

void gmC_addto (gmatrix g, int c1, poly f, int c2)
{
    poly a;

    a = p_mult(f, PREF(g->gens, c1));
    p_add(&PREF(g->gens, c2), &a);
}

void p_addto (poly *pol, int i1, poly f, int i2)
{
    poly result, temp, h;
    poly p;

    result = NULL;
    p = *pol;
    while (p ISNT NULL) {
        temp = p;
        p = p->next;
        temp->next = NULL;
        if (get_comp(temp) IS i1) {
            set_comp(temp, i2);
            h = p_mult(f, temp);
            p_add(&result, &h);
            set_comp(temp, i1);
        }
        p_add(&result, &temp);
    }
    *pol = result;
}

/*------------------------------------------------------*/

void gmC_mult (gmatrix g, int i, poly f)
{
    poly k, h;

    k = PREF(g->gens, i);
    h = p_mult(f, k);
    p_kill(&k);
    PREF(g->gens, i) = h;
    DREF(g->deggens, i) = degree(g, h);
}

void gmR_mult (gmatrix g, int i, poly f)
{
    int deg, j;

    deg = degree(g, f);
    DREF(g->degrees, i) -= deg;
    for (j=1; j<=ncols(g); j++)
        p_multcomp(&PREF(g->gens, j), i, f);
}

void p_multcomp (poly *pol, int i, poly f)
{
    poly result, temp, h;
    poly p;

    result = NULL;
    p = *pol;
    while (p ISNT NULL) {
        temp = p;
        p = p->next;
        temp->next = NULL;
        if (get_comp(temp) IS i) {
            h = p_mult(f, temp);
            p_add(&result, &h);
        } else p_add(&result, &temp);
    }
    *pol = result;
}
