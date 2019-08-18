#undef LOCAL_DEBUG_ENABLE
#define LOCAL_DEBUG_ENABLE 1

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
//#include <net/wireless.h>
#include <net/net_namespace.h>

#include "UtilTypeDef.h"
#include "mpTrace.h"
#include "os_mp52x.h"
//#include "net_nic.h"

extern struct net_device g_net_device2;
extern struct net_device   *netdev_global;

/**
 * usb_get_dev - increments the reference count of the usb device structure
 * @dev: the device being referenced
 */
struct usb_device *usb_get_dev(struct usb_device *dev)
{
	return dev;
}
EXPORT_SYMBOL_GPL(usb_get_dev);

/**
 * usb_ifnum_to_if - get the interface object with a given interface number
 * @dev: the device whose current configuration is considered
 * @ifnum: the desired interface
 *
 * This walks the device descriptor for the currently active configuration
 * and returns a pointer to the interface with that particular interface
 * number, or null.
 *
 * Note that configuration descriptors are not required to assign interface
 * numbers sequentially, so that it would be incorrect to assume that
 * the first interface in that descriptor corresponds to interface zero.
 * This routine helps device drivers avoid such mistakes.
 * However, you should make sure that you do the right thing with any
 * alternate settings available for this interfaces.
 *
 * Don't call this function unless you are bound to one of the interfaces
 * on this device or you have locked the device!
 */
struct usb_interface *usb_ifnum_to_if(const struct usb_device *dev,
				      unsigned ifnum)
{
#ifdef LINUX
	struct usb_host_config *config = dev->actconfig;
	int i;

	if (!config)
		return NULL;
	for (i = 0; i < config->desc.bNumInterfaces; i++)
		if (config->interface[i]->altsetting[0]
				.desc.bInterfaceNumber == ifnum)
			return config->interface[i];

#else
    MP_ASSERT(ifnum == 0);
	return dev->intf;
#endif
	return NULL;
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
struct urb *usb_get_urb(struct urb *urb)
{
	if (urb)
		kref_get(&urb->kref);
	return urb;
}
EXPORT_SYMBOL_GPL(usb_get_urb);
