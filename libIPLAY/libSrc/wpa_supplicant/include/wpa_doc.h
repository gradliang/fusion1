/* dbus.h  Convenience header including all other headers
 *
 * Copyright (C) 2002, 2003  Red Hat Inc.
 *
 * Licensed under the Academic Free License version 2.1
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef WPA_DOC_H
#define WPA_DOC_H


/**
 * @mainpage
 *
 * This manual documents the wireless and network APIs on MP6XX platform.
 *
 * The APIs include wireless driver API, network socket API and buffer/memory
 * APIs.
 */

/**
 * @defgroup WlanNetworkConfig Wireless Network: Configuration/Connection
 * @brief The API to control connection and disconnection to wireless network.
 *
 * On MP6XX platform, wpa_supplicant (a port from Linux 
 * <a href="http://hostap.epitest.fi/wpa_supplicant/">wpa_supplicant</a>)
 * is used to handle connection management and key management with the 
 * AP (Access Point).
 *
 * The API uses wpa_supplicant's control interface to communicate with 
 * wpa_supplicant via an UNIX socket.
 * The API is used to set the parameters of the network interface which
 * are specific to the wireless operation.  Also, you can add/delete a wireless
 * network to/from wpa_supplicant, enable/disable the network, and instructs
 * wpa_supplicant to connect/disconnect to/from the AP.
 *
 * @{
 */

/**
 * @var connect_ssid
 *
 * This is selected ESSID (or Network Name).  Its length is 32 octets.  Note that this is not null-terminated.
 */

/**
 * @var connect_ssidlen
 *
 * This is the length of selected ESSID, from 1 to 32, stored in connect_ssid.
 */

/**
 * @var connect_psk
 *
 * If the length is 64 octets, then it's treated as a PSK.  Otherwise, it is treated as a passphrase.
 * Minimum length of passphrase is 8 octets.
 */

/**
 * @var wlan_security
 *
 * Specify wireless security mode.  Its value can be one of WIFI_NO_SECURITY, WIFI_WEP,
 * WIFI_WPA, and WIFI_WPA2.
 */

/**
 * @var wep_key
 *
 * Up to 4 WEP keys can be set.  Key can be set in ASCII string or hex digits. 
 * Wireless driver uses the length of keys, <b>wep_key_len</b>, to determine
 * which one is used.
 */

/**
 * @var wep_key_len
 *
 * Length for the 4 WEP keys in wep_key.
 */

/**
 * @var wep_tx_keyidx
 *
 * Transmit key index for WEP mode.
 */

/**
 * @def WIFI_NO_SECURITY
 *
 * No security.
 */

/**
 * @def WIFI_WEP
 *
 * Use WEP-64 or WEP-128.
 */

/**
 * @def WIFI_WPA
 *
 * Use WPA-PSK.
 */

/**
 * @def WIFI_WPA2
 *
 * Use WPA2-PSK.
 */

/** @} */

/**
 * @defgroup WirelessExtension Wireless Extension for Linux
 * @brief The low-level API to wireless driver.
 *
 * On MP6XX platform, Wireless Extension for Linux (a port from 
 * <a href="http://www.hpl.hp.com/personal/Jean_Tourrilhes/Linux/Linux.Wireless.Extensions.html">
 * Wireless Extension for Linux</a>)
 * is supported. You can use <b>ioctl()</b> with any driver-supported IOCTL commands to 
 * control the wireless device.
 *
 * @{
 */

/** @} */

/**
 * @defgroup    WlanScan  Wireless Scan
 * @brief The API to instruct driver to start a wireless scan (also known as site survey).
 *   
 * @{
 */

/**
 * @var WiFiApBrowser
 *
 * This variable stores the results of wireless scan.
 */

/** @} */

WIFI_AP_BROWSER WiFiApBrowser;

#endif /* WPA_DOC_H */
