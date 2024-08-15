/* Author: Alexander G. Lopez
   ==========================
   This file implements the str_view interface as an interpretation of C++
   string_view type. There are some minor differences and C flavor thrown
   in. Additionally, there is a provided reimplementation of the Two-Way
   String-Searching algorithm, similar to glibc. */
#include "str_view.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* ========================   Type Definitions   =========================== */

/* Return the factorization step of two-way search in precompute phase. */
struct sv_factorization
{
    /* Position in the needle at which (local period = period). */
    int64_t critical_pos;
    /* A distance in the needle such that two letters always coincide. */
    int64_t period_dist;
};

/* Avoid giving the user a chance to dereference null as much as posssible
   by returning this for various edgecases when it makes sense to communicate
   empty, null, invalid, not found etc. Used on cases by case basis.
   The function interfaces protect us from null pointers but not always. */
static str_view const nil = {.s = "", .sz = 0};

/* =========================   Prototypes   =============================== */

static size_t sv_after_find(str_view, str_view);
static size_t sv_before_rfind(str_view, str_view);
static size_t sv_min(size_t, size_t);
static sv_threeway_cmp sv_char_cmp(char, char);
static int64_t sv_ssizet_max(int64_t, int64_t);
static size_t sv_pos_memo(int64_t hay_sz, char const ARR_GEQ(, hay_sz),
                          int64_t needle_sz, char const ARR_GEQ(, needle_sz),
                          int64_t, int64_t);
static size_t sv_pos_normal(int64_t hay_sz, char const ARR_GEQ(, hay_sz),
                            int64_t needle_sz, char const ARR_GEQ(, needle_sz),
                            int64_t, int64_t);
static size_t sv_rpos_memo(int64_t hay_sz, char const ARR_GEQ(, hay_sz),
                           int64_t needle_sz, char const ARR_GEQ(, needle_sz),
                           int64_t, int64_t);
static size_t sv_rpos_normal(int64_t hay_sz, char const ARR_GEQ(, hay_sz),
                             int64_t needle_sz, char const ARR_GEQ(, needle_sz),
                             int64_t, int64_t);
static size_t sv_tw_match(int64_t hay_sz, char const ARR_GEQ(, hay_sz),
                          int64_t needle_sz, char const ARR_GEQ(, needle_sz));
static size_t sv_tw_rmatch(int64_t hay_sz, char const ARR_GEQ(, hay_sz),
                           int64_t needle_sz, char const ARR_GEQ(, needle_sz));
static struct sv_factorization
sv_maximal_suffix(int64_t needle_sz, char const ARR_GEQ(, needle_sz));
static struct sv_factorization
sv_maximal_suffix_rev(int64_t needle_sz, char const ARR_GEQ(, needle_sz));
static struct sv_factorization
sv_rmaximal_suffix(int64_t needle_sz, char const ARR_GEQ(, needle_sz));
static struct sv_factorization
sv_rmaximal_suffix_rev(int64_t needle_sz, char const ARR_GEQ(, needle_sz));
static size_t sv_twobyte_strnstrn(size_t hay_sz,
                                  unsigned char const ARR_GEQ(, hay_sz),
                                  size_t n_sz,
                                  unsigned char const ARR_GEQ(, n_sz));
static size_t sv_threebyte_strnstrn(size_t sz,
                                    unsigned char const ARR_GEQ(, sz),
                                    size_t n_sz,
                                    unsigned char const ARR_GEQ(, n_sz));
static size_t sv_fourbyte_strnstrn(size_t sz, unsigned char const ARR_GEQ(, sz),
                                   size_t n_sz,
                                   unsigned char const ARR_GEQ(, n_sz));
static size_t sv_strcspn(size_t str_sz, char const ARR_GEQ(, str_sz),
                         size_t set_sz, char const ARR_GEQ(, set_sz));
static size_t sv_strspn(size_t str_sz, char const ARR_GEQ(, str_sz),
                        size_t set_sz, char const ARR_GEQ(, set_sz));
static size_t sv_strnstrn(int64_t hay_sz, char const ARR_GEQ(, hay_sz),
                          int64_t needle_sz, char const ARR_GEQ(, needle_sz));
static size_t sv_strnchr(size_t n, char const ARR_GEQ(, n), char);
static size_t sv_rstrnchr(size_t n, char const ARR_GEQ(, n), char);
static size_t sv_rstrnstrn(int64_t hay_sz, char const ARR_GEQ(, hay_sz),
                           int64_t needle_sz, char const ARR_GEQ(, needle_sz));
static size_t sv_rtwobyte_strnstrn(size_t sz, unsigned char const ARR_GEQ(, sz),
                                   size_t n_sz,
                                   unsigned char const ARR_GEQ(, n_sz));
static size_t sv_rthreebyte_strnstrn(size_t sz,
                                     unsigned char const ARR_GEQ(, sz),
                                     size_t n_sz,
                                     unsigned char const ARR_GEQ(, n_sz));
static size_t sv_rfourbyte_strnstrn(size_t sz,
                                    unsigned char const ARR_GEQ(, sz),
                                    size_t n_sz,
                                    unsigned char const ARR_GEQ(, n_sz));

/* ===================   Interface Implementation   ====================== */

str_view
sv(char const ARR_GEQ(str, 1))
{
    return (str_view){.s = str, .sz = strlen(str)};
}

str_view
sv_n(size_t n, char const ARR_GEQ(str, 1))
{
    return (str_view){.s = str, .sz = strnlen(str, n)};
}

str_view
sv_delim(char const ARR_GEQ(str, 1), char const ARR_GEQ(delim, 1))
{
    return sv_begin_tok((str_view){.s = str, .sz = strlen(str)},
                        (str_view){.s = delim, .sz = strlen(delim)});
}

void
sv_print(FILE *f, str_view const sv)
{
    if (!f || !sv.s || nil.s == sv.s || !sv.sz)
    {
        return;
    }
    /* printf does not output the null terminator in normal strings so
       as long as we output correct number of characters we do the same */
    (void)fwrite(sv.s, sizeof(char), sv.sz, f);
}

str_view
sv_copy(size_t const str_sz, char const ARR_GEQ(src_str, 1))
{
    return sv_n(str_sz, src_str);
}

size_t
sv_fill(size_t const dest_sz, char ARR_CONST_GEQ(dest_buf, dest_sz),
        str_view const src)
{
    if (!dest_sz || !src.s || !src.sz)
    {
        return 0;
    }
    size_t const bytes = sv_min(dest_sz, sv_size(src));
    memmove(dest_buf, src.s, bytes);
    dest_buf[bytes - 1] = '\0';
    return bytes;
}

bool
sv_empty(str_view const sv)
{
    return !sv.s || !sv.sz;
}

size_t
sv_len(str_view const sv)
{
    return sv.sz;
}

size_t
sv_size(str_view const sv)
{
    return sv.sz + 1;
}

size_t
sv_strsize(char const ARR_GEQ(str, 1))
{
    return strlen(str) + 1;
}

size_t
sv_minlen(char const ARR_GEQ(str, 1), size_t n)
{
    return strnlen(str, n);
}

char
sv_at(str_view const sv, size_t const i)
{
    if (i >= sv.sz)
    {
        return *nil.s;
    }
    return sv.s[i];
}

char const *
sv_null(void)
{
    return nil.s;
}

void
sv_swap(str_view *const a, str_view *const b)
{
    if (a == b || !a || !b)
    {
        return;
    }
    str_view const tmp_b = (str_view){.s = b->s, .sz = b->sz};
    b->s = a->s;
    b->sz = a->sz;
    a->s = tmp_b.s;
    a->sz = tmp_b.sz;
}

sv_threeway_cmp
sv_cmp(str_view const lhs, str_view const rhs)
{
    if (!lhs.s || !rhs.s)
    {
        return SV_ERR;
    }
    size_t const sz = sv_min(lhs.sz, rhs.sz);
    size_t i = 0;
    for (; i < sz && lhs.s[i] == rhs.s[i]; ++i)
    {}
    if (i == lhs.sz && i == rhs.sz)
    {
        return SV_EQL;
    }
    if (i < lhs.sz && i < rhs.sz)
    {
        return (uint8_t)lhs.s[i] < (uint8_t)rhs.s[i] ? SV_LES : SV_GRT;
    }
    return (i < lhs.sz) ? SV_GRT : SV_LES;
}

sv_threeway_cmp
sv_strcmp(str_view const lhs, char const ARR_GEQ(rhs, 1))
{
    if (!lhs.s)
    {
        return SV_ERR;
    }
    size_t const sz = lhs.sz;
    size_t i = 0;
    for (; i < sz && rhs[i] && lhs.s[i] == rhs[i]; ++i)
    {}
    if (i == lhs.sz && !rhs[i])
    {
        return SV_EQL;
    }
    if (i < lhs.sz && rhs[i])
    {
        return (uint8_t)lhs.s[i] < (uint8_t)rhs[i] ? SV_LES : SV_GRT;
    }
    return (i < lhs.sz) ? SV_GRT : SV_LES;
}

sv_threeway_cmp
sv_strncmp(str_view const lhs, char const ARR_GEQ(rhs, 1), size_t const n)
{
    if (!lhs.s)
    {
        return SV_ERR;
    }
    size_t const sz = sv_min(lhs.sz, n);
    size_t i = 0;
    for (; i < sz && rhs[i] && lhs.s[i] == rhs[i]; ++i)
    {}
    if (i == lhs.sz && sz == n)
    {
        return SV_EQL;
    }
    /* strncmp compares the first at most n bytes inclusive */
    if (i < lhs.sz && sz <= n)
    {
        return (uint8_t)lhs.s[i] < (uint8_t)rhs[i] ? SV_LES : SV_GRT;
    }
    return (i < lhs.sz) ? SV_GRT : SV_LES;
}

char
sv_front(str_view const sv)
{
    if (!sv.s || !sv.sz)
    {
        return *nil.s;
    }
    return *sv.s;
}

char
sv_back(str_view const sv)
{
    if (!sv.s || !sv.sz)
    {
        return *nil.s;
    }
    return sv.s[sv.sz - 1];
}

char const *
sv_begin(str_view const sv)
{
    if (!sv.s)
    {
        return nil.s;
    }
    return sv.s;
}

char const *
sv_end(str_view const sv)
{
    if (!sv.s || sv.s == nil.s)
    {
        return nil.s;
    }
    return sv.s + sv.sz;
}

char const *
sv_next(char const ARR_GEQ(c, 1))
{
    return ++c;
}

char const *
sv_rbegin(str_view const sv)
{
    if (!sv.s)
    {
        return nil.s;
    }
    if (!sv.sz)
    {
        return sv.s;
    }
    return sv.s + sv.sz - 1;
}

char const *
sv_rend(str_view const sv)
{
    if (!sv.s || sv.s == nil.s)
    {
        return nil.s;
    }
    if (!sv.sz)
    {
        return sv.s;
    }
    return sv.s - 1;
}

char const *
sv_rnext(char const ARR_GEQ(c, 1))
{
    return --c;
}

char const *
sv_pos(str_view const sv, size_t const i)
{
    if (!sv.s)
    {
        return nil.s;
    }
    if (i > sv.sz)
    {
        return sv_end(sv);
    }
    return sv.s + i;
}

str_view
sv_begin_tok(str_view src, str_view const delim)
{
    if (!src.s)
    {
        return nil;
    }
    if (!delim.s)
    {
        return (str_view){.s = src.s + src.sz, 0};
    }
    char const *const begin = src.s;
    size_t const sv_not = sv_after_find(src, delim);
    src.s += sv_not;
    if (begin + src.sz == src.s)
    {
        return (str_view){.s = src.s, .sz = 0};
    }
    src.sz -= sv_not;
    return (str_view){.s = src.s, .sz = sv_find(src, 0, delim)};
}

bool
sv_end_tok(str_view const src, str_view const tok)
{
    return !tok.sz || tok.s >= (src.s + src.sz);
}

str_view
sv_next_tok(str_view const src, str_view const tok, str_view const delim)
{
    if (!tok.s)
    {
        return nil;
    }
    if (!delim.s || !tok.s || !tok.s[tok.sz])
    {
        return (str_view){.s = tok.s + tok.sz, .sz = 0};
    }
    str_view next = {.s = tok.s + tok.sz, .sz = src.sz - tok.sz};
    if (next.s >= src.s + src.sz)
    {
        return (str_view){.s = src.s + src.sz, .sz = 0};
    }
    next.s += delim.sz;
    next.sz = src.sz - (next.s - src.s);
    /* There is a cheap easy way to skip repeating delimiters before the
       next search that should be faster than string comparison. */
    size_t const after_delim = sv_after_find(next, delim);
    next.s += after_delim;
    next.sz -= after_delim;
    if (next.s >= src.s + src.sz)
    {
        return (str_view){.s = src.s + src.sz, .sz = 0};
    }
    size_t const found
        = sv_strnstrn((int64_t)next.sz, next.s, (int64_t)delim.sz, delim.s);
    return (str_view){.s = next.s, .sz = found};
}

str_view
sv_rbegin_tok(str_view src, str_view const delim)
{
    if (!src.s)
    {
        return nil;
    }
    if (!delim.s)
    {
        return (str_view){.s = src.s + src.sz, 0};
    }
    size_t before_delim = sv_before_rfind(src, delim);
    src.sz = sv_min(src.sz, before_delim + 1);
    size_t start = sv_rfind(src, src.sz, delim);
    if (start == src.sz)
    {
        return src;
    }
    start += delim.sz;
    return (str_view){.s = src.s + start, .sz = before_delim - start + 1};
}

str_view
sv_rnext_tok(str_view const src, str_view const tok, str_view const delim)
{
    if (!tok.s)
    {
        return nil;
    }
    if (!tok.sz | !delim.s || tok.s == src.s || tok.s - delim.sz <= src.s)
    {
        return (str_view){.s = src.s, .sz = 0};
    }
    str_view const shorter = {.s = src.s, .sz = (tok.s - delim.sz) - src.s};
    /* Same as in the forward version, this method is a quick way to skip
       any number of repeating delimiters before starting the next search
       for a delimiter before a token. */
    size_t const before_delim = sv_before_rfind(shorter, delim);
    if (before_delim == shorter.sz)
    {
        return shorter;
    }
    size_t start = sv_rstrnstrn((int64_t)before_delim, shorter.s,
                                (int64_t)delim.sz, delim.s);
    if (start == before_delim)
    {
        return (str_view){.s = shorter.s, .sz = before_delim + 1};
    }
    start += delim.sz;
    return (str_view){.s = src.s + start, .sz = before_delim - start + 1};
}

bool
sv_rend_tok(str_view const src, str_view const tok)
{
    return !tok.sz && tok.s == src.s;
}

str_view
sv_extend(str_view sv)
{
    if (!sv.s)
    {
        return nil;
    }
    char const *i = sv.s;
    while (*i++)
    {}
    sv.sz = i - sv.s - 1;
    return sv;
}

bool
sv_starts_with(str_view const sv, str_view const prefix)
{
    if (prefix.sz > sv.sz)
    {
        return false;
    }
    return sv_cmp(sv_substr(sv, 0, prefix.sz), prefix) == SV_EQL;
}

str_view
sv_remove_prefix(str_view const sv, size_t const n)
{
    size_t const remove = sv_min(sv.sz, n);
    return (str_view){.s = sv.s + remove, .sz = sv.sz - remove};
}

bool
sv_ends_with(str_view const sv, str_view const suffix)
{
    if (suffix.sz > sv.sz)
    {
        return false;
    }
    return sv_cmp(sv_substr(sv, sv.sz - suffix.sz, suffix.sz), suffix)
           == SV_EQL;
}

str_view
sv_remove_suffix(str_view const sv, size_t const n)
{
    if (!sv.s)
    {
        return nil;
    }
    return (str_view){.s = sv.s, .sz = sv.sz - sv_min(sv.sz, n)};
}

str_view
sv_substr(str_view const sv, size_t const pos, size_t const count)
{
    if (pos > sv.sz)
    {
        return (str_view){.s = sv.s + sv.sz, .sz = 0};
    }
    return (str_view){.s = sv.s + pos, .sz = sv_min(count, sv.sz - pos)};
}

bool
sv_contains(str_view const hay, str_view const needle)
{
    if (needle.sz > hay.sz)
    {
        return false;
    }
    if (sv_empty(hay))
    {
        return false;
    }
    if (sv_empty(needle))
    {
        return true;
    }
    return hay.sz
           != sv_strnstrn((int64_t)hay.sz, hay.s, (int64_t)needle.sz, needle.s);
}

str_view
sv_match(str_view const hay, str_view const needle)
{
    if (!hay.s || !needle.s)
    {
        return nil;
    }
    if (needle.sz > hay.sz || sv_empty(hay) || sv_empty(needle))
    {
        return (str_view){.s = hay.s + hay.sz, .sz = 0};
    }
    size_t const found
        = sv_strnstrn((int64_t)hay.sz, hay.s, (int64_t)needle.sz, needle.s);
    return found == hay.sz ? (str_view){.s = hay.s + hay.sz, .sz = 0}
                           : (str_view){.s = hay.s + found, .sz = needle.sz};
}

str_view
sv_rmatch(str_view const hay, str_view const needle)
{
    if (!hay.s)
    {
        return nil;
    }
    if (sv_empty(hay) || sv_empty(needle))
    {
        return (str_view){.s = hay.s + hay.sz, .sz = 0};
    }
    size_t const found
        = sv_rstrnstrn((int64_t)hay.sz, hay.s, (int64_t)needle.sz, needle.s);
    return found == hay.sz ? (str_view){.s = hay.s + hay.sz, .sz = 0}
                           : (str_view){.s = hay.s + found, .sz = needle.sz};
}

size_t
sv_find(str_view const hay, size_t const pos, str_view const needle)
{
    if (needle.sz > hay.sz || pos > hay.sz)
    {
        return hay.sz;
    }
    return pos
           + sv_strnstrn((int64_t)(hay.sz - pos), hay.s + pos,
                         (int64_t)needle.sz, needle.s);
}

size_t
sv_rfind(str_view const h, size_t pos, str_view const n)
{
    if (!h.sz || n.sz > h.sz)
    {
        return h.sz;
    }
    if (pos >= h.sz)
    {
        pos = h.sz - 1;
    }
    size_t const found
        = sv_rstrnstrn((int64_t)pos + 1, h.s, (int64_t)n.sz, n.s);
    return found == pos + 1 ? h.sz : found;
}

size_t
sv_find_first_of(str_view const hay, str_view const set)
{
    if (!hay.s || !hay.sz)
    {
        return 0;
    }
    if (!set.s || !set.sz)
    {
        return hay.sz;
    }
    return sv_strcspn(hay.sz, hay.s, set.sz, set.s);
}

size_t
sv_find_last_of(str_view const hay, str_view const set)
{
    if (!hay.s || !hay.sz)
    {
        return 0;
    }
    if (!set.s || !set.sz)
    {
        return hay.sz;
    }
    /* It may be tempting to go right to left but consider if that really
       would be reliably faster across every possible string one encounters.
       The last occurence of a set char could be anywhere in the string. */
    size_t last_pos = hay.sz;
    for (size_t in = 0, prev = 0;
         (in += sv_strspn(hay.sz - in, hay.s + in, set.sz, set.s)) != hay.sz;
         ++in, prev = in)
    {
        if (in != prev)
        {
            last_pos = in - 1;
        }
    }
    return last_pos;
}

size_t
sv_find_first_not_of(str_view const hay, str_view const set)
{
    if (!hay.s || !hay.sz)
    {
        return 0;
    }
    if (!set.s || !set.sz)
    {
        return 0;
    }
    return sv_strspn(hay.sz, hay.s, set.sz, set.s);
}

size_t
sv_find_last_not_of(str_view const hay, str_view const set)
{
    if (!hay.s || !hay.sz)
    {
        return 0;
    }
    if (!set.s || !set.sz)
    {
        return hay.sz - 1;
    }
    size_t last_pos = hay.sz;
    for (size_t in = 0, prev = 0;
         (in += sv_strspn(hay.sz - in, hay.s + in, set.sz, set.s)) != hay.sz;
         ++in, prev = in)
    {
        if (in != prev)
        {
            last_pos = in;
        }
    }
    return last_pos;
}

size_t
sv_npos(str_view const sv)
{
    return sv.sz;
}

/* ======================   Static Helpers    ============================= */

static size_t
sv_after_find(str_view const hay, str_view const needle)
{
    if (needle.sz > hay.sz)
    {
        return 0;
    }
    size_t delim_i = 0;
    size_t i = 0;
    for (; i < hay.sz && needle.s[delim_i] == hay.s[i];
         delim_i = (delim_i + 1) % needle.sz, ++i)
    {}
    /* Also reset to the last mismatch found. If some of the delimeter matched
       but then the string changed into a mismatch go back to get characters
       that are partially in the delimeter. */
    return i - delim_i;
}

static size_t
sv_before_rfind(str_view const hay, str_view const needle)
{
    if (needle.sz > hay.sz || !needle.sz || !hay.sz)
    {
        return hay.sz;
    }
    size_t delim_i = 0;
    size_t i = 0;
    for (; i < hay.sz
           && needle.s[needle.sz - delim_i - 1] == hay.s[hay.sz - i - 1];
         delim_i = (delim_i + 1) % needle.sz, ++i)
    {}
    /* Ugly logic to account for the reverse nature of this modulo search.
       the position needs to account for any part of the delim that may
       have started to match but then mismatched. The 1 is because
       this in an index being returned not a length. */
    return i == hay.sz ? hay.sz : hay.sz - i + delim_i - 1;
}

static inline size_t
sv_min(size_t const a, size_t const b)
{
    return a < b ? a : b;
}

static inline int64_t
sv_ssizet_max(int64_t const a, int64_t const b)
{
    return a > b ? a : b;
}

static inline sv_threeway_cmp
sv_char_cmp(char const a, char const b)
{
    return (a > b) - (a < b);
}

/* ======================   Static Utilities    =========================== */

/* This is section is modeled after the musl string.h library. However,
   using str_view that may not be null terminated requires modifications. */

#define BITOP(a, b, op)                                                        \
    ((a)[(size_t)(b) / (8 * sizeof *(a))] op(size_t) 1                         \
     << ((size_t)(b) % (8 * sizeof *(a))))

/* This is dangerous. Do not use this under normal circumstances.
   This is an internal helper for the backwards two way string
   searching algorithm. It expects that both arguments are
   greater than or equal to n bytes in length similar to how
   the forward version expects the same. However, the comparison
   moves backward from the location provided for n bytes. */
static int
sv_rmemcmp(void const *const vl, void const *const vr, size_t n)
{
    unsigned char const *l = vl;
    unsigned char const *r = vr;
    for (; n && *l == *r; n--, l--, r--)
    {}
    return n ? *l - *r : 0;
}

/* strspn is based on musl C-standard library implementation
   http://git.musl-libc.org/cgit/musl/tree/src/string/strcspn.c
   A custom implemenatation is necessary because C standard library impls
   have no concept of a string view and will continue searching beyond the
   end of a view until null is found. This way, string searches are
   efficient and only within the range specified. */
static size_t
sv_strcspn(size_t const str_sz, char const ARR_CONST_GEQ(str, str_sz),
           size_t const set_sz, char const ARR_GEQ(set, set_sz))
{
    if (!set_sz)
    {
        return str_sz;
    }
    char const *a = str;
    size_t byteset[32 / sizeof(size_t)];
    if (!set[1])
    {
        for (size_t i = 0; i < str_sz && *a && *a != *set; a++)
        {}
        return a - str;
    }
    memset(byteset, 0, sizeof byteset);
    for (size_t i = 0;
         i < set_sz && *set && BITOP(byteset, *(unsigned char *)set, |=);
         set++, ++i)
    {}
    for (size_t i = 0;
         i < str_sz && *a && !BITOP(byteset, *(unsigned char *)a, &); a++)
    {}
    return a - str;
}

/* strspn is based on musl C-standard library implementation
   https://git.musl-libc.org/cgit/musl/tree/src/string/strspn.c
   A custom implemenatation is necessary because C standard library impls
   have no concept of a string view and will continue searching beyond the
   end of a view until null is found. This way, string searches are
   efficient and only within the range specified. */
static size_t
sv_strspn(size_t const str_sz, char const ARR_CONST_GEQ(str, str_sz),
          size_t const set_sz, char const ARR_GEQ(set, set_sz))
{
    char const *a = str;
    size_t byteset[32 / sizeof(size_t)] = {0};
    if (!set[0])
    {
        return str_sz;
    }
    if (!set[1])
    {
        for (size_t i = 0; i < str_sz && i < set_sz && *a == *set; a++, ++i)
        {}
        return a - str;
    }
    for (size_t i = 0;
         i < set_sz && *set && BITOP(byteset, *(unsigned char *)set, |=);
         set++, ++i)
    {}
    for (size_t i = 0;
         i < str_sz && *a && BITOP(byteset, *(unsigned char *)a, &); a++, ++i)
    {}
    return a - str;
}

/* Providing strnstrn rather than strstr at the lowest level works better
   for string views where the string may not be null terminated. There needs
   to always be the additional constraint that a search cannot exceed the
   hay length. Returns 0 based index position at which needle begins in
   hay if it can be found, otherwise the hay size is returned. */
static size_t
sv_strnstrn(int64_t const hay_sz, char const ARR_CONST_GEQ(hay, hay_sz),
            int64_t const needle_sz,
            char const ARR_CONST_GEQ(needle, needle_sz))
{
    if (!hay_sz || !needle_sz || needle_sz > hay_sz)
    {
        return hay_sz;
    }
    if (1 == needle_sz)
    {
        return sv_strnchr(hay_sz, hay, *needle);
    }
    if (2 == needle_sz)
    {
        return sv_twobyte_strnstrn(hay_sz, (unsigned char *)hay, 2,
                                   (unsigned char *)needle);
    }
    if (3 == needle_sz)
    {
        return sv_threebyte_strnstrn(hay_sz, (unsigned char *)hay, 3,
                                     (unsigned char *)needle);
    }
    if (4 == needle_sz)
    {
        return sv_fourbyte_strnstrn(hay_sz, (unsigned char *)hay, 4,
                                    (unsigned char *)needle);
    }
    return sv_tw_match(hay_sz, hay, needle_sz, needle);
}

/* For now reverse logic for backwards searches has been separated into
   other functions. There is a possible formula to unit the reverse and
   forward logic into one set of functions, but the code is ugly. See
   the start of the reverse two-way algorithm for more. May unite if
   a clean way exists. */
static size_t
sv_rstrnstrn(int64_t const hay_sz, char const ARR_CONST_GEQ(hay, hay_sz),
             int64_t const needle_sz,
             char const ARR_CONST_GEQ(needle, needle_sz))
{
    if (!hay_sz || !needle_sz || needle_sz > hay_sz)
    {
        return hay_sz;
    }
    if (1 == needle_sz)
    {
        return sv_rstrnchr(hay_sz, hay, *needle);
    }
    if (2 == needle_sz)
    {
        return sv_rtwobyte_strnstrn(hay_sz, (unsigned char *)hay, 2,
                                    (unsigned char *)needle);
    }
    if (3 == needle_sz)
    {
        return sv_rthreebyte_strnstrn(hay_sz, (unsigned char *)hay, 3,
                                      (unsigned char *)needle);
    }
    if (4 == needle_sz)
    {
        return sv_rfourbyte_strnstrn(hay_sz, (unsigned char *)hay, 4,
                                     (unsigned char *)needle);
    }
    return sv_tw_rmatch(hay_sz, hay, needle_sz, needle);
}

/*==============   Post-Precomputation Two-Way Search    =================*/

/* NOLINTBEGIN(*easily-swappable*) */

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

   Assumes the needle size is shorter than hay size. Sizes are needed
   for string view operations because strings may not be null terminated
   and the string view library is most likely search a view rather than
   an entire string. Returns the position at which needle begins if found
   and the size of the hay stack if not found. */
static inline size_t
sv_tw_match(int64_t const hay_sz, char const ARR_CONST_GEQ(hay, hay_sz),
            int64_t const needle_sz,
            char const ARR_CONST_GEQ(needle, needle_sz))
{
    /* Preprocessing to get critical position and period distance. */
    struct sv_factorization const s = sv_maximal_suffix(needle_sz, needle);
    struct sv_factorization const r = sv_maximal_suffix_rev(needle_sz, needle);
    struct sv_factorization const w = (s.critical_pos > r.critical_pos) ? s : r;
    /* Determine if memoization is available due to found border/overlap. */
    if (!memcmp(needle, needle + w.period_dist, w.critical_pos + 1))
    {
        return sv_pos_memo(hay_sz, hay, needle_sz, needle, w.period_dist,
                           w.critical_pos);
    }
    return sv_pos_normal(hay_sz, hay, needle_sz, needle, w.period_dist,
                         w.critical_pos);
}

/* Two Way string matching algorithm adapted from ESMAJ
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260 */
static size_t
sv_pos_memo(int64_t const hay_sz, char const ARR_CONST_GEQ(hay, hay_sz),
            int64_t const needle_sz,
            char const ARR_CONST_GEQ(needle, needle_sz),
            int64_t const period_dist, int64_t const critical_pos)
{
    int64_t lpos = 0;
    int64_t rpos = 0;
    /* Eliminate worst case quadratic time complexity with memoization. */
    int64_t memoize_shift = -1;
    while (lpos <= hay_sz - needle_sz)
    {
        rpos = sv_ssizet_max(critical_pos, memoize_shift) + 1;
        while (rpos < needle_sz && needle[rpos] == hay[rpos + lpos])
        {
            ++rpos;
        }
        if (rpos < needle_sz)
        {
            lpos += (rpos - critical_pos);
            memoize_shift = -1;
            continue;
        }
        /* r_pos >= needle_sz */
        rpos = critical_pos;
        while (rpos > memoize_shift && needle[rpos] == hay[rpos + lpos])
        {
            --rpos;
        }
        if (rpos <= memoize_shift)
        {
            return lpos;
        }
        lpos += period_dist;
        /* Some prefix of needle coincides with the text. Memoize the length
           of this prefix to increase length of next shift, if possible. */
        memoize_shift = needle_sz - period_dist - 1;
    }
    return hay_sz;
}

/* Two Way string matching algorithm adapted from ESMAJ
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260 */
static size_t
sv_pos_normal(int64_t const hay_sz, char const ARR_CONST_GEQ(hay, hay_sz),
              int64_t const needle_sz,
              char const ARR_CONST_GEQ(needle, needle_sz), int64_t period_dist,
              int64_t const critical_pos)
{
    period_dist
        = sv_ssizet_max(critical_pos + 1, needle_sz - critical_pos - 1) + 1;
    int64_t lpos = 0;
    int64_t rpos = 0;
    while (lpos <= hay_sz - needle_sz)
    {
        rpos = critical_pos + 1;
        while (rpos < needle_sz && needle[rpos] == hay[rpos + lpos])
        {
            ++rpos;
        }
        if (rpos < needle_sz)
        {
            lpos += (rpos - critical_pos);
            continue;
        }
        /* r_pos >= needle_sz */
        rpos = critical_pos;
        while (rpos >= 0 && needle[rpos] == hay[rpos + lpos])
        {
            --rpos;
        }
        if (rpos < 0)
        {
            return lpos;
        }
        lpos += period_dist;
    }
    return hay_sz;
}

/* ==============   Suffix and Critical Factorization    =================*/

/* Computing of the maximal suffix. Adapted from ESMAJ.
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260 */
static inline struct sv_factorization
sv_maximal_suffix(int64_t const needle_sz,
                  char const ARR_CONST_GEQ(needle, needle_sz))
{
    int64_t suff_pos = -1;
    int64_t period = 1;
    int64_t last_rest = 0;
    int64_t rest = 1;
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
        default:
            break;
        }
    }
    return (struct sv_factorization){.critical_pos = suff_pos,
                                     .period_dist = period};
}

/* Computing of the maximal suffix reverse. Sometimes called tilde.
   adapted from ESMAJ
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260 */
static inline struct sv_factorization
sv_maximal_suffix_rev(int64_t const needle_sz,
                      char const ARR_CONST_GEQ(needle, needle_sz))
{
    int64_t suff_pos = -1;
    int64_t period = 1;
    int64_t last_rest = 0;
    int64_t rest = 1;
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
        default:
            break;
        }
    }
    return (struct sv_factorization){.critical_pos = suff_pos,
                                     .period_dist = period};
}

/*=======================  Right to Left Search  ===========================*/

/* Two way algorithm is easy to reverse. Instead of trying to reverse all
   logic in the factorizations and two way searches, leave the algorithms
   and calculate the values returned as offsets from the end of the string
   instead of indices starting from 0. It would be even be possible to unite
   these functions into one with the following formula
   (using the suffix calculation as an example):

        int64_t suff_pos = -1;
        int64_t period = 1;
        int64_t last_rest = 0;
        int64_t rest = 1;
        int64_t negate_sz = 0;
        int64_t negate_one = 0;
        if (direction == FORWARD)
        {
            negate_sz = needle_sz;
            negate_one = 1;
        }
        while (last_rest + rest < needle_sz)
        {
            switch (sv_char_cmp(
                needle[needle_sz
                        - (last_rest + rest) - 1 + negate_sz + negate_one],
                needle[needle_sz
                        - (suff_pos + rest) - 1 + negate_sz + negate_one]))
            {
            ...

   That would save the code repitition across all of the following
   functions but probably would make the code even harder to read and
   maintain. These algorithms are dense enough already so I think repetion
   is fine. Leaving comment here if that changes or an even better way comes
   along. */

/* Searches a string from right to left with a two-way algorithm. Returns
   the position of the start of the strig if found and string size if not. */
static inline size_t
sv_tw_rmatch(int64_t const hay_sz, char const ARR_CONST_GEQ(hay, hay_sz),
             int64_t const needle_sz,
             char const ARR_CONST_GEQ(needle, needle_sz))
{
    struct sv_factorization const s = sv_rmaximal_suffix(needle_sz, needle);
    struct sv_factorization const r = sv_rmaximal_suffix_rev(needle_sz, needle);
    struct sv_factorization const w = (s.critical_pos > r.critical_pos) ? s : r;
    if (!sv_rmemcmp(needle + needle_sz - 1,
                    needle + needle_sz - w.period_dist - 1, w.critical_pos + 1))
    {
        return sv_rpos_memo(hay_sz, hay, needle_sz, needle, w.period_dist,
                            w.critical_pos);
    }
    return sv_rpos_normal(hay_sz, hay, needle_sz, needle, w.period_dist,
                          w.critical_pos);
}

static size_t
sv_rpos_memo(int64_t const hay_sz, char const ARR_CONST_GEQ(hay, hay_sz),
             int64_t const needle_sz,
             char const ARR_CONST_GEQ(needle, needle_sz),
             int64_t const period_dist, int64_t const critical_pos)
{
    int64_t lpos = 0;
    int64_t rpos = 0;
    int64_t memoize_shift = -1;
    while (lpos <= hay_sz - needle_sz)
    {
        rpos = sv_ssizet_max(critical_pos, memoize_shift) + 1;
        while (rpos < needle_sz
               && needle[needle_sz - rpos - 1]
                      == hay[hay_sz - (rpos + lpos) - 1])
        {
            ++rpos;
        }
        if (rpos < needle_sz)
        {
            lpos += (rpos - critical_pos);
            memoize_shift = -1;
            continue;
        }
        /* r_pos >= needle_sz */
        rpos = critical_pos;
        while (rpos > memoize_shift
               && needle[needle_sz - rpos - 1]
                      == hay[hay_sz - (rpos + lpos) - 1])
        {
            --rpos;
        }
        if (rpos <= memoize_shift)
        {
            return hay_sz - lpos - needle_sz;
        }
        lpos += period_dist;
        /* Some prefix of needle coincides with the text. Memoize the length
           of this prefix to increase length of next shift, if possible. */
        memoize_shift = needle_sz - period_dist - 1;
    }
    return hay_sz;
}

static size_t
sv_rpos_normal(int64_t const hay_sz, char const ARR_CONST_GEQ(hay, hay_sz),
               int64_t const needle_sz,
               char const ARR_CONST_GEQ(needle, needle_sz), int64_t period_dist,
               int64_t const critical_pos)
{
    period_dist
        = sv_ssizet_max(critical_pos + 1, needle_sz - critical_pos - 1) + 1;
    int64_t lpos = 0;
    int64_t rpos = 0;
    while (lpos <= hay_sz - needle_sz)
    {
        rpos = critical_pos + 1;
        while (rpos < needle_sz
               && (needle[needle_sz - rpos - 1]
                   == hay[hay_sz - (rpos + lpos) - 1]))
        {
            ++rpos;
        }
        if (rpos < needle_sz)
        {
            lpos += (rpos - critical_pos);
            continue;
        }
        /* r_pos >= needle_sz */
        rpos = critical_pos;
        while (rpos >= 0
               && needle[needle_sz - rpos - 1]
                      == hay[hay_sz - (rpos + lpos) - 1])
        {
            --rpos;
        }
        if (rpos < 0)
        {
            return hay_sz - lpos - needle_sz;
        }
        lpos += period_dist;
    }
    return hay_sz;
}

/* NOLINTEND(*easily-swappable*) */

static inline struct sv_factorization
sv_rmaximal_suffix(int64_t const needle_sz,
                   char const ARR_CONST_GEQ(needle, needle_sz))
{
    int64_t suff_pos = -1;
    int64_t period = 1;
    int64_t last_rest = 0;
    int64_t rest = 1;
    while (last_rest + rest < needle_sz)
    {
        switch (sv_char_cmp(needle[needle_sz - (last_rest + rest) - 1],
                            needle[needle_sz - (suff_pos + rest) - 1]))
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
        default:
            break;
        }
    }
    return (struct sv_factorization){.critical_pos = suff_pos,
                                     .period_dist = period};
}

static inline struct sv_factorization
sv_rmaximal_suffix_rev(int64_t const needle_sz,
                       char const ARR_CONST_GEQ(needle, needle_sz))
{
    int64_t suff_pos = -1;
    int64_t period = 1;
    int64_t last_rest = 0;
    int64_t rest = 1;
    while (last_rest + rest < needle_sz)
    {
        switch (sv_char_cmp(needle[needle_sz - (last_rest + rest) - 1],
                            needle[needle_sz - (suff_pos + rest) - 1]))
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
        default:
            break;
        }
    }
    return (struct sv_factorization){.critical_pos = suff_pos,
                                     .period_dist = period};
}

/* ======================   Brute Force Search    ==========================
 */

/* All brute force searches adapted from musl C library.
   http://git.musl-libc.org/cgit/musl/tree/src/string/strstr.c
   They must stop the search at hay size and therefore required slight
   modification because string views may not be null terminated. Reverse
   methods are my own additions provided to support a compliant reverse
   search for rfind which most string libraries specify must search right
   to left. Also having a reverse tokenizer is convenient and also relies
   on right to left brute force searches. */
static inline size_t
sv_strnchr(size_t n, char const ARR_GEQ(s, n), char const c)
{
    size_t i = 0;
    for (; n && *s != c; s++, --n, ++i)
    {}
    return i;
}

static inline size_t
sv_rstrnchr(size_t const n, char const ARR_CONST_GEQ(s, n), char const c)
{
    char const *x = s + n - 1;
    size_t i = n;
    for (; i && *x != c; x--, --i)
    {}
    return i ? i - 1 : n;
}

static inline size_t
sv_twobyte_strnstrn(size_t const sz, unsigned char const ARR_GEQ(h, sz),
                    size_t const n_sz,
                    unsigned char const ARR_CONST_GEQ(n, n_sz))
{
    uint16_t nw = n[0] << 8 | n[1];
    uint16_t hw = h[0] << 8 | h[1];
    size_t i = 1;
    for (h++; i < sz && *h && hw != nw; hw = (hw << 8) | *++h, ++i)
    {}
    return (i < sz) ? i - 1 : sz;
}

static inline size_t
sv_rtwobyte_strnstrn(size_t const sz, unsigned char const ARR_GEQ(h, sz),
                     size_t const n_sz,
                     unsigned char const ARR_CONST_GEQ(n, n_sz))
{
    h = h + sz - 2;
    uint16_t nw = n[0] << 8 | n[1];
    uint16_t hw = h[0] << 8 | h[1];
    size_t i = sz - 1;
    /* The search is right to left therefore the Most Significant Byte will
       be the leading character of the string and the previous leading
       character is shifted to the right. */
    for (; i && hw != nw; hw = (hw >> 8) | (*--h << 8), --i)
    {}
    return i ? i - 1 : sz;
}

static inline size_t
sv_threebyte_strnstrn(size_t const sz, unsigned char const ARR_GEQ(h, sz),
                      size_t const n_sz,
                      unsigned char const ARR_CONST_GEQ(n, n_sz))
{
    uint32_t nw = (uint32_t)n[0] << 24 | n[1] << 16 | n[2] << 8;
    uint32_t hw = (uint32_t)h[0] << 24 | h[1] << 16 | h[2] << 8;
    size_t i = 2;
    for (h += 2; i < sz && *h && hw != nw; hw = (hw | *++h) << 8, ++i)
    {}
    return (i < sz) ? i - 2 : sz;
}

static inline size_t
sv_rthreebyte_strnstrn(size_t const sz, unsigned char const ARR_GEQ(h, sz),
                       size_t const n_sz,
                       unsigned char const ARR_CONST_GEQ(n, n_sz))
{
    h = h + sz - 3;
    uint32_t nw = (uint32_t)n[0] << 16 | n[1] << 8 | n[2];
    uint32_t hw = (uint32_t)h[0] << 16 | h[1] << 8 | h[2];
    size_t i = sz - 2;
    /* Align the bits with fewer left shifts such that as the parsing
       progresses right left, the leading character always takes highest
       bit position and there is no need for any masking. */
    for (; i && hw != nw; hw = (hw >> 8) | (*--h << 16), --i)
    {}
    return i ? i - 1 : sz;
}

static inline size_t
sv_fourbyte_strnstrn(size_t const sz, unsigned char const ARR_GEQ(h, sz),
                     size_t const n_sz,
                     unsigned char const ARR_CONST_GEQ(n, n_sz))
{
    uint32_t nw = (uint32_t)n[0] << 24 | n[1] << 16 | n[2] << 8 | n[3];
    uint32_t hw = (uint32_t)h[0] << 24 | h[1] << 16 | h[2] << 8 | h[3];
    size_t i = 3;
    for (h += 3; i < sz && *h && hw != nw; hw = (hw << 8) | *++h, ++i)
    {}
    return (i < sz) ? i - 3 : sz;
}

static inline size_t
sv_rfourbyte_strnstrn(size_t const sz, unsigned char const ARR_GEQ(h, sz),
                      size_t const n_sz,
                      unsigned char const ARR_CONST_GEQ(n, n_sz))
{
    h = h + sz - 4;
    uint32_t nw = (uint32_t)n[0] << 24 | n[1] << 16 | n[2] << 8 | n[3];
    uint32_t hw = (uint32_t)h[0] << 24 | h[1] << 16 | h[2] << 8 | h[3];
    size_t i = sz - 3;
    /* Now that all four bytes of the unsigned int are used the shifting
       becomes more intuitive. The window slides left to right and the
       next leading character takes the high bit position. */
    for (; i && hw != nw; hw = (hw >> 8) | (*--h << 24), --i)
    {}
    return i ? i - 1 : sz;
}
