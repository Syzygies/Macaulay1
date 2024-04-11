/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "style.h"
#include "array.h"
#include "iring.h"
#include "vars.h"

// void dl_newall (dlist *result, int n, int size);
// void dl_new (dlist *result, int size);
// void dl_trans (dlist *result, dlist *a);
// void dl_concat (dlist *result, dlist *a, dlist *b);
// void dl_composite (dlist *result, dlist *a, dlist *b);
// void dl_addto (dlist *result, int n, dlist *a);
// int dl_max (dlist *dl);
// int dl_min (dlist *dl);
// void dlSpecSet (struct matches *p);
// boolean dlSpecial (char *s, int *val);
// int dlgetelem (char *s, int *first, int *second, dlist **dl);
// void dlConsume (dlist *dl, int argc, char *argv[], int len, int def);
// void dlVarConsume (dlist *dl, int argc, char *argv[]);
// void dlScan (dlist *dl, int len, int def);
// void dlVarScan (dlist *dl, int lo, int hi);
// boolean dlVarGet (dlist *dl, int lo, int hi);
// int /* returns number of characters actually displayed */ dlDisPart (FILE *fil, int elem, int repl);
// void dlDisplay (FILE *fil, dlist *dl, int rowsize);
// void dlIns (dlist *dl, int i, int n);

void dl_newall (dlist *result, int n, int size)
{
    int i;

    for (i=1; i<=size; i++)
        dl_insert(result, n);
}

void dl_new (dlist *result, int size)
{
    dl_newall(result, 0, size);
}

void dl_trans (dlist *result, dlist *a)
{
    int i;

    for (i=1; i<=length(a); i++)
        dl_insert(result, - DREF(*a, i));
}

void dl_concat (dlist *result, dlist *a, dlist *b)
{
    int i;

    for (i=1; i<=length(a); i++)
        dl_insert(result, DREF(*a, i));
    for (i=1; i<=length(b); i++)
        dl_insert(result, DREF(*b, i));
}

void dl_composite (dlist *result, dlist *a, dlist *b)
{
    int i;

    for (i=1; i<=length(a); i++)
        dl_insert(result, DREF(*b, DREF(*a, i)));
}

void dl_addto (dlist *result, int n, dlist *a)
{
    int i;

    for (i=1; i<=length(a); i++)
        dl_insert(result, n + DREF(*a, i));
}

int dl_max (dlist *dl)
{
    int n, i, a;

    if (length(dl) IS 0) return(0);
    n = DREF(*dl, 1);
    for (i=2; i<=length(dl); i++) {
        a = DREF(*dl, i);
        if (a > n)
            n = a;
    }
    return(n);
}

int dl_min (dlist *dl)
{
    int n, i, a;

    if (length(dl) IS 0) return(0);
    n = DREF(*dl, 1);
    for (i=2; i<=length(dl); i++) {
        a = DREF(*dl, i);
        if (a < n)
            n = a;
    }
    return(n);
}

#define DLCONT  0
#define DLRANGE 1
#define DLREPL  2
#define DLLIST  3
#define DLNONE  4

/* the following struct is used in scanning integer lists in order
 * to have special values for specific strings.  One calls "dlSpecSet"
 * with a pointer to such a struct (last entry in struct is: {"", 0}).
 * After calling dlConsume, etc... one then sets it back via
 * dlSetSpec(NULL).
 */

struct matches {
    char *matchname;
    int matchval;
};

struct matches varUse[4] = { {"w", RWTFCN},
    {"c", RCOMP},
    {"", 0}
};

struct matches *specelems;

void dlSpecSet (struct matches *p)
{
    if (p ISNT NULL)
        specelems = varUse;
    else
        specelems = p;
}

boolean dlSpecial (char *s, int *val)
{
    struct matches *p;
    if (specelems IS NULL)
        return(FALSE);
    for (p=specelems; p->matchname[0] ISNT '\0'; p++) {
        if (strcmp(s, p->matchname) IS 0)
        {
            *val = p->matchval;
            return(TRUE);
        }
    }
    return(FALSE);
}

int dlgetelem (char *s, int *first, int *second, dlist **dl)
{
    variable *p;
    char *t, ident[100];

    if (*s IS '\\') return(DLCONT);
    if (dlSpecial(s, first)) {
        *second = *first;
        return(DLRANGE);
    }

    /* now check to see if item is a matrix */
    if (canStartVar(*s) ISNT NULL) {
        t = s;
        getIdentifier(&t, ident);
        p = find_var(ident);
        if (p IS NULL) return(DLNONE);
        if (p->b_alias ISNT NULL)
            p = p->b_alias;
        if (is_a_module(p)) {
            *dl = &((VAR_MODULE(p))->degrees);
            return(DLLIST);
        }
    }

    *first = parseInt(&s);
    if (*s IS ':') {
        s++;
        *second = getInt(s);
        return(DLREPL);
    }
    if (*s IS '.') {
        s++;
        if (*s ISNT '.') {
            *second = *first-1;
            return(DLRANGE);
        }
        s++;
        *second = getInt(s);
        return(DLRANGE);
    }
    *second = *first;
    return(DLRANGE);
}

void dlConsume (dlist *dl, int argc, char *argv[], int len, int def)
/* dlist to be added to */
/* parameters to be consumed */
/* expected maximum length */
/* default value to place in every entry if argc is 0 */
{
    int i, j, l, n, kind;
    int first, second;
    dlist *partdl;

    i = 0;
    while (i < argc) {
        kind = dlgetelem(argv[i], &first, &second, &partdl);
        switch (kind) {
        case DLCONT:
            get_line(&argc, &argv);
            i = -1;
            break;
        case DLRANGE:
            for (j=first; j <=second; j++) {
                if (length(dl) IS len) return;
                dl_insert(dl, j);
            }
            break;
        case DLREPL:
            for (j=1; j<=second; j++) {
                if (length(dl) IS len) return;
                dl_insert(dl, first);
            }
            break;
        case DLLIST:
            for (j=1; j<=length(partdl); j++) {
                if (length(dl) IS len) return;
                dl_insert(dl, DREF(*partdl, j));
            }
            break;
        case DLNONE:
            break;
        }
        i++;
    }
    l = length(dl);
    if (l IS 0)
        n = def;
    else
        n = DREF(*dl, l);
    for (j=l+1; j<=len; j++)
        dl_insert(dl, n);
}

void dlVarConsume (dlist *dl, int argc, char *argv[])
{
    int i, j, kind, first, second;
    dlist *partdl;

    i = 0;
    while (i < argc) {
        kind = dlgetelem(argv[i], &first, &second, &partdl);
        switch (kind) {
        case DLCONT:
            get_line(&argc, &argv);
            i = -1;
            break;
        case DLRANGE:
            for (j=first; j<=second; j++) {
                dl_insert(dl, j);
            }
            break;
        case DLREPL:
            for (j=1; j<=second; j++) {
                dl_insert(dl, first);
            }
            break;
        case DLLIST:
            for (j=1; j<=length(partdl); j++) {
                dl_insert(dl, DREF(*partdl, j));
            }
            break;
        }
        i++;
    }
}

void dlScan (dlist *dl, int len, int def)
{
    int argc;
    char **argv;

    get_line(&argc, &argv);
    dlConsume(dl, argc, argv, len, def);
}

void dlVarScan (dlist *dl, int lo, int hi)
{
    int i, argc;
    char **argv;

    dl_init(dl);
    get_line(&argc, &argv);
    if (argc > 0)
        dlVarConsume(dl, argc, argv);
    else
        for (i=lo; i<=hi; i++)
            dl_insert(dl, i);
}

boolean dlVarGet (dlist *dl, int lo, int hi)
/* if <return> is typed, default = lo..hi */
{
    int i, d, argc;
    char **argv;

    dl_init(dl);
    get_line(&argc, &argv);
    if (argc > 0)
        dlVarConsume(dl, argc, argv);
    else
        for (i=lo; i<=hi; i++)
            dl_insert(dl, i);

    for (i=1; i<=length(dl); i++) {
        d = DREF(*dl, i);
        if ((d < lo) OR (d > hi)) {
            prerror("; error: integer out of bounds\n");
            return(FALSE);
        }
    }
    return(TRUE);
}

int /* returns number of characters actually displayed */ dlDisPart (FILE *fil, int elem, int repl)
{
    int sz;

    if (repl IS 1) {
        int_pprint(fil, elem, TRUE, FALSE);  /* T, F mean: print one,not+*/
        sz = int_size(elem, TRUE, FALSE);
    } else if (repl IS 2) {
        int_pprint(fil, elem, TRUE, FALSE);  /* T, F mean: print one,not+*/
        fprint(fil, " ");
        int_pprint(fil, elem, TRUE, FALSE);
        sz = 1 + 2*int_size(elem, TRUE, FALSE);
    } else {
        int_pprint(fil, elem, TRUE, FALSE);  /* T, F mean: print one,not+*/
        fprint(fil, ":");
        int_pprint(fil, repl, TRUE, FALSE);
        sz = 1 + int_size(elem, TRUE, FALSE) + int_size(repl, TRUE, FALSE);
    }
    return(sz);
}

extern int prcomment;

void dlDisplay (FILE *fil, dlist *dl, int rowsize)
{
    int len, sz, this, num, i;

    if (prcomment > 0) rowsize -= 2;
    len = length(dl);
    sz = 0;
    if (len IS 0) return;
    this = DREF(*dl, 1);
    num = 1;
    for (i=2; i<=len; i++)
        if (this IS DREF(*dl, i))
            num++;
        else  {
            if (sz >= rowsize-5) {
                fprint(fil, "\\");
                fnewline(fil);
                fprint(fil, "\n");
                sz = -1;
            }
            sz += 1 + dlDisPart(fil, this, num);
            fprint(fil, " ");
            this = DREF(*dl, i);
            num = 1;
        }
    dlDisPart(fil, this, num);
    fprint(fil, "\n");
} /* mod 24feb89 DB */

void dlIns (dlist *dl, int i, int n)
/* insert the integer "n" in the "i"th spot */
{
    int len, j;
    int *this, *last;

    len = length(dl);
    last = &DREF(*dl, len);
    dl_insert(dl, *last);
    for (j=len-1; j>=i; j--) {
        this = &DREF(*dl, j);
        *last = *this;
        last = this;
    }
    *last = n;
}
