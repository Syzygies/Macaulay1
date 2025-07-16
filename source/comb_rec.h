// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Combination record structure
struct comb_rec
{
    int *t;
    int length;
    int sign;
};

// Exported functions
void init_combs(int maxn);
void free_combs(void);
void init_comb_recs(int maxl);
void get_comb_rec(struct comb_rec *c, int l);
void init_comb_rec(struct comb_rec *c);
void display_comb_rec(struct comb_rec *c);
int comb_num(int *ary, int length, int n);
int next_comb(int *t, int *sign, int length, int range);
void free_comb_rec(struct comb_rec *c);
void free_comb_recs(void);

// Exported globals
extern int **comb_table;
extern int table_length;
extern char *recstash;
