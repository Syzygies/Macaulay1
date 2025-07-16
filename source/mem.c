// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "mem.h"
#include "stash.h"

// Constants
#ifdef SPARC
constexpr int WORD = 4;
#else
constexpr int WORD = 2;
#endif
constexpr int BLOCKSIZE = 4400;

// Global variables
// array_stash is defined in array.c and declared in array.h
char *mod_stash;
char *std_stash;

// Static variables for small allocation management
static char *small_ptr;   // these 3 vars are used for "small" storage,
static char *small_stash; // which won't ever be retrieved
static int amt_left;

void i_stashes(void)
{
    mod_stash = open_stash(sizeof(struct modrec), "modules");
    std_stash = open_stash(sizeof(struct mn_stdrec), "std bases");
    small_stash = open_stash(BLOCKSIZE, "small memory");
    amt_left = 0;
}

char *mkPolyStash(int nblocks)
{
    // Allocate space for the struct pol plus the monomial data
    // The coef field is already part of struct pol, so we don't add field_size
    // CRITICAL 64-bit fix: term data is smallmon (unsigned long), not int!
    unsigned int size =
        (unsigned int)sizeof(struct pol) + (unsigned int)nblocks * (unsigned int)sizeof(smallmon);
    return open_stash((int)size, "polys in ring");
}

gmatrix new_mod(void)
{
    return (gmatrix)(void *)get_slug((struct stash *)mod_stash);
}

char *get_small(int size)
{
    char *p;

    if (size == 0)
        return NULL; // added: Dec. 2, 85 MES

    while (size % WORD != 0)
        ++size;
    if (size > BLOCKSIZE)
        return NULL;
    if (size > amt_left)
    {
        small_ptr = get_slug((struct stash *)small_stash);
        amt_left = BLOCKSIZE;
    }
    p = small_ptr;
    small_ptr += size;
    amt_left -= size;
    return p;
}
