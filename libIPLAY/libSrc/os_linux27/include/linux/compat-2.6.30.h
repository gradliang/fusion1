#ifndef LINUX_26_30_COMPAT_H
#define LINUX_26_30_COMPAT_H

#include <linux/version.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))

#ifndef TP_PROTO
#define TP_PROTO(args...)	TPPROTO(args)
#endif
#ifndef TP_ARGS
#define TP_ARGS(args...)	TPARGS(args)
#endif

#define IRQ_WAKE_THREAD	(2)

/* From : include/linux/pm.h */
/* How to reorder dpm_list after device_move() */
enum dpm_order {
	DPM_ORDER_NONE,
	DPM_ORDER_DEV_AFTER_PARENT,
	DPM_ORDER_PARENT_BEFORE_DEV,
	DPM_ORDER_DEV_LAST,
};

#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)) */

#endif /* LINUX_26_30_COMPAT_H */
