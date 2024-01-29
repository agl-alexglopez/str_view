#ifndef STRING_VIEW
#define STRING_VIEW

#include <stdbool.h>
#include <stddef.h>

typedef struct string_view
{
    const char *s;
    size_t sz;
} string_view;

string_view sv_from_null(const char *);
string_view sv_from_delim(const char *, char delim);
const char *sv_null(void);
size_t sv_npos(string_view);

bool sv_empty(string_view);
size_t sv_len(string_view);
char sv_at(string_view, size_t i);
char sv_front(string_view);
char sv_back(string_view);
const char *sv_data(string_view);

string_view sv_copy(const char *src_str, size_t str_sz);
void sv_fill(char *dest_buf, size_t str_sz, string_view src);

const char *sv_begin(const string_view *);
const char *sv_end(const string_view *);
const char *sv_next(const char *);

string_view sv_begin_tok(string_view, char delim);
bool sv_end_tok(const string_view *, char delim);
string_view sv_next_tok(string_view, char delim);

size_t sv_find_first_of(string_view, char delim);

void sv_print(string_view);

#endif
