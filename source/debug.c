/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"

// void monom_cmd (int argc, char *argv[]);
// void debug_cmd (int argc, char *argv[]);

void monom_cmd (int argc, char *argv[])
{
    gmatrix M;
    arrow head, monmake();

    if (argc != 2) {
        print("monom <matrix>\n");
        return;
    }
    GET_MOD(M, 1);
    head = monmake(M, 1);
    mondisplay(head);
    monrefund(head);
}

void debug_cmd (int argc, char *argv[])
{
#pragma unused(argv,argc)
    /*
        if (argc > 1) pr_stats();
        else i_stats();
    */
    /*monom_cmd(argc, argv);*/

}
