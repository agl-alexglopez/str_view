#include "str_view.h"
#include "test.h"

#include <stddef.h>

int
main()
{
    str_view const s = SV("a");
    str_view const empty = sv(sv_pos(s, 1));
    CHECK(sv_at(s, 9), '\0', char, "%c");
    CHECK(sv_pos(s, 9), sv_end(s), char *const, "%s");
    str_view const sub_s = sv_substr(s, 9, 9);
    CHECK(sv_begin(sub_s), sv_begin(empty), char *const, "%s");
    CHECK(sv_len(sub_s), sv_len(empty), size_t, "%zu");
    return PASS;
}
