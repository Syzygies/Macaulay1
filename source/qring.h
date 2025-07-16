// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported functions
ring qrgMake(gmatrix M);
ring qrgSum(variable* R1, variable* R2, variable* S);
gmatrix qrgPresent(ring R, mn_standard rstd);
boolean isQRing(variable* p);
void qrgInstall(arrow r);
void qrgKill(arrow r, mn_standard rstd);
void qrgDisplay(FILE* fil, ring R);
void qrgReduce(poly* f);
void qring_cmd(int argc, char* argv[]);
void presentring_cmd(int argc, char* argv[]);
void qrgInsert(arrow head, mn_standard* rstd, poly f);
void qrgAddIdeal(arrow head, mn_standard* rstd, gmatrix M);
void qrgAddMonoms(arrow head, mn_standard* rstd, arrow rideal);
void qrgToRing(arrow head, mn_standard* rstd, arrow rideal, variable* R, variable* S);

// Exported globals
extern arrow Rideal;
