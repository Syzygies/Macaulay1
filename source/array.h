/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */

#ifndef _arrayh_
#define _arrayh_

/*--------------------------------------------------------------
 *      variable length arrays, degree lists, poly. lists
 *
 *  implements variable length arrays using linked lists of chunks.
 * Uses "stash.c"  (as does all storage allocation).  Currently, only
 * "plist's, dlist's use arrays.  Arrays occur in "array.c", and
 * routines for manipulating plist's, dlist's occur in "plist.c".
 *
 *-------------------------------------------------------------*/

#define NARRAY  600

typedef struct slabrec {
    char        vals[NARRAY] ;  /* values */
    struct slabrec *next ;  /* next slab in list */
} slab ;

typedef struct {
    int     len ;       /* length of array (1..length) */
    int     size ;      /* size, in  bytes, of each entry */
    int val_size ;  /* usable size of "val" array in each slab */
    slab *  elems ;     /* pointer to first slab of entries */
} array ;

typedef array plist ;   /* list of polynomials */
typedef array dlist ;   /* list of degree integers */

void init_array (array *a, int element_size);
void free_array (array *a);
int length (array *a);
char *ref (array *a, int i);
char *ins_array (array *a);

#define LENGTH(pl)          ((pl).len)
#define PREF(pl,i)          (*((poly *) ref(&(pl), (i))))
#define DREF(dl,i)      (*((int *) ref(&(dl), (i))))
#define pl_insert(pl,f)     { * ((poly *) ins_array(pl)) = f ; }
#define dl_insert(dl,i)     { * ((int *) ins_array(dl)) = i ; }
#define pl_init(pl)     { init_array((pl), sizeof(poly)) ; }
#define dl_init(dl)     { init_array((dl), sizeof(int)) ; }

#endif
