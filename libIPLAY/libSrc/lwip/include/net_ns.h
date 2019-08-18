#ifndef __NET_NS_H
#define __NET_NS_H

#include "net_socket.h"
#include "net_packet.h"
#include "error.h"

/*
 * Define constants based on RFC 883, RFC 1034, RFC 1035
 */
#define NS_PACKETSZ	512	/* maximum packet size */
#define NS_MAXDNAME	(255+1)	/* maximum domain name */
#define NS_MAXCDNAME	255	/* maximum compressed domain name */
#define NS_MAXLABEL	63	/* maximum length of domain label */
#define NS_HFIXEDSZ	12	/* #/bytes of fixed data in header */
#define NS_QFIXEDSZ	4	/* #/bytes of fixed data in query */
#define NS_RRFIXEDSZ	10	/* #/bytes of fixed data in r record */
#define NS_INT32SZ	4	/* #/bytes of data in a u_int32_t */
#define NS_INT16SZ	2	/* #/bytes of data in a u_int16_t */
#define NS_INT8SZ	1	/* #/bytes of data in a u_int8_t */
#define NS_INADDRSZ	4	/* IPv4 T_A */
#define NS_IN6ADDRSZ	16	/* IPv6 T_AAAA */
#define NS_CMPRSFLGS	0xc0	/* Flag bits indicating name compression. */
#define NS_DEFAULTPORT	53	/* For both TCP and UDP. */

#if PACKETSZ > 65536
#define MAXPACKET	PACKETSZ
#else
#define MAXPACKET	65536
#endif

/*
 * These can be expanded with synonyms, just keep ns_parse.c:ns_parserecord()
 * in synch with it.
 */
typedef enum __ns_sect {
	ns_s_qd = 0,		/* Query: Question. */
	ns_s_zn = 0,		/* Update: Zone. */
	ns_s_an = 1,		/* Query: Answer. */
	ns_s_pr = 1,		/* Update: Prerequisites. */
	ns_s_ns = 2,		/* Query: Name servers. */
	ns_s_ud = 2,		/* Update: Update. */
	ns_s_ar = 3,		/* Query|Update: Additional records. */
	ns_s_max = 4
} ns_sect;

/*
 * This is a message handle.  It is caller allocated and has no dynamic data.
 * This structure is intended to be opaque to all but ns_parse.c, thus the
 * leading _'s on the member names.  Use the accessor functions, not the _'s.
 */
typedef struct __ns_msg {
	const U08	*_msg, *_eom;
	U16	_id, _flags, _counts[ns_s_max];
	const U08	*_sections[ns_s_max];
	ns_sect		_sect;
	int		_rrnum;
	const U08	*_ptr;
} ns_msg;

/*
 * These don't have to be in the same order as in the packet flags word,
 * and they can even overlap in some cases, but they will need to be kept
 * in synch with ns_parse.c:ns_flagdata[].
 */
typedef enum __ns_flag {
	ns_f_qr,		/* Question/Response. */
	ns_f_opcode,		/* Operation code. */
	ns_f_aa,		/* Authoritative Answer. */
	ns_f_tc,		/* Truncation occurred. */
	ns_f_rd,		/* Recursion Desired. */
	ns_f_ra,		/* Recursion Available. */
	ns_f_z,			/* MBZ. */
	ns_f_ad,		/* Authentic Data (DNSSEC). */
	ns_f_cd,		/* Checking Disabled (DNSSEC). */
	ns_f_rcode,		/* Response code. */
	ns_f_max
} ns_flag;

/*
 * Currently defined opcodes.
 */
typedef enum __ns_opcode {
	ns_o_query = 0,		/* Standard query. */
	ns_o_iquery = 1,	/* Inverse query (deprecated/unsupported). */
	ns_o_status = 2,	/* Name server status query (unsupported). */
				/* Opcode 3 is undefined/reserved. */
	ns_o_notify = 4,	/* Zone change notification. */
	ns_o_update = 5,	/* Zone update message. */
	ns_o_max = 6
} ns_opcode;

/*
 * Currently defined response codes.
 */
typedef	enum __ns_rcode {
	ns_r_noerror = 0,	/* No error occurred. */
	ns_r_formerr = 1,	/* Format error. */
	ns_r_servfail = 2,	/* Server failure. */
	ns_r_nxdomain = 3,	/* Name error. */
	ns_r_notimpl = 4,	/* Unimplemented. */
	ns_r_refused = 5,	/* Operation refused. */
	/* these are for BIND_UPDATE */
	ns_r_yxdomain = 6,	/* Name exists */
	ns_r_yxrrset = 7,	/* RRset exists */
	ns_r_nxrrset = 8,	/* RRset does not exist */
	ns_r_notauth = 9,	/* Not authoritative for zone */
	ns_r_notzone = 10,	/* Zone of record different from zone section */
	ns_r_max = 11,
	/* The following are TSIG extended errors */
	ns_r_badsig = 16,
	ns_r_badkey = 17,
	ns_r_badtime = 18
} ns_rcode;


typedef struct {
	unsigned	id :16;		/* query identification number */
			/* fields in third byte */
	unsigned	qr: 1;		/* response flag */
	unsigned	opcode: 4;	/* purpose of message */
	unsigned	aa: 1;		/* authoritive answer */
	unsigned	tc: 1;		/* truncated message */
	unsigned	rd: 1;		/* recursion desired */
			/* fields in fourth byte */
	unsigned	ra: 1;		/* recursion available */
	unsigned	unused :1;	/* unused bits (MBZ as of 4.9.3a3) */
	unsigned	ad: 1;		/* authentic data from named */
	unsigned	cd: 1;		/* checking disabled by resolver */
	unsigned	rcode :4;	/* response code */
			/* remaining bytes */
	unsigned	qdcount :16;	/* number of question entries */
	unsigned	ancount :16;	/* number of answer entries */
	unsigned	nscount :16;	/* number of authority entries */
	unsigned	arcount :16;	/* number of resource entries */
} NS_QUERY_HEADER;


struct hostent * mpx_gethostbyname(S08* name);

#endif
