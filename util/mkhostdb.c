#ifndef __lint
static char rcsid[] = "@(#) $Header: /home/deyke/tmp/cvs/tcp/util/mkhostdb.c,v 1.2 1992-11-12 15:21:05 deyke Exp $";
#endif

#define _HPUX_SOURCE

#include <ctype.h>
#include <fcntl.h>
#include <ndbm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__STDC__)
#define __ARGS(x)       x
#else
#define __ARGS(x)       ()
#define const
#endif

#define DBHOSTADDR      "/tcp/hostaddr"
#define DBHOSTNAME      "/tcp/hostname"
#define DOMAINFILE      "/tcp/domain.txt"
#define HOSTSFILE       "/tcp/hosts"
#define LOCALDOMAIN     "ampr.org"
#define LOCALDOMAINFILE "/tcp/domain.local"

static DBM *Dbhostaddr;
static DBM *Dbhostname;

static int aton __ARGS((const char *name, long *addrptr));
static char *ntoa __ARGS((long addr));
static void store_in_db __ARGS((const char *name, const char *addrstr));
static void fix_line __ARGS((char *line));
static void read_hosts_file __ARGS((const char *filename));
static void read_domain_file __ARGS((const char *filename));
static void qaddr __ARGS((const char *name));
static void qname __ARGS((const char *addrstr));
int main __ARGS((int argc, char **argv));

/*---------------------------------------------------------------------------*/

static int aton(name, addrptr)
const char *name;
long *addrptr;
{

  const char * p;
  int i;
  long addr;

  if (!name || !isdigit(*name & 0xff)) return (-1);
  for (p = name; *p; p++)
    if (!isdigit(*p & 0xff) && *p != '.') return (-1);
  addr = 0;
  for (i = 24; i >= 0; i -= 8) {
    addr |= atol(name) << i;
    if (!(name = strchr(name, '.'))) break;
    name++;
    if (!*name) break;
  }
  *addrptr = addr;
  return 0;
}

/*---------------------------------------------------------------------------*/

static char *ntoa(addr)
long addr;
{
  static char buf[16];

  sprintf(buf, "%d.%d.%d.%d",
	  (addr >> 24) & 0xff,
	  (addr >> 16) & 0xff,
	  (addr >>  8) & 0xff,
	  (addr      ) & 0xff);
  return buf;
}

/*---------------------------------------------------------------------------*/

static void store_in_db(name, addrstr)
const char *name;
const char *addrstr;
{

  datum daddr;
  datum dname;
  int i;
  long addr;

  if (aton(addrstr, &addr) || !addr || !~addr) return;
  dname.dptr = (char *) name;
  dname.dsize = strlen(name) + 1;
  daddr.dptr = (char *) & addr;
  daddr.dsize = sizeof(addr);
  i = dbm_store(Dbhostaddr, dname, daddr, DBM_INSERT);
  if (i < 0) {
    perror("dbm_store");
    exit(1);
  }
  if (i > 0) fprintf(stderr, "duplicate name: %s\n", name);
  i = dbm_store(Dbhostname, daddr, dname, DBM_INSERT);
  if (i < 0) {
    perror("dbm_store");
    exit(1);
  }
  if (i > 0) fprintf(stderr, "duplicate addr: %s\n", addrstr);
}

/*---------------------------------------------------------------------------*/

static void fix_line(line)
char *line;
{
  for (; *line; line++) {
    if (*line == ';' || *line == '#') {
      *line = 0;
      return;
    }
    *line = tolower(*line & 0xff);
  }
}

/*---------------------------------------------------------------------------*/

static void read_hosts_file(filename)
const char *filename;
{

  FILE * fp;
  char addrstr[1024];
  char line[1024];
  char name[1024];

  if (!(fp = fopen(filename, "r"))) {
    perror(filename);
    return;
  }
  while (fgets(line, sizeof(line), fp)) {
    fix_line(line);
    if (sscanf(line, "%s %s", addrstr, name) == 2) {
      strcat(name, ".");
      strcat(name, LOCALDOMAIN);
      store_in_db(name, addrstr);
    }
  }
  fclose(fp);
}

/*---------------------------------------------------------------------------*/

static void read_domain_file(filename)
const char *filename;
{

  FILE * fp;
  char *p;
  char fullname[1024];
  char line[1024];
  char name[1024];
  char origin[1024];
  const char * delim = " \t\n";
  datum daddr;
  datum dname;
  int len;
  long addr;

  strcpy(origin, LOCALDOMAIN);
  *name = 0;
  if (!(fp = fopen(filename, "r"))) {
    perror(filename);
    return;
  }
  while (fgets(line, sizeof(line), fp)) {
    fix_line(line);

    if (!strncmp(line, "$origin", 7)) {
      if (p = strtok(line + 7, delim)) strcpy(origin, p);
      continue;
    }

    if (!(p = strtok(line, delim))) continue;

    if (!isspace(*line & 0xff)) {
      strcpy(name, p);
      p = strtok((char *) 0, delim);
    }

    while (p && (isdigit(*p & 0xff) || !strcmp(p, "in")))
      p = strtok((char *) 0, delim);
    if (!p) continue;

    if (strcmp(p, "a")) continue;

    if (!(p = strtok((char *) 0, delim))) continue;

    if (!strcmp(name, "@"))
      strcpy(fullname, origin);
    else if (name[strlen(name)-1] == '.')
      strcpy(fullname, name);
    else
      sprintf(fullname, "%s.%s", name, origin);

    len = strlen(fullname);
    if (len && fullname[len-1] == '.') fullname[len-1] = 0;

    store_in_db(fullname, p);

  }
  fclose(fp);

  strcpy(origin, LOCALDOMAIN);
  *name = 0;
  if (!(fp = fopen(filename, "r"))) {
    perror(filename);
    return;
  }
  while (fgets(line, sizeof(line), fp)) {
    fix_line(line);

    if (!strncmp(line, "$origin", 7)) {
      if (p = strtok(line + 7, delim)) strcpy(origin, p);
      continue;
    }

    if (!(p = strtok(line, delim))) continue;

    if (!isspace(*line & 0xff)) {
      strcpy(name, p);
      p = strtok((char *) 0, delim);
    }

    while (p && (isdigit(*p & 0xff) || !strcmp(p, "in")))
      p = strtok((char *) 0, delim);
    if (!p) continue;

    if (strcmp(p, "cname")) continue;

    if (!(p = strtok((char *) 0, delim))) continue;

    len = strlen(p);
    if (len && p[len-1] == '.') p[len-1] = 0;

    dname.dptr = p;
    dname.dsize = strlen(p) + 1;
    daddr = dbm_fetch(Dbhostaddr, dname);
    if (!daddr.dptr) {
      fprintf(stderr, "no such key: %s\n", p);
      continue;
    }
    memcpy(&addr, daddr.dptr, sizeof(addr));

    if (!strcmp(name, "@"))
      strcpy(fullname, origin);
    else
      strcpy(fullname, name);

    len = strlen(fullname);
    if (len && fullname[len-1] == '.') fullname[len-1] = 0;

    store_in_db(fullname, ntoa(addr));

  }
  fclose(fp);
}

/*---------------------------------------------------------------------------*/

static void qaddr(name)
const char *name;
{

  char fullname[1024];
  datum daddr;
  datum dname;
  int len;
  long addr;

  strcpy(fullname, name);
  len = strlen(fullname);

  if (len && fullname[len-1] == '.') {
    fullname[len-1] = 0;
    dname.dptr = fullname;
    dname.dsize = strlen(fullname) + 1;
    daddr = dbm_fetch(Dbhostaddr, dname);
    if (daddr.dptr) {
      memcpy(&addr, daddr.dptr, sizeof(addr));
      printf("%s  %s\n", ntoa(addr), fullname);
    } else
      fprintf(stderr, "no such key: %s\n", fullname);
    return;
  }

  strcat(fullname, ".");
  strcat(fullname, LOCALDOMAIN);
  dname.dptr = fullname;
  dname.dsize = strlen(fullname) + 1;
  daddr = dbm_fetch(Dbhostaddr, dname);
  if (daddr.dptr) {
    memcpy(&addr, daddr.dptr, sizeof(addr));
    printf("%s  %s\n", ntoa(addr), fullname);
    return;
  }

  dname.dptr = (char *) name;
  dname.dsize = strlen(name) + 1;
  daddr = dbm_fetch(Dbhostaddr, dname);
  if (daddr.dptr) {
    memcpy(&addr, daddr.dptr, sizeof(addr));
    printf("%s  %s\n", ntoa(addr), name);
  } else
    fprintf(stderr, "no such key: %s\n", name);
}

/*---------------------------------------------------------------------------*/

static void qname(addrstr)
const char *addrstr;
{

  datum daddr;
  datum dname;
  long addr;

  if (aton(addrstr, &addr)) {
    fprintf(stderr, "no such key: %s\n", addrstr);
    return;
  }
  daddr.dptr = (char *) & addr;
  daddr.dsize = sizeof(addr);
  dname = dbm_fetch(Dbhostname, daddr);
  if (dname.dptr)
    printf("%s  %s\n", ntoa(addr), dname.dptr);
  else
    fprintf(stderr, "no such key: %s\n", addrstr);
}

/*---------------------------------------------------------------------------*/

int main(argc, argv)
int argc;
char **argv;
{

  char buf[1024];
  int i;

  if (argc >= 1 && strstr(*argv, "qaddr")) {
    if (!(Dbhostaddr = dbm_open(DBHOSTADDR, O_RDONLY, 0644))) {
      perror(DBHOSTADDR);
      exit(1);
    }
    for (i = 1; i < argc; i++) qaddr(argv[i]);
    dbm_close(Dbhostaddr);
  } else if (argc >= 1 && strstr(*argv, "qname")) {
    if (!(Dbhostname = dbm_open(DBHOSTNAME, O_RDONLY, 0644))) {
      perror(DBHOSTNAME);
      exit(1);
    }
    for (i = 1; i < argc; i++) qname(argv[i]);
    dbm_close(Dbhostname);
  } else {
    sprintf(buf, "%s.dir", DBHOSTADDR);
    unlink(buf);
    sprintf(buf, "%s.pag", DBHOSTADDR);
    unlink(buf);
    sprintf(buf, "%s.dir", DBHOSTNAME);
    unlink(buf);
    sprintf(buf, "%s.pag", DBHOSTNAME);
    unlink(buf);
    if (!(Dbhostname = dbm_open(DBHOSTNAME, O_RDWR | O_CREAT, 0644))) {
      perror(DBHOSTNAME);
      exit(1);
    }
    if (!(Dbhostaddr = dbm_open(DBHOSTADDR, O_RDWR | O_CREAT, 0644))) {
      perror(DBHOSTADDR);
      exit(1);
    }
    store_in_db("localhost", "127.0.0.1");
    read_domain_file(LOCALDOMAINFILE);
    read_hosts_file(HOSTSFILE);
    read_domain_file(DOMAINFILE);
    dbm_close(Dbhostname);
    dbm_close(Dbhostaddr);
  }
  return 0;
}
