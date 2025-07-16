// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <setjmp.h>

// Global variables
extern int level;
extern jmp_buf envbuf;
extern FILE *outfile;

// Main functions
int main(void);
void shell(void);
void to_shell(void);

// Initialization functions
void do_init(void);
void reset_cmd(void);
boolean doInitFile(void);

// Command lookup functions
int issubseq(const char *s, const char *t);
int match_words(char *s, const char *t);
int cmd_lookup(char *s, int n);

// Command functions
void exit_cmd(int argc, char *argv[]);
void continue_cmd(void);
void break_cmd(void);
void shout_cmd(int argc, char *argv[]);
void vers_cmd(void);
void ech_cmd(int argc, char *argv[]);
void clac_cmd(int argc, char *argv[]);
