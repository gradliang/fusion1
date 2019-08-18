#ifndef _NET_NETLINK_H
#define _NET_NETLINK_H

#if 0
struct sockaddr_nl {
	sa_family_t sun_family;	/* AF_UNIX */
};
	sa_family_t	nl_family;	/* AF_NETLINK	*/
	unsigned short	nl_pad;		/* zero		*/
	__u32		nl_pid;		/* port ID	*/
       	__u32		nl_groups;	/* multicast groups mask */
#endif

#include <net/sock.h>
#include "typedef.h"

#define NLGRPSZ(x)	(ALIGN(x, sizeof(unsigned long) * 8) / 8)
#define NLGRPLONGS(x)	(NLGRPSZ(x)/sizeof(unsigned long))

struct netlink_sock {
    struct sock      sk;
	sa_family_t	nl_family;	/* AF_NETLINK	*/
    U32 pid;
    U16 u16SockId;
    U32 dst_pid;
    U32 dst_group;
    U32 flags;
	U32	subscriptions;
    U32 ngroups;
    unsigned long *groups;
    unsigned long state;
    wait_queue_head_t wait;
    struct netlink_callback *cb;
    struct mutex *cb_mutex;
    struct mutex cb_def_mutex;
    void (*netlink_rcv)(struct sk_buff *skb);
};

#define NETLINK_CB(skb)		(*(struct netlink_skb_parms*)&((skb)->cb))

#endif /* _NET_NETLINK_H */
