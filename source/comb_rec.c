// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <stdio.h>  // fprintf
#include <stdlib.h> // malloc, free
#include "shared.h"
#include "comb_rec.h"
#include "stash.h" // open_stash, get_slug, free_slug, endof_stash

// Inline functions to replace macros
static inline int combs(int n, int r)
{
    return comb_table[n][r];
}

static inline int ODD(int x)
{
    return (x % 2) != 0;
}

char *recstash;

// binomial combinations suite

int **comb_table;
int table_length;

void init_combs(int maxn)
{
    int n, r;

    // note, this is not memory efficient, but ought to be <fast>
    // (one could ditch more than half of this, and use a few if statements
    // in the call to get the value. It's a tradeoff

    if ((comb_table = (int **)malloc((size_t)(maxn + 1) * sizeof(int *))) == NULL)
        return;

    table_length = maxn + 1;

    for (n = 0; n <= maxn; n++)
        comb_table[n] = NULL;

    for (n = 0; n <= maxn; n++)
    {
        if ((comb_table[n] = (int *)malloc((size_t)(n + 1) * sizeof(int))) == NULL)
            return;

        comb_table[n][0] = 1;
        comb_table[n][n] = 1;
        for (r = 1; r < n; r++)
            comb_table[n][r] = comb_table[n - 1][r - 1] + comb_table[n - 1][r];
    }
}

void free_combs(void)
{
    int i;

    for (i = 0; i < table_length; i++)
        if (comb_table[i] != NULL)
            free(comb_table[i]);

    free(comb_table);
}

// *--------------------------------*
// * combination record suite       *
// *--------------------------------*

void init_comb_recs(int maxl) // prepare the memory allocation
{
    recstash = open_stash((int)((size_t)maxl * sizeof(int)), "combinations");
}

void get_comb_rec(struct comb_rec *c, int l) // allocate
{
    // The stash allocator returns aligned memory, safe to cast
    void *ptr = get_slug((struct stash *)recstash);
    c->t = (int *)ptr;
    c->length = l;
}

void init_comb_rec(struct comb_rec *c) // fill with consecutive integers
{
    int i;

    for (i = 0; i < c->length; i++)
        c->t[i] = i;
    c->sign = 1;
}

void display_comb_rec(struct comb_rec *c) // for debugging
{
    int i;

    for (i = 0; i < c->length - 1; i++)
        fprintf(stderr, "%d,", c->t[i]);
    fprintf(stderr, "%d", c->t[c->length - 1]);
}

// int comb_num - index of this combination, for
// lookup. Precondition: 1<n<=maxn, where maxn is
// the value with which init_combs was called.

int comb_num(int *ary, int length, int n)
{
    int i, j, a, result = 0;

    // go from slowest to fastest varying digits, adding up how many
    // combinations must have passed to get here

    for (i = 0, a = 0; i < length; i++, a++)
    {
        for (j = a; j < ary[i]; j++, a++)
        {
            result += combs(n - j - 1, length - i - 1);
        }
    }
    return result;
}

// This routine, like pincr_array, goes through combinations of a fixed
// size from a set of a fixed larger size, but it is zero initial

int next_comb(int *t, int *sign, int length, int range)
{
    int i, temp;

    for (i = length - 1; i >= 0 && t[i] - i == range - length; i--)
        ;

    if (i < 0)
        return 0;

    temp = t[i] - i; // t[i] - i is the number of skipped values

    *sign = ODD(temp) ? -1 : 1; // flip sign once for each skipped entry

    for (t[i++]++; i < length; i++)
        t[i] = t[i - 1] + 1;

    return 1;
}

void free_comb_rec(struct comb_rec *c) // deallocation / cleanup
{
    free_slug((struct stash *)recstash, (struct slug *)c->t);
}

void free_comb_recs(void) // deallocation / cleanup for everything
{
    endof_stash((struct stash *)recstash);
}
