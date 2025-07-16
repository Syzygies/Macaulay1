// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "shared.h"
#include "cmdnames.h"

// Include all headers that define command functions
#include <setjmp.h> // For jmp_buf type needed by shell.h
#include "cmds.h"
#include "stdcmds.h"
#include "rescmds.h"
#include "spcmds.h"
#include "betti.h"
#include "help.h"
#include "ring.h"
#include "qring.h"
#include "vars.h"
#include "set.h"
#include "shell.h"
#include "dot_fns.h"
#include "ed.h"
#include "human_io.h"
#include "input.h"
#include "hilb.h"
#include "hilb2.h"
#include "poly.h"
#include "prpoly.h"
#include "gm_cmds.h"
#include "gmatrix.h"
#include "boxcmds.h"
#include "stash.h"
#include "det.h"
#include "getdet.h"
#include "koszul.h"
#include "intersect.h"
#include "inhomog.h"
#include "charp.h"
#include "rmap.h"
#include "monideal.h"
#include "stats.h"
#include "hulb.h"
#include "monitor.h"

// To add a command:
// 1. Define the command function in the appropriate module (e.g., cmds.c, stdcmds.c)
// 2. Add the function declaration to that module's header file
// 3. Include that header file above
// 4. Add a cmd_rec entry below

// Multiple names for the same command are allowed.

// "args" is the preferred number of arguments when the command sets
// a variable, rather than reports to the user. This value is used
// in "dot_fns.c" to create temporary arguments automatically for
// return values in dot mode; to disable this feature, set "args" to 0.
// "args" is only helpful for commands which (optionally or not) return
// results via their last argument(s). Most but not all commands behave
// this way; please write any new commands following this protocol.
// The current offenders which could but don't use their last arg(s) are
// "imap", "int", "poly", "rmap", and "sat"; it is too late to change them.

// For example, "characteristic" can be called with either 1 or 2 args.
// If a 2nd arg is provided, that arg is set to the characteristic.
// Thus the correct value for "args" is 2.

// Suppress function cast warnings - legacy design uses both void and int returns
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
#pragma clang diagnostic ignored "-Wcast-function-type"
#endif
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

cmd_rec cmd_list[] = {{".", (cmd_func)dot_cmd, 0},
                      {"ac", (cmd_func)ac_cmd, 0},
                      {"add", (cmd_func)add_cmd, 3},
                      {"ar", (cmd_func)ar_cmd, 0},
                      {"args", (cmd_func)args_cmd, 0},
                      {"betti", (cmd_func)betti_cmd, 0},
                      {"binoms", (cmd_func)binoms_cmd, 3},
                      {"cache_mem", (cmd_func)cachemem_cmd, 0},
                      {"calc", (cmd_func)calc_cmd, 0},
                      {"cat", (cmd_func)cat_cmd, 2},
                      {"cdir", (cmd_func)cdir_cmd, 0},
                      {"ce", (cmd_func)ce_cmd, 0},
                      {"characteristic", (cmd_func)charac_cmd, 2},
                      {"chcalc", (cmd_func)chcalc_cmd, 0},
                      {"codim", (cmd_func)codim_cmd, 2},
                      {"coef", (cmd_func)coef_cmd, 3},
                      {"col_degree", (cmd_func)coldeg_cmd, 3},
                      {"col_degs", (cmd_func)coldegs_cmd, 0},
                      {"commands", (cmd_func)commands_cmd, 0},
                      {"compress", (cmd_func)compress_cmd, 2},
                      {"concat", (cmd_func)concat_cmd, 0},
                      {"continue", (cmd_func)continue_cmd, 0},
                      {"contract", (cmd_func)diff_cmd, 3},
                      {"copy", (cmd_func)copy_cmd, 2},
                      {"degree", (cmd_func)degree_cmd, 3},
                      {"determinants", (cmd_func)determinants_cmd, 3},
                      {"diag", (cmd_func)diag_cmd, 2},
                      {"diff", (cmd_func)diff_cmd, 3},
                      {"dshift", (cmd_func)dshift_cmd, 0},
                      {"dsum", (cmd_func)dsum_cmd, 3},
                      {"echo", (cmd_func)ech_cmd, 0},
                      {"edit", (cmd_func)edit_cmd, 0},
                      {"edit_map", (cmd_func)modrmap_cmd, 0},
                      {"elim", (cmd_func)elim_cmd, 2},
                      {"endmon", (cmd_func)end_monitor, 0},
                      {"ev", (cmd_func)ev_cmd, 3},
                      {"exit", (cmd_func)exit_cmd, 0},
                      {"fetch", (cmd_func)fetch_cmd, 2},
                      {"flatten", (cmd_func)flatten_cmd, 2},
                      {"forcestd", (cmd_func)forcestd_cmd, 2},
                      {"help", (cmd_func)help_cmd, 0},
                      {"help_file", (cmd_func)helpfile_cmd, 0},
                      {"hilb", (cmd_func)hilb_cmd, 0},
                      {"hilb_numer", (cmd_func)hilb_numer_cmd, 3},
                      {"homog", (cmd_func)homog_cmd, 3},
                      {"hulb", (cmd_func)hulb_cmd, 0},
                      {"ideal", (cmd_func)ideal_cmd, 1},
                      {"iden", (cmd_func)iden_cmd, 2},
                      {"if", (cmd_func)if_cmd, 0},
                      {"imap", (cmd_func)imap_cmd, 0},
                      {"in", (cmd_func)in_cmd, 2},
                      {"incr_set", (cmd_func)incset_cmd, 0},
                      {"inpart", (cmd_func)inpart_cmd, 2},
                      {"int", (cmd_func)int_cmd, 0},
                      {"intersect", (cmd_func)inter_cmd, 3},
                      {"is_zero", (cmd_func)iszero_cmd, 2},
                      {"jacob", (cmd_func)jacob_cmd, 2},
                      {"jump", (cmd_func)jump_cmd, 0},
                      {"k_basis", (cmd_func)basis_cmd, 2},
                      {"keep", (cmd_func)keep_cmd, 2},
                      {"kill", (cmd_func)kill_cmd, 0},
                      {"koszul", (cmd_func)koszul_cmd, 3},
                      {"lift", (cmd_func)lift_cmd, 3},
                      {"lift_std", (cmd_func)liftstd_cmd, 2},
                      {"listvars", (cmd_func)listvars_cmd, 0},
                      {"mat", (cmd_func)mat_cmd, 1},
                      {"max", (cmd_func)max_cmd, 2},
                      {"mc", (cmd_func)mc_cmd, 0},
                      {"min", (cmd_func)min_cmd, 2},
                      {"modulo", (cmd_func)modulo_cmd, 3},
                      {"monitor", (cmd_func)mon_cmd, 0},
                      {"monoms", (cmd_func)monoms_cmd, 3},
                      {"monprimes", (cmd_func)monprimes_cmd, 3},
                      {"mr", (cmd_func)mr_cmd, 0},
                      {"mult", (cmd_func)mult_cmd, 3},
                      {"ncols", (cmd_func)ncols_cmd, 2},
                      {"nres", (cmd_func)nres_cmd, 2},
                      {"nrows", (cmd_func)nrows_cmd, 2},
                      {"numinfo", (cmd_func)numinfo_cmd, 0},
                      {"nvars", (cmd_func)nvars_cmd, 2},
                      {"outer", (cmd_func)outer_cmd, 3},
                      {"path", (cmd_func)path_cmd, 0},
                      {"pc", (cmd_func)pc_cmd, 0},
                      {"pfaff", (cmd_func)pfaff_cmd, 2},
                      {"pmap", (cmd_func)pmap_cmd, 0},
                      {"poly", (cmd_func)poly_cmd, 0},
                      {"power", (cmd_func)power_cmd, 3},
                      {"pr", (cmd_func)pr_cmd, 0},
                      {"pres", (cmd_func)pres_cmd, 0},
                      {"present_ring", (cmd_func)presentring_cmd, 2},
                      {"pring", (cmd_func)pring_cmd, 0},
                      {"prmat", (cmd_func)prmat_cmd, 0},
                      {"putchange", (cmd_func)putchange_cmd, 2},
                      {"putmat", (cmd_func)putmat_cmd, 0},
                      {"putstd", (cmd_func)putstd_cmd, 2},
                      {"qring", (cmd_func)qring_cmd, 2},
                      {"quit", (cmd_func)exit_cmd, 0},
                      {"quotient", (cmd_func)quot_cmd, 3},
                      {"random", (cmd_func)random_cmd, 3},
                      {"reduce", (cmd_func)reduce_cmd, 3},
                      {"res", (cmd_func)res_cmd, 2},
                      {"reset", (cmd_func)reset_cmd, 0},
                      {"ring", (cmd_func)ring_cmd, 1},
                      {"ring_from_cols", (cmd_func)ringcol_cmd, 2},
                      {"ring_from_rows", (cmd_func)ringrow_cmd, 2},
                      {"ring_sum", (cmd_func)ringsum_cmd, 3},
                      {"rmap", (cmd_func)rmap_cmd, 0},
                      {"row_degree", (cmd_func)rowdeg_cmd, 3},
                      {"row_degs", (cmd_func)rowdegs_cmd, 0},
                      {"sat", (cmd_func)sat_cmd, 0},
                      {"set", (cmd_func)set_cmd, 0},
                      {"set_value", (cmd_func)setvalue_cmd, 2},
                      {"setcoldegs", (cmd_func)setcoldegs_cmd, 0},
                      {"setdegs", (cmd_func)setdegs_cmd, 0},
                      {"setring", (cmd_func)setring_cmd, 0},
                      {"shout", (cmd_func)shout_cmd, 0},
                      {"size", (cmd_func)size_cmd, 0},
                      {"smult", (cmd_func)smult_cmd, 3},
                      {"space", (cmd_func)space_cmd, 0},
                      {"spairs", (cmd_func)spairs_cmd, 0},
                      {"spare", (cmd_func)spare_cmd, 0},
                      {"sparse", (cmd_func)sparse_cmd, 1},
                      {"std", (cmd_func)std_cmd, 2},
                      {"std_minimal", (cmd_func)stdmin_cmd, 2},
                      {"stdpart", (cmd_func)stdpart_cmd, 2},
                      {"submat", (cmd_func)submat_cmd, 2},
                      {"subtract", (cmd_func)sub_cmd, 3},
                      {"syz", (cmd_func)syz_cmd, 3},
                      {"tensor", (cmd_func)tensor_cmd, 3},
                      {"trace", (cmd_func)trace_cmd, 2},
                      {"transpose", (cmd_func)trans_cmd, 2},
                      {"truncate", (cmd_func)truncate_cmd, 2},
                      {"type", (cmd_func)type_cmd, 0},
                      {"version", (cmd_func)vers_cmd, 0},
                      {"wedge", (cmd_func)wedge_cmd, 3},
                      {"", NULL, 0}};

// Re-enable warnings
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
