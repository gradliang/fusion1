#ifndef PHOTOFRAME_FUNC_H
#define PHOTOFRAME_FUNC_H

#include "utiltypedef.h"
#include "fs.h"
#include "display.h"

#define TIME_STAMP_CHAR_PER_LINE        10

typedef enum
{
    E_TIME_STAMP_FONT_0 = 0,        // '0'
    E_TIME_STAMP_FONT_1,            // '1'
    E_TIME_STAMP_FONT_2,            // '2'
    E_TIME_STAMP_FONT_3,            // '3'
    E_TIME_STAMP_FONT_4,            // '4'
    E_TIME_STAMP_FONT_5,            // '5'
    E_TIME_STAMP_FONT_6,            // '6'
    E_TIME_STAMP_FONT_7,            // '7'
    E_TIME_STAMP_FONT_8,            // '8'
    E_TIME_STAMP_FONT_9,            // '9'
    E_TIME_STAMP_FONT_A,            // 'A'
    E_TIME_STAMP_FONT_D,            // '/'`
    E_TIME_STAMP_FONT_M,            // 'M'
    E_TIME_STAMP_FONT_P,            // 'P'
    E_TIME_STAMP_FONT_T,            // ':'
    E_TIME_STAMP_FONT_TOTAL
} E_TIME_STAMP_FONT_INDEX;

typedef struct
{
    DWORD dwPointerAddr;
    DWORD dwOffset;
    WORD wWidth;
    WORD wHeight;
    DWORD colorKey;
} ST_TIMESTAMP_FONT_INFO;

typedef struct
{
    //// 0, 1, .., 9, A, P, M, :, /
    ST_TIMESTAMP_FONT_INFO stFontInfo[E_TIME_STAMP_FONT_TOTAL];
    BOOL boolTimeStampEnable;
    DWORD dwPhotoFrameBackupBuf;
    DWORD dwPhotoFrameWorkingBuf;
    WORD wHPos;         // releate to photo frame
    WORD wVPos;
    WORD wWidth;
    WORD wHeight;
    DWORD dwOffset;
} ST_TIMESTAMP_INFO;

typedef struct {
//    BOOL                    PhotoFrameApply;
    DWORD                   dwBufferAddr;
    DWORD                   dwLineOffset;
    DWORD                   dwColorKey;
    WORD                    wStartX;
    WORD                    wStartY;
    WORD                    wWidth;
    WORD                    wHeight;
} ST_PHOTO_FRAME_INFO;

void timeStampInfoChangeByAutoFolderNaming(BOOL forPreview);
void PhotoFrameInit(void);
SDWORD PhotoFrameSetup(const BYTE *frameBuf, WORD posX, WORD posY, BOOL forPreview);
void PhotoFrameRelease(BOOL forPreview);
void TimeStampFontInit(void);
SDWORD TimeStampFontSetup(const BYTE *fontBuf, DWORD bufSize, BOOL forPreview);
void TimeStampFontRelease(BOOL forPreview);
SDWORD TimeStampInfoSetup(WORD x, WORD y, BOOL forPreview);
//WORD TimeStamp[14];
#endif

