// define this module show debug message or not,  0 : disable, 1 : enable

#define LOCAL_DEBUG_ENABLE 0

//#include "typedef.h"
#include "linux/types.h"
#include "lwip_config.h"
#include "linux/bitops.h"
#include "lwip_opt.h"
#include "socket.h"
#include <string.h>
#include <sys/time.h>
#include "typedef.h"
#include "socket.h"
#include "SysConfig.h"
//#include "GpioIsr.h"
#ifndef MP600
#include "hal_gpio.h"
#endif
#include "net_device.h"
#include "net_socket.h"
#include "error.h"
#ifndef MP600
#include "Dm9ks.h"
#endif
#include "net_netctrl.h"
#ifndef MP600
#include "fs_api.h"
#else
#include "os_mp52x.h"
#endif
#include "taskid.h"
#include "netware.h"
#include "net_nic.h"
#include <linux/if_ether.h>

#define _TRANSMISSION_STATUS        0

#define NIC_DRIVER_PPP      0x00000001
#define NIC_DRIVER_WIFI     0x00000002
#define NIC_DRIVER_ETHER    0x00000004

struct net_device NicArray[NIC_DRIVER_MAX];   // index 0 is reserved for no driver avaiable
static U08 u08DefaultNicIndex = NIC_INDEX_NULL;
static U08 u08NetDeviceTaskId;
static U08 u08NetDeviceEnventId;
U08 u08NetSemaphoreId;
//#if Make_USB
static U08 u08NetReceiveTaskId;
static U08 u08NetReceiveMessageId;
//#endif
static U08 u08NetWifiMonitorTimerId;
static U32 u32LastReceiveTime;

static U32 u32TotalRxSize;
static U32 u32TotalTxSize;
static U32 u32PrevRxSize;
static U32 u32PrevTxSize;

const U08 NULL_MAC_ADDR[6] = {0, 0, 0, 0, 0, 0};
const U08 BROADCAST_MAC_ADDR[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
const U32 BROADCAST_IP_ADDR = 0xffffffff;
static U08 myMACAddr[6] = {0x00, 0x13, 0x49, 0x16, 0x33, 0x35};
static U08 netHostName[255+1];                     /* Internet host name */

void NetworkTaskInit();
S32 netLoadConfigFile();
void NetWifiMonitorTimerProc();
ST_SOCKET* SockGet(U16 u16SockId);
void mpx_NetLinkStatusSet(U08 if_index, BOOL conn);

extern struct net_device NicArray[NIC_DRIVER_MAX];

void NetPacketDump(U32 address, U32 size)
{
    U08* ptr = (U08*)address;
    U32 i = size;
    int cnt, len, bytecnt;
    int val;
    char buf[64];
    

    DPrintf("=======================================");
    len = 0;
    bytecnt = cnt = 0;
    while(size > 0)
    {
        size--;
        ++cnt;
        bytecnt++;

        val = *ptr;
        val = (val >> 4) & 0x0f;
        if (val <= 9)
            buf[len++] = val + '0';
        else
            buf[len++] = val - 10 + 'a';
        val = *ptr++;
        val = val & 0x0f;
        if (val <= 9)
            buf[len++] = val + '0';
        else
            buf[len++] = val - 10 + 'a';
        if (cnt == 2)
        {
            buf[len++] = ' ';
            cnt = 0;
        }
        if (bytecnt >= 16)
        {
            buf[len] = '\0';
            DPrintf("%s", buf);
            len = 0;
            bytecnt = 0;
        }
        else if (size == 0)
        {
            buf[len] = '\0';
            DPrintf("%s", buf);
            len = 0;
            bytecnt = 0;
        }
    }
    DPrintf("================END====================");
}

void NetPacketDump2(U32 address, U32 size, const char *str)
{
    U08* ptr = (U08*)address;
    U32 i = size;
    int cnt, len, bytecnt;
    int val;
    char buf[64];
    

    DPrintf("============%s============", str);
    len = 0;
    bytecnt = cnt = 0;
    while(size > 0)
    {
        size--;
        ++cnt;
        bytecnt++;

        val = *ptr;
        val = (val >> 4) & 0x0f;
        if (val <= 9)
            buf[len++] = val + '0';
        else
            buf[len++] = val - 10 + 'a';
        val = *ptr++;
        val = val & 0x0f;
        if (val <= 9)
            buf[len++] = val + '0';
        else
            buf[len++] = val - 10 + 'a';
        if (cnt == 2)
        {
            buf[len++] = ' ';
            cnt = 0;
        }
        if (bytecnt >= 16)
        {
            buf[len] = '\0';
            DPrintf("%s", buf);
            len = 0;
            bytecnt = 0;
        }
        else if (size == 0)
        {
            buf[len] = '\0';
            DPrintf("%s", buf);
            len = 0;
            bytecnt = 0;
        }
    }
    DPrintf("===============END=====================");
}
void NetAsciiDump(unsigned long address, unsigned long size)
{
    U08* ptr = (U08*)address;
    U32 i = size;
    int len, bytecnt;
    int val;
    char buf[256 + 1];
    short pr = FALSE;
    

    DPrintf("=======================================");
    len = 0;
    bytecnt = 0;
    while(size > 0)
    {
        size--;
        bytecnt++;

        val = *ptr++;
        if (isprint(val))
            buf[len++] = val;
        else if (val == '\r')
        {
            buf[len++] = val;
            if (*ptr != '\n')
                pr = TRUE;
        }
        else if (val == '\n')
        {
            buf[len++] = val;
            pr = TRUE;
        }
        else
            buf[len++] = ' ';
        if (pr || len >= 256)
        {
            buf[len] = '\0';
            DPrintf("%s", buf);
            len = 0;
            bytecnt = 0;
            pr = FALSE;
        }
        else if (size == 0)
        {
            buf[len] = '\0';
            DPrintf("%s", buf);
            len = 0;
            bytecnt = 0;
        }
    }
    DPrintf("=======================================");
}


U08 isForUs(U32 destIP){
#if ENABLE_LOOPBACK
    if (destIP == INADDR_LOOPBACK)
		return TRUE;
#endif
#if (DM9KS_ETHERNET_ENABLE || DM9621_ETHERNET_ENABLE)
	if(NicArray[NIC_INDEX_ETHER].u32DefaultIp == destIP)
		return TRUE;
#endif
	if(NicArray[NIC_INDEX_PPP].u32DefaultIp == destIP)
		return TRUE;
	if(NicArray[NIC_INDEX_WIFI].u32DefaultIp == destIP)
		return TRUE;
	return FALSE;
}

// Get the current default net driver handling index
U08 NetDefaultNicGet()
{
    return u08DefaultNicIndex;
}

void NetDefaultNicSet(U08 if_index)
{
    u08DefaultNicIndex = if_index;
	mpDebugPrint("u08DefaultNicIndex = %d", u08DefaultNicIndex);
}

U08 NetNicGet(U32 ipAddr)
{
    if (ipAddr == INADDR_ANY)
        return u08DefaultNicIndex;

	if (NicArray[NIC_INDEX_WIFI].u32DefaultIp == ipAddr)
		return NIC_INDEX_WIFI;
	else if (NicArray[NIC_INDEX_ETHER].u32DefaultIp == ipAddr)
		return NIC_INDEX_ETHER;
    else if (NicArray[NIC_INDEX_PPP].u32DefaultIp == ipAddr)
		return NIC_INDEX_PPP;
    else
		return NIC_INDEX_NULL;
}


struct net_device * NetNicDeviceGet(int if_index)
{
    return &NicArray[if_index];
}

void NetDefaultIpSet(U08 if_index, U32 ipAddr)
{
    NicArray[if_index].u32DefaultIp = ipAddr;
}

U32 NetDefaultIpGet()
{
    return NicArray[u08DefaultNicIndex].u32DefaultIp;
}

void NetSubnetMaskSet(U08 if_index, U32 snMask)
{
    NicArray[if_index].u32SubnetMask = snMask;
}

U32  NetSubnetMaskGet()
{
    return NicArray[u08DefaultNicIndex].u32SubnetMask;
}

void NetGatewayIpSet(U08 if_index, U32 gwAddr)
{
    NicArray[if_index].u32GatewayAddr = gwAddr;
}

U32  NetGatewayIpGet()
{
    return NicArray[u08DefaultNicIndex].u32GatewayAddr;
}

/* 
 * Index 0 Primary DNS server
 * Index 1 Secondary DNS server
 */
void NetDNSSet(U08 if_index, U32 DnsAddr,U08 index)
{
    if (index == 0 || index == 1)
    {
        NicArray[if_index].u32DnsAddr[index] = DnsAddr;
    }
}

/* 
 * Index 0 Primary DNS server
 * Index 1 Secondary DNS server
 */
U32  NetDNSGet(U08 index)
{

    if (index != 0 && index != 1)
		return INADDR_NONE;

	if(NicArray[u08DefaultNicIndex].u32DnsAddr[index] != 0)
    {
        return NicArray[u08DefaultNicIndex].u32DnsAddr[index];
    }
    else
        return INADDR_NONE;
}

/* 
 * Index 0 Primary DNS server
 * Index 1 Secondary DNS server
 */
U32 NetDNSGet2(U08 if_index, U08 index)
{
	if(NicArray[if_index].u32DnsAddr[index] != 0)
        return NicArray[if_index].u32DnsAddr[index];
    else
        return INADDR_NONE;
}


U08* NetNullMACGet()
{
    return NULL_MAC_ADDR;
}

U08* NetBroadcastMacGet()
{
    return BROADCAST_MAC_ADDR;
}

U32 NetBroadcaseIpGet()
{
    return BROADCAST_IP_ADDR;
}

extern BYTE myethaddr[6];
U08* NetLocalMACGet()
{
#ifndef MP600
    return NicArray[u08DefaultNicIndex].dev_addr;
#else
    return myethaddr;
#endif
}

void *NetDhcpSet(U32 dhcp, int if_idx)
{
    NicArray[if_idx].u32Dhcp = dhcp;
    return &NicArray[if_idx];
}

U32  NetDhcpGet(int if_idx)
{
    return NicArray[if_idx].u32Dhcp; /* return dhcp struct of an interface */
}

/*
 * NetHostNameSet:
 *
 * Set Internet host name of this system.
 *
 * Host names up to 255 characters are supported.  This host name will be
 * passed to DHCP server.
 *
 * Return values:
 *   NONE
 */
void NetHostNameSet(char *nm)
{
    MP_DEBUG("NetHostNameSet() %s",nm);
    memset(netHostName, 0, sizeof(netHostName));
    snprintf(netHostName, sizeof(netHostName),"%s",nm);
}

/*
 * NetHostNameGet:
 *
 * Retrieve the Internet host name of this system.
 *
 * Return values:
 *   non-NULL     = string of host name is returned.
 *   NULL         = No host name defined.
 */
char *NetHostNameGet(void)
{
    if (netHostName[0] == '\0')
        return NULL;
    else
    {
        MP_DEBUG("NetHostNameGet() %s",netHostName);
        return netHostName;
    }
}

void NetPrintPacketInfo(ST_NET_PACKET* packet)
{
    //ST_IP_PACKET* pstIPPacket = (ST_IP_PACKET*)packet;
    ST_ETHER_HEADER etherHeader;
    ST_IP_HEADER ipHeader;
    U08 ip[4];
    
    memcpy(&etherHeader, NET_PACKET_ETHER(packet), sizeof(ST_ETHER_HEADER));
    memcpy(&ipHeader, NET_PACKET_IP(packet), sizeof(ST_IP_HEADER));
    
    DPrintf(" ");
    DPrintf("=================================="); 
    DPrintf("From MAC: %2x-%2x-%2x\\-", etherHeader.u08SrcMacAddr[0],
                etherHeader.u08SrcMacAddr[1],
                etherHeader.u08SrcMacAddr[2]);
    DPrintf("-%2x-%2x-%2x", etherHeader.u08SrcMacAddr[3],
                etherHeader.u08SrcMacAddr[4],
                etherHeader.u08SrcMacAddr[5]);
    ip[0] = (ipHeader.u32SrcIp >> 24) & 0xff;
    ip[1] = (ipHeader.u32SrcIp >> 16) & 0xff;
    ip[2] = (ipHeader.u32SrcIp >> 8) & 0xff;
    ip[3] = ipHeader.u32SrcIp & 0xff;
    DPrintf("From IP: %3d.%3d.%3d.%3d", ip[0], ip[1], ip[2], ip[3]); 
    
    DPrintf("To MAC: %2x-%2x-%2x\\-", etherHeader.u08DstMacAddr[0],
                etherHeader.u08DstMacAddr[1],
                etherHeader.u08DstMacAddr[2]);
    DPrintf("-%2x-%2x-%2x", etherHeader.u08DstMacAddr[3],
                etherHeader.u08DstMacAddr[4],
                etherHeader.u08DstMacAddr[5]);
    ip[0] = (ipHeader.u32DstIp >> 24) & 0xff;
    ip[1] = (ipHeader.u32DstIp >> 16) & 0xff;
    ip[2] = (ipHeader.u32DstIp >> 8) & 0xff;
    ip[3] = ipHeader.u32DstIp & 0xff;
    DPrintf("To IP: %3d.%3d.%3d.%3d", ip[0], ip[1], ip[2], ip[3]);
    
    switch(ipHeader.u08Protocol)
    {
        case IP_PROTOCOL_ICMP:
            DPrintf("Protocol: ICMP");
        break;
        
        case IP_PROTOCOL_TCP:
            DPrintf("Protocol: TCP");
        break;
        
        case IP_PROTOCOL_UDP:
            DPrintf("Protocol: UDP");
        break;
    }
    
    DPrintf("==================================");
    DPrintf(" ");
}



///
///@brief   NIC driver send a interrupt service request to the net driver
///
///@param   request The point of ST_NET_MESSAGE, used to transfer the NIC driver point and the request 
///                 message to the net driver.
///
///@return  The return value is the same as the mpx_MessageSend function.
///
///@remark  It is not recommemded that the NIC driver occupy a long period to perform the ISR procedure
///         in the NIC ISR. NIC driver had better separate it's procedures and complete them by the 
///         net driver. By this function, NIC driver has a way to inform its ISR request to the net
///         driver and ask the net driver to complete them by call mpx_NicSendDone, mpx_NicReceive and
///         mpx_NicRequest.
///
S32 mpx_NetRequest(ST_NIC_REQUEST *request)
{
    //return mpx_MessageSend(u08NetDeviceEnventId, (U08 *)request, sizeof(ST_NIC_REQUEST));
    return 0;
}

/* 
 * If DRIVER_FREE_TXBUFFER is defined, the buffer pointed by <packet> will be 
 * freed in this function or in the low level driver.
 */
S32 NetPacketSend(ST_NET_PACKET *packet, U32 destIp, U16 u16Type)
{
    S32 status;
    U08 dstMac[6];
    int ret;

	if(destIp == BROADCAST_IP_ADDR)
	{
		//mpDebugPrint("NetPacketSend 1");
        NetMacAddrCopy(dstMac, BROADCAST_MAC_ADDR);
	}
    else if(destIp == 0){
        //yuming, add for fix mac
        ST_SOCKET *socket;
        U16 sid = packet->Net.u16SockId;
        socket = SockGet(sid);
        NetMacAddrCopy(dstMac, socket->Peer.u08Address);
    }
    else if(IN_MULTICAST(destIp)){              /* IP multicast */
        U32 lowaddr=htonl(destIp & 0x007fffff);
        lowaddr = htonl(lowaddr);
        memcpy(&dstMac[2], &lowaddr, 4);
        dstMac[0] = 0x01;
        dstMac[1] = 0x00;
        dstMac[2] = 0x5e;
    }
    else
    {
        U08 retry = 3;
        if(!NetIpMaskCompare(destIp, NetDefaultIpGet(), NetSubnetMaskGet())) //outside local network
        {
            destIp = NetGatewayIpGet();
            if(destIp == 0)
            {
                NetFreePacket(packet);
                return -ENETUNREACH;
            }
        }
        else if(NetIpMaskCompare(destIp, BROADCAST_IP_ADDR, ~NetSubnetMaskGet())) //directed broadcast
        {
            NetMacAddrCopy(dstMac, BROADCAST_MAC_ADDR);
            destIp = 0;
        }
        
        if (destIp)
        {
        while(retry)
        {
            if(NO_ERR == (ret = ArpQuery(destIp, dstMac)))
                break;
            else
            {
                if (ret == -ENETDOWN)
                {
                    NetFreePacket(packet);
                    return -ENETDOWN;
                }
                DPrintf("[NIC] ARP fail, retry...");
            }
            
            retry--;
        }
        if(retry == 0)
        {
            NetFreePacket(packet);
            return -EHOSTUNREACH;
        }
    }
    }
    
    //if(packet->Net.u08NetIndex == NIC_INDEX_ETHER || packet->Net.u08NetIndex == NIC_INDEX_WIFI)
    {
        U08* ptr = NET_PACKET_ETHER(packet);
        
        NetMacAddrCopy(ptr, dstMac); //Ether.u08DstMacAddr
        ptr += 6;
        NetMacAddrCopy(ptr, NetLocalMACGet()); //Ether.u08SrcMacAddr
        ptr += 6;
        
        NetPutW(ptr, u16Type); //Ether.u16TyepLength
        //DPrintf("NetPacketSend: dst=%02x:%02x:%02x", dstMac[3], dstMac[4], dstMac[5]);
    }
    //else
    //{
    //    DPrintf("unknown nic selected");
    //    return 0;
    //}
    
    packet->Net.len = packet->Net.u16PayloadSize + ETHERNET_HEADER_SIZE;
                U08 *udp = NET_PACKET_UDP(packet);
                U16 fromPort = NetGetW(&udp[UDP_SRC_PORT]);
//    DPrintf("NetPacketSend: len=%d, fport=%d", packet->Net.len, fromPort);
//    packet->Net.data = NET_PACKET_ETHER(packet); //Ether /* TODO redundant like x=x; */
    status = packet->Net.u16PayloadSize;
    u32TotalTxSize += packet->Net.u16PayloadSize;
    
    #if _TRANSMISSION_STATUS
    if((u32TotalTxSize - u32PrevTxSize) >= 0x20000)
    {
        u32PrevTxSize = u32TotalTxSize;
        DPrintf(">\\-");
    }
    #endif
    if (!(NicArray[packet->Net.u08NetIndex].flags & IFF_UP))
    {
		mpDebugPrint("!!!flags & IFF_UP!!!");
        NetFreePacket(packet);
        return -ENETDOWN;
    }
	//mpDebugPrint("u08NetSemaphoreId");    
    mpx_SemaphoreWait(u08NetSemaphoreId);
#ifndef MP600
    ret = NicArray[NetDefaultNicGet()].hard_start_xmit((struct sk_buff*)packet, &NicArray[NetDefaultNicGet()]);
#else
	/*
	 * Fill up proper skb fields.  Driver will need them.
	 */
	struct sk_buff *skb = (struct sk_buff*)packet;
	skb_reset_mac_header(skb);
	skb_set_network_header(skb, ETHERNET_HEADER_SIZE);
	skb_set_transport_header(skb, ETHERNET_HEADER_SIZE);
	skb_set_tail_pointer(skb, skb->len);
	skb->protocol = cpu_to_be16(u16Type);
//	mpDebugPrint("%s: o1=%d,o2=%d,o3=%d", __func__, offsetof(struct sk_buff, data), offsetof(ST_NET_PACKET, Net), offsetof(ST_HEADER, RESERVED));    
//	mpDebugPrint("%s: o1=%d,o2=%d,o3=%d", __func__, offsetof(struct sk_buff, RESERVED), offsetof(ST_HEADER, data), offsetof(ST_HEADER, truesize));    
	//mpDebugPrint("%p",NicArray[NIC_INDEX_ETHER].hard_start_xmit);
    ret = NicArray[packet->Net.u08NetIndex].hard_start_xmit((struct sk_buff*)packet, &NicArray[packet->Net.u08NetIndex]);

#endif
    mpx_SemaphoreRelease(u08NetSemaphoreId);

#ifdef DRIVER_FREE_TXBUFFER
    if (ret)
        NetFreePacket(packet);
#endif
    
    return status;
}

U08* NetPacketSrcMacGet(ST_NET_PACKET* packet)
{
    U08* srcMac = NET_PACKET_ETHER(packet) + 6;
    return srcMac;
}

U32 rxpkts1;
#if Make_USB
extern BYTE wifi_device_type;
#endif
S32 NetPacketReceive(ST_NET_PACKET* packet)
{
    
	U32 u32Message[2];
    MP_DEBUG("%s", __FUNCTION__);
    u32LastReceiveTime = mpx_SystemTickerGet();
    
    u32TotalRxSize += packet->Net.u16PayloadSize;
    
    #if _TRANSMISSION_STATUS
    if((u32TotalRxSize - u32PrevRxSize) >= 0x20000)
    {
        u32PrevRxSize = u32TotalRxSize;
        DPrintf("<\\-");
    }
    #endif
    
    if(packet->Net.u08NetIndex == NIC_INDEX_ETHER || packet->Net.u08NetIndex == NIC_INDEX_WIFI)
    {
        U16 packetType = NetGetW(((U08*)NET_PACKET_ETHER(packet) + 12));
        
		MP_DEBUG("%s: t=%x", __FUNCTION__, packetType);
        if(packetType == ETHERNET_TYPE_ARP)
            ArpPacketReceive(packet);
        else
		{
#if (DM9KS_ETHERNET_ENABLE || DM9621_ETHERNET_ENABLE)
            if (packet->Net.u08NetIndex == NIC_INDEX_ETHER)
                NetBufferPacketReceive(packet);
            else
#endif
			switch(wifi_device_type)
			{
				case WIFI_USB_DEVICE_AR2524:
				case WIFI_USB_DEVICE_AR9271:
				case WIFI_USB_DEVICE_RTL8188C:
				default:

                if (wifi_device_type == WIFI_USB_DEVICE_RTL8188E)
                {
                    /* forward ETH_P_PAE packets directly to wpa_supplicant */
                    packet = NetBufferPacketReceive2(packet);
                    if (!packet)
                        break;
                }

				u32Message[0] = NET_PACKET_RECEIVE;
				u32Message[1] = (U32)packet;
				if(NO_ERR != mpx_MessageDrop(u08NetReceiveMessageId, (U08 *)u32Message, sizeof(u32Message)))
				{
					mpDebugPrint("message queue(u08NetReceiveMessageId) full");
					//BREAK_POINT();
					netBufFree(packet);
					mpx_TaskYield(1);
				}
				else
				rxpkts1++;
					break;
				case WIFI_USB_DEVICE_RT73:
					NetBufferPacketReceive(packet);
					break;
				
			}
        }
    }
    else
    {
        mpDebugPrint("unknown device??");
        MP_ASSERT(0);
        netBufFree(packet);
    }
    
    return 0;
}



void NetNicInit()
{
    
    u08DefaultNicIndex = NIC_INDEX_NULL;
    memset(NicArray, 0, sizeof(NicArray));
    
    strcpy(NicArray[NIC_INDEX_WIFI].name, "wlan0");
	NicArray[NIC_INDEX_WIFI].ifindex = NIC_INDEX_WIFI;
    NicArray[NIC_INDEX_WIFI].u32DefaultIp = 0;
    NicArray[NIC_INDEX_WIFI].u32SubnetMask = 0;
    NicArray[NIC_INDEX_WIFI].u32GatewayAddr = 0;
    NicArray[NIC_INDEX_WIFI].u32Dhcp = 0;
    NicArray[NIC_INDEX_WIFI].boolConnected = FALSE;
	
	strcpy(NicArray[NIC_INDEX_PPP].name, "ppp0");
	NicArray[NIC_INDEX_PPP].ifindex = NIC_INDEX_PPP;
	NicArray[NIC_INDEX_PPP].u32DefaultIp = 0;
	NicArray[NIC_INDEX_PPP].u32SubnetMask = 0;
	NicArray[NIC_INDEX_PPP].u32GatewayAddr = 0;
	NicArray[NIC_INDEX_PPP].u32Dhcp = 0;
	NicArray[NIC_INDEX_PPP].boolConnected = FALSE;

	strcpy(NicArray[NIC_INDEX_ETHER].name, "eth0");
	NicArray[NIC_INDEX_ETHER].ifindex = NIC_INDEX_ETHER;
	NicArray[NIC_INDEX_ETHER].u32DefaultIp = 0;
	NicArray[NIC_INDEX_ETHER].u32SubnetMask = 0;
	NicArray[NIC_INDEX_ETHER].u32GatewayAddr = 0;
	NicArray[NIC_INDEX_ETHER].u32Dhcp = 0;
	NicArray[NIC_INDEX_ETHER].boolConnected = FALSE;		

}



S32 mpx_NetworkStart(U08 u08NicIdx)
{
    DPrintf("mpx_NetworkStart u08NicIdx %d",u08NicIdx);
    
    if((u08NicIdx != NIC_INDEX_WIFI) && (u08NicIdx != NIC_INDEX_ETHER) && (u08NicIdx != NIC_INDEX_PPP))
    {
        DPrintf("[NIC] unknown net interface");
        return ERR_NETWORK_INIT_FAIL;
    }
    
    u08DefaultNicIndex = u08NicIdx;

	if(u08DefaultNicIndex == NIC_INDEX_ETHER|| u08DefaultNicIndex == NIC_INDEX_WIFI){
#ifndef MP600
    //if(NIC_INDEX_ETHER == NetNicSelectGet())
    if(NicArray[u08DefaultNicIndex].init)
    {
        if(!NicArray[u08DefaultNicIndex].init(&NicArray[u08DefaultNicIndex]))
            return ERR_NETWORK_INIT_FAIL;
    }
    
    if(NicArray[u08DefaultNicIndex].open){
        if(NicArray[u08DefaultNicIndex].open(&NicArray[u08DefaultNicIndex])){
            if(u08DefaultNicIndex == NIC_INDEX_ETHER)
                DPrintf("[NIC] Ethernet Driver Init Success\n");
            else
            {
                U08 mac[6];
                
                DPrintf("[NIC] Wifi Driver Init Success\n");
                if(0 < mpx_WlanGetMacAddress(&mac))
                {
                    DPrintf("[NIC] Can't get WIFI MAC address");
                    return ERR_NETWORK_INIT_FAIL;
                }
                
                NetMacAddrCopy(NicArray[NIC_INDEX_WIFI].dev_addr, mac);
                DPrintf("mac = %2x:%2x:%2x:\\-",
                    NicArray[NIC_INDEX_WIFI].dev_addr[0],
                    NicArray[NIC_INDEX_WIFI].dev_addr[1],
                    NicArray[NIC_INDEX_WIFI].dev_addr[2]);
                DPrintf("%2x:%2x:%2x",
                    NicArray[NIC_INDEX_WIFI].dev_addr[3],
                    NicArray[NIC_INDEX_WIFI].dev_addr[4],
                    NicArray[NIC_INDEX_WIFI].dev_addr[5]);
            }
        }
        else{
            if(u08DefaultNicIndex == NIC_INDEX_ETHER)
                DPrintf("[NIC] Ethernet Driver Init Fail\n");
            else
                DPrintf("[NIC] Wifi Driver Init Fail\n");
            return ERR_NETWORK_INIT_FAIL;
        }
    }
#else
    {
        U08 mac[6];

        DPrintf("[NIC] Wifi Driver Init Success\n");
        if(0 < mpx_WlanGetMacAddress(&mac))
        {
            DPrintf("[NIC] Can't get WIFI MAC address");
            return ERR_NETWORK_INIT_FAIL;
        }

        NetMacAddrCopy(NicArray[NIC_INDEX_WIFI].dev_addr, mac);
        DPrintf("MAC = %02x:%02x:%02x:%02x:%02x:%02x",
                NicArray[NIC_INDEX_WIFI].dev_addr[0],
                NicArray[NIC_INDEX_WIFI].dev_addr[1],
                NicArray[NIC_INDEX_WIFI].dev_addr[2],
                NicArray[NIC_INDEX_WIFI].dev_addr[3],
                NicArray[NIC_INDEX_WIFI].dev_addr[4],
                NicArray[NIC_INDEX_WIFI].dev_addr[5]);

        NetMacAddrCopy(NicArray[NIC_INDEX_WIFI].dev_addr, mac);
//        NicArray[NIC_INDEX_WIFI].flags |= IFF_UP;           /* TODO dev.open */
    }
#endif
	}
	else if(u08DefaultNicIndex == NIC_INDEX_PPP){
		
	}
	
    DPrintf("[NIC] network driver init finish");
    
    NetCtrlStart();
	
    return NO_ERR;
}

S32 mpx_NetworkStop()
{
    u08DefaultNicIndex = NIC_INDEX_NULL;
    NetCtrlStop();
    return NO_ERR;
}

BOOL mpx_NetLinkStatusGet(U08 if_index)
{
    return NicArray[if_index].boolConnected;
}

void mpx_NetLinkStatusSet(U08 if_index, BOOL conn)
{
    short i;
    NicArray[if_index].boolConnected = conn;
    if (conn)
    {
        if (if_index > u08DefaultNicIndex)
            u08DefaultNicIndex = if_index;
    }
    else
    {
        if (if_index == u08DefaultNicIndex)
        {
            for (i=if_index-1; i>0; i--)
            {
                if (NicArray[i].boolConnected)
                {
                    u08DefaultNicIndex = if_index;
                    break;
                }
            }

#if PPP_ENABLE
            if (i==0)
                u08DefaultNicIndex = NIC_INDEX_NULL;
#endif
        }
    }
}

U32 NetInterfaceIpGet(char *if_name)
{
    if (strcmp(if_name, "wlan0") == 0)
        return NicArray[NIC_INDEX_WIFI].u32DefaultIp;
    else if (strcmp(if_name, "ppp0") == 0)
        return NicArray[NIC_INDEX_PPP].u32DefaultIp;
    else
    {
        MP_ASSERT(0);
        return INADDR_ANY;
    }
}

S32 mpx_NetOpen()
{
    DPrintf("[NIC] unimplemented");
    BREAK_POINT();
    return 0;
}

S32 mpx_NetClose()
{
    DPrintf("[NIC] unimplemented");
    BREAK_POINT();
    return 0;
}

U08 GetNetMessageId(){
    return u08NetDeviceEnventId;
}

U08 GetNetEventId()
{
    return u08NetDeviceEnventId;
}

U08 GetNetTaskId(){
    return u08NetDeviceTaskId;
}

U08 GetNetSemaphoreId()
{
    return u08NetSemaphoreId;
}

#if 0
void NetDeviceTask()
{
    ST_NIC_REQUEST  request;
    S32 status;
    U32 mask;
    
    DPrintf("NetDeviceTask: 1");
#ifndef MP600
    status = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_WAIT_SINGLE|OS_ATTR_EVENT_CLEAR, 0);
    if(status < 0)
    {
        DPrintf("[NIC] NetTask event create fail");
        BREAK_POINT();
    }
    else
        u08NetDeviceEnventId = (U08)status;
    
    /* Create Semaphore */
    status = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
    if(status < 0){
        DPrintf("NetTask: Semaphore Create Fail");
        BREAK_POINT();
    }
    else
        u08NetSemaphoreId = (U08)status;
#endif
        
    DPrintf("NetDeviceTask: event id = %d, semaphore id = %d", u08NetDeviceEnventId, u08NetSemaphoreId);
    
    while(1)
    {
        mask = 0;
        mpx_EventWait(u08NetDeviceEnventId, NIC_REQUEST_INTERRUPT, OS_EVENT_OR, &mask);
        DPrintf("NetDeviceTask: 1");
        
#ifndef MP600
        request.Nic = &NicArray[u08DefaultNicIndex];
        request.u32Request = 0;
        
        if((mask & NIC_REQUEST_INTERRUPT) && (request.Nic->hard_int_handler))
        {
            request.Nic->hard_int_handler(&request);
        
            if(request.u32Request & NIC_REQUEST_SEND)
            {
                DPrintf("NIC_REQUEST_SEND");
            }
    
            if(request.u32Request & NIC_REQUEST_RECEIVE)
            {
                request.Nic->hard_receive(&NicArray[NetDefaultNicGet()]);
            }
            
            if(request.u32Request & NIC_REQUEST_LINK_STATE_CHANGED)
            {
                DPrintf("NIC_REQUEST_LINK_STATE_CHANGED");
            }
            
            //enable interrupt
            if(request.Nic->hard_int_enable)
                request.Nic->hard_int_enable();
        }
#endif
    }
}
#endif



extern U32 rxpkts1, rxpkts3;
U32 rxpkts2;
extern void *allocs[100];
void NetReceiveTask()
{
    S32 status;
    U32 message[8];
    
    DPrintf("NetReceiveTask: 1");
#ifndef MP600
    status = mpx_MessageCreate(OS_ATTR_FIFO, 640);
    if(status < 0)
    {
        DPrintf("NetReceiveTask: message create fail");
        BREAK_POINT();
    }
    
    u08NetReceiveMessageId = (U08)status;
#endif
    DPrintf("NetReceiveTask: 1");
    while(1)
    {
        status = mpx_MessageReceive(u08NetReceiveMessageId, (U08*)message);
        
        if (status > 0)
        {
            rxpkts2++;
//            DPrintf("NetReceiveTask: r1=%d,r2=%d,r3=%d", rxpkts1,rxpkts2,rxpkts3);
#if 0
            int i;
            for (i=0;i<100;i++)
            {
                if (allocs[i] == (void *)message[1])
                    break;
            }
            if (i==100)
                DPrintf("NetReceiveTask: error mem=0x%x", message[1]);
#endif
            switch(message[0])
            {
                case NET_PACKET_RECEIVE:
                    NetBufferPacketReceive((ST_NET_PACKET*)(message[1]));
                    break;
                default:
                    DPrintf("NetReceiveTask: error message");
                    break;
            }
        }
        else
            DPrintf("NetReceiveTask: mpx_MessageReceive returns no message");
    }
}



void NetworkTaskInit()
{
    S32 status;
    U08 i;
    
#ifdef MP600
    status = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_WAIT_SINGLE|OS_ATTR_EVENT_CLEAR, 0);
    if(status < 0)
    {
        DPrintf("[NIC] NetTask event create fail");
        BREAK_POINT();
    }
    else
        u08NetDeviceEnventId = (U08)status;
    
    /* Create Semaphore */
    status = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
    if(status < 0){
        DPrintf("NetTask: Semaphore Create Fail");
        BREAK_POINT();
    }
    else
        u08NetSemaphoreId = (U08)status;
#endif

#ifdef MP600
//    status = mpx_TaskCreate(NetDeviceTask, NETWORK_TASK_PRI-1, TASK_STACK_SIZE*2);
    status = 0;
#else
    status = mpx_TaskCreate(NetDeviceTask, NETWORK_TASK_PRI+1, TASK_STACK_SIZE*2);
#endif
    if(status < 0)
    {
        DPrintf("NetDeviceTask create fail");
        BREAK_POINT();
    }
    u08NetDeviceTaskId = (U08)status;
    
    DPrintf("NetDeviceTask id = %d", u08NetDeviceTaskId);
    mpx_TaskStartup(u08NetDeviceTaskId);
    
#ifdef MP600
    status = mpx_MessageCreate(OS_ATTR_FIFO, 640);
    if(status < 0)
    {
        DPrintf("NetReceiveTask: message create fail");
        BREAK_POINT();
    }
    
    u08NetReceiveMessageId = (U08)status;
#endif
    status = mpx_TaskCreate(NetReceiveTask, NETWORK_TASK_PRI, TASK_STACK_SIZE*2);
    if(status < 0)
    {
        DPrintf("NetReceiveTask create fail");
        BREAK_POINT();
    }
    u08NetReceiveTaskId = (U08)status;
    DPrintf("NetReceiveTask id = %d", u08NetReceiveTaskId);
    mpx_TaskStartup(u08NetReceiveTaskId);
    DPrintf("NetworkTaskInit 1");
    
    mpx_TaskYield(10);
    
    NetCtrlInit();
    DPrintf("NetworkTaskInit 2");
    
    return;
}


S32 mpx_WlanOpen(){
    U08 ret;

    ret = NicArray[NIC_INDEX_WIFI].open(&NicArray[NIC_INDEX_WIFI]);
    return ret;
}

S32 mpx_WlanClose(){
    U08 ret;

     ret = NicArray[NIC_INDEX_WIFI].stop(&NicArray[NIC_INDEX_WIFI]);
     return ret;
}

S32 mpx_WlanGetMacAddress(U08* mac){
    U08 ret;

    ret = 0;
    memcpy(mac,myethaddr, ETH_ALEN);	   
    return ret;
}

S32 mpx_WlanSetMacAddress(U08* mac){
    U08 ret;

    ret = NicArray[NIC_INDEX_WIFI].set_mac_address(&NicArray[NIC_INDEX_WIFI], mac);
    return ret;
}

S32 mpx_WlanScan(wlan_ap_table* ap_table){
    U08 ret;    

    ret = NicArray[NIC_INDEX_WIFI].wlan_scan(&NicArray[NIC_INDEX_WIFI], ap_table);
    return ret;
}

S32 mpx_WlanSpecificSSIDScan(U08* ssid, wlan_ap_table* ap_table){
    U08 ret;

    ret = NicArray[NIC_INDEX_WIFI].wlan_specificSSID_scan(&NicArray[NIC_INDEX_WIFI], ssid, ap_table);
    return ret;
}
#if 0
S32 mpx_WlanConnect(U08* ssid, U08 key_mgmt, U08 auth_alg, U08 key_index, U08* wep_key, U08* psk){
#else
S32 mpx_WlanConnect(U08* ssid){
#endif
    U08 ret;
    S32 status;
    
#if 0
    ret = NicArray[NIC_INDEX_WIFI].wlan_connect(&NicArray[NIC_INDEX_WIFI], ssid, key_mgmt, auth_alg, key_index, wep_key, psk);
#else
    ret = NicArray[NIC_INDEX_WIFI].wlan_connect(&NicArray[NIC_INDEX_WIFI], ssid);
#endif

    
    //status = NetTimerInstall(NetWifiMonitorTimerProc, NET_TIMER_1_SEC);
    //if(status < 0)
    //{
    //    DPrintf("[NIC] wifi dummy write timer create fail");
    //    BREAK_POINT();
    //}
    //u08NetWifiMonitorTimerId = (U08)status;
    
    //NetTimerRun(u08NetWifiMonitorTimerId);
    
    return ret;
}

S32 mpx_WlanDisconnect(){
    U08 ret;

    ret = NicArray[NIC_INDEX_WIFI].wlan_disconnect(&NicArray[NIC_INDEX_WIFI]);
    return ret;
}

/*
 * @ingroup    WIFI
 * @brief    Set wireless mode of operation
 *
 * @param    mode    Mode of operation
 * 
 * @retval  0   Success.
 * @retval  -EBUSY Wireless hardware is busy.
 * @retval  -EINVAL Unsupported mode.
 * @retval  WLAN_STATUS_FAILURE Command failed.
 * 
 * @remark  The <mode> can be IW_MODE_ADHOC, IW_MODE_INFRA, or IW_MODE_AUTO.
 */
S32 mpx_WlanSetMode(U32 mode){

    return NicArray[NIC_INDEX_WIFI].wlan_setmode(&NicArray[NIC_INDEX_WIFI], mode);
}

S32 mpx_WlanInfo(){
     NicArray[NIC_INDEX_WIFI].wlan_info(&NicArray[NIC_INDEX_WIFI]);
     return 0;
}

S32 mpx_WlanSetWEP(U32 flags, U08 key_index, U08* wep_key){
    U08 ret;

    ret = NicArray[NIC_INDEX_WIFI].wlan_setWEP(&NicArray[NIC_INDEX_WIFI], flags, key_index, wep_key);
    return ret;
}

extern struct net_device g_net_device2;
struct net_device *NetDeviceGetByIfname(U08* ifname){
    U08 index = 0;
    
    for(index; index < NIC_DRIVER_MAX; index++){
        if(strcmp(NicArray[index].name, ifname) == 0)
            return &NicArray[index];
    }
#ifdef HAVE_HOSTAPD
    if(strcmp(g_net_device2.name, ifname) == 0)
        return &g_net_device2;
#endif
    return NULL;
}

#ifndef MP600
S32 netLoadConfigFile()
{
    FILE_HANDLE configFile = 0;
    S32 fileStatus = NO_ERR;
    
    if(NO_ERR == mpx_DirNodeLocate("\0m\0p\0x\0n\0e\0t\0.\0c\0f\0g\0\0"))
    {
        fileStatus = mpx_FileOpen();
        if(fileStatus > 0)
        {
            U32 fileId = 0;
            
            configFile = (FILE_HANDLE)fileStatus;
            mpx_FileRead(configFile, &fileId, 4);
            if(fileId != 0x20050515)
                return -1;
                
            mpx_FileRead(configFile, myMACAddr, 6);
            DPrintf("Local device address: %2x-%2x-%2x\\-", myMACAddr[0],
                myMACAddr[1],
                myMACAddr[2]);
            DPrintf("-%2x-%2x-%2x", myMACAddr[3],
                myMACAddr[4],
                myMACAddr[5]);
            
            mpx_FileClose(configFile);
            return NO_ERR;
        }
    }
    
    return -1;
}
#endif

void NetNicDataRateGet(U32* outTxBytes, U32* outRxBytes)
{
    if(outTxBytes)
        *outTxBytes = u32TotalTxSize;
        
    if(outRxBytes)
        *outRxBytes = u32TotalRxSize;
}

ST_NET_PACKET* NetNewPacket(BOOL clear)
{
#if 0
    ST_NET_PACKET* packet = (ST_NET_PACKET*)mpx_Malloc(sizeof(ST_NET_PACKET));
#else
    ST_NET_PACKET* packet = (ST_NET_PACKET*)netBufAlloc(sizeof(ST_NET_PACKET));
#endif
    
    if(packet)
    {
#if 0
        if(clear)
            memset(packet, 0, sizeof(ST_NET_PACKET));
#endif
#ifndef MP600
        packet->Net.data = packet->u08Data;
#else
#if Make_SDIO
        packet->Net.data = packet->Net.RESERVED; /* SDIO WIFI needs this */
#endif
#endif
    }
    
    return packet;
}

ST_NET_PACKET* NetNewPacketWithSize(U16 size, BOOL clear)
{
#if 0
    ST_NET_PACKET* packet = (ST_NET_PACKET*)mpx_Malloc(sizeof(ST_NET_PACKET) + size);
#else
    ST_NET_PACKET* packet = (ST_NET_PACKET*)netBufAlloc(size);
#endif
    
    if(packet)
    {
        if(clear)
            memset(packet, 0, sizeof(ST_NET_PACKET) + size);
#ifndef MP600
        packet->Net.data = packet->u08Data;
#else
#if Make_SDIO
        packet->Net.data = packet->Net.RESERVED;
#endif
#endif
    }
    
    return packet;
}

void NetFreePacket(ST_NET_PACKET* packet)
{
#if 0
    mpx_Free(packet);
#else
    netBufFree(packet);
#endif
}

void NetFreePacketMem(ST_NET_PACKET* packet)
{
    net_buf_mem_free(packet->Net.head);
    packet->Net.head = NULL;
    packet->Net.data = NULL;
}

void NetWifiMonitorTimerProc()
{
    if((mpx_SystemTickerGet() - u32LastReceiveTime) >= 250)
    {
        DPrintf("ouch!! wifi is dead??");
        if(NicArray[NIC_INDEX_WIFI].wlan_custom_proc)
            NicArray[NIC_INDEX_WIFI].wlan_custom_proc(0);
    }
}

static unsigned char default_operstate(const struct net_device *dev);
void netdev_state_change(struct net_device *dev);

unsigned dev_get_flags(const struct net_device *dev)
{
	unsigned flags;

	flags = (dev->flags & ~(IFF_RUNNING |
				IFF_LOWER_UP |
				IFF_DORMANT));

    if (netif_running(dev)) {
		if (netif_oper_up(dev))
			flags |= IFF_RUNNING;
		if (netif_carrier_ok(dev))
			flags |= IFF_LOWER_UP;
		if (netif_dormant(dev))
			flags |= IFF_DORMANT;
	}

	return flags;
}

static unsigned char default_operstate(const struct net_device *dev)
{
	if (!netif_carrier_ok(dev))
		return IF_OPER_DOWN;

	if (netif_dormant(dev))
		return IF_OPER_DORMANT;

	return IF_OPER_UP;
}

void rfc2863_policy(struct net_device *dev)
{
	unsigned char operstate = default_operstate(dev);

	if (operstate == dev->operstate)
		return;

	switch(dev->link_mode) {
	case IF_LINK_MODE_DORMANT:
		if (operstate == IF_OPER_UP)
			operstate = IF_OPER_DORMANT;
		break;

	case IF_LINK_MODE_DEFAULT:
	default:
		break;
	}

	dev->operstate = operstate;
}

void set_operstate(struct net_device *dev, unsigned char transition)
{
	unsigned char operstate;

    if (!dev)
        dev = &NicArray[NIC_INDEX_WIFI];

    operstate = dev->operstate;

    MP_DEBUG("set_operstate");
	switch(transition) {
	case IF_OPER_UP:
		if ((operstate == IF_OPER_DORMANT ||
		     operstate == IF_OPER_UNKNOWN) &&
		    !netif_dormant(dev))
			operstate = IF_OPER_UP;
		break;

	case IF_OPER_DORMANT:
		if (operstate == IF_OPER_UP ||
		    operstate == IF_OPER_UNKNOWN)
			operstate = IF_OPER_DORMANT;
		break;
	}

	if (dev->operstate != operstate) {
		dev->operstate = operstate;
		netdev_state_change(dev);
	}
}

void set_linkmode(struct net_device *dev, unsigned char link_mode)
{
    if (dev)
        dev->link_mode = link_mode;
    else
        NicArray[NIC_INDEX_WIFI].link_mode = link_mode;
}

/**
 *	netdev_state_change - device changes state
 *	@dev: device to cause notification
 *
 *	Called to indicate a device has changed state. This function calls
 *	the notifier chains for netdev_chain and sends a NEWLINK message
 *	to the routing socket.
 */
void netdev_state_change(struct net_device *dev)
{
	unsigned flags;
	if (dev->flags & IFF_UP) {
        NetLinkStateEventSet(dev);
	}
}

unsigned mpx_dev_get_flags(void)
{
    return dev_get_flags(&NicArray[NIC_INDEX_WIFI]);
}
