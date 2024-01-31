#ifndef STRING_VIEW
#define STRING_VIEW

#include <stdbool.h>
#include <stddef.h>

typedef struct string_view
{
    const char *s;
    size_t sz;
} string_view;

/* Constructs and returns a string view from a NULL TERMINATED string. */
string_view sv(const char *);

/* Constructs and returns a string view from a sequence of valid n bytes. */
string_view sv_n(const char *, size_t n);

/* Constructs and returns a string view from a NULL TERMINATED string
   broken on the first ocurrence of delimeter if found or null
   terminator if delim cannot be found. This constructor will also
   skip the delimeter if that delimeter starts the string. For example:

     const char *const str = "  Hello world!";
     sv_print(sv_delim(str, " "));
     <<< "Hello"

   Or the string may be empty if it is made of delims.

     const char *const str = "------";
     sv_print(sv_delim(str, "-"));
     <<< ""

   This is similar to the tokenizing function in the iterator section. */
string_view sv_delim(const char *, const char *delim);

/* WARNING This returns a heap allocated string that must be returned to
   this library for freeing. It is a memory leak to forget to do so. */
char *sv_alloc(string_view);

/* WARNING This frees the heap allocated string that was previously created
   by sv_alloc. The user must return the allocated pointer to the char *
   before program exit or a memory leak has occured. */
void sv_free(char *);

const char *sv_null(void);
size_t sv_npos(string_view);

int sv_svcmp(string_view, string_view);
int sv_strcmp(string_view, const char *str);
int sv_strncmp(string_view, const char *str, size_t n);

bool sv_empty(string_view);
size_t sv_len(string_view);
char sv_at(string_view, size_t i);
char sv_front(string_view);
char sv_back(string_view);
const char *sv_str(string_view);

string_view sv_copy(const char *src_str, size_t str_sz);
void sv_fill(char *dest_buf, size_t dest_sz, string_view src);

const char *sv_begin(const string_view *);
const char *sv_end(const string_view *);
const char *sv_next(const char *);

string_view sv_begin_tok(const char *data, size_t delim_sz, const char *delim);
bool sv_end_tok(const string_view *);
string_view sv_next_tok(string_view, size_t delim_sz, const char *delim);

string_view sv_remove_prefix(string_view, size_t n);
string_view sv_remove_suffix(string_view, size_t n);

string_view sv_substr(string_view, size_t pos, size_t count);

bool sv_contains(string_view haystack, string_view needle);
size_t sv_find_first_of(string_view, const char *delim);
size_t sv_find_last_of(string_view, const char *delim);
size_t sv_find_first_not_of(string_view, const char *delim, size_t delim_sz);
size_t sv_find_last_not_of(string_view, const char *delim);

void sv_print(string_view);

#endif
