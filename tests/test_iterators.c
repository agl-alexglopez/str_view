#include "str_view.h"
#include "test.h"

#include <stdio.h>

static enum test_result test_iter(void);
static enum test_result test_min_delim(void);
static enum test_result test_simple_delim(void);
static enum test_result test_tail_delim(void);
static enum test_result test_iter_repeating_delim(void);
static enum test_result test_iter_multichar_delim(void);
static enum test_result test_iter_multichar_delim_short(void);
static enum test_result test_iter_delim_larger_than_str(void);

#define NUM_TESTS (size_t)8
const struct fn_name all_tests[NUM_TESTS] = {
    {test_iter, "test_iter"},
    {test_min_delim, "test_min_delim"},
    {test_simple_delim, "test_simple_delim"},
    {test_tail_delim, "test_tail_delim"},
    {test_iter_repeating_delim, "test_iter_repeating_delim"},
    {test_iter_multichar_delim, "test_iter_multichar_delim"},
    {test_iter_multichar_delim_short, "test_iter_multichar_delim_short"},
    {test_iter_delim_larger_than_str, "test_iter_delim_larger_than_str"},
};

int
main()
{
    enum test_result res = PASS;
    for (size_t i = 0; i < NUM_TESTS; ++i)
    {
        const enum test_result t_res = all_tests[i].fn();
        if (t_res == FAIL)
        {
            printf("\n");
            printf("test_iterators test failed: %s\n", all_tests[i].name);
            res = FAIL;
        }
    }
    return res;
}

static enum test_result
test_iter(void)
{
    const char *const reference = "A B C D E G H I J K L M N O P";
    str_view chars = sv(reference);
    size_t i = 0;
    for (const char *cur = sv_begin(chars); cur != sv_end(chars);
         cur = sv_next(cur))
    {
        if (*cur != reference[i])
        {
            return FAIL;
        }
        ++i;
    }
    i = 0;
    /* This version should only give us the letters because delim is ' ' */
    str_view cur = sv_begin_tok(chars, (str_view){" ", 1});
    for (; !sv_end_tok(cur); cur = sv_next_tok(cur, (str_view){" ", 1}))
    {
        if (sv_front(cur) != reference[i])
        {
            return FAIL;
        }
        i += 2;
    }
    if (*cur.s != '\0')
    {
        return FAIL;
    }
    /* Do at least one token iteration if we can't find any delims */
    str_view cur2 = sv_begin_tok(chars, (str_view){",", 1});
    for (; !sv_end_tok(cur2); cur2 = sv_next_tok(cur2, (str_view){",", 1}))
    {
        if (sv_strcmp(cur2, reference) != 0)
        {
            return FAIL;
        }
    }
    if (*cur2.s != '\0')
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_min_delim(void)
{
    const char *const reference = "0/0";
    const char *const toks[2] = {"0", "0"};
    const str_view delim = {"/", SVLEN("/")};
    size_t i = 0;
    for (str_view tok = sv_begin_tok(sv(reference), delim); !sv_end_tok(tok);
         tok = sv_next_tok(tok, delim))
    {
        if (sv_strcmp(tok, toks[i]))
        {
            return FAIL;
        }
        ++i;
    }
    if (i != sizeof(toks) / sizeof(toks[0]))
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_simple_delim(void)
{
    const char *const reference = "0/1/2/2/3//3////3/4/4/4/////4";
    const char *const toks[11] = {
        "0", "1", "2", "2", "3", "3", "3", "4", "4", "4", "4",
    };
    const str_view delim = {"/", SVLEN("/")};
    size_t i = 0;
    for (str_view tok = sv_begin_tok(sv(reference), delim); !sv_end_tok(tok);
         tok = sv_next_tok(tok, delim))
    {
        if (sv_strcmp(tok, toks[i]))
        {
            return FAIL;
        }
        ++i;
    }
    if (i != sizeof(toks) / sizeof(toks[0]))
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_tail_delim(void)
{
    const char *const reference = "0/1//2//2//3//3////3//4//4//4///////4578";
    const char *const toks[10] = {
        "0/1", "2", "2", "3", "3", "3", "4", "4", "4", "/4578",
    };
    const str_view delim = {"//", SVLEN("//")};
    size_t i = 0;
    for (str_view tok = sv_begin_tok(sv(reference), delim); !sv_end_tok(tok);
         tok = sv_next_tok(tok, delim))
    {
        if (sv_strcmp(tok, toks[i]))
        {
            return FAIL;
        }
        ++i;
    }
    if (i != sizeof(toks) / sizeof(toks[0]))
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_iter_repeating_delim(void)
{
    const char *toks[14] = {
        "A",  "B", "C", "D",   "E", "F",  "G",
        "HI", "J", "K", "LMN", "O", "Pi", "\\(*.*)/",
    };
    const char *const reference
        = " A   B  C     D  E F G HI J   K LMN O   Pi  \\(*.*)/  ";
    const str_view ref_view = sv(reference);
    size_t i = 0;
    /* This version should only give us the letters because delim is ' ' */
    str_view cur = sv_begin_tok(ref_view, (str_view){" ", 1});
    for (; !sv_end_tok(cur); cur = sv_next_tok(cur, (str_view){" ", 1}))
    {
        if (sv_strcmp(cur, toks[i]) != 0)
        {
            return FAIL;
        }
        ++i;
    }
    if (*cur.s != '\0')
    {
        return FAIL;
    }
    /* Do at least one token iteration if we can't find any delims */
    str_view cur2 = sv_begin_tok(ref_view, (str_view){",", 1});
    for (; !sv_end_tok(cur2); cur2 = sv_next_tok(cur2, (str_view){",", 1}))
    {
        if (sv_strcmp(cur2, reference) != 0)
        {
            return FAIL;
        }
    }
    if (*cur2.s != '\0')
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_iter_multichar_delim(void)
{
    const char *toks[14] = {
        "A",     "B", "C", "D",      "E", "F",  "G",
        "HacbI", "J", "K", "LcbaMN", "O", "Pi", "\\(*.*)/",
    };
    const char *const reference
        = "abcAabcBabcCabcabcabcDabcEabcFabcGabcHacbIabcJabcabcabcabcKabcLcbaMN"
          "abcOabcabcPiabcabc\\(*.*)/abc";
    size_t i = 0;
    const char *const delim = "abc";
    const size_t delim_len = sv_strlen(delim);
    const str_view ref_view = sv(reference);
    str_view cur = sv_begin_tok(ref_view, (str_view){delim, delim_len});
    for (; !sv_end_tok(cur);
         cur = sv_next_tok(cur, (str_view){delim, delim_len}))
    {
        if (sv_strcmp(cur, toks[i]) != 0)
        {
            return FAIL;
        }
        ++i;
    }
    if (*cur.s != '\0')
    {
        return FAIL;
    }
    str_view cur2 = sv_begin_tok(ref_view, (str_view){" ", 1});
    for (; !sv_end_tok(cur2); cur2 = sv_next_tok(cur2, (str_view){" ", 1}))
    {
        if (sv_strcmp(cur2, reference) != 0)
        {
            return FAIL;
        }
    }
    if (*cur2.s != '\0')
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_iter_multichar_delim_short(void)
{
    const char *toks[14] = {
        "A",     "B", "C", "D",      "E",   "F",  "G",
        "H---I", "J", "K", "L-M--N", "--O", "Pi", "\\(*.*)/",
    };
    const char *const reference = "-----A-----B-----C-----D-----E-----F-----G--"
                                  "---H---I-----J-----K-----L-M--N"
                                  "-------O-----Pi-----\\(*.*)/-----";
    size_t i = 0;
    const char *const delim = "-----";
    const size_t delim_len = sv_strlen(delim);
    const str_view ref_view = sv(reference);
    str_view cur = sv_begin_tok(ref_view, (str_view){delim, delim_len});
    for (; !sv_end_tok(cur);
         cur = sv_next_tok(cur, (str_view){delim, delim_len}))
    {
        if (sv_strcmp(cur, toks[i]) != 0)
        {
            return FAIL;
        }
        ++i;
    }
    if (*cur.s != '\0')
    {
        return FAIL;
    }
    str_view cur2 = sv_begin_tok(ref_view, (str_view){" ", 1});
    for (; !sv_end_tok(cur2); cur2 = sv_next_tok(cur2, (str_view){" ", 1}))
    {
        if (sv_strcmp(cur2, reference) != 0)
        {
            return FAIL;
        }
    }
    if (*cur2.s != '\0')
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_iter_delim_larger_than_str(void)
{
    const char *const reference = "A-B";
    /* This delimeter is too large so we should just take the whole string */
    const char *const delim = "-----";
    const size_t delim_len = sv_strlen(delim);
    str_view constructed = sv_delim(reference, delim);
    str_view cur = sv_begin_tok((str_view){reference, sv_strlen(reference)},
                                (str_view){delim, delim_len});
    if (sv_svcmp(constructed, cur) != EQL
        || sv_strcmp(constructed, reference) != EQL
        || sv_strcmp(cur, reference) != EQL)
    {
        return FAIL;
    }
    for (; !sv_end_tok(cur);
         cur = sv_next_tok(cur, (str_view){delim, delim_len}))
    {
        if (sv_strcmp(cur, reference) != EQL)
        {
            return FAIL;
        }
    }
    if (*cur.s != '\0')
    {
        return FAIL;
    }
    return PASS;
}
