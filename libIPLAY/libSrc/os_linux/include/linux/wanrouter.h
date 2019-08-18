/*****************************************************************************
* wanrouter.h	Definitions for the WAN Multiprotocol Router Module.
*		This module provides API and common services for WAN Link
*		Drivers and is completely hardware-independent.
*
* Author: 	Nenad Corbic <ncorbic@sangoma.com>
*		Gideon Hack 	
* Additions:	Arnaldo Melo
*
* Copyright:	(c) 1995-2000 Sangoma Technologies Inc.
*
*		This program is free software; you can redistribute it and/or
*		modify it under the terms of the GNU General Public License
*		as published by the Free Software Foundation; either version
*		2 of the License, or (at your option) any later version.
* ============================================================================
* Jul 21, 2000  Nenad Corbic	Added WAN_FT1_READY State
* Feb 24, 2000  Nenad Corbic    Added support for socket based x25api
* Jan 28, 2000  Nenad Corbic    Added support for the ASYNC protocol.
* Oct 04, 1999  Nenad Corbic 	Updated for 2.1.0 release
* Jun 02, 1999  Gideon Hack	Added support for the S514 adapter.
* May 23, 1999	Arnaldo Melo	Added local_addr to wanif_conf_t
*				WAN_DISCONNECTING state added
* Jul 20, 1998	David Fong	Added Inverse ARP options to 'wanif_conf_t'
* Jun 12, 1998	David Fong	Added Cisco HDLC support.
* Dec 16, 1997	Jaspreet Singh	Moved 'enable_IPX' and 'network_number' to
*				'wanif_conf_t'
* Dec 05, 1997	Jaspreet Singh	Added 'pap', 'chap' to 'wanif_conf_t'
*				Added 'authenticator' to 'wan_ppp_conf_t'
* Nov 06, 1997	Jaspreet Singh	Changed Router Driver version to 1.1 from 1.0
* Oct 20, 1997	Jaspreet Singh	Added 'cir','bc','be' and 'mc' to 'wanif_conf_t'
*				Added 'enable_IPX' and 'network_number' to 
*				'wan_device_t'.  Also added defines for
*				UDP PACKET TYPE, Interrupt test, critical values
*				for RACE conditions.
* Oct 05, 1997	Jaspreet Singh	Added 'dlci_num' and 'dlci[100]' to 
*				'wan_fr_conf_t' to configure a list of dlci(s)
*				for a NODE 
* Jul 07, 1997	Jaspreet Singh	Added 'ttl' to 'wandev_conf_t' & 'wan_device_t'
* May 29, 1997 	Jaspreet Singh	Added 'tx_int_enabled' to 'wan_device_t'
* May 21, 1997	Jaspreet Singh	Added 'udp_port' to 'wan_device_t'
* Apr 25, 1997  Farhan Thawar   Added 'udp_port' to 'wandev_conf_t'
* Jan 16, 1997	Gene Kozin	router_devlist made public
* Jan 02, 1997	Gene Kozin	Initial version (based on wanpipe.h).
*****************************************************************************/

#ifndef	_ROUTER_H
#define	_ROUTER_H

#define	ROUTER_NAME	"wanrouter"	/* in case we ever change it */
#define	ROUTER_VERSION	1		/* version number */
#define	ROUTER_RELEASE	1		/* release (minor version) number */
#define	ROUTER_IOCTL	'W'		/* for IOCTL calls */
#define	ROUTER_MAGIC	0x524D4157L	/* signature: 'WANR' reversed */

#endif	/* _ROUTER_H */
