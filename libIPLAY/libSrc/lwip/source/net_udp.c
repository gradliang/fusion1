#include <errno.h>
#include <string.h>
#include "linux/types.h"
#include "global612.h"
#include "lwip_config.h"
#include "typedef.h"
#include "os.h"
#include "socket.h"
#include "net_ip.h"
#include "net_udp.h"
#include "error.h"
#include "net_nic.h"
#include "ndebug.h"

#define UDP_MAX_PORT_HANDLER    2

typedef struct 
{
    U08 u08State;
    U32 u32LocalIp;
    U32 u32PeerIp;
    U16 u16LocalPort;
    U16 u16PeerPort;
	struct list_head list;
} REAL_ST_UDP_PCB;

typedef void (*UDP_PORT_HANDLER)(ST_NET_PACKET*, U32, U16);

typedef struct
{
    U08 u08Used;
    U08 u08Reserved;
    U16 u16Port;
    U32 handler;
} ST_UDP_PORT_HANDLER;
static ST_UDP_PORT_HANDLER udpHandlerBank[UDP_MAX_PORT_HANDLER];

static struct list_head udp_pcbs = LIST_HEAD_INIT(udp_pcbs);

void UdpInit()
{
    memset(udpHandlerBank, 0, sizeof(udpHandlerBank));
}

S32 udpFindIdleEntry()
{
    S32 i;
    
    for(i = 0; i < UDP_MAX_PORT_HANDLER; i++)
    {
        if(udpHandlerBank[i].u08Used == FALSE)
            return i;
    }
    
    return -1;
}

S32 udpFindPortEntry(U16 u16Port)
{
    S32 i;
    
    for(i = 0; i < UDP_MAX_PORT_HANDLER; i++)
    {
        if((udpHandlerBank[i].u08Used) && (udpHandlerBank[i].u16Port == u16Port))
            return i;
    }
    
    return -1;
}

S32 UdpOfficialPortRegister(U16 u16Port, U32 u32Handler)
{
    S32 entry = udpFindIdleEntry();
    
    if(entry >= 0)
    {
        udpHandlerBank[entry].u08Used = TRUE;
        udpHandlerBank[entry].u16Port = u16Port;
        udpHandlerBank[entry].handler = u32Handler;
        
        return NO_ERR;
    }
    else
        return -1;        
}


extern U32 rxpkts1, rxpkts2, rxpkts3;
/* 
 * No matter whether DRIVER_FREE_TXBUFFER is defined, the buffer pointed by 
 * <packet> will be freed upon return from this function.
 */
S32 UdpPacketSend(ST_NET_PACKET *packet, U32 srcAddr, U16 srcPort, U32 destAddr, U16 destPort)
{
    U16 sendSize;
    U08* ptr = (U08*)NET_PACKET_PSEUDO(packet);
    U16 length = packet->Net.u16PayloadSize + sizeof(ST_UDP_HEADER);
    U16 checksum;
    U08* checkStart;
    
//    DPrintf("%s", __FUNCTION__);

    ptr += 8; //Pseudo.reserved[8]
    checkStart = ptr;
    if (srcAddr == INADDR_ANY)
        srcAddr = NetDefaultIpGet();
    NetPutDW(ptr, srcAddr); //Pseudo.u32SrcIp
    ptr += 4;
    NetPutDW(ptr, destAddr); //Pseudo.u32DstIp
    ptr += 4;
    *ptr++ = 0; //Pseudo.u08Zero
    *ptr++ = IP_PROTOCOL_UDP; //Pseudo.u08Protocol
    NetPutW(ptr, length); //Pseudo.u16Length
    ptr += 2;
    NetPutW(ptr, srcPort); //Udp.u16SrcPort
    ptr += 2;
    NetPutW(ptr, destPort); //Udp.u16DstPort
    ptr += 2;
    NetPutW(ptr, length); //Udp.u16Length
    ptr += 2;
                U08 *udp = NET_PACKET_UDP(packet);
                U16 fromPort = NetGetW(&udp[UDP_SRC_PORT]);
//    DPrintf("UdpPacketSend: fport=%d, srcPort=%d", fromPort, srcPort);
    
    NetPutW(ptr, 0);
    checksum = InetCsum(checkStart, length + UPD_PSEUDO_HEADER_LENGTH);
    NetPutW(ptr, checksum);
    
    packet->Net.u16PayloadSize += sizeof(ST_UDP_HEADER);
    
//    DPrintf("UdpPacketSend: r1=%d,r2=%d,r3=%d", rxpkts1,rxpkts2,rxpkts3);
    sendSize = IpPacketSend(packet, IP_PROTOCOL_UDP, srcAddr, destAddr);
#ifndef DRIVER_FREE_TXBUFFER
    NetFreePacket(packet);
#endif
    
    if (sendSize < 0)
        return -1;
    else
    return (sendSize - sizeof(ST_UDP_HEADER));
}


/* 
 * No matter whether DRIVER_FREE_TXBUFFER is defined, the buffer pointed by 
 * <packet> will be freed upon return from this function.
 */
S32 UdpPacketSend_if(ST_NET_PACKET *packet, U32 srcAddr, U16 srcPort, U32 destAddr, U16 destPort, struct net_device *dev)
{
    U16 sendSize;
    U08* ptr = (U08*)NET_PACKET_PSEUDO(packet);
    U16 length = packet->Net.u16PayloadSize + sizeof(ST_UDP_HEADER);
    U16 checksum;
    U08* checkStart;
    
    //DPrintf("%s, length = %d", __FUNCTION__, length);

    ptr += 8; //Pseudo.reserved[8]
    checkStart = ptr;
    if (srcAddr == INADDR_ANY)
        srcAddr = dev->u32DefaultIp;
    NetPutDW(ptr, srcAddr); //Pseudo.u32SrcIp
    ptr += 4;
    NetPutDW(ptr, destAddr); //Pseudo.u32DstIp
    ptr += 4;
    *ptr++ = 0; //Pseudo.u08Zero
    *ptr++ = IP_PROTOCOL_UDP; //Pseudo.u08Protocol
    NetPutW(ptr, length); //Pseudo.u16Length
    ptr += 2;
    NetPutW(ptr, srcPort); //Udp.u16SrcPort
    ptr += 2;
    NetPutW(ptr, destPort); //Udp.u16DstPort
    ptr += 2;
    NetPutW(ptr, length); //Udp.u16Length
    ptr += 2;
                U08 *udp = NET_PACKET_UDP(packet);
                U16 fromPort = NetGetW(&udp[UDP_SRC_PORT]);
//    DPrintf("UdpPacketSend: fport=%d, srcPort=%d", fromPort, srcPort);
    
    NetPutW(ptr, 0);
    checksum = InetCsum(checkStart, length + UPD_PSEUDO_HEADER_LENGTH);
    NetPutW(ptr, checksum);
    
    packet->Net.u16PayloadSize += sizeof(ST_UDP_HEADER);
    
//    DPrintf("UdpPacketSend: r1=%d,r2=%d,r3=%d", rxpkts1,rxpkts2,rxpkts3);
    sendSize = IpPacketSend_if(packet, IP_PROTOCOL_UDP, srcAddr, destAddr, dev);
#ifndef DRIVER_FREE_TXBUFFER
    NetFreePacket(packet);
#endif
    
    if (sendSize < 0)
        return -1;
    else
    return (sendSize - sizeof(ST_UDP_HEADER));
}

void *dns_input(ST_NET_PACKET *packet);

extern int resolv_socket;
void UdpPacketReceive(ST_NET_PACKET *packet, U32 srcAddr, U32 destAddr)
{
    U16 csum;
    U08 *udp, *pseudo;
    U16 length;
    U16 srcPort, dstPort;
    ST_IP_HEADER ipHdr;
    
    udp = (U08*)NET_PACKET_UDP(packet);
    length = NetGetW(&udp[UDP_LENGTH]);
    srcPort = NetGetW(&udp[UDP_SRC_PORT]);
    dstPort = NetGetW(&udp[UDP_DST_PORT]);
    csum = NetGetW(&udp[UDP_CHECKSUM]);
#if 0
	DPrintf("UdpPacketReceive srcport=%d dstport=%d", srcPort, dstPort);
#endif
    
    if(csum != 0)
    {
    
        memcpy(&ipHdr, NET_PACKET_IP(packet), sizeof(ST_IP_HEADER));
        
        pseudo = (U08*)NET_PACKET_PSEUDO(packet);
        NetPutDW(&pseudo[PSEUDO_SRC_IP], srcAddr);
        NetPutDW(&pseudo[PSEUDO_DST_IP], destAddr);
        pseudo[PSEUDO_ZERO] = 0;
        pseudo[PSEUDO_PROTOCOL] = IP_PROTOCOL_UDP;
        NetPutW(&pseudo[PSEUDO_LENGTH], length);
        
        csum = InetCsum(&pseudo[PSEUDO_SRC_IP], length + UPD_PSEUDO_HEADER_LENGTH);
        
        memcpy(NET_PACKET_IP(packet), &ipHdr, sizeof(ST_IP_HEADER));
    }
    //else
    //    DPrintf("[UDP] packet checksum is 0, skip checksum proc");
    
    if(csum != 0)
    {
        DPrintf("[UDP] udp packet checksum error", csum);
        DPrintf("From \\-");
        NetDebugPrintIP(srcAddr);
        DPrintf("To \\-");
        NetDebugPrintIP(destAddr);
        //NetPacketDump(NET_PACKET_UDP(packet), length);
        
        netBufFree(packet);
    }
    else
    {
        S32 entry = udpFindPortEntry(dstPort);
        
        packet->Net.u16PayloadSize -= sizeof(ST_UDP_HEADER);
        
#if 0
        if (srcPort == 53)                      /* DNS response */
        {
            if (dns_input(packet) == NULL)
                return;
        }
#endif
        if(entry >= 0)
        {
            ((UDP_PORT_HANDLER)(udpHandlerBank[entry].handler))(packet, srcAddr, srcPort);
        }
        else
        {
            SockSignalRxDone(srcAddr, srcPort, destAddr, dstPort, packet);
        }
    }
}



ST_UDP_PCB* UdpNew()
{
    REAL_ST_UDP_PCB *pcb;
    pcb = mpx_Malloc(sizeof(REAL_ST_UDP_PCB));
    if (pcb)
	{
        memset(pcb, 0, sizeof(REAL_ST_UDP_PCB));
		pcb->u08State = E_UDP_STATE_OPENED;
		list_add_tail(&pcb->list, &udp_pcbs);
	}
    return (ST_UDP_PCB *)pcb;
}



void UdpFree(ST_UDP_PCB* pcb)
{
    REAL_ST_UDP_PCB *real = (REAL_ST_UDP_PCB *)pcb;
	if (real)
	{
		list_del(&real->list);
		mpx_Free(real);
	}
}



S32 UdpBind(ST_UDP_PCB* pcb, U32 ipAddr, U16 port)
{
    pcb->u32LocalIp = ipAddr;
    pcb->u16LocalPort = port;
    
    return NO_ERR;
}



S32 UdpConnect(ST_UDP_PCB* pcb, U32 ipAddr, U16 port)
{
    pcb->u32PeerIp = ipAddr;
    pcb->u16PeerPort = port;
    
    return NO_ERR;
}



S32 UdpWrite(ST_UDP_PCB* pcb, U08* data, S32 size)
{
    S32 sendSize = 0;
    U08* dataPtr = data;
    U16 payloadSize = 0;
    ST_NET_PACKET *packet = 0;
    S32 status;
    
    while(size)
    {
        //payloadSize = size > MAX_UDP_DATA ? MAX_UDP_DATA : size;
        //payloadSize = size > IP_MAX_PACKET_LEN ? IP_MAX_PACKET_LEN : size;
        if (size > MAX_UDP_DATA)
            payloadSize = MAX_UDP_DATA;
        else
            payloadSize = size;
            
        //packet = NetNewPacket(FALSE);
        packet = NetNewPacketWithSize(payloadSize+sizeof(ST_IP_HEADER)+
                 sizeof(ST_UDP_HEADER)+ETHERNET_HEADER_SIZE, FALSE);
        if(!packet)
        {
            DPrintf("[UDP] allocate buffer fail");
            break;
        }
        
		
        mmcp_memcpy(NET_PACKET_UDP_DATA(packet), dataPtr, payloadSize);
        packet->Net.u16PayloadSize = payloadSize;
        
        status = UdpPacketSend(packet, pcb->u32LocalIp, pcb->u16LocalPort, pcb->u32PeerIp, pcb->u16PeerPort);
        
        if(status <= 0)
            break;
            
        sendSize += payloadSize;
        size -= payloadSize;
        dataPtr += payloadSize;
    }
    
    return sendSize;
}

/* from lower layer */
void UdpCtrlInput(int cmd, U32 peer_ip, U08 *iph)
{
	struct list_head *ptr;
    U32 srcIp;
    U08 *uh;
    U16 srcPort, dstPort;

    srcIp = NetGetDW(&iph[IP_SRC_IP]);
	uh = iph + ((iph[IP_VER_IHL] & 0xf) << 2);

    srcPort = NetGetW(&uh[UDP_SRC_PORT]);
    dstPort = NetGetW(&uh[UDP_DST_PORT]);

	switch (cmd)
	{
		case 3:                                 /* Port unreachable */
		list_for_each(ptr, &udp_pcbs) {
			REAL_ST_UDP_PCB *pcb = list_entry(ptr, REAL_ST_UDP_PCB, list);
			if (pcb->u32PeerIp == peer_ip && pcb->u32LocalIp == srcIp &&
					pcb->u16LocalPort == srcPort && pcb->u16PeerPort == dstPort)
			{
				pcb->u08State = E_UDP_STATE_CLOSED;
			}
		}
		break;
		default:
		break;
	}
}

int UdpWritable(ST_UDP_PCB *pcb)
{
	if (pcb)
		return (pcb->u08State == E_UDP_STATE_OPENED) ? TRUE : FALSE;
	else
		return 0;
}

