#include "str_view.h"
#include "test.h"

#include <stdbool.h>
#include <string.h>

static enum test_result test_from_null(void);
static enum test_result test_from_delim(void);
static enum test_result test_from_delim_multiple(void);
static enum test_result test_from_multichar_delim(void);
static enum test_result test_from_delim_no_delim(void);
static enum test_result test_empty_constructor(void);

#define NUM_TESTS (size_t)6
static test_fn const all_tests[NUM_TESTS] = {
    test_from_null,           test_from_delim,
    test_from_delim_multiple, test_from_multichar_delim,
    test_from_delim_no_delim, test_empty_constructor,
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
test_from_null(void)
{
    char const *const reference = "Don't miss the terminator!";
    str_view const s = sv(reference);
    size_t const reference_len = strlen(reference);
    CHECK(reference_len, sv_len(s), size_t, "%zu");
    CHECK(sv_strcmp(s, reference), SV_EQL, sv_threeway_cmp, "%d");
    char const *const chunk = "Don't";
    size_t const chunk_len = strlen(chunk);
    str_view const n_bytes = sv_n(chunk_len, reference);
    CHECK(sv_len(n_bytes), chunk_len, size_t, "%zu");
    CHECK(sv_strcmp(n_bytes, chunk), SV_EQL, sv_threeway_cmp, "%d");
    return PASS;
}

static enum test_result
test_from_delim(void)
{
    char const *const reference = "Don'tmissthedelim That was it!";
    char const *const reference_chunk = "Don'tmissthedelim";
    str_view const sv = sv_delim(reference, " ");
    size_t const reference_len = strlen(reference_chunk);
    CHECK(reference_len, sv_len(sv), size_t, "%zu");
    CHECK(sv_strcmp(sv, reference_chunk), SV_EQL, sv_threeway_cmp, "%d");
    /* If the string starts with delim we must skip it. */
    char const *const ref2 = ",Don't miss the delim, that was it!";
    char const *const ref2_chunk = "Don't miss the delim";
    str_view const sv2 = sv_delim(ref2, ",");
    size_t const ref2_len = strlen(ref2_chunk);
    CHECK(ref2_len, sv_len(sv2), size_t, "%zu");
    CHECK(sv_strcmp(sv2, ref2_chunk), SV_EQL, sv_threeway_cmp, "%d");
    CHECK(sv_strcmp(sv_extend(sv2), ref2 + 1), SV_EQL, sv_threeway_cmp, "%d");
    return PASS;
}

static enum test_result
test_from_delim_multiple(void)
{
    char const *const reference = ",,,Don'tmissthedelim,,,That was it!";
    char const *const reference_chunk = "Don'tmissthedelim";
    str_view const sv = sv_delim(reference, ",");
    size_t const reference_len = strlen(reference_chunk);
    CHECK(reference_len, sv_len(sv), size_t, "%zu");
    CHECK(sv_strcmp(sv, reference_chunk), SV_EQL, sv_threeway_cmp, "%d");
    return PASS;
}

static enum test_result
test_from_multichar_delim(void)
{
    char const *const reference = "delimDon'tmissthedelimThat was it!";
    char const *const reference_chunk = "Don'tmissthe";
    str_view const sv = sv_delim(reference, "delim");
    size_t const reference_len = strlen(reference_chunk);
    CHECK(reference_len, sv_len(sv), size_t, "%zu");
    CHECK(sv_strcmp(sv, reference_chunk), SV_EQL, sv_threeway_cmp, "%d");
    return PASS;
}

static enum test_result
test_from_delim_no_delim(void)
{
    char const *const reference = "Don'tmissthedelimbutnodelim!";
    str_view const sv = sv_delim(reference, " ");
    size_t const reference_len = strlen(reference);
    CHECK(reference_len, sv_len(sv), size_t, "%zu");
    CHECK(reference[reference_len - 1], sv_at(sv, sv_len(sv) - 1), char, "%c");
    return PASS;
}

static enum test_result
test_empty_constructor(void)
{
    char const *const reference = "------------";
    str_view const sv = sv_delim(reference, "-");
    CHECK(sv_len(sv), 0UL, size_t, "%zu");
    CHECK(sv_empty(sv), true, bool, "%d");
    return PASS;
}
