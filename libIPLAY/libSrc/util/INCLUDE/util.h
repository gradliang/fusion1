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
* Filename      : util.h
* Programmers   :
* Created       :
* Descriptions  :
*******************************************************************************
*/
#ifndef __SYS_UTIL_H
#define __SYS_UTIL_H

#include "iplaysysconfig.h"


typedef struct {
    BYTE y;
    BYTE cb;
    BYTE cr;
    BYTE reserved;
} ST_COLOR_KEY;

typedef struct {
    BYTE y_IncValue;
    BYTE y_DecValue;
    BYTE cb_IncValue;
    BYTE cb_DecValue;
    BYTE cr_IncValue;
    BYTE cr_DecValue;
} ST_COLOR_KEY_NR;

typedef struct {
    WORD posX;
    WORD posY;
} ST_BOX_POS;

typedef struct {
    WORD sizeW;
    WORD sizeH;
} ST_BOX_SIZE;

typedef struct {
    WORD posX;
    WORD posY;
    WORD sizeW;
    WORD sizeH;
} ST_BOX_DIM;


typedef enum
{
    E_COMPARE_ERROR = -1,
    E_COMPARE_DIFFERENT = 0,
    E_COMPARE_EQUAL = 1
} ST_COMPARE_RESULT;


/* utility APIs for 8-bit and 16-bit character/string processing [begin] */
ST_COMPARE_RESULT  ArrayCompare08(BYTE * a, BYTE * b, DWORD count);
ST_COMPARE_RESULT  ArrayCompare16(WORD * a, WORD * b, DWORD count);
ST_COMPARE_RESULT  StringCompare08(BYTE * a, BYTE * b);
ST_COMPARE_RESULT  StringCompare16(WORD * a, WORD * b);
ST_COMPARE_RESULT  StringNCompare08(BYTE * a, BYTE * b, DWORD len);
ST_COMPARE_RESULT  StringNCompare16(WORD * a, WORD * b, DWORD len);
ST_COMPARE_RESULT  StringCompare08_CaseInsensitive(BYTE * a, BYTE * b);
ST_COMPARE_RESULT  StringNCompare08_CaseInsensitive(BYTE * a, BYTE * b, DWORD len);
ST_COMPARE_RESULT  StringCompare16_CaseInsensitive(WORD * a, WORD * b);
ST_COMPARE_RESULT  StringNCompare16_CaseInsensitive(WORD * a, WORD * b, DWORD len);
void   CharUpperCase08(BYTE * pChar);
void   CharUpperCase16(WORD * pChar);
void   UpperCase08(BYTE * string);
void   UpperCase16(WORD * string);
BYTE   *StringNCopy08(BYTE * target, BYTE * source, DWORD count);
BYTE   *StringCopy08(BYTE * target, BYTE * source);
WORD   *StringNCopy16(WORD * target, WORD * source, DWORD count);
WORD   *StringCopy16(WORD * target, WORD * source);
BYTE   *StringNCopy1608(BYTE * target, WORD * source, DWORD count);
BYTE   *StringCopy1608(BYTE * target, WORD * source);
WORD   *StringNCopy0816(WORD * target, BYTE * source, DWORD count);
WORD   *StringCopy0816(WORD * target, BYTE * source);
BYTE   *HexString(BYTE * string, DWORD hex, BYTE length);
BYTE   *DecString(BYTE * string, DWORD dec, BYTE length, BYTE decimal);
BYTE   *SkipLeadingZero(BYTE * string);
void   *TrimOffString08TailSpaces(BYTE * string);
void   *TrimOffString16TailSpaces(WORD * string);
BYTE   *DigitStringSeparate(BYTE * string);
DWORD  Bin2Bcd(DWORD value);
WORD   *String16rchr(WORD * string, WORD wchar);
WORD   StringLength08(BYTE * string);
WORD   StringLength16(WORD * string);
BYTE   *String08_strstr(BYTE * haystack, BYTE * needle);
WORD   *String16_strstr(WORD * haystack, WORD * needle);
BOOL   IsDigit08(BYTE ch);
BOOL   IsDigit16(WORD wCh);
BOOL   IsAlpha08(BYTE ch);
BOOL   IsAlpha16(WORD wCh);
/* utility APIs for 8-bit and 16-bit character/string processing [end] */


/* utility APIs for text encoding/conversion processing [begin] */
U16  *mpx_UtilAsc2Uni(U16 * target, U08 * source, U32 max_length);
U08  *mpx_UtilAscCatAsc(U08 * target, U08 * source);
void mpx_UtilNum2String(U08 * string, U32 num, U32 length);
U08  *mpx_UtilU16ToU08(U08 * target, U16 * source);
U08  *mpx_UtilUni2Asc(U08 * target, U16 * source);
U08  *mpx_UtilUnicodeToBig5U08(U08 * target, U16 * source);
U16  *mpx_UtilUniCatAsc(U16 * target, U08 * source);
U16  *mpx_UtilUniCatUni(U16 * target, U16 * source);
S32  mpx_UtilUniCompare(U16 * string1, U16 * string2);
U16  *mpx_UtilUniUpperCase(U16 * source);
U16  *mpx_UtilBig5ToUnicodeU16(U16 * target, U16 * source);
U16  *mpx_UtilUtf8ToUnicodeU16(U16 * target, U08 * source);
/* utility APIs for text encoding/conversion processing [end] */

DWORD UtilConfigParser(BYTE *src, DWORD srcSize, BYTE *name, BYTE *value);


///
///@ingroup UTIL_FB
///
///@brief   Find out the color key in the image buffer with assigned box and noise margin.
///
///@param   *pstOverlapWin      The struct pointer of overlap buffer
///@param   *pstBoxDim          The start position (Top-Left) and size of overlap buffer
///@param   *pstColorkey        The pure color key
///@param   *pstColorKeyNR      The noise margin of color key
///
///@retval  PASS
///@retval  FAIL
///
///@remark  Only for YUV422 format.
///
SDWORD Util_FbColorKeyFilterWithBox(ST_IMGWIN *pstOverlapWin, ST_BOX_DIM *pstBoxDim, ST_COLOR_KEY *pstColorkey, ST_COLOR_KEY_NR *pstColorKeyNR);

///
///@ingroup UTIL_FB
///
///@brief   Find out the color key in the image buffer with assigned noise margin.
///
///@param   *pstOverlapWin      The struct pointer of overlap buffer
///@param   *pstColorkey        The pure color key
///@param   *pstColorKeyNR      The noise margin of color key
///
///@retval  PASS
///@retval  FAIL
///
///@remark  Only for YUV422 format.
///@remark  The special case of Util_FbColorKeyFilterWithBox (full image buffer)
///
SDWORD Util_FbColorKeyFilter(ST_IMGWIN *pstOverlapWin, ST_COLOR_KEY *pstColorkey, ST_COLOR_KEY_NR *pstColorKeyNR);

///
///@ingroup UTIL_FB
///
///@brief   Image overlap with color key by SW
///
///@param   *pstBottomWin       The struct pointer of buttom buffer (Will be overlap)
///@param   *pstBottomWinBoxDim The position and size of buttom buffer will be cover to target buffer after overlap.
///@param   *pstTopWin          The struct pointer of top buffer (Will overlap to buttom buffer)
///@param   *pstTopWinBoxDim    The position and size of top buffer will overlap to buttom buffer.
///@param   *pstTrgWin          The struct pointer of target buffer (The result of overlap will be move to here)
///@param   *pstTrgWinBoxPos    The position of target buffer will be cover by buttom buffer. The size is using pstBottomWinBoxDim's size info.
///@param   *pstOverlapPos      The position of buttom buffer will be overlap, and the size is using pstTopWinBoxDim's size info.
///@param   *pstColorkey        The pure color key
///@param   *pstColorKeyNR      The noise margin of color key
///
///@retval  PASS
///@retval  FAIL
///
///@remark  Only for YUV422 format.
///
SDWORD Util_FbOverlapByColorkeySW(ST_IMGWIN *pstBottomWin, ST_BOX_DIM *pstBottomWinBoxDim,
                                  ST_IMGWIN *pstTopWin, ST_BOX_DIM *pstTopWinBoxDim,
                                  ST_IMGWIN *pstTrgWin, ST_BOX_POS *pstTrgWinBoxPos,
                                  ST_BOX_POS *pstOverlapPos,
                                  ST_COLOR_KEY *pstColorkey, ST_COLOR_KEY_NR *pstColorKeyNR);

///
///@ingroup UTIL_FB
///
///@brief   Image overlap with color key
///
///@param   *pstBottomWin       The struct pointer of buttom buffer (Will be overlap)
///@param   *pstTopWin          The struct pointer of top buffer (Will overlap to buttom buffer)
///@param   *pstTrgWin          The struct pointer of target buffer (The result of overlap will be move to here)
///@param   *pstOverlapPos      The position of buttom buffer will be overlap, and the size is using pstTopWinBoxDim's size info.
///@param   *pstColorkey        The pure color key
///@param   *pstColorKeyNR      The noise margin of color key
///
///@retval  PASS
///@retval  FAIL
///
///@remark  Only for YUV422 format.
///@remark  The special case of Util_FbOverlapByColorkeySW (full image buffer)
///@remark  Top buffer should be samll than buttom buffer.
///@remark  Buttom buffer should be samll than target buffer.
///
SDWORD Util_FbOverlapByColorkey(ST_IMGWIN *pstBottomWin,
                                ST_IMGWIN *pstTopWin,
                                ST_IMGWIN *pstTrgWin,
                                ST_BOX_POS *pstOverlapPos,
                                ST_COLOR_KEY *pstColorkey, ST_COLOR_KEY_NR *pstColorKeyNR);

#endif  //#define __SYS_UTIL_H

