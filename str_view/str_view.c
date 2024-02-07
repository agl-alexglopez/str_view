/* Author: Alexander G. Lopez
   File: string_view.c
   ===================
   This file implements the string_view interface as an approximation of C++
   string_view type. There are some minor differences and C flavor thrown
   in with the additional twist of a reimplementation of the Two-Way
   String-Searching algorithm, similar to glibc. */
#include "str_view.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* ========================   Type Definitions   =========================== */

struct sv_factorization
{
    /* Position in the needle at which (local period = period). */
    ssize_t start_critical_pos;
    /* A distance in the needle such that two letters always coincide. */
    ssize_t period_dist;
};

struct sv_two_way_pack
{
    const char *const haystack;
    ssize_t haystack_sz;
    const char *const needle;
    ssize_t needle_sz;
    /* Taken from the factorization of needle for two-way searching */
    ssize_t period_dist;
    /* The critical position taken from our factorization. */
    ssize_t critical_pos;
};

/* Avoid giving the user a chance to dereference null as much as posssible
   by returning this for various edgecases when it makes sense to communicate
   empty, null, invalid, not found etc. Used on cases by case basis.
   It is usually better to justify giving back the user pointer in a
   string_view even if it sized 0 and pointing to null terminator. */
static const char *const nil = "";

/* =========================   Prototypes   =============================== */

static size_t sv_after_find(str_view, str_view);
static size_t sv_min(size_t, size_t);
static size_t sv_two_way(const char *, ssize_t, const char *, ssize_t);
static struct sv_factorization sv_maximal_suffix(const char *, ssize_t);
static struct sv_factorization sv_maximal_suffix_rev(const char *, ssize_t);
static size_t sv_two_way_memoization(struct sv_two_way_pack);
static size_t sv_two_way_normal(struct sv_two_way_pack);
static sv_threeway_cmp sv_char_cmp(char, char);
static ssize_t sv_ssizet_max(ssize_t, ssize_t);
static size_t sv_twobyte_strnstrn(const unsigned char *, size_t,
                                  const unsigned char *);
static size_t sv_threebyte_strnstrn(const unsigned char *, size_t,
                                    const unsigned char *);
static size_t sv_fourbyte_strnstrn(const unsigned char *, size_t,
                                   const unsigned char *);
static size_t sv_len(const char *);
static size_t sv_nlen(const char *, size_t);
size_t sv_strcspn(const char *, size_t, const char *, size_t);
size_t sv_strspn(const char *, size_t, const char *, size_t);
size_t sv_strnstrn(const char *, ssize_t, const char *, ssize_t);
void *sv_memmove(void *, const void *, size_t);

/* ===================   Interface Implementation   ====================== */

str_view
sv(const char *const str)
{
    if (!str)
    {
        return (str_view){.s = nil, .sz = 0};
    }
    return (str_view){.s = str, .sz = sv_strlen(str)};
}

str_view
sv_n(const char *const str, size_t n)
{
    if (!str || n == 0)
    {
        return (str_view){.s = nil, .sz = 0};
    }
    return (str_view){.s = str, .sz = sv_nlen(str, n)};
}

str_view
sv_delim(const char *const str, const char *const delim)
{
    if (!str)
    {
        return (str_view){.s = nil, .sz = 0};
    }
    if (!delim)
    {
        return (str_view){.s = str, .sz = sv_strlen(str)};
    }
    return sv_begin_tok(
        (str_view){
            .s = str,
            .sz = sv_strlen(str),
        },
        (str_view){
            .s = delim,
            .sz = sv_strlen(delim),
        });
}

void
sv_print(FILE *f, str_view s)
{
    if (!s.s || nil == s.s || 0 == s.sz || !f)
    {
        return;
    }
    /* printf does not output the null terminator in normal strings so
       as long as we output correct number of characters we do the same */
    (void)fwrite(s.s, sizeof(char), s.sz, f);
}

str_view
sv_copy(const char *const src_str, const size_t str_sz)
{
    return sv_n(src_str, str_sz);
}

size_t
sv_fill(char *dest_buf, size_t dest_sz, const str_view src)
{
    if (!dest_buf || 0 == dest_sz || !src.s || 0 == src.sz)
    {
        return 0;
    }
    const size_t paste = sv_min(dest_sz, sv_svbytes(src));
    sv_memmove(dest_buf, src.s, paste);
    dest_buf[paste - 1] = '\0';
    return paste;
}

bool
sv_empty(const str_view s)
{
    return s.sz == 0;
}

size_t
sv_svlen(str_view sv)
{
    return sv.sz;
}

size_t
sv_svbytes(str_view sv)
{
    return sv.sz + 1;
}

size_t
sv_strlen(const char *const str)
{
    return sv_len(str);
}

size_t
sv_strbytes(const char *const str)
{
    if (!str)
    {
        return 0;
    }
    return sv_len(str) + 1;
}

size_t
sv_minlen(const char *const str, size_t n)
{
    return sv_nlen(str, n);
}

char
sv_at(str_view sv, size_t i)
{
    if (i >= sv.sz)
    {
        (void)fprintf(stderr,
                      "string_view index out of range. size=%zu, index=%zu\n",
                      sv.sz, i);
        exit(1);
    }
    return sv.s[i];
}

const char *
sv_null(void)
{
    return nil;
}

void
sv_swap(str_view *a, str_view *b)
{
    if (a == b || !a || !b)
    {
        return;
    }
    const str_view tmp_b = (str_view){.s = b->s, .sz = b->sz};
    b->s = a->s;
    b->sz = a->sz;
    a->s = tmp_b.s;
    a->sz = tmp_b.sz;
}

int
sv_svcmp(str_view sv1, str_view sv2)
{
    if (!sv1.s || !sv2.s)
    {
        (void)fprintf(stderr, "sv_svcmp cannot compare NULL.\n");
        exit(1);
    }
    size_t i = 0;
    const size_t sz = sv_min(sv1.sz, sv2.sz);
    while (sv1.s[i] == sv2.s[i] && i < sz)
    {
        ++i;
    }
    if (i == sv1.sz && i == sv2.sz)
    {
        return EQL;
    }
    if (i < sv1.sz && i < sv2.sz)
    {
        return ((uint8_t)sv1.s[i] < (uint8_t)sv2.s[i] ? LES : GRT);
    }
    return (i < sv1.sz) ? GRT : LES;
}

int
sv_strcmp(str_view sv, const char *str)
{
    if (!sv.s || !str)
    {
        (void)fprintf(stderr, "sv_strcmp cannot compare NULL.\n");
        exit(1);
    }
    size_t i = 0;
    const size_t sz = sv.sz;
    while (str[i] != '\0' && sv.s[i] == str[i] && i < sz)
    {
        ++i;
    }
    if (i == sv.sz && str[i] == '\0')
    {
        return EQL;
    }
    if (i < sv.sz && str[i] != '\0')
    {
        return ((uint8_t)sv.s[i] < (uint8_t)str[i] ? LES : GRT);
    }
    return (i < sv.sz) ? GRT : LES;
}

int
sv_strncmp(str_view sv, const char *str, const size_t n)
{
    if (!sv.s || !str)
    {
        (void)fprintf(stderr, "sv_strncmp cannot compare NULL.\n");
        exit(1);
    }
    size_t i = 0;
    const size_t sz = sv_min(sv.sz, n);
    while (str[i] != '\0' && sv.s[i] == str[i] && i < sz)
    {
        ++i;
    }
    if (i == sv.sz && sz == n)
    {
        return EQL;
    }
    /* strncmp compares the first at most n bytes inclusive */
    if (i < sv.sz && sz <= n)
    {
        return ((uint8_t)sv.s[i] < (uint8_t)str[i] ? LES : GRT);
    }
    return (i < sv.sz) ? GRT : LES;
}

char
sv_front(struct str_view sv)
{
    if (!sv.s || 0 == sv.sz)
    {
        return *nil;
    }
    return *sv.s;
}

char
sv_back(struct str_view sv)
{
    if (!sv.s || 0 == sv.sz)
    {
        return *nil;
    }
    return sv.s[sv.sz - 1];
}

const char *
sv_begin(const str_view sv)
{
    if (!sv.s)
    {
        return nil;
    }
    return sv.s;
}

const char *
sv_end(const str_view sv)
{
    if (!sv.s)
    {
        return nil;
    }
    return sv.s + sv.sz;
}

const char *
sv_next(const char *c)
{
    if (!c)
    {
        return nil;
    }
    return ++c;
}

const char *
sv_pos(str_view sv, size_t i)
{
    if (!sv.s)
    {
        return nil;
    }
    if (i > sv.sz)
    {
        return sv_end(sv);
    }
    return sv.s + i;
}

str_view
sv_begin_tok(str_view sv, str_view delim)
{
    if (!sv.s)
    {
        return (str_view){.s = nil, .sz = 0};
    }
    if (!delim.s)
    {
        return (str_view){.s = sv.s + sv.sz, 0};
    }
    const size_t sv_not = sv_after_find(sv, delim);
    sv.s += sv_not;
    if (*sv.s == '\0')
    {
        return (str_view){.s = sv.s, .sz = 0};
    }
    return sv_substr(sv, 0, sv_find(sv, 0, delim));
}

bool
sv_end_tok(const str_view sv)
{
    return 0 == sv.sz;
}

str_view
sv_next_tok(str_view sv, str_view delim)
{
    if (!sv.s)
    {
        return (str_view){.s = nil, .sz = 0};
    }
    if (!delim.s || sv.s[sv.sz] == '\0')
    {
        return (str_view){.s = sv.s + sv.sz, .sz = 0};
    }
    const char *next = sv.s + sv.sz + delim.sz;
    const size_t next_sz = sv_strlen(next);
    next += sv_after_find((str_view){.s = next, .sz = next_sz}, delim);
    if (*next == '\0')
    {
        return (str_view){.s = next, .sz = 0};
    }
    const size_t found
        = sv_strnstrn(next, (ssize_t)next_sz, delim.s, (ssize_t)delim.sz);
    return (str_view){.s = next, .sz = found};
}

bool
sv_starts_with(str_view sv, str_view prefix)
{
    if (prefix.sz > sv.sz)
    {
        return false;
    }
    return sv_svcmp(sv_substr(sv, 0, prefix.sz), prefix) == 0;
}

str_view
sv_remove_prefix(const str_view sv, const size_t n)
{
    const size_t remove = sv_min(sv.sz, n);
    return (str_view){.s = sv.s + remove, .sz = sv.sz - remove};
}

bool
sv_ends_with(str_view sv, str_view suffix)
{
    if (suffix.sz > sv.sz)
    {
        return false;
    }
    return sv_svcmp(sv_substr(sv, sv.sz - suffix.sz, suffix.sz), suffix) == 0;
}

str_view
sv_remove_suffix(const str_view sv, const size_t n)
{
    return (str_view){.s = sv.s, .sz = sv.sz - sv_min(sv.sz, n)};
}

str_view
sv_substr(str_view sv, size_t pos, size_t count)
{
    if (pos > sv.sz)
    {
        printf("string_view index out of range. pos=%zu size=%zu", pos, sv.sz);
        exit(1);
    }
    return (str_view){.s = sv.s + pos, .sz = sv_min(count, sv.sz - pos)};
}

bool
sv_contains(str_view haystack, str_view needle)
{
    if (needle.sz > haystack.sz)
    {
        return false;
    }
    if (sv_empty(haystack))
    {
        return false;
    }
    if (sv_empty(needle))
    {
        return true;
    }
    const size_t found = sv_strnstrn(haystack.s, (ssize_t)haystack.sz, needle.s,
                                     (ssize_t)needle.sz);
    return (found == haystack.sz) ? false : true;
}

str_view
sv_svsv(str_view haystack, str_view needle)
{
    if (needle.sz > haystack.sz)
    {
        return (str_view){.s = nil, .sz = 0};
    }
    if (sv_empty(haystack))
    {
        return haystack;
    }
    if (sv_empty(needle))
    {
        return (str_view){.s = nil, .sz = 0};
    }
    const size_t found = sv_strnstrn(haystack.s, (ssize_t)haystack.sz, needle.s,
                                     (ssize_t)needle.sz);
    if (found == haystack.sz)
    {
        return (str_view){.s = haystack.s + haystack.sz, .sz = 0};
    }
    return (str_view){.s = haystack.s + found, .sz = needle.sz};
}

size_t
sv_find(str_view haystack, size_t pos, str_view needle)
{
    if (needle.sz > haystack.sz || pos > haystack.sz)
    {
        return haystack.sz;
    }
    return sv_strnstrn(haystack.s + pos, (ssize_t)(haystack.sz - pos), needle.s,
                       (ssize_t)needle.sz);
}

size_t
sv_rfind(str_view haystack, size_t pos, str_view needle)
{
    if (needle.sz >= haystack.sz)
    {
        return haystack.sz;
    }
    if (pos > haystack.sz)
    {
        pos = haystack.sz;
    }
    haystack.sz -= (haystack.sz - pos);
    size_t last_found = haystack.sz;
    size_t i = 0;
    for (;;)
    {
        i += sv_strnstrn(haystack.s + i, (ssize_t)(haystack.sz - i), needle.s,
                         (ssize_t)needle.sz);
        if (i == haystack.sz)
        {
            break;
        }
        last_found = i;
        ++i;
    }
    return last_found;
}

size_t
sv_find_first_of(str_view haystack, str_view set)
{
    if (!haystack.s)
    {
        return 0;
    }
    if (!set.s)
    {
        return haystack.sz;
    }
    return sv_strcspn(haystack.s, haystack.sz, set.s, set.sz);
}

size_t
sv_find_last_of(str_view haystack, str_view set)
{
    if (!haystack.s)
    {
        return 0;
    }
    if (!set.s)
    {
        return haystack.sz;
    }
    size_t last_pos = haystack.sz;
    size_t prev = 0;
    size_t in = 0;
    while ((in += sv_strspn(haystack.s + in, haystack.sz - in, set.s, set.sz))
           != haystack.sz)
    {
        if (in != prev)
        {
            last_pos = in - 1;
        }
        ++in;
        prev = in;
    }
    return last_pos;
}

size_t
sv_find_first_not_of(str_view haystack, str_view set)
{
    if (!haystack.s)
    {
        return 0;
    }
    if (!set.s)
    {
        return 0;
    }
    return sv_strspn(haystack.s, haystack.sz, set.s, set.sz);
}

size_t
sv_find_last_not_of(str_view haystack, str_view set)
{
    if (!haystack.s || 0 == haystack.sz)
    {
        return 0;
    }
    if (!set.s)
    {
        return haystack.sz - 1;
    }
    size_t last_pos = haystack.sz;
    size_t prev = 0;
    size_t in = 0;
    while ((in += sv_strspn(haystack.s + in, haystack.sz - in, set.s, set.sz))
           != haystack.sz)
    {
        if (in != prev)
        {
            last_pos = in;
        }
        ++in;
        prev = in;
    }
    return last_pos;
}

size_t
sv_npos(str_view sv)
{
    return sv.sz;
}

/* ======================   Static Helpers    ============================= */

static size_t
sv_after_find(str_view haystack, str_view needle)
{
    if (needle.sz > haystack.sz)
    {
        return 0;
    }
    size_t i = 0;
    size_t delim_i = 0;
    while (i < haystack.sz && needle.s[delim_i] == haystack.s[i])
    {
        ++i;
        delim_i = (delim_i + 1) % needle.sz;
    }
    /* We need to also reset to the last mismatch we found. Imagine
       we started to find the delimeter but then the string changed
       into a mismatch. We go back to get characters that are partially
       in the delimeter. */
    return i - delim_i;
}

static inline size_t
sv_min(size_t a, size_t b)
{
    return a < b ? a : b;
}

static inline ssize_t
sv_ssizet_max(ssize_t a, ssize_t b)
{
    return a > b ? a : b;
}

static inline sv_threeway_cmp
sv_char_cmp(char a, char b)
{
    return (a > b) - (a < b);
}

/* ======================   Static Utilities    =========================== */

/* This is section is modeled after the string.h library. However, using
   str_view that may not be null terminated requires modifications. Also
   it is important to not use string.h functionality which would force
   the user to include string.h in addition to custom implementations in
   this library. */

#define BITOP(a, b, op)                                                        \
    ((a)[(size_t)(b) / (8 * sizeof *(a))] op(size_t) 1                         \
     << ((size_t)(b) % (8 * sizeof *(a))))

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

/* Taken from musl implementation
   https://git.musl-libc.org/cgit/musl/tree/src/string/memset.c */
void *
sv_memset(void *dest, int c, size_t n)
{
    if (!dest)
    {
        return NULL;
    }
    unsigned char *s = dest;
    size_t k;
    /* Fill head and tail with minimal branching. Each
       conditional ensures that all the subsequently used
       offsets are well-defined and in the dest region. */
    if (!n)
    {
        return dest;
    }
    s[0] = c;
    s[n - 1] = c;
    if (n <= 2)
    {
        return dest;
    }
    s[1] = c;
    s[2] = c;
    s[n - 2] = c;
    s[n - 3] = c;
    if (n <= 6)
    {
        return dest;
    }
    s[3] = c;
    s[n - 4] = c;
    if (n <= 8)
    {
        return dest;
    }
    /* Advance pointer to align it at a 4-byte boundary,
       and truncate n to a multiple of 4. The previous code
       already took care of any head/tail that get cut off
       by the alignment. */
    k = -(uintptr_t)s & 3;
    s += k;
    n -= k;
    n &= -4;
    for (; n; n--, s++)
    {
        *s = c;
    }
    return dest;
}

/* Taken from musl
   http://git.musl-libc.org/cgit/musl/tree/src/string/strnlen.c */
size_t
sv_nlen(const char *const str, size_t n)
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

/* strspn is based on musl C-standard library implementation
   http://git.musl-libc.org/cgit/musl/tree/src/string/strcspn.c
   A custom implemenatation is necessary because C standard library impls
   have no concept of a string view and will continue searching beyond the
   end of a view until null is found. This way, string searches are efficient
   and only within the range specified. */
size_t
sv_strcspn(const char *str, size_t str_sz, const char *set, size_t set_sz)
{
    const char *a = str;
    size_t byteset[32 / sizeof(size_t)];
    if (!set[0] || !set[1])
    {
        return str_sz;
    }
    sv_memset(byteset, 0, sizeof byteset);
    for (size_t i = 0;
         *set && BITOP(byteset, *(unsigned char *)set, |=) && i < set_sz;
         set++, ++i)
        ;
    for (size_t i = 0;
         *str && !BITOP(byteset, *(unsigned char *)str, &) && i < str_sz; str++)
        ;
    return str - a;
}

/* strspn is based on musl C-standard library implementation
   https://git.musl-libc.org/cgit/musl/tree/src/string/strspn.c
   A custom implemenatation is necessary because C standard library impls
   have no concept of a string view and will continue searching beyond the
   end of a view until null is found. This way, string searches are efficient
   and only within the range specified. */
size_t
sv_strspn(const char *str, size_t str_sz, const char *set, size_t set_sz)
{
    const char *a = str;
    size_t byteset[32 / sizeof(size_t)] = {0};
    if (!set[0])
    {
        return str_sz;
    }
    if (!set[1])
    {
        for (size_t i = 0; *str == *set && i < str_sz && i < set_sz; str++, ++i)
            ;
        return str - a;
    }
    for (size_t i = 0;
         *set && BITOP(byteset, *(unsigned char *)set, |=) && i < set_sz;
         set++, ++i)
        ;
    for (size_t i = 0;
         *str && BITOP(byteset, *(unsigned char *)str, &) && i < str_sz;
         str++, ++i)
        ;
    return str - a;
}

/* Providing strnstrn rather than strstr at the lowest level works better for
   string views where the string may not be null terminated. There needs to
   always be the additional constraint that a search cannot exceed the
   haystack length. */
size_t
sv_strnstrn(const char *haystack, ssize_t haystack_sz, const char *needle,
            ssize_t needle_sz)
{
    if (!haystack || !needle || needle_sz == 0 || *needle == '\0'
        || needle_sz > haystack_sz)
    {
        return haystack_sz;
    }
    if (2 == needle_sz)
    {
        return sv_twobyte_strnstrn((unsigned char *)haystack, haystack_sz,
                                   (unsigned char *)needle);
    }
    if (3 == needle_sz)
    {
        return sv_threebyte_strnstrn((unsigned char *)haystack, haystack_sz,
                                     (unsigned char *)needle);
    }
    if (4 == needle_sz)
    {
        return sv_fourbyte_strnstrn((unsigned char *)haystack, haystack_sz,
                                    (unsigned char *)needle);
    }
    return sv_two_way(haystack, haystack_sz, needle, needle_sz);
}

/* ==============   Post-Precomputation Two-Way Search    ================== */

/* Definitions for Two-Way String-Matching taken from original authors:

   CROCHEMORE M., PERRIN D., 1991, Two-way string-matching,
   Journal of the ACM 38(3):651-675.

   This algorithm is used for its simplicity and low space requirement.
   What follows is a massively simplified approximation (due to my own
   lack of knowledge) of glibc's approach with the help of the ESMAJ
   website on string matching and the original paper in ACM.

   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260

   Variable names and descriptions attempt to communicate authors' original
   meaning from the 1991 paper. */

/* Two Way string matching algorithm adapted from ESMAJ
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260

   Assumes the needle size is shorter than haystack size. Sizes are needed
   for string view operations because strings may not be null terminated
   and the string view library is most likely search a view rather than
   an entire string. */
static inline size_t
sv_two_way(const char *const haystack, ssize_t haystack_sz,
           const char *const needle, ssize_t needle_sz)
{
    ssize_t critical_pos;
    ssize_t period_dist;
    /* Preprocessing to get critical position and period distance. */
    const struct sv_factorization s = sv_maximal_suffix(needle, needle_sz);
    const struct sv_factorization r = sv_maximal_suffix_rev(needle, needle_sz);
    if (s.start_critical_pos > r.start_critical_pos)
    {
        critical_pos = s.start_critical_pos;
        period_dist = s.period_dist;
    }
    else
    {
        critical_pos = r.start_critical_pos;
        period_dist = r.period_dist;
    }
    /* Determine if memoization is be available due to found border/overlap. */
    if (sv_memcmp(needle, needle + period_dist, critical_pos + 1) == 0)
    {
        return sv_two_way_memoization((struct sv_two_way_pack){
            .haystack = haystack,
            .haystack_sz = haystack_sz,
            .needle = needle,
            .needle_sz = needle_sz,
            .period_dist = period_dist,
            .critical_pos = critical_pos,
        });
    }
    return sv_two_way_normal((struct sv_two_way_pack){
        .haystack = haystack,
        .haystack_sz = haystack_sz,
        .needle = needle,
        .needle_sz = needle_sz,
        .period_dist = period_dist,
        .critical_pos = critical_pos,
    });
}

/* Two Way string matching algorithm adapted from ESMAJ
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260 */
static inline size_t
sv_two_way_memoization(struct sv_two_way_pack p)
{
    ssize_t l_pos = 0;
    ssize_t r_pos = 0;
    /* Eliminate worst case quadratic time complexity with memoization. */
    ssize_t memoize_shift = -1;
    while (l_pos <= p.haystack_sz - p.needle_sz)
    {
        r_pos = sv_ssizet_max(p.critical_pos, memoize_shift) + 1;
        while (r_pos < p.needle_sz
               && p.needle[r_pos] == p.haystack[r_pos + l_pos])
        {
            ++r_pos;
        }

        if (r_pos < p.needle_sz)
        {
            l_pos += (r_pos - p.critical_pos);
            memoize_shift = -1;
            continue;
        }

        /* p.r_pos >= p.needle_sz */
        r_pos = p.critical_pos;
        while (r_pos > memoize_shift
               && p.needle[r_pos] == p.haystack[r_pos + l_pos])
        {
            --r_pos;
        }
        if (r_pos <= memoize_shift)
        {
            return l_pos;
        }
        l_pos += p.period_dist;
        /* Some prefix of needle coincides with the text. Memoize the length
           of this prefix to increase length of next shift, if possible. */
        memoize_shift = p.needle_sz - p.period_dist - 1;
    }
    return p.haystack_sz;
}

/* Two Way string matching algorithm adapted from ESMAJ
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260 */
static inline size_t
sv_two_way_normal(struct sv_two_way_pack p)
{
    p.period_dist
        = sv_ssizet_max(p.critical_pos + 1, p.needle_sz - p.critical_pos - 1)
          + 1;
    ssize_t l_pos = 0;
    ssize_t r_pos = 0;
    while (l_pos <= p.haystack_sz - p.needle_sz)
    {
        r_pos = p.critical_pos + 1;
        while (r_pos < p.needle_sz
               && p.needle[r_pos] == p.haystack[r_pos + l_pos])
        {
            ++r_pos;
        }

        if (r_pos < p.needle_sz)
        {
            l_pos += (r_pos - p.critical_pos);
            continue;
        }

        /* p.r_pos >= p.needle_sz */
        r_pos = p.critical_pos;
        while (r_pos >= 0 && p.needle[r_pos] == p.haystack[r_pos + l_pos])
        {
            --r_pos;
        }
        if (r_pos < 0)
        {
            return l_pos;
        }
        l_pos += p.period_dist;
    }
    return p.haystack_sz;
}

/* ================   Suffix and Critical Factorization    ================= */

/* Computing of the maximal suffix. Adapted from ESMAJ.
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260 */
static inline struct sv_factorization
sv_maximal_suffix(const char *const needle, ssize_t needle_sz)
{
    ssize_t suff_pos = -1;
    ssize_t period = 1;
    ssize_t last_rest = 0;
    ssize_t rest = 1;
    while (last_rest + rest < needle_sz)
    {
        switch (sv_char_cmp(needle[last_rest + rest], needle[suff_pos + rest]))
        {
        case LES:
            last_rest += rest;
            rest = 1;
            period = last_rest - suff_pos;
            break;
        case EQL:
            if (rest != period)
            {
                ++rest;
            }
            else
            {
                last_rest += period;
                rest = 1;
            }
            break;
        case GRT:
            suff_pos = last_rest;
            last_rest = suff_pos + 1;
            rest = period = 1;
            break;
        }
    }
    return (struct sv_factorization){.start_critical_pos = suff_pos,
                                     .period_dist = period};
}

/* Computing of the maximal suffix reverse. Sometimes called tilde.
   adapted from ESMAJ
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260 */
static inline struct sv_factorization
sv_maximal_suffix_rev(const char *const needle, ssize_t needle_sz)
{
    ssize_t suff_pos = -1;
    ssize_t period = 1;
    ssize_t last_rest = 0;
    ssize_t rest = 1;
    while (last_rest + rest < needle_sz)
    {
        switch (sv_char_cmp(needle[last_rest + rest], needle[suff_pos + rest]))
        {
        case GRT:
            last_rest += rest;
            rest = 1;
            period = last_rest - suff_pos;
            break;
        case EQL:
            if (rest != period)
            {
                ++rest;
            }
            else
            {
                last_rest += period;
                rest = 1;
            }
            break;
        case LES:
            suff_pos = last_rest;
            last_rest = suff_pos + 1;
            rest = period = 1;
            break;
        }
    }
    return (struct sv_factorization){.start_critical_pos = suff_pos,
                                     .period_dist = period};
}

/* ======================   Brute Force Search    ========================== */

/* All brute force searches adapted from musl C library.
   http://git.musl-libc.org/cgit/musl/tree/src/string/strstr.c
   They must stop the search at haystack size and therefore required slight
   modification because string views may not be null terminated. */

static inline size_t
sv_twobyte_strnstrn(const unsigned char *h, size_t sz, const unsigned char *n)
{
    uint16_t nw = n[0] << 8 | n[1];
    uint16_t hw = h[0] << 8 | h[1];
    size_t i = 0;
    for (h++, i++; i < sz && *h && hw != nw; hw = (hw << 8) | *++h, ++i)
    {}
    return (i < sz) ? i - 1 : sz;
}

static inline size_t
sv_threebyte_strnstrn(const unsigned char *h, size_t sz, const unsigned char *n)
{
    uint32_t nw = (uint32_t)n[0] << 24 | n[1] << 16 | n[2] << 8;
    uint32_t hw = (uint32_t)h[0] << 24 | h[1] << 16 | h[2] << 8;
    size_t i = 0;
    for (h += 2, i += 2; i < sz && *h && hw != nw; hw = (hw | *++h) << 8, ++i)
    {}
    return (i < sz) ? i - 2 : sz;
}

static inline size_t
sv_fourbyte_strnstrn(const unsigned char *h, size_t sz, const unsigned char *n)
{
    uint32_t nw = (uint32_t)n[0] << 24 | n[1] << 16 | n[2] << 8 | n[3];
    uint32_t hw = (uint32_t)h[0] << 24 | h[1] << 16 | h[2] << 8 | h[3];
    size_t i = 0;
    for (h += 3, i += 3; i < sz && *h && hw != nw; hw = (hw << 8) | *++h, ++i)
    {}
    return (i < sz) ? i - 3 : sz;
}
