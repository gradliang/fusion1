/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		PACKET - implements raw packet sockets.
 *
 * Authors:	Ross Biro
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *		Alan Cox, <gw4pts@gw4pts.ampr.org>
 *
 * Fixes:
 *		Alan Cox	:	verify_area() now used correctly
 *		Alan Cox	:	new skbuff lists, look ma no backlogs!
 *		Alan Cox	:	tidied skbuff lists.
 *		Alan Cox	:	Now uses generic datagram routines I
 *					added. Also fixed the peek/read crash
 *					from all old Linux datagram code.
 *		Alan Cox	:	Uses the improved datagram code.
 *		Alan Cox	:	Added NULL's for socket options.
 *		Alan Cox	:	Re-commented the code.
 *		Alan Cox	:	Use new kernel side addressing
 *		Rob Janssen	:	Correct MTU usage.
 *		Dave Platt	:	Counter leaks caused by incorrect
 *					interrupt locking and some slightly
 *					dubious gcc output. Can you read
 *					compiler: it said _VOLATILE_
 *	Richard Kooijman	:	Timestamp fixes.
 *		Alan Cox	:	New buffers. Use sk->mac.raw.
 *		Alan Cox	:	sendmsg/recvmsg support.
 *		Alan Cox	:	Protocol setting support
 *	Alexey Kuznetsov	:	Untied from IPv4 stack.
 *	Cyrus Durgin		:	Fixed kerneld for kmod.
 *	Michal Ostrowski        :       Module initialization cleanup.
 *         Ulises Alonso        :       Frame number limit removal and
 *                                      packet_set_ring memory leak.
 *		Eric Biederman	:	Allow for > 8 byte hardware addresses.
 *					The convention is that longer addresses
 *					will simply extend the hardware address
 *					byte arrays at the end of sockaddr_ll
 *					and packet_mreq.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 */

#define LOCAL_DEBUG_ENABLE 1
#include <asm/uaccess.h>
#include <asm/system.h>
#include <linux/bitops.h>
#include <linux/capability.h>
#include <linux/cpu.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/notifier.h>
#include <linux/skbuff.h>
#include <net/net_namespace.h>
#include <net/sock.h>
#include <linux/rtnetlink.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/if_bridge.h>
#include <linux/if_macvlan.h>
#include <net/dst.h>
#include <net/pkt_sched.h>
#include <net/checksum.h>
#include <linux/highmem.h>
#include <linux/init.h>
#include <linux/kmod.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/netpoll.h>
#include <linux/rcupdate.h>
#include <linux/delay.h>
#include <net/wext.h>
#include <net/iw_handler.h>
#include <asm/current.h>
#include <linux/audit.h>
#include <linux/dmaengine.h>
#include <linux/err.h>
#include <linux/ctype.h>
#include <linux/if_arp.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/ipv6.h>
#include <linux/in.h>
#include <linux/jhash.h>
#include <linux/random.h>

#ifdef LINUX
#include "net-sysfs.h"
#endif

#include "mpTrace_copy.h"
#include "log.h"

/* copy from net_socket3.c;  always keep them sync */
typedef struct packet_sock {
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

struct packet_skb_cb {
	unsigned int origlen;
	union {
		struct sockaddr_pkt pkt;
		struct sockaddr_ll ll;
	} sa;
};

#define PACKET_SKB_CB(__skb)	((struct packet_skb_cb *)((__skb)->cb))

static inline struct packet_sock *pkt_sk(struct sock *sk)
{
	return (struct packet_sock *)sk;
}

static int packet_do_bind(struct sock *sk, struct net_device *dev, __be16 protocol)
{
	struct packet_sock *po = pkt_sk(sk);
	/*
	 *	Detach an existing hook if present.
	 */

#ifdef LINUX
	lock_sock(sk);

	spin_lock(&po->bind_lock);
#endif
	if (po->running) {
		__sock_put(sk);
		po->running = 0;
		po->num = 0;
#ifdef LINUX
		spin_unlock(&po->bind_lock);
#endif
		dev_remove_pack(&po->prot_hook);
#ifdef LINUX
		spin_lock(&po->bind_lock);
#endif
	}

	po->num = protocol;
	po->prot_hook.type = protocol;
	po->prot_hook.dev = dev;

	po->ifindex = dev ? dev->ifindex : 0;

	if (protocol == 0)
		goto out_unlock;

	if (!dev || (dev->flags & IFF_UP)) {
		dev_add_pack(&po->prot_hook);
		sock_hold(sk);
		po->running = 1;
	} else {
		sk->sk_err = ENETDOWN;
		if (!sock_flag(sk, SOCK_DEAD))
			sk->sk_error_report(sk);
	}

out_unlock:
#ifdef LINUX
	spin_unlock(&po->bind_lock);
	release_sock(sk);
#endif
	return 0;
}
static int packet_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len)
{
	struct sockaddr_ll *sll = (struct sockaddr_ll*)uaddr;
	struct sock *sk=sock->sk;
	struct net_device *dev = NULL;
	int err;

	if (sll->sll_ifindex) {
		err = -ENODEV;
		dev = dev_get_by_index(sock_net(sk), sll->sll_ifindex);
		if (dev == NULL)
			goto out;
	}
	err = packet_do_bind(sk, dev, sll->sll_protocol ? : pkt_sk(sk)->num);
	if (dev)
		dev_put(dev);

out:
	return err;
}

int m_packet_bind(struct packet_sock *po, struct sockaddr *uaddr, int addr_len)
{
    struct socket sock;

    sock.sk = &po->sk;
    return packet_bind(&sock, uaddr, addr_len);
}

extern int m_ieee80211_monitor_sock;
static int packet_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev)
{
	struct sock *sk;
	struct sockaddr_ll *sll;
	struct packet_sock *po;
	u8 * skb_head = skb->data;
	int skb_len = skb->len;
	unsigned int snaplen, res;

	if (skb->pkt_type == PACKET_LOOPBACK)
		goto drop;

	sk = pt->af_packet_priv;
	po = pkt_sk(sk);

#ifdef LINUX
	if (dev_net(dev) != sock_net(sk))
		goto drop;
#endif

	skb->dev = dev;

	if (dev->header_ops) {
		/* The device has an explicit notion of ll header,
		   exported to higher levels.

		   Otherwise, the device hides datails of it frame
		   structure, so that corresponding packet head
		   never delivered to user.
		 */
		if (sk->sk_type != SOCK_DGRAM)
			skb_push(skb, skb->data - skb_mac_header(skb));
		else if (skb->pkt_type == PACKET_OUTGOING) {
			/* Special case: outgoing packets have ll header at head */
			skb_pull(skb, skb_network_offset(skb));
		}
	}

	snaplen = skb->len;

#ifdef LINUX
	res = run_filter(skb, sk, snaplen);
	if (!res)
		goto drop_n_restore;
	if (snaplen > res)
		snaplen = res;

	if (atomic_read(&sk->sk_rmem_alloc) + skb->truesize >=
	    (unsigned)sk->sk_rcvbuf)
		goto drop_n_acct;
#endif

	if (skb_shared(skb)) {
		struct sk_buff *nskb = skb_clone(skb, GFP_ATOMIC);
		if (nskb == NULL)
			goto drop_n_acct;

		if (skb_head != skb->data) {
			skb->data = skb_head;
			skb->len = skb_len;
		}
		kfree_skb(skb);
		skb = nskb;
	}

#ifdef LINUX
	BUILD_BUG_ON(sizeof(*PACKET_SKB_CB(skb)) + MAX_ADDR_LEN - 8 >
		     sizeof(skb->cb));
#endif

	sll = &PACKET_SKB_CB(skb)->sa.ll;
	sll->sll_family = AF_PACKET;
	sll->sll_hatype = dev->type;
	sll->sll_protocol = skb->protocol;
	sll->sll_pkttype = skb->pkt_type;
#ifdef LINUX
	if (unlikely(po->origdev))
		sll->sll_ifindex = orig_dev->ifindex;
	else
#endif
		sll->sll_ifindex = dev->ifindex;

    MP_ASSERT(sll->sll_ifindex == 1 || sll->sll_ifindex == 2);
	sll->sll_halen = dev_parse_header(skb, sll->sll_addr);

	PACKET_SKB_CB(skb)->origlen = skb->len;

	if (pskb_trim(skb, snaplen))
		goto drop_n_acct;

	skb_set_owner_r(skb, sk);
	skb->dev = NULL;
#ifdef LINUX
	dst_release(skb->dst);
	skb->dst = NULL;

	/* drop conntrack reference */
	nf_reset(skb);
#endif

#ifdef LINUX
	spin_lock(&sk->sk_receive_queue.lock);
	po->stats.tp_packets++;
	__skb_queue_tail(&sk->sk_receive_queue, skb);
	spin_unlock(&sk->sk_receive_queue.lock);
	sk->sk_data_ready(sk, skb->len);
#else
    m_ieee80211_monitor_sock = po->u16SockId;
    SockIdSignalRxDone3(po->u16SockId, skb);
#endif
	return 0;

drop_n_acct:
#ifdef LINUX
	spin_lock(&sk->sk_receive_queue.lock);
	po->stats.tp_drops++;
	spin_unlock(&sk->sk_receive_queue.lock);
#endif

drop_n_restore:
	if (skb_head != skb->data && skb_shared(skb)) {
		skb->data = skb_head;
		skb->len = skb_len;
	}
drop:
	kfree_skb(skb);
	return 0;
}

static int packet_create(struct net *net, struct socket *sock, int protocol)
{
	struct sock *sk;
    struct packet_sock *po;

    sk = sock->sk;

    MP_ASSERT(sk);

	sock_init_data(sock, sk);

	sk->sk_family = PF_PACKET;

    po = pkt_sk(sk);

    po->prot_hook.func = packet_rcv;
    po->prot_hook.af_packet_priv = sk;
    if (protocol)
    {
        po->prot_hook.type = protocol;
        dev_add_pack(&po->prot_hook);
		sock_hold(sk);
		po->running = 1;
    }

}

static int packet_sendmsg(struct kiocb *iocb, struct socket *sock,
			  struct msghdr *msg, size_t len)
{
	struct sock *sk = sock->sk;
	struct sockaddr_ll *saddr=(struct sockaddr_ll *)msg->msg_name;
	struct sk_buff *skb;
	struct net_device *dev;
	__be16 proto;
	unsigned char *addr;
	int ifindex, err, reserve = 0;

	/*
	 *	Get and verify the address.
	 */

	if (saddr == NULL) {
		struct packet_sock *po = pkt_sk(sk);

		ifindex	= po->ifindex;
		proto	= po->num;
		addr	= NULL;
	} else {
		err = -EINVAL;
		if (msg->msg_namelen < sizeof(struct sockaddr_ll))
			goto out;
		if (msg->msg_namelen < (saddr->sll_halen + offsetof(struct sockaddr_ll, sll_addr)))
			goto out;
		ifindex	= saddr->sll_ifindex;
		proto	= saddr->sll_protocol;
		addr	= saddr->sll_addr;
	}

//    MP_ASSERT(0);

	dev = dev_get_by_index(sock_net(sk), ifindex);
	err = -ENXIO;
	if (dev == NULL)
		goto out_unlock;
#ifdef LINUX
	if (sock->type == SOCK_RAW)
		reserve = dev->hard_header_len;
#endif

	err = -ENETDOWN;
	if (!(dev->flags & IFF_UP))
		goto out_unlock;

	err = -EMSGSIZE;
#ifdef LINUX
	if (len > dev->mtu+reserve)
		goto out_unlock;
#endif

#ifdef PLATFORM_MPIXEL
#undef LL_ALLOCATED_SPACE
#define LL_ALLOCATED_SPACE(dev) \
	(((ETH_HLEN+(dev)->needed_headroom+(dev)->needed_tailroom)&~(HH_DATA_MOD - 1)) + HH_DATA_MOD)

#undef LL_RESERVED_SPACE
#define LL_RESERVED_SPACE(dev) \
	(((ETH_HLEN+(dev)->needed_headroom)&~(HH_DATA_MOD - 1)) + HH_DATA_MOD)
#endif

	skb = sock_alloc_send_skb(sk, len + LL_ALLOCATED_SPACE(dev),
				msg->msg_flags & MSG_DONTWAIT, &err);
	if (skb==NULL)
		goto out_unlock;

	skb_reserve(skb, LL_RESERVED_SPACE(dev));
	skb_reset_network_header(skb);

	err = -EINVAL;
	if (sock->type == SOCK_DGRAM &&
	    dev_hard_header(skb, dev, ntohs(proto), addr, NULL, len) < 0)
		goto out_free;

	/* Returns -EFAULT on error */
	err = memcpy_fromiovec(skb_put(skb,len), msg->msg_iov, len);
	if (err)
		goto out_free;

	skb->protocol = proto;
	skb->dev = dev;
	skb->priority = sk->sk_priority;

	/*
	 *	Now send it
	 */

	err = dev_queue_xmit(skb);
	if (err > 0 && (err = net_xmit_errno(err)) != 0)
		goto out_unlock;

	dev_put(dev);

	return(len);

out_free:
	kfree_skb(skb);
out_unlock:
	if (dev)
		dev_put(dev);
out:
	return err;
}

int m_packet_create(struct socket *sock, int protocol)
{
    return packet_create(NULL, sock, protocol);
}

int m_packet_sendmsg(struct sock *sk, struct msghdr *msg)
{
    int err = 0, ct;
    struct socket sock;
	struct iovec *iov = msg->msg_iov;
    sock.sk = sk;
    sock.type = sk->sk_type;

	for (ct = 0; ct < msg->msg_iovlen; ct++) {
		err += iov[ct].iov_len;
		/*
		 * Goal is not to verify user data, but to prevent returning
		 * negative value, which is interpreted as errno.
		 * Overflow is still possible, but it is harmless.
		 */
		if (err < 0)
			return -EMSGSIZE;
	}

    return packet_sendmsg(NULL, &sock, msg, err);
}

