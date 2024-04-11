/*
    varargs.h -- Variable arguments

    Lifted from HP9000 UNIX C-Header File varargs.h

    Maybe Copyright for parts of this source by Hewlett-Packard
    All rights reserved.

*/


#ifndef __VARARGS__
#define __VARARGS__


typedef char *va_list;

#define va_dcl int va_alist;
#define va_start(__list) __list = (char *) &va_alist
#define va_arg(__list,__mode) ((__mode *)(__list += sizeof(__mode)))[-1]
#define va_end(__list)


#endif __VARARGS__
