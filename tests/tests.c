#include "string_view.h"

#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int run(void);

int
main()
{
    return run();
}

const char *const pass_msg = "pass";
const char *const fail_msg = "fail";

/* Set this breakpoint on any line where you wish
   execution to stop. Under normal program runs the program
   will simply exit. If triggered in GDB execution will stop
   while able to explore the surrounding context, varialbes,
   and stack frames. Be sure to step "(gdb) up" out of the
   raise function to wherever it triggered. */
#define breakpoint()                                                           \
    do                                                                         \
    {                                                                          \
        (void)fprintf(stderr, "\n!!Break. Line: %d File: %s, Func: %s\n ",     \
                      __LINE__, __FILE__, __func__);                           \
        (void)raise(SIGTRAP);                                                  \
    } while (0)

typedef bool (*test_fn)(void);
static bool test_empty(void);
static bool test_out_of_bounds(void);
static bool test_from_null(void);
static bool test_from_delim(void);
static bool test_from_delim_no_delim(void);
static bool test_empty_constructor(void);
static bool test_front_back(void);
static bool test_copy_fill(void);
static bool test_iter(void);
static bool test_iter_repeating_delim(void);
static bool test_iter_multichar_delim(void);
static bool test_iter_multichar_delim_short(void);
static bool test_iter_delim_larger_than_str(void);
static bool test_find_blank_of(void);
static bool test_prefix_suffix(void);
static bool test_substr(void);
static bool test_svcmp(void);
static bool test_argv_argc(void);
static bool test_mini_alloc_free(void);
static bool test_get_line(void);

#define NUM_TESTS (size_t)20
const test_fn all_tests[NUM_TESTS] = {
    test_empty,
    test_out_of_bounds,
    test_from_null,
    test_from_delim,
    test_from_delim_no_delim,
    test_empty_constructor,
    test_front_back,
    test_copy_fill,
    test_iter,
    test_iter_repeating_delim,
    test_iter_multichar_delim,
    test_iter_multichar_delim_short,
    test_iter_delim_larger_than_str,
    test_find_blank_of,
    test_prefix_suffix,
    test_svcmp,
    test_substr,
    test_argv_argc,
    test_mini_alloc_free,
    test_get_line,
};

static int
run(void)
{
    printf("\n");
    size_t pass_count = 0;
    for (size_t i = 0; i < NUM_TESTS; ++i)
    {
        const bool passed = all_tests[i]();
        pass_count += passed;
        passed ? printf("...%s\n", pass_msg) : printf("...%s\n", fail_msg);
    }
    printf("PASSED %zu/%zu %s\n\n", pass_count, NUM_TESTS,
           (pass_count == NUM_TESTS) ? "\\(*.*)/\n" : ">.<\n");
    return 0;
}

static bool
test_empty(void)
{
    printf("test_empty");
    if (!sv_empty(sv("")))
    {
        return false;
    }
    if (*sv_null() != '\0')
    {
        return false;
    }
    return true;
}

static bool
test_out_of_bounds(void)
{
    printf("test_out_of_bounds\n");
    const string_view s = sv("");
    const pid_t exiting_child = fork();
    if (exiting_child == 0)
    {
        (void)sv_at(s, 1);
        /* We should not make it here */
        exit(0);
    }
    int status = 0;
    if (waitpid(exiting_child, &status, 0) < 0)
    {
        printf("Error waiting for failing child.\n");
        exit(1);
    }
    /* We expect to have exited with a status of 1 */
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 1)
    {
        return false;
    }
    return true;
}

static bool
test_from_null(void)
{
    printf("test_from_null\n");
    const char *const reference = "Don't miss the terminator!";
    const string_view s = sv(reference);
    printf("reference=[%s]\n", reference);
    printf("string vw=[");
    sv_print(s);
    printf("]\n");
    const size_t reference_len = strlen(reference);
    if (reference_len != sv_len(s))
    {
        return false;
    }
    if (reference[reference_len - 1] != sv_at(s, sv_len(s) - 1))
    {
        return false;
    }
    const char *const chunk = "Don't";
    const size_t chunk_len = strlen(chunk);
    const string_view n_bytes = sv_n(reference, chunk_len);
    printf("5 bytes=[%s]\n", chunk);
    printf("5 byte view=[");
    sv_print(n_bytes);
    printf("]\n");
    if (sv_len(n_bytes) != chunk_len)
    {
        return false;
    }
    if (chunk[chunk_len - 1] != sv_at(n_bytes, sv_len(n_bytes) - 1))
    {
        return false;
    }
    return true;
}

static bool
test_from_delim(void)
{
    printf("test_from_delim\n");
    const char *const reference = "Don'tmissthedelim That was it!";
    const char *const reference_delim = "Don'tmissthedelim";
    const string_view sv = sv_delim(reference, " ");
    const size_t reference_len = strlen(reference_delim);
    printf("delimiter=[,]\n");
    printf("reference=[%s]\n", reference);
    printf("first tok=[%s]\n", reference_delim);
    printf("this should be first tok=[");
    sv_print(sv);
    printf("]\n");
    if (reference_len != sv_len(sv))
    {
        return false;
    }
    if (reference_delim[reference_len - 1] != sv_at(sv, sv_len(sv) - 1))
    {
        return false;
    }
    /* If the string starts with delim we must skip it. */
    const char *const ref2 = ",Don't miss the delim, that was it!";
    const char *const ref2_delim = "Don't miss the delim";
    const string_view sv2 = sv_delim(ref2, ",");
    const size_t ref2_len = strlen(ref2_delim);
    printf("delimiter=[,]\n");
    printf("reference=[%s]\n", ref2);
    printf("first tok=[%s]\n", ref2_delim);
    printf("this should be first tok=[");
    sv_print(sv2);
    printf("]\n");
    if (ref2_len != sv_len(sv2))
    {
        return false;
    }
    if (ref2_delim[ref2_len - 1] != sv_at(sv2, sv_len(sv2) - 1))
    {
        return false;
    }
    return true;
}

static bool
test_from_delim_no_delim(void)
{
    printf("test_from_delim_no_delim\n");
    const char *const reference = "Don'tmissthedelimbutnodelim!";
    const string_view sv = sv_delim(reference, " ");
    const size_t reference_len = strlen(reference);
    printf("delimiter=[ ]\n");
    printf("reference=[%s]\n", reference);
    printf("this should be reference=[");
    sv_print(sv);
    printf("]\n");
    if (reference_len != sv_len(sv))
    {
        return false;
    }
    if (reference[reference_len - 1] != sv_at(sv, sv_len(sv) - 1))
    {
        return false;
    }
    return true;
}

static bool
test_empty_constructor(void)
{
    printf("test_empty_constructor\n");
    const char *const reference = "------------";
    const string_view sv = sv_delim(reference, "-");
    const size_t reference_len = strlen(reference);
    printf("delimiter=[-]\n");
    printf("reference=[%s]\n", reference);
    printf("this should be empty=[");
    sv_print(sv);
    printf("]\n");
    if (reference_len == sv_len(sv) || !sv_empty(sv))
    {
        return false;
    }
    return true;
}

static bool
test_front_back(void)
{
    printf("test_front_back");
    if (sv_back(sv("")) != '\0' || sv_front(sv("")) != '\0')
    {
        return false;
    }
    const char *const reference = "*The front was * the back is!";
    const string_view s = sv(reference);
    const size_t ref_len = strlen(reference);
    if (ref_len != sv_len(s))
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
test_copy_fill(void)
{
    printf("test_copy_fill");
    const char *const reference = "Copy this over there!";
    string_view this = sv_copy(reference, strlen(reference));
    char there[strlen(reference) + 1];
    sv_fill(there, strlen(reference), this);
    if (strcmp(sv_str(this), there) != 0)
    {
        return false;
    }
    return true;
}

static bool
test_iter(void)
{
    printf("test_iter");
    const char *const reference = "A B C D E G H I J K L M N O P";
    string_view chars = sv(reference);
    size_t i = 0;
    for (const char *cur = sv_begin(&chars); cur != sv_end(&chars);
         cur = sv_next(cur))
    {
        if (*cur != reference[i])
        {
            return false;
        }
        ++i;
    }
    i = 0;
    /* This version should only give us the letters because delim is ' ' */
    string_view cur = sv_begin_tok(reference, 1, " ");
    for (; !sv_end_tok(&cur); cur = sv_next_tok(cur, 1, " "))
    {
        if (sv_front(cur) != reference[i])
        {
            return false;
        }
        i += 2;
    }
    if (*cur.s != '\0')
    {
        return false;
    }
    /* Do at least one token iteration if we can't find any delims */
    string_view cur2 = sv_begin_tok(reference, 1, ",");
    for (; !sv_end_tok(&cur2); cur2 = sv_next_tok(cur2, 1, ","))
    {
        if (strcmp(cur2.s, reference) != 0)
        {
            return false;
        }
    }
    if (*cur2.s != '\0')
    {
        return false;
    }
    return true;
}

static bool
test_iter_repeating_delim(void)
{
    printf("test_iter_repeating_delim");
    const char *toks[14] = {
        "A",  "B", "C", "D",   "E", "F",  "G",
        "HI", "J", "K", "LMN", "O", "Pi", "\\(*.*)/",
    };
    const char *const reference
        = " A   B  C     D  E F G HI J   K LMN O   Pi  \\(*.*)/  ";
    size_t i = 0;
    /* This version should only give us the letters because delim is ' ' */
    string_view cur = sv_begin_tok(reference, 1, " ");
    for (; !sv_end_tok(&cur); cur = sv_next_tok(cur, 1, " "))
    {
        if (sv_strcmp(cur, toks[i]) != 0)
        {
            return false;
        }
        ++i;
    }
    if (*cur.s != '\0')
    {
        return false;
    }
    /* Do at least one token iteration if we can't find any delims */
    string_view cur2 = sv_begin_tok(reference, 1, ",");
    for (; !sv_end_tok(&cur2); cur2 = sv_next_tok(cur2, 1, ","))
    {
        if (strcmp(cur2.s, reference) != 0)
        {
            return false;
        }
    }
    if (*cur2.s != '\0')
    {
        return false;
    }
    return true;
}

static bool
test_iter_multichar_delim(void)
{
    printf("test_iter_multichar_delim");
    const char *toks[14] = {
        "A",     "B", "C", "D",      "E", "F",  "G",
        "HacbI", "J", "K", "LcbaMN", "O", "Pi", "\\(*.*)/",
    };
    const char *const reference
        = "abcAabcBabcCabcabcabcDabcEabcFabcGabcHacbIabcJabcabcabcabcKabcLcbaMN"
          "abcOabcabcPiabcabc\\(*.*)/abc";
    size_t i = 0;
    /* This version should only give us the letters because delim is ' ' */
    const char *const delim = "abc";
    const size_t delim_len = strlen(delim);
    string_view cur = sv_begin_tok(reference, delim_len, delim);
    for (; !sv_end_tok(&cur); cur = sv_next_tok(cur, delim_len, delim))
    {
        if (sv_strcmp(cur, toks[i]) != 0)
        {
            return false;
        }
        ++i;
    }
    if (*cur.s != '\0')
    {
        return false;
    }
    /* Do at least one token iteration if we can't find any delims */
    string_view cur2 = sv_begin_tok(reference, 1, " ");
    for (; !sv_end_tok(&cur2); cur2 = sv_next_tok(cur2, 1, " "))
    {
        if (strcmp(cur2.s, reference) != 0)
        {
            return false;
        }
    }
    if (*cur2.s != '\0')
    {
        return false;
    }
    return true;
}

static bool
test_iter_multichar_delim_short(void)
{
    printf("test_iter_multichar_delim_close");
    const char *toks[14] = {
        "A",     "B", "C", "D",      "E",   "F",  "G",
        "H---I", "J", "K", "L-M--N", "--O", "Pi", "\\(*.*)/",
    };
    const char *const reference = "-----A-----B-----C-----D-----E-----F-----G--"
                                  "---H---I-----J-----K-----L-M--N"
                                  "-------O-----Pi-----\\(*.*)/-----";
    size_t i = 0;
    /* This version should only give us the letters because delim is ' ' */
    const char *const delim = "-----";
    const size_t delim_len = strlen(delim);
    string_view cur = sv_begin_tok(reference, delim_len, delim);
    for (; !sv_end_tok(&cur); cur = sv_next_tok(cur, delim_len, delim))
    {
        if (sv_strcmp(cur, toks[i]) != 0)
        {
            return false;
        }
        ++i;
    }
    if (*cur.s != '\0')
    {
        return false;
    }
    /* Do at least one token iteration if we can't find any delims */
    string_view cur2 = sv_begin_tok(reference, 1, " ");
    for (; !sv_end_tok(&cur2); cur2 = sv_next_tok(cur2, 1, " "))
    {
        if (strcmp(cur2.s, reference) != 0)
        {
            return false;
        }
    }
    if (*cur2.s != '\0')
    {
        return false;
    }
    return true;
}

static bool
test_iter_delim_larger_than_str(void)
{
    printf("test_iter_delim_larger_than_str");
    const char *const reference = "A-B";
    /* This delimeter is too large so we should just take the whole string */
    const char *const delim = "-----";
    const size_t delim_len = strlen(delim);
    string_view constructed = sv_delim(reference, delim);
    string_view cur = sv_begin_tok(reference, delim_len, delim);
    if (sv_svcmp(constructed, cur) != 0
        || sv_strcmp(constructed, reference) != 0
        || sv_strcmp(cur, reference) != 0)
    {
        return false;
    }
    for (; !sv_end_tok(&cur); cur = sv_next_tok(cur, delim_len, delim))
    {
        if (sv_strcmp(cur, reference) != 0)
        {
            return false;
        }
    }
    if (*cur.s != '\0')
    {
        return false;
    }
    return true;
}

static bool
test_find_blank_of(void)
{
    printf("test_find_blank_of");
    const char ref[20] = {
        [0] = 'A',  [1] = 'A',  [2] = 'C',  [3] = ' ',  [4] = '!',
        [5] = '!',  [6] = '!',  [7] = ' ',  [8] = '*',  [9] = '*',
        [10] = ' ', [11] = '_', [12] = '_', [13] = ' ', [14] = '!',
        [15] = '!', [16] = '!', [17] = ' ', [18] = 'A', [19] = '\0',
    };
    string_view str = sv(ref);
    if (sv_find_first_of(str, "C") != 2)
    {
        return false;
    }
    if (sv_find_first_of(str, "") != 0)
    {
        return false;
    }
    if (sv_find_last_of(str, "!") != 16)
    {
        return false;
    }
    if (sv_find_first_not_of(str, "A", 1) != 2)
    {
        return false;
    }
    if (sv_find_last_not_of(str, " ") != 18)
    {
        return false;
    }
    return true;
}

static bool
test_prefix_suffix(void)
{
    printf("test_prefix_suffix");
    const char *const reference = "Remove the suffix! No, remove the prefix!";
    const char *const ref_prefix = "Remove the suffix!";
    const char *const ref_suffix = "No, remove the prefix!";
    string_view entire_string = sv(reference);
    string_view prefix = sv_remove_suffix(entire_string, 23);
    size_t i = 0;
    for (const char *c = sv_begin(&prefix); c != sv_end(&prefix);
         c = sv_next(c))
    {
        if (*c != ref_prefix[i])
        {
            return false;
        }
        ++i;
    }
    i = 0;
    const string_view suffix = sv_remove_prefix(entire_string, 19);
    for (const char *c = sv_begin(&suffix); c != sv_end(&suffix);
         c = sv_next(c))
    {
        if (*c != ref_suffix[i])
        {
            return false;
        }
        ++i;
    }
    if (!sv_empty(sv_remove_prefix(entire_string, ULLONG_MAX)))
    {
        return false;
    }
    if (!sv_empty(sv_remove_suffix(entire_string, ULLONG_MAX)))
    {
        return false;
    }
    return true;
}

static bool
test_svcmp(void)
{
    printf("test_svcmp");
    if (sv_svcmp(sv(""), sv("")) != 0)
    {
        return false;
    }
    if (sv_strcmp(sv(""), "") != 0)
    {
        return false;
    }
    if (sv_svcmp(sv("same"), sv("same")) != 0)
    {
        return false;
    }
    if (sv_svcmp(sv("samz"), sv("same")) <= 0)
    {
        return false;
    }
    if (sv_svcmp(sv("same"), sv("samz")) >= 0)
    {
        return false;
    }
    /* The comparison function should treat the end of a string view as
       null terminating character even if it points to a delimeter */
    if (sv_svcmp(sv("same"), sv_delim("same same", " ")) != 0)
    {
        return false;
    }
    if (sv_svcmp(sv("same"), sv_delim("samz same", " ")) >= 0)
    {
        return false;
    }
    if (sv_svcmp(sv_delim("sameez same", " "), sv("same")) <= 0)
    {
        return false;
    }
    const char *const str = "same";
    if (sv_strcmp(sv(str), str) != 0)
    {
        return false;
    }
    if (sv_strcmp(sv_delim("same same", " "), str) != 0)
    {
        return false;
    }
    if (sv_strcmp(sv_delim("samez same", " "), str) <= 0)
    {
        return false;
    }
    if (sv_strcmp(sv_delim("sameez same", " "), str) <= 0)
    {
        return false;
    }
    /* strncmp compares at most n bytes inclusize or stops at null term */
    if (sv_strncmp(sv_delim("sameez same", " "), str, 10) <= 0)
    {
        return false;
    }
    if (sv_strncmp(sv_delim("saaeez same", " "), str, 3) >= 0)
    {
        return false;
    }
    return true;
}

static bool
test_substr(void)
{
    printf("test_substr");
    const char ref[27] = {
        [0] = 'A',  [1] = ' ',  [2] = 's',   [3] = 'u',  [4] = 'b',  [5] = 's',
        [6] = 't',  [7] = 'r',  [8] = 'i',   [9] = 'n',  [10] = 'g', [11] = '!',
        [12] = ' ', [13] = 'H', [14] = 'a',  [15] = 'v', [16] = 'e', [17] = ' ',
        [18] = 'a', [19] = 'n', [20] = 'o',  [21] = 't', [22] = 'h', [23] = 'e',
        [24] = 'r', [25] = '!', [26] = '\0',
    };
    const char *const substr1 = "A substring!";
    const char *const substr2 = "Have another!";
    if (sv_strcmp(sv_substr(sv(ref), 0, strlen(substr1)), substr1) != 0)
    {
        return false;
    }
    if (sv_strcmp(sv_substr(sv(ref), strlen(substr1) + 1, strlen(substr2)),
                  substr2)
        != 0)
    {
        return false;
    }
    if (sv_strcmp(sv_substr(sv(ref), 0, ULLONG_MAX), ref) != 0)
    {
        return false;
    }
    /* Make sure the fill function adds null terminator */
    char dump_substr1[27] = {[12] = '!'};
    sv_fill(dump_substr1, 27, sv_substr(sv(ref), 0, strlen(substr1)));
    if (strcmp(substr1, dump_substr1) != 0)
    {
        return false;
    }
    /* Make sure the fill function adds null terminator */
    char dump_substr2[27] = {[13] = '!'};
    sv_fill(dump_substr2, 27,
            sv_substr(sv(ref), strlen(substr1) + 1, strlen(substr2)));
    if (strcmp(substr2, dump_substr2) != 0)
    {
        return false;
    }
    return true;
}

static bool
test_argv_argc(void)
{
    printf("test_argv_argc");
    const char *const expected[4]
        = {"./build/targets/tidy", "--source=file", "-i", "lib/string_view.c"};
    const void *const buf_data
        = "./build/targets/tidy  --source=file   -i   lib/string_view.c\0";
    /* This could be heap or stack allocated. Here we do stack space for
     * convenience testing */
    char argv[10][128];
    string_view view = sv(buf_data);
    size_t i = 0;
    for (string_view v = sv_begin_tok(sv_str(view), 1, " "); !sv_end_tok(&v);
         v = sv_next_tok(v, 1, " "))
    {
        sv_fill(argv[i], sv_len(v), v);
        ++i;
    }
    for (size_t arg = 0; arg < 4; ++arg)
    {
        if (strcmp(argv[arg], expected[arg]) != 0)
        {
            return false;
        }
    }
    return true;
}

static bool
test_mini_alloc_free(void)
{
    printf("test_mini_alloc_free");
    const char *const expected[4]
        = {"./build/targets/tidy", "--source=file", "-i", "lib/string_view.c"};
    const void *const buf_data
        = "./build/targets/tidy  --source=file   -i   lib/string_view.c\0";
    /* This could be heap or stack allocated. Here we do stack space for
     * convenience testing */
    string_view argv[10];
    string_view view = sv(buf_data);
    size_t i = 0;
    for (string_view v = sv_begin_tok(sv_str(view), 1, " "); !sv_end_tok(&v);
         v = sv_next_tok(v, 1, " "))
    {
        argv[i++] = v;
    }
    for (size_t arg = 0; arg < 4; ++arg)
    {
        char *str = sv_alloc(argv[arg]);
        if (!str)
        {
            return false;
        }
        const bool passes = strcmp(str, expected[arg]) == 0;
        sv_free(str);
        if (!passes)
        {
            return false;
        }
    }
    return true;
}

static bool
test_get_line(void)
{
    printf("test_get_line");
    /* We will pretend like we used getline to read this into a heap
       allocated buffer from a file and now we are reading/parsing
       minus the correct EOF handling possibly. */
    const char *lines[5] = {"1", "2", "3", "4", "5"};
    const char *const file_data = "1\n2\n3\n4\n5";
    size_t i = 0;
    for (string_view v = sv_begin_tok(file_data, 1, "\n"); !sv_end_tok(&v);
         v = sv_next_tok(v, 1, "\n"))
    {
        if (sv_strcmp(v, lines[i]) != 0)
        {
            return false;
        }
        ++i;
    }
    if (i != 5)
    {
        return false;
    }
    return true;
}
