// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported functions

// Matrix creation functions
gmatrix mkintersect(dlist* dl);
gmatrix mkquotient(void);

// Matrix manipulation functions
void addintersect(gmatrix result, gmatrix M, dlist* dl);
void addquotient(gmatrix result, gmatrix I, gmatrix J, int n);

// Utility functions
boolean isIdeal(gmatrix g);

// Command functions
void inter_cmd(int argc, char* argv[]);
void quot_cmd(int argc, char* argv[]);
