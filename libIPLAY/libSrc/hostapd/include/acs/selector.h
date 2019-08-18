/*
 * Automatic channel selector for ieee802.11 A, B and G modes.
 *
 * Copyright (c) 2005 by Wilibox (www.wilibox.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#ifndef __SELECTOR_H__
#define __SELECTOR_H__

//#include <sys/socket.h>
//#include <iwlib.h>
#define IW_MAX_FREQUENCIES	32

/* Quality data array size. Must be sufficient to hold any 
   of the 802.11a/b/g channel sets. */   
#define QUAL_DATA_SIZE IW_MAX_FREQUENCIES

/* Single channel characteristics */
typedef struct channel_quality
{
    unsigned int     channel;            /* channel no. */
    unsigned int     ap_count;           /* active APs on a channel count */ 
    unsigned long    my_quality;         /* sum of quality levels of all APs on a channel */
    unsigned long    neighbours_quality; /* neighbouring channels qualities sum */
} channel_quality;

/*
   Selects the "best" channel to use.
   
   wiface   - wireless interface name;
   channels - a subset of allowed by user channels;
   count    - count of allowed channels;
   
   Returns: "best" channel no. on success, 
            0 on error or no usable channel found. 
*/
int get_best_channel(const char* wiface, const int count, const int* channels);

#endif /* #ifndef __SELECTOR_H__ */
