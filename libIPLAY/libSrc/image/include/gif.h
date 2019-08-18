///
///@ingroup		Image
///@defgroup	GIF	GIF
///
///	The APIs for GIF file decoding
///
///	This file includes all the necessary interfaces to decode a GIF file
///
#ifndef __gif_h
#define __gif_h

#include "iplaysysconfig.h"

#if 0
#define GIF_NECESSARY_BUFFER_SIZE   (768 * 2 + 0xc000 + 256)
#define GIF_SIGNATURE           "GIF"
#endif

#define GIF_VERSION_87A         0
#define GIF_VERSION_89A         1

#define GIF_HEADER_SIZE         13
#define GIF_COLOR_TABLE_SIZE    (256*3)
#define GIF_IMAGE_DESC_SIZE     9

#define GIF_MASK_GLOBAL_COLOR_TABLE         0x80
#define GIF_MASK_COLOR_RESOLUTION           0x70
#define GIF_MASK_SORT_FLAG                  0x08
#define GIF_MASK_GLOBAL_COLOR_TABLE_SIZE    0x07

#define GIF_MASK_LOCAL_COLOR_TABLE          0x80
#define GIF_MASK_INTERLACE                  0x40
#define GIF_MASK_LOCAL_SORT                 0x20
#define GIF_MASK_LOCAL_COLOR_TABLE_SIZE     0x07

#define GIF_MASK_CONTROL_DISPOSAL           0x1c
#define GIF_MASK_CONTROL_USER_INPUT         0x02
#define GIF_MASK_CONTROL_TRANSPARENT        0x01

#define GIF_NO_DISPOSAL     0
#define GIF_DO_NOT_DISPOSE  1
#define GIF_RESTORE_TO_BG   2
#define GIF_RESTORE_TO_PREV 3

#define GIF_NO_ERROR                        0
#define GIF_ILLEGAL_FILE                    -1
#define GIF_UNSURPPORTED                    -2
#define GIF_ERROR_PARAMETER                 -3
#define GIF_MODULE_NOT_INITIALIZE           -4
#define GIF_MEMORY_NOT_ENOUGH               -5
#define GIF_GET_STREAM_FAIL                 -6
#define GIF_ERROR_FRAME_IDX                 -7
#define GIF_NO_AVAIL_FRAME                  -8
#define GIF_STREAM_ERROR                    -9
#define GIF_UNKNOWN_ERROR                   -10
#define GIF_NOT_SUPPORTED_SIZE              -11

//gif output data format
#define GIF_OUTPUT_RGB888           0
#define GIF_OUTPUT_RGB565           1

typedef struct
{
    BOOL bGlobalColorTable;
    BOOL bSort;
    BYTE bRev[2];

    BYTE u08Version;
    BYTE u08ColorResolution;
    BYTE u08BgColorIdx;        //background color index
    BYTE u08PixelAR;           //pixel aspect ratio

    WORD u16ScreenWidth;       //logical screen width
    WORD u16ScreenHeight;      //logical screen height
    WORD u16GlobalColorTableSize;
} ST_GIF_SCREEN_DESC;

typedef struct
{
    BOOL bLocalColorTable;
    BOOL bInterlace;
    BOOL bSort;
    BYTE bRev;

    WORD u16ImageLeftPos;
    WORD u16ImageTopPos;
    WORD u16ImageWidth;
    WORD u16ImageHeight;
    WORD u16LocalColorTableSize;
} ST_GIF_IMAGE_DESC;

typedef struct
{
    BYTE u08DisposalMethod;
    BOOL bUserInput;
    BOOL bTransparentColor;
    BYTE u08TransparentColorIdx;
    WORD u16DelayTime;
} ST_GIF_GRAPHIC_CONTROL;

typedef struct
{
    BYTE u08GifVersion;      //version info
    BYTE bRev[3];

    WORD u16Width;           //screen width
    WORD u16Height;          //screen height
    WORD u16FrameCount;      //total frame number
    WORD u16DelayTime;       //frame delay, 1/100 sec
} ST_GIF_INFO;

typedef struct
{
    BYTE u08DownSample;
    BYTE bRev[3];

    WORD u16HPos;
    WORD u16VPos;
    WORD u16Width;
    WORD u16Height;
    WORD prevIdx;
    WORD u16OutWidth;
    WORD u16OutHeight;
    DWORD u32PaletteAddr;
    DWORD prevPos;
} ST_GIF_IMAGE_DECODE_INFO;

//for lzw decode
typedef struct
{
    SDWORD prefix;
    SDWORD c;
    SDWORD len;
}ST_LZW_CODES;

typedef void* (*GIF_MALLOC_CALLBACK)(DWORD);
typedef void (*GIF_MFREE_CALLBACK)(DWORD);
typedef SDWORD (*GIF_FILL_BUFFER_CALLBACK)(DWORD, DWORD);
typedef SDWORD (*GIF_SEEK_SET_CALLBACK)(DWORD);
typedef DWORD (*GIF_GET_POS_CALLBACK)(void);

typedef struct
{
    GIF_MALLOC_CALLBACK         mAllocate;
    GIF_MFREE_CALLBACK          mFree;
    GIF_FILL_BUFFER_CALLBACK    refillBuffer;
    GIF_SEEK_SET_CALLBACK       seekSet;
    GIF_GET_POS_CALLBACK        getPos;
} ST_GIF_CALLBACK;

///
///@ingroup	GIF
///@brief	Init the GIF module
///
///@param	allocCallback	        Callback API for buffer allcation
///@param	freeCallback	        Callback API for buffer release
///@param	fillBufCallback	        Callback API for fill GIF data to buffer
///@param	seekSetCallback	        Callback API for seek the file position of a GIF file from the beginning
///
///@retval  GIF_NO_ERROR            Module is initialized successfully
///@retval  GIF_ERROR_PARAMETER     The caller did not pass the callback APIs
///
SDWORD GifModuleInit(GIF_MALLOC_CALLBACK allocCallback,
                    GIF_MFREE_CALLBACK freeCallback,
                    GIF_FILL_BUFFER_CALLBACK fillBufCallback,
                    GIF_SEEK_SET_CALLBACK seekSetCallback,
                    GIF_GET_POS_CALLBACK getPosCallback);


///
///@ingroup	GIF
///@brief	Release the GIF module
///
///@retval  GIF_NO_ERROR            Module is released successfully
///
SDWORD GifModuleRelease();


///
///@ingroup	GIF
///@brief	Decode the GIF image data
///
///@param	pstOutInfo	                The output ST_GIF_INFO structure.
///
///@retval  GIF_NO_ERROR                The GIF info is decoded successfully.
///@retval  GIF_MODULE_NOT_INITIALIZE   The caller did not init the module first.
///@retval  GIF_ERROR_PARAMETER         The caller did not pass the pointer to a ST_BMP.
///@retval  GIF_MEMORY_NOT_ENOUGH       GIF allocate buffer fail.
///@retval  GIF_ILLEGAL_FILE            It's not a GIF file.
///@retval  GIF_UNSURPPORTED            Unsupported BMP file format.
///@retval  GIF_GET_STREAM_FAIL         GIF get file stream fail.
///
SDWORD GifGetGifInfo(ST_GIF_INFO* pstOutInfo);


///
///@ingroup	GIF
///@brief	Decode the GIF image data by frame index
///
///@param	inFrameIdx	                The frame index to decode.
///@param   u32BufferAddr               The buffer address to store ouput image data.
///@param   u32BufferSize               The buffer size of the output image buffer.
///@param   reqDataFormat               The request output data format, only RGB888 and RGB565 are supported.
///@param   pu16ImageWidth              The ouput image width.
///@param   pu16ImageHeight             The output image height.
///@param   pu16DelayTime               The delay time between current frame and next frame.
///
///@retval  GIF_NO_ERROR                The BMP is decoded successfully.
///@retval  GIF_MODULE_NOT_INITIALIZE   The caller did not init the module first.
///@retval  GIF_ERROR_PARAMETER         The caller did not pass the pointer to a ST_BMP.
///@retval  GIF_MEMORY_NOT_ENOUGH       BMP allocate buffer fail.
///@retval  GIF_ILLEGAL_FILE            It's not a BMP file.
///@retval  GIF_UNSURPPORTED            Unsupported BMP file format.
///
SDWORD GifDecodeFrame(WORD inFrameIdx, DWORD u32BufferAddr, DWORD u32BufferSize, BYTE reqDataFormat,
                    WORD* pu16ImageWidth, WORD* pu16ImageHeight, WORD* pu16DelayTime);

void ResetGIFValue(void);
void GifCallBack(void);

#if 0
DWORD Img_GIFGetSize(BYTE * bpSource);
SWORD Img_GIFDecodeThumb(BYTE * bpSource, BYTE * bpTarget, DWORD dwSrcBufferSize);
SWORD Img_GIFDecode(BYTE * bpSource, BYTE * bpTarget, BYTE yield,DWORD dwSrcBufferSize);
#endif

#endif

