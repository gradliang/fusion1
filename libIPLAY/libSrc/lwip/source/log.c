/*
 *
 *  Connection Manager
 *
 *  Copyright (C) 2007-2012  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "global612.h"
#include "typedef.h"
#include "taskid.h"

extern void DPrintf(const unsigned char * format, ...);
/**
 * connman_debug:
 * @format: format string
 * @varargs: list of arguments
 *
 * Output debug message
 */
void connman_debug(const char *format, ...)
{
#if 0
    char buf[128];
    const int buflen = 128;
    int len;
	va_list ap;

	va_start(ap, format);

//    vprintf(format, ap);
    len = vsnprintf(buf, buflen, format, ap);
	va_end(ap);

    DPrintf(buf);
#endif
}

char *task_get_name(int tid)
{
    switch (tid)
    {
        case WPA_MAIN_TASK_ID:
            return "hostapd";
        case MAIN_TASK:
            return "main";
        case MCARD_TASK:
            return "mcard";
        case 14:
            return "UsbOtgHostDriverTask";
        case 26:
            return "usb_timer";
        case 41:
            return "worker_thread0"; /* reg_todo() name = "ath9k" */
        case 45:
            return "ath9k_wmi_event_tasklet";
        case 46:
            return "ath9k_rx_tasklet";
        default:
            return "unknown";
    }

    return "";
}
