/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#ifndef _PARSEH_
#define _PARSEH_

#include <setjmp.h>

#define LXEOI 0
#define LXINT 1
#define LXPOLY 2
#define LXLT 3
#define LXLE 4
#define LXGT 5
#define LXGE 6
#define LXEQ 7
#define LXNE 8
#define LXOR 9
#define LXAND 10
#define LXPLUS 11
#define LXMINUS 12
#define LXMULT 13
#define LXDIV 14
#define LXMOD 15
#define LXEXP 16
#define LXLP 17
#define LXRP 18
#define LXUMINUS 19 /* unary minus */
#define LXUPLUS 20  /* unary plus */
#define LXNOT 21    /* unary NOT */

#define NARGS 2     /* max. number of arguments of any of our ops */

extern int fprec[] ;
extern int gprec[] ;
extern int nargs[] ;

extern jmp_buf jmpparse ; /* used to catch parser errors */
extern char *beginStr ;   /* used for printing errors */
extern int amParsing;  /* =1 if setjmp(jmpparse), beginStr have been set, else 0*/


/* stack definitions */

#define NSTACK 50

typedef struct {
    int tos ;   /* top of stack */
    int vals[NSTACK] ; /* stack itself */
} stack ;

#endif
