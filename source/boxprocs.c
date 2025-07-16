// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "shared.h"
#include "boxprocs.h"
#include "vars.h"
#include "plist.h"
#include "generic.h"
#include "shell.h"
#include "parse.h"
#include "mem.h"
#include "monitor.h"
#include "poly.h"
#include "standard.h"
#include "hilb.h"
#include "inhomog.h"

// Message passing constants now in generic.h

// Variable type constants come from generic.h
// Variable scope constants MAINVAR and PARTVAR come from shared.h

// Inline function replacements for macros
static inline gmatrix VAR_MODULE(variable *p)
{
    return (gmatrix)p->value;
}

static inline variable *out_box(variable *box)
{
    return box->b_next;
}

// Message passing functions - message_proc comes from generic.h

static inline boolean do_degree(variable *box, int deg)
{
    const object_vtable *vtable = get_vtable(box->type);
    if (vtable && vtable->msg2.do_degree)
    {
        return vtable->msg2.do_degree(box, deg);
    }
    return TRUE; // Default if no handler
}

static inline boolean end_in_degree(variable *box, int deg)
{
    const object_vtable *vtable = get_vtable(box->type);
    if (vtable && vtable->end_degree)
    {
        return vtable->end_degree(box, deg);
    }
    return TRUE; // Default if no handler
}

void send_poly(variable *box, poly f, int deg)
{
    const object_vtable *vtable = get_vtable(box->type);
    if (vtable && vtable->poly_receive)
    {
        vtable->poly_receive(box, f, deg);
    }
}

static inline void send_degs(variable *box, dlist *dl, int deg)
{
    const object_vtable *vtable = get_vtable(box->type);
    if (vtable && vtable->msg2.degs_receive)
    {
        vtable->msg2.degs_receive(box, dl, deg);
    }
}

// Array access function - var_list comes from vars.h/shared.h
// External file handle - outfile comes from shell.h

// make routines for the various variable types.

variable *mkcollect(const char *name)
{
    variable *result;

    result = make_var(name, PARTVAR, VCOLLECT, (variable *)current_ring);
    set_value(result, mod_init());
    return result;
}

variable *mktrash(const char *name)
{
    return make_var(name, PARTVAR, VTRASH, (variable *)current_ring);
}

variable *mknres(const char *name, int intval)
{
    variable *result;

    result = make_var(name, PARTVAR, VNRES, (variable *)current_ring);
    result->intval = intval;
    set_value(result, mod_init());
    return result;
}

variable *mkstd(const char *name)
{
    variable *result;

    result = make_var(name, PARTVAR, VSTD, (variable *)current_ring);
    result->intval = 0; // not used
    set_value(result, mod_init());
    return result;
}

variable *mkistd(const char *name, int intval, gmatrix M)
{
    variable *result;

    result = make_var(name, PARTVAR, VISTD, (variable *)current_ring);
    result->intval = intval;
    set_value(result, mat_copy(M));
    return result;
}

void mk_sev_nres(int first, int num_to_make, int intval, int last_intval)
{
    char s[20];
    register int i;

    if (num_to_make <= 0)
    {
        char empty[] = "";
        mktrash(empty);
        return;
    }
    for (i = first; i < first + num_to_make - 1; i++)
    {
        sprintf(s, "%d", i);
        mknres(s, intval);
    }
    sprintf(s, "%d", first + num_to_make - 1);
    mknres(s, last_intval);
}

variable *mkres(const char *name, int intval, gmatrix M)
{
    variable *result;

    result = make_var(name, PARTVAR, VRES, (variable *)current_ring);
    result->intval = intval;
    set_value(result, mat_copy(M));
    return result;
}

variable *mkemit(const char *name, variable *Mvar)
{
    variable *result;

    result = make_var(name, PARTVAR, VEMIT, Mvar->b_ring);
    set_value(result, Mvar);
    return result;
}

variable *mkstdemit(const char *name, variable *Mvar)
{
    variable *result;

    result = make_var(name, PARTVAR, VSTDEMIT, Mvar->b_ring);
    set_value(result, Mvar);
    return result;
}

variable *mklift(const char *name, variable *Mvar)
{
    variable *result;

    result = make_var(name, PARTVAR, VLIFT, Mvar->b_ring);
    set_value(result, Mvar);
    return result;
}

variable *mkmerge(const char *name)
{
    variable *result;

    result = make_var(name, PARTVAR, VMERGE, NULL);
    result->intval = 0;
    set_value(result, NULL);
    return result;
}

variable *mkshift(const char *name, int shiftval)
{
    variable *result;

    result = make_var(name, PARTVAR, VSHIFT, NULL);
    result->intval = shiftval;
    set_value(result, NULL);
    return result;
}

variable *mkstarter(const char *name, int maxgen, int lastdeg, int numstart)
{
    variable *result;

    result = make_var(name, MAINVAR, VSTARTER, (variable *)current_ring);
    set_value(result, st_init(maxgen, lastdeg, numstart));
    return result;
}

// alias variable routines

void alias_type(variable *p)
{
    // varb_type(p) would return type info but it's not used here
}

// boxtype = VSTARTER

start_rec *st_init(int maxgen, int lastdeg, int numstart)
{
    start_rec *s;

    s = (start_rec *)(void *)get_small(sizeof(start_rec));
    s->maxgen = maxgen;
    s->lastdeg = lastdeg;
    s->numstart = numstart;
    s->iscomplete = false;
    s->hideg = s->lastdeg - 1; // so no comp. is in progress
    s->doalldegs = false;
    return s;
}

void st_kill(variable *box)
{
#ifdef ANSI
#pragma unused(box)
#endif
}

void st_bounds(variable *box, int argc, char *argv[])
{
    start_rec *s;

    s = (start_rec *)box->value;
    if (argc == 2)
    {
        s->doalldegs = true;
        s->hideg = s->lastdeg - 1;
    }
    else
    {
        s->hideg = getInt(argv[2]);
        s->doalldegs = false;
    }
}

void st_start(variable *box, int argc, char *argv[])
{
    st_bounds(box, argc, argv);
    docalc(box);
}

void calc(variable *box, boolean doalldegs, int hideg)
{
    start_rec *s;

    s = (start_rec *)box->value;
    s->doalldegs = doalldegs;
    if (doalldegs)
        s->hideg = s->lastdeg - 1;
    else
        s->hideg = hideg;
    docalc(box);
}

void docalc(variable *box)
{
    int deg;
    boolean b, done_yet;
    int B;
    variable *p;
    start_rec *s;

    s = (start_rec *)box->value;
    // rmmouse() removed - Mac-specific
    done_yet = s->iscomplete;
    newline(); // for commenting out flushed verbose info
    for (deg = s->lastdeg + 1; ((deg <= s->hideg) || s->doalldegs) && (!done_yet); deg++)
    {
        intflush("%d", deg);
        done_yet = (s->maxgen < deg);
        for (B = 1; B <= s->numstart; B++)
        {
            p = (variable *)ref(&var_list, box->var_num + B);
            b = do_degree(p, deg);
            done_yet = done_yet && b;
        }
        s->lastdeg++;
    }
    s->iscomplete = done_yet;
    s->hideg = s->lastdeg - 1;
    s->doalldegs = false;
    print("\n");
    if (done_yet)
    {
        newline();
        print("computation complete after degree %d\n", s->lastdeg);
    }
} // mod 24feb89 DB

void st_pprint(start_rec *s)
{
    fnewline(outfile);
    fprint(outfile, "computation is complete thru degree %d\n", s->lastdeg);
    fnewline(outfile);
    fprint(outfile, "highest degree generator has degree %d\n", s->maxgen);
    fnewline(outfile);
    fprint(outfile, "# to start = %d\n", s->numstart);
    fnewline(outfile);
    if (s->iscomplete)
        fprint(outfile, "the computation is complete\n");
    else
        fprint(outfile, "the computation is not yet complete\n");

    fnewline(outfile);
    if ((s->hideg >= s->lastdeg) || s->doalldegs)
    {
        fprint(outfile, "computation is in progress: ");
        if (s->doalldegs)
            fprint(outfile, "doing all degrees\n");
        else
            fprint(outfile, "doing thru degree %d\n", s->hideg);
    }
}

// Boxtype = VCOLLECT  "excess collector"

void coll_degs(variable *box, dlist *dl, int deg)
{
    gmatrix M;
#ifdef ANSI
#pragma unused(deg)
#endif

    M = VAR_MODULE(box);
    dl_kill(&(M->degrees));
    dl_copy(dl, &(M->degrees));
}

void coll_poly(variable *box, poly f, int deg)
{
    gmatrix M;

    M = VAR_MODULE(box);
    make1_monic(&f);
    plist_insert(&(M->gens), f);
    dlist_insert(&(M->deggens), deg);
}

// Boxtype = VTRASH  "trash collector"

void trash_poly(variable *box, poly f, int deg)
{
#ifdef ANSI
#pragma unused(box, deg)
#endif
    p_kill(&f);
}

// Boxtype = VEMIT  "emitter of generators of a module"

boolean emit_dodeg(variable *box, int deg)
{
    gmatrix M;
    variable *aliasM;
    int i;
    poly f;

    aliasM = (variable *)box->value;
    M = VAR_MODULE(aliasM);
    vrg_install(aliasM->b_ring);
    send_degs(out_box(box), &(M->degrees), deg);
    for (i = 1; i <= length(&(M->gens)); i++)
        if (dlist_ref(&(M->deggens), i) == deg)
        {
            f = p_copy(plist_ref(&(M->gens), i));
            send_poly(out_box(box), f, deg);
        }
    return end_in_degree(out_box(box), deg);
}

// Boxtype = VSTDEMIT  "emitter of generators of a module"

boolean semit_dodeg(variable *box, int deg)
{
    gmatrix M;
    mn_standard p;
    variable *aliasM;
    poly f;

    aliasM = (variable *)box->value;
    M = VAR_MODULE(aliasM);
    vrg_install(aliasM->b_ring);
    send_degs(out_box(box), &(M->degrees), deg);
    p = M->stdbasis;
    while (p != NULL)
    {
        f = p->standard;
        if (p->degree == deg)
        {
            f = p_copy(f);
            send_poly(out_box(box), f, deg);
        }
        p = p->next;
    }
    return end_in_degree(out_box(box), deg);
}

// Boxtype = VISTD "calc. std basis + syz's of an inhomog matrix

boolean istd_dodeg(variable *box, int deg)
{
    gmatrix M;
    int temp_len;

    vrg_install(box->b_ring);
    M = VAR_MODULE(box);

    if ((box->intval >= 0) && (box->intval < length(&(M->deggens))))
    {
        temp_len = M->deggens.len;
        M->deggens.len = box->intval;
        send_degs(out_box(box), &(M->deggens), deg);
        M->deggens.len = temp_len;
    }
    else
    {
        send_degs(out_box(box), &(M->deggens), deg);
    }

    IHorig_gens(M, box->intval, out_box(box));
    IHcalc_standard(M, out_box(box));
    end_in_degree(out_box(box), deg);
    return true;
}

// Boxtype = VRES "calc. std basis + syz's, keeping orig gens of
// ---- the module" --------------------------------------------------

boolean res_dodeg(variable *box, int deg)
{
    gmatrix M;
    boolean done_yet, rest_done;
    int temp_len;

    vrg_install(box->b_ring);
    M = VAR_MODULE(box);

    if ((box->intval >= 0) && (box->intval < length(&(M->deggens))))
    {
        temp_len = M->deggens.len;
        M->deggens.len = box->intval;
        send_degs(out_box(box), &(M->deggens), deg);
        M->deggens.len = temp_len;
    }
    else
    {
        send_degs(out_box(box), &(M->deggens), deg);
    }

    done_yet = calc_standard(M, deg, out_box(box));
    orig_gens(M, deg, box->intval, out_box(box));
    rest_done = end_in_degree(out_box(box), deg);
    return done_yet && rest_done;
}

// Boxtype = VNRES  "calc std. basis + syz's, not an emitter"

void ins_nres(variable *box, poly f, int deg)
{
    poly h, hrep;
    int i;
    gmatrix M;

    M = VAR_MODULE(box);
    h = reduce(M, &f);
    if (h == NULL)
    {
        return;
    }
    make1_monic(&h);
    plist_insert(&(M->gens), h);
    i = length(&(M->gens));
    dlist_insert(&(M->deggens), deg);
    if ((box->intval < 0) || (box->intval >= i))
        hrep = e_sub_i(i);
    else
        hrep = NULL;
    ins_elem(M, deg, p_copy(h), hrep);
}

boolean nres_enddeg(variable *box, int deg)
{
    gmatrix M;
    boolean done_yet, rest_done;
    int temp_len;

    M = VAR_MODULE(box);

    if ((box->intval >= 0) && (box->intval < length(&(M->deggens))))
    {
        temp_len = M->deggens.len;
        M->deggens.len = box->intval;
        send_degs(out_box(box), &(M->deggens), deg);
        M->deggens.len = temp_len;
    }
    else
    {
        send_degs(out_box(box), &(M->deggens), deg);
    }

    done_yet = calc_standard(M, deg + 1, out_box(box));
    rest_done = end_in_degree(out_box(box), deg + 1);
    return done_yet && rest_done;
}

// box type = STD

boolean std_enddeg(variable *box, int deg)
{
#ifdef ANSI
#pragma unused(deg)
#endif
    return mn_iscomplete(VAR_MODULE(box));
}

void std_degs(variable *box, dlist *dl, int deg)
{
    gmatrix M;

    coll_degs(box, dl, deg);
    M = VAR_MODULE(box);
    calc_standard(M, deg, NULL);
}

// boxtype = VMERGE

void mg_degs(variable *box, dlist *dl, int deg)
{
    send_degs(out_box(box), dl, deg);
}

void mg_poly(variable *box, poly f, int deg)
{
    send_poly(out_box(box), f, deg);
}

boolean mg_enddeg(variable *box, int deg)
{
    if (box->intval == 0)
    {
        box->intval = 1;
        return true; // other fork will return false if nec.
    }
    else
    {
        box->intval = 0;
        return end_in_degree(out_box(box), deg);
    }
}

// boxtype = VSHIFT

boolean shift_dodeg(variable *box, int deg)
{
    return do_degree(out_box(box), deg + box->intval);
}

// boxtype = VLIFT

boolean lift_enddeg(variable *box, int deg)
{
    return end_in_degree(out_box(box), deg);
}

void lift_poly(variable *box, poly f, int deg)
{
    gmatrix M;
    poly g, h;

    M = VAR_MODULE((variable *)box->value);
    division(M, &f, &g, &h);
    if (h != NULL)
    {
        prerror("; lifted polynomial isn't in desired module\n");
    }
    send_poly(out_box(box), g, deg);
}
