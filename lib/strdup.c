/* @(#) $Header: /home/deyke/tmp/cvs/tcp/lib/strdup.c,v 1.8 1996-03-14 17:07:07 deyke Exp $ */

#include "configure.h"

#if !HAS_STRDUP || defined _AIX

#include <stdlib.h>
#include <string.h>

#include "strdup.h"

char *strdup(const char *s)
{
  char *p;

  if ((p = (char *) malloc(strlen(s) + 1))) strcpy(p, s);
  return p;
}

#else

struct prevent_empty_file_message;

#endif
