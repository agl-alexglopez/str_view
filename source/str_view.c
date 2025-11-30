/* Author: Alexander G. Lopez
   ==========================
   This file implements the SV_Str_view interface as an interpretation of C++
   string_view type. There are some minor differences and C flavor thrown
   in. Additionally, there is a provided reimplementation of the Two-Way
   String-Searching algorithm, similar to glibc. */
#include "str_view.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Clang and GCC support static array parameter declarations while
   MSVC does not. This is how to solve the differing declaration
   signature requirements. */
#if defined(__GNUC__) || defined(__clang__) || defined(__INTEL_LLVM_COMPILER)
/* A static array parameter declaration helper. Function parameters
   may specify an array of a type of at least SIZE elements large,
   allowing compiler optimizations and safety errors. Specify
   a parameter such as `void func(int size, ARR_GEQ(arr, size))`. */
#    define ARR_GEQ(str, size) str[static(size)]
/* A static array parameter declaration helper. Function parameters
   may specify an array of a type of at least SIZE elements large,
   allowing compiler optimizations and safety errors. Specify
   a parameter such as `void func(int size, int ARR_GEQ(arr,size))`.
   This declarations adds the additional constraint that the pointer
   to the beginning of the array of types will not move. */
#    define ARR_CONST_GEQ(str, size) str[static const(size)]
#else
/* Dummy macro for MSVC compatibility. Specifies a function parameter shall
   have at least one element. Compiler warnings may differ from GCC/Clang. */
#    define ARR_GEQ(str, size) *str
/* Dummy macro for MSVC compatibility. Specifies a function parameter shall
   have at least one element. MSVC does not allow specification of a const
   pointer to the beginning of an array function parameter when using array
   size parameter syntax. Compiler warnings may differ from GCC/Clang. */
#    define ARR_CONST_GEQ(str, size) *const str
#endif

/* ========================   Type Definitions   =========================== */

/* Return the factorization step of two-way search in precompute phase. */
struct Factorization
{
    /* Position in the needle at which (local period = period). */
    ptrdiff_t critical_pos;
    /* A distance in the needle such that two letters always coincide. */
    ptrdiff_t period_dist;
};

/* Avoid giving the user a chance to dereference null as much as possible
   by returning this for various edge cases when it makes sense to communicate
   empty, null, invalid, not found etc. Used on cases by case basis.
   The function interfaces protect us from null pointers but not always. */
static SV_Str_view const nil = {
    .s = "",
    .len = 0,
};

/* =========================   Prototypes   =============================== */

static size_t after_find(SV_Str_view, SV_Str_view);
static size_t before_r_find(SV_Str_view, SV_Str_view);
static size_t min(size_t, size_t);
static SV_Order char_cmp(char, char);
static ptrdiff_t signed_max(ptrdiff_t, ptrdiff_t);

/* Once the user facing API has verified the lengths of strings provided to
   views as inputs, internal code can take advantage of compiler optimizations
   by assuming the strings are GREATER than or EQUAL TO certain lengths
   allowing for processing by larger units than 1 in compiled code. */

static size_t pos_memo(ptrdiff_t hay_sz, char const ARR_GEQ(, hay_sz),
                       ptrdiff_t needle_sz, char const ARR_GEQ(, needle_sz),
                       ptrdiff_t, ptrdiff_t);
static size_t pos_normal(ptrdiff_t hay_sz, char const ARR_GEQ(, hay_sz),
                         ptrdiff_t needle_sz, char const ARR_GEQ(, needle_sz),
                         ptrdiff_t, ptrdiff_t);
static size_t r_pos_memo(ptrdiff_t hay_sz, char const ARR_GEQ(, hay_sz),
                         ptrdiff_t needle_sz, char const ARR_GEQ(, needle_sz),
                         ptrdiff_t, ptrdiff_t);
static size_t r_pos_normal(ptrdiff_t hay_sz, char const ARR_GEQ(, hay_sz),
                           ptrdiff_t needle_sz, char const ARR_GEQ(, needle_sz),
                           ptrdiff_t, ptrdiff_t);
static size_t tw_match(ptrdiff_t hay_sz, char const ARR_GEQ(, hay_sz),
                       ptrdiff_t needle_sz, char const ARR_GEQ(, needle_sz));
static size_t tw_rmatch(ptrdiff_t hay_sz, char const ARR_GEQ(, hay_sz),
                        ptrdiff_t needle_sz, char const ARR_GEQ(, needle_sz));
static struct Factorization maximal_suffix(ptrdiff_t needle_sz,
                                           char const ARR_GEQ(, needle_sz));
static struct Factorization maximal_suffix_rev(ptrdiff_t needle_sz,
                                               char const ARR_GEQ(, needle_sz));
static struct Factorization r_maximal_suffix(ptrdiff_t needle_sz,
                                             char const ARR_GEQ(, needle_sz));
static struct Factorization
r_maximal_suffix_rev(ptrdiff_t needle_sz, char const ARR_GEQ(, needle_sz));
static size_t two_byte_view_n_view_n(size_t hay_sz,
                                     unsigned char const ARR_GEQ(, hay_sz),
                                     size_t n_sz,
                                     unsigned char const ARR_GEQ(, n_sz));
static size_t three_byte_view_n_view_n(size_t sz,
                                       unsigned char const ARR_GEQ(, sz),
                                       size_t n_sz,
                                       unsigned char const ARR_GEQ(, n_sz));
static size_t four_byte_view_n_view_n(size_t sz,
                                      unsigned char const ARR_GEQ(, sz),
                                      size_t n_sz,
                                      unsigned char const ARR_GEQ(, n_sz));
static size_t view_c_spn(size_t str_sz, char const ARR_GEQ(, str_sz),
                         size_t set_sz, char const ARR_GEQ(, set_sz));
static size_t view_r_spn(size_t str_sz, char const ARR_GEQ(, str_sz),
                         size_t set_sz, char const ARR_GEQ(, set_sz));
static size_t view_n_view_n(ptrdiff_t hay_sz, char const ARR_GEQ(, hay_sz),
                            ptrdiff_t needle_sz,
                            char const ARR_GEQ(, needle_sz));
static size_t view_n_chr(size_t n, char const ARR_GEQ(, n), char);
static size_t r_view_n_chr(size_t n, char const ARR_GEQ(, n), char);
static size_t r_view_n_view_n(ptrdiff_t hay_sz, char const ARR_GEQ(, hay_sz),
                              ptrdiff_t needle_sz,
                              char const ARR_GEQ(, needle_sz));
static size_t r_two_byte_view_n_view_n(size_t sz,
                                       unsigned char const ARR_GEQ(, sz),
                                       size_t n_sz,
                                       unsigned char const ARR_GEQ(, n_sz));
static size_t r_three_byte_view_n_view_n(size_t sz,
                                         unsigned char const ARR_GEQ(, sz),
                                         size_t n_sz,
                                         unsigned char const ARR_GEQ(, n_sz));
static size_t r_four_byte_view_n_view_n(size_t sz,
                                        unsigned char const ARR_GEQ(, sz),
                                        size_t n_sz,
                                        unsigned char const ARR_GEQ(, n_sz));

/* ===================   Interface Implementation   ====================== */

SV_Str_view
SV_from_terminated(char const *const str)
{
    if (!str)
    {
        return nil;
    }
    return (SV_Str_view){
        .s = str,
        .len = strlen(str),
    };
}

SV_Str_view
SV_from_view(size_t n, char const *const str)
{
    if (!str)
    {
        return nil;
    }
    return (SV_Str_view){
        .s = str,
        .len = strnlen(str, n),
    };
}

SV_Str_view
SV_from_delimiter(char const *const str, char const *const delim)
{
    if (!str)
    {
        return nil;
    }
    if (!delim)
    {
        return (SV_Str_view){
            .s = str,
            .len = strlen(str),
        };
    }
    return SV_begin_token(
        (SV_Str_view){
            .s = str,
            .len = strlen(str),
        },
        (SV_Str_view){
            .s = delim,
            .len = strlen(delim),
        });
}

SV_Str_view
SV_copy(size_t const str_sz, char const *const src_str)
{
    return SV_from_view(str_sz, src_str);
}

size_t
SV_fill(size_t const dest_sz, char *const dest_buf, SV_Str_view const src)
{
    if (!dest_buf || !dest_sz || !src.s || !src.len)
    {
        return 0;
    }
    size_t const bytes = min(dest_sz, SV_bytes(src));
    memmove(dest_buf, src.s, bytes);
    dest_buf[bytes - 1] = '\0';
    return bytes;
}

bool
SV_is_empty(SV_Str_view const sv)
{
    return !sv.len;
}

size_t
SV_len(SV_Str_view const sv)
{
    return sv.len;
}

size_t
SV_bytes(SV_Str_view const sv)
{
    return sv.len + 1;
}

size_t
SV_str_bytes(char const *const str)
{
    if (!str)
    {
        return 0;
    }
    return strlen(str) + 1;
}

size_t
SV_min_len(char const *const str, size_t n)
{
    return strnlen(str, n);
}

char
SV_at(SV_Str_view const sv, size_t const i)
{
    if (i >= sv.len)
    {
        return *nil.s;
    }
    return sv.s[i];
}

char const *
SV_null(void)
{
    return nil.s;
}

void
SV_swap(SV_Str_view *const a, SV_Str_view *const b)
{
    if (a == b || !a || !b)
    {
        return;
    }
    SV_Str_view const tmp_b = (SV_Str_view){
        .s = b->s,
        .len = b->len,
    };
    b->s = a->s;
    b->len = a->len;
    a->s = tmp_b.s;
    a->len = tmp_b.len;
}

SV_Order
SV_compare(SV_Str_view const lhs, SV_Str_view const rhs)
{
    if (!lhs.s || !rhs.s)
    {
        return SV_ORDER_ERROR;
    }
    size_t const sz = min(lhs.len, rhs.len);
    size_t i = 0;
    for (; i < sz && lhs.s[i] == rhs.s[i]; ++i)
    {}
    if (i == lhs.len && i == rhs.len)
    {
        return SV_ORDER_EQUAL;
    }
    if (i < lhs.len && i < rhs.len)
    {
        return (uint8_t)lhs.s[i] < (uint8_t)rhs.s[i] ? SV_ORDER_LESSER
                                                     : SV_ORDER_GREATER;
    }
    return (i < lhs.len) ? SV_ORDER_GREATER : SV_ORDER_LESSER;
}

SV_Order
SV_terminated_compare(SV_Str_view const lhs, char const *const rhs)
{
    if (!lhs.s || !rhs)
    {
        return SV_ORDER_ERROR;
    }
    size_t const sz = lhs.len;
    size_t i = 0;
    for (; i < sz && rhs[i] && lhs.s[i] == rhs[i]; ++i)
    {}
    if (i == lhs.len && !rhs[i])
    {
        return SV_ORDER_EQUAL;
    }
    if (i < lhs.len && rhs[i])
    {
        return (uint8_t)lhs.s[i] < (uint8_t)rhs[i] ? SV_ORDER_LESSER
                                                   : SV_ORDER_GREATER;
    }
    return (i < lhs.len) ? SV_ORDER_GREATER : SV_ORDER_LESSER;
}

SV_Order
SV_view_compare(SV_Str_view const lhs, char const *const rhs, size_t const n)
{
    if (!lhs.s || !rhs)
    {
        return SV_ORDER_ERROR;
    }
    size_t const sz = min(lhs.len, n);
    size_t i = 0;
    for (; i < sz && rhs[i] && lhs.s[i] == rhs[i]; ++i)
    {}
    if (i == lhs.len && sz == n)
    {
        return SV_ORDER_EQUAL;
    }
    /* strncmp compares the first at most n bytes inclusive */
    if (i < lhs.len && sz <= n)
    {
        return (uint8_t)lhs.s[i] < (uint8_t)rhs[i] ? SV_ORDER_LESSER
                                                   : SV_ORDER_GREATER;
    }
    return (i < lhs.len) ? SV_ORDER_GREATER : SV_ORDER_LESSER;
}

char
SV_front(SV_Str_view const sv)
{
    if (!sv.s || !sv.len)
    {
        return *nil.s;
    }
    return *sv.s;
}

char
SV_back(SV_Str_view const sv)
{
    if (!sv.s || !sv.len)
    {
        return *nil.s;
    }
    return sv.s[sv.len - 1];
}

char const *
SV_begin(SV_Str_view const sv)
{
    if (!sv.s)
    {
        return nil.s;
    }
    return sv.s;
}

char const *
SV_end(SV_Str_view const sv)
{
    if (!sv.s || sv.s == nil.s)
    {
        return nil.s;
    }
    return sv.s + sv.len;
}

char const *
SV_next(char const *c)
{
    if (!c)
    {
        return nil.s;
    }
    return ++c;
}

char const *
SV_reverse_begin(SV_Str_view const sv)
{
    if (!sv.s)
    {
        return nil.s;
    }
    if (!sv.len)
    {
        return sv.s;
    }
    return sv.s + sv.len - 1;
}

char const *
SV_reverse_end(SV_Str_view const sv)
{
    if (!sv.s || sv.s == nil.s)
    {
        return nil.s;
    }
    if (!sv.len)
    {
        return sv.s;
    }
    return sv.s - 1;
}

char const *
SV_reverse_next(char const *c)
{
    if (!c)
    {
        return nil.s;
    }
    return --c;
}

char const *
SV_pointer(SV_Str_view const sv, size_t const i)
{
    if (!sv.s)
    {
        return nil.s;
    }
    if (i > sv.len)
    {
        return SV_end(sv);
    }
    return sv.s + i;
}

SV_Str_view
SV_begin_token(SV_Str_view src, SV_Str_view const delim)
{
    if (!src.s)
    {
        return nil;
    }
    if (!delim.s)
    {
        return (SV_Str_view){.s = src.s + src.len, 0};
    }
    char const *const begin = src.s;
    size_t const sv_not = after_find(src, delim);
    src.s += sv_not;
    if (begin + src.len == src.s)
    {
        return (SV_Str_view){
            .s = src.s,
            .len = 0,
        };
    }
    src.len -= sv_not;
    return (SV_Str_view){
        .s = src.s,
        .len = SV_find(src, 0, delim),
    };
}

bool
SV_end_token(SV_Str_view const src, SV_Str_view const tok)
{
    return !tok.len || tok.s >= (src.s + src.len);
}

SV_Str_view
SV_next_token(SV_Str_view const src, SV_Str_view const tok,
              SV_Str_view const delim)
{
    if (!tok.s)
    {
        return nil;
    }
    if (!delim.s || !tok.s || !tok.s[tok.len])
    {
        return (SV_Str_view){
            .s = &tok.s[tok.len],
            .len = 0,
        };
    }
    SV_Str_view next = {
        .s = &tok.s[tok.len] + delim.len,
    };
    if (next.s >= &src.s[src.len])
    {
        return (SV_Str_view){
            .s = &src.s[src.len],
            .len = 0,
        };
    }
    next.len = &src.s[src.len] - next.s;
    /* There is a cheap easy way to skip repeating delimiters before the
       next search that should be faster than string comparison. */
    size_t const after_delim = after_find(next, delim);
    next.s += after_delim;
    next.len -= after_delim;
    if (next.s >= &src.s[src.len])
    {
        return (SV_Str_view){
            .s = &src.s[src.len],
            .len = 0,
        };
    }
    size_t const found = view_n_view_n((ptrdiff_t)next.len, next.s,
                                       (ptrdiff_t)delim.len, delim.s);
    return (SV_Str_view){
        .s = next.s,
        .len = found,
    };
}

SV_Str_view
SV_reverse_begin_token(SV_Str_view src, SV_Str_view const delim)
{
    if (!src.s)
    {
        return nil;
    }
    if (!delim.s)
    {
        return (SV_Str_view){
            .s = src.s + src.len,
            0,
        };
    }
    size_t before_delim = before_r_find(src, delim);
    src.len = min(src.len, before_delim + 1);
    size_t start = SV_reverse_find(src, src.len, delim);
    if (start == src.len)
    {
        return src;
    }
    start += delim.len;
    return (SV_Str_view){
        .s = src.s + start,
        .len = before_delim - start + 1,
    };
}

SV_Str_view
SV_reverse_next_token(SV_Str_view const src, SV_Str_view const tok,
                      SV_Str_view const delim)
{
    if (!tok.s)
    {
        return nil;
    }
    if (!tok.len | !delim.s || tok.s == src.s || tok.s - delim.len <= src.s)
    {
        return (SV_Str_view){
            .s = src.s,
            .len = 0,
        };
    }
    SV_Str_view const shorter = {
        .s = src.s,
        .len = (tok.s - delim.len) - src.s,
    };
    /* Same as in the forward version, this method is a quick way to skip
       any number of repeating delimiters before starting the next search
       for a delimiter before a token. */
    size_t const before_delim = before_r_find(shorter, delim);
    if (before_delim == shorter.len)
    {
        return shorter;
    }
    size_t start = r_view_n_view_n((ptrdiff_t)before_delim, shorter.s,
                                   (ptrdiff_t)delim.len, delim.s);
    if (start == before_delim)
    {
        return (SV_Str_view){
            .s = shorter.s,
            .len = before_delim + 1,
        };
    }
    start += delim.len;
    return (SV_Str_view){
        .s = src.s + start,
        .len = before_delim - start + 1,
    };
}

bool
SV_reverse_end_token(SV_Str_view const src, SV_Str_view const tok)
{
    return !tok.len && tok.s == src.s;
}

SV_Str_view
SV_extend(SV_Str_view sv)
{
    if (!sv.s)
    {
        return nil;
    }
    char const *i = sv.s;
    while (*i++)
    {}
    sv.len = i - sv.s - 1;
    return sv;
}

bool
SV_starts_with(SV_Str_view const sv, SV_Str_view const prefix)
{
    if (prefix.len > sv.len)
    {
        return false;
    }
    return SV_compare(SV_substr(sv, 0, prefix.len), prefix) == SV_ORDER_EQUAL;
}

SV_Str_view
SV_remove_prefix(SV_Str_view const sv, size_t const n)
{
    size_t const remove = min(sv.len, n);
    return (SV_Str_view){
        .s = sv.s + remove,
        .len = sv.len - remove,
    };
}

bool
SV_ends_with(SV_Str_view const sv, SV_Str_view const suffix)
{
    if (suffix.len > sv.len)
    {
        return false;
    }
    return SV_compare(SV_substr(sv, sv.len - suffix.len, suffix.len), suffix)
           == SV_ORDER_EQUAL;
}

SV_Str_view
SV_remove_suffix(SV_Str_view const sv, size_t const n)
{
    if (!sv.s)
    {
        return nil;
    }
    return (SV_Str_view){
        .s = sv.s,
        .len = sv.len - min(sv.len, n),
    };
}

SV_Str_view
SV_substr(SV_Str_view const sv, size_t const pos, size_t const count)
{
    if (pos > sv.len)
    {
        return (SV_Str_view){
            .s = sv.s + sv.len,
            .len = 0,
        };
    }
    return (SV_Str_view){
        .s = sv.s + pos,
        .len = min(count, sv.len - pos),
    };
}

bool
SV_contains(SV_Str_view const hay, SV_Str_view const needle)
{
    if (needle.len > hay.len)
    {
        return false;
    }
    if (SV_is_empty(hay))
    {
        return false;
    }
    if (SV_is_empty(needle))
    {
        return true;
    }
    return hay.len
           != view_n_view_n((ptrdiff_t)hay.len, hay.s, (ptrdiff_t)needle.len,
                            needle.s);
}

SV_Str_view
SV_match(SV_Str_view const hay, SV_Str_view const needle)
{
    if (!hay.s || !needle.s)
    {
        return nil;
    }
    if (needle.len > hay.len || SV_is_empty(hay) || SV_is_empty(needle))
    {
        return (SV_Str_view){
            .s = hay.s + hay.len,
            .len = 0,
        };
    }
    size_t const found = view_n_view_n((ptrdiff_t)hay.len, hay.s,
                                       (ptrdiff_t)needle.len, needle.s);
    if (found == hay.len)
    {
        return (SV_Str_view){
            .s = hay.s + hay.len,
            .len = 0,
        };
    }
    return (SV_Str_view){
        .s = hay.s + found,
        .len = needle.len,
    };
}

SV_Str_view
SV_reverse_match(SV_Str_view const hay, SV_Str_view const needle)
{
    if (!hay.s)
    {
        return nil;
    }
    if (SV_is_empty(hay) || SV_is_empty(needle))
    {
        return (SV_Str_view){
            .s = hay.s + hay.len,
            .len = 0,
        };
    }
    size_t const found = r_view_n_view_n((ptrdiff_t)hay.len, hay.s,
                                         (ptrdiff_t)needle.len, needle.s);
    if (found == hay.len)
    {
        return (SV_Str_view){.s = hay.s + hay.len, .len = 0};
    }
    return (SV_Str_view){
        .s = hay.s + found,
        .len = needle.len,
    };
}

size_t
SV_find(SV_Str_view const hay, size_t const pos, SV_Str_view const needle)
{
    if (needle.len > hay.len || pos > hay.len)
    {
        return hay.len;
    }
    return pos
           + view_n_view_n((ptrdiff_t)(hay.len - pos), hay.s + pos,
                           (ptrdiff_t)needle.len, needle.s);
}

size_t
SV_reverse_find(SV_Str_view const h, size_t pos, SV_Str_view const n)
{
    if (!h.len || n.len > h.len)
    {
        return h.len;
    }
    if (pos >= h.len)
    {
        pos = h.len - 1;
    }
    size_t const found
        = r_view_n_view_n((ptrdiff_t)pos + 1, h.s, (ptrdiff_t)n.len, n.s);
    return found == pos + 1 ? h.len : found;
}

size_t
SV_find_first_of(SV_Str_view const hay, SV_Str_view const set)
{
    if (!hay.s || !hay.len)
    {
        return 0;
    }
    if (!set.s || !set.len)
    {
        return hay.len;
    }
    return view_c_spn(hay.len, hay.s, set.len, set.s);
}

size_t
SV_find_last_of(SV_Str_view const hay, SV_Str_view const set)
{
    if (!hay.s || !hay.len)
    {
        return 0;
    }
    if (!set.s || !set.len)
    {
        return hay.len;
    }
    /* It may be tempting to go right to left but consider if that really
       would be reliably faster across every possible string one encounters.
       The last occurence of a set char could be anywhere in the string. */
    size_t last_pos = hay.len;
    for (size_t in = 0, prev = 0;
         (in += view_r_spn(hay.len - in, hay.s + in, set.len, set.s))
         != hay.len;
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
SV_find_first_not_of(SV_Str_view const hay, SV_Str_view const set)
{
    if (!hay.s || !hay.len)
    {
        return 0;
    }
    if (!set.s || !set.len)
    {
        return 0;
    }
    return view_r_spn(hay.len, hay.s, set.len, set.s);
}

size_t
SV_find_last_not_of(SV_Str_view const hay, SV_Str_view const set)
{
    if (!hay.s || !hay.len)
    {
        return 0;
    }
    if (!set.s || !set.len)
    {
        return hay.len - 1;
    }
    size_t last_pos = hay.len;
    for (size_t in = 0, prev = 0;
         (in += view_r_spn(hay.len - in, hay.s + in, set.len, set.s))
         != hay.len;
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
SV_npos(SV_Str_view const sv)
{
    return sv.len;
}

/* ======================   Static Helpers    ============================= */

static size_t
after_find(SV_Str_view const hay, SV_Str_view const needle)
{
    if (needle.len > hay.len)
    {
        return 0;
    }
    size_t delim_i = 0;
    size_t i = 0;
    for (; i < hay.len && needle.s[delim_i] == hay.s[i];
         delim_i = (delim_i + 1) % needle.len, ++i)
    {}
    /* Also reset to the last mismatch found. If some of the delimeter matched
       but then the string changed into a mismatch go back to get characters
       that are partially in the delimeter. */
    return i - delim_i;
}

static size_t
before_r_find(SV_Str_view const hay, SV_Str_view const needle)
{
    if (needle.len > hay.len || !needle.len || !hay.len)
    {
        return hay.len;
    }
    size_t delim_i = 0;
    size_t i = 0;
    for (; i < hay.len
           && needle.s[needle.len - delim_i - 1] == hay.s[hay.len - i - 1];
         delim_i = (delim_i + 1) % needle.len, ++i)
    {}
    /* Ugly logic to account for the reverse nature of this modulo search.
       the position needs to account for any part of the delim that may
       have started to match but then mismatched. The 1 is because
       this in an index being returned not a length. */
    return i == hay.len ? hay.len : hay.len - i + delim_i - 1;
}

static inline size_t
min(size_t const a, size_t const b)
{
    return a < b ? a : b;
}

static inline ptrdiff_t
signed_max(ptrdiff_t const a, ptrdiff_t const b)
{
    return a > b ? a : b;
}

static inline SV_Order
char_cmp(char const a, char const b)
{
    return (a > b) - (a < b);
}

/* ======================   Static Utilities    =========================== */

/* This is section is modeled after the musl string.h library. However,
   using SV_Str_view that may not be null terminated requires modifications. */

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
r_memcmp(void const *const vl, void const *const vr, size_t n)
{
    unsigned char const *l = vl;
    unsigned char const *r = vr;
    for (; n && *l == *r; n--, l--, r--)
    {}
    return n ? *l - *r : 0;
}

/* strcspn is based on musl C-standard library implementation
   http://git.musl-libc.org/cgit/musl/tree/src/string/strcspn.c
   A custom implementation is necessary because C standard library impls
   have no concept of a string view and will continue searching beyond the
   end of a view until null is found. This way, string searches are
   efficient and only within the range specified. */
static size_t
view_c_spn(size_t const str_sz, char const ARR_CONST_GEQ(str, str_sz),
           size_t const set_sz, char const ARR_GEQ(set, set_sz))
{
    if (!set_sz)
    {
        return str_sz;
    }
    char const *a = str;
    size_t byteset[32 / sizeof(size_t)];
    if (set_sz == 1)
    {
        for (size_t i = 0; i < str_sz && *a != *set; ++a, ++i)
        {}
        return a - str;
    }
    memset(byteset, 0, sizeof byteset);
    for (size_t i = 0; i < set_sz && BITOP(byteset, *(unsigned char *)set, |=);
         ++set, ++i)
    {}
    for (size_t i = 0; i < str_sz && !BITOP(byteset, *(unsigned char *)a, &);
         ++a)
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
view_r_spn(size_t const str_sz, char const ARR_CONST_GEQ(str, str_sz),
           size_t const set_sz, char const ARR_GEQ(set, set_sz))
{
    char const *a = str;
    size_t byteset[32 / sizeof(size_t)] = {0};
    if (!set_sz)
    {
        return str_sz;
    }
    if (set_sz == 1)
    {
        for (size_t i = 0; i < str_sz && *a == *set; ++a, ++i)
        {}
        return a - str;
    }
    for (size_t i = 0; i < set_sz && BITOP(byteset, *(unsigned char *)set, |=);
         ++set, ++i)
    {}
    for (size_t i = 0; i < str_sz && BITOP(byteset, *(unsigned char *)a, &);
         ++a, ++i)
    {}
    return a - str;
}

/* Providing strnstrn rather than strstr at the lowest level works better
   for string views where the string may not be null terminated. There needs
   to always be the additional constraint that a search cannot exceed the
   hay length. Returns 0 based index position at which needle begins in
   hay if it can be found, otherwise the hay size is returned. */
static size_t
view_n_view_n(ptrdiff_t const hay_sz, char const ARR_CONST_GEQ(hay, hay_sz),
              ptrdiff_t const needle_sz,
              char const ARR_CONST_GEQ(needle, needle_sz))
{
    if (!hay_sz || !needle_sz || needle_sz > hay_sz)
    {
        return hay_sz;
    }
    if (1 == needle_sz)
    {
        return view_n_chr(hay_sz, hay, *needle);
    }
    if (2 == needle_sz)
    {
        return two_byte_view_n_view_n(hay_sz, (unsigned char *)hay, 2,
                                      (unsigned char *)needle);
    }
    if (3 == needle_sz)
    {
        return three_byte_view_n_view_n(hay_sz, (unsigned char *)hay, 3,
                                        (unsigned char *)needle);
    }
    if (4 == needle_sz)
    {
        return four_byte_view_n_view_n(hay_sz, (unsigned char *)hay, 4,
                                       (unsigned char *)needle);
    }
    return tw_match(hay_sz, hay, needle_sz, needle);
}

/* For now reverse logic for backwards searches has been separated into
   other functions. There is a possible formula to unit the reverse and
   forward logic into one set of functions, but the code is ugly. See
   the start of the reverse two-way algorithm for more. May unite if
   a clean way exists. */
static size_t
r_view_n_view_n(ptrdiff_t const hay_sz, char const ARR_CONST_GEQ(hay, hay_sz),
                ptrdiff_t const needle_sz,
                char const ARR_CONST_GEQ(needle, needle_sz))
{
    if (!hay_sz || !needle_sz || needle_sz > hay_sz)
    {
        return hay_sz;
    }
    if (1 == needle_sz)
    {
        return r_view_n_chr(hay_sz, hay, *needle);
    }
    if (2 == needle_sz)
    {
        return r_two_byte_view_n_view_n(hay_sz, (unsigned char *)hay, 2,
                                        (unsigned char *)needle);
    }
    if (3 == needle_sz)
    {
        return r_three_byte_view_n_view_n(hay_sz, (unsigned char *)hay, 3,
                                          (unsigned char *)needle);
    }
    if (4 == needle_sz)
    {
        return r_four_byte_view_n_view_n(hay_sz, (unsigned char *)hay, 4,
                                         (unsigned char *)needle);
    }
    return tw_rmatch(hay_sz, hay, needle_sz, needle);
}

/*==============   Post-Precomputation Two-Way Search    =================*/

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
tw_match(ptrdiff_t const hay_sz, char const ARR_CONST_GEQ(hay, hay_sz),
         ptrdiff_t const needle_sz, char const ARR_CONST_GEQ(needle, needle_sz))
{
    /* Preprocessing to get critical position and period distance. */
    struct Factorization const s = maximal_suffix(needle_sz, needle);
    struct Factorization const r = maximal_suffix_rev(needle_sz, needle);
    struct Factorization const w = (s.critical_pos > r.critical_pos) ? s : r;
    /* Determine if memoization is available due to found border/overlap. */
    if (!memcmp(needle, needle + w.period_dist, w.critical_pos + 1))
    {
        return pos_memo(hay_sz, hay, needle_sz, needle, w.period_dist,
                        w.critical_pos);
    }
    return pos_normal(hay_sz, hay, needle_sz, needle, w.period_dist,
                      w.critical_pos);
}

/* Two Way string matching algorithm adapted from ESMAJ
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260 */
static size_t
pos_memo(ptrdiff_t const hay_sz, char const ARR_CONST_GEQ(hay, hay_sz),
         ptrdiff_t const needle_sz, char const ARR_CONST_GEQ(needle, needle_sz),
         ptrdiff_t const period_dist, ptrdiff_t const critical_pos)
{
    ptrdiff_t lpos = 0;
    ptrdiff_t rpos = 0;
    /* Eliminate worst case quadratic time complexity with memoization. */
    ptrdiff_t memoize_shift = -1;
    while (lpos <= hay_sz - needle_sz)
    {
        for (rpos = signed_max(critical_pos, memoize_shift) + 1;
             rpos < needle_sz && needle[rpos] == hay[rpos + lpos]; ++rpos)
        {}
        if (rpos < needle_sz)
        {
            lpos += (rpos - critical_pos);
            memoize_shift = -1;
            continue;
        }
        /* r_pos >= needle_sz */
        for (rpos = critical_pos;
             rpos > memoize_shift && needle[rpos] == hay[rpos + lpos]; --rpos)
        {}
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
pos_normal(ptrdiff_t const hay_sz, char const ARR_CONST_GEQ(hay, hay_sz),
           ptrdiff_t const needle_sz,
           char const ARR_CONST_GEQ(needle, needle_sz), ptrdiff_t period_dist,
           ptrdiff_t const critical_pos)
{
    period_dist
        = signed_max(critical_pos + 1, needle_sz - critical_pos - 1) + 1;
    ptrdiff_t lpos = 0;
    ptrdiff_t rpos = 0;
    while (lpos <= hay_sz - needle_sz)
    {
        for (rpos = critical_pos + 1;
             rpos < needle_sz && needle[rpos] == hay[rpos + lpos]; ++rpos)
        {}
        if (rpos < needle_sz)
        {
            lpos += (rpos - critical_pos);
            continue;
        }
        /* r_pos >= needle_sz */
        for (rpos = critical_pos; rpos >= 0 && needle[rpos] == hay[rpos + lpos];
             --rpos)
        {}
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
static inline struct Factorization
maximal_suffix(ptrdiff_t const needle_sz,
               char const ARR_CONST_GEQ(needle, needle_sz))
{
    ptrdiff_t suff_pos = -1;
    ptrdiff_t period = 1;
    ptrdiff_t last_rest = 0;
    ptrdiff_t rest = 1;
    while (last_rest + rest < needle_sz)
    {
        switch (char_cmp(needle[last_rest + rest], needle[suff_pos + rest]))
        {
        case SV_ORDER_LESSER:
            last_rest += rest;
            rest = 1;
            period = last_rest - suff_pos;
            break;
        case SV_ORDER_EQUAL:
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
        case SV_ORDER_GREATER:
            suff_pos = last_rest;
            last_rest = suff_pos + 1;
            rest = period = 1;
            break;
        default:
            break;
        }
    }
    return (struct Factorization){.critical_pos = suff_pos,
                                  .period_dist = period};
}

/* Computing of the maximal suffix reverse. Sometimes called tilde.
   adapted from ESMAJ
   http://igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260 */
static inline struct Factorization
maximal_suffix_rev(ptrdiff_t const needle_sz,
                   char const ARR_CONST_GEQ(needle, needle_sz))
{
    ptrdiff_t suff_pos = -1;
    ptrdiff_t period = 1;
    ptrdiff_t last_rest = 0;
    ptrdiff_t rest = 1;
    while (last_rest + rest < needle_sz)
    {
        switch (char_cmp(needle[last_rest + rest], needle[suff_pos + rest]))
        {
        case SV_ORDER_GREATER:
            last_rest += rest;
            rest = 1;
            period = last_rest - suff_pos;
            break;
        case SV_ORDER_EQUAL:
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
        case SV_ORDER_LESSER:
            suff_pos = last_rest;
            last_rest = suff_pos + 1;
            rest = period = 1;
            break;
        default:
            break;
        }
    }
    return (struct Factorization){
        .critical_pos = suff_pos,
        .period_dist = period,
    };
}

/*=======================  Right to Left Search  ===========================*/

/* Two way algorithm is easy to reverse. Instead of trying to reverse all
   logic in the factorizations and two way searches, leave the algorithms
   and calculate the values returned as offsets from the end of the string
   instead of indices starting from 0. It would be even be possible to unite
   these functions into one with the following formula
   (using the suffix calculation as an example):

        ptrdiff_t suff_pos = -1;
        ptrdiff_t period = 1;
        ptrdiff_t last_rest = 0;
        ptrdiff_t rest = 1;
        ptrdiff_t negate_sz = 0;
        ptrdiff_t negate_one = 0;
        if (direction == FORWARD)
        {
            negate_sz = needle_sz;
            negate_one = 1;
        }
        while (last_rest + rest < needle_sz)
        {
            switch (SV_char_cmp(
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
tw_rmatch(ptrdiff_t const hay_sz, char const ARR_CONST_GEQ(hay, hay_sz),
          ptrdiff_t const needle_sz,
          char const ARR_CONST_GEQ(needle, needle_sz))
{
    struct Factorization const s = r_maximal_suffix(needle_sz, needle);
    struct Factorization const r = r_maximal_suffix_rev(needle_sz, needle);
    struct Factorization const w = (s.critical_pos > r.critical_pos) ? s : r;
    if (!r_memcmp(needle + needle_sz - 1,
                  needle + needle_sz - w.period_dist - 1, w.critical_pos + 1))
    {
        return r_pos_memo(hay_sz, hay, needle_sz, needle, w.period_dist,
                          w.critical_pos);
    }
    return r_pos_normal(hay_sz, hay, needle_sz, needle, w.period_dist,
                        w.critical_pos);
}

static size_t
r_pos_memo(ptrdiff_t const hay_sz, char const ARR_CONST_GEQ(hay, hay_sz),
           ptrdiff_t const needle_sz,
           char const ARR_CONST_GEQ(needle, needle_sz),
           ptrdiff_t const period_dist, ptrdiff_t const critical_pos)
{
    ptrdiff_t lpos = 0;
    ptrdiff_t rpos = 0;
    ptrdiff_t memoize_shift = -1;
    while (lpos <= hay_sz - needle_sz)
    {
        for (rpos = signed_max(critical_pos, memoize_shift) + 1;
             rpos < needle_sz
             && needle[needle_sz - rpos - 1] == hay[hay_sz - (rpos + lpos) - 1];
             ++rpos)
        {}
        if (rpos < needle_sz)
        {
            lpos += (rpos - critical_pos);
            memoize_shift = -1;
            continue;
        }
        /* r_pos >= needle_sz */
        for (rpos = critical_pos;
             rpos > memoize_shift
             && needle[needle_sz - rpos - 1] == hay[hay_sz - (rpos + lpos) - 1];
             --rpos)
        {}
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
r_pos_normal(ptrdiff_t const hay_sz, char const ARR_CONST_GEQ(hay, hay_sz),
             ptrdiff_t const needle_sz,
             char const ARR_CONST_GEQ(needle, needle_sz), ptrdiff_t period_dist,
             ptrdiff_t const critical_pos)
{
    period_dist
        = signed_max(critical_pos + 1, needle_sz - critical_pos - 1) + 1;
    ptrdiff_t lpos = 0;
    ptrdiff_t rpos = 0;
    while (lpos <= hay_sz - needle_sz)
    {
        for (rpos = critical_pos + 1; rpos < needle_sz
                                      && (needle[needle_sz - rpos - 1]
                                          == hay[hay_sz - (rpos + lpos) - 1]);
             ++rpos)
        {}
        if (rpos < needle_sz)
        {
            lpos += (rpos - critical_pos);
            continue;
        }
        /* r_pos >= needle_sz */
        for (rpos = critical_pos;
             rpos >= 0
             && needle[needle_sz - rpos - 1] == hay[hay_sz - (rpos + lpos) - 1];
             --rpos)
        {}
        if (rpos < 0)
        {
            return hay_sz - lpos - needle_sz;
        }
        lpos += period_dist;
    }
    return hay_sz;
}

static inline struct Factorization
r_maximal_suffix(ptrdiff_t const needle_sz,
                 char const ARR_CONST_GEQ(needle, needle_sz))
{
    ptrdiff_t suff_pos = -1;
    ptrdiff_t period = 1;
    ptrdiff_t last_rest = 0;
    ptrdiff_t rest = 1;
    while (last_rest + rest < needle_sz)
    {
        switch (char_cmp(needle[needle_sz - (last_rest + rest) - 1],
                         needle[needle_sz - (suff_pos + rest) - 1]))
        {
        case SV_ORDER_LESSER:
            last_rest += rest;
            rest = 1;
            period = last_rest - suff_pos;
            break;
        case SV_ORDER_EQUAL:
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
        case SV_ORDER_GREATER:
            suff_pos = last_rest;
            last_rest = suff_pos + 1;
            rest = period = 1;
            break;
        default:
            break;
        }
    }
    return (struct Factorization){
        .critical_pos = suff_pos,
        .period_dist = period,
    };
}

static inline struct Factorization
r_maximal_suffix_rev(ptrdiff_t const needle_sz,
                     char const ARR_CONST_GEQ(needle, needle_sz))
{
    ptrdiff_t suff_pos = -1;
    ptrdiff_t period = 1;
    ptrdiff_t last_rest = 0;
    ptrdiff_t rest = 1;
    while (last_rest + rest < needle_sz)
    {
        switch (char_cmp(needle[needle_sz - (last_rest + rest) - 1],
                         needle[needle_sz - (suff_pos + rest) - 1]))
        {
        case SV_ORDER_GREATER:
            last_rest += rest;
            rest = 1;
            period = last_rest - suff_pos;
            break;
        case SV_ORDER_EQUAL:
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
        case SV_ORDER_LESSER:
            suff_pos = last_rest;
            last_rest = suff_pos + 1;
            rest = period = 1;
            break;
        default:
            break;
        }
    }
    return (struct Factorization){
        .critical_pos = suff_pos,
        .period_dist = period,
    };
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
view_n_chr(size_t n, char const ARR_GEQ(s, n), char const c)
{
    size_t i = 0;
    for (; n && *s != c; s++, --n, ++i)
    {}
    return i;
}

static inline size_t
r_view_n_chr(size_t const n, char const ARR_CONST_GEQ(s, n), char const c)
{
    char const *x = s + n - 1;
    size_t i = n;
    for (; i && *x != c; x--, --i)
    {}
    return i ? i - 1 : n;
}

static inline size_t
two_byte_view_n_view_n(size_t const sz, unsigned char const ARR_GEQ(h, sz),
                       size_t const n_sz,
                       unsigned char const ARR_CONST_GEQ(n, n_sz))
{
    unsigned char const *const end = h + sz;
    uint16_t nw = n[0] << 8 | n[1];
    uint16_t hw = h[0] << 8 | h[1];
    for (++h; hw != nw && ++h < end; hw = (hw << 8) | *h)
    {}
    return h >= end ? sz : (sz - (size_t)(end - h)) - 1;
}

static inline size_t
r_two_byte_view_n_view_n(size_t const sz,
                         unsigned char const ARR_CONST_GEQ(h, sz),
                         size_t const n_sz,
                         unsigned char const ARR_CONST_GEQ(n, n_sz))
{
    unsigned char const *i = h + (sz - 2);
    uint16_t nw = n[0] << 8 | n[1];
    uint16_t iw = i[0] << 8 | i[1];
    /* The search is right to left therefore the Most Significant Byte will
       be the leading character of the string and the previous leading
       character is shifted to the right. */
    for (; iw != nw && --i >= h; iw = (iw >> 8) | (*i << 8))
    {}
    return i < h ? sz : (size_t)(i - h);
}

static inline size_t
three_byte_view_n_view_n(size_t const sz, unsigned char const ARR_GEQ(h, sz),
                         size_t const n_sz,
                         unsigned char const ARR_CONST_GEQ(n, n_sz))
{
    unsigned char const *const end = h + sz;
    uint32_t nw = (uint32_t)n[0] << 24 | n[1] << 16 | n[2] << 8;
    uint32_t hw = (uint32_t)h[0] << 24 | h[1] << 16 | h[2] << 8;
    for (h += 2; hw != nw && ++h < end; hw = (hw | *h) << 8)
    {}
    return h >= end ? sz : (sz - (size_t)(end - h)) - 2;
}

static inline size_t
r_three_byte_view_n_view_n(size_t const sz,
                           unsigned char const ARR_CONST_GEQ(h, sz),
                           size_t const n_sz,
                           unsigned char const ARR_CONST_GEQ(n, n_sz))
{
    unsigned char const *i = h + (sz - 3);
    uint32_t nw = (uint32_t)n[0] << 16 | n[1] << 8 | n[2];
    uint32_t iw = (uint32_t)i[0] << 16 | i[1] << 8 | i[2];
    /* Align the bits with fewer left shifts such that as the parsing
       progresses right left, the leading character always takes highest
       bit position and there is no need for any masking. */
    for (; iw != nw && --i >= h; iw = (iw >> 8) | (*i << 16))
    {}
    return i < h ? sz : (size_t)(i - h);
}

static inline size_t
four_byte_view_n_view_n(size_t const sz, unsigned char const ARR_GEQ(h, sz),
                        size_t const n_sz,
                        unsigned char const ARR_CONST_GEQ(n, n_sz))
{
    unsigned char const *const end = h + sz;
    uint32_t nw = (uint32_t)n[0] << 24 | n[1] << 16 | n[2] << 8 | n[3];
    uint32_t hw = (uint32_t)h[0] << 24 | h[1] << 16 | h[2] << 8 | h[3];
    for (h += 3; hw != nw && ++h < end; hw = (hw << 8) | *h)
    {}
    return h >= end ? sz : (sz - (size_t)(end - h)) - 3;
}

static inline size_t
r_four_byte_view_n_view_n(size_t const sz,
                          unsigned char const ARR_CONST_GEQ(h, sz),
                          size_t const n_sz,
                          unsigned char const ARR_CONST_GEQ(n, n_sz))
{
    unsigned char const *i = h + (sz - 4);
    uint32_t nw = (uint32_t)n[0] << 24 | n[1] << 16 | n[2] << 8 | n[3];
    uint32_t iw = (uint32_t)i[0] << 24 | i[1] << 16 | i[2] << 8 | i[3];
    /* Now that all four bytes of the unsigned int are used the shifting
       becomes more intuitive. The window slides left to right and the
       next leading character takes the high bit position. */
    for (; iw != nw && --i >= h; iw = (iw >> 8) | (*i << 24))
    {}
    return i < h ? sz : (size_t)(i - h);
}
