#define LOCAL_DEBUG_ENABLE 0

#include <string.h>
#include <sys/time.h>
#include "linux/types.h"
#include "global612.h"
#include "typedef.h"
#include "socket.h"
#include "net_dhcp.h"
#include "os.h"
#include "net_netctrl.h"
#include "net_nic.h"
#include "ndebug.h"

#define DEBUG_DHCP      1

static U32 dhcpXid = 0x012DAE8E;
static U08 u08DhcpTimerId = 0xff;
static BOOL dhcp_initialized;

DHCP_STATUS_CALLBACK dhcpStatusCallback = 0;

/* network configurations */
#ifdef HAVE_HOSTAPD
BOOLEAN net_ipv4_method_dhcp = FALSE;
#else
BOOLEAN net_ipv4_method_dhcp = TRUE;
#endif

/* TODO: no default gateway */
BYTE net_ipv4_fixed_address[16] = "192.168.77.1";
BYTE netcfg_ipv4_fixed_netmask[16]= "255.255.255.0";
BYTE netcfg_ipv4_gateway[16]= "0.0.0.0";

static ST_NET_PACKET* dhcpCreateRequest(BOOL new_transaction, struct net_device *dev);
static U16 dhcpOptionTrailer(ST_NET_PACKET* packet, U16 u16OptionLen);
static U08* dhcpOptionGet(ST_NET_PACKET* packet, U08 u08OptionType);
static void dhcpHandleOffer(ST_NET_PACKET* packet);
static void dhcpHandleAck(ST_NET_PACKET* packet);
static void dhcpHandleNak(ST_NET_PACKET* packet);
static S32 dhcpDiscover(BOOL new_transaction);
static void dhcpSelect(int retries);
static void dhcpBind();
static void dhcpRelease(ST_DHCP* pstDhcpCtrl);
static void dhcpRebind(BOOL new_transaction);
static void dhcpRenew();
static void dhcpTimerProc();
static void dhcpTimeout();
static void dhcpLeaseTimeout();
static void dhcpRebindTimeout();
static void dhcpRenewTimeout();
static void DhcpPacketReceive(ST_NET_PACKET* packet, U32 u32SrcAddr, U16 u16SrcPort);

static inline void dhcpFreePacket(ST_NET_PACKET* packet)
{
    if (packet)
        NetFreePacket(packet);
}

static void dhcpTimerProc()
{
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    
//    MP_DEBUG("dhcpTimerProc is called");
    if(pstDhcpCtrl)
    {
        if (pstDhcpCtrl->u08NextState)
        {
            if (pstDhcpCtrl->u08NextState != pstDhcpCtrl->u08State)
            {
                if (pstDhcpCtrl->u08NextState == DHCP_STATE_OFF)
                    dhcpRelease(pstDhcpCtrl);
                else if (pstDhcpCtrl->u08NextState == DHCP_STATE_DISCOVERING)
                {
                    dhcpRelease(pstDhcpCtrl);
                    dhcpDiscover(TRUE);
                }
            }
            pstDhcpCtrl->u08NextState = 0;
        }

        if(pstDhcpCtrl->u32RequestTimeout > 1)
            pstDhcpCtrl->u32RequestTimeout--;
        else if(pstDhcpCtrl->u32RequestTimeout == 1)
        {
            pstDhcpCtrl->u32RequestTimeout--;
            MP_DEBUG("dhcpTimeout is called");
            dhcpTimeout();
        }
        
        if(pstDhcpCtrl->u32LeaseTimeout > 0)
        {
            if(--pstDhcpCtrl->u32LeaseTimeout == 0)
                dhcpLeaseTimeout();
        }

        if(pstDhcpCtrl->u32RebindTimeout > 0)
        {
            if(--pstDhcpCtrl->u32RebindTimeout == 0)
            dhcpRebindTimeout();
        }

        if(pstDhcpCtrl->u32RenewTimeout > 0)
        {
            pstDhcpCtrl->u32RenewTimeout--;
            if(pstDhcpCtrl->u32RenewTimeout == 0)
            dhcpRenewTimeout();
        }

        if (pstDhcpCtrl->u32LinkTimeout > 0)
        {
            pstDhcpCtrl->u32LinkTimeout--;
            if(pstDhcpCtrl->u32LinkTimeout == 0)
            {
                if(pstDhcpCtrl->u08State == DHCP_STATE_BOUND ||
                   pstDhcpCtrl->u08State == DHCP_STATE_REBINDING ||
                   pstDhcpCtrl->u08State == DHCP_STATE_RENEWING)
                {
                    if (!pstDhcpCtrl->u08LinkUp)
                    {
                        dhcpRelease(pstDhcpCtrl);
                        NetInterfaceEventSet(); /* notify network subsystem */
                    }
                    else
                    {
                        MP_DEBUG("dhcpTimerProc: force a renew");
                        pstDhcpCtrl->u08FastRenew = TRUE; /* force a renew */
                        if(pstDhcpCtrl->u08State == DHCP_STATE_BOUND)
                            pstDhcpCtrl->u32RenewTimeout = 1;
                    }
                }
            }
        }
    }
}

static void dhcpTimeout()
{
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    
    switch(pstDhcpCtrl->u08State)
    {
        case DHCP_STATE_DISCOVERING:
            if (pstDhcpCtrl->u08RetryCount >= DHCP_MAX_RETRIES)
                dhcpRelease(pstDhcpCtrl);
            else
            {
                pstDhcpCtrl->u08RetryCount++;
                dhcpDiscover(FALSE);                   /* re-transmission */
                pstDhcpCtrl->u32RequestTimeout += 2*pstDhcpCtrl->u08RetryCount;
            }
            break;
        case DHCP_STATE_BACKING_OFF:
        break;
        case DHCP_STATE_RENEWING:
        case DHCP_STATE_REBINDING:
            if (pstDhcpCtrl->u08RetryCount >= DHCP_MAX_RETRIES)
            {
                if (pstDhcpCtrl->u08FastRenew)
                {
                    pstDhcpCtrl->u08FastRenew = FALSE;
                    dhcpRelease(pstDhcpCtrl);
                    NetInterfaceEventSet(); /* notify network subsystem */
                    dhcpDiscover(TRUE);
                }
                else
                {
                    /* 
                     * Still no response.  Wait for a longer time (5 mins) to try again.
                     */
                    pstDhcpCtrl->u08RetryCount = -1;
                    pstDhcpCtrl->u32RequestTimeout = 300; /* 300 secs = 5 mins */
                }
            }
            else
            {
                pstDhcpCtrl->u08RetryCount++;
                if (pstDhcpCtrl->u08State == DHCP_STATE_REBINDING)
                {
                    if (pstDhcpCtrl->u08RetryCount == 0)
                        dhcpRebind(TRUE);
                    else
                        dhcpRebind(FALSE);
                }
                else if (pstDhcpCtrl->u08State == DHCP_STATE_RENEWING)
                {
                    if (pstDhcpCtrl->u08RetryCount == 0)
                        dhcpRenew(TRUE);
                    else
                        dhcpRenew(FALSE);
                }
                if (pstDhcpCtrl->u08FastRenew)
                {
                    pstDhcpCtrl->u32RequestTimeout = 6;
                    pstDhcpCtrl->u08RetryCount = DHCP_MAX_RETRIES;
                }
                else
                    pstDhcpCtrl->u32RequestTimeout += pstDhcpCtrl->u08RetryCount*4;
            }
        break;
        
        case DHCP_STATE_REQUESTING:
            if (pstDhcpCtrl->u08RetryCount >= DHCP_MAX_RETRIES)
                dhcpRelease(pstDhcpCtrl);
            else
            {
                pstDhcpCtrl->u08RetryCount++;
                dhcpSelect(pstDhcpCtrl->u08RetryCount);                   /* re-transmission */
                pstDhcpCtrl->u32RequestTimeout += 2*pstDhcpCtrl->u08RetryCount;
            }
            break;
        
        case DHCP_STATE_OFF:
            if (pstDhcpCtrl->dhcpDev->u32DefaultIp != INADDR_ANY)
                dhcpRelease(pstDhcpCtrl);                  /* release the lease */

            dhcpDiscover(TRUE);
        break;
    }
}

/* 
 * Lease expires
 */
static void dhcpLeaseTimeout()
{
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    
    pstDhcpCtrl->u08State = DHCP_STATE_OFF;
    pstDhcpCtrl->u32RequestTimeout = 1;
}

static void dhcpRebindTimeout()
{
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    
    pstDhcpCtrl->u08State = DHCP_STATE_REBINDING;
    pstDhcpCtrl->u08RetryCount = -1;
    pstDhcpCtrl->u32RequestTimeout = 1;
}

static void dhcpRenewTimeout()
{
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    
    pstDhcpCtrl->u08State = DHCP_STATE_RENEWING;
    pstDhcpCtrl->u08RetryCount = -1;
    pstDhcpCtrl->u32RequestTimeout = 1;
}

static ST_NET_PACKET* dhcpCreateRequest(BOOL new_transaction, struct net_device *dev)
{
    ST_NET_PACKET* packet = NetNewPacket(TRUE);
    U08 *dhcp;
    U16 i;
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(dev->ifindex);
    
    if(!packet)
        return 0;
    
    if (new_transaction)
        pstDhcpCtrl->u32Xid = dhcpXid++;
    
    dhcp = NET_PACKET_DHCP(packet);
    
    //clear DHCP header
    memset(dhcp,0x00,DHCP_HEADER_LEN);
	
    dhcp[DHCP_OP_CODE] = DHCP_BOOT_REQUEST;
    dhcp[DHCP_HW_TYPE] = DHCP_HW_TYPE_ETHER;
    dhcp[DHCP_HW_ADDR_LEN] = DHCP_HW_ADDR_LENGTH;
    dhcp[DHCP_HOPS] = 0;
    NetPutDW(&dhcp[DHCP_TRANS_ID], pstDhcpCtrl->u32Xid);
    NetPutW(&dhcp[DHCP_SECONDS], 0);
    NetPutW(&dhcp[DHCP_FLAGS], 0);
    NetPutDW(&dhcp[DHCP_CLIENT_IP], 0);
#if 1
	if(pstDhcpCtrl->u08State == DHCP_STATE_BOUND 
		|| pstDhcpCtrl->u08State == DHCP_STATE_RENEWING
		|| pstDhcpCtrl->u08State == DHCP_STATE_REBINDING){
    	NetPutDW(&dhcp[DHCP_CLIENT_IP], dev->u32DefaultIp);
	}
#endif
    NetPutDW(&dhcp[DHCP_YOUR_IP], 0);
    NetPutDW(&dhcp[DHCP_SERVER_IP], 0);
    NetPutDW(&dhcp[DHCP_RELAY_IP], 0);
    NetMacAddrCopy(&dhcp[DHCP_CLIENT_HW_ADDR], NetLocalMACGet());
    //dhcp[DHCP_SERVER_HOST_NAME] //leave this filed zero
    //dhcp[DHCP_BOOT_FILE_NAME] //leave this filed zero
    NetPutDW(&dhcp[DHCP_COOKIE], 0x63825363); //RFC1048 cookie
    
    //put an incrementing array for debugging
    for(i = 0; i < DHCP_OPTIONS_LEN; i++)
        dhcp[DHCP_OPTION+i] = i;
    
    return packet;
}

static U16 dhcpOptionTrailer(ST_NET_PACKET* packet, U16 u16OptionLen)
{
    U08* dhcp = NET_PACKET_DHCP(packet);
    U08* dhcpOption = &dhcp[DHCP_OPTION];
    
    dhcpOption[u16OptionLen++] = DHCP_OPTION_END;
    while((u16OptionLen < DHCP_OPTIONS_LEN) || (u16OptionLen & 0x3))
        dhcpOption[u16OptionLen++] = 0;
        
    return u16OptionLen;
}

static U08* dhcpOptionGet(ST_NET_PACKET* packet, U08 u08OptionType)
{
    U08* dhcp = NET_PACKET_DHCP(packet);
    U08* dhcpOption = &dhcp[DHCP_OPTION];
    U16 optionLen = packet->Net.u16PayloadSize - DHCP_HEADER_LEN;
    U16 offset = 0;
    U08 overload = DHCP_OVERLOAD_NONE;
    
    while((offset < optionLen) && (dhcpOption[offset] != DHCP_OPTION_END))
    {
        if(dhcpOption[offset] == DHCP_OPTION_OVERLOAD)
        {
            offset += 2;
            overload = dhcpOption[offset++];
            #ifdef DEBUG_DHCP
            MP_DEBUG("[DHCP] overload not supported currently");
            #endif
        }
        else if(dhcpOption[offset] == u08OptionType)
        {
            return (&dhcpOption[offset]);
        }
        else
        {
            offset++;
            offset += (1 + dhcpOption[offset]);
        }
    }
    
    return 0;
}

static void dhcpSelect(int retries)
{
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    ST_NET_PACKET* packet = dhcpCreateRequest(FALSE, pstDhcpCtrl->dhcpDev);
    U08* dhcp = NET_PACKET_DHCP(packet);
    U08* dhcpOption = &dhcp[DHCP_OPTION];
    U16 optionLen = 0;

	MP_DEBUG("dhcpSelect");
        
    if(packet)
    {
        dhcpOption[optionLen++] = DHCP_OPTION_MESSAGE_TYPE;
        dhcpOption[optionLen++] = DHCP_OPTION_MESSAGE_TYPE_LEN;
        dhcpOption[optionLen++] = DHCP_REQUEST;
        
        dhcpOption[optionLen++] = DHCP_OPTION_MAX_MSG_SIZE;
        dhcpOption[optionLen++] = DHCP_OPTION_MAX_MSG_SIZE_LEN;
        NetPutW(&dhcpOption[optionLen], 576);
        optionLen += 2;
        
        dhcpOption[optionLen++] = DHCP_OPTION_REQUESTED_IP;
        dhcpOption[optionLen++] = 4;
        NetPutDW(&dhcpOption[optionLen], pstDhcpCtrl->u32OfferedIpAddr);
        optionLen += 4;
        
        if (pstDhcpCtrl->u32ServerIpAddr)
        {
            dhcpOption[optionLen++] = DHCP_OPTION_SERVER_ID;
            dhcpOption[optionLen++] = 4;
            NetPutDW(&dhcpOption[optionLen], pstDhcpCtrl->u32ServerIpAddr);
            optionLen += 4;
        }

        dhcpOption[optionLen++] = DHCP_OPTION_PARAMETER_REQUEST_LIST;
        dhcpOption[optionLen++] = 4;
        dhcpOption[optionLen++] = DHCP_OPTION_SUBNET_MASK;
        dhcpOption[optionLen++] = DHCP_OPTION_ROUTER;
        dhcpOption[optionLen++] = DHCP_OPTION_BROADCAST;
        dhcpOption[optionLen++] = DHCP_OPTION_DNS_SERVER;
        
        /* ----------  add host name  ---------- */
        char *nm;
        int len;
        if (nm = NetHostNameGet())
        {
            len = strlen(nm);
            dhcpOption[optionLen++] = DHCP_OPTION_HOST_NAME;
            dhcpOption[optionLen++] = len;
            strcpy(&dhcpOption[optionLen], nm);
            optionLen += len;
        }

        optionLen = dhcpOptionTrailer(packet, optionLen);
        packet->Net.u16PayloadSize = DHCP_HEADER_LEN + optionLen;
        
        #ifdef DEBUG_DHCP
        MP_DEBUG("[DHCP] state change: DHCP_STATE_REQUESTING");
        #endif
        
        pstDhcpCtrl->u08State = DHCP_STATE_REQUESTING;
        UdpPacketSend_if(packet, 0, DHCP_CLIENT_PORT, NetBroadcaseIpGet(), DHCP_SERVER_PORT, pstDhcpCtrl->dhcpDev);
        pstDhcpCtrl->u32RequestTimeout = 3;         /* seconds */
        pstDhcpCtrl->u08RetryCount = retries;
    }
    else
    {
        #ifdef DEBUG_DHCP
        MP_DEBUG("[DHCP] dhcpSelect allocate buffer fail");
        #endif
    }
    
//    pstDhcpCtrl->u32RequestTimeout = DHCP_REQUEST_TIMEOUT;
}

static void dhcpRelease(ST_DHCP* pstDhcpCtrl)
{
    ST_NET_PACKET* packet;
    int nic;
    
    #ifdef DEBUG_DHCP
    MP_DEBUG("[DHCP]state change: DHCP_STATE_OFF");
    #endif
    
    pstDhcpCtrl->u08State = DHCP_STATE_OFF;
    
    /* stop all timers */
    pstDhcpCtrl->u32RequestTimeout = 
    pstDhcpCtrl->u32RebindTimeout = 
    pstDhcpCtrl->u32RenewTimeout = 
    pstDhcpCtrl->u32LeaseTimeout = 
    pstDhcpCtrl->u32LinkTimeout = 0;

    pstDhcpCtrl->u08FastRenew = FALSE;

    pstDhcpCtrl->u32ServerIpAddr = 0;    //dhcp server ip
    pstDhcpCtrl->u32OfferedIpAddr = 0;   //my ip
    pstDhcpCtrl->u32OfferedSnMask = 0;   //subnet mask
    pstDhcpCtrl->u32OfferedGwAddr = 0;   //gateway router
    pstDhcpCtrl->u32OfferedBcAddr = 0;   //broadcase address
    pstDhcpCtrl->u08DnsCount = 0;
    
    pstDhcpCtrl->u32OfferedLeaseTime = 0;
    pstDhcpCtrl->u32OfferedRenewTime = 0;
    pstDhcpCtrl->u32OfferedRebindTime = 0;
    if (pstDhcpCtrl->dhcpDev->u32DefaultIp != INADDR_ANY)
    {
        packet = dhcpCreateRequest(TRUE, pstDhcpCtrl->dhcpDev);
        if(packet)
        {
            U08* dhcp = NET_PACKET_DHCP(packet);
            U08* dhcpOption = &dhcp[DHCP_OPTION];
            U16 optionLen = 0;
            U32 message[6];

            dhcpOption[optionLen++] = DHCP_OPTION_MESSAGE_TYPE;
            dhcpOption[optionLen++] = DHCP_OPTION_MESSAGE_TYPE_LEN;
            dhcpOption[optionLen++] = DHCP_RELEASE;

            optionLen = dhcpOptionTrailer(packet, optionLen);

            message[0] = NETCTRL_MSG_UDP_PACKET_SEND;
            message[1] = (U32)packet;
            message[2] = 0;
            message[3] = DHCP_CLIENT_PORT;
            message[4] = NetBroadcaseIpGet();
            message[5] = DHCP_SERVER_PORT;
            mpx_MessageSend(NetCtrlMessageIdGet(), (U08 *)message, sizeof(message));
        }
    }

    nic = pstDhcpCtrl->dhcpDev->ifindex;
    NetDefaultIpSet(nic, 0);
	NetDNSSet(nic, 0, 0);
	NetDNSSet(nic, 0, 1);
    NetSubnetMaskSet(nic, 0);
    NetGatewayIpSet(nic, 0);
	mpx_NetLinkStatusSet(nic, FALSE);

#if PPP_ENABLE
	if(NicArray[NIC_INDEX_PPP].u32DefaultIp == INADDR_ANY)
		NetDefaultNicSet(NIC_INDEX_NULL);
	else
		NetDefaultNicSet(NIC_INDEX_PPP);
#endif
    pstDhcpCtrl->u32RequestTimeout = 10;    /* 30 secs */
}

static void dhcpRebind(BOOL new_transaction)
{
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    ST_NET_PACKET* packet;
    
    #ifdef DEBUG_DHCP
    MP_DEBUG("[DHCP] state change: DHCP_STATE_REBINDING");
    #endif
    
    pstDhcpCtrl->u08State = DHCP_STATE_REBINDING;
    packet = dhcpCreateRequest(new_transaction, pstDhcpCtrl->dhcpDev);
    if(packet)
    {
        U08* dhcp = NET_PACKET_DHCP(packet);
        U08* dhcpOption = &dhcp[DHCP_OPTION];
        U16 optionLen = 0;
        U32 message[6];
        
        dhcpOption[optionLen++] = DHCP_OPTION_MESSAGE_TYPE;
        dhcpOption[optionLen++] = DHCP_OPTION_MESSAGE_TYPE_LEN;
        dhcpOption[optionLen++] = DHCP_REQUEST;
        
        dhcpOption[optionLen++] = DHCP_OPTION_MAX_MSG_SIZE;
        dhcpOption[optionLen++] = DHCP_OPTION_MAX_MSG_SIZE_LEN;
        NetPutW(&dhcpOption[optionLen], 576);
        optionLen += 2;
        
        optionLen = dhcpOptionTrailer(packet, optionLen);
        packet->Net.u16PayloadSize = DHCP_HEADER_LEN + optionLen;
        
        message[0] = NETCTRL_MSG_UDP_PACKET_SEND;
        message[1] = (U32)packet;
        message[2] = 0;
        message[3] = DHCP_CLIENT_PORT;
        message[4] = NetBroadcaseIpGet();
        message[5] = DHCP_SERVER_PORT;
        mpx_MessageSend(NetCtrlMessageIdGet(), (U08 *)message, sizeof(message));
    }
    
    pstDhcpCtrl->u32RequestTimeout = DHCP_REQUEST_TIMEOUT;
}

static void dhcpRenew(BOOL new_transaction)
{
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    ST_NET_PACKET* packet;
    
    #ifdef DEBUG_DHCP
    MP_DEBUG("[DHCP] state change: DHCP_STATE_RENEWING");
    #endif
    
    pstDhcpCtrl->u08State = DHCP_STATE_RENEWING;
    packet = dhcpCreateRequest(new_transaction, pstDhcpCtrl->dhcpDev);
    if(packet)
    {
        U08* dhcp = NET_PACKET_DHCP(packet);
        U08* dhcpOption = &dhcp[DHCP_OPTION];
        U16 optionLen = 0;
        U32 message[6];
        
        dhcpOption[optionLen++] = DHCP_OPTION_MESSAGE_TYPE;
        dhcpOption[optionLen++] = DHCP_OPTION_MESSAGE_TYPE_LEN;
        dhcpOption[optionLen++] = DHCP_REQUEST;
        
        dhcpOption[optionLen++] = DHCP_OPTION_MAX_MSG_SIZE;
        dhcpOption[optionLen++] = DHCP_OPTION_MAX_MSG_SIZE_LEN;
        NetPutW(&dhcpOption[optionLen], 576);
        optionLen += 2;
        
        optionLen = dhcpOptionTrailer(packet, optionLen);
        packet->Net.u16PayloadSize = DHCP_HEADER_LEN + optionLen;
        
        message[0] = NETCTRL_MSG_UDP_PACKET_SEND;
        message[1] = (U32)packet;
        message[2] = NetDefaultIpGet();
        message[3] = DHCP_CLIENT_PORT;
        message[4] = pstDhcpCtrl->u32ServerIpAddr;
        message[5] = DHCP_SERVER_PORT;
        mpx_MessageSend(NetCtrlMessageIdGet(), (U08 *)message, sizeof(message));
    }
    
    pstDhcpCtrl->u32RequestTimeout = DHCP_REQUEST_TIMEOUT;
}

static void dhcpBind()
{
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    U32 snMask = pstDhcpCtrl->u32OfferedSnMask;
    U32 gwAddr = pstDhcpCtrl->u32OfferedGwAddr;
    int nic;
    
    pstDhcpCtrl->u32RenewTimeout = pstDhcpCtrl->u32OfferedRenewTime;
    if(pstDhcpCtrl->u32RenewTimeout == 0)
        pstDhcpCtrl->u32RenewTimeout = 1;
    
    pstDhcpCtrl->u32RebindTimeout = pstDhcpCtrl->u32OfferedRebindTime;
    if(pstDhcpCtrl->u32RebindTimeout == 0)
        pstDhcpCtrl->u32RebindTimeout = 1;
    
    pstDhcpCtrl->u32LeaseTimeout = pstDhcpCtrl->u32OfferedLeaseTime;

    if(snMask == 0)
        snMask = 0xffffff00;
        
    if(gwAddr == 0)
    {
        gwAddr = pstDhcpCtrl->u32OfferedIpAddr & snMask;
        gwAddr |= 0x00000001;
    }

    nic = pstDhcpCtrl->dhcpDev->ifindex;
    NetDefaultIpSet(nic, pstDhcpCtrl->u32OfferedIpAddr);
	NetDNSSet(nic, pstDhcpCtrl->u32OfferedDnsAddr[0], 0);
	NetDNSSet(nic, pstDhcpCtrl->u32OfferedDnsAddr[1], 1);
    NetSubnetMaskSet(nic, snMask);
    NetGatewayIpSet(nic, gwAddr);
	mpx_NetLinkStatusSet(nic, TRUE);
	NetDefaultNicSet(nic);
    
    #ifdef DEBUG_DHCP
    MP_DEBUG("[DHCP] state change: DHCP_STATE_BOUND");
    #endif
    
    pstDhcpCtrl->u32RequestTimeout = 0;
    pstDhcpCtrl->u08FastRenew = FALSE;
    pstDhcpCtrl->u08State = DHCP_STATE_BOUND;

    NetInterfaceEventSet();
}

static void dhcpHandleOffer(ST_NET_PACKET* packet)
{
    U08* dhcp = NET_PACKET_DHCP(packet);
    U08* dhcpOption = &dhcp[DHCP_OPTION];
    U08* optionPtr = dhcpOptionGet(packet, DHCP_OPTION_SERVER_ID);
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    
    if(optionPtr)
    {
        pstDhcpCtrl->u32ServerIpAddr = NetGetDW(&optionPtr[2]);
        pstDhcpCtrl->u32OfferedIpAddr = NetGetDW(&dhcp[DHCP_YOUR_IP]);
        mpDebugPrint("ZZZZZZZZZZZZZ%x %x",pstDhcpCtrl->u32ServerIpAddr,pstDhcpCtrl->u32OfferedIpAddr);
        dhcpSelect(0);
    }
}

static void dhcpHandleAck(ST_NET_PACKET* packet)
{
    U08* dhcp = NET_PACKET_DHCP(packet);
    U08* dhcpOption = &dhcp[DHCP_OPTION];
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    U08* optionPtr = 0;
    U08 i;
    
    pstDhcpCtrl->u32OfferedSnMask = 0;
    pstDhcpCtrl->u32OfferedGwAddr = 0;
    pstDhcpCtrl->u32OfferedBcAddr = 0;
    
    optionPtr = dhcpOptionGet(packet, DHCP_OPTION_LEASE_TIME);
    if(optionPtr)
        pstDhcpCtrl->u32OfferedLeaseTime = NetGetDW(optionPtr+2);
        
    optionPtr = dhcpOptionGet(packet, DHCP_OPTION_T1);
    if(optionPtr)
        pstDhcpCtrl->u32OfferedRenewTime = NetGetDW(optionPtr+2);
    else
        pstDhcpCtrl->u32OfferedRenewTime = pstDhcpCtrl->u32OfferedLeaseTime >> 1;
    
    optionPtr = dhcpOptionGet(packet, DHCP_OPTION_T2);
    if(optionPtr)
        pstDhcpCtrl->u32OfferedRebindTime = NetGetDW(optionPtr+2);
    else
        pstDhcpCtrl->u32OfferedRebindTime = (pstDhcpCtrl->u32OfferedLeaseTime >> 1) +
                                            (pstDhcpCtrl->u32OfferedLeaseTime >> 2);
        
    pstDhcpCtrl->u32OfferedIpAddr = NetGetDW(&dhcp[DHCP_YOUR_IP]);
    
    optionPtr = dhcpOptionGet(packet, DHCP_OPTION_SUBNET_MASK);
    if(optionPtr)
        pstDhcpCtrl->u32OfferedSnMask = NetGetDW(optionPtr+2);
        
    optionPtr = dhcpOptionGet(packet, DHCP_OPTION_ROUTER);
    if(optionPtr)
        pstDhcpCtrl->u32OfferedGwAddr = NetGetDW(optionPtr+2);
        
    optionPtr = dhcpOptionGet(packet, DHCP_OPTION_BROADCAST);
    if(optionPtr)
        pstDhcpCtrl->u32OfferedBcAddr = NetGetDW(optionPtr+2);
        
    optionPtr = dhcpOptionGet(packet, DHCP_OPTION_DNS_SERVER);
    if(optionPtr)
    {
        pstDhcpCtrl->u08DnsCount = optionPtr[1] >> 2;
        if(pstDhcpCtrl->u08DnsCount > DHCP_MAX_DNS)
            pstDhcpCtrl->u08DnsCount = DHCP_MAX_DNS;
        memset(&pstDhcpCtrl->u32OfferedDnsAddr[0], 0, sizeof(pstDhcpCtrl->u32OfferedDnsAddr));
        for(i = 0; i < pstDhcpCtrl->u08DnsCount; i++)
            pstDhcpCtrl->u32OfferedDnsAddr[i] = NetGetDW(&optionPtr[2+i*4]);
    }
    
    #ifdef DEBUG_DHCP
    DPrintf("[DHCP] ack");
    DPrintf("[DHCP] ip: \\-");
    NetDebugPrintIP(pstDhcpCtrl->u32OfferedIpAddr);
    DPrintf("[DHCP] server: \\-");
    NetDebugPrintIP(pstDhcpCtrl->u32ServerIpAddr);

    DPrintf("[DHCP] DNS server: \\-");
    NetDebugPrintIP(pstDhcpCtrl->u32OfferedDnsAddr[0]);
    NetDebugPrintIP(pstDhcpCtrl->u32OfferedDnsAddr[1]);
	
    DPrintf("[DHCP] sn mask: \\-");
    NetDebugPrintIP(pstDhcpCtrl->u32OfferedSnMask);
    DPrintf("[DHCP] gateway: \\-");
    NetDebugPrintIP(pstDhcpCtrl->u32OfferedGwAddr);
    DPrintf("[DHCP] offered lease time: 0x%x seconds", pstDhcpCtrl->u32OfferedLeaseTime);
    DPrintf("[DHCP] offered renew time: 0x%x seconds", pstDhcpCtrl->u32OfferedRenewTime);
    DPrintf("[DHCP] offered rebind time: 0x%x seconds", pstDhcpCtrl->u32OfferedRebindTime);
    #endif
}

static void dhcpHandleNak(ST_NET_PACKET* packet)
{
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    short to_send_event = FALSE;
    
    #ifdef DEBUG_DHCP
    MP_DEBUG("[DHCP] state change: DHCP_STATE_OFF");
    #endif
    
    if(pstDhcpCtrl->u08State == DHCP_STATE_BOUND ||
       pstDhcpCtrl->u08State == DHCP_STATE_REBINDING ||
       pstDhcpCtrl->u08State == DHCP_STATE_RENEWING)
        to_send_event = TRUE;

    dhcpRelease(pstDhcpCtrl);
    if (to_send_event)
        NetInterfaceEventSet();
}

static void DhcpPacketReceive(ST_NET_PACKET* packet, U32 u32SrcAddr, U16 u16SrcPort)
{
    U08* dhcp = NET_PACKET_DHCP(packet);
    U08* dhcpOption = &dhcp[DHCP_OPTION];
    U08* optionPtr = 0;
    U08 messageType;
    U16 optionOffset;
    U16 isSvrIdMatched;
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    
    //DPrintf("[DHCP] packet in");
    //NetPacketDump(packet, 64);
    if(dhcp[DHCP_OP_CODE] != DHCP_BOOT_REPLY)
        goto FINISH;
    
    if(!NetMacAddrComp(&dhcp[DHCP_CLIENT_HW_ADDR], NetLocalMACGet()))
    {
        goto FINISH;
    }
    
    if(NetGetDW(&dhcp[DHCP_TRANS_ID]) != pstDhcpCtrl->u32Xid)
    {
        #ifdef DEBUG_DHCP
        MP_DEBUG("[DHCP] Xid not match");
        #endif
        goto FINISH;
    }
    
    isSvrIdMatched = TRUE;
    optionPtr = dhcpOptionGet(packet, DHCP_OPTION_SERVER_ID);
    if (optionPtr)
    {
        U32 svr_ip = NetGetDW(&optionPtr[2]);
        if (pstDhcpCtrl->u32ServerIpAddr && (pstDhcpCtrl->u32ServerIpAddr != svr_ip))
        {
            isSvrIdMatched = FALSE;
            MP_DEBUG("[DHCP] Server ID mismatchs");
        }
    }

    if(packet->Net.u16PayloadSize <= DHCP_HEADER_LEN)
    {
        #ifdef DEBUG_DHCP
        MP_DEBUG("[DHCP] no option filed found");
        #endif
        goto FINISH;
    }
    
    optionPtr = dhcpOptionGet(packet, DHCP_OPTION_MESSAGE_TYPE);
    if(!optionPtr)
    {
        #ifdef DEBUG_DHCP
        MP_DEBUG("[DHCP] can not find option filed");
        #endif
        //NetPacketDump(dhcp, packet->Net.u16PayloadSize);
        goto FINISH;
    }
    
    messageType = optionPtr[2];
    if(messageType == DHCP_ACK)
    {
        if(pstDhcpCtrl->u08State == DHCP_STATE_REQUESTING)
        {
            #ifdef DEBUG_DHCP
            MP_DEBUG("[DHCP] ack/bind ip");
            #endif
            if (!isSvrIdMatched)
                goto FINISH;
            //NetPacketDump(NET_PACKET_ETHER(packet), 14);
            dhcpHandleAck(packet);
            pstDhcpCtrl->u32RequestTimeout = 0;
            dhcpBind();
            if(dhcpStatusCallback)
                dhcpStatusCallback(DHCP_STATUS_GET_IP_FINISH);
        }
        else if((pstDhcpCtrl->u08State == DHCP_STATE_REBOOTING) || (pstDhcpCtrl->u08State == DHCP_STATE_REBINDING) 
                    || (pstDhcpCtrl->u08State == DHCP_STATE_RENEWING))
        {
            #ifdef DEBUG_DHCP
            MP_DEBUG("[DHCP] renew/rebind ip");
            #endif
            pstDhcpCtrl->u32RequestTimeout = 0;
            
            optionPtr = dhcpOptionGet(packet, DHCP_OPTION_LEASE_TIME);
            if(optionPtr)
                pstDhcpCtrl->u32OfferedLeaseTime = NetGetDW(optionPtr+2);
                
            optionPtr = dhcpOptionGet(packet, DHCP_OPTION_T1);
            if(optionPtr)
                pstDhcpCtrl->u32OfferedRenewTime = NetGetDW(optionPtr+2);
            else
                pstDhcpCtrl->u32OfferedRenewTime = pstDhcpCtrl->u32OfferedLeaseTime >> 1;
            
            optionPtr = dhcpOptionGet(packet, DHCP_OPTION_T2);
            if(optionPtr)
                pstDhcpCtrl->u32OfferedRebindTime = NetGetDW(optionPtr+2);
            else
                pstDhcpCtrl->u32OfferedRebindTime = (pstDhcpCtrl->u32OfferedLeaseTime >> 1) +
                                                    (pstDhcpCtrl->u32OfferedLeaseTime >> 2);
            
            dhcpBind();
        }
        else
        {
            #ifdef DEBUG_DHCP
            MP_DEBUG("[DHCP] ouch");
            #endif
        }
    }
    else if((messageType == DHCP_NAK) && 
            ((pstDhcpCtrl->u08State == DHCP_STATE_REBOOTING) || (pstDhcpCtrl->u08State == DHCP_STATE_REQUESTING)
            || (pstDhcpCtrl->u08State == DHCP_STATE_REBINDING) || (pstDhcpCtrl->u08State == DHCP_STATE_RENEWING)))
    {
        #ifdef DEBUG_DHCP
        MP_DEBUG("[DHCP] NAK");
        #endif
        if (!isSvrIdMatched)
            goto FINISH;
        pstDhcpCtrl->u32RequestTimeout = 0;
        dhcpHandleNak(packet);
    }
    else if((messageType == DHCP_OFFER) && (pstDhcpCtrl->u08State == DHCP_STATE_DISCOVERING))
    {
        #ifdef DEBUG_DHCP
        MP_DEBUG("[DHCP] offer ip");
        #endif
        pstDhcpCtrl->u32RequestTimeout = 0;
        dhcpHandleOffer(packet);
    }
    else
    {
        #ifdef DEBUG_DHCP
        MP_DEBUG("[DHCP] RX packet type=%d discarded", messageType);
        #endif
    }
    
FINISH:
    dhcpFreePacket(packet);
}

S32 mpx_DhcpStart(int nic)
{
    S32 status = NO_ERR;
    ST_DHCP* pstDhcpCtrl = NetDhcpGet(nic);
    U32 counter = 0;
    U32 retry = 0;
    struct net_device *dev;
    
    MP_DEBUG("[DHCP] start");
    
    UdpOfficialPortRegister(DHCP_CLIENT_PORT, (U32)DhcpPacketReceive);
    
    if(!pstDhcpCtrl)
    {
        pstDhcpCtrl = (ST_DHCP*)mpx_Malloc(sizeof(ST_DHCP));
    
        if(!pstDhcpCtrl)
            return ERR_OUT_OF_MEMORY;
            
        pstDhcpCtrl->dhcpDev = NetDhcpSet((U32)pstDhcpCtrl, nic);
    }
    
    dev = pstDhcpCtrl->dhcpDev;
    memset(pstDhcpCtrl, 0, sizeof(ST_DHCP));
    pstDhcpCtrl->dhcpDev = dev;
    
    dhcpXid += mpx_SystemTickerGet();           /* give it some randomness */

    status = dhcpDiscover(TRUE);                /* Init the DHCP state machine */
    NetTimerRun(u08DhcpTimerId);
    
    while(1)
    {
//        if((pstDhcpCtrl->u08State == DHCP_STATE_BOUND) || (pstDhcpCtrl->u08State == DHCP_STATE_OFF))
        if((pstDhcpCtrl->u08State == DHCP_STATE_BOUND))
            break;
        
        mpx_TaskYield(10);
       		
        counter++;
        if(counter == 0x80)
        {
            counter = 0;
            //if (pstDhcpCtrl->u08State == DHCP_STATE_OFF)
            {
                DPrintf("[DHCP] discover again");
                //dhcpDiscover(TRUE);
                retry++;
                if(retry == 10)
                {
                    //pstDhcpCtrl->u08State = DHCP_STATE_OFF;
                    break;
                }
            }
        }
    }
    
    if(pstDhcpCtrl->u08State == DHCP_STATE_BOUND)
        status = NO_ERR;
    else
        status = -1;
    
    return status;
}

S32 mpx_DhcpManualIpSet(int ifindex, U32 u32IpAddr, U32 u32SubnetMask, U32 u32GatewayIp, U32 u32DnsServerIp)
{
    ST_DHCP* pstDhcpCtrl = NetDhcpGet(ifindex);
    struct net_device *dev;
    int nic;
    
    UdpOfficialPortRegister(DHCP_CLIENT_PORT, (U32)DhcpPacketReceive);
    
    if(!pstDhcpCtrl)
    {
        pstDhcpCtrl = (ST_DHCP*)mpx_Malloc(sizeof(ST_DHCP));
    
        if(!pstDhcpCtrl)
            return ERR_OUT_OF_MEMORY;
            
        pstDhcpCtrl->dhcpDev = NetDhcpSet((U32)pstDhcpCtrl, ifindex);
    }
    
    dev = pstDhcpCtrl->dhcpDev;
    memset(pstDhcpCtrl, 0, sizeof(ST_DHCP));
    pstDhcpCtrl->dhcpDev = dev;
    
    if(!u32IpAddr)
    {
        MP_DEBUG("[DHCP] invalid ip address");
        return -1;
    }
    
    pstDhcpCtrl->u32ServerIpAddr = 0;               //dhcp server ip
    pstDhcpCtrl->u32OfferedIpAddr = u32IpAddr;      //my ip
    pstDhcpCtrl->u32OfferedSnMask = u32SubnetMask;  //subnet mask
    pstDhcpCtrl->u32OfferedGwAddr = u32GatewayIp;   //gateway router
    pstDhcpCtrl->u32OfferedBcAddr = 0xffffffff;     //broadcase address
    pstDhcpCtrl->u32OfferedDnsAddr[0] = u32DnsServerIp;     //broadcase address
    pstDhcpCtrl->u32OfferedDnsAddr[1] = 0;
    pstDhcpCtrl->u08DnsCount = 1;
    
    if(!pstDhcpCtrl->u32OfferedSnMask)
        pstDhcpCtrl->u32OfferedSnMask = 0xffffff00;

    nic = pstDhcpCtrl->dhcpDev->ifindex;
    NetDefaultIpSet(nic, pstDhcpCtrl->u32OfferedIpAddr);
    NetSubnetMaskSet(nic, pstDhcpCtrl->u32OfferedSnMask);
    NetGatewayIpSet(nic, pstDhcpCtrl->u32OfferedGwAddr);
	NetDNSSet(nic, pstDhcpCtrl->u32OfferedDnsAddr[0], 0);
	NetDNSSet(nic, pstDhcpCtrl->u32OfferedDnsAddr[1], 1);
	mpx_NetLinkStatusSet(nic, TRUE);
	NetDefaultNicSet(nic);

    pstDhcpCtrl->u08State = DHCP_STATE_BOUND;

    pstDhcpCtrl->u08UseManualIp = TRUE;
    NetTimerStop(u08DhcpTimerId);

    NetInterfaceEventSet();
    return NO_ERR;
}

static S32 dhcpDiscover(BOOL new_transaction)
{
    ST_NET_PACKET* packet;
    U16 optionLen = 0;
    U08* dhcp;
    U08* dhcpOption;
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    
    if (!pstDhcpCtrl->u08LinkUp)
    {
        #ifdef DEBUG_DHCP
        MP_DEBUG("[DHCP] link down");
        #endif
        return -1;
    }
    packet = dhcpCreateRequest(new_transaction, pstDhcpCtrl->dhcpDev);
    if(!packet)
    {
        #ifdef DEBUG_DHCP
        MP_DEBUG("[DHCP] allocate packet fail");
        #endif
        return ERR_OUT_OF_MEMORY;
    }
    
    #ifdef DEBUG_DHCP
    MP_DEBUG("[DHCP] discover");
    #endif
    
    dhcp = NET_PACKET_DHCP(packet);
    dhcpOption = &dhcp[DHCP_OPTION];
    optionLen = 0;
    
    dhcpOption[optionLen++] = DHCP_OPTION_MESSAGE_TYPE;
    dhcpOption[optionLen++] = DHCP_OPTION_MESSAGE_TYPE_LEN;
    dhcpOption[optionLen++] = DHCP_DISCOVER;
    
    dhcpOption[optionLen++] = DHCP_OPTION_MAX_MSG_SIZE;
    dhcpOption[optionLen++] = DHCP_OPTION_MAX_MSG_SIZE_LEN;
    NetPutW(&dhcpOption[optionLen], 576);
    optionLen += 2;
    
    dhcpOption[optionLen++] = DHCP_OPTION_PARAMETER_REQUEST_LIST;
    dhcpOption[optionLen++] = 4;
    dhcpOption[optionLen++] = DHCP_OPTION_SUBNET_MASK;
    dhcpOption[optionLen++] = DHCP_OPTION_ROUTER;
    dhcpOption[optionLen++] = DHCP_OPTION_BROADCAST;
    dhcpOption[optionLen++] = DHCP_OPTION_DNS_SERVER;
    
    /* ----------  add host name  ---------- */
    char *nm;
    int len;
    if (nm = NetHostNameGet())
    {
        len = strlen(nm);
        dhcpOption[optionLen++] = DHCP_OPTION_HOST_NAME;
        dhcpOption[optionLen++] = len;
        strcpy(&dhcpOption[optionLen], nm);
        optionLen += len;
    }

    optionLen = dhcpOptionTrailer(packet, optionLen);
    
    packet->Net.u16PayloadSize = DHCP_HEADER_LEN + optionLen;
    
    #ifdef DEBUG_DHCP
    MP_DEBUG("[DHCP] state change: DHCP_STATE_DISCOVERING");
    #endif
    MP_DEBUG("%s: datalen=%d",__func__, packet->Net.u16PayloadSize);
    
    pstDhcpCtrl->u08State = DHCP_STATE_DISCOVERING;
    UdpPacketSend_if(packet, 0, DHCP_CLIENT_PORT, NetBroadcaseIpGet(), DHCP_SERVER_PORT, pstDhcpCtrl->dhcpDev);
    
//    pstDhcpCtrl->u32RequestTimeout = DHCP_REQUEST_TIMEOUT;
    pstDhcpCtrl->u32RequestTimeout = 1; /* seconds */
    if (new_transaction)
        pstDhcpCtrl->u08RetryCount = 0;
    
    return NO_ERR;
}

void DhcpInit()
{
    S32 status = NO_ERR;
    
    dhcpStatusCallback = 0;
    status = NetTimerInstall(dhcpTimerProc, NET_TIMER_1_SEC); //CJ modify
    //status = NetTimerInstall(dhcpTimerProc, NET_TIMER_1_SEC*30 ); //CJ modify
    if(status >= 0)
    {
        u08DhcpTimerId = (U08)status;
    }
}


S32 mpx_DhcpStatusCallbackSet(DHCP_STATUS_CALLBACK statusCallback)
{
    dhcpStatusCallback = statusCallback;
    return 0;
}

U32 mpx_DhcpIpAddrGet()
{
    return NetDefaultIpGet();
}

U32 mpx_DhcpSubnetMaskGet()
{
    return NetSubnetMaskGet();
}

U32 mpx_DhcpGatewayAddrGet()
{
    return NetGatewayIpGet();
}

U32 mpx_DhcpServerIpAddrGet()
{
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    return pstDhcpCtrl->u32ServerIpAddr;
}

U32 mpx_DhcpLeaseTimeGet()
{
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    if(pstDhcpCtrl->u08State == DHCP_STATE_BOUND ||
       pstDhcpCtrl->u08State == DHCP_STATE_REBINDING ||
       pstDhcpCtrl->u08State == DHCP_STATE_RENEWING)
        return pstDhcpCtrl->u32OfferedLeaseTime;
    else
        return 0;
}

U32 mpx_DhcpRemainedLeaseTimeGet()
{
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    if(pstDhcpCtrl->u08State == DHCP_STATE_BOUND ||
       pstDhcpCtrl->u08State == DHCP_STATE_REBINDING ||
       pstDhcpCtrl->u08State == DHCP_STATE_RENEWING)
        return pstDhcpCtrl->u32LeaseTimeout;
    else
        return 0;
}

U32 mpx_DhcpRemainedRenewTimeGet()
{
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    if(pstDhcpCtrl->u08State == DHCP_STATE_BOUND ||
       pstDhcpCtrl->u08State == DHCP_STATE_REBINDING ||
       pstDhcpCtrl->u08State == DHCP_STATE_RENEWING)
        return pstDhcpCtrl->u32RenewTimeout;
    else
        return 0;
}

U08 mpx_DhcpDnsAddrNumGet()
{
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    return pstDhcpCtrl->u08DnsCount;
}

U32 mpx_DhcpDnsAddrGet(U08 index)
{
    ST_DHCP* pstDhcpCtrl = (ST_DHCP*)NetDhcpGet(NetDefaultNicGet());
    if (index < DHCP_MAX_DNS)
        return pstDhcpCtrl->u32OfferedDnsAddr[index];
    else
        return 0;
}

S32 mpx_DhcpInform(U32 ipAddr)
{
    return 0;
}

void mpx_DhcpInit(struct net_device *dev)
{
    ST_DHCP* pstDhcpCtrl = NetDhcpGet(dev->ifindex);
    
    MP_DEBUG("[DHCP] init for interface %s", dev->name);
    
    if(!pstDhcpCtrl)
    {
        pstDhcpCtrl = (ST_DHCP*)mpx_Malloc(sizeof(ST_DHCP));
    
        if(!pstDhcpCtrl)
        {
            BREAK_POINT();
            return;
        }
            
        NetDhcpSet((U32)pstDhcpCtrl, dev->ifindex);
    }
    
    memset(pstDhcpCtrl, 0, sizeof(ST_DHCP));
    pstDhcpCtrl->dhcpDev = dev;
    
    if (net_ipv4_method_dhcp)
    {
        pstDhcpCtrl->u08UseManualIp = FALSE;
        dhcpRelease(pstDhcpCtrl);                /* Init the DHCP state machine */
    }
    else
    {
        struct in_addr addr;
        struct in_addr netmask;
        struct in_addr gw;

        /* use default IPv4 setttings */

        inet_aton(net_ipv4_fixed_address, &addr);
        inet_aton(netcfg_ipv4_fixed_netmask, &netmask);
        inet_aton(netcfg_ipv4_gateway, &gw);

        mpx_DhcpManualIpSet(dev->ifindex, addr.s_addr, netmask.s_addr, gw.s_addr, INADDR_ANY);

        pstDhcpCtrl->u08UseManualIp = TRUE;
    }

    
    return;
}

int mpx_DhcpRun(struct net_device *dev)
{
    ST_DHCP* pstDhcpCtrl = NetDhcpGet(dev->ifindex);
    
    MP_DEBUG("#########[DHCP] to get lease %d",dhcp_initialized);
    
    if (!dhcp_initialized)
    {
		mpDebugPrint("##############call UdpOfficialPortRegister");
        dhcp_initialized = TRUE;

        UdpOfficialPortRegister(DHCP_CLIENT_PORT, (U32)DhcpPacketReceive);
        dhcpXid += mpx_SystemTickerGet();           /* give it some randomness */
        pstDhcpCtrl->u08UseManualIp = FALSE;
        NetTimerRun(u08DhcpTimerId);
    }
    
    if (pstDhcpCtrl->u08NextState)
        return -EBUSY;

    if (pstDhcpCtrl->u08State == DHCP_STATE_OFF)
	{
        pstDhcpCtrl->u08NextState = DHCP_STATE_DISCOVERING;
	}
    return 0;
}

BOOL mpx_DhcpEnableGet(struct net_device *dev)
{
    ST_DHCP* pstDhcpCtrl = NetDhcpGet(dev->ifindex);
    return pstDhcpCtrl->u08UseManualIp ? FALSE : TRUE;
}

void mpx_DhcpEnableSet(BOOL dhcp_enabled, struct net_device *dev)
{
    ST_DHCP* pstDhcpCtrl = NetDhcpGet(dev->ifindex);
    pstDhcpCtrl->u08UseManualIp = dhcp_enabled ? FALSE : TRUE;
}

void mpx_DhcpLinkEventSend(BOOL link_up, struct net_device *dev)
{
    ST_DHCP* pstDhcpCtrl = NetDhcpGet(dev->ifindex);

	//Kevin Add for TMP
	//dhcp_initialized = 0;
	mpDebugPrint("link_up %d dhcp_initialized %d",link_up,dhcp_initialized);
    if (link_up && !dhcp_initialized)
        mpx_DhcpRun(dev);                       /* XXX */

    if (link_up)
    {
        /* link up */
        if (!pstDhcpCtrl->u08LinkUp)
        {
            pstDhcpCtrl->u08LinkUp = TRUE;
            if (pstDhcpCtrl->u08State == DHCP_STATE_OFF)
                pstDhcpCtrl->u32RequestTimeout = 1;
        }
    }
    else
    {
        /* link down */
        if (pstDhcpCtrl->u08LinkUp)
        {
            pstDhcpCtrl->u08LinkUp = FALSE;
            if (pstDhcpCtrl->u08State != DHCP_STATE_OFF)
            {
                pstDhcpCtrl->u32LinkTimeout = 3;    /* (re)start the timer (seconds) */
                MP_DEBUG("u32LinkTimeout is started");
            }
        }
    }
}
U08 mpx_DhcpGetTimerID()
{
   return u08DhcpTimerId;
}

