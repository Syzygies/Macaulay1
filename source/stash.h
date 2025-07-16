// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Forward declarations
struct stash;
struct slug;

// Exported functions

// Memory initialization and cleanup
void mem_init_once_only(void);
void init_mem(void);
void reset_mem(void);

// Rainy day memory management
void rainy_day_memory(long in, long out);

// Error handling
void kaboom(const char* message);

// Memory reporting
void report_mem(char* p, long n);

// Low-level memory allocation
char*gimmy(unsigned size);
void ungimmy(char* p);
void more_mem(void);

// Stash management
char*open_stash(int size, const char* name);
char*get_slug(struct stash* pstash);
void free_slug(struct stash* pstash, struct slug* ptr);
void endof_stash(struct stash* pstash);

// Block management (internal)
void get_block(struct stash* pstash);
int get_mem(struct stash* pstash);

// Command functions
void cachemem_cmd(int argc, char* argv[]);
void space_cmd(int argc);

// Memory stress test (debug only)
void memory_bomb(void);

// Exported globals
extern int showapair; // defined in stash.c, used by standard.c
extern const char* no_mem;
extern const char* too_big;
