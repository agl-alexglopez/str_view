#include "sv_util.h"

#include <stddef.h>

/* Taken from musl
   http://git.musl-libc.org/cgit/musl/tree/src/string/memcpy.c */
void *
sv_memcpy(void *restrict dest, const void *const restrict src, size_t n)
{
    unsigned char *d = dest;
    const unsigned char *s = src;
    for (; n; n--)
    {
        *d++ = *s++;
    }
    return dest;
}
