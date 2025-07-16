// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported functions
void init_path(void);
void path_cmd(int argc, char *argv[]);
void cdir_cmd(int argc, char *argv[]);
FILE *topen(const char *fileName, const char *mode);
void get_line(int *pargc, char ***pargv);
void get_cmd_line(int *pargc, char ***pargv, char **poutfile);
void raw_line(void);
int get_chars(FILE *fp, char *a);
void prompt(void);
int change_level(void);
void drop_voice(void);
void break_voice(void);
int open_journal(char *s);
int open_tour(const char *s);
int debug_voice(void);
void set_names(int ac, char **av);
void subs_names(void);
void branch(char *label);
void if_cmd(int argc, char *argv[]);
void jump_cmd(int argc, char *argv[]);
void sniff_prlevel(char *lb);
void unsniff_prlevel(void);

// Exported globals
extern char linebuf[];
extern char *argv[];
extern int cmd;
extern int com;
extern int remnull;
extern FILE *fd;
extern FILE *fj;
extern int jf;
extern int cur_voice;
extern int dotflag[];
