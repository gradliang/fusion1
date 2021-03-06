/*------------------------------------------------------------------------------
--                                                                            --
--       This software is confidential and proprietary and may be used        --
--        only as expressly authorized by a licensing agreement from          --
--                                                                            --
--                            Hantro Products Oy.                             --
--                                                                            --
--                   (C) COPYRIGHT 2006 HANTRO PRODUCTS OY                    --
--                            ALL RIGHTS RESERVED                             --
--                                                                            --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
--                                                                            --
--------------------------------------------------------------------------------
--
--  Abstract : Converts data to tiled format.
--
--------------------------------------------------------------------------------
--
--  Version control information, please leave untouched.
--
--  $RCSfile: tb_tiled.c,v $
--  $Date: 2007/11/27 10:02:36 $
--  $Revision: 1.5 $
--
------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>

#include "tb_tiled.h"
#include "tb_md5.h"


void TbWriteTiledOutput(FILE *file, u8 *data, u32 mbWidth, u32 mbHeight, 
        u32 picNum, u32 md5SumOutput, u32 inputFormat)
{

/* Variables */

    u8 *pTiledYuv = NULL;
    u8 *planarData = NULL;
    u8 *pOutPixel = NULL;
    u8 *pRead = NULL;
    u32 lineStartOffset = 0;
    u32 row = 0, col = 0;
    u32 i, line;
    u32 width = mbWidth << 4;
    u32 height = mbHeight << 4;
    u32 cbOffset = 0, crOffset = 0;
    u32 lumaPels, chPels, mbLumWidth, mbLumHeight, mbChWidth, mbChHeight;
    
    u8 *pFrame = NULL;

    pTiledYuv = (u8 *) malloc((width*height*3)/2);

    if (file == NULL) {
        return;
    }
    
    if(pTiledYuv == NULL) {
        return;
    }

    if(md5SumOutput) {
        pFrame = pTiledYuv;
    }

    pOutPixel = pTiledYuv;
    
    /* convert semi-planar to planar */
    if (inputFormat) 
    {
        u8* planarDataBase;
        u32 tmp = width*height;
        
        planarData = (u8*) malloc((width*height*3)/2);
        planarDataBase = planarData;

        if(planarData == NULL) {
            free(pTiledYuv);
            return;
        }       
     
        memcpy(planarData, data, tmp);
        planarData += tmp;
                
        for(i = 0; i < tmp / 4; i++)
        {
            memcpy(planarData, data + tmp + i * 2, 1);
            ++planarData;
        }
                    
        for(i = 0; i < tmp / 4; i++)
        {
            memcpy(planarData, data + tmp + 1 + i * 2, 1);
            ++planarData;
        }
        
        planarData = planarDataBase;
        data = planarData;
    }

/* Code */    

    lumaPels = 256;
    chPels = 64;
    mbLumWidth = 16;
    mbLumHeight = 16;
    mbChWidth = 8;
    mbChHeight = 8;
    /* Luma */
    /* loop through MB rows */
    for(row=0; row<mbHeight; row++)
    {
        /* loop through mbs in a MB row */
        for(col=0; col<mbWidth; col++)
        {
            /* MB start location */
            pRead = data + (row * mbWidth * lumaPels) + (col * mbLumWidth);

            /* loop through all the lines of the MB */
            for(line=0; line <  mbLumHeight; line++)
            {
                /* offset for each line inside the 
                 * MB from the start of the MB */
                lineStartOffset = (mbLumWidth * mbWidth * line);

                /* Write pels in a line to the tiled temp picture */
                for(i=0;i < mbLumWidth; i++)
                {
                                    /*mboffs + line inside MB  + this pixel*/
                    *(pOutPixel++) = *(pRead + lineStartOffset + i);
                }
            }
        }
    }
    
    /* Chroma */
    /* Cb start offset */
    cbOffset = width * height;
    crOffset = cbOffset + (cbOffset>>2);

    /* loop through MB rows */
    for(row=0; row<mbHeight; row++)
    {
        /* loop through mbs in a MB row */
        for(col=0; col<mbWidth; col++)
        {
            /* MB start location */
            pRead =  data + (row * mbWidth * chPels) + (col * mbChWidth);

            /* loop through all the lines of the MB */
            for(line=0; line <  mbChHeight; line++)
            {
                /* offset for each line inside the 
                 * MB from the start of the MB */
                lineStartOffset = (mbChWidth * mbWidth * line);

                /* Write pels in a line to the tiled temp picture */
                for(i=0;i < mbChWidth; i++)
                {
                    /* Write Cb */
                                    /*mboffs + line inside MB  + this pixel*/
                    *(pOutPixel++) = *(pRead + cbOffset + lineStartOffset + i);
                    /* Write Cr */
                    *(pOutPixel++) = *(pRead + crOffset + lineStartOffset + i);
                }
            }
        }
    }

    pRead = pTiledYuv;

    if(md5SumOutput) 
    {
        TBWriteFrameMD5Sum(file, pFrame, (width*height*3)/2, picNum);
    }
    else
    {
        /* Write temp frame out */
        fwrite(pRead, sizeof(u8), (width*height*3)/2, file);
    }

    free(pTiledYuv);
    if (planarData != NULL)
    {
        free(planarData);
    }
}

void TbChangeEndianess(u8 *data, u32 dataSize)
{
    u32 x = 0;
    for(x = 0; x < dataSize; x += 4)
    {
        u32 tmp = 0;
        u32 *word = (u32 *) (data + x);

        tmp |= (*word & 0xFF) << 24;
        tmp |= (*word & 0xFF00) << 8;
        tmp |= (*word & 0xFF0000) >> 8;
        tmp |= (*word & 0xFF000000) >> 24;
        *word = tmp;
    }
}
