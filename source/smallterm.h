// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported globals
extern symmBlock blockList;

// Exported functions
symmBlock findBlock(int n, int* degs);
symmBlock symmCreate(int n, int* degs, char** varnames);
void prSymmBlock(symmBlock symm);
smallmon btos(symmBlock glo, int* exp);
void stob1(symmBlock glo, smallmon val, int* exp);
smallmon madd1(symmBlock glo, smallmon val, int* exp);
void stob2(symmBlock glo, smallmon val, int* exp);
smallmon madd2(symmBlock glo, smallmon val, int* exp);
