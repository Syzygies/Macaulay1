/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "style.h"
#include "iring.h"
#include  "ddefs.h"
#include "mtypes.h"

// void putmonlist (arrow head);
// void monRemNode (arrow p);
// void monrefund (arrow h);
// arrow monadv (arrow head, int chase);
// arrow monbreed (arrow head, int stop);
// void mobfirst (arrow head, pfi htell);
void mobrecurse (arrow head, pfi htell, int plus);
// void htell (arrow head, int plus);
// int monsearch (arrow head, int backward, expterm nexp);
// void inlinkduk (arrow p, arrow q);
// void unlinkduk (arrow p);
// void moninsert (arrow head, expterm nexp);
// void mondelete (arrow head);
// arrow monnewhead (int en);
// void monreset (arrow head, int backward);
// int monadjoin (arrow head, expterm nexp, pfi htell);
// int monbagadjoin (arrow head, expterm nexp, char *bag);
// void i_genfun ();
// void mn_print (gmatrix M);
// void mn_lcm (arrow head, arrow *newh, expterm m);
// void mn_insert (gmatrix M, term m1, char * bag);
// void mn_stdinsert (gmatrix M, term m1, char * bag);
// boolean mn_rdiv (term m1, char **b, term m2);
// boolean mn_find_div (mn_table *a, term m1, char **b, term m2);
// void mn_first ();
// boolean mn_iscomplete (gmatrix M);
// int mn_next_pair (gmatrix M, int deg, char **i1, char **i2);

extern char *open_stash();
extern char *get_slug();
extern char *free_slug();
extern arrow monnewhead ();

extern arrow Rideal;

long genfun[GFSIZE] = { 1,0,0 };
char *headStash;
char *monStash;
char *parenStash;
char *pairStash;

#define FLY(A, B) { \
    print("A: B"); \
    while (getchar() != '\n'); \
    }

void putmonlist (arrow head)
{
    int i, en, pren;
    arrow p, loc;

    pren = head->umh.mpren;
    print ("\nheader");
    for ( i=0; i<pren; ++i)
        print (" %d",head->umh.mstack[i].mpre);
    print ("   %d\n",head->umh.mpred);
    print (" mii=%d eq=",head->umh.mii);
    for ( i=0; i<=head->umh.mii; ++i)
        print (" %d",head->umh.mstack[i].mi);
    print ("\n");
    p = head;
    en = head->umh.mn;
    loc = head->umh.mloc;
    do {
        p = p->uld.lda[FOW];
        if (p == loc) print ("* ");
        switch (p->uld.ldkind) {
        case 'm':
            for ( i=0; i<en; ++i)
                print (" %d",p->umn.mexp[i]);
            print ("\n");
            break;
        case '(':
        case ')':
            print ("paren %c, coor = %d\n",p->uld.ldkind,p->ump.mlev);
            break;
        default:
            print ("header\n\n");
        }
    }
    while (p->uld.ldkind != 'h');
}

void monRemNode (arrow p)
{
    char c;

    c = p->uld.ldkind;
    if (c == 'h') free_slug(headStash, p);
    else if (c == 'm') free_slug(monStash, p);
    else free_slug(parenStash, p);
}

void monrefund (arrow h)
{
    arrow p, q;

    p = h;
    do {
        q = p->uld.lda[FOW];
        monRemNode(p);
        p = q;
    } while (p != h);
}

arrow monadv (arrow head, int chase)
{
    int i, imin;
    arrow p;

    i = head->umh.mii;
    p = head->umh.mloc;
    imin = chase ? i : head->umh.mimin;
    while (TRUE) {
        p = p->uld.lda[FOW];
        switch (p->uld.ldkind) {
        case 'm':
            head->umh.mii = i;
            head->umh.mimin = imin;
            return head->umh.mloc = p;
        case '(':
            ++i;
            break;
        case ')':
            --i;
            imin = MIN(i,imin);
            break;
        case 'h':
            return p;
        }
    }
}

arrow monbreed (arrow head, int stop)
{
    expterm nexp;
    int i, ilow, mii, mimin;
    int en, prenew, preold, pren, deg;
    int *exp;
    arrow mloc, newh, end, p;

    mloc = head->umh.mloc;
    mii = head->umh.mii;
    mimin = head->umh.mimin;
    if (stop) {
        prenew = 1 + mimin;
        preold = head->umh.mpren;
        pren = prenew + preold;
        en = head->umh.mn - prenew;
        newh = monnewhead (MAX(en,pren+1));
        newh->umh.mn = en;
        newh->umh.mpren = pren;
        for ( i=0; i<preold; ++i)
            newh->umh.mstack[i].mpre = head->umh.mstack[i].mpre;
        exp = mloc->umn.mexp;
        deg = head->umh.mpred;
        for ( i=0; i<prenew; ++i)
            deg += newh->umh.mstack[i+preold].mpre = exp[i];
        exp += prenew;
        newh->umh.mpred = deg;
        end = mloc;
    }
    else {
        prenew = 0;
        en = head->umh.mn;
        newh = monnewhead (en);
        exp = mloc->umn.mexp;
        end = head;
    }
    head->umh.mloc = head;
    head->umh.mii = 0;
    while ((p = monadv (head,TRUE)) != end) {
        ilow = MAX(head->umh.mimin-prenew,0);
        for ( i=ilow; i<en; ++i)
            nexp[i] = MAX(exp[i],p->umn.mexp[i+prenew]);
        if (p != mloc) monadjoin (newh,nexp,NULL);
    }
    head->umh.mloc = mloc;
    head->umh.mii = mii;
    head->umh.mimin = mimin;
    return newh;
}

void mobfirst (arrow head, pfi htell)
{
    arrow newh;

    (*htell) (head,FALSE);
    newh = monbreed (head,FALSE);
    if (newh->umh.mloc != newh)
        mobrecurse (newh,htell,TRUE);
    monrefund (newh);
}

void mobrecurse (arrow head, pfi htell, int plus)
{
    arrow newh;
    int again = FALSE;

    head->umh.mloc = head;
    head->umh.mii = 0;
    while (monadv (head,FALSE) != head) {
        (*htell) (head,plus);
        if (again) {
            newh = monbreed (head,TRUE);
            mobrecurse (newh,htell,!plus);
            monrefund (newh);
        }
        else {
            again = TRUE;
            head->umh.mimin = head->umh.mii;
        }
    }
}

void htell (arrow head, int plus)
{
    int i, deg, en;
    int *a;

    deg = head->umh.mpred;
    en = head->umh.mn;
    a = head->umh.mloc->umn.mexp;
    for ( i=0; i<en; ++i)
        deg += a[i];
    if (deg >= GFSIZE) {
        print("\n");
        prerror("; hilb htell: degree bound exceeded!\n");
        to_shell();
    }
    else if (plus) genfun[deg]++;
    else genfun[deg]--;
}

#define GETDIF(A,I) \
  dif = (A) - nexp[I]; \
  if (backward) dif = -dif; \

#define IFPLUS(A) \
  if (dif > 0) { \
    if (A) done = 1; \
    else p = head->umh.mstack[--i].ma; \
  }

int monsearch (arrow head, int backward, expterm nexp)
/* use FOR or BAK for direction */
/* new monomial */
{
    arrow p;
    int again = 0, done = 0, divides = 0;
    int i, j;
    int dif, equ, fow, bak, lpar, rpar, kind, en;

    p = head->umh.mloc;
    i = head->umh.mii;
    en = head->umh.mn;
    if (backward)
        (fow=BAK, bak=FOW, lpar=')', rpar='(');
    else (fow=FOW, bak=BAK, lpar='(', rpar=')');
    do {
        if (again) p = p->uld.lda[fow];
        else again = 1;
        kind = p->uld.ldkind;
        if (kind == 'm') {           /* monomial  */
            equ = head->umh.mstack[i].mi;
            GETDIF(p->umn.mexp[i],i)
            IFPLUS(equ)
            else {
                if (equ && dif < 0) equ = 0;
                for ( j=i+1; j<en; ++j) {
                    GETDIF(p->umn.mexp[j],j)
                    if (dif > 0) {
                        if (equ) done = 1;
                        break;
                    }
                    else if (equ && dif < 0) equ = 0;
                }
                if (j == en) divides = 1;
            }
        }
        else if (kind == lpar) {     /* begin field of equal i coord */
            GETDIF(p->ump.mlev,i)
            IFPLUS(head->umh.mstack[i].mi)
            else {
                head->umh.mstack[i++].ma = p->uld.ldc.ca;
                head->umh.mstack[i].mi = head->umh.mstack[i-1].mi && dif == 0
                                         ? 1 : 0;
            }
        }
        else if (kind == rpar) {     /* end field of equal i-1 coord */
            if (head->umh.mstack[i].mi) done = 1;
            else --i;
        }
        else  done = 1;      /* header; end of list */
    }
    while (!(done || divides));
    head->umh.mloc = p;
    head->umh.mii = i;
    return divides;
}

void inlinkduk (arrow p, arrow q)
/* linkduk to insert */
/* ... in front of here */
{
    p->uld.lda[FOW] = q;
    p->uld.lda[BAK] = q->uld.lda[BAK];
    q->uld.lda[BAK] = p;
    p->uld.lda[BAK]->uld.lda[FOW] = p;
}

void unlinkduk (arrow p)
/* unlinkduk, free storage for p */
{
    p->uld.lda[FOW]->uld.lda[BAK] = p->uld.lda[BAK];
    p->uld.lda[BAK]->uld.lda[FOW] = p->uld.lda[FOW];
    monRemNode(p);
}

#define EQTEST(A) \
  q->uld.lda[A]->uld.ldkind == 'm' && q->uld.lda[A]->umn.mexp[j] == exp

#define PUTPAREN(A,B,C) \
  A = (arrow) get_slug(parenStash); \
  A->uld.ldkind = C; \
  A->ump.mlev = exp; \
  inlinkduk (A,B);

void moninsert (arrow head, expterm nexp)
/* header knows spot for insert */
/* exponent vector, new mon */
{
    arrow p, ql, qr, qlef, qrih;
    arrow q;
    int exp, j;
    int en, i;

    en = head->umh.mn;
    i = head->umh.mii;
    p = head->umh.mloc;
    q = head->umh.mloc = (arrow) get_slug(monStash);
    q->uld.ldkind = 'm';
    for ( j=0; j<en; ++j)
        q->umn.mexp[j] = nexp[j];
    inlinkduk (q,p);
    for( j=i; j<en-2; ++j) {   /* need parens, level j?  */
        exp = nexp[j];
        qlef = NULL;
        if (EQTEST(FOW)) {
            qlef = q;
            qrih = q->uld.lda[FOW]->uld.lda[FOW];
        }
        else if (EQTEST(BAK)) {
            qlef = q->uld.lda[BAK];
            qrih = q->uld.lda[FOW];
        }
        if (qlef == NULL) break;
        PUTPAREN(ql,qlef,'(')
        PUTPAREN(qr,qrih,')')
        ql->uld.ldc.ca = qr;
        qr->uld.ldc.ca = ql;
    }
}

void mondelete (arrow head)
/* head knows spot to delete */
{
    arrow p;
    arrow q;
    int tryparens = 0, backward = 1;

    p = head->umh.mloc;
    q = p->uld.lda[FOW];
    unlinkduk (p);
    if (q->uld.ldkind == 'm') {
        tryparens = 1;
    }
    else if (q->uld.lda[BAK]->uld.ldkind == 'm') {
        tryparens = 1;
        backward = 0;
        q = q->uld.lda[BAK];
    }
    if (tryparens)
        while (q->uld.lda[BAK]->uld.ldkind == '('
                && q->uld.lda[FOW]->uld.ldkind == ')') {
            unlinkduk (q->uld.lda[BAK]);
            unlinkduk (q->uld.lda[FOW]);
            --head->umh.mii;
        }
    if (backward) q = q->uld.lda[BAK];
    head->umh.mloc = q;
}

arrow monnewhead (int en)
{
    arrow head;

    head = (arrow) get_slug(headStash);
    head->uld.lda[0] = head->uld.lda[1] = head->umh.mloc = head;
    head->uld.ldkind = 'h';
    head->umh.mii = 0;
    head->umh.mn = en;
    head->umh.mpren = head->umh.mpred = 0;
    head->umh.mstack[0].mi = 1;
    return head;
}

void monreset (arrow head, int backward)
/* use FOR or BAK for direction */
{
    head->umh.mloc = head->uld.lda[ backward ? BAK : FOW ];
    head->umh.mii = 0;
}

int monadjoin (arrow head, expterm nexp, pfi htell)
/* new exponent */
{
    int divides;
    arrow new;

    monreset (head,FOW);
    divides = monsearch (head,FOW,nexp);
    if (!divides) {
        moninsert (head,nexp);
        if (htell != NULL) mobfirst (head,htell);
        new = head->umh.mloc;
        monreset (head,BAK);
        while (TRUE) {
            monsearch (head,BAK,nexp);
            if (new == head->umh.mloc) break;
            else mondelete (head);
        }
    }
    return divides;
}

int monbagadjoin (arrow head, expterm nexp, char *bag)
/* new exponent */
{
    int divides;
    arrow new;

    monreset (head,FOW);
    divides = monsearch (head,FOW,nexp);
    if (!divides) {
        moninsert (head,nexp);
        new = head->umh.mloc;
        new->uld.ldc.ci = bag;
        monreset (head,BAK);
        while (TRUE) {
            monsearch (head,BAK,nexp);
            if (new == head->umh.mloc) break;
            else mondelete (head);
        }
    }
    return divides;
}

void i_genfun ()
{
    int i;

    genfun[0] = 1;
    for (i=1; i<GFSIZE; i++)
        genfun[i] = 0;
}

void mn_print (gmatrix M)
{
    int i, d, len;
    mn_syzes *b;
    mn_pair *p;

    b = &M->monsyz;
    d = M->mn_lodeg;
    len = length(b);
    print("mn_print: len = %d, lodeg = %d\n", len, d);
    for (i=1; i<=len; ++i) {
        print("degree %d: ", i+d);
        p = * (mn_pair **) ref(b,i);
        while (p != NULL) {
            print("[%d,%d] ", p->id1, p->id2);
            p = p->mpp;
        }
        print("\n");
    }
}

void mn_lcm (arrow head, arrow *newh, expterm m)
/* monomial ideal */
/* (possibly) new monom ideal of l.c.m.'s of head w. "m" */
{
    arrow p, newhead;
    int i;
    expterm nexp;

    if ((head != NULL) AND (head != head->uld.lda[FOW])) {
        head->umh.mloc = head;
        head->umh.mii = 0;
        if (*newh == NULL)
            *newh = monnewhead (numvars);
        newhead = *newh;
        while ((p = monadv (head,TRUE)) != head) {
            for (i=head->umh.mimin; i<numvars; ++i)
                nexp[i] = MAX(m[i],p->umn.mexp[i]);
            if (!monadjoin (newhead,nexp,NULL))
                newhead->umh.mloc->uld.ldc.ci = p->uld.ldc.ci;
        }
    }
}

void mn_insert (gmatrix M, term m1, char * bag)
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
        dl_lohi(&M->degrees, &M->mn_lodeg, &i); /* disregard &i */
    len = length(a);
    if (len < c) for (i=len+1; i<=c; ++i)
            *((char **) ins_array(a)) = NULL;
    pp = (arrow *) ref(a,c);
    if (*pp == NULL) *pp = monnewhead(numvars);
    head = *pp;

    newh = NULL;
    mn_lcm(head, &newh, m);
    mn_lcm(Rideal, &newh, m);
    if (newh != NULL) {
        newh->umh.mloc = newh;
        while ((p=monadv(newh,TRUE))!=newh) {
            q = (mn_pair *) get_slug(pairStash);
            d = DREF(M->degrees, c) - M->mn_lodeg;
            d += exp_degree(p->umn.mexp);
            q->id1 = bag;
            q->id2 = p->uld.ldc.ci;
            len = length(b);
            if (len < d) for (i=len+1; i<=d; ++i)
                    *((char **) ins_array(b)) = NULL;
            r = (mn_pair **) ref(b,d);
            q->mpp = *r;
            *r = q;
        }
        monrefund (newh);
    }
    monreset(head, FOW);
    monsearch(head, FOW, m);
    moninsert(head, m);
    head->umh.mloc->uld.ldc.ci = bag;
}

/* mn_stdinsert is about the same as mn_insert, except no S-pairs are
 * computed.
 */

void mn_stdinsert (gmatrix M, term m1, char * bag)
{
    expterm m;
    int c, len, i;
    mn_table *a;
    mn_syzes *b;
    arrow *pp, head;

    a = &M->montab;
    b = &M->monsyz;
    c = tm_component(m1);
    sToExp(m1, m);
    if (M->mn_lodeg == MAXNEG)
        dl_lohi(&M->degrees, &M->mn_lodeg, &i); /* disregard &i */
    len = length(a);
    if (len < c) for (i=len+1; i<=c; ++i)
            *((char **) ins_array(a)) = NULL;
    pp = (arrow *) ref(a,c);
    if (*pp == NULL) *pp = monnewhead(numvars);
    head = *pp;

    monreset(head, FOW);
    monsearch(head, FOW, m);
    moninsert(head, m);
    head->umh.mloc->uld.ldc.ci = bag;
}

boolean mn_rdiv (term m1, char **b, term m2)
{
    int i;
    int *n, c;
    expterm nexp;

    if (Rideal != NULL) {
        c = tm_component(m1);
        sToExp(m1, nexp);
        monreset(Rideal, FOW);
        if (monsearch(Rideal, FOW, nexp)) {
            *b = Rideal->umh.mloc->uld.ldc.ci;
            n = Rideal->umh.mloc->umn.mexp;
            for (i=0; i<numvars; ++i) nexp[i] -= n[i];
            expToS(nexp, c, m2);
            return TRUE;
        }
    }
    return(FALSE);
}

boolean mn_find_div (mn_table *a, term m1, char **b, term m2)
/* returns baggage pointer */
{
    expterm nexp;
    int c, len, i;
    arrow head;
    int *n;

    c = tm_component(m1);
    if (Rideal != NULL) {
        sToExp(m1, nexp);
        monreset(Rideal, FOW);
        if (monsearch(Rideal, FOW, nexp)) {
            *b = Rideal->umh.mloc->uld.ldc.ci;
            n = Rideal->umh.mloc->umn.mexp;
            for (i=0; i<numvars; ++i) nexp[i] -= n[i];
            expToS(nexp, c, m2);
            return TRUE;
        }
    }
    len = length(a);
    if (len < c) return FALSE;
    head = * (arrow *) ref(a,c);
    if (head == NULL) return FALSE;
    monreset(head, FOW);
    if (Rideal == NULL) sToExp(m1, nexp);
    if (!monsearch(head, FOW, nexp)) return FALSE;
    *b = head->umh.mloc->uld.ldc.ci;
    n = head->umh.mloc->umn.mexp;
    for (i=0; i<numvars; ++i) nexp[i] -= n[i];
    expToS(nexp, 0, m2);
    return TRUE;
}

mn_ring(R) /* init at ring level */
ring R;
{
    int numvars;
    unsigned int size;

    numvars = R->nvars;
    size = sizeof(monhead) + (numvars-NVARS)*sizeof(struct mstk);
    if (numvars == 1) size += sizeof(struct mstk);
    R->headStash = open_stash(size,"mon tables: monom heads");
    size = sizeof(mmonom) + (numvars-NVARS)*sizeof(int);
    R->monStash = open_stash(size,"mon tables: monomials");

    R->parenStash = open_stash(sizeof(monparen), "mon tables: parens");
}

mn_unring(R) /* kill at ring level */
ring R;
{
    endof_stash(R->headStash);
    endof_stash(R->monStash);
    endof_stash(R->parenStash);
}

mn_init(M) /* init at module level */
gmatrix M;
{
    M->mn_lodeg = MAXNEG;
    init_array(&M->montab, sizeof(char *));
    init_array(&M->monsyz, sizeof(char *));
}

mn_kill(M) /* kill at module level */
gmatrix M;
{
    int i, len;
    mn_table *a;
    mn_syzes *b;
    arrow p;
    mn_pair *q, *r;

    a = &M->montab;
    len = length(a);
    for (i=1; i<=len; ++i) {
        p = * (arrow *) ref(a,i);
        if (p != NULL) monrefund(p);
    }
    free_array(a);

    b = &M->monsyz;
    len = length(b);
    for (i=1; i<=len; ++i) {
        q = * (mn_pair **) ref(b,i);
        while (q != NULL) {
            r = q->mpp;
            free_slug(pairStash, q);
            q = r;
        }
    }
    free_array(b);
}

void mn_first ()
{
    pairStash = open_stash(sizeof(mn_pair), "monomial syzygies");
}

boolean mn_iscomplete (gmatrix M)
{
    int i, len;
    mn_syzes *b;

    b = &M->monsyz;
    len = length(b);
    for (i=1; i<=len; i++)
        if (* (mn_pair **) ref(b,i) ISNT NULL) {
            return(FALSE);
        }
    return(TRUE);
}

int mn_next_pair (gmatrix M, int deg, char **i1, char **i2)
/* returned baggage */
{
    mn_pair **p, *q;
    mn_syzes *b;

    deg -= M->mn_lodeg;
    b = &M->monsyz;
    if (deg <= 0 || deg > length(b)) return FALSE;
    p = (mn_pair **) ref(b, deg);
    q = *p;
    if (q != NULL) {
        *i1 = q->id1;
        *i2 = q->id2;
        *p = q->mpp;
        free_slug(pairStash, q);
        return TRUE;
    }
    return FALSE;
}
