#ifndef _STRDUP_H
#define _STRDUP_H

#ifndef _CONFIGURE_H
#include "configure.h"
#endif

#if !HAVE_STRDUP

/* In strdup.c: */
char *strdup(const char *s);

#endif

#endif  /* _STRDUP_H */
