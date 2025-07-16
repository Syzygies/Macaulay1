// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported functions
int nrows(gmatrix g);
int ncols(gmatrix g);
void gmInsert(gmatrix g, poly f);
poly gm_elem(gmatrix g, int i, int j, int comp);
gmatrix gm_add(gmatrix f, gmatrix g);
gmatrix gm_sub(gmatrix f, gmatrix g);
gmatrix gm_transpose(gmatrix f);
gmatrix gm_mult(gmatrix f, gmatrix g);
gmatrix gm_smult(gmatrix M, poly f);
void gm_dsum(gmatrix result, gmatrix M);
gmatrix gm_iden(int n);
gmatrix gm_diag(gmatrix M);
gmatrix gm_submat(gmatrix g, dlist* drows, dlist* dcols);
gmatrix gm_wedge(gmatrix g, int p);
gmatrix gm_pfaff4(gmatrix g);
gmatrix gm_flatten(gmatrix g);
gmatrix gm_genMat(dlist* rows, dlist* cols, int firstvar);
gmatrix gm_jacobian(gmatrix g, expterm vars);
void gm_concat(gmatrix result, gmatrix g);
gmatrix gm_lift(gmatrix M, gmatrix N);
void gm_reduce(gmatrix M, gmatrix N, gmatrix* redN, gmatrix* liftN);
gmatrix gm_trace(gmatrix M);
gmatrix gm_tensor(gmatrix M, gmatrix N);
gmatrix gm_outer(gmatrix M, gmatrix N);
void powerp(gmatrix M, gmatrix result, poly f, int lastn, int pow);
gmatrix gm_power(gmatrix M, int n);
gmatrix gm_diff(gmatrix M, gmatrix N, boolean usecoef);
gmatrix gm_random(int r, int c);
gmatrix gm_compress(gmatrix M);
gmatrix gm_vars(void);
