/* @(#) $Header: /home/deyke/tmp/cvs/tcp/lib/strdup.h,v 1.3 1996-08-11 18:17:33 deyke Exp $ */

#ifndef _STRDUP_H
#define _STRDUP_H

#ifndef _CONFIGURE_H
#include "configure.h"
#endif

#if !HAS_STRDUP

/* In strdup.c: */
char *strdup(const char *s);

#endif

#endif  /* _STRDUP_H */
