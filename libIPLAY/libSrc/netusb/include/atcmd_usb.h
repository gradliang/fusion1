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

#ifndef _ATCMD_USB_H
#define _ATCMD_USB_H

#include <linux/completion.h>
#include <linux/netdevice.h>
#include <linux/spinlock.h>
#include <linux/skbuff.h>
#include <linux/usb.h>

struct atmcmd_request;                                     /* forward declaration */
typedef void (*atcmd_complete_t)(struct atmcmd_request *);

struct atmcmd_request {
    char *cmd;
    char *rx_pattern;
    char *data;
    int length;
    int status;

#define ATCMD_NO_OK       	BIT0
#define ATCMD_SMS        	BIT1
#define ATCMD_PENDING       BIT2
    int flags;

    int timeout;
	void *context;			/* (in) context for completion */
	atcmd_complete_t complete;	/* (in) completion routine */

	struct list_head list;
};

typedef enum{
	LONG_AN_STRING,
	SHORT_AN_STRING,
	NUMERIC_ID
}cops_format_;

typedef struct _format_flag_{
	BOOL long_0;
	BOOL short_1;
	BOOL numeric_2;
}format_flag_;

#endif /* _ATCMD_USB_H */
