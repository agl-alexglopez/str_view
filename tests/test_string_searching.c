#include "str_view.h"
#include "test.h"

#include <limits.h>
#include <stdbool.h>
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
static enum test_result test_long_substring(void);

#define NUM_TESTS (size_t)11
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
    {test_long_substring, "test_long_substring"},
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
            (void)fprintf(stderr,
                          RED "test_string_searching.c test failed: " CYAN
                              "%s\n" NONE,
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
    CHECK(sv_find(str, 0, sv("C")), 2UL, size_t, "%zu");
    CHECK(sv_find(str, 0, sv("")), 19UL, size_t, "%zu");
    CHECK(sv_find(str, 0, sv("_")), 11UL, size_t, "%zu");
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
    CHECK(sv_rfind(str, sv_len(str), sv("!")), 16UL, size_t, "%zu");
    CHECK(sv_rfind(str, sv_len(str), sv("Y")), 0UL, size_t, "%zu");
    CHECK(sv_rfind(str, sv_len(str), sv("X")), 19UL, size_t, "%zu");
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
    CHECK(sv_find_first_of(str, sv("CB!")), 2UL, size_t, "%zu");
    CHECK(sv_find_first_of(str, sv("")), 24UL, size_t, "%zu");
    CHECK(sv_find_last_of(str, sv("! _")), 21UL, size_t, "%zu");
    CHECK(sv_find_last_not_of(str, sv("CBA!")), 22UL, size_t, "%zu");
    CHECK(sv_find_first_not_of(str, sv("ACB!;:, *.")), 14UL, size_t, "%zu");
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
    const str_view haystack_view = sv(haystack);

    const str_view one_byte_view = sv_match(haystack_view, sv(one_byte_needle));
    CHECK(sv_strcmp(one_byte_view, one_byte_needle), EQL, sv_threeway_cmp,
          "%d");
    const str_view two_byte_view = sv_match(haystack_view, sv(two_byte_needle));
    CHECK(sv_strcmp(two_byte_view, two_byte_needle), EQL, sv_threeway_cmp,
          "%d");
    const str_view three_byte_view
        = sv_match(haystack_view, sv(three_byte_needle));
    CHECK(sv_strcmp(three_byte_view, three_byte_needle), EQL, sv_threeway_cmp,
          "%d");
    const str_view four_byte_view
        = sv_match(haystack_view, sv(four_byte_needle));
    CHECK(sv_strcmp(four_byte_view, four_byte_needle), EQL, sv_threeway_cmp,
          "%d");
    const str_view needle_view = sv_match(haystack_view, sv(needle));
    CHECK(sv_strcmp(needle_view, needle), EQL, sv_threeway_cmp, "%d");
    const char *one_byte_found = strstr(haystack, one_byte_needle);
    CHECK(one_byte_found, sv_begin(one_byte_view), char *const, "%s");
    const char *two_byte_found = strstr(haystack, two_byte_needle);
    CHECK(two_byte_found, sv_begin(two_byte_view), char *const, "%s");
    const char *three_byte_found = strstr(haystack, three_byte_needle);
    CHECK(three_byte_found, sv_begin(three_byte_view), char *const, "%s");
    const char *four_byte_found = strstr(haystack, four_byte_needle);
    CHECK(four_byte_found, sv_begin(four_byte_view), char *const, "%s");
    const char *needle_found = strstr(haystack, needle);
    CHECK(needle_found, sv_begin(needle_view), char *const, "%s");
    const str_view one_byte_fail = sv_match(haystack_view, sv("J"));
    CHECK(sv_len(one_byte_fail), 0UL, size_t, "%zu");
    const str_view two_byte_fail = sv_match(haystack_view, sv("XY"));
    CHECK(sv_len(two_byte_fail), 0UL, size_t, "%zu");
    const str_view three_byte_fail = sv_match(haystack_view, sv("ZZY"));
    CHECK(sv_len(three_byte_fail), 0UL, size_t, "%zu");
    const str_view four_byte_fail = sv_match(haystack_view, sv("8888"));
    CHECK(sv_len(four_byte_fail), 0UL, size_t, "%zu");
    const str_view needle_fail = sv_match(haystack_view, sv("this is failure"));
    CHECK(sv_len(needle_fail), 0UL, size_t, "%zu");
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
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(one_byte_needle));
    CHECK(one_byte_pos, (size_t)(one_byte_found - haystack), size_t, "%zu");
    const size_t two_byte_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(two_byte_needle));
    CHECK(two_byte_pos, (size_t)(two_byte_found - haystack), size_t, "%zu");
    const size_t three_byte_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(three_byte_needle));
    CHECK(three_byte_pos, (size_t)(three_byte_found - haystack), size_t, "%zu");
    const size_t four_byte_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(four_byte_needle));
    CHECK(four_byte_pos, (size_t)(four_byte_found - haystack), size_t, "%zu");
    const size_t needle_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(needle));
    CHECK(needle_pos, (size_t)(needle_found - haystack), size_t, "%zu");

    const str_view one_byte_rsvsv
        = sv_rmatch(haystack_view, sv(one_byte_needle));
    CHECK(sv_strcmp(one_byte_rsvsv, one_byte_needle), EQL, sv_threeway_cmp,
          "%d");
    const str_view two_byte_rsvsv
        = sv_rmatch(haystack_view, sv(two_byte_needle));
    CHECK(sv_strcmp(two_byte_rsvsv, two_byte_needle), EQL, sv_threeway_cmp,
          "%d");
    const str_view three_byte_rsvsv
        = sv_rmatch(haystack_view, sv(three_byte_needle));
    CHECK(sv_strcmp(three_byte_rsvsv, three_byte_needle), EQL, sv_threeway_cmp,
          "%d");
    const str_view four_byte_rsvsv
        = sv_rmatch(haystack_view, sv(four_byte_needle));
    CHECK(sv_strcmp(four_byte_rsvsv, four_byte_needle), EQL, sv_threeway_cmp,
          "%d");
    const str_view needle_rsvsv = sv_rmatch(haystack_view, sv(needle));
    CHECK(sv_strcmp(needle_rsvsv, needle), EQL, sv_threeway_cmp, "%d");

    const size_t one_byte_fail
        = sv_rfind(haystack_view, sv_len(haystack_view), sv("J"));
    CHECK(one_byte_fail, sv_len(haystack_view), size_t, "%zu");
    const size_t two_byte_fail
        = sv_rfind(haystack_view, sv_len(haystack_view), sv("ZZ"));
    CHECK(two_byte_fail, sv_len(haystack_view), size_t, "%zu");
    const size_t three_byte_fail
        = sv_rfind(haystack_view, sv_len(haystack_view), sv("888"));
    CHECK(three_byte_fail, sv_len(haystack_view), size_t, "%zu");
    const size_t four_byte_fail
        = sv_rfind(haystack_view, sv_len(haystack_view), sv("1738"));
    CHECK(four_byte_fail, sv_len(haystack_view), size_t, "%zu");
    const size_t needle_fail = sv_rfind(haystack_view, sv_len(haystack_view),
                                        sv("this is a failure"));
    CHECK(needle_fail, sv_len(haystack_view), size_t, "%zu");
    const str_view one_byte_fail_rsvsv = sv_rmatch(haystack_view, sv("J"));
    CHECK(sv_empty(one_byte_fail_rsvsv), true, bool, "%b");
    const str_view two_byte_fail_rsvsv = sv_rmatch(haystack_view, sv("ZZ"));
    CHECK(sv_empty(two_byte_fail_rsvsv), true, bool, "%b");
    const str_view three_byte_fail_rsvsv = sv_rmatch(haystack_view, sv("888"));
    CHECK(sv_empty(three_byte_fail_rsvsv), true, bool, "%b");
    const str_view four_byte_fail_rsvsv = sv_rmatch(haystack_view, sv("1738"));
    CHECK(sv_empty(four_byte_fail_rsvsv), true, bool, "%b");
    const str_view needle_fail_rsvsv
        = sv_rmatch(haystack_view, sv("this is a failure"));
    CHECK(sv_empty(needle_fail_rsvsv), true, bool, "%b");
    return PASS;
}

static enum test_result
test_rfind_off_by_one(void)
{
    const char *one_byte_needle = "Z";
    CHECK(sv_rfind(sv(one_byte_needle), 1, sv("Z")), 0UL, size_t, "%zu");
    CHECK(sv_rfind(sv(one_byte_needle), 1, sv("A")), 1UL, size_t, "%zu");
    const char *two_byte_needle = "BB";
    CHECK(sv_rfind(sv(two_byte_needle), 2, sv("BB")), 0UL, size_t, "%zu");
    CHECK(sv_rfind(sv(two_byte_needle), 2, sv("AB")), 2UL, size_t, "%zu");
    CHECK(sv_rfind(sv(two_byte_needle), 2, sv("BA")), 2UL, size_t, "%zu");
    const char *three_byte_needle = "DCC";
    CHECK(sv_rfind(sv(three_byte_needle), 3, sv("DCC")), 0UL, size_t, "%zu");
    CHECK(sv_rfind(sv(three_byte_needle), 3, sv("ACC")), 3UL, size_t, "%zu");
    CHECK(sv_rfind(sv(three_byte_needle), 3, sv("DAC")), 3UL, size_t, "%zu");
    CHECK(sv_rfind(sv(three_byte_needle), 3, sv("DCA")), 3UL, size_t, "%zu");
    const char *four_byte_needle = "YDDD";
    CHECK(sv_rfind(sv(four_byte_needle), 4, sv("YDDD")), 0UL, size_t, "%zu");
    CHECK(sv_rfind(sv(four_byte_needle), 4, sv("ADDD")), 4UL, size_t, "%zu");
    CHECK(sv_rfind(sv(four_byte_needle), 4, sv("YDBD")), 4UL, size_t, "%zu");
    CHECK(sv_rfind(sv(four_byte_needle), 4, sv("YDDA")), 4UL, size_t, "%zu");
    const char *needle = "Zind the needle!";
    CHECK(sv_rfind(sv(needle), strlen(needle), sv(needle)), 0UL, size_t, "%zu");
    const char *const haystack = "DDDD++CCC+++AB+++A+++find the needle!+++";
    const str_view haystack_view = sv(haystack);
    const size_t one_byte_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(one_byte_needle));
    CHECK(one_byte_pos, sv_len(haystack_view), size_t, "%zu");
    const size_t two_byte_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(two_byte_needle));
    CHECK(two_byte_pos, sv_len(haystack_view), size_t, "%zu");
    const size_t three_byte_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(three_byte_needle));
    CHECK(three_byte_pos, sv_len(haystack_view), size_t, "%zu");
    const size_t four_byte_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(four_byte_needle));
    CHECK(four_byte_pos, sv_len(haystack_view), size_t, "%zu");
    const size_t needle_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(needle));
    CHECK(needle_pos, sv_len(haystack_view), size_t, "%zu");
    const char *const haystack2 = "this entire string should be a match";
    const char *const needle2 = "this entire string should be a match";
    CHECK(sv_rfind(sv(haystack2), strlen(haystack2), sv(needle2)), 0UL, size_t,
          "%zu");
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
    CHECK(forward_needle_pos, (size_t)(forward_found - haystack), size_t,
          "%zu");
    const size_t backward_needle_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(needle_backward));
    CHECK(backward_needle_pos, (size_t)(backward_found - haystack), size_t,
          "%zu");
    return PASS;
}

static enum test_result
test_substring_off_by_one(void)
{
    const char *needle = "needle";
    const size_t needle_len = strlen(needle);
    const char *const haystack = "needle_haystackhaystackhaystack_needle";
    const str_view haystack_view = sv(haystack);
    const str_view needle_view = sv(needle);
    const char *ref = strstr(haystack, needle);
    const str_view found_first = sv_match(haystack_view, needle_view);
    CHECK(sv_strcmp(found_first, needle), EQL, sv_threeway_cmp, "%d");
    CHECK(sv_begin(found_first), ref, char *const, "%s");

    const size_t find_pos = sv_find(haystack_view, 0, needle_view);
    CHECK(find_pos, (size_t)(ref - haystack), size_t, "%zu");

    const char *ref2 = strstr(haystack + needle_len, needle);
    const str_view found_second = sv_match(
        sv_substr(haystack_view, needle_len, ULLONG_MAX), needle_view);
    CHECK(sv_begin(found_second), ref2, char *const, "%s");
    CHECK(sv_strcmp(found_second, needle), EQL, sv_threeway_cmp, "%d");

    const size_t find_pos2 = sv_find(
        sv_substr(haystack_view, needle_len, ULLONG_MAX), 0, needle_view);
    CHECK(find_pos2, (size_t)(ref2 - (haystack + needle_len)), size_t, "%zu");
    const size_t find_pos2_rev
        = sv_rfind(haystack_view, sv_len(haystack_view), needle_view);
    CHECK((size_t)(ref2 - haystack), find_pos2_rev, size_t, "%zu");
    return PASS;
}

static enum test_result
test_substring_search(void)
{
    const char *needle = "needle";
    const size_t needle_len = strlen(needle);
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

    str_view b = sv_n(needle_len, a);
    str_view c = sv_match(haystack_view, needle_view);

    CHECK(sv_cmp(b, c), EQL, sv_threeway_cmp, "%d");
    CHECK(sv_begin(c), a, char *const, "%s");
    a += needle_len;
    a = strstr(a, needle);
    if (!a)
    {
        printf("clibrary strstr failed?\n");
        return FAIL;
    }
    const str_view new_haystack_view = sv(a);
    b = sv_n(needle_len, a);
    c = sv_match(new_haystack_view, needle_view);
    CHECK(sv_cmp(b, c), EQL, sv_threeway_cmp, "%d");
    CHECK(sv_begin(c), a, char *const, "%s");
    const str_view first_chunk
        = sv_substr(haystack_view, 0, sv_find(haystack_view, 0, needle_view));
    const str_view remaining_string
        = sv(sv_begin(first_chunk) + sv_len(first_chunk) + needle_len);
    const str_view second_chunk = sv_substr(
        remaining_string, 0, sv_find(remaining_string, 0, needle_view));
    /* There are two needles so we get two string chunks chunks. */
    size_t i = 0;
    for (str_view v = sv_begin_tok(haystack_view, needle_view);
         !sv_end_tok(haystack_view, v);
         v = sv_next_tok(haystack_view, v, needle_view))
    {
        if (i == 0)
        {
            CHECK(sv_cmp(v, first_chunk), EQL, sv_threeway_cmp, "%d");
        }
        else
        {
            CHECK(sv_cmp(v, second_chunk), EQL, sv_threeway_cmp, "%d");
        }
        ++i;
    }
    CHECK(i, 2UL, size_t, "%zu");
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
        return ERROR;
    }
    const str_view middle_needle = sv_rmatch(haystack_view, needle_view);
    const size_t middle_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), needle_view);
    CHECK(sv_cmp(middle_needle, needle_view), EQL, sv_threeway_cmp, "%d");
    CHECK(sv_begin(middle_needle), middle, char *const, "%s");
    CHECK(middle_pos, (size_t)(middle - haystack), size_t, "%zu");
    const str_view first_chunk_view = sv_n(middle_pos, haystack);
    const str_view begin_needle = sv_rmatch(first_chunk_view, needle_view);
    const size_t begin_pos
        = sv_rfind(first_chunk_view, sv_len(first_chunk_view), needle_view);
    CHECK(sv_cmp(begin_needle, needle_view), EQL, sv_threeway_cmp, "%d");
    CHECK(sv_begin(begin_needle), begin, char *const, "%s");
    CHECK(begin_pos, (size_t)(begin - haystack), size_t, "%zu");
    return PASS;
}

static enum test_result
test_long_substring(void)
{
    const char *needle = "This needle will make up most of the string such "
                         "that the two-way string searching algorithm has to "
                         "continue for many iterations during a match.";
    const char *const haystack
        = "Here is the string containing the longer needle. This needle will "
          "make up most of the string such that the two-way string searching "
          "algorithm has to continue for many iterations during a match. There "
          "went the needle.";
    const char *strstr_needle = strstr(haystack, needle);
    const str_view haystack_view = sv(haystack);
    const str_view needle_view = sv(needle);
    const str_view svsv_needle = sv_match(haystack_view, needle_view);
    CHECK(sv_begin(svsv_needle), strstr_needle, char *const, "%s");
    const size_t find_pos = sv_find(haystack_view, 0, needle_view);
    CHECK(find_pos, (size_t)(strstr_needle - haystack), size_t, "%zu");
    const str_view rsvsv_needle = sv_rmatch(haystack_view, needle_view);
    CHECK(sv_begin(rsvsv_needle), strstr_needle, char *const, "%s");
    const size_t rfind_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), needle_view);
    CHECK(rfind_pos, (size_t)(strstr_needle - haystack), size_t, "%zu");
    CHECK(sv_cmp(svsv_needle, rsvsv_needle), EQL, sv_threeway_cmp, "%d");
    return PASS;
}
