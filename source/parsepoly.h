// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Forward declaration
typedef struct stack_struct stack;

// Exported functions

// Main parsing functions
poly parsePoly(char** str, int comp);
poly readPoly(char** str, int comp);
poly getPoly(char* s, int comp);
poly rdPoly(int whichComp);
poly rdPolyStr(int argc, char** argv, int whichComp);

// Internal parsing functions (exposed for parse module)
void pGetNext(char** str);
int pCheckError(int lasttok, int thistok, char* end);
void pDoAction(int op, char* parseStr);
poly eatPoly(char** str, int comp);

// Exported globals
extern int pthistok;     // one of LXEOI, ..., LXUMINUS
extern poly pthisval;    // if pthistok = LXPOLY, this is the poly that it is
extern int ithisval;     // integer value, if pthistok = LXINT: ONLY after '^'
extern int pComponent;   // row number of resulting polynomial
extern stack pOpStack;   // operator stack
extern stack pValStack;  // value stack
extern int pParenlevel;  // should never be negative. End value should be 0
