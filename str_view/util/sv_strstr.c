#include "sv_util.h"

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

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

enum sv_cmp
{
    SV_LES = -1,
    SV_EQL = 0,
    SV_GRT = 1,
};

/* =========================   Prototypes   =============================== */

static size_t sv_two_way(const char *, ssize_t, const char *, ssize_t);
static struct sv_factorization sv_maximal_suffix(const char *, ssize_t);
static struct sv_factorization sv_maximal_suffix_rev(const char *, ssize_t);
static size_t sv_two_way_memoization(struct sv_two_way_pack);
static size_t sv_two_way_normal(struct sv_two_way_pack);
static enum sv_cmp sv_char_cmp(char, char);
static ssize_t sv_ssizet_max(ssize_t, ssize_t);
static size_t sv_twobyte_strnstrn(const unsigned char *, size_t,
                                  const unsigned char *);
static size_t sv_threebyte_strnstrn(const unsigned char *, size_t,
                                    const unsigned char *);
static size_t sv_fourbyte_strnstrn(const unsigned char *, size_t,
                                   const unsigned char *);

/* =========================    strnstrn    =============================== */

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

/* ================   Suffix and Critical Factorization    ===================
 */

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
        case SV_LES:
            last_rest += rest;
            rest = 1;
            period = last_rest - suff_pos;
            break;
        case SV_EQL:
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
        case SV_GRT:
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
        case SV_GRT:
            last_rest += rest;
            rest = 1;
            period = last_rest - suff_pos;
            break;
        case SV_EQL:
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
        case SV_LES:
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

/* ======================   Static Helpers    ============================= */

static inline ssize_t
sv_ssizet_max(ssize_t a, ssize_t b)
{
    return a > b ? a : b;
}

static inline enum sv_cmp
sv_char_cmp(char a, char b)
{
    return (a > b) - (a < b);
}
