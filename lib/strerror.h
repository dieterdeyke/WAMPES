#ifndef _STRERROR_H
#define _STRERROR_H

#ifndef _CONFIGURE_H
#include "configure.h"
#endif

#if !HAS_STRERROR

/* In strerror.c: */
char *strerror(int errnum);

#endif

#endif  /* _STRERROR_H */
