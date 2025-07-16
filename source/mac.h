// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Global path variables
extern char* MclyPath;
extern char* MclyCdir;
extern char* helpFile;
extern char* newFile;

// Timer variables
extern long startTime;

// Platform initialization
void spec_init(void);

// Interrupt handling
void intr_shell(void);
boolean have_intr(void);
void rmmouse(void);

// Timer functions
void markTime(void);
long nSeconds(void);
long get_ticks(void);
void prTime(const char* str);

// Other platform functions
void setTextSize(int n);
void prsetup(void);
