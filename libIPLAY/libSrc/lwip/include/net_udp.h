#ifndef __NET_UDP_H
#define __NET_UDP_H

#include "typedef.h"
#include "net_packet.h"

// this is the origin UDP pseudo header structure
//typedef struct
//{
//    U32 u32SrcIp;           // same as the u32SrcIp of ST_IP_HEADER
//    U32 U32dstIp;           // same as the u32DstIp of ST_IP_HEADER
//    U08 u08Zero;            // fix to zero
//    U08 u08Protocol;        // same as the u08Protocol of ST_IP_HEADER
//    U16 u16UdpLength;       // same as the u16Length of ST_UDP_HEADER
//} ST_UDP_PSEUDO_HEADER;

#define UPD_PSEUDO_HEADER_LENGTH    12

// rearragne the UDP pseudo header to make use of the tail of the IP header
typedef struct
{
    U08 reserved[8];
    U32 u32SrcIp;
    U32 u32DstIp;
    U08 u08Zero;
    U08 u08Protocol;
    U16 u16Length;
} ST_UDP_PSEUDO;

typedef struct
{    
    U16 u16SrcPort;         // source port number
    U16 u16DstPort;         // destination port number
    U16 u16Length;          // total size of UDP packet, including header and data
    U16 u16Checksum;        // checksum on pseudo UDP header, UDP header and data part
} ST_UDP_HEADER;

#define UDP_SRC_PORT        0
#define UDP_DST_PORT        2
#define UDP_LENGTH          4
#define UDP_CHECKSUM        6

typedef struct
{
    U08 u08State;
    U32 u32LocalIp;
    U32 u32PeerIp;
    U16 u16LocalPort;
    U16 u16PeerPort;
} ST_UDP_PCB;

ST_UDP_PCB* UdpNew();
void UdpFree(ST_UDP_PCB* pcb);
S32 UdpBind(ST_UDP_PCB* pcb, U32 ipAddr, U16 port);
S32 UdpConnect(ST_UDP_PCB* pcb, U32 ipAddr, U16 port);
S32 UdpWrite(ST_UDP_PCB* pcb, U08* data, S32 size);
#endif
