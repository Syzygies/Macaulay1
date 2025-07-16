// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported functions
int get_int(const char* s);
int get_defint(const char* s, int n);
char*get_str(const char* s);
boolean get_contstr(const char* s, char** str);
boolean get_char(char c);
int int_scan(char** str);
void int_pprint(FILE* fil, int m, boolean p_one, boolean p_plus);
int int_size(int m, boolean p_one, boolean p_plus);
field fd_scan(char** str);
void fd_pprint(FILE* fil, field a, boolean p_one, boolean p_plus);
int fd_size(field a, boolean p_one, boolean p_plus);
void p_pprint(FILE* fil, poly f, int comp);
void pdeb(poly f, int comp);
int p_size(poly f, int comp);
void pl_scan(plist* pl, int numcols, int numrows);
void plSparse_scan(plist* pl, int numcols, int numrows);
void putmat(FILE* fil, gmatrix M);
int pr_size(poly f, int maxcomp);
void pl_pprint(FILE* fil, plist* pl, int numrows, int rowsize);
void dl_pprint(FILE* fil, dlist* dl);
boolean check_homog(gmatrix M);
void setdeggens(gmatrix M);
void setdegs(gmatrix M);
void setzerodegs(gmatrix M, int numrows);
gmatrix mat_scan(void);
gmatrix sparse_scan(void);
gmatrix ideal_scan(void);
gmatrix poly_scan(int argc, char* argv[]);
