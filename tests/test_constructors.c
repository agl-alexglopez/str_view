#include "str_view.h"
#include "test.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static bool test_from_null(void);
static bool test_from_delim(void);
static bool test_from_delim_no_delim(void);
static bool test_empty_constructor(void);

#define NUM_TESTS (size_t)4
const struct fn_name all_tests[NUM_TESTS] = {
    {test_from_null, "construct from terminated string"},
    {test_from_delim, "construct from delimiter break"},
    {test_from_delim_no_delim, "construct from delim but no delim found"},
    {test_empty_constructor, "construct from empty string"},
};

int
main()
{
    printf("\n");
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
    printf("reference=[%s]\n", reference);
    printf("string vw=[");
    sv_print(stdout, s);
    printf("]\n");
    const size_t reference_len = strlen(reference);
    if (reference_len != sv_svlen(s))
    {
        return false;
    }
    if (reference[reference_len - 1] != sv_at(s, sv_svlen(s) - 1))
    {
        return false;
    }
    const char *const chunk = "Don't";
    const size_t chunk_len = strlen(chunk);
    const str_view n_bytes = sv_n(reference, chunk_len);
    printf("5 bytes=[%s]\n", chunk);
    printf("5 byte view=[");
    sv_print(stdout, n_bytes);
    printf("]\n");
    if (sv_svlen(n_bytes) != chunk_len)
    {
        return false;
    }
    if (chunk[chunk_len - 1] != sv_at(n_bytes, sv_svlen(n_bytes) - 1))
    {
        return false;
    }
    return true;
}

static bool
test_from_delim(void)
{
    const char *const reference = "Don'tmissthedelim That was it!";
    const char *const reference_delim = "Don'tmissthedelim";
    const str_view sv = sv_delim(reference, " ");
    const size_t reference_len = strlen(reference_delim);
    printf("delimiter=[,]\n");
    printf("reference=[%s]\n", reference);
    printf("first tok=[%s]\n", reference_delim);
    printf("this should be first tok=[");
    sv_print(stdout, sv);
    printf("]\n");
    if (reference_len != sv_svlen(sv))
    {
        return false;
    }
    if (reference_delim[reference_len - 1] != sv_at(sv, sv_svlen(sv) - 1))
    {
        return false;
    }
    /* If the string starts with delim we must skip it. */
    const char *const ref2 = ",Don't miss the delim, that was it!";
    const char *const ref2_delim = "Don't miss the delim";
    const str_view sv2 = sv_delim(ref2, ",");
    const size_t ref2_len = strlen(ref2_delim);
    printf("delimiter=[,]\n");
    printf("reference=[%s]\n", ref2);
    printf("first tok=[%s]\n", ref2_delim);
    printf("this should be first tok=[");
    sv_print(stdout, sv2);
    printf("]\n");
    if (ref2_len != sv_svlen(sv2))
    {
        return false;
    }
    if (ref2_delim[ref2_len - 1] != sv_at(sv2, sv_svlen(sv2) - 1))
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
    printf("delimiter=[ ]\n");
    printf("reference=[%s]\n", reference);
    printf("this should be reference=[");
    sv_print(stdout, sv);
    printf("]\n");
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
    printf("delimiter=[-]\n");
    printf("reference=[%s]\n", reference);
    printf("this should be empty=[");
    sv_print(stdout, sv);
    printf("]\n");
    if (reference_len == sv_svlen(sv) || !sv_empty(sv))
    {
        return false;
    }
    return true;
}
