// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Global variable for maximum terms per line
extern int maxterms;

// Function declarations
void fd_wr(FILE* fil, field a, boolean p_one);
void tm_wr(FILE* fil, term t);
void p_wr(FILE* fil, poly f, int comp);
void mat_wr(FILE* fil, gmatrix M);
void prmat_cmd(int argc, char* argv[]);
