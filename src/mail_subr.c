/* @(#) $Header: /home/deyke/tmp/cvs/tcp/src/mail_subr.c,v 1.6 1993-05-17 13:45:08 deyke Exp $ */

#include <stdlib.h>
#include <string.h>

#include "timer.h"
#include "mail.h"

/*---------------------------------------------------------------------------*/

char *get_user_from_path(char *path)
{
  char *cp;

  return (cp = strrchr(path, '!')) ? cp + 1 : path;
}

/*---------------------------------------------------------------------------*/

char *get_host_from_path(char *path)
{

  char *cp;
  static char tmp[1024];

  strcpy(tmp, path);
  if (!(cp = strrchr(tmp, '!'))) return "";
  *cp = '\0';
  return (cp = strrchr(tmp, '!')) ? cp + 1 : tmp;
}

/*---------------------------------------------------------------------------*/

void mailer_failed(struct mailsys *sp)
{
  struct mailjob *jp;

  while (jp = sp->jobs) {
    sp->jobs = jp->next;
    free(jp);
  }
  sp->state = MS_FAILURE;
  sp->nexttime = secclock() + RETRYTIME;
}

