// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported functions

// Coefficient extraction functions
boolean moneq(expterm exp, term t, expterm vars);
poly strip_poly(poly* f, expterm vars, poly* hmonom);
void gm_coef(gmatrix M, expterm vars, gmatrix* Mmonoms, gmatrix* Mcoefs);

// Homogenization functions
poly p_homog(gmatrix M, poly f, int v);
gmatrix gm_homog(gmatrix M, int v);

// Command functions
void coef_cmd(int argc, char* argv[]);
void homog_cmd(int argc, char* argv[]);
