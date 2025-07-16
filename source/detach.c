// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#ifdef USEDETACH
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shared.h"
#include "detach.h"
#include "monitor.h"
#include "input.h"

constexpr int sigA = SIGFPE;
constexpr int sigB = SIGIOT;

char fname[] = "ttyfile";
int sendPID;        // used for sending signals from zombie process to orphan
int zombiePID = 0;  // used to send KILL signal to zombie at exit time

void exitMacaulay(void)
{
    if (zombiePID != 0)
        kill(zombiePID, SIGKILL);
    exit(0);
}

static FILE*filOpen(char* s) // either "r" or "w"
{
    FILE* fil;

    fil = topen(fname, s);
    if (fil == NULL)
    {
        prerror("; can't open %s\n", fname);
        return (NULL);
    }
    return (fil);
}

static void wrTTY(void)
{
    FILE* fil;

    fil = filOpen("w");
    if (fil == NULL)
        return;
    fprint(fil, "%d\n", getpid());
    fprint(fil, "%s\n", ttyname(0));
    fprint(fil, "%s\n", ttyname(1));
    fprint(fil, "%s\n", ttyname(2));
    fclose(fil);
}

static int rdPID(void)
{
    FILE* fil;
    int n;

    n = 0;
    fil = filOpen("r");
    fscanf(fil, "%d", &n);
    fclose(fil);
    return (n);
}

static void wrPID(int n)
{
    FILE* fil;

    fil = filOpen("w");
    fprint(fil, "%d", n);
    fclose(fil);
}

static int attachTTY(void) // returns pid in file
{
    FILE* fil;
    char str[100];
    int pid, n;

    fil = filOpen("r");

    n = fscanf(fil, "%d", &pid);
    if ((n == 0) || (n == EOF))
        return (0);
    fscanf(fil, "%99s", str);
    freopen(str, "r", stdin);
    fscanf(fil, "%99s", str);
    freopen(str, "w", stdout);
    fscanf(fil, "%99s", str);
    freopen(str, "w", stderr);
    fclose(fil);
    fil = filOpen("w");
    fclose(fil);
    return (pid);
}

static void orphanHandler(int sig)
{
    (void)sig; // C23: suppress unused parameter warning
    signal(sigA, SIG_IGN);
    zombiePID = attachTTY();
    if (zombiePID == 0)
        prerror("; can't attach to Macaulay\n");
    else
        prerror("; just attached...\n");
}

static void doOrphan(void)
{
    signal(sigA, orphanHandler);
    prerror("; made it into orphan...\n");
}

static void sendINT(int sig)
{
    (void)sig; // C23: suppress unused parameter warning
    kill(sendPID, SIGINT);
}

static void sendTERM(int sig)
{
    (void)sig; // C23: suppress unused parameter warning
    kill(sendPID, SIGTERM);
    exitMacaulay();
}

static void sendCONT(int sig)
{
    (void)sig; // C23: suppress unused parameter warning
    kill(sendPID, SIGCONT);
}

static void doZombie(int pid)
{
    sendPID = pid;

    signal(SIGINT, sendINT);
    signal(SIGTERM, sendTERM);
    signal(SIGCONT, sendCONT);
}

void wait_cmd(void)
{
    sigpause(0);
}

void detach_cmd(int argc, char* argv[])
{
    int pid;

    if (argc > 2)
    {
        printnew("detach [output file]\n");
        return;
    }
    signal(SIGHUP, SIG_IGN);
    pid = fork();
    if (pid == 0)
    {
        if (argc > 1)
            freopen(argv[1], "w", stdout);
        doOrphan();
    }
    else
    {
        wrPID(pid);
        exitMacaulay();
    }
}

void attach_cmd(void)
{
    int pid;

    pid = rdPID();
    if (pid == 0)
    {
        prerror("; can't attach to Macaulay\n");
        return;
    }
    wrTTY();
    doZombie(pid);
    kill(pid, sigA);
    while (1)
        sigpause(0);
}

void sighup_cmd(void)
{
    signal(SIGHUP, SIG_IGN);
    wrPID(getpid());
    doOrphan();
}

#else

#include "shared.h"
#include "detach.h"

void exitMacaulay(void)
{
#ifdef SIOW
    print("\n[Macaulay has quit]\n");
#endif
    exit(0);
}

// void wait_cmd(void) {}
// void detach_cmd(int argc, char *argv[]) { (void)argc; (void)argv; }
// void attach_cmd(void) {}
// void sighup_cmd(void) {}
#endif
