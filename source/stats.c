// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "stats.h"
#include "monitor.h" // For print()

// Always define these variables, regardless of STATISTICS
long stcomp;
long stadd;
long stradd;
long stdiv;
long stloop;
long stspecial;

#ifdef STATISTICS

void i_stats(void)
{
    stcomp = stadd = stradd = 0L;
    stdiv = stloop = stspecial = 0L;
}

void pr_stats(void)
{
    print("# tm_compare  = %ld\n", stcomp);
    print("# tm_add      = %ld\n", stadd);
    print("# division    = %ld\n", stdiv);
    print("# div loops   = %ld\n", stloop);
    print("# special sub = %ld\n", stspecial);
}

#else

void i_stats(void)
{
    stcomp = stadd = stradd = 0L;
    stdiv = stloop = stspecial = 0L;
}

void pr_stats(void)
{
}

#endif
