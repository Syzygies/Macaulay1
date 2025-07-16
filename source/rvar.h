// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported functions
boolean validVar(char c);
int parseVar(char **str);
int getVar(const char *s);
int getVarRel(char **varnames, int len, char *s);
char **getVars(int nvars);

// Exported globals
extern jmp_buf jmpseq;
