/* @(#) $Header: /home/deyke/tmp/cvs/tcp/src/finger.h,v 1.5 1993-05-17 13:44:54 deyke Exp $ */

/*
 *
 *      Finger support...
 *
 *      Finger header defines.  Written by Michael T. Horne - KA7AXD.
 *      Copyright 1988 by Michael T. Horne, All Rights Reserved.
 *      Permission granted for non-commercial use and copying, provided
 *      that this notice is retained.
 *
 */

#define FINGER_PORT     79      /* TCP port for `who' (finger) service */
#define FINGNAMELEN     8       /* max len of user name */

/* finger protocol control block */
struct finger {
	struct tcb      *tcb;           /* pointer to associated tcb */
	struct session  *session;       /* pointer to finger session */
	char            *user;          /* name of user to finger */
};
#define NULLFING        (struct finger *) 0

extern char             *fingersuf,     /* see files.c */
			*fingerpath;

/* In finger.c: */
void fingcli_rcv(struct tcb *tcb, int cnt);

