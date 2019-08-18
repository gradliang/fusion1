
#define LOCAL_DEBUG_ENABLE 0



/*
// Include section 
*/
#if 0
#include "global612.h"
#else
#include "typedef.h"
#endif
#include "mpTrace.h"
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include "taskid.h"
#include "ieee80211_i.h"

#include "zdmpx.h"

//#include "zydas_common.h"
#include "zd1205.h"
#include "zdapi.h"
#include "zdglobal.h"
#include "zdsorts.h"

//#include "net_socket.h"
#include "zdsm.h"
//#include "zd_chip.h"
#include "ndebug.h"

struct net_device g_net_device2;
struct net_device g_net_device3;

struct net_device   *netdev_global;
int zd1211_OperationMode = 1; //Infrastructure=1, AP=4, IBSS=0, PSEUDO=3
int zd1211_allowChannel = -1; //High 16bit, default CH. Lower 16bit. BG allow CH bitmap

#define MAX_SIGNAL_NUM		128



U8 AuthReqState = STE_AUTH_REQ_IDLE;
U8 AsocState = STE_ASOC_IDLE;
BOOLEAN	mcBuffered = FALSE;

int ar2524_initialized;
int wifi_event_create = 1;

extern int usb_sema;


void usb_init_timer(void);
void usb_timer_task(void);
int mpx_ieee80211_init(void);
void urb_cleanup_init(struct usb_device *udev);
void urb_queue_init(struct usb_device *udev);

int mp_usb_init()
{
    int ret;
    int usb_bulk_wait;
    short i;

    if(!usb_sema)
    {
        ret = (int)SemaphoreCreate(USB_CONTROL_SEMA, OS_ATTR_PRIORITY, 1);
        MP_ASSERT(ret == 0);
        SemaphoreWait(USB_CONTROL_SEMA);

        ret = (int)mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        MP_ASSERT(ret > 0);
        if (ret <= 0)
            return -1;
        usb_sema = ret;
    }
    else
        SemaphorePolling(USB_CONTROL_SEMA);

    if (ar2524_initialized)
    {
        return 0;
    }
#if 0
    urb_sema = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);			
    MP_ASSERT(urb_sema > 0);
#endif

	mpx_ieee80211_init();

    /* ----------  simulate Linux kernel internal timer  ---------- */

    usb_init_timer();

    if (!ar2524_initialized)
    {
        if(wifi_event_create)
    	{
	        wifi_event_create = EventCreate(WIFI_EVENT, (OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE |OS_ATTR_EVENT_CLEAR), 0);
	        MP_ASSERT(wifi_event_create == 0);
    	}
        ret = TaskCreate(HISR_TASK, usb_timer_task,WIFI_PRIORITY,0x1000);
        MP_ASSERT(ret == 0);
    }

    urb_queue_init(NULL);
    urb_cleanup_init(NULL);

	TaskStartup(HISR_TASK);
    return 0;
}

int register_netdev(struct net_device *dev)
{
    netdev_global = dev;
    return 0;
}


extern struct zd1205_private *macp_global;

void zd1205_config_wep_keys(struct zd1205_private *macp);

int zd1205_wpa_ioctl(struct zd1205_private *macp, struct zydas_wlan_param *zdparm);

int mp_set_key(struct ieee80211_hw *hw, enum set_key_cmd cmd,
		       const u8 *local_address, const u8 *address,
		       struct ieee80211_key_conf *key)
{
	struct zd1205_private *macp = macp_global;
	card_Setting_t *pSetting = &macp->cardSetting;
	struct zydas_wlan_param zdparm_buf, *zdparm=&zdparm_buf;

//    MP_FUNCTION_ENTER();

    mpDebugPrint("%s: alg=%d,cmd=%d", __func__,key->alg, cmd);
    if (cmd == SET_KEY)
    {
		key->flags |= IEEE80211_KEY_FLAG_GENERATE_IV;
		if (key->alg == ALG_TKIP)
			key->flags |= IEEE80211_KEY_FLAG_GENERATE_MMIC;
    }
    else
    {
        key->flags &= ~IEEE80211_KEY_FLAG_GENERATE_IV;
        if (key->alg == ALG_TKIP)
            key->flags &= ~IEEE80211_KEY_FLAG_GENERATE_MMIC;
    }
    MP_FUNCTION_EXIT();
    return 0;
}

extern zd_80211Obj_t *pdot11Obj;
void mp_zd1211_sta_notify(struct ieee80211_hw *hw, struct ieee80211_vif *vif, enum sta_notify_cmd cmd, const u8 *addr)
{
    struct ieee80211_sub_if_data *sdata;
	struct ieee80211_if_sta *ifsta;

	sdata = vif_to_sdata(vif);
    ifsta = &sdata->u.sta;

    if (LOCAL_DEBUG_ENABLE) 
    {
        DECLARE_MAC_BUF(mac1);
        MP_ALERT("%s: cmd=%d, addr=%s", __func__,cmd, print_mac(mac1, addr));
    }
    else
        MP_ALERT("%s: cmd=%d", __func__,cmd);
    if (cmd == STA_NOTIFY_AUTH)
    {
        MP_DEBUG("%s: ssid=%s", __func__,wpa_ssid_string(NULL,ifsta->ssid,ifsta->ssid_len));
        mSsid.buf[0] = EID_SSID;
        mSsid.buf[1] = ifsta->ssid_len;
        memcpy((char *)&mSsid.buf[2], ifsta->ssid, ifsta->ssid_len);
        memcpy((char *)&mBssId, ifsta->bssid, ETH_ALEN);
        UpdateStaStatus(addr, STATION_STATE_AUTH_OPEN, 0);
    }
    else
    if (cmd == STA_NOTIFY_ADD)
    {
		DECLARE_MAC_BUF(mac);
        MP_DEBUG("%s: ssid=%s,bssid=%s", __func__,wpa_ssid_string(NULL,ifsta->ssid,ifsta->ssid_len), print_mac(mac, ifsta->bssid));
        mSsid.buf[0] = EID_SSID;
        mSsid.buf[1] = ifsta->ssid_len;
        memcpy((char *)&mSsid.buf[2], ifsta->ssid, ifsta->ssid_len);
        memcpy((char *)&mBssId, ifsta->bssid, ETH_ALEN);
        UpdateStaStatus(addr, STATION_STATE_ASOC, 0);
    }
    else /* else STA_NOTIFY_REMOVE */
    {
//		memset((U8 *)&mBssId, 0, ETH_ALEN);
        if (ifsta->state == IEEE80211_IBSS_JOINED)
        {
            UpdateStaStatus(addr, STATION_STATE_DIS_ASOC, 0);
        }
        else
        {
            mSsid.buf[1] = 0;                       /* reset mSsid */
            UpdateStaStatus(addr, STATION_STATE_DIS_ASOC, 0);
        }
    }

}

int mp_netdevice_present(void)
{
    struct net_device *dev = netdev_global;
    //mpDebugPrint("%s: netdev_global=%p", __func__, dev);
    MP_DEBUG2("%s: netdev_global=%p", __func__, dev);  //cj modify
    if (dev)
    {
        //mpDebugPrint("%s: state=%x", __func__, dev->state);
	MP_DEBUG2("%s: state=%x", __func__, dev->state); //cj modify
    }
    if (!dev || !test_bit(__LINK_STATE_PRESENT, &dev->state))
        return FALSE;
    else
        return TRUE;
}


u32 mp_get_encryption_type(struct zd_chip *chip);
void mp_zd1211_dump(void)
{
    struct net_device *dev = &g_net_device3;
    struct zd1205_private *macp = dev->priv;
	card_Setting_t *pSetting = &macp->cardSetting;
	struct ifreq ifr;
    struct zdap_ioctl z;

    mpDebugPrint("%s: ---------------------------------", __func__);
    mpDebugPrint("pSetting->BssType=%d,macp->bAssoc=%d", pSetting->BssType, macp->bAssoc);
    mpDebugPrint("pSetting->WPASupport=%d,FragThreshold=%d", pSetting->WPASupport, pSetting->FragThreshold);
    mpDebugPrint("mBssType=%d,mPwrState=%d", mBssType, mPwrState);
    mpDebugPrint("hello=%d,mPwrState=%d", mBssType, mPwrState);
    mpDebugPrint("encryption_type=%d", mp_get_encryption_type(macp->chip));

#if 0
    z.cmd = ZD_IOCTL_CAM_DUMP;
	memset(&ifr, 0, sizeof(ifr));
//	strncpy(ifr.ifr_name, drv->ifname, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &z;
	if (ioctl(0, ZDAPIOCTL, &ifr) < 0) {
        mpDebugPrint("ioctl ZDAPIOCTL failed errno=%d", errno);
    }
    else {
        mpDebugPrint("CAM Dump=%s", macp->zdreq.data);
    }
#endif

}

/* ----------  softirq.c  ---------- */

void tasklet_kill(struct tasklet_struct *t)
{
    EventSet(t->event_id, 1);
}

#if !Make_ADHOC
BOOLEAN zd_CmdProcess(U16 CmdId, void *parm1, U32 parm2)
{
    return 0;
}
#endif
void zd1211_set_multicast(struct zd1205_private *macp)
{

}
void
zd1205_watchdog(struct zd1205_private *macp)
{

}
void zd1205_house_keeping(struct zd1205_private *macp)
{
}
void zd1205_connect_mon(struct zd1205_private *macp)
{

}
void zd1205_DeviceReset(struct zd1205_private *macp)
{
}
void zd1205_tx_isr(struct zd1205_private *macp)
{
}
int zd1205_dis_connect(struct zd1205_private *macp)
{
    return 0;
}
void zd_SigProcess(void)
{
    return;
}
struct rx_list_elem *zd1205_start_ru(struct zd1205_private *macp)
{
    return 0;
}
void zd1205_enable_int(void)
{
}
u32 zd1205_rx_isr(struct zd1205_private *macp)
{
}
void zd1205_alloc_skbs(struct zd1205_private *macp)
{
}
int zd1211_USB_PACKAGE_WRITE_REGISTER(u16 *Address, u16 *Value, u16 RegCount, u8 bAddUSBCSRAddress)
{
}

BYTE zd_rf_off = FALSE;

BYTE zd_get_rf_flag()
{
   return zd_rf_off;
}
void mp_zd1211_disable_rf(void)
{
    struct net_device *dev = &g_net_device3;
    struct zd1205_private *macp = dev->priv;

    MP_DEBUG("macp=%p, chip=%p", macp, macp->chip);
    if (macp && macp->chip)
    {
        zd_chip_switch_radio_off(macp->chip);
		zd_rf_off = TRUE;
    }
}

void mp_zd1211_enable_rf(void)
{
    struct net_device *dev = &g_net_device3;
    struct zd1205_private *macp = dev->priv;

    MP_DEBUG("macp=%p, chip=%p", macp, macp->chip);
    if (macp && macp->chip)
    {
        zd_chip_switch_radio_on(macp->chip);
		zd_rf_off = FALSE;

    }
}
