/*
*******************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based 
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2004, Magic Pixel Inc, Hsinchu, Taiwan
*
* All rights reserved. Magic Pixel's source code is an unpublished work and the 
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in 
* deciphering, decoding, reverse engineering or in ay way altering the source 
* code is strictly prohibited, unless the prior written consent of Magic 
* Pixel is obtained.
*
* Filename      : mp650_ms.c
* Programmer(s) : 
* Created       : 
* Descriptions  :
*******************************************************************************
*/
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

//#define MS_IRQ
/*
// Include section 
*/
#include "global612.h"
#if (MS_ENABLE  && ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660)))
#include "mpTrace.h"
#include "Mcard.h"
#include "uti.h"
#include "ms.h"

#define MsProFaster	0
#define MsProNormal	1
//#define MsSpeedStatus MsProFaster
#define MsSpeedStatus MsProNormal

#define MS_PRO_HG        0

/*
// Constant declarations
*/
#define TPC_READ_LONG_DATA      0x00000002
#define TPC_READ_SHORT_DATA     0x00000003
#define TPC_WRITE_LONG_DATA     0x0000000d
#define TPC_WRITE_SHORT_DATA    0x0000000c
#define TPC_READ_QUAD_DATA      0x00000005
#define TPC_WRITE_QUAD_DATA     0x0000000a
#define TPC_GET_INT             0x00000007
#define TPC_SET_CMD             0x0000000e
#define TPC_EX_SET_CMD          0x00000009
#define TPC_WRITE_REG           0x0000000b
#define TPC_READ_REG            0x00000004
#define TPC_SET_RW_REG_ADR      0x00000008

#define DATA_PATH_BY_DMA        0x00000040
#define DATA_PATH_BY_CPU        0x00000000
#define PAGE_MOD_512_BYTE       0x00000080
#define PAGE_MOD_BY_DATA_LEN    0x00000000

#define STATUS_HWRREQ           0x00000001
#define STATUS_HRDREQ           0x00000002
#define STATUS_FIFOIDLE         0x00000004
#define STATUS_FIFOFULL         0x00000008
#define STATUS_FIFOEMPTY        0x00000010
#define STATUS_INVALIDACC       0x00000020
#define STATUS_CMDINT           0x00000040
#define STATUS_CMDEND           0x00000080
#define STATUS_CRCERROR         0x00000100
#define STATUS_TIMEOUT0         0x00000200
#define STATUS_TIMEOUT1         0x00000400
#define STATUS_INVALIDCMD       0x00020000
#define STATUS_CED              0x00040000
#define STATUS_ERR              0x00080000
#define STATUS_BREQ             0x00100000
#define STATUS_CMDNK            0x00200000


#define IC_HWRREQ               0x00000001
#define IC_HRDREQ               0x00000002
#define IC_CMDINT               0x00000004
#define IC_CMDEND               0x00000008
#define IC_TIMEOUT1             0x00000010
#define IC_INVALIDCMD           0x00000020
#define IM_HWRREQ               0x00000100
#define IM_HRDREQ               0x00000200
#define IM_CMDINT               0x00000400
#define IM_CMDEND               0x00000800
#define IM_TIMEOUT1             0x00001000
#define IM_INVALIDCMD           0x00002000
#define PL_HWRREQ               0x00010000
#define PL_HRDREQ               0x00020000
#define PL_CMDINT               0x00040000
#define PL_CMDEND               0x00080000
#define PL_TIMEOUT1             0x00100000
#define PL_INVALIDCMD           0x00200000
#define PL_ALL_HIGH (PL_HWRREQ | PL_HRDREQ | PL_CMDINT | PL_CMDEND | PL_TIMEOUT1 | PL_INVALIDCMD)
#define MD_HWRREQ               0x01000000
#define MD_HRDREQ               0x02000000
#define MD_CMDINT               0x04000000
#define MD_CMDEND               0x08000000
#define MD_TIMEOUT1             0x10000000
#define MD_INVALIDCMD           0x20000000
#define GPIO_DEFINE_0           0x00000010
#define GPIO_DEFINE_1           0x00008000
#define GPIO_DEFINE_2           0x0f000f00

#define WATCHDOG_TIMER          0x003fffff
#define BUS_WIDTH_4             0x20
#define BUS_WIDTH_1             0x60
#define BUS_WIDTH_8             0x1020

// MS Pro Spec Chap. 7 Data Format
#define MS_SIGNATURE_CODE           0xA5C3
#define SYS_INFO                    0x10                //big
#define SYS_INFO_SIZE               96
#define MODEL_NAME                  0x15
#define MODEL_NAME_SIZE             48
#define MBR_VALUE                   0x20                //lit
#define MBR_VALUE_SIZE              16
#define PBR_VALUE_12                0x21                //lit
#define PBR_VALUE_12_SIZE           64
#define PBR_VALUE_32                0x22                //lit
#define PBR_VALUE_32_SIZE           96
#define SPEC_FILE_VALUE_1           0x25                //lit
#define SPEC_FILE_VALUE_1_SIZE      32
#define SPEC_FILE_VALUE_2           0x26                //lit
#define SPEC_FILE_VALUE_2_SIZE      32
#define IDENTIFY_DEV_INFO           0x30
#define IDENTIFY_DEV_INFO_SIZE      16


#define ACCESS_LONG_DATA            0
#define ACCESS_2K_DATA              1

///
///@defgroup MS Memory Stick
///@ingroup CONSTANT
///@{


/// Unsupport memory stick type.
#define TYPE_NOT_SUPPORT            -2
#define CMD_FAIL                    -3
/// Transfer protocol command's ready signal fail.
#define TPC_RB_TIMEOUT              -4
/// Transfer protocol command's interrupt signal fail.
#define TPC_INT_TIMEOUT             -5
/// When reading data, the data had failed the CRC examination.
#define READ_DATA_CRC_ERROR         -6
/// Wait for DMA end Fail.
#define DMA_TIMEOUT                 -7
/// Memory Stick has been switched to write protect ON.
#define WRITE_PROTECT               -8
#define WRITE_PS_NG                 -9
///@}
// Define Loop count time out
#define COUNT_TIME_OUT              -10
//Define switch interface fail
#define SWITCH_FAIL                 -11

#define RETRY_COUNT                  3

#define READ_TIMEOUT1    0x33333	// 5ms = x * 1/6.75MHz
#define WRITE_TIMEOUT1   0x66666	// 10ms = x * 1/6.75MHz
#define ERASE_TIMEOUT1   0x400000	// 100ms = x * 1/6.75MHz
#define PRO_TIMEOUT1     0x2800000  //0x134fd90	// 1000ms

// define command code for MS
#define BLOCK_READ					0x00
#define BLOCK_WRITE					0x01
#define BLOCK_END					0x02
#define BLOCK_ERASE					0x03
#define SLEEP						0x00
#define CLEAR_BUF					0x01
#define RESET						0x02

// define command code for MS PRO
#define PRO_READ_DATA               0x00
#define PRO_WRITE_DATA              0x01
#define PRO_READ_ATRB               0x02
#define PRO_STOP                    0x03
#define PRO_ERASE                   0x04
#define PRO_READ_2K_DATA            0x05
#define PRO_WRITE_2K_DATA           0x06
#define PRO_FORMAT                  0x00
#define PRO_SLEEP                   0x01

#define CED         0x80
#define ERR         0x40
#define BREQ        0x20
#define CMDNK       0x01

#define UCFG        0x01
#define FGER        0x02
#define UCEX        0x04
#define EXER        0x08
#define UCDT        0x10
#define DTER        0x20
#define FB1         0x40
#define MB          0x80

#define MAXSEGMENT              32
#define MAXLOGBLOCK             496
#define MAXPHYBLOCK             512

#define FORMAT_PARAM_MAX_SIZE   6

#define MS_1BIT_CLK_KHZ 20000
#define MS_4BIT_CLK_KHZ 40000
#define MS_8BIT_CLK_KHZ 40000//32000//60000

#define FMT_READ_TPC     0x0100    // 0x0000: READ_SHORT_DATA     0x0100: READ_LONG_DATA
#define FORMAT_MODE      0x01      // 0x00: Quick Format   0x01: Full Format


#define TYPE_MS        0
#define TYPE_MS_PRO    1
#define TYPE_MS_PRO_HG 2

/*
// Structure declarations
*/
struct ST_PRO_PARAM_REG_TAG
{
	BYTE bCmdCode;
	BYTE bDataCount1;
	BYTE bDataCount0;
	BYTE bDataAddr3;
	BYTE bDataAddr2;
	BYTE bDataAddr1;
	BYTE bDataAddr0;
	BYTE bTpcParameter;
	BYTE bCmdParameter;
	BYTE bReserve[3];
};

struct ST_INFO_TAG
{
	BYTE bBootBlock;
	BYTE bWriteProtected;
	BYTE bMsPro;              /* 0 = Ms, 1 = MsPro,  2 = MsPro-HG */
	WORD wBusWidth;           /* serial=0x60, 4-bit=0x20, 8-bit=0x1020 */
	BYTE bBootProtectFlag;
	BYTE bProtectBlockCount;
	BYTE bReserve0[1];
	BYTE bExtraData[32][4];
	WORD wProtectBlockTable[11];
	BYTE bReserve1[2];
	WORD *wLog2PhyTable; //[MAXSEGMENT * MAXPHYBLOCK];
	WORD wReAssignTable[MAXSEGMENT];
	DWORD dwSegment;
	DWORD dwBlockSize;
	DWORD dwBlock;
	DWORD dwEffBlock;
	DWORD dwPagePerBlock;
	DWORD dwCapacity;
       BYTE bLockStatus;
	BYTE bSwitchFail;  
};

struct ST_RW_REG_TPC_ADDR_TAG
{
	BYTE bReadRegAddr;
	BYTE bReadRegSize;
	BYTE bWriteRegAddr;
	BYTE bWriteRegSize;
};

struct ST_EXTRA_DATA_TAG
{
	BYTE bOverWriteFlag;
	BYTE bManagementFlag;
	BYTE bLogicalAddr1;
	BYTE bLogicalAddr0;
	BYTE bReserveArea4;
	BYTE bReserveArea3;
	BYTE bReserveArea2;
	BYTE bReserveArea1;
	BYTE bReserveArea0;
	BYTE bReserve[3];
};

struct ST_PARAM_REG_TAG
{
	BYTE bSysParameter;
	BYTE bBlockAddrReg2;
	BYTE bBlockAddrReg1;
	BYTE bBlockAddrReg0;
	BYTE bCmdParameter;
	BYTE bPageAddr;
//	BYTE bReserve[2];
	struct ST_EXTRA_DATA_TAG sExtra;
};

struct ST_FORMAT_PARAM_TAG
{
	BYTE bBlockSize;
	WORD wCapacityID;	
	BYTE bMbrParam[16];
	BYTE bPbrParam[62];
};

struct ST_DEV_INFO_ENTRY_TAG
{
     DWORD dwEntryAddr;
     DWORD dwEntrySize;
     BYTE bDevInfoID;
     BYTE bReserved[3];
};

struct ST_ATTR_INFO_AREA_TAG
{
      WORD  wSignCode;
      WORD  wVersionInfo;
      BYTE bDevEntryCount;
      BYTE bReserved0[11];
      struct ST_DEV_INFO_ENTRY_TAG stDevInfoEntry[12];
      BYTE bReserved1[256];
};


/*
struct ST_SYSTEM_PARAM_TAG
{
    BYTE  bMsClass;
    BYTE  bReserved0;
    WORD wBlockSize;
    WORD wTotalBlocks;
    WORD wUserAreaBlocks;
    WORD wPageSize;
    BYTE bReserved1[2];
    BYTE bTimeDiffGMT;
    WORD wYear;
    BYTE bMonth;
    BYTE bDay;
    BYTE bHour;
    BYTE bMinute;
    BYTE bSecond;
    DWORD dwSerialNo;
    BYTE bAsmMarkerCode;
    BYTE bAsmModelCode[3];
    WORD wMemMarkerCode;
    WORD wMemModelCode;
    BYTE bReversed2[4];
    BYTE bVcc;
    BYTE vVpp;
    WORD wCtrlNo;
    WORD wCtrlFunc;
    WORD wStartSector;
    WORD wUnitSize;
    BYTE bMsSubClass;
    BYTE bReserved3[4];
    BYTE bInterfaceType;
    WORD wCtrlCode;
    BYTE bFormatType;
    BYTE bReserved4;
    BYTE bDeviceType;
    BYTE bReserved5[7];
    BYTE bMSProID[16];
    BYTE bReserved6[16];
};    
*/

/*
// Type declarations
*/
typedef struct ST_INFO_TAG ST_INFO;
typedef struct ST_RW_REG_TPC_ADDR_TAG ST_RW_REG_TPC_ADDR;
typedef struct ST_EXTRA_DATA_TAG ST_EXTRA_DATA;
typedef struct ST_PARAM_REG_TAG ST_PARAM_REG;
typedef struct ST_PRO_PARAM_REG_TAG ST_PRO_PARAM_REG;
typedef struct ST_FORMAT_PARAM_TAG ST_FORMAT_PARAM;
typedef struct ST_DEV_INFO_ENTRY_TAG ST_DEV_INFO_ENTRY; 
typedef struct ST_ATTR_INFO_AREA_TAG ST_ATTR_INFO_AREA;
//typedef struct ST_SYSTEM_PARAM_TAG ST_SYSTEM_PARAM;

/*
// Variable declarations
*/
static DWORD bMS_CLOCK = MS_1BIT_CLK_KHZ;
static ST_INFO sInfo;
static ST_RW_REG_TPC_ADDR sRWRegTpcAddr;
static ST_PARAM_REG sParamReg;
static ST_PRO_PARAM_REG sProParamReg;
static const BYTE bFlashCMD[5] = { 0xAA, 0x55, 0x33, 0x99, 0xCC };
static const BYTE bFuncCMD[3] = { 0x5A, 0xC3, 0x3C };

static const BYTE bProFlashCMD[7] = { 0x20, 0x21, 0x24, 0x25, 0x26, 0x27, 0x28 };
static const BYTE bProFuncCMD[2] = { 0x10, 0x11 };

static BYTE bDescriptor[] = "MS";
static const BYTE bSpecialFile[12] =
	{ 'M', 'E', 'M', 'S', 'T', 'I', 'C', 'K', 'I', 'N', 'D', 0X03 };

//static ST_SYSTEM_PARAM sSysParam;

/*
// Macro declarations
*/
/*
// Macro declarations
*/
#pragma alignvar(4)
//#define MS_TASK_YIELD
#ifdef MS_TASK_YIELD
#define TASK_YIELD() TaskYield()
#else
#define TASK_YIELD()
#endif

#define SetRWRegTpcAddr(bReadAddr, bReadSize, bWriteAddr, bWriteSize)   \
{                                                                       \
	register ST_RW_REG_TPC_ADDR *psRWRegTpcAddr = &sRWRegTpcAddr;		\
    psRWRegTpcAddr->bReadRegAddr = bReadAddr;                             \
    psRWRegTpcAddr->bReadRegSize = bReadSize;                             \
    psRWRegTpcAddr->bWriteRegAddr = bWriteAddr;                           \
    psRWRegTpcAddr->bWriteRegSize = bWriteSize;                           \
}

#define SetParaReg(bSysParam, dwBlockAddrReg, bCmdParam, bPageAdr)      \
{                                                                       \
	register ST_PARAM_REG *psParamReg = &sParamReg;						\
    psParamReg->bSysParameter  = bSysParam;                             \
    psParamReg->bBlockAddrReg2 = (BYTE)(dwBlockAddrReg >> 16);          \
    psParamReg->bBlockAddrReg1 = (BYTE)(dwBlockAddrReg >> 8);           \
    psParamReg->bBlockAddrReg0 = (BYTE)(dwBlockAddrReg);                \
    psParamReg->bCmdParameter  = bCmdParam;                             \
    psParamReg->bPageAddr      = bPageAdr;                              \
}

#define SetWriteParaReg(bSysParam, dwPhyAdr, bCmdParam, bPageAdr)           \
{                                                                           \
	register ST_PARAM_REG *psParamReg = &sParamReg;							\
    psParamReg->bSysParameter = bSysParam;                                    \
    psParamReg->bBlockAddrReg2 = (BYTE)(dwPhyAdr >> 16);                      \
    psParamReg->bBlockAddrReg1 = (BYTE)(dwPhyAdr >> 8);                       \
    psParamReg->bBlockAddrReg0 = (BYTE)(dwPhyAdr);                            \
    psParamReg->bCmdParameter = bCmdParam;                                    \
    psParamReg->bPageAddr = bPageAdr;                                         \
}

#define SetExtraParaReg(bOverWrite, bManagement, bLogAddr1, bLogAddr0)      \
{                                                                           \
	register ST_PARAM_REG *psParamReg = &sParamReg;							\
    psParamReg->sExtra.bOverWriteFlag = bOverWrite;                           \
    psParamReg->sExtra.bManagementFlag  = bManagement;                        \
    psParamReg->sExtra.bLogicalAddr1 = bLogAddr1;                             \
    psParamReg->sExtra.bLogicalAddr0  = bLogAddr0;                            \
}


#define SetDataDma(dwBuffer)                            \
{                                                       \
    register CHANNEL *sChannel;                         \
    sChannel = (CHANNEL *)(DMA_MC_BASE);                \
	sChannel->Control = 0;                              \
	sChannel->StartA = dwBuffer;                        \
    sChannel->EndA = dwBuffer + MCARD_SECTOR_SIZE - 1;  \
    sChannel->Control = MCARD_DMA_ENABLE;               \
}


#define Set2KDataDma(dwBuffer)                            \
{                                                       \
    register CHANNEL *sChannel;                         \
    sChannel = (CHANNEL *)(DMA_MC_BASE);                \
	sChannel->Control = 0;                              \
	sChannel->StartA = dwBuffer;                        \
    sChannel->EndA = dwBuffer + (MCARD_SECTOR_SIZE<<2) - 1;  \
    sChannel->Control = MCARD_DMA_ENABLE;               \
}


void static inline SetProParaReg(register BYTE bCmd, register WORD wSectorCount,
								 register DWORD dwLogAddr, register BYTE bTpcParam,
								 register BYTE bCmdParam)
{
	register ST_PRO_PARAM_REG *psProParamReg = &sProParamReg;

	psProParamReg->bCmdCode = bCmd;
	if (wSectorCount == 0)
	{
		psProParamReg->bDataCount1 = 0;
		psProParamReg->bDataCount0 = 0;
	}
	else
	{
		psProParamReg->bDataCount1 = (BYTE) (wSectorCount >> 8);
		psProParamReg->bDataCount0 = (BYTE) (wSectorCount);
	}
	if (dwLogAddr == 0)
	{
		psProParamReg->bDataAddr3 = 0;
		psProParamReg->bDataAddr2 = 0;
		psProParamReg->bDataAddr1 = 0;
		psProParamReg->bDataAddr0 = 0;
	}
	else
	{
		psProParamReg->bDataAddr3 = (BYTE) (dwLogAddr >> 24);
		psProParamReg->bDataAddr2 = (BYTE) (dwLogAddr >> 16);
		psProParamReg->bDataAddr1 = (BYTE) (dwLogAddr >> 8);
		psProParamReg->bDataAddr0 = (BYTE) (dwLogAddr);
	}
	psProParamReg->bTpcParameter = bTpcParam;
	psProParamReg->bCmdParameter = bCmdParam;
}

/*
// Static function prototype
*/
static void CommandProcess(void *McardDev);
static void Select(void);
static void DeSelect(void);
static SWORD ProLogicalWrite(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress);
static SWORD Pro2KLogicalWrite(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress);
static SWORD ProLogicalRead(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress);
static SWORD Pro2KLogicalRead(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress);
static SWORD ProReadAttribute(DWORD dwBufferAddress, DWORD dwLogAddr);
static SWORD ProWaitCpuStartup(void);
//static SWORD WaitDMAEnd(void);
static DWORD WaitCmdEnd(void);
static SWORD GetIntValue(BYTE * bIntValue);
//static SWORD Wait_INT_Signal_BREQ(DWORD *dwMcMsStatusTemp);
//static SWORD Wait_INT_Signal_CED(DWORD *dwMcMsStatusTemp);
//static SWORD Wait_INT_Signal(DWORD *dwMcMsStatusTemp);
static SWORD WaitTime1INT(void);
//static SWORD Wait_For_INT_Pulse(DWORD *dwMcMsStatusTemp);

static SWORD WriteTpcBs(BYTE bTpc, BYTE * pbBuffer, BYTE bLen);
static SWORD ReadTpcBs(BYTE bTpc, BYTE * pbBuffer, BYTE bLen);
//static SWORD ReadTpcBsPage(void);
static SWORD ReadTpcBsPage(BYTE access2Kdata);
//static SWORD WriteTpcBsPage(void);
static SWORD WriteTpcBsPage(BYTE access2Kdata);
static SWORD InitLog2PhyTable(void);
static SWORD CheckDisableBlock(DWORD dwPhyAddr, BYTE * pbDisableAddr);
static WORD GetEmptyBlock(DWORD dwSegNum);
static WORD GetSameBlockPages(DWORD dwLogAddr, DWORD dwSectorCount);
static SWORD IdentifyType(void);
static SWORD ReadExtraData(DWORD dwBlockAddr, DWORD dwPageNumber, DWORD dwBufferAddress);
static SWORD PhysicalRead(DWORD dwBlockAddr, DWORD dwStartPage, DWORD dwSectorCount,
						  DWORD dwBufferAddress);
static SWORD LogicalRead(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress);
static SWORD FlatRead(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr);
static SWORD OverWriteExtraData(DWORD dwBlockAddr, DWORD dwPageNumber, BYTE bOverWriteFlag);
static SWORD WriteExtraData(DWORD dwBlockAddr, DWORD dwPageNumber, DWORD dwLogAddr);
static SWORD PhysicalErase(DWORD dwBlockAddr);
static SWORD PhysicalWrite(DWORD dwPhyAddr, DWORD dwStartPage, DWORD dwSectorCount,
						   DWORD dwBufferAddress, DWORD dwLogAddr);
static SWORD LogicalWrite(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress);
static SWORD FlatWrite(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr);
static SWORD GetAttributeInfo(void);
static SWORD BootProtectProcess(BYTE bBlockMaxCount);
static SWORD Identify(void);
static SWORD Format(void);
static SWORD ChangeBusWidth(WORD wBusWidth);
static SWORD CheckSystemParameter( DWORD pbBuffer, BYTE *pbSysParam ,WORD *wBusWidth);
static SWORD RecognizeFileSystem(void);
static SWORD LowLevelFormat();

/*
// Definition of internal functions
*/

static SWORD WaitDataTransfer()
{
	CHANNEL *pstChannel = (CHANNEL *)DMA_MC_BASE;
	DWORD tmr = GetSysTime();
	SWORD ret = FAIL;
	do
	{
		if (!(pstChannel->Control & MCARD_DMA_ENABLE))
		{
			ret = PASS;
			break;
		}
	}
	while (SystemGetElapsedTime(tmr) < 1000);	// 1sec
	if (ret == FAIL)
	{
		mpDebugPrint("MS: DMA timeout, DMA_C=%x", pstChannel->Control);
//		while(1);
	}

	return ret;
}

static void Power_On(void)
{
	McardPowerOn(DEV_MS);
}

static void Power_Off(void)
{
	McardPowerOff(DEV_MS);
}

/*
// Definition of local functions 
*/
void MsInit(ST_MCARD_DEV * sDev)
{
	sInfo.wLog2PhyTable = NULL;
	sDev->pbDescriptor = bDescriptor;
	sDev->wMcardType = DEV_MS;
	sDev->Flag.Installed = 1;
	sDev->CommandProcess = CommandProcess;
}

static inline BOOL Polling_MS_Status(void)
{
	return (!Mcard_GetDetected(DEV_MS));
}

static void CommandProcess(void *pMcardDev)
{
	GPIO *sGpio;
	DWORD dwTemp;
	register DWORD i;
	register ST_MCARD_DEV *pDev = ((ST_MCARD_DEV *) pMcardDev);
	ST_MCARD_MAIL *mail = pDev->sMcardRMail;
	sGpio = (GPIO *) (GPIO_BASE);

	//mpDebugPrint("MS(%x) Command: %x(%x,%d)", pDev->wMcardType, mail->wCmd, mail->dwBlockAddr, mail->dwBlockCount);
	SetMcardClock(bMS_CLOCK);
	Select();
	switch (mail->wCmd)
	{
		case INIT_CARD_CMD:
			Power_On();
			mpDebugPrint("CARD IN INIT");
			sInfo.bSwitchFail = 0;
			if (pDev->Flag.Detected)
			{
				if( (sInfo.wBusWidth == BUS_WIDTH_4)||(sInfo.wBusWidth == BUS_WIDTH_8))
				{
					MP_DEBUG("switch bus from 4 bit or 8 bit to 1 bit");
					ChangeBusWidth(BUS_WIDTH_1);
				}
				else
				{
					sInfo.wBusWidth = BUS_WIDTH_1;
				}

				bMS_CLOCK = MS_1BIT_CLK_KHZ;
				SetMcardClock(bMS_CLOCK);
				
				mail->swStatus = Identify();
#if MCARD_POWER_CTRL
				if( mail->swStatus == SWITCH_FAIL)     // if first identify fail
				{
					Power_Off();						//turn off and turn on the power 
					Power_On();
					sInfo.bSwitchFail = 1;				// switch fail = 1; this flag will force the card in 1 bit mode
					sInfo.wBusWidth = BUS_WIDTH_1;
					bMS_CLOCK = MS_1BIT_CLK_KHZ;
					SetMcardClock(bMS_CLOCK);					
					mail->swStatus = Identify();		//retry to identify the card again in 1 bit mode
					mpDebugPrint("switch Fail");
				}
#endif				
				if(mail->swStatus)
				{
					pDev->Flag.Present = 0;
					pDev->dwCapacity = 0;
					pDev->wSectorSize = 0;
					pDev->wSectorSizeExp = 0;
					bMS_CLOCK = MS_1BIT_CLK_KHZ;
					Power_Off();
					MP_ALERT("MS identify fail!");
				}
				else
				{
					//bMS_CLOCK = MS_4BIT_CLK_KHZ;
					//SetMcardClock(bMS_CLOCK);
					pDev->wRenewCounter++;
					pDev->Flag.ReadOnly = sInfo.bWriteProtected;
					pDev->Flag.Present = 1;
					pDev->dwCapacity = sInfo.dwCapacity;
					pDev->wSectorSize = MCARD_SECTOR_SIZE;
					pDev->wSectorSizeExp = MCARD_SECTOR_SIZE_EXP;
				}
			}
			else
			{
				//card out
				if(sInfo.wLog2PhyTable != NULL)
				{
					ker_mem_free(sInfo.wLog2PhyTable);
					sInfo.wLog2PhyTable = NULL;				
				}

				// sInfo.wBusWidth = BUS_WIDTH_1;
				pDev->Flag.Present = 0;
				pDev->Flag.ReadOnly = 0;
				pDev->Flag.PipeEnable = 0;
				mail->swStatus = 0;
				pDev->dwCapacity = 0;
				pDev->wSectorSize = 0;                     
				pDev->wSectorSizeExp = 0;
				bMS_CLOCK = MS_1BIT_CLK_KHZ;
				Power_Off();
			}              
			//__asm("break 100");
			break;
		case REMOVE_CARD_CMD:		//Athena 03.11.2006 seperate card in & out
			//card out
			if(sInfo.wLog2PhyTable != NULL)
			{
				ker_mem_free(sInfo.wLog2PhyTable);
				sInfo.wLog2PhyTable = NULL;				
			}
			sInfo.wBusWidth = BUS_WIDTH_1;
			pDev->Flag.Present = 0;
			pDev->Flag.ReadOnly = 0;
			pDev->Flag.PipeEnable = 0;
			mail->swStatus = 0;
			pDev->dwCapacity = 0;
			pDev->wSectorSize = 0;
			pDev->wSectorSizeExp = 0;
			bMS_CLOCK = MS_1BIT_CLK_KHZ;
			sInfo.bSwitchFail = 0;
			mpDebugPrint("CARD OUT");
			Power_Off();
			break;
		case SECTOR_READ_CMD:
			if( sInfo.bLockStatus)
			{
				mail->swStatus = FAIL;
				mpDebugPrint("R  sInfo.bLockStatus = %d",sInfo.bLockStatus);
			}
			else
			{
				mail->swStatus =
				FlatRead(mail->dwBuffer, mail->dwBlockCount, mail->dwBlockAddr);
			}
			break;

		case SECTOR_WRITE_CMD:
			if( sInfo.bLockStatus)
			{
				mail->swStatus = FAIL;
				mpDebugPrint("W sInfo.bLockStatus = %d",sInfo.bLockStatus);
			}
			else
			{        
				mail->swStatus =
				FlatWrite(mail->dwBuffer, mail->dwBlockCount, mail->dwBlockAddr);
			}
			break;

		case RAW_FORMAT_CMD:
			if( sInfo.bLockStatus || sInfo.bWriteProtected)
			{
				mail->swStatus = FAIL;
				mpDebugPrint("W sInfo.bLockStatus = %d",sInfo.bLockStatus);
			}
			else
			{
				mail->swStatus = LowLevelFormat();
			}	
			break;

		default:
			MP_DEBUG("-E- INVALID CMD");
			break;
	}
	DeSelect();
}

static void DeSelect(void)
{
	// Some MS card will hold data pins in interrupt state.
	// Send an incomplete dummy command to prevent MS staying in interrupt state
#if ((CHIP_VER_MSB != CHIP_VER_660))
	((MCARD *)MCARD_BASE)->McMsCmd = 0x10b;    // wrong tpc command force the card into two state access mode. and data 4~7 will not be pulled when nand flash R/W
	/*
	 *    platform MP660
	 *    One card has problem if we use ths mechanism!!   - MS Lex???? 128MB
	 */
	
#endif
	// if the card socket is 8 dat pins for HG, the MS Pro will be wrong when read/write nand flash. 
	
	// if the card socket is 8 dat pins for HG, the MS Pro will be wrong when read/write nand flash.
	McardDeselect(PIN_DEFINE_FOR_MS);
}

static void Select(void)
{
	register MCARD *sMcard;
	register GPIO *sGpio;
	DWORD dwTemp;

	sGpio = (GPIO *) (GPIO_BASE);
	sMcard = (MCARD *) (MCARD_BASE);
	sMcard->McardC = 0;

	McardSelect(PIN_DEFINE_FOR_MS);
	sMcard->McardC = (MCARD_FIFO_EMPTY | MCARD_MS_ENABLE) + sInfo.wBusWidth;	// enable MS function
	sMcard->McWdt = WATCHDOG_TIMER;
}

static SWORD ProWaitCpuStartup(void)
{
	SWORD swRetValue;
	DWORD i;
	BYTE bGetValue;

	for (i = 0; i < 0x4000; i++)
	{
		if ((swRetValue = ReadTpcBs(TPC_GET_INT, &bGetValue, 1)))
		{
			MP_DEBUG1("-E- TPC_GET_INT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		if (bGetValue & 0x80)	//CED
		{
			if ((swRetValue = ReadTpcBs(TPC_GET_INT, &bGetValue, 1)))
			{
				MP_DEBUG1("-E- TPC_GET_INT FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}

			if (bGetValue & 0x40)     //detect  ERR 
			{			
				if (bGetValue & 0x01)    //detect CMDNK
				{
					sInfo.bWriteProtected = 1;
					return PASS;
				}
				else
				{
					MP_DEBUG1("-E- ms cpu startup FIAL (status: %x)", bGetValue);
					Power_Off();         //   T7-010  2-7-9
					return FAIL;
				}
			}
			else
			{
				return PASS;
			}
		}
		IODelay(100);
	}
	Power_Off();
	MP_DEBUG("-E- ms cpu startup FIAL");
	return FAIL;
}

#if MsSpeedStatus  // rick ms pro faster 7/4
static SWORD ProLogicalWrite(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress)
{
	register MCARD *sMcard;
	DWORD i;
	SWORD swRetValue;
	BYTE bIntValue;

	sMcard = (MCARD *) (MCARD_BASE);
	sMcard->McMsT1 = PRO_TIMEOUT1;
	//Set Parameter Register
	SetProParaReg(bProFlashCMD[PRO_WRITE_DATA], dwSectorCount, dwLogAddr, 0x00, 0x00);

       if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
	{
		MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = WaitTime1INT()))
	{
		MP_DEBUG1("-E- EX WaitTime1INT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = GetIntValue(&bIntValue)))
	{
		MP_DEBUG1("-E- EX GetINT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ( bIntValue == (CED|ERR|CMDNK))
      {
		MP_DEBUG1("-E- Write disabled status (bIntValue: %d)", bIntValue);
		sInfo.bWriteProtected = 1;
		return WRITE_PROTECT;
	}
       else if ((bIntValue & BREQ) && (bIntValue & ERR))
       {
		//Set Parameter Register
		MP_DEBUG1("-E- EX Error termination (bIntValue: %d)", bIntValue);
		SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
		if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
		{
			MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		return FAIL;
	}

	for (i = 0; i < dwSectorCount; i++)
	{
		SetDataDma(dwBufferAddress);    
		if ((swRetValue = WriteTpcBsPage(ACCESS_LONG_DATA)))
		{
			MP_DEBUG1("-E- WriteTpcBsPage FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		if((swRetValue = WaitTime1INT()))
		{
			MP_DEBUG1("-E- GetINT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		if ((swRetValue = GetIntValue(&bIntValue)))
		{
			MP_DEBUG1("-E- GetINT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		if (bIntValue == (CED|ERR|CMDNK))
		{
		       sInfo.bWriteProtected = 1;
                     //sInfo.bLockStatus = 1;
			MP_DEBUG1("-E- Write disabled status (bIntValue: %d)", bIntValue);
			return WRITE_PROTECT;
		}
              else if ((bIntValue & CED) && (bIntValue & ERR))
		{
			//Set Parameter Register
			MP_DEBUG1("-E- Error termination (bIntValue: %d)", bIntValue);
			SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
			if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
			{
				MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}
			return FAIL;
		}
		WaitDataTransfer();
              // force update cache
              SetDataCacheInvalid();                   
		dwBufferAddress += MCARD_SECTOR_SIZE;
	}
	return PASS;
}


static SWORD Pro2KLogicalWrite(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress)
{
	register MCARD *sMcard;
	DWORD i;
	SWORD swRetValue;
	BYTE bIntValue;

	sMcard = (MCARD *) (MCARD_BASE);
	sMcard->McMsT1 = PRO_TIMEOUT1;
	//Set Parameter Register

       if((dwLogAddr&0x03)||(dwSectorCount&0x03))
        {
            __asm("break 100");
       }
	SetProParaReg(bProFlashCMD[PRO_WRITE_2K_DATA], dwSectorCount, dwLogAddr, 0x00, 0x00);

       if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
	{
		MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = WaitTime1INT()))
	{
		MP_DEBUG1("-E- EX WaitTime1INT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = GetIntValue(&bIntValue)))
	{
		MP_DEBUG1("-E- EX GetINT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ( bIntValue == (CED|ERR|CMDNK))
      {
		MP_DEBUG1("-E- Write disabled status (bIntValue: %d)", bIntValue);
		sInfo.bWriteProtected = 1;
		return WRITE_PROTECT;
	}
	else if ((bIntValue & BREQ) && (bIntValue & ERR))
	{
		//Set Parameter Register
		MP_DEBUG1("-E- EX Error termination (bIntValue: %d)", bIntValue);
		SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
		if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
		{
			MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		return FAIL;
	}

	for (i = 0; i < dwSectorCount; i+=4)
	{
		Set2KDataDma(dwBufferAddress);    
		if ((swRetValue = WriteTpcBsPage(ACCESS_2K_DATA)))
		{
			MP_DEBUG1("-E- WriteTpcBsPage FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		if((swRetValue = WaitTime1INT()))
		{
			MP_DEBUG1("-E- GetINT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		if ((swRetValue = GetIntValue(&bIntValue)))
		{
			MP_DEBUG1("-E- GetINT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		if ( bIntValue == (CED|ERR|CMDNK))
      		{
			MP_DEBUG1("-E- Write disabled status (bIntValue: %d)", bIntValue);
			sInfo.bWriteProtected = 1;
			return WRITE_PROTECT;
		}		
		else if ((bIntValue & CED) && (bIntValue & ERR))
		{
			//Set Parameter Register
			MP_DEBUG1("-E- Error termination (bIntValue: %d)", bIntValue);
			SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
			if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
			{
				MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}
			return FAIL;
		}
		WaitDataTransfer();
              // force update cache
              SetDataCacheInvalid();     
		dwBufferAddress += (MCARD_SECTOR_SIZE<<2);
	}
	return PASS;
}

static SWORD ProLogicalRead(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress)
{
	register MCARD *sMcard;
	register DWORD i;
	register SWORD swRetValue,stopRetValue;
	BYTE bIntValue;

	sMcard = (MCARD *) (MCARD_BASE);
	sMcard->McMsT1 = PRO_TIMEOUT1;
	//Set Parameter Register
	SetProParaReg(bProFlashCMD[PRO_READ_DATA], dwSectorCount, dwLogAddr, 0x00, 0x00);
	//MP_DEBUG("EX_SET_CMD  count = %d dwBufferAddress = 0x%x",dwSectorCount,dwBufferAddress);
	if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
	{
		MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}
	if ((swRetValue = WaitTime1INT()))
	{
		MP_DEBUG1("-E- EX WaitTime1INT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = GetIntValue(&bIntValue)))
	{
		MP_DEBUG1("-E- EX GetINT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}
	if ((bIntValue & CMDNK) || ((bIntValue & BREQ) && (bIntValue & ERR)))
	{
		MP_DEBUG1("-E- EX CMD Error termination (bIntValue: %d)", bIntValue);
		//Set Parameter Register
		/*
		SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
		if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
		{
			MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		*/
		return FAIL;
	}

	for (i = 0; i < dwSectorCount; i++)
	{
		SetDataDma(dwBufferAddress);
		if ((swRetValue = ReadTpcBsPage(ACCESS_LONG_DATA)))
		{
			MP_DEBUG1("-E- ReadTpcBsPage FAIL (swRetValue: %d)", swRetValue);
			/*
			MP_DEBUG("dwSecTor = %d    dwBufferAddress=0x%x",i, dwBuffAddr);
			SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
			if ((stopRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
			{
				MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
				return stopRetValue;
			}
			*/
			return swRetValue;
		}

		if((swRetValue = WaitTime1INT()))
		{
			MP_DEBUG1("-E- GetINT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		if ((swRetValue = GetIntValue(&bIntValue)))
		{
			MP_DEBUG1("-E- GetINT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		if ((bIntValue & CMDNK) || ((bIntValue & BREQ) && (bIntValue & ERR)))
		{
			MP_DEBUG1("-E- CMD Error termination (bIntValue: %d)", bIntValue);
			//Set Parameter Register
			/*
			SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
			if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
			{
				MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}
			*/
			return FAIL;
		}
		WaitDataTransfer();
		// force update cache
		SetDataCacheInvalid();              
		dwBufferAddress += MCARD_SECTOR_SIZE;
	}

	return PASS;
}

static SWORD Pro2KLogicalRead(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress)
{
	register MCARD *sMcard;
	register DWORD i;
	register SWORD swRetValue,stopRetValue;
	BYTE bIntValue;

	sMcard = (MCARD *) (MCARD_BASE);
	sMcard->McMsT1 = PRO_TIMEOUT1;
	//Set Parameter Register
	SetProParaReg(bProFlashCMD[PRO_READ_2K_DATA], dwSectorCount, dwLogAddr, 0x00, 0x00);
       //MP_DEBUG("EX_SET_CMD  count = %d dwBufferAddress = 0x%x",dwSectorCount,dwBufferAddress);
       if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
	{
		MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}
	if ((swRetValue = WaitTime1INT()))
	{
		MP_DEBUG1("-E- EX WaitTime1INT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = GetIntValue(&bIntValue)))
	{
		MP_DEBUG1("-E- EX GetINT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}
	if ((bIntValue & CMDNK) || ((bIntValue & BREQ) && (bIntValue & ERR)))
	{
		MP_DEBUG1("-E- EX CMD Error termination (bIntValue: %d)", bIntValue);
		//Set Parameter Register
		/*
		SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
		if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
		{
			MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		*/
		return FAIL;
	}

	for (i = 0; i < dwSectorCount; i+=4)
	{
		Set2KDataDma(dwBufferAddress);
		if ((swRetValue = ReadTpcBsPage(ACCESS_2K_DATA)))
		{
		       MP_DEBUG1("-E- ReadTpcBsPage FAIL (swRetValue: %d)", swRetValue);
                     /*
                     MP_DEBUG("dwSecTor = %d    dwBufferAddress=0x%x",i, dwBuffAddr);
		       SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
                     if ((stopRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
			{
				MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
				return stopRetValue;
			}
			*/
			return swRetValue;
		}

		if((swRetValue = WaitTime1INT()))
		{
			MP_DEBUG1("-E- GetINT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		if ((swRetValue = GetIntValue(&bIntValue)))
		{
			MP_DEBUG1("-E- GetINT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		if ((bIntValue & CMDNK) || ((bIntValue & BREQ) && (bIntValue & ERR)))
		{
                    MP_DEBUG1("-E- CMD Error termination (bIntValue: %d)", bIntValue);
                    //Set Parameter Register
                    /*
			SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
			if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
			{
				MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}
			*/
			return FAIL;
		}
		WaitDataTransfer();
              // force update cache
              SetDataCacheInvalid();   
		dwBufferAddress += (MCARD_SECTOR_SIZE<<2);
	}
	return PASS;
}

static SWORD ProReadAttribute(DWORD dwBufferAddress, DWORD dwLogAddr)
{
	register MCARD *sMcard;
	SWORD swRetValue;
	BYTE bIntValue;

	sMcard = (MCARD *) (MCARD_BASE);
	sMcard->McMsT1 = PRO_TIMEOUT1;

	//Set Parameter Register 
	SetProParaReg(bProFlashCMD[PRO_READ_ATRB], 0x01, dwLogAddr, 0x00, 0x00);

	if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
	{
		MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = WaitTime1INT()))
	{
		MP_DEBUG1("-E- WaitTime1INT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = GetIntValue(&bIntValue)))
	{
		MP_DEBUG1("-E- GetINT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((bIntValue & CMDNK) || ((bIntValue & BREQ) && (bIntValue & ERR)))
	{
		//Set Parameter Register
		MP_DEBUG1("-E- Error termination (bIntValue: %d)", bIntValue);
		SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
		if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
		{
			MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		return FAIL;
	}

	SetDataDma(dwBufferAddress);

	if ((swRetValue = ReadTpcBsPage(ACCESS_LONG_DATA)))
	{
		MP_DEBUG1("-E- ReadTpcBsPage FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = WaitTime1INT()))
	{
		MP_DEBUG1("-E- WaitTime1INT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = GetIntValue(&bIntValue)))
	{
		MP_DEBUG1("-E- GetINT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}
	WaitDataTransfer();
	return PASS;
}

#else
/*
static SWORD ProLogicalWrite(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress)
{
	register MCARD *sMcard;
	DWORD i;
	SWORD swRetValue;
	BYTE bIntValue;
	DWORD dwMcMsStatusTemp;

	sMcard = (MCARD *) (MCARD_BASE);
	sMcard->McMsT1 = PRO_TIMEOUT1;
	//Set Parameter Register
	SetProParaReg(bProFlashCMD[PRO_WRITE_DATA], dwSectorCount, dwLogAddr, 0x00, 0x00);
	if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
	{
		MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = Wait_For_INT_Pulse(&dwMcMsStatusTemp)))
	{	
		MP_DEBUG1("-E- Wait_For_INT_Pulse FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}
	if ((swRetValue = Wait_INT_Signal_BREQ(&dwMcMsStatusTemp)))
	{
		MP_DEBUG1("-E- Wait_INT_Signal_BREQ FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}	
	if(dwMcMsStatusTemp & (STATUS_ERR || STATUS_CMDNK))
	{
		if ((swRetValue = GetIntValue(&bIntValue)))
		{
			MP_DEBUG1("-E- GetINT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		if ((bIntValue & CMDNK) || ((bIntValue & BREQ) && (bIntValue & ERR)))
		{
			//Set Parameter Register
			MP_DEBUG1("-E- Error termination (bIntValue: %d)", bIntValue);
			SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
			if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
			{
				MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}
			return FAIL;
		}
		else if (((bIntValue & CED) && (bIntValue & ERR) && (bIntValue & CMDNK)))
		{
			MP_DEBUG1("-E- Write disabled status (bIntValue: %d)", bIntValue);
			return FAIL;
		}
	}
	for (i = 0; i < dwSectorCount; i++)
	{
		SetDataDma(dwBufferAddress);
		if ((swRetValue = WriteTpcBsPage(ACCESS_LONG_DATA)))
		{
			MP_DEBUG1("-E- WriteTpcBsPage FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		if ((swRetValue = Wait_INT_Signal(&dwMcMsStatusTemp)))
		{
			MP_DEBUG1("-E- Wait_INT_Signal FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		if(i !=(dwSectorCount-1))
		{
			if ((swRetValue = Wait_INT_Signal_BREQ(&dwMcMsStatusTemp)))
			{
					MP_DEBUG1("-E- Wait_INT_Signal_BREQ FAIL (swRetValue: %d)", swRetValue);
					return swRetValue;
			}
		}
		else
		{
			if ((swRetValue = Wait_INT_Signal_CED(&dwMcMsStatusTemp)))
			{
					MP_DEBUG1("-E- Wait_INT_Signal_CED FAIL (swRetValue: %d)", swRetValue);
					return swRetValue;
			}
		}
if(dwMcMsStatusTemp & (STATUS_ERR || STATUS_CMDNK))
{
		if ((swRetValue = GetIntValue(&bIntValue)))
		{
			MP_DEBUG1("-E- GetINT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		if ((bIntValue & CMDNK) || ((bIntValue & BREQ) && (bIntValue & ERR)))
		{
			//Set Parameter Register
			MP_DEBUG1("-E- Error termination (bIntValue: %d)", bIntValue);
			SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
			if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
			{
				MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}
			return FAIL;
		}
		else if (((bIntValue & CED) && (bIntValue & ERR) && (bIntValue & CMDNK)))
		{
			MP_DEBUG1("-E- Write disabled status (bIntValue: %d)", bIntValue);
			return FAIL;
		}
}		
		
		dwBufferAddress += MCARD_SECTOR_SIZE;
	}
	return PASS;
}

static SWORD ProLogicalRead(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress)
{
	register MCARD *sMcard;
	register DWORD i;
	register SWORD swRetValue;
	BYTE bIntValue;
	DWORD dwMcMsStatusTemp;

	sMcard = (MCARD *) (MCARD_BASE);
	sMcard->McMsT1 = PRO_TIMEOUT1;

	//Set Parameter Register
	SetProParaReg(bProFlashCMD[PRO_READ_DATA], dwSectorCount, dwLogAddr, 0x00, 0x00);

	if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
	{
		MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}
	if ((swRetValue = Wait_For_INT_Pulse(&dwMcMsStatusTemp)))
	{
		MP_DEBUG1("-E- Wait_For_INT_Pulse FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}
	if ((swRetValue = Wait_INT_Signal_BREQ(&dwMcMsStatusTemp)))
	{
		MP_DEBUG1("-E- Wait_INT_Signal_BREQ FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}
	if(dwMcMsStatusTemp & (STATUS_ERR ||STATUS_CMDNK))
	{
		if ((swRetValue = GetIntValue(&bIntValue)))
		{
			MP_DEBUG1("-E- GetINT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		if ((bIntValue & CMDNK) || ((bIntValue & BREQ) && (bIntValue & ERR)))
		{
			MP_DEBUG1("-E- Error termination (bIntValue: %d)", bIntValue);
			//Set Parameter Register

                     SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
			if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
			{
				MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}
			return FAIL;
		}
	}
	for (i = 0; i < dwSectorCount; i++)
	{
		SetDataDma(dwBufferAddress);
		if ((swRetValue = ReadTpcBsPage()))
		{
			MP_DEBUG1("-E- ReadTpcBsPage FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		if ((swRetValue = Wait_INT_Signal(&dwMcMsStatusTemp)))
		{
			MP_DEBUG1("-E- Wait_INT_Signal FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		if(i !=(dwSectorCount-1))
		{
			if ((swRetValue = Wait_INT_Signal_BREQ(&dwMcMsStatusTemp)))
			{
					MP_DEBUG1("-E- Wait_INT_Signal_BREQ FAIL (swRetValue: %d)", swRetValue);
					return swRetValue;
			}
		}
		else
		{
			if ((swRetValue = Wait_INT_Signal_CED(&dwMcMsStatusTemp)))
			{
					MP_DEBUG1("-E- Wait_INT_Signal_CED FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		}
		if(dwMcMsStatusTemp & (STATUS_ERR ||STATUS_CMDNK))
		{
			if ((swRetValue = GetIntValue(&bIntValue)))
			{	
				MP_DEBUG1("-E- GetINT FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}

			if ((bIntValue & CMDNK) || ((bIntValue & BREQ) && (bIntValue & ERR)))
			{
				//Set Parameter Register
				SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
				if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
				{
					MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
					return swRetValue;
				}
				return FAIL;
			}
		}		
		dwBufferAddress += MCARD_SECTOR_SIZE;
	}
	return PASS;
}

static SWORD ProReadAttribute(DWORD dwBufferAddress)
{
	register MCARD *sMcard;
	SWORD swRetValue;
	BYTE bIntValue;
	DWORD dwMcMsStatusTemp;
	sMcard = (MCARD *) (MCARD_BASE);
	sMcard->McMsT1 = PRO_TIMEOUT1;

	//Set Parameter Register 
	SetProParaReg(bProFlashCMD[PRO_READ_ATRB], 0x01, 0x00, 0x00, 0x00);

	if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
	{
		MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = Wait_For_INT_Pulse(&dwMcMsStatusTemp)))
	{
		MP_DEBUG1("-E- WaitTime1INT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	
	if(dwMcMsStatusTemp & (STATUS_ERR ||STATUS_CMDNK))
	{
		if ((swRetValue = GetIntValue(&bIntValue)))
		{
			MP_DEBUG1("-E- GetINT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
	
		if ((bIntValue & CMDNK) || ((bIntValue & BREQ) && (bIntValue & ERR)))
		{
			//Set Parameter Register
			MP_DEBUG1("-E- Error termination (bIntValue: %d)", bIntValue);
			SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
			if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
			{
				MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}
			return FAIL;
		}
	}

	
	SetDataDma(dwBufferAddress);

	if ((swRetValue = ReadTpcBsPage()))
	{
		MP_DEBUG1("-E- ReadTpcBsPage FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = Wait_For_INT_Pulse(&dwMcMsStatusTemp)))
	{
		MP_DEBUG1("-E- WaitTime1INT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}
	
	if(dwMcMsStatusTemp & (STATUS_ERR||STATUS_CMDNK))
	{
		if ((swRetValue = GetIntValue(&bIntValue)))
		{
			MP_DEBUG1("-E- GetINT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
	}
	return PASS;
}
*/
#endif


static SWORD ChangeBusWidth(WORD wBusWidth)
{
	SWORD swRetValue;
	BYTE bSysParameter;

	SetRWRegTpcAddr(0x00, 0x00, 0x10, 0x01);
	if (wBusWidth == BUS_WIDTH_4)
	{
		bSysParameter = 0x00;
	}
	else if(wBusWidth == BUS_WIDTH_1)
	{
		bSysParameter = 0x80;
	}
       else if(wBusWidth == BUS_WIDTH_8)
       {
            if((sInfo.wBusWidth == BUS_WIDTH_4)&&(sInfo.bMsPro == TYPE_MS_PRO_HG))  //MS Pro HG
            {
                bSysParameter = 0x40; 
            }
            else
            {
                 MP_DEBUG("-E- not 4 bits to 8 bits or not MsPro HG");
                 return FAIL;
            }
       }
       else 
       {
            MP_DEBUG("-E- Parameter Error ");
            __asm("break 100");
       }

	if ((swRetValue = WriteTpcBs(TPC_SET_RW_REG_ADR, (BYTE *) (&sRWRegTpcAddr), 4)))
	{
		MP_DEBUG1("-E- TPC_SET_RW_REG_ADR FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}
	
	mpDebugPrint("mode = 0x%x   wBusWidth = 0x%x",bSysParameter,wBusWidth);
	if ((swRetValue = WriteTpcBs(TPC_WRITE_REG, (BYTE *) (&bSysParameter), 1)))
	{
		MP_DEBUG1("-E- TPC_WRITE_REG FAIL (swRetValue: %d)", swRetValue);
		mpDebugPrint("-E- Interface switch fail!!");
		return SWITCH_FAIL;
	}
	if (wBusWidth == BUS_WIDTH_4)
	{
		sInfo.wBusWidth = BUS_WIDTH_4;
		bMS_CLOCK = MS_4BIT_CLK_KHZ;
		SetMcardClock(bMS_CLOCK);
	}
	else if(wBusWidth == BUS_WIDTH_1)
	{
		sInfo.wBusWidth = BUS_WIDTH_1;
		bMS_CLOCK = MS_1BIT_CLK_KHZ;
		SetMcardClock(bMS_CLOCK);
	}
	else if ( wBusWidth == BUS_WIDTH_8)
	{
		sInfo.wBusWidth = BUS_WIDTH_8;
		bMS_CLOCK = MS_8BIT_CLK_KHZ;
		SetMcardClock(bMS_CLOCK);
	}

	MP_DEBUG("--MsBusWidthClock bus=0x%x, clockId=%d", sInfo.wBusWidth , bMS_CLOCK);
	return PASS;
}


/*
static SWORD WaitDMAEnd(void)
{
	CHANNEL *sChannel;
	DWORD dwTimeOut;

	sChannel = (CHANNEL *) (DMA_MC_BASE);
	dwTimeOut = 0x00100000;
	while ((sChannel->Control & MCARD_DMA_ENABLE))
	{
		if (Polling_MS_Status())
			return FAIL;
		if (dwTimeOut == 0)
		{
			return DMA_TIMEOUT;
		}
		dwTimeOut--;
		TASK_YIELD();
	}
	return PASS;
}
*/

static DWORD WaitCmdEnd(void)
{
	MCARD *sMcard;
	DWORD dwTimeOut;

	sMcard = (MCARD *) (MCARD_BASE);
	dwTimeOut = 0x001000;
#ifdef MS_IRQ
	while (!McardGetInt())
	{
		if (Polling_MS_Status())
			return FAIL;
		if (sMcard->
			McMsStatus & (STATUS_TIMEOUT1 | STATUS_TIMEOUT0 | STATUS_CRCERROR | STATUS_CMDEND))
		{
			break;
		}

		if (dwTimeOut == 0)		// abel add
		{
			sMcard->McMsStatus = STATUS_TIMEOUT1;
			break;
		}

		dwTimeOut--;
		TASK_YIELD();
	}

	sMcard->McMsIc = 0;
	SystemIntDis(IC_MCARD);
	McardClrInt();
#else
	/* wait FIFO empty then check command end  */
	//while( (sMcard->McMsStatus & STATUS_FIFOEMPTY) != STATUS_FIFOEMPTY );
	while((sMcard->McMsStatus & STATUS_CMDEND) != STATUS_CMDEND);	
#endif

	return sMcard->McMsStatus;
}

static SWORD GetIntValue(BYTE * bIntValue)
{
	register SWORD swRetValue;
	register BYTE bTemp;
	register MCARD *sMcard;

	sMcard = (MCARD *) (MCARD_BASE);
	if( sInfo.wBusWidth == BUS_WIDTH_1 )
	{
		if ((swRetValue = ReadTpcBs(TPC_GET_INT, bIntValue, 1)))
		{
			return swRetValue;
		}
	}
	else if( (sInfo.wBusWidth == BUS_WIDTH_4)||(sInfo.wBusWidth == BUS_WIDTH_8) )
	{
		*bIntValue = 0;
		if(sMcard->McMsStatus & (STATUS_CED|STATUS_ERR|STATUS_BREQ|STATUS_CMDNK))
		{
			*bIntValue += (BYTE)((sMcard->McMsStatus & STATUS_CED)>>11);
			*bIntValue += (BYTE)((sMcard->McMsStatus & STATUS_ERR)>>13);
			*bIntValue += (BYTE)((sMcard->McMsStatus & STATUS_BREQ)>>15);
			*bIntValue += (BYTE)((sMcard->McMsStatus & STATUS_CMDNK)>>21);
			//mpDebugPrint("bIntValue = 0x%x", *bIntValue);
		}
		else
		{   
			MP_DEBUG("INT TEST");
			swRetValue = ReadTpcBs(TPC_GET_INT,bIntValue, 1);
			__asm("break 100");
			if( *bIntValue & CED )
			{
				swRetValue = ReadTpcBs(TPC_GET_INT, bIntValue, 1);
				bTemp = *bIntValue ;
				swRetValue = ReadTpcBs(TPC_GET_INT, bIntValue, 1);

				if( *bIntValue == bTemp)
					return PASS;
				else 
					return swRetValue;
			}
		}
	}
	return PASS;
}


/*
static SWORD Wait_INT_Signal(DWORD *dwMcMsStatusTemp)
{
	register MCARD *sMcard;
	DWORD dwTimeOut, i;

	sMcard = (MCARD *) (MCARD_BASE);
	for (dwTimeOut = 0; dwTimeOut < 0x10000000; dwTimeOut++)
	{
		*dwMcMsStatusTemp = sMcard->McMsStatus;
		if (*dwMcMsStatusTemp & STATUS_CMDINT)
		{
//			MP_DEBUG1("Wait_INT_Signal McMsStatus = %x",*dwMcMsStatusTemp);
			return PASS;
		}
		TASK_YIELD();
	}
	return TPC_INT_TIMEOUT;
}
*/



static SWORD Wait_INT_Signal_BREQ(DWORD *dwMcMsStatusTemp)
{
	register MCARD *sMcard;
	DWORD dwTimeOut, i;

	sMcard = (MCARD *) (MCARD_BASE);
	for (dwTimeOut = 0; dwTimeOut < 0x10000000; dwTimeOut++)
	{
		*dwMcMsStatusTemp = sMcard->McMsStatus;
		if (*dwMcMsStatusTemp & STATUS_BREQ)
		{
//			MP_DEBUG1("Wait_INT_Signal_BREQ McMsStatus = %x",*dwMcMsStatusTemp);
			return PASS;
		}
//		if(MsWrite == TRUE)
//			McardIODelay(50); // rick ,if remove will cause write fail

		TASK_YIELD();
	}

	return TPC_INT_TIMEOUT;
}


/*
static SWORD Wait_INT_Signal_CED(DWORD *dwMcMsStatusTemp)
{
	register MCARD *sMcard;
	DWORD dwTimeOut, i;

	sMcard = (MCARD *) (MCARD_BASE);
	for (dwTimeOut = 0; dwTimeOut < 0x10000000; dwTimeOut++)
	{
		*dwMcMsStatusTemp = sMcard->McMsStatus;
		if (*dwMcMsStatusTemp & STATUS_CED)
		{
//			MP_DEBUG1("Wait_INT_Signal_CED McMsStatus = %x",*dwMcMsStatusTemp);
			return PASS;
		}
//		if(MsWrite == TRUE)
//			McardIODelay(50); // rick ,if remove will cause write fail

		TASK_YIELD();
	}
	return TPC_INT_TIMEOUT;
}
*/


// Use hardware timeout (time1 setting)
// used at TPC_SET_CMD(TPC_EX_SET_CMD)

static SWORD WaitTime1INT(void)
{
	MCARD *sMcard;
	DWORD dwTimeout;

	sMcard = (MCARD *) (MCARD_BASE);
	dwTimeout = 0x01000000;

	//while (!(sMcard->McMsStatus & (STATUS_TIMEOUT1 | STATUS_CMDINT)))
	while(!(sMcard->McMsStatus & STATUS_CMDINT))
	{
		if (Polling_MS_Status())
			return FAIL;
		if (dwTimeout == 0)
		{
			return TPC_INT_TIMEOUT;
		}
		//UartOutText(">");
		//McardIODelay(1);
		dwTimeout--;
	}

	return PASS;
}

/*
static SWORD Wait_For_INT_Pulse(DWORD *dwMcMsStatusTemp)//rick ms pro faster 7/4
{
	MCARD *sMcard;
	DWORD dwTimeout;
	sMcard = (MCARD *) (MCARD_BASE);
	dwTimeout = 0x01000000;
	while (!((*dwMcMsStatusTemp = sMcard->McMsStatus) & (STATUS_TIMEOUT1 | STATUS_CMDINT)))
	{
		if (Polling_MS_Status())
			return FAIL;
		if (dwTimeout == 0)
		{
			return TPC_INT_TIMEOUT;
		}
//		McardIODelay(1);
		dwTimeout--;
	}

	return PASS;
}
*/


//Memory Stick Write TPC Bus State (none page mode)
static SWORD WriteTpcBs(register BYTE bTpc, register BYTE * pbBuffer, register BYTE bLen)
{
	register MCARD *sMcard;
	register DWORD dwTemp, dwTimeout;
	register BYTE i;
	register DWORD retry = RETRY_COUNT;

	sMcard = (MCARD *) (MCARD_BASE);
	//if (bTpc == TPC_WRITE_REG)    __asm("break 100");
	//MP_DEBUG("write bTpc = 0x%x   buswidth=0x%x , bLen= %d", bTpc, sInfo.wBusWidth,bLen);
	while ( retry )
	{
		sMcard->McMsT0 &=0x7FFFFFFF;
		sMcard->McardC = 0;
		sMcard->McMsT0 |=0x80000000;
		retry--;
		sMcard->McardC = MCARD_DMA_DIR_MC + sInfo.wBusWidth + (MCARD_FIFO_EMPTY | MCARD_MS_ENABLE);	// enable MS function
		// Set int mask
		sMcard->McMsIc = 0;
#ifdef MS_IRQ
		sMcard->McMsIc = (PL_ALL_HIGH | IM_CMDEND | IM_TIMEOUT1 | IM_INVALIDCMD);	// tpc cmd end, INT time out, invalid tpc cmd
		SystemIntEna(IC_MCARD);
#else
		SystemIntDis(IC_MCARD);
#endif
		if( bLen > 16 ) // fifo have 4 registers (16 Bytes), fill tpc first then fifo
		{
			MP_DEBUG("shit");
			sMcard->McMsCmd = bTpc + DATA_PATH_BY_CPU + PAGE_MOD_BY_DATA_LEN + (bLen << 8);

			// Wait write request
			dwTimeout = 0x800000;
#if 1 //rick ms pro fast , 7/25
			while (!(sMcard->McMsStatus & STATUS_HWRREQ))
			{
				if (Polling_MS_Status()) return FAIL;
				if (dwTimeout == 0)
				{
					//return TPC_RB_TIMEOUT;
					return COUNT_TIME_OUT;
				}
				dwTimeout--;
			}
#endif    
		}
		for (i = 0; i < bLen; i = i + 4)
		{
			dwTemp = (pbBuffer[i] << 24) + (pbBuffer[i + 1] << 16) + (pbBuffer[i + 2] << 8) + (pbBuffer[i + 3]);
			sMcard->McMsFifo = dwTemp;

			//IODelay(100);
			if( bLen > 16 ) // fifo have 4 registers (16 Bytes)
			{
				dwTimeout = 0x00100000;
				while (sMcard->McMsStatus & STATUS_FIFOFULL)
				{
					if (Polling_MS_Status()) return FAIL;
					if (dwTimeout == 0)
					{
						return COUNT_TIME_OUT;
					}
					dwTimeout--;
				}
			}
		}
		//MP_DEBUG("tpc = %x  bLen = %d",bTpc, bLen); 


		if( bLen <= 16 ) // fifo have 4 registers (16 Bytes), fill fifo first then tpc.
		{
			sMcard->McMsCmd = bTpc + DATA_PATH_BY_CPU + PAGE_MOD_BY_DATA_LEN + (bLen << 8);
		}

		// Wait cmd end
		IODelay(5);      // wait for 25 clk delay after tpc command send to  avoid the CMDEND not be set    T3-013
		dwTemp = WaitCmdEnd();
		if (sMcard->McMsStatus & STATUS_TIMEOUT0)   // check for Rdy/Bsy Timeout
		{
			MP_DEBUG("W Rdy/Bsy timeout, count=%d  blen=%d", retry, bLen);
			if( !retry )
			{
				//MP_DEBUG("1");
				return TPC_RB_TIMEOUT;
			}
			else
			{
				continue;
			}
		}
		else
		{
			//MP_DEBUG("2");
			return PASS;
		}
	}
}


//Memory Stick Read TPC Bus State (none page mode)
static SWORD ReadTpcBs(register BYTE bTpc, register BYTE * pbBuffer, register BYTE bLen)
{
	register MCARD *sMcard;
	register DWORD dwTemp, dwTimeout;
	register WORD i;
	register WORD retry = RETRY_COUNT;

	sMcard = (MCARD *) (MCARD_BASE);
	//MP_DEBUG("read bTpc = 0x%x   buswidth=0x%x ", bTpc, sInfo.wBusWidth);
	while( retry )
	{
		sMcard->McMsT0 &=0x7FFFFFFF;
		sMcard->McardC = 0;
		sMcard->McMsT0 |=0x80000000;
		//IODelay(100);
		//MP_DEBUG("kuku");
		retry--;
		// Mcard Control Register Set     
		sMcard->McardC = MCARD_DMA_DIR_CM + sInfo.wBusWidth + (MCARD_FIFO_EMPTY | MCARD_MS_ENABLE);		

		// Time out count    Wait for FIFOIDLE
		dwTimeout = 0x100000;
#if 1//rick ms pro fast , 7/25
		while (!(sMcard->McMsStatus & STATUS_FIFOIDLE))	// Wait fifo idle
		{
			if (Polling_MS_Status()) return FAIL;
			if (dwTimeout == 0)
			{
				//return TPC_RB_TIMEOUT;
				return COUNT_TIME_OUT;
			}
			dwTimeout--;
		}
#endif
		// Set int mask
		sMcard->McMsIc = 0;
		//Mask Mcard Int
#ifdef MS_IRQ
		sMcard->McMsIc = (PL_ALL_HIGH | IM_CMDEND | IM_TIMEOUT1 | IM_INVALIDCMD | IM_HRDREQ);	// read req, tpc cmd end, INT time out, invalid tpc cmd
		SystemIntEna(IC_MCARD);
#else
		SystemIntDis(IC_MCARD);
#endif
		// Set Mcard   McMsCmd Register to send TPC  	
		sMcard->McMsCmd = bTpc + DATA_PATH_BY_CPU + PAGE_MOD_BY_DATA_LEN + (bLen << 8);

		// Wait Commnad End  ( polling McMsStatus STATUS_CMDEND )
		IODelay(5);    // wait for 25 clk delay after tpc command send to  avoid the CMDEND not be set   T3-013
		dwTemp = WaitCmdEnd();
		if( sMcard->McMsStatus & STATUS_TIMEOUT0 )     // check for Rdy/Bsy Timeout
		{
			MP_DEBUG("R Rdy/Bsy  Timeout");
			if( !retry )
				return  TPC_RB_TIMEOUT;
			else
				continue;
		}              
		else if ( sMcard->McMsStatus & STATUS_CRCERROR )   // check for CRC ERROR
		{
			MP_DEBUG("R CRC Error!");
			if( !retry )
				return  READ_DATA_CRC_ERROR;
			else
				continue;
		}              
		else
		{
			//Read Data from FIFO
			i = 0;
			while (i < bLen)
			{
				if (!(i & 0x03))
				{
					//Wait for FIFO  Empty
					dwTimeout = 0x100000;
					while (sMcard->McMsStatus & STATUS_FIFOEMPTY)
					{
						if (Polling_MS_Status()) return FAIL;
						if (dwTimeout == 0)
						{
							return COUNT_TIME_OUT;
						}
						dwTimeout--;
					}
					dwTemp = sMcard->McMsFifo;
				}
				pbBuffer[i] = (BYTE) ((dwTemp & 0xff000000) >> 24);
				dwTemp <<= 8;
				i++;
			}
			return PASS;
		}
	}
}

//Memory Stick Read TPC Bus State (page mode)
#if 0
static SWORD ReadTpcBsPage(void)
{
	register MCARD *sMcard;
	register SWORD swRetValue;
	register DWORD dwTemp, dwTimeout;
       register DWORD retry = RETRY_COUNT;
       register CHANNEL *sChannel;
	sMcard = (MCARD *) (MCARD_BASE);
       sChannel = (CHANNEL *)(DMA_MC_BASE);
       while( retry )
        {
	     sMcard->McMsT0 &=0x7FFFFFFF;
	     sMcard->McardC = 0;
            sMcard->McMsT0 |=0x80000000;
            retry--;
            sChannel->Control = 0;             //while retry, reset DMA control disable  T3-017       
            sMcard->McDmarl = 0;
	sMcard->McardC = MCARD_DMA_DIR_CM + sInfo.wBusWidth + (MCARD_FIFO_EMPTY | MCARD_MS_ENABLE);		
	sMcard->McDmarl = MCARD_DMA_LIMIT_ENABLE + (MCARD_SECTOR_SIZE >> 2);
	sMcard->McardC |= (MCARD_FIFO_ENABLE);
       sChannel->Control = MCARD_DMA_ENABLE;    //while retry, reset DMA control enable   T3-017
        // Wait fifo idle
	dwTimeout = 0x100000;
	#if 1//rick ms pro fast , 7/25
	while (!(sMcard->McMsStatus & STATUS_FIFOIDLE))
	{
            //if (Polling_MS_Status()) __asm("break 100");//return FAIL;
		if (dwTimeout == 0)
		{
			//return TPC_RB_TIMEOUT;
			return COUNT_TIME_OUT;
		}
		dwTimeout--;
	}
	#endif
	// Set int mask
	sMcard->McMsIc = 0;
	#ifdef MS_IRQ
	sMcard->McMsIc = (PL_ALL_HIGH | IM_CMDEND | IM_TIMEOUT1 | IM_INVALIDCMD);	// tpc cmd end, INT time out, invalid tpc cmd
	SystemIntEna(IC_MCARD);
	#else
	SystemIntDis(IC_MCARD);
	#endif
	sMcard->McMsCmd = TPC_READ_LONG_DATA + DATA_PATH_BY_DMA + PAGE_MOD_512_BYTE;

	// Wait cmd end	
	IODelay(5);    // wait for 25 clk delay after tpc command send to  avoid the CMDEND not be set   T3-013
	dwTemp = WaitCmdEnd();// this procedure can not remove
       if (sMcard->McMsStatus & STATUS_TIMEOUT0)
	{
	       MP_DEBUG("R PAGE Rdy/Bsy timeout");
	       if( !retry)
		    return TPC_RB_TIMEOUT;
              else
                 continue;
	}
	// Wait dma done
/*	
	if ((swRetValue = WaitDMAEnd())) // can not remove or will affect MS read to wrong data
	{
	       //__asm("break 100"); 
		return swRetValue;
	}
*/    
	if ( sMcard->McMsStatus& STATUS_CRCERROR)
	{
	       MP_DEBUG("R page CRC Error ");
		return READ_DATA_CRC_ERROR;
	}
	else
	{
		return PASS;
	}
        }
}
#endif
static SWORD ReadTpcBsPage(BYTE access2kdata)
{
	register MCARD *sMcard;
	register SWORD swRetValue;
	register DWORD dwTemp, dwTimeout;
	register DWORD retry = RETRY_COUNT;
	register CHANNEL *sChannel;
	sMcard = (MCARD *) (MCARD_BASE);
	sChannel = (CHANNEL *)(DMA_MC_BASE);

	while( retry )
	{
		sMcard->McMsT0 &=0x7FFFFFFF;
		sMcard->McardC = 0;
		sMcard->McMsT0 |=0x80000000;
		retry--;
		sChannel->Control = 0;             //while retry, reset DMA control disable  T3-017       
		sMcard->McDmarl = 0;
		sMcard->McardC = MCARD_DMA_DIR_CM + sInfo.wBusWidth + (MCARD_FIFO_EMPTY | MCARD_MS_ENABLE);		

		if( access2kdata)
			sMcard->McDmarl = MCARD_DMA_LIMIT_ENABLE +  MCARD_SECTOR_SIZE;    //512  double word
		else
			sMcard->McDmarl = MCARD_DMA_LIMIT_ENABLE + (MCARD_SECTOR_SIZE >> 2);

       sMcard->McardC |= (MCARD_FIFO_ENABLE);
       sChannel->Control = MCARD_DMA_ENABLE;    //while retry, reset DMA control enable   T3-017
		// Wait fifo idle
		dwTimeout = 0x100000;
#if 1//rick ms pro fast , 7/25
		while (!(sMcard->McMsStatus & STATUS_FIFOIDLE))
		{
			//if (Polling_MS_Status()) __asm("break 100");//return FAIL;
			if (dwTimeout == 0)
			{
				//return TPC_RB_TIMEOUT;
				return COUNT_TIME_OUT;
			}
			dwTimeout--;
		}
#endif
		// Set int mask
		sMcard->McMsIc = 0;
#ifdef MS_IRQ
		sMcard->McMsIc = (PL_ALL_HIGH | IM_CMDEND | IM_TIMEOUT1 | IM_INVALIDCMD);	// tpc cmd end, INT time out, invalid tpc cmd
		SystemIntEna(IC_MCARD);
#else
		SystemIntDis(IC_MCARD);
#endif

		if( access2kdata )
			sMcard->McMsCmd = TPC_READ_QUAD_DATA + DATA_PATH_BY_DMA;
		else
			sMcard->McMsCmd = TPC_READ_LONG_DATA + DATA_PATH_BY_DMA + PAGE_MOD_512_BYTE;

		// Wait cmd end	
		IODelay(5);    // wait for 25 clk delay after tpc command send to  avoid the CMDEND not be set   T3-013
		dwTemp = WaitCmdEnd();// this procedure can not remove
		if (sMcard->McMsStatus & STATUS_TIMEOUT0)
		{
			MP_DEBUG("R PAGE Rdy/Bsy timeout");
			if( !retry)
				return TPC_RB_TIMEOUT;
			else
				continue;
		}
		// Wait dma done
		/*	
		if ((swRetValue = WaitDMAEnd())) // can not remove or will affect MS read to wrong data
		{
		//__asm("break 100"); 
		return swRetValue;
		}
		*/    
		if ( sMcard->McMsStatus& STATUS_CRCERROR)
		{
			MP_DEBUG("R page CRC Error ");
			return READ_DATA_CRC_ERROR;
		}
		else
		{
			return PASS;
		}
	}
}

//Memory Stick Write TPC Bus State (page mode)
static SWORD WriteTpcBsPage(BYTE access2kdata)
{
	register MCARD *sMcard;
	register SWORD swRetValue;
	register DWORD dwTemp;
	register DWORD dwTimeout;
	register CHANNEL *sChannel;
	register DWORD retry = RETRY_COUNT;

	sMcard = (MCARD *) (MCARD_BASE);
	sChannel = (CHANNEL *)(DMA_MC_BASE);
	//sMcard->McardC = 0x00000400;

	while( retry )
	{
		sMcard->McMsT0 &=0x7FFFFFFF;
		sMcard->McardC = 0;
		sMcard->McMsT0 |=0x80000000;
		retry--;
		sChannel->Control = 0;             //while retry, reset DMA control disable  T3-017
		sMcard->McardC = MCARD_DMA_DIR_MC + sInfo.wBusWidth + (MCARD_FIFO_EMPTY | MCARD_MS_ENABLE);		
		if (access2kdata )
			sMcard->McDmarl = MCARD_DMA_LIMIT_ENABLE + (MCARD_SECTOR_SIZE); 
		else
			sMcard->McDmarl = MCARD_DMA_LIMIT_ENABLE + (MCARD_SECTOR_SIZE >> 2);

		sMcard->McardC |= (MCARD_FIFO_ENABLE);
		sChannel->Control = MCARD_DMA_ENABLE;    //while retry, reset DMA control enable   T3-017
		// Set int mask
		sMcard->McMsIc = 0;
		sMcard->McMsIc = (PL_ALL_HIGH | IM_HWRREQ);
		if(access2kdata)
			sMcard->McMsCmd = TPC_WRITE_QUAD_DATA+ DATA_PATH_BY_DMA;   //set tpc WRITE_QUAD_DATA
		else
			sMcard->McMsCmd = TPC_WRITE_LONG_DATA + DATA_PATH_BY_DMA + PAGE_MOD_512_BYTE;    //set tpc WRITE_LONG_DATA

		// Wait cmd end
		dwTimeout = 0x100000;
#if 1//rick ms pro fast , 7/25
		while (!(sMcard->McMsStatus & (STATUS_TIMEOUT1 | STATUS_TIMEOUT0 | STATUS_CMDEND)))
		{
			if (Polling_MS_Status())
				return FAIL;
			if (dwTimeout == 0)
			{
				return TPC_RB_TIMEOUT;
			}
			dwTimeout--;
		}
#endif
		IODelay(5);       // wait for 25 clk delay after tpc command send to  avoid the CMDEND not be set    T3-013
		dwTemp = WaitCmdEnd(); 
		if (sMcard->McMsStatus & STATUS_TIMEOUT0)
		{  
			MP_DEBUG("w Rdy/Bsy timeout");
			if(!retry)
				return TPC_RB_TIMEOUT;
			else
				continue;
		}
		else
		{
/*	   
		  if ((swRetValue = WaitDMAEnd()))
		  {
		       MP_DEBUG1("-E- WaitDMAEnd FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		   }
*/		   
		return PASS;
	    }
        }
}




//Make Mapping Table Init Size 16(seg)*512(block)
//
//           0 ~ 495        |      496 ~ 511
//         Log2Phy mapping  |   Free block table
//
//ps: frist seg only use 494 blocks , reserved 2 block for boot block
static SWORD InitLog2PhyTable(void)
{
	register DWORD dwSegNum, dwPhyBlockAddr, dwLogBlockaddr, i;
	SWORD swRetValue;
	WORD wReAssignState, *wTempLogBufer, wTemp;
	register WORD *pwLog2PhyTable;
	WORD *pwReAssignTable;
	BYTE bDisableBlock[MCARD_SECTOR_SIZE], bExtraBuffer[4], bTempBuffer[4];
	BYTE *pbDisableBlock, *pbBuffer1, *pbBuffer2;

	pbBuffer1 = (BYTE *) ker_mem_malloc(2048, TaskGetId());
	
	if(pbBuffer1 == NULL)
		return FAIL;
	
	wTempLogBufer = (WORD *) ((DWORD) pbBuffer1);

	pwLog2PhyTable = sInfo.wLog2PhyTable;
	pwReAssignTable = sInfo.wReAssignTable;
	pbDisableBlock = (BYTE *) (((DWORD) (&bDisableBlock) | 0xa0000000));
	if ((swRetValue = PhysicalRead(sInfo.bBootBlock, 1, 1, (DWORD) pbDisableBlock)))
	{
		ker_mem_free(pbBuffer1);
		MP_DEBUG1("-E- PhysicalRead FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	// segment count    
	register DWORD dwSegBlockAddr = 0;
	register WORD *pwCurLog2PhyTable;

	for (dwSegNum = 0; dwSegNum < sInfo.dwSegment; dwSegNum++, dwSegBlockAddr += MAXPHYBLOCK)
	{
		DWORD dwLogAddrCheckValue = (dwSegNum * 496 + 494);

		//ver.1.00 spec A.8 Logical/physical corresponding information creation process
		//Init all Log2PhyTable(LPTable) and tmpLogBufer(FreeTable) to 0xffff
		pwCurLog2PhyTable = &pwLog2PhyTable[dwSegBlockAddr];
		for (dwPhyBlockAddr = 0; dwPhyBlockAddr < MAXPHYBLOCK; dwPhyBlockAddr++)
		{

			*pwCurLog2PhyTable = 0xffff;
			pwCurLog2PhyTable++;
			wTempLogBufer[dwPhyBlockAddr] = 0xffff;
		}

		//set 0 to wReAssignState(:Number of alternative[free] blocks)
		wReAssignState = 0;

		// Get all logical addr of one segment            
		// If segment 0, start at block 2 (first 2 block are reserved for boot block
		for (dwPhyBlockAddr = (!dwSegNum) ? (sInfo.bBootBlock + 1) : 0;
			 dwPhyBlockAddr < MAXPHYBLOCK; dwPhyBlockAddr++)
		{
			// Check if disabled block
			if (CheckDisableBlock((dwPhyBlockAddr + dwSegBlockAddr), pbDisableBlock))
			{
				continue;
			}

			// read extra data area on page 0 in block x
			// uncorrectable error occured
			if ((swRetValue =
				 ReadExtraData((dwSegBlockAddr + dwPhyBlockAddr), 0, (DWORD) bExtraBuffer)))
			{
				MP_DEBUG1("-E- ReadExtraData FAIL (swRetValue: %d)", swRetValue);

				// add for Payton one ms card will be identify to can not write issue
				if (sInfo.bWriteProtected != 1)
					PhysicalErase((dwSegBlockAddr + dwPhyBlockAddr));
				wTempLogBufer[wReAssignState] = (WORD) dwPhyBlockAddr;
				wReAssignState++;				
				
				continue;
			}

			//last segment
			//transformation table bit of block is 0
			if ((dwSegNum == (sInfo.dwSegment - 1)) && (!(bExtraBuffer[1] & 0x8)) && (sInfo.bWriteProtected != 1))
			{
				PhysicalErase((dwSegBlockAddr + dwPhyBlockAddr));
			}

			// BS of block x is 0(: NG)
			if (!(bExtraBuffer[0] & 0x80))
			{
				continue;
			}

			//PS of page 0 in block x is 3 or 6 ?? (: OK)
			if (!(bExtraBuffer[0] & 0x60))
			{
				continue;
			}

			//set dwLogBlockaddr to Logical Address read
			dwLogBlockaddr = (bExtraBuffer[2] << 8) + bExtraBuffer[3];

			//dwLogBlockaddr is 0xffff
			if (dwLogBlockaddr == 0xffff)
			{
				//erase block and register block on wTmpLogBufer(FreeTable)
				if (sInfo.bWriteProtected != 1)
					PhysicalErase((dwSegBlockAddr + dwPhyBlockAddr));
				wTempLogBufer[wReAssignState] = (WORD) dwPhyBlockAddr;
				wReAssignState++;
				continue;
			}
			//logical address is out of predefined segment range
			//note: segment number 0 is different to others
			if (dwSegNum)
			{
				//if ((dwLogBlockaddr < ((dwSegNum - 1) * 496 + 494)) || (dwLogBlockaddr > (dwSegNum * 496 + 494)))
				if ((dwLogBlockaddr < (dwLogAddrCheckValue - 496))
					|| (dwLogBlockaddr > dwLogAddrCheckValue))
				{
					//erase block and register block on wTmpLogBufer(FreeTable)
					if (sInfo.bWriteProtected != 1)
					{
						PhysicalErase((dwSegBlockAddr + dwPhyBlockAddr));
						wTempLogBufer[wReAssignState] = (WORD) dwPhyBlockAddr;
						wReAssignState++;
					}
					continue;
				}
				dwLogBlockaddr = (dwLogBlockaddr - 494) % 496;
			}
			else
			{
				if (dwLogBlockaddr > 493)
				{
					//erase block and register block on wTmpLogBufer(FreeTable)
					if (sInfo.bWriteProtected != 1)
					{
						PhysicalErase((dwSegBlockAddr + dwPhyBlockAddr));
						wTempLogBufer[wReAssignState] = (WORD) dwPhyBlockAddr;
						wReAssignState++;
					}
					continue;
				}
			}

			//Log2PhyTable(LPTable) is 0xffff
			//Check if the same logical addr with others
			pwCurLog2PhyTable = &pwLog2PhyTable[dwSegBlockAddr + dwLogBlockaddr];
			if (*pwCurLog2PhyTable == 0xffff)
			{
				*pwCurLog2PhyTable = (WORD) dwPhyBlockAddr;
			}
			else
			{
				MP_DEBUG("compare update status of Log2PhyTable");	
				//compare update status of Log2PhyTable(LPTable) and dwPhyBlockAddr
				ReadExtraData(*pwCurLog2PhyTable, 0, (DWORD) bTempBuffer);

				// update status is different
				if ((bTempBuffer[0] & 0x10) == (bExtraBuffer[0] & 0x10))
				{
					//Log2PhyTable(LPTable) > dwPhyBlockAddr
					if (*pwCurLog2PhyTable > (WORD) dwPhyBlockAddr)
					{
						//erase block and register block on wTmpLogBufer(FreeTable)
						if (sInfo.bWriteProtected != 1)
							PhysicalErase((dwSegBlockAddr + dwPhyBlockAddr));
						wTempLogBufer[wReAssignState] = (WORD) dwPhyBlockAddr;
						wReAssignState++;
					}
					else
					{
						//erase block and register block on wTmpLogBufer(FreeTable)
						if (sInfo.bWriteProtected != 1)
							PhysicalErase(*pwCurLog2PhyTable);
						wTempLogBufer[wReAssignState] = *pwCurLog2PhyTable;
						wReAssignState++;
						*pwCurLog2PhyTable = (WORD) dwPhyBlockAddr;
					}
				}
				else
				{
					//update staus of block dwPhyBlockAddr is 0
					if ((bExtraBuffer[0] & 0x10))
					{
						//erase block and register block on wTmpLogBufer(FreeTable)
						if (sInfo.bWriteProtected != 1)
						{
							PhysicalErase((dwSegBlockAddr + dwPhyBlockAddr));
							wTempLogBufer[wReAssignState] = (WORD) dwPhyBlockAddr;
							wReAssignState++;
						}
					}
					else
					{
						//erase block and register block on wTmpLogBufer(FreeTable)
						if (sInfo.bWriteProtected != 1)
							PhysicalErase(*pwCurLog2PhyTable);
						wTempLogBufer[wReAssignState] = *pwCurLog2PhyTable;
						wReAssignState++;
						*pwCurLog2PhyTable = (WORD) dwPhyBlockAddr;
					}
				}
			}
		}

		//ver.1.00 spec A.9 Logical Address confirmation process
		if (dwSegNum == (sInfo.dwSegment - 1))
		{
			if (wReAssignState < 2)
			{
				ker_mem_free(pbBuffer1);
				sInfo.bWriteProtected = 1;
					MP_DEBUG("write protect");	
				return PASS;
			}
		}
		else
		{
			if (wReAssignState < 1)
			{
				ker_mem_free(pbBuffer1);
				sInfo.bWriteProtected = 1;
				MP_DEBUG("write protect");	

				return PASS;
			}
		}

		//Insert free blaock address into Log2PhyTable
		for (i = 0; i < MAXPHYBLOCK; i++)
		{
			if ((pwLog2PhyTable[dwSegBlockAddr + i] == 0xffff) && (wReAssignState != 0))
			{
				if (dwSegNum == 0)
				{
					if ((i == 494) || (i == 495))
					{
						continue;
					}
				}
				if ((i < 496) && (!sInfo.bWriteProtected))
				{
					MP_DEBUG("Insert free blaock address into Log2PhyTable");
					WriteExtraData((wTempLogBufer[wReAssignState - 1] + (dwSegBlockAddr)), 0,
								   (DWORD) (dwSegBlockAddr + i));
				}
				pwLog2PhyTable[dwSegBlockAddr + i] = wTempLogBufer[wReAssignState - 1];
				wReAssignState--;
			}
		}
		pwReAssignTable[dwSegNum] = 0;

		MP_DEBUG1(" (dwSegNum: %d)", dwSegNum);

	}

	if ((sInfo.bWriteProtected == 1) && (sInfo.bProtectBlockCount != 0))
	{
		register WORD *pwTable = &sInfo.wProtectBlockTable[0];

		for (i = 0; i < sInfo.bProtectBlockCount; i++)
		{
			pwLog2PhyTable[*pwTable] = i;
			pwTable++;
		}
	}
	ker_mem_free(pbBuffer1);
	return PASS;
}

static SWORD CheckDisableBlock(register DWORD dwPhyAddr, register BYTE * pbDisableAddr)
{
	register WORD wDisableCount, wDisableBlock;

	wDisableCount = 0;
	do
	{
		if (Polling_MS_Status())
			return FAIL;
		wDisableBlock =
			(WORD) ((pbDisableAddr[wDisableCount] << 8) + pbDisableAddr[wDisableCount + 1]);
		if (dwPhyAddr == wDisableBlock)
		{
			return FAIL;
		}
		wDisableCount += 2;
	}
	while (wDisableBlock != 0xffff);
	return PASS;
}

static WORD GetEmptyBlock(DWORD dwSegNum)
{
	WORD wBlankAddr;
	WORD *pwLog2PhyTable, *pwReAssignTable;

	pwLog2PhyTable = sInfo.wLog2PhyTable;
	pwReAssignTable = sInfo.wReAssignTable;
	wBlankAddr = pwLog2PhyTable[dwSegNum * MAXPHYBLOCK + pwReAssignTable[dwSegNum] + 496];
	return wBlankAddr;
}

static WORD GetSameBlockPages(register DWORD dwLogAddr, register DWORD dwSectorCount)
{
	register WORD wPage;
	register DWORD dwPagePerBlock = sInfo.dwPagePerBlock;

	wPage = dwLogAddr % dwPagePerBlock;
	if (dwSectorCount > (dwPagePerBlock - wPage))
	{
		return (dwPagePerBlock - wPage);
	}
	else
	{
		return dwSectorCount;
	}
}

static SWORD IdentifyType(void)
{
	register SWORD swRetValue;
	BYTE abTempBuf[6];

	SetRWRegTpcAddr(0x02, 0x06, 0x10, 0x01);	//Set R/W Register TPC Addr

	if ((swRetValue = WriteTpcBs(TPC_SET_RW_REG_ADR, (BYTE *) &sRWRegTpcAddr, 4)))	// SET_RW_REG_ADRS
	{
		MP_DEBUG("WriteTPcBs fail...");
		return swRetValue;
	}
	// READ_REG
	if ((swRetValue = ReadTpcBs(TPC_READ_REG, abTempBuf, 6)))
	{
		MP_DEBUG("ReadTPcBs fail...");
		return swRetValue;
	}
	if (abTempBuf[2] == 0x00)
	{
		if (abTempBuf[4] >= 0x80)
		{
			MP_DEBUG("-E- ms type FAIL");
			return FAIL;
		}
		else if ((abTempBuf[4] >= 0x01) && (abTempBuf[4] <= 0x7F))
		{
			MP_DEBUG("-I- ms i/o expanded module");
			return TYPE_NOT_SUPPORT;
		}
		else if (abTempBuf[5] >= 0x04)
		{
			MP_DEBUG("-E- ms type FAIL");
			return FAIL;
		}
	}
	else if (abTempBuf[2] == 0x01)
	{
		if (abTempBuf[4] != 0x00)
		{
			MP_DEBUG("-E- ms type FAIL");
			return FAIL;
		}

		if (abTempBuf[5] == 0x00)
		{
			if (abTempBuf[0] & 0x1)
			{
				MP_DEBUG("-I- write protected");
				sInfo.bWriteProtected = 1;
			}
		}
		else if ((abTempBuf[5] == 0x01) || (abTempBuf[5] == 0x02) || (abTempBuf[5] == 0x03))
		{
			MP_DEBUG("-I- write protected");
			sInfo.bWriteProtected = 1;
		}
		else
		{
			MP_DEBUG("-E- ms type FAIL");
			return FAIL;
		}
        
		/* MS-Pro-HG 8-bit parallel I/F support */
		if( abTempBuf[3] == 0x07 ) /* IF Mode Register=111b : support serial, 4-bit and 8-bit parallel I/F mode */
		{
			sInfo.bMsPro = TYPE_MS_PRO_HG;		
			mpDebugPrint("-I- MS PRO-HG type");
		}
		else if( abTempBuf[3] == 0x00 )
		{
			sInfo.bMsPro = TYPE_MS_PRO;
			mpDebugPrint("-I- ms pro type");
		}
		else 
		{
			MP_DEBUG("-E- ms type fail");
			return FAIL;
		}
		return PASS;
	}
	else if (abTempBuf[2] == 0xFF)
	{
		if ((abTempBuf[4] == 0x00) || ((abTempBuf[4] >= 0x80) && (abTempBuf[4] <= 0xFE)))
		{
			MP_DEBUG("-E- ms type FAIL");
			return FAIL;
		}
		else if ((abTempBuf[4] >= 0x01) && (abTempBuf[4] <= 0x7F))
		{
			MP_DEBUG("-I- ms i/o expanded module");
			return TYPE_NOT_SUPPORT;
		}
		else if ((abTempBuf[5] == 0x00) || ((abTempBuf[5] >= 0x04) && (abTempBuf[4] <= 0xFE)))
		{
			MP_DEBUG("-E- ms type FAIL");
			return FAIL;
		}
	}
	else
	{
		MP_DEBUG("-E- ms type FAIL");
              Power_Off();                          //T7-006   2-7-5
		return FAIL;
	}


	if ((abTempBuf[5] >= 0x01) && (abTempBuf[5] <= 0x03))
	{					
		MP_DEBUG("-I- write protected");
		sInfo.bWriteProtected = 1;
	}

	if (abTempBuf[0] & 0x1)
	{
		MP_DEBUG("-I- write protected");
		sInfo.bWriteProtected = 1;
	}
	sInfo.bMsPro = TYPE_MS;
	mpDebugPrint("-I- ms type");
	return PASS;
}

// Get Logical addr of (blockAddr + pageNo) to buffer
static SWORD ReadExtraData(register DWORD dwBlockAddr, DWORD dwPageNumber, DWORD dwBufferAddress)
{
	register MCARD *sMcard;
	register SWORD swRetValue;
	BYTE bIntValue, bTempBuf;
	register BYTE *pbBuffer;

	pbBuffer = (BYTE *) dwBufferAddress;
	sMcard = (MCARD *) (MCARD_BASE);
	sMcard->McMsT1 = READ_TIMEOUT1;

	//Set R/W Register TPC Addr
	SetRWRegTpcAddr(0x16, 0x04, 0x10, 0x06);

	//Set Parameter Register 
	SetParaReg(0x80, dwBlockAddr, 0x40, dwPageNumber);

	// SET_RW_REG_ADRS
	if ((swRetValue = WriteTpcBs(TPC_SET_RW_REG_ADR, (BYTE *) (&sRWRegTpcAddr), 4)))
	{
		MP_DEBUG1("-E- TPC_SET_RW_REG_ADR FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	// WRITE_REG
	if ((swRetValue = WriteTpcBs(TPC_WRITE_REG, (BYTE *) (&sParamReg), 6)))
	{
		MP_DEBUG1("-E- TPC_WRITE_REG FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	// SET_CMD:BLOCK_READ
	if ((swRetValue = WriteTpcBs(TPC_SET_CMD, (BYTE *) (&bFlashCMD[BLOCK_READ]), 1)))
	{
		MP_DEBUG1("-E- TPC_SET_CMD FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = WaitTime1INT()))
	{
		MP_DEBUG1("-E- WaitTime1INT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = GetIntValue(&bIntValue)))
	{
		MP_DEBUG1("-E- GetIntValue FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if (bIntValue & CMDNK)
	{
		MP_DEBUG1("-E- Error termination (bIntValue: %d)", bIntValue);
	}
	else if ((bIntValue & CED) && (bIntValue & ERR))
	{
		MP_DEBUG1("-E- Flash read error occurred (bIntValue: %d)", bIntValue);

		//Set R/W Register TPC Addr
		SetRWRegTpcAddr(0x03, 0x01, 0x10, 0x06);

		// SET_RW_REG_ADRS
		if ((swRetValue = WriteTpcBs(TPC_SET_RW_REG_ADR, (BYTE *) (&sRWRegTpcAddr), 4)))
		{
			MP_DEBUG1("-E- TPC_SET_RW_REG_ADR FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		// READ_REG
		if ((swRetValue = ReadTpcBs(TPC_READ_REG, &bTempBuf, 1)))
		{
			MP_DEBUG1("-E- TPC_READ_REG FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		if ((bTempBuf & UCEX) || (bTempBuf & UCFG))
		{
			MP_DEBUG1("-E- uncorrectable error (bTempBuf: %d)", bTempBuf);
		}
	}
	else if (bIntValue & CED)
	{
		// Get Logical addr
		if ((swRetValue = ReadTpcBs(TPC_READ_REG, pbBuffer, 4)))
		{
			MP_DEBUG1("-E- TPC_READ_REG FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		return PASS;
	}
	return FAIL;
}

static SWORD PhysicalRead(register DWORD dwBlockAddr, register DWORD dwStartPage,
						  register DWORD dwSectorCount, register DWORD dwBufferAddress)
{
	register MCARD *sMcard;
	register DWORD dwTempBuf, dwCurPage;
	register SWORD swRetValue;
	BYTE bIntValue, bTempBuf;

	sMcard = (MCARD *) (MCARD_BASE);
	sMcard->McMsT1 = READ_TIMEOUT1;

	dwTempBuf = dwBufferAddress;

	//Set R/W Register TPC Addr
	SetRWRegTpcAddr(0x16, 0x04, 0x10, 0x06);

	//Set Parameter Register 
	if (dwSectorCount == 1)
	{
		SetParaReg(0x80, dwBlockAddr, 0x20, dwStartPage)
	}
	else
	{
		SetParaReg(0x80, dwBlockAddr, 0x00, dwStartPage)
	}

	// SET_RW_REG_ADRS
	if ((swRetValue = WriteTpcBs(TPC_SET_RW_REG_ADR, (BYTE *) & sRWRegTpcAddr, 4)))
	{
		MP_DEBUG1("-E- TPC_SET_RW_REG_ADR FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	// WRITE_REG	
	if ((swRetValue = WriteTpcBs(TPC_WRITE_REG, (BYTE *) & sParamReg, 6)))
	{
		MP_DEBUG1("-E- TPC_WRITE_REG FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	// SET_CMD:BLOCK_READ		
	if ((swRetValue = WriteTpcBs(TPC_SET_CMD, (BYTE *) &bFlashCMD[BLOCK_READ], 1)))
	{
		MP_DEBUG1("-E- TPC_SET_CMD FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if (dwSectorCount == 1)
	{
		// WAIT_INT
		if ((swRetValue = WaitTime1INT()))
		{
			MP_DEBUG1("-E- WAIT_INT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		// GET_INT
		if ((swRetValue = GetIntValue(&bIntValue)))
		{
			MP_DEBUG1("-E- GetIntValue FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		if (bIntValue & CMDNK)
		{
			MP_DEBUG1("-E- Error termination (bIntValue: %d)", bIntValue);
			return FAIL;
		}
		else if ((bIntValue & CED) && (bIntValue & BREQ) && (bIntValue & ERR))
		{
			MP_DEBUG1("-E- Flash read error occurred (bIntValue: %d)", bIntValue);

			//Set R/W Register TPC Addr
			SetRWRegTpcAddr(0x03, 0x01, 0x10, 0x06);

			// SET_RW_REG_ADRS
			if ((swRetValue = WriteTpcBs(TPC_SET_RW_REG_ADR, (BYTE *) (&sRWRegTpcAddr), 4)))
			{
				MP_DEBUG1("-E- TPC_SET_RW_REG_ADR FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}

			// READ_REG
			if ((swRetValue = ReadTpcBs(TPC_READ_REG, &bTempBuf, 1)))
			{
				MP_DEBUG1("-E- TPC_READ_REG FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}

			if ((bTempBuf & UCEX) || (bTempBuf & UCFG) || (bTempBuf & UCDT))
			{
				MP_DEBUG1("-E- uncorrectable error (bTempBuf: %d)", bTempBuf);
				return FAIL;
			}

			if ((dwBlockAddr == 0 && dwStartPage == 0) || (dwBlockAddr == 1 && dwStartPage == 1))
			{
				sInfo.bExtraData[dwStartPage][0] = 0xC0;
				sInfo.bExtraData[dwStartPage][1] = 0xFB;
				sInfo.bExtraData[dwStartPage][2] = 0xFF;
				sInfo.bExtraData[dwStartPage][3] = 0xFF;
				SetDataDma(dwTempBuf);
				if ((swRetValue = ReadTpcBsPage(ACCESS_LONG_DATA)))
				{
					MP_DEBUG1("-E- DMA FAIL (swRetValue: %d)", swRetValue);
					return swRetValue;
				}
				WriteTpcBs(TPC_SET_CMD, (BYTE *) & bFlashCMD[RESET], 1);
				WaitDataTransfer();
				return PASS;
			}
			else
			{
				// READ_REG
				if ((swRetValue = ReadTpcBs(TPC_READ_REG, sInfo.bExtraData[dwStartPage], 4)))
				{
					MP_DEBUG1("-E- TPC_READ_REG FAIL (swRetValue: %d)", swRetValue);
					return swRetValue;
				}

				if (((sInfo.bExtraData[dwStartPage][0] & 0x60) >> 5) != 3)
				{
					return FAIL;
				}

				if (sInfo.bWriteProtected != 1)
				{
					sInfo.bExtraData[dwStartPage][0] =
						((sInfo.bExtraData[dwStartPage][0] & (~0x60)) | 0x20);
					OverWriteExtraData(dwBlockAddr, dwStartPage, sInfo.bExtraData[dwStartPage][0]);
				}

				SetDataDma(dwTempBuf);
				if ((swRetValue = ReadTpcBsPage(ACCESS_LONG_DATA)))
				{
					MP_DEBUG1("-E- DMA FAIL (swRetValue: %d)", swRetValue);
					return swRetValue;
				}
				WaitDataTransfer();
				return FAIL;
			}
		}
		// READ_REG
		if ((swRetValue = ReadTpcBs(TPC_READ_REG, sInfo.bExtraData[dwStartPage], 4)))
		{
			MP_DEBUG1("-E- TPC_READ_REG FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		SetDataDma(dwTempBuf);
		if ((swRetValue = ReadTpcBsPage(ACCESS_LONG_DATA)))
		{
			MP_DEBUG1("-E- DMA FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		WaitDataTransfer();
	}
	else
	{
		dwCurPage = dwStartPage;
		while (dwSectorCount)
		{
			// WAIT_INT
			if ((swRetValue = WaitTime1INT()))
			{
				MP_DEBUG1("-E- WAIT_INT FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}

			// GET_INT
			if ((swRetValue = GetIntValue(&bIntValue)))
			{
				MP_DEBUG1("-E- GetIntValue FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}

			if (bIntValue & CMDNK)
			{
				MP_DEBUG1("-E- Error termination (bIntValue: %d)", bIntValue);
				return FAIL;
			}
			else if ((bIntValue & CED) && (bIntValue & BREQ) && (bIntValue & ERR))
			{
				MP_DEBUG1("-E- Flash Read error (bIntValue: %d)", bIntValue);

				// READ_REG
				if ((swRetValue = ReadTpcBs(TPC_READ_REG, sInfo.bExtraData[dwCurPage], 4)))
				{
					MP_DEBUG1("-E- TPC_READ_REG FAIL (swRetValue: %d)", swRetValue);
					return swRetValue;
				}

				if (((sInfo.bExtraData[dwCurPage][0] & 0x60) >> 5) != 3)
				{
					return FAIL;
				}

				if (sInfo.bWriteProtected != 1)
				{
					sInfo.bExtraData[dwCurPage][0] =
						((sInfo.bExtraData[dwCurPage][0] & (~0x60)) | 0x20);
					OverWriteExtraData(dwBlockAddr, dwCurPage, sInfo.bExtraData[dwCurPage][0]);
				}

				SetDataDma(dwTempBuf);
				if ((swRetValue = ReadTpcBsPage(ACCESS_LONG_DATA)))
				{
					MP_DEBUG1("-E- DMA FAIL (swRetValue: %d)", swRetValue);
					return swRetValue;
				}
				WaitDataTransfer();
				return WRITE_PS_NG;
			}

			dwSectorCount--;
			if (dwSectorCount == 0)
			{
				if (swRetValue = WriteTpcBs(TPC_SET_CMD, (BYTE *) & bFlashCMD[BLOCK_END], 1))
				{
					MP_DEBUG1("-E- TPC_SET_CMD FAIL (swRetValue: %d)", swRetValue);
					return swRetValue;
				}

				// WAIT_INT
				if ((swRetValue = WaitTime1INT()))
				{
					MP_DEBUG1("-E- WAIT_INT FAIL (swRetValue: %d)", swRetValue);
					return swRetValue;
				}

				// GET_INT
				if ((swRetValue = GetIntValue(&bIntValue)))
				{
					MP_DEBUG1("-E- GetIntValue FAIL (swRetValue: %d)", swRetValue);
					return swRetValue;
				}
			}

			// READ_REG
			if ((swRetValue = ReadTpcBs(TPC_READ_REG, sInfo.bExtraData[dwCurPage], 4)))
			{
				MP_DEBUG1("-E- TPC_READ_REG FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}

			dwCurPage++;
			SetDataDma(dwTempBuf);
			if ((swRetValue = ReadTpcBsPage(ACCESS_LONG_DATA)))
			{
				MP_DEBUG1("-E- DMA FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}
			dwTempBuf += MCARD_SECTOR_SIZE;
			WaitDataTransfer();
			// force update cache
            SetDataCacheInvalid();
		}
	}
	return PASS;
}

static SWORD LogicalRead(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress)
{
	register DWORD dwLogBlock_l, dwLogBlock_s, dwPhyBlock;
	register SWORD swRetValue;
	register BYTE bPage, bSegment;

	if (!dwSectorCount)
	{
		return PASS;
	}

	bPage = dwLogAddr % sInfo.dwPagePerBlock;
	dwLogBlock_l = dwLogAddr / sInfo.dwPagePerBlock;

	if (dwLogBlock_l > 493)
	{
		bSegment = ((dwLogBlock_l - 494) / 496) + 1;
		dwLogBlock_s = (dwLogBlock_l - 494) % 496;	//block addr outside seg 0        
	}
	else
	{
		bSegment = 0;
		dwLogBlock_s = dwLogBlock_l;	//block addr inside seg 0
	}
	dwPhyBlock = sInfo.wLog2PhyTable[bSegment * MAXPHYBLOCK + dwLogBlock_s];
	dwPhyBlock = dwPhyBlock + MAXPHYBLOCK * bSegment;	//Get phyical block addr

	if ((swRetValue = PhysicalRead(dwPhyBlock, bPage, dwSectorCount, dwBufferAddress)))
	{
		MP_DEBUG1("-E- PhysicalRead FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}
	return PASS;
}

static SWORD FlatRead(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr)
{
	register DWORD dwCount;
	register SWORD swRetValue;
	register BYTE bRetry;
	register DWORD dwhgCount,dwhgBuffAddr,dwhgLogAddr, dwTemp;
	//MP_DEBUG2("ms read %d,%d",dwSectorCount,dwLogAddr);	
	while (dwSectorCount)
	{
		bRetry = RETRY_COUNT;
		if (sInfo.bMsPro)
		{
			if (dwSectorCount > 0x80)
			{
				dwCount = 0x80;
			}
			else
			{
				dwCount = dwSectorCount;
			}

			if(sInfo.bMsPro == TYPE_MS_PRO)
			{
				while(bRetry) 
				{
					if ((swRetValue = ProLogicalRead(dwLogAddr, dwCount, dwBufferAddress)))
					{
						bRetry--;
						MP_DEBUG1("-I- remain retry times %d", bRetry);
						SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
						if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
						{
							MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
							return swRetValue;
						}
					}
					else
					{
						break;
					}
				}
			}
			else if  (sInfo.bMsPro == TYPE_MS_PRO_HG)
			{
				dwhgLogAddr = dwLogAddr;
				dwhgBuffAddr = dwBufferAddress;
				if( dwCount < 4)
					dwhgCount = dwCount;
				else
					dwhgCount = (0x04 - (dwLogAddr & 0x03))&0x03;

				if(dwhgCount > 0)
				{
					//MP_DEBUG("1 LogAddr =0x%x   Count = 0x%x  Buffer = 0x%x",dwhgLogAddr, dwhgCount, dwhgBuffAddr);
					bRetry = RETRY_COUNT;
					while(bRetry)
					{
						if ((swRetValue = ProLogicalRead(dwhgLogAddr, dwhgCount, dwhgBuffAddr)))
						{
							bRetry--;
							MP_DEBUG1("-I- remain retry times %d", bRetry);
							SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
							if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
							{
								MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
								return swRetValue;
							}
							continue;
						}
						else
						{
							break;
						}
					}
				}

				dwTemp = dwhgCount;
				dwhgLogAddr = dwLogAddr + dwTemp;
				dwhgBuffAddr = dwBufferAddress + (MCARD_SECTOR_SIZE * dwhgCount);
				dwhgCount = (dwCount - dwTemp)&0xFFFFFFFC;

				if(dwhgCount > 0)
				{
					//MP_DEBUG("2 LogAddr =0x%x   Count = 0x%x  Buffer = 0x%x",dwhgLogAddr, dwhgCount, dwhgBuffAddr);
					bRetry = RETRY_COUNT;
					while(bRetry)
					{
						if ((swRetValue = Pro2KLogicalRead(dwhgLogAddr, dwhgCount, dwhgBuffAddr)))    
						{
							bRetry--;
							MP_DEBUG1("-I- remain retry times %d", bRetry);
							SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
							if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
							{
								MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
								return swRetValue;
							}
							continue;
						}
						else
						{
							break;
						}
					}
				}

				dwTemp += dwhgCount;
				dwhgLogAddr = dwLogAddr + dwTemp;
				dwhgBuffAddr = dwBufferAddress + (MCARD_SECTOR_SIZE*dwTemp);
				dwhgCount = dwCount - dwTemp;

				if(dwhgCount > 0)
				{
					//MP_DEBUG("3 LogAddr =0x%x   Count = 0x%x  Buffer = 0x%x",dwhgLogAddr, dwhgCount, dwhgBuffAddr);
					bRetry = RETRY_COUNT;
					while(bRetry)
					{
						if ((swRetValue = ProLogicalRead(dwhgLogAddr, dwhgCount, dwhgBuffAddr)))
						{
							bRetry--;
							MP_DEBUG1("-I- remain retry times %d", bRetry);
							SetProParaReg(bProFlashCMD[PRO_STOP], 0, 0, 0x00, 0x00);
							if ((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *) (&sProParamReg), 7)))
							{
								MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
								return swRetValue;
							}
							continue;
						}
						else
						{
							break;
						}
					}
				}

			}
			else
			{
				MP_DEBUG("-E- ms type fail");
				break;
			}
		}
		else
		{
			while(bRetry)
			{
				dwCount = GetSameBlockPages(dwLogAddr, dwSectorCount);	                            
				if ((swRetValue = LogicalRead(dwLogAddr, dwCount, dwBufferAddress)))
				{
					bRetry--;
					WriteTpcBs(TPC_SET_CMD, (BYTE *) & bFlashCMD[RESET], 1);
					MP_DEBUG1("-I- remain retry times %d", bRetry);
				}
				else
				{
					break;
				}
			}
		}

		if (!bRetry)
		{
			MP_DEBUG1("-E- retry FAIL (swRetValue: %d)", swRetValue);                     
			return swRetValue;
		}
		dwLogAddr += dwCount;
		dwBufferAddress += (MCARD_SECTOR_SIZE * dwCount);
		dwSectorCount -= dwCount;
		//MP_DEBUG("dwSectorCount = %d   dwCount = %d",dwSectorCount, dwCount);
	}

	return PASS;
}

static SWORD OverWriteExtraData(DWORD dwBlockAddr, DWORD dwPageNumber, BYTE bOverWriteFlag)
{
	register MCARD *sMcard;
	DWORD i;
	SWORD swRetValue;
	BYTE bIntValue;

	sMcard = (MCARD *) (MCARD_BASE);
	sMcard->McMsT1 = WRITE_TIMEOUT1;

	//Set R/W Register TPC Addr
	SetRWRegTpcAddr(0x16, 0x04, 0x10, 0x07);

	//Set Parameter Register
	SetWriteParaReg(0x80, dwBlockAddr, 0x80, dwPageNumber);
	SetExtraParaReg(bOverWriteFlag, 0xff, 0, 0);

	if ((swRetValue = WriteTpcBs(TPC_SET_RW_REG_ADR, (BYTE *) & sRWRegTpcAddr, 4)))
	{
		MP_DEBUG1("-E- TPC_SET_RW_REG_ADR FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	// WRITE_REG
	if ((swRetValue = WriteTpcBs(TPC_WRITE_REG, (BYTE *) & sParamReg, 0x07)))
	{
		MP_DEBUG1("-E- TPC_WRITE_REG FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	// SET_CMD:BLOCK_WRITE
	if ((swRetValue = WriteTpcBs(TPC_SET_CMD, (BYTE *) & bFlashCMD[BLOCK_WRITE], 1)))
	{
		MP_DEBUG1("-E- TPC_SET_CMD FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = WaitTime1INT()))
	{
		MP_DEBUG1("-E- WaitTime1INT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	// GET_INT
	if ((swRetValue = GetIntValue(&bIntValue)))
	{
		MP_DEBUG1("-E- GetIntValue FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((bIntValue & CMDNK) || ((bIntValue & CED) && (bIntValue & ERR)))
	{
		MP_DEBUG1("-E- Error termination (bIntValue: %d)", bIntValue);
		return FAIL;
	}
	return PASS;
}

// Get Logical addr of (blockAddr + pageNo) to buffer
static SWORD WriteExtraData(DWORD dwBlockAddr, DWORD dwPageNumber, DWORD dwLogAddr)
{
	register MCARD *sMcard;
	register DWORD i;
	register SWORD swRetValue;
	BYTE bIntValue;

	sMcard = (MCARD *) (MCARD_BASE);
	sMcard->McMsT1 = WRITE_TIMEOUT1;

	//Set R/W Register TPC Addr
	SetRWRegTpcAddr(0x16, 0x04, 0x10, 0x0a);

	//Set Parameter Register
	SetWriteParaReg(0x80, dwBlockAddr, 0x40, dwPageNumber);
	SetExtraParaReg(0xf8, 0xff, ((dwLogAddr & 0xff00) >> 8), (dwLogAddr & 0x00ff));

	if ((swRetValue = WriteTpcBs(TPC_SET_RW_REG_ADR, (BYTE *) & sRWRegTpcAddr, 4)))
	{
		MP_DEBUG1("-E- TPC_SET_RW_REG_ADR FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	// WRITE_REG
	if ((swRetValue = WriteTpcBs(TPC_WRITE_REG, (BYTE *) & sParamReg, 0x0a)))
	{
		MP_DEBUG1("-E- TPC_WRITE_REG FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	// SET_CMD:BLOCK_WRITE
	if ((swRetValue = WriteTpcBs(TPC_SET_CMD, (BYTE *) & bFlashCMD[BLOCK_WRITE], 1)))
	{
		MP_DEBUG1("-E- TPC_SET_CMD FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = WaitTime1INT()))
	{
		MP_DEBUG1("-E- WaitTime1INT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	// GET_INT
	if ((swRetValue = GetIntValue(&bIntValue)))
	{
		MP_DEBUG1("-E- GetIntValue FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((bIntValue & CMDNK) || ((bIntValue & CED) && (bIntValue & ERR)))
	{
		MP_DEBUG1("-E- Error termination (bIntValue: %d)", bIntValue);
		return FAIL;
	}
	return PASS;
}

static SWORD PhysicalErase(DWORD dwBlockAddr)
{
	register MCARD *sMcard;
	register SWORD swRetValue;
	BYTE bIntValue;

	sMcard = (MCARD *) (MCARD_BASE);
	sMcard->McMsT1 = ERASE_TIMEOUT1;

	//Set R/W Register TPC Addr
	SetRWRegTpcAddr(0x16, 0x04, 0x10, 0x04);

	//Set Parameter Register 
	SetParaReg(0x80, dwBlockAddr, 0x00, 0x00);

	// SET_RW_REG_ADRS
	if ((swRetValue = WriteTpcBs(TPC_SET_RW_REG_ADR, (BYTE *) & sRWRegTpcAddr, 4)))
	{
		MP_DEBUG1("-E- TPC_SET_RW_REG_ADR FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = WriteTpcBs(TPC_WRITE_REG, (BYTE *) & sParamReg, 4)))
	{
		MP_DEBUG1("-E- TPC_WRITE_REG FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	// SET_CMD:BLOCK_ERASE
	if ((swRetValue = WriteTpcBs(TPC_SET_CMD, (BYTE *) & bFlashCMD[BLOCK_ERASE], 1)))
	{
		MP_DEBUG1("-E- TPC_WRITE_REG FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = WaitTime1INT()))
	{
		MP_DEBUG1("-E- WaitTime1INT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	// GET_INT
	if ((swRetValue = GetIntValue(&bIntValue)))
	{
		MP_DEBUG1("-E- GetIntValue FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((bIntValue & CMDNK) || ((bIntValue & CED) && (bIntValue & ERR)))
	{
		MP_DEBUG1("-E- Error termination (bIntValue: %d)", bIntValue);
		return FAIL;
	}
	return PASS;
}

// Write one block of data
// The function is for physical write
static SWORD PhysicalWrite(DWORD dwPhyAddr, DWORD dwStartPage, DWORD dwSectorCount,
						   DWORD dwBufferAddress, DWORD dwLogAddr)
{
	register MCARD *sMcard;
	DWORD dwCount;
	SWORD swRetValue;
	BYTE bIntValue;

	sMcard = (MCARD *) (MCARD_BASE);
	sMcard->McMsT1 = WRITE_TIMEOUT1;


	//Set R/W Register TPC Addr
	SetRWRegTpcAddr(0x16, 0x04, 0x10, 0x0a);

	//Set Parameter Register
	if (dwSectorCount == 1)
	{
		SetWriteParaReg(0x80, dwPhyAddr, 0x20, dwStartPage);
	}
	else
	{
		SetWriteParaReg(0x80, dwPhyAddr, 0x00, dwStartPage);
	}
	SetExtraParaReg(0xf8, 0xff, ((dwLogAddr & 0xff00) >> 8), (dwLogAddr & 0x00ff));

	if ((swRetValue = WriteTpcBs(TPC_SET_RW_REG_ADR, (BYTE *) & sRWRegTpcAddr, 4)))
	{
		MP_DEBUG1("-E- TPC_SET_RW_REG_ADR FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	// WRITE_REG
	if ((swRetValue = WriteTpcBs(TPC_WRITE_REG, (BYTE *) & sParamReg, 10)))
	{
		MP_DEBUG1("-E- TPC_WRITE_REG FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	// SET_CMD:BLOCK_WRITE
	if ((swRetValue = WriteTpcBs(TPC_SET_CMD, (BYTE *) & bFlashCMD[BLOCK_WRITE], 1)))
	{
		MP_DEBUG1("-E- TPC_SET_CMD FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if (dwSectorCount == 1)
	{
		if ((swRetValue = WaitTime1INT()))
		{
			MP_DEBUG1("-E- WaitTime1INT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		// GET_INT
		if ((swRetValue = GetIntValue(&bIntValue)))
		{
			MP_DEBUG1("-E- GetIntValue FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		if (bIntValue & CMDNK)
		{
			MP_DEBUG1("-E- Error termination (bIntValue: %d)", bIntValue);
			return FAIL;
		}

		SetDataDma(dwBufferAddress);
		if ((swRetValue = WriteTpcBsPage(ACCESS_LONG_DATA)))
		{
			MP_DEBUG1("-E- GetINT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		if ((swRetValue = WaitTime1INT()))
		{
			MP_DEBUG1("-E- WaitTime1INT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		// GET_INT
		if ((swRetValue = GetIntValue(&bIntValue)))
		{
			MP_DEBUG1("-E- GetIntValue FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		if (((bIntValue & CED) && (bIntValue & ERR)))
		{
			MP_DEBUG1("-E- Error termination (bIntValue: %d)", bIntValue);
			return FAIL;
		}
		WaitDataTransfer();
	}
	else
	{
		if ((swRetValue = WaitTime1INT()))
		{
			MP_DEBUG1("-E- WaitTime1INT FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		// GET_INT
		if ((swRetValue = GetIntValue(&bIntValue)))
		{
			MP_DEBUG1("-E- GetIntValue FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		if ((bIntValue & CMDNK) || ((bIntValue & CED) && (bIntValue & ERR)))
		{
			MP_DEBUG1("-E- Error termination (bIntValue: %d)", bIntValue);
			return FAIL;
		}

		dwCount = dwSectorCount;
		while (dwCount)
		{
			dwCount--;
			SetDataDma(dwBufferAddress);
			if ((swRetValue = WriteTpcBsPage(ACCESS_LONG_DATA)))
			{
				MP_DEBUG1("-E- GetINT FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}

			if ((swRetValue = WaitTime1INT()))
			{
				MP_DEBUG1("-E- WaitTime1INT FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}

			// GET_INT
			if ((swRetValue = GetIntValue(&bIntValue)))
			{
				MP_DEBUG1("-E- GetIntValue FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}

			if ((bIntValue & CMDNK) || ((bIntValue & CED) && (bIntValue & ERR)))
			{
				MP_DEBUG1("-E- Error termination (bIntValue: %d)", bIntValue);
				return FAIL;
			}

			dwBufferAddress += MCARD_SECTOR_SIZE;
			WaitDataTransfer();
            // force update cache
            SetDataCacheInvalid();			
		}

		if ((dwStartPage + dwSectorCount) != sInfo.dwPagePerBlock)
		{
			// SET_CMD:BLOCK_END
			if ((swRetValue = WriteTpcBs(TPC_SET_CMD, (BYTE *) & bFlashCMD[BLOCK_END], 1)))
			{
				MP_DEBUG1("-E- TPC_SET_CMD FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}

			if ((swRetValue = WaitTime1INT()))
			{
				MP_DEBUG1("-E- WaitTime1INT FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}

			// GET_INT
			if ((swRetValue = GetIntValue(&bIntValue)))
			{
				MP_DEBUG1("-E- GetIntValue FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}

			if ((bIntValue & CMDNK) || ((bIntValue & CED) && (bIntValue & ERR)))
			{
				MP_DEBUG1("-E- Error termination (bIntValue: %d)", bIntValue);
				return FAIL;
			}
		}
	}
	return PASS;
}

// Write number of count pages in the same block
// The function is for logical addr read
static SWORD LogicalWrite(DWORD dwLogAddr, DWORD dwSectorCount, DWORD dwBufferAddress)
{
	DWORD i, j;
	SWORD swRetValue;
	WORD wLogBlock_l, wLogBlock_s, wPhyBlockAddr, wNewBlockAddr;
	BYTE bPage, bSegment, bHead, bTail, bNGFlag;
	BYTE *pbTempBuffer, *pbBuffer, *pbTemp;

	pbTemp = NULL;
	bNGFlag = FALSE;
	pbBuffer = (BYTE *) dwBufferAddress;

	bPage = dwLogAddr % sInfo.dwPagePerBlock;
	wLogBlock_l = dwLogAddr / sInfo.dwPagePerBlock;
	if (wLogBlock_l > 493)
	{
		bSegment = ((wLogBlock_l - 494) / 496) + 1;
		wLogBlock_s = (wLogBlock_l - 494) % 496;
	}
	else
	{
		bSegment = 0;
		wLogBlock_s = wLogBlock_l;
	}

	//Get Actual Phy Address
	wPhyBlockAddr = sInfo.wLog2PhyTable[wLogBlock_s + MAXPHYBLOCK * bSegment];
	wPhyBlockAddr = wPhyBlockAddr + MAXPHYBLOCK * bSegment;

	//build write data buffer
	bHead = bPage;
	bTail = sInfo.dwPagePerBlock - bPage - dwSectorCount;

	if ((bHead == 0) && (bTail == 0))
	{
		pbTempBuffer = (BYTE *) dwBufferAddress;
		ReadExtraData(wPhyBlockAddr, 0, (DWORD) sInfo.bExtraData[0]);
		for (i = 0; i < dwSectorCount; i++)
		{
			sInfo.bExtraData[i][0] = 0xf8;
			sInfo.bExtraData[i][1] = 0xff;
			sInfo.bExtraData[i][2] = sInfo.bExtraData[0][2];
			sInfo.bExtraData[i][3] = sInfo.bExtraData[0][3];
		}
	}
	else
	{
		pbTemp = (BYTE *)ext_mem_malloc(0x4000);//(BYTE *) bTempBuffer;
		if(pbTemp == NULL)
			return FAIL;
		
		pbTempBuffer = pbTemp;
		
		if (bHead != 0)
		{
			if (PhysicalRead(wPhyBlockAddr, 0, bHead, (DWORD) pbTempBuffer) == WRITE_PS_NG)
			{
				sInfo.bExtraData[0][0] = sInfo.bExtraData[0][0] & (~0x80);
				OverWriteExtraData(wPhyBlockAddr, 0, sInfo.bExtraData[0][0]);
				bNGFlag = TRUE;
			}

			pbTempBuffer += (bHead * MCARD_SECTOR_SIZE);
		}
		else
		{
			ReadExtraData(wPhyBlockAddr, 0, (DWORD) sInfo.bExtraData[0]);
		}

		for (i = 0; i < dwSectorCount; i++)
		{
			for (j = 0; j < MCARD_SECTOR_SIZE; j++)
			{
				pbTempBuffer[j] = pbBuffer[j];
			}
			pbTempBuffer += MCARD_SECTOR_SIZE;
			pbBuffer += MCARD_SECTOR_SIZE;
			sInfo.bExtraData[bHead + i][0] = 0xf8;
			sInfo.bExtraData[bHead + i][1] = 0xff;
			sInfo.bExtraData[bHead + i][2] = sInfo.bExtraData[0][2];
			sInfo.bExtraData[bHead + i][3] = sInfo.bExtraData[0][3];
		}

		if (bTail != 0)
		{
			if (PhysicalRead(wPhyBlockAddr, (bHead + dwSectorCount), bTail, (DWORD) pbTempBuffer) ==
				WRITE_PS_NG)
			{
				sInfo.bExtraData[0][0] = sInfo.bExtraData[0][0] & (~0x80);
				OverWriteExtraData(wPhyBlockAddr, 0, sInfo.bExtraData[0][0]);
				bNGFlag = TRUE;
			}
		}
		pbTempBuffer = pbTemp;
	}



	//chage source block to updating status
	sInfo.bExtraData[0][0] &= ~(0x10);
	OverWriteExtraData(wPhyBlockAddr, 0, sInfo.bExtraData[0][0]);


	//Get free Block Address
	wNewBlockAddr = GetEmptyBlock(bSegment);
	if (wNewBlockAddr == 0xffff)
	{
		sInfo.wReAssignTable[bSegment] = 0;
		wNewBlockAddr = GetEmptyBlock(bSegment);
		if (wNewBlockAddr == 0xffff)
		{
			if(pbTemp != NULL)
				ext_mem_free(pbTemp);
				
			return FAIL;
		}
	}

	wNewBlockAddr += (bSegment * MAXPHYBLOCK);
	PhysicalErase(wNewBlockAddr);

	if (swRetValue =
		PhysicalWrite(wNewBlockAddr, 0, sInfo.dwPagePerBlock, (DWORD) pbTempBuffer, wLogBlock_l))
	{
		OverWriteExtraData(wNewBlockAddr, 0, 0x78);

		if(pbTemp != NULL)
			ext_mem_free(pbTemp);
		
		return swRetValue;
	}

	sInfo.wLog2PhyTable[MAXPHYBLOCK * bSegment + wLogBlock_s] =
		(WORD) (wNewBlockAddr % MAXPHYBLOCK);
	if (bNGFlag != TRUE)
	{
		for (i = 0; i < 32; i++)
		{
			if (sInfo.bExtraData[i][0] != 0xf8)
			{
				sInfo.bExtraData[i][0] |= 0x10;
				OverWriteExtraData(wNewBlockAddr, i, sInfo.bExtraData[i][0]);
			}
		}
		PhysicalErase(wPhyBlockAddr);
		sInfo.wLog2PhyTable[bSegment * MAXPHYBLOCK + sInfo.wReAssignTable[bSegment] + 496] =
			(WORD) (wPhyBlockAddr % MAXPHYBLOCK);
	}
	else
	{
		for (i = 0; i < 32; i++)
		{
			if (((sInfo.bExtraData[i][0] & 0x60) >> 5) == 1)
			{
				sInfo.bExtraData[i][0] = sInfo.bExtraData[i][0] & (~0x20);
				OverWriteExtraData(wNewBlockAddr, i, sInfo.bExtraData[i][0]);
			}
		}
		for (i = sInfo.wReAssignTable[bSegment]; i < (16 - 1); i++)
		{
			sInfo.wLog2PhyTable[bSegment * MAXPHYBLOCK + i + 496] =
				sInfo.wLog2PhyTable[bSegment * MAXPHYBLOCK + (i + 1) + 496];
		}
		sInfo.wLog2PhyTable[bSegment * MAXPHYBLOCK + i + 496] = 0xffff;
	}

	wNewBlockAddr = sInfo.wReAssignTable[bSegment];
	wNewBlockAddr++;
	if (wNewBlockAddr >= 16)
	{
		wNewBlockAddr = 0;
	}
	sInfo.wReAssignTable[bSegment] = wNewBlockAddr;
	
	if(pbTemp != NULL)
		ext_mem_free(pbTemp);
	
	return PASS;
}

static SWORD FlatWrite(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr)
{
	DWORD dwCount;
	SWORD swRetValue;
	BYTE bRetry;
	DWORD dwhgCount,dwhgBuffAddr,dwhgLogAddr, dwTemp;

	if (sInfo.bWriteProtected)
	{
		return WRITE_PROTECT;
	}

	//MP_DEBUG2("ms write %d,%d",dwSectorCount,dwLogAddr);
	while (dwSectorCount)
	{
		bRetry = MCARD_RETRY_TIME;
		while (bRetry)
		{
			if (Polling_MS_Status())
				return FAIL;
			if (sInfo.bMsPro)
			{
				if (dwSectorCount > 0x80)
				{
					dwCount = 0x80;
				}
				else
				{
					dwCount = dwSectorCount;
				}
				if(sInfo.bMsPro == TYPE_MS_PRO)
				{
					if ((swRetValue = ProLogicalWrite(dwLogAddr, dwCount, dwBufferAddress)))
					{
						if(swRetValue == WRITE_PROTECT)
						{
							return swRetValue;    //   when the state go to write-disabled states     return the swRetValue
						}
						else
						{
							bRetry--;
							MP_DEBUG1("-I- remain retry times %d", bRetry);
						}
					}
				    else
				    {
					    break;
				    }
				}
				else if( sInfo.bMsPro == TYPE_MS_PRO_HG)
				{
					dwhgLogAddr = dwLogAddr;
					dwhgBuffAddr = dwBufferAddress;
					if( dwCount < 4)
						dwhgCount = dwCount;
					else
						dwhgCount = (0x04 - (dwLogAddr & 0x03))&0x03;
					//MP_DEBUG("1 LogAddr =0x%x   Count = 0x%x  Buffer = 0x%x",dwhgLogAddr, dwhgCount, dwhgBuffAddr);
					if(dwhgCount > 0)
					{
						if ((swRetValue = ProLogicalWrite(dwhgLogAddr, dwhgCount, dwhgBuffAddr)))
						{
							if( swRetValue == WRITE_PROTECT)
							{
								return swRetValue;
							}
							else
							{
								bRetry--;
								MP_DEBUG1("-I- remain retry times %d", bRetry);
								continue;
							}
						}
						if((dwCount<4)&&(swRetValue==PASS))
							break;
					}
                                
					dwTemp = dwhgCount;
					dwhgLogAddr = dwLogAddr + dwTemp;
					dwhgBuffAddr = dwBufferAddress + (MCARD_SECTOR_SIZE * dwhgCount);
					dwhgCount = (dwCount - dwTemp)&0xFFFFFFFC;
					//MP_DEBUG("2 LogAddr =0x%x   Count = 0x%x  Buffer = 0x%x",dwhgLogAddr, dwhgCount, dwhgBuffAddr);
					if(dwhgCount > 0)
					{
						if ((swRetValue = Pro2KLogicalWrite(dwhgLogAddr, dwhgCount, dwhgBuffAddr)))
						{
							if( swRetValue == WRITE_PROTECT)
							{
								return swRetValue;
							}
							else
							{
								bRetry--;
								MP_DEBUG1("-I- remain retry times %d", bRetry);
								continue;
							}
						}   
					}

					dwTemp += dwhgCount;
					dwhgLogAddr = dwLogAddr + dwTemp;
					dwhgBuffAddr = dwBufferAddress + (MCARD_SECTOR_SIZE*dwTemp);
					dwhgCount = dwCount - dwTemp;
					//MP_DEBUG("3 LogAddr =0x%x   Count = 0x%x  Buffer = 0x%x",dwhgLogAddr, dwhgCount, dwhgBuffAddr);
					if(dwhgCount > 0)
					{
						if ((swRetValue = ProLogicalWrite(dwhgLogAddr, dwhgCount, dwhgBuffAddr)))
						{
							if( swRetValue == WRITE_PROTECT)
							{
								return swRetValue;
							}
							else
							{
								bRetry--;
								MP_DEBUG1("-I- remain retry times %d", bRetry);
								continue;
							}
						}
					}

					if(swRetValue == PASS)
						break;
				}
				else
				{
					MP_DEBUG("-E- ms type fail");
					break;
				}
			}
			else
			{
				dwCount = GetSameBlockPages(dwLogAddr, dwSectorCount);					
				if ((swRetValue = LogicalWrite(dwLogAddr, dwCount, dwBufferAddress)))
				{	
					bRetry--;
					WriteTpcBs(TPC_SET_CMD, (BYTE *) & bFlashCMD[RESET], 1);
					MP_DEBUG1("-I- remain retry times %d", bRetry);
				}
				else
				{
					break;
				}
			}

			if(sInfo.bWriteProtected)
			{
				mpDebugPrint("-E- write protected");
				break;
			}
		}
		if (!bRetry)
		{
			MP_DEBUG1("-E- retry FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		dwLogAddr += dwCount;
		dwBufferAddress += (MCARD_SECTOR_SIZE * dwCount);
		dwSectorCount -= dwCount;
	}

	return PASS;
}

static SWORD GetAttributeInfo(void)
{
	DWORD i;
	WORD wUnitSize;
	SWORD swRetValue;

	#pragma alignvar(4)
	BYTE bDataBuf[MCARD_SECTOR_SIZE], bDataBuf1[MCARD_SECTOR_SIZE], bSysParam[100],
			bBlockCount, bTempBuf[2], bBootBlockCount;
	BYTE *pbDataBuf;
	WORD wBusWidth;
	ST_ATTR_INFO_AREA *pAttrInfo;
	BYTE  bEntry, bSysInfoFind;
	DWORD dwEntrySize, dwEntryAddr;

	pbDataBuf = (BYTE *)(((DWORD) (&bDataBuf) | 0xa0000000));
	if (sInfo.bMsPro)
	{
		if ((swRetValue = ProWaitCpuStartup()))
		{
			MP_DEBUG1("-E- ProWaitCpuStartup FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		if(!sInfo.bSwitchFail)
		{
			if((swRetValue = ChangeBusWidth(BUS_WIDTH_4)))
			{
				MP_DEBUG1("-E- ChangeBusWidth 4bit FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}
		}
#if MS_PRO_HG
		if(!sInfo.bSwitchFail)
		{
			if(sInfo.bMsPro == TYPE_MS_PRO_HG)
			{
				if((swRetValue = ChangeBusWidth(BUS_WIDTH_8)))
				{
					MP_DEBUG1("-E- ChangeBusWidth 4bit FAIL (swRetValue: %d)", swRetValue);
					return swRetValue;
				}
			}
		}	
#endif		
		//ver.1.00 spec A.1.5 procedure of obtaining and confirming attribute information
		if ((swRetValue = ProReadAttribute((DWORD) pbDataBuf, 0x0)))
		{
			MP_DEBUG1("-E- ProReadAttribute FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		//Fig A.1.5.4 procedure to confirm system information
		// check memory stick class
		pAttrInfo = (ST_ATTR_INFO_AREA *)pbDataBuf;

		if( pAttrInfo->wSignCode != MS_SIGNATURE_CODE)
		{
			MP_DEBUG("-E- ms type FAIL : SIGNATURE CODE error");
			return FAIL;
		}
		if( (pAttrInfo->bDevEntryCount <1)||(pAttrInfo->bDevEntryCount >12))
		{
			MP_DEBUG("-E- ms type FAIL : Entry Count  error");
			return FAIL;
		}
		bSysInfoFind = FALSE;
		for( i = 0; i < pAttrInfo->bDevEntryCount; i++)
		{
			if( pAttrInfo->stDevInfoEntry[i].bDevInfoID == SYS_INFO)
			{
				bEntry = i;
				dwEntrySize = pAttrInfo->stDevInfoEntry[i].dwEntrySize;
				dwEntryAddr = pAttrInfo->stDevInfoEntry[i].dwEntryAddr;
				bSysInfoFind = TRUE;
				break;
			}
			MP_DEBUG("DevID = %x    Size = %x   Addr = %x",pAttrInfo->stDevInfoEntry[i].bDevInfoID, pAttrInfo->stDevInfoEntry[i].dwEntrySize, pAttrInfo->stDevInfoEntry[i].dwEntryAddr);
		}

		if( bSysInfoFind == FALSE )
		{
		    MP_DEBUG("-E- ms type fail: Not Find System Parameter");
		    return FAIL;
		}

		if( dwEntrySize != SYS_INFO_SIZE)
		{
		    MP_DEBUG("-E- ms type fail: System Parameter size error");
		    return FAIL;
		}

		if( (dwEntryAddr < 0x1A0)||( (dwEntryAddr+dwEntrySize) > 0x8000))
		{
		    MP_DEBUG("-E- ms type fail : Entry Address Error");
		    return FAIL;
		}
		if((swRetValue = CheckSystemParameter( dwEntryAddr, bSysParam ,&wBusWidth )))
		{
		    return swRetValue;                          
		}
		pbDataBuf = (BYTE *) (((DWORD) (&bSysParam)));

		MP_DEBUG("wBusWidth = 0x%x",wBusWidth);

		//get information
		sInfo.dwBlockSize = *(WORD *) ((BYTE *) pbDataBuf + 0x02);	//uint (KB)
		sInfo.dwBlock = *(WORD *) ((BYTE *) pbDataBuf + 0x04);         //Total Block
		sInfo.dwEffBlock = *(WORD *) ((BYTE *) pbDataBuf + 0x06) - 2;	//minus 2 for boot block;
		wUnitSize = *(WORD *) ((BYTE *) pbDataBuf + 0x2c);
		LONG lTemp;
		lTemp = sInfo.dwEffBlock * ((sInfo.dwBlockSize * wUnitSize) / MCARD_SECTOR_SIZE);
		sInfo.dwCapacity = lTemp;  //sInfo.dwEffBlock * ((sInfo.dwBlockSize * wUnitSize) / MCARD_SECTOR_SIZE);	//total sectors
		sInfo.dwPagePerBlock = sInfo.dwBlockSize * wUnitSize / MCARD_SECTOR_SIZE;
		sInfo.dwSegment = sInfo.dwBlock / 512;
/*
		pbDataBuf = (BYTE *) (((DWORD) (&bDataBuf1) | 0xa0000000));

		if((swRetValue = ProReadAttribute((DWORD)pbDataBuf, 0x01)))
		{
			MP_DEBUG1("-E- ProReadAttribute FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
*/
/*
		MP_DEBUG("RecognizeFileSystem");
		if((swRetValue = RecognizeFileSystem()))
		{
			return swRetValue;
		}

		MP_DEBUG("RecognizeFileSystem END");
*/
	}
	else
	{
		//ver.1.40 Spec A.5 Boot Block Search Process
		sInfo.bBootProtectFlag = FALSE;
		bBlockCount = 0;
		bBootBlockCount = 0;
		while (bBlockCount <= 11)
		{
			if (Polling_MS_Status())
				return FAIL;
			//read block data
			if ((swRetValue = PhysicalRead(bBlockCount, 0, 1, (DWORD) pbDataBuf)))
			{
				MP_DEBUG1("-E- PhysicalRead FAIL (swRetValue: %d)", swRetValue);
				bBlockCount++;
				continue;
			}

			//get/check block status data
			SetRWRegTpcAddr(0x3, 0x1, 0x10, 0x1);
			if ((swRetValue = WriteTpcBs(TPC_SET_RW_REG_ADR, (BYTE *) & sRWRegTpcAddr, 4)))
			{
				bBlockCount++;
				continue;
			}
			if ((swRetValue = ReadTpcBs(TPC_READ_REG, bTempBuf, 0x1)))
			{
				bBlockCount++;
				continue;
			}

			//block status
			if (bTempBuf[0] & 0x11)
			{
				bBlockCount++;
			}
			else
			{
				//check block extra data
				//system bit
				if (!(sInfo.bExtraData[0][0] & 0x80))
				{
					bBlockCount++;
					sInfo.bBootProtectFlag = TRUE;
				}
				else if (sInfo.bExtraData[0][1] & 0x04)
				{
					bBlockCount++;
					sInfo.bBootProtectFlag = TRUE;
				}
				else
				{
					if ((pbDataBuf[0] == 0x00) && (pbDataBuf[1] == 0x01))
					{
						bBootBlockCount++;
						sInfo.bBootBlock = bBlockCount;
						if (bBootBlockCount == 2)
						{
							break;
						}
						for (i = 0; i < MCARD_SECTOR_SIZE; i++)
						{
							bDataBuf1[i] = pbDataBuf[i];
						}
						bBlockCount++;
					}
					else
					{
						bBlockCount++;
						sInfo.bBootProtectFlag = TRUE;
					}

				}
			}
		}

		if (bBootBlockCount == 0)
		{
			MP_DEBUG("-E- Get Boot Block FAIL");
			return FAIL;
		}

		// Get attribute information
		// spec A.6 Boot block contents check process
		// check memory stick class
		if (bDataBuf1[0x1a0] != 0x1)
		{
			MP_DEBUG("-E- ms type FAIL");
			return FAIL;
		}

		//check memory stick device type
		if (bDataBuf1[0x1D8] != 0x0)
		{
			MP_DEBUG("-E- ms type not support");
			return TYPE_NOT_SUPPORT;
		}

		//check support parallel or not
		if (bDataBuf1[0x1D3] == 0x1)
		{
			MP_DEBUG("this ms support parallel mode");
			wBusWidth = BUS_WIDTH_4;
			MP_DEBUG("-I- Support parallel mode");
			wBusWidth = BUS_WIDTH_1;
		}
		else
		{
			MP_DEBUG("this ms only support serial mode");
			wBusWidth = BUS_WIDTH_1;
		}
		//get information
		sInfo.dwBlockSize = *(WORD *) ((BYTE *) bDataBuf1 + 0x1A2);	//uint (KB)
		sInfo.dwBlock = *(WORD *) ((BYTE *) bDataBuf1 + 0x1A4);
		sInfo.dwEffBlock = *(WORD *) ((BYTE *) bDataBuf1 + 0x1A6) - 2;	//minus 2 for boot block;
		sInfo.dwCapacity = (sInfo.dwEffBlock * sInfo.dwBlockSize * 2) ; // / MCARD_SECTOR_SIZE;	//total sectors
		sInfo.dwPagePerBlock = sInfo.dwBlockSize * 1024 / MCARD_SECTOR_SIZE;
		sInfo.dwSegment = sInfo.dwBlock / 512;
		if ((DWORD) ((WORD *) bDataBuf1)[0x1C8] == 0x1001)
		{
				MP_DEBUG("this is magicgate type");
			return TYPE_NOT_SUPPORT;
		}
	}


	if(sInfo.bMsPro == TYPE_MS)
	{
		if(wBusWidth == BUS_WIDTH_4)
		{
			if((swRetValue = ChangeBusWidth(wBusWidth)))
			{
				MP_DEBUG1("-E- ChangeBusWidth 4bit FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}
		}
	}

	if (sInfo.bBootProtectFlag == TRUE)
	{
		BootProtectProcess(sInfo.bBootBlock);
	}

	return PASS;
}

static SWORD BootProtectProcess(BYTE bBlockMaxCount)
{
	DWORD dwPhyBlockAddr;
	SWORD swRetValue;
	WORD wLogAddr;

#pragma alignvar(4)
	BYTE bDataBuf[MCARD_SECTOR_SIZE], bDisableBuf[MCARD_SECTOR_SIZE], bBlockCount, bTempBuf[4];
	BYTE *pbDataBuf, *pbDisableBuf;

	pbDataBuf = (BYTE *) (((DWORD) (&bDataBuf) | 0xa0000000));
	pbDisableBuf = (BYTE *) (((DWORD) (&bDisableBuf) | 0xa0000000));

	if ((swRetValue = PhysicalRead(sInfo.bBootBlock, 1, 1, (DWORD) pbDisableBuf)))
	{
		MP_DEBUG1("-E- PhysicalRead FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	bBlockCount = 0;
	while (bBlockCount < bBlockMaxCount)
	{
		if (Polling_MS_Status())
			return FAIL;
		if ((swRetValue = PhysicalRead(bBlockCount, 0, 1, (DWORD) pbDataBuf)))
		{
			MP_DEBUG1("-E- PhysicalRead FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		if ((pbDataBuf[0] == 0x00) && (pbDataBuf[1] == 0x01))
		{
			bBlockCount++;
			continue;
		}

		//get/check block status data
		SetRWRegTpcAddr(0x3, 0x1, 0x10, 0x1);
		if ((swRetValue = WriteTpcBs(TPC_SET_RW_REG_ADR, (BYTE *) & sRWRegTpcAddr, 4)))
		{
			bBlockCount++;
			continue;
		}
		if ((swRetValue = ReadTpcBs(TPC_READ_REG, bTempBuf, 0x1)))
		{
			bBlockCount++;
			continue;
		}

		//block status
		if (bTempBuf[0] & 0x11)
		{
			bBlockCount++;
			continue;
		}
		//check block extra data
		wLogAddr = (WORD) ((sInfo.bExtraData[0][2] << 8) | (sInfo.bExtraData[0][3]));
		if (wLogAddr != 0xffff)
		{
			if (sInfo.bWriteProtected == 1)
			{
				sInfo.bProtectBlockCount++;
				sInfo.wProtectBlockTable[bBlockCount] = wLogAddr;
			}
			else
			{
				for (dwPhyBlockAddr = (bBlockMaxCount + 1); dwPhyBlockAddr < MAXPHYBLOCK;
					 dwPhyBlockAddr++)
				{
					// Check if disabled block
					if (CheckDisableBlock(dwPhyBlockAddr, pbDisableBuf))
					{
						continue;
					}

					// Get logical addr
					if ((swRetValue = ReadExtraData(dwPhyBlockAddr, 0x0, (DWORD) bTempBuf)))
					{
						MP_DEBUG1("-E- ReadExtraData FAIL (swRetValue: %d)", swRetValue);
						return swRetValue;
					}

					if ((WORD) ((bTempBuf[2] << 8) | (bTempBuf[3])) == 0xffff)
					{
						PhysicalErase(dwPhyBlockAddr);
						PhysicalWrite(dwPhyBlockAddr, 0, 1, (DWORD) pbDataBuf, wLogAddr);
						break;
					}
				}
			}
		}

		if (sInfo.bWriteProtected != 1)
		{
			if ((swRetValue = OverWriteExtraData(bBlockCount, 0, (bTempBuf[0] & (~0x80)))))
			{
				bBlockCount++;
				continue;
			}
		}
		bBlockCount++;
	}
	return PASS;
}

static SWORD Identify(void)
{
	SWORD swRetValue;
	sInfo.bWriteProtected = 0;
	sInfo.bMsPro = TYPE_MS;
	sInfo.bBootProtectFlag = FALSE;
	sInfo.bProtectBlockCount = 0;
	if ((swRetValue = IdentifyType()))
	{
		MP_DEBUG1("-E- IdentifyType FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}
	mpDebugPrint("identify type ok");
	if ((swRetValue = GetAttributeInfo()))
	{
		MP_DEBUG1("-E- GetAttributeInfo FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}
	mpDebugPrint("get atrribute info ok");
	if (!sInfo.bMsPro)
	{
		if(sInfo.wLog2PhyTable == NULL)
		{
			sInfo.wLog2PhyTable = (WORD *)ker_mem_malloc(sInfo.dwSegment * MAXPHYBLOCK*2, TaskGetId());			
			if(sInfo.wLog2PhyTable == NULL)
				return FAIL;
		}
				MP_DEBUG("start initlog2phy...");
		if ((swRetValue = InitLog2PhyTable()))
		{				
			MP_DEBUG1("-E- InitLog2PhyTable FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
		}
		MP_DEBUG("initlog2phy ok");
	}
	return PASS;
}

static SWORD CheckSystemParameter( DWORD dwEntryAddr, BYTE *bSysParam, WORD *wBusWidth)
{
#pragma alignvar(4)
	BYTE bDataBuf[MCARD_SECTOR_SIZE];

	BYTE *pbDataBuf, *pbSysParam;
	DWORD dwCopySize;
	DWORD dwSector;
	SWORD swRetValue;
	DWORD dwAddr;
	BYTE bMsSubClass;

	pbDataBuf = (BYTE *) (((DWORD) (&bDataBuf) | 0xa0000000));
	pbSysParam= bSysParam;
	dwAddr = dwEntryAddr & 0x1FF;
	dwCopySize = 0x200 - dwAddr;
	dwSector = dwEntryAddr>>9;

	if (dwCopySize < 96)
	{
	    if((swRetValue = ProReadAttribute((DWORD)pbDataBuf, dwSector)))
	    {
			MP_DEBUG1("-E- ProReadAttribute FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
	    }

	    memcpy( pbSysParam, &pbDataBuf[dwAddr], dwCopySize);
	    pbSysParam += dwCopySize;
	    dwCopySize = 96 - dwCopySize;

	    if((swRetValue = ProReadAttribute((DWORD)pbDataBuf, dwSector + 1)))
	    {
			MP_DEBUG1("-E- ProReadAttribute FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
	    }
	    memcpy( pbSysParam, &pbDataBuf[0], dwCopySize);
	}
	else
	{
	    if((swRetValue = ProReadAttribute((DWORD)pbDataBuf, dwSector)))
	    {
			MP_DEBUG1("-E- ProReadAttribute FAIL (swRetValue: %d)", swRetValue);
			return swRetValue;
	    }
	    memcpy( pbSysParam, &pbDataBuf[dwAddr], dwCopySize);
	}

	if (*(bSysParam)!= 0x02)     //Memory Stick
	{
	    MP_DEBUG("-E- ms type FAIL: NOT Memory Stick ");
	    return FAIL;
	}

	//check memory stick device type
	if (*(bSysParam + 0x38) != 0x0)    // R/W
	{
		if (*(bSysParam + 0x38) <= 0x03)  // ReadOnly
		{									
			sInfo.bWriteProtected = 1;
		}
		else
		{
			MP_DEBUG("-E- ms type not support");
			return TYPE_NOT_SUPPORT;
		}
	}

	//check subclass 00xxxxxxb
	//bMsSubClass = *(bSysParam+0x2E);
	//if ( (bMsSubClass & 0x80)||(bMsSubClass & 0x40))     // for testor 2-7-20
	sInfo.bLockStatus = 0;

	//if( *(bSysParam+0x2E) & 0xc0)
	if(*(bSysParam+0x2E) & 0xC0)
	{								
	    mpDebugPrint("-I- read/write protected status");
	    sInfo.bLockStatus = 1;
	    sInfo.bWriteProtected = 1;
		return WRITE_PROTECT;
	}

	if( *(bSysParam + 0x33) == 1)
	{
	    *wBusWidth = BUS_WIDTH_4;
	}
	else
	{
	    *wBusWidth = BUS_WIDTH_1;
	}

	return PASS;
}


static SWORD RecognizeFileSystem(void)
{
#pragma alignvar(4)
	BYTE bDataBuf[MCARD_SECTOR_SIZE], bDataBuf1[MCARD_SECTOR_SIZE];
	BYTE *pbDataBuf;

	PARTITION  *par;
	BOOT_SECTOR *bootsector;
	SWORD  swRetValue;
	DWORD dwlbaTemp;

	pbDataBuf = (BYTE *) (((DWORD) (&bDataBuf) | 0xa0000000));
	MP_DEBUG("1");
	if((swRetValue = ProLogicalRead(0x00, 0x01, (DWORD) pbDataBuf)))
	{
		MP_DEBUG("-E- Read MBR Error : swRetValue : %d",swRetValue);
		return swRetValue;
	}
	IODelay(1000);
	MP_DEBUG("2");

    if((bDataBuf[0x00]!= 0xEB)&&(bDataBuf[0x00]!=0xE9))         // Super Floppy Format   T7-025
	{
		par = (PARTITION *)(pbDataBuf +0x1BE);
		MP_DEBUG("2 -1");
		if(par->Active != 0x80)               // Check 0x01BE   value    T7-022
		{
			MP_DEBUG("-E- MBR Partition Table Active = %x",par->Active);     
			return FAIL;
		}
		MP_DEBUG("2 - 2");
		if( *(WORD *)(pbDataBuf + 0x01FE) != 0x55AA)         // Check 0x01FE   value  0x55AA   T7-023
		{
			MP_DEBUG("-E- MBR Signature Code Error");
			return FAIL;
		}
		MP_DEBUG("2 -3");

		if((*(WORD *)(pbDataBuf + 0x01C6) == 0x00)&&(*(WORD *)(pbDataBuf + 0x01C8) == 0x00))
		{
			MP_DEBUG("-E- MBR contain value Error:  Size Error");
			return FAIL;
		}

		if((*(WORD *)(pbDataBuf + 0x01CA) == 0x00)&&(*(WORD *)(pbDataBuf + 0x01CC) == 0x00))
		{
			MP_DEBUG("-E- MBR contain value Error:  Start Address Error");
			return FAIL;
		}

		if(*(BYTE *)(pbDataBuf + 0x01C2) == 0x00)
		{
			MP_DEBUG("-E- MBR contain value Error:  Type Error");
			return FAIL;
		}
		MP_DEBUG("3");
		dwlbaTemp =  LoadAlien32(&par->Start);
		MP_DEBUG("4");
		pbDataBuf = (BYTE *) (((DWORD) (&bDataBuf1) | 0xa0000000));
		if((swRetValue = ProLogicalRead(dwlbaTemp, 0x01, (DWORD) pbDataBuf)))
		{
			MP_DEBUG("-E- Read PBR Error : swRetValue : %d",swRetValue);
			return swRetValue;
		}
		}

		bootsector = (BOOT_SECTOR *)(pbDataBuf);
		MP_DEBUG("5");
		if((bootsector->JumpCode[0] != 0xEB)&&(bootsector->JumpCode[0]!=0xE9))
		{
			MP_DEBUG("-E- PBR JumpCode Error  0x%x",bootsector->JumpCode[0]);
			return FAIL;
		}
		MP_DEBUG("6");
		if( *(WORD *)(pbDataBuf + 0x01FE) != 0x55AA)         // Check 0x01FE   value  0x55AA   T7-023
		{
			MP_DEBUG("-E- PBR Signature Code Error");
			return FAIL;
		}    

		return PASS;
}


static SWORD WaitTime1INT_FMT(void)
{
	MCARD *sMcard;
	DWORD dwTimeout;

	sMcard = (MCARD *)(MCARD_BASE);
	dwTimeout = 0x03000000;

	while(!(sMcard->McMsStatus & STATUS_CMDINT))
	{
		if(Polling_MS_Status())
			return FAIL;
		if( dwTimeout == 0)
		{
			return TPC_INT_TIMEOUT;
		}
		dwTimeout--;
	}

	return PASS;

}

static SWORD LowLevelFormat()
{
	register MCARD *sMcard = (MCARD *)(MCARD_BASE);
	register SWORD swRetValue;
	BYTE bIntValue;
#pragma alignvar(4)
	BYTE bBuffer[MCARD_SECTOR_SIZE];
	DWORD *pbBuffer;
	pbBuffer =  (DWORD *)((DWORD)(&bBuffer)|0xa0000000);

	mpDebugPrint("Format CMD");
	sMcard->McMsT1 = PRO_TIMEOUT1*3;
	if(sInfo.bWriteProtected == 1)
	{
		return WRITE_PROTECT;
	}

	SetProParaReg( bProFuncCMD[PRO_FORMAT], FMT_READ_TPC|FORMAT_MODE, 0x00, 0x00, 0x00);
	if((swRetValue = WriteTpcBs(TPC_EX_SET_CMD, (BYTE *)(&sProParamReg), 7)))
	{
		MP_DEBUG1("-E- TPC_EX_SET_CMD FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if((swRetValue = WaitTime1INT_FMT()))
	{
		MP_DEBUG1("-E- EX WaitTime1INT_FMT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if((swRetValue = GetIntValue(&bIntValue)))
	{
		MP_DEBUG1("-E- EX GetINT FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if((bIntValue & CMDNK) || ((bIntValue & CED) && (bIntValue & ERR)))
	{
		MP_DEBUG("-E- EX CMD Error termination (bIntValue: %d)", bIntValue);

		return FAIL;
	}

	sMcard->McMsT1 = PRO_TIMEOUT1*3;
	while(1)
	{
		SetDataDma((DWORD)pbBuffer);
		if((swRetValue = ReadTpcBsPage(ACCESS_LONG_DATA)))
		{
			MP_DEBUG("-E- ReadTpcBsPage FAIL( swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		if((swRetValue = WaitTime1INT_FMT()))
		{
			MP_DEBUG("-E- GetINT TIMEOUT (swRetValue: %d)", swRetValue);
			return swRetValue;
		}

		if((swRetValue = GetIntValue(&bIntValue)))
		{
			MP_DEBUG("-E- GetINT FAIL (swRetValue : %d)",swRetValue);
			return swRetValue;
		}

		if((bIntValue & CMDNK) || ((bIntValue & CED)&&(bIntValue & ERR)))
		{
			MP_DEBUG("-E- CMD Error termination (bIntValue: %d)", bIntValue);
			return FAIL;
		}
		else if( bIntValue & BREQ)
		{
			mpDebugPrint("ALL  Sect: %d   Processd Sect: %d", pbBuffer[0], pbBuffer[1]);                     
		}
		else if( bIntValue & CED)
		{
			mpDebugPrint("MS  FORMAT FINISH!!");
			break;
		}
		WaitDataTransfer();
	}

	return PASS;
}

#endif //#if MS_ENABLE //byAlexWang 24jun2007 m2project

