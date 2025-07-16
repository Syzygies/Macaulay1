// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "debug.h"
#include "vars.h"
#include "monitor.h"
#include "cmds.h" // For vget_mod()
#include "monideal.h"
#include "hilb.h"

void monom_cmd(int argc, char* argv[])
{
    gmatrix M;
    arrow head;

    if (argc != 2)
    {
        print("monom <matrix>\n");
        return;
    }
    if (1 >= argc)
        return;
    M = vget_mod(argv[1]);
    if (M == NULL)
        return;
    head = monmake(M, 1, NULL);
    mondisplay(head);
    monrefund(head);
}

void debug_cmd(int argc, char* argv[])
{
    // Original ANSI pragma for unused parameters - keeping for historical accuracy
#ifdef ANSI
#pragma unused(argv, argc)
#endif
    // if (argc > 1) pr_stats() ;
    // else i_stats() ;
    // monom_cmd(argc, argv) ;

    // C23: Use standard attribute to suppress unused warnings
    (void)argc;
    (void)argv;
}
