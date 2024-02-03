#include "str_view.h"

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

const char *const pass_msg = "⬤";
const char *const fail_msg = "X";
const char *const red = "\033[38;5;9m";
const char *const green = "\033[38;5;10m";
const char *const cyan = "\033[38;5;14m";
const char *const none = "\033[0m";

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
static bool test_find_rfind(void);
static bool test_find_of_sets(void);
static bool test_prefix_suffix(void);
static bool test_substr(void);
static bool test_svcmp(void);
static bool test_argv_argc(void);
static bool test_get_line(void);
static bool test_substring_search(void);

#define NUM_TESTS (size_t)21
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
    test_find_rfind,
    test_find_of_sets,
    test_prefix_suffix,
    test_svcmp,
    test_substr,
    test_argv_argc,
    test_get_line,
    test_substring_search,
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
        passed ? printf("%s...%s%s%s\n", cyan, green, pass_msg, none)
               : printf("%s...%s%s%s\n", cyan, red, fail_msg, none);
    }
    if (pass_count == NUM_TESTS)
    {
        printf("%sPASSED %zu/%zu \\(*.*)/%s\n\n", green, pass_count, NUM_TESTS,
               none);
    }
    else
    {
        printf("%sPASSED %zu/%zu T_T%s\n\n", red, pass_count, NUM_TESTS, none);
    }
    return 0;
}

static bool
test_empty(void)
{
    printf("%stest_empty...%s", cyan, none);
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
    printf("%stest_out_of_bounds...\n%s", cyan, none);
    const str_view s = sv("");
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
    printf("%stest_from_null...\n%s", cyan, none);
    const char *const reference = "Don't miss the terminator!";
    const str_view s = sv(reference);
    printf("reference=[%s]\n", reference);
    printf("string vw=[");
    sv_print(s);
    printf("]\n");
    const size_t reference_len = strlen(reference);
    if (reference_len != sv_svlen(s))
    {
        return false;
    }
    if (reference[reference_len - 1] != sv_at(s, sv_svlen(s) - 1))
    {
        return false;
    }
    const char *const chunk = "Don't";
    const size_t chunk_len = strlen(chunk);
    const str_view n_bytes = sv_n(reference, chunk_len);
    printf("5 bytes=[%s]\n", chunk);
    printf("5 byte view=[");
    sv_print(n_bytes);
    printf("]\n");
    if (sv_svlen(n_bytes) != chunk_len)
    {
        return false;
    }
    if (chunk[chunk_len - 1] != sv_at(n_bytes, sv_svlen(n_bytes) - 1))
    {
        return false;
    }
    return true;
}

static bool
test_from_delim(void)
{
    printf("%stest_from_delim...\n%s", cyan, none);
    const char *const reference = "Don'tmissthedelim That was it!";
    const char *const reference_delim = "Don'tmissthedelim";
    const str_view sv = sv_delim(reference, " ");
    const size_t reference_len = strlen(reference_delim);
    printf("delimiter=[,]\n");
    printf("reference=[%s]\n", reference);
    printf("first tok=[%s]\n", reference_delim);
    printf("this should be first tok=[");
    sv_print(sv);
    printf("]\n");
    if (reference_len != sv_svlen(sv))
    {
        return false;
    }
    if (reference_delim[reference_len - 1] != sv_at(sv, sv_svlen(sv) - 1))
    {
        return false;
    }
    /* If the string starts with delim we must skip it. */
    const char *const ref2 = ",Don't miss the delim, that was it!";
    const char *const ref2_delim = "Don't miss the delim";
    const str_view sv2 = sv_delim(ref2, ",");
    const size_t ref2_len = strlen(ref2_delim);
    printf("delimiter=[,]\n");
    printf("reference=[%s]\n", ref2);
    printf("first tok=[%s]\n", ref2_delim);
    printf("this should be first tok=[");
    sv_print(sv2);
    printf("]\n");
    if (ref2_len != sv_svlen(sv2))
    {
        return false;
    }
    if (ref2_delim[ref2_len - 1] != sv_at(sv2, sv_svlen(sv2) - 1))
    {
        return false;
    }
    return true;
}

static bool
test_from_delim_no_delim(void)
{
    printf("%stest_from_delim_no_delim...\n%s", cyan, none);
    const char *const reference = "Don'tmissthedelimbutnodelim!";
    const str_view sv = sv_delim(reference, " ");
    const size_t reference_len = strlen(reference);
    printf("delimiter=[ ]\n");
    printf("reference=[%s]\n", reference);
    printf("this should be reference=[");
    sv_print(sv);
    printf("]\n");
    if (reference_len != sv_svlen(sv))
    {
        return false;
    }
    if (reference[reference_len - 1] != sv_at(sv, sv_svlen(sv) - 1))
    {
        return false;
    }
    return true;
}

static bool
test_empty_constructor(void)
{
    printf("%stest_empty_constructor...\n%s", cyan, none);
    const char *const reference = "------------";
    const str_view sv = sv_delim(reference, "-");
    const size_t reference_len = strlen(reference);
    printf("delimiter=[-]\n");
    printf("reference=[%s]\n", reference);
    printf("this should be empty=[");
    sv_print(sv);
    printf("]\n");
    if (reference_len == sv_svlen(sv) || !sv_empty(sv))
    {
        return false;
    }
    return true;
}

static bool
test_front_back(void)
{
    printf("%stest_front_back...%s", cyan, none);
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
test_copy_fill(void)
{
    printf("%stest_copy_fill...%s", cyan, none);
    const char *const reference = "Copy this over there!";
    str_view this = sv_copy(reference, strlen(reference));
    char there[sv_strbytes(reference)];
    sv_fill(there, sv_strbytes(reference), this);
    if (strcmp(sv_begin(this), there) != 0)
    {
        return false;
    }
    return true;
}

static bool
test_iter(void)
{
    printf("%stest_iter...%s", cyan, none);
    const char *const reference = "A B C D E G H I J K L M N O P";
    str_view chars = sv(reference);
    size_t i = 0;
    for (const char *cur = sv_begin(chars); cur != sv_end(chars);
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
    str_view cur = sv_begin_tok(chars, (str_view){" ", 1});
    for (; !sv_end_tok(cur); cur = sv_next_tok(cur, (str_view){" ", 1}))
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
    str_view cur2 = sv_begin_tok(chars, (str_view){",", 1});
    for (; !sv_end_tok(cur2); cur2 = sv_next_tok(cur2, (str_view){",", 1}))
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
    printf("%stest_iter_repeating_delim...%s", cyan, none);
    const char *toks[14] = {
        "A",  "B", "C", "D",   "E", "F",  "G",
        "HI", "J", "K", "LMN", "O", "Pi", "\\(*.*)/",
    };
    const char *const reference
        = " A   B  C     D  E F G HI J   K LMN O   Pi  \\(*.*)/  ";
    const str_view ref_view = sv(reference);
    size_t i = 0;
    /* This version should only give us the letters because delim is ' ' */
    str_view cur = sv_begin_tok(ref_view, (str_view){" ", 1});
    for (; !sv_end_tok(cur); cur = sv_next_tok(cur, (str_view){" ", 1}))
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
    str_view cur2 = sv_begin_tok(ref_view, (str_view){",", 1});
    for (; !sv_end_tok(cur2); cur2 = sv_next_tok(cur2, (str_view){",", 1}))
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
    printf("%stest_iter_multichar_delim...%s", cyan, none);
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
    const str_view ref_view = sv(reference);
    str_view cur = sv_begin_tok(ref_view, (str_view){delim, delim_len});
    for (; !sv_end_tok(cur);
         cur = sv_next_tok(cur, (str_view){delim, delim_len}))
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
    str_view cur2 = sv_begin_tok(ref_view, (str_view){" ", 1});
    for (; !sv_end_tok(cur2); cur2 = sv_next_tok(cur2, (str_view){" ", 1}))
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
    printf("%stest_iter_multichar_delim_close...%s", cyan, none);
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
    const str_view ref_view = sv(reference);
    str_view cur = sv_begin_tok(ref_view, (str_view){delim, delim_len});
    for (; !sv_end_tok(cur);
         cur = sv_next_tok(cur, (str_view){delim, delim_len}))
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
    str_view cur2 = sv_begin_tok(ref_view, (str_view){" ", 1});
    for (; !sv_end_tok(cur2); cur2 = sv_next_tok(cur2, (str_view){" ", 1}))
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
    printf("%stest_iter_delim_larger_than_str...%s", cyan, none);
    const char *const reference = "A-B";
    /* This delimeter is too large so we should just take the whole string */
    const char *const delim = "-----";
    const size_t delim_len = strlen(delim);
    str_view constructed = sv_delim(reference, delim);
    str_view cur = sv_begin_tok((str_view){reference, sv_strlen(reference)},
                                (str_view){delim, delim_len});
    if (sv_svcmp(constructed, cur) != EQL
        || sv_strcmp(constructed, reference) != EQL
        || sv_strcmp(cur, reference) != EQL)
    {
        return false;
    }
    for (; !sv_end_tok(cur);
         cur = sv_next_tok(cur, (str_view){delim, delim_len}))
    {
        if (sv_strcmp(cur, reference) != EQL)
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
test_find_rfind(void)
{
    printf("%stest_find_rfind...%s", cyan, none);
    const char ref[20] = {
        [0] = 'A',  [1] = 'A',  [2] = 'C',  [3] = ' ',  [4] = '!',
        [5] = '!',  [6] = '!',  [7] = ' ',  [8] = '*',  [9] = '*',
        [10] = ' ', [11] = '_', [12] = '_', [13] = ' ', [14] = '!',
        [15] = '!', [16] = '!', [17] = ' ', [18] = 'A', [19] = '\0',
    };
    str_view str = sv(ref);
    if (sv_find(str, 0,
                (str_view){
                    .s = "C",
                    .sz = 1,
                })
        != 2)
    {
        return false;
    }
    if (sv_find(str, 0,
                (str_view){
                    .s = "",
                    .sz = 1,
                })
        != 19)
    {
        return false;
    }
    if (sv_rfind(str, str.sz,
                 (str_view){
                     .s = "!",
                     .sz = 1,
                 })
        != 16)
    {
        return false;
    }
    return true;
}

static bool
test_find_of_sets(void)
{
    printf("%stest_find_of_sets...%s", cyan, none);
    const char ref[25] = {
        [0] = 'A',  [1] = 'A',  [2] = 'C',  [3] = 'B',  [4] = '!',
        [5] = '!',  [6] = '!',  [7] = ' ',  [8] = '*',  [9] = '.',
        [10] = ':', [11] = ';', [12] = ',', [13] = ' ', [14] = '?',
        [15] = ' ', [16] = '_', [17] = '_', [18] = ' ', [19] = '!',
        [20] = '!', [21] = '!', [22] = 'Z', [23] = 'z', [24] = '\0',
    };
    str_view str = sv(ref);
    if (sv_find_first_of(str,
                         (str_view){
                             .s = "CB!",
                             .sz = strlen("CB!"),
                         })
        != 2)
    {
        return false;
    }
    if (sv_find_first_of(str,
                         (str_view){
                             .s = "",
                             .sz = 0,
                         })
        != 24)
    {
        return false;
    }
    if (sv_find_last_of(str,
                        (str_view){
                            .s = "! _",
                            .sz = strlen("! _"),
                        })
        != 21)
    {
        return false;
    }
    if (sv_find_last_not_of(str,
                            (str_view){
                                .s = "CBA!",
                                .sz = strlen("CBA!"),
                            })
        != 22)
    {
        return false;
    }
    if (sv_find_first_not_of(str,
                             (str_view){
                                 .s = "ACB!;:, *.",
                                 .sz = strlen("ACB!;:, *."),
                             })
        != 14)
    {
        return false;
    }
    return true;
}

static bool
test_prefix_suffix(void)
{
    printf("%stest_prefix_suffix...%s", cyan, none);
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
            return false;
        }
        ++i;
    }
    i = 0;
    const str_view suffix = sv_remove_prefix(entire_string, 19);
    for (const char *c = sv_begin(suffix); c != sv_end(suffix); c = sv_next(c))
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
    printf("%stest_svcmp...%s", cyan, none);
    if (sv_svcmp(sv(""), sv("")) != EQL)
    {
        return false;
    }
    if (sv_strcmp(sv(""), "") != EQL)
    {
        return false;
    }
    if (sv_svcmp(sv("same"), sv("same")) != EQL)
    {
        return false;
    }
    if (sv_svcmp(sv("samz"), sv("same")) <= EQL)
    {
        return false;
    }
    if (sv_svcmp(sv("same"), sv("samz")) >= EQL)
    {
        return false;
    }
    /* The comparison function should treat the end of a string view as
       null terminating character even if it points to a delimeter */
    if (sv_svcmp(sv("same"), sv_delim("same same", " ")) != EQL)
    {
        return false;
    }
    if (sv_svcmp(sv("same"), sv_delim("samz same", " ")) >= EQL)
    {
        return false;
    }
    if (sv_svcmp(sv_delim("sameez same", " "), sv("same")) <= EQL)
    {
        return false;
    }
    const char *const str = "same";
    if (sv_strcmp(sv(str), str) != 0)
    {
        return false;
    }
    if (sv_strcmp(sv_delim("same same", " "), str) != EQL)
    {
        return false;
    }
    if (sv_strcmp(sv_delim("samez same", " "), str) <= EQL)
    {
        return false;
    }
    if (sv_strcmp(sv_delim("sameez same", " "), str) <= EQL)
    {
        return false;
    }
    /* strncmp compares at most n bytes inclusize or stops at null term */
    if (sv_strncmp(sv_delim("sameez same", " "), str, 10) <= EQL)
    {
        return false;
    }
    if (sv_strncmp(sv_delim("saaeez same", " "), str, 3) >= EQL)
    {
        return false;
    }
    return true;
}

static bool
test_substr(void)
{
    printf("%stest_substr...%s", cyan, none);
    const char ref[27] = {
        [0] = 'A',  [1] = ' ',  [2] = 's',   [3] = 'u',  [4] = 'b',  [5] = 's',
        [6] = 't',  [7] = 'r',  [8] = 'i',   [9] = 'n',  [10] = 'g', [11] = '!',
        [12] = ' ', [13] = 'H', [14] = 'a',  [15] = 'v', [16] = 'e', [17] = ' ',
        [18] = 'a', [19] = 'n', [20] = 'o',  [21] = 't', [22] = 'h', [23] = 'e',
        [24] = 'r', [25] = '!', [26] = '\0',
    };
    const char *const substr1 = "A substring!";
    const char *const substr2 = "Have another!";
    if (sv_strcmp(sv_substr(sv(ref), 0, strlen(substr1)), substr1) != EQL)
    {
        return false;
    }
    if (sv_strcmp(sv_substr(sv(ref), strlen(substr1) + 1, strlen(substr2)),
                  substr2)
        != 0)
    {
        return false;
    }
    if (sv_strcmp(sv_substr(sv(ref), 0, ULLONG_MAX), ref) != EQL)
    {
        return false;
    }
    /* Make sure the fill function adds null terminator */
    char dump_substr1[27] = {[13] = '@'};
    sv_fill(dump_substr1, 27, sv_substr(sv(ref), 0, strlen(substr1)));
    if (strcmp(substr1, dump_substr1) != EQL)
    {
        return false;
    }
    /* Make sure the fill function adds null terminator */
    char dump_substr2[27] = {[14] = '@'};
    sv_fill(dump_substr2, 27,
            sv_substr(sv(ref), strlen(substr1) + 1, strlen(substr2)));
    if (strcmp(substr2, dump_substr2) != EQL)
    {
        return false;
    }
    return true;
}

static bool
test_argv_argc(void)
{
    printf("%stest_argv_argc...%s", cyan, none);
    const char *const expected[4]
        = {"./build/targets/tidy", "--source=file", "-i", "lib/string_view.c"};
    const void *const buf_data
        = "./build/targets/tidy  --source=file   -i   lib/string_view.c\0";
    /* This could be heap or stack allocated. Here we do stack space for
     * convenience testing */
    char argv[10][128];
    str_view view = sv(buf_data);
    size_t i = 0;
    for (str_view v = sv_begin_tok(view, (str_view){" ", 1}); !sv_end_tok(v);
         v = sv_next_tok(v, (str_view){" ", 1}))
    {
        sv_fill(argv[i], sv_svbytes(v), v);
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
test_get_line(void)
{
    printf("%stest_get_line...%s", cyan, none);
    /* We will pretend like we used getline to read this into a heap
       allocated buffer from a file and now we are reading/parsing
       minus the correct EOF handling possibly. */
    const char *lines[5] = {"1", "2", "3", "4", "5"};
    const char *const file_data = "1\n2\n3\n4\n5";
    size_t i = 0;
    for (str_view v = sv_begin_tok((str_view){file_data, sv_strlen(file_data)},
                                   (str_view){"\n", 1});
         !sv_end_tok(v); v = sv_next_tok(v, (str_view){"\n", 1}))
    {
        if (sv_strcmp(v, lines[i]) != EQL)
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

static bool
test_substring_search(void)
{
    printf("%stest_substring_search...%s", cyan, none);
    const char *needle = "needle";
    const size_t needle_len = strlen(needle);
    const char *const haystack
        = "haystackhaystackhaystackhaystackhaystackhaystackhaystackhaystack"
          "haystackhaystackhaystackhaystackhaystackhaystack--------___---**"
          "haystackhaystackhaystackhaystackhaystackhaystack\n\n\n\n\n\n\n\n"
          "neeedleneeddleneedlaneeeeeeeeeeeeeedlenedlennnnnnnnnneeeedneeddl"
          "_______________________needle___________________________________"
          "neeedleneeddleneedlaneeeeeeeeeeeeeedlenedlennneeeeeeeeeeedneeddl"
          "haystackhaystackhaystackhaystackhaystackhaystackhaystack__needle";
    const str_view haystack_view = sv(haystack);
    const str_view needle_view = sv(needle);
    const char *a = strstr(haystack, needle);
    if (!a)
    {
        printf("clibrary strstr failed?\n");
        return false;
    }

    str_view b = sv_n(a, needle_len);
    str_view c = sv_svsv(haystack_view, needle_view);

    if (sv_svcmp(b, c) != EQL || c.s != a)
    {
        return false;
    }
    a += needle_len;
    a = strstr(a, needle);
    if (!a)
    {
        printf("clibrary strstr failed?\n");
        return false;
    }
    const str_view new_haystack_view = sv(a);
    b = sv_n(a, needle_len);
    c = sv_svsv(new_haystack_view, needle_view);
    if (sv_svcmp(b, c) != 0 || c.s != a)
    {
        return false;
    }
    /* There are two needles so we get two string chunks chunks. */
    size_t i = 0;
    for (str_view v
         = sv_begin_tok(haystack_view, (str_view){"needle", needle_len});
         !sv_end_tok(v); v = sv_next_tok(v, (str_view){"needle", needle_len}))
    {
        ++i;
    }
    if (i != 2)
    {
        return false;
    }
    return true;
}
