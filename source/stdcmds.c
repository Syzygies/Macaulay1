// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "stdcmds.h"
#include <stdio.h>
#include <string.h>
#include <setjmp.h> // for jmp_buf in parse.h
#include "gmatrix.h"
#include "poly.h"
#include "term.h"
#include "hilb.h"     // for monnewhead, monbagadjoin, monreset, monrefund, mn_stdinsert
#include "monideal.h" // for monnext
#include "cmds.h"     // for vget_mod
#include "plist.h"    // for mod_init, dl_copy, degree
#include "gm_poly.h"  // for get_comp
#include "gm_dlist.h" // for dlDisplay
#include "rvar.h"     // for parseVar, getVar
#include "input.h"    // for get_line
#include "vars.h"
#include "parse.h"
#include "stash.h"
#include "array.h"
#include "monitor.h"
#include "human_io.h" // for check_homog, p_pprint
#include "shell.h"    // for outfile
#include "set.h"      // for linesize
#include "ring.h"     // for nblocks, varnames, numvars
#include "mem.h"      // for std_stash
#include "generic.h"  // for VMODULE and other constants

// External variables
// outfile is from shell.h
// linesize is from set.h
// nblocks, varnames, numvars are from ring.h
// std_stash is from mem.h

// Variable type constants come from generic.h

// Inline functions to replace macros
// initial() is already defined in poly.c and included via poly.h

static inline poly pref(plist pl, int i)
{
    return plist_ref(&pl, i);
}

// max_int() is already defined in shared.h

// Inline functions for command argument handling
static inline bool get_mod(gmatrix *m, int argc, const char **argv, int i)
{
    if (i >= argc)
        return false;
    *m = vget_mod(argv[i]);
    return (*m != nullptr);
}

static inline bool get_mod_with_error(gmatrix *m, int argc, const char **argv, int i)
{
    if (!get_mod(m, argc, argv, i))
    {
        printnew("variable not defined");
        newline();
        return false;
    }
    return true;
}

static inline variable *new_mod_var(int argc, const char **argv, int i)
{
    if (i >= argc)
    {
        prerror("not enough arguments");
        return nullptr;
    }
    variable *p = make_var(argv[i], 0, VMODULE, nullptr);
    if (p == nullptr)
    {
        prerror("can't create variable");
    }
    return p;
}

void stdWarning(gmatrix M)
{
    if (M == nullptr)
        return;
    if (M->modtype == MMAT)
    {
        newline();
        print("warning: no standard basis. Using initial terms of");
        print(" matrix\n");
    }
}

int stdNum(gmatrix M)
{
    if (M == nullptr)
        return 0;
    if (M->modtype == MMAT)
        return ncols(M);
    else
        return M->nstandard;
}

// There are really 3 types of generators here:
// 1. MSTD: standard basis exists.  Each time stdNext is called,
// the current element is issued, and then incremented.
// 2. MISTD: inhomog. standard basis exists.  Each time stdNext
// is called, the current element is incremented, and THEN returned.
// 3. MMAT: neither standard basis type is present.  Each time stdNext
// is called, ith entry is returned, and then i is incremented.

void stdFirst(gmatrix M, modgen *mg, int isstd)
{
    mg->misstd = isstd;
    if (M == nullptr)
        mg->mtype = MNOTHING;
    // else /f ((isstd != USEMAT) && (M->modtype == MSTD)) {
    // mg->mtype = MSTD;
    // mg->mp = M->stdbasis;
    // }
    else if ((isstd != USEMAT) && (M->modtype != MMAT))
    {
        mg->mtype = MISTD;
        mg->mmontab = &M->montab;
        mg->mi = 0;
        mg->mq = nullptr;
    }
    else if (isstd == USECHANGE)
    { // but there isn't one so set to NOTHING
        mg->mtype = MNOTHING;
    }
    else
    {
        mg->mtype = MMAT;
        mg->mgens = &M->gens;
        mg->mi = 1;
    }
}

poly stdNext(modgen *mg)
{
    poly result;
    arrow head;
    mn_standard q;

    if (mg->mtype == MNOTHING)
        return nullptr;
    // if (mg->mtype IS MSTD) {
    // if (mg->mp == nullptr) {
    // mg->mtype = MNOTHING;
    // return(nullptr);
    // }
    // if (mg->misstd IS USESTD)
    // result = mg->mp->standard;
    // else
    // result = mg->mp->change;
    // mg->mp = mg->mp->next;
    // return(result);
    // } else
    if (mg->mtype != MMAT)
    {
        if (mg->mq != nullptr)
        {
            mg->mq = monnext(mg->mq);
            if (mg->mq != nullptr)
            {
                q = (mn_standard)mg->mq->umn.base.ldc.ca;
                if (mg->misstd == USESTD)
                    result = q->standard;
                else
                    result = q->change;
                return result;
            }
        }
        mg->mi++;
        for (; mg->mi <= length(mg->mmontab); mg->mi++)
        {
            head = *(arrow *)ref(mg->mmontab, mg->mi);
            if (head == nullptr)
                continue;
            monreset(head, 0);
            mg->mq = monnext(head);
            if (mg->mq == nullptr)
                continue;
            q = (mn_standard)mg->mq->umn.base.ldc.ca;
            if (mg->misstd == USESTD)
                return q->standard;
            else
                return q->change;
        }
        mg->mtype = MNOTHING;
        return nullptr;
    }
    else
    { // type = MMAT
        if (mg->mi <= length(mg->mgens))
        {
            result = pref(*(mg->mgens), mg->mi);
            mg->mi++;
            return result;
        }
        mg->mtype = MNOTHING;
        return nullptr;
    }
}

// elimination

gmatrix gm_elim(gmatrix M, int n, boolean doelim)
{
    gmatrix N;
    modgen mg;
    poly f;
    boolean doit;

    N = mod_init();
    dl_copy(&M->degrees, &N->degrees);
    stdWarning(M);
    stdFirst(M, &mg, USESTD);
    while ((f = stdNext(&mg)) != nullptr)
    {
        doit = in_subring(initial(f), n);
        if (!doelim)
            doit = !doit;
        if (doit)
        {
            f = p_copy(f);
            gmInsert(N, f);
        }
    }
    return N;
}

void elim_cmd(int argc, const char *argv[])
{
    gmatrix M;
    variable *p;
    int n;

    if ((argc < 3) || (argc > 4))
    {
        printnew("elim <standard basis> <result matrix>");
        print(" [n]\n");
        return;
    }
    if (!get_mod_with_error(&M, argc, argv, 1))
        return;
    if (argc == 3)
        n = 1;
    else
        n = getInt(argv[3]);
    p = new_mod_var(argc, argv, 2);
    if (p == nullptr)
        return;
    set_value(p, gm_elim(M, n, 1));
}

void keep_cmd(int argc, const char *argv[])
{
    gmatrix M;
    variable *p;
    int n;

    if ((argc < 3) || (argc > 4))
    {
        printnew("keep <standard basis> <result matrix>");
        print(" [n]\n");
        return;
    }
    if (!get_mod_with_error(&M, argc, argv, 1))
        return;
    if (argc == 3)
        n = 1;
    else
        n = getInt(argv[3]);
    p = new_mod_var(argc, argv, 2);
    if (p == nullptr)
        return;
    set_value(p, gm_elim(M, n, 0));
}

// initial submodules

gmatrix gm_in(gmatrix M, int n)
{
    gmatrix result;
    modgen mg;
    poly f;

    result = mod_init();
    dl_copy(&M->degrees, &result->degrees);
    stdWarning(M);
    stdFirst(M, &mg, USESTD);
    while ((f = stdNext(&mg)) != nullptr)
        gmInsert(result, p_in(f, n));
    return result;
}

void in_pprint(FILE *fil, term t, int comp)
{
    if (comp != tm_component(t))
        fprint(fil, "0");
    else
        tm_pprint(fil, t);
}

void put_in(FILE *fil, gmatrix M)
{
    modgen mg;
    poly f;
    int j, r;
    int c;

    r = nrows(M);
    c = stdNum(M);
    fnewline(fil);
    fprint(fil, "%d\n", r);
    fnewline(fil);
    fprint(fil, "%d\n", c);
    stdWarning(M);
    stdFirst(M, &mg, USESTD);
    while ((f = stdNext(&mg)) != nullptr)
    {
        for (j = 1; j <= r; j++)
        {
            fnewline(fil);
            in_pprint(fil, initial(f), j);
            fprint(fil, "\n");
        }
    }
    dlDisplay(fil, &M->degrees, linesize);
}

void in_cmd(int argc, const char *argv[])
{
    gmatrix M;
    variable *p;
    int n;

    if ((argc < 2) || (argc > 4))
    {
        printnew("in <standard basis> [optional result matrix] [n]\n");
        return;
    }
    if (!get_mod_with_error(&M, argc, argv, 1))
        return;
    if (argc == 2)
        put_in(outfile, M);
    else
    {
        if (argc == 3)
            n = nblocks;
        else
            n = getInt(argv[3]);
        p = new_mod_var(argc, argv, 2);
        if (p == nullptr)
            return;
        set_value(p, gm_in(M, n));
    }
}

// obtaining a variable list

boolean varLoHi(const char *s, expterm exp) // returns 1 if no error
{
    char s_copy[256]; // parseVar modifies the string
    strncpy(s_copy, s, sizeof(s_copy) - 1);
    s_copy[sizeof(s_copy) - 1] = '\0';
    char *s_ptr = s_copy;
    int i, lo, hi;

    lo = parseVar(&s_ptr);
    if (*s_ptr == '\0')
        hi = lo;
    else
    {
        if (*s_ptr == '.')
            s_ptr++;
        if (*s_ptr != '.')
            hi = lo - 1;
        else
        {
            s_ptr++;
            hi = parseVar(&s_ptr);
        }
    }
    if ((lo < 0) || (hi < 0) || (hi < lo))
    {
        prerror("; bad variable list\n");
        return 0;
    }
    for (i = lo; i <= hi; i++)
        exp[i] = 1;
    return 1;
}

boolean getVarList(int argc, const char *argv[], expterm exp)
{
    int i;
    boolean flip;
    // Make mutable copies for get_line
    int local_argc = argc;
    char **local_argv = (char **)(void *)argv;

    for (i = 0; i < numvars; i++)
        exp[i] = 0;
    if (local_argc == 0)
    {
        getfirstblock(exp);
        return 1;
    }

    if (*local_argv[0] == '!')
    {
        flip = 1;
        local_argc--;
        local_argv++;
    }
    else
        flip = 0;

    i = 0;
    while (i < local_argc)
    {
        if (*local_argv[i] == '\\')
        {
            prinput("more variables");
            get_line(&local_argc, &local_argv);
            i = 0;
            continue;
        }
        if (!varLoHi(local_argv[i], exp))
            return 0;
        i++;
    }
    if (flip)
    {
        for (i = 0; i < numvars; i++)
            if (exp[i] == 0)
                exp[i] = 1;
            else
                exp[i] = 0;
    }
    return 1;
}

// initial coefficients

boolean exp_eqcoef(expterm exp, expterm exp2, expterm vars)
{
    int i;

    for (i = 0; i < numvars; i++)
        if (vars[i] != 0)
        {
            if (exp[i] != exp2[i])
                return 0;
            else
                exp2[i] = 0;
        }
    return 1;
}

poly p_incoef(poly f, expterm vars)
{
    term t;
    expterm exp, exp2;
    poly result, g;

    if (f == nullptr)
        return nullptr;
    t = (term)initial(f);
    sToExp(t, exp);
    result = nullptr;
    while (f != nullptr)
    {
        sToExp(initial(f), exp2);
        if (!exp_eqcoef(exp, exp2, vars))
            break;
        g = p_monom(f->coef);
        expToS(exp2, get_comp(f), initial(g));
        f = f->next;
        p_add(&result, &g);
    }
    return result;
}

gmatrix gm_incoef(gmatrix M, expterm vars)
{
    gmatrix result;
    modgen mg;
    poly f;

    result = mod_init();
    dl_copy(&M->degrees, &result->degrees);
    stdWarning(M);
    stdFirst(M, &mg, USESTD);
    while ((f = stdNext(&mg)) != nullptr)
        gmInsert(result, p_incoef(f, vars));
    return result;
}

void incoef_cmd(int argc, const char *argv[])
{
    gmatrix M;
    variable *p;
    expterm vars;

    if (argc < 3)
    {
        printnew("incoef <standard basis> <result matrix> [variable list]\n");
        return;
    }
    if (!get_mod_with_error(&M, argc, argv, 1))
        return;
    p = new_mod_var(argc, argv, 2);
    if (p == nullptr)
        return;
    if (!getVarList(argc - 3, argv + 3, vars))
        return;
    set_value(p, gm_incoef(M, vars));
}

// initial ideals for "part" of the variables

void exp_inpart(expterm exp, expterm vars)
{
    int i;

    for (i = 0; i <= numvars; i++)
        if (vars[i] == 0)
            exp[i] = 0;
}

poly p_inpart(poly f, expterm vars)
{
    poly result;
    expterm exp;

    if (f == nullptr)
        return nullptr;
    result = p_monom(f->coef);
    sToExp(initial(f), exp);
    exp_inpart(exp, vars);
    expToS(exp, get_comp(f), initial(result));
    return result;
}

gmatrix gm_stdpart(gmatrix M, expterm vars, boolean initialOnly)
{
    gmatrix result;
    int i;
    arrow p, head;
    modgen mg;
    expterm exp;
    poly f;

    result = mod_init();
    dl_copy(&M->degrees, &result->degrees);
    stdWarning(M);
    for (i = 1; i <= nrows(M); i++)
    {
        head = monnewhead(numvars);
        stdFirst(M, &mg, USESTD);
        while ((f = stdNext(&mg)) != nullptr)
        {
            if (i != get_comp(f))
                continue;
            sToExp(initial(f), exp);
            exp_inpart(exp, vars);
            monbagadjoin(head, exp, (char *)f);
        }
        p = head;
        while ((p = monnext(p)) != nullptr)
        {
            f = (poly)p->uld.ldc.ca;
            if (initialOnly)
                f = p_inpart(f, vars);
            else
                f = p_copy(f);
            gmInsert(result, f);
        }
        monrefund(head);
    }
    return result;
}

void stdpart_cmd(int argc, const char *argv[])
{
    gmatrix M;
    variable *p;
    expterm vars;

    if (argc < 3)
    {
        printnew("stdpart <standard basis> <result matrix> [variable list]\n");
        return;
    }
    if (!get_mod_with_error(&M, argc, argv, 1))
        return;
    p = new_mod_var(argc, argv, 2);
    if (p == nullptr)
        return;
    if (!getVarList(argc - 3, argv + 3, vars))
        return;
    set_value(p, gm_stdpart(M, vars, 0));
}

void inpart_cmd(int argc, const char *argv[])
{
    gmatrix M;
    variable *p;
    expterm vars;

    if (argc < 3)
    {
        printnew("inpart <standard basis> <result matrix> [variable list]\n");
        return;
    }
    if (!get_mod_with_error(&M, argc, argv, 1))
        return;
    p = new_mod_var(argc, argv, 2);
    if (p == nullptr)
        return;
    if (!getVarList(argc - 3, argv + 3, vars))
        return;
    set_value(p, gm_stdpart(M, vars, 1));
}

// finding subset of std basis whose in(f) are min. gens of in(I)

void ins_stdmin(gmatrix result, gmatrix M, expterm nexp, int thiscomp, expterm vars)
{
    modgen mg;
    poly f;
    expterm exp;

    stdFirst(M, &mg, USESTD);
    while ((f = stdNext(&mg)) != nullptr)
    {
        if (thiscomp != get_comp(f))
            continue;
        sToExp(initial(f), exp);
        if (exp_eqcoef(nexp, exp, vars))
            gmInsert(result, p_copy(f));
    }
}

gmatrix gm_stdmin(gmatrix M, expterm vars)
{
    gmatrix result;
    int i;
    arrow p, head;
    modgen mg;
    poly f;
    expterm exp;

    result = mod_init();
    dl_copy(&M->degrees, &result->degrees);
    stdWarning(M);
    for (i = 1; i <= nrows(M); i++)
    {
        head = monnewhead(numvars);
        stdFirst(M, &mg, USESTD);
        while ((f = stdNext(&mg)) != nullptr)
        {
            if (i != get_comp(f))
                continue;
            sToExp(initial(f), exp);
            exp_inpart(exp, vars);
            monbagadjoin(head, exp, (char *)f);
        }
        p = head;
        while ((p = monnext(p)) != nullptr)
        {
            ins_stdmin(result, M, p->umn.mexp, i, vars);
        }
        monrefund(head);
    }
    return result;
}

void stdmin_cmd(int argc, const char *argv[])
{
    gmatrix M;
    variable *p;
    expterm vars;

    if (argc < 3)
    {
        printnew("std_minimal <standard basis> <result matrix> [variable list]\n");
        return;
    }
    if (!get_mod_with_error(&M, argc, argv, 1))
        return;
    p = new_mod_var(argc, argv, 2);
    if (p == nullptr)
        return;
    if (!getVarList(argc - 3, argv + 3, vars))
        return;
    set_value(p, gm_stdmin(M, vars));
}

// saturation

int p_maxdiv(poly f, int n)
{
    int result;
    expterm exp;

    if (f == nullptr)
        return 0;
    sToExp(initial(f), exp);
    result = exp[n];
    while ((f = f->next) != nullptr)
    {
        sToExp(initial(f), exp);
        if (exp[n] < result)
            result = exp[n];
        if (result == 0)
            return 0;
    }
    return result;
}

poly p_vardiv(poly f, int n, int maxpower)
{
    expterm exp;
    poly result, g;

    result = nullptr;
    while (f != nullptr)
    {
        sToExp(initial(f), exp);
        if (maxpower <= exp[n])
            exp[n] -= maxpower;
        else
            exp[n] = 0;
        g = p_monom(f->coef);
        expToS(exp, get_comp(f), initial(g));
        p_add(&result, &g);
        f = f->next;
    }
    return result;
}

gmatrix gm_vardiv(gmatrix M, int n, int maxpower)
{
    gmatrix result;
    int i, max;
    poly f;
    modgen mg;

    result = mod_init();
    dl_copy(&M->degrees, &result->degrees);
    stdWarning(M);
    stdFirst(M, &mg, USESTD);
    max = -1;
    while ((f = stdNext(&mg)) != nullptr)
    {
        i = p_maxdiv(f, n);
        if ((maxpower > 0) && (i > maxpower))
            i = maxpower;
        if (max == -1)
            max = i;
        else
            max = max_int(max, i);
        gmInsert(result, p_vardiv(f, n, i));
    }

    if (max > 0)
    {
        newline();
        print("largest division was by %s", varnames[n]);
        if (max > 1)
            print("%d", max);
        print("\n");
    }
    return result;
}

void sat_cmd(int argc, const char *argv[])
{
    gmatrix M;
    variable *p;
    int maxpower, varnum;

    if ((argc < 3) || (argc > 5))
    {
        printnew("sat <standard basis> <result matrix> <variable> [max power]\n");
        return;
    }
    if (!get_mod_with_error(&M, argc, argv, 1))
        return;
    varnum = getVar(argv[3]);
    if (varnum == -1)
    {
        prerror("; bad ring variable name\n");
        return;
    }
    p = new_mod_var(argc, argv, 2);
    if (p == nullptr)
        return;
    if (argc < 5)
        maxpower = -1;
    else
        maxpower = getInt(argv[4]);

    set_value(p, gm_vardiv(M, varnum, maxpower));
}

// obtaining std, change

gmatrix gm_std(gmatrix M, int isstd)
{
    gmatrix result;
    modgen mg;
    poly f;

    result = mod_init();
    if (isstd == USESTD)
        dl_copy(&M->degrees, &result->degrees);
    else
        dl_copy(&M->deggens, &result->degrees);
    stdFirst(M, &mg, isstd);
    while ((f = stdNext(&mg)) != nullptr)
        gmInsert(result, p_copy(f));
    return result;
}

void put_std(FILE *fil, gmatrix M, int isstd)
{
    int i, j, r;
    modgen mg;
    poly f;

    r = (isstd == USESTD ? nrows(M) : ncols(M));
    fnewline(fil);
    fprint(fil, "%d\n", r);
    fnewline(fil);
    fprint(fil, "%d\n", stdNum(M));
    stdFirst(M, &mg, isstd);
    for (i = 1; i <= stdNum(M); i++)
    {
        f = stdNext(&mg);
        for (j = 1; j <= r; j++)
        {
            fnewline(fil);
            p_pprint(fil, f, j);
            fprint(fil, "\n");
        }
    }
}

// force standard basis

gmatrix gm_force(gmatrix M, gmatrix Mchange)
{
    gmatrix result;
    poly f, g;
    mn_standard q;
    modgen mg, mgchange;

    if (M == nullptr)
        return nullptr;
    if (Mchange != nullptr)
    {
        if (nrows(Mchange) > ncols(M))
        {
            prerror("; change of basis matrix has too many rows\n");
            return nullptr;
        }
    }
    result = mod_init();
    if (check_homog(M))
        result->modtype = MSTD;
    else
        result->modtype = MISTD;

    dl_copy(&M->degrees, &result->degrees);
    stdFirst(M, &mg, USEMAT);
    stdFirst(Mchange, &mgchange, USEMAT);
    while ((f = stdNext(&mg)) != nullptr)
    {
        f = p_copy(f);
        gmInsert(result, f);
        g = stdNext(&mgchange);
        f = p_copy(f);
        g = p_copy(g);
        make2_monic(&f, &g);
        q = (mn_standard)(void *)get_slug((struct stash *)std_stash);
        q->standard = f;
        q->change = g;
        q->degree = degree(M, f);
        if (result->modtype == MSTD)
        {
            q->next = result->stdbasis;
            result->stdbasis = q;
        }
        else
            q->next = nullptr;
        result->nstandard++;
        mn_stdinsert(result, initial(f), (char *)q);
    }
    return result;
}

void forcestd_cmd(int argc, const char *argv[])
{
    gmatrix M, Mchange;
    variable *p;

    if ((argc < 3) || (argc > 4))
    {
        printnew("forcestd <matrix> <result standard basis> [change of basis]\n");
        return;
    }
    if (!get_mod_with_error(&M, argc, argv, 1))
        return;
    if (argc == 4)
    {
        if (!get_mod_with_error(&Mchange, argc, argv, 3))
            return;
    }
    else
        Mchange = nullptr;
    p = new_mod_var(argc, argv, 2);
    if (p == nullptr)
        return;
    set_value(p, gm_force(M, Mchange));
}
