/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "style.h"

// void i_sniff ();
// void itoa (int n, char *s, int base);
void ltoa (long n, char *s, int base);
void printn (char **s, unsigned long a, int base);
// void putit (char *outbuf, FILE *stream);
// void doPrint (char *fmt, va_list *ap, FILE *stream);

#include <stdarg.h>
#include <string.h>
#define MAXARGS 100

extern FILE *monfile;
extern int am_monitoring;

void i_sniff ()
{
}

/*------------------------------------------------------------*/

#define NOUTBUF 200
static char outbuf[NOUTBUF];

char *digits = "0123456789ABCDEF";

void itoa (int n, char *s, int base)
{
    long a = (long) n;

    ltoa(a, s, base);
}

void ltoa (long n, char *s, int base)
{
    unsigned long a;

    if ((n < 0) AND (base IS 10)) {
        *s++ = '-';
        a = (unsigned long) (-n);
    } else
        a = (unsigned long) n;
    printn(&s, a, base);
    *s = '\0';
}

void printn (char **s, unsigned long a, int base)
{
    unsigned long b;
    int rem;

    b = a / base;
    if (b ISNT 0)
        printn(s, b, base);
    rem = (int) (a % base);
    if (rem < 0)
        rem += base;
    *(*s)++ = digits[rem];
}

void putit (char *outbuf, FILE *stream)
{
    fputs(outbuf, stream);

    if ((stream == stdout) || (stream == stderr))
        if (am_monitoring == 1) {
            fputs(outbuf, monfile);
            fflush(monfile);
        }
}

void doPrint (char *fmt, va_list *ap, FILE *stream)
{
    int c, n, i;
    boolean left;
    int aint;
    long along;
    char *astr;

    while (TRUE) {
        i = 0;
        while ((c = *fmt++) ISNT '%') {
            outbuf[i++] = c;
            if (c IS '\0') {
                putit(outbuf, stream);
                return;
            }
        }
        outbuf[i] = '\0';
        putit(outbuf, stream);

        c = *fmt++;
        if (c IS '-') {
            c = *fmt++;
            left = TRUE;
        } else
            left = FALSE;

        for (n=0; isdigit(c); c = *fmt++)
            n = 10 * n + (c - '0');

        switch (c) {
        case 'c':
            aint = va_arg(*ap, int);
            outbuf[0] = (char) (aint & 0x7F);
            outbuf[1] = '\0';
            break;
        case 'd':
            aint = va_arg(*ap, int);
            itoa(aint, outbuf, 10);
            break;
        case 'o':
            aint = va_arg(*ap, int);
            itoa(aint, outbuf, 8);
            break;
        case 'x':
            aint = va_arg(*ap, int);
            itoa(aint, outbuf, 16);
            break;
        case 'l':
            along = va_arg(*ap, long);
            ltoa(along,  outbuf, 10);
            fmt++;
            break;
        case 's':
            astr = va_arg(*ap, char *);
            strcpy(outbuf, astr);
            break;
        case 'r':
            fprintf(stderr,
                    "internal warning: 'r' used in printing -- a no no\n");
        /*
                ip = *((char **) ip);
                fmt = *((char **) ip);
                outbuf[0] = '\0';
                SPincr(ip, SPptr);
                break;
        */
        default:
            outbuf[0] = c;
            outbuf[1] = '\0';
            break;
        } /* end case stmt */

        if (left) {
            for (i=strlen(outbuf); i<n; i++)
                outbuf[i] = ' ';
            outbuf[i] = '\0';
        } else if (n > (i=strlen(outbuf))) {
            outbuf[n] = '\0';
            for (--i, --n; i>=0; --n, --i)
                outbuf[n] = outbuf[i];
            for (; n>=0; --n)
                outbuf[n] = ' ';
        }

        putit(outbuf, stream);

    }
}
