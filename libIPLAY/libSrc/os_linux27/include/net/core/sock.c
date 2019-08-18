/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Generic socket support routines. Memory allocators, socket lock/release
 *		handler for protocols to use and generic option handler.
 *
 *
 * Authors:	Ross Biro
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *		Florian La Roche, <flla@stud.uni-sb.de>
 *		Alan Cox, <A.Cox@swansea.ac.uk>
 *
 * Fixes:
 *		Alan Cox	: 	Numerous verify_area() problems
 *		Alan Cox	:	Connecting on a connecting socket
 *					now returns an error for tcp.
 *		Alan Cox	:	sock->protocol is set correctly.
 *					and is not sometimes left as 0.
 *		Alan Cox	:	connect handles icmp errors on a
 *					connect properly. Unfortunately there
 *					is a restart syscall nasty there. I
 *					can't match BSD without hacking the C
 *					library. Ideas urgently sought!
 *		Alan Cox	:	Disallow bind() to addresses that are
 *					not ours - especially broadcast ones!!
 *		Alan Cox	:	Socket 1024 _IS_ ok for users. (fencepost)
 *		Alan Cox	:	sock_wfree/sock_rfree don't destroy sockets,
 *					instead they leave that for the DESTROY timer.
 *		Alan Cox	:	Clean up error flag in accept
 *		Alan Cox	:	TCP ack handling is buggy, the DESTROY timer
 *					was buggy. Put a remove_sock() in the handler
 *					for memory when we hit 0. Also altered the timer
 *					code. The ACK stuff can wait and needs major
 *					TCP layer surgery.
 *		Alan Cox	:	Fixed TCP ack bug, removed remove sock
 *					and fixed timer/inet_bh race.
 *		Alan Cox	:	Added zapped flag for TCP
 *		Alan Cox	:	Move kfree_skb into skbuff.c and tidied up surplus code
 *		Alan Cox	:	for new sk_buff allocations wmalloc/rmalloc now call alloc_skb
 *		Alan Cox	:	kfree_s calls now are kfree_skbmem so we can track skb resources
 *		Alan Cox	:	Supports socket option broadcast now as does udp. Packet and raw need fixing.
 *		Alan Cox	:	Added RCVBUF,SNDBUF size setting. It suddenly occurred to me how easy it was so...
 *		Rick Sladkey	:	Relaxed UDP rules for matching packets.
 *		C.E.Hawkins	:	IFF_PROMISC/SIOCGHWADDR support
 *	Pauline Middelink	:	identd support
 *		Alan Cox	:	Fixed connect() taking signals I think.
 *		Alan Cox	:	SO_LINGER supported
 *		Alan Cox	:	Error reporting fixes
 *		Anonymous	:	inet_create tidied up (sk->reuse setting)
 *		Alan Cox	:	inet sockets don't set sk->type!
 *		Alan Cox	:	Split socket option code
 *		Alan Cox	:	Callbacks
 *		Alan Cox	:	Nagle flag for Charles & Johannes stuff
 *		Alex		:	Removed restriction on inet fioctl
 *		Alan Cox	:	Splitting INET from NET core
 *		Alan Cox	:	Fixed bogus SO_TYPE handling in getsockopt()
 *		Adam Caldwell	:	Missing return in SO_DONTROUTE/SO_DEBUG code
 *		Alan Cox	:	Split IP from generic code
 *		Alan Cox	:	New kfree_skbmem()
 *		Alan Cox	:	Make SO_DEBUG superuser only.
 *		Alan Cox	:	Allow anyone to clear SO_DEBUG
 *					(compatibility fix)
 *		Alan Cox	:	Added optimistic memory grabbing for AF_UNIX throughput.
 *		Alan Cox	:	Allocator for a socket is settable.
 *		Alan Cox	:	SO_ERROR includes soft errors.
 *		Alan Cox	:	Allow NULL arguments on some SO_ opts
 *		Alan Cox	: 	Generic socket allocation to make hooks
 *					easier (suggested by Craig Metz).
 *		Michael Pall	:	SO_ERROR returns positive errno again
 *              Steve Whitehouse:       Added default destructor to free
 *                                      protocol private data.
 *              Steve Whitehouse:       Added various other default routines
 *                                      common to several socket families.
 *              Chris Evans     :       Call suser() check last on F_SETOWN
 *		Jay Schulist	:	Added SO_ATTACH_FILTER and SO_DETACH_FILTER.
 *		Andi Kleen	:	Add sock_kmalloc()/sock_kfree_s()
 *		Andi Kleen	:	Fix write_space callback
 *		Chris Evans	:	Security fixes - signedness again
 *		Arnaldo C. Melo :       cleanups, use skb_queue_purge
 *
 * To Fix:
 *
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */

#ifdef PLATFORM_MPIXEL
#define LOCAL_DEBUG_ENABLE 0
#include <mpTrace_copy.h>
#endif

#include <linux/capability.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/string.h>
#include <linux/sockios.h>
#include <linux/net.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/tcp.h>
#include <linux/init.h>
#include <linux/highmem.h>

#include <asm/uaccess.h>
#include <asm/system.h>

#include <linux/netdevice.h>
#include <net/protocol.h>
#include <linux/skbuff.h>
#include <net/net_namespace.h>
#include <net/request_sock.h>
#include <net/sock.h>
#include <net/xfrm.h>
#include <linux/ipsec.h>

#include <linux/filter.h>

#ifdef CONFIG_INET
#include <net/tcp.h>
#endif

/*
 * sk_callback_lock locking rules are per-address-family,
 * so split the lock classes by using a per-AF key:
 */
static struct lock_class_key af_callback_keys[AF_MAX];

/* Take into consideration the size of the struct sk_buff overhead in the
 * determination of these values, since that is non-constant across
 * platforms.  This makes socket queueing behavior and performance
 * not depend upon such differences.
 */
#define _SK_MEM_PACKETS		256
#define _SK_MEM_OVERHEAD	(sizeof(struct sk_buff) + 256)
#define SK_WMEM_MAX		(_SK_MEM_OVERHEAD * _SK_MEM_PACKETS)
#define SK_RMEM_MAX		(_SK_MEM_OVERHEAD * _SK_MEM_PACKETS)

static void sock_def_readable(struct sock *sk, int len)
{
	read_lock(&sk->sk_callback_lock);
#ifdef LINUX
	if (sk->sk_sleep && waitqueue_active(sk->sk_sleep))
		wake_up_interruptible_sync(sk->sk_sleep);
	sk_wake_async(sk, SOCK_WAKE_WAITD, POLL_IN);
#else
	read_unlock(&sk->sk_callback_lock);
#endif
}

#ifdef LINUX
void sk_free(struct sock *sk)
{
#ifdef LINUX
	struct sk_filter *filter;

	if (sk->sk_destruct)
		sk->sk_destruct(sk);

	filter = rcu_dereference(sk->sk_filter);
	if (filter) {
		sk_filter_uncharge(sk, filter);
		rcu_assign_pointer(sk->sk_filter, NULL);
	}

	sock_disable_timestamp(sk);

	if (atomic_read(&sk->sk_omem_alloc))
		printk(KERN_DEBUG "%s: optmem leakage (%d bytes) detected.\n",
		       __func__, atomic_read(&sk->sk_omem_alloc));

	put_net(sock_net(sk));
	sk_prot_free(sk->sk_prot_creator, sk);
#else
    MP_ASSERT(0);
#endif
}
#endif

/*
 *	Simple resource managers for sockets.
 */


/*
 * Write buffer destructor automatically called from kfree_skb.
 */
void sock_wfree(struct sk_buff *skb)
{
	struct sock *sk = skb->sk;

	/* In case it might be waiting for more memory. */
	atomic_sub(skb->truesize, &sk->sk_wmem_alloc);
	if (!sock_flag(sk, SOCK_USE_WRITE_QUEUE))
		sk->sk_write_space(sk);
	sock_put(sk);
}

/*
 * Read buffer destructor automatically called from kfree_skb.
 */
void sock_rfree(struct sk_buff *skb)
{
	struct sock *sk = skb->sk;

#ifdef LINUX
	skb_truesize_check(skb);
	atomic_sub(skb->truesize, &sk->sk_rmem_alloc);
	sk_mem_uncharge(skb->sk, skb->truesize);
#endif
}

extern int m_kernel_generic_semaphore();
void sock_init_data(struct socket *sock, struct sock *sk)
{
#ifdef PLATFORM_MPIXEL
    sk->sk_receive_queue.lock = m_kernel_generic_semaphore();     /* this queue is not really used */
    sk->sk_write_queue.lock = m_kernel_generic_semaphore();     /* this queue is not really used */
    sk->sk_error_queue.lock = m_kernel_generic_semaphore();     /* this queue is not really used */
#endif
	skb_queue_head_init(&sk->sk_receive_queue);
	skb_queue_head_init(&sk->sk_write_queue);
	skb_queue_head_init(&sk->sk_error_queue);
#ifdef CONFIG_NET_DMA
	skb_queue_head_init(&sk->sk_async_wait_queue);
#endif

	sk->sk_send_head	=	NULL;

	init_timer(&sk->sk_timer);

#ifdef LINUX
	sk->sk_allocation	=	GFP_KERNEL;
	sk->sk_rcvbuf		=	sysctl_rmem_default;
	sk->sk_sndbuf		=	sysctl_wmem_default;
	sk->sk_state		=	TCP_CLOSE;
#endif
	sk_set_socket(sk, sock);

	sock_set_flag(sk, SOCK_ZAPPED);

	if (sock) {
		sk->sk_type	=	sock->type;
		sk->sk_sleep	=	&sock->wait;
		sock->sk	=	sk;
	} else
		sk->sk_sleep	=	NULL;

	rwlock_init(&sk->sk_dst_lock);
	rwlock_init(&sk->sk_callback_lock);
	lockdep_set_class_and_name(&sk->sk_callback_lock,
			af_callback_keys + sk->sk_family,
			af_family_clock_key_strings[sk->sk_family]);

#ifdef LINUX
	sk->sk_state_change	=	sock_def_wakeup;
#endif
	sk->sk_data_ready	=	sock_def_readable;
#ifdef LINUX
	sk->sk_write_space	=	sock_def_write_space;
	sk->sk_error_report	=	sock_def_error_report;
	sk->sk_destruct		=	sock_def_destruct;

	sk->sk_sndmsg_page	=	NULL;
	sk->sk_sndmsg_off	=	0;

	sk->sk_peercred.pid 	=	0;
	sk->sk_peercred.uid	=	-1;
	sk->sk_peercred.gid	=	-1;
	sk->sk_write_pending	=	0;
	sk->sk_rcvlowat		=	1;
	sk->sk_rcvtimeo		=	MAX_SCHEDULE_TIMEOUT;
	sk->sk_sndtimeo		=	MAX_SCHEDULE_TIMEOUT;

	sk->sk_stamp = ktime_set(-1L, 0);
#endif

	atomic_set(&sk->sk_refcnt, 1);
#ifdef LINUX
	atomic_set(&sk->sk_drops, 0);
#endif
}

struct sk_buff *sock_alloc_send_skb(struct sock *sk, unsigned long size,
				    int noblock, int *errcode)
{
#ifdef LINUX
	return sock_alloc_send_pskb(sk, size, 0, noblock, errcode);
#else
    struct sk_buff *skb;

    skb = alloc_skb(size, 0);
    if (skb)
    {
        skb_set_owner_w(skb, sk);
    }
    return skb;
#endif
}

