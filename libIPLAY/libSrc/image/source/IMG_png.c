///
///@defgroup    IMAGE    Image
///

///
///@ingroup     IMAGE
///

///
///@defgroup    PNG    PNG codec
///
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"

#if PNG
#include "jpeg.h"
#include "mpapi.h"

//==========================================================================================================//
typedef struct
{
    BYTE*    pu08Data;
    DWORD     u32Length;
    BYTE     u08Current;
    DWORD     u32Index;
    SDWORD     s32SubIndex;
    SWORD     s16Error;
} ST_INPUT_DATA;

typedef struct
{
    SDWORD *ps32Data;
    SDWORD s32LastFreeParent;
    SDWORD s32LastDepth;
} ST_HUFFMAN_TREE;

ST_INPUT_DATA stInputData;
ST_HUFFMAN_TREE stMainTree, stDistancesTree, stHelperTree;
ST_PNG_CALLBACK pngCallback;
//extern ST_PNG_CALLBACK pngCallback;

static SDWORD extra_bits[] = { 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5,  6,  6,  7,  7,  8,  8,   9,   9,  10,  10,  
                            11,  11,  12,   12,   13,   13 };
static SDWORD distances[]  = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,
                            4097,6145,8193,12289,16385,24577 };
const BYTE PNG_SIGNATURE[8] = {137, 80, 78, 71, 13, 10, 26, 10};

DWORD u32CRCTable[256];
DWORD u32PngPaletteAddr = 0;
DWORD u32HuffmanMainTreeBuffer = 0;
DWORD u32HuffmanDistTreeBuffer = 0;
DWORD u32HuffmanHelpTreeBuffer = 0;
DWORD gPNGBufferStart = 0;
DWORD gPNGBufferEnd = 0;
DWORD gPNGBufferPtr = 0;
BYTE DEINTERLACE_ARRAY[7][4] = { {8, 0, 8, 0},
                                {8, 0, 8, 4},
                                {8, 4, 4, 0},
                                {4, 0, 4, 2},
                                {4, 2, 2, 0},
                                {2, 0, 2, 1},
                                {2, 1, 1, 0}};
//======================================= Data input handling ==============================================//
static void InitInputData(DWORD u32DataAddress, DWORD u32DataLength);
static SDWORD InputReadBit();
static SDWORD InputReadBits(SDWORD count);
static BYTE InputReadByte();
static DWORD InputReadBytes(SDWORD count);
//======================================= Huffman ==========================================================//
static void HuffmanTreeReset(ST_HUFFMAN_TREE* pstTree);
static void HuffmanTreeCreate(ST_HUFFMAN_TREE* pstTree);
static SDWORD HuffmanTreeDestroy(ST_HUFFMAN_TREE* pstTree);
static SDWORD HuffmanTreeAdd(ST_HUFFMAN_TREE* pstTree, SDWORD depth, SDWORD value);
static SDWORD HuffmanTreeReadNextCode(ST_HUFFMAN_TREE* pstTree);
static SDWORD get_distance(ST_HUFFMAN_TREE* pstDistanceTree);
static SDWORD load_fixed_tree(ST_HUFFMAN_TREE* pstMainTree, ST_HUFFMAN_TREE* pstDistancesTree);
static SDWORD load_dynamic_tree(ST_HUFFMAN_TREE* pstMainTree, ST_HUFFMAN_TREE* pstDistancesTree, 
                                ST_HUFFMAN_TREE* pstHelperTree);
//======================================= PNG info =========================================================//
static void pngUnfilter(WORD u16Width, WORD u16Height, BYTE u08ColorType, BYTE u08Depth, DWORD u32DataAddress, DWORD u32DataSize);
static void pngOutputRGB888(ST_PNG_INFO* pstPngInfo, DWORD u32DataAddress);
static void pngOutputRGB888Deinterlace(ST_PNG_INFO* pstPngInfo, DWORD u32DataAddress);
static void pngOutputRGB565(ST_PNG_INFO* pstPngInfo, DWORD u32DataAddress);
static void pngOutputRGB565Deinterlace(ST_PNG_INFO* pstPngInfo, DWORD u32DataAddress);
static void fillInterlaceHeader(ST_PNG_INFO* pstPngInfo);
static DWORD Inflate(BYTE* pu08CompressData, DWORD u32DataLength, BYTE* pstOutput, DWORD outputSize);
static DWORD calcInterlaceDataLength(WORD u16Width, WORD u16Height, BYTE u08Depth, BYTE u08ColorType);
//==========================================================================================================//
//Decode memory
#define PNG_PALETTE_SIZE            (256 * 3) //Jasmine 070903
#define PNG_HUFFMAN_MAIN_TREE_SIZE  (256 * 1024) //(65536 * 4)
#define PNG_HUFFMAN_DIST_TREE_SIZE  (256 * 1024)
#define PNG_HUFFMAN_HELP_TREE_SIZE  (256 * 1024)
#define PNG_COMPRESS_DATA_SIZE      (512 * 1024)

BYTE *pbPngDecodeBuffer; //Jamsine 070903

DWORD PNG_PALETTE_ADDRESS;
DWORD PNG_HUFFMAN_MAIN_TREE_ADD;
DWORD PNG_HUFFMAN_DIST_TREE_ADD;
DWORD PNG_HUFFMAN_HELP_TREE_ADD;
DWORD PNG_COMPRESS_DATA_ADD;
DWORD PNG_DEINTERLACE_ADD;

DWORD Png_GetImageSize(IMAGEFILE *psImage);
int Png_Decoder_Init(IMAGEFILE *psImage);
int Png_Decoder_DecodeImage(IMAGEFILE *psImage);
int Png_Decode_Bits(IMAGEFILE *psImage, BYTE *bpSource, DWORD dwSrcBufferSize);

//==========================================================================================================//
SDWORD PngModuleInit(PNG_MALLOC_CALLBACK mallocCallback,
                    PNG_MFREE_CALLBACK mFreeCallback,
                    PNG_FILL_BUFFER_CALLBACK fillBufferCallback,
                    PNG_SEEK_SET_CALLBACK seekSetCallback,
                    PNG_GET_POS_CALLBACK getPosCallback)
{
    if(!mallocCallback || !mFreeCallback || !fillBufferCallback || !seekSetCallback || !getPosCallback)
        return PNG_ERROR_PARAMETER;
    
    pngCallback.mAllocate = mallocCallback;
    pngCallback.mFree = mFreeCallback;
    pngCallback.refillBuffer = fillBufferCallback;
    pngCallback.seekSet = seekSetCallback;
    pngCallback.getPos = getPosCallback;
    
    u32PngPaletteAddr = 0;
    u32HuffmanMainTreeBuffer = 0;
    u32HuffmanDistTreeBuffer = 0;
    u32HuffmanHelpTreeBuffer = 0;      

    PNG_PALETTE_ADDRESS = (DWORD)pbPngDecodeBuffer;
    PNG_HUFFMAN_MAIN_TREE_ADD	= PNG_PALETTE_ADDRESS + PNG_PALETTE_SIZE;
    PNG_HUFFMAN_DIST_TREE_ADD	= PNG_HUFFMAN_MAIN_TREE_ADD + PNG_HUFFMAN_MAIN_TREE_SIZE;
    PNG_HUFFMAN_HELP_TREE_ADD = PNG_HUFFMAN_DIST_TREE_ADD + PNG_HUFFMAN_DIST_TREE_SIZE;
    PNG_COMPRESS_DATA_ADD = PNG_HUFFMAN_HELP_TREE_ADD	 + PNG_HUFFMAN_HELP_TREE_SIZE;
    PNG_DEINTERLACE_ADD = PNG_COMPRESS_DATA_ADD + PNG_COMPRESS_DATA_SIZE;

    MP_DEBUG("\r\nMEMORY ALLOCATE\r\n");
    MP_DEBUG("PNG_PALETTE_ADDRESS=%x", PNG_PALETTE_ADDRESS);
    MP_DEBUG("PNG_HUFFMAN_MAIN_TREE_ADD=%x", PNG_HUFFMAN_MAIN_TREE_ADD);
    MP_DEBUG("PNG_HUFFMAN_DIST_TREE_ADD=%x", PNG_HUFFMAN_DIST_TREE_ADD);
    MP_DEBUG("PNG_HUFFMAN_HELP_TREE_ADD=%x", PNG_HUFFMAN_HELP_TREE_ADD);
    MP_DEBUG("PNG_COMPRESS_DATA_ADD=%x", PNG_COMPRESS_DATA_ADD);
    MP_DEBUG("PNG_DEINTERLACE_ADD=%x", PNG_DEINTERLACE_ADD);
    
    u32PngPaletteAddr = (DWORD)PNG_PALETTE_ADDRESS;//(DWORD)pngCallback.mAllocate(PNG_PLTE_MAX_SIZE);
    u32HuffmanMainTreeBuffer = (DWORD)PNG_HUFFMAN_MAIN_TREE_ADD;//(DWORD)pngCallback.mAllocate(HUFFMAN_TREE_DATA_SIZE);
    u32HuffmanDistTreeBuffer = (DWORD)PNG_HUFFMAN_DIST_TREE_ADD;//(DWORD)pngCallback.mAllocate(HUFFMAN_TREE_DATA_SIZE);
    u32HuffmanHelpTreeBuffer = (DWORD)PNG_HUFFMAN_HELP_TREE_ADD;//(DWORD)pngCallback.mAllocate(HUFFMAN_TREE_DATA_SIZE);
    
    if((!u32PngPaletteAddr) || (!u32HuffmanMainTreeBuffer) || (!u32HuffmanDistTreeBuffer) || (!u32HuffmanHelpTreeBuffer))
    {
        if(u32PngPaletteAddr)
        {
            pngCallback.mFree(u32PngPaletteAddr);
            u32PngPaletteAddr = 0;
        }
        
        if(u32HuffmanMainTreeBuffer)
        {
            pngCallback.mFree(u32HuffmanMainTreeBuffer);
            u32HuffmanMainTreeBuffer = 0;
        }
        
        if(u32HuffmanDistTreeBuffer)
        {
            pngCallback.mFree(u32HuffmanDistTreeBuffer);
            u32HuffmanDistTreeBuffer = 0;
        }
        
        if(u32HuffmanHelpTreeBuffer)
        {
            pngCallback.mFree(u32HuffmanHelpTreeBuffer);
            u32HuffmanHelpTreeBuffer = 0;
        }
        
        return PNG_MEMORY_NOT_ENOUGH;
    }
    
    SetHuffmanTreeBuffer(u32HuffmanMainTreeBuffer, u32HuffmanDistTreeBuffer, u32HuffmanHelpTreeBuffer);
    
    return PNG_NO_ERROR;
}
//-------------------------------------------------------------------------------------
SDWORD PngModuleRelease()
{   
    if(u32PngPaletteAddr)
    {
        pngCallback.mFree(u32PngPaletteAddr);
        u32PngPaletteAddr = 0;
    }
    
    if(u32HuffmanMainTreeBuffer)
    {
        pngCallback.mFree(u32HuffmanMainTreeBuffer);
        u32PngPaletteAddr = 0;
    }
    
    if(u32HuffmanDistTreeBuffer)
    {
        pngCallback.mFree(u32HuffmanDistTreeBuffer);
        u32PngPaletteAddr = 0;
    }
    
    if(u32HuffmanHelpTreeBuffer)
    {
        pngCallback.mFree(u32HuffmanHelpTreeBuffer);
        u32PngPaletteAddr = 0;
    }
    
    pngCallback.mAllocate = 0;
    pngCallback.mFree = 0;
    pngCallback.refillBuffer = 0;
    pngCallback.seekSet = 0;
    pngCallback.getPos = 0;

    return PNG_NO_ERROR;
}
//-------------------------------------------------------------------------------------
SDWORD PngCheckHeader(ST_PNG_INFO* pstPngInfo)
{
    BYTE signature[8];
    DWORD i;
    SDWORD retCode = PNG_NO_ERROR;
    DWORD backupPos = 0;

    MP_DEBUG("PngCheckHeader");
    pstPngInfo->u32DeinterlaceBuffer = 0;
        
    if(!pstPngInfo)
        return PNG_ERROR_PARAMETER;
    
    pngCallback.seekSet(0); //from start
    if(8 != pngCallback.refillBuffer((DWORD)signature, 8))
        return PNG_STREAM_ERROR;
    
    for(i = 0; i < 8; i++)
    {
        if(signature[i] != PNG_SIGNATURE[i])
        {
            retCode = PNG_ILLEGAL_FILE;
            break;
        }
    }
    
    CRCTableCreate();
    
    if(retCode == PNG_NO_ERROR)
    {
        BYTE tempBuf[4];

        DWORD chunkLength, chunkType;
        BOOL finish = FALSE;

        BOOL firstChunk = TRUE;
        
        pstPngInfo->u32PngFlag = 0;
        pstPngInfo->u32FirstIDATPos = 0;
        pstPngInfo->u32CompressDataLength = 0;
        
        /*=============================================================================
        chunk layout:
         --------
        |        |  chunk data length, 4 bytes, not include chunk type, crc and itself, can be 0
         --------
        |        |  chunk type, 4 bytes, the identify of this chunk data
         --------
        |        |  chunk data
         --------
        |        |  CRC, 4 bytes
         --------
        =============================================================================*/
        
        while(!finish)
        {
            DWORD crc = 0;
            
            backupPos = pngCallback.getPos();
            
            pngCallback.refillBuffer((DWORD)tempBuf, 4);
            chunkLength = PNG_GET_4_BYTE(tempBuf);
            
            if(chunkLength > PNG_MAX_U32)
            {
                retCode = PNG_ILLEGAL_FILE;
                break;
            }
            
            pngCallback.refillBuffer((DWORD)tempBuf, 4);
            chunkType = PNG_GET_4_BYTE(tempBuf);
            
            if(firstChunk)
            {
                if(chunkType != PNG_TAG_IMAGE_HEADER)
                    return PNG_ILLEGAL_FILE;
                
                firstChunk = FALSE;
            }
            
            switch(chunkType)
            {
                case PNG_TAG_IMAGE_HEADER:
                    retCode = pngProcessIHDR(pstPngInfo);
                break;
                
                case PNG_TAG_PALETTE:
                    retCode = pngProcessPLTE(pstPngInfo, chunkLength);
                break;
                
                case PNG_TAG_IMAGE_DATA: //check header, and no need to decode image
                    if(!pstPngInfo->u32FirstIDATPos)
                    {
                        pstPngInfo->u32FirstIDATPos = backupPos;
                    }
                    
                    pstPngInfo->u32CompressDataLength += chunkLength;
                    pstPngInfo->u32PngFlag |= PNG_FLAG_IDAT_AVAIL;
                break;
                
                case PNG_TAG_IMAGE_TRAILER: //png end
                    finish = TRUE;
                    if(!(pstPngInfo->u32PngFlag & 0x00000004))//PNG_FLAG_IDAT_AVAIL))
                        retCode = PNG_ILLEGAL_FILE;
                break;
                
                case PNG_TAG_BACKGROUND_COLOR:
                    retCode = pngProcessbKGD(pstPngInfo);
                break;
                
                case PNG_TAG_PRIMARY_CHROMA:
                    retCode = pngProcesscHRM(pstPngInfo);
                break;
                
                case PNG_TAG_IMAGE_GAMA:
                    retCode = pngProcessgAMA(pstPngInfo);
                break;
                
                case PNG_TAG_TRANSPARENCY:
                    retCode = pngProcesstRNS(pstPngInfo);
                break;
                
                case PNG_TAG_IMAGE_HISTOGRAM:
                case PNG_TAG_PHYSICAL_PIXEL:
                case PNG_TAG_SIGNIFICANT_BITS:
                case PNG_TAG_TEXTUAL_DATA:
                case PNG_TAG_LAST_MODIFY_TIME:
                    //i don't want to process these tags
                break;
                
                default:
                break;
            }
            
            if(retCode != PNG_NO_ERROR)
                break;
            
            crc = CrcCalculate(backupPos+4, chunkLength+4);
            pngCallback.seekSet(backupPos + 8 + chunkLength);
            pngCallback.refillBuffer((DWORD)tempBuf, 4);
            if(crc != PNG_GET_4_BYTE(tempBuf))
            {
                retCode = PNG_ILLEGAL_FILE;
                break;
            }
            
            pngCallback.seekSet(backupPos + 8 + chunkLength + 4); //chunk start + chunk type + chunk length + chunk data + crc
        }
        
        if(retCode == PNG_NO_ERROR) //check the capability
        {
            if((pstPngInfo->u08Filter != 0) || (pstPngInfo->u08Interlace > PNG_INTERLACE_ADM7) ||
                (pstPngInfo->u08Compression != 0))
                retCode = PNG_UNSURPPORTED;
            else if((pstPngInfo->u08Depth == 1) || (pstPngInfo->u08Depth == 2))
                retCode = PNG_UNSURPPORTED;
            else if((pstPngInfo->u08Interlace == PNG_INTERLACE_ADM7) && (pstPngInfo->u08Depth < 8))
                retCode = PNG_UNSURPPORTED;
            else if(!(pstPngInfo->u32PngFlag & PNG_FLAG_IDAT_AVAIL))
                retCode = PNG_ILLEGAL_FILE;
            else if(pstPngInfo->u32CompressDataLength < 7)
                retCode = PNG_ILLEGAL_FILE;
                
            if(retCode == PNG_NO_ERROR)
            {
                switch(pstPngInfo->u08ColorType)
                {
                    case PNG_COLOR_TYPE_GRAY_SCALE:
                        if((pstPngInfo->u08Depth != 1) && (pstPngInfo->u08Depth != 2) &&
                            (pstPngInfo->u08Depth != 4) && (pstPngInfo->u08Depth != 8) && (pstPngInfo->u08Depth != 16))
                            retCode = PNG_ILLEGAL_FILE;
                    break;
                    
                    case PNG_COLOR_TYPE_INDEX_COLOR:
                        if((pstPngInfo->u08Depth != 1) && (pstPngInfo->u08Depth != 2) &&
                            (pstPngInfo->u08Depth != 4) && (pstPngInfo->u08Depth != 8))
                            retCode = PNG_ILLEGAL_FILE;
                    break;
                    
                    case PNG_COLOR_TYPE_GRAY_SCALE_ALPHA:
                    case PNG_COLOR_TYPE_TRUE_COLOR:
                    case PNG_COLOR_TYPE_TRUE_COLOR_ALPHA:
                        if((pstPngInfo->u08Depth != 8) && (pstPngInfo->u08Depth != 16))
                            retCode = PNG_ILLEGAL_FILE;
                    break;
                }
            }
                
            if(retCode == PNG_NO_ERROR)
            {
                if(pstPngInfo->u08Interlace == PNG_INTERLACE_ADM7)
                {
                    fillInterlaceHeader(pstPngInfo);
                }
            }
        }
    }
    
    return retCode;
}
//-------------------------------------------------------------------------------------
void CRCTableCreate()
{
    DWORD c;
    DWORD n, k;
    
    for(n = 0; n < 256; n++)
    {
        c = (DWORD)n;
        for(k = 0; k < 8; k++)
        {
            if(c & 1)
                c = 0xedb88320L ^ (c >> 1);
            else
                c = c >> 1;
        }
        
        u32CRCTable[n] = c;
    }
}
//-------------------------------------------------------------------------------------
DWORD CrcCalculate(DWORD startPos, DWORD len)
{
//    DWORD c = 0xffffffffL;
    DWORD c = 0xffffffff;
    DWORD i;
    BYTE tempVal;
    
    pngCallback.seekSet(startPos);
    
    for(i = 0; i < len; i++)
    {
        pngCallback.refillBuffer((DWORD)&tempVal, 1);
        c = u32CRCTable[(c^tempVal) & 0xff] ^ (c >> 8);
    }
    c ^= 0xffffffff;
//    return (c^0xffffffffL);
	return c;
}
//-------------------------------------------------------------------------------------
SDWORD pngProcessIHDR(ST_PNG_INFO* pstPngInfo)
{
    BYTE tempBuf[PNG_IHDR_LEN];
    SDWORD retCode = PNG_NO_ERROR;
    
    pngCallback.refillBuffer((DWORD)tempBuf, PNG_IHDR_LEN);
    
    pstPngInfo->u16Width = PNG_GET_4_BYTE(tempBuf);
    pstPngInfo->u16Height = PNG_GET_4_BYTE(tempBuf + 4);
    
    pstPngInfo->u08Depth = tempBuf[8];
    pstPngInfo->u08ColorType = tempBuf[9];
    pstPngInfo->u08Compression = tempBuf[10];
    pstPngInfo->u08Filter = tempBuf[11];
    pstPngInfo->u08Interlace = tempBuf[12];
    
    pstPngInfo->u32PngFlag |= PNG_FLAG_IHDR_AVAIL;
    
    return retCode;
}
//-------------------------------------------------------------------------------------
SDWORD pngProcessPLTE(ST_PNG_INFO* pstPngInfo, DWORD paletteLen)
{
    if(paletteLen > PNG_PLTE_MAX_SIZE)
        return PNG_ILLEGAL_FILE;
    if(pstPngInfo->u32PngFlag & PNG_FLAG_PLTE_AVAIL)
        return PNG_ILLEGAL_FILE;
    if((paletteLen % 3) != 0)
        return PNG_ILLEGAL_FILE;
        
    pngCallback.refillBuffer(u32PngPaletteAddr, paletteLen);
    pstPngInfo->u32PaletteAddr = u32PngPaletteAddr;
    pstPngInfo->u16PaletteNum = paletteLen / 3;
    
    pstPngInfo->u32PngFlag |= PNG_FLAG_PLTE_AVAIL;
    
    return PNG_NO_ERROR;
}
//-------------------------------------------------------------------------------------
SDWORD pngProcessbKGD(ST_PNG_INFO* pstPngInfo)
{
    BYTE tempBuf[2];
    
    if(pstPngInfo->u32PngFlag & PNG_FLAG_IDAT_AVAIL) //bKGD must precede the first IDAT
        return PNG_ILLEGAL_FILE;
        
    switch(pstPngInfo->u08ColorType)
    {
        case PNG_COLOR_TYPE_GRAY_SCALE:
        case PNG_COLOR_TYPE_GRAY_SCALE_ALPHA:
            pngCallback.refillBuffer((DWORD)tempBuf, 2);
            pstPngInfo->u16BGColor[0] = PNG_GET_2_BYTE(tempBuf);
        break;
        
        case PNG_COLOR_TYPE_TRUE_COLOR:
        case PNG_COLOR_TYPE_TRUE_COLOR_ALPHA:
            pngCallback.refillBuffer((DWORD)tempBuf, 2);
            pstPngInfo->u16BGColor[0] = PNG_GET_2_BYTE(tempBuf);
            pngCallback.refillBuffer((DWORD)tempBuf, 2);
            pstPngInfo->u16BGColor[1] = PNG_GET_2_BYTE(tempBuf);
            pngCallback.refillBuffer((DWORD)tempBuf, 2);
            pstPngInfo->u16BGColor[2] = PNG_GET_2_BYTE(tempBuf);
        break;
        
        case PNG_COLOR_TYPE_INDEX_COLOR:
            pngCallback.refillBuffer((DWORD)tempBuf, 1);
            pstPngInfo->u16BGColor[0] = tempBuf[0];
        break;
    }
    
    pstPngInfo->u32PngFlag |= PNG_FLAG_bKGD_AVAIL;
    
    return PNG_NO_ERROR;
}
//-------------------------------------------------------------------------------------
SDWORD pngProcesscHRM(ST_PNG_INFO* pstPngInfo)
{
    BYTE tempBuf[PNG_cHRM_LEN];
    
    if((pstPngInfo->u32PngFlag & PNG_FLAG_IDAT_AVAIL) || 
        (pstPngInfo->u32PngFlag & PNG_FLAG_PLTE_AVAIL)) //cHRM must precede the first IDAT & PLTE
        return PNG_ILLEGAL_FILE;
        
    pngCallback.refillBuffer((DWORD)tempBuf, PNG_cHRM_LEN);
    pstPngInfo->stChromaInfo.u32WPointX = PNG_GET_4_BYTE(tempBuf);
    pstPngInfo->stChromaInfo.u32WPointY = PNG_GET_4_BYTE(tempBuf + 4);
    pstPngInfo->stChromaInfo.u32RedX = PNG_GET_4_BYTE(tempBuf + 8);
    pstPngInfo->stChromaInfo.u32RedY = PNG_GET_4_BYTE(tempBuf + 12);
    pstPngInfo->stChromaInfo.u32GreenX = PNG_GET_4_BYTE(tempBuf + 16);
    pstPngInfo->stChromaInfo.u32GreenY = PNG_GET_4_BYTE(tempBuf + 20);
    pstPngInfo->stChromaInfo.u32BlueX = PNG_GET_4_BYTE(tempBuf + 24);
    pstPngInfo->stChromaInfo.u32BlueY = PNG_GET_4_BYTE(tempBuf + 28);
    
    pstPngInfo->u32PngFlag |= PNG_FLAG_cHRM_AVAIL;
    return PNG_NO_ERROR;
}
//-------------------------------------------------------------------------------------
SDWORD pngProcessgAMA(ST_PNG_INFO* pstPngInfo)
{
    BYTE tempBuf[PNG_gAMA_LEN];
    
    if((pstPngInfo->u32PngFlag & PNG_FLAG_IDAT_AVAIL) || 
        (pstPngInfo->u32PngFlag & PNG_FLAG_PLTE_AVAIL)) //gAMA must precede the first IDAT & PLTE
        return PNG_ILLEGAL_FILE;
        
    pngCallback.refillBuffer((DWORD)tempBuf, PNG_gAMA_LEN);
    pstPngInfo->u32Gamma = PNG_GET_4_BYTE(tempBuf);
    
    pstPngInfo->u32PngFlag |= PNG_FLAG_gAMA_AVAIL;
    return PNG_NO_ERROR;
}
//-------------------------------------------------------------------------------------
SDWORD pngProcesstRNS(ST_PNG_INFO* pstPngInfo)
{
    SDWORD retCode = PNG_NO_ERROR;
    BYTE tempBuf[2];
    /*
    if((pstPngInfo->u32PngFlag & PNG_FLAG_IDAT_AVAIL) || 
        (pstPngInfo->u32PngFlag & PNG_FLAG_PLTE_AVAIL)) //tRNS must precede the first IDAT & PLTE
        return PNG_ILLEGAL_FILE;
    */
    switch(pstPngInfo->u08ColorType)
    {
        case PNG_COLOR_TYPE_GRAY_SCALE:
            pngCallback.refillBuffer((DWORD)tempBuf, 2);
            pstPngInfo->u16TransColor[0] = PNG_GET_2_BYTE(tempBuf);
        break;
        
        case PNG_COLOR_TYPE_TRUE_COLOR:
            pngCallback.refillBuffer((DWORD)tempBuf, 2);
            pstPngInfo->u16TransColor[0] = PNG_GET_2_BYTE(tempBuf);
            pngCallback.refillBuffer((DWORD)tempBuf, 2);
            pstPngInfo->u16TransColor[1] = PNG_GET_2_BYTE(tempBuf);
            pngCallback.refillBuffer((DWORD)tempBuf, 2);
            pstPngInfo->u16TransColor[2] = PNG_GET_2_BYTE(tempBuf);
        break;
        
        case PNG_COLOR_TYPE_INDEX_COLOR:
            //do nothing here because i don't want to support this
        break;
            
        case PNG_COLOR_TYPE_GRAY_SCALE_ALPHA:
        case PNG_COLOR_TYPE_TRUE_COLOR_ALPHA:
            retCode = PNG_ILLEGAL_FILE;
        break;
    }
    
    pstPngInfo->u32PngFlag |= PNG_FLAG_tRNS_AVAIL;
    return retCode;
}
//-------------------------------------------------------------------------------------
DWORD PngEvaluateImageBufferSize(ST_PNG_INFO* pstPngInfo, BYTE u08InReqDataFormat)
{
    DWORD bufferSize = pstPngInfo->u16Width * pstPngInfo->u16Height;
    DWORD uncompressSize = pngCalcOutputDataSize(pstPngInfo);
    
    if(u08InReqDataFormat == PNG_OUTPUT_RGB888)
        bufferSize *= 3;
    else
        bufferSize *= 2;
    
    return (bufferSize > uncompressSize ? bufferSize : uncompressSize);
}
//-------------------------------------------------------------------------------------
DWORD pngCalcFilterDelta(BYTE u08ColorType, BYTE u08Depth)
{
    switch(u08ColorType)
    {
        case PNG_COLOR_TYPE_GRAY_SCALE:
            if(u08Depth == 16)
            {
                return 2;
            }
        break;
        case PNG_COLOR_TYPE_TRUE_COLOR:
            if(u08Depth == 8)
            {
                return 3;
            }
            if(u08Depth == 16)
            {
                return 6;
            }
        break;
        case PNG_COLOR_TYPE_GRAY_SCALE_ALPHA:
            if(u08Depth == 8)
            {
                return 2;
            }
            if(u08Depth == 16)
            {
                return 4;
            }
        break;
        case PNG_COLOR_TYPE_TRUE_COLOR_ALPHA:
            if(u08Depth == 8)
            {
                return 4;
            }
            if(u08Depth == 16)
            {
                return 8;
            }
        break;
    }
    return 1;
}
//-------------------------------------------------------------------------------------
BYTE paethPredictor(BYTE ua, BYTE ub, BYTE uc)
{
    SDWORD a,b,c,p,pa,pb,pc;

	a = (SDWORD)ua;
	b = (SDWORD)ub;
	c = (SDWORD)uc;

	p = a+b-c;
	pa = (p>a ? p-a : a-p);
	pb = (p>b ? p-b : b-p);
	pc = (p>c ? p-c : c-p);

	if(pa <= pb && pa <= pc)
	{
		return a;
	}
	else if(pb <= pc)
	{
		return b;
	}
	else
	{
		return c;
	}
}
//-------------------------------------------------------------------------------------
DWORD pngCalcOutputDataSize(ST_PNG_INFO* pstPngInfo)
{
    SDWORD i = 0, j = 0;
    DWORD dataLength = 0;
    
    if(pstPngInfo->u16Width == 0)
        return 0;
    
    if(pstPngInfo->u08Interlace == PNG_INTERLACE_NONE)
    {
        switch(pstPngInfo->u08Depth)
        {
            case 1:
                i = (pstPngInfo->u16Width + 7) / 8;
            break;
            
            case 2:
                i = (pstPngInfo->u16Width + 3) / 4;
            break;
            
            case 4:
                i = (pstPngInfo->u16Width + 1) / 2;
            break;
            
            case 8:
                i = pstPngInfo->u16Width;
            break;
            
            case 16:
                i = 2 * pstPngInfo->u16Width;
            break;
        }
        
        switch(pstPngInfo->u08ColorType)
        {
            case 0:
            case 3:
                j = 1;
            break;
            
            case 2:
                j = 3;
            break;
            
            case 4:
                j = 2;
            break;
            
            case 6:
                j = 4;
            break;
        }
        
        dataLength = pstPngInfo->u16Height * i * j + pstPngInfo->u16Height;
    }
    else if(pstPngInfo->u08Interlace == PNG_INTERLACE_ADM7)
    {
        for(i = 0; i < 7; i++)
        {
            dataLength += pstPngInfo->stFakeHeader[i].u32RawDataLength;
        }
    }
    
    return dataLength;
}
//-------------------------------------------------------------------------------------
static void fillInterlaceHeader(ST_PNG_INFO* pstPngInfo)
{
    pstPngInfo->stFakeHeader[0].u16Width = (pstPngInfo->u16Width + 7) >> 3;
    pstPngInfo->stFakeHeader[0].u16Height = (pstPngInfo->u16Height + 7) >> 3;
    pstPngInfo->stFakeHeader[0].u32RawDataLength = calcInterlaceDataLength(pstPngInfo->stFakeHeader[0].u16Width,
                                                    pstPngInfo->stFakeHeader[0].u16Height,
                                                    pstPngInfo->u08Depth,
                                                    pstPngInfo->u08ColorType);
    pstPngInfo->stFakeHeader[1].u16Width = (pstPngInfo->u16Width - 4 + 7) >> 3;
    pstPngInfo->stFakeHeader[1].u16Height = (pstPngInfo->u16Height + 7) >> 3;
    pstPngInfo->stFakeHeader[1].u32RawDataLength = calcInterlaceDataLength(pstPngInfo->stFakeHeader[1].u16Width,
                                                    pstPngInfo->stFakeHeader[1].u16Height,
                                                    pstPngInfo->u08Depth,
                                                    pstPngInfo->u08ColorType);
    pstPngInfo->stFakeHeader[2].u16Width = (pstPngInfo->u16Width + 3) >> 2;
    pstPngInfo->stFakeHeader[2].u16Height = (pstPngInfo->u16Height - 4 + 7) >> 3;
    pstPngInfo->stFakeHeader[2].u32RawDataLength = calcInterlaceDataLength(pstPngInfo->stFakeHeader[2].u16Width,
                                                    pstPngInfo->stFakeHeader[2].u16Height,
                                                    pstPngInfo->u08Depth,
                                                    pstPngInfo->u08ColorType);
    pstPngInfo->stFakeHeader[3].u16Width = (pstPngInfo->u16Width - 2 + 3) >> 2;
    pstPngInfo->stFakeHeader[3].u16Height = (pstPngInfo->u16Height + 3) >> 2;
    pstPngInfo->stFakeHeader[3].u32RawDataLength = calcInterlaceDataLength(pstPngInfo->stFakeHeader[3].u16Width,
                                                    pstPngInfo->stFakeHeader[3].u16Height,
                                                    pstPngInfo->u08Depth,
                                                    pstPngInfo->u08ColorType);
    pstPngInfo->stFakeHeader[4].u16Width = (pstPngInfo->u16Width + 1) >> 1;
    pstPngInfo->stFakeHeader[4].u16Height = (pstPngInfo->u16Height - 2 + 3) >> 2;
    pstPngInfo->stFakeHeader[4].u32RawDataLength = calcInterlaceDataLength(pstPngInfo->stFakeHeader[4].u16Width,
                                                    pstPngInfo->stFakeHeader[4].u16Height,
                                                    pstPngInfo->u08Depth,
                                                    pstPngInfo->u08ColorType);
    pstPngInfo->stFakeHeader[5].u16Width = (pstPngInfo->u16Width - 1 + 1) >> 1;
    pstPngInfo->stFakeHeader[5].u16Height = (pstPngInfo->u16Height + 1) >> 1;
    pstPngInfo->stFakeHeader[5].u32RawDataLength = calcInterlaceDataLength(pstPngInfo->stFakeHeader[5].u16Width,
                                                    pstPngInfo->stFakeHeader[5].u16Height,
                                                    pstPngInfo->u08Depth,
                                                    pstPngInfo->u08ColorType);
    pstPngInfo->stFakeHeader[6].u16Width = pstPngInfo->u16Width;
    pstPngInfo->stFakeHeader[6].u16Height = (pstPngInfo->u16Height - 1 + 1) >> 1;
    pstPngInfo->stFakeHeader[6].u32RawDataLength = calcInterlaceDataLength(pstPngInfo->stFakeHeader[6].u16Width,
                                                    pstPngInfo->stFakeHeader[6].u16Height,
                                                    pstPngInfo->u08Depth,
                                                    pstPngInfo->u08ColorType);
}
//-------------------------------------------------------------------------------------
static DWORD calcInterlaceDataLength(WORD u16Width, WORD u16Height, BYTE u08Depth, BYTE u08ColorType)
{
    SDWORD i = 0, j = 0;
    
    if(u16Width == 0)
        return 0;
        
    switch(u08Depth)
    {
        case 1:
            i = (u16Width + 7) / 8;
        break;
        
        case 2:
            i = (u16Width + 3) / 4;
        break;
        
        case 4:
            i = (u16Width + 1) / 2;
        break;
        
        case 8:
            i = u16Width;
        break;
        
        case 16:
            i = 2 * u16Width;
        break;
    }
    
    switch(u08ColorType)
    {
        case 0:
        case 3:
            j = 1;
        break;
        
        case 2:
            j = 3;
        break;
        
        case 4:
            j = 2;
        break;
        
        case 6:
            j = 4;
        break;
    }
    
    return u16Height * i * j + u16Height;
}
//==================================================================================================
//call back function
void* mallocCallback(DWORD size)
{
    //return malloc(size);
    return ext_mem_malloc(size);
}

void mFreeCallback(DWORD address)
{
    //free((void*)address);
    ext_mem_free((void*)address);
}

DWORD fillBufferCallback(DWORD address, DWORD size)
{
    if((gPNGBufferPtr + size) > gPNGBufferEnd)
        size = gPNGBufferEnd - gPNGBufferPtr;

    memcpy((void*)address, (void*)gPNGBufferPtr, size);
    gPNGBufferPtr += size;

    return size;
}

DWORD seekSetCallback(DWORD pos)
{
    if((pos + gPNGBufferStart) > gPNGBufferEnd)
        pos = gPNGBufferEnd - gPNGBufferStart;
    gPNGBufferPtr = gPNGBufferStart + pos;

    return pos;
}

DWORD getPosCallback()
{
    return (gPNGBufferPtr - gPNGBufferStart);
}
//-------------------------------------------------------------------------------------
static void pngUnfilter(WORD u16Width, WORD u16Height, BYTE u08ColorType, BYTE u08Depth, DWORD u32DataAddress, DWORD u32DataSize)
{
    DWORD filterDelta = pngCalcFilterDelta(u08ColorType, u08Depth);
    DWORD y, x, i, a, b, c, j;
    BYTE *pSrc, *pDest;
    BYTE filterType;
    DWORD rowBytes;
    
    rowBytes = u32DataSize / u16Height;
    
    pSrc = (BYTE*)u32DataAddress;
    pDest = (BYTE*)u32DataAddress;
    for(y = 0; y < u16Height; y++)
    {
        i = y * rowBytes;
        filterType = pSrc[i];
        
        switch(filterType)
        {
            case 0:
            continue;
            
            case 1: //sub
                for(x = filterDelta + 1; x < rowBytes; x++)
                {
                    pDest[i + x] += pSrc[i + x - filterDelta];
                }
            break;
            
            case 2: //up
                if(y > 0)
                {
                    for(x = 1; x < rowBytes; x++)
                    {
                        pDest[i + x] += pSrc[i + x - rowBytes];
                    }
                }
            break;
            
            case 3: // Average 
                if(y > 0)
                {
                    for(x = 1; x <= filterDelta; x++)
                    {
                        pDest[i + x] += pSrc[i + x - rowBytes] / 2;
                    }
                    for(x = filterDelta + 1; x < rowBytes; x++)
                    {
                        a = pSrc[i + x - filterDelta];
                        b = pSrc[i + x - rowBytes];
                        pDest[i + x] += (a + b) / 2;
                    }
                }
                else
                {
                    for(x = filterDelta + 1; x < rowBytes; x++)
                    {
                        pDest[i + x] += pSrc[i + x - filterDelta] / 2;
                    }
                }
            break;
            
            case 4: // Paeth 
                if(y > 0)
                {
                    for(x = 1; x <= filterDelta; x++) 
                    {
                        pDest[i + x] += pSrc[i + x - rowBytes];
                    }
                    
                    for(x = filterDelta + 1; x < rowBytes; x++) 
                    {
                        a = pSrc[i + x - filterDelta];
                        b = pSrc[i + x - rowBytes];
                        c = pSrc[i + x - rowBytes - filterDelta];
                        pDest[i + x] += paethPredictor(a, b, c);
                    }
                }
                else
                {
                    for(x = filterDelta + 1; x < rowBytes; x++)
                    {
                        pDest[i + x] += pSrc[i + x - filterDelta];
                    }
                }
            break;
            
            default:
                //PNG_DEBUG_STR("error filter type\n");
            break;
        }
    }
}
//-------------------------------------------------------------------------------------
static void pngOutputRGB888(ST_PNG_INFO* pstPngInfo, DWORD u32DataAddress)
{
    DWORD i, j;
    DWORD rowBytes;

    
    MP_DEBUG("pngOutputRGB888");
    MP_DEBUG("w x h-> %d x %d", pstPngInfo->u16Width, pstPngInfo->u16Height);
    MP_DEBUG("u08ColorType=%d, u08Depth=%d", pstPngInfo->u08ColorType, pstPngInfo->u08Depth);
   
    switch(pstPngInfo->u08ColorType)
    {
        case PNG_COLOR_TYPE_GRAY_SCALE:
        {
            switch(pstPngInfo->u08Depth)
            {
                case 1:
                break;
                
                case 2:
                break;
                
                case 4:
                {
                    BYTE *pSrc, *pDest;
                    
                    rowBytes = ((pstPngInfo->u16Width + 1) >> 1) + 1;
                    for(i = pstPngInfo->u16Height; i > 0; i--)
                    {
                        pSrc = (BYTE*)(u32DataAddress + rowBytes * i - 1);
                        pDest = (BYTE*)(u32DataAddress + pstPngInfo->u16Width * 3 * (i-1) + 
                                    (pstPngInfo->u16Width-1) * 3);
                        for(j = pstPngInfo->u16Width; j > 0; j--)
                        {
                            if((j-1) & 0x1)
                            {
                                pDest[0] = pDest[1] = pDest[2] = ((*pSrc) & 0xf) << 4;
                            }
                            else
                            {
                                pDest[0] = pDest[1] = pDest[2] = (((*pSrc) >> 4) & 0xf) << 4;
                            }
                            
                            if(!((j-1) & 0x1))
                                pSrc--;
                            pDest -= 3;
                        }
                    }
                }
                break;
                
                case 8:
                {
                    BYTE *pSrc, *pDest;
                    
                    rowBytes = pstPngInfo->u16Width + 1;
                    for(i = pstPngInfo->u16Height; i > 0; i--)
                    {
                        pSrc = (BYTE*)(u32DataAddress + rowBytes * i - 1);
                        pDest = (BYTE*)(u32DataAddress + pstPngInfo->u16Width * 3 * i - 3);
                        for(j = pstPngInfo->u16Width; j > 0; j--)
                        {
                            pDest[0] = pDest[1] = pDest[2] = *pSrc;
                            
                            pSrc --;
                            pDest -= 3;
                        }
                    }
                }
                break;
                
                case 16:
                {
                    BYTE *pSrc, *pDest;
                    
                    rowBytes = pstPngInfo->u16Width * 2 + 1;
                    for(i = pstPngInfo->u16Height; i > 0; i--)
                    {
                        pSrc = (BYTE*)(u32DataAddress + rowBytes * i - 1);
                        pDest = (BYTE*)(u32DataAddress + pstPngInfo->u16Width * 3 * (i-1) + 
                                    (pstPngInfo->u16Width-1) * 3);        
                        for(j = pstPngInfo->u16Width; j > 0; j--)
                        {
                            pDest[0] = pDest[1] = pDest[2] = (PNG_GET_2_BYTE(pSrc) >> 8) & 0xff;
                            
                            pSrc -= 2;
                            pDest -= 3;
                        }
                    }
                }
                break;
            }
        }
        break;
        
        case PNG_COLOR_TYPE_TRUE_COLOR:
        {
            if(pstPngInfo->u08Depth == 8)
            {
                BYTE *pSrc, *pDest;
                
                rowBytes = 3 * pstPngInfo->u16Width + 1;
                for(i = 0; i < pstPngInfo->u16Height; i++)
                {
                    pSrc = (BYTE*)u32DataAddress + rowBytes * i + 1;
                    pDest = (BYTE*)(u32DataAddress + pstPngInfo->u16Width * 3 * i);
                    for(j = 0; j < pstPngInfo->u16Width; j++)
                    {
                        pDest[0] = pSrc[0];
                        pDest[1] = pSrc[1];
                        pDest[2] = pSrc[2];
                        
                        pDest += 3;
                        pSrc += 3;
                    }
                }
            }
            else if(pstPngInfo->u08Depth == 16)
            {
                BYTE *pSrc; 
                BYTE *pDest;
                WORD r, g, b;
                
                rowBytes = 6 * pstPngInfo->u16Width + 1;
                for(i = 0; i < pstPngInfo->u16Height; i++)
                {
                    pSrc = (BYTE*)(u32DataAddress + rowBytes * i + 1);
                    pDest = (BYTE*)(u32DataAddress + pstPngInfo->u16Width * 3 * i);
                    for(j = 0; j < pstPngInfo->u16Width; j++)
                    {
                        r = PNG_GET_2_BYTE(pSrc);
                        g = PNG_GET_2_BYTE(pSrc+2);
                        b = PNG_GET_2_BYTE(pSrc+4);
                        pDest[0] = (r >> 8) & 0xff;
                        pDest[1] = (g >> 8) & 0xff;
                        pDest[2] = (b >> 8) & 0xff;
                        
                        pDest += 3;
                        pSrc += 6;
                    }
                }
            }
        }
        break;
        
        case PNG_COLOR_TYPE_INDEX_COLOR:
        {
            switch(pstPngInfo->u08Depth)
            {
                case 1:
                    
                break;
                
                case 2:
                break;
                
                case 4:
                {
                    BYTE *pSrc, *pDest;
                    BYTE *palette;
                    BYTE index;
                    
                    rowBytes = ((pstPngInfo->u16Width + 1) >> 1) + 1;
                    for(i = pstPngInfo->u16Height; i > 0; i--)
                    {
                        pSrc = (BYTE*)(u32DataAddress + rowBytes * i - 1);
                        pDest = (BYTE*)(u32DataAddress + pstPngInfo->u16Width * 3 * i - 3);
                        for(j = pstPngInfo->u16Width; j > 0; j--)
                        {
                            if((j-1) & 0x1)
                                index = (*pSrc) & 0xf;
                            else
                                index = ((*pSrc) >> 4) & 0xf;
                                
                            palette = (BYTE*)(pstPngInfo->u32PaletteAddr + 3 * index);
                            pDest[0] = palette[0];
                            pDest[1] = palette[1];
                            pDest[2] = palette[2];
                            
                            if(!((j-1) & 0x1))
                                pSrc--;
                            pDest -= 3;
                        }
                    }
                }
                break;
                
                case 8:
                {
                    BYTE *pSrc, *pDest;
                    BYTE *palette;
                    
                    rowBytes = pstPngInfo->u16Width + 1;
                    for(i = pstPngInfo->u16Height; i > 0; i--)
                    {
                        pSrc = (BYTE*)(u32DataAddress + rowBytes * (i-1) + (pstPngInfo->u16Width - 1) + 1);
                        pDest = (BYTE*)(u32DataAddress + pstPngInfo->u16Width * 3 * (i-1) + 
                                    (pstPngInfo->u16Width-1) * 3);
                        for(j = pstPngInfo->u16Width; j > 0; j--)
                        {
                            palette = (BYTE*)(pstPngInfo->u32PaletteAddr + 3 * (*pSrc));
                            pDest[0] = palette[0];
                            pDest[1] = palette[1];
                            pDest[2] = palette[2];
                            
                            pSrc--;
                            pDest -= 3;
                        }
                    }
                }
                break;
            }
        }
        break;
        
        case PNG_COLOR_TYPE_GRAY_SCALE_ALPHA:
        {
            if(pstPngInfo->u08Depth == 8)
            {
                BYTE *pSrc, *pDest;
                BYTE bg = 255;
                BYTE alpha, gray;
                
                if(pstPngInfo->u32PngFlag & PNG_FLAG_bKGD_AVAIL)
                    bg = pstPngInfo->u16BGColor[0] & 0xff;
                
                rowBytes = 2 * pstPngInfo->u16Width + 1;
                for(i = pstPngInfo->u16Height; i > 0; i--)
                {
                    pSrc = (BYTE*)(u32DataAddress + rowBytes * (i-1) + 1 + ((pstPngInfo->u16Width-1) << 1));
                    pDest = (BYTE*)(u32DataAddress + pstPngInfo->u16Width * 3 * (i-1) + 
                                    (pstPngInfo->u16Width-1) * 3);
                                    
                    for(j = pstPngInfo->u16Width; j > 0; j--)
                    {
                        gray = pSrc[0];
                        alpha = pSrc[1];
                        
                        if(alpha == 0)
                        {
                            pDest[0] = pDest[1] = pDest[2] = bg;
                        }
                        else if(pSrc[1] == 255)
                        {
                            pDest[0] = pDest[1] = pDest[2] = gray;
                        }
                        else
                        {
                            pDest[0] = pDest[1] = pDest[2] = (gray * alpha + bg * (256 - alpha)) >> 8;
                        }
                        
                        pSrc -= 2;
                        pDest -= 3;
                    }
                }
            }
            else if(pstPngInfo->u08Depth == 16)
            {
                BYTE *pSrc, *pDest;
                WORD bg = 65535;
                WORD alpha, gray;
                
                if(pstPngInfo->u32PngFlag & PNG_FLAG_bKGD_AVAIL)
                    bg = pstPngInfo->u16BGColor[0];
                
                rowBytes = 4 * pstPngInfo->u16Width + 1;
                for(i = pstPngInfo->u16Height; i > 0; i--)
                {
                    pSrc = (BYTE*)(u32DataAddress + rowBytes * (i-1) + 1 + ((pstPngInfo->u16Width-1) << 2));
                    pDest = (BYTE*)(u32DataAddress + pstPngInfo->u16Width * 3 * (i-1) + 
                                    (pstPngInfo->u16Width-1) * 3);
                                    
                    for(j = pstPngInfo->u16Width; j > 0; j--)
                    {
                        gray = PNG_GET_2_BYTE(pSrc);
                        alpha = PNG_GET_2_BYTE(pSrc+2);
                        
                        if(alpha == 0)
                        {
                            pDest[0] = pDest[1] = pDest[2] = (bg >> 8) & 0xff;
                        }
                        else if(alpha == 65535)
                        {
                            pDest[0] = pDest[1] = pDest[2] = (gray >> 8) & 0xff;
                        }
                        else
                        {
                            pDest[0] = pDest[1] = pDest[2] = (((gray * alpha + bg * (65536 - alpha)) >> 16) >> 8) & 0xff;
                        }
                        
                        pSrc -= 4;
                        pDest -= 3;
                    }
                }
            }
        }
        break;
        
        case PNG_COLOR_TYPE_TRUE_COLOR_ALPHA:
        {
            if(pstPngInfo->u08Depth == 8)
            {
                BYTE *pSrc, *pDest;
                BYTE bgR = 255, bgG = 255, bgB = 255;
                
                if(pstPngInfo->u32PngFlag & PNG_FLAG_bKGD_AVAIL)
                {
                    bgR = pstPngInfo->u16BGColor[0] & 0xff;
                    bgG = pstPngInfo->u16BGColor[1] & 0xff;
                    bgB = pstPngInfo->u16BGColor[2] & 0xff;
                }
                
                rowBytes = 4 * pstPngInfo->u16Width + 1;
                for(i = 0; i < pstPngInfo->u16Height; i++)
                {
                    pSrc = (BYTE*)u32DataAddress + rowBytes * i + 1;
                    pDest = (BYTE*)(u32DataAddress + pstPngInfo->u16Width * 3 * i);
                    for(j = 0; j < pstPngInfo->u16Width; j++)
                    {
                        if(pSrc[3] == 0)
                        {
                            pDest[0] = bgR;
                            pDest[1] = bgG;
                            pDest[2] = bgB;
                        }
                        else if(pSrc[3] == 255)
                        {
                            pDest[0] = pSrc[0];
                            pDest[1] = pSrc[1];
                            pDest[2] = pSrc[2];
                        }
                        else
                        {
                            pDest[0] = (pSrc[0] * pSrc[3] + bgR * (256 - pSrc[3])) >> 8;
                            pDest[1] = (pSrc[1] * pSrc[3] + bgG * (256 - pSrc[3])) >> 8;
                            pDest[2] = (pSrc[2] * pSrc[3] + bgB * (256 - pSrc[3])) >> 8;
                        }
                        pDest += 3;
                        pSrc += 4;
                    }
                }
            }
            else if(pstPngInfo->u08Depth == 16)
            {
                BYTE *pSrc; 
                BYTE *pDest;
                WORD bgR = 65535, bgG = 65535, bgB = 65535;
                WORD r, g, b, alpha;
                
                if(pstPngInfo->u32PngFlag & PNG_FLAG_bKGD_AVAIL)
                {
                    bgR = pstPngInfo->u16BGColor[0];
                    bgG = pstPngInfo->u16BGColor[1];
                    bgB = pstPngInfo->u16BGColor[2];
                }
                
                rowBytes = 8 * pstPngInfo->u16Width + 1;
                for(i = 0; i < pstPngInfo->u16Height; i++)
                {
                    pSrc = (BYTE*)(u32DataAddress + rowBytes * i + 1);
                    pDest = (BYTE*)(u32DataAddress + pstPngInfo->u16Width * 3 * i);
                    for(j = 0; j < pstPngInfo->u16Width; j++)
                    {
                        alpha = PNG_GET_2_BYTE(pSrc+6);
                        if(alpha == 0)
                        {
                            pDest[0] = (bgR >> 8) & 0xff;
                            pDest[1] = (bgG >> 8) & 0xff;
                            pDest[2] = (bgB >> 8) & 0xff;
                        }
                        else if(alpha == 65535)
                        {
                            r = PNG_GET_2_BYTE(pSrc);
                            g = PNG_GET_2_BYTE(pSrc+2);
                            b = PNG_GET_2_BYTE(pSrc+4);
                            pDest[0] = (r >> 8) & 0xff;
                            pDest[1] = (g >> 8) & 0xff;
                            pDest[2] = (b >> 8) & 0xff;
                        }
                        else
                        {
                            r = PNG_GET_2_BYTE(pSrc);
                            g = PNG_GET_2_BYTE(pSrc+2);
                            b = PNG_GET_2_BYTE(pSrc+4);
                            pDest[0] = (((r * alpha + bgR * (65536 - alpha)) >> 16) >> 8) & 0xff;
                            pDest[1] = (((g * alpha + bgG * (65536 - alpha)) >> 16) >> 8) & 0xff;
                            pDest[2] = (((b * alpha + bgB * (65536 - alpha)) >> 16) >> 8) & 0xff;
                        }
                        
                        pDest += 3;
                        pSrc += 8;
                    }
                }
            }
        }
        break;
    }
}
//-------------------------------------------------------------------------------------
static void pngOutputRGB888Deinterlace(ST_PNG_INFO* pstPngInfo, DWORD u32DataAddress)
{
    DWORD i, j;
    DWORD rowBytes;
    DWORD u32SrcAddress = pstPngInfo->u32DeinterlaceBuffer;
    BYTE pass = 0;
    
    for(i = 0; i < 7; i++)
    {
        pngUnfilter(pstPngInfo->stFakeHeader[i].u16Width, pstPngInfo->stFakeHeader[i].u16Height,
                    pstPngInfo->u08ColorType, pstPngInfo->u08Depth, 
                    u32SrcAddress, pstPngInfo->stFakeHeader[i].u32RawDataLength);
        u32SrcAddress += pstPngInfo->stFakeHeader[i].u32RawDataLength;
    }
    
    u32SrcAddress = pstPngInfo->u32DeinterlaceBuffer;
    
    switch(pstPngInfo->u08ColorType)
    {
        case PNG_COLOR_TYPE_GRAY_SCALE:
        {
            switch(pstPngInfo->u08Depth)
            {
                case 1:
                break;
                
                case 2:
                break;
                
                case 4:
                {
                }
                break;
                
                case 8:
                {
                    BYTE *pSrc, *pDest;
                
                    for(pass = 0; pass < 7; pass++)
                    {
                        rowBytes = pstPngInfo->stFakeHeader[pass].u16Width + 1;
                        for(i = 0; i < pstPngInfo->stFakeHeader[pass].u16Height; i++)
                        {
                            pSrc = (BYTE*)u32SrcAddress + rowBytes * i + 1;
                            pDest = (BYTE*)u32DataAddress + 
                                    (i * DEINTERLACE_ARRAY[pass][0] + DEINTERLACE_ARRAY[pass][1]) * 3 * pstPngInfo->u16Width +
                                    3 * DEINTERLACE_ARRAY[pass][3];
                            for(j = 0; j < pstPngInfo->stFakeHeader[pass].u16Width; j++)
                            {
                                pDest[0] = pDest[1] = pDest[2] = *pSrc;
                                
                                pSrc++;
                                pDest += (DEINTERLACE_ARRAY[pass][2] * 3);
                            }
                        }
                        u32SrcAddress += pstPngInfo->stFakeHeader[pass].u32RawDataLength;
                    }
                }
                break;
                
                case 16:
                {
                    BYTE *pSrc, *pDest;
                    BYTE tempVal;
                
                    for(pass = 0; pass < 7; pass++)
                    {
                        rowBytes = pstPngInfo->stFakeHeader[pass].u16Width * 2 + 1;
                        for(i = 0; i < pstPngInfo->stFakeHeader[pass].u16Height; i++)
                        {
                            pSrc = (BYTE*)u32SrcAddress + rowBytes * i + 1;
                            pDest = (BYTE*)u32DataAddress + 
                                    (i * DEINTERLACE_ARRAY[pass][0] + DEINTERLACE_ARRAY[pass][1]) * 3 * pstPngInfo->u16Width +
                                    3 * DEINTERLACE_ARRAY[pass][3];
                            for(j = 0; j < pstPngInfo->stFakeHeader[pass].u16Width; j++)
                            {
                                pDest[0] = pDest[1] = pDest[2] = (PNG_GET_2_BYTE(pSrc) >> 8) & 0xff;
                                
                                pSrc += 2;
                                pDest += (DEINTERLACE_ARRAY[pass][2] * 3);
                            }
                        }
                        u32SrcAddress += pstPngInfo->stFakeHeader[pass].u32RawDataLength;
                    }
                }
                break;
            }
        }
        break;
        
        case PNG_COLOR_TYPE_TRUE_COLOR:
        {
            if(pstPngInfo->u08Depth == 8)
            {
                BYTE *pSrc, *pDest;
                
                for(pass = 0; pass < 7; pass++)
                {
                    rowBytes = 3 * pstPngInfo->stFakeHeader[pass].u16Width + 1;
                    for(i = 0; i < pstPngInfo->stFakeHeader[pass].u16Height; i++)
                    {
                        pSrc = (BYTE*)u32SrcAddress + rowBytes * i + 1;
                        pDest = (BYTE*)u32DataAddress + 
                                (i * DEINTERLACE_ARRAY[pass][0] + DEINTERLACE_ARRAY[pass][1]) * 3 * pstPngInfo->u16Width +
                                3 * DEINTERLACE_ARRAY[pass][3];
                        for(j = 0; j < pstPngInfo->stFakeHeader[pass].u16Width; j++)
                        {
                            pDest[0] = pSrc[0];
                            pDest[1] = pSrc[1];
                            pDest[2] = pSrc[2];
                            
                            pSrc += 3;
                            pDest += (DEINTERLACE_ARRAY[pass][2] * 3);
                        }
                    }
                    u32SrcAddress += pstPngInfo->stFakeHeader[pass].u32RawDataLength;
                }
            }
            else if(pstPngInfo->u08Depth == 16)
            {
                BYTE *pSrc, *pDest;
                WORD r, g, b;
                
                for(pass = 0; pass < 7; pass++)
                {
                    rowBytes = 6 * pstPngInfo->stFakeHeader[pass].u16Width + 1;
                    for(i = 0; i < pstPngInfo->stFakeHeader[pass].u16Height; i++)
                    {
                        pSrc = (BYTE*)u32SrcAddress + rowBytes * i + 1;
                        pDest = (BYTE*)u32DataAddress + 
                                (i * DEINTERLACE_ARRAY[pass][0] + DEINTERLACE_ARRAY[pass][1]) * 3 * pstPngInfo->u16Width +
                                3 * DEINTERLACE_ARRAY[pass][3];
                        for(j = 0; j < pstPngInfo->stFakeHeader[pass].u16Width; j++)
                        {
                            r = PNG_GET_2_BYTE(pSrc);
                            g = PNG_GET_2_BYTE(pSrc+2);
                            b = PNG_GET_2_BYTE(pSrc+4);
                            
                            pDest[0] = (r >> 8) & 0xff;
                            pDest[1] = (g >> 8) & 0xff;
                            pDest[2] = (b >> 8) & 0xff;
                            
                            pSrc += 6;
                            pDest += (DEINTERLACE_ARRAY[pass][2] * 3);
                        }
                    }
                    u32SrcAddress += pstPngInfo->stFakeHeader[pass].u32RawDataLength;
                }
            }
        }
        break;
        
        case PNG_COLOR_TYPE_INDEX_COLOR:
        {
            switch(pstPngInfo->u08Depth)
            {
                case 8:
                {
                    BYTE *pSrc, *pDest;
                    BYTE *palette;
                    
                    for(pass = 0; pass < 7; pass++)
                    {
                        rowBytes = pstPngInfo->stFakeHeader[pass].u16Width + 1;
                        for(i = 0; i < pstPngInfo->stFakeHeader[pass].u16Height; i++)
                        {
                            pSrc = (BYTE*)u32SrcAddress + rowBytes * i + 1;
                            pDest = (BYTE*)u32DataAddress + 
                                    (i * DEINTERLACE_ARRAY[pass][0] + DEINTERLACE_ARRAY[pass][1]) * 3 * pstPngInfo->u16Width +
                                    3 * DEINTERLACE_ARRAY[pass][3];
                            for(j = 0; j < pstPngInfo->stFakeHeader[pass].u16Width; j++)
                            {
                                palette = (BYTE*)(pstPngInfo->u32PaletteAddr + 3 * (*pSrc));
                                pDest[0] = palette[0];
                                pDest[1] = palette[1];
                                pDest[2] = palette[2];
                                    
                                pSrc++;
                                pDest += (DEINTERLACE_ARRAY[pass][2] * 3);
                            }
                        }
                        u32SrcAddress += pstPngInfo->stFakeHeader[pass].u32RawDataLength;
                    }
                }
                break;
            }
        }
        break;
        
        case PNG_COLOR_TYPE_GRAY_SCALE_ALPHA:
        {
            if(pstPngInfo->u08Depth == 8)
            {
                BYTE *pSrc, *pDest;
                BYTE bg = 255;
                BYTE alpha, gray;
                
                if(pstPngInfo->u32PngFlag & PNG_FLAG_bKGD_AVAIL)
                    bg = pstPngInfo->u16BGColor[0] & 0xff;
                    
                for(pass = 0; pass < 7; pass++)
                {
                    rowBytes = 2 * pstPngInfo->stFakeHeader[pass].u16Width + 1;
                    for(i = 0; i < pstPngInfo->stFakeHeader[pass].u16Height; i++)
                    {
                        pSrc = (BYTE*)u32SrcAddress + rowBytes * i + 1;
                        pDest = (BYTE*)u32DataAddress + 
                                (i * DEINTERLACE_ARRAY[pass][0] + DEINTERLACE_ARRAY[pass][1]) * 3 * pstPngInfo->u16Width +
                                3 * DEINTERLACE_ARRAY[pass][3];
                        for(j = 0; j < pstPngInfo->stFakeHeader[pass].u16Width; j++)
                        {
                            gray = pSrc[0];
                            alpha = pSrc[1];
                        
                            if(alpha == 0)
                            {
                                pDest[0] = pDest[1] = pDest[2] = bg;
                            }
                            else if(pSrc[1] == 255)
                            {
                                pDest[0] = pDest[1] = pDest[2] = gray;
                            }
                            else
                            {
                                pDest[0] = pDest[1] = pDest[2] = (gray * alpha + bg * (256 - alpha)) >> 8;
                            }
                            
                            pSrc += 2;
                            pDest += (DEINTERLACE_ARRAY[pass][2] * 3);
                        }
                    }
                    u32SrcAddress += pstPngInfo->stFakeHeader[pass].u32RawDataLength;
                }
            }
            else if(pstPngInfo->u08Depth == 16)
            {
                BYTE *pSrc, *pDest;
                WORD bg = 65535;
                WORD alpha, gray;
                
                if(pstPngInfo->u32PngFlag & PNG_FLAG_bKGD_AVAIL)
                    bg = pstPngInfo->u16BGColor[0];
                    
                for(pass = 0; pass < 7; pass++)
                {
                    rowBytes = 4 * pstPngInfo->stFakeHeader[pass].u16Width + 1;
                    for(i = 0; i < pstPngInfo->stFakeHeader[pass].u16Height; i++)
                    {
                        pSrc = (BYTE*)u32SrcAddress + rowBytes * i + 1;
                        pDest = (BYTE*)u32DataAddress + 
                                (i * DEINTERLACE_ARRAY[pass][0] + DEINTERLACE_ARRAY[pass][1]) * 3 * pstPngInfo->u16Width +
                                3 * DEINTERLACE_ARRAY[pass][3];
                        for(j = 0; j < pstPngInfo->stFakeHeader[pass].u16Width; j++)
                        {
                            gray = PNG_GET_2_BYTE(pSrc);
                            alpha = PNG_GET_2_BYTE(pSrc+2);
                        
                            if(alpha == 0)
                            {
                                pDest[0] = pDest[1] = pDest[2] = (bg >> 8) & 0xff;
                            }
                            else if(alpha == 65535)
                            {
                                pDest[0] = pDest[1] = pDest[2] = (gray >> 8) & 0xff;
                            }
                            else
                            {
                                pDest[0] = pDest[1] = pDest[2] = (((gray * alpha + bg * (65536 - alpha)) >> 16) >> 8) & 0xff;
                            }
                            
                            pSrc += 4;
                            pDest += (DEINTERLACE_ARRAY[pass][2] * 3);
                        }
                    }
                    u32SrcAddress += pstPngInfo->stFakeHeader[pass].u32RawDataLength;
                }
            }
        }
        break;
        
        case PNG_COLOR_TYPE_TRUE_COLOR_ALPHA:
        {
            if(pstPngInfo->u08Depth == 8)
            {
                BYTE *pSrc, *pDest;
                BYTE bgR = 255, bgG = 255, bgB = 255;
                
                if(pstPngInfo->u32PngFlag & PNG_FLAG_bKGD_AVAIL)
                {
                    bgR = pstPngInfo->u16BGColor[0] & 0xff;
                    bgG = pstPngInfo->u16BGColor[1] & 0xff;
                    bgB = pstPngInfo->u16BGColor[2] & 0xff;
                }
                
                for(pass = 0; pass < 7; pass++)
                {
                    rowBytes = 4 * pstPngInfo->stFakeHeader[pass].u16Width + 1;
                    for(i = 0; i < pstPngInfo->stFakeHeader[pass].u16Height; i++)
                    {
                        pSrc = (BYTE*)u32SrcAddress + rowBytes * i + 1;
                        pDest = (BYTE*)u32DataAddress + 
                                (i * DEINTERLACE_ARRAY[pass][0] + DEINTERLACE_ARRAY[pass][1]) * 3 * pstPngInfo->u16Width +
                                3 * DEINTERLACE_ARRAY[pass][3];
                        for(j = 0; j < pstPngInfo->stFakeHeader[pass].u16Width; j++)
                        {
                            if(pSrc[3] == 0)
                            {
                                pDest[0] = bgR;
                                pDest[1] = bgG;
                                pDest[2] = bgB;
                            }
                            else if(pSrc[3] == 255)
                            {
                                pDest[0] = pSrc[0];
                                pDest[1] = pSrc[1];
                                pDest[2] = pSrc[2];
                            }
                            else
                            {
                                pDest[0] = (pSrc[0] * pSrc[3] + bgR * (256 - pSrc[3])) >> 8;
                                pDest[1] = (pSrc[1] * pSrc[3] + bgG * (256 - pSrc[3])) >> 8;
                                pDest[2] = (pSrc[2] * pSrc[3] + bgB * (256 - pSrc[3])) >> 8;
                            }
                            
                            pSrc += 4;
                            pDest += (DEINTERLACE_ARRAY[pass][2] * 3);
                        }
                    }
                    u32SrcAddress += pstPngInfo->stFakeHeader[pass].u32RawDataLength;
                }
            }
            else if(pstPngInfo->u08Depth == 16)
            {
                BYTE *pSrc; 
                BYTE *pDest;
                WORD bgR = 65535, bgG = 65535, bgB = 65535;
                WORD r, g, b, alpha;
                
                if(pstPngInfo->u32PngFlag & PNG_FLAG_bKGD_AVAIL)
                {
                    bgR = pstPngInfo->u16BGColor[0];
                    bgG = pstPngInfo->u16BGColor[1];
                    bgB = pstPngInfo->u16BGColor[2];
                }
                
                for(pass = 0; pass < 7; pass++)
                {
                    rowBytes = 8 * pstPngInfo->stFakeHeader[pass].u16Width + 1;
                    for(i = 0; i < pstPngInfo->stFakeHeader[pass].u16Height; i++)
                    {
                        pSrc = (BYTE*)u32SrcAddress + rowBytes * i + 1;
                        pDest = (BYTE*)u32DataAddress + 
                                (i * DEINTERLACE_ARRAY[pass][0] + DEINTERLACE_ARRAY[pass][1]) * 3 * pstPngInfo->u16Width +
                                3 * DEINTERLACE_ARRAY[pass][3];
                        for(j = 0; j < pstPngInfo->stFakeHeader[pass].u16Width; j++)
                        {
                            alpha = PNG_GET_2_BYTE(pSrc+6);
                            if(alpha == 0)
                            {
                                pDest[0] = (bgR >> 8) & 0xff;
                                pDest[1] = (bgG >> 8) & 0xff;
                                pDest[2] = (bgB >> 8) & 0xff;
                            }
                            else if(alpha == 65535)
                            {
                                r = PNG_GET_2_BYTE(pSrc);
                                g = PNG_GET_2_BYTE(pSrc+2);
                                b = PNG_GET_2_BYTE(pSrc+4);
                                pDest[0] = (r >> 8) & 0xff;
                                pDest[1] = (g >> 8) & 0xff;
                                pDest[2] = (b >> 8) & 0xff;
                            }
                            else
                            {
                                r = PNG_GET_2_BYTE(pSrc);
                                g = PNG_GET_2_BYTE(pSrc+2);
                                b = PNG_GET_2_BYTE(pSrc+4);
                                pDest[0] = (((r * alpha + bgR * (65536 - alpha)) >> 16) >> 8) & 0xff;
                                pDest[1] = (((g * alpha + bgG * (65536 - alpha)) >> 16) >> 8) & 0xff;
                                pDest[2] = (((b * alpha + bgB * (65536 - alpha)) >> 16) >> 8) & 0xff;
                            }
                            
                            pSrc += 8;
                            pDest += (DEINTERLACE_ARRAY[pass][2] * 3);
                        }
                    }
                    u32SrcAddress += pstPngInfo->stFakeHeader[pass].u32RawDataLength;
                }
            }
        }
        break;
    }
}
//-------------------------------------------------------------------------------------
static void pngOutputRGB565(ST_PNG_INFO* pstPngInfo, DWORD u32DataAddress)
{
    DWORD i, j;
    DWORD rowBytes;
    
    switch(pstPngInfo->u08ColorType)
    {
        case PNG_COLOR_TYPE_GRAY_SCALE:
        {
            switch(pstPngInfo->u08Depth)
            {
                case 1:
                break;
                
                case 2:
                break;
                
                case 4:
                {
                    BYTE *pSrc;
                    WORD *pDest;
                    BYTE tempVal;
                    
                    rowBytes = ((pstPngInfo->u16Width + 1) >> 1) + 1;
                    for(i = pstPngInfo->u16Height; i > 0; i--)
                    {
                        pSrc = (BYTE*)(u32DataAddress + rowBytes * i - 1);
                        pDest = (WORD*)(u32DataAddress + pstPngInfo->u16Width * 2 * (i-1) + 
                                    (pstPngInfo->u16Width-1) * 2);
                        for(j = pstPngInfo->u16Width; j > 0; j--)
                        {
                            if((j-1) & 0x1)
                            {
                                tempVal = ((*pSrc) & 0xf) << 4;
                            }
                            else
                            {
                                tempVal = (((*pSrc) >> 4) & 0xf) << 4;
                            }
                            
                            *pDest = PNG_888_TO_565(tempVal, tempVal, tempVal);
                            
                            if(!((j-1) & 0x1))
                                pSrc--;
                            pDest--;
                        }
                    }
                }
                break;
                
                case 8:
                {
                    BYTE *pSrc;
                    WORD *pDest;
                    
                    rowBytes = pstPngInfo->u16Width + 1;
                    for(i = pstPngInfo->u16Height; i > 0; i--)
                    {
                        pSrc = (BYTE*)(u32DataAddress + rowBytes * i - 1);
                        pDest = (WORD*)(u32DataAddress + pstPngInfo->u16Width * 2 * i - 2);
                        for(j = pstPngInfo->u16Width; j > 0; j--)
                        {
                            *pDest = PNG_888_TO_565((*pSrc), (*pSrc), (*pSrc));
                            
                            pSrc--;
                            pDest--;
                        }
                    }
                }
                break;
                
                case 16:
                {
                    BYTE *pSrc;
                    WORD *pDest;
                    BYTE tempVal;
                    
                    rowBytes = pstPngInfo->u16Width * 2 + 1;
                    for(i = pstPngInfo->u16Height; i > 0; i--)
                    {
                        pSrc = (BYTE*)(u32DataAddress + rowBytes * i - 1);
                        pDest = (WORD*)(u32DataAddress + pstPngInfo->u16Width * 2 * (i-1) + 
                                    (pstPngInfo->u16Width-1) * 2);        
                        for(j = pstPngInfo->u16Width; j > 0; j--)
                        {
                            tempVal = (PNG_GET_2_BYTE(pSrc) >> 8) & 0xff;
                            *pDest = PNG_888_TO_565(tempVal, tempVal, tempVal);
                            
                            pSrc -= 2;
                            pDest--;
                        }
                    }
                }
                break;
            }
        }
        break;
        
        case PNG_COLOR_TYPE_TRUE_COLOR:
        {
            if(pstPngInfo->u08Depth == 8)
            {
                BYTE *pSrc;
                WORD *pDest;
                
                rowBytes = 3 * pstPngInfo->u16Width + 1;
                for(i = 0; i < pstPngInfo->u16Height; i++)
                {
                    pSrc = (BYTE*)u32DataAddress + rowBytes * i + 1;
                    pDest = (WORD*)(u32DataAddress + pstPngInfo->u16Width * 2 * i);
                    for(j = 0; j < pstPngInfo->u16Width; j++)
                    {
                        *pDest = PNG_888_TO_565(pSrc[0], pSrc[1], pSrc[2]);
                        
                        pDest++;
                        pSrc += 3;
                    }
                }
            }
            else if(pstPngInfo->u08Depth == 16)
            {
                BYTE *pSrc; 
                WORD *pDest;
                WORD r, g, b;
                
                rowBytes = 6 * pstPngInfo->u16Width + 1;
                for(i = 0; i < pstPngInfo->u16Height; i++)
                {
                    pSrc = (BYTE*)(u32DataAddress + rowBytes * i + 1);
                    pDest = (WORD*)(u32DataAddress + pstPngInfo->u16Width * 2 * i);
                    for(j = 0; j < pstPngInfo->u16Width; j++)
                    {
                        r = PNG_GET_2_BYTE(pSrc);
                        g = PNG_GET_2_BYTE(pSrc+2);
                        b = PNG_GET_2_BYTE(pSrc+4);
                        *pDest = PNG_888_TO_565(((r >> 8) & 0xff), ((g >> 8) & 0xff), ((b >> 8) & 0xff));
                        
                        pDest++;
                        pSrc += 6;
                    }
                }
            }
        }
        break;
        
        case PNG_COLOR_TYPE_INDEX_COLOR:
        {
            switch(pstPngInfo->u08Depth)
            {
                case 1:
                    
                break;
                
                case 2:
                break;
                
                case 4:
                {
                    BYTE *pSrc;
                    WORD *pDest;
                    BYTE *palette;
                    BYTE index;
                    
                    rowBytes = ((pstPngInfo->u16Width + 1) >> 1) + 1;
                    for(i = pstPngInfo->u16Height; i > 0; i--)
                    {
                        pSrc = (BYTE*)(u32DataAddress + rowBytes * i - 1);
                        pDest = (WORD*)(u32DataAddress + pstPngInfo->u16Width * 2 * i - 2);
                        for(j = pstPngInfo->u16Width; j > 0; j--)
                        {
                            if((j-1) & 0x1)
                                index = (*pSrc) & 0xf;
                            else
                                index = ((*pSrc) >> 4) & 0xf;
                                
                            palette = (BYTE*)(pstPngInfo->u32PaletteAddr + 3 * index);
                            *pDest = PNG_888_TO_565(palette[0], palette[1], palette[2]);
                            
                            if(!((j-1) & 0x1))
                                pSrc--;
                            pDest--;
                        }
                    }
                }
                break;
                
                case 8:
                {
                    BYTE *pSrc;
                    WORD *pDest;
                    BYTE *palette;
                    
                    rowBytes = pstPngInfo->u16Width + 1;
                    for(i = pstPngInfo->u16Height; i > 0; i--)
                    {
                        pSrc = (BYTE*)(u32DataAddress + rowBytes * (i-1) + (pstPngInfo->u16Width - 1) + 1);
                        pDest = (WORD*)(u32DataAddress + pstPngInfo->u16Width * 2 * (i-1) + 
                                    (pstPngInfo->u16Width-1) * 2);
                        for(j = pstPngInfo->u16Width; j > 0; j--)
                        {
                            palette = (BYTE*)(pstPngInfo->u32PaletteAddr + 3 * (*pSrc));
                            *pDest = PNG_888_TO_565(palette[0], palette[1], palette[2]);
                            
                            pSrc--;
                            pDest--;
                        }
                    }
                }
                break;
            }
        }
        break;
        
        case PNG_COLOR_TYPE_GRAY_SCALE_ALPHA:
        {
            if(pstPngInfo->u08Depth == 8)
            {
                BYTE *pSrc;
                WORD *pDest;
                BYTE bg = 255;
                BYTE alpha, gray;
                BYTE tempVal;
                
                if(pstPngInfo->u32PngFlag & PNG_FLAG_bKGD_AVAIL)
                    bg = pstPngInfo->u16BGColor[0] & 0xff;
                
                rowBytes = 2 * pstPngInfo->u16Width + 1;
                for(i = 0 ; i < pstPngInfo->u16Height; i++)
                {
                    pSrc = (BYTE*)(u32DataAddress + rowBytes * i + 1);
                    pDest = (WORD*)(u32DataAddress + pstPngInfo->u16Width * 2 * i);
                    for(j = 0; j < pstPngInfo->u16Width; j++)
                    {
                        gray = pSrc[0];
                        alpha = pSrc[1];
                        
                        if(alpha == 0)
                        {
                            *pDest = 0xffff;
                        }
                        else if(pSrc[1] == 255)
                        {
                            *pDest = PNG_888_TO_565(gray, gray, gray);
                        }
                        else
                        {
                            tempVal = (gray * alpha + bg * (256 - alpha)) >> 8;
                            *pDest = PNG_888_TO_565(tempVal, tempVal, tempVal);
                        }
                        
                        pSrc += 2;
                        pDest++;
                    }
                }
            }
            else if(pstPngInfo->u08Depth == 16)
            {
                BYTE *pSrc;
                WORD *pDest;
                WORD bg = 65535;
                WORD alpha, gray;
                WORD tempVal;
                
                if(pstPngInfo->u32PngFlag & PNG_FLAG_bKGD_AVAIL)
                    bg = pstPngInfo->u16BGColor[0];
                
                rowBytes = 4 * pstPngInfo->u16Width + 1;
                for(i = 0 ; i < pstPngInfo->u16Height; i++)
                {
                    pSrc = (BYTE*)(u32DataAddress + rowBytes * i + 1);
                    pDest = (WORD*)(u32DataAddress + pstPngInfo->u16Width * 2 * i);
                    for(j = 0; j < pstPngInfo->u16Width; j++)
                    {
                        gray = PNG_GET_2_BYTE(pSrc);
                        alpha = PNG_GET_2_BYTE(pSrc+2);
                        
                        if(alpha == 0)
                        {
                            *pDest = 0xffff;
                        }
                        else if(alpha == 65535)
                        {
                            tempVal = (gray >> 8) & 0xff;
                            *pDest = PNG_888_TO_565(gray, gray, gray);
                        }
                        else
                        {
                            tempVal = (((gray * alpha + bg * (65536 - alpha)) >> 16) >> 8) & 0xff;
                            *pDest = PNG_888_TO_565(gray, gray, gray);
                        }
                        
                        pSrc += 4;
                        pDest++;
                    }
                }
            }
        }
        break;
        
        case PNG_COLOR_TYPE_TRUE_COLOR_ALPHA:
        {
            if(pstPngInfo->u08Depth == 8)
            {
                BYTE *pSrc;
                WORD *pDest;
                BYTE bgR = 255, bgG = 255, bgB = 255;
                BYTE r, g, b;
                
                if(pstPngInfo->u32PngFlag & PNG_FLAG_bKGD_AVAIL)
                {
                    bgR = pstPngInfo->u16BGColor[0] & 0xff;
                    bgG = pstPngInfo->u16BGColor[1] & 0xff;
                    bgB = pstPngInfo->u16BGColor[2] & 0xff;
                }
                
                rowBytes = 4 * pstPngInfo->u16Width + 1;
                for(i = 0; i < pstPngInfo->u16Height; i++)
                {
                    pSrc = (BYTE*)u32DataAddress + rowBytes * i + 1;
                    pDest = (WORD*)(u32DataAddress + pstPngInfo->u16Width * 2 * i);
                    for(j = 0; j < pstPngInfo->u16Width; j++)
                    {
                        if(pSrc[3] == 0)
                        {
                            *pDest = PNG_888_TO_565(bgR, bgG, bgB);
                        }
                        else if(pSrc[3] == 255)
                        {
                            *pDest = PNG_888_TO_565(pSrc[0], pSrc[1], pSrc[2]);
                        }
                        else
                        {
                            r = (pSrc[0] * pSrc[3] + bgR * (256 - pSrc[3])) >> 8;
                            g = (pSrc[1] * pSrc[3] + bgG * (256 - pSrc[3])) >> 8;
                            b = (pSrc[2] * pSrc[3] + bgB * (256 - pSrc[3])) >> 8;
                            *pDest = PNG_888_TO_565(r, g, b);
                        }
                        pDest++;
                        pSrc += 4;
                    }
                }
            }
            else if(pstPngInfo->u08Depth == 16)
            {
                BYTE *pSrc; 
                WORD *pDest;
                WORD bgR = 65535, bgG = 65535, bgB = 65535;
                WORD r, g, b, alpha;
                
                if(pstPngInfo->u32PngFlag & PNG_FLAG_bKGD_AVAIL)
                {
                    bgR = pstPngInfo->u16BGColor[0];
                    bgG = pstPngInfo->u16BGColor[1];
                    bgB = pstPngInfo->u16BGColor[2];
                }
                
                rowBytes = 8 * pstPngInfo->u16Width + 1;
                for(i = 0; i < pstPngInfo->u16Height; i++)
                {
                    pSrc = (BYTE*)(u32DataAddress + rowBytes * i + 1);
                    pDest = (WORD*)(u32DataAddress + pstPngInfo->u16Width * 2 * i);
                    for(j = 0; j < pstPngInfo->u16Width; j++)
                    {
                        alpha = PNG_GET_2_BYTE(pSrc+6);
                        if(alpha == 0)
                        {
                            r = (bgR >> 8) & 0xff;
                            g = (bgG >> 8) & 0xff;
                            b = (bgB >> 8) & 0xff;
                        }
                        else if(alpha == 65535)
                        {
                            r = PNG_GET_2_BYTE(pSrc);
                            g = PNG_GET_2_BYTE(pSrc+2);
                            b = PNG_GET_2_BYTE(pSrc+4);
                        }
                        else
                        {
                            r = PNG_GET_2_BYTE(pSrc);
                            g = PNG_GET_2_BYTE(pSrc+2);
                            b = PNG_GET_2_BYTE(pSrc+4);
                            r = (((r * alpha + bgR * (65536 - alpha)) >> 16) >> 8) & 0xff;
                            g = (((g * alpha + bgG * (65536 - alpha)) >> 16) >> 8) & 0xff;
                            b = (((b * alpha + bgB * (65536 - alpha)) >> 16) >> 8) & 0xff;
                        }
                        *pDest = PNG_888_TO_565(r, g, b);
                        
                        pDest++;
                        pSrc += 8;
                    }
                }
            }
        }
        break;
    }
}
//-------------------------------------------------------------------------------------
static void pngOutputRGB565Deinterlace(ST_PNG_INFO* pstPngInfo, DWORD u32DataAddress)
{
    DWORD i, j;
    DWORD rowBytes;
    DWORD u32SrcAddress = pstPngInfo->u32DeinterlaceBuffer;
    BYTE pass = 0;
    
    for(i = 0; i < 7; i++)
    {
        pngUnfilter(pstPngInfo->stFakeHeader[i].u16Width, pstPngInfo->stFakeHeader[i].u16Height,
                    pstPngInfo->u08ColorType, pstPngInfo->u08Depth, 
                    u32SrcAddress, pstPngInfo->stFakeHeader[i].u32RawDataLength);
        u32SrcAddress += pstPngInfo->stFakeHeader[i].u32RawDataLength;
    }
    
    u32SrcAddress = pstPngInfo->u32DeinterlaceBuffer;
    
    switch(pstPngInfo->u08ColorType)
    {
        case PNG_COLOR_TYPE_GRAY_SCALE:
        {
            switch(pstPngInfo->u08Depth)
            {
                case 1:
                break;
                
                case 2:
                break;
                
                case 4:
                {
                }
                break;
                
                case 8:
                {
                    BYTE *pSrc;
                    WORD *pDest;
                
                    for(pass = 0; pass < 7; pass++)
                    {
                        rowBytes = pstPngInfo->stFakeHeader[pass].u16Width + 1;
                        for(i = 0; i < pstPngInfo->stFakeHeader[pass].u16Height; i++)
                        {
                            pSrc = (BYTE*)u32SrcAddress + rowBytes * i + 1;
                            pDest = (WORD*)(u32DataAddress + 
                                    (i * DEINTERLACE_ARRAY[pass][0] + DEINTERLACE_ARRAY[pass][1]) * 3 * pstPngInfo->u16Width +
                                    3 * DEINTERLACE_ARRAY[pass][3]);
                            for(j = 0; j < pstPngInfo->stFakeHeader[pass].u16Width; j++)
                            {
                               *pDest = PNG_888_TO_565(*pSrc, *pSrc, *pSrc);
                                
                                pSrc++;
                                pDest += (DEINTERLACE_ARRAY[pass][2] * 2);
                            }
                        }
                        u32SrcAddress += pstPngInfo->stFakeHeader[pass].u32RawDataLength;
                    }
                }
                break;
                
                case 16:
                {
                    BYTE *pSrc;
                    WORD *pDest;
                    WORD tempVal;
                
                    for(pass = 0; pass < 7; pass++)
                    {
                        rowBytes = pstPngInfo->stFakeHeader[pass].u16Width * 2 + 1;
                        for(i = 0; i < pstPngInfo->stFakeHeader[pass].u16Height; i++)
                        {
                            pSrc = (BYTE*)u32SrcAddress + rowBytes * i + 1;
                            pDest = (WORD*)(u32DataAddress + 
                                    (i * DEINTERLACE_ARRAY[pass][0] + DEINTERLACE_ARRAY[pass][1]) * 3 * pstPngInfo->u16Width +
                                    3 * DEINTERLACE_ARRAY[pass][3]);
                            for(j = 0; j < pstPngInfo->stFakeHeader[pass].u16Width; j++)
                            {
                                tempVal = (PNG_GET_2_BYTE(pSrc) >> 8) & 0xff;
                                *pDest = PNG_888_TO_565(tempVal, tempVal, tempVal);
                                
                                pSrc += 2;
                                pDest += (DEINTERLACE_ARRAY[pass][2] * 2);
                            }
                        }
                        u32SrcAddress += pstPngInfo->stFakeHeader[pass].u32RawDataLength;
                    }
                }
                break;
            }
        }
        break;
        
        case PNG_COLOR_TYPE_TRUE_COLOR:
        {
            if(pstPngInfo->u08Depth == 8)
            {
                BYTE *pSrc;
                WORD *pDest;
                
                for(pass = 0; pass < 7; pass++)
                {
                    rowBytes = 3 * pstPngInfo->stFakeHeader[pass].u16Width + 1;
                    for(i = 0; i < pstPngInfo->stFakeHeader[pass].u16Height; i++)
                    {
                        pSrc = (BYTE*)u32SrcAddress + rowBytes * i + 1;
                        pDest = (WORD*)(u32DataAddress + 
                                (i * DEINTERLACE_ARRAY[pass][0] + DEINTERLACE_ARRAY[pass][1]) * 3 * pstPngInfo->u16Width +
                                3 * DEINTERLACE_ARRAY[pass][3]);
                        for(j = 0; j < pstPngInfo->stFakeHeader[pass].u16Width; j++)
                        {
                            *pDest = PNG_888_TO_565(pSrc[0], pSrc[1], pSrc[2]);
                            
                            pSrc += 3;
                            pDest += (DEINTERLACE_ARRAY[pass][2] * 2);
                        }
                    }
                    u32SrcAddress += pstPngInfo->stFakeHeader[pass].u32RawDataLength;
                }
            }
            else if(pstPngInfo->u08Depth == 16)
            {
                BYTE *pSrc;
                WORD *pDest;
                BYTE r, g, b;
                
                for(pass = 0; pass < 7; pass++)
                {
                    rowBytes = 6 * pstPngInfo->stFakeHeader[pass].u16Width + 1;
                    for(i = 0; i < pstPngInfo->stFakeHeader[pass].u16Height; i++)
                    {
                        pSrc = (BYTE*)u32SrcAddress + rowBytes * i + 1;
                        pDest = (WORD*)(u32DataAddress + 
                                (i * DEINTERLACE_ARRAY[pass][0] + DEINTERLACE_ARRAY[pass][1]) * 3 * pstPngInfo->u16Width +
                                3 * DEINTERLACE_ARRAY[pass][3]);
                        for(j = 0; j < pstPngInfo->stFakeHeader[pass].u16Width; j++)
                        {
                            r = (PNG_GET_2_BYTE(pSrc) >> 8) & 0xff;
                            g = (PNG_GET_2_BYTE(pSrc+2) >> 8) & 0xff;
                            b = (PNG_GET_2_BYTE(pSrc+4) >> 8) & 0xff;
                            
                            *pDest = PNG_888_TO_565(r, g, b);
                            
                            pSrc += 6;
                            pDest += (DEINTERLACE_ARRAY[pass][2] * 2);
                        }
                    }
                    u32SrcAddress += pstPngInfo->stFakeHeader[pass].u32RawDataLength;
                }
            }
        }
        break;
        
        case PNG_COLOR_TYPE_INDEX_COLOR:
        {
            switch(pstPngInfo->u08Depth)
            {
                case 8:
                {
                    BYTE *pSrc;
                    WORD *pDest;
                    BYTE *palette;
                    
                    for(pass = 0; pass < 7; pass++)
                    {
                        rowBytes = pstPngInfo->stFakeHeader[pass].u16Width + 1;
                        for(i = 0; i < pstPngInfo->stFakeHeader[pass].u16Height; i++)
                        {
                            pSrc = (BYTE*)u32SrcAddress + rowBytes * i + 1;
                            pDest = (WORD*)(u32DataAddress + 
                                    (i * DEINTERLACE_ARRAY[pass][0] + DEINTERLACE_ARRAY[pass][1]) * 3 * pstPngInfo->u16Width +
                                    3 * DEINTERLACE_ARRAY[pass][3]);
                            for(j = 0; j < pstPngInfo->stFakeHeader[pass].u16Width; j++)
                            {
                                palette = (BYTE*)(pstPngInfo->u32PaletteAddr + 3 * (*pSrc));
                                *pDest = PNG_888_TO_565(palette[0], palette[1], palette[2]);
                                    
                                pSrc++;
                                pDest += (DEINTERLACE_ARRAY[pass][2] * 2);
                            }
                        }
                        u32SrcAddress += pstPngInfo->stFakeHeader[pass].u32RawDataLength;
                    }
                }
                break;
            }
        }
        break;
        
        case PNG_COLOR_TYPE_GRAY_SCALE_ALPHA:
        {
            if(pstPngInfo->u08Depth == 8)
            {
                BYTE *pSrc;
                WORD *pDest;
                BYTE bg = 255;
                BYTE alpha, gray;
                
                if(pstPngInfo->u32PngFlag & PNG_FLAG_bKGD_AVAIL)
                    bg = pstPngInfo->u16BGColor[0] & 0xff;
                    
                for(pass = 0; pass < 7; pass++)
                {
                    rowBytes = 2 * pstPngInfo->stFakeHeader[pass].u16Width + 1;
                    for(i = 0; i < pstPngInfo->stFakeHeader[pass].u16Height; i++)
                    {
                        pSrc = (BYTE*)u32SrcAddress + rowBytes * i + 1;
                        pDest = (WORD*)(u32DataAddress + 
                                (i * DEINTERLACE_ARRAY[pass][0] + DEINTERLACE_ARRAY[pass][1]) * 3 * pstPngInfo->u16Width +
                                3 * DEINTERLACE_ARRAY[pass][3]);
                        for(j = 0; j < pstPngInfo->stFakeHeader[pass].u16Width; j++)
                        {
                            gray = pSrc[0];
                            alpha = pSrc[1];
                        
                            if(alpha == 0)
                            {
                                *pDest = PNG_888_TO_565(bg, bg, bg);
                            }
                            else if(alpha == 255)
                            {
                                *pDest = PNG_888_TO_565(gray, gray, gray);
                            }
                            else
                            {
                                gray = (gray * alpha + bg * (256 - alpha)) >> 8;
                                *pDest = PNG_888_TO_565(gray, gray, gray);
                            }
                            
                            pSrc += 2;
                            pDest += (DEINTERLACE_ARRAY[pass][2] * 2);
                        }
                    }
                    u32SrcAddress += pstPngInfo->stFakeHeader[pass].u32RawDataLength;
                }
            }
            else if(pstPngInfo->u08Depth == 16)
            {
                BYTE *pSrc;
                WORD *pDest;
                WORD bg = 65535;
                WORD alpha, gray;
                BYTE tempVal;
                
                if(pstPngInfo->u32PngFlag & PNG_FLAG_bKGD_AVAIL)
                    bg = pstPngInfo->u16BGColor[0];
                    
                for(pass = 0; pass < 7; pass++)
                {
                    rowBytes = 4 * pstPngInfo->stFakeHeader[pass].u16Width + 1;
                    for(i = 0; i < pstPngInfo->stFakeHeader[pass].u16Height; i++)
                    {
                        pSrc = (BYTE*)u32SrcAddress + rowBytes * i + 1;
                        pDest = (WORD*)(u32DataAddress + 
                                (i * DEINTERLACE_ARRAY[pass][0] + DEINTERLACE_ARRAY[pass][1]) * 3 * pstPngInfo->u16Width +
                                3 * DEINTERLACE_ARRAY[pass][3]);
                        for(j = 0; j < pstPngInfo->stFakeHeader[pass].u16Width; j++)
                        {
                            gray = PNG_GET_2_BYTE(pSrc);
                            alpha = PNG_GET_2_BYTE(pSrc+2);
                        
                            if(alpha == 0)
                            {
                                tempVal = (bg >> 8) & 0xff;
                            }
                            else if(alpha == 65535)
                            {
                                tempVal = (gray >> 8) & 0xff;
                            }
                            else
                            {
                                tempVal = (((gray * alpha + bg * (65536 - alpha)) >> 16) >> 8) & 0xff;
                            }
                            
                            *pDest = PNG_888_TO_565(tempVal, tempVal, tempVal);
                            
                            pSrc += 4;
                            pDest += (DEINTERLACE_ARRAY[pass][2] * 2);
                        }
                    }
                    u32SrcAddress += pstPngInfo->stFakeHeader[pass].u32RawDataLength;
                }
            }
        }
        break;
        
        case PNG_COLOR_TYPE_TRUE_COLOR_ALPHA:
        {
            if(pstPngInfo->u08Depth == 8)
            {
                BYTE *pSrc;
                WORD *pDest;
                BYTE bgR = 255, bgG = 255, bgB = 255;
                
                if(pstPngInfo->u32PngFlag & PNG_FLAG_bKGD_AVAIL)
                {
                    bgR = pstPngInfo->u16BGColor[0] & 0xff;
                    bgG = pstPngInfo->u16BGColor[1] & 0xff;
                    bgB = pstPngInfo->u16BGColor[2] & 0xff;
                }
                
                for(pass = 0; pass < 7; pass++)
                {
                    rowBytes = 4 * pstPngInfo->stFakeHeader[pass].u16Width + 1;
                    for(i = 0; i < pstPngInfo->stFakeHeader[pass].u16Height; i++)
                    {
                        pSrc = (BYTE*)u32SrcAddress + rowBytes * i + 1;
                        pDest = (WORD*)(u32DataAddress + 
                                (i * DEINTERLACE_ARRAY[pass][0] + DEINTERLACE_ARRAY[pass][1]) * 3 * pstPngInfo->u16Width +
                                3 * DEINTERLACE_ARRAY[pass][3]);
                        for(j = 0; j < pstPngInfo->stFakeHeader[pass].u16Width; j++)
                        {
                            if(pSrc[3] == 0)
                            {
                                PNG_888_TO_565(bgR, bgG, bgB);
                            }
                            else if(pSrc[3] == 255)
                            {
                                PNG_888_TO_565(pSrc[0], pSrc[1], pSrc[2]);
                            }
                            else
                            {
                                bgR = (pSrc[0] * pSrc[3] + bgR * (256 - pSrc[3])) >> 8;
                                bgG = (pSrc[1] * pSrc[3] + bgG * (256 - pSrc[3])) >> 8;
                                bgB = (pSrc[2] * pSrc[3] + bgB * (256 - pSrc[3])) >> 8;
                                PNG_888_TO_565(bgR, bgG, bgB);
                            }
                            
                            pSrc += 4;
                            pDest += (DEINTERLACE_ARRAY[pass][2] * 2);
                        }
                    }
                    u32SrcAddress += pstPngInfo->stFakeHeader[pass].u32RawDataLength;
                }
            }
            else if(pstPngInfo->u08Depth == 16)
            {
                BYTE *pSrc; 
                WORD *pDest;
                WORD bgR = 65535, bgG = 65535, bgB = 65535;
                WORD r, g, b, alpha;
                
                if(pstPngInfo->u32PngFlag & PNG_FLAG_bKGD_AVAIL)
                {
                    bgR = pstPngInfo->u16BGColor[0];
                    bgG = pstPngInfo->u16BGColor[1];
                    bgB = pstPngInfo->u16BGColor[2];
                }
                
                for(pass = 0; pass < 7; pass++)
                {
                    rowBytes = 8 * pstPngInfo->stFakeHeader[pass].u16Width + 1;
                    for(i = 0; i < pstPngInfo->stFakeHeader[pass].u16Height; i++)
                    {
                        pSrc = (BYTE*)u32SrcAddress + rowBytes * i + 1;
                        pDest = (WORD*)(u32DataAddress + 
                                (i * DEINTERLACE_ARRAY[pass][0] + DEINTERLACE_ARRAY[pass][1]) * 3 * pstPngInfo->u16Width +
                                3 * DEINTERLACE_ARRAY[pass][3]);
                        for(j = 0; j < pstPngInfo->stFakeHeader[pass].u16Width; j++)
                        {
                            alpha = PNG_GET_2_BYTE(pSrc+6);
                            if(alpha == 0)
                            {
                                r = (bgR >> 8) & 0xff;
                                g = (bgG >> 8) & 0xff;
                                b = (bgB >> 8) & 0xff;
                            }
                            else if(alpha == 65535)
                            {
                                r = PNG_GET_2_BYTE(pSrc);
                                g = PNG_GET_2_BYTE(pSrc+2);
                                b = PNG_GET_2_BYTE(pSrc+4);
                            }
                            else
                            {
                                r = PNG_GET_2_BYTE(pSrc);
                                g = PNG_GET_2_BYTE(pSrc+2);
                                b = PNG_GET_2_BYTE(pSrc+4);
                                r = (((r * alpha + bgR * (65536 - alpha)) >> 16) >> 8) & 0xff;
                                g = (((g * alpha + bgG * (65536 - alpha)) >> 16) >> 8) & 0xff;
                                b = (((b * alpha + bgB * (65536 - alpha)) >> 16) >> 8) & 0xff;
                            }
                            PNG_888_TO_565(r, g, b);
                            
                            pSrc += 8;
                            pDest += (DEINTERLACE_ARRAY[pass][2] * 2);
                        }
                    }
                    u32SrcAddress += pstPngInfo->stFakeHeader[pass].u32RawDataLength;
                }
            }
        }
        break;
    }
}
//-------------------------------------------------------------------------------------
DWORD Inflate(BYTE* pu08CompressData, DWORD u32DataLength, BYTE* pstOutput, DWORD outputSize)
{
    SDWORD bfinal, btype; /* as defined by the deflate spec */
    SDWORD code, length, distance, i;
    DWORD unpacked_index = 0;
    
    HuffmanTreeCreate(&stMainTree);
    HuffmanTreeCreate(&stDistancesTree);
    HuffmanTreeCreate(&stHelperTree);
    
    InitInputData((DWORD)pu08CompressData+2, u32DataLength-2); /* ignore zlib header, 2bytes */

    do
    {
        bfinal = InputReadBit();
        if(stInputData.s16Error)
        {
            //PNG_DEBUG_STR("Data shorter than one bit!");
            return 0;
        }
        
        btype = InputReadBits(2);
        if(stInputData.s16Error)
        {
            //PNG_DEBUG_STR("Data shorter than one byte!");
            return 0;
        }
        
        switch(btype)
        {
            case 0:
                length = InputReadBytes(2);
                if(stInputData.s16Error)
                {
                    //PNG_DEBUG_STR("Cannot read length of uncompressed block!");
                    return 0;
                }
                i = InputReadBytes(2);
                if(stInputData.s16Error)
                {
                    //PNG_DEBUG_STR("Cannot read length of uncompressed block!");
                    return 0;
                }
                if(length != (i ^ 0xFFFF))
                {
                    //PNG_DEBUG_STR("Uncompressed block corruption detected!");
                    return 0;
                }
                
                for(i = 0; i < length; i++)
                {
                    code = InputReadByte();
                    if(stInputData.s16Error)
                    {
                        //PNG_DEBUG_STR("Uncompressed block incomplete!");
                        return 0;
                    }
                    if(unpacked_index >= outputSize)
                    {
                        //PNG_DEBUG_STR("Too much data!");
                        return 0;
                    }
                    else
                    {
                        pstOutput[unpacked_index++] = code;
                    }
                }
            break;
            
            case 1:
                if(!load_fixed_tree(&stMainTree, &stDistancesTree))
                {
                    //PNG_DEBUG_STR("Cannot load fixed Huffman tree!");
                    return 0;
                }
            break;
            
            case 2:
                if(!load_dynamic_tree(&stMainTree, &stDistancesTree, &stHelperTree))
                {
                    //PNG_DEBUG_STR("Cannot load dynamic Huffman tree!");
                    return 0;
                }
            break;
            
            case 3:
                //PNG_DEBUG_STR("Unsupported (non-existing) type of compression!");
                return 0;
            break;
        }
        
        if(btype > 0)
        {
            code = HuffmanTreeReadNextCode(&stMainTree);
            if(code == -1)
            {
                //PNG_DEBUG_STR("Invalid data!");
                return 0;
            }
            
            while(code != 256)
            {
                if(code < 256)
                {
                    if(unpacked_index >= outputSize)
                    {
                        //PNG_DEBUG_STR("Too much data!");
                        return 0;
                    }
                    
                    pstOutput[unpacked_index++] = code;
                }
                else
                {
                    length = 0;
                    distance = 0;
                    if(code >= 257 && code <= 264)
                    {
                        length = code - 257 + 3;
                    }
                    else if(code >= 265 && code <= 268)
                    {
                        length = (code - 265) * 2 + 11 + InputReadBit();
                    }
                    else if(code >= 269 && code <= 272)
                    {
                        length = (code - 269) * 4 + 19 + InputReadBits(2);
                    }
                    else if(code >= 273 && code <= 276)
                    {
                        length = (code - 273) * 8 + 35 + InputReadBits(3);
                    }
                    else if(code >= 277 && code <= 280)
                    {
                        length = (code - 277) * 16 + 67 + InputReadBits(4);
                    }
                    else if(code >= 281 && code <= 284)
                    {
                        length = (code - 281) * 32 + 131 + InputReadBits(5);
                    }
                    else if(code == 285)
                    {
                        length = 258;
                    }
                    else
                    {
                        //PNG_DEBUG_STR("Algorithm error: unknown length!");
                        return 0;
                    }
                    
                    if(stInputData.s16Error)
                    {
                        //PNG_DEBUG_STR("Cannot read length!");
                        return 0;
                    }

                    distance = get_distance(&stDistancesTree);
                    if(distance == -1)
                    {
                        //PNG_DEBUG_STR("Algorithm error: bad distance!");
                        return 0;
                    }

                    for(i = 0; i < length; i++)
                    {
                        code= pstOutput[unpacked_index - distance];
                        if(unpacked_index >= outputSize)
                        {
                            //PNG_DEBUG_STR("Too much data!");
                            return 0;
                        }

                        pstOutput[unpacked_index++] = code;
                    }
                }
                code = HuffmanTreeReadNextCode(&stMainTree);
                if(code == -1)
                {
                    //PNG_DEBUG_STR("Invalid data!");
                    return 0;
                }
            }
        }
    } while (bfinal == 0);
    
    return unpacked_index;
}
//-------------------------------------------------------------------------------------
static void InitInputData(DWORD u32DataAddress, DWORD u32DataLength)
{
    stInputData.u32Length = u32DataLength;
    stInputData.pu08Data = (BYTE*)u32DataAddress;
    stInputData.s16Error = 0;
    stInputData.u32Index = 0;
    stInputData.s32SubIndex = 8;
}
//-------------------------------------------------------------------------------------
static SDWORD InputReadBit()
{
    SDWORD res;
    if(stInputData.s32SubIndex > 7)
    {
        if(stInputData.u32Index == stInputData.u32Length)
        {
            stInputData.s16Error = 1;
            return -1;
        }
        stInputData.u08Current = stInputData.pu08Data[stInputData.u32Index++];
        stInputData.s32SubIndex = 0;
    }
    res = (stInputData.u08Current >> stInputData.s32SubIndex) & 1;
    stInputData.s32SubIndex++;
    return res;
}
//-------------------------------------------------------------------------------------
static SDWORD InputReadBits(SDWORD count)
{
    SDWORD i, res = 0;
    for (i = 0; i < count; i++)
    {
        res += (InputReadBit() << i);
        if(stInputData.s16Error)
        {
            return -1;
        }
    }
    return res;
}
//-------------------------------------------------------------------------------------
static BYTE InputReadByte()
{
    if(stInputData.u32Index == stInputData.u32Length)
    {
        stInputData.s16Error = 1;
        return -1;
    }
    
    if(stInputData.s32SubIndex != 8)
        stInputData.s32SubIndex = 8;
    
    stInputData.u08Current = stInputData.pu08Data[stInputData.u32Index++];
    return stInputData.u08Current;
}
//-------------------------------------------------------------------------------------
static DWORD InputReadBytes(SDWORD count)
{
    SDWORD i;
    DWORD res = 0;
    for (i = 0; i < count; i++)
    {
        res += (InputReadByte() << (i * 8));
        if(stInputData.s16Error)
        {
            return -1;
        }
    }
    return res;
}
//-------------------------------------------------------------------------------------
static void HuffmanTreeReset(ST_HUFFMAN_TREE* pstTree)
{
    SDWORD i = 1;
    while (pstTree->ps32Data[1] != HUFFMAN_EMPTY_NODE)
    {
        if(i * 2 > HUFFMAN_HEAP_MAX)
        {
            pstTree->ps32Data[i] = HUFFMAN_EMPTY_NODE;
            i /= 2;
        }
        else if(pstTree->ps32Data[i * 2] != HUFFMAN_EMPTY_NODE)
        {
            i = i * 2;
        }
        else if(pstTree->ps32Data[i * 2 + 1] != HUFFMAN_EMPTY_NODE)
        {
            i = i * 2 + 1;
        }
        else
        {
            pstTree->ps32Data[i] = HUFFMAN_EMPTY_NODE;
            i /= 2;
        }
    }
    pstTree->s32LastFreeParent = 1;
    pstTree->s32LastDepth = 0;
    pstTree->ps32Data[1] = HUFFMAN_NONLEAF_NODE;
}
//-------------------------------------------------------------------------------------
static void HuffmanTreeCreate(ST_HUFFMAN_TREE* pstTree)
{
    if(pstTree)
    {
        if(pstTree->ps32Data)
        {
            DWORD i = 0;
            for(i = 0; i < HUFFMAN_HEAP_SIZE; i++)
                pstTree->ps32Data[i] = HUFFMAN_EMPTY_NODE;
        }
    }
}
//-------------------------------------------------------------------------------------
static SDWORD HuffmanTreeDestroy(ST_HUFFMAN_TREE* pstTree)
{
    return 0;
}
//-------------------------------------------------------------------------------------
static SDWORD HuffmanTreeAdd(ST_HUFFMAN_TREE* pstTree, SDWORD depth, SDWORD value)
{
    SDWORD i;
    if(depth > 15 || depth <= pstTree->s32LastDepth)
    {
        return 0;
    }
    
    while(1)
    {
        i = 2 * pstTree->s32LastFreeParent;
        if(i > HUFFMAN_HEAP_MAX)
        {
            return 0;
        }
        
        if(pstTree->s32LastDepth == depth - 1)
        {
            if(pstTree->ps32Data[i] == HUFFMAN_EMPTY_NODE)
            {
                pstTree->ps32Data[i] = value;
            }
            else
            {
                pstTree->ps32Data[i + 1] = value;
                while (pstTree->s32LastFreeParent != 1 && pstTree->ps32Data[2 * pstTree->s32LastFreeParent + 1] != HUFFMAN_EMPTY_NODE)
                {
                    pstTree->ps32Data[pstTree->s32LastFreeParent] = HUFFMAN_NONLEAF_NODE;
                    pstTree->s32LastFreeParent /= 2;
                    pstTree->s32LastDepth--;
                }
            }
            return 1;
        }
        else
        {
            pstTree->s32LastDepth++;
            if(pstTree->ps32Data[i] == HUFFMAN_EMPTY_NODE)
            {
                pstTree->s32LastFreeParent = i;
            }
            else
            {
                pstTree->s32LastFreeParent = i + 1;
            }
        }
    }
}
//-------------------------------------------------------------------------------------
static SDWORD HuffmanTreeReadNextCode(ST_HUFFMAN_TREE* pstTree)
{
    SDWORD i = 1;
    SDWORD bit;

    while(pstTree->ps32Data[i] < 0)
    {
        bit = InputReadBit();
        if(stInputData.s16Error)
        {
            return HUFFMAN_EMPTY_NODE;
        }
        
        switch(bit)
        {
            case 0:
                i = i * 2;
            break;
            
            case 1:
                i = i * 2 + 1;
            break;
        }
        
        if(i > HUFFMAN_HEAP_MAX)
        {
            return -1;
        }
    }

    return pstTree->ps32Data[i];
}
//-------------------------------------------------------------------------------------
static SDWORD get_distance(ST_HUFFMAN_TREE* pstDistanceTree)
{
    SDWORD distance;
    SDWORD i;
    
    i = HuffmanTreeReadNextCode(pstDistanceTree);
    if(i == -1)
    {
        return -1;
    }
    if(i >= (SDWORD)(sizeof(distances) / sizeof(SDWORD)))
    {
        return -1;
    }
    distance = distances[i];
    if(extra_bits[i] > 0)
    {
        distance += InputReadBits(extra_bits[i]);
        if(stInputData.s16Error)
        {
            return -1;
        }
    }
    if(distance < 1 || distance > 32768)
    {
        return -1;
    }
    return distance;
}
//-------------------------------------------------------------------------------------
static SDWORD load_fixed_tree(ST_HUFFMAN_TREE* pstMainTree, ST_HUFFMAN_TREE* pstDistancesTree)
{
    SDWORD i;
    HuffmanTreeReset(pstMainTree);
    for(i = 256; i <= 279; i++)
    {
        if(!HuffmanTreeAdd(pstMainTree, 7, i))
        {
            return 0;
        }
    }
    for(i = 0; i <= 143; i++)
    {
        if(!HuffmanTreeAdd(pstMainTree, 8, i))
        {
            return 0;
        }
    }
    for(i = 280; i <= 287; i++)
    {
        if(!HuffmanTreeAdd(pstMainTree, 8, i))
        {
            return 0;
        }
    }
    for(i = 144; i <= 255; i++)
    {
        if(!HuffmanTreeAdd(pstMainTree, 9, i))
        {
            return 0;
        }
    }
    HuffmanTreeReset(pstDistancesTree);
    for(i = 0; i < 32; i++)
    {
        if(!HuffmanTreeAdd(pstDistancesTree, 5, i))
        {
            return 0;
        }
    }
    return 1;
}
//-------------------------------------------------------------------------------------
static SDWORD load_dynamic_tree(ST_HUFFMAN_TREE* pstMainTree, ST_HUFFMAN_TREE* pstDistancesTree, ST_HUFFMAN_TREE* pstHelperTree)
{
    SDWORD hlit, hdist, hclen; /* see deflate spec for details */
    SDWORD code_length_codes[19];
    SDWORD literal_length_codes[286];
    SDWORD distance_codes[32];
    SDWORD i, j, k, l, m;  /* counters */
    SDWORD code_lengths_order[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

    hlit = InputReadBits(5) + 257;
    if(stInputData.s16Error)
    {
        return 0;
    }
    hdist = InputReadBits(5) + 1;
    if(stInputData.s16Error)
    {
        return 0;
    }
    hclen = InputReadBits(4) + 4;
    if(stInputData.s16Error)
    {
        return 0;
    }
    
    for(i = 0; i < 19; i++)
        code_length_codes[i] = 0;
    
    for(i = 0; i < hclen; i++)
    {
        code_length_codes[code_lengths_order[i]] = InputReadBits(3);
        if(stInputData.s16Error)
        {
            return 0;
        }
    }

    HuffmanTreeReset(pstHelperTree);
    for(i = 1; i < 2*2*2; i++)
    {
        for(j = 0; j < 19; j++)
        {
            if(code_length_codes[j] == i)
            {
                if(!HuffmanTreeAdd(pstHelperTree, i, j))
                {
                    return 0;
                }
            }
        }
    }
    
    for(i = 0; i < 286; i++)
        literal_length_codes[i] = 0;
    
    i = 0;
    while(i < hlit)
    {
        j = HuffmanTreeReadNextCode(pstHelperTree);
        if(j == -1)
        {
            return 0;
        }
        if(j <= 15)
        {
            literal_length_codes[i++] = j;
        }
        else
        {
            switch (j)
            {
                case 16:
                {
                    k = InputReadBits(2) + 3;
                    if(stInputData.s16Error)
                    {
                        return 0;
                    }
                    l = literal_length_codes[i - 1];
                    for(m = 0; m < k; m++)
                    {
                        literal_length_codes[i++] = l;
                    }
                }
                break;
                case 17:
                {
                    k = InputReadBits(3) + 3;
                    if(stInputData.s16Error)
                    {
                        return 0;
                    }
                    for(m = 0; m < k; m++)
                    {
                        literal_length_codes[i++] = 0;
                    }
                }
                break;
                case 18: 
                {
                    k = InputReadBits(7) + 11;
                    if(stInputData.s16Error)
                    {
                        return 0;
                    }
                    for(m = 0; m < k; m++)
                    {
                        literal_length_codes[i++] = 0;
                    }
                }
                break;
            }
        }
    }
    HuffmanTreeReset(pstMainTree);
    for(i = 1; i <= 15; i++)
    {
        for(j = 0; j < 286; j++)
        {
            if(literal_length_codes[j] == i)
            {
                if(!HuffmanTreeAdd(pstMainTree, i, j))
                {
                    return 0;
                }
            }
        }
    }
    
    for(i = 0; i < 32; i++)
        distance_codes[i] = 0;
    
    i = 0;
    while(i < hdist)
    {
        j = HuffmanTreeReadNextCode(pstHelperTree);
        if(j == -1)
        {
            return 0;
        }
        if(j <= 15)
        {
            distance_codes[i++] = j;
        }
        else
        {
            switch (j)
            {
                case 16:
                {
                    k = InputReadBits(2) + 3;
                    if(stInputData.s16Error)
                    {
                        return 0;
                    }
                    l = distance_codes[i - 1];
                    for(m = 0; m < k; m++)
                    {
                        distance_codes[i++] = l;
                    }
                }
                break;
                case 17:
                {
                    k = InputReadBits(3) + 3;
                    if(stInputData.s16Error)
                    {
                        return 0;
                    }
                    for(m = 0; m < k; m++)
                    {
                        distance_codes[i++] = 0;
                    }
                }
                break;
                case 18:
                {
                    k = InputReadBits(7) + 11;
                    if(stInputData.s16Error)
                    {
                        return 0;
                    }
                    for(m = 0; m < k; m++)
                    {
                        distance_codes[i++] = 0;
                    }
                }
                break;
            }
        }
    }
    HuffmanTreeReset(pstDistancesTree);
    for(i = 1; i <= 15; i++)
    {
        for(j = 0; j < 32; j++)
        {
            if(distance_codes[j] == i)
            {
                if(!HuffmanTreeAdd(pstDistancesTree, i, j))
                {
                    return 0;
                }
            }
        }
    }

    return 1;
}
//-------------------------------------------------------------------------------------
void SetHuffmanTreeBuffer(DWORD u32MainTreeBuffer, DWORD u32DistancesTreeBuffer, DWORD u32HelperTreeBuffer)
{
    stMainTree.ps32Data = (SDWORD*)u32MainTreeBuffer;
    stDistancesTree.ps32Data = (SDWORD*)u32DistancesTreeBuffer;
    stHelperTree.ps32Data = (SDWORD*)u32HelperTreeBuffer;
}
//======================================= Function implementation ==========================================//
SDWORD PngDecodeImage(ST_PNG_INFO* pstPngInfo, DWORD u32InBufferAddr, DWORD u32InBufferSize, BYTE u08InReqDataFormat, 
                    WORD* pu16OutWidth, WORD* pu16OutHeight, BOOL* pbOutIsDs)
{
    SDWORD retCode = PNG_NO_ERROR;
    BOOL finish = FALSE;
    BYTE tempBuf[4];
    DWORD chunkLength, chunkType;
    DWORD backupPos;
    BYTE* ptr;
    DWORD outputDataSize = 0;

    MP_DEBUG("PngDecodeImage");
    if(!pngCallback.mAllocate || !pngCallback.mFree || !pngCallback.refillBuffer || !pngCallback.seekSet)
        return PNG_MODULE_NOT_INITIALIZE;
    
    pstPngInfo->u32CompressDataBuffer = 0;
    pstPngInfo->u32CompressDataBuffer = (DWORD)PNG_COMPRESS_DATA_ADD;//(DWORD)pngCallback.mAllocate(pstPngInfo->u32CompressDataLength);
    if(!pstPngInfo->u32CompressDataBuffer)
        return PNG_MEMORY_NOT_ENOUGH;
    
    pngCallback.seekSet(pstPngInfo->u32FirstIDATPos);
    
    ptr = (BYTE*)pstPngInfo->u32CompressDataBuffer;
    
    pngCallback.seekSet(pstPngInfo->u32FirstIDATPos);
    while(!finish)
    {
        backupPos = pngCallback.getPos();
        
        pngCallback.refillBuffer((DWORD)tempBuf, 4);
        chunkLength = PNG_GET_4_BYTE(tempBuf);
        pngCallback.refillBuffer((DWORD)tempBuf, 4);
        chunkType = PNG_GET_4_BYTE(tempBuf);
        
        switch(chunkType)
        {
            case PNG_TAG_IMAGE_DATA:
                pngCallback.refillBuffer((DWORD)ptr, chunkLength);
                ptr += chunkLength;
            break;
            
            case PNG_TAG_IMAGE_TRAILER:
                finish = TRUE;
            break;
        }
        
        pngCallback.seekSet(backupPos + 8 + chunkLength + 4);
    }
    
    if(retCode == PNG_NO_ERROR)
    {
        DWORD actualOutput = 0;
        
        outputDataSize = pngCalcOutputDataSize(pstPngInfo);
        
        if(pstPngInfo->u08Interlace == PNG_INTERLACE_ADM7)
        {
            pstPngInfo->u32DeinterlaceBuffer = 0;
            pstPngInfo->u32DeinterlaceBuffer = (DWORD)PNG_DEINTERLACE_ADD;//pngCallback.mAllocate(PngEvaluateImageBufferSize(pstPngInfo, u08InReqDataFormat));
            if(!pstPngInfo->u32DeinterlaceBuffer)
                retCode = PNG_MEMORY_NOT_ENOUGH;
            else
            {
                actualOutput = Inflate((BYTE*)pstPngInfo->u32CompressDataBuffer, pstPngInfo->u32CompressDataLength,
                                        (BYTE*)pstPngInfo->u32DeinterlaceBuffer, outputDataSize);
            }
        }
        else
        {
            actualOutput = Inflate((BYTE*)pstPngInfo->u32CompressDataBuffer, pstPngInfo->u32CompressDataLength,
                                    (BYTE*)u32InBufferAddr, outputDataSize);
                                    
            if(outputDataSize != actualOutput)
                retCode = PNG_STREAM_ERROR;
            else
            {
                pngUnfilter(pstPngInfo->u16Width, pstPngInfo->u16Height, pstPngInfo->u08ColorType,
                            pstPngInfo->u08Depth, u32InBufferAddr, outputDataSize);
            }
        }
        
        if(retCode == PNG_NO_ERROR)
        {
            if(u08InReqDataFormat == PNG_OUTPUT_RGB888)
            {
                if(pstPngInfo->u08Interlace == PNG_INTERLACE_NONE)
                    pngOutputRGB888(pstPngInfo, u32InBufferAddr);
                else if(pstPngInfo->u08Interlace == PNG_INTERLACE_ADM7)
                    pngOutputRGB888Deinterlace(pstPngInfo, u32InBufferAddr);
            }
            else if(u08InReqDataFormat == PNG_OUTPUT_RGB565)
            {
                if(pstPngInfo->u08Interlace == PNG_INTERLACE_NONE)
                    pngOutputRGB565(pstPngInfo, u32InBufferAddr);
                else if(pstPngInfo->u08Interlace == PNG_INTERLACE_ADM7)
                    pngOutputRGB565Deinterlace(pstPngInfo, u32InBufferAddr);
            }
            
            *pu16OutWidth = pstPngInfo->u16Width;
            *pu16OutHeight = pstPngInfo->u16Height;
        }
    }

    if (pbOutIsDs)
        *pbOutIsDs = FALSE; //always false, png does not support down sample now
    
//    pngCallback.mFree(pstPngInfo->u32CompressDataBuffer);
    pstPngInfo->u32CompressDataBuffer = 0;
    
    if(pstPngInfo->u32DeinterlaceBuffer)
    {
//        pngCallback.mFree(pstPngInfo->u32DeinterlaceBuffer);
        pstPngInfo->u32DeinterlaceBuffer = 0;
    }
    
    return retCode;
}
//-------------------------------------------------------------------------------
//decode
int Png_Decode_Bits(IMAGEFILE *psImage, BYTE *bpSource, DWORD dwSrcBufferSize)//, STREAM *aHandle)
{
    SDWORD iErrorCode = PASS;
    ST_PNG_INFO pngInfo;
    BYTE *bporgStream;
    BYTE * bpStream;
    DWORD dwData, dwTempData;
    BYTE Yo, Y1, Cb, Cr; 
		DWORD BufSizeTemp;

    MP_DEBUG("Png_Decode_Bits");
    gPNGBufferStart = gPNGBufferPtr = bpSource;
    gPNGBufferEnd = gPNGBufferStart + dwSrcBufferSize;

    MP_DEBUG("ext_mem_get_free_space() =%d", ext_mem_get_free_space());
		BufSizeTemp=psImage->wImageWidth*psImage->wImageHeight*2+ PNG_PALETTE_SIZE+ 
			PNG_HUFFMAN_MAIN_TREE_SIZE + PNG_HUFFMAN_DIST_TREE_SIZE + PNG_HUFFMAN_HELP_TREE_SIZE + 
			PNG_COMPRESS_DATA_SIZE ;
    //if(ext_mem_get_free_space() < PNG_DECODE_BUFFER_SIZE)//Jasmine 070903
    if(PNG_DECODE_BUFFER_SIZE<BufSizeTemp)//TYChen 20100715
    {
        return ERR_MEM_MALLOC;
    }   

    pbPngDecodeBuffer = (BYTE *)ext_mem_malloc(PNG_DECODE_BUFFER_SIZE);//allocate free space

    if (pbPngDecodeBuffer == NULL) return ERR_MEM_MALLOC;        
            
    PngModuleInit(mallocCallback, mFreeCallback, fillBufferCallback, seekSetCallback, getPosCallback);
    iErrorCode = PngCheckHeader(&pngInfo);

    if(iErrorCode)
    {
        if(pbPngDecodeBuffer != NULL) ext_mem_free(pbPngDecodeBuffer);//free memory
        return iErrorCode;
    }   
    
    if(PASS == iErrorCode)
    {
        DWORD rawSize;// = pngInfo.u16Width * pngInfo.u16Height * 3;
        DWORD rawBuffer;// = (DWORD)malloc(rawSize);
        WORD width, height;
        BYTE outFileName[256];
        BOOL isDs;
        DWORD i, j;
        BYTE offset;
        
        rawSize = PngEvaluateImageBufferSize(&pngInfo, PNG_OUTPUT_RGB888);        
        //rawBuffer = ((DWORD)psImage->pbTarget) + 0x10000; //Jamsine 070906
	rawBuffer = (BYTE *)ext_mem_malloc(ALIGN_16(psImage->wImageWidth) * ALIGN_16(psImage->wImageHeight) * 4 + 2048);

        MP_DEBUG("rawSize=%d, psImage->dwTargetSize=%d", rawSize, psImage->dwTargetSize);    
        
        if((rawSize > MAX_PNG_RESOLUTION) || rawSize > psImage->dwTargetSize)//Jasmine 070903
        {
            if(pbPngDecodeBuffer != NULL) ext_mem_free(pbPngDecodeBuffer);//free memory
            if(rawBuffer != NULL) ext_mem_free(rawBuffer);
            return ERR_MEM_MALLOC;
        }

        iErrorCode =  PngDecodeImage(&pngInfo, rawBuffer, rawSize, PNG_OUTPUT_RGB888, &width, &height, &isDs);     

        if(pbPngDecodeBuffer != NULL) ext_mem_free(pbPngDecodeBuffer);//free memory        
        if(iErrorCode) 
        {
					if(rawBuffer != NULL) ext_mem_free(rawBuffer);        
					return iErrorCode;
				}


        //align width to 16
		bporgStream = (BYTE *)rawBuffer;
		bpStream = (BYTE *)psImage->pbTarget;      
        
		if(psImage->wTargetWidth != psImage->wImageWidth)
		{
			for(i = 0; i < psImage->wImageHeight; i++)
			{
				for(j = 0; j < psImage->wImageWidth * 3; j++)
				{   
					*bpStream = *bporgStream; 
					bpStream ++;
					bporgStream ++;
				}

    			for(j = psImage->wImageWidth; j < psImage->wTargetWidth; j++)
           			bpStream += 3;
			}
            bporgStream = bpStream = (BYTE *)psImage->pbTarget; 
		}
	      
        //RGB8882YUV
		for(i = 0; i < (psImage->wTargetWidth * psImage->wTargetHeight >> 1); i ++)
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

		if((psImage->wTargetWidth * psImage->wTargetHeight % 2) != 0)
		{
			dwData = RGB2YUV(*bporgStream, *(bporgStream +1) , *(bporgStream + 2));
			*bpStream = Yo = dwData >> 24;
			*(bpStream + 1) = Y1 = dwData >> 24;
			*(bpStream + 2) = Cb = (dwData & 0xff00) >> 8;
			*(bpStream + 3) = Cr = dwData & 0xff;
		}		
		if(rawBuffer != NULL)
		ext_mem_free(rawBuffer);


		
    }
    
    MP_DEBUG("\r\n Png_Decode_Bits End\r\n");
    return iErrorCode;
}
//-------------------------------------------------------------------------------
int Png_Decoder_DecodeImage(IMAGEFILE *psImage)
{
	MP_DEBUG("Png_Decoder_DecodeImage");
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
		return PNG_STREAM_ERROR;
	}
    
	dwReadSize = mpxStreamRead(bpSource, 1, dwSrcBufferSize, psStream); //read all image   
    
    if(iErrorCode == PASS)
        iErrorCode = Png_Decode_Bits(psImage, bpSource, dwSrcBufferSize);

    if(bpSource !=NULL) 
        ext_mem_free((BYTE *)((DWORD)bpSource & ~0x20000000));

	return iErrorCode;
}
//-------------------------------------------------------------------------------
DWORD Png_GetImageSize(IMAGEFILE *psImage)
{   
	if (psImage == NULL) return 0;
	if (psImage->psStream == NULL) return 0;
	
	ST_MPX_STREAM *psStream = psImage->psStream;
	psImage->wImageWidth = 0;
	psImage->wImageHeight = 0;
	MP_DEBUG("Png_GetImageSize"); 
    
	mpxStreamSeek(psStream, 0, SEEK_SET);
	//type "PN", P=0x89, N=0x50
	if (mpxStreamReadWord(psStream) != IMAGE_TAG_PNG)
		return PNG_ILLEGAL_FILE;
    
	//image header
	mpxStreamSeek(psStream, 18, SEEK_SET);

	psImage->wImageWidth = mpxStreamReadWord(psStream);    

	mpxStreamSeek(psStream, 22, SEEK_SET);
   
	psImage->wImageHeight = mpxStreamReadWord(psStream);		  

	psImage->wTargetWidth = ALIGN_16(psImage->wImageWidth);
	psImage->wTargetHeight = psImage->wImageHeight;
    
   	psImage->wRealTargetWidth = psImage->wImageWidth;    
	psImage->wRealTargetHeight = psImage->wImageHeight;    

    MP_DEBUG("w %d, h %d", psImage->wImageWidth, psImage->wImageHeight);
	return (psImage->wImageWidth << 16) | psImage->wImageHeight;  
}
//-------------------------------------------------------------------------------
int Png_Decoder_Init(IMAGEFILE *psImage)
{
	MP_ASSERT(psImage != NULL);
	
	MP_DEBUG("\r\nPng_Decoder_Init");
	psImage->ImageDecodeThumb = Png_Decoder_DecodeImage;
	psImage->ImageDecodeImage = Png_Decoder_DecodeImage;
			
	if (Png_GetImageSize(psImage) != 0)
		return PASS;
	else
		return PNG_NOT_SUPPORTED_SIZE;
}


int Png_Decode_From_Buffer(BYTE *pbSource, DWORD dwSourceSize, ST_IMGWIN * pTrgWin)
{
	ST_MPX_STREAM sStream, g_sPNGStream;
	ST_IMGWIN decode_win ;
	IMAGEFILE sImage;
	DWORD dwSizeLimit, dwSrcSize;
	int iRet, iErrorCode ;
	
	//sStream = &g_sPNGStream ;
	iRet = mpxStreamOpenBuffer(&sStream, pbSource, dwSourceSize);
	
	sImage.psStream = &sStream ;
	
	if(Png_GetImageSize(&sImage) != 0)
	{
		MP_DEBUG("PNG width = %d", sImage.wImageWidth);
	}
	else
	{
		mpDebugPrint("PNG_NOT_SUPPORTED_SIZE");
		return PNG_NOT_SUPPORTED_SIZE;
	}
	dwSizeLimit = ALIGN_16(sImage.wImageWidth) * sImage.wImageHeight * 4 + 0x10000;
	sImage.dwTargetSize = dwSizeLimit ;
	sImage.pbTarget = (BYTE *)ext_mem_malloc(dwSizeLimit);
	MP_DEBUG("PNG target mem alloc = %d", dwSizeLimit);
	
	if(sImage.pbTarget == NULL)
	{
		mpDebugPrint("PNG malloc fail");
		return FAIL;
	}
	
	iErrorCode = Png_Decode_Bits(&sImage, pbSource, dwSourceSize);
	
	pTrgWin->wWidth = sImage.wTargetWidth ;
 	pTrgWin->wHeight = sImage.wTargetHeight ;
 	pTrgWin->pdwStart = sImage.pbTarget ;
  pTrgWin->dwOffset = (pTrgWin->wWidth<<1) ;  
	
	if(iErrorCode)
	{
		mpDebugPrint("PNG decode fail");
		return FAIL ;
	}	
	else
		return 0 ;
	
}
#endif

