// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Type definitions
typedef struct matches
{
    const char *name;
    int used;
} matches;

// Creation and initialization
void dl_newall(dlist *result, int n, int size);
void dl_new(dlist *result, int size);

// List operations
void dl_trans(dlist *result, dlist *a);
void dl_concat(dlist *result, dlist *a, dlist *b);
void dl_composite(dlist *result, dlist *a, dlist *b);
void dl_addto(dlist *result, int n, dlist *a);

// Min/max functions
int dl_max(dlist *dl);
int dl_min(dlist *dl);

// Special element handling
void dlSpecSet(matches *p);
boolean dlSpecial(char *s, int *val);

// Parsing and consuming
int dlgetelem(char *s, int *first, int *second, dlist **dl);
void dlConsume(dlist *dl, int argc, char *argv[], int len, int def);
void dlVarConsume(dlist *dl, int argc, char *argv[]);
void dlScan(dlist *dl, int len, int def);
void dlVarScan(dlist *dl, int lo, int hi);
boolean dlVarGet(dlist *dl, int lo, int hi);

// Display
int dlDisPart(FILE *fil, int elem, int repl);
void dlDisplay(FILE *fil, dlist *dl, int rowsize);

// Insertion and modification
void dlIns(dlist *dl, int i, int n);
void dl_set(dlist *dl, int i, int val);

// Exported global variable
extern matches varUse[];
