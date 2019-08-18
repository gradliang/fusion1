#ifndef __PNG_H
#define __PNG_H

//#include "typedef.h"

#define PNG_DEBUG_STR   printf

#define PNG_FIX_BUFFER_SIZE                 (256 * 3)

#define PNG_FOUR_CHAR_INT(a, b, c, d)       ((a << 24) | (b << 16) | (c << 8) | d)
#define PNG_888_TO_565(r, g, b)             (((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3))

//tag defination
#define PNG_TAG_IMAGE_HEADER                PNG_FOUR_CHAR_INT('I','H','D','R')
#define PNG_TAG_PALETTE                     PNG_FOUR_CHAR_INT('P','L','T','E')
#define PNG_TAG_IMAGE_DATA                  PNG_FOUR_CHAR_INT('I','D','A','T')
#define PNG_TAG_IMAGE_TRAILER               PNG_FOUR_CHAR_INT('I','E','N','D')
#define PNG_TAG_BACKGROUND_COLOR            PNG_FOUR_CHAR_INT('b','K','G','D')
#define PNG_TAG_PRIMARY_CHROMA              PNG_FOUR_CHAR_INT('c','H','R','M')
#define PNG_TAG_IMAGE_GAMA                  PNG_FOUR_CHAR_INT('g','A','M','A')
#define PNG_TAG_IMAGE_HISTOGRAM             PNG_FOUR_CHAR_INT('h','I','S','T')
#define PNG_TAG_PHYSICAL_PIXEL              PNG_FOUR_CHAR_INT('p','H','Y','s')
#define PNG_TAG_SIGNIFICANT_BITS            PNG_FOUR_CHAR_INT('s','B','I','T')
#define PNG_TAG_TEXTUAL_DATA                PNG_FOUR_CHAR_INT('t','E','X','t')
#define PNG_TAG_LAST_MODIFY_TIME            PNG_FOUR_CHAR_INT('t','I','M','E')
#define PNG_TAG_TRANSPARENCY                PNG_FOUR_CHAR_INT('t','R','N','S')
#define PNG_TAG_COMPRESS_TEXTUAL            PNG_FOUR_CHAR_INT('z','T','X','t')


//tag data length
#define PNG_IHDR_LEN                        13
#define PNG_PLTE_MAX_SIZE                   (256 * 3)
#define PNG_cHRM_LEN                        32
#define PNG_gAMA_LEN                        4


//tEXt and zTXt key word
#define PNG_TXT_TITLE                       "Title"
#define PNG_TXT_AUTHOR                      "Author"
#define PNG_TXT_DESCRIPTION                 "Description"
#define PNG_TXT_COPYRIGHT                   "Copyright"
#define PNG_TXT_CREATION_TIME               "Creation Time"
#define PNG_TXT_SOFTWARE                    "Software"
#define PNG_TXT_DISCLAIMER                  "Disclaimer"
#define PNG_TXT_WARNING                     "Warning"
#define PNG_TXT_SOURCE                      "Source"
#define PNG_TXT_COMMENT                     "Comment"


//filter definition
#define PNG_FILTER_NONE                     0
#define PNG_FILTER_SUB                      1
#define PNG_FILTER_UP                       2
#define PNG_FILTER_AVERAGE                  3
#define PNG_FILTER_PAETH                    4


//png output data format
#define PNG_OUTPUT_RGB888                   0
#define PNG_OUTPUT_RGB565                   1


//png color type
#define PNG_COLOR_TYPE_GRAY_SCALE           0
#define PNG_COLOR_TYPE_TRUE_COLOR           2
#define PNG_COLOR_TYPE_INDEX_COLOR          3
#define PNG_COLOR_TYPE_GRAY_SCALE_ALPHA     4
#define PNG_COLOR_TYPE_TRUE_COLOR_ALPHA     6


//interlace method
#define PNG_INTERLACE_NONE                  0
#define PNG_INTERLACE_ADM7                  1


//png return code
#define PNG_NO_ERROR                        0
#define PNG_ILLEGAL_FILE                    1
#define PNG_UNSURPPORTED                    2
#define PNG_ERROR_PARAMETER                 3
#define PNG_MODULE_NOT_INITIALIZE           4
#define PNG_MEMORY_NOT_ENOUGH               5
#define PNG_UNSUPPORT_DATA_FORMAT           6
#define PNG_STREAM_ERROR                    7


#define PNG_MAX_U32                         ((DWORD)0x7fffffffL)


//png info flag
#define PNG_FLAG_IHDR_AVAIL                 0x00000001
#define PNG_FLAG_PLTE_AVAIL                 0x00000002
#define PNG_FLAG_IDAT_AVAIL                 0x00000004
#define PNG_FLAG_bKGD_AVAIL                 0x00000008
#define PNG_FLAG_gAMA_AVAIL                 0x00000010
#define PNG_FLAG_tRNS_AVAIL                 0x00000020
#define PNG_FLAG_cHRM_AVAIL                 0x00000040
#define PNG_FLAG_CRC_TABLE_CREATED          0x80000000


#define PNG_GET_4_BYTE(p)   ((*((BYTE*)p) << 24) | (*((BYTE*)(p+1)) << 16) | (*((BYTE*)(p+2)) << 8) | (*((BYTE*)(p+3))))
#define PNG_GET_2_BYTE(p)   (((*(BYTE*)p) << 8) | (*(BYTE*)(p+1)))


typedef struct
{
    DWORD             u32WPointX;
    DWORD             u32WPointY;
    DWORD             u32RedX;
    DWORD             u32RedY;
    DWORD             u32GreenX;
    DWORD             u32GreenY;
    DWORD             u32BlueX;
    DWORD             u32BlueY;
} ST_PNG_CHROMA;

typedef struct
{
    WORD             u16Width;
    WORD             u16Height;
    DWORD             u32RawDataLength;
} ST_FAKE_HEADER;

typedef struct
{
    WORD             u16Width;
    WORD             u16Height;
    BYTE             u08Depth;
    BYTE             u08ColorType;
    BYTE             u08Compression;
    BYTE             u08Filter;
    BYTE             u08Interlace;
    DWORD             u32PngFlag;
    DWORD             u32FirstIDATPos;
    DWORD             u32MaxIDATSize;
    DWORD             u32PaletteAddr;
    WORD             u16PaletteNum;
    WORD             u16BGColor[3];
    ST_PNG_CHROMA   stChromaInfo;
    DWORD             u32Gamma;
    WORD             u16TransColor[3];
    DWORD             u32CompressDataLength;
    DWORD             u32CompressDataBuffer;
    DWORD             u32FilterBuffer;
    ST_FAKE_HEADER  stFakeHeader[7];
    DWORD             u32DeinterlaceBuffer;
} ST_PNG_INFO;


typedef void* (*PNG_MALLOC_CALLBACK)(DWORD);
typedef void (*PNG_MFREE_CALLBACK)(DWORD);
typedef SDWORD (*PNG_FILL_BUFFER_CALLBACK)(DWORD, DWORD);
typedef SDWORD (*PNG_SEEK_SET_CALLBACK)(SDWORD);
typedef SDWORD (*PNG_GET_POS_CALLBACK)(DWORD *);


typedef struct
{
    PNG_MALLOC_CALLBACK         mAllocate;
    PNG_MFREE_CALLBACK          mFree;
    PNG_FILL_BUFFER_CALLBACK    refillBuffer;
    PNG_SEEK_SET_CALLBACK       seekSet;
    PNG_GET_POS_CALLBACK        getPos;
} ST_PNG_CALLBACK;


SDWORD PngModuleInit(PNG_MALLOC_CALLBACK mallocCallback,
                    PNG_MFREE_CALLBACK mFreeCallback,
                    PNG_FILL_BUFFER_CALLBACK fillBufferCallback,
                    PNG_SEEK_SET_CALLBACK seekSetCallback,
                    PNG_GET_POS_CALLBACK getPosCallback);
                    
SDWORD PngModuleRelease();

SDWORD PngCheckHeader(ST_PNG_INFO* pstPngInfo);

DWORD PngEvaluateImageBufferSize(ST_PNG_INFO* pstPngInfo, BYTE u08InReqDataFormat);

SDWORD PngDecodeImage(ST_PNG_INFO* pstPngInfo, DWORD u32InBufferAddr, DWORD u32InBufferSize, BYTE u08InReqDataFormat, 
                    WORD* pu16OutWidth, WORD* pu16OutHeight, BOOL* pbOutIsDs);
                    

SDWORD pngProcessIHDR(ST_PNG_INFO* pstPngInfo);
SDWORD pngProcessPLTE(ST_PNG_INFO* pstPngInfo, DWORD paletteLen);
SDWORD pngProcessbKGD(ST_PNG_INFO* pstPngInfo);
SDWORD pngProcesscHRM(ST_PNG_INFO* pstPngInfo);
SDWORD pngProcessgAMA(ST_PNG_INFO* pstPngInfo);
SDWORD pngProcesstRNS(ST_PNG_INFO* pstPngInfo);
DWORD pngCalcFilterDelta(BYTE, BYTE);
BYTE paethPredictor(BYTE ua, BYTE ub, BYTE uc);
void CRCTableCreate();
DWORD CrcCalculate(DWORD startPos, DWORD len);
DWORD pngCalcOutputDataSize(ST_PNG_INFO* pstPngInfo);

#define HUFFMAN_HEAP_MAX            65535
#define HUFFMAN_HEAP_SIZE           (HUFFMAN_HEAP_MAX + 1)
#define HUFFMAN_EMPTY_NODE          -1
#define HUFFMAN_NONLEAF_NODE        -2
#define HUFFMAN_TREE_DATA_SIZE      (HUFFMAN_HEAP_SIZE * 4)	//65535 * 4 = 0x3fffc

void SetHuffmanTreeBuffer(DWORD u32MainTreeBuffer, DWORD u32DistancesTreeBuffer, DWORD u32HelperTreeBuffer);

DWORD Img_PNGGetSize(BYTE * bpSource);

SWORD Img_PNGDecodeThumb(BYTE * bpSource, BYTE * bpTarget, DWORD dwSrcBufferSize);

SWORD Img_PNGDecode(BYTE * bpSource, BYTE * bpTarget, BYTE yield,DWORD dwSrcBufferSize);

#endif
