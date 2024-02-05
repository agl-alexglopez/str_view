#include "str_view.h"
#include "test.h"

int
main()
{
    enum test_result res = PASS;
    if (!sv_empty(sv("")))
    {
        res = FAIL;
    }
    if (*sv_null() != '\0')
    {
        res = FAIL;
    }
    return res;
}
