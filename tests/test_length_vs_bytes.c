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
    CHECK(len, sv_strlen(ref));
    CHECK(len, sv_len(sv(ref)));
    CHECK(bytes, sv_strbytes(ref));
    CHECK(bytes, sv_svbytes(sv(ref)));
    CHECK(len, sv_npos(sv(ref)));
    CHECK(len, sv_minlen(ref, -1));
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
    const str_view snip_view = sv_n(ref + 6, len);
    CHECK(sv_strlen(snip), len);
    CHECK(sv_len(snip_view), len);
    CHECK(sv_strbytes(snip), bytes);
    CHECK(sv_svbytes(snip_view), bytes);
    CHECK(len, sv_npos(snip_view));
    CHECK(len, sv_minlen(snip, 99));
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
    const str_view view = sv_n(ref, 99);
    CHECK(len, sv_strlen(ref));
    CHECK(len, sv_len(view));
    CHECK(bytes, sv_strbytes(ref));
    CHECK(bytes, sv_svbytes(view));
    CHECK(len, sv_npos(view));
    CHECK(len, sv_minlen(ref, 99));
    const str_view view2 = sv_n(ref, -1);
    CHECK(len, sv_strlen(ref));
    CHECK(len, sv_len(view2));
    CHECK(bytes, sv_strbytes(ref));
    CHECK(bytes, sv_svbytes(view2));
    CHECK(len, (sv_npos(view2)));
    CHECK(len, sv_minlen(ref, 99));
    return PASS;
}
