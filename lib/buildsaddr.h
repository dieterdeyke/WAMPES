/* @(#) $Header: /home/deyke/tmp/cvs/tcp/lib/buildsaddr.h,v 1.3 1993-05-17 13:44:48 deyke Exp $ */

#ifndef _BUILDSADDR_H
#define _BUILDSADDR_H

/* In buildsaddr.c: */
struct sockaddr *build_sockaddr(const char *name, int *addrlen);

#endif  /* _BUILDSADDR_H */
