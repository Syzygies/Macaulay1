/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "mtypes.h"

// void array_init ();
// void init_array (array *a, int element_size);
// void free_array (array *a);
// int length (array *a);
// char *ref (array *a, int i);
// char *ins_array (array *a);

void array_init ()
{
    array_stash = open_stash(NARRAY + sizeof(struct slabrec *),
                             "arrays");
}

void init_array (array *a, int element_size)
{
    a->len = 0;
    a->size = element_size;
    a->elems = NULL;
    if (element_size IS 0) a->val_size = NARRAY;
    else a->val_size = (NARRAY / element_size) * element_size;
}

void free_array (array *a)
{
    slab *p;
    slab *temp;

    p = a->elems;
    while (p ISNT NULL) {
        temp = p;
        p = p->next;
        free_slug(array_stash, temp);
    }
    init_array(a, 0);
}

int length (array *a)
{
    return(a->len);
}

char *ref (array *a, int i)
{
    slab *p = a->elems;
    int where = (a->size)*(i-1);
    int narray = a->val_size;

    if ((i <= 0) OR (i > a->len)) {
        prerror(";internal error: array ref out of bounds: index = ");
        prerror("%d, len = %d , size = %d\n", i, a->len, a->size);
        return(NULL);
    }
    while (where >= narray) {
        where -= narray;
        p = p->next;
    }
    return(&(p->vals[where]));
}

char *ins_array (array *a)
{
    slab *p;
    int where;
    int narray = a->val_size;

    if (a->len IS 0) {
        p = (slab *) get_slug(array_stash);
        p->next = NULL;
        a->elems = p;
        where = 0;
    } else {
        p = a->elems;
        where = (a->size)*(a->len);
        while (where >= narray) {
            where -= narray;
            if (where IS 0) {
                p->next = (slab *) get_slug(array_stash);
                p->next->next = NULL;
            }
            p = p->next;
        }
    }
    a->len++;
    return(&(p->vals[where]));
}
