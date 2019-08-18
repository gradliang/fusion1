
#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <linux/kobject.h>
#include <linux/list.h>
#include <linux/lockdep.h>
#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/pm.h>
#include <asm/semaphore.h>
#include <asm/atomic.h>

struct device {
	struct device		*parent;
	void		*driver_data;	/* data private to the driver */
};

#define dev_printk(level, dev, format, arg...)	\
	printk(format , \
	       ## arg)

#define dev_err(dev, format, arg...)		\
	mpDebugPrint(format , ## arg)
#define dev_warn(dev, format, arg...)		\
	dev_printk(KERN_WARNING , dev , format , ## arg)
#define dev_info(dev, format, arg...)		\
	mpDebugPrint(format , ## arg)
#define dev_dbg(dev, format, arg...)

#endif

