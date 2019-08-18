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
#include <linux/leds.h>
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

/* ----------  drivers/leds/led-core.c --------- */

DECLARE_RWSEM(leds_list_lock);
EXPORT_SYMBOL_GPL(leds_list_lock);

LIST_HEAD(leds_list);
EXPORT_SYMBOL_GPL(leds_list);

/* ----------  drivers/led/led-class.c --------- */

static void led_update_brightness(struct led_classdev *led_cdev)
{
	if (led_cdev->brightness_get)
		led_cdev->brightness = led_cdev->brightness_get(led_cdev);
}

/**
 * led_classdev_register - register a new object of led_classdev class.
 * @dev: The device to register.
 * @led_cdev: the led_classdev structure for this device.
 */
int led_classdev_register(struct device *parent, struct led_classdev *led_cdev)
{
	int rc;

#ifdef LINUX
	led_cdev->dev = device_create_drvdata(leds_class, parent, 0, led_cdev,
					      "%s", led_cdev->name);
	if (IS_ERR(led_cdev->dev))
		return PTR_ERR(led_cdev->dev);

	/* register the attributes */
	rc = device_create_file(led_cdev->dev, &dev_attr_brightness);
	if (rc)
		goto err_out;
#endif

	/* add to the list of leds */
	down_write(&leds_list_lock);
	list_add_tail(&led_cdev->node, &leds_list);
	up_write(&leds_list_lock);

	led_update_brightness(led_cdev);

#ifdef CONFIG_LEDS_TRIGGERS
#ifdef LINUX
	init_rwsem(&led_cdev->trigger_lock);

	rc = device_create_file(led_cdev->dev, &dev_attr_trigger);
	if (rc)
		goto err_out_led_list;
#endif

	led_trigger_set_default(led_cdev);
#endif

	printk(KERN_INFO "Registered led device: %s\n",
			led_cdev->name);

	return 0;

#ifdef CONFIG_LEDS_TRIGGERS
err_out_led_list:
#ifdef LINUX
	device_remove_file(led_cdev->dev, &dev_attr_brightness);
#endif
	list_del(&led_cdev->node);
#endif
err_out:
#ifdef LINUX
	device_unregister(led_cdev->dev);
#endif
	return rc;
}

void led_classdev_unregister(struct led_classdev *led_cdev)
{
#ifdef LINUX
	device_remove_file(led_cdev->dev, &dev_attr_brightness);
#endif
#ifdef CONFIG_LEDS_TRIGGERS
#ifdef LINUX
	device_remove_file(led_cdev->dev, &dev_attr_trigger);
#endif
	down_write(&led_cdev->trigger_lock);
	if (led_cdev->trigger)
		led_trigger_set(led_cdev, NULL);
	up_write(&led_cdev->trigger_lock);
#endif

#ifdef LINUX
	device_unregister(led_cdev->dev);
#endif

	down_write(&leds_list_lock);
	list_del(&led_cdev->node);
	up_write(&leds_list_lock);

}

