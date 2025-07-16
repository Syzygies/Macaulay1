// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported functions
boolean member(int e, int p, int* t);
boolean subset(int p1, int* t1, int p2, int* t2);
void kremove(int p, int* t, int i, int* tresult);
int binom(int n, int p);
int loc(int p, int n, int* t);
int kz_getdeg(gmatrix M, int p, int* tcol);
void kz_setrowdegs(dlist* dl, gmatrix M, int n, int p);
poly kosz_row(gmatrix M, int p, int n, int* tcol);
gmatrix gm_koszul(gmatrix M, int n, int p);
void koszul_cmd(int argc, char* argv[]);
