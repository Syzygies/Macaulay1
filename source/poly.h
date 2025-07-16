// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported functions
poly p_listhead(void);
void init_polys(void);
poly p_monom(field a);
poly p_initial(poly f);
void p_kill(poly* f);
poly e_sub_i(int i);
poly p_intpoly(int n, int comp);
poly p_xjei(int j, int i);
poly p_copy(poly f);
poly p_in(poly f, int n);
void p_add(poly* f1, poly* g1);
void p_negate(poly* g);
void p_sub(poly* f, poly* g);
poly p1_mult(field a, term t, poly f);
poly p_mult(poly f, poly g);
void special_sub(poly* f, field a, term t, poly h);
poly mult_sub(term s1, poly g1, term s2, poly g2);
void make1_monic(poly* f);
void make2_monic(poly* f, poly* frep);

// Exported globals
extern poly addnode;
