// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <setjmp.h> // For jmp_buf type required by parse.h
#include <string.h> // For strcmp()
#include "shared.h"
#include "set.h"
#include "cmds.h"    // For doIntCmd()
#include "monitor.h" // For newline(), print(), prerror(), printnew()
#include "parse.h"   // For getInt()
#include "shell.h"   // For issubseq()

constexpr int NSETVARS = 16;
int bailout;
int linesize;
int verbose;
int pr_charp;
int prlevel;
int timer;
int autocalc;
int autodegree;
int maxdegree;
int echo;
int autoReduce;
int nlines;
int prcomment;
int showmem;
int showpairs;
int iodelay;

const char *setNames[NSETVARS + 1];
int *setLocs[NSETVARS];
int setDefaults[NSETVARS];
const char *setWhat[NSETVARS];

void i_setvar(int n, const char *name, int *loc, int defaultVal, const char *whatami)
{
    setNames[n] = name;
    setLocs[n] = loc;
    setDefaults[n] = defaultVal;
    setWhat[n] = whatami;
    *loc = defaultVal;
}

void i_set(void)
{
    i_setvar(0, "abort", &bailout, 0,
             ">0 means exit when out of memory (increments on each failure)");
    i_setvar(1, "auto", &autoReduce, 0, ">0 means do NOT do autoreduction");
    i_setvar(2, "autocalc", &autocalc, -1,
             "=0 no autocalc, <0 calc until done, >0 calc to autodegree");
    i_setvar(3, "autodegree", &autodegree, 0, "if autocalc>0, this is highest degree to compute");
    i_setvar(4, "char0", &pr_charp, 0, ">0 means do NOT (try to) lift to rationals");
    i_setvar(5, "echo", &echo, 0, ">0 tells Macaulay to echo all input");
    i_setvar(6, "iodelay", &iodelay, 0, "delay (in system ticks) for caching verbose output");
    i_setvar(7, "linesize", &linesize, 79, "size of line for matrix display");
    i_setvar(8, "maxdegree", &maxdegree, 512,
             "maximum degree for monomials in rings created using 'ring'");
    i_setvar(9, "nlines", &nlines, 1, "number of blank lines between commands");
    i_setvar(10, "prcomment", &prcomment, 0, "if >0 start each output line with ;");
    i_setvar(11, "prlevel", &prlevel, 0, ">0 means suppress ALL output, except error messages");
    i_setvar(12, "showmem", &showmem, 1, ">0 means report memory usage at each increase");
    i_setvar(13, "showpairs", &showpairs, 0, ">0 means report S-pairs left during computations");
    i_setvar(14, "timer", &timer, 0, ">0 means display execution time");
    i_setvar(15, "verbose", &verbose, 0, ">0 means give verbose output");
    setNames[NSETVARS] = "";
} // 5/18/89 DB 6/6

static int lookup(const char *names[], const char *s)
{
    int i, val;

    for (i = 0; *(names[i]) != '\0'; i++)
    {
        val = issubseq(s, names[i]);
        if (val == 0)
            return i;
        if (val == -1)
            return -1;
    }
    return (-1);
}

void doSet(int argc, char *argv[], boolean doIncr)
{
    int i;

    if (argc == 1)
    {
        for (i = 0; i < NSETVARS; i++)
        {
            newline();
            print("%-10s %3d    %s\n", setNames[i], *(setLocs[i]), setWhat[i]);
        }
        return;
    }

    if (strcmp(argv[1], "default") == 0)
    {
        i_set();
        return;
    }

    i = lookup(setNames, argv[1]);
    if (i == -1)
    {
        prerror("; no set variable named %s\n", argv[1]);
        return;
    }

    if (argc == 2)
        *(setLocs[i]) = setDefaults[i];
    else if (argc == 3)
    {
        if (doIncr)
            *(setLocs[i]) += getInt(argv[2]);
        else
            *(setLocs[i]) = getInt(argv[2]);
    }
}

void incset_cmd(int argc, char *argv[])
{
    doSet(argc, argv, true); // true = do increment
}

void set_cmd(int argc, char *argv[])
{
    doSet(argc, argv, false); // false = do set, not increment
}

void setvalue_cmd(int argc, char *argv[])
{
    int i, val;

    if ((argc < 2) || (argc > 3))
    {
        printnew("set_value <set variable> [result integer]\n");
        return;
    }
    i = lookup(setNames, argv[1]);
    if (i == -1)
    {
        prerror("; no set variable named %s\n", argv[1]);
        return;
    }
    val = *(setLocs[i]);
    doIntCmd(argv[2], val, (argc == 3), argv[1]);
}
