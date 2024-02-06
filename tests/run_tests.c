#include "str_view.h"
#include "test.h"

#include <dirent.h>
#include <linux/limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

struct path_bin
{
    str_view path;
    str_view bin;
};

static const str_view test_prefix = {.s = "test_", .sz = 5};
const char *const pass_msg = "⬤";
const char *const fail_msg = "X";
const char *const red = "\033[38;5;9m";
const char *const green = "\033[38;5;10m";
const char *const cyan = "\033[38;5;14m";
const char *const none = "\033[0m";

static int run(str_view);
enum test_result run_test_process(struct path_bin);
static DIR *open_test_dir(str_view);
static bool fill_path(char *, str_view, str_view);

int
main(int argc, char **argv)
{
    if (argc == 1)
    {
        return 0;
    }
    str_view arg_view = sv(argv[1]);
    return run(arg_view);
}

static int
run(const str_view tests_dir)
{
    DIR *dir_ptr = open_test_dir(tests_dir);
    if (!dir_ptr)
    {
        return 1;
    }
    char absolute_path[PATH_MAX];
    size_t tests = 0;
    size_t passed = 0;
    const struct dirent *d;
    while ((d = readdir(dir_ptr)) != NULL)
    {
        const str_view entry = sv(d->d_name);
        if (!sv_starts_with(entry, test_prefix))
        {
            continue;
        }
        if (!fill_path(absolute_path, tests_dir, entry))
        {
            return 1;
        }
        printf("%s(%s%s\n", cyan, sv_begin(entry), none);
        (void)fflush(stdout);
        const enum test_result res
            = run_test_process((struct path_bin){sv(absolute_path), entry});
        switch (res)
        {
        case ERROR:
            (void)fprintf(stderr, "Test process failed abnormally: %s\n",
                          sv_begin(entry));
            break;
        case PASS:
            printf("%s%s%s)%s\n", green, pass_msg, cyan, none);
            break;
        case FAIL:
            printf("%s%s%s)%s\n", red, fail_msg, cyan, none);
            break;
        }
        passed += 1 - res;
        ++tests;
    }
    if (passed == tests)
    {
        printf("%sPASSED %zu/%zu \\(*.*)/%s\n\n", green, passed, tests, none);
    }
    else
    {
        printf("%sPASSED %zu/%zu T_T%s\n\n", red, passed, tests, none);
    }
    return 0;
}

enum test_result
run_test_process(struct path_bin pb)
{
    if (sv_empty(pb.path))
    {
        (void)fprintf(stderr, "No test provided.\n");
        return ERROR;
    }
    const pid_t test_proc = fork();
    if (test_proc == 0)
    {
        execl(sv_begin(pb.path), sv_begin(pb.bin), NULL);
        (void)fprintf(stderr, "Child test proces could not start.\n");
        exit(1);
    }
    int status = 0;
    if (waitpid(test_proc, &status, 0) < 0)
    {
        (void)fprintf(stderr, "Error running test: %s\n", sv_begin(pb.bin));
        return ERROR;
    }
    if (WIFEXITED(status) && WEXITSTATUS(status) == FAIL)
    {
        return FAIL;
    }
    return PASS;
}

static DIR *
open_test_dir(str_view tests_folder)
{
    if (sv_empty(tests_folder) || sv_svlen(tests_folder) > PATH_MAX)
    {
        (void)fprintf(stderr, "Invalid input to path to test executables %s\n",
                      sv_begin(tests_folder));
        return NULL;
    }
    DIR *dir_ptr = opendir(sv_begin(tests_folder));
    if (!dir_ptr)
    {
        (void)fprintf(stderr, "Could not open directory %s\n",
                      sv_begin(tests_folder));
        return NULL;
    }
    return dir_ptr;
}

static bool
fill_path(char *path_buf, str_view tests_dir, str_view entry)
{
    const size_t dir_bytes = sv_fill(path_buf, PATH_MAX, tests_dir);
    if (PATH_MAX - dir_bytes < sv_svbytes(entry))
    {
        (void)fprintf(stderr, "Relative path exceeds PATH_MAX?\n%s", path_buf);
        return false;
    }
    (void)sv_fill(path_buf + sv_svlen(tests_dir), PATH_MAX - dir_bytes, entry);
    return true;
}
