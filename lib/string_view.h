#ifndef STRING_VIEW
#define STRING_VIEW

#include <stdbool.h>
#include <stddef.h>

/* Read-only view of string data. Prefer the provided functions for
   string manipulations rather than using the provided fields. This
   interface is modeled after std::string_view in C++ with elements
   of C mixed in. The string_view type is 16 bytes meaning it is cheap
   to copy and flexible to work with in the provided functions. No
   functions accept string_view by reference except for swap. */
typedef struct string_view
{
    const char *s;
    size_t sz;
} string_view;

/* Constructs and returns a string view from a NULL TERMINATED string. */
string_view sv(const char *);

/* Constructs and returns a string view from a sequence of valid n bytes
   or string length whichever comes first. */
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

/* WARNING This returns a heap allocated, null terminated copy of the
   string_view as a string that must be returned to this library for
   freeing. It is a memory leak to forget to do so. */
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
size_t sv_lenstr(const char *);
size_t sv_lenstrn(const char *, size_t n);

char sv_at(string_view, size_t i);
char sv_front(string_view);
char sv_back(string_view);
void sv_swap(string_view *a, string_view *b);

string_view sv_copy(const char *src_str, size_t str_sz);
void sv_fill(char *dest_buf, size_t dest_sz, string_view src);

const char *sv_begin(string_view);
const char *sv_end(string_view);
const char *sv_next(const char *);
const char *sv_pos(string_view, size_t i);

string_view sv_begin_tok(const char *data, size_t delim_sz, const char *delim);
bool sv_end_tok(string_view);
string_view sv_next_tok(string_view, size_t delim_sz, const char *delim);

string_view sv_substr(string_view, size_t pos, size_t count);

bool sv_starts_with(string_view, string_view prefix);
string_view sv_remove_prefix(string_view, size_t n);

bool sv_ends_with(string_view, string_view suffix);
string_view sv_remove_suffix(string_view, size_t n);

string_view sv_svsv(string_view haystack, string_view needle);
string_view sv_svstr(string_view haystack, const char *needle,
                     size_t needle_sz);
string_view sv_strstr(const char *haystack, size_t haystack_sz,
                      const char *needle, size_t needle_sz);

bool sv_contains(string_view haystack, string_view needle);

size_t sv_find(string_view haystack, string_view needle);
size_t sv_rfind(string_view haystack, string_view needle);

size_t sv_find_first_of(string_view haystack, string_view set);
size_t sv_find_first_not_of(string_view haystack, string_view set);

size_t sv_find_last_of(string_view haystack, string_view set);
size_t sv_find_last_not_of(string_view haystack, string_view set);

void sv_print(string_view);

#endif /* STRING_VIEW */
