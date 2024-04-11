/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "mtypes.h"

#define IDSIZE 50 /* max size of an identifier */

#define MTYPE 0
#define MKILL 1

#define MSTART 2
#define MDODEG 2
#define MDEGS_RECEIVE 2
#define MPOLY_RECEIVE 3
#define MENDDEG 4

#define VNOTYPE 0
#define VINT 1
#define VRING 2
#define VPOLY 3
#define VMODULE 4
#define VCOMPLEX 5

#define VSTARTER 6

#define VEMIT 7
#define VSTDEMIT 8
#define VRES 9
#define VSHIFT 10

#define VCOLLECT 11
#define VTRASH 12

#define VLIFT 13
#define VMERGE 14
#define VNRES 15
#define VSTD 16
#define VISTD 17

#define VKNAME 0
#define VKINDEX 1
#define VKABSOLUTE 2

extern char *type_names[] ;
extern pfi *mess[] ;

extern variable *find_var() ;
extern variable *make_var() ;
extern variable *newvar() ;

extern boolean do_nothing_proc() ;
extern void vrg_type(ring R) ;
extern void vmod_type(gmatrix M) ;
extern void rgKill(ring R) ;
extern mod_kill() ;
extern pfi message_proc() ;

/*
define varb_read(fil,pv,offset)  ((*(message_proc(pv, MREAD))) \
                    (fil, &(pv->value), offset))
define varb_write(fil,pv) ((*(message_proc(pv, MWRITE)))(fil,pv->value))
*/
#define varb_kill(pv)      ((*(message_proc(pv, MKILL)))(pv->value))
#define varb_type(pv)      (((pv)->b_alias IS NULL) ? \
    (*(message_proc((pv), MTYPE)))((pv)->value) \
    : (*(message_proc((pv)->b_alias, MTYPE)))((pv)->b_alias->value))

#define out_box(box)        (box->b_next)

#define start(box, argc, argv)  ((*(message_proc(box, MSTART)))  \
                                        (box, argc, argv))

#define do_degree(box, deg)     ((*(message_proc(box, MDODEG)))  \
                    (box, deg))

#define end_in_degree(box, deg) ((*(message_proc(box, MENDDEG))) \
                    (box, deg))
#define send_poly(box, f, deg)  ((*(message_proc(box, MPOLY_RECEIVE)))  \
                                        (box, f, deg))
#define send_degs(box, dl, deg) ((*(message_proc(box, MDEGS_RECEIVE))) \
                                        (box, dl, deg))

extern variable *mkcollect() ;
extern variable *mktrash() ;
extern variable *mknres() ;
extern variable *mkres() ;
extern variable *mkstd() ;
extern variable *mkistd() ;
extern variable *mkemit() ;
extern variable *mkstdemit() ;
extern variable *mklift() ;
extern variable *mkmerge() ;
extern variable *mkshift() ;
extern variable *mkstarter() ;

extern start_rec *st_init() ;

#define GET_MOD(g, i)   { if (((g) = vget_mod(argv[(i)])) IS NULL) \
                return ; }

#define GET_rgMOD(g, i) { if (((g) = vget_rgmod(argv[(i)])) IS NULL) \
                return ; }

#define GET_VMOD(p, i)  { if (((p) = vget_vmod(argv[(i)])) IS NULL) \
                return ; }

#define NEW_MOD(p, i)   { if (((p) = make_var(argv[(i)], MAINVAR, VMODULE, \
                current_ring)) IS NULL) \
                return ; }

#define NEW_INT(p, i)   { if (((p) = make_var(argv[(i)], MAINVAR, VINT, \
                NULL)) IS NULL) \
                return ; }

#define NEW_RING(p, i)  { if (((p) = make_var(argv[(i)], MAINVAR, VRING, \
                NULL)) IS NULL) \
                return ; }

#define NEW_savMOD(p, s) { if (((p) = make_var((s), MAINVAR, VMODULE, \
                current_ring)) IS NULL) \
                return ; }

#define NEW_savINT(p, s) { if (((p) = make_var((s), MAINVAR, VINT, \
                NULL)) IS NULL) \
                return ; }

#define NEW_savRING(p, s) { if (((p) = make_var((s), MAINVAR, VRING, \
                NULL)) IS NULL) \
                return ; }

#define GET_RING(R, i)  { if (((R) = get_ring(argv[(i)])) IS NULL) \
                    return ; }
#define GET_VRING(p, n)  { if (((p) = vget_ring(argv[n])) IS NULL) return;}
#define GET_CRING(p)    { if (((p) = current_ring) IS NULL) return;}

extern gmatrix vget_mod() ;
extern variable *vget_vmod() ;
extern gmatrix vget_rgmod() ;
extern variable *vget_ring() ;
extern ring get_ring() ;
