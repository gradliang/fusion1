#ifndef ZYDAS_COMMON_H
#define ZYDAS_COMMON_H

#define ZD_IOCTL_WPA			(SIOCDEVPRIVATE + 1)
#define ZD_IOCTL_PARAM			(SIOCDEVPRIVATE + 2)
#define ZD_IOCTL_GETWPAIE       (SIOCDEVPRIVATE + 3)
#if ZDCONF_WPS_SUPPORT == 1
#define ZD_IOCTL_GETWSCIE       (SIOCDEVPRIVATE + 5)
#define ZD_IOCTL_SETAPPIEBUF        (SIOCDEVPRIVATE + 6)
#endif

#define ZD_PARAM_ROAMING		0x0001
#define ZD_PARAM_PRIVACY		0x0002
#define ZD_PARAM_WPA			0x0003
#define ZD_PARAM_COUNTERMEASURES	0x0004
#define ZD_PARAM_DROPUNENCRYPTED	0x0005
#define ZD_PARAM_AUTH_ALGS		0x0006
#define ZD_PARAM_WPS_FILTER     0x0007
                                                                                                                                                                
#define ZD_CMD_SET_ENCRYPT_KEY		0x0001
#define ZD_CMD_SET_MLME			0x0002
#define ZD_CMD_SCAN_REQ			0x0003
#define ZD_CMD_SET_GENERIC_ELEMENT	0x0004

#define ZD_FLAG_SET_TX_KEY              0x0001

#define ZD_GENERIC_ELEMENT_HDR_LEN \
((int) (&((struct zydas_wlan_param *) 0)->u.generic_elem.data))

#define ZD_CRYPT_ALG_NAME_LEN		16
#define ZD_MAX_KEY_SIZE			32
#define ZD_MAX_GENERIC_SIZE		64

#define IEEE80211_ADDR_LEN      6
#define IEEE80211_MAX_IE_SIZE   256
#if ZDCONF_WPS_SUPPORT == 1
#define IEEE80211_MAX_WSC_IE        256
#endif

/* structure definition */

struct zydas_wlan_param {
	u32 cmd;
	u8 sta_addr[ETH_ALEN];
	union {
		struct {
			u8 alg[ZD_CRYPT_ALG_NAME_LEN];
			u32 flags;
			u32 err;
			u8 idx;
			u8 seq[8]; /* sequence counter (set: RX, get: TX) */
			u16 key_len;
			u8 key[ZD_MAX_KEY_SIZE];
		} crypt;
		struct {
			u32 flags_and;
			u32 flags_or;
		} set_flags_sta;
		struct {
			u8 len;
			u8 data[ZD_MAX_GENERIC_SIZE];
		} generic_elem;
		struct {
#define MLME_STA_DEAUTH 0
#define MLME_STA_DISASSOC 1
			u16 cmd;
			u16 reason_code;
		} mlme;
		struct {
			u8 ssid_len;
			u8 ssid[32];
		} scan_req;
	} u;
};
struct ieee80211req_wpaie
{
    u8 wpa_macaddr[IEEE80211_ADDR_LEN];
    u8 wpa_ie[IEEE80211_MAX_IE_SIZE];
};

#if ZDCONF_WPS_SUPPORT == 1 && ZDCONF_AP_SUPPORT == 1
struct ieee80211req_wscie {
    u8 wsc_macaddr[IEEE80211_ADDR_LEN];
    u8 wsc_ie[IEEE80211_MAX_WSC_IE];
};

/* added APPIEBUF related definations */
enum{
    IEEE80211_APPIE_FRAME_BEACON     = 0,
    IEEE80211_APPIE_FRAME_PROBE_REQ  = 1,   
    IEEE80211_APPIE_FRAME_PROBE_RESP = 2,   
    IEEE80211_APPIE_FRAME_ASSOC_REQ  = 3,   
    IEEE80211_APPIE_FRAME_ASSOC_RESP = 4,   
    IEEE80211_APPIE_NUM_OF_FRAME     = 5   
};

#define IEEE80211_APPIE_MAX        1024
struct ieee80211req_getset_appiebuf {
    u8 app_frmtype; /*management frame type for which buffer is added*/
    u8 app_buflen;  /*application supplied buffer length */
    u8 app_buf[IEEE80211_MAX_WSC_IE];
};

#endif // support WPS


#endif

