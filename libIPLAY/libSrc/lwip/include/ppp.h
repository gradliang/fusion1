/*****************************************************************************
* ppp.h - Network Point to Point Protocol header file.
*
* Copyright (c) 2003 by Marc Boucher, Services Informatiques (MBSI) inc.
* portions Copyright (c) 1997 Global Election Systems Inc.
*
* The authors hereby grant permission to use, copy, modify, distribute,
* and license this software and its documentation for any purpose, provided
* that existing copyright notices are retained in all copies and that this
* notice and the following disclaimer are included verbatim in any 
* distributions. No written agreement, license, or royalty fee is required
* for any of the authorized uses.
*
* THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS *AS IS* AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
* IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
* REVISION HISTORY
*
* 03-01-01 Marc Boucher <marc@mbsi.ca>
*   Ported to lwIP.
* 97-11-05 Guy Lancaster <glanca@gesn.com>, Global Election Systems Inc.
*	Original derived from BSD codes.
*****************************************************************************/

#ifndef PPP_H
#define PPP_H
#include "global612.h"
#include "mpTrace.h"
#include "typedef.h"

#include "SysConfig.h"

#include "UtilTypeDef.h"

#include "ppp_opt.h"

#ifndef PACK_STRUCT_BEGIN
#define PACK_STRUCT_BEGIN
#endif /* PACK_STRUCT_BEGIN */

#ifndef PACK_STRUCT_END
#define PACK_STRUCT_END
#endif /* PACK_STRUCT_END */

#include "sio.h"
#include "socket.h"
#include "net_nic.h"
#include "net_socket.h"
#include "net_socket2.h"

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#endif

#define BYTE_ORDER BIG_ENDIAN

#define sys_jiffies() mpx_SystemTickerGet() /* since power up. */

#if PPP_SUPPORT > 0

#ifndef PACK_STRUCT_FIELD
#define PACK_STRUCT_FIELD(x) x
#endif /* PACK_STRUCT_FIELD */

#define sys_msleep(x) TaskSleep(x)

#define pbuf_free(x) netBufFree(x)

#define pbuf_alloc(x)	netBufAlloc(x)

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */

#ifndef PBUF_POOL_BUFSIZE
#define PBUF_POOL_BUFSIZE               2048
#endif


#define LWIP_MAX(x , y)  (x) > (y) ? (x) : (y)
#define LWIP_MIN(x , y)  (x) < (y) ? (x) : (y)

#define IN_CLASSA(a)        ((((u32_t)(a)) & 0x80000000) == 0)
#define IN_CLASSA_NET       0xff000000
#define	IN_CLASSA_NSHIFT	24

#define IN_CLASSB(a)        ((((u32_t)(a)) & 0xc0000000) == 0x80000000)
#define IN_CLASSB_NET       0xffff0000

#define IN_CLASSC_NET       0xffffff00



#define IN_MULTICAST(a)     IN_CLASSD(a)

#define	IN_EXPERIMENTAL(a)	((((long int) (a)) & 0xf0000000) == 0xf0000000)
#define	IN_BADCLASS(a)		IN_EXPERIMENTAL((a))


#define IN_LOOPBACKNET      127                 /* official! */

#define IP4_ADDR(ipaddr, a,b,c,d) \
        (ipaddr)->addr = htonl(((u32_t)((a) & 0xff) << 24) | \
                               ((u32_t)((b) & 0xff) << 16) | \
                               ((u32_t)((c) & 0xff) << 8) | \
                                (u32_t)((d) & 0xff))



//add from ip_addr.h
struct ip_addr {
  u32_t addr;
};


//#define TIMEOUT(f, a, t)    AddTimerProc(t*1000, f)
//#define UNTIMEOUT(f, a)     RemoveTimerProc(f)


# ifndef _BSDTYPES_DEFINED
# ifndef __u_char_defined

/* Type definitions for BSD code. */
typedef unsigned long u_long;
typedef unsigned int u_int;
typedef unsigned short u_short;
typedef unsigned char u_char;

#endif
#endif

/*
 * Constants and structures defined by the internet system,
 * Per RFC 790, September 1981, and numerous additions.
 */

/*
 * The basic PPP frame.
 */
#define PPP_HDRLEN  4       /* octets for standard ppp header */
#define PPP_FCSLEN  2       /* octets for FCS */


/*
 * Significant octet values.
 */
#define PPP_ALLSTATIONS 0xff    /* All-Stations broadcast address */
#define PPP_UI          0x03    /* Unnumbered Information */
#define PPP_FLAG        0x7e    /* Flag Sequence */
#define PPP_ESCAPE      0x7d    /* Asynchronous Control Escape */
#define PPP_TRANS       0x20    /* Asynchronous transparency modifier */

/*
 * Protocol field values.
 */
#define PPP_IP          0x21    /* Internet Protocol */
#define PPP_AT          0x29    /* AppleTalk Protocol */
#define PPP_VJC_COMP    0x2d    /* VJ compressed TCP */
#define PPP_VJC_UNCOMP  0x2f    /* VJ uncompressed TCP */
#define PPP_COMP        0xfd    /* compressed packet */
#define PPP_IPCP        0x8021  /* IP Control Protocol */
#define PPP_ATCP        0x8029  /* AppleTalk Control Protocol */
#define PPP_CCP         0x80fd  /* Compression Control Protocol */
#define PPP_LCP         0xc021  /* Link Control Protocol */
#define PPP_PAP         0xc023  /* Password Authentication Protocol */
#define PPP_LQR         0xc025  /* Link Quality Report protocol */
#define PPP_CHAP        0xc223  /* Cryptographic Handshake Auth. Protocol */
#define PPP_CBCP        0xc029  /* Callback Control Protocol */

/*
 * Values for FCS calculations.
 */
#define PPP_INITFCS 0xffff  /* Initial FCS value */
#define PPP_GOODFCS 0xf0b8  /* Good final FCS value */
#define PPP_FCS(fcs, c) (((fcs) >> 8) ^ fcstab[((fcs) ^ (c)) & 0xff])


/*
 * Extended asyncmap - allows any character to be escaped.
 */
typedef u_char  ext_accm[32];

/*
 * What to do with network protocol (NP) packets.
 */
enum NPmode {
    NPMODE_PASS,        /* pass the packet through */
    NPMODE_DROP,        /* silently drop the packet */
    NPMODE_ERROR,       /* return an error */
    NPMODE_QUEUE        /* save it up for later. */
};

/*
 * Inline versions of get/put char/short/long.
 * Pointer is advanced; we assume that both arguments
 * are lvalues and will already be in registers.
 * cp MUST be u_char *.
 */
#define GETCHAR(c, cp) { \
    (c) = *(cp)++; \
}
#define PUTCHAR(c, cp) { \
    *(cp)++ = (u_char) (c); \
}


#define GETSHORT(s, cp) { \
    (s) = *(cp); (cp)++; (s) <<= 8; \
    (s) |= *(cp); (cp)++; \
}
#define PUTSHORT(s, cp) { \
    *(cp)++ = (u_char) ((s) >> 8); \
    *(cp)++ = (u_char) (s); \
}

#define GETLONG(l, cp) { \
    (l) = *(cp); (cp)++; (l) <<= 8; \
    (l) |= *(cp); (cp)++; (l) <<= 8; \
    (l) |= *(cp); (cp)++; (l) <<= 8; \
    (l) |= *(cp); (cp)++; \
}
#define PUTLONG(l, cp) { \
    *(cp)++ = (u_char) ((l) >> 24); \
    *(cp)++ = (u_char) ((l) >> 16); \
    *(cp)++ = (u_char) ((l) >> 8); \
    *(cp)++ = (u_char) (l); \
}


#define INCPTR(n, cp)   ((cp) += (n))
#define DECPTR(n, cp)   ((cp) -= (n))

#define BCMP(s0, s1, l)     memcmp((u_char *)(s0), (u_char *)(s1), (l))
#define BCOPY(s, d, l)      memcpy((d), (s), (l))
#define BZERO(s, n)         memset(s, 0, n)
#if PPP_DEBUG
#define PRINTMSG(m, l)  { m[l] = '\0'; ppp_trace(LOG_INFO, "Remote message: %s\n", m); }
#else
#define PRINTMSG(m, l)
#endif

/*
 * MAKEHEADER - Add PPP Header fields to a packet.
 */
#define MAKEHEADER(p, t) { \
    PUTCHAR(PPP_ALLSTATIONS, p); \
    PUTCHAR(PPP_UI, p); \
    PUTSHORT(t, p); }

/*************************
*** PUBLIC DEFINITIONS ***
*************************/

/* Error codes. */
#define PPPERR_NONE 0				/* No error. */
#define PPPERR_PARAM -1				/* Invalid parameter. */
#define PPPERR_OPEN -2				/* Unable to open PPP session. */
#define PPPERR_DEVICE -3			/* Invalid I/O device for PPP. */
#define PPPERR_ALLOC -4				/* Unable to allocate resources. */
#define PPPERR_USER -5				/* User interrupt. */
#define PPPERR_CONNECT -6			/* Connection lost. */
#define PPPERR_AUTHFAIL -7			/* Failed authentication challenge. */
#define PPPERR_PROTOCOL -8			/* Failed to meet protocol. */

/*
 * PPP IOCTL commands.
 */
/*
 * Get the up status - 0 for down, non-zero for up.  The argument must
 * point to an int.
 */
#define PPPCTLG_UPSTATUS 100	/* Get the up status - 0 down else up */
#define PPPCTLS_ERRCODE 101		/* Set the error code */
#define PPPCTLG_ERRCODE 102		/* Get the error code */
#define	PPPCTLG_FD		103		/* Get the fd associated with the ppp */

/************************
*** PUBLIC DATA TYPES ***
************************/

/*
 * The following struct gives the addresses of procedures to call
 * for a particular protocol.
 */
struct protent {
    u_short protocol;       /* PPP protocol number */
    /* Initialization procedure */
    void (*init) (int unit);
    /* Process a received packet */
    void (*input) (int unit, u_char *pkt, int len);
    /* Process a received protocol-reject */
    void (*protrej) (int unit);
    /* Lower layer has come up */
    void (*lowerup) (int unit);
    /* Lower layer has gone down */
    void (*lowerdown) (int unit);
    /* Open the protocol */
    void (*open) (int unit);
    /* Close the protocol */
    void (*close) (int unit, char *reason);
#if 0
    /* Print a packet in readable form */
    int  (*printpkt) (u_char *pkt, int len,
              void (*printer) (void *, char *, ...),
              void *arg);
    /* Process a received data packet */
    void (*datainput) (int unit, u_char *pkt, int len);
#endif
    int  enabled_flag;      /* 0 iff protocol is disabled */
    char *name;         /* Text name of protocol */
#if 0
    /* Check requested options, assign defaults */
    void (*check_options) (u_long);
    /* Configure interface for demand-dial */
    int  (*demand_conf) (int unit);
    /* Say whether to bring up link for this pkt */
    int  (*active_pkt) (u_char *pkt, int len);
#endif
};

/*
 * The following structure records the time in seconds since
 * the last NP packet was sent or received.
 */
struct ppp_idle {
    u_short xmit_idle;      /* seconds since last NP packet sent */
    u_short recv_idle;      /* seconds since last NP packet received */
};

struct ppp_settings {

	u_int  disable_defaultip : 1;   /* Don't use hostname for default IP addrs */
	u_int  auth_required : 1;      /* Peer is required to authenticate */
	u_int  explicit_remote : 1;    /* remote_name specified with remotename opt */
	u_int  refuse_pap : 1;         /* Don't wanna auth. ourselves with PAP */
	u_int  refuse_chap : 1;        /* Don't wanna auth. ourselves with CHAP */
	u_int  usehostname : 1;        /* Use hostname for our_name */
	u_int  usepeerdns : 1;         /* Ask peer for DNS adds */

	u_short idle_time_limit; /* Shut down link if idle for this long */
	int  maxconnect;         /* Maximum connect time (seconds) */

	char user[MAXNAMELEN + 1];/* Username for PAP */
	char passwd[MAXSECRETLEN + 1];           /* Password for PAP, secret for CHAP */
	char our_name[MAXNAMELEN + 1];         /* Our name for authentication purposes */
	char remote_name[MAXNAMELEN + 1];      /* Peer's name for authentication */
};


struct ppp_addrs {
    struct ip_addr our_ipaddr, his_ipaddr, netmask, dns1, dns2;
};

/*****************************
*** PUBLIC DATA STRUCTURES ***
*****************************/
/* Buffers for outgoing packets. */
extern u_char outpacket_buf[NUM_PPP][PPP_MRU+PPP_HDRLEN];

extern struct ppp_settings ppp_settings;

extern struct protent *ppp_protocols[];/* Table of pointers to supported protocols */

/***********************
*** PUBLIC FUNCTIONS ***
***********************/

/* Initialize the PPP subsystem. */
void pppInit(void);

/* Warning: Using PPPAUTHTYPE_ANY might have security consequences.
 * RFC 1994 says:
 *
 * In practice, within or associated with each PPP server, there is a
 * database which associates "user" names with authentication
 * information ("secrets").  It is not anticipated that a particular
 * named user would be authenticated by multiple methods.  This would
 * make the user vulnerable to attacks which negotiate the least secure
 * method from among a set (such as PAP rather than CHAP).  If the same
 * secret was used, PAP would reveal the secret to be used later with
 * CHAP.
 *
 * Instead, for each user name there should be an indication of exactly
 * one method used to authenticate that user name.  If a user needs to
 * make use of different authentication methods under different
 * circumstances, then distinct user names SHOULD be employed, each of
 * which identifies exactly one authentication method.
 *
 */
enum pppAuthType {
    PPPAUTHTYPE_NONE,
    PPPAUTHTYPE_ANY,
    PPPAUTHTYPE_PAP,
    PPPAUTHTYPE_CHAP
};

void pppSetAuth(enum pppAuthType authType, const char *user, const char *passwd);


/* Get appropriate netmask for address */
u32_t GetMask (u32_t); 

#define LWIP_ASSERT(x,y)


#define integrated_board	1
#define MMS_Enable	0

#define Signal_Test			0x00000001
#define PPP_connect			0x00000002
#define PPP_disconnect		0x00000004
#define PPP_Shutdown		0x00000008
#define RegisterState_Test	0x00000010

#define EdgeStatus_Read		0x00000020          /* Read edge network status */
#define Ceer_Read			0x00000040
#define SMS_sync			0x00000080
#define POP3_test			0x00000100
#define EEVENT_TIMEOUT		0x00000400
#define EEVENT_REQ_GPRS		0x00000800
#define EEVENT_REQ_SMS		0x00001000
#define EEVENT_PPP_CONNECTED	0x00002000
#define EEVENT_PPP_DISCONNECTED	0x00004000
#define EEVENT_GSM_REGISTER_STATUS	0x00008000

/* for WWAN call control */
#define PEVENT_MODEM_STATUS	    0x00010000
#define PEVENT_PPP_STATUS	    0x00020000
#define PEVENT_TIMEOUT		    0x00040000
#define PEVENT_PPP_CONNECT	    0x00080000
#define PEVENT_PPP_DISCONNECT	0x00100000

#define EDGE_TEST	0


enum PPP_STATE{
	PPP_STATE_DOWN,
	PPP_STATE_PROCESS,
	PPP_STATE_UP
};

void EDGE_init();
int  EdgeConnect();
void EdgeDisconnect();
void EdgeShutDown(void);
void EdgeGetRegisterState(void);
void EdgeGetImeiCcid(BYTE ** _ccid, BYTE** _imei);


#endif /* PPP_SUPPORT */

#endif /* PPP_H */


