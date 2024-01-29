#include "string_view.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *const nil = "";

static size_t max(size_t, size_t);
static size_t min(size_t, size_t);

string_view
sv_from_null(const char *const str)
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
sv_from_delim(const char *const str, char delim)
{
    if (!str)
    {
        return (string_view){.s = nil, .sz = 0};
    }
    size_t sz = 0;
    while (str[sz] != delim && str[sz] != '\0')
    {
        ++sz;
    }
    return (string_view){.s = str, .sz = sz};
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
                      "string_view index out of range. Size=%zu, i=%zu\n",
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
sv_data(struct string_view sv)
{
    return sv.s;
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
sv_begin_tok(string_view sv, char delim)
{
    return sv_copy(sv.s, sv_find_first_of(sv, delim));
}

bool
sv_end_tok(const string_view *sv, char delim)
{
    return sv_find_first_of(*sv, delim) == 0;
}

string_view
sv_next_tok(string_view sv, char delim)
{
    if (sv.s[sv.sz - 1] == '\0')
    {
        return (string_view){.s = sv.s + sv.sz, .sz = 0};
    }
    const char *next_search = sv.s + sv.sz + 1;
    size_t next_size = 0;
    while (next_search[next_size] != delim && next_search[next_size] != '\0')
    {
        ++next_size;
    }
    return (string_view){.s = next_search, .sz = next_size};
}

size_t
sv_find_first_of(string_view sv, char delim)
{
    size_t i = 0;
    while (sv.s[i] != delim && sv.s[i] != '\0')
    {
        ++i;
    }
    return i;
}

size_t
sv_npos(string_view sv)
{
    return sv.sz;
}

static inline size_t
max(size_t a, size_t b)
{
    return a > b ? a : b;
}

static inline size_t
min(size_t a, size_t b)
{
    return a < b ? a : b;
}
