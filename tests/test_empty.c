#include "str_view.h"
#include "test.h"

int
main()
{
    if (!sv_empty(sv("")))
    {
        return FAIL;
    }
    if (*sv_null() != '\0')
    {
        return FAIL;
    }
    return PASS;
}
