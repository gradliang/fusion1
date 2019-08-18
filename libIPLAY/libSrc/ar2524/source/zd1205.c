/*  src/zd1205.c
*
* 
*
* Copyright (C) 2004 ZyDAS Inc.  All Rights Reserved.
* --------------------------------------------------------------------
*
* 
*
*   The contents of this file are subject to the Mozilla Public
*   License Version 1.1 (the "License"); you may not use this file
*   except in compliance with the License. You may obtain a copy of
*   the License at http://www.mozilla.org/MPL/
*
*   Software distributed under the License is distributed on an "AS
*   IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*   implied. See the License for the specific language governing
*   rights and limitations under the License.
*
*   Alternatively, the contents of this file may be used under the
*   terms of the GNU Public License version 2 (the "GPL"), in which
*   case the provisions of the GPL are applicable instead of the
*   above.  If you wish to allow the use of your version of this file
*   only under the terms of the GPL and not to allow others to use
*   your version of this file under the MPL, indicate your decision
*   by deleting the provisions above and replace them with the notice
*   and other provisions required by the GPL.  If you do not delete
*   the provisions above, a recipient may use your version of this
*   file under either the MPL or the GPL.
*
* -------------------------------------------------------------------- */
#define __KERNEL_SYSCALLS__

#define LOCAL_DEBUG_ENABLE 0

#include <sys/errno.h>
#include <sys/types.h>
#include "typedef.h"
#include "ndebug.h"
#include <net/checksum.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/errno.h>

#include "zdmpx.h"

#include "zd1205.h"
#include "zdinlinef.h"                 
#include "zddebug.h"
#include "zddebug2.h"
#include "menu_drv_macro.h"
#include "zdhw.h"
#include "zdsorts.h"                        
#include "zdglobal.h"
#include "zdutils.h"
#include "zdmisc.h"
#include "zdhci.h"
#include "taskid.h"

#ifdef HOST_IF_USB 
	#include "zd1211.h"
#endif	


#if WIRELESS_EXT > 12
    #include <net/iw_handler.h>        
#endif

#if ZDCONF_LP_SUPPORT == 1
#include "zdlpmgt.h"
#include "zdturbo_burst.h"
#endif
#if ZDPRODUCTIOCTL
#include "zdreq.h"
#endif
#if ZDCONF_FULL_REGTABLE == 1
#include "zdregDomain.h"
#endif
//#include "zd_chip.h"
//#include "net_socket.h"
extern U16 mTmRetryConnect;
extern BOOLEAN mProbeWithSsid;
extern u8 mMacMode;
extern U8 mBssType;
extern Element mSsid;
extern Element dot11DesiredSsid;
int errno;
extern u8 mCurrConnUser;
extern U8 mNumBOnlySta;

extern u8 mBssNum;
extern U8 mKeyFormat; //Init value: WEP64_USED(1)
extern BOOLEAN mPrivacyInvoked; // Init value: FALSE
extern U8 mKeyVector[4][16]; // Store WEP key 
extern U8 mWepKeyLen;
extern U8 mKeyId;  // Init value: 0
extern U16 mCap;   // Init value: CAP_ESS(1);
extern u16 CurrScanCH;
extern MacAddr_t dot11MacAddress;

extern BOOLEAN zd_CmdProbeReq(U8 bWithSSID);
extern Hash_t *HashSearch(MacAddr_t *pMac);
extern void re_initFdescBuf(void);
int sta_lock_global, sta_flaglock_global;
int local_sta_lock_global, local_key_lock_global, local_sta_bss_lock_global;
int mac_lock_global;
int intr_lock_global, tx_lock_global, rx_lock_global;
int key_mutex_global, rate_ctrl_mutex_global;
extern int ar2524_initialized;

/******************************************************************************
*						   C O N S T A N T S
*******************************************************************************
*/

static u8	ZD_SNAP_HEADER[6] = {0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00};
static u8	ZD_SNAP_BRIDGE_TUNNEL[6] = {0xAA, 0xAA, 0x03, 0x00, 0x00, 0xF8};
static u8  zd_Snap_Apple_Type[] = {0xAA,0xAA,0x03,0x08,0x00,0x07,0x80,0x9b};
//Slow Pairwise key install issue is casued by a too fast response 1/2
//group key update before PTK is installed. The gorup update is discarded
//caused key update fails.
//<Slow Pairwise Key Install Fix>
static u8   ZD_SNAP_EAPOL[] = {0xAA,0xAA,0x03, 0x00,0x00,0x00, 0x88,0x8E};
//</Slow Pairwise Key Install Fix>

static u16 IPX=0x8137;
//static u16 NOVELL=0xe0e0;
static u16 APPLE_TALK=0x80f3;
static u16 EAPOL=0x888e;
#if ZDCONF_APC == 1
U8 APC_NT[16][6];
#endif

extern struct net_device g_net_device3;


#define MAX_MULTICAST_ADDRS     32
#define NUM_WEPKEYS     4

#define bGroup(pWlanHdr)			(pWlanHdr->Address1[0] & BIT_0)
#define getSeq(pWlanHdr)			(((u16)pWlanHdr->SeqCtrl[1] << 4) + (u16)((pWlanHdr->SeqCtrl[0] & 0xF0) >> 4))
#define getFrag(pWlanHdr)			(pWlanHdr->SeqCtrl[0] & 0x0F)
#define	getTA(pWlanHdr)				(&pWlanHdr->Address2[0])
#define isWDS(pWlanHdr)				(((pWlanHdr->FrameCtrl[1] & TO_DS_FROM_DS) == TO_DS_FROM_DS) ? 1 : 0) 
#define bRetryBit(pWlanHdr)			(pWlanHdr->FrameCtrl[1] & RETRY_BIT)
#define bWepBit(pWlanHdr)			(pWlanHdr->FrameCtrl[1] & ENCRY_BIT) 
#define bMoreFrag(pWlanHdr)			(pWlanHdr->FrameCtrl[1] & MORE_FRAG)
#define bMoreData(pWlanHdr)			(pWlanHdr->FrameCtrl[1] & MORE_DATA)
#define bPSM(pWlanHdr)              (pWlanHdr->FrameCtrl[1] & PWR_BIT)
#define BaseFrameType(pWlanHdr)		(pWlanHdr->FrameCtrl[0] & 0x0C)
#define SubFrameType(pWlanHdr)		(pWlanHdr->FrameCtrl[0])
#define bDataMgtFrame(pWlanHdr)		(((pWlanHdr->FrameCtrl[0] & 0x04) == 0))
#ifndef HOST_IF_USB 
    #define nowT()					(zd_readl(TSF_LowPart))  //us unit
#else
    #define nowT()					(jiffies) //tick (10ms) unit
#endif    
/******************************************************************************
*			   F U N C T I O N	 D E C L A R A T I O N S
*******************************************************************************
*/

//wireless extension helper functions
void zd1205_lock(struct zd1205_private *macp);
void zd1205_unlock(struct zd1205_private *macp);
static int zd1205_ioctl_setiwencode(struct net_device *dev, struct iw_point *erq, char* key);
static int zd1205_ioctl_getiwencode(struct net_device *dev, struct iw_point *erq, char* key);
static int zd1205_ioctl_setessid(struct net_device *dev, struct iw_point *erq);
static int zd1205_ioctl_setbssid(struct net_device *dev, struct iwreq *wrq);

static int zd1205_ioctl_getessid(struct net_device *dev, struct iw_point *erq);
static int zd1205_ioctl_setfreq(struct net_device *dev, struct iw_freq *frq);
//static int zd1205_ioctl_setsens(struct net_device *dev, struct iw_param *srq);
static int zd1205_ioctl_setrts(struct net_device *dev, struct iw_param *rrq);
static int zd1205_ioctl_setfrag(struct net_device *dev, struct iw_param *frq);

static int zd1205_ioctl_getfrag(struct net_device *dev, struct iw_param *frq);
static int zd1205_ioctl_setrate(struct net_device *dev, struct iw_param *frq);
static int zd1205_ioctl_getrate(struct net_device *dev, struct iw_param *frq);
//static int zd1205_ioctl_settxpower(struct net_device *dev, struct iw_param *prq);
//static int zd1205_ioctl_gettxpower(struct net_device *dev, struct iw_param *prq);
static int zd1205_ioctl_setpower(struct net_device *dev, struct iw_param *prq);
static int zd1205_ioctl_getpower(struct net_device *dev, struct iw_param *prq);
static int zd1205_ioctl_setmode(struct net_device *dev, __u32 *mode);

/* Wireless Extension Handler functions */
static int zd1205wext_giwfreq(struct net_device *dev, struct iw_request_info *info, struct iw_freq *freq, char *extra);
static int zd1205wext_siwmode(struct net_device *dev, struct iw_request_info *info, __u32 *mode, char *extra);
static int zd1205wext_giwmode(struct net_device *dev, struct iw_request_info *info, __u32 *mode, char *extra);
static int zd1205wext_giwrate(struct net_device *dev, struct iw_request_info *info, struct iw_param *rrq, char *extra);
static int zd1205wext_giwrts(struct net_device *dev, struct iw_request_info *info, struct iw_param *rts, char *extra);
static int zd1205wext_giwfrag(struct net_device *dev, struct iw_request_info *info, struct iw_param *frag, char *extra);
//static int zd1205wext_giwtxpow(struct net_device *dev, struct iw_request_info *info, struct iw_param *rrq, char *extra);
//static int zd1205wext_siwtxpow(struct net_device *dev, struct iw_request_info *info, struct iw_param *rrq, char *extra);
static int zd1205wext_giwrange(struct net_device *dev, struct iw_request_info *info, struct iw_point *data, char *extra);



#if WIRELESS_EXT > 13
static int zd1205wext_siwscan(struct net_device *dev, struct iw_request_info *info, struct iw_point *data, char *extra);
static int zd1205wext_giwscan(struct net_device *dev, struct iw_request_info *info, struct iw_point *data, char *extra);
#endif

/* functions to support 802.11 protocol stack */
void zdcb_rx_ind(U8 *pData, U32 length, void *buf, U32 LP_MAP);
void zdcb_release_buffer(void *buf);
void zdcb_tx_completed(void);
void zdcb_start_timer(U32 timeout, U32 event);
void zdcb_stop_timer(U32 TimerId);
void zd1205_set_zd_cbs(zd_80211Obj_t *pObj);
void zdcb_set_reg(void *reg, U32 offset, U32 value);
void chal_tout_cb(unsigned long ptr);
unsigned long zdcb_dis_intr(void);
void zdcb_set_intr_mask(unsigned long flags);
BOOLEAN zdcb_check_tcb_avail(U8	num_of_frag);
BOOLEAN zdcb_setup_next_send(fragInfo_t *frag_info);
void zd1205_start_download(u32 phyAddr);

//wireless extension helper functions    
/* taken from orinoco.c ;-) */
const u32 channel_frequency[] = {
	2412, 2417, 2422, 2427, 2432, 2437, 2442,
	2447, 2452, 2457, 2462, 2467, 2472, 2484

};
#define NUM_CHANNELS ( sizeof(channel_frequency) / sizeof(channel_frequency[0]) )
#define MAX_KEY_SIZE    13
/******************************************************************************

*						 P U B L I C   D A T A
*******************************************************************************
*/
static BOOLEAN AsocTimerStat = FALSE; //If the Asoc Timer is enabled
extern Hash_t *sstByAid[MAX_RECORD];

struct net_device *g_dev;
u8 *mTxOFDMType; 
zd_80211Obj_t dot11Obj = {0};
extern zd_80211Obj_t *pdot11Obj;
struct zd1205_private macp_buf;
struct zd1205_private *macp_global = &macp_buf;
extern int wpa_4way;
#ifdef PLATFORM_MPIXEL
#if Make_WPA
extern int is_packet_4_of_4;
#endif
#endif
#if Make_ADHOC
u8 OfdmRateTbl[12] = {
	0x00,  //1M
	0x01,  //2M
	0x02,  //5.5M
	0x03,  //11M
	0x1b,  //6M
	0x1f,  //9M
	0x1a,  //12M
	0x1e,  //18M
	0x19,  //24M
	0x1d,  //36M
	0x18,  //48M
	0x1c   //54M
};	
#endif
void zdcb_set_reg(void *reg, U32 offset, U32 value);
U32 zdcb_get_reg(void *reg, U32 offset);
void udelay(unsigned long usecs);
/* Definition of Wireless Extension */

/*
 * Structures to export the Wireless Handlers

 */
#if ZDCONF_REGION_CONFIG == 1
typedef enum _ZD_REGION
{
 ZD_REGION_Default   = 0x00,//All channel
 ZD_REGION_USA    = 0x10,//G channel->ch1-11;
 ZD_REGION_Canada   = 0x20,//G channel->ch1-11;
 ZD_REGION_Argentina         = 0x21,//G channel->ch1-11;
 ZD_REGION_Brazil            = 0x22,//G channel->ch1-11;
 ZD_REGION_Europe         = 0x30,//G channel->ETSI ch1-13;
 ZD_REGION_Spain    = 0x31,//G channel->ETSI ch1-13;
 ZD_REGION_France   = 0x32,//G channel->ch10-13;
 ZD_REGION_Ukraine           = 0x33,//G channel->ch1-11;
 ZD_REGION_AustriaBelgium    = 0x34,//Austria and Belgium G channel->ch1-13;;
 ZD_REGION_Switzerland       = 0x35,//G channel->ch1-13;
 ZD_REGION_Japan    = 0x40,//G channel->ch1-14;
 ZD_REGION_Australia         = 0x42,//G channel->ch1-13;
 ZD_REGION_China             = 0x43,//G channel->ch1-11;
 ZD_REGION_HongKong          = 0x44,//G channel->ch1-11;
 ZD_REGION_Korea             = 0x45,//G channel->ch1-11;
 ZD_REGION_NewZealand        = 0x46,//G channel->ch1-11;
 ZD_REGION_Singapore         = 0x47,//G channel->ch10-13;
 ZD_REGION_Taiwan            = 0x48,//G channel->ch1-13;
 ZD_REGION_Israel   = 0x50,//G channel->ch3-9;
 ZD_REGION_Mexico   = 0x51 //G channel->ch10,11;
} ZD_REGION;
#endif

#define ZD1211_DBG_LEVEL    1

#define MAX_NUM_WORKQUEUES   2
static int workqueue_events[MAX_NUM_WORKQUEUES];
static int workqueue_semaphores[MAX_NUM_WORKQUEUES];
static int workqueue_tasks[MAX_NUM_WORKQUEUES];

int worker_thread0(void);
int worker_thread1(void);

#define MAX_NUM_SEMAPHORES   12
static int mp_zd1211_semaphores[MAX_NUM_SEMAPHORES]; /* a semaphore pool */
static int mp_zd1211_semaphore_index;

#define MAX_NUM_TASKLETS   1
static int tasklet_events[MAX_NUM_TASKLETS];
static int tasklet_tasks[MAX_NUM_TASKLETS];
static void *tasklet_funcs[MAX_NUM_TASKLETS];
static u32 tasklet_data[MAX_NUM_TASKLETS];

static void tasklet_thread0(void);

void mp_zd1211_preinit(void)
{
	struct zd1205_private *macp = macp_global;
	card_Setting_t *pSetting = &macp->cardSetting;
    int ret;

    mp_zd1211_semaphore_index = 0;
    wpa_4way = 0;
#ifdef PLATFORM_MPIXEL
#if Make_WPA
    is_packet_4_of_4 = 0;
#endif
#endif

	/* 
	 * local->sta_lock
	 */
    if (!local_sta_lock_global)
    {
        ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        MP_ASSERT(ret > 0);
        local_sta_lock_global = ret;
    }

	/* 
	 * local->key_lock
	 */
    if (!local_key_lock_global)
    {
        ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        MP_ASSERT(ret > 0);
        local_key_lock_global = ret;
    }
	/* 
	 * local->sta_bss_lock
	 */
    if (!local_sta_bss_lock_global)
    {
        ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        MP_ASSERT(ret > 0);
        local_sta_bss_lock_global = ret;
    }

	/* 
	 * mac->lock
	 */
    if (!mac_lock_global)
    {
        ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        MP_ASSERT(ret > 0);
        mac_lock_global = ret;
    }

	/* 
	 * intr->lock
	 */
    if (!intr_lock_global)
    {
        ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        MP_ASSERT(ret > 0);
        intr_lock_global = ret;
    }

	/* 
	 * rx->lock
	 */
    if (!rx_lock_global)
    {
        ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        MP_ASSERT(ret > 0);
        rx_lock_global = ret;
    }

	/* 
	 * tx->lock
	 */
    if (!tx_lock_global)
    {
        ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        MP_ASSERT(ret > 0);
        tx_lock_global = ret;
    }

	/* 
	 * key_mutex.semaphore
	 */
    if (!key_mutex_global)
    {
        ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        MP_ASSERT(ret > 0);
        key_mutex_global = ret;
    }

	/* 
	 * key_mutex.semaphore
	 */
    if (!rate_ctrl_mutex_global)
    {
        ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        MP_ASSERT(ret > 0);
        rate_ctrl_mutex_global = ret;
    }

	/* 
	 * workqueues
	 */
    if (!ar2524_initialized)
    {
        short i;
        for (i=0; i<MAX_NUM_WORKQUEUES; i++)
        {
            ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
            MP_ASSERT(ret > 0);
            workqueue_semaphores[i] = ret;

            ret = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_WAIT_SINGLE|OS_ATTR_EVENT_CLEAR, 0);
            MP_ASSERT(ret > 0);
            workqueue_events[i] = ret;
        }

        ret = mpx_TaskCreate(worker_thread0, WIFI_PRIORITY, 0x2000);
        MP_ASSERT(ret > 0);
        workqueue_tasks[0] = ret;

        ret = mpx_TaskCreate(worker_thread1, WIFI_PRIORITY, 0x2000);
        MP_ASSERT(ret > 0);
        workqueue_tasks[1] = ret;
    }

	/* 
	 * tastlet
	 */
    if (!ar2524_initialized)
    {
        short i;
        ret = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_WAIT_SINGLE|OS_ATTR_EVENT_CLEAR, 0);
        MP_ASSERT(ret > 0);
        tasklet_events[0] = ret;

        ret = mpx_TaskCreate(tasklet_thread0, WIFI_PRIORITY, 0x2000);
        MP_ASSERT(ret > 0);
        tasklet_tasks[0] = ret;
    }

	/* 
	 * general use
	 */
    if (!ar2524_initialized)
    {
        short i;
        for (i=0; i<MAX_NUM_SEMAPHORES; i++)
        {
            ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
            MP_ASSERT(ret > 0);
            mp_zd1211_semaphores[i] = ret;
        }

    }

}

void mp_zd1211_init(struct usb_device *usb, struct zd_chip *chip)
{
	struct zd1205_private *macp = macp_global;
	card_Setting_t *pSetting = &macp->cardSetting;

	MP_ASSERT(usb);

	macp->chip = chip;
	macp->usb = usb;
	macp->USBCSRAddress = 0x9000;
	macp->dbg_flag = ZD1211_DBG_LEVEL;
	macp->cardSetting.MacMode = mMacMode;
    if (macp->conf_lock == 0)
    {
        static int do_once;
		MP_ASSERT(do_once == 0);
		int ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
		MP_ASSERT(ret > 0);
		if (ret > 0)
			macp->conf_lock = ret;
        do_once++;
    }


//    pdot11Obj = &dot11Obj;
	memset(&dot11Obj, 0, sizeof dot11Obj);
	dot11Obj.reg = chip;
	dot11Obj.SetReg = zdcb_set_reg;
	dot11Obj.GetReg = zdcb_get_reg;
	dot11Obj.DelayUs = udelay;

	/* 
	 * zd1205_init_card_setting()
	 */
	pSetting->OperationMode = CAM_STA;
	pSetting->BssType = INFRASTRUCTURE_BSS;
    pSetting->EncryOnOff = 0;
    pSetting->EncryMode = NO_WEP;
    pSetting->EncryKeyId = 0;
    pSetting->WPAIeLen = 0;
    pSetting->SwCipher = 0;
	pSetting->MacMode = mMacMode;
	pSetting->ShortSlotTime = 1;
	pSetting->PreambleType = 1; //short preamble
	pSetting->RadioOn = 1;
	MP_ASSERT(pSetting->MacMode == MIXED_MODE);

    memset(pSetting->WPAIe,0,sizeof(pSetting->WPAIe));

    /* ----------  ZyDAS vendor driver's net_device  ---------- */
 	zd1211_InitSetup(&g_net_device3, macp);
	set_bit(__LINK_STATE_START, &g_net_device3.state);

	/* 
	 * zd_CmdProcess() case CMD_RESET_80211
	 */
    zd_Reset80211(&dot11Obj);
	MP_ASSERT(sstByAid[0]);

	initWepState();
}

u8 mp_get_rf_type(struct zd_chip *chip);
void mp_zd1211_open(struct zd_chip *chip)
{
	struct zd1205_private *macp = macp_global;
	card_Setting_t *pSetting = &macp->cardSetting;
    int ret;
    extern u32 pod_value_global;

	macp->RF_Mode = pod_value_global;
    mpDebugPrint("%s: pod=%x", __func__,macp->RF_Mode);
//	macp->RF_Mode = mp_get_rf_type(chip);

	/* 
	 * zd1205_init()
	 */
    macp->PA_Type = ((macp->RF_Mode) >> 16) & 0xF;//hardware type 2, bit0-3
    printk(KERN_ERR "PA type: %01x\n", macp->PA_Type);

    pdot11Obj->HWFeature = macp->RF_Mode & 0xfff0;
    if((macp->RF_Mode >> 16) & BIT_15)
    {
        printk("PHYNEWLayout = 1\n");
        pdot11Obj->PHYNEWLayout = 1;
    }
    if((macp->RF_Mode >> 16) & BIT_7)
    {
        printk("PHY_Decrease_CR128_state = 1\n");
        pdot11Obj->PHY_Decrease_CR128_state = 1;
    }

    if (((macp->RF_Mode & 0xf) == AL2230_RF) && (pdot11Obj->HWFeature & BIT_7) )
        macp->RF_Mode = AL2230S_RF;
    else
        macp->RF_Mode &= 0x0f;

  	pdot11Obj->rfMode = macp->RF_Mode;

    if (pdot11Obj->rfMode == UW2453_RF || pdot11Obj->rfMode == AR2124_RF
            || pdot11Obj->rfMode == AL2230_RF || pdot11Obj->rfMode == AL2230S_RF)
        pdot11Obj->S_bit_cnt = 24;

	/* 
	 * zd1205_open()
	 */
    HW_CAM_Write(&dot11Obj, DEFAULT_ENCRY_TYPE, 0);

	/* 
	 * zd_StartSTA()
	 */
	pdot11Obj->SetReg(chip, ZD_CAM_MODE, CAM_STA); 

	/* 
	 * sta->lock
	 */
    if (!sta_lock_global)
    {
        ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        MP_ASSERT(ret > 0);
        sta_lock_global = ret;
    }

	/* 
	 * sta->flaglock
	 */
    if (!sta_flaglock_global)
    {
        ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        MP_ASSERT(ret > 0);
        sta_flaglock_global = ret;
    }

}

int mp_workqueue_event(char *name)
{
    int ret;
    if (strcmp(name, "ar2524") == 0)
        ret = workqueue_events[0];
    else if (strcmp(name, "ar2524_wiphy") == 0)
        ret = workqueue_events[1];
    else
    {
//        mpDebugPrint("%s: name=%s", __func__,name);
        MP_ASSERT(0);
        ret = 0;
    }
    return ret;
}

int mp_workqueue_semaphore(char *name)
{
    int ret;
    if (strcmp(name, "ar2524") == 0)
        ret = workqueue_semaphores[0];
    else if (strcmp(name, "ar2524_wiphy") == 0)
        ret = workqueue_semaphores[1];
    else
    {
        MP_ASSERT(0);
        ret = 0;
    }
    return ret;
}

int mp_workqueue_task(char *name)
{
    int ret;
    if (strcmp(name, "ar2524") == 0)
        ret = workqueue_tasks[0];
    else if (strcmp(name, "ar2524_wiphy") == 0)
        ret = workqueue_tasks[1];
    else
    {
        MP_ASSERT(0);
        ret = 0;
    }
    return ret;
}

int mp_tasklet_event(void)
{
    return tasklet_events[0];
}

int mp_tasklet_task(void *func, unsigned long data)
{
    int ret;
    int free1 = -1, i;
    for (i=0; i<MAX_NUM_TASKLETS; i++)
    {
        if ((u32)tasklet_funcs[i] == 0)
        {
            if (free1 < 0)
                free1 = i;
        }

        if ((u32)tasklet_funcs[i] == (u32)func)
            break;
    }

    if (i<MAX_NUM_TASKLETS)
    {
        tasklet_data[i] = data;
        return tasklet_tasks[i];
    }
    else if (free1 >= 0)
    {
        i = free1;
        tasklet_funcs[i] = func;
        tasklet_data[i] = data;

        return tasklet_tasks[i];
    }
    else
    {
        MP_ASSERT(0);
        ret = -1;
    }
    return ret;
}

static void tasklet_thread0(void)
{
    void (*func)(unsigned long);
    MP_ASSERT(tasklet_funcs[0]);
    MP_ASSERT(tasklet_data[0]);

    func = tasklet_funcs[0];

    while (1)
    {
        mpDebugPrint("%s: enter func", __func__);
        func(tasklet_data[0]);
        mpDebugPrint("%s: exit func", __func__);
        TaskSleep(0);
    }
}

int mp_OsSemaphore(void)
{
    MP_ASSERT(mp_zd1211_semaphore_index < MAX_NUM_SEMAPHORES);
	mpDebugPrint("%s: mp_zd1211_semaphore_index=%d", __func__, mp_zd1211_semaphore_index);
    return mp_zd1211_semaphores[mp_zd1211_semaphore_index++];
}

u8	a_get_cal_36M_setpoint_val( u8 index)
{
	u32	tmpvalue;

	if (index < 16){
		tmpvalue=zd_readl(ZD_E2P_A36M_CAL_VALUE+((index>>2)<<2));
		return ((u8)(tmpvalue >> (index%4*8)));
	}
	else{
		printk("Error in a_get_cal_36M_setpoint_val\n");
		return FALSE;
	}
}

void zd1205_ArReset(struct zd1205_private *macp)
{
	u8 i;
	defrag_Array_t *pArray = &macp->defragArray;

	for (i=0; i<MAX_DEFRAG_NUM; i++)
		pArray->mpdu[i].inUse = 0;
}

void zd1205_ClearTupleCache(struct zd1205_private *macp)
{
	int i;
	tuple_Cache_t *pCache = &macp->cache;
	
	pCache->freeTpi = 0;
	for (i=0; i<TUPLE_CACHE_SIZE; i++){
		pCache->cache[i].full = 0;
	}
}	


u8 zd1205_SearchTupleCache(struct zd1205_private *macp, u8 *pAddr, u16 seq, u8 frag)
{
 	int k;
	tuple_Cache_t *pCache = &macp->cache;
	
	for (k=0; k<TUPLE_CACHE_SIZE; k++){
		if ((memcmp((char *)&pCache->cache[k].ta[0], (char *)pAddr, 6) == 0) 
			&& (pCache->cache[k].sn == seq) && (pCache->cache[k].fn == frag)
			&& (pCache->cache[k].full))
			return 1;
	}
	
	return 0;			
}

static void zd1205_config(struct zd1205_private *macp)
{
	u32 tmpValue;
	int i, jj;
    extern int zd1211_allowChannel;

    ZENTER(1);
    

    // Retrieve Feature BitMap
    zd_writel(macp->cardSetting.EncryMode, EncryptionType);
    macp->dtimCount = 0;

    
    /* Setup Physical Address */
	zd_writel(cpu_to_le32(*(u32 *)&macp->macAdr[0]), MACAddr_P1);
	zd_writel(cpu_to_le32(*(u32 *)&macp->macAdr[4]), MACAddr_P2);
#if ZDCONF_AP_SUPPORT == 1
	if (macp->cardSetting.BssType == AP_BSS){
		/* Set bssid = MacAddress */
		macp->BSSID[0] = macp->macAdr[0];
		macp->BSSID[1] = macp->macAdr[1];
		macp->BSSID[2] = macp->macAdr[2];
		macp->BSSID[3] = macp->macAdr[3];
 		macp->BSSID[4] = macp->macAdr[4];
		macp->BSSID[5] = macp->macAdr[5];
 		zd_writel(cpu_to_le32(*(u32 *)&macp->macAdr[0]), BSSID_P1);
		zd_writel(cpu_to_le32(*(u32 *)&macp->macAdr[4]), BSSID_P2);
	}
	else 
#endif
    {
		zd_writel(STA_RX_FILTER, ZD_Rx_Filter);
	}		
	

    macp->intrMask = ZD1205_INT_MASK;

	if (macp->intrMask & DTIM_NOTIFY_EN)
 		macp->dtim_notify_en = 1;
	else 
		macp->dtim_notify_en = 0;	
	

	if (macp->intrMask & CFG_NEXT_BCN_EN)

		macp->config_next_bcn_en = 1;
	else 
		macp->config_next_bcn_en = 0;



    zd1205_ClearTupleCache(macp);
	zd1205_ArReset(macp);

	macp->bTraceSetPoint = 1;
	macp->bFixedRate = 0;
    dot11Obj.bDeviceInSleep = 0; 

   	macp->bGkInstalled = 0;
    macp->PwrState = PS_CAM;

	// Get Allowed Channel and Default Channel
#ifdef HOST_IF_USB
	tmpValue = zd_readl(E2P_SUBID);
 	macp->RegionCode = (u16)(tmpValue >> 16);

    if(zd1211_allowChannel == -1)
    {
#if ZDCONF_FULL_REGTABLE == 1
        dot11Obj.AllowedChannel = zd_getReg_AllowedChannel(macp->RegionCode);//zd_readl(ZD_E2P_ALLOWED_CHANNEL);
        if(dot11Obj.AllowedChannel == 0)
        {
            printk("Unknown regionCode = %04x\n",macp->RegionCode);
            dot11Obj.AllowedChannel = 0x107FF;
        }
#else
        dot11Obj.AllowedChannel = 0x107FF;
#endif
    }
    else
    {
        dot11Obj.AllowedChannel = zd1211_allowChannel;
        printk("User Specify AllowChannel = %08x\n",dot11Obj.AllowedChannel);
    }

	
	if (!(dot11Obj.AllowedChannel & 0xFFFF0000)){
		dot11Obj.AllowedChannel |= 0x10000;
	}

	dot11Obj.RegionCode = macp->RegionCode;
    macp->LinkLEDn = LED1;
    printk(KERN_DEBUG "zd1205_config: EEP(HWFeature)=0x%X\n", dot11Obj.HWFeature);
    if (dot11Obj.HWFeature & BIT_4){


        macp->LinkLEDn = LED2;
        ZD1211DEBUG(0, "LED2\n");
    }
    ZD1211DEBUG(0, "LinkLEDn = %x\n", macp->LinkLEDn);    

    if (dot11Obj.HWFeature & BIT_8){

		dot11Obj.bOverWritePhyRegFromE2P = 1;
        ZD1211DEBUG(0, "OverWritePhyRegFromE2P\n");


    }    


	if (dot11Obj.HWFeature & BIT_9){
		dot11Obj.bIsNormalSize = 1;
        ZD1211DEBUG(0, "NormalSize\n");
    }
    
    macp->LinkLED_OnDur = 2;
    macp->LinkLED_OffDur = 1;
    macp->DataLED = 0;
    if (dot11Obj.HWFeature & BIT_24){

        macp->LinkLED_OnDur = ((dot11Obj.HWFeature) >> 25) & 0x3;
        macp->LinkLED_OffDur = ((dot11Obj.HWFeature) >> 27) & 0x3;
        if (dot11Obj.HWFeature & BIT_29)
            macp->DataLED = 1;
    }
    ZD1211DEBUG(1, "LinkLED_OnDur = %d\n", macp->LinkLED_OnDur);

    ZD1211DEBUG(1, "LinkLED_OffDur = %d\n", macp->LinkLED_OffDur);

    if (!(dot11Obj.HWFeature & BIT_10)){ // The IPC protection: the default is disablesd 
		macp->IPCFlag = 4;
    }    
	
	tmpValue = zd_readl(FW_USB_SPEED);
	dot11Obj.IsUSB2_0 = (u8) tmpValue;
#else
	dot11Obj.bIsNormalSize = 1;
 	dot11Obj.IsUSB2_0 = 1;
#endif

	printk("AllowedChannel = %08x\n", (u32)dot11Obj.AllowedChannel);
    printk("Region:%u\n",(u32) dot11Obj.RegionCode);


	ZD1211DEBUG(1, "IsUSB2_0 = %d\n", dot11Obj.IsUSB2_0);
	// read Set Point from EEPROM

	tmpValue = zd_readl(ZD_E2P_PWR_INT_VALUE1);
	tmpValue -= cPWR_INT_VALUE_GUARD;
	dot11Obj.IntValue[0] = (u8)tmpValue;
	dot11Obj.IntValue[1] = (u8)(tmpValue >> 8);

	dot11Obj.IntValue[2] = (u8)(tmpValue >> 16);
	dot11Obj.IntValue[3] = (u8)(tmpValue >> 24);
	
	tmpValue = zd_readl(ZD_E2P_PWR_INT_VALUE2);
	tmpValue -= cPWR_INT_VALUE_GUARD;
	dot11Obj.IntValue[4] = (u8)tmpValue;
	dot11Obj.IntValue[5] = (u8)(tmpValue >> 8);
	dot11Obj.IntValue[6] = (u8)(tmpValue >> 16);
	dot11Obj.IntValue[7] = (u8)(tmpValue >> 24);
	
	tmpValue = zd_readl(ZD_E2P_PWR_INT_VALUE3);
	tmpValue -= cPWR_INT_VALUE_GUARD;
	dot11Obj.IntValue[8] = (u8)tmpValue;
	dot11Obj.IntValue[9] = (u8)(tmpValue >> 8);
	dot11Obj.IntValue[10] = (u8)(tmpValue >> 16);
	dot11Obj.IntValue[11] = (u8)(tmpValue >> 24);
	
	tmpValue = zd_readl(ZD_E2P_PWR_INT_VALUE4);
	tmpValue -= cPWR_INT_VALUE_GUARD;
	dot11Obj.IntValue[12] = (u8)tmpValue;
	dot11Obj.IntValue[13] = (u8)(tmpValue >> 8);

	//Initiate a_Calibration_Data CH field
#if ZDCONF_80211A_SUPPORT == 1
	for (i=0;i<a_CALIBRATED_CH_NUM;i++){
		if((i == 0) || ((i != 0) && (a_ChannelMap[i-1] != a_ChannelMap[i]))){
			a_Calibration_Data[0][i] = a_ChannelMap[i];		
		}
		else{
			a_Calibration_Data[0][i] = 0xff;
		}
	}	
	//Initiate a_Interpolation_Data CH field
	for (i=0;i<a_INTERPOLATION_CH_NUM;i++)
		a_Interpolation_Data[0][i] = a_InterpolationTbl[i].a_Channel;

	for (i=0;i<a_CALIBRATED_CH_NUM;i++){
		//Adapter->a_Calibration_Data[0][i] = a_ChannelMap[i];
		if(a_Calibration_Data[0][i] != 0xff){
			a_Calibration_Data[1][i] = a_get_cal_int_val((u8)i) - cPWR_INT_VALUE_GUARD;
			a_Calibration_Data[2][i] = a_get_cal_36M_setpoint_val((u8)i);
			a_Calibration_Data[3][i] = a_get_cal_48M_54M_setpoint_val((u8)i);
		}
		else{
			a_Calibration_Data[1][i] = 0xff;
			a_Calibration_Data[2][i] = 0xff;
			a_Calibration_Data[3][i] = 0xff;

		}
	}

	//Calculate Interpolation SetPoints(For 802.11a)
	for (i=0;i<a_INTERPOLATION_CH_NUM;i++){
	//Adapter->a_Interpolation_Data[0][i] = a_InterpolationTbl[i].a_Channel;
		if(a_InterpolationTbl[i].Left_Most_Channel == a_InterpolationTbl[i].Right_Most_Channel){
			for (jj=0;jj<a_CALIBRATED_CH_NUM;jj++){
				if(a_Calibration_Data[0][jj] == a_InterpolationTbl[i].Left_Most_Channel){
					a_Interpolation_Data[1][i] = a_Calibration_Data[1][jj];
					a_Interpolation_Data[2][i] = a_Calibration_Data[2][jj];
					a_Interpolation_Data[3][i] = a_Calibration_Data[3][jj];
				}//Directly used certain calibrated channel values
			}
		}
		else{			
			if(0xff == a_get_interpolation_value((u8)i, &a_Interpolation_Data[1][i],
				&a_Interpolation_Data[2][i], &a_Interpolation_Data[3][i]))
				printk("Get Int/Cal wrong in Interpolation Tbl(%d)\n",i);
		}
	}
	//End Calculate Interpolation SetPoints(For 802.11a)
#endif

	
    
#if fTX_PWR_CTRL
	for (jj = 0; jj < 3; jj ++){

		for (i = 0; i < 4; i++){
 			tmpValue = zd_readl(E2P_36M_CAL_VALUE + jj*0x20 + i*4);
			macp->SetPointOFDM[jj][i*4] = (u8) tmpValue;
			macp->SetPointOFDM[jj][i*4+1] = (u8) (tmpValue >> 8);
			if (i != 3){
				macp->SetPointOFDM[jj][i*4+2] = (u8) (tmpValue >> 16);
				macp->SetPointOFDM[jj][i*4+3] = (u8) (tmpValue >> 24);
			}
		}
	}
#endif


	zd_writel(0x00000064,ZD_BCNInterval);
    HW_UpdateBcnInterval(&dot11Obj, 0x00000064);

   	// read Set Point from EEPROM
	tmpValue = zd_readl(ZD_E2P_PWR_CAL_VALUE1);
	macp->EepSetPoint[0] = (u8)tmpValue;
	macp->EepSetPoint[1] = (u8)(tmpValue >> 8);
	macp->EepSetPoint[2] = (u8)(tmpValue >> 16);
	macp->EepSetPoint[3] = (u8)(tmpValue >> 24);
 	
	tmpValue = zd_readl(ZD_E2P_PWR_CAL_VALUE2);

	macp->EepSetPoint[4] = (u8)tmpValue;

	macp->EepSetPoint[5] = (u8)(tmpValue >> 8);
	macp->EepSetPoint[6] = (u8)(tmpValue >> 16);
	macp->EepSetPoint[7] = (u8)(tmpValue >> 24);

	
	tmpValue = zd_readl(ZD_E2P_PWR_CAL_VALUE3);
	macp->EepSetPoint[8] = (u8)tmpValue;

	macp->EepSetPoint[9] = (u8)(tmpValue >> 8);
	macp->EepSetPoint[10] = (u8)(tmpValue >> 16);
	macp->EepSetPoint[11] = (u8)(tmpValue >> 24);
	
	tmpValue = zd_readl(ZD_E2P_PWR_CAL_VALUE4);
	macp->EepSetPoint[12] = (u8)tmpValue;
	macp->EepSetPoint[13] = (u8)(tmpValue >> 8);

//	HW_SetRfChannel(&dot11Obj, (dot11Obj.AllowedChannel >> 16), 0, MIXED_MODE);
#if ZDCONF_AP_SUPPORT == 1
    if(macp->cardSetting.BssType == AP_BSS)
	    HW_SetRfChannel(&dot11Obj, (dot11Obj.AllowedChannel >> 16), 1, MIXED_MODE);
#endif
	// For Antenna Diversity Parameters
	macp->bEnableSwAntennaDiv = 0;
	macp->Ant_MonitorDur1 = 10;//100;
	macp->Ant_MonitorDur2 = 1;
	macp->NiceSQThr = 48;

	macp->rxOffset = ZD_RX_OFFSET;

	macp->bPSMSupported = 0;
	macp->NormalBackoff = 0x7f047f;
	macp->UrgentBackoff = 0x7f0407;
	macp->LooseBackoff = 0x7f107f;
	macp->WorseSQThr = 0x48;
#if ZDCONF_SETMULTI_SUPPORT == 1
	macp->MulticastAddr[0] = 0;
#endif
	macp->iv16 = 0;
	macp->iv32 = 0;
			macp->EnableTxPwrCtrl = 1;
	macp->PSThreshhold = 10000;
	
#if fANT_DIVERSITY
    //    macp->NiceSQThr_OFDM = 12 * 4;    // 12 dB --> 48 %
    macp->NiceSQThr_OFDM = 48;       // 48 %
    macp->bEnableSwAntennaDiv = 1;
#endif

#if fWRITE_WORD_REG
    macp->FlashType = 0xFF;

#endif


    macp->PHYTestIndex = 5;
	macp->PHYTestRssiBound = 0x3a;
	macp->PHYTestTimer = 30;
	macp->TrafficBound = 200;

	macp->PHYLowPower = 3;  // Tx/Rx enable
	dot11Obj.CR122Flag = 2; // initial value

    dot11Obj.CR31Flag = 2; // initial value
	dot11Obj.CR203Flag = 2; // initial value
	dot11Obj.PhyTest = 4; 
	macp->AdapterMaxRate = 0x0B;  // initail max rate = 54M
    ZEXIT(0);
}	

void zd_writel(u32 value, u32 offset)
{
	struct zd1205_private *macp = macp_global;
	int r;
	if (offset < 0x8000){
		offset += macp_global->USBCSRAddress;
    }
	MP_DEBUG("%s: offset=%x,v=%x", __func__,offset, value);
	r = zd_iowrite32(macp->chip, (zd_addr_t) offset, value);
}
u32 zd_readl(u32 offset)
{
	struct zd1205_private *macp = macp_global;
    u32 val;
	int r;
	if (offset < 0x8000){
		offset += macp_global->USBCSRAddress;
    }
	MP_DEBUG("%s: offset=%x", __func__,offset);
    r = zd_ioread32(macp->chip, (zd_addr_t)offset, &val);
    return val;
}

void zd1205_dump_data(char *info, u8 *data, u32 data_len)
{
#if Make_ADHOC
   int i;

   mpDebugPrint("%s",info);
   NetPacketDump(data, data_len);
#endif
}

void
zd1205_lock(struct zd1205_private *macp)
{
#ifndef HOST_IF_USB
	spin_lock_bh(&macp->conf_lock);
#else
	spin_lock(&macp->conf_lock);
#endif    
}

void
zd1205_unlock(struct zd1205_private *macp)
{
#ifndef HOST_IF_USB    
	spin_unlock_bh(&macp->conf_lock);
#else
	spin_unlock(&macp->conf_lock);
#endif
}

#define fDISABLE_LED            0
void iLED_ON(struct zd1205_private *macp, u32 LEDn)
{
#if !fDISABLE_LED
	
	u32   tmpvalue;

    tmpvalue = zd_readl(rLED_CTRL);
    tmpvalue |= LEDn;
    zd_writel(tmpvalue, rLED_CTRL);

#ifdef ROBIN_KAO
    tmpvalue = zd_readl(FW_LINK_STATUS);
    tmpvalue |= 0x1;
    zd_writel(tmpvalue, FW_LINK_STATUS);
#endif

#endif
}

                                                                                                     
void iLED_OFF(struct zd1205_private *macp, u32 LEDn)
{
#if !fDISABLE_LED	

	u32   tmpvalue;


    tmpvalue = zd_readl(rLED_CTRL);
    tmpvalue &= ~LEDn;
    zd_writel(tmpvalue, rLED_CTRL);


#ifdef ROBIN_KAO
	zd_writel(0x0, FW_LINK_STATUS);
#endif

#endif
}


void iLED_SWITCH(struct zd1205_private *macp, u32 LEDn)
{
#if !fDISABLE_LED	
	u32   tmpvalue;

    tmpvalue = zd_readl(rLED_CTRL);
    tmpvalue ^= LEDn;
    zd_writel(tmpvalue, rLED_CTRL);
#endif    
}
void zd1205_config_wep_keys(struct zd1205_private *macp)
{
	card_Setting_t *pSetting = &macp->cardSetting;
    u32 EncType = 0;

	u8 encryMode = pSetting->EncryMode;
	u8 i, j;
	u8 DynKeyMode = pSetting->DynKeyMode;
	u8 keyLength;

    MP_FUNCTION_ENTER();
	mpDebugPrint("%s: EncryOnOff=%d,m=%d,d=%d", __func__,pSetting->EncryOnOff,
			encryMode,DynKeyMode);
    if ((pSetting->EncryOnOff == 0) || (encryMode == 0) || (DynKeyMode != 0)){
		HW_CAM_Write(&dot11Obj, DEFAULT_ENCRY_TYPE, NO_WEP);
		MP_FUNCTION_EXIT();
		return;
	}	

	if (pSetting->OperationMode != CAM_AP_VAP){
		MP_TRACE_LINE();
		HW_CAM_ResetRollTbl(&dot11Obj); //force CAM to use default encry type
		
		switch(encryMode){
			case WEP64:
				ZD1211DEBUG(0, "WEP64 Mode\n");	
				keyLength = 5;
 				break;
		
			case WEP128:
				ZD1211DEBUG(0, "WEP128 Mode\n");	
				keyLength = 13;
				break;
		
			case WEP256:
				ZD1211DEBUG(0, "WEP256 Mode\n");	
				keyLength = 29;
				break;	
			
			default:
 				ZD1211DEBUG(0, "Not supported Mode\n");	
				ZD1211DEBUG(0, "encryMode = %d\n", encryMode);

				return;		
		}
		MP_TRACE_LINE();
	
        EncType = HW_CAM_Read(&dot11Obj, DEFAULT_ENCRY_TYPE);
        HW_CAM_Write(&dot11Obj, DEFAULT_ENCRY_TYPE, EncType|(encryMode << (macp->cardSetting.EncryKeyId * 4)));

 
		for (i=0, j=0; i<4; i++, j+=8){ //one key occupy 32 bytes space
			HW_ConfigStatKey(&dot11Obj, &pSetting->keyVector[i][0], keyLength, STA_KEY_START_ADDR+j);
		}
	}
	MP_FUNCTION_EXIT();
	return;
}

static int
zd1205_ioctl_getessid(struct net_device *dev, struct iw_point *erq)
{
	struct zd1205_private *macp = dev->priv;
	char essidbuf[IW_ESSID_MAX_SIZE+1];
	u8 len;

	zd1205_lock(macp);

	if (macp->bAssoc){
		len = dot11Obj.CurrSsid[1];
		memcpy(essidbuf, &dot11Obj.CurrSsid[2], len);
	}
	else {    
		len = macp->cardSetting.Info_SSID[1];    
		memcpy(essidbuf, &macp->cardSetting.Info_SSID[2], len);
	}

	essidbuf[len] = 0;
	zd1205_unlock(macp);

 	erq->flags = 1;
 	erq->length = strlen(essidbuf);

	WPADEBUG("zd1205_ioctl_getessid: %s\n", essidbuf);

	//erq->length = strlen(essidbuf) + 1;
	//zd1205_dump_data("essidbuf", (u8 *)essidbuf, erq->length);

	if (erq->pointer)
		if ( copy_to_user(erq->pointer, essidbuf, erq->length) )
			return -EFAULT;
	return 0;
}

static int
zd1205_ioctl_setfreq(struct net_device *dev, struct iw_freq *frq)
{
	struct zd1205_private *macp = dev->priv;
	int chan = -1;
	int fflag=0; //Found Flag

	if (macp->cardSetting.BssType == INFRASTRUCTURE_BSS)
		return -EINVAL;
	
	if ( (frq->e == 0) && (frq->m <= 1000) ) {
		/* Setting by channel number */
		chan = frq->m;
		fflag=1;
	} else {
		/* Setting by frequency - search the table */
		int mult = 1;
		int i;
 
		for (i = 0; i < (6 - frq->e); i++)
			mult *= 10;

		if(PURE_A_MODE != mMacMode ) {
			for (i = 0; i < NUM_CHANNELS; i++)
				if (frq->m == (channel_frequency[i] * mult)) {
					chan = i+1;
					fflag=1;
					break;
				}
		}
#if ZDCONF_80211A_SUPPORT == 1
		else {
			for (i = 0; i < NUM_CHANNELS_11A; i++)
            	if (frq->m == (channel_frequency_11A[i*2+1] * mult)) {
                	chan = channel_frequency_11A[i*2];
					fflag=1;
					break;
				}
		}
#endif

	}

	if(PURE_A_MODE != mMacMode) {
		if ( (chan < 1) || (14 < chan) ) {
			printk("We Can't Found Required Channel in ioctl_setfreq(2.4G)\n");
			return -EINVAL;
		}
	}
#if ZDCONF_80211A_SUPPORT == 1
	else {
            if ( (chan < 1) || (0 == fflag) ) {
				printk("We Can't Found Required Channel in ioctl_setfreq(5G)\n");
            	return -EINVAL;
			}
			if( 0 == channel_11A_to_Freq(chan) ) {
				printk("The channel isn't exist(%d)\n",chan);	
				return -EINVAL;
			}

	}
#endif

 	zd1205_lock(macp);
	macp->cardSetting.Channel = chan;
#if ZDCONF_ADHOC_SUPPORT == 1
    macp->IBSS_DesiredChannel = chan;
#endif
	zd1205_unlock(macp);

	return 0;  
}

static int
zd1205_ioctl_setrts(struct net_device *dev, struct iw_param *rrq)
{
	struct zd1205_private *macp = dev->priv;
	int val = rrq->value;

	if (rrq->disabled)
		val = 2347;

	if ( (val < 0) || (val > 2347) )
		return -EINVAL;

	zd1205_lock(macp);

	macp->cardSetting.RTSThreshold = val;
    if (rrq->disabled)
        macp->cardSetting.RTSThreshold = 9999;

	zd1205_unlock(macp);

	return 0;

}

static int
zd1205_ioctl_setfrag(struct net_device *dev, struct iw_param *frq)
{
	struct zd1205_private *macp = dev->priv;

	int err = 0;
 
	zd1205_lock(macp);

    if (frq->disabled)
    {
        macp->cardSetting.FragThreshold = 9999;
    }
    else
    {
#if ZDCONF_LP_SUPPORT == 1
        if(dot11Obj.LP_MODE)
        {
            printk("You can't turn on fragment when lp_mode is on\n");
            printk("issue iwpriv ethX lp_mode 0 to turn it off\n");
            err = -EINVAL; 
        }
        else 
#endif
        {
            if ( (frq->value < 256) || (frq->value > 2346) )
            {
                err = -EINVAL;
            }
            else
            {
                /* must be even */
                macp->cardSetting.FragThreshold= frq->value & ~0x1;
            }
        }
    }

    zd1205_unlock(macp);
    return err;
}

    static int
zd1205_ioctl_getfrag(struct net_device *dev, struct iw_param *frq)
{
	struct zd1205_private *macp = dev->priv;

	u16 val;

	zd1205_lock(macp);
	val = macp->cardSetting.FragThreshold;
	frq->value = val;
	frq->disabled = (val >= 2346);
	frq->fixed = 1;
	zd1205_unlock(macp);

	return 0;
}

/* For WIRELESS_EXT > 12 */
static int zd1205wext_giwfreq(struct net_device *dev, struct iw_request_info *info, struct iw_freq *freq, char *extra)
{
#ifdef LINUX
	struct zd1205_private *macp;
   if(!netif_running(dev))
        return -EINVAL;

	macp = dev->priv;
	freq->m = zd1205_hw_get_freq(macp);
	freq->e = 1;
#endif
	return 0;
}

static int zd1205_ioctl_setmode(struct net_device *dev, __u32 *mode)
{
	struct zd1205_private *macp = dev->priv;
    static unsigned long setmodeLock = 0;
    int ScanWaitCnt = 0;
    
	//zd1205_lock(macp);
#if Make_ADHOC
    if(test_and_set_bit(0, &setmodeLock))
    {
        printk("change mode at the same time\n");
        return 0;
    }
	switch(*mode) {
#if ZDCONF_ADHOC_SUPPORT == 1
		case IW_MODE_ADHOC:
			ZD1211DEBUG(0, "Switch to Ad-Hoc mode\n");
			macp->cardSetting.BssType = INDEPENDENT_BSS;

                        if (macp->bDefaultIbssMacMode==0)
                //macp->cardSetting.MacMode=PURE_B_MODE;
			    macp->cardSetting.MacMode=MIXED_MODE;
			zd_writel(STA_RX_FILTER, Rx_Filter);
			break;
#endif

		case IW_MODE_INFRA:
#if (ZDCONF_WEXT_WPA == 1) && (WIRELESS_EXT > 17)
            // wpa_supplicant wext driver will set mode to INFRA each time it
            // try to connect to a new AP. This will set AuthMode to OPEN. So
            // , we will get problem when SharedKey used.
            if(mBssType != INFRASTRUCTURE_BSS)
#endif
            {
			ZD1211DEBUG(0, "Switch to Infra mode\n");
 			macp->cardSetting.BssType = INFRASTRUCTURE_BSS;
			macp->cardSetting.AuthMode = 0;

                        if (macp->bDefaultIbssMacMode==0)
                        {
                            macp->cardSetting.MacMode=MIXED_MODE;
                        }
			
			zd_writel(STA_RX_FILTER, Rx_Filter);
            }
			break;

			
		case IW_MODE_MASTER:
#if ZDCONF_AP_SUPPORT == 1
			ZD1211DEBUG(0, "Switch to AP mode\n");
            while(dot11Obj.bChScanning)
            {
                dot11Obj.DelayUs(1000*1000);
                if(ScanWaitCnt++ > 10)
                {
                    printk("Change Mode wait too long\n");
                    break;
                }
            }
			if(ScanWaitCnt > 0)
				dot11Obj.DelayUs(1000*1000);
			macp->cardSetting.BssType = AP_BSS;

			// Set bssid = MacAddress 

 			macp->BSSID[0] = macp->macAdr[0];
 			macp->BSSID[1] = macp->macAdr[1];
			macp->BSSID[2] = macp->macAdr[2];

			macp->BSSID[3] = macp->macAdr[3];
 			macp->BSSID[4] = macp->macAdr[4];
			macp->BSSID[5] = macp->macAdr[5];

			zd_writel(cpu_to_le32(*(u32 *)&macp->macAdr[0]), BSSID_P1);
			zd_writel(cpu_to_le32(*(u32 *)&macp->macAdr[4]), BSSID_P2);
			macp->cardSetting.AuthMode = 2; 	//auto auth
			zd_writel(AP_RX_FILTER, Rx_Filter);
			netif_start_queue(dev);
#else
            clear_bit(0, &setmodeLock); 
            return -EINVAL;
#endif
			break;

		default:
			printk("Switch to PSEUDO_IBSS mode\n");
			macp->cardSetting.BssType = PSEUDO_IBSS;
			zd_writel(STA_RX_FILTER, Rx_Filter);
			break;

	}

	macp->bAssoc = 0;
	if (macp->usb->speed != USB_SPEED_HIGH)
		macp->cardSetting.MacMode = PURE_B_MODE;
	else {
#if 0        
        if (macp->cardSetting.BssType == INDEPENDENT_BSS)
            macp->cardSetting.MacMode = PURE_B_MODE;
        else
	        macp->cardSetting.MacMode = MIXED_MODE;
#endif                             
	}
#endif	//Make_ADHOC

#ifdef LINUX
	zd1205_SetRatesInfo(macp);    
#endif

	//zd1205_unlock(macp);
#if LINUX	
    clear_bit(0, &setmodeLock); 
#endif
	return 0;
}	

static int
zd1205_ioctl_getretry(struct net_device *dev, struct iw_param *prq)
{
	return 0;  
}             

static int zd1205wext_siwmode(struct net_device *dev, struct iw_request_info *info, __u32 *mode, char *extra)
{
	int err;
	err = zd1205_ioctl_setmode(dev, mode);
	return err;
}

static int zd1205wext_giwmode(struct net_device *dev, struct iw_request_info *info, __u32 *mode, char *extra)
{
	struct zd1205_private *macp = dev->priv;
	u8 BssType = macp->cardSetting.BssType;

   if(!netif_running(dev))
        return -EINVAL;

	zd1205_lock(macp);

	switch(BssType){
		case AP_BSS:
			*mode = IW_MODE_MASTER;
    			break;

	        case INFRASTRUCTURE_BSS:
    			*mode = IW_MODE_INFRA;
    			break;	
#if ZDCONF_ADHOC_SUPPORT == 1    		
		case INDEPENDENT_BSS:
    			*mode = IW_MODE_ADHOC;
    			break;	
#endif

		default:
			*mode = IW_MODE_ADHOC;
			break;		
	}	

	zd1205_unlock(macp);
	return 0;
}

static int zd1205wext_giwrate(struct net_device *dev, struct iw_request_info *info, struct iw_param *rrq, char *extra)
{
   if(!netif_running(dev))
        return -EINVAL;

 	return zd1205_ioctl_getrate(dev, rrq);
}

static int zd1205wext_giwrts(struct net_device *dev, struct iw_request_info *info, struct iw_param *rts, char *extra)
{
	struct zd1205_private *macp;
	macp = dev->priv;
                
   if(!netif_running(dev))
        return -EINVAL;

	rts->value = macp->cardSetting.RTSThreshold;
	rts->disabled = (rts->value == 2347);
	rts->fixed = 1;

	return 0;
}

static int zd1205wext_giwfrag(struct net_device *dev, struct iw_request_info *info, struct iw_param *frag, char *extra)
{
   if(!netif_running(dev))
        return -EINVAL;

	return zd1205_ioctl_getfrag(dev, frag);
}
#if 0
static int zd1205wext_giwtxpow(struct net_device *dev, struct iw_request_info *info, struct iw_param *rrq, char *extra)
{
   if(!netif_running(dev))
        return -EINVAL;

	return zd1205_ioctl_gettxpower(dev, rrq);
}

static int zd1205wext_siwtxpow(struct net_device *dev, struct iw_request_info *info, struct iw_param *rrq, char *extra)
{
	return zd1205_ioctl_settxpower(dev, rrq);
}
#endif
static int zd1205wext_giwrange(struct net_device *dev, struct iw_request_info *info, struct iw_point *data, char *extra)
{
	struct iw_range *range = (struct iw_range *) extra;
	int i, val;
#if !((ZDCONF_WEXT_WPA == 1) && (WIRELESS_EXT > 17))
    // This function is needed for WPA support judgement for NetworkManager. 
    // Even in down state.
   if(!netif_running(dev))
        return -EINVAL;
#endif

                
#if WIRELESS_EXT > 9
	range->txpower_capa = IW_TXPOW_DBM;
	// XXX what about min/max_pmp, min/max_pmt, etc.
#endif
                                
#if WIRELESS_EXT > 10
	range->we_version_compiled = WIRELESS_EXT;
 	range->we_version_source = 13;
 	range->retry_capa = IW_RETRY_LIMIT;
	range->retry_flags = IW_RETRY_LIMIT;
#if (ZDCONF_WEXT_WPA == 1) && (WIRELESS_EXT > 17)
    range->enc_capa = IW_ENC_CAPA_WPA | IW_ENC_CAPA_WPA2 |
                      IW_ENC_CAPA_CIPHER_TKIP | IW_ENC_CAPA_CIPHER_CCMP;
#endif
	range->min_retry = 0;
	range->max_retry = 255;

#endif /* WIRELESS_EXT > 10 */

                                                                                

    /* XXX need to filter against the regulatory domain &| active set */
	val = 0;
	if(PURE_A_MODE != mMacMode ) {
		for (i = 0; i < NUM_CHANNELS ; i++) {
			range->freq[val].i = i + 1;
			range->freq[val].m = channel_frequency[i] * 100000;
			range->freq[val].e = 1;
			val++;
		}
	}
#if ZDCONF_80211A_SUPPORT == 1
	else if(PURE_A_MODE == mMacMode) {
               for (i = 0; i < NUM_CHANNELS_11A && i < 32; i++) {
                        range->freq[val].i = channel_frequency_11A[i*2];;
                        range->freq[val].m = channel_frequency_11A[i*2+1] * 100000;
                        range->freq[val].e = 1;
                        val++;
			//For 802.11a, there are too more frequency. We can't return them all
                }

	}
#endif


	range->num_frequency = val;
	
	/* Max of /proc/net/wireless */
	range->max_qual.qual = 100;
	range->max_qual.level = 100;

	range->max_qual.noise = -96;
	range->sensitivity = 3;

	// XXX these need to be nsd-specific!
	range->min_rts = 256;
	range->max_rts = 2346;

	range->min_frag = 256;
    range->max_frag = 2346;
	range->max_encoding_tokens = NUM_WEPKEYS;
	range->num_encoding_sizes = 2;
	range->encoding_size[0] = 5;
	range->encoding_size[1] = 13;
                                        
	// XXX what about num_bitrates/throughput?
	range->num_bitrates = 0;
                                                        
	/* estimated max throughput */
	// XXX need to cap it if we're running at ~2Mbps..
	range->throughput = 5500000;
	
	return 0;
}


static int
zd1205_ioctl_setrate(struct net_device *dev, struct iw_param *frq)
{

    struct zd1205_private *macp=(struct zd1205_private *)g_dev->priv;
    if(frq->value == 0)
        macp->bFixedRate = 0;
    else
    {
        switch(frq->value/1000000)
        {
            case 1:
                macp->cardSetting.FixedRate = RATE_1M;
                break;
            case 2:
                macp->cardSetting.FixedRate = RATE_2M;
                break;

            case 5:
                macp->cardSetting.FixedRate = RATE_5M;
                break;
            case 11:
                macp->cardSetting.FixedRate = RATE_11M;
                break;
            case 6:
                macp->cardSetting.FixedRate = RATE_6M;
                break;
            case 9:
                macp->cardSetting.FixedRate = RATE_9M;
                break;
            case 12:
                macp->cardSetting.FixedRate = RATE_12M;
                break;
            case 18:
                macp->cardSetting.FixedRate = RATE_18M;
                break;
            case 24:
                macp->cardSetting.FixedRate = RATE_24M;
                break;
            case 36:
                macp->cardSetting.FixedRate = RATE_36M;
                break;
            case 48:
                macp->cardSetting.FixedRate = RATE_48M;
                break;
            case 54:
                macp->cardSetting.FixedRate = RATE_54M;
                break;
            default:
                printk("Rate = %d\n", frq->value);
                return -EINVAL;
        }
        macp->bFixedRate=1;
    }
    return 0;
}
    
static int
zd1205_ioctl_getrate(struct net_device *dev, struct iw_param *frq)
{
 	struct zd1205_private *macp = dev->priv;
        

	frq->fixed = 0;
	frq->disabled = 0;
	frq->value = 0;
    if(!macp->bFixedRate) return 0;
	switch(macp->cardSetting.FixedRate)
	{
		case RATE_1M:
			frq->value = 1000000;
			break;
                
		case RATE_2M:

			frq->value = 2000000;
			break;
              
		case RATE_5M:
			frq->value = 5500000;
			break;

		case RATE_11M:
			frq->value = 11000000;
			break;

		case RATE_6M:
			frq->value = 6000000;
			break;

		case RATE_9M:
			frq->value = 9000000;
			break;

		case RATE_12M:
			frq->value = 12000000;
			break;

		case RATE_18M:
			frq->value = 18000000;
			break;

		case RATE_24M:
			frq->value = 24000000;
			break;

		case RATE_36M:
			frq->value = 36000000;
			break;

		case RATE_48M:
			frq->value = 48000000;
			break;

		case RATE_54M:
			frq->value = 54000000;
			break;        

		default:
		    return -EINVAL;
	}
                                                                                                                                                                                                               
	return 0;
}


void zd1205_init_card_setting(struct zd1205_private *macp)
{
	card_Setting_t *pSetting = &macp->cardSetting;
    extern int zd1211_OperationMode;
    int i;


	pSetting->BssType = zd1211_OperationMode;
	//pSetting->BssType = AP_BSS;
	//pSetting->BssType = INDEPENDENT_BSS;
	//pSetting->BssType = PSEUDO_IBSS;
	pSetting->HiddenSSID = 0; 	//disable hidden essid
 	pSetting->LimitedUser = 32;
	pSetting->RadioOn = 1;

	pSetting->BlockBSS = 0;
	pSetting->EncryOnOff = 0;
	//pSetting->PreambleType = 0; //long preamble
	pSetting->PreambleType = 1; //short preamble
	pSetting->Channel = 6;
	pSetting->EncryMode = NO_WEP;
	pSetting->EncryKeyId = 0;
	pSetting->TxPowerLevel = 0;
#if ZDCONF_AP_SUPPORT == 1
	if (pSetting->BssType == AP_BSS) {
		pSetting->AuthMode = 2; 	//auto auth
		pSetting->Info_SSID[0] = 0;
		pSetting->Info_SSID[1] = 0x08;
		pSetting->Info_SSID[2] = 'Z';
		pSetting->Info_SSID[3] = 'D';
		pSetting->Info_SSID[4] = '1';
		pSetting->Info_SSID[5] = '2';
		pSetting->Info_SSID[6] = '1';
		pSetting->Info_SSID[7] = '1';
		pSetting->Info_SSID[8] = 'A';
		pSetting->Info_SSID[9] = 'P';
	} 
#endif
	if (pSetting->BssType == INFRASTRUCTURE_BSS) {
		pSetting->AuthMode = 0; 	//open syatem

		pSetting->Info_SSID[0] = 0;
		//pSetting->Info_SSID[1] = 0x05;
		pSetting->Info_SSID[1] = 0x00;
		pSetting->Info_SSID[2] = 'G';
		pSetting->Info_SSID[3] = '1';
		pSetting->Info_SSID[4] = '0';
		pSetting->Info_SSID[5] = '0';
		pSetting->Info_SSID[6] = '0';
		//pSetting->Info_SSID[7] = 'A';
		//pSetting->Info_SSID[8] = 'B';
	}
#if ZDCONF_ADHOC_SUPPORT == 1
	if (pSetting->BssType == INDEPENDENT_BSS) {
		pSetting->AuthMode = 0; 	//open syatem
		pSetting->Info_SSID[0] = 0;
		pSetting->Info_SSID[1] = 0x09;
		pSetting->Info_SSID[2] = '1';
		pSetting->Info_SSID[3] = '2';

		pSetting->Info_SSID[4] = '1';
		pSetting->Info_SSID[5] = '1';
		pSetting->Info_SSID[6] = 'A';
		pSetting->Info_SSID[7] = 'd';
		pSetting->Info_SSID[8] = 'H';
		pSetting->Info_SSID[9] = 'o';
		pSetting->Info_SSID[10] = 'c';
	}
#endif

#if	!(defined(GCCK) && defined(OFDM)) 
	pSetting->Info_SupportedRates[0] = 0x01;
	pSetting->Info_SupportedRates[1] = 0x05;
	pSetting->Info_SupportedRates[2] = 0x82;
	pSetting->Info_SupportedRates[3] = 0x84;
  	pSetting->Info_SupportedRates[4] = 0x8B;
	pSetting->Info_SupportedRates[5] = 0x96;
    pSetting->Info_SupportedRates[6] = 0x21;

 

	MP_ASSERT(0);
	if ((dot11Obj.rfMode == AL2210MPVB_RF) || (dot11Obj.rfMode == AL2210_RF)){
		pSetting->Rate275 = 1;
		pSetting->Info_SupportedRates[7] = 0x2C;//22
		pSetting->Info_SupportedRates[8] = 0x37;//27.5
		pSetting->Info_SupportedRates[1] = 0x07;
	}
	else
    	pSetting->Rate275 = 0;    
#else
    if (macp->usb->speed != USB_SPEED_HIGH)
        pSetting->MacMode = PURE_B_MODE;
    else {

        //if (pSetting->BssType == INDEPENDENT_BSS)
           //pSetting->MacMode = PURE_B_MODE;
        //else   
	        pSetting->MacMode = MIXED_MODE;
    }    
	zd1205_SetRatesInfo(macp);
	//pCardSetting->UartEnable = 1;	
	//pCardSetting->BaudRate = BAUD_RATE_115200;


#endif		

 
	pSetting->FragThreshold = 9999;
	pSetting->RTSThreshold = 9999;

	pSetting->BeaconInterval = 100;
	pSetting->DtimPeriod = 1;
    pSetting->SwCipher = 0;


	pSetting->DynKeyMode = 0;
	pSetting->WpaBcKeyLen = 32; // Tmp key(16) + Tx Mic key(8) + Rx Mic key(8)


#if ZDCONF_WPS_SUPPORT == 1 && ZDCONF_AP_SUPPORT == 1
    for(i=0;i<5;i++) 
    {
        memset(pSetting->appie[i],0,IEEE80211_MAX_WSC_IE);
        pSetting->appieLen[i]=0;
    }
#endif

	//dot11Obj.MicFailure = NULL;
	//dot11Obj.AssocRequest = NULL;
	//dot11Obj.WpaIe =  NULL;
}

void zd1205_load_card_setting(struct zd1205_private *macp, u8 bInit)
{
#if 0
	int ifp;
	int bcount = 0;
	mm_segment_t fs;
	unsigned int file_length;
	u8 *buffer, *old_buffer;
	int i, parse_id, count = 0;
	char *token;
	card_Setting_t *pSetting = &macp->cardSetting;
	u8 ssidLen;
	u16 frag;

	//struct stat file_info;

	// Open the code file
	// for file opening temporarily tell the kernel I am not a user for
	// memory management segment access

	fs = get_fs();
	set_fs(KERNEL_DS);

	// open the file with the firmware for uploading
	if (ifp = open(config_filename, O_RDONLY, 0 ), ifp < 0){
		// error opening the file
		ZD1211DEBUG(0, "File opening did not success\n");
		set_fs(fs);
		return;
	}

	/* Get information about the file. */
	//fstat (ifp, &file_info);
	//sys_fstat(ifp, &file_info);
	//file_length = file_info.st_size;
    
	file_length = 512;
	buffer = kmalloc(file_length, GFP_ATOMIC);
	old_buffer = buffer;

	/* Read the file into the buffer. */
	bcount = read(ifp, buffer, file_length);
	ZD1211DEBUG(1, "bcount=%d\n", bcount);

	// close the file
	close(ifp);

	// switch back the segment setting
	set_fs(fs);

	parse_id = 0;
	while ((token=strsep((char **)&buffer, "=\n"))){
		count++;

		if (count % 2){
			if (!strcmp(token, "mode"))
				parse_id = 1;
			else if (!strcmp(token, "essid"))
				parse_id = 2;
			else if (!strcmp(token, "channel"))
				parse_id = 3;
			else if (!strcmp(token, "rts"))
				parse_id = 4;
			else if (!strcmp(token, "frag"))
				parse_id = 5;
			else
				parse_id = 0;
		}
		else {
			switch (parse_id){
			case 1:
				if (!strcmp(token, "Managed"))
					pSetting->BssType = INFRASTRUCTURE_BSS;
				else if (!strcmp(token, "Ad-Hoc"))
					pSetting->BssType = INDEPENDENT_BSS;
				else if (!strcmp(token, "Master"))       
					pSetting->BssType = AP_BSS;
				break;

			case 2:
				pSetting->Info_SSID[0] = 0;
				ssidLen = strnlen(token, 32);
				pSetting->Info_SSID[1] = ssidLen;

				for (i=0; i<ssidLen; i++)
					pSetting->Info_SSID[2+i] = token[i];
				break;

			case 3:
				pSetting->Channel = (u8)simple_strtoul(token, &token, 0);
				break;

			case 4:
				pSetting->RTSThreshold = (u16)simple_strtoul(token, &token, 0);
				break;

			case 5:
				frag = (u16)simple_strtoul(token, &token, 0);

				if (frag < 256)
					frag = 256;
				pSetting->FragThreshold = frag;
				break;

			default:
				break;                                                                                                                            
			}
        	}

		if (count > 9)
			break;
	}

	kfree(old_buffer);

	if (!bInit)
		zd_UpdateCardSetting(pSetting);
        
	//zd1205_show_card_setting(macp);
	return;
#endif
}


void zd1205_save_card_setting(struct zd1205_private *macp)
{
#if 0
	int ifp;
	int bcount = 0;
	mm_segment_t fs;
	unsigned int file_length;
	u8 *buffer, *old_buffer;
	u8 ssidLen;
	char ssid[33];
	int write_byte = 0;
	card_Setting_t *pSetting = &macp->cardSetting;

	//struct stat file_info;

	// Open the code file
	// for file opening temporarily tell the kernel I am not a user for
	// memory management segment access

	fs = get_fs();
	set_fs(KERNEL_DS);

	// open the file with the firmware for uploading
	if (ifp = open(config_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666 ), ifp < 0){
		// error opening the file
		ZD1211DEBUG(0, "File opening did not success\n");
		set_fs(fs);
		return;
	}

	/* Get information about the file. */
	//fstat (ifp, &file_info);
	//sys_fstat(ifp, &file_info);
	//file_length = file_info.st_size;

	file_length = 512;

	buffer = kmalloc(file_length, GFP_ATOMIC);
	old_buffer = buffer;

	ssidLen = pSetting->Info_SSID[1];
	memcpy(ssid, &pSetting->Info_SSID[2], ssidLen);
	ssid[ssidLen] = '\0';

	if (pSetting->BssType == INFRASTRUCTURE_BSS)
		bcount = snprintf(buffer, file_length, "mode=Managed\n");
	else if (pSetting->BssType == INDEPENDENT_BSS)
		bcount = snprintf(buffer, file_length, "mode=Ad-Hoc\n");
	else if (pSetting->BssType == AP_BSS)
		bcount = snprintf(buffer, file_length, "mode=Master\n");
	
	ZD1211DEBUG(1, "mode bcount=%d\n", bcount);         
	write_byte = bcount;
	buffer += bcount;
    
	bcount = snprintf(buffer, file_length, "essid=%s\n", ssid);
	ZD1211DEBUG(1, "essid bcount=%d\n", bcount);
	write_byte += bcount;
	buffer += bcount;

	bcount = snprintf(buffer, file_length, "channel=%d\n", pSetting->Channel);
	ZD1211DEBUG(1, "channel bcount=%d\n", bcount);
	write_byte += bcount;
	buffer += bcount;
    
	bcount = snprintf(buffer, file_length, "rts=%d\n", pSetting->RTSThreshold);
	ZD1211DEBUG(1, "rts bcount=%d\n", bcount);
	write_byte += bcount;
	buffer += bcount;
    
	bcount = snprintf(buffer, file_length, "frag=%d\n", pSetting->FragThreshold);
	ZD1211DEBUG(1, "frag bcount=%d\n", bcount);
	write_byte += bcount;
    
	/* Write the file into the buffer. */
	ZD1211DEBUG(1, "write_byte=%d\n", write_byte);
	bcount = write(ifp, old_buffer, write_byte);
	ZD1211DEBUG(1, "bcount=%d\n", bcount);

	// close the file
	close(ifp);

	// switch back the segment setting
	set_fs(fs);

	kfree(old_buffer);
	return;
#endif
}                    	



#ifndef HOST_IF_USB	
int
zd1205_found1(struct pci_dev *pcid, const struct pci_device_id *ent)
{
	static int first_time = true;
  	struct net_device *dev = NULL;
	struct zd1205_private *macp = NULL;

	int rc = 0;
	

	ZENTER(0);

	dev = alloc_etherdev(sizeof (struct zd1205_private));


	if (dev == NULL) {
		printk(KERN_ERR "zd1205: Not able to alloc etherdev struct\n");
		rc = -ENODEV;
		goto out;

	}



 	g_dev = dev;  //save this for CBs use
	SET_MODULE_OWNER(dev);

	if (first_time) {
		first_time = false;

        printk(KERN_NOTICE "%s - version %s\n",
	    	zd1205_full_driver_name, zd1205_driver_version);

		printk(KERN_NOTICE "%s\n", zd1205_copyright);
		printk(KERN_NOTICE "\n");
	}

	macp = dev->priv;
	macp->pdev = pcid;
	macp->device = dev;

 	pci_set_drvdata(pcid, dev);
	macp->numTcb = NUM_TCB;

	macp->numTbd = NUM_TBD;
	macp->numRfd = NUM_RFD;

	macp->numTbdPerTcb = NUM_TBD_PER_TCB;
	macp->regp = 0;
    macp->rxOffset  = ZD_RX_OFFSET;
    macp->rfd_size = 24; // form CbStatus to NextCbPhyAddrHighPart


	init_timer(&macp->watchdog_timer);


    macp->watchdog_timer.data = (unsigned long) dev;
	macp->watchdog_timer.function = (void *) &zd1205_watchdog_cb;
    init_timer(&macp->tm_hking_id);
    macp->tm_hking_id.data = (unsigned long) dev;

	macp->tm_hking_id.function = (void *) &HKeepingCB;

    init_timer(&macp->tm_mgt_id);
    macp->tm_mgt_id.data = (unsigned long) dev;
	macp->tm_mgt_id.function = (void *) &zd1205_mgt_mon_cb;


	if ((rc = zd1205_pci_setup(pcid, macp)) != 0) {
		goto err_dev;
	}



	if (!zd1205_init(macp)) {
		printk(KERN_ERR "zd1025: Failed to initialize, instance \n");

		rc = -ENODEV;

		goto err_pci;
	}


	dev->irq = pcid->irq;
	dev->open = &zd1205_open;
	dev->hard_start_xmit = &zd1205_xmit_frame;

	dev->stop = &zd1205_close;
	dev->change_mtu = &zd1205_change_mtu;
 	dev->get_stats = &zd1205_get_stats;
 	dev->set_multicast_list = &zd1205_set_multi;

	dev->set_mac_address = &zd1205_set_mac;

	dev->do_ioctl = &zd1205_ioctl;
    dev->features |= NETIF_F_SG | NETIF_F_HW_CSUM;

	if ((rc = register_netdev(dev)) != 0) {
		goto err_pci;
	}

	


    memcpy(macp->ifname, dev->name, IFNAMSIZ);
    macp->ifname[IFNAMSIZ-1] = 0;	



    if (netif_carrier_ok(macp->device))
		macp->cable_status = "Cable OK";
	else
		macp->cable_status = "Not Available";

    if (zd1205_create_proc_subdir(macp) < 0) {
		printk(KERN_ERR "zd1205: Failed to create proc dir for %s\n",
	       macp->device->name);

	}    


	printk(KERN_NOTICE "\n");
	goto out;

err_pci:

	iounmap(macp->regp);
	pci_release_regions(pcid);
	pci_disable_device(pcid);
	

err_dev:

	pci_set_drvdata(pcid, NULL);


	kfree(dev);

out:
    ZEXIT(0);
	return rc;
}
#endif



/**
 * zd1205_clear_structs - free resources
  * @dev: adapter's net_device struct

 *
 * Free all device specific structs, unmap i/o address, etc.
 */

void 

zd1205_clear_structs(struct net_device *dev)
{
#ifndef HOST_IF_USB
	struct zd1205_private *macp = dev->priv;
#endif

// 	zd1205_sw_release();
#ifndef HOST_IF_USB	 	
	iounmap(macp->regp);
	pci_release_regions(macp->pdev);
 	pci_disable_device(macp->pdev);
	pci_set_drvdata(macp->pdev, NULL);
#endif

	//kfree(dev);
	free_netdev(dev); //kernel 2,6
}

#ifndef HOST_IF_USB	
void 
zd1205_remove1(struct pci_dev *pcid)
{
 	struct net_device *dev;
 	struct zd1205_private *macp;


	ZENTER(0);	

	if (!(dev = (struct net_device *) pci_get_drvdata(pcid)))

		return;

	macp = dev->priv;
	unregister_netdev(dev);
    zd1205_remove_proc_subdir(macp);
	zd1205_clear_structs(dev);


    ZEXIT(0);
}
#endif


#if 0  //move codes to zdpci_hotplug.c
static struct pci_device_id zd1205_id_table[] __devinitdata =
{


	{0x167b, 0x2102, PCI_ANY_ID, PCI_ANY_ID, 0, 0, ZD_1202},
	{0x167b, 0x2100, PCI_ANY_ID, PCI_ANY_ID, 0, 0, ZD_1202},
    {0x167b, 0x2105, PCI_ANY_ID, PCI_ANY_ID, 0, 0, ZD_1205},

	{0,}			
};



MODULE_DEVICE_TABLE(pci, zd1205_id_table);



static struct pci_driver zd1205_driver = {
 	.name         = "zd1205",
	.id_table     = zd1205_id_table,
	.probe        = zd1205_found1,

	.remove       = __devexit_p(zd1205_remove1),
};


static int __init
zd1205_init_module(void)
{

	int ret;

    ret = pci_module_init(&zd1205_driver);
	return ret;
}

static void __exit
zd1205_cleanup_module(void)
{
	pci_unregister_driver(&zd1205_driver);
}

module_init(zd1205_init_module);

module_exit(zd1205_cleanup_module);
#endif

/*************************************************************************/
BOOLEAN zdcb_setup_next_send(fragInfo_t *frag_info_origin)
{
#ifdef LINUX
    int tbdidx;
    fragInfo_t *frag_info = frag_info_origin;
#if ZDCONF_LP_SUPPORT == 1
    struct lp_desc *lp_bucket = NULL;
	U32 xxx;
#endif
    U32 PrvFragLen = 0;
	struct zd1205_private *macp = g_dev->priv;
 	struct sk_buff *skb = (struct sk_buff *)frag_info->buf;
	U8 bIntraBss =  frag_info->bIntraBss;
	U8 MsgID = frag_info->msgID;
	U8 numOfFrag = frag_info->totalFrag;

	U16 aid = frag_info->aid;
	U8 hdrLen = frag_info->hdrLen;
	zd1205_SwTcb_t 	*sw_tcb;
 	zd1205_HwTCB_t	*hw_tcb;
 	zd1205_TBD_t	*pTbd;
	U8		*hdr, *pBody;
 	U32		bodyLen, length;
	U32 		tcb_tbd_num = 0;
	int 		i;
	U16 		pdu_size = 0;

 	void 		*addr;
	wla_Header_t	*wla_hdr;
 	U32		CurrFragLen;
 	U32		NextFragLen;

	skb_frag_t *frag = NULL;
	ctrl_Set_parm_t ctrl_setting_parms;
	U32 TotalLength = 0;


#ifndef HOST_IF_USB
	zd1205_SwTcb_t 	*next_sw_tcb;
	U32 		tmp_value, tmp_value3;
	unsigned long lock_flag;
	U32 loopCnt = 0;
#endif
#if ZDCONF_LP_SUPPORT == 1
    static U32 LP_Threshold_StartTime=0; //Threshold to trigger LP_MODE
    static U32 LP_Threshold_PKT_NUM=0; //Threshold to trigger LP_MODE
    static BOOLEAN LP_Over_Threshold = FALSE;

    U8 tmpBuf[64];

    U32 bodyLength = 0;
    U32 cfgLength;
    extern U32 LP_CNT_PUSH_SUCC;
    extern U32 LP_CNT_PUSH_FAIL;
    extern U32 LP_CNT_POP_SUCC;
    extern U32 LP_CNT_POP_FAIL;
    extern U32 LP_CNT_PERIOD_POP_SUCC;
    extern U32 LP_CNT_PERIOD_POP_FAIL;
    extern U32 LP_CNT_LAST_LATENCY;

    if(frag_info->msgID != 254 )
    {
        LP_Threshold_PKT_NUM++;
        if(LP_Threshold_PKT_NUM > 100)
        {
            LP_Threshold_PKT_NUM = 0;
            if(jiffies-LP_Threshold_StartTime<= HZ)
                LP_Over_Threshold = TRUE;
            else
                LP_Over_Threshold = FALSE;
            LP_Threshold_StartTime = jiffies;
        }  
    }
#endif
    
    memset(&ctrl_setting_parms,0, sizeof(ctrl_Set_parm_t)); 


#ifdef HOST_IF_USB
 
	if (!test_bit(ZD1211_RUNNING, &macp->flags))
		return FALSE;
#endif

	ZD1211DEBUG(2, "===== zdcb_setup_next_send enter =====\n");
	ZD1211DEBUG(2, "zd1211: bIntraBss = %x\n", bIntraBss);
	ZD1211DEBUG(2, "zd1211: numOfFrag = %x\n", numOfFrag);
	ZD1211DEBUG(2, "zd1211: skb = %x\n", (u32)skb);
	ZD1211DEBUG(2, "zd1211: aid = %x\n", aid);
    
#ifndef HOST_IF_USB
	spin_lock_irqsave(&macp->bd_non_tx_lock, lock_flag);
#endif    


	if ((frag_info->msgID != 254) && (skb) && (!bIntraBss)){   //data frame from upper layer
		if (skb_shinfo(skb)->nr_frags){   //got frag buffer
			frag = &skb_shinfo(skb)->frags[0];
			
			if (skb->len > macp->cardSetting.FragThreshold){  //need fragment
				pdu_size = macp->cardSetting.FragThreshold - 24 - 4; //mac header and crc32 length
				numOfFrag = (skb->len + (pdu_size-1) ) / pdu_size;

				if (numOfFrag == 0)
					numOfFrag = 1;
				ZD1211DEBUG(2, "zd1211: numOfFrag = %x\n", numOfFrag);
			}
		}
	}

	if (macp->freeTxQ->count -1 < numOfFrag){
		printk(KERN_ERR "zd1205: Not enough freeTxQ\n");
		//printk(KERN_ERR "zd1205: Cnt of freeTxQ = %x\n", macp->freeTxQ->count);
		//zd_EventNotify(EVENT_TX_COMPLETE, ZD_TX_CONFIRM, (U32)MsgID, aid);
#ifndef HOST_IF_USB        
		spin_unlock_irqrestore(&macp->bd_non_tx_lock, lock_flag);
#endif
		return FALSE;
	}

#if ZDCONF_LP_SUPPORT == 1
    if(dot11Obj.LP_MODE ) {
        if(frag_info->msgID == 254) {
		static int loopCnt = 0;
		//if(loopCnt++ % 3 !=0 ) return FALSE;
			if(macp->freeTxQ->count - 1 < 1) {
				printk(KERN_ERR "FreeTxQ[0] is empty to popPkt,P\n");
                return FALSE;
			}
    	    lp_bucket = popPkt(FALSE, 0, nowT());
            if(lp_bucket == NULL)  {
				LP_CNT_PERIOD_POP_FAIL++;
				return FALSE;
			}
			LP_CNT_PERIOD_POP_SUCC++;
            //printk("Got frame in Periodid poll\n");
    	    frag_info = &lp_bucket->pkt[0];
            frag_info->macHdr[0][0]=0x88;	//SLOW DOWN
			//if(lp_bucket->pktSize >= 1600) frag_info->qid = 0;
		}
		else if(
                (sstByAid[frag_info->aid]->Turbo_AMSDU) &&
               (LP_Over_Threshold) && //LP works when data arrival rate not low
               !(frag_info->EthHdr[0] & BIT_0) &&
               ((frag_info->macHdr[0][0] & 0x0C) == 0x08) && 
               ((((U16) frag_info->EthHdr[12]) << 8) + (frag_info->EthHdr[13]) != 0x888e) && 
               !(frag_info->macHdr[0][4] & BIT_0) && 
               (mBssType == INFRASTRUCTURE_BSS || sstByAid[frag_info->aid]->LP_CAP ) 
            ) {

			static U32 LastData = 0;
			int tmpDataTime;
			//tmpDataTime = LastData;
			//LastData = nowT();
            if(pushPkt(frag_info, sstByAid[frag_info->aid]->Turbo_AMSDU_LEN,nowT()) != 0) {
	 	    
				LP_CNT_PUSH_FAIL++;
				return FALSE;
	    	}
			LP_CNT_PUSH_SUCC++;
TX_LOOP:
            if(macp->freeTxQ->count - 1 < 1) {
                printk(KERN_ERR "FreeTxQ[0] is empty to popPkt,N\n");
                return TRUE;
            }
		    lp_bucket = popPkt(FALSE, sstByAid[frag_info->aid]->Turbo_AMSDU_LEN,nowT());
		    
		    if(lp_bucket == NULL) {
				//printk("popPkt NULL\n");
				LP_CNT_POP_FAIL++;
				return TRUE;
			}
			LP_CNT_POP_SUCC++;
	    	frag_info = &lp_bucket->pkt[0];
			memset(&ctrl_setting_parms,0, sizeof(ctrl_Set_parm_t));
			//if(nowT() - tmpDataTime > 500) 
			//	ctrl_setting_parms.DurationLen = 500;	
		    //printk("Got a Data Frm from lp_bucket\n");
		    frag_info->macHdr[0][0]=0x88; //SLOW DOWN
			//if(lp_bucket->pktSize >= 1600) frag_info->qid = 0;
		}
		skb = (struct sk_buff *)frag_info->buf;
    	bIntraBss =  frag_info->bIntraBss;
		MsgID = frag_info->msgID;
    	numOfFrag = frag_info->totalFrag;
		aid = frag_info->aid;
    	hdrLen = frag_info->hdrLen;
		//qid = frag_info->qid;
    }
    if(frag_info->macHdr[0][0] == 0x88)
    {
        if(mBssType == INFRASTRUCTURE_BSS)
            memcpy(frag_info->macHdr[0]+4+12, frag_info->macHdr[0]+4,6);
        else if(mBssType == AP_BSS)
            memcpy(frag_info->macHdr[0]+4+12, frag_info->macHdr[0]+4+6,6);
        else
            printk("ADHOC or WDS don't support A-MSDU now\n");
    }
#endif

	ctrl_setting_parms.Rate = frag_info->rate;
	ctrl_setting_parms.Preamble = frag_info->preamble;
	ctrl_setting_parms.encryType = frag_info->encryType;
	//ctrl_setting_parms.vapId = frag_info->vapId;

	for (i=0; i<numOfFrag; i++){
		ZD1211DEBUG(2, "zd1211: Cnt of freeTxQ = %x\n", macp->freeTxQ->count);
		ZD1211DEBUG(2, "zd1211: Frag Num = %x\n", i);

		sw_tcb = zd1205_first_txq(macp, macp->freeTxQ);
#if ZDCONF_LP_SUPPORT == 1
        if(sw_tcb == NULL) {
            printk(KERN_ERR "FreeTxQ is NULL !!! Very Serious\n");
            printk(KERN_ERR "Maybe Cause by LP with Multiple Push");
            printk(KERN_ERR "Check LP vs. FreeTxQ\n");
        }
        memcpy(sw_tcb->Padding, "abc",3);


        if(frag_info->macHdr[0][0]==0x88)
        {
            memcpy(tmpBuf, frag_info->macHdr[0]+24, frag_info->hdrLen-24);
            memcpy(frag_info->macHdr[0]+26, tmpBuf, frag_info->hdrLen-24);
            frag_info->macHdr[0][24]=BIT_7;
            frag_info->macHdr[0][25]=0;
            frag_info->hdrLen+=2;
            hdrLen = frag_info->hdrLen;
        }



        if(dot11Obj.LP_MODE)
            sw_tcb->LP_bucket = lp_bucket;
#endif

#ifdef HOST_IF_USB
		//sw_tcb->bHasCompleteBeforeSend = 0;
		//sw_tcb->bHasBeenDelayedBefore = 0;
#endif
		hdr = frag_info->macHdr[i];

		if (macp->dbg_flag > 4)
			zd1205_dump_data("header part", (U8 *)hdr, 24);
            

		if (skb){
			if ((bIntraBss) || (!frag)){  //wireless forwarding or tx data from upper layer and no linux frag
			ZD1211DEBUG(2, "zd1211: Wireless forwarding or no linux frag\n");
			pBody = frag_info->macBody[i];
			bodyLen = frag_info->bodyLen[i];
			CurrFragLen = bodyLen;
			NextFragLen = frag_info->nextBodyLen[i];
			if (i == (numOfFrag - 1))
				sw_tcb->LastFrag = 1;
			else
				sw_tcb->LastFrag = 0;
		}
		else{ //tx data from upper layer with linux frag
			ZD1211DEBUG(2, "zd1211: tx data from upper layer with linux frag\n");
			pBody = skb->data;
			bodyLen = skb->len;

			if (i == (numOfFrag - 1)){
				CurrFragLen = bodyLen - i*pdu_size;
				NextFragLen = 0;
				sw_tcb->LastFrag = 1;
			}
			else{
				CurrFragLen = pdu_size;
				sw_tcb->LastFrag = 0;

			        if (i == (numOfFrag-2))
				        NextFragLen = bodyLen - (i+1)*pdu_size;
			        else
				        NextFragLen = pdu_size;

			}
		}
	}
        else{  //mgt frame
		//ZD1211DEBUG(2, "zd1211: mgt frame\n");

		pBody = frag_info->macBody[i];
		bodyLen = frag_info->bodyLen[i];
		CurrFragLen = bodyLen;
		NextFragLen = frag_info->nextBodyLen[i];
		sw_tcb->LastFrag = 1;
        }

	wla_hdr = (wla_Header_t *)hdr;
	hw_tcb = sw_tcb->pTcb;
	pTbd = sw_tcb->pFirstTbd;
	tcb_tbd_num = 0;
	hw_tcb->TxCbTbdNumber = 0;
	
	sw_tcb->FrameType = hdr[0];
	sw_tcb->MsgID = MsgID;
	sw_tcb->aid = aid;
        sw_tcb->skb = skb;
        sw_tcb->bIntraBss = bIntraBss;
        sw_tcb->Rate = frag_info->rate;

#if ZDCONF_LP_SUPPORT == 1
//ZD1211DEBUG(2, "zd1212: sw_tcb = %x\n", sw_tcb);
        ZD1211DEBUG(2, "zd1212: hw_tcb = %lx\n", (U32)hw_tcb);
        ZD1211DEBUG(2, "zd1212: first tbd = %lx\n", (U32)pTbd);
        if(dot11Obj.LP_MODE && lp_bucket) {
            for(i=1;i<lp_bucket->pktCnt;i++) {
                CurrFragLen += lp_bucket->pkt[i].bodyLen[0]+14;
                if(i != lp_bucket->pktCnt - 1)
                    CurrFragLen+= (4-((lp_bucket->pkt[i].bodyLen[0]+14) % 4))%4;
                NextFragLen += lp_bucket->pkt[i].nextBodyLen[0]+14;
            }
            CurrFragLen += 14; //SLOW DOWN
            if(lp_bucket->pktCnt != 1)
                CurrFragLen += (4-((lp_bucket->pkt[0].bodyLen[0]+14) % 4))%4;
            CurrFragLen += 2; //For QoS Ctrl Field
            //CurrFragLen += lp_bucket->pktCnt*2+2;
            //printk("Total CurrFragLen : %d\n", CurrFragLen);
        }

        if (!macp->bPSMSupported)
            if (wla_hdr->Duration[0] != 0 || wla_hdr->Duration[1] != 0)
                printk(KERN_ERR "Dur = %d\n", wla_hdr->Duration[0] + ((U16)wla_hdr->Duration[1]<<8));

#endif


	ctrl_setting_parms.CurrFragLen = CurrFragLen;
	ctrl_setting_parms.NextFragLen = NextFragLen;

	/* Control Setting */
	length = Cfg_CtrlSetting(macp, sw_tcb, wla_hdr, &ctrl_setting_parms);
    if(wla_hdr->FrameCtrl[0] == PS_POLL)
    {
        if((*(U16 *)wla_hdr->Duration) ==  0)
            printk("AAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBB\n");
    }
	TotalLength = length;

	pTbd->TbdBufferAddrHighPart = (sw_tcb->HwCtrlPhys >> 31) >> 1;
	pTbd->TbdBufferAddrLowPart = (U32)sw_tcb->HwCtrlPhys;
	pTbd->TbdCount = length;
	pTbd++;
	tcb_tbd_num++;
#if ZDCONF_LP_SUPPORT == 1
        cfgLength = length;
#endif
    //if(*(u16 *)wla_hdr->Duration != 0) { //Possible Alignment Problem
    //if(wla_hdr->Duration[0]+wla_hdr->Duration[1] != 0)
    if(wla_hdr->FrameCtrl[0] != PS_POLL)
    {  
        wla_hdr->Duration[0] = 0;
        wla_hdr->Duration[1] = 0;
    }

	/* MAC Header */
	if (ctrl_setting_parms.encryType == TKIP){
		length = Cfg_MacHeader(macp, sw_tcb, wla_hdr, hdrLen);
		pTbd->TbdBufferAddrHighPart = (sw_tcb->HwHeaderPhys >> 31) >> 1;
		pTbd->TbdBufferAddrLowPart = (U32)sw_tcb->HwHeaderPhys;
	}
	else { //WPA will failed, why??
		length = hdrLen;
		pTbd->TbdBufferAddrHighPart = (((unsigned long)hdr) >> 31) >> 1;
		pTbd->TbdBufferAddrLowPart = (u32)hdr;
	}

		
	TotalLength += length;
	pTbd->TbdCount = length;
	pTbd++;
	tcb_tbd_num++;

#if defined(AMAC)
#if ZDCONF_LP_SUPPORT == 1
        if(!lp_bucket || !dot11Obj.LP_MODE)
#endif
	TotalLength += CurrFragLen;
	#ifdef ZD1211
		sw_tcb->pHwCtrlPtr->CtrlSetting[18] = (u8)TotalLength;
		sw_tcb->pHwCtrlPtr->CtrlSetting[19] = (u8)(TotalLength >> 8);
	#endif
#endif

	/* Frame Body */
	if ((!skb) || ((skb) && (!frag)) ) {
		unsigned long body_dma, tbdidx;

		ZD1211DEBUG(2, "zd1211: Management frame body or No linux frag\n");
#if ZDCONF_LP_SUPPORT == 1
        if(!dot11Obj.LP_MODE || !lp_bucket)
        {
#endif
            if (macp->dbg_flag > 4)
                zd1205_dump_data("data part", (U8 *)pBody, 14);

            pTbd->TbdBufferAddrHighPart = 0;
#ifndef HOST_IF_USB	
            body_dma =  pci_map_single(macp->pdev, pBody, bodyLen, PCI_DMA_TODEVICE);
#else
            body_dma = (unsigned long)pBody;            
#endif

            ZD1211DEBUG(2, "zd1211: body_dma = %x\n", (u32)body_dma);

            pTbd->TbdBufferAddrHighPart =  (body_dma >> 31) >> 1;
            pTbd->TbdBufferAddrLowPart =  (U32)body_dma;
            pTbd->TbdCount = CurrFragLen;
            pBody += CurrFragLen;

#ifdef HOST_IF_USB
            pTbd->PrvFragLen = PrvFragLen;
            PrvFragLen += CurrFragLen;
#endif

            pTbd++;
            tcb_tbd_num++;
        }
#if ZDCONF_LP_SUPPORT == 1
        else if(dot11Obj.LP_MODE && lp_bucket) //Long Packet 
        {
            //printk("pktCnt:%d\n", lp_bucket->pktCnt);

            for(tbdidx = 0; tbdidx < lp_bucket->pktCnt;tbdidx++) {
                //SLOW DOWN

                pTbd->TbdBufferAddrHighPart = 0;
                lp_bucket->pkt[tbdidx].EthHdr[13] = (14+lp_bucket->pkt[tbdidx].bodyLen[0]) & 0xFF; 
                lp_bucket->pkt[tbdidx].EthHdr[12] = (14+lp_bucket->pkt[tbdidx].bodyLen[0]) >> 8;
                xxx =
                    be16_to_cpu(*(U16 *)(lp_bucket->pkt[tbdidx].EthHdr+12)) > 3500;
                if(xxx > 3500)
                    printk("What!? size > 3500 : %ld\n", xxx);
                memcpy(sw_tcb->HdrInfo[tbdidx],lp_bucket->pkt[tbdidx].EthHdr,14);
                pBody = sw_tcb->HdrInfo[tbdidx];
                bodyLen = 14;
                body_dma =  (unsigned long)pBody;
                    
                if(!body_dma)
                    printk("!!!!!!!!! body_dma is NULL\n");
                pTbd->TbdBufferAddrHighPart =  (body_dma >> 31) >> 1;
                pTbd->TbdBufferAddrLowPart =  (U32)body_dma;
                pTbd->TbdCount = bodyLen;

                pTbd++;
                tcb_tbd_num++;
                TotalLength += bodyLen;
                bodyLength += bodyLen;

                pTbd->TbdBufferAddrHighPart = 0;
                pBody = lp_bucket->pkt[tbdidx].macBody[0]; //SLOW
                bodyLen = lp_bucket->pkt[tbdidx].bodyLen[0];//SLOW
                body_dma =  (unsigned long)pBody;
                if(!body_dma)
                    printk("!!!!!!!!!!!!! body_dma is NULL\n");
                pTbd->TbdBufferAddrHighPart =  (body_dma >> 31) >> 1;
                pTbd->TbdBufferAddrLowPart =  (U32)body_dma;
                pTbd->TbdCount = bodyLen;

                pTbd++;
                tcb_tbd_num++;
                TotalLength += bodyLen;
                bodyLength += bodyLen;

                //For A-MSDU Padding
                if(tbdidx != lp_bucket->pktCnt -1 && ((bodyLen+14)%4))
                {
                    pTbd->TbdBufferAddrHighPart = 0;
                    pBody = sw_tcb->Padding; //SLOW
                    bodyLen = (4-(bodyLen + 14) % 4)%4;
                    body_dma =  (unsigned long)pBody;
                    pTbd->TbdBufferAddrHighPart =  (body_dma >> 31) >> 1;
                    pTbd->TbdBufferAddrLowPart =  (U32)body_dma;
                    pTbd->TbdCount = bodyLen;

                    pTbd++;
                    tcb_tbd_num++;
                    TotalLength += bodyLen;
                    bodyLength += bodyLen;
                }

            }
        }
    }    
#endif
    else {
        while(CurrFragLen ){
            unsigned long body_dma;


            if (CurrFragLen >= frag->size ){
                printk(KERN_DEBUG "zd1205: linux more frag skb\n");
                addr = ((void *) page_address(frag->page) + frag->page_offset);
                pTbd->TbdBufferAddrHighPart = 0;
#ifndef HOST_IF_USB	               	    
                body_dma = pci_map_single(macp->pdev, addr, frag->size, PCI_DMA_TODEVICE);
#else
                body_dma = (unsigned long)addr;                    
#endif
                pTbd->TbdBufferAddrHighPart =  (body_dma >> 31) >> 1;
                pTbd->TbdBufferAddrLowPart =  (U32)body_dma;
                pTbd->TbdCount = frag->size;
                tcb_tbd_num++;
#ifdef HOST_IF_USB
                pTbd->PrvFragLen = PrvFragLen;
                PrvFragLen += CurrFragLen;
#endif    			    
                CurrFragLen -= frag->size;
                frag++;
            }
            else{
                printk(KERN_DEBUG "zd1205: linux last frag skb\n");
                addr = ((void *) page_address(frag->page) + frag->page_offset);
                pTbd->TbdBufferAddrHighPart = 0;
#ifndef HOST_IF_USB						
                body_dma = cpu_to_le32(pci_map_single(macp->pdev, addr, pdu_size, PCI_DMA_TODEVICE));
#else
                body_dma =  (unsigned long)addr; 
#endif					                  
                pTbd->TbdBufferAddrHighPart =  (body_dma >> 31) >> 1;
                pTbd->TbdBufferAddrLowPart =  (U32)body_dma;
                frag->page_offset += CurrFragLen;
                frag->size -= CurrFragLen;
#ifdef HOST_IF_USB
                pTbd->PrvFragLen = PrvFragLen;
                PrvFragLen += CurrFragLen;
#endif   					
                CurrFragLen = 0;
            }

            printk(KERN_DEBUG "zd1205: page_address = %lx\n", (unsigned long)addr);
            printk(KERN_DEBUG "zd1205: body_dma = %lx\n", (unsigned long)body_dma);
            pTbd++;
            tcb_tbd_num++;
        }
    }

    hw_tcb->TxCbTbdNumber = tcb_tbd_num;
    macp->txCnt++;

#ifndef HOST_IF_USB        
    hw_tcb->CbCommand = 0; /* set this TCB belong to bus master */
    wmb();

    while(1){
        tmp_value = zd_readl(DeviceState);
        tmp_value &= 0xf;

        if ((tmp_value == TX_READ_TCB) || (tmp_value == TX_CHK_TCB)){
            /* Device is now checking suspend or not.
               Keep watching until it finished check. */
            loopCnt++;

            if (loopCnt > 1000000)
                break;

            udelay(1);
            continue;
        }
        else
            break;
    }

    if (loopCnt > 1000000)
        ZD1211DEBUG(0, "I am in zdcb_setup_next_send loop\n") ;

    ZD1211DEBUG(1, "zd1211: Device State = %x\n", (u32)tmp_value);

    if (tmp_value == TX_IDLE){ /* bus master get suspended TCB */
        macp->txIdleCnt++;

        /* Tx bus master is in idle state. */
        //tmpValue1 = zd_readl(InterruptCtrl);
        /* No retry fail happened */
        tmp_value3 = zd_readl(ReadTcbAddress);
        next_sw_tcb = macp->freeTxQ->first;

        if (tmp_value3 != le32_to_cpu(next_sw_tcb->pTcb->NextCbPhyAddrLowPart)){
            /* Restart Tx again */
            zd1205_start_download(sw_tcb->TcbPhys);
            ZD1211DEBUG(1, "zd1211: Write  PCI_TxAddr_p1 = %x\n", sw_tcb->TcbPhys);
        }
    }

    else if (tmp_value == 0xA){ //Dtim Notify Int happened
        zd1205_start_download(sw_tcb->TcbPhys | BIT_0);
    }

    ZD1211DEBUG(2, "zd1211: NAV_CCA = %x\n", zd_readl(NAV_CCA));
    ZD1211DEBUG(2, "zd1211: NAC_CNT = %x\n", zd_readl(NAV_CNT));

#endif


#ifdef HOST_IF_USB
    if(wla_hdr->FrameCtrl[0] == PS_POLL)
    {
        if((*(U16 *)wla_hdr->Duration) ==  0)
            printk("AAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBB\n");
    }

    //The following code is to handle cross fragment MIC
    memcpy(sw_tcb->CalMIC,frag_info->CalSwMic, MIC_LNG+1);
    sw_tcb->MIC_Start=0;
    sw_tcb->MIC_Len=0;
    if(i==numOfFrag-1 && sw_tcb->CalMIC[MIC_LNG]==TRUE) {
        if(frag_info->bodyLen[i] < MIC_LNG) {
            sw_tcb->MIC_Start=MIC_LNG-frag_info->bodyLen[i];
            sw_tcb->MIC_Len=frag_info->bodyLen[i];
        }
        else {
            sw_tcb->MIC_Start=0;
            sw_tcb->MIC_Len=MIC_LNG;
        }
        zd1205_qlast_txq(macp, macp->activeTxQ, sw_tcb);
        zd1211_submit_tx_urb(macp,TRUE);
    }
    else if(sw_tcb->CalMIC[MIC_LNG] == TRUE && (i == numOfFrag-2)) {
        if(frag_info->bodyLen[i+1] < MIC_LNG) {
            sw_tcb->MIC_Start=0;
            sw_tcb->MIC_Len=MIC_LNG-frag_info->bodyLen[i+1];
        }
        else {
            sw_tcb->MIC_Start=0;
            sw_tcb->MIC_Len=0;
        }
        zd1205_qlast_txq(macp, macp->activeTxQ, sw_tcb);
        zd1211_submit_tx_urb(macp,TRUE);
    }
    else
    {
        zd1205_qlast_txq(macp, macp->activeTxQ, sw_tcb);
        zd1211_submit_tx_urb(macp,FALSE);
    }
    //zd1205_tx_isr(macp); //for debug only
#endif
    g_dev->trans_start = jiffies;
    macp->TxStartTime = nowT();
    ZD1211DEBUG(2, "zd1211: Cnt of activeQ = %x\n", macp->activeTxQ->count);
}

#ifndef HOST_IF_USB    
spin_unlock_irqrestore(&macp->bd_non_tx_lock, lock_flag);
#endif    

ZD1211DEBUG(2, "===== zdcb_setup_next_send exit =====\n");
#endif
return TRUE;
}

void zdcb_release_buffer(void *buf)
{
    struct sk_buff *skb = (struct sk_buff *)buf;
    struct zd1205_private *macp = g_dev->priv;

    if (skb)
    {
        dev_kfree_skb_any(skb);
        macp->devBuf_free++;
    }
    //dev_kfree_skb_irq(skb);
}

#ifdef LINUX
void zdcb_rx_ind(U8 *pData, U32 length, void *buf, U32 LP_MAP)
{
    struct zd1205_private *macp = g_dev->priv;
    struct sk_buff *skb1 = (struct sk_buff *)buf;
    struct sk_buff *skb = NULL;
    U32 i,dataOff=0, pktLen=0,j=0;
    U8 totalValid = 0;
    U8 wCnt;
    U16 loopCheck = 0;
    unsigned long flags;
#if ZDCONF_TR_DBG == 1
    static unsigned long RX_TIME = 0;
    static unsigned long LAST_RX_TIME= 0;
    static unsigned long RX_BYTES = 0;
    static unsigned long LAST_RX_BYTES = 0;
#endif
#if ZDCONF_DRV_FORWARD != 1
	Hash_t *pHash = NULL;
#endif

    ZENTER(3);
    skb = skb1;
#if ZDCONF_LP_SUPPORT == 1
    if(LP_MAP) {
        //for(i=0;i<be16_to_cpu(*(U16 *)pData);i++) {
        length-=2; //Ignore QoS Ctrl
        pData+=2;
        wCnt = 0;
        while(1)
        {
            // Not used for general Driver
            if(loopCheck++ > 100)
            {
                printk("infinite loop occurs in %s\n", __FUNCTION__);
                loopCheck = 0;
                break;
            }

            wCnt++;
            pktLen = be16_to_cpu(*(U16 *)(pData+dataOff+12));
            //if(memcmp(pData+dataOff, ZD_DROPIT_TAG,6) != 0)
            if(LP_MAP & ( BIT_0<<(wCnt-1) ) )
                totalValid++;
            dataOff+=pktLen;
            if(dataOff >= length) break;
            dataOff += (4-(dataOff % 4))%4;

        }
        if(totalValid == 0)
        {
            dev_kfree_skb_any(skb);
            macp->devBuf_free++;
            return;
        }
        dataOff = 0;
        while(1)
        {
            // Not used for General Driver
            if(loopCheck++ > 100)
            {
                printk("infinite loop occurs in %s\n", __FUNCTION__);
                loopCheck = 0;
                break;
            }

            j++;
            pktLen = be16_to_cpu(*(U16 *)(pData+dataOff+12));
            //if(memcmp(pData+dataOff, ZD_DROPIT_TAG,6) == 0)
            if(!(LP_MAP & ( BIT_0<<(wCnt-1) ) ))
            {
                dataOff+=pktLen;
                if(dataOff >= length) break;
                dataOff += (4-(dataOff % 4))%4;
                continue;

            }
            if(dataOff + pktLen != length) {
                skb = skb_clone(skb1,GFP_ATOMIC);
                if(skb == NULL) printk(KERN_ERR "### skb NULL ##\n");
            }
            else
                skb = skb1;
            totalValid--;
            if(dataOff+pktLen > length) {
                printk("** pData+dataOff **\n");
                for(i=-15;i<20;i++)
                    printk("%02x ", *(pData+dataOff+i));
                printk("** pData **\n");
                for(i=-15;i<26;i++)
                    printk("%02x ", *(pData+i));
                printk("\n");

                printk("%ldth,Wrong !! dataOff+pktLen > length\n,%ld,%ld",j,dataOff+pktLen,length);
                return;
            }
            memcpy(pData+dataOff+14, pData+dataOff+6,6);
            memcpy(pData+dataOff+8, pData+dataOff,6);
        #ifdef NET_SKBUFF_DATA_USES_OFFSET
            skb->tail = 0;
            skb->data = pData+dataOff+8;
        #else
            skb->tail = skb->data = pData+dataOff+8;
        #endif
            skb_put(skb, pktLen-8);
            skb->protocol = eth_type_trans(skb, g_dev);
            skb->ip_summed = CHECKSUM_NONE;               //TBD
            g_dev->last_rx = jiffies;

            switch(netif_rx(skb))
            {
                case NET_RX_BAD:
                case NET_RX_DROP:
                case NET_RX_CN_MOD:
                case NET_RX_CN_HIGH:
                    break;
                default:
                    macp->drv_stats.net_stats.rx_packets++;
                    macp->drv_stats.net_stats.rx_bytes += skb->len;
                    break;
            }
            dataOff+=pktLen;
            if(dataOff >= length) break;
            dataOff += (4-(dataOff % 4))%4;
            if(totalValid == 0) return;
        }
        return;
    }
#endif
#if ZDCONF_DRV_FORWARD != 1 
#if ZDCONF_AP_SUPPORT == 1
    if(mBssType == AP_BSS)
    {
        int i;
        char *p;
        if(pData[0] & BIT_0) // Grup Address
        {
            struct sk_buff *intra_skb = NULL;
#ifdef NET_SKBUFF_DATA_USES_OFFSET
            skb->tail = 0;
            skb->data = pData;
#else
            skb->tail = skb->data = pData; 
#endif
            skb_put(skb, length);
            if(skb->len > 1518)
                printk("len > 1518 here, %d,%d\n",__LINE__,skb->len);

            intra_skb = skb_copy(skb,GFP_ATOMIC);
            macp->devBuf_alloc++;
            skb->len = 0; //skb->len is set in skb_put(). We will call skb_put again later

            //p=pData;
            //printk("IntraBSS_G:");
            //for(i=0;i<20;i++)
            //    printk("%02x ", (U8)p[i]);
            //printk("\n"); 
            if(!intra_skb)
                printk("intra_skb is NULL\n");
            else if(macp->intraBufCnt > 64)
            {
                dev_kfree_skb_any(intra_skb);
                macp->devBuf_free++;
            }
            else
            {
                intra_skb->dev = g_dev;
                intra_skb->next = NULL;
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,21))      
                intra_skb->mac.raw = intra_skb->data;
                intra_skb->nh.raw = intra_skb->data+14;
#else
                intra_skb->mac_header = intra_skb->data;
                intra_skb->network_header = intra_skb->data+14;
#endif

                g_dev->last_rx = jiffies;

                flags = dot11Obj.EnterCS();
                if(!macp->intraBuf)
                {
                    macp->intraBuf = intra_skb;
                    macp->last_intraBuf = intra_skb;
                }
                else
                {
                    macp->last_intraBuf->next=intra_skb;
                    macp->last_intraBuf = intra_skb;
                }
                macp->intraBufCnt++;
                dot11Obj.ExitCS(flags);
                defer_kevent(macp, KEVENT_DEV_QUEUE_XMIT);
                //dev_queue_xmit(intra_skb);
            }
        }
        else if(zd_QueryStaTable(pData, (void**)&pHash)) //Unicast to STA
        {
            int i;
            //p=pData;
            //printk("IntraBSS_U:");
#ifdef NET_SKBUFF_DATA_USES_OFFSET
            skb->tail = 0;
            skb->data = pData;
#else
            skb->tail = skb->data = pData; 
#endif
            skb_put(skb, length);
            skb->dev = g_dev;
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,21))      
            skb->mac.raw = skb->data;
            skb->nh.raw = skb->data+14;
#else
            skb->mac_header = skb->data;
            skb->network_header = skb->data+14;
#endif
            g_dev->last_rx = jiffies;

            //dev_queue_xmit(skb);
            skb->next = NULL;
            if(macp->intraBufCnt > 64)
            {
                dev_kfree_skb_any(skb);
                macp->devBuf_free++;
                return;
            }
            flags = dot11Obj.EnterCS();
            if(!macp->intraBuf)
            {
                macp->intraBuf = skb;
                macp->last_intraBuf = skb;
            }
            else
            {
                macp->last_intraBuf->next=skb;
                macp->last_intraBuf = skb;
            }
            macp->intraBufCnt++;
            dot11Obj.ExitCS(flags);
            defer_kevent(macp, KEVENT_DEV_QUEUE_XMIT);
            return;
        } 
    }
#endif
#endif

    //copy packet for IP header is located on 4-bytes alignment
#if ZDCONF_R8610_FRAG_FIX == 1
    if (1)
#else
    if (length < RX_COPY_BREAK)
#endif
    {
        dev_kfree_skb_any(skb);
        macp->devBuf_free++;
        skb = dev_alloc_skb(length+2);
        macp->devBuf_alloc++;

        if (skb){
            skb->dev = g_dev;
            skb_reserve(skb, 2);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23))      
            eth_copy_and_sum(skb, pData, length, 0);
#else
            skb_copy_from_linear_data(skb,pData,length);
#endif

            skb_put(skb, length);
        }
    }
    else{
    #ifdef NET_SKBUFF_DATA_USES_OFFSET
        skb->tail = 0;
        skb->data = pData;
    #else
        skb->tail = skb->data = pData;
    #endif
        skb_put(skb, length);
    }

    //zd1205_dump_data("rx_ind", (U8 *)skb->data, skb->len);

    ZD1211DEBUG(2, "zd1211: rx_ind length = %x\n", (u32)length);
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,21))      
    skb->mac.raw = skb->data;
    skb->nh.raw = skb->data+14;
#else
    skb->mac_header = skb->data;
    skb->network_header = skb->data+14;
#endif
    /* set the protocol */
    skb->protocol = eth_type_trans(skb, g_dev);
    skb->ip_summed = CHECKSUM_NONE;	//TBD
    g_dev->last_rx = jiffies;

#if ZDCONF_TR_DBG == 1
    if(macp->trDbg == 2) // 0: Disable, 1:TX ONLY, 2:RX ONLY
    {
        RX_TIME=jiffies;
        RX_BYTES += skb->len;
        if(RX_TIME - LAST_RX_TIME > HZ)
        {
            printk("RX Rate = %lu KB/s\n", (RX_BYTES-LAST_RX_BYTES)/1024);
            LAST_RX_TIME = RX_TIME;
            LAST_RX_BYTES = RX_BYTES;
        }
        dev_kfree_skb_any(skb);
        macp->devBuf_free++;
        return;
    }
#endif
    if(skb->len > 1518)
    {
        printk("data to upper layer is too long = %d\n",skb->len);
        dev_kfree_skb_any(skb);
        macp->devBuf_free++;
        return;
    }
    else
    {
        macp->devBuf_upper++;
        switch(netif_rx(skb)){
            case NET_RX_BAD:
            case NET_RX_DROP:
            case NET_RX_CN_MOD:
            case NET_RX_CN_HIGH:
                break;

            default:
                macp->drv_stats.net_stats.rx_packets++;
                macp->drv_stats.net_stats.rx_bytes += skb->len;
                break;
        }
    }
    ZEXIT(3);
    }

    U16 zdcb_status_notify(U16 status, U8 *StaAddr)
    {
        struct zd1205_private *macp = g_dev->priv;
    U16 result = 0;
    int newassoc = 0;

    switch (status){
        case STA_AUTH_REQ:
            break;

        case STA_ASOC_REQ:
            break;

        case STA_REASOC_REQ:
            break;		

        case STA_ASSOCIATED:
        case STA_REASSOCIATED:
            macp->bAssoc = 1;
            mTmRetryConnect=0;
            iLED_ON(macp, macp->LinkLEDn);
#ifdef HOST_IF_USB
            macp-> LinkTimer = 0;

            if (macp->DataLED == 0){
#ifdef ROBIN_KAO
                zd_writel(0x03, FW_LINK_STATUS);
#else
                zd_writel(0x01, FW_LINK_STATUS);                
#endif                
            }
            else
                zd_writel(0x00, FW_LINK_STATUS);
#endif
            memcpy(&macp->BSSID[0], StaAddr, 6);

            //if (macp->cardSetting.BssType == INFRASTRUCTURE_BSS)
            if (macp->cardSetting.BssType != AP_BSS)
            {
                netif_wake_queue(macp->device);
                netif_carrier_on(macp->device);
            }

            if (status == STA_ASSOCIATED) 
            {
                ZD1211DEBUG(0, "STA_ASSOCIATED with " MACSTR "\n", MAC2STR(StaAddr));
                newassoc = 1;
            }
            else
            {
                ZD1211DEBUG(0, "STA_REASSOCIATED with " MACSTR "\n", MAC2STR(StaAddr));
            }
            /* Generate a wireless event to the upper layer */
            if(macp->cardSetting.ap_scan != 1 || test_and_clear_bit(CTX_FLAG_ESSID_WAS_SET, (void*)&macp->flags))
            {
                zd1205_notify_join_event(macp);
            }

            break;

        case STA_DISASSOCIATED:
        case STA_DEAUTHED:
#ifndef HOST_IF_USB
            iLED_OFF(macp, LED1);
#else
            zd_writel(0x0, FW_LINK_STATUS);            
#endif        

            if (macp->cardSetting.BssType == INFRASTRUCTURE_BSS)
            {
                union iwreq_data wreq;
                macp->bAssoc = 0;
                memset(&wreq, 0, sizeof(wreq));
                wreq.addr.sa_family=ARPHRD_ETHER;
			#if Make_WPA
                wireless_send_event(macp->device, SIOCGIWAP,&wreq, NULL);
			#endif
                memset(&macp->BSSID[0], 0, 6);
                netif_stop_queue(macp->device);
                //zd1205_dis_connect(macp);
                netif_carrier_off(macp->device);
            }    
            if (status == STA_DISASSOCIATED)
                printk("STA_DISASSOCIATED:" MACSTR "\n",MAC2STR(StaAddr));
            else
                printk("STA_DEAUTHED:" MACSTR "\n",MAC2STR(StaAddr));

            break;

        default:
            break;
    }

    return result;
}
#endif

    U16 zdcb_status_notify(U16 status, U8 *StaAddr)
    {
        struct zd1205_private *macp = g_dev->priv;
    U16 result = 0;
    int newassoc = 0;

    switch (status){
        case STA_AUTH_REQ:
            break;

        case STA_ASOC_REQ:
            break;

        case STA_REASOC_REQ:
            break;		

        case STA_ASSOCIATED:
        case STA_REASSOCIATED:
            macp->bAssoc = 1;
            mTmRetryConnect=0;
            iLED_ON(macp, macp->LinkLEDn);
#ifdef HOST_IF_USB
            macp-> LinkTimer = 0;

            if (macp->DataLED == 0){
#ifdef ROBIN_KAO
                zd_writel(0x03, FW_LINK_STATUS);
#else
                zd_writel(0x01, FW_LINK_STATUS);                
#endif                
            }
            else
                zd_writel(0x00, FW_LINK_STATUS);
#endif
            memcpy(&macp->BSSID[0], StaAddr, 6);

            //if (macp->cardSetting.BssType == INFRASTRUCTURE_BSS)
            if (macp->cardSetting.BssType != AP_BSS)
            {
                netif_wake_queue(macp->device);
                netif_carrier_on(macp->device);
            }

            if (status == STA_ASSOCIATED) 
            {
                ZD1211DEBUG(0, "STA_ASSOCIATED with " MACSTR "\n", MAC2STR(StaAddr));
                newassoc = 1;
            }
            else
            {
                ZD1211DEBUG(0, "STA_REASSOCIATED with " MACSTR "\n", MAC2STR(StaAddr));
            }
            /* Generate a wireless event to the upper layer */
            if(macp->cardSetting.ap_scan != 1 || test_and_clear_bit(CTX_FLAG_ESSID_WAS_SET, (void*)&macp->flags))
            {
                zd1205_notify_join_event(macp);
            }

            break;

        case STA_DISASSOCIATED:
        case STA_DEAUTHED:
#ifndef HOST_IF_USB
            iLED_OFF(macp, LED1);
#else
            zd_writel(0x0, FW_LINK_STATUS);            
#endif        

            if (macp->cardSetting.BssType == INFRASTRUCTURE_BSS)
            {
                union iwreq_data wreq;
                macp->bAssoc = 0;
                memset(&wreq, 0, sizeof(wreq));
                wreq.addr.sa_family=ARPHRD_ETHER;
			#if Make_WPA
                wireless_send_event(macp->device, SIOCGIWAP,&wreq, NULL);
			#endif
                memset(&macp->BSSID[0], 0, 6);
                netif_stop_queue(macp->device);
                //zd1205_dis_connect(macp);
                netif_carrier_off(macp->device);
            }    
            if (status == STA_DISASSOCIATED)
                printk("STA_DISASSOCIATED:" MACSTR "\n",MAC2STR(StaAddr));
            else
                printk("STA_DEAUTHED:" MACSTR "\n",MAC2STR(StaAddr));

            break;

        default:
            break;
    }

    return result;
}

void zdcb_tx_completed(void)
{

}

void chal_tout_cb(unsigned long ptr)
{
#ifdef HOST_IF_USB
    struct zd1205_private *macp = g_dev->priv;
    defer_kevent(macp, KEVENT_TCHAL_TIMEOUT);
#else       
    zd_EventNotify(EVENT_TCHAL_TIMEOUT, 0, 0, 0);
#endif
}

void scan_tout_cb(unsigned long ptr)
{
#ifdef HOST_IF_USB
    struct zd1205_private *macp = g_dev->priv;
    defer_kevent(macp, KEVENT_SCAN_TIMEOUT);
#else
    zd_EventNotify(EVENT_SCAN_TIMEOUT, 0, 0, 0);
#endif
}

void asoc_tout_cb(unsigned long ptr)
{
#ifdef HOST_IF_USB
    struct zd1205_private *macp = g_dev->priv;
    defer_kevent(macp, KEVENT_AUTH_TIMEOUT);
#else    
    zd_EventNotify(EVENT_ASOC_TIMEOUT, 0, 0, 0);
#endif    
}

void auth_tout_cb(unsigned long ptr)
{
#ifdef HOST_IF_USB
    struct zd1205_private *macp = g_dev->priv;
    defer_kevent(macp, KEVENT_AUTH_TIMEOUT);
#else
    zd_EventNotify(EVENT_AUTH_TIMEOUT, 0, 0, 0);
#endif        
}
void zdcb_start_timer(U32 timeout, U32 event)
{
    struct zd1205_private *macp = g_dev->priv;
    u32	timeout_in_jiffies;
    if (!macp->bUSBDeveiceAttached)
        return;
    timeout_in_jiffies= (timeout*HZ)/1000; // Conver ms to jiffies

    switch (event){
        case DO_CHAL:
            init_timer(&macp->tm_chal_id);
            macp->tm_chal_id.data = (unsigned long) g_dev;
            macp->tm_chal_id.expires = jiffies + timeout_in_jiffies;
            macp->tm_chal_id.function = chal_tout_cb;
            add_timer(&macp->tm_chal_id);
            break;

        case DO_SCAN:
            init_timer(&macp->tm_scan_id);
            macp->tm_scan_id.data = (unsigned long) g_dev;
            macp->tm_scan_id.expires = jiffies + timeout_in_jiffies;
            macp->tm_scan_id.function = scan_tout_cb;
            add_timer(&macp->tm_scan_id);
            break;


        case DO_AUTH:
            init_timer(&macp->tm_auth_id);
            macp->tm_auth_id.data = (unsigned long) g_dev;
            macp->tm_auth_id.expires = jiffies + timeout_in_jiffies;
            macp->tm_auth_id.function = auth_tout_cb;
            add_timer(&macp->tm_auth_id);
            break;

        case DO_ASOC:
            if(AsocTimerStat) {
                del_timer_sync(&macp->tm_asoc_id);
                AsocTimerStat = FALSE;
            }
            init_timer(&macp->tm_asoc_id);
            macp->tm_asoc_id.data = (unsigned long) g_dev;
            macp->tm_asoc_id.expires = jiffies + timeout_in_jiffies;
            macp->tm_asoc_id.function = asoc_tout_cb;
            add_timer(&macp->tm_asoc_id);
            AsocTimerStat = TRUE;
            break;	

        default:
            break;
    }				
}

void zdcb_stop_timer(U32 TimerId)
{
    struct zd1205_private *macp = g_dev->priv;

    switch (TimerId){
        case DO_CHAL:
            del_timer(&macp->tm_chal_id);
            break;

        case DO_AUTH:
            del_timer(&macp->tm_auth_id);
            break;


        case DO_ASOC:
            del_timer(&macp->tm_asoc_id);
            AsocTimerStat = FALSE;
            break;

        default:
            break;			

    }
}

unsigned long zdcb_dis_intr(void)
{
    struct zd1205_private *macp = g_dev->priv;
    U32 flags = 0;

#if 1//ndef HOST_IF_USB
    spin_lock_irqsave(&macp->cs_lock, flags);
#else    
    spin_lock(&macp->cs_lock);
#endif    
    return flags;
}

    void
zdcb_set_intr_mask(unsigned long flags)
{
    struct zd1205_private *macp = g_dev->priv;

#if 1//ndef HOST_IF_USB
    spin_unlock_irqrestore(&macp->cs_lock, flags);
#else
    spin_unlock(&macp->cs_lock);
#endif    
}

U32 zdcb_vir_to_phy_addr(U32 virtAddr) //TBD
{
    return virtAddr;
}

void zdcb_set_reg(void *reg, U32 offset, U32 value)
{
    zd_writel(value, offset);
}

U32 zdcb_get_reg(void *reg, U32 offset)
{
    return zd_readl(offset);
}

    BOOLEAN
zdcb_check_tcb_avail(U8	num_of_frag)
{
    struct zd1205_private *macp = g_dev->priv;
    BOOLEAN ret;

    unsigned long flags;

    spin_lock_irqsave(&macp->q_lock, flags);
    if (macp->freeTxQ->count < (num_of_frag+1))
        ret = FALSE;
    else
        ret = TRUE;

    spin_unlock_irqrestore(&macp->q_lock, flags);
    return ret;
}


void zdcb_delay_us(U32 ustime)
{
    struct zd1205_private *macp=g_dev->priv;
    //Convert microseconds to jiffies. 20060809 MZCai
    U32 delay_ms = ustime/1000;
    U32 delay_jiffies = delay_ms / (1000/HZ);
    if(delay_jiffies == 0) delay_jiffies = 1;

#if 0//(LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0))      
    wait_event_interruptible_timeout(macp->msdelay, 0, delay_jiffies);
#else
#ifdef LINUX
    interruptible_sleep_on_timeout(&macp->msdelay, delay_jiffies);
#else
    udelay(ustime);
#endif
#endif

    //udelay(ustime);
}
void * zdcb_AllocBuffer(U16 dataSize, U8 **pData)
{
    struct sk_buff *new_skb = NULL;
    struct zd1205_private *macp = (struct zd1205_private *)g_dev->priv;

    new_skb = (struct sk_buff *) dev_alloc_skb(dataSize);
    if (new_skb){
        *pData = new_skb->data;
        macp->devBuf_alloc++;
    }

    return (void *)new_skb;	
}	

unsigned long int next = 1;

int zdcb_Rand(U32 seed)
{
    static int first = 1;

    if (first){	
        next = seed;
        first = 0;
    }    

    next = next * 1103515245 + 12345;
    return ((unsigned int)(next/65535)%32768);
}

void zdcb_AcquireDoNotSleep(void)
{
    struct zd1205_private *macp = g_dev->priv;
    atomic_inc(&macp->DoNotSleep);
}

void zdcb_ReleaseDoNotSleep(void)
{
    struct zd1205_private *macp = g_dev->priv;
    atomic_dec(&macp->DoNotSleep);
}         

void zd1205_list_bss(struct zd1205_private *macp)
{
#if ZDCONF_DBGMSG_NORMAL == 1
    int i, j;
    u16 cap;
    bss_info_t *pBssInfo;

    printk("\nSSID  BSSID     CH  Signal  Mode  AP-Type Other");
    printk("\n-----------------------------------------------------------------");

    for (i=0; i<macp->bss_index; i++)
    {
        pBssInfo = &macp->BSSInfo[i];

        printk("\n");
        printk("%2d ",i+1);
        for (j=0; j<pBssInfo->ssid[1]; j++)
        {
            printk("%c", pBssInfo->ssid[2+j]);
        }
        printk("\n");
        printk("  ");

        printk("%02x%02x%02x%02x%02x%02x",
            pBssInfo->bssid[0], pBssInfo->bssid[1], pBssInfo->bssid[2],
            pBssInfo->bssid[3], pBssInfo->bssid[4], pBssInfo->bssid[5]);

        printk(" %4d", pBssInfo->channel);
        printk(" %4d", pBssInfo->signalStrength);
        printk("   ");

        cap = pBssInfo->cap;
        cap &= (0x10 | 0x02 | 0x01);

        switch(cap)
        {
            case 0x01:
                printk(" I");
                break;

            case 0x02:
                printk(" A");
                break;

            case 0x11:
                printk("Iw");
                break;

            case 0x12:
                printk("Aw");
                break;

            default :
                break;
        }

        printk("    ");
/*

        for (j=0; j<pBssInfo->supRates[1]; j++)
        {
            printk("%2d", (pBssInfo->supRates[2+j] & 0x7F)*5/10);
            if(j != pBssInfo->supRates[1]-1)
                printk(",");
        }

        printk("  ");
        for (j=0; j<pBssInfo->extRates[1]; j++)
        {
            printk("%2d", (pBssInfo->extRates[2+j] & 0x7F)*5/10);
            if(j != pBssInfo->extRates[1]-1)
                printk(",");
        }
*/

        if (pBssInfo->apMode == PURE_B_AP)
            printk(" B-AP");
        else if (pBssInfo->apMode == PURE_G_AP)
            printk(" G-AP");
        else if  (pBssInfo->apMode == MIXED_AP)
            printk(" M-AP");
        else if (pBssInfo->apMode == PURE_A_AP)
            printk(" A-AP");
        else
            {VerAssert();}
#if ZDCONF_LP_SUPPORT == 1
        if(pBssInfo->zdIE_Info_BURST[0] == EID_ZYDAS)
        {
            if(pBssInfo->zdIE_Info_BURST[2] == (U8)ZDOUI_BURST &&
               pBssInfo->zdIE_Info_BURST[3] == (U8)(ZDOUI_BURST >> 8) &&
               pBssInfo->zdIE_Info_BURST[4] == (U8)(ZDOUI_BURST >> 16))
            {
            
                if(pBssInfo->zdIE_Info_BURST[8] & BIT_7)
                    printk(" BurstOn(0x%02x)", pBssInfo->zdIE_Info_BURST[8] & 0x7F);
                else
                    printk(" BurstOff ");
            }
        }

        if(pBssInfo->zdIE_Info_AMSDU[0] == EID_ZYDAS)
        {
            if(pBssInfo->zdIE_Info_AMSDU[2] == (U8)ZDOUI_AMSDU &&
               pBssInfo->zdIE_Info_AMSDU[3] == (U8)(ZDOUI_AMSDU >> 8) &&
               pBssInfo->zdIE_Info_AMSDU[4] == (U8)(ZDOUI_AMSDU >> 16))
            {
            
                if(pBssInfo->zdIE_Info_AMSDU[8] & BIT_0)
                    printk(" AMSDU_On(%d)", pBssInfo->zdIE_Info_AMSDU[8] & BIT_1);
                else
                    printk(" AMSDU_Off");
            }
        }
#endif
#if ZDCONF_SES_SUPPORT == 1
        if(pBssInfo->SES_Element_Valid)
        {
            printk(" SES(%d)", pBssInfo->SES_Element.buf[1]);
        }
#endif
    }
    printk("\n");
#endif

}    
static int
zd1205_ioctl_setpower(struct net_device *dev, struct iw_param *frq)
{
	struct zd1205_private *macp = dev->priv;

	int err = 0;
#if ZDCONF_STA_PSM == 1
	zd1205_lock(macp);
	
	if (frq->disabled){
		printk(KERN_ERR "power save disabed\n");
		macp->cardSetting.ATIMWindow = 0x0;
		macp->bPSMSupported = 0;
		macp->PwrState = PS_CAM;
		zd_EventNotify(EVENT_PS_CHANGE, (U8)macp->PwrState, 0, 0);
	}	
	else{ 
        if(frq->flags != IW_POWER_TIMEOUT)
        {
            printk("The PSM command syntax :\n");
            printk(" iwconfig ethX power timeout DATA_COUNTu \n");
            printk("When the data is less than DATA_COUNT, STA enters PowerSaving, else WakeUP\n"); 
            printk("Exampel : iwconfig eth1 power timeout 500000u\n");
            printk("   When traffic is less than 500k/s, Enter Power Saving\n");
            err = -EINVAL;
        }
        if(!err)
        {
            printk(KERN_ERR "power save enabled\n");
            printk("The PSM Threshold is %dK %dBytes\n", frq->value/1024,frq->value%1024);
            macp->PSThreshhold= frq->value;
            macp->cardSetting.ATIMWindow = 0x5;
            macp->bPSMSupported = 1;
        }
	}	

	zd1205_unlock(macp);
    printk("dot11Obj.BeaconInterval:%d,BEFORE_BEACON:%d\n",dot11Obj.BeaconInterval,BEFORE_BEACON);
    if(!err)
        HW_UpdatePreTBTT(&dot11Obj, dot11Obj.BeaconInterval-BEFORE_BEACON);	
#endif
	return err;
}
    
static int
zd1205_ioctl_getpower(struct net_device *dev, struct iw_param *frq)
{
	struct zd1205_private *macp = dev->priv;

	zd1205_lock(macp);
	if (macp->bPSMSupported)
		frq->disabled = 0;
	else 
		frq->disabled = 1;	
	zd1205_unlock(macp);

	return 0;
}

static long
zd1205_hw_get_freq(struct zd1205_private *macp)
{
	u32 freq;
	zd1205_lock(macp);
	if(PURE_A_MODE != mMacMode)
		freq = channel_frequency[dot11Obj.Channel-1] * 100000;
#if ZDCONF_80211A_SUPPORT == 1
	else if(PURE_A_MODE == mMacMode)
	//for PURE_A_MODE the Channel Number is not required to sub one.
	//Because the channel is get from setting not the order in array
		freq =  channel_11A_to_Freq(dot11Obj.Channel) * 100000;
#endif
	zd1205_unlock(macp);
	return freq;

}  

#define MIN_KEY_SIZE    5                  

static int
zd1205_ioctl_setiwencode(struct net_device *dev, struct iw_point *erq, char *key)
{
    	//BOOLEAN bReconnect=FALSE;
	struct zd1205_private *macp = dev->priv;
	card_Setting_t *pSetting = &macp->cardSetting;
    
	if (erq->length > 0)
	{
		int index = (erq->flags & IW_ENCODE_INDEX) - 1;
		int current_index =  pSetting->EncryKeyId;

	//	ZD1211DEBUG(1, "index = %d\n", index);
	//	ZD1211DEBUG(1, "erq->length = %d\n", erq->length);
        
		if (erq->length > MAX_KEY_SIZE)
			return -EINVAL;
            
		if ((index < 0) || (index >= 4))
			index = current_index;

		/* Set the length */
		if (erq->length > MIN_KEY_SIZE){
			pSetting->WepKeyLen = MAX_KEY_SIZE;
			pSetting->EncryMode = WEP128;
		}
		else {
		//	if (erq->length > 0){
				pSetting->WepKeyLen = MIN_KEY_SIZE;
				pSetting->EncryMode = WEP64;
		//	}    
		//	else { 
		//		pSetting->WepKeyLen = 0;   /* Disable the key */
		//		pSetting->EncryMode = NO_WEP;
		//	}
		}

		/* Check if the key is not marked as invalid */
		if (!(erq->flags & IW_ENCODE_NOKEY))
		{  // for command: key xxxxxxxxxx [n]
	//		ZD1211DEBUG(0, "Set contents of key %d\n", index+1);
            pSetting->EncryOnOff=1;      
			pSetting->EncryKeyId = index;
			memcpy(&pSetting->keyVector[index][0], key, pSetting->WepKeyLen);
			zd1205_config_wep_keys(macp);
		}
		else
		{ // For command: key on
//			ZD1211DEBUG(0, "key %d is enabled\n", index+1);
		}

		/* WE specify that if a valid key is set, encryption
		 * should be enabled (user may turn it off later)
		 * This is also how "iwconfig ethX key on" works */                           
		/*if ((index == current_index) && (pSetting->WepKeyLen > 0) &&
			(pSetting->EncryOnOff == 0)) {
			pSetting->EncryOnOff = 1;
		} */
	}
	else if(erq->flags & IW_ENCODE_DISABLED)
	{       // for command: key off
	//	ZD1211DEBUG(0, "Disable Encryption\n");
		pSetting->EncryOnOff=0;
        zd1205_config_wep_keys(macp);
	}
	else
	{ 
		/* Do we want to just set the transmit key index ? */
		// For command: (erq->length==0)
		//              key on (If no key ever set)
		//              key [n] , change current active key  
		int index = (erq->flags & IW_ENCODE_INDEX) - 1;
		//ZD1211DEBUG(0, "change key %d as active key\n", index+1);
		if ((index >= 0) && (index < 4)) 
		{
//			ZD1211DEBUG(0, "Active key id=%d\n", index+1);
			pSetting->EncryKeyId = index; // Because pSetting->WepKeyLen has been set, it is not necessary to set it again!
			pSetting->EncryOnOff = 1;
		} 
		else	/* Don't complain if only change the mode */
		{
			if(!(erq->flags & IW_ENCODE_MODE))
			{
//				ZD1211DEBUG(0, "change mode for invalid key id:%d\n",index+1); 
				return -EINVAL;
			}
		}
	}
	if(erq->flags & IW_ENCODE_RESTRICTED){
		pSetting->EncryOnOff = 1;	
	}

	if(erq->flags & IW_ENCODE_OPEN) {
		pSetting->EncryOnOff = 1;	// Only Wep
	}

//	ZD1211DEBUG(0,"pSetting->EncryOnOff: %d\n", pSetting->EncryOnOff);
    if (mPrivacyInvoked == pSetting->EncryOnOff)
    { // Privacy setting is the same as before one, No need do reconnect, just update some global parameters.
      
        mKeyFormat = pSetting->EncryMode;
        mKeyId = pSetting->EncryKeyId;
        mPrivacyInvoked = pSetting->EncryOnOff;
        if (mPrivacyInvoked)
            mCap |= CAP_PRIVACY;
        else
            mCap &= ~CAP_PRIVACY;
        memcpy(&mKeyVector[0][0], &pSetting->keyVector[0][0],sizeof(mKeyVector));
        mWepKeyLen = pSetting->WepKeyLen;
        printk(KERN_DEBUG "Just Update WEP key\n");
        return 0;
    }
    printk(KERN_DEBUG "Update CardSetting\n");
    

#ifdef HOST_IF_USB
	defer_kevent(macp, KEVENT_UPDATE_SETTING);	
#else		
	zd_UpdateCardSetting(pSetting);
#endif	
	return 0;
}
    
static int
zd1205_ioctl_getiwencode(struct net_device *dev, struct iw_point *erq, char *key)


{
    struct zd1205_private *macp = dev->priv;
    card_Setting_t *pSetting = &macp->cardSetting;



    int index = (erq->flags & IW_ENCODE_INDEX) - 1;

   	zd1205_lock(macp);
    if (pSetting->EncryOnOff){
        erq->flags = IW_ENCODE_OPEN;
    }

    else {
        erq->flags = IW_ENCODE_DISABLED;
    }

    /* We can't return the key, so set the proper flag and return zero */
	erq->flags |= IW_ENCODE_NOKEY;
    memset(key, 0, 16);
    
    /* Which key do we want ? -1 -> tx index */

	if((index < 0) || (index >= 4))
		index = pSetting->EncryKeyId;

        
	erq->flags |= index + 1;
	/* Copy the key to the user buffer */

	erq->length = pSetting->WepKeyLen;
	if (erq->length > 16) {
		erq->length = 0;


	}
    zd1205_unlock(macp);         


    return 0;
}

static int
zd1205_ioctl_setessid(struct net_device *dev, struct iw_point *erq)
{
	struct zd1205_private *macp = dev->priv;
	char essidbuf[IW_ESSID_MAX_SIZE+1];


	memset(&essidbuf, 0, sizeof(essidbuf));


 	if (erq->flags) {
		if (erq->length > (IW_ESSID_MAX_SIZE+1))
 			return -E2BIG;

		if (copy_from_user(&essidbuf, erq->pointer, erq->length))
			return -EFAULT;
	}

	zd1205_lock(macp);

	//essidbuf[erq->length] = '\0';
	memcpy(&macp->cardSetting.Info_SSID[2], essidbuf, erq->length);
	macp->cardSetting.Info_SSID[1] = strlen(essidbuf);

	//memcpy(&macp->cardSetting.Info_SSID[2], essidbuf, erq->length-1);
	//macp->cardSetting.Info_SSID[1] = erq->length-1;
	zd1205_unlock(macp);

	return 0;
}
static int
zd1205_ioctl_setbssid(struct net_device *dev, struct iwreq *wrq)
{
	//struct zd1205_private *macp = dev->priv;
    memcpy(dot11DesiredBssid, &wrq->u.ap_addr.sa_data, ETH_ALEN);
	//ZD1211DEBUG(0,"set AP BSSID=" MACSTR "\n",MAC2STR(dot11DesiredBssid));
	return 0;
	
}


/**
 * zd1205_sw_init - initialize software structs
 * @macp: atapter's private data struct
 * 
 * This routine initializes all software structures. Sets up the
 * circular structures for the RFD's & TCB's. Allocates the per board
 * structure for storing adapter information. The CSR is also memory 
  * mapped in this routine.
 *
 * Returns :
 *      true: if S/W was successfully initialized

 *      false: otherwise
 */


static unsigned char

zd1205_sw_init(struct zd1205_private *macp)
{

    MP_ASSERT(0);
    //ZENTER(0);
	zd1205_init_card_setting(macp);
    zd1205_load_card_setting(macp, 1);
    zd1205_set_zd_cbs(&dot11Obj);
//	zd_CmdProcess(CMD_RESET_80211, &dot11Obj, 0);
    
	/* Initialize our spinlocks */
	spin_lock_init(&(macp->bd_lock));
    spin_lock_init(&(macp->bd_non_tx_lock));
    //spin_lock_init(&(macp->q_lock));
    spin_lock_init(&(macp->conf_lock));

#ifdef LINUX
	tasklet_init(&macp->zd1205_tasklet, zd1205_action, 0);
#endif
#if ZDCONF_AP_SUPPORT == 1
    tasklet_init(&macp->zd1205_ps_tasklet, zd1205_ps_action, 0);
#endif
#ifdef LINUX
    tasklet_init(&macp->zd1205_tx_tasklet, zd1205_tx_action, 0);
#endif
    
#ifdef HOST_IF_USB
    //spin_lock_init(&(macp->intr_lock));
    spin_lock_init(&(macp->rx_pool_lock));
    tasklet_init(&macp->zd1211_rx_tasklet, zd1211_rx_isr, (unsigned long)macp);
    tasklet_init(&macp->zd1211_tx_tasklet, zd1211_tx_isr, (unsigned long)macp);
    tasklet_init(&macp->rx_buff_tasklet, zd1211_alloc_rx, (unsigned long)macp);
#endif	


#ifdef LINUX
	macp->isolate_lock = RW_LOCK_UNLOCKED;
#endif

	macp->driver_isolated = false;
#if ZDCONF_LP_SUPPORT == 1
    dot11Obj.LP_MODE = 0;
    dot11Obj.BURST_MODE = 0;
#endif


    //ZEXIT(0);
	return 1;

}


/**
 * zd1205_hw_init - initialized tthe hardware
 * @macp: atapter's private data struct


 * @reset_cmd: s/w reset or selective reset
 *

 * This routine performs a reset on the adapter, and configures the adapter.
 * This includes configuring the 82557 LAN controller, validating and setting


 * the node address, detecting and configuring the Phy chip on the adapter,
 * and initializing all of the on chip counters.
 *
 * Returns:
 *      true - If the adapter was initialized
 *      false - If the adapter failed initialization
 */
unsigned char
zd1205_hw_init(struct zd1205_private *macp)

{

    //ZENTER(0);
	HW_ResetPhy(&dot11Obj);
    HW_InitHMAC(&dot11Obj);
	zd1205_config(macp);

    //ZEXIT(0);
	return true;
}

/////////////////////////////////////////
int
zd1205_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
#if (ZDCONF_WEXT_WPA == 1) && (WIRELESS_EXT > 17)
    int i;
    unsigned char *key;
    struct iw_pmksa *pmk;
    struct iw_encode_ext *ext;
    struct iw_mlme *mlme;
    struct zydas_wlan_param szdparm;
    struct zydas_wlan_param *zdparm = &szdparm;
#endif
	struct zd1205_private *macp;
	void *regp;
 	struct zdap_ioctl *zdreq;
	struct iwreq *wrq = (struct iwreq *)ifr;
	int err = 0;
	int changed = 0;
    card_Setting_t *pSetting = NULL;
    extern U8 UpdateBcn;

//    MP_FUNCTION_ENTER();

    MP_ASSERT((u32)dev == (u32)&g_net_device3);

	macp = dev->priv;
    MP_ASSERT((u32)macp == (u32)macp_global);
    pSetting = &macp->cardSetting;
	regp = macp->regp;
#if (ZDCONF_WEXT_WPA == 1) && (WIRELESS_EXT > 17)
    // SIOCGIWNAME and SIOCGIWRANGE are necessary for NetworkManager for 
    // judgement of WPA / Wireless supporting.
    if((cmd == SIOCGIWNAME) && !netif_running(dev))
    {
        strcpy(wrq->u.name, "802.11b/g NIC");
        return 0;
    }
    if((cmd == SIOCGIWRANGE) && !netif_running(dev))
    {
        ZD1211DEBUG(1, "%s: SIOCGIWRANGE\n", dev->name);
        if ( wrq->u.data.pointer != NULL) {
            struct iw_range range;
            err = zd1205wext_giwrange(dev, NULL, &wrq->u.data, (char *) &range);                    

            /* Push that up to the caller */
            if (copy_to_user(wrq->u.data.pointer, &range, sizeof(range)))
                err = -EFAULT;
        }
        return err;
    }
#endif

	if(!netif_running(dev))
    {
        MP_FUNCTION_EXIT();
		return -EINVAL;	
    }
    
    zdreq = kmalloc(sizeof(struct zdap_ioctl), GFP_KERNEL);
    if(zdreq == NULL)
    {
        printk("OOM in %s\n", __func__);
        return -ENOMEM;
    }
#ifdef LINUX
    down(&macp->ioctl_sem);
#endif
    printk("%s: cmd=%x\n", __func__, cmd);
	switch (cmd) {
#if (ZDCONF_WEXT_WPA == 1) && (WIRELESS_EXT > 17)
        case SIOCSIWAUTH:
            printk("Get SIOCSIWAUTH\n");
            u16 flags = wrq->u.param.flags & IW_AUTH_INDEX;
            printk("wrq->u.param.flags = %08x\n", wrq->u.param.flags);
            printk("wrq->u.param.value = %08x\n", wrq->u.param.value);
            if(flags ==  IW_AUTH_WPA_ENABLED)
            {
                macp->cardSetting.WPASupport = wrq->u.param.value;
                if(!wrq->u.param.value)
                {
                    macp->cardSetting.WPAIe[1] = 0;
                    macp->cardSetting.ap_scan=0;
                }
                else
                    macp->cardSetting.ap_scan=1;
                macp->cardSetting.ap_scan=1;
                printk("WPA=%d\n",wrq->u.param.value);
                printk("macp->cardSetting.ap_scan=%d\n",macp->cardSetting.ap_scan);
                break;
            }
            if(flags == IW_AUTH_PRIVACY_INVOKED)
            {
				if(wrq->u.param.value) {
                	mCap |= CAP_PRIVACY;
					macp->cardSetting.EncryOnOff = 1;
					WPADEBUG("enable\n");
				}
				else {
			        mCap &= ~CAP_PRIVACY;
					macp->cardSetting.EncryOnOff = 0;
					WPADEBUG("disable\n");
				}
                break;
            }
            if(flags == IW_AUTH_TKIP_COUNTERMEASURES)
            {
				if(wrq->u.param.value) {
					if(dot11Obj.MIC_CNT)
						mCounterMeasureState = 1;
					WPADEBUG("CounterMeasure Enable\n");
				}
				else {
					mCounterMeasureState = 0;
                    WPADEBUG("CounterMeasure Disable\n");
				}
                break;
            }
            if(flags == IW_AUTH_WPA_VERSION)
            {
				if(wrq->u.param.value == 1) {
                    macp->WEXT_Block = 0;
				}
				else {
                    macp->WEXT_Block = 1;
				}
                break;
            }
            if(flags == IW_AUTH_80211_AUTH_ALG)
            {
				WPADEBUG("ZD_PARAM_AUTH_ALGS: ");

				if( wrq->u.param.value == 1) {
                    macp->cardSetting.AuthMode = 0;
					printk("OPEN_SYSTEM\n");
				}
				else {
                    macp->cardSetting.AuthMode = 1;
					printk("SHARED_KEY\n");
				}
                changed = 1;
                break;
            }
#ifdef PLATFORM_MPIXEL
            if(flags ==  IW_AUTH_CIPHER_PAIRWISE)
            {
                if(!wrq->u.param.value)
                {
                    macp->cardSetting.WPASupport = 0;
                    macp->cardSetting.WPAIe[1] = 0;
                }
                else
                    macp->cardSetting.WPASupport = 1;
                printk("WPA=%d\n",wrq->u.param.value);
                changed = 1;
                break;
            }
#endif
            break;
        case SIOCSIWPMKSA:
            printk("Get SIOCSIWPMKSA\n");
            pmk = (struct iw_pmksa *)wrq->u.data.pointer;
            printk("PMK CMD=%08x\n",pmk->cmd);
            printk("BSSID=");
            for(i=0;i<6;i++)
                printk("%02x ",pmk->bssid.sa_data[i] & 0xFF);
            printk("\n");
            printk("PMKID=");
            for(i=0;i<16;i++)
                printk("%02x ",pmk->pmkid[i] & 0xFF);
            printk("\n");
            break;
        case SIOCSIWGENIE:
            printk("Get SIOCSIWGENIE\n");
            key = (unsigned char *)(wrq->u.data.pointer);
            memset(zdparm,0,sizeof(struct zydas_wlan_param));
            zdparm->cmd = ZD_CMD_SET_GENERIC_ELEMENT;
            zdparm->u.generic_elem.len = wrq->u.data.length;
            memcpy(zdparm->u.generic_elem.data,key,wrq->u.data.length);
                
            zd1205_wpa_ioctl(macp, zdparm);
#ifdef PLATFORM_MPIXEL
            changed = 1;
#endif
            break;
        case SIOCSIWMLME:
            mlme = (struct iw_mlme *)wrq->u.data.pointer;    
            memset(zdparm,0,sizeof(struct zydas_wlan_param));
            if(mlme->cmd == IW_MLME_DEAUTH)
                zdparm->cmd = MLME_STA_DEAUTH;
            else if(mlme->cmd == IW_MLME_DISASSOC)
                zdparm->cmd =  MLME_STA_DISASSOC;
            else
                printk("Unknown MLME command = %d\n",mlme->cmd);
            memcpy(zdparm->sta_addr, mlme->addr.sa_data,ETH_ALEN);
            zdparm->u.mlme.reason_code = mlme->reason_code;
            zd1205_wpa_ioctl(macp, zdparm);
            break;
        case SIOCSIWENCODEEXT:
            ext = (struct iw_encode_ext *)wrq->u.encoding.pointer;
            key = (unsigned char *)(((unsigned long)wrq->u.encoding.pointer)+sizeof(struct iw_encode_ext));
            printk("Get SIOCSIWENCODEEXT\n");
            printk("wrq->u.encoding.falgs = %08x\n",wrq->u.encoding.flags);
            printk("ext->ext_flags = %08x\n",ext->ext_flags);
            printk("ext->addr.sa_data=");
            for(i=0;i<6;i++)
                printk("%02x ",ext->addr.sa_data[i] & 0xFF);
            printk("\n");
            printk("ext->key_len=%d\n",ext->key_len);
            printk("Key=");
            for(i=0;i<ext->key_len;i++)
                printk("%02x ",key[i] & 0xFF); 
            printk("\n");
            printk("ext->alg = %08x\n",ext->alg);

            memset(zdparm, 0, sizeof(struct zydas_wlan_param));
            zdparm->cmd = ZD_CMD_SET_ENCRYPT_KEY;
            switch(ext->alg)
            {
                case 0:
                    strcpy(zdparm->u.crypt.alg,"NONE");
                    break;
                case 1:
                    strcpy(zdparm->u.crypt.alg,"WEP");
                    break;
                case 2:
                    strcpy(zdparm->u.crypt.alg,"TKIP");
                    break;
                case 3:
                    strcpy(zdparm->u.crypt.alg,"CCMP");
                    break;
                default:
                    printk("adsfasdfasdfasdfasdfdddd\n");
                    return;
            }
            //zdparm->u.crypt.alg = ext->alg;
            zdparm->u.crypt.idx = wrq->u.encoding.flags & 3;
            if(zdparm->u.crypt.idx == 0)
                printk("!? zdparm->u.crypt.idx == 0\n");
            else
                zdparm->u.crypt.idx--;
            zdparm->u.crypt.flags =  wrq->u.encoding.flags;
            zdparm->u.crypt.key_len = ext->key_len;
            memcpy(zdparm->sta_addr,ext->addr.sa_data,6);
            memcpy(zdparm->u.crypt.key,key,ext->key_len);
            zd1205_wpa_ioctl(macp, zdparm);
            
            break;
#endif
		case SIOCGIWNAME:
			ZD1211DEBUG(1, "%s: SIOCGIWNAME\n", dev->name);
			//strcpy(wrq->u.name, "IEEE 802.11-DS");
			strcpy(wrq->u.name, "802.11b/g NIC");
			break;

		case SIOCGIWAP:
			ZD1211DEBUG(1, "%s: SIOCGIWAP\n", dev->name);
			wrq->u.ap_addr.sa_family = ARPHRD_ETHER;

#if ZDCONF_AP_SUPPORT == 1
			if (macp->cardSetting.BssType == AP_BSS)
				memcpy(wrq->u.ap_addr.sa_data, macp->macAdr, 6);
			else 
#endif
            {
				if(macp->bAssoc)
					memcpy(wrq->u.ap_addr.sa_data, macp->BSSID, 6);
				else
					memset(wrq->u.ap_addr.sa_data, 0, 6);
			}
			break;

		case SIOCGIWRANGE:
			ZD1211DEBUG(1, "%s: SIOCGIWRANGE\n", dev->name);
			if ( wrq->u.data.pointer != NULL) {
				struct iw_range range;
				err = zd1205wext_giwrange(dev, NULL, &wrq->u.data, (char *) &range);                    

				/* Push that up to the caller */
				if (copy_to_user(wrq->u.data.pointer, &range, sizeof(range)))
					err = -EFAULT;
			}
			break;

		case SIOCSIWMODE:
			ZD1211DEBUG(0, "%s: SIOCSIWMODE\n", dev->name);
			err = zd1205wext_siwmode(dev, NULL, &wrq->u.mode, NULL);

			if (!err)
				changed = 1;
			break;

		case SIOCGIWMODE:
			ZD1211DEBUG(1, "%s: SIOCGIWMODE\n", dev->name);
			err = zd1205wext_giwmode(dev, NULL, &wrq->u.mode, NULL);
			break;

		case SIOCSIWENCODE:
		{
			char keybuf[MAX_KEY_SIZE];
			ZD1211DEBUG(0, "%s: SIOCSIWENCODE\n", dev->name);

			if (wrq->u.encoding.pointer){
				if (wrq->u.encoding.length > MAX_KEY_SIZE){
					err = -E2BIG;
					break;
				}
				
				if (copy_from_user(keybuf, wrq->u.encoding.pointer, wrq->u.encoding.length)) {
					err = -EFAULT;
					break;
				}
			}

			zd1205_dump_data("keybuf", keybuf, wrq->u.encoding.length);         
			err = zd1205_ioctl_setiwencode(dev, &wrq->u.encoding, keybuf);

			if (!err)
			changed = 0;
		}
		break;

		case SIOCGIWENCODE:
		{
			char keybuf[MAX_KEY_SIZE];
		
			ZD1211DEBUG(1, "%s: SIOCGIWENCODE\n", dev->name);
			err = zd1205_ioctl_getiwencode(dev, &wrq->u.encoding, keybuf);

			if (wrq->u.encoding.pointer){
				if (copy_to_user(wrq->u.encoding.pointer, keybuf, wrq->u.encoding.length))
					err = -EFAULT;
			}    
		}    
		break;

		case SIOCSIWESSID:
			//ZD1211DEBUG(0, "%s: SIOCSIWESSID\n", dev->name);
			err = zd1205_ioctl_setessid(dev, &wrq->u.essid);	
			if (!err && macp->cardSetting.ap_scan != 1)
            {
			    changed = 1;
			    ZD1211DEBUG(0, "%s: SIOCSIWESSID,ap_scan=%d\n", dev->name,macp->cardSetting.ap_scan);
            }
			break;
		case SIOCSIWAP:
			//ZD1211DEBUG(0, "%s: SIOCSIWAP\n", dev->name);
			err = zd1205_ioctl_setbssid(dev, wrq);
			if (!err && macp->cardSetting.ap_scan == 1)
			{
             	//set_bit(CTX_FLAG_ESSID_WAS_SET,(void*)&macp->flags);
				changed = 1;
			    ZD1211DEBUG(0, "%s: SIOCSIWAP,ap_scan=%d\n", dev->name,macp->cardSetting.ap_scan);
			}

			break;

		case SIOCGIWESSID:
			ZD1211DEBUG(1, "%s: SIOCGIWESSID\n", dev->name);
			err = zd1205_ioctl_getessid(dev, &wrq->u.essid);
			break;

#ifdef LINUX
		case SIOCGIWFREQ:
			ZD1211DEBUG(1, "%s: SIOCGIWFREQ\n", dev->name);
			wrq->u.freq.m = zd1205_hw_get_freq(macp);
			wrq->u.freq.e = 1;
			break;
#endif
		case SIOCSIWFREQ:
			ZD1211DEBUG(0, "%s: SIOCSIWFREQ\n", dev->name);
			
			err = zd1205_ioctl_setfreq(dev, &wrq->u.freq);
			//if (!err)
			//	changed = 1;
			break;

		case SIOCGIWRTS:
			ZD1211DEBUG(1, "%s: SIOCGIWRTS\n", dev->name);
			zd1205wext_giwrts(dev, NULL, &wrq->u.rts, NULL);
			break;

		case SIOCSIWRTS:
			ZD1211DEBUG(1, "%s: SIOCSIWRTS\n", dev->name);


			err = zd1205_ioctl_setrts(dev, &wrq->u.rts);
			if (! err)
				changed = 1;
			break;

		case SIOCSIWFRAG:
			ZD1211DEBUG(1, "%s: SIOCSIWFRAG\n", dev->name);
			
			err = zd1205_ioctl_setfrag(dev, &wrq->u.frag);
			if (! err)
				changed = 1;
			break;

		case SIOCGIWFRAG:
			ZD1211DEBUG(1, "%s: SIOCGIWFRAG\n", dev->name);
			err = zd1205_ioctl_getfrag(dev, &wrq->u.frag);
			break;

		case SIOCSIWRATE:
			ZD1211DEBUG(1, "%s: SIOCSIWRATE\n", dev->name);
			
			err = zd1205_ioctl_setrate(dev, &wrq->u.bitrate);
			if (! err)
				changed = 1;

			break;

		case SIOCGIWRATE:
			ZD1211DEBUG(1, "%s: SIOCGIWRATE\n", dev->name);
			err = zd1205_ioctl_getrate(dev, &wrq->u.bitrate);
			break;

		case SIOCSIWPOWER:
			ZD1211DEBUG(1, "%s: SIOCSIWPOWER\n", dev->name);

			//err = zd1205_ioctl_setpower(dev, &wrq->u.power);
            err = -ENOTSUPP;
			if (!err)
				changed = 0;
			break;


		case SIOCGIWPOWER:
			ZD1211DEBUG(1, "%s: SIOCGIWPOWER\n", dev->name);
			err = zd1205_ioctl_getpower(dev, &wrq->u.power);
			break;

#if WIRELESS_EXT > 10
		case SIOCSIWRETRY:
			ZD1211DEBUG(1, "%s: SIOCSIWRETRY\n", dev->name);
			err = -EOPNOTSUPP;
			break;


		case SIOCGIWRETRY:
			ZD1211DEBUG(1, "%s: SIOCGIWRETRY\n", dev->name);
			err = zd1205_ioctl_getretry(dev, &wrq->u.retry);
			break;
#endif /* WIRELESS_EXT > 10 */                                          
		case SIOCGIWPRIV:
			if (wrq->u.data.pointer) {
				struct iw_priv_args privtab[] = {
				{ SIOCIWFIRSTPRIV + 0x0, 0, 0, "list_bss" },
				{ SIOCIWFIRSTPRIV + 0x1, 0, 0, "card_reset" },
				{ SIOCIWFIRSTPRIV + 0x2, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_auth" },  /* 0 - open, 1 - shared key */
				{ SIOCIWFIRSTPRIV + 0x3, 0, IW_PRIV_TYPE_CHAR | 12, "get_auth" },
				{ SIOCIWFIRSTPRIV + 0x4, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_preamble" },  /* 0 - long, 1 - short */      
				{ SIOCIWFIRSTPRIV + 0x5, 0, IW_PRIV_TYPE_CHAR | 6, "get_preamble" },
				{ SIOCIWFIRSTPRIV + 0x6, 0, 0, "cnt" },
				{ SIOCIWFIRSTPRIV + 0x7, 0, 0, "regs" },
				{ SIOCIWFIRSTPRIV + 0x8, 0, 0, "probe" },
//				{ SIOCIWFIRSTPRIV + 0x9, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "dbg_flag" },
				{ SIOCIWFIRSTPRIV + 0xA, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "connect" },
                { SIOCIWFIRSTPRIV + 0xF , IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "lp_mode" },
				{ SIOCIWFIRSTPRIV + 0xB, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_mac_mode" },
				{ SIOCIWFIRSTPRIV + 0xC, 0, IW_PRIV_TYPE_CHAR | 12, "get_mac_mode" },
//				{ SIOCIWFIRSTPRIV + 0xD, 0, 0, "save_conf" },
//			    { SIOCIWFIRSTPRIV + 0xF, 0, IW_PRIV_TYPE_CHAR | 14, "get_Region" },
#if ZDCONF_REGION_CONFIG == 1
			    { SIOCIWFIRSTPRIV + 0x9,IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_Region" },
#endif
 				};

				err = access_ok(VERIFY_WRITE, wrq->u.data.pointer, sizeof(privtab));
				if (err)
					break;
			
				wrq->u.data.length = sizeof(privtab) / sizeof(privtab[0]);
 				if (copy_to_user(wrq->u.data.pointer, privtab, sizeof(privtab)))
					err = -EFAULT;
			}
			break;

#ifdef LINUX
		case SIOCIWFIRSTPRIV + 0x0: /* list_bss */
			ZD1211DEBUG(1, "%s: SIOCIWFIRSTPRIV + 0x0 (list_bss)\n", dev->name);
			macp->bss_index = zd_GetBssList(&macp->BSSInfo[0]);
			zd1205_list_bss(macp);
			break;
#endif

		case SIOCIWFIRSTPRIV + 0x1: /* card_reset */
			ZD1211DEBUG(1, "%s: SIOCIWFIRSTPRIV + 0x1 (card_reset)\n", dev->name);
#ifdef LINUX
			if (! capable(CAP_NET_ADMIN)) {
				err = -EPERM;
				break;
			}
#endif
		
			printk(KERN_DEBUG "%s: Force scheduling reset!\n", dev->name);
            defer_kevent(macp, KEVENT_USB_DEVICE_RESET);
			err = 0;
			break;

		case SIOCIWFIRSTPRIV + 0x2: /* set_auth */
			ZD1211DEBUG(1, "%s: SIOCIWFIRSTPRIV + 0x2 (set_auth)\n", dev->name);
#ifdef LINUX
			if (! capable(CAP_NET_ADMIN)) {
				err = -EPERM;
				break;
			}
#endif
			{
				int val = *( (int *) wrq->u.name );
				if ((val < 0) || (val > 1)){
					err = -EINVAL;
					break;
				}    
				else {    
					zd1205_lock(macp);
					macp->cardSetting.AuthMode = val;
					zd1205_unlock(macp);
					err = 0;
					changed = 1;
				}
			}
			break;

		case SIOCIWFIRSTPRIV + 0x3: /* get_auth */
			ZD1211DEBUG(1, "%s: SIOCIWFIRSTPRIV + 0x3 (get_auth)\n", dev->name);

			if (wrq->u.data.pointer){
				wrq->u.data.flags = 1;

				if (macp->cardSetting.AuthMode == 0) {
					wrq->u.data.length = 12;
					
					if (copy_to_user(wrq->u.data.pointer, "open system", 12)){
						err = -EFAULT;
                        break;
					}
				}
				else if (macp->cardSetting.AuthMode == 1){
					wrq->u.data.length = 11;

					if (copy_to_user(wrq->u.data.pointer, "shared key", 11)){
						err = -EFAULT;
                        break;
					}
				}
				else if (macp->cardSetting.AuthMode == 2){
					wrq->u.data.length = 10;

					if (copy_to_user(wrq->u.data.pointer, "auto mode", 10)){
						err = -EFAULT;
                        break;
					}
				}
				else {
					err = -EFAULT;
                    break;
				}
			}
			break;

		case SIOCIWFIRSTPRIV + 0x4: /* set_preamble */
			ZD1211DEBUG(1, "%s: SIOCIWFIRSTPRIV + 0x4 (set_preamble)\n", dev->name);

#ifdef LINUX
			if (! capable(CAP_NET_ADMIN)) {
				err = -EPERM;
 				break;
			}
#endif
			{
				int val = *( (int *) wrq->u.name );
                
				if ((val < 0) || (val > 1)){
					err = -EINVAL;
					break;
				}
				else {    
					zd1205_lock(macp);

					if (val)
						mPreambleType = macp->cardSetting.PreambleType = 1;
					else
						mPreambleType = macp->cardSetting.PreambleType = 0;
					
					zd1205_unlock(macp);
					err = 0;
					changed = 0;
				}    
			}
			break;


		case SIOCIWFIRSTPRIV + 0x5: /* get_preamble */
			ZD1211DEBUG(1, "%s: SIOCIWFIRSTPRIV + 0x5 (get_preamble)\n", dev->name);

			if (wrq->u.data.pointer){
				wrq->u.data.flags = 1;

				if (macp->cardSetting.PreambleType){
					wrq->u.data.length = 6;

					if (copy_to_user(wrq->u.data.pointer, "short", 6)){
						err = -EFAULT;
                        break;
					}
				}
				else {
					wrq->u.data.length = 5;
					
					if (copy_to_user(wrq->u.data.pointer, "long", 5)){
						err = -EFAULT;
                        break;
					}  
				}
			}            
			break;

		case SIOCIWFIRSTPRIV + 0x6: /* dump_cnt */
			ZD1211DEBUG(1, "%s: SIOCIWFIRSTPRIV + 0x6 (dump_cnt)\n", dev->name);
			zd1205_dump_cnters(macp);
			break;

		case SIOCIWFIRSTPRIV + 0x7: /* dump_reg */
			ZD1211DEBUG(1, "%s: SIOCIWFIRSTPRIV + 0x7 (dump_cnt)\n", dev->name);
			zd1205_dump_regs(macp);
			break;

		case SIOCIWFIRSTPRIV + 0x8: /* probe */
			ZD1211DEBUG(1, "%s: SIOCIWFIRSTPRIV + 0x8 (probe)\n", dev->name);
//			zd_CmdProcess(CMD_PROBE_REQ, 0, 0);
			break;

//		case SIOCIWFIRSTPRIV + 0x9: /* set_dbgflag */
//			ZD1211DEBUG(1, "%s: SIOCIWFIRSTPRIV + 0x9 (set_dbgflag)\n", dev->name);

//			if (! capable(CAP_NET_ADMIN)) {
//				err = -EPERM;
//				break;
//			}
//			{
//				int val = *( (int *) wrq->u.name );

//				if ((val < 0) || (val > 5)){
//					err = -EINVAL;
//					break;
//				}
//				else {
//					zd1205_lock(macp);
//					macp->dbg_flag = val;
//					zd1205_unlock(macp);
//					err = 0;
//				}
//			}
//			break;

#if ZDCONF_IWPRIV_CONNECT == 1
		case SIOCIWFIRSTPRIV + 0xA: // connect 
			ZD1211DEBUG(1, "%s: SIOCIWFIRSTPRIV + 0xA (connect)\n", dev->name);
#if LINUX
			if (! capable(CAP_NET_ADMIN)) {
				err = -EPERM;
				break;
			}
#endif			
			{
                int val = 1;//*( (int *) wrq->u.name );
#if LINUX
				if ((val < 1) || (val >macp->bss_index)){
					err = -EINVAL;
					break;
				}
				else 
#endif					
                {
                	U8  bssTypeToConnect;
                    U16 capabilities;
					u8 ChangeToMacMode=MIXED_MODE;
                    capabilities = CAP_IBSS;//macp->BSSInfo[val-1].cap;
#if LINUX                
                    capabilities = macp->BSSInfo[val-1].cap;
					//If you connect to non-A AP while in 5G Band, or
					//you connect to A AP while in 2.4G, you need to 
					//do mac_mode change first
					if((PURE_A_AP == macp->BSSInfo[val-1].apMode  && 
						PURE_A_MODE != mMacMode) || 
					   (PURE_A_AP != macp->BSSInfo[val-1].apMode  &&
						PURE_A_MODE == mMacMode) )
					{
						if(PURE_A_AP == macp->BSSInfo[val-1].apMode)
							ChangeToMacMode = PURE_A_MODE;

						printk("Changed macmode in connect\n");
                        macp->cardSetting.Channel = 8;//Default Channel to 8
                    	macp->cardSetting.MacMode = ChangeToMacMode;
                  	    macp->bDefaultIbssMacMode=1;
						//set_mac_mode command has been issued by the user.
                    	zd1205_SetRatesInfo(macp);
                    	err = 0;
						zd_UpdateCardSetting(&macp->cardSetting);
					}
#endif					
                    if (capabilities & (CAP_IBSS | CAP_ESS)) {
				    	zd1205_lock(macp);
#if Make_ADHOC
                       	memcpy((U8*)&mSsid,(U8*)macp->cardSetting.Info_SSID,34);
						mpDebugPrint("mSsid %s",mSsid);
						memcpy((U8*)&dot11DesiredSsid, &mSsid, 34);
						memcpy((U8*)&mBssInfo[mBssIndex].ssid, &mSsid, 34);
#else						
                       	memcpy((U8*)&mSsid,(U8*)macp->BSSInfo[val-1].ssid,34);
                       	memcpy((U8*)&dot11DesiredSsid, &mSsid, 34);
                       	macp->BSSInfo[val-1].ssid[mSsid.buf[1]+2]=0;
#endif
                       	mProbeWithSsid=TRUE;
                        if (capabilities & CAP_IBSS) {
#if ZDCONF_ADHOC_SUPPORT == 1
                        	if (macp->bDefaultIbssMacMode==0)
                            	//mMacMode=macp->cardSetting.MacMode=PURE_B_MODE;
                            	mMacMode=macp->cardSetting.MacMode=MIXED_MODE;
                            bssTypeToConnect=INDEPENDENT_BSS;
#endif
                        }
                        else {
                        	if (macp->bDefaultIbssMacMode==0)
                           		mMacMode=macp->cardSetting.MacMode=MIXED_MODE;
                            bssTypeToConnect=INFRASTRUCTURE_BSS;
                        }
                        mBssType=macp->cardSetting.BssType=bssTypeToConnect;
						zd_CmdProcess(CMD_CONNECT, &bssTypeToConnect, val);
				        zd1205_unlock(macp);
                     }
				     err = 0;
				}
			}	
			break;
#endif

#if ZDCONF_LP_SUPPORT == 1
        case SIOCIWFIRSTPRIV + 0xF: //LP_MODE
        {
            int val = *((int *)wrq->u.name);
            if( val == 5) {
                dot11Obj.BURST_MODE = 0;
                printk("Burst is set 0\n");
            }
            else if(val == 4) {
                dot11Obj.BURST_MODE = 1;
                printk("Burst is set 1\n");
            }

            else if(val == 3) {
                LP_CNT_SHOW();
            }
            else if (val == 2) {
                printk("Current LP Mode = %d\n", dot11Obj.LP_MODE);
            }
            else if(val == 1) {
                if(macp->cardSetting.FragThreshold < 4000)
                {
                    printk("You can't turn on LP_Mode when fragment is on\n");
                    printk("issue iwconfig ethX frag off to turn it off\n");
                    err = -EINVAL;
                    break;
                }
                zd1205_lock(macp);
                dot11Obj.LP_MODE = 1;	
                mod_timer(&(macp->tm_lp_poll_id), jiffies + (1*HZ)/100);
                zd1205_unlock(macp);
                printk("LP_MODE set 1\n");

            }
            else if(val == 0) {
                dot11Obj.LP_MODE = 0;
                del_timer_sync(&macp->tm_lp_poll_id);
                printk("LP_MODE set 0\n");
            }	
            else
            {
                err = -EFAULT;
                break;
            }
        }
        break;
#endif

		case SIOCIWFIRSTPRIV + 0xB: /* set_mac_mode */
		MP_ASSERT(0);
			ZD1211DEBUG(1, "%s: SIOCIWFIRSTPRIV + 0xB (set_mac_mode)\n", dev->name);

#ifdef LINUX
			if (! capable(CAP_NET_ADMIN)) {
				err = -EPERM;
				break;
			}
#endif
			{
				int val = *( (int *) wrq->u.name );
				int mac_mode_limit;

				MP_ASSERT(0);
				if(AL7230B_RF == dot11Obj.rfMode)
					mac_mode_limit = 4; //4 = A,B,G
				else if (AL2230_RF == dot11Obj.rfMode)	
					mac_mode_limit = 3; //3 = B,G
                else if (AL2230S_RF == dot11Obj.rfMode)
                    mac_mode_limit = 3;
                else if (UW2453_RF == dot11Obj.rfMode)
                    mac_mode_limit = 3;
                else if (AR2124_RF == dot11Obj.rfMode)
                    mac_mode_limit = 3;
				else {
					printk("Unknown RF Module. You are not allowed to set mac mode\n");
					mac_mode_limit = 0;
				}
				if ((val < 1) || (val > mac_mode_limit)){
					err = -EINVAL;
					break;
				}
				else {
					//If Band changed from 2.4G <-> 5G, we need 
					//to set default channel
					if( (macp->cardSetting.MacMode != PURE_A_MODE && 
						val == PURE_A_MODE))
					{ 
						macp->cardSetting.Channel = 36;
					}
					else if(macp->cardSetting.MacMode == PURE_A_MODE &&
						val != PURE_A_MODE) {
						macp->cardSetting.Channel = 1;	
					}
#if ZDCONF_ADHOC_SUPPORT == 1
					macp->IBSS_DesiredMacMode = val;	
                    macp->IBSS_DesiredChannel = macp->cardSetting.Channel;
#endif
					macp->cardSetting.MacMode = val;
                    macp->bDefaultIbssMacMode=1;// Indicates that the set_mac_mode command has been issued by the user.
					zd1205_SetRatesInfo(macp);
					err = 0;
					changed = 1;
				}
			}
			break;

		case SIOCIWFIRSTPRIV + 0xC: /* get_mac_mode */
        {
			ZD1211DEBUG(1, "%s: SIOCIWFIRSTPRIV + 0xC (get_mac_mode)\n", dev->name);
            //zd1211_submit_tx_urb();
            //zd1205_recycle_tx(macp);

/*            
             zd1205_Ctrl_Set_t *pCtrlSet;

            zd1205_SwTcb_t  *sw_tcb;
            zd1205_TBD_t *Tbd;
            int i;
            if(macp->activeTxQ->count)
            {
                sw_tcb = macp->activeTxQ->first;
                pCtrlSet = sw_tcb->pHwCtrlPtr;
                Tbd = sw_tcb->pFirstTbd;
                Tbd++;
                printk("##### Control Setting #####\n");
                for(i=0;i<24;i++) 
                    printk("%02x ", *((U8 *)pCtrlSet+i));
                printk("\n");
                printk("##### MAC Header #####\n");
                for(i=0;i<24;i++)
                    printk("%02x ", *(U8 *)(Tbd->TbdBufferAddrLowPart+i));
                printk("\n");

            }     
*/        
			if (wrq->u.data.pointer){
				wrq->u.data.flags = 1;

				if (macp->cardSetting.MacMode == MIXED_MODE){
					wrq->u.data.length = 11;
					if (copy_to_user(wrq->u.data.pointer, "Mixed Mode", 11)){
						err = -EFAULT;
                        break;
					}
				}
				else if (macp->cardSetting.MacMode == PURE_G_MODE){
					wrq->u.data.length = 12;
					if (copy_to_user(wrq->u.data.pointer, "Pure G Mode", 12)){
						err = -EFAULT;
                        break;
					}
				}
				else if (macp->cardSetting.MacMode == PURE_B_MODE){
					wrq->u.data.length = 12;
					if (copy_to_user(wrq->u.data.pointer, "Pure B Mode", 12)){
						err = -EFAULT;
                        break;
					}
				}
				else if (macp->cardSetting.MacMode == PURE_A_MODE) {
					wrq->u.data.length = 12;
                    if (copy_to_user(wrq->u.data.pointer, "Pure A Mode", 12)){
                        err = -EFAULT;
                        break;
                    }
				}
				else
                {
					err = -EFAULT;
                    break;
                }
			}
			break;
            }
/*
		case SIOCIWFIRSTPRIV + 0xD: // save_conf 
			ZD1211DEBUG(1, "%s: SIOCIWFIRSTPRIV + 0xD (save_conf)\n", dev->name);
			zd1205_save_card_setting(macp);
			break;

		case SIOCIWFIRSTPRIV + 0xE: // load_conf 
			ZD1211DEBUG(1, "%s: SIOCIWFIRSTPRIV + 0xE (load_conf)\n", dev->name);
			zd1205_load_card_setting(macp, 0);
			break;
		case SIOCIWFIRSTPRIV + 0xF: //get_Region
			//zd1205_dumpEEPROM(macp);
            if (wrq->u.data.pointer){
                wrq->u.data.flags = 1;

                if (ZD_REGION_USA == dot11Obj.RegionCode){
                    wrq->u.data.length = 3;
                    if (copy_to_user(wrq->u.data.pointer, "USA", 4))
                        return -EFAULT;
                }
                else if (ZD_REGION_Europe == dot11Obj.RegionCode){
                    wrq->u.data.length = 13;
                    if (copy_to_user(wrq->u.data.pointer, "Taiwan/Europe", 14))
                        return -EFAULT;
                }
                else if (ZD_REGION_France == dot11Obj.RegionCode){
                    wrq->u.data.length = 6;
                    if (copy_to_user(wrq->u.data.pointer, "France", 7))
                        return -EFAULT;
                }
				else if (ZD_REGION_Japan == dot11Obj.RegionCode){
                    wrq->u.data.length = 5;

                    if (copy_to_user(wrq->u.data.pointer, "Japan", 6))
                        return -EFAULT;
                }
				else if (ZD_REGION_Israel == dot11Obj.RegionCode){
                    wrq->u.data.length = 6;
                    if (copy_to_user(wrq->u.data.pointer, "Israel", 7))
                        return -EFAULT;
                }
				else if (ZD_REGION_Mexico == dot11Obj.RegionCode){
                    wrq->u.data.length = 6;
                    if (copy_to_user(wrq->u.data.pointer, "Mexico", 7))
                        return -EFAULT;
                }
                else
                    return -EFAULT;
            }

            break;
*/
#if ZDCONF_REGION_CONFIG == 1
		case SIOCIWFIRSTPRIV + 0x9 : //set_Region
		{
                int val = *( (int *) wrq->u.name );

                if ((val < 1) || (val > 6)){
                    err = -EINVAL;
                    break;
                }
                else {
                    switch(val) {
						case 1 : macp->RegionCode = ZD_REGION_USA;break;
						case 2 : macp->RegionCode = ZD_REGION_Europe;break;
                        case 3 : macp->RegionCode = ZD_REGION_France;break;
                        case 4 : macp->RegionCode = ZD_REGION_Japan;break;
                        case 5 : macp->RegionCode = ZD_REGION_Israel;break;
                        case 6 : macp->RegionCode = ZD_REGION_Mexico;break;
					}
				}
				dot11Obj.RegionCode = macp->RegionCode;
				switch(val) {
                	case 1 : dot11Obj.AllowedChannel = 0x107ff;break;//1-11
                	case 2 : dot11Obj.AllowedChannel = 0x11fff;break;//1-13
                	case 3 : dot11Obj.AllowedChannel = 0xa1e00;break;//10-13
                	case 4 : dot11Obj.AllowedChannel = 0x13fff;break;//1-14
                	case 5 : dot11Obj.AllowedChannel = 0x301fc;break;//3-9
                	case 6 : dot11Obj.AllowedChannel = 0xa0600;break;//10,11
                }

			break;
		}
#endif
		////////////////////////////
        case ZDAPIOCTLGET:    //ZD1202 debug command
            //copy_to_user(((struct zdap_ioctl *)ifr->ifr_data)->data, "HelloKitty", 11);
            if( macp->DataReady == 1)
            {
                if (copy_to_user(ifr->ifr_data, &macp->zdreq, sizeof(struct zdap_ioctl)))
                    printk("copy to user fail\n");
                macp->DataReady = 0;
            }
            else
                err = -EAGAIN;
    
                
            break;
        case ZDAPIOCTL:    //ZD1202 debug command
            if (copy_from_user(zdreq, ifr->ifr_data, sizeof (struct zdap_ioctl)))
            {
                printk(KERN_ERR "ZDAPIOCTL: copy_from_user error\n");
                err = -EFAULT;
                break;
            }


			//printk(KERN_DEBUG "zd1211: cmd = %2x, reg = 0x%04x, value = 0x%08x\n", zdreq.cmd, zdreq.addr, zdreq.value);

		//	zd1205_lock(macp);
    #if 0//(LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
            if (in_interrupt())
    #else
            if (in_atomic())
    #endif
            {
                printk("!! IOCTL in Interrupt Context !!\n");
                mmcp_memcpy(&macp->zdreq, zdreq, sizeof(struct zdap_ioctl));
                defer_kevent(macp, KEVENT_ZD_IOCTL);
            }
            else
                zd1205_zd_dbg_ioctl(macp, zdreq, &changed);
		//	zd1205_unlock(macp);
			err = 0;
			break;    
#if PRODUCTION == 1
        case ZDPRODUCTIOCTL:
            zdproduction_ioctl(macp, (struct zd_point *)&wrq->u.data);
            err = 0;
            break;
#endif
		case ZD_IOCTL_WPA:
			if (copy_from_user(&macp->zd_wpa_req, ifr->ifr_data, sizeof(struct zydas_wlan_param))){
				printk("ZD_IOCTL_WPA: copy_from_user error\n");
				err = -EFAULT;
                break;
			}

			zd1205_lock(macp);
//In some kernels, the ioctl service is in atomic.
//So, we need to schedule it in kevent.
//defer_kevent may cause wpa problem in slow system.
    #if 0 //(LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
			if (in_interrupt())
    #else
			if (in_atomic())
    #endif
			{
				defer_kevent(macp, KEVENT_ZD_WPA_IOCTL);
				printk("defer WPA_IOCTL may result in slow key installation in WPA!!!\n");
				printk("Or zd_wpa_req is overwritten in pairwise / group key installation in WPA2!!!\n");
			}
			else
			{
				zd1205_wpa_ioctl(macp, &macp->zd_wpa_req);
			}
			zd1205_unlock(macp);
			err = 0;
			break;

		case ZD_IOCTL_PARAM:
		{
			int *p;
			int op;
			int arg;

			/* Point to the name field and retrieve the
			 * op and arg elements.			 */			
			p = (int *)wrq->u.name;
			op = *p++;
			arg = *p;

			if(op == ZD_PARAM_COUNTERMEASURES) {
				if(arg) {
					if(dot11Obj.MIC_CNT)
						mCounterMeasureState = 1;
					WPADEBUG("CounterMeasure Enable\n");
				}
				else {
					mCounterMeasureState = 0;
                    WPADEBUG("CounterMeasure Disable\n");
				}
			}
			if(op == ZD_PARAM_ROAMING) {
				macp->cardSetting.ap_scan=(U8)arg;
				WPADEBUG("************* ZD_PARAM_ROAMING: %d\n", arg);
			}
			if(op == ZD_PARAM_WPS_FILTER) {
				macp->wps_filter = (U8)arg;
				WPADEBUG("************* ZD_PARAM_WPS_FILTER: %d\n", arg);
			}
			if(op == ZD_PARAM_PRIVACY) {
				WPADEBUG("ZD_IOCTL_PRIVACY: ");

				/* Turn on the privacy invoke flag */
				if(arg) {
                	mCap |= CAP_PRIVACY;
					macp->cardSetting.EncryOnOff = 1;
					WPADEBUG("enable\n");
				}
				else {
			        mCap &= ~CAP_PRIVACY;
					macp->cardSetting.EncryOnOff = 0;
					WPADEBUG("disable\n");
				}
			}
			if(op == ZD_PARAM_WPA) {
				WPADEBUG("ZD_PARAM_WPA: ");
				
				if(arg && (arg != 3)) {
					WPADEBUG("enable\n");
					macp->cardSetting.WPASupport = 1;
				}
				else {
					/* Reset the WPA related variables */
					WPADEBUG("disable\n");
					macp->cardSetting.WPASupport = 0;

					/* Now we only set the length in the WPA IE
					 * field to zero.                         */
					macp->cardSetting.WPAIe[1] = 0;
				}
			}
			if(op == ZD_PARAM_COUNTERMEASURES) {
				WPADEBUG("ZD_PARAM_COUNTERMEASURES: ");
				
				if(arg) {
					WPADEBUG("enable\n");
				}
				else {
					WPADEBUG("disable\n");
				}
			}
			if(op == ZD_PARAM_DROPUNENCRYPTED) {
				WPADEBUG("ZD_PARAM_DROPUNENCRYPTED: ");
				
				if(arg) {
					WPADEBUG("enable\n");
				}
				else {
					WPADEBUG("disable\n");
				}
			}
			if(op == ZD_PARAM_AUTH_ALGS) {
				WPADEBUG("ZD_PARAM_AUTH_ALGS: ");

				if(arg == 0) {
                    macp->cardSetting.AuthMode = 0;
					WPADEBUG("OPEN_SYSTEM\n");
				}
				else {
                    macp->cardSetting.AuthMode = 1;
					WPADEBUG("SHARED_KEY\n");
				}
			}
		}
			err = 0;
			break;
#ifdef HOSTAPD_SUPPORT
        case ZD_IOCTL_GETWPAIE:
        {
            struct ieee80211req_wpaie req_wpaie;
            Hash_t *pHash;

            if(copy_from_user(&req_wpaie, ifr->ifr_data, sizeof(struct ieee80211req_wpaie)))
            {
                printk("zd1211: copy from user error\n");
                err = -EFAULT;
                break;
            }
            pHash = HashSearch((MacAddr_t *)req_wpaie.wpa_macaddr);
            if(pHash == NULL)
            {
                int i;
                for(i=0;i<16;i++)
                    if(sstByAid[i]->bValid)
                        printk("MAC:%02x %02x %02x %02x %02x %02x\n",
                            (U8)sstByAid[i]->mac[0],
                            (U8)sstByAid[i]->mac[1],
                            (U8)sstByAid[i]->mac[2],
                            (U8)sstByAid[i]->mac[3],
                            (U8)sstByAid[i]->mac[4],
                            (U8)sstByAid[i]->mac[5]);
                printk("WPA Search MAC Fails\n");
                printk("MAC:%02x %02x %02x %02x %02x %02x\n",
                    (U8)req_wpaie.wpa_macaddr[0],
                    (U8)req_wpaie.wpa_macaddr[1],
                    (U8)req_wpaie.wpa_macaddr[2],
                    (U8)req_wpaie.wpa_macaddr[3],
                    (U8)req_wpaie.wpa_macaddr[4],
                    (U8)req_wpaie.wpa_macaddr[5]);
                err = -EINVAL;
                break;
            }
            else
            {
                memcpy(&req_wpaie.wpa_ie, &pHash->WPAIE, sizeof(pHash->WPAIE));
                printk("MAC:%02x %02x %02x %02x %02x %02x\n",
                    (U8)req_wpaie.wpa_macaddr[0],
                    (U8)req_wpaie.wpa_macaddr[1],
                    (U8)req_wpaie.wpa_macaddr[2],
                    (U8)req_wpaie.wpa_macaddr[3],
                    (U8)req_wpaie.wpa_macaddr[4],
                    (U8)req_wpaie.wpa_macaddr[5]);

                if(copy_to_user(wrq->u.data.pointer, &req_wpaie, sizeof(struct ieee80211req_wpaie)))
                {
                    printk("Copy to user failed, in %s,%d\n",__func__,__LINE__); 
                    err = -EFAULT;
                    break;
                }
            }
            
        }
            err = 0; 
            break;

#endif
#if ZDCONF_WPS_SUPPORT == 1 && ZDCONF_AP_SUPPORT == 1
    case ZD_IOCTL_GETWSCIE:
    {
        struct ieee80211req_wscie req_wscie;
        Hash_t *pHash;

        if (copy_from_user(&req_wscie, ifr->ifr_data, sizeof(struct ieee80211req_wscie)))
        {
                printk("COpy from user fail in %s, %d", __func__, __LINE__);
            err = -EFAULT;
            break;
        }

        /* First search the STA's MAC address */
        pHash = HashSearch((MacAddr_t *)req_wscie.wsc_macaddr);

        if(pHash == NULL)
        {
            printk("pHash is NULL %s, %d", __func__, __LINE__);
            err = -EINVAL;
            break;
        }
        else
        {
            memcpy(&req_wscie.wsc_ie, &pHash->WSCIE, sizeof(pHash->WSCIE));

            if (copy_to_user(wrq->u.data.pointer, &req_wscie, sizeof(struct ieee80211req_wscie)))
            {
                printk("COpy to user fail in %s, %d", __func__, __LINE__);
                err = -EFAULT;
                break;
            }
        }
    }

        err = 0;
        break;

    case ZD_IOCTL_SETAPPIEBUF:
    {
        struct ieee80211req_getset_appiebuf* iebuf = (struct ieee80211req_getset_appiebuf *)ifr->ifr_data;

        printk("***** Hank debug - app_frmtype = %d app_buflen =%d, UpdateBcn=%d\n",iebuf->app_frmtype,iebuf->app_buflen, UpdateBcn);

#if 0 // always true
        if ( iebuf->app_buflen > IEEE80211_MAX_WSC_IE )
        {
            printk("app_buflen too long\n");
            err = -EINVAL;
            BUG();
            break;
        }
#endif

        if ( (iebuf->app_frmtype == IEEE80211_APPIE_FRAME_BEACON) ||
             (iebuf->app_frmtype == IEEE80211_APPIE_FRAME_PROBE_RESP) ||
             (iebuf->app_frmtype == IEEE80211_APPIE_FRAME_ASSOC_RESP) )
        {
            if(iebuf->app_frmtype == IEEE80211_APPIE_FRAME_BEACON)
                UpdateBcn++;
            if (iebuf->app_buflen) 
            {
                memcpy(pSetting->appie[iebuf->app_frmtype],iebuf->app_buf,iebuf->app_buflen);
                pSetting->appieLen[iebuf->app_frmtype] = iebuf->app_buflen;
            }
            else 
            {
                pSetting->appieLen[iebuf->app_frmtype] = 0;
            }
        }
        else
        {
            printk("Unknown Frame Type\n");
            err = -EINVAL;
            break;
        }
    }

        err = 0;
        break;
#endif

		default:
			//ZD1211DEBUG(0, "zd1205: unknown cmd = %2x\n", cmd);
			err = -EOPNOTSUPP;
			break;
	}
	
	if ((!err) && changed) {
#ifdef LINUX
#ifdef HOST_IF_USB
		defer_kevent(macp, KEVENT_UPDATE_SETTING);	
#else		
		zd_UpdateCardSetting(&macp->cardSetting);
#endif	
#else
		zd_UpdateCardSetting(&macp->cardSetting);
#endif
	}		
#ifdef LINUX
    up(&macp->ioctl_sem);
#endif
    kfree(zdreq);
//    MP_FUNCTION_EXIT();
	return err;
}

/**
 * zd1205init - initialize the adapter
 * @macp: atapter's private data struct
 *
 * This routine is called when this driver is loaded. This is the initialization
 * routine which allocates memory, configures the adapter and determines the
 * system resources.
 *
 * Returns:


 *      true: if successful
 *      false: otherwise
 */

unsigned char
zd1205_init(struct zd1205_private *macp)
{
    int i;
    u32 tmpValue;
    
	MP_ASSERT(0);
    //ZENTER(0);
#if fPROG_FLASH
    macp->bAllowAccessRegister = 1;
#endif
	/* read the MAC address from the eprom */
#ifdef LINUX
	mTxOFDMType = &(((struct zd1205_private *)g_dev->priv)->TxOFDMType);
	zd1205_rd_eaddr(macp);
#endif

    zd_writel(0x01, AfterPNP);
    
#if fWRITE_WORD_REG || fREAD_MUL_REG

    // Must get this information before any register write


    tmpValue = zd1211_readl(cADDR_ENTRY_TABLE, false);
    macp->AddrEntryTable = (u16) tmpValue;
    ZD1211DEBUG(0, "AddrEntryTable = %04x\n", macp->AddrEntryTable);
#endif

	macp->RF_Mode = zd_readl(E2P_POD);
    ZD1211DEBUG(0, "RF_Mode = %08x\n", macp->RF_Mode);
    macp->PA_Type = ((macp->RF_Mode) >> 16) & 0xF;//hardware type 2, bit0-3
    printk(KERN_ERR "PA type: %01x\n", macp->PA_Type);

    dot11Obj.HWFeature = macp->RF_Mode & 0xfff0;
    if((macp->RF_Mode >> 16) & BIT_15)
    {
        printk("PHYNEWLayout = 1\n");
        dot11Obj.PHYNEWLayout = 1;
    }
    if((macp->RF_Mode >> 16) & BIT_7)
    {
        printk("PHY_Decrease_CR128_state = 1\n");
        dot11Obj.PHY_Decrease_CR128_state = 1;
    }
        


    if (((macp->RF_Mode & 0xf) == AL2230_RF) && (dot11Obj.HWFeature & BIT_7) )
        macp->RF_Mode = AL2230S_RF;
    else
        macp->RF_Mode &= 0x0f;

	MP_ASSERT(0);
  	dot11Obj.rfMode = macp->RF_Mode;


	if ((dot11Obj.rfMode == 0x04) || (dot11Obj.rfMode == 0x07))
        printk("AiroHa AL2230RF\n");
    else if (dot11Obj.rfMode == 0x07)
        printk("Airoha AL7230B_RF\n");
    else if (dot11Obj.rfMode == 0x0a)
        printk("Airoha AL2230S_RF\n");
    else if (dot11Obj.rfMode == 0x0d)
        printk("RFMD RF\n");
	else if (dot11Obj.rfMode == 0x05)
		printk("AiroHa 7230B_RF\n");
    else if (dot11Obj.rfMode == UW2453_RF || dot11Obj.rfMode == AR2124_RF)
    {
        //dot11Obj.UWPowerControlFlag = TRUE;
        dot11Obj.UWPowerControlFlag = FALSE;
        tmpValue = zd_readl(E2P_DEVICE_VER+12);
        //printk("EEP( 0x%X) = 0x%X\n", E2P_DEVICE_VER+12, tmpValue);
        //printk("EEP( 0x%X) = 0x%X\n", E2P_PHY_REG, tmpValue);
        dot11Obj.UW2453SelectAntennaAUX = tmpValue >> 16;
        for(i=0;i<14;i++)
        {
            if((1 << i) & dot11Obj.UW2453SelectAntennaAUX)
            {
                dot11Obj.UW2453ChannelSelectAntennaAUX[i] = TRUE;
                printk(" UW_CSAA[%d]=1\n",i);
            } 
            else
                dot11Obj.UW2453ChannelSelectAntennaAUX[i] = FALSE;
        }
        if((1 << 14) & dot11Obj.UW2453SelectAntennaAUX)
        {
            printk(" UW_NoTxFolRx=1\n");
            dot11Obj.UW2453NoTXfollowRX = TRUE;
        }
        if((1 << 15) & dot11Obj.UW2453SelectAntennaAUX)
        {
            printk(" UW_MiCa=1\n");
            dot11Obj.UW2453MiniCard = TRUE;
        }
        
        if (dot11Obj.rfMode == UW2453_RF)
            printk("UW2453 RF\n");
        else
            printk("AR2124 RF\n");
    }
    else      
        printk("Unknown RF_Mode = %x\n", (u8)dot11Obj.rfMode);



    zd_writel(0x00, GPI_EN);                

    zd1205_sw_init(macp);
	zd1205_hw_init(macp);
	zd1205_disable_int();

    ZEXIT(0);
	return true;
}

//setup callback functions for protocol stack
void zd1205_set_zd_cbs(zd_80211Obj_t *pObj)
{
    pObj->QueueFlag = 0;
    pObj->ConfigFlag = 0;
    pObj->SetReg = zdcb_set_reg;
    pObj->GetReg = zdcb_get_reg;

    pObj->ReleaseBuffer = zdcb_release_buffer;
//    pObj->RxInd = zdcb_rx_ind;
    pObj->TxCompleted = zdcb_tx_completed;
    pObj->StartTimer = zdcb_start_timer;
    pObj->StopTimer = zdcb_stop_timer;
    pObj->SetupNextSend = zdcb_setup_next_send;

    pObj->StatusNotify = zdcb_status_notify;
    pObj->ExitCS = zdcb_set_intr_mask;
    pObj->EnterCS = zdcb_dis_intr;
    pObj->Vir2PhyAddr = zdcb_vir_to_phy_addr;
    pObj->CheckTCBAvail = zdcb_check_tcb_avail;
    pObj->DelayUs = zdcb_delay_us;
    pObj->AllocBuffer = zdcb_AllocBuffer;
    pObj->Rand = zdcb_Rand;
    pObj->AcquireDoNotSleep = zdcb_AcquireDoNotSleep;
    pObj->ReleaseDoNotSleep = zdcb_ReleaseDoNotSleep;
    pObj->bChScanning=0; 

    // wpa support
#ifdef HOSTAPD_SUPPORT
    pObj->MicFailure = zdcb_MicFailure;
    pObj->AssocRequest = zdcb_AssocRequest;
    pObj->WpaIe = NULL;
#else
    // wpa support
    pObj->MicFailure = NULL;
    pObj->AssocRequest = NULL;
    pObj->WpaIe = NULL;
#endif

}

void zd1205_SetRatesInfo(struct zd1205_private *macp)
{

    u8 bRatesSet = 1;
    card_Setting_t *pCardSetting;

	MP_ASSERT(0);
    pCardSetting = &macp->cardSetting;

    if (pCardSetting->MacMode == MIXED_MODE){
        ZD1211DEBUG(0, "Mixed Mode\n");
        zd_writel(CW_SHORT_SLOT, CWmin_CWmax);
        pCardSetting->ShortSlotTime = 1;
        pCardSetting->PreambleType = 1; //short preamble
        bRatesSet = 1;
    }
    else if (pCardSetting->MacMode == PURE_G_MODE){
        ZD1211DEBUG(0, "Pure G-Mode\n");
        zd_writel(CW_SHORT_SLOT, CWmin_CWmax);
        pCardSetting->ShortSlotTime = 1;
        pCardSetting->PreambleType = 1; //short preamble
        bRatesSet = 2;
    }	
    else if (pCardSetting->MacMode == PURE_A_MODE) {
        ZD1211DEBUG(0, "Pure A-Mode\n");
        zd_writel(CW_SHORT_SLOT, CWmin_CWmax);
        pCardSetting->ShortSlotTime = 1;
        pCardSetting->PreambleType = 1; //short preamble
        bRatesSet = 4;

    }
    else if (pCardSetting->MacMode == PURE_B_MODE) { // pure B mode
        ZD1211DEBUG(0, "Pure B-Mode\n");
        zd_writel(CW_NORMAL_SLOT, CWmin_CWmax);
        pCardSetting->ShortSlotTime = 0;
        pCardSetting->PreambleType = 1; //short preamble
        bRatesSet = 3;
    }
    else
        VerAssert();

    if (bRatesSet == 1){ //wi-fi set1
        // supported rates
        pCardSetting->Info_SupportedRates[0] = 0x01;
        pCardSetting->Info_SupportedRates[1] = 0x04;
        pCardSetting->Info_SupportedRates[2] = 0x82; //basic rate
        pCardSetting->Info_SupportedRates[3] = 0x84; //basic rate
        pCardSetting->Info_SupportedRates[4] = 0x8B; //basic rate
        pCardSetting->Info_SupportedRates[5] = 0x96; //basic rate

        //Extended supported rates
        pCardSetting->Ext_SupportedRates[0] = 0x32;
        pCardSetting->Ext_SupportedRates[1] = 0x08;
        pCardSetting->Ext_SupportedRates[2] = 0x0c;
        pCardSetting->Ext_SupportedRates[3] = 0x12;
        pCardSetting->Ext_SupportedRates[4] = 0x18;
        pCardSetting->Ext_SupportedRates[5] = 0x60;
        pCardSetting->Ext_SupportedRates[6] = 0x24;
        pCardSetting->Ext_SupportedRates[7] = 0x30;
        pCardSetting->Ext_SupportedRates[8] = 0x48;
        pCardSetting->Ext_SupportedRates[9] = 0x6c;
        zd_writel(0x150f, MandatoryRateTbl); //1,2,5.5,11,6,12,24
    }		
    else if (bRatesSet == 2){ //wi-fi set2
        // supported rates
        pCardSetting->Info_SupportedRates[0] = 0x01; 
        pCardSetting->Info_SupportedRates[1] = 0x04; 
        pCardSetting->Info_SupportedRates[2] = 0x82; //basic rate
        pCardSetting->Info_SupportedRates[3] = 0x84; //basic rate
        pCardSetting->Info_SupportedRates[4] = 0x8B; //basic rate
        pCardSetting->Info_SupportedRates[5] = 0x96; //basic rate

        //Extended supported rates
        pCardSetting->Ext_SupportedRates[0] = 0x32;
        pCardSetting->Ext_SupportedRates[1] = 0x08;
        pCardSetting->Ext_SupportedRates[2] = 0x8c; //basic rate
        pCardSetting->Ext_SupportedRates[3] = 0x12;
        pCardSetting->Ext_SupportedRates[4] = 0x98; //basic rate
        pCardSetting->Ext_SupportedRates[6] = 0x24; 
        pCardSetting->Ext_SupportedRates[7] = 0xb0; //basic rate
        pCardSetting->Ext_SupportedRates[8] = 0x48;
        pCardSetting->Ext_SupportedRates[5] = 0x60;
        pCardSetting->Ext_SupportedRates[9] = 0x6c;

        zd_writel(0x150f, MandatoryRateTbl); //1,2,5.5,11,6,12,24
    }
    else if (bRatesSet == 3){ //pure b mode
        // supported rates
        pCardSetting->Info_SupportedRates[0] = 0x01;
        pCardSetting->Info_SupportedRates[1] = 0x04;
        pCardSetting->Info_SupportedRates[2] = 0x82; //basic rate
        pCardSetting->Info_SupportedRates[3] = 0x84; //basic rate
        pCardSetting->Info_SupportedRates[4] = 0x8B; //basic rate
        pCardSetting->Info_SupportedRates[5] = 0x96; //basic rate		
        zd_writel(0x0f, MandatoryRateTbl); //1,2,5.5,11
    }
    else if (bRatesSet == 4) {//Pure A
        pCardSetting->Info_SupportedRates[0] = 0x01; //Element ID
        pCardSetting->Info_SupportedRates[1] = 0x08; //Rates Amount
        pCardSetting->Info_SupportedRates[2] = 0x80+12 ; //RateByte = Mandatory Bit + 500k x 12
        pCardSetting->Info_SupportedRates[3] = 0x00+18; //Supported Rate
        pCardSetting->Info_SupportedRates[4] = 0x80+24; //basic rate
        pCardSetting->Info_SupportedRates[5] = 0x00+36; 
        pCardSetting->Info_SupportedRates[6] = 0x80+48; //basic rate
        pCardSetting->Info_SupportedRates[7] = 0x00+72; 
        pCardSetting->Info_SupportedRates[8] = 0x00+96; 
        pCardSetting->Info_SupportedRates[9] = 0x00+108;

        zd_writel(0x0f, MandatoryRateTbl); //6,9,12,18,24,36,48,54

    }

}

#if ZDCONF_SIGNAL_INFO == 1
u16 ZDLOGTEN[] = {0, 
    0   ,  30 ,  47 ,  60 ,  69 ,  77 ,  84 ,  90 ,  95 , 100 ,
    104 , 107 , 111 , 114 , 117 , 120 , 123 , 125 , 127 , 130 ,
    132 , 134 , 136 , 138 , 139 , 141 , 143 , 144 , 146 , 147 ,
    149 , 150 , 151 , 153 , 154 , 155 , 156 , 157 , 159 , 160 ,
    161 , 162 , 163 , 164 , 165 , 166 , 167 , 168 , 169 , 169 ,
    170 , 171 , 172 , 173 , 174 , 174 , 175 , 176 , 177 , 177 ,
    178 , 179 , 179 , 180 , 181 , 181 , 182 , 183 , 183 , 184 ,
    185 , 185 , 186 , 186 , 187 , 188 , 188 , 189 , 189 , 190 ,
    190 , 191 , 191 , 192 , 192 , 193 , 193 , 194 , 194 , 195 ,
    195 , 196 , 196 , 197 , 197 , 198 , 198 , 199 , 199 , 200 ,
    200 , 200 , 210 , 210 , 220 , 220 , 220 , 230 , 230 , 240 ,
    240 , 240 , 250 , 250 , 260 , 260 , 260 , 270 , 270 , 270 ,
    280 , 280 , 280 , 290 , 290 , 210 , 210 , 210 , 211 , 211 ,
    211 , 212 , 212 , 212 , 213 , 213 , 213 , 213 , 214 , 214 ,
    214 , 215 , 215 , 215 , 216 , 216 , 216 , 217 , 217 , 217 ,
    217 , 218 , 218 , 218 , 219 , 219 , 219 , 219 , 220 , 220 ,
    220 , 220 , 221 , 221 , 221 , 222 , 222 , 222 , 222 , 223 ,
    223 , 223 , 223 , 224 , 224 , 224 , 224 , 225 , 225 , 225 ,
    225
};

    u16
ZDLog10multiply100(int data)
{
    if ((data >= 0) && (data <= 0xb5))
        return ZDLOGTEN[data];
    else
        return 225;
}



u32 X_Constant[] = {
    715, 655, 585, 540, 470, 410, 360, 315,
    270, 235, 205, 175, 150, 125, 105, 85,
    65, 50, 40, 25, 15

};    


u8 X_To_dB(u32 X, u8 rate)
{
    u8 ret = 0;
    int i;

    int SizeOfX_Con = sizeof(X_Constant);

    switch (rate)
    {
        case 0x0B:  // 6M
        case 0x0A:  // 12M
        case 0x09:  // 24M
            X /= 2;
            break;
        case 0x0F:  // 9M
        case 0x0E:  // 18M
        case 0x0D:  // 36M
        case 0x0C:  // 54M
            X *= 3;
            X /= 4;
            break;
        case 0x08:  // 48M
            X *= 2;
            X /= 3;
            break;
        default:
            break;
    }
    for (i=0; i<SizeOfX_Con; i++){
        if (X > X_Constant[i])
            break;
    }

    switch (rate)
    {
        case 0x0B:  // 6M
        case 0x0F:  // 9M
            ret = i + 3;
            break;
        case 0x0A:  // 12M
        case 0x0E:  // 18M
            ret = i + 5;
            break;
        case 0x09:  // 24M
        case 0x0D:  // 36M
            ret = i + 9;
            break;
        case 0x08:  // 48M
        case 0x0C:  // 54M
            ret = i + 15;
            break;
        default:
            break;
    }
    return ret;        
}
#endif
u8 rssi2Quality(u8 rssi)
{
    int dBm = rssi-96;
    if(dBm > -40)
        return 100;
    else
        return 100 - (-40-dBm)*16/10;
}
u8 CalculateStrength(struct zd1205_private *macp, zd1205_RFD_t *rfd)
{
#if ZDCONF_SIGNAL_INFO == 1
    // return in ? , the Value-105 = dB
    // the value from PHY is in ?
    u32 frame_len;
    u32 tot_len;
    u8 i, rssi, tmp;
    u32 tmpvalue = 2;
    plcp_wla_Header_t *wla_hdr;
    //u8 rxOffset = macp->rxOffset;	

    wla_hdr = (plcp_wla_Header_t *)&rfd->RxBuffer[macp->rxOffset];
    tot_len = rfd->ActualCount & 0x3fff;
    frame_len = tot_len - EXTRA_INFO_LEN;
    rssi = rfd->RxBuffer[frame_len+1];

    if ( (((macp->cardSetting.BssType == INFRASTRUCTURE_BSS)&&
                    (!memcmp(wla_hdr->Address2, macp->BSSID, 6))) ||
                ((macp->cardSetting.BssType == INDEPENDENT_BSS)&&
                 (!memcmp(wla_hdr->Address3, macp->BSSID, 6))) ||
                (macp->cardSetting.BssType == PSEUDO_IBSS)) &&
            (macp->bAssoc) ){
        for(i=0; i<macp->PHYTestIndex-1; i++)
            tmpvalue *= 2; 

        //if ( (dot11Obj.CR122Flag == 1)||(dot11Obj.CR203Flag == 1) )
        //	rssi += 22;
        tmp = macp->PHYTestRssi;
        macp->PHYTestTotal = macp->PHYTestTotal 
            - (macp->PHYTestTotal/tmpvalue)
            + rssi;
        macp->PHYTestRssi = (u8) (macp->PHYTestTotal/tmpvalue);
    }

    return rssi;
#else
    return 0;
#endif
}

void zd1205_initCAM(struct zd1205_private *macp)
{
    int i;

    zd_writel(0, CAM_ROLL_TB_LOW);
    zd_writel(0, CAM_ROLL_TB_HIGH);

    for (i=0; i<445; i++){
        HW_CAM_Write(&dot11Obj, i, 0);
    }	
}
#if ZDCONF_AP_SUPPORT == 1
int zd1205_CheckOverlapBss(struct zd1205_private *macp, plcp_wla_Header_t *pWlanHdr, u8 *pMacBody, u32 bodyLen)
{
    u8 *pByte;
    u32 currPos = 0;
    u8 elemId, elemLen;
    u8 gAP = 0;
    u8 ErpInfo = 0;
    U16 loopCheck = 0;

    //get element
    pByte = pMacBody+SSID_OFFSET;
    currPos = SSID_OFFSET;

    while(currPos < bodyLen)
    {
        //To prevent incorrect elemLen (ex. 0)
        if(loopCheck++ > 100)
        {
            printk("infinite loop occurs in %s\n", __FUNCTION__);
            loopCheck = 0;
            break;
        }

        elemId = *pByte;
        elemLen = *(pByte+1);

        switch(elemId){
            case ELEID_ERP_INFO: //ERP info
                gAP = 1;
                ErpInfo = *(pByte+2);
                pByte += (elemLen+2); 
                break;

            default:
                pByte += (elemLen+2); 	
                break;

        }

        currPos += elemLen+2;
    }	

    if (gAP){
        if (ErpInfo & NON_ERP_PRESENT_BIT){ //with B sta associated
            return 1;
        }	
        else
            return 0;	
    }	
    else // B AP exist, enable protection mode
        return 1;
}	
#endif
void zd1205_HandleQosRequest(struct zd1205_private *macp)
{
    zd1205_SwTcb_t *sw_tcb;

    if (!macp->activeTxQ->count)
        sw_tcb = macp->freeTxQ->first;
    else
        sw_tcb = macp->activeTxQ->first;
    zd1205_start_download(sw_tcb->TcbPhys | BIT_0);
}

/**
 * zd1205_notify_join_event - Notify wireless join event to the upper layer
 * @macp: atapter's private data struct
 * @newassoc: new associate or not
 *
 */

void zd1205_notify_join_event(struct zd1205_private *macp)
{
    union iwreq_data wreq;

    memset(&wreq, 0, sizeof(wreq));
    memcpy(wreq.addr.sa_data, &macp->BSSID[0], 6);
    wreq.addr.sa_family = ARPHRD_ETHER;

    {
        ZD1211DEBUG(0, "Notify_join_event:" MACSTR "\n",MAC2STR(macp->BSSID));
        /*	int ii;

            WPADEBUG("zd1205_notfiy_join_event: MAC= ");
            for(ii = 0; ii < 6; ii++)
            WPADEBUG("%02x ", macp->BSSID[ii] & 0xff);
            WPADEBUG("\n");*/
    }

    if(macp->cardSetting.BssType != AP_BSS) {
	#if Make_WPA
        wireless_send_event(macp->device, SIOCGIWAP, &wreq, NULL);
	#endif
    }
#if WIRELESS_EXT >= 15
#if ZDCONF_AP_SUPPORT == 1
    else if(macp->cardSetting.BssType == AP_BSS) {
#if Make_WPA
        wireless_send_event(macp->device, IWEVREGISTERED, &wreq, NULL);
#endif
    }
#endif
#endif
}
void zd1205_notify_disjoin_event(struct zd1205_private *macp, u8 *mac)
{
    union iwreq_data wreq;

	MP_ASSERT(0);
    memset(&wreq, 0, sizeof(wreq));
    //memcpy(wreq.addr.sa_data, &macp->BSSID[0], 6);
    wreq.addr.sa_family = ARPHRD_ETHER;
    printk(KERN_DEBUG "zd1205_notify_disjoin_event\n");
    /*{
      int ii;

      WPADEBUG("zd1205_notfiy_join_event: MAC= ");
      for(ii = 0; ii < 6; ii++)
      WPADEBUG("%02x ", macp->BSSID[ii] & 0xff);
      WPADEBUG("\n");
      }*/

    if(macp->cardSetting.BssType == INFRASTRUCTURE_BSS) {
        //wireless_send_event(macp->device, SIOCGIWSCAN, &wreq, NULL);
        macp->bAssoc = 0;
        memset(macp->BSSID,0,sizeof(macp->BSSID));
	#if Make_WPA
        wireless_send_event(macp->device, SIOCGIWAP, &wreq, NULL);
	#endif
    }
#if WIRELESS_EXT >= 15
#if ZDCONF_AP_SUPPORT == 1
    else if(macp->cardSetting.BssType == AP_BSS) 
    {
        if(mac != NULL)
            memcpy(wreq.addr.sa_data,mac,6);
        else
            printk("mac == NULL !\n");
		#if Make_WPA
        wireless_send_event(macp->device, IWEVEXPIRED, &wreq, NULL);
		#endif
    }
#endif
#endif
}
void zd1205_notify_scan_done(struct zd1205_private *macp)
{
    union iwreq_data wreq;
    wreq.data.length = 0;
    wreq.data.flags = 0;
#if Make_WPA
    wireless_send_event(macp->device, SIOCGIWSCAN, &wreq, NULL);
#endif
}
#if ZDCONF_MIC_CHECK == 1
#if WIRELESS_EXT >= 18
void hostap_michael_mic_failure(struct zd1205_private *macp,
        struct hostap_ieee80211_hdr *hdr,
        int keyidx)
{
    union iwreq_data wrqu;
    struct iw_michaelmicfailure ev;

    /* TODO: needed parameters: count, keyid, key type, TSC */
    memset(&ev, 0, sizeof(ev));
    ev.flags = keyidx & IW_MICFAILURE_KEY_ID;
    if (hdr->addr1[0] & 0x01)
        ev.flags |= IW_MICFAILURE_GROUP;
    else
        ev.flags |= IW_MICFAILURE_PAIRWISE;
    ev.src_addr.sa_family = ARPHRD_ETHER;
    memcpy(ev.src_addr.sa_data, hdr->addr2, ETH_ALEN);
    memset(&wrqu, 0, sizeof(wrqu));
    wrqu.data.length = sizeof(ev);
#if Make_WPA
    wireless_send_event(g_dev, IWEVMICHAELMICFAILURE, &wrqu, (char *) &ev);
#endif
}
#elif WIRELESS_EXT >= 15
// For kernel 2.6.5(FC2), WIRELESS_EXT is 16
void hostap_michael_mic_failure(struct zd1205_private *macp,
        struct hostap_ieee80211_hdr *hdr,
        int keyidx)
{
    union iwreq_data wrqu;
    char buf[128];

    /* TODO: needed parameters: count, keyid, key type, TSC */
    sprintf(buf, "MLME-MICHAELMICFAILURE.indication(keyid=%d %scast addr="
            MACSTR ")", keyidx, hdr->addr1[0] & 0x01 ? "broad" : "uni",
            MAC2STR(hdr->addr2));
    memset(&wrqu, 0, sizeof(wrqu));
    wrqu.data.length = strlen(buf);
    printk("MLME-MICHAELMICFAILURE.indication(keyid=%d %scast addr=" 
            MACSTR ")", keyidx, hdr->addr1[0] & 0x01 ? "broad" : "uni",
            MAC2STR(hdr->addr2));
#if Make_WPA
    wireless_send_event(g_dev, IWEVCUSTOM, &wrqu, buf);
#endif
}
#else /* WIRELESS_EXT >= 15 */
void hostap_michael_mic_failure(struct zd1205_private *macp,
        struct hostap_ieee80211_hdr *hdr,
        int keyidx)
{
}
#endif /* WIRELESS_EXT >= 15 */
#endif //ZDCONF_MIC_CHECK == 1
#if Make_ADHOC
BssInfo_t *zd1212_bssid_to_BssInfo(U8 *bssid)
{
    int i;
    for(i=0;i<mBssNum;i++)
    {
        if(memcmp(&mBssInfo[i].bssid, bssid, ETH_ALEN) == 0)
            return &mBssInfo[i];
    }
    return NULL;
}
#endif

void ChangeMacMode(u8 MAC_Mode, u8 Channel) {
    struct zd1205_private *macp;

    if(NULL != g_dev && NULL != g_dev->priv)
        macp = (struct zd1205_private *)g_dev->priv;
    else
    {
        LongPrint("NULL macp in ChnageMacMode\n",1);
        return;
    }


    zd1205_lock(macp);
    macp->cardSetting.Channel = Channel; //Default Channel to 8
    dot11Obj.Channel = Channel;
    macp->cardSetting.MacMode = MAC_Mode ;
    macp->bDefaultIbssMacMode=1;
    zd1205_unlock(macp);

    //set_mac_mode command has been issued by the user.
    zd1205_SetRatesInfo(macp);
    //zd_UpdateCardSetting(&(macp->cardSetting));
}
#if ZDCONF_WE_STAT_SUPPORT == 1
    struct iw_statistics *
zd1205_iw_getstats(struct net_device *dev)
{
    struct zd1205_private *macp = (struct zd1205_private *)dev->priv;

    macp->iwstats.discard.fragment = macp->ArAgedCnt
        + macp->ArFreeFailCnt;

    macp->iwstats.discard.retries = macp->retryFailCnt;
    macp->iwstats.discard.misc = macp->invalid_frame_good_crc
        + macp->rxDupCnt;

    return &macp->iwstats;

}
#elif !defined(ZDCONF_WE_STAT_SUPPORT)
#error "Undefine ZDCONF_WE_STAT_SUPPORT"
#endif
void zd1205_start_download(u32 phyAddr)
{
#ifdef HOST_IF_USB
	return;
#endif
}	
void zd1205_disable_int(void)
{

	/* Disable interrupts on our PCI board by setting the mask bit */
	zd_writel(0, InterruptCtrl);
}

int
zd1205_set_mac(struct net_device *dev, void *addr)
{
}
void zd1205_set_multicast(struct net_device *dev)
{

//#if ZDCONF_SETMULTI_SUPPORT == 1
    //struct net_device *dev = macp->device;
	struct dev_mc_list *mc_list;
 	unsigned int i,GroupHashP1,GroupHashP2;

    u8 *pKey;
    u32 tmpValue;
    u8  mcBuffer[32];
    u16 mcLen;
    u8 MulticastAddr[64];
	//mpDebugPrint("zd1211_set_multicast");
    //if (!(dev->flags & IFF_UP))
    //    return;
#if ZDCONF_AP_SUPPORT == 1 
    if (macp->cardSetting.BssType == AP_BSS)
    	return;
#endif


    zd_writel(0, GroupHash_P1);
	//TaskYield();
    zd_writel(0x80000000, GroupHash_P2);
    MulticastAddr[0] = dev->mc_count;
    mcLen = dev->mc_count*ETH_ALEN ;
	mc_list = dev->mc_list;
    for (i = 0;i< dev->mc_count;i++) {
        zd1205_dump_data("mc addr", (u8 *)&(mc_list->da_addr), ETH_ALEN);
  		memcpy(&MulticastAddr[1+i * ETH_ALEN], (u8 *) &(mc_list->da_addr), ETH_ALEN);
		//mc_list = mc_list->next;
	}
    MulticastAddr[mcLen +1] = 0;
    zd1205_dump_data("MulticastAddr", (u8 *)MulticastAddr, mcLen +2);
		
    memcpy(mcBuffer, &MulticastAddr[1], mcLen);

    zd1205_dump_data("mcBuffer", (u8 *)mcBuffer, mcLen);
    pKey = mcBuffer;
    for (i=0; i<mcLen; i++){
        if ((i%6) == 5){
            *(pKey+i) = (*(pKey+i)) >> 2;
            if (*(pKey+i) >= 32){
                tmpValue = zd_readl(GroupHash_P2);

                tmpValue |= (0x01 << (*(pKey+i)-32));
				
				mpDebugPrint("GroupHash_P2 tmpValue %x",tmpValue);
                zd_writel(tmpValue, GroupHash_P2);
				
            }
            else {
                tmpValue = zd_readl(GroupHash_P1);
                tmpValue |= (0x01 << (*(pKey+i)));
				mpDebugPrint("GroupHash_P1 tmpValue %x",tmpValue);
                zd_writel(tmpValue, GroupHash_P1);
            }
        }
    }
	
	//if(g_dev->flags & (IFF_PROMISC | IFF_ALLMULTI)) 
	if(dev->mc_list->da_addr[0]==0xFF)
    {
		mpDebugPrint("Promiscuous or ALLMulti mode enabled.\n");
		zd_writel(0xffffffff,GroupHash_P1);
		zd_writel(0xffffffff,GroupHash_P2);
	}
//#endif

#if 0
    GroupHashP1 = zd_readl(GroupHash_P1);


    GroupHashP2 = zd_readl(GroupHash_P2);

    //ZD1211DEBUG(1, "GroupHashP1 = %x\n", macp->GroupHashP1);
    //ZD1211DEBUG(1, "GroupHashP2 = %x\n", macp->GroupHashP2);
    mpDebugPrint("GroupHashP1 = %x\n", GroupHashP1);
    mpDebugPrint("GroupHashP2 = %x\n", GroupHashP2);

    //for debug only
    //zd_writel(0xffffffff, GroupHash_P1);

    //zd_writel(0xffffffff, GroupHash_P2);
#endif    
}

int sizeof_zydas_wlan_param(void)
{
    return (sizeof(struct zydas_wlan_param));
}

int
mp_zd1205_ioctl(struct iwreq *iwr, int cmd)
{
    struct zd1205_private *macp = macp_global;
    return zd1205_ioctl(macp_global->device, iwr, cmd);
}

