// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported functions

// Standard basis utilities - used by other modules
void stdWarning(gmatrix M);
void stdFirst(gmatrix M, modgen* mg, int isstd);
poly stdNext(modgen* mg);

// Internal utility - exposed for potential use
int stdNum(gmatrix M);

// Command handlers
void elim_cmd(int argc, const char* argv[]);
void keep_cmd(int argc, const char* argv[]);
void in_cmd(int argc, const char* argv[]);
void incoef_cmd(int argc, const char* argv[]);
void inpart_cmd(int argc, const char* argv[]);
void stdpart_cmd(int argc, const char* argv[]);
void stdmin_cmd(int argc, const char* argv[]);
void sat_cmd(int argc, const char* argv[]);
void forcestd_cmd(int argc, const char* argv[]);

// Internal functions - not typically called from other modules
gmatrix gm_elim(gmatrix M, int n, boolean doelim);
gmatrix gm_in(gmatrix M, int n);
void in_pprint(FILE* fil, term t, int comp);
void put_in(FILE* fil, gmatrix M);
boolean varLoHi(const char* s, expterm exp);
boolean getVarList(int argc, const char* argv[], expterm exp);
boolean exp_eqcoef(expterm exp, expterm exp2, expterm vars);
poly p_incoef(poly f, expterm vars);
gmatrix gm_incoef(gmatrix M, expterm vars);
void exp_inpart(expterm exp, expterm vars);
poly p_inpart(poly f, expterm vars);
gmatrix gm_stdpart(gmatrix M, expterm vars, boolean initialOnly);
void ins_stdmin(gmatrix result, gmatrix M, expterm nexp, int thiscomp, expterm vars);
gmatrix gm_stdmin(gmatrix M, expterm vars);
int p_maxdiv(poly f, int n);
poly p_vardiv(poly f, int n, int maxpower);
gmatrix gm_vardiv(gmatrix M, int n, int maxpower);
gmatrix gm_std(gmatrix M, int isstd);
void put_std(FILE* fil, gmatrix M, int isstd);
gmatrix gm_force(gmatrix M, gmatrix Mchange);
