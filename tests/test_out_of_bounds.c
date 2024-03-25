#include "str_view.h"
#include "test.h"

int
main()
{
    const str_view s = sv("a");
    const str_view empty = (str_view){s.s + 1, 0};
    CHECK(sv_at(s, 9), '\0');
    CHECK(sv_pos(s, 9), s.s + 1);
    const str_view sub_s = sv_substr(s, 9, 9);
    CHECK(sub_s.s, empty.s);
    CHECK(sub_s.sz, empty.sz);
    return PASS;
}
