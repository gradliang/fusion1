#ifndef __LINUX_CPUMASK_H
#define __LINUX_CPUMASK_H

#include <linux/kernel.h>
#include <linux/threads.h>
#include <linux/bitmap.h>

typedef struct { DECLARE_BITMAP(bits, NR_CPUS); } cpumask_t;

#endif /* __LINUX_CPUMASK_H */
