/*
 * =====================================================================================
 *
 *       Filename:  net_socket3.c
 *
 *    Description:  
 *        Company:  Magic Pixel, Inc
 *
 * =====================================================================================
 */

#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"
#include "lwip_config.h"
#include "net_netctrl.h"
#include "net_socket.h"
#include "net_nic.h"
#include "linux/if_arp.h"
#include "net_netlink.h"
#include "log.h"
#include "net/genetlink.h"
#include "util_queue.h"
#include "lwip_opt.h"

#if LOCAL_DEBUG_ENABLE
#define SOCKET_DEBUGF(debug,x) mpDebugPrint x
#else
#define SOCKET_DEBUGF(debug,x)
#endif

#ifndef SOL_NETLINK
#define SOL_NETLINK 270
#endif

#define NETLINK_KERNEL_SOCKET	0x1
#define NETLINK_RECV_PKTINFO	0x2


/*  
 * bitmap flags for socket event  (copy from net_socket.c)
 */
#define SOCK_SIGNAL_TX_DONE     1
#define SOCK_SIGNAL_RX_DONE     2
#define SOCK_SIGNAL_TX_READY    SOCK_SIGNAL_TX_DONE
#define SOCK_SIGNAL_SOCKET_STATE_CHANGED    4

struct lwip_stats_s {
    unsigned long rx_dropped;
};

struct task_data {
    BYTE         task_id;
    BYTE         u08SockEventId;
    ST_SOCKET    SockBank[SOCK_MAXIMAL_NUMBER];
    ST_NET_BUF_LIST stPacketList[SOCK_MAXIMAL_NUMBER];
    struct lwip_stats_s stats[SOCK_MAXIMAL_NUMBER];
    ST_QUEUE* sockConnQueue[SOCK_MAXIMAL_NUMBER];
};

static struct task_data *TaskData[OS_MAX_NUMBER_OF_TASK];

struct packet_sock {
	/* struct sock has to be the first member of packet_sock */
    struct sock sk;
    struct list_head list;
	int			ifindex;	/* bound device		*/
	unsigned int		running:1,	/* prot_hook is attached*/
				auxdata:1,
				origdev:1;
	__be16			num;
    struct packet_type prot_hook;
    struct sockaddr_ll addr;
    u16 u16SockId;
};

S32 NetBufferPacketPush2(ST_NET_BUF_LIST *list, U32 packet);
void SockIdSignalRxDone3(U16 sid, void* packet);
ST_UNIX_PCB* UnixNew();
static int sock_Disconnect2(int sid, ST_SOCKET *socket);
static S32 NetBufferDataRead2(U16 sid, U08* buffer, U32 bufferSize, struct sockaddr *from, ST_NET_BUF_LIST *stPacketList);
static void NetBufferTypeSet2(ST_NET_BUF_LIST *list, U16 u16Type);
static void NetBufferFamilySet2(ST_NET_BUF_LIST *list, U16 u16Family);
static U16 NetBufferPacketNumGet2(ST_NET_BUF_LIST *list);
static void NetBufferRelease2(U16 sid, ST_NET_BUF_LIST *list);

extern void dev_add_pack(struct packet_type *pt);
extern int UnixConnect(ST_UNIX_PCB* pcb, struct sockaddr_un *addr);
extern BYTE myethaddr[6];
extern S32 _TcpClose(ST_TCP_PCB* pcb);

static inline void NetlinkFree(ST_NETLINK_PCB* pcb)
{
    mpx_Free(pcb);
}

U32 sockQueueLengthGet2(ST_QUEUE *_queue)
{
    if (_queue)
        return _queue->length;
    else
    {
        MP_ASSERT(0);
        return 0;
    }
}
/*
 *	Copy iovec to kernel. Returns -EFAULT on error.
 *
 *	Note: this modifies the original iovec.
 */

static int NetlinkSocketFind(U32 pid)
{
    int sid;
    short i;
    struct netlink_sock *nlk;
    struct task_data *tk;

    MP_ASSERT(0);
    /* TODO: how many socket banks ?  iterate all of them */
    for (i=0; i<OS_MAX_NUMBER_OF_TASK; i++)
    {
        tk = TaskData[i];

        for (sid=1; sid<SOCK_MAXIMAL_NUMBER; sid++)
        {
            if(tk->SockBank[sid].u08Type != SOCK_TYPE_ALLOCATED)
                continue;

            if (nlk->dst_pid == pid)
                break;
        }

        if (sid < SOCK_MAXIMAL_NUMBER)
            break;
    }

    if (sid < SOCK_MAXIMAL_NUMBER)
        return sid;
    else
        return 0;
}

extern void genl_rcv(struct sk_buff *skb);

int NetlinkRead(struct sock *sk, struct sk_buff *skb)
{
    struct netlink_sock *nlk = (struct netlink_sock *) sk;

    MP_ASSERT(nlk->u16SockId > 0);
//    DBG("sid=%x, skb=%p", nlk->u16SockId, skb);
//    if (skb->len == 36)
//        MP_ASSERT(0);
    SockIdSignalRxDone3(nlk->u16SockId, skb);
    return 0;
}

extern int m_packet_sendmsg(struct sock *sk, struct msghdr *msg);
ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags)
{
    struct sockaddr_nl *addr = msg->msg_name;
    U32 dst_pid;
    U32 dst_group;
    BYTE tid = sockfd >> 8;
    short sid = sockfd & 0xff;
//	struct iovec iovstack[UIO_FASTIOV], *iov = iovstack;
	struct iovec *iov = msg->msg_iov;
    int len;
    struct sk_buff *skb;
    int err;
    struct msghdr msg_kernel;
	struct iovec iov_kernel[msg->msg_iovlen];

    struct task_data *tk = TaskData[tid];

    ST_SOCKET *msock = &tk->SockBank[sid];
    struct netlink_sock *nlk = msock->pcb.netlink;
    struct sock *sk = &nlk->sk;
    size_t size;

//    DBG("sid=%x", sockfd);
    int ct;

    if (msock->u16Family == AF_PACKET)
    {
//        MP_ASSERT(0);
        struct packet_sock *po = msock->pcb.packet;
        return m_packet_sendmsg(&po->sk, msg);
    }
	memset(&msg_kernel, 0, sizeof(msg_kernel));
	msg_kernel.msg_iov = iov_kernel;
	msg_kernel.msg_iovlen = msg->msg_iovlen;
	msg_kernel.msg_name = msg->msg_name;
	msg_kernel.msg_namelen = msg->msg_namelen;
	size = msg->msg_iovlen * sizeof(struct iovec);
    memcpy(iov_kernel, iov, size);

    err = 0;
	for (ct = 0; ct < msg->msg_iovlen; ct++) {
		err += iov[ct].iov_len;
		/*
		 * Goal is not to verify user data, but to prevent returning
		 * negative value, which is interpreted as errno.
		 * Overflow is still possible, but it is harmless.
		 */
		if (err < 0)
        {
            MP_ASSERT(0);
			return -EMSGSIZE;
        }
	}
    len = err;

    if (msg->msg_namelen) {
        if (addr->nl_family != AF_NETLINK)
        {
            setErrno(EINVAL);
            MP_ASSERT(0);
            return SOCKET_ERROR;
        }

        dst_pid = addr->nl_pid;
        dst_group = ffs(addr->nl_groups);
    }
    else {
        dst_pid = nlk->dst_pid;
        dst_group = nlk->dst_group;
    }

    if (!nlk->pid) {
            MP_ASSERT(0);
#if 0
        err = netlink_autobind(sock); TODO
        if (err)
        {
            goto out;
        }
#endif
    }

#ifdef LINUX
    err = -EMSGSIZE;
    if (len > sk->sk_sndbuf - 32)
        goto out;
#endif

#if 0
    skb = alloc_skb(len, GFP_KERNEL);
#else
    skb = nlmsg_new(len, GFP_KERNEL);
#endif
    if (!skb)
    {
        MP_ASSERT(0);
        goto out;
    }

    NETLINK_CB(skb).pid	= nlk->pid;
	NETLINK_CB(skb).dst_group = dst_group;

//    DBG("calls memcpy_fromiovec() len=%d", len);
    err = -EFAULT;
	if (memcpy_fromiovec(skb_put(skb, len), msg_kernel.msg_iov, len)) {
        MP_ASSERT(0);
		kfree_skb(skb);
		goto out;
	}

    err = -ENOBUFS;
	if (dst_group) {
		atomic_inc(&skb->users);
		netlink_broadcast(sk, skb, dst_pid, dst_group, GFP_KERNEL);
	}
//    DBG("before netlink_unicast skb=%p", skb);
//    MP_ASSERT(0);
//    DBG("calls netlink_unicast skb=%p, sk(%d)=%p", skb, dst_pid, sk);
    return netlink_unicast(sk, skb, dst_pid, msg->msg_flags&MSG_DONTWAIT);

out:
    MP_ASSERT(0);
    return -1;
}

extern int m_netlink_recvmsg(struct socket *sock, struct msghdr *msg, size_t len, int flags);

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags)
{
//    struct socket *sock;
    BYTE tid = sockfd >> 8;
    struct task_data *tk = TaskData[tid];
    int noblock = flags&MSG_DONTWAIT;
//    DBG("sid=%x", sockfd);
    sockfd &= 0xff;
    ST_SOCKET *sock = &tk->SockBank[sockfd];
	struct netlink_sock *nlk = sock->pcb.netlink;
	struct sock *sk = (struct sock *)nlk;
    int err;
    struct socket bsd_sock;
    size_t len;
	struct iovec *iov = msg->msg_iov;
    struct msghdr msg_kernel;
	struct iovec iov_kernel[msg->msg_iovlen];

    MP_ASSERT(sock->u16Family == AF_NETLINK);

    int ct;
    size_t size;
    len = 0;
	memset(&msg_kernel, 0, sizeof(msg_kernel));
	msg_kernel.msg_iov = iov_kernel;
	msg_kernel.msg_iovlen = msg->msg_iovlen;
	msg_kernel.msg_name = msg->msg_name;
	msg_kernel.msg_namelen = msg->msg_namelen;
	size = msg->msg_iovlen * sizeof(struct iovec);
    memcpy(iov_kernel, iov, size);

	for (ct = 0; ct < msg->msg_iovlen; ct++) {
		len += iov[ct].iov_len;
	}


    bsd_sock.sk = sk;
    err =  m_netlink_recvmsg(&bsd_sock, &msg_kernel, len, flags);
    if (err < 0)
    {
        MP_ASSERT(0);
        setErrno(-err);
        return -1;
    }
    else if (err == 0)
    {
        if (noblock)
        {
            setErrno(EWOULDBLOCK);
            return -1;
        }
        else
        {
            MP_ASSERT(0);
            return 0;
        }
    }
    else
    {
        if (msg_kernel.msg_namelen > 0)
        {
            msg->msg_namelen = msg_kernel.msg_namelen;
            // TODO iov_len == ?
        }
        return err;
    }

}

S32 SockAllocate2(ST_SOCKET *sbank)
{
	ST_SOCKET *point;
	U16 id;
	S32 status;
    
    status = ERR_NO_FD;                 // preset status to no file descriptor available
    id = 1;                             // reserve id = 0 for the convenience of AP impletement
    point = &sbank[1];
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

static ST_NETLINK_PCB* NetlinkNew(int type, int protocol)
{
    extern int m_netlink_create(struct socket *sock, int protocol);
    ST_NETLINK_PCB*ret= mpx_Malloc(sizeof(ST_NETLINK_PCB));
    if (ret)
    {
        struct socket bsd_socket;
        memset(ret, 0, sizeof(*ret));
        bsd_socket.sk = ret;
        bsd_socket.type = type;
        if (m_netlink_create(&bsd_socket, protocol) < 0)
        {
            MP_ASSERT(0);
            mpx_Free(ret);
            ret = NULL;
        }
    }
    return ret;
}

static ST_PACKET_PCB* PacketNew(int type, int protocol)
{
    ST_PACKET_PCB*ret= mpx_Malloc(sizeof(ST_PACKET_PCB));
    return ret;
}

/* NOTE: id doesn't have task ID part */
static void SockFree2(U16 id, struct task_data *tk)
{
    IntDisable();
    NetBufferRelease2(id, tk->stPacketList);
    tk->SockBank[id].u08Type = SOCK_TYPE_FREE;
    IntEnable();
}

int lwip_socket(int domain, int type, int protocol)
{
    register int status;
    register ST_SOCKET *socket;
    BYTE tid = TaskGetId();
    struct task_data *tk;
    U16 s;

    if (!TaskData[tid])
        TaskData[tid] = mm_zalloc(sizeof(struct task_data));

    tk = TaskData[tid];
    
    SOCKET_DEBUGF(0, ("lwip_socket: af=%d,type=%x,proto=%d", domain, type, protocol));

    type &= ~SOCK_CLOEXEC;

    if (domain == AF_UNSPEC)
        domain = AF_INET;                       /* use default */

    if(domain != AF_INET && domain != AF_PACKET && domain != AF_UNIX && domain != AF_NETLINK)
    {
        setErrno(EAFNOSUPPORT);
        MP_ASSERT(0);
		return SOCKET_ERROR;
    }
    else if(type != SOCK_STREAM && type != SOCK_DGRAM && type != SOCK_RAW)
    {
        setErrno(EPROTONOSUPPORT);
        MP_ASSERT(0);
		return SOCKET_ERROR;
    }
    else if(domain == AF_INET && type == SOCK_STREAM && protocol == IPPROTO_UDP)
    {
        setErrno(EPROTOTYPE);
        MP_ASSERT(0);
		return SOCKET_ERROR;
    }
    else if(domain == AF_INET && type == SOCK_RAW && (protocol == IPPROTO_UDP || protocol == IPPROTO_TCP))
    {
        setErrno(EPROTONOSUPPORT);
        MP_ASSERT(0);
		return SOCKET_ERROR;
    }
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
        status = (int)SockAllocate2(tk->SockBank);
        IntEnable();
        
        s = status + (tid << 8);                        /* add task ID part */

        if(status > 0)
        {
            socket = &tk->SockBank[status];
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
                    SockFree2((U16)status, tk);
                    setErrno(ENOBUFS);
                    MP_ASSERT(0);
                    return SOCKET_ERROR;
                }
            }
            else if(domain == AF_NETLINK)
            {
                if(type != SOCK_RAW && type != SOCK_DGRAM)
                {
                    SockFree2((U16)status, tk);
                    setErrno(EPROTONOSUPPORT);
                    MP_ASSERT(0);
                    return SOCKET_ERROR;
                }

                socket->pcb.netlink = NetlinkNew(type, protocol);
                if(!socket->pcb.netlink)
                {
                    SockFree2((U16)status, tk);
                    setErrno(ENOBUFS);
                    MP_ASSERT(0);
                    return SOCKET_ERROR;
                }
                else
                    socket->pcb.netlink->u16SockId = (U16)s;
            }
            else if(domain == AF_PACKET)
            {
                ST_PACKET_PCB *po;
                po = socket->pcb.packet = PacketNew(type, protocol);
                if(!socket->pcb.packet)
                {
                    SockFree2((U16)status, tk);
                    setErrno(ENOBUFS);
                    MP_ASSERT(0);
                    return SOCKET_ERROR;
                }
                else
                {
                    struct socket sock;

                    po->u16SockId = (U16)s;

                    sock.type = type;
                    sock.sk = &po->sk;
                    po->num = protocol;

                    m_packet_create(&sock, protocol);
                }
            }
            else if(type == SOCK_STREAM)
            {
                socket->pcb.tcp = TcpNew();
                if(!socket->pcb.tcp)
                {
                    SockFree2((U16)status, tk);
                    setErrno(ENOBUFS);
                    return SOCKET_ERROR;
                }
                else
                    socket->pcb.tcp->u16SockId = (U16)s;
            }
            else if(type == SOCK_DGRAM)
            {
                socket->pcb.udp = UdpNew();
                if(!socket->pcb.udp)
                {
                    SockFree2((U16)status, tk);
                    setErrno(ENOBUFS);
                    MP_ASSERT(0);
                    return SOCKET_ERROR;
                }
            }
            
            memset(&socket->Local, 0, sizeof(ST_SOCK_ADDR));
            memset(&socket->Peer, 0, sizeof(ST_SOCK_ADDR));
            
            NetBufferTypeSet2(&tk->stPacketList[status], type);
            NetBufferFamilySet2(&tk->stPacketList[status], domain);
        }
        else
        {
            status = SOCKET_ERROR;
            setErrno(ENFILE);                   /* BSD compliant */
        }
    }
    
    MP_ASSERT(status > 0);
    if (status > 0)
        status = s;
    if (status > 0)
        SOCKET_DEBUGF(0, ("lwip_socket: returns s=%x"));
    return status;
}


int lwip_select(int nfds, fd_set *read, fd_set *write, fd_set *excep, struct timeval *timeout)
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
    register short i, k, kk;
    ST_SOCK_SET eventMask;
    BYTE tid = TaskGetId();                     /* current task */
    struct task_data *tk = TaskData[tid];
    
    if (timeout)
        tm = (timeout->tv_sec * 1000 + timeout->tv_usec / 1000); /* in millisecond */
    else
        tm = 0;                                 /* blocking indefinitely */

    MPX_FD_ZERO(&readMask);
    MPX_FD_ZERO(&writeMask);
    MPX_FD_ZERO(&exceptMask);

    MPX_FD_ZERO(&eventMask);

    /* start from 1 (not 0) */
    /* each task has up to 32 sockets at the max 
     * don't want to loop here too long 
     */
//    for (i = 1; i < nfds; i++)
//        MP_ASSERT(0);
    for (i =1; i < SOCK_MAXIMAL_NUMBER ; i++)
    {
        if (read && FD_ISSET(i,read))
        {
            kk = i & 0xff;
            MPX_FD_SET(kk, &readMask);
            if (kk < SOCK_MAXIMAL_NUMBER)
                MPX_FD_SET(kk, &eventMask);
            else
                MPX_FD_SET(0, &eventMask);
        }

        if (write && FD_ISSET(i,write))
        {
            kk = i & 0xff;
            MPX_FD_SET(kk, &writeMask);
            if (kk < SOCK_MAXIMAL_NUMBER)
                MPX_FD_SET(kk, &eventMask);
            else
                MPX_FD_SET(0, &eventMask);
        }

        if (excep && FD_ISSET(i,excep))
        {
            kk = i & 0xff;
            MPX_FD_SET(kk, &exceptMask);
            if (kk < SOCK_MAXIMAL_NUMBER)
                MPX_FD_SET(kk, &eventMask);
            else
                MPX_FD_SET(0, &eventMask);
        }
    }

    FD_ZERO(read);
    FD_ZERO(write);
    FD_ZERO(excep);

    maskSave = eventMask.u32Mask;

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
            struct task_data *sock_tk;      /* task that owns the socket */
            if(result & mask)
            {
                socket = &tk->SockBank[sid];
                if (socket->u16SockState & SS_ISCLONE)
                {
                    MP_ASSERT(socket->u16Reserved2);
                    U16 otid = socket->u16Reserved2;   /* owner task id */
                    socket = &TaskData[otid]->SockBank[sid];
                    sock_tk = TaskData[otid];
                }
                else
                    sock_tk = tk;
                if(mask & readMask.u32Mask)   
                { 
                    if (socket->u16SockType==SOCK_STREAM) { /* TCP */
                        if ((socket->u16SockState & SS_NOFDREF)) { /* TODO */
                            status++;
                            FD_SET(sid, read);
                        }
                        else if (socket->u16SockOptions & SO_ACCEPTCONN) {
                            if (sockQueueLengthGet2(sock_tk->sockConnQueue[sid]) > 0) {
                                status++;
                                FD_SET(sid, read);
                            }
                        }
                        else if (socket->u16SockState & SS_ISCONNECTED) {
                            if ((NetBufferPacketNumGet2(&sock_tk->stPacketList[sid]) > 0)) {
                                status++;
                                FD_SET(sid, read);
                            }
                        }
                        else if ( !(socket->u16SockState & (SS_ISCONNECTING|SS_ISCONNECTED|SS_ISDISCONNECTING)) ) {
                            status++;
                            FD_SET(sid, read);
                        }
                    }
                    else if ((NetBufferPacketNumGet2(&sock_tk->stPacketList[sid]) > 0)) {
                        status++;
                        FD_SET(sid, read);
                    }
                }


                if(mask & writeMask.u32Mask)  
                { 
                    if (socket->u16SockType==SOCK_STREAM) { /* TCP */
                        if ((socket->u16SockState & SS_ISCONNECTED) &&
                                !(socket->u16SockState & SS_CANTSENDMORE) &&
                                (socket->pcb.tcp->u16SendBuffer > TCP_SNDLOWAT))     /* send buffer is available */
                        {
                            status++;
                            FD_SET(sid, write);

                            socket->u32Signal &= ~SOCK_SIGNAL_TX_READY;
                        }
                    } 
                    else {                                  /* UDP/RAW */
                        if (!(socket->u16SockState & SS_NOFDREF)) { /* TODO */
                            status++;
                            FD_SET(sid, write);
                        }
                    }
                }

                    if(mask & exceptMask.u32Mask)  
                    { 
                        if (socket->u16SockType == SOCK_STREAM) { /* TCP */

                            /* ---  In processing a connect call (non-blocking), connect failed  --- */

                            if ( !(socket->u16SockState & (SS_ISCONNECTING|SS_ISCONNECTED|SS_ISDISCONNECTING)) )
                            {
                                FD_SET(sid, excep);
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
#if 0
        if (tid == 21)
            MP_ASSERT(0);
#endif
        status = mpx_EventWaitWto(tk->u08SockEventId, maskSave, OS_EVENT_OR, &result, tm);
#if 0
        if (tid == 21)
        {
            if (result)
                MP_ASSERT(0);
        }
#endif
        //MP_DEBUG1("select: mpx_EventWaitWto returns %d", status);
    } while (status == 0);

    if (status == OS_STATUS_TIMEOUT){
        status = 0;                             /* time limit expires */
    }
    else 
    if (status < 0)
    {
        MP_ASSERT(0);
        status = SOCKET_ERROR;
        setErrno(ENETDOWN);
    }

    return status;
}

int lwip_bind(int sid, const struct sockaddr *name, int namelen)
{
    BYTE tid = sid >> 8;
    struct task_data *tk = TaskData[tid];
    register ST_SOCKET *socket;
    ST_SOCK_ADDR *addr;
    struct sockaddr_in *inp;
	struct sockaddr_nl *local;
    int ret, s = sid;

//	if (sid == 0x1503)
//        MP_ASSERT(0);
    sid &= 0xff;
    socket = &tk->SockBank[sid];
    if ((sid < 0) || (sid >= SOCK_MAXIMAL_NUMBER))
    {
        MP_ASSERT(0);
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
                    if(tk->SockBank[index].u08Type == SOCK_TYPE_ALLOCATED &&
                            index != sid &&
                            tk->SockBank[index].u16Family == PF_INET &&
                            tk->SockBank[index].u16Protocol == proto)
                    {
                        if ( (((ST_SOCK_ADDR_IN *)&tk->SockBank[index].Local.u08Address)->u32Addr == srcIp) &&
                                (tk->SockBank[index].Local.u16Port == port) )
                        {
                            setErrno(EADDRINUSE);
                            return SOCKET_ERROR;
                        }
                    }
                    index ++;
                }
            }
        }

        addr->u16Family = inp->sin_family;
        if (inp->sin_port == 0)
            addr->u16Port = mpx_NewLocalPort();
        else
            addr->u16Port = inp->sin_port;

        memcpy(addr->u08Address, (char *)&(inp->sin_addr), sizeof(struct in_addr));

        SOCKET_DEBUGF(0, ("lwip_bind: s=%d,p=%d", s,addr->u16Port));
    }
    else if (socket->u16Family == PF_LOCAL)
        SOCKET_DEBUGF(0, ("lwip_bind: s=%d,p=%s", s,((struct sockaddr_un *)name)->sun_path));
    else if (socket->u16Family == PF_NETLINK)
    {
        extern int m_netlink_bind(struct socket *sock, struct sockaddr *addr, int addr_len);
        ST_NETLINK_PCB *nlk = socket->pcb.netlink;
        struct socket sock;

        memset(&sock, 0, sizeof(sock));
        local = (struct sockaddr_nl *)name;
        nlk->nl_family = local->nl_family;
        sock.sk = &nlk->sk;
        return m_netlink_bind(&sock, name, namelen);
    }
    else if (socket->u16Family == PF_PACKET)
    {
        extern int m_packet_bind(struct packet_sock *po, struct sockaddr *uaddr, int addr_len);
        ST_PACKET_PCB *po = socket->pcb.packet;
        po->addr = *(struct sockaddr_ll *)name;
        return m_packet_bind(po, name, namelen);
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

    MP_ASSERT(ret == 0);
    return ret;
}

static S32 sockAccept2(U16 u16ListenSid, ST_TCP_PCB* pcb)
{
    BYTE tid = u16ListenSid >> 8;
    struct task_data *tk = TaskData[tid];
    register U32 mask;
    int ret;
    
    SOCKET_DEBUGF(0, ("sockAccept2: u16ListenSid=%x", u16ListenSid));
    u16ListenSid &= 0xff;

    if ((u16ListenSid < 0) || (u16ListenSid >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
        MP_ASSERT(0);
		return SOCKET_ERROR;
    }

    if(0 == mpx_QueuePush(tk->sockConnQueue[u16ListenSid], (U32)pcb))
    {
        mask = 1UL << u16ListenSid;
        MP_DEBUG2("sockAccept2: mpx_EventSet(%d, 0x%x)", tk->u08SockEventId,mask);
        ret = mpx_EventSet(tk->u08SockEventId, mask);
        MP_DEBUG3("sockAccept2: mpx_EventSet(%d, 0x%x) returns %d", tk->u08SockEventId,mask,ret);
        
        return 0;
    }
    else
    {
        //MP_DEBUG1("sockAccept2: mpx_QueuePush(%d) returns error", u16ListenSid);
        return -1;
    }
}

int lwip_listen(int sockId, int backlog)
{
    BYTE tid = sockId >> 8;
    struct task_data *tk = TaskData[tid];
    S32 err = NO_ERR;
    register ST_SOCKET *socket;
    
    SOCKET_DEBUGF(0, ("listen: s=%x", sockId));

    sockId &= 0xff;
    socket = &tk->SockBank[sockId];

    if ((sockId < 0) || (sockId >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
        MP_ASSERT(0);
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
    
    if(tk->sockConnQueue[sockId] == 0)
    {
        tk->sockConnQueue[sockId] = mpx_QueueCreate();
        if(tk->sockConnQueue[sockId] == 0)
        {
            DPrintf("[SOCKET] queue create fail");
            setErrno(ENOBUFS);
            return SOCKET_ERROR;
        }
    }
    
    TcpAcceptCallbackSet(socket->pcb.tcp, sockAccept2);
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

int lwip_accept(int sid, struct sockaddr *addr, int *addrlen)
{
    BYTE tid = sid >> 8;
    struct task_data *tk = TaskData[tid];
    ST_SOCK_ADDR address, *ap=&address;
    struct sockaddr_in *inp = (struct sockaddr_in *)addr;
    register ST_SOCKET *socket;
    int ret;

    SOCKET_DEBUGF(0, ("lwip_accept: s=%x", sid));

    sid &= 0xff;
    socket = &tk->SockBank[sid];

    if(sid <=0 || (sid >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
        MP_ASSERT(0);
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
        register ST_SOCKET *listenSocket = socket;
        register ST_SOCKET *newSocket;

        if(0 == mpx_QueueLengthGet(tk->sockConnQueue[sid]))
        {
            if (socket->u16SockState &  SS_NBIO)   /* non-blocking socket */
                setErrno(EWOULDBLOCK);
            else
                setErrno(ENOENT);
            return SOCKET_ERROR;
        }

        if(0 != mpx_QueuePop(tk->sockConnQueue[sid], &item))
        {
            DPrintf("[SOCKET] lwip_accept: queue pop fail");
            setErrno(ENOENT);
            return SOCKET_ERROR;
        }

        pcb = (ST_TCP_PCB*)item;


        TcpLock();

        if (pcb->eState != E_TCP_STATE_ESTABLISHED)
        {
            TcpFree(pcb);
            TcpUnlock();
            DPrintf("[SOCKET] accept failed - tcp connection(%d) already closed by peer", pcb->eState);
            setErrno(ECONNABORTED);
            return SOCKET_ERROR;
        }

        IntDisable();
        // critical section, protect it for thread-safe
        ret = SockAllocate2(tk->SockBank);
        IntEnable();

        if (ret > 0)
        {
            newSocket = &tk->SockBank[ret];

            newSocket->u08NetIndex = listenSocket->u08NetIndex;
            newSocket->u16Family = listenSocket->u16Family;
            newSocket->u16SockType = listenSocket->u16SockType;
            newSocket->u16Protocol = IPPROTO_TCP;  // TODO

            newSocket->pcb.tcp = pcb;

            DPrintf("[SOCKET] accept connection");
            DPrintf("   from \\-");
            NetDebugPrintIP(pcb->u32PeerIp);
            DPrintf("   port %d", pcb->u16PeerPort);
            DPrintf("   local ipaddr %x", pcb->u32LocalIp);
            DPrintf("   at local port %d", pcb->u16LocalPort);

            mpx_SockAddrSet(&newSocket->Local, pcb->u32LocalIp, pcb->u16LocalPort);
            mpx_SockAddrSet(&newSocket->Peer, pcb->u32PeerIp, pcb->u16PeerPort);
            mpx_SockAddrSet(ap, pcb->u32PeerIp, pcb->u16PeerPort);

            NetBufferTypeSet2(&tk->stPacketList[ret], SOCK_STREAM);
            NetBufferFamilySet2(&tk->stPacketList[ret], newSocket->u16Family);

            _TcpAccept(pcb, (U16)ret | (tid << 8));

            TcpUnlock();

            newSocket->u16SockState |= SS_ISCONNECTED;

            if (inp)
            {
                inp->sin_family = ap->u16Family;
                inp->sin_port = ap->u16Port;
				
                memcpy(&(inp->sin_addr), ap->u08Address, sizeof(inp->sin_addr));
                if (addrlen)
                    *addrlen = sizeof(struct sockaddr_in);
            }

            ret |= (TaskGetId() << 8);
        }
        else
        {
            TcpClose(pcb);
            TcpUnlock();
            ret = SOCKET_ERROR;
            setErrno(ENFILE);
        }

    }
    return ret;
}

int lwip_recvfrom(int sid, void *message, int length, int flags, struct sockaddr *from, socklen_t *fromlen)
{
    BYTE tid = sid >> 8;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *socket;
    S32 dataLen = 0;
	int noblock = flags&MSG_DONTWAIT;
    
#if 0
    if (sid == 0x1506)
    {
//        MP_ASSERT(0);
    }
#endif

    MP_ASSERT(tid > 0);
    SOCKET_DEBUGF(0, ("lwip_recvfrom: s=%x", sid));
    sid &= 0xff;
    socket = &tk->SockBank[sid];

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

    if(socket->u16Family == AF_NETLINK)
    {
        dataLen = NetBufferDataRead2(sid, (U08*)message, length, from, tk->stPacketList);
        MP_ASSERT(0);
    }
    else if(socket->u16SockType == SOCK_DGRAM)
    {
        if(socket->u16Protocol == IPPROTO_UDP)
        {
            dataLen = NetBufferDataRead2(sid, (U08*)message, length, NULL, tk->stPacketList);
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
            dataLen = NetBufferDataRead2(sid, (U08*)message, length, NULL, tk->stPacketList);
            if(dataLen > 0 && from)
            {
                memset(from, 0, sizeof(struct sockaddr));
                from->sa_family = tk->SockBank[sid].u16Family;
                if (tk->SockBank[sid].u16Family == PF_PACKET)
                {
                    ST_SOCK_ADDR_LL *llp;
                    llp = (ST_SOCK_ADDR_LL *)from;
					
                    memcpy(llp->sll_addr, (void *)NetBufferFromMacAddrGet(sid), ETHERNET_MAC_LENGTH);
                }
            }
        }
        else
        {
            dataLen = NetBufferDataRead2(sid, (U08*)message, length, from, tk->stPacketList);
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

        dataLen = NetBufferDataRead2(sid, (U08*)message, length, NULL, tk->stPacketList);
        
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
        dataLen = NetBufferDataRead2(sid, (U08*)message, length, from, tk->stPacketList);
//        if (socket->u16Family == AF_PACKET)
//            DPrintf("[SOCKET] NetBufferDataRead2 returns %d", dataLen);
    }
    else
    {
        DPrintf("[SOCKET] unknown socket type");
        setErrno(ENOTSOCK);
        return SOCKET_ERROR;
    }
        
    if(dataLen == 0)
    {
        if ( (socket->u16SockState &  SS_NBIO) || noblock )  /* non-blocking socket */
        {
            setErrno(EWOULDBLOCK);
            dataLen = SOCKET_ERROR;
        }
        else                                    /* blocking socket */
        {
            U32 mask = 1UL << sid;
            mpx_EventWait(tk->u08SockEventId, mask, OS_EVENT_OR, &mask);//TODO

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
            else if (from->sa_family == AF_NETLINK)
                *fromlen = sizeof(struct sockaddr_nl);
        }
    }

    return dataLen;
}

int lwip_recv(int sid, void *mem, int len, unsigned int flags)
{
//    SOCKET_DEBUGF(0, ("recv: s=%x", sid));
    return lwip_recvfrom(sid, mem, len, flags, NULL, NULL);
}

int lwip_sendto(int sid, const char *buffer, int size, int flags,
			const struct sockaddr *to, int tolen)
{
    BYTE tid = sid >> 8;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *socket;
    S32 status;
    struct sockaddr sa_buf, *from;
    struct sockaddr_in *peer;
    ST_NET_PACKET *packet;
    
    SOCKET_DEBUGF(0, ("sendto: s=%x,sz=%d", sid,size));
    sid &= 0xff;
    socket = &tk->SockBank[sid];

    if ((sid <= 0) || (sid >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
        return SOCKET_ERROR;
    }

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
        packet->Net.u16SockId = sid; // TODO
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
                status = lwip_bind(sid + (tid << 8), from, sizeof(struct sockaddr));
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
            return lwip_send(sid, buffer, size, flags);
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

int lwip_send(int sid, const char *buffer, int size, int flags)
{
    BYTE tid = sid >> 8;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *socket;
    S32 status;
    struct sockaddr sa_buf, *from;
    int s = sid;
    
    SOCKET_DEBUGF(0, ("send: s=%d,sz=%d", sid,size));
    sid &= 0xff;
    socket = &tk->SockBank[sid];
    
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
    else if (socket->u16Family == AF_NETLINK)
    {
        return sendmsg(s, (struct msghdr *)buffer, flags);
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
            status = lwip_bind(sid + (tid << 8), from, sizeof(struct sockaddr));
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

#ifndef HAVE_RTSP
        int r = TcpWrite(socket->pcb.tcp, buffer, size);
#else
        int r = TcpWrite2(socket->pcb.tcp, buffer, size);
#endif
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

int lwip_closesocket(int sockId)
{
    BYTE tid = sockId >> 8;
    struct task_data *tk = TaskData[tid];
    int s = sockId;
    sockId &= 0xff;
    register ST_SOCKET *socket = &tk->SockBank[sockId];
    U32 mask = 1UL << sockId;
    
//    MP_ASSERT(sockId);
    SOCKET_DEBUGF(0, ("lwip_closesocket: s=%x", s));
    if ((sockId <= 0) || (sockId >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }
	
    if (socket->u16SockState & SS_NOFDREF)
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }

    TcpLock();

    if(socket->pcb.tcp)
    {
        if(socket->u16Family == AF_UNIX)
        {
            UnixFree(socket->pcb.unix);
            socket->pcb.unix = 0;
        }
        else if(socket->u16Family == AF_NETLINK)
        {
            NetlinkFree(socket->pcb.unix);
            socket->pcb.unix = 0;
        }
        else if(socket->u16SockType == SOCK_DGRAM)
        {
            UdpFree(socket->pcb.udp);
            socket->pcb.udp = 0;
        }
        else if(socket->u16SockType == SOCK_STREAM)
        {
            DPrintf("lwip_closesocket(%d) calls sock_Disconnect",sockId);
            sock_Disconnect2(sockId, socket);

            socket->pcb.tcp->u16SockId = 0;
            socket->pcb.tcp = 0;
        }
    }
    
    TcpUnlock();
    
    if (socket->u16SockState & SS_HASCLONE)
    {
        MP_ASSERT(socket->u16Reserved2);
        U16 atid = socket->u16Reserved2;
        MP_ASSERT(TaskData[atid]->SockBank[sockId].u16SockState & SS_ISCLONE);
        TaskData[atid]->SockBank[sockId].u08Type = SOCK_TYPE_FREE;
    }

    mpx_EventClear(tk->u08SockEventId, ~mask);
    
    SockFree2(sockId, tk);       /* deallocate socket */
    return NO_ERR;
}

/*
 * For per-task sockets 
 */
int m_ieee80211_monitor_sock;
int m_ieee80211_monitor_drop_frames;
void SockIdSignalRxDone3(U16 sid, void* packet)
{
    BYTE tid = sid >> 8;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *socket;
//    DPrintf("[SOCKET] SockIdSignalRxDone3 s=%x", sid);
    int s = sid;

    sid = sid & 0xff; 
    socket = &tk->SockBank[sid];

    if((sid >= SOCK_MAXIMAL_NUMBER) || (sid == 0))
    {
        MP_ASSERT(0);
        if (sid)
            DPrintf("[SOCKET] socket id %d is invalid", sid);
        if (packet)
            kfree_skb(packet);
        return;
    }
    
    if(socket->u08Type != SOCK_TYPE_ALLOCATED)
    {
        DPrintf("[SOCKET] socket %d not allocated", sid);
        if (packet)
            kfree_skb(packet);
        return;
    }
    
	if(!packet){
		DPrintf("[SOCKET] socket is closed/reset");
		NetBufferPacketPush2(&tk->stPacketList[sid], NULL);
	}
    else 
    {
        if(NO_ERR != NetBufferPacketPush2(&tk->stPacketList[sid], (U32)packet))
        {
            tk->stats[sid].rx_dropped++;
            if ((tk->stats[sid].rx_dropped % 128) == 0)
                DPrintf("[SOCKET] s=%x,dropped=%u: receive queue full", s, tk->stats[sid].rx_dropped);
            kfree_skb(packet);
            if (m_ieee80211_monitor_sock == s)
                m_ieee80211_monitor_drop_frames += 3;
            return;
        }
    }

    U32 mask;

#if 0
    if (tid == 21 && sid == 6)
    {
        dod100++;
//        MP_ASSERT(0);
    }
#endif
    mask = 1UL << sid;
    socket->u32Signal |= SOCK_SIGNAL_RX_DONE;
    if (socket->u16SockState & SS_HASCLONE)
    {
        MP_ASSERT(socket->u16Reserved2);
        U16 atid = socket->u16Reserved2;
        MP_DEBUG("[SOCKET] socket %x has different owner (tid=%d) than default", s, atid);
        mpx_EventSet(TaskData[atid]->u08SockEventId, mask);
    }
    else
        mpx_EventSet(tk->u08SockEventId, mask);
    return;
}

#ifdef HAVE_RTSP
void SockIdSignalRxDone4(U16 sid)
{
    BYTE tid = sid >> 8;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *socket;
//    DPrintf("[SOCKET] SockIdSignalRxDone4 s=%x", sid);
    int s = sid;

    if (!tk)
        return;

    sid = sid & 0xff; 
    socket = &tk->SockBank[sid];

    if((sid >= SOCK_MAXIMAL_NUMBER) || (sid == 0))
    {
        MP_ASSERT(0);
        if (sid)
            DPrintf("[SOCKET] socket id %d is invalid", sid);
        return;
    }
    
    if(socket->u08Type != SOCK_TYPE_ALLOCATED)
    {
        DPrintf("[SOCKET] socket %d not allocated", sid);
        return;
    }
    
    U32 mask;

    mask = 1UL << sid;
    socket->u32Signal |= SOCK_SIGNAL_RX_DONE;
    if (socket->u16SockState & SS_HASCLONE)
    {
        MP_ASSERT(socket->u16Reserved2);
        U16 atid = socket->u16Reserved2;
        MP_DEBUG("[SOCKET] socket %x has different owner (tid=%d) than default", s, atid);
        if (TaskData[atid])
            mpx_EventSet(TaskData[atid]->u08SockEventId, mask);
    }
    else
        mpx_EventSet(tk->u08SockEventId, mask);
    return;
}
#endif

int lwip_setsockopt(int sid, int level, int optname, const char *optval, int optlen)
{
    BYTE tid = sid >> 8;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *socket;
    long 	  OptVal = 0;
    int        i;

    SOCKET_DEBUGF(0, ("lwip_setsockopt: s=%x,o=%d", sid, optname));

    sid &= 0xff;

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

	socket = (ST_SOCKET *)&tk->SockBank[sid];

    if ((level != (int)SOL_SOCKET) && (level != IPPROTO_TCP) &&
        (level != IPPROTO_IP) && (level != SOL_NETLINK))
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

            case SO_PRIORITY:     /* ignore */
                break;
            default:
                setErrno(ENOPROTOOPT);
                MP_ASSERT(0);
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
    else if (level == SOL_NETLINK)
    {
        extern int m_netlink_setsockopt(struct socket *sock, int level, int optname, char __user *optval, int optlen);
        struct socket sock;
        memset(&sock, 0, sizeof(sock));
        sock.sk = &socket->pcb.netlink->sk;
        return m_netlink_setsockopt(&sock, level, optname, optval, optlen);
    }

    return 0;
}

/**
 * Retrieves the local name for a socket
 */
int lwip_getsockname(int s, struct sockaddr *name, socklen_t *addrlen)
{
    BYTE tid = s >> 8;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *sock;

    if (!name || !addrlen)
        return -1;

    s &= 0xff;
    sock = &tk->SockBank[s];

    if((s >= SOCK_MAXIMAL_NUMBER) || (s == 0))
    {
        MP_ASSERT(0);
        setErrno(ENOTSOCK);
        return -1;
    }

    if (sock->u16Family == AF_NETLINK)
    {
        struct sockaddr_nl *nl_addr = (struct sockaddr_nl *)name;
        ST_NETLINK_PCB *nl = sock->pcb.netlink;

        nl_addr->nl_family = AF_NETLINK;
        nl_addr->nl_pad = 0;
        nl_addr->nl_pid = nl->pid;
        *addrlen = sizeof(struct sockaddr_nl);
    }
    else if (sock->u16Family == AF_INET)
    {
        struct sockaddr_in *nm;
        nm = (struct sockaddr_in *)name;
        nm->sin_family = AF_INET;
        nm->sin_port = sock->Local.u16Port;
        memcpy(&nm->sin_addr.s_addr, sock->Local.u08Address, 4);
        *addrlen = sizeof(struct sockaddr_in);
    }
    else 
        MP_ASSERT(0);

	return 0;
}

struct netlink_sock *netlink_socket_get(int s)
{
    BYTE tid = s >> 8;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *sock;

    s &= 0xff;
    sock = &tk->SockBank[s];

    return sock->pcb.netlink;
}

static S32 NetBufferDataRead2(U16 sid, U08* buffer, U32 bufferSize, struct sockaddr *from, ST_NET_BUF_LIST *stPacketList)
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
            break;
        }
        
        if(stPacketList[sid].stAccessInfo.packet == 0)
        {
            U08* ip;
            
            stPacketList[sid].stAccessInfo.packet = NetBufferPacketPop2(&stPacketList[sid]);
#if 0
            if (sid == 6)
                DPrintf("[SOCKET] NetBufferPacketPop2 returns %p", stPacketList[sid].stAccessInfo.packet);
#endif
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
            
            if (stPacketList[sid].u16Family == AF_NETLINK)
            {
                stPacketList[sid].stAccessInfo.dataPtr = (U08*)NET_PACKET_ETHER(packet);
                stPacketList[sid].stAccessInfo.u16TotalLength = packet->Net.u16PayloadSize;
                stPacketList[sid].stAccessInfo.u16DataLength = stPacketList[sid].stAccessInfo.u16TotalLength;
                /* ----------  Get source address  ---------- */
                if (from)
                {
                    extern u32 m_netlink_group_mask(u32 group);
                    struct sk_buff *skb = (struct sk_buff *)(packet);
                    
                    struct sockaddr_nl addr;
                    addr.nl_family = AF_NETLINK;
                    addr.nl_pad = 0;
                    addr.nl_pid	= NETLINK_CB(skb).pid;
                    addr.nl_groups	= m_netlink_group_mask(NETLINK_CB(skb).dst_group);
                    memcpy(from, &addr, sizeof(addr));
                }
            }
            else
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
                    if (stPacketList[sid].u16Type == SOCK_RAW)
                    {
                        stPacketList[sid].stAccessInfo.dataPtr = (U08*)NET_PACKET_ETHER(packet);
                        stPacketList[sid].stAccessInfo.u16TotalLength = packet->Net.len;
                    }
                    else
                    {
                        stPacketList[sid].stAccessInfo.dataPtr = (U08*)NET_PACKET_PSEUDO(packet);
                        stPacketList[sid].stAccessInfo.u16TotalLength = packet->Net.u16PayloadSize;
                    }
                    if (stPacketList[sid].stAccessInfo.u16TotalLength == 0) // TODO
                        MP_ASSERT(0);
//                    DPrintf("[SOCKET] NetBufferDataRead2 AF_PACKET %d", stPacketList[sid].stAccessInfo.u16TotalLength);
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
                kfree_skb(stPacketList[sid].stAccessInfo.packet);
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

struct sk_buff *NetBufferPacketPop3(int sid)
{
    U32 packet = 0;
    int len;
    BYTE tid = sid >> 8;
    struct task_data *tk = TaskData[tid];
    sid &= 0xff;
    ST_SOCKET *sock = &tk->SockBank[sid];
    ST_NET_BUF_LIST *list = &tk->stPacketList[sid];
    
    packet = netBufferNodeRemove(list);

    if (packet && list->u16Family == AF_INET)
    {
        if(list->u16Type == SOCK_STREAM)
            len = ((ST_NET_PACKET *)packet)->Net.u16PayloadSize - sizeof(ST_TCP_HEADER);
        else
            len = ((ST_NET_PACKET *)packet)->Net.u16PayloadSize - sizeof(ST_UDP_HEADER);
        list->u32DataLen -= len;
    }
    else if (packet && list->u16Family == AF_NETLINK)
    {
        /* anything to do ? XXX */
    }
    return packet;
}

static int sock_Disconnect2(int sid, ST_SOCKET *socket)
{
    DPrintf("sock_Disconnect(%d)",sid);
    if ((sid <= 0) || (sid >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }
    
    if (socket->u16SockState & SS_ISDISCONNECTING)
        return 0;
    if (!(socket->u16SockState & SS_ISCONNECTED))
        return 0;

    if(socket->pcb.tcp == NULL)
        return 0;
#if 0
    if(NO_ERR != TcpDisconnect(socket->pcb.tcp))
#else
    if(NO_ERR != _TcpClose(socket->pcb.tcp))
#endif
    {
        DPrintf("[SOCKET] tcp abort");
        TcpAbort(socket->pcb.tcp);
    }
    socket->u16SockState &= ~(SS_ISCONNECTING);
    socket->u16SockState |= SS_ISDISCONNECTING;
    return 0;
}

const char etheraddr_zero[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


extern struct net_device g_net_device2;
int lwip_ioctlsocket(int sid, long cmd, unsigned long *argp)
{
    BYTE tid = sid >> 8;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *socket;
	
    SOCKET_DEBUGF(0, ("lwip_ioctlsocket: s=%x,o=%x", sid, cmd));
    sid &= 0xff;
    if ((sid < 0) || (sid >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
        MP_ASSERT(0);
        return SOCKET_ERROR;
    }

   socket = &tk->SockBank[sid];

   switch (cmd)
   {
       case FIONBIO:
           if ((unsigned long)*argp == 0)       /* set to blocking */
               socket->u16SockState &= ~SS_NBIO;
           else                                 /* set to non-blocking */
               socket->u16SockState |= SS_NBIO;
           break;

       case FIONREAD:
           *argp = NetBufferDataLengthGet(sid);// TODO
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

               if (memcmp(myethaddr, etheraddr_zero, 6) == 0)
               {
                   setErrno(ENODEV);
                   return SOCKET_ERROR;
               }
               memset(sa, 0, sizeof(*sa));
               sa->sa_family = ARPHRD_ETHER;
			   
               memcpy(sa->sa_data,myethaddr, ETH_ALEN);	   
           }
           break;

       case SIOCGIFFLAGS:
           {
               struct ifreq *ifr = (struct ifreq *)argp;
               struct net_device *dev = NetDeviceGetByIfname(ifr->ifr_name);
               MP_ASSERT(0);
               if (dev)
                   ifr->ifr_flags = dev->flags;
               else if (strcmp(ifr->ifr_name, "mon.wlan0") == 0)
               {
                   dev = &g_net_device2;
                   if (dev)
                       ifr->ifr_flags = dev->flags;
               }
               else
               {
                   setErrno(ENODEV);
                   return SOCKET_ERROR;
               }
           }
           break;
       case SIOCSIFFLAGS:
           {
               struct ifreq *ifr = (struct ifreq *)argp;
               DBG("SIOCSIFFLAGS %s", ifr->ifr_name);
               struct net_device *dev = NetDeviceGetByIfname(ifr->ifr_name);
               MP_ASSERT(0);
               if (dev)
                   dev->flags = ifr->ifr_flags;// TODO
               else if (strcmp(ifr->ifr_name, "mon.wlan0") == 0)
               {
                   dev = &g_net_device2;
                   if (dev)
                       dev->flags = ifr->ifr_flags;
               }
               else
               {
                   DBG("netdev not found %s", ifr->ifr_name);
                   setErrno(ENODEV);
                   return SOCKET_ERROR;
               }
           }
           break;
       default:
           setErrno(EINVAL);
           MP_ASSERT(0);
           return SOCKET_ERROR;
   }

   return 0;
}

static void NetBufferTypeSet2(ST_NET_BUF_LIST *list, U16 u16Type)
{
    list->u16Type = u16Type;
}

static void NetBufferFamilySet2(ST_NET_BUF_LIST *list, U16 u16Family)
{
    list->u16Family = u16Family;
}

int m_task_socket_data_setup(int task_id)
{
    struct task_data *tk;

    if (!TaskData[task_id])
    {
        TaskData[task_id] = mm_malloc(sizeof(struct task_data));
        TaskData[task_id]->task_id = 0;
    }

    tk = TaskData[task_id];
    if (task_id == tk->task_id)
        return 0;

    memset(tk, 0, sizeof(*tk));
    tk->task_id = task_id;
    tk->u08SockEventId = mpx_EventCreate(OS_ATTR_EVENT_CLEAR, 0);

    return 0;
}

static U16 NetBufferPacketNumGet2(ST_NET_BUF_LIST *list)
{
    if(list->u16ItemNum)
        return list->u16ItemNum;
    else if(list->stAccessInfo.u16DataLength)
        return 1;
    else if(list->stAccessInfo.u08LastPacket == 1)
        return 1;
    else
        return 0;
}

static void NetBufferRelease2(U16 sid, ST_NET_BUF_LIST *list)
{
    if(list[sid].stAccessInfo.packet)
    {
        DPrintf("release un-read data");
        kfree_skb(list[sid].stAccessInfo.packet);
    }
    
    memset(&list[sid].stAccessInfo, 0, sizeof(ST_NET_BUF_ACCESS_INFO));
    
    while(list[sid].u16ItemNum)
    {
        U32 packet = 0;
        packet = netBufferNodeRemove(&list[sid]);
        if(packet)
            kfree_skb(packet);
    }
}

int sock_Detach2(int sid)
{
    BYTE tid = sid >> 8;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *socket;

    sid &= 0xff;
    if ((sid <= 0) || (sid >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
        if (sid)
            MP_ASSERT(0);
		return SOCKET_ERROR;
    }
    
    socket = &tk->SockBank[sid];
    socket->pcb.tcp = NULL;
    return 0;
}

BOOL sockIsCantRecvMore2(int sid)
{
    BYTE tid = sid >> 8;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *socket;

//    DPrintf("[SOCKET] sockIsCantRecvMore2: socket id=%x", sid);
    sid &= 0xff;
    if ((sid <= 0) || (sid >= SOCK_MAXIMAL_NUMBER))
    {
		return TRUE;
    }
    
    socket = &tk->SockBank[sid];
    return (socket->u16SockState & SS_CANTRCVMORE) ? TRUE : FALSE;
}
void SockIdSignalTcpFinRecvd2(U16 sid)
{
    BYTE tid = sid >> 8;
    struct task_data *tk = TaskData[tid];
    U16 s = sid;
    ST_SOCKET *socket;

    DPrintf("[SOCKET] SockIdSignalTcpFinRecvd2: socket id=%x", sid);
    sid &= 0xff;
    socket = &tk->SockBank[sid];
    if((sid >= SOCK_MAXIMAL_NUMBER) || (sid == 0))
    {
    	DPrintf("[SOCKET] socket id %d is invalid", sid);
        return;
    }
    
    if(socket->u08Type != SOCK_TYPE_ALLOCATED)
    {
        DPrintf("[SOCKET] socket %d not allocated", sid);
        return;
    }
    
#ifdef NEW_NETSOCKET
    if (sockIsCantRecvMore2(s))
        return;
    else
#endif
		NetBufferPacketPush2(&tk->stPacketList[sid], NULL);

    U32 mask = 0;

    mask = 1UL << sid;
    socket->u32Signal |= SOCK_SIGNAL_RX_DONE;
    if (socket->u16SockState & SS_HASCLONE)
    {
        MP_ASSERT(socket->u16Reserved2);
        U16 atid = socket->u16Reserved2;
        MP_DEBUG("[SOCKET] socket %x has different owner (tid=%d)", s, atid);
        mpx_EventSet(TaskData[atid]->u08SockEventId, mask);
    }
    else
        mpx_EventSet(tk->u08SockEventId, mask);
}

void
socket_clone(int s)
{
    struct task_data *tk = TaskData[TaskGetId()];           /* current task */
    struct task_data *otk = TaskData[(unsigned int)s >> 8]; /* task that owns (or create) the socket */
    U16 fd = s & 0xff;
    ST_SOCKET *socket = &tk->SockBank[fd];
    ST_SOCKET *osocket = &otk->SockBank[fd];

    MP_ASSERT(socket->u08Type == SOCK_TYPE_FREE);

    socket->u08Type = SOCK_TYPE_ALLOCATED;

    socket->u08NetIndex = osocket->u08NetIndex;
    socket->u16Family = osocket->u16Family;
    socket->u16SockType = osocket->u16SockType;
    socket->u16Protocol = osocket->u16Protocol;
    socket->Local = osocket->Local;
    socket->Peer = osocket->Peer;
    socket->u16SockState = osocket->u16SockState;
    socket->u16SockOptions = osocket->u16SockOptions;
    

    socket->u16SockState |= SS_ISCLONE;
    socket->u16Reserved2 = s >> 8;                   /* save the actual task id */

    osocket->u16SockState |= SS_HASCLONE;
    osocket->u16Reserved2 = TaskGetId();                   /* save new task id */
}

int Conn2(int sid)
{
    BYTE tid = sid >> 8;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *socket;
    
    socket = &tk->SockBank[sid & 0xff];
    if (socket->u16SockState & SS_ISCONNECTED)
        return TRUE;
    else 
        return FALSE;
}

void sockErrorSet2(int sid, int error)
{
    BYTE tid = sid >> 8;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *socket;
    
    socket = &tk->SockBank[sid & 0xff];
    socket->u16SockError = error;
}

int sockIsDisconnected2(int sid)
{
    BYTE tid = sid >> 8;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *socket;
    int s = sid & 0xff;

    if ((s <= 0) || (s >= SOCK_MAXIMAL_NUMBER))
    {
        setErrno(ENOTSOCK);
		return SOCKET_ERROR;
    }
    
    socket = &tk->SockBank[s];

    if (!(socket->u16SockState & (SS_ISCONNECTED|SS_ISCONNECTING)))
        return 0;

    socket->u16SockState &= ~(SS_ISCONNECTING|SS_ISCONNECTED|SS_ISDISCONNECTING);
    MP_DEBUG("sockIsDisconnected2(%x)",sid);
    SockIdSignalRxDone(sid, NULL);
    return 0;
}

int lwip_connect(int sockId, const struct sockaddr *name, int namelen)
{
    BYTE tid = sockId >> 8;
    struct task_data *tk = TaskData[tid];
    register ST_SOCKET *socket;
    struct sockaddr_in *addr;
    ST_SOCK_ADDR *destAddr;
    S32 status = NO_ERR;
    
    SOCKET_DEBUGF(0, ("lwip_connect: s=%x", sockId));

    sockId &= 0xff;
    socket = &tk->SockBank[sockId];

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

        addr = (struct sockaddr_in *) name;
        destAddr = &socket->Peer;
        destAddr->u16Family = addr->sin_family;
        destAddr->u16Port = addr->sin_port;
		
        memcpy(destAddr->u08Address, (char *)&(addr->sin_addr), sizeof(struct in_addr));
        SOCKET_DEBUGF(0, ("lwip_connect: port=%d", destAddr->u16Port));
    }
   
    if(socket->u16SockType == SOCK_DGRAM)
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
            U32 mask = 1UL << sockId;
            mpx_EventWait(tk->u08SockEventId, mask, OS_EVENT_OR, &mask);

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

//#include <sys/poll.h>
/* Event types that can be polled for.  These bits may be set in `events'
   to indicate the interesting event types; they will appear in `revents'
   to indicate the status of the file descriptor.  */
#define POLLIN		0x001		/* There is data to read.  */
#define POLLPRI		0x002		/* There is urgent data to read.  */
#define POLLOUT		0x004		/* Writing now will not block.  */

/* Data structure describing a polling request.  */
struct pollfd
  {
    int fd;			/* File descriptor to poll.  */
    short int events;		/* Types of events poller cares about.  */
    short int revents;		/* Types of events that actually occurred.  */
  };

/* Type used for the number of file descriptors.  */
typedef unsigned long int nfds_t;

#ifdef LINUX_3_6_COMPAT_H
#if Make_USB == AR9271_WIFI
extern int m_ath9k_tx_queue_H(void);
extern int m_ath9k_tx_queue_L(void);
#endif
#endif
int dod603[8];
int dod604[8];
const int netpool_low_watermark = 12;

short tx_udp_stopped;
int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    short i;

    for (i=0; i<nfds; i++)
    {
        int s = fds[i].fd;
        unsigned short tid = s >> 8;
        unsigned short fd = s & 0xff;
        struct task_data *tk = TaskData[tid];

        fds[i].revents = 0;

        if (s < 0)
            continue;

        if (fds[i].events & POLLIN)
        {
            if (NetBufferPacketNumGet2(&tk->stPacketList[fd]) > 0)
                fds[i].revents |= POLLIN;
        }
        if (fds[i].events & POLLOUT)
        {
            ST_SOCKET *sk = &tk->SockBank[fd];

            if ((sk->u16Protocol == IPPROTO_UDP) )
            {
                /* only check for UDP; packet may be dropped. */

#ifdef LINUX_3_6_COMPAT_H
#if Make_USB == AR9271_WIFI
                if (tx_udp_stopped && !m_ath9k_tx_queue_L())
                    tx_udp_stopped = FALSE;
                else if (!tx_udp_stopped && m_ath9k_tx_queue_H())
                    tx_udp_stopped = TRUE;
#endif
#endif

                if (!tx_udp_stopped)
                {
                    if (m_free_buffers() < netpool_low_watermark)
                    {
                        if (dod603[5] == 0)
                            dod604[5] = GetSysTime();
                        dod603[5]++;
                    }
                    else if (UdpWritable(sk->pcb.udp))
                        fds[i].revents |= POLLOUT;
                }
                else
                {
                    if (dod603[0] == 0)
                        dod604[0] = GetSysTime();
                    dod603[0]++;
                }
            }
            else
                fds[i].revents |= POLLOUT;      /* TODO */
        }
    }
    return 0;
}

int m_semaphoreCreate(int count)
{
    return mpx_SemaphoreCreate(OS_ATTR_PRIORITY, count);
}

int m_socket_writable(int s)
{
    unsigned short tid = s >> 8;
    unsigned short fd = s & 0xff;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *socket;

    socket = &tk->SockBank[fd];

    if (socket->u16Protocol == IPPROTO_TCP)
    {
        return (socket->pcb.tcp->u16SendBuffer > TCP_SNDLOWAT) ? 1 : 0;
    }
    else
    {
        MP_ASSERT(0);
        return 0;
    }
}

#ifdef HAVE_RTSP
extern int tcp_writable(ST_TCP_PCB *pcb);
int m_socket_writable2(int s)
{
    unsigned short tid = s >> 8;
    unsigned short sd = s & 0xff;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *socket;

    socket = &tk->SockBank[sd];

    if (socket->u16Protocol == IPPROTO_TCP)
    {
#if 0
        return tcp_writable(socket->pcb.tcp);
#else
        return tcp_writable3(socket->pcb.tcp);
#endif
    }
    else
    {
        MP_ASSERT(0);
        return 0;
    }
}

int m_socket_writable3(int s)
{
    unsigned short tid = s >> 8;
    unsigned short sd = s & 0xff;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *socket;

    socket = &tk->SockBank[sd];

    if (socket->u16Protocol == IPPROTO_TCP)
    {
        return tcp_writable2(socket->pcb.tcp);
    }
    else
    {
        return 1;
        }
}
#endif

int m_socket_tcp_sendbuf(int s)
{
    unsigned short tid = s >> 8;
    unsigned short fd = s & 0xff;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *socket;

    socket = &tk->SockBank[fd];

    if (socket->u16Protocol == IPPROTO_TCP)
    {
        return socket->pcb.tcp->u16SendBuffer;
    }
    else
    {
        MP_ASSERT(0);
    return 0;
    }
}

ST_TCP_PCB *m_socket_tcp_pcb(int s)
{
    unsigned short tid = s >> 8;
    unsigned short fd = s & 0xff;
    struct task_data *tk = TaskData[tid];
    ST_SOCKET *socket;

    socket = &tk->SockBank[fd];

    return socket->pcb.tcp;
}

