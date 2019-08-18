
#ifndef __NET_SOCKET2_H
#define __NET_SOCKET2_H


#include <sys/time.h>
#include "typedef.h"
#include "net_tcp.h"
#include "net_udp.h"

/*
 * Protocols
 */
#define IPPROTO_IP              0               /* dummy for IP */
#define IPPROTO_ICMP            1               /* control message protocol */
#define IPPROTO_IGMP            2               /* internet group management protocol */
#define IPPROTO_GGP             3               /* gateway^2 (deprecated) */
#define IPPROTO_TCP             6               /* tcp */
#define IPPROTO_PUP             12              /* pup */
#define IPPROTO_UDP             17              /* user datagram protocol */
#define IPPROTO_IDP             22              /* xns idp */
#define IPPROTO_ND              77              /* UNOFFICIAL net disk proto */

#define IPPROTO_RAW             255             /* raw IP packet */
#define IPPROTO_MAX             256

/*
 * Socket state bits.
 */
#define	SS_NOFDREF		0x001	/* no file table ref any more */
#define	SS_ISCONNECTED		0x002	/* socket connected to a peer */
#define	SS_ISCONNECTING		0x004	/* in process of connecting to peer */
#define	SS_ISDISCONNECTING	0x008	/* in process of disconnecting */
#define	SS_CANTSENDMORE		0x010	/* can't send more data to peer */
#define	SS_CANTRCVMORE		0x020	/* can't receive more data from peer */
#define	SS_RCVATMARK		0x040	/* at mark on input */

#define	SS_NBIO			0x080	/* non-blocking ops */
#define	SS_ASYNC		0x100	/* async i/o notify */
#define	SS_ISCONFIRMING		0x200	/* deciding to accept connection req */
#define	SS_MORETOCOME		0x400	/*
					 * hint from sosend to lower layer;
					 * more data coming
					 */
#define 	SS_ISAPIPE 		0x800 /* socket is implementing a pipe */
#define 	SS_ISCLONE 		0x4000 /* socket is a clone */
#define 	SS_HASCLONE 	0x8000 /* socket has a clone */

/*
 * Option flags per-socket.
 */
#define	SO_DEBUG	0x0001		/* turn on debugging info recording */
#define	SO_ACCEPTCONN	0x0002		/* socket has had listen() */
#define	SO_REUSEADDR	0x0004		/* allow local address reuse */
#define	SO_KEEPALIVE	0x0008		/* keep connections alive */
#define	SO_DONTROUTE	0x0010		/* just use interface addresses */
#define	SO_BROADCAST	0x0020		/* permit sending of broadcast msgs */
#define	SO_USELOOPBACK	0x0040		/* bypass hardware when possible */
#define	SO_LINGER	0x0080		/* linger on close if data present */
#define	SO_OOBINLINE	0x0100		/* leave received OOB data in line */
#define	SO_REUSEPORT	0x0200		/* allow local address & port reuse */

/*
 * Additional options.
 */
#define SO_SNDBUF       0x1001          /* send buffer size */
#define SO_RCVBUF       0x1002          /* receive buffer size */
#define SO_SNDLOWAT     0x1003          /* send low-water mark */
#define SO_RCVLOWAT     0x1004          /* receive low-water mark */
#define SO_SNDTIMEO     0x1005          /* send timeout */
#define SO_RCVTIMEO     0x1006          /* receive timeout */
#define SO_ERROR        0x1007          /* get error status and clear */
#define SO_TYPE         0x1008          /* get socket type */

struct sockaddr;

#ifndef __sockaddr_ll_defined
// link level (layer 2) socket address
struct sockaddr_ll {
  U16             sll_family;    /* Always AF_PACKET */
  U16             sll_protocol;  /* link level protocol */
  S32             sll_ifindex;   /* Interface number */
  unsigned short  sll_hatype;    /* Header type */
  unsigned char   sll_pkttype;   /* Packet type */
  unsigned char   sll_halen;     /* Length of address */ 
  unsigned char   sll_addr[8];
};
#define __sockaddr_ll_defined
#endif

typedef struct sockaddr_ll ST_SOCK_ADDR_LL;

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define SOL_SOCKET      0xffff          /* options for socket level */

#define	SOCKET_ERROR		 (-1)




#define FIONREAD    127 /* _IOR('f', 127, u_long) */ /* get # bytes to read */
#define FIONBIO     126 /* _IOW('f', 126, u_long) */ /* set/clear non-blocking i/o */

/* Socket configuration controls. */
#define SIOCGIFNAME	0x8910		/* get iface name		*/
#define SIOCGIFFLAGS	0x8913		/* get flags			*/
#define SIOCSIFFLAGS	0x8914		/* set flags			*/
#define SIOCGIFADDR	0x8915		/* get PA address		*/
#define SIOCGIFHWADDR	0x8927		/* Get hardware address		*/
#define SIOCADDMULTI	0x8931		/* Multicast address lists	*/
#define SIOCDELMULTI	0x8932
#define SIOCGIFINDEX	0x8933		/* name -> if_index mapping	*/

/* Device private ioctl calls */
#define SIOCDEVPRIVATE	0x89F0	/* to 89FF */

typedef int socklen_t;
#define CONFIG_NO_SOCKLEN_T_TYPEDEF

/*
 * @ingroup SOCKET
 * @brief Create a socket.
 *
 * @param domain  Address family.  The following are supported:
 *              AF_INET
 *              AF_PACKET
 *              AF_UNIX                         - UNIX domain socket
 * @param type  Type for the new socket.  The following types are supported:
 *              SOCK_STREAM
 *              SOCK_DGRAM
 *              SOCK_RAW
 * @param protocol Protocol to be used with the socket.
 *        0 Indicates that the default protocol for the <type> selected is to be used.
 *        IPPROTO_TCP
 *        IPPROTO_UDP
 *
 * @retval  >0  ID of the new socket.
 * @retval  SOCKET_ERROR   An error occurred.  The error code can be retrieved
 *          by calling getErrno().  Errno can be one of the following:
 *
 *              EAFNOSUPPORT        The specified address family is not supported.
 *              EPROTONOSUPPORT     The specified protocol is not supported.
 *              EPROTOTYPE          The specified protocol is the wrong type for this socket.
 *              ENOBUFS             No buffer space is available. The socket cannot be created.
*/
int socket(int domain, int type, int protocol);

/*
 * @ingroup SOCKET
 * @brief Determine the status of one or more sockets.
 * 
 * @param nfds  Ignored; included only for compatibility with Berkeley sockets 
 * @param read  A set of sockets to be checked for readability. Optional parameter.
 * @param write A set of sockets to be checked for writability. Optional parameter.
 * @param excep A set of sockets to be checked for errors. Optional parameter.
 * @param timeout  Maximum wait time.  Set the <timeout> parameter to NULL for 
 *              blocking operations.  If time of the <timeout> parameter is set 
 *              to zero ({0,0}), then select will return immediately.
 * 
 * @retval  >0  The number of the sockets that meet the conditions.
 * @retval  0   No sockets that meet the conditions.
 * @retval  SOCKET_ERROR   An error occurred.  The error code can be retrieved
 *          by calling getErrno().  Errno can be one of the following:
 *
 *              ENETDOWN        (Internal error in the network subsystem.)
 *              EINVAL          (No socket in all three descriptor set parameters.)
*/
int select(int nfds, ST_SOCK_SET *read, ST_SOCK_SET *write, ST_SOCK_SET *excep, struct timeval *timeout);
int closesocket(int sockId);

/*
 * shutdown:
 *
 * Disable sends or receives on a socket
 * This routine shuts down all, or part, of a connection-based socket <s>.
 * If the value of <how> is 0, receives are disallowed.  If <how> is 1,
 * sends are disallowed.  If <how> is 2, both sends and receives are
 * disallowed.
 *
 * RETURNS: OK, or ERROR if the socket is invalid or not connected.
 */
int shutdown(int sockId, int how);

/*
 * bind:
 *
 * Associate a local address with a socket
 *
 * For TCP/IP, if the port is specified as zero, the bind assigns a unique port
 * to the application with a value between 4096 (SOCK_PORT_EPHEMERAL_START) to 
 * 0x7FFF (SOCK_PORT_EPHEMERAL_END).
 * Return values:
 *   0            = successful
 *   SOCKET_ERROR = failed
 *
 * Error codes (errno): 
 *   ENOTSOCK
 *   EINVAL
 *   EFAULT
 *
 * Note: Always use getErrno() to retrieve the value of errno.
 */
int bind(int sid, const struct sockaddr *name, int namelen);
int accept(int sid, struct sockaddr *addr, int *addrlen);

/*
 * connect:
 *
 * Initiate a connection to a remote socket
 *
 * Return values:
 *   > 0  = new socket id
 *   SOCKET_ERROR  = failed
 *
 */
int connect(int sockId, const struct sockaddr *name, int namelen);

/*
 * @ingroup SOCKET
 * @brief Send data to a specific destination.  If the socket is not bound before, 
 *        sendto() will do an implicit bind.
 * 
 * @param sid  Socket ID 
 * @param buffer  Buffer containing the data to be transmitted.
 * @param size Length of the data in buffer, in bytes.
 * @param flags Indicator specifying the way in which the call is made.
 * @param to  Optional pointer to a sockaddr structure that contains the address of the target socket.
 * @param tolen Size of the address in to, in bytes.
 * 
 * @retval  >=0  The total number of bytes sent.
 * @retval  SOCKET_ERROR   An error occurred.  The error code can be retrieved
 *          by calling getErrno().  Errno can be one of the following:
 *
 *              ENOTSOCK        Not a valid socket.
 *              EMSGSIZE        The socket is message oriented, and the message is larger than the maximum supported by the underlying transport.
 *              EDESTADDRREQ    A destination address is required.
 *              ENOBUFS         No buffer space is available.
 *              EFAULT          Socket type is not supported.
 *
*/
int sendto(int sid, const char *buffer, int size, int flags, const struct sockaddr *to, int tolen);

/*
 * @ingroup SOCKET
 * @brief Send data on a connected socket
 * 
 * @param sid  ID of a connected socket 
 * @param buffer  Buffer containing the data to be transmitted.
 * @param size Length of the data in buffer, in bytes.
 * @param flags Indicator specifying the way in which the call is made.
 * 
 * @retval  >=0  The total number of bytes sent.
 * @retval  SOCKET_ERROR   An error occurred.  The error code can be retrieved
 *          by calling getErrno().  Errno can be one of the following:
 *
 *              ENOTSOCK        Not a valid socket.
 *              EMSGSIZE        The socket is message oriented, and the message is larger than the maximum supported by the underlying transport.
 *              EDESTADDRREQ    A destination address is required.
 *              ENOBUFS         No buffer space is available.
 *              EFAULT          Socket type is not supported.
 *
*/
int send(int sid, const char *buffer, int size, int flags);

/*
 * recvfrom:
 *
 * Receive data from a socket
 *
 * For TCP/IP, if the port is specified as zero, the bind assigns a unique port
 * to the application with a value between 4096 (SOCK_PORT_EPHEMERAL_START) to 
 * 0x7FFF (SOCK_PORT_EPHEMERAL_END).
 * Return values:
 *   > 0          = number of bytes received.
 *   0            = connection is closed gracefully.
 *   SOCKET_ERROR = failed
 *
 * Error codes (use getErrno() to retrive it):
 *   ENOTSOCK
 *   ESHUTDOWN
 *   EWOULDBLOCK
 *   ECONNRESET
 *   ETIMEDOUT     The connection has been dropped.
 */
int recvfrom(int sid, void *message, int length, int flags, struct sockaddr *from, socklen_t *fromlen);

int recv(int sid, void *mem, int len, unsigned int flags);

/*
 * getsockopt
 *
 * Get options associated with the specified socket.  Options supported include:
 *
 *  SO_ERROR
 *
 * Levels supported include:
 *
 *  SOL_SOCKET
 */
int getsockopt(int sno, int level, int optname, char *optval, int *optlen);

/*
 * setsockopt:
 *
 * @ingroup SOCKET
 * @brief Set a socket option.
 *
 * @param level  Level at which the option is defined.
 * @param optname  Socket option for which the value is to be set;  the supported
 *                 options include SO_LINGER, SO_BROADCAST.
 * @param optval  Pointer to the buffer in which the value for the requested 
 *                option is specified.
 * @param optlen  Size of the optval buffer, in bytes.
 */
int setsockopt(int sid, int level, int optname, const char *optval, int optlen);

/*
 * ioctlsocket:
 *
 * Control the I/O mode of a socket
 *
 * Commands supported include:
 *
 *  FIONBIO             The <argp> parameter is a pointer to an unsigned long value.
 *                      Set argp to a nonzero value if the nonblocking mode should 
 *                      be enabled, or zero if the nonblocking mode should be 
 *                      disabled. When a socket is created, it operates 
 *                      in blocking mode by default (nonblocking mode is disabled).
 *  SIOCGIFADDR         Get PA address (IP address).  The <argp> parameter is
 *                      a pointer to a ifreq struct.
 *  SIOCGIFHWADDR       Get hardware address.  The <argp> parameter is
 *                      a pointer to a ifreq struct.
 *
 */
int ioctlsocket(int sid, long cmd, unsigned long *argp);

void sockErrorSet(int sid, int error);
void sockIsTimedout(int sid);
int sockCantRecvMore(int sid);
BOOL sockIsCantRecvMore(int sid);
void setErrno(int error);
int getErrno(void);

/* 
 * inet_addr
 *
 * The function converts a string containing an (IPv4) Internet 
 * Protocol dotted address into a actual IP address in a binary 
 * representation.  The address is returned in network byte order.
 * 
 * If the string does not contain a legitimate IP address, INADDR_NONE
 * is returned.
 */
unsigned long inet_addr(const char *cp);

struct hostent* gethostbyname(const char* name);

int mpx_DoConnect(U32 u32DstAddr, U16 u16DstPort, BOOL isBlocking);

/* 
 * inet_addr
 *
 * The function converts a string containing an (IPv4) Internet 
 * Protocol dotted address into a actual IP address in a binary 
 * representation.  The address is returned in network byte order.
 * 
 * If the string does not contain a legitimate IP address, INADDR_NONE
 * is returned.
 */
U32 inet_addr(const char *cp);

#endif      // __TC_SOCKET2_H_

