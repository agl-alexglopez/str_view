#include "str_view.h"
#include "test.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static bool test_from_null(void);
static bool test_from_delim(void);
static bool test_from_delim_multiple(void);
static bool test_from_multichar_delim(void);
static bool test_from_delim_no_delim(void);
static bool test_empty_constructor(void);

#define NUM_TESTS (size_t)6
const struct fn_name all_tests[NUM_TESTS] = {
    {test_from_null, "construct from terminated string"},
    {test_from_delim, "construct from delimiter"},
    {test_from_delim_multiple, "construct from consecutive delimiter"},
    {test_from_multichar_delim, "construct from multicharacter delimiter"},
    {test_from_delim_no_delim, "construct from delim but no delim found"},
    {test_empty_constructor, "construct from empty string"},
};

int
main()
{
    enum test_result res = PASS;
    for (size_t i = 0; i < NUM_TESTS; ++i)
    {
        const bool passed = all_tests[i].fn();
        if (!passed)
        {
            printf("test_constructors test failed: %s\n", all_tests[i].name);
            res = FAIL;
        }
    }
    return res;
}

static bool
test_from_null(void)
{
    const char *const reference = "Don't miss the terminator!";
    const str_view s = sv(reference);
    const size_t reference_len = strlen(reference);
    if (reference_len != sv_svlen(s))
    {
        return false;
    }
    if (sv_strcmp(s, reference) != EQL)
    {
        return false;
    }
    const char *const chunk = "Don't";
    const size_t chunk_len = strlen(chunk);
    const str_view n_bytes = sv_n(reference, chunk_len);
    if (sv_svlen(n_bytes) != chunk_len)
    {
        return false;
    }
    if (sv_strcmp(n_bytes, chunk) != EQL)
    {
        return false;
    }
    return true;
}

static bool
test_from_delim(void)
{
    const char *const reference = "Don'tmissthedelim That was it!";
    const char *const reference_chunk = "Don'tmissthedelim";
    const str_view sv = sv_delim(reference, " ");
    const size_t reference_len = strlen(reference_chunk);
    if (reference_len != sv_svlen(sv))
    {
        return false;
    }
    if (sv_strcmp(sv, reference_chunk) != EQL)
    {
        return false;
    }
    /* If the string starts with delim we must skip it. */
    const char *const ref2 = ",Don't miss the delim, that was it!";
    const char *const ref2_chunk = "Don't miss the delim";
    const str_view sv2 = sv_delim(ref2, ",");
    const size_t ref2_len = strlen(ref2_chunk);
    if (ref2_len != sv_svlen(sv2))
    {
        return false;
    }
    if (sv_strcmp(sv2, ref2_chunk) != EQL)
    {
        return false;
    }
    return true;
}

static bool
test_from_delim_multiple(void)
{
    const char *const reference = ",,,Don'tmissthedelim,,,That was it!";
    const char *const reference_chunk = "Don'tmissthedelim";
    const str_view sv = sv_delim(reference, ",");
    const size_t reference_len = strlen(reference_chunk);
    if (reference_len != sv_svlen(sv))
    {
        return false;
    }
    if (sv_strcmp(sv, reference_chunk) != EQL)
    {
        return false;
    }
    return true;
}

static bool
test_from_multichar_delim(void)
{
    const char *const reference = "delimDon'tmissthedelimThat was it!";
    const char *const reference_chunk = "Don'tmissthe";
    const str_view sv = sv_delim(reference, "delim");
    const size_t reference_len = strlen(reference_chunk);
    if (reference_len != sv_svlen(sv))
    {
        return false;
    }
    if (sv_strcmp(sv, reference_chunk) != EQL)
    {
        return false;
    }
    return true;
}

static bool
test_from_delim_no_delim(void)
{
    const char *const reference = "Don'tmissthedelimbutnodelim!";
    const str_view sv = sv_delim(reference, " ");
    const size_t reference_len = strlen(reference);
    if (reference_len != sv_svlen(sv))
    {
        return false;
    }
    if (reference[reference_len - 1] != sv_at(sv, sv_svlen(sv) - 1))
    {
        return false;
    }
    return true;
}

static bool
test_empty_constructor(void)
{
    const char *const reference = "------------";
    const str_view sv = sv_delim(reference, "-");
    const size_t reference_len = strlen(reference);
    if (reference_len == sv_svlen(sv) || !sv_empty(sv))
    {
        return false;
    }
    return true;
}
