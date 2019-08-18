#ifndef __NET_IP_H
#define __NET_IP_H

#include "typedef.h"
#include "net_packet.h"

// ip header field offset
#define IP_VER_IHL                  0
#define IP_SERVICE                  1
#define IP_LENGTH                   2
#define IP_ID                       4
#define IP_FLAG                     6
#define IP_TTL                      8
#define IP_PROTOCOL                 9
#define IP_CHECKSUM                 10
#define IP_SRC_IP                   12
#define IP_DST_IP                   16
#define IP_PAYLOAD                  20

// ip protocol
#define IP_PROTOCOL_ICMP            1
#define IP_PROTOCOL_TCP             6
#define IP_PROTOCOL_UDP             17

#define IP_FRAGMENT_ENABLE          1

#define IP_REASS_FLAG_LASTFRAG      0x01

#define IP_HEADER_LENGTH            20
#define IP_REASSEMBLE_BUFFER_SIZE   5760

#define IP_MAX_PACKET_LEN           IP_REASSEMBLE_BUFFER_SIZE

#define IP_REASS_MAXAGE             3 //seconds

//ip flag definition
#define IP_FLAG_DF_NOT_FRAGMENT     0x4000
#define IP_FLAG_MF_MORE_FRAGMENT    0x2000

#define IP_FRAGMENT_OFFSET_MASK     0x1fff

// ip header
typedef struct
{
    U08 u08VerIhl;          // IP version [7:4] and header length in U32 [3:0]
    U08 u08Service;         // type of service
    U16 u16Length;          // number of byte in packet
    U16 u16Id;              // identification value
    U16 u16Flag;            // flags [15:13] and fragment offset [12:0]
    U08 u08TimeToLive;      // limits of the lifetime of this datagram
    U08 u08Protocol;        // 1.ICMP 6.TCP 17.UDP
    U16 u16Checksum;        // header-only checksum
    U32 u32SrcIp;           // source IP address
    U32 u32DstIp;           // destination IP address
} ST_IP_HEADER;

struct ip_statistics {
	u32 rx_ip_pkts;
	u32 rx_ip_chksum;
	u32 rx_ip_dropped;
};
extern struct ip_statistics ip_stat;

void IpInit();
S32 IpPacketSend(ST_NET_PACKET *packet, U08 protocol, U32 srcAddr, U32 destAddr);
void IpPacketReceive(ST_NET_PACKET *packet, BOOL isMcast, struct net_device* dev);
#endif
