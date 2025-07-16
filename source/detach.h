// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported functions
void exitMacaulay(void);

#ifdef USEDETACH
void wait_cmd(void);
void detach_cmd(int argc, char* argv[]);
void attach_cmd(void);
void sighup_cmd(void);
#endif
