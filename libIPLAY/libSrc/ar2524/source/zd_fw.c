/* zd_usb.c
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/firmware.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/skbuff.h>
#include <linux/usb.h>
#include <linux/workqueue.h>
#ifdef LINUX
#include <asm/unaligned.h>
#endif

#include "zd_def.h"
#include "zd_mac.h"
#include "zd_usb.h"

#define ZD1211B

const u8 WS11Ub[]
	#include "WS11Ub.h"
const size_t WS11Ub_length = sizeof WS11Ub;

const u8 WS11UPhr[]
	#include "WS11UPhR.h"
const size_t WS11UPhr_length = sizeof WS11UPhr;

const u8 WS11Ur[]
	#include "WS11Ur.h"
const size_t WS11Ur_length = sizeof WS11Ur;
