#include "string_view.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum cmp
{
    LES = -1,
    EQL = 0,
    GRT = 1,
};

struct two_way_pack
{
    const char *const haystack;
    ssize_t haystack_sz;
    const char *const needle;
    ssize_t needle_sz;
    ssize_t suffix_i;
    ssize_t global_period;
    ssize_t longest_prefix_suffix;
};

struct factorization
{
    ssize_t suffix;
    ssize_t period;
};

static const char *const nil = "";

static size_t sv_min(size_t, size_t);
static struct factorization sv_maximal_suffix(const char *, ssize_t);
static struct factorization sv_maximal_suffix_rev(const char *, ssize_t);
static const char *sv_two_way(const char *, ssize_t, const char *, ssize_t);
static inline const char *sv_way_one(struct two_way_pack);
static inline const char *sv_way_two(struct two_way_pack);

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
    size_t sz = 0;
    while (str[sz] != '\0' && sz < n)
    {
        ++sz;
    }
    return (string_view){.s = str, .sz = n};
}

string_view
sv_delim(const char *const str, const char *const delim)
{
    if (!str)
    {
        return (string_view){.s = nil, .sz = 0};
    }
    return sv_begin_tok(str, strlen(delim), delim);
}

char *
sv_alloc(string_view sv)
{
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
    if (!src_str || str_sz == 0)
    {
        return (string_view){.s = nil, .sz = 0};
    }
    size_t i = 0;
    while (src_str[i] != '\0' && i < str_sz)
    {
        ++i;
    }
    return (string_view){.s = src_str, .sz = i};
}

void
sv_fill(char *dest_buf, size_t dest_sz, const string_view src)
{
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
sv_str(struct string_view sv)
{
    return sv.s;
}

int
sv_svcmp(string_view sv1, string_view sv2)
{
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
sv_begin(const string_view *const sv)
{
    return sv->s;
}

const char *
sv_end(const string_view *const sv)
{
    return sv->s + sv->sz;
}

const char *
sv_next(const char *c)
{
    return ++c;
}

string_view
sv_begin_tok(const char *const data, const size_t delim_sz,
             const char *const delim)
{
    string_view start = {.s = data, .sz = strlen(data)};
    const size_t start_not = sv_find_first_not_of(start, delim, delim_sz);
    start.s += start_not;
    if (*start.s == '\0')
    {
        return (string_view){.s = nil, .sz = 0};
    }
    size_t found_pos = sv_find_first_of(start, delim);
    return sv_substr(start, 0, found_pos);
}

bool
sv_end_tok(const string_view *sv)
{
    return 0 == sv->sz;
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
    next += sv_find_first_not_of((string_view){.s = next, .sz = next_sz}, delim,
                                 delim_sz);
    if (*next == '\0')
    {
        return (string_view){.s = next, .sz = 0};
    }
    const char *const found
        = sv_two_way(next, (ssize_t)next_sz, delim, (ssize_t)delim_sz);
    return (string_view){.s = next, .sz = found - next};
}

string_view
sv_remove_prefix(const string_view sv, const size_t n)
{
    const size_t remove = sv_min(sv.sz, n);
    return (string_view){.s = sv.s + remove, .sz = sv.sz - remove};
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
    const char *const found = sv_two_way(haystack.s, (ssize_t)haystack.sz,
                                         needle.s, (ssize_t)needle.sz);
    return (found == haystack.s + haystack.sz) ? false : true;
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
    const char *const end = haystack.s + haystack.sz;
    const char *const found = sv_two_way(haystack.s, (ssize_t)haystack.sz,
                                         needle.s, (ssize_t)needle.sz);
    if (found == end)
    {
        return (string_view){.s = end, .sz = 0};
    }
    return (string_view){.s = found, .sz = needle.sz};
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
    const char *const end = haystack.s + haystack.sz;
    if (0 == needle_sz)
    {
        return (string_view){.s = end, .sz = 0};
    }
    const char *found = sv_two_way(haystack.s, (ssize_t)haystack.sz, needle,
                                   (ssize_t)needle_sz);
    if (found == end)
    {
        return (string_view){.s = end, .sz = 0};
    }
    return (string_view){.s = found, .sz = needle_sz};
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
    const char *const end = haystack + haystack_sz;
    if ('\0' == *needle)
    {
        return (string_view){.s = end, .sz = 0};
    }
    const char *found = sv_two_way(haystack, (ssize_t)haystack_sz, needle,
                                   (ssize_t)needle_sz);
    if (found == end)
    {
        return (string_view){.s = end, .sz = 0};
    }
    return (string_view){.s = found, .sz = needle_sz};
}

size_t
sv_find_first_of(string_view haystack, const char *const needle)
{
    const size_t delim_sz = strlen(needle);
    if (delim_sz > haystack.sz)
    {
        return haystack.sz;
    }
    const char *const found = sv_two_way(haystack.s, (ssize_t)haystack.sz,
                                         needle, (ssize_t)delim_sz);
    return found - haystack.s;
}

size_t
sv_find_last_of(string_view haystack, const char *const needle)
{
    if (strlen(needle) >= haystack.sz)
    {
        return haystack.sz;
    }
    const char *last_found = haystack.s + haystack.sz;
    const size_t needle_sz = strlen(needle);
    const char *found = haystack.s;
    for (;;)
    {
        found = sv_two_way(found, (ssize_t)haystack.sz - (found - haystack.s),
                           needle, (ssize_t)needle_sz);
        if ((size_t)(found - haystack.s) == haystack.sz)
        {
            break;
        }
        last_found = found;
        ++found;
    }
    return last_found - haystack.s;
}

size_t
sv_find_first_not_of(string_view haystack, const char *const needle,
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
sv_find_last_not_of(string_view haystack, const char *const needle)
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
sv_ssize_t_max(ssize_t a, ssize_t b)
{
    return a > b ? a : b;
}

static inline enum cmp
sv_char_cmp(char a, char b)
{
    return (a > b) - (a < b);
}

/* Computing of the maximal suffix. Adapted from ESMAJ.
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260 */
static struct factorization
sv_maximal_suffix(const char *const needle, ssize_t needle_sz)
{
    ssize_t max_suf = -1;
    ssize_t j = 0;
    ssize_t period = 1;
    ssize_t k = 1;
    while (j + k < needle_sz)
    {
        switch (sv_char_cmp(needle[j + k], needle[max_suf + k]))
        {
        case LES:
        {
            j += k;
            k = 1;
            period = j - max_suf;
        }
        break;
        case EQL:
        {
            if (k != period)
            {

                ++k;
            }
            else
            {
                j += period;
                k = 1;
            }
        }
        break;
        case GRT:
        {
            max_suf = j;
            j = max_suf + 1;
            k = period = 1;
        }
        break;
        }
    }
    return (struct factorization){.suffix = max_suf, .period = period};
}

/* Computing of the maximal suffix reverse. Sometimes called tilde.
   adapted from ESMAJ
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260 */
static struct factorization
sv_maximal_suffix_rev(const char *const needle, ssize_t needle_sz)
{
    ssize_t max_suf = -1;
    ssize_t j = 0;
    ssize_t period = 1;
    ssize_t k = 1;
    while (j + k < (int)needle_sz)
    {
        switch (sv_char_cmp(needle[j + k], needle[max_suf + k]))
        {
        case GRT:
        {
            j += k;
            k = 1;
            period = j - max_suf;
        }
        break;
        case EQL:
        {
            if (k != period)
            {
                ++k;
            }
            else
            {
                j += period;
                k = 1;
            }
        }
        break;
        case LES:
        {
            max_suf = (int)j;
            j = max_suf + 1;
            k = period = 1;
        }
        break;
        }
    }
    return (struct factorization){.suffix = max_suf, .period = period};
}

/* Two Way string matching algorithm adapted from ESMAJ
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260

   It is helpful to be able to pass in the string and the length
   as I choose because strstr has no concept of where my string views
   end and therefore searches the entire remaining string which could
   be wasteful. Therefore, I needed my own implementation of string
   searching so I can control the size of the search to be the size
   of a string view. I also return the pointer even if it is to the
   the null terminating character so that in many cases I can return
   the full string when something is not found. This is faster and
   more convenient than running another search to find the end of
   the string. The null returned by strstr is often not helpful. */
static const char *
sv_two_way(const char *const haystack, ssize_t haystack_sz,
           const char *const needle, ssize_t needle_sz)
{
    ssize_t ell;
    ssize_t global_period;
    /* Preprocessing */
    struct factorization i = sv_maximal_suffix(needle, needle_sz);
    struct factorization j = sv_maximal_suffix_rev(needle, needle_sz);
    if (i.suffix > j.suffix)
    {
        ell = i.suffix;
        global_period = i.period;
    }
    else
    {
        ell = j.suffix;
        global_period = j.period;
    }
    /* Searching */
    if (memcmp(needle, needle + global_period, ell + 1) == 0)
    {
        return sv_way_one((struct two_way_pack){
            .haystack = haystack,
            .haystack_sz = haystack_sz,
            .needle = needle,
            .needle_sz = needle_sz,
            .suffix_i = i.suffix,
            .global_period = global_period,
            .longest_prefix_suffix = ell,
        });
    }
    return sv_way_two((struct two_way_pack){
        .haystack = haystack,
        .haystack_sz = haystack_sz,
        .needle = needle,
        .needle_sz = needle_sz,
        .suffix_i = i.suffix,
        .global_period = global_period,
        .longest_prefix_suffix = ell,
    });
}

static inline const char *
sv_way_one(struct two_way_pack p)
{
    ssize_t j = 0;
    ssize_t memorize_shift = -1;
    while (j <= p.haystack_sz - p.needle_sz)
    {
        p.suffix_i
            = sv_ssize_t_max(p.longest_prefix_suffix, memorize_shift) + 1;
        while (p.suffix_i < p.needle_sz
               && p.needle[p.suffix_i] == p.haystack[p.suffix_i + j])
        {
            ++p.suffix_i;
        }
        if (p.suffix_i < p.needle_sz)
        {
            j += (p.suffix_i - p.longest_prefix_suffix);
            memorize_shift = -1;
            continue;
        }
        p.suffix_i = p.longest_prefix_suffix;
        while (p.suffix_i > memorize_shift
               && p.needle[p.suffix_i] == p.haystack[p.suffix_i + j])
        {
            --p.suffix_i;
        }
        if (p.suffix_i <= memorize_shift)
        {
            return p.haystack + j;
        }
        j += p.global_period;
        memorize_shift = p.needle_sz - p.global_period - 1;
    }
    return p.haystack + p.haystack_sz;
}

static inline const char *
sv_way_two(struct two_way_pack p)
{
    p.global_period = sv_ssize_t_max(p.longest_prefix_suffix + 1,
                                     p.needle_sz - p.longest_prefix_suffix - 1)
                      + 1;
    ssize_t j = 0;
    while (j <= p.haystack_sz - p.needle_sz)
    {
        p.suffix_i = p.longest_prefix_suffix + 1;
        while (p.suffix_i < p.needle_sz
               && p.needle[p.suffix_i] == p.haystack[p.suffix_i + j])
        {
            ++p.suffix_i;
        }
        if (p.suffix_i < p.needle_sz)
        {
            j += (p.suffix_i - p.longest_prefix_suffix);
            continue;
        }
        p.suffix_i = p.longest_prefix_suffix;
        while (p.suffix_i >= 0
               && p.needle[p.suffix_i] == p.haystack[p.suffix_i + j])
        {
            --p.suffix_i;
        }
        if (p.suffix_i < 0)
        {
            return p.haystack + j;
        }
        j += p.global_period;
    }
    return p.haystack + p.haystack_sz;
}
