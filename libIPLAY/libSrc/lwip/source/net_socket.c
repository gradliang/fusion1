
///
///@defgroup    SOCKET  Socket Interface
///

#define LOCAL_DEBUG_ENABLE 0
#include "global612.h"
#include "mpTrace.h"
#include "lwip_config.h"
#include "linux/types.h"
#include <string.h>
#include <sys/time.h>
#include "ndebug.h"
#include "typedef.h"
#include "os.h"
#include "net_packet.h"
#include "socket.h"
#include "net_socket.h"
#include "error.h"
#include "util_queue.h"
#include "taskid.h"
#include <linux/if.h>
#include "net_nic.h"

#ifdef NET_NAPI
#include "xpgNet.h"
#endif

/*  
 * bitmap flags for socket event
 */
#define SOCK_SIGNAL_TX_DONE     1
#define SOCK_SIGNAL_RX_DONE     2
#define SOCK_SIGNAL_TX_READY    SOCK_SIGNAL_TX_DONE
#define SOCK_SIGNAL_SOCKET_STATE_CHANGED    4

static ST_SOCKET    SockBank[SOCK_MAXIMAL_NUMBER];
static U08          u08SockEventId;

static ST_QUEUE* sockConnQueue[SOCK_MAXIMAL_NUMBER];
static S32 sockAccept(U16 u16ListenSid, ST_TCP_PCB* pcb);

extern void SockIdSignalRxDone3(U16 sid, void* packet);

void SockInit()
{
    register U32 index;
    
    // initialize the socket event with 32-bit corresponding to 32 sockets
    u08SockEventId = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE|OS_ATTR_EVENT_CLEAR, 0);
    //u08SockEventId = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE, 0);
    
    // let all the socket to be unallocated.
    for(index = 0; index < SOCK_MAXIMAL_NUMBER; index ++)
    {
        SockBank[index].u08Type = SOCK_TYPE_FREE;
        SockBank[index].pcb.tcp = 0;
        
        sockConnQueue[index] = 0;
    }
}



S32 SockAllocate()
{
	ST_SOCKET *point;
	U16 id;
	S32 status;
    
    status = ERR_NO_FD;                 // preset status to no file descriptor available
    id = 1;                             // reserve id = 0 for the convenience of AP impletement
    point = &SockBank[1];
    while(id < SOCK_MAXIMAL_NUMBER)
    {
        if(point->u08Type == SOCK_TYPE_FREE)
        {
            memset(point, 0, sizeof(*point));
            point->u08Type = SOCK_TYPE_ALLOCATED;
            status = id;                // get the socket id and ready to return this id
            break;
        }
        
        id ++;
        point ++;
    }
    
    return status;
}



void SockFree(U16 id)
{
    IntDisable();
    NetBufferRelease(id);
    SockBank[id].u08Type = SOCK_TYPE_FREE;
    IntEnable();
}



void SockSignalTxDone(U16 id)
{
    register U32 mask;
    
    mask = 0x00000001 << id;
    SockBank[id].u32Signal |= SOCK_SIGNAL_TX_DONE;
    mpx_EventSet(u08SockEventId, mask);
}



void SockTcpFreed(U16 sid)
{
    SockBank[sid].pcb.tcp = 0;
}



void SockIdSignalRxDone(U16 sid, void* packet)
{
#ifdef HAVE_LWIP_SOCKET
    if (sid > 0x100)
    {
        SockIdSignalRxDone3(sid, packet);
        return;
    }
#endif
    
    if((sid >= SOCK_MAXIMAL_NUMBER) || (sid == 0))
    {
        if (sid)
            DPrintf("[SOCKET] socket id %d is invalid", sid);
        if (packet)
            netBufFree(packet);
        return;
    }
    
    if(SockBank[sid].u08Type != SOCK_TYPE_ALLOCATED)
    {
        DPrintf("[SOCKET] socket %d not allocated", sid);
        if (packet)
            netBufFree(packet);
        return;
    }
    
	if(!packet){
		DPrintf("[SOCKET] socket is closed/reset");
		NetBufferPacketPush(sid, NULL);
	}
    else 
    {
#ifdef NEW_NETSOCKET
        if (sockIsCantRecvMore(sid))
        {
            netBufFree(packet);
            return;
        }
        else
#endif
        if(NO_ERR != NetBufferPacketPush(sid, (U32)packet))
        {
            DPrintf("[SOCKET] tcp packet buffer full");
            netBufFree(packet);
            return;
        }
    }

    U32 mask = 0;

    mask = 0x00000001 << sid;
    SockBank[sid].u32Signal |= SOCK_SIGNAL_RX_DONE;
    mpx_EventSet(u08SockEventId, mask);
}

/*
 * For Unix socket only
 */
void SockIdSignalRxDone2(U16 sid, void* packet)
{
    
    if((sid >= SOCK_MAXIMAL_NUMBER) || (sid == 0))
    {
        if (sid)
            DPrintf("[SOCKET] socket id %d is invalid", sid);
        if (packet)
            ext_mem_free(packet);
        return;
    }
    
    if(SockBank[sid].u08Type != SOCK_TYPE_ALLOCATED)
    {
        DPrintf("[SOCKET] socket %d not allocated", sid);
        if (packet)
            ext_mem_free(packet);
        return;
    }
    
	if(!packet){
		DPrintf("[SOCKET] socket is closed/reset");
		NetBufferPacketPush(sid, NULL);
	}
    else 
    {
#ifdef NEW_NETSOCKET
        if (sockIsCantRecvMore(sid))
        {
            ext_mem_free(packet);
            return;
        }
        else
#endif
        if(NO_ERR != NetBufferPacketPush(sid, (U32)packet))
        {
            DPrintf("[SOCKET] tcp packet buffer full");
            ext_mem_free(packet);
            return;
        }
    }

    U32 mask = 0;

    mask = 0x00000001 << sid;
    SockBank[sid].u32Signal |= SOCK_SIGNAL_RX_DONE;
    mpx_EventSet(u08SockEventId, mask);
}


void SockSignalRxDone(U32 peerIp, U16 peerPort, U32 localIp, U16 localPort, void *packet)
{
    register U32 index;
    register U32 mask;
    
    // find the target soc
    index = 1;                               // actually, the socket number begins at one
    while(index < SOCK_MAXIMAL_NUMBER)
    {
        if(SockBank[index].u08Type == SOCK_TYPE_ALLOCATED)
        {
            if((((ST_SOCK_ADDR *)&SockBank[index].Local)->u16Port == localPort) &&
                ((((ST_SOCK_ADDR_IN *)&SockBank[index].Peer.u08Address)->u32Addr == peerIp) ||
                (((ST_SOCK_ADDR_IN *)&SockBank[index].Peer.u08Address)->u32Addr == 0) ||
                (localIp == NetBroadcaseIpGet()) ||
                (((ST_SOCK_ADDR_IN *)&SockBank[index].Local.u08Address)->u32Addr == localIp)))
                    break;
        }
        index ++;
    }

    if(index < SOCK_MAXIMAL_NUMBER)
    {
        mask = 0x00000001 << index;
        
        if(SockBank[index].u16SockType == SOCK_DGRAM)
        {
            if(0 != NetBufferPacketPush(index, (U32)packet))
            {
                DPrintf("packet buffer full, drop it");
                netBufFree(packet);
            }
            else
            {
                SockBank[index].u32Signal |= SOCK_SIGNAL_RX_DONE;
                mpx_EventSet(u08SockEventId, mask);
            }
        }
        else if(SockBank[index].u16SockType == SOCK_STREAM)
        {
            DPrintf("[SOCKET] unimplemented situation");
#if 0
            SockBank[index].inPoint = packet;
            
            if(SockBank[index].inPoint)
            {
                SockBank[index].u32Signal |= SOCK_SIGNAL_RX_DONE;
                mpx_EventSet(u08SockEventId, mask);
            }
#else
            netBufFree(packet);
#endif
        }
        else
        {
            NET_DEBUG_STR("SockSignalRxDone: error socket type s=%d", index);
            netBufFree(packet);
        }
    }
    else
    {
        netBufFree(packet);   // abandon this packet because no socket is for this packet
    }
}

S32 ProtocolPacketReceive(U16 ethertype, void* packet){
    register U32 index;
    register U32 mask;

    //DPrintf("%s", __FUNCTION__);
    // find the target soc
    index = 1;                         // actually, the socket number begins at one
    while(index < SOCK_MAXIMAL_NUMBER)
    {
        if(SockBank[index].u08Type == SOCK_TYPE_ALLOCATED && 
           SockBank[index].u16Family != AF_INET &&
           SockBank[index].u16Protocol == ethertype)
        {
            //yuming, can be extend 
            break;
        }
        index ++;
    }
    
    if(index < SOCK_MAXIMAL_NUMBER)
    {
        mask = 0x00000001 << index;
        
        if(SockBank[index].u16SockType == SOCK_DGRAM || SockBank[index].u16SockType == SOCK_RAW)
        {
            if(0 != NetBufferPacketPush(index, (U32)packet))
            {
                DPrintf("packet buffer full, drop it");
                netBufFree(packet);
            }
            else
            {
                SockBank[index].u32Signal |= SOCK_SIGNAL_RX_DONE;
                mpx_EventSet(u08SockEventId, mask);
            }
        }
        else if(SockBank[index].u16SockType == SOCK_STREAM)
        {
            DPrintf("[SOCKET] unimplemented situation");
#if 0
            SockBank[index].inPoint = packet;
            
            if(SockBank[index].inPoint)
            {
                SockBank[index].u32Signal |= SOCK_SIGNAL_RX_DONE;
                mpx_EventSet(u08SockEventId, mask);
            }
#else
            return 0;
#endif
        }
        else
        {
            NET_DEBUG_STR("error socket type: s=%d,type=0x%x", index,ethertype);
            return 0;
        }
    }
    else
    {
        return 0;
    }

    return 1;

}

void SockLoadPacket(U16 sid)
{
    ST_SOCKET* socket = &SockBank[sid];
    U32 mask = 0x00000001 << sid;
    
    socket->inPoint = NetBufferPacketPop(sid);
    if(socket->inPoint)
        socket->u32Signal |= SOCK_SIGNAL_RX_DONE;
    
    if(0 < NetBufferPacketNumGet(sid))
    {
        mpx_EventSet(u08SockEventId, mask);
    }
    else
        mpx_EventClear(u08SockEventId, ~mask);
}



ST_SOCKET* SockGet(U16 u16SockId)
{
    if(u16SockId >= SOCK_MAXIMAL_NUMBER)
        return 0;
    else
        return (&SockBank[u16SockId]);
}



void mpx_SockAddrSet(ST_SOCK_ADDR *addr, U32 ipAddr, U16 port)
{
    ST_SOCK_ADDR_IN *inetAddr;
    
    // support IPV4 currently
    addr->u16Family = AF_INET;
    addr->u16Port = port;
    
    inetAddr = (ST_SOCK_ADDR_IN *)(addr->u08Address);
    inetAddr->u32Addr = ipAddr;
}



void mpx_SockAddrGet(ST_SOCK_ADDR *addr, U32* ipAddr, U16* port)
{
    ST_SOCK_ADDR_IN *inetAddr;
    
    if(port)
        *port = addr->u16Port;
    
    if(ipAddr)
    {
        inetAddr = (ST_SOCK_ADDR_IN *)(addr->u08Address);
        *ipAddr = inetAddr->u32Addr;
    }
}



U32 mpx_SockLocalAddrGet(U16 sid)
{
    ST_SOCK_ADDR_IN *inetAddr = (ST_SOCK_ADDR_IN *)(SockBank[sid].Local.u08Address);
    return inetAddr->u32Addr;
}


U32 mpx_SockPeerAddrGet(U16 sid)
{
    ST_SOCK_ADDR_IN *inetAddr = (ST_SOCK_ADDR_IN *)(SockBank[sid].Peer.u08Address);
    return inetAddr->u32Addr;
}



///
///@ingroup SOCKET
///@brief   To be implemented.
S32 mpx_GetAddrInfo(S08 *nodeName, S08 *portName, ST_ADDR_INFO *hints, ST_ADDR_INFO *result)
{
   return 0;
}



S32 mpx_GetNameInfo()
{
   return 0;
}



S32 mpx_Socket(U16 domain, U16 type)
{
    register S32 status;
    register ST_SOCKET *socket;

    if(domain != AF_INET && domain != AF_PACKET)
        status = ERR_WRONG_AF;
    else if(type != SOCK_STREAM && type != SOCK_DGRAM)
        status = ERR_WRONG_TYPE;
    else
    {
        IntDisable();
        // critical section, protect it for thread-safe
        status = SockAllocate();
        IntEnable();
        
        if(status > 0)
        {
            socket = &SockBank[(U16)status];
            socket->u08NetIndex = NetDefaultNicGet();
            socket->u16Family = domain;
            socket->u16SockType = type;
            socket->u16Protocol = 0;            // fix at 0 anyway
            
            socket->pcb.tcp = 0;
            if(type == SOCK_STREAM)
            {
                socket->pcb.tcp = TcpNew();
                if(!socket->pcb.tcp)
                    status = ERR_OUT_OF_MEMORY;
                else
                    socket->pcb.tcp->u16SockId = (U16)status;
            }
            else if(type == SOCK_DGRAM)
            {
                socket->pcb.udp = UdpNew();
                if(!socket->pcb.udp)
                    status = ERR_OUT_OF_MEMORY;
            }
            
            memset(&socket->Local, 0, sizeof(ST_SOCK_ADDR));
            memset(&socket->Peer, 0, sizeof(ST_SOCK_ADDR));
            
            NetBufferTypeSet((U16)status, type);
            NetBufferFamilySet((U16)status, domain);
        }
    }
    
    return status;
}



S32 mpx_SocketClose(U16 sockId)
{
    register ST_SOCKET *socket = &SockBank[sockId];
    U32 mask = 0x00000001 << sockId;
    
    if(socket->pcb.tcp)
    {
        if(socket->u16SockType == SOCK_DGRAM)
        {
            UdpFree(socket->pcb.udp);
            socket->pcb.udp = 0;
        }
        else if(socket->u16SockType == SOCK_STREAM)
        {
            if(NO_ERR != TcpClose(socket->pcb.tcp))
            {
                DPrintf("[SOCKET] tcp abort");
                TcpAbort(socket->pcb.tcp);
            }
            
            socket->pcb.tcp = 0;
        }
    }
    
    mpx_EventClear(u08SockEventId, ~mask);
    
#ifdef NEW_NETSOCKET
    socket->u16SockState = SS_NOFDREF;
#endif
    SockFree(sockId);
    return NO_ERR;
}



S32 mpx_Bind(U16 sid, ST_SOCK_ADDR *addr)
{
    register ST_SOCKET *socket = &SockBank[sid];
    int ret = 0;
    
    if(socket->u08Type == SOCK_TYPE_FREE)
        return -1;
    memcpy(&socket->Local, addr, sizeof(ST_SOCK_ADDR));

    //yuming, fix mpx_socket no protocol 
    if(addr->u16Family == PF_PACKET){
        socket->u16Protocol = addr->u16Port;
    }

    switch(socket->u16SockType)
    {
        case SOCK_STREAM:
            ret = TcpBind(socket->pcb.tcp, ((ST_SOCK_ADDR_IN *)(addr->u08Address))->u32Addr, addr->u16Port);
            MP_DEBUG2("[mpx_Bind] TcpBind(%d) returns = %d", sid, ret);
        break;
        
        case SOCK_DGRAM:
            ret = UdpBind(socket->pcb.udp, ((ST_SOCK_ADDR_IN *)(addr->u08Address))->u32Addr, addr->u16Port);
        break;
    }
    
    return ret;
}



S32 mpx_Listen(U16 sockId, U16 backlog)
{
    S32 err = NO_ERR;
    register ST_SOCKET *socket = &SockBank[sockId];
    
    if(socket->u08Type == SOCK_TYPE_FREE)
        return -1;
        
    if(socket->u16SockType != SOCK_STREAM)
        return ERR_WRONG_TYPE;
    
    if(sockConnQueue[sockId] == 0)
    {
        sockConnQueue[sockId] = mpx_QueueCreate();
        if(sockConnQueue[sockId] == 0)
        {
            DPrintf("[SOCKET] queue create fail");
            BREAK_POINT();
        }
    }
    
    TcpAcceptCallbackSet(socket->pcb.tcp, sockAccept);
    err = TcpListen(socket->pcb.tcp);
    
    return err;
}



S32 mpx_Accept(U16 sid, ST_SOCK_ADDR* addr)
{
    U32 item;
    ST_TCP_PCB* pcb;
    S32 status = NO_ERR;
    register ST_SOCKET *listenSocket = &SockBank[sid];
    register ST_SOCKET *newSocket;
    
    if(!sid || (sid > SOCK_MAXIMAL_NUMBER))
        return ERR_INVALID_SOCKET;
        
    if(SockBank[sid].u16SockType != SOCK_STREAM)
        return ERR_WRONG_TYPE;
        
    if(0 == mpx_QueueLengthGet(sockConnQueue[sid]))
    {
        DPrintf("[SOCKET] queue length get fail");
        return 0;
    }
        
    if(0 != mpx_QueuePop(sockConnQueue[sid], &item))
    {
        DPrintf("[SOCKET] queue pop fail");
        return 0;
    }
    
    pcb = (ST_TCP_PCB*)item;
    
    IntDisable();
    // critical section, protect it for thread-safe
    status = SockAllocate();
    IntEnable();
    
    if(status > 0)
    {
        newSocket = &SockBank[(U16)status];
        
        newSocket->u08NetIndex = listenSocket->u08NetIndex;
        newSocket->u16Family = listenSocket->u16Family;
        newSocket->u16SockType = listenSocket->u16SockType;
        newSocket->u16Protocol = 0;            // fix at 0 anyway
        
        newSocket->pcb.tcp = pcb;
        pcb->u16SockId = (U16)status;
        
        DPrintf("[SOCKET] accept connection");
        DPrintf("   from \\-");
        NetDebugPrintIP(pcb->u32PeerIp);
        DPrintf("   port %d", pcb->u16PeerPort);
        DPrintf("   at local port %d", pcb->u16LocalPort);
        
        mpx_SockAddrSet(&newSocket->Local, pcb->u32LocalIp, pcb->u16LocalPort);
        mpx_SockAddrSet(&newSocket->Peer, pcb->u32PeerIp, pcb->u16PeerPort);
        mpx_SockAddrSet(addr, pcb->u32PeerIp, pcb->u16PeerPort);
        
        NetBufferTypeSet((U16)status, SOCK_STREAM);
        
        TcpAccept(pcb, (U16)status);
    }
    
    return status;
}



S32 mpx_Connect(U16 sockId, ST_SOCK_ADDR *destAddr)
{
    register ST_SOCKET *socket = &SockBank[sockId];
    S32 status = NO_ERR;
    
    if(!sockId || (sockId > SOCK_MAXIMAL_NUMBER))
        return ERR_INVALID_SOCKET;
    
    if(!socket->Local.u16Family)
        mpx_SockAddrSet(&socket->Local, NetDefaultIpGet(), mpx_NewLocalPort());
	
    memcpy(&socket->Peer, destAddr, sizeof(ST_SOCK_ADDR));
    
    if(SockBank[sockId].u16SockType == SOCK_DGRAM)
    {   // UDP connection
        status = UdpConnect(socket->pcb.udp, ((ST_SOCK_ADDR_IN *)(destAddr->u08Address))->u32Addr, destAddr->u16Port);
    }
    else
    {
        status = TcpConnect(socket->pcb.tcp, ((ST_SOCK_ADDR_IN *)(destAddr->u08Address))->u32Addr, destAddr->u16Port);
    }
    
    return status;
}



///
///@ingroup SOCKET
///@brief Wait until the specific socket set has any activities
///
///@param read  The socket set that is wait for the reading activity
///@param write The socket set that is wait for the writing activity
///@param excep the socket set that is wait for the exception status
///@param tick  wait time in millisecond
///
///@retval  >0  The number of the sockets that have activities in socket set of read, write and excep.
///@retval  0   This function return because the time-out happen. No socket has activity when this return value.
///
S32 mpx_Select(ST_SOCK_SET *read, ST_SOCK_SET *write, ST_SOCK_SET *excep, U32 tick)
{
#if 0
    
#else
    register S32 status;
    register U32 mask;
    U32 result;
    U16 i = 0;
    
    //socketEventStatusUpdate();
    
    mask = 0;
    if(read)   mask |= read->u32Mask;
    if(write)  mask |= write->u32Mask;
    if(excep)  mask |= excep->u32Mask;
    
    status = mpx_EventWaitWto(u08SockEventId, mask, OS_EVENT_OR, &result, tick);
    if(status == OS_STATUS_OK)
    {
        status = 0;         // to be the socket counter
        mask = 0x00000001;
        while(mask)
        {
            //if(result | mask)       // test if the corresponding is triggerred
            if(result & mask)
                status ++;
            else
            {
                if(read)   read->u32Mask &= ~mask;
                if(write)  write->u32Mask &= ~mask;
                if(excep)  excep->u32Mask &= ~mask;
            }
            
            mask <<= 1;
        }
    }
    else
        status = 0;
    
    return status;
#endif
}



S32 mpx_SockWrite(U16 sid, U08 *buffer, S32 size)
{
    return mpx_Send(sid, buffer, size);
}



S32 mpx_Send(U16 sid, U08 *buffer, S32 size)
{
    ST_SOCKET *socket;
    S32 status;
    
    socket = &SockBank[sid];
    
    if(socket->u16SockType == SOCK_DGRAM)
    {
        if(!socket->pcb.udp->u32LocalIp)
            socket->pcb.udp->u32LocalIp = NetDefaultIpGet();
        if(!socket->pcb.udp->u16LocalPort)
            socket->pcb.udp->u16LocalPort = mpx_NewLocalPort();
        if((!socket->pcb.udp->u32PeerIp) || (!socket->pcb.udp->u16PeerPort))
        {
            DPrintf("[SOCKET] not peer info is bound in a UDP socket");
            return -1;
        }
        
        return UdpWrite(socket->pcb.udp, buffer, size);
    }
    else if(socket->u16SockType == SOCK_STREAM)
        return TcpWrite(socket->pcb.tcp, buffer, size);
    
    return ERR_WRONG_TYPE;
}



///
///@ingroup SOCKTE
///@brief   The mpx_SendTo() function shall send a message through a connection-mode or connectionless-mode socket.
///         If the socket is connectionless-mode, the message shall be sent to the address specified by 'dest'. 
///         If the socket is connection-mode, 'dest' shall be ignored.
///
///@param   sid         Specify the socket ID.
///@param   *message    Points to a buffer containing the message to be sent.
///@param   length      Specifies the size of the message in bytes.
///@param   flag        Should be set to zero at this implementation
///@param   *dest       Points to a ST_SOCK_ADDR structure containing the destination address. The length and format 
///                     of the address depend on the address family of the socket. 
///
///@retval  >0          The transferred size in byte, if the function succeed.
///@retval  
///
///@remark
///
S32 mpx_SendTo(U16 sid, void *message, S32 length, S32 flag, ST_SOCK_ADDR *dest)
{
    ST_SOCKET *socket;
    ST_NET_PACKET *packet;
    S32 status;
    U08* ptr;
    
    socket = &SockBank[sid];
    
    if(socket->u16SockType != SOCK_DGRAM)   return ERR_WRONG_TYPE;
    
    if(!socket->Local.u16Family)    // if socket is not binded, set it's local address here
        mpx_SockAddrSet(&socket->Local, NetDefaultIpGet(), mpx_NewLocalPort());
	
    memcpy(&socket->Peer, dest, sizeof(ST_SOCK_ADDR));  // set the target socket address here   
    
    if(socket->u16Family == PF_PACKET)
    {
        packet = NetNewPacket(FALSE);
        if(!packet)
        {
            DPrintf("no memory for SendTo");
            return ERR_NO_MEMORY;
        }
        
        socket->outPoint = (void*)packet;
			mmcp_memcpy(NET_PACKET_IP(packet), message, length);          // and then the data portion
        packet->Net.u16PayloadSize = (U16)length;
        packet->Net.u16SockId = sid;
        packet->Net.u08NetIndex = NetDefaultNicGet();
        status = NetPacketSend(packet, 0, dest->u16Port);
#ifndef DRIVER_FREE_TXBUFFER
        NetFreePacket(packet);
#endif
    }
    else
    {
        U16 payloadSize = 0;
        S32 sendSize = 0;
        U08* dataPtr = message;
        
        while(length)
        {
            payloadSize = length > MAX_UDP_DATA ? MAX_UDP_DATA : length;
            
            packet = NetNewPacket(FALSE);
            if(!packet)
            {
                DPrintf("no memory for SendTo");
                return ERR_NO_MEMORY;
            }
            
            socket->outPoint = (void*)packet;
			
				mmcp_memcpy(NET_PACKET_UDP_DATA(packet), dataPtr, payloadSize);          // and then the data portion
            
            packet->Net.u16PayloadSize = payloadSize;
            packet->Net.u16SockId = sid;
            
            status = UdpPacketSend(packet, 
                                ((ST_SOCK_ADDR_IN*)(socket->Local.u08Address))->u32Addr,
                                socket->Local.u16Port,
                                ((ST_SOCK_ADDR_IN*)(socket->Peer.u08Address))->u32Addr,
                                socket->Peer.u16Port);
            
            if(status < 0)
            {
                errno = -status;
                return -1;
            }
            
            sendSize += payloadSize;
            length -= payloadSize;
            dataPtr += payloadSize;
        }
        
        status = sendSize;
    }
    
    return status;
}



void mpx_SendMsg()
{
    DPrintf(" To be implemented.");
}



S32 mpx_SockRead(U16 fd, U08 *buffer, U32 size)
{
    return mpx_RecvFrom(fd, buffer, size, 0, 0);
}



S32 mpx_Recv(U16 sid, U08* buffer, U32 size)
{
    return mpx_RecvFrom(sid, buffer, size, 0, 0);
}



S32 mpx_RecvFrom(U16 sid, void *message, U32 length, S32 flag, ST_SOCK_ADDR *from)
{
    ST_SOCKET *socket = SockGet(sid);
    U08* pData = 0;
    S32 dataLen = 0;
    
    if(!socket)
    {
        DPrintf("[SOCKET] mpx_RecvFrom: invalid socket");
        return ERR_INVALID_SOCKET;
    }
    
    if(socket->u16SockType == SOCK_DGRAM)
    {
        if(socket->u16Protocol != 0)
        {
            SockLoadPacket(sid);
            if(socket->inPoint)
            {
                U08 *udp = NET_PACKET_UDP(socket->inPoint);
                U08* ip = NET_PACKET_IP(socket->inPoint);
                U32 fromIp = NetGetDW(&ip[IP_SRC_IP]);
                U16 fromPort = NetGetW(&udp[UDP_SRC_PORT]);
                U08 *ether = NET_PACKET_ETHER(socket->inPoint);

		  MP_DEBUG2("fromIp = 0x%x, fromPort = 0x%x", fromIp, fromPort);
                mpx_SockAddrSet(&(socket->Peer), fromIp, fromPort);
                if(from)
                {
                
                    memcpy(from, &(socket->Peer), sizeof(ST_SOCK_ADDR)); 
                    NetMacAddrCopy(from->u08Address, &ether[6]);
                }
                //yuming, need to add switch if much protocol join
                pData = NET_PACKET_IP(socket->inPoint);
                dataLen = NetGetW(&pData[2]) + 4;// 802.1x 
            }
            else
            {
                DPrintf("[SOCKET] no data available");
                return 0;
            }
            
            if(length < dataLen)
            {
                DPrintf("mpx_RecvFrom: buffer not enough");
                NetFreePacket(socket->inPoint);
                socket->u32Signal = 0;
                return ERR_BUFFER_NOT_ENOUGH;
            }
            
            if(socket->u32Signal == SOCK_SIGNAL_RX_DONE)
            {
            
					memcpy(message, pData, dataLen);
                if(socket->inPoint)
                {
                    NetFreePacket(socket->inPoint);
                    socket->inPoint = 0;
                }
                socket->u32Signal = 0;
            }
            else
            {
                DPrintf("mpx_RecvFrom: rx not ready");
                return 0;
            }
        }
        else
        {
            dataLen = NetBufferDataRead(sid, (U08*)message, length, NULL);
            DPrintf("mpx_RecvFrom: dataLen=0x%x", dataLen);
            mpx_SockAddrSet(&(socket->Peer), NetBufferFromIpGet(sid), NetBufferFromPortGet(sid));
            if(from)
            {
            
                memcpy(from, &(socket->Peer), sizeof(ST_SOCK_ADDR));
            }
        }
    }
    else if(socket->u16SockType == SOCK_STREAM)
    {
        dataLen = NetBufferDataRead(sid, (U08*)message, length, NULL);
        
        if(dataLen > 0)
//            TcpReceived(socket->pcb.tcp, dataLen);
            TcpReceived(socket->pcb.tcp, dataLen, sid);
    }
    else
        DPrintf("[SOCKET] unknown socket type");
        
    //socketEventStatusUpdate();
    
    return dataLen;
}



void mpx_RecvMsg()
{
    DPrintf(" To be implemented.");
}


void resolv_init(void);
int mm_init(void);

S32 mpx_NetworkInit()
{
    SDWORD status;
    static BOOL net_init;
    DPrintf("[socket] network task init");
    
	mpDebugPrint("net_init %d",net_init);
    if (net_init)
        return NO_ERR;

#if 0
    status = SemaphoreCreate(NET_SEM_ID, OS_ATTR_PRIORITY, 1);
    if (status != 0)
    {
        BREAK_POINT();
    }
#endif

    NetNicInit();
    
    SockInit();
    NetworkTaskInit();
    
    IpInit();
    DhcpInit();
    UdpInit();
    TcpInit();
    ArpInit();
    resolv_init();


#ifdef MP600
#if MAKE_XPG_PLAYER
    NetFileBrowerInitial();
#endif	
#endif
#if HAVE_FTP_SERVER
    mpx_FtpServiceEnable();
#endif

#ifdef NET_NAPI
    net_todo_init();
    net_done_init();
    xpgNetCtor(xpgNetHsm);
#endif

#ifdef VPERF_TEST
    vperf_Start();
#endif

#ifdef NETWORK_DEBUG_ENABLE
    NetDebugEnable();
#endif
    net_init = TRUE;
    return NO_ERR;
}



void mpx_SocketDebugPrint(U16 sid)
{
    ST_SOCKET *socket = SockGet(sid);
    
    DPrintf("[SOCKET] id = %d, local port = %d, peer port = %d", sid, socket->Local.u16Port, socket->Peer.u16Port);
}

/* 
 * mpx_NewLocalPort
 *
 * Return a unused port between SOCK_PORT_EPHEMERAL_START and 
 * (SOCK_PORT_EPHEMERAL_END - 1)
 */
U16 mpx_NewLocalPort()
{
    static U16 localPort = SOCK_PORT_EPHEMERAL_START;
    U16 port;
    U08 i;
    
    localPort += (mpx_SystemTickerGet() & 0xFFF);         /* add randomness */
    if (localPort < SOCK_PORT_EPHEMERAL_START)
        localPort = SOCK_PORT_EPHEMERAL_START;
    while(1)
    {
        if(localPort == SOCK_PORT_EPHEMERAL_END)
            port = localPort = SOCK_PORT_EPHEMERAL_START;
        else
            port = localPort++;
            
        for(i = 0; i < SOCK_MAXIMAL_NUMBER; i++)
        {
            if((SockBank[i].u08Type == SOCK_TYPE_ALLOCATED) && (SockBank[i].Local.u16Port == port))
                break;
        }
        
        if (i < SOCK_MAXIMAL_NUMBER)
            continue;                           /* already used */
        
        return port;
    }
    
    return 0;
}

extern int doprint;
static S32 sockAccept(U16 u16ListenSid, ST_TCP_PCB* pcb)
{
    //U16 sockId = pcb->u16SockId;
    register U32 mask;
    int ret;
    
    if(0 == mpx_QueuePush(sockConnQueue[u16ListenSid], (U32)pcb))
    {
        //MP_DEBUG1("sockAccept: mpx_QueuePush(%d) returns OK", u16ListenSid);
        //DPrintf("[SOCKET] push pcb %d %x", u16ListenSid, pcb);
        mask = 0x00000001 << u16ListenSid;
        MP_DEBUG2("sockAccept: mpx_EventSet(%d, 0x%x)", u08SockEventId,mask);
        ret = mpx_EventSet(u08SockEventId, mask);
        MP_DEBUG3("sockAccept: mpx_EventSet(%d, 0x%x) returns %d", u08SockEventId,mask,ret);
        
        return 0;
    }
    else
    {
        //MP_DEBUG1("sockAccept: mpx_QueuePush(%d) returns error", u16ListenSid);
        return -1;
    }
}

U16 mpx_SockType(U16 sid)
{
    ST_SOCKET *socket = SockGet(sid);
    return socket->u16SockType;
}

U16 mpx_SockLocalPortGet(U16 sid)
{
    ST_SOCKET *socket = SockGet(sid);
    return socket->Local.u16Port;
}

U16 mpx_SockPeerPortGet(U16 sid)
{
    ST_SOCKET *socket = SockGet(sid);
    return socket->Peer.u16Port;
}


#ifdef NEW_NETSOCKET
#include "net_socket2.c"
#endif

#if 1
U32 sockQueueLengthGet(int sid)
{
    return sockConnQueue[sid]->length;
}
#endif

