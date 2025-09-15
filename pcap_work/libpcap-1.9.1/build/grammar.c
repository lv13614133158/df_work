/* original parser id follows */
/* yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93" */
/* (use YYMAJOR/YYMINOR for ifdefs dependent on parser version) */

#define YYBYACC 1
#define YYMAJOR 2
#define YYMINOR 0
#define YYPATCH 20220114

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)
#define YYENOMEM       (-2)
#define YYEOF          0
#undef YYBTYACC
#define YYBTYACC 0
#define YYDEBUGSTR YYPREFIX "debug"

#ifndef yyparse
#define yyparse    pcap_parse
#endif /* yyparse */

#ifndef yylex
#define yylex      pcap_lex
#endif /* yylex */

#ifndef yyerror
#define yyerror    pcap_error
#endif /* yyerror */

#ifndef yychar
#define yychar     pcap_char
#endif /* yychar */

#ifndef yyval
#define yyval      pcap_val
#endif /* yyval */

#ifndef yylval
#define yylval     pcap_lval
#endif /* yylval */

#ifndef yydebug
#define yydebug    pcap_debug
#endif /* yydebug */

#ifndef yynerrs
#define yynerrs    pcap_nerrs
#endif /* yynerrs */

#ifndef yyerrflag
#define yyerrflag  pcap_errflag
#endif /* yyerrflag */

#ifndef yylhs
#define yylhs      pcap_lhs
#endif /* yylhs */

#ifndef yylen
#define yylen      pcap_len
#endif /* yylen */

#ifndef yydefred
#define yydefred   pcap_defred
#endif /* yydefred */

#ifndef yystos
#define yystos     pcap_stos
#endif /* yystos */

#ifndef yydgoto
#define yydgoto    pcap_dgoto
#endif /* yydgoto */

#ifndef yysindex
#define yysindex   pcap_sindex
#endif /* yysindex */

#ifndef yyrindex
#define yyrindex   pcap_rindex
#endif /* yyrindex */

#ifndef yygindex
#define yygindex   pcap_gindex
#endif /* yygindex */

#ifndef yytable
#define yytable    pcap_table
#endif /* yytable */

#ifndef yycheck
#define yycheck    pcap_check
#endif /* yycheck */

#ifndef yyname
#define yyname     pcap_name
#endif /* yyname */

#ifndef yyrule
#define yyrule     pcap_rule
#endif /* yyrule */

#if YYBTYACC

#ifndef yycindex
#define yycindex   pcap_cindex
#endif /* yycindex */

#ifndef yyctable
#define yyctable   pcap_ctable
#endif /* yyctable */

#endif /* YYBTYACC */

#define YYPREFIX "pcap_"

#define YYPURE 1

#line 27 "../grammar.y"
/*
 * Copyright (c) 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#ifndef _WIN32
#include <sys/types.h>
#include <sys/socket.h>

#if __STDC__
struct mbuf;
struct rtentry;
#endif

#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* _WIN32 */

#include <stdio.h>

#include "diag-control.h"

#include "pcap-int.h"

#include "gencode.h"
#include "grammar.h"
#include "scanner.h"

#ifdef HAVE_NET_PFVAR_H
#include <net/if.h>
#include <net/pfvar.h>
#include <net/if_pflog.h>
#endif
#include "llc.h"
#include "ieee80211.h"
#include <pcap/namedb.h>

#ifdef HAVE_OS_PROTO_H
#include "os-proto.h"
#endif

#ifdef YYBYACC
/*
 * Both Berkeley YACC and Bison define yydebug (under whatever name
 * it has) as a global, but Bison does so only if YYDEBUG is defined.
 * Berkeley YACC define it even if YYDEBUG isn't defined; declare it
 * here to suppress a warning.
 */
#if !defined(YYDEBUG)
extern int yydebug;
#endif

/*
 * In Berkeley YACC, yynerrs (under whatever name it has) is global,
 * even if it's building a reentrant parser.  In Bison, it's local
 * in reentrant parsers.
 *
 * Declare it to squelch a warning.
 */
extern int yynerrs;
#endif

#define QSET(q, p, d, a) (q).proto = (unsigned char)(p),\
			 (q).dir = (unsigned char)(d),\
			 (q).addr = (unsigned char)(a)

struct tok {
	int v;			/* value */
	const char *s;		/* string */
};

static const struct tok ieee80211_types[] = {
	{ IEEE80211_FC0_TYPE_DATA, "data" },
	{ IEEE80211_FC0_TYPE_MGT, "mgt" },
	{ IEEE80211_FC0_TYPE_MGT, "management" },
	{ IEEE80211_FC0_TYPE_CTL, "ctl" },
	{ IEEE80211_FC0_TYPE_CTL, "control" },
	{ 0, NULL }
};
static const struct tok ieee80211_mgt_subtypes[] = {
	{ IEEE80211_FC0_SUBTYPE_ASSOC_REQ, "assocreq" },
	{ IEEE80211_FC0_SUBTYPE_ASSOC_REQ, "assoc-req" },
	{ IEEE80211_FC0_SUBTYPE_ASSOC_RESP, "assocresp" },
	{ IEEE80211_FC0_SUBTYPE_ASSOC_RESP, "assoc-resp" },
	{ IEEE80211_FC0_SUBTYPE_REASSOC_REQ, "reassocreq" },
	{ IEEE80211_FC0_SUBTYPE_REASSOC_REQ, "reassoc-req" },
	{ IEEE80211_FC0_SUBTYPE_REASSOC_RESP, "reassocresp" },
	{ IEEE80211_FC0_SUBTYPE_REASSOC_RESP, "reassoc-resp" },
	{ IEEE80211_FC0_SUBTYPE_PROBE_REQ, "probereq" },
	{ IEEE80211_FC0_SUBTYPE_PROBE_REQ, "probe-req" },
	{ IEEE80211_FC0_SUBTYPE_PROBE_RESP, "proberesp" },
	{ IEEE80211_FC0_SUBTYPE_PROBE_RESP, "probe-resp" },
	{ IEEE80211_FC0_SUBTYPE_BEACON, "beacon" },
	{ IEEE80211_FC0_SUBTYPE_ATIM, "atim" },
	{ IEEE80211_FC0_SUBTYPE_DISASSOC, "disassoc" },
	{ IEEE80211_FC0_SUBTYPE_DISASSOC, "disassociation" },
	{ IEEE80211_FC0_SUBTYPE_AUTH, "auth" },
	{ IEEE80211_FC0_SUBTYPE_AUTH, "authentication" },
	{ IEEE80211_FC0_SUBTYPE_DEAUTH, "deauth" },
	{ IEEE80211_FC0_SUBTYPE_DEAUTH, "deauthentication" },
	{ 0, NULL }
};
static const struct tok ieee80211_ctl_subtypes[] = {
	{ IEEE80211_FC0_SUBTYPE_PS_POLL, "ps-poll" },
	{ IEEE80211_FC0_SUBTYPE_RTS, "rts" },
	{ IEEE80211_FC0_SUBTYPE_CTS, "cts" },
	{ IEEE80211_FC0_SUBTYPE_ACK, "ack" },
	{ IEEE80211_FC0_SUBTYPE_CF_END, "cf-end" },
	{ IEEE80211_FC0_SUBTYPE_CF_END_ACK, "cf-end-ack" },
	{ 0, NULL }
};
static const struct tok ieee80211_data_subtypes[] = {
	{ IEEE80211_FC0_SUBTYPE_DATA, "data" },
	{ IEEE80211_FC0_SUBTYPE_CF_ACK, "data-cf-ack" },
	{ IEEE80211_FC0_SUBTYPE_CF_POLL, "data-cf-poll" },
	{ IEEE80211_FC0_SUBTYPE_CF_ACPL, "data-cf-ack-poll" },
	{ IEEE80211_FC0_SUBTYPE_NODATA, "null" },
	{ IEEE80211_FC0_SUBTYPE_NODATA_CF_ACK, "cf-ack" },
	{ IEEE80211_FC0_SUBTYPE_NODATA_CF_POLL, "cf-poll"  },
	{ IEEE80211_FC0_SUBTYPE_NODATA_CF_ACPL, "cf-ack-poll" },
	{ IEEE80211_FC0_SUBTYPE_QOS|IEEE80211_FC0_SUBTYPE_DATA, "qos-data" },
	{ IEEE80211_FC0_SUBTYPE_QOS|IEEE80211_FC0_SUBTYPE_CF_ACK, "qos-data-cf-ack" },
	{ IEEE80211_FC0_SUBTYPE_QOS|IEEE80211_FC0_SUBTYPE_CF_POLL, "qos-data-cf-poll" },
	{ IEEE80211_FC0_SUBTYPE_QOS|IEEE80211_FC0_SUBTYPE_CF_ACPL, "qos-data-cf-ack-poll" },
	{ IEEE80211_FC0_SUBTYPE_QOS|IEEE80211_FC0_SUBTYPE_NODATA, "qos" },
	{ IEEE80211_FC0_SUBTYPE_QOS|IEEE80211_FC0_SUBTYPE_NODATA_CF_POLL, "qos-cf-poll" },
	{ IEEE80211_FC0_SUBTYPE_QOS|IEEE80211_FC0_SUBTYPE_NODATA_CF_ACPL, "qos-cf-ack-poll" },
	{ 0, NULL }
};
static const struct tok llc_s_subtypes[] = {
	{ LLC_RR, "rr" },
	{ LLC_RNR, "rnr" },
	{ LLC_REJ, "rej" },
	{ 0, NULL }
};
static const struct tok llc_u_subtypes[] = {
	{ LLC_UI, "ui" },
	{ LLC_UA, "ua" },
	{ LLC_DISC, "disc" },
	{ LLC_DM, "dm" },
	{ LLC_SABME, "sabme" },
	{ LLC_TEST, "test" },
	{ LLC_XID, "xid" },
	{ LLC_FRMR, "frmr" },
	{ 0, NULL }
};
struct type2tok {
	int type;
	const struct tok *tok;
};
static const struct type2tok ieee80211_type_subtypes[] = {
	{ IEEE80211_FC0_TYPE_MGT, ieee80211_mgt_subtypes },
	{ IEEE80211_FC0_TYPE_CTL, ieee80211_ctl_subtypes },
	{ IEEE80211_FC0_TYPE_DATA, ieee80211_data_subtypes },
	{ 0, NULL }
};

static int
str2tok(const char *str, const struct tok *toks)
{
	int i;

	for (i = 0; toks[i].s != NULL; i++) {
		if (pcap_strcasecmp(toks[i].s, str) == 0)
			return (toks[i].v);
	}
	return (-1);
}

static const struct qual qerr = { Q_UNDEF, Q_UNDEF, Q_UNDEF, Q_UNDEF };

static void
yyerror(void *yyscanner _U_, compiler_state_t *cstate, const char *msg)
{
	bpf_set_error(cstate, "can't parse filter expression: %s", msg);
}

#ifdef HAVE_NET_PFVAR_H
static int
pfreason_to_num(compiler_state_t *cstate, const char *reason)
{
	const char *reasons[] = PFRES_NAMES;
	int i;

	for (i = 0; reasons[i]; i++) {
		if (pcap_strcasecmp(reason, reasons[i]) == 0)
			return (i);
	}
	bpf_set_error(cstate, "unknown PF reason");
	return (-1);
}

static int
pfaction_to_num(compiler_state_t *cstate, const char *action)
{
	if (pcap_strcasecmp(action, "pass") == 0 ||
	    pcap_strcasecmp(action, "accept") == 0)
		return (PF_PASS);
	else if (pcap_strcasecmp(action, "drop") == 0 ||
		pcap_strcasecmp(action, "block") == 0)
		return (PF_DROP);
#if HAVE_PF_NAT_THROUGH_PF_NORDR
	else if (pcap_strcasecmp(action, "rdr") == 0)
		return (PF_RDR);
	else if (pcap_strcasecmp(action, "nat") == 0)
		return (PF_NAT);
	else if (pcap_strcasecmp(action, "binat") == 0)
		return (PF_BINAT);
	else if (pcap_strcasecmp(action, "nordr") == 0)
		return (PF_NORDR);
#endif
	else {
		bpf_set_error(cstate, "unknown PF action");
		return (-1);
	}
}
#else /* !HAVE_NET_PFVAR_H */
static int
pfreason_to_num(compiler_state_t *cstate, const char *reason _U_)
{
	bpf_set_error(cstate, "libpcap was compiled on a machine without pf support");
	return (-1);
}

static int
pfaction_to_num(compiler_state_t *cstate, const char *action _U_)
{
	bpf_set_error(cstate, "libpcap was compiled on a machine without pf support");
	return (-1);
}
#endif /* HAVE_NET_PFVAR_H */

/*
 * For calls that might return an "an error occurred" value.
 */
#define CHECK_INT_VAL(val)	if (val == -1) YYABORT
#define CHECK_PTR_VAL(val)	if (val == NULL) YYABORT

DIAG_OFF_BISON_BYACC
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#line 291 "../grammar.y"
typedef union YYSTYPE {
	int i;
	bpf_u_int32 h;
	char *s;
	struct stmt *stmt;
	struct arth *a;
	struct {
		struct qual q;
		int atmfieldtype;
		int mtp3fieldtype;
		struct block *b;
	} blk;
	struct block *rblk;
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
#line 406 "grammar.c"

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
# ifdef YYPARSE_PARAM_TYPE
#  define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
# else
#  define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
# endif
#else
# define YYPARSE_DECL() yyparse(void *yyscanner, compiler_state_t *cstate)
#endif

/* Parameters sent to lex. */
#ifdef YYLEX_PARAM
# ifdef YYLEX_PARAM_TYPE
#  define YYLEX_DECL() yylex(YYSTYPE *yylval, YYLEX_PARAM_TYPE YYLEX_PARAM)
# else
#  define YYLEX_DECL() yylex(YYSTYPE *yylval, void * YYLEX_PARAM)
# endif
# define YYLEX yylex(&yylval, YYLEX_PARAM)
#else
# define YYLEX_DECL() yylex(YYSTYPE *yylval, void *yyscanner)
# define YYLEX yylex(&yylval, yyscanner)
#endif

/* Parameters sent to yyerror. */
#ifndef YYERROR_DECL
#define YYERROR_DECL() yyerror(void *yyscanner, compiler_state_t *cstate, const char *s)
#endif
#ifndef YYERROR_CALL
#define YYERROR_CALL(msg) yyerror(yyscanner, cstate, msg)
#endif

extern int YYPARSE_DECL();

#define DST 257
#define SRC 258
#define HOST 259
#define GATEWAY 260
#define NET 261
#define NETMASK 262
#define PORT 263
#define PORTRANGE 264
#define LESS 265
#define GREATER 266
#define PROTO 267
#define PROTOCHAIN 268
#define CBYTE 269
#define ARP 270
#define RARP 271
#define IP 272
#define SCTP 273
#define TCP 274
#define UDP 275
#define ICMP 276
#define IGMP 277
#define IGRP 278
#define PIM 279
#define VRRP 280
#define CARP 281
#define ATALK 282
#define AARP 283
#define DECNET 284
#define LAT 285
#define SCA 286
#define MOPRC 287
#define MOPDL 288
#define TK_BROADCAST 289
#define TK_MULTICAST 290
#define NUM 291
#define INBOUND 292
#define OUTBOUND 293
#define PF_IFNAME 294
#define PF_RSET 295
#define PF_RNR 296
#define PF_SRNR 297
#define PF_REASON 298
#define PF_ACTION 299
#define TYPE 300
#define SUBTYPE 301
#define DIR 302
#define ADDR1 303
#define ADDR2 304
#define ADDR3 305
#define ADDR4 306
#define RA 307
#define TA 308
#define LINK 309
#define GEQ 310
#define LEQ 311
#define NEQ 312
#define ID 313
#define EID 314
#define HID 315
#define HID6 316
#define AID 317
#define LSH 318
#define RSH 319
#define LEN 320
#define IPV6 321
#define ICMPV6 322
#define AH 323
#define ESP 324
#define VLAN 325
#define MPLS 326
#define PPPOED 327
#define PPPOES 328
#define GENEVE 329
#define ISO 330
#define ESIS 331
#define CLNP 332
#define ISIS 333
#define L1 334
#define L2 335
#define IIH 336
#define LSP 337
#define SNP 338
#define CSNP 339
#define PSNP 340
#define STP 341
#define IPX 342
#define NETBEUI 343
#define LANE 344
#define LLC 345
#define METAC 346
#define BCC 347
#define SC 348
#define ILMIC 349
#define OAMF4EC 350
#define OAMF4SC 351
#define OAM 352
#define OAMF4 353
#define CONNECTMSG 354
#define METACONNECT 355
#define VPI 356
#define VCI 357
#define RADIO 358
#define FISU 359
#define LSSU 360
#define MSU 361
#define HFISU 362
#define HLSSU 363
#define HMSU 364
#define SIO 365
#define OPC 366
#define DPC 367
#define SLS 368
#define HSIO 369
#define HOPC 370
#define HDPC 371
#define HSLS 372
#define LEX_ERROR 373
#define OR 374
#define AND 375
#define UMINUS 376
#define YYERRCODE 256
typedef int YYINT;
static const YYINT pcap_lhs[] = {                        -1,
    0,    0,   24,    1,    1,    1,    1,    1,   20,   21,
    2,    2,    2,    3,    3,    3,    3,    3,    3,    3,
    3,    3,   23,   22,    4,    4,    4,    7,    7,    5,
    5,    8,    8,    8,    8,    8,    8,    6,    6,    6,
    6,    6,    6,    6,    6,    6,    6,    6,    9,    9,
   10,   10,   10,   10,   10,   10,   10,   10,   10,   10,
   10,   10,   11,   11,   11,   11,   12,   16,   16,   16,
   16,   16,   16,   16,   16,   16,   16,   16,   16,   16,
   16,   16,   16,   16,   16,   16,   16,   16,   16,   16,
   16,   16,   16,   16,   16,   16,   16,   16,   16,   16,
   16,   16,   16,   16,   16,   16,   25,   25,   25,   25,
   25,   25,   25,   25,   25,   25,   25,   25,   25,   25,
   25,   25,   25,   25,   25,   26,   26,   26,   26,   26,
   26,   27,   27,   27,   27,   42,   42,   43,   43,   44,
   28,   28,   28,   45,   45,   41,   41,   40,   18,   18,
   18,   19,   19,   19,   13,   13,   14,   14,   14,   14,
   14,   14,   14,   14,   14,   14,   14,   14,   14,   14,
   14,   15,   15,   15,   15,   15,   17,   17,   29,   29,
   29,   29,   29,   29,   29,   30,   30,   30,   30,   31,
   31,   33,   33,   33,   33,   32,   34,   34,   35,   35,
   35,   35,   35,   35,   36,   36,   36,   36,   36,   36,
   36,   36,   38,   38,   38,   38,   37,   39,   39,
};
static const YYINT pcap_len[] = {                         2,
    2,    1,    0,    1,    3,    3,    3,    3,    1,    1,
    1,    1,    3,    1,    3,    3,    1,    3,    1,    1,
    1,    2,    1,    1,    1,    3,    3,    1,    1,    1,
    2,    3,    2,    2,    2,    2,    2,    2,    3,    1,
    3,    3,    1,    1,    1,    2,    1,    2,    1,    0,
    1,    1,    3,    3,    3,    3,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    2,    2,    2,    2,
    4,    1,    1,    2,    1,    2,    1,    1,    2,    1,
    2,    1,    1,    2,    1,    2,    2,    2,    2,    2,
    2,    4,    2,    2,    2,    1,    1,    1,    1,    1,
    1,    2,    2,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    4,    6,    3,    3,
    3,    3,    3,    3,    3,    3,    3,    3,    2,    3,
    1,    1,    1,    1,    1,    1,    1,    3,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    2,    2,    3,    1,    1,    3,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    2,    2,    3,    1,    1,    3,
};
static const YYINT pcap_defred[] = {                      3,
    0,    0,    0,    0,    0,   70,   71,   69,   72,   73,
   74,   75,   76,   77,   78,   79,   80,   81,   82,   83,
   84,   85,   87,   86,  177,  112,  113,    0,    0,    0,
    0,    0,    0,   68,  171,   88,   89,   90,   91,    0,
    0,  118,    0,    0,   92,   93,  102,   94,   95,   96,
   97,   98,   99,  101,  100,  103,  104,  105,  179,    0,
  180,  181,  184,  185,  182,  183,  186,  187,  188,  189,
  190,  191,  106,  199,  200,  201,  202,  203,  204,  205,
  206,  207,  208,  209,  210,  211,  212,   23,    0,   24,
    0,    4,   30,    0,    0,    0,  156,    0,  155,    0,
    0,   43,  123,  125,   44,   45,    0,   47,    0,  109,
  110,    0,  126,  127,  128,  129,  146,  147,  130,  148,
  131,  114,    0,  116,  119,  121,  143,  142,    0,    0,
    0,   10,    9,    0,    0,   14,   20,    0,    0,   21,
   38,   11,   12,    0,    0,    0,    0,   63,   67,   64,
   65,   66,   35,   36,  107,  108,    0,    0,    0,   57,
   58,   59,   60,   61,   62,    0,   34,   37,  124,  150,
  152,  154,    0,    0,    0,    0,    0,    0,    0,    0,
  149,  151,  153,    0,    0,    0,    0,    0,    0,    0,
    0,   31,  196,    0,    0,    0,  192,   46,  217,    0,
    0,    0,  213,   48,  173,  172,  175,  176,  174,    0,
    0,    0,    6,    5,    0,    0,    0,    8,    7,    0,
    0,    0,   25,    0,    0,    0,   22,    0,    0,    0,
    0,  136,  137,    0,  140,  134,  144,  145,  135,   32,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   39,  170,  178,  193,  194,  197,    0,
  214,  215,  218,    0,  111,    0,   16,   15,   18,   13,
    0,    0,   54,   56,   53,   55,    0,  157,    0,  195,
    0,  216,    0,   26,   27,  138,  139,  132,    0,  198,
  219,  158,
};
#if defined(YYDESTRUCT_CALL) || defined(YYSTYPE_TOSTRING)
static const YYINT pcap_stos[] = {                        0,
  378,  402,  265,  266,  269,  270,  271,  272,  273,  274,
  275,  276,  277,  278,  279,  280,  281,  282,  283,  284,
  285,  286,  287,  288,  291,  292,  293,  294,  295,  296,
  297,  298,  299,  309,  320,  321,  322,  323,  324,  325,
  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,
  336,  337,  338,  339,  340,  341,  342,  343,  344,  345,
  346,  347,  348,  349,  350,  351,  352,  353,  354,  355,
  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,
  366,  367,  368,  369,  370,  371,  372,   33,   45,   40,
  379,  383,  384,  386,  387,  391,  392,  394,  395,  400,
  401,  403,  404,  406,  407,  408,  409,  413,  414,  291,
  291,  291,  313,  313,  291,  291,  291,  313,  419,  313,
  418,  395,  400,  395,  395,  395,  296,  313,  391,  394,
  400,  374,  375,  398,  399,  313,  314,  315,  316,  317,
  380,  381,  395,  400,  401,  257,  258,  259,  260,  261,
  263,  264,  267,  268,  289,  290,  300,  301,  302,  303,
  304,  305,  306,  307,  308,  388,  389,  390,  405,  310,
  311,  312,  318,  319,  124,   38,   43,   45,   42,   47,
   62,   61,   60,   37,   94,  396,  397,   91,  379,  392,
  395,  383,  291,  396,  397,  400,  410,  411,  291,  396,
  397,  400,  415,  416,  124,   38,   62,   61,   60,  393,
  395,  391,  380,  383,  395,  400,  401,  380,  383,  262,
   47,   47,  381,  382,  385,  395,  380,  374,  375,  374,
  375,  291,  313,  420,  313,  422,  291,  313,  423,  389,
  391,  391,  391,  391,  391,  391,  391,  391,  391,  391,
  391,  391,  391,   41,   41,   41,  291,  291,  410,  412,
  291,  291,  415,  417,  291,  395,  315,  291,  291,   41,
  398,  399,  258,  258,  257,  257,  301,   93,   58,   41,
  399,   41,  399,  380,  380,  291,  313,  421,  291,  410,
  415,   93,
};
#endif /* YYDESTRUCT_CALL || YYSTYPE_TOSTRING */
static const YYINT pcap_dgoto[] = {                       1,
  189,  227,  142,  224,   92,   93,  225,   94,   95,  166,
  167,  168,   96,   97,  210,  130,   99,  186,  187,  134,
  135,  131,  145,    2,  102,  103,  169,  104,  105,  106,
  107,  197,  198,  260,  108,  109,  203,  204,  264,  121,
  119,  234,  288,  236,  239,
};
static const YYINT pcap_sindex[] = {                      0,
    0,  432, -273, -265, -254,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0, -262, -229, -183,
 -169, -284, -209,    0,    0,    0,    0,    0,    0,  -24,
  -24,    0,  -24,  -24,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -286,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  654,    0,
 -249,    0,    0,  -21,  896,  856,    0,   14,    0,  432,
  432,    0,    0,    0,    0,    0,  140,    0,  237,    0,
    0,  -27,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  -24,    0,    0,    0,    0,    0,  -14,   14,
  654,    0,    0,  320,  320,    0,    0,  -47,   81,    0,
    0,    0,    0,  -21,  -21, -186, -170,    0,    0,    0,
    0,    0,    0,    0,    0,    0, -274, -182, -270,    0,
    0,    0,    0,    0,    0,  -86,    0,    0,    0,    0,
    0,    0,  654,  654,  654,  654,  654,  654,  654,  654,
    0,    0,    0,  654,  654,  654,  654,  654,  -39,   97,
  100,    0,    0, -148, -146, -141,    0,    0,    0, -125,
 -122, -117,    0,    0,    0,    0,    0,    0,    0, -115,
  100,  990,    0,    0,    0,  320,  320,    0,    0, -159,
 -112, -110,    0,  150, -249,  100,    0,  -65,  -63,  -51,
  -49,    0,    0,  -87,    0,    0,    0,    0,    0,    0,
  484,  484,  184,  530,  191,  191,  -14,  -14,  990,  990,
  990,  990,  973,    0,    0,    0,    0,    0,    0,  -37,
    0,    0,    0,  -36,    0,  100,    0,    0,    0,    0,
  -21,  -21,    0,    0,    0,    0, -184,    0,  -66,    0,
 -141,    0, -117,    0,    0,    0,    0,    0,  137,    0,
    0,    0,
};
static const YYINT pcap_rindex[] = {                      0,
    0,  574,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    6,
    8,    0,   13,   25,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   28,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  232,    0,    0,    0,    0,    0,    0,    1,    0,  993,
  993,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    3,    0,
    0,    0,    0,  993,  993,    0,    0,   32,   38,    0,
    0,    0,    0,    0,    0,  248,  592,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   53,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  906,
  961,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   15,  993,  993,    0,    0,    0,
    0,    0,    0, -138,    0, -135,    0,    0,    0,    0,
    0,    0,    0,   70,    0,    0,    0,    0,    0,    0,
   30,  124,  158,  149,   99,  110,   40,   74,  183,  194,
   72,   89,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  768,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,
};
#if YYBTYACC
static const YYINT pcap_cindex[] = {                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,
};
#endif
static const YYINT pcap_gindex[] = {                      0,
  247,   12, -113,    0,   62,    0,    0,    0,    0,    0,
   84,    0, 1003,  -64,    0,  112, 1004,  -85,   11,   41,
 -205, 1017,  354,    0,    0,    0,    0,    0,    0,    0,
    0, -187,    0,    0,    0,    0, -188,    0,    0,    0,
    0,    0,    0,    0,    0,
};
#define YYTABLESIZE 1309
static const YYINT pcap_table[] = {                     221,
   40,  254,  169,  280,  282,  115,  117,  117,  259,  127,
  206,   88,  120,  263,   12,   90,  232,  110,   90,  272,
  237,  194,  184,  200,  122,  111,  128,  141,  118,  167,
  223,   17,  209,  208,  207,  190,  112,   19,  233,  161,
  169,   40,  238,  169,  169,  169,  115,  169,  117,  169,
  113,  155,  155,  120,  281,   12,  155,  155,  283,  155,
  169,  155,  169,  169,  169,  122,  190,  167,  141,  133,
  167,   41,   17,  162,  155,  155,  155,  161,   19,  185,
  161,  161,  161,  114,  161,   33,  161,  167,   42,  167,
  167,  167,   33,  290,  291,  169,  205,  161,  159,  161,
  161,  161,  223,  120,  188,  141,  286,  115,  155,  160,
  133,  162,   41,   98,  162,  162,  162,  195,  162,  201,
  162,  116,  167,  168,  132,  133,  169,  222,  287,   42,
  235,  162,  161,  162,  162,  162,  159,  255,  155,  159,
  256,  159,  257,  159,  258,  213,  218,  160,  164,  193,
  160,  190,  160,  167,  160,  267,  159,  165,  159,  159,
  159,  168,  192,  161,  168,  261,  162,  160,  262,  160,
  160,  160,  148,  199,  150,  265,  151,  152,  268,   90,
  269,  168,  163,  168,  168,  168,  164,  228,  229,  164,
  270,  159,  273,  166,  274,  214,  219,  162,  165,  183,
  182,  181,  160,  230,  231,  275,  164,  276,  164,  164,
  164,   98,   98,  277,  220,  165,  168,  165,  165,  165,
  184,  176,  159,  163,  289,  179,  177,  184,  178,  292,
  180,    1,  179,  160,  166,   29,   29,  180,   28,   28,
  163,  164,  163,  163,  163,   98,   98,  168,   91,  240,
  165,  166,    0,  166,  166,  166,    0,   49,   49,   49,
   49,   49,    0,   49,   49,  271,   25,   49,   49,   25,
    0,    0,  164,    0,    0,  163,   90,  185,  192,    0,
   52,  165,  284,  285,  185,    0,  166,   52,    0,   49,
   49,  136,  137,  138,  139,  140,  183,  182,  181,    0,
   49,   49,   49,   49,   49,   49,   49,   49,   49,    0,
    0,    0,  169,  169,  169,    0,    0,    0,    0,    0,
  169,  169,    0,    0,  155,  155,  155,   98,   98,    0,
    0,    0,  155,  155,  132,  133,  132,  132,    0,  167,
  167,  167,    0,   33,    0,    0,    0,  167,  167,  161,
  161,  161,   88,    0,    0,  101,    0,  161,  161,   90,
    0,    0,    0,    0,   89,   33,   33,   33,   33,   33,
    0,    0,    0,    0,   40,   40,  169,  169,    0,  115,
  115,  117,  117,  162,  162,  162,  120,  120,   12,   12,
    0,  162,  162,    0,    0,    0,    0,    0,  122,  122,
    0,  141,  141,  167,  167,   17,   17,    0,  159,  159,
  159,   19,   19,  161,  161,    0,  159,  159,    0,  160,
  160,  160,    0,    0,    0,    0,    0,  160,  160,    0,
  193,    0,    0,  168,  168,  168,    0,    0,    0,    0,
    0,  168,  168,  133,  133,   41,   41,  162,  162,  170,
  171,  172,    0,  101,  101,    0,    0,    0,  164,  164,
  164,    0,   42,   42,   88,    0,    0,  165,  165,  165,
    0,   90,  159,  159,    0,    0,   89,    0,    0,    0,
    0,    0,    0,  160,  160,    0,    0,  217,  217,    0,
    0,    0,  163,  163,  163,    0,    0,  168,  168,    0,
    0,  173,  174,  166,  166,  166,   52,    0,   52,    0,
   52,   52,    0,    0,    0,    0,    0,    0,    0,    0,
  184,    0,  164,  164,    0,  179,  177,  199,  178,    0,
  180,  165,  165,    0,    0,    0,    0,    0,   52,    0,
    0,    0,    0,    0,    0,    0,  170,  171,  172,    0,
    0,    0,    0,    0,    0,    0,  163,  163,    0,    0,
   52,   52,   52,   52,   52,    0,  184,  166,  166,  217,
  217,  179,  177,    2,  178,    0,  180,  185,    0,    0,
    0,    0,    0,    0,    3,    4,    0,    0,    5,    6,
    7,    8,    9,   10,   11,   12,   13,   14,   15,   16,
   17,   18,   19,   20,   21,   22,   23,   24,    0,    0,
   25,   26,   27,   28,   29,   30,   31,   32,   33,    0,
    0,    0,    0,  185,   51,    0,    0,    0,   34,    0,
    0,   51,  136,  137,  138,  139,  140,    0,    0,   35,
   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,
   46,   47,   48,   49,   50,   51,   52,   53,   54,   55,
   56,   57,   58,   59,   60,   61,   62,   63,   64,   65,
   66,   67,   68,   69,   70,   71,   72,   73,   74,   75,
   76,   77,   78,   79,   80,   81,   82,   83,   84,   85,
   86,   87,    0,   90,    0,    0,    3,    4,   89,    0,
    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
   15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
    0,    0,   25,   26,   27,   28,   29,   30,   31,   32,
   33,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   34,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   35,   36,   37,   38,   39,   40,   41,   42,   43,
   44,   45,   46,   47,   48,   49,   50,   51,   52,   53,
   54,   55,   56,   57,   58,   59,   60,   61,   62,   63,
   64,   65,   66,   67,   68,   69,   70,   71,   72,   73,
   74,   75,   76,   77,   78,   79,   80,   81,   82,   83,
   84,   85,   86,   87,  155,  155,    0,    0,    0,  155,
  155,    0,  155,    0,  155,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  155,  155,  155,
   50,   50,   50,   50,   50,    0,   50,   50,    0,    0,
   50,   50,    0,    0,    0,    0,    0,  173,  174,    0,
   51,    0,   51,    0,   51,   51,    0,    0,    0,    0,
    0,  155,   50,   50,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   50,   50,   50,   50,   50,   50,   50,
   50,   50,   51,    0,    0,    0,    0,    0,    0,    0,
    0,  155,  184,  176,    0,    0,    0,  179,  177,    0,
  178,    0,  180,    0,   51,   51,   51,   51,   51,    0,
    0,    0,    0,    0,    0,  183,  182,  181,    0,    0,
    0,    0,    0,    6,    7,    8,    9,   10,   11,   12,
   13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
   23,   24,  156,  156,   25,    0,    0,  156,  156,  185,
  156,    0,  156,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   34,    0,    0,  156,  156,  156,    0,    0,
    0,    0,    0,   35,   36,   37,   38,   39,    0,  175,
    0,    0,    0,   45,   46,   47,   48,   49,   50,   51,
   52,   53,   54,   55,   56,   57,   58,  155,  155,  156,
    0,    0,  155,  155,    0,  155,    0,  155,    0,  184,
  176,   73,    0,    0,  179,  177,    0,  178,  100,  180,
  155,  155,  155,    0,    0,    0,  184,  176,    0,  156,
  279,  179,  177,    0,  178,    0,  180,    0,    0,    0,
    0,    0,    0,  122,  124,    0,  125,  126,    0,    0,
    0,    0,    0,    0,  155,    0,  123,  123,    0,  123,
  123,    0,    0,    0,    0,  278,  185,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  155,  155,  155,
    0,    0,    0,  185,  155,  155,  155,    0,    0,    0,
    0,  129,    0,    0,    0,    0,  175,  143,    0,    0,
    0,    0,    0,  191,    0,    0,    0,    0,    0,    0,
  144,    0,    0,  175,    0,    0,  100,  100,    0,    0,
    0,    0,    0,  196,    0,  202,  211,    0,    0,    0,
    0,    0,    0,  212,  191,    0,    0,  215,  215,  123,
    0,   28,   28,    0,    0,    0,    0,  226,  143,    0,
  216,  216,  146,  147,  148,  149,  150,    0,  151,  152,
  123,  144,  153,  154,    0,  170,  171,  172,    0,    0,
    0,    0,    0,  173,  174,  241,  242,  243,  244,  245,
  246,  247,  248,    0,  155,  156,  249,  250,  251,  252,
  253,    0,    0,    0,    0,  157,  158,  159,  160,  161,
  162,  163,  164,  165,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  156,  156,  156,    0,  266,
  215,    0,    0,  156,  156,    0,    0,    0,    0,    0,
    0,    0,  100,  216,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   50,
   50,   50,   50,   50,    0,   50,   50,    0,    0,   50,
   50,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  155,  155,  155,    0,  143,  143,    0,    0,  155,  155,
    0,   50,   50,    0,    0,    0,    0,  144,  144,    0,
  173,  174,   50,   50,   50,   50,   50,   50,   50,   50,
   50,    0,    0,    0,    0,    0,    0,  173,  174,
};
static const YYINT pcap_check[] = {                      47,
    0,   41,    0,   41,   41,    0,  291,    0,  196,  296,
   38,   33,    0,  202,    0,   40,  291,  291,   40,  225,
  291,  107,   37,  109,    0,  291,  313,    0,  313,    0,
  144,    0,   60,   61,   62,  100,  291,    0,  313,    0,
   38,   41,  313,   41,   42,   43,   41,   45,   41,   47,
  313,   37,   38,   41,  260,   41,   42,   43,  264,   45,
   58,   47,   60,   61,   62,   41,  131,   38,   41,    0,
   41,    0,   41,    0,   60,   61,   62,   38,   41,   94,
   41,   42,   43,  313,   45,   33,   47,   58,    0,   60,
   61,   62,   40,  281,  283,   93,  124,   58,    0,   60,
   61,   62,  216,  313,   91,   94,  291,  291,   94,    0,
   41,   38,   41,    2,   41,   42,   43,  107,   45,  109,
   47,  291,   93,    0,  374,  375,  124,   47,  313,   41,
  313,   58,   93,   60,   61,   62,   38,   41,  124,   41,
   41,   43,  291,   45,  291,  134,  135,   38,    0,  291,
   41,  216,   43,  124,   45,  315,   58,    0,   60,   61,
   62,   38,  101,  124,   41,  291,   93,   58,  291,   60,
   61,   62,  259,  291,  261,  291,  263,  264,  291,   40,
  291,   58,    0,   60,   61,   62,   38,  374,  375,   41,
   41,   93,  258,    0,  258,  134,  135,  124,   41,   60,
   61,   62,   93,  374,  375,  257,   58,  257,   60,   61,
   62,  100,  101,  301,  262,   58,   93,   60,   61,   62,
   37,   38,  124,   41,  291,   42,   43,   37,   45,   93,
   47,    0,   42,  124,   41,  374,  375,   47,  374,  375,
   58,   93,   60,   61,   62,  134,  135,  124,    2,  166,
   93,   58,   -1,   60,   61,   62,   -1,  257,  258,  259,
  260,  261,   -1,  263,  264,  225,  291,  267,  268,  291,
   -1,   -1,  124,   -1,   -1,   93,   40,   94,  217,   -1,
   33,  124,  271,  272,   94,   -1,   93,   40,   -1,  289,
  290,  313,  314,  315,  316,  317,   60,   61,   62,   -1,
  300,  301,  302,  303,  304,  305,  306,  307,  308,   -1,
   -1,   -1,  310,  311,  312,   -1,   -1,   -1,   -1,   -1,
  318,  319,   -1,   -1,  310,  311,  312,  216,  217,   -1,
   -1,   -1,  318,  319,  374,  375,  374,  374,   -1,  310,
  311,  312,   -1,  291,   -1,   -1,   -1,  318,  319,  310,
  311,  312,   33,   -1,   -1,    2,   -1,  318,  319,   40,
   -1,   -1,   -1,   -1,   45,  313,  314,  315,  316,  317,
   -1,   -1,   -1,   -1,  374,  375,  374,  375,   -1,  374,
  375,  374,  375,  310,  311,  312,  374,  375,  374,  375,
   -1,  318,  319,   -1,   -1,   -1,   -1,   -1,  374,  375,
   -1,  374,  375,  374,  375,  374,  375,   -1,  310,  311,
  312,  374,  375,  374,  375,   -1,  318,  319,   -1,  310,
  311,  312,   -1,   -1,   -1,   -1,   -1,  318,  319,   -1,
  291,   -1,   -1,  310,  311,  312,   -1,   -1,   -1,   -1,
   -1,  318,  319,  374,  375,  374,  375,  374,  375,  310,
  311,  312,   -1,  100,  101,   -1,   -1,   -1,  310,  311,
  312,   -1,  374,  375,   33,   -1,   -1,  310,  311,  312,
   -1,   40,  374,  375,   -1,   -1,   45,   -1,   -1,   -1,
   -1,   -1,   -1,  374,  375,   -1,   -1,  134,  135,   -1,
   -1,   -1,  310,  311,  312,   -1,   -1,  374,  375,   -1,
   -1,  318,  319,  310,  311,  312,  259,   -1,  261,   -1,
  263,  264,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   37,   -1,  374,  375,   -1,   42,   43,  291,   45,   -1,
   47,  374,  375,   -1,   -1,   -1,   -1,   -1,  291,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  310,  311,  312,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  374,  375,   -1,   -1,
  313,  314,  315,  316,  317,   -1,   37,  374,  375,  216,
  217,   42,   43,    0,   45,   -1,   47,   94,   -1,   -1,
   -1,   -1,   -1,   -1,  265,  266,   -1,   -1,  269,  270,
  271,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  284,  285,  286,  287,  288,   -1,   -1,
  291,  292,  293,  294,  295,  296,  297,  298,  299,   -1,
   -1,   -1,   -1,   94,   33,   -1,   -1,   -1,  309,   -1,
   -1,   40,  313,  314,  315,  316,  317,   -1,   -1,  320,
  321,  322,  323,  324,  325,  326,  327,  328,  329,  330,
  331,  332,  333,  334,  335,  336,  337,  338,  339,  340,
  341,  342,  343,  344,  345,  346,  347,  348,  349,  350,
  351,  352,  353,  354,  355,  356,  357,  358,  359,  360,
  361,  362,  363,  364,  365,  366,  367,  368,  369,  370,
  371,  372,   -1,   40,   -1,   -1,  265,  266,   45,   -1,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  281,  282,  283,  284,  285,  286,  287,  288,
   -1,   -1,  291,  292,  293,  294,  295,  296,  297,  298,
  299,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  309,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  320,  321,  322,  323,  324,  325,  326,  327,  328,
  329,  330,  331,  332,  333,  334,  335,  336,  337,  338,
  339,  340,  341,  342,  343,  344,  345,  346,  347,  348,
  349,  350,  351,  352,  353,  354,  355,  356,  357,  358,
  359,  360,  361,  362,  363,  364,  365,  366,  367,  368,
  369,  370,  371,  372,   37,   38,   -1,   -1,   -1,   42,
   43,   -1,   45,   -1,   47,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   60,   61,   62,
  257,  258,  259,  260,  261,   -1,  263,  264,   -1,   -1,
  267,  268,   -1,   -1,   -1,   -1,   -1,  318,  319,   -1,
  259,   -1,  261,   -1,  263,  264,   -1,   -1,   -1,   -1,
   -1,   94,  289,  290,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  300,  301,  302,  303,  304,  305,  306,
  307,  308,  291,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  124,   37,   38,   -1,   -1,   -1,   42,   43,   -1,
   45,   -1,   47,   -1,  313,  314,  315,  316,  317,   -1,
   -1,   -1,   -1,   -1,   -1,   60,   61,   62,   -1,   -1,
   -1,   -1,   -1,  270,  271,  272,  273,  274,  275,  276,
  277,  278,  279,  280,  281,  282,  283,  284,  285,  286,
  287,  288,   37,   38,  291,   -1,   -1,   42,   43,   94,
   45,   -1,   47,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  309,   -1,   -1,   60,   61,   62,   -1,   -1,
   -1,   -1,   -1,  320,  321,  322,  323,  324,   -1,  124,
   -1,   -1,   -1,  330,  331,  332,  333,  334,  335,  336,
  337,  338,  339,  340,  341,  342,  343,   37,   38,   94,
   -1,   -1,   42,   43,   -1,   45,   -1,   47,   -1,   37,
   38,  358,   -1,   -1,   42,   43,   -1,   45,    2,   47,
   60,   61,   62,   -1,   -1,   -1,   37,   38,   -1,  124,
   58,   42,   43,   -1,   45,   -1,   47,   -1,   -1,   -1,
   -1,   -1,   -1,   40,   41,   -1,   43,   44,   -1,   -1,
   -1,   -1,   -1,   -1,   94,   -1,   40,   41,   -1,   43,
   44,   -1,   -1,   -1,   -1,   93,   94,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  310,  311,  312,
   -1,   -1,   -1,   94,  124,  318,  319,   -1,   -1,   -1,
   -1,   89,   -1,   -1,   -1,   -1,  124,   94,   -1,   -1,
   -1,   -1,   -1,  100,   -1,   -1,   -1,   -1,   -1,   -1,
   94,   -1,   -1,  124,   -1,   -1,  100,  101,   -1,   -1,
   -1,   -1,   -1,  107,   -1,  109,  123,   -1,   -1,   -1,
   -1,   -1,   -1,  131,  131,   -1,   -1,  134,  135,  123,
   -1,  374,  375,   -1,   -1,   -1,   -1,  144,  145,   -1,
  134,  135,  257,  258,  259,  260,  261,   -1,  263,  264,
  144,  145,  267,  268,   -1,  310,  311,  312,   -1,   -1,
   -1,   -1,   -1,  318,  319,  173,  174,  175,  176,  177,
  178,  179,  180,   -1,  289,  290,  184,  185,  186,  187,
  188,   -1,   -1,   -1,   -1,  300,  301,  302,  303,  304,
  305,  306,  307,  308,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  310,  311,  312,   -1,  216,
  217,   -1,   -1,  318,  319,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  216,  217,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  257,
  258,  259,  260,  261,   -1,  263,  264,   -1,   -1,  267,
  268,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  310,  311,  312,   -1,  271,  272,   -1,   -1,  318,  319,
   -1,  289,  290,   -1,   -1,   -1,   -1,  271,  272,   -1,
  318,  319,  300,  301,  302,  303,  304,  305,  306,  307,
  308,   -1,   -1,   -1,   -1,   -1,   -1,  318,  319,
};
#if YYBTYACC
static const YYINT pcap_ctable[] = {                     -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
};
#endif
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 376
#define YYUNDFTOKEN 424
#define YYTRANSLATE(a) ((a) > YYMAXTOKEN ? YYUNDFTOKEN : (a))
#if YYDEBUG
static const char *const pcap_name[] = {

"$end",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'!'",0,
0,0,"'%'","'&'",0,"'('","')'","'*'","'+'",0,"'-'",0,"'/'",0,0,0,0,0,0,0,0,0,0,
"':'",0,"'<'","'='","'>'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,"'['",0,"']'","'^'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'|'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,"error","DST","SRC","HOST","GATEWAY","NET",
"NETMASK","PORT","PORTRANGE","LESS","GREATER","PROTO","PROTOCHAIN","CBYTE",
"ARP","RARP","IP","SCTP","TCP","UDP","ICMP","IGMP","IGRP","PIM","VRRP","CARP",
"ATALK","AARP","DECNET","LAT","SCA","MOPRC","MOPDL","TK_BROADCAST",
"TK_MULTICAST","NUM","INBOUND","OUTBOUND","PF_IFNAME","PF_RSET","PF_RNR",
"PF_SRNR","PF_REASON","PF_ACTION","TYPE","SUBTYPE","DIR","ADDR1","ADDR2",
"ADDR3","ADDR4","RA","TA","LINK","GEQ","LEQ","NEQ","ID","EID","HID","HID6",
"AID","LSH","RSH","LEN","IPV6","ICMPV6","AH","ESP","VLAN","MPLS","PPPOED",
"PPPOES","GENEVE","ISO","ESIS","CLNP","ISIS","L1","L2","IIH","LSP","SNP","CSNP",
"PSNP","STP","IPX","NETBEUI","LANE","LLC","METAC","BCC","SC","ILMIC","OAMF4EC",
"OAMF4SC","OAM","OAMF4","CONNECTMSG","METACONNECT","VPI","VCI","RADIO","FISU",
"LSSU","MSU","HFISU","HLSSU","HMSU","SIO","OPC","DPC","SLS","HSIO","HOPC",
"HDPC","HSLS","LEX_ERROR","OR","AND","UMINUS","$accept","prog","expr","id",
"nid","pid","term","rterm","qid","head","pqual","dqual","aqual","ndaqual",
"arth","narth","byteop","pname","pnum","relop","irelop","and","or","paren",
"not","null","other","pfvar","p80211","pllc","atmtype","atmmultitype",
"atmfield","atmfieldvalue","atmvalue","atmlistvalue","mtp2type","mtp3field",
"mtp3fieldvalue","mtp3value","mtp3listvalue","action","reason","type","subtype",
"type_subtype","dir","illegal-symbol",
};
static const char *const pcap_rule[] = {
"$accept : prog",
"prog : null expr",
"prog : null",
"null :",
"expr : term",
"expr : expr and term",
"expr : expr and id",
"expr : expr or term",
"expr : expr or id",
"and : AND",
"or : OR",
"id : nid",
"id : pnum",
"id : paren pid ')'",
"nid : ID",
"nid : HID '/' NUM",
"nid : HID NETMASK HID",
"nid : HID",
"nid : HID6 '/' NUM",
"nid : HID6",
"nid : EID",
"nid : AID",
"nid : not id",
"not : '!'",
"paren : '('",
"pid : nid",
"pid : qid and id",
"pid : qid or id",
"qid : pnum",
"qid : pid",
"term : rterm",
"term : not term",
"head : pqual dqual aqual",
"head : pqual dqual",
"head : pqual aqual",
"head : pqual PROTO",
"head : pqual PROTOCHAIN",
"head : pqual ndaqual",
"rterm : head id",
"rterm : paren expr ')'",
"rterm : pname",
"rterm : arth relop arth",
"rterm : arth irelop arth",
"rterm : other",
"rterm : atmtype",
"rterm : atmmultitype",
"rterm : atmfield atmvalue",
"rterm : mtp2type",
"rterm : mtp3field mtp3value",
"pqual : pname",
"pqual :",
"dqual : SRC",
"dqual : DST",
"dqual : SRC OR DST",
"dqual : DST OR SRC",
"dqual : SRC AND DST",
"dqual : DST AND SRC",
"dqual : ADDR1",
"dqual : ADDR2",
"dqual : ADDR3",
"dqual : ADDR4",
"dqual : RA",
"dqual : TA",
"aqual : HOST",
"aqual : NET",
"aqual : PORT",
"aqual : PORTRANGE",
"ndaqual : GATEWAY",
"pname : LINK",
"pname : IP",
"pname : ARP",
"pname : RARP",
"pname : SCTP",
"pname : TCP",
"pname : UDP",
"pname : ICMP",
"pname : IGMP",
"pname : IGRP",
"pname : PIM",
"pname : VRRP",
"pname : CARP",
"pname : ATALK",
"pname : AARP",
"pname : DECNET",
"pname : LAT",
"pname : SCA",
"pname : MOPDL",
"pname : MOPRC",
"pname : IPV6",
"pname : ICMPV6",
"pname : AH",
"pname : ESP",
"pname : ISO",
"pname : ESIS",
"pname : ISIS",
"pname : L1",
"pname : L2",
"pname : IIH",
"pname : LSP",
"pname : SNP",
"pname : PSNP",
"pname : CSNP",
"pname : CLNP",
"pname : STP",
"pname : IPX",
"pname : NETBEUI",
"pname : RADIO",
"other : pqual TK_BROADCAST",
"other : pqual TK_MULTICAST",
"other : LESS NUM",
"other : GREATER NUM",
"other : CBYTE NUM byteop NUM",
"other : INBOUND",
"other : OUTBOUND",
"other : VLAN pnum",
"other : VLAN",
"other : MPLS pnum",
"other : MPLS",
"other : PPPOED",
"other : PPPOES pnum",
"other : PPPOES",
"other : GENEVE pnum",
"other : GENEVE",
"other : pfvar",
"other : pqual p80211",
"other : pllc",
"pfvar : PF_IFNAME ID",
"pfvar : PF_RSET ID",
"pfvar : PF_RNR NUM",
"pfvar : PF_SRNR NUM",
"pfvar : PF_REASON reason",
"pfvar : PF_ACTION action",
"p80211 : TYPE type SUBTYPE subtype",
"p80211 : TYPE type",
"p80211 : SUBTYPE type_subtype",
"p80211 : DIR dir",
"type : NUM",
"type : ID",
"subtype : NUM",
"subtype : ID",
"type_subtype : ID",
"pllc : LLC",
"pllc : LLC ID",
"pllc : LLC PF_RNR",
"dir : NUM",
"dir : ID",
"reason : NUM",
"reason : ID",
"action : ID",
"relop : '>'",
"relop : GEQ",
"relop : '='",
"irelop : LEQ",
"irelop : '<'",
"irelop : NEQ",
"arth : pnum",
"arth : narth",
"narth : pname '[' arth ']'",
"narth : pname '[' arth ':' NUM ']'",
"narth : arth '+' arth",
"narth : arth '-' arth",
"narth : arth '*' arth",
"narth : arth '/' arth",
"narth : arth '%' arth",
"narth : arth '&' arth",
"narth : arth '|' arth",
"narth : arth '^' arth",
"narth : arth LSH arth",
"narth : arth RSH arth",
"narth : '-' arth",
"narth : paren narth ')'",
"narth : LEN",
"byteop : '&'",
"byteop : '|'",
"byteop : '<'",
"byteop : '>'",
"byteop : '='",
"pnum : NUM",
"pnum : paren pnum ')'",
"atmtype : LANE",
"atmtype : METAC",
"atmtype : BCC",
"atmtype : OAMF4EC",
"atmtype : OAMF4SC",
"atmtype : SC",
"atmtype : ILMIC",
"atmmultitype : OAM",
"atmmultitype : OAMF4",
"atmmultitype : CONNECTMSG",
"atmmultitype : METACONNECT",
"atmfield : VPI",
"atmfield : VCI",
"atmvalue : atmfieldvalue",
"atmvalue : relop NUM",
"atmvalue : irelop NUM",
"atmvalue : paren atmlistvalue ')'",
"atmfieldvalue : NUM",
"atmlistvalue : atmfieldvalue",
"atmlistvalue : atmlistvalue or atmfieldvalue",
"mtp2type : FISU",
"mtp2type : LSSU",
"mtp2type : MSU",
"mtp2type : HFISU",
"mtp2type : HLSSU",
"mtp2type : HMSU",
"mtp3field : SIO",
"mtp3field : OPC",
"mtp3field : DPC",
"mtp3field : SLS",
"mtp3field : HSIO",
"mtp3field : HOPC",
"mtp3field : HDPC",
"mtp3field : HSLS",
"mtp3value : mtp3fieldvalue",
"mtp3value : relop NUM",
"mtp3value : irelop NUM",
"mtp3value : paren mtp3listvalue ')'",
"mtp3fieldvalue : NUM",
"mtp3listvalue : mtp3fieldvalue",
"mtp3listvalue : mtp3listvalue or mtp3fieldvalue",

};
#endif

#if YYDEBUG
int      yydebug;
#endif

#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
#ifndef YYLLOC_DEFAULT
#define YYLLOC_DEFAULT(loc, rhs, n) \
do \
{ \
    if (n == 0) \
    { \
        (loc).first_line   = YYRHSLOC(rhs, 0).last_line; \
        (loc).first_column = YYRHSLOC(rhs, 0).last_column; \
        (loc).last_line    = YYRHSLOC(rhs, 0).last_line; \
        (loc).last_column  = YYRHSLOC(rhs, 0).last_column; \
    } \
    else \
    { \
        (loc).first_line   = YYRHSLOC(rhs, 1).first_line; \
        (loc).first_column = YYRHSLOC(rhs, 1).first_column; \
        (loc).last_line    = YYRHSLOC(rhs, n).last_line; \
        (loc).last_column  = YYRHSLOC(rhs, n).last_column; \
    } \
} while (0)
#endif /* YYLLOC_DEFAULT */
#endif /* defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED) */
#if YYBTYACC

#ifndef YYLVQUEUEGROWTH
#define YYLVQUEUEGROWTH 32
#endif
#endif /* YYBTYACC */

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH  10000
#endif
#endif

#ifndef YYINITSTACKSIZE
#define YYINITSTACKSIZE 200
#endif

typedef struct {
    unsigned stacksize;
    YYINT    *s_base;
    YYINT    *s_mark;
    YYINT    *s_last;
    YYSTYPE  *l_base;
    YYSTYPE  *l_mark;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    YYLTYPE  *p_base;
    YYLTYPE  *p_mark;
#endif
} YYSTACKDATA;
#if YYBTYACC

struct YYParseState_s
{
    struct YYParseState_s *save;    /* Previously saved parser state */
    YYSTACKDATA            yystack; /* saved parser stack */
    int                    state;   /* saved parser state */
    int                    errflag; /* saved error recovery status */
    int                    lexeme;  /* saved index of the conflict lexeme in the lexical queue */
    YYINT                  ctry;    /* saved index in yyctable[] for this conflict */
};
typedef struct YYParseState_s YYParseState;
#endif /* YYBTYACC */

/* For use in generated program */
#define yydepth (int)(yystack.s_mark - yystack.s_base)
#if YYBTYACC
#define yytrial (yyps->save)
#endif /* YYBTYACC */

#if YYDEBUG
#include <stdio.h>	/* needed for printf */
#endif

#include <stdlib.h>	/* needed for malloc, etc */
#include <string.h>	/* needed for memset */

/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(YYSTACKDATA *data)
{
    int i;
    unsigned newsize;
    YYINT *newss;
    YYSTYPE *newvs;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    YYLTYPE *newps;
#endif

    if ((newsize = data->stacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return YYENOMEM;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = (int) (data->s_mark - data->s_base);
    newss = (YYINT *)realloc(data->s_base, newsize * sizeof(*newss));
    if (newss == 0)
        return YYENOMEM;

    data->s_base = newss;
    data->s_mark = newss + i;

    newvs = (YYSTYPE *)realloc(data->l_base, newsize * sizeof(*newvs));
    if (newvs == 0)
        return YYENOMEM;

    data->l_base = newvs;
    data->l_mark = newvs + i;

#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    newps = (YYLTYPE *)realloc(data->p_base, newsize * sizeof(*newps));
    if (newps == 0)
        return YYENOMEM;

    data->p_base = newps;
    data->p_mark = newps + i;
#endif

    data->stacksize = newsize;
    data->s_last = data->s_base + newsize - 1;

#if YYDEBUG
    if (yydebug)
        fprintf(stderr, "%sdebug: stack size increased to %d\n", YYPREFIX, newsize);
#endif
    return 0;
}

#if YYPURE || defined(YY_NO_LEAKS)
static void yyfreestack(YYSTACKDATA *data)
{
    free(data->s_base);
    free(data->l_base);
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    free(data->p_base);
#endif
    memset(data, 0, sizeof(*data));
}
#else
#define yyfreestack(data) /* nothing */
#endif /* YYPURE || defined(YY_NO_LEAKS) */
#if YYBTYACC

static YYParseState *
yyNewState(unsigned size)
{
    YYParseState *p = (YYParseState *) malloc(sizeof(YYParseState));
    if (p == NULL) return NULL;

    p->yystack.stacksize = size;
    if (size == 0)
    {
        p->yystack.s_base = NULL;
        p->yystack.l_base = NULL;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        p->yystack.p_base = NULL;
#endif
        return p;
    }
    p->yystack.s_base    = (YYINT *) malloc(size * sizeof(YYINT));
    if (p->yystack.s_base == NULL) return NULL;
    p->yystack.l_base    = (YYSTYPE *) malloc(size * sizeof(YYSTYPE));
    if (p->yystack.l_base == NULL) return NULL;
    memset(p->yystack.l_base, 0, size * sizeof(YYSTYPE));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    p->yystack.p_base    = (YYLTYPE *) malloc(size * sizeof(YYLTYPE));
    if (p->yystack.p_base == NULL) return NULL;
    memset(p->yystack.p_base, 0, size * sizeof(YYLTYPE));
#endif

    return p;
}

static void
yyFreeState(YYParseState *p)
{
    yyfreestack(&p->yystack);
    free(p);
}
#endif /* YYBTYACC */

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab
#if YYBTYACC
#define YYVALID        do { if (yyps->save)            goto yyvalid; } while(0)
#define YYVALID_NESTED do { if (yyps->save && \
                                yyps->save->save == 0) goto yyvalid; } while(0)
#endif /* YYBTYACC */

int
YYPARSE_DECL()
{
    int      yyerrflag;
    int      yychar;
    YYSTYPE  yyval;
    YYSTYPE  yylval;
    int      yynerrs;

#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    YYLTYPE  yyloc; /* position returned by actions */
    YYLTYPE  yylloc; /* position from the lexer */
#endif

    /* variables for the parser stack */
    YYSTACKDATA yystack;
#if YYBTYACC

    /* Current parser state */
    static YYParseState *yyps = 0;

    /* yypath != NULL: do the full parse, starting at *yypath parser state. */
    static YYParseState *yypath = 0;

    /* Base of the lexical value queue */
    static YYSTYPE *yylvals = 0;

    /* Current position at lexical value queue */
    static YYSTYPE *yylvp = 0;

    /* End position of lexical value queue */
    static YYSTYPE *yylve = 0;

    /* The last allocated position at the lexical value queue */
    static YYSTYPE *yylvlim = 0;

#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    /* Base of the lexical position queue */
    static YYLTYPE *yylpsns = 0;

    /* Current position at lexical position queue */
    static YYLTYPE *yylpp = 0;

    /* End position of lexical position queue */
    static YYLTYPE *yylpe = 0;

    /* The last allocated position at the lexical position queue */
    static YYLTYPE *yylplim = 0;
#endif

    /* Current position at lexical token queue */
    static YYINT  *yylexp = 0;

    static YYINT  *yylexemes = 0;
#endif /* YYBTYACC */
    int yym, yyn, yystate, yyresult;
#if YYBTYACC
    int yynewerrflag;
    YYParseState *yyerrctx = NULL;
#endif /* YYBTYACC */
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    YYLTYPE  yyerror_loc_range[3]; /* position of error start/end (0 unused) */
#endif
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
    if (yydebug)
        fprintf(stderr, "%sdebug[<# of symbols on state stack>]\n", YYPREFIX);
#endif
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    memset(yyerror_loc_range, 0, sizeof(yyerror_loc_range));
#endif

    yyerrflag = 0;
    yychar = 0;
    memset(&yyval,  0, sizeof(yyval));
    memset(&yylval, 0, sizeof(yylval));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    memset(&yyloc,  0, sizeof(yyloc));
    memset(&yylloc, 0, sizeof(yylloc));
#endif

#if YYBTYACC
    yyps = yyNewState(0); if (yyps == 0) goto yyenomem;
    yyps->save = 0;
#endif /* YYBTYACC */
    yym = 0;
    /* yyn is set below */
    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

#if YYPURE
    memset(&yystack, 0, sizeof(yystack));
#endif

    if (yystack.s_base == NULL && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    yystack.s_mark = yystack.s_base;
    yystack.l_mark = yystack.l_base;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    yystack.p_mark = yystack.p_base;
#endif
    yystate = 0;
    *yystack.s_mark = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
#if YYBTYACC
        do {
        if (yylvp < yylve)
        {
            /* we're currently re-reading tokens */
            yylval = *yylvp++;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            yylloc = *yylpp++;
#endif
            yychar = *yylexp++;
            break;
        }
        if (yyps->save)
        {
            /* in trial mode; save scanner results for future parse attempts */
            if (yylvp == yylvlim)
            {   /* Enlarge lexical value queue */
                size_t p = (size_t) (yylvp - yylvals);
                size_t s = (size_t) (yylvlim - yylvals);

                s += YYLVQUEUEGROWTH;
                if ((yylexemes = (YYINT *)realloc(yylexemes, s * sizeof(YYINT))) == NULL) goto yyenomem;
                if ((yylvals   = (YYSTYPE *)realloc(yylvals, s * sizeof(YYSTYPE))) == NULL) goto yyenomem;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                if ((yylpsns   = (YYLTYPE *)realloc(yylpsns, s * sizeof(YYLTYPE))) == NULL) goto yyenomem;
#endif
                yylvp   = yylve = yylvals + p;
                yylvlim = yylvals + s;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                yylpp   = yylpe = yylpsns + p;
                yylplim = yylpsns + s;
#endif
                yylexp  = yylexemes + p;
            }
            *yylexp = (YYINT) YYLEX;
            *yylvp++ = yylval;
            yylve++;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            *yylpp++ = yylloc;
            yylpe++;
#endif
            yychar = *yylexp++;
            break;
        }
        /* normal operation, no conflict encountered */
#endif /* YYBTYACC */
        yychar = YYLEX;
#if YYBTYACC
        } while (0);
#endif /* YYBTYACC */
        if (yychar < 0) yychar = YYEOF;
#if YYDEBUG
        if (yydebug)
        {
            if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
            fprintf(stderr, "%s[%d]: state %d, reading token %d (%s)",
                            YYDEBUGSTR, yydepth, yystate, yychar, yys);
#ifdef YYSTYPE_TOSTRING
#if YYBTYACC
            if (!yytrial)
#endif /* YYBTYACC */
                fprintf(stderr, " <%s>", YYSTYPE_TOSTRING(yychar, yylval));
#endif
            fputc('\n', stderr);
        }
#endif
    }
#if YYBTYACC

    /* Do we have a conflict? */
    if (((yyn = yycindex[yystate]) != 0) && (yyn += yychar) >= 0 &&
        yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yychar)
    {
        YYINT ctry;

        if (yypath)
        {
            YYParseState *save;
#if YYDEBUG
            if (yydebug)
                fprintf(stderr, "%s[%d]: CONFLICT in state %d: following successful trial parse\n",
                                YYDEBUGSTR, yydepth, yystate);
#endif
            /* Switch to the next conflict context */
            save = yypath;
            yypath = save->save;
            save->save = NULL;
            ctry = save->ctry;
            if (save->state != yystate) YYABORT;
            yyFreeState(save);

        }
        else
        {

            /* Unresolved conflict - start/continue trial parse */
            YYParseState *save;
#if YYDEBUG
            if (yydebug)
            {
                fprintf(stderr, "%s[%d]: CONFLICT in state %d. ", YYDEBUGSTR, yydepth, yystate);
                if (yyps->save)
                    fputs("ALREADY in conflict, continuing trial parse.\n", stderr);
                else
                    fputs("Starting trial parse.\n", stderr);
            }
#endif
            save                  = yyNewState((unsigned)(yystack.s_mark - yystack.s_base + 1));
            if (save == NULL) goto yyenomem;
            save->save            = yyps->save;
            save->state           = yystate;
            save->errflag         = yyerrflag;
            save->yystack.s_mark  = save->yystack.s_base + (yystack.s_mark - yystack.s_base);
            memcpy (save->yystack.s_base, yystack.s_base, (size_t) (yystack.s_mark - yystack.s_base + 1) * sizeof(YYINT));
            save->yystack.l_mark  = save->yystack.l_base + (yystack.l_mark - yystack.l_base);
            memcpy (save->yystack.l_base, yystack.l_base, (size_t) (yystack.l_mark - yystack.l_base + 1) * sizeof(YYSTYPE));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            save->yystack.p_mark  = save->yystack.p_base + (yystack.p_mark - yystack.p_base);
            memcpy (save->yystack.p_base, yystack.p_base, (size_t) (yystack.p_mark - yystack.p_base + 1) * sizeof(YYLTYPE));
#endif
            ctry                  = yytable[yyn];
            if (yyctable[ctry] == -1)
            {
#if YYDEBUG
                if (yydebug && yychar >= YYEOF)
                    fprintf(stderr, "%s[%d]: backtracking 1 token\n", YYDEBUGSTR, yydepth);
#endif
                ctry++;
            }
            save->ctry = ctry;
            if (yyps->save == NULL)
            {
                /* If this is a first conflict in the stack, start saving lexemes */
                if (!yylexemes)
                {
                    yylexemes = (YYINT *) malloc((YYLVQUEUEGROWTH) * sizeof(YYINT));
                    if (yylexemes == NULL) goto yyenomem;
                    yylvals   = (YYSTYPE *) malloc((YYLVQUEUEGROWTH) * sizeof(YYSTYPE));
                    if (yylvals == NULL) goto yyenomem;
                    yylvlim   = yylvals + YYLVQUEUEGROWTH;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                    yylpsns   = (YYLTYPE *) malloc((YYLVQUEUEGROWTH) * sizeof(YYLTYPE));
                    if (yylpsns == NULL) goto yyenomem;
                    yylplim   = yylpsns + YYLVQUEUEGROWTH;
#endif
                }
                if (yylvp == yylve)
                {
                    yylvp  = yylve = yylvals;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                    yylpp  = yylpe = yylpsns;
#endif
                    yylexp = yylexemes;
                    if (yychar >= YYEOF)
                    {
                        *yylve++ = yylval;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                        *yylpe++ = yylloc;
#endif
                        *yylexp  = (YYINT) yychar;
                        yychar   = YYEMPTY;
                    }
                }
            }
            if (yychar >= YYEOF)
            {
                yylvp--;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                yylpp--;
#endif
                yylexp--;
                yychar = YYEMPTY;
            }
            save->lexeme = (int) (yylvp - yylvals);
            yyps->save   = save;
        }
        if (yytable[yyn] == ctry)
        {
#if YYDEBUG
            if (yydebug)
                fprintf(stderr, "%s[%d]: state %d, shifting to state %d\n",
                                YYDEBUGSTR, yydepth, yystate, yyctable[ctry]);
#endif
            if (yychar < 0)
            {
                yylvp++;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                yylpp++;
#endif
                yylexp++;
            }
            if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM)
                goto yyoverflow;
            yystate = yyctable[ctry];
            *++yystack.s_mark = (YYINT) yystate;
            *++yystack.l_mark = yylval;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            *++yystack.p_mark = yylloc;
#endif
            yychar  = YYEMPTY;
            if (yyerrflag > 0) --yyerrflag;
            goto yyloop;
        }
        else
        {
            yyn = yyctable[ctry];
            goto yyreduce;
        }
    } /* End of code dealing with conflicts */
#endif /* YYBTYACC */
    if (((yyn = yysindex[yystate]) != 0) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yychar)
    {
#if YYDEBUG
        if (yydebug)
            fprintf(stderr, "%s[%d]: state %d, shifting to state %d\n",
                            YYDEBUGSTR, yydepth, yystate, yytable[yyn]);
#endif
        if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
        yystate = yytable[yyn];
        *++yystack.s_mark = yytable[yyn];
        *++yystack.l_mark = yylval;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        *++yystack.p_mark = yylloc;
#endif
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if (((yyn = yyrindex[yystate]) != 0) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag != 0) goto yyinrecovery;
#if YYBTYACC

    yynewerrflag = 1;
    goto yyerrhandler;
    goto yyerrlab; /* redundant goto avoids 'unused label' warning */

yyerrlab:
    /* explicit YYERROR from an action -- pop the rhs of the rule reduced
     * before looking for error recovery */
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    yystack.p_mark -= yym;
#endif

    yynewerrflag = 0;
yyerrhandler:
    while (yyps->save)
    {
        int ctry;
        YYParseState *save = yyps->save;
#if YYDEBUG
        if (yydebug)
            fprintf(stderr, "%s[%d]: ERROR in state %d, CONFLICT BACKTRACKING to state %d, %d tokens\n",
                            YYDEBUGSTR, yydepth, yystate, yyps->save->state,
                    (int)(yylvp - yylvals - yyps->save->lexeme));
#endif
        /* Memorize most forward-looking error state in case it's really an error. */
        if (yyerrctx == NULL || yyerrctx->lexeme < yylvp - yylvals)
        {
            /* Free old saved error context state */
            if (yyerrctx) yyFreeState(yyerrctx);
            /* Create and fill out new saved error context state */
            yyerrctx                 = yyNewState((unsigned)(yystack.s_mark - yystack.s_base + 1));
            if (yyerrctx == NULL) goto yyenomem;
            yyerrctx->save           = yyps->save;
            yyerrctx->state          = yystate;
            yyerrctx->errflag        = yyerrflag;
            yyerrctx->yystack.s_mark = yyerrctx->yystack.s_base + (yystack.s_mark - yystack.s_base);
            memcpy (yyerrctx->yystack.s_base, yystack.s_base, (size_t) (yystack.s_mark - yystack.s_base + 1) * sizeof(YYINT));
            yyerrctx->yystack.l_mark = yyerrctx->yystack.l_base + (yystack.l_mark - yystack.l_base);
            memcpy (yyerrctx->yystack.l_base, yystack.l_base, (size_t) (yystack.l_mark - yystack.l_base + 1) * sizeof(YYSTYPE));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            yyerrctx->yystack.p_mark = yyerrctx->yystack.p_base + (yystack.p_mark - yystack.p_base);
            memcpy (yyerrctx->yystack.p_base, yystack.p_base, (size_t) (yystack.p_mark - yystack.p_base + 1) * sizeof(YYLTYPE));
#endif
            yyerrctx->lexeme         = (int) (yylvp - yylvals);
        }
        yylvp          = yylvals   + save->lexeme;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        yylpp          = yylpsns   + save->lexeme;
#endif
        yylexp         = yylexemes + save->lexeme;
        yychar         = YYEMPTY;
        yystack.s_mark = yystack.s_base + (save->yystack.s_mark - save->yystack.s_base);
        memcpy (yystack.s_base, save->yystack.s_base, (size_t) (yystack.s_mark - yystack.s_base + 1) * sizeof(YYINT));
        yystack.l_mark = yystack.l_base + (save->yystack.l_mark - save->yystack.l_base);
        memcpy (yystack.l_base, save->yystack.l_base, (size_t) (yystack.l_mark - yystack.l_base + 1) * sizeof(YYSTYPE));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        yystack.p_mark = yystack.p_base + (save->yystack.p_mark - save->yystack.p_base);
        memcpy (yystack.p_base, save->yystack.p_base, (size_t) (yystack.p_mark - yystack.p_base + 1) * sizeof(YYLTYPE));
#endif
        ctry           = ++save->ctry;
        yystate        = save->state;
        /* We tried shift, try reduce now */
        if ((yyn = yyctable[ctry]) >= 0) goto yyreduce;
        yyps->save     = save->save;
        save->save     = NULL;
        yyFreeState(save);

        /* Nothing left on the stack -- error */
        if (!yyps->save)
        {
#if YYDEBUG
            if (yydebug)
                fprintf(stderr, "%sdebug[%d,trial]: trial parse FAILED, entering ERROR mode\n",
                                YYPREFIX, yydepth);
#endif
            /* Restore state as it was in the most forward-advanced error */
            yylvp          = yylvals   + yyerrctx->lexeme;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            yylpp          = yylpsns   + yyerrctx->lexeme;
#endif
            yylexp         = yylexemes + yyerrctx->lexeme;
            yychar         = yylexp[-1];
            yylval         = yylvp[-1];
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            yylloc         = yylpp[-1];
#endif
            yystack.s_mark = yystack.s_base + (yyerrctx->yystack.s_mark - yyerrctx->yystack.s_base);
            memcpy (yystack.s_base, yyerrctx->yystack.s_base, (size_t) (yystack.s_mark - yystack.s_base + 1) * sizeof(YYINT));
            yystack.l_mark = yystack.l_base + (yyerrctx->yystack.l_mark - yyerrctx->yystack.l_base);
            memcpy (yystack.l_base, yyerrctx->yystack.l_base, (size_t) (yystack.l_mark - yystack.l_base + 1) * sizeof(YYSTYPE));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            yystack.p_mark = yystack.p_base + (yyerrctx->yystack.p_mark - yyerrctx->yystack.p_base);
            memcpy (yystack.p_base, yyerrctx->yystack.p_base, (size_t) (yystack.p_mark - yystack.p_base + 1) * sizeof(YYLTYPE));
#endif
            yystate        = yyerrctx->state;
            yyFreeState(yyerrctx);
            yyerrctx       = NULL;
        }
        yynewerrflag = 1;
    }
    if (yynewerrflag == 0) goto yyinrecovery;
#endif /* YYBTYACC */

    YYERROR_CALL("syntax error");
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    yyerror_loc_range[1] = yylloc; /* lookahead position is error start position */
#endif

#if !YYBTYACC
    goto yyerrlab; /* redundant goto avoids 'unused label' warning */
yyerrlab:
#endif
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if (((yyn = yysindex[*yystack.s_mark]) != 0) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    fprintf(stderr, "%s[%d]: state %d, error recovery shifting to state %d\n",
                                    YYDEBUGSTR, yydepth, *yystack.s_mark, yytable[yyn]);
#endif
                if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
                yystate = yytable[yyn];
                *++yystack.s_mark = yytable[yyn];
                *++yystack.l_mark = yylval;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                /* lookahead position is error end position */
                yyerror_loc_range[2] = yylloc;
                YYLLOC_DEFAULT(yyloc, yyerror_loc_range, 2); /* position of error span */
                *++yystack.p_mark = yyloc;
#endif
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    fprintf(stderr, "%s[%d]: error recovery discarding state %d\n",
                                    YYDEBUGSTR, yydepth, *yystack.s_mark);
#endif
                if (yystack.s_mark <= yystack.s_base) goto yyabort;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                /* the current TOS position is the error start position */
                yyerror_loc_range[1] = *yystack.p_mark;
#endif
#if defined(YYDESTRUCT_CALL)
#if YYBTYACC
                if (!yytrial)
#endif /* YYBTYACC */
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                    YYDESTRUCT_CALL("error: discarding state",
                                    yystos[*yystack.s_mark], yystack.l_mark, yystack.p_mark);
#else
                    YYDESTRUCT_CALL("error: discarding state",
                                    yystos[*yystack.s_mark], yystack.l_mark);
#endif /* defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED) */
#endif /* defined(YYDESTRUCT_CALL) */
                --yystack.s_mark;
                --yystack.l_mark;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                --yystack.p_mark;
#endif
            }
        }
    }
    else
    {
        if (yychar == YYEOF) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
            fprintf(stderr, "%s[%d]: state %d, error recovery discarding token %d (%s)\n",
                            YYDEBUGSTR, yydepth, yystate, yychar, yys);
        }
#endif
#if defined(YYDESTRUCT_CALL)
#if YYBTYACC
        if (!yytrial)
#endif /* YYBTYACC */
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            YYDESTRUCT_CALL("error: discarding token", yychar, &yylval, &yylloc);
#else
            YYDESTRUCT_CALL("error: discarding token", yychar, &yylval);
#endif /* defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED) */
#endif /* defined(YYDESTRUCT_CALL) */
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
    yym = yylen[yyn];
#if YYDEBUG
    if (yydebug)
    {
        fprintf(stderr, "%s[%d]: state %d, reducing by rule %d (%s)",
                        YYDEBUGSTR, yydepth, yystate, yyn, yyrule[yyn]);
#ifdef YYSTYPE_TOSTRING
#if YYBTYACC
        if (!yytrial)
#endif /* YYBTYACC */
            if (yym > 0)
            {
                int i;
                fputc('<', stderr);
                for (i = yym; i > 0; i--)
                {
                    if (i != yym) fputs(", ", stderr);
                    fputs(YYSTYPE_TOSTRING(yystos[yystack.s_mark[1-i]],
                                           yystack.l_mark[1-i]), stderr);
                }
                fputc('>', stderr);
            }
#endif
        fputc('\n', stderr);
    }
#endif
    if (yym > 0)
        yyval = yystack.l_mark[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)

    /* Perform position reduction */
    memset(&yyloc, 0, sizeof(yyloc));
#if YYBTYACC
    if (!yytrial)
#endif /* YYBTYACC */
    {
        YYLLOC_DEFAULT(yyloc, &yystack.p_mark[-yym], yym);
        /* just in case YYERROR is invoked within the action, save
           the start of the rhs as the error start position */
        yyerror_loc_range[1] = yystack.p_mark[1-yym];
    }
#endif

    switch (yyn)
    {
case 1:
#line 363 "../grammar.y"
	{
	CHECK_INT_VAL(finish_parse(cstate, yystack.l_mark[0].blk.b));
}
#line 2265 "grammar.c"
break;
case 3:
#line 368 "../grammar.y"
	{ yyval.blk.q = qerr; }
#line 2270 "grammar.c"
break;
case 5:
#line 371 "../grammar.y"
	{ gen_and(yystack.l_mark[-2].blk.b, yystack.l_mark[0].blk.b); yyval.blk = yystack.l_mark[0].blk; }
#line 2275 "grammar.c"
break;
case 6:
#line 372 "../grammar.y"
	{ gen_and(yystack.l_mark[-2].blk.b, yystack.l_mark[0].blk.b); yyval.blk = yystack.l_mark[0].blk; }
#line 2280 "grammar.c"
break;
case 7:
#line 373 "../grammar.y"
	{ gen_or(yystack.l_mark[-2].blk.b, yystack.l_mark[0].blk.b); yyval.blk = yystack.l_mark[0].blk; }
#line 2285 "grammar.c"
break;
case 8:
#line 374 "../grammar.y"
	{ gen_or(yystack.l_mark[-2].blk.b, yystack.l_mark[0].blk.b); yyval.blk = yystack.l_mark[0].blk; }
#line 2290 "grammar.c"
break;
case 9:
#line 376 "../grammar.y"
	{ yyval.blk = yystack.l_mark[-1].blk; }
#line 2295 "grammar.c"
break;
case 10:
#line 378 "../grammar.y"
	{ yyval.blk = yystack.l_mark[-1].blk; }
#line 2300 "grammar.c"
break;
case 12:
#line 381 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.blk.b = gen_ncode(cstate, NULL, (bpf_u_int32)yystack.l_mark[0].i,
						   yyval.blk.q = yystack.l_mark[-1].blk.q))); }
#line 2306 "grammar.c"
break;
case 13:
#line 383 "../grammar.y"
	{ yyval.blk = yystack.l_mark[-1].blk; }
#line 2311 "grammar.c"
break;
case 14:
#line 385 "../grammar.y"
	{ CHECK_PTR_VAL(yystack.l_mark[0].s); CHECK_PTR_VAL((yyval.blk.b = gen_scode(cstate, yystack.l_mark[0].s, yyval.blk.q = yystack.l_mark[-1].blk.q))); }
#line 2316 "grammar.c"
break;
case 15:
#line 386 "../grammar.y"
	{ CHECK_PTR_VAL(yystack.l_mark[-2].s); CHECK_PTR_VAL((yyval.blk.b = gen_mcode(cstate, yystack.l_mark[-2].s, NULL, yystack.l_mark[0].i,
				    yyval.blk.q = yystack.l_mark[-3].blk.q))); }
#line 2322 "grammar.c"
break;
case 16:
#line 388 "../grammar.y"
	{ CHECK_PTR_VAL(yystack.l_mark[-2].s); CHECK_PTR_VAL((yyval.blk.b = gen_mcode(cstate, yystack.l_mark[-2].s, yystack.l_mark[0].s, 0,
				    yyval.blk.q = yystack.l_mark[-3].blk.q))); }
#line 2328 "grammar.c"
break;
case 17:
#line 390 "../grammar.y"
	{
				  CHECK_PTR_VAL(yystack.l_mark[0].s);
				  /* Decide how to parse HID based on proto */
				  yyval.blk.q = yystack.l_mark[-1].blk.q;
				  if (yyval.blk.q.addr == Q_PORT) {
				  	bpf_set_error(cstate, "'port' modifier applied to ip host");
				  	YYABORT;
				  } else if (yyval.blk.q.addr == Q_PORTRANGE) {
				  	bpf_set_error(cstate, "'portrange' modifier applied to ip host");
				  	YYABORT;
				  } else if (yyval.blk.q.addr == Q_PROTO) {
				  	bpf_set_error(cstate, "'proto' modifier applied to ip host");
				  	YYABORT;
				  } else if (yyval.blk.q.addr == Q_PROTOCHAIN) {
				  	bpf_set_error(cstate, "'protochain' modifier applied to ip host");
				  	YYABORT;
				  }
				  CHECK_PTR_VAL((yyval.blk.b = gen_ncode(cstate, yystack.l_mark[0].s, 0, yyval.blk.q)));
				}
#line 2351 "grammar.c"
break;
case 18:
#line 409 "../grammar.y"
	{
				  CHECK_PTR_VAL(yystack.l_mark[-2].s);
#ifdef INET6
				  CHECK_PTR_VAL((yyval.blk.b = gen_mcode6(cstate, yystack.l_mark[-2].s, NULL, yystack.l_mark[0].i,
				    yyval.blk.q = yystack.l_mark[-3].blk.q)));
#else
				  bpf_set_error(cstate, "'ip6addr/prefixlen' not supported "
					"in this configuration");
				  YYABORT;
#endif /*INET6*/
				}
#line 2366 "grammar.c"
break;
case 19:
#line 420 "../grammar.y"
	{
				  CHECK_PTR_VAL(yystack.l_mark[0].s);
#ifdef INET6
				  CHECK_PTR_VAL((yyval.blk.b = gen_mcode6(cstate, yystack.l_mark[0].s, 0, 128,
				    yyval.blk.q = yystack.l_mark[-1].blk.q)));
#else
				  bpf_set_error(cstate, "'ip6addr' not supported "
					"in this configuration");
				  YYABORT;
#endif /*INET6*/
				}
#line 2381 "grammar.c"
break;
case 20:
#line 431 "../grammar.y"
	{ CHECK_PTR_VAL(yystack.l_mark[0].s); CHECK_PTR_VAL((yyval.blk.b = gen_ecode(cstate, yystack.l_mark[0].s, yyval.blk.q = yystack.l_mark[-1].blk.q))); }
#line 2386 "grammar.c"
break;
case 21:
#line 432 "../grammar.y"
	{ CHECK_PTR_VAL(yystack.l_mark[0].s); CHECK_PTR_VAL((yyval.blk.b = gen_acode(cstate, yystack.l_mark[0].s, yyval.blk.q = yystack.l_mark[-1].blk.q))); }
#line 2391 "grammar.c"
break;
case 22:
#line 433 "../grammar.y"
	{ gen_not(yystack.l_mark[0].blk.b); yyval.blk = yystack.l_mark[0].blk; }
#line 2396 "grammar.c"
break;
case 23:
#line 435 "../grammar.y"
	{ yyval.blk = yystack.l_mark[-1].blk; }
#line 2401 "grammar.c"
break;
case 24:
#line 437 "../grammar.y"
	{ yyval.blk = yystack.l_mark[-1].blk; }
#line 2406 "grammar.c"
break;
case 26:
#line 440 "../grammar.y"
	{ gen_and(yystack.l_mark[-2].blk.b, yystack.l_mark[0].blk.b); yyval.blk = yystack.l_mark[0].blk; }
#line 2411 "grammar.c"
break;
case 27:
#line 441 "../grammar.y"
	{ gen_or(yystack.l_mark[-2].blk.b, yystack.l_mark[0].blk.b); yyval.blk = yystack.l_mark[0].blk; }
#line 2416 "grammar.c"
break;
case 28:
#line 443 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.blk.b = gen_ncode(cstate, NULL, (bpf_u_int32)yystack.l_mark[0].i,
						   yyval.blk.q = yystack.l_mark[-1].blk.q))); }
#line 2422 "grammar.c"
break;
case 31:
#line 448 "../grammar.y"
	{ gen_not(yystack.l_mark[0].blk.b); yyval.blk = yystack.l_mark[0].blk; }
#line 2427 "grammar.c"
break;
case 32:
#line 450 "../grammar.y"
	{ QSET(yyval.blk.q, yystack.l_mark[-2].i, yystack.l_mark[-1].i, yystack.l_mark[0].i); }
#line 2432 "grammar.c"
break;
case 33:
#line 451 "../grammar.y"
	{ QSET(yyval.blk.q, yystack.l_mark[-1].i, yystack.l_mark[0].i, Q_DEFAULT); }
#line 2437 "grammar.c"
break;
case 34:
#line 452 "../grammar.y"
	{ QSET(yyval.blk.q, yystack.l_mark[-1].i, Q_DEFAULT, yystack.l_mark[0].i); }
#line 2442 "grammar.c"
break;
case 35:
#line 453 "../grammar.y"
	{ QSET(yyval.blk.q, yystack.l_mark[-1].i, Q_DEFAULT, Q_PROTO); }
#line 2447 "grammar.c"
break;
case 36:
#line 454 "../grammar.y"
	{
#ifdef NO_PROTOCHAIN
				  bpf_set_error(cstate, "protochain not supported");
				  YYABORT;
#else
				  QSET(yyval.blk.q, yystack.l_mark[-1].i, Q_DEFAULT, Q_PROTOCHAIN);
#endif
				}
#line 2459 "grammar.c"
break;
case 37:
#line 462 "../grammar.y"
	{ QSET(yyval.blk.q, yystack.l_mark[-1].i, Q_DEFAULT, yystack.l_mark[0].i); }
#line 2464 "grammar.c"
break;
case 38:
#line 464 "../grammar.y"
	{ yyval.blk = yystack.l_mark[0].blk; }
#line 2469 "grammar.c"
break;
case 39:
#line 465 "../grammar.y"
	{ yyval.blk.b = yystack.l_mark[-1].blk.b; yyval.blk.q = yystack.l_mark[-2].blk.q; }
#line 2474 "grammar.c"
break;
case 40:
#line 466 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.blk.b = gen_proto_abbrev(cstate, yystack.l_mark[0].i))); yyval.blk.q = qerr; }
#line 2479 "grammar.c"
break;
case 41:
#line 467 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.blk.b = gen_relation(cstate, yystack.l_mark[-1].i, yystack.l_mark[-2].a, yystack.l_mark[0].a, 0)));
				  yyval.blk.q = qerr; }
#line 2485 "grammar.c"
break;
case 42:
#line 469 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.blk.b = gen_relation(cstate, yystack.l_mark[-1].i, yystack.l_mark[-2].a, yystack.l_mark[0].a, 1)));
				  yyval.blk.q = qerr; }
#line 2491 "grammar.c"
break;
case 43:
#line 471 "../grammar.y"
	{ yyval.blk.b = yystack.l_mark[0].rblk; yyval.blk.q = qerr; }
#line 2496 "grammar.c"
break;
case 44:
#line 472 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.blk.b = gen_atmtype_abbrev(cstate, yystack.l_mark[0].i))); yyval.blk.q = qerr; }
#line 2501 "grammar.c"
break;
case 45:
#line 473 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.blk.b = gen_atmmulti_abbrev(cstate, yystack.l_mark[0].i))); yyval.blk.q = qerr; }
#line 2506 "grammar.c"
break;
case 46:
#line 474 "../grammar.y"
	{ yyval.blk.b = yystack.l_mark[0].blk.b; yyval.blk.q = qerr; }
#line 2511 "grammar.c"
break;
case 47:
#line 475 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.blk.b = gen_mtp2type_abbrev(cstate, yystack.l_mark[0].i))); yyval.blk.q = qerr; }
#line 2516 "grammar.c"
break;
case 48:
#line 476 "../grammar.y"
	{ yyval.blk.b = yystack.l_mark[0].blk.b; yyval.blk.q = qerr; }
#line 2521 "grammar.c"
break;
case 50:
#line 480 "../grammar.y"
	{ yyval.i = Q_DEFAULT; }
#line 2526 "grammar.c"
break;
case 51:
#line 483 "../grammar.y"
	{ yyval.i = Q_SRC; }
#line 2531 "grammar.c"
break;
case 52:
#line 484 "../grammar.y"
	{ yyval.i = Q_DST; }
#line 2536 "grammar.c"
break;
case 53:
#line 485 "../grammar.y"
	{ yyval.i = Q_OR; }
#line 2541 "grammar.c"
break;
case 54:
#line 486 "../grammar.y"
	{ yyval.i = Q_OR; }
#line 2546 "grammar.c"
break;
case 55:
#line 487 "../grammar.y"
	{ yyval.i = Q_AND; }
#line 2551 "grammar.c"
break;
case 56:
#line 488 "../grammar.y"
	{ yyval.i = Q_AND; }
#line 2556 "grammar.c"
break;
case 57:
#line 489 "../grammar.y"
	{ yyval.i = Q_ADDR1; }
#line 2561 "grammar.c"
break;
case 58:
#line 490 "../grammar.y"
	{ yyval.i = Q_ADDR2; }
#line 2566 "grammar.c"
break;
case 59:
#line 491 "../grammar.y"
	{ yyval.i = Q_ADDR3; }
#line 2571 "grammar.c"
break;
case 60:
#line 492 "../grammar.y"
	{ yyval.i = Q_ADDR4; }
#line 2576 "grammar.c"
break;
case 61:
#line 493 "../grammar.y"
	{ yyval.i = Q_RA; }
#line 2581 "grammar.c"
break;
case 62:
#line 494 "../grammar.y"
	{ yyval.i = Q_TA; }
#line 2586 "grammar.c"
break;
case 63:
#line 497 "../grammar.y"
	{ yyval.i = Q_HOST; }
#line 2591 "grammar.c"
break;
case 64:
#line 498 "../grammar.y"
	{ yyval.i = Q_NET; }
#line 2596 "grammar.c"
break;
case 65:
#line 499 "../grammar.y"
	{ yyval.i = Q_PORT; }
#line 2601 "grammar.c"
break;
case 66:
#line 500 "../grammar.y"
	{ yyval.i = Q_PORTRANGE; }
#line 2606 "grammar.c"
break;
case 67:
#line 503 "../grammar.y"
	{ yyval.i = Q_GATEWAY; }
#line 2611 "grammar.c"
break;
case 68:
#line 505 "../grammar.y"
	{ yyval.i = Q_LINK; }
#line 2616 "grammar.c"
break;
case 69:
#line 506 "../grammar.y"
	{ yyval.i = Q_IP; }
#line 2621 "grammar.c"
break;
case 70:
#line 507 "../grammar.y"
	{ yyval.i = Q_ARP; }
#line 2626 "grammar.c"
break;
case 71:
#line 508 "../grammar.y"
	{ yyval.i = Q_RARP; }
#line 2631 "grammar.c"
break;
case 72:
#line 509 "../grammar.y"
	{ yyval.i = Q_SCTP; }
#line 2636 "grammar.c"
break;
case 73:
#line 510 "../grammar.y"
	{ yyval.i = Q_TCP; }
#line 2641 "grammar.c"
break;
case 74:
#line 511 "../grammar.y"
	{ yyval.i = Q_UDP; }
#line 2646 "grammar.c"
break;
case 75:
#line 512 "../grammar.y"
	{ yyval.i = Q_ICMP; }
#line 2651 "grammar.c"
break;
case 76:
#line 513 "../grammar.y"
	{ yyval.i = Q_IGMP; }
#line 2656 "grammar.c"
break;
case 77:
#line 514 "../grammar.y"
	{ yyval.i = Q_IGRP; }
#line 2661 "grammar.c"
break;
case 78:
#line 515 "../grammar.y"
	{ yyval.i = Q_PIM; }
#line 2666 "grammar.c"
break;
case 79:
#line 516 "../grammar.y"
	{ yyval.i = Q_VRRP; }
#line 2671 "grammar.c"
break;
case 80:
#line 517 "../grammar.y"
	{ yyval.i = Q_CARP; }
#line 2676 "grammar.c"
break;
case 81:
#line 518 "../grammar.y"
	{ yyval.i = Q_ATALK; }
#line 2681 "grammar.c"
break;
case 82:
#line 519 "../grammar.y"
	{ yyval.i = Q_AARP; }
#line 2686 "grammar.c"
break;
case 83:
#line 520 "../grammar.y"
	{ yyval.i = Q_DECNET; }
#line 2691 "grammar.c"
break;
case 84:
#line 521 "../grammar.y"
	{ yyval.i = Q_LAT; }
#line 2696 "grammar.c"
break;
case 85:
#line 522 "../grammar.y"
	{ yyval.i = Q_SCA; }
#line 2701 "grammar.c"
break;
case 86:
#line 523 "../grammar.y"
	{ yyval.i = Q_MOPDL; }
#line 2706 "grammar.c"
break;
case 87:
#line 524 "../grammar.y"
	{ yyval.i = Q_MOPRC; }
#line 2711 "grammar.c"
break;
case 88:
#line 525 "../grammar.y"
	{ yyval.i = Q_IPV6; }
#line 2716 "grammar.c"
break;
case 89:
#line 526 "../grammar.y"
	{ yyval.i = Q_ICMPV6; }
#line 2721 "grammar.c"
break;
case 90:
#line 527 "../grammar.y"
	{ yyval.i = Q_AH; }
#line 2726 "grammar.c"
break;
case 91:
#line 528 "../grammar.y"
	{ yyval.i = Q_ESP; }
#line 2731 "grammar.c"
break;
case 92:
#line 529 "../grammar.y"
	{ yyval.i = Q_ISO; }
#line 2736 "grammar.c"
break;
case 93:
#line 530 "../grammar.y"
	{ yyval.i = Q_ESIS; }
#line 2741 "grammar.c"
break;
case 94:
#line 531 "../grammar.y"
	{ yyval.i = Q_ISIS; }
#line 2746 "grammar.c"
break;
case 95:
#line 532 "../grammar.y"
	{ yyval.i = Q_ISIS_L1; }
#line 2751 "grammar.c"
break;
case 96:
#line 533 "../grammar.y"
	{ yyval.i = Q_ISIS_L2; }
#line 2756 "grammar.c"
break;
case 97:
#line 534 "../grammar.y"
	{ yyval.i = Q_ISIS_IIH; }
#line 2761 "grammar.c"
break;
case 98:
#line 535 "../grammar.y"
	{ yyval.i = Q_ISIS_LSP; }
#line 2766 "grammar.c"
break;
case 99:
#line 536 "../grammar.y"
	{ yyval.i = Q_ISIS_SNP; }
#line 2771 "grammar.c"
break;
case 100:
#line 537 "../grammar.y"
	{ yyval.i = Q_ISIS_PSNP; }
#line 2776 "grammar.c"
break;
case 101:
#line 538 "../grammar.y"
	{ yyval.i = Q_ISIS_CSNP; }
#line 2781 "grammar.c"
break;
case 102:
#line 539 "../grammar.y"
	{ yyval.i = Q_CLNP; }
#line 2786 "grammar.c"
break;
case 103:
#line 540 "../grammar.y"
	{ yyval.i = Q_STP; }
#line 2791 "grammar.c"
break;
case 104:
#line 541 "../grammar.y"
	{ yyval.i = Q_IPX; }
#line 2796 "grammar.c"
break;
case 105:
#line 542 "../grammar.y"
	{ yyval.i = Q_NETBEUI; }
#line 2801 "grammar.c"
break;
case 106:
#line 543 "../grammar.y"
	{ yyval.i = Q_RADIO; }
#line 2806 "grammar.c"
break;
case 107:
#line 545 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_broadcast(cstate, yystack.l_mark[-1].i))); }
#line 2811 "grammar.c"
break;
case 108:
#line 546 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_multicast(cstate, yystack.l_mark[-1].i))); }
#line 2816 "grammar.c"
break;
case 109:
#line 547 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_less(cstate, yystack.l_mark[0].i))); }
#line 2821 "grammar.c"
break;
case 110:
#line 548 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_greater(cstate, yystack.l_mark[0].i))); }
#line 2826 "grammar.c"
break;
case 111:
#line 549 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_byteop(cstate, yystack.l_mark[-1].i, yystack.l_mark[-2].i, yystack.l_mark[0].i))); }
#line 2831 "grammar.c"
break;
case 112:
#line 550 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_inbound(cstate, 0))); }
#line 2836 "grammar.c"
break;
case 113:
#line 551 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_inbound(cstate, 1))); }
#line 2841 "grammar.c"
break;
case 114:
#line 552 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_vlan(cstate, (bpf_u_int32)yystack.l_mark[0].i, 1))); }
#line 2846 "grammar.c"
break;
case 115:
#line 553 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_vlan(cstate, 0, 0))); }
#line 2851 "grammar.c"
break;
case 116:
#line 554 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_mpls(cstate, (bpf_u_int32)yystack.l_mark[0].i, 1))); }
#line 2856 "grammar.c"
break;
case 117:
#line 555 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_mpls(cstate, 0, 0))); }
#line 2861 "grammar.c"
break;
case 118:
#line 556 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_pppoed(cstate))); }
#line 2866 "grammar.c"
break;
case 119:
#line 557 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_pppoes(cstate, (bpf_u_int32)yystack.l_mark[0].i, 1))); }
#line 2871 "grammar.c"
break;
case 120:
#line 558 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_pppoes(cstate, 0, 0))); }
#line 2876 "grammar.c"
break;
case 121:
#line 559 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_geneve(cstate, (bpf_u_int32)yystack.l_mark[0].i, 1))); }
#line 2881 "grammar.c"
break;
case 122:
#line 560 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_geneve(cstate, 0, 0))); }
#line 2886 "grammar.c"
break;
case 123:
#line 561 "../grammar.y"
	{ yyval.rblk = yystack.l_mark[0].rblk; }
#line 2891 "grammar.c"
break;
case 124:
#line 562 "../grammar.y"
	{ yyval.rblk = yystack.l_mark[0].rblk; }
#line 2896 "grammar.c"
break;
case 125:
#line 563 "../grammar.y"
	{ yyval.rblk = yystack.l_mark[0].rblk; }
#line 2901 "grammar.c"
break;
case 126:
#line 566 "../grammar.y"
	{ CHECK_PTR_VAL(yystack.l_mark[0].s); CHECK_PTR_VAL((yyval.rblk = gen_pf_ifname(cstate, yystack.l_mark[0].s))); }
#line 2906 "grammar.c"
break;
case 127:
#line 567 "../grammar.y"
	{ CHECK_PTR_VAL(yystack.l_mark[0].s); CHECK_PTR_VAL((yyval.rblk = gen_pf_ruleset(cstate, yystack.l_mark[0].s))); }
#line 2911 "grammar.c"
break;
case 128:
#line 568 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_pf_rnr(cstate, yystack.l_mark[0].i))); }
#line 2916 "grammar.c"
break;
case 129:
#line 569 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_pf_srnr(cstate, yystack.l_mark[0].i))); }
#line 2921 "grammar.c"
break;
case 130:
#line 570 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_pf_reason(cstate, yystack.l_mark[0].i))); }
#line 2926 "grammar.c"
break;
case 131:
#line 571 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_pf_action(cstate, yystack.l_mark[0].i))); }
#line 2931 "grammar.c"
break;
case 132:
#line 575 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_p80211_type(cstate, yystack.l_mark[-2].i | yystack.l_mark[0].i,
					IEEE80211_FC0_TYPE_MASK |
					IEEE80211_FC0_SUBTYPE_MASK)));
				}
#line 2939 "grammar.c"
break;
case 133:
#line 579 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_p80211_type(cstate, yystack.l_mark[0].i,
					IEEE80211_FC0_TYPE_MASK)));
				}
#line 2946 "grammar.c"
break;
case 134:
#line 582 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_p80211_type(cstate, yystack.l_mark[0].i,
					IEEE80211_FC0_TYPE_MASK |
					IEEE80211_FC0_SUBTYPE_MASK)));
				}
#line 2954 "grammar.c"
break;
case 135:
#line 586 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_p80211_fcdir(cstate, yystack.l_mark[0].i))); }
#line 2959 "grammar.c"
break;
case 137:
#line 590 "../grammar.y"
	{ CHECK_PTR_VAL(yystack.l_mark[0].s);
				  yyval.i = str2tok(yystack.l_mark[0].s, ieee80211_types);
				  if (yyval.i == -1) {
				  	bpf_set_error(cstate, "unknown 802.11 type name");
				  	YYABORT;
				  }
				}
#line 2970 "grammar.c"
break;
case 139:
#line 600 "../grammar.y"
	{ const struct tok *types = NULL;
				  int i;
				  CHECK_PTR_VAL(yystack.l_mark[0].s);
				  for (i = 0;; i++) {
				  	if (ieee80211_type_subtypes[i].tok == NULL) {
				  		/* Ran out of types */
						bpf_set_error(cstate, "unknown 802.11 type");
						YYABORT;
					}
					if (yystack.l_mark[-2].i == ieee80211_type_subtypes[i].type) {
						types = ieee80211_type_subtypes[i].tok;
						break;
					}
				  }

				  yyval.i = str2tok(yystack.l_mark[0].s, types);
				  if (yyval.i == -1) {
					bpf_set_error(cstate, "unknown 802.11 subtype name");
					YYABORT;
				  }
				}
#line 2995 "grammar.c"
break;
case 140:
#line 623 "../grammar.y"
	{ int i;
				  CHECK_PTR_VAL(yystack.l_mark[0].s);
				  for (i = 0;; i++) {
				  	if (ieee80211_type_subtypes[i].tok == NULL) {
				  		/* Ran out of types */
						bpf_set_error(cstate, "unknown 802.11 type name");
						YYABORT;
					}
					yyval.i = str2tok(yystack.l_mark[0].s, ieee80211_type_subtypes[i].tok);
					if (yyval.i != -1) {
						yyval.i |= ieee80211_type_subtypes[i].type;
						break;
					}
				  }
				}
#line 3014 "grammar.c"
break;
case 141:
#line 640 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_llc(cstate))); }
#line 3019 "grammar.c"
break;
case 142:
#line 641 "../grammar.y"
	{ CHECK_PTR_VAL(yystack.l_mark[0].s);
				  if (pcap_strcasecmp(yystack.l_mark[0].s, "i") == 0) {
					CHECK_PTR_VAL((yyval.rblk = gen_llc_i(cstate)));
				  } else if (pcap_strcasecmp(yystack.l_mark[0].s, "s") == 0) {
					CHECK_PTR_VAL((yyval.rblk = gen_llc_s(cstate)));
				  } else if (pcap_strcasecmp(yystack.l_mark[0].s, "u") == 0) {
					CHECK_PTR_VAL((yyval.rblk = gen_llc_u(cstate)));
				  } else {
					int subtype;

					subtype = str2tok(yystack.l_mark[0].s, llc_s_subtypes);
					if (subtype != -1) {
						CHECK_PTR_VAL((yyval.rblk = gen_llc_s_subtype(cstate, subtype)));
					} else {
						subtype = str2tok(yystack.l_mark[0].s, llc_u_subtypes);
						if (subtype == -1) {
					  		bpf_set_error(cstate, "unknown LLC type name \"%s\"", yystack.l_mark[0].s);
					  		YYABORT;
					  	}
						CHECK_PTR_VAL((yyval.rblk = gen_llc_u_subtype(cstate, subtype)));
					}
				  }
				}
#line 3046 "grammar.c"
break;
case 143:
#line 665 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.rblk = gen_llc_s_subtype(cstate, LLC_RNR))); }
#line 3051 "grammar.c"
break;
case 145:
#line 669 "../grammar.y"
	{ CHECK_PTR_VAL(yystack.l_mark[0].s);
				  if (pcap_strcasecmp(yystack.l_mark[0].s, "nods") == 0)
					yyval.i = IEEE80211_FC1_DIR_NODS;
				  else if (pcap_strcasecmp(yystack.l_mark[0].s, "tods") == 0)
					yyval.i = IEEE80211_FC1_DIR_TODS;
				  else if (pcap_strcasecmp(yystack.l_mark[0].s, "fromds") == 0)
					yyval.i = IEEE80211_FC1_DIR_FROMDS;
				  else if (pcap_strcasecmp(yystack.l_mark[0].s, "dstods") == 0)
					yyval.i = IEEE80211_FC1_DIR_DSTODS;
				  else {
					bpf_set_error(cstate, "unknown 802.11 direction");
					YYABORT;
				  }
				}
#line 3069 "grammar.c"
break;
case 146:
#line 685 "../grammar.y"
	{ yyval.i = yystack.l_mark[0].i; }
#line 3074 "grammar.c"
break;
case 147:
#line 686 "../grammar.y"
	{ CHECK_PTR_VAL(yystack.l_mark[0].s); CHECK_INT_VAL((yyval.i = pfreason_to_num(cstate, yystack.l_mark[0].s))); }
#line 3079 "grammar.c"
break;
case 148:
#line 689 "../grammar.y"
	{ CHECK_PTR_VAL(yystack.l_mark[0].s); CHECK_INT_VAL((yyval.i = pfaction_to_num(cstate, yystack.l_mark[0].s))); }
#line 3084 "grammar.c"
break;
case 149:
#line 692 "../grammar.y"
	{ yyval.i = BPF_JGT; }
#line 3089 "grammar.c"
break;
case 150:
#line 693 "../grammar.y"
	{ yyval.i = BPF_JGE; }
#line 3094 "grammar.c"
break;
case 151:
#line 694 "../grammar.y"
	{ yyval.i = BPF_JEQ; }
#line 3099 "grammar.c"
break;
case 152:
#line 696 "../grammar.y"
	{ yyval.i = BPF_JGT; }
#line 3104 "grammar.c"
break;
case 153:
#line 697 "../grammar.y"
	{ yyval.i = BPF_JGE; }
#line 3109 "grammar.c"
break;
case 154:
#line 698 "../grammar.y"
	{ yyval.i = BPF_JEQ; }
#line 3114 "grammar.c"
break;
case 155:
#line 700 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.a = gen_loadi(cstate, yystack.l_mark[0].i))); }
#line 3119 "grammar.c"
break;
case 157:
#line 703 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.a = gen_load(cstate, yystack.l_mark[-3].i, yystack.l_mark[-1].a, 1))); }
#line 3124 "grammar.c"
break;
case 158:
#line 704 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.a = gen_load(cstate, yystack.l_mark[-5].i, yystack.l_mark[-3].a, yystack.l_mark[-1].i))); }
#line 3129 "grammar.c"
break;
case 159:
#line 705 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.a = gen_arth(cstate, BPF_ADD, yystack.l_mark[-2].a, yystack.l_mark[0].a))); }
#line 3134 "grammar.c"
break;
case 160:
#line 706 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.a = gen_arth(cstate, BPF_SUB, yystack.l_mark[-2].a, yystack.l_mark[0].a))); }
#line 3139 "grammar.c"
break;
case 161:
#line 707 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.a = gen_arth(cstate, BPF_MUL, yystack.l_mark[-2].a, yystack.l_mark[0].a))); }
#line 3144 "grammar.c"
break;
case 162:
#line 708 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.a = gen_arth(cstate, BPF_DIV, yystack.l_mark[-2].a, yystack.l_mark[0].a))); }
#line 3149 "grammar.c"
break;
case 163:
#line 709 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.a = gen_arth(cstate, BPF_MOD, yystack.l_mark[-2].a, yystack.l_mark[0].a))); }
#line 3154 "grammar.c"
break;
case 164:
#line 710 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.a = gen_arth(cstate, BPF_AND, yystack.l_mark[-2].a, yystack.l_mark[0].a))); }
#line 3159 "grammar.c"
break;
case 165:
#line 711 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.a = gen_arth(cstate, BPF_OR, yystack.l_mark[-2].a, yystack.l_mark[0].a))); }
#line 3164 "grammar.c"
break;
case 166:
#line 712 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.a = gen_arth(cstate, BPF_XOR, yystack.l_mark[-2].a, yystack.l_mark[0].a))); }
#line 3169 "grammar.c"
break;
case 167:
#line 713 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.a = gen_arth(cstate, BPF_LSH, yystack.l_mark[-2].a, yystack.l_mark[0].a))); }
#line 3174 "grammar.c"
break;
case 168:
#line 714 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.a = gen_arth(cstate, BPF_RSH, yystack.l_mark[-2].a, yystack.l_mark[0].a))); }
#line 3179 "grammar.c"
break;
case 169:
#line 715 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.a = gen_neg(cstate, yystack.l_mark[0].a))); }
#line 3184 "grammar.c"
break;
case 170:
#line 716 "../grammar.y"
	{ yyval.a = yystack.l_mark[-1].a; }
#line 3189 "grammar.c"
break;
case 171:
#line 717 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.a = gen_loadlen(cstate))); }
#line 3194 "grammar.c"
break;
case 172:
#line 719 "../grammar.y"
	{ yyval.i = '&'; }
#line 3199 "grammar.c"
break;
case 173:
#line 720 "../grammar.y"
	{ yyval.i = '|'; }
#line 3204 "grammar.c"
break;
case 174:
#line 721 "../grammar.y"
	{ yyval.i = '<'; }
#line 3209 "grammar.c"
break;
case 175:
#line 722 "../grammar.y"
	{ yyval.i = '>'; }
#line 3214 "grammar.c"
break;
case 176:
#line 723 "../grammar.y"
	{ yyval.i = '='; }
#line 3219 "grammar.c"
break;
case 178:
#line 726 "../grammar.y"
	{ yyval.i = yystack.l_mark[-1].i; }
#line 3224 "grammar.c"
break;
case 179:
#line 728 "../grammar.y"
	{ yyval.i = A_LANE; }
#line 3229 "grammar.c"
break;
case 180:
#line 729 "../grammar.y"
	{ yyval.i = A_METAC;	}
#line 3234 "grammar.c"
break;
case 181:
#line 730 "../grammar.y"
	{ yyval.i = A_BCC; }
#line 3239 "grammar.c"
break;
case 182:
#line 731 "../grammar.y"
	{ yyval.i = A_OAMF4EC; }
#line 3244 "grammar.c"
break;
case 183:
#line 732 "../grammar.y"
	{ yyval.i = A_OAMF4SC; }
#line 3249 "grammar.c"
break;
case 184:
#line 733 "../grammar.y"
	{ yyval.i = A_SC; }
#line 3254 "grammar.c"
break;
case 185:
#line 734 "../grammar.y"
	{ yyval.i = A_ILMIC; }
#line 3259 "grammar.c"
break;
case 186:
#line 736 "../grammar.y"
	{ yyval.i = A_OAM; }
#line 3264 "grammar.c"
break;
case 187:
#line 737 "../grammar.y"
	{ yyval.i = A_OAMF4; }
#line 3269 "grammar.c"
break;
case 188:
#line 738 "../grammar.y"
	{ yyval.i = A_CONNECTMSG; }
#line 3274 "grammar.c"
break;
case 189:
#line 739 "../grammar.y"
	{ yyval.i = A_METACONNECT; }
#line 3279 "grammar.c"
break;
case 190:
#line 742 "../grammar.y"
	{ yyval.blk.atmfieldtype = A_VPI; }
#line 3284 "grammar.c"
break;
case 191:
#line 743 "../grammar.y"
	{ yyval.blk.atmfieldtype = A_VCI; }
#line 3289 "grammar.c"
break;
case 193:
#line 746 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.blk.b = gen_atmfield_code(cstate, yystack.l_mark[-2].blk.atmfieldtype, (bpf_int32)yystack.l_mark[0].i, (bpf_u_int32)yystack.l_mark[-1].i, 0))); }
#line 3294 "grammar.c"
break;
case 194:
#line 747 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.blk.b = gen_atmfield_code(cstate, yystack.l_mark[-2].blk.atmfieldtype, (bpf_int32)yystack.l_mark[0].i, (bpf_u_int32)yystack.l_mark[-1].i, 1))); }
#line 3299 "grammar.c"
break;
case 195:
#line 748 "../grammar.y"
	{ yyval.blk.b = yystack.l_mark[-1].blk.b; yyval.blk.q = qerr; }
#line 3304 "grammar.c"
break;
case 196:
#line 750 "../grammar.y"
	{
	yyval.blk.atmfieldtype = yystack.l_mark[-1].blk.atmfieldtype;
	if (yyval.blk.atmfieldtype == A_VPI ||
	    yyval.blk.atmfieldtype == A_VCI)
		CHECK_PTR_VAL((yyval.blk.b = gen_atmfield_code(cstate, yyval.blk.atmfieldtype, (bpf_int32) yystack.l_mark[0].i, BPF_JEQ, 0)));
	}
#line 3314 "grammar.c"
break;
case 198:
#line 758 "../grammar.y"
	{ gen_or(yystack.l_mark[-2].blk.b, yystack.l_mark[0].blk.b); yyval.blk = yystack.l_mark[0].blk; }
#line 3319 "grammar.c"
break;
case 199:
#line 761 "../grammar.y"
	{ yyval.i = M_FISU; }
#line 3324 "grammar.c"
break;
case 200:
#line 762 "../grammar.y"
	{ yyval.i = M_LSSU; }
#line 3329 "grammar.c"
break;
case 201:
#line 763 "../grammar.y"
	{ yyval.i = M_MSU; }
#line 3334 "grammar.c"
break;
case 202:
#line 764 "../grammar.y"
	{ yyval.i = MH_FISU; }
#line 3339 "grammar.c"
break;
case 203:
#line 765 "../grammar.y"
	{ yyval.i = MH_LSSU; }
#line 3344 "grammar.c"
break;
case 204:
#line 766 "../grammar.y"
	{ yyval.i = MH_MSU; }
#line 3349 "grammar.c"
break;
case 205:
#line 769 "../grammar.y"
	{ yyval.blk.mtp3fieldtype = M_SIO; }
#line 3354 "grammar.c"
break;
case 206:
#line 770 "../grammar.y"
	{ yyval.blk.mtp3fieldtype = M_OPC; }
#line 3359 "grammar.c"
break;
case 207:
#line 771 "../grammar.y"
	{ yyval.blk.mtp3fieldtype = M_DPC; }
#line 3364 "grammar.c"
break;
case 208:
#line 772 "../grammar.y"
	{ yyval.blk.mtp3fieldtype = M_SLS; }
#line 3369 "grammar.c"
break;
case 209:
#line 773 "../grammar.y"
	{ yyval.blk.mtp3fieldtype = MH_SIO; }
#line 3374 "grammar.c"
break;
case 210:
#line 774 "../grammar.y"
	{ yyval.blk.mtp3fieldtype = MH_OPC; }
#line 3379 "grammar.c"
break;
case 211:
#line 775 "../grammar.y"
	{ yyval.blk.mtp3fieldtype = MH_DPC; }
#line 3384 "grammar.c"
break;
case 212:
#line 776 "../grammar.y"
	{ yyval.blk.mtp3fieldtype = MH_SLS; }
#line 3389 "grammar.c"
break;
case 214:
#line 779 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.blk.b = gen_mtp3field_code(cstate, yystack.l_mark[-2].blk.mtp3fieldtype, (u_int)yystack.l_mark[0].i, (u_int)yystack.l_mark[-1].i, 0))); }
#line 3394 "grammar.c"
break;
case 215:
#line 780 "../grammar.y"
	{ CHECK_PTR_VAL((yyval.blk.b = gen_mtp3field_code(cstate, yystack.l_mark[-2].blk.mtp3fieldtype, (u_int)yystack.l_mark[0].i, (u_int)yystack.l_mark[-1].i, 1))); }
#line 3399 "grammar.c"
break;
case 216:
#line 781 "../grammar.y"
	{ yyval.blk.b = yystack.l_mark[-1].blk.b; yyval.blk.q = qerr; }
#line 3404 "grammar.c"
break;
case 217:
#line 783 "../grammar.y"
	{
	yyval.blk.mtp3fieldtype = yystack.l_mark[-1].blk.mtp3fieldtype;
	if (yyval.blk.mtp3fieldtype == M_SIO ||
	    yyval.blk.mtp3fieldtype == M_OPC ||
	    yyval.blk.mtp3fieldtype == M_DPC ||
	    yyval.blk.mtp3fieldtype == M_SLS ||
	    yyval.blk.mtp3fieldtype == MH_SIO ||
	    yyval.blk.mtp3fieldtype == MH_OPC ||
	    yyval.blk.mtp3fieldtype == MH_DPC ||
	    yyval.blk.mtp3fieldtype == MH_SLS)
		CHECK_PTR_VAL((yyval.blk.b = gen_mtp3field_code(cstate, yyval.blk.mtp3fieldtype, (u_int) yystack.l_mark[0].i, BPF_JEQ, 0)));
	}
#line 3420 "grammar.c"
break;
case 219:
#line 797 "../grammar.y"
	{ gen_or(yystack.l_mark[-2].blk.b, yystack.l_mark[0].blk.b); yyval.blk = yystack.l_mark[0].blk; }
#line 3425 "grammar.c"
break;
#line 3427 "grammar.c"
    default:
        break;
    }
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    yystack.p_mark -= yym;
#endif
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
        {
            fprintf(stderr, "%s[%d]: after reduction, ", YYDEBUGSTR, yydepth);
#ifdef YYSTYPE_TOSTRING
#if YYBTYACC
            if (!yytrial)
#endif /* YYBTYACC */
                fprintf(stderr, "result is <%s>, ", YYSTYPE_TOSTRING(yystos[YYFINAL], yyval));
#endif
            fprintf(stderr, "shifting from state 0 to final state %d\n", YYFINAL);
        }
#endif
        yystate = YYFINAL;
        *++yystack.s_mark = YYFINAL;
        *++yystack.l_mark = yyval;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        *++yystack.p_mark = yyloc;
#endif
        if (yychar < 0)
        {
#if YYBTYACC
            do {
            if (yylvp < yylve)
            {
                /* we're currently re-reading tokens */
                yylval = *yylvp++;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                yylloc = *yylpp++;
#endif
                yychar = *yylexp++;
                break;
            }
            if (yyps->save)
            {
                /* in trial mode; save scanner results for future parse attempts */
                if (yylvp == yylvlim)
                {   /* Enlarge lexical value queue */
                    size_t p = (size_t) (yylvp - yylvals);
                    size_t s = (size_t) (yylvlim - yylvals);

                    s += YYLVQUEUEGROWTH;
                    if ((yylexemes = (YYINT *)realloc(yylexemes, s * sizeof(YYINT))) == NULL)
                        goto yyenomem;
                    if ((yylvals   = (YYSTYPE *)realloc(yylvals, s * sizeof(YYSTYPE))) == NULL)
                        goto yyenomem;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                    if ((yylpsns   = (YYLTYPE *)realloc(yylpsns, s * sizeof(YYLTYPE))) == NULL)
                        goto yyenomem;
#endif
                    yylvp   = yylve = yylvals + p;
                    yylvlim = yylvals + s;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                    yylpp   = yylpe = yylpsns + p;
                    yylplim = yylpsns + s;
#endif
                    yylexp  = yylexemes + p;
                }
                *yylexp = (YYINT) YYLEX;
                *yylvp++ = yylval;
                yylve++;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                *yylpp++ = yylloc;
                yylpe++;
#endif
                yychar = *yylexp++;
                break;
            }
            /* normal operation, no conflict encountered */
#endif /* YYBTYACC */
            yychar = YYLEX;
#if YYBTYACC
            } while (0);
#endif /* YYBTYACC */
            if (yychar < 0) yychar = YYEOF;
#if YYDEBUG
            if (yydebug)
            {
                if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
                fprintf(stderr, "%s[%d]: state %d, reading token %d (%s)\n",
                                YYDEBUGSTR, yydepth, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == YYEOF) goto yyaccept;
        goto yyloop;
    }
    if (((yyn = yygindex[yym]) != 0) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
    {
        fprintf(stderr, "%s[%d]: after reduction, ", YYDEBUGSTR, yydepth);
#ifdef YYSTYPE_TOSTRING
#if YYBTYACC
        if (!yytrial)
#endif /* YYBTYACC */
            fprintf(stderr, "result is <%s>, ", YYSTYPE_TOSTRING(yystos[yystate], yyval));
#endif
        fprintf(stderr, "shifting from state %d to state %d\n", *yystack.s_mark, yystate);
    }
#endif
    if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    *++yystack.s_mark = (YYINT) yystate;
    *++yystack.l_mark = yyval;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    *++yystack.p_mark = yyloc;
#endif
    goto yyloop;
#if YYBTYACC

    /* Reduction declares that this path is valid. Set yypath and do a full parse */
yyvalid:
    if (yypath) YYABORT;
    while (yyps->save)
    {
        YYParseState *save = yyps->save;
        yyps->save = save->save;
        save->save = yypath;
        yypath = save;
    }
#if YYDEBUG
    if (yydebug)
        fprintf(stderr, "%s[%d]: state %d, CONFLICT trial successful, backtracking to state %d, %d tokens\n",
                        YYDEBUGSTR, yydepth, yystate, yypath->state, (int)(yylvp - yylvals - yypath->lexeme));
#endif
    if (yyerrctx)
    {
        yyFreeState(yyerrctx);
        yyerrctx = NULL;
    }
    yylvp          = yylvals + yypath->lexeme;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    yylpp          = yylpsns + yypath->lexeme;
#endif
    yylexp         = yylexemes + yypath->lexeme;
    yychar         = YYEMPTY;
    yystack.s_mark = yystack.s_base + (yypath->yystack.s_mark - yypath->yystack.s_base);
    memcpy (yystack.s_base, yypath->yystack.s_base, (size_t) (yystack.s_mark - yystack.s_base + 1) * sizeof(YYINT));
    yystack.l_mark = yystack.l_base + (yypath->yystack.l_mark - yypath->yystack.l_base);
    memcpy (yystack.l_base, yypath->yystack.l_base, (size_t) (yystack.l_mark - yystack.l_base + 1) * sizeof(YYSTYPE));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    yystack.p_mark = yystack.p_base + (yypath->yystack.p_mark - yypath->yystack.p_base);
    memcpy (yystack.p_base, yypath->yystack.p_base, (size_t) (yystack.p_mark - yystack.p_base + 1) * sizeof(YYLTYPE));
#endif
    yystate        = yypath->state;
    goto yyloop;
#endif /* YYBTYACC */

yyoverflow:
    YYERROR_CALL("yacc stack overflow");
#if YYBTYACC
    goto yyabort_nomem;
yyenomem:
    YYERROR_CALL("memory exhausted");
yyabort_nomem:
#endif /* YYBTYACC */
    yyresult = 2;
    goto yyreturn;

yyabort:
    yyresult = 1;
    goto yyreturn;

yyaccept:
#if YYBTYACC
    if (yyps->save) goto yyvalid;
#endif /* YYBTYACC */
    yyresult = 0;

yyreturn:
#if defined(YYDESTRUCT_CALL)
    if (yychar != YYEOF && yychar != YYEMPTY)
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        YYDESTRUCT_CALL("cleanup: discarding token", yychar, &yylval, &yylloc);
#else
        YYDESTRUCT_CALL("cleanup: discarding token", yychar, &yylval);
#endif /* defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED) */

    {
        YYSTYPE *pv;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        YYLTYPE *pp;

        for (pv = yystack.l_base, pp = yystack.p_base; pv <= yystack.l_mark; ++pv, ++pp)
             YYDESTRUCT_CALL("cleanup: discarding state",
                             yystos[*(yystack.s_base + (pv - yystack.l_base))], pv, pp);
#else
        for (pv = yystack.l_base; pv <= yystack.l_mark; ++pv)
             YYDESTRUCT_CALL("cleanup: discarding state",
                             yystos[*(yystack.s_base + (pv - yystack.l_base))], pv);
#endif /* defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED) */
    }
#endif /* defined(YYDESTRUCT_CALL) */

#if YYBTYACC
    if (yyerrctx)
    {
        yyFreeState(yyerrctx);
        yyerrctx = NULL;
    }
    while (yyps)
    {
        YYParseState *save = yyps;
        yyps = save->save;
        save->save = NULL;
        yyFreeState(save);
    }
    while (yypath)
    {
        YYParseState *save = yypath;
        yypath = save->save;
        save->save = NULL;
        yyFreeState(save);
    }
#endif /* YYBTYACC */
    yyfreestack(&yystack);
    return (yyresult);
}
