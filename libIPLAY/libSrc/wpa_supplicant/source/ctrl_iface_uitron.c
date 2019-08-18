/*
 * WPA Supplicant / UNIX domain socket -based control interface
 * Copyright (c) 2004-2005, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"
#ifdef LINUX
#include <sys/un.h>
#else
#include <net_unix.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <grp.h>

#include "common.h"
#include "eloop.h"
#include "config.h"
//#include "eapol_sm.h"
#include "wpa_supplicant_i.h"
#include "ctrl_iface.h"
//#include "net_device.h"

/* Per-interface ctrl_iface */

/**
 * struct wpa_ctrl_dst - Internal data structure of control interface monitors
 *
 * This structure is used to store information about registered control
 * interface monitors into struct wpa_supplicant. This data is private to
 * ctrl_iface_unix.c and should not be touched directly from other files.
 */
struct wpa_ctrl_dst {
	struct wpa_ctrl_dst *next;
	struct sockaddr_un addr;
	socklen_t addrlen;
	int debug_level;
	int errors;
};


struct ctrl_iface_priv {
	struct wpa_supplicant *wpa_s;
	int sock;
	struct wpa_ctrl_dst *ctrl_dst;
};


static void wpa_supplicant_ctrl_iface_receive(int sock, void *eloop_ctx,
					      void *sock_ctx)
{
	struct wpa_supplicant *wpa_s = eloop_ctx;
	struct ctrl_iface_priv *priv = sock_ctx;
	char buf[256];
	int res;
	struct sockaddr_un from;
	socklen_t fromlen = sizeof(from);
	char *reply = NULL;
	size_t reply_len = 0;
	int new_attached = 0;

	res = recvfrom(sock, buf, sizeof(buf) - 1, 0,
		       (struct sockaddr *) &from, &fromlen);
	if (res < 0) {
		perror("recvfrom(ctrl_iface)");
		return;
	}
	buf[res] = '\0';

	{
		reply = wpa_supplicant_ctrl_iface_process(wpa_s, buf,
							  &reply_len);
	}

	if (reply) {
		sendto(sock, reply, reply_len, 0, (struct sockaddr *) &from,
		       fromlen);
		os_free(reply);
	} else if (reply_len == 1) {
		sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *) &from,
		       fromlen);
	} else if (reply_len == 2) {
		sendto(sock, "OK\n", 3, 0, (struct sockaddr *) &from,
		       fromlen);
	}

	if (new_attached)
		eapol_sm_notify_ctrl_attached(wpa_s->eapol);
}


static char * wpa_supplicant_ctrl_iface_path(struct wpa_supplicant *wpa_s)
{
	char *buf;
	size_t len;
	char *pbuf, *dir = NULL, *gid_str = NULL;

	if (wpa_s->conf->ctrl_interface == NULL)
		return NULL;

	pbuf = os_strdup(wpa_s->conf->ctrl_interface);
	if (pbuf == NULL)
		return NULL;
	if (os_strncmp(pbuf, "DIR=", 4) == 0) {
		dir = pbuf + 4;
		gid_str = os_strstr(dir, " GROUP=");
		if (gid_str) {
			*gid_str = '\0';
			gid_str += 7;
		}
	} else
		dir = pbuf;

	len = os_strlen(dir) + os_strlen(wpa_s->ifname) + 2;
	buf = os_malloc(len);
	if (buf == NULL) {
		os_free(pbuf);
		return NULL;
	}

	os_snprintf(buf, len, "%s/%s", dir, wpa_s->ifname);
#ifdef __CYGWIN__
	{
		/* Windows/WinPcap uses interface names that are not suitable
		 * as a file name - convert invalid chars to underscores */
		char *pos = buf;
		while (*pos) {
			if (*pos == '\\')
				*pos = '_';
			pos++;
		}
	}
#endif /* __CYGWIN__ */
	os_free(pbuf);
	return buf;
}


#ifdef CONFIG_CTRL_IFACE
struct ctrl_iface_priv *
wpa_supplicant_ctrl_iface_init(struct wpa_supplicant *wpa_s)
{
	struct ctrl_iface_priv *priv;
	struct sockaddr_un addr;
	char *fname = NULL;

	priv = os_zalloc(sizeof(*priv));
	if (priv == NULL)
		return NULL;
	priv->wpa_s = wpa_s;
	priv->sock = -1;

	if (wpa_s->conf->ctrl_interface == NULL)
		return priv;

	priv->sock = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (priv->sock < 0) {
		perror("socket(PF_UNIX)");
		goto fail;
	}

	os_memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
//	fname = wpa_supplicant_ctrl_iface_path(wpa_s);
	fname = wpa_s->ifname;
	if (fname == NULL)
		goto fail;
	os_strncpy(addr.sun_path, fname, sizeof(addr.sun_path));
	if (bind(priv->sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		wpa_printf(MSG_DEBUG, "ctrl_iface bind(PF_UNIX) failed: %s",
			   strerror(errno));
	}

//	os_free(fname);

	eloop_register_read_sock(priv->sock, wpa_supplicant_ctrl_iface_receive,
				 wpa_s, priv);

	return priv;

fail:
	if (priv->sock >= 0)
		close(priv->sock);
	os_free(priv);
	if (fname) {
		unlink(fname);
		os_free(fname);
	}
	return NULL;
}


void wpa_supplicant_ctrl_iface_deinit(struct ctrl_iface_priv *priv)
{
	struct wpa_ctrl_dst *dst, *prev;

	if (priv->sock > -1) {
		char *fname;
		char *buf, *dir = NULL, *gid_str = NULL;
		eloop_unregister_read_sock(priv->sock);
		if (priv->ctrl_dst) {
			/*
			 * Wait a second before closing the control socket if
			 * there are any attached monitors in order to allow
			 * them to receive any pending messages.
			 */
			wpa_printf(MSG_DEBUG, "CTRL_IFACE wait for attached "
				   "monitors to receive messages");
			os_sleep(1, 0);
		}
		close(priv->sock);
		priv->sock = -1;
		fname = wpa_supplicant_ctrl_iface_path(priv->wpa_s);
		if (fname) {
			unlink(fname);
			os_free(fname);
		}

		buf = os_strdup(priv->wpa_s->conf->ctrl_interface);
		if (buf == NULL)
			goto free_dst;
		if (os_strncmp(buf, "DIR=", 4) == 0) {
			dir = buf + 4;
			gid_str = os_strstr(dir, " GROUP=");
			if (gid_str) {
				*gid_str = '\0';
				gid_str += 7;
			}
		} else
			dir = buf;

		os_free(buf);
	}

free_dst:
	dst = priv->ctrl_dst;
	while (dst) {
		prev = dst;
		dst = dst->next;
		os_free(prev);
	}
	os_free(priv);
}




/* Global ctrl_iface */

struct ctrl_iface_global_priv {
	struct wpa_global *global;
	int sock;
};


static void wpa_supplicant_global_ctrl_iface_receive(int sock, void *eloop_ctx,
						     void *sock_ctx)
{
	struct wpa_global *global = eloop_ctx;
	char buf[256];
	int res;
	struct sockaddr_un from;
	socklen_t fromlen = sizeof(from);
	char *reply;
	size_t reply_len;

	res = recvfrom(sock, buf, sizeof(buf) - 1, 0,
		       (struct sockaddr *) &from, &fromlen);
	if (res < 0) {
		perror("recvfrom(ctrl_iface)");
		return;
	}
	buf[res] = '\0';

	reply = wpa_supplicant_global_ctrl_iface_process(global, buf,
							 &reply_len);

	if (reply) {
		sendto(sock, reply, reply_len, 0, (struct sockaddr *) &from,
		       fromlen);
		os_free(reply);
	} else if (reply_len) {
		sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *) &from,
		       fromlen);
	}
}


struct ctrl_iface_global_priv *
wpa_supplicant_global_ctrl_iface_init(struct wpa_global *global)
{
	struct ctrl_iface_global_priv *priv;
	struct sockaddr_un addr;

	priv = os_zalloc(sizeof(*priv));
	if (priv == NULL)
		return NULL;
	priv->global = global;
	priv->sock = -1;

	if (global->params.ctrl_interface == NULL)
		return priv;

	wpa_printf(MSG_DEBUG, "Global control interface '%s'",
		   global->params.ctrl_interface);

	priv->sock = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (priv->sock < 0) {
		perror("socket(PF_UNIX)");
		goto fail;
	}

	os_memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	os_strncpy(addr.sun_path, global->params.ctrl_interface,
		   sizeof(addr.sun_path));
	if (bind(priv->sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind(PF_UNIX)");
        goto fail;
	}

	eloop_register_read_sock(priv->sock,
				 wpa_supplicant_global_ctrl_iface_receive,
				 global, NULL);

	return priv;

fail:
	if (priv->sock >= 0)
		close(priv->sock);
	os_free(priv);
	return NULL;
}


void
wpa_supplicant_global_ctrl_iface_deinit(struct ctrl_iface_global_priv *priv)
{
	if (priv->sock >= 0) {
		eloop_unregister_read_sock(priv->sock);
		close(priv->sock);
	}
	if (priv->global->params.ctrl_interface)
		unlink(priv->global->params.ctrl_interface);
	os_free(priv);
}
#endif /* CONFIG_CTRL_IFACE */
