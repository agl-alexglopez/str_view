#include "str_view.h"
#include "test.h"

#include <stdio.h>
#include <string.h>

static enum test_result test_front_back_terminated(void);
static enum test_result test_front_back_view(void);

#define NUM_TESTS (size_t)2
const struct fn_name all_tests[NUM_TESTS] = {
    {test_front_back_terminated, "test_front_back_terminated"},
    {test_front_back_view, "test_front_back_view"},
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
            printf("test_str_positions test failed: %s\n", all_tests[i].name);
            res = FAIL;
        }
    }
    return res;
}

static enum test_result
test_front_back_terminated(void)
{
    if (sv_back(sv("")) != '\0' || sv_front(sv("")) != '\0')
    {
        return FAIL;
    }
    const char *const reference = "*The front was * the back is!";
    const str_view s = sv(reference);
    const size_t ref_len = strlen(reference);
    if (ref_len != sv_svlen(s))
    {
        return FAIL;
    }
    if (sv_front(s) != '*' || sv_back(s) != '!')
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_front_back_view(void)
{
    const char reference[20] = {
        [0] = 'A',  [1] = 'A',  [2] = 'C',  [3] = ' ',  [4] = '^',
        [5] = '!',  [6] = '!',  [7] = ' ',  [8] = '*',  [9] = '*',
        [10] = ' ', [11] = '@', [12] = '_', [13] = ' ', [14] = '!',
        [15] = '!', [16] = '!', [17] = ' ', [18] = 'A', [19] = '\0',
    };
    const str_view s = sv_n(reference + 4, 8);
    if (sv_front(s) != '^' || sv_back(s) != '@' || sv_svlen(s) != 8)
    {
        return FAIL;
    }
    return PASS;
}
