
// define this module show debug message or not,  0 : disable, 1 : enable
#define LOCAL_DEBUG_ENABLE 1

#include "global612.h"
#include "mpTrace.h"
#include "memory.h"

#include "linux/types.h"
#include "linux/list.h"
#include "uip.h"
#include "net_sys.h"
#include "timer.h"
#include "taskid.h"
#include "netware.h"
#include "devio.h"
#include "net_autosearch.h"
#include "net_nic.h"
#include "net_dhcp.h"
#include "..\..\xml\include\netfs.h"
#include "SysConfig.h"

#include "net_device.h"
#include "wlan_sys.h"
#include "typedef.h"
#include "wireless.h"
#include "wlan_common.h"
#include <linux/if_ether.h>
#include "ndebug.h"

extern struct net_device NicArray[NIC_DRIVER_MAX];
extern BYTE g_bXpgStatus;
extern SERVER_BROWSER ServerBrowser;
extern BYTE g_Net_SetupList;
extern BYTE g_Net_WifiStatus;
extern ST_SYSTEM_CONFIG *g_psSystemConfig;
extern WIFI_AP_BROWSER WiFiApBrowser;
extern WLAN_NETWORK config_wlan_network;
void EnableNetWareTask();


#if  (Make_SDIO == MARVELL_WIFI)
extern wlan_private *wlanpriv;
#endif


#define NUM_WEP_KEYS 4
#define MAX_WEP_KEY_LEN 16
#define WIFI_NO_SECURITY		0
#define WIFI_WEP                1
#define WIFI_WPA                2
#define WPA_KEY_MGMT_PSK 2
extern char connect_ssid[];
extern int connect_ssidlen;
extern int key_mgmt;
extern int wlan_security;
extern char wep_key[NUM_WEP_KEYS][MAX_WEP_KEY_LEN];
extern short wep_key_len[NUM_WEP_KEYS];
extern int wep_tx_keyidx;
extern char connect_psk[];

static int __Net_Recv_Http_Data2(BYTE *url, BYTE type, DWORD timeout, char *, Net_App_State *);
static int __Net_Recv_Image_Data2(BYTE *url, BYTE type, DWORD timeout, char *if_name, Net_App_State *app);
static XML_ImageBUFF_link_t * get_xml_imagebuffer2(Net_App_State *app);

/*----------------------- Network API -------------------------------*/

/*
 * @param if_name Specify the outgoing interface for the connection if its value is non-NULL.
 *
 * CURL Error codes (curl_result):
 *   @li <b>CURLE_INTERFACE_FAILED</b>        The specified outgoing interface is not up.
 */
int Net_Recv_Data2(Net_App_State *app, BYTE *url, BYTE type, DWORD timeout, char *if_name)
{
    int total_data_len;
    void *xp, *prev;
	DRIVE *w_sDrv,*sDrv;
	STREAM *w_shandle;
	int ret;
	XML_BUFF_link_t* buffer;
	BYTE *url1;

	mpDebugPrint("[%s]: bXpgStatus = %d, App's state = %x", __func__,g_bXpgStatus,app->dwState);
    MP_DEBUG("[Net_Recv_Data2] URL=\r\n%s", url);


	if (app->dwState & NET_RECVHTTP) 
    { 
		return __Net_Recv_Http_Data2(url, type, timeout, if_name, app);
	}
	else if (app->dwState &  NET_RECVBINARYDATA)
	{
		return __Net_Recv_Image_Data2(url, type, timeout, if_name, app);
	}

#if Make_WLS
	if (app->dwState &  NET_WLS)
	{
		total_data_len = __Net_Recv_Image_Data2(url, type, timeout, if_name, app);
		url1 = windows_live_get_photo_url(xp->BUFF,total_data_len);
		Xml_BUFF_free(NET_WLS);
		Xml_BUFF_init(NET_WLS);
		app->flags = 0;
		total_data_len = Get_Image_File2(url1, xp->BUFF,timeout, if_name, app);
		MP_DEBUG("333333 Net_Recv_Data URL=%s  ;file len=%d", url1, total_data_len);
		if (total_data_len == 0)
			return -1;
		else
			return total_data_len;
	}
#endif
}


/**
 * @ingroup    NET_HTTP
 *
 * Transfer a URL.
 *
 * This is a high-level HTTP transfer API.  It works with XML buffers
 * (Xml_BUFF_init and Xml_BUFF_free) and the global variable, App_State.
 * The downloaded JPEG or XML file is stored in chained buffers and pointed
 * by App_State.XML_BUF (for text file, such as XML/HTML) or
 * App_State.XML_BUF1 (for binary file, such as JPEG).
 *
 * @param url  URL to transfer
 * @param type  No longer used
 * @param timeout Time in seconds to wait for the HTTP transfer to complete before
 *		timing out (if 0 the wait is forever)
 *
 * @retval >0    Number of bytes received for this URL if successful.
 * @retval -1    An error occurs
 *
 * @note Since CURL has a default timeout value internally, this function
 *		will not actually wait forever if value of @p timeout is 0.
 *		The function returns if CURL times out.
 */
static int __Net_Recv_Http_Data2(BYTE *url, BYTE type, DWORD timeout, char *if_name, Net_App_State *app)
{
	XML_BUFF_link_t * xp = app->ptr;
    int total_data_len;

	if (!xp)
	{
		xp = ext_mem_malloc(sizeof(XML_BUFF_link_t));
		if (! xp)
      {
			mpDebugPrint("%s-->malloc fail", __func__);
			return NULL;
      }
            memset(xp, 0, sizeof(XML_BUFF_link_t));
            app->ptr = app->XML_BUF = xp;
      	}
	else
	{
		while(xp->link)
			xp = xp->link;
	}

    app->flags = 0;
    if(if_name && (strcmp(if_name, "ppp0") == 0))
        app->flags |= HTTP_TRANSFER_PPP;
    total_data_len = Get_Image_File2(url, xp->BUFF, timeout, if_name, app);
    MP_DEBUG("111111 Net_Recv_Data URL=%s len=%d", url, total_data_len);

    if (total_data_len == 0)
    {
        //xpgCb_NetPhoto_Reconnet();

        return -1;
    }
    else
        return total_data_len;
}

static int __Net_Recv_Image_Data2(BYTE *url, BYTE type, DWORD timeout, char *if_name, Net_App_State *app)
{
    XML_ImageBUFF_link_t *xp = get_xml_imagebuffer2(app);
    int total_data_len;

    app->flags = 0;
    if(if_name && (strcmp(if_name, "ppp0") == 0))
        app->flags |= HTTP_TRANSFER_PPP;
    total_data_len = Get_Image_File(url, xp->BUFF, timeout, if_name, app);
    MP_DEBUG("222222 __Net_Recv_Image_Data2 URL=%s  ;file len=%d", url, total_data_len);

    if (total_data_len == 0)
    {
        return -1;
    }
    else
        return total_data_len;
}

static XML_ImageBUFF_link_t * get_xml_imagebuffer2(Net_App_State *app)
{
	XML_ImageBUFF_link_t * xp = app->qtr;

	if (!xp)
	{
		xp = ext_mem_malloc(sizeof(XML_ImageBUFF_link_t));
		if (! xp)
		{
			mpDebugPrint("%s-->malloc fail", __func__);
			return NULL;
		}
             memset(xp, 0, sizeof(XML_ImageBUFF_link_t));
             app->qtr = app->XML_BUF1 = xp;
	}
	else
	{
		while(xp->link)
			xp = xp->link;
	}

	return xp;
}

