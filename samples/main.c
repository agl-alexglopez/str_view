#include "str_view.h"
#include <stdio.h>

int
main()
{
    sv_print(stdout, sv("Hello from the sandbox!\n"));
    return 0;
}
