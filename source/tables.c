/*-----------------------------------------------------------*/
/* tables.c  procedures for maintaining the data structures  */
/* which keep track of previously calculated minors          */
/*-----------------------------------------------------------*/
#include <stdio.h>

// int init_tables (gmatrix M, int width, int height, int minorsize);
void free_tables ();
// poly *stored_value (int *r, int *c, int l);

#include <stdlib.h>

#include "mtypes.h"
#include "comb_rec.h"
#include "getdet.h"

#ifdef TABLESTAT
int hit=0,miss=0;
int hitlist[20],misslist[20];
#endif

extern poly gm_elem();

typedef struct
{
    poly      *datum;
    int        width,height;
} table;

poly NO_ENTRY;

/* this macro defined so we can allocate multidimentional
   arrays and reference by two coordinates */
#define TABLESUB(tab,x,y) (*((tab).datum + (tab).width*(y) + (x)))

table *tablepile;           /* THE structure */

int widthrange,heightrange,tabledepth;

/*-----------------------------------------------------------*/
/* init_tables                                               */
/* prepare the storage structure. Allocate the array of      */
/* pointers to tables, and then the tables themselves. we    */
/* don't need to store things smaller than 1x1, but we do    */
/* need minorsizexminorsize. Shift the pointer so that we    */
/* can lookup without shifting all the time.                 */
/*-----------------------------------------------------------*/
int init_tables (gmatrix M, int width, int height, int minorsize)
{
    int i,x,y;

    if ((NO_ENTRY = (poly)malloc(1)) == NULL)
        return 0;

    if ((tablepile = (table *)malloc((minorsize)*sizeof(table))) == NULL)
        return 0;

    tablepile -= 1; /* shift access range so 1x1's come in first alloc'd slot */

    tablepile[1].width = combs(width,1);
    tablepile[1].height = combs(height,1);
    if ((tablepile[1].datum =
                (poly *)malloc(tablepile[1].width
                               *tablepile[1].height
                               *sizeof(poly)))
            == NULL) {
        tabledepth = 0;
        free_tables();
        return 0;
    }

    for(x=0; x<tablepile[1].width; x++)
        for(y=0; y<tablepile[1].height; y++)
        {
            TABLESUB(tablepile[1],x,y) = gm_elem(M,y+1,x+1,1);
        }
    printf("read in matrix...\n");
    fflush(stdout);
    for(i = 2; i<=minorsize; i++)
    {
        tablepile[i].width = combs(width,i);
        tablepile[i].height = combs(height,i);
        if ((tablepile[i].datum =
                    (poly *)malloc(tablepile[i].width
                                   *tablepile[i].height
                                   *sizeof(poly)))
                == NULL) {
            tabledepth = i-1;
            free_tables();
            return 0;
        }
        else {
            for(x=0; x<tablepile[i].width; x++)
                for(y=0; y<tablepile[i].height; y++)
                {
                    TABLESUB(tablepile[i],x,y) = NO_ENTRY;
                }
        }
    }

    widthrange = width;
    heightrange = height;
    tabledepth = minorsize;

#ifdef TABLESTAT
    /* stats for design. Remove in final version. (?) */
    for(i=0; i<=minorsize; i++)
        hitlist[i]=misslist[i]=0;
#endif
}

/*-----------------------------------------------------------*/
/* free_tables                                               */
/* deallocate the tables. Each table contains pointers to    */
/* allocated values which must be freed. Free the values,    */
/* free the tables, free the table list                      */
/*-----------------------------------------------------------*/
void free_tables ()
{
    int i,x,y;

    for (i=2; i<=tabledepth; i++)
    {
        for(x=0; x<tablepile[i].width; x++)
            for(y=0; y<tablepile[i].height; y++)
                if (TABLESUB(tablepile[i],x,y) != NO_ENTRY)
                    p_kill(&TABLESUB(tablepile[i],x,y));
        free(tablepile[i].datum);
    }

    tablepile += 1;         /* unshift tablepile for the free, see init_tables */
    free(tablepile);

    free(NO_ENTRY);

#ifdef TABLESTAT
    fprintf(stderr,"%d hits, %d misses, (%d total).\n",hit,miss,hit+miss);
    fprintf(stderr,"breakdown:\n\tsize\thits\tmisses\ttotal\n");
    for(i=2; i<tabledepth; i++)
        fprintf(stderr,"\t%d\t%d\t%d\t%d\n",i,
                hitlist[i],misslist[i],hitlist[i]+misslist[i]);
#endif
}

/*-----------------------------------------------------------*/
/* poly **stored_value                                       */
/* assuming the data structures have been allocated, return  */
/* a pointer to the table entry where the pointer to the     */
/* the value of the determinant specified ought to be. If    */
/* the table entry is null, it still needs to be calculated. */
/* otherwise, the caller can simply reference it.            */
/*-----------------------------------------------------------*/
poly *stored_value (int *r, int *c, int l)
/* row and column lists */
/* length of lists = dimension of determinant */
{
    int x,y; /* indeces of row and column lists */

#ifdef TABLESTAT
    poly *temp;
#endif

    x = comb_num(c,l,widthrange);       /* c is a list of horiz. coord.s */
    y = comb_num(r,l,heightrange);      /* r is a list of vert. coord.s */

#ifdef TABLESTAT

    if (*(temp=&TABLESUB(tablepile[l],x,y)) != NO_ENTRY)
    {
        hit++;
        hitlist[l]++;
    }
    else
    {
        miss++;
        misslist[l]++;
    }

    return temp;

#else

    return (&TABLESUB(tablepile[l],x,y));

#endif
}
