/* comb_rec.h   -   header file for comb_rec.c */

struct comb_rec
{
    int *t;
    int length;
    int sign;
};

extern int **comb_table;

#define combs(n,r)  (((r)<0) || ((r)>(n)))?printf("ACK!"):comb_table[(n)][(r)]

extern     init_combs();
extern     free_combs();
extern int comb_num();
extern     next_comb();

