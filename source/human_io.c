/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "mtypes.h"
#include "ddefs.h"

// int abs (int m);
// int get_int (char *s);
// int get_defint (char *s, int n);
// char * get_str (char *s);
// boolean get_char (char c);
// int int_scan (char **str);
// void int_pprint (FILE *fil, int m, boolean p_one, boolean p_plus);
// int int_size (int m, boolean p_one, boolean p_plus);
// field fd_scan (char **str);
// void fd_pprint (FILE *fil, field a, boolean p_one, boolean p_plus);
// int fd_size (field a, boolean p_one, boolean p_plus);
// poly p_scan (char **str, int comp);
// void p_pprint (FILE *fil, poly f, int comp);
// void pdeb (poly f, int comp);
// int p_size (poly f, int comp);
// void pl_scan (plist *pl, int numcols, int numrows);
// void plSparse_scan (plist *pl, int numcols, int numrows);
// void putmat (FILE *fil, gmatrix M);
// int pr_size (poly f, int maxcomp);
// boolean /* returns FALSE if user wants out */ printsofar (FILE *fil, plist *pl, int sizes[], int first, int last, int numrows);
// void pl_pprint (FILE *fil, plist *pl, int numrows, int rowsize);
// void dl_pprint (FILE *fil, dlist *dl);
// boolean check_homog (gmatrix M);
// void setdeggens (gmatrix M);
// void setdegs (gmatrix M);
// void setzerodegs (gmatrix M, int numrows);
// gmatrix mat_scan ();
// gmatrix sparse_scan ();
// gmatrix ideal_scan ();
// gmatrix poly_scan (int argc, char *argv[]);

extern int prlevel;
#define PR_POLY 100

int abs (int m)
{
    if (m < 0) return(-m);
    else return(m);
}

/*
 *  The following 3 routines are used, along with get_line, as a
 *  input interface.
 *  get_int : check input for an integer, get it, and return it.
 *  get_str : same, except returns a string.  CAUTION: better use
 *      its value soon, or it will be overwritten; ie it isn't
 *      copied from the input buffer.
 *  get_char :  get_char(c) checks the input for the char. "c".  If
 *      it is found, and its the only thing on the line, then TRUE
 *      is returned, else FALSE is returned.  This is used for default
 *      values on input.
 */

int get_int (char *s)
{
    int n, argc;
    char **argv;

    if (*s != '\0') prinput("%s",s);
    get_line(&argc, &argv);
    while (argc ISNT 1) {
        /*while ((argc ISNT 1) OR (!isdigit(*(argv[0])))) {*/
        prinput_error("What? %s", s);
        get_line(&argc, &argv);
    }
    n = getInt(argv[0]);
    return(n);
}

int get_defint (char *s, int n)
{
    int argc;
    char **argv;

    if (*s != '\0') prinput("%s",s);
    get_line(&argc, &argv);
    /*if ((argc ISNT 1) OR (!isdigit(*(argv[0])))) */
    if (argc ISNT 1)
        return(n);
    n = getInt(argv[0]);
    return(n);
}
char * get_str (char *s)
{
    int argc;
    char **argv;

    if (*s != '\0') prinput("%s",s);
    get_line(&argc, &argv);
    while (argc ISNT 1) {
        prinput_error("What? %s", s);
        get_line(&argc, &argv);
    }
    return(argv[0]);
}

boolean get_contstr(s, str)     /* get possibly continued line */
char *s;
char **str;
{
    int argc;
    char **argv;

    while (TRUE) {
        get_line(&argc, &argv);
        *str = argv[0];
        if (argc IS 1)
            return(FALSE);
        if ((argc IS 2) AND (*(argv[1])  IS  '\\'))
            return(TRUE);
        prinput_error("What? %s",s);
    }
}

boolean get_char (char c)
{
    int argc;
    char **argv;

    get_line(&argc, &argv);
    if (argc ISNT 1) return(FALSE);
    if (*argv[0] ISNT c) return(FALSE);
    if (argv[0][1] ISNT '\0') return(FALSE);
    return(TRUE);
}

int int_scan (char **str)
{
    int sum = 0;

    if (! isdigit(**str))
        sum = 1;
    else do {
            sum = 10 * sum + (**str) - '0';
            (*str)++;
        } while (isdigit(**str));
    return(sum);
}

void int_pprint (FILE *fil, int m, boolean p_one, boolean p_plus)
{
    if (m < 0)
        fprint(fil, "-");
    else if (p_plus)
        fprint(fil, "+");

    m = abs(m);
    if ((m IS 1) AND (!p_one))
        return;
    fprint(fil,"%d",m);
}

int int_size (int m, boolean p_one, boolean p_plus)
{
    int sum = 0;
    if (m < 0) sum++;
    else if (p_plus) sum++;
    m = abs(m);
    if ((m IS 1) AND (!p_one)) return(sum);
    for (; m >= 10; (sum++), m = m/10);
    sum++;
    return(sum);
}

field fd_scan (char **str)
{
    char c;
    field ans;
    field numer, denom;
    long int neg;  /* should be set to "arith" */

    c = *((*str)++);
    neg = 1;
    if (c IS '-')  neg = -1;
    else if (c ISNT '+')
        (*str)--;
    numer = neg * int_scan(str);
    denom = 1;
    if (**str IS '?') (*str)++;
    else if (**str IS '/') {
        (*str)++;
        denom = int_scan(str);
    }
    fd_div(numer, denom, &ans);
    return(ans);
}

void fd_pprint (FILE *fil, field a, boolean p_one, boolean p_plus)
{
    int n, m;

    if (pr_charp > 0) {
        int_pprint(fil, a, p_one, p_plus);
        return;
    }
    lift(charac, a, &n, &m); /* n = denom., m = numerator */
    if (n IS 0) {
        int_pprint(fil, a, p_one, p_plus);
        /*fprint(fil, "?");*/
        return;
    }
    if (n < 0) {
        n = -n;
        m = -m;
    }
    if (n IS 1)
        int_pprint(fil, m, p_one, p_plus);
    else {
        int_pprint(fil, m, TRUE, p_plus);
        fprint(fil, "/");
        int_pprint(fil, n, FALSE, FALSE);
    }
}

int fd_size (field a, boolean p_one, boolean p_plus)
{
    int n, m;
    int sum = 0;

    if (pr_charp > 0) {
        sum = int_size(a, p_one, p_plus);
        return(sum);
    }
    lift(charac, a, &n, &m);
    if (n IS 0) {
        sum = int_size(a, p_one, p_plus);
        return(sum); /* sum+1 if "?" is to be used, which it isn't */
    }
    if (n < 0) {
        n = -n;
        m = -m;
    }
    if (n IS 1)
        sum = int_size(m, p_one, p_plus);
    else {
        sum = int_size(m, TRUE, p_plus);
        sum++;
        sum += int_size(n, FALSE, FALSE);
    }
    return(sum);
}
/*
poly p_scan (char **str, int comp)
{
    int c;
    poly temp, result;
    field a;
    poly parsePoly();

    return(parsePoly(str, comp));
    result = NULL;
    while (**str ISNT '\0') {
        a = fd_scan(str);
        temp = p_monom(a);
        tm_scan(str, comp, INITIAL(temp));
    if (a IS 0)
        p_kill(&temp);
        else
        p_add(&result, &temp);
    }
    qrgReduce(&result);
    return(result);
}
*/

extern int linesize, prcomment;

void p_pprint (FILE *fil, poly f, int comp)
{
    boolean z, printed_any;
    int psize, tsize, rowsize;
    boolean donewline;  /* don't usually want beginning '; ' if sent to file*/

    donewline = ((fil IS stdout) && (prcomment > 0));

    rowsize = (donewline ? linesize - 2 : linesize);
    /* linesize, prcomment are global vars */
    psize = 0;
    printed_any = FALSE;
    while (f ISNT NULL)
    {
        if (tm_component(INITIAL(f)) IS comp) {
            z = tm_iszero(INITIAL(f));
            tsize = fd_size(f->coef,z,printed_any);
            tsize += tm_size(INITIAL(f));
            if (psize+tsize > rowsize-1) {
                fprint(fil," \\\n");
                if (donewline) fnewline(fil);
                fprint(fil,"    ");
                psize = 4;
            }
            fd_pprint(fil,f->coef,z,printed_any);
            tm_pprint(fil,INITIAL(f));
            printed_any = TRUE;
            psize += tsize;
        }
        f = f->next;
    }
    if (!printed_any)
        fprint(fil, "0");
} /* mod 24feb89 DB */

void pdeb (poly f, int comp)
{
    p_pprint(stdout,f,comp);
}

int p_size (poly f, int comp)
{
    boolean z, printed_any;
    int sum = 0;

    printed_any = FALSE;
    while (f ISNT NULL)
    {
        if (tm_component(INITIAL(f)) IS comp) {
            z = tm_iszero(INITIAL(f));
            sum += fd_size(f->coef,z,printed_any);
            sum += tm_size(INITIAL(f));
            printed_any = TRUE;
        }
        f = f->next;
    }
    if (!printed_any) sum = 1;
    return(sum);
}

/* pl_scan : reads in a list of polys, numrows = rank
                                       numcols = numelems */

void pl_scan (plist *pl, int numcols, int numrows)
{
    int i, j;
    poly f, g;

    pl_init(pl);
    for (i=1; i<=numcols; i++) {
        f = NULL;
        for (j=1; j<=numrows; j++) {
            prinput("(%d,%d)",j,i);
            g = rdPoly(j);
            p_add(&f, &g);
        }
        pl_insert(pl, f);
    }
}

void plSparse_scan (plist *pl, int numcols, int numrows)
{
    int i, r, c, argc;
    char **argv;
    poly f;

    pl_init(pl);
    for (i=1; i<=numcols; i++) pl_insert(pl, NULL);
    while (TRUE) {
        prinput("row column value ");
        get_line(&argc, &argv);
        if ((argc < 3) OR (argc > 4))
            return;
        r = getInt(argv[0]);
        c = getInt(argv[1]);
        if ((r<1) OR (r>numrows) OR (c<1) OR (c>numcols)) {
            prerror("; entry ignored, index out of range\n");
            continue;
        }
        f = rdPolyStr(argc-2,argv+2, r);
        p_add(&(PREF(*pl, c)), &f);
    }
}

void putmat (FILE *fil, gmatrix M)
{
    int i, j;
    boolean donewline;  /* don't usually want beginning '; ' if sent to file*/

    donewline = (fil IS stdout);
    if (donewline) fnewline(fil);
    fprint(fil,"%d\n", nrows(M));
    if (donewline) fnewline(fil);
    fprint(fil,"%d\n",ncols(M));
    for (i=1; i<=ncols(M); i++)
        for (j=1; j<=nrows(M); j++) {
            if (donewline) fnewline(fil);
            p_pprint(fil, PREF(M->gens, i), j);
            fprint(fil, "\n");
        }
}

int pr_size (poly f, int maxcomp)
{
    int i, d, result = 0;
    for (i=1; i<=maxcomp; i++)
        if ((d=p_size(f,i)) > result)
            result = d;
    return(result);
}

boolean /* returns FALSE if user wants out */ printsofar (FILE *fil, plist *pl, int sizes[], int first, int last, int numrows)
{
    int c, r, s, j;

    if (first ISNT 1) fprint(fil, "\n");
    for (r=1; r<=numrows; r++) {
        fnewline(fil);
        for (c=first; c<=last; c++) {
            p_pprint(fil, PREF(*pl, c), r);
            s = p_size(PREF(*pl, c), r);
            for (j=0;  j<sizes[c-first]-s; j++)
                fprint(fil, " ");
        }
        fprint(fil, "\n");
        if (have_intr()) return(FALSE);
    }
    return(TRUE);
}

void pl_pprint (FILE *fil, plist *pl, int numrows, int rowsize)
{
    int sizes[PR_POLY];
    int total, s;
    int first, i;
    boolean isintr;

    if (prcomment > 0) rowsize -= 2;
    total = 0;
    first = 1;
    for (i=1; i<=LENGTH(*pl); i++) {
        s = pr_size(PREF(*pl,i), numrows) + 1;
        if (((s+total) > rowsize) AND (i > first)) {
            isintr = printsofar(fil, pl, sizes, first, i-1, numrows);
            if (NOT isintr) return;    /* user wants out */
            total = 0;
            first = i;
        }
        total += s;
        sizes[i-first] = s;
    }
    if (total > 0)
        printsofar(fil, pl, sizes, first, LENGTH(*pl), numrows);
} /* mod 24feb89 DB */

void dl_pprint (FILE *fil, dlist *dl)
{
    int this, num, i;

    if (LENGTH(*dl) IS 0) return;
    this = DREF(*dl, 1);
    num = 1;
    for (i=2; i<=LENGTH(*dl); i++)
        if (this IS DREF(*dl, i))
            num++;
        else  {
            fprint(fil, "%d [%d] ", this, num);
            this = DREF(*dl, i);
            num = 1;
        }
    fprint(fil, "%d [%d]\n", this, num);
}

boolean check_homog (gmatrix M)
{
    int i, d;
    poly f;

    for (i=1; i<=LENGTH(M->gens); i++) {
        f = PREF(M->gens, i);
        d = DREF(M->deggens, i);
        while (f ISNT NULL) {
            if (d ISNT degree(M, f))
                return(FALSE);
            f = f->next;
        }
    }
    return(TRUE);
}

void setdeggens (gmatrix M)
{
    int i, c;
    poly f;
    int argc;
    char **argv;

    prinput("degrees for each column (1 thru %d, default=compute from rows)",
            ncols(M));
    c = ncols(M);
    dl_kill(&M->deggens);
    dl_init(&M->deggens);

    get_line(&argc, &argv);
    if (argc > 0) {
        dlConsume(&M->deggens, argc, argv, c, 0);
        if (!check_homog(M)) print("; warning: module isn't homogeneous\n");
        return;
    }

    for (i=1; i<=length(&M->gens); i++)
        if ((f = PREF(M->gens, i)) IS NULL) {
            dl_insert(&M->deggens, 0)
        } else
            dl_insert(&M->deggens, degree(M, f));
    if (!check_homog(M)) print("; warning: module isn't homogeneous\n");
}

void setdegs (gmatrix M)
{
    int r;

    prinput("degrees for each row (1 thru %d, default=0's)", nrows(M));
    r = nrows(M);
    dl_kill(&M->degrees);
    dl_init(&M->degrees);
    dlScan(&M->degrees, r, 0);
    setdeggens(M);
}

void setzerodegs (gmatrix M, int numrows)
{
    int i, numcols;
    poly f;

    numcols = length(&M->gens);
    for (i=1; i<=numrows; i++)
        dl_insert(&M->degrees, 0);
    for (i=1; i<=numcols; i++) {
        f = PREF(M->gens, i);
        if (f IS NULL) {
            dl_insert(&M->deggens, 0);
        } else {
            dl_insert(&M->deggens, degree(M, f));
        }
    }
}

gmatrix mat_scan ()
{
    int numrows, numcols;
    gmatrix M;

    numrows = get_defint("number of rows   ", 0);
    numcols = get_defint("number of columns", 0);
    M = mod_init();

    pl_scan(&M->gens, numcols, numrows);
    setzerodegs(M, numrows);
    return(M);
}

gmatrix sparse_scan ()
{
    int numrows, numcols;
    gmatrix M;

    numrows = get_defint("number of rows   ", 0);
    numcols = get_defint("number of columns", 0);
    M = mod_init();

    plSparse_scan(&M->gens, numcols, numrows);
    setzerodegs(M, numrows);
    return(M);
}

gmatrix ideal_scan ()
{
    int numgens;
    gmatrix M;

    M = mod_init();
    numgens = get_int("number of generators");

    pl_scan(&M->gens, numgens, 1);
    setzerodegs(M, 1);
    return(M);
}

gmatrix poly_scan (int argc, char *argv[])
{
    gmatrix M;
    poly f;

    M = mod_init();
    f = rdPolyStr(argc, argv, 1);
    setzerodegs(M, 1);
    gmInsert(M, f);
    return(M);
}
