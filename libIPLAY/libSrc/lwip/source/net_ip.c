#include <string.h>
#include "linux/types.h"
#include "lwip_config.h"
#include "typedef.h"
#include "os.h"
#include "lwip_opt.h"
#include "socket.h"
#include "net_ip.h"
#include "net_udp.h"
#include "net_netctrl.h"
#include "net_device.h"
#include "net_nic.h"
#include <linux/if_ether.h>

#define DEBUG_IP        0

U16 u16IpId = 1;
struct ip_statistics ip_stat;

#if IP_FRAGMENT_ENABLE
static U08 ipReassBuf[IP_HEADER_LENGTH + IP_REASSEMBLE_BUFFER_SIZE];
static U08 ipReassBitmap[IP_REASSEMBLE_BUFFER_SIZE / (8*8) + 1];
static const U08 bitmapBits[8] = {0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01};
static U16 ipReassLength;
static U08 ipReassFlags;
static U08 ipReassTimer;
static U08 ipReassTimerId;

static ST_NET_PACKET* ipReassemble(ST_NET_PACKET* packet);
static void ipReassTimerProc();
static S32 ipFragmentOutput(ST_NET_PACKET* packet, U32 destIp);
#endif

#if ENABLE_LOOPBACK
int _loop_output(struct net_device   *_dev, ST_NET_PACKET *packet, U32 destAddr);
#endif

void *IpProtocolPacketReceive(U32 srcIp, U32 dstIp, U08 proto, void *packet);

void IpInit()
{
    S32 status;
    
#if IP_FRAGMENT_ENABLE
    ipReassTimer = 0;
    ipReassFlags = 0;
    ipReassLength = 0;
    memset(ipReassBitmap, 0, sizeof(ipReassBitmap));
    
    status = NetTimerInstall(ipReassTimerProc, NET_TIMER_1_SEC);
    if(status >= 0)
    {
        ipReassTimerId = (U08)status;
        NetTimerRun(ipReassTimerId);
    }
    else
    {
        DPrintf("[IP] timer create fail");
        BREAK_POINT();
    }
#endif
}

S32 IpPacketSend(ST_NET_PACKET *packet, U08 protocol, U32 srcAddr, U32 destAddr)
{
    int sendSize;
    U08* ipHdr = NET_PACKET_IP(packet);
    
    //DPrintf("%s", __FUNCTION__);
    memset(ipHdr, 0, sizeof(ST_IP_HEADER));
    
    ipHdr[IP_VER_IHL] = 0x40 + (sizeof(ST_IP_HEADER) >> 2);
    ipHdr[IP_SERVICE] = 0;
    NetPutW(&ipHdr[IP_LENGTH], packet->Net.u16PayloadSize + sizeof(ST_IP_HEADER));
    NetPutW(&ipHdr[IP_ID], u16IpId++);
    NetPutW(&ipHdr[IP_FLAG], IP_FLAG_DF_NOT_FRAGMENT); //don't fragment
    ipHdr[IP_TTL] = 128;
    ipHdr[IP_PROTOCOL] = protocol;
    NetPutW(&ipHdr[IP_CHECKSUM], 0);
    if (srcAddr == INADDR_ANY)
    {
        srcAddr = NetDefaultIpGet();
        packet->Net.u08NetIndex = NetDefaultNicGet();
    }
    else
    {
        packet->Net.u08NetIndex = NetNicGet(srcAddr);
        if (packet->Net.u08NetIndex == NIC_INDEX_NULL)
        {
            DPrintf("[IP] IP layer down");
            NetFreePacket(packet);
            return -1;
        }
    }

    NetPutDW(&ipHdr[IP_SRC_IP], srcAddr);
    NetPutDW(&ipHdr[IP_DST_IP], destAddr);
    
    if((packet->Net.u16PayloadSize + sizeof(ST_IP_HEADER)) > MAX_TX_UNIT)
    {
#if IP_FRAGMENT_ENABLE
        return ipFragmentOutput(packet, destAddr);
#else
        DPrintf("[IP] packet size exceed ethernet MTU");
        NetFreePacket(packet);
        return -1;
#endif
    }
    
    packet->Net.u16PayloadSize += sizeof(ST_IP_HEADER);
    NetPutW(&ipHdr[IP_CHECKSUM], InetCsum(NET_PACKET_IP(packet), sizeof(ST_IP_HEADER)));

#if ENABLE_LOOPBACK
    if (destAddr == INADDR_LOOPBACK)
    {
		sendSize = packet->Net.u16PayloadSize;
        /* Packet to self, enqueue it for loopback */
        LWIP_DEBUGF(IP_DEBUG, ("netif_loop_output()"));
        if (_loop_output(NULL, packet, destAddr) == 0)
            return (sendSize - sizeof(ST_IP_HEADER));
        else
            return -1;
    }
#endif

#if PPP_ENABLE
	if(packet->Net.u08NetIndex == NIC_INDEX_PPP)
	{
#if Make_MagicSync
		aalMs_NETHandler_TxIPpkt(packet->u08Data, packet->Net.u16PayloadSize);
		sendSize = packet->Net.u16PayloadSize;

		netBufFree((struct sk_buff *)packet);
		return (sendSize - sizeof(ST_IP_HEADER));
#else
		int ret;
		packet->Net.len = packet->Net.u16PayloadSize;
		sendSize = packet->Net.len;
		ret = pppifOutput(packet, NULL);
		if(ret != 0){
			
            NetFreePacket(packet);
			mpDebugPrint("pppifOutput fail");
		}
		return (sendSize - sizeof(ST_IP_HEADER));
#endif
	}
	else
#endif
    {
	    sendSize = NetPacketSend(packet, destAddr, ETHERNET_TYPE_IP);
	    if (sendSize < 0)
	    {
	        return sendSize;
	    }
	    else
	    return (sendSize - sizeof(ST_IP_HEADER));
	}
}


S32 IpPacketSend_if(ST_NET_PACKET *packet, U08 protocol, U32 srcAddr, U32 destAddr, struct net_device *dev)
{
    int sendSize;
    U08* ipHdr = NET_PACKET_IP(packet);
    
    //DPrintf("%s", __FUNCTION__);
    memset(ipHdr, 0, sizeof(ST_IP_HEADER));
    
    ipHdr[IP_VER_IHL] = 0x40 + (sizeof(ST_IP_HEADER) >> 2);
    ipHdr[IP_SERVICE] = 0;
    NetPutW(&ipHdr[IP_LENGTH], packet->Net.u16PayloadSize + sizeof(ST_IP_HEADER));
    NetPutW(&ipHdr[IP_ID], u16IpId++);
    NetPutW(&ipHdr[IP_FLAG], IP_FLAG_DF_NOT_FRAGMENT); //don't fragment
    ipHdr[IP_TTL] = 128;
    ipHdr[IP_PROTOCOL] = protocol;
    NetPutW(&ipHdr[IP_CHECKSUM], 0);
    if (srcAddr == INADDR_ANY)
        srcAddr = dev->u32DefaultIp;

    NetPutDW(&ipHdr[IP_SRC_IP], srcAddr);
    NetPutDW(&ipHdr[IP_DST_IP], destAddr);
    
    packet->Net.u08NetIndex = dev->ifindex;
    
    if((packet->Net.u16PayloadSize + sizeof(ST_IP_HEADER)) > MAX_TX_UNIT)
    {
#if IP_FRAGMENT_ENABLE
        return ipFragmentOutput(packet, destAddr);
#else
        DPrintf("[IP] packet size exceed ethernet MTU");
        return -1;
#endif
    }
    
    packet->Net.u16PayloadSize += sizeof(ST_IP_HEADER);
    NetPutW(&ipHdr[IP_CHECKSUM], InetCsum(NET_PACKET_IP(packet), sizeof(ST_IP_HEADER)));

#if PPP_ENABLE
	if(dev->ifindex == NIC_INDEX_PPP){
#if Make_MagicSync
		aalMs_NETHandler_TxIPpkt(packet->u08Data, packet->Net.u16PayloadSize);
		sendSize = packet->Net.u16PayloadSize;

		netBufFree((struct sk_buff *)packet);
		return (sendSize - sizeof(ST_IP_HEADER));
#else
		int ret;
		packet->Net.len = packet->Net.u16PayloadSize;
		sendSize = packet->Net.len;
		ret = pppifOutput(packet, NULL);
		if(ret != 0){
			
			mpDebugPrint("pppifOutput fail");
		}
		return (sendSize - sizeof(ST_IP_HEADER));
#endif
	}
	else
#endif
    {
	    sendSize = NetPacketSend(packet, destAddr, ETHERNET_TYPE_IP);
	    if (sendSize < 0)
	    {
	        return sendSize;
	    }
	    else
	    return (sendSize - sizeof(ST_IP_HEADER));
	}
}


void IpPacketReceive(ST_NET_PACKET *packet, BOOL isMcast, struct net_device* dev)
{
    U16 csum;
    U08* ptr = (U08*)NET_PACKET_IP(packet);
    U08 headerLen;
    
    if((csum = InetCsum(ptr, sizeof(ST_IP_HEADER))))
    {
        //NET_DEBUG_STR("ip packet check sum error");
        //NetPacketDump(packet, packet->Net.u16PayloadSize);
        netBufFree(packet);       // if the checksum is wrong, just abandon it
        ip_stat.rx_ip_chksum++;
    }
    else
    {
        U32 srcIp, dstIp;
        U16 packetLen;
        
        headerLen = (ptr[IP_VER_IHL] & 0xf) << 2;
        if(headerLen != 20)
        {
            #if DEBUG_IP
            DPrintf("[IP] unsupported ip header length");
            #endif
            
            netBufFree(packet);
            ip_stat.rx_ip_dropped++;
            return;
        }
        
        if(NetGetW(&ptr[IP_FLAG]) & (IP_FLAG_MF_MORE_FRAGMENT|IP_FRAGMENT_OFFSET_MASK))
        {
#if IP_FRAGMENT_ENABLE
            packet = ipReassemble(packet);
            if(!packet)
            {
                ip_stat.rx_ip_dropped++;
                return;
            }
            ptr = (U08*)NET_PACKET_IP(packet);
#else
            DPrintf("[IP] fragmented ip packet received");
            netBufFree(packet);
            return;
#endif
        }
        
        srcIp = NetGetDW(&ptr[IP_SRC_IP]);
        dstIp = NetGetDW(&ptr[IP_DST_IP]);
        packetLen = NetGetW(&ptr[IP_LENGTH]);
#if 0
        unsigned char *chp = &srcIp;
        DPrintf("IpPacketReceive srcip => %3u:%3u:%3u:%3u",
                chp[0], chp[1], chp[2], chp[3]);
        chp = &dstIp;
        DPrintf("IpPacketReceive dstip => %3u:%3u:%3u:%3u",
                chp[0], chp[1], chp[2], chp[3]);
#endif
        
        #if DEBUG_IP
        DPrintf("[IP] dest ip \\-");
        NetDebugPrintIP(dstIp);
        #endif
        
        if((dstIp != NetBroadcaseIpGet()) && !isForUs(dstIp)) //not broadcast and not for me
        {
            if((0 == dev->u32DefaultIp) && (ptr[IP_PROTOCOL] == IP_PROTOCOL_UDP))
            {
                U08* udp = (U08*)NET_PACKET_UDP(packet);
                U16 dstPort = NetGetW((udp+2));
                
                if(dstPort == 68)
                    ArpUpdateEntry(srcIp, NetPacketSrcMacGet(packet));
                else
                {
                    netBufFree(packet);
                    ip_stat.rx_ip_dropped++;
                    return;
                }
            }
            else if ( !((0 != dev->u32DefaultIp) && isMcast) )   /* don't drop multicast packets */
            {
                netBufFree(packet);
                ip_stat.rx_ip_dropped++;
                return;
            }
        }
        else if(dstIp == NetBroadcaseIpGet()) //for broadcast only, update the arp table
        {
            if((0 == dev->u32DefaultIp) && (ptr[IP_PROTOCOL] == IP_PROTOCOL_UDP))
            {
                U08* udp = (U08*)NET_PACKET_UDP(packet);
                U16 dstPort = NetGetW((udp+2));
                
                if(dstPort != 68)
                {
                    netBufFree(packet);
                    ip_stat.rx_ip_dropped++;
                    return;
                }
            }

//            ArpUpdateEntry(srcIp, NetPacketSrcMacGet(packet));
        }
        
        packet->Net.u16PayloadSize = packetLen - headerLen;
        if(ptr[IP_PROTOCOL] == IP_PROTOCOL_TCP)
        {
            ip_stat.rx_ip_pkts++;
            TcpPacketInput(packet, srcIp, dstIp);
        }
        else if(ptr[IP_PROTOCOL] == IP_PROTOCOL_UDP)
        {
            ip_stat.rx_ip_pkts++;
            UdpPacketReceive(packet, srcIp, dstIp);
        }
        else
        {
            if (IpProtocolPacketReceive(srcIp, dstIp, ptr[IP_PROTOCOL],packet))
            {
                if(ptr[IP_PROTOCOL] == IP_PROTOCOL_ICMP)
                {
                    ip_stat.rx_ip_pkts++;
                    IcmpPacketReceive(packet);
                }
                else    // not supported packet, just abandon it
                {
                    NET_DEBUG_STR("unsupported packet, drop it...");
                    netBufFree(packet);
                    ip_stat.rx_ip_dropped++;
                }
            }

        }
    }
}



#if IP_FRAGMENT_ENABLE
static ST_NET_PACKET* ipReassemble(ST_NET_PACKET* packet)
{
    U08* ipHdr, *fragHdr;
    U16 offset, length;
    U16 i;
    
    ipHdr = ipReassBuf;
    fragHdr = NET_PACKET_IP(packet);
    
    if(ipReassTimer == 0) //no packets in reassemble buffer
    {
        memcpy(ipHdr, fragHdr, IP_HEADER_LENGTH);
        ipReassTimer = IP_REASS_MAXAGE;
        ipReassFlags = 0;
        
        memset(ipReassBitmap, 0, sizeof(ipReassBitmap));
    }
    
    if((NetGetDW(&ipHdr[IP_SRC_IP]) == NetGetDW(&fragHdr[IP_SRC_IP])) &&
        (NetGetDW(&ipHdr[IP_DST_IP]) == NetGetDW(&fragHdr[IP_DST_IP])) &&
        (NetGetW(&ipHdr[IP_ID]) == NetGetW(&fragHdr[IP_ID])))
    {
        length = NetGetW(&fragHdr[IP_LENGTH]) - ((fragHdr[IP_VER_IHL] & 0xf) << 2);
        offset = (NetGetW(&fragHdr[IP_FLAG]) & IP_FRAGMENT_OFFSET_MASK) * 8;
        
        #if DEBUG_IP
        DPrintf("[IP REASSEMBLE] fragment offset %d, length %d", offset, length);
        #endif
        
        if((offset > IP_REASSEMBLE_BUFFER_SIZE) || ((offset + length) > IP_REASSEMBLE_BUFFER_SIZE))
        {
            #if DEBUG_IP
            DPrintf("[IP REASSEMBLE] fragment offset over buffer size");
            #endif
            
            ipReassTimer = 0;
            goto ERROR;
        }
		   	memcpy(&ipReassBuf[IP_HEADER_LENGTH + offset], &fragHdr[IP_PAYLOAD], length);
        if((offset / (8*8)) == ((offset+length) / (8*8)))
        {
            ipReassBitmap[offset / (8*8)] |= (bitmapBits[(offset/8)&7] & ~bitmapBits[((offset+length)/8)&7]);
        }
        else
        {
            ipReassBitmap[offset / (8*8)] |= bitmapBits[(offset / 8) & 7];
            for(i = (1 + offset / (8*8)); i < ((offset + length) / (8*8)); ++i)
            {
                ipReassBitmap[i] = 0xff;
            }
            
            ipReassBitmap[(offset + length) / (8*8)] |= ~bitmapBits[((offset + length) / 8) & 7];
        }
        
        if((NetGetW(&fragHdr[IP_FLAG]) & IP_FLAG_MF_MORE_FRAGMENT) == 0)
        {
            ipReassFlags |= IP_REASS_FLAG_LASTFRAG;
            ipReassLength = offset + length;
            
            #if DEBUG_IP
            DPrintf("[IP REASSEMBLE] total length = %d", ipReassLength);
            #endif
        }
        
        if(ipReassFlags & IP_REASS_FLAG_LASTFRAG)
        {
            ST_NET_PACKET* newPacket;
            
            for(i = 0; i < ipReassLength/(8*8) - 1; ++i)
            {
                if(ipReassBitmap[i] != 0xff)
                {
                    #if DEBUG_IP
                    DPrintf("[IP REASSEMBLE] last fragment received, but bitmap not completed");
                    #endif
                    
                    goto ERROR;
                }
            }
            
            if(ipReassBitmap[ipReassLength / (8*8)] != ((U08) ~bitmapBits[ipReassLength / 8 & 7]))
            {
                goto ERROR;
            }
            
            ipReassLength += IP_HEADER_LENGTH;
            NetPutW(&ipHdr[IP_LENGTH], ipReassLength);
            NetPutW(&ipHdr[IP_FLAG], IP_FLAG_DF_NOT_FRAGMENT);
            NetPutW(&ipHdr[IP_CHECKSUM], 0);
            
            ipReassTimer = 0;
            //newPacket = (ST_NET_PACKET*)mpx_Malloc(ipReassLength + sizeof(ST_HEADER));
            //newPacket = NetNewPacket(FALSE);
            newPacket = (ST_NET_PACKET*)netBufAlloc(ipReassLength + ETH_HLEN);
            if(newPacket)
            {
#ifndef SKB_OPT
                memcpy(newPacket, packet, sizeof(ST_HEADER));
#else
                newPacket->Net.dev = packet->Net.dev;
                newPacket->Net.u08NetIndex = packet->Net.u08NetIndex;
#endif
                newPacket->Net.u16PayloadSize = ipReassLength + sizeof(ST_HEADER);
#ifndef MP600
                newPacket->Net.data = newPacket->u08Data;
#else
#ifndef SKB_OPT
                newPacket->Net.data = newPacket->Net.RESERVED;
#endif
#endif
                
					memcpy(NET_PACKET_IP(newPacket), ipReassBuf, ipReassLength);
            }
            else
            {
                #if DEBUG_IP
                DPrintf("[IP REASSEMBLE] allocate buffer fail");
                #endif
            }
            
            NetFreePacket(packet);
            return newPacket;
        }
        
        NetFreePacket(packet);
        return 0;
    }
    
ERROR:
    ipReassTimer = 0;
    NetFreePacket(packet);
    return 0;
}

static void ipReassTimerProc()
{
    if(ipReassTimer > 0)
    {
        ipReassTimer--;
        if(ipReassTimer == 0)
        {
            #if DEBUG_IP
            DPrintf("[IP REASSEMBLE] reassembly timed out");
            #endif
        }
    }
}

static S32 ipFragmentOutput(ST_NET_PACKET* packet, U32 destIp)
{
    U16 totalLength = packet->Net.u16PayloadSize;
    U16 packetLength;
    U16 left = totalLength;
    U08* ipHdr = NET_PACKET_IP(packet);
    U16 offset = 0;
    U16 flag;
    
    NetPutW(&ipHdr[IP_FLAG], 0);
    
    while(left)
    {
        ST_NET_PACKET* fragPacket = NetNewPacket(FALSE);
        U08* fragHdr;
        
        if(!fragPacket)
            return -1;
        
        fragHdr = NET_PACKET_IP(fragPacket);
        
        flag = 0;
#ifndef SKB_OPT
//        memcpy(fragPacket, packet, sizeof(ST_HEADER) + sizeof(ST_ETHER_HEADER) + sizeof(ST_IP_HEADER));
        memcpy(fragPacket->Net.data, packet->Net.data, sizeof(ST_ETHER_HEADER) + sizeof(ST_IP_HEADER));
#else
        fragPacket->Net.dev = packet->Net.dev;
        fragPacket->Net.u08NetIndex = packet->Net.u08NetIndex;
        memcpy(fragHdr, NET_PACKET_IP(packet), sizeof(ST_IP_HEADER));
#endif
        packetLength = left > MAX_IP_DATA ? MAX_IP_DATA : left;
        left -= packetLength;
        
        fragPacket->Net.u16PayloadSize = packetLength + sizeof(ST_IP_HEADER);
#ifndef MP600
        fragPacket->Net.data = fragPacket->u08Data;
#else
#if 0
        fragPacket->Net.data = fragPacket->Net.RESERVED;
#endif
#endif
        NetPutW(&fragHdr[IP_LENGTH], fragPacket->Net.u16PayloadSize);
        
        flag = offset / 8;
        if(left)
            flag |= IP_FLAG_MF_MORE_FRAGMENT;
        NetPutW(&fragHdr[IP_FLAG], flag);
        NetPutW(&fragHdr[IP_CHECKSUM], InetCsum(NET_PACKET_IP(fragPacket), sizeof(ST_IP_HEADER)));
        
        memcpy(&fragHdr[IP_PAYLOAD], &ipHdr[IP_PAYLOAD + offset], packetLength);
        NetPacketSend(fragPacket, destIp, ETHERNET_TYPE_IP);
#ifndef DRIVER_FREE_TXBUFFER
        NetFreePacket(fragPacket);
#endif
        
        offset += packetLength;
    }
    
#ifdef DRIVER_FREE_TXBUFFER
    NetFreePacket(packet);                      /* free original packet */
#endif

    //totalLength = packet->Net.u16PayloadSize;
    return (S32)totalLength;
}
#endif
