/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#ifndef _ringh_
#define _ringh_

#include "array.h"
#include <stdio.h>

/*--- values which may be modified, subject to certain constraints ---*/

#define LARGE_PRIME 31991

#define NVARS   512

/*--- private types ---*/
#ifndef _iring_

typedef char * ring ;
typedef char * term ;

#endif
/*--- visible types ---*/

typedef int expterm[NVARS] ;
typedef int bigterm[NVARS] ;
typedef unsigned long allocterm[NVARS] ;

/*--- routines involving rings ---*/

extern void rgInstall (ring R);
extern void rgKill(ring R);
extern ring rgCreate();
extern ring rgScan();
extern void rgDisplay (FILE *fil, ring R);
extern void rgPut (FILE *fil, ring R);

/*--- variables defined after doing "rgInstall" ---*/

extern char **varnames ;/* names of all vars in current ring, 0..numvars-1 */
extern int numvars ;    /* number of variables in current ring */
extern int nblocks ;    /* number of variable blocks in current ring */
extern term zerodegs ;
extern dlist rgDegs ;   /* degrees of each variable 1..numvars */

extern int   charac ;
extern char *headStash ;
extern char *monStash ;
extern char *parenStash ;
extern char *pStash ;

/*--- routines involving monomials (terms) ---*/

#endif
