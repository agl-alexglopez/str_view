#ifndef STRING_VIEW
#define STRING_VIEW

#include <stdbool.h>
#include <stddef.h>

/* Read-only view of string data. Prefer the provided functions for
   string manipulations rather than using the provided fields. This
   interface is modeled after std::string_view in C++ with elements
   of C mixed in. The string_view type is 16 bytes meaning it is cheap
   to copy and flexible to work with in the provided functions. No
   functions accept string_view by reference, except for swap. */
typedef struct string_view
{
    const char *s;
    size_t sz;
} string_view;

typedef enum sv_threeway_cmp
{
    LES = -1,
    EQL = 0,
    GRT = 1,
} sv_threeway_cmp;

/* Constructs and returns a string view from a NULL TERMINATED string. */
string_view sv(const char *);

/* Constructs and returns a string view from a sequence of valid n bytes
   or string length, whichever comes first. */
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

/* A sentinel empty string. Safely dereferenced to view a null terminator. */
const char *sv_null(void);

/* The end of a string_view guaranted to be greater than or equal to size */
size_t sv_npos(string_view);

/* Returns the standard C threeway comparison between cmp(lhs, rhs)
   between two string views.
   lhs LES(-1) rhs, lhs EQL(0) rhs, lhs (GRT) rhs*/
sv_threeway_cmp sv_svcmp(string_view, string_view);
/* Returns the standard C threeway comparison between cmp(lhs, rhs)
   between a string_view and a c-string.
   lhs LES(-1) rhs, lhs EQL(0) rhs, lhs (GRT) rhs*/
sv_threeway_cmp sv_strcmp(string_view, const char *str);

/* Returns the standard C threeway comparison between cmp(lhs, rhs)
   between a string_view and the first n bytes (inclusive) of str
   or stops at the null terminator if that is encountered first.
   lhs LES(-1) rhs, lhs EQL(0) rhs, lhs (GRT) rhs*/
sv_threeway_cmp sv_strncmp(string_view, const char *str, size_t n);

/* Returns true if the provided string_view is empty, false otherwise. */
bool sv_empty(string_view);

/* Returns the size of the string view O(1). */
size_t sv_svlen(string_view);

/* Returns the bytes of string_view including null terminator. Note that
   string views may not actually be null terminated but the position at
   string_view[string_view.sz] is interpreted as the null terminator. */
size_t sv_svbytes(string_view);

/* Returns the size of the null terminated string O(n) */
size_t sv_strlen(const char *);

/* Returns the bytes of the string pointer to, null terminator included. */
size_t sv_strbytes(const char *);

/* Returns the maximum between the string size vs n bytes. */
size_t sv_maxlen(const char *, size_t n);

/* The characer in the string at position i with bounds checking.
   The program will exit if an out of bounds error occurs. */
char sv_at(string_view, size_t i);

/* The character at the first position of string_view. An empty
   string_view or NULL pointer is valid and will return '\0'. */
char sv_front(string_view);

/* The character at the last position of string_view. An empty
   string_view or NULL pointer is valid and will return '\0'. */
char sv_back(string_view);

/* Swaps the contents of a and b. Becuase these are read only views
   only pointers and sizes are exchanged. */
void sv_swap(string_view *a, string_view *b);

/* Copies the max of str_sz or src_str length into a view, whichever
   ends first. This is the same as sv_n. */
string_view sv_copy(const char *src_str, size_t str_sz);

/* Fills the destination buffer with the minimum between
   destination size and source view size, null terminating
   the string. This may cut off src data if dest_sz < src.sz */
void sv_fill(char *dest_buf, size_t dest_sz, string_view src);

/* Returns a read only pointer to the beginning of the string view,
   the first valid character in the view. If the view stores NULL,
   the placeholder sv_null() is returned. */
const char *sv_begin(string_view);

/* Returns a read only pointer to the end of the string view. This
   may or may not be a null terminated character depending on the
   view. If the view stores NULL, the placeholder sv_null() is returned. */
const char *sv_end(string_view);

/* Advances the pointer from its previous position. If NULL is provided
   sv_null() is returned. */
const char *sv_next(const char *);

/* Returns the character pointer at the minimum between the indicated
   position and the end of the string view. If NULL is stored by the
   string_view then sv_null() is returned. */
const char *sv_pos(string_view, size_t i);

/* Finds the first tokenized position in the string view given the any
   sized delimeter string_view. This means that if the string begins
   with a delimeter, that delimeter is skipped until a string token
   is found. For example:

     const char *const str = "  Hello world!";
     sv_print(sv_delim(str, " "));
     <<< "Hello"

   This is similar to the sv_delim constructor. If the string_view
   to be searched stores NULL than the sv_null() is returned. If
   delim stores NULL, that is interpreted as a search for the null
   terminating character or empty string and the size zero substring
   at the final position in the string_view is returned wich may or
   may not be the null termiator. */
string_view sv_begin_tok(string_view, string_view delim);

/* Returns true if no further tokes are found and position is at the end
   position, meaning a call to sv_next_tok has yielded a size 0 string_view */
bool sv_end_tok(string_view);

/* Advances to the next token in the remaining view seperated by the delim.
   Repeating delimter patterns will be skipped until the next token or end
   of string is found. If string_view stores NULL the sv_null() placeholder
   is returned. If delim stores NULL the end position of the string_view
   is returned which may or may not be the null terminator. */
string_view sv_next_tok(string_view, string_view delim);

/* Creates the substring from position pos for count length. The count is
   the minimum value between count and (string_view.sz - pos). The process
   will exit if position is greater than string_view size. */
string_view sv_substr(string_view, size_t pos, size_t count);

/* Searching for needle in haystack starting at position. If the needle
   is larger than the haystack of position is greater than haystack length
   then haystack length is returned. */
size_t sv_find(string_view haystack, size_t pos, string_view needle);

/* Searching for the last occurence of needle in haystack starting from pos.
   If needle is larger than haystack, haystack length is returned. If the
   position is larger than the haystack, the entire haystack is searched. */
size_t sv_rfind(string_view haystack, size_t pos, string_view needle);

/* Returns true if the needle is found in the haystack, false otherwise. */
bool sv_contains(string_view haystack, string_view needle);

/* Returns a view of the needle found in haystack at the first found
   position. If the needle cannot be found the empty view at the
   haystack length position is returned. This may or may not be null
   terminated at that position. */
string_view sv_svsv(string_view haystack, string_view needle);

/* Returns true if a prefix <= string_view is present, false otherwise. */
bool sv_starts_with(string_view, string_view prefix);

/* Removes the minimum between string_view length and n from the start. */
string_view sv_remove_prefix(string_view, size_t n);

/* Returns true if a suffix of length <= string_view is present,
   false otherwise. */
bool sv_ends_with(string_view, string_view suffix);

/* Removes the minimum between string_view length and n from the end. */
string_view sv_remove_suffix(string_view, size_t n);

/* Finds the first position of an occurence of any character in set.
   If no occurence is found haystack size is returned. An empty set (NULL)
   is valid and will return position at haystack size. An empty haystack
   returns 0. */
size_t sv_find_first_of(string_view haystack, string_view set);

/* Finds the first position at which no characters in set can be found.
   If the string is all characters in set haystack length is returned.
   An empty set (NULL) is valid and will return position 0. An empty
   haystack returns 0. */
size_t sv_find_first_not_of(string_view haystack, string_view set);

/* Finds the last position of any character in set in haystack. If
   no position is found haystack size is returned. An empty set (NULL)
   is valid and returns haystack size. An empty haystack returns
   0. */
size_t sv_find_last_of(string_view haystack, string_view set);

/* Finds the last position at which no character in set can be found.
   An empty set (NULL) is valid and will return position 0. An empty
   haystack will return 0. */
size_t sv_find_last_not_of(string_view haystack, string_view set);

/* Writes all characters in string_view to stdout. */
void sv_print(string_view);

#endif /* STRING_VIEW */
