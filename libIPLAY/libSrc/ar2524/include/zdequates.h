/****************************************************************************
** COPYRIGHT (C) 2002 ZYDAS CORPORATION                                    **
** HTTP://WWW.ZYDAS.COM.TW/                                                **
****************************************************************************/

#ifndef _ZDEQUATES_H
#define _ZDEQUATES_H

//-------------------------------------------------------------------------
// Bit Mask definitions
//-------------------------------------------------------------------------
#define BIT_0       		0x0001
#define BIT_1       		0x0002
#define BIT_2       		0x0004
#define BIT_3       		0x0008
#define BIT_4       		0x0010
#define BIT_5       		0x0020
#define BIT_6       		0x0040
#define BIT_7       		0x0080
#define BIT_8       		0x0100
#define BIT_9       		0x0200
#define BIT_10      		0x0400
#define BIT_11      		0x0800
#define BIT_12      		0x1000
#define BIT_13      		0x2000
#define BIT_14      		0x4000
#define BIT_15      		0x8000
#define BIT_16      		0x00010000
#define BIT_17      		0x00020000
#define BIT_18      		0x00040000
#define BIT_19      		0x00080000
#define BIT_20      		0x00100000
#define BIT_21				0x00200000
#define BIT_22				0x00400000
#define BIT_23				0x00800000
#define BIT_24      		0x01000000
#define BIT_25				0x02000000
#define BIT_26				0x04000000
#define BIT_27				0x08000000
#define BIT_28      		0x10000000
#define BIT_29      		0x20000000
#define BIT_30      		0x40000000
#define BIT_31      		0x80000000

#define cMAX_MULTI_WRITE_REG_NUM    	15
#define cMIN_MULTI_WRITE_REG_NUM    	0
#define cMAX_MULTI_RF_REG_NUM       	28
#define cMAX_MULTI_READ_REG_NUM     	15

    #define	MAX_WLAN_SIZE				4800
    #define ZD_MAX_WLAN_SIZE			4800

#define HOST_PEND				BIT_31
#define CAM_WRITE				BIT_31
#define MAC_LENGTH				6
#define RX_MIC_ERROR_IND	    BIT_4 // Bit4 of ExtraInfo[6], its Bit3-Bit0 indicates the encryption type.
#define RX_HW_MIC_ENABLE	    BIT_25 // The subfield of ZD_SnifferOn

#define MIC_FINISH				BIT_0

#define RX_MIC_ERROR_IND		BIT_4
#define HW_MIC_ENABLE			BIT_25


#define PLCP_HEADER						5
#define EXTRA_INFO_LEN					5 	//8 for ZD1212

enum Frame_Control_Bit {
	TO_DS = BIT_0,
	FROM_DS = BIT_1,
	MORE_FRAG = BIT_2,
	RETRY_BIT = BIT_3,
	PWR_BIT = BIT_4,
	MORE_DATA = BIT_5,
	ENCRY_BIT = BIT_6,
	ODER_BIT = BIT_7

};

#define MAX_USER				40
#define MAX_KEY_LENGTH			16
#define ENCRY_TYPE_START_ADDR	60
#define DEFAULT_ENCRY_TYPE		65	
#define KEY_START_ADDR			66
#define STA_KEY_START_ADDR		386
#define COUNTER_START_ADDR		418
#define STA_COUNTER_START_ADDR	423

#define IBSS_MODE				BIT_25

#define BSS_INFO_NUM					64

//for USB
#define cTX_CCK                     	1       
#define cTX_OFDM                    	2       // 6M - 36M
#define cTX_48M                     	3
#define cTX_54M                     	4
#define cPWR_CTRL_GUARD             	4       // CR57: 4 -> 0.5 dB
#define cPWR_INT_VALUE_GUARD        	8       // CR31: 4 -> 1 dB; 8 -> 2 dB
#define cPWR_STRONG_SIG_DROP        	(0x18 - cPWR_INT_VALUE_GUARD)

/* Firmware Interface */
#define cTX_QUEUE_LEN               	4
// make sure already Tx by HMAC (for UMAC System)
// 1.Host->UMAC, 2.In UMAC Queue, 3.HMAC Sent
#define cTX_SENT_LEN                	(cTX_QUEUE_LEN + 2)
#define cFIRMWARE_OLD_ADDR          	0xEC00
#define cFIRMWARE_START_ADDR        	0xEE00
#define cFIRMWARE_EXT_CODE          	0x1000
#define cADDR_ENTRY_TABLE           	(cFIRMWARE_START_ADDR + 0x1D)
#define cBOOTCODE_START_ADDR        	0xF800
#define cINT_VECT_ADDR              	0xFFF5
#define cEEPROM_SIZE                	0x800   // 2k word (4k byte)

// in word (16 bit width)
#define cLOAD_CODE_LEN              	0xE
#define cLOAD_VECT_LEN              	(0x10000 - 0xFFF7)
#define cEPDATA_OFFSET              	(cLOAD_CODE_LEN + cLOAD_VECT_LEN)
#define USB_BASE_ADDR_EEPROM        	0x9900
#ifdef ZD1211B
	#define USB_BASE_ADDR_HOST         	0x9F00
#elif defined(ZD1211)
	#define USB_BASE_ADDR_HOST			0x9B00
#else
	#error	"***** You Need To Specified ZD1211 or ZD1211B *****"
#endif
#define BASE_ADDR_MASK_HOST         	(~0x00FF)
#define cFIRMWARE_EEPROM_OFFSET     	(cBOOTCODE_START_ADDR + cEPDATA_OFFSET)

#define TUPLE_CACHE_SIZE				16
#define MAX_DEFRAG_NUM					8

#define PS_CAM							0x0
#define	PS_PSM							0x1

// RF TYPE
#define UW2451_RF					0x2
#define uChip_RF					0x3
#define	AL2230_RF					0x4
#define	AL2210MPVB_RF				0x4
#define AL7230B_RF					0x5 //a,b,g RF
#define	THETA_RF					0x6
#define	AL2210_RF					0x7
#define	MAXIM_NEW_RF				0x8
#define	AR2124_RF				    0x8
#define	UW2453_RF					0x9
#define	AL2230S_RF                  0xA
#define	RALINK_RF					0xB
#define	INTERSIL_RF					0xC
#define	RFMD_RF						0xD

#define ELEID_SSID					0


#define	ELEID_SUPRATES				1
#define ELEID_DSPARMS				3
#define ELEID_TIM					5
#define ELEID_ERP_INFO				42
#define ELEID_EXT_RATES				50

#define mBIT(b)                 (1 << (b))
#define mMASK(w)                (mBIT(w) - 1)
#define mSET_MASK(a, b)         ((a) | (b))
#define mCLR_MASK(a, b)         ((a) & (~(b)))
#define mSET_BIT(a, b)          mSET_MASK(a, mBIT(b))
#define mCLR_BIT(a, b)          mCLR_MASK(a, mBIT(b))
#define mCHK_BIT1(a, b)         ((a) & mBIT(b))
#define mTEST_BIT(a, b)         mCHK_BIT1(a, b)
#endif
