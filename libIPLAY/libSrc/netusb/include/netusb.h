/* ZD1211 USB-WLAN driver for Linux
 *
 * Copyright (C) 2005-2007 Ulrich Kunitz <kune@deine-taler.de>
 * Copyright (C) 2006-2007 Daniel Drake <dsd@gentoo.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef _NETUSB_H
#define _NETUSB_H

#include <linux/completion.h>
#include <linux/netdevice.h>
#include <linux/spinlock.h>
#include <linux/skbuff.h>
#include <linux/usb.h>

#define MAX_USB_DEVICES USBOTG_NONE

/*
 * Qisda H21 HSUPA USB modem
 */
#define EP_VENDOR_IN		1                   /* vendor-specific bulk ep */
#define EP_VENDOR_OUT		1                   /* vendor-specific bulk ep */
#define EP_COMM_IN			2                   /* interrupt ep */
#define EP_DATA_OUT			2                   /* bulk ep */
#define EP_DATA_IN			3                   /* bulk ep */

/*
* Datang LC-6311 TD-SCDMA USB modem
*/
#define EP_DATA_OUT_DATANG	6		/* bulk ep */
#define EP_DATA_IN_DATANG		6		/* bulk ep */

/* Contains the usb parts. The structure doesn't require a lock because intf
 * will not be changed after initialization.
 */
struct qd_usb {
	short   idx;
	struct usb_device *udevice;
	int	    ppp_unit;
	bool	to_connect;
	void   *app_context;
	int new_plug_type;
    	int old_plug_type;
};

typedef void (*netusb_complete_t)(void *, int);

/* free memory if the pointer is valid and zero the pointer */
#ifndef SAFE_FREE
#define SAFE_FREE(x) do { if ((x) != NULL) {mpx_Free(x); (x)=NULL;} } while(0)
#endif

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
#define MEMCOPY(d,s,l) mmcp_memcpy_polling(d, s, l)
#else
#define MEMCOPY(d,s,l) memcpy(d, s, l)
#endif

#endif /* _NETUSB_H */
