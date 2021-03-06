#ifndef _ZDHCI_C_
#define _ZDHCI_C_
#include "typedef.h"
#include "ndebug.h"
#include "zdmpx.h"
#include "zd80211.h"
#include "zdhci.h"
#include "zdequates.h"
#include "zd1205.h"
#include "zd1211.h"
#include "zddebug.h"

#define MAX_CHANNEL_ALLOW				13

BOOLEAN zd_PseudoIbssConnect(void);
extern U8 UpdateBcn ;

static U8 zd_Snap_Header[6] = {0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00};
static U8 zd_SnapBridgeTunnel[6] = {0xAA, 0xAA, 0x03, 0x00, 0x00, 0xF8};
static U8 zd_Snap_Apple_Type[] = {0xAA,0xAA,0x03,0x08,0x00,0x07,0x80,0x9b};
static U8 zd_Snap_Apple_AARP[] = {0xAA,0xAA,0x03,0x00,0x00,0x00,0x80,0xf3};

extern void zd1205_notify_scan_done(struct zd1205_private *macp);
extern BOOLEAN AP_sendNullDataFrame(Signal_t *signal, FrmDesc_t *pfrmDesc, MacAddr_t *addr1, int aid);

extern struct net_device *g_dev;
zd_80211Obj_t *pdot11Obj = 0;
#if ZDCONF_80211A_SUPPORT == 1
extern const U16 dot11A_Channel[];
extern const U16 dot11A_Channel_Amount;
#endif
extern void ChangeMacMode(u8 MAC_Mode,u8 Channel);
extern U32     mTimeBeforeAdhocRoaming;


U16 CurrScanCH = 1;
U8 UpdateBcn = 0;

#ifdef LINUX
BOOLEAN zd_SendPkt(U8 *pEthHdr, U8 *pBody, U32 bodyLen, void *buf, U8 bEapol, void *pHash)
{
    struct zd1205_private *macp = (struct zd1205_private *)g_dev->priv;
	Signal_t *signal;
#if ZDCONF_LP_SUPPORT == 1
    U8 EthHdr[14];
#endif
	FrmDesc_t *pfrmDesc;
	Frame_t *frame;
	U8	vapId = 0;
	//FPRINT("zd_SendPkt");
	if (mPsStaCnt){
		if (zd_CheckTotalQueCnt() > TXQ_THRESHOLD){
			//FPRINT("Drop Tx packet");
			return FALSE; 
		} 
	}

	signal = allocSignal();

	if (!signal){
		FPRINT("zd_SendPkt out of signal");
		FPRINT_V("freeSignalCount", freeSignalCount);
		return FALSE;
	}
#if ZDCONF_LP_SUPPORT == 1
    memcpy(signal->EthHdr,pEthHdr,14);
#endif
	
	pfrmDesc = allocFdesc();
	
	if (!pfrmDesc){
		freeSignal(signal);
		FPRINT("zd_SendPkt out of description");
		FPRINT_V("freeFdescCount", freeFdescCount);
		return FALSE;
	}
	
	frame = pfrmDesc->mpdu;
	/*	FrameControl(2) Duration/ID(2) A1(6) A2(6) A3(6) Seq(2) A4/LLCHdr(6) LLCHdr(6) */
#if ZDCONF_AP_SUPPORT == 1
	if (mBssType == AP_BSS){
		memcpy((char *)&(frame->header[4]), (char *)&(pEthHdr[0]), 6);		/* Set DA to A1 */
		memcpy((char *)&(frame->header[16]), (char *)&(pEthHdr[6]), 6);		/* Set SA to A3 */
		frame->header[1] = FROM_DS_BIT;
	}	
#endif
	if (mBssType == INFRASTRUCTURE_BSS){
		memcpy((char *)&(frame->header[4]), (char *)&mBssId, 6);			/* Set BSSID to A1 */
		memcpy((char *)&(frame->header[16]), (char *)&(pEthHdr[0]), 6);		/* Set DA to A3 */
		frame->header[1] = TO_DS_BIT;
		if (mPwrState  && macp->bAssoc){
			frame->header[1] |= PW_SAVE_BIT;
		}
		else
			frame->header[1] &= ~PW_SAVE_BIT;
	}	
#if ZDCONF_ADHOC_SUPPORT == 1 || ZDCONF_PSEUDO_SUPPORT == 1
	else if ((mBssType == INDEPENDENT_BSS) || (mBssType == PSEUDO_IBSS)){
		memcpy((char *)&(frame->header[4]), (char *)&(pEthHdr[0]), 6);		/* Set DA to A1 */
		memcpy((char *)&(frame->header[16]), (char *)&mBssId, 6);			/* Set Bssid to A3 */
		frame->header[1] = 0;
	}
#endif

	frame->bodyLen = bodyLen;
	frame->body = pBody;
	signal->buf = buf;
	signal->vapId = vapId;
	pfrmDesc->ConfigSet &= ~INTRA_BSS_SET;
	frame->HdrLen = MAC_HDR_LNG;
	frame->header[0] = ST_DATA;
    if(!macp->wds)
        setAddr2(frame, &dot11MacAddress);
    else
        setAddr2(frame, &pEthHdr[6]);


	if (bEapol){
		pfrmDesc->ConfigSet |= EAPOL_FRAME_SET;
	}	
	else
		pfrmDesc->ConfigSet &= ~EAPOL_FRAME_SET;	
		
	signal->bDataFrm = 1;
	//pfrmDesc->bDataFrm = 1;
	pfrmDesc->pHash = (Hash_t *)pHash;
    if (pHash == NULL && !(pEthHdr[0]&BIT_0) )
    	printk(KERN_ERR "===== ==== ===pHash is NULL in zd_SendPkt\n"); 
	mkFragment(signal, pfrmDesc, pEthHdr); //10 us
#if 0
	//force free for debug only
	zd1205_dump_data("header", (u8 *)&frame->header[0], frame->HdrLen);
	zd1205_dump_data("body", (u8 *)frame->body, frame->bodyLen);
	freeSignal(signal);
	freeFdesc(pfrmDesc);
	return TRUE;
#endif
       

#if 1
	if (SendPkt(signal, pfrmDesc, TRUE))  //4727 
		return FALSE;
	else 
		return TRUE;
#else
	SendPkt(signal, pfrmDesc, TRUE);
	return TRUE;
#endif    
}
#endif

#if ZDCONF_DRV_FORWARD == 1
#define	LP_FORWARD		0
#define	BC_FORWARD		1
#define BSS_FORWARD		2	
#ifdef LINUX
void zd_WirelessForward(U8 *pHdr, U8 *pBody, U32 len, void *buf, U8 mode, void *pHash, U8 *pEthHdr)
{
	Signal_t *signal;
	FrmDesc_t *pfrmDesc;
	Frame_t *frame;
	U8	vapId = 0;
	//FPRINT("zd_WirelessForward");
    
	if (mPsStaCnt){
		if (zd_CheckTotalQueCnt() > TXQ_THRESHOLD){
			//FPRINT("Drop Intra-BSS packet");
			pdot11Obj->ReleaseBuffer(buf); 
			return;  
		}
	}	

	signal = allocSignal();
	if (!signal){
    	FPRINT("zd_WirelessForward out of signal");
    	FPRINT_V("freeSignalCount", freeSignalCount);
    	pdot11Obj->ReleaseBuffer(buf);
		return;
	}	
	
	pfrmDesc = allocFdesc();
	if (!pfrmDesc){
		freeSignal(signal);
		FPRINT("zd_WirelessForward out of description");
		FPRINT_V("freeFdescCount", freeFdescCount);
		pdot11Obj->ReleaseBuffer(buf);
		return;
	}
	
	frame = pfrmDesc->mpdu;
	/* FrameControl(2) Duration/ID(2) A1(6) A2(6) A3(6) Seq(2) A4/LLCHdr(6) LLCHdr(6) */
	memcpy((char *)&(frame->header[4]), (char *)&(pHdr[16]), 6);	/* Set DA to A1 */
	memcpy((char *)&(frame->header[16]), (char *)&(pHdr[10]), 6);	/* Set SA to A3 */
#if ZDCONF_LP_SUPPORT == 1
    memcpy(signal->EthHdr, (char *)&(frame->header[4]),6);
    memcpy(signal->EthHdr+ETH_ALEN, (char *)&(frame->header[16]),6);
#endif
	
	frame->bodyLen = len;
	frame->body = pBody;
	signal->buf = buf;
	signal->vapId = vapId;
	
	if (mode == LP_FORWARD){
		memcpy((char *)&(frame->header[4]), (char *)&(pHdr[10]), 6);	/* Set DA to A1 */
		memcpy((char *)&(frame->header[16]), (char *)&dot11MacAddress, 6);	/* Set SA to A3 */
		frame->body[6] = 0x38;
		frame->body[7] = 0x39;
	}	
	
	pfrmDesc->ConfigSet |= INTRA_BSS_SET;
	pfrmDesc->ConfigSet &= ~EAPOL_FRAME_SET;
	signal->bDataFrm = 1;
	//pfrmDesc->bDataFrm = 1;
	frame->HdrLen = MAC_HDR_LNG;
	frame->header[0] = ST_DATA;
	frame->header[1] = FROM_DS_BIT;
	setAddr2(frame, &dot11MacAddress);
	pfrmDesc->pHash = (Hash_t *)pHash;
	mkFragment(signal, pfrmDesc, pEthHdr);
	SendPkt(signal, pfrmDesc, FALSE);
	return; 
}
#endif
#endif

#ifdef LINUX
void zd_SendDeauthFrame(U8 *sta, U8 ReasonCode)
{
	Signal_t *signal;
	
        printk(KERN_ERR "SendDeauthFrame with ReasonCode=%u\n",ReasonCode);
#if ZDCONF_ADHOC_SUPPORT == 1 || ZDCONF_PSEUDO_SUPPORT == 1
	if ((mBssType == INDEPENDENT_BSS) || (mBssType == PSEUDO_IBSS))
		return;
#endif
	
	if ((signal = allocSignal()) == NULL)  
		return;
	signal->id = SIG_DEAUTH_REQ;
	signal->block = BLOCK_AUTH_REQ;
	signal->vapId = 0;
	memcpy(&signal->frmInfo.Sta, sta, 6);
	signal->frmInfo.rCode =  ReasonCode;
	sigEnque(pMgtQ, (signal));
	
	return;
}

void zd_SendClass2ErrorFrame(MacAddr_t *sta, U8 vapId)
{
	Signal_t *signal;
	
	//FPRINT("zd_sendClass2ErrorFrame");
	
#if ZDCONF_ADHOC_SUPPORT == 1 || ZDCONF_PSEUDO_SUPPORT == 1
	if ((mBssType == INDEPENDENT_BSS) || (mBssType == PSEUDO_IBSS))
		return;
#endif
	
	if ((signal = allocSignal()) == NULL)  
		return;
	signal->id = SIG_DEAUTH_REQ;
	signal->block = BLOCK_AUTH_REQ;
	signal->vapId = vapId;
	memcpy(&signal->frmInfo.Sta, sta, 6);
	signal->frmInfo.rCode = RC_CLASS2_ERROR;
	sigEnque(pMgtQ, (signal));
	
	return;
}	

void zd_SendClass3ErrorFrame(MacAddr_t *sta, U8 vapId)
{
	Signal_t *signal;
	
	//FPRINT("zd_SendClass3ErrorFrame");
#if ZDCONF_ADHOC_SUPPORT == 1 || ZDCONF_PSEUDO_SUPPORT == 1
	if ((mBssType == INDEPENDENT_BSS) || (mBssType == PSEUDO_IBSS))
 		return;
#endif
		
	if ((signal = allocSignal()) == NULL)  
		return;
		
	signal->id = SIG_DIASSOC_REQ;
	signal->block = BLOCK_ASOC;
	signal->vapId = vapId;
	memcpy(&signal->frmInfo.Sta, sta, 6);
	signal->frmInfo.rCode = RC_CLASS3_ERROR;
	sigEnque(pMgtQ, (signal));
	
	return;
}	
#endif

#define MIC_HEADER_LEN	16
#if ZDCONF_MIC_CHECK == 1
BOOLEAN zd_CheckMic(U8 *pHdr, U8 *pBody, U32 bodyLen, Hash_t *pHash, U8 *pEthHdr)
{
	MICvar pRxMicKey;
	U8 PkInstalled = 0;
	U8 *pByte;
	U8 CalMic[8];
	int i = 0;
	U8 *pIV = pHdr + 24;
    U8 keyId ;

	/* Always return TRUE, 4D06 */
	//return TRUE;
	//if (!pHash)
	//	return FALSE;
	
	if (pIV[3] & EIV_BIT)
        {
        keyId =  (pIV[3] & (BIT_6|BIT_7)) >> 6;
        //printk("check mic key id =%d\n",keyId);
		if (pHdr[4] & 1) // Use group key
		{
			if (mGkInstalled)
				memcpy(&pRxMicKey, &mBcMicKey[keyId],sizeof(pRxMicKey));
			else
				return FALSE;
		}
		else // Use Unicast key
		{
			if (!pHash)
				return FALSE;
			if ((PkInstalled=pHash->pkInstalled))
				memcpy(&pRxMicKey , &pHash->RxMicKey, sizeof(pRxMicKey));
			else
				return FALSE;
		}
				
		//zd1205_dump_data("IV = ", pIV, 8);
		//zd1205_dump_data("MIC K0= ", (U8 *)&pRxMicKey->K0, 4);
		//zd1205_dump_data("MIC K1= ", (U8 *)&pRxMicKey->K1, 4);
		//pRxMicKey = &pHash->RxMicKey;	

		//PkInstalled = pHash->pkInstalled;	
					
		if (1){
			U32 Len = bodyLen - MIC_LNG;
#if 0			
			void *reg = pdot11Obj->reg;
			U32 BolckLen = 0;
			U32 tmpValue = 0;
			U32 BlockNum = 2;
			U32 MicLow, MicHigh;
			U32 MicStatus = 0;
			U32 HwMicStatus = 0;
			register int j = 0;
			U32 RxMicWrBackAddr = (U32)pEthHdr + MIC_HEADER_LEN;
			U32 HwMicHighPhys = RxMicWrBackAddr + 4;
			U32 HwMicStatusPhys = HwMicHighPhys + 4;
				
			//FPRINT("************* RX MIC ****************");	
			
			//reser HW MIC status
			memset(RxMicWrBackAddr, 0x11, 12);
			
			//wait last MIC finish, then start this one
			MicStatus = pdot11Obj->GetReg(reg, ZD_MIC_STATUS);
			while (MicStatus & MIC_BUSY){
				pdot11Obj->DelayUs(1);
				MicStatus = pdot11Obj->GetReg(reg, ZD_MIC_STATUS);
				j++;
				if (j>1000){
					bMicFinish = FALSE;	
					//FPRINT("Rx MIC can't start !!!");
					//FPRINT_V("MicStatus", MicStatus);
					zdCnt.RxMicNoStart++;
					break;
				}
			}	
			
			//set mic key	
			pdot11Obj->SetReg(reg, ZD_MIC_KEY_LOW, pRxMicKey->K0);
			pdot11Obj->SetReg(reg, ZD_MIC_KEY_HIGH, pRxMicKey->K1);
									
			//set 802.3 header
			pdot11Obj->SetReg(reg, ZD_MIC_START_ADDR0, (U32)pEthHdr);
			pdot11Obj->SetReg(reg, ZD_MIC_BLOCK0_LEN, MIC_HEADER_LEN);
					
			//set mac body
			pdot11Obj->SetReg(reg, ZD_MIC_START_ADDR1, (U32)pBody);
			pdot11Obj->SetReg(reg, ZD_MIC_BLOCK1_LEN, Len);
			
			//set write back address
			pdot11Obj->SetReg(reg, ZD_MIC_WRITE_BACK_ADDRS, (U32)RxMicWrBackAddr);
					
			BolckLen = MIC_HEADER_LEN + Len;
			tmpValue = (BlockNum | (BolckLen << 16));
			pdot11Obj->SetReg(reg, ZD_MIC_TOTAL_BLOCK_NUM, tmpValue);
			
			// busy waiting MIC finish	
			j= 0;				
			HwMicStatus = zd_le32_to_cpu(*(U32 *)(HwMicStatusPhys));
			while (HwMicStatus != HW_MIC_FINISH){
				pdot11Obj->DelayUs(1);
				HwMicStatus = zd_le32_to_cpu(*(U32 *)(HwMicStatusPhys));
				j++;
				if (j>1000){
					bMicFinish = FALSE;	
					//FPRINT("Rx MIC not finish !!!");
					//FPRINT_V("HwMicStatus", HwMicStatus);
					zdCnt.RxMicNoFinish++;
					break;
				}
			}		
							
			MicLow = zd_le32_to_cpu (*(U32 *)RxMicWrBackAddr);	
			MicHigh = zd_le32_to_cpu (*(U32 *)(HwMicHighPhys));
			
			pByte = pBody + Len; //point to MIC start
			CalMic[0] = (U8) MicLow;
			CalMic[1] = (U8) (MicLow >> 8);
			CalMic[2] = (U8) (MicLow >> 16);
			CalMic[3] = (U8) (MicLow >> 24);
			CalMic[4] = (U8) MicHigh;
			CalMic[5] = (U8) (MicHigh >> 8);
			CalMic[6] = (U8) (MicHigh >> 16);
			CalMic[7] = (U8) (MicHigh >> 24);
#else
			//Software MIC Calculation, HW MIC failed
			MICclear(&pRxMicKey);
	
			//pByte = pEthHdr;
			if (mBssType == INFRASTRUCTURE_BSS || 
                            mBssType == INDEPENDENT_BSS)
			   pByte = &pHdr[4]; // DA = Addr1 
			else //if (mBssType == AP_BSS)
			   pByte = &pHdr[16]; //DA = Addr3
			
			for (i=0; i<6; i++){
				MICappendByte(*pByte++, &pRxMicKey);
			}
			if (mBssType == AP_BSS ||
                            mBssType == INDEPENDENT_BSS)
			   pByte = &pHdr[10]; // SA=Addr2
			else // if (mBssType == INFRASTRUCTURE_BSS)
			   pByte = &pHdr[16]; // SA=Addr3
                        for (i=0; i<6; i++){
				MICappendByte(*pByte++, &pRxMicKey);
			}
	                MICappendByte(0,&pRxMicKey);//priority
			MICappendByte(0,&pRxMicKey);//3 zeros
			MICappendByte(0,&pRxMicKey);
			MICappendByte(0,&pRxMicKey);
		
			pByte = pBody;
			for (i=0; i<Len; i++){
				MICappendByte(*pByte++, &pRxMicKey);
			}
			
			MICgetMIC(CalMic, &pRxMicKey); // Append MIC (8 byte)
			
#endif			
			//FPRINT_V("Calcu HW MIC", RxCompLogBuf[RxComplogPktCnt][10]-RxCompLogBuf[RxComplogPktCnt][9]);
															
			// now pBye point to MIC area
			if (pdot11Obj->MIC_CNT && memcmp(CalMic, pByte, MIC_LNG) != 0){
				zd1205_dump_data("pHdr = ", pHdr, 32);
				//FPRINT_V("Body Addr", (U32)pBody);
				zd1205_dump_data("pBody = ", pBody, bodyLen+16);
				zd1205_dump_data("CalMic = ", CalMic, 8);
				zd1205_dump_data("ReceMic = ", pByte, 8);

				printk(KERN_ERR "SW MIC Check fail\n");
				hostap_michael_mic_failure((struct zd1205_private *)g_dev->priv, (struct hostap_ieee80211_hdr *)pHdr, pIV[3] & KEYID_MASK);
				//pdot11Obj->MicFailure(&pEthHdr[6]);
				return FALSE;
			}
			else{
				//FPRINT("***** MIC success *****");
				//printk(KERN_ERR "SW MIC check OK\n");
				return TRUE;
			}
		}	
	}
	
	return FALSE;


}	
#endif

#ifdef LINUX
void zd_ReceivePkt(U8 *pHdr, U32 hdrLen, U8 *pBody, U32 bodyLen, void *buf, U8 *pEthHdr, rxInfo_t *pRxInfo, const BOOLEAN LP)
{
	U8 *x;
	Signal_t *pRxSignal;
	FrmDesc_t *pRxFdesc;
	Frame_t *pRxFrame;
	MacAddr_t *pDa, *pSa;
	StationState sas;
	PsMode psm = PSMODE_STA_ACTIVE;
	U32 dataLen = 0xFFFFFFFF;
	U8 *pData = NULL;
	U8 mode;
	void *bcBuf;
	U8 *pBcData;
	Hash_t *pHash;
	U8 vapId = 0;
	U8 rate = pRxInfo->rate;
	U8 bDataFrm = pRxInfo->bDataFrm;
	U8 SaIndex = pRxInfo->SaIndex;		
#if ZDCONF_SIGNAL_INFO == 1
	U8 signalStrength = pRxInfo->signalStrength;
	U8 signalQuality = pRxInfo->signalQuality;
#endif
	U8 bSwCheckMIC = pRxInfo->bSwCheckMIC;
    U32 LP_MAP = 0;
#if ZDCONF_LP_SUPPORT == 1
// ####### FOR A-MSDU ########
	U16 wCnt;
    struct sk_buff *skb = NULL;
    Signal_t *signal;
    FrmDesc_t *pfrmDesc;
    Frame_t *frame;
    U16 dataOff = 0;
    U16 idx, pktLen,idx2;
    int j=0;
    Hash_t *sHash = NULL;
    U8 HDR[24];
    U8 *pBody2; 
    U32 len2;
// ###########################


    if(LP)
    {
        LP_MAP = 0xFFFFFFFF;
    }
#endif

	
	ZDEBUG("zd_ReceivePkt");
	

	if (mBssType == INFRASTRUCTURE_BSS){
		pDa = (MacAddr_t *)&pHdr[4];  //A1 will be my MAC
		//pSa = (MacAddr_t *)&pHdr[16]; //A3
		pSa = (MacAddr_t *)&pHdr[10]; //A2 for Asoc status check
		pHash = sstByAid[0];
        pHash->RxRate = rate;
	}
#if ZDCONF_AP_SUPPORT == 1
	else if (mBssType == AP_BSS){
		pDa = (MacAddr_t *)&pHdr[16]; //A3
		pSa = (MacAddr_t *)&pHdr[10]; //A2
		if (bDataFrm){
			//don't care PS Bit in authenticate, (Re)assoicate and Probe Reguest frame
			psm = (PsMode)((pHdr[1] & PW_SAVE_BIT) ? PSMODE_POWER_SAVE : PSMODE_STA_ACTIVE);
		}	
		if (SaIndex == 0)
			pHash = RxInfoIndicate(pSa, psm, rate); //12us update ps and rate information 
		else { 
			pHash = sstByAid[SaIndex];
			if (pHash)
				RxInfoUpdate(pHash, psm, rate);
		}
	}
#endif
#if ZDCONF_ADHOC_SUPPORT == 1 || ZDCONF_PSEUDO_SUPPORT == 1
	else{ // INDEPENDENT_BSS or PSEUDO_IBSS
		pDa = (MacAddr_t *)&pHdr[4];  //A1
		pSa = (MacAddr_t *)&pHdr[10]; //A2
		pHash = RxInfoIndicate(pSa, 0, rate);
	}		
#endif
#if ZDCONF_LP_SUPPORT == 1
    if(pHash) pHash->LP_CAP = LP;
#endif

	
	if (bDataFrm){
		if (!bodyLen)
			goto rx_release;
		
		if (!pHash){
			zd_SendClass2ErrorFrame(pSa, vapId);
			goto rx_release;
		}
		else {
			sas = pHash->asoc;
#if ZDCONF_AP_SUPPORT == 1
			if ((sas != STATION_STATE_ASOC) && (mBssType == AP_BSS)){
			//if (sas != STATION_STATE_ASOC){
				zd_SendClass3ErrorFrame(pSa, vapId);
				printk(KERN_ERR "Class3ErrFrm:%02X %02X %02X %02X %02X %02X\n",pSa->mac[0],pSa->mac[1],pSa->mac[2],pSa->mac[3],pSa->mac[4],pSa->mac[5]);
 				goto rx_release;
			}	 
#endif
 		}	

		if (sas == STATION_STATE_ASOC)
        { //association station
#if ZDCONF_AP_SUPPORT == 1
			if (mBssType == AP_BSS)
            {
				if (isGroup(pDa))
                {
   					if (pHash->keyLength == 32)
                    {
						if (!pHash->pkInstalled)
							goto rx_release;
						else if (!mGkInstalled)
							goto rx_release;
						else if ((pHdr[1] & WEP_BIT) && (hdrLen == 32))
                        {
							if (bSwCheckMIC)
                            {
#if ZDCONF_MIC_CHECK == 1
								if (!zd_CheckMic(pHdr, pBody, bodyLen, pHash, pEthHdr))
                                {
									goto rx_release;
								}
								else 
#endif
                                {
									bodyLen -= MIC_LNG; //remove MIC
								}
							}
						}	
					}		
#if ZDCONF_DRV_FORWARD == 1
					if (mCurrConnUser > 1)
                    {
						mode = BC_FORWARD;
						bcBuf = pdot11Obj->AllocBuffer(bodyLen, &pBcData);
						if (bcBuf)
                        {
                        
							memcpy(pBcData, pBody, bodyLen);
							zd_WirelessForward(pHdr, pBcData, bodyLen, bcBuf, mode, NULL, pEthHdr);
						}
					}
#endif
					goto rx_ind;
				}
				else 
                {
#if ZDCONF_DRV_FORWARD == 1
					void *pTxHash = NULL;
		
					if (mBlockBSS)
                    { //discard IntraBSS packet
						goto rx_release;
					}

					zd_QueryStaTable((U8 *)pDa, &pTxHash); //Automatic wireless forwarding		

					if (pTxHash)
                    {	
						if (bSwCheckMIC)
						{
							if ((pHash->keyLength==32) && (pHdr[1] & WEP_BIT) && (hdrLen == 32))
                            {
#if ZDCONF_MIC_CHECK == 1
								if (!zd_CheckMic(pHdr, pBody, bodyLen, pHash, pEthHdr))
                                {
									goto rx_release;
								}
#endif
								else
									bodyLen -= MIC_LNG; //remove MIC
							}		
						}		
				
						mode = BSS_FORWARD;
						zd_WirelessForward(pHdr, pBody, bodyLen, buf, mode, pTxHash, pEthHdr);
						return; 
					}	
#endif
				}	
			}
#endif //ZDCONF_AP_SUPPORT == 1
			// mic check
			if (bSwCheckMIC) // For TKIP, always use sw-mic check.
			{
				//if ((pHash->keyLength==32) && (pHdr[1] & WEP_BIT) && (hdrLen == 32))
				{
#if ZDCONF_MIC_CHECK == 1
					if (!zd_CheckMic(pHdr, pBody, bodyLen, pHash, pEthHdr))
					{// sw-mic check failed, discard this packet.
						goto rx_release;
					}
					else
#endif
                    {// sw-mic check ok, remove MIC
						bodyLen -= MIC_LNG;
					}	
				}		
			}		
rx_ind:
            if(LP)
            {
#if ZDCONF_LP_SUPPORT == 1
                pData = pBody;
                dataLen = bodyLen;
#endif
            }
			//If Typelen field is not used for len
			else if(memcmp(pBody,zd_Snap_Apple_AARP,8)==0 || memcmp(pBody,zd_Snap_Apple_Type,8)==0) {
		
                pData = pBody - 14;
                dataLen = bodyLen + 14;     /* Plus DA, SA and TypeLen */
                pData[12] = (bodyLen>>8) & 0xFF;
                pData[13] = bodyLen & 0xFF;

			}

			else if ((bodyLen > 5 ) && (memcmp(pBody, zd_Snap_Header, 6) == 0 
				|| memcmp(pBody, zd_SnapBridgeTunnel, 6) == 0)){
				
				pData = pBody - 6;	
				dataLen = bodyLen + 6;		/* Plus DA, SA*/
			}
/*    24            6      2     n
*---------------*------*-----*------------------------*
|    WLAN HDR   | SNAP | LEN | DATA                   |
*---------------*------*-----*------------------------*
*/

			else{
				pData = pBody - 14;	
				dataLen = bodyLen + 14;		/* Plus DA, SA and TypeLen */
				pData[12] = (bodyLen>>8) & 0xFF;
				pData[13] = bodyLen & 0xFF;
			}
/*    24            6      
*---------------*------------------------*
|    WLAN HDR   | DATA                   |
*---------------*------------------------*
*/

            if(!LP)
            {
                memcpy(pData, pEthHdr, 6);	/* Set DA */
                memcpy(pData+6, pEthHdr+6, 6);	/* Set SA */
            }
			//if (Type == 0x888e)
				//zd1205_dump_data("pData = ", pData, dataLen);
            if(pHash)
                pHash->RxBytes += dataLen;
			pdot11Obj->RxInd(pData, dataLen, buf, LP_MAP);
			return;
		}	
	}
	else {	//Mgt Frame
		pRxSignal = allocSignal();
		if (!pRxSignal){
	   		FPRINT("zd_ReceivePkt out of signal");
	   		FPRINT_V("freeSignalCount", freeSignalCount);
			goto rx_release;
		}	
		
		pRxFdesc = allocFdesc();
		if (!pRxFdesc){
    		FPRINT("zd_ReceivePkt out of description");
    		FPRINT_V("freeFdescCount", freeFdescCount);
    		freeSignal(pRxSignal);
			goto rx_release;
		}	
		else{
			//pRxFdesc->bDataFrm = bDataFrm;
#if ZDCONF_SIGNAL_INFO == 1
			pRxFdesc->signalStrength = signalStrength;
			pRxFdesc->signalQuality = signalQuality;
#endif
			pRxFrame = pRxFdesc->mpdu;
			pRxFrame->HdrLen = hdrLen;
			pRxFrame->bodyLen = bodyLen;
			memcpy(pRxFrame->header, pHdr, hdrLen);
			pRxFrame->body = pBody;
			pRxSignal->buf = buf;
			pRxSignal->vapId = vapId;
			pRxSignal->frmInfo.frmDesc = pRxFdesc;
			if (!RxMgtMpdu(pRxSignal)){
				freeSignal(pRxSignal);
				freeFdesc(pRxFdesc);
				pdot11Obj->ReleaseBuffer(buf);
			}	
			return;	
		}
	}
	
rx_release:	
	pdot11Obj->ReleaseBuffer(buf);
	return;
}	
#endif
void zd_InitWepData(void)
{
    memset(mWepIv,0, sizeof(mWepIv));
    memset(mBcIv,0,sizeof(mBcIv));
#if 0
	mWepIv[0] = 0;
	mWepIv[1] = 0;
	mWepIv[2] = 0;
	mWepIv[3] = 0;
	mBcIv[0] = 0;
	mBcIv[1] = 0;
	mBcIv[2] = 0;
	mBcIv[3] = 0;
#endif
}	

#ifdef LINUX
void zd_Release_80211_Buffer(void)
{
	releaseSignalBuf();
	releaseFdescBuf();
}
#endif

//Cmd Functions
BOOLEAN zd_Reset80211(zd_80211Obj_t * pObj)

{
	pdot11Obj = pObj;
	
//	initSignalBuf();
//	initFdescBuf();
	ResetPSMonitor();
//	ResetPMFilter();
	zd_InitWepData();
        mBssCnt=0;
	return TRUE;
}	

#if ZDCONF_AP_SUPPORT == 1
BOOLEAN zd_HandlePsPoll(U8 *pHdr)
{
	Frame_t psPollFrame;
	
	//PSDEBUG("zd_HandlePsPoll");
	psPollFrame.HdrLen = 16;
	psPollFrame.bodyLen = 0;
	memcpy(&psPollFrame.header[0], pHdr, 16);
	RxPsPoll(&psPollFrame);
	return TRUE;
}	
#endif

#if ZDCONF_AP_SUPPORT == 1
BOOLEAN zd_StartAP(void)
{
	void *reg = pdot11Obj->reg;
    
	HW_SetRfChannel(pdot11Obj, mRfChannel, 0, mMacMode);
	HW_SetRfChannel(pdot11Obj, mRfChannel, 1, mMacMode);
 
#if defined(AMAC)		
	pdot11Obj->SetReg(reg, ZD_BasicRateTbl, 0);
#endif
	HW_SetSupportedRate(pdot11Obj, (U8 *)&mBrates);
    
#if defined(OFDM)	
	if(PURE_A_MODE != mMacMode)
		HW_SetSupportedRate(pdot11Obj, (U8 *)&mExtRates);

#endif

        /* Set CAM_MODE to AP Mode */
        pdot11Obj->SetReg(reg, ZD_CAM_MODE, CAM_AP);                                                         

	ConfigBcnFIFO();
	HW_EnableBeacon(pdot11Obj, mBeaconPeriod, mDtimPeriod, AP_BSS);
	HW_RadioOnOff(pdot11Obj, mRadioOn);
	return TRUE;
}
#endif

#ifdef LINUX
BOOLEAN zd_ProbeReq(void)
{
	Signal_t *signal;
	//FPRINT("zd_ProbeReq");
	
	if ((signal = allocSignal()) == NULL){
		return FALSE;
	}	
    signal->vapId=0;	
	signal->id = SIG_PROBE_REQ;
	signal->block = BLOCK_SYNCH;
	sigEnque(pMgtQ, (signal));
	zd_SigProcess();
	
	return TRUE;
}	
BOOLEAN zd_ScanReq(void)
{
    Signal_t *signal;
	if ((signal = allocSignal()) == NULL){
		return FALSE;
	}	
	signal->vapId=0x12; //This member is not used in zd1211, we can use it to carry additional information, 0x1234 indicates we don't want to start a scantimer after sending a Probe Request frame.

	signal->id = SIG_PROBE_REQ;
	signal->block = BLOCK_SYNCH;
	sigEnque(pMgtQ, (signal));
	zd_SigProcess();
        return TRUE;
}
#endif
#if 0	
void zd_ScanEnd()
{
    void *reg=pdot11Obj->reg;
    if (mBssType == AP_BSS)
	pdot11Obj->SetReg(reg, ZD_Rx_Filter, AP_RX_FILTER);
    else 
	pdot11Obj->SetReg(reg, ZD_Rx_Filter, STA_RX_FILTER);
    if (mAssoc) {
//		printk("HW_SetRfChannel:%s(%d)\n",__FILE__,__LINE__);
        HW_SetRfChannel(pdot11Obj, mRfChannel, 1,mMacMode);
    }

    mBssCnt=mBssNum;
    pdot11Obj->ConfigFlag &= ((~ACTIVE_CHANNEL_SCAN_SET) & (~JUST_CHANNEL_SCAN));
}   
void zd_ScanBegin()
{
    void *reg=pdot11Obj->reg;
    mBssNum=0;
    pdot11Obj->ConfigFlag |= (ACTIVE_CHANNEL_SCAN_SET | JUST_CHANNEL_SCAN);
    pdot11Obj->SetReg(reg, ZD_Rx_Filter, (BIT_5|BIT_8));
} 
void zd_CmdScanReq(u16 channel)
{
	if(mMacMode != PURE_A_MODE) {
//		printk("HW_SetRfChannel:%s(%d)\n",__FILE__,__LINE__);
	    HW_SetRfChannel(pdot11Obj, channel, 1,mMacMode);
	}
	else {
//		printk("HW_SetRfChannel:%s(%d)\n",__FILE__,__LINE__);
		HW_SetRfChannel(pdot11Obj,dot11A_Channel[channel-1],1 ,mMacMode); 
	}


    zd_ScanReq();
    return;

}
#endif
#ifdef LINUX
BOOLEAN zd_CmdProbeReq(U8 bWithSSID)
{
	void *reg = pdot11Obj->reg;
    U32 tempValue;
    int ret = TRUE;
    struct zd1205_private *macp = (struct zd1205_private *)g_dev->priv;
    int cnt = 0;
    extern FrmDesc_t *FreeFdescList;
    
	//FPRINT("zd_CmdProbeReq");
	if (pdot11Obj->bChScanning){
		FPRINT("Channel is under scanning....");

		if (mRequestFlag & CHANNEL_SCAN_SET)
			mRequestFlag &= ~CHANNEL_SCAN_SET;
		return FALSE;
	}
    if(pdot11Obj->bDeviceInSleep)
    {
        printk("In SLEEP @ %s\n", __FUNCTION__);
        return FALSE;
    }
    // To ensure all frames is flushed before scanning
    // This is used to prevent out of frmdesc and cause serious problem.
    pdot11Obj->bChScanning=1;
    while (((macp->activeTxQ->count > 0) || (!FreeFdescList)) && (cnt++ < 1000))
        mdelay(10);
    if(cnt>=1000) 
        printk("Wait too long to start Scanning\n");

    tempValue = pdot11Obj->GetReg(reg, ZD_BCNInterval);
    tempValue &= ~BIT_25; // disable IBSS Beacon transmission temporarily.
    pdot11Obj->SetReg(reg, ZD_BCNInterval, tempValue);


	pdot11Obj->SetReg(reg, ZD_Rx_Filter, (BIT_5|BIT_8)); //only accept beacon and ProbeRsp frame 	
	pdot11Obj->ConfigFlag |= ACTIVE_CHANNEL_SCAN_SET;
	
	mBssNum = 0;
	if(mMacMode != PURE_A_MODE) {
//		printk("HW_SetRfChannel:%s(%d)\n",__FILE__,__LINE__);
		HW_SetRfChannel(pdot11Obj, CurrScanCH, 1,mMacMode);
	}
#if ZDCONF_80211A_SUPPORT == 1
	else  {
//		printk("HW_SetRfChannel:%s(%d)\n",__FILE__,__LINE__);
		HW_SetRfChannel(pdot11Obj, dot11A_Channel[CurrScanCH-1], 1,mMacMode);
	}
#endif

	ret = zd_ProbeReq();
    
	if (mRequestFlag & CHANNEL_SCAN_SET)
		mRequestFlag &= ~CHANNEL_SCAN_SET;

	//pdot11Obj->bChScanning = 1;
	return ret;
}	
#endif

BOOLEAN zd_StartSTA(BOOLEAN bEnable)
{
	void *reg = pdot11Obj->reg;
	MP_ASSERT(0);
	//FPRINT("zd_StartSTA");
	HW_SetSupportedRate(pdot11Obj, (U8 *)&mBrates);
	HW_RadioOnOff(pdot11Obj, mRadioOn);
       

	if (mBssType == INFRASTRUCTURE_BSS)
        	pdot11Obj->SetReg(reg, ZD_CAM_MODE, CAM_STA); 
	else                                                        
        	pdot11Obj->SetReg(reg, ZD_CAM_MODE, CAM_IBSS);                                                         
#if ZDCONF_PSEUDO_SUPPORT == 1
	if (mBssType == PSEUDO_IBSS){
		zd_PseudoIbssConnect();
	}
	else 
#endif
    {
		//if (!bEnable)
		{
			pdot11Obj->ConfigFlag |= SCAN_AND_CONNECT_SET;
			mRequestFlag |= CHANNEL_SCAN_SET;
		}
	}

	return TRUE;
}

BOOLEAN zd_CmdDisasoc(MacAddr_t *sta, U8 rCode)
{
	Signal_t *signal;
	Hash_t *pHash = NULL;
	//FPRINT("zd_CmdDisasoc");
#ifdef LINUX
	
	//if (isGroup(sta))
	//	return FALSE;

	if ((signal = allocSignal()) == NULL)
		return FALSE;
	
	signal->id = SIG_DIASSOC_REQ;
	signal->block = BLOCK_ASOC;
	if (mBssType == INFRASTRUCTURE_BSS){
		memcpy(&signal->frmInfo.Sta, (U8 *)&mBssId, 6);
	}
	else {	
		memcpy(&signal->frmInfo.Sta, sta, 6);
	}	
	signal->frmInfo.rCode = (ReasonCode)rCode;
	sigEnque(pMgtQ, (signal));
#endif
	pHash = HashSearch((MacAddr_t *)sta->mac); 
    if(pHash==NULL)
        return TRUE;
    printk("In %s Disasoc a Connected STA\n",__func__);
    pHash->asoc = STATION_STATE_NOT_AUTH; 
	
	return TRUE;
}	

BOOLEAN zd_CmdDeauth(MacAddr_t *sta, U8 rCode)
{
	Signal_t *signal;
	Hash_t *pHash = NULL;
#ifdef LINUX
	
	if ((signal = allocSignal()) == NULL)  
		return FALSE;
		
	signal->id = SIG_DEAUTH_REQ;
	signal->block = BLOCK_AUTH_REQ;
	memcpy(&signal->frmInfo.Sta, sta, 6);
	signal->frmInfo.rCode = (ReasonCode)rCode;
	sigEnque(pMgtQ, (signal));
	
#endif
	pHash = HashSearch((MacAddr_t *)sta->mac); 
    if(pHash==NULL)
        return TRUE;
    printk("In %s Deauth a Connected STA\n",__func__);
    pHash->asoc = STATION_STATE_NOT_AUTH; 

	return TRUE;
}	
#ifdef LINUX

BOOLEAN zd_PassiveScan(void)
{
	void *reg = pdot11Obj->reg;
	
	//FPRINT("zd_PassiveScan");
	
	if (pdot11Obj->ConfigFlag & PASSIVE_CHANNEL_SCAN_SET)
		return FALSE;
		
	pdot11Obj->ConfigFlag |= PASSIVE_CHANNEL_SCAN_SET;
	pdot11Obj->SetReg(reg, ZD_Rx_Filter, 0x100); //only accept beacon frame 
	if(mMacMode != PURE_A_MODE) {
//		printk("HW_SetRfChannel:%s(%d)\n",__FILE__,__LINE__);
		HW_SetRfChannel(pdot11Obj, CurrScanCH, 1,mMacMode);
	}
#if ZDCONF_80211A_SUPPORT == 1
	else {
//		printk("HW_SetRfChannel:%s(%d)\n",__FILE__,__LINE__);
		HW_SetRfChannel(pdot11Obj,dot11A_Channel[CurrScanCH-1],1 ,mMacMode); 
	}
#endif

	pdot11Obj->StartTimer(SCAN_TIMEOUT, DO_SCAN);
	
	return TRUE;
}	

BOOLEAN zd_DisasocAll(U8 rCode)
{
	int i;
	MacAddr_t *sta;
#if ZDCONF_AP_SUPPORT == 1
	if (mBssType == AP_BSS){
		for (i=1; i<(MAX_AID+1); i++){
			if (sstByAid[i]->asoc == STATION_STATE_ASOC){
				sta = (MacAddr_t *)&sstByAid[i]->mac[0];
				zd_CmdDisasoc(sta, rCode);
			}
		}
		
		FlushQ(pTxQ);
	}
#endif
	if (mBssType == INFRASTRUCTURE_BSS){
		if (mAssoc)
			zd_CmdDisasoc(&mBssId, rCode);
	}
	return TRUE;

}

BOOLEAN zd_ChooseAP(BOOLEAN bUseBssid)
{
    U8 i;
    U16 cap;
    U16 strength= 0;
	BOOLEAN found = FALSE;

    mBssIndex = 0xff;
    for (i=0; i<mBssCnt; i++)
    {
		if (bUseBssid)
		{
			if (memcmp(&dot11DesiredBssid, &mBssInfo[i].bssid.mac[0], ETH_ALEN) == 0 && memcmp(&dot11DesiredBssid, zeroMacAddress, ETH_ALEN)!=0) 
			{
				//ZD1211DEBUG(0, "zd_ChooseAP: Bssid" MACSTR "matched to index:%d\n",MAC2STR(dot11DesiredBssid), i);
				mBssIndex = i;
				break;
			}
		}		

    	cap = mBssInfo[i].cap;
        if ((memcmp(&dot11DesiredSsid, &mBssInfo[i].ssid, dot11DesiredSsid.buf[1]+2) == 0) 
        	|| (!mProbeWithSsid) ) {
                            
			if(1) 
				;
			else if ((mMacMode == PURE_B_MODE) && (mBssInfo[i].apMode == PURE_G_AP))
				continue;
            else if ((mMacMode == PURE_B_MODE) && (mBssInfo[i].apMode == PURE_A_AP))
				continue; 
			else if ((mMacMode == PURE_G_MODE) && (mBssInfo[i].apMode == PURE_B_AP))
				continue;
			else if ((mMacMode == PURE_G_MODE) && (mBssInfo[i].apMode == PURE_A_AP))
				continue;
			
			else if ((mMacMode == PURE_A_MODE) && (mBssInfo[i].apMode != PURE_A_AP))
				continue;

			else if ((mMacMode == MIXED_MODE) && (mBssInfo[i].apMode == PURE_A_AP))
				continue;

            //check capability ...
            if (cap & CAP_PRIVACY){
			    if (!mPrivacyInvoked)
				    continue;
    		}
		    else {
			    if (mPrivacyInvoked)
				    continue;
			}
            if (!pdot11Obj->IsUSB2_0){ //host is usb 1.1
                if (mBssInfo[i].apMode == PURE_G_AP)
                    continue;
            }    
			
			if (cap & CAP_ESS){
				if (mBssInfo[i].signalStrength >= strength){
                    strength =  mBssInfo[i].signalStrength;
					mBssIndex = i;
                    //FPRINT_V("cap", cap);
				}
			}
				
            //break; 
        }
    }
    if (mBssIndex< mBssCnt){
        found = TRUE;
        //FPRINT_V("Desired AP Found, Bss Index", mBssIndex);
        if (pdot11Obj->ConfigFlag & SCAN_AND_CONNECT_SET){
	    //printk("zd_ChooseAP: set mRequestFlag.BSS_CONNECT_SET\n");
            mRequestFlag |= BSS_CONNECT_SET;
            pdot11Obj->ConfigFlag &= ~SCAN_AND_CONNECT_SET;
        }    
    }
    else{
        //printk(" \n");
        //printk(KERN_ERR "****** Can't find desiredSSID:");
        //for (i=0; i<dot11DesiredSsid.buf[1]; i++) {
		//	printk("%c", dot11DesiredSsid.buf[2+i]);
		//}
        //printk(" \n");
    }     
//	int j;
//	if(0xff != mBssIndex) {
//		for(j=0;j<mBssInfo[mBssIndex].ssid.buf[1];j++)
//			printk("%c",mBssInfo[mBssIndex].ssid.buf[2+j]);
//		printk("  ChooseAP(Mac=%d,Ch:%d)\n",mBssInfo[mBssIndex].apMode,mBssInfo[mBssIndex].Phpm.buf[2]);
//	}

    return found;              
}
BOOLEAN zd_InfraConnect(U8 index)
{
	Signal_t *signal;
	struct zd1205_private *macp = (struct zd1205_private *)g_dev->priv;

	MacAddr_t *pBssid;
	Element *pSsid = NULL;
	U32 tmpvalue;
	BOOLEAN	bChooseAPResult;
	void *reg = pdot11Obj->reg;
	//FPRINT("zd_InfraConnect");
	if (mBssNum == 0)
		return FALSE;
		
	if ((signal = allocSignal()) == NULL)
		return FALSE;
	if(mCounterMeasureState) {
		mRequestFlag &= ~BSS_CONNECT_SET;
        freeSignal(signal);
		return FALSE;
	}
	// look up global scan result table according to desired ssid,
	// because mBssInfo order may be different from macp->BSSInfo[].
	/*if (mBssCnt)
	{
	    U8	ssidLength;
	    for(i=0; i<mBssCnt; i++)
	    {
	        ssidLength=dot11DesiredSsid.buf[1]+2;
		if (mBssInfo[i].cap & CAP_ESS)
		{
		    if (memcmp((U8*)&dot11DesiredSsid,(U8*)&mBssInfo[i].ssid,ssidLength)==0)
		    {
			break;
		    }
		}
	    }		
	} */
	// Use zd_ChooseAP instead of above code
	bChooseAPResult=zd_ChooseAP(0);
	if (!bChooseAPResult || 1)
	{// dot11DesiredSsid can't be found in table mBssInfo[]
	    if ((index+1) <= mBssCnt)
	    {
		//printk(KERN_ERR "Desired SSID can't be found in current table\n");
	    	mBssIndex=index; // Can't found in the latest scan result table, use the old index.
	    }
	    else
	    {
            freeSignal(signal);
	    	return FALSE; // The index exceed mBssInfo[].
	    }
	}
	else
	{
	    //printk(KERN_ERR "Desired SSID found in location %d\n",mBssIndex);
	}
	//Disable IBSS mode
    if(pdot11Obj->bDeviceInSleep)
    {
        printk("In SLEEP @ %s\n", __FUNCTION__);
        return FALSE;
    }
	tmpvalue = pdot11Obj->GetReg(reg, ZD_BCNInterval);
	tmpvalue &= ~BIT_25;
	pdot11Obj->SetReg(reg, ZD_BCNInterval, tmpvalue);
	mCap |= BIT_0;
	mCap &= ~BIT_1;	
	
	// For IBSS Connect
	if (mBssInfo[index].cap & CAP_IBSS)
	{
		
	}
	//mBssIndex = index;
	//update beacon interval
	HW_UpdateBcnInterval(pdot11Obj, mBssInfo[mBssIndex].bcnInterval);
	
	pSsid = &mBssInfo[mBssIndex].ssid;
	memcpy((U8 *)&mSsid, (U8 *)pSsid, pSsid->buf[1]+2); 	
	pBssid = &mBssInfo[mBssIndex].bssid;	
	memcpy((U8 *)&mBssId, (U8 *)pBssid, 6);
    
	//update Bssid
	pdot11Obj->SetReg(reg, ZD_BSSID_P1, zd_cpu_to_le32(*(U32 *)&mBssId.mac[0]));
	pdot11Obj->SetReg(reg, ZD_BSSID_P2, zd_cpu_to_le32(*(U32 *)&mBssId.mac[4]));
	// Update channel number contained in the DS Parameter Set element of the received probe response or beacon frame. 
	mRfChannel = mBssInfo[mBssIndex].Phpm.buf[2];

	if(PURE_A_AP == mBssInfo[mBssIndex].apMode) {
		ChangeMacMode(PURE_A_MODE,mRfChannel);
		zd_UpdateCardSetting(&(macp->cardSetting));
		pdot11Obj->DelayUs(1000);
		HW_SetRfChannel(pdot11Obj, mRfChannel, 1, PURE_A_MODE);
		
	}
	else {
		if(PURE_A_MODE == mMacMode)
			mMacMode = MIXED_MODE;
		ChangeMacMode(mMacMode,mRfChannel);
		zd_UpdateCardSetting(&(macp->cardSetting));
		pdot11Obj->DelayUs(1000);
		HW_SetRfChannel(pdot11Obj, mRfChannel, 1, MIXED_MODE);
	}

	signal->id = SIG_AUTH_REQ;
	signal->block = BLOCK_AUTH_REQ;
	memcpy(&signal->frmInfo.Sta, (U8 *)&mBssId, 6);
	sigEnque(pMgtQ, (signal));
	
	if (mRequestFlag & CONNECT_TOUT_SET)
		mRequestFlag &= ~CONNECT_TOUT_SET;
	if (mRequestFlag & BSS_CONNECT_SET)
		mRequestFlag &= ~BSS_CONNECT_SET;    
		
	return TRUE;
}
#endif

#if ZDCONF_ADHOC_SUPPORT == 1
#define NIC_INDEX_WIFI          2
char *Adhoc_Ssid(void);
extern int adhoc_create;
BOOLEAN zd_IbssConnect(void)
{
    extern void zd1205_SetRatesInfo(struct zd1205_private *macp);
    struct zd1205_private *macp=g_dev->priv;
	int i,j;
	U8 Length;
	BOOLEAN bBSSFound = FALSE;	
	MacAddr_t *pBssid;
	BssInfo_t *pBssInfo;
	void *reg = pdot11Obj->reg;
	U32 tmpvalue;
    U32 seed;
	int ret;
	struct iwreq iwr;

	//FPRINT("zd_IbssConnect");
#if defined(OFDM) 
	pdot11Obj->SetReg(reg, ZD_CWmin_CWmax, CW_NORMAL_SLOT);
	// not to use short slot time
	mCap &= ~BIT_10;
	pdot11Obj->ConfigFlag &= ~SHORT_SLOT_TIME_SET;

	if (mMacMode == PURE_G_MODE){
		// not use protection mechanism
		pdot11Obj->ConfigFlag &= ~ENABLE_PROTECTION_SET;
	}
	else if(mMacMode != PURE_A_MODE) {
		// force use protection mechanism
		pdot11Obj->ConfigFlag |= ENABLE_PROTECTION_SET;
	}

#endif

	
	// Recover the EIFS to 0x200.
	// We need not use SleepResetDevice. In IBSS mode, we can easily
	// get others traffic. (not less of Rx-Frame)
 	pdot11Obj->SetReg(reg, ZD_IFS_Value, 0xa200032);
	if (mATIMWindow != 0){
		// Do not set macp->PwrState = PSM, otherwise
		// throughput with Cirrus card (in PS mode) will down.
		//macp->PwrState = PSM;
		// PwrMgt = 1
        if(pdot11Obj->bDeviceInSleep)
        {
            printk("In SLLLLLLLLLLLLLLLLLLP @ %s\n", __FUNCTION__);
            return FALSE;
        }

		tmpvalue = pdot11Obj->GetReg(reg, ZD_BCNInterval);
		tmpvalue |= BIT_26;
		pdot11Obj->SetReg(reg, ZD_BCNInterval, tmpvalue);
	}
	else{
		//macp->PwrState = CAM;
		// PwrMgt = 0;
        if(pdot11Obj->bDeviceInSleep)
        {
            printk("In SLLLLLLLLLLLLLLLLLLP @ %s\n", __FUNCTION__);
            return FALSE;
        }

		tmpvalue = pdot11Obj->GetReg(reg, ZD_BCNInterval);
		tmpvalue &= ~BIT_26;
		pdot11Obj->SetReg(reg, ZD_BCNInterval, tmpvalue);
	}
						
						
	mCap &= ~BIT_0;
	mCap |= BIT_1;	

    mTimeBeforeAdhocRoaming = (jiffies + dot11MacAddress.mac[0] + 3) & 0x1F;
	if (mBssCnt ){	// IBSS found
        mpDebugPrint("!!!!IBSS FOUND!!!!");
		for (i=0; i<mBssCnt; i++){
			Length = dot11DesiredSsid.buf[1]+2;
			if (mBssInfo[i].cap & CAP_IBSS){
				if (memcmp((U8 *)&dot11DesiredSsid, (U8 *)&mBssInfo[i].ssid, Length) == 0){
					break;
				}	
			}	
		}	
		if (i < mBssCnt){
			bBSSFound = TRUE;
			mBssIndex = i;

		pBssInfo = &mBssInfo[mBssIndex];
		//init SSID
		mSsid.buf[0] = 0;//element id
		mSsid.buf[1] = pBssInfo->ssid.buf[1];//length
		for(i = 0;i<mSsid.buf[1];i++)
		{
			mSsid.buf[i+2] = pBssInfo->ssid.buf[i+2];
		}

		mBssId.mac[0] = pBssInfo->bssid.mac[0];
		mBssId.mac[1] = pBssInfo->bssid.mac[1];
		mBssId.mac[2] = pBssInfo->bssid.mac[2];
		mBssId.mac[3] = pBssInfo->bssid.mac[3];
		mBssId.mac[4] = pBssInfo->bssid.mac[4];
		mBssId.mac[5] = pBssInfo->bssid.mac[5];
		
		memcpy(dot11DesiredBssid,mBssId.mac,6);
		memset(&iwr, 0, sizeof(iwr));
		strncpy(iwr.ifr_name, "wlan0", IFNAMSIZ);
		
		memcpy(iwr.u.ap_addr.sa_data,dot11DesiredBssid,6);
			
		if (ioctl(0, SIOCSIWAP, &iwr) < 0) {
			perror("ioctl[SIOCSIWAP]");
			ret = -1;
		}
		zd1205_dump_data("mBssId = ", (U8 *)&mBssId, 6);
		//update Bssid	
		pdot11Obj->SetReg(reg, ZD_BSSID_P1, zd_cpu_to_le32(*(U32 *)&mBssId.mac[0]));
		pdot11Obj->SetReg(reg, ZD_BSSID_P2, zd_cpu_to_le32(*(U32 *)&mBssId.mac[4]));

#if LINUX		
        if(pBssInfo->apMode == PURE_A_AP)
            ChangeMacMode(PURE_A_MODE, pBssInfo->Phpm.buf[2]);
        else 
            ChangeMacMode(MIXED_MODE, pBssInfo->Phpm.buf[2]);
#endif
        zd1205_SetRatesInfo(macp);
        mMacMode = macp->cardSetting.MacMode;
        //mRfChannel = macp->cardSetting.Channel;
        mRfChannel = pBssInfo->Phpm.buf[2];
        mPhpm.buf[0] = EID_DSPARMS;
        mPhpm.buf[1] = 1;
        mPhpm.buf[2] = mRfChannel;

        printk("Ibss Join, MAC_MODE:%d, CHANNEL:%d\n", mMacMode,mRfChannel);
		HW_SetRfChannel(pdot11Obj, mRfChannel, 1, mMacMode);
		//FPRINT_V("mRfChannel", mRfChannel);
		
		//update beacon interval
        //mBeaconPeriod = pBssInfo->bcnInterval; //MZCai
		HW_UpdateBcnInterval(pdot11Obj, mBeaconPeriod);
		//FPRINT_V("mBeaconPeriod", mBeaconPeriod);
		
		//update supported rated
        //memcpy((U8 *)&mBrates, (U8 *)&pBssInfo->supRates, pBssInfo->supRates.buf[1]+2); //MZCAI
        //init support rate
		mBrates.buf[0] = 1;//element id ;
		mBrates.buf[1] = 4;//length
		mBrates.buf[2] = 0x82;
		mBrates.buf[3] = 0x84;
		mBrates.buf[4] = 0x8b;
		mBrates.buf[5] = 0x96;

        HW_SetSupportedRate(pdot11Obj, (U8 *)&mBrates);
		//zd1205_dump_data("mBrates = ", (U8 *)&mBrates, mBrates.buf[1]+2);
#if defined(OFDM)		
        //init support extrate
        //Extended supported rates
        mExtRates.buf[0] = 0x32;
        mExtRates.buf[1] = 0x08;
        mExtRates.buf[2] = 0x0c;
        mExtRates.buf[3] = 0x12;
        mExtRates.buf[4] = 0x18;
        mExtRates.buf[5] = 0x60;
        mExtRates.buf[6] = 0x24;
        mExtRates.buf[7] = 0x30;
        mExtRates.buf[8] = 0x48;
        mExtRates.buf[9] = 0x6c;

		if(mMacMode != PURE_A_MODE && mMacMode != PURE_B_MODE) //MZCai
        {
            if(pBssInfo->extRates.buf[0] == EID_EXT_RATES)
            {
                //if(pdot11Obj->IsUSB2_0) //MZCAI
                memcpy((U8 *)&mExtRates, (U8 *)&pBssInfo->extRates, pBssInfo->extRates.buf[1]+2); //MZCai
                HW_SetSupportedRate(pdot11Obj, (U8 *)&mExtRates);
            }
        }

			zd1205_dump_data("mExtRates = ", (U8 *)&mExtRates, mExtRates.buf[1]+2);
#endif
		//update ATIM Window
        mATIMWindow = pBssInfo->IbssParms.buf[2]+(((U16)pBssInfo->IbssParms.buf[3]) << 8); //MZCai
        memcpy((U8 *)&mIbssParms, (U8 *)&pBssInfo->IbssParms, pBssInfo->IbssParms.buf[1]+2); //MZCai
		HW_UpdateATIMWindow(pdot11Obj, mATIMWindow);
		//FPRINT_V("mATIMWindow", mATIMWindow);
		
		ConfigBcnFIFO();
		HW_EnableBeacon(pdot11Obj, mBeaconPeriod, 0, INDEPENDENT_BSS);
		HW_RadioOnOff(pdot11Obj, mRadioOn);
		mRequestFlag &= ~IBSS_CONNECT_SET;
		mAssoc = TRUE;
		memcpy(&pdot11Obj->CurrSsid[0], (U8 *)&mSsid, mSsid.buf[1]+2); 
		//pdot11Obj->StatusNotify(STA_ASSOCIATED, (U8 *)&mBssId);
		macp->device->ifindex = NIC_INDEX_WIFI;
		netif_carrier_on(macp->device);
		NetInterfaceEventSet(); /* notify network subsystem */
        //UpdateStaStatus(mBssId.mac, STATION_STATE_ASOC, 0);
        xpgGotoNetFunction();
        return TRUE;
        }
	}
	else {
		//if (!(pdot11Obj->ConfigFlag & IBSS_CHANNEL_SCAN_SET)){ 
		//	pdot11Obj->ConfigFlag |= IBSS_CHANNEL_SCAN_SET;
		//	zd_CmdProbeReq(1);
		//}	
		//return FALSE;
        printk("We found no IBSS, and we ignore the above code");
        printk("without confirm\n");
	}	
	
	if (!bBSSFound){
		char *ssid = Adhoc_Ssid();
		mpDebugPrint("IBSS not found, create it !!!");
		adhoc_create = TRUE;
		//init SSID
		mSsid.buf[0] = 0;//element id
		mSsid.buf[1] = strlen(ssid);//length
		for(i = 0;i<strlen(ssid);i++)
		{
			mSsid.buf[i+2] = ssid[i];
		}
		/****************************************************/
		/* We generate an IBSS								*/
		/****************************************************/
        if(pdot11Obj->bDeviceInSleep)
        {
            printk("In SLLLLLLLLLLLLLLLLLLP @ %s\n", __FUNCTION__);
            return FALSE;
        }
		seed = pdot11Obj->GetReg(reg, ZD_TSF_LowPart);
		mBssIndex = 0xff;
		//generate random BSSID
	#ifdef	LINUX
		mBssId.mac[0] = (U8)((pdot11Obj->Rand(seed) & ~0x3) | 0x2); // I/G = 0, U/L = 1
		mBssId.mac[1] = (U8)pdot11Obj->Rand(seed);
		mBssId.mac[2] = (U8)pdot11Obj->Rand(seed);
		mBssId.mac[3] = (U8)pdot11Obj->Rand(seed);
		mBssId.mac[4] = (U8)pdot11Obj->Rand(seed);
		mBssId.mac[5] = (U8)pdot11Obj->Rand(seed);
   #else  
		mBssId.mac[0] = (U8)((zdcb_Rand(seed)& ~0x3)| 0x2);
		mBssId.mac[1] = (U8)zdcb_Rand(seed);
		mBssId.mac[2] = (U8)zdcb_Rand(seed);
		mBssId.mac[3] = (U8)zdcb_Rand(seed);
		mBssId.mac[4] = (U8)zdcb_Rand(seed);
		mBssId.mac[5] = (U8)zdcb_Rand(seed);
	#endif	
		memcpy(dot11DesiredBssid,mBssId.mac,6);
		memset(&iwr, 0, sizeof(iwr));
		strncpy(iwr.ifr_name, "wlan0", IFNAMSIZ);
		memcpy(iwr.u.ap_addr.sa_data,dot11DesiredBssid,6);
			
		if (ioctl(0, SIOCSIWAP, &iwr) < 0) {
			perror("ioctl[SIOCSIWAP]");
			ret = -1;
		}
	
		zd1205_dump_data("mBssId = ", (U8 *)&mBssId, 6);
		//update Bssid	
		pdot11Obj->SetReg(reg, ZD_BSSID_P1, zd_cpu_to_le32(*(U32 *)&mBssId.mac[0]));
		pdot11Obj->SetReg(reg, ZD_BSSID_P2, zd_cpu_to_le32(*(U32 *)&mBssId.mac[4]));
#if LINUX		
        ChangeMacMode(macp->IBSS_DesiredMacMode, macp->IBSS_DesiredChannel);
#endif

        mMacMode = macp->cardSetting.MacMode;
        mRfChannel = macp->cardSetting.Channel;
        mPhpm.buf[0] = EID_DSPARMS;
        mPhpm.buf[1] = 1;
        mPhpm.buf[2] = 1/*mRfChannel*/;

        printk("Ibss Create, MAC_MODE:%d, CHANNEL:%d\n", mMacMode,1/*mRfChannel*/);
		HW_SetRfChannel(pdot11Obj, 1/*mRfChannel*/, 1, mMacMode);
		//FPRINT_V("mRfChannel", mRfChannel);
		
		//update beacon interval
		HW_UpdateBcnInterval(pdot11Obj, mBeaconPeriod);
		//FPRINT_V("mBeaconPeriod", mBeaconPeriod);
		
		//update supported rated
		//init support rate
		mBrates.buf[0] = 1;//element id ;
		mBrates.buf[1] = 4;//length
		mBrates.buf[2] = 0x82;
		mBrates.buf[3] = 0x84;
		mBrates.buf[4] = 0x8b;
		mBrates.buf[5] = 0x96;

		HW_SetSupportedRate(pdot11Obj, (U8 *)&mBrates);
		zd1205_dump_data("mBrates = ", (U8 *)&mBrates, mBrates.buf[1]+2);
#if defined(OFDM)		
        //init support extrate
        //Extended supported rates
        mExtRates.buf[0] = 0x32;
        mExtRates.buf[1] = 0x08;
        mExtRates.buf[2] = 0x0c;
        mExtRates.buf[3] = 0x12;
        mExtRates.buf[4] = 0x18;
        mExtRates.buf[5] = 0x60;
        mExtRates.buf[6] = 0x24;
        mExtRates.buf[7] = 0x30;
        mExtRates.buf[8] = 0x48;
        mExtRates.buf[9] = 0x6c;

		if(mMacMode != PURE_A_MODE && mMacMode != PURE_B_MODE)
			//if (pdot11Obj->IsUSB2_0)
				HW_SetSupportedRate(pdot11Obj, (U8 *)&mExtRates);

			zd1205_dump_data("mExtRates = ", (U8 *)&mExtRates, mExtRates.buf[1]+2);
#endif
		//update ATIM Window
		HW_UpdateATIMWindow(pdot11Obj, mATIMWindow);
		//FPRINT_V("mATIMWindow", mATIMWindow);
		ConfigBcnFIFO();
		HW_EnableBeacon(pdot11Obj, mBeaconPeriod, 0, INDEPENDENT_BSS);
		HW_RadioOnOff(pdot11Obj, mRadioOn);
		mRequestFlag &= ~IBSS_CONNECT_SET;
		mAssoc = TRUE;
		memcpy(&pdot11Obj->CurrSsid[0], (U8 *)&mSsid, mSsid.buf[1]+2); 
		//pdot11Obj->StatusNotify(STA_ASSOCIATED, (U8 *)&mBssId);
		//macp->device->ifindex = NIC_INDEX_WIFI;
		//netif_carrier_on(macp->device);
		//NetInterfaceEventSet(); /* notify network subsystem */
		return TRUE;
	}
	return FALSE;
}
void zd_Cpdot11DesiredSsid(BYTE *desiredssid)
{
  dot11DesiredSsid.buf[0] = 0;
  dot11DesiredSsid.buf[1] = strlen(desiredssid);
  memcpy(&dot11DesiredSsid.buf[2], desiredssid, strlen(desiredssid));
  //mpDebugPrint("zd_Cpdot11DesiredSsid %s",desiredssid);
}
void zd_CpBssInfo(BYTE *ssid,BYTE *bssid,DWORD channel)
{
  int i=0;
  mBssInfo[0].cap|=CAP_IBSS;
  mBssInfo[0].ssid.buf[0]=0;
  mBssInfo[0].ssid.buf[1]=strlen(ssid);
  for(i=0;i<mBssInfo[0].ssid.buf[1];i++)
  {
    mBssInfo[0].ssid.buf[i+2] = ssid[i];
  }
  memcpy(mBssInfo[0].bssid.mac,bssid,6);
  mBssInfo[0].Phpm.buf[2] = channel;
}

#endif

#ifdef LINUX
void zd_ResetDevice(void)
{
	U16	BeaconInterval = 0x2;
	U32	tmpvalue;
	void *reg = pdot11Obj->reg;
	// Device will reset after 1ms
	HW_UpdateBcnInterval(pdot11Obj, BeaconInterval);
	pdot11Obj->SetReg(reg, ZD_Pre_TBTT, 0x1);	
	//++ Ensure the following is an atomic operation.
#ifndef HOST_IF_USB    
	i_state = pdot11Obj->EnterCS();
#endif
    
    if(pdot11Obj->bDeviceInSleep)
    {
        printk("In SLLLLLLLLLLLLLLLLLLP @ %s\n", __FUNCTION__);
        return;
    }

	tmpvalue = pdot11Obj->GetReg(reg, ZD_PS_Ctrl);
	pdot11Obj->SetReg(reg, ZD_PS_Ctrl, (tmpvalue | BIT_0));	
	pdot11Obj->bDeviceInSleep = 1;
    
#ifndef HOST_IF_USB     
	pdot11Obj->ExitCS(i_state);
    // Delay 1ms to ensure device had been reset
	pdot11Obj->DelayUs(1000);
#endif    
	
}

BOOLEAN zd_PseudoIbssConnect(void)
{
	void *reg = pdot11Obj->reg;
	U8 IBSS_BSSID[6];
	memset(IBSS_BSSID, 0, 6);	
	//++
	// Set EIFS=0x32 to prevent chamber low Tx-throughput (sometimes)
	// problem. In chamber environment, almost no Rx-frame, once 
	// we detect a CRC16/CRC32 error frame, we adopt EIFS, because of
	// less of RX-frame, it's less posibility to change EIFS to DIFS
	// by FA (Frame Analyzer), and therefore damage the Tx-Throughput.
	// We must use SleepResetDevice to trigger FA to adpot 0x32.
	pdot11Obj->SetReg(reg, ZD_IFS_Value, 0xa032032);
	zd_ResetDevice();
	
	//update Bssid	
	pdot11Obj->SetReg(reg, ZD_BSSID_P1, zd_cpu_to_le32(*(U32 *)&IBSS_BSSID[0]));
	pdot11Obj->SetReg(reg, ZD_BSSID_P2, zd_cpu_to_le32(*(U32 *)&IBSS_BSSID[4]));
	HW_SetRfChannel(pdot11Obj, mRfChannel, 1, mMacMode);
	mAssoc = TRUE;
	pdot11Obj->StatusNotify(STA_ASSOCIATED, (U8 *)IBSS_BSSID);
	
	return TRUE;
}
	
#endif

BOOLEAN zd_CmdConnect(U8 index, U8 bssType)
{
	if (bssType == INFRASTRUCTURE_BSS)
	{
		//printk(KERN_ERR "Build Infra-Type BSS\n");
#ifdef LINUX
		return zd_InfraConnect(index-1);
#endif
	}
#if ZDCONF_ADHOC_SUPPORT == 1
	else if (bssType==INDEPENDENT_BSS)
	{
		//printk(KERN_ERR "Build IBSS\n");
		return zd_IbssConnect();
	}
#endif
#if ZDCONF_PSEUDO_SUPPORT == 1
	else if (bssType == PSEUDO_IBSS)
		return zd_PseudoIbssConnect();
#endif
	
	
	return TRUE;
}	

BOOLEAN zd_CmdDisConnect(void)
{
	mAssoc = FALSE;
	mRequestFlag |= DIS_CONNECT_SET;
	
	return TRUE;
}

#ifdef LINUX
BOOLEAN zd_CmdRoaming(void)
{
        if ((mRequestFlag & (ROAMING_SET | CHANNEL_SCAN_SET)) || pdot11Obj->bChScanning || (pdot11Obj->ConfigFlag & SCAN_AND_CONNECT_SET))
            return FALSE;
       

    mAssoc = FALSE;
    
#if ZDCONF_ADHOC_SUPPORT == 1
    if (mBssType == INDEPENDENT_BSS)
        mRequestFlag |= CHANNEL_SCAN_SET;
    else
#endif
        mRequestFlag |= ROAMING_SET;

    return TRUE;

}


BOOLEAN zd_CmdFlushQ(void)
{
	if (pdot11Obj->QueueFlag & TX_QUEUE_SET){
		FlushQ(pTxQ);
	}
	else if (pdot11Obj->QueueFlag & MGT_QUEUE_SET){
		FlushQ(pMgtQ);	
	}
	else if (pdot11Obj->QueueFlag & AWAKE_QUEUE_SET){
		FlushQ(pAwakeQ);	
	}	
	
	return TRUE;
}
#endif	
#if Make_ADHOC
BOOLEAN zd_CmdProcess(U16 CmdId, void *parm1, U32 parm2)
{
	BOOLEAN status = TRUE;
	
	switch(CmdId){
#if LINUX		
		case CMD_RESET_80211:
			status = zd_Reset80211((zd_80211Obj_t *)parm1);
 			break;
		
		case CMD_ENABLE:
#if ZDCONF_AP_SUPPORT == 1
			if (mBssType == AP_BSS)
			    status = zd_StartAP();
			else 
#endif
                        {
                            if ((pdot11Obj->ConfigFlag & SCAN_AND_CONNECT_SET)||(mRequestFlag & CHANNEL_SCAN_SET))
                            {
                            //    printk("Scan and connect is underGoing\n");
                                break;
                            }
                	    else if (!mIfaceOpened)
                            {
            	                status = zd_StartSTA(1);
                                mIfaceOpened = TRUE;
                            }
                            else
                            {
                                pdot11Obj->ConfigFlag |= SCAN_AND_CONNECT_SET;
                                zd_ChooseAP(0);
                            }    
                        } 

			break;
			
		case CMD_DISASOC: //IAPP cth
			status = zd_CmdDisasoc((MacAddr_t*)parm1, (U8)parm2);
			break;
			
		case CMD_DEAUTH://MAC filter cth
			status = zd_CmdDeauth((MacAddr_t*)parm1, (U8)parm2);
			break;
			
#if ZDCONF_AP_SUPPORT == 1
		case CMD_PS_POLL:
			//PSDEBUG("CMD_PS_POLL");
			status = zd_HandlePsPoll((U8 *)parm1);
			break;	
#endif
		
		case CMD_PASSIVE_SCAN:
			status = zd_PassiveScan();
			break;
			
		case CMD_DISASOC_ALL:	
			status = zd_DisasocAll((U8)parm2);
			break;
#endif			
		case CMD_CONNECT:
			{
				U8 *pBsstype=(U8*)parm1;
				status = zd_CmdConnect((U8)parm2,*pBsstype);
			}
			break;
#if LINUX
		case CMD_PROBE_REQ:
			//FPRINT("CMD_PROBE_REQ");
			status = zd_CmdProbeReq((U8)parm2);
			pdot11Obj->ConfigFlag |= JUST_CHANNEL_SCAN;
			break;
			
		case CMD_DIS_CONNECT:
			status = zd_CmdDisConnect();
			break;		
			
		case CMD_FLUSH_QUEUE:
			status = zd_CmdFlushQ();
			break;

		case CMD_ROAMING:
			status = zd_CmdRoaming();
			break;      		
		
		default:
			status = FALSE;	
			break;		
#endif			
	}	
	
	return status;
}	
#endif
#if LINUX
//Event Nofify Functions
void zd_NextBcn(void)
{   
#if ZDCONF_AP_SUPPORT == 1
	if (mBssType == AP_BSS){
		if (mDtimCount == 0)
			mDtimCount = mDtimPeriod;
		mDtimCount--;
	}
#endif

	ConfigBcnFIFO();
	
	if (pTxQ->cnt)
   		pdot11Obj->QueueFlag |= TX_QUEUE_SET;
	return;
}	
#endif
#if ZDCONF_AP_SUPPORT == 1
void zd_DtimNotify(void)
{
	SendMcPkt();
	return; 
}
#endif

#ifdef LINUX
extern BOOLEAN Tchal_WaitChalRsp(Signal_t *signal);
void zd_SendTChalMsg(void)
{
	Tchal_WaitChalRsp(NULL);
	return;
}	

extern BOOLEAN AuthTimeOut(Signal_t *signal);
void zd_SendTAuthMsg(void)
{
	AuthTimeOut(NULL);
	return;
}	

extern BOOLEAN AsocTimeOut(Signal_t *signal);
void zd_SendTAsocMsg(void)
{
	AsocTimeOut(NULL);
	return;
}	


void zd_SwitchNextCH(void)
{
	void *reg = pdot11Obj->reg;
	static u8 LastScanMacMode;
	static u8 ScanAround = 0;
	//static u8 ScanWait = 0;
	static u8 initMAC_Mode = 0xff;
    struct zd1205_private *macp  = (struct zd1205_private *)g_dev->priv;

    if(initMAC_Mode == 0xff)
        initMAC_Mode = mMacMode;
	//FPRINT("zd_SwitchNextCH");

	if ((PURE_A_MODE != mMacMode && CurrScanCH > MAX_CHANNEL_ALLOW) 
#if ZDCONF_80211A_SUPPORT == 1
            || (PURE_A_MODE == mMacMode && CurrScanCH > dot11A_Channel_Amount - 1) 
#endif
		//In 11a, channel array index 0 is also meaningful.
	   ){ //Scan Finish...
#ifdef HMAC_DEBUG
	U8 i, j; 	
	U16 cap;
#endif

#if ZDCONF_AP_SUPPORT == 1
	if (mBssType == AP_BSS)
	    pdot11Obj->SetReg(reg, ZD_Rx_Filter, AP_RX_FILTER);
	else 
#endif
	    pdot11Obj->SetReg(reg, ZD_Rx_Filter, STA_RX_FILTER);
				
	if (pdot11Obj->ConfigFlag & PASSIVE_CHANNEL_SCAN_SET)
	    pdot11Obj->ConfigFlag &= ~PASSIVE_CHANNEL_SCAN_SET;
			
	if (pdot11Obj->ConfigFlag & IBSS_CHANNEL_SCAN_SET)
        {
	    pdot11Obj->ConfigFlag &= ~IBSS_CHANNEL_SCAN_SET;
	    //mRequestFlag |= IBSS_CONNECT_SET;
	}

	CurrScanCH = 1;
	pdot11Obj->bChScanning = 0;
    set_bit(CTX_FLAG_ESSID_WAS_SET, (void *)&macp->flags);
    zd1205_notify_scan_done(macp);
	ScanAround=0;
		
	if (pdot11Obj->ConfigFlag & ACTIVE_CHANNEL_SCAN_SET)
        {
#ifdef HMAC_DEBUG			
	    printk("\nSSID          BSSID            CH  Signal  Mode     Basic-Rates  Ext-Rates    b/g AP");
	    printk("\n------------------------------------------------------------------------------------");
	    for (i=0; i<mBssNum; i++) 
            {
	        printk("\n");                                              
		for (j=0; j<mBssInfo[i].ssid.buf[1]; j++) {
		    printk("%c", mBssInfo[i].ssid.buf[2+j]);
		}
			
		for (j=mBssInfo[i].ssid.buf[1]; j<12; j++) {
		    printk(" ");
		}
			
		printk("%02x:%02x:%02x:%02x:%02x:%02x",
			mBssInfo[i].bssid.mac[0], mBssInfo[i].bssid.mac[1], mBssInfo[i].bssid.mac[2],
			mBssInfo[i].bssid.mac[3], mBssInfo[i].bssid.mac[4], mBssInfo[i].bssid.mac[5]);

		printk("  %2d", mBssInfo[i].Phpm.buf[2]);
		printk("   %2d", mBssInfo[i].signalStrength);
			
		cap = mBssInfo[i].cap;
		cap &= (CAP_PRIVACY | CAP_IBSS | CAP_ESS);

		switch(cap) {
           	    case 0x01:
			printk("   Infra   ");
			break;
		    case 0x02:
			printk("   Ad_Hoc  ");
			break;
		    case 0x11:
			printk("   Infra, W");
			break;
		    case 0x12:
			printk("   Ad_Hoc,W");
			break;
	            default :
			break;
		}
				
		printk("  ");

		for (j=0; j<mBssInfo[i].supRates.buf[1]; j++) {
		    printk(" %x", mBssInfo[i].supRates.buf[2+j]);
		}
                
		printk("  ");
		for (j=0; j<mBssInfo[i].extRates.buf[1]; j++) {
                    printk(" %x", mBssInfo[i].extRates.buf[2+j]);
		}

		if (mBssInfo[i].apMode == PURE_B_AP)
		    printk("   B-AP");
		else if (mBssInfo[i].apMode == PURE_G_AP)
		    printk("   G-AP");
		else if  (mBssInfo[i].apMode == MIXED_AP)
		    printk("   M-AP");      
		}
		else if (mBssInfo[i].apMode == PURE_A_AP)
			printk("   A_AP");
		else
			VerAssert();


		printk("\n");

		FPRINT("****** Scan Finished ******");
#endif		
		pdot11Obj->ConfigFlag &= ~ACTIVE_CHANNEL_SCAN_SET;
		mBssCnt = mBssNum;
	}//End of ACTIVE_CHANNEL_SCAN_SET
	
	if (pdot11Obj->ConfigFlag & JUST_CHANNEL_SCAN){
	    pdot11Obj->ConfigFlag &= ~JUST_CHANNEL_SCAN;

        if (mAssoc || (mBssType == AP_BSS))
        {
            mMacMode = initMAC_Mode;
            HW_SetRfChannel(pdot11Obj, mRfChannel, 1, initMAC_Mode);
        }
    }
	else 
    {    
	    if (mBssType == INFRASTRUCTURE_BSS)
            zd_ChooseAP(0);
#if ZDCONF_ADHOC_SUPPORT == 1
	    if (mBssType == INDEPENDENT_BSS)
            zd_IbssConnect();
#endif
	}

	if (pdot11Obj->ConfigFlag & SCAN_AND_CONNECT_SET)
    {
        pdot11Obj->ConfigFlag &= ~SCAN_AND_CONNECT_SET;
	}
	initMAC_Mode = 0xff;
	return;
    }//End of (CurrentScannedChannel > MAX_CHANNEL_ALLOW)
	
    CurrScanCH++;
    if(mMacMode != PURE_A_MODE && CurrScanCH <= MAX_CHANNEL_ALLOW + 1) {
        //printk("HW_SetRfChannel:%s(%d)\n",__FILE__,__LINE__);
        HW_SetRfChannel(pdot11Obj, CurrScanCH, 1,mMacMode);
        LastScanMacMode = mMacMode;
    }
#if ZDCONF_80211A_SUPPORT == 1
    else if(mMacMode == PURE_A_MODE && CurrScanCH<=dot11A_Channel_Amount){
        //printk("HW_SetRfChannel:%s(%d)\n",__FILE__,__LINE__);
        HW_SetRfChannel(pdot11Obj, dot11A_Channel[CurrScanCH-1],1, mMacMode);
        LastScanMacMode = mMacMode;
    }
#endif
    if(PURE_A_MODE != LastScanMacMode && CurrScanCH > MAX_CHANNEL_ALLOW && ScanAround <1) {
#if ZDCONF_RF_AL7230B_SUPPORT == 1
        if(pdot11Obj->rfMode == AL7230B_RF) {
#if ZDCONF_80211A_SUPPORT == 1
            mMacMode = PURE_A_MODE;
            CurrScanCH = 1;
            ChangeMacMode(PURE_A_MODE,dot11A_Channel[CurrScanCH-1]);
            pdot11Obj->DelayUs(1000);
            HW_SetRfChannel(pdot11Obj, dot11A_Channel[CurrScanCH-1],0,mMacMode);            ScanAround ++;
#endif
        }
#endif


    }
#if ZDCONF_80211A_SUPPORT == 1
    else if(ScanAround < 1 &&PURE_A_MODE == LastScanMacMode && CurrScanCH > dot11A_Channel_Amount - 1){
        ScanAround ++;
        CurrScanCH = 1;
        mMacMode = MIXED_MODE;
        ChangeMacMode(MIXED_MODE, CurrScanCH);
        pdot11Obj->DelayUs(1000);
        HW_SetRfChannel(pdot11Obj, CurrScanCH, 0,mMacMode);
    }
#endif


    //for debug
    //pdot11Obj->SetReg(reg, ZD_USB_DEBUG_PORT, 0x22222222);
     
    if  (pdot11Obj->ConfigFlag & PASSIVE_CHANNEL_SCAN_SET)
	pdot11Obj->StartTimer(SCAN_TIMEOUT, DO_SCAN);
	
    if (pdot11Obj->ConfigFlag & ACTIVE_CHANNEL_SCAN_SET)
	zd_ProbeReq();

    return;
}
#endif

void zd_UpdateCurrTxRate(U8 rate, U16 aid)
{
	Hash_t *pHash;
	if (mBssType == INFRASTRUCTURE_BSS){
		pHash = sstByAid[0];
		pHash->CurrTxRate = rate;
	}
	else {
		if (aid){
			pHash = sstByAid[aid];
#if !defined(OFDM)		
			pHash->CurrTxRate = rate;
#else	
			if (rate < pHash->CurrTxRate){ //Retry Failed happened
				pHash->FailedFrames++;
				//FPRINT_V("FailedFrames", pHash->FailedFrames);
			}
#endif	
		}	
	}	
}

void zd_EnableProtection(U8 protect)
{
	U32 tmpValue;
	void *reg = pdot11Obj->reg;
	
    UpdateBcn++;
	if (protect){
		//FPRINT("zd_EnableProtection");
		pdot11Obj->ConfigFlag |= ENABLE_PROTECTION_SET;
		mErp.buf[2] |= USE_PROTECTION;
        if(pdot11Obj->bDeviceInSleep)
        {
            printk("In SLLLLLLLLLLLLLLLLLLP @ %s\n", __FUNCTION__);
            return ;
        }

		tmpValue = pdot11Obj->GetReg(reg, ZD_RTS_CTS_Rate); 
		tmpValue &= ~CTS_MOD_TYPE_OFDM;
		tmpValue |= CTS_RATE_11M;
		pdot11Obj->SetReg(reg, ZD_RTS_CTS_Rate, tmpValue);
	}
	else {
		//FPRINT("zd_DisableProtection");
		pdot11Obj->ConfigFlag &= ~ENABLE_PROTECTION_SET;
		mErp.buf[2] &= ~USE_PROTECTION;
		
		//pdot11Obj->ConfigFlag &= ~SHORT_SLOT_TIME_SET;
		mCap |= CAP_SHORT_SLOT_TIME;	
		pdot11Obj->SetReg(reg, ZD_CWmin_CWmax, CW_SHORT_SLOT); 
	}		
}

void zd_EnableBarker(U8 barker)
{
	void *reg = pdot11Obj->reg;
    UpdateBcn++;
	if (barker){
		//FPRINT("zd_EnableBarker");
		pdot11Obj->ConfigFlag |= BARKER_PREAMBLE_SET;
		mErp.buf[2] |= BARKER_PREAMBLE;
		pdot11Obj->SetReg(reg, ZD_RTS_CTS_Rate, 0x30000);
	}
	else {
		//FPRINT("zd_DisableBarker");
		pdot11Obj->ConfigFlag &= ~BARKER_PREAMBLE_SET;
		mErp.buf[2] &= ~BARKER_PREAMBLE;
		pdot11Obj->SetReg(reg, ZD_RTS_CTS_Rate, 0x30000);

	}
	if(PURE_A_MODE == mMacMode) //Use Slowest rate when CTS/RTS,MZCai
                pdot11Obj->SetReg(reg, ZD_RTS_CTS_Rate,0x01090109);
	else
                pdot11Obj->SetReg(reg, ZD_RTS_CTS_Rate, 0x30000);

}

void zd_EnableShortSlot(U8 slot)
{
	void *reg = pdot11Obj->reg;
	if (slot){
		//FPRINT("zd_EnableShortSlot");
		pdot11Obj->ConfigFlag |= SHORT_SLOT_TIME_SET;
		pdot11Obj->SetReg(reg, ZD_IFS_Value, 0xa47c00a);
		pdot11Obj->SetReg(reg, ZD_CWmin_CWmax, CW_NORMAL_SLOT);
	}
	else {
		//FPRINT("zd_DisableShortSlot");
		pdot11Obj->ConfigFlag &= ~SHORT_SLOT_TIME_SET;
		pdot11Obj->SetReg(reg, ZD_IFS_Value, 0xa47c032);
		pdot11Obj->SetReg(reg, ZD_CWmin_CWmax, CW_LONG_SLOT);
	}
}	

void zd_PsChange(U8 PwrState)
{
	//FPRINT("zd_PsChange");
	
	mPwrState = PwrState;
	mRequestFlag |= PS_CHANGE_SET;
	return;
}

void zd_EventNotify(U16 EventId, U32 parm1, U32 parm2, U32 parm3)
{
#ifdef LINUX
	//struct zd1205_private *macp = g_dev->priv;
	

	switch(EventId){
#ifdef LINUX
		case EVENT_TBCN:
			zd_NextBcn();
			break;
#endif
#if ZDCONF_AP_SUPPORT == 1			
		case EVENT_DTIM_NOTIFY:
			zd_DtimNotify();
			break;		
#endif
			
		case EVENT_TX_COMPLETE:
			TxCompleted(parm1, (U8)parm2, (U16)parm3);
			break;
			
		case EVENT_TCHAL_TIMEOUT:	
			zd_SendTChalMsg();	
			break;
			
		case EVENT_SCAN_TIMEOUT:
			zd_SwitchNextCH();
			break;	
			
		case EVENT_UPDATE_TX_RATE:
			zd_UpdateCurrTxRate((U8)parm1, (U16)parm2);
			break;		
		
		case EVENT_SW_RESET:
			//zd_SwReset();
			break;

		case EVENT_BUF_RELEASE:
			zd_Release_80211_Buffer();
			break;

		case EVENT_AUTH_TIMEOUT:
			zd_SendTAuthMsg();
			break;	
			
		case EVENT_ASOC_TIMEOUT:
			zd_SendTAsocMsg();
			break;	
			
		case EVENT_PS_CHANGE:
			zd_PsChange((U8)parm1);
			break;
			
		case EVENT_MORE_DATA:
			mRequestFlag |= PS_POLL_SET;
			break;	
			
		case EVENT_ENABLE_PROTECTION:
			zd_EnableProtection((U8)parm1);
			break;

		case EVENT_ENABLE_BARKER:
			zd_EnableBarker((U8)parm1);
			break;

		case EVENT_SHORT_SLOT:
			zd_EnableShortSlot((U8)parm1);
			break;

		default:
			break;		
	}	
	
	return;
#else
    MP_ASSERT(0);
#endif
}	

BOOLEAN zd_CleanupTxQ(void)
{	
#ifdef LINUX
	//FPRINT("*****zd_CleanupTxQ*****");
    int loopCheck = 0;
    int CleanResult = 1;
	while(CleanResult)
    {
        // To prevent unknown reason that can't clean up the txq(ex. re-queue in CleanupTxQ())
        if(loopCheck++ > 100)
        {
            printk("infinite loop occurs in %s\n", __FUNCTION__);
            loopCheck = 0;
            break;
        }
        CleanResult = CleanupTxQ();
    }
    
	if (!pTxQ->cnt){
   		pdot11Obj->QueueFlag &= ~TX_QUEUE_SET;
		return TRUE;
	}
	else
		return FALSE;
#else
    MP_ASSERT(0);
    return FALSE;
#endif
}  

#if ZDCONF_AP_SUPPORT == 1
BOOLEAN zd_CleanupAwakeQ(void)
{	
	//PSDEBUG("*****zd_CleanupAwakeQ*****");
    int loopCheck = 0;
	while(1)
    {
        // To prevent unknown reason that can't clean Awake Queue
        if(loopCheck++ > 100)
        {
            printk("infinite loop occurs in %s\n", __FUNCTION__);
            loopCheck = 0;
            break;
        }
        if(!CleanupAwakeQ()) break;
    }
    
	if (!pAwakeQ->cnt){
   		pdot11Obj->QueueFlag &= ~AWAKE_QUEUE_SET;
		return TRUE;
	}
	else{
#if 0
		Signal_t *signal;
		FrmInfo_t *pfrmInfo;
		FrmDesc_t *pfrmDesc;
        
		while(pAwakeQ->cnt){
			signal = sigDeque(pAwakeQ);
			pfrmInfo = &signal->frmInfo;
			pfrmDesc = pfrmInfo->frmDesc;
			freeFdesc(pfrmDesc);
			pdot11Obj->ReleaseBuffer(signal->buf);
			freeSignal(signal);
		}
#endif		
		return FALSE;
	}
}
#endif
void zd_ShowQInfo(void)
{
#if ZDCONF_DBGMSG_NORMAL == 1
	printk(KERN_DEBUG "AwakeQ = %x, MgtQ = %x, TxQ  = %x, mcQ  = %x\n",
	pAwakeQ->cnt, pMgtQ->cnt, pTxQ->cnt, pPsQ[0]->cnt);
	printk(KERN_DEBUG "PsQ1   = %x, PsQ2 = %x, PsQ3 = %x, PsQ4 = %x\n",
	pPsQ[1]->cnt, pPsQ[2]->cnt, pPsQ[3]->cnt, pPsQ[4]->cnt);     
#endif
}

extern U8 AuthReqState;
extern U8 AsocState;
void zd_ShowState(void)
{
#if ZDCONF_DBGMSG_NORMAL == 1
#ifdef LINUX
	printk(KERN_DEBUG "AuthReqState    = %04x, AsocState      = %04x\n", AuthReqState, AsocState);
	printk(KERN_DEBUG "mPwrState       = %04x, mAssoc         = %04x\n", mPwrState, mAssoc);
	printk(KERN_DEBUG "mAuthAlg        = %04x, mBssIndex      = %04x\n", mAuthAlg, mBssIndex);
	printk(KERN_DEBUG "mBssType        = %04x, ConfigFlag     = %04x\n", mBssType, pdot11Obj->ConfigFlag);
#endif
#endif
}
	
void zd_ShowHashInfo(U8 aid)
{
#if ZDCONF_DBGMSG_NORMAL == 1
	Hash_t *pHash = NULL;
	if (mBssType == INFRASTRUCTURE_BSS){
		aid = 0;
	}	
		pHash = sstByAid[aid];
		zd1205_dump_data("Mac Addr = ", pHash->mac, 6);
		FPRINT_V("Auth", pHash->auth);
		FPRINT_V("Asoc", pHash->asoc);
		FPRINT_V("psm", pHash->psm);
		FPRINT_V("Aid", pHash->aid);
		FPRINT_V("vapId", pHash->vapId);
		FPRINT_V("bErpSta", pHash->bErpSta);
		FPRINT_V("lsInterval", pHash->lsInterval);
		FPRINT_V("encryMode", pHash->encryMode);
		FPRINT_V("pkInstalled", pHash->pkInstalled);
		FPRINT_V("ZydasMode", pHash->ZydasMode);
		FPRINT_V("AlreadyIn", pHash->AlreadyIn);
		FPRINT_V("CurrTxRate", pHash->CurrTxRate);
		FPRINT_V("MaxRate", pHash->MaxRate);
		FPRINT_V("Preamble", pHash->Preamble);
		FPRINT_V("KeyId", pHash->KeyId);
		FPRINT_V("Rx IV16", pHash->RxSeed.IV16);
		FPRINT_V("Rx IV32", pHash->RxSeed.IV32);
		zd1205_dump_data("TK = ", pHash->TxSeed.TK, 16);
		zd1205_dump_data("Tx MIC K0 = ", (U8 *)&pHash->TxMicKey.K0, 4);
		zd1205_dump_data("Tx MIC K1 = ", (U8 *)&pHash->TxMicKey.K1, 4);
		zd1205_dump_data("Rx MIC K0 = ", (U8 *)&pHash->RxMicKey.K0, 4);
		zd1205_dump_data("Rx MIC K1 = ", (U8 *)&pHash->RxMicKey.K1, 4);
#if 0
		FPRINT_V("KeyId", mWpaBcKeyId);
		FPRINT_V("GkInstalled", mGkInstalled);
		FPRINT_V("IV16", mIv16);
		FPRINT_V("IV32", mIv32);
		zd1205_dump_data("keyContent = ", pHash->keyContent, 16);
		zd1205_dump_data("TK = ", mBcSeed.TK, 16);
		zd1205_dump_data("Tx MIC K0 = ", (U8 *)&mBcMicKey.K0, 4);
		zd1205_dump_data("Tx MIC K1 = ", (U8 *)&mBcMicKey.K1, 4);
#endif
#endif
}

void zd_UpdateCardSetting(card_Setting_t *pSetting)
{
#if 1//def LINUX
	void *reg = pdot11Obj->reg;
	static BOOLEAN InitConfig = TRUE;
	U8 bcAddr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	U32 tmpValue;
	BOOLEAN bReConnect = FALSE;
	//FPRINT("zd_UpdateCardSetting");
    UpdateBcn++;
    
	if (pSetting->AuthMode == 0){ //open system only
		mAuthAlogrithms[0] = OPEN_SYSTEM;
		mAuthAlogrithms[1] = NULL_AUTH;
		mAuthAlg = OPEN_SYSTEM;
	}
	else if (pSetting->AuthMode == 1){	//shared key only
		mAuthAlogrithms[0] = SHARE_KEY;
		mAuthAlogrithms[1] = NULL_AUTH;
		mAuthAlg = SHARE_KEY;
	}
	else if (pSetting->AuthMode == 2){	//auto auth mode
		mAuthAlogrithms[0] = OPEN_SYSTEM;
		mAuthAlogrithms[1] = SHARE_KEY;
        
	}

#ifdef LINUX
	if (mAuthMode != pSetting->AuthMode){
		if (!InitConfig)
			bReConnect = TRUE;
	}
#endif

	mAuthMode = pSetting->AuthMode;     	
	
	if (mLimitedUser != pSetting->LimitedUser){
		mLimitedUser = pSetting->LimitedUser;
	}
		
	mBlockBSS = pSetting->BlockBSS;
	mSwCipher = pSetting->SwCipher;	
	mKeyFormat = pSetting->EncryMode;
	mKeyId = pSetting->EncryKeyId;
	mBcKeyId = pSetting->BcKeyId;
	mDynKeyMode = pSetting->DynKeyMode;
	mFragThreshold = pSetting->FragThreshold;
	mRtsThreshold = pSetting->RTSThreshold;
#ifdef LINUX
	mBeaconPeriod = pSetting->BeaconInterval;
	mDtimPeriod = pSetting->DtimPeriod;
	
	if (!InitConfig)
    {
		MP_ASSERT(0);
        if (pSetting->BssType == INFRASTRUCTURE_BSS) // In IBSS mode, Enable beacon transmission at IBSS creation or Join.
            HW_EnableBeacon(pdot11Obj, mBeaconPeriod, mDtimPeriod, pSetting->BssType);
        
    }
#endif

		//HW_EnableBeacon(pdot11Obj, mBeaconPeriod, mDtimPeriod, mBssType);
	
	if (mRadioOn != pSetting->RadioOn){
	 	mRadioOn = pSetting->RadioOn;
	 	if (!InitConfig)
	 		HW_RadioOnOff(pdot11Obj, mRadioOn);
	 }	
	
#ifdef LINUX
    mPhpm.buf[0] = EID_DSPARMS;
    mPhpm.buf[1] = 1;
    mPhpm.buf[2] = mRfChannel;
	if (mRfChannel != pSetting->Channel){
		mRfChannel = pSetting->Channel;
		mPhpm.buf[0] = EID_DSPARMS;
		mPhpm.buf[1] = 1;
		mPhpm.buf[2] = mRfChannel;

		if (!InitConfig){
			if (pSetting->BssType != INFRASTRUCTURE_BSS)
            {
				HW_SetRfChannel(pdot11Obj, mRfChannel, 0,pSetting->MacMode);
#if ZDCONF_AP_SUPPORT == 1
                if(pSetting->BssType == AP_BSS)
                    HW_SetRfChannel(pdot11Obj, mRfChannel, 1,pSetting->MacMode);
#endif

            }

#if ZDCONF_ADHOC_SUPPORT == 1                
			if (pSetting->BssType == INDEPENDENT_BSS){
				mRequestFlag |= CHANNEL_SCAN_SET;
				pdot11Obj->ConfigFlag |= SCAN_AND_CONNECT_SET;
				//bReConnect = FALSE;
			}
#endif
		}    
	}	
#endif
	
	mPreambleType = pSetting->PreambleType;	

	if (mPreambleType)
		mCap |= CAP_SHORT_PREAMBLE;
	else
		mCap &= ~CAP_SHORT_PREAMBLE;
	
	mPrivacyInvoked = pSetting->EncryOnOff;
	if (pSetting->DynKeyMode > 0)
		mPrivacyInvoked = TRUE;
		
	if (mPrivacyInvoked)
		mCap |= CAP_PRIVACY;
	else
		mCap &= ~CAP_PRIVACY;	
		
	memcpy(&dot11DesiredSsid, pSetting->Info_SSID,  pSetting->Info_SSID[1]+2);

	if (dot11DesiredSsid.buf[1] == 0)
		mProbeWithSsid = FALSE; 
	else
    {
        memcpy(&mSsid, &dot11DesiredSsid,  dot11DesiredSsid.buf[1]+2);
		mProbeWithSsid = TRUE;
    }
    
	//mProbeWithSsid = FALSE;  //debug for ANY connection   
		
#ifdef LINUX
	if ((pSetting->BssType == INFRASTRUCTURE_BSS) || (pSetting->BssType == INDEPENDENT_BSS)) {
	        if (!InitConfig) {
         //   if (memcmp(&mSsid, &dot11DesiredSsid,  dot11DesiredSsid.buf[1]+2) != 0 ){
				bReConnect = TRUE;
        //    }
		}
#if ZDCONF_ADHOC_SUPPORT == 1        
		if (pSetting->BssType == INDEPENDENT_BSS){
			memcpy(&mSsid, &dot11DesiredSsid,  dot11DesiredSsid.buf[1]+2);
			mATIMWindow = pSetting->ATIMWindow;
		}	   
#endif
	}    
#endif
	

	mHiddenSSID = pSetting->HiddenSSID;
	if (mHiddenSSID){
		mSsid.buf[0] = EID_SSID;
		mSsid.buf[1] = 1;
		mSsid.buf[2] = 0x0;
	}
	
	memcpy(&mBrates, pSetting->Info_SupportedRates, pSetting->Info_SupportedRates[1]+2);

#ifdef LINUX
	if (!InitConfig){
#if defined(AMAC)		
		pdot11Obj->SetReg(reg, ZD_BasicRateTbl, 0);
#endif		
		HW_SetSupportedRate(pdot11Obj, (U8 *)&mBrates);
	}	
#endif

#ifdef LINUX	// TODO
#if defined(OFDM)	
	if (pSetting->MacMode != PURE_B_MODE){
		if (pSetting->ShortSlotTime){
			pdot11Obj->ConfigFlag |= SHORT_SLOT_TIME_SET;
			mCap |= CAP_SHORT_SLOT_TIME;
		}	
		else{
			pdot11Obj->ConfigFlag &= ~SHORT_SLOT_TIME_SET;
			mCap &= ~CAP_SHORT_SLOT_TIME;	
		}
		
		mMaxTxRate = 0x0b; 	
		if(PURE_A_MODE != pSetting->MacMode) {
			memcpy(&mExtRates, pSetting->Ext_SupportedRates, pSetting->Ext_SupportedRates[1]+2);
			if (!InitConfig)
				HW_SetSupportedRate(pdot11Obj, (U8 *)&mExtRates);
		}

	}
	else
		mMaxTxRate = 0x03;

	if (!InitConfig) {
		if (mMacMode != pSetting->MacMode) {   //MacMode changed
			bReConnect = TRUE;
		}
	}        

	mMacMode = pSetting->MacMode;     		
#endif		
#endif
		
	memcpy((U8 *)&dot11MacAddress, pSetting->MacAddr, 6);
	memcpy(&mKeyVector[0][0], &pSetting->keyVector[0][0], sizeof(mKeyVector));
	mWepKeyLen = pSetting->WepKeyLen;
	memcpy(&mBcKeyVector[0], &pSetting->BcKeyVector[0], sizeof(mBcKeyVector));

	mBcKeyLen = pSetting->BcKeyLen;

	/* Check if we need to copy the WPA IE */	
	//if ((mDynKeyMode == DYN_KEY_TKIP) || (mDynKeyMode == DYN_KEY_AES)
	  //  || (pSetting->WPASupport == 1 && pSetting->WPAIe[1] != 0)){
	if ((pSetting->WPASupport==1 || pSetting->WPAIeLen))
	{
		//printk(KERN_ERR "Copy WPA IE in the zd_UpdateCardSetting\n");
			memcpy(&mWPAIe[0], pSetting->WPAIe, pSetting->WPAIeLen);
	}
    else 
    {
		//printk(KERN_ERR "Copy WPA IE 0 in the zd_UpdateCardSetting\n");
        memset(&mWPAIe[0], 0, sizeof(mWPAIe));
    }


#ifdef LINUX
#if defined(AMAC)
	mOperationMode = pSetting->OperationMode;
	if (!InitConfig)
	{
		//HW_CAM_ResetRollTbl(pdot11Obj);
#if ZDCONF_AP_SUPPORT == 1
		if ((mOperationMode == CAM_AP_VAP) || (mOperationMode == CAM_AP_CLIENT)){
			// for Ack response
			HW_CAM_ResetRollTbl(pdot11Obj);
			HW_CAM_SetMAC(pdot11Obj, CAM_VAP_START_AID, (U8 *)&dot11MacAddress);
			HW_CAM_UpdateRollTbl(pdot11Obj, CAM_VAP_START_AID);
			
			// for Address1 matching
			HW_CAM_SetMAC(pdot11Obj, 0, (U8 *)&bcAddr);
			HW_CAM_UpdateRollTbl(pdot11Obj, 0);	
		}
#endif
        if (pSetting->BssType == INFRASTRUCTURE_BSS)
		{// Don't clear key in AP_BSS and IBSS mode.
		//	HW_CAM_ClearRollTbl(pdot11Obj, CAM_VAP_START_AID);	
		//	HW_CAM_ClearRollTbl(pdot11Obj, 0);	
		    HW_CAM_ResetRollTbl(pdot11Obj); // Clear all bits.
		}	
	}
	else
	{// Only clear all keys in the first time.
		HW_CAM_ResetRollTbl(pdot11Obj);
	}

#endif	
#else
	mOperationMode = pSetting->OperationMode;
	if (!InitConfig)
	{
		//HW_CAM_ResetRollTbl(pdot11Obj);
        if (pSetting->BssType == INFRASTRUCTURE_BSS)
		{// Don't clear key in AP_BSS and IBSS mode.
		//	HW_CAM_ClearRollTbl(pdot11Obj, CAM_VAP_START_AID);	
		//	HW_CAM_ClearRollTbl(pdot11Obj, 0);	
		    HW_CAM_ResetRollTbl(pdot11Obj); // Clear all bits.
		}	
	}
	else
	{// Only clear all keys in the first time.
		HW_CAM_ResetRollTbl(pdot11Obj);
	}
#endif
    
	//mPwrState = pSetting->PwrState;
#if ZDCONF_AP_SUPPORT == 1
	if (pSetting->BssType == AP_BSS)
    {
		memcpy(&mSsid, &dot11DesiredSsid,  dot11DesiredSsid.buf[1]+2);
		memcpy((U8 *)&mBssId, pSetting->MacAddr, 6);
		
		// Update the mCap information
		mCap &= ~BIT_1;
		mCap |= BIT_0;
		
		//mGkInstalled = 0;
		zd_InitWepData();	
		if (!InitConfig){
			zd_CmdProcess(CMD_DISASOC_ALL, 0, ZD_UNSPEC_REASON);
		}	

	}
#endif
	
#ifdef LINUX
#if defined(OFDM)
	mErp.buf[2] = 0; //reset erp info
	
	if ((mCap & CAP_SHORT_PREAMBLE) == 0){
		mErp.buf[2] |= BARKER_PREAMBLE; 
		if (pdot11Obj){
            if(pdot11Obj->bDeviceInSleep)
            {
                printk("Sleeping... @ %s\n", __FUNCTION__);
                return ;
            }

			tmpValue = pdot11Obj->GetReg(reg, ZD_RTS_CTS_Rate); 
			tmpValue &= ~NON_BARKER_PMB_SET;
			tmpValue |= CTS_RATE_11M;
			pdot11Obj->SetReg(reg, ZD_RTS_CTS_Rate, tmpValue); 
		}	
	}	
	
	if (pdot11Obj)
		pdot11Obj->ConfigFlag &= ~ENABLE_PROTECTION_SET;
#if ZDCONF_ADHOC_SUPPORT == 1	
	if (pSetting->BssType == INDEPENDENT_BSS){
		if (mMacMode == PURE_G_MODE)
			mErp.buf[2] = 0;
		else
			mErp.buf[2] = (NON_ERP_PRESENT | USE_PROTECTION | BARKER_PREAMBLE);	
	}	
#endif
#endif	
#endif
	
	if (!InitConfig){
		//if (mBssType != pSetting->BssType){
		if (mBssType != pSetting->BssType)
		//if (pSetting->BssType != INFRASTRUCTURE_BSS)
        {
			//int i;
#if LINUX
			mBssType = pSetting->BssType;
#if ZDCONF_AP_SUPPORT == 1
			if (pSetting->BssType == AP_BSS){
				zd_StartAP();
 				bReConnect = FALSE;
 			}
			else
#endif
            {
				zd_StartSTA(0);
				bReConnect = FALSE;
			}
				
			//for (i=0; i<(MAX_AID+1); i++)
				//CleanupHash(sstByAid[i]);
			//InitHashTbl();	
			zd_InitWepData();	
			//zd_CmdFlushQ();	
#endif	
			
		}	
	}
	
	mBssType = pSetting->BssType;
	pdot11Obj->BssType = mBssType;



#ifdef LINUX
    if (bReConnect)
    {
        if (pSetting->BssType == INFRASTRUCTURE_BSS)
        {
		    BOOLEAN ret;
		    pdot11Obj->ConfigFlag |= SCAN_AND_CONNECT_SET;
	#if	0	
			if (pSetting->ap_scan == 1)
			{
				ret = zd_ChooseAP(1); // Choose AP by dot11DesiredBssid.
			}
			else
				ret = zd_ChooseAP(0);
	#endif		
    
	    	if (ret == FALSE)
	        {
	            zd_StartSTA(0);//ReScan all channels to find the selected BSS.
	        }
		}
#if ZDCONF_ADHOC_SUPPORT == 1 || ZDCONF_PSEUDO_SUPPORT == 1
        else if(pSetting->BssType == INDEPENDENT_BSS || pSetting->BssType == PSEUDO_IBSS)
        {
            zd_StartSTA(0);
            printk("Call StartSTA(IBSS or AP)\n");
        }
#endif
	}
#endif
	
	InitConfig = FALSE;
#else
    MP_ASSERT(0);
#endif
}	


void zd_PsPoll(void)
{
#if ZDCONF_STA_PSM == 1
	Signal_t *signal;
	FrmDesc_t *pfrmDesc;
	
	//FPRINT("zd_PsPoll");
	
	if ((signal = allocSignal()) == NULL){
		return;
	}	
		
	pfrmDesc = allocFdesc();
	if(!pfrmDesc){
		freeSignal(signal);
		return;
	}
	
	sendPsPollFrame(signal, pfrmDesc, &mBssId, mAid);
	mRequestFlag &= ~PS_POLL_SET;
	return;
#endif
}
#ifdef LINUX
void zd_AP_NullData(U8 *mac, int aid)
{
    Signal_t *signal;
    FrmDesc_t *pfrmDesc;
    
    //FPRINT("zd_NullData");
    
    if(!mac) while(1) printk("%s:%d\n", __FUNCTION__, __LINE__);
    if ((signal = allocSignal()) == NULL){
        return;
    }   
        
    pfrmDesc = allocFdesc();
    if(!pfrmDesc){
        freeSignal(signal);
        return;
    }
    
    AP_sendNullDataFrame(signal, pfrmDesc, (MacAddr_t * )mac, aid);
    return;
}

void zd_NullData(void)
{
	Signal_t *signal;
	FrmDesc_t *pfrmDesc;
	
	//FPRINT("zd_NullData");
	
	if ((signal = allocSignal()) == NULL){
		return;
	}	
		
	pfrmDesc = allocFdesc();
	if(!pfrmDesc){
		freeSignal(signal);
		return;
	}
	
	sendNullDataFrame(signal, pfrmDesc, &mBssId);
	mRequestFlag &= ~PS_CHANGE_SET;
	return;
}
#endif

#ifdef LINUX
void zd_DisConnect(void)
{
	//FPRINT("zd_DisConnect");
	
        zd_CmdProcess(CMD_ROAMING,0,0);
	mRequestFlag &= ~DIS_CONNECT_SET;
	
	/*pdot11Obj->ConfigFlag |= SCAN_AND_CONNECT_SET;
	pdot11Obj->bChScanning = 0;
	mRequestFlag &= ~DIS_CONNECT_SET;
#if 0    
	zd_CmdProbeReq((U8)mProbeWithSsid);
#else
	zd_ChooseAP();
#endif    */
}

void zd_Roaming(void)
{
	//FPRINT("zd_Roaming");
	pdot11Obj->ConfigFlag |= SCAN_AND_CONNECT_SET;
	//pdot11Obj->bChScanning = 0;
	mRequestFlag &= ~ROAMING_SET;
	zd_CmdProbeReq((U8)mProbeWithSsid);
}
	
void zd_ConnectMon(void)
{
	//FPRINT_V("mRequestFlag", mRequestFlag);
	if (mRequestFlag & DIS_CONNECT_SET){
		mRequestFlag = 0;
		FPRINT("DIS_CONNECT_SET");
		zd_DisConnect();
		goto end;
	}

	if (mRequestFlag & ROAMING_SET){
		mRequestFlag = 0;
		FPRINT("ROAMING_SET");
		zd_Roaming();
		goto end;
	}	
    
	if (mRequestFlag & CHANNEL_SCAN_SET){
		//FPRINT("CHANNEL_SCAN_SET");
		mRequestFlag &= ~BSS_CONNECT_SET;
		mRequestFlag &= ~CONNECT_TOUT_SET;
		zd_CmdProbeReq((U8)mProbeWithSsid);
		goto end;
	}
      
	if (mRequestFlag & BSS_CONNECT_SET){
		//FPRINT("BSS_CONNECT_SET");
		mRequestFlag &= ~CHANNEL_SCAN_SET;
		mRequestFlag &= ~CONNECT_TOUT_SET;
		zd_InfraConnect(mBssIndex);
		goto end;
	}

	if (mRequestFlag & CONNECT_TOUT_SET){
		//FPRINT("CONNECT_TOUT_SET");
		mConnRetryCnt++;

        if ((mConnRetryCnt > 6) || (pdot11Obj->bChScanning)){
			mRequestFlag &= ~CONNECT_TOUT_SET;
			mConnRetryCnt = 0;
			mSsid.buf[1] = 0; //reset mSsid
			//return;
		}
		else {
			FPRINT("Connect Timeout, Re-Connect...");
			zd_InfraConnect(mBssIndex);
			//return;
		}
		goto end; 
	}
    
	if (mRequestFlag & PS_POLL_SET){
		zd_PsPoll();
		goto end;
	}
    
	if (mRequestFlag & PS_CHANGE_SET){
		zd_NullData();
		goto end;
	}
#if ZDCONF_ADHOC_SUPPORT == 1 
	if (mRequestFlag & IBSS_CONNECT_SET){
		zd_IbssConnect();
		goto end;
	}
#endif
end:         	
	zd_SigProcess();
}	
#endif

extern BOOLEAN SynchEntry(Signal_t* signal);
extern BOOLEAN AuthReqEntry(Signal_t* signal);
extern BOOLEAN AuthRspEntry(Signal_t* signal);
extern BOOLEAN AsocEntry(Signal_t* signal);	
//State machine entry point
#ifdef LINUX
void zd_SigProcess(void)
{
	Signal_t* 	signal = NULL;
	BOOLEAN		ret;
    int loopChk = 0;
	
	while((signal = sigDeque(&mgtQ)) != NULL){
        if(loopChk++ > 100)
        {
            sigEnque(&mgtQ, signal);
	        while((signal = sigDeque(&mgtQ)) != NULL)       
            {
                printk("We are trapped in %s, Going to release all,id=%d,block=%d\n", __FUNCTION__,signal->id,signal->block);
                //if((signal->id == 1) && (signal->block == 1))
                {
                    FrmDesc_t *pfrmDesc;
                    Frame_t *rdu;
                    MacAddr_t sta;
                    int i;

                    pfrmDesc = signal->frmInfo.frmDesc;
                    rdu = pfrmDesc->mpdu;
                    for(i=0;i<24;i++)
                        printk("%02x ", rdu->header[i] & 0xFF);
                    printk("\n");
                }


                if(signal->frmInfo.frmDesc != NULL)
                {
                    freeFdesc(signal->frmInfo.frmDesc);
                    signal->frmInfo.frmDesc = NULL;
                }
                pdot11Obj->ReleaseBuffer(signal->buf);
                freeSignal(signal);
            }
            break;
        }
		switch(signal->block){
			case BLOCK_SYNCH:
				ret = SynchEntry(signal);
				break;
				
			case BLOCK_AUTH_REQ:
				ret = AuthReqEntry(signal);
				break;	
				
			case BLOCK_AUTH_RSP:
				ret = AuthRspEntry(signal);
				break;	
				
			case BLOCK_ASOC:
				ret = AsocEntry(signal);
				break;	
				
			default:
				ret = TRUE;
				break;		
		}	
		
		if (ret){
            if(signal->frmInfo.frmDesc != NULL)
            {
                freeFdesc(signal->frmInfo.frmDesc);
                signal->frmInfo.frmDesc = NULL;
            }
			pdot11Obj->ReleaseBuffer(signal->buf);
			freeSignal(signal);

		}	
	}
	pdot11Obj->QueueFlag &= ~MGT_QUEUE_SET;
}

U8 zd_CheckTotalQueCnt(void)
{
	U8 TotalQueCnt = 0;
	U32 flags;
	int i;
		
	flags = pdot11Obj->EnterCS();
	
	for (i=0; i<MAX_AID+1; i++)
		TotalQueCnt += pPsQ[i]->cnt;
		
	TotalQueCnt += pAwakeQ->cnt;
	TotalQueCnt += pTxQ->cnt;
	TotalQueCnt += pMgtQ->cnt;	
	pdot11Obj->ExitCS(flags);
	

	return TotalQueCnt;
}	
void zd_RateAdaption(BOOLEAN FORCED_DOWN)
{
    int i=1;
    U32 SucessFrmCnt;
    U32 FailFmrCnt;
    Hash_t *pHash;
    U32 Ratio = 0;
    U32 UserLimit=1;
    U8 idx,totalUser;

						MP_ASSERT(0);
#if defined(OFDM)
//For STA, it do RateAdaption with AP only
//For Others, with Associated STAs
    if(INFRASTRUCTURE_BSS == mBssType)
    {
        i=0;
        UserLimit=0;
    }
    //for(idx=0,totalUser=0;idx<=NUM_VAP;idx++)
    //    totalUser += mCurrConnUser[i];
    totalUser = mCurrConnUser;
    if (totalUser >= UserLimit)
    {
        for (; i<(MAX_AID+1); i++)
        {
            pHash = sstByAid[i];
            if (pHash->bValid)
            {
#if ZDCONF_RATE_DEGRADE == 1
                if(FORCED_DOWN)
                {
                    if(pHash->CurrTxRate > 0 && PURE_A_MODE != mMacMode)
                    {
                        pHash->CurrTxRate--;
						MP_ASSERT(0);
                        if(pdot11Obj->rfMode == UW2453_RF || pdot11Obj->rfMode == AR2124_RF)
                            if(pHash->CurrTxRate == 1)
                                pHash->CurrTxRate --;

                    }
                    else if(pHash->CurrTxRate > 4 && PURE_A_MODE == mMacMode)
                        pHash->CurrTxRate--;
                }
#endif
                SucessFrmCnt = pHash->SuccessFrames;
                FailFmrCnt = pHash->FailedFrames;

//FPRINT_V("Aid", i);

                if (SucessFrmCnt + FailFmrCnt < 20)
                {
//FPRINT(" FmrCnt < 20");
                    continue;
                }

                if ((SucessFrmCnt) && (FailFmrCnt == 0))
                {
                    pHash->RiseConditionCount++;
                    pHash->DownConditionCount = 0;
//FPRINT(" FailFmrCnt == 0");
                    goto JudgeRate;
                }
                else
                {
                    Ratio = SucessFrmCnt / FailFmrCnt;
                    if (Ratio >= 9 && pHash->CurrTxRate >= RATE_36M)
                    {
                       pHash->RiseConditionCount++;
                      pHash->DownConditionCount = 0;
                       goto JudgeRate;
                    }
                    else if (Ratio > LINE1)       //100
                    {
//FPRINT(" > LINE1");
                        if (pHash->CurrTxRate > RATE_36M)
                        {
                            pHash->RiseConditionCount = 0;
                            pHash->DownConditionCount++;
                            goto JudgeRate;
                        }
                        else if (pHash->CurrTxRate == RATE_36M)
                        {
                            pHash->RiseConditionCount = 0;
                            pHash->DownConditionCount = 0;
                            goto JudgeRate;
                        }
                        else
                        {
                            pHash->RiseConditionCount++;
                            pHash->DownConditionCount = 0;
                            goto JudgeRate;
                        }
                    }                             // LINE1
                    else
                    {
                        if (Ratio >= LINE2)       //10
                        {
//FPRINT(" > LINE2");
                            if (pHash->CurrTxRate > RATE_24M)
                            {
                                pHash->RiseConditionCount = 0;
                                pHash->DownConditionCount++;
                                goto JudgeRate;
                            }
                            else if (pHash->CurrTxRate == RATE_24M)
                            {
                                pHash->RiseConditionCount = 0;
                                pHash->DownConditionCount = 0;
                                goto JudgeRate;
                            }
                            else
                            {
                                pHash->RiseConditionCount++;
                                pHash->DownConditionCount = 0;
                                goto JudgeRate;
                            }
                        }                         // LINE2
                        else
                        {
                            if (Ratio >= LINE3)
                            {
//FPRINT(" > LINE3");
                                if (pHash->CurrTxRate > RATE_18M)
                                {
                                    pHash->RiseConditionCount = 0;
                                    pHash->DownConditionCount++;
                                    goto JudgeRate;
                                }
                                else if (pHash->CurrTxRate == RATE_18M)
                                {
                                    pHash->RiseConditionCount = 0;
                                    pHash->DownConditionCount = 0;
                                    goto JudgeRate;
                                }
                                else
                                {
                                    pHash->RiseConditionCount++;
                                    pHash->DownConditionCount = 0;
                                    goto JudgeRate;
                                }
                            }                     // LINE3
                            else
                            {
//FPRINT(" < LINE3");
                                pHash->RiseConditionCount = 0;
                                pHash->DownConditionCount++;
                                goto JudgeRate;

                            }
                        }
                    }
                }
                JudgeRate:
                if (pHash->bJustRiseRate)
                {
                    if (pHash->DownConditionCount)
                    {
                        if (pHash->CurrTxRate > 0 && PURE_A_MODE != mMacMode)
                        {
                            pHash->CurrTxRate--;
                            if(pdot11Obj->rfMode == UW2453_RF || pdot11Obj->rfMode == AR2124_RF)
                                if(pHash->CurrTxRate == 1)
                                    pHash->CurrTxRate --;
//FPRINT_V("Case 1: Down Rate, NewRate", pHash->CurrTxRate);
                        }
                        else if(pHash->CurrTxRate > 4)
                        {
                            pHash->CurrTxRate--;
                        }
                        pHash->DownConditionCount = 0;
                        pHash->bJustRiseRate = FALSE;
                    }
                    else
                    {
                        pHash->bJustRiseRate = FALSE;
                    }
                }
                else
                {
                    pHash->bJustRiseRate = 0;
                    if (pHash->RiseConditionCount >= RISE_CONDITION_THRESHOLD)
                    {
                        if (pHash->MaxRate > pHash->CurrTxRate)
                        {
                            pHash->CurrTxRate++;
                            if(pdot11Obj->rfMode == UW2453_RF || pdot11Obj->rfMode == AR2124_RF)
                                if(pHash->CurrTxRate == 1)
                                    pHash->CurrTxRate++ ;

                            pHash->bJustRiseRate = TRUE;
//FPRINT_V("Case 2: Rise Rate, NewRate", pHash->CurrTxRate);
                        }
                        pHash->DownConditionCount = 0;
                        pHash->RiseConditionCount = 0;
                    }
                    else if (pHash->DownConditionCount >= DOWN_CONDITION_THRESHOLD)
                    {
                        if (pHash->CurrTxRate > 0 && PURE_A_MODE != mMacMode)
                        {
                            pHash->CurrTxRate--;
                            if(pdot11Obj->rfMode == UW2453_RF || pdot11Obj->rfMode == AR2124_RF)
                                if(pHash->CurrTxRate == 1)
                                    pHash->CurrTxRate --;

//FPRINT_V("Case 3: Down Rate, NewRate", pHash->CurrTxRate);
                        }
                        else if(pHash->CurrTxRate > 4)
                        {
                            pHash->CurrTxRate--;
                        }

                        pHash->DownConditionCount = 0;
                        pHash->RiseConditionCount = 0;
                    }
                }
                pHash->SuccessFrames = 0;
                pHash->FailedFrames = 0;

            }                                     // end of pHash->bValid
        }                                         // end of for loop
    }
#endif
}
#endif

void zd_PerSecTimer(void)
{
	static U32 sec = 0;
    U8 oldERP, oldBARKER, newERP, newBARKER ;
	sec++;
	
#if ZDCONF_AP_SUPPORT == 1
	if (mBssType == AP_BSS)
    {
		if (sec > AGE_HASH_PERIOD){
			U32 tmpValue;
			void *reg = pdot11Obj->reg;
			
			mZyDasModeClient = FALSE;
            oldERP = pdot11Obj->ConfigFlag & NON_ERP_PRESENT_SET;
            oldBARKER = pdot11Obj->ConfigFlag & BARKER_PREAMBLE_SET;

			pdot11Obj->ConfigFlag &= ~NON_ERP_PRESENT_SET;
			pdot11Obj->ConfigFlag &= ~BARKER_PREAMBLE_SET;

			AgeHashTbl();

            newERP = pdot11Obj->ConfigFlag & NON_ERP_PRESENT_SET;
            newBARKER = pdot11Obj->ConfigFlag & BARKER_PREAMBLE_SET;

            if((newERP != oldERP) || (newBARKER != oldBARKER))
                UpdateBcn++;
			sec = 0;

#if defined(OFDM)
			if (mMacMode != PURE_B_MODE && mMacMode != PURE_A_MODE){

				if (pdot11Obj->ConfigFlag & NON_ERP_PRESENT_SET){
					//FPRINT("Enable Protection Mode");
					mErp.buf[2] |= (NON_ERP_PRESENT | USE_PROTECTION);
					pdot11Obj->ConfigFlag |= ENABLE_PROTECTION_SET;
                    if(pdot11Obj->bDeviceInSleep)
                    {
                        printk("In Sleep @ %s\n", __FUNCTION__);
                        return ;
                    }
					tmpValue = pdot11Obj->GetReg(reg, ZD_RTS_CTS_Rate); 
					tmpValue &= ~CTS_MOD_TYPE_OFDM;
					tmpValue |= CTS_RATE_11M;
					pdot11Obj->SetReg(reg, ZD_RTS_CTS_Rate, tmpValue);
					pdot11Obj->ConfigFlag &= ~SHORT_SLOT_TIME_SET;
					mCap &= ~CAP_SHORT_SLOT_TIME;	
					pdot11Obj->SetReg(reg, ZD_CWmin_CWmax, CW_NORMAL_SLOT); 
				
					if (((mCap & CAP_SHORT_PREAMBLE) == 0) || (pdot11Obj->ConfigFlag & BARKER_PREAMBLE_SET)){
						mErp.buf[2] |= BARKER_PREAMBLE;
                        if(pdot11Obj->bDeviceInSleep)
                        {
                            printk("In SLLLLLLLLLLLLLLLLLLP @ %s\n", __FUNCTION__);
                            return ;
                        }

						tmpValue = pdot11Obj->GetReg(reg, ZD_RTS_CTS_Rate); 
						tmpValue &= ~NON_BARKER_PMB_SET;
						tmpValue |= CTS_RATE_11M;
						pdot11Obj->SetReg(reg, ZD_RTS_CTS_Rate, tmpValue);
						//FPRINT("Enable Barker Preamble");
					}	
				}	 
				else{
				    //#if 0 for pure g mode testing
					//FPRINT("Disable Protection Mode");
					mErp.buf[2] &= ~(NON_ERP_PRESENT);
					pdot11Obj->ConfigFlag &= ~ENABLE_PROTECTION_SET;
			
					//FPRINT("Disable Barker Preamble");
					mErp.buf[2] &= ~(BARKER_PREAMBLE);

					pdot11Obj->ConfigFlag &= ~SHORT_SLOT_TIME_SET;
					mCap |= CAP_SHORT_SLOT_TIME;	
					pdot11Obj->SetReg(reg, ZD_CWmin_CWmax, CW_SHORT_SLOT); 
		
				}
                //UpdateBcn++;
     
			}
#endif            	
		}	
	}	
#endif //ZDCONF_AP_SUPPORT == 1
}
#undef HERE_MAX_USER
	
	
BOOLEAN zd_QueryStaTable(U8 *sta, void **ppHash)
{
	Hash_t *pHash = NULL;
	MacAddr_t *addr = (MacAddr_t*) sta;
	
	pHash = HashSearch(addr); 
	
	*ppHash = pHash;
	
	if (!pHash) 
		return FALSE;

	if (pHash->asoc == STATION_STATE_ASOC)
		return TRUE; 
	else
		return FALSE; 
}

#ifdef LINUX
U8  zd_GetBssList(bss_info_t *pBssList)
{
    U8 i;
    
	for (i=0; i < mBssCnt; i++, pBssList++){
		memcpy(pBssList->bssid, (U8 *)&mBssInfo[i].bssid, 6);
		pBssList->beaconInterval = mBssInfo[i].bcnInterval;
		pBssList->channel = mBssInfo[i].Phpm.buf[2];
		pBssList->cap = mBssInfo[i].cap;
		memcpy(pBssList->ssid, (U8 *)&mBssInfo[i].ssid, mBssInfo[i].ssid.buf[1]+2);
		//printk("ssid: %s\r\n", &mBssInfo[i].ssid.buf[2]);
		memcpy(pBssList->supRates, (U8 *)&mBssInfo[i].supRates, mBssInfo[i].supRates.buf[1]+2);
		memcpy(pBssList->extRates, (U8 *)&mBssInfo[i].extRates, mBssInfo[i].extRates.buf[1]+2);
		pBssList->atimWindow = mBssInfo[i].IbssParms.buf[2] + ((U16)(mBssInfo[i].IbssParms.buf[3]) << 8);
#if ZDCONF_SIGNAL_INFO == 1
		pBssList->signalStrength = mBssInfo[i].signalStrength;
		pBssList->signalQuality = mBssInfo[i].signalQuality;
#endif
		pBssList->apMode = mBssInfo[i].apMode;

		/* Copy WPAIe */
		memcpy(pBssList->WPAIe, (U8 *)&mBssInfo[i].WPAIe, mBssInfo[i].WPAIe[1]+2);
	    memcpy(pBssList->RSNIe, (U8 *)&mBssInfo[i].RSNIe, mBssInfo[i].RSNIe[1]+2);

#if ZDCONF_LP_SUPPORT == 1
        memcpy(pBssList->zdIE_Info_BURST, (U8 *)&mBssInfo[i].zdIE_BURST, sizeof(pBssList->zdIE_Info_BURST));
        memcpy(pBssList->zdIE_Info_AMSDU, (U8 *)&mBssInfo[i].zdIE_AMSDU, sizeof(pBssList->zdIE_Info_AMSDU));
#endif
#if ZDCONF_SES_SUPPORT == 1
        pBssList->SES_Element_Valid = mBssInfo[i].SES_Element_Valid;
        memcpy(&(pBssList->SES_Element), &(mBssInfo[i].SES_Element), sizeof(Element));
#endif
		//printk(" [zd_GetBssList] wpa ie len = %d\r\n", mBssInfo[i].WPAIe[1]+2);
	}

	return  mBssCnt;       
}            	
#endif

U16 zd_AidLookUp(U8 *addr)
{
	MacAddr_t *sta = (MacAddr_t *)addr;
	return AIdLookup(sta);
}
#if 0
void zd_UpdateIbssInfo(U8 *addr, U8 tmpMaxRate, U8 preamble, U8 erpSta)
{
	U8 MaxRate;
	MaxRate = RateConvert((tmpMaxRate & 0x7f));
	if (MaxRate > mMaxTxRate)
		MaxRate = mMaxTxRate;

	UpdateStaStatus((MacAddr_t *)addr, STATION_STATE_ASOC, 0);
	AssocInfoUpdate((MacAddr_t *)addr, MaxRate, 0, 0, preamble, erpSta, 0);
	//FPRINT_V("MaxRate", MaxRate);
	//FPRINT_V("erpSta", erpSta);
}    
#endif
#ifdef LINUX
void zd1205_DeviceReset(struct zd1205_private *macp)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0) /* tune me! */
#  define SUBMIT_URB(u,f)       usb_submit_urb(u,f)
#  define USB_ALLOC_URB(u,f)    usb_alloc_urb(u,f)
#else
#  define SUBMIT_URB(u,f)       usb_submit_urb(u)
#  define USB_ALLOC_URB(u,f)    usb_alloc_urb(u)
#endif

    extern unsigned char zd1205_hw_init(struct zd1205_private *macp);
    U32 tmp=0,i;
    struct net_device *dev = macp->device;

    clear_bit(ZD1211_RUNNING, &macp->flags);
    set_bit(ZD1211_UNPLUG, &macp->flags);
    zd1205_disable_int();
    del_timer_sync(&macp->watchdog_timer);
    del_timer_sync(&macp->tm_hking_id);
    del_timer_sync(&macp->tm_mgt_id);
    del_timer_sync(&macp->tm_scan_id);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
    usb_kill_urb(macp->rx_urb);
    usb_kill_urb(macp->tx_urb);
    usb_kill_urb(macp->ctrl_urb);
    usb_kill_urb(macp->reg_urb);
    usb_kill_urb(macp->intr_urb);
#else
    usb_unlink_urb(macp->rx_urb);
    usb_unlink_urb(macp->tx_urb);
    usb_unlink_urb(macp->ctrl_urb);
    usb_unlink_urb(macp->reg_urb);
    usb_unlink_urb(macp->intr_urb);
#endif
    flush_scheduled_work();


    usb_reset_device(macp->usb);
    mdelay(500);
    macp->bUSBDeveiceAttached=1;

    if (!zd1211_alloc_all_urbs(macp)){
        printk("Calling zd1211_alloc_all_urbs fails in %s\n", __func__);
    }

    if (zd1211_Download_IncludeFile(macp) != 0)
    {
        printk(KERN_ERR "zd1211_Download_IncludeFile failed in %s\n", __func__);
    }
    if ((usb_control_msg(macp->usb, usb_sndctrlpipe(macp->usb,0),USB_REQ_SET_CONFIGURATION,0, 1, 0, NULL, 0, HZ))<0)
    {
        //#endif
        printk(KERN_ERR "usb_set_configuration() failed in %s\n", __func__);
    }
    clear_bit(ZD1211_UNPLUG, &macp->flags);
    set_bit(ZD1211_RUNNING, &macp->flags);
    usb_fill_bulk_urb(macp->intr_urb, macp->usb,
            usb_rcvbulkpipe(macp->usb, EP_INT_IN),
            macp->IntEPBuffer, MAX_EPINT_BUFFER,
            zd1211_intr_cb, macp);
#if 0 // (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,14))
    macp->intr_urb->transfer_flags |= URB_ASYNC_UNLINK;
#endif
    if ((SUBMIT_URB(macp->intr_urb, GFP_KERNEL))){
        printk(KERN_ERR "zd1211: failed intr_urb\n");
    }
    zd1205_hw_init(macp);
    if(!zd1211_submit_rx_urb(macp))
        printk("submit rx urb failed\n");

    init_timer(&macp->tm_scan_id);
    init_timer(&macp->tm_auth_id);
    init_timer(&macp->watchdog_timer);
    macp->watchdog_timer.data = (unsigned long) dev;

    macp->watchdog_timer.function = (void *) &zd1205_watchdog_cb;

    init_timer(&macp->tm_hking_id);
    macp->tm_hking_id.data = (unsigned long) dev;
    macp->tm_hking_id.function = (void *) &HKeepingCB;

    init_timer(&macp->tm_mgt_id);
    macp->tm_mgt_id.data = (unsigned long) dev;
    macp->tm_mgt_id.function = (void *) &zd1205_mgt_mon_cb;



    mod_timer(&(macp->watchdog_timer), jiffies + (1*HZ));  //1 sec
    mod_timer(&(macp->tm_hking_id), jiffies + (1*HZ)/10);  //100 ms
    mod_timer(&(macp->tm_mgt_id), jiffies + (1*HZ)/50);   //20 ms


    HW_SetRfChannel(pdot11Obj, mRfChannel, 0, mMacMode);
    HW_SetRfChannel(pdot11Obj, mRfChannel, 1, mMacMode);
    zd_writel(0x01, AfterPNP);
    zd_writel(0x00, GPI_EN);                

    macp->intrMask = ZD1205_INT_MASK;
    macp->bHandleNonRxTxRunning = 0;


    //Recovery CAM status
    for(i=0;i<32;i++)
        if(sstByAid[i]->bValid)
            tmp = tmp | (1<<i); 
    zd_writel(tmp, 0x704); //AID0~31 valid bits
    zd_writel(0xf00, 0x708); //Default Key valid bits
    if(mBssType == AP_BSS || mBssType == INDEPENDENT_BSS)
    {
        UpdateBcn++;
        zd_CmdProcess(CMD_ENABLE, pdot11Obj, 0);  //AP start send beacon , STA start scan 
    }
    zd1205_enable_int();
    clear_bit(ZD1211_TX_BUSY, &macp->flags);





}
#endif

#if ZDCONF_ADHOC_SUPPORT == 1
void zd_UpdateIbssInfo(U8 *addr, U8 tmpMaxRate, U8 preamble, U8 erpSta)
{
	U8 MaxRate;
	MacAddr_t	*Sta;
	Hash_t		*pHash;
	MaxRate = RateConvert((tmpMaxRate & 0x7f));

	if (MaxRate > mMaxTxRate)
		MaxRate = mMaxTxRate;
	Sta = (MacAddr_t *)addr;
	UpdateStaStatus(Sta, STATION_STATE_ASOC, 0);

	pHash = HashSearch(Sta);
	if (pHash != NULL)
	{
		if (!pHash->AlreadyIn)
		{
			pHash->AlreadyIn=1;
			mCurrConnUser++;
			if (erpSta == FALSE)
			{
				mNumBOnlySta++;
			}
		}
	}
	AssocInfoUpdate((MacAddr_t *)addr, MaxRate, 0, 0, preamble, erpSta,0,0,1,0);
	//FPRINT_V("MaxRate", MaxRate);
	//FPRINT_V("erpSta", erpSta);
}
#endif
         	
#endif

