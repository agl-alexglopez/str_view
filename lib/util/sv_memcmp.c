#include "sv_util.h"

/* Taken from musl
   http://git.musl-libc.org/cgit/musl/tree/src/string/memcmp.c */
int
sv_memcmp(const void *const vl, const void *const vr, size_t n)
{
    const unsigned char *l = vl;
    const unsigned char *r = vr;
    for (; n && *l == *r; n--, l++, r++)
    {}
    return n ? *l - *r : 0;
}
