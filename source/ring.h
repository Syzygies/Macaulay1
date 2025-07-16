// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported globals
extern int numvars;        // number of variables in current ring
extern int nblocks;        // number of variable blocks in current ring
extern symmBlock* blocks;  // array(0..nblocks-1) of info for each block
extern int compLoc;        // = -1 if comp doesn't exist in terms, else is location
extern term zerodegs;
extern char** varnames;  // names of vars of current ring, 0..numvars-1
extern dlist rgDegs;     // degrees of vars, 1..numvars
extern char* pStash;     // polynomial stash for this ring

// Exported functions
void init_rings(void);
int nvars(ring R);
int getRingVar(ring R, char* s);
char*varName(ring R, int i);
int varWeight(ring R, int i);
void rgInstall(ring R);
void rgKill(ring R);
ring rgCreate(int charac, char** varnames, dlist* degs, dlist* monorder, dlist* wtfcns);
void getMonOrder(dlist* monorder, int numvars);
void getWtFcn(int n, dlist* wtfcns, dlist* degs, char** vars);
ring rgScan(void);
void rgDisplay(FILE* fil, ring R);
void rgPut(FILE* fil, ring R);
ring rgSum(ring R1, ring R2);
ring rgFromDegrees(dlist* degs, char** vars);
