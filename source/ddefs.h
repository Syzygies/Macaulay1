/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
/* ddefs.h  */

#ifndef _ddefsh_
#define _ddefsh_

#include "ring.h"

#define FOW 0      /* forward link  */
#define BAK 1      /* backward link  */
#define NCOMS 5000
#define NDEGS 400

#define AR (arrow) &

#define BAG(p)  ((p)->umn.ldc.ci)

typedef union {
    char * ci;
    union all *ca;
} cargo;

#define LINKDUK \
  union all *lda[2]; \
  cargo ldc; \
  char ldkind;

typedef struct {
    LINKDUK
} linkduk;

typedef struct {
    LINKDUK
    expterm mexp ;
} mmonom;

typedef struct {
    LINKDUK
    int mlev;
} monparen;

struct mstk {
    short int mi, mpre;
    union all *ma;
} ;

typedef struct {
    LINKDUK
    union all *mloc;
    int mn, mii, mimin, mpren, mpred; /* short int changed 12/5/85 MES*/
    struct mstk mstack[NVARS-1];
} monhead;

union all {
    int ui;
    union all *ua;
    cargo ucg;
    linkduk uld;
    mmonom umn;
    monparen ump;
    monhead umh;
};

typedef union all *arrow;

typedef struct mn_pr {
    char *id1, *id2;
    struct mn_pr *mpp;
} mn_pair;

extern arrow monnewhead() ;
extern arrow monnext() ;
extern arrow monadv() ;

#endif
