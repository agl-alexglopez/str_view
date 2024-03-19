#include "str_view.h"
#include "test.h"

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
    if (reference_len != sv_svlen(s))
    {
        return FAIL;
    }
    if (sv_strcmp(s, reference) != EQL)
    {
        return FAIL;
    }
    const char *const chunk = "Don't";
    const size_t chunk_len = strlen(chunk);
    const str_view n_bytes = sv_n(reference, chunk_len);
    if (sv_svlen(n_bytes) != chunk_len)
    {
        return FAIL;
    }
    if (sv_strcmp(n_bytes, chunk) != EQL)
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_from_delim(void)
{
    const char *const reference = "Don'tmissthedelim That was it!";
    const char *const reference_chunk = "Don'tmissthedelim";
    const str_view sv = sv_delim(reference, " ");
    const size_t reference_len = strlen(reference_chunk);
    if (reference_len != sv_svlen(sv))
    {
        return FAIL;
    }
    if (sv_strcmp(sv, reference_chunk) != EQL)
    {
        return FAIL;
    }
    /* If the string starts with delim we must skip it. */
    const char *const ref2 = ",Don't miss the delim, that was it!";
    const char *const ref2_chunk = "Don't miss the delim";
    const str_view sv2 = sv_delim(ref2, ",");
    const size_t ref2_len = strlen(ref2_chunk);
    if (ref2_len != sv_svlen(sv2))
    {
        return FAIL;
    }
    if (sv_strcmp(sv2, ref2_chunk) != EQL
        || sv_strcmp(sv_extend(sv2), ref2 + 1) != EQL)
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_from_delim_multiple(void)
{
    const char *const reference = ",,,Don'tmissthedelim,,,That was it!";
    const char *const reference_chunk = "Don'tmissthedelim";
    const str_view sv = sv_delim(reference, ",");
    const size_t reference_len = strlen(reference_chunk);
    if (reference_len != sv_svlen(sv))
    {
        return FAIL;
    }
    if (sv_strcmp(sv, reference_chunk) != EQL)
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_from_multichar_delim(void)
{
    const char *const reference = "delimDon'tmissthedelimThat was it!";
    const char *const reference_chunk = "Don'tmissthe";
    const str_view sv = sv_delim(reference, "delim");
    const size_t reference_len = strlen(reference_chunk);
    if (reference_len != sv_svlen(sv))
    {
        return FAIL;
    }
    if (sv_strcmp(sv, reference_chunk) != EQL)
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_from_delim_no_delim(void)
{
    const char *const reference = "Don'tmissthedelimbutnodelim!";
    const str_view sv = sv_delim(reference, " ");
    const size_t reference_len = strlen(reference);
    if (reference_len != sv_svlen(sv))
    {
        return FAIL;
    }
    if (reference[reference_len - 1] != sv_at(sv, sv_svlen(sv) - 1))
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_empty_constructor(void)
{
    const char *const reference = "------------";
    const str_view sv = sv_delim(reference, "-");
    const size_t reference_len = strlen(reference);
    if (reference_len == sv_svlen(sv) || !sv_empty(sv))
    {
        return FAIL;
    }
    return PASS;
}
