/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */

// void i_stats ();
// void pr_stats ();

#ifdef STATISTICS
long stcomp;
long stadd;
long stradd;
long stdiv;
long stloop;
long stspecial;

void i_stats ()
{
    stcomp = stadd = stradd = 0L;
    stdiv = stloop = stspecial = 0L;
}

void pr_stats ()
{
    print("# tm_compare  = %ld\n", stcomp);
    print("# tm_add      = %ld\n", stadd);
    print("# division    = %ld\n", stdiv);
    print("# div loops   = %ld\n", stloop);
    print("# special sub = %ld\n", stspecial);
}

#else

void i_stats ()
{};
void pr_stats ()
{};

#endif
