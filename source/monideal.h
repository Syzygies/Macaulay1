// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Type definitions
typedef void (*ass_prime_func)(int, int*);
typedef struct cheek_rec
{
    struct cheek_rec* next;
    int exp[];
} cheek;

// Exported functions
arrow monnext(arrow p);
arrow monmake(gmatrix M, int comp, void (*fcn)(expterm));
arrow monhilb(gmatrix M, int comp, void (*fcn)(arrow, int));
void tm_radical(expterm nexp);
arrow monradical(gmatrix M, int comp);
void tm_mdivide(expterm nexp);
arrow mondivide(gmatrix M, int comp, int* exp);
void tm_mdiv(expterm nexp);
arrow mondiv(gmatrix M, int comp, expterm vars);
int redExp(int* t, int* exp);
void findAssPrimes(arrow p, int* exp1, int codim);
void wrAss(int* exp, int codim);
void mondisplay(arrow head);
int codim(gmatrix M);
int codim_cmd(int argc, char* argv[]);
void monprimes(gmatrix M);
int monprimes_cmd(int argc, char* argv[]);
long int mondegree(arrow head, int nvars);
void enumMons(int lastval);
long int modDegree(gmatrix M, int thisCodim);
int degree_cmd(int argc, char* argv[]);
void accumMons(int lastval);
gmatrix gm_basis(gmatrix M, expterm vars, int lo, int hi, boolean isbasis);
int basis_cmd(int argc, char* argv[]);
int truncate_cmd(int argc, char* argv[]);

// Exported globals
extern ass_prime_func gotAss;
extern int* mondivExp;
extern int topCodim;
extern char* monprimes_stash;
extern gmatrix monprimes_N;
extern cheek* cheeks;
extern expterm MDmon;
extern arrow MDhead;
extern int MDnvars;
extern long int MDdeg;
extern gmatrix DDmat;
extern int DDcodim;
extern int DDcomp;
extern long int DDdegree;
extern gmatrix MAresult;
extern arrow MAhead;
extern int* MAmon;
extern int MAcomp;
extern int MAcompDeg;
extern int MAlo, MAhi;
extern int* MAvars;
