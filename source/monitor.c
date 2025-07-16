// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <stdarg.h> // va_list, va_start, va_end
#include <stdio.h>  // FILE, stdout, stderr, fopen, fclose, fprintf, vsnprintf
#include <stdlib.h> // exit()
#include <string.h> // strlen()
#include "shared.h"
#include "monitor.h"
#include "shell.h"  // get_line(), outfile
#include "mac.h"    // get_ticks()
#include "printf.h" // doPrint()
#include "input.h"  // topen()
#include "set.h"    // linesize, iodelay, prlevel, prcomment

FILE *monfile;
int am_monitoring = 0;

int flushnum; /* everytime prflush is called, this is incremented by 1.
                 Once this number is 50, a newline is displayed.  Also,
                 This number is reset to 0 before every command, in shell.c*/

static void doPr(const char *fmt, va_list *ap, FILE *fil)
{
    if ((fil == stdout) && (prlevel > 0))
        return;
    doPrint(fmt, ap, fil); // will print to monitor file if appropriate
}

void fprint(FILE *fil, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    doPr(fmt, &ap, fil);
    va_end(ap);
}

void print(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    doPr(fmt, &ap, stdout);
    va_end(ap);
}

void fprintnew(FILE *fil, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fnewline(fil);
    doPr(fmt, &ap, fil);
    va_end(ap);
}

void printnew(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    newline();
    doPr(fmt, &ap, stdout);
    va_end(ap);
}

void prinput(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    print("! ");
    doPr(fmt, &ap, stdout);
    print(" ? ");
    va_end(ap);
}

void prinput_error(const char *fmt, ...)
{
    va_list ap;
    int old_prlevel;

    va_start(ap, fmt);

    old_prlevel = prlevel;
    prlevel = 0;
    print("! ");
    doPr(fmt, &ap, stdout);
    print(" ? ");
    prlevel = old_prlevel;
    va_end(ap);
}

int cmd_error;

void prerror(const char *fmt, ...)
{
    va_list ap;
    int old_prlevel;

    va_start(ap, fmt);

    old_prlevel = prlevel;
    prlevel = 0;
    doPr(fmt, &ap, stdout);
    prlevel = old_prlevel;
    cmd_error = 1;
    va_end(ap);
}

void intflush(const char *fmt, int num)
{
    // call like printf, with format, 1 int argument
    // prepares so prflush can correctly count length
    char buf[128];

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
    sprintf(buf, fmt, num);
#pragma GCC diagnostic pop
    prflush("%s", buf);
} // new 24feb89 DB

// linesize and iodelay are defined in set.c and declared extern in monitor_.h

static long last_fflush = 0;

void prflush(const char *fmt, ...)
{
    va_list ap;
    long ticks;

    // sets flushnum correctly when no args (fmt only)
    int len;

    va_start(ap, fmt);

    // Create a temporary buffer to calculate actual length
    char temp[1024];
    vsnprintf(temp, sizeof(temp), fmt, ap);
    len = (int)strlen(temp);

    flushnum += len;
    if (flushnum > linesize)
    {
        print("\n");
        flushnum = len;
        newline(); // leave last!
    }

    // Reset va_list and print for real
    va_end(ap);
    va_start(ap, fmt);
    doPr(fmt, &ap, stdout);

    if ((ticks = get_ticks()) > last_fflush + iodelay)
    {
        last_fflush = ticks;
        fflush(stdout);
    }
    va_end(ap);
} // mod 24feb89 DB, 16aug93 DB

void debugpr(const char *fmt, ...)
{
    va_list ap;
    int argc;
    char **argv;

    va_start(ap, fmt);
    doPr(fmt, &ap, stdout);
    print(" = ");
    get_line(&argc, &argv);
    if (argc > 0)
        exit(0);
    va_end(ap);
}

void newline(void)
{
    if (prcomment > 0)
    {
        print("; ");
        flushnum += 2;
    }
} // mod 24feb89 DB

void fnewline(FILE *fil)
{
    if (prcomment > 0)
    {
        fprint(fil, "; ");
        flushnum += 2;
    }
}

void monprint(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    if (am_monitoring == 1)
        doPrint(fmt, &ap, monfile);
    va_end(ap);
}

void mon_cmd(int argc, char *argv[])
{
    if (argc != 2)
    {
        print("monitor <file>\n");
        return;
    }
    if (am_monitoring == 1)
    {
        print("monitoring is already being done\n");
        return;
    }
    monfile = topen(argv[1], "w");
    if (monfile == NULL)
    {
        print("can't open %s\n", argv[1]);
        return;
    }
    am_monitoring = 1;
}

void end_monitor(void)
{
    int val;

    am_monitoring = 0;
    val = fclose(monfile);
    if (val != 0)
        print("error writing monitor file\n");
}
