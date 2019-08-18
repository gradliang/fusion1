/*
 * This file provides /sys/class/ieee80211/<wiphy name>/
 * and some default attributes.
 *
 * Copyright 2005-2006	Jiri Benc <jbenc@suse.cz>
 * Copyright 2006	Johannes Berg <johannes@sipsolutions.net>
 *
 * This file is GPLv2 as found in COPYING.
 */

#include <linux/device.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/nl80211.h>
#include <linux/rtnetlink.h>
#include <net/cfg80211.h>
#include "sysfs.h"
#include "core.h"

int wiphy_sysfs_init(void)
{
#ifdef LINUX
	return class_register(&ieee80211_class);
#else
	return 0;
#endif
}

void wiphy_sysfs_exit(void)
{
#ifdef LINUX
	class_unregister(&ieee80211_class);
#endif
}
