/*
 * =====================================================================================
 *
 *       Filename:  driver_event.c
 *
 *    Description:  This is an implementation of Linux wireless_send_event() 
 *                  for MP52x/uItron system.  Wireless driver uses this
 *                  routine to send event to WPA Supplicant, such as 
 *                  ASSOCIATION, DISASSOCIATION, et al.
 *
 *                  On Linux system, events are sent by using Netlink socket.
 *                  On MP52x/uItron system, we use messaging (mailbox) to 
 *                  send events.
 *
 *                  This implementation is modified from Linux version.
 *
 *        Company:  Magic Pixel Inc.
 *
 *        Copyright (c) 2008-     Magic Pixel, Inc.
 * =====================================================================================
 */

#define LOCAL_DEBUG_ENABLE 0
#define __KERNEL__

#include "includes.h"
//#include "os_defs.h"
//#include "iw_handler.h"
#include <net/iw_handler.h>
#include "error.h"
#include "net_device.h"
#include "os.h"

#ifndef KERN_ERR
#define KERN_ERR
#endif
#define printk  mpDebugPrint

/*
 * Meta-data about all the standard Wireless Extension request we
 * know about.
 */
static const struct iw_ioctl_description standard_ioctl[] = {
	{
		IW_HEADER_TYPE_NULL,      /* SIOCSIWCOMMIT */
	},
	{
		IW_HEADER_TYPE_CHAR,      /* SIOCGIWNAME */
	},
	{
		IW_HEADER_TYPE_PARAM,      /* SIOCSIWNWID */
	},
	{
		IW_HEADER_TYPE_PARAM,      /* SIOCGIWNWID */
	},
	{
		IW_HEADER_TYPE_FREQ,      /* SIOCSIWFREQ */
	},
	{
		IW_HEADER_TYPE_FREQ,      /* SIOCGIWFREQ */
	},
	{
		IW_HEADER_TYPE_UINT,      /* SIOCSIWMODE */
	},
	{
		IW_HEADER_TYPE_UINT,      /* SIOCGIWMODE */
	},
	{
		IW_HEADER_TYPE_PARAM,      /* SIOCSIWSENS */
	},
	{
		IW_HEADER_TYPE_PARAM,      /* SIOCGIWSENS */
	},
	 {
		IW_HEADER_TYPE_NULL,      /* SIOCSIWRANGE */
	},
	 {
		IW_HEADER_TYPE_POINT,      /* SIOCGIWRANGE */
	},
	{
		IW_HEADER_TYPE_NULL,      /* SIOCSIWPRIV */
	},
	{ /* (handled directly by us) */
		IW_HEADER_TYPE_NULL,      /* SIOCGIWPRIV */
	},
	 {
		IW_HEADER_TYPE_NULL,      /* SIOCSIWSTATS */
	},
	 { /* (handled directly by us) */
		IW_HEADER_TYPE_NULL,      /* SIOCGIWSTATS */
	},
	{
		IW_HEADER_TYPE_POINT,      /* SIOCSIWSPY */
	},
	{
		IW_HEADER_TYPE_POINT,      /* SIOCGIWSPY */
	},
	{
		IW_HEADER_TYPE_POINT,      /* SIOCSIWTHRSPY */
	},
	{
		IW_HEADER_TYPE_POINT,      /* SIOCGIWTHRSPY */
	},
	{
		IW_HEADER_TYPE_ADDR,      /* SIOCSIWAP */
	},
	{
		IW_HEADER_TYPE_ADDR,      /* SIOCGIWAP */
	},
	{
		IW_HEADER_TYPE_POINT,      /* SIOCGIWAPLIST */
	},
	 {
		IW_HEADER_TYPE_PARAM,      /* SIOCSIWSCAN */
	},
	 {
		IW_HEADER_TYPE_POINT,      /* SIOCGIWSCAN */
	},
	 {
		IW_HEADER_TYPE_POINT,      /* SIOCSIWESSID */
	},
	 {
		IW_HEADER_TYPE_POINT,      /* SIOCGIWESSID */
	},
	 {
		IW_HEADER_TYPE_POINT,      /* SIOCSIWNICKN */
	},
	 {
		IW_HEADER_TYPE_POINT,      /* SIOCGIWNICKN */
	},
	 {
		IW_HEADER_TYPE_PARAM,      /* SIOCSIWRATE */
	},
	 {
		IW_HEADER_TYPE_PARAM,      /* SIOCGIWRATE */
	},
	 {
		IW_HEADER_TYPE_PARAM,      /* SIOCSIWRTS */
	},
	 {
		IW_HEADER_TYPE_PARAM,      /* SIOCGIWRTS */
	},
	 {
		IW_HEADER_TYPE_PARAM,      /* SIOCSIWFRAG */
	},
	 {
		IW_HEADER_TYPE_PARAM,      /* SIOCGIWFRAG */
	},
	 {
		IW_HEADER_TYPE_PARAM,      /* SIOCSIWTXPOW */
	},
	 {
		IW_HEADER_TYPE_PARAM,      /* SIOCGIWTXPOW */
	},
	 {
		IW_HEADER_TYPE_PARAM,      /* SIOCSIWRETRY */
	},
	 {
		IW_HEADER_TYPE_PARAM,      /* SIOCGIWRETRY */
	},
	 {
		IW_HEADER_TYPE_POINT,      /* SIOCSIWENCODE */
	},
	{
		IW_HEADER_TYPE_POINT,      /* SIOCGIWENCODE */
	},
	 {
		IW_HEADER_TYPE_PARAM,      /* SIOCSIWPOWER */
	},
	 {
		IW_HEADER_TYPE_PARAM,      /* SIOCGIWPOWER */
	},
};
static const unsigned standard_ioctl_num = ARRAY_SIZE(standard_ioctl);

static const struct iw_ioctl_description standard_event[] = {
	{
			 IW_HEADER_TYPE_ADDR,      /* IWEVTXDROP */
	},
	{
			 IW_HEADER_TYPE_QUAL,      /* IWEVQUAL */
	},
	{
		.header_type	= IW_HEADER_TYPE_POINT,      /* IWEVCUSTOM */
		.token_size	= 1,
		.max_tokens	= IW_CUSTOM_MAX,
	},
	{
			 IW_HEADER_TYPE_ADDR,      /* IWEVREGISTERED */
	},
	{
			 IW_HEADER_TYPE_ADDR,       /* IWEVEXPIRED */
	},
	{
		.header_type	= IW_HEADER_TYPE_POINT,      /* IWEVGENIE */
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	{
		.header_type	= IW_HEADER_TYPE_POINT,      /* IWEVMICHAELMICFAILURE */
		.token_size	= 1,
		.max_tokens	= sizeof(struct iw_michaelmicfailure),
	},
	{
		.header_type	= IW_HEADER_TYPE_POINT,      /* IWEVASSOCREQIE */
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	{
		.header_type	= IW_HEADER_TYPE_POINT,      /* IWEVASSOCRESPIE */
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
};
static const unsigned standard_event_num = ARRAY_SIZE(standard_event);

/* Size (in bytes) of various events */
static const int event_type_size[] = {
	IW_EV_LCP_LEN,			/* IW_HEADER_TYPE_NULL */
	0,
	IW_EV_CHAR_LEN,			/* IW_HEADER_TYPE_CHAR */
	0,
	IW_EV_UINT_LEN,			/* IW_HEADER_TYPE_UINT */
	IW_EV_FREQ_LEN,			/* IW_HEADER_TYPE_FREQ */
	IW_EV_ADDR_LEN,			/* IW_HEADER_TYPE_ADDR */
	0,
	IW_EV_POINT_LEN,		/* Without variable payload */
	IW_EV_PARAM_LEN,		/* IW_HEADER_TYPE_PARAM */
	IW_EV_QUAL_LEN,			/* IW_HEADER_TYPE_QUAL */
};

static int wpa_event_open(const char *event_path);

/*
 * wireless_send_event:
 *
 * Based on Linux's wireless_send_event.
 *
 * Main event dispatcher. Called from other parts and drivers.
 * Send the event on the appropriate channels.
 * May be called from interrupt context.
 */
void wireless_send_event(struct net_device *	dev,
			 unsigned int		cmd,
			 union iwreq_data *	wrqu,
			 char *			extra)
{
	const struct iw_ioctl_description *	descr = NULL;
	int extra_len = 0;
    char buf[IW_CUSTOM_MAX+sizeof(struct iw_event)];
	struct iw_event  *event=(struct iw_event *)buf;
	int event_len;				/* Its size */
	int hdr_len;				/* Size of the event header */
	int wrqu_off = 0;			/* Offset in wrqu */
	unsigned	cmd_index;		/* *MUST* be unsigned */
    S32 ret;

    if (dev->wireless_event_sock == 0)
    {
        dev->wireless_event_sock = wpa_event_open("/wpa_event");
    }

	/* Get the description of the IOCTL */
	if(cmd <= SIOCIWLAST) {
		cmd_index = cmd - SIOCIWFIRST;
		if(cmd_index < standard_ioctl_num)
			descr = &(standard_ioctl[cmd_index]);
	} else {
		cmd_index = cmd - IWEVFIRST;
		if(cmd_index < standard_event_num)
			descr = &(standard_event[cmd_index]);
	}

	/* Check extra parameters and set extra_len */
	if(descr->header_type == IW_HEADER_TYPE_POINT) {
		/* Check if number of token fits within bounds */
		if (wrqu->data.length > descr->max_tokens) {
			printk(KERN_ERR "%s (WE) : Wireless Event too big (%d)\n", dev->name, wrqu->data.length);
			MP_ASSERT(0);
			return;
		}
		if (wrqu->data.length < descr->min_tokens) {
			printk(KERN_ERR "%s (WE) : Wireless Event too small (%d)\n", dev->name, wrqu->data.length);
			MP_ASSERT(0);
			return;
		}
		/* Calculate extra_len - extra is NULL for restricted events */
		if (extra != NULL)
			extra_len = wrqu->data.length * descr->token_size;
		/* Always at an offset in wrqu */
		wrqu_off = IW_EV_POINT_OFF;
	}

	/* Total length of the event */
	hdr_len = event_type_size[descr->header_type];
	event_len = hdr_len + extra_len;
	MP_ASSERT(event_len <= sizeof(buf));

    if (event_len & 0x03)   /* mpx_MessageDrop requires 4-byte aligned */
        event_len += 4 - (event_len & 0x03);

    mpDebugPrint("wireless_send_event: cmd=0x%X, len=%d", cmd, event_len);

	/* Fill event */
	event->len = event_len;
	event->cmd = cmd;
	memcpy(&event->u, ((char *) wrqu) + wrqu_off, hdr_len - IW_EV_LCP_LEN);
	if (extra)
		memcpy(((char *) event) + hdr_len, extra, extra_len);

	if (send(dev->wireless_event_sock, event, event_len, 0) < 0) {
        mpDebugPrint("wireless_send_event: send returns err=%d", errno);
        MP_ASSERT(0);
	}
	return;
}

static int wpa_event_open(const char *event_path)
{
	static int counter = 0;
	struct sockaddr_un local;
	struct sockaddr_un dest;
    int s;

	s = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (s < 0) {
        MP_ASSERT(0);
		return 0;
	}

	local.sun_family = AF_UNIX;
	os_snprintf(local.sun_path, sizeof(local.sun_path),
		    "/wpa_event_%d", counter++);
	if (bind(s, (struct sockaddr *) &local, sizeof(local)) < 0) {
		closesocket(s);
        MP_ASSERT(0);
		return 0;
	}

	dest.sun_family = AF_UNIX;
	os_snprintf(dest.sun_path, sizeof(dest.sun_path), "%s", event_path);
	if (connect(s, (struct sockaddr *) &dest,
		    sizeof(dest)) < 0) {
		closesocket(s);
        MP_ASSERT(0);
		return 0;
	}

	return s;
}
