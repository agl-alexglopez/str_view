/* Author: Alexander G. Lopez */
#ifndef SV_STR_VIEW
#define SV_STR_VIEW

/* SV_ATTRIB_PURE has no side effects when given the same arguments with the
   same data, producing the same return value. A SV_Str_view points to char
   const * data which may change in between SV_Str_view calls. SV_ATTRIB_CONST
   applies only where no pointers are accessed or dereferenced. Other
   attributes provide more string safety and opportunities to optimize.
   Credit Harith on Code Review Stack Exchange. */
#if defined(__GNUC__) || defined(__clang__) || defined(__INTEL_LLVM_COMPILER)
#    ifdef __has_attribute
#        if __has_attribute(pure)
#            define SV_ATTRIB_PURE __attribute__((pure))
#        else
#            define SV_ATTRIB_PURE /**/
#        endif
#        if __has_attribute(pure)
#            define SV_ATTRIB_CONST __attribute__((const))
#        else
#            define SV_ATTRIB_CONST /**/
#        endif
#        if __has_attribute(null_terminated_string_arg)
#            define SV_ATTRIB_NULLTERM(...)                                    \
                __attribute__((null_terminated_string_arg(__VA_ARGS__)))
#        else
#            define SV_ATTRIB_NULLTERM(...) /**/
#        endif
#    else
#        define SV_ATTRIB_PURE          /**/
#        define SV_ATTRIB_CONST         /**/
#        define SV_ATTRIB_NULLTERM(...) /**/
#    endif
/* A helper macro to enforce only string literals for the SV_from constructor
   macro. GCC and Clang allow this syntax to create more errors when bad
   input is provided to the SV_from constructor.*/
#    define SV_STR_LITERAL(str) "" str ""
#else
#    define SV_ATTRIB_PURE          /**/
#    define SV_ATTRIB_CONST         /**/
#    define SV_ATTRIB_NULLTERM(...) /**/
/* MSVC does not allow strong enforcement of string literals to the SV_from
   constructor. This is a dummy wrapper for compatibility. */
#    define SV_STR_LITERAL(str) str
#endif /* __GNUC__ || __clang__ || __INTEL_LLVM_COMPILER */

#if defined(_MSVC_VER) || defined(_WIN32) || defined(_WIN64)
#    if defined(SV_BUILD_DLL)
#        define SV_API __declspec(dllexport)
#    elif defined(SV_CONSUME_DLL)
#        define SV_API __declspec(dllimport)
#    else
#        define SV_API /**/
#    endif
#else
#    define SV_API /**/
#endif             /* _MSVC_VER */

#include <stdbool.h>
#include <stddef.h>

/* A SV_Str_view is a read-only view of string data in C. It is modeled after
   the C++ std::string_view. It consists of a pointer to char const data
   and a size_t field. Therefore, the exact size of this type may be platform
   dependent but it is small enough that one should use the provided functions
   and pass by copy whenever possible. Avoid accessing struct fields. */
typedef struct
{
    /* The read only data to which we point. */
    char const *s;
    /* The length, not including the NULL terminator position. However, it is
       possible that a view is not NULL terminated at that position. */
    size_t len;
} SV_Str_view;

/* Standard three way comparison type in C. See the comparison
   functions for how to interpret the comparison results. ERR
   is returned if bad input is provided to any comparison. */
typedef enum
{
    SV_ORDER_LESSER = -1,
    SV_ORDER_EQUAL,
    SV_ORDER_GREATER,
    SV_ORDER_ERROR,
} SV_Order;

/*==========================  Construction  ================================*/

/* A macro to reduce the chance for errors in repeating oneself when
   constructing an inline or const SV_Str_view. The input must be a string
   literal. For example:

      static SV_Str_view const prefix = SV_from("test_");

   One can even use this in code when string literals are used rather than
   saved constants to avoid errors in SV_Str_view constructions.

       for (SV_Str_view cur = SV_begin_token(src, SV_from(" "));
            !SV_end_token(src, cur);
            cur = SV_next_token(src, cur, SV_from(" "))
       {}

   However saving the SV_Str_view in a constant may be more convenient. */
#define SV_from(str_literal)                                                   \
    ((SV_Str_view){SV_STR_LITERAL(str_literal), sizeof(str_literal) - 1})

/* Constructs and returns a string view from a NULL TERMINATED string.
   It is undefined to construct a SV_Str_view from a non terminated string. */
SV_API SV_Str_view SV_from_terminated(char const *str)
    SV_ATTRIB_NULLTERM(1) SV_ATTRIB_PURE;

/* Constructs and returns a string view from a sequence of valid n bytes
   or string length, whichever comes first. The resulting SV_Str_view may
   or may not be null terminated at the index of its size. */
SV_API SV_Str_view SV_from_view(size_t n, char const *str)
    SV_ATTRIB_NULLTERM(2) SV_ATTRIB_PURE;

/* Constructs and returns a string view from a NULL TERMINATED string
   broken on the first ocurrence of delimiter if found or null
   terminator if delim cannot be found. This constructor will also
   skip the delimiter if that delimiter starts the string. This is similar
   to the tokenizing function in the iteration section. */
SV_API SV_Str_view SV_from_delimiter(char const *str, char const *delim)
    SV_ATTRIB_NULLTERM(1) SV_ATTRIB_NULLTERM(2) SV_ATTRIB_PURE;

/* Returns the bytes of the string pointer to, null terminator included. */
SV_API size_t SV_str_bytes(char const *str)
    SV_ATTRIB_NULLTERM(1) SV_ATTRIB_PURE;

/* Copies the max of str_sz or src_str length into a view, whichever
   ends first. This is the same as SV_n. */
SV_API SV_Str_view SV_copy(size_t str_sz, char const *src_str)
    SV_ATTRIB_NULLTERM(2) SV_ATTRIB_PURE;

/* Fills the destination buffer with the minimum between
   destination size and source view size, null terminating
   the string. This may cut off src data if dest_sz < src.len.
   Returns how many bytes were written to the buffer. */
SV_API size_t SV_fill(size_t dest_sz, char *dest_buf, SV_Str_view src);

/* Returns the standard C threeway comparison between cmp(lhs, rhs)
   between a SV_Str_view and a c-string.
   SV_Str_view LESSER( -1  ) rhs (SV_Str_view is less than str)
   SV_Str_view EQUAL(  0  ) rhs (SV_Str_view is equal to str)
   SV_Str_view GREATER(  1  ) rhs (SV_Str_view is greater than str)
   Comparison is bounded by the shorter SV_Str_view length. ERR is
   returned if bad input is provided such as a SV_Str_view with a
   NULL pointer field. */
SV_API SV_Order SV_terminated_compare(SV_Str_view lhs, char const *rhs)
    SV_ATTRIB_NULLTERM(2) SV_ATTRIB_PURE;

/* Returns the standard C threeway comparison between cmp(lhs, rhs)
   between a SV_Str_view and the first n bytes (inclusive) of str
   or stops at the null terminator if that is encountered first.
   SV_Str_view LESSER( -1  ) rhs (SV_Str_view is less than str)
   SV_Str_view EQUAL(  0  ) rhs (SV_Str_view is equal to str)
   SV_Str_view GREATER(  1  ) rhs (SV_Str_view is greater than str)
   Comparison is bounded by the shorter SV_Str_view length. ERR is
   returned if bad input is provided such as a SV_Str_view with a
   NULL pointer field. */
SV_API SV_Order SV_view_compare(SV_Str_view lhs, char const *rhs, size_t n)
    SV_ATTRIB_NULLTERM(2) SV_ATTRIB_PURE;

/* Returns the minimum between the string size vs n bytes. */
SV_API size_t SV_min_len(char const *str, size_t n)
    SV_ATTRIB_NULLTERM(1) SV_ATTRIB_PURE;

/* Advances the pointer from its previous position. If NULL is provided
   SV_null() is returned. */
SV_API char const *SV_next(char const *c) SV_ATTRIB_NULLTERM(1) SV_ATTRIB_PURE;

/* Advances the iterator to the next character in the SV_Str_view
   being iterated through in reverse. It is undefined behavior
   to change the SV_Str_view one is iterating through during
   iteration. If the char pointer is null, SV_null() is returned. */
SV_API char const *SV_reverse_next(char const *c) SV_ATTRIB_PURE;

/* Creates the substring from position pos for count length. The count is
   the minimum value between count and (length - pos). If an invalid
   position is given greater than SV_Str_view length an empty view is returned
   positioned at the end of SV_Str_view. This position may or may not hold the
   null terminator. */
SV_API SV_Str_view SV_substr(SV_Str_view sv, size_t pos,
                             size_t count) SV_ATTRIB_PURE;

/* A sentinel empty string. Safely dereferenced to view a null terminator.
   This may be returned from various functions when bad input is given
   such as NULL as the underlying SV_Str_view string pointer. */
SV_API char const *SV_null(void) SV_ATTRIB_PURE;

/* The end of a SV_Str_view guaranteed to be greater than or equal to size.
   May be used for the idiomatic check for most string searching function
   return values when something is not found. If a size is returned from
   a searching function it is possible to check it against npos. */
SV_API size_t SV_npos(SV_Str_view sv) SV_ATTRIB_CONST;

/* Returns true if the provided SV_Str_view is empty, false otherwise.
   This is a useful function to check for SV_Str_view searches that yield
   an empty view at the end of a SV_Str_view when an element cannot be
   found. */
SV_API bool SV_is_empty(SV_Str_view sv) SV_ATTRIB_CONST;

/* Returns the length of the SV_Str_view in O(1) time. The position at
   SV_Str_view size is interpreted as the null terminator and not
   counted toward length of a SV_Str_view. */
SV_API size_t SV_len(SV_Str_view sv) SV_ATTRIB_CONST;

/* Returns the bytes of SV_Str_view including null terminator. Note that
   string views may not actually be null terminated but the position at
   SV_Str_view[SV_Str_view.len] is interpreted as the null terminator and thus
   counts towards the byte count. */
SV_API size_t SV_bytes(SV_Str_view sv) SV_ATTRIB_CONST;

/* Swaps the contents of a and b. Because these are read only views only
   pointers and sizes are exchanged. */
SV_API void SV_swap(SV_Str_view *a, SV_Str_view *b);

/* Returns a SV_Str_view of the entirety of the underlying string, starting
   at the current view pointer position. This guarantees that the SV_Str_view
   returned ends at the null terminator of the underlying string as all
   strings used with SV_Str_views are assumed to be null terminated. It is
   undefined behavior to provide non null terminated strings to any
   SV_Str_view code. */
SV_API SV_Str_view SV_extend(SV_Str_view sv) SV_ATTRIB_PURE;

/* Returns the standard C threeway comparison between cmp(lhs, rhs)
   between two string views.
   lhs LESSER ( -1  ) rhs (lhs is less than rhs)
   lhs EQUAL  (  0  ) rhs (lhs is equal to rhs)
   lhs GREATER(  1  ) rhs (lhs is greater than rhs).
   Comparison is bounded by the shorter SV_Str_view length. ERR is
   returned if bad input is provided such as a SV_Str_view with a
   NULL pointer field. */
SV_API SV_Order SV_compare(SV_Str_view lhs, SV_Str_view rhs) SV_ATTRIB_PURE;

/* Finds the first tokenized position in the string view given any length
   delim SV_Str_view. Skips leading delimiters in construction. If the
   SV_Str_view to be searched stores NULL than the SV_null() is returned. If
   delim stores NULL, that is interpreted as a search for the null terminating
   character or empty string and the size zero substring at the final position
   in the SV_Str_view is returned wich may or may not be the null termiator. If
   no delim is found the entire SV_Str_view is returned. */
SV_API SV_Str_view SV_begin_token(SV_Str_view src,
                                  SV_Str_view delim) SV_ATTRIB_PURE;

/* Returns true if no further tokens are found and position is at the end
   position, meaning a call to SV_next_tok has yielded a size 0 SV_Str_view
   that points at the end of the src SV_Str_view which may or may not be null
   terminated. */
SV_API bool SV_end_token(SV_Str_view src, SV_Str_view tok) SV_ATTRIB_PURE;

/* Advances to the next token in the remaining view separated by the delim.
   Repeating delimiter patterns will be skipped until the next token or end
   of string is found. If SV_Str_view stores NULL the SV_null() placeholder
   is returned. If delim stores NULL the end position of the SV_Str_view
   is returned which may or may not be the null terminator. The tok is
   bounded by the length of the view between two delimiters or the length
   from a delimiter to the end of src, whichever comes first. */
SV_API SV_Str_view SV_next_token(SV_Str_view src, SV_Str_view tok,
                                 SV_Str_view delim) SV_ATTRIB_PURE;

/* Obtains the last token in a string in preparation for reverse tokenized
   iteration. Any delimiters that end the string are skipped, as in the
   forward version. If src is NULL SV_null is returned. If delim is null
   the entire src view is returned. Though the SV_Str_view is tokenized in
   reverse, the token view will start at the first character and be the
   length of the token found. */
SV_API SV_Str_view SV_reverse_begin_token(SV_Str_view src,
                                          SV_Str_view delim) SV_ATTRIB_PURE;

/* Given the current SV_Str_view being iterated through and the current token
   in the iteration returns true if the ending state of a reverse tokenization
   has been reached, false otherwise. */
SV_API bool SV_reverse_end_token(SV_Str_view src,
                                 SV_Str_view tok) SV_ATTRIB_PURE;

/* Advances the token in src to the next token between two delimiters provided
   by delim. Repeating delimiters are skipped until the next token is found.
   If no further tokens can be found an empty SV_Str_view is returned with its
   pointer set to the start of the src string being iterated through. Note
   that a multicharacter delimiter may yield different tokens in reverse
   than in the forward direction when partial matches occur and some portion
   of the delimiter is in a token. This is because the string is now being
   parsed from right to left. However, the token returned starts at the first
   character and is read from left to right between two delimiters as in the
   forward tokenization.  */
SV_API SV_Str_view SV_reverse_next_token(SV_Str_view src, SV_Str_view tok,
                                         SV_Str_view delim) SV_ATTRIB_PURE;

/* Returns a read only pointer to the beginning of the string view,
   the first valid character in the view. If the view stores NULL,
   the placeholder SV_null() is returned. */
SV_API char const *SV_begin(SV_Str_view sv) SV_ATTRIB_PURE;

/* Returns a read only pointer to the end of the string view. This
   may or may not be a null terminated character depending on the
   view. If the view stores NULL, the placeholder SV_null() is returned. */
SV_API char const *SV_end(SV_Str_view sv) SV_ATTRIB_PURE;

/* Returns the reverse iterator beginning, the last character of the
   current view. If the view is null SV_null() is returned. If the
   view is sized zero with a valid pointer that pointer in the
   view is returned. */
SV_API char const *SV_reverse_begin(SV_Str_view sv) SV_ATTRIB_PURE;

/* The ending position of a reverse iteration. It is undefined
   behavior to access or use rend. It is undefined behavior to
   pass in any SV_Str_view not being iterated through as started
   with reverse_begin. */
SV_API char const *SV_reverse_end(SV_Str_view sv) SV_ATTRIB_PURE;

/* Returns the character pointer at the minimum between the indicated
   position and the end of the string view. If NULL is stored by the
   SV_Str_view then SV_null() is returned. */
SV_API char const *SV_pointer(SV_Str_view sv, size_t i) SV_ATTRIB_PURE;

/* The characer in the string at position i with bounds checking.
   If i is greater than or equal to the size of SV_Str_view the null
   terminator character is returned. */
SV_API char SV_at(SV_Str_view sv, size_t i) SV_ATTRIB_PURE;

/* The character at the first position of SV_Str_view. An empty
   SV_Str_view or NULL pointer is valid and will return '\0'. */
SV_API char SV_front(SV_Str_view sv) SV_ATTRIB_PURE;

/* The character at the last position of SV_Str_view. An empty
   SV_Str_view or NULL pointer is valid and will return '\0'. */
SV_API char SV_back(SV_Str_view sv) SV_ATTRIB_PURE;

/*============================  Searching  =================================*/

/* Searches for needle in hay starting from pos. If the needle
   is larger than the hay, or position is greater than hay length,
   then hay length is returned. */
SV_API size_t SV_find(SV_Str_view hay, size_t pos,
                      SV_Str_view needle) SV_ATTRIB_PURE;

/* Searches for the last occurence of needle in hay starting from pos
   from right to left. If found the starting position of the string
   is returned, the same as find. If not found hay size is returned.
   The only difference from find is the search direction. If needle
   is larger than hay, hay length is returned. If the position is
   larger than the hay, the entire hay is searched. */
SV_API size_t SV_reverse_find(SV_Str_view hay, size_t pos,
                              SV_Str_view needle) SV_ATTRIB_PURE;

/* Returns true if the needle is found in the hay, false otherwise. */
SV_API bool SV_contains(SV_Str_view hay, SV_Str_view needle) SV_ATTRIB_PURE;

/* Returns a view of the needle found in hay at the first found
   position. If the needle cannot be found the empty view at the
   hay length position is returned. This may or may not be null
   terminated at that position. If needle is greater than
   hay length an empty view at the end of hay is returned. If
   hay is NULL, SV_null is returned (modeled after strstr). */
SV_API SV_Str_view SV_match(SV_Str_view hay, SV_Str_view needle) SV_ATTRIB_PURE;

/* Returns a view of the needle found in hay at the last found
   position. If the needle cannot be found the empty view at the
   hay length position is returned. This may or may not be null
   terminated at that position. If needle is greater than
   hay length an empty view at hay size is returned. If hay is
   NULL, SV_null is returned (modeled after strstr). */
SV_API SV_Str_view SV_reverse_match(SV_Str_view hay,
                                    SV_Str_view needle) SV_ATTRIB_PURE;

/* Returns true if a prefix shorter than or equal in length to
   the SV_Str_view is present, false otherwise. */
SV_API bool SV_starts_with(SV_Str_view sv, SV_Str_view prefix) SV_ATTRIB_PURE;

/* Removes the minimum between SV_Str_view length and n from the start
   of the SV_Str_view. It is safe to provide n larger than SV_Str_view
   size as that will result in a size 0 view to the end of the
   current view which may or may not be the null terminator. */
SV_API SV_Str_view SV_remove_prefix(SV_Str_view sv, size_t n) SV_ATTRIB_PURE;

/* Returns true if a suffix less or equal in length to SV_Str_view is
   present, false otherwise. */
SV_API bool SV_ends_with(SV_Str_view sv, SV_Str_view suffix) SV_ATTRIB_PURE;

/* Removes the minimum between SV_Str_view length and n from the end. It
   is safe to provide n larger than SV_Str_view and that will result in
   a size 0 view to the end of the current view which may or may not
   be the null terminator. */
SV_API SV_Str_view SV_remove_suffix(SV_Str_view sv, size_t n) SV_ATTRIB_PURE;

/* Finds the first position of an occurence of any character in set.
   If no occurence is found hay size is returned. An empty set (NULL)
   is valid and will return position at hay size. An empty hay
   returns 0. */
SV_API size_t SV_find_first_of(SV_Str_view hay, SV_Str_view set) SV_ATTRIB_PURE;

/* Finds the first position at which no characters in set can be found.
   If the string is all characters in set hay length is returned.
   An empty set (NULL) is valid and will return position 0. An empty
   hay returns 0. */
SV_API size_t SV_find_first_not_of(SV_Str_view hay,
                                   SV_Str_view set) SV_ATTRIB_PURE;

/* Finds the last position of any character in set in hay. If
   no position is found hay size is returned. An empty set (NULL)
   is valid and returns hay size. An empty hay returns 0. */
SV_API size_t SV_find_last_of(SV_Str_view hay, SV_Str_view set) SV_ATTRIB_PURE;

/* Finds the last position at which no character in set can be found.
   An empty set (NULL) is valid and will return the final character
   in the SV_Str_view. An empty hay will return 0. */
SV_API size_t SV_find_last_not_of(SV_Str_view hay,
                                  SV_Str_view set) SV_ATTRIB_PURE;

#endif /* SV_STR_VIEW */
