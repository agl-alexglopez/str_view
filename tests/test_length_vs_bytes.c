#include "str_view.h"
#include "test.h"

#include <stdio.h>
#include <string.h>

static enum test_result test_length_terminated(void);
static enum test_result test_length_unterminated(void);
static enum test_result test_length_innacurate(void);

#define NUM_TESTS (size_t)3
const struct fn_name all_tests[NUM_TESTS] = {
    {test_length_terminated, "test_length_terminated"},
    {test_length_unterminated, "test_length_unterminated"},
    {test_length_innacurate, "test_length_innacurate"},
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
            printf("test_length_vs_bytes test failed: %s\n", all_tests[i].name);
            res = FAIL;
        }
    }
    return res;
}

static enum test_result
test_length_terminated(void)
{
    const char ref[6] = {
        [0] = 'H', [1] = 'e', [2] = 'l', [3] = 'l', [4] = 'l', [5] = '\0',
    };
    const size_t len = strlen(ref);
    const size_t bytes = sizeof ref;
    CHECK(len, strlen(ref), "%zu");
    CHECK(len, sv_len(sv(ref)), "%zu");
    CHECK(bytes, sv_strsize(ref), "%zu");
    CHECK(bytes, sv_size(sv(ref)), "%zu");
    CHECK(len, sv_npos(sv(ref)), "%zu");
    CHECK(len, sv_minlen(ref, -1), "%zu");
    return PASS;
}

static enum test_result
test_length_unterminated(void)
{
    const char ref[12] = {
        [0] = 'H', [1] = 'e', [2] = 'l', [3] = 'l', [4] = 'l',  [5] = ' ',
        [6] = 's', [7] = 'n', [8] = 'i', [9] = 'p', [10] = '!', [11] = '\0',
    };
    const char snip[5] = "snip\0";
    const size_t len = strlen(snip);
    const size_t bytes = sizeof snip;
    const str_view snip_view = sv_n(len, ref + 6);
    CHECK(strlen(snip), len, "%zu");
    CHECK(sv_len(snip_view), len, "%zu");
    CHECK(sv_strsize(snip), bytes, "%zu");
    CHECK(sv_size(snip_view), bytes, "%zu");
    CHECK(len, sv_npos(snip_view), "%zu");
    CHECK(len, sv_minlen(snip, 99), "%zu");
    return PASS;
}

static enum test_result
test_length_innacurate(void)
{
    const char ref[18]
        = {[0] = 'H',  [1] = 'e',   [2] = 'l',  [3] = 'l',  [4] = 'l',
           [5] = ' ',  [6] = 's',   [7] = 'n',  [8] = 'i',  [9] = 'p',
           [10] = '!', [11] = '\0', [12] = 's', [13] = 'n', [14] = 'i',
           [15] = 'p', [16] = '!',  [17] = '\0'};
    const size_t len = strlen(ref);
    const size_t bytes = len + 1;
    const str_view view = sv_n(sizeof(ref), ref);
    CHECK(len, strlen(ref), "%zu");
    CHECK(len, sv_len(view), "%zu");
    CHECK(bytes, sv_strsize(ref), "%zu");
    CHECK(bytes, sv_size(view), "%zu");
    CHECK(len, sv_npos(view), "%zu");
    CHECK(len, sv_minlen(ref, sizeof(ref)), "%zu");
    const str_view view2 = sv_n(sizeof(ref), ref);
    CHECK(len, strlen(ref), "%zu");
    CHECK(len, sv_len(view2), "%zu");
    CHECK(bytes, sv_strsize(ref), "%zu");
    CHECK(bytes, sv_size(view2), "%zu");
    CHECK(len, (sv_npos(view2)), "%zu");
    CHECK(len, sv_minlen(ref, sizeof(ref)), "%zu");
    return PASS;
}
