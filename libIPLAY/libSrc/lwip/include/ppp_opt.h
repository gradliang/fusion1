#include "global612.h"

#ifndef __PPP_OPT_H__
#define __PPP_OPT_H__

/* ---------- PPP options ---------- */

#ifndef PPP_SUPPORT
	#if PPP_ENABLE
#define PPP_SUPPORT                     1      /* Set for PPP */
	#else
		#define PPP_SUPPORT				0
	#endif
#endif

#if PPP_SUPPORT 

#define NUM_PPP                         1      /* Max PPP sessions. */



#ifndef PAP_SUPPORT
#define PAP_SUPPORT                     1      /* Set for PAP. */
#endif

#ifndef CHAP_SUPPORT
#define CHAP_SUPPORT                    1      /* Set for CHAP. */
#endif

#define MSCHAP_SUPPORT                  0      /* Set for MSCHAP (NOT FUNCTIONAL!) */
#define CBCP_SUPPORT                    0      /* Set for CBCP (NOT FUNCTIONAL!) */
#define CCP_SUPPORT                     0      /* Set for CCP (NOT FUNCTIONAL!) */

#ifndef VJ_SUPPORT
#define VJ_SUPPORT                      0      /* Set for VJ header compression. */
#endif

#ifndef MD5_SUPPORT
#define MD5_SUPPORT                     1      /* Set for MD5 (see also CHAP) */
#endif


/*
 * Timeouts.
 */
#define FSM_DEFTIMEOUT                  6       /* Timeout time in seconds */
#define FSM_DEFMAXTERMREQS              2       /* Maximum Terminate-Request transmissions */
#define FSM_DEFMAXCONFREQS              10      /* Maximum Configure-Request transmissions */
#define FSM_DEFMAXNAKLOOPS              5       /* Maximum number of nak loops */

#define UPAP_DEFTIMEOUT                 6       /* Timeout (seconds) for retransmitting req */
#define UPAP_DEFREQTIME                 30      /* Time to wait for auth-req from peer */

#define CHAP_DEFTIMEOUT                 6       /* Timeout time in seconds */
#define CHAP_DEFTRANSMITS               10      /* max # times to send challenge */


/* Interval in seconds between keepalive echo requests, 0 to disable. */
#if 1
#define LCP_ECHOINTERVAL                0
#else
#define LCP_ECHOINTERVAL                10
#endif

/* Number of unanswered echo requests before failure. */
#define LCP_MAXECHOFAILS                3

/* Max Xmit idle time (in jiffies) before resend flag char. */
#define PPP_MAXIDLEFLAG                 100

/*
 * Packet sizes
 *
 * Note - lcp shouldn't be allowed to negotiate stuff outside these
 *    limits.  See lcp.h in the pppd directory.
 * (XXX - these constants should simply be shared by lcp.c instead
 *    of living in lcp.h)
 */
#define PPP_MTU                         1500     /* Default MTU (size of Info field) */
#if 0
#define PPP_MAXMTU  65535 - (PPP_HDRLEN + PPP_FCSLEN)
#else
#define PPP_MAXMTU                      1500 /* Largest MTU we allow */
#endif
#define PPP_MINMTU                      64
#define PPP_MRU                         1500     /* default MRU = max length of info field */
#define PPP_MAXMRU                      1500     /* Largest MRU we allow */
#define PPP_DEFMRU                      296             /* Try for this */
#define PPP_MINMRU                      128             /* No MRUs below this */


#define MAXNAMELEN                      256     /* max length of hostname or name for auth */
#define MAXSECRETLEN                    256     /* max length of password or secret */

#endif /* PPP_SUPPORT */

/** flag for LWIP_DEBUGF to enable that debug message */
#define DBG_ON  0x80U
/** flag for LWIP_DEBUGF to disable that debug message */
#define DBG_OFF 0x00U


#ifndef PPP_DEBUG 
#define PPP_DEBUG                       DBG_OFF
#endif



#endif /* __PPP_OPT_H__ */


