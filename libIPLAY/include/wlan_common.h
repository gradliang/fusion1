/*
*******************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based 
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2008, Magic Pixel Inc, Hsinchu, Taiwan
*
* All rights reserved. Magic Pixel's source code is an unpublished work and the 
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in 
* deciphering, decoding, reverse engineering or in ay way altering the source 
* code is strictly prohibited, unless the prior written consent of Magic 
* Pixel is obtained.
*
* Filename      : wlan_common.h
* Programmer(s) : 
* Created       : 
* Descriptions  : WLAN common defines and structures.
*******************************************************************************
*/
#ifndef _WLAN_COMMON_H
#define _WLAN_COMMON_H

#ifndef NUM_WEP_KEYS
#define NUM_WEP_KEYS 4
#define MAX_WEP_KEY_LEN 16
#endif

/**
 * @ingroup WlanNetworkConfig
 * Define a wireless network.  
 *
 * It is based on what's defined in 
 * wpa_supplicant (struct wpa_ssid).  Currently, 'priority' of struct wpa_ssid 
 * is not supported.
 */
typedef struct wlan_network_s {
    unsigned char alloc;            /**< Indicates if this network has been created in wpa_supplicant.  TRUE/FALSE */
    unsigned short network_id;      /**< Unique ID identifying a network inside wpa_supplicant */
    char ssid[32];                  /**< Service set identifier (network name) */
    unsigned short ssid_len;        /**< Length of the ssid */
    char key[64];                   /**< WPA passphrase or pre-shared key (PSK) */
    unsigned short key_len;         /**< Length of the WPA key */
    unsigned char proto;            /**< Bitfield of allowed protocols, WPA_PROTO_* */
	
#if Make_WPS
	unsigned int key_mgmt;
	char bssid[6];
#else
    unsigned char key_mgmt;         /**< Key management */
#endif
    char wep_key[NUM_WEP_KEYS][MAX_WEP_KEY_LEN];/**< Up to four set of WEP keys */
    unsigned short wep_key_len[NUM_WEP_KEYS];/**< Length of WEP keys */
    int wep_tx_key;                 /**< Key index for TX frames using WEP */
    unsigned char enabled;          /**< Whether this network is currently enabled in wpa_supplicant */
    unsigned char mode;             /**< ESS/IBSS */
    unsigned short frequency;       /**< frequency in MHz for IBSS */
} WLAN_NETWORK;

#endif	/* _WLAN_COMMON_H */

