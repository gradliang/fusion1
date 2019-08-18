/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Definitions for the Interfaces handler.
 *
 * Version:	@(#)dev.h	1.0.10	08/12/93
 *
 * Authors:	Ross Biro
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *		Corey Minyard <wf-rch!minyard@relay.EU.net>
 *		Donald J. Becker, <becker@cesdis.gsfc.nasa.gov>
 *		Alan Cox, <Alan.Cox@linux.org>
 *		Bjorn Ekwall. <bj0rn@blox.se>
 *              Pekka Riikonen <priikone@poseidon.pspt.fi>
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 *		Moved to /usr/include/linux for NET3
 */
#ifndef _LINUX_NETDEVICE_H
#define _LINUX_NETDEVICE_H

#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#ifdef __KERNEL__
#include <linux/timer.h>
#include "net_device.h"
//#include <asm/atomic.h>
//#include <asm/cache.h>
//#include <asm/byteorder.h>
#endif
#include <linux/skbuff.h>

#define SET_MODULE_OWNER(dev) do { } while (0)
#define SET_NETDEV_DEV(net, pdev)	((net)->dev.parent = (pdev))

#define	NETDEV_ALIGN		32
#define	NETDEV_ALIGN_CONST	(NETDEV_ALIGN - 1)

struct netdev_queue {
	struct net_device	*dev;
	struct Qdisc		*qdisc;
	unsigned long		state;
	spinlock_t		_xmit_lock;
	int			xmit_lock_owner;
	struct Qdisc		*qdisc_sleeping;
};

struct hh_cache
{

};

struct header_ops {
	int	(*create) (struct sk_buff *skb, struct net_device *dev,
			   unsigned short type, const void *daddr,
			   const void *saddr, unsigned len);
	int	(*parse)(const struct sk_buff *skb, unsigned char *haddr);
	int	(*rebuild)(struct sk_buff *skb);
#define HAVE_HEADER_CACHE
	void	(*cache_update)(struct hh_cache *hh,
				const struct net_device *dev,
				const unsigned char *haddr);
};

static inline void netif_stop_queue(struct net_device *dev)
{
}
static inline int netif_queue_stopped(const struct net_device *dev)
{
#ifdef LINUX
	return test_bit(__LINK_STATE_XOFF, &dev->state);
#else
	return 0;
#endif
}

static inline void netif_start_queue(struct net_device *dev)
{
#ifdef LINUX
	clear_bit(__LINK_STATE_XOFF, &dev->state);
#endif
}

static inline void netif_wake_queue(struct net_device *dev)
{
}
static inline int netif_subqueue_stopped(const struct net_device *dev,
					 struct sk_buff *skb)
{
    return 0;
}
static inline void netif_start_subqueue(struct net_device *dev, u16 queue_index)
{
}
static inline void netif_wake_subqueue(struct net_device *dev, u16 queue_index)
{
}
static inline void netif_tx_lock_bh(struct net_device *dev)
{
}
static inline void netif_tx_unlock_bh(struct net_device *dev)
{
}
static inline void netif_addr_lock(struct net_device *dev)
{
}
static inline void netif_stop_subqueue(struct net_device *dev, u16 queue_index)
{
}
/**
 *	dev_put - release reference to device
 *	@dev: network device
 *
 * Release reference to device to allow it to be freed.
 */
static inline void dev_put(struct net_device *dev)
{
}

extern int netReceive(void *pDev, struct sk_buff * skb);

static inline int netif_rx(struct sk_buff *skb)
{
	skb_push(skb, ETH_HLEN);
    return netReceive(skb->dev, skb);
}

extern void		free_netdev(struct net_device *dev);
extern void		ether_setup(struct net_device *dev);

static inline void netif_tx_start_all_queues(struct net_device *dev)
{
}

/**
 *	netif_subqueue_stopped - test status of subqueue
 *	@dev: network device
 *	@queue_index: sub queue index
 *
 * Check individual transmit queue of a device with multiple transmit queues.
 */
static inline int __netif_subqueue_stopped(const struct net_device *dev,
					 u16 queue_index)
{
	return 1;
}

static inline void netif_tx_stop_all_queues(struct net_device *dev)
{
}

static inline void __netif_tx_unlock_bh(struct netdev_queue *txq)
{
}

static inline void netif_addr_lock_bh(struct net_device *dev)
{
}
static inline void netif_addr_unlock(struct net_device *dev)
{
}
static inline void netif_addr_unlock_bh(struct net_device *dev)
{
}
extern struct net_device *alloc_netdev_mq(int sizeof_priv, const char *name,
				       void (*setup)(struct net_device *),
				       unsigned int queue_count);
extern int		dev_open(struct net_device *dev);
extern int		dev_close(struct net_device *dev);

#endif

