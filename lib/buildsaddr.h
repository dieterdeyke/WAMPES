/* @(#) $Header: /home/deyke/tmp/cvs/tcp/lib/buildsaddr.h,v 1.2 1993-04-26 14:47:17 deyke Exp $ */

#ifndef _BUILDSADDR_H
#define _BUILDSADDR_H

/* In buildsaddr.c: */
struct sockaddr *build_sockaddr __ARGS((const char *name, int *addrlen));

#endif  /* _BUILDSADDR_H */
