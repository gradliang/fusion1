/*
 * LWIP - Build time configuration defines
 * Copyright (c) 2008-    , Magic Pixel Inc.
 *
 */

#ifndef LWIP_CONFIG_H
#define LWIP_CONFIG_H

#include "iplaysysconfig.h"

/* Insert configuration defines, e.g., #define EAP_MD5, here, if needed. */

#if Make_USB || PPP_ENABLE || DM9621_ETHERNET_ENABLE || DM9KS_ETHERNET_ENABLE
#define DRIVER_FREE_TXBUFFER
#endif


#endif /* LWIP_CONFIG_H */
