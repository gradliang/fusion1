#ifndef LINUX_26_COMPAT_H
#define LINUX_26_COMPAT_H

#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))
#include <generated/autoconf.h>
#else
#include <linux/autoconf.h>
#endif

/*
 * Each compat file represents compatibility code for new kernel
 * code introduced for *that* kernel revision.
 */


#endif /* LINUX_26_COMPAT_H */
