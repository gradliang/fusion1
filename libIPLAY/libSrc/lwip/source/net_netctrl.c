#define LOCAL_DEBUG_ENABLE 0

#include <string.h>
#include <sys/time.h>
#include "linux/types.h"
#include "global612.h"
#include "lwip_config.h"
#include "typedef.h"
#include "socket.h"
#include "net_netctrl.h"
#include "net_socket.h"
#include "error.h"
#include "SysConfig.h"
#include "net_nic.h"
#include "ndebug.h"

ST_NET_BUF_LIST stPacketList[SOCK_MAXIMAL_NUMBER];
ST_NET_TIMER_REC netTimerBank[NET_TIMER_MAX];
static U08 u08NetCtrlTaskId;
static U08 u08NetCtrlMessageId;
U08 u08NetTimerId;

static void NetCtrlTask();
static void NetTimerProc();
U32 netBufferNodeRemove(ST_NET_BUF_LIST* list);
static S32 netBufferNodeAdd(ST_NET_BUF_LIST* list, U32 item);

struct l2_statistics l2_stat;

void NetTimerProc()
{
    U08 i;
    
//    DPrintf("NetTimerProc is called");
    for(i = 0; i < NET_TIMER_MAX; i++)
    {
        if((netTimerBank[i].used) && (netTimerBank[i].running))
        {
            netTimerBank[i].u32Counter++;
            if(netTimerBank[i].u32Counter >= netTimerBank[i].u32Interval)
            {
                (netTimerBank[i].proc)();
                netTimerBank[i].u32Counter = 0;
            }
        }
    }
}

S32 NetTimerInstall(NET_TIMER_CALLBACK proc, U32 interval)
{
    U08 i;
    
    mpx_SemaphoreWait(NET_SEM_ID);
    for(i = 0; i < NET_TIMER_MAX; i++)
    {
        if(!netTimerBank[i].used)
        {
            mpx_TimerPause(u08NetTimerId);
            netTimerBank[i].u32Interval = interval;
            netTimerBank[i].u32Counter = 0;
            netTimerBank[i].running = FALSE;
            netTimerBank[i].proc = proc;
            netTimerBank[i].used = TRUE;
            mpx_SemaphoreRelease(NET_SEM_ID);
            mpx_TimerResume(u08NetTimerId);
            return i;
        }
    }
    mpx_SemaphoreRelease(NET_SEM_ID);
    
    DPrintf("[NIC] net timer install fail");
    BREAK_POINT();
    return -1;
}

void NetTimerRun(U08 u08TimerId)
{
    if(u08TimerId < NET_TIMER_MAX)
    {
        netTimerBank[u08TimerId].running = TRUE;
    }
}

void NetTimerStop(U08 u08TimerId)
{
    if(u08TimerId < NET_TIMER_MAX)
    {
        netTimerBank[u08TimerId].running = FALSE;
        netTimerBank[u08TimerId].u32Counter = 0;
    }
}

void NetTimerDelete(U08 u08TimerId)
{
    if(u08TimerId < NET_TIMER_MAX)
    {
        netTimerBank[u08TimerId].running = FALSE;
        netTimerBank[u08TimerId].used = FALSE;
    }
}

void NetBufferInit()
{
    memset(stPacketList, 0, sizeof(stPacketList));
}

void NetBufferTypeSet(U16 sid, U16 u16Type)
{
    stPacketList[sid].u16Type = u16Type;
}

void NetBufferFamilySet(U16 sid, U16 u16Family)
{
    stPacketList[sid].u16Family = u16Family;
}

void NetBufferRelease(U16 sid)
{
    //DPrintf("%d packets to be freed", stPacketList[sid].u16ItemNum);
    
    if(stPacketList[sid].stAccessInfo.packet)
    {
        DPrintf("release un-read data");
        netBufFree(stPacketList[sid].stAccessInfo.packet);
    }
    
    memset(&stPacketList[sid].stAccessInfo, 0, sizeof(ST_NET_BUF_ACCESS_INFO));
    
    while(stPacketList[sid].u16ItemNum)
    {
        U32 packet = 0;
        packet = netBufferNodeRemove(&stPacketList[sid]);
        if(packet)
            netBufFree(packet);
    }
}

//add a node to the tail of the list
static S32 netBufferNodeAdd(ST_NET_BUF_LIST* list, U32 item)
{
    int len;
    ST_NET_PACKET* packet;
    if(list && item && (list->u16ItemNum <= NET_BUFFER_MAX_PACKET_NUM))
    {
        ST_NET_BUF_NODE* node = mpx_Malloc(sizeof(ST_NET_BUF_NODE));
        if(!node){
			DPrintf("[NETCTRL] buffer node allocate fail");
            return -1;
        }
        
        IntDisable();
        
        node->u32Data = item;
        node->next = 0; //i am the new tail
        
        if(!list->head) //first item
        {
            node->prev = 0;
            list->head = list->tail = node;
            list->u16ItemNum = 1;
        }
        else
        {
            node->prev = (U32)list->tail;
            list->tail->next = (U32)node;
            list->tail = node;
            list->u16ItemNum++;
        }
        
        packet = (ST_NET_PACKET *)item;
        if(list->u16Family == AF_INET)
        {
            if(list->u16Type == SOCK_STREAM)
                len = packet->Net.u16PayloadSize - sizeof(ST_TCP_HEADER);
            else
                len = packet->Net.u16PayloadSize - sizeof(ST_UDP_HEADER);
            list->u32DataLen += len;
        }
        
        IntEnable();
        
        return 0;
    }

	DPrintf("List is null or item is null or list full");
    return -1;
}

//remove a node from the head of the list
U32 netBufferNodeRemove(ST_NET_BUF_LIST* list)
{
    if(list)
    {
        U32 data = 0;
        
        IntDisable();
        if(list->u16ItemNum > 0)
        {
            ST_NET_BUF_NODE* node = 0;
            
            
            node = list->head;
            if(node->next)
            {
                list->head = (ST_NET_BUF_NODE*)(node->next);
                list->head->prev = 0;
            }
            else //only one item in this list
            {
                list->head = list->tail = 0;
            }
            
            data = node->u32Data;
            list->u16ItemNum--;
            IntEnable();
            //free(node);
            mpx_Free(node);

            IntDisable();
        }
        
        IntEnable();
        
        return data;
    }
    
    return 0;
}

S32 NetBufferPacketPush(U16 sid, U32 packet)
{
    S32 err = 0;
    U32 timeout = 3000;
    
    if (!packet)                                /* TCP FIN received */
    {
        stPacketList[sid].stAccessInfo.u08LastPacket = 1;
        return 0;
    }

    while(timeout)
    {
        err = netBufferNodeAdd(&stPacketList[sid], packet);
        
        if(err != 0)
        {
            timeout--;
            mpx_TaskYield(1);
        }
        else
        {
            break;
        }
    }
    
    return err;
}

U32 NetBufferPacketPop(U16 sid)
{
    U32 packet = 0;
    int len;
    
    packet = netBufferNodeRemove(&stPacketList[sid]);
    //DPrintf("pop %d, %d", sid, NetBufferPacketNumGet(sid));
    //DPrintf("   %x", packet);
    if (packet && stPacketList[sid].u16Family == AF_INET)
    {
        if(stPacketList[sid].u16Type == SOCK_STREAM)
            len = ((ST_NET_PACKET *)packet)->Net.u16PayloadSize - sizeof(ST_TCP_HEADER);
        else
            len = ((ST_NET_PACKET *)packet)->Net.u16PayloadSize - sizeof(ST_UDP_HEADER);
        stPacketList[sid].u32DataLen -= len;
    }
    return packet;
}

U16 NetBufferFromPortGet(U16 sid)
{
    return stPacketList[sid].stAccessInfo.u16FromPort;
}

U32 NetBufferFromIpGet(U16 sid)
{
    return stPacketList[sid].stAccessInfo.u32FromIp;
}

U32 NetBufferFromMacAddrGet(U16 sid)
{
    return stPacketList[sid].stAccessInfo.u08Address;
}

U32 NetBufferDataLengthGet(U16 sid)
{
    mpDebugPrint("%d: 1=%u,2=%u", __LINE__, stPacketList[sid].u32DataLen, stPacketList[sid].stAccessInfo.u16DataLength);
    return stPacketList[sid].u32DataLen + stPacketList[sid].stAccessInfo.u16DataLength;
}

/*
 * NetBufferDataRead:
 *
 * Read received TCP/UDP/Raw data from TCP/UDP/IP stack.
 *
 * Return values:
 *   > 0          = Number of bytes received.
 *   0            = No data available.  If it's TCP socket, this indicates peer 
 *                  finishes sending of all its data and a TCP FIN is received
 *                  from the peer.
 *   -1           = For TCP socket only.  This indicates no data, but no TCP FIN
 *                  has been received yet.
 */
S32 NetBufferDataRead(U16 sid, U08* buffer, U32 bufferSize, struct sockaddr *from)
{
    S32 totalLength = 0;
    ST_NET_PACKET* packet;
    U08* destPtr = buffer;
    U32 copyLength = 0;
	U08* tcp;
	U16 lenFlag = 0;
    while(1)
    {
        if(totalLength == bufferSize){
//			DPrintf("Not enough BufferSize");
            break;
        }
        
        if(stPacketList[sid].stAccessInfo.packet == 0)
        {
            U08* ip;
            
            stPacketList[sid].stAccessInfo.packet = NetBufferPacketPop(sid);
            if(!stPacketList[sid].stAccessInfo.packet)
            {
                break;
            }
            
            packet = (ST_NET_PACKET*)stPacketList[sid].stAccessInfo.packet;
            
            if (stPacketList[sid].u16Family == AF_INET)
            {
            ip = NET_PACKET_IP(packet);
            stPacketList[sid].stAccessInfo.u32FromIp = NetGetDW(&ip[IP_SRC_IP]);
            }
            
            if(stPacketList[sid].u16Type == SOCK_STREAM)
            {
                lenFlag = 0; 
                stPacketList[sid].stAccessInfo.dataPtr = (U08*)NET_PACKET_TCP_DATA(stPacketList[sid].stAccessInfo.packet);
                stPacketList[sid].stAccessInfo.u16TotalLength = packet->Net.u16PayloadSize - sizeof(ST_TCP_HEADER);
                stPacketList[sid].stAccessInfo.u16DataLength = stPacketList[sid].stAccessInfo.u16TotalLength;
                
                tcp = NET_PACKET_TCP(packet);
                lenFlag = NetGetW(&tcp[TCP_LENGTH_FLAG]);
                stPacketList[sid].stAccessInfo.u16FromPort = NetGetW(&tcp[TCP_SRC_PORT]);
                
                if(lenFlag & TCP_FIN)
                {
                    stPacketList[sid].stAccessInfo.u08LastPacket = 1;
                    //DPrintf("[NET BUFFER] last packet");
                }
            }
            else if(stPacketList[sid].u16Type == SOCK_DGRAM)
            {
                
                if (stPacketList[sid].u16Family == AF_INET)
                {
                    U08 *udp = (U08*)NET_PACKET_UDP(packet);
                    stPacketList[sid].stAccessInfo.dataPtr = (U08*)NET_PACKET_UDP_DATA(packet);
                    stPacketList[sid].stAccessInfo.u16TotalLength = NetGetW(&udp[UDP_LENGTH]) - sizeof(ST_UDP_HEADER);
                    stPacketList[sid].stAccessInfo.u16DataLength = stPacketList[sid].stAccessInfo.u16TotalLength;
                    stPacketList[sid].stAccessInfo.u16FromPort = NetGetW(&udp[UDP_SRC_PORT]);
                }
                else if (stPacketList[sid].u16Family == AF_PACKET)
                {
                    stPacketList[sid].stAccessInfo.dataPtr = (U08*)NET_PACKET_PSEUDO(packet);
                    stPacketList[sid].stAccessInfo.u16TotalLength = packet->Net.u16PayloadSize;
                    stPacketList[sid].stAccessInfo.u16DataLength = stPacketList[sid].stAccessInfo.u16TotalLength;
                    /* ----------  Get source MAC address  ---------- */
                    memset(stPacketList[sid].stAccessInfo.u08Address, 0, sizeof(stPacketList[sid].stAccessInfo.u08Address));
                    memcpy(stPacketList[sid].stAccessInfo.u08Address, NET_PACKET_ETHER(packet)+ETHERNET_MAC_LENGTH, ETHERNET_MAC_LENGTH);
                    if (from)
                    {
                        from->sa_family = AF_PACKET;
                        memset(from->sa_data, 0, sizeof(from->sa_data));
                        memcpy(from->sa_data, NET_PACKET_ETHER(packet)+ETHERNET_MAC_LENGTH, ETHERNET_MAC_LENGTH);
                    }
                }
                else if (stPacketList[sid].u16Family == AF_UNIX)
                {
                    ST_UNIX_PACKET *upkt = (ST_UNIX_PACKET*)packet;
                    stPacketList[sid].stAccessInfo.dataPtr = ((U08*)upkt + sizeof(upkt->u16DataLen) + sizeof(struct sockaddr_un));
                    stPacketList[sid].stAccessInfo.u16TotalLength = upkt->u16DataLen;
                    stPacketList[sid].stAccessInfo.u16DataLength = stPacketList[sid].stAccessInfo.u16TotalLength;
                    /* ----------  Get source address  ---------- */
                    if (from)
                    {
                        memcpy(from, upkt, sizeof(sa_family_t));
                        strncpy(((struct sockaddr_un *)from)->sun_path, (U08 *)upkt+sizeof(sa_family_t),
                                UNIX_PATH_MAX);
                        ((struct sockaddr_un *)from)->sun_path[UNIX_PATH_MAX-1] = '\0';
                    }
                }
                else
                {
                    DPrintf("[NET BUFFER] bad socket family");
                }
            }
            else if(stPacketList[sid].u16Type == SOCK_RAW)
            {
                if (stPacketList[sid].u16Family == AF_INET)
                {
                    U08 *udp = (U08*)NET_PACKET_UDP(packet);
                    stPacketList[sid].stAccessInfo.dataPtr = (U08*)NET_PACKET_UDP(packet);
                    stPacketList[sid].stAccessInfo.u16TotalLength = packet->Net.u16PayloadSize;
                    stPacketList[sid].stAccessInfo.u16DataLength = stPacketList[sid].stAccessInfo.u16TotalLength;
                    if (from)
                    {
                        struct sockaddr_in *inp = (struct sockaddr_in *)from;
                        inp->sin_family = AF_INET;
                        inp->sin_port = 0;
                        inp->sin_addr.s_addr = htonl(stPacketList[sid].stAccessInfo.u32FromIp);
                    }
                }
                else if (stPacketList[sid].u16Family == AF_PACKET)
                {
                    stPacketList[sid].stAccessInfo.dataPtr = (U08*)NET_PACKET_PSEUDO(packet);
                    stPacketList[sid].stAccessInfo.u16TotalLength = packet->Net.u16PayloadSize;
                    stPacketList[sid].stAccessInfo.u16DataLength = stPacketList[sid].stAccessInfo.u16TotalLength;
                    /* ----------  Get source MAC address  ---------- */
                    memset(stPacketList[sid].stAccessInfo.u08Address, 0, sizeof(stPacketList[sid].stAccessInfo.u08Address));
                    memcpy(stPacketList[sid].stAccessInfo.u08Address, NET_PACKET_ETHER(packet)+ETHERNET_MAC_LENGTH, ETHERNET_MAC_LENGTH);
                    if (from)
                    {
                        from->sa_family = AF_PACKET;
                        memset(from->sa_data, 0, sizeof(from->sa_data));
                        memcpy(from->sa_data, NET_PACKET_ETHER(packet)+ETHERNET_MAC_LENGTH, ETHERNET_MAC_LENGTH);
                    }
                }
                else
                {
                    DPrintf("[NET BUFFER] bad socket family");
                }
            }
        }
        
        if((totalLength + stPacketList[sid].stAccessInfo.u16DataLength) <= bufferSize)
            copyLength = stPacketList[sid].stAccessInfo.u16DataLength;
        else
        {
            copyLength = bufferSize - totalLength;
        }
        
			memcpy(destPtr, stPacketList[sid].stAccessInfo.dataPtr, copyLength);
        stPacketList[sid].stAccessInfo.dataPtr += copyLength;
        stPacketList[sid].stAccessInfo.u16DataLength -= copyLength;
        totalLength += copyLength;
        destPtr += copyLength;
        
        if(stPacketList[sid].stAccessInfo.u16DataLength == 0)
        {
            if (stPacketList[sid].u16Family == AF_UNIX)
                ext_mem_free(stPacketList[sid].stAccessInfo.packet);
            else
                netBufFree(stPacketList[sid].stAccessInfo.packet);
            stPacketList[sid].stAccessInfo.packet = 0;
            stPacketList[sid].stAccessInfo.dataPtr = 0;
            stPacketList[sid].stAccessInfo.u16TotalLength = 0;
            
            if(stPacketList[sid].u16Type == SOCK_DGRAM || stPacketList[sid].u16Type == SOCK_RAW)
            {
                break;
            }
        }
		
		//if(stPacketList[sid].u16Type == SOCK_STREAM)
		{
		  //If receive message need to make off the data read
		  //if(lenFlag & TCP_PUSH)
		  	//break;
		}
		
    }
    
    if(stPacketList[sid].u16Type == SOCK_STREAM)
    {
        if((totalLength == 0) && (stPacketList[sid].stAccessInfo.u08LastPacket == 0)){
            MP_DEBUG("tatalLength = -1");
            totalLength = -1;
        }
    }
    
    return totalLength;
}

U16 NetBufferPacketNumGet(U16 sid)
{
    if(stPacketList[sid].u16ItemNum)
        return stPacketList[sid].u16ItemNum;
    else if(stPacketList[sid].stAccessInfo.u16DataLength)
        return 1;
    else if(stPacketList[sid].stAccessInfo.u08LastPacket == 1)
        return 1;
    else
        return 0;
}

extern void *allocs[100];
S32 NetBufferPacketReceive(ST_NET_PACKET* packet)
{
    U08* dstMac = (U08*)NET_PACKET_ETHER(packet);
    BOOL broad, forme, isMcast = FALSE;
        U16 type = NetGetW(((U08*)NET_PACKET_ETHER(packet) + 12));
    U08* srcMac = dstMac + 6;
    
#if 0
    DPrintf("%s", __FUNCTION__);
	DPrintf("NetBufferPacketReceive dst => %02x:%02x:%02x:%02x:%02x:%02x",
		dstMac[0], dstMac[1], dstMac[2],
        dstMac[3], dstMac[4], dstMac[5]);
	DPrintf("NetBufferPacketReceive src => %02x:%02x:%02x:%02x:%02x:%02x",
		srcMac[0], srcMac[1], srcMac[2],
        srcMac[3], srcMac[4], srcMac[5]);
	DPrintf("NetBufferPacketReceive type => 0x%x", type);
#endif
    broad = NetMacAddrComp(dstMac, NetBroadcastMacGet());
    forme = NetMacAddrComp(dstMac, NetLocalMACGet());

    if (!broad)
    {
        isMcast = broad = dstMac[0] & 0x01;               /* multicast */
    }
    
    if(broad || forme) //a broadcast/multicast packet or my packet
    {
        U16 packetType = NetGetW(((U08*)NET_PACKET_ETHER(packet) + 12));
        
        switch(packetType)
        {
            case ETHERNET_TYPE_IP:
                IpPacketReceive(packet, isMcast, &NicArray[NIC_INDEX_WIFI]);
            break;
            
            case ETHERNET_TYPE_ARP:
                ArpPacketReceive(packet);
            break;
            
            default:
                if(!ProtocolPacketReceive(packetType, packet))
                {
#if 0
                    DPrintf("NetBufferPacketReceive 1 dst => %02x:%02x:%02x:%02x:%02x:%02x",
                            dstMac[0], dstMac[1], dstMac[2],
                            dstMac[3], dstMac[4], dstMac[5]);
                    DPrintf("NetBufferPacketReceive src => %02x:%02x:%02x:%02x:%02x:%02x",
                            srcMac[0], srcMac[1], srcMac[2],
                            srcMac[3], srcMac[4], srcMac[5]);
#endif
                    netBufFree(packet);
                    l2_stat.rx_dropped++;
                }
            break;
        }
    }
    else
    {
        netBufFree(packet);
        l2_stat.rx_dropped++;
    }
    return NO_ERR;
}

#define ETH_P_PAE 0x888E	/* Port Access Entity (IEEE 802.1X) */
/*
 * A fast forwarding path to forward EAPoL frames to wpa_supplicant.
 */
void *NetBufferPacketReceive2(ST_NET_PACKET* packet)
{
    U08* dstMac = (U08*)NET_PACKET_ETHER(packet);
    BOOL forme;
    
    forme = NetMacAddrComp(dstMac, NetLocalMACGet());

    if (forme)
    {
        U16 packetType = NetGetW(((U08*)NET_PACKET_ETHER(packet) + 12));
        
        /*
         * Use fast path for EAPoL packets: send directly to wpa_supplicant.
         */
        if (packetType == ETH_P_PAE)
        {
            if(ProtocolPacketReceive(packetType, packet))
            {
#if (DEMO_PID&&(Make_DM9621_ETHERNET==0))
    TaskSleep(1);
#else
                TaskYield();
#endif
                return NULL;
            }
        }
    }

    return packet;
}

void NetCtrlTask()
{
    S32 status;
    U32 u32Message[8];
    
#ifndef MP600
    status = mpx_MessageCreate(OS_ATTR_FIFO, 640);
    if(status < 0){
        DPrintf("NetCtrlTask: message create fail");
        BREAK_POINT();
    }
    else
        u08NetCtrlMessageId = (U08)status;
#endif
    
    while(1)
    {
        status = mpx_MessageReceive(u08NetCtrlMessageId, (U08*)u32Message);
//        DPrintf("NetCtrlTask: s=%d,m=%d",status,u32Message[0]);
        if(status > 0)
        {
            switch(u32Message[0])
            {
                case NETCTRL_MSG_NET_PACKET_SEND:
                    NetPacketSend(u32Message[1], u32Message[2], u32Message[3] & 0xffff);
#ifndef DRIVER_FREE_TXBUFFER
                    NetFreePacket(u32Message[1]);
#endif
                break;
                        
                case NETCTRL_MSG_IP_PACKET_SEND:
                    IpPacketSend(u32Message[1], u32Message[2] & 0xff, u32Message[3], u32Message[4]);
#ifndef DRIVER_FREE_TXBUFFER
                    NetFreePacket(u32Message[1]);
#endif
                break;
                
                case NETCTRL_MSG_UDP_PACKET_SEND:
                    UdpPacketSend_if(u32Message[1], u32Message[2], u32Message[3] & 0xffff, u32Message[4], u32Message[5] & 0xffff, &NicArray[NIC_INDEX_WIFI]);
                break;
                
                case NETCTRL_MSG_TCP_FAST_TIMER:
                    (*((TCP_FAST_TIMER_PROC)u32Message[1]))();
                break;
                
                case NETCTRL_MSG_TCP_SLOW_TIMER:
                    (*((TCP_SLOW_TIMER_PROC)u32Message[1]))();
                break;
                
                case NETCTRL_MSG_TCP_RECEIVED:
                    (*((TCP_RECEIVED_CALLBACK)u32Message[1]))(u32Message[2], (U16)(u32Message[3]));
                break;
                
                case NETCTRL_MSG_GENERAL_CALLBACK:
                    (*((NET_CALLBACK_PROC)u32Message[1]))();
                break;
            }
        }
    }
}



S32 NetCtrlInit()
{
    S32 status;
    
    memset(netTimerBank, 0, sizeof(netTimerBank));
    
#ifdef MP600
    status = mpx_MessageCreate(OS_ATTR_FIFO, 640);
    if(status < 0){
        DPrintf("NetCtrlInit: mailbox create fail");
        BREAK_POINT();
    }
    else
        u08NetCtrlMessageId = (U08)status;
#endif

    NetBufferInit();
    
#ifdef MP600
    status = mpx_TaskCreate(NetCtrlTask, NETWORK_TASK_PRI, TASK_STACK_SIZE*2);
#else
    status = mpx_TaskCreate(NetCtrlTask, NETWORK_TASK_PRI-2, TASK_STACK_SIZE);
#endif
    if(status < 0)
    {
        DPrintf("NetBufferTask create fail");
        BREAK_POINT();
    }
    u08NetCtrlTaskId = (U08)status;
    
    DPrintf("[NETCTRL] NetCtrlTask id = %d", u08NetCtrlTaskId);
    mpx_TaskStartup(u08NetCtrlTaskId);
    
    status = mpx_TimerCreate(NetTimerProc, OS_TIMER_ROUGH, 1, 4); // network timer period = 250 ms
    if(status < 0)
    {
        DPrintf("[NETCTRL] timer create fail");
        BREAK_POINT();
    }
    
    u08NetTimerId = (U08)status;
    
    /*
    status = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_EVENT_CLEAR, 0);
    if(status < 0)
    {
        DPrintf("[NETCTRL] event create fail");
        BREAK_POINT();
    }
    
    u08NetCtrlEventId = (U08)status;
    */
    
    return NO_ERR;
}

S32 NetCtrlStart()
{
    return mpx_TimerStart(u08NetTimerId, 0xffffffff);
}

void NetCtrlStop()
{
    mpx_TimerStop(u08NetTimerId);
}

U08 NetCtrlTaskIdGet()
{
    return u08NetCtrlTaskId;
}

U08 NetCtrlMessageIdGet()
{
    return u08NetCtrlMessageId;
}

#if 1
/* 
 * for netlink sockets
 */
S32 NetBufferPacketPush2(ST_NET_BUF_LIST *list, U32 packet)
{
    S32 err = 0;
//    U32 timeout = 3000;
    
    if (!packet)
    {
        list->stAccessInfo.u08LastPacket = 1;
        return 0;
    }

//    while(timeout)
    {
        err = netBufferNodeAdd(list, packet);
        
        if(err != 0)
        {
//            timeout--;
            mpx_TaskYield(1);
        }
        else
        {
//            DPrintf("[SOCKET] NetBufferPacketPush2 list=%p", list);
//            break;
        }
    }
    
    return err;
}

U32 NetBufferPacketPop2(ST_NET_BUF_LIST *list)
{
    U32 packet = 0;
    int len;
    
    packet = netBufferNodeRemove(list);

    if (packet && list->u16Family == AF_INET)
    {
        if(list->u16Type == SOCK_STREAM)
            len = ((ST_NET_PACKET *)packet)->Net.u16PayloadSize - sizeof(ST_TCP_HEADER);
        else
            len = ((ST_NET_PACKET *)packet)->Net.u16PayloadSize - sizeof(ST_UDP_HEADER);
        list->u32DataLen -= len;
    }
    return packet;
}

#endif

int m_kernel_generic_semaphore()
{
    return NET_SEM_ID;
}

