/* @(#) $Header: /home/deyke/tmp/cvs/tcp/src/global.h,v 1.33 1994-10-09 08:22:49 deyke Exp $ */

#ifndef _GLOBAL_H
#define _GLOBAL_H

/* Global definitions used by every source file.
 * Some may be compiler dependent.
 *
 * This file depends only on internal macros or those defined on the
 * command line, so it may be safely included first.
 */

/* Resolve C++ name conflicts */

#ifdef __cplusplus
#define EXTERN_C        extern "C"
#define class           class_
#define new             new_
#define private         private_
#define signed
#define this            this_
#define volatile
#else
#define EXTERN_C
#endif

/*
 * Definitions for byte order, according to byte significance from low
 * address to high.
 */
#define LITTLE_ENDIAN   1234    /* LSB first: i386, vax */
#define BIG_ENDIAN      4321    /* MSB first: 68000, ibm, net */
#define PDP_ENDIAN      3412    /* LSB first in word, MSW first in long */

#if defined linux || defined ULTRIX_RISC || defined __386BSD__ || defined __bsdi__
#define BYTE_ORDER      LITTLE_ENDIAN
#else
#define BYTE_ORDER      BIG_ENDIAN
#endif

#if     !defined(AMIGA) && (defined(LATTICE) || defined(MAC) || defined(__TURBOC__))
/* These compilers require special open modes when reading binary files.
 *
 * "The single most brilliant design decision in all of UNIX was the
 * choice of a SINGLE character as the end-of-line indicator" -- M. O'Dell
 *
 * "Whoever picked the end-of-line conventions for MS-DOS and the Macintosh
 * should be shot!" -- P. Karn's corollary to O'Dell's declaration
 */
#define READ_BINARY     "rb"
#define WRITE_BINARY    "wb"
#define APPEND_BINARY   "ab+"
#define READ_TEXT       "rt"
#define WRITE_TEXT      "wt"
#define APPEND_TEXT     "at+"

#else

#define READ_BINARY     "r"
#define WRITE_BINARY    "w"
#define APPEND_BINARY   "a+"
#define READ_TEXT       "r"
#define WRITE_TEXT      "w"
#define APPEND_TEXT     "a+"

#endif

/* These two lines assume that your compiler's longs are 32 bits and
 * shorts are 16 bits. It is already assumed that chars are 8 bits,
 * but it doesn't matter if they're signed or unsigned.
 */
typedef long int32;             /* 32-bit signed integer */
typedef unsigned short uint16;  /* 16-bit unsigned integer */
typedef unsigned char byte_t;   /*  8-bit unsigned integer */
#define uchar(x) ((unsigned char)(x))
#define MAXINT16 65535          /* Largest 16-bit integer */
#define MAXINT32 4294967295L    /* Largest 32-bit integer */
#define NBBY    8               /* 8 bits/byte */

#define HASHMOD 7               /* Modulus used by hash_ip() function */

/* The "interrupt" keyword is non-standard, so make it configurable */
#if     defined(__TURBOC__) && defined(MSDOS)
#define INTERRUPT       void interrupt
#else
#define INTERRUPT       void
#endif

/* Note that these definitions are on by default if none of the Turbo-C style
 * memory model definitions are on; this avoids having to change them when
 * porting to 68K environments.
 */
#if     !defined(__TINY__) && !defined(__SMALL__) && !defined(__MEDIUM__)
#define LARGEDATA       1
#endif

#if     !defined(__TINY__) && !defined(__SMALL__) && !defined(__COMPACT__)
#define LARGECODE       1
#endif

/* Since not all compilers support structure assignment, the ASSIGN()
 * macro is used. This controls how it's actually implemented.
 */
#ifdef  NOSTRUCTASSIGN  /* Version for old compilers that don't support it */
#define ASSIGN(a,b)     memcpy((char *)&(a),(char *)&(b),sizeof(b));
#else                   /* Version for compilers that do */
#define ASSIGN(a,b)     ((a) = (b))
#endif

/* Define null object pointer in case stdio.h isn't included */
#ifndef NULL
/* General purpose NULL pointer */
#define NULL 0
#endif
#define NULLCHAR (char *)0      /* Null character pointer */
#define NULLCHARP (char **)0    /* Null character pointer pointer */
#define NULLINT (int *)0        /* Null integer pointer */
#define NULLFP 0                /* Null pointer to function returning int */
#define NULLVFP 0               /* Null pointer to function returning void */
#define NULLVIFP (INTERRUPT (*)())0
#define NULLFILE (FILE *)0      /* Null file pointer */

/* standard boolean constants */
#define FALSE 0
#define TRUE 1
#define NO 0
#define YES 1

#define CTLA 0x1
#define CTLB 0x2
#define CTLC 0x3
#define CTLD 0x4
#define CTLE 0x5
#define CTLF 0x6
#define CTLG 0x7
#define CTLH 0x8
#define CTLI 0x9
#define CTLJ 0xa
#define CTLK 0xb
#define CTLL 0xc
#define CTLM 0xd
#define CTLN 0xe
#define CTLO 0xf
#define CTLP 0x10
#define CTLQ 0x11
#define CTLR 0x12
#define CTLS 0x13
#define CTLT 0x14
#define CTLU 0x15
#define CTLV 0x16
#define CTLW 0x17
#define CTLX 0x18
#define CTLY 0x19
#define CTLZ 0x1a

#define BELL    CTLG
#define BS      CTLH
#define TAB     CTLI
#define LF      CTLJ
#define FF      CTLL
#define CR      CTLM
#define XON     CTLQ
#define XOFF    CTLS
#define ESC     0x1b
#define DEL     0x7f

/* string equality shorthand */
#define STREQ(x,y) (strcmp(x,y) == 0)

/* Extract a short from a long */
#ifndef hiword
#define hiword(x)       ((uint16)((x) >> 16))
#define loword(x)       ((uint16)(x))
#endif

/* Extract a byte from a short */
#ifndef hibyte
#define hibyte(x)       ((unsigned char)((x) >> 8))
#define lobyte(x)       ((unsigned char)(x))
#endif

/* Extract nibbles from a byte */
#define hinibble(x)     (((x) >> 4) & 0xf)
#define lonibble(x)     ((x) & 0xf)

/* Various low-level and miscellaneous functions */
int availmem(void);
void *callocw(unsigned nelem,unsigned size);
int dirps(void);
int htob(char c);
int htoi(char *);
int readhex(char *,char *,int);
long htol(char *);
char *inbuf(uint16 port,char *buf,uint16 cnt);
uint16 hash_ip(int32 addr);
int istate(void);
void log(void *tcb,const char *fmt,const char *arg);
int log2(uint16 x);
#define ltop(x) ((void *) (x))
void *mallocw(unsigned nb);
int memcnt(char *buf,char c,int size);
char *outbuf(uint16 port,char *buf,uint16 cnt);
#define ptol(x) ((long) (x))
void restore(int);
void rip(char *);
char *smsg(char *msgs[],unsigned nmsgs,unsigned n);
void stktrace(void);
#if 0
char *strdup(const char *);
#endif
int wildmat(char *s,char *p,char **argv);

#ifdef ibm032
#include <stddef.h>
typedef int pid_t;
#endif

#include <stdlib.h>
#ifndef ibm032
#include <string.h>
#endif

int stricmp(char *s1, char *s2);
int strnicmp(char *s1, char *s2, size_t maxlen);

#ifdef  AZTEC
#define rewind(fp)      fseek(fp,0L,0);
#endif

#if     defined(__TURBOC__) && defined(MSDOS)
#define movblock(so,ss,do,ds,c) movedata(ss,so,ds,do,c)

#else

/* General purpose function macros already defined in turbo C */
#ifndef min
#define min(x,y)        ((x)<(y)?(x):(y))       /* Lesser of two args */
#endif
#ifndef max
#define max(x,y)        ((x)>(y)?(x):(y))       /* Greater of two args */
#endif
#ifdef  MSDOS
#define MK_FP(seg,ofs)  ((void far *) \
			(((unsigned long)(seg) << 16) | (unsigned)(ofs)))
#endif
#endif  /* __TURBOC __ */

#ifdef  AMIGA
/* super kludge de WA3YMH */
#ifndef fileno
#include <stdio.h>
#endif
#define fclose(fp)      amiga_fclose(fp)
extern int amiga_fclose(FILE *);
extern FILE *tmpfile(void);

extern char *sys_errlist[];
extern int errno;
#endif

#define inet_ntoa Xinet_ntoa    /* Resolve name conflict */

/* Externals used by getopt */
extern int optind;
extern char *optarg;

/* Threshold setting on available memory */
extern int32 Memthresh;

/* System clock - count of ticks since startup */
extern int32 Clock;

/* Various useful strings */
extern char Badhost[];
extern char Nospace[];
extern char Notval[];
extern char *Hostname;
extern char Version[];
extern char Whitespace[];

/* Your system's end-of-line convention */
extern char Eol[];

/* Your system OS - set in files.c */
extern char System[];

/* Your system's temp directory */
extern char *Tmpdir;

extern unsigned Nfiles; /* Maximum number of open files */
extern unsigned Nsock;  /* Maximum number of open sockets */

extern void (*Gcollect[])();

struct asy;
struct ax25_cb;
struct iface;
struct mbuf;
struct usock;

extern int Debug;
extern int Shortstatus;

/* Use functions in misc.c because some platforms are broken, eg 386BSD */
int Xtolower(int);
int Xtoupper(int);

#endif  /* _GLOBAL_H */
