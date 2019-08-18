#define LOCAL_DEBUG_ENABLE 1

#include <sys/time.h>
#include "typedef.h"
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
#include <net/wireless.h>
#include <net/net_namespace.h>

#include "UtilTypeDef.h"
#include "mpTrace.h"
#include "os_mp52x.h"
#include "net_nic.h"

#if Make_USB == AR2524_WIFI||Make_USB == RALINK_AR2524_WIFI

extern struct net_device NicArray[NIC_DRIVER_MAX];
extern struct net_device g_net_device2;
extern struct net_device   *netdev_global;
extern DWORD TickCount;
/*
 * All net warning printk()s should be guarded by this function.
 */
int net_ratelimit(void)
{
#if !Make_ADHOC
    MP_ALERT("net_ratelimit: printk: messages suppressed.");
#endif
	return 1;
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
    struct net_device *dev;
    dev = &NicArray[NIC_INDEX_WIFI];

    if (dev->priv)
    {
        mpx_Free(dev->priv);
        dev->priv = NULL;
    }
    dev->flags = 0;
	
	dev = &g_net_device2;
    if (dev->priv)
    {
    	//mpDebugPrint("free_netdev dev->priv %x",dev->priv);
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

static void netdev_init_queues(struct net_device *dev)
{
}
static struct net_device_stats *internal_stats(struct net_device *dev)
{
	return &dev->stats;
}


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
u16 ieee80211_select_queue(struct net_device *dev, struct sk_buff *skb)
{
	return 0;
}
int register_netdevice(struct net_device *dev)
{
	mpDebugPrint("%s: dev name=%s", __func__, dev->name);
	set_bit(__LINK_STATE_PRESENT, &dev->state);
    netdev_global = dev;
	return 0;
}
void unregister_netdevice(struct net_device *dev)
{
	mpDebugPrint("%s: dev name=%s", __func__, dev->name);

	/* If device is running, close it first. */
	dev_close(dev);

	if (strcmp(dev->name, "wlan0") == 0)
		netdev_global = NULL;
}
void wiphy_unregister(struct wiphy *wiphy)
{
}
void wiphy_free(struct wiphy *wiphy)
{
	kfree(wiphy);
}
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

#ifdef LINUX
    setup(dev);
#endif
	snprintf(dev->name, IFNAMSIZ, name, 0);
	mpDebugPrint("alloc_netdev_mq: n=%s,dev=%p", dev->name, dev);
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
    MP_ASSERT(netdev_global);
    return netdev_global;
}
struct net_device *__dev_get_by_index(struct net *net, int ifindex)
{
    return dev_get_by_index(net, ifindex);
}
/**
 * kref_init - initialize object.
 * @kref: object in question.
 */
void kref_init(struct kref *kref)
{
    kref->refcount.counter = 1;
}
/**
 * kref_get - increment refcount for object.
 * @kref: object.
 */
void kref_get(struct kref *kref)
{
    SemaphoreWait(atomic_sema);
    kref->refcount.counter++;
    SemaphoreRelease(atomic_sema);
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
    SemaphoreWait(atomic_sema);
    int cnt = --kref->refcount.counter;
    SemaphoreRelease(atomic_sema);

    if (cnt == 0)
    {
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

#endif

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
    }
	return ret;
}
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

/**
 *	call_netdevice_notifiers - call all network notifier blocks
 *      @val: value passed unmodified to notifier function
 *      @v:   pointer passed unmodified to notifier function
 *
 *	Call all network notifier blocks.  Parameters and return value
 *	are as for raw_notifier_call_chain().
 */

int call_netdevice_notifiers(unsigned long val, void *v)
{
#ifdef LINUX
	return raw_notifier_call_chain(&netdev_chain, val, v);
#else
	return 0;
#endif
}


/**
 * usb_get_urb - increments the reference count of the urb
 * @urb: pointer to the urb to modify, may be NULL
 *
 * This must be  called whenever a urb is transferred from a device driver to a
 * host controller driver.  This allows proper reference counting to happen
 * for urbs.
 *
 * A pointer to the urb with the incremented reference counter is returned.
 */
struct urb * usb_get_urb(struct urb *urb)
{
#ifdef LINUX
	if (urb)
		kref_get(&urb->kref);
#endif
	return urb;
}

/**
 * usb_unanchor_urb - unanchors an URB
 * @urb: pointer to the urb to anchor
 *
 * Call this to stop the system keeping track of this URB
 */
void usb_unanchor_urb(struct urb *urb)
{
}

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
}
