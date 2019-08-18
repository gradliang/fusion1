/**
 * @file
 *
 * This is the new socket library.  New applications should use this library,
 * instead of the old library.
 * 
 * Old socket library (e.g. mpx_Socket, mpx_Bind, ...etc) are still included.
 * Both libraries should be able to co-exist in a system.
 *
 * Implementation of BSD socket functions.  All functions follow BSD socket
 * library's usage as much as possible.
 *
 *  @li socket
 *  @li closesocket
 *  @li shutdown
 *  @li bind
 *  @li listen
 *  @li accept
 *  @li connect
 *  @li select
 *  @li sendto
 *  @li send
 *  @li recvfrom
 *  @li recv
 *  @li getsockopt
 *  @li setsockopt
 *  @li ioctlsocket
 *
 * The socket address arguments in the functions, bind(), accept(), and 
 * connect(), all use BSD's 'struct sockaddr', which is different from 
 * ST_SOCK_ADDR. ST_SOCK_ADDR is still used internally in Magic Pixel's
 * socket library.  The sizes of the these two structs:
 *
 *  @li struct sockaddr  - 16 octets
 *  @li ST_SOCK_ADDR     - 28 octets
 *
 * Copyright (c) 2007-2009 Magic Pixel Inc.
 * All rights reserved.
 */

/*
 * Modification History
 *
 * 20071221  Only socket(), select(), connect() are tested with CURL application.
 *           Other functions are not tested yet.
 *           Socket's read, write, and exception status are all supported in select().
 *           The function, connect(), supports blocking and non-blocking modes.
 */

#include <linux/if_ether.h>
#include "typedef.h"
#include "os.h"
#include "net_packet.h"
#include "socket.h"
#include "net_socket.h"
#include "error.h"
#include "util_queue.h"


#if 0
#define SOCKET_DEBUGF(debug,x) mpDebugPrint x
#else
#define SOCKET_DEBUGF(debug,x)
#endif

extern BYTE myethaddr[6];

void setErrno(int error);
int sock_Disconnect(int sid);
ST_UNIX_PCB* UnixNew();
void UnixFree(ST_UNIX_PCB* pcb);
int UnixBind(ST_UNIX_PCB* pcb, struct sockaddr_un *addr);
int UnixConnect(ST_UNIX_PCB* pcb, struct sockaddr_un *addr);
static int UnixSocketFind(char *path);
int UnixWrite(ST_UNIX_PCB* pcb, U08* data, S32 size, struct sockaddr_un *to);
extern BOOL sockIsCantRecvMore2(int sid);
extern void SockIdSignalTcpFinRecvd2(U16 sid);

void SockSignalTxReady(U16 id)
{
    register U32 mask;
    
    mask = 0x00000001 << id;
    SockBank[id].u32Signal |= SOCK_SIGNAL_TX_READY;
//    DPrintf("SockSignalTxReady: socket=%d", id);
    mpx_EventSet(u08SockEventId, mask);
}

void SockIdSignalTcpFinRecvd(U16 sid)
{
    
#ifdef HAVE_LWIP_SOCKET
    if (sid > 0x100)
    {
        SockIdSignalTcpFinRecvd2(sid);
        return;
    }
#endif

    if((sid >= SOCK_MAXIMAL_NUMBER) || (sid == 0))
    {
    	DPrintf("[SOCKET] socket id %d is invalid", sid);
        return;
    }
    
    if(SockBank[sid].u08Type != SOCK_TYPE_ALLOCATED)
    {
        DPrintf("[SOCKET] socket %d not allocated", sid);
        return;
    }
    
#ifdef NEW_NETSOCKET
    if (sockIsCantRecvMore(sid))
        return;
    else
#endif
        NetBufferPacketPush(sid, NULL);

    U32 mask = 0;

    mask = 0x00000001 << sid;
    SockBank[sid].u32Signal |= SOCK_SIGNAL_RX_DONE;
    mpx_EventSet(u08SockEventId, mask);
}

/**
 * @ingroup NET_SOCKET
 * @brief Create a socket.
 * 
 * Currently, the maximum number of sockets supported is 31.
 *
 * @param domain  Address family.  The following are supported:
 *              AF_INET,
 *              AF_PACKET,
 *              AF_UNIX.
 * @param type  Type for the new socket.  The following types are supported:
 *              SOCK_STREAM,
 *              SOCK_DGRAM,
 *              SOCK_RAW.
 * @param protocol Protocol to be used with the socket.
 *        0 Indicates that the default protocol for the @p type selected is to be used.
 *        The following types are supported:
 *        IPPROTO_TCP,
 *        IPPROTO_UDP.
 * @retval  >0  ID of the new socket.
 * @retval  -1   An error occurred.  The error code can be retrieved
 *          by calling getErrno().  Errno can be one of the following:
 * 
 *              @li <b>EAFNOSUPPORT</b>        The specified address family is not supported.
 *              @li <b>EPROTONOSUPPORT</b>     The specified protocol is not supported.
 *              @li <b>EPROTOTYPE</b>          The specified protocol is the wrong type for this socket.
 *              @li <b>ENOBUFS</b>             No buffer space is available. The socket cannot be created.
 *              @li <b>ENFILE</b>              No more socket descriptor available.
*/
int socket(int domain, int type, int protocol)
{
    register int status;
    register ST_SOCKET *socket;

    SOCKET_DEBUGF(0, ("socket: af=%d,type=%d,proto=%d", domain, type, protocol));

    if (domain == AF_UNSPEC)
        domain = AF_INET;                       /* use default */

    if(domain != AF_INET && domain != AF_PACKET && domain != AF_UNIX)
    {
        setErrno(EAFNOSUPPORT);
		return SOCKET_ERROR;
    }
    else if(type != SOCK_STREAM && type != SOCK_DGRAM && type != SOCK_RAW)
    {
        setErrno(EPROTONOSUPPORT);
		return SOCKET_ERROR;
    }
    else if(domain == AF_INET && type == SOCK_STREAM && protocol == IPPROTO_UDP)
    {
        setErrno(EPROTOTYPE);
		return SOCKET_ERROR;
    }
    else if(domain == AF_INET && type == SOCK_RAW && (protocol == IPPROTO_UDP || protocol == IPPROTO_TCP))
    {
        setErrno(EPROTONOSUPPORT);
		return SOCKET_ERROR;
    }
#if 1
    else if(domain == AF_NETLINK)
    {
        setErrno(EPROTONOSUPPORT);
		return SOCKET_ERROR;
    }
#endif
    else
    {
        if (protocol == 0)                      /* caller didn't specify protocol */
        {
            if(domain == AF_INET)
            {
                if (type == SOCK_STREAM)
                    protocol = IPPROTO_TCP;
                else if (type == SOCK_DGRAM)
                    protocol = IPPROTO_UDP;
            }
        }
        IntDisable();
        // critical section, protect it for thread-safe
        status = (int)SockAllocate();
        IntEnable();
        
        if(status > 0)
        {
            socket = &SockBank[status];
            socket->u08NetIndex = NetDefaultNicGet();
            socket->u16Family = domain;
            socket->u16SockType = type;
            socket->u16Protocol = protocol;
            
            socket->pcb.tcp = 0;
            if(domain == AF_UNIX)
            {
                socket->pcb.unix = UnixNew();
                if(!socket->pcb.unix)
                {
                    SockFree((U16)status);
                    setErrno(ENOBUFS);
                    return SOCKET_ERROR;
                }
            }
            else if(type == SOCK_STREAM)
            {
                socket->pcb.tcp = TcpNew();
                if(!socket->pcb.tcp)
                {
                    SockFree((U16)status);
                    setErrno(ENOBUFS);
                    return SOCKET_ERROR;
                }
                else
                    socket->pcb.tcp->u16SockId = (U16)status;
            }
            else if(type == SOCK_DGRAM)
            {
                socket->pcb.udp = UdpNew();
                if(!socket->pcb.udp)
                {
                    SockFree((U16)status);
                    setErrno(ENOBUFS);
                    return SOCKET_ERROR;
                }
            }
            
            memset(&socket->Local, 0, sizeof(ST_SOCK_ADDR));
            memset(&socket->Peer, 0, sizeof(ST_SOCK_ADDR));
            
            NetBufferTypeSet((U16)status, type);
            NetBufferFamilySet((U16)status, domain);
        }
        else
        {
            status = SOCKET_ERROR;
            setErrno(ENFILE);                   /* BSD compliant */
        }
    }
    
    return status;
}



/**
 * @ingroup NET_SOCKET
 * @brief Close an existing socket.
 * 
 * @param sockId  Descriptor identifying the socket to close.
 *
 * @retval  >0  ID of the new socket.
 * @retval  -1   An error occurred. Errno can be one of the following:
 * 
 *              @li <b>ENOTSOCK</b>        Not a valid socket.
 *
*/
int closesocket(int sockId)
{
    register ST_SOCKET *socket = &SockBank[sockId];
    U32 mask = 1UL << sockId;
    
    SOCKET_DEBUGF(0, ("closesocket: s=%d", sockId));
    if ((sockId < 0) || (sockId >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }
	
    if (socket->u16SockState & SS_NOFDREF)
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }

    if(socket->pcb.tcp)
    {
        if(socket->u16Family == AF_UNIX)
        {
            UnixFree(socket->pcb.unix);
            socket->pcb.unix = 0;
        }
        else if(socket->u16SockType == SOCK_DGRAM)
        {
            UdpFree(socket->pcb.udp);
            socket->pcb.udp = 0;
        }
        else if(socket->u16SockType == SOCK_STREAM)
        {
            DPrintf("closesocket(%d) calls sock_Disconnect",sockId);
            sock_Disconnect(sockId);

            socket->pcb.tcp->u16SockId = 0;
            socket->pcb.tcp = 0;
        }
    }
    
    mpx_EventClear(u08SockEventId, ~mask);
    
    SockFree(sockId);       /* deallocate socket */
    return NO_ERR;
}

/**
 * @ingroup NET_SOCKET
 * Disable sends or receives on a socket
 *
 * This routine shuts down all, or part, of a connection-based socket.
 * If the value of @p how is 0, receives are disallowed.  If @p how is 1,
 * sends are disallowed.  If @p how is 2, both sends and receives are
 * disallowed.
 *
 * @param sockId  Descriptor identifying a socket 
 * @param how  Flag that describes what types of operation will no longer be allowed
 *
 * @retval  0    successful
 * @retval  -1   failed
 *
 *              @li <b>ENOTSOCK</b>        Not a valid socket.
 *              @li <b>ENOTCONN</b>        The socket is not connected (connection-oriented sockets only).
 *              @li <b>EINVAL</b>        The how parameter is not valid.
 */
int shutdown(int sockId, int how)
{
    register ST_SOCKET *socket = &SockBank[sockId];

    SOCKET_DEBUGF(0, ("shutdown: s=%d,h=%d", sockId, how));

    if ((sockId <= 0) || (sockId >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
        return SOCKET_ERROR;
    }

    if(socket->u16SockType == SOCK_STREAM)
    {
        if (!(socket->u16SockState & SS_ISCONNECTED))
        {
            setErrno(ENOTCONN);
            return SOCKET_ERROR;
        }
    }

    switch (how)                           /* turn off flags */
    {
       case 0:
          socket->u16SockState |= (SS_CANTRCVMORE);
          break;
       case 1:
          socket->u16SockState |= (SS_CANTSENDMORE);
          if(socket->u16SockType == SOCK_STREAM)
              sock_Disconnect(sockId);
          break;
       case 2:
          socket->u16SockState |= (SS_CANTRCVMORE|SS_CANTSENDMORE);
          if(socket->u16SockType == SOCK_STREAM)
              sock_Disconnect(sockId);
          break;
       default:
          setErrno(EINVAL);
          return SOCKET_ERROR;
          break;
    }

    return 0;
}


/**
 * @ingroup NET_SOCKET
 * Associate a local address with a socket
 *
 * For TCP/IP, if the port is specified as zero, the bind assigns a unique port
 * to the application with a value between 4096 (SOCK_PORT_EPHEMERAL_START) to 
 * 0x7FFF (SOCK_PORT_EPHEMERAL_END).
 *
 * @param sid  Descriptor identifying an unbound socket 
 * @param name  Address to assign to the socket
 * @param namelen  Length of the value in the @p name parameter, in bytes.
 *
 * @retval  0  successful.
 * @retval  -1   An error occurred.  The error code can be retrieved
 *          by calling getErrno().  Errno can be one of the following:
 *
 *              @li <b>ENOTSOCK</b>        Not a valid socket.
 *              @li <b>EFAULT</b>        
 *              @li <b>EINVAL</b>         The socket is already bound to an address.
 */
int bind(int sid, const struct sockaddr *name, int namelen)
{
    register ST_SOCKET *socket = &SockBank[sid];
    ST_SOCK_ADDR *addr;
    struct sockaddr_in *inp;
    int ret;

    if ((sid < 0) || (sid >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }
	
    if(socket->u08Type == SOCK_TYPE_FREE)
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }
    
    if (socket->u16Family == PF_INET)
    {
    addr = &socket->Local;
    if(addr->u16Port != 0)
    {
        setErrno(EINVAL);                       /* already bound */
		return SOCKET_ERROR;
    }
    
        inp = (struct sockaddr_in *)name;
#if 1
        if ((inp->sin_addr.s_addr != INADDR_ANY) && (inp->sin_port != 0))
        {
            if (!(socket->u16SockOptions & SO_REUSEADDR))   /* SO_REUSEADDR is not set */
            {
                register U32 index = 1;
                U16 proto = socket->u16Protocol;
                in_addr_t srcIp = inp->sin_addr.s_addr;
                in_port_t port = inp->sin_port;
                while(index < SOCK_MAXIMAL_NUMBER)
                {
                    if(SockBank[index].u08Type == SOCK_TYPE_ALLOCATED &&
                       index != sid &&
                       SockBank[index].u16Family == PF_INET &&
                       SockBank[index].u16Protocol == proto)
                    {
                        if ( (((ST_SOCK_ADDR_IN *)&SockBank[index].Local.u08Address)->u32Addr == srcIp) &&
                             (SockBank[index].Local.u16Port == port) )
                        {
                            setErrno(EADDRINUSE);
                            return SOCKET_ERROR;
                        }
                    }
                    index ++;
                }
            }
        }
#endif
    addr->u16Family = inp->sin_family;
    if (inp->sin_port == 0)
        addr->u16Port = mpx_NewLocalPort();
    else
        addr->u16Port = inp->sin_port;
	
    memcpy(addr->u08Address, (char *)&(inp->sin_addr), sizeof(struct in_addr));

        SOCKET_DEBUGF(0, ("bind: s=%d,p=%d", sid,addr->u16Port));
    }
    else if (socket->u16Family == PF_LOCAL)
        SOCKET_DEBUGF(0, ("bind: s=%d,p=%s", sid,((struct sockaddr_un *)name)->sun_path));
    else if (socket->u16Family == PF_NETLINK)
    {
        setErrno(ENOTSUPP);
		return SOCKET_ERROR;
    }

    switch(socket->u16SockType)
    {
        case SOCK_STREAM:
            ret = TcpBind2(socket->pcb.tcp, ((ST_SOCK_ADDR_IN *)(addr->u08Address))->u32Addr, ntohs(addr->u16Port));
            if (ret < 0)
                setErrno(EFAULT);
        break;
        
        case SOCK_DGRAM:
            if (socket->u16Family == PF_LOCAL)
                ret = UnixBind(socket->pcb.unix, (struct sockaddr_un *)name);
            else if (socket->u16Family == PF_INET)
            ret = UdpBind(socket->pcb.udp, ((ST_SOCK_ADDR_IN *)(addr->u08Address))->u32Addr, ntohs(addr->u16Port));
            else /* if (socket->u16Family == PF_PACKET) TODO */
                ret = 0;
    if (ret < 0)
                setErrno(EFAULT);
        break;
        default:
            ret = SOCKET_ERROR;
            setErrno(EFAULT);
        break;
    }
    
    return ret;
}



/**
 * @ingroup NET_SOCKET
 * @brief Place a socket in a state in which it is listening for an incoming connection
 *
 * @param sockId   Descriptor identifying a bound, unconnected socket.
 * @param backlog  Maximum length of the queue of pending connections.
 *
 * @retval  0  no error occurs.
 * @retval  -1   An error occurred.  The error code can be retrieved
 *          by calling getErrno().  Errno can be one of the following:
 *
 *              @li <b>ENOTSOCK</b>        Not a valid socket.
 *              @li <b>ENOTSUPP</b>        The referenced socket is not of a type that supports the listen operation.
 *              @li <b>EINVAL</b>          The socket has not been bound with bind.
 */
int listen(int sockId, int backlog)
{
    S32 err = NO_ERR;
    register ST_SOCKET *socket = &SockBank[sockId];
    
    SOCKET_DEBUGF(0, ("listen: s=%d", sockId));

    if ((sockId < 0) || (sockId >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }

    if(socket->u08Type == SOCK_TYPE_FREE)
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }
        
    if(socket->u16SockType != SOCK_STREAM)
    {
        setErrno(ENOTSUPP);                     /* TODO */
		return SOCKET_ERROR;
    }
    
    if(sockConnQueue[sockId] == 0)
    {
        sockConnQueue[sockId] = mpx_QueueCreate();
        if(sockConnQueue[sockId] == 0)
        {
            DPrintf("[SOCKET] queue create fail");
            setErrno(ENOBUFS);
            return SOCKET_ERROR;
        }
    }
    
    TcpAcceptCallbackSet(socket->pcb.tcp, sockAccept);
    err = TcpListen(socket->pcb.tcp);
    if (err == NO_ERR)
        socket->u16SockOptions |= SO_ACCEPTCONN;
    else
    {
        setErrno(EINVAL);
        return SOCKET_ERROR;
    }
    
    return 0;
}



/**
 * @ingroup NET_SOCKET
 * @brief Accept an incoming connection attempt on a socket
 *
 * @param sid   Descriptor identifying a socket that has been placed in a listening state with the listen function.
 * @param addr  Optional pointer to a buffer that receives the address of the connecting entity.
 * @param addrlen  Optional pointer to a integer that containes the length of @p addr.
 *
 * @retval  > 0  New socket ID if no error occurs.
 * @retval  -1   An error occurred.  The error code can be retrieved
 *          by calling getErrno().  Errno can be one of the following:
 *
 *              @li <b>ENOTSOCK</b>        Not a valid socket.
 *              @li <b>EINVAL</b>          The listen function was not invoked prior to accept.
 *              @li <b>ENFILE</b>          No more socket descriptor available.
 */
int accept(int sid, struct sockaddr *addr, int *addrlen)
{
    ST_SOCK_ADDR address, *ap=&address;
    struct sockaddr_in *inp = (struct sockaddr_in *)addr;
    register ST_SOCKET *socket = &SockBank[sid];
    int ret;

    SOCKET_DEBUGF(0, ("accept: s=%d", sid));

    if(sid <=0 || (sid >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }
        
    if (!(socket->u16SockOptions & SO_ACCEPTCONN))
    {
        setErrno(EINVAL);
        return SOCKET_ERROR;
    }

//    ret =  mpx_Accept(sid, ap);

    {
        U32 item;
        ST_TCP_PCB* pcb;
        register ST_SOCKET *listenSocket = &SockBank[sid];
        register ST_SOCKET *newSocket;

        if(0 == mpx_QueueLengthGet(sockConnQueue[sid]))
            return 0;

        if(0 != mpx_QueuePop(sockConnQueue[sid], &item))
        {
            DPrintf("[SOCKET] queue pop fail");
            return 0;
        }

        pcb = (ST_TCP_PCB*)item;

        IntDisable();
        // critical section, protect it for thread-safe
        ret = SockAllocate();
        IntEnable();

        if (ret > 0)
        {
            newSocket = &SockBank[ret];

            newSocket->u08NetIndex = listenSocket->u08NetIndex;
            newSocket->u16Family = listenSocket->u16Family;
            newSocket->u16SockType = listenSocket->u16SockType;
            newSocket->u16Protocol = 0;            // fix at 0 anyway

            newSocket->pcb.tcp = pcb;

            DPrintf("[SOCKET] accept connection");
            DPrintf("   from \\-");
            NetDebugPrintIP(pcb->u32PeerIp);
            DPrintf("   port %d", pcb->u16PeerPort);
            DPrintf("   at local port %d", pcb->u16LocalPort);

            mpx_SockAddrSet(&newSocket->Local, pcb->u32LocalIp, pcb->u16LocalPort);
            mpx_SockAddrSet(&newSocket->Peer, pcb->u32PeerIp, pcb->u16PeerPort);
            mpx_SockAddrSet(ap, pcb->u32PeerIp, pcb->u16PeerPort);

            NetBufferTypeSet((U16)ret, SOCK_STREAM);

            TcpAccept(pcb, (U16)ret);

            newSocket->u16SockState |= SS_ISCONNECTED;

            if (inp)
            {
                inp->sin_family = ap->u16Family;
                inp->sin_port = ap->u16Port;
				
                memcpy(&(inp->sin_addr), ap->u08Address, sizeof(inp->sin_addr));
            }
        }
        else
        {
            ret = SOCKET_ERROR;
            setErrno(ENFILE);
        }

    }
    return ret;
}



/**
 * @ingroup NET_SOCKET
 * @brief Initiate a connection to a remote socket
 *
 * @param sockId  Descriptor identifying an unconnected socket
 * @param name  Name of the socket to which the connection should be established.
 * @param namelen  Length of @p name, in bytes.
 *
 * @retval  > 0  New socket ID.
 * @retval  -1   An error occurred.  The error code can be retrieved
 *          by calling getErrno().  Errno can be one of the following:
 *
 *              @li <b>ENOTSOCK</b>        Not a valid socket.
 *              @li <b>EFAULT</b>        
 *              @li <b>EISCONN</b>         The socket is already connected.
 *              @li <b>EALREADY</b>        A nonblocking connect call is in progress on the specified socket.
 *              @li <b>EWOULDBLOCK</b>     The socket is marked as nonblocking and the connection cannot be completed immediately.
 *              @li <b>ENOBUFS</b>         No buffer space is available. The socket cannot be connected.
 *
 */
int connect(int sockId, const struct sockaddr *name, int namelen)
{
    register ST_SOCKET *socket = &SockBank[sockId];
    struct sockaddr_in *addr;
    ST_SOCK_ADDR *destAddr;
    S32 status = NO_ERR;
    
    SOCKET_DEBUGF(0, ("connect: s=%d", sockId));
    if(!sockId || (sockId >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
        return SOCKET_ERROR;
    }
    
    if (socket->u16Family == AF_INET)
    {
        if(!socket->Local.u16Family)
        {
            mpx_SockAddrSet(&socket->Local, NetDefaultIpGet(), mpx_NewLocalPort());
            addr = (struct sockaddr_in *)&socket->Local;
            switch(socket->u16Protocol)
            {
                case IPPROTO_TCP:
                    status = TcpBind2(socket->pcb.tcp, ntohl(addr->sin_addr.s_addr), ntohs(addr->sin_port));
                    break;

                case IPPROTO_UDP:
                    status = UdpBind(socket->pcb.udp, ntohl(addr->sin_addr.s_addr), ntohs(addr->sin_port));
                    break;
                default:
                    status = SOCKET_ERROR;
                    break;
            }

            if (status < 0)
            {
                setErrno(EFAULT);
                return SOCKET_ERROR;
            }
        }
        //    memcpy(&socket->Peer, destAddr, sizeof(ST_SOCK_ADDR));
        addr = (struct sockaddr_in *) name;
        destAddr = &socket->Peer;
        destAddr->u16Family = addr->sin_family;
        destAddr->u16Port = addr->sin_port;
		
        memcpy(destAddr->u08Address, (char *)&(addr->sin_addr), sizeof(struct in_addr));
        SOCKET_DEBUGF(0, ("connect: socket=%d,port=%d", sockId, destAddr->u16Port));
    }
    
    if(SockBank[sockId].u16SockType == SOCK_DGRAM)
    {
        if (socket->u16Family == AF_LOCAL)      /* unix (local) domain socket */
            status = UnixConnect(socket->pcb.unix, (struct sockaddr_un *)name);
        else if (socket->u16Family == AF_INET)
        {   // UDP connection
            status = UdpConnect(socket->pcb.udp, ((ST_SOCK_ADDR_IN *)(destAddr->u08Address))->u32Addr, destAddr->u16Port);
        }
    }
    else
    {
        if (socket->u16SockState & SS_ISCONNECTED)
        {
            errno = EISCONN;
            return SOCKET_ERROR;
        }
        if (socket->u16SockState & SS_ISCONNECTING)
        {
            errno = EALREADY;
            return SOCKET_ERROR;
        }

        status = TcpConnect2(socket->pcb.tcp, ((ST_SOCK_ADDR_IN *)(destAddr->u08Address))->u32Addr, destAddr->u16Port);

        if (status)
        {
            errno = ENOBUFS;
            return SOCKET_ERROR;
        }

        socket->u16SockState |= SS_ISCONNECTING;

        if ( socket->u16SockState &  SS_NBIO )  /* non-blocking socket */
        {
            errno = EWOULDBLOCK;
            status = SOCKET_ERROR;
        }
        else                                    /* blocking socket */
        {
            U32 mask = 1 << sockId;
            mpx_EventWait(u08SockEventId, mask, OS_EVENT_OR, &mask);

            if ( socket->u16SockError )         /* connection failed */
            {
                errno = (int) socket->u16SockError;
                socket->u16SockError = 0;
                status = SOCKET_ERROR;
            }
        }
    }
    
    return status;
}



/**
 * @ingroup NET_SOCKET
 * @brief Determine the status of one or more sockets, waiting if necessary.
 * 
 * @param nfds  Ignored; included only for compatibility with Berkeley sockets 
 * @param read  A set of sockets to be checked for readability. Optional parameter.
 * @param write A set of sockets to be checked for writability. Optional parameter.
 * @param excep A set of sockets to be checked for errors. Optional parameter.
 * @param timeout  Maximum wait time.  Set the @p timeout parameter to NULL for 
 *              blocking operations.  If value of the @p timeout parameter is set 
 *              to zero ({0,0}), then select will return immediately.
 * 
 * @retval  >0  The number of the sockets that meet the conditions.
 * @retval  0   No sockets that meet the conditions.
 * @retval  -1   An error occurred.  The error code can be retrieved
 *          by calling getErrno().  Errno can be one of the following:
 * 
 *              @li <b>ENETDOWN</b>        Internal error in the network subsystem.
 *              @li <b>EINVAL</b>          No socket in all three descriptor set parameters and the time-out value is not valid.
 *              @li <b>ENETDOWN</b>        The network subsystem has failed.
*/
int select(int nfds, ST_SOCK_SET *read, ST_SOCK_SET *write, ST_SOCK_SET *excep, struct timeval *timeout)
{
    register S32 status;
    register U32 mask;
    U32 maskSave;
    S32 tm;
    U32 result;
    ST_SOCKET *socket;
    ST_SOCK_SET readMask, writeMask, exceptMask;
    S32 old_ticks, new_ticks;
    S32 elapsed;
    
    if (timeout)
    tm = (timeout->tv_sec * 1000 + timeout->tv_usec / 1000); /* in millisecond */
    else
        tm = 0;                                 /* blocking indefinitely */

    MPX_FD_ZERO(&readMask);
    MPX_FD_ZERO(&writeMask);
    MPX_FD_ZERO(&exceptMask);

    mask = 0;
    if(read)   { mask |= read->u32Mask; readMask = *read; MPX_FD_ZERO(read); }
    if(write)  { mask |= write->u32Mask; writeMask = *write; MPX_FD_ZERO(write); }
    if(excep)  { mask |= excep->u32Mask; exceptMask = *excep; MPX_FD_ZERO(excep); }
    maskSave = mask;

    if (maskSave == 0 && !tm)
    {
        setErrno(EINVAL);
        return SOCKET_ERROR;
    }
    
    old_ticks = mpx_SystemTickerGet();

    result = readMask.u32Mask | writeMask.u32Mask | exceptMask.u32Mask;

    do {
        int sid;
        status = 0;         // to be the socket count
        mask = 1L;
        if (result == 0)
            goto chk_status;
        for (sid=0; sid<SOCK_MAXIMAL_NUMBER; sid++, mask<<=1)       /* check fds */
        {
            if(result & mask)
            {
                socket = &SockBank[sid];
                if(mask & readMask.u32Mask)   
                { 
                    socket = &SockBank[sid];
                    if (socket->u16SockType==SOCK_STREAM) { /* TCP */
                        if ((socket->u16SockState & SS_NOFDREF)) { /* TODO */
                            status++;
                            read->u32Mask |= mask;
                        }
                        else if (socket->u16SockOptions & SO_ACCEPTCONN) {
                            if (mpx_QueueLengthGet(sockConnQueue[sid]) > 0) {
                                status++;
                                read->u32Mask |= mask;
                            }
                        }
                        else if (socket->u16SockState & SS_ISCONNECTED) {
                            if ((NetBufferPacketNumGet(sid) > 0)) {
                                status++;
                                read->u32Mask |= mask;
                            }
                        }
                        else if ( !(socket->u16SockState & (SS_ISCONNECTING|SS_ISCONNECTED|SS_ISDISCONNECTING)) ) {
                            status++;
                            read->u32Mask |= mask;
                        }
                    }
                    else if ((NetBufferPacketNumGet(sid) > 0)) {
                        status++;
                        read->u32Mask |= mask;
                    }
                }


                if(mask & writeMask.u32Mask)  
                { 
                    socket = &SockBank[sid];
                    if (socket->u16SockType==SOCK_STREAM) { /* TCP */
                        if ((socket->u16SockState & SS_ISCONNECTED) &&
                                !(socket->u16SockState & SS_CANTSENDMORE) &&
                                (socket->pcb.tcp->u16SendBuffer > 0))     /* send buffer is available */
                        {
                            status++;
                            write->u32Mask |= mask;

                            socket->u32Signal &= ~SOCK_SIGNAL_TX_READY;
                        }
                    } 
                    else {                                  /* UDP/RAW */
                        if (!(socket->u16SockState & SS_NOFDREF)) { /* TODO */
                            status++;
                            write->u32Mask |= mask;
                        }
                    }
                }

                    if(mask & exceptMask.u32Mask)  
                    { 
                        if (socket->u16SockType == SOCK_STREAM) { /* TCP */

                            /* ---  In processing a connect call (non-blocking), connect failed  --- */

                            if ( !(socket->u16SockState & (SS_ISCONNECTING|SS_ISCONNECTED|SS_ISDISCONNECTING)) )
                            {
                                excep->u32Mask |= mask;
                                status++;

                                errno = (int) socket->u16SockError;
                                socket->u16SockError = 0;
                            }
                        }
                    }
                }

        }

chk_status:
        if (status > 0)
            break;                              /* done */
        else if (tm == 0 && timeout)
            break;                              /* return immediately */
        else if (tm > 0)
        {
            new_ticks = mpx_SystemTickerGet();
            elapsed = (new_ticks - old_ticks) * 1000 / HZ; /* milliseconds */
            old_ticks = new_ticks;
            if (tm <= elapsed)
                break;                              /* time limit expires */
            else
                tm -= elapsed;
        }


        //MP_DEBUG3("select: mpx_EventWaitWto(%d,0x%x,%d)", u08SockEventId, maskSave,tm);
        /* --- if tm == 0, then wait forever until at least one socket meets the conditions --- */
        status = mpx_EventWaitWto(u08SockEventId, maskSave, OS_EVENT_OR, &result, tm);
        //MP_DEBUG1("select: mpx_EventWaitWto returns %d", status);
    } while (status == 0);

    if (status == OS_STATUS_TIMEOUT){
        status = 0;                             /* time limit expires */
    }
    else 
    if (status < 0)
    {
        status = SOCKET_ERROR;
        setErrno(ENETDOWN);
    }

    return status;
}

/**
 * @ingroup NET_SOCKET
 * @brief Send data to a specific destination.  If the socket is not bound before, 
 *        sendto() will do an implicit bind.
 * 
 * @param sid  Socket ID 
 * @param buffer  Buffer containing the data to be transmitted.
 * @param size Length of the data in buffer, in bytes.
 * @param flags Indicator specifying the way in which the call is made.
 * @param to  Optional pointer to a sockaddr structure that contains the address of the target socket.
 *            If it's a connection-oriented socket, this parameter is ignored.
 * @param tolen Size of the address in to, in bytes.
 * 
 * @retval  >=0  The total number of bytes sent.
 * @retval  -1   An error occurred.  The error code can be retrieved
 *          by calling getErrno().  Errno can be one of the following:
 *
 *              @li <b>ENOTSOCK</b>        Not a valid socket.
 *              @li <b>EMSGSIZE</b>        The socket is message oriented, and the message is larger than the maximum supported by the underlying transport.
 *              @li <b>EDESTADDRREQ</b>    A destination address is required.
 *              @li <b>ENOBUFS</b>         No buffer space is available.
 *              @li <b>EFAULT</b>          Socket type is not supported.
 *              @li <b>ENETDOWN</b>        Network subsystem is down.
 *
*/
int sendto(int sid, const char *buffer, int size, int flags,
			const struct sockaddr *to, int tolen)
{
    ST_SOCKET *socket;
    S32 status;
    struct sockaddr sa_buf, *from;
    struct sockaddr_in *peer;
    ST_NET_PACKET *packet;
    
    SOCKET_DEBUGF(0, ("sendto: s=%d,sz=%d", sid,size));
    if ((sid <= 0) || (sid >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
        return SOCKET_ERROR;
    }

    socket = &SockBank[sid];

    if(socket->u16Family == PF_PACKET)
    {
		struct sockaddr_ll *ll;
	
        packet = NetNewPacket(FALSE);
        if(!packet)
        {
            setErrno(ENOBUFS);
            return SOCKET_ERROR;
        }
        
        socket->outPoint = (void*)packet;

		peer = (struct sockaddr_in *)to;
		ll = (struct sockaddr_ll *)to;

		NetMacAddrCopy(socket->Peer.u08Address, ll->sll_addr);
	
			memcpy(NET_PACKET_IP(packet), buffer, size);          // and then the data portion
        packet->Net.u16PayloadSize = size;
        packet->Net.u16SockId = sid;
        packet->Net.u08NetIndex = NetDefaultNicGet();
        status = NetPacketSend(packet, 0, ntohs(peer->sin_port));
#ifndef DRIVER_FREE_TXBUFFER
        NetFreePacket(packet);
#endif
        if (status < 0)
        {
            setErrno(-status);
            return -1;
        }
        else
            return status;
    }
    else if(socket->u16Family == AF_LOCAL)
    {
        if(socket->u16SockType == SOCK_DGRAM)
        {
            return UnixWrite(socket->pcb.unix, buffer, size, to);
        }
        else
        {
            setErrno(EAFNOSUPPORT);
            return SOCKET_ERROR;
        }
    }
    else
    {
        if(socket->u16SockType == SOCK_DGRAM)
        {
            if((!socket->Local.u16Port))            /* socket is not bound */
            {
                DPrintf("[SOCKET] socket is not bound s=%d. Do a implicit bind", sid);
                from = &sa_buf;
                memset(from, 0, sizeof(struct sockaddr));
                from->sa_family = AF_INET;
                status = bind(sid, from, sizeof(struct sockaddr));
                if (status < 0)
                    return SOCKET_ERROR;
            }

            if (size > MAX_UDP_DATA)
            {
                setErrno(EMSGSIZE);
                return SOCKET_ERROR;
            }

            if(!to)
            {
                setErrno(EDESTADDRREQ);
                return SOCKET_ERROR;
            }

            packet = NetNewPacket(FALSE);
            if(!packet)
            {
                setErrno(ENOBUFS);
                return SOCKET_ERROR;
            }

            socket->outPoint = (void*)packet;
			
				mmcp_memcpy(NET_PACKET_UDP_DATA(packet), buffer, size);          // and then the data portion

            packet->Net.u16PayloadSize = size;
            packet->Net.u16SockId = sid;

            peer = (struct sockaddr_in *)to;

            status = UdpPacketSend(packet, 
                    ((ST_SOCK_ADDR_IN*)(socket->Local.u08Address))->u32Addr,
                    socket->Local.u16Port,
                    ntohl(peer->sin_addr.s_addr),
                    ntohs(peer->sin_port));


            if (status < 0)
            {
                setErrno(-status);
                return -1;
            }
            else
                return status;
        }
        else if(socket->u16SockType == SOCK_STREAM)
        {
            return send(sid, buffer, size, flags);
        }
        else if(socket->u16SockType == SOCK_RAW)
        {
            if (size > MAX_IP_DATA)
            {
                setErrno(EMSGSIZE);
                return SOCKET_ERROR;
            }

            if(!to)
            {
                setErrno(EDESTADDRREQ);
                return SOCKET_ERROR;
            }

            packet = NetNewPacket(FALSE);
            if(!packet)
            {
                setErrno(ENOBUFS);
                return SOCKET_ERROR;
            }

            socket->outPoint = (void*)packet;
			
				mmcp_memcpy(NET_PACKET_UDP(packet), buffer, size);          // and then the data portion

            packet->Net.u16PayloadSize = size;
            packet->Net.u16SockId = sid;

            peer = (struct sockaddr_in *)to;

            status = IpPacketSend(packet, 
                    socket->u16Protocol,
                    ((ST_SOCK_ADDR_IN*)(socket->Local.u08Address))->u32Addr,
                    ntohl(peer->sin_addr.s_addr));

#ifndef DRIVER_FREE_TXBUFFER
            NetFreePacket(packet);
#endif

            if (status < 0)
            {
                setErrno(-status);
                return -1;
            }
            else
                return status;
        }
        else
        {
            setErrno(EFAULT);
            return SOCKET_ERROR;
        }
    }
}

/**
 * @ingroup NET_SOCKET
 * @brief Send data on a connected socket
 * 
 * @param sid  ID of a connected socket 
 * @param buffer  Buffer containing the data to be transmitted.
 * @param size Length of the data in buffer, in bytes.
 * @param flags Indicator specifying the way in which the call is made.
 * 
 * @retval  >=0  The total number of bytes sent.
 * @retval  -1   An error occurred.  The error code can be retrieved
 *          by calling getErrno().  Errno can be one of the following:
 *
 *              @li <b>ENOTSOCK</b>        Not a valid socket.
 *              @li <b>EMSGSIZE</b>        The socket is message oriented, and the message is larger than the maximum supported by the underlying transport.
 *              @li <b>EDESTADDRREQ</b>    A destination address is required.
 *              @li <b>ENOBUFS</b>         No buffer space is available.
 *              @li <b>EFAULT</b>          Socket type is not supported.
 *              @li <b>ENETDOWN</b>        Network subsystem is down.
 *
*/
int send(int sid, const char *buffer, int size, int flags)
{
    ST_SOCKET *socket;
    S32 status;
    struct sockaddr sa_buf, *from;
    
    SOCKET_DEBUGF(0, ("send: s=%d,sz=%d", sid,size));
    socket = &SockBank[sid];
    
    if(socket->u08Type == SOCK_TYPE_FREE)
	{
        return -1;
	}
    if(socket->u16Family == AF_LOCAL)
    {
        if(socket->u16SockType == SOCK_DGRAM)
        {
            return UnixWrite(socket->pcb.unix, buffer, size, NULL);
        }
        else
        {
            setErrno(EAFNOSUPPORT);
            return SOCKET_ERROR;
        }
    }
    else
    if(socket->u16SockType == SOCK_DGRAM)
    {
        if(socket->u16Family == AF_INET && (!socket->Local.u16Port))            /* socket is not bound */
        {
            DPrintf("[SOCKET] socket is not bound s=%d. Do a implicit bind", sid);
            from = &sa_buf;
            memset(from, 0, sizeof(struct sockaddr));
            from->sa_family = AF_INET;
            status = bind(sid, from, sizeof(struct sockaddr));
            if (status < 0)
                return SOCKET_ERROR;
        }

        if((!socket->pcb.udp->u32PeerIp) || (!socket->pcb.udp->u16PeerPort))
        {
            DPrintf("[SOCKET] peer address is not set");
            setErrno(ENOTCONN);
            return SOCKET_ERROR;
        }
        
        return UdpWrite(socket->pcb.udp, buffer, size);
    }
    else if(socket->u16SockType == SOCK_STREAM)
    {
        if (!(socket->u16SockState & SS_ISCONNECTED))
        {
            setErrno(ENOTCONN);
            return SOCKET_ERROR;
        }
        else if (socket->u16SockState & SS_CANTSENDMORE)
        {
            setErrno(ESHUTDOWN);
            return SOCKET_ERROR;
        }

        int r = TcpWrite(socket->pcb.tcp, buffer, size);
        if (r < 0)
        {
            setErrno(-r);
            return -1;
        }
        else
            return r;
    }
    
    setErrno(EFAULT);
    return SOCKET_ERROR;
}

/**
 * @ingroup NET_SOCKET
 * Receive data from a socket
 *
 * @param sid  Descriptor identifying a bound socket 
 * @param message  Buffer for the incoming data
 * @param length Length of @p message, in bytes
 * @param flags Indicator specifying the way in which the call is made.
 * @param from  Optional pointer to a buffer that will hold the source address upon return.
 * @param fromlen Optional pointer to the size, in bytes, of the @p from buffer.
 *
 * @retval >0    number of bytes received.
 * @retval  0    connection is closed gracefully.
 * @retval  -1   failed
 *
 * Error codes:
 *   @li <b>ENOTSOCK</b>        The descriptor is not a socket
 *   @li <b>ESHUTDOWN</b>       The socket has been shut down; it is not possible to recvfrom on a socket after shutdown has been invoked with how set to 0 or 2.
 *   @li <b>EWOULDBLOCK</b>     The socket is marked as nonblocking and the recvfrom operation would block.
 *   @li <b>ECONNRESET</b>      The virtual circuit was reset by the remote side 
 *   @li <b>ETIMEDOUT</b>       The connection has been dropped.
 */
int recvfrom(int sid, void *message, int length, int flags, struct sockaddr *from, socklen_t *fromlen)
{
    ST_SOCKET *socket = SockGet(sid);
    S32 dataLen = 0;
    
    SOCKET_DEBUGF(0, ("recvfrom: s=%d", sid));
    if(!socket)
    {
        setErrno(ENOTSOCK);
        return SOCKET_ERROR;
    }
    
    if (socket->u16SockState & SS_NOFDREF)
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }

tryagain:

    if(socket->u16SockType == SOCK_DGRAM)
    {
        if(socket->u16Protocol == IPPROTO_UDP)
        {
            dataLen = NetBufferDataRead(sid, (U08*)message, length, NULL);
            if(dataLen > 0)
            {
#if 0
                mpx_SockAddrSet(&(socket->Peer), NetBufferFromIpGet(sid), NetBufferFromPortGet(sid));
#endif
                if(from)
                {
#if 0
                    //                memcpy(from, &(socket->Peer), sizeof(ST_SOCK_ADDR));
                    ((struct sockaddr_in *)from)->sin_family = socket->Peer.u16Family;
                    ((struct sockaddr_in *)from)->sin_port = socket->Peer.u16Port;

                    NetMacAddrCopy(&((struct sockaddr_in *)from)->sin_addr, &socket->Peer.u08Address);
#else
                    mpx_SockAddrSet((ST_SOCK_ADDR *)from, NetBufferFromIpGet(sid), NetBufferFromPortGet(sid));
#endif
                }
            }
        }
        else if(socket->u16Protocol != 0)
        {
            dataLen = NetBufferDataRead(sid, (U08*)message, length, NULL);
            if(dataLen > 0 && from)
            {
                memset(from, 0, sizeof(struct sockaddr));
                from->sa_family = SockBank[sid].u16Family;
                if (SockBank[sid].u16Family == PF_PACKET)
                {
                    ST_SOCK_ADDR_LL *llp;
                    llp = (ST_SOCK_ADDR_LL *)from;
					
                    memcpy(llp->sll_addr, (void *)NetBufferFromMacAddrGet(sid), ETHERNET_MAC_LENGTH);
                }
            }
        }
        else
        {
            dataLen = NetBufferDataRead(sid, (U08*)message, length, from);
            if(dataLen > 0)
            {
                if (dataLen > length)
                {
                    BREAK_POINT();
                }
            }
        }
    }
    else if(socket->u16SockType == SOCK_STREAM)
    {
        if (socket->u16SockState & SS_CANTRCVMORE)
        {
            setErrno(ESHUTDOWN);
            return SOCKET_ERROR;
        }

        dataLen = NetBufferDataRead(sid, (U08*)message, length, NULL);
        
        if(dataLen > 0)
        {
//            TcpReceived(socket->pcb.tcp, dataLen);
            TcpReceived(socket->pcb.tcp, dataLen, sid);
        }
        else if(dataLen == 0)                   /* no more data and TCP FIN has been received */
        {
            if (!(socket->u16SockState & (SS_ISCONNECTED|SS_ISCONNECTING)))
            {
                if ( socket->u16SockError )         /* connection is reset/timedout */
                {
                    setErrno(socket->u16SockError);
                    socket->u16SockError = 0;
                    return SOCKET_ERROR;
                }
            }
            return 0;
        }
        else
            dataLen = 0;
    }
    else if(socket->u16SockType == SOCK_RAW)
    {
        if (fromlen && *fromlen > 0)
            memset(from, 0, *fromlen);
        else
            memset(from, 0, sizeof(struct sockaddr));
        dataLen = NetBufferDataRead(sid, (U08*)message, length, from);
    }
    else
    {
        DPrintf("[SOCKET] unknown socket type");
        setErrno(ENOTSOCK);
        return SOCKET_ERROR;
    }
        
    if(dataLen == 0)
    {
        if ( socket->u16SockState &  SS_NBIO )  /* non-blocking socket */
        {
            setErrno(EWOULDBLOCK);
            dataLen = SOCKET_ERROR;
        }
        else                                    /* blocking socket */
        {
            U32 mask = 1L << sid;
            mpx_EventWait(u08SockEventId, mask, OS_EVENT_OR, &mask);

            goto tryagain;
        }
    }
    else if (dataLen > 0)
    {
        if (from && fromlen)
        {
            if (from->sa_family == AF_INET)
                *fromlen = sizeof(sa_family_t) + 2 + 4;
            else if (from->sa_family == AF_PACKET)
                *fromlen = sizeof(sa_family_t) + 6;
            else if (from->sa_family == AF_UNIX)
                *fromlen = sizeof(sa_family_t) + strlen(((struct sockaddr_un *)from)->sun_path);
        }
    }

    return dataLen;
}

/**
 * @ingroup NET_SOCKET
 * Receive data from a connected or bound socket
 *
 * @param sid  Descriptor identifying a connected socket 
 * @param mem  Buffer for the incoming data
 * @param len Length of @p message, in bytes
 * @param flags Indicator specifying the way in which the call is made.
 *
 * @retval >0    number of bytes received.
 * @retval  0    connection is closed gracefully.
 * @retval  -1   failed
 *
 * Error codes:
 *   @li <b>ENOTSOCK</b>        The descriptor is not a socket
 *   @li <b>ESHUTDOWN</b>       The socket has been shut down; it is not possible to recvfrom on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH.
 *   @li <b>EWOULDBLOCK</b>     The socket is marked as nonblocking and the recvfrom operation would block.
 *   @li <b>ECONNRESET</b>      The virtual circuit was reset by the remote side 
 *   @li <b>ETIMEDOUT</b>       The connection has been dropped.
 */
int recv(int sid, void *mem, int len, unsigned int flags)
{
    SOCKET_DEBUGF(0, ("recv: s=%d", sid));
    return recvfrom(sid, mem, len, flags, NULL, NULL);
}


/**
 * @ingroup NET_SOCKET
 * Get options associated with the specified socket.  
 *
 * Options supported include:
 *
 *  <b>SO_ERROR</b>
 *
 * Levels supported include:
 *
 *  <b>SOL_SOCKET</b>
 *
 * @param sid  Descriptor identifying a socket 
 * @param level  Level at which the option is defined.
 * @param optname  Socket option for which the value is to be retrieved.  The supported
 *                 options include SO_ERROR, SO_REUSEADDR.
 * @param optval  Pointer to the buffer in which the value for the requested 
 *                option is to be returned.
 * @param optlen  Pointer to the size of the @p optval buffer, in bytes. 
 *
 * @retval  0  successful
 * @retval  -1   An error occurred.  The error code can be retrieved
 *          by calling getErrno().  Errno can be one of the following:
 *
 *              @li <b>ENOTSOCK</b>        Not a valid socket.
 *              @li <b>EINVAL</b>          Level is invalid or unknown.
 *              @li <b>EFAULT</b>          optlen or optval is not valid.
 *              @li <b>ENOPROTOOPT</b>     The option is unknown or unsupported by the indicated protocol family.
 */
int getsockopt(int sid, int level, int optname, char *optval, int *optlen)
{
    ST_SOCKET *socket;
	
    SOCKET_DEBUGF(0, ("getsockopt: s=%d", sid));
    if ((sid < 0) || (sid >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }
	
	if (optval == NULL)
	{
		setErrno(EINVAL);
		return SOCKET_ERROR;
	}
	
	if (optlen == (int *)NULL)
	{
		setErrno(EFAULT);
		return SOCKET_ERROR;
	}
	
	socket = (ST_SOCKET *)&SockBank[sid];
	
	if ((level != (int)SOL_SOCKET) && (level != IPPROTO_TCP) &&
	    (level != IPPROTO_IP) )
	{
		setErrno(ENOPROTOOPT);
		return SOCKET_ERROR;
	}
	
	if ((socket->u16SockType != SOCK_STREAM) && (level == IPPROTO_TCP))
	{
		setErrno(ENOPROTOOPT);
		return SOCKET_ERROR;
	}
	
	/* check if buffer length is enough */

	switch (optname)
    {
        case SO_ERROR:
            if (*optlen < sizeof(BOOL))
            {
                setErrno(EFAULT);
                return SOCKET_ERROR;
            }
            *optlen = sizeof(BOOL);
            break;
        default:
            if (*optlen < sizeof(int))
            {
                setErrno(EFAULT);
                return SOCKET_ERROR;
            }
            *optlen = sizeof(int);
            break;
    }
	
   if (level == (int)SOL_SOCKET)
   {
	   switch (optname)
	   {
	   case SO_ERROR:
		   *((int *)optval) = socket->u16SockError;
		   socket->u16SockError = 0;                /* clear error */
		   break;
		   
	   case SO_BROADCAST:
		   *((int *)optval) = true;             /* TODO PPP ? */
		   break;
		   
	   case SO_REUSEADDR:
		   if (socket->u16SockOptions & SO_REUSEADDR)
               *((int *)optval) = 1;
           else
               *((int *)optval) = 0;
		   break;
		   
	   default:
		   setErrno(ENOPROTOOPT);
		   return SOCKET_ERROR;
		}
	}


	return 0;
}

/**
 * @ingroup NET_SOCKET
 * @brief Set a socket option.
 *
 * @param level  Level at which the option is defined.
 * @param optname  Socket option for which the value is to be set;  the supported
 *                 options include SO_LINGER, SO_BROADCAST, SO_REUSEADDR.
 * @param optval  Pointer to the buffer in which the value for the requested 
 *                option is specified.
 * @param optlen  Size of the @p optval buffer, in bytes.
 *
 * @retval  0  successful
 * @retval  -1   An error occurred.  The error code can be retrieved
 *          by calling getErrno().  Errno can be one of the following:
 *
 *              @li <b>ENOTSOCK</b>        Not a valid socket.
 *              @li <b>EINVAL</b>          Level is not valid, or the information in optval is not valid.
 *              @li <b>ENOPROTOOPT</b>     The option is unknown or unsupported for the specified socket .
 */
int setsockopt(int sid, int level, int optname, const char *optval, int optlen)
{
    ST_SOCKET *socket;
    long 	  OptVal = 0;
    int        i;

    SOCKET_DEBUGF(0, ("setsockopt: s=%d,o=%d", sid, optname));
    if ((sid <= 0) || (sid >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
        return SOCKET_ERROR;
    }

    if (optval == NULL)
    {
        setErrno(EINVAL);
        return SOCKET_ERROR;
    }

	socket = (ST_SOCKET *)&SockBank[sid];

    if ((level != (int)SOL_SOCKET) && (level != IPPROTO_TCP) &&
        (level != IPPROTO_IP))
    {
        setErrno(ENOPROTOOPT);
        return SOCKET_ERROR;
    }

    if ((socket->u16SockType != SOCK_STREAM) && (level == IPPROTO_TCP))
    {
        setErrno(ENOPROTOOPT);
        return SOCKET_ERROR;
    }

    if (level == SOL_SOCKET)
    {
        switch (optname) 
        {
            case SO_BROADCAST:  /* allow transmission of broadcast packets on the socket */
                if (optlen != sizeof(long))
                {
                    setErrno(EINVAL);
                    return SOCKET_ERROR;
                }
                OptVal |= (int)optval[0];
                for(i=1;i<optlen;i++)
                {
                    OptVal <<= 8;
                    OptVal |= (int)optval[i];				 	
                }

                if (socket->u16SockType != SOCK_DGRAM) /* not UDP */
                {
                    setErrno(ENOPROTOOPT);
                    return SOCKET_ERROR;
                }
                if (OptVal == 0) /* turn off BROADCAST */
                    socket->u16SockOptions &= (~SO_BROADCAST);
                else	/* turn on BROADCAST */
                    socket->u16SockOptions |= SO_BROADCAST;       
                break;
            case SO_LINGER:     /* linger on close if unsent data is present */
                if (socket->u16SockType != SOCK_STREAM) /* not TCP */
                {
                    setErrno(ENOPROTOOPT);
                    return SOCKET_ERROR;
                }
                if (((struct linger *)optval)->l_onoff == 0) /* turn off linger */
                    socket->u16SockOptions &= ~SO_LINGER;
                else /* turn on linger and set linger value */
                {
                    socket->u16SockOptions |= SO_LINGER;
                    socket->u32SockLingerTime = ((struct linger *)optval)->l_linger;
                }
                break;

            case SO_REUSEADDR:     /* allow local address reuse */
                OptVal |= (int)optval[0];
                for(i=1;i<optlen;i++)
                {
                    OptVal <<= 8;
                    OptVal |= (int)optval[i];				 	
                }
                if (OptVal == 0)
                    socket->u16SockOptions &= (~SO_REUSEADDR);
                else
                    socket->u16SockOptions |= SO_REUSEADDR;       
                break;

            default:
                setErrno(ENOPROTOOPT);
                return SOCKET_ERROR;
        }
    }
    else if (level == IPPROTO_TCP)
    {
                                                /* TODO */
        setErrno(ENOPROTOOPT);
        return SOCKET_ERROR;
    }
    else if (level == IPPROTO_IP)
    {
                                                /* TODO */
        setErrno(ENOPROTOOPT);
        return SOCKET_ERROR;
    }

    return 0;
}

/**
 * @ingroup NET_SOCKET
 * Control the I/O mode of a socket
 *
 * Commands supported include:
 *
 *  @li <b>FIONBIO</b>             <br>The <b>argp</b> parameter is a pointer to an unsigned long value.
 *                      Set <b>argp</b> to a nonzero value if the nonblocking mode should 
 *                      be enabled, or zero if the nonblocking mode should be 
 *                      disabled. When a socket is created, it operates 
 *                      in blocking mode by default (nonblocking mode is disabled).
 *  @li <b>SIOCGIFADDR</b>         <br>Get PA address (IP address).  The <b>argp</b> parameter is
 *                      a pointer to a ifreq struct.
 *  @li <b>SIOCGIFHWADDR</b>       <br>Get hardware address.  The <b>argp</b> parameter is
 *                      a pointer to a ifreq struct.
 *
 * @param sid  Descriptor identifying a socket 
 * @param cmd  Command to perform on the socket.
 * @param argp  Pointer to a parameter for @p cmd.
 *
 * @retval  0  successful
 * @retval  -1   An error occurred.  The error code can be retrieved
 *          by calling getErrno().  Errno can be one of the following:
 *              @li <b>ENOTSOCK</b>        Not a valid socket.
 *              @li <b>EINVAL</b>          Command is unknown or invalid.
 */
int ioctlsocket(int sid, long cmd, unsigned long *argp)
{
    ST_SOCKET *socket;
	
    SOCKET_DEBUGF(0, ("ioctlsocket: s=%d,o=%d", sid, cmd));
    if ((sid < 0) || (sid >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
        return SOCKET_ERROR;
    }

   socket = &SockBank[sid];

   switch (cmd)
   {
       case FIONBIO:
           if ((unsigned long)*argp == 0)       /* set to blocking */
               socket->u16SockState &= ~SS_NBIO;
           else                                 /* set to non-blocking */
               socket->u16SockState |= SS_NBIO;
           break;

       case FIONREAD:
           *argp = NetBufferDataLengthGet(sid);
           break;

       case SIOCGIFADDR:        /* get PA (or IP) address */
           {
               struct ifreq *ifr = (struct ifreq *)argp;
               struct sockaddr_in *sin = (struct sockaddr_in *)&ifr->ifr_addr;

               memset(sin, 0, sizeof(*sin));
               sin->sin_addr.s_addr = htonl(NetDefaultIpGet());
               sin->sin_family = AF_INET;
           }
           break;

       case SIOCGIFHWADDR:
           {
               struct ifreq *ifr = (struct ifreq *)argp;
               struct sockaddr *sa = (struct sockaddr *)&ifr->ifr_addr;

               memset(sa, 0, sizeof(*sa));
               sa->sa_family = AF_PACKET;
			   
               memcpy(sa->sa_data,myethaddr, ETH_ALEN);	   
           }
           break;

       default:
           setErrno(EINVAL);
           return SOCKET_ERROR;
   }

   return 0;
}

void sockErrorSet(int sid, int error)
{
    ST_SOCKET *socket;
    
#ifdef HAVE_LWIP_SOCKET
    if (sid > 0x100)
        return sockErrorSet2(sid, error);
#endif

    socket = &SockBank[sid];
    socket->u16SockError = error;
    //DPrintf("sockErrorSet: sockerror=%d", error);
}

int sockIsConnected(int sid)
{
    ST_SOCKET *socket;
    
    if ((sid <= 0) || (sid >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }
    socket = &SockBank[sid];
    socket->u16SockState &= ~(SS_ISCONNECTING|SS_ISDISCONNECTING|SS_ISCONFIRMING);
    socket->u16SockState |= SS_ISCONNECTED;
    DPrintf("sockIsConnected(%d)", sid);
    SockSignalTxReady(sid);
    return 0;
}

int sockIsDisconnected(int sid)
{
    ST_SOCKET *socket;

#ifdef HAVE_LWIP_SOCKET
    if (sid > 0x100)
        return sockIsDisconnected2(sid);
#endif

    if ((sid <= 0) || (sid >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }
    
    socket = &SockBank[sid];

    if (!(socket->u16SockState & (SS_ISCONNECTED|SS_ISCONNECTING)))
        return 0;

    socket->u16SockState &= ~(SS_ISCONNECTING|SS_ISCONNECTED|SS_ISDISCONNECTING);
//    socket->u16SockState |= SS_CANTRCVMORE|SS_CANTSENDMORE; XXX
    DPrintf("sockIsDisconnected(%d)",sid);
    SockIdSignalRxDone(sid, NULL);
    return 0;
}

int sockIsDisconnecting(int sid)
{
    ST_SOCKET *socket;
    
    if ((sid <= 0) || (sid >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }
    socket = &SockBank[sid];
    socket->u16SockState &= ~(SS_ISCONNECTING);
    socket->u16SockState |= SS_ISDISCONNECTING;
    DPrintf("sockIsDisconnecting(%d)", sid);
    return 0;
}

int sock_Disconnect(int sid)
{
    ST_SOCKET *socket;
    if ((sid <= 0) || (sid >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }
    
    DPrintf("sock_Disconnect(%d)",sid);
    socket = &SockBank[sid];
    if (socket->u16SockState & SS_ISDISCONNECTING)
        return 0;
    if (!(socket->u16SockState & SS_ISCONNECTED))
        return 0;

    if(socket->pcb.tcp == NULL)
        return 0;
    if(NO_ERR != TcpDisconnect(socket->pcb.tcp))
    {
        DPrintf("[SOCKET] tcp abort");
        TcpAbort(socket->pcb.tcp);
    }
    socket->u16SockState &= ~(SS_ISCONNECTING);
    socket->u16SockState |= SS_ISDISCONNECTING;
    return 0;
}

/* 
 *  sock_Detach -
 *  
 *  Detach from the lower layer protocol
 * =====================================================================================
 */
int sock_Detach(int sid)
{
    ST_SOCKET *socket;
#ifdef HAVE_LWIP_SOCKET
    if (sid > 0x100)
        return sock_Detach2(sid);
#endif
    if ((sid <= 0) || (sid >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }
    
    socket = &SockBank[sid];
    socket->pcb.tcp = NULL;
    return 0;
}

/* 
 *  sockIsCantRecvMore -
 *  
 *  Is socket not able to receive any more packets ?
 * =====================================================================================
 */
BOOL sockIsCantRecvMore(int sid)
{
    ST_SOCKET *socket;
#ifdef HAVE_LWIP_SOCKET
    if (sid > 0x100)
        return sockIsCantRecvMore2(sid);
#endif
    if ((sid <= 0) || (sid >= SOCK_MAXIMAL_NUMBER))
		return TRUE;
    
    socket = &SockBank[sid];
    return (socket->u16SockState & SS_CANTRCVMORE) ? TRUE : FALSE;
}


/* 
 *  sockCantRecvMore -
 *  
 *  Can't receive any more packets
 * =====================================================================================
 */
int sockCantRecvMore(int sid)
{
    ST_SOCKET *socket;
    if ((sid <= 0) || (sid >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }
    
    socket = &SockBank[sid];
    socket->u16SockState |= SS_CANTRCVMORE;
    DPrintf("sockCantRecvMore");
    return 0;
}

/* 
 *  sockCantSendMore -
 *  
 *  Can't send any more packets
 * =====================================================================================
 */
void sockCantSendMore(int sid)
{
    ST_SOCKET *socket;
    
    socket = &SockBank[sid];
    socket->u16SockState |= SS_CANTSENDMORE;
    DPrintf("sockCantSendMore");
}

void setErrno(int error)
{
    errno = error;
}

int getErrno(void)
{
    return errno;
}

int Conn(int sid)
{
    ST_SOCKET *socket;
    
#ifdef HAVE_LWIP_SOCKET
    if (sid > 0x100)
        return Conn2(sid);
#endif
    socket = &SockBank[sid];
    if (socket->u16SockState & SS_ISCONNECTED)
        return TRUE;
    else 
        return FALSE;
}

int sockIsNbio(int sid)
{
    ST_SOCKET *socket;
    
    socket = &SockBank[sid];
    if ( socket->u16SockState &  SS_NBIO )  /* non-blocking socket */
        return TRUE;
    else 
        return FALSE;
}

int mpx_DoConnect(U32 u32DstAddr, U16 u16DstPort, BOOL isBlocking)
{
    U16 sockId = 0;
    int error = NO_ERR;
    struct sockaddr_in peerAddr, localAddr;
    U32 ipaddr;
    
    error = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(error < 0 )
    {
        DPrintf("[FTP] open socket fail %d", getErrno());
        goto ERROR;
    }
    
    sockId = (U16)error;
    
//    mpx_SockAddrSet(&localAddr, NetDefaultIpGet(), mpx_NewLocalPort());
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = 0;
    ipaddr = NetDefaultIpGet();
    localAddr.sin_addr.s_addr = htonl(ipaddr);
    error = bind(sockId, (struct sockaddr *)&localAddr, sizeof(localAddr));
    if(error < 0 )
    {
        DPrintf("[FTP] bind socket fail");
        goto ERROR;
    }
    
//    mpx_SockAddrSet(&peerAddr, u32DstAddr, u16DstPort);
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = u16DstPort;
    peerAddr.sin_addr.s_addr = u32DstAddr;

    /*
    DPrintf("[FTP] connecting to \\-");
    NetDebugPrintIP(u32DstAddr);
    */
    
    unsigned long val = 1;
    if (isBlocking)
        val = 0;
    ioctlsocket(sockId, FIONBIO, &val);

    error = connect(sockId, (struct sockaddr *)&peerAddr, sizeof(peerAddr));
    if(error == 0)
    {
        //DPrintf("[FTP] connected");
        return sockId;
    }
    else if (getErrno() == EWOULDBLOCK)
        return sockId;
    
ERROR:
    if (sockId > 0)
        closesocket(sockId);
    return error;
}

ST_UNIX_PCB* UnixNew()
{
    ST_UNIX_PCB*ret= mpx_Malloc(sizeof(ST_UNIX_PCB));
    if (ret)
    memset(ret, 0, sizeof(*ret));
    return ret;
}

void UnixFree(ST_UNIX_PCB* pcb)
{
    mpx_Free(pcb);
}

int UnixBind(ST_UNIX_PCB* pcb, struct sockaddr_un *addr)
{
    int len;

    if (addr->sun_family != AF_UNIX)
        return SOCKET_ERROR;

    len = strlen(addr->sun_path);
    if (len >= UNIX_PATH_MAX)
        return SOCKET_ERROR;

    strncpy(pcb->u08LocalPath, addr->sun_path, len+1);
    
    return 0;
}

static int UnixSocketFind(char *path)
{
    int len, len2, sid;
    ST_UNIX_PCB *ucb;

    len = strlen(path);
    if (len == 0)
        return 0;
    for (sid=1; sid<SOCK_MAXIMAL_NUMBER; sid++)
    {
        if(SockBank[sid].u08Type != SOCK_TYPE_ALLOCATED ||
           SockBank[sid].u16Family != AF_LOCAL)
            continue;

        ucb = SockBank[sid].pcb.unix;
        len2 = strlen(ucb->u08LocalPath);
        if (len == len2 && !strncmp(path, ucb->u08LocalPath, len))
            break;
    }

    if (sid < SOCK_MAXIMAL_NUMBER)
        return sid;
    else
        return 0;
}

int UnixConnect(ST_UNIX_PCB* pcb, struct sockaddr_un *addr)
{
    int len;
    int peerSid;

    if (addr->sun_family != AF_UNIX)
        return SOCKET_ERROR;

    len = strlen(addr->sun_path);
    if (len >= UNIX_PATH_MAX)
        return SOCKET_ERROR;

    if ((peerSid=UnixSocketFind(addr->sun_path)) == 0)
    {
        setErrno(ECONNREFUSED);
        return SOCKET_ERROR;
    }
    
    strncpy(pcb->u08PeerPath, addr->sun_path, len+1);

    return 0;
}

int UnixWrite(ST_UNIX_PCB* pcb, U08* data, S32 size, struct sockaddr_un *to)
{
    ST_UNIX_PACKET *packet;
    int peerSid = 0;
    int err;

    if (!to)
    {
        if (strlen(pcb->u08PeerPath) == 0)
            err = EDESTADDRREQ;
        else
        {
            err = ENOENT;
            peerSid = UnixSocketFind(pcb->u08PeerPath);
        }
    }
    else
    {
        err = ENOENT;
        peerSid = UnixSocketFind(to->sun_path);
    }

    if (peerSid == 0)
    {
        setErrno(err);
        return SOCKET_ERROR;
    }
    
//    packet = NetNewPacketWithSize(size, FALSE);
    void *p = ext_mem_malloc(size+sizeof(packet->from)+sizeof(packet->u16DataLen));
    packet = (ST_UNIX_PACKET *)p;
    if(!packet)
    {
        DPrintf("[UnixWrite] allocate buffer fail");
        return 0;
    }
	    mmcp_memcpy(packet->u08Data, data, size);
    packet->u16DataLen = size;
    packet->from.sun_family = AF_LOCAL;
    strcpy(packet->from.sun_path, pcb->u08LocalPath);

//    SockIdSignalRxDone(peerSid, packet);
    SockIdSignalRxDone2(peerSid, p);
    return size;
}

void *IpProtocolPacketReceive(U32 srcIp, U32 dstIp, U08 proto, void *packet)
{
    register U32 index;
    register U32 mask;

    // find the target soc
    index = 1;                               // actually, the socket number begins at one
    while(index < SOCK_MAXIMAL_NUMBER)
    {
        if(SockBank[index].u08Type == SOCK_TYPE_ALLOCATED &&
           SockBank[index].u16Family == AF_INET &&
           SockBank[index].u16SockType == SOCK_RAW)
        {
            if(((SockBank[index].u16Protocol == 0) || (SockBank[index].u16Protocol == proto)) &&
               ( ((((ST_SOCK_ADDR_IN *)&SockBank[index].Peer.u08Address)->u32Addr == srcIp) ||
                (((ST_SOCK_ADDR_IN *)&SockBank[index].Peer.u08Address)->u32Addr == 0)) &&
                ((dstIp == NetBroadcaseIpGet()) ||
                (((ST_SOCK_ADDR_IN *)&SockBank[index].Local.u08Address)->u32Addr == dstIp))))
                    break;
        }
        index ++;
    }

    if(index < SOCK_MAXIMAL_NUMBER)
    {
        mask = 1UL << index;

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
        return NULL;
    }
    else
        return packet;
}

/**
 * Retrieves the local name for a socket
 */
int getsockname(int s, struct sockaddr *name, socklen_t *addrlen)
{
    struct sockaddr *nm = (struct sockaddr *)&SockBank[s].Local;

    if((s >= SOCK_MAXIMAL_NUMBER) || (s == 0))
    {
        setErrno(ENOTSOCK);
        return -1;
    }

    memset(name, 0, *addrlen);
	memcpy(name, nm, 8);
	*addrlen = 8;

	return 0;
}

int getpeername(int s, struct sockaddr *sa, socklen_t *len)
{
    ST_SOCKET *socket = &SockBank[s];
    if (socket->u16Protocol == IPPROTO_TCP)
    {
        if (socket->u16SockState & SS_ISCONNECTED)
        {
            memcpy(sa, &socket->Peer, sizeof(*sa));
            *len = sizeof(*sa);
            return 0;
        }
        else
            errno = ENOTCONN;
    }
    else if (socket->u16Protocol == IPPROTO_UDP)
    {
        MP_ASSERT(0);                           /* TODO */
    }
    return -1;
}

void *gettcppcb(int s)
{
    register ST_SOCKET *socket;
    socket = &SockBank[s];
    return socket->pcb.tcp;
}
