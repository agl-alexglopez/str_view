#include "str_view.h"
#include "test.h"

#include <stdbool.h>
#include <string.h>

static enum test_result test_compare_single(void);
static enum test_result test_compare_equal(void);
static enum test_result test_compare_equal_view(void);
static enum test_result test_compare_view_equals_str(void);
static enum test_result test_compare_view_off_by_one(void);
static enum test_result test_compare_terminated(void);
static enum test_result test_compare_different_lengths_terminated(void);
static enum test_result test_compare_different_lengths_views(void);
static enum test_result test_compare_misc(void);

#define NUM_TESTS (size_t)9
static test_fn const all_tests[NUM_TESTS] = {
    test_compare_single,
    test_compare_equal,
    test_compare_equal_view,
    test_compare_terminated,
    test_compare_different_lengths_terminated,
    test_compare_view_equals_str,
    test_compare_view_off_by_one,
    test_compare_different_lengths_views,
    test_compare_misc,
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
test_compare_single(void)
{
    char const e1[2] = {
        [0] = 'A',
        [1] = '\0',
    };
    char const e2[2] = {
        [0] = 'B',
        [1] = '\0',
    };
    str_view const e1_view = sv(e1);
    str_view const e2_view = sv(e2);
    int const cmp_res = strcmp(e1, e2);
    int const cmp_res2 = strcmp(e2, e1);
    CHECK(cmp_res < 0, sv_strcmp(e1_view, e2) < 0, bool, "%d");
    CHECK(cmp_res < 0, sv_cmp(e1_view, e2_view) < 0, bool, "%d");
    CHECK(cmp_res2 > 0, sv_strcmp(e2_view, e1) > 0, bool, "%d");
    CHECK(cmp_res2 > 0, sv_cmp(e2_view, e1_view) > 0, bool, "%d");
    return PASS;
}

static enum test_result
test_compare_equal(void)
{
    char const e1[5] = {
        [0] = 'N', [1] = 'I', [2] = 'C', [3] = 'E', [4] = '\0',
    };
    char const e2[5] = {
        [0] = 'N', [1] = 'I', [2] = 'C', [3] = 'E', [4] = '\0',
    };
    str_view const e1_view = sv(e1);
    str_view const e2_view = sv(e2);
    int const cmp_res = strcmp(e1, e2);
    int const cmp_res2 = strcmp(e2, e1);
    CHECK(cmp_res == 0, sv_strcmp(e1_view, e2) == 0, bool, "%d");
    CHECK(cmp_res == 0, sv_cmp(e1_view, e2_view) == 0, bool, "%d");
    CHECK(cmp_res2 == 0, sv_strcmp(e2_view, e1) == 0, bool, "%d");
    CHECK(cmp_res2 == 0, sv_cmp(e2_view, e1_view) == 0, bool, "%d");
    return PASS;
}

static enum test_result
test_compare_equal_view(void)
{
    char const e1[5] = {
        [0] = 'N', [1] = 'I', [2] = 'C', [3] = 'E', [4] = '\0',
    };
    char const e2[9] = {
        [0] = 'N', [1] = 'I', [2] = 'C', [3] = 'E',  [4] = 'N',
        [5] = 'E', [6] = 'S', [7] = 'S', [8] = '\0',
    };
    str_view const e1_view = sv(e1);
    str_view const e2_view = sv_n(strlen(e1), e2);
    int const cmp_res = strcmp(e1, e1);
    CHECK(cmp_res == 0, sv_cmp(e1_view, e2_view) == 0, bool, "%d");
    CHECK(cmp_res == 0, sv_strcmp(e2_view, e1) == 0, bool, "%d");
    CHECK(cmp_res == 0, sv_cmp(e2_view, e1_view) == 0, bool, "%d");
    return PASS;
}

static enum test_result
test_compare_terminated(void)
{
    char const lesser[5] = {
        [0] = 'A', [1] = 'A', [2] = 'A', [3] = 'A', [4] = '\0',
    };
    char const greater[5] = {
        [0] = 'A', [1] = 'A', [2] = 'A', [3] = 'B', [4] = '\0',
    };
    str_view const lesser_view = sv(lesser);
    str_view const greater_view = sv(greater);
    int const cmp_res = strcmp(lesser, greater);
    int const cmp_res2 = strcmp(greater, lesser);
    CHECK(cmp_res < 0, sv_strcmp(lesser_view, greater) < 0, bool, "%d");
    CHECK(cmp_res < 0, sv_cmp(lesser_view, greater_view) < 0, bool, "%d");
    CHECK(cmp_res2 > 0, sv_strcmp(greater_view, lesser) > 0, bool, "%d");
    CHECK(cmp_res2 > 0, sv_cmp(greater_view, lesser_view) > 0, bool, "%d");
    return PASS;
}

static enum test_result
test_compare_different_lengths_terminated(void)
{
    char const lesser[5]
        = {[0] = 'A', [1] = 'A', [2] = 'A', [3] = 'A', [4] = '\0'};
    char const greater[3] = {[0] = 'A', [1] = 'A', [2] = '\0'};
    str_view const less_view = sv(lesser);
    str_view const greater_view = sv(greater);
    int const cmp_res = strcmp(lesser, greater);
    int const cmp_res2 = strcmp(greater, lesser);
    CHECK(cmp_res < 0, sv_strcmp(less_view, greater) < 0, bool, "%d");
    CHECK(cmp_res < 0, sv_cmp(less_view, greater_view) < 0, bool, "%d");
    CHECK(cmp_res2 > 0, sv_strcmp(greater_view, lesser) > 0, bool, "%d");
    CHECK(cmp_res2 > 0, sv_cmp(greater_view, less_view) > 0, bool, "%d");
    return PASS;
}

static enum test_result
test_compare_view_equals_str(void)
{
    char const *const views
        = "this string constains substring1, substring2, and substring3";
    char const *const str1 = "substring1";
    char const *const str2 = "substring2";
    char const *const str3 = "substring3";
    str_view const s1 = sv_match(sv(views), SV("substring1"));
    str_view const s2 = sv_match(sv(views), SV("substring2"));
    str_view const s3 = sv_rmatch(sv(views), SV("substring3"));
    CHECK(sv_strcmp(s1, str1), SV_EQL, sv_threeway_cmp, "%d");
    CHECK(sv_strcmp(s2, str2), SV_EQL, sv_threeway_cmp, "%d");
    CHECK(sv_strcmp(s3, str3), SV_EQL, sv_threeway_cmp, "%d");
    int const cmp_1_2 = strcmp(str1, str2);
    CHECK(sv_strcmp(s1, str2) < 0, cmp_1_2 < 0, bool, "%d");
    int const cmp_2_3 = strcmp(str2, str3);
    CHECK(sv_strcmp(s2, str3) < 0, cmp_2_3 < 0, bool, "%d");
    int const cmp_3_1 = strcmp(str3, str1);
    CHECK(sv_strcmp(s3, str1) > 0, cmp_3_1 > 0, bool, "%d");
    return PASS;
}

static enum test_result
test_compare_view_off_by_one(void)
{
    char const *const views
        = "this string constains substring12, substring2, and substring";
    /* Three views of interest withing the string. */
    str_view const v1 = sv_match(sv(views), SV("substring12"));
    str_view const v2 = sv_match(sv(views), SV("substring2"));
    str_view const v3 = sv_rmatch(sv(views), SV("substring"));
    /* The null terminated version of those three views. */
    char const *const s1 = "substring12";
    char const *const s2 = "substring2";
    char const *const s3 = "substring";
    /* These strings will not match the substrings within the string. */
    char const *const str1 = "substring1";
    char const *const str2 = "substring22";
    char const *const str3 = "substring3";
    int const cmp_1_1 = strcmp(s1, str1);
    CHECK(sv_strcmp(v1, str1) > 0, cmp_1_1 > 0, bool, "%d");
    int const cmp_2_2 = strcmp(s2, str2);
    CHECK(sv_strcmp(v2, str2) < 0, cmp_2_2 < 0, bool, "%d");
    int const cmp_3_3 = strcmp(s3, str3);
    CHECK(sv_strcmp(v3, str3) < 0, cmp_3_3 < 0, bool, "%d");
    /* Now do the same test but the off by one mismatch is other way. */
    char const *const str1_v2 = "substring121";
    char const *const str2_v2 = "substring";
    char const *const str3_v2 = "substrin";
    int const cmp_1_1_v2 = strcmp(s1, str1_v2);
    CHECK(sv_strcmp(v1, str1_v2) < 0, cmp_1_1_v2 < 0, bool, "%d");
    int const cmp_2_2_v2 = strcmp(s2, str2_v2);
    CHECK(sv_strcmp(v2, str2_v2) > 0, cmp_2_2_v2 > 0, bool, "%d");
    int const cmp_3_3_v2 = strcmp(s3, str3_v2);
    CHECK(sv_strcmp(v3, str3_v2) > 0, cmp_3_3_v2 > 0, bool, "%d");
    return PASS;
}

static enum test_result
test_compare_different_lengths_views(void)
{

    char const lesser[5] = {
        [0] = 'A', [1] = 'A', [2] = 'A', [3] = 'A', [4] = '\0',
    };
    char const greater_str[5] = {
        [0] = 'A', [1] = 'A', [2] = 'A', [3] = 'B', [4] = '\0',
    };
    char const greater_longer[9] = {
        [0] = 'A', [1] = 'A', [2] = 'A', [3] = 'B',  [4] = 'A',
        [5] = 'B', [6] = 'Y', [7] = 'Z', [8] = '\0',
    };
    int const str_cmp = strcmp(lesser, greater_str);
    int const str_cmp2 = strcmp(greater_str, lesser);
    str_view const greater_view = sv_n(strlen(greater_str), greater_longer);
    str_view const lesser_view = sv(lesser);
    CHECK(str_cmp2 > 0, sv_strcmp(greater_view, lesser) > 0, bool, "%d");
    CHECK(str_cmp < 0, sv_strcmp(lesser_view, greater_str) < 0, bool, "%d");
    CHECK(str_cmp < 0, sv_cmp(lesser_view, greater_view) < 0, bool, "%d");
    CHECK(str_cmp2 > 0, sv_cmp(greater_view, lesser_view) > 0, bool, "%d");
    return PASS;
}

static enum test_result
test_compare_misc(void)
{
    CHECK(sv_cmp(SV(""), SV("")), SV_EQL, sv_threeway_cmp, "%d");
    CHECK(sv_strcmp(SV(""), ""), SV_EQL, sv_threeway_cmp, "%d");
    CHECK(sv_cmp(SV("same"), SV("same")), SV_EQL, sv_threeway_cmp, "%d");
    CHECK(sv_cmp(SV("samz"), SV("same")), SV_GRT, sv_threeway_cmp, "%d");
    CHECK(sv_cmp(SV("same"), SV("samz")), SV_LES, sv_threeway_cmp, "%d");
    /* The comparison function should treat the end of a string view as
       null terminating character even if it points to a delimeter */
    CHECK(sv_cmp(SV("same"), sv_delim("same same", " ")), SV_EQL,
          sv_threeway_cmp, "%d");
    CHECK(sv_cmp(SV("same"), sv_delim("samz same", " ")), SV_LES,
          sv_threeway_cmp, "%d");
    CHECK(sv_cmp(sv_delim("sameez same", " "), SV("same")), SV_GRT,
          sv_threeway_cmp, "%d");
    char const *const str = "same";
    CHECK(sv_strcmp(sv(str), str), SV_EQL, sv_threeway_cmp, "%d");
    CHECK(sv_strcmp(sv_delim("same same", " "), str), SV_EQL, sv_threeway_cmp,
          "%d");
    CHECK(sv_strcmp(sv_delim("samez same", " "), str), SV_GRT, sv_threeway_cmp,
          "%d");
    CHECK(sv_strcmp(sv_delim("sameez same", " "), str), SV_GRT, sv_threeway_cmp,
          "%d");
    /* strncmp compares at most n bytes inclusize or stops at null. */
    CHECK(sv_strncmp(sv_delim("sameez same", " "), str, 10), SV_GRT,
          sv_threeway_cmp, "%d");
    CHECK(sv_strncmp(sv_delim("saaeez same", " "), str, 3), SV_LES,
          sv_threeway_cmp, "%d");
    return PASS;
}
