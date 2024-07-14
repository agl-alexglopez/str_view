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
static enum test_result test_consecutive_find(void);
static enum test_result test_consecutive_rfind(void);
static enum test_result test_substring_off_by_one(void);
static enum test_result test_substring_search(void);
static enum test_result test_rsubstring_search(void);
static enum test_result test_long_substring(void);

#define NUM_TESTS (size_t)13

static test_fn const all_tests[NUM_TESTS] = {
    test_small_find,
    test_small_rfind,
    test_find_of_sets,
    test_substring_brute_force,
    test_rfind_brute_force,
    test_rfind_off_by_one,
    test_find_rfind_memoization,
    test_consecutive_find,
    test_consecutive_rfind,
    test_substring_off_by_one,
    test_substring_search,
    test_rsubstring_search,
    test_long_substring,
};

int
main()
{
    enum test_result res = PASS;
    for (size_t i = 0; i < NUM_TESTS; ++i)
    {
        enum test_result const t_res = all_tests[i]();
        if (t_res == FAIL)
        {
            res = FAIL;
        }
    }
    return res;
}

static enum test_result
test_small_find(void)
{
    char const ref[20] = {
        [0] = 'A',  [1] = 'A',  [2] = 'C',  [3] = ' ',  [4] = '!',
        [5] = '!',  [6] = '!',  [7] = ' ',  [8] = '*',  [9] = '*',
        [10] = ' ', [11] = '_', [12] = '_', [13] = ' ', [14] = '!',
        [15] = '!', [16] = '!', [17] = ' ', [18] = 'A', [19] = '\0',
    };
    str_view str = sv(ref);
    CHECK(sv_find(str, 0, SV("C")), 2UL, size_t, "%zu");
    CHECK(sv_find(str, 0, SV("")), 19UL, size_t, "%zu");
    CHECK(sv_find(str, 0, SV("_")), 11UL, size_t, "%zu");
    return PASS;
}

static enum test_result
test_small_rfind(void)
{
    char const ref[20] = {
        [0] = 'Y',  [1] = 'A',  [2] = 'C',  [3] = ' ',  [4] = '!',
        [5] = '!',  [6] = '!',  [7] = ' ',  [8] = '*',  [9] = '*',
        [10] = ' ', [11] = '_', [12] = '_', [13] = ' ', [14] = '!',
        [15] = '!', [16] = '!', [17] = ' ', [18] = 'A', [19] = '\0',
    };
    str_view str = sv(ref);
    CHECK(sv_rfind(str, sv_len(str), SV("!")), 16UL, size_t, "%zu");
    CHECK(sv_rfind(str, sv_len(str), SV("Y")), 0UL, size_t, "%zu");
    CHECK(sv_rfind(str, sv_len(str), SV("X")), 19UL, size_t, "%zu");
    return PASS;
}

static enum test_result
test_find_of_sets(void)
{
    char const ref[25] = {
        [0] = 'A',  [1] = 'A',  [2] = 'C',  [3] = 'B',  [4] = '!',
        [5] = '!',  [6] = '!',  [7] = ' ',  [8] = '*',  [9] = '.',
        [10] = ':', [11] = ';', [12] = ',', [13] = ' ', [14] = '?',
        [15] = ' ', [16] = '_', [17] = '_', [18] = ' ', [19] = '!',
        [20] = '!', [21] = '!', [22] = 'Z', [23] = 'z', [24] = '\0',
    };
    str_view str = sv(ref);
    CHECK(sv_find_first_of(str, SV("CB!")), 2UL, size_t, "%zu");
    CHECK(sv_find_first_of(str, SV("")), 24UL, size_t, "%zu");
    CHECK(sv_find_last_of(str, SV("! _")), 21UL, size_t, "%zu");
    CHECK(sv_find_last_not_of(str, SV("CBA!")), 22UL, size_t, "%zu");
    CHECK(sv_find_first_not_of(str, SV("ACB!;:, *.")), 14UL, size_t, "%zu");
    return PASS;
}

static enum test_result
test_substring_brute_force(void)
{
    char const *one_byte_needle = "A";
    char const *two_byte_needle = "AA";
    char const *three_byte_needle = "AAA";
    char const *four_byte_needle = "AAAA";
    char const *needle = "find the needle!";
    char const *const haystack = "__A__AA___AAA___AAAA___find the needle!___";
    str_view const haystack_view = sv(haystack);

    str_view const one_byte_view = sv_match(haystack_view, sv(one_byte_needle));
    CHECK(sv_strcmp(one_byte_view, one_byte_needle), SV_EQL, sv_threeway_cmp,
          "%d");
    str_view const two_byte_view = sv_match(haystack_view, sv(two_byte_needle));
    CHECK(sv_strcmp(two_byte_view, two_byte_needle), SV_EQL, sv_threeway_cmp,
          "%d");
    str_view const three_byte_view
        = sv_match(haystack_view, sv(three_byte_needle));
    CHECK(sv_strcmp(three_byte_view, three_byte_needle), SV_EQL,
          sv_threeway_cmp, "%d");
    str_view const four_byte_view
        = sv_match(haystack_view, sv(four_byte_needle));
    CHECK(sv_strcmp(four_byte_view, four_byte_needle), SV_EQL, sv_threeway_cmp,
          "%d");
    str_view const needle_view = sv_match(haystack_view, sv(needle));
    CHECK(sv_strcmp(needle_view, needle), SV_EQL, sv_threeway_cmp, "%d");
    char const *one_byte_found = strstr(haystack, one_byte_needle);
    CHECK(one_byte_found, sv_begin(one_byte_view), char *const, "%s");
    char const *two_byte_found = strstr(haystack, two_byte_needle);
    CHECK(two_byte_found, sv_begin(two_byte_view), char *const, "%s");
    char const *three_byte_found = strstr(haystack, three_byte_needle);
    CHECK(three_byte_found, sv_begin(three_byte_view), char *const, "%s");
    char const *four_byte_found = strstr(haystack, four_byte_needle);
    CHECK(four_byte_found, sv_begin(four_byte_view), char *const, "%s");
    char const *needle_found = strstr(haystack, needle);
    CHECK(needle_found, sv_begin(needle_view), char *const, "%s");
    str_view const one_byte_fail = sv_match(haystack_view, SV("J"));
    CHECK(sv_len(one_byte_fail), 0UL, size_t, "%zu");
    str_view const two_byte_fail = sv_match(haystack_view, SV("XY"));
    CHECK(sv_len(two_byte_fail), 0UL, size_t, "%zu");
    str_view const three_byte_fail = sv_match(haystack_view, SV("ZZY"));
    CHECK(sv_len(three_byte_fail), 0UL, size_t, "%zu");
    str_view const four_byte_fail = sv_match(haystack_view, SV("8888"));
    CHECK(sv_len(four_byte_fail), 0UL, size_t, "%zu");
    str_view const needle_fail = sv_match(haystack_view, SV("this is failure"));
    CHECK(sv_len(needle_fail), 0UL, size_t, "%zu");
    return PASS;
}

static enum test_result
test_rfind_brute_force(void)
{
    char const *one_byte_needle = "A";
    char const *two_byte_needle = "BB";
    char const *three_byte_needle = "CCC";
    char const *four_byte_needle = "DDDD";
    char const *needle = "find the needle!";
    char const *const haystack = "++DDDD++CCC+++BB+++A+++find the needle!+++";
    char const *one_byte_found = strstr(haystack, one_byte_needle);
    char const *two_byte_found = strstr(haystack, two_byte_needle);
    char const *three_byte_found = strstr(haystack, three_byte_needle);
    char const *four_byte_found = strstr(haystack, four_byte_needle);
    char const *needle_found = strstr(haystack, needle);
    str_view const haystack_view = sv(haystack);
    size_t const one_byte_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(one_byte_needle));
    CHECK(one_byte_pos, (size_t)(one_byte_found - haystack), size_t, "%zu");
    size_t const two_byte_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(two_byte_needle));
    CHECK(two_byte_pos, (size_t)(two_byte_found - haystack), size_t, "%zu");
    size_t const three_byte_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(three_byte_needle));
    CHECK(three_byte_pos, (size_t)(three_byte_found - haystack), size_t, "%zu");
    size_t const four_byte_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(four_byte_needle));
    CHECK(four_byte_pos, (size_t)(four_byte_found - haystack), size_t, "%zu");
    size_t const needle_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(needle));
    CHECK(needle_pos, (size_t)(needle_found - haystack), size_t, "%zu");

    str_view const one_byte_rsvsv
        = sv_rmatch(haystack_view, sv(one_byte_needle));
    CHECK(sv_strcmp(one_byte_rsvsv, one_byte_needle), SV_EQL, sv_threeway_cmp,
          "%d");
    str_view const two_byte_rsvsv
        = sv_rmatch(haystack_view, sv(two_byte_needle));
    CHECK(sv_strcmp(two_byte_rsvsv, two_byte_needle), SV_EQL, sv_threeway_cmp,
          "%d");
    str_view const three_byte_rsvsv
        = sv_rmatch(haystack_view, sv(three_byte_needle));
    CHECK(sv_strcmp(three_byte_rsvsv, three_byte_needle), SV_EQL,
          sv_threeway_cmp, "%d");
    str_view const four_byte_rsvsv
        = sv_rmatch(haystack_view, sv(four_byte_needle));
    CHECK(sv_strcmp(four_byte_rsvsv, four_byte_needle), SV_EQL, sv_threeway_cmp,
          "%d");
    str_view const needle_rsvsv = sv_rmatch(haystack_view, sv(needle));
    CHECK(sv_strcmp(needle_rsvsv, needle), SV_EQL, sv_threeway_cmp, "%d");

    size_t const one_byte_fail
        = sv_rfind(haystack_view, sv_len(haystack_view), SV("J"));
    CHECK(one_byte_fail, sv_len(haystack_view), size_t, "%zu");
    size_t const two_byte_fail
        = sv_rfind(haystack_view, sv_len(haystack_view), SV("ZZ"));
    CHECK(two_byte_fail, sv_len(haystack_view), size_t, "%zu");
    size_t const three_byte_fail
        = sv_rfind(haystack_view, sv_len(haystack_view), SV("888"));
    CHECK(three_byte_fail, sv_len(haystack_view), size_t, "%zu");
    size_t const four_byte_fail
        = sv_rfind(haystack_view, sv_len(haystack_view), SV("1738"));
    CHECK(four_byte_fail, sv_len(haystack_view), size_t, "%zu");
    size_t const needle_fail = sv_rfind(haystack_view, sv_len(haystack_view),
                                        SV("this is a failure"));
    CHECK(needle_fail, sv_len(haystack_view), size_t, "%zu");
    str_view const one_byte_fail_rsvsv = sv_rmatch(haystack_view, SV("J"));
    CHECK(sv_empty(one_byte_fail_rsvsv), true, bool, "%d");
    str_view const two_byte_fail_rsvsv = sv_rmatch(haystack_view, SV("ZZ"));
    CHECK(sv_empty(two_byte_fail_rsvsv), true, bool, "%d");
    str_view const three_byte_fail_rsvsv = sv_rmatch(haystack_view, SV("888"));
    CHECK(sv_empty(three_byte_fail_rsvsv), true, bool, "%d");
    str_view const four_byte_fail_rsvsv = sv_rmatch(haystack_view, SV("1738"));
    CHECK(sv_empty(four_byte_fail_rsvsv), true, bool, "%d");
    str_view const needle_fail_rsvsv
        = sv_rmatch(haystack_view, SV("this is a failure"));
    CHECK(sv_empty(needle_fail_rsvsv), true, bool, "%d");
    return PASS;
}

static enum test_result
test_consecutive_find(void)
{
    char const needles[13] = {
        [0] = 'a',  [1] = 'a',  [2] = 'a',   [3] = 'Z', [4] = 'a',
        [5] = 'a',  [6] = 'Z',  [7] = 'a',   [8] = 'a', [9] = 'a',
        [10] = 'a', [11] = 'Z', [12] = '\0',
    };
    size_t const found_positions[3] = {3, 6, 11};
    size_t const size = sizeof(found_positions) / sizeof(found_positions[0]);
    str_view const hay = sv(needles);
    str_view const needle = SV("Z");
    size_t pos = 0;
    size_t i = 0;
    bool found = false;
    while (i < size && (pos = sv_find(hay, pos, needle)) != sv_npos(hay))
    {
        found = true;
        CHECK(pos, found_positions[i], size_t, "%zu");
        ++pos;
        ++i;
    }
    CHECK(found, true, bool, "%d");
    CHECK(i, size, size_t, "%zu");
    return PASS;
}

static enum test_result
test_consecutive_rfind(void)
{
    char const needles[13] = {
        [0] = 'a',  [1] = 'a',  [2] = 'a',   [3] = 'Z', [4] = 'a',
        [5] = 'a',  [6] = 'Z',  [7] = 'a',   [8] = 'a', [9] = 'a',
        [10] = 'a', [11] = 'Z', [12] = '\0',
    };
    size_t const found_positions[3] = {3, 6, 11};
    str_view const hay = sv(needles);
    str_view const needle = SV("Z");
    size_t pos = sv_len(hay);
    size_t i = sizeof(found_positions) / sizeof(found_positions[0]);
    bool found = false;
    while (i && (pos = sv_rfind(hay, pos, needle)) != sv_npos(hay))
    {
        --i;
        found = true;
        CHECK(pos, found_positions[i], size_t, "%zu");
        --pos;
    }
    CHECK(found, true, bool, "%d");
    CHECK(i, 0ULL, size_t, "%zu");
    return PASS;
}

static enum test_result
test_rfind_off_by_one(void)
{
    char const *one_byte_needle = "Z";
    CHECK(sv_rfind(sv(one_byte_needle), 1, SV("Z")), 0UL, size_t, "%zu");
    CHECK(sv_rfind(sv(one_byte_needle), 1, SV("A")), 1UL, size_t, "%zu");
    char const *two_byte_needle = "BB";
    CHECK(sv_rfind(sv(two_byte_needle), 2, SV("BB")), 0UL, size_t, "%zu");
    CHECK(sv_rfind(sv(two_byte_needle), 2, SV("AB")), 2UL, size_t, "%zu");
    CHECK(sv_rfind(sv(two_byte_needle), 2, SV("BA")), 2UL, size_t, "%zu");
    char const *three_byte_needle = "DCC";
    CHECK(sv_rfind(sv(three_byte_needle), 3, SV("DCC")), 0UL, size_t, "%zu");
    CHECK(sv_rfind(sv(three_byte_needle), 3, SV("ACC")), 3UL, size_t, "%zu");
    CHECK(sv_rfind(sv(three_byte_needle), 3, SV("DAC")), 3UL, size_t, "%zu");
    CHECK(sv_rfind(sv(three_byte_needle), 3, SV("DCA")), 3UL, size_t, "%zu");
    char const *four_byte_needle = "YDDD";
    CHECK(sv_rfind(sv(four_byte_needle), 4, SV("YDDD")), 0UL, size_t, "%zu");
    CHECK(sv_rfind(sv(four_byte_needle), 4, SV("ADDD")), 4UL, size_t, "%zu");
    CHECK(sv_rfind(sv(four_byte_needle), 4, SV("YDBD")), 4UL, size_t, "%zu");
    CHECK(sv_rfind(sv(four_byte_needle), 4, SV("YDDA")), 4UL, size_t, "%zu");
    char const *needle = "Zind the needle!";
    CHECK(sv_rfind(sv(needle), strlen(needle), sv(needle)), 0UL, size_t, "%zu");
    char const *const haystack = "DDDD++CCC+++AB+++A+++find the needle!+++";
    str_view const haystack_view = sv(haystack);
    size_t const one_byte_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(one_byte_needle));
    CHECK(one_byte_pos, sv_len(haystack_view), size_t, "%zu");
    size_t const two_byte_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(two_byte_needle));
    CHECK(two_byte_pos, sv_len(haystack_view), size_t, "%zu");
    size_t const three_byte_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(three_byte_needle));
    CHECK(three_byte_pos, sv_len(haystack_view), size_t, "%zu");
    size_t const four_byte_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(four_byte_needle));
    CHECK(four_byte_pos, sv_len(haystack_view), size_t, "%zu");
    size_t const needle_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(needle));
    CHECK(needle_pos, sv_len(haystack_view), size_t, "%zu");
    char const *const haystack2 = "this entire string should be a match";
    char const *const needle2 = "this entire string should be a match";
    CHECK(sv_rfind(sv(haystack2), strlen(haystack2), sv(needle2)), 0UL, size_t,
          "%zu");
    return PASS;
}

static enum test_result
test_find_rfind_memoization(void)
{
    char const *needle_forward = "aabbaabba";
    char const *needle_backward = "abbaabbaa";
    char const *const haystack
        = "forward border aabbaabba backward border abbaabbaa!";
    char const *forward_found = strstr(haystack, needle_forward);
    char const *backward_found = strstr(haystack, needle_backward);
    str_view const haystack_view = sv(haystack);

    size_t const forward_needle_pos
        = sv_find(haystack_view, 0, sv(needle_forward));
    CHECK(forward_needle_pos, (size_t)(forward_found - haystack), size_t,
          "%zu");
    size_t const backward_needle_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), sv(needle_backward));
    CHECK(backward_needle_pos, (size_t)(backward_found - haystack), size_t,
          "%zu");
    return PASS;
}

static enum test_result
test_substring_off_by_one(void)
{
    char const *needle = "needle";
    size_t const needle_len = strlen(needle);
    char const *const haystack = "needle_haystackhaystackhaystack_needle";
    str_view const haystack_view = sv(haystack);
    str_view const needle_view = sv(needle);
    char const *ref = strstr(haystack, needle);
    str_view const found_first = sv_match(haystack_view, needle_view);
    CHECK(sv_strcmp(found_first, needle), SV_EQL, sv_threeway_cmp, "%d");
    CHECK(sv_begin(found_first), ref, char *const, "%s");

    size_t const find_pos = sv_find(haystack_view, 0, needle_view);
    CHECK(find_pos, (size_t)(ref - haystack), size_t, "%zu");

    char const *ref2 = strstr(haystack + needle_len, needle);
    str_view const found_second = sv_match(
        sv_substr(haystack_view, needle_len, ULLONG_MAX), needle_view);
    CHECK(sv_begin(found_second), ref2, char *const, "%s");
    CHECK(sv_strcmp(found_second, needle), SV_EQL, sv_threeway_cmp, "%d");

    size_t const find_pos2 = sv_find(
        sv_substr(haystack_view, needle_len, ULLONG_MAX), 0, needle_view);
    CHECK(find_pos2, (size_t)(ref2 - (haystack + needle_len)), size_t, "%zu");
    size_t const find_pos2_rev
        = sv_rfind(haystack_view, sv_len(haystack_view), needle_view);
    CHECK((size_t)(ref2 - haystack), find_pos2_rev, size_t, "%zu");
    return PASS;
}

static enum test_result
test_substring_search(void)
{
    char const *needle = "needle";
    size_t const needle_len = strlen(needle);
    char const *const haystack
        = "haystackhaystackhaystackhaystackhaystackhaystackhaystackhaystack"
          "haystackhaystackhaystackhaystackhaystackhaystack--------___---**"
          "haystackhaystackhaystackhaystackhaystackhaystack\n\n\n\n\n\n\n\n"
          "neeedleneeddleneedlaneeeeeeeeeeeeeedlenedlennnnnnnnnneeeedneeddl"
          "_______________________needle___________________________________"
          "neeedleneeddleneedlaneeeeeeeeeeeeeedlenedlennneeeeeeeeeeedneeddl"
          "haystackhaystackhaystackhaystackhaystackhaystackhaystack__needle";
    str_view const haystack_view = sv(haystack);
    str_view const needle_view = sv(needle);
    char const *a = strstr(haystack, needle);
    if (!a)
    {
        printf("clibrary strstr failed?\n");
        return FAIL;
    }

    str_view b = sv_n(needle_len, a);
    str_view c = sv_match(haystack_view, needle_view);

    CHECK(sv_cmp(b, c), SV_EQL, sv_threeway_cmp, "%d");
    CHECK(sv_begin(c), a, char *const, "%s");
    a += needle_len;
    a = strstr(a, needle);
    if (!a)
    {
        printf("clibrary strstr failed?\n");
        return FAIL;
    }
    str_view const new_haystack_view = sv(a);
    b = sv_n(needle_len, a);
    c = sv_match(new_haystack_view, needle_view);
    CHECK(sv_cmp(b, c), SV_EQL, sv_threeway_cmp, "%d");
    CHECK(sv_begin(c), a, char *const, "%s");
    str_view const first_chunk
        = sv_substr(haystack_view, 0, sv_find(haystack_view, 0, needle_view));
    str_view const remaining_string
        = sv(sv_begin(first_chunk) + sv_len(first_chunk) + needle_len);
    str_view const second_chunk = sv_substr(
        remaining_string, 0, sv_find(remaining_string, 0, needle_view));
    /* There are two needles so we get two string chunks chunks. */
    size_t i = 0;
    for (str_view v = sv_begin_tok(haystack_view, needle_view);
         !sv_end_tok(haystack_view, v);
         v = sv_next_tok(haystack_view, v, needle_view))
    {
        if (i == 0)
        {
            CHECK(sv_cmp(v, first_chunk), SV_EQL, sv_threeway_cmp, "%d");
        }
        else
        {
            CHECK(sv_cmp(v, second_chunk), SV_EQL, sv_threeway_cmp, "%d");
        }
        ++i;
    }
    CHECK(i, 2UL, size_t, "%zu");
    return PASS;
}

static enum test_result
test_rsubstring_search(void)
{
    char const *needle = "needle";
    char const *const haystack
        = "needle___khaystackhaystackhaystackhaystackhaystackhaystackhaystack"
          "haystackhaystackhaystackhaystackhaystackhaystack--------___---**"
          "haystackhaystackhaystackhaystackhaystackhaystack\n\n\n\n\n\n\n\n"
          "neeedleneeddleneedlaneeeeeeeeeeeeeedlenedlennnnnnnnnneeeedneeddl"
          "_______________________needle___________________________________"
          "neeedleneeddleneedlaneeeeeeeeeeeeeedlenedlennneeeeeeeeeeedneeddl"
          "haystackhaystackhaystackhaystackhaystackhaystackhaystack";
    str_view const haystack_view = sv(haystack);
    str_view const needle_view = sv(needle);
    char const *middle = strstr(haystack + 1, needle);
    char const *begin = strstr(haystack, needle);
    if (!middle || !begin || begin == middle)
    {
        printf("clibrary strstr failed?\n");
        return ERROR;
    }
    str_view const middle_needle = sv_rmatch(haystack_view, needle_view);
    size_t const middle_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), needle_view);
    CHECK(sv_cmp(middle_needle, needle_view), SV_EQL, sv_threeway_cmp, "%d");
    CHECK(sv_begin(middle_needle), middle, char *const, "%s");
    CHECK(middle_pos, (size_t)(middle - haystack), size_t, "%zu");
    str_view const first_chunk_view = sv_n(middle_pos, haystack);
    str_view const begin_needle = sv_rmatch(first_chunk_view, needle_view);
    size_t const begin_pos
        = sv_rfind(first_chunk_view, sv_len(first_chunk_view), needle_view);
    CHECK(sv_cmp(begin_needle, needle_view), SV_EQL, sv_threeway_cmp, "%d");
    CHECK(sv_begin(begin_needle), begin, char *const, "%s");
    CHECK(begin_pos, (size_t)(begin - haystack), size_t, "%zu");
    return PASS;
}

static enum test_result
test_long_substring(void)
{
    char const *needle = "This needle will make up most of the string such "
                         "that the two-way string searching algorithm has to "
                         "continue for many iterations during a match.";
    char const *const haystack
        = "Here is the string containing the longer needle. This needle will "
          "make up most of the string such that the two-way string searching "
          "algorithm has to continue for many iterations during a match. There "
          "went the needle.";
    char const *strstr_needle = strstr(haystack, needle);
    str_view const haystack_view = sv(haystack);
    str_view const needle_view = sv(needle);
    str_view const svsv_needle = sv_match(haystack_view, needle_view);
    CHECK(sv_begin(svsv_needle), strstr_needle, char *const, "%s");
    size_t const find_pos = sv_find(haystack_view, 0, needle_view);
    CHECK(find_pos, (size_t)(strstr_needle - haystack), size_t, "%zu");
    str_view const rsvsv_needle = sv_rmatch(haystack_view, needle_view);
    CHECK(sv_begin(rsvsv_needle), strstr_needle, char *const, "%s");
    size_t const rfind_pos
        = sv_rfind(haystack_view, sv_len(haystack_view), needle_view);
    CHECK(rfind_pos, (size_t)(strstr_needle - haystack), size_t, "%zu");
    CHECK(sv_cmp(svsv_needle, rsvsv_needle), SV_EQL, sv_threeway_cmp, "%d");
    return PASS;
}
