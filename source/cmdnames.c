/* Copyright 1993 Dave Bayer and Mike Stillman. All rights reserved. */
#include "style.h"
#include "mtypes.h"

extern dot_cmd();
extern ac_cmd();
extern add_cmd();
extern ar_cmd();
extern args_cmd();
extern betti_cmd();
extern binoms_cmd();
extern cachemem_cmd();
extern calc_cmd();
extern cat_cmd();
extern cdir_cmd();
extern ce_cmd();
extern charac_cmd();
extern chcalc_cmd();
extern codim_cmd();
extern coef_cmd();
extern coldeg_cmd();
extern coldegs_cmd();
extern commands_cmd();
extern compress_cmd();
extern concat_cmd();
extern continue_cmd();
extern copy_cmd();
extern degree_cmd();
extern determinants_cmd();
extern diag_cmd();
extern diff_cmd();
extern dshift_cmd();
extern dsum_cmd();
extern ech_cmd();
extern edit_cmd();
extern modrmap_cmd();
extern elim_cmd();
extern end_monitor();
extern ev_cmd();
extern exit_cmd();
extern fetch_cmd();
extern flatten_cmd();
extern forcestd_cmd();
extern help_cmd();
extern helpfile_cmd();
extern hilb_cmd();
extern hilb_numer_cmd();
extern homog_cmd();
extern hulb_cmd();
extern ideal_cmd();
extern iden_cmd();
extern if_cmd();
extern imap_cmd();
extern in_cmd();
extern incset_cmd();
extern inpart_cmd();
extern int_cmd();
extern inter_cmd();
extern iszero_cmd();
extern jacob_cmd();
extern jump_cmd();
extern basis_cmd();
extern keep_cmd();
extern kill_cmd();
extern koszul_cmd();
extern lift_cmd();
extern liftstd_cmd();
extern listvars_cmd();
extern mat_cmd();
extern max_cmd();
extern mc_cmd();
extern min_cmd();
extern modulo_cmd();
extern mon_cmd();
extern monoms_cmd();
extern monprimes_cmd();
extern mr_cmd();
extern mult_cmd();
extern ncols_cmd();
extern nres_cmd();
extern nrows_cmd();
extern numinfo_cmd();
extern nvars_cmd();
extern outer_cmd();
extern path_cmd();
extern pc_cmd();
extern pfaff_cmd();
extern pmap_cmd();
extern poly_cmd();
extern power_cmd();
extern pr_cmd();
extern pres_cmd();
extern presentring_cmd();
extern pring_cmd();
extern prmat_cmd();
extern putchange_cmd();
extern putmat_cmd();
extern putstd_cmd();
extern qring_cmd();
extern quot_cmd();
extern random_cmd();
extern reduce_cmd();
extern res_cmd();
extern reset_cmd();
extern ring_cmd();
extern ringcol_cmd();
extern ringrow_cmd();
extern ringsum_cmd();
extern rmap_cmd();
extern rowdeg_cmd();
extern rowdegs_cmd();
extern sat_cmd();
extern set_cmd();
extern setvalue_cmd();
extern setcoldegs_cmd();
extern setdegs_cmd();
extern setring_cmd();
extern shout_cmd();
extern size_cmd();
extern smult_cmd();
extern space_cmd();
extern spairs_cmd();
extern spare_cmd();
extern sparse_cmd();
extern std_cmd();
extern stdmin_cmd();
extern stdpart_cmd();
extern submat_cmd();
extern sub_cmd();
extern syz_cmd();
extern tensor_cmd();
extern trace_cmd();
extern trans_cmd();
extern truncate_cmd();
extern type_cmd();
extern vers_cmd();
extern wedge_cmd();

/*---------------------------------------------------------------
Defined in mtypes.h:

typedef struct cmd_rec {
    char *name;
    pfv proc;
    int args;
} cmd_rec;

To add a command, provide an extern defn above, and a cmd_rec
entry below. Multiple names for the same command are allowed.
"args" is the preferred number of arguments when the command sets
a variable, rather than reports to the user. This value is used
in "dot_fns.c" to create temporary arguments automatically for
return values in dot mode; to disable this feature, set "args" to 0.
"args" is only helpful for commands which (optionally or not) return
results via their last argument(s). Most but not all commands behave
this way; please write any new commands following this protocol.
The current offenders which could but don't use their last arg(s) are
"imap", "int", "poly", "rmap", and "sat"; it is too late to change them.

For example, "characteristic" can be called with either 1 or 2 args.
If a 2nd arg is provided, that arg is set to the characteristic.
Thus the correct value for "args" is 2.

---------------------------------------------------------------*/

cmd_rec cmd_list[] = {
    { ".", dot_cmd, 0 },
    { "ac", ac_cmd, 0 },
    { "add", add_cmd, 3 },
    { "ar", ar_cmd, 0 },
    { "args", args_cmd, 0 },
    { "betti", betti_cmd, 0 },
    { "binoms", binoms_cmd, 3 },
    { "cache_mem", cachemem_cmd, 0 },
    { "calc", calc_cmd, 0 },
    { "cat", cat_cmd, 2 },
    { "cdir", cdir_cmd, 0 },
    { "ce", ce_cmd, 0 },
    { "characteristic", charac_cmd, 2 },
    { "chcalc", chcalc_cmd, 0 },
    { "codim", codim_cmd, 2 },
    { "coef", coef_cmd, 3 },
    { "col_degree", coldeg_cmd, 3 },
    { "col_degs", coldegs_cmd, 0 },
    { "commands", commands_cmd, 0 },
    { "compress", compress_cmd, 2 },
    { "concat", concat_cmd, 0 },
    { "continue", continue_cmd, 0 },
    { "contract", diff_cmd, 3 },
    { "copy", copy_cmd, 2 },
    { "degree", degree_cmd, 3 },
    { "determinants", determinants_cmd, 3 },
    { "diag", diag_cmd, 2 },
    { "diff", diff_cmd, 3 },
    { "dshift", dshift_cmd, 0 },
    { "dsum", dsum_cmd, 3 },
    { "echo", ech_cmd, 0 },
    { "edit", edit_cmd, 0 },
    { "edit_map", modrmap_cmd, 0 },
    { "elim", elim_cmd, 2 },
    { "endmon", end_monitor, 0 },
    { "ev", ev_cmd, 3 },
    { "exit", exit_cmd, 0 },
    { "fetch", fetch_cmd, 2 },
    { "flatten", flatten_cmd, 2 },
    { "forcestd", forcestd_cmd, 2 },
    { "help", help_cmd, 0 },
    { "help_file", helpfile_cmd, 0 },
    { "hilb", hilb_cmd, 0 },
    { "hilb_numer", hilb_numer_cmd, 3 },
    { "homog", homog_cmd, 3 },
    { "hulb", hulb_cmd, 0 },
    { "ideal", ideal_cmd, 1 },
    { "iden", iden_cmd, 2 },
    { "if", if_cmd, 0 },
    { "imap", imap_cmd, 0 },
    { "in", in_cmd, 2 },
    { "incr_set", incset_cmd, 0 },
    { "inpart", inpart_cmd, 2 },
    { "int", int_cmd, 0 },
    { "intersect", inter_cmd, 3 },
    { "is_zero", iszero_cmd, 2 },
    { "jacob", jacob_cmd, 2 },
    { "jump", jump_cmd, 0 },
    { "k_basis", basis_cmd, 2 },
    { "keep", keep_cmd, 2 },
    { "kill", kill_cmd, 0 },
    { "koszul", koszul_cmd, 3 },
    { "lift", lift_cmd, 3 },
    { "lift_std", liftstd_cmd, 2 },
    { "listvars", listvars_cmd, 0 },
    { "mat", mat_cmd, 1 },
    { "max", max_cmd, 2 },
    { "mc", mc_cmd, 0 },
    { "min", min_cmd, 2 },
    { "modulo", modulo_cmd, 3 },
    { "monitor", mon_cmd, 0 },
    { "monoms", monoms_cmd, 3 },
    { "monprimes", monprimes_cmd, 3 },
    { "mr", mr_cmd, 0 },
    { "mult", mult_cmd, 3 },
    { "ncols", ncols_cmd, 2 },
    { "nres", nres_cmd, 2 },
    { "nrows", nrows_cmd, 2 },
    { "numinfo", numinfo_cmd, 0 },
    { "nvars", nvars_cmd, 2 },
    { "outer", outer_cmd, 3 },
    { "path", path_cmd, 0 },
    { "pc", pc_cmd, 0 },
    { "pfaff", pfaff_cmd, 2 },
    { "pmap", pmap_cmd, 0 },
    { "poly", poly_cmd, 0 },
    { "power", power_cmd, 3 },
    { "pr", pr_cmd, 0 },
    { "pres", pres_cmd, 0 },
    { "present_ring", presentring_cmd, 2 },
    { "pring", pring_cmd, 0 },
    { "prmat", prmat_cmd, 0 },
    { "putchange", putchange_cmd, 2 },
    { "putmat", putmat_cmd, 0 },
    { "putstd", putstd_cmd, 2 },
    { "qring", qring_cmd, 2 },
    { "quit", exit_cmd, 0 },
    { "quotient", quot_cmd, 3 },
    { "random", random_cmd, 3 },
    { "reduce", reduce_cmd, 3 },
    { "res", res_cmd, 2 },
    { "reset", reset_cmd, 0 },
    { "ring", ring_cmd, 1 },
    { "ring_from_cols", ringcol_cmd, 2 },
    { "ring_from_rows", ringrow_cmd, 2 },
    { "ring_sum", ringsum_cmd, 3 },
    { "rmap", rmap_cmd, 0 },
    { "row_degree", rowdeg_cmd, 3 },
    { "row_degs", rowdegs_cmd, 0 },
    { "sat", sat_cmd, 0 },
    { "set", set_cmd, 0 },
    { "set_value", setvalue_cmd, 2 },
    { "setcoldegs", setcoldegs_cmd, 0 },
    { "setdegs", setdegs_cmd, 0 },
    { "setring", setring_cmd, 0 },
    { "shout", shout_cmd, 0 },
    { "size", size_cmd, 0 },
    { "smult", smult_cmd, 3 },
    { "space", space_cmd, 0 },
    { "spairs", spairs_cmd, 0 },
    { "spare", spare_cmd, 0 },
    { "sparse", sparse_cmd, 1 },
    { "std", std_cmd, 2 },
    { "std_minimal", stdmin_cmd, 2 },
    { "stdpart", stdpart_cmd, 2 },
    { "submat", submat_cmd, 2 },
    { "subtract", sub_cmd, 3 },
    { "syz", syz_cmd, 3 },
    { "tensor", tensor_cmd, 3 },
    { "trace", trace_cmd, 2 },
    { "transpose", trans_cmd, 2 },
    { "truncate", truncate_cmd, 2 },
    { "type", type_cmd, 0 },
    { "version", vers_cmd, 0 },
    { "wedge", wedge_cmd, 3 },
    { "", NULL, 0 }
};
