// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "array.h"
#include "stash.h"
#include "monitor.h"

// Verify we're on a 64-bit system
static_assert(sizeof(void *) == 8, "64-bit system required");

// Global array stash
char *array_stash;

void array_init(void)
{
    array_stash = open_stash(NARRAY + sizeof(struct slabrec *), "arrays");
}

void init_array(array *a, int element_size)
{
    a->len = 0;
    a->size = element_size;
    a->elems = nullptr;
    if (element_size == 0)
        a->val_size = NARRAY;
    else
        a->val_size = (NARRAY / element_size) * element_size;
}

void free_array(array *a)
{
    slab *p = a->elems;
    slab *temp;

    while (p != nullptr)
    {
        temp = p;
        p = p->next;
        free_slug((struct stash *)array_stash, (struct slug *)temp);
    }
    init_array(a, 0);
}

int length(array *a)
{
    return a->len;
}

void *ref(array *a, int i)
{
    slab *p = a->elems;
    int where = (a->size) * (i - 1);
    int narray = a->val_size;

    if ((i <= 0) || (i > a->len))
    {
        prerror(";internal error: array ref out of bounds: index = ");
        prerror("%d, len = %d , size = %d\n", i, a->len, a->size);
        return nullptr;
    }
    while (where >= narray)
    {
        where -= narray;
        p = p->next;
    }
    return &(p->vals[where]);
}

void *ins_array(array *a)
{
    slab *p;
    int where;
    int narray = a->val_size;

    if (a->len == 0)
    {
        p = (slab *)(void *)get_slug((struct stash *)array_stash);
        p->next = nullptr;
        a->elems = p;
        where = 0;
    }
    else
    {
        p = a->elems;
        where = (a->size) * (a->len);
        while (where >= narray)
        {
            where -= narray;
            if (where == 0)
            {
                p->next = (slab *)(void *)get_slug((struct stash *)array_stash);
                p->next->next = nullptr;
            }
            p = p->next;
        }
    }
    a->len++;
    return &(p->vals[where]);
}
