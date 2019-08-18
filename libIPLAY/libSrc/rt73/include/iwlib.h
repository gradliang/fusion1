/*
 *	Wireless Tools
 *
 *		Jean II - HPLB 97->99 - HPL 99->04
 *
 * Common header for the Wireless Extension library...
 *
 * This file is released under the GPL license.
 *     Copyright (c) 1997-2004 Jean Tourrilhes <jt@hpl.hp.com>
 */

#ifndef IWLIB_H
#define IWLIB_H

/*#include "CHANGELOG.h"*/

/***************************** INCLUDES *****************************/
/* Proposed by Dr. Michael Rietz <rietz@mail.amps.de>, 27.3.2 */
/* If this works for all, it might be more stable on the long term - Jean II */

/* Private copy of Wireless extensions */
#include "wireless.h"
//#include "socket.h"

#define	IFNAMSIZ	16

/****************************** DEBUG ******************************/

/************************ CONSTANTS & MACROS ************************/
/*iwlist define */
#define IW_SCANNING   0//"{ "scanning",		print_scanning_info,	0, 5 },
#define IW_FREQUENCY  1// { "frequency",	print_freq_info,	0, 0 },
#define IW_CHANNEL    2// { "channel",		print_freq_info,	0, 0 },
#define IW_BITRATE    3// { "bitrate",		print_bitrate_info,	0, 0 },
#define IW_RATE       4// { "rate",		print_bitrate_info,	0, 0 },
#define IW_ENCRYPTION 5// { "encryption",	print_keys_info,	0, 0 },
#define IW_KEY        6// { "key",		print_keys_info,	0, 0 },
#define IW_POWER      7// { "power",		print_pm_info,		0, 0 },
#define IW_TXPOWER    8// { "txpower",		print_txpower_info,	0, 0 },
#define IW_RETRY      9// { "retry",		print_retry_info,	0, 0 },
#define IW_APINFO     10//{ "ap",		print_ap_info,		0, 0 },
                        //{ "accesspoints",	print_ap_info,		0, 0 },
#define IW_PEERS      11//{ "peers",		print_ap_info,		0, 0 },
#define IW_EVENT      12//{ "event",		print_event_capa_info,	0, 0 },

/* ARP protocol HARDWARE identifiers. */
#define ARPHRD_NETROM	0		/* from KA9Q: NET/ROM pseudo	*/
#define ARPHRD_ETHER 	1		/* Ethernet 10Mbps		*/
#define	ARPHRD_EETHER	2		/* Experimental Ethernet	*/
#define	ARPHRD_AX25	3		/* AX.25 Level 2		*/
#define	ARPHRD_PRONET	4		/* PROnet token ring		*/
#define	ARPHRD_CHAOS	5		/* Chaosnet			*/
#define	ARPHRD_IEEE802	6		/* IEEE 802.2 Ethernet/TR/TB	*/
#define	ARPHRD_ARCNET	7		/* ARCnet			*/
#define	ARPHRD_APPLETLK	8		/* APPLEtalk			*/
#define ARPHRD_DLCI	15		/* Frame Relay DLCI		*/
#define ARPHRD_ATM	19		/* ATM 				*/
#define ARPHRD_METRICOM	23		/* Metricom STRIP (new IANA id)	*/
#define	ARPHRD_IEEE1394	24		/* IEEE 1394 IPv4 - RFC 2734	*/
#define ARPHRD_EUI64	27		/* EUI-64                       */
#define ARPHRD_INFINIBAND 32		/* InfiniBand			*/

/* Various versions information */
/* Recommended Wireless Extension version */
#define WE_VERSION	17
/* Version of Wireless Tools */
#define WT_VERSION	27

/* Paths */
#define PROC_NET_WIRELESS	"/proc/net/wireless"
#define PROC_NET_DEV		"/proc/net/dev"

/* Some usefull constants */
#define KILO	1e3
#define MEGA	1e6
#define GIGA	1e9
/* For doing log10/exp10 without libm */
#define LOG10_MAGIC	1.25892541179

/* Backward compatibility for Wireless Extension 9 */
#ifndef IW_POWER_MODIFIER
#define IW_POWER_MODIFIER	0x000F	/* Modify a parameter */
#define IW_POWER_MIN		0x0001	/* Value is a minimum  */
#define IW_POWER_MAX		0x0002	/* Value is a maximum */
#define IW_POWER_RELATIVE	0x0004	/* Value is not in seconds/ms/us */
#endif /* IW_POWER_MODIFIER */

#ifndef IW_ENCODE_NOKEY
#define IW_ENCODE_NOKEY         0x0800  /* Key is write only, so not here */
#define IW_ENCODE_MODE		0xF000	/* Modes defined below */
#endif /* IW_ENCODE_NOKEY */
#ifndef IW_ENCODE_TEMP
#define IW_ENCODE_TEMP		0x0400  /* Temporary key */
#endif /* IW_ENCODE_TEMP */

/* More backward compatibility */
#ifndef SIOCSIWCOMMIT
#define SIOCSIWCOMMIT	SIOCSIWNAME
#endif /* SIOCSIWCOMMIT */

/* Still more backward compatibility */
#ifndef IW_FREQ_FIXED
#define IW_FREQ_FIXED	0x01
#endif /* IW_FREQ_FIXED */

/****************************** TYPES ******************************/

/* Shortcuts */
typedef struct iw_statistics	iwstats;
typedef struct iw_range		iwrange;
typedef struct iw_param		iwparam;
typedef struct iw_freq		iwfreq;
typedef struct iw_quality	iwqual;
typedef struct iw_priv_args	iwprivargs;
//typedef struct sockaddr		sockaddr;

/* Structure for storing all wireless information for each device
 * This is a cut down version of the one above, containing only
 * the things *truly* needed to configure a card.
 * Don't add other junk, I'll remove it... */
typedef struct wireless_config
{
  char		name[IFNAMSIZ + 1];	/* Wireless/protocol name */
  int		has_nwid;
  iwparam	nwid;			/* Network ID */
  int		has_freq;
  int	freq;			/* Frequency/channel */
  int		freq_flags;
  int		has_key;
  unsigned char	key[IW_ENCODING_TOKEN_MAX];	/* Encoding key used */
  int		key_size;		/* Number of bytes */
  int		key_flags;		/* Various flags */
  int		has_essid;
  int		essid_on;
  char		essid[IW_ESSID_MAX_SIZE + 1];	/* ESSID (extended network) */
  int		has_mode;
  int		mode;			/* Operation mode */
} wireless_config;

/* Structure for storing all wireless information for each device
 * This is pretty exhaustive... */
typedef struct wireless_info
{
  struct wireless_config	b;	/* Basic information */

  int		has_sens;
  iwparam	sens;			/* sensitivity */
  int		has_nickname;
  char		nickname[IW_ESSID_MAX_SIZE + 1]; /* NickName */
  int		has_ap_addr;
//  sockaddr	ap_addr;		/* Access point address */
  int		has_bitrate;
  iwparam	bitrate;		/* Bit rate in bps */
  int		has_rts;
  iwparam	rts;			/* RTS threshold in bytes */
  int		has_frag;
  iwparam	frag;			/* Fragmentation threshold in bytes */
  int		has_power;
  iwparam	power;			/* Power management parameters */
  int		has_txpower;
  iwparam	txpower;		/* Transmit Power in dBm */
  int		has_retry;
  iwparam	retry;			/* Retry limit or lifetime */

  /* Stats */
  iwstats	stats;
  int		has_stats;
  iwrange	range;
  int		has_range;
} wireless_info;

/* Structure for storing an entry of a wireless scan.
 * This is only a subset of all possible information, the flexible
 * structure of scan results make it impossible to capture all
 * information in such a static structure. */
typedef struct wireless_scan
{
  /* Linked list */
  struct wireless_scan *	next;

  /* Cell identifiaction */
  int		has_ap_addr;
  //sockaddr	ap_addr;		/* Access point address */

  /* Other information */
  struct wireless_config	b;	/* Basic information */
  iwstats	stats;			/* Signal strength */
  int		has_stats;
} wireless_scan;

/*
 * Context used for non-blocking scan.
 */
typedef struct wireless_scan_head
{
  wireless_scan *	result;		/* Result of the scan */
  int			retry;		/* Retry level */
} wireless_scan_head;

/* Structure used for parsing event streams, such as Wireless Events
 * and scan results */
typedef struct stream_descr
{
  char *	end;		/* End of the stream */
  char *	current;	/* Current event in stream of events */
  char *	value;		/* Current value in event */
} stream_descr;

/* Prototype for handling display of each single interface on the
 * system - see iw_enum_devices() */
typedef int (*iw_enum_handler)(int	skfd,
			       char *	ifname,
			       char *	args[],
			       int	count);
			       
struct ether_addr{
        unsigned char ether_addr_octet[6];
};			       


/**************************** PROTOTYPES ****************************/
/*
 * All the functions in iwcommon.c
 */

/* ---------------------- SOCKET SUBROUTINES -----------------------*/
int
	iw_sockets_open(void);
void
	iw_enum_devices(int		skfd,
			iw_enum_handler fn,
			char *		args[],
			int		count);
/* --------------------- WIRELESS SUBROUTINES ----------------------*/
int
	iw_get_kernel_we_version(void);
int
	iw_print_version_info(const char *	toolname);
int
	iw_get_range_info(int		skfd,
			  const char *	ifname,
			  iwrange *	range);
int
	iw_get_priv_info(int		skfd,
			 const char *	ifname,
			 iwprivargs **	ppriv);
int
	iw_get_basic_config(int			skfd,
			    const char *	ifname,
			    wireless_config *	info);
int
	iw_set_basic_config(int			skfd,
			    const char *	ifname,
			    wireless_config *	info);
/* --------------------- PROTOCOL SUBROUTINES --------------------- */
int
	iw_protocol_compare(const char *	protocol1,
			    const char *	protocol2);
/* -------------------- FREQUENCY SUBROUTINES --------------------- */
void
	iw_float2freq(double	in,
		      iwfreq *	out);
double
	iw_freq2float(iwfreq *	in);
void
	iw_print_freq_value(char *	buffer,
			    int		buflen,
			    double	freq);
void
	iw_print_freq(char *	buffer,
		      int	buflen,
		      double	freq,
		      int	channel,
		      int	freq_flags);
int
	iw_freq_to_channel(double			freq,
			   const struct iw_range *	range);
int
	iw_channel_to_freq(int				channel,
			   double *			pfreq,
			   const struct iw_range *	range);
void
	iw_print_bitrate(char *	buffer,
			 int	buflen,
			 int	bitrate);
/* ---------------------- POWER SUBROUTINES ----------------------- */
int
	iw_dbm2mwatt(int	in);
int
	iw_mwatt2dbm(int	in);
void
	iw_print_txpower(char *			buffer,
			 int			buflen,
			 struct iw_param *	txpower);
/* -------------------- STATISTICS SUBROUTINES -------------------- */
int
	iw_get_stats(int		skfd,
		     const char *	ifname,
		     iwstats *		stats,
		     const iwrange *	range,
		     int		has_range);
void
	iw_print_stats(char *		buffer,
		       int		buflen,
		       const iwqual *	qual,
		       const iwrange *	range,
		       int		has_range);
/* --------------------- ENCODING SUBROUTINES --------------------- */
void
	iw_print_key(char *			buffer,
		     int			buflen,
		     const unsigned char *	key,
		     int			key_size,
		     int			key_flags);
int
	iw_in_key(const char *		input,
		  unsigned char *	key);
int
	iw_in_key_full(int		skfd,
		       const char *	ifname,
		       const char *	input,
		       unsigned char *	key,
		       __u16 *		flags);
/* ----------------- POWER MANAGEMENT SUBROUTINES ----------------- */
void
	iw_print_pm_value(char *	buffer,
			  int		buflen,
			  int		value,
			  int		flags);
void
	iw_print_pm_mode(char *		buffer,
			 int		buflen,
			 int		flags);
/* --------------- RETRY LIMIT/LIFETIME SUBROUTINES --------------- */
void
	iw_print_retry_value(char *	buffer,
			     int	buflen,
			     int	value,
			     int	flags);
/* ----------------------- TIME SUBROUTINES ----------------------- */
#if 0 //cj
void
	iw_print_timeval(char *			buffer,
			 int			buflen,
			 const struct timeval *	time);
#endif
			 
/* --------------------- ADDRESS SUBROUTINES ---------------------- */
int
	iw_check_mac_addr_type(int	skfd,
			       char *	ifname);
int
	iw_check_if_addr_type(int	skfd,
			      char *	ifname);

int
	iw_check_addr_type(int		skfd,
			   char *	ifname);

int
	iw_get_mac_addr(int			skfd,
			const char *		ifname,
			struct ether_addr *	eth,
			unsigned short *	ptype);
void
	iw_ether_ntop(const struct ether_addr* eth, char* buf);
char*
	iw_ether_ntoa(const struct ether_addr* eth);
int
	iw_ether_aton(const char* bufp, struct ether_addr* eth);
//int
//	iw_in_inet(char *bufp, struct sockaddr *sap);
//int
//	iw_in_addr(int			skfd,
//		   char *		ifname,
//		   char *		bufp,
//		   struct sockaddr *	sap);
/* ----------------------- MISC SUBROUTINES ------------------------ */
int
	iw_get_priv_size(int		args);

/* ---------------------- EVENT SUBROUTINES ---------------------- */
void
	iw_init_event_stream(struct stream_descr *	stream,
			     char *			data,
			     int			len);
int
	iw_extract_event_stream(struct stream_descr *	stream,
				struct iw_event *	iwe,
				int			we_version);
/* --------------------- SCANNING SUBROUTINES --------------------- */
int
	iw_process_scan(int			skfd,
			char *			ifname,
			int			we_version,
			wireless_scan_head *	context);
int
	iw_scan(int			skfd,
		char *			ifname,
		int			we_version,
		wireless_scan_head *	context);

/**************************** VARIABLES ****************************/

/* Modes as human readable strings */
//extern const char * const	iw_operation_mode[];
#define IW_NUM_OPER_MODE	7

/************************* INLINE FUNTIONS *************************/
/*
 * Functions that are so simple that it's more efficient inlining them
 */

/*
 * Note : I've defined wrapper for the ioctl request so that
 * it will be easier to migrate to other kernel API if needed
 */

/*------------------------------------------------------------------*/
/*
 * Wrapper to push some Wireless Parameter in the driver
 */
static inline int
iw_set_ext(int			skfd,		/* Socket to the kernel */
	   const char *		ifname,		/* Device name */
	   int			request,	/* WE ID */
	   struct iwreq *	pwrq)		/* Fixed part of the request */
{
  /* Set device name */
  //strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
  /* Do the request */
  return(rt_ioctl(skfd, request, pwrq));
}

/*------------------------------------------------------------------*/
/*
 * Wrapper to extract some Wireless Parameter out of the driver
 */
static inline int
iw_get_ext(int			skfd,		/* Socket to the kernel */
	   const char *		ifname,		/* Device name */
	   int			request,	/* WE ID */
	   struct iwreq *	pwrq)		/* Fixed part of the request */
{
  /* Set device name */
  //strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
  /* Do the request */
  //return(ioctl(skfd, request,pwrq));
  
  return 0;
}

/*------------------------------------------------------------------*/
/*
 * Close the socket used for ioctl.
 */
static inline void
iw_sockets_close(int	skfd)
{
  close(skfd);
}

/*------------------------------------------------------------------*/
/* Backwards compatability
 * Actually, those form are much easier to use when dealing with
 * struct sockaddr... */
static inline char*
iw_pr_ether(char* bufp, const unsigned char* addr)
{
  iw_ether_ntop((const struct ether_addr *) addr, bufp);
  return bufp;
}
/* Backwards compatability */
//static inline int
//iw_in_ether(const char *bufp, struct sockaddr *sap)
//{
//  sap->sa_family = ARPHRD_ETHER;
//  return iw_ether_aton(bufp, (struct ether_addr *) sap->sa_data) ? 0 : -1;
//}

/*------------------------------------------------------------------*/
/*
 * Create an Ethernet broadcast address
 */

//static inline void
//iw_broad_ether(struct sockaddr *sap)
//{
//  sap->sa_family = ARPHRD_ETHER;
//  memset((char *) sap->sa_data, 0xFF, ETH_ALEN);
//}

/*------------------------------------------------------------------*/
/*
 * Create an Ethernet NULL address
 */
 
//static inline void
//iw_null_ether(struct sockaddr *sap)
//{
//  sap->sa_family = ARPHRD_ETHER;
//  memset((char *) sap->sa_data, 0x00, ETH_ALEN);
//}

#endif	/* IWLIB_H */
