#ifndef __lint
static const char rcsid[] = "@(#) $Header: /home/deyke/tmp/cvs/tcp/tools/makeiprt.c,v 1.5 1993-10-10 08:19:33 deyke Exp $";
#endif

#include <sys/types.h>

#include <ctype.h>
#include <fcntl.h>
#include <ndbm.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define DBHOSTADDR      "/tcp/hostaddr"
#define DBHOSTNAME      "/tcp/hostname"
#define LOCALDOMAIN     "ampr.org"

struct cache {
  struct cache *next;
  long addr;
  char name[1];
};

struct iface {
  struct iface *next;
  char name[1];
};

struct route {
  long dest;
  int bits;
  const struct iface *iface;
  long gateway;
  int metric;
  int private;
  struct route *next;
};

struct link {
  struct node *node;
  const struct iface *iface;
  long gateway;
  int private;
  struct link *next;
};

struct node {
  long addr;
  struct link *links;
  struct route *routes;
  int changed;
  struct node *next;
};

static DBM *Dbhostaddr;
static DBM *Dbhostname;
static const struct iface *Loopback_iface;
static int Usegethostby;
static struct cache *Cache;
static struct iface *Ifaces;
static struct node *Nodes;

/*---------------------------------------------------------------------------*/

static void strlwc(char *to, const char *from)
{
  while (*to++ = tolower(*from++ & 0xff)) ;
}

/*---------------------------------------------------------------------------*/

static long aton(const char *name)
{

  const char * cp;
  int i;
  long addr;

  if (!name || !isdigit(*name & 0xff)) return (-1);
  for (cp = name; *cp; cp++)
    if (!isdigit(*cp & 0xff) && *cp != '.') return (-1);
  addr = 0;
  for (i = 24; i >= 0; i -= 8) {
    addr |= atol(name) << i;
    if (!(name = strchr(name, '.'))) break;
    name++;
    if (!*name) break;
  }
  return addr;
}

/*---------------------------------------------------------------------------*/

static void add_to_cache(const char *name, long addr)
{
  struct cache *cp;

  cp = malloc(sizeof(*cp) + strlen(name));
  strcpy(cp->name, name);
  cp->addr = addr;
  cp->next = Cache;
  Cache = cp;
}

/*---------------------------------------------------------------------------*/

static long resolve(const char *name)
{

  char *p;
  char names[3][1024];
  datum daddr;
  datum dname;
  int i;
  long addr;
  struct cache *curr;
  struct cache *prev;
  struct hostent *hp;

  if ((addr = aton(name)) != -1L) return addr;

  strlwc(names[0], name);
  p = names[0] + strlen(names[0]) - 1;
  if (*p == '.') {
    *p = 0;
    names[1][0] = 0;
  } else {
    strcpy(names[1], names[0]);
    strcat(names[0], ".");
    strcat(names[0], LOCALDOMAIN);
    names[2][0] = 0;
  }

  for (i = 0; names[i][0]; i++) {
    for (prev = 0, curr = Cache; curr; prev = curr, curr = curr->next)
      if (!strcmp(curr->name, names[i])) {
	if (prev) {
	  prev->next = curr->next;
	  curr->next = Cache;
	  Cache = curr;
	}
	return curr->addr;
      }
  }

  if (Dbhostaddr || (Dbhostaddr = dbm_open(DBHOSTADDR, O_RDONLY, 0644)))
    for (i = 0; names[i][0]; i++) {
      dname.dptr = names[i];
      dname.dsize = strlen(names[i]) + 1;
      daddr = dbm_fetch(Dbhostaddr, dname);
      if (daddr.dptr) {
	memcpy((char *) &addr, daddr.dptr, sizeof(addr));
	add_to_cache(names[i], addr);
	return addr;
      }
    }

  if (Usegethostby && (hp = gethostbyname(name))) {
    addr = ntohl(((struct in_addr *)(hp->h_addr))->s_addr);
    strlwc(names[0], hp->h_name);
    add_to_cache(names[0], addr);
    return addr;
  }

  fprintf(stderr, "Cannot resolve \"%s\"\n", name);
  exit(1);
  return 0;
}

/*---------------------------------------------------------------------------*/

static const char *resolve_a(long addr)
{

  char buf[1024];
  datum daddr;
  datum dname;
  struct cache *curr;
  struct cache *prev;
  struct hostent *hp;
  struct in_addr in_addr;

  for (prev = 0, curr = Cache; curr; prev = curr, curr = curr->next)
    if (curr->addr == addr) {
      if (prev) {
	prev->next = curr->next;
	curr->next = Cache;
	Cache = curr;
      }
      return Cache->name;
    }

  if (Dbhostname || (Dbhostname = dbm_open(DBHOSTNAME, O_RDONLY, 0644))) {
    daddr.dptr = (char *) &addr;
    daddr.dsize = sizeof(addr);
    dname = dbm_fetch(Dbhostname, daddr);
    if (dname.dptr) {
      add_to_cache(dname.dptr, addr);
      return Cache->name;
    }
  }

  if (Usegethostby) {
    in_addr.s_addr = htonl(addr);
    hp = gethostbyaddr((char *) &in_addr, sizeof(in_addr), AF_INET);
    if (hp) {
      strlwc(buf, hp->h_name);
      add_to_cache(buf, addr);
      return Cache->name;
    }
  }

  sprintf(buf, "%u.%u.%u.%u",
	  (addr >> 24) & 0xff,
	  (addr >> 16) & 0xff,
	  (addr >>  8) & 0xff,
	  (addr      ) & 0xff);
  add_to_cache(buf, addr);
  return Cache->name;
}

/*---------------------------------------------------------------------------*/

static const struct iface *get_iface(const char *name)
{
  struct iface *ip;

  for (ip = Ifaces; ip && strcmp(ip->name, name); ip = ip->next) ;
  if (!ip) {
    ip = malloc(sizeof(*ip) + strlen(name));
    strcpy(ip->name, name);
    ip->next = Ifaces;
    Ifaces = ip;
  }
  return ip;
}

/*---------------------------------------------------------------------------*/

static int is_in(long dest1, int bits1, long dest2, int bits2)
{
  long mask2;

  if (bits1 < bits2) return 0;
  if (!bits2) return 1;
  mask2 = (~0L) << (32 - bits2);
  return ((dest1 & mask2) == dest2);
}

/*---------------------------------------------------------------------------*/

static void add_route(struct node *np, long dest, int bits, const struct iface *iface, long gateway, int metric, int private)
{
  struct route *rp;

  for (rp = np->routes; rp; rp = rp->next)
    if (dest == rp->dest && bits == rp->bits && metric >= rp->metric) return;
  rp = calloc(1, sizeof(*rp));
  rp->dest = dest;
  rp->bits = bits;
  rp->iface = iface;
  rp->gateway = gateway;
  rp->metric = metric;
  rp->private = private;
  rp->next = np->routes;
  np->routes = rp;
  np->changed = 1;
}

/*---------------------------------------------------------------------------*/

static struct node *get_node(long addr)
{
  struct node *np;

  for (np = Nodes; np && np->addr != addr; np = np->next) ;
  if (!np) {
    np = calloc(1, sizeof(*np));
    np->addr = addr;
    np->next = Nodes;
    Nodes = np;
    add_route(np, addr, 32, Loopback_iface, 0, 0, 0);
  }
  return np;
}

/*---------------------------------------------------------------------------*/

static void read_routes(void)
{

  FILE * fp;
  char deststr[1024];
  char gatewaystr[1024];
  char hoststr[1024];
  char ifacestr[1024];
  char line[1024];
  char privatestr[1024];
  int bits;
  int linnum;
  long dest;
  long gateway;
  long host;
  struct node *np;

  if (!(fp = fopen("ip_routes", "r"))) {
    perror("ip_routes");
    exit(1);
  }
  linnum = 0;
  while (fgets(line, sizeof(line), fp)) {
    linnum++;
    if (*line == '\n' || *line == '#') continue;
    strcpy(gatewaystr, "0");
    *privatestr = 0;
    if (sscanf(line, "%s %s %d %s %s %s", hoststr, deststr, &bits, ifacestr, gatewaystr, privatestr) < 4 || bits < 0 || bits > 32) {
      fprintf(stderr, "ip_routes: syntax error in line %d\n", linnum);
      exit(1);
    }
    host = resolve(hoststr);
    if (bits)
      dest = resolve(deststr) & ((~0L) << (32 - bits));
    else
      dest = 0;
    gateway = resolve(gatewaystr);
    np = get_node(host);
    if (bits == 32 && !gateway) gateway = dest;
    add_route(np, dest, bits, get_iface(ifacestr), gateway, 0, *privatestr);
  }
}

/*---------------------------------------------------------------------------*/

static void create_links(void)
{

  struct link *lp;
  struct link *nlp;
  struct node *nnp;
  struct node *np;
  struct route *rp;

  for (np = Nodes; np; np = np->next)
    for (rp = np->routes; rp; rp = rp->next)
      if (rp->bits == 32 && rp->dest != np->addr) {
	for (nnp = Nodes; nnp && nnp->addr != rp->dest; nnp = nnp->next) ;
	if (nnp) {
	  lp = malloc(sizeof(*lp));
	  lp->node = nnp;
	  lp->iface = rp->iface;
	  lp->gateway = rp->gateway;
	  lp->private = rp->private;
	  lp->next = np->links;
	  np->links = lp;
	}
      }

  for (np = Nodes; np; np = np->next)
    for (lp = np->links; lp; lp = lp->next) {
      for (nlp = lp->node->links; nlp; nlp = nlp->next)
	if (nlp->node == np) goto Found;
      fprintf(stderr, "Missing route from %s to %s\n", resolve_a(lp->node->addr), resolve_a(np->addr));
      exit(1);
Found:
      ;
    }

}

/*---------------------------------------------------------------------------*/

static void propagate_routes(void)
{

  int changed;
  struct link *lp;
  struct node *np;
  struct route *rp;

  do {
    changed = 0;
    for (np = Nodes; np; np = np->next) {
      np->changed = 0;
      for (lp = np->links; lp; lp = lp->next)
	for (rp = lp->node->routes; rp; rp = rp->next)
	  if (!rp->private)
	    add_route(np, rp->dest, rp->bits, lp->iface, lp->gateway, rp->metric + 1, lp->private);
      if (np->changed) changed = 1;
    }
  } while (changed);
}

/*---------------------------------------------------------------------------*/

static void sort_routes(void)
{

  int bits;
  struct node *np;
  struct route *rp;
  struct route *rt[33];

  for (np = Nodes; np; np = np->next) {
    for (bits = 0; bits <= 32; bits++)
      rt[bits] = 0;
    while (rp = np->routes) {
      np->routes = rp->next;
      rp->next = rt[rp->bits];
      rt[rp->bits] = rp;
    }
    for (bits = 0; bits <= 32; bits++) {
      while (rp = rt[bits]) {
	rt[bits] = rp->next;
	rp->next = np->routes;
	np->routes = rp;
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

static void merge_routes(void)
{

  struct node *np;
  struct route *curr;
  struct route *prev;
  struct route *rp;

  for (np = Nodes; np; np = np->next) {
Retry:
    for (prev = 0, curr = np->routes; curr; prev = curr, curr = curr->next) {
      for (rp = curr->next; rp; rp = rp->next) {
	if (is_in(curr->dest, curr->bits, rp->dest, rp->bits)) {
	  if (curr->iface != rp->iface || curr->gateway != rp->gateway)
	    break;
	  if (prev)
	    prev->next = curr->next;
	  else
	    np->routes = curr->next;
	  free(curr);
	  goto Retry;
	}
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

static void print_links(void)
{

  struct link *lp;
  struct node *np;

  printf("\nLINKS:\n\n");
  for (np = Nodes; np; np = np->next) {
    for (lp = np->links; lp; lp = lp->next)
      printf("%-20s %-20s %-8s %-20s\n", resolve_a(np->addr), resolve_a(lp->node->addr), lp->iface->name, resolve_a(lp->gateway));
  }
}

/*---------------------------------------------------------------------------*/

static void print_routes(void)
{

  const char *gateway;
  char dest[1024];
  struct node *np;
  struct route *rp;

  printf("\nROUTES:\n\n");
  for (np = Nodes; np; np = np->next) {
    for (rp = np->routes; rp; rp = rp->next) {
      if (!rp->bits)
	strcpy(dest, "default");
      else if (rp->bits == 32)
	strcpy(dest, resolve_a(rp->dest));
      else
	sprintf(dest, "%s/%d", resolve_a(rp->dest), rp->bits);
      if (rp->iface != Loopback_iface) {
	if (!rp->gateway || rp->bits == 32 && rp->dest == rp->gateway)
	  gateway = "";
	else
	  gateway = resolve_a(rp->gateway);
	printf("%-20s %-20s %-8s %-20s %2d %c\n", resolve_a(np->addr), dest, rp->iface->name, gateway, rp->metric, rp->private ? 'P' : ' ');
      }
    }
    printf("\n");
  }
}

/*---------------------------------------------------------------------------*/

static void make_route_files(void)
{

  FILE * fp;
  char *cp;
  char dest[1024];
  char filename[1024];
  const char * gateway;
  struct node *np;
  struct route *rp;

  for (np = Nodes; np; np = np->next) {
    strcpy(dest, resolve_a(np->addr));
    if (cp = strchr(dest, '.')) *cp = 0;
    sprintf(filename, "/tmp/iprt.%s", dest);
    if (!(fp = fopen(filename, "w"))) {
      perror(filename);
      exit(1);
    }
    fprintf(fp, "\n");
    for (rp = np->routes; rp; rp = rp->next) {
      if (!rp->bits)
	strcpy(dest, "default");
      else if (rp->bits == 32)
	strcpy(dest, resolve_a(rp->dest));
      else
	sprintf(dest, "%s/%d", resolve_a(rp->dest), rp->bits);
      if (rp->iface != Loopback_iface) {
	if (!rp->gateway || rp->bits == 32 && rp->dest == rp->gateway)
	  gateway = "";
	else
	  gateway = resolve_a(rp->gateway);
	fprintf(fp, "route add%c %-20s %-8s %-20s\n", rp->private ? 'p' : ' ', dest, rp->iface->name, gateway);
      }
    }
    fprintf(fp, "\n");
    fclose(fp);
  }
}

/*---------------------------------------------------------------------------*/

int main()
{
  Loopback_iface = get_iface("loopback");
  add_to_cache("hpcsos.ampr.org", aton("44.1.1.1"));

  read_routes();
  create_links();
  propagate_routes();
  sort_routes();
  merge_routes();

#if 0
  print_links();
#endif
#if 0
  print_routes();
#endif
#if 1
  make_route_files();
#endif

  return 0;
}

