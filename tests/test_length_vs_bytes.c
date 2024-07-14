#include "str_view.h"
#include "test.h"

#include <string.h>

static enum test_result test_length_terminated(void);
static enum test_result test_length_unterminated(void);
static enum test_result test_length_innacurate(void);

#define NUM_TESTS (size_t)3

static test_fn const all_tests[NUM_TESTS] = {
    test_length_terminated,
    test_length_unterminated,
    test_length_innacurate,
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
test_length_terminated(void)
{
    char const ref[6] = {
        [0] = 'H', [1] = 'e', [2] = 'l', [3] = 'l', [4] = 'l', [5] = '\0',
    };
    size_t const len = strlen(ref);
    size_t const bytes = sizeof ref;
    CHECK(len, strlen(ref), size_t, "%zu");
    CHECK(len, sv_len(sv(ref)), size_t, "%zu");
    CHECK(bytes, sv_strsize(ref), size_t, "%zu");
    CHECK(bytes, sv_size(sv(ref)), size_t, "%zu");
    CHECK(len, sv_npos(sv(ref)), size_t, "%zu");
    CHECK(len, sv_minlen(ref, -1), size_t, "%zu");
    return PASS;
}

static enum test_result
test_length_unterminated(void)
{
    char const ref[12] = {
        [0] = 'H', [1] = 'e', [2] = 'l', [3] = 'l', [4] = 'l',  [5] = ' ',
        [6] = 's', [7] = 'n', [8] = 'i', [9] = 'p', [10] = '!', [11] = '\0',
    };
    char const snip[5] = "snip\0";
    size_t const len = strlen(snip);
    size_t const bytes = sizeof snip;
    str_view const snip_view = sv_n(len, ref + 6);
    CHECK(strlen(snip), len, size_t, "%zu");
    CHECK(sv_len(snip_view), len, size_t, "%zu");
    CHECK(sv_strsize(snip), bytes, size_t, "%zu");
    CHECK(sv_size(snip_view), bytes, size_t, "%zu");
    CHECK(len, sv_npos(snip_view), size_t, "%zu");
    CHECK(len, sv_minlen(snip, 99), size_t, "%zu");
    return PASS;
}

static enum test_result
test_length_innacurate(void)
{
    char const ref[18]
        = {[0] = 'H',  [1] = 'e',   [2] = 'l',  [3] = 'l',  [4] = 'l',
           [5] = ' ',  [6] = 's',   [7] = 'n',  [8] = 'i',  [9] = 'p',
           [10] = '!', [11] = '\0', [12] = 's', [13] = 'n', [14] = 'i',
           [15] = 'p', [16] = '!',  [17] = '\0'};
    size_t const len = strlen(ref);
    size_t const bytes = len + 1;
    str_view const view = sv_n(sizeof(ref), ref);
    CHECK(len, strlen(ref), size_t, "%zu");
    CHECK(len, sv_len(view), size_t, "%zu");
    CHECK(bytes, sv_strsize(ref), size_t, "%zu");
    CHECK(bytes, sv_size(view), size_t, "%zu");
    CHECK(len, sv_npos(view), size_t, "%zu");
    CHECK(len, sv_minlen(ref, sizeof(ref)), size_t, "%zu");
    str_view const view2 = sv_n(sizeof(ref), ref);
    CHECK(len, strlen(ref), size_t, "%zu");
    CHECK(len, sv_len(view2), size_t, "%zu");
    CHECK(bytes, sv_strsize(ref), size_t, "%zu");
    CHECK(bytes, sv_size(view2), size_t, "%zu");
    CHECK(len, (sv_npos(view2)), size_t, "%zu");
    CHECK(len, sv_minlen(ref, sizeof(ref)), size_t, "%zu");
    return PASS;
}
