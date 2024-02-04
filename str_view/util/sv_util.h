#ifndef SV_UTIL
#define SV_UTIL

#include <stddef.h>
#include <sys/types.h>

/* These utilities are provided to the string view library to avoid
   using a c library <string.h> import. The string view implementation
   is already required to implement a replacement strstr function and
   therefore using other string utilities from the standard library
   would place unneccessary code bloat/repetition on the user when it is
   not needed. Most of these are trivial to implement and provide to the user.
   If they decide they need to import string.h in their own code they are
   free to do so but the string_view library will not force that choice. */

/* Compares strings according to lengths provided. Position length
   is always assumed to be the null terminator even if no null
   terminator is present. */
size_t sv_strnstrn(const char *, ssize_t, const char *, ssize_t);

/* Returns the length of the string segment containing only elements in set. */
size_t sv_strspn(const char *, size_t, const char *set, size_t);

/* Returns the length of the string containing no elements in set. */
size_t sv_strcspn(const char *, size_t, const char *set, size_t);

/* Returns the length of the string O(n). Must be null terminated. */
size_t sv_len(const char *);

/* Returns the length of the string or n whichever comes first. */
size_t sv_nlen(const char *, size_t n);

/* Sets the first n bytes of buffer to int.*/
void *sv_memset(void *, int, size_t);

/* Compares the first n bytes of two buffers. */
int sv_memcmp(const void *, const void *, size_t);

/* Copies the first n bytes of destination to source. No overlap */
void *sv_memcpy(void *dest, const void *src, size_t);

/* Copies the first n bytes of src to dest. No intermediate copies. */
void *sv_memmove(void *dest, const void *src, size_t);

#endif /* SV_UTIL */
