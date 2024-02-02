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

size_t sv_strnstrn(const char *, ssize_t, const char *, ssize_t);

size_t sv_strspn(const char *, size_t, const char *, size_t);

size_t sv_strcspn(const char *, size_t, const char *, size_t);

size_t sv_strlen(const char *);

size_t sv_strnlen(const char *, size_t);

void *sv_memset(void *, int, size_t);

int sv_memcmp(const void *, const void *, size_t);

void *sv_memcpy(void *, const void *, size_t);

void *sv_memmove(void *, const void *, size_t);

#endif /* SV_UTIL */
