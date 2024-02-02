/* Author: Alexander G. Lopez
   File: string_view.c
   ===================
   This file implements the string_view interface as an approximation of C++
   string_view type. There are some minor differences and C flavor thrown
   in with the additional twist of a reimplementation of the Two-Way
   String-Searching algorithm, similar to glibc. */
#include "string_view.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Avoid giving the user a chance to dereference null as much as posssible
   by returning this for various edgecases when it makes sense to communicate
   empty, null, invalid, not found etc. Used on cases by case basis.
   It is usually better to justify giving back the user pointer in a
   string_view even if it sized 0 and pointing to null terminator. */
static const char *const nil = "";

/* Two-Way String-Searching Algorithm by Crochemore and Perrin. See
   the end of the file for implementation details. */
static size_t sv_two_way(const char *, ssize_t, const char *, ssize_t);
static size_t sv_min(size_t, size_t);

/* ===================   Interface Implementation   ====================== */

string_view
sv(const char *const str)
{
    if (!str)
    {
        return (string_view){.s = nil, .sz = 0};
    }
    return (string_view){.s = str, .sz = strlen(str)};
}

string_view
sv_n(const char *const str, size_t n)
{
    if (!str || n == 0)
    {
        return (string_view){.s = nil, .sz = 0};
    }
    return (string_view){.s = str, .sz = strnlen(str, n)};
}

string_view
sv_delim(const char *const str, const char *const delim)
{
    if (!str)
    {
        return (string_view){.s = nil, .sz = 0};
    }
    if (!delim)
    {
        return (string_view){.s = str, .sz = strlen(str)};
    }
    return sv_begin_tok(str, strlen(delim), delim);
}

char *
sv_alloc(string_view sv)
{
    if (!sv.s || sv.sz == 0)
    {
        (void)fprintf(stderr, "sv_alloc accepted invalid string_view.\n");
        exit(1);
    }
    char *const ret = malloc(sv.sz + 1);
    sv_fill(ret, sv.sz, sv);
    return ret;
}

void
sv_free(char *const s)
{
    if (!s)
    {
        return;
    }
    free(s);
}

void
sv_print(string_view s)
{
    if (!s.s || nil == s.s || 0 == s.sz)
    {
        return;
    }
    /* printf does not output the null terminator in normal strings so
       as long as we output correct number of characters we do the same */
    (void)fwrite(s.s, sizeof(char), s.sz, stdout);
}

string_view
sv_copy(const char *const src_str, const size_t str_sz)
{
    return sv_n(src_str, str_sz);
}

void
sv_fill(char *dest_buf, size_t dest_sz, const string_view src)
{
    if (!dest_buf || 0 == dest_sz || !src.s || 0 == src.sz)
    {
        return;
    }
    const size_t paste = sv_min(dest_sz, src.sz);
    memmove(dest_buf, src.s, paste);
    dest_buf[paste] = '\0';
}

bool
sv_empty(const string_view s)
{
    return s.sz == 0;
}

size_t
sv_len(string_view sv)
{
    return sv.sz;
}

char
sv_at(string_view sv, size_t i)
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

const char *
sv_data(string_view sv)
{
    return sv.s;
}

void
sv_swap(string_view *a, string_view *b)
{
    if (a == b || !a || !b)
    {
        return;
    }
    const string_view tmp_b = (string_view){.s = b->s, .sz = b->sz};
    b->s = a->s;
    b->sz = a->sz;
    a->s = tmp_b.s;
    a->sz = tmp_b.sz;
}

int
sv_svcmp(string_view sv1, string_view sv2)
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
        return 0;
    }
    if (i < sv1.sz && i < sv2.sz)
    {
        return ((uint8_t)sv1.s[i] < (uint8_t)sv2.s[i] ? -1 : +1);
    }
    return (i < sv1.sz) ? +1 : -1;
}

int
sv_strcmp(string_view sv, const char *str)
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
        return 0;
    }
    if (i < sv.sz && str[i] != '\0')
    {
        return ((uint8_t)sv.s[i] < (uint8_t)str[i] ? -1 : +1);
    }
    return (i < sv.sz) ? +1 : -1;
}

int
sv_strncmp(string_view sv, const char *str, const size_t n)
{
    if (!sv.s || !str)
    {
        (void)fprintf(stderr, "sv_strcmp cannot compare NULL.\n");
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
        return 0;
    }
    /* strncmp compares the first at most n bytes inclusive */
    if (i < sv.sz && sz <= n)
    {
        return ((uint8_t)sv.s[i] < (uint8_t)str[i] ? -1 : +1);
    }
    return (i < sv.sz) ? +1 : -1;
}

char
sv_front(struct string_view sv)
{
    if (!sv.s || 0 == sv.sz)
    {
        return *nil;
    }
    return *sv.s;
}

char
sv_back(struct string_view sv)
{
    if (!sv.s || 0 == sv.sz)
    {
        return *nil;
    }
    return sv.s[sv.sz - 1];
}

const char *
sv_begin(const string_view sv)
{
    if (!sv.s)
    {
        return nil;
    }
    return sv.s;
}

const char *
sv_end(const string_view sv)
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

string_view
sv_begin_tok(const char *const data, const size_t delim_sz,
             const char *const delim)
{
    if (!data)
    {
        return (string_view){.s = nil, .sz = 0};
    }
    if (!delim)
    {
        return (string_view){.s = data + strlen(data), 0};
    }
    string_view start = {.s = data, .sz = strlen(data)};
    const size_t start_not = sv_first_not_of(start, delim, delim_sz);
    start.s += start_not;
    if (*start.s == '\0')
    {
        return (string_view){.s = start.s, .sz = 0};
    }
    size_t found_pos = sv_first_of(start, delim);
    return sv_substr(start, 0, found_pos);
}

bool
sv_end_tok(const string_view sv)
{
    return 0 == sv.sz;
}

string_view
sv_next_tok(string_view sv, const size_t delim_sz, const char *const delim)
{
    if (sv.s[sv.sz] == '\0')
    {
        return (string_view){.s = sv.s + sv.sz, .sz = 0};
    }
    const char *next = sv.s + sv.sz + delim_sz;
    const size_t next_sz = strlen(next);
    next += sv_first_not_of((string_view){.s = next, .sz = next_sz}, delim,
                            delim_sz);
    if (*next == '\0')
    {
        return (string_view){.s = next, .sz = 0};
    }
    const size_t found
        = sv_two_way(next, (ssize_t)next_sz, delim, (ssize_t)delim_sz);
    return (string_view){.s = next, .sz = found};
}

bool
sv_starts_with(string_view sv, string_view prefix)
{
    if (prefix.sz > sv.sz)
    {
        return false;
    }
    return sv_svcmp(sv_substr(sv, 0, prefix.sz), prefix) == 0;
}

string_view
sv_remove_prefix(const string_view sv, const size_t n)
{
    const size_t remove = sv_min(sv.sz, n);
    return (string_view){.s = sv.s + remove, .sz = sv.sz - remove};
}

bool
sv_ends_with(string_view sv, string_view suffix)
{
    if (suffix.sz > sv.sz)
    {
        return false;
    }
    return sv_svcmp(sv_substr(sv, sv.sz - suffix.sz, suffix.sz), suffix) == 0;
}

string_view
sv_remove_suffix(const string_view sv, const size_t n)
{
    return (string_view){.s = sv.s, .sz = sv.sz - sv_min(sv.sz, n)};
}

string_view
sv_substr(string_view sv, size_t pos, size_t count)
{
    if (pos > sv.sz)
    {
        printf("string_view index out of range. pos=%zu size=%zu", pos, sv.sz);
        exit(1);
    }
    return (string_view){.s = sv.s + pos, .sz = sv_min(count, sv.sz - pos)};
}

bool
sv_contains(string_view haystack, string_view needle)
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
    const size_t found = sv_two_way(haystack.s, (ssize_t)haystack.sz, needle.s,
                                    (ssize_t)needle.sz);
    return (found == haystack.sz) ? false : true;
}

string_view
sv_svsv(string_view haystack, string_view needle)
{
    if (needle.sz > haystack.sz)
    {
        return (string_view){.s = nil, .sz = 0};
    }
    if (sv_empty(haystack))
    {
        return haystack;
    }
    if (sv_empty(needle))
    {
        return (string_view){.s = nil, .sz = 0};
    }
    const size_t found = sv_two_way(haystack.s, (ssize_t)haystack.sz, needle.s,
                                    (ssize_t)needle.sz);
    if (found == haystack.sz)
    {
        return (string_view){.s = haystack.s + haystack.sz, .sz = 0};
    }
    return (string_view){.s = haystack.s + found, .sz = needle.sz};
}

string_view
sv_svstr(string_view haystack, const char *needle, size_t needle_sz)
{
    if (needle_sz > haystack.sz)
    {
        return (string_view){.s = nil, .sz = 0};
    }
    if (sv_empty(haystack))
    {
        return haystack;
    }
    if (0 == needle_sz)
    {
        return (string_view){.s = haystack.s + haystack.sz, .sz = 0};
    }
    const size_t found = sv_two_way(haystack.s, (ssize_t)haystack.sz, needle,
                                    (ssize_t)needle_sz);
    if (found == haystack.sz)
    {
        return (string_view){.s = haystack.s + haystack.sz, .sz = 0};
    }
    return (string_view){.s = haystack.s + found, .sz = needle_sz};
}

string_view
sv_strstr(const char *haystack, size_t haystack_sz, const char *needle,
          size_t needle_sz)
{
    if (!needle || !haystack || needle_sz > haystack_sz || 0 == needle_sz)
    {
        return (string_view){.s = nil, .sz = 0};
    }
    if ('\0' == *haystack)
    {
        return (string_view){.s = haystack, .sz = 0};
    }
    if ('\0' == *needle)
    {
        return (string_view){.s = haystack + haystack_sz, .sz = 0};
    }
    const size_t found = sv_two_way(haystack, (ssize_t)haystack_sz, needle,
                                    (ssize_t)needle_sz);
    if (found == haystack_sz)
    {
        return (string_view){.s = haystack + haystack_sz, .sz = 0};
    }
    return (string_view){.s = haystack + found, .sz = needle_sz};
}

size_t
sv_first_of(string_view haystack, const char *const needle)
{
    const size_t delim_sz = strlen(needle);
    if (delim_sz > haystack.sz)
    {
        return haystack.sz;
    }
    return sv_two_way(haystack.s, (ssize_t)haystack.sz, needle,
                      (ssize_t)delim_sz);
}

size_t
sv_last_of(string_view haystack, const char *const needle)
{
    if (strlen(needle) >= haystack.sz)
    {
        return haystack.sz;
    }
    const size_t needle_sz = strlen(needle);
    size_t last_found = haystack.sz;
    size_t i = 0;
    for (;;)
    {
        i += sv_two_way(haystack.s + i, (ssize_t)(haystack.sz - i), needle,
                        (ssize_t)needle_sz);
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
sv_first_not_of(string_view haystack, const char *const needle,
                const size_t needle_sz)
{
    if (needle_sz > haystack.sz)
    {
        return 0;
    }
    size_t i = 0;
    size_t delim_i = 0;
    while (i < haystack.sz && needle[delim_i] == haystack.s[i])
    {
        ++i;
        delim_i = (delim_i + 1) % needle_sz;
    }
    /* We need to also reset to the last mismatch we found. Imagine
       we started to find the delimeter but then the string changed
       into a mismatch. We go back to get characters that are partially
       in the delimeter. */
    return i - delim_i;
}

size_t
sv_last_not_of(string_view haystack, const char *const needle)
{
    const size_t delim_sz = strlen(needle);
    if (delim_sz >= haystack.sz)
    {
        return haystack.sz;
    }
    const char *last_found = NULL;
    const char *found = haystack.s;
    while ((found = strstr(found, needle)) != NULL)
    {
        if ((size_t)(found - haystack.s) > haystack.sz)
        {
            break;
        }
        last_found = found;
        found += delim_sz;
    }
    if (NULL == last_found)
    {
        return haystack.sz;
    }
    return (last_found + delim_sz) - haystack.s;
}

size_t
sv_npos(string_view sv)
{
    return sv.sz;
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

/* =======================   String Searching   ========================== */

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

static struct sv_factorization sv_maximal_suffix(const char *, ssize_t);
static struct sv_factorization sv_maximal_suffix_rev(const char *, ssize_t);
static size_t sv_two_way_period_memoization(struct sv_two_way_pack);
static size_t sv_two_way_normal(struct sv_two_way_pack);
static enum sv_cmp sv_char_cmp(char, char);

static inline enum sv_cmp
sv_char_cmp(char a, char b)
{
    return (a > b) - (a < b);
}

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
        {
            last_rest += rest;
            rest = 1;
            period = last_rest - suff_pos;
        }
        break;
        case SV_EQL:
        {
            if (rest != period)
            {
                ++rest;
            }
            else
            {
                last_rest += period;
                rest = 1;
            }
        }
        break;
        case SV_GRT:
        {
            suff_pos = last_rest;
            last_rest = suff_pos + 1;
            rest = period = 1;
        }
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
    while (last_rest + rest < (int)needle_sz)
    {
        switch (sv_char_cmp(needle[last_rest + rest], needle[suff_pos + rest]))
        {
        case SV_GRT:
        {
            last_rest += rest;
            rest = 1;
            period = last_rest - suff_pos;
        }
        break;
        case SV_EQL:
        {
            if (rest != period)
            {
                ++rest;
            }
            else
            {
                last_rest += period;
                rest = 1;
            }
        }
        break;
        case SV_LES:
        {
            suff_pos = (int)last_rest;
            last_rest = suff_pos + 1;
            rest = period = 1;
        }
        break;
        }
    }
    return (struct sv_factorization){.start_critical_pos = suff_pos,
                                     .period_dist = period};
}

/* Two Way string matching algorithm adapted from ESMAJ
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260

   It is helpful to be able to pass in the string and the length
   as I choose because strstr has no concept of where my string views
   end and therefore searches the entire remaining string which could
   be wasteful. Therefore, I needed my own implementation of string
   searching so I can control the size of the search to be the size
   of a string view. I also return the position even if it is at the
   null terminating character so that in many cases I can return
   the full string when something is not found. This is faster and
   more convenient than running another search to find the end of
   the string. The null returned by strstr is often not helpful. */
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
    if (memcmp(needle, needle + period_dist, critical_pos + 1) == 0)
    {
        return sv_two_way_period_memoization((struct sv_two_way_pack){
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
sv_two_way_period_memoization(struct sv_two_way_pack p)
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
