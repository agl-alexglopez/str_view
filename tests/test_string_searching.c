#include "str_view.h"
#include "test.h"
#include <string.h>

static enum test_result test_find_rfind(void);
static enum test_result test_find_of_sets(void);
static enum test_result test_substring_brute_force(void);
static enum test_result test_substring_off_by_one(void);
static enum test_result test_substring_search(void);

#define NUM_TESTS (size_t)5
const struct fn_name all_tests[NUM_TESTS] = {
    {test_find_rfind, "test_find_rfind"},
    {test_find_of_sets, "test_find_of_sets"},
    {test_substring_brute_force, "test_substring_brute_force"},
    {test_substring_off_by_one, "test_substring_off_by_one"},
    {test_substring_search, "test_substring_search"},
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
test_find_rfind(void)
{
    const char ref[20] = {
        [0] = 'A',  [1] = 'A',  [2] = 'C',  [3] = ' ',  [4] = '!',
        [5] = '!',  [6] = '!',  [7] = ' ',  [8] = '*',  [9] = '*',
        [10] = ' ', [11] = '_', [12] = '_', [13] = ' ', [14] = '!',
        [15] = '!', [16] = '!', [17] = ' ', [18] = 'A', [19] = '\0',
    };
    str_view str = sv(ref);
    if (sv_find(str, 0,
                (str_view){
                    .s = "C",
                    .sz = 1,
                })
        != 2)
    {
        return FAIL;
    }
    if (sv_find(str, 0,
                (str_view){
                    .s = "",
                    .sz = 1,
                })
        != 19)
    {
        return FAIL;
    }
    if (sv_rfind(str, str.sz,
                 (str_view){
                     .s = "!",
                     .sz = 1,
                 })
        != 16)
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
    if (sv_find_first_of(str,
                         (str_view){
                             .s = "CB!",
                             .sz = sv_strlen("CB!"),
                         })
        != 2)
    {
        return FAIL;
    }
    if (sv_find_first_of(str,
                         (str_view){
                             .s = "",
                             .sz = 0,
                         })
        != 24)
    {
        return FAIL;
    }
    if (sv_find_last_of(str,
                        (str_view){
                            .s = "! _",
                            .sz = sv_strlen("! _"),
                        })
        != 21)
    {
        return FAIL;
    }
    if (sv_find_last_not_of(str,
                            (str_view){
                                .s = "CBA!",
                                .sz = sv_strlen("CBA!"),
                            })
        != 22)
    {
        return FAIL;
    }
    if (sv_find_first_not_of(str,
                             (str_view){
                                 .s = "ACB!;:, *.",
                                 .sz = sv_strlen("ACB!;:, *."),
                             })
        != 14)
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
    const str_view found_second
        = sv_svsv(sv_substr(haystack_view, needle_len, 99), needle_view);
    const size_t find_pos2
        = sv_find(sv_substr(haystack_view, needle_len, 99), 0, needle_view);
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
    for (str_view v
         = sv_begin_tok(haystack_view, (str_view){"needle", needle_len});
         !sv_end_tok(v); v = sv_next_tok(v, (str_view){"needle", needle_len}))
    {
        ++i;
    }
    if (i != 2)
    {
        return FAIL;
    }
    return PASS;
}
