/*
 * wpa_ctrl_mpx.c -- API functions to change configuration/trigger reassociation 
 *                   via WPA Supplicant's control interface.
 *
 * wpa_WlanNetworkConnect
 * wpa_WlanNetworkReconnect
 * wpa_WlanNetworkSet
 * wpa_WlanNetworkAdd
 * wpa_WlanNetworkRemove
 * wpa_WlanNetworkDisable
 *
 * Copyright (c) Magic Pixel Inc., 2008-    . All Rights Reserved.
 *
 */

/******************************** Description *********************************/

/*
 *	
 *	
 */

/********************************* Includes ***********************************/

#define LOCAL_DEBUG_ENABLE 1
#define __KERNEL__

//#include "global612.h"
#include "mpTrace.h"
#include "includes.h"

#include "net_device.h"
#include "common.h"
#include "config.h"
#include "os_mp52x.h"
#include "netware.h"
#include "wlan_common.h"
#include "net_nic.h"


/*********************************** Locals ***********************************/


/****************************** Forward Declarations **************************/

extern void *wpa_ctrl_init(void);
int wpa_WlanNetworkReconnect(void);
static int doSetNetwork(WLAN_NETWORK *wn_p);
static int doSetApScan(int ap_scan);


/******************************************************************************/
/*
 *	T
 */

void *wpa_ctrl_ctx;
extern struct net_device NicArray[NIC_DRIVER_MAX];
extern char connect_ssid[];
extern int wlan_security;
extern int wep_tx_keyidx;
extern char wep_key[NUM_WEP_KEYS][MAX_WEP_KEY_LEN];
extern short wep_key_len[NUM_WEP_KEYS];
extern char connect_psk[];
extern int connect_ssidlen;
extern int wlan_mode;
extern int wlan_channel;

extern BYTE WpsRun;
extern BYTE g_Net_WifiStatus;
extern BYTE WpsPinCode[8];

#if Make_WPS 
extern BYTE WpsReconnect;
#endif

int m_ieee80211_channel_to_frequency(int chan)
{
	if (chan < 14)
		return 2407 + chan * 5;

	if (chan == 14)
		return 2484;

	/* FIXME: 802.11j 17.3.8.3.2 */
	return (chan + 1000) * 5;
}
EXPORT_SYMBOL(m_ieee80211_channel_to_frequency);

/**
 * @addtogroup WlanNetworkConfig
 * @{
 */

/**
 * Instructs wpa_supplicant to connect to the specified wireless network
 * by enabling the network.
 *
 * The wireless network is specified by @p wn_p.  An ENABLE_NETWORK request
 * is sent to wpa_supplicant.
 * If this is the first enabled network in wpa_supplicant, then 
 * wpa_supplicant will start association attempt immediately after receiving this 
 * request.
 *
 * @param wn_p  Pointer to a previously-configured wireless network in 
 * wpa_supplicant.
 *
 * @note Wpa_supplicant supports multiple wireless networks.  If 
 * multiple networks are enabled in wpa_supplicant, then no guarantee that which 
 * network will be joined successfully.  Wpa_supplicant supports 'priority' 
 * attribute for every wireless network.  However, it is not currently supported 
 * by this API. But it can be added easily.
 * 
 * @retval  0     Request is successful
 * @retval  < 0   Request failed
 *
 * An example usage for a WPA-PSK wireless network would be:
 *
 * @code
 *  char ssid[32+1];
 *  WLAN_NETWORK config_wlan_network;   // Use only one WLAN_NETWORK variable assuming we always want only one network.
 *  strcpy(ssid, "MyWirelessNetwork");
 *
 *  wlan_security = WIFI_WPA;
 *  wep_tx_keyidx = 0;
 *  connect_ssidlen = strlen(ssid);
 *  memcpy(connect_ssid, ssid, connect_ssidlen);
 *
 *  strcpy(connect_psk, "12345678");
 *
 *  if (!config_wlan_network.alloc)
 *      wpa_WlanNetworkAdd(&config_wlan_network); // Create a network with empty configuration in wpa_supplicant
 *  else
 *  {
 *      // config_wlan_network is already configured for an existing network in 
 *      // wpa_supplicant.  Remove it first from wpa_supplicant.
 *      // Assume we don't want to have more than one network in wpa_supplicant.
 *      wpa_WlanNetworkRemove(&config_wlan_network);
 *      wpa_WlanNetworkAdd(&config_wlan_network); // Use config_wlan_network again to create a new network.
 *  }
 *  if (config_wlan_network.alloc)
 *  { 
 *      // The network is successfully created in wpa_supplicant.
 *      // Go set the configuration of the network in wpa_supplicant
 *      // based on wlan_security, connect_ssid, connect_ssidlen, and connect_psk, ..etc.
 *      wpa_WlanNetworkSet(&config_wlan_network);
 *      wpa_WlanNetworkConnect(&config_wlan_network); // Enable the network.
 *  }		
 * @endcode
 */
int wpa_WlanNetworkConnect(WLAN_NETWORK *wn_p)
{
    char buf[32], cmd[32];
    size_t len;
    int ret;
    short is_associated = FALSE;
	
	mpDebugPrint("wpa_WlanNetworkConnect");

    if (!wn_p->mode && netif_carrier_ok(&NicArray[NIC_INDEX_WIFI])) {
        NicArray[NIC_INDEX_WIFI].flags = dev_get_flags(&NicArray[NIC_INDEX_WIFI]);
        is_associated = TRUE;
        MP_DEBUG("NicArray[NIC_INDEX_WIFI]=%x", NicArray[NIC_INDEX_WIFI].flags);
    }

    len = sizeof buf;
    if (!wn_p->enabled)
    {
        snprintf(cmd, sizeof cmd, "ENABLE_NETWORK %d", wn_p->network_id);
        ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
        if (ret < 0)
        {
            MP_DEBUG("ENABLE_NETWORK failed ret=%d", ret);
            return -1;
        }
        if (len > 0)
        {
            buf[len] = '\0';
            MP_DEBUG("ENABLE_NETWORK ret=%s", buf);
            wn_p->enabled = TRUE;
        }
        else
            return -2;
    }

    if (is_associated)
    {
        MP_ALERT("%s: is_associated=%d", __FUNCTION__, is_associated);
        wpa_WlanNetworkReconnect();
    }
	#if Make_WPS
    if(WpsReconnect==TRUE)
		WpsReconnect = FALSE;
#endif
    return 0;
}

/**
 * Force wpa_supplicant to do a re-association with current
 * connected AP.
 *
 * @retval  0     Request is successful
 * @retval  < 0   Request failed
 */
int wpa_WlanNetworkReconnect(void)
{
    char buf[32], cmd[32];
    size_t len;
    int ret;

    len = sizeof buf;
    snprintf(cmd, sizeof cmd, "REASSOCIATE");
	ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
    if (ret < 0)
    {
        MP_DEBUG("REASSOCIATE failed ret=%d", ret);
        return -1;
    }
    if (len > 0)
    {
        buf[len] = '\0';
        MP_DEBUG("REASSOCIATE ret=%s", buf);
    }
    else
        return -2;
    return 0;
}

/**
 * Configure a previously-created wireless network in wpa_supplicant.
 *
 * A WLAN_NETWORK structure will be filled up based on some of the following global 
 * variables, depending on the security mode.
 *
 * @li     <b>wlan_security</b>
 * @li     <b>connect_ssid</b>
 * @li     <b>connect_ssidlen</b>
 * @li     <b>wep_tx_keyidx</b>
 * @li     <b>wep_key</b>
 * @li     <b>wep_key_len</b>
 * @li     <b>connect_psk</b>
 * @li     <b>wlan_mode</b>
 * @li     <b>wlan_channel</b>
 *
 * This function can be modified depending on how a wireless network
 * is represented in UI subsystem.
 *
 * @param wn_p  Pointer to a previously-created wireless network in 
 * wpa_supplicant.
 *
 * @note The variable, <b>key_mgmt</b>, is not used anymore.
 * @note Before calling this function, all relevant variables, depending on
 * <b>wlan_security</b>, should be set correctly.
 * @todo Use a single structure to hold all these variables.
 * 
 * @retval  0     Request is successful
 * @retval  < 0   Request failed
 */
int wpa_WlanNetworkSet(WLAN_NETWORK *wn_p)
{
    int len;

    MP_DEBUG("wpa_WlanNetworkSet");

    if (!wn_p->alloc)
        return -4;

    wn_p->ssid_len = connect_ssidlen;
    memcpy(wn_p->ssid, connect_ssid, connect_ssidlen);
#if Make_WPS	
	WpsRun = 0;
#endif	
	mpDebugPrint("wlan_security %x",wlan_security);
	
	mpDebugPrint("wep_tx_keyidx %x",wep_tx_keyidx);
	if (wep_tx_keyidx >= 0 && wep_tx_keyidx < NUM_WEP_KEYS)
		mpDebugPrint("wep_key_len[wep_tx_keyidx] %x",wep_key_len[wep_tx_keyidx]);
    if (wlan_security == WIFI_WPA2 || wlan_security == WIFI_WPA)
    {
        wn_p->key_mgmt = WPA_KEY_MGMT_PSK;      /* use default */
		
		mpDebugPrint("connect_psk %s",connect_psk);
			
        if ((len=strlen(connect_psk)) == 0)
            return -1;
        wn_p->key_len = len;
        strcpy(wn_p->key, connect_psk);

        wn_p->wep_tx_key = -1;
        memset(&wn_p->wep_key_len, 0, sizeof wn_p->wep_key_len);
    }
    else if (wlan_security == WIFI_WEP)
    {
        if (wep_tx_keyidx < 0 || wep_tx_keyidx >= NUM_WEP_KEYS)
            return -2;
        if (wep_key_len[wep_tx_keyidx] > MAX_WEP_KEY_LEN)
            return -3;
        wn_p->key_len = 0;
        wn_p->key_mgmt = WPA_KEY_MGMT_NONE;     /* WPA is not used */

        memset(&wn_p->wep_key_len, 0, sizeof wn_p->wep_key_len);

        wn_p->wep_tx_key = wep_tx_keyidx;
        wn_p->wep_key_len[wep_tx_keyidx] = wep_key_len[wep_tx_keyidx];
        memcpy((char *)&wn_p->wep_key[wep_tx_keyidx], (char *)&wep_key[wep_tx_keyidx], wep_key_len[wep_tx_keyidx]);
		
		mpDebugPrint("connect_wep %s",wn_p->wep_key[wep_tx_keyidx]);
    }
    else                                        /* no security */
    {
        wn_p->key_len = 0;
        wn_p->key_mgmt = WPA_KEY_MGMT_NONE;     /* WPA is not used */
        wn_p->wep_tx_key = -1;
        memset(&wn_p->wep_key_len, 0, sizeof wn_p->wep_key_len);
    }

    if (wlan_mode)
    {
        wn_p->mode = 1;
        if (wlan_channel > 0)
            wn_p->frequency = m_ieee80211_channel_to_frequency(wlan_channel);
        doSetApScan(2);
    }
    else
    {
        wn_p->mode = 0;
        doSetApScan(1);
    }
    return doSetNetwork(wn_p);
}

/**
 * Create a network in wpa_supplicant with empty configuration. 
 *
 * If created successful, then wpa_supplicant's network ID is returned.  
 * The caller has to provide the pointer to a buffer in a WLAN_NETWORK structure.
 * This structure will be zeroed out and, if successful, the <b>network_id</b> field of
 * the WLAN_NETWORK structure is set to the network ID.  The <b>alloc</b> field is set
 * to TRUE.
 *
 * @param wn_p  Pointer to a buffer in a WLAN_NETWORK structure
 *
 * @retval  >= 0    A network is created successfully and its network ID is returned.
 * @retval  < 0     Request failed
 */
int wpa_WlanNetworkAdd(WLAN_NETWORK *wn_p)
{
    char cmd[24], buf[24];
    int len, ret;
    int id;                                     /* network ID */

    if (!wpa_ctrl_ctx)
        wpa_ctrl_ctx = wpa_ctrl_init();

    if (wn_p->alloc)
    {
        MP_ASSERT(0);
        return -1;
    }

	memset(wn_p, 0, sizeof *wn_p);

    len = sizeof buf;
    snprintf(cmd, sizeof cmd, "ADD_NETWORK");
    cmd[23] = '\0';
	ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
    if (ret < 0)
    {
        MP_DEBUG("wpa_WlanNetworkAdd: ADD_NETWORK failed ret=%d", ret);
        return -2;
    }

    if (len > 0)
    {
        buf[len] = '\0';
        MP_DEBUG("ADD_NETWORK  ret=%s", buf);
        id = atoi(buf);                         /* network id */
        if (id >= 0)
        {
            wn_p->network_id = id;
            wn_p->alloc = TRUE;
        }
        return id;
    }
    else
        return -3;
}

/**
 * Remove a network from wpa_supplicant.
 *
 * @param wn_p  Pointer to a previously-created wireless network in 
 * wpa_supplicant.
 *
 * @retval  0    Request is successful
 * @retval  < 0  Request failed
 */
int wpa_WlanNetworkRemove(WLAN_NETWORK *wn_p)
{
    char buf[32], cmd[32];
    size_t len;
    int ret;

    if (wn_p->alloc)
    {
        len = sizeof buf;
        snprintf(cmd, sizeof cmd, "REMOVE_NETWORK %d", wn_p->network_id);
        ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
        if (ret < 0)
        {
            MP_DEBUG("REMOVE_NETWORK failed ret=%d", ret);
            return -1;
        }
        if (len > 0)
        {
            buf[len] = '\0';
            MP_DEBUG("REMOVE_NETWORK ret=%s", buf);
            wn_p->alloc = FALSE;
        }
        else
            return -2;
    }

    return 0;
}
/**
 * Disable a network in wpa_supplicant.
 *
 * @param wn_p  Pointer to a previously-created wireless network in 
 * wpa_supplicant.
 *
 * @retval  0    Request is successful
 * @retval  < 0  Request failed
 */
int wpa_WlanNetworkDisable(WLAN_NETWORK *wn_p)
{
    char buf[32], cmd[32];
    size_t len;
    int ret;

    if (wn_p->enabled)
    {
        len = sizeof buf;
        snprintf(cmd, sizeof cmd, "DISABLE_NETWORK %d", wn_p->network_id);
        ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
        if (ret < 0)
        {
            MP_DEBUG("DISABLE_NETWORK failed ret=%d", ret);
            return -1;
        }
        if (len > 0)
        {
            buf[len] = '\0';
            MP_DEBUG("DISABLE_NETWORK ret=%s", buf);
            wn_p->enabled = FALSE;
        }
        else
            return -2;
    }

    return 0;
}

/** @} */

static int doSetNetwork(WLAN_NETWORK *wn_p)
{
    int ret, id = wn_p->network_id;
    size_t len;
    char *cmd = NULL, *buf = NULL;
    short key_required = FALSE;
    short i;

    mpDebugPrint("doSetNetwork");
    mpDebugPrint("wn_p->wep %x",wn_p->wep_tx_key);

    if (!wpa_ctrl_ctx)
        wpa_ctrl_ctx = wpa_ctrl_init();

    if (!wpa_ctrl_ctx)
    {
        MP_ALERT("out of memory");
        goto done;
    }

#define BUF_LEN 256
    cmd = (char *)mpx_Malloc(BUF_LEN);
    if (!cmd)
        goto done;

    buf = (char *)mpx_Malloc(BUF_LEN);
    if (!buf)
        goto done;

    len = BUF_LEN;
#if 0
    snprintf(cmd, BUF_LEN, "SET_NETWORK %d ssid \"%s\"",id, wn_p->ssid); /* ASCII string SSID needs double quotation */
#else
    mpDebugPrint("SET_NETWORK %d ssid %s",id, wn_p->ssid); 

    if (!ascii_to_hex(wn_p->ssid, buf, wn_p->ssid_len))
        goto done;
    buf[wn_p->ssid_len * 2] = '\0';

    snprintf(cmd, BUF_LEN, "SET_NETWORK %d ssid %s",id, buf);
#endif
	ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
    if (ret < 0)
    {
        mpDebugPrint("SET_NETWORK ssid failed ret=%d", ret);
        goto done;
    }
    if (len > 0)
    {
        buf[len] = '\0';
        mpDebugPrint("SET_NETWORK  ret=%s", buf);
    }
	
	mpDebugPrint("\nwn_p->proto %x\n",wn_p->proto);

    if (wn_p->proto)
    {
        len = BUF_LEN;
        buf[0] = '\0';
        if (wn_p->proto & WPA_PROTO_WPA)
            strcat(buf, "WPA");
        if (wn_p->proto & WPA_PROTO_RSN)
            strcat(buf, " RSN");
        snprintf(cmd, BUF_LEN, "SET_NETWORK %d proto %s",id, buf);
        ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
        if (ret < 0)
        {
            mpDebugPrint("SET_NETWORK proto failed ret=%d", ret);
            goto done;
        }
        if (len > 0)
        {
            buf[len] = '\0';
            mpDebugPrint("SET_NETWORK  ret=%s", buf);
        }
    }
	mpDebugPrint("\nwn_p->key_mgmt %x\n",wn_p->key_mgmt);
    if (wn_p->key_mgmt)
    {
        len = BUF_LEN;
        buf[0] = '\0';
        if (wn_p->key_mgmt & WPA_KEY_MGMT_NONE)
        {
            strcat(buf, "NONE");
        }
        if (wn_p->key_mgmt & WPA_KEY_MGMT_PSK)
        {
            strcat(buf, " WPA-PSK");
            key_required = TRUE;
        }
		
			
		snprintf(cmd, BUF_LEN, "SET_NETWORK %d key_mgmt %s",id, buf);
		ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
		if (ret < 0)
		{
			mpDebugPrint("SET_NETWORK key_mgmt failed ret=%d", ret);
			goto done;
		}
		if (len > 0)
		{
			buf[len] = '\0';
			mpDebugPrint("SET_NETWORK key_mgmt ret=%s", buf);
		}
    }
    if (key_required && wn_p->key_len > 0)
    {
        
		mpDebugPrint("wn_p->key_len =%d", wn_p->key_len);
		wn_p->key[wn_p->key_len] = '\0';
        len = BUF_LEN;
        snprintf(cmd, BUF_LEN, "SET_NETWORK %d psk \"%s\"",id, wn_p->key); /* ASCII string needs double quotation */
        ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
        if (ret < 0)
        {
            mpDebugPrint("SET_NETWORK psk failed ret=%d", ret);
            goto done;
        }
        if (len > 0)
        {
            buf[len] = '\0';
            mpDebugPrint("SET_NETWORK psk ret=%s", buf);
        }
    }

    for (i=0; i < NUM_WEP_KEYS; i++)
    {
        if ((len=wn_p->wep_key_len[i]) > 0)
        {
            if (len == 5 || len == 13)          /* in ASCII string */
            {
                if (!ascii_to_hex(wn_p->wep_key[i], buf, len))
                    goto done;
                snprintf(cmd, BUF_LEN, "SET_NETWORK %d wep_key%d %s",id, i, buf);
            }
            else                                /* in hex digit string */
                snprintf(cmd, BUF_LEN, "SET_NETWORK %d wep_key%d %s",id, i, wn_p->wep_key[i]);

            len = BUF_LEN;
            ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
            if (ret < 0)
            {
                mpDebugPrint("SET_NETWORK wep_key failed ret=%d", ret);
                goto done;
            }
            if (len > 0)
            {
                buf[len] = '\0';
                mpDebugPrint("SET_NETWORK wep_key ret=%s", buf);
            }
        }
    }

    if (wn_p->wep_tx_key >= 0)
    {
        snprintf(cmd, BUF_LEN, "SET_NETWORK %d wep_tx_keyidx %d",id,wn_p->wep_tx_key);

        len = BUF_LEN;
        ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
        if (ret < 0)
        {
            mpDebugPrint("SET_NETWORK wep_tx_keyidx failed ret=%d", ret);
            goto done;
        }
        if (len > 0)
        {
            buf[len] = '\0';
            mpDebugPrint("SET_NETWORK wep_tx_keyidx ret=%s", buf);
        }
    }

    if (wn_p->mode)
    {
        len = BUF_LEN;
        buf[0] = '\0';
        snprintf(cmd, BUF_LEN, "SET_NETWORK %d mode %d",id, wn_p->mode);
        ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
        if (ret < 0)
        {
            mpDebugPrint("SET_NETWORK mode failed ret=%d", ret);
            goto done;
        }
        if (len > 0)
        {
            buf[len] = '\0';
            MP_DEBUG("SET_NETWORK mode ret=%s", buf);
        }

        if (wn_p->frequency)
        {
            len = BUF_LEN;
            buf[0] = '\0';
            snprintf(cmd, BUF_LEN, "SET_NETWORK %d frequency %d",id, wn_p->frequency);
            ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
            if (ret < 0)
            {
                mpDebugPrint("SET_NETWORK frequency failed ret=%d", ret);
                goto done;
            }
            if (len > 0)
            {
                buf[len] = '\0';
                MP_DEBUG("SET_NETWORK frequency ret=%s", buf);
            }
        }
    }

    if (cmd)
        mpx_Free(cmd);
    if (buf)
        mpx_Free(buf);
    return 0;

done:
    mpDebugPrint("doSetNetwork returns error");
    if (cmd)
        mpx_Free(cmd);
    if (buf)
        mpx_Free(buf);
    return -1;
}

static int doSetApScan(int ap_scan)
{
    int ret;
    size_t len;
    char *cmd = NULL, *buf = NULL;

    cmd = (char *)mpx_Malloc(BUF_LEN);
    if (!cmd)
        goto done;

    buf = (char *)mpx_Malloc(BUF_LEN);
    if (!buf)
        goto done;

    len = BUF_LEN;

    snprintf(cmd, BUF_LEN, "AP_SCAN %d",ap_scan);
    ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
    if (ret < 0)
    {
        mpDebugPrint("AP_SCAN failed ret=%d", ret);
        goto done;
    }
    if (len > 0)
    {
        buf[len] = '\0';
        mpDebugPrint("AP_SCAN ret=%s", buf);
    }

    if (cmd)
        mpx_Free(cmd);
    if (buf)
        mpx_Free(buf);
    return 0;

done:
    if (cmd)
        mpx_Free(cmd);
    if (buf)
        mpx_Free(buf);
    return -1;
}

#if Make_WPS
int wpa_WPSGenerate_PIN(BYTE *wpspin)
{
		int ret;
		size_t len;
		char *cmd = NULL;
		short i;
		BYTE buf[8];
	
		MP_DEBUG("wpa_WPSGenerate_PIN");
	
		if (!wpa_ctrl_ctx)
			wpa_ctrl_ctx = wpa_ctrl_init();
	
		if (!wpa_ctrl_ctx)
		{
			MP_ALERT("out of memory");
			goto done;
		}
	
#define BUF_LEN 256
		cmd = (char *)mpx_Malloc(BUF_LEN);
		if (!cmd)
			goto done;
	
			len = BUF_LEN;
			buf[0] = '\0';
			strcat(buf, "any");
			
			snprintf(cmd, BUF_LEN, "WPS_PIN %s",buf);
			ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
			if (ret < 0)
			{
				MP_DEBUG("WPS_PIN any failed ret=%d", ret);
				goto done;
			}
			if (len > 0)
			{
				buf[len] = '\0';
				MP_DEBUG("WPS_PIN any ret=%s", buf);
				memcpy(wpspin,buf,8);
			}
	
		if (cmd)
			mpx_Free(cmd);
		return 0;
	
	done:
		if (cmd)
			mpx_Free(cmd);
		return -1;
}
int wpa_WPS_PBC(WLAN_NETWORK *wn_p)
{
		int ret, id = wn_p->network_id;
		size_t len;
		char *cmd = NULL, *buf = NULL;
		short i;
		BYTE string[32];
		MP_DEBUG("wpa_WPS_PBC");
	
		if (!wpa_ctrl_ctx)
			wpa_ctrl_ctx = wpa_ctrl_init();
	
		if (!wpa_ctrl_ctx)
		{
			MP_ALERT("out of memory");
			goto done;
		}
	
#define BUF_LEN 256
		cmd = (char *)mpx_Malloc(BUF_LEN);
		if (!cmd)
			goto done;
	
		buf = (char *)mpx_Malloc(BUF_LEN);
		if (!buf)
			goto done;
	//Set SSID
		len = BUF_LEN;
		mpDebugPrint("SET_NETWORK %d ssid %s",id, wn_p->ssid); 
	
		if (!ascii_to_hex(wn_p->ssid, buf, wn_p->ssid_len))
			goto done;
		buf[wn_p->ssid_len * 2] = '\0';
	
		snprintf(cmd, BUF_LEN, "SET_NETWORK %d ssid %s",id, buf);
		ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
		if (ret < 0)
		{
			mpDebugPrint("SET_NETWORK ssid failed ret=%d", ret);
			goto done;
		}
		if (len > 0)
		{
			buf[len] = '\0';
			mpDebugPrint("SET_NETWORK  ret=%s", buf);
		}
		//Set key mgmt
        len = BUF_LEN;
        buf[0] = '\0';
		
		strcat(buf, " WPS");
		wn_p->key_mgmt = WPA_KEY_MGMT_WPS;
			
		snprintf(cmd, BUF_LEN, "SET_NETWORK %d key_mgmt %s",id, buf);
		ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
		if (ret < 0)
		{
			mpDebugPrint("SET_NETWORK key_mgmt failed ret=%d", ret);
			goto done;
		}
		if (len > 0)
		{
			buf[len] = '\0';
			mpDebugPrint("SET_NETWORK key_mgmt ret=%s", buf);
		}
		//Set WPS PBC
		{
			len = BUF_LEN;
			buf[0] = '\0';
			for(i=0;i<6;i++)
			{
				HexString(string,wn_p->bssid[i],2);
				strcat(buf, string);
				if(i<5)
					strcat(buf, ":");
			}
			snprintf(cmd, BUF_LEN, "WPS_PBC %s",buf);
			ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
			if (ret < 0)
			{
				MP_DEBUG("WPS_PBC any failed ret=%d", ret);
				goto done;
			}
			if (len > 0)
			{
				buf[len] = '\0';
				MP_DEBUG("WPS_PBC any ret=%s", buf);
			}
		}
	
		if (cmd)
			mpx_Free(cmd);
		if (buf)
			mpx_Free(buf);
		return 0;
	
	done:
		MP_DEBUG("WPS_PBC returns error");
		if (cmd)
			mpx_Free(cmd);
		if (buf)
			mpx_Free(buf);
		return -1;
}
extern BYTE WpsPinCode[8];

int wpa_WPS_PIN(WLAN_NETWORK *wn_p)
{
		int ret, id = wn_p->network_id;
		size_t len;
		char *cmd = NULL, *buf = NULL;
		short i;
		BYTE string[32];
		MP_DEBUG("wpa_WPS_PIN");
	
		if (!wpa_ctrl_ctx)
			wpa_ctrl_ctx = wpa_ctrl_init();
	
		if (!wpa_ctrl_ctx)
		{
			MP_ALERT("out of memory");
			goto done;
		}
	
#define BUF_LEN 256
		cmd = (char *)mpx_Malloc(BUF_LEN);
		if (!cmd)
			goto done;
	
		buf = (char *)mpx_Malloc(BUF_LEN);
		if (!buf)
			goto done;
	
		{
			len = BUF_LEN;
			buf[0] = '\0';
			for(i=0;i<6;i++)
			{
				HexString(string,wn_p->bssid[i],2);
				strcat(buf, string);
				if(i<5)
					strcat(buf, ":");
			}
			
			snprintf(cmd, BUF_LEN, "WPS_PIN %s %c%c%c%c%c%c%c%c",buf,WpsPinCode[0],WpsPinCode[1]
			,WpsPinCode[2],WpsPinCode[3],WpsPinCode[4],WpsPinCode[5],WpsPinCode[6],WpsPinCode[7]);
			ret = wpa_ctrl_request(wpa_ctrl_ctx, cmd, strlen(cmd), buf, &len);
			if (ret < 0)
			{
				MP_DEBUG("WPS_PIN any failed ret=%d", ret);
				goto done;
			}
			if (len > 0)
			{
				buf[len] = '\0';
				MP_DEBUG("WPS_PIN any ret=%s", buf);
			}
		}
	
		if (cmd)
			mpx_Free(cmd);
		if (buf)
			mpx_Free(buf);
		return 0;
	
	done:
		MP_DEBUG("WPS_PIN returns error");
		if (cmd)
			mpx_Free(cmd);
		if (buf)
			mpx_Free(buf);
		return -1;
}
#endif
