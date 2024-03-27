#include "str_view.h"
#include "test.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static enum test_result test_from_null(void);
static enum test_result test_from_delim(void);
static enum test_result test_from_delim_multiple(void);
static enum test_result test_from_multichar_delim(void);
static enum test_result test_from_delim_no_delim(void);
static enum test_result test_empty_constructor(void);

#define NUM_TESTS (size_t)6
const struct fn_name all_tests[NUM_TESTS] = {
    {test_from_null, "test_from_null"},
    {test_from_delim, "test_from_delim"},
    {test_from_delim_multiple, "test_from_delim_multiple"},
    {test_from_multichar_delim, "test_from_multichar_delim"},
    {test_from_delim_no_delim, "test_from_delim_no_delim"},
    {test_empty_constructor, "test_empty_constructor"},
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
            printf("test_constructors test failed: %s\n", all_tests[i].name);
            res = FAIL;
        }
    }
    return res;
}

static enum test_result
test_from_null(void)
{
    const char *const reference = "Don't miss the terminator!";
    const str_view s = sv(reference);
    const size_t reference_len = strlen(reference);
    CHECK(reference_len, sv_len(s), "%zu");
    CHECK(sv_strcmp(s, reference), EQL, "%d");
    const char *const chunk = "Don't";
    const size_t chunk_len = strlen(chunk);
    const str_view n_bytes = sv_n(chunk_len, reference);
    CHECK(sv_len(n_bytes), chunk_len, "%zu");
    CHECK(sv_strcmp(n_bytes, chunk), EQL, "%d");
    return PASS;
}

static enum test_result
test_from_delim(void)
{
    const char *const reference = "Don'tmissthedelim That was it!";
    const char *const reference_chunk = "Don'tmissthedelim";
    const str_view sv = sv_delim(reference, " ");
    const size_t reference_len = strlen(reference_chunk);
    CHECK(reference_len, sv_len(sv), "%zu");
    CHECK(sv_strcmp(sv, reference_chunk), EQL, "%d");
    /* If the string starts with delim we must skip it. */
    const char *const ref2 = ",Don't miss the delim, that was it!";
    const char *const ref2_chunk = "Don't miss the delim";
    const str_view sv2 = sv_delim(ref2, ",");
    const size_t ref2_len = strlen(ref2_chunk);
    CHECK(ref2_len, sv_len(sv2), "%zu");
    CHECK(sv_strcmp(sv2, ref2_chunk), EQL, "%d");
    CHECK(sv_strcmp(sv_extend(sv2), ref2 + 1), EQL, "%d");
    return PASS;
}

static enum test_result
test_from_delim_multiple(void)
{
    const char *const reference = ",,,Don'tmissthedelim,,,That was it!";
    const char *const reference_chunk = "Don'tmissthedelim";
    const str_view sv = sv_delim(reference, ",");
    const size_t reference_len = strlen(reference_chunk);
    CHECK(reference_len, sv_len(sv), "%zu");
    CHECK(sv_strcmp(sv, reference_chunk), EQL, "%d");
    return PASS;
}

static enum test_result
test_from_multichar_delim(void)
{
    const char *const reference = "delimDon'tmissthedelimThat was it!";
    const char *const reference_chunk = "Don'tmissthe";
    const str_view sv = sv_delim(reference, "delim");
    const size_t reference_len = strlen(reference_chunk);
    CHECK(reference_len, sv_len(sv), "%zu");
    CHECK(sv_strcmp(sv, reference_chunk), EQL, "%d");
    return PASS;
}

static enum test_result
test_from_delim_no_delim(void)
{
    const char *const reference = "Don'tmissthedelimbutnodelim!";
    const str_view sv = sv_delim(reference, " ");
    const size_t reference_len = strlen(reference);
    CHECK(reference_len, sv_len(sv), "%zu");
    CHECK(reference[reference_len - 1], sv_at(sv, sv_len(sv) - 1), "%c");
    return PASS;
}

static enum test_result
test_empty_constructor(void)
{
    const char *const reference = "------------";
    const str_view sv = sv_delim(reference, "-");
    CHECK(sv_len(sv), 0UL, "%zu");
    CHECK(sv_empty(sv), true, "%b");
    return PASS;
}
