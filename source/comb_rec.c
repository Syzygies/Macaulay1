/*-----------------------------------------------------------*/
/* comb_rec.c  procedures for dealing with lists of columns  */
/* and rows, and combinations (needed for calculating a      */
/* unique number for a give set of columns or rows, for      */
/* indexing)                                                 */
/*-----------------------------------------------------------*/
#include <stdio.h>
#include "getdet.h"

// int init_combs (int maxn);
// void free_combs ();
// int comb_num (int *ary, int length, int n);
// int next_comb (int *t, int *sign, int length, int range);

#include <stdlib.h>

char *recstash;

extern char *free_slug();
extern char *get_slug();
extern char *open_stash();

/*---------------------------------*/
/* binomial combinations suite     */
/*---------------------------------*/

int **comb_table;
int   table_length;

#define combs(n,r)  comb_table[(n)][(r)]

int init_combs (int maxn)
{
    int n,r;

    /* note, this is not memory efficient, but ought to be <fast> */
    /* (one could ditch more than half of this, and use a few if statements
       in the call to get the value. It's a tradeoff */

    if ((comb_table = (int **)malloc((maxn+1)*sizeof(int *))) == NULL)
        return 0;

    table_length = maxn + 1;

    for (n=0; n <= maxn; n++)
        comb_table[n] = NULL;

    for (n=0; n <= maxn; n++)
    {
        if ((comb_table[n] = (int *)malloc((n+1)*sizeof(int))) == NULL)
            return 0;

        comb_table[n][0] = 1;
        comb_table[n][n] = 1;
        for(r=1; r<n; r++)
            comb_table[n][r] = comb_table[n-1][r-1]+comb_table[n-1][r];
    }
}

void free_combs ()
{
    int i;

    for (i=0; i<table_length; i++)
        if (comb_table[i] != NULL)
            free(comb_table[i]);

    free(comb_table);
}

/**--------------------------------**/
/** combination record suite       **/
/**--------------------------------**/

struct comb_rec
{
    int *t;
    int length;
    int sign;
};

init_comb_recs(maxl)              /* prepare the memory allocation */
int maxl;
{
    recstash = open_stash(maxl*sizeof(int),"combinations");
}

get_comb_rec(c,l)                  /* allocate */
struct comb_rec *c;
int l;
{
    c->t = (int *)get_slug(recstash);
    c->length = l;
}

init_comb_rec(c)                   /* fill with consecutive integers */
struct comb_rec *c;
{
    int i;

    for(i=0; i<c->length; i++)
        c->t[i] = i;
    c->sign = 1;
}

display_comb_rec(c)                /* for debugging */
struct comb_rec *c;
{
    int i;

    for (i=0; i< c->length-1; i++)
        fprintf(stderr,"%d,",c->t[i]);
    fprintf(stderr,"%d",c->t[c->length-1]);
}

/*---------------------------------------------------*/
/* int comb_num - index of this combination, for     */
/* lookup. Precondition: 1<n<=maxn, where maxn is    */
/* the value with which init_combs was called.       */
/*---------------------------------------------------*/
int comb_num (int *ary, int length, int n)
{
    int i,j,a,result=0;

    /* go from slowest to fastest varying digits, adding up how many
       combinations must have passed to get here */

    for(i=0,a=0; i<length; i++,a++)
    {
        for(j=a; j < ary[i]; j++,a++)
        {
            result += combs(n-j-1,length-i-1);
        }
    }
    return result;
}

/* This routine, like pincr_array, goes through combinations of a fixed
   size from a set of a fixed larger size, but it is zero initial */

int next_comb (int *t, int *sign, int length, int range)
{
    int i,temp;

    for(i=length-1;
            i >= 0 && t[i] - i == range - length;
            i--);

    if (i < 0)
        return 0;

    temp = t[i] - i;             /* t[i] - i is the number of skipped values */

    *sign = ODD(temp)?-1:1;      /* flip sign once for each skipped entry */

    for (t[i++]++; i<length; i++)
        t[i]=t[i-1]+1;

    return 1;
}

free_comb_rec(c)               /* deallocation / cleanup */
struct comb_rec *c;
{
    free_slug(recstash,(char *)c->t);
}

free_comb_recs()               /* deallocation / cleanup for everything */
{
    endof_stash(recstash);
}
