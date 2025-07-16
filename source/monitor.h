// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported functions

// Print functions with various behaviors
void print(const char* fmt, ...);
void fprint(FILE* fil, const char* fmt, ...);
void printnew(const char* fmt, ...);
void fprintnew(FILE* fil, const char* fmt, ...);
void prinput(const char* fmt, ...);
void prinput_error(const char* fmt, ...);
void prerror(const char* fmt, ...);
void prflush(const char* fmt, ...);
void intflush(const char* fmt, int num);
void debugpr(const char* fmt, ...);

// Monitor file operations
void monprint(const char* fmt, ...);
void mon_cmd(int argc, char* argv[]);
void end_monitor(void);

// Utility functions
void newline(void);
void fnewline(FILE* fil);

// Exported globals
extern FILE* monfile;
extern int am_monitoring;
extern int flushnum;
extern int cmd_error;
