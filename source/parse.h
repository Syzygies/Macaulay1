// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <setjmp.h> // for jmp_buf

// Forward declaration
typedef struct stack_struct stack;

// Exported functions
int parseInt(char **str);
int getInt(const char *s);
int readInt(char **str);
boolean emptyStack(stack *st);
void initStack(stack *st);
void *pop(stack *st);
void push(stack *st, void *val);
void *tos(stack *st);
int collectInt(char c, char **str);
int eatInt(char **str);
void parseErr(char *end, const char *mess);

// Exported globals
extern int fprec[];
extern int gprec[];
extern int nargs[];
extern jmp_buf jmpparse;
extern char *beginStr;
extern int amParsing;
