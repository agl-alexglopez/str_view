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

#define CHECK(RESULT, EXPECTED)                                                \
    do                                                                         \
    {                                                                          \
        if ((RESULT) != (EXPECTED))                                            \
        {                                                                      \
            return FAIL;                                                       \
        }                                                                      \
    } while (0)

#endif
