
#ifndef _LINUX_IP_H
#define _LINUX_IP_H

#include <linux/types.h>
#include <asm/byteorder.h>
#include <linux/skbuff.h>
struct iphdr {
	__u8	version:4,
  		ihl:4;
	__u8	tos;
	__be16	tot_len;
	__be16	id;
	__be16	frag_off;
	__u8	ttl;
	__u8	protocol;
	__sum16	check;
	__be32	saddr;
	__be32	daddr;
	/*The options start here. */
};

#ifdef __KERNEL__
#include <linux/skbuff.h>
#include <linux/if_ether.h>

static inline struct iphdr *ip_hdr(const struct sk_buff *skb)
{
	return (struct iphdr *)(skb->data + ETH_HLEN);
}

#endif

#endif	/* _LINUX_IP_H */
