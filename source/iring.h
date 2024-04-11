/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#ifndef _iring_
#define _iring_

typedef unsigned long smallmon ;
typedef smallmon *term ;
typedef struct symblock *symmBlock ;
typedef struct ringrec *ring ;


#include "ring.h"

#define RSYMM   (-1)    /* see below Symmblock for meaning */
#define RCOMP   (-2)
#define RWTFCN  (-3)    /* these numbers must be negative, see getNumVars */

struct symblock {
    int rtyp ;  /* = RSYMM for this type */
    int nvars;  /* number of variables in this block */
    int *   degs ;  /* positive integer weight of each variable 0..nvars-1 */
    char ** names ; /* name of each variable */

    smallmon *  table ; /* 2 dim. array[0..nvars-1, 0..maxdeg] */
    smallmon ** cols ;  /* points into table: 0..nvars-1 */
    smallmon    maxval ;/* largest allowable value */

    boolean w1 ;    /* TRUE means all weights are = 1. */
    int maxdeg;     /* largest allowed degree */
    int defaultmax ;    /* what "maxdegree" was set to when this symmblock
               was created */
    int over ;      /* maxdeg+1;  used to speed some code */
    int n1 ;        /* nvars-1;  used to speed some code */
    int degree;     /* cached value for last degree used; speeds up code */

    void      (*stob)() ;   /* routine small-->big */
    smallmon  (*btos)() ;   /* routine big-->small */
    smallmon  (*madd)() ;   /* monomial add: 1 big with 1 small */

    symmBlock   nextBlock ; /* used to prevent table duplication */
} ;

/*-----------
 * Types of slots:
 *
 *  RSYMM: encoded monomial, the "usual" case
 *  RCOMP: the slot for the module component, only one allowed
 *      No fields (except styp) have meaning.
 *  RWTFCN: a weight function.
 *      The weight function is given by the fields "degs", which has
 *      length = numvars for the whole ring.  No other fields have
 *      meaning.
 *
 * This is a bit of a kludge, maybe making their own types would be better...
 *------------*/

struct ringrec {
    int     charac ;    /* characteristic */
    int     nvars ;     /* number of variables */
    char ** varnames ;  /* array of names, indexed: 0..nvars-1 */
    dlist   degs ;      /* array of weights, indexed: 1..nvars */
    dlist   monorder ;  /* monomial order */
    dlist   wtfcns ;    /* weight functions, mashed together */
    int     nblocks ;   /* size of monomials (how many smallmon's) */
    symmBlock * symmalg ;   /* array of info on each block: 0..nblocks-1 */
    int     compLoc ;   /* location of component, -1 = no where */
    term    zerodegs ;  /* exponent of all zeros */
    char *  pStash ;    /* stash for polynomials */
    char *  headStash ;
    char *  monStash ;
    char *  parenStash ;
    char *  Rideal ;    /* type=arrow, NULL: no ideal here */
    char *  Rstd ;      /* type=mn_standard, list of Rideal */
} ;

extern symmBlock *blocks ; /* array(0..nblocks-1) of info for each block in
                current ring */
extern int compLoc ;

extern char *gimmy() ;
extern symmBlock symmCreate() ;
extern char **getVars() ;

#endif
