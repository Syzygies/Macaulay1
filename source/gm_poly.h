// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported functions

// Component access functions
int get_comp(poly f);
void set_comp(poly f, int n);

// Matrix/column operations
void getcol(plist *a, poly f, int r);

// Polynomial arithmetic operations
poly p11_mult(poly f, bigterm big, poly g, int comp);
poly dot1(poly f, poly g, int comp);
poly p_dot(poly f, poly g, int comp);

// Component shifting operations
poly compshift(poly f, int n);
poly tensorShift(poly f, int m, int j);

// Extraction operations
poly extr1(poly f, int i, int j);
poly extract(poly f, dlist *a);

// Matrix entry modification
void p_chEntry(poly *f, int i, poly g);

// Division operations
void p_divmod(poly *ff, poly g, poly *hdiv, poly *hmod);
poly p_div(poly *f, poly g);
poly p_mod(poly *f, poly g);

// Differentiation and contraction
poly p_diff(poly f, int var, int comp);
poly p_contract(poly x, poly f, boolean usecoef, int comp);
