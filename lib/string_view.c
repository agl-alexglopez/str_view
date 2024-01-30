#include "string_view.h"
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *const nil = "";

static size_t min(size_t, size_t);

string_view
sv(const char *const str)
{
    if (!str)
    {
        return (string_view){.s = nil, .sz = 0};
    }
    size_t sz = 0;
    while (str[sz] != '\0')
    {
        ++sz;
    }
    return (string_view){.s = str, .sz = sz};
}

string_view
sv_delim(const char *const str, const char *const delim)
{
    if (!str)
    {
        return (string_view){.s = nil, .sz = 0};
    }
    return sv_begin_tok(
        (string_view){
            .s = str,
            .sz = strlen(str),
        },
        delim, strlen(delim));
}

void
sv_print(string_view s)
{
    if (!s.s || nil == s.s || 0 == s.sz)
    {
        return;
    }
    (void)fwrite(s.s, sizeof(char), s.sz, stdout);
}

string_view
sv_copy(const char *const src_str, const size_t str_sz)
{
    if (!src_str || str_sz == 0)
    {
        return (string_view){.s = nil, .sz = 0};
    }
    return (string_view){.s = src_str, .sz = str_sz};
}

void
sv_fill(char *dest_buf, size_t str_sz, const string_view src)
{
    const size_t paste = min(str_sz, src.sz);
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
    const size_t sz = min(sv1.sz, sv2.sz);
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
    const size_t sz = min(sv.sz, n);
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
sv_begin_tok(string_view sv, const char *const delim, const size_t delim_sz)
{
    const size_t start_not = sv_find_first_not_of(sv, delim, delim_sz);
    sv.s += start_not;
    sv.sz -= start_not;
    sv = sv_substr(sv, 0, sv_find_first_of(sv, delim));
    return sv;
}

bool
sv_end_tok(const string_view *sv)
{
    return 0 == sv->sz;
}

string_view
sv_next_tok(string_view sv, const char *const delim, const size_t delim_sz)
{
    if (sv.s[sv.sz] == '\0')
    {
        return (string_view){.s = sv.s + sv.sz, .sz = 0};
    }
    const char *next = sv.s + sv.sz + delim_sz;
    next += sv_find_first_not_of((string_view){.s = next, .sz = ULLONG_MAX},
                                 delim, delim_sz);
    const char *found = strstr(next, delim);
    if (NULL == found)
    {
        size_t i = 0;
        while (next[i] != '\0')
        {
            ++i;
        }
        return (string_view){.s = next, .sz = i};
    }
    return (string_view){.s = next, .sz = found - next};
}

string_view
sv_remove_prefix(const string_view sv, const size_t n)
{
    const size_t remove = min(sv.sz, n);
    return (string_view){.s = sv.s + remove, .sz = sv.sz - remove};
}

string_view
sv_remove_suffix(const string_view sv, const size_t n)
{
    return (string_view){.s = sv.s, .sz = sv.sz - min(sv.sz, n)};
}

string_view
sv_substr(string_view sv, size_t pos, size_t count)
{
    if (pos > sv.sz)
    {
        printf("string_view index out of range. pos=%zu size=%zu", pos, sv.sz);
        exit(1);
    }
    return (string_view){.s = sv.s + pos, .sz = min(count, sv.sz - pos)};
}

size_t
sv_find_first_of(string_view sv, const char *const delim)
{
    if (strlen(delim) >= sv.sz)
    {
        return sv.sz;
    }
    const char *const found = strstr(sv.s, delim);
    if (NULL == found)
    {
        return sv.sz;
    }
    return found - sv.s;
}

size_t
sv_find_last_of(string_view sv, const char *const delim)
{
    if (strlen(delim) >= sv.sz)
    {
        return sv.sz;
    }
    const char *last_found = NULL;
    const char *found = sv.s;
    while ((found = strstr(found, delim)) != NULL)
    {
        last_found = found;
        ++found;
    }
    if (NULL == last_found)
    {
        return sv.sz;
    }
    return last_found - sv.s;
}

size_t
sv_find_first_not_of(string_view sv, const char *const delim,
                     const size_t delim_sz)
{
    if (delim_sz > sv.sz)
    {
        return 0;
    }
    size_t i = 0;
    size_t delim_i = 0;
    while (sv.s[i] != '\0' && delim[delim_i] == sv.s[i])
    {
        ++i;
        delim_i = (delim_i + 1) % delim_sz;
    }
    /* We need to also reset to the last mismatch we found. Imagine
       we started to find the delimeter but then the string changed
       into a mismatch. We go back to get characters that are partially
       in the delimeter. */
    return i - delim_i;
}

size_t
sv_find_last_not_of(string_view sv, const char *const delim)
{
    const size_t delim_sz = strlen(delim);
    if (delim_sz >= sv.sz)
    {
        return sv.sz;
    }
    const char *last_found = NULL;
    const char *found = sv.s;
    while ((found = strstr(found, delim)) != NULL)
    {
        last_found = found;
        ++found;
    }
    if (NULL == last_found)
    {
        return sv.sz;
    }
    return (last_found + delim_sz) - sv.s;
}

size_t
sv_npos(string_view sv)
{
    return sv.sz;
}

static inline size_t
min(size_t a, size_t b)
{
    return a < b ? a : b;
}
