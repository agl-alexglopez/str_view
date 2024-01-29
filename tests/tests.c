#include "string_view.h"

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
static bool test_front_back(void);
static bool test_copy_fill(void);
static bool test_iter(void);

#define NUM_TESTS (size_t)8
const test_fn all_tests[NUM_TESTS] = {
    test_empty,
    test_out_of_bounds,
    test_from_null,
    test_from_delim,
    test_from_delim_no_delim,
    test_front_back,
    test_copy_fill,
    test_iter,
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
    if (!sv_empty(sv_from_null("")))
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
    const string_view sv = sv_from_null("");
    const pid_t exiting_child = fork();
    if (exiting_child == 0)
    {
        (void)sv_at(sv, 1);
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
    const char *const reference = "Don't miss the terminator!\n";
    const string_view sv = sv_from_null(reference);
    const size_t reference_len = strlen(reference);
    if (reference_len != sv_len(sv))
    {
        return false;
    }
    if (reference[reference_len - 1] != sv_at(sv, sv_len(sv) - 1))
    {
        return false;
    }
    printf("%s", reference);
    sv_print(sv);
    return true;
}

static bool
test_from_delim(void)
{
    printf("test_from_delim\n");
    const char *const reference = "Don'tmissthedelim That was it!";
    const char *const reference_delim = "Don'tmissthedelim";
    const string_view sv = sv_from_delim(reference, ' ');
    const size_t reference_len = strlen(reference_delim);
    printf("%s\n", reference_delim);
    sv_print(sv);
    printf("\n");
    if (reference_len != sv_len(sv))
    {
        return false;
    }
    if (reference_delim[reference_len - 1] != sv_at(sv, sv_len(sv) - 1))
    {
        return false;
    }
    const char *const ref2 = "Don't miss the delim, that was it!";
    const char *const ref2_delim = "Don't miss the delim";
    const string_view sv2 = sv_from_delim(ref2, ',');
    const size_t ref2_len = strlen(ref2_delim);
    printf("%s\n", ref2_delim);
    sv_print(sv2);
    printf("\n");
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
    const string_view sv = sv_from_delim(reference, ' ');
    const size_t reference_len = strlen(reference);
    printf("%s\n", reference);
    sv_print(sv);
    printf("\n");
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
test_front_back(void)
{
    printf("test_front_back");
    if (sv_back(sv_from_null("")) != '\0' || sv_front(sv_from_null("")) != '\0')
    {
        return false;
    }
    const char *const reference = "*The front was * the back is!";
    const string_view sv = sv_from_null(reference);
    const size_t ref_len = strlen(reference);
    if (ref_len != sv_len(sv))
    {
        return false;
    }
    if (sv_front(sv) != '*' || sv_back(sv) != '!')
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
    if (strcmp(sv_data(this), there) != 0)
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
    string_view chars = sv_from_null(reference);
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
    for (string_view cur = sv_begin_tok(chars, ' '); !sv_end_tok(&cur, ' ');
         cur = sv_next_tok(cur, ' '))
    {
        if (sv_front(cur) != reference[i])
        {
            return false;
        }
        i += 2;
    }
    return true;
}
