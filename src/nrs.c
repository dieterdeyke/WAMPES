/* @(#) $Header: /home/deyke/tmp/cvs/tcp/src/nrs.c,v 1.13 1994-10-06 16:15:33 deyke Exp $ */

/* This module implements the serial line framing method used by
 * net/rom nodes.  This allows the net/rom software to talk to
 * an actual net/rom over its serial interface, which is useful
 * if we want to do packet switching for multi-line wormholes.
 *
 * Copyright 1989 Dan Frank, W9NK
 */
#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "iface.h"
#include "pktdrvr.h"
#include "ax25.h"
#include "nrs.h"
#include "asy.h"
#include "trace.h"
#include "commands.h"

static struct mbuf *nrs_encode(struct mbuf *bp);
static struct mbuf *nrs_decode(int dev,char c);

/* control structures, sort of overlayed on async control blocks */
struct nrs Nrs[ASY_MAX];

int
nrs_init(
struct iface *ifp)
{
	int xdev;
	struct nrs *np;
	char *ifn;

	/* Set up a net/rom serial iface */
	for(xdev = 0;xdev < NRS_MAX;xdev++){
		np = &Nrs[xdev];
		if(np->iface == NULLIF)
			break;
	}
	if(xdev >= NRS_MAX) {
		printf("Too many nrs devices\n");
		return -1;
	}
	/* no call supplied? */
	ifp->ioctl = asy_ioctl;
	ifp->raw = nrs_raw;

	ifp->hwaddr = (char *) mallocw(AXALEN);
	memcpy(ifp->hwaddr,Mycall,AXALEN);
	ifp->xdev = xdev;
	np->iface = ifp;
	np->send = asy_send;
	np->get = get_asy;
#if 0
	ifp->rxproc = newproc( ifn = if_name( ifp, " nrs" ),
		256,nrs_recv,xdev,NULL,NULL,0);
	free(ifn);
#else
	ifp->rxproc = nrs_recv;
#endif
	return 0;
}
int
nrs_free(
struct iface *ifp)
{
	if(Nrs[ifp->xdev].iface == ifp)
		Nrs[ifp->xdev].iface = NULLIF;
	return 0;
}
/* Send a raw net/rom serial frame */
int
nrs_raw(
struct iface *iface,
struct mbuf *bp)
{
	struct mbuf *bp1;

	dump(iface,IF_TRACE_OUT,bp);
	iface->rawsndcnt++;
	iface->lastsent = secclock();

	if((bp1 = nrs_encode(bp)) == NULLBUF){
		free_p(bp);
		return -1;
	}
	return Nrs[iface->xdev].send(iface->dev,bp1);
}

/* Encode a packet in net/rom serial format */
static struct mbuf *
nrs_encode(
struct mbuf *bp)
{
	struct mbuf *lbp;       /* Mbuf containing line-ready packet */
	register char *cp;
	int c;
	unsigned char csum = 0;

	/* Allocate output mbuf that's twice as long as the packet.
	 * This is a worst-case guess (consider a packet full of STX's!)
	 * Add five bytes for STX, ETX, checksum, and two nulls.
	 */
	lbp = alloc_mbuf((uint16)(2*len_p(bp) + 5));
	if(lbp == NULLBUF){
		/* No space; drop */
		free_p(bp);
		return NULLBUF;
	}
	cp = lbp->data;

	*cp++ = STX;

	/* Copy input to output, escaping special characters */
	while((c = PULLCHAR(&bp)) != -1){
		switch(c){
		case STX:
		case ETX:
		case DLE:
			*cp++ = DLE;
			/* notice drop through to default */
		default:
			*cp++ = c;
		}
		csum += c;
	}
	*cp++ = ETX;
	*cp++ = csum;
	*cp++ = NUL;
	*cp++ = NUL;

	lbp->cnt = cp - lbp->data;
	return lbp;
}
/* Process incoming bytes in net/rom serial format
 * When a buffer is complete, return it; otherwise NULLBUF
 */
static struct mbuf *
nrs_decode(
int dev,        /* net/rom unit number */
char c)         /* Incoming character */
{
	struct mbuf *bp;
	register struct nrs *sp;

	sp = &Nrs[dev];
	switch(sp->state) {
		case NRS_INTER:
			if(uchar(c) == STX) {   /* look for start of frame */
				sp->state = NRS_INPACK; /* we're in a packet */
				sp->csum = 0;                           /* reset checksum */
			}
			return NULLBUF;
		case NRS_CSUM:
			bp = sp->rbp;
			sp->rbp = NULLBUF;
			sp->rcnt = 0;
			sp->state = NRS_INTER;  /* go back to inter-packet state */
			if(sp->csum == uchar(c)) {
				sp->packets++;
			} else {
				free_p(bp);     /* drop packet with bad checksum */
				bp = NULLBUF;
				sp->errors++;   /* increment error count */
			}
			return bp;
		case NRS_ESCAPE:
			sp->state = NRS_INPACK; /* end of escape */
			break;                  /* this will drop through to char processing */
		case NRS_INPACK:
			switch (uchar(c)) {
			/* If we see an STX in a packet, assume that previous */
			/* packet was trashed, and start a new packet */
			case STX:
				free_p(sp->rbp);
				sp->rbp = NULLBUF;
				sp->rcnt = 0;
				sp->csum = 0;
				sp->errors++;
				return NULLBUF;
			case ETX:
				sp->state = NRS_CSUM;   /* look for checksum */
				return NULLBUF;
			case DLE:
				sp->state = NRS_ESCAPE;
				return NULLBUF;
			}
	}
	/* If we get to here, it's with a character that's part of the packet.
	 * Make sure there's space for it.
	 */
	if(sp->rbp == NULLBUF){
		/* Allocate first mbuf for new packet */
		if((sp->rbp1 = sp->rbp = alloc_mbuf(NRS_ALLOC)) == NULLBUF) {
			sp->state = NRS_INTER;
			return NULLBUF; /* No memory, drop */
		}
		sp->rcp = sp->rbp->data;
	} else if(sp->rbp1->cnt == NRS_ALLOC){
		/* Current mbuf is full; link in another */
		if((sp->rbp1->next = alloc_mbuf(NRS_ALLOC)) == NULLBUF){
			/* No memory, drop whole thing */
			free_p(sp->rbp);
			sp->rbp = NULLBUF;
			sp->rcnt = 0;
			sp->state = NRS_INTER;
			return NULLBUF;
		}
		sp->rbp1 = sp->rbp1->next;
		sp->rcp = sp->rbp1->data;
	}
	/* Store the character, increment fragment and total
	 * byte counts
	 */
	*sp->rcp++ = c;
	sp->rbp1->cnt++;
	sp->rcnt++;
	sp->csum += uchar(c);   /* add to checksum */
	return NULLBUF;
}

/* Process net/rom serial line I/O */
void
nrs_recv(
struct iface *iface)
{

	char *cp,buf[4096];
	int cnt,dev;
	struct mbuf *bp;
	struct nrs *np;

	dev = iface->xdev;
	np = &Nrs[dev];
	/* Process any pending input */
	cnt = (*np->get)(iface->dev,cp=buf,sizeof(buf));
	while(--cnt >= 0){
		if((bp = nrs_decode(dev,*cp++)) == NULLBUF)
			continue;
		net_route(np->iface,bp);
	}
#if 0
	if(np->iface->rxproc == Curproc)
		np->iface->rxproc = NULLPROC;
#endif
}
/* donrstat:  display status of active net/rom serial interfaces */
int
donrstat(
int argc,
char *argv[],
void *p)
{
	register struct nrs *np;
	register int i;

	printf("Interface   RcvB  NumReceived  CSumErrors\n");

	for(i = 0, np = Nrs; i < ASY_MAX; i++, np++)
		if(np->iface != NULLIF)
			if(printf(" %8s   %4d   %10lu  %10lu\n",
			 np->iface->name, np->rcnt,
			 np->packets, np->errors) == EOF)
				break;

	return 0;
}
