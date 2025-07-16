// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported functions

// Helper functions
boolean hom_check(gmatrix M, char* name);
void doAutocalc(char* s);

// Resolution creation functions
variable*res(char* name, gmatrix M, int depth);
variable*nres(char* name, variable* p, int depth);
variable*std(char* name, variable* p);
variable*lift_std(char* name, gmatrix M);
variable*syz(char* name, gmatrix M, int ncomps);
variable*modulo(char* name, gmatrix M, gmatrix N);
variable*istd(char* name, gmatrix M, int ncomps);

// Command functions
void res_cmd(int argc, char* argv[]);
void nres_cmd(int argc, char* argv[]);
void std_cmd(int argc, char* argv[]);
void liftstd_cmd(int argc, char* argv[]);
void syz_cmd(int argc, char* argv[]);
void modulo_cmd(int argc, char* argv[]);
void istd_cmd(int argc, char* argv[]);
