// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "shared.h"
#include "shell.h"
#include "cmdnames.h"
#include "monitor.h"
#include "input.h"
#include "set.h"
#include "mem.h"
#include "stash.h"
#include "array.h"
#include "ring.h"
#include "generic.h"
#include "vars.h"
#include "poly.h"
#include "standard.h"
#include "dot_fns.h"
#include "monoms.h"
#include "mac.h"
#include "printf.h"
#include "boxcmds.h"
#include "version.h"

// Declare functions from unistd.h to avoid conflict with vars.h select()
extern int isatty(int);
extern int fileno(FILE *);

int level;
jmp_buf envbuf;
FILE *outfile; // either stdout, or redirected file

const char *greet = "\n"
                    "                    Macaulay\n"
                    "    A computer algebra system for algebraic geometry\n"
                    "\n"
                    "    Copyright 2025 Dave Bayer and Mike Stillman\n"
                    "    Licensed under CC0 1.0 Universal (Public Domain)\n"
                    "\n"
                    "    Dave Bayer                   Mike Stillman\n"
                    "    Department of Mathematics    Department of Mathematics\n"
                    "    Barnard College              Cornell University\n"
                    "\n";

// issubseq: returns -1 if s < t, 1 if s > t, and
// 0 if s is a substring of t.
int issubseq(const char *s, const char *t)
{
    while ((*s != '\0') && (*t != '\0'))
    {
        if (*s > *t)
            return 1;
        if (*s < *t)
            return -1;
        s++;
        t++;
    }
    if (*s == '\0')
        return 0;
    else
        return 1;
}

#ifdef COMMENT // old cmd_lookup()
int cmd_lookup(char *s, int n)
{
    int i, ind, len, cnt;
    cmd_rec *r;
    char *t;

    len = strlen(s);
    cnt = 0;
    t = NULL; // to avoid spurious warning
    for (i = 0, r = cmd_list; *r->name != '\0'; ++i, ++r)
    {
        if (strncmp(r->name, s, len) == 0)
        {
            ind = i;
            if (len == strlen(r->name))
            {
                // exact match
                cnt = 1;
                break;
            }
            else if (len >= n)
            {
                // good abbrev
                ++cnt;
            }
            if (cnt > 1)
                print("%s\n", t); // prev match
            t = r->name;
        }
    }
    if (cnt > 1)
    {
        print("%s\n\n", t); // last match
        prerror("; ambiguous command: %s\n", s);
    }
    if (cnt == 1)
        return ind;
    else if (cnt > 1)
        return -2;
    else // cnt == 0
        return -1;
}

#endif // COMMENT

int match_words(char *s, const char *t)
{
    // 0 if no match
    // 1 if exact match
    // 2 if s is prefix of t
    // 3 if s is contraction of t (omit any interior letters)
    char *p;
    const char *q;

    if (*s != *t)
        return 0;
    for (p = s + 1, q = t + 1; *p != '\0' && *p == *q; ++p, ++q)
        ;
    if (*p == '\0')
    {
        if (*q == '\0')
            return 1;
        else
            return 2;
    }
    for (; *p != '\0' && *q != '\0'; ++p, ++q)
    {
        while (*p != *q && *q != '\0')
            ++q;
        if (*q == '\0')
            break;
    }
    if (*p == '\0' && *q == '\0')
        return 3;
    else
        return 0;
}

int cmd_lookup(char *s, int n)
{
    int i, ind, ind3, cnt1, cnt2, cnt3, m;
    size_t len;
    cmd_rec *r;

    len = strlen(s);
    cnt1 = cnt2 = cnt3 = 0;
    ind = ind3 = 0; // initialize to avoid warnings
    for (i = 0, r = cmd_list; *r->name != '\0'; ++i, ++r)
    {
        if ((m = match_words(s, r->name)) > 0)
        {
            if (m == 1)
            {
                // exact match
                ind = i;
                cnt1 = 1;
                break;
            }
            else if (m > 0 && len >= (size_t)n)
            {
                if (m == 2)
                {
                    // prefix
                    ind = i;
                    ++cnt2;
                }
                else
                {
                    // contraction
                    ind3 = i;
                    ++cnt3;
                }
            }
        }
    }

    if (cnt1 == 1 || cnt2 == 1 || cnt2 + cnt3 == 1)
    {
        if (cnt1 + cnt2 == 0)
            ind = ind3;
        if (cnt3)
            print("%s\n", cmd_list[ind].name);
        return ind;
    }
    else if (cnt2 > 1 || cnt3 > 1)
    {
        for (i = 0, r = cmd_list; *r->name != '\0'; ++i, ++r)
            if (match_words(s, r->name) > 1)
                print("%s\n", r->name);
        prerror("\n; ambiguous command: %s\n", s);
        return -2;
    }
    else
        return -1;
}

void do_init(void)
{
    i_set();
    init_mem();
    init_path();
    i_stashes();
    array_init();
    init_rings();
    init_vars();
    init_polys();
    init_division();
    init_dot();
    // mn_first(); */ /* TODO: Find or implement this function
}

void reset_cmd(void)
{
    i_set();
    reset_mem();
    init_path();
    i_stashes();
    array_init();
    init_rings();
    init_vars();
    init_polys();
    init_division();
    init_dot();
    // mn_first(); */ /* TODO: Find or implement this function
    to_shell();
}

boolean doInitFile(void)
{
    FILE *fil;

    fil = topen("Macaulay.init", "r");
    if (fil == nullptr)
        return FALSE;
    fclose(fil);
    open_tour("Macaulay.init");
    return TRUE;
}

int main(void)
{
    mem_init_once_only();
    i_sniff();
    spec_init();
    do_init();

    // Auto-enable echo when stdin is not a terminal (pipe/file)
    if (!isatty(fileno(stdin)))
    {
        echo = 1;
        // Force stdout to be unbuffered when stdin is piped
        setbuf(stdout, NULL);
    }

    if (setjmp(envbuf))
        prerror("; back to the top\n");

    level = -1;
    change_level();
    shell();
    exit(0);  // exitMacaulay();
    return 0; // never reached, but needed for C23
}

void exit_cmd(int argc, char *argv[])
{
    (void)argv; // unused parameter
    if (argc > 1)
        exit(0); // exitMacaulay();
    else
    {
        level--;
        change_level();
    }
}

void continue_cmd(void)
{
    // need level = 0 means at "top" level

    if (level > 0)
    {
        level--;
        change_level();
    }
}

void break_cmd(void)
{
    break_voice();
}

void shout_cmd(int argc, char *argv[])
{
    int i;
    int oldlevel;
    char *p;

    if (argc == 1)
    {
        printnew("shout command arg1 arg2 ...\n");
        return;
    }
    oldlevel = prlevel;
    prlevel = 0;

    for (p = argv[1]; *p != '\0'; ++p)
        if (*p == '-')
            *p = '_';
    i = cmd_lookup(argv[1], 1);
    if (i >= 0)
        (*cmd_list[i].proc)(argc - 1, argv + 1);
    fflush(outfile);
    prlevel = oldlevel;
}

void vers_cmd(void)
{
    fnewline(outfile);
    fprint(outfile, "version %s, created %s\n", Version, Date);
}

void to_shell(void)
{
    longjmp(envbuf, -1);
}

void shell(void)
{
    char *outname, *p;
    int argc;
    char **argv;
    int i, thislevel;

    flushnum = 0; // used to control verbose output
    if (level == -1)
    {
        level = 0;
        change_level();
        if (!doInitFile())
            print("%s", greet);
        print("Macaulay version %s, created %s\n", Version, Date);
        fflush(stdout);
    }
    else
    {
        level++;
        if (!change_level())
        {
            prerror("; Sorry, too many levels deep. Call first next time!\n");
            level--;
            return;
        }
    }
    thislevel = level; // only calling exit_cmd changes level
    while (thislevel == level)
    {
        get_cmd_line(&argc, &argv, &outname);
        if (outname == nullptr)
            outfile = stdout;
        else
        {
            outfile = topen(outname, "w");
            if (outfile == nullptr)
            {
                prerror("; output file can't be opened\n");
                outfile = stdout;
            }
        }
        flushnum = 0; // reset verbose output
        if (in_dot_cmd_mode)
        {
            rmmouse(); // remove any pending interrupts
            markTime();
            dot_cmd(argc, argv);
        }
        else
        {
            for (p = argv[0]; *p != '\0'; ++p)
                if (*p == '-')
                    *p = '_';
            i = cmd_lookup(argv[0], 1);
            rmmouse(); // remove any pending interrupts
            markTime();
            if (i >= 0)
                (*cmd_list[i].proc)(argc, argv);
            else if (i == -1)
                prerror("; command not found: %s\n", argv[0]);
        }
        prTime("elapsed time : ");

        if (outfile != stdout)
            fclose(outfile);
        garb_collect();
    }
}

void ech_cmd(int argc, char *argv[])
{
    int i;

    for (i = 1; i < argc; ++i)
        fprint(outfile, "%s ", argv[i]);
    fprint(outfile, "\n");
}

void clac_cmd(int argc, char *argv[])
{
    newline();
    print("calc\n");
    calc_cmd(argc, argv);
}
