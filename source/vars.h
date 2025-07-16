// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported globals
extern array var_list;
extern int last_var;
extern variable *lastv;
extern variable *cur_ring;
extern boolean allocStr;

// Exported functions
void rgDebug(variable *R);
void vrg_type(ring R);
void vmod_type(gmatrix M);
void int_type(int n);
boolean canStartVar(char c);
boolean getIdentifier(char **str, char *ident);
int getIdVal(const char *name);
poly getPolyId(const char *s, int comp);
void init_vars(void);
int whichkind(char *name, int *n, char **p, char **partname);
variable *select(variable *p, const char *partname);
variable *fvar(const char *name, const char *partname);
variable *newvar(const char *name, int isMain, int type, variable *bring);
variable *find_var(const char *name);
variable *make_var(const char *name, int isMain, int type, variable *bring);
void vrg_install(variable *p);
boolean is_a_module(variable *p);
boolean is_alias(variable *p);
void set_value(variable *p, ADDRESS val);
void set_alias(variable *p, variable *aliasp);
void connect_vars(variable *p, variable *b_next);
void putInternals(variable *p);
void listvars_cmd(int argc, char *argv[]);
void mark(variable *p);
void mark_vars(void);
void cp_varnode(int new_idx, variable *pnew, variable *pold);
variable *next_used(int n);
void contract_vars(void);
void garb_collect(void);
void rem_var(variable *p);
void kill_cmd(int argc, char *argv[]);
void spare_cmd(int argc, char *argv[]);
void type_cmd(int argc, char *argv[]);
