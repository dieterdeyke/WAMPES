/* @(#) $Header: /home/deyke/tmp/cvs/tcp/src/login.h,v 1.6 1991-04-25 18:27:11 deyke Exp $ */

#ifndef _LOGIN_H
#define _LOGIN_H

/* In login.c: */
void fixutmpfile __ARGS((void));
struct passwd *getpasswdentry __ARGS((char *name, int create));
struct login_cb *login_open __ARGS((char *user, char *protocol,
	void (*read_upcall) __ARGS((void *upcall_arg)),
	void (*close_upcall) __ARGS((void *upcall_arg)),
	void *upcall_arg));
void login_close __ARGS((struct login_cb *tp));
struct mbuf *login_read __ARGS((struct login_cb *tp, int cnt));
void login_write __ARGS((struct login_cb *tp, struct mbuf *bp));

/* In netlog.c: */
void write_log __ARGS((int fd, char *buf, int cnt));

#endif  /* _LOGIN_H */
