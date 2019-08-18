#ifndef __LINUX_CACHE_H
#define __LINUX_CACHE_H

#include <linux/kernel.h>
#include <asm/cache.h>

#ifndef __read_mostly
#define __read_mostly
#endif

#ifndef ____cacheline_aligned
#define ____cacheline_aligned
#endif

#ifndef ____cacheline_aligned_in_smp
#define ____cacheline_aligned_in_smp
#endif

#endif

