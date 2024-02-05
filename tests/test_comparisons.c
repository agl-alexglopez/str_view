#include "str_view.h"
#include "test.h"
#include <string.h>

static enum test_result test_compare_terminated(void);
static enum test_result test_compare_different_lengths_terminated(void);

#define NUM_TESTS (size_t)2
const struct fn_name all_tests[NUM_TESTS] = {
    {test_compare_terminated, "test_compare_terminated"},
    {test_compare_different_lengths_terminated,
     "test_compare_different_lengths_terminated"},
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
            printf("test_comparisons test failed: %s\n", all_tests[i].name);
            res = FAIL;
        }
    }
    return res;
}

static enum test_result
test_compare_terminated(void)
{
    const char lesser[5] = {
        [0] = 'A', [1] = 'A', [2] = 'A', [3] = 'A', [4] = '\0',
    };
    const char greater[5] = {
        [0] = 'A', [1] = 'A', [2] = 'A', [3] = 'B', [4] = '\0',
    };
    const str_view less_view = sv(lesser);
    const str_view greater_view = sv(greater);
    const int cmp_res = strcmp(lesser, greater);
    if (cmp_res != sv_strcmp(less_view, greater)
        || cmp_res != sv_svcmp(less_view, greater_view))
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_compare_different_lengths_terminated(void)
{
    const char lesser[5] = {
        [0] = 'A', [1] = 'A', [2] = 'A', [3] = 'A', [4] = '\0',
    };
    const char greater[3] = {
        [0] = 'A',
        [1] = 'A',
        [2] = '\0',
    };
    const str_view less_view = sv(lesser);
    const str_view greater_view = sv(greater);
    const int cmp_res = strcmp(lesser, greater);
    if (cmp_res != sv_strcmp(less_view, greater)
        || cmp_res != sv_svcmp(less_view, greater_view))
    {
        return FAIL;
    }
    return PASS;
}
