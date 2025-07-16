// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Array size constants
constexpr int NVARS = 20;
constexpr int GFSIZE = 200;
constexpr int NCOMS = 5000;
constexpr int NDEGS = 400;
constexpr int NARRAY = 600;

// General definitions
typedef int boolean;
constexpr int TRUE = -1;
constexpr int FALSE = 0;

// Comparison results
constexpr int LT = -1;
constexpr int EQ = 0;
constexpr int GT = 1;

// Function pointer type
typedef int (*pfi)(void);

// Min/Max functions
static inline int min_int(int a, int b)
{
    return (a < b) ? a : b;
}

static inline int max_int(int a, int b)
{
    return (a > b) ? a : b;
}

static inline long min_long(long a, long b)
{
    return (a < b) ? a : b;
}

static inline long max_long(long a, long b)
{
    return (a > b) ? a : b;
}

// Debug assertion function - modern C23 replacement
static inline void debug_assert(const char* msg, bool condition)
{
#ifdef DEBUG
    if (!condition)
    {
        fprintf(stderr, "Assertion failed: %s\n", msg);
        abort();
    }
#else
    (void)msg;
    (void)condition;
#endif
}

// Array structures
typedef struct slabrec
{
    char vals[NARRAY];
    struct slabrec* next;
} slab;

typedef struct
{
    int len;
    int size;
    int val_size;
    slab* elems;
} array;

typedef array plist;
typedef array dlist;

// Forward declarations
typedef unsigned long smallmon;
typedef smallmon* term;
typedef struct symblock* symmBlock;
typedef struct ringrec* ring;
typedef struct pol* poly;
typedef struct modrec* gmatrix;
typedef struct mn_stdrec* mn_standard;
typedef union all* arrow;
typedef void* ADDRESS;
typedef struct varb_rec variable;

// Ring type constants
constexpr int RSYMM = -1;
constexpr int RCOMP = -2;
constexpr int RWTFCN = -3;
constexpr int LARGE_PRIME = 32749;

// Module type constants
constexpr int MAXNEG = -30000;
constexpr int MNOTHING = -1;
constexpr int MMAT = 0;
constexpr int MSTD = 1;
constexpr int MISTD = 2;

// Iterator type constants
constexpr int USEMAT = 0;
constexpr int USESTD = 1;
constexpr int USECHANGE = 2;

// Constants for forward/backward links
constexpr int FOW = 0;
constexpr int BAK = 1;

// Symmetric block structure
struct symblock
{
    int rtyp;
    int nvars;
    int* degs;
    char** names;
    smallmon* table;
    smallmon** cols;
    smallmon maxval;
    int w1;
    int maxdeg;
    int defaultmax;
    int over;
    int n1;
    int degree;
    void (*stob)(void);
    smallmon (*btos)(void);
    smallmon (*madd)(void);
    symmBlock nextBlock;
};

// Ring structure
struct ringrec
{
    int charac;
    int nvars;
    char** varnames;
    dlist degs;
    dlist monorder;
    dlist wtfcns;
    int nblocks;
    symmBlock* symmalg;
    int compLoc;
    term zerodegs;
    void* pStash;
    void* headStash;
    void* monStash;
    void* parenStash;
    void* Rideal;
    void* Rstd;
};

// Polynomial structure
struct pol
{
    struct pol* next;
    short coef;
    void* monom;
};

// Standard basis element
struct mn_stdrec
{
    mn_standard next;
    poly standard;
    poly change;
    char ismin;
    int degree;
};

// Module generator iterator
typedef struct
{
    char mtype;
    int misstd;
    mn_standard mp;
    arrow mq;
    array* mmontab;
    int mi;
    plist* mgens;
} modgen;

// Module/matrix type
struct modrec
{
    struct modrec* next;
    int modtype;
    dlist degrees;
    dlist deggens;
    plist gens;
    int nstandard;
    mn_standard stdbasis;
    array montab;
    array monsyz;
    int mn_lodeg;
};

// Variable existence state constants (for varb_rec.exists field)
// From original mtypes.h
constexpr int REMOVED = 0;  // Variable has been removed/deleted
constexpr int MAINVAR = 1;  // Main/primary variable (e.g., 'm', 'R')
constexpr int PARTVAR = 2;  // Partial/indexed variable (e.g., 'm.2')

// Variable structure
struct varb_rec
{
    char exists;
    char garbage;
    int var_num;
    char* name;
    int type;
    variable* b_ring;
    variable* b_next;
    variable* b_alias;
    ADDRESS value;
    int intval;
};

// Cargo type
typedef union
{
    char* ci;
    union all* ca;
} cargo;

// Base structure for linked data - replaces LINKDUK macro
typedef struct linkduk_base
{
    union all* lda[2];
    cargo ldc;
    char ldkind;
} linkduk_base;

// For compatibility, define linkduk as alias
typedef linkduk_base linkduk;

// Type definitions
typedef int expterm[NVARS];
typedef short field;
typedef int bigterm[NVARS];
typedef unsigned long allocterm[NVARS];
typedef array mn_table;
typedef array mn_syzes;

// Monomial structures
typedef struct
{
    linkduk_base base; // Common fields first
    expterm mexp;
} mmonom;

typedef struct
{
    linkduk_base base; // Common fields first
    int mlev;
} monparen;

struct mstk
{
    short int mi, mpre;
    union all* ma;
};

typedef struct
{
    linkduk_base base; // Common fields first
    union all* mloc;
    int mn, mii, mimin, mpren, mpred;
    struct mstk mstack[NVARS - 1];
} monhead;

// Complete union all
union all
{
    int ui;
    union all* ua;
    cargo ucg;
    linkduk_base uld;
    mmonom umn;
    monparen ump;
    monhead umh;
};

// Monomial pair structure
typedef struct mn_pr
{
    char* id1, * id2;
    struct mn_pr* mpp;
} mn_pair;

// Global variables
extern char* array_stash;
extern int numvars;
extern arrow Rideal;
extern array var_list;
extern variable* current_ring;

// Array functions
void array_init(void);
void init_array(array* a, int element_size);
void free_array(array* a);
int length(array* a);
void*ref(array* a, int i);
void*ins_array(array* a);

// Inline array access functions
static inline int array_length(array* a)
{
    return a->len;
}

static inline poly plist_ref(plist* pl, int i)
{
    return *(poly*)ref((array*)pl, i);
}

static inline int dlist_ref(dlist* dl, int i)
{
    return *(int*)ref((array*)dl, i);
}

static inline void plist_init(plist* pl)
{
    init_array((array*)pl, sizeof(poly));
}

static inline void dlist_init(dlist* dl)
{
    init_array((array*)dl, sizeof(int));
}

static inline void plist_insert(plist* pl, poly f)
{
    *((poly*)ins_array((array*)pl)) = f;
}

static inline void dlist_insert(dlist* dl, int i)
{
    *((int*)ins_array((array*)dl)) = i;
}

static inline poly*plist_ref_ptr(plist* pl, int i)
{
    return (poly*)ref((array*)pl, i);
}

static inline void plist_set(plist* pl, int i, poly f)
{
    *plist_ref_ptr(pl, i) = f;
}

static inline void dlist_set(dlist* dl, int i, int val)
{
    *(int*)ref((array*)dl, i) = val;
}

// Polynomial access functions
static inline term poly_initial(poly f)
{
    return (term)(f->monom);
}

static inline short poly_lead_coef(poly f)
{
    return f->coef;
}

// Variable access functions
static inline int var_get_int(variable* v)
{
    return v->intval;
}

static inline void var_set_int(variable* v, int val)
{
    v->intval = val;
}

static inline poly var_get_poly(variable* v)
{
    return (poly)(v->value);
}

static inline gmatrix var_get_module(variable* v)
{
    return (gmatrix)(v->value);
}

static inline ring var_get_ring(variable* v)
{
    return (ring)(v->value);
}

static inline variable*vref(int n)
{
    return (variable*)ref(&var_list, n);
}

// Common polynomial accessor - used by multiple modules
static inline term initial(poly f)
{
    return (term)(f->monom);
}
