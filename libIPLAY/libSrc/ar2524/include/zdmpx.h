#ifndef __ZDMPX_H__
#define __ZDMPX_H__

#include "iplaysysconfig.h"

#define HOST_IF_USB
#define PLATFORM_MPIXEL	1

#define bus_to_virt(p)	(p)
#if Make_ADHOC
#define ZDCONF_ADHOC_SUPPORT   1
#define OFDM
#endif

#endif

