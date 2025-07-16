// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported functions
void init_charp(int characteristic);
field normalize(long int n);
void gcd_extended(long int a, long int b, long int* u, long int* v, long int* g);
void lift(int m, int c, int* v2, int* v3);
void fd_add(field a, field b, field* c);
void fd_sub(field a, field b, field* c);
void fd_mult(field a, field b, field* c);
void fd_negate(field* a);
void fd_div(field a, field b, field* c);
void fd_recip(field* a);
field fd_copy(field a);
boolean fd_iszero(field a);

// Exported globals
extern int charac;
extern field fd_zero;
extern field fd_one;
extern int field_size;
