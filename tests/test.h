#ifndef TEST
#define TEST

enum test_result
{
    ERROR = -1,
    PASS = 0,
    FAIL = 1,
};

typedef enum test_result (*test_fn)(void);

struct fn_name
{
    test_fn fn;
    const char *const name;
};

/* Set this breakpoint on any line where you wish
   execution to stop. Under normal program runs the program
   will simply exit. If triggered in GDB execution will stop
   while able to explore the surrounding context, varialbes,
   and stack frames. Be sure to step "(gdb) up" out of the
   raise function to wherever it triggered. */
#define BREAKPOINT()                                                           \
    do                                                                         \
    {                                                                          \
        (void)fprintf(stderr, "\n!!Break. Line: %d File: %s, Func: %s\n ",     \
                      __LINE__, __FILE__, __func__);                           \
        (void)raise(SIGTRAP);                                                  \
    } while (0)

/* The CHECK macro evaluates a result and compares it to an
   expectation of what should happen. This is a standard
   idiom like an assert or any check a framework like google
   test provides. However, this is simply meant to be called
   in the test files where PASS or FAIL is expected to return.
   If the check fails FAIL is returned. If the check passes,
   nothing happens. This makes it easy to see where execution
   haltes in the function if stepped through in gdb. RESULT
   and EXPECTED must be comparable with ==/!=. */
#define CHECK(RESULT, EXPECTED)                                                \
    do                                                                         \
    {                                                                          \
        if ((RESULT) != (EXPECTED))                                            \
        {                                                                      \
            (void)fprintf(stderr, "this check failed on line %d:\n%s == %s\n", \
                          __LINE__, #RESULT, #EXPECTED);                       \
            return FAIL;                                                       \
        }                                                                      \
    } while (0)

#endif
