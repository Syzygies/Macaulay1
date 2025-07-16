// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include "shared.h"
#include "gm_dlist.h"
#include "vars.h"
#include "parse.h"
#include "shell.h"
#include "monitor.h"
#include "array.h"
#include "printf.h"
#include "input.h"
#include "human_io.h"
#include "set.h"

// Inline functions for macros
static inline gmatrix VAR_MODULE(variable* p)
{
    return var_get_module(p);
}

// Inline functions for compatibility
static inline void dl_insert(dlist* dl, int i)
{
    dlist_insert(dl, i);
}

static inline int DREF(dlist dl, int i)
{
    return dlist_ref(&dl, i);
}

static inline void dl_init(dlist* dl)
{
    dlist_init(dl);
}

void dl_newall(dlist* result, int n, int size)
{
    int i;

    for (i = 1; i <= size; i++)
        dl_insert(result, n);
}

void dl_new(dlist* result, int size)
{
    dl_newall(result, 0, size);
}

void dl_trans(dlist* result, dlist* a)
{
    int i;

    for (i = 1; i <= length(a); i++)
        dl_insert(result, -DREF(*a, i));
}

void dl_concat(dlist* result, dlist* a, dlist* b)
{
    int i;

    for (i = 1; i <= length(a); i++)
        dl_insert(result, DREF(*a, i));
    for (i = 1; i <= length(b); i++)
        dl_insert(result, DREF(*b, i));
}

void dl_composite(dlist* result, dlist* a, dlist* b)
{
    int i;

    for (i = 1; i <= length(a); i++)
        dl_insert(result, DREF(*b, DREF(*a, i)));
}

void dl_addto(dlist* result, int n, dlist* a)
{
    int i;

    for (i = 1; i <= length(a); i++)
        dl_insert(result, n + DREF(*a, i));
}

int dl_max(dlist* dl)
{
    int n, i, a;

    if (length(dl) == 0)
        return (0);
    n = DREF(*dl, 1);
    for (i = 2; i <= length(dl); i++)
    {
        a = DREF(*dl, i);
        if (a > n)
            n = a;
    }
    return (n);
}

int dl_min(dlist* dl)
{
    int n, i, a;

    if (length(dl) == 0)
        return (0);
    n = DREF(*dl, 1);
    for (i = 2; i <= length(dl); i++)
    {
        a = DREF(*dl, i);
        if (a < n)
            n = a;
    }
    return (n);
}

constexpr int DLCONT = 0;
constexpr int DLRANGE = 1;
constexpr int DLREPL = 2;
constexpr int DLLIST = 3;
constexpr int DLNONE = 4;

// the following struct is used in scanning integer lists in order
// to have special values for specific strings.  One calls "dlSpecSet"
// with a pointer to such a struct (last entry in struct is: {"", 0}).
// After calling dlConsume, etc... one then sets it back via
// dlSetSpec(NULL).

matches varUse[4] = { { "w", RWTFCN }, { "c", RCOMP }, { "", 0 } };

matches* specelems;

void dlSpecSet(matches* p)
{
    if (p != NULL)
        specelems = p; // C23: Fixed logic - use provided matches
    else
        specelems = varUse; // Use default when NULL
}

boolean dlSpecial(char* s, int* val)
{
    matches* p;
    if (specelems == NULL)
        return (0);
    for (p = specelems; p->name[0] != '\0'; p++)
    {
        if (strcmp(s, p->name) == 0)
        {
            *val = p->used;
            return (1);
        }
    }
    return (0);
}

int dlgetelem(char* s, int* first, int* second, dlist** dl)
{
    variable* p;
    char* t, ident[100];

    if (*s == '\\')
        return (DLCONT);
    if (dlSpecial(s, first))
    {
        *second = *first;
        return (DLRANGE);
    }

    // now check to see if item is a matrix
    if (canStartVar(*s))
    {
        t = s;
        getIdentifier(&t, ident);
        p = find_var(ident);
        if (p == NULL)
            return (DLNONE);
        if (p->b_alias != NULL)
            p = p->b_alias;
        if (is_a_module(p))
        {
            *dl = &((VAR_MODULE(p))->degrees);
            return (DLLIST);
        }
    }

    *first = parseInt(&s);
    if (*s == ':')
    {
        s++;
        *second = getInt(s);
        return (DLREPL);
    }
    if (*s == '.')
    {
        s++;
        if (*s != '.')
        {
            *second = *first - 1;
            return (DLRANGE);
        }
        s++;
        *second = getInt(s);
        return (DLRANGE);
    }
    *second = *first;
    return (DLRANGE);
}

void dlConsume(dlist* dl, int argc, char* argv[], int len, int def)
{
    int i, j, l, n, kind;
    int first, second;
    dlist* partdl;

    i = 0;
    while (i < argc)
    {
        kind = dlgetelem(argv[i], &first, &second, &partdl);
        switch (kind)
        {
        case DLCONT:
            get_line(&argc, &argv);
            i = -1;
            break;
        case DLRANGE:
            for (j = first; j <= second; j++)
            {
                if (length(dl) == len)
                    return;
                dl_insert(dl, j);
            }
            break;
        case DLREPL:
            for (j = 1; j <= second; j++)
            {
                if (length(dl) == len)
                    return;
                dl_insert(dl, first);
            }
            break;
        case DLLIST:
            for (j = 1; j <= length(partdl); j++)
            {
                if (length(dl) == len)
                    return;
                dl_insert(dl, DREF(*partdl, j));
            }
            break;
        case DLNONE:
            break;
        }
        i++;
    }
    l = length(dl);
    if (l == 0)
        n = def;
    else
        n = DREF(*dl, l);
    for (j = l + 1; j <= len; j++)
        dl_insert(dl, n);
}

void dlVarConsume(dlist* dl, int argc, char* argv[])
{
    int i, j, kind, first, second;
    dlist* partdl;

    i = 0;
    while (i < argc)
    {
        kind = dlgetelem(argv[i], &first, &second, &partdl);
        switch (kind)
        {
        case DLCONT:
            get_line(&argc, &argv);
            i = -1;
            break;
        case DLRANGE:
            for (j = first; j <= second; j++)
            {
                dl_insert(dl, j);
            }
            break;
        case DLREPL:
            for (j = 1; j <= second; j++)
            {
                dl_insert(dl, first);
            }
            break;
        case DLLIST:
            for (j = 1; j <= length(partdl); j++)
            {
                dl_insert(dl, DREF(*partdl, j));
            }
            break;
        }
        i++;
    }
}

void dlScan(dlist* dl, int len, int def)
{
    int argc;
    char** argv;

    get_line(&argc, &argv);
    dlConsume(dl, argc, argv, len, def);
}

void dlVarScan(dlist* dl, int lo, int hi)
{
    int i, argc;
    char** argv;

    dl_init(dl);
    get_line(&argc, &argv);
    if (argc > 0)
        dlVarConsume(dl, argc, argv);
    else
        for (i = lo; i <= hi; i++)
            dl_insert(dl, i);
}

boolean dlVarGet(dlist* dl, int lo, int hi)
{
    int i, d, argc;
    char** argv;

    dl_init(dl);
    get_line(&argc, &argv);
    if (argc > 0)
        dlVarConsume(dl, argc, argv);
    else
        for (i = lo; i <= hi; i++)
            dl_insert(dl, i);

    for (i = 1; i <= length(dl); i++)
    {
        d = DREF(*dl, i);
        if ((d < lo) || (d > hi))
        {
            prerror("; error: integer out of bounds\n");
            return (0);
        }
    }
    return (1);
}

int // returns number of characters actually displayed
dlDisPart(FILE* fil, int elem, int repl)
{
    int sz;

    if (repl == 1)
    {
        int_pprint(fil, elem, 1, 0); // T, F mean: print one,not+
        sz = int_size(elem, 1, 0);
    }
    else if (repl == 2)
    {
        int_pprint(fil, elem, 1, 0); // T, F mean: print one,not+
        fprint(fil, " ");
        int_pprint(fil, elem, 1, 0);
        sz = 1 + 2 * int_size(elem, 1, 0);
    }
    else
    {
        int_pprint(fil, elem, 1, 0); // T, F mean: print one,not+
        fprint(fil, ":");
        int_pprint(fil, repl, 1, 0);
        sz = 1 + int_size(elem, 1, 0) + int_size(repl, 1, 0);
    }
    return (sz);
}

void dlDisplay(FILE* fil, dlist* dl, int rowsize)
{
    int len, sz, this, num, i;

    if (prcomment > 0)
        rowsize -= 2;
    len = length(dl);
    sz = 0;
    if (len == 0)
        return;
    this = DREF(*dl, 1);
    num = 1;
    for (i = 2; i <= len; i++)
        if (this == DREF(*dl, i))
            num++;
        else
        {
            if (sz >= rowsize - 5)
            {
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
} // mod 24feb89 DB

void dlIns(dlist* dl, int i, int n)
{
    int len, j;
    int* this, * last;

    len = length(dl);
    last = (int*)ref((array*)dl, len);
    dl_insert(dl, *last);
    for (j = len - 1; j >= i; j--)
    {
        this = (int*)ref((array*)dl, j);
        last = (int*)ref((array*)dl, j + 1);
        *last = *this;
    }
    *(int*)ref((array*)dl, i) = n;
}

void dl_set(dlist* dl, int i, int val)
{
    *(int*)ref((array*)dl, i) = val;
}
