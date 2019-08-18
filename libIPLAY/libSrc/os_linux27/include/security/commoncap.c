/* Common capabilities, needed by capability.o and root_plug.o
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 */

#include <linux/capability.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/security.h>
#include <linux/file.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/hugetlb.h>
#include <linux/mount.h>
#include <linux/sched.h>
#include <linux/prctl.h>
#include <linux/securebits.h>

int cap_netlink_send(struct sock *sk, struct sk_buff *skb)
{
#ifdef LINUX
	NETLINK_CB(skb).eff_cap = current->cap_effective;
#endif
	return 0;
}

int cap_netlink_recv(struct sk_buff *skb, int cap)
{
#ifdef LINUX
	if (!cap_raised(NETLINK_CB(skb).eff_cap, cap))
		return -EPERM;
#endif
	return 0;
}

EXPORT_SYMBOL(cap_netlink_recv);

