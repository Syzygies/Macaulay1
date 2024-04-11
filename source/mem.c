/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"

// void i_stashes ();
// char * mkPolyStash (int nblocks);
// gmatrix new_mod ();
// char * get_small (int size);

#define WORD 4

#define BLOCKSIZE 4400

extern variable *fabsvar();
extern int last_var;

char *array_stash;     /* should be set by array_init at beginning
                            of program */
char *mod_stash;
char *std_stash;
char *small_ptr;       /* these 3 vars are used for "small" storage, */
char *small_stash;     /* which won't ever be retrieved */
int amt_left;

void i_stashes ()
{
    mod_stash = open_stash(sizeof(struct modrec), "modules");
    std_stash = open_stash(sizeof(struct mn_stdrec), "std bases");
    small_stash = open_stash(BLOCKSIZE, "small memory");
    amt_left = 0;
}

char * mkPolyStash (int nblocks)
{
    return(open_stash(field_size + sizeof(poly) + (nblocks)*sizeof(int),
                      "polys in ring"));
}

gmatrix new_mod ()
{
    return((gmatrix ) get_slug(mod_stash));
}

char * get_small (int size)
{
    char *p;

    if (size IS 0) return(NULL);   /* added: Dec. 2, 85 MES */

    while (size%WORD != 0) ++size;
    if (size > BLOCKSIZE) return(NULL);
    if (size > amt_left) {
        small_ptr = get_slug(small_stash);
        amt_left = BLOCKSIZE;
    }
    p = small_ptr;
    small_ptr += size;
    amt_left -= size;
    return(p);
}
