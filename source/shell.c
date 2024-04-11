/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "mtypes.h"

// int issubseq (char *s, char *t);
// int match_words (char *s, char *t);
// int cmd_lookup (char *s, int n);
// void do_init ();
// void reset_cmd ();
// boolean doInitFile ();
// int main ();
// void exit_cmd (int argc, int argv);
// void continue_cmd ();
// void break_cmd ();
// void shout_cmd (int argc, char *argv[]);
// void vers_cmd ();
void to_shell ();
void shell ();
// void ech_cmd (int argc, char *argv[]);
// void clac_cmd (int argc, char *argv[]);

#include <setjmp.h>
extern int prlevel;
extern int flushnum;  /* used to control verbose output: reset in shell() */

int level;
jmp_buf envbuf;
FILE *outfile;     /* either stdout, or redirected file */

extern cmd_rec cmd_list[]; /* table in cmdnames.c describing all commands */

char *greet[] = {
    "                          Macaulay\n",
    "        A computer algebra system for algebraic geometry",
    "\n\n",
    "    This program, Macaulay, may be freely copied for others.  We\n",
    "request that you write us to join a mailing list of Macaulay users,\n",
    "so that we can keep you informed of updates to Macaulay.\n",
    /*
    "    Type \"help\" for a list of commands.\n",
    "    Type \"new\" for a list of new features.\n",
    */
    "\n",
    "    Dave Bayer               ",
    "    Mike Stillman\n",
    "    Department of Mathematics",
    "    Department of Mathematics\n",
    "    Barnard College          ",
    "    Cornell University\n",
    "    New York, NY 10027       ",
    "    Ithaca, NY 14853\n",
    "    (212)854-2643, 864-4235  ",
    "    (607)255-7240, 257-5320\n",
    "    dab@math.columbia.edu    ",
    "    mike@math.cornell.edu\n",
    "\n",
    ""
};

/* issubseq: returns -1 if s < t, 1 if s > t, and
 * 0 if s is a substring of t.
 */

int issubseq (char *s, char *t)
{
    while ((*s ISNT '\0') AND (*t ISNT '\0')) {
        if (*s > *t)
            return(1);
        if (*s < *t)
            return(-1);
        s++;
        t++;
    }
    if (*s IS '\0')
        return(0);
    else
        return(1);
}

int match_words (char *s, char *t)
{
    /* 0 if no match
     * 1 if exact match
     * 2 if s is prefix of t
     * 3 if s is contraction of t (omit any interior letters)
     */
    char *p, *q;

    if (*s != *t) return 0;
    for (p=s+1, q=t+1; *p!='\0' && *p==*q; ++p, ++q);
    if (*p=='\0') {
        if (*q=='\0')
            return 1;
        else return 2;
    }
    for (; *p!='\0' && *q!='\0'; ++p, ++q) {
        while (*p!=*q && *q!='\0') ++q;
        if (*q=='\0') break;
    }
    if (*p=='\0' && *q=='\0') return 3;
    else return 0;
}

int cmd_lookup (char *s, int n)
{
    int i, ind, ind3, len, cnt1, cnt2, cnt3, m;
    cmd_rec *r;
    char *t;

    len = strlen (s);
    cnt1 = cnt2 = cnt3 = 0;
    for (i=0, r=cmd_list; *r->name != '\0'; ++i, ++r) {
        if ((m = match_words (s, r->name)) > 0) {
            if (m == 1) {
                /* exact match */
                ind = i;
                cnt1 = 1;
                break;
            }
            else if (m > 0 && len >= n) {
                if (m == 2) {
                    /* prefix */
                    ind = i;
                    ++cnt2;
                }
                else {
                    /* contraction */
                    ind3 = i;
                    ++cnt3;
                }
            }
        }
    }

    if (cnt1 == 1 || cnt2 == 1 || cnt2 + cnt3 == 1) {
        if (cnt1 + cnt2 == 0)
            ind = ind3;
        if (cnt3)
            print ("%s\n", cmd_list[ind].name);
        return ind;
    }
    else if (cnt2 > 1 || cnt3 > 1) {
        for (i=0, r=cmd_list; *r->name != '\0'; ++i, ++r)
            if (match_words (s, r->name) > 1)
                print ("%s\n", r->name);
        prerror ("\n; ambiguous command: %s\n", s);
        return -2;
    }
    else
        return -1;
}

void do_init ()
{
    extern char *Version, *Date;

    i_set();
    init_mem();
    init_path();
    i_stashes();
    array_init();
    init_rings();
    init_generic();
    init_vars();
    init_polys();
    init_division();
    init_dot();
    mn_first();
}

void reset_cmd ()
{
    i_set();
    reset_mem();
    init_path();
    i_stashes();
    array_init();
    init_rings();
    init_generic();
    init_vars();
    init_polys();
    init_division();
    init_dot();
    mn_first();
    to_shell();
}

boolean doInitFile ()
{
    FILE *fil, *topen();

    fil = topen("Macaulay.init", "r");
    if (fil IS NULL) return(FALSE);
    fclose(fil);
    open_tour("Macaulay.init");
    return(TRUE);
}

int main ()
{
    mem_init_once_only();
    i_sniff();
    spec_init();
    do_init();
    if (setjmp(envbuf))
        prerror("; back to the top\n");

    level = -1;
    change_level();
    shell();
    exitMacaulay();
}

void exit_cmd (int argc, int argv)
{
#pragma unused(argv)
    if (argc > 1)
        exitMacaulay();
    else {
        level--;
        change_level();
    }
}

void continue_cmd ()
{
    /* need level = 0 means at "top" level */

    if (level > 0) {
        level--;
        change_level();
    }
}

void break_cmd ()
{
    break_voice();
}

void shout_cmd (int argc, char *argv[])
{
    int i;
    int oldlevel;
    char *p;

    if (argc IS 1) {
        printnew("shout command arg1 arg2 ...\n");
        return;
    }
    oldlevel = prlevel;
    prlevel = 0;

    for (p=argv[1]; *p!='\0'; ++p) if (*p == '-') *p = '_';
    i = cmd_lookup(argv[1], 1);
    if (i >= 0) (*cmd_list[i].proc) (argc-1, argv+1);
    fflush(outfile);
    prlevel = oldlevel;
}

void vers_cmd ()
{
    extern char *Version, *Date;

    fnewline(outfile);
    fprint(outfile, "version %s, created %s\n", Version, Date);
}

void to_shell ()
{
    longjmp(envbuf, -1);
}

void shell ()
{
    char *outname, *p;
    int argc;
    char **argv;
    int i, thislevel;
    FILE *topen();
    extern char *Version, *Date; /* BSS 4/19/93 */
    extern int in_dot_cmd_mode;

    flushnum = 0;  /* used to control verbose output */
    if (level == -1) {
        level = 0;
        change_level();
        if (NOT doInitFile())
            for (i=0; *greet[i]!='\0'; ++i)
                print(greet[i]);
        print("Macaulay version %s, created %s\n", Version, Date);
        fflush (stdout);
    } else {
        level++;
        if (!change_level()) {
            prerror("; Sorry, too many levels deep. Call first next time!\n");
            level--;
            return;
        }
    }
    thislevel = level; /* only calling exit_cmd changes level */
    while (thislevel == level) {
        get_cmd_line(&argc, &argv, &outname);
        if (outname IS NULL)
            outfile = stdout;
        else {
            outfile = topen(outname, "w");
            if (outfile IS NULL) {
                prerror("; output file can't be opened\n");
                outfile = stdout;
            }
        }
        flushnum = 0;  /* reset verbose output */
        if (in_dot_cmd_mode)
        {
            rmmouse(); /* remove any pending interrupts */
            markTime();
            dot_cmd (argc, argv);
        }
        else
        {
            for (p=argv[0]; *p!='\0'; ++p) if (*p == '-') *p = '_';
            i = cmd_lookup(argv[0], 1);
            rmmouse(); /* remove any pending interrupts */
            markTime();
            if (i >= 0) (*cmd_list[i].proc) (argc, argv);
            else if (i == -1) prerror("; command not found: %s\n", argv[0]);
        }
        prTime("elapsed time : ");

        if (outfile ISNT stdout) fclose(outfile);
        garb_collect();
    }
}

void ech_cmd (int argc, char *argv[])
{
    int i;

    for (i=1; i<argc; ++i) fprint(outfile, "%s ", argv[i]);
    fprint(outfile, "\n");
}

void clac_cmd (int argc, char *argv[])
{
    newline();
    print("calc\n");
    calc_cmd(argc, argv);
}
