// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported globals
extern bigterm helpexp;

// Exported functions
void prSmall(term t);
void prBig(bigterm big);
void prExp(bigterm exp);
void expToS(bigterm sexp, int component, term result);
void sToExp(term s, bigterm exp);
void sToBig(term t, bigterm big);
void bigToS(bigterm big, term t);
void hilbToS(bigterm hexp, int component, term result);
void sToHilb(term s, bigterm hexp, int* component);
void tm_pprint(FILE* fil, term t);
int tm_size(term t);
void tm_add(term s, bigterm big, term result);
int tm_compare(term s, term t);
void term_i(int i, term t);
void tm_xjei(int j, int i, term t);
void tm_copy(term s, term t);
int tm_component(term t);
void tm_setcomp(term t, int c);
boolean tm_iszero(term t);
int tm_degree(term t);
int exp_degree(expterm exp);
void joinminus(term t1, term t2, int c1, int c2, term result1, term result2);
void tm_joinminus(term t1, term t2, term result1, term result2);
void tm_rg_join(term t1, term t2, term result1, term result2);
boolean tm_divides(term s, term t, int c, term result);
void tm_diff(term s, term result, int v, int* power);
void tm_contract(term x, term s, term result, int* coefval);
boolean in_subring(term t, int i);
boolean tm_inEq(term s, term t, int i);
void getfirstblock(expterm exp);
