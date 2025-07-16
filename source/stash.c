// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <setjmp.h> // For jmp_buf type required by parse.h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shared.h"
#include "stash.h"
#include "monitor.h"  // For print(), newline(), printnew(), intflush()
#include "parse.h"    // For parseInt()
#include "shell.h"    // For to_shell()
#include "set.h"      // For verbose, showmem, showpairs, bailout

int showapair = 0; // defined here, used by standard.c

constexpr int BLOCKSIZE = 4400;    // pick any good size
constexpr long POOLSIZE = 64412L;  // pick any good size
constexpr long RESERVE = 40960L;   // leave 40K for toolbox on Mac
constexpr int ALIGN = 0;           // to align .space in struct block

// Use proper alignment for 64-bit systems
constexpr size_t WORD = sizeof(void*);

struct puddle
{
    struct puddle* ppud;
    long pudsize, pudtop;
    char pudmuck[1];
};

struct slug
{
    struct slug* pslug;
};

struct block
{
    struct block* pblock;
    char space[BLOCKSIZE];
};

struct stash
{
    struct slug* slugs;
    struct block* blocks;
    struct stash* prev, * next;
    const char* name;
    int slug_size;
    long n_blocks, n_slugs, slugs_used;
};

struct puddle* wet_puddles = NULL, * dry_puddles = NULL;
struct block* extra_blocks = NULL;
struct stash primal_stash = {
    NULL, NULL, &primal_stash, &primal_stash, "storage allocator", sizeof(struct stash), 0, 0, 0
};

const char* no_mem = "out of memory";
const char* too_big = "slug size too large";

char* last_alloc = NULL;
struct puddle* last_pud;
long last_size;

static long rainy_in = 0, rainy_out = 0;
static void* rainy_day_in = NULL, * rainy_day_out = NULL;

void rainy_day_memory(long in, long out)
{
    // change memory reserves by releasing and reallocating

    if (rainy_in && rainy_day_in)
    {
        report_mem(rainy_day_in, rainy_in * BLOCKSIZE);
        rainy_day_in = NULL;
        rainy_in = 0;
    }
    if (in)
    {
        rainy_day_in = malloc((size_t)(in * BLOCKSIZE));
        if (rainy_day_in)
            rainy_in = in;
    }
    if (rainy_out && rainy_day_out)
    {
        free(rainy_day_out);
        rainy_day_out = NULL;
        rainy_out = 0;
    }
    if (out)
    {
        rainy_day_out = malloc((size_t)(out * 1024));
        if (rainy_day_out)
            rainy_out = out;
    }
} // 8/17/93 DB

void mem_init_once_only(void)
{
    // Cache some memory to use in kaboom()
    if (bailout <= 0)
        rainy_day_memory(2, 4);  // adjust these defaults if insufficient
}                                // 8/17/93 DB

void cachemem_cmd(int argc, char* argv[])
{
    long in, out;

    if (argc != 3)
    {
        printnew("cache_mem <internal allocator blocks> <K's for malloc>\n");
        printf("(currently reserving %ld blocks, and %ldK for malloc)\n", rainy_in, rainy_out);
        return;
    }
    in = parseInt(argv + 1);
    out = parseInt(argv + 2);
    if (in >= 0 && out >= 0)
        rainy_day_memory(in, out);
} // 8/17/93 DB

// Modify kaboom with extreme caution; it has earned its name in past
// incarnations. Many machines have been brought down, causing ire of
// system managers, by infinite loop provoked by an attempt to come up
// for air here. The output typically used all remaining disk space on
// the host machine, and rendered the user persona non grata.

void kaboom(const char* message)
{
    print("\n");
    newline();
    print("!!!!!! %s !!!!!!\n", message);
    if (bailout > 0)
        exit(0);
    else
    {
        ++bailout; // use one life
        if (rainy_in || rainy_out)
        {
            print("Using rainy day memory reserves\n");
            rainy_day_memory(0, 0);
        }
        to_shell();
    }
} // 5/18/89 DB 2/6, 8/17/93 DB

static long totmem = 0; // used only by report_mem(), below

void report_mem(char* p, long n)
// reports existence of memory pool *p, n bytes long
{
    struct puddle* q;

    if (p == NULL)
        kaboom(no_mem);
    totmem += n;
    if ((showmem || verbose) && totmem != n)
    {
        intflush("[%dk]", 1 + (int)(totmem - 1) / 1024L);
        if (showpairs > 1)
            showapair = 1;
    }
    // Ensure proper alignment
    q = (struct puddle*)(void*)p;
    q->ppud = wet_puddles;
    wet_puddles = q;
    n -= q->pudmuck - (char*)q;
    while (n % (long)WORD != 0)
        --n;
    q->pudsize = q->pudtop = n;
} // 5/18/89 DB 1/6

char*gimmy(unsigned size)
{
    struct puddle* p;

    if (wet_puddles == NULL)
        more_mem();
    p = wet_puddles;
    while (p->pudtop < BLOCKSIZE)
    {
        wet_puddles = p->ppud;
        p->ppud = dry_puddles;
        dry_puddles = p;
        if (wet_puddles == NULL)
            more_mem();
        p = wet_puddles;
    }
    while (size % WORD != 0)
        ++size;
    if (size > BLOCKSIZE)
        while (p->pudtop < size)
        {
            if (p->ppud == NULL)
            {
                more_mem();
                p = wet_puddles;
                break;
            }
            else
                p = p->ppud;
        }
    p->pudtop -= size;
    last_alloc = &p->pudmuck[p->pudtop];
    last_pud = p;
    last_size = size;
    return last_alloc;
}

void ungimmy(char* p)
{
    if (last_alloc == p)
        last_pud->pudtop += last_size;
}

void reset_mem(void)
{
    struct puddle* p;

    if (wet_puddles != NULL)
    {
        for (p = wet_puddles; p->ppud != NULL; p = p->ppud)
            ;
        p->ppud = dry_puddles;
    }
    else
    {
        wet_puddles = dry_puddles;
    }
    dry_puddles = NULL;
    for (p = wet_puddles; p != NULL; p = p->ppud)
        p->pudtop = p->pudsize;
    last_alloc = NULL;
    extra_blocks = NULL;
    primal_stash.slugs = NULL;
    primal_stash.blocks = NULL;
    primal_stash.prev = &primal_stash;
    primal_stash.next = &primal_stash;
    primal_stash.n_blocks = 0L;
    primal_stash.n_slugs = 0L;
    primal_stash.slugs_used = 0L;
}

void init_mem(void)
{
}

void more_mem(void)
// system dependent code for getting another memory pool
{
    char* p;
    p = malloc((size_t)POOLSIZE);
    report_mem(p, POOLSIZE);
}

char*open_stash(int size, const char* name)
{
    register struct stash* p;

    p = (struct stash*)(void*)get_slug(&primal_stash);
    if (p == NULL)
        return NULL;
    p->slugs = NULL;
    p->blocks = NULL;

    // hook stash into linked list
    p->prev = &primal_stash;
    p->next = primal_stash.next;
    p->prev->next = p;
    p->next->prev = p;

    p->name = name;
    p->n_blocks = 0;
    p->n_slugs = 0;
    p->slugs_used = 0;
    while (size % (int)WORD != 0)
        ++size;
    p->slug_size = size > (int)sizeof(char*) ? size : (int)sizeof(char*);
    return (char*)p;
}

char*get_slug(struct stash* pstash)
{
    register struct stash* p;
    register struct slug* q;

    if ((p = pstash) == NULL)
        return NULL;
    if (p->slugs == NULL)
    {
        get_block(p);
        if (p->slugs == NULL)
            return NULL;
    }
    q = p->slugs;
    p->slugs = q->pslug;
    ++p->slugs_used;
    return (char*)q;
}

void memory_bomb(void) // do not try this at home
{
    int i, j;

    printf("\n");
    for (i = 1024, j = 0; i >= 32; i /= 2)
    {
        while (malloc((size_t)(i * 1024)) != NULL)
            j += i;
        printf("[%12d]\n", j * 1024);
        fflush(stdout);
    }
    printf("[%12d] DONE.\n", j);
    fflush(stdout);
}

#ifdef COMMENT
static int fuse = 10;
if (--fuse == 0)
    memory_bomb(); // for get_block()
#endif

void get_block(struct stash* pstash)
{
    register struct stash* p;
    register struct slug* q, * q2;
    register struct block* r;
    register int i, j;
    int count, k;

    p = pstash;
    j = p->slug_size;
    if (j > BLOCKSIZE - ALIGN)
        kaboom(too_big);
    if (extra_blocks == NULL)
        if (!get_mem(p))
            return;
    r = extra_blocks;
    extra_blocks = r->pblock;
    q = (struct slug*)(void*)(r->space + ALIGN);
    count = 1;
    q->pslug = NULL;
    k = BLOCKSIZE - j;
    i = j + ALIGN;
    while (i <= k)
    {
        q2 = (struct slug*)(void*)(r->space + i);
        ++count;
        q2->pslug = q;
        q = q2;
        i += j;
    }
    p->slugs = q;
    r->pblock = p->blocks;
    p->blocks = r;
    ++p->n_blocks;
    p->n_slugs += count;
}

int get_mem(struct stash* pstash)
{
    register struct block* r;

    (void)pstash; // unused parameter
    r = (struct block*)(void*)gimmy(sizeof(struct block));
    if (r == NULL)
        return 0;
    r->pblock = extra_blocks;
    extra_blocks = r;
    return 1;
}

void free_slug(struct stash* pstash, struct slug* ptr)
{
    ptr->pslug = pstash->slugs;
    pstash->slugs = ptr;
    --pstash->slugs_used;
}

void endof_stash(struct stash* pstash)
{
    register struct block* p;

    p = pstash->blocks;
    if (p != NULL)
    {
        while (p->pblock != NULL)
            p = p->pblock;

        p->pblock = extra_blocks;
        extra_blocks = pstash->blocks;
    }

    // unhook stash from linked list
    pstash->prev->next = pstash->next;
    pstash->next->prev = pstash->prev;

    free_slug(&primal_stash, (struct slug*)pstash);
}

static inline long K(long A)
{
    return (1023 + A) / 1024;
}

static inline long PUP(long A, long B)
{
    return B ? (100 * A + B - 1) / B : 100;
}

static inline long PDN(long A, long B)
{
    return B ? (100 * A) / B : 100;
}

void space_cmd(int argc)
{
    long b_alloc, b_stash, b_free, k_alloc, k_stash, k_avail, k_used;
    long p_stash, p_avail, p_used, k_this, p_this;
    // k_free and p_free computed but not used - preserved for historical accuracy
    long k_free __attribute__((unused));
    long p_free __attribute__((unused));
    struct block* b;
    struct stash* p;
    struct puddle* q;

    // do below when extra argument
    if (argc > 1)
    {
        print("       #blocks  #slugs   #used    size   %%used\n");
        b_stash = b_free = k_avail = k_used = 0;
        for (b = extra_blocks; b != NULL; b = b->pblock)
            ++b_free;
        b_alloc = b_free;
        p = &primal_stash;
        do
        {
            b_stash += p->n_blocks;
            k_this = p->n_blocks * (long)sizeof(struct block);
            k_avail += p->n_slugs * p->slug_size;
            k_used += p->slugs_used * p->slug_size;
            p_this = PUP(p->slugs_used, p->n_slugs);
            print("%5ldK%8ld%8ld%8ld%8d%7ld%%  %s\n", K(k_this), p->n_blocks, p->n_slugs,
                  p->slugs_used, p->slug_size, p_this, p->name);
            p = p->prev;
        } while (p != &primal_stash);
        b_alloc += b_stash;
        k_alloc = b_alloc * (long)sizeof(struct block);
        k_stash = b_stash * (long)sizeof(struct block);
        k_free = b_free * (long)sizeof(struct block);
        p_stash = PUP(b_stash, b_alloc);
        p_free = PDN(b_free, b_alloc);
        p_avail = PUP(k_avail, k_stash);
        p_used = PUP(k_used, k_avail);
        print("%5ldK   %5ld  blocks under control of storage allocator\n", K(k_alloc), b_alloc);
        print("%5ldK   %5ld  blocks in use    ", K(k_stash), b_stash);
        print("(%4ld%% of allocated blocks )\n", p_stash);
        print("%5ldK  allocated into slugs     ", K(k_avail));
        print("(%4ld%% yield               )\n", p_avail);
        print("%5ldK  slugs in actual use      ", K(k_used));
        print("(%4ld%% of allocated slugs  )\n", p_used);
    }

    // do below always
    b_alloc = b_free = 0;
    for (q = wet_puddles; q != NULL; q = q->ppud)
    {
        b_alloc += q->pudsize;
        b_free += q->pudtop;
    }
    for (q = dry_puddles; q != NULL; q = q->ppud)
    {
        b_alloc += q->pudsize;
        b_free += q->pudtop;
    }
    print("%5ldK  total memory requested by allocator\n", K(b_alloc));
    print("%5ldK  of requested memory not yet used\n", K(b_free));
} /* 2/10/89 DB */ /* 2/20/89 MES newline's added */

// The following is being kept only temporarily...
// space_cmd()
// {
// [Original commented-out version omitted for brevity]
// }
