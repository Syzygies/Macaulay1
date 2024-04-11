/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"

// boolean do_nothing_proc ();
// pfi message_proc (variable *p, int messtype);
// void init_generic ();

extern int_type();

//extern vrg_type();
//extern rgKill();

//extern vmod_type();
//extern mod_kill();

extern st_pprint();
extern st_kill();

extern alias_type();

extern st_start();
extern boolean emit_dodeg();
extern boolean semit_dodeg();
extern boolean res_dodeg();
extern boolean shift_dodeg();

extern boolean istd_dodeg();

extern coll_degs();
extern coll_poly();
extern trash_poly();
extern lift_poly();
extern boolean lift_enddeg();
extern mg_degs();
extern mg_poly();
extern boolean mg_enddeg();
extern ins_generator();
extern boolean nres_enddeg();
extern ins_nres();
extern boolean std_enddeg();
extern std_degs();

char *type_names[] = {"none", "int", "ring", "poly", "matrix",
                      "complex", "computation", "emitter",
                      "emit standard",
                      "res", "shift",
                      "collector", "trash",
                      "lifter", "merger", "nres", "std", "istd"
                     };

#define NTYPES 18
pfi mint[] = {int_type, do_nothing_proc};
pfi mring[] = {vrg_type, rgKill};
pfi mpoly[] = {do_nothing_proc, do_nothing_proc};
pfi mmod[] = {vmod_type, mod_kill};

pfi mstarter[] = {st_pprint, st_kill,
                  st_start
                 };
pfi memit[] = {alias_type, do_nothing_proc,
               emit_dodeg
              };
pfi mstdemit[] = {alias_type, do_nothing_proc,
                  semit_dodeg
                 };
pfi mres[] = {vmod_type, mod_kill,
              res_dodeg
             };
pfi mshift[] = {do_nothing_proc, do_nothing_proc,
                shift_dodeg
               };
pfi mcollect[] = {vmod_type, mod_kill,
                  coll_degs, coll_poly, do_nothing_proc
                 };
pfi mtrash[] = {do_nothing_proc, do_nothing_proc,
                do_nothing_proc,trash_poly, do_nothing_proc
               };
pfi mlift[] = {alias_type, do_nothing_proc,
               mg_degs, lift_poly, lift_enddeg
              };
pfi mmerge[] = {do_nothing_proc, do_nothing_proc,
                mg_degs, mg_poly, mg_enddeg
               };
/*pfi mnres[] = {vmod_type, mod_kill,
                    coll_degs, ins_generator, nres_enddeg};*/

pfi mnres[] = {vmod_type, mod_kill,
               coll_degs, ins_nres, nres_enddeg
              };
pfi mstd[] = {vmod_type, mod_kill,
              std_degs, ins_nres, std_enddeg
             };
pfi mistd[] = {vmod_type, mod_kill,
               istd_dodeg
              };

pfi *mess[NTYPES];

boolean do_nothing_proc ()
{
    return(TRUE);
}

pfi message_proc (variable *p, int messtype)
{
    if (p IS NULL) return(do_nothing_proc);
    return(mess[p->type][messtype]);
}

void init_generic ()
{
    mess[VNOTYPE] = NULL;
    mess[VINT] = mint;
    mess[VRING] = mring;
    mess[VPOLY] = mpoly;
    mess[VMODULE] = mmod;
    mess[VSTARTER] = mstarter;
    mess[VEMIT] = memit;
    mess[VSTDEMIT] = mstdemit;
    mess[VRES] = mres;
    mess[VSHIFT] = mshift;
    mess[VCOLLECT] = mcollect;
    mess[VTRASH] = mtrash;
    mess[VLIFT] = mtrash;
    mess[VMERGE] = mmerge;
    mess[VNRES] = mnres;
    mess[VSTD] = mstd;
    mess[VISTD] = mistd;
}
