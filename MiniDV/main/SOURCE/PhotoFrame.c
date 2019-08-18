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
* Filename      : PhotoFrame.c
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  :
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "Peripheral.h"
#include "PhotoFrame.h"
#include "Peripheral.h"
#include "ui_timer.h"
#include "camcorder_func.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif

#if PHOTO_FRAME
static ST_PHOTO_FRAME_INFO stPhotoFrameInfo[2] = {{0}};
static ST_TIMESTAMP_INFO stTimeStampInfo[2] = {{0}};

WORD *TimeStampStr;

SDWORD TimeStampFontCopy(ST_TIMESTAMP_INFO *stTimeStampInfoPtr, ST_TIMESTAMP_FONT_INFO *stFontInfoPtr, WORD startX, WORD startY)
{
    MP_DEBUG("%s: startX = %d, startY = %d", __func__, startX, startY);
    WORD row, col;
    DWORD *srcBufPtr, *destBufPtr;

    for (row = 0; row < stFontInfoPtr->wHeight; row++)
    {
        destBufPtr = (DWORD *) (stTimeStampInfoPtr->dwPhotoFrameWorkingBuf + stTimeStampInfoPtr->dwOffset * (row + startY) + (startX << 1));
        srcBufPtr = (DWORD *) (stFontInfoPtr->dwPointerAddr + stFontInfoPtr->dwOffset * row);

        for (col = 0; col < (stFontInfoPtr->wWidth >> 1); col++)
        {
            if (*srcBufPtr != stFontInfoPtr->colorKey)
            {
                *destBufPtr = *srcBufPtr; // 0xFFFF8080 - ¥þ¥Õ debug
            }

            srcBufPtr++;
            destBufPtr++;
        }
    }
}



void PhotoFrameInit(void)
{
    MP_DEBUG("%s -", __FUNCTION__);

    MpMemSet((BYTE *) &stPhotoFrameInfo[0], 0, sizeof(stPhotoFrameInfo));
    TimeStampFontInit();
}



SDWORD PhotoFrameSetup(const BYTE *frameBuf, WORD posX, WORD posY, BOOL forPreview)
{
    WORD *wFontBuf;
    ST_IMGWIN stSrcSubWin;
    BYTE mode;
    MP_DEBUG("%s -", __FUNCTION__);
    posX &= 0xFFFE;
    if (forPreview)
        mode = 0;
    else
        mode = 1;

    if (stPhotoFrameInfo[mode].dwBufferAddr)
    {
        ext_mem_free((void *) stPhotoFrameInfo[mode].dwBufferAddr);
        stPhotoFrameInfo[mode].dwBufferAddr = 0;
    }

    wFontBuf = (WORD *) frameBuf;
    stPhotoFrameInfo[mode].wWidth = wFontBuf[0];
    stPhotoFrameInfo[mode].wHeight = wFontBuf[1];
    stPhotoFrameInfo[mode].dwColorKey = *(DWORD *) &wFontBuf[2];
    stPhotoFrameInfo[mode].dwLineOffset = stPhotoFrameInfo[mode].wWidth << 1;
    stPhotoFrameInfo[mode].dwBufferAddr = (DWORD) ext_mem_malloc(stPhotoFrameInfo[mode].dwLineOffset * stPhotoFrameInfo[mode].wHeight);

    if (stPhotoFrameInfo[mode].dwBufferAddr == 0)
    {
        MP_ALERT("--E-- Out of memory for initial photo frame");

        return FAIL;
    }

    memcpy((BYTE *) stPhotoFrameInfo[mode].dwBufferAddr, (BYTE *) &wFontBuf[4], stPhotoFrameInfo[mode].dwLineOffset * stPhotoFrameInfo[mode].wHeight);
    MP_DEBUG("YUV Photo Frame - 0x%08X With = %d, Height = %d, ColorKey = 0x%08X",
             stPhotoFrameInfo[mode].dwBufferAddr,
             stPhotoFrameInfo[mode].wWidth, stPhotoFrameInfo[mode].wHeight,
             stPhotoFrameInfo[mode].dwColorKey);

    stSrcSubWin.pdwStart = (DWORD *) stPhotoFrameInfo[mode].dwBufferAddr;
    stSrcSubWin.dwOffset = stPhotoFrameInfo[mode].dwLineOffset;
    stSrcSubWin.wWidth = stPhotoFrameInfo[mode].wWidth;
    stSrcSubWin.wHeight = stPhotoFrameInfo[mode].wHeight;
    stSrcSubWin.wX = posX;
    stSrcSubWin.wY = posY;
    
    MP_DEBUG("%s -", __FUNCTION__);
    MP_DEBUG("stSrcSubWin.pdwStart = 0x%08X", stSrcSubWin.pdwStart);
    MP_DEBUG("stSrcSubWin.dwOffset = %d", stSrcSubWin.dwOffset);
    MP_DEBUG("stSrcSubWin.wWidth, wHeight = %d x %d", stSrcSubWin.wWidth, stSrcSubWin.wHeight);
    MP_DEBUG("stSrcSubWin.wX, wY = %d, %d", stSrcSubWin.wX, stSrcSubWin.wY);

    if (forPreview)
    {
        MP_DEBUG("--- forPreview = %d ---", forPreview);
        SetSensorOverlayParam(&stSrcSubWin,
                              stPhotoFrameInfo[mode].dwColorKey >> 24,
                              (stPhotoFrameInfo[mode].dwColorKey >> 8) & 0xFF,
                              stPhotoFrameInfo[mode].dwColorKey & 0xFF);
    }
    else
    {
        MP_DEBUG("--- forPreview = %d ---", forPreview);
        SetSensorCaptureOverlayParam(&stSrcSubWin,
                                     stPhotoFrameInfo[mode].dwColorKey >> 24,
                                     (stPhotoFrameInfo[mode].dwColorKey >> 8) & 0xFF,
                                     stPhotoFrameInfo[mode].dwColorKey & 0xFF);
    }
    SetSensorOverlayEnable(ENABLE);
    return PASS;
}



void PhotoFrameRelease(BOOL forPreview)
{
    BYTE mode;

    MP_DEBUG("%s -", __FUNCTION__);

    if (forPreview)
        mode = 0;
    else
        mode = 1;

    if (stPhotoFrameInfo[mode].dwBufferAddr)
        ext_mem_free((void *) stPhotoFrameInfo[mode].dwBufferAddr);

    MpMemSet((BYTE *) &stPhotoFrameInfo[mode], 0, sizeof(ST_PHOTO_FRAME_INFO));
    TimeStampFontRelease(mode);

    if ((stPhotoFrameInfo[0].dwBufferAddr == 0) && (stPhotoFrameInfo[1].dwBufferAddr == 0))
        SetSensorOverlayEnable(DISABLE);
}


//------------------------------------------------------------------------------
// TimeStamp
//------------------------------------------------------------------------------

void TimeStampFontInit(void)
{
    MP_DEBUG("%s -", __FUNCTION__);

    MpMemSet((BYTE *) &stTimeStampInfo[0], 0, sizeof(stTimeStampInfo));
}



SDWORD TimeStampFontSetup(const BYTE *fontBuf, DWORD bufSize, BOOL forPreview)
{
    MP_DEBUG("%s - bufSize = %d, forPreview = %d", __func__, bufSize, forPreview);
    ST_IMAGEINFO stYuvBuffer;
    DWORD colorKey, fontSize;
    WORD *wFontBuf, i;
    BYTE mode;

    MP_DEBUG("%s -", __FUNCTION__);

    if (forPreview)
        mode = 0;
    else
        mode = 1;
    MP_DEBUG("mode = %d", mode);

    wFontBuf = (WORD *) fontBuf;
    stYuvBuffer.wWidth = wFontBuf[0];
    stYuvBuffer.wHeight = wFontBuf[1];
    colorKey = ((DWORD) wFontBuf[2] << 16) + wFontBuf[3];
    stYuvBuffer.dwOffset = stYuvBuffer.wWidth << 1;
    fontSize = stYuvBuffer.dwOffset * stYuvBuffer.wHeight;
    mpDebugPrint("stYuvBuffer.dwOffset = %d, fontSize = %d", stYuvBuffer.dwOffset, fontSize);

    if ((8 + (fontSize * E_TIME_STAMP_FONT_TOTAL)) != bufSize)
    {
        MP_ALERT("\r\n--E-- Font Size is %d, total size should be %d", fontSize, 8 + (fontSize * E_TIME_STAMP_FONT_TOTAL));
        MP_ALERT("--E-- Buffer size is %d, not match !!!", bufSize);

        return FAIL;
    }

    MP_ALERT("Font size is %d x %d, Color Key is 0x%08X", stYuvBuffer.wWidth, stYuvBuffer.wHeight, colorKey);

    for (i = 0; i < E_TIME_STAMP_FONT_TOTAL; i++)
    {
        stTimeStampInfo[mode].stFontInfo[i].dwPointerAddr = (DWORD) ext_mem_malloc(fontSize);

        if (stTimeStampInfo[mode].stFontInfo[i].dwPointerAddr == 0)
        {
            MP_ALERT("\r\n--E-- Out of memory for initial time stamp font !!!");

            for (i = 0; i < E_TIME_STAMP_FONT_TOTAL; i++)
            {
                if (stTimeStampInfo[mode].stFontInfo[i].dwPointerAddr)
                {
                    ext_mem_free((void *) stTimeStampInfo[mode].stFontInfo[i].dwPointerAddr);
                    stTimeStampInfo[mode].stFontInfo[i].dwPointerAddr = 0;
                }
            }

            return FAIL;
        }

        memcpy((BYTE *) stTimeStampInfo[mode].stFontInfo[i].dwPointerAddr, (BYTE *) &fontBuf[8 + i * fontSize], fontSize);
        stTimeStampInfo[mode].stFontInfo[i].colorKey = colorKey;
        stTimeStampInfo[mode].stFontInfo[i].wWidth = stYuvBuffer.wWidth;
        stTimeStampInfo[mode].stFontInfo[i].wHeight = stYuvBuffer.wHeight;
        stTimeStampInfo[mode].stFontInfo[i].dwOffset = stYuvBuffer.dwOffset;
    }

    return PASS;
}



SDWORD TimeStampInfoSetup(WORD x, WORD y, BOOL forPreview)
{
    DWORD srcBufAddr, size;
    BYTE mode;

    MP_DEBUG("%s : x = %d, y = %d", __func__, x, y);

    if (forPreview)
        mode = 0;
    else
        mode = 1;

    if (stTimeStampInfo[mode].dwPhotoFrameBackupBuf)
    {
        ext_mem_free((void *) stTimeStampInfo[mode].dwPhotoFrameBackupBuf);
        stTimeStampInfo[mode].dwPhotoFrameBackupBuf = 0;
    }

    if (stTimeStampInfo[mode].dwPhotoFrameWorkingBuf)
    {
        ext_mem_free((void *) stTimeStampInfo[mode].dwPhotoFrameWorkingBuf);
        stTimeStampInfo[mode].dwPhotoFrameWorkingBuf = 0;
    }

    stTimeStampInfo[mode].boolTimeStampEnable = FALSE;
    stTimeStampInfo[mode].wWidth = stTimeStampInfo[mode].stFontInfo[0].wWidth * TIME_STAMP_CHAR_PER_LINE;
    stTimeStampInfo[mode].wHeight = stTimeStampInfo[mode].stFontInfo[0].wHeight << 1;
    stTimeStampInfo[mode].dwOffset = stTimeStampInfo[mode].wWidth << 1;

    // Boundary check
    if ((stPhotoFrameInfo[mode].wWidth < stTimeStampInfo[mode].wWidth) ||
        (stPhotoFrameInfo[mode].wHeight < stTimeStampInfo[mode].wHeight))
    {
        MP_ALERT("\r\n--E-- %s: Photo frame size is too small !!!");
        MP_ALERT("--E-- Photo frame size is %d x %d", stPhotoFrameInfo[mode].wWidth, stPhotoFrameInfo[mode].wHeight);
        MP_ALERT("--E-- Time stamp size is %d x %d", stTimeStampInfo[mode].wWidth, stTimeStampInfo[mode].wHeight);

        return FAIL;;
    }

    if ((x + stTimeStampInfo[mode].wWidth) > stPhotoFrameInfo[mode].wWidth)
    {
        x = (stPhotoFrameInfo[mode].wWidth - stTimeStampInfo[mode].wWidth) & 0xFFFE;
    }

    if ((y + stTimeStampInfo[mode].wHeight) > stPhotoFrameInfo[mode].wHeight)
    {
        y = stPhotoFrameInfo[mode].wHeight - stTimeStampInfo[mode].wHeight;
    }

    stTimeStampInfo[mode].wHPos = x;
    stTimeStampInfo[mode].wVPos = y;
    size = stTimeStampInfo[mode].dwOffset * stTimeStampInfo[mode].wHeight;
    stTimeStampInfo[mode].dwPhotoFrameBackupBuf = (DWORD) ext_mem_malloc(size);
    stTimeStampInfo[mode].dwPhotoFrameWorkingBuf = (DWORD) ext_mem_malloc(size);

    if (!stTimeStampInfo[mode].dwPhotoFrameBackupBuf || !stTimeStampInfo[mode].dwPhotoFrameWorkingBuf)
    {
        MP_ALERT("--E-- %s: memory alloc fail !!!", __FUNCTION__);

        if (stTimeStampInfo[mode].dwPhotoFrameBackupBuf)
            ext_mem_free((void *) stTimeStampInfo[mode].dwPhotoFrameBackupBuf);

        if (stTimeStampInfo[mode].dwPhotoFrameWorkingBuf)
            ext_mem_free((void *) stTimeStampInfo[mode].dwPhotoFrameWorkingBuf);

        return FAIL;
    }

    stTimeStampInfo[mode].boolTimeStampEnable = TRUE;

    MP_DEBUG("Time Stamp Info -");
    MP_DEBUG("stTimeStampInfo.dwPhotoFrameBackupBuf = 0x%08X, %u", stTimeStampInfo[mode].dwPhotoFrameBackupBuf, size);
    MP_DEBUG("stTimeStampInfo.dwPhotoFrameWorkingBuf = 0x%08X, %u", stTimeStampInfo[mode].dwPhotoFrameWorkingBuf, size);
    MP_DEBUG("stTimeStampInfo.wHPos = %d", stTimeStampInfo[mode].wHPos);
    MP_DEBUG("stTimeStampInfo.wVPos = %d", stTimeStampInfo[mode].wVPos);
    MP_DEBUG("stTimeStampInfo.wWidth = %d", stTimeStampInfo[mode].wWidth);
    MP_DEBUG("stTimeStampInfo.wHeight = %d", stTimeStampInfo[mode].wHeight);
    MP_DEBUG("stTimeStampInfo.dwOffset = %d", stTimeStampInfo[mode].dwOffset);

    srcBufAddr = stPhotoFrameInfo[mode].dwBufferAddr + stTimeStampInfo[mode].wVPos * stPhotoFrameInfo[mode].dwLineOffset + (stTimeStampInfo[mode].wHPos << 1);

    mmcp_block((BYTE *) srcBufAddr, (BYTE *) stTimeStampInfo[mode].dwPhotoFrameBackupBuf,
               stTimeStampInfo[mode].wHeight, stTimeStampInfo[mode].wWidth << 1,
               stPhotoFrameInfo[mode].dwLineOffset,
               stTimeStampInfo[mode].dwOffset);
    memcpy((BYTE *) stTimeStampInfo[mode].dwPhotoFrameWorkingBuf, (BYTE *) stTimeStampInfo[mode].dwPhotoFrameBackupBuf, size);

    return PASS;
}



static void timeStampInfoChange(BOOL forPreview)
{
    MP_ALERT("%s: forPreview = %d", __func__, forPreview);
    DWORD destBufAddr;
    WORD x, y, number;
    ST_SYSTEM_TIME stCurrTime;
    BYTE mode;

    if (forPreview)
        mode = 0;
    else
        mode = 1;

    if (!stPhotoFrameInfo[mode].dwBufferAddr || \
        !stTimeStampInfo[mode].dwPhotoFrameWorkingBuf || \
        !stTimeStampInfo[mode].dwPhotoFrameBackupBuf || \
        (stTimeStampInfo[mode].boolTimeStampEnable == FALSE))
    {
        //MP_ALERT("--E-- %s: Null pointer !!!", __FUNCTION__);
        return;
    }

    destBufAddr = stPhotoFrameInfo[mode].dwBufferAddr + \
                  stTimeStampInfo[mode].wVPos * stPhotoFrameInfo[mode].dwLineOffset + \
                  (stTimeStampInfo[mode].wHPos << 1);
    mmcp_block((BYTE *) stTimeStampInfo[mode].dwPhotoFrameWorkingBuf, (BYTE *) destBufAddr,
               stTimeStampInfo[mode].wHeight, stTimeStampInfo[mode].wWidth << 1,
               stTimeStampInfo[mode].dwOffset, stPhotoFrameInfo[mode].dwLineOffset);

    SystemTimeGet(&stCurrTime);
    memcpy((BYTE *) stTimeStampInfo[mode].dwPhotoFrameWorkingBuf,
           (BYTE *) stTimeStampInfo[mode].dwPhotoFrameBackupBuf,
           stTimeStampInfo[mode].dwOffset * stTimeStampInfo[mode].wHeight);

    // Year
    x = 0;
    y = 0;
    number = stCurrTime.u16Year / 1000;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    
    number = (stCurrTime.u16Year / 100) % 10;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth; 

    number = (stCurrTime.u16Year / 10) % 10;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;

    number = stCurrTime.u16Year % 10;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    
    // /
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_D], x, y);
    x += stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_D].wWidth;

    // Month
    number = stCurrTime.u08Month / 10;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;

    number = stCurrTime.u08Month % 10;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    
    // /
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_D], x, y);
    x += stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_D].wWidth;

    // Date
    number = stCurrTime.u08Day / 10;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;

    number = stCurrTime.u08Day % 10;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);

    // P/A
    x = 0;
    y = stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_A].wHeight;

    if (stCurrTime.u08Hour > 11)
        number = E_TIME_STAMP_FONT_P;
    else
        number = E_TIME_STAMP_FONT_A;

    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;

    // M
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_M], x, y);
    x += stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_M].wWidth;

    // Hour
    number = stCurrTime.u08Hour / 10;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    
    number = stCurrTime.u08Hour % 10;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;    

    // :
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_T], x, y);
    x += stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_T].wWidth;

    // Minute
    number = stCurrTime.u08Minute / 10;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;

    number = stCurrTime.u08Minute % 10;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    
    // :
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_T], x, y);
    x += stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_T].wWidth;

    // Second
    number = stCurrTime.u08Second / 10;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;

    number = stCurrTime.u08Second % 10;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    
}



void TimeStampFontChangeForCapture(void)
{
    MP_DEBUG("%s", __func__);
    timeStampInfoChange(FALSE);
}



void TimeStampFontChange(void)
{
    MP_DEBUG("%s", __func__);
    static DWORD preSecondCount = 0;

    AddTimerProc(333, TimeStampFontChange); // why 333 ? 

    if (preSecondCount == RTC_ReadCount())
        return;

    preSecondCount = RTC_ReadCount();
    timeStampInfoChangeByAutoFolderNaming(TRUE); // preview = TRUE
    timeStampInfoChangeByAutoFolderNaming(FALSE); // preview = FALSE
}



void TimeStampFontRelease(BOOL forPreview)
{
    MP_DEBUG("%s", __func__);
    BYTE i, mode;

    IntDisable();

    if (forPreview)
        mode = 0;
    else
        mode = 1;

    stTimeStampInfo[mode].boolTimeStampEnable = FALSE;
    IntEnable();

    for (i = 0; i < E_TIME_STAMP_FONT_TOTAL; i++)
    {
        if (stTimeStampInfo[mode].stFontInfo[i].dwPointerAddr)
        {
            ext_mem_free((void *) stTimeStampInfo[mode].stFontInfo[i].dwPointerAddr);
        }
    }

    if (stTimeStampInfo[mode].dwPhotoFrameBackupBuf)
    {
        ext_mem_free((void *) stTimeStampInfo[mode].dwPhotoFrameBackupBuf);
    }

    if (stTimeStampInfo[mode].dwPhotoFrameWorkingBuf)
    {
        ext_mem_free((void *) stTimeStampInfo[mode].dwPhotoFrameWorkingBuf);
    }

    MpMemSet((BYTE *) &stTimeStampInfo[mode], 0, sizeof(ST_TIMESTAMP_INFO));

    if ((stTimeStampInfo[0].boolTimeStampEnable == 0) && (stTimeStampInfo[1].boolTimeStampEnable == 0))
        RemoveTimerProc(TimeStampFontChange);
}



void timeStampInfoChangeByAutoFolderNaming(BOOL forPreview)
{
    MP_DEBUG("%s: forPreview = %d", __func__, forPreview);
    DWORD destBufAddr;
    WORD x, y, number;
    ST_SYSTEM_TIME stCurrTime;
    BYTE mode;

    if (forPreview)
        mode = 0;
    else
        mode = 1;

    if (!stPhotoFrameInfo[mode].dwBufferAddr || \
        !stTimeStampInfo[mode].dwPhotoFrameWorkingBuf || \
        !stTimeStampInfo[mode].dwPhotoFrameBackupBuf || \
        (stTimeStampInfo[mode].boolTimeStampEnable == FALSE))
    {
        return;
    }

    destBufAddr = stPhotoFrameInfo[mode].dwBufferAddr + \
                  stTimeStampInfo[mode].wVPos * stPhotoFrameInfo[mode].dwLineOffset + \
                  (stTimeStampInfo[mode].wHPos << 1);
    mmcp_block((BYTE *) stTimeStampInfo[mode].dwPhotoFrameWorkingBuf, (BYTE *) destBufAddr,
               stTimeStampInfo[mode].wHeight, stTimeStampInfo[mode].wWidth << 1,
               stTimeStampInfo[mode].dwOffset, stPhotoFrameInfo[mode].dwLineOffset);

    SystemTimeGet(&stCurrTime);
    memcpy((BYTE *) stTimeStampInfo[mode].dwPhotoFrameWorkingBuf,
           (BYTE *) stTimeStampInfo[mode].dwPhotoFrameBackupBuf,
           stTimeStampInfo[mode].dwOffset * stTimeStampInfo[mode].wHeight);

    // Year
    x = 0;
    y = 0;
    //MP_DEBUG("--- stCurrTime.u16Year = %d ---", stCurrTime.u16Year);
    number = stCurrTime.u16Year / 1000;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    //MP_DEBUG("--- x =%d, Year-number:%d ---", x, number);
    TimeStamp[0] = number;    
    
    number = (stCurrTime.u16Year / 100) % 10;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    //MP_DEBUG("--- x =%d, Year-number:%d ---", x, number);
    TimeStamp[1] = number;
    
    number = ((stCurrTime.u16Year / 10) % 10);
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    //MP_DEBUG("--- x =%d, Year-number:%d ---", x, number); 
    TimeStamp[2] = number;

    number = (stCurrTime.u16Year % 10);
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    //MP_DEBUG("--- x =%d, Year-number:%d ---", x, number); 
    TimeStamp[3] = number;

    // /
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_D], x, y);
    x += stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_D].wWidth;

    // Month
    //MP_DEBUG("--- stCurrTime.u08Month = %d ---", stCurrTime.u08Month);
    number = stCurrTime.u08Month / 10;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    //MP_DEBUG("--- x =%d, Month-number:%d ---", x, number); 
    TimeStamp[4] = number;
    
    number = (stCurrTime.u08Month % 10);
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    //MP_DEBUG("--- x =%d, Month-number:%d ---", x, number); 
    TimeStamp[5] = number;
    
    // /
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_D], x, y);
    x += stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_D].wWidth;

    // Date
    //MP_DEBUG("--- stCurrTime.u08Day = %d ---", stCurrTime.u08Day);
    number = (stCurrTime.u08Day / 10);
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    //MP_DEBUG("--- x =%d, Date-number:%d ---", x, number); 
    TimeStamp[6] = number;

    number = (stCurrTime.u08Day % 10);
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    //MP_DEBUG("--- x =%d, Date-number:%d ---", x, number); 
    TimeStamp[7] = number;

    // P/A
    x = 0;
    y = stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_A].wHeight;
    //MP_DEBUG("--- stCurrTime.u08Hour = %d ---", stCurrTime.u08Hour);
    if (stCurrTime.u08Hour > 11)
        number = E_TIME_STAMP_FONT_P;
    else
        number = E_TIME_STAMP_FONT_A;

    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;

    // M  
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_M], x, y);
    x += stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_M].wWidth;

    // Hour
    number = stCurrTime.u08Hour / 10;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    //MP_DEBUG("--- x =%d, Hour-number:%d ---", x, number); 
    TimeStamp[8] = number;
    
    number = (stCurrTime.u08Hour % 10);
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    //MP_DEBUG("--- x =%d, Hour-number:%d ---", x, number);    
    TimeStamp[9] = number;

    // :
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_T], x, y);
    x += stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_T].wWidth;

    // Minute
    //MP_DEBUG("--- stCurrTime.u08Minute = %d ---", stCurrTime.u08Minute);
    number = (stCurrTime.u08Minute / 10);
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    //MP_DEBUG("--- x =%d, Minute-number:%d ---", x, number); 
    TimeStamp[10] = number;

    number = (stCurrTime.u08Minute % 10);
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    //MP_DEBUG("--- x =%d, Minute-number:%d ---", x, number);
    TimeStamp[11] = number;
    
    // :
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_T], x, y);
    x += stTimeStampInfo[mode].stFontInfo[E_TIME_STAMP_FONT_T].wWidth;

    // Second
    //MP_DEBUG("--- stCurrTime.u08Second = %d ---", stCurrTime.u08Second);
    number = stCurrTime.u08Second / 10;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    //MP_DEBUG("--- x =%d, Second-number:%d ---", x, number);
    TimeStamp[12] = number;

    number = stCurrTime.u08Second % 10;
    TimeStampFontCopy(&stTimeStampInfo[mode], &stTimeStampInfo[mode].stFontInfo[number], x, y);
    x += stTimeStampInfo[mode].stFontInfo[number].wWidth;
    //MP_DEBUG("--- x =%d, Second-number:%d ---", x, number);
    TimeStamp[13] = number;
#if 0
    WORD i;
    for(i=0;i<14;i++){
        MP_ALERT("### TimeStamp-number:%d ###", TimeStamp[i]);
    }
#endif    
}

#endif

#if 0

#define FWIDTH	232
#define FHEIGHT	24
#define KEYCOLOR 0x00008080

static DWORD WriteBuffer[FWIDTH*FHEIGHT/2];
static DWORD FBuffer1[FWIDTH*FHEIGHT/2];
static DWORD FBuffer2[FWIDTH*FHEIGHT/2];
static ST_IMGWIN WriteWin;
static ST_IMGWIN FWins[2];

int TimeStampPFrameSetup(BOOL forPreview)
{
	int i, mode;
    E_CAMCORDER_TIMING dx;
	if (forPreview)
		mode = 0;
	else
		mode = 1;

	for (i = 0; i < FWIDTH*FHEIGHT/2; i++) {
		WriteBuffer[i] = KEYCOLOR;
		FBuffer1[i] = KEYCOLOR;
		FBuffer2[i] = KEYCOLOR;
	}

	mpWinInit(&WriteWin, WriteBuffer, FHEIGHT, FWIDTH);
	mpWinInit(&FWins[0], FBuffer1, FHEIGHT, FWIDTH);
	mpWinInit(&FWins[1], FBuffer2, FHEIGHT, FWIDTH);

    dx = SetupCamcorderTimingGet();
    if (dx == CAMCORDER_RESOLUTION_720P)
    {
        WriteWin.wX = 1280-FWIDTH;
	    FWins[0].wX = 1280-FWIDTH;
	    FWins[1].wX = 1280-FWIDTH;

	    WriteWin.wY = 720-FHEIGHT;
	    FWins[0].wY = 720-FHEIGHT;
	    FWins[1].wY = 720-FHEIGHT;
    }
    else
    {
        WriteWin.wX = 640-FWIDTH;
	    FWins[0].wX = 640-FWIDTH;
	    FWins[1].wX = 640-FWIDTH;

	    WriteWin.wY = 480-FHEIGHT;
	    FWins[0].wY = 480-FHEIGHT;
	    FWins[1].wY = 480-FHEIGHT;
    }

	if (forPreview)
		SetSensorOverlayParam(&FWins[mode], KEYCOLOR >> 24, (KEYCOLOR >> 8) & 0xFF, KEYCOLOR & 0xFF);
	else
		SetSensorCaptureOverlayParam(&FWins[mode], KEYCOLOR >> 24, (KEYCOLOR >> 8) & 0xFF, KEYCOLOR & 0xFF);

	SetSensorOverlayEnable(ENABLE);
	return PASS;
}

static void timeStampInfoPrintChange(BOOL forPreview)
{
	int i, mode;
	if (forPreview)
		mode = 0;
	else
		mode = 1;
	for (i = 0; i < FWIDTH*FHEIGHT/2; i++)
		FWins[mode].pdwStart[i] = WriteWin.pdwStart[i];
}


static void WriteTimeStampString()
{
	int i;
	char strbuff[64];
	ST_SYSTEM_TIME stCurrTime;
	
	SystemTimeGet(&stCurrTime);

	for (i = 0; i < FWIDTH*FHEIGHT/2; i++)
		WriteWin.pdwStart[i] = KEYCOLOR;

	sprintf(strbuff, "%04d/%02d/%02d %02d:%02d:%02d", stCurrTime.u16Year, stCurrTime.u08Month, stCurrTime.u08Day,
		stCurrTime.u08Hour, stCurrTime.u08Minute, stCurrTime.u08Second);
    //Idu_SetFontColor(255, 255, 0);
	Idu_PrintString(&WriteWin, strbuff, 0, 0, 0, 0);
}

/*
void WriteTimeStampStringToPhoto(ST_IMGWIN * pWin)
{
	int i;
	char strbuff[64];
	ST_SYSTEM_TIME stCurrTime;
	
	SystemTimeGet(&stCurrTime);

	//for (i = 0; i < FWIDTH*FHEIGHT/2; i++)
	//	pWin->pdwStart[i] = KEYCOLOR;

	sprintf(strbuff, "%04d/%02d/%02d %02d:%02d:%02d", stCurrTime.u16Year, stCurrTime.u08Month, stCurrTime.u08Day,
		stCurrTime.u08Hour, stCurrTime.u08Minute, stCurrTime.u08Second);
    //Idu_SetFontColor(255, 255, 0);
	Idu_PrintString(pWin, strbuff, 1280-FWIDTH, 720-FHEIGHT, 0, 0);
}
*/

void TimeStampPrintStringChange_1(void);
void TimeStampPrintStringChange_2(void);
static void TimeStampPrintStringChange(void);

void TimeStampPrintStringChange_1(void)
{
    Ui_TimerProcAdd(333, TimeStampPrintStringChange_2);
    TimeStampPrintStringChange();
}

void TimeStampPrintStringChange_2(void)
{
    Ui_TimerProcAdd(333, TimeStampPrintStringChange_1);
    TimeStampPrintStringChange();
}

static void TimeStampPrintStringChange(void)
{
	static DWORD preSecondCount = 0;
	DWORD nowSecondCount;

	nowSecondCount = RTC_ReadCount();//WriteTimeStampString();
	timeStampInfoPrintChange(TRUE);
	timeStampInfoPrintChange(FALSE);
	if (preSecondCount == nowSecondCount) {
        if (nowSecondCount == 0)
        {
            WriteTimeStampString();
	        timeStampInfoPrintChange(TRUE);
	        timeStampInfoPrintChange(FALSE);
        }
		return;
	}

	preSecondCount = nowSecondCount;

	WriteTimeStampString();
	timeStampInfoPrintChange(TRUE);
	timeStampInfoPrintChange(FALSE);
	return;
}

void TimeStampPrintStringChangeStart(void)
{
    Ui_TimerProcRemove(TimeStampPrintStringChange_1);
    Ui_TimerProcRemove(TimeStampPrintStringChange_2);
    TimeStampPrintStringChange_1();
}

void TimeStampPFrameRelease(void)
{
	Ui_TimerProcRemove(TimeStampPrintStringChange_1);
    Ui_TimerProcRemove(TimeStampPrintStringChange_2);
	SetSensorOverlayEnable(DISABLE);
}


#endif


