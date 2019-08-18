#include "typedef.h"
#include "linux/types.h"
#include "global612.h"
#include "lwip_config.h"
#include "socket.h"
#include "net_icmp.h"
#include "error.h"
#include "net_netctrl.h"
#include "net_ip.h"
#include "net_nic.h"

S32 IcmpPacketReceive(ST_NET_PACKET* packet)
{
    U08* ip = (U08*)NET_PACKET_IP(packet);
    U08 ipHLen = (ip[IP_VER_IHL] & 0xf) << 2;
    U08* icmp;
    U32 srcIp, dstIp;
    U16 icmpLen;
    U16 checksum;
    
    srcIp = NetGetDW(&ip[IP_SRC_IP]);
    dstIp = NetGetDW(&ip[IP_DST_IP]);
    
    icmp = ip + ipHLen;
    icmpLen = NetGetW(&ip[IP_LENGTH]) - ipHLen;
    
    if(0 != (checksum = InetCsum(icmp, icmpLen)))
    {
        DPrintf("[ICMP] checksum error(%d)", checksum);
        //NetPacketDump(icmp, (icmpLen+1) & 0xfffe);
        netBufFree(packet);
        return NO_ERR;
    }
    
    switch(icmp[ICMP_TYPE])
    {
        case ICMP_TYPE_ECHO:
        {
            icmp[ICMP_TYPE] = ICMP_TYPE_ER;
            
            NetPutW(&(icmp[ICMP_CHECKSUM]), 0);
            checksum = InetCsum(icmp, icmpLen);
            NetPutW(&(icmp[ICMP_CHECKSUM]), checksum);
            
            packet->Net.u16PayloadSize = icmpLen;
            
            if(dstIp == NetDefaultIpGet())
            {
#define ICMP_DIRECT_REPLY
#ifdef ICMP_DIRECT_REPLY
                IpPacketSend(packet, IP_PROTOCOL_ICMP, dstIp, srcIp);
#ifndef DRIVER_FREE_TXBUFFER
                netBufFree(packet);
#endif
#else
                U32 message[5];
                
                message[0] = NETCTRL_MSG_IP_PACKET_SEND;
                message[1] = (U32)packet;
                message[2] = IP_PROTOCOL_ICMP;
                message[3] = dstIp;
                message[4] = srcIp;
                mpx_MessageSend(NetCtrlMessageIdGet(), message, sizeof(message));
#endif
            }
            else
                netBufFree(packet);
        }
        break;
        
        default:
            netBufFree(packet);
        break;
    }
    
    return NO_ERR;
}


