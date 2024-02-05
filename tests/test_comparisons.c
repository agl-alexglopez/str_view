#include "str_view.h"
#include "test.h"
#include <string.h>

static enum test_result test_compare_single(void);
static enum test_result test_compare_equal(void);
static enum test_result test_compare_equal_view(void);
static enum test_result test_compare_terminated(void);
static enum test_result test_compare_different_lengths_terminated(void);
static enum test_result test_compare_different_lengths_views(void);
static enum test_result test_compare_misc(void);

#define NUM_TESTS (size_t)7
const struct fn_name all_tests[NUM_TESTS] = {
    {test_compare_single, "test_compare_single"},
    {test_compare_equal, "test_compare_equal"},
    {test_compare_equal_view, "test_compare_equal_view"},
    {test_compare_terminated, "test_compare_terminated"},
    {test_compare_different_lengths_terminated,
     "test_compare_different_lengths_terminated"},
    {test_compare_different_lengths_views,
     "test_compare_different_lengths_views"},
    {test_compare_misc, "test_compare_misc"},
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
test_compare_single(void)
{
    const char e1[2] = {
        [0] = 'A',
        [1] = '\0',
    };
    const char e2[2] = {
        [0] = 'B',
        [1] = '\0',
    };
    const str_view e1_view = sv(e1);
    const str_view e2_view = sv(e2);
    const int cmp_res = strcmp(e1, e2);
    const int cmp_res2 = strcmp(e2, e1);
    if (cmp_res != sv_strcmp(e1_view, e2)
        || cmp_res != sv_svcmp(e1_view, e2_view)
        || cmp_res2 != sv_strcmp(e2_view, e1)
        || cmp_res2 != sv_svcmp(e2_view, e1_view))
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_compare_equal(void)
{
    const char e1[5] = {
        [0] = 'N', [1] = 'I', [2] = 'C', [3] = 'E', [4] = '\0',
    };
    const char e2[5] = {
        [0] = 'N', [1] = 'I', [2] = 'C', [3] = 'E', [4] = '\0',
    };
    const str_view e1_view = sv(e1);
    const str_view e2_view = sv(e2);
    const int cmp_res = strcmp(e1, e2);
    const int cmp_res2 = strcmp(e2, e1);
    if (cmp_res != sv_strcmp(e1_view, e2)
        || cmp_res != sv_svcmp(e1_view, e2_view)
        || cmp_res2 != sv_strcmp(e2_view, e1)
        || cmp_res2 != sv_svcmp(e2_view, e1_view))
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_compare_equal_view(void)
{
    const char e1[5] = {
        [0] = 'N', [1] = 'I', [2] = 'C', [3] = 'E', [4] = '\0',
    };
    const char e2[9] = {
        [0] = 'N', [1] = 'I', [2] = 'C', [3] = 'E',  [4] = 'N',
        [5] = 'E', [6] = 'S', [7] = 'S', [8] = '\0',
    };
    const str_view e1_view = sv(e1);
    const str_view e2_view = sv_n(e2, sv_strlen(e1));
    const int cmp_res = strcmp(e1, e1);
    const int cmp_res2 = strcmp(e1, e1);
    if (cmp_res != sv_svcmp(e1_view, e2_view)
        || cmp_res2 != sv_strcmp(e2_view, e1)
        || cmp_res2 != sv_svcmp(e2_view, e1_view))
    {
        return FAIL;
    }
    return PASS;
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
    const int cmp_res2 = strcmp(greater, lesser);
    if (cmp_res != sv_strcmp(less_view, greater)
        || cmp_res != sv_svcmp(less_view, greater_view)
        || cmp_res2 != sv_strcmp(greater_view, lesser)
        || cmp_res2 != sv_svcmp(greater_view, less_view))
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_compare_different_lengths_terminated(void)
{
    const char lesser[5]
        = {[0] = 'A', [1] = 'A', [2] = 'A', [3] = 'A', [4] = '\0'};
    const char greater[3] = {[0] = 'A', [1] = 'A', [2] = '\0'};
    const str_view less_view = sv(lesser);
    const str_view greater_view = sv(greater);
    const int cmp_res = strcmp(lesser, greater);
    const int cmp_res2 = strcmp(greater, lesser);
    if (cmp_res != sv_strcmp(less_view, greater)
        || cmp_res != sv_svcmp(less_view, greater_view)
        || cmp_res2 != sv_strcmp(greater_view, lesser)
        || cmp_res2 != sv_svcmp(greater_view, less_view))
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_compare_different_lengths_views(void)
{

    const char lesser[5] = {
        [0] = 'A', [1] = 'A', [2] = 'A', [3] = 'A', [4] = '\0',
    };
    const char greater_str[5] = {
        [0] = 'A', [1] = 'A', [2] = 'A', [3] = 'B', [4] = '\0',
    };
    const char greater_longer[9] = {
        [0] = 'A', [1] = 'A', [2] = 'A', [3] = 'B',  [4] = 'A',
        [5] = 'B', [6] = 'Y', [7] = 'Z', [8] = '\0',
    };
    const int str_cmp = strcmp(lesser, greater_str);
    const int str_cmp2 = strcmp(greater_str, lesser);
    const str_view greater_view = sv_n(greater_longer, sv_strlen(greater_str));
    const str_view lesser_view = sv(lesser);
    if (str_cmp2 != sv_strcmp(greater_view, lesser)
        || str_cmp != sv_strcmp(lesser_view, greater_str)
        || str_cmp != sv_svcmp(lesser_view, greater_view)
        || str_cmp2 != sv_svcmp(greater_view, lesser_view))
    {
        return FAIL;
    }
    return PASS;
}

static enum test_result
test_compare_misc(void)
{
    if (sv_svcmp(sv(""), sv("")) != EQL)
    {
        return FAIL;
    }
    if (sv_strcmp(sv(""), "") != EQL)
    {
        return FAIL;
    }
    if (sv_svcmp(sv("same"), sv("same")) != EQL)
    {
        return FAIL;
    }
    if (sv_svcmp(sv("samz"), sv("same")) <= EQL)
    {
        return FAIL;
    }
    if (sv_svcmp(sv("same"), sv("samz")) >= EQL)
    {
        return FAIL;
    }
    /* The comparison function should treat the end of a string view as
       null terminating character even if it points to a delimeter */
    if (sv_svcmp(sv("same"), sv_delim("same same", " ")) != EQL)
    {
        return FAIL;
    }
    if (sv_svcmp(sv("same"), sv_delim("samz same", " ")) >= EQL)
    {
        return FAIL;
    }
    if (sv_svcmp(sv_delim("sameez same", " "), sv("same")) <= EQL)
    {
        return FAIL;
    }
    const char *const str = "same";
    if (sv_strcmp(sv(str), str) != 0)
    {
        return FAIL;
    }
    if (sv_strcmp(sv_delim("same same", " "), str) != EQL)
    {
        return FAIL;
    }
    if (sv_strcmp(sv_delim("samez same", " "), str) <= EQL)
    {
        return FAIL;
    }
    if (sv_strcmp(sv_delim("sameez same", " "), str) <= EQL)
    {
        return FAIL;
    }
    /* strncmp compares at most n bytes inclusize or stops at null term */
    if (sv_strncmp(sv_delim("sameez same", " "), str, 10) <= EQL)
    {
        return FAIL;
    }
    if (sv_strncmp(sv_delim("saaeez same", " "), str, 3) >= EQL)
    {
        return FAIL;
    }
    return PASS;
}
