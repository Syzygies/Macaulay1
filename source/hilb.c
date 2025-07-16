// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <stdio.h>
#include "shared.h"
#include "hilb.h"
#include "stash.h"
#include "monitor.h"
#include "array.h"
#include "term.h"
#include "ring.h"
#include "plist.h"
#include "shell.h"

long genfun[GFSIZE] = {1, 0, 0};
char *headStash;
char *monStash;
char *parenStash;
char *pairStash;

// FLY macro removed - was unused debug code

void putmonlist(arrow head)
{
    int i, en, pren;
    arrow p, loc;

    pren = head->umh.mpren;
    print("\nheader");
    for (i = 0; i < pren; ++i)
        print(" %d", head->umh.mstack[i].mpre);
    print("   %d\n", head->umh.mpred);
    print(" mii=%d eq=", head->umh.mii);
    for (i = 0; i <= head->umh.mii; ++i)
        print(" %d", head->umh.mstack[i].mi);
    print("\n");
    p = head;
    en = head->umh.mn;
    loc = head->umh.mloc;
    do
    {
        p = p->uld.lda[FOW];
        if (p == loc)
            print("* ");
        switch (p->uld.ldkind)
        {
        case 'm':
            for (i = 0; i < en; ++i)
                print(" %d", p->umn.mexp[i]);
            print("\n");
            break;
        case '(':
        case ')':
            print("paren %c, coor = %d\n", p->uld.ldkind, p->ump.mlev);
            break;
        default:
            print("header\n\n");
        }
    } while (p->uld.ldkind != 'h');
}

void monRemNode(arrow p)
{
    char c;

    c = p->uld.ldkind;
    if (c == 'h')
        free_slug((struct stash *)headStash, (struct slug *)p);
    else if (c == 'm')
        free_slug((struct stash *)monStash, (struct slug *)p);
    else
        free_slug((struct stash *)parenStash, (struct slug *)p);
}

void monrefund(arrow h)
{
    arrow p, q;

    p = h;
    do
    {
        q = p->uld.lda[FOW];
        monRemNode(p);
        p = q;
    } while (p != h);
}

arrow monadv(arrow head, int chase)
{
    int i, imin;
    arrow p;

    i = head->umh.mii;
    p = head->umh.mloc;
    imin = chase ? i : head->umh.mimin;
    while (TRUE)
    {
        p = p->uld.lda[FOW];
        switch (p->uld.ldkind)
        {
        case 'm':
            head->umh.mii = i;
            head->umh.mimin = imin;
            return head->umh.mloc = p;
        case '(':
            ++i;
            break;
        case ')':
            --i;
            imin = min_int(i, imin);
            break;
        case 'h':
            return p;
        }
    }
}

arrow monbreed(arrow head, int stop)
{
    expterm nexp;
    int i, ilow, mii, mimin;
    int en, prenew, preold, pren, deg;
    int *exp;
    arrow mloc, newh, end, p;

    mloc = head->umh.mloc;
    mii = head->umh.mii;
    mimin = head->umh.mimin;
    if (stop)
    {
        prenew = 1 + mimin;
        preold = head->umh.mpren;
        pren = prenew + preold;
        en = head->umh.mn - prenew;
        newh = monnewhead(max_int(en, pren + 1));
        newh->umh.mn = en;
        newh->umh.mpren = pren;
        for (i = 0; i < preold; ++i)
            newh->umh.mstack[i].mpre = head->umh.mstack[i].mpre;
        exp = mloc->umn.mexp;
        deg = head->umh.mpred;
        for (i = 0; i < prenew; ++i)
            deg += newh->umh.mstack[i + preold].mpre = (short)exp[i];
        exp += prenew;
        newh->umh.mpred = deg;
        end = mloc;
    }
    else
    {
        prenew = 0;
        en = head->umh.mn;
        newh = monnewhead(en);
        exp = mloc->umn.mexp;
        end = head;
    }
    head->umh.mloc = head;
    head->umh.mii = 0;
    while ((p = monadv(head, TRUE)) != end)
    {
        ilow = max_int(head->umh.mimin - prenew, 0);
        for (i = ilow; i < en; ++i)
            nexp[i] = max_int(exp[i], p->umn.mexp[i + prenew]);
        if (p != mloc)
            monadjoin(newh, nexp, NULL);
    }
    head->umh.mloc = mloc;
    head->umh.mii = mii;
    head->umh.mimin = mimin;
    return newh;
}

void mobfirst(arrow head, htell_func htell_cb)
{
    arrow newh;

    (*htell_cb)(head, FALSE);
    newh = monbreed(head, FALSE);
    if (newh->umh.mloc != newh)
        mobrecurse(newh, htell_cb, TRUE);
    monrefund(newh);
}

void mobrecurse(arrow head, htell_func htell_cb, int plus)
{
    arrow newh;
    int again = FALSE;

    head->umh.mloc = head;
    head->umh.mii = 0;
    while (monadv(head, FALSE) != head)
    {
        (*htell_cb)(head, plus);
        if (again)
        {
            newh = monbreed(head, TRUE);
            mobrecurse(newh, htell_cb, !plus);
            monrefund(newh);
        }
        else
        {
            again = TRUE;
            head->umh.mimin = head->umh.mii;
        }
    }
}

void htell(arrow head, int plus)
{
    int i, deg, en;
    int *a;

    deg = head->umh.mpred;
    en = head->umh.mn;
    a = head->umh.mloc->umn.mexp;
    for (i = 0; i < en; ++i)
        deg += a[i];
    if (deg >= GFSIZE)
    {
        print("\n");
        prerror("; hilb htell: degree bound exceeded!\n");
        to_shell();
    }
    else if (plus)
        genfun[deg]++;
    else
        genfun[deg]--;
}

// Inline functions to replace macros
static inline void getdif(int *dif, int val, int idx, expterm nexp, int backward)
{
    *dif = val - nexp[idx];
    if (backward)
        *dif = -*dif;
}

static inline void ifplus(int dif, int cond, int *done, arrow *p, arrow head, int *i)
{
    if (dif > 0)
    {
        if (cond)
            *done = 1;
        else
        {
            --(*i);
            *p = head->umh.mstack[*i].ma;
        }
    }
}

int monsearch(arrow head, int backward, expterm nexp)
{
    arrow p;
    int again = 0, done = 0, divides = 0;
    int i, j;
    int dif, equ, fow, lpar, rpar, kind, en;

    p = head->umh.mloc;
    i = head->umh.mii;
    en = head->umh.mn;
    if (backward)
        (fow = BAK, lpar = ')', rpar = '(');
    else
        (fow = FOW, lpar = '(', rpar = ')');
    do
    {
        if (again)
            p = p->uld.lda[fow];
        else
            again = 1;
        kind = p->uld.ldkind;
        if (kind == 'm')
        { // monomial
            equ = head->umh.mstack[i].mi;
            getdif(&dif, p->umn.mexp[i], i, nexp, backward);
            ifplus(dif, equ, &done, &p, head, &i);
            if (dif <= 0)
            {
                if (equ && dif < 0)
                    equ = 0;
                for (j = i + 1; j < en; ++j)
                {
                    getdif(&dif, p->umn.mexp[j], j, nexp, backward);
                    if (dif > 0)
                    {
                        if (equ)
                            done = 1;
                        break;
                    }
                    else if (equ && dif < 0)
                        equ = 0;
                }
                if (j == en)
                    divides = 1;
            }
        }
        else if (kind == lpar)
        { // begin field of equal i coord
            getdif(&dif, p->ump.mlev, i, nexp, backward);
            ifplus(dif, head->umh.mstack[i].mi, &done, &p, head, &i);
            if (dif <= 0)
            {
                head->umh.mstack[i++].ma = p->uld.ldc.ca;
                head->umh.mstack[i].mi = head->umh.mstack[i - 1].mi && dif == 0 ? 1 : 0;
            }
        }
        else if (kind == rpar)
        { // end field of equal i-1 coord
            if (head->umh.mstack[i].mi)
                done = 1;
            else
                --i;
        }
        else
            done = 1; // header; end of list
    } while (!(done || divides));
    head->umh.mloc = p;
    head->umh.mii = i;
    return divides;
}

void inlinkduk(arrow p, arrow q)
{
    p->uld.lda[FOW] = q;
    p->uld.lda[BAK] = q->uld.lda[BAK];
    q->uld.lda[BAK] = p;
    p->uld.lda[BAK]->uld.lda[FOW] = p;
}

void unlinkduk(arrow p)
{
    p->uld.lda[FOW]->uld.lda[BAK] = p->uld.lda[BAK];
    p->uld.lda[BAK]->uld.lda[FOW] = p->uld.lda[FOW];
    monRemNode(p);
}

// Inline functions to replace macros
static inline int eqtest(arrow q, int dir, int j, int exp)
{
    return q->uld.lda[dir]->uld.ldkind == 'm' && q->uld.lda[dir]->umn.mexp[j] == exp;
}

static inline void putparen(arrow *a, arrow b, char c, int exp, char *parenStash)
{
    *a = (arrow)(void *)get_slug((struct stash *)parenStash);
    (*a)->uld.ldkind = c;
    (*a)->ump.mlev = exp;
    inlinkduk(*a, b);
}

void moninsert(arrow head, expterm nexp)
{
    arrow p, ql, qr, qlef, qrih;
    arrow q;
    int exp, j;
    int en, i;

    en = head->umh.mn;
    i = head->umh.mii;
    p = head->umh.mloc;
    q = head->umh.mloc = (arrow)(void *)get_slug((struct stash *)monStash);
    q->uld.ldkind = 'm';
    for (j = 0; j < en; ++j)
        q->umn.mexp[j] = nexp[j];
    inlinkduk(q, p);
    for (j = i; j < en - 2; ++j)
    { // need parens, level j?
        exp = nexp[j];
        qlef = NULL;
        if (eqtest(q, FOW, j, exp))
        {
            qlef = q;
            qrih = q->uld.lda[FOW]->uld.lda[FOW];
        }
        else if (eqtest(q, BAK, j, exp))
        {
            qlef = q->uld.lda[BAK];
            qrih = q->uld.lda[FOW];
        }
        if (qlef == NULL)
            break;
        putparen(&ql, qlef, '(', exp, parenStash);
        putparen(&qr, qrih, ')', exp, parenStash);
        ql->uld.ldc.ca = qr;
        qr->uld.ldc.ca = ql;
    }
}

void mondelete(arrow head)
{
    arrow p;
    register arrow q;
    int tryparens = 0, backward = 1;

    p = head->umh.mloc;
    q = p->uld.lda[FOW];
    unlinkduk(p);
    if (q->uld.ldkind == 'm')
    {
        tryparens = 1;
    }
    else if (q->uld.lda[BAK]->uld.ldkind == 'm')
    {
        tryparens = 1;
        backward = 0;
        q = q->uld.lda[BAK];
    }
    if (tryparens)
        while (q->uld.lda[BAK]->uld.ldkind == '(' && q->uld.lda[FOW]->uld.ldkind == ')')
        {
            unlinkduk(q->uld.lda[BAK]);
            unlinkduk(q->uld.lda[FOW]);
            --head->umh.mii;
        }
    if (backward)
        q = q->uld.lda[BAK];
    head->umh.mloc = q;
}

arrow monnewhead(int en)
{
    arrow head;

    head = (arrow)(void *)get_slug((struct stash *)headStash);
    head->uld.lda[0] = head->uld.lda[1] = head->umh.mloc = head;
    head->uld.ldkind = 'h';
    head->umh.mii = 0;
    head->umh.mn = en;
    head->umh.mpren = head->umh.mpred = 0;
    head->umh.mstack[0].mi = 1;
    return head;
}

void monreset(arrow head, int backward)
{
    head->umh.mloc = head->uld.lda[backward ? BAK : FOW];
    head->umh.mii = 0;
}

int monadjoin(arrow head, expterm nexp, htell_func htell_cb)
{
    int divides;
    arrow new;

    monreset(head, FOW);
    divides = monsearch(head, FOW, nexp);
    if (!divides)
    {
        moninsert(head, nexp);
        if (htell_cb != NULL)
            mobfirst(head, htell_cb);
        new = head->umh.mloc;
        monreset(head, BAK);
        while (TRUE)
        {
            monsearch(head, BAK, nexp);
            if (new == head->umh.mloc)
                break;
            else
                mondelete(head);
        }
    }
    return divides;
}

int monbagadjoin(arrow head, expterm nexp, char *bag)
{
    int divides;
    arrow new;

    monreset(head, FOW);
    divides = monsearch(head, FOW, nexp);
    if (!divides)
    {
        moninsert(head, nexp);
        new = head->umh.mloc;
        new->uld.ldc.ca = (union all *)(void *)bag;
        monreset(head, BAK);
        while (TRUE)
        {
            monsearch(head, BAK, nexp);
            if (new == head->umh.mloc)
                break;
            else
                mondelete(head);
        }
    }
    return divides;
}

void i_genfun(void)
{
    register int i;

    genfun[0] = 1;
    for (i = 1; i < GFSIZE; i++)
        genfun[i] = 0;
}

void mn_print(gmatrix M)
{
    int i, d, len;
    mn_syzes *b;
    mn_pair *p;

    b = &M->monsyz;
    d = M->mn_lodeg;
    len = length(b);
    print("mn_print: len = %d, lodeg = %d\n", len, d);
    for (i = 1; i <= len; ++i)
    {
        print("degree %d: ", i + d);
        p = *(mn_pair **)ref(b, i);
        while (p != NULL)
        {
            print("[%d,%d] ", p->id1, p->id2);
            p = p->mpp;
        }
        print("\n");
    }
}

void mn_lcm(arrow head, arrow *newh, expterm m)
{
    arrow p, newhead;
    int i;
    expterm nexp;

    if ((head != NULL) && (head != head->uld.lda[FOW]))
    {
        head->umh.mloc = head;
        head->umh.mii = 0;
        if (*newh == NULL)
            *newh = monnewhead(numvars);
        newhead = *newh;
        while ((p = monadv(head, TRUE)) != head)
        {
            for (i = head->umh.mimin; i < numvars; ++i)
                nexp[i] = max_int(m[i], p->umn.mexp[i]);
            if (!monadjoin(newhead, nexp, NULL))
                newhead->umh.mloc->uld.ldc.ca = p->uld.ldc.ca;
        }
    }
}

void mn_insert(gmatrix M, term m1, char *bag)
{
    expterm m;
    int c, d, len, i;
    mn_table *a;
    mn_syzes *b;
    arrow *pp, head, newh, p;
    mn_pair *q, **r;

    a = &M->montab;
    b = &M->monsyz;
    c = tm_component(m1);
    sToExp(m1, m);
    if (M->mn_lodeg == MAXNEG)
        dl_lohi(&M->degrees, &M->mn_lodeg, &i); // disregard &i
    len = length(a);
    if (len < c)
        for (i = len + 1; i <= c; ++i)
            *((char **)ins_array(a)) = NULL;
    pp = (arrow *)ref(a, c);
    if (*pp == NULL)
        *pp = monnewhead(numvars);
    head = *pp;

    newh = NULL;
    mn_lcm(head, &newh, m);
    mn_lcm(Rideal, &newh, m);
    if (newh != NULL)
    {
        newh->umh.mloc = newh;
        while ((p = monadv(newh, TRUE)) != newh)
        {
            q = (mn_pair *)(void *)get_slug((struct stash *)pairStash);
            d = dlist_ref(&M->degrees, c) - M->mn_lodeg;
            d += exp_degree(p->umn.mexp);
            q->id1 = bag;
            q->id2 = (char *)p->uld.ldc.ca;
            len = length(b);
            if (len < d)
                for (i = len + 1; i <= d; ++i)
                    *((char **)ins_array(b)) = NULL;
            r = (mn_pair **)ref(b, d);
            q->mpp = *r;
            *r = q;
        }
        monrefund(newh);
    }
    monreset(head, FOW);
    monsearch(head, FOW, m);
    moninsert(head, m);
    head->umh.mloc->uld.ldc.ca = (union all *)(void *)bag;
}

// mn_stdinsert is about the same as mn_insert, except no S-pairs are
// computed.

void mn_stdinsert(gmatrix M, term m1, char *bag)
{
    expterm m;
    int c, len, i;
    mn_table *a;
    arrow *pp, head;

    a = &M->montab;
    c = tm_component(m1);
    sToExp(m1, m);
    if (M->mn_lodeg == MAXNEG)
        dl_lohi(&M->degrees, &M->mn_lodeg, &i); // disregard &i
    len = length(a);
    if (len < c)
        for (i = len + 1; i <= c; ++i)
            *((char **)ins_array(a)) = NULL;
    pp = (arrow *)ref(a, c);
    if (*pp == NULL)
        *pp = monnewhead(numvars);
    head = *pp;

    monreset(head, FOW);
    monsearch(head, FOW, m);
    moninsert(head, m);
    head->umh.mloc->uld.ldc.ca = (union all *)(void *)bag;
}

boolean mn_rdiv(term m1, char **b, term m2)
{
    register int i;
    int *n, c;
    expterm nexp;

    if (Rideal != NULL)
    {
        c = tm_component(m1);
        sToExp(m1, nexp);
        monreset(Rideal, FOW);
        if (monsearch(Rideal, FOW, nexp))
        {
            *b = (char *)Rideal->umh.mloc->uld.ldc.ca;
            n = Rideal->umh.mloc->umn.mexp;
            for (i = 0; i < numvars; ++i)
                nexp[i] -= n[i];
            expToS(nexp, c, m2);
            return TRUE;
        }
    }
    return (FALSE);
}

boolean mn_find_div(mn_table *a, term m1, char **b, term m2)
{
    expterm nexp;
    int c, len, i;
    arrow head;
    int *n;

    c = tm_component(m1);
    if (Rideal != NULL)
    {
        sToExp(m1, nexp);
        monreset(Rideal, FOW);
        if (monsearch(Rideal, FOW, nexp))
        {
            *b = (char *)Rideal->umh.mloc->uld.ldc.ca;
            n = Rideal->umh.mloc->umn.mexp;
            for (i = 0; i < numvars; ++i)
                nexp[i] -= n[i];
            expToS(nexp, c, m2);
            return TRUE;
        }
    }
    len = length(a);
    if (len < c)
        return FALSE;
    head = *(arrow *)ref(a, c);
    if (head == NULL)
        return FALSE;
    monreset(head, FOW);
    if (Rideal == NULL)
        sToExp(m1, nexp);
    if (!monsearch(head, FOW, nexp))
        return FALSE;
    *b = (char *)head->umh.mloc->uld.ldc.ca;
    n = head->umh.mloc->umn.mexp;
    for (i = 0; i < numvars; ++i)
        nexp[i] -= n[i];
    expToS(nexp, 0, m2);
    return TRUE;
}

void mn_ring(ring R) // init at ring level
{
    int nvars;
    unsigned int size;

    nvars = R->nvars;
    size = (unsigned int)sizeof(monhead) +
           (unsigned int)(nvars - NVARS) * (unsigned int)sizeof(struct mstk);
    if (nvars == 1)
        size += (unsigned int)sizeof(struct mstk);
    R->headStash = open_stash((int)size, "mon tables: monom heads");
    size = (unsigned int)sizeof(mmonom) + (unsigned int)(nvars - NVARS) * (unsigned int)sizeof(int);
    R->monStash = open_stash((int)size, "mon tables: monomials");

    R->parenStash = open_stash((int)sizeof(monparen), "mon tables: parens");
}

void mn_unring(ring R) // kill at ring level
{
    endof_stash((struct stash *)R->headStash);
    endof_stash((struct stash *)R->monStash);
    endof_stash((struct stash *)R->parenStash);
}

void mn_init(gmatrix M) // init at module level
{
    M->mn_lodeg = MAXNEG;
    init_array(&M->montab, sizeof(char *));
    init_array(&M->monsyz, sizeof(char *));
}

void mn_kill(gmatrix M) // kill at module level
{
    int i, len;
    mn_table *a;
    mn_syzes *b;
    arrow p;
    mn_pair *q, *r;

    a = &M->montab;
    len = length(a);
    for (i = 1; i <= len; ++i)
    {
        p = *(arrow *)ref(a, i);
        if (p != NULL)
            monrefund(p);
    }
    free_array(a);

    b = &M->monsyz;
    len = length(b);
    for (i = 1; i <= len; ++i)
    {
        q = *(mn_pair **)ref(b, i);
        while (q != NULL)
        {
            r = q->mpp;
            free_slug((struct stash *)pairStash, (struct slug *)q);
            q = r;
        }
    }
    free_array(b);
}

void mn_first(void)
{
    pairStash = open_stash(sizeof(mn_pair), "monomial syzygies");
}

boolean mn_iscomplete(gmatrix M)
{
    int i, len;
    mn_syzes *b;

    b = &M->monsyz;
    len = length(b);
    for (i = 1; i <= len; i++)
        if (*(mn_pair **)ref(b, i) != NULL)
        {
            return (FALSE);
        }
    return (TRUE);
}

int mn_next_pair(gmatrix M, int deg, char **i1, char **i2)
{
    mn_pair **p, *q;
    mn_syzes *b;

    deg -= M->mn_lodeg;
    b = &M->monsyz;
    if (deg <= 0 || deg > length(b))
        return FALSE;
    p = (mn_pair **)ref(b, deg);
    q = *p;
    if (q != NULL)
    {
        *i1 = q->id1;
        *i2 = q->id2;
        *p = q->mpp;
        free_slug((struct stash *)pairStash, (struct slug *)q);
        return TRUE;
    }
    return FALSE;
}
