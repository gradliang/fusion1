#ifndef __ZDGLOBAL_C__
#define __ZDGLOBAL_C__

#include "typedef.h"
//#include "zd80211.h"
#include "zddebug.h"

u8 		*DbgStrEncryType[]={"NOWEP","WEP64","TKIP","NA3","AES","WEP128","WEP256", "NA7"};
u8 		*DbgStrDynKeyMode[]={"NOWEP","WEP64","WEP128","NA3","TKIP","AES","NA6","NA7"};

BOOLEAN		mAssoc = FALSE;
U8		mBcKeyLen;
U8		mBcKeyId;
U8		mWpaBcKeyLen = 0;
U8      mWPAIe[128];                            /* own WPA IE */
U8		mCounterMeasureState=0;
U8		mKeyId = 0;                             /* tx key id */
U8		mKeyFormat = WEP64_USED;
BOOLEAN 	mPrivacyInvoked = FALSE;
U16 		mCap = CAP_ESS;				
U8		mKeyVector[4][16];
U8		mWepKeyLen;
MICvar		mBcMicKey[4];
U8		mBssType = INFRASTRUCTURE_BSS;
U8		mOperationMode;
Element		mSsid;				
MacAddr_t	mBssId;				
U32		mDebugFlag = 0;
U8		mDynKeyMode = 0;
U8		mGkInstalled = 0;
U8	 	mPsStaCnt = 0;	//Station count for associated and in power save mode
MacAddr_t	dot11BCAddress = {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};
U8		mPreambleType = LONG_PREAMBLE;
U8		mWpaBcKeyId = 1;
MacAddr_t	dot11MacAddress = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
U8 		mWepIv[4];
U8 		mBcIv[4];
U8		mBssNum = 0;
U8		mBssCnt = 0;
U8		mBssIndex = 0;
U8 		dot11DesiredBssid[6];  // When macp->ap_scan=1, use this to associate with an AP.
const	U8  zeroMacAddress[6] = {0,0,0,0,0,0};
U16		mRequestFlag = 0;
Element		mBrates;
Element		dot11DesiredSsid;
BOOLEAN		mProbeWithSsid = FALSE;
Element		mErp = {{EID_ERP, 1, 0x00}};
U8		mPwrState = 0;
U8		mAuthAlogrithms[2] = {OPEN_SYSTEM, SHARE_KEY};
U16		mAuthAlg = OPEN_SYSTEM;
U8		mLimitedUser = 0;
U8		mBlockBSS = 0;
U8		mSwCipher = 0;
U16		mBeaconPeriod = 100;		
U16		mDtimPeriod = 1;
U16		mFragThreshold = 2432;
U16 		mRtsThreshold = 2432;
U16		mTmRetryConnect=0;
U8		mRadioOn = 1;
U16		mRfChannel = 0;				
Element 	mPhpm;
U8		mHiddenSSID = 0;
U8		mBcKeyVector[16];

U8		mMacMode = MIXED_MODE;
U8		mAuthMode;
//For Adhoc mode
#if Make_ADHOC
U8 WFAOUI_WPA[4] = { 0x00, 0x50, 0xF2, 0x01 }; 
U8 WFAOUI_ALL[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
U16		mATIMWindow;
U32     mTimeBeforeAdhocRoaming;
BssInfo_t	mBssInfo[1];
Element		mIbssParms;
U8		mMaxTxRate = 0x0b;
U8		mCurrConnUser = 0;
U8		mNumBOnlySta=0;
U16		mIv16 = 0;
U32		mIv32 = 0;

Element		mExtRates;
#endif
//end
#endif
