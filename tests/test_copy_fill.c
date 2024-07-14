#include "str_view.h"
#include "test.h"

#include <string.h>

static enum test_result test_copy_fill(void);
static enum test_result test_copy_section(void);

#define NUM_TESTS (size_t)2
static test_fn const all_tests[NUM_TESTS] = {
    test_copy_fill,
    test_copy_section,
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
test_copy_fill(void)
{
    char const *const reference = "Copy this over there!";
    str_view this = sv_copy(strlen(reference), reference);
    char there[sv_strsize(reference)];
    CHECK(sv_fill(sizeof there, there, this), sizeof there, size_t, "%zu");
    CHECK(strcmp(sv_begin(this), there), 0, sv_threeway_cmp, "%d");
    CHECK(strlen(there), sv_len(this), size_t, "%zu");
    CHECK(there[(sizeof there) - 1], '\0', char, "%c");
    return PASS;
}

static enum test_result
test_copy_section(void)
{
    char const ref[20] = {
        [0] = 'A',  [1] = 'A',  [2] = 'C',  [3] = ' ',  [4] = '!',
        [5] = 's',  [6] = 'n',  [7] = 'i',  [8] = 'p',  [9] = '!',
        [10] = ' ', [11] = '_', [12] = '_', [13] = ' ', [14] = '!',
        [15] = '!', [16] = '!', [17] = ' ', [18] = 'A', [19] = '\0',
    };
    char const *const expected_snip = "snip!";
    str_view const ref_view = sv(ref);
    str_view const snip = sv_substr(ref_view, 5, 5);
    char snip_buf[sv_size(snip)];
    CHECK(sv_fill(sizeof snip_buf, snip_buf, snip), sv_size(snip), size_t,
          "%zu");
    CHECK(strcmp(expected_snip, snip_buf), 0, sv_threeway_cmp, "%d");
    CHECK(snip_buf[(sizeof snip_buf) - 1], '\0', char, "%c");
    return PASS;
}
