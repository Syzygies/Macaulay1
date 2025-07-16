// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <stdio.h>  // For printf()
#include <string.h> // For strlen(), strcpy(), sprintf()
#include <ctype.h>  // For isalpha(), isdigit()
#include <setjmp.h> // For jmp_buf type required by parse.h
#include <limits.h> // For INT_MAX
#include "shared.h"
#include "vars.h"
#include "array.h"    // For init_array(), ref_array(), ins_array(), length()
#include "monitor.h"  // For print(), printnew(), prerror(), fnewline(), fprint()
#include "ring.h"     // For rgDisplay(), nvars()
#include "gmatrix.h"  // For nrows()
#include "human_io.h" // For pl_pprint()
#include "parse.h"    // For parseInt()
#include "stash.h"    // For gimmy()
#include "mem.h"      // For initializeGB(), get_small()
#include "input.h"    // For getInt()
#include "boxprocs.h" // For isGBModule(), recalc_boxes(), vtable dispatch
#include "poly.h"     // For getPolyId()
#include "set.h"      // For linesize
#include "shell.h"    // For outfile
#include "generic.h"  // For variable type constants, type_names[], message_proc()
#include "cmds.h"     // For vget_rgmod()
#include "ed.h"       // For editvar

// Inline functions to access variable union fields
static inline int VAR_INT(variable *p)
{
    return var_get_int(p);
}

static inline ring VAR_RING(variable *p)
{
    return (ring)p->value;
}

// Inline function for accessing variables by number
static inline variable *VREF(int n)
{
    return (variable *)ref(&var_list, n);
}

// Constants for whichkind()
enum
{
    VKNAME = 1,
    VKINDEX = 2
};

// Variable existence states - REMOVED, MAINVAR and PARTVAR come from shared.h

// Variable type constants now come from generic.h

// Garbage collection states
static const int DEAD = 0;
static const int KEEP = 1;

// Message constants for varb_kill and varb_type - defined in generic.h

// type_names[] and message_proc() are declared in generic.h

// Inline functions for variable operations using vtable dispatch
static inline void varb_kill(variable *pv)
{
    const object_vtable *vtable = get_vtable(pv->type);
    if (vtable && vtable->kill)
    {
        vtable->kill(pv->value);
    }
}

static inline void varb_type(variable *pv)
{
    variable *target = (pv->b_alias == NULL) ? pv : pv->b_alias;
    const object_vtable *vtable = get_vtable(target->type);
    if (vtable && vtable->get_type)
    {
        vtable->get_type(target->value);
    }
}

array var_list;
int last_var;    // integer # of last variable made
variable *lastv; // pointer to this variable

variable *current_ring;
variable *cur_ring; // current ring: used in garbage collection
// editvar is declared in ed.h

boolean allocStr = TRUE; /* hack: true means alloc. space for
                            char. strings coming in.  If false, string
                            is supposed to be static */

void rgDebug(variable *R)
{
    ring RR;
    RR = VAR_RING(R);
    printf("number of variables = %d\n", nvars(RR));
}

// The following are generic procedures for various types.

void vrg_type(ring R)
{
    rgDisplay(outfile, R);
}

void vmod_type(gmatrix M)
{
    if (M == NULL)
        return;
    pl_pprint(outfile, &(M->gens), nrows(M), linesize);
}

void int_type(int n)
{
    fnewline(outfile);
    fprint(outfile, "%d\n", n);
}

boolean canStartVar(char c)
{
    return ((isalpha(c) || (c == '\'') || (c == '@') || (c == '_') || (c == '{')));
}

boolean getIdentifier(char **str, char *ident)
{
    char *t;
    char c;
    int n;
    boolean hasbrace;

    if (!canStartVar(**str))
        return (FALSE);
    t = ident;
    hasbrace = FALSE;
    if (**str == '{')
    {
        hasbrace = TRUE;
        (*str)++;
    }
    c = *(*str)++;

    while ((isalpha(c) || (c == '\'') || (c == '@') || (c == '_') || (isdigit(c)) || (c == '.')))
    {

        if (c == '.')
        {
            if ((**str) == '.')
                break;
            n = parseInt(str);
            sprintf(t, ".%d", n);
            if (hasbrace)
            {
                if ((**str) != '}')
                    prerror("; missing '}' for identifier %s\n", ident);
                else
                    (*str)++;
            }
            return (TRUE);
        }

        *t++ = c;
        c = *(*str)++;
    }
    *t = '\0';
    if (hasbrace)
    {
        if (c != '}')
        {
            prerror("; missing '}' for identifier %s\n", ident);
            (*str)--;
        }
    }
    else
        (*str)--;
    return (TRUE);
}

int getIdVal(const char *name)
{
    variable *p;

    p = find_var(name);
    if (p == NULL)
        return (0);
    if (p->type != VINT)
    {
        prerror("; variable isn't an integer\n");
        return (0);
    }
    return (VAR_INT(p));
}

poly getPolyId(const char *s, int comp)
{
    variable *p;
    int n;
    gmatrix M;

    p = find_var(s);
    if (p == NULL)
        return (NULL);
    if (p->type == VINT)
    {
        n = VAR_INT(p);
        return (p_intpoly(n, comp));
    }
    else
    {
        M = vget_rgmod(s); // returns NULL if problem
        if (M == NULL)
            return (NULL);
        return (gm_elem(M, 1, 1, comp));
    }
}

// The following routines form the external interface to deal with
// variables.

void init_vars(void)
{
    init_array(&var_list, sizeof(variable));
    last_var = 0;
    lastv = NULL;
    current_ring = NULL;
}

// whichkind returns : VKNAME, or VKINDEX, depending on
// the Variable Kind.  In the last case, the int. value is placed
// in "n".  In the second case, the '.' in "name" is changed to '\0',
// and the pointer to this exact location is placed in "p" (to be
// able to change it back later), and partial name is put in "partname"

int whichkind(char *name, int *n, char **p, char **partname)
{
    *p = (char *)name;
    while ((**p != '\0') && (**p != '.'))
        (*p)++;
    if (**p == '\0')
    {
        return (VKNAME);
    }
    **p = '\0';
    *partname = (*p) + 1;
    return (VKINDEX);
}

variable *select(variable *p, const char *partname)
{
    register int j;
    register variable *q;

    j = p->var_num;
    for (j++; j <= last_var; j++)
    {
        q = VREF(j);
        if (q->exists != PARTVAR)
            return (NULL);
        if (strcmp(q->name, partname) == 0)
            return (q);
    }
    return (NULL);
}

variable *fvar(const char *name, const char *partname)
{
    register int j;
    register variable *p;

    for (j = last_var; j >= 1; j--)
    {
        p = VREF(j);
        if (p->exists != MAINVAR)
            continue;
        if (strcmp(p->name, name) == 0)
        {
            if (partname == NULL)
                return (p);
            return (select(p, partname));
        }
    }
    return (NULL);
}

variable *newvar(const char *name, int isMain, int type, variable *bring)
{
    register variable *result;

    last_var++;
    if (last_var > length(&var_list))
    {
        result = (variable *)ins_array(&var_list);
    }
    else
    {
        result = VREF(last_var);
    }
    if ((allocStr) && (strlen(name) > 0))
    {
        size_t len = strlen(name) + 1;
        if (len > INT_MAX)
        {
            prerror("; variable name too long\n");
            return NULL;
        }
        result->name = get_small((int)len);
        strcpy(result->name, name);
    }
    else
    {
        // When allocStr is false, name is expected to be static
        // Use intermediate void* cast to suppress warning
        result->name = (char *)(void *)(uintptr_t)name;
    }
    result->type = type;
    result->exists = (char)isMain;
    if (isMain == PARTVAR)
        lastv->b_next = result;
    result->b_ring = bring;
    result->b_next = NULL;
    result->b_alias = NULL;
    result->intval = 0;
    result->value = NULL;
    result->var_num = last_var;
    lastv = result;
    return (result);
}

// find_var    finds the variable with name "name", if it exists.  If
// it doesn't exist, an error message is generated, and NULL is
// returned.  Otherwise, pointer to the variable is returned.
// Format of variables:
// i.   name only, e.g.   m, this_mat
// ii.  name, and index:    m.2, this_mat.3, m.(0+1)

variable *find_var(const char *name)
{
    register int kind;
    register variable *result;
    char *q, *partname, *inname, ident[100];
    int n;
    variable *mainres;
    char s[100];
    char namecopy[256];

    // Make a copy to avoid modifying const string
    strncpy(namecopy, name, sizeof(namecopy) - 1);
    namecopy[sizeof(namecopy) - 1] = '\0';
    inname = namecopy;
    if (!getIdentifier(&inname, ident))
    {
        prerror("; bad identifier: %s\n", name);
        return (NULL);
    }
    else if (*inname != '\0')
    {
        prerror("; bad identifier: %s\n", name);
        return (NULL);
    }

    kind = whichkind(ident, &n, &q, &partname);
    switch (kind)
    {
    case VKNAME:
        result = fvar(ident, NULL);
        break;
    case VKINDEX:
        result = fvar(ident, partname);
        if (result == NULL)
        { // try to find ring'zero
            mainres = fvar(ident, NULL);
            if ((mainres != NULL) && (mainres->b_ring != NULL))
            {
                sprintf(s, "%s'zero", mainres->b_ring->name);
                result = fvar(s, NULL);
            }
        }
        *q = '.';
        break;
    }
    if (result == NULL)
        prerror("; var %s doesn't exist\n", ident);
    return (result);
}

// make_var creates a variable: either a main var, or a partial
// variable.  If it is a main variable, and the var. already exists,
// the old copy is removed first. "name" shouldn't be an indexed
// var, or an absolute var. number: if so
// NULL is returned (with error message).  Otherwise, a variable node
// is created, and its name, type, base ring fields are set with the
// given parameters.  Its b_next field is set to NULL. The b_next field
// of the previous last variable (lastv) is modified to point to this
// variable if "isMain" = PARTVAR, otherwise it isn't modified.

variable *make_var(const char *name, int isMain, int type, variable *bring)
{
    register int kind;
    int n;
    char *q, *partname, *inname, ident[100];
    register variable *result, *p;
    char namecopy[256];

    if (isMain == PARTVAR)
    { // partial vars are special case...
        result = newvar(name, PARTVAR, type, bring);
        return (result);
    }

    // Make a copy to avoid modifying const string
    strncpy(namecopy, name, sizeof(namecopy) - 1);
    namecopy[sizeof(namecopy) - 1] = '\0';
    inname = namecopy;
    if (!getIdentifier(&inname, ident))
    {
        prerror("; bad identifier: %s\n", name);
        return (NULL);
    }
    else if (*inname != '\0')
    {
        prerror("; bad identifier: %s\n", name);
        return (NULL);
    }

    kind = whichkind(ident, &n, &q, &partname);
    if (kind != VKNAME)
    {
        prerror("; can't create an indexed variable\n");
        return (NULL);
    }

    p = fvar(ident, NULL);
    if (p != NULL)
    {
        // kill this var, and use it anyway
        rem_var(p);
    }

    result = newvar(ident, MAINVAR, type, bring);
    return (result);
}

// vrg_install        Installs a new ring as current ring.

void vrg_install(variable *p)
{
    if ((p != NULL) && (p != current_ring) && (VAR_RING(p) != NULL))
    {
        current_ring = p; // needed for rgInstall
        rgInstall(VAR_RING(p));
    }
    else if (p != current_ring)
        current_ring = NULL;
}

// is_a_module    Returns TRUE if "p"s value is indeed a module,
// else FALSE is returned.

boolean is_a_module(variable *p)
{
    register int t;

    t = p->type;
    return ((t == VMODULE) || (t == VCOLLECT) || (t == VRES) || (t == VNRES) || (t == VSTD) ||
            (t == VISTD));
}

boolean is_alias(variable *p)
{
    register int t;

    t = p->type;
    return ((t == VEMIT) || (t == VSTDEMIT) || (t == VLIFT));
}

// set_value        sets p->value.
// connect_vars    sets p->b_next: needs to modify its count then

void set_value(variable *p, ADDRESS val)
{
    p->value = val;
}

void set_alias(variable *p, variable *aliasp)
{
    p->b_alias = aliasp;
}

void connect_vars(variable *p, variable *b_next)
{
    p->b_next = b_next;
}

void putInternals(variable *p)
{
    print("  ");
    if (p->b_ring == NULL)
        print("  -- ");
    else
        print("%4d ", p->b_ring->var_num);
    if (p->b_next == NULL)
        print("  -- ");
    else
        print("%4d ", p->b_next->var_num);
    if (p->exists == MAINVAR)
        print("  main   ");
    else if (p->exists == PARTVAR)
        print("  part   ");
    else
        print("  removed");
    print("\n");
}

void listvars_cmd(int argc, char *argv[])
{
    variable *p;
    int i;

    if (last_var == 0)
    {
        newline();
        print("no variables defined\n");
        return;
    }
    newline();
    print("  var#   name       type           ");
    if (argc >= 3)
        print("  ring  next status\n");
    else
        print("\n");
    newline();
    print("  ------------------\n");
    for (i = 1; i <= last_var; i++)
    {
        p = VREF(i);

        if (argc == 1)
        {
            char *s;
            for (s = p->name; *s != '\0' && *s != '\''; ++s)
                ;
            if (*s != '\0' && strcmp("'zero", s) == 0)
                continue;
        } // 8/25/93 DAB

        if ((p->exists == MAINVAR) || ((argc >= 3) && (p->exists == REMOVED)))
        {
            newline();
            print("%6d   %-10s %-14s", p->var_num, p->name, type_names[p->type]);
            if (argc >= 3)
                putInternals(p);
            else
                print("\n");
        };
        if ((argc >= 2) && (p->exists == PARTVAR))
        {
            if ((argc >= 3) || (*(p->name) != '\0'))
                newline();
            print("%6d     %-8s %-14s", p->var_num, p->name, type_names[p->type]);
            if (argc >= 3)
                putInternals(p);
            else
                print("\n");
        };
    }
}

// the following routines are responsible for killing variables and
// their associated storage.

void mark(variable *p)
{
    if (p == NULL)
        return;
    if (p->garbage == KEEP)
        return;
    p->garbage = KEEP;
    mark(p->b_ring);
    mark(p->b_next);
    mark(p->b_alias);
    if (is_alias(p))
        mark((variable *)p->value);
}

void mark_vars(void)
{
    register variable *p;
    register int i;

    for (i = 1; i <= last_var; i++)
    {
        p = VREF(i);
        p->garbage = DEAD;
    }
    for (i = 1; i <= last_var; i++)
    {
        p = VREF(i);
        if (p->exists != REMOVED)
            mark(p);
    }
}

void cp_varnode(int new_idx, variable *pnew, variable *pold)
{
    register variable *p;
    register int i;

    *pnew = *pold;
    pnew->var_num = new_idx;
    pold->garbage = DEAD;
    if (cur_ring == pold)
        cur_ring = pnew;
    if (editvar == pold)
        editvar = pnew;
    for (i = 1; i <= last_var; i++)
    {
        p = VREF(i);
        if (p->b_ring == pold)
            p->b_ring = pnew;
        if (p->b_next == pold)
            p->b_next = pnew;
        if (p->b_alias == pold)
            p->b_alias = pnew;
        if ((is_alias(p)) && (p->value == (ADDRESS)pold))
            p->value = (ADDRESS)pnew;
    }
}

variable *next_used(int n)
{
    register int i;
    register variable *p;

    for (i = n + 1; i <= last_var; i++)
    {
        p = VREF(i);
        if (p->garbage == KEEP)
            return (p);
    }
    return (NULL);
}

void contract_vars(void)
{
    register int first_free;
    register variable *p;
    register variable *pold;

    if (last_var == 0)
        return;
    for (first_free = 1; first_free <= last_var; first_free++)
    {
        p = VREF(first_free);
        if (p->garbage == DEAD)
            break;
    }
    if (p->garbage == KEEP)
        return;
    while ((pold = next_used(first_free)) != NULL)
    {
        p = VREF(first_free);
        cp_varnode(first_free, p, pold);
        first_free++;
    }
    last_var = first_free - 1; // last one with actual data
    lastv = p;                 // hasn't been changed since set
}

void garb_collect(void)
{
    register int i;
    register variable *p;

    mark_vars();
    if (current_ring == NULL)
        cur_ring = NULL;
    else if (current_ring->garbage == DEAD)
        cur_ring = NULL;
    else
        cur_ring = current_ring;
    if ((editvar != NULL) && (editvar->exists == REMOVED))
        editvar = NULL;
    for (i = last_var; i >= 1; i--)
    {
        p = VREF(i);
        if (p->garbage == DEAD)
        {
            vrg_install(p->b_ring);
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
#endif
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
            varb_kill(p);
#pragma GCC diagnostic pop
#ifdef __clang__
#pragma clang diagnostic pop
#endif
            p->value = NULL;
        }
    }
    contract_vars();
    vrg_install(cur_ring);
}

void rem_var(variable *p)
{
    variable *q;
    int i;

    p->exists = REMOVED;
    for (i = p->var_num + 1; i <= last_var; i++)
    {
        q = VREF(i);
        if (q->exists == PARTVAR)
            q->exists = REMOVED;
        else
            break;
    }
}

void kill_cmd(int argc, char *argv[])
{
    variable *p;
    int i;

    if (argc == 1)
    {
        printnew("kill <var1> ... <var n>\n");
        return;
    }
    for (i = 1; i < argc; i++)
    {
        p = find_var(argv[i]);
        if (p == NULL)
            continue;
        rem_var(p);
    }
}

void spare_cmd(int argc, char *argv[])
{
    // kills all @ vars not excepted
    variable *p;
    int i, j;
    char at[8], *s;

    sprintf(at, "@%02d@", cur_voice);
    for (i = 1; i <= last_var; ++i)
    {
        p = VREF(i);
        for (s = p->name; *s != '\0' && *s != '\''; ++s)
            ;
        if (p->exists != MAINVAR || strncmp(at, p->name, 4) != 0 ||
            (*s != '\0' && strcmp("'zero", s) == 0))
            continue;
        for (j = 1; j < argc; ++j)
        {
            if (strcmp(argv[j], p->name) == 0)
                break;
        }
        if (j == argc)
            rem_var(p);
    }
} // 8/25/93 DAB

void type_cmd(int argc, char *argv[])
{
    variable *p;

    if (argc != 2)
    {
        printnew("type <var>\n");
        return;
    }
    if ((p = find_var(argv[1])) == NULL)
        return;
    if (p->b_ring != NULL)
        vrg_install(p->b_ring);
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
#endif
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
    varb_type(p);
#pragma GCC diagnostic pop
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}
