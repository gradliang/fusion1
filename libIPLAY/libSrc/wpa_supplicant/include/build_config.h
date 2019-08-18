/*
 * wpa_supplicant/hostapd - Build time configuration defines
 * Copyright (c) 2005-2006, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 *
 * This header file can be used to define configuration defines that were
 * originally defined in Makefile. This is mainly meant for IDE use or for
 * systems that do not have suitable 'make' tool. In these cases, it may be
 * easier to have a single place for defining all the needed C pre-processor
 * defines.
 */

#ifndef BUILD_CONFIG_H
#define BUILD_CONFIG_H

#include "iplaysysconfig.h"

/* Insert configuration defines, e.g., #define EAP_MD5, here, if needed. */
#define CONFIG_WIN32_DEFAULTS

#ifdef CONFIG_WIN32_DEFAULTS
//#define CONFIG_NO_STDOUT_DEBUG
//#define CONFIG_NO_WPA_MSG
//#define CONFIG_NATIVE_WINDOWS
//#define CONFIG_ANSI_C_EXTRA
//#define CONFIG_WINPCAP
#if Make_USB == REALTEK_RTL8188CU
#define IEEE8021X_EAPOL
#else
//#define IEEE8021X_EAPOL
#endif
#define EAP_TLS_FUNCS
#define PKCS12_FUNCS
//#define PCSC_FUNCS
#define CONFIG_CTRL_IFACE
#define CONFIG_CTRL_IFACE_UNIX
//#define CONFIG_CTRL_IFACE_NAMED_PIPE
//#define CONFIG_DRIVER_NDIS
#if (Make_SDIO == REALTEK_WIFI)			
#define CONFIG_DRIVER_REALTEK
#elif (Make_SDIO == MARVELL_WIFI)
#define CONFIG_DRIVER_MARVELL
#endif
#if (Make_USB == REALTEK_WIFI_USB)			
#define CONFIG_DRIVER_REALTEK_USB
#elif (Make_USB == RALINK_WIFI)
#define CONFIG_DRIVER_RALINK
#elif (Make_USB == AR2524_WIFI)
#define CONFIG_DRIVER_AR2524
#elif (Make_USB == RALINK_AR2524_WIFI)
#define CONFIG_DRIVER_RALINK
#define CONFIG_DRIVER_AR2524
#elif (Make_USB == AR9271_WIFI)
#define CONFIG_DRIVER_WEXT
#elif (Make_USB == REALTEK_RTL8188CU)
#define CONFIG_DRIVER_WEXT
#elif (Make_USB == REALTEK_RTL8188E)
#define CONFIG_DRIVER_WEXT
#endif
//#define CONFIG_NDIS_EVENTS_INTEGRATED
//#define CONFIG_DEBUG_FILE
#if Make_USB == REALTEK_RTL8188CU
#define EAP_MD5
#define EAP_TLS
#define EAP_MSCHAPv2
#define EAP_PEAP
#define EAP_TTLS
#define EAP_GTC
#define EAP_OTP
#define EAP_LEAP
#define EAP_TNC
#else
//#define EAP_MD5
//#define EAP_TLS
//#define EAP_MSCHAPv2
//#define EAP_PEAP
//#define EAP_TTLS
//#define EAP_GTC
//#define EAP_OTP
//#define EAP_LEAP
//#define EAP_TNC
#endif
#define _CRT_SECURE_NO_DEPRECATE
#define USE_INTERNAL_CRYPTO
#if Make_WPS
#define IEEE8021X_EAPOL
#define CONFIG_WPS
#define EAP_WSC
#endif

#ifdef USE_INTERNAL_CRYPTO
#define CONFIG_TLS_INTERNAL
#define CONFIG_TLS_INTERNAL_CLIENT
#define CONFIG_INTERNAL_LIBTOMMATH
#define INTERNAL_AES
#define INTERNAL_SHA1
#define INTERNAL_SHA256
#define INTERNAL_MD5
#define INTERNAL_MD4
#define INTERNAL_DES
#define CONFIG_INTERNAL_X509
#define CONFIG_CRYPTO_INTERNAL
#endif /* USE_INTERNAL_CRYPTO */

#define CONFIG_CPU_R4X00
#define CONFIG_WIRELESS_EXT

#endif /* CONFIG_WIN32_DEFAULTS */

#endif /* BUILD_CONFIG_H */
