#include "str_view.h"
#include "test.h"

#include <stdbool.h>
#include <string.h>

static bool test_front_back_terminated(void);
static bool test_front_back_view(void);

#define NUM_TESTS (size_t)2
const struct fn_name all_tests[NUM_TESTS] = {
    {test_front_back_terminated, "construct from terminated string"},
    {test_front_back_view, "construct from view of string"},
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
            printf("\n");
            printf("test_constructors test failed: %s\n", all_tests[i].name);
            res = FAIL;
        }
    }
    return res;
}

static bool
test_front_back_terminated(void)
{
    if (sv_back(sv("")) != '\0' || sv_front(sv("")) != '\0')
    {
        return false;
    }
    const char *const reference = "*The front was * the back is!";
    const str_view s = sv(reference);
    const size_t ref_len = strlen(reference);
    if (ref_len != sv_svlen(s))
    {
        return false;
    }
    if (sv_front(s) != '*' || sv_back(s) != '!')
    {
        return false;
    }
    return true;
}

static bool
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
        return false;
    }
    return true;
}
