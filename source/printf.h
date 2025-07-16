// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// Exported functions
void i_sniff(void);
void itoa(int n, char *s, int base);
void ltoa(long n, char *s, int base);
void printn(char **s, unsigned long a, int base);
void putit(char *buf, FILE *stream);
void doPrint(const char *fmt, va_list *ap, FILE *stream);

// Exported globals
extern const char *digits;
