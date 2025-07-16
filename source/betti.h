// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Betti number computation functions
void resLoHi(int v, int* ResLength, int* lo, int* hi, int betti[]);
void resDeg(int v, int deg, int betti[]);
void betti(FILE* fil, variable* p);

// Module size analysis functions
void gmSize(gmatrix M);
void nSPairs(gmatrix M);
void spairs_flush(gmatrix M);

// Command functions
void betti_cmd(int argc, char* argv[]);
void spairs_cmd(int argc, char* argv[]);
void size_cmd(int argc, char* argv[]);

// Helper functions
int dl_ndeg(dlist* dl, int deg);
void pl_lohi(gmatrix M, plist* pl, int* lo, int* hi);
void std_lohi(gmatrix M, int* lo, int* hi);
void getNumMonoms(gmatrix M, plist* pl, int deg, long int* ngens, long int* nmonoms);
void getNstdMonoms(gmatrix M, int deg, long int* ngens, long int* nmonoms);
long int nMonoms(poly f);
