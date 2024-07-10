#include "str_view.h"
#include "test.h"

#include <stdbool.h>

int
main()
{
    CHECK(sv_empty(sv("")), true, bool, "%d");
    CHECK(*sv_null(), '\0', char, "%c");
    return PASS;
}
