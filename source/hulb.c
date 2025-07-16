// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <stdio.h>
#include "shared.h"
#include "hulb.h"
#include "koszul.h"    // For binom()
#include "cmds.h"      // For vget_mod()
#include "mac.h"       // For have_intr()
#include "monitor.h"   // For print()
#include "stdcmds.h"   // For stdWarning()
#include "parse.h"     // For getInt()
#include "hilb.h"      // For monrefund()
#include "monideal.h"  // For monhilb()

// Global variables
long tullexp[NVARS];
int deg;

void i_tull(int nvars, int d)
{
    int i;

    for (i = 0; i < nvars; i++)
        tullexp[i] = 0;
    deg = d;
}

void tull(arrow head, int plus)
{
    int i, j, d, e, n, n2, c1, c2, s;
    int* a;

    d = head->umh.mpred;
    n = head->umh.mn;
    n2 = head->umh.mpren;
    a = head->umh.mloc->umn.mexp;
    for (i = 0; i < n; ++i)
        d += a[i];
    if (d > deg)
        return;
    // d == degree of new monomial
    e = deg - d;
    c1 = binom(numvars - 1 + e, e);
    c2 = (e == 0 ? 0 : binom(numvars - 1 + e, e - 1));
    s = (plus ? -1 : 1); // yeah, that's right, buster
    for (i = 0, j = 0; i < n2; ++i, ++j)
        tullexp[j] += s * (c1 * head->umh.mstack[i].mpre + c2);
    for (i = 0; i < n; ++i, ++j)
        tullexp[j] += s * (c1 * a[i] + c2);
}

boolean hulb(gmatrix M, int d)
{
    // returns TRUE if user doesn't interrupt
    arrow head;

    i_tull(numvars, d);
    if ((head = monhilb(M, 1, tull)) == NULL)
        return FALSE;
    monrefund(head);
    return TRUE;
}

// boolean hulb(gmatrix M, int d)
// {

// expterm nexp;
// poly f;
// arrow head;
// modgen mg;

// i_tull(numvars, d);
// head = monnewhead(numvars);
// stdFirst(M, &mg, USESTD);
// while ((f=stdNext(&mg)) != NULL) {
// if (have_intr()) {
// monrefund(head);
// print("\n");
// return(FALSE);
// }
// sToExp(INITIAL(f), nexp);
// monadjoin(head, nexp, tull);
// }
// monrefund(head);
// return(TRUE);
// }

void hulb_cmd(int argc, char* argv[])
{
    gmatrix M;
    int i, d;

    if (argc != 3)
    {
        print("hulb <standard basis> <deg>\n");
        return;
    }
    if ((M = vget_mod(argv[1])) == NULL)
    {
        print("Error: invalid module\n");
        return;
    }
    d = getInt(argv[2]);
    stdWarning(M);
    hulb(M, d);
    for (i = 0; i < numvars; i++)
        print("%ld ", tullexp[i]);
    print("\n");
}
