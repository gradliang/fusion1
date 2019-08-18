#ifndef _ZD_DEBUG_C_
#define _ZD_DEBUG_C_

#include <linux/types.h>
#include "ndebug.h"
#include "zdmpx.h"
#include "zddebug.h"
#include "zdhw.h"
#include "zdutils.h"
#include "zdpsmon.h"
#ifdef HOST_IF_USB
    #include "zd1211.h"
#endif
#include "zdglobal.h"
#if WIRELESS_EXT > 12
    #include <net/iw_handler.h>
#endif
#include "zdusb.h"
#include "zdversion.h"


extern zd_80211Obj_t dot11Obj;

//for debug message show
extern u32 freeSignalCount;
extern u32 freeFdescCount;
extern void zd_ShowQInfo(void);
extern void zd_ShowState(void);
extern BOOLEAN mPrivacyInvoked;
extern U16 mCap;
extern U8 mWpaBcKeyLen;
extern U8 mGkInstalled;
extern U8 mDynKeyMode;
extern U8 mKeyFormat;
int zd1205_cont_tx(struct zd1205_private *macp, u8 rate);
extern int zd_SetKeyContext(U8 *addr, U8 encryMode, U8 keyLength, U8 KeyId, U8 *pKeyContent);
extern void ConfigBcnFIFO(void);
extern BOOLEAN zd_CmdDeauth(MacAddr_t *sta, U8 rCode);
extern BOOLEAN zd_CmdDisasoc(MacAddr_t *sta, U8 rCode);
extern void update_beacon_interval(struct zd1205_private *macp, int val);
#if ZDCONF_RF_UW2453_SUPPORT == 1 || ZDCONF_RF_AR2124_SUPPORT == 1
extern void PHY_UWTxPower(zd_80211Obj_t *pObj, U8 TxLevel);
#endif
extern U8 mMacMode;
#if ZDCONF_APC == 1
extern U8 APC_NT[256][6];
#endif
#if ZDCONF_ANT_DIVERSITY == 1
extern int zfiHp_AntennaScheme(struct zd1205_private *macp, u8 idx);
#endif



void zd1205_dump_regs(struct zd1205_private *macp)
{
#if ZDCONF_DBGMSG_NORMAL == 1
#ifndef HOST_IF_USB    
	spin_lock_irqsave(&macp->q_lock, flags);
#endif    

	printk(KERN_DEBUG "*******************************************************\n");
	printk(KERN_DEBUG "MACAddr_P1         = %08x  MACAddr_P2    = %08x\n",
        zd_readl(MACAddr_P1), zd_readl(MACAddr_P2));
	printk(KERN_DEBUG "BCNInterval        = %08x. BCNPLCPCfg    = %08x\n",
        zd_readl(BCNInterval), zd_readl(BCNPLCPCfg));
	printk(KERN_DEBUG "TSF_LowPart        = %08x, TSF_HighPart  = %08x\n",
        zd_readl(TSF_LowPart), zd_readl(TSF_HighPart));
	printk(KERN_DEBUG "DeviceState        = %08x, NAV_CCA       = %08x\n",
        zd_readl(DeviceState), zd_readl(NAV_CCA));
	printk(KERN_DEBUG "CRC32Cnt           = %08x, CRC16Cnt      = %08x\n",
        zd_readl(CRC32Cnt), zd_readl(CRC16Cnt));
	printk(KERN_DEBUG "TotalRxFrm         = %08x, TotalTxFrm    = %08x\n",
        zd_readl(TotalRxFrm), zd_readl(TotalTxFrm));
	printk(KERN_DEBUG "RxFIFOOverrun      = %08x, UnderrunCnt   = %08x\n",
        zd_readl(RxFIFOOverrun), zd_readl(UnderrunCnt));
	printk(KERN_DEBUG "BSSID_P1           = %08x, BSSID_P2      = %08x\n",
        zd_readl(BSSID_P1), zd_readl(BSSID_P2));
	printk(KERN_DEBUG "Pre_TBTT           = %08x, ATIMWndPeriod = %08x\n",
        zd_readl(Pre_TBTT), zd_readl(ATIMWndPeriod));
	printk(KERN_DEBUG "RetryCnt           = %08x, IFS_Value     = %08x\n",
        zd_readl(RetryCnt), zd_readl(IFS_Value));
	printk(KERN_DEBUG "NAV_CNT            = %08x, CWmin_CWmax   = %08x\n",
        zd_readl(NAV_CNT), zd_readl(CWmin_CWmax));
	//printk(KERN_DEBUG "GroupHash_P1       = %08x, GroupHash_P2  = %08x\n",
	//    zd_readl(GroupHash_P1), zd_readl(GroupHash_P2));
	printk(KERN_DEBUG "DecrypErr_UNI      = %08x, DecrypErr_Mul = %08x\n",
        zd_readl(DecrypErr_UNI), zd_readl(DecrypErr_Mul));       
    printk(KERN_DEBUG "intrMask           = %08x\n", macp->intrMask);
            
#ifndef HOST_IF_USB
	printk(KERN_DEBUG "InterruptCtrl      = %08x, Rx_Filter     = %08x\n",
        zd_readl(InterruptCtrl), zd_readl(Rx_Filter));
	printk(KERN_DEBUG "ReadTcbAddress     = %08x, ReadRfdAddress= %08x\n",
        zd_readl(ReadTcbAddress), zd_readl(ReadRfdAddress));
	printk(KERN_DEBUG "BCN_FIFO_Semaphore = %08x, CtlReg1       = %08x\n",
        zd_readl(BCN_FIFO_Semaphore),  zd_readl(CtlReg1));    
	printk(KERN_DEBUG "RX_OFFSET_BYTE     = %08x, RX_TIME_OUT   = %08x\n",
        zd_readl(RX_OFFSET_BYTE), zd_readl(RX_TIME_OUT));
	printk(KERN_DEBUG "CAM_DEBUG          = %08x, CAM_STATUS    = %08x\n",
        zd_readl(CAM_DEBUG), zd_readl(CAM_STATUS));
	printk(KERN_DEBUG "CAM_ROLL_TB_LOW    = %08x, CAM_ROLL_TB_HIGH = %08x\n",
        zd_readl(CAM_ROLL_TB_LOW), zd_readl(CAM_ROLL_TB_HIGH));
	printk(KERN_DEBUG "CAM_MODE           = %08x, intrMask = %08x\n", zd_readl(CAM_MODE), macp->intrMask);
#endif    

#ifndef HOST_IF_USB        
	spin_unlock_irqrestore(&macp->q_lock, flags);
#endif    
#endif
}

void zd1205_dump_cnters(struct zd1205_private *macp)
{
#if ZDCONF_DBGMSG_NORMAL == 1
	zd1205_lock(macp);
	printk(KERN_DEBUG "*************************************************\n");
	printk(KERN_DEBUG "freeTxQ         = %08d, activeTxQ      = %08d\n", macp->freeTxQ->count, macp->activeTxQ->count);
	printk(KERN_DEBUG "freeSignalCount = %08d, freeFdescCount = %08d\n", freeSignalCount, freeFdescCount);
	//printk(KERN_DEBUG "hwTotalRxFrm    = %08d, hwTotalTxFrm   = %08d\n", macp->hwTotalRxFrm, macp->hwTotalTxFrm);
	//printk(KERN_DEBUG "hwRxFIFOOverrun = %08d, hwUnderrunCnt  = %08d\n", macp->hwRxFIFOOverrun, macp->hwUnderrunCnt);
	//printk(KERN_DEBUG "hwCRC32Cnt      = %08d, hwCRC16Cnt     = %08d\n", macp->hwCRC32Cnt, macp->hwCRC16Cnt);

	//printk(KERN_DEBUG "ErrLongFrmCnt   = %08d, ErrShortFrmCnt = %08d\n", macp->ErrLongFrmCnt, macp->ErrShortFrmCnt);
	//printk(KERN_DEBUG "ErrToHostFrmCnt = %08d, ErrZeroLenFrmCnt= %08d\n", macp->ErrToHostFrmCnt, macp->ErrZeroLenFrmCnt);
	printk(KERN_DEBUG "rxOFDMDataFrame = %08d, rx11bDataFrame = %08d\n", macp->rxOFDMDataFrame, macp->rx11bDataFrame);
	printk(KERN_DEBUG "rxSignalQuality = %08d, rxSignalStrength= %08d\n", macp->rxSignalQuality, macp->rxSignalStrength);
	printk(KERN_DEBUG "rxRate          = %08d, txRate         = %08dx\n", macp->rxInfo.rate, macp->cardSetting.CurrTxRate);

	//printk(KERN_DEBUG "rxNeedFragCnt   = %08d, rxCompFragCnt  = %08d\n", macp->rxNeedFragCnt, macp->rxCompFragCnt);
	//printk(KERN_DEBUG "ArFreeFailCnt   = %08d, ArAgedCnt      = %08d\n", macp->ArFreeFailCnt, macp->ArAgedCnt);
	//printk(KERN_DEBUG "ArSearchFailCnt = %08d, DropFirstFragCnt= %08d\n", macp->ArSearchFailCnt, macp->DropFirstFragCnt);
	//printk(KERN_DEBUG "skb_req         = %08d, AllocSkbFailCnt= %08d\n", macp->skb_req, macp->AllocSkbFailCnt);
	printk(KERN_DEBUG "txQueToUpCnt    = %08d, txQueSetCnt    = %08d\n", macp->txQueToUpCnt, macp->txQueSetCnt);
	printk(KERN_DEBUG "sleepCnt        = %08d, wakeupCnt      = %08d\n", macp->sleepCnt, macp->wakeupCnt);
	printk(KERN_DEBUG "WaitLenInfoCnt  = %08d, CompLenInfoCnt = %08d\n", macp->WaitLenInfoCnt, macp->CompLenInfoCnt);
	printk(KERN_DEBUG "Continue2Rx     = %08d, NoMergedRxCnt  = %08d\n", macp->Continue2Rx, macp->NoMergedRxCnt);
	printk(KERN_DEBUG "bcnCnt          = %08d, dtimCnt        = %08d\n", macp->bcnCnt, macp->dtimCnt);
	printk(KERN_DEBUG "txCnt           = %08d, txCmpCnt       = %08d\n", macp->txCnt, macp->txCmpCnt);
	printk(KERN_DEBUG "retryFailCnt    = %08d, rxCnt          = %08d\n", macp->retryFailCnt, macp->rxCnt);
	printk(KERN_DEBUG "usbTxCnt        = %08d, usbTxCompCnt   = %08d\n", macp->usbTxCnt, macp->usbTxCompCnt);
    
	printk(KERN_DEBUG "regWaitRCompCnt = %08d, regWaitWCompCnt= %08d\n", macp->regWaitRCompCnt, macp->regWaitWCompCnt);
	printk(KERN_DEBUG "regRWCompCnt    = %08d, regUnCompCnt   = %08d\n", macp->regRWCompCnt, macp->regUnCompCnt);
	printk(KERN_DEBUG "regWaitRspCnt   = %08d, regRspCompCnt  = %08d\n", macp->regWaitRspCnt, macp->regRspCompCnt);
	printk(KERN_DEBUG "regRdSleepCnt   = %08d, regRspCompCnt  = %08d\n", macp->regRdSleepCnt, macp->regRspCompCnt);
    printk(KERN_DEBUG "lastRxComp = %08d\n", macp->lastRxComp);
        printk(KERN_DEBUG "macp.flags = %08x ", (u32) macp->flags);
        if(macp->bHandleNonRxTxRunning)
        {
            printk("NonRxTxRunning ");
        }

        if(test_bit(ZD1211_TX_BUSY,&macp->flags)) //B4
            printk("TX_BUSY ");
        if(test_bit(ZD1211_REQ_COMP,&macp->flags)) //B2
            printk("REQ_COMP ");
        if(test_bit(ZD1211_RUNNING,&macp->flags)) //B3
            printk("RUNNING ");
        if(test_bit(ZD1211_CMD_FINISH,&macp->flags)) //B5
            printk("CMD_FINISH ");
        printk("\n");
    printk("bDeviceInSleep:%d\n", dot11Obj.bDeviceInSleep);
    printk("intraBufCnt=%d,fabBufCnt=%d,psBufCnt[0]=%d\n",macp->intraBufCnt, macp->fabBufCnt,macp->psBufCnt[0]);
        

	zd_ShowQInfo();
	zd_ShowState();
	macp->bcnCnt = 0;
	macp->dtimCnt = 0;
	macp->rxCnt = 0;
	macp->txCmpCnt = 0;
	macp->txCnt = 0;
	macp->retryFailCnt = 0;
	macp->txIdleCnt = 0;
	macp->rxIdleCnt = 0;
	macp->hwTotalRxFrm = 0;
	macp->hwTotalTxFrm = 0;

	macp->hwRxFIFOOverrun = 0;
	macp->hwUnderrunCnt = 0;
	macp->hwCRC32Cnt =0;
	macp->hwCRC16Cnt =0;

	macp->ErrLongFrmCnt = 0;
	macp->ErrShortFrmCnt = 0;
	macp->ErrToHostFrmCnt = 0;
	macp->ErrZeroLenFrmCnt = 0;
	macp->rxOFDMDataFrame = 0;
	macp->rx11bDataFrame = 0;

	macp->ArFreeFailCnt = 0;
	macp->ArAgedCnt = 0;
	macp->ArSearchFailCnt = 0;
	macp->DropFirstFragCnt = 0;
	macp->rxNeedFragCnt = 0;
	macp->rxCompFragCnt = 0;
	macp->txQueToUpCnt = 0;
	macp->txQueSetCnt = 0;
	//macp->sleepCnt = 0;
	//macp->wakeupCnt = 0;
	macp->Continue2Rx = 0;
	macp->NoMergedRxCnt = 0;
#ifdef WPA_DEBUG
	printk("cardSet.WPAIeLen=%d\n",macp->cardSetting.WPAIeLen);
	printk("mDynKeyMode:%d,mKeyFormat:%d,mPrivacyInvoked:%d,mCap:0x%X,mWpaBcKenLen:%d\n",mDynKeyMode,mKeyFormat,mPrivacyInvoked,mCap,mWpaBcKeyLen);
#endif
	zd1205_unlock(macp);
#endif
}

void zd1205_update_brate(struct zd1205_private *macp, u32 value)
{
	u8 ii;
	u8 nRate;
	u8 *pRate;
	card_Setting_t *pSetting = &macp->cardSetting;
	u8 rate_list[4] = { 0x02, 0x04, 0x0B, 0x16 };
                                        
	/* Get the number of rates we support */
	nRate = pSetting->Info_SupportedRates[1];

	pRate = &(pSetting->Info_SupportedRates[2]);
                                                                
	for(ii = 0; ii < nRate; ii++)
	{
		/* If the Rate is less than the basic Rate, mask 0x80 with the value. */
		if((*pRate & 0x7f) <= rate_list[value])
			*pRate |= 0x80;
		else
			*pRate &= 0x7f;
		
		pRate++;
	}
}

void zd1205_dump_phy(struct zd1205_private *macp)
{
#if ZDCONF_DBGMSG_NORMAL == 1
	void *regp = macp->regp;

	u32 regValue[4];
	int i;

	LockPhyReg(&dot11Obj);
	for (i=0; i<256; i+=4) {
	    //acquire_ctrl_of_phy_req(regp);
		if (i==4)//The offset of CR4 to CR8 are not multiplied by 4 directly.
		{
			regValue[0]=zd_readl(ZD_CR4);
			regValue[1]=zd_readl(ZD_CR5);
			regValue[2]=zd_readl(ZD_CR6);
			regValue[3]=zd_readl(ZD_CR7);
		}
		else if (i==8)
		{
			regValue[0]=zd_readl(ZD_CR8);
			regValue[1]=zd_readl(ZD_CR9);
			regValue[2]=zd_readl(ZD_CR10);
			regValue[3]=zd_readl(ZD_CR11);
		}
		else
		{
			regValue[0] = zd_readl(4*i);
			regValue[1] = zd_readl(4*(i+1));
			regValue[2] = zd_readl(4*(i+2));
			regValue[3] = zd_readl(4*(i+3));
		}

		printk("CR%03d = %02x  CR%03d = %02x  CR%03d = %02x  CR%03d = %02x\n",
			i, (u8)regValue[0],  i+1, (u8)regValue[1], i+2, (u8)regValue[2], i+3, (u8)regValue[3]);
		//release_ctrl_of_phy_req(regp);
	}
	UnLockPhyReg(&dot11Obj);
#endif
}
/*
void zd1205_dump_eeprom(struct zd1205_private *macp)
{
	u32 e2pValue, e2pValue1;
	int i;

	for (i=0; i<20; i+=2) {
		e2pValue = zd_readl(E2P_SUBID+4*i);
		e2pValue1 = zd_readl(E2P_SUBID+4*(i+1));
		printk(KERN_DEBUG "0x%x = %08x,     0x%x = %08x\n", E2P_SUBID+4*i, e2pValue,  E2P_SUBID+4*(i+1), e2pValue1);
	}
}
*/
void zd1205_dump_eeprom(struct zd1205_private *macp)
{
#if ZDCONF_DBGMSG_NORMAL == 1
    u32 V1,V2,V3,V4 ;
    int i;
    char buf[128];

    for (i=0; i<0x30; i+=4) {
        V1 = zd_readl(E2P_SUBID+4*i);
        V2 = zd_readl(E2P_SUBID+4*(i+1));
                V3 = zd_readl(E2P_SUBID+4*(i+2));
                V4 = zd_readl(E2P_SUBID+4*(i+3));

        sprintf(buf,"0x%x = %08x %08x %08x %08x \n", E2P_SUBID+4*i, V1,V2,V3,V4);
        strcat(macp->zdreq.data,buf);
    }
#endif

}

void zd1205_show_card_setting(struct zd1205_private *macp)
{
#if ZDCONF_DBGMSG_NORMAL == 1
	card_Setting_t *pSetting = &macp->cardSetting;
	
  	printk(KERN_DEBUG "RTSThreshold   = %04x   FragThreshold  = %04x\n", 
  		pSetting->RTSThreshold, pSetting->FragThreshold);
	printk(KERN_DEBUG "DtimPeriod     = %04x   BeaconInterval = %04x\n", 
		pSetting->DtimPeriod, pSetting->BeaconInterval);
	printk(KERN_DEBUG "EncryMode      = %04x   EncryOnOff     = %04x\n", 
		pSetting->EncryMode, pSetting->EncryOnOff);
	printk(KERN_DEBUG "EncryKeyId     = %04x   WepKeyLen      = %04x\n", 
		pSetting->EncryKeyId, pSetting->WepKeyLen);
	printk(KERN_DEBUG "PreambleType   = %04x   AuthMode       = %04x\n", 
		pSetting->PreambleType, pSetting->AuthMode);
	printk(KERN_DEBUG "Channel        = %04x   BssType        = %04x\n", 
		pSetting->Channel, pSetting->BssType);
	printk(KERN_DEBUG "SuggestionMode = %04x   PwrState       = %04x\n",
		macp->SuggestionMode, macp->PwrState);
	printk(KERN_DEBUG "bPSMSupported  = %04x   bAssoc         = %04x\n",
		macp->bPSMSupported, macp->bAssoc);
	printk(KERN_DEBUG "bAnyActivity   = %04x   BSS_Members    = %04x\n",
		macp->bAnyActivity, macp->BSS_Members);
#endif
}
#if 0
void zd1205_dump_cam(struct zd1205_private *macp)
{
	int ii;
	
	for(ii = 0; ii < 445; ii++) {
		u32 data = HW_CAM_Read(&dot11Obj, ii);

		if((ii % 4) == 0)
			printk(KERN_ERR "\nAddr=0x%04x ", ii);

		printk(KERN_ERR "0x%08x ", data);
	}

	printk(KERN_ERR "\n");
}
#endif
void zd1205_dump_cam(struct zd1205_private *macp,u32 beginAddr, u32 length)
{
#if ZDCONF_DBGMSG_NORMAL == 1
	u8  valid_uid[40];
	u8  valid_uid_cnt;
	u32 data;
	int ii,jj;
	//u32 RollCallTblLow;
	//u32 RollCallTblHigh;
	u32 MACINCAM[6];
	u32 tmpRollCallTblLow;
	u32 tmpRollCallTblHigh;
	//u32 bDisplay;
	int UserIdBase;
	char *EncTypeStr[]={"","WEP64","TKIP","","AES","WEP128","WEP256",""};
	char *CamModeStr[]={"IBSS","AP","STA","WDS","Client","VAP","",""};

	for(ii = 0; ii < length; ii++) {
		data = HW_CAM_Read(&dot11Obj, beginAddr+ii);
		printk(KERN_ERR "\nAddr(%03u)=0x%08x", ii+beginAddr, data);
	}
	printk(KERN_ERR "\n");
	data = zd_readl(0x700); // Cam mode: MACREG(0x700)[2:0]
	printk(KERN_ERR "CAM Mode: %s\n", CamModeStr[data&7]);
	//tmpRollCallTblLow=RollCallTblLow=zd_readl(0x704);
	//tmpRollCallTblHigh=RollCallTblHigh=zd_readl(0x708)&0xf;
	tmpRollCallTblLow=zd_readl(0x704);
	tmpRollCallTblHigh=zd_readl(0x708)&0xf;
		//Scan user ID of CAM
		valid_uid_cnt=0; //Reset number of user ID
		for (ii=0; ii<40; ii++)
		{
		    valid_uid[ii]=0;// Reset to invalid
		    
		    if (ii<32)
		    {// For user 0 - 31
		        if (tmpRollCallTblLow & 1)
		        {
		    	    valid_uid[ii]=1;// set to valid
			    valid_uid_cnt++;
		        }
		        tmpRollCallTblLow = tmpRollCallTblLow >> 1;
		    }
	 	    else
	 	    {
		        if (tmpRollCallTblHigh & 1)
		        { 
		    	    valid_uid[ii]=1; // set to valid
			    valid_uid_cnt++;
		        }
		        tmpRollCallTblHigh = tmpRollCallTblHigh >> 1;
		    }
		}
	// Dump MAC address
	UserIdBase=0;
	for(ii = 0; ii < 60; ii+=6) 
	{
		//UserIdBase = UserIdBase+4;
		MACINCAM[0]=HW_CAM_Read(&dot11Obj, ii);
		MACINCAM[1]=HW_CAM_Read(&dot11Obj, ii+1);
		MACINCAM[2]=HW_CAM_Read(&dot11Obj, ii+2);
		MACINCAM[3]=HW_CAM_Read(&dot11Obj, ii+3);
		MACINCAM[4]=HW_CAM_Read(&dot11Obj, ii+4);
		MACINCAM[5]=HW_CAM_Read(&dot11Obj, ii+5);
	        for (jj=0; jj<4; jj++)
		{	
                    if (valid_uid[UserIdBase+jj])
		    {
			printk(KERN_ERR "UID:%d Mac:%02x:%02x:%02x:%02x:%02x:%02x\n",UserIdBase+jj, MACINCAM[0]&255, MACINCAM[1]&255, MACINCAM[2]&255, MACINCAM[3]&255,MACINCAM[4]&255,MACINCAM[5]&255);
	 	    }
		    MACINCAM[0]=MACINCAM[0]>>8;
		    MACINCAM[1]=MACINCAM[1]>>8;
		    MACINCAM[2]=MACINCAM[2]>>8;
		    MACINCAM[3]=MACINCAM[3]>>8;
		    MACINCAM[4]=MACINCAM[4]>>8;
		    MACINCAM[5]=MACINCAM[5]>>8;
		}
		UserIdBase = UserIdBase+4;
	}
	// Dump Encryption type: CAM location: 60-65
	//tmpRollCallTblLow=RollCallTblLow;
	//tmpRollCallTblHigh=RollCallTblHigh;
	for(ii=60; ii<66; ii++)
	{
	    data = HW_CAM_Read(&dot11Obj, ii);
	    UserIdBase=(ii-60)*8; //One location for 8 users.
	    if (UserIdBase >= 40)
	    {
	        {//location 65:For default key
	            printk(KERN_ERR "DefaultKeySet:%s\n",EncTypeStr[data&7]);
	        }
	    }
	    else
	    {
	        for(jj=0; jj<8; jj++)
	        {
		    if (valid_uid[UserIdBase+jj])
		    {
			printk(KERN_ERR "UID:%02d:%s\n",UserIdBase+jj,EncTypeStr[data&7]);
			valid_uid[UserIdBase+jj] |= ((data & 7)<<1);
		    }
                    data = data >> 4; // Next user.
	        }
	    }
        }   
	printk(KERN_ERR "KeyContents:\n");
	for (ii=0; ii<40; ii++)
	{
	    u32 keylen;
	    u32 keytype;
		
	    if (valid_uid[ii])
	    {
		keytype=valid_uid[ii]>>1;
		switch(keytype){
		case 2://TKIP
		    keylen=32;
		    break;
		case 4://AES
		    keylen=16;
		    break;
		case 1://WEP64
		    keylen=8;
		    break;
		case 5://WEP128
		    keylen=16;
		    break;
		default:
		    keylen=0;
		    break;
		}
		keylen = keylen >> 2;
		printk(KERN_ERR "UID:%02d\n", ii);
		for (jj=0; jj<keylen; jj++)
		{    
	    	    data = HW_CAM_Read(&dot11Obj, (66+(8*ii))+jj);
		    printk(KERN_ERR "%08x\n", data);
		}
	    }	
	}		
	printk(KERN_ERR "\n");
#endif
}
void zd1205_cam_read(struct zd1205_private *macp, u32 addr)
{
	u32 value = HW_CAM_Read(&dot11Obj, addr);
	printk(KERN_ERR "Addr: 0x%08x, value = 0x%08x\n", addr, value);
}

void zd1205_cam_write(struct zd1205_private *macp, u32 addr, u32 value)
{
	HW_CAM_Write(&dot11Obj, addr, value);
	printk(KERN_ERR "Write value: 0x%08x to CAM address: 0x%08x\n", value, addr);
}

void zd1205_cam_rest(struct zd1205_private *macp, int mode)
{

}

int zd1205_zd_dbg_ioctl(struct zd1205_private *macp, struct zdap_ioctl *zdreq, int *changed)
{
	void *regp = macp->regp;
	card_Setting_t *pSetting = &macp->cardSetting;
	u16 zd_cmd;
	u32 tmp_value;
	u32 tmp_addr;
	u32 CRn;
 
	zd_cmd = zdreq->cmd;

	switch(zd_cmd) {
	case ZD_IOCTL_WPS_PBC:
    {
#if ZDCONF_WPS_SUPPORT == 1
        char tag[] = "PUSH-BUTTON.indication";
        union iwreq_data wrqu;
        memset(&wrqu, 0, sizeof(wrqu));
        wrqu.data.length = strlen(tag);
        wireless_send_event(macp->device, IWEVCUSTOM,&wrqu,tag);
        sprintf(macp->zdreq.data,"PBC detected\n");

#endif
    }

		break;
	case ZD_IOCTL_HIDE_SSID:
        pSetting->HiddenSSID = 1;
        *changed = 1;
        break;
	case ZD_IOCTL_DEBUG_FLAG:
		macp->debugflag = zdreq->addr;
		mDebugFlag = zdreq->value;
		break;
	case ZD_IOCTL_REG_READ:
		LockPhyReg(&dot11Obj);
		tmp_value = zd_readl(zdreq->addr);
		UnLockPhyReg(&dot11Obj);
		zdreq->value = tmp_value;

		printk("zd1211 read register:  reg = 0x%04x, value = 0x%08x\n",
			zdreq->addr, zdreq->value);
		//if (copy_to_user(ifr->ifr_data, &zdreq, sizeof (zdreq)))
		//return -EFAULT;
		break;

	case ZD_IOCTL_REG_WRITE:
		LockPhyReg(&dot11Obj);
		zd_writel(zdreq->value, zdreq->addr);
		UnLockPhyReg(&dot11Obj); 

		if (zdreq->addr == RX_OFFSET_BYTE)
			macp->rxOffset = zdreq->value;
		break;

	case ZD_IOCTL_MEM_DUMP:
		zd1205_dump_data("mem", (u8 *)zdreq->addr, zdreq->value);
		//memcpy(&zdreq->data[0], (u8 *)zdreq->addr, zdreq->value);
		//if (copy_to_user(ifr->ifr_data, &zdreq, sizeof (zdreq)))
		//return -EFAULT;
		break;

	case ZD_IOCTL_RATE:
		/* Check for the validation of vale */
		if(zdreq->value > 3 || zdreq->value < 0)
		{
			printk(KERN_DEBUG "zd1205: Basic Rate %x doesn't support\n", zdreq->value);
			break;
		}

		printk(KERN_DEBUG "zd1205: Basic Rate = %x\n", zdreq->value);
		zd1205_update_brate(macp, zdreq->value);
		break;

#ifdef LINUX
        case ZD_IOCTL_SNIFFER:
		macp->sniffer_on = zdreq->value;
		printk(KERN_DEBUG "zd1205: sniffer_on = %x\n", macp->sniffer_on);
		zd1205_set_sniffer_mode(macp);
		break;
#endif
/* 
 	case ZD_IOCTL_CAM_DUMP://Arg1: Location, Arg2: Length
	{
		u32 startAddr, length;
		startAddr=((zdreq->addr & 0xF00)>>8)*100+
			((zdreq->addr & 0xF0)>>4)*10+
			(zdreq->addr & 0xF);
		length=((zdreq->value & 0xF00)>>8)*100+
			((zdreq->value & 0xF0)>>4)*10+
			(zdreq->value & 0xF);
 		printk(KERN_DEBUG "zd1205: dump cam\n");
 		zd1205_dump_cam(macp,startAddr,length);
 		break;
 	}
*/
        case ZD_IOCTL_DUMP_PHY:
		printk(KERN_DEBUG "zd1205: dump phy\n");
		zd1205_dump_phy(macp);
		break;
		case ZD_IOCTL_READ_PHY:
		case ZD_IOCTL_WRITE_PHY:	
			LockPhyReg(&dot11Obj);
			tmp_addr = zdreq->addr;
			CRn=    ((tmp_addr & 0xF00)>>8)*100+
				((tmp_addr & 0xF0)>>4)*10+
				(tmp_addr & 0xF);
			if (CRn >= 4 && CRn <= 8)//Special handling for CR4 to CR8
			{
				u8 cnvtbl1[]={0x20, 0x10, 0x14, 0x18, 0x1c};
				tmp_addr = cnvtbl1[CRn-4];
			}
			else
			{
				tmp_addr = CRn*4;
			}
			if (zd_cmd == ZD_IOCTL_READ_PHY)
			{	
				zdreq->value = zd_readl(tmp_addr);
                sprintf(zdreq->data,"CR%d=0x%x\n",CRn, zdreq->value);
			}
			else
			{// ZD_IOCTL_WRITE_PHY
				zd_writel(zdreq->value, tmp_addr);
			}
			UnLockPhyReg(&dot11Obj);
			break;

        case ZD_IOCTL_CARD_SETTING:
		printk(KERN_DEBUG "zd1205: card setting\n");
		zd1205_show_card_setting(macp);
		break;

#ifdef LINUX
	case ZD_IOCTL_HASH_DUMP:
		printk(KERN_DEBUG "zd1205: aid = %x\n", zdreq->value);
		zd1205_show_hash(macp, zdreq->value);
		break;    

	case ZD_IOCTL_RFD_DUMP:
		printk(KERN_DEBUG "===== zd1205 rfd dump =====\n");
		zd1205_dump_rfds(macp);
		break;
#endif
                                            
	case ZD_IOCTL_MEM_READ: {
		u32 *p;

		p = (u32 *) bus_to_virt(zdreq->addr);
		printk(KERN_DEBUG "zd1205: read memory addr: 0x%08x value: 0x%08x\n", zdreq->addr, *p);
		break;
	}

        case ZD_IOCTL_MEM_WRITE: {
		u32 *p;

		p = (u32 *) bus_to_virt(zdreq->addr);
		*p = zdreq->value;
		printk(KERN_DEBUG "zd1205: write value: 0x%08x to memory addr: 0x%08x\n", zdreq->value, zdreq->addr);
		break;
        }
         	
	case ZD_IOCTL_TX_RATE:
		printk(KERN_DEBUG "zd1205: set tx rate = %d\n", zdreq->value);

		if (zdreq->value < 0x0c){
			macp->cardSetting.FixedRate = zdreq->value;
			macp->bFixedRate = 1;
		}
		else
			macp->bFixedRate = 0;    
		break;

	case ZD_IOCTL_EEPROM:
		printk(KERN_DEBUG "zd1205: dump eeprom\n");
		zd1205_dump_eeprom(macp);
		break;

#ifdef LINUX
	/* Generate the beacon */
	case ZD_IOCTL_BCN:
		dot11Obj.dbg_cmd |= DBG_CMD_BEACON;
		printk(KERN_DEBUG "zd1205: configuration beacon\n");
		ConfigBcnFIFO();
		break;
#endif

	case ZD_IOCTL_REG_READ16:
		tmp_value = zd1211_readl(zdreq->addr, false);
		zdreq->value = tmp_value & 0xffff;
		printk(KERN_DEBUG "zd1205 read register:  reg = %4x, value = %4x\n",
			zdreq->addr, zdreq->value);
		break;

        case ZD_IOCTL_REG_WRITE16:
		tmp_value = zdreq->value & 0xffff;
		zd1211_writel(zdreq->addr, tmp_value, false);
		printk(KERN_DEBUG "zd1205 write register: reg = %4x, value = %4x\n",
                           zdreq->addr, zdreq->value);
		break;

	case ZD_IOCTL_CAM_READ:
		printk(KERN_ERR "zd1205: cam read, addr: 0x%08x\n", zdreq->addr);
		zd1205_cam_read(macp, zdreq->addr);
		break;

	case ZD_IOCTL_CAM_WRITE:
		printk(KERN_ERR "zd1205: cam write, addr: 0x%08x value: 0x%08x\n", zdreq->addr, zdreq->value);
		zd1205_cam_write(macp, zdreq->addr, zdreq->value);
		break;
	
	case ZD_IOCTL_CAM_RESET:
		printk(KERN_ERR "zd1205: reset cam\n");
		zd1205_cam_rest(macp, zdreq->value);
		break;

#if PRODUCTION == 1
	case ZD_IOCTL_CONT_TX:
        macp->ContTxEver = 1;
		zd1205_cont_tx(macp, zdreq->value);
		break;
#endif
#ifdef LINUX
    case ZD_IOCTL_STA_LIST:
        zd1205_list_STAs(macp);
        break;
    case ZD_IOCTL_SYSTEM_INFO:
        zd1205_system_info(macp);
        break;
    case ZD_IOCTL_ACS:
        macp->ACS = 1;
        break;
#endif

#if ZDCONF_TR_DBG == 1
    case ZD_IOCTL_TR_DBG:
        macp->trDbg = zdreq->addr;
        sprintf(macp->zdreq.data, "We set trDbg = %d\n", zdreq->addr);
        break;
#endif
#if ZDCONF_ANT_DIVERSITY == 1
    case ZD_IOCTL_ANTDIVERSITY:
        if(zdreq->addr == 0)
        {
            sprintf(macp->zdreq.data,"Antenna = 0\n");
            macp->bEnableSwAntennaDiv = 0;
        }
        else if(zdreq->addr == 1)
        {
            sprintf(macp->zdreq.data,"Antenna = 1\n");
            macp->bEnableSwAntennaDiv = 0;
        }
        else if(zdreq->addr == 2)
        {
            sprintf(macp->zdreq.data,"Antenna = Auto\n");
            macp->bEnableSwAntennaDiv = 1;
        }
        if(zfiHp_AntennaScheme(macp, zdreq->addr))
            printk("Set Antenna incorrect\n");

        break;
#endif

#if ZDCONF_DBGMSG_NORMAL == 1
    case ZD_IOCTL_CAM_DUMP:
        {
            int idx,idx2;
            U32 tmp;
            U64 cam_valid_bits;
            U8 mac[100];
            char buf[128];
#ifdef LINUX
            int target = macp->zdreq.addr;
#endif
            tmp = dot11Obj.GetReg(dot11Obj.reg, 0x708);
            cam_valid_bits = tmp ;
            cam_valid_bits = cam_valid_bits << 32;
            cam_valid_bits += dot11Obj.GetReg(dot11Obj.reg, 0x704);

            printk("hello 1");
            printk("\n");
            for(idx=0;idx<=43;idx++)
            {
                if(idx<40)
                    if(! (cam_valid_bits & (BIT_0 << idx)))
                        continue;
                HW_CAM_GetMAC(&dot11Obj,idx, mac);
                if(idx<40)
                    printk("%02d-", idx);
                else
                    printk("DEF%d-", idx-40);

                strcat(macp->zdreq.data, buf);
                tmp = HW_CAM_GetEncryType(&dot11Obj, idx);
                if(tmp == NO_WEP)
                    printk("%6s-", "NO_WEP");
                else if(tmp == WEP128)
                    printk("%6s-", "WEP128");
                else if(tmp == WEP64)
                    printk("%6s-", "WEP64");
                else if(tmp == TKIP)
                    printk("%6s-", "TKIP");
                else if(tmp == AES)
                    printk("%6s-", "AES");
                else
                    printk("? ? ? -");
                strcat(macp->zdreq.data,buf);

                    if(idx<40)
                    {
                        for(idx2=0;idx2<6;idx2++)
                        {
                            printk("%02x", mac[idx2]);
                            strcat(macp->zdreq.data,buf);
                            if(idx2!=5)     
                            {
                                printk(":");
                                strcat(macp->zdreq.data,buf);
                            }
                        }
                    }
                    //strcat(macp->zdreq.data," ");
                    memset(mac,0,sizeof(mac));
                    HW_CAM_GetKey(&dot11Obj, idx,16, mac);
#ifdef LINUX
                    for(idx2=0;idx2<16;idx2++)
                    {
                        printk("%02x ",  mac[idx2]);
                        strcat(macp->zdreq.data, buf);
                    }
#else
                    NetPacketDump(mac,32);
#endif
                    printk("\n");
                    //strcat(macp->zdreq.data, buf);
                
            }
            printk("hello 2");
            }
            break;


#endif
	case ZD_IOCTL_SET_MIC_CNT_ENABLE:
		dot11Obj.MIC_CNT = zdreq->value>0?1:0;
        sprintf(macp->zdreq.data, "WPA MIC Counter Measure Feature : %s\n",
                dot11Obj.MIC_CNT ? "Enable":"Disalbe");


		break;
	case ZD_IOCTL_GET_MIC_CNT_ENABLE:
        sprintf(macp->zdreq.data,"WPA MIC Counter Measure Feature : %s\n",
                dot11Obj.MIC_CNT ? "Enable":"Disalbe");

		break;
#if ZDCONF_RF_UW2453_SUPPORT == 1 || ZDCONF_RF_AR2124_SUPPORT == 1
    case ZD_IOCTL_UW_PWR :
        if(zdreq->addr == 0)
            sprintf(macp->zdreq.data, "Current Level : %d\n",dot11Obj.UWCurrentTxLevel);
        else if(zdreq->addr == 1)
        {
            sprintf(macp->zdreq.data, "Set Current Level : %d\n", zdreq->value);
            if(zdreq->value < 19 && zdreq->value >= 0)
            {
                dot11Obj.UWCurrentTxLevel = zdreq->value;
                dot11Obj.UWDefaulTxLevel  = zdreq->value;
                PHY_UWTxPower(&dot11Obj, dot11Obj.UWDefaulTxLevel);
            }
        }
        else
            sprintf(macp->zdreq.data, "Unknown Command : %d\n", zdreq->addr);

        break;
#endif
#if ZDCONF_APC == 1
    case ZD_IOCTL_APC:
        if(zdreq->addr == 0)
        {
            U8 NULL_MAC[]={0,0,0,0,0,0};
            int i,j;
            for(i=0;i<sizeof(APC_NT)/6;i++)
            {
                if(memcmp(APC_NT[i], NULL_MAC,6) != 0)
                {
                    for(j=0;j<6;j++)
                        printk("%02x ", (U8)APC_NT[i][j]);
                    printk("\n");
                }
            }
        }
        else if(zdreq->addr == 1)
        {
            macp->wds = zdreq->value;
            if(macp->wds) 
            {
                printk("Turn On WDS\n");
                dot11Obj.SetReg(&dot11Obj, 0x700, 3); 
            }
            else
            {
                printk("Turn Off WDS\n");
                dot11Obj.SetReg(&dot11Obj, 0x700, 1); 
            }
        }
        break; 
#endif

	default :
		printk(KERN_ERR "zd_dbg_ioctl: error command = %x\n", zd_cmd);
		break;
	}
    macp->DataReady = 1;

	return 0;
}    

int zd1205_wpa_ioctl(struct zd1205_private *macp, struct zydas_wlan_param *zdparm)
{
    extern U8 UpdateBcn;
	card_Setting_t *pSetting = &macp->cardSetting;
	int ret = 0;
	u8 keylen;
	u8 idx;
	u8 *pkey;
	u8 CamEncryType=0;
	//u8 tmpDynKeyMode;
    u8 bTxKey;
    u8 bClrPerStaKey=0;


    u8 mac_addr[80];

//Test write permission
    printk("%s: zdparm->cmd=%x\n", __func__, zdparm->cmd);
	switch(zdparm->cmd) {
		case ZD_CMD_SET_ENCRYPT_KEY:
				{// Dump key info:
#ifdef LINUX
					printk("SET_ENCRYPT_KEY: alg=%s key_idx=%d set_tx=%d key_len=%d ,WPAIeLen=%d, for " MACSTR "\n", zdparm->u.crypt.alg, zdparm->u.crypt.idx, zdparm->u.crypt.flags, zdparm->u.crypt.key_len, macp->cardSetting.WPAIeLen, MAC2STR(zdparm->sta_addr));
#endif
				}
                bTxKey = zdparm->u.crypt.flags;
				keylen = zdparm->u.crypt.key_len;
				idx = zdparm->u.crypt.idx;
				pkey = zdparm->u.crypt.key;

				if (!strcmp(zdparm->u.crypt.alg, "NONE")) // TODO
				{
					U8 zero_mac[]={0,0,0,0,0,0};
				
                                        CamEncryType = NO_WEP;
				//	pSetting->DynKeyMode = 0;
                                  //      pSetting->EncryMode=0;
                                    //    mKeyFormat=0;
#if ZDCONF_ADHOC_SUPPORT == 1
                    if (macp->cardSetting.BssType == INDEPENDENT_BSS)
                    {
                        WPADEBUG("Ignore del key request in IBSS\n");
                        return ret;
                    }   
#endif
					
					zd_SetKeyInfo(zdparm->sta_addr, CamEncryType, keylen, idx, pkey);
					if (zdparm->sta_addr[0] & 1)//del group key
					{
                        if ((macp->cardSetting.BssType == INFRASTRUCTURE_BSS) || macp->cardSetting.WPASupport ==0 || macp->cardSetting.WPAIeLen==0)
						{//802.1x dynamic WEP
                            WPADEBUG("deleting Group key\n");
							pSetting->DynKeyMode = mDynKeyMode = 0;
							mKeyFormat = 0;
							mPrivacyInvoked=FALSE;
							mCap &= ~CAP_PRIVACY;
                            macp->cardSetting.EncryOnOff=macp->cardSetting.EncryMode = 0;
                            macp->cardSetting.EncryKeyId=0;

						}
						mWpaBcKeyLen = mGkInstalled = 0;
					}
					else
					{
						if (memcmp(zero_mac,zdparm->sta_addr, 6)==0)
						{
							mDynKeyMode=0;
							mKeyFormat=0;
							pSetting->DynKeyMode=0;
							pSetting->EncryMode=0;
							mDynKeyMode=0;
						}
					}
					return ret;
				}
				else if (!strcmp(zdparm->u.crypt.alg, "TKIP")) 
				{
                    CamEncryType = TKIP;
					//if (idx == 0)
					{// Pairwise key
						mKeyFormat = CamEncryType;
						mDynKeyMode = pSetting->DynKeyMode = DYN_KEY_TKIP;
					}
				}
				else if (!strcmp(zdparm->u.crypt.alg, "CCMP"))
				{
                    CamEncryType = AES;
					//if (idx == 0)
					{// Pairwise key
						mKeyFormat = CamEncryType;
						mDynKeyMode = pSetting->DynKeyMode = DYN_KEY_AES;
					}
				}
				else if (!strcmp(zdparm->u.crypt.alg, "WEP"))
				{
                    WPADEBUG("**********************Set WEP key\n");
                    mPrivacyInvoked=TRUE;
                    mCap |= CAP_PRIVACY;
                    macp->cardSetting.EncryOnOff=1;

					if (keylen == 5)
					{ // WEP 64
						CamEncryType = WEP64;
						//tmpDynKeyMode=DYN_KEY_WEP64;
					}
                    else if (keylen == 13)
                    {//keylen=13, WEP 128
                        CamEncryType = WEP128;
                        //tmpDynKeyMode=DYN_KEY_WEP128;
                    }
                    else
                    {
                        CamEncryType = WEP256;
                    }

                    // For Dynamic WEP key (Non-WPA Radius), the key ID range: 0-3
                    // In WPA/RSN mode, the key ID range: 1-3, usually, a broadcast key.
                    // For WEP key setting: we set mDynKeyMode and mKeyFormat in following case:
                    //   1. For 802.1x dynamically generated WEP key method.
                    //   2. For WPA/RSN mode, but key id == 0. (But this is an impossible case)
                    // So, only check case 1.
                    if (macp->cardSetting.WPAIeLen==0)
                    //if (macp->cardSetting.WPASupport == 0)
                    {
                        WPADEBUG("set WEP Enc type =%d\n", CamEncryType); 
                        macp->cardSetting.EncryMode = CamEncryType;
                        mKeyFormat = CamEncryType;
                        //mDynKeyMode = pSetting->DynKeyMode = tmpDynKeyMode;
                        if (macp->cardSetting.BssType == INDEPENDENT_BSS)
                        {
                            mDynKeyMode = 0;
                        }
                        else if (bTxKey)
                        {
                            mDynKeyMode = 0;
                            bClrPerStaKey = 1;
                            macp->cardSetting.EncryKeyId = idx;
                        }
                    }

				}
				/* DUMP key context */
#ifdef WPA_DEBUG
				if (keylen > 0)
				{
					printk("zddebug:Key Context:\n");
                    NetPacketDump(pkey, keylen);
				}
#endif
                zd_SetKeyInfo(zdparm->sta_addr, CamEncryType, keylen, idx + (bTxKey << 7) + (bClrPerStaKey << 6), pkey);

				break;

		case ZD_CMD_SET_MLME:
				WPADEBUG("zd1205_wpa_ioctl: ZD_CMD_SET_MLME\n");

                /* Translate STA's address */
                sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x", zdparm->sta_addr[0], zdparm->sta_addr[1],
                    zdparm->sta_addr[2], zdparm->sta_addr[3], zdparm->sta_addr[4], zdparm->sta_addr[5]);

                switch(zdparm->u.mlme.cmd) {
                    case MLME_STA_DEAUTH:
                        //printk(" WPA Call zd_CmdDeauth, reason:%d\n",zdparm->u.mlme.reason_code);
                        if(zd_CmdDeauth((MacAddr_t *) zdparm->sta_addr, zdparm->u.mlme.reason_code) == FALSE)
                            printk("WPA : Can't deauthencate STA: %s\n", mac_addr);
                        else
                            printk("WPA : Deauthenticate STA: %s with reason code: %d\n", mac_addr, zdparm->u.mlme.reason_code);
                        break;

                    case MLME_STA_DISASSOC:
                        if(zd_CmdDisasoc((MacAddr_t *) zdparm->sta_addr, zdparm->u.mlme.reason_code) == FALSE)
                            printk("WPA : Can't disassociate STA: %s\n", mac_addr);
                        else
                            printk("WPA : Disassociate STA: %s with reason code: %d\n", mac_addr, zdparm->u.mlme.reason_code);
                        break;

                    default:
                        printk("WPA : MLME command: 0x%04x not support\n", zdparm->u.mlme.cmd);
                        break;
                }


				break;

		case ZD_CMD_SCAN_REQ:
				WPADEBUG("zd1205_wpa_ioctl: ZD_CMD_SCAN_REQ\n");
				break;

		case ZD_CMD_SET_GENERIC_ELEMENT:
				WPADEBUG("zd1205_wpa_ioctl: ZD_CMD_SET_GENERIC_ELEMENT\n");
				
				/* Copy the WPA IE */
				pSetting->WPAIeLen = zdparm->u.generic_elem.len;
					mmcp_memcpy(&pSetting->WPAIe, zdparm->u.generic_elem.data, pSetting->WPAIeLen);

				{
					int ii;
					
					WPADEBUG("pSetting->WPAIeLen: %d\n", pSetting->WPAIeLen);

#ifdef LINUX
					/* DUMP WPA IE */
					for(ii = 0; ii < pSetting->WPAIeLen;) {
						WPADEBUG("0x%02x ", pSetting->WPAIe[ii]);
						
						if((++ii % 16) == 0)
							WPADEBUG("\n");
					}
					WPADEBUG("\n");
#else
#ifdef WPA_DEBUG
					NetPacketDump(pSetting->WPAIe,pSetting->WPAIeLen);
#endif
#endif
				}
			#ifdef HOSTAPD_SUPPORT
#if ZDCONF_AP_SUPPORT == 1
				if (pSetting->BssType == AP_BSS)
				{// Update Beacon FIFO in the next TBTT.
					memcpy(&mWPAIe[0], pSetting->WPAIe, pSetting->WPAIeLen);
					WPADEBUG("Copy WPA IE into mWPAIe\n");
                    UpdateBcn++;
				}
#endif
			#endif

				break;

		default:
				WPADEBUG("zd1205_wpa_ioctl: default\n");
				ret = -EINVAL;
                MP_ASSERT(0);
				break;
	}

	return ret;
}
#if PRODUCTION == 1
int zd1205_cont_tx(struct zd1205_private *macp, u8 rate) 
{
			void *reg = dot11Obj.reg;
                    printk(KERN_ERR "ZDContinuousTx\n");
/*
                    if (le32_to_cpu(pZDRD->ZDRdLength) < sizeof(ZD_RD_STRUCT))
                    {
                        pZDRD->ZDRdLength = cpu_to_le32(sizeof(ZD_RD_STRUCT));
                        *BytesNeeded = sizeof(ZD_RD_STRUCT);
                        *BytesRead = 0;
                        Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                        break;
                    }
*/

//                    macp->bContinueTxMode = le32_to_cpu(pZDRD->Buffer[0]);
//                    macp->bContinueTx = le32_to_cpu(pZDRD->Buffer[1]);
                    macp->bContinueTx = rate>0xB ? 0:1;

/* Use the Fixed Rate instead of LastSentTxRate */
//macp->LastZDContinuousTxRate = macp->cardSetting.LastSentTxRate;
                    macp->LastZDContinuousTxRate = rate;//macp->cardSetting.FixedRate;

                                        if ((macp->RF_Mode == AL7230B_RF) && (mMacMode != PURE_A_MODE)) {
                                                if (macp->LastZDContinuousTxRate > 3) {
                                                        HW_Set_IF_Synthesizer(&dot11Obj, 0x00093c);
                                                }
                                                else if (macp->LastZDContinuousTxRate <= 3) {
                                                        HW_Set_IF_Synthesizer(&dot11Obj, 0x000abc);
                                                }
                                        }
#ifdef ZDCONF_BANDEDGE_ADJUST
//                                      if((macp->RF_Mode == AL7230B_RF) &&
//                                              (mMacMode == PURE_A_MODE) &&
//                                              (macp->LastZDContinuousTxRate  == 4) &&
//                                              (macp->PHY_36M_Setpoint_Flag == 0)
//                                      )
//                                              macp->PHY_36M_Setpoint_Flag = 1;
//                                  else if((macp->RF_Mode == AL7230B_RF) &&
//                                              (mMacMode == PURE_A_MODE) &&
//                                              (macp->LastZDContinuousTxRate != 4) &&
//                                              (macp->PHY_36M_Setpoint_Flag == 2)
//                                      )
//                                              macp->PHY_36M_Setpoint_Flag = 3;

                        if(dot11Obj.HWFeature & BIT_21) { //band edge adjust for calibration, the production test tool must do integration value /set point compensation for other channels which are not calibrated
				if( (macp->RF_Mode == AL7230B_RF) &&
						((4 <= macp->LastZDContinuousTxRate) && (macp->LastZDContinuousTxRate <= 9)) &&
						(mMacMode != PURE_A_MODE) &&
						(macp->cardSetting.Channel == 1 || macp->cardSetting.Channel == 11)
				  )
				{
					if(macp->cardSetting.Channel == 1)
						HW_Set_IF_Synthesizer(&dot11Obj, 0x000abc);
					LockPhyReg(&dot11Obj);
					dot11Obj.SetReg(dot11Obj.reg, ZD_CR128, 0x10);
					dot11Obj.SetReg(dot11Obj.reg, ZD_CR129, 0x10);
					UnLockPhyReg(&dot11Obj);
				}
				else if((macp->RF_Mode == AL7230B_RF) &&
						(macp->LastZDContinuousTxRate > 3) &&
						(mMacMode != PURE_A_MODE))
				{
					if((macp->LastZDContinuousTxRate > 9) ||
							(macp->cardSetting.Channel != 1 &&
							 macp->cardSetting.Channel != 11))
					{
						if(macp->cardSetting.Channel == 1)
							HW_Set_IF_Synthesizer(&dot11Obj, 0x00093c);
						LockPhyReg(&dot11Obj);
						dot11Obj.SetReg(&dot11Obj.reg, ZD_CR128, 0x14);
						dot11Obj.SetReg(&dot11Obj, ZD_CR129, 0x12);
						UnLockPhyReg(&dot11Obj);
					}
				}
				else if((macp->RF_Mode == AL7230B_RF) &&
						(macp->LastZDContinuousTxRate <=3) &&
						(mMacMode != PURE_A_MODE))
				{
					LockPhyReg(&dot11Obj);
					dot11Obj.SetReg(&dot11Obj.reg, ZD_CR128, 0x14);
					dot11Obj.SetReg(&dot11Obj, ZD_CR129, 0x12);
					UnLockPhyReg(&dot11Obj);
				}
			}

#elif !defined(ZDCONF_BANDEDGE_ADJUST)
#error "Undefined ZDCONF_BANDEDGE_ADJUST"
#endif
			// Roger 2004-11-10 , Set for Dr.Wang request , set 0x0001c4 when CCK mode with AL2230
#ifdef ZDCONF_RF_AL2230_SUPPORT
			if (dot11Obj.rfMode == AL2230_RF || dot11Obj.rfMode == AL2230S_RF)
			{
				//if (macp->cardSetting.LastSentTxRate > 3) {
				if (macp->LastZDContinuousTxRate > 3)
				{
					HW_Set_IF_Synthesizer(&dot11Obj, 0x0005a4);
				}
				else if (macp->LastZDContinuousTxRate <= 3)
				{
					HW_Set_IF_Synthesizer(&dot11Obj, 0x0001c4);
				}
			}
#endif

                    printk("ZDContinuousTx TxMode=%d TxStart=%d TxRate=0x%x\r\n",
                        macp->bContinueTxMode,
                        macp->bContinueTx,
                        macp->LastZDContinuousTxRate);

// Start
                    //if(le32_to_cpu(pZDRD->Buffer[1]) == ContTx_Start)
                    if(rate <= RATE_54M)
                    {
                        U8   tmpChr = 0;
                        U16    RateTmp= 0;
                        U32 tmpvalue;
                        U32   nLoop;
                        LockPhyReg(&dot11Obj);
                        dot11Obj.SetReg(reg, ZD1205_CR2, 0x3F);
                        dot11Obj.SetReg(reg, ZD1205_CR138, 0x28);
                        dot11Obj.SetReg(reg, ZD1205_CR33, 0x20);
// Query CR60 until change to 0x04
                        nLoop = 20;
                        while(nLoop--)
                        {
                            dot11Obj.DelayUs(10*1000);// sleep 10ms
                            tmpvalue = dot11Obj.GetReg(reg, ZD1205_CR60);

                            if(tmpvalue == 0x04)
                                break;
                        }
                        if(nLoop == 0) printk("nLoop count down to zero. But it still fails\n");

                        UnLockPhyReg(&dot11Obj);
                        //switch(le32_to_cpu(pZDRD->Buffer[0]))
			            if(1)
                        {
                            //case ContTx_Normal:   // Normal continous transmit

                                printk("Start ContTx_Normal\n");

                                macp->bContinueTx = 1;
                                LockPhyReg(&dot11Obj);
                                macp->PHYTestTimer = 0;
//ZD1205_WRITE_REGISTER(Adapter,CR122, 0xFF);  2004/10/22 mark
                                UnLockPhyReg(&dot11Obj);

                                LockPhyReg(&dot11Obj);
/* In order to avoid the uninitial length problem,
   force to set length to 0x20.
 */
                                dot11Obj.SetReg(reg, ZD1205_CR134, 0x20);

                                switch (macp->LastZDContinuousTxRate)
                                {
                                    case 4:       //6M
                                        RateTmp = 0xB;
                                        break;
                                    case 5:       //9M
                                        RateTmp = 0xF;
                                        break;
                                    case 6:       //12M
                                        RateTmp = 0xA;
                                        break;
                                    case 7:       //18M
                                        RateTmp = 0xE;
                                        break;
                                    case 8:       //24M
                                        RateTmp = 0x9;
                                        break;
                                    case 9:       //36M
                                        RateTmp = 0xD;
                                        break;

                                    case 0xA:     //48M
                                        RateTmp = 0x8;
                                        break;

                                    case 0xB:     //54M
                                        RateTmp = 0xC;
                                        break;

                                    default:
                                        RateTmp = 0;
                                        break;
                                }

                                printk("RateTmp=0x%08x\n", RateTmp);

                                if (RateTmp)
                                {
                                    dot11Obj.SetReg(reg, ZD1205_CR132, RateTmp);

//AcquireCtrOfPhyReg(Adapter);
                                    tmpvalue = dot11Obj.GetReg(reg, ZD1205_CR159);
                                    tmpvalue &= ~(BIT_0 + BIT_1 );
                                    tmpvalue |= BIT_2;
                                    dot11Obj.SetReg(reg, ZD1205_CR159, tmpvalue);

                                    UnLockPhyReg(&dot11Obj);

                                    dot11Obj.SetReg(reg, 0x644, 7);

                                    tmpvalue = dot11Obj.GetReg(reg, 0x648);
                                    tmpvalue &= ~BIT_0;
                                    dot11Obj.SetReg(reg, 0x648, tmpvalue);
                                    printk("OFDM type\n");
                                    return 0;
                                }
                             else
                                printk("CCK type\n");
                                tmpChr = macp->LastZDContinuousTxRate;
//                                printk("tmpChr=0x%x\n", tmpChr);

#if 0
                                if (macp->preambleMode == 1)
                                    macp->cardSetting.PreambleType = 0x00;
                                else if (macp->preambleMode == 2)
                                    macp->cardSetting.PreambleType = 0x20;
#endif

                                if (macp->cardSetting.PreambleType == SHORT_PREAMBLE)
                                {
// short premable
                                    tmpChr |= BIT_5;
                                }
                                else
                                {
// long premable
                                    tmpChr &= ~BIT_5;
                                }

                                if (macp->RegionCode == 0x10)
                                    tmpChr &= ~BIT_6;//USA
                                if (macp->RegionCode == 0x40)
                                    tmpChr |= BIT_6;//japan

                                dot11Obj.SetReg(reg, ZD1205_CR5, tmpChr);
                                UnLockPhyReg(&dot11Obj);

                                dot11Obj.SetReg(reg, 0x644, 3);
                             //   break;

                            //default:
                            //    printk("Continuous Tx mode: %d not support\n", pZDRD->Buffer[0]);
                            //    break;
                        }
                    }
                    else
                    {
                        U32 tmpvalue;

// Roger 2004-11-10 , Set for Dr.Wang request , set 0x0001c4 when CCK mode with AL2230
#ifdef ZDCONF_RF_AL2230_SUPPORT
                        if (dot11Obj.rfMode == AL2230_RF || dot11Obj.rfMode == AL2230S_RF)
                        {
                            HW_Set_IF_Synthesizer(&dot11Obj, 0x0005a4);
                        }
#endif

                        LockPhyReg(&dot11Obj);
                        dot11Obj.SetReg(reg, ZD1205_CR2, 0x26);
                        dot11Obj.SetReg(reg, ZD1205_CR138, 0xA8);
                        dot11Obj.SetReg(reg, ZD1205_CR33, 0x08);
                        UnLockPhyReg(&dot11Obj);
                        //switch(pZDRD->Buffer[0])
			            if(1)
                        {
                            //case ContTx_Normal:   // Normal continous transmit
                                printk("Stop Normal Continuous Transmit\n");

                                macp->bContinueTx = 0;
                                LockPhyReg(&dot11Obj);
                                macp->PHYTestTimer = 30;
//                                              ZD1205_WRITE_REGISTER(Adapter,CR122, 0x0);
                                UnLockPhyReg(&dot11Obj);

                                if (macp->LastZDContinuousTxRate >= 4)
                                {
                                    LockPhyReg(&dot11Obj);
                                    tmpvalue = dot11Obj.GetReg(reg, ZD1205_CR159);
                                    tmpvalue &= ~(BIT_0 + BIT_1 + BIT_2 );
                                    dot11Obj.SetReg(reg, ZD1205_CR159, tmpvalue);
                                    UnLockPhyReg(&dot11Obj);

                                    dot11Obj.SetReg(reg, 0x644, 0);

                                    tmpvalue = dot11Obj.GetReg(reg, 0x648);
                                    tmpvalue |= BIT_0;
                                    dot11Obj.SetReg(reg, 0x648, tmpvalue);

                                }
                                else
                                {

                                    dot11Obj.SetReg(reg, 0x644, 0);

                                    tmpvalue = dot11Obj.GetReg(reg, 0x648);
                                    tmpvalue |= BIT_0;
                                    dot11Obj.SetReg(reg, 0x648, tmpvalue);
                                }

                             //   break;
                        }

//dot11Obj.SetReg(reg, ZD_PS_Ctrl, 0x1);
//Roger 2004-11-16 SoftwareReset here to solve RX fail after TxContinue problem
                        zd1205_device_reset(macp);
                    //}
            }

    return 0;
}
#endif

#endif

