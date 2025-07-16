// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

// Note: struct matches is defined in gm_dlist.h (included below)
// struct stack_struct has no proper header - this is a project-wide issue
// that requires consolidating the multiple definitions into a single header

#include "shared.h"
#include "human_io.h"
#include "charp.h"      // For charac, lift(), fd_div()
#include "cmds.h"       // For vget_mod()
#include "gmatrix.h"    // For nrows(), ncols(), gmInsert()
#include "input.h"      // For get_line()
#include "mac.h"        // For have_intr()
#include "monitor.h"    // For print functions
#include "plist.h"      // For degree(), mod_init(), dl_kill()
#include "poly.h"       // For p_monom(), p_add(), p_kill()
#include "qring.h"      // For qrgReduce()
#include "set.h"        // For prlevel, linesize, prcomment, pr_charp
#include "term.h"       // For tm_component(), tm_iszero(), tm_size(), tm_pprint(), tm_scan()
#include "parse.h"      // For getInt()
#include "gm_dlist.h"   // For dlConsume(), dlScan()
#include "parsepoly.h"  // For rdPoly(), rdPolyStr()

constexpr int PR_POLY = 100;

static int hu_abs(int m)
{
    if (m < 0)
        return (-m);
    else
        return (m);
}

// The following 3 routines are used, along with get_line, as a
// input interface.
// get_int : check input for an integer, get it, and return it.
// get_str : same, except returns a string.  CAUTION: better use
// its value soon, or it will be overwritten; ie it isn't
// copied from the input buffer.
// get_char :  get_char(c) checks the input for the char. "c".  If
// it is found, and its the only thing on the line, then TRUE
// is returned, else FALSE is returned.  This is used for default
// values on input.

int get_int(const char* s)
{
    int n, argc;
    char** argv;

    if (*s != '\0')
        prinput("%s", s);
    get_line(&argc, &argv);
    while (argc != 1)
    {
        // while ((argc != 1) || (!isdigit(*(argv[0])))) {
        prinput_error("What? %s", s);
        get_line(&argc, &argv);
    }
    n = getInt(argv[0]);
    return (n);
}

int get_defint(const char* s, int n)
{
    int argc;
    char** argv;

    if (*s != '\0')
        prinput("%s", s);
    get_line(&argc, &argv);
    // if ((argc != 1) OR (!isdigit(*(argv[0]))))
    if (argc != 1)
        return (n);
    n = getInt(argv[0]);
    return (n);
}

char*get_str(const char* s)
{
    int argc;
    char** argv;

    if (*s != '\0')
        prinput("%s", s);
    get_line(&argc, &argv);
    while (argc != 1)
    {
        prinput_error("What? %s", s);
        get_line(&argc, &argv);
    }
    return (argv[0]);
}

boolean get_contstr(const char* s, char** str) // get possibly continued line
{
    int argc;
    char** argv;

    while (TRUE)
    {
        get_line(&argc, &argv);
        *str = argv[0];
        if (argc == 1)
            return (FALSE);
        if ((argc == 2) && (*(argv[1]) == '\\'))
            return (TRUE);
        prinput_error("What? %s", s);
    }
}

boolean get_char(char c)
{
    int argc;
    char** argv;

    get_line(&argc, &argv);
    if (argc != 1)
        return (FALSE);
    if (*argv[0] != c)
        return (FALSE);
    if (argv[0][1] != '\0')
        return (FALSE);
    return (TRUE);
}

int int_scan(char** str)
{
    int sum = 0;

    if (!isdigit(**str))
        sum = 1;
    else
        do
        {
            sum = 10 * sum + (**str) - '0';
            (*str)++;
        } while (isdigit(**str));
    return (sum);
}

void int_pprint(FILE* fil, int m, boolean p_one, boolean p_plus)
{
    if (m < 0)
        fprint(fil, "-");
    else if (p_plus)
        fprint(fil, "+");

    m = hu_abs(m);
    if ((m == 1) && (!p_one))
        return;
    fprint(fil, "%d", m);
}

int int_size(int m, boolean p_one, boolean p_plus)
{
    int sum = 0;
    if (m < 0)
        sum++;
    else if (p_plus)
        sum++;
    m = hu_abs(m);
    if ((m == 1) && (!p_one))
        return (sum);
    for (; m >= 10; (sum++), m = m / 10)
        ;
    sum++;
    return (sum);
}

field fd_scan(char** str)
{
    char c;
    field ans;
    field numer, denom;
    long int neg; // should be set to "arith"

    c = *((*str)++);
    neg = 1;
    if (c == '-')
        neg = -1;
    else if (c != '+')
        (*str)--;
    numer = (field)(neg * int_scan(str));
    denom = 1;
    if (**str == '?')
        (*str)++;
    else if (**str == '/')
    {
        (*str)++;
        denom = (field)int_scan(str);
    }
    fd_div(numer, denom, &ans);
    return (ans);
}

void fd_pprint(FILE* fil, field a, boolean p_one, boolean p_plus)
{
    int n, m;

    if (pr_charp > 0)
    {
        int_pprint(fil, a, p_one, p_plus);
        return;
    }
    lift(charac, a, &n, &m); // n = denom., m = numerator
    if (n == 0)
    {
        int_pprint(fil, a, p_one, p_plus);
        // fprint(fil, "?") ;
        return;
    }
    if (n < 0)
    {
        n = -n;
        m = -m;
    }
    if (n == 1)
        int_pprint(fil, m, p_one, p_plus);
    else
    {
        int_pprint(fil, m, TRUE, p_plus);
        fprint(fil, "/");
        int_pprint(fil, n, FALSE, FALSE);
    }
}

int fd_size(field a, boolean p_one, boolean p_plus)
{
    int n, m;
    int sum = 0;

    if (pr_charp > 0)
    {
        sum = int_size(a, p_one, p_plus);
        return (sum);
    }
    lift(charac, a, &n, &m);
    if (n == 0)
    {
        sum = int_size(a, p_one, p_plus);
        return (sum); // sum+1 if "?" is to be used, which it isn't
    }
    if (n < 0)
    {
        n = -n;
        m = -m;
    }
    if (n == 1)
        sum = int_size(m, p_one, p_plus);
    else
    {
        sum = int_size(m, TRUE, p_plus);
        sum++;
        sum += int_size(n, FALSE, FALSE);
    }
    return (sum);
}

// poly p_scan(str, comp)
// char **str;
// int comp ;
// {
// int c ;
// poly temp, result ;
// field a ;
// poly parsePoly() ;

// return(parsePoly(str, comp)) ;
// result = NULL ;
// while (**str != '\0') {
// a = fd_scan(str) ;
// temp = p_monom(a) ;
// tm_scan(str, comp, INITIAL(temp)) ;
// if (a == 0)
// p_kill(&temp) ;
// else
// p_add(&result, &temp) ;
// }
// qrgReduce(&result) ;
// return(result) ;
// }

void p_pprint(FILE* fil, poly f, int comp)
{
    boolean z, printed_any;
    int psize, tsize, rowsize;
    boolean donewline; // don't usually want beginning '; ' if sent to file

    donewline = ((fil == stdout) && (prcomment > 0));

    rowsize = (donewline ? linesize - 2 : linesize);
    // linesize, prcomment are global vars
    psize = 0;
    printed_any = FALSE;
    while (f != NULL)
    {
        if (tm_component(poly_initial(f)) == comp)
        {
            z = tm_iszero(poly_initial(f));
            tsize = fd_size(f->coef, z, printed_any);
            tsize += tm_size(poly_initial(f));
            if (tsize > rowsize - 1 - psize)
            {
                fprint(fil, " \\\n");
                if (donewline)
                    fnewline(fil);
                fprint(fil, "    ");
                psize = 4;
            }
            fd_pprint(fil, f->coef, z, printed_any);
            tm_pprint(fil, poly_initial(f));
            printed_any = TRUE;
            psize += tsize;
        }
        f = f->next;
    }
    if (!printed_any)
        fprint(fil, "0");
} // mod 24feb89 DB

void pdeb(poly f, int comp)
{
    p_pprint(stdout, f, comp);
}

int p_size(poly f, int comp)
{
    boolean z, printed_any;
    int sum = 0;

    printed_any = FALSE;
    while (f != NULL)
    {
        if (tm_component(poly_initial(f)) == comp)
        {
            z = tm_iszero(poly_initial(f));
            sum += fd_size(f->coef, z, printed_any);
            sum += tm_size(poly_initial(f));
            printed_any = TRUE;
        }
        f = f->next;
    }
    if (!printed_any)
        sum = 1;
    return (sum);
}

// pl_scan : reads in a list of polys, numrows = rank
// numcols = numelems

void pl_scan(plist* pl, int numcols, int numrows)
{
    int i, j;
    poly f, g;

    plist_init(pl);
    for (i = 1; i <= numcols; i++)
    {
        f = NULL;
        for (j = 1; j <= numrows; j++)
        {
            prinput("(%d,%d)", j, i);
            g = rdPoly(j); // Pass component number j directly
            p_add(&f, &g);
        }
        plist_insert(pl, f);
    }
}

void plSparse_scan(plist* pl, int numcols, int numrows)
{
    int i, r, c, argc;
    char** argv;
    poly f;

    plist_init(pl);
    for (i = 1; i <= numcols; i++)
        plist_insert(pl, NULL);
    while (TRUE)
    {
        prinput("row column value ");
        get_line(&argc, &argv);
        if ((argc < 3) || (argc > 4))
            return;
        r = getInt(argv[0]);
        c = getInt(argv[1]);
        if ((r < 1) || (r > numrows) || (c < 1) || (c > numcols))
        {
            prerror("; entry ignored, index out of range\n");
            continue;
        }
        f = rdPolyStr(argc - 2, argv + 2, r);
        poly* pref_ptr = (poly*)ref((array*)pl, c);
        p_add(pref_ptr, &f);
    }
}

void putmat(FILE* fil, gmatrix M)
{
    int i, j;
    boolean donewline; // don't usually want beginning '; ' if sent to file

    donewline = (fil == stdout);
    if (donewline)
        fnewline(fil);
    fprint(fil, "%d\n", nrows(M));
    if (donewline)
        fnewline(fil);
    fprint(fil, "%d\n", ncols(M));
    for (i = 1; i <= ncols(M); i++)
        for (j = 1; j <= nrows(M); j++)
        {
            if (donewline)
                fnewline(fil);
            p_pprint(fil, plist_ref(&M->gens, i), j);
            fprint(fil, "\n");
        }
}

int pr_size(poly f, int maxcomp)
{
    int i, d, result = 0;
    for (i = 1; i <= maxcomp; i++)
        if ((d = p_size(f, i)) > result)
            result = d;
    return (result);
}

static boolean printsofar(FILE* fil, plist* pl, int sizes[], int first, int last,
                          int numrows) // returns FALSE if user wants out
{
    int c, r, s, j;

    if (first != 1)
        fprint(fil, "\n");
    for (r = 1; r <= numrows; r++)
    {
        fnewline(fil);
        for (c = first; c <= last; c++)
        {
            p_pprint(fil, plist_ref(pl, c), r);
            s = p_size(plist_ref(pl, c), r);
            for (j = 0; j < sizes[c - first] - s; j++)
                fprint(fil, " ");
        }
        fprint(fil, "\n");
        if (have_intr())
            return (FALSE);
    }
    return (TRUE);
}

void pl_pprint(FILE* fil, plist* pl, int numrows, int rowsize)
{
    int sizes[PR_POLY];
    int total, s;
    int first, i;
    boolean isintr;

    if (prcomment > 0)
        rowsize -= 2;
    total = 0;
    first = 1;
    for (i = 1; i <= array_length((array*)pl); i++)
    {
        s = pr_size(plist_ref(pl, i), numrows) + 1;
        if (((s + total) > rowsize) && (i > first))
        {
            isintr = printsofar(fil, pl, sizes, first, i - 1, numrows);
            if (!isintr)
                return; // user wants out
            total = 0;
            first = i;
        }
        total += s;
        sizes[i - first] = s;
    }
    if (total > 0)
        printsofar(fil, pl, sizes, first, array_length((array*)pl), numrows);
} // mod 24feb89 DB

void dl_pprint(FILE* fil, dlist* dl)
{
    int this, num, i;

    if (array_length((array*)dl) == 0)
        return;
    this = dlist_ref(dl, 1);
    num = 1;
    for (i = 2; i <= array_length((array*)dl); i++)
        if (this == dlist_ref(dl, i))
            num++;
        else
        {
            fprint(fil, "%d [%d] ", this, num);
            this = dlist_ref(dl, i);
            num = 1;
        }
    fprint(fil, "%d [%d]\n", this, num);
}

boolean check_homog(gmatrix M)
{
    int i, d;
    poly f;

    for (i = 1; i <= array_length(&M->gens); i++)
    {
        f = plist_ref(&M->gens, i);
        d = dlist_ref(&M->deggens, i);
        while (f != NULL)
        {
            if (d != degree(M, f))
                return (FALSE);
            f = f->next;
        }
    }
    return (TRUE);
}

void setdeggens(gmatrix M)
{
    register int i, c;
    register poly f;
    int argc;
    char** argv;

    prinput("degrees for each column (1 thru %d, default=compute from rows)", ncols(M));
    c = ncols(M);
    dl_kill(&M->deggens);
    dlist_init(&M->deggens);

    get_line(&argc, &argv);
    if (argc > 0)
    {
        dlConsume(&M->deggens, argc, argv, c, 0);
        if (!check_homog(M))
            print("; warning: module isn't homogeneous\n");
        return;
    }

    for (i = 1; i <= array_length(&M->gens); i++)
        if ((f = plist_ref(&M->gens, i)) == NULL)
        {
            dlist_insert(&M->deggens, 0);
        }
        else
            dlist_insert(&M->deggens, degree(M, f));
    if (!check_homog(M))
        print("; warning: module isn't homogeneous\n");
}

void setdegs(gmatrix M)
{
    int r;

    prinput("degrees for each row (1 thru %d, default=0's)", nrows(M));
    r = nrows(M);
    dl_kill(&M->degrees);
    dlist_init(&M->degrees);
    dlScan(&M->degrees, r, 0);
    setdeggens(M);
}

void setzerodegs(gmatrix M, int numrows)
{
    register int i, numcols;
    poly f;

    numcols = array_length(&M->gens);
    for (i = 1; i <= numrows; i++)
        dlist_insert(&M->degrees, 0);
    for (i = 1; i <= numcols; i++)
    {
        f = plist_ref(&M->gens, i);
        if (f == NULL)
        {
            dlist_insert(&M->deggens, 0);
        }
        else
        {
            dlist_insert(&M->deggens, degree(M, f));
        }
    }
}

gmatrix mat_scan(void)
{
    int numrows, numcols;
    gmatrix M;

    numrows = get_defint("number of rows   ", 0);
    numcols = get_defint("number of columns", 0);
    M = mod_init();

    pl_scan(&M->gens, numcols, numrows);
    setzerodegs(M, numrows);
    return (M);
}

gmatrix sparse_scan(void)
{
    int numrows, numcols;
    gmatrix M;

    numrows = get_defint("number of rows   ", 0);
    numcols = get_defint("number of columns", 0);
    M = mod_init();

    plSparse_scan(&M->gens, numcols, numrows);
    setzerodegs(M, numrows);
    return (M);
}

gmatrix ideal_scan(void)
{
    int numgens;
    gmatrix M;

    M = mod_init();
    numgens = get_int("number of generators");

    pl_scan(&M->gens, numgens, 1);
    setzerodegs(M, 1);
    return (M);
}

gmatrix poly_scan(int argc, char* argv[])
{
    gmatrix M;
    poly f;

    M = mod_init();
    f = rdPolyStr(argc, argv, 1);
    setzerodegs(M, 1);
    gmInsert(M, f);
    return (M);
}
