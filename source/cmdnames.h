// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Command function pointer type - handles both void and int returns
typedef void (*cmd_func)(int argc, char *argv[]);

// Command record type - should eventually be in shared.h
typedef struct cmd_rec
{
    const char *name; // Command name
    cmd_func proc;    // Command function pointer
    int args;         // Preferred number of arguments
} cmd_rec;

// Exported globals
extern cmd_rec cmd_list[];
