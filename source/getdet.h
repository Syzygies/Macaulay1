// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Variable type constants
// MAINVAR comes from shared.h
constexpr int VMODULE = 4;

// Global variable
extern int Temp;

// Inline functions to replace macros
static inline void swapints(int* x, int* y)
{
    int temp = *x;
    *x = *y;
    *y = temp;
}

static inline int odd(int x)
{
    return x % 2;
}

static inline int even(int x)
{
    return !(x % 2);
}

// GET_MOD inline function - defined in getdet.c after includes
bool get_mod(gmatrix* g, int argc, char** argv, int i);

// NO_ENTRY constant - defined in tables.c
extern poly NO_ENTRY;

// Table access - defined in tables.c
extern poly matrixsub(int x, int y);

// Determinant calculation functions
poly calc_det(gmatrix M, int* r, int* c, int p);
gmatrix determinants(gmatrix M, int s);

// Command interface
void determinants_cmd(int argc, char* argv[]);

#ifdef OPCOUNT
extern unsigned multcount, addcount;
#endif
