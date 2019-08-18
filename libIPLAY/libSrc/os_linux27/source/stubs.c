#undef LOCAL_DEBUG_ENABLE
#define LOCAL_DEBUG_ENABLE 0

#include <sys/time.h>
//#include "typedef.h"
#include "ndebug.h"
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/if_packet.h>
#include <linux/mutex.h>
#include <linux/kref.h>
#include <linux/interrupt.h>
#include <linux/usb.h>
#include <linux/mempool.h>
#include <linux/net.h>
#include <net/dst.h>
#include <net/arp.h>
#include <net/sock.h>
#include <net/ipv6.h>
#include <net/ip.h>
#include <net/cfg80211.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,31))
//#include <net/wireless.h>
#endif
#include <net/net_namespace.h>

#include "UtilTypeDef.h"
#include "mpTrace.h"
#include "os_mp52x.h"
//#include "net_nic.h"
#include "net_const.h"

#define NIC_INDEX_WIFI          2       /* copy from lwip/include/net_nic.h */
#define NIC_DRIVER_MAX          4       /* copy from lwip/include/net_nic.h */

int wifi_event_create = 1;

extern struct net_device NicArray[NIC_DRIVER_MAX];
struct net_device g_net_device2;
extern struct net_device   *netdev_global;
/**
 *	dev_close - shutdown an interface.
 *	@dev: device to shutdown
 *
 *	This function moves an active device into down state. A
 *	%NETDEV_GOING_DOWN is sent to the netdev notifier chain. The device
 *	is then deactivated and finally a %NETDEV_DOWN is sent to the notifier
 *	chain.
 */
int dev_close(struct net_device *dev)
{
	if (!(dev->flags & IFF_UP))
		return 0;

	clear_bit(__LINK_STATE_START, &dev->state);

	/*
	 *	Call the device specific close. This cannot fail.
	 *	Only if device is UP
	 *
	 *	We allow it to be called even after a DETACH hot-plug
	 *	event.
	 */
	if (dev->stop)
		dev->stop(dev);

	/*
	 *	Device is now down.
	 */

	dev->flags &= ~IFF_UP;

    return 0;
}

/*
 * All net warning printk()s should be guarded by this function.
 */
int net_ratelimit(void)
{
#if !Make_Adhoc
    MP_ALERT("net_ratelimit: printk: messages suppressed.");
#endif
	return 1;
}

int dev_open(struct net_device *dev)
{
	int ret = 0;

	if (dev->flags & IFF_UP)
		return 0;

	/*
	 *	Is it even present?
	 */
	if (!netif_device_present(dev))
		return -ENODEV;

	/*
	 *	Call device private open method
	 */
	set_bit(__LINK_STATE_START, &dev->state);

    if (dev->open)
        ret = dev->open(dev);

	/*
	 *	If it went open OK then:
	 */

	if (ret)
		clear_bit(__LINK_STATE_START, &dev->state);
    else
    {
		/*
		 *	Set the flags.
		 */
        dev->flags |= IFF_UP;
        rfc2863_policy(dev);

		/*
		 *	... and announce new interface.
		 */
		call_netdevice_notifiers(NETDEV_UP, dev);
    }
	return ret;
}
int dev_mc_sync(struct net_device *to, struct net_device *from)
{
	MP_DEBUG("%s: to=%s,frm=%s", __func__, to->name, from->name);
    return 0;
}
void dev_mc_unsync(struct net_device *to, struct net_device *from)
{
	MP_DEBUG("%s: to=%s,frm=%s", __func__, to->name, from->name);
}

/**
 *	free_netdev - free network device
 *	@dev: device
 *
 * We don't use dynamic memory for dev, so don't free dev itself.
 */
void free_netdev(struct net_device *dev)
{
    if (dev->priv)
    {
        mpx_Free(dev->priv);
        dev->priv = NULL;
    }
    dev->flags = 0;
}
/**
 *	skb_trim - remove end from a buffer
 *	@skb: buffer to alter
 *	@len: new length
 *
 *	Cut the length of a buffer down by removing data from the tail. If
 *	the buffer is already under the length specified it is not modified.
 *	The skb must be linear.
 */
void skb_trim(struct sk_buff *skb, unsigned int len)
{
	if (skb->len > len)
		__skb_trim(skb, len);
}
int dev_queue_xmit(struct sk_buff *skb)
{
	struct net_device *dev = skb->dev;
	int rc = -ENOMEM;

    MP_ASSERT(dev);
	if (dev->flags & IFF_UP) {
        rc = 0;
		return dev->hard_start_xmit(skb, dev);
    }
	rc = -ENETDOWN;
	kfree_skb(skb);
	return rc;
}
struct sk_buff *skb_clone(struct sk_buff *skb, gfp_t gfp_mask)
{
    MP_ASSERT(0);
    return NULL;
}

void skb_unlink(struct sk_buff *skb, struct sk_buff_head *list)
{
	unsigned long flags;

	spin_lock_irqsave(&list->lock, flags);
	__skb_unlink(skb, list);
	spin_unlock_irqrestore(&list->lock, flags);
}

#if 0
struct wiphy *wiphy_new(struct cfg80211_ops *ops, int sizeof_priv)
{
	int alloc_size;
	struct wiphy *wiphy;

	alloc_size = sizeof(struct wiphy) + sizeof_priv;

	wiphy = kzalloc(alloc_size, GFP_KERNEL);
	if (!wiphy)
		return NULL;

#ifdef LINUX
	drv->ops = ops;


	device_initialize(&drv->wiphy.dev);
	drv->wiphy.dev.class = &ieee80211_class;
	drv->wiphy.dev.platform_data = drv;
#endif

	return wiphy;
}
int wiphy_register(struct wiphy *wiphy)
{
    return 0;
}
#endif

static void netdev_init_queues(struct net_device *dev)
{
}
static struct net_device_stats *internal_stats(struct net_device *dev)
{
	return &dev->stats;
}

#ifndef HAVE_AF_NETLINK
static DEFINE_MUTEX(rtnl_mutex);
void rtnl_lock(void)
{
}
void rtnl_unlock(void)
{
}

void __rtnl_unlock(void)
{
	mutex_unlock(&rtnl_mutex);
}
int rtnl_is_locked(void)
{
	return 1;
}
#endif
int register_netdevice(struct net_device *dev)
{
    int ret;
	mpDebugPrint("%s: dev name=%s", __func__, dev->name);
//        __asm("break 100");
	set_bit(__LINK_STATE_PRESENT, &dev->state);
    if (strncmp(dev->name, "wlan", 4) == 0)
        netdev_global = dev;
#if (Make_USB == AR9271_WIFI)
    else if (strncmp(dev->name, "mon.wlan", 8) == 0)
        dev->ifindex = 1;
#endif

	/* Notify protocols, that a new device appeared. */
	ret = call_netdevice_notifiers(NETDEV_REGISTER, dev);
	return 0;
}

/**
 *      register_netdev - register a network device
 *      @dev: device to register
 *
 *      Take a completed network device structure and add it to the kernel
 *      interfaces. A %NETDEV_REGISTER message is sent to the netdev notifier
 *      chain. 0 is returned on success. A negative errno code is returned
 *      on a failure to set up the device, or if the name is a duplicate.
 *
 *      This is a wrapper around register_netdevice that takes the rtnl semaphore
 *      and expands the device name if you passed a format string to
 *      alloc_netdev.
 */
int register_netdev(struct net_device *dev)
{
        int err;

        rtnl_lock();

        /*
         * If the name is a format string the caller wants us to do a
         * name allocation.
         */
        if (strchr(dev->name, '%')) {
                 err = dev_alloc_name(dev, dev->name);
                if (err < 0)
                        goto out;
        }

        err = register_netdevice(dev);
out:
        rtnl_unlock();
        return err;
}
EXPORT_SYMBOL(register_netdev);

void unregister_netdevice(struct net_device *dev)
{
	mpDebugPrint("%s: dev name=%s", __func__, dev->name);

	/* If device is running, close it first. */
	dev_close(dev);

	if (strcmp(dev->name, "wlan0") == 0)
		netdev_global = NULL;
}

/**
 *      unregister_netdev - remove device from the kernel
 *      @dev: device
 *
 *      This function shuts down a device interface and removes it
 *      from the kernel tables.
 *
 *      This is just a wrapper for unregister_netdevice that takes
 *      the rtnl semaphore.  In general you want to use this and not
 *      unregister_netdevice.
 */
void unregister_netdev(struct net_device *dev)
{
        rtnl_lock();
        unregister_netdevice(dev);
        rtnl_unlock();
}
EXPORT_SYMBOL(unregister_netdev);

#if 0
void wiphy_free(struct wiphy *wiphy)
{
	kfree(wiphy);
}
#endif
/**
 *	alloc_netdev_mq - allocate network device
 *	@sizeof_priv:	size of private data to allocate space for
 *	@name:		device name format string
 *	@setup:		callback to initialize device
 *	@queue_count:	the number of subqueues to allocate
 *
 *	Allocates a struct net_device with private data area for driver use
 *	and performs basic initialization.  Also allocates subquue structs
 *	for each queue on the device at the end of the netdevice.
 */
struct net_device *alloc_netdev_mq(int sizeof_priv, const char *name,
		void (*setup)(struct net_device *), unsigned int queue_count)
{
    struct net_device *dev;
//    MP_FUNCTION_ENTER();
//	mpDebugPrint("alloc_netdev_mq: name=%s", name);
    if (strncmp(name, "wlan", 4) != 0)
        dev = &g_net_device2;
    else
        dev = &NicArray[NIC_INDEX_WIFI];
    if (sizeof_priv)
    {
        dev->priv = mpx_Malloc(sizeof_priv);
        memset(dev->priv, 0, sizeof(sizeof_priv));
    }

    setup(dev);

	snprintf(dev->name, IFNAMSIZ, name, 0);
//	mpDebugPrint("alloc_netdev_mq: n=%s,dev=%p", dev->name, dev);
	return dev;
}
/**
 *	dev_alloc_name - allocate a name for a device
 *	@dev: device
 *	@name: name format string
 *
 *	Passed a format string - eg "lt%d" it will try and find a suitable
 *	id. It scans list of devices to build up a free map, then chooses
 *	the first empty slot. The caller must hold the dev_base or rtnl lock
 *	while allocating the name and adding the device in order to avoid
 *	duplicates.
 *	Limited to bits_per_byte * page size devices (ie 32K on most platforms).
 *	Returns the number of the unit assigned or a negative errno code.
 */

int dev_alloc_name(struct net_device *dev, const char *name)
{
    snprintf(dev->name, IFNAMSIZ, name, 0);
	mpDebugPrint("dev_alloc_name: name=%s", dev->name);
    return 0;
}

/**
 *	dev_get_by_index - find a device by its ifindex
 *	@net: the applicable net namespace
 *	@ifindex: index of device
 *
 *	Search for an interface by index. Returns NULL if the device
 *	is not found or a pointer to the device. The device returned has
 *	had a reference added and the pointer is safe until the user calls
 *	dev_put to indicate they have finished with it.
 */

struct net_device *dev_get_by_index(struct net *net, int ifindex)
{
//    MP_ASSERT(netdev_global);
#if (Make_USB == 5)
    if (ifindex == NIC_INDEX_WIFI)
        return netdev_global;
    else if (ifindex == 1)
        return &g_net_device2;
#else
    return netdev_global;
#endif
}
struct net_device *__dev_get_by_index(struct net *net, int ifindex)
{
    return dev_get_by_index(net, ifindex);
}

/* ----------  lib/kref.c --------- */

void kref_set(struct kref *kref, int num)
{
	atomic_set(&kref->refcount, num);
}
/**
 * kref_init - initialize object.
 * @kref: object in question.
 */
void kref_init(struct kref *kref)
{
	kref_set(kref, 1);
}
/**
 * kref_get - increment refcount for object.
 * @kref: object.
 */
void kref_get(struct kref *kref)
{
	atomic_inc(&kref->refcount);
}

/**
 * kref_put - decrement refcount for object.
 * @kref: object.
 * @release: pointer to the function that will clean up the object when the
 *	     last reference to the object is released.
 *	     This pointer is required, and it is not acceptable to pass kfree
 *	     in as this function.
 *
 * Decrement the refcount, and if 0, call release().
 * Return 1 if the object was removed, otherwise return 0.  Beware, if this
 * function returns 0, you still can not count on the kref from remaining in
 * memory.  Only use the return value if you want to see if the kref is now
 * gone, not present.
 */
int kref_put(struct kref *kref, void (*release)(struct kref *kref))
{
	WARN_ON(release == NULL);
	WARN_ON(release == (void (*)(struct kref *))kfree);

	if (atomic_dec_and_test(&kref->refcount)) {
		release(kref);
		return 1;
	}
	return 0;
}
void local_bh_enable(void)
{
}
void local_bh_disable(void)
{
}
void get_random_bytes(void *buf, int nbytes)
{
    memcpy(buf, &TickCount, nbytes);

}

#if 0
int ieee80211_channel_to_frequency(int chan)
{
	if (chan < 14)
		return 2407 + chan * 5;

	if (chan == 14)
		return 2484;

	/* FIXME: 802.11j 17.3.8.3.2 */
	return (chan + 1000) * 5;
}
int ieee80211_frequency_to_channel(int freq)
{
	if (freq == 2484)
		return 14;

	if (freq < 2484)
		return (freq - 2407) / 5;

	/* FIXME: 802.11j 17.3.8.3.2 */
	return freq/5 - 1000;
}
struct ieee80211_channel *__ieee80211_get_channel(struct wiphy *wiphy,
						  int freq)
{
	enum ieee80211_band band;
	struct ieee80211_supported_band *sband;
	int i;

	for (band = 0; band < IEEE80211_NUM_BANDS; band++) {
		sband = wiphy->bands[band];

		if (!sband)
			continue;

		for (i = 0; i < sband->n_channels; i++) {
			if (sband->channels[i].center_freq == freq)
				return &sband->channels[i];
		}
	}

	return NULL;
}
#endif
unsigned long usecs_to_jiffies(const unsigned int u)
{
	return (u + (USEC_PER_SEC / HZ) - 1) / (USEC_PER_SEC / HZ);
}
int arp_find(unsigned char *haddr, struct sk_buff *skb)
{
	struct net_device *dev = skb->dev;
	__be32 paddr;

    MP_ASSERT(0);

	return 1;
}

/* ----------  kernel/softirq.c  ---------- */

#if Make_USB == AR2524_WIFI
void tasklet_init(struct tasklet_struct *t,
		  void (*func)(unsigned long), unsigned long data)
{
    int ret;
    ret = mp_tasklet_task(func, data);
    MP_ASSERT(ret > 0);
    MP_DEBUG("%s: task %d", __func__, ret);
	t->task_id = ret;
    ret = mp_tasklet_event();
    MP_ASSERT(ret > 0);
    MP_DEBUG("tasklet_init evt %d", ret);
	t->event_id = ret;
	t->next = NULL;
	t->state = 0;
	atomic_set(&t->count, 0);
	t->func = func;
	t->data = data;
}
#endif

int _atomic_dec_and_lock(atomic_t *atomic, spinlock_t *lock)
{
	spin_lock(lock);
	if (atomic_dec_and_test(atomic))
		return 1;
	spin_unlock(lock);
	return 0;
}

int usb_unlink_urb(struct urb *urb)
{
    return 0;
}

void usb_free_urb(struct urb *urb)
{
	if (urb) {
        if (atomic_dec_and_test(&urb->kref.refcount)) {
            if (urb->transfer_flags & URB_FREE_BUFFER)
                kfree(urb->transfer_buffer);

            kfree(urb);
        }
    }
}

/* ----------  dev.c  ---------- */

/**
 * netif_device_detach - mark device as removed
 * @dev: network device
 *
 * Mark device as removed from system and therefore no longer available.
 */
void netif_device_detach(struct net_device *dev)
{
	if (test_and_clear_bit(__LINK_STATE_PRESENT, &dev->state) &&
	    netif_running(dev)) {
		netif_stop_queue(dev);
	}
}
EXPORT_SYMBOL(netif_device_detach);

/**
 * netif_device_attach - mark device as attached
 * @dev: network device
 *
 * Mark device as attached from system and restart if needed.
 */
void netif_device_attach(struct net_device *dev)
{
	if (!test_and_set_bit(__LINK_STATE_PRESENT, &dev->state) &&
	    netif_running(dev)) {
		netif_wake_queue(dev);
#ifdef LINUX
		__netdev_watchdog_up(dev);
#endif
	}
}
EXPORT_SYMBOL(netif_device_attach);

/* ----------  kernel/wait.c --------- */

void init_waitqueue_head(wait_queue_head_t *q)
{
#ifdef HAVE_HOSTAPD
    if (TaskGetId() == 21)
    {
        int _id = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        q->lock = (spinlock_t)_id;
    }
    else
#endif
	spin_lock_init(&q->lock);
	INIT_LIST_HEAD(&q->task_list);
}

EXPORT_SYMBOL(init_waitqueue_head);



/* ----------  net/core.c --------- */

struct list_head cfg80211_rdev_list;            /* XXX ? */


/* ----------  lib/hexdump.c --------- */

void print_hex_dump_bytes(const char *prefix_str, int prefix_type,
			const void *buf, size_t len)
{
#ifdef LINUX
	print_hex_dump(KERN_DEBUG, prefix_str, prefix_type, 16, 1,
			buf, len, 1);
#else
	NetPacketDump(buf, len);
#endif
}
EXPORT_SYMBOL(print_hex_dump_bytes);

/* ----------  drivers/usb/core/urb.c --------- */

/**
 * usb_anchor_urb - anchors an URB while it is processed
 * @urb: pointer to the urb to anchor
 * @anchor: pointer to the anchor
 *
 * This can be called to have access to URBs which are to be executed
 * without bothering to track them
 */
void usb_anchor_urb(struct urb *urb, struct usb_anchor *anchor)
{
#if 1
	unsigned long flags;

	spin_lock_irqsave(&anchor->lock, flags);
	usb_get_urb(urb);
	list_add_tail(&urb->anchor_list, &anchor->urb_list);
	urb->anchor = anchor;
	spin_unlock_irqrestore(&anchor->lock, flags);
#else
	BREAK_POINT();
#endif
}

/**
 * usb_unanchor_urb - unanchors an URB
 * @urb: pointer to the urb to anchor
 *
 * Call this to stop the system keeping track of this URB
 */
void usb_unanchor_urb(struct urb *urb)
{
	unsigned long flags;
	struct usb_anchor *anchor;

	if (!urb)
		return;

	anchor = urb->anchor;
	if (!anchor)
		return;

	spin_lock_irqsave(&anchor->lock, flags);
	if (unlikely(anchor != urb->anchor)) {
		/* we've lost the race to another thread */
		spin_unlock_irqrestore(&anchor->lock, flags);
		return;
	}
	urb->anchor = NULL;
	list_del(&urb->anchor_list);
	spin_unlock_irqrestore(&anchor->lock, flags);
	usb_put_urb(urb);
	if (list_empty(&anchor->urb_list))
#ifdef LINUX
		wake_up(&anchor->wait);
#else
		;
#endif
}

/**
 * usb_kill_anchored_urbs - cancel transfer requests en masse
 * @anchor: anchor the requests are bound to
 *
 * this allows all outstanding URBs to be killed starting
 * from the back of the queue
 */
void usb_kill_anchored_urbs(struct usb_anchor *anchor)
{
	struct urb *victim;

	spin_lock_irq(&anchor->lock);
	while (!list_empty(&anchor->urb_list)) {
		victim = list_entry(anchor->urb_list.prev, struct urb,
				    anchor_list);
		/* we must make sure the URB isn't freed before we kill it*/
		usb_get_urb(victim);
		spin_unlock_irq(&anchor->lock);
		/* this will unanchor the URB */
		usb_kill_urb(victim);
		usb_put_urb(victim);
		spin_lock_irq(&anchor->lock);
	}
	spin_unlock_irq(&anchor->lock);
}

/* Trims skb to length len. It can change skb pointers.
 */

int ___pskb_trim(struct sk_buff *skb, unsigned int len)
{
	BREAK_POINT();
}

/* ----------  lib/bitmap.c --------- */

int __bitmap_empty(const unsigned long *bitmap, int bits)
{
	int k, lim = bits/BITS_PER_LONG;
	for (k = 0; k < lim; ++k)
		if (bitmap[k])
			return 0;

	if (bits % BITS_PER_LONG)
		if (bitmap[k] & BITMAP_LAST_WORD_MASK(bits))
			return 0;

	return 1;
}
EXPORT_SYMBOL(__bitmap_empty);

/* ----------  lib/dump_stack.c --------- */

void dump_stack(void)
{
#if LOCAL_DEBUG_ENABLE
	printk(KERN_NOTICE
		"This architecture does not implement dump_stack()\n");
#endif
}

EXPORT_SYMBOL(dump_stack);

/* ----------  kernel/rwsem.c --------- */

/*
 * lock for reading
 */
void __sched down_read(struct rw_semaphore *sem)
{
}

EXPORT_SYMBOL(down_read);
/*
 * lock for writing
 */
void __sched down_write(struct rw_semaphore *sem)
{
}

EXPORT_SYMBOL(down_write);

/*
 * release a read lock on the semaphore
 */
void __up_read(struct rw_semaphore *sem)
{
#ifdef LINUX
	unsigned long flags;

	spin_lock_irqsave(&sem->wait_lock, flags);

	if (--sem->activity == 0 && !list_empty(&sem->wait_list))
		sem = __rwsem_wake_one_writer(sem);

	spin_unlock_irqrestore(&sem->wait_lock, flags);
#endif
}

/*
 * release a write lock on the semaphore
 */
void __up_write(struct rw_semaphore *sem)
{
#ifdef LINUX
	unsigned long flags;

	spin_lock_irqsave(&sem->wait_lock, flags);

	sem->activity = 0;
	if (!list_empty(&sem->wait_list))
		sem = __rwsem_do_wake(sem, 1);

	spin_unlock_irqrestore(&sem->wait_lock, flags);
#endif
}
/*
 * release a read lock
 */
void up_read(struct rw_semaphore *sem)
{
#ifdef LINUX
	rwsem_release(&sem->dep_map, 1, _RET_IP_);

	__up_read(sem);
#else
#endif
}

EXPORT_SYMBOL(up_read);

/*
 * release a write lock
 */
void up_write(struct rw_semaphore *sem)
{
	rwsem_release(&sem->dep_map, 1, _RET_IP_);

	__up_write(sem);
}

EXPORT_SYMBOL(up_write);

/* ----------  drivers/base/platform.c --------- */

struct platform_device {
	const char	* name;
	int		id;
	struct device	dev;
	u32		num_resources;
};

struct platform_object {
	struct platform_device pdev;
	char name[1];
};

/**
 * platform_device_register - add a platform-level device
 * @pdev: platform device we're adding
 */
int platform_device_register(struct platform_device *pdev)
{
#ifdef LINUX
	return platform_device_add(pdev);
#else
	return 0;
#endif
}
EXPORT_SYMBOL_GPL(platform_device_register);

void platform_device_unregister(struct platform_device *pdev)
{
}
EXPORT_SYMBOL_GPL(platform_device_unregister);

struct platform_device *platform_device_alloc(const char *name, int id)
{
	struct platform_object *pa;

	pa = kzalloc(sizeof(struct platform_object) + strlen(name), GFP_KERNEL);
	if (pa) {
	}

	return pa ? &pa->pdev : NULL;
}

EXPORT_SYMBOL_GPL(platform_device_alloc);
struct platform_device *platform_device_register_simple(const char *name,
							int id,
							struct resource *res,
							unsigned int num)
{
	struct platform_device *pdev;
	int retval;

	pdev = platform_device_alloc(name, id);
	if (!pdev) {
		retval = -ENOMEM;
		goto error;
	}

	return pdev;

error:
//	platform_device_put(pdev);
	return ERR_PTR(retval);
}

void *memdup(const void *p, size_t size)
{
	void *p2;
	if (size == 0)
		return NULL;
	p2 = mm_malloc(size);
	if (!p2)
		return NULL;
	memcpy(p2, p, size);
	return p2;
}

/* ----------  drivers/base/platform.c --------- */

void device_initialize(struct device *dev)
{

}

int dev_set_name(struct device *dev, const char *fmt, ...)
{
	va_list vargs;

	va_start(vargs, fmt);
	vsnprintf(dev->bus_id, sizeof(dev->bus_id), fmt, vargs);
	va_end(vargs);
	return 0;
}

/* ----------  net/core/net_namespace.c --------- */

struct net init_net;
EXPORT_SYMBOL(init_net);

/* ----------  drivers/base/platform.c --------- */

int device_add(struct device *dev)
{
    return 0;
}

void device_del(struct device *dev)
{

}

/* ----------  mm/nommu.c  ---------- */
/*
 *	vmalloc  -  allocate virtually continguos memory
 *
 *	@size:		allocation size
 *
 *	Allocate enough pages to cover @size from the page level
 *	allocator and map them into continguos kernel virtual space.
 *
 *	For tight control over page level allocator and protection flags
 *	use __vmalloc() instead.
 */
void *vmalloc(unsigned long size)
{
       return ext_mem_malloc(size);
}
EXPORT_SYMBOL(vmalloc);

void vfree(void *addr)
{
	ext_mem_free(addr);
}


/* ----------  lib/vsprintf.c ----- */
unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base)
{
    return 0;
}

/**
 *	sk_filter - run a packet through a socket filter
 *	@sk: sock associated with &sk_buff
 *	@skb: buffer to filter
 *
 * Run the filter code and then cut skb->data to correct size returned by
 * sk_run_filter. If pkt_len is 0 we toss packet. If skb->len is smaller
 * than pkt_len we keep whole skb->data. This is the socket level
 * wrapper to sk_run_filter. It returns 0 if the packet should
 * be accepted or -EPERM if the packet should be tossed.
 *
 */
int sk_filter(struct sock *sk, struct sk_buff *skb)
{
#ifdef LINUX
	int err;
	struct sk_filter *filter;

	err = security_sock_rcv_skb(sk, skb);
	if (err)
		return err;

	rcu_read_lock_bh();
	filter = rcu_dereference(sk->sk_filter);
	if (filter) {
		unsigned int pkt_len = sk_run_filter(skb, filter->insns,
				filter->len);
		err = pkt_len ? pskb_trim(skb, pkt_len) : -EPERM;
	}
	rcu_read_unlock_bh();

	return err;
#else
	return 0;
#endif
}

/* drivers/base/core.c */

/**
 * put_device - decrement reference count.
 * @dev: device in question.
 */
void put_device(struct device *dev)
{
#ifdef LINUX
	/* might_sleep(); */
	if (dev)
		kobject_put(&dev->kobj);
#endif
}

void sock_release(struct socket *sock)
{
#ifdef LINUX
	if (sock->ops) {
		struct module *owner = sock->ops->owner;

		sock->ops->release(sock);
		sock->ops = NULL;
		module_put(owner);
	}

	if (sock->fasync_list)
		printk(KERN_ERR "sock_release: fasync list not empty!\n");

	get_cpu_var(sockets_in_use)--;
	put_cpu_var(sockets_in_use);
	if (!sock->file) {
		iput(SOCK_INODE(sock));
		return;
	}
	sock->file = NULL;
#else
    MP_ASSERT(0);
#endif
}

/*
 * Last sock_put should drop referrence to sk->sk_net. It has already
 * been dropped in sk_change_net. Taking referrence to stopping namespace
 * is not an option.
 * Take referrence to a socket to remove it from hash _alive_ and after that
 * destroy it in the context of init_net.
 */
void sk_release_kernel(struct sock *sk)
{
	if (sk == NULL || sk->sk_socket == NULL)
		return;

#ifdef LINUX
	sock_hold(sk);
	sock_release(sk->sk_socket);
	release_net(sock_net(sk));
	sock_net_set(sk, get_net(&init_net));
	sock_put(sk);
#else
    MP_ASSERT(0);
#endif
}
EXPORT_SYMBOL(sk_release_kernel);

extern int m_netlink_init(void);
extern int mpx_workqueue_preinit(int num);
extern int m_rtnetlink_net_init(struct net *net);
void m_rcu_init();

spinlock_t cleanup_queue_lock;
spinlock_t cleanup_queue_lock2;

void m_linux_kernel_init(void)
{
    /* init some linux kernel subsystems */

#ifdef LINUX
    spin_lock_init(&cleanup_queue_lock);
    spin_lock_init(&cleanup_queue_lock2);
#else
    cleanup_queue_lock = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
    cleanup_queue_lock2 = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
#endif

    m_net_dev_init();

#ifdef HAVE_AF_NETLINK
    m_netlink_init();
    genl_lock_init();
    m_genl_init();
    m_rtnl_lock_init();
    m_rtnetlink_net_init(NULL);
#endif

    /* rcu_data */
    m_rcu_init();

    mpx_WorkQueueInit(2);
#if CONFIG_CFG80211
    if (mpx_cfg80211_init() < 0)
        MP_ASSERT(0);
#endif

}

/* from kernel/rcuclassic.c */

//#include <linux/rcuclassic.h>
struct rcu_data {
	/* 1) quiescent state handling : */
	int		qs_pending;	 /* core waits for quiesc state */

	/* 2) batch handling */
	long  	       	batch;           /* Batch # for current RCU batch */
	struct rcu_head *nxtlist;
	struct rcu_head **nxttail;
	long            qlen; 	 	 /* # of queued callbacks */
	struct rcu_head *donelist;
	struct rcu_head **donetail;
	long		blimit;		 /* Upper limit on a processed batch */
	int cpu;
};

static int blimit = 10;
static int qhimark = 10000;
static int qlowmark = 100;

static struct rcu_data g_rdp;
BYTE rcu_do_event, rcu_do_sema;
BYTE rcu_do_task_id;

/**
 * call_rcu - Queue an RCU callback for invocation after a grace period.
 * @head: structure to be used for queueing the RCU updates.
 * @func: actual update function to be invoked after the grace period
 *
 * The update function will be invoked some time after a full grace
 * period elapses, in other words after all currently executing RCU
 * read-side critical sections have completed.  RCU read-side critical
 * sections are delimited by rcu_read_lock() and rcu_read_unlock(),
 * and may be nested.
 */
void call_rcu(struct rcu_head *head,
				void (*func)(struct rcu_head *rcu))
{
	unsigned long flags;
	struct rcu_data *rdp;

	head->func = func;
	head->next = NULL;

	rdp = &g_rdp;

    mpx_SemaphoreWait(rcu_do_sema);

	*rdp->nxttail = head;
	rdp->nxttail = &head->next;
	if (unlikely(++rdp->qlen > qhimark)) {
		rdp->blimit = INT_MAX;
#ifdef LINUX
		force_quiescent_state(rdp, &rcu_ctrlblk);
#endif
	}

    mpx_SemaphoreRelease(rcu_do_sema);

    EventSet(rcu_do_event, BIT(0));
}
EXPORT_SYMBOL_GPL(call_rcu);

static void rcu_init_percpu_data(int cpu, struct rcu_ctrlblk *rcp,
						struct rcu_data *rdp)
{
	memset(rdp, 0, sizeof(*rdp));
	rdp->nxttail = &rdp->nxtlist;
	rdp->donetail = &rdp->donelist;
#ifdef LINUX
	rdp->quiescbatch = rcp->completed;
#endif
	rdp->qs_pending = 0;
	rdp->cpu = cpu;
	rdp->blimit = blimit;
}

static void rcu_do_batch(struct rcu_data *rdp)
{
	struct rcu_head *next, *list;
	int count = 0;

	list = rdp->donelist;
	while (list) {
		next = list->next;
		prefetch(next);
		list->func(list);
		list = next;
		if (++count >= rdp->blimit)
			break;
	}
	rdp->donelist = list;

    mpx_SemaphoreWait(rcu_do_sema);
	rdp->qlen -= count;
    mpx_SemaphoreRelease(rcu_do_sema);

	if (rdp->blimit == INT_MAX && rdp->qlen <= qlowmark)
		rdp->blimit = blimit;

	if (!rdp->donelist)
		rdp->donetail = &rdp->donelist;
	else
        EventSet(rcu_do_event, BIT(0));
}

static void rcu_do_task(void)
{
    struct rcu_data *rdp = &g_rdp;
    u32 mask;

    while (1) {
        mpx_EventWait(rcu_do_event, 0x1, OS_EVENT_OR, &mask);

        if (mask & BIT(0))
        {
            if (rdp->nxtlist && !rdp->donelist) {
                mpx_SemaphoreWait(rcu_do_sema);
                rdp->donelist = rdp->nxtlist;
                rdp->donetail = rdp->nxttail;
                rdp->nxtlist = NULL;
                rdp->nxttail = &rdp->nxtlist;
                mpx_SemaphoreRelease(rcu_do_sema);

                /*
                 * start the next batch of callbacks
                 */

#ifdef LINUX
                /* determine batch number */
                rdp->batch = rcp->cur + 1;
#endif
            }

            if (rdp->donelist)
                rcu_do_batch(rdp);
        }

        TaskYield();
    }
}

#ifndef DRIVER_PRIORITY
#define DRIVER_PRIORITY             8
#endif
void m_rcu_init()
{
    rcu_init_percpu_data(0, NULL, &g_rdp);

	rcu_do_event = mpx_EventCreate((OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);
	rcu_do_sema = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);

    rcu_do_task_id = mpx_TaskCreate(rcu_do_task, DRIVER_PRIORITY-2, 0x1000 * 4);
    TaskStartup(rcu_do_task_id);
}

#ifndef HAVE_AF_NETLINK
int netlink_broadcast(struct sock *ssk, struct sk_buff *skb, u32 pid,
		      u32 group, gfp_t allocation)
{
    return -1;
}
int netlink_unicast(struct sock *ssk, struct sk_buff *skb,
		    u32 pid, int nonblock)
{
    return -1;
}
int m_netlink_recvmsg(struct socket *sock,
			   struct msghdr *msg, size_t len,
			   int flags)
{
    return -1;
}
int m_netlink_bind(struct socket *sock, struct sockaddr *addr,
			int addr_len)
{
    return -1;
}
int m_netlink_setsockopt(struct socket *sock, int level, int optname,
			      char __user *optval, int optlen)
{
    return -1;
}
u32 m_netlink_group_mask(u32 group)
{
    return 0;
}
int m_netlink_create(struct socket *sock, int protocol)
{
    return -1;
}
#endif
#ifndef HAVE_AF_PACKET
int m_packet_sendmsg(struct sock *sk, struct msghdr *msg)
{
    return -1;
}
int m_packet_create(struct socket *sock, int protocol)
{
    return -1;
}
int m_packet_bind(struct packet_sock *po, struct sockaddr *uaddr, int addr_len)
{
    return -1;
}
#endif


/* --------------------spinlock related routines ------------------------- */

void m_spin_lock(spinlock_t *lock)
{
    if (*lock)
        SemaphoreWait(*lock);
    /* else do nothing now */
}

void m_spin_unlock(spinlock_t *lock)
{
    if (*lock)
        SemaphoreRelease(*lock);
    /* do nothing now */
}

void m_spin_lock_bh(spinlock_t *lock)
{
    IntDisable();
    return;
}

unsigned long m_spin_lock_irqsave(spinlock_t *lock)
{
    if (*lock)
        SemaphoreWait(*lock);
    else
        IntDisable();
    return 0;
}

void m_spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
    if (*lock)
        SemaphoreRelease(*lock);
    else
        IntEnable();
    return;
}

unsigned long m_spin_lock_irqsave_nested(spinlock_t *lock)
{
    IntDisable();
    MP_ASSERT(0);
    return 0;
}

/* ----------  kernel/signal.c  ---------- */

int kill_pid(struct pid *pid, int sig, int priv)
{
    MP_ASSERT(0);
    return 0;
}

/* ----------  kernel/pid.c  ---------- */

struct pid *find_vpid(int nr)
{
    return NULL;
}

/* ----------    ---------- */

void complete_and_exit(struct completion *comp, long i)
{
}

/* ----------  kernel/exit.c  ---------- */

int allow_signal(int sig)
{
    return 0;
}

void daemonize(const char *name, ...)
{
}

/* ----------  drivers/usb/core/message.c  ---------- */

int usb_clear_halt(struct usb_device *dev, int pipe)
{
    MP_ASSERT(0);
    return 0;
}

/* ----------  kernel/semaphore.c  ---------- */

int down_interruptible(struct semaphore * sem)
{
    int r;
    MP_ASSERT(sem->lock > 0);
    r = SemaphoreWait(sem->lock);
    return r ? (-1) : 0;
}

void up(struct semaphore *sem)
{
    MP_ASSERT(sem->lock > 0);
    SemaphoreRelease(sem->lock);
}

