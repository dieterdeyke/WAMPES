/* @(#) $Header: /home/deyke/tmp/cvs/tcp/src/nrdump.c,v 1.3 1994-10-06 16:15:33 deyke Exp $ */

/* NET/ROM header tracing routines
 * Copyright 1991 Phil Karn, KA9Q
 */
#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "timer.h"
#include "ax25.h"
#include "netrom.h"
#include "trace.h"

/* Display NET/ROM network and transport headers */
void
netrom_dump(
FILE *fp,
struct mbuf **bpp,
int check)
{
	char src[AXALEN],dest[AXALEN];
	char tmp[AXBUF];
	char thdr[NR4MINHDR];
	register i;

	if(bpp == NULLBUFP || *bpp == NULLBUF)
		return;
	/* See if it is a routing broadcast */
	if(uchar(*(*bpp)->data) == NR3NODESIG) {
		(void)PULLCHAR(bpp);            /* Signature */
		pullup(bpp,tmp,ALEN);
		tmp[ALEN] = '\0';
		fprintf(fp,"NET/ROM Routing: %s\n",tmp);
		for(i = 0;i < NRDESTPERPACK;i++) {
			if (pullup(bpp,src,AXALEN) < AXALEN)
				break;
			fprintf(fp,"        %12s",pax25(tmp,src));
			pullup(bpp,tmp,ALEN);
			tmp[ALEN] = '\0';
			fprintf(fp,"%8s",tmp);
			pullup(bpp,src,AXALEN);
			fprintf(fp,"    %12s",pax25(tmp,src));
			tmp[0] = PULLCHAR(bpp);
			fprintf(fp,"    %3u\n",uchar(tmp[0]));
		}
		return;
	}
	/* Decode network layer */
	pullup(bpp,src,AXALEN);
	fprintf(fp,"NET/ROM: %s",pax25(tmp,src));

	pullup(bpp,dest,AXALEN);
	fprintf(fp,"->%s",pax25(tmp,dest));

	i = PULLCHAR(bpp);
	fprintf(fp," ttl %d\n",i);

	/* Read first five bytes of "transport" header */
	pullup(bpp,thdr,NR4MINHDR);
	switch(thdr[4] & NR4OPCODE){
	case NR4OPPID:  /* network PID extension */
		if (thdr[0] == NRPROTO_IP && thdr[1] == NRPROTO_IP) {
			ip_dump(fp,bpp,check) ;
			return;
		}
		else
			fprintf(fp,"         protocol family %x, proto %x",
			 uchar(thdr[0]), uchar(thdr[1])) ;
		break ;
	case NR4OPCONRQ:        /* Connect request */
		fprintf(fp,"         conn rqst: ckt %d/%d",uchar(thdr[0]),uchar(thdr[1]));
		i = PULLCHAR(bpp);
		fprintf(fp," wnd %d",i);
		pullup(bpp,src,AXALEN);
		fprintf(fp," %s",pax25(tmp,src));
		pullup(bpp,dest,AXALEN);
		fprintf(fp,"@%s",pax25(tmp,dest));
		break;
	case NR4OPCONAK:        /* Connect acknowledgement */
		fprintf(fp,"         conn ack: ur ckt %d/%d my ckt %d/%d",
		 uchar(thdr[0]), uchar(thdr[1]), uchar(thdr[2]),
		 uchar(thdr[3]));
		i = PULLCHAR(bpp);
		fprintf(fp," wnd %d",i);
		break;
	case NR4OPDISRQ:        /* Disconnect request */
		fprintf(fp,"         disc: ckt %d/%d",
		 uchar(thdr[0]),uchar(thdr[1]));
		break;
	case NR4OPDISAK:        /* Disconnect acknowledgement */
		fprintf(fp,"         disc ack: ckt %d/%d",
		 uchar(thdr[0]),uchar(thdr[1]));
		break;
	case NR4OPINFO: /* Information (data) */
		fprintf(fp,"         info: ckt %d/%d",
		 uchar(thdr[0]),uchar(thdr[1]));
		fprintf(fp," txseq %d rxseq %d",
		 uchar(thdr[2]), uchar(thdr[3]));
		break;
	case NR4OPACK:  /* Information acknowledgement */
		fprintf(fp,"         info ack: ckt %d/%d",
		 uchar(thdr[0]),uchar(thdr[1]));
		fprintf(fp," txseq %d rxseq %d",
		 uchar(thdr[2]), uchar(thdr[3]));
		break;
	default:
		fprintf(fp,"         unknown transport type %d",
		 thdr[4] & 0x0f) ;
		break;
	}
	if(thdr[4] & NR4CHOKE)
		fprintf(fp," CHOKE");
	if(thdr[4] & NR4NAK)
		fprintf(fp," NAK");
	if(thdr[4] & NR4MORE)
		fprintf(fp," MORE");
	putc('\n',fp);
}

