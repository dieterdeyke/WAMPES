/* @(#) $Header: /home/deyke/tmp/cvs/tcp/lib/buildsaddr.h,v 1.4 1996-08-11 18:17:33 deyke Exp $ */

#ifndef _BUILDSADDR_H
#define _BUILDSADDR_H

/* In buildsaddr.c: */
struct sockaddr *build_sockaddr(const char *name, int *addrlen);

#endif  /* _BUILDSADDR_H */
