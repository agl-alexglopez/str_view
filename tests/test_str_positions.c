#include "str_view.h"
#include "test.h"

#include <string.h>

static enum test_result test_front_back_terminated(void);
static enum test_result test_front_back_view(void);

#define NUM_TESTS (size_t)2

static test_fn const all_tests[NUM_TESTS] = {
    test_front_back_terminated,
    test_front_back_view,
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
test_front_back_terminated(void)
{
    CHECK(sv_back(SV("")), '\0', char, "%c");
    CHECK(sv_front(SV("")), '\0', char, "%c");
    char const *const reference = "*The front was * the back is!";
    str_view const s = sv(reference);
    size_t const ref_len = strlen(reference);
    CHECK(ref_len, sv_len(s), size_t, "%zu");
    CHECK(sv_front(s), '*', char, "%c");
    CHECK(sv_back(s), '!', char, "%c");
    return PASS;
}

static enum test_result
test_front_back_view(void)
{
    char const reference[20] = {
        [0] = 'A',  [1] = 'A',  [2] = 'C',  [3] = ' ',  [4] = '^',
        [5] = '!',  [6] = '!',  [7] = ' ',  [8] = '*',  [9] = '*',
        [10] = ' ', [11] = '@', [12] = '_', [13] = ' ', [14] = '!',
        [15] = '!', [16] = '!', [17] = ' ', [18] = 'A', [19] = '\0',
    };
    str_view const s = sv_n(8, reference + 4);
    CHECK(sv_front(s), '^', char, "%c");
    CHECK(sv_back(s), '@', char, "%c");
    CHECK(sv_len(s), 8UL, size_t, "%zu");
    return PASS;
}
