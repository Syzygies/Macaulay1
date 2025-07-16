// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported global variables
extern variable* editvar;
extern gmatrix editmat;
extern boolean ispfaff;

// Exported functions
int edit_cmd(int argc, char* argv[]);
int pr_cmd(int argc, char* argv[]);
int pc_cmd(int argc, char* argv[]);
int mr_cmd(int argc, char* argv[]);
int mc_cmd(int argc, char* argv[]);
int dr_cmd(int argc, char* argv[]);
int dc_cmd(int argc, char* argv[]);
int ar_cmd(int argc, char* argv[]);
int ac_cmd(int argc, char* argv[]);
int ce_cmd(int argc, char* argv[]);

boolean ok_edit(int row_or_col);
boolean rcheck(int n);
boolean ccheck(int n);

void gmR_permute(gmatrix g, int r1, int r2);
void gmR_addto(gmatrix g, int r1, poly f, int r2);
void gmR_mult(gmatrix g, int i, poly f);

void gmC_permute(gmatrix g, int c1, int c2);
void gmC_addto(gmatrix g, int c1, poly f, int c2);
void gmC_mult(gmatrix g, int i, poly f);

void dl_permute(dlist* dl, int i1, int i2);
void p_perm(poly* f, int i, int j);
void p_addto(poly* pol, int i1, poly f, int i2);
void p_multcomp(poly* pol, int i, poly f);
