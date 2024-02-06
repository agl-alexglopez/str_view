#include "str_view.h"
#include "test.h"

#include <stdio.h>

static enum test_result test_iter(void);
static enum test_result test_iter_repeating_delim(void);
static enum test_result test_iter_multichar_delim(void);
static enum test_result test_iter_multichar_delim_short(void);
static enum test_result test_iter_delim_larger_than_str(void);

#define NUM_TESTS (size_t)5
const struct fn_name all_tests[NUM_TESTS] = {
    {test_iter, "test_iter"},
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
    /* This version should only give us the letters because delim is ' ' */
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
    /* Do at least one token iteration if we can't find any delims */
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
    /* This version should only give us the letters because delim is ' ' */
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
    /* Do at least one token iteration if we can't find any delims */
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
