#include "sv_util.h"

#include <stddef.h>

/* Taken from musl
   http://git.musl-libc.org/cgit/musl/tree/src/string/strnlen.c */
size_t
sv_strnlen(const char *const str, size_t n)
{
    if (!str)
    {
        return 0;
    }
    const char *i = str;
    for (; n && *i; n--, ++i)
    {}
    return i - str;
}
