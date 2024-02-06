#include "str_view.h"
#include "test.h"

#include <limits.h>
#include <stdio.h>

static enum test_result test_prefix_suffix(void);
static enum test_result test_substr(void);

#define NUM_TESTS (size_t)2
const struct fn_name all_tests[NUM_TESTS] = {
    {test_prefix_suffix, "test_prefix_suffix"},
    {test_substr, "test_substr"},
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
            printf("test_view_editing test failed: %s\n", all_tests[i].name);
            res = FAIL;
        }
    }
    return res;
}

static enum test_result
test_prefix_suffix(void)
{
    const char *const reference = "Remove the suffix! No, remove the prefix!";
    const char *const ref_prefix = "Remove the suffix!";
    const char *const ref_suffix = "No, remove the prefix!";
    str_view entire_string = sv(reference);
    str_view prefix = sv_remove_suffix(entire_string, 23);
    size_t i = 0;
    for (const char *c = sv_begin(prefix); c != sv_end(prefix); c = sv_next(c))
    {
        if (*c != ref_prefix[i])
        {
            return FAIL;
        }
        ++i;
    }
    i = 0;
    const str_view suffix = sv_remove_prefix(entire_string, 19);
    for (const char *c = sv_begin(suffix); c != sv_end(suffix); c = sv_next(c))
    {
        if (*c != ref_suffix[i])
        {
            return FAIL;
        }
        ++i;
    }
    if (!sv_empty(sv_remove_prefix(entire_string, ULLONG_MAX)))
    {
        return FAIL;
    }
    if (!sv_empty(sv_remove_suffix(entire_string, ULLONG_MAX)))
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_substr(void)
{
    const char ref[27] = {
        [0] = 'A',  [1] = ' ',  [2] = 's',   [3] = 'u',  [4] = 'b',  [5] = 's',
        [6] = 't',  [7] = 'r',  [8] = 'i',   [9] = 'n',  [10] = 'g', [11] = '!',
        [12] = ' ', [13] = 'H', [14] = 'a',  [15] = 'v', [16] = 'e', [17] = ' ',
        [18] = 'a', [19] = 'n', [20] = 'o',  [21] = 't', [22] = 'h', [23] = 'e',
        [24] = 'r', [25] = '!', [26] = '\0',
    };
    const char *const substr1 = "A substring!";
    const char *const substr2 = "Have another!";
    const str_view substr1_view = sv(substr1);
    const str_view substr2_view = sv(substr2);
    if (sv_strcmp(sv_substr(sv(ref), 0, sv_strlen(substr1)), substr1) != EQL)
    {
        return FAIL;
    }
    if (sv_strcmp(
            sv_substr(sv(ref), sv_strlen(substr1) + 1, sv_strlen(substr2)),
            substr2)
        != 0)
    {
        return FAIL;
    }
    if (sv_strcmp(sv_substr(sv(ref), 0, ULLONG_MAX), ref) != EQL)
    {
        return FAIL;
    }
    /* Make sure the fill function adds null terminator */
    char dump_substr1[27] = {[13] = '@'};
    (void)sv_fill(dump_substr1, 27, sv_substr(sv(ref), 0, sv_strlen(substr1)));
    if (sv_strcmp(substr1_view, dump_substr1) != EQL)
    {
        return FAIL;
    }
    /* Make sure the fill function adds null terminator */
    char dump_substr2[27] = {[14] = '@'};
    (void)sv_fill(
        dump_substr2, 27,
        sv_substr(sv(ref), sv_strlen(substr1) + 1, sv_strlen(substr2)));
    if (sv_strcmp(substr2_view, dump_substr2) != EQL)
    {
        return FAIL;
    }
    return PASS;
}
