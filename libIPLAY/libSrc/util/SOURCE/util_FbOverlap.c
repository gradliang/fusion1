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
* Filename      : util_FbOverlap.c
* Programmers   : TY Miao
* Created       : TY Miao
* Descriptions  : FB Overlap function
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0


#include "global612.h"
#include "mpTrace.h"
#include "util.h"

#if (CHIP_VER_MSB != CHIP_VER_615)
//#define __USING_HW_IPU_OVERLAP
#define __USING_HW_MEMCPY
#endif

#if FB_OVERLAP
SDWORD Util_FbColorKeyFilterWithBox(ST_IMGWIN *pstOverlapWin, ST_BOX_DIM *pstBoxDim, ST_COLOR_KEY *pstColorkey, ST_COLOR_KEY_NR *pstColorKeyNR)
{
    int f, k;
    BYTE *pbTmpAddr;
    SWORD yMax, yMin, cbMax, cbMin, crMax, crMin;
    ST_BOX_DIM stTmpBoxDim;

    if (!pstOverlapWin || !pstBoxDim || !pstColorkey || !pstColorKeyNR)
    {
        mpDebugPrint("--E-- Util_FbColorKeyFilterWithBox - NULL pointer");

        return FAIL;
    }

    stTmpBoxDim.posX = pstBoxDim->posX & 0xFFFE;
    stTmpBoxDim.posY = pstBoxDim->posY;
    stTmpBoxDim.sizeW = pstBoxDim->sizeW & 0xFFFE;
    stTmpBoxDim.sizeH = pstBoxDim->sizeH;

    if ((stTmpBoxDim.posX + stTmpBoxDim.sizeW) > pstOverlapWin->wWidth)
    {
        mpDebugPrint("--E-- Util_FbColorKeyFilterWithBox: H - Out of range");

        return FAIL;
    }

    if ((stTmpBoxDim.posY + stTmpBoxDim.sizeH) > pstOverlapWin->wHeight)
    {
        mpDebugPrint("--E-- Util_FbColorKeyFilterWithBox: V - Out of range");

        return FAIL;
    }

    yMax = (SWORD) pstColorkey->y + (WORD) pstColorKeyNR->y_IncValue;
    yMin = (SWORD) pstColorkey->y - (WORD) pstColorKeyNR->y_DecValue;
    cbMax = (SWORD) pstColorkey->cb + (WORD) pstColorKeyNR->cb_IncValue;
    cbMin = (SWORD) pstColorkey->cb - (WORD) pstColorKeyNR->cb_DecValue;
    crMax = (SWORD) pstColorkey->cr + (WORD) pstColorKeyNR->cr_IncValue;
    crMin = (SWORD) pstColorkey->cr - (WORD) pstColorKeyNR->cr_DecValue;

    for (f = stTmpBoxDim.posY; f < (stTmpBoxDim.posY + stTmpBoxDim.sizeH); f++)
    {
        pbTmpAddr = (BYTE *) (((DWORD) pstOverlapWin->pdwStart) + pstOverlapWin->dwOffset * f + (stTmpBoxDim.posX << 1));

        for (k = 0; k < stTmpBoxDim.sizeH; k += 2)
        {

            if ( (((yMax > pbTmpAddr[0]) && (yMin < pbTmpAddr[0])) || ((yMax > pbTmpAddr[1]) && (yMin < pbTmpAddr[1]))) &&
                 ((cbMax > pbTmpAddr[2]) && (cbMin < pbTmpAddr[2])) &&
                 ((crMax > pbTmpAddr[3]) && (crMin < pbTmpAddr[3])) )
            {
                *(DWORD *) pbTmpAddr = (((DWORD) pstColorkey->y) << 24) |
                                     (((DWORD) pstColorkey->y) << 16) |
                                     (((DWORD) pstColorkey->cb) << 8) |
                                     (DWORD) pstColorkey->cr;
            }

            pbTmpAddr += 4;
        }
    }

    return PASS;
}



SDWORD Util_FbColorKeyFilter(ST_IMGWIN *pstOverlapWin, ST_COLOR_KEY *pstColorkey, ST_COLOR_KEY_NR *pstColorKeyNR)
{
    ST_BOX_DIM stTmpBoxDim = {0};

    if (!pstOverlapWin || !pstColorkey || !pstColorKeyNR)
    {
        mpDebugPrint("--E-- Util_FbColorKeyFilter - NULL pointer");

        return FAIL;
    }

    //stTmpBoxDim.posX = 0;
    //stTmpBoxDim.posY = 0;
    stTmpBoxDim.sizeW = pstOverlapWin->wWidth;
    stTmpBoxDim.sizeH = pstOverlapWin->wHeight;

    return Util_FbColorKeyFilterWithBox(pstOverlapWin, &stTmpBoxDim, pstColorkey, pstColorKeyNR);
}



SDWORD Util_FbOverlapByColorkeySW(ST_IMGWIN *pstBottomWin, ST_BOX_DIM *pstBottomWinBoxDim,
                                  ST_IMGWIN *pstTopWin, ST_BOX_DIM *pstTopWinBoxDim,
                                  ST_IMGWIN *pstTrgWin, ST_BOX_POS *pstTrgWinBoxPos,
                                  ST_BOX_POS *pstOverlapPos,
                                  ST_COLOR_KEY *pstColorkey, ST_COLOR_KEY_NR *pstColorKeyNR)
{
    SWORD yMax, yMin, cbMax, cbMin, crMax, crMin;
    ST_BOX_POS stTmpBottomWinBoxPos, stTmpTopWinBoxPos, stTmpTrgWinBoxPos, stTmpOverlapBoxPos;
    ST_BOX_SIZE stTmpBottomWinBoxSize, stTmpTopWinBoxSize;
    DWORD addr;

    MP_DEBUG("Util_FbOverlapByColorkeySW -");

    if (!pstBottomWin || !pstBottomWinBoxDim ||
        !pstTopWin || !pstTopWinBoxDim ||
        !pstTrgWin || !pstTrgWinBoxPos ||
        !pstColorkey || !pstColorKeyNR)
    {
        mpDebugPrint("--E-- Util_FbOverlapByColorkeySW - NULL pointer");

        return FAIL;
    }

    stTmpBottomWinBoxPos.posX = pstBottomWinBoxDim->posX & 0xFFFE;
    stTmpBottomWinBoxPos.posY = pstBottomWinBoxDim->posY;
    stTmpBottomWinBoxSize.sizeW = pstBottomWinBoxDim->sizeW & 0xFFFE;
    stTmpBottomWinBoxSize.sizeH = pstBottomWinBoxDim->sizeH;

    stTmpTopWinBoxPos.posX = pstTopWinBoxDim->posX & 0xFFFE;
    stTmpTopWinBoxPos.posY = pstTopWinBoxDim->posY;
    stTmpTopWinBoxSize.sizeW = pstTopWinBoxDim->sizeW & 0xFFFE;
    stTmpTopWinBoxSize.sizeH = pstTopWinBoxDim->sizeH;

    stTmpTrgWinBoxPos.posX = pstTrgWinBoxPos->posX & 0xFFFE;
    stTmpTrgWinBoxPos.posY = pstTrgWinBoxPos->posY;

    stTmpOverlapBoxPos.posX = pstOverlapPos->posX & 0xFFFE;
    stTmpOverlapBoxPos.posY = pstOverlapPos->posY;

    MP_DEBUG("stTmpBottomWinBoxPos is %d, %d", stTmpBottomWinBoxPos.posX, stTmpBottomWinBoxPos.posY);
    MP_DEBUG("stTmpBottomWinBoxSize = %d, %d", stTmpBottomWinBoxSize.sizeW, stTmpBottomWinBoxSize.sizeH);
    MP_DEBUG("stTmpTopWinBoxPos is %d, %d", stTmpTopWinBoxPos.posX, stTmpTopWinBoxPos.posY);
    MP_DEBUG("stTmpTopWinBoxSize = %d, %d", stTmpTopWinBoxSize.sizeW, stTmpTopWinBoxSize.sizeH);
    MP_DEBUG("stTmpTrgWinBoxPos is %d, %d", stTmpTrgWinBoxPos.posX, stTmpTrgWinBoxPos.posY);
    MP_DEBUG("stTmpOverlapBoxPos is %d, %d", stTmpOverlapBoxPos.posX, stTmpOverlapBoxPos.posY);

    // Check bottom win's buffer pointer
    addr = ((DWORD) pstBottomWin->pdwStart) & ~0xA0000000;

    if (addr == 0)
    {
        mpDebugPrint("Null pointer of Bottom Win Buf Addr");

        return FAIL;
    }

    // Check overlap win's buffer pointer
    addr = ((DWORD) pstTopWin->pdwStart) & ~0xA0000000;

    if (addr == 0)
    {
        mpDebugPrint("Null pointer of Overlap Win Buf Addr");

        return FAIL;
    }

    // Check target win's buffer pointer
    addr = ((DWORD) pstTrgWin->pdwStart) & ~0xA0000000;

    if (addr == 0)
    {
        mpDebugPrint("Null pointer of Trg Win Buf Addr");

        return FAIL;
    }

    // Check bottom win's dim
    if ( ((stTmpBottomWinBoxPos.posX + stTmpBottomWinBoxSize.sizeW) > pstBottomWin->wWidth) ||
         ((stTmpBottomWinBoxPos.posY + stTmpBottomWinBoxSize.sizeH) > pstBottomWin->wHeight) )
    {
        mpDebugPrint("--E-- Bottom win's box size - Out of range");

        MP_DEBUG("stTmpBottomBoxPos.posX = %d", stTmpBottomWinBoxPos.posX);
        MP_DEBUG("stTmpBottomBoxPos.posY = %d", stTmpBottomWinBoxPos.posY);
        MP_DEBUG("stTmpBottomWinBoxSize.sizeW = %d", stTmpBottomWinBoxSize.sizeW);
        MP_DEBUG("stTmpBottomWinBoxSize.sizeH = %d", stTmpBottomWinBoxSize.sizeH);
        MP_DEBUG("pstBottomWin->wWidth = %d", pstBottomWin->wWidth);
        MP_DEBUG("pstBottomWin->wHeight = %d", pstBottomWin->wHeight);

        return FAIL;
    }

    // Check top win's dim
    if ( ((stTmpTopWinBoxPos.posX + stTmpTopWinBoxSize.sizeW) > pstTopWin->wWidth) ||
         ((stTmpTopWinBoxPos.posY + stTmpTopWinBoxSize.sizeH) > pstTopWin->wHeight) )
    {
        mpDebugPrint("--E-- Top win's box size - Out of range");

        MP_DEBUG("stTmpTopBoxPos.posX = %d", stTmpTopWinBoxPos.posX);
        MP_DEBUG("stTmpTopBoxPos.posY = %d", stTmpTopWinBoxPos.posY);
        MP_DEBUG("stTmpTopWinBoxSize.sizeW = %d", stTmpTopWinBoxSize.sizeW);
        MP_DEBUG("stTmpTopWinBoxSize.sizeH = %d", stTmpTopWinBoxSize.sizeH);
        MP_DEBUG("pstTopWin->wWidth = %d", pstTopWin->wWidth);
        MP_DEBUG("pstTopWin->wHeight = %d", pstTopWin->wHeight);

        return FAIL;
    }

    // Check overlap region, Top win's box to Bottom win
    if ( ((stTmpOverlapBoxPos.posX + stTmpTopWinBoxSize.sizeW) > pstBottomWin->wWidth) ||
         ((stTmpOverlapBoxPos.posY + stTmpTopWinBoxSize.sizeH) > pstBottomWin->wHeight) )
    {
        mpDebugPrint("--E-- Overlap region - Out of bottom win's range");

        MP_DEBUG("stTmpBottomBoxPos.posX = %d", stTmpBottomWinBoxPos.posX);
        MP_DEBUG("stTmpBottomBoxPos.posY = %d", stTmpBottomWinBoxPos.posY);
        MP_DEBUG("stTmpBottomWinBoxSize.sizeW = %d", stTmpBottomWinBoxSize.sizeW);
        MP_DEBUG("stTmpBottomWinBoxSize.sizeH = %d", stTmpBottomWinBoxSize.sizeH);
        MP_DEBUG("pstBottomWin->wWidth = %d", pstBottomWin->wWidth);
        MP_DEBUG("pstBottomWin->wHeight = %d", pstBottomWin->wHeight);

        return FAIL;
    }

    // Check main win's dim with target win
    if ( (stTmpTrgWinBoxPos.posX + stTmpBottomWinBoxSize.sizeW > pstTrgWin->wWidth) ||
         (stTmpTrgWinBoxPos.posY + stTmpBottomWinBoxSize.sizeH > pstTrgWin->wHeight) )
    {
        mpDebugPrint("--E-- Copy region - Out of target win's range");

        MP_DEBUG("stTmpTrgWinBoxPos.posX = %d", stTmpTrgWinBoxPos.posX);
        MP_DEBUG("stTmpTrgWinBoxPos.posY = %d", stTmpTrgWinBoxPos.posY);
        MP_DEBUG("stTmpBottomWinBoxSize.sizeW = %d", stTmpBottomWinBoxSize.sizeW);
        MP_DEBUG("stTmpBottomWinBoxSize.sizeH = %d", stTmpBottomWinBoxSize.sizeH);
        MP_DEBUG("pstTrgWin->wWidth = %d", pstTrgWin->wWidth);
        MP_DEBUG("pstTrgWin->wHeight = %d", pstTrgWin->wHeight);

        return FAIL;
    }

    yMax = (SWORD) pstColorkey->y + (WORD) pstColorKeyNR->y_IncValue;
    yMin = (SWORD) pstColorkey->y - (WORD) pstColorKeyNR->y_DecValue;
    cbMax = (SWORD) pstColorkey->cb + (WORD) pstColorKeyNR->cb_IncValue;
    cbMin = (SWORD) pstColorkey->cb - (WORD) pstColorKeyNR->cb_DecValue;
    crMax = (SWORD) pstColorkey->cr + (WORD) pstColorKeyNR->cr_IncValue;
    crMin = (SWORD) pstColorkey->cr - (WORD) pstColorKeyNR->cr_DecValue;

    if ((DWORD) pstBottomWin->pdwStart != (DWORD) pstTrgWin->pdwStart)
    {
        BYTE *pbBottomWinAddr, *pbTrgWinAddr;

        MP_DEBUG("Main win is not Target win case, copy main win to target win");

#ifdef __USING_HW_MEMCPY
        pbBottomWinAddr = (BYTE *) (((DWORD) pstBottomWin->pdwStart) +
                                    (pstBottomWin->dwOffset * stTmpBottomWinBoxPos.posY) +
                                    (stTmpBottomWinBoxPos.posX << 1));

        pbTrgWinAddr = (BYTE *) (((DWORD) pstTrgWin->pdwStart) +
                                 (pstTrgWin->dwOffset * stTmpTrgWinBoxPos.posY) +
                                 (stTmpTrgWinBoxPos.posX << 1));

        mmcp_block(pbBottomWinAddr, pbTrgWinAddr,
                   stTmpBottomWinBoxSize.sizeW, stTmpBottomWinBoxSize.sizeH,
                   pstBottomWin->dwOffset, pstTrgWin->dwOffset);
#else
        DWORD bottomWinRow, trgWinRow;

        for (bottomWinRow = stTmpBottomWinBoxPos.posY, trgWinRow = stTmpTrgWinBoxPos.posY;
             bottomWinRow < (stTmpBottomWinBoxPos.posY + stTmpBottomWinBoxSize.sizeH);
             bottomWinRow++, trgWinRow++)
        {
            pbBottomWinAddr = (BYTE *) (((DWORD) pstBottomWin->pdwStart) +
                                        (pstBottomWin->dwOffset * bottomWinRow) +
                                        (stTmpBottomWinBoxPos.posX << 1));

            pbTrgWinAddr = (BYTE *) (((DWORD) pstTrgWin->pdwStart) +
                                     (pstTrgWin->dwOffset * trgWinRow) +
                                     (stTmpTrgWinBoxPos.posX << 1));

            memcpy(pbTrgWinAddr, pbBottomWinAddr, stTmpBottomWinBoxSize.sizeW << 1);
        }
#endif
    }
    else
    {
        MP_DEBUG("Main win is Target win case");
    }

    // Calculate relation position of Bottom win's box
    if (stTmpOverlapBoxPos.posX > stTmpBottomWinBoxPos.posX)
        stTmpOverlapBoxPos.posX -= stTmpBottomWinBoxPos.posX;
    else
    {   // New Overlap pos and new Overlap size
        stTmpTopWinBoxPos.posX += stTmpBottomWinBoxPos.posX - stTmpOverlapBoxPos.posX;
        stTmpTopWinBoxSize.sizeW -= stTmpBottomWinBoxPos.posX - stTmpOverlapBoxPos.posX;
        stTmpOverlapBoxPos.posX = 0;
    }

    if (stTmpOverlapBoxPos.posY > stTmpBottomWinBoxPos.posY)
        stTmpOverlapBoxPos.posY -= stTmpBottomWinBoxPos.posY;
    else
    {   // New Overlap pos and new Overlap size
        stTmpTopWinBoxPos.posY += stTmpBottomWinBoxPos.posY - stTmpOverlapBoxPos.posY;
        stTmpTopWinBoxSize.sizeH -= stTmpBottomWinBoxPos.posY - stTmpOverlapBoxPos.posY;
        stTmpOverlapBoxPos.posY = 0;
    }

    // Calculate relation position of Target win's pos
    stTmpOverlapBoxPos.posX += stTmpTrgWinBoxPos.posX;
    stTmpOverlapBoxPos.posY += stTmpTrgWinBoxPos.posY;

    MP_DEBUG("Last overlap pos is %d, %d", stTmpOverlapBoxPos.posX, stTmpOverlapBoxPos.posY);
    MP_DEBUG("Last overlap size is %d, %d", stTmpTopWinBoxSize.sizeW, stTmpTopWinBoxSize.sizeH);

    {
        WORD topWinRow, trgWinRow, colCount;
        BYTE *pbTopWinAddr, *pbTrgWinAddr;

        for (topWinRow = stTmpTopWinBoxPos.posY, trgWinRow = stTmpOverlapBoxPos.posY;
             topWinRow < (stTmpTopWinBoxPos.posY + stTmpTopWinBoxSize.sizeH);
             topWinRow++, trgWinRow++)
        {
            pbTopWinAddr = (BYTE *) (((DWORD) pstTopWin->pdwStart) +
                                        (pstTopWin->dwOffset * topWinRow) +
                                        (stTmpTopWinBoxPos.posX << 1));
            pbTrgWinAddr = (BYTE *) (((DWORD) pstTrgWin->pdwStart) +
                                     (pstTrgWin->dwOffset * trgWinRow) +
                                     (stTmpOverlapBoxPos.posX << 1));

            for (colCount = 0; colCount < stTmpTopWinBoxSize.sizeW; colCount += 2)
            {
                if ( (((yMax > pbTopWinAddr[0]) && (yMin < pbTopWinAddr[0])) || ((yMax > pbTopWinAddr[1]) && (yMin < pbTopWinAddr[1]))) &&
                     ((cbMax > pbTopWinAddr[2]) && (cbMin < pbTopWinAddr[2])) &&
                     ((crMax > pbTopWinAddr[3]) && (crMin < pbTopWinAddr[3])) )
                {   // Color Key Part
                    // Do not thing
                }
                else
                {   // Non Color Key Part
                    *(DWORD *) pbTrgWinAddr = *(DWORD *) pbTopWinAddr;
                }

                pbTopWinAddr += 4;
                pbTrgWinAddr += 4;
            }
        }
    }

    return PASS;
}



SDWORD Util_FbOverlapByColorkey(ST_IMGWIN *pstBottomWin,
                                ST_IMGWIN *pstTopWin,
                                ST_IMGWIN *pstTrgWin,
                                ST_BOX_POS *pstOverlapPos,
                                ST_COLOR_KEY *pstColorkey, ST_COLOR_KEY_NR *pstColorKeyNR)
{
    BOOL usingIpuOverlap = FALSE;
    ST_BOX_DIM stBottomWinBoxDim = {0}, stTopWinBoxDim = {0};
    ST_BOX_POS stTrgWinBoxPos = {0};

    if (!pstBottomWin || !pstTopWin || !pstTrgWin ||
        !pstOverlapPos ||
        !pstColorkey || !pstColorKeyNR)
    {
        mpDebugPrint("--E-- Util_FbOverlapByColorkey - NULL pointer");

        return FAIL;
    }

    stBottomWinBoxDim.sizeW = pstBottomWin->wWidth;
    stBottomWinBoxDim.sizeH = pstBottomWin->wHeight;
    stTopWinBoxDim.sizeW = pstTopWin->wWidth;
    stTopWinBoxDim.sizeH = pstTopWin->wHeight;

#ifdef __USING_HW_IPU_OVERLAP
    if ((pstBottomWin->wWidth == pstTrgWin->wWidth) &&
        (pstBottomWin->wHeight == pstTrgWin->wHeight) &&
        (pstBottomWin->dwOffset == pstTrgWin->dwOffset))
    {   // HW limitation
        usingIpuOverlap = TRUE;
    }

    if (usingIpuOverlap)
    {
        if (Util_FbColorKeyFilter(pstTopWin, pstColorkey, pstColorKeyNR) == PASS)
        {
            return Ipu_Overlay(pstBottomWin, pstTopWin, pstTrgWin,
                               pstOverlapPos->posX, pstOverlapPos->posY,
                               0, ENABLE, 2,
                               pstColorkey->y, pstColorkey->cb, pstColorkey->cr);
        }
        else
            return FAIL;
    }
    else
#endif
    {
        return Util_FbOverlapByColorkeySW(pstBottomWin, &stBottomWinBoxDim,
                                          pstTopWin, &stTopWinBoxDim,
                                          pstTrgWin, &stTrgWinBoxPos,
                                          pstOverlapPos,
                                          pstColorkey, pstColorKeyNR);
    }
}

#endif
