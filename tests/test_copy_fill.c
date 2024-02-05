#include "str_view.h"
#include "test.h"
#include <string.h>

static enum test_result test_copy_fill(void);
static enum test_result test_copy_section(void);

#define NUM_TESTS (size_t)2
const struct fn_name all_tests[NUM_TESTS] = {
    {test_copy_fill, "fill a buffer and null terminate"},
    {test_copy_section, "copy a section of string and fill buffer"},
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
            printf("test_copy_fill test failed: %s\n", all_tests[i].name);
            res = FAIL;
        }
    }
    return res;
}

static enum test_result
test_copy_fill(void)
{
    const char *const reference = "Copy this over there!";
    str_view this = sv_copy(reference, strlen(reference));
    char there[sv_strbytes(reference)];
    if (sv_fill(there, sizeof there, this) != sizeof there)
    {
        return FAIL;
    }
    if (strcmp(sv_begin(this), there) != 0)
    {
        return FAIL;
    }
    if (strlen(there) != sv_svlen(this))
    {
        return FAIL;
    }
    if (there[(sizeof there) - 1] != '\0')
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_copy_section(void)
{
    const char ref[20] = {
        [0] = 'A',  [1] = 'A',  [2] = 'C',  [3] = ' ',  [4] = '!',
        [5] = 's',  [6] = 'n',  [7] = 'i',  [8] = 'p',  [9] = '!',
        [10] = ' ', [11] = '_', [12] = '_', [13] = ' ', [14] = '!',
        [15] = '!', [16] = '!', [17] = ' ', [18] = 'A', [19] = '\0',
    };
    const char *const expected_snip = "snip!";
    const str_view ref_view = sv(ref);
    const str_view snip = sv_substr(ref_view, 5, 5);
    char snip_buf[sv_svbytes(snip)];
    if (sv_fill(snip_buf, sizeof snip_buf, snip) != sv_svbytes(snip))
    {
        return FAIL;
    }
    if (strcmp(expected_snip, snip_buf) != 0)
    {
        return FAIL;
    }
    if (snip_buf[(sizeof snip_buf) - 1] != '\0')
    {
        return FAIL;
    }
    return PASS;
}
