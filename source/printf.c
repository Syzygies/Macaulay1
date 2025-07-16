// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shared.h"
#include "printf.h"
#include "monitor.h"

// Dynamic buffer to handle arbitrary length strings
static char *outbuf = NULL;
static size_t outbuf_size = 0;

const char *digits = "0123456789ABCDEF";

// Ensure output buffer has at least 'needed' bytes available
static void ensure_outbuf(size_t needed)
{
    if (needed > outbuf_size)
    {
        // Grow buffer by at least 1.5x or to needed size, whichever is larger
        size_t new_size = outbuf_size * 3 / 2;
        if (new_size < needed)
            new_size = needed;
        if (new_size < 256)
            new_size = 256; // Minimum initial size

        char *new_buf = realloc(outbuf, new_size);
        if (!new_buf)
        {
            fprintf(stderr, "Memory allocation failed in printf module\n");
            exit(1);
        }
        outbuf = new_buf;
        outbuf_size = new_size;
    }
}

void i_sniff(void)
{
    // Initialize with a reasonable default size
    ensure_outbuf(256);
}

void itoa(int n, char *s, int base)
{
    long a = (long)n;
    ltoa(a, s, base);
}

void ltoa(long n, char *s, int base)
{
    register unsigned long a;

    if ((n < 0) && (base == 10))
    {
        *s++ = '-';
        a = (unsigned long)(-n);
    }
    else
        a = (unsigned long)n;
    printn(&s, a, base);
    *s = '\0';
}

void printn(char **s, unsigned long a, int base)
{
    register unsigned long b;
    register int rem;

    b = a / (unsigned long)base;
    if (b != 0)
        printn(s, b, base);
    rem = (int)(a % (unsigned long)base);
    if (rem < 0)
        rem += base;
    *(*s)++ = digits[rem];
}

void putit(char *buf, FILE *stream)
{
    fputs(buf, stream);

    if ((stream == stdout) || (stream == stderr))
        if (am_monitoring == 1)
        {
            fputs(buf, monfile);
            fflush(monfile);
        }
}

void doPrint(const char *fmt, va_list *ap, FILE *stream)
{
    register int c, n, i;
    boolean left;
    int aint;
    long along;
    char *astr;
    size_t fmt_chunk_size = 256; // Initial chunk size for format string processing

    while (true)
    {
        // Process format string up to next % or end
        i = 0;
        // First pass: count how many chars we need
        const char *tmp_fmt = fmt;
        while (*tmp_fmt && *tmp_fmt != '%')
        {
            tmp_fmt++;
            i++;
        }

        // Ensure buffer can hold this chunk plus null terminator
        ensure_outbuf((size_t)(i + 1));

        // Now copy the characters
        i = 0;
        while ((c = *fmt++) != '%')
        {
            outbuf[i++] = (char)c;
            if (c == '\0')
            {
                putit(outbuf, stream);
                return;
            }
        }
        outbuf[i] = '\0';
        putit(outbuf, stream);

        c = *fmt++;
        if (c == '-')
        {
            c = *fmt++;
            left = true;
        }
        else
            left = false;

        for (n = 0; isdigit(c); c = *fmt++)
            n = 10 * n + (c - '0');

        switch (c)
        {
        case 'c':
            aint = va_arg(*ap, int);
            ensure_outbuf(2);
            outbuf[0] = (char)(aint & 0x7F);
            outbuf[1] = '\0';
            break;
        case 'd':
            aint = va_arg(*ap, int);
            ensure_outbuf(32); // Enough for any int
            itoa(aint, outbuf, 10);
            break;
        case 'o':
            aint = va_arg(*ap, int);
            ensure_outbuf(32); // Enough for any int
            itoa(aint, outbuf, 8);
            break;
        case 'x':
            aint = va_arg(*ap, int);
            ensure_outbuf(32); // Enough for any int
            itoa(aint, outbuf, 16);
            break;
        case 'l':
            along = va_arg(*ap, long);
            ensure_outbuf(64); // Enough for any long
            ltoa(along, outbuf, 10);
            fmt++;
            break;
        case 's':
            astr = va_arg(*ap, char *);
            {
                size_t len = strlen(astr);
                ensure_outbuf(len + 1);
                strcpy(outbuf, astr);
            }
            break;
        case 'r':
            fprintf(stderr, "internal warning: 'r' used in printing -- a no no\n");
            break;
        default:
            ensure_outbuf(2);
            outbuf[0] = (char)c;
            outbuf[1] = '\0';
            break;
        }

        if (left)
        {
            i = (int)strlen(outbuf);
            if (n > i)
            {
                ensure_outbuf((size_t)(n + 1));
                for (; i < n; i++)
                    outbuf[i] = ' ';
                outbuf[i] = '\0';
            }
        }
        else if (n > (i = (int)strlen(outbuf)))
        {
            ensure_outbuf((size_t)(n + 1));
            outbuf[n] = '\0';
            for (--i, --n; i >= 0; --n, --i)
                outbuf[n] = outbuf[i];
            for (; n >= 0; --n)
                outbuf[n] = ' ';
        }

        putit(outbuf, stream);
    }
}
