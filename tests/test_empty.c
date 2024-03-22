#include "str_view.h"
#include "test.h"

#include <stdbool.h>

int
main()
{
    enum test_result res = PASS;
    CHECK(sv_empty(sv("")), true);
    CHECK(*sv_null(), '\0');
    return res;
}
