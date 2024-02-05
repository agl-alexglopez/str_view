/* Author: Alexander G. Lopez
   File: string_view.c
   ===================
   This file implements the string_view interface as an approximation of C++
   string_view type. There are some minor differences and C flavor thrown
   in with the additional twist of a reimplementation of the Two-Way
   String-Searching algorithm, similar to glibc. */
#include "str_view.h"
#include "sv_util.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

/* Avoid giving the user a chance to dereference null as much as posssible
   by returning this for various edgecases when it makes sense to communicate
   empty, null, invalid, not found etc. Used on cases by case basis.
   It is usually better to justify giving back the user pointer in a
   string_view even if it sized 0 and pointing to null terminator. */
static const char *const nil = "";

static size_t sv_after_find(str_view, str_view);
static size_t sv_min(size_t, size_t);

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
