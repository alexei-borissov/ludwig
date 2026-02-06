/*****************************************************************************
 *
 *  util_string.c
 *
 *  Edinburgh Soft Matter and Statistical Physics Group and
 *  Edinburgh Parallel Computing Centre
 *
 *  (c) 2025 The University of Edinburgh
 *
 *  Kevin Stratford (kevin@epcc.ed.ac.uk)
 *
 *****************************************************************************/

#include <string.h>

/*****************************************************************************
 *
 *  util_strnlen
 *
 *  A replacement for strnlen() which is C. strnlen() is only Posix.
 *
 *  Behaviour is undefined if s is NULL.
 *
 *****************************************************************************/

size_t util_strnlen(const char * s, size_t maxlen) {

  const char * nullchar = (const char *) memchr(s, '\0', maxlen);

  return nullchar ? (size_t) (nullchar - s) : maxlen;
}
