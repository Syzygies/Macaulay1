// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported functions
variable *vgetmod(const char *name);
variable *vget_vmod(const char *name);
gmatrix vget_mod(const char *name);
gmatrix vget_rgmod(const char *name);
variable *vget_ring(const char *name);
ring get_ring(const char *name);
int do_ring_vars(void);
int ring_cmd(int argc, char *argv[]);
int ringsum_cmd(int argc, char *argv[]);
int setring_cmd(int argc, char *argv[]);
int pring_cmd(int argc, char *argv[]);
int ringcol_cmd(int argc, char *argv[]);
int ringrow_cmd(int argc, char *argv[]);
int mat_cmd(int argc, char *argv[]);
int sparse_cmd(int argc, char *argv[]);
int ideal_cmd(int argc, char *argv[]);
int poly_cmd(int argc, char *argv[]);
int cpx_cmd(int argc, char *argv[]);
int putstd_cmd(int argc, char *argv[]);
int putchange_cmd(int argc, char *argv[]);
int rowdegs_cmd(int argc, char *argv[]);
int coldegs_cmd(int argc, char *argv[]);
int putmat_cmd(int argc, char *argv[]);
int pres_cmd(int argc, char *argv[]);
int copy_cmd(int argc, char *argv[]);
int numinfo_cmd(int argc, char *argv[]);
int setdegs_cmd(int argc, char *argv[]);
int setcoldegs_cmd(int argc, char *argv[]);
int dshift_cmd(int argc, char *argv[]);
int int_cmd(int argc, char *argv[]);
int nrows_cmd(int argc, char *argv[]);
int ncols_cmd(int argc, char *argv[]);
int nvars_cmd(int argc, char *argv[]);
int charac_cmd(int argc, char *argv[]);
int rowdeg_cmd(int argc, char *argv[]);
int coldeg_cmd(int argc, char *argv[]);
int max_cmd(int argc, char *argv[]);
int min_cmd(int argc, char *argv[]);
int iszero_cmd(int argc, char *argv[]);
int args_cmd(int argc, char *argv[]);
int doIntCmd(char *name, int val, boolean setval, const char *mess);

// Exported globals
// None - cmds.c doesn't define any global variables
