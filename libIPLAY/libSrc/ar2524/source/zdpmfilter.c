#ifndef __ZDPMFILTER_C__
#define __ZDPMFILTER_C__

#include "zd80211.h"
#include "zd1205.h"
#include "zd1211.h"
#include "zddebug.h"
#include "zdmpx.h"

extern struct net_device *g_dev;

#define DBG_USE_SERIAL_WRITE 0
#if ZDCONF_AP_SUPPORT == 1
void AgePsQ(U16 aid);
extern void zd_AP_NullData(U8 *mac, int aid);
#endif
BOOLEAN TxSend(Signal_t *signal, FrmDesc_t *pfrmDesc);
#define write_str(a,b)
U8 usedID = 0;
U8 TimBitMap[(MAX_AID/8)+2];
Signal_t *txRequest[BURST_NUM] = { NULL };
extern BOOLEAN	mcBuffered;

//make tim for beacon frame
void mkTim(Element *tim, TrafficMap_t *trf, U8 dtc, U8 dtp, U16 aidLo, U16 aidHi, BOOLEAN bc)
{
	int	i;
	U8 *map = (U8*)trf->t;
	U16	N1 = 0;
	U16	N2 = 0;
	U8	index = 0;
	
	tim->buf[0] = EID_TIM;
	tim->buf[2] = dtc;	
	tim->buf[3] = dtp;	
	
	// Calculate N1
	for (i=0; i<=(aidHi/8); i++){
		if (map[i] != 0){
			break;
		}
	}
	
	if (i>0){
		// N1 is the largest even number
		N1 = (U16)(i & ~0x01);
	}
	
	// Calculate N2
	for (i=(aidHi/8); i>=0; i--){
		if (map[i] != 0){
			break;
		}
	}
	
	if (i>0){
		N2 = (U16)i;
	}
	
	// Fill the content
	if (N2==0){
		tim->buf[4] = 0;
		tim->buf[5] = map[0];
		tim->buf[1] = 4;
	}
	else{
		tim->buf[4] = (U8)N1;
		for (i=N1; i<=N2; i++){
			tim->buf[5+index++] = map[i];
		}
		tim->buf[1] = N2-N1+4;
	}

	if (bc){
		tim->buf[4] |= 0x01;
	}
}


void TxCompleted(U32 result, U8 retID, U16 aid) //in isr routine
{
	Signal_t *signal;
	FrmInfo_t *pfrmInfo;
	FrmDesc_t *pfrmDesc;
	void *buf;
	Hash_t *pHash = NULL;
	U8	bIntraBss = 0;
	
#ifdef HOST_IF_USB
	if ((result != ZD_TX_CONFIRM) && (retID == 0xff)){
		pHash = sstByAid[aid];
		if (!pHash)
			return;
		else
			pHash->FailedFrames++;	
		
		return;	
	}	
#endif	
	
	signal = txRequest[retID];
	if (signal == NULL)
	{
	    //printk(KERN_ERR "TxCompleted: input signal is NULL\n");
	    mpDebugPrint("TxCompleted: input signal is NULL\n");
	    return;
	}
	pfrmInfo = &signal->frmInfo;
	pfrmDesc = pfrmInfo->frmDesc;

	buf = signal->buf;
    if (pfrmDesc)
	    bIntraBss = (pfrmDesc->ConfigSet & INTRA_BSS_SET);

	//if (aid)
	{
		pHash = sstByAid[aid];
		if (!pHash)
			goto no_rate_info;
			
		if (result == ZD_TX_CONFIRM){
		//for rate adaption
			pHash->SuccessFrames++;
		}	
		else  //retry failed
			pHash->FailedFrames++;

	}		
			
	
no_rate_info:
    if (pfrmDesc)
	    freeFdesc(pfrmDesc);
	pdot11Obj->ReleaseBuffer(buf);
	freeSignal(signal);
	txRequest[retID] = NULL;

	if ((buf) && (!bIntraBss)){	//upper layer data
		pdot11Obj->TxCompleted();
	}		
}	


char DbgStr1[]="Tx ProbReq";
char DbgStr2[]="Tx Auth";
char DbgStr3[]="Tx Data";
char DbgStr4[]="msg4";
char DbgStr5[]="msg5";
char DbgStr6[]="msg6";
char DbgStr7[]="msg7";
char DbgStr8[]="msg8";
char DbgStr9[]="msg9";
char DbgStr10[]="msg10";
char DbgStr11[]="msg11";

char DbgStr12[]="msg12";
BOOLEAN TxSend(Signal_t *signal, FrmDesc_t *pfrmDesc)
{	
    BOOLEAN ret = TRUE;
	FrmInfo_t *pfrmInfo;
    U32 XCount = 0;
   	U32 nextBodyLen; 
	fragInfo_t fragInfo;
	int i;
	U8 bIntraBss = 0;
	Frame_t *pf; 
	Frame_t *nextPf;
	unsigned long  flags;
	Hash_t *pHash = NULL;
	U32 *tmpiv = NULL;
	//U8 WepKeyLen = 0;
	//U8 *pWepKey = NULL;
        U8 tmp4pIv[8];
	U8 *pIv = tmp4pIv;
	U8 KeyId = 5;
	U32 iv32 = 0;
	BOOLEAN	bExtIV = FALSE;
	U8 EncryType;
	BOOLEAN	bGroupAddr = FALSE;
	U8 bWep = 0;
	U8 vapId = 0;
	U8 Num;
    U8 bDataFrm;
    //U8 KeyInstalled = 0;
    flags = pdot11Obj->EnterCS();
    if(signal == NULL)
    {
        mpDebugPrint("Wrong ... signal is NULL\n");
        pdot11Obj->ExitCS(flags);
        return FALSE;
    }
    bDataFrm = signal->bDataFrm; 


	ZDEBUG("TxSend");
		

	//txRequest[usedID] = signal;		
	memcpy(fragInfo.CalSwMic, pfrmDesc->CalMIC, MIC_LNG+1);
	pfrmInfo = &signal->frmInfo;
	bIntraBss = (pfrmDesc->ConfigSet & INTRA_BSS_SET);

	pf = pfrmDesc->mpdu;
	
	//for PS-POLL handling
	if (pfrmDesc->ConfigSet & PS_POLL_SET){
		if(mMacMode != PURE_A_MODE) {
			fragInfo.rate = RATE_1M;
			fragInfo.preamble = mPreambleType;
		}
		else if(mMacMode == PURE_A_MODE) {
			fragInfo.rate = RATE_6M;
			fragInfo.preamble = SHORT_PREAMBLE; //802.11A frame must
		}

		fragInfo.aid = 0;
		fragInfo.macHdr[0] = &pf->header[0];
		fragInfo.macBody[0] = pf->body;
		fragInfo.bodyLen[0] = 0;
		fragInfo.nextBodyLen[0] = 0;
		//fragInfo.msgID = usedID;
		fragInfo.totalFrag = 1;
		fragInfo.hdrLen = pf->HdrLen;
		fragInfo.encryType = WEP_NOT_USED;	
		fragInfo.vapId = vapId;
		fragInfo.bIntraBss = bIntraBss;
		fragInfo.buf = signal->buf;
		goto just_send;
	}

	pHash = pfrmDesc->pHash;
	bWep = wepBit(pf);
	EncryType = mKeyFormat;

   /*   if ((pf->header[0] & 0xFC) == 0x40)
        { //Probe Req, for debugging purpose.
            if (pf->body[0]==0)
            {
                if (pf->body[1] == 0)
                     printk(KERN_ERR "Probe Request with Broadcase ssid\n");
                else
                     printk(KERN_ERR "Probe Request with ssid=%s",&pf->body[2]);
            }
        }    
        else if ((pf->header[0] & 0xFC) == 0xB0)
	{
            //printk(KERN_ERR "Tx Auth\n");
	    //serial_printf(" Tx Auth\n");	
	}
        else if ((pf->header[0] & 0xFC) == 0x08)
	{
            //printk(KERN_ERR "Tx Data,keyMode=%d\n",mDynKeyMode);
            //serial_printf("Tx Data,keyMode=%d\n",mDynKeyMode);
        } */
	if (isGroup(addr1(pf))){
		bGroupAddr = TRUE;
		fragInfo.rate = pdot11Obj->BasicRate;
		fragInfo.aid = 0;
		if(PURE_A_MODE != mMacMode)
			fragInfo.preamble = 0;
		else
			fragInfo.preamble = 1;
	
		/*if ((mSwCipher) && (bWep)){
			if ((mDynKeyMode == DYN_KEY_WEP64) || (mDynKeyMode == DYN_KEY_WEP128)){
				WepKeyLen = mBcKeyLen;
				pWepKey = &mBcKeyVector[0];
				pIv = &mBcIv[1];
				KeyId = mBcKeyId;
				tmpiv = (U32 *)mBcIv;
			}
		}*/ /* The software encryption is always disabled.*/	

		if (bWep){// For 802.1x dynamic key mode
			if ((mDynKeyMode == DYN_KEY_WEP64) || (mDynKeyMode == DYN_KEY_WEP128)){
				if (mDynKeyMode == DYN_KEY_WEP64)
					EncryType = WEP64_USED;
				else
					EncryType = WEP128_USED;	
				pIv = &mBcIv[1];
				KeyId = mBcKeyId;
				tmpiv = (U32 *)mBcIv;
			}
		}
	
	}
	else{ // unicast frame		
		if (!pHash){
			// Should be Probe Response frame
			fragInfo.rate = pdot11Obj->BasicRate;
			//FPRINT_V("pHash = 0; fragInfo.rate", fragInfo.rate);
			fragInfo.aid = 0;
			if(mMacMode != PURE_A_MODE)
				fragInfo.preamble = 0;
			else
				fragInfo.preamble = 1;

		}	
		else{
			fragInfo.rate = pHash->CurrTxRate;

			//FPRINT_V("pHash != 0 fragInfo.rate", fragInfo.rate);
			fragInfo.aid = (U16)pHash->aid;
			fragInfo.preamble = pHash->Preamble;

			//if (mBssType == AP_BSS)
            {
                EncryType = pHash->encryMode;
            }

            //get pairwise key
			if (bWep)
                        {
				if ((mDynKeyMode == DYN_KEY_WEP64) || (mDynKeyMode == DYN_KEY_WEP128)){
					pIv = &pHash->wepIv[1];
					KeyId = pHash->KeyId;
					tmpiv = (U32 *)pHash->wepIv;
				}	
			}	 
		}
	}	

	if (bWep){
		if (mDynKeyMode == 0){  // static 4 keys, wep64, 128 or 256
            mIv32++;
            mWepIv[3] = (mIv32 >> 0) & 0xFF;
            mWepIv[2] = (mIv32 >> 8) & 0xFF;
            mWepIv[1] = (mIv32 >>16) & 0xFF;
			pIv = &mWepIv[1];
			KeyId = mKeyId;
			tmpiv = (U32 *)mWepIv;
			EncryType = mKeyFormat;
		}
	}
	
	Num = pfrmInfo->fTot;
	//FPRINT_V("Tx fTot", pfrmInfo->fTot);
	for (i=0; i<Num; i++){
		pf = &pfrmDesc->mpdu[i]; 
		if (Num == 1)
                {
           	        nextBodyLen = 0; 
			if (!bDataFrm)
            { //Management frame
				bIntraBss = 0;
			}		
		}		
		else
                {	
			if (Num != (i+1))
                        {
				nextPf = &pfrmDesc->mpdu[i+1];
				nextBodyLen = nextPf->bodyLen;
			}	
			else
                        {
				nextBodyLen = 0;
                        }
		}	
		
		//prepare frag information
		fragInfo.macHdr[i] = &pf->header[0];
		fragInfo.macBody[i] = pf->body;
		fragInfo.bodyLen[i] = pf->bodyLen;
		fragInfo.nextBodyLen[i] = nextBodyLen;
		
		if (bWep)
		{// Encryption is needed.
			if ((mDynKeyMode == DYN_KEY_TKIP) || (mDynKeyMode == DYN_KEY_AES))
			{// WPA encryption mode.
				if (bGroupAddr)
				{
					if (mGkInstalled)
					{
						switch(mWpaBcKeyLen)
						{
						case 32:// Group TKIP
							mIv16++;
							if (mIv16 == 0)
								mIv32++;
							pIv[0] = Hi8(mIv16);
							pIv[1] = (Hi8(mIv16) | 0x20) & 0x7f;
							pIv[2] = Lo8(mIv16);
							KeyId = mWpaBcKeyId;
							iv32 = mIv32;
							bExtIV = TRUE;
							EncryType = TKIP_USED;
							break;
						case 5: // Group WEP64
							pIv = &mBcIv[1];
							KeyId = mWpaBcKeyId;
							tmpiv = (U32 *)mBcIv;
							EncryType = WEP64_USED;
							break;
						case 13: // Group WEP128
							pIv = &mBcIv[1];
							KeyId = mWpaBcKeyId;
							tmpiv = (U32 *)mBcIv;
							EncryType = WEP128_USED;
							break;
						case 16:// Group AES
							mIv16++;
							if (mIv16 == 0)
								mIv32++;
							pIv[0] = Lo8(mIv16);
    							pIv[1] = Hi8(mIv16);
    							pIv[2] = 0;
							KeyId = mWpaBcKeyId;
							iv32 = mIv32;
							bExtIV = TRUE;
							EncryType = AES_USED;
							break;
						default: // Group key Len error
                                            		bWep=FALSE;
                                            		fragInfo.macHdr[i][1] &= ~WEP_BIT;
							break;
						}
					}
					else
					{// Group key was not installed yet.
                                            	bWep=FALSE;
                                            	fragInfo.macHdr[i][1] &= ~WEP_BIT;
					}			
				}//endof group frame.
				else 
                		{ //unicast
                                        //printk(KERN_ERR "send unicast packet,pkeyinstalled:%d\n",pHash->pkInstalled);
					//KeyInstalled = pHash->pkInstalled;
					if ((pHash) && (pHash->pkInstalled))
                    			{
						pHash->iv16++;
						if (pHash->iv16 == 0)
							pHash->iv32++;
						if (EncryType == TKIP_USED)
						{
							pIv[0]  = Hi8(pHash->iv16);
    							pIv[1]  = (Hi8(pHash->iv16) | 0x20) & 0x7f;
    							pIv[2]  = Lo8(pHash->iv16);
    						}
   						else if (EncryType == AES_USED)
   						{
    							pIv[0]  = Lo8(pHash->iv16);
    							pIv[1]  = Hi8(pHash->iv16);
    							pIv[2]  = 0;
    						}	
						KeyId = pHash->KeyId;
						iv32 = pHash->iv32;
						bExtIV = TRUE;
					}
                    			else
                    			{// No key has been installed before.
                        			bWep=FALSE;
                        			fragInfo.macHdr[i][1] &= ~WEP_BIT;
                    			}
			 	}	
			} // end of ((mDynKeyMode == DYN_KEY_TKIP) || (mDynKeyMode == DYN_KEY_AES))
			fragInfo.macHdr[i][MAC_HDR_LNG] = pIv[0];
			fragInfo.macHdr[i][MAC_HDR_LNG+1] = pIv[1];
			fragInfo.macHdr[i][MAC_HDR_LNG+2] = pIv[2];
			fragInfo.macHdr[i][MAC_HDR_LNG+3] = KeyId << 6;
            //if (mDynKeyMode != DYN_KEY_TKIP && mDynKeyMode != DYN_KEY_AES)
			if (EncryType == WEP64_USED || EncryType == WEP128_USED)
			    *tmpiv  = (((*tmpiv) & 0x00FFFFFF) + 1) | ((*tmpiv) & 0xFF000000);

 			if (bExtIV){
				fragInfo.macHdr[i][MAC_HDR_LNG+3] |= 0x20;
				fragInfo.macHdr[i][MAC_HDR_LNG+4] = (U8)(iv32);
				fragInfo.macHdr[i][MAC_HDR_LNG+5] = (U8)(iv32 >> 8);
				fragInfo.macHdr[i][MAC_HDR_LNG+6] = (U8)(iv32 >> 16);
				fragInfo.macHdr[i][MAC_HDR_LNG+7] = (U8)(iv32 >> 24);
			}	
		}
	
	}

	//fragInfo.msgID = usedID;
	fragInfo.bIntraBss = bIntraBss;
	fragInfo.buf = signal->buf;
	fragInfo.totalFrag = Num;
	fragInfo.hdrLen = MAC_HDR_LNG;
	
	if (bWep)
	{
		fragInfo.hdrLen += IV_LNG;
		if (bExtIV){
			fragInfo.hdrLen += EIV_LNG;
		}
		fragInfo.encryType = EncryType;	 
	}	
    	else
	    fragInfo.encryType=WEP_NOT_USED;

	//if (mAssoc)
	//FPRINT_V("EncryType", fragInfo.encryType);
	//fragInfo.vapId = vapId;	

	//if (fragInfo.encryType == TKIP)
	{
		//fragInfo.bWaitingMIC = pfrmDesc->bWaitingMIC;
		//fragInfo.bSwCalcMIC = pfrmDesc->bSwCalcMIC;
		//fragInfo.HwMicPhys = (U32)pfrmDesc->HwMicPhys;
	}
    
just_send:
 	//flags = pdot11Obj->EnterCS();
         // The following 5 lines must be protected by a critical section.
	fragInfo.msgID = usedID;
	txRequest[usedID] = signal;		
#if ZDCONF_LP_SUPPORT == 1
    memcpy(fragInfo.EthHdr, signal->EthHdr,14);
#endif

    ret = pdot11Obj->SetupNextSend(&fragInfo);

    do 
    {
        XCount++;
        usedID++;
        if (usedID > (BURST_NUM -1))
            usedID = 0;
        if(XCount > 1000) break;
    }while(txRequest[usedID] != NULL);
    pdot11Obj->ExitCS(flags);

    if(ret == FALSE)
    {
        mpDebugPrint("Out of resouce in setup_next_send, free the frame\n");
        zd_EventNotify(EVENT_TX_COMPLETE, ZD_TX_CONFIRM, (U32)fragInfo.msgID, fragInfo.aid);
        
    }

    if(XCount > 1000 && freeSignalCount != 0 )
    {
        for(;XCount > 0;XCount--)
            mpDebugPrint("XCount!!!!\n");
    }


	return FALSE;
}

void FlushQ(SignalQ_t *Q)
{
	Signal_t *signal;
    FrmInfo_t *pfrmInfo;
    FrmDesc_t *pfrmDesc;
    int loopCheck = 0;
	
	while((signal = sigDeque(Q)) != NULL){
        // General loop check only. for future hidden bug
        if(loopCheck++ > 100)
        {
            mpDebugPrint("infinite loop occurs in %s\n", __FUNCTION__);
            loopCheck = 0;
            break;
        }
        pfrmInfo = &signal->frmInfo;
	    pfrmDesc = pfrmInfo->frmDesc;
	    pdot11Obj->ReleaseBuffer(signal->buf);
		freeFdesc(pfrmDesc);
		freeSignal(signal);
	}
}


void TimMapSet(U8 *map, U16 aid, BOOLEAN flag)
{
	U8	mask, index;


	if ((aid == 0) || (aid > MAX_AID)) 
		return;
		
	index = aid / 8;
	mask = 0x01 << (aid % 8);
	
	if (flag)
		map[index] |= mask;
	else
		map[index] &= ~mask;	
}



BOOLEAN CleanupTxQ(void)
{
	Signal_t *signal;
	FrmInfo_t *pfrmInfo;
	FrmDesc_t *pfrmDesc;

	//PSDEBUG("CleanupTxQ");
	if (pTxQ->cnt > 0){
 		signal = pTxQ->first;
 		if (!signal)
 			return FALSE;
	 			
 		pfrmInfo = &signal->frmInfo;
 		pfrmDesc = pfrmInfo->frmDesc;

        if (!pdot11Obj->CheckTCBAvail(pfrmInfo->fTot))
          	return FALSE;
            	
 		signal = sigDeque(pTxQ);
		goto send_PduReq;
	}
	
	return FALSE;


send_PduReq:
    TxSend(signal, pfrmDesc);
    return TRUE;
}


#if ZDCONF_AP_SUPPORT == 1
BOOLEAN CleanupAwakeQ(void)
{
	Signal_t *signal;
	FrmInfo_t *pfrmInfo;
	FrmDesc_t *pfrmDesc;

	//PSDEBUG("CleanupAwakeQ");
 	if (pAwakeQ->cnt > 0){
        signal = pAwakeQ->first;
        if (!signal)
			return FALSE;
	 			
        pfrmInfo = &signal->frmInfo;

        pfrmDesc = pfrmInfo->frmDesc;
        if (!pdot11Obj->CheckTCBAvail(pfrmInfo->fTot))
          	return FALSE;
            	
 		signal = sigDeque(pAwakeQ);
		//PSDEBUG("===== Queue out awakeQ");	
		//PSDEBUG_V("pAwakeQ->cnt", pAwakeQ->cnt);	
 		goto send_PduReq;
	}
	
	return FALSE;


send_PduReq:
    TxSend(signal, pfrmDesc);
    return TRUE;
}
#endif

#if ZDCONF_AP_SUPPORT == 1
#define j2m(x) (((((x)*1000)/HZ)?(((x)*1000)/HZ):1)) //Jiffies to MS, minimal is 1.
void AgePsQ(U16 aid)
{
	Signal_t *psSignal;
	U16 interval;
    FrmDesc_t *pfrmDesc;
    U32 eol;
    FrmInfo_t *pfrmInfo;
    U16 loopCheck = 0;

	if ((aid == 0) || (aid > MAX_AID))	//Invalid AID
		return;
	

	while (pPsQ[aid]->cnt){ 
        // General Loop Check only. For future hidden bug
        if(loopCheck++ > 100)
        {
            mpDebugPrint("infinite loop occurs in %s\n", __FUNCTION__);
            loopCheck = 0;
            break;
        }

		interval = sstByAid[aid]->lsInterval; 
		if (interval == 0)
			interval = 1;
			
		psSignal = pPsQ[aid]->first;
		
		if (!psSignal)
			break;
			
		pfrmInfo = &psSignal->frmInfo;
		eol = pfrmInfo->eol;
#ifndef HOST_IF_USB 		
		if ((HW_GetNow(pdot11Obj) - eol) < (2*interval*mBeaconPeriod*1024)) //us
			break;
		
		if ((HW_GetNow(pdot11Obj) - eol) < (1024*1024)) //us
			break;	
#else
        if (j2m(HW_GetNow(pdot11Obj) - eol) < (2*interval*mBeaconPeriod)) //10ms
			break;

		if (j2m(HW_GetNow(pdot11Obj) - eol) < (100)) //10ms
			break;	
#endif		
		//Data life time-out
		psSignal = sigDeque(pPsQ[aid]); 
		if (!psSignal)	
			break;
			
		PSDEBUG_V("*****Data life time-out, AID", aid);	
        pfrmDesc = pfrmInfo->frmDesc;

        freeFdesc(pfrmDesc);
       	pdot11Obj->ReleaseBuffer(psSignal->buf);
        freeSignal(psSignal);
	}

	
	if (!pPsQ[aid]->cnt)
		TimMapSet(TimBitMap, aid, FALSE);
		
	return;
}	
#undef j2m
#endif

#if ZDCONF_AP_SUPPORT == 1
void PsPolled(MacAddr_t *sta, U16 aid)
{
	Signal_t *signal;
	FrmInfo_t *pfrmInfo;
	FrmDesc_t *pfrmDesc;
	Frame_t	*frame; 
	int i;
	U8 Num;

	//PSDEBUG("PsPolled");

	signal = sigDeque(pPsQ[aid]);
	if (!signal){
		PSDEBUG("No Queue data for PS-POLL!!!");	
		TimMapSet(TimBitMap, aid, FALSE);
        zd_AP_NullData((U8 *)&sta->mac[0], aid);
		return;
	}
	else{
        do
        {
		PSDEBUG_V("Queue out psQ, AID ", aid);	
		PSDEBUG_V("cnt ", pPsQ[aid]->cnt);
		pfrmInfo = &signal->frmInfo;
		pfrmDesc = pfrmInfo->frmDesc;

		Num = pfrmInfo->fTot;
		for (i=0; i<Num; i++){
			frame = &pfrmDesc->mpdu[i];
			//PSDEBUG_V("pfrmInfo ", (U32)pfrmInfo);
			//PSDEBUG_V("eol ", (U32)pfrmInfo->eol);
			PSDEBUG_V("pfrmDesc ", (U32)pfrmDesc);
			//PSDEBUG_V("frame ", (U32)frame);
			if (!pPsQ[aid]->cnt){
				frame->header[1] &= ~MORE_DATA_BIT;
				PSDEBUG("More bit 0");
			}	
			else {	
				frame->header[1] |= MORE_DATA_BIT;
				PSDEBUG("More bit 1");
			}	

			PSDEBUG_V("bodyLen ", frame->bodyLen);	
		}	

	
		if (!pdot11Obj->CheckTCBAvail(Num)){
            PSDEBUG("*****Fail to send out!!!");	
			PSDEBUG_V("Queue in psQ, AID", aid);
			sigEnqueFirst(pPsQ[aid], signal);
			return;
    	}
		else
			sigEnque(pTxQ, signal);
        }while( (signal=sigDeque(pPsQ[aid])) );
			return;
	}	
}	


void StaWakeup(MacAddr_t *sta)
{
	U16 aid;
    Signal_t *signal;
    U16 loopCheck = 0;
    extern zd_80211Obj_t dot11Obj;
    unsigned long flag;
    struct zd1205_private *macp=g_dev->priv;	
    struct sk_buff *skb;
    int i = 0;

	aid = AIdLookup(sta);
	if ((aid == 0) || (aid > MAX_AID))
		return;

            mpDebugPrint("A[%d]", aid);
            flag = dot11Obj.EnterCS();
            while(macp->psBuf[aid])
            {
                skb = macp->psBuf[aid];
                macp->psBuf[aid] = macp->psBuf[aid]->next;
                if(macp->psBuf[aid] == NULL) 
                    macp->last_psBuf[aid] = NULL;
                macp->psBufCnt[aid]--;
                if(macp->psBufCnt[aid] < 0)
                    mpDebugPrint("!! %s,%d\n",__func__,__LINE__);
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
                skb->next = NULL;
                if(skb->dev != g_dev)
                    mpDebugPrint("skb->dev != g_dev\n");
                //dot11Obj.ExitCS(flag);
                //flag = dot11Obj.EnterCS();

            }
            //macp->last_psBuf[aid] = NULL;
            dot11Obj.ExitCS(flag);
            defer_kevent(macp, KEVENT_DEV_QUEUE_XMIT);

            mpDebugPrint("O\n");

#if 0
    while((signal = sigDeque(pPsQ[aid])) != NULL){
        // General Loop Check only. For future hidden bug
        if(loopCheck++ > 100)
        {
            printk("infinite loop occurs in %s\n", __FUNCTION__);
            loopCheck = 0;
            break;
        }

		sigEnque(pTxQ, signal); 
	}
#endif

}
#endif


void InitPMFilterQ(void)
{
	U8	i;
	static	BOOLEAN bFirstTime = TRUE;
	
	if (bFirstTime){
		bFirstTime = FALSE;
 		for (i=0; i < MAX_RECORD; i++){
			pPsQ[i] = &psQ[i];
			initSigQue(pPsQ[i]);
		}	
		pTxQ = &txQ;
		pAwakeQ = &awakeQ;
		initSigQue(pTxQ);
		initSigQue(pAwakeQ);
	}
	else{
		for (i=0; i < MAX_RECORD; i++)
			FlushQ(pPsQ[i]);
		FlushQ(pTxQ);
		FlushQ(pAwakeQ);
	}
	
	memset(TimBitMap, 0, sizeof(TimBitMap));
}

extern BYTE myethaddr[6];

void ConfigBcnFIFO(void)
{
	int i, j;	
	BOOLEAN	bcst = FALSE;
	//__asm("break 100");
    struct zd1205_private *macp=g_dev->priv;	
	Signal_t *signal;
	U8 *tim;
    U8 *Beacon;
	U16 BcnIndex = 0;
	U16	Len;
    U16 loopCheck = 0;
    //Declare as array may cause stack overflow
    tim = ext_mem_malloc(256);//kmalloc(256, GFP_KERNEL);
    Beacon = ext_mem_malloc(MAX_BCN_LEN);//kmalloc(MAX_BCN_LEN, GFP_KERNEL);
    if(tim == NULL || Beacon == NULL)
    {
        mpDebugPrint("OOM occur in %s\n", __func__);
        return;
    }
    memset(tim, 0, 256);
    memset(Beacon, 0, MAX_BCN_LEN);


    //FPRINT("ConfigBcnFIFO");

#if ZDCONF_AP_SUPPORT == 1
	if (mBssType == AP_BSS)
    {
#if ZDCONF_FULL_TIM_FIX == 0
		if (mPsStaCnt > 0) 
#endif
        {
			HW_SetSTA_PS(pdot11Obj, 1);
			for (i=1; i < (MAX_AID+1); i++){
				AgePsQ(i);
#if ZDCONF_FULL_TIM_FIX == 0
				if (pPsQ[i]->cnt)
#endif
                {
					TimMapSet(TimBitMap, i, TRUE);
					mpDebugPrint("TimMapSet Aid", i);	
				}	
			}	
		}
#if ZDCONF_FULL_TIM_FIX == 0
		else
#endif
        {
			HW_SetSTA_PS(pdot11Obj, 0);
			//send McQ
			if (pPsQ[0]->cnt){
                //General loop check only. For future hidden bug 
				while(1){
                    if(loopCheck++ > 100)
                    {
                        mpDebugPrint("infinite loop occurs in %s\n", __FUNCTION__);
                        loopCheck = 0;
                        break;
                    }

					signal = sigDeque(pPsQ[0]);
					if (!signal)
						break;
				
					sigEnque(pTxQ, signal); 
				}
			}
		}
	}		
#endif
		
	/* make beacon frame */
	/* Frame control */
    memset(Beacon,0,sizeof(Beacon));
	Beacon[BcnIndex++] = 0x80;	
	Beacon[BcnIndex++] = 0x00; 
	
	/* Duration HMAC will fill this field */
	Beacon[BcnIndex++] = 0x00;	
	Beacon[BcnIndex++] = 0x00;
	
	/* Address1 */
	Beacon[BcnIndex++] = 0xff;	
	Beacon[BcnIndex++] = 0xff;
	Beacon[BcnIndex++] = 0xff;
	Beacon[BcnIndex++] = 0xff;
	Beacon[BcnIndex++] = 0xff;
	Beacon[BcnIndex++] = 0xff;
	
	/* Address2 */
	for (j=0; j<6; j++)
		//Beacon[BcnIndex++] = mBssId.mac[j];	
		Beacon[BcnIndex++] = myethaddr[j];//dot11MacAddress.mac[j];	
	
	/* Address3 */
	for (j=0; j<6; j++)

		Beacon[BcnIndex++] = mBssId.mac[j];
	
	/* Sequence control	HMAC will fill this field */
	//Beacon[BcnIndex++] = 0x00;	
	//Beacon[BcnIndex++] = 0x00;
	BcnIndex += 2;
	
	/* Timestamp	HMAC will fill this field */
	//for (j=0; j<8; j++)
		//Beacon[BcnIndex++] = 0x00;
	BcnIndex += 8;	
	
	/* BeaconInterval */
	Beacon[BcnIndex++] = mBeaconPeriod;
	Beacon[BcnIndex++] = mBeaconPeriod >> 8;

	/* Display the Capability */
	if(pdot11Obj->dbg_cmd & DBG_CMD_BEACON)
		mpDebugPrint("mCap: 0x%04x\n", mCap);

	/* Capability */
	Beacon[BcnIndex++] = mCap;
	Beacon[BcnIndex++] = mCap >> 8;
	
	/* SSID */
    if(mHiddenSSID)
    {
		Beacon[BcnIndex++] = EID_SSID;
		Beacon[BcnIndex++] = 1;
		Beacon[BcnIndex++] = 0;
    }
    else
    {
        Len = eLen(&mSsid)+2;
        for (j=0; j<Len; j++)
            Beacon[BcnIndex++] = mSsid.buf[j];
    }

	
	/* Supported rates */
	Len = eLen(&mBrates)+2;
	for (j=0; j<Len; j++)
		Beacon[BcnIndex++] = mBrates.buf[j];
	
	/* DS parameter */
	Beacon[BcnIndex++] = mPhpm.buf[0];	
	Beacon[BcnIndex++] = mPhpm.buf[1];	
	Beacon[BcnIndex++] = mPhpm.buf[2];	

	if (mBssType == INDEPENDENT_BSS){
		Beacon[BcnIndex++] = EID_IBPARMS;
		Beacon[BcnIndex++] = 0x2;
		Beacon[BcnIndex++] = mATIMWindow;
		Beacon[BcnIndex++] = mATIMWindow >> 8;
	}
		
	/* Tim */
	//if ((mDtimCount == 0) && (pPsQ[0]->cnt > 0)){ //dtim and buffer for mc
#if ZDCONF_AP_SUPPORT == 1
	if (mBssType == AP_BSS){
		if ((mDtimCount == 0) && mcBuffered){
			bcst = TRUE;
		}	
#if ZDCONF_FULL_TIM_FIX == 1
        bcst = TRUE;
#endif
		
		mkTim((Element*)tim, (TrafficMap_t *)&TimBitMap, mDtimCount, mDtimPeriod, 1, MAX_AID, bcst);
		Len = tim[1]+2;
		for (j=0; j<Len; j++)
			Beacon[BcnIndex++] = tim[j];
	}
#endif
    	
#if defined(OFDM)
	if (mMacMode != PURE_B_MODE && mMacMode != PURE_A_MODE)
	{

		//ERP element

		Beacon[BcnIndex++] = mErp.buf[0];
		Beacon[BcnIndex++] = mErp.buf[1];
		Beacon[BcnIndex] = mErp.buf[2];
		/*if (pdot11Obj->bDisProtection==1)
		{//Disable protection
			Beacon[BcnIndex] &= ~USE_PROTECTION;
		}*/
		BcnIndex++;

		//Extended supported rates
		Len = mExtRates.buf[1]+2;;
		for (j=0; j<Len; j++)
			Beacon[BcnIndex++] = mExtRates.buf[j];
	}		
#endif		
	
	//WPA IE
/*	if ((mDynKeyMode == DYN_KEY_TKIP) || (mDynKeyMode == DYN_KEY_AES)){
		Len = mWPAIe.buf[1]+2;
		for (j=0; j<Len; j++)
			Beacon[BcnIndex++] = mWPAIe.buf[j];
	}	*/
	Len  = macp->cardSetting.WPAIeLen;
	if (Len)
	{ 
			memcpy(&Beacon[BcnIndex], &mWPAIe[0], Len);
		BcnIndex += Len; 
	}	
		
	/* CRC32 HMAC will calucate this value */
	//for (j=0; j<4; j++)
	//	Beacon[BcnIndex++] = 0x00;
#if ZDCONF_WPS_SUPPORT == 1 && ZDCONF_AP_SUPPORT == 1
    Len  = macp->cardSetting.appieLen[IEEE80211_APPIE_FRAME_BEACON];
    if (Len)
    {
        mpDebugPrint("Update Beacon for APPIE\n");
        memcpy(&Beacon[BcnIndex], macp->cardSetting.appie[IEEE80211_APPIE_FRAME_BEACON], Len);
        BcnIndex += Len;
    }
#endif

	BcnIndex += 4;

    //if(memcmp(Beacon, macp->lastBCN, MAX_BCN_LEN) !=0)	
    {
        mpDebugPrint("Update BCN @ %lu\n",jiffies);
        HW_SetBeaconFIFO(pdot11Obj, &Beacon[0], BcnIndex);	
        //memcpy(macp->lastBCN, Beacon, MAX_BCN_LEN);
    }
	memset(TimBitMap, 0, sizeof(TimBitMap));
    //kfree(tim);
    //kfree(Beacon);
    ext_mem_free(tim);
    ext_mem_free(Beacon);

}

#if ZDCONF_AP_SUPPORT == 1
void SendMcPkt(void)
{
	Signal_t *signal;
    U16 loopCheck = 0;

	while(pPsQ[0]->cnt > 0){
        //General loop check only. For future hidden bug check
        if(loopCheck++ > 100)
        {
            mpDebugPrint("infinite loop occurs in %s\n", __FUNCTION__);
            loopCheck = 0;
            break;
        }

		signal = pPsQ[0]->first;
		if (!signal) 
			break;

		signal = sigDeque(pPsQ[0]);
		//PSDEBUG("Queue in awakeQ");	
		sigEnque(pAwakeQ, signal);
		//PSDEBUG_V("pAwakeQ->cnt", pAwakeQ->cnt);
	}
		
	if (pAwakeQ->cnt > 0){
		mcBuffered = TRUE;
	}
	else
		mcBuffered = FALSE;
}	
#endif


void ResetPMFilter(void)
{
	int i;
	
	for (i=0; i<BURST_NUM; i++)
		txRequest[i] = NULL;
		
	InitPMFilterQ();
}


BOOLEAN SendPkt(Signal_t* signal, FrmDesc_t *pfrmDesc, BOOLEAN bImmediate)
{
    Frame_t *pf;
	FrmInfo_t *pfrmInfo;
	Frame_t	*frame;
	int i;
	Hash_t *pHash;
	U8	Num;

	//PSDEBUG("SendPkt");
	pfrmInfo = &signal->frmInfo;
	frame = pfrmDesc->mpdu;
	pHash = pfrmDesc->pHash;
	Num = pfrmInfo->fTot;

	if (!signal->bDataFrm) {
		//if (!pfrmDesc->bDataFrm){
		goto direct_send;
	}
    
    goto direct_send;
#if ZDCONF_AP_SUPPORT == 1
	if (!isGroup(addr1(frame))) { //unicast
		PsMode dpsm;
		U16 aid;

		if (mBssType != AP_BSS)
			goto direct_send;
            
		//PsInquiry(addr1(frame), &dpsm, &aid);
		dpsm = pHash->psm;
		aid = pHash->aid;

        pf = pfrmDesc->mpdu;

        if ((dpsm == PSMODE_POWER_SAVE) && (aid > 0) && (aid <(MAX_AID+1)) && (baseType(pf) != ST_NULL_FRAME)) {
			AgePsQ(aid);

			if (zd_CheckTotalQueCnt() > TXQ_THRESHOLD) {
				PSDEBUG("*****Drop PS packet*****");
				freeFdesc(pfrmDesc);
				pdot11Obj->ReleaseBuffer(signal->buf);
				freeSignal(signal);
				return FALSE;
			}
			else {
				//for (i=0; i<Num; i++){
				//	setMoreData((&pfrmDesc->mpdu[i]), 1);
				//  pfrmDesc->mpdu[i].header[i] |= MORE_DATA_BIT;
				//}	
				pfrmInfo->eol = HW_GetNow(pdot11Obj);	//Set timestamp

				sigEnque(pPsQ[aid], signal);			//Queue in PS Queue
				PSDEBUG_V("Queue in PS Queue, AID ", aid);
				PSDEBUG_V("cnt ", pPsQ[aid]->cnt);
				//PSDEBUG_V("pfrmInfo ", (U32)pfrmInfo);
				//PSDEBUG_V("eol ", (U32)pfrmInfo->eol);
				PSDEBUG_V("pfrmDesc ", (U32)pfrmDesc);
				//PSDEBUG_V("frame ", (U32)frame);
				PSDEBUG_V("bodyLen ", frame->bodyLen);
				TimMapSet(TimBitMap, aid, TRUE);
				return FALSE;
			}
		}
		else {			
			goto direct_send;
        	}
	}
	else{   //group address
		if ((orderBit(frame) == 0) && (mPsStaCnt > 0)){
#if ZDCONF_DTIM_BC_FIX == 1
            goto direct_send;
#endif            

			if ((zd_CheckTotalQueCnt() > TXQ_THRESHOLD) || (pPsQ[0]->cnt > MCQ_THRESHOLD)){
				PSDEBUG("*****Drop MC packet*****");
				freeFdesc(pfrmDesc);
				pdot11Obj->ReleaseBuffer(signal->buf);
				freeSignal(signal);
				return FALSE;
			}
			else{
				for (i=0; i<Num; i++){
					pfrmDesc->mpdu[i].header[1] |= MORE_DATA_BIT;
				}	

				sigEnque(pPsQ[0], signal);	// psQ[0] is for mcQ
				//PSDEBUG("+++++ Queue in mcQ");
				//PSDEBUG_V("mcQ->cnt", pPsQ[0]->cnt);
				return FALSE;
			}
		}
		else{
			goto direct_send;
		}
	}
#endif


direct_send:
	if (!bImmediate){
        mpDebugPrint("!! RUN NOT IMMEDIATE !!\n");
		sigEnque(pTxQ, signal);
		return FALSE;
	}
	else{	
		if (!pdot11Obj->CheckTCBAvail(Num)){
		#if 1  //727
			freeFdesc(pfrmDesc);
            pdot11Obj->ReleaseBuffer(signal->buf);
			freeSignal(signal);
			return FALSE;
		#else
			sigEnque(pTxQ, signal);
			return FALSE;
		#endif 
			//PSDEBUG("Queue in TxQ");
			//PSDEBUG_V("Cnt of TxQ", pTxQ->cnt);
    		}
		return TxSend(signal, pfrmDesc); //14 us
	}
}

#endif

