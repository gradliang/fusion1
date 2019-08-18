
#ifndef __NET_PACKET_H
#define __NET_PACKET_H

#include "typedef.h"
#include "net_utility.h"
#include "net_sockaddr.h"
#include "net_unix.h"
#include <asm/atomic.h> /* for atomic_t */


#define NET_DEBUG
#ifdef NET_DEBUG
    #define NET_DEBUG_STR               DPrintf
#elif
    #define NET_DEBUG_STR
#endif

#define DRIVER_TYPE_ETHER               1
#define DRIVER_TYPE_80211               2
#define DRIVER_TYPE_PPP                 3


#define MAX_DEVICE_HEADER_SIZE          32
#define ETHERNET_HEADER_SIZE            14

#define ETHERNET_TYPE_IP                0x0800
#define ETHERNET_TYPE_ARP               0x0806
#define ETHERNET_MIN_TYPE               0x0600

#define ETHERNET_MAC_LENGTH             6
#define ETHERNET_IP_LENGTH              4

#define MAX_TX_UNIT                     1500 // fix to the Ethernet MTU 
#define MAX_IP_DATA                     (MAX_TX_UNIT - sizeof(ST_IP_HEADER))
#define MAX_UDP_DATA                    (MAX_IP_DATA - sizeof(ST_UDP_HEADER))
#define MAX_TCP_DATA                    (MAX_IP_DATA - sizeof(ST_TCP_HEADER))


typedef struct
{
    U08 u08DstMacAddr[ETHERNET_MAC_LENGTH];
    U08 u08SrcMacAddr[ETHERNET_MAC_LENGTH];
    U16 u16TypeLength;
} ST_ETHER_HEADER;

typedef unsigned char *_sk_buff_data_t;

// the generic packet header, all the type of the packet will contain this header
typedef struct
{
    //---------------------------------------------//
    /* sk_buff used*/
    void *next;
    void *prev;

    struct sock         *sk;
    struct net_device	*dev;

	char cb[48];
    U32 len, data_len, mac_len, csum;
    U32 priority;
    U08 *head, *data, *tail, *end;
	__u8			local_df:1,
				cloned:1,
				ip_summed:2,
				nohdr:1,
				nfctinfo:3;
	__u8			pkt_type:3,
				fclone:2,
				ipvs_property:1,
				nf_trace:1;
	__be16			protocol;

	int			iif;
	__u16			queue_mapping;

	__u8			do_not_encrypt:1,
					data_allocated:1;

	_sk_buff_data_t		transport_header;
	_sk_buff_data_t		network_header;
	_sk_buff_data_t		mac_header;
	unsigned int		truesize;
	atomic_t		users;
    //----------------------------------------------//

#ifndef MP600
    U16 u16PayloadSize;                     // UDP or TCP data size
    U16 u16SockId;                          // the socket id when transmission
#endif
    U08 u08Type;                            
    U08 u08NetIndex;                        // net driver index
#ifdef MP600
    U16 u16PayloadSize;                     // UDP or TCP data size
    U16 u16SockId;                          // the socket id when transmission
    U08 RESERVED[128]__attribute__((aligned(32)));
#else
    U08 RESERVED[14];                       // for 32bytes boundary
    U08 u08DeviceHeader[MAX_DEVICE_HEADER_SIZE];
#endif
} ST_HEADER;

typedef struct st_net_packet
{
    ST_HEADER       Net;
    U08             u08Data[MAX_TX_UNIT + ETHERNET_HEADER_SIZE];
} ST_NET_PACKET;

typedef struct
{
    struct sockaddr_un       from;
    U16             u16DataLen;
    U08             u08Data[MAX_TX_UNIT];
} ST_UNIX_PACKET;

#ifndef MP600
struct sk_buff {
    /* These two members must be first. */
    struct sk_buff *next;
    struct sk_buff *prev;
	struct net_device	*dev;
    U32 len, data_len, mac_len, csum;
    U32 priority;
    U08 *head, *data, *tail, *end;
    //-------------------------------------------------------//
    
    //ST_NET_PACKET used
    U16 u16PayloadSize;                     // UDP or TCP data size
    U16 u16SockId;                          // the socket id when transmissions
    U08 u08Type;                            
    U08 u08NetIndex;                        // net driver index
    U08 RESERVED[14];
    U08 u08Data[MAX_TX_UNIT + MAX_DEVICE_HEADER_SIZE + ETHERNET_HEADER_SIZE];
    //-------------------------------------------------------//
};
#endif

#define NET_PACKET_ETHER(packet)        ((U08*)(((ST_NET_PACKET*)packet)->Net.data))
#define NET_PACKET_IP(packet)           (NET_PACKET_ETHER(packet)+ETHERNET_HEADER_SIZE)
#define NET_PACKET_PSEUDO(packet)       NET_PACKET_IP(packet)
#define NET_PACKET_UDP(packet)          (NET_PACKET_IP(packet)+sizeof(ST_UDP_PSEUDO))
#define NET_PACKET_UDP_DATA(packet)     (NET_PACKET_UDP(packet)+sizeof(ST_UDP_HEADER))
#define NET_PACKET_TCP(packet)          NET_PACKET_UDP(packet)
#define NET_PACKET_TCP_DATA(packet)     (NET_PACKET_TCP(packet)+sizeof(ST_TCP_HEADER))

#define NET_TCP_SEQNO(seg)              NetGetDW(&(NET_PACKET_TCP(((seg)->packet))[TCP_SEQUENCE])) /* TODO never tested */

typedef enum
{
    E_UDP_STATE_FREE,
    E_UDP_STATE_CLOSED,
    E_UDP_STATE_OPENED,
} E_UDP_STATE;


// public port number
#define PORT_ECHO                       7
#define PORT_DISCARD                    9
#define PORT_DAYTIME                    13
#define PORT_FTP                        21
#define PORT_TELNET                     23
#define PORT_SMTP                       25
#define PORT_TIME                       37
#define PORT_HTTP                       80


// udp and tcp pseudo header fields offset
#define PSEUDO_SRC_IP                   8
#define PSEUDO_DST_IP                   12
#define PSEUDO_ZERO                     16
#define PSEUDO_PROTOCOL                 17
#define PSEUDO_LENGTH                   18


#endif  // __TC_PACKET_H_

