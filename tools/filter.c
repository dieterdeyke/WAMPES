/* @(#) $Header: /home/deyke/tmp/cvs/tcp/tools/filter.c,v 1.2 1993-01-29 06:48:54 deyke Exp $ */

#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/

int main(int argc, char **argv)
{

  char buf[64][256];
  int i;
  int line;
  int lines;

  for (; ; ) {

    for (line = 0; ; ) {
      if (!gets(buf[line])) break;
      line++;
      if (!buf[line-1][0]) break;
    }
    lines = line;

    if (!lines) return 0;

    for (line = 0; line < lines; line++)
      for (i = 1; i < argc; i++)
	if (strstr(buf[line], argv[i])) {
	  for (line = 0; line < lines; line++) puts(buf[line]);
	  goto done;
	}
done:
    ;

  }
}

