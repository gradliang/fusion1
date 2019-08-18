
#ifndef __JPEG_H
#define __JPEG_H


//#include "global612.h"
#include "../../libIPLAY/libsrc/display/include/displaystructure.h"
#include "../../libIPLAY/libsrc/mpapi/include/mpStreamApi.h"

typedef struct  {
	ST_MPX_STREAM *psStream;
	BYTE *pbTarget;
    BYTE *pbTarget_r; // for 3D 
    BYTE bRImg;       // for 3D
	BYTE *pbSource;
	
	DWORD dwSourceSize;
	DWORD dwTargetSize;

	DWORD dwCduDri;
	DWORD dwCduControl;
	DWORD dwOffset;
	int  iErrorCode;
	
	DWORD dwDhtPos[4];
	DWORD dwDqtPos[2];
	DWORD dwApp1Pos;	
	
	DWORD dwEcsStart;
	DWORD dwEoiPoint;

	DWORD dwThumbEcsStart;
	DWORD dwThumbEoiPoint;
	
	BYTE *paMinTable;
	BYTE *pbSymbolTable;
	WORD *pwBaseTable;

	WORD wImageWidth;
	WORD wImageHeight;

	WORD wOriginalHeight, wOriginalWidth;
	WORD wThumbHeight, wThumbWidth;
	
	WORD wTargetWidth;
	WORD wTargetHeight;
	WORD wRealTargetWidth;
	WORD wRealTargetHeight;	
	
	WORD wCduWidth;
	WORD wCduHeight;
	WORD wCduResidue;
	WORD wCellSize;
	
	WORD wImageType;
	BYTE bReserve[2];    

	BYTE ac1, ac2, dc1, dc2;
	
	BYTE boWithDQ;
	BYTE bDhtHit;
	BYTE bDqtHit;	
	BYTE bProgressive;

	BYTE blNintyDegreeFlag;
	BYTE blSpecialSizeFlag;
	
	BYTE bJResizeRatio;
	BYTE bDecodeMode;
	
	BYTE bMONO;
	BYTE b422V;
	BYTE bRotate;		// 1:CLOCKWISE, 2:ANTI-CLOCKWISE, 3:UPSIDE-DOWN
	BYTE bCMYK;

	BYTE bWaitDecodeFinish;

} ST_JPEG;

#if OPEN_EXIF
#define CAMERAINFOSIZE 52

#if 0 //Frank lin close
typedef struct {
WORD OrientationValue;
WORD Reserved;
BYTE MakeValue[CAMERAINFOSIZE];
BYTE ModelValue[CAMERAINFOSIZE];
BYTE DateTimeValue[CAMERAINFOSIZE];
BYTE WindowsTitleValue[CAMERAINFOSIZE];
BYTE WindowsArtistValue[CAMERAINFOSIZE];
BYTE WindowsUserCommentValue[CAMERAINFOSIZE];
BYTE WindowsMakerNoteValue[CAMERAINFOSIZE];
BYTE WindowsKeywordValue[CAMERAINFOSIZE];
BYTE ArtistValue[CAMERAINFOSIZE];
BYTE CopyRightValue[CAMERAINFOSIZE];
BYTE ImageDescriptionValue[CAMERAINFOSIZE];
BYTE UserCommentValue[CAMERAINFOSIZE];


}ST_EXIF;
#endif

/*for EXIF TAG Definition*/
#define EXIF_TYPE_USIGNED_BYTE    1 
#define EXIF_TYPE_ASCII_STRING    2
#define EXIF_TYPE_USIGNED_SHORT   3
#define EXIF_TYPE_USIGNED_LONG    4
#define EXIF_TYPE_RATIONAL        5
#define EXIF_TYPE_SIGNED_BYTE     6

#define EXIF_TYPE_UNDEFINED       7 
#define EXIF_TYPE_SRATIONAL       10

#define EXIF_TYPE_SKIP_TYPE       255


/*the parameter for the structure of ST_EXIF */
#define NUM_OF_IFD0TAG  40
#define NUM_OF_SubIFDTAG 40

#if MPO
#define NUM_OF_MakerNote  2
#endif



typedef struct {

WORD   IFD0Tag[NUM_OF_IFD0TAG];/*default:18.  Depend on the number of Tag supported.*/
WORD   IFD0Type[NUM_OF_IFD0TAG];
DWORD  IFD0Count[NUM_OF_IFD0TAG];
DWORD  IFD0Value1[NUM_OF_IFD0TAG][6];/*Value numerator  */
DWORD  IFD0Value2[NUM_OF_IFD0TAG][6];/*Value  denominator */
BYTE   IFD0String[NUM_OF_IFD0TAG][CAMERAINFOSIZE];
DWORD  NoOfZeroIFD;


WORD  SubIFDTag[NUM_OF_SubIFDTAG];/*default:18.  Depend on the number of Tag supported.*/
WORD  SubIFDType[NUM_OF_IFD0TAG];
DWORD SubIFDCount[NUM_OF_IFD0TAG];
DWORD SubIFDValue1[NUM_OF_IFD0TAG][6];/*Value numerator  3=number of rational group*/
DWORD SubIFDValue2[NUM_OF_IFD0TAG][6];/*Value  denominator */
BYTE  SubIFDString[NUM_OF_IFD0TAG][CAMERAINFOSIZE];
DWORD NoOfSubIFD;
//DWORD SubIFDOffset;

#if MPO
/*EXIF Maker Note*/
/* TAG = 0x927C need to set in SubIFDTag*/
WORD  MakerNoteTag[NUM_OF_MakerNote];/*default:2.  Depend on the number of Tag supported.*/
WORD  MakerNoteType[NUM_OF_MakerNote];
DWORD MakerNoteCount[NUM_OF_MakerNote];
DWORD MakerNoteValue1[NUM_OF_MakerNote][1];/*Value numerator  1=number of rational group*/
DWORD MakerNoteValue2[NUM_OF_MakerNote][1];/*Value  denominator */
DWORD NoOfMakerNote;
#endif



BYTE  EXIFDec2EncEnable;/*0:Encode with EXIF info(need to set En). 1*/
BYTE  REVERSE1;
BYTE  REVERSE2;
BYTE  REVERSE3;

}ST_EXIF;

typedef struct {
  WORD OrientationValue;
  BYTE DateTimeValue[CAMERAINFOSIZE];
  BYTE DateTimeOriginalValue[CAMERAINFOSIZE];

}ST_EXIF_INFO;



/*-------- Get EXIF to UI--------*/
#define EXIF_STRING_LENGTH 52

typedef struct
{
    /*------ IFD0  ------*/
	DWORD ImageWidth;
	DWORD ImageHeight;
	BYTE Make[EXIF_STRING_LENGTH];   /*TAG=0x010f*/
    BYTE Model[EXIF_STRING_LENGTH];  /*TAG=0x0110*/
    DWORD Orientation;                /*TAG=0x0112*/

	/*------ EXIF SubIFD ------*/
    DWORD ExposureTimeN;/*numerator */
    DWORD ExposureTimeD;/*denominator */
	DWORD FNumberN;/*FNumber*/
    DWORD FNumberD;
	DWORD ExposureProgram;
	DWORD ISOSpeedRatings;
	DWORD FocalLengthN;/*FocalLength */
    DWORD FocalLengthD;
    DWORD MaxApertureValueN;/*MaxApertureValue*/
    DWORD MaxApertureValueD;
	DWORD MeteringMode;
	DWORD Flash;
    BYTE  DateTimeOriginal[20];
	BYTE  ExifVersion[5];


	/*Flag*/
	BYTE Make_flag;
	BYTE Model_flag;
	BYTE Orientation_flag;
	BYTE ExposureTime_flag;
	BYTE FNumber_flag;
	BYTE ExposureProgram_flag;
	BYTE ISOSpeedRatings_flag;
	BYTE FocalLength_flag;
	BYTE MaxApertureValue_flag;
	BYTE MeteringMode_flag;
	BYTE Flash_flag;
	BYTE DateTimeOriginal_flag;
	BYTE ExifVersion_flag;
	
} FILE_EXIF_INFO_TYPE;







#endif

// define the Markers
#define	MARKER_FIRST	0xff
#define	ECS_STUFFED	0
#define	MARKER_SOF0	0xc0	// start of frame for Baseline line DCT

#define	MARKER_SOF1	0xc1	// not supported marker
#define	MARKER_SOF2	0xc2	//
#define	MARKER_SOF3	0xc3	//
#define	MARKER_SOF5	0xc5	//
#define	MARKER_SOF6	0xc6	//
#define	MARKER_SOF7	0xc7	//
#define	MARKER_JPG		0xc8	//
#define	MARKER_SOF9	0xc9	//
#define	MARKER_SOF10	0xca	//
#define	MARKER_SOF11	0xcb	//
#define	MARKER_DAC	0xcc	//
#define	MARKER_SOF13	0xcd	//
#define	MARKER_SOF14	0xce	//
#define	MARKER_SOF15	0xcf	//

#define	MARKER_DHT	0xc4	// Define Huffman table(s)
#define	MARKER_RST0	0xd0	// Restart with modulo 8 count 0
#define	MARKER_RST1	0xd1	// Restart with modulo 8 count 1
#define	MARKER_RST2	0xd2	// Restart with modulo 8 count 2
#define	MARKER_RST3	0xd3	// Restart with modulo 8 count 3
#define	MARKER_RST4	0xd4	// Restart with modulo 8 count 4
#define	MARKER_RST5	0xd5	// Restart with modulo 8 count 5
#define	MARKER_RST6	0xd6	// Restart with modulo 8 count 6
#define	MARKER_RST7	0xd7	// Restart with modulo 8 count 7
#define	MARKER_SOI		0xd8	// Start of image
#define	MARKER_EOI		0xd9	// End of image
#define	MARKER_SOS	0xda	// Start of scan
#define	MARKER_DQT	0xdb	// Define quantization table(s)
#define	MARKER_DNL	0xdc	// Define number of line
#define	MARKER_DRI		0xdd	// Define restart interval
#define	MARKER_DHP	0xde
#define	MARKER_EXP		0xdf	// Expand reference component(s)

#define	MARKER_APP0	0xe0	// application segment
#define	MARKER_APP1	0xe1	// application segment
#define	MARKER_APP2	0xe2	// application segment
#define	MARKER_APP3	0xe3	// application segment
#define	MARKER_APP4	0xe4	// application segment
#define	MARKER_APP5	0xe5	// application segment
#define	MARKER_APP6	0xe6	// application segment
#define	MARKER_APP7	0xe7	// application segment
#define	MARKER_APP8	0xe8	// application segment
#define	MARKER_APP9	0xe9	// application segment
#define	MARKER_APP10	0xea	// application segment
#define	MARKER_APP11	0xeb	// application segment
#define	MARKER_APP12	0xec	// application segment
#define	MARKER_APP13	0xed	// application segment
#define	MARKER_APP14	0xee	// application segment
#define	MARKER_APP15	0xef	// application segment

#define	MARKER_JPG0	0xf0	// JPEG extension
#define	MARKER_JPG1	0xf1	// JPEG extension
#define	MARKER_JPG2	0xf2	// JPEG extension
#define	MARKER_JPG3	0xf3	// JPEG extension
#define	MARKER_JPG4	0xf4	// JPEG extension
#define	MARKER_JPG5	0xf5	// JPEG extension
#define	MARKER_JPG6	0xf6	// JPEG extension
#define	MARKER_JPG7	0xf7	// JPEG extension
#define	MARKER_JPG8	0xf8	// JPEG extension
#define	MARKER_JPG9	0xf9	// JPEG extension
#define	MARKER_JPG10	0xfa	// JPEG extension
#define	MARKER_JPG11	0xfb	// JPEG extension
#define	MARKER_JPG12	0xfc	// JPEG extension
#define	MARKER_JPG13	0xfd	// JPEG extension
#define	MARKER_COM		0xfe	// Comment

#define	NOT_JPEG_FILE					-1
#define	NOT_SUPPORTED_FRAME_TYPE	-2
#define	NOT_SUPPORTED_FRAME_SIZE	-3
#define	NOT_SUPPORTED_VIDEO_FORMAT	-4
#define	FILE_EXTRACT_FAIL				-6
#define	JPEG_DECODE_TIMEOUT			-7
#define	JPEG_DECODE_CANCEL			-8
#define JPEG_FILE_ERROR				-9
#define JPEG_DECODE_ERROR			-10
#define JPEG_PROGRESSIVE_MODE		-11
#define JPEG_THUMB_DECODED			1

#define	VIDEO_422		0x00
#define	VIDEO_420		0x40
#define	VIDEO_444		0x80
#define	VFMT_MASK		0xc0

// JPEG each format MCU size
#define	FORMAT_444_WIDTH		8
#define	FORMAT_444_HEIGHT		8
#define	FORMAT_422H_WIDTH		16
#define	FORMAT_422H_HEIGHT	8
#define	FORMAT_422V_WIDTH		8
#define	FORMAT_422V_HEIGHT	16
#define	FORMAT_420_WIDTH		16
#define	FORMAT_420_HEIGHT		16

// JPEG stream operation macros
#define	GET_BYTE(x)		*(x);(x)++
#define	JPEG_GET_WORD(x,y)	(y)=*(x);(x)++;(y)=(y<<8)+*(x);(x)++
#define	SKIP_BYTE(x)	(x)++
#define	SKIP_WORD(x)	(x)+=2
#define	SKIP_N(x,n)		(x)+=(n)

#define	JPEG_HEADER_LENGTH			589
#define	JPEG_HEADER_OFFSET_X			561
#define	JPEG_HEADER_OFFSET_QT0		7

#define THUMB_HEADER_LENGTH 		328

#define	JPEG_MODE_FINE			0
#define	JPEG_MODE_STANDARD		1
#define	JPEG_MODE_ECONOMIC		2

#ifndef JPEG_EXTRA_WAIT_TIME
	#define JPEG_EXTRA_WAIT_TIME	1
#endif


/*0 : disable Set Resize Ratio by target width*/
#define SET_RATIO_BY_WIDTH 0  
//#define SET_RATIO_BY_BUF_WIDTH 1


typedef struct{    
    BYTE *addr[20];
    BYTE *fileAddr;
    BYTE bNum;
    BYTE bDataFilled;
    BYTE bReserve[2];
}ST_JPEG_TAG;

typedef struct
{
		DWORD Start ;
		DWORD End ;
		WORD  Width ;
		WORD  Height ;
}ThumbInfo;

//extern WORD wJpegWidth, wJpegHeight, wOriginalWidth, wOriginalHeight, wImageWidth, wImageHeight;

#if MPO
typedef struct  {
	DWORD dwFirstApp2Pos;
    void* mpoPrivate;
} ST_JPEG_AddFor_Mpo;
#endif /*MPO*/



WORD Jpg_GetTargetWidth();
WORD Jpg_GetTargetHeight();
WORD Jpg_GetImageWidth();
WORD Jpg_GetImageHeight();
DWORD Jpg_GetImageFrameWidth();


SWORD Img_ExtractThumbnail(BYTE *, BYTE *, DWORD);
WORD Img_GetOriginalWidth();
WORD Img_GetOriginalHeight();
//WORD Img_GetImgWidth();
//WORD Img_GetImgHeight();

#define Img_GetImgWidth   Jpg_GetTargetWidth
#define Img_GetImgHeight  Jpg_GetTargetHeight
//#define Img_GetOriginalWidth   Jpg_GetImageWidth
//#define Img_GetOriginalHeight  Jpg_GetImageHeight
#define Img_JpegGetCurWidth   Jpg_GetTargetWidth
#define Img_JpegGetCurHeight  Jpg_GetTargetHeight
#define Img_JpegGetWidth    Jpg_GetImageWidth
#define Img_JpegGetHeight    Jpg_GetImageHeight

//DWORD Img_JpegGetSize(BYTE *);
#if THREE_D_PROJECT
ST_JPEG * Jpg_Decoder_Init(ST_MPX_STREAM *psStream, BYTE * pbTarget, BYTE * pbTarget_3D, DWORD dwTargetSize, BYTE bMode, DWORD dwOffset);
#else
ST_JPEG * Jpg_Decoder_Init(ST_MPX_STREAM *psStream, BYTE * pbTarget, DWORD dwTargetSize, BYTE bMode, DWORD dwOffset);
#endif
void *ImageAllocTargetBuffer(DWORD dwSize);


WORD GetScaleArray(BYTE *, WORD, WORD, WORD, WORD);
BYTE GetRealZoom(void);
void Img_Fit(ST_IMGWIN *, ST_IMGWIN *);
void Img_FitWithTaskYield(ST_IMGWIN *trg, ST_IMGWIN *src);
void Img_JpegCloseFit(ST_IMGWIN *, ST_IMGWIN *);
int Img_Jpeg2Img(ST_IMGWIN *, BYTE *);
int Img_Jpeg2ImgBuf(BYTE *, BYTE *, BYTE, DWORD, DWORD);

void Img_ClearWin(ST_IMGWIN *);
void Img_Rotate90(ST_IMGWIN *, ST_IMGWIN *);
void Img_Rotate270(ST_IMGWIN *, ST_IMGWIN *);
void Img_Rotate180(ST_IMGWIN *, ST_IMGWIN *);

#endif

