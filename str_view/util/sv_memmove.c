#include "sv_util.h"

#include <stddef.h>
#include <stdint.h>

/* Taken from musl
   http://git.musl-libc.org/cgit/musl/tree/src/string/memmove.c */
void *
sv_memmove(void *dest, const void *const src, size_t n)
{
    char *d = dest;
    const char *s = src;
    if (d == s)
    {
        return d;
    }
    if ((uintptr_t)s - (uintptr_t)d - n <= -2 * n)
    {
        return sv_memcpy(d, s, n);
    }
    if (d < s)
    {
        for (; n; n--)
        {
            *d++ = *s++;
        }
    }
    else
    {
        while (n)
        {
            n--;
            d[n] = s[n];
        }
    }
    return dest;
}
