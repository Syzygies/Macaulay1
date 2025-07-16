// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Constants
constexpr int NDET = 15;

// Type definitions
typedef poly detmat[NDET][NDET];

// Array manipulation functions
boolean pincr_array(int p, int n, int t[], int* a, int* sign);
void pinit_array(int p, int n, int t[], int* a, int* sign);
void parr_pr(int p, int t[], int a);

// Wedge product functions
void dl_wedge(int p, dlist* dl, dlist* result);
void pl_wedge(int p, int nrows, int ncols, plist* pl, plist* result);

// Determinant computation functions
void todetmat(int p, plist* pl, int rows[], int cols[], detmat MAT);
void get_pivot(detmat MAT, int p, int* colpivot, poly* pivot);
poly detmult(poly f1, poly g1, poly f2, poly g2, poly divpoly);
void gauss(detmat MAT, int r, int p, int colpivot, poly pivot, poly lastpivot);
poly det(detmat MAT, int size, int sign, int comp);

// Pfaffian functions
poly pfaff4(gmatrix g, int t[]);
