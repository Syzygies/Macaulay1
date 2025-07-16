// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported globals

// Global configuration variables - widely used throughout Macaulay
extern int bailout;     // >0 means exit when out of memory (increments on each failure)
extern int linesize;    // size of line for matrix display
extern int verbose;     // >0 means give verbose output
extern int pr_charp;    // >0 means do NOT (try to) lift to rationals
extern int prlevel;     // >0 means suppress ALL output, except error messages
extern int timer;       // >0 means display execution time
extern int autocalc;    // =0 no autocalc, <0 calc until done, >0 calc to autodegree
extern int autodegree;  // if autocalc>0, this is highest degree to compute
extern int maxdegree;   // maximum degree for monomials in rings created using 'ring'
extern int echo;        // >0 tells Macaulay to echo all input
extern int autoReduce;  // >0 means do NOT do autoreduction
extern int nlines;      // number of blank lines between commands
extern int prcomment;   // if >0 start each output line with ;
extern int showmem;     // >0 means report memory usage at each increase
extern int showpairs;   // >0 means report S-pairs left during computations
extern int iodelay;     // delay (in system ticks) for caching verbose output

// Exported functions

// Initialization function - called by shell.c
void i_set(void);

// Command handlers
void set_cmd(int argc, char* argv[]);
void incset_cmd(int argc, char* argv[]);
void setvalue_cmd(int argc, char* argv[]);

// Internal functions - not typically called from other modules
void i_setvar(int n, const char* name, int* loc, int defaultVal, const char* whatami);
void doSet(int argc, char* argv[], boolean doIncr);
