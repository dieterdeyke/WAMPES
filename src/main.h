/* @(#) $Header: /home/deyke/tmp/cvs/tcp/src/main.h,v 1.7 1996-08-11 18:16:09 deyke Exp $ */

#ifndef _MAIN_H
#define _MAIN_H

#ifndef _PROC_H
#include "proc.h"
#endif

extern char Badhost[];
extern char *Hostname;
extern char Nospace[];                  /* Generic malloc fail message */

extern int main_exit;                   /* from main program (flag) */
extern int stop_repeat;
extern long StartTime;                  /* time that NOS was started */

void keyboard(int,void *,void *);

#endif  /* _MAIN_H */
