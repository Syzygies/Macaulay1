// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "generic.h"
#include "ring.h"
#include "vars.h"
#include "plist.h"
#include "boxprocs.h"

// Constants
constexpr int NTYPES = 18;

const char* type_names[] = { "none",    "int",         "ring",    "poly",          "matrix",
                             "complex", "computation", "emitter", "emit standard", "res",
                             "shift",   "collector",   "trash",   "lifter",        "merger",
                             "nres",    "std",         "istd" };

// Type-safe vtable implementations

// Helper for do-nothing handlers
static void do_nothing_void(void* obj)
{
    (void)obj;
}

// Helper for do-nothing boolean handlers
static boolean do_nothing_boolean(void* obj, int deg)
{
    (void)obj;
    (void)deg;
    return TRUE;
}

// Wrapper for int_type which expects int, not void*
static void int_type_wrapper(void* obj)
{
    // In the original code, this is called with an int cast to pfi
    // We need to handle this carefully for 64-bit systems
    intptr_t n = (intptr_t)obj;
    int_type((int)n);
}

// VNOTYPE (0) - No operations
static const object_vtable null_vtable = { .get_type = NULL,
                                           .kill = NULL,
                                           .msg2 = { .do_degree = NULL },
                                           .poly_receive = NULL,
                                           .end_degree = NULL };

// VINT (1)
static const object_vtable int_vtable = { .get_type = int_type_wrapper,
                                          .kill = do_nothing_void,
                                          .msg2 = { .do_degree = NULL },
                                          .poly_receive = NULL,
                                          .end_degree = NULL };

// VRING (2)
static const object_vtable ring_vtable = { .get_type = (void (*)(void*)) vrg_type,
                                           .kill = (void (*)(void*)) rgKill,
                                           .msg2 = { .do_degree = NULL },
                                           .poly_receive = NULL,
                                           .end_degree = NULL };

// VPOLY (3)
static const object_vtable poly_vtable = { .get_type = do_nothing_void,
                                           .kill = do_nothing_void,
                                           .msg2 = { .do_degree = NULL },
                                           .poly_receive = NULL,
                                           .end_degree = NULL };

// VMODULE (4)
static const object_vtable module_vtable = { .get_type = (void (*)(void*)) vmod_type,
                                             .kill = (void (*)(void*)) mod_kill,
                                             .msg2 = { .do_degree = NULL },
                                             .poly_receive = NULL,
                                             .end_degree = NULL };

// VCOMPLEX (5) - Not implemented in original
static const object_vtable complex_vtable = { .get_type = NULL,
                                              .kill = NULL,
                                              .msg2 = { .do_degree = NULL },
                                              .poly_receive = NULL,
                                              .end_degree = NULL };

// VSTARTER (6)
static const object_vtable starter_vtable = {
    .get_type = (void (*)(void*)) st_pprint,
    .kill = (void (*)(void*)) st_kill,
    .msg2 = { .start = (void (*)(void*, int, char**)) st_start },
    .poly_receive = NULL,
    .end_degree = NULL
};

// VEMIT (7)
static const object_vtable emit_vtable = {
    .get_type = (void (*)(void*)) alias_type,
    .kill = do_nothing_void,
    .msg2 = { .do_degree = (boolean (*)(void*, int)) emit_dodeg },
    .poly_receive = NULL,
    .end_degree = NULL
};

// VSTDEMIT (8)
static const object_vtable stdemit_vtable = {
    .get_type = (void (*)(void*)) alias_type,
    .kill = do_nothing_void,
    .msg2 = { .do_degree = (boolean (*)(void*, int)) semit_dodeg },
    .poly_receive = NULL,
    .end_degree = NULL
};

// VRES (9)
static const object_vtable res_vtable = { .get_type = (void (*)(void*)) vmod_type,
                                          .kill = (void (*)(void*)) mod_kill,
                                          .msg2 = { .do_degree = (boolean (*)(void*,
                                                                              int)) res_dodeg },
                                          .poly_receive = NULL,
                                          .end_degree = NULL };

// VSHIFT (10)
static const object_vtable shift_vtable = {
    .get_type = do_nothing_void,
    .kill = do_nothing_void,
    .msg2 = { .do_degree = (boolean (*)(void*, int)) shift_dodeg },
    .poly_receive = NULL,
    .end_degree = NULL
};

// VCOLLECT (11)
static const object_vtable collect_vtable = {
    .get_type = (void (*)(void*)) vmod_type,
    .kill = (void (*)(void*)) mod_kill,
    .msg2 = { .degs_receive = (void (*)(void*, void*, int)) coll_degs },
    .poly_receive = (void (*)(void*, poly, int)) coll_poly,
    .end_degree = do_nothing_boolean
};

// VTRASH (12)
static const object_vtable trash_vtable = { .get_type = do_nothing_void,
                                            .kill = do_nothing_void,
                                            .msg2 = { .do_degree = NULL },
                                            .poly_receive = (void (*)(void*, poly, int)) trash_poly,
                                            .end_degree = do_nothing_boolean };

// VLIFT (13)
static const object_vtable lift_vtable = {
    .get_type = (void (*)(void*)) alias_type,
    .kill = do_nothing_void,
    .msg2 = { .degs_receive = (void (*)(void*, void*, int)) mg_degs },
    .poly_receive = (void (*)(void*, poly, int)) lift_poly,
    .end_degree = (boolean (*)(void*, int)) lift_enddeg
};

// VMERGE (14)
static const object_vtable merge_vtable = {
    .get_type = do_nothing_void,
    .kill = do_nothing_void,
    .msg2 = { .degs_receive = (void (*)(void*, void*, int)) mg_degs },
    .poly_receive = (void (*)(void*, poly, int)) mg_poly,
    .end_degree = (boolean (*)(void*, int)) mg_enddeg
};

// VNRES (15)
static const object_vtable nres_vtable = {
    .get_type = (void (*)(void*)) vmod_type,
    .kill = (void (*)(void*)) mod_kill,
    .msg2 = { .degs_receive = (void (*)(void*, void*, int)) coll_degs },
    .poly_receive = (void (*)(void*, poly, int)) ins_nres,
    .end_degree = (boolean (*)(void*, int)) nres_enddeg
};

// VSTD (16)
static const object_vtable std_vtable = {
    .get_type = (void (*)(void*)) vmod_type,
    .kill = (void (*)(void*)) mod_kill,
    .msg2 = { .degs_receive = (void (*)(void*, void*, int)) std_degs },
    .poly_receive = (void (*)(void*, poly, int)) ins_nres,
    .end_degree = (boolean (*)(void*, int)) std_enddeg
};

// VISTD (17)
static const object_vtable istd_vtable = {
    .get_type = (void (*)(void*)) vmod_type,
    .kill = (void (*)(void*)) mod_kill,
    .msg2 = { .do_degree = (boolean (*)(void*, int)) istd_dodeg },
    .poly_receive = NULL,
    .end_degree = NULL
};

// Static vtable array
static const object_vtable* vtables[NTYPES] = {
    [VNOTYPE] = &null_vtable,     [VINT] = &int_vtable,       [VRING] = &ring_vtable,
    [VPOLY] = &poly_vtable,       [VMODULE] = &module_vtable, [VCOMPLEX] = &complex_vtable,
    [VSTARTER] = &starter_vtable, [VEMIT] = &emit_vtable,     [VSTDEMIT] = &stdemit_vtable,
    [VRES] = &res_vtable,         [VSHIFT] = &shift_vtable,   [VCOLLECT] = &collect_vtable,
    [VTRASH] = &trash_vtable,     [VLIFT] = &lift_vtable,     [VMERGE] = &merge_vtable,
    [VNRES] = &nres_vtable,       [VSTD] = &std_vtable,       [VISTD] = &istd_vtable
};

// Type-safe vtable dispatch functions
const object_vtable*get_vtable(int type)
{
    if (type < 0 || type >= NTYPES)
        return NULL;
    return vtables[type];
}

void*get_message_handler(int type, int message)
{
    if (type < 0 || type >= NTYPES)
        return NULL;

    const object_vtable* vtable = vtables[type];
    if (!vtable)
        return NULL;

    switch (message)
    {
    case MTYPE:
        return (void*)vtable->get_type;
    case MKILL:
        return (void*)vtable->kill;
    case MSTART:  // Same as MDODEG and MDEGS_RECEIVE (all are 2)
                  // These share the same slot in the union
        return (void*)vtable->msg2.do_degree;
    case MPOLY_RECEIVE:
        return (void*)vtable->poly_receive;
    case MENDDEG:
        return (void*)vtable->end_degree;
    default:
        return NULL;
    }
}
