
#ifndef __NET_SOCKET_H
#define __NET_SOCKET_H



#include "typedef.h"
#include "net_tcp.h"
#include "net_udp.h"
#include "net_unix.h"

// address family or protocol domain
#define AF_UNSPEC           0
#define AF_UNIX		1	/* Unix domain sockets 		*/
#define AF_LOCAL	1	/* POSIX name for AF_UNIX	*/
#define AF_INET             2
#define AF_INET6            10
#define	AF_NETLINK	16
#define AF_PACKET	17	/* Packet family		*/

/* Protocol families, same as address families. */
#define PF_UNIX		AF_UNIX		/* Unix domain sockets 		*/
#define PF_LOCAL	AF_LOCAL	/* POSIX name for AF_UNIX	*/
#define PF_INET		AF_INET
#define	PF_NETLINK	AF_NETLINK
#define PF_PACKET	AF_PACKET

// socket type definition
#define SOCK_STREAM         1
#define SOCK_DGRAM          2
#define SOCK_RAW            3

/* Maximum queue length specifiable by listen.  */
#define SOMAXCONN	128

/* Flags we can use with send/ and recv. 
   Added those for 1003.1g not all are supported yet
 */
 
#define MSG_DONTWAIT	0x40	/* Nonblocking io		 */

// Error code definition
#define ERR_WRONG_AF                -101    // the specified address family is invalid
#define ERR_NO_FD                   -102    // no socket file descriptor can be allocated
#define ERR_WRONG_FD                -103    // the socket argument is a invalid file descriptor
#define ERR_WRONG_TYPE              -104    // not supported specifed socket type
#define ERR_NO_MEMORY               -105    // out of system memory
#define ERR_ADDR_INUSE              -106    // the specified socket address is already in use.
#define ERR_ADDR_NOT_AVAIL          -107    // the specified address is not available from the local machine.
#define ERR_SOCK_BOUND              -108    // the socket is already bound to an address.
#define ERR_NOT_SOCK                -109    // the socket argument does not refer to a socket.
#define ERR_AF_NOT_SUPPORT          -110    // address in the specified address family can not be used with this socket
#define ERR_INVALID_SOCKET          -111    // socket is not allocated.
#define ERR_ERROR_STATE             -112    // socket operation on wrong state.
#define ERR_BUFFER_NOT_ENOUGH       -113
#define ERR_SEND_FAIL               -114


// socket address define for IPV4
typedef struct
{
    //U16 u16Family;
    //U16 u16Port;
    U32 u32Addr;
} ST_SOCK_ADDR_IN;


// socket address define for IPV6
typedef struct
{
    //U16 u16Family;
    //U16 u16Port;
    U32 u32FlowInfo;
    U64 u64Addr;
    U32 u32ScopeId;
} ST_SOCK_ADDR_IN6;


#define SOCK_ADDR_MAX_SIZE  sizeof(ST_SOCK_ADDR_IN6)

typedef struct
{
    U16 u16Family;
    U16 u16Port;
    U08 u08Address[SOCK_ADDR_MAX_SIZE];
} ST_SOCK_ADDR;


// address information request number
#define AI_PASSIVE          1   // Socket address is intended for bind().
#define AI_CANONNAME        2   // Request for canonical name.
#define AI_NUMERICHOST      4   // Return numeric host address as name.
#define AI_NUMERICSERV      8   // Inhibit service name resolution.
#define AI_V4MAPPED         16  // If no IPv6 addresses are found, query for IPv4 addresses and 
                                // return them to the caller as IPv4-mapped IPv6 addresses. 
#define AI_ALL              32  // Query for both IPv4 and IPv6 addresses. 
#define AI_ADDRCONFIG       64  // Query for IPv4 addresses only when an IPv4 address is configured; 
                                // query for IPv6 addresses only when an IPv6 address is configured. 

typedef struct _addrinfo_
{
    U16             u16Flag;                        // input flag
    U16             u16Family;
    U16             u16SockType;
    U16             u16Protocol;
    ST_SOCK_ADDR    SockAddr;
    struct _addrinfo_   *Next;
} ST_ADDR_INFO;



#define SOCK_MAXIMAL_NUMBER         32
#define SOCK_PORT_EPHEMERAL_START   4096
#define SOCK_PORT_EPHEMERAL_END     0x7FFF

#define SOCK_TYPE_FREE              0
#define SOCK_TYPE_ALLOCATED         1


#define FLAG_DONT_WAIT              0x40

struct netlink_sock;
typedef struct netlink_sock ST_NETLINK_PCB;

struct packet_sock;
typedef struct packet_sock ST_PACKET_PCB;

typedef struct
{
    U08             u08Type;            // SOCK_TYPE_FREE or SOCK_TYPE_ALLOCATED
    U08             u08NetIndex;
    U08             u08Reserved[2];
    U16             u16Family;
    U16             u16SockType;        // SOCK_STREAM or SOCK_DGRAM or SOCK_RAW
    U16             u16Protocol;
    U16             u16Reserved2;
    
    ST_SOCK_ADDR    Local;
    ST_SOCK_ADDR    Peer;
    
    union
    {
        ST_TCP_PCB* tcp;
        ST_UDP_PCB* udp;
        ST_UNIX_PCB* unix;
        ST_NETLINK_PCB *netlink;
        ST_PACKET_PCB *packet;      /* PF_PACKET */
    }               pcb;
    
    U32             u32Signal;          // designated the current network operation about this socket
    void            *inPoint;             // operation object or packet point
    void            *outPoint;
    U16             u16SockState;		/* socket state */
    U16             u16SockError;		/* error affecting connection */
    U16             u16SockOptions;		/* socket options */
    U32             u32SockLingerTime;		/* socket linger time (in seconds) */
} ST_SOCKET;


/**
 * @ingroup NET_SOCKET
 * @brief Similar to the fd_set structure, but from the old days.
 * 
 * @note ST_SOCK_SET is 4 bytes long, but fd_set is 8 bytes only.  Since
 * only 31 sockets are supported currently, so it doesn't cause any problems.
*/
typedef struct
{
    U32 u32Mask;
} ST_SOCK_SET;

#define MPX_FD_SET(n, p)    ((p)->u32Mask |= (1 << (n)))
#define MPX_FD_CLR(n, p)    ((p)->u32Mask &= ~(1 << (n)))
#define MPX_FD_ISSET(n, p)  ((p)->u32Mask & (1 << (n)))
#define MPX_FD_ZERO(p)      ((p)->u32Mask = 0)


S32 mpx_NetworkInit();



///
///@brief   The mpx_Socket() function shall create an unbound socket in a communications domain, 
///         and return a socket ID that can be used in later function calls that operate on sockets.
///
///@param   domain      Specifies the communications domain in which a socket is to be created. 
///@param   type        Specifies the type of socket to be created.
///
///@retval  >0              Function complete successfully, the return code represent the socket ID.
///@retval  ERR_WRONG_AF    The specified communication domain(address family) is not supported. 
///@retval  ERR_NO_FD       No more file descriptors are available for new socket. 
///@retval  ERR_WRONG_TYPE  The socket type is not supported. 
///@retval  ERR_NO_MEMORY   Insufficient memory was available to fulfill the request. 
///
///@remark  The domain argument specifies the address family used in the communications domain. 
///         The address families supported by the system are implementation-defined.
///@remark  AF_INET     Internet domain sockets for use with IPv4 addresses.
///@remark  AF_INET6    Internet domain sockets for use with IPv6 addresses. Not supported at the current stage.
///@remark  AF_UNSPEC   Not specified. Use the default value, AF_INET.
///@remark  Currently only AF_INET is supported.
///@remark  
///@remark  The type argument specifies the socket type, which determines the semantics of communication over 
///         the socket.
///@remark  SOCK_STREAM     Provides sequenced, reliable, bidirectional, connection-mode byte streams, and may
///         provide a transmission mechanism for out-of-band data. Not Supported at the current stage.
///@remark  SOCK_DGRAM      Provides datagrams, which are connectionless-mode, unreliable messages of fixed 
///         maximum length. 
///
S32 mpx_Socket(U16 domain, U16 type);

void mpx_SockAddrSet(ST_SOCK_ADDR *, U32, U16);

S32 mpx_Accept(U16 sid, ST_SOCK_ADDR* addr);
S32 mpx_Bind(U16 sid, ST_SOCK_ADDR *addr);
S32 mpx_GetAddrInfo(S08 *nodeName, S08 *portName, ST_ADDR_INFO *hints, ST_ADDR_INFO *result);
S32 mpx_GetNameInfo();
S32 mpx_Connect(U16 sockId, ST_SOCK_ADDR *destAddr);
S32 mpx_Listen(U16 sockId, U16 backlog);
S32 mpx_Recv(U16 sid, U08* buffer, U32 size);
S32 mpx_SockRead(U16 fd, U08 *buffer, U32 size);
S32 mpx_RecvFrom(U16 sid, void *message, U32 length, S32 flag, ST_SOCK_ADDR *from);
S32 mpx_Send(U16 sid, U08 *buffer, S32 size);
S32 mpx_SendTo(U16, void *, S32, S32, ST_SOCK_ADDR *);
S32 mpx_SockWrite(U16 sid, U08 *buffer, S32 size);
S32 mpx_SocketClose(U16);
S32 mpx_Select(ST_SOCK_SET *read, ST_SOCK_SET *write, ST_SOCK_SET *excep, U32 tick);

U16 mpx_NewLocalPort();
U16 mpx_SockType(U16);
U32 mpx_SockLocalAddrGet(U16 sid);
U32 mpx_SockPeerAddrGet(U16 sid);
U16 mpx_SockLocalPortGet(U16 sid);
U16 mpx_SockPeerPortGet(U16 sid);

#ifdef NEW_NETSOCKET
#include "net_socket2.h"
#endif

#endif      // __NET_SOCKET_H

