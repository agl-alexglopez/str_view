#include "str_view.h"
#include "test.h"

#include <stdio.h>

static enum test_result test_iter(void);
static enum test_result test_iter2(void);
static enum test_result test_riter(void);
static enum test_result test_riter2(void);
static enum test_result test_riter_multi(void);
static enum test_result test_min_delim(void);
static enum test_result test_simple_delim(void);
static enum test_result test_tail_delim(void);
static enum test_result test_iter_repeating_delim(void);
static enum test_result test_iter_multichar_delim(void);
static enum test_result test_iter_multichar_delim_short(void);
static enum test_result test_iter_delim_larger_than_str(void);
static enum test_result test_tokenize_not_terminated(void);
static enum test_result test_tokenize_three_views(void);

#define NUM_TESTS (size_t)14
const struct fn_name all_tests[NUM_TESTS] = {
    {test_iter, "test_iter"},
    {test_iter2, "test_iter2"},
    {test_riter, "test_riter"},
    {test_riter_multi, "test_riter_multi"},
    {test_riter2, "test_riter2"},
    {test_min_delim, "test_min_delim"},
    {test_simple_delim, "test_simple_delim"},
    {test_tail_delim, "test_tail_delim"},
    {test_iter_repeating_delim, "test_iter_repeating_delim"},
    {test_iter_multichar_delim, "test_iter_multichar_delim"},
    {test_iter_multichar_delim_short, "test_iter_multichar_delim_short"},
    {test_iter_delim_larger_than_str, "test_iter_delim_larger_than_str"},
    {test_tokenize_not_terminated, "test_tokenize_not_terminated"},
    {test_tokenize_three_views, "test_tokenize_three_view"},
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
        CHECK(*cur, reference[i]);
        ++i;
    }
    i = 0;
    /* This version should only give us the letters because delim is ' ' */
    str_view cur = sv_begin_tok(chars, sv(" "));
    for (; !sv_end_tok(chars, cur); cur = sv_next_tok(chars, cur, sv(" ")))
    {
        CHECK(sv_front(cur), reference[i]);
        i += 2;
    }
    CHECK(*cur.s, '\0');
    /* Do at least one token iteration if we can't find any delims */
    str_view cur2 = sv_begin_tok(chars, sv(","));
    for (; !sv_end_tok(chars, cur2); cur2 = sv_next_tok(chars, cur2, sv(",")))
    {
        CHECK(sv_strcmp(cur2, reference), EQL);
    }
    CHECK(*cur2.s, '\0');
    return PASS;
}

static enum test_result
test_iter2(void)
{
    /* Start and end the string with spaces to check edgecases. */
    const char *const reference = " A B C D E G H I J K L M N O P ";
    const size_t size = 15;
    const char *const toks[15] = {
        "A", "B", "C", "D", "E", "G", "H", "I",
        "J", "K", "L", "M", "N", "O", "P",
    };
    str_view chars = sv(reference);
    size_t i = 0;
    for (const char *cur = sv_begin(chars);
         cur != sv_end(chars) && i < sv_svlen(chars); cur = sv_next(cur))
    {
        CHECK(*cur, reference[i]);
        ++i;
    }
    i = 0;
    /* This version should only give us the letters because delim is ' ' */
    str_view cur = sv_begin_tok(chars, sv(" "));
    for (; !sv_end_tok(chars, cur) && i < size;
         cur = sv_next_tok(chars, cur, sv(" ")))
    {
        CHECK(sv_front(cur), *toks[i]);
        CHECK(sv_svlen(cur), sv_strlen(toks[i]));
        ++i;
    }
    CHECK(*cur.s, '\0');
    /* Do at least one token iteration if we can't find any delims */
    i = 0;
    str_view cur2 = sv_begin_tok(chars, sv(","));
    for (; !sv_end_tok(chars, cur2) && i < 1;
         cur2 = sv_next_tok(chars, cur2, sv(",")))
    {
        CHECK(sv_strcmp(cur2, reference), EQL);
        ++i;
    }
    CHECK(*cur2.s, '\0');
    return PASS;
}

static enum test_result
test_riter(void)
{
    const str_view ref = sv("A B C D E G H I J K L M N O P");
    size_t i = ref.sz - 1;
    /* This version should only give us the letters because delim is ' ' */
    str_view cur = sv_rbegin_tok(ref, sv(" "));
    for (; !sv_rend_tok(ref, cur); cur = sv_rnext_tok(ref, cur, sv(" ")))
    {
        CHECK(sv_front(cur), *sv_pos(ref, i));
        i -= 2;
    }
    CHECK(cur.s, ref.s);
    /* Do at least one token iteration if we can't find any delims */
    str_view cur2 = sv_rbegin_tok(ref, sv(","));
    for (; !sv_rend_tok(ref, cur2); cur2 = sv_rnext_tok(ref, cur2, sv(",")))
    {
        CHECK(sv_svcmp(cur2, ref), EQL);
    }
    CHECK(cur2.s, ref.s);
    return PASS;
}

static enum test_result
test_riter2(void)
{
    /* Start and end the string with spaces to check edgecases. */
    const str_view ref = sv(" A B C D E G H I J K L M N O P ");
    const size_t size = 15;
    const char *const toks[15] = {
        "A", "B", "C", "D", "E", "G", "H", "I",
        "J", "K", "L", "M", "N", "O", "P",
    };
    size_t i = size;
    /* This version should only give us the letters because delim is ' ' */
    str_view cur = sv_rbegin_tok(ref, sv(" "));
    for (; !sv_rend_tok(ref, cur) && i; cur = sv_rnext_tok(ref, cur, sv(" ")))
    {
        --i;
        CHECK(sv_front(cur), *toks[i]);
        CHECK(sv_svlen(cur), sv_strlen(toks[i]));
    }
    CHECK(cur.s, ref.s);
    /* Do at least one token iteration if we can't find any delims */
    i = 1;
    str_view cur2 = sv_rbegin_tok(ref, sv(","));
    for (; !sv_rend_tok(ref, cur2) && i;
         cur2 = sv_rnext_tok(ref, cur2, sv(",")))
    {
        --i;
        CHECK(sv_svcmp(cur2, ref), EQL);
        CHECK(sv_svlen(cur2), sv_svlen(ref));
    }
    CHECK(cur2.s, ref.s);
    return PASS;
}

static enum test_result
test_riter_multi(void)
{
    const str_view ref = sv("//A//B//C//D//E//G//H//I//J//K//L//M//N//O//P//");
    const str_view delim = sv("//");
    const size_t size = 15;
    const char *const toks[15] = {
        "A", "B", "C", "D", "E", "G", "H", "I",
        "J", "K", "L", "M", "N", "O", "P",
    };
    size_t i = size;
    const size_t last_delim_pos = sv_rfind(ref, sv_svlen(ref), delim);
    CHECK(last_delim_pos, sv_svlen(ref) - 2);
    /* This version should only give us the letters because delim is ' ' */
    str_view cur = sv_rbegin_tok(ref, delim);
    for (; !sv_rend_tok(ref, cur) && i; cur = sv_rnext_tok(ref, cur, delim))
    {
        --i;
        CHECK(sv_front(cur), *toks[i]);
        CHECK(sv_svlen(cur), sv_strlen(toks[i]));
    }
    CHECK(cur.s, ref.s);
    /* Do at least one token iteration if we can't find any delims */
    i = 1;
    str_view cur2 = sv_rbegin_tok(ref, SV(","));
    for (; !sv_rend_tok(ref, cur2) && i;
         cur2 = sv_rnext_tok(ref, cur2, SV(",")))
    {
        --i;
        CHECK(sv_svcmp(cur2, ref), EQL);
        CHECK(sv_svlen(cur2), sv_svlen(ref));
    }
    CHECK(cur2.s, ref.s);
    return PASS;
}

static enum test_result
test_min_delim(void)
{
    const char *const reference = "/0/0";
    const char *const toks[2] = {"0", "0"};
    const size_t size = sizeof(toks) / sizeof(toks[0]);
    const str_view delim = sv("/");
    const str_view ref_view = sv(reference);
    size_t i = 0;
    for (str_view tok = sv_begin_tok(ref_view, delim);
         i < size && !sv_end_tok(ref_view, tok);
         tok = sv_next_tok(ref_view, tok, delim))
    {
        CHECK(sv_strcmp(tok, toks[i]), EQL);
        CHECK(sv_svlen(tok), sv_strlen(toks[i]));
        ++i;
    }
    CHECK(i, sizeof(toks) / sizeof(toks[0]));
    return PASS;
}

static enum test_result
test_simple_delim(void)
{
    const char *const reference = "0/1/2/2/3//3////3/4/4/4/////4";
    const char *const toks[11] = {
        "0", "1", "2", "2", "3", "3", "3", "4", "4", "4", "4",
    };
    const str_view ref_view = sv(reference);
    const str_view delim = sv("/");
    size_t i = 0;
    for (str_view tok = sv_begin_tok(ref_view, delim);
         !sv_end_tok(ref_view, tok) && i < sizeof(toks) / sizeof(toks[0]);
         tok = sv_next_tok(ref_view, tok, delim))
    {
        CHECK(sv_strcmp(tok, toks[i]), EQL);
        CHECK(sv_svlen(tok), sv_strlen(toks[i]));
        ++i;
    }
    CHECK(i, sizeof(toks) / sizeof(toks[0]));
    return PASS;
}

static enum test_result
test_tail_delim(void)
{
    const char *const reference = "0/1//2//2//3//3////3//4//4//4///////4578";
    const char *const toks[10] = {
        "0/1", "2", "2", "3", "3", "3", "4", "4", "4", "/4578",
    };
    const str_view ref_view = sv(reference);
    const str_view delim = sv("//");
    size_t i = 0;
    for (str_view tok = sv_begin_tok(ref_view, delim);
         !sv_end_tok(ref_view, tok); tok = sv_next_tok(ref_view, tok, delim))
    {
        CHECK(sv_strcmp(tok, toks[i]), EQL);
        CHECK(sv_svlen(tok), sv_strlen(toks[i]));
        ++i;
    }
    CHECK(i, sizeof(toks) / sizeof(toks[0]));
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
    str_view cur = sv_begin_tok(ref_view, sv(" "));
    for (; !sv_end_tok(ref_view, cur);
         cur = sv_next_tok(ref_view, cur, sv(" ")))
    {
        CHECK(sv_strcmp(cur, toks[i]), EQL);
        CHECK(sv_svlen(cur), sv_strlen(toks[i]));
        ++i;
    }
    CHECK(*cur.s, '\0');
    /* Do at least one token iteration if we can't find any delims */
    str_view cur2 = sv_begin_tok(ref_view, sv(","));
    for (; !sv_end_tok(ref_view, cur2);
         cur2 = sv_next_tok(ref_view, cur2, sv(",")))
    {
        CHECK(sv_strcmp(cur2, reference), EQL);
        CHECK(sv_svlen(cur2), sv_strlen(reference));
    }
    CHECK(*cur2.s, '\0');
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
    for (; !sv_end_tok(ref_view, cur);
         cur = sv_next_tok(ref_view, cur, (str_view){delim, delim_len}))
    {
        CHECK(sv_strcmp(cur, toks[i]), EQL);
        CHECK(sv_svlen(cur), sv_strlen(toks[i]));
        ++i;
    }
    CHECK(*cur.s, '\0');
    str_view cur2 = sv_begin_tok(ref_view, sv(" "));
    for (; !sv_end_tok(ref_view, cur2);
         cur2 = sv_next_tok(ref_view, cur2, sv(" ")))
    {
        CHECK(sv_strcmp(cur2, reference), EQL);
        CHECK(sv_svlen(cur2), sv_strlen(reference));
    }
    CHECK(*cur2.s, '\0');
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
    for (; !sv_end_tok(ref_view, cur);
         cur = sv_next_tok(ref_view, cur, (str_view){delim, delim_len}))
    {
        CHECK(sv_strcmp(cur, toks[i]), EQL);
        CHECK(sv_svlen(cur), sv_strlen(toks[i]));
        ++i;
    }
    CHECK(*cur.s, '\0');
    str_view cur2 = sv_begin_tok(ref_view, sv(" "));
    for (; !sv_end_tok(ref_view, cur2);
         cur2 = sv_next_tok(ref_view, cur2, sv(" ")))
    {
        CHECK(sv_strcmp(cur2, reference), EQL);
        CHECK(sv_svlen(cur2), sv_strlen(reference));
    }
    CHECK(*cur2.s, '\0');
    return PASS;
}

static enum test_result
test_iter_delim_larger_than_str(void)
{
    const char *const reference = "A-B";
    /* This delimeter is too large so we should just take the whole string */
    const str_view delim = sv("-----");
    str_view constructed = sv_delim(reference, delim.s);
    str_view cur
        = sv_begin_tok((str_view){reference, sv_strlen(reference)}, delim);
    CHECK(sv_svcmp(constructed, cur), EQL);
    CHECK(sv_strcmp(constructed, reference), EQL);
    CHECK(sv_strcmp(cur, reference), EQL);

    for (; !sv_end_tok(sv(reference), cur);
         cur = sv_next_tok(sv(reference), cur, delim))
    {
        CHECK(sv_strcmp(cur, reference), EQL);
        CHECK(sv_svlen(cur), sv_strlen(reference));
    }
    CHECK(*cur.s, '\0');
    return PASS;
}

static enum test_result
test_tokenize_not_terminated(void)
{
    const char *const path_str = "this/path/will/be/missing/its/child";
    const char *const toks[6] = {
        "this", "path", "will", "be", "missing", "its",
    };
    const str_view path = sv(path_str);
    const str_view delim = sv("/");
    const str_view childless_path
        = sv_remove_suffix(path, sv_svlen(path) - sv_find_last_of(path, delim));
    size_t i = 0;
    for (str_view tok = sv_begin_tok(childless_path, delim);
         !sv_end_tok(childless_path, tok);
         tok = sv_next_tok(childless_path, tok, delim))
    {
        CHECK(sv_strcmp(tok, toks[i]), EQL);
        CHECK(sv_svlen(tok), sv_strlen(toks[i]));
        ++i;
    }
    CHECK(i, sizeof(toks) / sizeof(toks[0]));
    return PASS;
}

static enum test_result
test_tokenize_three_views(void)
{
    const char *const path_str = "all/of/these/paths/are/unique/and/split/up";
    const char *const toks[3][3] = {
        {"all", "of", "these"},
        {"paths", "are", "unique"},
        {"and", "split", "up"},
    };
    const size_t size = sizeof(toks) / sizeof(toks[0]);
    const str_view path = sv(path_str);
    const str_view delim = sv("/");
    const str_view first = sv_substr(path, 0, sv_find(path, 0, sv("/paths/")));
    const str_view second = sv_substr(path, sv_find(path, 0, sv("/paths/")),
                                      sv_find(path, 0, sv("/and/"))
                                          - sv_find(path, 0, sv("/paths/")));
    const str_view third
        = sv_substr(path, sv_find(path, 0, sv("/and/")),
                    sv_svlen(path) - sv_find(path, 0, sv("/and/")));
    size_t i = 0;
    for (str_view tok1 = sv_begin_tok(first, delim),
                  tok2 = sv_begin_tok(second, delim),
                  tok3 = sv_begin_tok(third, delim);
         !sv_end_tok(first, tok1) && !sv_end_tok(second, tok2)
         && !sv_end_tok(third, tok3) && i < size;
         tok1 = sv_next_tok(first, tok1, delim),
                  tok2 = sv_next_tok(second, tok2, delim),
                  tok3 = sv_next_tok(third, tok3, delim))
    {
        CHECK(sv_strcmp(tok1, toks[0][i]), EQL);
        CHECK(sv_svlen(tok1), sv_strlen(toks[0][i]));
        CHECK(sv_strcmp(tok2, toks[1][i]), EQL);
        CHECK(sv_svlen(tok2), sv_strlen(toks[1][i]));
        CHECK(sv_strcmp(tok3, toks[2][i]), EQL);
        CHECK(sv_svlen(tok3), sv_strlen(toks[2][i]));
        ++i;
    }
    CHECK(i, sizeof(toks) / sizeof(toks[0]));
    return PASS;
}
