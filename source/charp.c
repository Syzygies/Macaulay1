// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "charp.h"

typedef long int arith;

int charac;
arith characl;
int half_pos;
int half_neg;

field fd_zero = 0;
field fd_one = 1;
int field_size = sizeof(field);

void init_charp(int characteristic)
{
    charac = characteristic;
    characl = charac;
    if (charac == 2)
    {
        half_pos = 1;
        half_neg = 0;
    }
    else
    {
        half_pos = (charac - 1) / 2;
        half_neg = -half_pos;
    }
}

field normalize(arith n)
{
    register field a;

    a = (field)(n % charac);
    if (a < half_neg)
        a = (field)(a + charac);
    else if (a > half_pos)
        a = (field)(a - charac);
    return (a);
}

void gcd_extended(arith a, arith b, arith* u, arith* v, arith* g)
{
    register arith q;
    register arith u1, v1, g1;
    register arith utemp, vtemp, gtemp;

    g1 = b;
    u1 = 0;
    v1 = 1;
    *g = a;
    *u = 1;
    *v = 0;
    while (g1 != 0)
    {
        q = *g / g1;
        gtemp = (*g) - q * g1;
        utemp = (*u) - q * u1;
        vtemp = (*v) - q * v1;
        *g = g1;
        *u = u1;
        *v = v1;
        g1 = gtemp;
        u1 = utemp;
        v1 = vtemp;
    }
}

void lift(int m, int c, int* v2, int* v3)
{
    register arith vv2, vv3;
    register int u1, u2, u3, r1, r2, r3, v1;
    register int q;

    vv3 = c;
    v1 = 0;
    vv2 = 1;
    u3 = m;
    u1 = 1;
    u2 = 0;
    while (2 * (vv3) * (vv3) >= m)
    {
        q = u3 / (int)(vv3);
        r3 = (u3) - q * (int)(vv3);
        r1 = (u1) - q * v1;
        r2 = (u2) - q * (int)(vv2);
        u3 = (int)vv3;
        u1 = v1;
        u2 = (int)vv2;
        vv3 = r3;
        v1 = r1;
        vv2 = r2;
    }
    if (2 * (vv2) * (vv2) >= m)
        (vv2) = 0;
    *v2 = (int)vv2;
    *v3 = (int)vv3;
}

void fd_add(field a, field b, field* c)
{
    register arith val = a;
    val += b;
    *c = normalize(val);
}

void fd_sub(field a, field b, field* c)
{
    register arith val = a;
    val -= b;
    *c = normalize(val);
}

void fd_mult(field a, field b, field* c)
{
    register arith val = a;
    val *= b;
    *c = normalize(val);
}

void fd_negate(field* a)
{
    register arith val = -(*a);
    *a = normalize(val);
}

void fd_div(field a, field b, field* c)
{
    register arith bl = b;
    register arith val;
    arith binv, m, n;

    gcd_extended(bl, characl, &binv, &m, &n);
    val = a * binv * n; // n = 1, or -1
    *c = normalize(val);
}

void fd_recip(field* a)
{
    register arith al = *a;
    register arith val;
    arith ainv, m, n;

    gcd_extended(al, characl, &ainv, &m, &n);
    val = ainv * n;
    *a = normalize(val);
}

field fd_copy(field a)
{
    return (a);
}

boolean fd_iszero(field a)
{
    return (a == 0);
}
