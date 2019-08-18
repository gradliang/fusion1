#ifndef __NET_ARP_H
#define __NET_ARP_H

#include "net_packet.h"
#include "error.h"

#define ARP_HW_TYPE_ETHERNET        1

#define ARP_OP_REQUEST              1
#define ARP_OP_REPLY                2

#define ARP_MAX_AGE                 240
#define ARP_MAX_PENDING             24
#define ARP_TABLE_SIZE              10

#define ARP_NO_ERR                  NO_ERR
#define ARP_HOST_UNAVAILABLE        -1
#define ARP_PRIVATE_ERROR           -2

#define NET_PACKET_ARP(packet)      (NET_PACKET_ETHER(packet)+ETHERNET_HEADER_SIZE)

typedef enum
{
    E_ARP_STATE_EMPTY,
    E_ARP_STATE_EXPIRED,
    E_ARP_STATE_PENDING,
    E_ARP_STATE_STABLE,
} E_ARP_STATE;

typedef struct
{
    U32             u32IpAddr;
    U08             u08EtherAddr[6];
    E_ARP_STATE     eState;
    U08             u08Age;
} ST_ARP_ENTRY;

typedef struct
{
    U16 u16HWType;          //hardware type
    U16 u16ProtType;        //protocol type
    U08 u08HWAddrLength;    //hardware address length
    U08 u08ProtAddrLength;  //protocol address length
    U16 u16Operation;
    U08 u08ARPData[20];     //for ethernet IPV4
    U08 u08Padding[18];
} ST_ARP_HEADER;

// arp header field offset
#define ARP_HARDWARE_TYPE           0
#define ARP_PORTOCOL_TYPE           (ARP_HARDWARE_TYPE+2)
#define ARP_HW_ADDR_LENGTH          (ARP_PORTOCOL_TYPE+2)
#define ARP_PROT_ADDR_LENGTH        (ARP_HW_ADDR_LENGTH+1)
#define ARP_OPERATION               (ARP_PROT_ADDR_LENGTH+1)
#define ARP_DATA                    (ARP_OPERATION+2)

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


S32 ArpInit();
S32 ArpQuery(U32 ipAddress, U08* macAddress);
S32 ArpPacketReceive(ST_NET_PACKET* packet);
ST_NET_PACKET* ArpPacketReceive2(ST_NET_PACKET* packet);
void ArpDumpTable();

#endif
