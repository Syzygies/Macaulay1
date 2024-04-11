/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"

// variable *mkcollect (char *name);
// variable *mktrash (char *name);
// variable *mknres (char *name, int intval);
// variable *mkstd (char *name);
// variable *mkistd (char *name, int intval, gmatrix M);
// void mk_sev_nres (int first, int num_to_make, int intval, int last_intval);
// variable *mkres (char *name, int intval, gmatrix M);
// variable *mkemit (char *name, variable *Mvar);
// variable *mkstdemit (char *name, variable *Mvar);
// variable *mklift (char *name, variable *Mvar);
// variable *mkmerge (char *name);
// variable *mkshift (char *name, int shiftval);
// variable *mkstarter (char *name, int maxgen, int lastdeg, int numstart);
// void alias_type (variable *p);
// start_rec * st_init (int maxgen, int lastdeg, int numstart);
// void st_kill (variable *box);
// void st_bounds (variable *box, int argc, char *argv[]);
// void st_start (variable *box, int argc, char *argv[]);
// void calc (variable *box, boolean doalldegs, int hideg);
void docalc (variable *box);
// void st_pprint (start_rec *s);
// void coll_degs (variable *box, dlist *dl, int deg);
// void coll_poly (variable *box, poly f, int deg);
// void trash_poly (variable *box, poly f, int deg);
// boolean emit_dodeg (variable *box, int deg);
// boolean semit_dodeg (variable *box, int deg);
// boolean istd_dodeg (variable *box, int deg);
// boolean res_dodeg (variable *box, int deg);
// void ins_nres (variable *box, poly f, int deg);
// boolean nres_enddeg (variable *box, int deg);
// boolean std_enddeg (variable *box, int deg);
// void std_degs (variable *box, dlist *dl, int deg);
// void mg_degs (variable *box, dlist *dl, int deg);
// void mg_poly (variable *box, poly f, int deg);
// boolean mg_enddeg (variable *box, int deg);
// boolean shift_dodeg (variable *box, int deg);
// boolean lift_enddeg (variable *box, int deg);
// void lift_poly (variable *box, poly f, int deg);

/*
 *  make routines for the various variable types.
 *
 */

variable *mkcollect (char *name)
{
    variable *result;

    result = make_var(name, PARTVAR, VCOLLECT, current_ring);
    set_value(result, mod_init());
    return(result);
}

variable *mktrash (char *name)
{
    return(make_var(name, PARTVAR, VTRASH, current_ring));
}

variable *mknres (char *name, int intval)
{
    variable *result;

    result = make_var(name, PARTVAR, VNRES, current_ring);
    result->intval = intval;
    set_value(result, mod_init());
    return(result);
}

variable *mkstd (char *name)
{
    variable *result;

    result = make_var(name, PARTVAR, VSTD, current_ring);
    result->intval = 0;    /* not used */
    set_value(result, mod_init());
    return(result);
}

variable *mkistd (char *name, int intval, gmatrix M)
{
    variable *result;

    result = make_var(name, PARTVAR, VISTD, current_ring);
    result->intval = intval;
    set_value(result, mat_copy(M));
    return(result);
}

void mk_sev_nres (int first, int num_to_make, int intval, int last_intval)
{
    char s[20];
    int i;

    if (num_to_make <= 0) {
        mktrash("");
        return;
    }
    for (i=first; i<first+num_to_make-1; i++) {
        sprintf(s, "%d\0", i);
        mknres(s, intval);
    }
    sprintf(s, "%d\0", first+num_to_make-1);
    mknres(s, last_intval);
}

variable *mkres (char *name, int intval, gmatrix M)
{
    variable *result;

    result = make_var(name, PARTVAR, VRES, current_ring);
    result->intval = intval;
    set_value(result, mat_copy(M));
    return(result);
}

variable *mkemit (char *name, variable *Mvar)
{
    variable *result;

    result = make_var(name, PARTVAR, VEMIT, Mvar->b_ring);
    set_value(result, Mvar);
    return(result);
}

variable *mkstdemit (char *name, variable *Mvar)
{
    variable *result;

    result = make_var(name, PARTVAR, VSTDEMIT, Mvar->b_ring);
    set_value(result, Mvar);
    return(result);
}

variable *mklift (char *name, variable *Mvar)
{
    variable *result;

    result = make_var(name, PARTVAR, VLIFT, Mvar->b_ring);
    set_value(result, Mvar);
    return(result);
}

variable *mkmerge (char *name)
{
    variable *result;

    result = make_var(name, PARTVAR, VMERGE, NULL);
    result->intval = 0;
    set_value(result, NULL);
    return(result);
}

variable *mkshift (char *name, int shiftval)
{
    variable *result;

    result = make_var(name, PARTVAR, VSHIFT, NULL);
    result->intval = shiftval;
    set_value(result, NULL);
    return(result);
}

variable *mkstarter (char *name, int maxgen, int lastdeg, int numstart)
{
    variable *result;

    result = make_var(name, MAINVAR, VSTARTER, current_ring);
    set_value(result, st_init(maxgen, lastdeg, numstart));
    return(result);
}

/*--- alias variable routines ---------*/

void alias_type (variable *p)
{
    varb_type(p);
}

/*--- boxtype = VSTARTER  --------*/

start_rec * st_init (int maxgen, int lastdeg, int numstart)
{
    start_rec *s;

    s = (start_rec *) get_small(sizeof(start_rec));
    s->maxgen = maxgen;
    s->lastdeg = lastdeg;
    s->numstart = numstart;
    s->iscomplete = FALSE;
    s->hideg = s->lastdeg - 1;     /* so no comp. is in progress */
    s->doalldegs = FALSE;
    return(s);
}

void st_kill (variable *box)
{
#pragma unused(box)
}

void st_bounds (variable *box, int argc, char *argv[])
{
    start_rec *s;

    s = (start_rec *) box->value;
    if (argc IS 2) {
        s->doalldegs = TRUE;
        s->hideg = s->lastdeg - 1;
    } else {
        s->hideg = getInt(argv[2]);
        s->doalldegs = FALSE;
    }
}

void st_start (variable *box, int argc, char *argv[])
{
    st_bounds(box, argc, argv);
    docalc(box);
}

void calc (variable *box, boolean doalldegs, int hideg)
{
    start_rec *s;

    s = (start_rec *) box->value;
    s->doalldegs = doalldegs;
    if (doalldegs)
        s->hideg = s->lastdeg - 1;
    else
        s->hideg = hideg;
    docalc(box);
}
void docalc (variable *box)
{
    int deg;
    boolean b, done_yet;
    int B;
    variable *p;
    start_rec *s;

    s = (start_rec *) box->value;
    rmmouse();
    done_yet = s->iscomplete;
    newline();  /* for commenting out flushed verbose info */
    for (deg = s->lastdeg+1; ((deg <= s->hideg) OR (s->doalldegs))
            AND (NOT done_yet); deg++) {
        intflush("%d", deg);
        done_yet = (s->maxgen < deg);
        for (B=1; B <= s->numstart; B++) {
            p = (variable *) ref(&(var_list), box->var_num + B);
            b = do_degree(p, deg);
            done_yet = done_yet AND b;
        }
        s->lastdeg++;
    }
    s->iscomplete = done_yet;
    s->hideg = s->lastdeg - 1;
    s->doalldegs = FALSE;
    print("\n");
    if (done_yet) {
        newline();
        print("computation complete after degree %d\n", s->lastdeg);
    }
} /* mod 24feb89 DB */

void st_pprint (start_rec *s)
{
    fnewline(outfile);
    fprint(outfile, "computation is complete thru degree %d\n", s->lastdeg);
    fnewline(outfile);
    fprint(outfile, "highest degree generator has degree %d\n", s->maxgen);
    fnewline(outfile);
    fprint(outfile,"# to start = %d\n", s->numstart);
    fnewline(outfile);
    if (s->iscomplete)
        fprint(outfile,"the computation is complete\n");
    else fprint(outfile,"the computation is not yet complete\n");

    fnewline(outfile);
    if ((s->hideg >= s->lastdeg) OR (s->doalldegs)) {
        fprint(outfile, "computation is in progress: ");
        if (s->doalldegs)
            fprint(outfile, "doing all degrees\n");
        else
            fprint(outfile, "doing thru degree %d\n", s->hideg);
    }
}
/*---- Boxtype = VCOLLECT  "excess collector" -------------------*/

void coll_degs (variable *box, dlist *dl, int deg)
{
    gmatrix M;
#pragma unused(deg)

    M = VAR_MODULE(box);
    dl_kill(&(M->degrees));
    dl_copy(dl, &(M->degrees));
}

void coll_poly (variable *box, poly f, int deg)
{
    gmatrix M;

    M = VAR_MODULE(box);
    make1_monic(&f);
    pl_insert(&(M->gens), f);
    dl_insert(&(M->deggens), deg);
}

/*---- Boxtype = VTRASH  "trash collector" ---------------------*/

void trash_poly (variable *box, poly f, int deg)
{
#pragma unused(box,deg)
    p_kill(&f);
}

/*---- Boxtype = VEMIT  "emitter of generators of a module" -----------*/

boolean emit_dodeg (variable *box, int deg)
{
    gmatrix M;
    variable *aliasM;
    int i;
    poly f;

    aliasM = (variable *) box->value;
    M = VAR_MODULE(aliasM);
    vrg_install(aliasM->b_ring);
    send_degs(out_box(box), &(M->degrees), deg);
    for (i=1; i<=LENGTH(M->gens); i++)
        if (DREF(M->deggens, i) IS deg) {
            f = p_copy(PREF(M->gens, i));
            send_poly(out_box(box), f, deg);
        }
    return(end_in_degree(out_box(box), deg));
}

/*---- Boxtype = VSTDEMIT  "emitter of generators of a module" --------*/

boolean semit_dodeg (variable *box, int deg)
{
    gmatrix M;
    mn_standard p;
    variable *aliasM;
    poly f;

    aliasM = (variable *) box->value;
    M = VAR_MODULE(aliasM);
    vrg_install(aliasM->b_ring);
    send_degs(out_box(box), &(M->degrees), deg);
    p = M->stdbasis;
    while (p ISNT NULL) {
        f = p->standard;
        if (p->degree IS deg) {
            f = p_copy(f);
            send_poly(out_box(box), f, deg);
        }
        p = p->next;
    }
    return(end_in_degree(out_box(box), deg));
}

/*---- Boxtype = VISTD "calc. std basis + syz's of an inhomog matrix --*/

boolean istd_dodeg (variable *box, int deg)
{
    gmatrix M;
    int temp_len;

    vrg_install(box->b_ring);
    M = VAR_MODULE(box);

    if (((box->intval) >= 0) AND (box->intval < length(&(M->deggens)))) {
        temp_len = M->deggens.len;
        M->deggens.len = box->intval;
        send_degs(out_box(box), &(M->deggens), deg);
        M->deggens.len = temp_len;
    } else send_degs(out_box(box), &(M->deggens), deg);

    IHorig_gens(M, box->intval, out_box(box));
    IHcalc_standard(M, out_box(box));
    end_in_degree(out_box(box), deg);
    return(TRUE);
}

/*---- Boxtype = VRES "calc. std basis + syz's, keeping orig gens of
  ---- the module" --------------------------------------------------*/

boolean res_dodeg (variable *box, int deg)
{
    gmatrix M;
    boolean done_yet, rest_done;
    int temp_len;

    vrg_install(box->b_ring);
    M = VAR_MODULE(box);

    if (((box->intval) >= 0) AND (box->intval < length(&(M->deggens)))) {
        temp_len = M->deggens.len;
        M->deggens.len = box->intval;
        send_degs(out_box(box), &(M->deggens), deg);
        M->deggens.len = temp_len;
    } else send_degs(out_box(box), &(M->deggens), deg);

    done_yet = calc_standard(M, deg, out_box(box));
    orig_gens(M, deg, box->intval, out_box(box));
    rest_done = end_in_degree(out_box(box), deg);
    return(done_yet AND rest_done);
}

/*---- Boxtype = VNRES  "calc std. basis + syz's, not an emitter" -----*/

void ins_nres (variable *box, poly f, int deg)
{
    poly h, hrep;
    int i;
    gmatrix M;

    M = VAR_MODULE(box);
    h = reduce(M, &f);
    if (h IS NULL) return;
    make1_monic(&h);
    pl_insert(&(M->gens), h);
    i = LENGTH(M->gens);
    dl_insert(&(M->deggens), deg);
    if ((box->intval < 0) OR (box->intval >= i))
        hrep = e_sub_i(i);
    else hrep = NULL;
    ins_elem(M, deg, p_copy(h), hrep);
}

boolean nres_enddeg (variable *box, int deg)
{
    gmatrix M;
    boolean done_yet, rest_done;
    int temp_len;

    M = VAR_MODULE(box);

    if (((box->intval) >= 0) AND (box->intval < length(&(M->deggens)))) {
        temp_len = M->deggens.len;
        M->deggens.len = box->intval;
        send_degs(out_box(box), &(M->deggens), deg);
        M->deggens.len = temp_len;
    } else send_degs(out_box(box), &(M->deggens), deg);

    done_yet = calc_standard(M, deg+1, out_box(box));
    rest_done = end_in_degree(out_box(box), deg+1);
    return(done_yet AND rest_done);
}

/*--- box type = STD ----------------*/

boolean std_enddeg (variable *box, int deg)
{
#pragma unused(deg)
    return(mn_iscomplete(VAR_MODULE(box)));
}

void std_degs (variable *box, dlist *dl, int deg)
{
    gmatrix M;

    coll_degs(box, dl, 0);
    M = VAR_MODULE(box);
    calc_standard(M, deg, NULL);
}

/*--- boxtype = VMERGE --------------*/

void mg_degs (variable *box, dlist *dl, int deg)
{
    send_degs(out_box(box), dl, deg);
}

void mg_poly (variable *box, poly f, int deg)
{
    send_poly(out_box(box), f, deg);
}

boolean mg_enddeg (variable *box, int deg)
{
    if (box->intval IS 0) {
        box->intval = 1;
        return(TRUE); /* other fork will return FALSE if nec. */
    } else {
        box->intval = 0;
        return(end_in_degree(out_box(box), deg));
    }
}

/*--- boxtype = VSHIFT -----------------*/

boolean shift_dodeg (variable *box, int deg)
{
    return(do_degree(out_box(box), deg + box->intval));
}

/*--- boxtype = VLIFT ------------------*/

boolean lift_enddeg (variable *box, int deg)
{
    return(end_in_degree(out_box(box), deg));
}

void lift_poly (variable *box, poly f, int deg)
{
    gmatrix M;
    poly g, h;

    M = VAR_MODULE(((variable *) box->value));
    division(M, &f, &g, &h);
    if (h ISNT NULL)
        prerror("; lifted polynomial isn't in desired module\n");
    send_poly(out_box(box), g, deg);
}
