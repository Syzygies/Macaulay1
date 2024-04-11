/*----------------------------------------------------------*/
/* getdet.c  for calculating minors of a matrix             */
/* two implementations                                      */
/*----------------------------------------------------------*/
#include <stdio.h>

// poly calc_det (gmatrix M, int *r, int *c, int p);
// gmatrix determinants (gmatrix M, int s);
// void determinants_cmd (int argc, char *argv[]);

#include "vars.h"
#include "comb_rec.h"
#include "tables.h"

#include "getdet.h"
int Temp;             /* keep this with swapint */

#define WIDTH 100
#define HEIGHT 100

extern poly gm_elem();

#ifdef OPCOUNT
unsigned multcount=0,addcount=0;
#endif

/*----------------------------------------------------------*/
/* poly calc_det                                            */
/* calculate the minor with rows r[0] through r[p-1] and    */
/* columns c[0] through c[p-1] by expansion and cofactors   */
/*----------------------------------------------------------*/
poly calc_det (gmatrix M, int *r, int *c, int p)
/* matrix from which to calculate */
/* vectors of row and column references */
/* size of determinant to calculate */
{
    int negate = 0;
    int i;
    poly result;
    int temp;
    poly *prevresult;             /* lookup to see if we've done this before */
    poly ptemp;
    poly ptemp2;

    /* NOTE - base case will occur at p==1 when prevresult will be
       found in the table */

    if (*(prevresult = stored_value(r,c,p))        /* get a pointer to where
                            the result belongs */
            != NO_ENTRY)
        return (*prevresult);                        /* this one has already been
                            done */

    /* pull out each column and put it at the beginning, then calculate
       the remaining minor */

    if ((ptemp2 = MATRIXSUB(*c,*r)) == NULL)
        result = NULL;
    else
        result = p_mult(ptemp2, calc_det(M,r+1,c+1,p-1));

#ifdef OPCOUNT
    multcount++;
#endif

    for(i=1; i<p; i++)
    {
        negate = !negate;

        SWAPINTS(c[0],c[i]);

        if ((ptemp2 = MATRIXSUB(*c,*r)) != NULL)
        {
            ptemp = p_mult(ptemp2,
                           calc_det(M,r+1,c+1,p-1));

            if (negate)
                p_sub(&result,&ptemp);
            else
                p_add(&result,&ptemp);

#ifdef OPCOUNT
            multcount++;
            addcount++;
#endif

        }
    }

    /* pulling out the columns has disordered c. Fix it */

    temp = c[0];
    for(i=0; i<p-1; i++)
        c[i] = c[i+1];
    c[p-1] = temp;

    /* stick the newly calculated number in the table at the previously
       looked up spot */

    *prevresult = result;

    return result;
}

gmatrix determinants (gmatrix M, int s)
{
    int x,y,i;
    struct comb_rec rowlist,collist;
    gmatrix result;

    x = ncols(M);
    y = nrows(M);

    result = mod_init();
    dl_insert(&result->degrees,0);

    if (init_combs(MAX(x,y)) == 0) {
        printf("not enough memory to use this command\n");
        return result;
    }
    if (init_comb_recs(s) == 0) {
        printf("not enough memory to use this command\n");
        return result;
    }
    if (init_tables(M,x,y,s) == 0) {
        printf("not enough memory to use this command\n");
        free_comb_recs();
        free_combs();
        return result;
    }

    get_comb_rec(&rowlist,s);
    get_comb_rec(&collist,s);
    init_comb_rec(&rowlist);

    do
    {
        i = 0;
        init_comb_rec(&collist);
        do
        {
            gmInsert(result,p_copy(calc_det(M,rowlist.t,collist.t,s)));
        }
        while(next_comb(collist.t,&collist.sign,collist.length,x));
    }
    while(next_comb(rowlist.t,&rowlist.sign,rowlist.length,y));

    free_comb_rec(&rowlist);
    free_comb_rec(&collist);

    free_tables();
    free_comb_recs();
    free_combs();

#ifdef OPCOUNT
    fprintf(stderr,"Used %u adds and %u multiplies.\n",addcount,multcount);
#endif

    return result;
}

void determinants_cmd (int argc, char *argv[])
{
    gmatrix   g;
    int       n;
    variable *p;

    if (argc != 4)
    {
        printnew("determinants <matrix> <p> <result>\n");
        return;
    }

    GET_MOD(g, 1);

    n = getInt(argv[2]);

    if ((n <= 0) OR (n > nrows(g)) OR (n > ncols(g)))
    {
        prerror("; all size %d minors are zero\n", n);
        return;
    }

    NEW_MOD(p, 3);
    set_value(p, determinants(g, n));
}
