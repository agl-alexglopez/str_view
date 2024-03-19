#include "str_view.h"
#include "test.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

static enum test_result test_small_find(void);
static enum test_result test_small_rfind(void);
static enum test_result test_rfind_off_by_one(void);
static enum test_result test_find_of_sets(void);
static enum test_result test_substring_brute_force(void);
static enum test_result test_rfind_brute_force(void);
static enum test_result test_find_rfind_memoization(void);
static enum test_result test_substring_off_by_one(void);
static enum test_result test_substring_search(void);
static enum test_result test_rsubstring_search(void);

#define NUM_TESTS (size_t)10
const struct fn_name all_tests[NUM_TESTS] = {
    {test_small_find, "test_small_find"},
    {test_small_rfind, "test_small_rfind"},
    {test_find_of_sets, "test_find_of_sets"},
    {test_substring_brute_force, "test_substring_brute_force"},
    {test_rfind_brute_force, "test_rfind_brute_force"},
    {test_rfind_off_by_one, "test_rfind_off_by_one"},
    {test_find_rfind_memoization, "test_find_rfind_memoization"},
    {test_substring_off_by_one, "test_substring_off_by_one"},
    {test_substring_search, "test_substring_search"},
    {test_rsubstring_search, "test_rsubstring_search"},
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
            printf("test_string_searching test failed: %s\n",
                   all_tests[i].name);
            res = FAIL;
        }
    }
    return res;
}

static enum test_result
test_small_find(void)
{
    const char ref[20] = {
        [0] = 'A',  [1] = 'A',  [2] = 'C',  [3] = ' ',  [4] = '!',
        [5] = '!',  [6] = '!',  [7] = ' ',  [8] = '*',  [9] = '*',
        [10] = ' ', [11] = '_', [12] = '_', [13] = ' ', [14] = '!',
        [15] = '!', [16] = '!', [17] = ' ', [18] = 'A', [19] = '\0',
    };
    str_view str = sv(ref);
    if (sv_find(str, 0, sv("C")) != 2)
    {
        return FAIL;
    }
    if (sv_find(str, 0, sv("")) != 19)
    {
        return FAIL;
    }
    if (sv_find(str, 0, sv("_")) != 11)
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_small_rfind(void)
{
    const char ref[20] = {
        [0] = 'Y',  [1] = 'A',  [2] = 'C',  [3] = ' ',  [4] = '!',
        [5] = '!',  [6] = '!',  [7] = ' ',  [8] = '*',  [9] = '*',
        [10] = ' ', [11] = '_', [12] = '_', [13] = ' ', [14] = '!',
        [15] = '!', [16] = '!', [17] = ' ', [18] = 'A', [19] = '\0',
    };
    str_view str = sv(ref);
    if (sv_rfind(str, str.sz, sv("!")) != 16)
    {
        return FAIL;
    }
    if (sv_rfind(str, str.sz, sv("Y")) != 0)
    {
        return FAIL;
    }
    if (sv_rfind(str, str.sz, sv("X")) != 19)
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_find_of_sets(void)
{
    const char ref[25] = {
        [0] = 'A',  [1] = 'A',  [2] = 'C',  [3] = 'B',  [4] = '!',
        [5] = '!',  [6] = '!',  [7] = ' ',  [8] = '*',  [9] = '.',
        [10] = ':', [11] = ';', [12] = ',', [13] = ' ', [14] = '?',
        [15] = ' ', [16] = '_', [17] = '_', [18] = ' ', [19] = '!',
        [20] = '!', [21] = '!', [22] = 'Z', [23] = 'z', [24] = '\0',
    };
    str_view str = sv(ref);
    if (sv_find_first_of(str, sv("CB!")) != 2)
    {
        return FAIL;
    }
    if (sv_find_first_of(str, sv("")) != 24)
    {
        return FAIL;
    }
    if (sv_find_last_of(str, sv("! _")) != 21)
    {
        return FAIL;
    }
    if (sv_find_last_not_of(str, sv("CBA!")) != 22)
    {
        return FAIL;
    }
    if (sv_find_first_not_of(str, sv("ACB!;:, *.")) != 14)
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_substring_brute_force(void)
{
    const char *one_byte_needle = "A";
    const char *two_byte_needle = "AA";
    const char *three_byte_needle = "AAA";
    const char *four_byte_needle = "AAAA";
    const char *needle = "find the needle!";
    const char *const haystack = "__A__AA___AAA___AAAA___find the needle!___";
    const char *one_byte_found = strstr(haystack, one_byte_needle);
    const char *two_byte_found = strstr(haystack, two_byte_needle);
    const char *three_byte_found = strstr(haystack, three_byte_needle);
    const char *four_byte_found = strstr(haystack, four_byte_needle);
    const char *needle_found = strstr(haystack, needle);
    const str_view haystack_view = sv(haystack);
    const str_view one_byte_view = sv_svsv(haystack_view, sv(one_byte_needle));
    const str_view two_byte_view = sv_svsv(haystack_view, sv(two_byte_needle));
    const str_view three_byte_view
        = sv_svsv(haystack_view, sv(three_byte_needle));
    const str_view four_byte_view
        = sv_svsv(haystack_view, sv(four_byte_needle));
    const str_view needle_view = sv_svsv(haystack_view, sv(needle));
    if (one_byte_found != sv_begin(one_byte_view)
        || sv_strcmp(one_byte_view, one_byte_needle) != EQL
        || two_byte_found != sv_begin(two_byte_view)
        || sv_strcmp(two_byte_view, two_byte_needle) != EQL
        || three_byte_found != sv_begin(three_byte_view)
        || sv_strcmp(three_byte_view, three_byte_needle) != EQL
        || four_byte_found != sv_begin(four_byte_view)
        || sv_strcmp(four_byte_view, four_byte_needle) != EQL
        || needle_found != sv_begin(needle_view)
        || sv_strcmp(needle_view, needle) != EQL)
    {
        return FAIL;
    }
    const str_view one_byte_fail = sv_svsv(haystack_view, sv("J"));
    const str_view two_byte_fail = sv_svsv(haystack_view, sv("XY"));
    const str_view three_byte_fail = sv_svsv(haystack_view, sv("ZZY"));
    const str_view four_byte_fail = sv_svsv(haystack_view, sv("8888"));
    const str_view needle_fail = sv_svsv(haystack_view, sv("this is failure"));
    if (one_byte_fail.sz != 0 || two_byte_fail.sz != 0
        || three_byte_fail.sz != 0 || four_byte_fail.sz != 0
        || needle_fail.sz != 0)
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_rfind_brute_force(void)
{
    const char *one_byte_needle = "A";
    const char *two_byte_needle = "BB";
    const char *three_byte_needle = "CCC";
    const char *four_byte_needle = "DDDD";
    const char *needle = "find the needle!";
    const char *const haystack = "++DDDD++CCC+++BB+++A+++find the needle!+++";
    const char *one_byte_found = strstr(haystack, one_byte_needle);
    const char *two_byte_found = strstr(haystack, two_byte_needle);
    const char *three_byte_found = strstr(haystack, three_byte_needle);
    const char *four_byte_found = strstr(haystack, four_byte_needle);
    const char *needle_found = strstr(haystack, needle);
    const str_view haystack_view = sv(haystack);
    const size_t one_byte_pos
        = sv_rfind(haystack_view, haystack_view.sz, sv(one_byte_needle));
    const size_t two_byte_pos
        = sv_rfind(haystack_view, haystack_view.sz, sv(two_byte_needle));
    const size_t three_byte_pos
        = sv_rfind(haystack_view, haystack_view.sz, sv(three_byte_needle));
    const size_t four_byte_pos
        = sv_rfind(haystack_view, haystack_view.sz, sv(four_byte_needle));
    const size_t needle_pos
        = sv_rfind(haystack_view, haystack_view.sz, sv(needle));
    const str_view one_byte_rsvsv
        = sv_rsvsv(haystack_view, sv(one_byte_needle));
    const str_view two_byte_rsvsv
        = sv_rsvsv(haystack_view, sv(two_byte_needle));
    const str_view three_byte_rsvsv
        = sv_rsvsv(haystack_view, sv(three_byte_needle));
    const str_view four_byte_rsvsv
        = sv_rsvsv(haystack_view, sv(four_byte_needle));
    const str_view needle_rsvsv = sv_rsvsv(haystack_view, sv(needle));
    if (one_byte_pos != (size_t)(one_byte_found - haystack)
        || two_byte_pos != (size_t)(two_byte_found - haystack)
        || three_byte_pos != (size_t)(three_byte_found - haystack)
        || four_byte_pos != (size_t)(four_byte_found - haystack)
        || needle_pos != (size_t)(needle_found - haystack)
        || sv_strcmp(one_byte_rsvsv, one_byte_needle) != EQL
        || sv_strcmp(two_byte_rsvsv, two_byte_needle) != EQL
        || sv_strcmp(three_byte_rsvsv, three_byte_needle) != EQL
        || sv_strcmp(four_byte_rsvsv, four_byte_needle) != EQL
        || sv_strcmp(needle_rsvsv, needle) != EQL)
    {
        return FAIL;
    }
    const size_t one_byte_fail
        = sv_rfind(haystack_view, haystack_view.sz, sv("J"));
    const size_t two_byte_fail
        = sv_rfind(haystack_view, haystack_view.sz, sv("ZZ"));
    const size_t three_byte_fail
        = sv_rfind(haystack_view, haystack_view.sz, sv("888"));
    const size_t four_byte_fail
        = sv_rfind(haystack_view, haystack_view.sz, sv("1738"));
    const size_t needle_fail
        = sv_rfind(haystack_view, haystack_view.sz, sv("this is a failure"));
    const str_view one_byte_fail_rsvsv = sv_rsvsv(haystack_view, sv("J"));
    const str_view two_byte_fail_rsvsv = sv_rsvsv(haystack_view, sv("ZZ"));
    const str_view three_byte_fail_rsvsv = sv_rsvsv(haystack_view, sv("888"));
    const str_view four_byte_fail_rsvsv = sv_rsvsv(haystack_view, sv("1738"));
    const str_view needle_fail_rsvsv
        = sv_rsvsv(haystack_view, sv("this is a failure"));
    if (one_byte_fail != haystack_view.sz || two_byte_fail != haystack_view.sz
        || three_byte_fail != haystack_view.sz
        || four_byte_fail != haystack_view.sz || needle_fail != haystack_view.sz
        || !sv_empty(one_byte_fail_rsvsv) || !sv_empty(two_byte_fail_rsvsv)
        || !sv_empty(three_byte_fail_rsvsv) || !sv_empty(four_byte_fail_rsvsv)
        || !sv_empty(needle_fail_rsvsv))
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_rfind_off_by_one(void)
{
    const char *one_byte_needle = "Z";
    const char *two_byte_needle = "BB";
    const char *three_byte_needle = "DCC";
    const char *four_byte_needle = "YDDD";
    const char *needle = "Zind the needle!";
    const char *const haystack = "DDDD++CCC+++AB+++A+++find the needle!+++";
    const str_view haystack_view = sv(haystack);
    const size_t one_byte_pos
        = sv_rfind(haystack_view, haystack_view.sz, sv(one_byte_needle));
    const size_t two_byte_pos
        = sv_rfind(haystack_view, haystack_view.sz, sv(two_byte_needle));
    const size_t three_byte_pos
        = sv_rfind(haystack_view, haystack_view.sz, sv(three_byte_needle));
    const size_t four_byte_pos
        = sv_rfind(haystack_view, haystack_view.sz, sv(four_byte_needle));
    const size_t needle_pos
        = sv_rfind(haystack_view, haystack_view.sz, sv(needle));
    if (one_byte_pos != haystack_view.sz || two_byte_pos != haystack_view.sz
        || three_byte_pos != haystack_view.sz
        || four_byte_pos != haystack_view.sz || needle_pos != haystack_view.sz)
    {
        return FAIL;
    }
    const char *const haystack2 = "this entire string should be a match";
    const char *const needle2 = "this entire string should be a match";
    if (sv_rfind(sv(haystack2), sv_strlen(haystack2), sv(needle2)) != 0)
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_find_rfind_memoization(void)
{
    const char *needle_forward = "aabbaabba";
    const char *needle_backward = "abbaabbaa";
    const char *const haystack
        = "forward border aabbaabba backward border abbaabbaa!";
    const char *forward_found = strstr(haystack, needle_forward);
    const char *backward_found = strstr(haystack, needle_backward);
    const str_view haystack_view = sv(haystack);
    const size_t forward_needle_pos
        = sv_find(haystack_view, 0, sv(needle_forward));
    const size_t backward_needle_pos
        = sv_rfind(haystack_view, haystack_view.sz, sv(needle_backward));
    if (forward_needle_pos != (size_t)(forward_found - haystack)
        || backward_needle_pos != (size_t)(backward_found - haystack))
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_substring_off_by_one(void)
{
    const char *needle = "needle";
    const size_t needle_len = sv_strlen(needle);
    const char *const haystack = "needle_haystackhaystackhaystack_needle";
    const str_view haystack_view = sv(haystack);
    const str_view needle_view = sv(needle);
    const char *ref = strstr(haystack, needle);
    const str_view found_first = sv_svsv(haystack_view, needle_view);
    const size_t find_pos = sv_find(haystack_view, 0, needle_view);
    if (sv_strcmp(found_first, needle) != 0 || sv_begin(found_first) != ref
        || find_pos != (size_t)(ref - haystack))
    {
        return FAIL;
    }
    const char *ref2 = strstr(haystack + needle_len, needle);
    const str_view found_second = sv_svsv(
        sv_substr(haystack_view, needle_len, ULLONG_MAX), needle_view);
    const size_t find_pos2 = sv_find(
        sv_substr(haystack_view, needle_len, ULLONG_MAX), 0, needle_view);
    const size_t find_pos2_rev
        = sv_rfind(haystack_view, sv_svlen(haystack_view), needle_view);
    if (sv_strcmp(found_second, needle) != 0 || sv_begin(found_second) != ref2
        || find_pos2 != (size_t)(ref2 - (haystack + needle_len))
        || (size_t)(ref2 - haystack) != find_pos2_rev)
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_substring_search(void)
{
    const char *needle = "needle";
    const size_t needle_len = sv_strlen(needle);
    const char *const haystack
        = "haystackhaystackhaystackhaystackhaystackhaystackhaystackhaystack"
          "haystackhaystackhaystackhaystackhaystackhaystack--------___---**"
          "haystackhaystackhaystackhaystackhaystackhaystack\n\n\n\n\n\n\n\n"
          "neeedleneeddleneedlaneeeeeeeeeeeeeedlenedlennnnnnnnnneeeedneeddl"
          "_______________________needle___________________________________"
          "neeedleneeddleneedlaneeeeeeeeeeeeeedlenedlennneeeeeeeeeeedneeddl"
          "haystackhaystackhaystackhaystackhaystackhaystackhaystack__needle";
    const str_view haystack_view = sv(haystack);
    const str_view needle_view = sv(needle);
    const char *a = strstr(haystack, needle);
    if (!a)
    {
        printf("clibrary strstr failed?\n");
        return FAIL;
    }

    str_view b = sv_n(a, needle_len);
    str_view c = sv_svsv(haystack_view, needle_view);

    if (sv_svcmp(b, c) != EQL || c.s != a)
    {
        return FAIL;
    }
    a += needle_len;
    a = strstr(a, needle);
    if (!a)
    {
        printf("clibrary strstr failed?\n");
        return FAIL;
    }
    const str_view new_haystack_view = sv(a);
    b = sv_n(a, needle_len);
    c = sv_svsv(new_haystack_view, needle_view);
    if (sv_svcmp(b, c) != 0 || c.s != a)
    {
        return FAIL;
    }
    /* There are two needles so we get two string chunks chunks. */
    size_t i = 0;
    for (str_view v = sv_begin_tok(haystack_view, sv("needle"));
         !sv_end_tok(haystack_view, v);
         v = sv_next_tok(haystack_view, v, sv("needle")))
    {
        ++i;
    }
    if (i != 2)
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_rsubstring_search(void)
{
    const char *needle = "needle";
    const char *const haystack
        = "needle___khaystackhaystackhaystackhaystackhaystackhaystackhaystack"
          "haystackhaystackhaystackhaystackhaystackhaystack--------___---**"
          "haystackhaystackhaystackhaystackhaystackhaystack\n\n\n\n\n\n\n\n"
          "neeedleneeddleneedlaneeeeeeeeeeeeeedlenedlennnnnnnnnneeeedneeddl"
          "_______________________needle___________________________________"
          "neeedleneeddleneedlaneeeeeeeeeeeeeedlenedlennneeeeeeeeeeedneeddl"
          "haystackhaystackhaystackhaystackhaystackhaystackhaystack";
    const str_view haystack_view = sv(haystack);
    const str_view needle_view = sv(needle);
    const char *middle = strstr(haystack + 1, needle);
    const char *begin = strstr(haystack, needle);
    if (!middle || !begin || begin == middle)
    {
        printf("clibrary strstr failed?\n");
        return FAIL;
    }
    const str_view middle_needle = sv_rsvsv(haystack_view, needle_view);
    const size_t middle_pos
        = sv_rfind(haystack_view, haystack_view.sz, needle_view);
    if (sv_svcmp(middle_needle, needle_view) != EQL || middle_needle.s != middle
        || middle_pos != (size_t)(middle - haystack))
    {
        return FAIL;
    }
    const str_view first_chunk_view = sv_n(haystack, middle_pos);
    const str_view begin_needle = sv_rsvsv(first_chunk_view, needle_view);
    const size_t begin_pos
        = sv_rfind(first_chunk_view, first_chunk_view.sz, needle_view);
    if (sv_svcmp(begin_needle, needle_view) != EQL || begin_needle.s != begin
        || begin_pos != (size_t)(begin - haystack))
    {
        return FAIL;
    }
    return PASS;
}
