/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "vars.h"

// void rgDebug (variable *R);
// void vrg_type (ring R);
// void vmod_type (gmatrix M);
// void int_type (int n);
// boolean canStartVar (char c);
// boolean getIdentifier (char **str, char *ident);
// int getIdVal (char *name);
// poly getPolyId (char *s, int comp);
// void init_vars ();
// int whichkind (char *name, int *n, char **p, char **partname);
// variable * select (variable *p, char *partname);
// variable *fvar (char *name, char *partname);
// variable *newvar (char *name, int isMain, int type, variable *bring);
// variable *find_var (char *name);
// variable *make_var (char *name, int isMain, int type, variable *bring);
// void vrg_install (variable *p);
// boolean is_a_module (variable *p);
// boolean is_alias (variable *p);
// void set_value (variable *p, ADDRESS val);
// void set_alias (variable *p, variable *aliasp);
// void connect_vars (variable *p, variable *b_next);
// void putInternals (variable *p);
// void listvars_cmd (int argc, char *argv[]);
// void mark (variable *p);
// void mark_vars ();
// void cp_varnode (int new, variable *pnew, variable *pold);
// variable *next_used (int n);
// void contract_vars ();
// void garb_collect ();
void rem_var (variable *p);
// void kill_cmd (int argc, char *argv[]);
// void spare_cmd (int argc, char *argv[]);
// void type_cmd (int argc, char *argv[]);

array var_list;
int last_var;      /* integer # of last variable made */
variable *lastv;   /* pointer to this variable */

variable *current_ring;
variable *cur_ring;    /* current ring: used in garbage collection*/
extern variable *editvar;  /* needed here for garbage collection */

boolean allocStr = TRUE;   /* hack: true means alloc. space for
                   char. strings coming in.  If false, string
                   is supposed to be static */

void rgDebug (variable *R)
{
    ring RR;
    RR = VAR_RING(R);
    printf("number of variables = %d\n", nvars(RR));
}

/*
 *  The following are generic procedures for various types.
 */

void vrg_type (ring R)
{
    rgDisplay(outfile, R);
}

void vmod_type (gmatrix M)
{
    if (M IS NULL) return;
    pl_pprint(outfile, &(M->gens), nrows(M), linesize);
}

void int_type (int n)
{
    fnewline(outfile);
    fprint(outfile, "%d\n", n);
}

boolean canStartVar (char c)
{
    return((isalpha(c) OR (c IS '\'') OR (c IS '@') OR (c IS '_')
            OR (c IS '{')));
}

boolean getIdentifier (char **str, char *ident)
{
    char *t;
    char  c;
    int n;
    boolean hasbrace;

    if (NOT canStartVar(**str)) return(FALSE);
    t = ident;
    hasbrace = FALSE;
    if (**str IS '{') {
        hasbrace = TRUE;
        (*str)++;
    }
    c = *(*str)++;

    while ((isalpha(c) OR (c IS '\'') OR (c IS '@') OR (c IS '_')
            OR (isdigit(c)) OR (c IS '.'))) {

        if (c IS '.') {
            if ((**str) IS '.') break;
            n = parseInt(str);
            sprintf(t, ".%d", n);
            if (hasbrace) {
                if ((**str) ISNT '}')
                    prerror("; missing '}' for identifier %s\n", ident);
                else
                    (*str)++;
            }
            return(TRUE);
        }

        *t++ = c;
        c = *(*str)++;
    }
    *t = '\0';
    if (hasbrace) {
        if (c ISNT '}') {
            prerror("; missing '}' for identifier %s\n", ident);
            (*str)--;
        }
    } else
        (*str)--;
    return(TRUE);
}

int getIdVal (char *name)
{
    variable *p;

    p = find_var(name);
    if (p IS NULL) return(0);
    if (p->type ISNT VINT) {
        prerror("; variable isn't an integer\n");
        return(0);
    }
    return(VAR_INT(p));
}

poly getPolyId (char *s, int comp)
{
    variable *p;
    int n;
    gmatrix M;
    poly p_intpoly();
    poly gm_elem();

    p = find_var(s);
    if (p IS NULL) return(NULL);
    if (p->type IS VINT) {
        n = VAR_INT(p);
        return(p_intpoly(n,comp));
    } else {
        M = vget_rgmod(s); /* returns NULL if problem */
        if (M IS NULL) return(NULL);
        return(gm_elem(M,1,1,comp));
    }
}

/*
 *  The following routines form the external interface to deal with
 *  variables.
 *
 */

void init_vars ()
{
    init_array(&var_list, sizeof(variable));
    last_var = 0;
    lastv = NULL;
    current_ring = NULL;
}

/*
 *  whichkind returns : VKNAME, or VKINDEX, depending on
 *  the Variable Kind.  In the last case, the int. value is placed
 *  in "n".  In the second case, the '.' in "name" is changed to '\0',
 *  and the pointer to this exact location is placed in "p" (to be
 *  able to change it back later), and partial name is put in "partname"
 */

int whichkind (char *name, int *n, char **p, char **partname)
{
#pragma unused(n)
    *p = name;
    while ((**p ISNT '\0') AND (**p ISNT '.'))
        (*p)++;
    if (**p IS '\0') {
        return(VKNAME);
    }
    **p = '\0';
    *partname = (*p)+1;
    return(VKINDEX);
}

variable * select (variable *p, char *partname)
{
    int j;
    variable *q;

    j = p->var_num;
    for (j++; j<=last_var; j++) {
        q = VREF(j);
        if (q->exists ISNT PARTVAR)
            return(NULL);
        if (strcmp(q->name, partname) IS 0)
            return(q);
    }
    return(NULL);
}

variable *fvar (char *name, char *partname)
{
    int j;
    variable *p;

    for (j=last_var; j>= 1; j--) {
        p = VREF(j);
        if (p->exists ISNT MAINVAR) continue;
        if (strcmp(p->name, name) IS 0) {
            if (partname IS NULL) return(p);
            return(select(p, partname));
        }
    }
    return(NULL);
}

variable *newvar (char *name, int isMain, int type, variable *bring)
/* MAINVAR if variable is main, PARTVAR if partial */
{
    variable *result;

    last_var++;
    if (last_var > length(&var_list)) {
        result = (variable *) ins_array(&var_list);
    } else {
        result = VREF(last_var);
    }
    if ((allocStr) AND (strlen(name) > 0)) {
        result->name = get_small(strlen(name)+1);
        strcpy(result->name, name);
    } else
        result->name = name;
    result->type = type;
    result->exists = isMain;
    if (isMain IS PARTVAR)
        lastv->b_next = result;
    result->b_ring = bring;
    result->b_next = NULL;
    result->b_alias = NULL;
    result->intval = 0;
    result->value = NULL;
    result->var_num = last_var;
    lastv = result;
    return(result);
}

/*
 *  find_var    finds the variable with name "name", if it exists.  If
 * it doesn't exist, an error message is generated, and NULL is
 * returned.  Otherwise, pointer to the variable is returned.
 * Format of variables:
 *      i.   name only, e.g.    m, this_mat
 *      ii.  name, and index:   m.2, this_mat.3, m.(0+1)
 */

variable *find_var (char *name)
{
    int kind;
    variable *result;
    char *q, *partname, *inname, ident[100];
    int n;
    variable *mainres;
    char s[100];

    inname = name;
    if (NOT getIdentifier(&inname, ident)) {
        prerror("; bad identifier: %s\n", name);
        return(NULL);
    } else if (*inname ISNT '\0') {
        prerror("; bad identifier: %s\n", name);
        return(NULL);
    }

    kind = whichkind(ident, &n, &q, &partname);
    switch (kind) {
    case VKNAME :
        result = fvar(ident, NULL);
        break;
    case VKINDEX :
        result = fvar(ident, partname);
        if (result IS NULL) {  /* try to find ring'zero */
            mainres = fvar(ident, NULL);
            if ((mainres ISNT NULL) AND (mainres->b_ring ISNT NULL)) {
                sprintf(s, "%s'zero", mainres->b_ring->name);
                result = fvar(s, NULL);
            }
        }
        *q = '.';
        break;
    }
    if (result IS NULL) prerror("; var %s doesn't exist\n", ident);
    return(result);
}

/*
 *      make_var creates a variable: either a main var, or a partial
 * variable.  If it is a main variable, and the var. already exists,
 * the old copy is removed first. "name" shouldn't be an indexed
 * var, or an absolute var. number: if so
 * NULL is returned (with error message).  Otherwise, a variable node
 * is created, and its name, type, base ring fields are set with the
 * given parameters.  Its b_next field is set to NULL. The b_next field
 * of the previous last variable (lastv) is modified to point to this
 * variable if "isMain" = PARTVAR, otherwise it isn't modified.
 */

variable *make_var (char *name, int isMain, int type, variable *bring)
/* MAINVAR if variable is main, PARTVAR if partial */
{
    int kind;
    int n;
    char *q, *partname, *inname, ident[100];
    variable *result, *p;

    if (isMain IS PARTVAR) { /* partial vars are special case... */
        result = newvar(name, PARTVAR, type, bring);
        return(result);
    }

    inname = name;
    if (NOT getIdentifier(&inname, ident)) {
        prerror("; bad identifier: %s\n", name);
        return(NULL);
    } else if (*inname ISNT '\0') {
        prerror("; bad identifier: %s\n", name);
        return(NULL);
    }

    kind = whichkind(ident, &n, &q, &partname);
    if (kind ISNT VKNAME) {
        prerror("; can't create an indexed variable\n");
        return(NULL);
    }

    p = fvar(ident, NULL);
    if (p ISNT NULL) {
        /* kill this var, and use it anyway */
        rem_var(p);
    }

    result = newvar(ident, MAINVAR, type, bring);
    return(result);
}

/*
 *      vrg_install     Installs a new ring as current ring.
 *
 */

void vrg_install (variable *p)
{
    if ((p ISNT NULL) AND (p ISNT current_ring)
            AND (VAR_RING(p) ISNT NULL)) {
        current_ring = p; /* needed for rgInstall */
        rgInstall(VAR_RING(p));
    } else if (p ISNT current_ring)
        current_ring = NULL;
}

/*
 *      is_a_module Returns TRUE if "p"s value is indeed a module,
 * else FALSE is returned.
 */

boolean is_a_module (variable *p)
{
    int t;

    t = p->type;
    return((t IS VMODULE) OR (t IS VCOLLECT) OR (t IS VRES)
           OR (t IS VNRES) OR (t IS VSTD)
           OR (t IS VISTD));
}

boolean is_alias (variable *p)
{
    int t;

    t = p->type;
    return((t IS VEMIT) OR (t IS VSTDEMIT) OR (t IS VLIFT));
}

/*
 *      set_value       sets p->value.
 *      connect_vars    sets p->b_next: needs to modify its count then
 */

void set_value (variable *p, ADDRESS val)
{
    p->value = val;
}

void set_alias (variable *p, variable *aliasp)
{
    p->b_alias = aliasp;
}

void connect_vars (variable *p, variable *b_next)
{
    p->b_next = b_next;
}

void putInternals (variable *p)
{
    print("  ");
    if (p->b_ring IS NULL)
        print("  -- ");
    else print("%4d ", p->b_ring->var_num);
    if (p->b_next IS NULL)
        print("  -- ");
    else print("%4d ", p->b_next->var_num);
    if (p->exists IS MAINVAR) print("  main   ");
    else if (p->exists IS PARTVAR) print("  part   ");
    else print("  removed");
    print("\n");
}

void listvars_cmd (int argc, char *argv[])
{
#pragma unused(argv)
    variable *p;
    int i;

    if (last_var IS 0) {
        newline();
        print("no variables defined\n");
        return;
    }
    newline();
    print("  var#   name       type           ");
    if (argc >= 3)
        print("  ring  next status\n");
    else print("\n");
    newline();
    print("  ------------------\n");
    for (i=1; i<=last_var; i++) {
        p = VREF(i);

        if (argc == 1) {
            char *s;
            for (s=p->name; *s!='\0' && *s!='\''; ++s);
            if (*s!='\0' && strcmp ("'zero", s) == 0)
                continue;
        } /* 8/25/93 DAB */

        if ((p->exists IS MAINVAR) OR
                ((argc >= 3) AND (p->exists IS REMOVED))) {
            newline();
            print("%6d   %-10s %-14s", p->var_num, p->name,
                  type_names[p->type]);
            if (argc >= 3)
                putInternals(p);
            else print("\n");
        };
        if ((argc >= 2) AND (p->exists IS PARTVAR)) {
            if ((argc >= 3) OR (*(p->name) ISNT '\0'))
                newline();
            print("%6d     %-8s %-14s", p->var_num, p->name,
                  type_names[p->type]);
            if (argc >= 3)
                putInternals(p);
            else print("\n");
        };
    }
}

/*
 *  the following routines are responsible for killing variables and
 * their associated storage.
 */

void mark (variable *p)
{
    if (p IS NULL) return;
    if (p->garbage IS KEEP) return;
    p->garbage = KEEP;
    mark(p->b_ring);
    mark(p->b_next);
    mark(p->b_alias);
    if (is_alias(p)) mark(p->value);
}

void mark_vars ()
{
    variable *p;
    int i;

    for (i=1; i<=last_var; i++) {
        p = VREF(i);
        p->garbage = DEAD;
    }
    for (i=1; i<=last_var; i++) {
        p = VREF(i);
        if (p->exists ISNT REMOVED)
            mark(p);
    }
}

void cp_varnode (int new, variable *pnew, variable *pold)
{
    variable *p;
    int i;

    *pnew = *pold;
    pnew->var_num = new;
    pold->garbage = DEAD;
    if (cur_ring IS pold)  cur_ring = pnew;
    if (editvar IS pold) editvar = pnew;
    for (i=1; i<=last_var; i++) {
        p = VREF(i);
        if (p->b_ring IS pold)  p->b_ring = pnew;
        if (p->b_next IS pold)  p->b_next = pnew;
        if (p->b_alias IS pold) p->b_alias = pnew;
        if ((is_alias(p)) AND (p->value IS ((ADDRESS) pold)))
            p->value = (ADDRESS) pnew;
    }
}

variable *next_used (int n)
{
    int i;
    variable *p;

    for (i=n+1; i<=last_var; i++) {
        p = VREF(i);
        if (p->garbage IS KEEP)
            return(p);
    }
    return(NULL);
}

void contract_vars ()
{
    int first_free;
    variable *p;
    variable *pold;

    if (last_var IS 0) return;
    for (first_free=1; first_free<=last_var; first_free++) {
        p = VREF(first_free);
        if (p->garbage IS DEAD) break;
    }
    if (p->garbage IS KEEP) return;
    while ((pold = next_used(first_free)) ISNT NULL) {
        p = VREF(first_free);
        cp_varnode(first_free, p, pold);
        first_free++;
    }
    last_var = first_free - 1; /* last one with actual data */
    lastv = p;             /* hasn't been changed since set */
}

void garb_collect ()
{
    int i;
    variable *p;

    mark_vars();
    if (current_ring IS NULL)
        cur_ring = NULL;
    else if (current_ring->garbage IS DEAD)
        cur_ring = NULL;
    else
        cur_ring = current_ring;
    if ((editvar ISNT NULL) AND (editvar->exists IS REMOVED))
        editvar = NULL;
    for (i=last_var; i>=1; i--) {
        p = VREF(i);
        if (p->garbage IS DEAD) {
            vrg_install(p->b_ring);
            varb_kill(p);
            p->value = NULL;
        }
    }
    contract_vars();
    vrg_install(cur_ring);
}

void rem_var (variable *p)
{
    variable *q;
    int i;

    p->exists = REMOVED;
    for (i=p->var_num+1; i<=last_var; i++) {
        q = VREF(i);
        if (q->exists IS PARTVAR)
            q->exists = REMOVED;
        else break;
    }
}

void kill_cmd (int argc, char *argv[])
{
    variable *p;
    int i;

    if (argc IS 1) {
        printnew("kill <var1> ... <var n>\n");
        return;
    }
    for (i=1; i<argc; i++) {
        p = find_var(argv[i]);
        if (p IS NULL) continue;
        rem_var(p);
    }
}

void spare_cmd (int argc, char *argv[])
{
    /* kills all @ vars not excepted */
    variable *p;
    int i, j;
    extern int cur_voice;
    char at[8], *s;

    sprintf(at, "@%02d@", cur_voice);
    for (i=1; i<=last_var; ++i) {
        p = VREF(i);
        for (s=p->name; *s!='\0' && *s!='\''; ++s);
        if (p->exists != MAINVAR ||
                strncmp (at, p->name, 4) != 0 ||
                (*s!='\0' && strcmp ("'zero", s) == 0))
            continue;
        for (j=1; j<argc; ++j) {
            if (strcmp (argv[j], p->name) == 0)
                break;
        }
        if (j == argc) rem_var(p);
    }
} /* 8/25/93 DAB */

void type_cmd (int argc, char *argv[])
{
    variable *p;

    if (argc ISNT 2) {
        printnew("type <var>\n");
        return;
    }
    if ((p = find_var(argv[1])) IS NULL) return;
    if (p->b_ring ISNT NULL)
        vrg_install(p->b_ring);
    varb_type(p);
}
