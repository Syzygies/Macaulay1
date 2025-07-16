// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported globals - only exist when TABLESTAT is defined
#ifdef TABLESTAT
extern int hit, miss;
extern int hitlist[20], misslist[20];
#endif

// Exported functions
int init_tables(gmatrix M, int width, int height, int minorsize);
void free_tables(void);
poly*stored_value(int* r, int* c, int l);
poly matrixsub(int col, int row);
