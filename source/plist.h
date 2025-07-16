// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// List operations
void dl_copy(dlist* dl1, dlist* dl2);
void pl_copy(plist* pl1, plist* pl2);
void pl_kill(plist* pl);
void dl_kill(dlist* dl);
void dl_lohi(dlist* dl, int* lo, int* hi);

// Module operations
gmatrix mod_init(void);
gmatrix mat_copy(gmatrix M);
void mod_kill(gmatrix M);
void std_kill(mn_standard p);

// Degree computation
int degree(gmatrix M, poly f);
