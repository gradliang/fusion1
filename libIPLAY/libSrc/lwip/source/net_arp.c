#define LOCAL_DEBUG_ENABLE 0

#include <string.h>
#include "linux/types.h"
#include "global612.h"
#include "lwip_config.h"
#include "typedef.h"
#include "socket.h"
#include "net_arp.h"
#include "os.h"
#include "net_netctrl.h"
#include "net_nic.h"

static ST_ARP_ENTRY arpTable[ARP_TABLE_SIZE];

typedef struct
{
    U32 u32IpAddr;
    U08 u08EtherAddr[6];
    S08 s08Index;
} ST_ARP_CACHE;

static ST_ARP_CACHE arpCache;
static U08 u08NetArpTimer = 0;


static void arpTimerProc()
{
    U08 i;
    
    for(i = 0; i < ARP_TABLE_SIZE; i++)
    {
 #if DM9KS_ETHERNET_ENABLE||DM9621_ETHERNET_ENABLE
 #if (P2P_TEST==0&&MAKE_XPG_PLAYER)
      if(!GetNetConfigP2PTestFlag())
 #else
      BYTE test = 0;
      if(test)
 #endif
 #endif
      {
        if (arpTable[i].eState > E_ARP_STATE_EXPIRED)
        {
          arpTable[i].u08Age++;
          if((arpTable[i].eState == E_ARP_STATE_STABLE) && (arpTable[i].u08Age > ARP_MAX_AGE))
            arpTable[i].eState = E_ARP_STATE_EXPIRED;
          else if((arpTable[i].eState == E_ARP_STATE_PENDING) && (arpTable[i].u08Age > ARP_MAX_PENDING))
            arpTable[i].eState = E_ARP_STATE_EXPIRED;
        }
      }
            
        if(arpTable[i].eState == E_ARP_STATE_EXPIRED)
        {
            DPrintf("[ARP] cache expired: (%2d) \\-", i);
            NetDebugPrintIP(arpTable[i].u32IpAddr);
            
            if(arpCache.u32IpAddr == arpTable[i].u32IpAddr)
            {
                arpCache.u32IpAddr = 0;
                memset(arpCache.u08EtherAddr, 0, 6);
                arpCache.s08Index = -1;
            }
            
            arpTable[i].eState = E_ARP_STATE_EMPTY;
            arpTable[i].u32IpAddr = 0;
            memset(arpTable[i].u08EtherAddr, 0, 6);
            arpTable[i].u08Age = 0;
        }
    }
}


static S32 arpFindEntry(U32 ipAddr)
{
    U08 u08Empty = ARP_TABLE_SIZE, u08OldStable = ARP_TABLE_SIZE, u08OldPending = ARP_TABLE_SIZE;
    U08 u08AgePending = 0, u08AgeStable = 0;
    U08 i;
    
    IntDisable();
    
    for(i = 0; i < ARP_TABLE_SIZE; i++)
    {
        if((u08Empty == ARP_TABLE_SIZE) && (arpTable[i].eState == E_ARP_STATE_EMPTY))
            u08Empty = i;
        else if(arpTable[i].eState == E_ARP_STATE_PENDING)
        {
            if(ipAddr == arpTable[i].u32IpAddr)
                goto FINISH;
            else
            {
                if(arpTable[i].u08Age >= u08AgePending)
                {
                    u08OldPending = i;
                    u08AgePending = arpTable[i].u08Age;
                }
            }
        }
        else if(arpTable[i].eState == E_ARP_STATE_STABLE)
        {
            if(ipAddr == arpTable[i].u32IpAddr)
                goto FINISH;
            else
            {
                if(arpTable[i].u08Age >= u08AgeStable)
                {
                    u08OldStable = i;
                    u08AgeStable = arpTable[i].u08Age;
                }
            }
        }
    }
    
    //no ip address fit the request ip address
    if(u08Empty < ARP_TABLE_SIZE) //there is an empty entry
        i = u08Empty;
    else if(u08OldStable < ARP_TABLE_SIZE) //no empty entry, reuse the oldest stable entry
        i = u08OldStable;
    else if(u08OldPending < ARP_TABLE_SIZE) //no stable entry, reuse the oldest pending entry
        i = u08OldPending;
    else
    {
        IntEnable();
        return -1; //ouch!!, nothing we can use!!
    }
        
    arpTable[i].eState = E_ARP_STATE_EMPTY;
    arpTable[i].u32IpAddr = ipAddr;
    arpTable[i].u08Age = 0;
    
FINISH:
    IntEnable();
    return i;
}



static void arpRefreshCache(U32 ipAddr, U08* macAddr, S08 entry)
{
    arpCache.u32IpAddr = ipAddr;
    NetMacAddrCopy(arpCache.u08EtherAddr, macAddr);
    if (entry < 0)
        arpCache.s08Index = -1;
    else
        arpCache.s08Index = entry;
}



S32 ArpWaitReplay(U08 entry)
{
    U32 start = GetSysTime();                   /* start time in ms */
    arpTable[entry].u08Age = 0;
    
    while(arpTable[entry].eState != E_ARP_STATE_STABLE)
    {
        if (((long)GetSysTime() - (long)start) > 1000 )
            return ARP_HOST_UNAVAILABLE;
        mpx_TaskYield(1);
        if(arpTable[entry].eState < E_ARP_STATE_PENDING)
        {
            DPrintf("[ARP] wait arp reply time out");
            return ARP_HOST_UNAVAILABLE;
        }
    }
    
    return NO_ERR;
}



S32 ArpRquest(U32 ipAddr)
{
    ST_NET_PACKET* packet = NetNewPacket(FALSE);
    ST_ARP_HEADER arpHeader;
    U08* ptr;
    U08* arpHdr = NET_PACKET_IP(packet);
    int ret;
    
    if(!packet)
    {
        DPrintf("[ARP] allocate buffer fail");
        return ERR_OUT_OF_MEMORY;
    }
    
    // put arp header
    NetPutW(&arpHdr[ARP_HARDWARE_TYPE], ARP_HW_TYPE_ETHERNET);
    NetPutW(&arpHdr[ARP_PORTOCOL_TYPE], ETHERNET_TYPE_IP);
    arpHdr[ARP_HW_ADDR_LENGTH] = ETHERNET_MAC_LENGTH;
    arpHdr[ARP_PROT_ADDR_LENGTH] = ETHERNET_IP_LENGTH;
    NetPutW(&arpHdr[ARP_OPERATION], ARP_OP_REQUEST);
    
    // put arp data
    ptr = &arpHdr[ARP_DATA];
    NetMacAddrCopy(ptr, NetLocalMACGet());
    ptr += ETHERNET_MAC_LENGTH;
    NetPutDW(ptr, NetDefaultIpGet());
    ptr += ETHERNET_IP_LENGTH;
    NetMacAddrCopy(ptr, NetNullMACGet());
    ptr += ETHERNET_MAC_LENGTH;
    NetPutDW(ptr, ipAddr);
    ptr += ETHERNET_IP_LENGTH;
    
    packet->Net.u16PayloadSize = sizeof(ST_ARP_HEADER);
    packet->Net.u08NetIndex = NetDefaultNicGet();
    
    ret = NetPacketSend(packet, NetBroadcaseIpGet(), ETHERNET_TYPE_ARP);
#ifndef DRIVER_FREE_TXBUFFER
    NetFreePacket(packet);
#endif
    
    if (ret < 0)
        return ret;
    else
        return NO_ERR;
}



S32 ArpReply(U32 ipAddr, U08* hwAddr)
{
    ST_NET_PACKET* packet = NetNewPacket(FALSE);
    U08* ptr;
    U08* arpHdr = NET_PACKET_IP(packet);
    
    if(!packet)
    {
        DPrintf("[ARP] allocate buffer fail");
        return ERR_OUT_OF_MEMORY;
    }
    
    // put arp header
    NetPutW(&arpHdr[ARP_HARDWARE_TYPE], ARP_HW_TYPE_ETHERNET);
    NetPutW(&arpHdr[ARP_PORTOCOL_TYPE], ETHERNET_TYPE_IP);
    arpHdr[ARP_HW_ADDR_LENGTH] = ETHERNET_MAC_LENGTH;
    arpHdr[ARP_PROT_ADDR_LENGTH] = ETHERNET_IP_LENGTH;
    NetPutW(&arpHdr[ARP_OPERATION], ARP_OP_REPLY);
    
    // put arp data
    ptr = &arpHdr[ARP_DATA];
    NetMacAddrCopy(ptr, NetLocalMACGet());
    ptr += ETHERNET_MAC_LENGTH;
    NetPutDW(ptr, NetDefaultIpGet());
    ptr += ETHERNET_IP_LENGTH;
    NetMacAddrCopy(ptr, hwAddr);
    ptr += ETHERNET_MAC_LENGTH;
    NetPutDW(ptr, ipAddr);
    ptr += ETHERNET_IP_LENGTH; 

    packet->Net.u16PayloadSize = sizeof(ST_ARP_HEADER);
    packet->Net.u08NetIndex = NetDefaultNicGet();
    
    {
        U32 message[4];
        
        if (LOCAL_DEBUG_ENABLE)
        {
            DPrintf("[ARP] reply to \\-");
            NetDebugPrintIP(ipAddr);
        }
        
        message[0] = NETCTRL_MSG_NET_PACKET_SEND;
        message[1] = (U32)packet;
        message[2] = ipAddr;
        message[3] = ETHERNET_TYPE_ARP;
        if (mpx_MessageSend(NetCtrlMessageIdGet(), message, sizeof(message)) != OS_STATUS_OK)
            DPrintf("ArpReply: MessageSend returns error");
    }
    
    return NO_ERR;
}



S32 ArpUpdateEntry(U32 ipAddr, U08* macAddr)
{
    S08 i;
    
    if((ipAddr == arpCache.u32IpAddr) && NetMacAddrComp(macAddr, arpCache.u08EtherAddr))
    {
        if (arpCache.s08Index >= 0)
            arpTable[arpCache.s08Index].u08Age = 0; /* reset the age */
        return NO_ERR;
    }
    
    i = arpFindEntry(ipAddr);
    if(i < 0)
        return -1;
    
    if(arpTable[i].eState != E_ARP_STATE_STABLE)
    {
        MP_DEBUG("[ARP] update entry(%1d)", i);
        NetMacAddrCopy(arpTable[i].u08EtherAddr, macAddr);
        arpTable[i].u08Age = 0;
        arpTable[i].eState = E_ARP_STATE_STABLE;
    }
    else
    {
        arpTable[i].u08Age = 0;
        if (!NetMacAddrComp(macAddr, arpTable[i].u08EtherAddr))
            NetMacAddrCopy(arpTable[i].u08EtherAddr, macAddr);
    }
    
    return i;
}



S32 ArpPacketReceive(ST_NET_PACKET* packet)
{
    U32 senderIP, targetIP;
    U08 *senderHW, *targetHW;
    U16 operation;
    U08 *arpData, *ptr;
    U08* arpHdr = NET_PACKET_IP(packet);
    S08 entry;
    
    if(INADDR_ANY == NetDefaultIpGet())
        goto ERROR;
    if((ARP_HW_TYPE_ETHERNET != NetGetW(&arpHdr[ARP_HARDWARE_TYPE])) || (ETHERNET_TYPE_IP != NetGetW(&arpHdr[ARP_PORTOCOL_TYPE])))
    {
        DPrintf("[ARP] unsupported packet received");
        //DPrintf("   HW type = %d, protcol type = %d", NetGetW(&arpHdr[ARP_HARDWARE_TYPE]), NetGetW(&arpHdr[ARP_PORTOCOL_TYPE]));
        NetPacketDump(NET_PACKET_ETHER(packet), packet->Net.u16PayloadSize);
        goto ERROR;
    }
    
    if((ETHERNET_MAC_LENGTH != arpHdr[ARP_HW_ADDR_LENGTH]) || (ETHERNET_IP_LENGTH != arpHdr[ARP_PROT_ADDR_LENGTH]))
    {
        DPrintf("[ARP] unsupported IP version");
        goto ERROR;
    }
    
    operation = NetGetW(&arpHdr[ARP_OPERATION]);
    
    arpData = &arpHdr[ARP_DATA];
    
    ptr = arpData + ETHERNET_MAC_LENGTH * 2 + ETHERNET_IP_LENGTH;
    targetIP = NetGetDW(ptr);
    if(targetIP != NetDefaultIpGet())
        goto ERROR;
    
    ptr = arpData;
    senderHW = ptr;
    ptr += ETHERNET_MAC_LENGTH;
    senderIP = NetGetDW(ptr);
    ptr += ETHERNET_IP_LENGTH;
    targetHW = ptr;
    
    if(operation == ARP_OP_REPLY)
    {
        if (LOCAL_DEBUG_ENABLE)
        {
            DPrintf("[ARP] arp reply from \\-");
            NetDebugPrintIP(senderIP);
        }
        
        entry = (S08)ArpUpdateEntry(senderIP, senderHW);
        arpRefreshCache(senderIP, senderHW, entry);
    }
    else if(operation == ARP_OP_REQUEST)
    {
        if (LOCAL_DEBUG_ENABLE)
        {
            DPrintf("[ARP] arp request from \\-");
            NetDebugPrintIP(senderIP);
        }
        
        ArpUpdateEntry(senderIP, senderHW);
        ArpReply(senderIP, senderHW);
    }
    else
    {
        DPrintf("[ARP] unsupported operation");
    }
    
ERROR:
    netBufFree(packet);
    return NO_ERR;
}

char CheckisARP(ST_NET_PACKET* packet)
{
    U32 senderIP, targetIP;
    U08 *senderHW;
    U16 operation;
    U08 *arpData, *ptr;
    U08* arpHdr = NET_PACKET_IP(packet);
    S08 entry;
    
    U16 packetType = NetGetW(((U08*)NET_PACKET_ETHER(packet) + 12));
    if(packetType != ETHERNET_TYPE_ARP)
        return 0;

    U08* dstMac = (U08*)NET_PACKET_ETHER(packet);
    BOOL bcast, forme;

    bcast = NetMacAddrComp(dstMac, NetBroadcastMacGet());
    forme = NetMacAddrComp(dstMac, NetLocalMACGet());

    if (!bcast && !forme)
        return -1;

	else
        return 1;

}

ST_NET_PACKET* ArpPacketReceive2(ST_NET_PACKET* packet)
{
    U32 senderIP, targetIP;
    U08 *senderHW;
    U16 operation;
    U08 *arpData, *ptr;
    U08* arpHdr = NET_PACKET_IP(packet);
    S08 entry;
    
    U16 packetType = NetGetW(((U08*)NET_PACKET_ETHER(packet) + 12));
    if(packetType != ETHERNET_TYPE_ARP)
        return packet;

    if(INADDR_ANY == NetDefaultIpGet())
        goto ERROR;

    l2_stat.rx_arp++;

    U08* dstMac = (U08*)NET_PACKET_ETHER(packet);
    BOOL bcast, forme;

    bcast = NetMacAddrComp(dstMac, NetBroadcastMacGet());
    forme = NetMacAddrComp(dstMac, NetLocalMACGet());

    if (!bcast && !forme)
        goto ERROR;                             /* drop the ARP packet */

    if (bcast)
        l2_stat.rx_bcast_arp++;
    else
        l2_stat.rx_ucast_arp++;

    arpData = &arpHdr[ARP_DATA];
    
    ptr = arpData + ETHERNET_MAC_LENGTH * 2 + ETHERNET_IP_LENGTH;
    targetIP = NetGetDW(ptr);
    if(targetIP != NetDefaultIpGet())
        goto ERROR;
    
    if((ETHERNET_MAC_LENGTH != arpHdr[ARP_HW_ADDR_LENGTH]) || (ETHERNET_IP_LENGTH != arpHdr[ARP_PROT_ADDR_LENGTH]))
    {
        DPrintf("[ARP] unsupported IP version");
        goto ERROR;
    }
    
    operation = NetGetW(&arpHdr[ARP_OPERATION]);
    
    ptr = arpData;
    senderHW = ptr;
    ptr += ETHERNET_MAC_LENGTH;
    senderIP = NetGetDW(ptr);
    
    if (operation == ARP_OP_REPLY)
    {
        if (LOCAL_DEBUG_ENABLE)
        {
            DPrintf("[ARP] arp reply from \\-");
            NetDebugPrintIP(senderIP);
        }
        
        entry = (S08)ArpUpdateEntry(senderIP, senderHW);
        arpRefreshCache(senderIP, senderHW, entry);
    }
    else if(operation == ARP_OP_REQUEST)
    {
        if (LOCAL_DEBUG_ENABLE)
        {
            DPrintf("[ARP] arp request from \\-");
            NetDebugPrintIP(senderIP);
        }
        
        ArpUpdateEntry(senderIP, senderHW);
        ArpReply(senderIP, senderHW);
    }
    else
    {
        DPrintf("[ARP] unsupported operation");
    }
    
ERROR:
    netBufFree(packet);
    return NULL;
}


S32 ArpInit()
{
    U08 i;
    S32 status;
    
    for(i = 0; i < ARP_TABLE_SIZE; i++)
    {
        arpTable[i].u32IpAddr = 0;
        memset(arpTable[i].u08EtherAddr, 0, 6);
        arpTable[i].eState = E_ARP_STATE_EMPTY;
        arpTable[i].u08Age = 0;
    }
    
    arpCache.u32IpAddr = 0;
    arpCache.s08Index = -1;
    memset(arpCache.u08EtherAddr, 0, 6);
    
    status = NetTimerInstall(arpTimerProc, 5 * NET_TIMER_1_SEC); //5 seconds
    if(status >= 0)
    {
        u08NetArpTimer = (U08)status;
        NetTimerRun(u08NetArpTimer);
    }
    
    return NO_ERR;
}



S32 ArpQuery(U32 ipAddress, U08* macAddress)
{
    S08 entry;
    int ret;
    
    if(ipAddress == arpCache.u32IpAddr)
    {
        NetMacAddrCopy(macAddress, arpCache.u08EtherAddr);
        return NO_ERR;
    }
    
    entry = arpFindEntry(ipAddress);
    
    if(entry < 0)
    {
        DPrintf("[ARP] arp table full");
        return ARP_PRIVATE_ERROR;
    }
        
    if(arpTable[entry].eState == E_ARP_STATE_EMPTY)
        arpTable[entry].eState = E_ARP_STATE_PENDING;
        
    if(arpTable[entry].eState == E_ARP_STATE_PENDING)
    {
        ret = ArpRquest(ipAddress);
        if (ret < 0)
            return ret;        /* don't wait if something wrong */
        if(NO_ERR != ArpWaitReplay(entry))
            return ARP_HOST_UNAVAILABLE;
    }
    
    if(arpTable[entry].eState == E_ARP_STATE_STABLE)
    {
        //DPrintf("[ARP] %2x-%2x-%2x\\-", arpTable[entry].u08EtherAddr[0],
        //        arpTable[entry].u08EtherAddr[1],
        //        arpTable[entry].u08EtherAddr[2]);
        //DPrintf("-%2x-%2x-%2x", arpTable[entry].u08EtherAddr[3],
        //        arpTable[entry].u08EtherAddr[4],
        //        arpTable[entry].u08EtherAddr[5]);
        NetMacAddrCopy(macAddress, arpTable[entry].u08EtherAddr);
        arpRefreshCache(arpTable[entry].u32IpAddr, arpTable[entry].u08EtherAddr, entry);
        return NO_ERR;
    }
    else
    {
        DPrintf("[ARP] host unavailale: \\-");
        NetDebugPrintIP(ipAddress);
        return ARP_HOST_UNAVAILABLE;
    }
}

void ArpDumpTable()
{
    U08 i;
    BOOL tableEmpty = TRUE;
    
    DPrintf(" ");
    DPrintf("================================================");
    DPrintf("[ARP] arp table");
            
    for(i = 0; i < ARP_TABLE_SIZE; i++)
    {
        if(arpTable[i].eState == E_ARP_STATE_STABLE)
        {
            tableEmpty = FALSE;
            DPrintf("   IP: \\-");
            NetDebugPrintIP(arpTable[i].u32IpAddr);
            DPrintf("   MAC: \\-");
            DPrintf("%2x-%2x-%2x\\-", arpTable[i].u08EtherAddr[0],
                arpTable[i].u08EtherAddr[1],
                arpTable[i].u08EtherAddr[2]);
            DPrintf("-%2x-%2x-%2x", arpTable[i].u08EtherAddr[3],
                arpTable[i].u08EtherAddr[4],
                arpTable[i].u08EtherAddr[5]);
            DPrintf("   Expired time: %d seconds", (ARP_MAX_AGE - arpTable[i].u08Age) * 5);
            DPrintf(" ");
        }
    }
    if(tableEmpty)
        DPrintf("   No entry found");
    DPrintf("================================================");
}


