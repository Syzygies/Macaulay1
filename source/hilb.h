// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Type definitions
typedef void (*htell_func)(arrow head, int plus);
typedef array mn_table;
typedef array mn_syzes;

// Exported global variables
extern long genfun[GFSIZE];
extern char* headStash;
extern char* monStash;
extern char* parenStash;
extern char* pairStash;

// Initialization functions
void i_genfun(void);
void mn_first(void);
void mn_ring(ring R);
void mn_unring(ring R);
void mn_init(gmatrix M);
void mn_kill(gmatrix M);

// Monomial ideal operations
arrow monnewhead(int en);
arrow monadv(arrow head, int chase);
arrow monbreed(arrow head, int stop);
int monadjoin(arrow head, expterm nexp, htell_func htell_cb);
int monbagadjoin(arrow head, expterm nexp, char* bag);
void monreset(arrow head, int backward);
int monsearch(arrow head, int backward, expterm nexp);
void moninsert(arrow head, expterm nexp);
void mondelete(arrow head);
void monrefund(arrow h);
void monRemNode(arrow p);

// Hilbert function computation
void mobfirst(arrow head, htell_func htell_cb);
void mobrecurse(arrow head, htell_func htell_cb, int plus);
void htell(arrow head, int plus);

// S-pair and divisibility operations
void mn_lcm(arrow head, arrow* newh, expterm m);
void mn_insert(gmatrix M, term m1, char* bag);
void mn_stdinsert(gmatrix M, term m1, char* bag);
boolean mn_rdiv(term m1, char** b, term m2);
boolean mn_find_div(mn_table* a, term m1, char** b, term m2);
boolean mn_iscomplete(gmatrix M);
int mn_next_pair(gmatrix M, int deg, char** i1, char** i2);

// Utility functions
void putmonlist(arrow head);
void mn_print(gmatrix M);
void inlinkduk(arrow p, arrow q);
void unlinkduk(arrow p);
