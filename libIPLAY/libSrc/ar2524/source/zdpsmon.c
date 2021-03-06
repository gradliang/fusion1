#ifndef __ZDPSMON_C__
#define __ZDPSMON_C__

#define LOCAL_DEBUG_ENABLE 0
#include "typedef.h"
#include "ndebug.h"
#include "zdmpx.h"
#include "zd80211.h"
#include "zddebug.h"

#ifdef LINUX
#define GetEntry(pMac)		(((pMac->mac[3]) ^ (pMac->mac[4]) ^ (pMac->mac[5])) & (MAX_AID-1))
#endif

Hash_t *FreeHashList;
Hash_t HashBuf[MAX_RECORD];
Hash_t *HashTbl[MAX_RECORD];
Hash_t *sstByAid[MAX_RECORD];
U32 freeHashCount;
#ifdef LINUX
extern struct net_device *g_dev;
#endif
#ifdef PLATFORM_MPIXEL
extern struct zd1205_private *macp_global;
#endif

// original user id setting for STA mode before 2.16.0.0(included)
//#define PAIRWISE_USER_ID_STA 8
//#define GROUP_USER_ID_STA    32 
// 
// fix ZD1211_TX_BUSY issue, (TxUnderRunCnt is also not zero)
#define PAIRWISE_USER_ID_STA 0
#define GROUP_USER_ID_STA    8  
extern void zd1205_config_dyn_key(u8 DynKeyMode, u8 *pkey, int idx);
Hash_t *HashInsert(MacAddr_t *pMac);
extern void zd1205_notify_disjoin_event(struct zd1205_private *macp, u8 *mac);


void CleanupHash(Hash_t *hash)
{
	memset(hash->mac, 0, 6);
	hash->asoc = STATION_STATE_DIS_ASOC;
	hash->auth = STATION_STATE_NOT_AUTH;
	hash->psm = PSMODE_STA_ACTIVE;
	hash->encryMode = WEP_NOT_USED;
	hash->ZydasMode = 0;
	hash->pkInstalled = 0;
	hash->AlreadyIn = 0;
	hash->ContSuccFrames = 0;
	hash->ttl = 0;
	hash->bValid = FALSE;
	hash->Preamble = 0;
	hash->keyLength = 0;
	hash->KeyId = 0;
	memset(hash->wepIv, 0, 4);
	memset(&hash->TxSeed, 0, sizeof(Seedvar));
	memset(&hash->RxSeed, 0, sizeof(Seedvar));
	memset(&hash->TxMicKey, 0, sizeof(MICvar));
	memset(&hash->RxMicKey, 0, sizeof(MICvar));
	hash->SuccessFrames = 0;
	hash->FailedFrames = 0;
	hash->bJustRiseRate = FALSE;
	hash->RiseConditionCount = 0;
	hash->DownConditionCount = 0;
	hash->vapId = 0;

	hash->RxBytes = 0;
	hash->TxBytes = 0;
#if defined(OFDM)
	hash->bErpSta = TRUE;
#else
	hash->bErpSta = FALSE; 
#endif	
#ifdef HOSTAPD_SUPPORT
	memset(hash->WPAIE.buf,0,sizeof(hash->WPAIE));
#endif 
#if ZDCONF_WPS_SUPPORT == 1 && ZDCONF_AP_SUPPORT == 1
	memset(hash->WSCIE.buf,0,sizeof(hash->WSCIE));
#endif	
}	


void CleanupKeyInfo(Hash_t *hash)
{
	hash->encryMode = WEP_NOT_USED;
	hash->pkInstalled = 0;
	hash->keyLength = 0;
	hash->KeyId = 0;
	memset(hash->wepIv, 0, 4);
	memset(&hash->TxSeed, 0, sizeof(Seedvar));
	memset(&hash->RxSeed, 0, sizeof(Seedvar));
	memset(&hash->TxMicKey, 0, sizeof(MICvar));
	memset(&hash->RxMicKey, 0, sizeof(MICvar));
}	


void initHashBuf(void)
{
	int i;

	freeHashCount = MAX_RECORD;

	for (i=0; i<MAX_AID; i++){ //from 0 to 31
		HashBuf[i].pNext = &HashBuf[i+1];
		sstByAid[i] = &HashBuf[i];
		HashBuf[i].aid = i;
		CleanupHash(&HashBuf[i]);
	}
	
	//aid 32 is here
	HashBuf[MAX_AID].pNext = NULL;
	sstByAid[MAX_AID] = &HashBuf[MAX_AID];
	HashBuf[MAX_AID].aid = MAX_AID;
	CleanupHash(&HashBuf[MAX_AID]);

	FreeHashList = &HashBuf[1]; //by pass aid = 0
	
	//deal with aid = 0
	HashBuf[0].pNext = NULL;
}


Hash_t *allocHashBuf(void)
{
	Hash_t *hash = NULL;
	U32 flags;	
	
	//HSDEBUG("*****allocHashBuf*****");
	flags = pdot11Obj->EnterCS();
	if (FreeHashList != NULL){
		hash = FreeHashList;
		FreeHashList = FreeHashList->pNext;
		hash->pNext = NULL;
		freeHashCount--;
	}
	pdot11Obj->ExitCS(flags);
	return hash;
}



void freeHashBuf(Hash_t *hash)
{
#ifdef LINUX
    struct zd1205_private *macp = (struct zd1205_private *)g_dev->priv;
	U32 flags;
    struct sk_buff *skb = NULL;
	
	//HSDEBUG("*****freeHashBuf*****");
	flags = pdot11Obj->EnterCS();
	if (hash->AlreadyIn){
		if (mCurrConnUser > 0)
			mCurrConnUser--;
		if (hash->bErpSta == FALSE && mNumBOnlySta > 0)
		{
			mNumBOnlySta--;
			if (mNumBOnlySta==0)
			{
				pdot11Obj->ConfigFlag &= ~NON_ERP_PRESENT_SET;
				mErp.buf[2] &= ~NON_ERP_PRESENT;
			}
		}
	}
		
	if (hash->psm == PSMODE_POWER_SAVE){
		if (mPsStaCnt > 0)
			mPsStaCnt--;
	}	

//#if defined(AMAC)
//	HW_CAM_ClearRollTbl(pdot11Obj, hash->aid);	
//#endif	
	
    while(macp->psBuf[hash->aid])
    {
        skb = macp->psBuf[hash->aid];
        macp->psBuf[hash->aid] = macp->psBuf[hash->aid]->next;
        dev_kfree_skb_any(skb);
        macp->devBuf_free++;
        macp->psBufCnt[hash->aid]--;
    }
    macp->last_psBuf[hash->aid] = NULL;
    if(macp->psBufCnt[hash->aid] < 0)
        printk("!! %s,%d\n",__func__,__LINE__);
	CleanupHash(hash);
	hash->pNext = FreeHashList;
	FreeHashList = hash;
	freeHashCount++;  
	pdot11Obj->ExitCS(flags);
#if defined(AMAC)
	HW_CAM_ClearRollTbl(pdot11Obj, hash->aid);	
#endif	
#else
    MP_ASSERT(0);
#endif
}


void InitHashTbl(void)
{
	int i;
	
	for (i=0; i<MAX_RECORD; i++){
		HashTbl[i] = NULL;
	}	
}	


Hash_t *HashSearch(MacAddr_t *pMac)
{
	U8 entry;
	Hash_t *hash = NULL;
	U32 flags;
    U16 loopCheck = 0;
	
	char buf1[32];
	char buf2[32];
//	mpDebugPrint("%s: mBssId=%s,pMac=%s", __func__,print_mac(buf1,&mBssId),print_mac(buf2,pMac));
//	mpDebugPrint("%s: mBssType=%d(%d),sstByAid[0]=%p", __func__,mBssType, INFRASTRUCTURE_BSS,sstByAid[0]);
	if (mBssType == INFRASTRUCTURE_BSS){
		if (memcmp(&mBssId, pMac, 6) == 0 ||
            (sstByAid[0] && memcmp(&sstByAid[0]->mac, pMac, 6) == 0))
		{
			MP_FUNCTION_EXIT();
			return sstByAid[0];
		}	
		else	
		{
			MP_FUNCTION_EXIT();
			return NULL;
		}
	}
#ifdef LINUX
		
	//HSDEBUG("HashSearch");
	entry = GetEntry(pMac); 
	flags = pdot11Obj->EnterCS();
	if (HashTbl[entry] == NULL) {
		goto exit;
	}	
	else{
		hash = HashTbl[entry];
		do {
            //to prevent hash->pNext equals to its self
            if(loopCheck++ > 100)
            {
                printk("infinite loop occurs in %s\n", __FUNCTION__);
                loopCheck = 0;
                break;
            }

            if ((hash->bValid) && (memcmp(hash->mac, (U8 *)pMac, 6) == 0)){
				//HSDEBUG("Search got one");
				goto exit;
			}	
			else
				hash = hash->pNext;

		}while(hash != NULL);
	}	
	
exit:
	pdot11Obj->ExitCS(flags);		 
	if (hash){
#if 0		
		printf("macaddr = %02x:%02x:%02x:%02x:%02x:%02x\n", 
			hash->mac[0],  hash->mac[1], hash->mac[2], 
			hash->mac[3], hash->mac[4], hash->mac[5]);
		printf("asoc = %x\n", hash->asoc);	
		printf("auth = %x\n", hash->auth);
		printf("psm = %x\n", hash->psm);
		printf("aid = %x\n", hash->aid);
		printf("lsInterval = %x\n", hash->lsInterval);
#endif		
	}	
	else
		;//HSDEBUG("Search no one");
#endif
		
	MP_FUNCTION_EXIT();
	return hash; 
}





	
Hash_t *HashInsert(MacAddr_t *pMac)
{
	U8 entry;
	Hash_t *hash;
	U32 flags;
	
	HSDEBUG("HashInsert");
	
	if (mBssType == INFRASTRUCTURE_BSS){
		hash = sstByAid[0];
		memcpy(hash->mac, (U8 *)pMac, 6);
		hash->ttl = HW_GetNow(pdot11Obj);
		hash->bValid = TRUE;
		return hash;		
	}
	
#ifdef LINUX
	hash = allocHashBuf();
	if (!hash){
		HSDEBUG("No free one");
		//Age Hash table
		AgeHashTbl();
		return NULL; // no free one
	}	
	else{
		entry = GetEntry(pMac);
		HSDEBUG_V("entry", entry);

		if (HashTbl[entry] == NULL){ //entry is null
			HashTbl[entry] = hash;
			HSDEBUG("Entry is null");
		}
		else{ //insert list head
			flags = pdot11Obj->EnterCS();
			hash->pNext = HashTbl[entry];
			HashTbl[entry] = hash;
			pdot11Obj->ExitCS(flags);	
			HSDEBUG("Insert to list head");
		}
		
		memcpy(hash->mac, (U8 *)pMac, 6);
		hash->ttl = HW_GetNow(pdot11Obj);
		hash->bValid = TRUE;
		return hash;	
	}	
#endif
}	


#ifdef LINUX
BOOLEAN AgeHashTbl(void)
{
    struct zd1205_private *macp = (struct zd1205_private *)g_dev->priv;
	BOOLEAN ret = FALSE;
#if ZDCONF_AP_SUPPORT == 1  
	U32 now, ttl, idleTime;
	U8 entry, firstLayer;
    U16 loopCheck = 0;

	int i;
	MacAddr_t *pMac;
	Hash_t *hash, *preHash = NULL;
	
	HSDEBUG("*****AgeHashTbl*****");
	now = HW_GetNow(pdot11Obj);
	
	for (i=1; i<(MAX_AID+1); i++){
		ttl = sstByAid[i]->ttl;
		if (now >= ttl)
			idleTime = now - ttl;
		else
        {
            idleTime =  0;
            sstByAid[i]->ttl = now;
        }


		
		if (sstByAid[i]->bValid){
			if (idleTime > IDLE_TIMEOUT ){
				printk("*****Age one*****\n");
				printk("aid:%d\n", i);
				printk("now:%d\n", now);
				printk("ttl:%d\n", ttl);
				printk("idleTime:%d\n", idleTime);
			
				pMac = (MacAddr_t *)&sstByAid[i]->mac[0];
				entry = GetEntry(pMac);
				HSDEBUG_V("entry", entry);
				hash = HashTbl[entry];
				firstLayer = 1;
				do {
                    // For AP only
                    if(loopCheck++ > 100)
                    {
                        printk("infinite loop occurs in %s\n", __FUNCTION__);
                        loopCheck = 0;
                        break;
                    }

                    if (hash == sstByAid[i]){
                        if (firstLayer == 1){
                            HSDEBUG("*****firstLayer*****");
							if (hash->pNext != NULL)
								HashTbl[entry] = hash->pNext;
							else
								HashTbl[entry] = NULL;
						}			
						else{
							HSDEBUG("*****Not firstLayer*****");
							preHash->pNext = hash->pNext;
						}
                        zd1205_notify_disjoin_event(macp, hash->mac);
						zd_CmdProcess(CMD_DISASOC, &hash->mac[0], ZD_INACTIVITY);
						freeHashBuf(hash);
                        return TRUE;//We age one at once
						break;
					}	
					else{
						preHash = hash;
						hash = hash->pNext;
						firstLayer = 0;
					}	
				}while(hash != NULL);
				ret = TRUE;
			}
			else {
				if (sstByAid[i]->ZydasMode == 1)
					mZyDasModeClient = TRUE;
					
				if (sstByAid[i]->bErpSta == FALSE && mMacMode != PURE_A_MODE){	
                    pdot11Obj->ConfigFlag |= NON_ERP_PRESENT_SET;
					pdot11Obj->ConfigFlag |= ENABLE_PROTECTION_SET;
					if (sstByAid[i]->Preamble == 0){ //long preamble
						pdot11Obj->ConfigFlag |= BARKER_PREAMBLE_SET;
					}
				}	
			}		
		}		
	
	}
	
	//HSDEBUG_V("ret", ret);
#endif
	return ret;
}
#endif
	

void ResetPSMonitor(void)
{
	ZDEBUG("ResetPSMonitor");
	initHashBuf();
	InitHashTbl();
	mPsStaCnt = 0;
}


Hash_t *RxInfoIndicate(MacAddr_t *sta, PsMode psm, U8 rate)
{
	Hash_t *pHash;
    StationState asoc = 0;
    PsMode oldPsm = 0;
	
	ZDEBUG("RxInfoIndicate");
    
	//if (isGroup(sta))
		//return NULL;

	pHash = HashSearch(sta);
	if (!pHash){
#if ZDCONF_PSEUDO_SUPPORT == 1 
        if (mBssType == PSEUDO_IBSS){
            pHash = HashInsert(sta);
            if (!pHash)
				return NULL;
            else{
                pHash->asoc = STATION_STATE_ASOC;
                zd1205_dump_data(" HashInsert macAddr = ", (U8 *)&pHash->mac[0], 6);
                goto updateInfo;
            }       
        }
        else        
#endif
		    return NULL;
    }    	
	else{
        oldPsm  = pHash->psm;
        asoc = pHash->asoc;

updateInfo:		
		if (rate > pHash->MaxRate)
			pHash->MaxRate = rate;
            
		pHash->RxRate = rate;
		pHash->ttl = HW_GetNow(pdot11Obj);
		
#if ZDCONF_AP_SUPPORT == 1
		if (mBssType == AP_BSS)
        {
			if (psm == PSMODE_STA_ACTIVE){
				if (oldPsm == PSMODE_POWER_SAVE){
					StaWakeup(sta);
					if (asoc == STATION_STATE_ASOC){
						if (mPsStaCnt >0){ 
							mPsStaCnt--;
						}	
					}
				}		
			}
			else {
				if (oldPsm == PSMODE_STA_ACTIVE){
					if (asoc == STATION_STATE_ASOC){
						if (mPsStaCnt < MAX_AID){ 
							mPsStaCnt++;
						}	
					}	
				}
				else if (oldPsm == PSMODE_POWER_SAVE){
					if (asoc == STATION_STATE_ASOC){
						if (mPsStaCnt == 0) 
							mPsStaCnt++;
					}	
				}		
			}	
		}	
#endif
		
		pHash->psm = psm;
	}
	
	return pHash;
}

#if ZDCONF_AP_SUPPORT == 1
void RxInfoUpdate(Hash_t *pHash, PsMode psm, U8 rate)
{
	PsMode oldPsm = pHash->psm;
	StationState asoc = pHash->asoc;
		
	if (rate > pHash->MaxRate)
		pHash->MaxRate = rate;
			
	pHash->RxRate = rate;
	pHash->ttl = HW_GetNow(pdot11Obj);
		
	if (psm == PSMODE_STA_ACTIVE){
		if (oldPsm == PSMODE_POWER_SAVE){
			StaWakeup((MacAddr_t *)pHash->mac);
			if (asoc == STATION_STATE_ASOC){
				if (mPsStaCnt >0){ 
					mPsStaCnt--;
				}	
			}
		}		
	}
	else {
		if (oldPsm == PSMODE_STA_ACTIVE){
			if (asoc == STATION_STATE_ASOC){
				if (mPsStaCnt < MAX_AID){ 
					mPsStaCnt++;
				}	
			}	
		}
		else if (oldPsm == PSMODE_POWER_SAVE){
			if (asoc == STATION_STATE_ASOC){
				if (mPsStaCnt == 0) 
					mPsStaCnt++;
			}	

		}		
	}	

	
	pHash->psm = psm;
}
#endif


BOOLEAN UpdateStaStatus(MacAddr_t *sta, StationState staSte, U8 vapId)
{
#ifdef LINUX
    struct zd1205_private *macp = (struct zd1205_private *)g_dev->priv;
#else
    struct zd1205_private *macp = macp_global;
#endif
	Hash_t *pHash;
    U16 loopCheck = 0;
	
	ZDEBUG("UpdateStaStatus");
	
#if ZDCONF_AP_SUPPORT == 1
	if (mBssType == AP_BSS){
		pHash = HashSearch(sta);
		if (pHash)
			goto UpdateStatus;
		else{	
			if ((STATION_STATE_AUTH_OPEN == staSte) || (STATION_STATE_AUTH_KEY == staSte)){
				if ((mCurrConnUser + 1) > mLimitedUser){
					//AgeHashTbl();
                    printk("Reject Auth Due to (mCurrConnUser + 1) > mLimitedUser\n");
					return FALSE;
				}	
				else{	
					pHash = HashInsert(sta);
					if (!pHash)
                    {
                        printk("Reject Auth Due to HashInsert fail\n");
						return FALSE; 
                    }
                    goto UpdateStatus;
                }
			}	
			else
            {
                printk("Reject Auth Due to %s,%d. staSte=%d\n",__FILE__, __LINE__,staSte);
				return FALSE; 
            }
		}
	}
#endif
	if (mBssType == INFRASTRUCTURE_BSS){
		if ((STATION_STATE_AUTH_OPEN == staSte) || (STATION_STATE_AUTH_KEY == staSte)){
            if (mWPAIe[1] != 0)
            {
                HW_CAM_SetMAC(pdot11Obj, PAIRWISE_USER_ID_STA, (U8*) sta);
                HW_CAM_SetMAC(pdot11Obj, GROUP_USER_ID_STA, (U8*) &dot11BCAddress);
                WPADEBUG("-----State:%d mWPAIeLen(SetMAC)=%d\n", staSte, mWPAIe[1]);
                
            }
            else
            {
                WPADEBUG("-----State:%d mWPAIeLen(NonWPA)=%d\n", staSte, mWPAIe[1]);
            }
			CleanupHash(sstByAid[0]);
			pHash = HashInsert(sta);
		} else {	
			pHash = sstByAid[0]; //use aid = 0 to store AP's info
		}	
	}	
#if ZDCONF_ADHOC_SUPPORT == 1
	else if (mBssType == INDEPENDENT_BSS){	
		pHash = HashSearch(sta);
		if (pHash)
			goto UpdateStatus;
		else {
			pHash = HashInsert(sta);
            if (!pHash)
				return FALSE;
            else
                zd1205_dump_data(" HashInsert macAddr = ", (U8 *)&pHash->mac[0], 6);
		}	
	}
#endif
    else
    {
        printk("Reject Auth Due to %s,%d, mBssType=%d\n",__FILE__, __LINE__, mBssType);
        return FALSE;	
    }

UpdateStatus:	
	switch(staSte){
		case STATION_STATE_AUTH_OPEN:
		case STATION_STATE_AUTH_KEY:
			pHash->auth = staSte;
			break;

		case STATION_STATE_ASOC:
#if ZDCONF_AP_SUPPORT == 1
			if (mBssType == AP_BSS)
            {
				if (((mCurrConnUser + 1) > mLimitedUser) && (!pHash->AlreadyIn)){
                    printk("Reject Auth Due to %s,%d\n",__FILE__, __LINE__);
					return FALSE; 
				}
				
				if (pHash->psm == PSMODE_POWER_SAVE){
					if (mPsStaCnt > 0){ 
						mPsStaCnt--;
					}	

				}	
						
				pHash->asoc = STATION_STATE_ASOC;
				/*if (!pHash->AlreadyIn){
					pHash->AlreadyIn = 1;
					mCurrConnUser++;
				}*/
			}
            else
#endif
            {
				pHash->asoc = STATION_STATE_ASOC;
			}		

            if (mBssType != INDEPENDENT_BSS)
            {
                if((mBssType == INFRASTRUCTURE_BSS) && pHash->pkInstalled)
                {
                   zd1205_notify_disjoin_event(macp,NULL);	// TODO
                   printk("WPA Stuck occur @ %lu\n", jiffies);
                }
			    CleanupKeyInfo(pHash);
            }

            memcpy(&pdot11Obj->CurrSsid[0], (U8 *)&mSsid, mSsid.buf[1]+2);   
			break;

		case STATION_STATE_NOT_AUTH:
		case STATION_STATE_DIS_ASOC:
#if ZDCONF_AP_SUPPORT == 1
			if (mBssType == AP_BSS)
            {
				if (pHash->asoc == STATION_STATE_ASOC){
					if (pHash->psm == PSMODE_POWER_SAVE){
						FlushQ(pPsQ[pHash->aid]);
						if (mPsStaCnt > 0){ 
							mPsStaCnt--;
							if (mPsStaCnt == 0){
								FlushQ(pAwakeQ);
								FlushQ(pPsQ[0]);
							}	
						}	
					}
					/*if (pHash->AlreadyIn){
						pHash->AlreadyIn = 0;
						mCurrConnUser--;	
					}*/	
				}
			}			
#endif
			
			pHash->auth = STATION_STATE_NOT_AUTH;
			pHash->asoc = STATION_STATE_DIS_ASOC;
#ifdef LINUX
			CleanupKeyInfo(pHash);
			//for Rx-Retry filter
			HW_CAM_ClearRollTbl(pdot11Obj, pHash->aid);
#endif

//#ifdef PLATFORM_MPIXEL
//			CleanupHash(pHash);
//#endif

#ifdef LINUX
			{
				MacAddr_t	*pMac;
				Hash_t 		*sta_info;
				U8 			entry;
				pMac = (MacAddr_t *) pHash->mac;
				entry = GetEntry(pMac);
				sta_info=HashTbl[entry];
				if (sta_info)
				{
					if (memcmp(sta_info->mac, pHash->mac, 6)==0)
					{
						HashTbl[entry]=sta_info->pNext;
						freeHashBuf(pHash);
					}
					else
					{
						while (sta_info->pNext != NULL && memcmp(sta_info->pNext->mac, pHash->mac, 6) != 0)
                        {
                            // To prevent self-link
                            if(loopCheck++ > 100)
                            {
                                printk("infinite loop occurs in %s\n", __FUNCTION__);
                                loopCheck = 0;
                                break;
                            }

                            sta_info = sta_info->pNext;
                        }
                        if (sta_info->pNext != NULL)
                        {
                            Hash_t	*sta_info1;
                            sta_info1 = sta_info->pNext;
                            sta_info->pNext =  sta_info->pNext->pNext;
                            freeHashBuf(sta_info1);	
                        }
                        else
                        {
                            printk(KERN_DEBUG "Could not remove STA:" MACSTR "\n", MAC2STR(pHash->mac));
                        }
					}
				}
			}
#endif

			break;

	}
	
	return TRUE;
}


void SsInquiry(MacAddr_t *sta, StationState *sst, StationState *asst)
{
	ZDEBUG("SsInquiry");
	if (isGroup(sta)){
		*asst = STATION_STATE_NOT_AUTH;
		*sst = STATION_STATE_DIS_ASOC;
	}
	else{
		Hash_t *pHash;
		pHash = HashSearch(sta);

		if (!pHash){
			*asst = STATION_STATE_NOT_AUTH;
			*sst = STATION_STATE_DIS_ASOC;
		}
		else{
			*asst = pHash->auth;
			if ((*asst == STATION_STATE_AUTH_OPEN) || (*asst == STATION_STATE_AUTH_KEY))
				*sst = pHash->asoc;
			else
				*sst = STATION_STATE_DIS_ASOC;
		}	
	}

}


U16 AIdLookup(MacAddr_t *sta)
{
	Hash_t *pHash;
	
	ZDEBUG("AIdLookup");
	pHash = HashSearch(sta);
	if (!pHash)
		return (U16)0;
	else
		return pHash->aid;
}


void AssocInfoUpdate(MacAddr_t *sta, U8 MaxRate, U8 lsInterval, U8 ZydasMode, U8 Preamble, BOOLEAN bErpSta, U8 Burst, U8 AMSDU, U8 AMSDU_LEN, U8 vapId)

{
	Hash_t *pHash;
	
	ZDEBUG("AssocInfoUpdate");
	if (isGroup(sta))
		return;

	pHash = HashSearch(sta);
	if (!pHash)
		return;	
	else{
		pHash->MaxRate = MaxRate;
		pHash->CurrTxRate = MaxRate;
		pHash->lsInterval = lsInterval;
		pHash->ZydasMode = ZydasMode;
		pHash->Preamble = Preamble;
		pHash->bErpSta = bErpSta;
		pHash->vapId = vapId;
#if ZDCONF_LP_SUPPORT == 1
        pHash->Turbo_Burst = Burst;
        pHash->Turbo_AMSDU = AMSDU;
        pHash->Turbo_AMSDU_LEN = AMSDU_LEN;
#endif


	}
}
int zd_SetKeyInfo(U8 *addr, U8 encryMode, U8 keyLength, U8 key_id, U8 *pKeyContent)
{
    Hash_t *pHash;
    MacAddr_t *sta = (MacAddr_t *)addr;
    U32 EncWord;

    U8 ZeroAddr[6]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00};    
    U16 aid;
    U8 change_enc = 0;
    U8 bTxKey = key_id & BIT_7;
    U8 bClrRollTable = key_id & BIT_6;
    U8 KeyId = key_id & 0xF;

    switch (encryMode)
    {
    case WEP64:
        keyLength = 5;
        break;
    case WEP128:
        keyLength = 13;
        break;
    case WEP256:
        keyLength = 29;
        break;
    case TKIP:
        keyLength = 32;
        break;
    case AES:
        keyLength = 16;
        break;
    default:
#ifdef LINUX
        return 0;
#else
        break;
#endif
    }   
    if (isGroup(sta))
    {
        change_enc = 1;
        if (keyLength == 0)
        { // No entry chance.
            WPADEBUG("Clear Group key RollTbl (aid0)\n");
#ifdef PLATFORM_MPIXEL
            HW_CAM_Write(pdot11Obj, DEFAULT_ENCRY_TYPE, NO_WEP);
#endif
//            HW_CAM_ClearRollTbl(pdot11Obj, 0);//Clear group key.(aid0)
            HW_CAM_ClearRollTbl(pdot11Obj, 8);//Clear group key.(aid0)
            return 0;
        }   
        if (mWpaBcKeyLen == keyLength && mGkInstalled == 1)
            change_enc = 0; // Nonfirst time group key update.
        mWpaBcKeyLen = keyLength;
        mBcKeyId = KeyId;
        mGkInstalled = 1;

        if (encryMode == WEP64 || encryMode == WEP128 || encryMode == WEP256)
        {
            if (mOperationMode != CAM_AP_VAP)
            {
                if (0)
                {
                    HW_ConfigDynaKey(pdot11Obj, 32, (U8*)&dot11BCAddress, pKeyContent, keyLength, encryMode, change_enc);
                }
                else if (bTxKey)
                {
                    if (bClrRollTable)
                        HW_CAM_ResetRollTbl(pdot11Obj);// Reset all.
                    mKeyId = KeyId;
                }
                // Also set default key for Multicast case to avoid Tx-underrun.
                HW_CAM_Write(pdot11Obj, DEFAULT_ENCRY_TYPE, encryMode);
                HW_ConfigStatKey(pdot11Obj, pKeyContent, keyLength, STA_KEY_START_ADDR+(KeyId * 8)); 
#ifdef PLATFORM_MPIXEL
                char key[32];
                short i,j;
                memset(key,0,sizeof key);
                for (i=0, j=0; i<4; i++, j+=8) //one key occupy 32 bytes space
                {
                    if (i==0)
                        continue;
                    HW_ConfigStatKey(pdot11Obj, pKeyContent, keyLength, STA_KEY_START_ADDR+j);
                }
#endif
            }
            else
                HW_ConfigDynaKey(pdot11Obj, CAM_VAP_START_AID, (U8 *)&dot11MacAddress, pKeyContent, keyLength, encryMode,change_enc);
            
            return 0;
        }
        else if (encryMode == TKIP)
        {
            if (mWpaBcKeyLen == 32)
            {
                if (mOperationMode != CAM_AP_VAP)
                {
                    EncWord = encryMode;
                    HW_CAM_Write(pdot11Obj, DEFAULT_ENCRY_TYPE, EncWord);
                    HW_ConfigStatKey(pdot11Obj, pKeyContent, 16, STA_KEY_START_ADDR+(KeyId * 8)); // ZD1211B use software to calculate MIC, no need to setup MIC key into the CAM.

                }
                else
                {
                    HW_ConfigDynaKey(pdot11Obj, CAM_VAP_START_AID, (U8 *)&dot11MacAddress, pKeyContent, keyLength, encryMode, change_enc);
                }   
                printk("install key ,keyId=%d\n",KeyId);
                mWpaBcKeyId = KeyId;
                if (mBssType == INFRASTRUCTURE_BSS)
                    MICsetKey(&pKeyContent[24], &mBcMicKey[KeyId]); //Set Tx Mic key
                else
                    MICsetKey(&pKeyContent[16], &mBcMicKey[KeyId]);// For Infra-STA mode.
            }
            return 0;
        }
        else if (encryMode == AES)
        {
            if (mWpaBcKeyLen == 16)
            {   
                if (mOperationMode != CAM_AP_VAP)
                {
                    mWpaBcKeyId = KeyId;
                    EncWord = encryMode;
                    HW_CAM_Write(pdot11Obj, DEFAULT_ENCRY_TYPE, EncWord);
                    HW_ConfigStatKey(pdot11Obj, pKeyContent, keyLength, STA_KEY_START_ADDR + (KeyId * 8));
                }
                else
                {
                    HW_ConfigDynaKey(pdot11Obj, CAM_VAP_START_AID, (U8 *)&dot11MacAddress, pKeyContent, keyLength, encryMode, change_enc);
                }
            }       
            return 0;
        }
        else{
            return -1;
        }   
    }// End of Group key setting.   

    // Start of Pairwise key setting.
    pHash = HashSearch(sta);
    if (!pHash)
    {
		mpDebugPrint("%s: hash not found", __func__);
        if (!memcmp(&sta->mac[0], ZeroAddr, 6))
        {
            int i;
            HW_CAM_ResetRollTbl(pdot11Obj);
            if (mGkInstalled)
            {
                HW_CAM_UpdateRollTbl(pdot11Obj,0);//ReEnable group key. 
            }
            if (mBssType != INFRASTRUCTURE_BSS)
            {//AP mode.
                WPADEBUG("clear all tx key\n");
                for (i=0; i<MAX_RECORD; i++)
                    HashBuf[i].pkInstalled=0;
            }
            else
            {// STA mode.
                WPADEBUG("clear key of aid %d\n",sstByAid[0]->aid);
                sstByAid[0]->pkInstalled=0;
            }
        }
        return -1;  
    }
    else{
        pHash->keyLength = keyLength;
        if (pHash->encryMode != encryMode)
            change_enc = 1;
        pHash->encryMode = encryMode;
        aid = pHash->aid;
        
        if (encryMode != NO_WEP)
            WPADEBUG("********* Set key%s for aid:%d\n",DbgStrEncryType[encryMode & 7],aid);    
        else
            WPADEBUG("********* Clear key for aid:%d\n",aid);   
        if (encryMode == NO_WEP)
        {// Clear pairwise key
            pHash->pkInstalled = 0;
            if (mBssType == INFRASTRUCTURE_BSS)
            {
//                HW_CAM_ClearRollTbl(pdot11Obj, 8); /* XXX */
                HW_CAM_ClearRollTbl(pdot11Obj, 0); /* XXX */
            }
            else
                HW_CAM_ClearRollTbl(pdot11Obj, aid);
        }
        else if (encryMode == TKIP)
        {
            if (mBssType == INFRASTRUCTURE_BSS)
            {   
                // To improve the performence of install PTK, use the following 3 lines instead of using HW_ConfigDynaKey(..)
                HW_CAM_Write(pdot11Obj, ENCRY_TYPE_START_ADDR, encryMode);
				HW_CAM_SetKey(pdot11Obj, PAIRWISE_USER_ID_STA, 16, pKeyContent);
                pdot11Obj->SetReg(pdot11Obj->reg, ZD_CAM_ROLL_TB_LOW, 1);
                //HW_ConfigDynaKey(pdot11Obj, PAIRWISE_USER_ID_STA, addr, pKeyContent, 32, encryMode, change_enc);
            }
            else
                HW_ConfigDynaKey(pdot11Obj, aid, addr, pKeyContent, 32, encryMode, change_enc);
            
			MP_TRACE_LINE();
            MICsetKey(&pKeyContent[16], &pHash->TxMicKey);
            MICsetKey(&pKeyContent[24], &pHash->RxMicKey);
            pHash->KeyId = KeyId;
            pHash->pkInstalled = 1;
        }   
        else //if (encryMode == AES)
        {
            if (mBssType == INFRASTRUCTURE_BSS)
            {
                // To improve the performence of install PTK, use the following 3 lines instead of using HW_ConfigDynaKey(..)
                HW_CAM_Write(pdot11Obj, ENCRY_TYPE_START_ADDR, encryMode);
				HW_CAM_SetKey(pdot11Obj, PAIRWISE_USER_ID_STA, 16, pKeyContent);
                pdot11Obj->SetReg(pdot11Obj->reg, ZD_CAM_ROLL_TB_LOW, 1);
                //HW_ConfigDynaKey(pdot11Obj, PAIRWISE_USER_ID_STA, addr, pKeyContent, keyLength, encryMode, change_enc);
            }
            else
                HW_ConfigDynaKey(pdot11Obj, aid, addr, pKeyContent, keyLength, encryMode, change_enc);
            pHash->KeyId = KeyId;
            pHash->pkInstalled = 1;
        }   
        return 0;   
    }

}	

BOOLEAN zd_GetKeyInfo(U8 *addr, U8 *encryMode, U8 *keyLength, U8 *pKeyContent)
{
	Hash_t *pHash;
	MacAddr_t *sta = (MacAddr_t *)addr;
	
	ZDEBUG("zd_GetKeyInfo");
	if (isGroup(sta)){
		return FALSE; 
	}	
			
	pHash = HashSearch(sta);
	if (!pHash){
		*encryMode = 0;
		*keyLength = 0;
		return FALSE; 
	}	
	else{
		*encryMode = pHash->encryMode;
		*keyLength = pHash->keyLength;
		memcpy(pKeyContent, &pHash->keyContent[0], pHash->keyLength);
		return TRUE;
	}	
}			

/**
 * zd_SetKeyContext - Set Key context to CAM (used for WPA/WPA2)
 * @addr: MAC address of AP we associated with
 * @encryMode: Encryption mode
 * @keyLength: Length of key context
 * @keyId: Key index
 * @pKeyContent: Context of key
 */
#if 0

int zd_SetKeyContext(U8 *addr, U8 encryMode, U8 keyLength, U8 KeyId, U8 *pKeyContent)
{
	Hash_t *pHash;

	if (isGroup(addr)) {
		mWpaBcKeyLen = keyLength;
		mWpaBcKeyId = KeyId;
		
		if (encryMode == DYN_KEY_TKIP) {
			if (keyLength == 32) {
				zd1205_config_dyn_key(encryMode, pKeyContent, KeyId);
				MICsetKey(&pKeyContent[24], &mBcMicKey);
			}

			mGkInstalled = 1;
			return 0;
		}
		else if (encryMode == DYN_KEY_AES) {
                        printk(KERN_ERR "***** set group key ID: %d\n",KeyId);
			zd1205_config_dyn_key(encryMode, pKeyContent, KeyId);
			mGkInstalled = 1;
			return 0;
		}
		else {
			WPADEBUG("zd_SetKeyContext: encryMode: %d not support\n", encryMode);
			return -1;
		}	
	
	}

	pHash = HashSearch((MacAddr_t*)addr);

	if(!pHash) {
		WPADEBUG("Can't find AP's MAC address in the hash table\n");
		return -1;
	}
	else {
		pHash->encryMode = encryMode;

		if (encryMode == DYN_KEY_TKIP) {
			zd1205_config_dyn_key(encryMode, pKeyContent, KeyId);

			MICsetKey(&pKeyContent[16], &pHash->TxMicKey);
			MICsetKey(&pKeyContent[24], &pHash->RxMicKey);
			pHash->KeyId = KeyId;
			pHash->pkInstalled = 1;
		}
		else if (encryMode == DYN_KEY_AES) {
			zd1205_config_dyn_key(encryMode, pKeyContent, KeyId);
			pHash->KeyId = KeyId;
			pHash->pkInstalled = 1;
		}

		else {
			WPADEBUG("zd_SetKeyContext: encryMode: %d not support\n", encryMode);
		}
	}

	return 0;
}
#endif

#if defined(PHY_1202)
int zd_GetKeyInfo_ext(U8 *addr, U8 *encryMode, U8 *keyLength, U8 *pKeyContent, U16 iv16, U32 iv32)
{
	Hash_t *pHash;
	MacAddr_t *sta = (MacAddr_t *)addr;
	
	ZDEBUG("zd_GetKeyInfo_ext");
	if (isGroup(sta)){
		return -1; 
	}	
	
	if (mDynKeyMode != DYN_KEY_TKIP)
		return -1;
				
	pHash = HashSearch(sta);
	if (!pHash){
		*encryMode = 0;
		*keyLength = 0;
		return -1; 
	}	
	else{
		if (pHash->pkInstalled == 0)
			return -2;
			
		if ((iv16 == pHash->RxSeed.IV16) && (iv32 == pHash->RxSeed.IV32)){
			// iv out of sequence
			//FPRINT_V("iv16", iv16);
			//FPRINT_V("iv32", iv32);
			//return -3;
		}
		
		*encryMode = pHash->encryMode;
		*keyLength = pHash->keyLength;
		//do key mixing	
		Tkip_phase1_key_mix(iv32, &pHash->RxSeed);
		Tkip_phase2_key_mix(iv16, &pHash->RxSeed);
		Tkip_getseeds(iv16, pKeyContent, &pHash->RxSeed);	
		pHash->RxSeed.IV16 = iv16;
		pHash->RxSeed.IV32 = iv32;
		return pHash->aid;
	}	
}			


int zd_SetTsc(U8 *addr, U8 KeyId, U8 direction, U32 tscHigh, U16 tscLow)
{
	Hash_t *pHash;
	MacAddr_t *sta = (MacAddr_t *)addr;
	
	ZDEBUG("zd_SetTsc");
	if (isGroup(sta)){
		return -1;
	}	

	pHash = HashSearch(sta);
	if (!pHash)
		return -1;	
	else{
		pHash->KeyId = KeyId;
		if (direction == 0){ //Tx
			pHash->TxSeed.IV16 = tscLow;
			pHash->TxSeed.IV32 = tscHigh;
		}	
		else if (direction == 1){ //Rx
			pHash->RxSeed.IV16 = tscLow;
			pHash->RxSeed.IV32 = tscHigh;
		}	
		return 0;	
	}
}	


int zd_GetTsc(U8 *addr, U8 KeyId, U8 direction, U32 *tscHigh, U16 *tscLow)
{
	Hash_t *pHash;
	MacAddr_t *sta = (MacAddr_t *)addr;
	
	ZDEBUG("zd_GetTsc");
	if (isGroup(sta)){
		return -1;
	}	

	pHash = HashSearch(sta);
	if (!pHash)
		return -1;	
	else{
		if (direction == 0){ //Tx
			*tscLow = pHash->TxSeed.IV16;
			*tscHigh = pHash->TxSeed.IV32;
		}	
		else if (direction == 1){ //Rx
			*tscLow = pHash->RxSeed.IV16;
			*tscHigh = pHash->RxSeed.IV32;
		}	
		return 0;	
	}
}	
#endif


BOOLEAN zd_CheckIvSeq(U8 aid, U16 iv16, U32 iv32)
{
	Hash_t *pHash = NULL;
	U16 oldIv16;
	U32 oldIv32;

	
	ZDEBUG("zd_CheckIvSeq");
	
	if (mDynKeyMode != DYN_KEY_TKIP){
		FPRINT("Not in DYN_KEY_TKIP mode");
		return FALSE;
	}	
				
	pHash = sstByAid[aid];
	if (!pHash){
		FPRINT("zd_CheckIvSeq failed");
		return FALSE;
	}	
	else{
		if (pHash->pkInstalled == 0){
			FPRINT("pkInstalled == 0");
			return FALSE;
		}	
		
		oldIv16 = pHash->RxSeed.IV16;
		oldIv32 = pHash->RxSeed.IV32;
	
#if 1	
		if ((oldIv16 == iv16) && (oldIv32 == iv32)){
			// iv out of sequence
				FPRINT("iv out of sequence");
				FPRINT_V("iv16", iv16);
				FPRINT_V("iv32", iv32);
				return FALSE;
		}

#else //If fifo overrun, this will failed		
		if (iv32 == oldIv32){
			if (iv16 != oldIv16+1){
				// iv out of sequence
				FPRINT("iv out of sequence");
				FPRINT_V("iv16", iv16);
				FPRINT_V("iv32", iv32);
				return FALSE;
			}	
		}
		else {
			if ((iv16 != 0) || (oldIv16 != 0xffff)){
				// iv out of sequence
				FPRINT("iv out of sequence");
				FPRINT_V("iv16", iv16);
				FPRINT_V("iv32", iv32);
				return FALSE;
			}	
		}
#endif	
					
		pHash->RxSeed.IV16 = iv16;
		pHash->RxSeed.IV32 = iv32;
		return TRUE;
	}
}

#ifdef PLATFORM_MPIXEL
int mp_HwKeyInstalled(U8 *addr)
{
	Hash_t *pHash = sstByAid[0];

    if (!pHash)
        return FALSE;
    return pHash->pkInstalled ? TRUE : FALSE;
}
#endif

#endif

