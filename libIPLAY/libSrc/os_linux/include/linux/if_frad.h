/*
 * DLCI/FRAD	Definitions for Frame Relay Access Devices.  DLCI devices are
 *		created for each DLCI associated with a FRAD.  The FRAD driver
 *		is not truly a network device, but the lower level device
 *		handler.  This allows other FRAD manufacturers to use the DLCI
 *		code, including its RFC1490 encapsulation alongside the current
 *		implementation for the Sangoma cards.
 *
 * Version:	@(#)if_ifrad.h	0.15	31 Mar 96
 *
 * Author:	Mike McLagan <mike.mclagan@linux.org>
 *
 * Changes:
 *		0.15	Mike McLagan	changed structure defs (packed)
 *					re-arranged flags
 *					added DLCI_RET vars
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */

#ifndef _FRAD_H_
#define _FRAD_H_

#include <linux/if.h>


#endif
