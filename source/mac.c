// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <signal.h>  // signal, sig_atomic_t, SIGINT
#include <time.h>    // clock, clock_t, CLOCKS_PER_SEC
#include "shared.h"
#include "mac.h"
#include "monitor.h"  // print, newline
#include "shell.h"    // shell
#include "set.h"      // timer

// Default file search path
// Can be modified via environment variable MacaulayPath,
// then by the file Macaulay.path, then via the path command.
// For Unix: use current directory as default
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
char* MclyPath = (char*)".";
#pragma GCC diagnostic pop

// The default current directory can be initialized, but to what?
char* MclyCdir = NULL;

// The location of the following two files is installation dependent.
// If they can be found via MclyPath, no changes are needed below.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
char* helpFile = (char*)"Macaulay.help";
char* newFile = (char*)"Macaulay.new";
#pragma GCC diagnostic pop

// Timer start reference
long startTime;

// Interrupt handling using standard signals
static volatile sig_atomic_t intFlag = 0;

static void handle_signal(int sig)
{
    (void)sig; // unused parameter
    intFlag = 1;
}

void spec_init(void)
{
    intFlag = 0;
    signal(SIGINT, handle_signal);
}

void intr_shell(void)
{
    if (intFlag)
    {
        intFlag = 0;
        print("\n");
        shell();
        intFlag = 0; // Historical quirk: double assignment preserved
    }
}

boolean have_intr(void)
{
    if (intFlag)
    {
        rmmouse();
        return TRUE;
    }
    return FALSE;
}

void rmmouse(void)
{
    intFlag = 0;
}

// Timer implementation using standard C library
void markTime(void)
{
    startTime = (long)clock();
}

long nSeconds(void)
{
    long elapsed = (long)clock() - startTime;
    return (long)((clock_t)elapsed / CLOCKS_PER_SEC);
}

long get_ticks(void)
{
    return (long)clock();
}

// Timer display routine
void prTime(const char* str)
{
    long tot, s, m;

    if (timer <= 0)
        return;
    tot = nSeconds();
    if (tot == 0)
        return;
    s = tot % 60;
    m = tot / 60;
    newline();
    print("%s ", str);
    if (m > 1)
        print("%ld minutes and ", m);
    else if (m == 1)
        print("1 minute and ");
    if (s != 1)
        print("%ld seconds\n", s);
    else
        print("1 second\n");
}

// Text size setting - no-op for Unix
void setTextSize(int n)
{
    (void)n; // unused parameter
}

// Printer setup - historical no-op preserved
void prsetup(void)
{
}
