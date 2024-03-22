# c-str-view

The `str_view` type is a simple, copyable, flexible, read only view of `const char *` data in C. This implementation is experimental for now, lacking any official packaging or robust sample programs. However, this library is well tested and does what is advertised in the interface. The entire implementation can be viewed in `str_view/str_view.h/.c` and included in any project for some convenient string helpers.

A `str_view` is a 16-byte struct and, due to this size, is treated throughout the interface as a copyable type. This is neither a trivially cheap nor excessively expensive type to copy. The intention of this library is to abstract away many sharp edges of working with C-strings to provide usage that "just works," not optimize for performance at this time.

There are still improvements to be made to this library as time allows for packaging, sample programs, and further experimentation.

## Interface

```c
#ifndef STR_VIEW
#define STR_VIEW

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/* Read-only view of string data. Prefer the provided functions for
   string manipulations rather than using the provided fields. This
   interface is modeled after std::string_view in C++ with elements
   of C mixed in. The str_view type is 16 bytes meaning it is cheap
   to copy and flexible to work with in the provided functions. No
   functions accept str_view by reference, except for swap. */
typedef struct
{
    const char *s;
    size_t sz;
} str_view;

/* Standard three way comparison type in C. See the comparison
   functions for how to interpret the sv comparison results. */
typedef enum
{
    LES = -1,
    EQL = 0,
    GRT = 1,
} sv_threeway_cmp;

/* A macro provided to make str_view constants less error prone at
   compile time. For example, if it is desirable to construct a str_view
   constant the following will obtain the length field for the user.
   Simply copy and paste the same string twice for best results.

      static const str_view prefix = {.s = "test_", .sz = SVLEN("test_")};
      int main()
      {
         ...
      }

   At runtime, prefer the provided functions for all other str_view needs. */
#define SVLEN(str) ((sizeof((str)) / sizeof((str)[0])) - sizeof((str)[0]))

/* A macro to further reduce the chance for errors in repeating oneself
   when constructing inline or const str_views. The input must be a string
   literal. For example, the above example becomes:

      static const str_view prefix = SV("test_");

   But more importantly this allows for inline constructors that are
   easier to read than struct declarations and don't risk mistakes in
   counting characters. For example:

       for (str_view cur = sv_begin_tok(ref, SV(" "));
            !sv_end_tok(ref_view, cur);
            cur = sv_next_tok(ref_view, cur, SV(" "))
       {}

   However saving the delimiter in a constant may be more convenient. */
#define SV(str) ((str_view){(str), SVLEN((str))})

/* Constructs and returns a string view from a NULL TERMINATED string. */
str_view sv(const char *);

/* Constructs and returns a string view from a sequence of valid n bytes
   or string length, whichever comes first. */
str_view sv_n(const char *, size_t n);

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
str_view sv_delim(const char *, const char *delim);

/* A sentinel empty string. Safely dereferenced to view a null terminator. */
const char *sv_null(void);

/* The end of a str_view guaranted to be greater than or equal to size */
size_t sv_npos(str_view);

/* Returns the standard C threeway comparison between cmp(lhs, rhs)
   between two string views.
   lhs LES(-1) rhs (lhs is less than rhs)
   lhs EQL(0) rhs (lhs is equal to rhs)
   lhs GRT(1) rhs (lhs is greater than rhs)*/
sv_threeway_cmp sv_svcmp(str_view, str_view);

/* Returns the standard C threeway comparison between cmp(lhs, rhs)
   between a str_view and a c-string.
   str_view LES(-1) rhs (str_view is less than str)
   str_view EQL(0) rhs (str_view is equal to str)
   str_view GRT(1) rhs (str_view is greater than str)*/
sv_threeway_cmp sv_strcmp(str_view, const char *str);

/* Returns the standard C threeway comparison between cmp(lhs, rhs)
   between a str_view and the first n bytes (inclusive) of str
   or stops at the null terminator if that is encountered first.
   str_view LES(-1) rhs (str_view is less than str)
   str_view EQL(0) rhs (str_view is equal to str)
   str_view GRT(1) rhs (str_view is greater than str)*/
sv_threeway_cmp sv_strncmp(str_view, const char *str, size_t n);

/* Returns true if the provided str_view is empty, false otherwise. */
bool sv_empty(str_view);

/* Returns the size of the string view O(1). */
size_t sv_len(str_view);

/* Returns the bytes of str_view including null terminator. Note that
   string views may not actually be null terminated but the position at
   str_view[str_view.sz] is interpreted as the null terminator. */
size_t sv_svbytes(str_view);

/* Returns the size of the null terminated string O(n) */
size_t sv_strlen(const char *);

/* Returns the bytes of the string pointer to, null terminator included. */
size_t sv_strbytes(const char *);

/* Returns the minimum between the string size vs n bytes. */
size_t sv_minlen(const char *, size_t n);

/* The characer in the string at position i with bounds checking.
   The program will exit if an out of bounds error occurs. */
char sv_at(str_view, size_t i);

/* The character at the first position of str_view. An empty
   str_view or NULL pointer is valid and will return '\0'. */
char sv_front(str_view);

/* The character at the last position of str_view. An empty
   str_view or NULL pointer is valid and will return '\0'. */
char sv_back(str_view);

/* Swaps the contents of a and b. Becuase these are read only views
   only pointers and sizes are exchanged. */
void sv_swap(str_view *a, str_view *b);

/* Copies the max of str_sz or src_str length into a view, whichever
   ends first. This is the same as sv_n. */
str_view sv_copy(const char *src_str, size_t str_sz);

/* Fills the destination buffer with the minimum between
   destination size and source view size, null terminating
   the string. This may cut off src data if dest_sz < src.sz.
   Returns how many bytes were written to the buffer. */
size_t sv_fill(char *dest_buf, size_t dest_sz, str_view src);

/* Returns a read only pointer to the beginning of the string view,
   the first valid character in the view. If the view stores NULL,
   the placeholder sv_null() is returned. */
const char *sv_begin(str_view);

/* Returns a read only pointer to the end of the string view. This
   may or may not be a null terminated character depending on the
   view. If the view stores NULL, the placeholder sv_null() is returned. */
const char *sv_end(str_view);

/* Advances the pointer from its previous position. If NULL is provided
   sv_null() is returned. */
const char *sv_next(const char *);

/* Returns the reverse iterator beginning, the last character of the
   current view. If the view is null sv_null() is returned. If the
   view is sized zero with a valid pointer that pointer in the
   view is returned. */
const char *sv_rbegin(str_view);

/* The ending position of a reverse iteration. It is undefined
   behavior to access or use rend. It is undefined behavior to
   pass in any str_view not being iterated through as started
   with rbegin. */
const char *sv_rend(str_view);

/* Advances the iterator to the next character in the str_view
   being iterated through in reverse. It is undefined behavior
   to change the str_view one is iterating through during
   iteration. If the char pointer is null, sv_null() is returned. */
const char *sv_rnext(const char *);

/* Returns the character pointer at the minimum between the indicated
   position and the end of the string view. If NULL is stored by the
   str_view then sv_null() is returned. */
const char *sv_pos(str_view, size_t i);

/* Finds the first tokenized position in the string view given the any
   sized delimeter str_view. This means that if the string begins
   with a delimeter, that delimeter is skipped until a string token
   is found. For example:

     const char *const str = "  Hello world!";
     sv_print(sv_delim(str, " "));
     <<< "Hello"

   This is similar to the sv_delim constructor. If the str_view
   to be searched stores NULL than the sv_null() is returned. If
   delim stores NULL, that is interpreted as a search for the null
   terminating character or empty string and the size zero substring
   at the final position in the str_view is returned wich may or
   may not be the null termiator. */
str_view sv_begin_tok(str_view src, str_view delim);

/* Returns true if no further tokes are found and position is at the end
   position, meaning a call to sv_next_tok has yielded a size 0 str_view */
bool sv_end_tok(str_view src, str_view tok);

/* Advances to the next token in the remaining view seperated by the delim.
   Repeating delimter patterns will be skipped until the next token or end
   of string is found. If str_view stores NULL the sv_null() placeholder
   is returned. If delim stores NULL the end position of the str_view
   is returned which may or may not be the null terminator. */
str_view sv_next_tok(str_view src, str_view tok, str_view delim);

/* Obtains the last token in a string in preparation for reverse tokenized
   iteration. Any delimeters that start the string are skipped, as in the
   forward version. If src is null sv_null is returned. If delim is null
   the entire src view is returned. */
str_view sv_rbegin_tok(str_view src, str_view delim);

/* Given the current str_view being iterated through and the current token
   in the iteration returns true if the ending state of a reverse tokenization
   has been reached, false otherwise. */
bool sv_rend_tok(str_view src, str_view tok);

/* Advances the token in src to the next token between two delimeters provided
   by delim. Repeating delimiters are skipped until the next token is found.
   If no further tokens can be found an empty str_view is returned with its
   pointer set to the start of the src string being iterated through. Note
   that a multicharacter delimiter may yeild different tokens in reverse
   than in the forward direction when partial matches occur and some portion
   of the delimeter is in a token. This is because the string is now being
   read from right to left. */
str_view sv_rnext_tok(str_view src, str_view tok, str_view delim);

/* Returns a str_view of the entirety of the underlying string, starting
   at the current views pointer position. This guarantees that the str_view
   returned ends at the null terminator of the underlying string as all
   strings used with str_views are assumed to be null terminated. It is
   undefined behavior to provide non null terminated strings to any
   str_view code. */
str_view sv_extend(str_view src);

/* Creates the substring from position pos for count length. The count is
   the minimum value between count and (str_view.sz - pos). The process
   will exit if position is greater than str_view size. */
str_view sv_substr(str_view, size_t pos, size_t count);

/* Searches for needle in hay starting from pos. If the needle
   is larger than the hay, or position is greater than hay length,
   then hay length is returned. */
size_t sv_find(str_view hay, size_t pos, str_view needle);

/* Searches for the last occurence of needle in hay starting from pos
   from right to left. If found the starting position of the string
   is returned, the same as find. The only difference is the search
   direction. If needle is larger than hay, hay length is returned.
   If the position is larger than the hay, the entire hay is searched. */
size_t sv_rfind(str_view hay, size_t pos, str_view needle);

/* Returns true if the needle is found in the hay, false otherwise. */
bool sv_contains(str_view hay, str_view needle);

/* Returns a view of the needle found in hay at the first found
   position. If the needle cannot be found the empty view at the
   hay length position is returned. This may or may not be null
   terminated at that position. If needle is greater than
   hay length sv_null is returned. */
str_view sv_svsv(str_view hay, str_view needle);

/* Returns a view of the needle found in hay at the last found
   position. If the needle cannot be found the empty view at the
   hay length position is returned. This may or may not be null
   terminated at that position. If needle is greater than
   hay length sv_null is returned. */
str_view sv_rsvsv(str_view hay, str_view needle);

/* Returns true if a prefix <= str_view is present, false otherwise. */
bool sv_starts_with(str_view, str_view prefix);

/* Removes the minimum between str_view length and n from the start. */
str_view sv_remove_prefix(str_view, size_t n);

/* Returns true if a suffix of length <= str_view is present,
   false otherwise. */
bool sv_ends_with(str_view, str_view suffix);

/* Removes the minimum between str_view length and n from the end. */
str_view sv_remove_suffix(str_view, size_t n);

/* Finds the first position of an occurence of any character in set.
   If no occurence is found hay size is returned. An empty set (NULL)
   is valid and will return position at hay size. An empty hay
   returns 0. */
size_t sv_find_first_of(str_view hay, str_view set);

/* Finds the first position at which no characters in set can be found.
   If the string is all characters in set hay length is returned.
   An empty set (NULL) is valid and will return position 0. An empty
   hay returns 0. */
size_t sv_find_first_not_of(str_view hay, str_view set);

/* Finds the last position of any character in set in hay. If
   no position is found hay size is returned. An empty set (NULL)
   is valid and returns hay size. An empty hay returns
   0. */
size_t sv_find_last_of(str_view hay, str_view set);

/* Finds the last position at which no character in set can be found.
   An empty set (NULL) is valid and will return position 0. An empty
   hay will return 0. */
size_t sv_find_last_not_of(str_view hay, str_view set);

/* Writes all characters in str_view to stdout. */
void sv_print(FILE *, str_view);

#endif /* STR_VIEW */
```

Thanks for reading!
