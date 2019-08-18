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
* Filename      : global612.c
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"

#include "devio.h"
#include "ui.h"
#include "taskid.h"
#include "flagdefine.h"
#include "peripheral.h"
#include "bios.h"
#include "Usbotg.h"


///////////////////////////////////////////////////////////////////////////
//
//  Variable declarations
//
///////////////////////////////////////////////////////////////////////////
//HW register pointer
AIU         *g_psAiu = (AIU *) AIU_BASE;
BIU         *g_psBiu = (BIU *) BIU_BASE;
CLOCK       *g_psClock = (CLOCK *) CLOCK_BASE;
DMA         *g_psDma = (DMA *) DMA_BASE;
CHANNEL     *g_psDmaAiu = (CHANNEL *) DMA_AIU_BASE;
GPIO        *g_psGpio = (GPIO *) GPIO_BASE;
#if ((CHIP_VER & 0xffff0000) == CHIP_VER_615)
CDU         *g_psCDU = (CDU *) CDU_BASE;
IPU         *g_psIpu = (IPU *) IPU_BASE;
#endif

DWORD g_bAniFlag = 0;

//b[0:5]: Index of volume matrix
//b[6]: When this bit is set, it means the volume was changed but didn't be written
//into ALC202 yet.
//b[7]: Mute flag.  1:Mute 0:Normal
BYTE g_bVolumeIndex = ((VOLUME_DEGREE >> 1) | 0x40);
#pragma alignvar(4)

PCLKSS1 g_psClkSs1 = (PCLKSS1) &((CLOCK *) CLOCK_BASE)->Clkss1;
PCLKSS2 g_psClkSs2 = (PCLKSS2) &((CLOCK *) CLOCK_BASE)->Clkss2;
ST_IMGWIN g_sMovieWin;

///////////////////////////////////////////////////////////////////////////
//
//  Constant declarations
//
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
//
//  Static function prototype
//
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
//
//  Definition of external functions
//
///////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------------
static SBYTE tempString[64];

BYTE *GetSystemErrMsg(DWORD dwErrorCode)
{
    memset(tempString, 0, 64);

    //strcpy(tempString,"MPX Greeting");
    strcpy(tempString,"Not Supported");

    switch(dwErrorCode)
    {
    case ERR_MEM_MALLOC:
        strcpy(tempString,"Memory malloc error");
        break;

    case ERR_NO_STORAGE:
        strcpy(tempString,"No Storage");
        break;

    case ERR_USER_CANCEL_COPY:
    case ERR_Print_is_canceled:
        strcpy(tempString,"User canceled");
        break;

    case ERR_FILE_SYSTEM:
        strcpy(tempString,"File System Error");
        break;

    case ERR_JPG_SOUR_OVERFLOW:
        strcpy(tempString,"Jpeg file too large");
        break;

    case ERR_WRITE_PROTECT:
        strcpy(tempString,"Write protect");
        break;

    case ERR_DISK_FULL:
        strcpy(tempString,"Card is full");
        break;

    case ERR_IMAGE_DECODER:
        //strcpy(tempString,"Image Decoder Error");
        break;

    case ERR_JPG_DECODE:
        //strcpy(tempString,"Jpeg Decode Error");
        break;

    case ERR_FILE_FORMAT:
        strcpy(tempString,"File format not supported");
        break;

    case ERR_VIDEO_INVALID:
        //strcpy(tempString,"Invalid File Format");
        break;

    case ERR_VIDEO_DECOMPRESSOR:
        //strcpy(tempString,"No Suitable Decompressor");
        break;

    case ERR_VIDEO_FORMAT:
        //strcpy(tempString,"Format Is Not Supported");
        break;

    case ERR_VIDEO_OPENFILE:
        strcpy(tempString,"Cannot Open File");
        break;

    case ERR_VIDEO_ERROR:
        //strcpy(tempString,"Media File Error");
        break;

    case ERR_VIDEO_STREAM:
        strcpy(tempString,"Stream Initialize Error");
        break;

    case ERR_VIDEO_DEMUX:
        //strcpy(tempString,"No Suitable Demux Found");
        break;

    case ERR_VIDEO_DEMUX_OPEN:
        //strcpy(tempString,"Demux Open Failed");
        break;

    case ERR_VIDEO_CODEC:
        strcpy(tempString,"Not Supported Video Codec");
        break;

    case ERR_AUDIO_CODEC:
        strcpy(tempString,"Not Supported Audio Codec");
        break;

    case ERR_VIDEO_OUT:
        strcpy(tempString,"Video Out Initialize Failed");
        break;

    case ERR_AV_STYSTEM:
        strcpy(tempString,"AV system error");
        break;

    case ERR_AUDIO_OUT:
        strcpy(tempString,"Audio Out Initialize Failed");
        break;

    case ERR_BITRATE:
        strcpy(tempString,"Bitrate Too High. Disable Audio?");
        break;

    case ERR_RESOLUTION:
        strcpy(tempString,"Resolution Too High. Disable Audio?");
        break;

    case ERR_PICT_FATAL_ERROR:
        strcpy(tempString,"Printer Fatal Error");
        break;

    case ERR_PICT_WARNING_ERROR:
        strcpy(tempString,"Printer Warning Error");
        break;

    case ERR_PICT_PAPER:
        strcpy(tempString,"Paper Error");
        break;

    case ERR_PICT_PAPER_EMPTY:
        strcpy(tempString,"There is no paper.");
        break;

    case ERR_PICT_PAPER_JAM:
        strcpy(tempString,"There is paper jam.");
        break;

    case ERR_PICT_INK:
        strcpy(tempString,"Ink Error.");
        break;

    case ERR_PICT_INK_EMPTY:
        strcpy(tempString,"There is no ink.");
        break;

    case ERR_PICT_HARDWARE:
        strcpy(tempString,"Printer Hardware Error.");
        break;

    case ERR_PICT_HW_COVER_OPEN:
        strcpy(tempString,"Printer cover is open.");
        break;

    case ERR_PICT_FILE:
        strcpy(tempString,"Printer File Error.");
        break;

    case ERR_PICT_FILE_DECODE_ERROR:
        strcpy(tempString,"File format is not supported by printer.");
        break;

    case ERR_PICT_DISCONNECTED:
        strcpy(tempString,"Printer disconnected.");
        break;

    case ERR_AVSYNC:
        strcpy(tempString,"AVSync Performance Not Enough. Disable Audio?");
        break;

    case ERR_NO_SECOND_STORAGE:
        strcpy(tempString,"No Target Storage");
        break;

    case ERR_REMOVE_CURDEV :
        strcpy(tempString,"Current drive removed.");
        break;

    case ERR_USBH_DETECT_ERROR:
        strcpy(tempString,"USB device detects fail.");
        break;
	case ERR_NOTSUPPORT_OVER720P:
        strcpy(tempString,"Not support frame size over 720P");
        break;
	case ERR_Video_MEMORY_NOTENOUGH:
		break;

    default:
        //strcpy(tempString,"Not Supported");
        break;
    }

    return  tempString;
    //return 0;
}



inline void SystemChkChipVer(void)
{
    DWORD dwChipVer = g_psBiu->BiuChid;

    //MP_ALERT("Chip is %08X", dwChipVer);
    MP_ALERT("Chip is %08X, CHIP_VER = 0x%08X(RevB or above = 0x0001)", dwChipVer, CHIP_VER);

//    if (dwChipVer != CHIP_VER)
//        MP_ALERT("Wrong Chip Version");
}



void ScalerClockReduce()
{
    if (mGetMemCks() == MEMCKS_PLL1)
        mSetScaCks(SCACKS_PLL1_DIV_3);
    else
        mSetScaCks(SCACKS_PLL2_DIV_3);
}



void ScalerClockReset()
{
    if (mGetMemCks() == MEMCKS_PLL1)
        mSetScaCks(SCACKS_PLL1_DIV_2);
    else
        mSetScaCks(SCACKS_PLL2_DIV_2);
}



/////////////////////////////////////////////////////////////////////
//
//
//
/////////////////////////////////////////////////////////////////////
static DWORD relativeMs = 0;

DWORD GetRelativeMs(void)
{
    DWORD r;

    r = SystemGetElapsedTime(relativeMs);
    relativeMs = GetSysTime();

    return r;
}



//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
typedef enum _E_POLL_TYPE
{
    PollTypeByte = 0,
    PollTypeWord,
    PollTypeDWord,
    PollTypeFunc,
} E_POLL_TYPE;

static int GeneralPoll(E_POLL_TYPE type, int is_busy, DWORD us, volatile void *arg1, DWORD arg2, DWORD arg3)
{
    DWORD start_time = 0;
    DWORD current_time = 0;

    if (us == 0)
        return 0;               /* fail */

    us = (us + 100000 - 1) / 100000;    /* In MP612, each timestamp is 100,000 us */
    start_time = SystemGetTimeStamp();

    while (current_time < us)
    {
        if (type == PollTypeByte)
        {
            if ((*(volatile BYTE *) arg1 & (BYTE) arg3) == ((BYTE) arg2 & (BYTE) arg3))
                break;
        }
        else if (type == PollTypeWord)
        {
            if ((*(volatile WORD *) arg1 & (WORD) arg3) == ((WORD) arg2 & (WORD) arg3))
                break;
        }
        else if (type == PollTypeDWord)
        {
            if ((*(volatile DWORD *) arg1 & (DWORD) arg3) == ((DWORD) arg2 & (DWORD) arg3))
                break;
        }
        else if (type == PollTypeFunc)
        {
            int (*poll_func) (void *);

            poll_func = (int (*)(void *)) arg1;

            if (poll_func((void *) arg2) > 0)
            {
                break;
            }
        }
        else
        {
            return 0;           /* fail */
        }

        if (!is_busy)
            TaskYield();

        current_time = SystemGetElapsedTime(start_time);
    }

#if 0
    if (current_time >= us)
    {
        return 0;               /* fail */
    }
#endif

    return 1;                   /* success */
}



//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
int PollByte(BYTE * polling_data, BYTE target_value, DWORD us)
{
    return GeneralPoll(PollTypeByte, 0, us, (void *) polling_data, (DWORD) target_value, 0xFF);
}



//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
int PollWord(WORD * polling_data, WORD target_value, DWORD us)
{
    return GeneralPoll(PollTypeWord, 0, us, (void *) polling_data, (DWORD) target_value, 0xFFFF);
}



//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
int PollDWord(DWORD * polling_data, DWORD target_value, DWORD us)
{
    return GeneralPoll(PollTypeDWord, 0, us, (void *) polling_data, (DWORD) target_value,
                       0xFFFFFFFF);
}



/*
//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
int Poll(int (*polling_func) (void *), void *func_data, DWORD us)
{
    return GeneralPoll(PollTypeFunc, 0, us, polling_func, (DWORD) func_data, 0);
}



//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
int BusyPollByte(BYTE * polling_data, BYTE target_value, DWORD us)
{
    return GeneralPoll(PollTypeByte, 1, us, (void *) polling_data, (DWORD) target_value, 0xFF);
}



//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
int BusyPollWord(WORD * polling_data, WORD target_value, DWORD us)
{
    return GeneralPoll(PollTypeWord, 1, us, (void *) polling_data, (DWORD) target_value, 0xFFFF);
}



//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
int BusyPollDWord(DWORD * polling_data, DWORD target_value, DWORD us)
{
    return GeneralPoll(PollTypeDWord, 1, us, (void *) polling_data, (DWORD) target_value,
                       0xFFFFFFFF);
}



//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
int BusyPoll(int (*polling_func) (void *), void *func_data, DWORD us)
{
    return GeneralPoll(PollTypeFunc, 1, us, polling_func, (DWORD) func_data, 0);
}



//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
int _PollRegister8(volatile BYTE * polling_data, BYTE target_value, BYTE register_mask, DWORD us)
{
    return GeneralPoll(PollTypeByte, 0, us, (void *) polling_data, (DWORD) target_value,
                       register_mask);
}



//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
int _PollRegister16(volatile WORD * polling_data, WORD target_value, WORD register_mask, DWORD us)
{
    return GeneralPoll(PollTypeWord, 0, us, (void *) polling_data, (DWORD) target_value,
                       register_mask);
}
*/



//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
int _PollRegister32(volatile DWORD *polling_data, DWORD target_value, DWORD register_mask, DWORD us)
{
    return GeneralPoll(PollTypeDWord, 0, us, (void *) polling_data, (DWORD) target_value,
                       register_mask);
}



/*
//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
int _BusyPollRegister8(volatile BYTE * polling_data, BYTE target_value, BYTE register_mask, DWORD us)
{
    return GeneralPoll(PollTypeByte, 1, us, (void *) polling_data, (DWORD) target_value,
                       register_mask);
}



//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
int _BusyPollRegister16(volatile WORD * polling_data, WORD target_value, WORD register_mask, DWORD us)
{
    return GeneralPoll(PollTypeWord, 1, us, (void *) polling_data, (DWORD) target_value,
                       register_mask);
}
*/



int _BusyPollRegister32(volatile DWORD * polling_data, DWORD target_value, DWORD register_mask,
                    DWORD us)
{
    return GeneralPoll(PollTypeDWord, 1, us, (void *) polling_data, (DWORD) target_value,
                       register_mask);
}





//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
static BYTE eventId = UI_EVENT;
static EVENT_POLLING_CALLBACK eventPollingCallback[32] = {0};

SDWORD SystemPollingEventIdSet(BYTE newEventId)
{
    if (newEventId >= TOTAL_OBJECT_NUMBER)
        return FAIL;

    eventId = newEventId;

    return PASS;
}



SDWORD SystemPollingEventHandlerRegister(DWORD eventBitField, EVENT_POLLING_CALLBACK funPtr)
{
    DWORD i = 0;

    for (i = 0; i < 32; i++)
    {
        if (eventBitField & (BIT0 << i))
        {
            eventPollingCallback[i] = funPtr;

            return PASS;
        }
    }

    return FAIL;
}


static WORD pollingEventEntryTimes = 0;

BOOL Polling_Event(void)
{
    BOOL blRet = 0;
    DWORD dwEvent, i;

    IntDisable();

    if (pollingEventEntryTimes > 0)
    {
        TaskYield();
        //IntEnable();

        return 0;
    }

    pollingEventEntryTimes = 1;
    //IntEnable();

    if (EventPolling(eventId, 0xFFFFFFFF, OS_EVENT_OR, &dwEvent) == OS_STATUS_OK)
    {
        for (i = 0; i < 32; i++)
        {
            if (dwEvent & (BIT0 << i))
            {
                if (eventPollingCallback[i])
                    blRet |= eventPollingCallback[i]();
                else
                {
                    switch (i)
                    {
                    case EVENT_TIMER:
                        break;

                    defalut:
                        MP_ALERT("-W- Polling_Event BIT%02d - Null", i);
                        break;
                    }
                }
            }
        }
    }

    IntDisable();
    pollingEventEntryTimes = 0;
    IntEnable();

    if (blRet == 0)
        TaskYield();
    else
        MP_DEBUG("Polling_Event - %1d, Task ID=%2d", blRet, TaskGetId());

    return blRet;
}


//////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////
void SystemExceptionInit(void)
{
    MP_DEBUG("%s -", __FUNCTION__);

#if (RTC_AUTO_CORRECT_EN == 1)
    SysTimerProcAdd(1000, RTC_SetReservedCount, FALSE);
#endif
    SystemCardExceptionRegister();

#if (SC_USBDEVICE)
    SystemUsbDetectInit();
#endif
}



#if (BLUETOOTH == 1)
#define BT_INIT_BIT				0x30303031  //0001

int IspReadBTDevice(BYTE *trg, SWORD size, SWORD *fp)
{
    SWORD status,i;
    WORD maxSector;
    BYTE *pbbuf;
    WORD *pwbuf;
    DWORD *pdwbuf;
    SDWORD retVal = PASS;
//    MP_DEBUG("-%s()-", __FUNCTION__);
    if(Sys_SettingTableOpen() != PASS)
    {
        mpDebugPrint("Can't open bt setting table.");
        return FAIL;
    }
    pdwbuf = (DWORD *) ext_mem_malloc(2048);
    pbbuf = (BYTE *) pdwbuf;
    memset(pbbuf,0,2048);
    if((retVal = Sys_SettingTableGet("MPBT", pbbuf, 2048)) == 0)                // No setting.sys exist, build it
    {
        MP_DEBUG("setting.sys put succeed");
    }
    if(*fp == 0)
    {
//        mpDebugPrint("\r\n pdwbuf[0] = 0x%x \r\n",pdwbuf[0]);
//        mpDebugPrint("\r\n pdwbuf[1] = 0x%x \r\n",pdwbuf[1]);
        if((pdwbuf[0] != BT_TAG)||(pdwbuf[1] != BT_INIT_BIT))
        {
            *fp = -1;
            ext_mem_free(pdwbuf);
//            mpDebugPrint("\r\n No BT_TAG \r\n");
    	    return FAIL;
        }
        *fp += 8;
    }
    memcpy(trg,pbbuf+(*fp),size);
    *fp += size;
//    mpDebugPrint("\r\n *fp = 0x%x",*fp);
    ext_mem_free(pdwbuf);
    if(retVal = Sys_SettingTableClose() == FAIL)                                // save setting table to system drive
    {
        MP_ALERT("-E- Can't save bt setting table to system drive!!!");
    return FAIL;
}
    MP_DEBUG("bt setting.sys close succeed");
    return retVal;
}



int IspWriteBTDevice(BYTE *data, uint16_t len)
{
    SDWORD retVal = PASS;
//    MP_ALERT("-%s()-", __FUNCTION__);
    if(Sys_SettingTableOpen() != PASS)
    {
        MP_ALERT("--E-- %s- Can't open bt setting table.", __FUNCTION__);
        return FAIL;
    }
    if(Sys_SettingTablePut("MPBT", data, len)!= len)
    {
        MP_DEBUG("setting.sys put fail");
    }
    if(retVal = Sys_SettingTableClose() == FAIL)                                // save setting table to system drive
    {
        MP_ALERT("-E- Can't save setting table to system drive!!!");
        return FAIL;
    }
    return PASS;
}
#endif

int isUTF8Style1(BYTE b1)                 
{
	return (((b1)&0x80)==0) ? 1 : 0;
}
int isUTF8Style2(BYTE b1, BYTE b2)
{
	return ((((b1)&0xE0)==0xC0) && (((b2)&0xC0)==0x80)) ? 1 : 0;
}
int isUTF8Style3(BYTE b1, BYTE b2, BYTE b3)           
{
	return ( (((b1)&0xF0)==0xE0) && (((b2)&0xC0)==0x80) && (((b3)&0xC0)==0x80) ) ? 1 : 0;
}
int isUTF8Style4(BYTE b1, BYTE b2, BYTE b3, BYTE b4)
{
	return ( (((b1)&0xF8)==0xF0) && (((b2)&0xC0)==0x80) && (((b3)&0xC0)==0x80) && (((b4)&0xC0)==0x80) ) ? 1 : 0;
}
int isUTF8Style5(BYTE b1, BYTE b2, BYTE b3, BYTE b4, BYTE b5)
{
	return ( (((b1)&0xFC)==0xF8) && (((b2)&0xC0)==0x80) && (((b3)&0xC0)==0x80) && (((b4)&0xC0)==0x80) && (((b5)&0xC0)==0x80) ) ? 1 : 0;
}
int isUTF8Style6(BYTE b1, BYTE b2, BYTE b3, BYTE b4, BYTE b5, BYTE b6)
{
	return ( (((b1)&0xFE)==0xFC) && (((b2)&0xC0)==0x80) && (((b3)&0xC0)==0x80) && (((b4)&0xC0)==0x80) && (((b5)&0xC0)==0x80) && (((b6)&0xC0)==0x80) ) ? 1 : 0;
}
unsigned UTF8ToDCharS1(BYTE b1)                
{
	return ( (unsigned)(b1) );
}
unsigned UTF8ToDCharS2(BYTE b1, BYTE b2)             
{
	return ( ((((unsigned)(b1))&0x1F)<<6)  | (((unsigned)(b2))&0x3F) );
}
unsigned UTF8ToDCharS3(BYTE b1, BYTE b2, BYTE b3)          
{
	return ( ((((unsigned)(b1))&0x0F)<<12) | ((((unsigned)(b2))&0x3F)<<6) | (((unsigned)(b3))&0x3F) );
}
unsigned UTF8ToDCharS4(BYTE b1, BYTE b2, BYTE b3, BYTE b4)       
{
	return ( ((((unsigned)(b1))&0x07)<<18) | ((((unsigned)(b2))&0x3F)<<12) | ((((unsigned)(b3))&0x3F)<<6) | (((unsigned)(b4))&0x3F) );
}
unsigned UTF8ToDCharS5(BYTE b1, BYTE b2, BYTE b3, BYTE b4, BYTE b5)    
{
	return ( ((((unsigned)(b1))&0x03)<<24) | ((((unsigned)(b2))&0x3F)<<18) | ((((unsigned)(b3))&0x3F)<<12) | ((((unsigned)(b4))&0x3F)<<6) | (((unsigned)(b5))&0x3F) );
}
unsigned UTF8ToDCharS6(BYTE b1, BYTE b2, BYTE b3, BYTE b4, BYTE b5, BYTE b6) 
{
	return ( ((((unsigned)(b1))&0x01)<<30) | ((((unsigned)(b2))&0x3F)<<24) | ((((unsigned)(b3))&0x3F)<<18) | ((((unsigned)(b4))&0x3F)<<12) | ((((unsigned)(b5))&0x3F)<<6) | (((unsigned)(b6))&0x3F) );
}

// UTF-8 to UTF-32, donot check valid unicode range
void U8BuffToU32Char(const unsigned char* pbUtf8Buff, int* pUtf8len, unsigned* pU32Char, int* pValid)
{
    const unsigned char *pb = pbUtf8Buff;
    if ( isUTF8Style1(pb[0]) )                              // 1 byte
    {
        *pU32Char = UTF8ToDCharS1(pb[0]);
        *pUtf8len = 1;
        *pValid = 1;
    }
    else if ( isUTF8Style2(pb[0], pb[1]) )                  // 2 bytes
    {
        *pU32Char = UTF8ToDCharS2(pb[0], pb[1]);
        *pUtf8len = 2;
        *pValid = 1;
    }
    else if ( isUTF8Style3(pb[0], pb[1], pb[2]) )           // 3 bytes
    {
        *pU32Char = UTF8ToDCharS3(pb[0], pb[1], pb[2]);
        *pUtf8len = 3;
        *pValid = 1;
    }
    else if ( isUTF8Style4(pb[0], pb[1], pb[2], pb[3]) )    // 4 bytes
    {
        *pU32Char = UTF8ToDCharS4(pb[0], pb[1], pb[2], pb[3]);
        *pUtf8len = 4;
        *pValid = 1;
    }
    else if ( isUTF8Style5(pb[0], pb[1], pb[2], pb[3], pb[4]) ) // 5 bytes
    {
        *pU32Char = UTF8ToDCharS5(pb[0], pb[1], pb[2], pb[3], pb[4]);
        *pUtf8len = 5;
        *pValid = 1;
    }
    else if ( isUTF8Style6(pb[0], pb[1], pb[2], pb[3], pb[4], pb[5]) )  // 6 bytes
    {
        *pU32Char = UTF8ToDCharS6(pb[0], pb[1], pb[2], pb[3], pb[4], pb[5]);
        *pUtf8len = 6;
        *pValid = 1;
    }
    else
    {
        *pU32Char = pb[0];
        *pUtf8len = 1;
        *pValid = 0;
    }
}

void U32CharToU8Buff(unsigned u32char, unsigned char* pbUtf8Buff, int* pUtf8len, int* pValid)
{
    if ( u32char < 0x80 )               // 7-bit
    {
        *pbUtf8Buff = (unsigned char)u32char;
        *pUtf8len = 1;
        *pValid = 1;
    }
    else if ( u32char < 0x800 )         // 11-bit
    {
        *pbUtf8Buff = (unsigned char) ((u32char>>6)|0xC0);
        *(pbUtf8Buff+1) = (unsigned char) (u32char & 0x3F | 0x80);
        *pUtf8len = 2;
        *pValid = 1;
    }
    else if ( u32char < 0x10000 )       // 16-bit
    {
        *pbUtf8Buff = (unsigned char) ((u32char>>12)|0xE0);
        *(pbUtf8Buff+1) = (unsigned char) ((u32char >> 6) & 0x3F | 0x80);
        *(pbUtf8Buff+2) = (unsigned char) (u32char & 0x3F | 0x80);
        *pUtf8len = 3;
        *pValid = 1;
    }
    else if ( u32char < 0x200000 )      // 21-bit
    {
        *pbUtf8Buff = (unsigned char) ((u32char>>18)|0xF0);
        *(pbUtf8Buff+1) = (unsigned char) ((u32char >> 12) & 0x3F | 0x80);
        *(pbUtf8Buff+2) = (unsigned char) ((u32char >> 6) & 0x3F | 0x80);
        *(pbUtf8Buff+3) = (unsigned char) (u32char & 0x3F | 0x80);
        *pUtf8len = 4;
        *pValid = 1;
    }
    else
    {
        *pbUtf8Buff = (unsigned char)u32char;
        *pUtf8len = 1;
        *pValid = 0;
    }
}



