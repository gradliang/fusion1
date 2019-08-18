/**
 * @file
 *
 * Implementation of IOCTLs for network interface.
 *
 * These routines are part of driver APIs that are exported to user 
 * applications.
 *
 * Copyright (c) 2007-2009 Magic Pixel Inc.
 * All rights reserved.
 */

#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"
#include <linux/types.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/rtnetlink.h>
#include "ndebug.h"

#include <string.h>
#include <sys/time.h>
#include "typedef.h"
#include "os.h"
#include "socket.h"
#include "net_nic.h"
#include "SysConfig.h"
#include "net_socket.h"
#include "error.h"
#include "net_netctrl.h"
#include "net_device.h"
//#include "lwip_incl.h"
#include "iw_handler.h"
#include "linux/netdevice.h"
#include "linux/sockios.h"
#include "linux/notifier.h"

#if 0
#define IOCTL_DEBUGF(debug,x) mpDebugPrint x
#else
#define IOCTL_DEBUGF(debug,x) 
#endif

#ifndef MIN
#define MIN(a,b)		((a) < (b) ? (a) : (b))
#endif
/* Set a certain interface flag. 
 *
 * ioctl - SIOCSIFFLAGS
 * flags supported include:
 *  IFF_PROMISC
 *  IFF_ALLMULTI
 */
int if_set_flag(int if_index, short flag)
{
    struct net_device *dev = &NicArray[if_index];
    unsigned int oldflags = dev->flags;

    dev->flags |= flag;

    if (oldflags != dev->flags)
    {
        if (flag & (IFF_ALLMULTI|IFF_PROMISC))
        {
            if (dev->set_multicast_list)
                (*dev->set_multicast_list)(dev);
        }
    }
    return (0);
}

/* Clear a certain interface flag. 
 *
 * ioctl - SIOCSIFFLAGS
 * flags supported include:
 *  IFF_PROMISC
 *  IFF_ALLMULTI
 */
int if_clr_flag(int if_index, short flag)
{
    struct net_device *dev = &NicArray[if_index];
    unsigned int oldflags = dev->flags;

    dev->flags &= ~flag;

    if (oldflags != dev->flags)
    {
        if (flag & (IFF_ALLMULTI|IFF_PROMISC))
        {
            if (dev->set_multicast_list)
                (*dev->set_multicast_list)(dev);
        }
    }
    return (0);
}

/* Add hardware multicast address to device's multicast filter list.
 *
 * ioctl - SIOCADDMULTI;
 *
 * You can enter multiple multicast addresses by calling this routine many 
 * times.  But you have to issue a call like the following:
 *
 *      if_add_mc_address(if_index, NULL, 0);
 *
 * after all addresses are entered.  Then driver will write all entered addresses 
 * to the WLAN card.
 */
int if_add_mc_address(int if_index, struct sockaddr *if_hwaddr, int addrlen)
{
    struct net_device *dev = &NicArray[if_index];
    struct dev_addr_list *new_addr;

    if (if_hwaddr == NULL)
    {
        if (dev->set_multicast_list)
            (*dev->set_multicast_list)(dev);
        return (0);
    }

#ifdef MP600
    new_addr = (struct dev_addr_list *)mpx_Malloc(sizeof(struct dev_addr_list));
#else
    new_addr = netAlloc(sizeof(struct dev_addr_list));
#endif
    if (new_addr == NULL)
        return -1;

    memset(new_addr, 0, sizeof(struct dev_addr_list));

    memcpy(new_addr->da_addr, if_hwaddr->sa_data, addrlen);
    new_addr->da_addrlen = addrlen;

    new_addr->next = dev->mc_list;
    dev->mc_list = new_addr;

    dev->mc_count++;

    return (0);
}

int dev_change_flags(struct net_device *dev, unsigned flags)
{
	int ret, changes;
	int old_flags = dev->flags;

	ASSERT_RTNL();

	/*
	 *	Set the flags on our device.
	 */

#ifdef LINUX
	dev->flags = (flags & (IFF_DEBUG | IFF_NOTRAILERS | IFF_NOARP |
			       IFF_DYNAMIC | IFF_MULTICAST | IFF_PORTSEL |
			       IFF_AUTOMEDIA)) |
		     (dev->flags & (IFF_UP | IFF_VOLATILE | IFF_PROMISC |
				    IFF_ALLMULTI));

	/*
	 *	Load in the correct multicast list now the flags have changed.
	 */

	if ((old_flags ^ flags) & IFF_MULTICAST)
		dev_change_rx_flags(dev, IFF_MULTICAST);

	dev_set_rx_mode(dev);
#endif

	/*
	 *	Have we downed the interface. We handle IFF_UP ourselves
	 *	according to user attempts to set it, rather than blindly
	 *	setting it.
	 */

	ret = 0;
	if ((old_flags ^ flags) & IFF_UP) {	/* Bit is different  ? */
		ret = ((old_flags & IFF_UP) ? dev_close : dev_open)(dev);

#ifdef LINUX
		if (!ret)
			dev_set_rx_mode(dev);
#endif
	}

#ifdef LINUX
	if (dev->flags & IFF_UP &&
	    ((old_flags ^ dev->flags) &~ (IFF_UP | IFF_PROMISC | IFF_ALLMULTI |
					  IFF_VOLATILE)))
		call_netdevice_notifiers(NETDEV_CHANGE, dev);

	if ((flags ^ dev->gflags) & IFF_PROMISC) {
		int inc = (flags & IFF_PROMISC) ? +1 : -1;
		dev->gflags ^= IFF_PROMISC;
		dev_set_promiscuity(dev, inc);
	}

	/* NOTE: order of synchronization of IFF_PROMISC and IFF_ALLMULTI
	   is important. Some (broken) drivers set IFF_PROMISC, when
	   IFF_ALLMULTI is requested not asking us and not reporting.
	 */
	if ((flags ^ dev->gflags) & IFF_ALLMULTI) {
		int inc = (flags & IFF_ALLMULTI) ? +1 : -1;
		dev->gflags ^= IFF_ALLMULTI;
		dev_set_allmulti(dev, inc);
	}

	/* Exclude state transition flags, already notified */
	changes = (old_flags ^ dev->flags) & ~(IFF_UP | IFF_RUNNING);
	if (changes)
		rtmsg_ifinfo(RTM_NEWLINK, dev, changes);        // TODO
#endif

	return ret;
}

/*
 *	Perform the SIOCxIFxxx calls, inside rtnl_lock()
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
static int dev_ifsioc(struct net *net, struct ifreq *ifr, unsigned int cmd)
#else
static int dev_ifsioc(struct ifreq *ifr, unsigned int cmd)
#endif
{
	int err;
	struct net_device *dev = NetDeviceGetByIfname(ifr->ifr_name);

	if (!dev)
    {
        MP_ASSERT(0);
		return -ENODEV;
    }

	switch (cmd) {
		case SIOCSIFFLAGS:	/* Set interface flags */
			return dev_change_flags(dev, ifr->ifr_flags);

		case SIOCSIFHWADDR:
#ifdef LINUX
			return dev_set_mac_address(dev, &ifr->ifr_hwaddr);
#else
            MP_ASSERT(0);
#endif

		case SIOCSIFHWBROADCAST:
			if (ifr->ifr_hwaddr.sa_family != dev->type)
				return -EINVAL;
			memcpy(dev->broadcast, ifr->ifr_hwaddr.sa_data,
			       min(sizeof ifr->ifr_hwaddr.sa_data, (size_t) dev->addr_len));
			call_netdevice_notifiers(NETDEV_CHANGEADDR, dev);
			return 0;

		/*
		 *	Unknown or private ioctl
		 */

		default:
			if ((cmd >= SIOCDEVPRIVATE &&
			    cmd <= SIOCDEVPRIVATE + 15) ||
			    cmd == SIOCBONDENSLAVE ||
			    cmd == SIOCBONDRELEASE ||
			    cmd == SIOCBONDSETHWADDR ||
			    cmd == SIOCBONDSLAVEINFOQUERY ||
			    cmd == SIOCBONDINFOQUERY ||
			    cmd == SIOCBONDCHANGEACTIVE ||
			    cmd == SIOCGMIIPHY ||
			    cmd == SIOCGMIIREG ||
			    cmd == SIOCSMIIREG ||
			    cmd == SIOCBRADDIF ||
			    cmd == SIOCBRDELIF ||
			    cmd == SIOCWANDEV) {
				err = -EOPNOTSUPP;
				if (dev->do_ioctl) {
					if (netif_device_present(dev))
						err = dev->do_ioctl(dev, ifr,
								    cmd);
					else
						err = -ENODEV;
				}
			} else
				err = -EINVAL;

	}
	return err;
}

/**
 *	dev_ioctl	-	network device ioctl
 *	@cmd: command to issue
 *	@arg: pointer to a struct ifreq in user space
 *
 *	Issue ioctl functions to devices. This is normally called by the
 *	user space syscall interfaces but can sometimes be useful for
 *	other purposes. The return value is the return from the syscall if
 *	positive or a negative errno code on error.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
int dev_ioctl(struct net *net, unsigned int cmd, void __user *arg)
#else
int dev_ioctl(unsigned int cmd, void __user *arg)
#endif
{
    int ret;
	struct ifreq *ifr = (struct ifreq *)arg;
#ifndef MP600
    struct net_device *dev = &NicArray[NetDefaultNicGet()];
#else
    struct net_device *dev = NetDeviceGetByIfname(ifr->ifr_name);
#endif
    struct dev_addr_list *new_addr;


	ifr->ifr_name[IFNAMSIZ-1] = 0;

    IOCTL_DEBUGF(0, ("dev_ioctl: cmd=0x%x,name=%s", cmd, ifr->ifr_name));

	/*
	 *	See which interface the caller is talking about.
	 */

	switch (cmd) {
		/*
		 *	These ioctl calls:
		 *	- can be done by all.
		 *	- return a value
		 */
		case SIOCGIFFLAGS:	/* Get interface flags */
			ifr->ifr_flags = dev_get_flags(dev);
			return 0;
		case SIOCGIFINDEX:
			ifr->ifr_ifindex = NetDefaultNicGet();
			return 0;
		case SIOCGIFHWADDR:
            memcpy(ifr->ifr_hwaddr.sa_data, dev->dev_addr,
				       MIN(sizeof ifr->ifr_hwaddr.sa_data, (size_t) 6));
			ifr->ifr_hwaddr.sa_family = ARPHRD_ETHER;
			return 0;

#if 0
		/*
		 *	These ioctl calls:
		 *	- require superuser power.
		 *	- require strict serialization.
		 *	- do not return a value
		 */
		case SIOCSIFFLAGS:
		case SIOCSIFHWADDR:
		case SIOCADDMULTI:
		case SIOCDELMULTI:
			/* fall through */
			ret = dev_ifsioc(&ifr, cmd);
			return ret;
#else
		case SIOCSIFFLAGS:
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
			ret = dev_ifsioc(NULL, ifr, cmd);
#else
			ret = dev_ifsioc(ifr, cmd);
#endif
			return ret;
#endif
		case SIOCADDMULTI:
			dev = &NicArray[NIC_INDEX_WIFI];
			dev->mc_count = 1;
			memcpy(new_addr->da_addr,arg,6); 
			new_addr->da_addrlen = 6;
			dev->mc_list = new_addr;
            if (dev->set_multicast_list)
                (*dev->set_multicast_list)(dev);
			return 0;
			
		case SIOCDELMULTI:
			dev = &NicArray[NIC_INDEX_WIFI];
			dev->mc_count = 0;
			new_addr->da_addrlen = 0;
			dev->mc_list = NULL;
			if (dev->set_multicast_list)
				(*dev->set_multicast_list)(dev);
			return 0;


		/*
		 *	Unknown or private ioctl.
		 */
		default:
			return -EINVAL;
	}
}
#if Make_USB == AR2524_WIFI||Make_USB == RALINK_AR2524_WIFI
int wpa_4way;
#define ZD_IOCTL_WPA			(SIOCDEVPRIVATE + 1)
extern struct net_device *g_dev;
extern BYTE wifi_device_type;
void mp_do_ioctl(struct net_device *dev, struct ifreq *ifr, u32 cmd);
#endif

/**
 * @ingroup WirelessExtension
 * Ioctl interface to wireless device driver.
 *
 * This is an implementation of Linux ioctl() to configure wireless device.
 *
 * @param  fd    a file descriptor (not used)
 * @param  cmd   a request number
 * @param  arg   either an integer value or a pointer to data
 * @retval  0    successful
 * @retval  -1   failed; check <b>errno</b> for error code.
 */
int ioctl(int fd, unsigned int cmd, unsigned long arg)
{
	struct iwreq *iwrp;
    struct ifreq *ifr = (struct ifreq *)arg;
    struct net_device *dev;
    char *extra = NULL;
    iw_handler func = NULL;
    unsigned int index;
	int ret;

    IOCTL_DEBUGF(0, ("ioctl: cmd=0x%x", cmd));

    dev = NetDeviceGetByIfname(ifr->ifr_name);
    if (!dev)
    {
        IOCTL_DEBUGF(0, ("ioctl: invalid if=%s", iwrp->ifr_ifrn.ifrn_name));
	if(cmd != SIOCADDMULTI)//For UPNP set multicase
	{
            errno = ENODEV;
            return -1;
        }
    }

    if (cmd < SIOCGIFNAME)
	{
        errno = EOPNOTSUPP;
        IOCTL_DEBUGF(0, ("ioctl: EOPNOTSUPP"));
        return -1;
	}

    if (cmd < SIOCDEVPRIVATE)                   /* Support network device ioctl */
    {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
        ret = dev_ioctl(NULL, cmd, (void *)arg);
#else
        ret = dev_ioctl(cmd, (void *)arg);
#endif
		if (ret < 0)
		{
			errno = -ret;
			ret = -1;
			IOCTL_DEBUGF(0, ("ioctl: dev_ioctl errno=%d", errno));
		}
        return ret;
    }
#if Make_USB == AR2524_WIFI||Make_USB == RALINK_AR2524_WIFI
	if(wifi_device_type == WIFI_USB_DEVICE_AR2524)
	{
    if (cmd < 0x8A00)                           /* Support network device private ioctl */
    {
        MP_ASSERT(g_dev);
        if (wpa_4way)
        {
            dev = g_dev;
            mp_do_ioctl(dev, (struct ifreq *)arg, cmd);
            return 0;
        }
        else
        {
            if (g_dev)
            {
                dev = g_dev;
                ret = dev->do_ioctl(dev, (struct ifreq *)arg, cmd);
                if (ret < 0)
                {
                    errno = -ret;
                    ret = -1;
					IOCTL_DEBUGF(0, ("ioctl: dev->do_ioctl errno=%d", errno));
                }
            }
            else
            {
                errno = EOPNOTSUPP;
				IOCTL_DEBUGF(0, ("ioctl: EOPNOTSUPP - g_dev=NULL"));
                ret = -1;
            }
            return ret;
        }
    }
	}
#endif	

    if (cmd < SIOCIWFIRST)                      /* Support Linux Wireless Extension */
	{
        errno = EOPNOTSUPP;
		IOCTL_DEBUGF(0, ("ioctl: cmd < SIOCIWFIRST"));
        return -1;
	}

	/* Check if we have some wireless handlers defined */
	if (dev->wireless_handlers == NULL)
	{
		errno = EOPNOTSUPP;
		IOCTL_DEBUGF(0, ("ioctl: dev->wireless_handlers == NULL"));
		return -1;
	}

	/* Try as a standard command */
    index = cmd - SIOCIWFIRST;
	if (index < dev->wireless_handlers->num_standard)
		func = dev->wireless_handlers->standard[index];

    if (func)
    {
        iwrp = (struct iwreq *)arg;
        ret = (*func)(dev, NULL, &iwrp->u, iwrp->u.essid.pointer); /* TODO */
		if (ret < 0)
		{
			errno = -ret;
			ret = -1;
			IOCTL_DEBUGF(0, ("ioctl: standard WEXT (%p) errno=%d", func, errno));
		}
        return ret;
    }

	/* Try as a private command */
	index = cmd - SIOCIWFIRSTPRIV;
	if (index < dev->wireless_handlers->num_private)
		func = dev->wireless_handlers->private[index];

    if (func)
    {
        iwrp = (struct iwreq *)arg;
        ret = (*func)(dev, NULL, &iwrp->u, iwrp->u.essid.pointer); /* TODO */
		if (ret < 0)
		{
			errno = -ret;
			ret = -1;
			IOCTL_DEBUGF(0, ("ioctl: private WEXT errno=%d", errno));
		}
        return ret;
    }

	/* Old driver API : call driver ioctl handler */
    if (dev->do_ioctl)
	{
		ret = dev->do_ioctl(dev, (struct ifreq *)arg, cmd);
		if (ret < 0)
		{
			errno = -ret;
			ret = -1;
			IOCTL_DEBUGF(0, ("ioctl: dev->do_ioctl errno=%d", errno));
		}
        return ret;
	}

	errno = EOPNOTSUPP;
	IOCTL_DEBUGF(0, ("ioctl: Not supported"));
    return -1;
}

struct ioctl_req {
	struct net_device *dev;
	struct ifreq ifr;
    u32 cmd;

	/* for todo list */
	struct list_head todo;
};

#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
static LIST_HEAD(todo_list);
static DEFINE_SPINLOCK(todo_lock);
void ioctl_todo(void);
static void key_todo(struct work_struct *work)
{
	ioctl_todo();
}

static DECLARE_WORK(todo_work, key_todo);

#if Make_USB == AR2524_WIFI||Make_USB == RALINK_AR2524_WIFI
static void add_todo2(struct ioctl_req *key)
{
    MP_DEBUG("%s", __func__);
	if (!key)
		return;

#ifdef PLATFORM_MPIXEL
    if (todo_lock == 0)
    {
		int ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
		MP_ASSERT(ret > 0);
		if (ret > 0)
			todo_lock = ret;
    }
#endif

	spin_lock(&todo_lock);
	/*
	 * Remove again if already on the list so that we move it to the end.
	 */
	if (!list_empty(&key->todo))
		list_del(&key->todo);
	list_add_tail(&key->todo, &todo_list);
	schedule_work(&todo_work);
	spin_unlock(&todo_lock);
}
#endif
void ioctl_todo(void)
{
	struct ioctl_req *key;
    struct net_device *dev;
    int ret;
    u32 cmd;

    MP_DEBUG("%s", __func__);
	spin_lock(&todo_lock);
	while (!list_empty(&todo_list)) {
		key = list_first_entry(&todo_list, struct ioctl_req, todo);
		list_del_init(&key->todo);
		spin_unlock(&todo_lock);

#if USB_WIFI 
		if(wifi_device_type == WIFI_USB_DEVICE_AR2524)
		{

        cmd = key->cmd;
        if (cmd < 0x8A00)                           /* Support network device private ioctl */
        {
            dev = key->dev;
            ret = dev->do_ioctl(dev, &key->ifr, cmd);
            if (ret < 0)
            {
                errno = -ret;
                ret = -1;
            }
        }
		}
#endif
        if (key->ifr.ifr_data)
            mpx_Free(key->ifr.ifr_data);
        mpx_Free(key);

		spin_lock(&todo_lock);
	}
	spin_unlock(&todo_lock);

}

#if Make_USB == AR2524_WIFI||Make_USB == RALINK_AR2524_WIFI
int sizeof_zydas_wlan_param(void);
void mp_do_ioctl(struct net_device *dev, struct ifreq *ifr, u32 cmd)
{
    struct ioctl_req *key;
    int sz = sizeof_zydas_wlan_param();

    MP_DEBUG("%s", __func__);
    key = mpx_Malloc(sizeof(*key));
    if (!key)
        return;

	key->ifr = *ifr;
    if (ifr->ifr_data)
    {
        key->ifr.ifr_data = mpx_Malloc(sz);
        if (!key->ifr.ifr_data)
        {
            mpx_Free(key);
            return;
        }
        memcpy(key->ifr.ifr_data, ifr->ifr_data, sz);
    }
	key->cmd = cmd;
	key->dev = dev;
    INIT_LIST_HEAD(&key->todo);

    add_todo2(key);
}
#endif
