#include "str_view.h"
#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int
main()
{
    printf("\n");
    const str_view s = sv("");
    const pid_t exiting_child = fork();
    if (exiting_child == 0)
    {
        (void)sv_at(s, 1);
        /* We should not make it here */
        exit(ERROR);
    }
    int status = 0;
    if (waitpid(exiting_child, &status, 0) < 0)
    {
        printf("Error waiting for failing child.\n");
        exit(1);
    }
    /* We expect to have exited with a status of 1 */
    const bool correct_failure = WIFEXITED(status) && WEXITSTATUS(status) == 1;
    return correct_failure ? PASS : FAIL;
}
