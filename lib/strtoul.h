#ifndef _STRTOUL_H
#define _STRTOUL_H

#ifndef _CONFIGURE_H
#include "configure.h"
#endif

#if !HAVE_STRTOUL

/* In strtoul.c: */
unsigned long strtoul(const char *nptr, char **endptr, register int base);

#endif

#endif  /* _STRTOUL_H */
