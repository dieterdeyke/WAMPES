/* @(#) $Header: /home/deyke/tmp/cvs/tcp/src/icmpdump.c,v 1.6 1994-10-06 16:15:26 deyke Exp $ */

/* ICMP header tracing
 * Copyright 1991 Phil Karn, KA9Q
 */
#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "internet.h"
#include "netuser.h"
#include "icmp.h"
#include "trace.h"
#include "ip.h"

/* Dump an ICMP header */
void
icmp_dump(
FILE *fp,
struct mbuf **bpp,
int32 source,int32 dest,
int check)              /* If 0, bypass checksum verify */
{
	struct icmp icmp;
	uint16 csum;

	if(bpp == NULLBUFP || *bpp == NULLBUF)
		return;
	csum = cksum(NULLHEADER,*bpp,len_p(*bpp));

	ntohicmp(&icmp,bpp);

	fprintf(fp,"ICMP: type %s",smsg(Icmptypes,ICMP_TYPES,uchar(icmp.type)));

	switch(uchar(icmp.type)){
	case ICMP_DEST_UNREACH:
		fprintf(fp," code %s",smsg(Unreach,NUNREACH,uchar(icmp.code)));
		break;
	case ICMP_REDIRECT:
		fprintf(fp," code %s",smsg(Redirect,NREDIRECT,uchar(icmp.code)));
		fprintf(fp," new gateway %s",inet_ntoa(icmp.args.address));
		break;
	case ICMP_TIME_EXCEED:
		fprintf(fp," code %s",smsg(Exceed,NEXCEED,uchar(icmp.code)));
		break;
	case ICMP_PARAM_PROB:
		fprintf(fp," pointer %u",icmp.args.pointer);
		break;
	case ICMP_ECHO:
	case ICMP_ECHO_REPLY:
	case ICMP_INFO_RQST:
	case ICMP_INFO_REPLY:
	case ICMP_TIMESTAMP:
	case ICMP_TIME_REPLY:
		fprintf(fp," id %u seq %u",icmp.args.echo.id,icmp.args.echo.seq);
		break;
	}
	if(check && csum != 0){
		fprintf(fp," CHECKSUM ERROR (%u)",csum);
	}
	putc('\n',fp);
	/* Dump the offending IP header, if any */
	switch(icmp.type){
	case ICMP_DEST_UNREACH:
	case ICMP_TIME_EXCEED:
	case ICMP_PARAM_PROB:
	case ICMP_QUENCH:
	case ICMP_REDIRECT:
		fprintf(fp,"Returned ");
		ip_dump(fp,bpp,0);
	}
}

