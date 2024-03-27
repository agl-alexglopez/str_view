#include "str_view.h"
#include "test.h"

#include <stdbool.h>

int
main()
{
    CHECK(sv_empty(sv("")), true, "%b");
    CHECK(*sv_null(), '\0', "%c");
    return PASS;
}
