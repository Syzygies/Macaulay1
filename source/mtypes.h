/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#define MAXNEG (-30000)
#define BYTE    0377

#include "style.h"

#include "array.h"
#include "ddefs.h"

#ifndef _iring_
#include "ring.h"
#endif

/*--------------------------------------------------------------
 * Used by cmdnames.c and routines accessing list of commands
 *-------------------------------------------------------------*/

typedef struct cmd_rec {
    char *name;
    pfv proc;
    int args;
} cmd_rec;

/*--------------------------------------------------------------
 *      polynomial data types
 *
 *      These include field elements (characteristic p numbers),
 * monomials, and polynomials themselves.  Files which include the
 * routines for manipulating these are : "charp.c", "term.c", and
 * "poly.c".  Depends heavily on what the current ring is.
 *-------------------------------------------------------------*/

#define INITIAL(f)      ((f)->monom)
#define LEAD_COEF(f)    ((f)->coef)

typedef short field ;

typedef struct pol *poly ;
struct pol {
    poly    next ;          /* next monom. in poly */
    field       coef ;          /* coef. of this monom */
    allocterm   monom ;         /* variables, and component of monom */
} ;

/*--------------------------------------------------------------
 *      module's (graded matrices)
 *
 *      gmatrices are used both for matrices (only degrees, deggens,
 * and gens are used in this case), and for standard basis
 * computations (keeps other info).  Some routines for manipulating
 * modules appear in "plist.c".  More complicated routines (std. basis)
 * occur in "standard.c".
 * Monomial routines occur in "monoms.c".
 *-------------------------------------------------------------*/

#define MNOTHING (-1)
#define MMAT 0
#define MSTD 1
#define MISTD 2

#define USEMAT 0
#define USESTD 1
#define USECHANGE 2

typedef struct mn_stdrec *mn_standard ;
struct mn_stdrec {
    mn_standard next ;
    poly    standard ;
    poly    change ;
    char    ismin ;     /* used only in inhomog std bases,boolean*/
    int     degree ;
} ;

/* see stdcmds.c for details about this "module generator" */
typedef struct {
    char mtype ;    /* one of MSTD, MISTD, MMAT */
    int misstd ;    /* USESTD, USECHANGE, or USEMAT */
    mn_standard mp ;    /* used in MSTD */
    arrow mq ;      /* used in MISTD */
    array *mmontab ;    /* used in MISTD */
    int mi ;        /* used in MISTD, MMAT */
    plist *mgens ;  /* in MMAT, plist used to find matrix */
} modgen ;

typedef array mn_table ;
typedef array mn_syzes ;

typedef struct modrec *gmatrix ;
struct modrec {
    gmatrix next ;      /* points to next in complex */
    int     modtype ;   /* MMAT, MSTD, MISTD */
    dlist       degrees ;       /* degrees of each component */
    dlist       deggens ;       /* degrees of generators */
    plist       gens ;          /* generators for module */
    int     nstandard ; /* number of standard basis elements */
    mn_standard stdbasis ;  /* list of generators of std basis,
                   in descending degree order */
    mn_table    montab ;    /* fast monomial lookup */
    mn_syzes    monsyz ;    /* list of S-pairs to be done */
    int     mn_lodeg ;  /* lowest deg in "degrees", or MAXNEG,
                    if not set yet */
} ;

/*-----
 *
 * Hilbert functions
 *
 *-----*/

#define GFSIZE 256
typedef struct {
    int codim ;
    int deg0 ;  /* degree of t^0 in genfun */
    long degree ;   /* sum of entries in genfun */
    long genfun[GFSIZE] ;
} hilbFunc ;

/*--------------------------------------------------------------
 *      variables
 *
 *      These are variables that the user controls.  Each variable
 * has a base ring, name, type etc.  The entire list of variables is
 * kept in an array "var_list".  Variable manipulations
 * occur in "vars.c", and alot of shell commands for dealing with
 * variables occur in "cmds.c".
 *-------------------------------------------------------------*/

#define VREF(n)         ((variable *) ref(&var_list, (n)))

#define VAR_RING(p)     ((ring) p->value)
#define VAR_MODULE(p)       ((gmatrix) p->value)
#define VAR_POLY(p)     ((poly) p->value)
#define VAR_INT(p)      ((int) p->value)

typedef char * ADDRESS ;

#define DEAD 0
#define KEEP 1

#define REMOVED 0
#define MAINVAR 1
#define PARTVAR 2

typedef struct varb_rec variable ;
struct varb_rec {
    char exists ;       /* 2 values: EXISTS, REMOVED */
    char garbage ;      /* 2 values: DEAD, KEEP: used only in
                    garbage collection of variables */
    int var_num ;       /* integer number of variable */
    char *name ;        /* pointer to null terminated name */
    int type ;      /* 0=no type, 1=int, 2=ring, poly,
                    module, rmap, starter, ... */
    variable *b_ring ;  /* points to base ring for var, if any*/
    variable *b_next ;  /* next var: used in box comps and
                    complexes */
    variable *b_alias ; /* alias var: used in std,istd,syz */
    ADDRESS value ;     /* usually a pointer to data */
    int intval ;        /* can be used by variable in any way desired*/
} ;

typedef struct {
    int maxgen ;
    int lastdeg ;
    int numstart ;
    boolean iscomplete ;
    int hideg ;     /* if hideg >= lastdeg, this comp. is in progr.*/
    boolean doalldegs ;
} start_rec ;

/*--------------------------------------------------------------
 *      external variables
 *
 *      there are several sets:
 *
 *  1. special field, monomial, or poly vars.
 *      e.g:  fd_one, fd_zero, field_size.
 *  2. stash variables: pointers to various storage areas.
 *      e.g:  mod_stash, array_stash.
 *  3. shell variables: various global values used by the shell (and
 *          sometimes by others, e.g. verbose, outfile
 *      e.g:  outfile, linesize, verbose, level, var_list.
 *-------------------------------------------------------------*/

extern field fd_one ;           /* fd_one, fd_zero are usable globally */
extern field fd_zero ;          /* but dont change them! */
extern int field_size ;
extern int term_size ;

extern variable *current_ring ;

extern char *std_stash ;
extern char *mod_stash ;
extern char *array_stash ;

extern FILE *outfile ;
extern int level ;
extern array var_list ;
extern int last_var ;

/*--------------------------------------------------------------
 *      external routines
 *
 *      the only routines appearing here are those that return
 * pointer values.  For the generic routines of "vars.c", see "vars.h".
 *
 *-------------------------------------------------------------*/

extern poly p_listhead() ;
extern poly p_monom (field a);
extern poly p_initial() ;
extern poly e_sub_i() ;
extern poly p_mult() ;
extern poly p_copy() ;
extern poly mult_sub() ;
extern poly p1_mult() ;
extern poly reduce() ;

extern char *var_name() ;
extern gmatrix mod_init() ;
extern gmatrix mat_scan() ;
extern gmatrix ideal_scan() ;

extern char *open_stash() ;
extern char *get_slug() ;
extern gmatrix mat_copy() ;
extern gmatrix new_mod() ;
extern char *get_small() ;

extern char *ref() ;
extern char *ins_array() ;

extern char *get_str() ;
extern gmatrix vget_mod() ;

extern FILE * vf_open() ;

extern poly stdNext() ;

extern poly parsePoly() ;
extern poly readPoly() ;
extern poly getPoly() ;
extern poly rdPoly() ;
extern poly rdPolyStr() ;

/* set variables */

extern int linesize ;
extern int verbose ;
extern int pr_charp ;
extern int prlevel ;
extern int timer ;
extern int autocalc ;
extern int autodegree ;
extern int maxdegree ;
extern int echo ;
extern int autoReduce ;
extern int nlines ;
extern int prcomment ;
