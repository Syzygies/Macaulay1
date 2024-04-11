/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#ifndef _styleh_
#define _styleh_

#define comp compa

#ifdef DEBUG
#define ASSERT(A,B) \
    if (!(B)) { printf("%s : assertion (B) fails\n", A); to_shell(); }
#else
#define ASSERT(A,B)
#endif

/* general definitions */

typedef int boolean ;
#define TRUE    (-1)
#define FALSE   0

#define IS      ==
#define ISNT    !=
#define AND     &&
#define OR      ||
#define NOT     !

#define LT (-1)
#define EQ 0
#define GT 1


typedef int (*pfi)() ;
typedef void (*pfv)() ;

#include <stdio.h>
#include <ctype.h>

#ifndef MAX
#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)        (((a) > (b)) ? (b) : (a))
#endif

void fprint (FILE * fil, char * fmt, ...);
void print (char * fmt, ...);
void fprintnew (FILE * fil, char * fmt, ...);
void printnew (char * fmt, ...);
void prinput (char * fmt, ...);
void prinput_error (char * fmt, ...);
void prerror (char * fmt, ...);
void prflush (char * fmt, ...);
void debugpr (char * fmt, ...);
void monprint (char * fmt, ...);

#endif

