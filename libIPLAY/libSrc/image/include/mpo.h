#ifndef __MPO_H
#define __MPO_H

#include "global612.h"
#include "mpTrace.h"
#include "mpapi.h"
#include "image.h"
#include "Jpeg.h"


/*
Define MPO and MPO_ENCODE
These should be defined in platform.h to Enable or Disable.
-- reference example --
#define MPO   1
#if (MPO == ENABLE)
	#define MPO_ENCODE   ENABLE
#endif
*/

#define MP_FORMAT_ID                        0x4d504600  //"MPF"
#define MP_LITTLE_ENDIAN                    0x49492a00
#define MP_BIG_ENDIAN                       0x4d4d002a

//MP Index IFD Tag Id
#define TAG_MPF_VERSION                     0xb000
#define TAG_NUMBER_OF_IMAGES                0xb001
#define TAG_MP_ENTRY                        0xb002
#define TAG_IMAGE_UID_LIST                  0xb003
#define TAG_TOTAL_FRAMES                    0xb004


#define EXIF_TIFF_ID                        0x002A
#define EXIF_INTEL_ORDER                    0x4949
#define EXIF_MOTO_ORDER                     0x4D4D

/*MPO_ENCODE */
#define USE_DEFAULT_MPO_INFO  0
#define USE_DECODED_MPO_INFO  1

typedef struct __st_mp_header
{
    DWORD dwMPEndian;
    DWORD dwOffsetFirstIFD;
} ST_MP_HEADER;

typedef struct __st_mpo_entry
{
    DWORD dwAttribute;
    DWORD dwImageSize;
    DWORD dwOffset;
    WORD wDependent1;
    WORD wDependent2;
} ST_MPO_ENTRY;

typedef struct __st_mpo_data
{
    DWORD dwImageNumber;
    DWORD dwMPEntryOffset;
    DWORD dwImageUIDOffset;
    DWORD dwTotalFrames;
    ST_MPO_ENTRY* pEntry;
} ST_MPO_DATA;

// Data Type
typedef enum {
    EXIF_TYPE_BYTE_MPO = 1,                     // U08
    EXIF_TYPE_ASCII_MPO,                        // ASCII
    EXIF_TYPE_SHORT_MPO,                        // U16
    EXIF_TYPE_LONG_MPO,                         // U32
    EXIF_TYPE_RATIONAL_MPO,                     // U64
    EXIF_TYPE_SBYTE_MPO,                        // S08
    EXIF_TYPE_UNDEFINED_MPO,                    //
    EXIF_TYPE_SSHORT_MPO,                       // S16
    EXIF_TYPE_SLONG_MPO,                        // S32
    EXIF_TYPE_SRATIONAL_MPO,                    // S64
    EXIF_TYPE_FLOAT_MPO,                        // float
    EXIF_TYPE_DOUBLE_MPO,                       // double float

    EXIF_MAX_TYPES_MPO
} E_EXIF_TYPE;

typedef enum {
    kByteCount = 1,
    kAsciiCount = 1,
    kShortCount = 2,
    kLongCount = 4,
    kRationalCount = 8,
    kSByteCount = 1,
    kUndefineCount = 1,
    kSShortCount = 2,
    kSlongCount = 4,
    kSRationalCount = 8,
    kFloatCount = 4,
    kDoubleCount = 8
} E_JPG_EXIF_DATA_LENGTH;


#if MPO_ENCODE

typedef struct {
    WORD u16Tag;
    WORD u16DataType;
    DWORD u32DataLength;
    BYTE u08BufferPtr[32];
	DWORD Value1[1];                /*SRATIONAL numerator*/
	DWORD Value2[1];                /*SRATIONAL denominator*/
    DWORD unsavedFlag;              // reserved for MPO file create, don't used it.
} ST_JPG_EXIF_TAG_INFO;


/*MPO Encode: Smart Copy*/
/*
These TAG must be encoded in MPO.
TAG_MPF_VERSION
TAG_NUMBER_OF_IMAGES
TAG_MP_ENTRY
MAX_NUM_OF_MpIndexIfd define 3
*/
#ifndef MAX_NUM_OF_MpIndexIfd
	#define MAX_NUM_OF_MpIndexIfd 3
#endif


#endif /*MPO_ENCODE*/



int Mpo_Decoder_ImageFile_Init(IMAGEFILE *psImage);
ST_JPEG * Mpo_Decoder_Init(ST_MPX_STREAM *psStream, BYTE * pbTarget, DWORD dwTargetSize, BYTE bMode, DWORD dwOffset);
int Mpo_Decoder_DecodeImage(IMAGEFILE *psImage);
int Mpo_Decoder_DecodeImage_Right(IMAGEFILE *psImage);
int Mpo_Decoder_Finish(IMAGEFILE* psImage);

BOOL ImageDraw_Check3DFile(BYTE* extension);
BYTE ImageDraw_CheckSize_3D(WORD srcWidth, WORD srcHeight, DWORD check_size, BYTE bThumbnail, WORD dstWidth, WORD dstHeight);
SWORD ImageDraw_Decode_3D_Init(IMAGEFILE *stImageFile,WORD wWidth, WORD wHeight, BYTE bMode);
//SWORD ImageDraw_3D(IMAGEFILE *stImageFile, ST_IMGWIN *pLeftWin, ST_IMGWIN *pRightWin, BOOL bSlideShow);
SWORD ImageDraw_Decode_3D_Finish(IMAGEFILE *stImageFile, BOOL bSlideShow);

#if MPO_ENCODE
SDWORD Mpo_FileCreate(STREAM *fileHandle, BYTE *filePrefix, ST_IMGWIN *pLWin, ST_IMGWIN *pRWin, ST_EXIF *psEXIF, ST_EXIF *psEXIF_R, BYTE bUseDecodedMPOInof,BYTE bQualityTable);
#endif


#endif








