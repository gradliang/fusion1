///
///@defgroup    IMAGE    Image
///

///
///@ingroup     IMAGE
///

///
///@defgroup    GIF    GIF codec
///
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"

#if GIF
#include "jpeg.h"
#include "mpapi.h"

#define GIF_PALETTE_GET_565(x)      ((((*(BYTE*)x) >> 3) << 11) | (((*((BYTE*)x+1)) >> 2) << 5) | ((*((BYTE*)x+2)) >> 3))
#define GIF_PALETTE_GET_R(x)        (*((BYTE*)x))
#define GIF_PALETTE_GET_G(x)        (*(((BYTE*)x) + 1))
#define GIF_PALETTE_GET_B(x)        (*(((BYTE*)x) + 2))

typedef struct
{
	BYTE red;
	BYTE green;
	BYTE blue;
    BYTE bRev;
}ST_GLOBAL_BG_COLOR;

typedef struct
{
    ST_GIF_INFO stGifInfo;
    BYTE u08GifTimerId;
    BYTE bRev[3];
    
    WORD u16CurFrameIdx;
    WORD u16OutFrameWidth;
    WORD u16OutFrameHeight;
    DWORD u32ImageBuffer;
    DWORD u32ImageBufferSize;
    BYTE u08DataFormat;
    BYTE u08BackupCpuClk;
    BOOL needCloseFile;
    BOOL closeNotify;
//    ST_IP_IMAGE_SIZE stStartPos;
//    ST_IP_IMAGE_SIZE stWindowSize;
} ST_GIF_CONTROL;

ST_GIF_CONTROL* pstGifControl;
ST_GIF_CALLBACK gifCallback;
ST_GIF_SCREEN_DESC gifScreenDesc;
ST_GIF_IMAGE_DECODE_INFO gifCurrentFrameInfo;
ST_GIF_GRAPHIC_CONTROL gifControl;

BYTE *pGifGlobalColorTable = 0;
BYTE *pGifLocalColorTable = 0;
BYTE *pGifTempBuf = 0;

BYTE gifOutputFormat = GIF_OUTPUT_RGB888;
BYTE g_bAniGIF = FALSE;
WORD gifFrameNum = 0;

DWORD gGIFBufferStart = 0;
DWORD gGIFBufferEnd = 0;
DWORD gGIFBufferPtr = 0;

static WORD gifCurrentFrameInfo_prevIdx = 0;
static WORD gifCurrentFrameInfo_prevPos = 0;
static WORD pstGifControl_u16CurFrameIdx = 0;
static ST_GLOBAL_BG_COLOR gstglobalcolor = {0, 0, 0};

//local function
static SDWORD gifCheckScreenDesc(ST_GIF_SCREEN_DESC* outDesc);
static SDWORD gifCheckImageDesc(ST_GIF_IMAGE_DESC* outDesc);
static SDWORD gifProcessExtension();
static SDWORD gifBypassExtension();
static SDWORD gifFillBackgroundColor(DWORD u32BufferAddr);
static SDWORD gifFillTransparentColor(DWORD u32BufferAddr);
static SDWORD gifProcessImage(DWORD u32BufferAddr, DWORD u32BufferSize);
static SDWORD gifBypassImage();
static SDWORD gifSearchFrameStart(WORD inFrameIdx);
//==========================================================================================================//
//Decode memory
static WORD wImageHeight, wImageWidth;
static WORD wOriginalHeight, wOriginalWidth;

#define GIF_GLOBAL_COLOR_TABLE_SIZE 768
#define GIF_LOCAL_COLOR_TABLE_SIZE  768
#define GIF_LZW_CODE_SIZE           (576 * 1024)//589814	//0xc000 * 3* 4
#define GIF_TEMP_SIZE               (256 * 3)

BYTE *pbGifDecodeBuffer;

DWORD GIF_GLOBAL_COLOR_TABLE_ADDRESS;
DWORD GIF_LOCAL_COLOR_TABLE;
DWORD GIF_LZW_CODE_ADDRESS;
DWORD GIF_TEMP_ADDRESS;
DWORD GIF_TEMP_LZW_ADDRESS;

DWORD Gif_GetImageSize(IMAGEFILE *psImage);
int Gif_Decoder_Init(IMAGEFILE *psImage);
int Gif_Decoder_DecodeImage(IMAGEFILE *psImage);
int Gif_Decode_Bits(IMAGEFILE *psImage, BYTE *bpSource, DWORD dwSrcBufferSize);
//==========================================================================================================//

//call back function
void* allocProc(DWORD size)
{
    //return malloc(size);
    return ext_mem_malloc(size);
}

void freeProc(DWORD address)
{
    //free((void*)address);
    ext_mem_free((void*)address);
}

DWORD fillBufferProc(DWORD address, DWORD size)
{
    if((gGIFBufferPtr + size) > gGIFBufferEnd)
        size = gGIFBufferEnd - gGIFBufferPtr;

    memcpy((void*)address, (void*)gGIFBufferPtr, size);
    gGIFBufferPtr += size;

    return size;
}

DWORD seekSetProc(DWORD pos)
{
    if((pos + gGIFBufferStart) > gGIFBufferEnd)
        pos = gGIFBufferEnd - gGIFBufferStart;
    gGIFBufferPtr = gGIFBufferStart + pos;

    return pos;
}

DWORD getPosProc()
{
    return (gGIFBufferPtr - gGIFBufferStart);
}

//==================interlaced gif=================//
#define GROUP_1_START_ROW               0
#define GROUP_2_START_ROW               4
#define GROUP_3_START_ROW               2
#define GROUP_4_START_ROW               1

#define GROUP_1_INTERVAL                8
#define GROUP_2_INTERVAL                8
#define GROUP_3_INTERVAL                4
#define GROUP_4_INTERVAL                2

SDWORD lzwDecodeRGB888Deinterlace(DWORD u32BufferAddress);
SDWORD lzwDecodeRGB565Deinterlace(DWORD u32BufferAddress);

//==================ext-lzw decode=================//
ST_LZW_CODES* gstLZWCodes = 0;

SDWORD lzwDecodeRGB888(DWORD u32BufferAddress);
SDWORD lzwDecodeRGB888DownSample(DWORD u32BufferAddress);
SDWORD lzwDecodeRGB565(DWORD u32BufferAddress);
SDWORD lzwDecodeRGB565DownSample(DWORD u32BufferAddress);
SDWORD lzwReadCode(BYTE* buf, SDWORD* pBitPos, SDWORD bitSize);
void lzwWritePixel(DWORD u32BufferAddress, SDWORD pos, SDWORD code);

SDWORD lzwReadCode(BYTE* buf, SDWORD* pBitPos, SDWORD bitSize)
{
    SDWORD i, code = 0, pos = 1;
    BYTE ch;

    for(i = 0; i < bitSize; i++)
    {
        SDWORD bytePos = (*pBitPos >> 3) & 0xff;

        if(bytePos == 0)
        {
            SDWORD dataLen;
            gifCallback.refillBuffer((DWORD)&ch, 1);
            dataLen = ch;
            if(dataLen == 0)
            {
                return -1;
            }

            gifCallback.refillBuffer((DWORD)buf + 256 - dataLen, dataLen);
            bytePos = 256 - dataLen;
            *pBitPos = bytePos << 3;
        }

        if(buf[bytePos] & (1 << (*pBitPos & 7)))
            code += pos;
        pos += pos;
        (*pBitPos)++;
    }

    return code;
}



void lzwWritePixel(DWORD u32BufferAddress, SDWORD pos, SDWORD code)
{
    WORD hPos = pos % gifCurrentFrameInfo.u16Width;
    WORD vPos = pos / gifCurrentFrameInfo.u16Width;
    BYTE* ptr, *palette;

    if((gifControl.bTransparentColor) && (code == gifControl.u08TransparentColorIdx))
        return;

    {
        hPos += gifCurrentFrameInfo.u16HPos;
        vPos += gifCurrentFrameInfo.u16VPos;

        if((hPos % gifCurrentFrameInfo.u08DownSample) || (vPos % gifCurrentFrameInfo.u08DownSample))
            return;

        hPos /= gifCurrentFrameInfo.u08DownSample;
        vPos /= gifCurrentFrameInfo.u08DownSample;

        palette = (BYTE*)(gifCurrentFrameInfo.u32PaletteAddr + code * 3);

        if(gifOutputFormat == GIF_OUTPUT_RGB888)
        {
            ptr = (BYTE*)(u32BufferAddress + vPos * gifCurrentFrameInfo.u16OutWidth * 3 + hPos * 3);

            *ptr++ = GIF_PALETTE_GET_R(palette);
            *ptr++ = GIF_PALETTE_GET_G(palette);
            *ptr++ = GIF_PALETTE_GET_B(palette);
        }
        else if(gifOutputFormat == GIF_OUTPUT_RGB565)
        {
            ptr = (BYTE*)(u32BufferAddress + vPos * gifCurrentFrameInfo.u16OutWidth * 2 + hPos * 2);
            *(WORD*)ptr = GIF_PALETTE_GET_565(palette);
        }
    }
}



SDWORD lzwDecodeRGB888(DWORD u32BufferAddress)
{
    SDWORD origBitSize, bitSize, bitPos, clearMarker, endMarker;
    DWORD gifoffset, j;
    BYTE* buf = 0;
    ST_LZW_CODES* pstCodes = gstLZWCodes;
    SDWORD n, i, prev, code, c, outPos = 0;
    BYTE ch;
    SDWORD retCode = GIF_NO_ERROR;
    WORD hPos, vPos;
    BYTE *ptr, *palette, *lzw;

    buf = pGifTempBuf;
//  gifoffset = (wImageWidth - wJpegWidth) * 3;
    gifCallback.refillBuffer((DWORD)&ch, 1);

    origBitSize = ch;
    n = 2 + (1 << origBitSize);

    for(i = 0; i <n; i++)
    {
        pstCodes[i].c = i;
        pstCodes[i].len = 0;
    }

    clearMarker = n - 2;
    endMarker = n - 1;

    bitSize = origBitSize +1;
    bitPos = 0;

    prev = lzwReadCode(buf, &bitPos, bitSize);
    if(prev == -1)
        retCode = GIF_STREAM_ERROR;
    if(retCode == GIF_NO_ERROR)
    {
        do
        {
            code = lzwReadCode(buf, &bitPos, bitSize);
            if(code == -1)
            {
                retCode = GIF_STREAM_ERROR;
                break;
            }

            if(code == clearMarker)
            {
                bitSize = origBitSize;
                n = 1 << bitSize;
                n += 2;
                bitSize++;
                prev = code;
                continue;
            }

            if(code == endMarker)
                break;

            if(code < n)
                c = code;
            else
                c = prev;

            outPos += pstCodes[c].len;
            i = 0;
            do
            {
                //lzwWritePixel(u32BufferAddress, outPos-i, pstCodes[c].c);
                if(!((gifControl.bTransparentColor) && (pstCodes[c].c == gifControl.u08TransparentColorIdx)))
                {
                    hPos = (outPos - i) % gifCurrentFrameInfo.u16Width + gifCurrentFrameInfo.u16HPos;
                    vPos = (outPos - i) / gifCurrentFrameInfo.u16Width + gifCurrentFrameInfo.u16VPos;
                    ptr = (BYTE*)(u32BufferAddress + vPos * gifCurrentFrameInfo.u16OutWidth * 3 + hPos * 3);
//                    ptr = (BYTE*)(GIF_TEMP_LZW_ADDRESS + vPos * gifCurrentFrameInfo.u16OutWidth * 3 + hPos * 3);
                    palette = (BYTE*)(gifCurrentFrameInfo.u32PaletteAddr + pstCodes[c].c * 3);
                    *ptr++ = GIF_PALETTE_GET_R(palette);
                    *ptr++ = GIF_PALETTE_GET_G(palette);
                    *ptr++ = GIF_PALETTE_GET_B(palette);
                }

                if(pstCodes[c].len)
                    c = pstCodes[c].prefix;
                else
                    break;

                i++;
            }while(1);

            outPos++;

            if(code >= n)
            {
                //lzwWritePixel(u32BufferAddress, outPos, pstCodes[c].c);
                if(!((gifControl.bTransparentColor) && (pstCodes[c].c == gifControl.u08TransparentColorIdx)))
                {
                    hPos = outPos % gifCurrentFrameInfo.u16Width + gifCurrentFrameInfo.u16HPos;
                    vPos = outPos / gifCurrentFrameInfo.u16Width + gifCurrentFrameInfo.u16VPos;
                    palette = (BYTE*)(gifCurrentFrameInfo.u32PaletteAddr + pstCodes[c].c * 3);
                    ptr = (BYTE*)(u32BufferAddress + vPos * gifCurrentFrameInfo.u16OutWidth * 3 + hPos * 3);
//                    ptr = (BYTE*)(GIF_TEMP_LZW_ADDRESS + vPos * gifCurrentFrameInfo.u16OutWidth * 3 + hPos * 3);
                    *ptr++ = GIF_PALETTE_GET_R(palette);
                    *ptr++ = GIF_PALETTE_GET_G(palette);
                    *ptr++ = GIF_PALETTE_GET_B(palette);
                }
                outPos++;
            }

            if(prev != clearMarker)
            {
                pstCodes[n].prefix = prev;
                pstCodes[n].len = pstCodes[prev].len + 1;
                pstCodes[n].c = pstCodes[c].c;
                n++;
            }

            if(n == (1 << bitSize))
            {
                if(bitSize < 12)
                    bitSize++;
            }

            prev = code;
        }while(1);
    }
    
/*
//add offset data to decode data
	ptr = (BYTE *)GIF_TEMP_LZW_ADDRESS;
	lzw = u32BufferAddress;//(BYTE *) SystemGetMemAddr(JPEG_TARGET_MEM_ID);//(BYTE *)JPEG_TARGET_START_ADDRESS;
//set start X	
	lzw += ((wImageWidth - wOriginalWidth) >>1) * 3;	
	for(i = 0; i < wOriginalHeight; i++)
	{
		for(j = 0; j < (wOriginalWidth* 3); j++)
		{
			*lzw = *ptr;
			ptr++;
			lzw++;
		}
		for(j = 0; j < (wImageWidth - wOriginalWidth); j++)
		{
			*lzw = gstglobalcolor.red;		lzw++;
			*lzw = gstglobalcolor.green; 	lzw++;
			*lzw = gstglobalcolor.blue;		lzw++;
		}
	}
*/    
    return retCode;
}



SDWORD lzwDecodeRGB888Deinterlace(DWORD u32BufferAddress)
{
    SDWORD origBitSize, bitSize, bitPos, clearMarker, endMarker;
    BYTE* buf = 0;
    ST_LZW_CODES* pstCodes = gstLZWCodes;
    SDWORD n, i, j, prev, code, c, outPos = 0;
    BYTE ch;
    SDWORD retCode = GIF_NO_ERROR;
    WORD hPos, vPos;
    BYTE *ptr, *palette, *lzw;

    WORD groupStart1, groupStart2, groupStart3, groupStart4;
    WORD groupCount1 = 0, groupCount2 = 0, groupCount3 = 0, groupCount4 = 0;

    for(i = GROUP_1_START_ROW; i < gifCurrentFrameInfo.u16Height; i += GROUP_1_INTERVAL)
        groupCount1++;
    for(i = GROUP_2_START_ROW; i < gifCurrentFrameInfo.u16Height; i += GROUP_2_INTERVAL)
        groupCount2++;
    for(i = GROUP_3_START_ROW; i < gifCurrentFrameInfo.u16Height; i += GROUP_3_INTERVAL)
        groupCount3++;
    for(i = GROUP_4_START_ROW; i < gifCurrentFrameInfo.u16Height; i += GROUP_4_INTERVAL)
        groupCount4++;

    groupStart1 = 0;
    groupStart2 = groupStart1 + groupCount1;
    groupStart3 = groupStart2 + groupCount2;
    groupStart4 = groupStart3 + groupCount3;

    buf = pGifTempBuf;

    gifCallback.refillBuffer((DWORD)&ch, 1);
    origBitSize = ch;
    n = 2 + (1 << origBitSize);

    for(i = 0; i <n; i++)
    {
        pstCodes[i].c = i;
        pstCodes[i].len = 0;
    }

    clearMarker = n - 2;
    endMarker = n - 1;

    bitSize = origBitSize +1;
    bitPos = 0;

    prev = lzwReadCode(buf, &bitPos, bitSize);
    if(prev == -1)
        retCode = GIF_STREAM_ERROR;
    if(retCode == GIF_NO_ERROR)
    {
        do
        {
            code = lzwReadCode(buf, &bitPos, bitSize);
            if(code == -1)
            {
                retCode = GIF_STREAM_ERROR;
                break;
            }

            if(code == clearMarker)
            {
                bitSize = origBitSize;
                n = 1 << bitSize;
                n += 2;
                bitSize++;
                prev = code;
                continue;
            }

            if(code == endMarker)
                break;

            if(code < n)
                c = code;
            else
                c = prev;

            outPos += pstCodes[c].len;
            i = 0;
            do
            {
                if(!((gifControl.bTransparentColor) && (pstCodes[c].c == gifControl.u08TransparentColorIdx)))
                {
                    hPos = (outPos - i) % gifCurrentFrameInfo.u16Width;
                    vPos = (outPos - i) / gifCurrentFrameInfo.u16Width;

                    if(vPos >= groupStart4) //group 4
                    {
                        vPos = GROUP_4_START_ROW + ((vPos - groupStart4)<<1);
                    }
                    else if(vPos >= groupStart3) //group 3
                    {
                        vPos = GROUP_3_START_ROW + ((vPos - groupStart3)<<2);
                    }
                    else if(vPos >= groupStart2) // group 2
                    {
                        vPos = GROUP_2_START_ROW + ((vPos - groupStart2)<<3);
                    }
                    else
                    {
                        vPos = GROUP_1_START_ROW + (vPos<<3);
                    }

                    hPos += gifCurrentFrameInfo.u16HPos;
                    vPos += gifCurrentFrameInfo.u16VPos;

                    ptr = (BYTE*)(u32BufferAddress + vPos * gifCurrentFrameInfo.u16OutWidth * 3 + hPos * 3);
//                    ptr = (BYTE*)(GIF_TEMP_LZW_ADDRESS + vPos * gifCurrentFrameInfo.u16OutWidth * 3 + hPos * 3);
                    palette = (BYTE*)(gifCurrentFrameInfo.u32PaletteAddr + pstCodes[c].c * 3);
                    *ptr++ = GIF_PALETTE_GET_R(palette);
                    *ptr++ = GIF_PALETTE_GET_G(palette);
                    *ptr++ = GIF_PALETTE_GET_B(palette);
                }

                if(pstCodes[c].len)
                    c = pstCodes[c].prefix;
                else
                    break;

                i++;
            }while(1);

            outPos++;

            if(code >= n)
            {
                if(!((gifControl.bTransparentColor) && (pstCodes[c].c == gifControl.u08TransparentColorIdx)))
                {
                    hPos = outPos % gifCurrentFrameInfo.u16Width;
                    vPos = outPos / gifCurrentFrameInfo.u16Width;

                    if(vPos >= groupStart4) //group 4
                    {
                        vPos = GROUP_4_START_ROW + ((vPos - groupStart4)<<1);
                    }
                    else if(vPos >= groupStart3) //group 3
                    {
                        vPos = GROUP_3_START_ROW + ((vPos - groupStart3)<<2);
                    }
                    else if(vPos >= groupStart2) // group 2
                    {
                        vPos = GROUP_2_START_ROW + ((vPos - groupStart2)<<3);
                    }
                    else
                    {
                        vPos = GROUP_1_START_ROW + (vPos<<3);
                    }

                    hPos += gifCurrentFrameInfo.u16HPos;
                    vPos += gifCurrentFrameInfo.u16VPos;

                    palette = (BYTE*)(gifCurrentFrameInfo.u32PaletteAddr + pstCodes[c].c * 3);
                    ptr = (BYTE*)(u32BufferAddress + vPos * gifCurrentFrameInfo.u16OutWidth * 3 + hPos * 3);
//                    ptr = (BYTE*)(GIF_TEMP_LZW_ADDRESS + vPos * gifCurrentFrameInfo.u16OutWidth * 3 + hPos * 3);
                    *ptr++ = GIF_PALETTE_GET_R(palette);
                    *ptr++ = GIF_PALETTE_GET_G(palette);
                    *ptr++ = GIF_PALETTE_GET_B(palette);
                }
                outPos++;
            }

            if(prev != clearMarker)
            {
                pstCodes[n].prefix = prev;
                pstCodes[n].len = pstCodes[prev].len + 1;
                pstCodes[n].c = pstCodes[c].c;
                n++;
            }

            if(n == (1 << bitSize))
            {
                if(bitSize < 12)
                    bitSize++;
            }

            prev = code;
        }while(1);
    }

/*
//add offset data to decode data, add offset
	ptr = (BYTE *)GIF_TEMP_LZW_ADDRESS;
	lzw = u32BufferAddress;//(BYTE *) SystemGetMemAddr(JPEG_TARGET_MEM_ID);//(BYTE *)JPEG_TARGET_START_ADDRESS;
//set start X	
	lzw += ((wImageWidth - wOriginalWidth) >>1) * 3;
	for(i = 0; i < wOriginalHeight; i++)
	{
		for(j = 0; j < (wOriginalWidth* 3); j++)
		{
			*lzw = *ptr;
			ptr++;
			lzw++;
		}
		for(j = 0; j < (wImageWidth - wOriginalWidth); j++)
		{
			*lzw = gstglobalcolor.red;		lzw++;
			*lzw = gstglobalcolor.green; 	lzw++;
			*lzw = gstglobalcolor.blue;		lzw++;
		}
	}
*/    
    return retCode;
}


SDWORD lzwDecodeRGB888DownSample(DWORD u32BufferAddress)
{
    SDWORD origBitSize, bitSize, bitPos, clearMarker, endMarker;
    BYTE* buf = 0;
    ST_LZW_CODES* pstCodes = gstLZWCodes;
    SDWORD n, i, prev, code, c, outPos = 0;
    BYTE ch;
    SDWORD retCode = GIF_NO_ERROR;
    WORD hPos, vPos;
    BYTE *ptr, *palette;

    buf = pGifTempBuf;

    gifCallback.refillBuffer((DWORD)&ch, 1);
    origBitSize = ch;
    n = 2 + (1 << origBitSize);

    for(i = 0; i <n; i++)
    {
        pstCodes[i].c = i;
        pstCodes[i].len = 0;
    }

    clearMarker = n - 2;
    endMarker = n - 1;

    bitSize = origBitSize +1;
    bitPos = 0;

    prev = lzwReadCode(buf, &bitPos, bitSize);
    if(prev == -1)
        retCode = GIF_STREAM_ERROR;
    if(retCode == GIF_NO_ERROR)
    {
        do
        {
            code = lzwReadCode(buf, &bitPos, bitSize);
            if(code == -1)
            {
                retCode = GIF_STREAM_ERROR;
                break;
            }

            if(code == clearMarker)
            {
                bitSize = origBitSize;
                n = 1 << bitSize;
                n += 2;
                bitSize++;
                prev = code;
                continue;
            }

            if(code == endMarker)
                break;

            if(code < n)
                c = code;
            else
                c = prev;

            outPos += pstCodes[c].len;
            i = 0;
            do
            {
                if(!gifControl.bTransparentColor || (pstCodes[c].c != gifControl.u08TransparentColorIdx))
                {
                    hPos = (outPos - i) % gifCurrentFrameInfo.u16Width + gifCurrentFrameInfo.u16HPos;
                    vPos = (outPos - i) / gifCurrentFrameInfo.u16Width + gifCurrentFrameInfo.u16VPos;

                    if(!(hPos % gifCurrentFrameInfo.u08DownSample) && !(vPos % gifCurrentFrameInfo.u08DownSample))
                    {
                        hPos /= gifCurrentFrameInfo.u08DownSample;
                        vPos /= gifCurrentFrameInfo.u08DownSample;

                        palette = (BYTE*)(gifCurrentFrameInfo.u32PaletteAddr + pstCodes[c].c * 3);
                        ptr = (BYTE*)(u32BufferAddress + vPos * gifCurrentFrameInfo.u16OutWidth * 3 + hPos * 3);
                        *ptr++ = GIF_PALETTE_GET_R(palette);
                        *ptr++ = GIF_PALETTE_GET_G(palette);
                        *ptr++ = GIF_PALETTE_GET_B(palette);
                    }
                }

                if(pstCodes[c].len)
                    c = pstCodes[c].prefix;
                else
                    break;

                i++;
            }while(1);

            outPos++;

            if(code >= n)
            {
                if(!gifControl.bTransparentColor || (pstCodes[c].c != gifControl.u08TransparentColorIdx))
                {
                    hPos = outPos % gifCurrentFrameInfo.u16Width + gifCurrentFrameInfo.u16HPos;
                    vPos = outPos / gifCurrentFrameInfo.u16Width + gifCurrentFrameInfo.u16VPos;

                    if(!(hPos % gifCurrentFrameInfo.u08DownSample) && !(vPos % gifCurrentFrameInfo.u08DownSample))
                    {
                        hPos /= gifCurrentFrameInfo.u08DownSample;
                        vPos /= gifCurrentFrameInfo.u08DownSample;

                        palette = (BYTE*)(gifCurrentFrameInfo.u32PaletteAddr + pstCodes[c].c * 3);
                        ptr = (BYTE*)(u32BufferAddress + vPos * gifCurrentFrameInfo.u16OutWidth * 3 + hPos * 3);
                        *ptr++ = GIF_PALETTE_GET_R(palette);
                        *ptr++ = GIF_PALETTE_GET_G(palette);
                        *ptr++ = GIF_PALETTE_GET_B(palette);
                    }
                }
                outPos++;
            }

            if(prev != clearMarker)
            {
                pstCodes[n].prefix = prev;
                pstCodes[n].len = pstCodes[prev].len + 1;
                pstCodes[n].c = pstCodes[c].c;
                n++;
            }

            if(n == (1 << bitSize))
            {
                if(bitSize < 12)
                    bitSize++;
            }

            prev = code;
        }while(1);
    }

    return retCode;
}



SDWORD lzwDecodeRGB565(DWORD u32BufferAddress)
{
    SDWORD origBitSize, bitSize, bitPos, clearMarker, endMarker;
    BYTE* buf = 0;
    ST_LZW_CODES* pstCodes = gstLZWCodes;
    SDWORD n, i, prev, code, c, outPos = 0;
    BYTE ch;
    SDWORD retCode = GIF_NO_ERROR;
    WORD hPos, vPos;
    BYTE *ptr, *palette;

    buf = pGifTempBuf;

    gifCallback.refillBuffer((DWORD)&ch, 1);
    origBitSize = ch;
    n = 2 + (1 << origBitSize);

    for(i = 0; i <n; i++)
    {
        pstCodes[i].c = i;
        pstCodes[i].len = 0;
    }

    clearMarker = n - 2;
    endMarker = n - 1;

    bitSize = origBitSize +1;
    bitPos = 0;

    prev = lzwReadCode(buf, &bitPos, bitSize);
    if(prev == -1)
        retCode = GIF_STREAM_ERROR;

    if(retCode == GIF_NO_ERROR)
    {
        do
        {
            //code = lzwReadCode(buf, &bitPos, bitSize);
            SDWORD j, pos = 1;

            code = 0;
            for(j = 0; j < bitSize; j++)
            {
                SDWORD bytePos = (bitPos >> 3) & 0xff;
                if(!bytePos)
                {
                    SDWORD dataLen;
                    gifCallback.refillBuffer((DWORD)&ch, 1);
                    dataLen = ch;
                    if(dataLen == 0)
                    {
                        return GIF_NO_ERROR;
                    }

                    gifCallback.refillBuffer((DWORD)buf + 256 - dataLen, dataLen);
                    bytePos = 256 - dataLen;
                    bitPos = bytePos << 3;
                }

                if(buf[bytePos] & (1 << (bitPos&7)))
                    code += pos;
                pos += pos;
                bitPos++;
            }

            if(code == -1)
            {
                retCode = GIF_STREAM_ERROR;
                break;
            }

            if(code == clearMarker)
            {
                bitSize = origBitSize;
                n = 1 << bitSize;
                n += 2;
                bitSize++;
                prev = code;
                continue;
            }

            if(code == endMarker)
                break;

            if(code < n)
                c = code;
            else
                c = prev;

            outPos += pstCodes[c].len;
            i = 0;
            do
            {
                if(!((gifControl.bTransparentColor) && (pstCodes[c].c == gifControl.u08TransparentColorIdx)))
                {
                    hPos = (outPos - i) % gifCurrentFrameInfo.u16Width + gifCurrentFrameInfo.u16HPos;
                    vPos = (outPos - i) / gifCurrentFrameInfo.u16Width + gifCurrentFrameInfo.u16VPos;
                    ptr = (BYTE*)(u32BufferAddress + vPos * gifCurrentFrameInfo.u16OutWidth * 2 + hPos * 2);
                    palette = (BYTE*)(gifCurrentFrameInfo.u32PaletteAddr + pstCodes[c].c * 3);
                    *(WORD*)ptr = GIF_PALETTE_GET_565(palette);
                }

                ptr -= 2;

                if(pstCodes[c].len)
                    c = pstCodes[c].prefix;
                else
                    break;

                i++;
            }while(1);

            outPos++;

            if(code >= n)
            {
                if(!((gifControl.bTransparentColor) && (pstCodes[c].c == gifControl.u08TransparentColorIdx)))
                {
                    hPos = outPos % gifCurrentFrameInfo.u16Width + gifCurrentFrameInfo.u16HPos;
                    vPos = outPos / gifCurrentFrameInfo.u16Width + gifCurrentFrameInfo.u16VPos;
                    palette = (BYTE*)(gifCurrentFrameInfo.u32PaletteAddr + pstCodes[c].c * 3);
                    ptr = (BYTE*)(u32BufferAddress + vPos * gifCurrentFrameInfo.u16OutWidth * 2 + hPos * 2);
                    *(WORD*)ptr = GIF_PALETTE_GET_565(palette);
                }

                outPos++;
            }

            if(prev != clearMarker)
            {
                pstCodes[n].prefix = prev;
                pstCodes[n].len = pstCodes[prev].len + 1;
                pstCodes[n].c = pstCodes[c].c;
                n++;
            }

            if(n == (1 << bitSize))
            {
                if(bitSize < 12)
                    bitSize++;
            }

            prev = code;
        }while(1);
    }

    return retCode;
}



SDWORD lzwDecodeRGB565Deinterlace(DWORD u32BufferAddress)
{
    SDWORD origBitSize, bitSize, bitPos, clearMarker, endMarker;
    BYTE* buf = 0;
    ST_LZW_CODES* pstCodes = gstLZWCodes;
    SDWORD n, i, prev, code, c, outPos = 0;
    BYTE ch;
    SDWORD retCode = GIF_NO_ERROR;
    WORD hPos, vPos;
    BYTE *ptr, *palette;
    WORD groupStart1, groupStart2, groupStart3, groupStart4;
    WORD groupCount1 = 0, groupCount2 = 0, groupCount3 = 0, groupCount4 = 0;

    for(i = GROUP_1_START_ROW; i < gifCurrentFrameInfo.u16Height; i += GROUP_1_INTERVAL)
        groupCount1++;
    for(i = GROUP_2_START_ROW; i < gifCurrentFrameInfo.u16Height; i += GROUP_2_INTERVAL)
        groupCount2++;
    for(i = GROUP_3_START_ROW; i < gifCurrentFrameInfo.u16Height; i += GROUP_3_INTERVAL)
        groupCount3++;
    for(i = GROUP_4_START_ROW; i < gifCurrentFrameInfo.u16Height; i += GROUP_4_INTERVAL)
        groupCount4++;

    groupStart1 = 0;
    groupStart2 = groupStart1 + groupCount1;
    groupStart3 = groupStart2 + groupCount2;
    groupStart4 = groupStart3 + groupCount3;

    buf = pGifTempBuf;

    gifCallback.refillBuffer((DWORD)&ch, 1);
    origBitSize = ch;
    n = 2 + (1 << origBitSize);

    for(i = 0; i <n; i++)
    {
        pstCodes[i].c = i;
        pstCodes[i].len = 0;
    }

    clearMarker = n - 2;
    endMarker = n - 1;

    bitSize = origBitSize +1;
    bitPos = 0;

    prev = lzwReadCode(buf, &bitPos, bitSize);
    if(prev == -1)
        retCode = GIF_STREAM_ERROR;

    if(retCode == GIF_NO_ERROR)
    {
        do
        {
            //code = lzwReadCode(buf, &bitPos, bitSize);
            SDWORD j, pos = 1;

            code = 0;
            for(j = 0; j < bitSize; j++)
            {
                SDWORD bytePos = (bitPos >> 3) & 0xff;
                if(!bytePos)
                {
                    SDWORD dataLen;
                    gifCallback.refillBuffer((DWORD)&ch, 1);
                    dataLen = ch;
                    if(dataLen == 0)
                    {
                        return GIF_NO_ERROR;
                    }

                    gifCallback.refillBuffer((DWORD)buf + 256 - dataLen, dataLen);
                    bytePos = 256 - dataLen;
                    bitPos = bytePos << 3;
                }

                if(buf[bytePos] & (1 << (bitPos&7)))
                    code += pos;
                pos += pos;
                bitPos++;
            }

            if(code == -1)
            {
                retCode = GIF_STREAM_ERROR;
                break;
            }

            if(code == clearMarker)
            {
                bitSize = origBitSize;
                n = 1 << bitSize;
                n += 2;
                bitSize++;
                prev = code;
                continue;
            }

            if(code == endMarker)
                break;

            if(code < n)
                c = code;
            else
                c = prev;

            outPos += pstCodes[c].len;
            i = 0;
            do
            {
                if(!((gifControl.bTransparentColor) && (pstCodes[c].c == gifControl.u08TransparentColorIdx)))
                {
                    hPos = (outPos - i) % gifCurrentFrameInfo.u16Width;
                    vPos = (outPos - i) / gifCurrentFrameInfo.u16Width;

                    if(vPos >= groupStart4) //group 4
                    {
                        vPos = GROUP_4_START_ROW + ((vPos - groupStart4)<<1);
                    }
                    else if(vPos >= groupStart3) //group 3
                    {
                        vPos = GROUP_3_START_ROW + ((vPos - groupStart3)<<2);
                    }
                    else if(vPos >= groupStart2) // group 2
                    {
                        vPos = GROUP_2_START_ROW + ((vPos - groupStart2)<<3);
                    }
                    else
                    {
                        vPos = GROUP_1_START_ROW + (vPos<<3);
                    }

                    hPos += gifCurrentFrameInfo.u16HPos;
                    vPos += gifCurrentFrameInfo.u16VPos;

                    ptr = (BYTE*)(u32BufferAddress + vPos * gifCurrentFrameInfo.u16OutWidth * 2 + hPos * 2);
                    palette = (BYTE*)(gifCurrentFrameInfo.u32PaletteAddr + pstCodes[c].c * 3);
                    *(WORD*)ptr = GIF_PALETTE_GET_565(palette);
                }

                ptr -= 2;

                if(pstCodes[c].len)
                    c = pstCodes[c].prefix;
                else
                    break;

                i++;
            }while(1);

            outPos++;

            if(code >= n)
            {
                if(!((gifControl.bTransparentColor) && (pstCodes[c].c == gifControl.u08TransparentColorIdx)))
                {
                    hPos = outPos % gifCurrentFrameInfo.u16Width;
                    vPos = outPos / gifCurrentFrameInfo.u16Width;

                    if(vPos >= groupStart4) //group 4
                    {
                        vPos = GROUP_4_START_ROW + ((vPos - groupStart4)<<1);
                    }
                    else if(vPos >= groupStart3) //group 3
                    {
                        vPos = GROUP_3_START_ROW + ((vPos - groupStart3)<<2);
                    }
                    else if(vPos >= groupStart2) // group 2
                    {
                        vPos = GROUP_2_START_ROW + ((vPos - groupStart2)<<3);
                    }
                    else
                    {
                        vPos = GROUP_1_START_ROW + (vPos<<3);
                    }

                    hPos += gifCurrentFrameInfo.u16HPos;
                    vPos += gifCurrentFrameInfo.u16VPos;

                    palette = (BYTE*)(gifCurrentFrameInfo.u32PaletteAddr + pstCodes[c].c * 3);
                    ptr = (BYTE*)(u32BufferAddress + vPos * gifCurrentFrameInfo.u16OutWidth * 2 + hPos * 2);
                    *(WORD*)ptr = GIF_PALETTE_GET_565(palette);
                }

                outPos++;
            }

            if(prev != clearMarker)
            {
                pstCodes[n].prefix = prev;
                pstCodes[n].len = pstCodes[prev].len + 1;
                pstCodes[n].c = pstCodes[c].c;
                n++;
            }

            if(n == (1 << bitSize))
            {
                if(bitSize < 12)
                    bitSize++;
            }

            prev = code;
        }while(1);
    }

    return retCode;
}



SDWORD lzwDecodeRGB565DownSample(DWORD u32BufferAddress)
{
    SDWORD origBitSize, bitSize, bitPos, clearMarker, endMarker;
    BYTE* buf = 0;
    ST_LZW_CODES* pstCodes = gstLZWCodes;
    SDWORD n, i, prev, code, c, outPos = 0;
    BYTE ch;
    SDWORD retCode = GIF_NO_ERROR;
    WORD hPos, vPos;
    BYTE *ptr, *palette;

    buf = pGifTempBuf;

    gifCallback.refillBuffer((DWORD)&ch, 1);
    origBitSize = ch;
    n = 2 + (1 << origBitSize);

    for(i = 0; i <n; i++)
    {
        pstCodes[i].c = i;
        pstCodes[i].len = 0;
    }

    clearMarker = n - 2;
    endMarker = n - 1;

    bitSize = origBitSize +1;
    bitPos = 0;

    prev = lzwReadCode(buf, &bitPos, bitSize);
    if(prev == -1)
        retCode = GIF_STREAM_ERROR;
    if(retCode == GIF_NO_ERROR)
    {
        do
        {
            code = lzwReadCode(buf, &bitPos, bitSize);
            if(code == -1)
            {
                retCode = GIF_STREAM_ERROR;
                break;
            }

            if(code == clearMarker)
            {
                bitSize = origBitSize;
                n = 1 << bitSize;
                n += 2;
                bitSize++;
                prev = code;
                continue;
            }

            if(code == endMarker)
                break;

            if(code < n)
                c = code;
            else
                c = prev;

            outPos += pstCodes[c].len;
            i = 0;
            do
            {
                if(!gifControl.bTransparentColor || (pstCodes[c].c != gifControl.u08TransparentColorIdx))
                {
                    hPos = (outPos - i) % gifCurrentFrameInfo.u16Width + gifCurrentFrameInfo.u16HPos;
                    vPos = (outPos - i) / gifCurrentFrameInfo.u16Width + gifCurrentFrameInfo.u16VPos;

                    if(!(hPos % gifCurrentFrameInfo.u08DownSample) && !(vPos % gifCurrentFrameInfo.u08DownSample))
                    {
                        hPos /= gifCurrentFrameInfo.u08DownSample;
                        vPos /= gifCurrentFrameInfo.u08DownSample;

                        palette = (BYTE*)(gifCurrentFrameInfo.u32PaletteAddr + pstCodes[c].c * 3);
                        ptr = (BYTE*)(u32BufferAddress + vPos * gifCurrentFrameInfo.u16OutWidth * 2 + hPos * 2);
                        *(WORD*)ptr = GIF_PALETTE_GET_565(palette);
                    }
                }

                if(pstCodes[c].len)
                    c = pstCodes[c].prefix;
                else
                    break;

                i++;
            }while(1);

            outPos++;

            if(code >= n)
            {
                if(!gifControl.bTransparentColor || (pstCodes[c].c != gifControl.u08TransparentColorIdx))
                {
                    hPos = outPos % gifCurrentFrameInfo.u16Width + gifCurrentFrameInfo.u16HPos;
                    vPos = outPos / gifCurrentFrameInfo.u16Width + gifCurrentFrameInfo.u16VPos;

                    if(!(hPos % gifCurrentFrameInfo.u08DownSample) && !(vPos % gifCurrentFrameInfo.u08DownSample))
                    {
                        hPos /= gifCurrentFrameInfo.u08DownSample;
                        vPos /= gifCurrentFrameInfo.u08DownSample;

                        palette = (BYTE*)(gifCurrentFrameInfo.u32PaletteAddr + pstCodes[c].c * 3);
                        ptr = (BYTE*)(u32BufferAddress + vPos * gifCurrentFrameInfo.u16OutWidth * 2 + hPos * 2);
                        *(WORD*)ptr = GIF_PALETTE_GET_565(palette);
                    }
                }
                outPos++;
            }

            if(prev != clearMarker)
            {
                pstCodes[n].prefix = prev;
                pstCodes[n].len = pstCodes[prev].len + 1;
                pstCodes[n].c = pstCodes[c].c;
                n++;
            }

            if(n == (1 << bitSize))
            {
                if(bitSize < 12)
                    bitSize++;
            }

            prev = code;
        }while(1);
    }

    return retCode;
}
//=================================================//


void ResetGIFValue()
{
	gifCurrentFrameInfo_prevIdx = 0;
	gifCurrentFrameInfo_prevPos = 0;
	pstGifControl_u16CurFrameIdx = 0;
}



SDWORD GifModuleInit(GIF_MALLOC_CALLBACK allocCallback,
                    GIF_MFREE_CALLBACK freeCallback,
                    GIF_FILL_BUFFER_CALLBACK fillBufCallback,
                    GIF_SEEK_SET_CALLBACK seekSetCallback,
                    GIF_GET_POS_CALLBACK getPosCallback)
{
    if(!allocCallback || !freeCallback || !fillBufCallback || !seekSetCallback || !getPosCallback)
        return GIF_ERROR_PARAMETER;

    gifCallback.mAllocate = allocCallback;
    gifCallback.mFree = freeCallback;
    gifCallback.refillBuffer = fillBufCallback;
    gifCallback.seekSet = seekSetCallback;
    gifCallback.getPos = getPosCallback;

    gifCurrentFrameInfo.prevIdx = 0;
    gifCurrentFrameInfo.prevPos = 0;    

    GIF_GLOBAL_COLOR_TABLE_ADDRESS = (DWORD)pbGifDecodeBuffer;
    GIF_LOCAL_COLOR_TABLE = GIF_GLOBAL_COLOR_TABLE_ADDRESS + GIF_GLOBAL_COLOR_TABLE_SIZE;
    GIF_LZW_CODE_ADDRESS = GIF_LOCAL_COLOR_TABLE + GIF_LOCAL_COLOR_TABLE_SIZE;
    GIF_TEMP_ADDRESS = GIF_LZW_CODE_ADDRESS + GIF_LZW_CODE_SIZE;
    GIF_TEMP_LZW_ADDRESS = GIF_TEMP_ADDRESS + GIF_TEMP_SIZE;

    MP_DEBUG("\r\nMEMORY ALLOCATE\r\n");
    MP_DEBUG("GIF_GLOBAL_COLOR_TABLE_ADDRESS=%x", GIF_GLOBAL_COLOR_TABLE_ADDRESS);
    MP_DEBUG("GIF_LOCAL_COLOR_TABLE=%x", GIF_LOCAL_COLOR_TABLE);
    MP_DEBUG("GIF_LZW_CODE_ADDRESS=%x", GIF_LZW_CODE_ADDRESS);
    MP_DEBUG("GIF_TEMP_ADDRESS=%x", GIF_TEMP_ADDRESS);
    MP_DEBUG("GIF_TEMP_LZW_ADDRESS=%x", GIF_TEMP_LZW_ADDRESS);

    pGifGlobalColorTable = (BYTE*)GIF_GLOBAL_COLOR_TABLE_ADDRESS;
    pGifLocalColorTable = (BYTE*)GIF_LOCAL_COLOR_TABLE;
    gstLZWCodes = (ST_LZW_CODES*)GIF_LZW_CODE_ADDRESS;
    pGifTempBuf = (BYTE*)GIF_TEMP_ADDRESS;
    
    return GIF_NO_ERROR;
}

SDWORD GifModuleRelease()
{
    gifCallback.mFree((DWORD)pGifGlobalColorTable);
    gifCallback.mFree((DWORD)pGifLocalColorTable);
    gifCallback.mFree((DWORD)gstLZWCodes);
    gifCallback.mFree((DWORD)pGifTempBuf);


    pGifGlobalColorTable = 0;
    pGifLocalColorTable = 0;
    pGifTempBuf = 0;
    gstLZWCodes = 0;

    gifFrameNum = 0;

    gifCallback.mAllocate = 0;
    gifCallback.mFree = 0;
    gifCallback.refillBuffer = 0;
    gifCallback.seekSet = 0;
    gifCallback.getPos = 0;

    gifCurrentFrameInfo.prevIdx = 0;
    gifCurrentFrameInfo.prevPos = 0;

    return GIF_NO_ERROR;
}

SDWORD gifCheckScreenDesc(ST_GIF_SCREEN_DESC* outDesc)
{
    BYTE tempString[4];
    BYTE *pGifStreamData = 0;
    BYTE *pGifPtr;
    WORD i;

    MP_DEBUG("gifCheckScreenDesc");
    pGifStreamData = pGifTempBuf;

    gifCallback.seekSet(0);//from start
    gifCallback.refillBuffer((DWORD)pGifStreamData, GIF_HEADER_SIZE);

    pGifPtr = pGifStreamData;

    tempString[0] = *pGifPtr++;
    tempString[1] = *pGifPtr++;
    tempString[2] = *pGifPtr++;
    tempString[3] = 0;

    if((tempString[0] != 'G') && (tempString[1] != 'I') && (tempString[2] != 'F'))
    {
        return GIF_ILLEGAL_FILE;
    }

    tempString[0] = *pGifPtr++;
    tempString[1] = *pGifPtr++;
    tempString[2] = *pGifPtr++;
    tempString[3] = 0;

    if((tempString[0] == '8') && (tempString[1] == '7') && (tempString[2] == 'a'))
        outDesc->u08Version = GIF_VERSION_87A;
    else if((tempString[0] == '8') && (tempString[1] == '9') && (tempString[2] == 'a'))
        outDesc->u08Version = GIF_VERSION_89A;
    else
    {
        return GIF_UNSURPPORTED;
    }

    outDesc->u16ScreenWidth = wOriginalWidth = *pGifPtr | ((*(pGifPtr+1)) << 8);
    wImageWidth = ((outDesc->u16ScreenWidth + 15)& 0xfff0);	//ImageWidth must multiple of 16
//    wImageWidth = outDesc->u16ScreenWidth;	//ImageWidth must multiple of 16

    pGifPtr += 2;
    outDesc->u16ScreenHeight = wImageHeight = wOriginalHeight = *pGifPtr | ((*(pGifPtr+1)) << 8);
    pGifPtr += 2;

    tempString[0] = *pGifPtr++;
    if(tempString[0] & GIF_MASK_GLOBAL_COLOR_TABLE)
    {
        outDesc->bGlobalColorTable = TRUE;
        outDesc->u16GlobalColorTableSize = tempString[0] & GIF_MASK_GLOBAL_COLOR_TABLE_SIZE;
        outDesc->u16GlobalColorTableSize = 1 << (outDesc->u16GlobalColorTableSize + 1);
    }
    else
    {
        outDesc->bGlobalColorTable = FALSE;
        outDesc->u16GlobalColorTableSize = 0;
    }

    outDesc->u08ColorResolution = (tempString[0] & GIF_MASK_COLOR_RESOLUTION) >> 4;
    if(tempString[0] & GIF_MASK_SORT_FLAG)
        outDesc->bSort = TRUE;
    else
        outDesc->bSort = FALSE;

    outDesc->u08BgColorIdx = *pGifPtr++;
    outDesc->u08PixelAR = *pGifPtr++;

    if(outDesc->bGlobalColorTable)
    {
        gifCallback.refillBuffer((DWORD)pGifGlobalColorTable, outDesc->u16GlobalColorTableSize * 3);
    }
    else
    {
        for(i = 0; i < GIF_COLOR_TABLE_SIZE; i++)
            pGifGlobalColorTable[i] = 0;
    }

    return GIF_NO_ERROR;
}



SDWORD gifCheckImageDesc(ST_GIF_IMAGE_DESC* outDesc)
{
    BYTE *pGifPtr, *pGifStreamData;
    SDWORD returnCode = GIF_NO_ERROR;

    MP_DEBUG("gifCheckImageDesc");
    if(!outDesc)
        return GIF_ERROR_PARAMETER;

    pGifStreamData = pGifTempBuf;

    gifCallback.refillBuffer((DWORD)pGifStreamData, GIF_IMAGE_DESC_SIZE);

    pGifPtr = pGifStreamData;

    outDesc->u16ImageLeftPos = *pGifPtr | ((*(pGifPtr+1)) << 8);
    pGifPtr += 2;
    outDesc->u16ImageTopPos = *pGifPtr | ((*(pGifPtr+1)) << 8);
    pGifPtr += 2;
    outDesc->u16ImageWidth = *pGifPtr | ((*(pGifPtr+1)) << 8);
    pGifPtr += 2;
    outDesc->u16ImageHeight = *pGifPtr | ((*(pGifPtr+1)) << 8);
    pGifPtr += 2;
    outDesc->bLocalColorTable = ((*pGifPtr) & GIF_MASK_LOCAL_COLOR_TABLE) ? TRUE : FALSE;
    outDesc->bInterlace = ((*pGifPtr) & GIF_MASK_INTERLACE) ? TRUE : FALSE;
    outDesc->bSort = ((*pGifPtr) & GIF_MASK_LOCAL_SORT) ? TRUE : FALSE;

    if(outDesc->bLocalColorTable)
    {
        outDesc->u16LocalColorTableSize = (*pGifPtr) & GIF_MASK_LOCAL_COLOR_TABLE_SIZE;
        outDesc->u16LocalColorTableSize = 1 << (outDesc->u16LocalColorTableSize + 1);
    }
    else
        outDesc->u16LocalColorTableSize = 0;

    return returnCode;
}



SDWORD gifFillBackgroundColor(DWORD u32BufferAddr)
{
    DWORD i, j;
    BYTE *pDest;
    WORD r, g, b;

    MP_DEBUG("gifFillBackgroundColor");
    
    BYTE* palette = (BYTE*)(gifCurrentFrameInfo.u32PaletteAddr + gifScreenDesc.u08BgColorIdx * 3);
//    DWORD totalPixel = gifCurrentFrameInfo.u16OutWidth * gifCurrentFrameInfo.u16OutHeight;
    DWORD totalPixel = wImageWidth * gifCurrentFrameInfo.u16OutHeight;

    if(gifOutputFormat == GIF_OUTPUT_RGB888)
    {

        r = GIF_PALETTE_GET_R(palette);
        g = GIF_PALETTE_GET_G(palette);
        b = GIF_PALETTE_GET_B(palette);
	if((r == 0) && (g == 0) && (b == 0))
	{
		r = g = b = 255;
	}

	gstglobalcolor.red = r;
	gstglobalcolor.green = g;
	gstglobalcolor.blue = b;
	
        pDest = (BYTE*)u32BufferAddr;
        for(i = 0; i < totalPixel; i++)
        {
            *pDest++ = r;
            *pDest++ = g;
            *pDest++ = b;
        }
    }
    else if(gifOutputFormat == GIF_OUTPUT_RGB565)
    {
        WORD *pwDest = (WORD*)u32BufferAddr;
        WORD data = GIF_PALETTE_GET_565(palette);

        for(i = 0; i < totalPixel; i++)
            *pwDest++ = data;
    }
    else
        return GIF_ERROR_PARAMETER;

    return GIF_NO_ERROR;
}



SDWORD gifFillTransparentColor(DWORD u32BufferAddr)
{
    DWORD i, j;
    BYTE *pDest;
    WORD r, g, b;

    BYTE* palette = (BYTE*)(gifCurrentFrameInfo.u32PaletteAddr + gifScreenDesc.u08BgColorIdx * 3);
    DWORD totalPixel = gifCurrentFrameInfo.u16OutWidth * gifCurrentFrameInfo.u16OutHeight;

    MP_DEBUG("gifFillTransparentColor");
    if(gifOutputFormat == GIF_OUTPUT_RGB888)
    {

        r = GIF_PALETTE_GET_R(palette);
        g = GIF_PALETTE_GET_G(palette);
        b = GIF_PALETTE_GET_B(palette);
	if((r == 0) && (g == 0) && (b == 0))
	{
		r = g = b = 255;
	}

        pDest = (BYTE*)u32BufferAddr;
        for(i = 0; i < totalPixel; i++)
        {
            *pDest++ = r;
            *pDest++ = g;
            *pDest++ = b;
        }
    }
    else if(gifOutputFormat == GIF_OUTPUT_RGB565)
    {
        WORD *pwDest = (WORD*)u32BufferAddr;
        WORD data = GIF_PALETTE_GET_565(palette);

        for(i = 0; i < totalPixel; i++)
            *pwDest++ = data;
    }
    else
        return GIF_ERROR_PARAMETER;

    return GIF_NO_ERROR;
}


SDWORD gifProcessImage(DWORD u32BufferAddr, DWORD u32BufferSize)
{
    ST_GIF_IMAGE_DESC imageDesc;

    MP_DEBUG("gifProcessImage");
    gifCheckImageDesc(&imageDesc);

    gifCurrentFrameInfo.u16HPos = imageDesc.u16ImageLeftPos;
    gifCurrentFrameInfo.u16VPos = imageDesc.u16ImageTopPos;
    gifCurrentFrameInfo.u16Width = imageDesc.u16ImageWidth;
    gifCurrentFrameInfo.u16Height = imageDesc.u16ImageHeight;

    if(imageDesc.bLocalColorTable)
    {
        gifCallback.refillBuffer((DWORD)pGifLocalColorTable, 3 * imageDesc.u16LocalColorTableSize);
        gifCurrentFrameInfo.u32PaletteAddr = (DWORD)pGifLocalColorTable;
    }
    else
    {
        gifCurrentFrameInfo.u32PaletteAddr = (DWORD)pGifGlobalColorTable;
    }

    if(gifControl.u08DisposalMethod == GIF_RESTORE_TO_BG)
    {
    	gifFillBackgroundColor(u32BufferAddr);//JPEG_TARGET_START_ADDRESS);
    	//gifFillTransparentColor(GIF_TEMP_LZW_ADDRESS);
    	gifFillTransparentColor(u32BufferAddr);
    }

    MP_DEBUG("bInterlace=%d, u08DownSample=%d", imageDesc.bInterlace, gifCurrentFrameInfo.u08DownSample);

    if(imageDesc.bInterlace)
    {
        if(gifCurrentFrameInfo.u08DownSample == 1)
        {
            if(gifOutputFormat == GIF_OUTPUT_RGB888)
                lzwDecodeRGB888Deinterlace(u32BufferAddr);
            else
                lzwDecodeRGB565Deinterlace(u32BufferAddr);
        }
        else
        {
            return GIF_UNSURPPORTED;
        }
    }
    else
    {
	    if(gifCurrentFrameInfo.u08DownSample == 1)
	    {
	        if(gifOutputFormat == GIF_OUTPUT_RGB888)
	            lzwDecodeRGB888(u32BufferAddr);
	        else
	            lzwDecodeRGB565(u32BufferAddr);
	    }
	    else
	    {
	        if(gifOutputFormat == GIF_OUTPUT_RGB888)
	            lzwDecodeRGB888DownSample(u32BufferAddr);
	        else
	            lzwDecodeRGB565DownSample(u32BufferAddr);
	    }
	}
	return GIF_NO_ERROR;
}



SDWORD gifBypassImage()
{
    ST_GIF_IMAGE_DESC imageDesc;
    SDWORD returnCode;
    BYTE ch;
    BYTE* tempBuffer = pGifTempBuf;

    MP_DEBUG("gifBypassImage");
    returnCode = gifCheckImageDesc(&imageDesc);
    if(returnCode != 0)
        return returnCode;

    if(imageDesc.bLocalColorTable)
        gifCallback.refillBuffer((DWORD)tempBuffer, 3 * imageDesc.u16LocalColorTableSize);

    gifCallback.refillBuffer((DWORD)&ch, 1); //code size
    gifCallback.refillBuffer((DWORD)&ch, 1); //block size
    while(ch)
    {
        gifCallback.refillBuffer((DWORD)tempBuffer, ch);
        gifCallback.refillBuffer((DWORD)&ch, 1);
    }
	return GIF_NO_ERROR;
}



SDWORD gifBypassExtension()
{
    BYTE ch, temp;
    BYTE tempBuffer[256];

    MP_DEBUG("gifBypassExtension");
    gifCallback.refillBuffer((DWORD)&ch, 1); //extension id
    gifCallback.refillBuffer((DWORD)&ch, 1); //block size
    while(ch)
    {
        gifCallback.refillBuffer((DWORD)tempBuffer, ch); //block content
        gifCallback.refillBuffer((DWORD)&ch, 1);
    }

    return GIF_NO_ERROR;
}



SDWORD gifProcessExtension()
{
    BYTE blockSize;
    BYTE ch;
    BYTE* tempBuf = 0;
    SDWORD retCode = GIF_NO_ERROR;

    tempBuf = pGifTempBuf;

    gifCallback.refillBuffer((DWORD)&ch, 1);

    gifCallback.refillBuffer((DWORD)&blockSize, 1);
    gifCallback.refillBuffer((DWORD)tempBuf, blockSize);

    switch(ch)
    {
        case 0xf9:
            {
                gifControl.bTransparentColor = tempBuf[0] & GIF_MASK_CONTROL_TRANSPARENT;
                gifControl.bUserInput = tempBuf[0] & GIF_MASK_CONTROL_USER_INPUT;
                gifControl.u08DisposalMethod = (tempBuf[0] & GIF_MASK_CONTROL_DISPOSAL) >> 2;
                gifControl.u16DelayTime = (tempBuf[2] << 8) | tempBuf[1];
                if(gifControl.bTransparentColor)
                {
                    gifControl.u08TransparentColorIdx = tempBuf[3];
                }
                else
                {
                    gifControl.u08TransparentColorIdx = 0;
                }
            }
        break;

        case 0xfe:
            tempBuf[blockSize] = 0;
        break;

        case 0x01:
        break;

        case 0xff:
        break;

        default:
        break;
    }

    gifCallback.refillBuffer((DWORD)&ch, 1);
    while(ch)
    {
        gifCallback.refillBuffer((DWORD)tempBuf, ch);
        gifCallback.refillBuffer((DWORD)&ch, 1);
    }

    return retCode;
}



SDWORD gifSearchFrameStart(WORD inFrameIdx)
{
    BYTE ch;
    SDWORD retCode = GIF_NO_ERROR;
    BOOL found = FALSE;
    WORD frameCount = 0;

    MP_DEBUG("gifSearchFrameStart, inFrameIdx=%d", inFrameIdx);
    
    if(!gifCallback.mAllocate || !gifCallback.mFree || !gifCallback.refillBuffer || !gifCallback.seekSet)
        return GIF_MODULE_NOT_INITIALIZE;
    if(inFrameIdx >= gifFrameNum)
        return GIF_ERROR_PARAMETER;

    if(gifScreenDesc.bGlobalColorTable)
        gifCallback.seekSet(GIF_HEADER_SIZE + gifScreenDesc.u16GlobalColorTableSize * 3);
    else
        gifCallback.seekSet(GIF_HEADER_SIZE);
    if(gifCurrentFrameInfo_prevPos)
    {
        if(inFrameIdx == (gifCurrentFrameInfo.prevIdx + 1))
        {
	        gifCallback.seekSet(gifCurrentFrameInfo_prevPos);
            frameCount = inFrameIdx;
        }
    }

    if(retCode == GIF_NO_ERROR)
    {
        do
        {
            gifCallback.refillBuffer((DWORD)&ch, 1);

            switch(ch)
            {
                case 0x21:
                    //gifBypassExtension();
                    retCode = gifProcessExtension();
                    if(retCode != 0)
    			        return retCode;
                break;

                case 0x2c:
                {
                    if(inFrameIdx == frameCount)
                    {
                        found = TRUE;
                    }
                    else
                    {
                        retCode = gifBypassImage();
                        frameCount++;
                        if(retCode != 0)			
			                return retCode;
                    }
                }
                break;

                case 0x3b:
                    if(!found)
                    {
                        retCode = GIF_UNKNOWN_ERROR;
                        found = TRUE;
                    }
                break;
            }
        }while(!found);
    }

    return retCode;
}



SDWORD GifGetGifInfo(ST_GIF_INFO* pstOutInfo)
{
    SDWORD returnCode = GIF_NO_ERROR;
    ST_GIF_IMAGE_DESC imageDesc;
    WORD blockCount = 0;

    MP_DEBUG("GifGetGifInfo");
    returnCode = gifCheckScreenDesc(&gifScreenDesc);
    if(returnCode != 0)
        return returnCode;    

    if(returnCode == GIF_NO_ERROR)	//neil modify	2007-8-22
    {
        BOOL finish = FALSE;
        BYTE ch;

        do
        {
            if(1 != gifCallback.refillBuffer((DWORD)&ch, 1))
            {
                returnCode = GIF_GET_STREAM_FAIL;
                finish = TRUE;

                break;
            }

            switch(ch)
            {
                case 0x21: //extension block
                    returnCode = gifProcessExtension();
    		   if(returnCode != 0)
    			   return returnCode;
                break;

                case 0x2c: //image descriptor
                {
                    returnCode = gifBypassImage();
                    gifFrameNum++;
                    if(returnCode != 0)
                        return returnCode;
                }
                break;

                case 0x3b: //terminator
                    finish = TRUE;
                break;

                default:
                    returnCode = GIF_UNSURPPORTED;
                    finish = TRUE;
                break;
            }
        }while(!finish);

        if(returnCode == GIF_NO_ERROR)
        {
            pstOutInfo->u08GifVersion = gifScreenDesc.u08Version;
            pstOutInfo->u16Width = gifScreenDesc.u16ScreenWidth;
            pstOutInfo->u16Height = gifScreenDesc.u16ScreenHeight;
            pstOutInfo->u16FrameCount = gifFrameNum;
            pstOutInfo->u16DelayTime = gifControl.u16DelayTime;
        }
    }

    return returnCode;
}


void GifCallBack(void)
{
#if 0
    ST_IMGWIN *pWin;

    pWin = Idu_GetNextWin();

    MP_DEBUG("GifCallBack, prevIdx=%d, gifFrameNum=%d", gifCurrentFrameInfo_prevIdx, gifFrameNum);
    gifCurrentFrameInfo_prevIdx++;
    if(gifCurrentFrameInfo_prevIdx == gifFrameNum)
    {
        gifCurrentFrameInfo_prevIdx = 0;
        gifCurrentFrameInfo_prevPos = 0;
    }
    ImageDraw(pWin, 0);

//avoid animation file have no delay time
#if 0
    if(pstGifControl->stGifInfo.u16DelayTime == 0)
        pstGifControl->stGifInfo.u16DelayTime = 100;
    else
    	pstGifControl->stGifInfo.u16DelayTime *= 10;
    
    AddTimerProc(pstGifControl->stGifInfo.u16DelayTime, GifCallBack);
#else
    AddTimerProc(0, GifCallBack);
#endif
#endif
}

SDWORD GifDecodeFrame(WORD inFrameIdx, DWORD u32BufferAddr, DWORD u32BufferSize, BYTE reqDataFormat,
                    WORD* pu16ImageWidth, WORD* pu16ImageHeight, WORD* pu16DelayTime)
{
    SDWORD retCode = GIF_NO_ERROR;
    BYTE ch;

    MP_DEBUG("GifDecodeFrame inFrameIdx=%d", inFrameIdx);
    if((reqDataFormat == GIF_OUTPUT_RGB888) || (reqDataFormat == GIF_OUTPUT_RGB565))
    {
        gifOutputFormat = reqDataFormat;

        retCode = gifSearchFrameStart(inFrameIdx);
        if(retCode != GIF_NO_ERROR)
			return retCode;

        if(GIF_NO_ERROR == retCode)
        {
            DWORD dataSize;

            if(reqDataFormat == GIF_OUTPUT_RGB888)
                dataSize = gifScreenDesc.u16ScreenWidth * gifScreenDesc.u16ScreenHeight * 3;
            else if(reqDataFormat == GIF_OUTPUT_RGB565)
                dataSize = gifScreenDesc.u16ScreenWidth * gifScreenDesc.u16ScreenHeight * 2;

            gifCurrentFrameInfo.u08DownSample = 1;

            MP_DEBUG("dataSize=%d, u32BufferSize=%d", dataSize, u32BufferSize);
            if(dataSize > u32BufferSize) //if > MAX_GIF_RESOLUTION
            {
#if 1
                return ERR_MEM_MALLOC; //Jasmine 070903
#else
                do
                {
                    gifCurrentFrameInfo.u08DownSample++;

                    *pu16ImageWidth = gifScreenDesc.u16ScreenWidth / gifCurrentFrameInfo.u08DownSample;
                    *pu16ImageHeight = gifScreenDesc.u16ScreenHeight / gifCurrentFrameInfo.u08DownSample;

                    if(reqDataFormat == GIF_OUTPUT_RGB888)
                        dataSize = (*pu16ImageWidth) * (*pu16ImageHeight) * 3;
                    else if(reqDataFormat == GIF_OUTPUT_RGB565)
                        dataSize = (*pu16ImageWidth) * (*pu16ImageHeight) * 2;
                }while(dataSize > u32BufferSize);
#endif                
            }
            else
            {
                *pu16ImageWidth = ALIGN_16(gifScreenDesc.u16ScreenWidth);//Jasmine 070831: add align 16
                //*pu16ImageWidth = gifScreenDesc.u16ScreenWidth;
                *pu16ImageHeight = gifScreenDesc.u16ScreenHeight;
            }

            gifCurrentFrameInfo.u16OutWidth = *pu16ImageWidth;
            gifCurrentFrameInfo.u16OutHeight = *pu16ImageHeight;

            *pu16DelayTime = gifControl.u16DelayTime;
//first decode must fill bg first
            if(inFrameIdx == 0)
            {
                if(gifScreenDesc.bGlobalColorTable)
                {
                    gifCurrentFrameInfo.u32PaletteAddr = (DWORD)pGifGlobalColorTable;
                    gifFillBackgroundColor(u32BufferAddr);//JPEG_TARGET_START_ADDRESS);
                    //gifFillTransparentColor(GIF_TEMP_LZW_ADDRESS);
                    gifFillTransparentColor(u32BufferAddr);
                }
            }

            gifProcessImage(u32BufferAddr, u32BufferSize);
            gifCurrentFrameInfo.prevIdx =  gifCurrentFrameInfo_prevIdx = inFrameIdx;
            gifCurrentFrameInfo.prevPos = gifCurrentFrameInfo_prevPos = gifCallback.getPos();
        }
    }
    else
    {
        retCode = GIF_ERROR_PARAMETER;
    }

    return retCode;
}

int Gif_Decode_Bits(IMAGEFILE *psImage, BYTE *bpSource, DWORD dwSrcBufferSize)//, STREAM *aHandle)
{
    DWORD i, j, gifoffset, dwData,dwTempData;
    BYTE *bporgStream;
    BYTE *bpStream;
    BYTE *pbSrc, *pbTrg;
    BYTE Yo, Y1, Cb, Cr;

    SDWORD iErrorCode = PASS;

    MP_DEBUG("\r\nGif_Decode_Bits\r\n");
   
    gifFrameNum = 0;
    gifCurrentFrameInfo_prevIdx = 0;
    gifCurrentFrameInfo_prevPos = 0;
    pstGifControl->u32ImageBuffer = NULL;

    gGIFBufferStart = gGIFBufferPtr = bpSource;
    gGIFBufferEnd = gGIFBufferStart + dwSrcBufferSize;

    MP_DEBUG("ext_mem_get_free_space() =%x", ext_mem_get_free_space());
    if(ext_mem_get_free_space() < GIF_DECODE_BUFFER_SIZE)//Jasmine 070903
    {
        return ERR_MEM_MALLOC;
    }   

    pbGifDecodeBuffer = (BYTE *)ext_mem_malloc(GIF_DECODE_BUFFER_SIZE); //Allocate free space

    if (pbGifDecodeBuffer == NULL) return ERR_MEM_MALLOC;    

    iErrorCode = GifModuleInit(allocProc, freeProc, fillBufferProc, seekSetProc, getPosProc);
    if(iErrorCode != 0)
    {
        if(pbGifDecodeBuffer != NULL) ext_mem_free(pbGifDecodeBuffer);
		return iErrorCode;
    }

    pstGifControl->u16CurFrameIdx = 0;
    pstGifControl->u08DataFormat = GIF_OUTPUT_RGB888;

    if(gifFrameNum == 0)
        iErrorCode = GifGetGifInfo(&pstGifControl->stGifInfo);
   
    if(iErrorCode != 0)
    {
        if(pbGifDecodeBuffer != NULL) ext_mem_free(pbGifDecodeBuffer);
		return iErrorCode;
    }

    MP_DEBUG("u16FrameCount = %d", pstGifControl->stGifInfo.u16FrameCount);
    if(pstGifControl->stGifInfo.u16FrameCount > 0)
    {
        pstGifControl->u32ImageBufferSize = (DWORD) psImage->dwTargetSize;
        pstGifControl->u32ImageBuffer = (DWORD) psImage->pbTarget;
        
        iErrorCode = GifDecodeFrame(0, 
                                pstGifControl->u32ImageBuffer,
                                pstGifControl->u32ImageBufferSize,
                                pstGifControl->u08DataFormat,
                                &pstGifControl->u16OutFrameWidth,
                                &pstGifControl->u16OutFrameHeight,
                                &pstGifControl->stGifInfo.u16DelayTime);

        if(pbGifDecodeBuffer != NULL) ext_mem_free(pbGifDecodeBuffer);        
        if(iErrorCode != 0) return iErrorCode;

        //RGB2YUV
        bporgStream = bpStream = psImage->pbTarget;

        for(i = 0; i < (wImageWidth * wImageHeight >> 1); i ++)
        {
			dwData = RGB2YUV(*bporgStream, *(bporgStream +1) , *(bporgStream + 2));
			dwTempData = RGB2YUV(*(bporgStream + 3), *(bporgStream + 4), *(bporgStream + 5));
			*bpStream = Yo = dwData >> 24;
			*(bpStream + 1) = Y1 = dwTempData >> 24;
			*(bpStream + 2) = Cb = (((dwData & 0xff00) >> 8) + ((dwTempData & 0xff00) >> 8)) >> 1;
			*(bpStream + 3) = Cr = ((dwData & 0xff) + (dwTempData & 0xff)) >> 1;

			bporgStream += 6;
			bpStream += 4;
		}
	    //if not even pair
		if((wImageWidth * wImageHeight % 2) != 0)
		{
			dwData = RGB2YUV(*bporgStream, *(bporgStream +1) , *(bporgStream + 2));
			*bpStream = Yo = dwData >> 24;
			*(bpStream + 1) = Y1 = dwData >> 24;
			*(bpStream + 2) = Cb = (dwData & 0xff00) >> 8;
			*(bpStream + 3) = Cr = dwData & 0xff;
		}
    }  
    
    MP_DEBUG("\r\nGif_Decode_Bits End\r\n");
	return iErrorCode;
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
int Gif_Decoder_DecodeImage(IMAGEFILE *psImage)
{
	MP_DEBUG("Gif_Decoder_DecodeImage");
	int iErrorCode = PASS;    
    BYTE *bpSource = NULL;
	DWORD dwReadSize, dwSrcBufferSize;

    ST_MPX_STREAM *psStream = psImage->psStream;
    
    dwSrcBufferSize = mpxStreamGetSize(psStream);    

    bpSource = (BYTE *)ext_mem_malloc(dwSrcBufferSize + 256);
    bpSource = (BYTE *)((DWORD)bpSource | 0xa0000000);


	if (mpxStreamSeek(psStream, 0, SEEK_SET) == FAIL) {
		MP_DEBUG("-E- mpxStreamSeek(psStream, 0, SEEK_SET) == FAIL");
		ext_mem_free((BYTE *)((DWORD)bpSource & ~0x20000000));
		return GIF_STREAM_ERROR;
	}
    
	dwReadSize = mpxStreamRead(bpSource, 1, dwSrcBufferSize, psStream); //read all image   
    
    if(iErrorCode == PASS)
        iErrorCode = Gif_Decode_Bits(psImage, bpSource, dwSrcBufferSize);

    if(bpSource !=NULL) 
        ext_mem_free((BYTE *)((DWORD)bpSource & ~0x20000000));

	return iErrorCode;
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
DWORD Gif_GetImageSize(IMAGEFILE *psImage)
{   
	if (psImage == NULL) return 0;
	if (psImage->psStream == NULL) return 0;
	
	ST_MPX_STREAM *psStream = psImage->psStream;
	psImage->wImageWidth = 0;
	psImage->wImageHeight = 0;

	MP_DEBUG("Gif_GetImageSize");   
	
	mpxStreamSeek(psStream, 0, SEEK_SET);
	//type "GI", B=0x47, M=0x49
	if (mpxStreamReadWord(psStream) != IMAGE_TAG_GIF)
		return GIF_ILLEGAL_FILE;
   
	//image header
	mpxStreamSeek(psStream, 6, SEEK_SET);
   
	wOriginalWidth = psImage->wImageWidth = mpxStreamReadWord_le(psStream);
	wOriginalHeight = psImage->wImageHeight = mpxStreamReadWord_le(psStream);		  

	wImageWidth = psImage->wTargetWidth = ALIGN_16(psImage->wImageWidth);
	wImageHeight = psImage->wTargetHeight = psImage->wImageHeight;
    
   	psImage->wRealTargetWidth = psImage->wImageWidth;    
	psImage->wRealTargetHeight = psImage->wImageHeight;    

    MP_DEBUG("w %d, h %d", psImage->wImageWidth, psImage->wImageHeight);
	return (psImage->wImageWidth << 16) | psImage->wImageHeight;  
}

//-------------------------------------------------------------------------------
int Gif_Decoder_Init(IMAGEFILE *psImage)
{
	MP_ASSERT(psImage != NULL);
	
	MP_DEBUG("Gif_Decoder_Init");
	psImage->ImageDecodeThumb = Gif_Decoder_DecodeImage;
	psImage->ImageDecodeImage = Gif_Decoder_DecodeImage;
			
	if (Gif_GetImageSize(psImage) != 0)
		return PASS;
	else
		return GIF_NOT_SUPPORTED_SIZE;
}
#endif

