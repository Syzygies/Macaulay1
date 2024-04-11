/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "style.h"
#undef comp

// void handle ();
// void spec_init ();
// void intr_shell ();
// boolean have_intr ();
void rmmouse ();
// void prsetup ();
// void setTextSize (int n);
// void markTime ();
// long nSeconds ();
// long get_ticks ();
// void prTime (char *str);

/* A default file search path must be provided below,
 * and can be modified via the environment variable MacaulayPath,
 * then by the file Macaulay.path, then via the path command.
 * Feel free to modify this default path for your local installation.
 * Macintosh sample: ":,A:user:Macaulay:bin:"
 * Unix sample: ".:/usr/local/math/Macaulay/bin"
 */

char *MclyPath = ".";

/* The default current directory can be initialized, but to what? */
char *MclyCdir = NULL;

/*
 * The location of the following two files is installation dependent.
 * If they can be found via MclyPath, no changes are needed below.
 */

char *helpFile = "Macaulay.help";
char *newFile  = "Macaulay.new";

extern int timer;
long startTime;

/*------------------------------------------------------
 *
 *  interrupt, timer and fontsize code for Macintosh
 *
 *------------------------------------------------------*/

#define MOUSE SIGINT

#include <signal.h>
/*(*signal())();*/
int intFlag;   /* set to 1 if an interrupt is given */

void handle ()
{
    intFlag = 1;
    signal(MOUSE, handle);
}

void spec_init ()
{
    intFlag = 0;
    signal(MOUSE, handle);
}

void intr_shell ()
{
    if (intFlag == 1) {
        intFlag = 0;
        print("\n");
        shell();
        intFlag = 0;
    }
}

boolean have_intr ()
{
    if (intFlag == 1) {
        rmmouse();
        return(TRUE);
    }
    return(FALSE);
}

void rmmouse ()
{
    intFlag = 0;
}

void prsetup ()
{}

void setTextSize (int n)
{
#pragma unused(n)
}

#define UNIX_TIMER
#define TICK 60

#include <sys/types.h>
#include <sys/times.h>

static struct tms time_buf;

void markTime ()
{
    times(&time_buf);
    startTime = time_buf.tms_utime + time_buf.tms_stime;
}

long nSeconds ()
{
    times(&time_buf);
    return((time_buf.tms_utime + time_buf.tms_stime - startTime)/TICK);
}

long get_ticks ()
{
    times(&time_buf);
    return time_buf.tms_utime + time_buf.tms_stime;
}

/*------------------------------------------------------
 *
 *  timer routine (machine indep.)
 *
 *------------------------------------------------------*/

void prTime (char *str)
{
    long tot, s, m;

    if (timer <= 0) return;
    tot = nSeconds();
    if (tot == 0) return;
    s = tot % 60;
    m = tot / 60;
    newline();
    print("%s ",str);
    if (m > 1)
        print("%ld minutes and ", m);
    else if (m == 1)
        print("1 minute and ");
    if (s != 1)
        print("%ld seconds\n", s);
    else
        print("1 second\n");
}
