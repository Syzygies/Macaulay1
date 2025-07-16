// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Initialization and cleanup
void mo_init(gmatrix M);
void mo_kill(gmatrix M);

// Insertion operations
void mo_insert(gmatrix M, term t, char* i);
void mo_rg_insert(term t, char* i);

// Query operations
boolean mo_iscomplete(gmatrix M, int deg);
int tdegree(gmatrix M, term t);

// Iteration support
void mo_reset(gmatrix M, char** i, char** j);
void mo_rgreset(char** i, char** j, int jmax);
boolean mo_next_pair(gmatrix M, int deg, char** i, char** j);
boolean mo_rg_pair(gmatrix M, int deg, char** i, char** j);

// Division operations
boolean mo_find_div(gmatrix M, term t, char** i, term s);
boolean mo_rg_div(plist* pl, term t, char** i, term s);
