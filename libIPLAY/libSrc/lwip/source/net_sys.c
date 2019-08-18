// define this module show debug message or not,  0 : disable, 1 : enable

#define LOCAL_DEBUG_ENABLE 1

#include "global612.h"
#include "mpTrace.h"

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
//#include "os_defs.h"
#include "typedef.h"
#include "wireless.h"
//AAADDDD
#include "..\..\..\..\libIPLAY\libSrc\netstream\INCLUDE\netstream.h"
#include "wlan_common.h"
#include <linux/if_ether.h>
#include "ndebug.h"
#include "log.h"

#define SELECT_AP 0

#ifdef NET_NAPI
#include "xpgNet.h"
#endif

int gcurl_filesize;
unsigned char  bwritetonand;
char upnp_flash_file_name[16];
char upnp_flash_file_ext[8];

#if NET_UPNP
void SSDPReplyTask(void);
void RunMiniServer(void);
void RunSSDPServer(void);
void UpnpdownloadTask(void);
extern int IXMLSemid;
extern int UPNPHandleLock;
#if	HAVE_UPNP_MEDIA_SERVER
extern unsigned char UPNP_Server_Event;
#endif
#endif

#if	Make_WEBSERVER
DRIVE *net_web_sDrv=NULL;
STREAM *net_web_shandle=NULL;
#endif

#pragma alignvar(4)
uip_eth_addr eth_addr;
NetWorkInfo getNetWorkInfo;

int atomic_sema;


/**
 * @ingroup    NET_HTTP
 *
 * @brief A global variable of Net_App_State structure to keep track of
 * a URL transfer.
 *
 * The resulting file (XML/HTML or JPEG) is stored
 * in chained buffers, pointed by App_State.XML_BUF1 or App_State.XML_BUF.
 *
 * This variable is used extensively in Xml_BUFF_init(), Xml_BUFF_free(), and
 * Net_Recv_Data() API functions..
 */
Net_App_State App_State;

/*
 * callback functions for network events
 */
static Net_Event *App_Net_Event;
static void *App_Data;

DWORD g_recvsize;
struct net_device g_net_device;
extern struct net_device NicArray[NIC_DRIVER_MAX];
timer periodic_timer, arp_timer;
BYTE myethaddr[6];
BYTE NetBackupDrvID;
extern BYTE g_bXpgStatus;
extern SERVER_BROWSER ServerBrowser;
extern BYTE g_Net_SetupList;
extern BYTE g_Net_WifiStatus;
extern ST_SYSTEM_CONFIG *g_psSystemConfig;
#if MAKE_XPG_PLAYER
extern WIFI_AP_BROWSER WiFiApBrowser;
#endif
extern WLAN_NETWORK config_wlan_network;
void webclient_newdata(void);
DWORD NetConfiged();
void NetDisConfiged();
void EnableNetWareTask();
void MP_Wifi_APInfo_init();
size_t net_buf_size_get(void *rmem);

#if 1
wifi_timer_t wifi_timer;
static BOOLEAN wifi_timer_expired(wifi_timer_t *t);
#endif

#if  (Make_SDIO == MARVELL_WIFI)
extern wlan_private *wlanpriv;
#endif

extern BOOLEAN net_ipv4_method_dhcp;
extern BYTE net_ipv4_fixed_address[16];
extern BYTE netcfg_ipv4_fixed_netmask[16];

#if USB_WIFI

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
#if (SHOW_FRAME_TEST || AUTO_CONNECTION)
extern int wlan_mode;
extern int wlan_channel;
#endif

/**
 * @ingroup WlanScan
 * @brief Callback function after scan is done.
 *
 * If it is set to a function, then it will be called in the wlan driver
 * after a scan is done.
 * Currently, NetScanDoneEventSet_Fn is used by ar2524 driver only.
 *
 * @note To reduce unnecessary overhead, set this variable to NULL if you are
 * not waiting for scan results.
 * During normal wireless operation, wlan driver may perform wireless scan
 * sometimes (e.g., after loss of a connection).
 */
void (*NetScanDoneEventSet_Fn)(void);
void NetScanDoneEventSet(void);

#if (DM9KS_ETHERNET_ENABLE || DM9621_ETHERNET_ENABLE)
void Ethernet_RecvTask(void);
void Ethernet_CheckArp(void);
extern u8 gethernet_is_link_up;
extern struct net_device* dmfe_dev;
#endif
/*
 * Callback function for hotplug.
 */
void (*NetDeviceAttachEvent_Fn)(WORD idVendor, WORD idProduct, int attached);
void MyNetDeviceAttachEvent(WORD idVendor, WORD idProduct, int attached);

#ifdef HAVE_SMB
void SmbCli_Init(void);
bool set_mysocketaddress(char *ipaddr);
#endif

clock_time_t clock_time(void)
{

  	return mpx_SystemTickerGet();
}

void wlan_AP_Scan(struct net_device *dev)
{
	int ret;
	struct iwreq iwr;
	struct iw_scan_req req;
	MP_DEBUG("wlan_AP_Scan()...");
#if Make_USB
	MP_Wifi_APInfo_init();
#endif
	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, dev->name, IFNAMSIZ);

	if (ioctl(0, SIOCSIWSCAN, &iwr) < 0) {
		mpDebugPrint("ioctl[SIOCSIWSCAN]");
		ret = -1;
	}
}

/*
 * wlan_AP_Connect
 *
 * Connect to a wireless network by enabling the wlan network.
 *
 */
void wlan_AP_Connect(struct net_device *dev)
{
	MP_DEBUG("wlan_AP_Connect()...");
#if (Make_WPA == 1)
#if (MAKE_XPG_PLAYER == 0)
    wpa_WlanNetworkAdd(&config_wlan_network);
    wpa_WlanNetworkSet(&config_wlan_network);
#endif
    wpa_WlanNetworkConnect(&config_wlan_network);
#endif
}

/*
 * wlan_AP_Disconnect
 *
 * Disconnect from a wireless network by disabling the wlan network.
 *
 */
void wlan_AP_Disconnect(struct net_device *dev)
{
	MP_DEBUG("wlan_AP_Disconnect()...");
#if (Make_WPA == 1)
    wpa_WlanNetworkDisable(&config_wlan_network);
#endif
}

/*
 * wlan_AP_Remove
 *
 * Remove a wireless network from wpa_supplicant.
 *
 */
void wlan_AP_Remove(struct net_device *dev)
{
	MP_DEBUG("wlan_AP_Remove()...");
#if (Make_WPA == 1)
    wpa_WlanNetworkRemove(&config_wlan_network);
#endif
}

/*
 * wlan_AP_Disable
 *
 * Disable a wireless network in wpa_supplicant.
 *
 */
void wlan_AP_Disable(struct net_device *dev)
{
	MP_DEBUG("wlan_AP_Disable()...");
#if (Make_WPA == 1)
    wpa_WlanNetworkDisable(&config_wlan_network);
#endif
}

void ShowWirelessAPTable(void)
{
#if MAKE_XPG_PLAYER
	g_Net_WifiStatus = NET_STATUS_NULL;
       g_bXpgStatus = XPG_MODE_NET_AP;
       xpgSearchAndGotoPage("NetworkAPTab", 12);
       xpgUpdateStage();
#endif	   
}

void MP_Wifi_APInfo_init()
{
#if MAKE_XPG_PLAYER
	memset(&WiFiApBrowser, 0x00, sizeof WiFiApBrowser);
#endif
}

#else //SDIO WIFI
void Net_Null(void)
{

}

void NetApRegCallBack(void (*NetCallBack) (void))
{
  	App_State.Net_AP_CallBack = NetCallBack;
}


void NetApClearCallBack(void)
{
  	App_State.Net_AP_CallBack = Net_Null;
}

clock_time_t clock_time(void)
{

  	return GetSysTime();
}

void tcpip_appinit(void)
{

    webclient_init();
    NetApClearCallBack();

}


void tcpip_output(void)
{


}

void NWArpTimerHandle()
{
/* ----------  not used for LWIP  ---------- */
}
#endif

/*---------------------------------------------------------------------------*/


int netReceive(void *pDev, struct sk_buff * skb)
{
    skb->u08NetIndex = NIC_INDEX_WIFI;
    skb->u16PayloadSize = skb->len - ETH_HLEN;
    skb->dev = pDev;
    return (int)NetPacketReceive(skb);
}

#if (USB_WIFI || PPP_ENABLE || Make_DM9621_ETHERNET || Make_DM9KS_ETHERNET)
extern BYTE wifi_device_type;

/**
 * @ingroup NET_BUFFER
 * @brief Allocate a network buffer
 *
 * The sk_buff structure is similar to Linux's sk_buff, but with
 * some unused fields removed.
 *
 * @retval  non-zero     A buffer is returned.
 * @retval  NULL  No buffer is avaiable.
 *
*/
#ifndef SKB_OPT
struct sk_buff *netBufAlloc(size_t sz)
{
    struct sk_buff *skb;
    skb = (struct sk_buff *)net_buf_mem_malloc(sizeof (struct sk_buff) + sz);
	//skb = (struct sk_buff *)mem_malloc(sizeof (struct sk_buff) + sz);
    if (skb == NULL)
    {
        MP_ALERT("netBufAlloc: no more buffer");
        MP_ASSERT(0);
        return NULL;
    }
	memset(skb, 0, offsetof(struct sk_buff, RESERVED));
	atomic_set(&skb->users, 1);
    skb->data = (unsigned char *)&skb->Media;
    skb->head = (unsigned char *)&skb->RESERVED;
    skb->tail = skb->data;
    skb->end = (u8 *)skb + net_buf_size_get((void *)skb);
    return skb;
}
#else
struct sk_buff *netBufAlloc(size_t sz)
{
    struct sk_buff *skb;
    skb = (struct sk_buff *)mpx_Malloc(sizeof (struct sk_buff));
    void *data = net_buf_mem_malloc(128 + sz);
    if (skb == NULL || data == NULL)
    {
        MP_ALERT("netBufAlloc: no more buffer");
        MP_ASSERT(0);
        goto error;
    }
	memset(skb, 0, offsetof(struct sk_buff, RESERVED));
    skb->truesize = sz + sizeof(struct sk_buff);
	atomic_set(&skb->users, 1);
    skb->data = (unsigned char *)data + 128;
    skb->head = (unsigned char *)data;
    skb->tail = skb->data;
    skb->end = data + net_buf_size_get(data);
    skb->data_allocated = 1;
    return skb;
error:
    if (skb)
        mpx_Free(skb);
    if (data)
        net_buf_mem_free(data);
    return NULL;
}
#endif

#elif SDIO_WIFI_ENABLE
struct sk_buff *netBufAlloc(size_t sz)
{
    struct sk_buff *skb;
    skb = (struct sk_buff *)net_buf_mem_malloc(sz);
    if (skb == NULL)
    {
        MP_ALERT("netBufAlloc: no more buffer");
    }
    return skb;
}

#else

struct sk_buff *netBufAlloc(size_t sz)
{
    struct sk_buff *skb;
    skb = (struct sk_buff *)net_buf_mem_malloc(sz);
    if (skb == NULL)
    {
        MP_ALERT("netBufAlloc: no more buffer");
    }
    return skb;
}

#endif


#if 0	/* moved to net_nic.h */
void netBufFree(struct sk_buff *skb)
{
    net_buf_mem_free(skb);
	//mem_free(skb);
}
#endif

void SetNWPeriodicTimerEvent()
{
	App_State.dwNWEvent |= NET_EVENT_PERIODIC_TIMER;

}

void SetNWArpTimerEvent()
{
	App_State.dwNWEvent |= NET_EVENT_ARP_TIMER;

}


void ClrNWPeriodicTimerEvent()
{
	App_State.dwNWEvent &= (~NET_EVENT_PERIODIC_TIMER);

}

void ClrNWArpTimerEvent()
{
	App_State.dwNWEvent &= (~NET_EVENT_ARP_TIMER);

}

DWORD ChkNWPeriodicTimerEvent()
{
	return (App_State.dwNWEvent & NET_EVENT_PERIODIC_TIMER);

}

DWORD ChkNWArpTimerEvent()
{
	return (App_State.dwNWEvent & NET_EVENT_ARP_TIMER);

}



DWORD ChkNWTimerEvent()
{
	return (App_State.dwNWEvent & (NET_EVENT_PERIODIC_TIMER));

}
#if  (Make_SDIO == MARVELL_WIFI)
unsigned char intcnt = 0;
#endif

int nthcnt;
void NetTimerHandle()
{
    static int cnt;
	static int waitmcard = 0;
	static char first_jump = 0;
	if(!(App_State.dwState &  NET_ENABLE))
		return;
#if 0
    if ((cnt++ % 64) == 0)
	{
		UartOutText(".");
	}
#endif
	if(timer_expired(&periodic_timer))
	{
		timer_reset(&periodic_timer);
#if 0
		SetNWPeriodicTimerEvent();
    		//TaskWakeup(WIFI_MAIN_TASK);
		EventSet(WIFI_EVENT, EVENT_WIFI_INTERRUPT);
#else
		/*
		 * Move network timer processing to NetTask
		 * from wlan_service_main_thread.
		 */
#if (Make_SDIO == MARVELL_WIFI)
		intcnt++;
		if( intcnt > 5 )
		{
			intcnt = 0 ;
			wlanpriv->adapter->IntCounter++;
		}
#endif
		EventSet(NETWARE_EVENT, EVENT_NW_TIMER);
#if (Make_SDIO == MARVELL_WIFI) || (Make_SDIO == REALTEK_WIFI)
		EventSet(WIFI_EVENT, EVENT_WIFI_INTERRUPT);
#endif
//#if (Make_SDIO == REALTEK_WIFI)
		EventSet(WIFI_EVENT, EVENT_WIFI_TIMER);
//#endif
#endif
	}

	if(wifi_timer_expired(&wifi_timer))
		EventSet(WIFI_EVENT, EVENT_WIFI_TIMER);
}

extern int usb_timer_proc_in_progress;
static BOOLEAN wifi_timer_expired(wifi_timer_t *t)
{
    BOOLEAN expired = FALSE;
    long now;
    if (!t->expires)
        return FALSE;
    if (usb_timer_proc_in_progress)
        return FALSE;
    now = (long)clock_time();
    if (((now - (long)t->expires) >= 0))
        expired = TRUE;
    else if ((((long)t->expires - now) <= 4)) /* expires soon */
        expired = TRUE;

    return expired;
}

void wifi_timer_set(unsigned long next_expires)
{
    wifi_timer.expires = next_expires;
	if(wifi_timer_expired(&wifi_timer))
		EventSet(WIFI_EVENT, EVENT_WIFI_TIMER);
}

void NWPeriodicTimerHandle()
{
    extern U08 u08NetTimerId;
    void (*func)(void);
    ClrNWPeriodicTimerEvent();
    if (func = NWTimerHandler(u08NetTimerId))
        (*func)();
}

void ShowWirelessConnect(void)
{
#if MAKE_XPG_PLAYER

	if ((wifi_device_type == ETHERNET_USB_DEVICE_DM9621) || (g_bXpgStatus == XPG_MODE_NET_AP))
	{
		g_Net_WifiStatus = NET_STATUS_SUCCESS_CONNECT;
		g_bXpgStatus = XPG_MODE_NET_FUNC;
#ifdef NET_NAPI
        mpDebugPrint("XpgNet: State Machine is started");
		HsmOnStart(xpgNetHsm);
#endif
		mpDebugPrint("ShowWirelessConnect");
		xpgSearchAndGotoPage("Net_Func",8);
		xpgUpdateStage();
	}
#endif		

}

#if USB_WIFI
    static int driver_initialized = 0;              /* support USB hotplug */
#endif

void NetTask()
{
	uip_ipaddr_t ipaddr;
	int ret;
	DWORD dwNWEvent;
	DRIVE *sDrv;
	BYTE i;
	struct net_device *wifi_dev = &NicArray[NIC_INDEX_WIFI];
	struct net_device *ppp_dev = &NicArray[NIC_INDEX_PPP];
    int dev_flags = 0;
    BOOLEAN wait_scan;                          /* wait for scan results */


	//MP_DEBUG("NetTask start 1<==========================");
	mpDebugPrint("NetTask RUN");
	timer_set(&periodic_timer, CLOCK_SECOND/2);

	SemaphoreCreate(NET_CACHE_SEMA_ID, OS_ATTR_PRIORITY, 1);
	InitNetCache();
#if MAKE_XPG_PLAYER
	TaskYield();
#else
    TaskSleep(1);
#endif

	mpx_NetworkInit();//uip_init();

 //  Init_Net_APP_List();

	NetCtrlStart();
#if (DM9KS_ETHERNET_ENABLE || DM9621_ETHERNET_ENABLE)
	dmfe_dev = &NicArray[NIC_INDEX_ETHER];
	strcpy(dmfe_dev->name,"eth0");
    mpx_DhcpInit(dmfe_dev);
#endif

#if USB_WIFI
    mpx_DhcpInit(wifi_dev);
#endif

	while(1)
	{
  		//MP_DEBUG("NetTask start 3<=========================");
//		if(EventWait(NETWARE_EVENT, 0x00000fff, OS_EVENT_OR, &dwNWEvent) == OS_STATUS_OK)
		#if ((MAKE_XPG_PLAYER==0)&&HAVE_HTTP_SERVER)
		ret =EventWait(NETWARE_EVENT, 0xffffffff, OS_EVENT_OR, &dwNWEvent);
		#else
		ret =EventWaitWithTO(NETWARE_EVENT, 0xffffffff, OS_EVENT_OR, &dwNWEvent, 10*1000);
		#endif

		//mpDebugPrint("dwNWEvent %x",dwNWEvent);
		if(ret == OS_STATUS_TIMEOUT)
        {
            dwNWEvent = 0;
            continue;
        }
		if(ret == OS_STATUS_OK || ret == OS_STATUS_TIMEOUT)
		{
			if (dwNWEvent != EVENT_NW_TIMER)
			MP_DEBUG1("dwNWEvent = 0x%x",dwNWEvent);
			if(dwNWEvent & EVENT_NW_DISABLE)
			{
				dwNWEvent = 0;
				App_State.dwState &= ~NET_ENABLE;
			}
			if( dwNWEvent & EVENT_NW_ENABLE)
			{
				App_State.dwState |= NET_ENABLE;
			}
			if( dwNWEvent & EVENT_NW_DRIVER_UP )
			{
#if (DM9KS_ETHERNET_ENABLE || DM9621_ETHERNET_ENABLE)
				if (wifi_device_type == ETHERNET_USB_DEVICE_DM9621)
				{
					EnableNetWareTask();
					dmfe_dev->flags |= IFF_UP;
					rfc2863_policy(dmfe_dev);
				}
				else 
#endif
                if( !driver_initialized )
				{
#if USB_WIFI
					UartOutText("Wifi Driver up\n");

					EnableNetWareTask();
#if (HAVE_WPA_SUPPLICANT)
					wifi_dev->flags |= IFF_UP;
#endif
					rfc2863_policy(wifi_dev);
#if (HAVE_WPA_SUPPLICANT||Make_ADHOC||defined(HAVE_HOSTAPD))
					EventSet(WPA_EVENT, BIT0);
#endif
#if MAKE_XPG_PLAYER
					TaskYield();
#else
					TaskSleep(1);
#endif
					wait_scan = TRUE;
					driver_initialized++;
#if Make_CURL
#if MAKE_XPG_PLAYER
					http_proxy_config_get();
#endif
#endif
#endif
				}
			}
			if( dwNWEvent & EVENT_NW_DISCONNECT_AP)
			{
                wlan_AP_Disconnect(wifi_dev);
			}

			if( dwNWEvent & EVENT_NW_SCAN_REQ)
			{
				wait_scan = TRUE;
                NetScanDoneEventSet_Fn = NetScanDoneEventSet;
#if Make_USB
				MP_Wifi_APInfo_init();
#endif
				wlan_AP_Scan(wifi_dev);
			}

			if( dwNWEvent & EVENT_NW_SCAN_DONE)
			{
                if (wait_scan)
                {
                    wait_scan = FALSE;
                    UartOutText("NET: Scan done");
                    NetScanDoneEventSet_Fn = NULL;
                    if(wifi_device_type == WIFI_USB_DEVICE_AR2524)
					{

#if Make_USB == AR2524_WIFI||Make_USB == RALINK_AR2524_WIFI
#if MAKE_XPG_PLAYER
                    MP_Wifi_APScanResults(wifi_dev);
#endif
#if SELECT_AP
					WiFiApSetSSID("Matt_AP1");//For FPGA select AP
#endif
#if (SHOW_FRAME_TEST && Make_ADHOC && (Make_USB==3))
		wlan_AP_Config(NULL, "MPX_ADHOC");
		sentTo_testWiFi_task(3);
#endif
#endif
					}
                    else if (wifi_device_type == WIFI_USB_DEVICE_AR9271)
                    {
#if Make_USB == AR9271_WIFI
                        MP_ATH9K_APScanResults(wifi_dev);
#endif
#if AUTO_CONNECTION
		wlan_AP_Config(NULL,"Joyce_EDIMAX");/*Need to set "" for your choose Wireless AP SSID*/
#endif
                    }
                    else if (wifi_device_type == WIFI_USB_DEVICE_RTL8188C)
                    {
#if Make_USB == REALTEK_RTL8188CU
                        MP_RTL8188C_APScanResults(wifi_dev);
#endif
                    }
                    else if (wifi_device_type == WIFI_USB_DEVICE_RTL8188E)
                    {
#if Make_USB == REALTEK_RTL8188E
                        MP_RTL8188E_APScanResults(wifi_dev);
#endif
                    }
#if MAKE_XPG_PLAYER
					ShowWirelessAPTable();
#endif
                    TaskYield();

                }
			}
			if( dwNWEvent & EVENT_NW_CONNECT_AP)
			{
                wlan_AP_Connect(wifi_dev);
			}
#if 0
			if( dwNWEvent & EVENT_NW_SCAN_DONE)
			{
				UartOutText("Scan done");
                TaskYield();
                wlan_AP_ConnectWEP128(dev, "Asus Bill", 9);
//                wlan_AP_ConnectWEP(dev, "Asus Bill", 9);
//                wlan_AP_Connect(dev, "ZyXEL", 5);
//				wlan_AP_Connect(dev, "PCI 54MF-AA",11);
			}
#endif
			if(dwNWEvent & EVENT_LINKSTATE)
			{
#if (DM9KS_ETHERNET_ENABLE || DM9621_ETHERNET_ENABLE)
				if (wifi_device_type == ETHERNET_USB_DEVICE_DM9621)
				{
					dmfe_dev->flags = dev_get_flags(dmfe_dev);
#if DM9KS_ETHERNET_ENABLE
					if(GetNetConfigDhcpFlag()==TRUE)
						NetInterfaceEventSet();
					else
#endif
					{
						if (dmfe_dev->flags & IFF_RUNNING)
						{
							mpDebugPrint("notify DHCP module %x",(dmfe_dev->flags & IFF_RUNNING) );
							mpx_DhcpLinkEventSend(TRUE, dmfe_dev); 
						}
						else
							mpx_DhcpLinkEventSend(FALSE, dmfe_dev); /* notify DHCP module */
					}					
				}
				else
#endif
				{
	                wifi_dev->flags = dev_get_flags(wifi_dev);
	                if ((wifi_dev->flags & IFF_RUNNING) ^ (dev_flags & IFF_RUNNING))
	                {
	                if (wifi_dev->flags & IFF_RUNNING)
	                    {
	                        //if (!NetConfiged())
	                        {
	                            if (mpx_DhcpEnableGet(wifi_dev))
	                            {
	                                MP_DEBUG("DHCP link up event");
#if !SHOW_FRAME_TEST
	                                mpx_DhcpLinkEventSend(TRUE, wifi_dev); /* notify DHCP module */
#endif
	                            }
	                            /* else it's static IP */
	                        }
	                    }
	                    else
	                    {
	                        //if (NetConfiged())
	                        {
	                            if (mpx_DhcpEnableGet(wifi_dev))
	                            {
	                                MP_DEBUG("DHCP link down event");
	                                mpx_DhcpLinkEventSend(FALSE, wifi_dev); /* notify DHCP module */
	                            }
	                            /* else it's static IP */
	                    	}
	            		}
	                dev_flags = wifi_dev->flags;
					}
				}
			}

			// Get IP by DHCP
//			if(dwNWEvent & EVENT_NW_GETIP)
			if(dwNWEvent & EVENT_NW_INTERFACE)
			{
				NetDisConfiged();	//cj added
				memset(&getNetWorkInfo,0x00,sizeof(NetWorkInfo));
				MP_DEBUG("DHCP/Static Config....");

				MP_DEBUG6("MAC = %02x:%02x:%02x:%02x:%02x:%02x",
							myethaddr[0],
							myethaddr[1],
							myethaddr[2],
							myethaddr[3],
							myethaddr[4],
							myethaddr[5]);
				GetDebugPrintMac(getNetWorkInfo.MAC,myethaddr);
				//GetDebugPrintIP(getNetWorkInfo.MAC,myethaddr);
				//if(NO_ERR != mpx_NetworkStart(NIC_INDEX_WIFI))
                //		{
                //    			DPrintf("[NET TEST] net init fail");
                //		}
                		//dhcp start
                DPrintf("Getting IP address...\\-");
                		//mpx_DhcpStatusCallbackSet(dhcpCallback);
#if DM9KS_ETHERNET_ENABLE
               if(GetNetConfigDhcpFlag()==TRUE)
               {
	               NetDefaultIpSet(NIC_INDEX_ETHER, GetNetConfigIPaddr());
				   DPrintf("IP: \\-");
				   NetDebugPrintIP(GetNetConfigIPaddr());
			   NetDNSSet(NIC_INDEX_ETHER, 0, 0);
			   NetDNSSet(NIC_INDEX_ETHER, 0, 1);
				   DPrintf("Subnet mask: \\-");
				   NetDebugPrintIP(GetNetConfigNetmask());
				   NetSubnetMaskSet(NIC_INDEX_ETHER, GetNetConfigNetmask());
				   DPrintf("Default gateway: \\-");
				   NetDebugPrintIP(GetNetConfigGateway());
				   NetGatewayIpSet(NIC_INDEX_ETHER, GetNetConfigGateway());
					mpDebugPrint("Set NET_CONFIGED");
	                App_State.dwState |= NET_CONFIGED;
					dma_invalid_dcache();
	                EventSet(NETWARE_EVENT, EVENT_NET_CONFIGURED);
					ShowWirelessConnect();		//cj added

               }
			   else
#endif               
                if(INADDR_ANY != mpx_DhcpIpAddrGet())
                {
					U32 ipLeaseTime;
					DPrintf("OK");

					DPrintf("IP: \\-");
					NetDebugPrintIP(mpx_DhcpIpAddrGet());
					GetDebugPrintIP(getNetWorkInfo.IP,mpx_DhcpIpAddrGet());

					DPrintf("Subnet mask: \\-");
					NetDebugPrintIP(mpx_DhcpSubnetMaskGet());
					GetDebugPrintIP(getNetWorkInfo.SubMask,mpx_DhcpSubnetMaskGet());

					DPrintf("Default gateway: \\-");
					NetDebugPrintIP(mpx_DhcpGatewayAddrGet());
					GetDebugPrintIP(getNetWorkInfo.DefaultGetway,mpx_DhcpGatewayAddrGet());

					ipLeaseTime = mpx_DhcpLeaseTimeGet();
					DPrintf("Lease time: %2d:%2d:%2d", ipLeaseTime/3600, (ipLeaseTime%3600)/60, (ipLeaseTime%3600)%60);
					for(i = 0; i < mpx_DhcpDnsAddrNumGet(); i++)
					{
						DPrintf("Dns server %d: \\-", i);
						NetDebugPrintIP(mpx_DhcpDnsAddrGet(i));
						GetDebugPrintIP(getNetWorkInfo.DNS[i],mpx_DhcpDnsAddrGet(i));
					}
#ifdef HAVE_SMB
                    set_mysocketaddress(getNetWorkInfo.IP);
#endif
					mpDebugPrint("Set NET_CONFIGED");
                    App_State.dwState |= NET_CONFIGED;
					dma_invalid_dcache();
                    EventSet(NETWARE_EVENT, EVENT_NET_CONFIGURED);
#if HAVE_FTP_SERVER
                    mpx_FtpServerEnable();
#endif
					ShowWirelessConnect();		//cj added

                }
#if Make_ADHOC
				else if(WirelessNetworkSetupITEM.Mode == WIFI_MODE_IBSS)
				{

				   U32 ipLeaseTime;
				   U32 static_ip = 0,static_mask_ip = 0;
				   BYTE *ipstr = NULL;
				   extern BYTE MPX_ADHOC_SSID[];

				   //static_ip = StringToIp(WirelessNetworkSetupITEM.ManualSetup.IP);
				   ipstr = Adhoc_Set_IPAddress(myethaddr[5]);
				   static_ip = StringToIp(ipstr);
				   static_mask_ip = StringToIp(WirelessNetworkSetupITEM.ManualSetup.SubMask);
				   NetDefaultNicSet(NIC_INDEX_WIFI);
				   NetDefaultIpSet(NIC_INDEX_WIFI,static_ip);
				   NetSubnetMaskSet(NIC_INDEX_WIFI,static_mask_ip);
				   DPrintf("IP: \\-");
				   NetDebugPrintIP(mpx_DhcpIpAddrGet());
			 	   GetDebugPrintIP(getNetWorkInfo.IP,mpx_DhcpIpAddrGet());

			       DPrintf("Subnet mask: \\-");
				   NetDebugPrintIP(mpx_DhcpSubnetMaskGet());
			       GetDebugPrintIP(getNetWorkInfo.SubMask,mpx_DhcpSubnetMaskGet());

			       DPrintf("Default gateway: \\-");
				   NetDebugPrintIP(mpx_DhcpGatewayAddrGet());
				   GetDebugPrintIP(getNetWorkInfo.DefaultGetway,mpx_DhcpGatewayAddrGet());

			       ipLeaseTime = mpx_DhcpLeaseTimeGet();
				   DPrintf("Lease time: %2d:%2d:%2d", ipLeaseTime/3600, (ipLeaseTime%3600)/60, (ipLeaseTime%3600)%60);
				   for(i = 0; i < mpx_DhcpDnsAddrNumGet(); i++)
				   {
					   DPrintf("Dns server %d: \\-", i);
					   NetDebugPrintIP(mpx_DhcpDnsAddrGet(i));
					   GetDebugPrintIP(getNetWorkInfo.DNS[i],mpx_DhcpDnsAddrGet(i));
				   }
#ifdef HAVE_SMB
				   set_mysocketaddress(getNetWorkInfo.IP);
#endif
				   App_State.dwState |= NET_CONFIGED;
				   EventSet(NETWARE_EVENT, EVENT_NET_CONFIGURED);

				}
#endif/*Make_ADHOC*/
                else
                {
                    DPrintf("DHCP failed/No static IP");
                    App_State.dwState &= ~NET_CONFIGED;
                    EventSet(NETWARE_EVENT, EVENT_NET_CONFIGURED);
#if (MAKE_XPG_PLAYER==0)
                   NetWifiConnectToAP();
#endif
					
                }
			}

			/* notify application about IP up/down event */
			if(dwNWEvent & EVENT_NET_CONFIGURED)
			{
#if SHOW_FRAME_TEST
				sentTo_testWiFi_task(4);
#endif
				if (App_Net_Event && App_Net_Event->netEvNetworkUp)
					(*App_Net_Event->netEvNetworkUp)(App_Data);
			}

			// net work file cache
			if(dwNWEvent & EVENT_FILE_CACHE)
			{
			#if MAKE_XPG_PLAYER
				SemaphoreWait(NET_CACHE_SEMA_ID);
				FillNetCacheBuffer();
				SemaphoreRelease(NET_CACHE_SEMA_ID);
			#endif	/*MAKE_XPG_PLAYER*/
			}

			/*
			 * Move network timer processing to NetTask
			 * from wlan_service_main_thread.
			 */
			//if(ChkNWPeriodicTimerEvent())
			if(dwNWEvent & EVENT_NW_TIMER)
			{
				//ClrNWPeriodicTimerEvent();
				NWPeriodicTimerHandle();
			}

			if(dwNWEvent & EVENT_NW_CONFIG_CHANGED)
			{
#ifdef HAVE_HOSTAPD
                short old, new; 
                old = !!mpx_DhcpEnableGet(wifi_dev);
                new = !!net_ipv4_method_dhcp;

                if (new ^ old)
                {
                    mpx_DhcpEnableSet(new, wifi_dev);
                }
                if (!new)
                {
                    struct in_addr addr;
                    inet_aton(net_ipv4_fixed_address, &addr);
                    NetDefaultIpSet(wifi_dev->ifindex, addr.s_addr);
                    inet_aton(netcfg_ipv4_fixed_netmask, &addr);
                    NetSubnetMaskSet(wifi_dev->ifindex, addr.s_addr);
                }
#endif
			}
		}//Event
#if ((MAKE_XPG_PLAYER==0)&&HAVE_HTTP_SERVER)
        TaskSleep(1);
#else
		TaskYield();
#endif

	}//while

	return;
}

#if POP3_TEST
void TEST_NET(){
while(1){
	TaskSleep(30000);

	NetScanRequestEventSet();
	TaskSleep(30000);

	wlan_AP_Config(NULL,"aass");


	TaskSleep(30000);
	{
	MP_DEBUG("pop3_test");
		struct list_head *MailQ;
		MailQ = pop3_fetch_messages("pop.gmail.com", 995, "mpxwifi", "1mpxwifi1", TRUE,0);
	}

}
}
#endif

void NetTaskInit()
{
    int ret;

	MP_DEBUG1("NetTaskInit = %d>>>>>>>>>>>>>>>>>>>>> ",sizeof(App_State));
	memset(&App_State,0x00,sizeof(App_State));
	  App_State.dwState = 0;

	//init timer
	SysTimerProcAdd(1,NetTimerHandle,FALSE);


    mm_init();

#if USB_WIFI
    NetDeviceAttachEvent_Fn = MyNetDeviceAttachEvent;
#endif

#ifdef VPERF_TEST
	vperf_Init();
#endif

#if (DM9KS_ETHERNET_ENABLE || DM9621_ETHERNET_ENABLE)
#if (DM9621_ETHERNET_ENABLE&&(DEMO_PID==0))
	TaskCreate(ETHERNET_RECV_TASK, Ethernet_RecvTask, WIFI_PRIORITY, 0x8000);
#else
	TaskCreate(ETHERNET_RECV_TASK, Ethernet_RecvTask, DRIVER_PRIORITY-1, 0x4000);
#endif
	EventCreate(ETHERNET_RECV_EVENT, (OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);
#if DM9KS_ETHERNET_ENABLE
	TaskCreate(ETHERNET_ARPRECV_TASK, Ethernet_CheckArp, DRIVER_PRIORITY-1, 0x4000);
	EventCreate(ETHERNET_ARPRECV_EVENT, (OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);
	TaskStartup(ETHERNET_ARPRECV_TASK);
#endif
	TaskStartup(ETHERNET_RECV_TASK);
	SemaphoreCreate(CFETHERNET_MCARD_SEMA, OS_ATTR_PRIORITY, 1);
#endif
    //SemaphoreCreate(WIFI_Rx_SEMA_ID, OS_ATTR_PRIORITY, 1);
	EventCreate(NETWARE_EVENT, (OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);
	SemaphoreCreate(NET_SEM_ID, OS_ATTR_PRIORITY, 1);
	TaskCreate(NETWARE_TASK, NetTask, WIFI_PRIORITY, 0x2000);
//#else
//	TaskCreate(NETWARE_TASK, NetTask, WIFI_PRIORITY, 4096);
//#endif
//#if !Make_USB
	TaskStartup(NETWARE_TASK);
//#endif
#if HAVE_NETSTREAM
#if !Make_ADHOC
#if MAKE_XPG_PLAYER
	EventCreate(NETWORK_STREAM_EVENT, (OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);
	EventClear(NETWORK_STREAM_EVENT,0);
	TaskCreate(NETWORK_STREAM_TASK, NetStream, NETWORK_STREAM_PRIORITY, 0x8000);
    if(NetStrean_Buffer_Alloc(NETSTREAM_MAX_BUFSIZE+8192))	   
    TaskStartup(NETWORK_STREAM_TASK);
#endif	
#endif	
#endif //HAVE_NETSTREAM
#if 0
extern void  Adhoc_ServerStart();
	EventCreate(NETWORK_STREAM_EVENT, (OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);
	EventClear(NETWORK_STREAM_EVENT,0);
	TaskCreate(NETWORK_STREAM_TASK, Adhoc_ServerStart, CONTROL_PRIORITY, 0x4000);
	TaskStartup(NETWORK_STREAM_TASK);
#endif

#if NET_UPNP
#if !Make_ADHOC
	EventCreate(UPNP_START_EVENT, (OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);
	EventClear(UPNP_START_EVENT,0);
	TaskCreate (UPNP_MINI_SERVER_TASK,RunMiniServer, CONTROL_PRIORITY, 0x4000);
    TaskStartup(UPNP_MINI_SERVER_TASK);
	TaskYield();
	TaskCreate (UPNP_CTRL_TASK,SSDPReplyTask, CONTROL_PRIORITY, 0x8000);
    TaskStartup(UPNP_CTRL_TASK);
	IXMLSemid = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
	MP_ASSERT(IXMLSemid > 0);
	UPNPHandleLock = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
	MP_ASSERT(UPNPHandleLock > 0);
#if	HAVE_UPNP_MEDIA_SERVER
    ret = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_EVENT_CLEAR, 0);
    if(ret < 0)
    {
        DPrintf("[NETCTRL] event create fail");
        BREAK_POINT();
    }
    UPNP_Server_Event = (U08)ret;
	TaskYield();
	MP_DEBUG("UPNP_Server_Event %d",UPNP_Server_Event);
#endif
	ret = mpx_TaskCreate(RunSSDPServer, CONTROL_PRIORITY, 0x8000);
	MP_DEBUG("RunSSDPServer ret %x",ret);
    if(ret < 0)
    {
        DPrintf("RunSSDPServer Task Create Fail");
        BREAK_POINT();
    }
	mpx_TaskStartup((unsigned char) ret);
	TaskCreate (UPNP_DOWNLOAD_TASK,UpnpdownloadTask, CONTROL_PRIORITY, 0x4000);
    TaskStartup(UPNP_DOWNLOAD_TASK);
#endif
#endif
#ifdef CONFIG_ATH9K_HTC_TX99
	void UARTTask_Main(void);
	ret = mpx_TaskCreate(UARTTask_Main, CONTROL_PRIORITY, 0x2000);
	mpx_TaskStartup((unsigned char) ret);
#endif

#ifdef HAVE_SMB
	SmbCli_Init();
#endif
#ifdef VIPHONE_ENABLED
    sviphone_init();
#endif
    
    /* used for atomic arithmetic operations */
	atomic_sema = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
	MP_ASSERT(atomic_sema > 0);
}


void webclient_closed(void)
{
	//xpgShowDialog("Webclient: connection closed");
    	MP_DEBUG("Webclient: connection closed");
}
void webclient_aborted(void)
{
	//xpgShowDialog("Webclient: connection aborted");
    	MP_DEBUG("Webclient: connection aborted");

		App_State.dwState |= NET_TIMEOUT;
#if (Make_SDIO == MARVELL_WIFI)
	wlan_tx_timeout();
#endif
}


void webclient_timedout(void)
{
	//xpgShowDialog("Webclient: connection timed out");
    MP_ALERT("Webclient: connection timed out\n");

		App_State.dwState |= NET_TIMEOUT;
#if (Make_SDIO == MARVELL_WIFI)
       wlan_tx_timeout();
#endif
}


//=========================================================
//
//
//
//=========================================================
static BYTE Webbuf[256];

BYTE * Net_GetWebSite(BYTE *wesite)
{
	BYTE *ptr1,*ptr2;

	memset(Webbuf,0x00,256);
	ptr1 = wesite;
	ptr2 = Webbuf;
	while((*ptr1 != ':') || (*(ptr1+1) != '/') || (*(ptr1+2) != '/'))
	{
		ptr1++;
	}
	ptr1 +=3;

	while((*ptr1 != '/') && (*ptr1 != ':'))
	{
		*ptr2 = *ptr1;
		ptr1++;
		ptr2++;
	}
	MP_DEBUG1("Net_GetWebSite <<<<   = %s",Webbuf);
	return Webbuf;
}

U16 Net_GetWebSitePort(BYTE *wesite)
{
	BYTE *ptr1,*ptr2;
	BYTE port[16];
	ptr1 = wesite;
	ptr2 = port;
	memset(port,0,16);

	while((*ptr1 != ':') || (*(ptr1+1) != '/') || (*(ptr1+2) != '/'))
	{
		ptr1++;
	}
	ptr1 +=3;
	while((*ptr1 != '/') && (*ptr1 != ':'))
	{
		ptr1++;
	}
	if( *ptr1 == ':' )
	{
		MP_DEBUG1("port %s",ptr1);
		ptr1++;
		while( (*ptr1 != '/') && (*ptr1 != '\0'))
		{
			MP_DEBUG1("port %c",*ptr1);
			*ptr2 = *ptr1;
			ptr1++;
			ptr2++;
		}
		MP_DEBUG1("port %s",ptr2);
		return atoi(port);
	}
	return 80;
}


void webclient_datahandler(char *data, u16_t len)
{
#if MAKE_XPG_PLAYER
    XML_BUFF_link_t  *ptr;
    XML_ImageBUFF_link_t *qtr;
    BYTE *tar;
    BYTE *src;
    int i,num;
    DWORD recvsize;
    BYTE *gifptr;

    App_State.dwTotallen += len;
    //MP_DEBUG("@@webclient_datahandler<=%x ",App_State.dwState);

	if (Net_AppState_isImage(App_State.dwState))
	{
		 //MP_DEBUG2("2@@<=%x %d ",App_State.dwTotallen, g_recvsize);
		if(App_State.dwTotallen > g_recvsize)
		{
			//MP_DEBUG2("3@@<=%d %d",App_State.dwTotallen,App_State.HttpContentLength);
			g_recvsize += 10000;
#if MAKE_XPG_PLAYER
			NetListProcess(App_State.dwTotallen, App_State.HttpContentLength, 0, 1);
#endif
		}
	}
	 //UartOutText(".");
	if((len == 0) && (App_State.dwEndToggle == 0))
	{
		//MP_DEBUG("w1 ");
		App_State.dwEndToggle = 1;
       	return;
	}
	else if((len == 0) && (App_State.dwEndToggle == 1))
	{
		//MP_DEBUG("w2 ");
#if MAKE_XPG_PLAYER
		NetListProcess(App_State.dwTotallen,App_State.dwTotallen,0,1);
#endif
		App_State.dwState &= ~NET_TX_RX_DATA;
		return;

	}
	else if((len != 0) && (App_State.dwEndToggle == 1)) //query for filesize
	{
		//MP_DEBUG("w3 ");
		App_State.dwEndToggle = 0;
		return;
	}

	if (App_State.dwState &  NET_RECVHTTP)
    {
        //MP_DEBUG("webclient_datahandler 1");
        u16_t copylen;
        XML_BUFF_link_t *xp, *prev;

        xp = App_State.ptr;
        prev = NULL;
        while(xp)
        {
            prev = xp;
            xp = xp->link;
        }
        xp = prev;

        while(len > 0)
        {
            if(xp->buff_len < IMAGE_BUF)
            {
                tar = xp->BUFF + xp->buff_len;
                copylen = MIN(len, IMAGE_BUF - xp->buff_len);
                for(i=0; i<copylen; i++)
                {
                    *tar++ = *data++;
                }
                xp->buff_len += copylen;
                len -= copylen;
            }
            else
            {
                ptr = (XML_BUFF_link_t  *)ext_mem_malloc(sizeof(XML_BUFF_link_t));
                if (!ptr)
                {
                    MP_ALERT("webclient_datahandler: out of memory");
                    BREAK_POINT();
                }
                memset(ptr->BUFF, 0x00, IMAGE_BUF);
                ptr->buff_len = 0;
                ptr->link = NULL;

                xp->link = ptr;
                xp = ptr;
            }
        }
    }
	else if (Net_AppState_isImage(App_State.dwState))
	{
        u16_t copylen;
        XML_ImageBUFF_link_t *xp, *prev;

		xp = App_State.qtr;
        prev = NULL;
        while(xp)
        {
            prev = xp;
            xp = xp->link;
        }
        xp = prev;

        while(len > 0)
        {
//        	MP_DEBUG("webclient_datahandler 2 xp->buff_len = %d", xp->buff_len);
            if(xp->buff_len < IMAGE_BUF)
            {
                tar = xp->BUFF + xp->buff_len;
                copylen = MIN(len, IMAGE_BUF - xp->buff_len);
                for(i=0; i<copylen; i++)
                {
                    *tar++ = *data++;
                }
                xp->buff_len += copylen;
                len -= copylen;
            }
            else
            {
            MP_DEBUG("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&");
                qtr = (XML_ImageBUFF_link_t  *)ext_mem_malloc(sizeof(XML_ImageBUFF_link_t));
                if (!qtr)
                {
                    MP_ALERT("webclient_datahandler: out of memory");
                    MP_ASSERT(0);
                }
                memset(qtr, 0x00, sizeof(XML_ImageBUFF_link_t));

                xp->link = qtr;
                xp = qtr;
            }
        }
	}
	else
	{
        //UartOutText("w4 ");
        App_State.dwDataPtr = (DWORD)data;
        App_State.dwDataLength = (DWORD)len;

        tar = (BYTE *)(App_State.dwBuffer + App_State.dwOffset);
        src = (BYTE *)(App_State.dwDataPtr);
        MpMemCopy(tar,src,App_State.dwDataLength);

        App_State.dwOffset += App_State.dwDataLength;
	}
#endif
}

#if HAVE_CURL == 0
char uip_recvbuff[4*1400];
void *uip_appdata;           /* The uip_appdata pointer points to
				    				application data. */
U16 uip_len, uip_slen;
#endif


#if Write_To_NAND
void Save_Jpeg_Files(char* name, int len, XML_BUFF_link_t* buffer){
    BOOL boDriveAdded = FALSE;
	BYTE bMcardId = NAND;
	STREAM *handle = NULL;
	static DRIVE *sDrv;
	static int ret = 1;
	char* file_ext;
	int fswr = 0;
	int need_write = 0;
	DRIVE *drv;

	BYTE CurId = DriveCurIdGet();


	file_ext = name;

	while(*file_ext != 0x2e){
		file_ext++;
	}
	*file_ext = '\0';
	file_ext++;

	strtoupper(name);
	strtoupper(file_ext);

    if (SystemCardPlugInCheck(bMcardId))
	{
		SystemDeviceInit(bMcardId);
#if 1
		if (!SystemCardPresentCheck(bMcardId))
		{
			MP_DEBUG("-E- SystemDeviceInit fail");
			goto Error;
		}
		else
		{
			if (!(boDriveAdded = DriveAdd(bMcardId))){
				MP_DEBUG("-E- DriveAdd fail");
				goto Error;
			}
		}
#endif
		if (boDriveAdded)
		{
			DRIVE *drv;
			drv = DriveChange(bMcardId);
			if (DirReset(drv) != FS_SUCCEED)
				return;

			handle = FileSearch(drv, name, file_ext, E_FILE_TYPE);
			if (handle != NULL)
			{
				MP_DEBUG("not found");
					sDrv = DriveGet(bMcardId);
					ret = CreateFile(sDrv, name, file_ext);
					if (ret)
					{
						MP_DEBUG("create file fail\r\n");
						goto Error;
					}
					handle = FileOpen(sDrv);
					if (!handle)
					{
						MP_DEBUG("open file fail\r\n");
						goto Error;
					}
					else
					{
						fswr = FileWrite(handle,buffer->BUFF,buffer->buff_len);
						while (buffer->link)
						{
							buffer = buffer->link;
							fswr = FileWrite(handle,buffer->BUFF,buffer->buff_len);
						}

						FileClose(handle);
					}

			}
			else
			{
				MP_DEBUG("found");
				sDrv = DriveGet(bMcardId);
				handle = FileOpen(sDrv);
				DeleteFile(handle);
				#if 1
				ret = CreateFile(sDrv, name, file_ext);
				if (ret)
				{
					MP_DEBUG("create file fail\r\n");
					goto Error;
				}
				handle = FileOpen(sDrv);
				if (!handle)
				{
					MP_DEBUG("open file fail\r\n");
					goto Error;
				}
				else
				{
					fswr += FileWrite(handle,buffer->BUFF,buffer->buff_len);
					//need_write += buffer->buff_len;
					while (buffer->link)
					{
						buffer = buffer->link;
						fswr += FileWrite(handle,buffer->BUFF,buffer->buff_len);
						//need_write += buffer->buff_len;
					}
					FileClose(handle);

					//MP_DEBUG("total write %d", fswr);
					//MP_DEBUG("need_write %d", need_write);
				}
				#endif
			}
		}
    }
Error:
	drv = DriveChange(CurId);
	if (DirReset(drv) != FS_SUCCEED)
		return;

}

#endif

STREAM* OpenFile(char* name, int* CurId){
    BOOL boDriveAdded = FALSE;
	BYTE bMcardId = SD_MMC;
	static DRIVE *sDrv;
	STREAM* handle = NULL;
	static int ret = 1;
	char* file_ext;
	int fswr = 0;
	DRIVE *drv;

	*CurId = DriveCurIdGet();


	file_ext = name;

	while (*file_ext != 0x2e)
	{
		file_ext++;
	}
	*file_ext = '\0';
	file_ext++;

	strtoupper(name);
	strtoupper(file_ext);

    if (SystemCardPlugInCheck(bMcardId))
	{
		SystemDeviceInit(bMcardId);

		if (!SystemCardPresentCheck(bMcardId))
		{
			MP_DEBUG("-E- SystemDeviceInit fail");
			return 0;
		}
		else
		{
			if (!(boDriveAdded = DriveAdd(bMcardId))){
				MP_DEBUG("-E- DriveAdd fail");
				return 0;
			}
		}

		if (boDriveAdded)
		{
			drv = DriveChange(bMcardId);
			if (DirReset(drv) != FS_SUCCEED)
				return 0;

			handle = FileSearch(drv, name, file_ext, E_FILE_TYPE);
			if (handle != NULL)
			{
				MP_DEBUG("not fund");
					sDrv = DriveGet(bMcardId);
					ret = CreateFile(sDrv, name, file_ext);
					if (ret)
					{
						MP_DEBUG("create file fail\r\n");
						goto Error;
					}
					handle = FileOpen(sDrv);
					if (!handle)
					{
						MP_DEBUG("open file fail\r\n");
						goto Error;
					}

			}
			else
			{
				MP_DEBUG("fund");
				sDrv = DriveGet(bMcardId);
				handle = FileOpen(sDrv);
				DeleteFile(handle);
				ret = CreateFile(sDrv, name, file_ext);
				if (ret)
				{
					MP_DEBUG("create file fail\r\n");
					goto Error;
				}
				handle = FileOpen(sDrv);
				if (!handle)
				{
					MP_DEBUG("open file fail\r\n");
					goto Error;
				}
			}
		}
    }
	return handle;
Error:
	drv = DriveChange(CurId);
	if (DirReset(drv) != FS_SUCCEED)
		return 0;

	return 0;

}

int CloseFile(STREAM *handle, int CurId){
	DRIVE *drv;

	FileClose(handle);
	drv = DriveChange(CurId);
	if (DirReset(drv) != FS_SUCCEED)
		return NULL;

}

int Get_Image_File(void* URL, U08* Buffer, DWORD timeout);
#if HAVE_CURL
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
 * @param size No longer used
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
int __Net_Recv_Http_Data(BYTE *url, BYTE type, DWORD size, DWORD timeout)
{
	XML_BUFF_link_t * xp = App_State.ptr;
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
            App_State.ptr = App_State.XML_BUF = xp;
      	}
	else
	{
		while(xp->link)
			xp = xp->link;
	}

       App_State.flags = 0;
       total_data_len = Get_Image_File(url, xp->BUFF, timeout);
        MP_DEBUG("111111 Net_Recv_Data URL=%s len=%d", url, total_data_len);

       if (total_data_len == 0)
       {
       	//AAADDDD
		//xpgCb_NetPhoto_Reconnet();

		return -1;
       }
       else
            return total_data_len;
}

XML_ImageBUFF_link_t * get_xml_imagebuffer()
{
	XML_ImageBUFF_link_t * xp = App_State.qtr;

	if (!xp)
	{
		xp = ext_mem_malloc(sizeof(XML_ImageBUFF_link_t));
		if (! xp)
		{
			mpDebugPrint("%s-->malloc fail", __func__);
			return NULL;
		}
             memset(xp, 0, sizeof(XML_ImageBUFF_link_t));
             App_State.qtr = App_State.XML_BUF1 = xp;
	}
	else
	{
		while(xp->link)
			xp = xp->link;
	}

	return xp;
}


int __Net_Recv_Image_Data(BYTE *url, BYTE type, DWORD size, DWORD timeout)
{
	XML_ImageBUFF_link_t *xp = get_xml_imagebuffer();
	int total_data_len;
	char filename[8];
	BYTE  curDrvId;
	STREAM *fptr;
	DRIVE *sDrv;
	//char *writebuf;
	static unsigned int ki =0;
	XML_BUFF_link_t *tmplink;

	App_State.flags = 0;
	total_data_len = Get_Image_File(url, xp->BUFF, timeout);
	MP_DEBUG("333333 Net_Recv_Data URL=%s  ;file len=%d", url, total_data_len);

	if (total_data_len == 0)
	{
		return -1;
	}
	else
	{
#if 0	
		//Kevin Add Test for only
		if( ki < 30 )
		{
			sprintf(filename,"GGzz%02d",ki++);
			curDrvId = DriveCurIdGet();
			DriveChange(CF);
			sDrv = DriveGet(DriveCurIdGet());
			if (CreateFile(DriveChange(CF), filename, "jpg") != PASS)
			{
				mpDebugPrint("Failed to create file!");
				DriveChange(curDrvId);
				return total_data_len;
			}
			else if ((fptr = FileOpen(sDrv)) == NULL)
			{
				mpDebugPrint("File handle is unavailable!");
				return total_data_len;
			}
			tmplink = xp;
			//writebuf = xp->BUFF;
			while( tmplink )
			{
				FileWrite(fptr, tmplink->BUFF, tmplink->buff_len);
				tmplink = tmplink->link;
			}
/*
			if( total_data_len < 128 * 1024 )
			{
				FileWrite(fptr, (BYTE *)xp->BUFF, total_data_len);
			}
			else
				FileWrite(fptr, (BYTE *)xp->BUFF, 128 * 1024);
*/
			if( fptr )
				FileClose(fptr);
			DriveChange(curDrvId);
		}
#endif
		//End kevin
		return total_data_len;
	}
}


int __Net_Recv_WebServer_Data(BYTE * url, BYTE type, DWORD size, DWORD timeout)
{
	int total_data_len = -1;
#if	Make_WEBSERVER
	total_data_len = __Net_Recv_Image_Data(url, type, size, timeout);
	if (total_data_len == 0)
		return -1;

	net_web_sDrv = DriveGet(SD_MMC);//SD_MMC//NAND

	if (net_web_shandle != NULL)
		FileClose(net_web_shandle);

	net_web_sDrv = DriveGet(SD_MMC); //SD_MMC //NAND
	if (FS_SUCCEED != FileSearch(net_web_sDrv, "nowplay", "tmp", E_FILE_TYPE))
	{
		SemaphoreWait(FILE_READ_SEMA_ID);
	      	ret = CreateFile(net_web_sDrv, "nowplay", "tmp");
		SemaphoreRelease(FILE_READ_SEMA_ID);
		if (ret)
		{
			UartOutText("create file fail\r\n");
#if DM9KS_ETHERNET_ENABLE
			DriveChange(CF_ETHERNET_DEVICE);
#else
			DriveChange(USB_WIFI_DEVICE);
#endif
			return total_data_len;
		}
	}

	if (FS_SUCCEED == FileSearch(net_web_sDrv, "nowplay", "tmp", E_FILE_TYPE))
	{
		net_web_shandle = FileOpen(net_web_sDrv);

		Seek(net_web_shandle, 0);

		if (!net_web_shandle)
		{
			UartOutText("open file fail\r\n");
#if DM9KS_ETHERNET_ENABLE
			DriveChange(CF_ETHERNET_DEVICE);
#else
			DriveChange(USB_WIFI_DEVICE);
#endif
			return total_data_len;
		}

		buffer = App_State.XML_BUF1;

		while(buffer)
		{
			ret=FileWrite(net_web_shandle, buffer->BUFF, buffer->buff_len);
			//mpDebugPrint("##############FileWrite============> %d",ret);
			buffer = buffer->link;
		}

		if(!ret)
		{
			UartOutText("write file fail\r\n");
			FileClose(net_web_shandle);
			DriveChange(USB_WIFI_DEVICE);
			return total_data_len;
		}

		ret = FileClose(net_web_shandle);

		mpDebugPrint("2#############FileClose===========%x",net_web_shandle);
		if(!ret)
		{
			net_web_shandle = NULL;
		}
		else
			mpDebugPrint("2#############FileClose===========FAIL");
	}

#if DM9KS_ETHERNET_ENABLE
	DriveChange(CF_ETHERNET_DEVICE);
#else
	DriveChange(USB_WIFI_DEVICE);
#endif

#endif

	 return total_data_len;
}

#ifdef NET_NAPI
int do_Net_Recv_Data(BYTE *url, BYTE type, DWORD size, DWORD timeout)
#else
int Net_Recv_Data(BYTE *url, BYTE type, DWORD size, DWORD timeout)
#endif
{
	mpDebugPrint("[%s]: bXpgStatus = %d, App_State = %x", __func__,g_bXpgStatus,App_State.dwState);

	if (App_State.dwState & NET_RECVHTTP)
    	{
		return __Net_Recv_Http_Data(url, type, size, timeout);
	}
	else if (App_State.dwState &  NET_RECVBINARYDATA)
	{
		return __Net_Recv_Image_Data(url, type, size, timeout);
	}

#if Make_WLS
	if (App_State.dwState &  NET_WLS)
	{
		int total_data_len = __Net_Recv_Image_Data(url, type, size, timeout);
		url1 = windows_live_get_photo_url(xp->BUFF,total_data_len);
		Xml_BUFF_free(NET_WLS);
		Xml_BUFF_init(NET_WLS);
		App_State.flags = 0;
		total_data_len = Get_Image_File(url1, xp->BUFF,timeout);
		MP_DEBUG("333333 Net_Recv_Data URL=%s  ;file len=%d", url1, total_data_len);
		if (total_data_len == 0)
			return -1;
		else
			return total_data_len;
	}
#endif

}

#else
#if 0
int Net_Recv_Data(BYTE *url, BYTE type, DWORD size, BYTE *req)
{
	WORD port;
	BYTE * ipaddr;
	BYTE HostStr[256];
	BYTE *website;
	DWORD  index;
	DWORD start;
	int sockid;
	int ret;

	//Kevin Modify 1021
	int selectStatus = NO_ERR;
	ST_SOCK_SET stReadSet;
	ST_SOCK_SET *wfds, *rfds;
	unsigned long val = 0;
	struct timeval tv;
	//End Kevin


    //EnableNetWareTask();
	memset(HostStr, 0x00, 256);
	App_State.type = type;

	if(type == NETFS_MPX_LIST)			// MagicPixel proprietary File Sharing
	{
		port = MPX_DATA_PORT;
		//MP_DEBUG2("PC  <=== %2x %2x ", *url,*(url+1));
		ConvertIP2Str(url, (BYTE *)(&HostStr[0]));
		url = ServerBrowser.ServerList[ServerBrowser.dwCurrentServer].UrlName;
	}
	else if(type == NETFS_PC)
	{
		port = MPX_DATA_PORT;
		ConvertIP2Str((BYTE *)(&ServerBrowser.ServerList[ServerBrowser.dwCurrentServer].ipaddr[0]), (BYTE *)(&HostStr[0]));
	}
    else if (
              (type == NETFS_RSS)				// RSS / NEWS
              || (type == NETFS_FLICKR)			// Flickr
              || (type == NETFS_PICASA)			//Picasa
              || (type == NETFS_GCE)            //GCE
#if YOUGOTPHOTO
              || (type == NETFS_YOUGOTPHOTO)	//YouGotPhoto.com
#endif
#if HAVE_FRAMECHANNEL
			  || (type == NETFS_FRAMECHANNEL)	  //YouGotPhoto.com
#endif
              || (type == NETFS_SHOUTCAST)	//SHOUTCAST
#if HAVE_FRAMEIT
               || (type == NETFS_FRAMEIT)
#endif
#if HAVE_SNAPFISH
               || (type == NETFS_SNAPFISH)
#endif
#if NET_UPNP
              || (type == NETFS_UPNP_LIST)
#endif
            )
	{
#if NET_UPNP
		if (type == NETFS_UPNP_LIST)
			port = Net_GetWebSitePort(url);
		else
#endif
		port = WEB_SERVER_PORT;
		website =(BYTE *) Net_GetWebSite(url);
		MP_DEBUG3("%s  URL(%d) >%s< ",website,strlen(url),url);
        strcpy(HostStr, website);
	}
    else
    {
        MP_DEBUG("Net_Recv_Data: type is not supported");
        return 0;
    }

	App_State.dwTotallen = 0;
	g_recvsize = 0;

	App_State.dwState |= NET_TX_RX_DATA;
	sockid = webclient_get(HostStr, port, url, 0, size);
    if (sockid <= 0)
        return 0;

    init_connection(req);

    webclient_connected();

    webclient_senddata(sockid, req);

	BYTE *buf;
    size = sizeof(uip_recvbuff);
    buf = uip_recvbuff;


    do
    {
        ret = recv(sockid, buf, size, 0);
        if(ret > 0)
        {
            uip_len = ret;
            uip_appdata = buf;
            webclient_newdata();
        }
        else if(ret == 0)
        {
      		webclient_datahandler(NULL, 0);
            DPrintf("[WEB] TCP connection closed gracefully");
            break;
        }
        else /* if(ret < 0) */
        {
      		webclient_datahandler(NULL, 0);
            DPrintf("[WEBC] client aborted (errno=%d)", getErrno());
            break;
        }

		if (App_State.dwState & NET_TIMEOUT)
        {
            DPrintf("[WEBC] client times out");
            break;
        }
    } while(1);

    if (sockid)
    {
        MP_DEBUG1("WEBC: closesocket(%d)", sockid);
        ret = closesocket(sockid);
        MP_DEBUG2("WEBC: closesocket(%d) returns %d", sockid,ret);
    }


	if((type == NETFS_PICASA) ||
	    (type == NETFS_GCE) ||
	    (type == NETFS_FLICKR)
#if YOUGOTPHOTO
	    ||(type == NETFS_YOUGOTPHOTO)
#endif
#if HAVE_FRAMECHANNEL
	    ||(type == NETFS_FRAMECHANNEL)
#endif
#if HAVE_FRAMEIT
	    ||(type == NETFS_FRAMEIT)
#endif
#if HAVE_SNAPFISH
	    ||(type == NETFS_SNAPFISH)
#endif
#if NET_UPNP
	    ||(type == NETFS_UPNP_LIST)
#endif
	)
	{
		MP_DEBUG1("Net_Recv_Data returns %d", App_State.HttpContentLength);
		return App_State.HttpContentLength;
	}
	else if(type == NETFS_RSS)
	{
		App_State.HttpFileSize = App_State.dwTotallen;
		return App_State.HttpFileSize;
	}
	//MP_DEBUG1("Net_Recv_Data = %d",App_State.HttpFileSize);
	return App_State.HttpFileSize;
}

#endif
#endif
/*----------------------- Network API ---------------------------------------*/

/*
 * Return the network interface status
 * RETURN:
 *      non-zero: if network interface is up and ready for TX/RX of packets.
 *      zero: if network interface is down.
 */
DWORD NetConfiged()
{
  return  (App_State.dwState & NET_CONFIGED) ? TRUE : FALSE;
}

void NetDisConfiged()
{
  	App_State.dwState &= ~NET_CONFIGED;
}


/*
 * Return the connection status to a WLAN network (or an AP).
 */
DWORD NetConnected(void)
{
  struct net_device *dev = &NicArray[NetDefaultNicGet()]; /* return default interface's status */
  return (dev->flags & IFF_RUNNING) ? TRUE : FALSE;
}

/*
 * Return true if USB WIFI dongle is plugged in and driver is up and running.
 * Return false if USB WIFI dongle is removed or driver is not initialized yet.
 *
 * NOTE: This routine doesn't work with SDIO WIFI
 */
int NetDeviceInitialized(void)
{
  struct net_device *dev = &NicArray[NIC_INDEX_WIFI];
  return (dev->flags & IFF_UP) ? TRUE : FALSE;
}

/*
 * Return true if USB WIFI dongle is plugged in.
 * Return false if USB WIFI dongle is removed.
 *
 * NOTE: This routine doesn't work with SDIO WIFI
 */
int NetDevicePresent(void)
{
  struct net_device *dev = &NicArray[NIC_INDEX_WIFI];
  if (netif_device_present(dev))
      return TRUE;
  else
      return FALSE;
}

/*
 * Cancel current HTTP transfer
 *
 * This function doesn't terminate the transfer immediately.  It just sets a flag
 * in App_State.flags. CURL can check this flag to terminate transfer in the
 * CURLOPT_PROGRESSFUNCTION callback function. prog_cb() of net_curl_simple.c
 * is an example.
 */
void NetHttpTransferCancel(void)
{
  App_State.flags |= HTTP_TRANSFER_CANCEL;
}

#if 0
/*---------------------------------------------------------------------------*/

DWORD NetInitialed()
{
  return  (App_State.dwState & NET_INITED);
}
#endif


/*---------------------------------------------------------------------------*/
void DisableNetWareTask()
{
    MP_DEBUG("Network disabled !!\n");
	EventSet(NETWARE_EVENT, EVENT_NW_DISABLE);

}

void EnableNetWareTask()
{
    MP_DEBUG("Network enabled !!\n");
	EventSet(NETWARE_EVENT, EVENT_NW_ENABLE);
}

void NetInterfaceEventSet()
{
    MP_DEBUG("Interface Event !!\n");
	EventSet(NETWARE_EVENT, EVENT_NW_INTERFACE);
}

void NetLinkStateEventSet(struct net_device *dev)
{
    MP_DEBUG("Link State Event %d!!\n", dev->ifindex);
	if(dev->ifindex == NIC_INDEX_WIFI || dev->ifindex == NIC_INDEX_ETHER)
		EventSet(NETWARE_EVENT, EVENT_LINKSTATE);
	//Kevin Add
	//EnableNetWareTask();
}

#if USB_WIFI
void NetDriverUpEventSet(int nic)
{
    MP_DEBUG("Network driver up %d!!\n", nic);
    NetDefaultNicSet(nic);
	EventSet(NETWARE_EVENT, EVENT_NW_DRIVER_UP);

}
/**
 * @ingroup WlanScan
 * @brief   Send a notification to NetTask after scan is done.
 *
 * This is usually registered as a callback function to be called
 * in wlan driver after scan is done.  See NetScanDoneEventSet_Fn.
 *
 * @note
 */
void NetScanDoneEventSet(void)
{
	EventSet(NETWARE_EVENT, EVENT_NW_SCAN_DONE);

}

/*
 * MyNetDeviceAttachEvent
 *
 * My callback function for hot plug/unplug.
 *
 * NOTE: For the case of plug-in(attached=TRUE), this callback is called
 * before device driver is initialized.
 */
void MyNetDeviceAttachEvent(WORD idVendor, WORD idProduct, int attached)
{
    if (attached)                   /* USB dongle is attached */
    {
        MP_ALERT("A new USB network device is attached:");
        MP_ALERT("VendorID= 0x%04x, ProductID= 0x%04x", idVendor, idProduct);
    }
    else                            /* else USB dongle is removed */
    {
        MP_ALERT("USB network device is removed:");
        MP_ALERT("VendorID= 0x%04x, ProductID= 0x%04x", idVendor, idProduct);

        /* ----------  notify wpa_supplicant  ---------- */
		#if Make_USB
		UsbOtgWifiPlugout();
        wlan_AP_Disconnect(NULL);
		#endif
    }
}

/**
 * @ingroup WlanScan
 * @brief   Send a scan request to NetTask.
 *
 * After NetTask receives the request, it invokes SIOCSIWSCAN ioctl to
 * instruct the wireless driver to start a scan.
 *
 * The results of the scan is written to the global variable <b>WiFiApBrowser</b>
 * of datatype WIFI_AP_BROWSER.
 *
 * @note Wpa_supplicant is not involved in this operation.
 */
void NetScanRequestEventSet(void)
{
	EventSet(NETWARE_EVENT, EVENT_NW_SCAN_REQ);
}

/*
 * NetWifiConnectToAP
 *
 * Send a connect request to NetTask.
 */
void NetWifiConnectToAP()
{
	MP_DEBUG("Wifi Connect to AP !!\n");
	EventSet(NETWARE_EVENT, EVENT_NW_CONNECT_AP);
}

/*
 * NetWifiDisonnectToAP
 *
 * Send a disconnect request to NetTask.
 */
void NetWifiDisonnectToAP()
{
	MP_DEBUG("Wifi Disonnect AP !!\n");
	EventSet(NETWARE_EVENT, EVENT_NW_DISCONNECT_AP);
}

/*
 * NetConfigChanged
 *
 * Send a net-config-changed event to NetTask.
 */
void NetConfigChanged(const u8 *ipv4_method, const u8 *ipv4_address, const u8 *ipv4_netmask)
{
    BOOLEAN changed = FALSE;
    size_t len;

    if (strcmp(ipv4_method, "dhcp") == 0 && !net_ipv4_method_dhcp)
    {
        net_ipv4_method_dhcp = TRUE;
        changed = TRUE;
    }
    else if (strcmp(ipv4_method, "manual") == 0 && net_ipv4_method_dhcp)
    {
        net_ipv4_method_dhcp = FALSE;
        strncpy(netcfg_ipv4_fixed_netmask, ipv4_netmask, sizeof(netcfg_ipv4_fixed_netmask));
        netcfg_ipv4_fixed_netmask[sizeof(netcfg_ipv4_fixed_netmask)-1] = '\0';
        changed = TRUE;
    }

    len = strlen(ipv4_address);
    if (strcmp(ipv4_method, "manual") == 0 && 
            (len != strlen(net_ipv4_fixed_address) || (strncmp(ipv4_address, net_ipv4_fixed_address, len) != 0)))
    {
        strncpy(net_ipv4_fixed_address, ipv4_address, sizeof(net_ipv4_fixed_address));
        net_ipv4_fixed_address[sizeof(net_ipv4_fixed_address)-1] = '\0';
        changed = TRUE;
    }
    if (changed)
        EventSet(NETWARE_EVENT, EVENT_NW_CONFIG_CHANGED);
}

void NetWifiDongleUnplug()
{
#if USB_WIFI
	driver_initialized = 0; //cj added ,if Uplug
#endif
	MP_DEBUG("Wifi DongleUplug !!\n");
	EventSet(NETWARE_EVENT, EVENT_NW_DISCONNECT_AP);
	if(!netif_carrier_ok(NetNicDeviceGet(NIC_INDEX_PPP)))
		DisableNetWareTask();
	wifi_device_type = WIFI_USB_DEVICE_NONE;
#if MAKE_XPG_PLAYER
	NetRecoverCurDrvID();
#endif
}

#endif

/*
 * Set callback functions for network events
 */
void NetEventHandlerSet(void *ctx, Net_Event *event_handler)
{
    App_Net_Event = event_handler;
    App_Data = ctx;
}

/*---------------------------------------------------------------------------*/

SDWORD NetConfigIPAddress()
{
	DWORD dwNWEvent;
    int cnt;

    MP_DEBUG("NetConfigIPAddress");
	//EnableNetWareTask();
#if 0
	EventSet(NETWARE_EVENT, EVENT_NW_INIT);
#endif

    cnt = 50 * 1000 / 50;                       /* 50 secs */
    do {
        if(NetConfiged())
                break;
        TaskSleep(50);
    } while (cnt-- > 0);

    if(NetConfiged())
    {
        MP_DEBUG("Network NetConfiged !!\n");
        return PASS;
    }
    else
    {
        MP_ALERT("Network config timeout !!\n");
        return FAIL;
    }

}


void netif_carrier_on(struct net_device *dev)
{
    App_State.dwState |= NET_LINKUP;
    if (!(dev->flags & IFF_LOWER_UP))
    {
        dev->flags |= IFF_LOWER_UP;
        rfc2863_policy(dev);
        NetLinkStateEventSet(dev);
    }

    MP_DEBUG("netif_carrier_on");
}
void netif_carrier_off(struct net_device *dev)
{
    App_State.dwState &= ~NET_LINKUP;
    if (dev->flags & IFF_LOWER_UP)
    {
        dev->flags &= ~IFF_LOWER_UP;
        rfc2863_policy(dev);
        NetLinkStateEventSet(dev);
    }
    MP_DEBUG("netif_carrier_off");
}

int netif_carrier_ok(const struct net_device *dev)
{
//    if (App_State.dwState & NET_LINKUP)
    if (dev->flags & IFF_LOWER_UP)
    {
        //MP_DEBUG("netif_carrier_ok TRUE");
        return TRUE;
    }
    else
    {
        //MP_DEBUG("netif_carrier_ok FALSE");
        return FALSE;
    }

}


/**
 * This is an implementation of Linux's alloc_etherdev.
 *
 * alloc_etherdev - Allocates and sets up an Ethernet device
 * @sizeof_priv: Size of additional driver-private structure to be allocated
 *	for this Ethernet device
 *
 * Fill in the fields of the device structure with Ethernet-generic
 * values. Basically does everything except registering the device.
 *
 * Constructs a new net device, complete with a private data area of
 * size (sizeof_priv).  A 32-byte (not bit) alignment is enforced for
 * this private data area.
 */
struct net_device *alloc_etherdev(int sizeof_priv)
{
    strcpy(NicArray[NIC_INDEX_WIFI].name, "wlan0");
#if Make_USB
	if((wifi_device_type == WIFI_USB_DEVICE_AR2524) ||
		(wifi_device_type == WIFI_USB_DEVICE_RTL8188C) || 
		(wifi_device_type == WIFI_USB_DEVICE_RTL8188EUS))
    NicArray[NIC_INDEX_WIFI].priv = mpx_Malloc(sizeof_priv);
#endif
	return &NicArray[NIC_INDEX_WIFI];
}

#if Make_SDIO

/**
 *	free_netdev - free network device
 *	@dev: device
 *
 *	This function does the last stage of destroying an allocated device
 * 	interface. The reference to the device object is released.
 *	If this is the last reference then it will be freed.
 */
void free_netdev(struct net_device *dev)
{
//	mem_free(dev);
}

/**
 * ether_setup - setup Ethernet network device
 * @dev: network device
 * Fill in the fields of the device structure with Ethernet-generic values.
 */
void ether_setup(struct net_device *dev)
{
	dev->set_mac_address 	= NULL;

	dev->flags		= IFF_BROADCAST|IFF_MULTICAST;

	memset(dev->broadcast, 0xFF, ETH_ALEN);
}

void NetScanDoneEventSet(void)
{
    /* stub function.  not used by SDIO WIFI */
}

void MyNetDeviceAttachEvent(int attached)
{
    /* stub function.  not used by SDIO WIFI */
}
#endif
/**
 *	alloc_netdev - allocate network device
 *	@sizeof_priv:	size of private data to allocate space for
 *	@name:		device name format string
 *	@setup:		callback to initialize device
 *
 *	Allocates a struct net_device with private data area for driver use
 *	and performs basic initialization.  Also allocates subquue structs
 *	for each queue on the device at the end of the netdevice.
 */
struct net_device *alloc_netdev(int sizeof_priv, const char *name,
		void (*setup)(struct net_device *))
{
    struct net_device *dev;
    dev = &NicArray[NIC_INDEX_WIFI];
    if (sizeof_priv)
    {
        dev->priv = mpx_Malloc(sizeof_priv);
        memset(dev->priv, 0, sizeof_priv);
    }

    setup(dev);
	strncpy(dev->name, name, IFNAMSIZ);
}

#ifdef NET_NAPI

/*
 * This is a synchronous call so the caller will block until the request
 * processing is finished.
 */
int Net_Recv_Data(BYTE *url, BYTE type, DWORD size, DWORD timeout)
{
    int total_data_len;
    int ret;
    void *data;

	MP_DEBUG("%s %d", __func__, type);

	if (App_State.dwState & NET_RECVTEXTDATA)
    {
        data = App_State.ptr;
    }
    else
        data = App_State.qtr;

	ret = net_http_request(url, data, 0, &total_data_len, timeout);

	if (ret)
        return -1;

	if (App_State.dwState & NET_RECVTEXTDATA)
    {
        if (total_data_len == 0)
       	{
       		//AAADDDD
			//xpgCb_NetPhoto_Reconnet();

			return -1;
       	}
        else
            return total_data_len;
    }
    else
    {
       	if (total_data_len == 0)
       	{
       		//AAADDDD
			//xpgCb_NetPhoto_Reconnet();

			return -1;
       	}
		else
        {

		    return total_data_len;
        }
    }

}

void Net_Recv_Data_todo(ST_NET_WORK *work)
{
	int ret;

	MP_DEBUG("%s", __func__);
	if (!work)
		return;
//    type = (BYTE)work->data2;

    Xml_BUFF_init(NET_RECVBINARYDATA);

    ret = do_Net_Recv_Data((BYTE *)work->data1, 0,0, work->timeout);
    if (ret > 0)
    {
        work->actual_length = ret;
        work->done++;
        work->result = 0;
    }
    else
    {
#if HAVE_CURL
        work->result = Get_Curl_Result();   /* result from CURL transfer */
#endif

    }
	return;
}

void * net_submit_Net_Recv_Data(BYTE *url, BYTE type)
{
    int ret;
    ST_NET_WORK *work;

    work = mpx_Zalloc(sizeof(*work));
    if (!work)
        return NULL;

    http_fill_request(work, url, Net_Recv_Data_todo, NULL, 0);
	ret = http_submit_request(work);
    if (ret == 0)
    {
        mpx_Free(work);
        work = NULL;
    }
	return work;
}

void *net_submit_FetchNetImage(char *url, int type)
{
    return net_submit_Net_Recv_Data(url, type);
}

#endif

#if HAVE_WPA_SUPPLICANT
#define AUTO_CONNECT
#endif

#ifdef AUTO_CONNECT
#if 1
int wlan_AP_Config(struct net_device *dev, char *ssid)
{
	int ret = 0;

    mpDebugPrint("wlan_AP_Connect ssid=%s", ssid);
#if (AUTO_CONNECTION && (SHOW_FRAME_TEST != 1))
	wlan_mode = 0;
	wlan_channel = 0;
#if (AUTO_CONNECTION == 1)	/*No Security*/
	mpDebugPrint("Check: Connect to Open AP\n");
	wlan_security = WIFI_NO_SECURITY;
	wep_tx_keyidx = -1;
key_mgmt = 4;
#elif (AUTO_CONNECTION == 2)	/*WEP Security*/
	mpDebugPrint("Check: Connect to WEP AP\n");
	wlan_security = WIFI_WEP;
	wep_tx_keyidx = 0;;/*Need to set WEP key index for your choose Wireless AP configuration*/
key_mgmt = 4;
	strcpy(wep_key[wep_tx_keyidx], "asdfg");/*Need to set WEP key for your choose Wireless AP configuration*/
	wep_key_len[wep_tx_keyidx] = strlen(wep_key[wep_tx_keyidx]);
#elif (AUTO_CONNECTION == 3)	/*WPA-PSK/WPA2-PSK Security*/
	mpDebugPrint("Check: Connect to WPA-PSK AP\n");
	wlan_security = WIFI_WPA;
	wep_tx_keyidx = -1;
	key_mgmt = -1;
	memset(connect_psk,0,64);
	strcpy(connect_psk, "asdfghjk");/*Need to set WPA key for your choose Wireless AP configuration*/
	int psk_len = strlen(connect_psk);
	connect_psk[psk_len] = '\0';
#endif
#else
#if (SHOW_FRAME_TEST && Make_ADHOC && (Make_USB == 3))
	wlan_security = Adhoc_Security();
	wep_tx_keyidx = -1;
	key_mgmt = 4;
	wlan_mode = 1;
	wlan_channel = Adhoc_Channel();
#else
   	wlan_security = WIFI_WPA;
	wep_tx_keyidx = -1;
    strcpy(connect_psk, "asdfghjk");
#endif    
#endif
	connect_ssidlen = strlen(ssid);
	memcpy(connect_ssid, ssid, connect_ssidlen);
	connect_ssid[connect_ssidlen] = '\0';

    if (!config_wlan_network.alloc)
        wpa_WlanNetworkAdd(&config_wlan_network);
    else
    {
        wpa_WlanNetworkRemove(&config_wlan_network);
        wpa_WlanNetworkAdd(&config_wlan_network);
    }
    if (config_wlan_network.alloc)
    {
        wpa_WlanNetworkSet(&config_wlan_network);
wpa_WlanNetworkConnect(&config_wlan_network);
    }

    return ret;
}
#else
int wlan_AP_Config(struct net_device *dev, char *ssid)
{
	int ret = 0;
#if (Make_WPA == 1)

    mpDebugPrint("wlan_AP_Config: ssid=%s\n", ssid);

    	wlan_security = WIFI_NO_SECURITY;
#if SHOW_FRAME_TEST
	wep_tx_keyidx = -1;
	key_mgmt = 4;
	wlan_mode = 1;
	wlan_channel = Adhoc_Channel();
#else
	wep_tx_keyidx = 0;
#endif
	connect_ssidlen = strlen(ssid);
	memcpy(connect_ssid, ssid, connect_ssidlen);
	connect_ssid[connect_ssidlen] = '\0';

    if (!config_wlan_network.alloc)
        wpa_WlanNetworkAdd(&config_wlan_network);
    else
    {
        wpa_WlanNetworkRemove(&config_wlan_network);
        wpa_WlanNetworkAdd(&config_wlan_network);
    }
    if (config_wlan_network.alloc)
    {
        wpa_WlanNetworkSet(&config_wlan_network);
		wpa_WlanNetworkConnect(&config_wlan_network);
    }
#endif
    return ret;
}
#endif
#endif

#ifdef HAVE_HOSTAPD
extern void hostapd_main(void);
void m_hostapd_initialize()
{
//	MP_DEBUG("%s[%u]: %s:%s() hostapd task=%d", "unknown", TaskGetId(), __FILE__, __func__, WPA_MAIN_TASK_ID);
    DBG("");
	EventCreate(WPA_EVENT, (OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);
    TaskCreate(WPA_MAIN_TASK_ID, hostapd_main, DRIVER_PRIORITY, 0x1000 * 4);
    TaskStartup(WPA_MAIN_TASK_ID);
}

#endif


