// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Ring map operations
void rmap_kill(void);
poly change1_vars(gmatrix rmap, variable* R, variable* S, poly f);
poly change_vars(gmatrix rmap, variable* R, variable* S, poly f);
gmatrix mat_apply(gmatrix rmap, variable* R, variable* S, gmatrix M);

// Ring map construction and manipulation
gmatrix rmap_scan(variable* R, variable* S);
void rmap_pprint(gmatrix rmap, variable* R, variable* S);
gmatrix imap(variable* R, variable* S, boolean ones);
void edit_rmap(gmatrix rmap, variable* R, variable* S);

// Command interface
void rmap_cmd(int argc, char* argv[]);
void imap_cmd(int argc, char* argv[]);
void ev_cmd(int argc, char* argv[]);
void fetch_cmd(int argc, char* argv[]);
void modrmap_cmd(int argc, char* argv[]);
void pmap_cmd(int argc, char* argv[]);
