/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include <stdio.h>

/* 2022 convert.rb ++++++++++++++++ */
static void doPr (char *fmt, va_list *ap, FILE *fil);
void fprint (FILE * fil, char * fmt, ...);
void print (char * fmt, ...);
void fprintnew (FILE * fil, char * fmt, ...);
void printnew (char * fmt, ...);
void prinput (char * fmt, ...);
void prinput_error (char * fmt, ...);
void prerror (char * fmt, ...);
void intflush (char *fmt, int num);
void prflush (char * fmt, ...);
void debugpr (char * fmt, ...);
void newline ();
void fnewline (FILE *fil);
void monprint (char * fmt, ...);
void mon_cmd (int argc, char *argv[]);
void end_monitor ();
/* 2022 convert.rb ---------------- */

#include <stdarg.h>
#include <string.h>
#define MAXARGS 100

FILE *monfile;
int am_monitoring = 0;
extern int prlevel;
extern int prcomment;

int flushnum;  /* everytime prflush is called, this is incremented by 1.
           Once this number is 50, a newline is displayed.  Also,
           This number is reset to 0 before every command, in shell.c*/

static void doPr (char *fmt, va_list *ap, FILE *fil)
{
    if ((fil == stdout) && (prlevel > 0)) return;
    doPrint(fmt, ap, fil);  /* will print to monitor file if appropriate */
}

void fprint (FILE * fil, char * fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    doPr(fmt, &ap, fil);
    va_end(ap);
}

void print (char * fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    doPr(fmt, &ap, stdout);
    va_end(ap);
}

void fprintnew (FILE * fil, char * fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fnewline(fil);
    doPr(fmt, &ap, fil);
    va_end(ap);
}

void printnew (char * fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    newline();
    doPr(fmt, &ap, stdout);
    va_end(ap);
}

void prinput (char * fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    print("! ");
    doPr(fmt, &ap, stdout);
    print(" ? ");
    va_end(ap);
}

void prinput_error (char * fmt, ...)
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

void prerror (char * fmt, ...)
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

void intflush (char *fmt, int num)
{
    /* call like printf, with format, 1 int argument */
    /* prepares so prflush can correctly count length */
    char buf[128];

    sprintf(buf, fmt, num);
    prflush(buf);
} /* new 24feb89 DB */

extern int linesize; /* new 24feb89 DB */

static long last_fflush = 0;

void prflush (char * fmt, ...)
{
    va_list ap;
    long ticks, get_ticks();
    extern int iodelay;

    /* sets flushnum correctly when no args (fmt only) */
    int len;

    va_start(ap, fmt);
    len = strlen(fmt);
    flushnum += len;
    if (flushnum > linesize) {
        print("\n");
        flushnum = len;
        newline(); /* leave last! */
    }
    doPr(fmt, &ap, stdout);

    if ((ticks=get_ticks()) > last_fflush + iodelay) {
        last_fflush = ticks;
        fflush(stdout);
    }
    va_end(ap);
} /* mod 24feb89 DB, 16aug93 DB */

void debugpr (char * fmt, ...)
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

extern int flushnum;

void newline ()
{
    if (prcomment > 0) {
        print("; ");
        flushnum += 2;
    }
} /* mod 24feb89 DB */

void fnewline (FILE *fil)
{
    if (prcomment > 0) {
        fprint(fil, "; ");
        flushnum += 2;
    }
}

void monprint (char * fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    if (am_monitoring == 1)
        doPrint(fmt, &ap, monfile);
    va_end(ap);
}

void mon_cmd (int argc, char *argv[])
{
    FILE *topen();
    if (argc != 2) {
        print("monitor <file>\n");
        return;
    }
    if (am_monitoring == 1) {
        print("monitoring is already being done\n");
        return;
    }
    monfile = topen(argv[1], "w");
    if (monfile == NULL) {
        print("can't open %s\n", argv[1]);
        return;
    }
    am_monitoring = 1;
}

void end_monitor ()
{
    int val;

    am_monitoring = 0;
    val = fclose(monfile);
    if (val != 0)
        print("error writing monitor file\n");
}

