// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported globals
extern poly divnode;

// Exported functions
void init_division(void);
void deb_print(poly f, int n);
poly reduce(gmatrix M, poly* f);
void division(gmatrix M, poly* f, poly* g, poly* h);
boolean calc_standard(gmatrix M, int deg, variable* B);
void send_off(gmatrix M, int deg, variable* B, poly h, poly hrep);
void calc_s_pair(gmatrix M, mn_standard i, mn_standard j, poly* h, poly* hrep);
void ins_elem(gmatrix M, int deg, poly h, poly hrep);
void auto_reduce(gmatrix M, int deg, poly h, poly hrep);
boolean occurs_in(poly f, term t, field* a);
void orig_gens(gmatrix M, int deg, int intval, variable* box);
void ins_generator(variable* box, poly f, int deg);
void div_sub(poly* f1, field a, term t, poly h1);
void auto_sub(poly* f1, field a, poly h1);
