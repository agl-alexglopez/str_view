#include "sv_util.h"

/* Taken from musl
   http://git.musl-libc.org/cgit/musl/tree/src/string/strlen.c */
size_t
sv_len(const char *const str)
{
    if (!str)
    {
        return 0;
    }
    const char *i = str;
    for (; *i; ++i)
    {}
    return i - str;
}
