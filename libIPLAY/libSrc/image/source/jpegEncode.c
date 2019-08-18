
/////////////////////////////////////////////////////////////////////////////
//					for jpeg encode
/////////////////////////////////////////////////////////////////////////////
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"
#include "../../file/include/file.h"
#include "taskid.h"
#include "jpeg.h"
#include "mpo.h"


#define JPEG_ENCODE					1

#define JPEG_ENCODE_TIMEOUT_CNT     0x10000



#define JPEG_HEADER_LENGTH			589
#define JPEG_HEADER_OFFSET_QT0		7
#define JPEG_HEADER_OFFSET_QT1		72
#define JPEG_HEADER_OFFSET_X		561
#define JPEG_HEADER_OFFSET_Y		563
#define JPEG_HEADER_OFFSET_SOS      575

#define JPEG_HEARDER_SOS_LENGTH     14

#define SPUTC(x,y)                  (*(x)++ = (y))

#if JPEG_ENCODE


/*
  for  Table 8
  dynamic QT, JPEG encoder quality sellect.
*/
#define JPEG_ENC_HighLevel    0
#define JPEG_ENC_MedlleLevel  1
#define JPEG_ENC_LowLevel     2


#if 0 //
//frank remove
/*Select the quality of JPEG encode*/
// Standard QT =  TABLE[ JPEG_ENCODE_TABLE_NUM ]*/
/*Total number of the encode table is (JPEG_ENCODE_TABLE_NUM+1)*/
#define   JPEG_ENCODE_QUALITY_SELECT      0
#define   JPEG_ENCODE_QUALITY_SELECT      1
#if JPEG_ENCODE_QUALITY_SELECT
  #define ENCODE_NORMAL					    6  //7
  #define ENCODE_BETTER 					3  // 4
  #define ENCODE_BEST						1
  #define JPEG_ENCODE_TABLE_NUM   ENCODE_NORMAL
#endif


#endif


#if JPEG_ENCODE_QUALITY_SELECT
typedef struct {
    WORD wQuality;
    WORD wMinFactor;
    WORD wAvrFactor;
    WORD wMaxFactor;    
}JPEG_TABLE;//Mason 20080801, use for encode size prediction
#endif

//Min factor from "Hell_Pic.jpg"
//100:458 / 95:664 / 90:838 / 80:1112 / 60:1458 / 40:1755 / 20:2080

//Min factor from 710 encoded images
//100:553 / 95:882 / 90:1200 / 80:1758 / 60:2628 / 40:3351 / 20:4060

//Average factor from 710 encoded images
//100:895 / 95:1584 / 90:2256 / 80:3495 / 60:5461 / 40:7029 / 20:8627
#if 0//Not test copy from FAE Fuji project

#if JPEG_ENCODE_QUALITY_SELECT
const JPEG_TABLE stQualityTable[JPEG_ENCODE_TABLE_NUM] = {
#if (JPEG_ENCODE_TABLE_NUM >= ENCODE_BEST)
//    {100,   458,    895,    0},
    {95,    664,   1584,    0},
#endif
#if (JPEG_ENCODE_TABLE_NUM >= ENCODE_BETTER)
//    {95,    664,   1584,    0},
    {90,    838,   2256,    0},
    {80,   1112,   3495,    0},
#endif
#if (JPEG_ENCODE_TABLE_NUM >= ENCODE_NORMAL)
    {60,   1458,   5461,    0},
    {40,   1755,   7029,    0},
    {20,   2080,   8627,    0},
#endif
};
#endif

#endif //Not test copy from FAE Fuji project


BYTE g_jpegEcodeQualityLevel = JPEG_ENC_MedlleLevel;//JPEG_ENC_MedlleLevel;

//Standard QT
#define Standard_QT_NUM 8 


const BYTE StandardQt[9][130]= {
{
  /*Table 0*/
  //Quality = 100    //Factor=966, Min=855
  0x00, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
},
{
  /*Table 1*/
  //Quality = 95     //Factor=1771, Min=1564
  0x00, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x03, 0x02, 0x02, 
  0x01, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0x03, 0x03, 0x04, 0x04, 0x05, 0x04, 0x04, 0x04, 
  0x05, 0x04, 0x03, 0x03, 0x05, 0x06, 0x05, 0x05, 
  0x05, 0x06, 0x06, 0x06, 0x06, 0x03, 0x04, 0x07, 
  0x07, 0x07, 0x06, 0x07, 0x05, 0x06, 0x06, 0x06, 
  0x01, 
  0x01, 0x01, 0x01, 0x02, 0x01, 0x02, 0x04, 0x02, 
  0x02, 0x04, 0x09, 0x06, 0x05, 0x06, 0x09, 0x09, 
  0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 
  0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 
  0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 
  0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 
  0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 
  0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
},
{
  /*Table 2*/
  //Quality = 90     //Factor=2588, Min=2222
  0x00, 
  0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 
  0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x03, 0x05, 
  0x03, 0x03, 0x02, 0x02, 0x03, 0x06, 0x04, 0x04, 
  0x03, 0x05, 0x07, 0x06, 0x07, 0x07, 0x07, 0x06, 
  0x07, 0x06, 0x08, 0x09, 0x0B, 0x09, 0x08, 0x08, 
  0x0A, 0x08, 0x06, 0x07, 0x0A, 0x0D, 0x0A, 0x0A,
  0x0B, 0x0C, 0x0C, 0x0D, 0x0C, 0x07, 0x09, 0x0E, 
  0x0F, 0x0E, 0x0C, 0x0F, 0x0B, 0x0C, 0x0C, 0x0C, 
  0x01, 
  0x03, 0x03, 0x03, 0x04, 0x03, 0x04, 0x08, 0x04, 
  0x04, 0x08, 0x12, 0x0C, 0x0A, 0x0C, 0x12, 0x12, 
  0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 
  0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 
  0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 
  0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 
  0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 
  0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 
},
{
  /*Table 3*/	
  //Quality = 80     //Factor=4110, Min=3370
  0x00, 
  0x04, 0x02, 0x03, 0x03, 0x03, 0x02, 0x04, 0x03, 
  0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x06, 0x0A,
  0x06, 0x06, 0x05, 0x05, 0x06, 0x0C, 0x08, 0x09, 
  0x07, 0x0A, 0x0E, 0x0C, 0x0F, 0x0F, 0x0E, 0x0C, 
  0x0E, 0x0D, 0x10, 0x12, 0x17, 0x13, 0x10, 0x11, 
  0x15, 0x11, 0x0D, 0x0E, 0x14, 0x1B, 0x14, 0x15, 
  0x17, 0x18, 0x19, 0x1A, 0x19, 0x0F, 0x13, 0x1C, 
  0x1E, 0x1C, 0x19, 0x1E, 0x17, 0x19, 0x19, 0x18, 
  0x01, 
  0x06, 0x06, 0x06, 0x09, 0x07, 0x09, 0x11, 0x09, 
  0x09, 0x11, 0x25, 0x18, 0x15, 0x18, 0x25, 0x25, 
  0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 
  0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 
  0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 
  0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 
  0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 
  0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 
},
{
  /*Table 4*/
  //Quality = 60     //Factor=6521, Min=5087
  0x00, 
  0x08, 0x05, 0x06, 0x07, 0x06, 0x05, 0x08, 0x07, 
  0x06, 0x07, 0x09, 0x08, 0x08, 0x09, 0x0C, 0x14, 
  0x0D, 0x0C, 0x0B, 0x0B, 0x0C, 0x18, 0x11, 0x12, 
  0x0E, 0x14, 0x1D, 0x19, 0x1E, 0x1E, 0x1C, 0x19, 
  0x1C, 0x1B, 0x20, 0x24, 0x2E, 0x27, 0x20, 0x22, 
  0x2B, 0x22, 0x1B, 0x1C, 0x28, 0x36, 0x28, 0x2B, 
  0x2F, 0x31, 0x33, 0x34, 0x33, 0x1F, 0x26, 0x38, 
  0x3C, 0x38, 0x32, 0x3C, 0x2E, 0x32, 0x33, 0x31, 
  0x01, 
  0x0C, 0x0D, 0x0D, 0x12, 0x0F, 0x12, 0x23, 0x13, 
  0x13, 0x23, 0x4A, 0x31, 0x2A, 0x31, 0x4A, 0x4A, 
  0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 
  0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 
  0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 
  0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 
  0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 
  0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 
},
{
  /*Table 5*/
  //Quality = 40     //Factor=8369, Min=6373
  0x00, 
  0x0C, 0x08, 0x09, 0x0A, 0x09, 0x07, 0x0C, 0x0A, 
  0x09, 0x0A, 0x0D, 0x0C, 0x0C, 0x0E, 0x12, 0x1E, 
  0x13, 0x12, 0x10, 0x10, 0x12, 0x24, 0x1A, 0x1B, 
  0x15, 0x1E, 0x2B, 0x26, 0x2D, 0x2D, 0x2A, 0x26, 
  0x2A, 0x29, 0x30, 0x36, 0x45, 0x3A, 0x30, 0x33, 
  0x41, 0x33, 0x29, 0x2A, 0x3C, 0x51, 0x3C, 0x41, 
  0x47, 0x49, 0x4D, 0x4E, 0x4D, 0x2E, 0x39, 0x54, 
  0x5A, 0x54, 0x4B, 0x5A, 0x45, 0x4B, 0x4D, 0x4A, 
  0x01, 
  0x13, 0x14, 0x14, 0x1B, 0x17, 0x1B, 0x34, 0x1D, 
  0x1D, 0x34, 0x6F, 0x4A, 0x3F, 0x4A, 0x6F, 0x6F, 
  0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 
  0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 
  0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 
  0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 
  0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 
  0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 
},
{
  /*Table 6*/
  //Quality = 20     //Factor=10241, Min=7595
  0x00, 
  0x10, 0x0B, 0x0C, 0x0E, 0x0C, 0x0A, 0x10, 0x0E, 
  0x0D, 0x0E, 0x12, 0x11, 0x10, 0x13, 0x18, 0x28, 
  0x1A, 0x18, 0x16, 0x16, 0x18, 0x31, 0x23, 0x25, 
  0x1D, 0x28, 0x3A, 0x33, 0x3D, 0x3C, 0x39, 0x33, 
  0x38, 0x37, 0x40, 0x48, 0x5C, 0x4E, 0x40, 0x44, 
  0x57, 0x45, 0x37, 0x38, 0x50, 0x6D, 0x51, 0x57, 
  0x5F, 0x62, 0x67, 0x68, 0x67, 0x3E, 0x4D, 0x71, 
  0x79, 0x70, 0x64, 0x78, 0x5C, 0x65, 0x67, 0x63, 
  0x01, 
  0x19, 0x1B, 0x1B, 0x24, 0x1F, 0x24, 0x46, 0x27, 
  0x27, 0x46, 0x94, 0x63, 0x54, 0x63, 0x94, 0x94, 
  0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 
  0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 
  0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 
  0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 
  0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 
  0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 0x94, 
},
{
  /*Table 7*/
  //Standard QT
  0x00, // first quantization table
  16, 11, 10, 16, 24, 40, 51, 61,
  12, 12, 14, 19, 26, 58, 60, 55,
  14, 13, 16, 24, 40, 57, 69, 56,
  14, 17, 22, 29, 51, 87, 80, 62,
  18, 22, 37, 56, 68, 109, 103, 77,
  24, 35, 55, 64, 81, 104, 113, 92,
  49, 64, 78, 87, 103, 121, 120, 101,
  72, 92, 95, 98, 112, 100, 103, 99,
  0x01, // second quantization table
  17, 18, 24, 47, 99, 99, 99, 99, 
  18, 21, 26, 66, 99, 99, 99, 99,
  24, 26, 56, 99, 99, 99, 99, 99,
  47, 66, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99,
  99, 99, 99, 99, 99, 99, 99, 99
},
{
  /*Table 8*/
  //dynamic QT
  0x00, // first quantization table
  0x10, 0x0B, 0x0C, 0x0E, 0x0C, 0x0A, 0x10, 0x0E,
  0x0D, 0x0E, 0x12, 0x11, 0x10, 0x13, 0x18, 0x28,
  0x1A, 0x18, 0x16, 0x16, 0x18, 0x31, 0x23, 0x25,
  0x1D, 0x28, 0x3A, 0x33, 0x3D, 0x3C, 0x39, 0x33,
  0x38, 0x37, 0x40, 0x48, 0x5C, 0x4E, 0x40, 0x44,
  0x57, 0x45, 0x37, 0x38, 0x50, 0x6D, 0x51, 0x57,
  0x5F, 0x62, 0x67, 0x68, 0x67, 0x3E, 0x4D, 0x71,
  0x79, 0x70, 0x64, 0x78, 0x5C, 0x65, 0x67, 0x63,
  0x01, // second quantization table
  0x11, 0x12, 0x12, 0x18, 0x15, 0x18, 0x2F, 0x1A, 
  0x1A, 0x2F, 0x63, 0x42, 0x38, 0x42, 0x63, 0x63,
  0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
  0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
  0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
  0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
  0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
  0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63
}

};
/*--------------------------------------------------------*/

const WORD CduQt[0x100] =
{
	0x0000, 0x0001, 0x0400, 0x02ab, 0x0200, 0x0b33, 0x0155, 0x0a49, 0x0100,
	0x09c7, 0x00cd, 0x12e9, 0x0955, 0x093b, 0x1249, 0x0911, 0x0080, 0x08f1,
	0x11c7, 0x11af, 0x08cd, 0x08c3, 0x08ba, 0x08b2, 0x1155, 0x251f, 0x113b,
	0x1a5f, 0x1a49, 0x1a35, 0x1111, 0x2421, 0x0880, 0x23e1, 0x10f1, 0x10ea,
	0x19c7, 0x19bb, 0x19af, 0x10d2, 0x10cd, 0x231f, 0x10c3, 0x197d, 0x10ba,
	0x10b6, 0x10b2, 0x22b9, 0x1955, 0x229d, 0x10a4, 0x2d05, 0x193b, 0x1935,
	0x225f, 0x1095, 0x2249, 0x223f, 0x2235, 0x2c57, 0x1911, 0x2219, 0x1084,
	0x1082, 0x1080, 0x18fc, 0x18f8, 0x21e9, 0x18f1, 0x21db, 0x18ea, 0x2b9b,
	0x21c7, 0x21c1, 0x21bb, 0x21b5, 0x21af, 0x2b53, 0x18d2, 0x367b, 0x18cd,
	0x2b29, 0x18c8, 0x218b, 0x18c3, 0x2b03, 0x217d, 0x2af1, 0x18ba, 0x18b8,
	0x18b6, 0x18b4, 0x18b2, 0x2ac1, 0x2ab9, 0x2159, 0x2155, 0x3547, 0x2a9d,
	0x214b, 0x18a4, 0x2a89, 0x2141, 0x189f, 0x213b, 0x189c, 0x2135, 0x34c9,
	0x2a5f, 0x2a59, 0x1895, 0x349d, 0x2a49, 0x1891, 0x2a3f, 0x211d, 0x2a35,
	0x188c, 0x2a2b, 0x344d, 0x2111, 0x343b, 0x2a19, 0x2a15, 0x1884, 0x3419,
	0x1882, 0x1881, 0x1880, 0x20fe, 0x20fc, 0x33e9, 0x20f8, 0x3fb3, 0x29e9,
	0x33cb, 0x20f1, 0x33bd, 0x29db, 0x33af, 0x20ea, 0x29d1, 0x20e7, 0x3395,
	0x29c7, 0x20e2, 0x29c1, 0x20df, 0x29bb, 0x20dc, 0x29b5, 0x20d9, 0x29af,
	0x3359, 0x3353, 0x29a7, 0x20d2, 0x3343, 0x299f, 0x20ce, 0x20cd, 0x2997,
	0x3329, 0x20c9, 0x20c8, 0x298d, 0x298b, 0x3311, 0x20c3, 0x3e0f, 0x3303,
	0x3dfd, 0x297d, 0x297b, 0x2979, 0x32ed, 0x20ba, 0x3dc9, 0x20b8, 0x20b7,
	0x20b6, 0x20b5, 0x20b4, 0x20b3, 0x20b2, 0x3d89, 0x20b0, 0x32bd, 0x295d,
	0x3d6b, 0x2959, 0x2957, 0x2955, 0x32a7, 0x20a9, 0x20a8, 0x20a7, 0x3299,
	0x294b, 0x3293, 0x20a4, 0x20a3, 0x3289, 0x2943, 0x2941, 0x3cff, 0x209f,
	0x3279, 0x293b, 0x3273, 0x209c, 0x326d, 0x2935, 0x3ccf, 0x2099, 0x3cc3,
	0x325f, 0x2097, 0x3259, 0x3cad, 0x2095, 0x3251, 0x2927, 0x2093, 0x3249,
	0x3c8d, 0x2091, 0x3c83, 0x323f, 0x3c79, 0x291d, 0x3c6f, 0x3235, 0x3c65,
	0x208c, 0x2917, 0x208b, 0x3229, 0x3227, 0x3c49, 0x2911, 0x2088, 0x290f,
	0x3c37, 0x3219, 0x3217, 0x3215, 0x3c25, 0x2084, 0x3c1d, 0x2083, 0x2905,
	0x2082, 0x2903, 0x2081, 0x2901
};



const WORD CduHet[0x180] =
{
	0x100, 0x101, 0x204, 0x30b, 0x41a, 0x678, 0x7f8, 0x9f6, 0xf82, 0xf83,
	0x30c, 0x41b, 0x679, 0x8f6, 0xaf6, 0xf84, 0xf85, 0xf86, 0xf87, 0xf88,
	0x41c, 0x7f9, 0x9f7, 0xbf4, 0xf89, 0xf8a, 0xf8b, 0xf8c, 0xf8d, 0xf8e,
	0x53a, 0x8f7, 0xbf5, 0xf8f, 0xf90, 0xf91, 0xf92, 0xf93, 0xf94, 0xf95,
	0x53b, 0x9f8, 0xf96, 0xf97, 0xf98, 0xf99, 0xf9a, 0xf9b, 0xf9c, 0xf9d,
	0x67a, 0xaf7, 0xf9e, 0xf9f, 0xfa0, 0xfa1, 0xfa2, 0xfa3, 0xfa4, 0xfa5,
	0x67b, 0xbf6, 0xfa6, 0xfa7, 0xfa8, 0xfa9, 0xfaa, 0xfab, 0xfac, 0xfad,
	0x7fa, 0xbf7, 0xfae, 0xfaf, 0xfb0, 0xfb1, 0xfb2, 0xfb3, 0xfb4, 0xfb5,
	0x8f8, 0xec0, 0xfb6, 0xfb7, 0xfb8, 0xfb9, 0xfba, 0xfbb, 0xfbc, 0xfbd,
	0x8f9, 0xfbe, 0xfbf, 0xfc0, 0xfc1, 0xfc2, 0xfc3, 0xfc4, 0xfc5, 0xfc6,
	0x8fa, 0xfc7, 0xfc8, 0xfc9, 0xfca, 0xfcb, 0xfcc, 0xfcd, 0xfce, 0xfcf,
	0x9f9, 0xfd0, 0xfd1, 0xfd2, 0xfd3, 0xfd4, 0xfd5, 0xfd6, 0xfd7, 0xfd8,
	0x9fa, 0xfd9, 0xfda, 0xfdb, 0xfdc, 0xfdd, 0xfde, 0xfdf, 0xfe0, 0xfe1,
	0xaf8, 0xfe2, 0xfe3, 0xfe4, 0xfe5, 0xfe6, 0xfe7, 0xfe8, 0xfe9, 0xfea,
	0xfeb, 0xfec, 0xfed, 0xfee, 0xfef, 0xff0, 0xff1, 0xff2, 0xff3, 0xff4,
	0xff5, 0xff6, 0xff7, 0xff8, 0xff9, 0xffa, 0xffb, 0xffc, 0xffd, 0xffe,
	0x30a, 0xaf9, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0xfd0, 0xfd1,
	0xfd2, 0xfd3, 0xfd4, 0xfd5, 0xfd6, 0xfd7, 0x101, 0x204, 0x30a, 0x418,
	0x419, 0x538, 0x678, 0x8f4, 0x9f6, 0xbf4, 0x30b, 0x539, 0x7f6, 0x8f5,
	0xaf6, 0xbf5, 0xf88, 0xf89, 0xf8a, 0xf8b, 0x41a, 0x7f7, 0x9f7, 0xbf6,
	0xec2, 0xf8c, 0xf8d, 0xf8e, 0xf8f, 0xf90, 0x41b, 0x7f8, 0x9f8, 0xbf7,
	0xf91, 0xf92, 0xf93, 0xf94, 0xf95, 0xf96, 0x53a, 0x8f6, 0xf97, 0xf98,
	0xf99, 0xf9a, 0xf9b, 0xf9c, 0xf9d, 0xf9e, 0x53b, 0x9f9, 0xf9f, 0xfa0,
	0xfa1, 0xfa2, 0xfa3, 0xfa4, 0xfa5, 0xfa6, 0x679, 0xaf7, 0xfa7, 0xfa8,
	0xfa9, 0xfaa, 0xfab, 0xfac, 0xfad, 0xfae, 0x67a, 0xaf8, 0xfaf, 0xfb0,
	0xfb1, 0xfb2, 0xfb3, 0xfb4, 0xfb5, 0xfb6, 0x7f9, 0xfb7, 0xfb8, 0xfb9,
	0xfba, 0xfbb, 0xfbc, 0xfbd, 0xfbe, 0xfbf, 0x8f7, 0xfc0, 0xfc1, 0xfc2,
	0xfc3, 0xfc4, 0xfc5, 0xfc6, 0xfc7, 0xfc8, 0x8f8, 0xfc9, 0xfca, 0xfcb,
	0xfcc, 0xfcd, 0xfce, 0xfcf, 0xfd0, 0xfd1, 0x8f9, 0xfd2, 0xfd3, 0xfd4,
	0xfd5, 0xfd6, 0xfd7, 0xfd8, 0xfd9, 0xfda, 0x8fa, 0xfdb, 0xfdc, 0xfdd,
	0xfde, 0xfdf, 0xfe0, 0xfe1, 0xfe2, 0xfe3, 0xaf9, 0xfe4, 0xfe5, 0xfe6,
	0xfe7, 0xfe8, 0xfe9, 0xfea, 0xfeb, 0xfec, 0xde0, 0xfed, 0xfee, 0xfef,
	0xff0, 0xff1, 0xff2, 0xff3, 0xff4, 0xff5, 0xec3, 0xff6, 0xff7, 0xff8,
	0xff9, 0xffa, 0xffb, 0xffc, 0xffd, 0xffe, 0x100, 0x9fa, 0x7ff, 0x7ff,
	0x7ff, 0x7ff, 0x7ff, 0x7ff, 0xfd0, 0xfd1, 0xfd2, 0xfd3, 0xfd4, 0xfd5,
	0xfd6, 0xfd7, 0x100, 0x202, 0x203, 0x204, 0x205, 0x206, 0x30e, 0x41e,
	0x53e, 0x67e, 0x7fe, 0x8fe, 0xfff, 0xfff, 0xfff, 0xfff, 0x100, 0x101,
	0x102, 0x206, 0x30e, 0x41e, 0x53e, 0x67e, 0x7fe, 0x8fe, 0x9fe, 0xafe,
	0xfff, 0xfff, 0xfff, 0xfff
};


const BYTE bJpegHeader[JPEG_HEADER_LENGTH] = {
	0xff, 0xd8,					// SOI marker
	0xff, 0xdb,					// DQT marker
	0x00, 0x84,					// Lq

	0x00,						// Pq, Tq
	1, 1, 1, 1, 2, 4, 5, 6,		// first quantization table
	1, 1, 1, 1, 2, 5, 6, 5, 1, 1, 1, 2, 4, 5, 6, 5, 1, 1, 2, 2, 5, 8, 8, 6, 1,
	2, 3, 5, 6, 10, 10, 7, 2, 3, 5, 6, 8, 10, 11, 9, 4, 6, 7, 8, 10, 12, 12,
	10, 7, 9, 9, 9, 11, 10, 10, 9, 0x01,	// Pq, Tq
	1, 1, 2, 4, 9, 9, 9, 9,		// second quantization table
	1, 2, 2, 6, 9, 9, 9, 9, 2, 2, 5, 9, 9, 9, 9, 9, 4, 6, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 0xff, 0xc4,	// DHT marker
	0x01, 0xa2,					// Lh

	0x00,						// Tc, Th
	0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,		// L1..L16
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,	// Vi,j
	0x10,						// Tc, Th
	0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04,
	0x00, 0x00, 0x01, 0x7d,		// L1..L16
	0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06,
	0x13, 0x51, 0x61, 0x07,		// Vi,j
	0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1,
	0x15, 0x52, 0xd1, 0xf0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
	0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x34, 0x35,
	0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65,
	0x66, 0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
	0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94,
	0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,
	0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
	0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
	0xe7, 0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
	0xf9, 0xfa, 0x01,			// Tc, Th
	0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00,		// L1.. L16
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,	// Vi,j
	0x11,						// Tc, Th
	0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04,
	0x00, 0x01, 0x02, 0x77,		// L1..L16
	0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41,
	0x51, 0x07, 0x61, 0x71,		// Vi,j
	0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x09,
	0x23, 0x33, 0x52, 0xf0, 0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
	0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26, 0x27, 0x28, 0x29, 0x2a,
	0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
	0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64,
	0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
	0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92,
	0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
	0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8,
	0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
	0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe2, 0xe3, 0xe4, 0xe5,
	0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
	0xf9, 0xfa, 0xff, 0xc0,		// SOF marker
	0x00, 0x11,					// Lf
	0x08,						// P
	0x00, 0x00,					// X, need to be filled by program
	0x00, 0x00,					// Y, need to be filled by program
	0x03,						// Nf
	0x01, 0x21, 0x00,			// C1, H1, V1, Tg1      Note: this is a 422 format
	0x02, 0x11, 0x01,			// C2, H2, V2, Tg2
	0x03, 0x11, 0x01,			// C3, H3, V3, Tg3

	0xff, 0xda,					// SOS marker
	0x00, 0x0c,					// Ls
	0x03,						// Ns
	0x01,						// Cs1
	0x00,						// Td1, Ta1
	0x02,						// Cs2
	0x11,						// Td2, Ta2
	0x03,						// Cs3
	0x11,						// Td3, Ta3
	0x00,						// Ss
	0x3F,						// Se
	0x00						// Ah, Al
};


static void TurnOnCDUClk()
{
	CLOCK *clock;

	clock = (CLOCK *) (CLOCK_BASE);
	clock->MdClken |= 0x00001000;
}


static void TurnOffCDUClk()
{
	CLOCK *clock;

	clock = (CLOCK *) (CLOCK_BASE);
	clock->MdClken &= 0xffffefff;
}




DWORD JpegEncode(BYTE *target, ST_IMGWIN *pWin, BYTE bQualityTable)
{
        MP_DEBUG1("- %s -", __FUNCTION__);
	
#if JPEG_ENCODE
	/*Add by Frank Lin ,for EXIF*/
	DWORD ExifAddLength=0;

	BYTE * p08;
	WORD * src, count;
	DWORD * trg;
	CDU * cdu;
	CHANNEL * channel;
	DWORD lineOffset;
	DWORD dwTimeoutCount;



	cdu = (CDU *) (CDU_BASE);
	TurnOnCDUClk();

	channel = (CHANNEL *) (DMA_JVLC_BASE);
	channel->Control = 0;
	channel = (CHANNEL *) (DMA_JMCU_BASE);
	channel->Control = 0;

	cdu->CduOp = 0;

    BIU * biu=(BIU *)BIU_BASE;
    biu->BiuArst &= 0xffffffef;
    biu->BiuArst |= 0x00000010;
    //biu->BiuSrst &= 0xffffffef;
		//biu->BiuSrst |= 0x00000010;

	cdu->CduC = (1 << 1)						// selection for encode
	+ (0 << 4)									// restart marker is not inserted
	+ (0 << 5)									// enable VLC port DMA
	+ (0 << 6)									// VLC format is 422
	+ ((((DWORD) target /*+ JPEG_HEADER_LENGTH+ExifAddLength*/) & 0x3) << 8)		// set CDU_DBS
	+ (0 << 10);								// image discontinuous mode

	cdu->CduDri = 8;							// restart interval is zero

	cdu->CduSize = ((pWin->wHeight - 1) << 16) + pWin->wWidth - 1;
	cdu->CduNmcu = (pWin->wHeight * pWin->wWidth >> 7) - 1;		// only for 422 format (16x8)
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	cdu->CduC |= 0x00004000 ;
	cdu->CduCmp0 = 0x00000010;
	cdu->CduCmp1 = 0x00000007;
	cdu->CduCmp2 = 0x00000007;
	cdu->JVLC_BUFSZ = 0x00000000;
#endif

	memcpy((BYTE *)(bJpegHeader + 6), &StandardQt[bQualityTable][0], 130);

	//memcpy((BYTE *)(bJpegHeader + 9), StandardQt, 130);

	// program the quantization encoding table
	p08 = (BYTE *) (&bJpegHeader[JPEG_HEADER_OFFSET_QT0]);
	trg = (DWORD *)cdu->CduQt;
	count = 64;
	while (count --)
	{
		src = (WORD *) CduQt + *p08;
		*trg = *src;
		trg ++;
		p08 ++;
	}

	p08 ++;
	count = 64;
	while (count --)
	{
		src = (WORD *) CduQt + *p08;
		*trg = *src;
		trg ++;
		p08 ++;
	}

	// program the Huffman encoding table
	src = (WORD *) CduHet;
	trg = (DWORD *)cdu->CduHet;
	count = 0x180;
	while (count --)
	{
		*trg = *src;
		trg ++;
		src ++;
	}

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	lineOffset = pWin->dwOffset - pWin->wWidth * 2;
	cdu->JMCU_DMASA = (DWORD)pWin->pdwStart | 0xa0000000;
	//cdu->JMCU_FBWID = lineOffset ;
	cdu->JMCU_FBWID = pWin->wWidth << 1 ;
	cdu->JVLC_DMASA = ((DWORD) target /*+ JPEG_HEADER_LENGTH +ExifAddLength*/) & 0xfffffffc;
	cdu->JVLC_DMASA = cdu->JVLC_DMASA | 0xa0000000;
	channel = (CHANNEL *) (DMA_JVLC_BASE);
	channel->Control = 1;
	channel = (CHANNEL *) (DMA_JMCU_BASE);
	channel->Control = 1;
#else
	channel = (CHANNEL *) (DMA_JVLC_BASE);
	channel->StartA = ((DWORD) target /*+ JPEG_HEADER_LENGTH +ExifAddLength*/ ) & 0xfffffffc;
	channel->EndA = (DWORD) target + 0x1fffff;
	channel->Control = 1;

	lineOffset = pWin->dwOffset - pWin->wWidth * 2;
	channel = (CHANNEL *) (DMA_JMCU_BASE);
	channel->StartA = (DWORD)pWin->pdwStart;
	channel->EndA = (DWORD)pWin->pdwStart + pWin->wHeight * pWin->dwOffset - 1 - lineOffset;
	channel->LineCount = (lineOffset << 16) + pWin->wWidth * 2 - 1;
	channel->Control = 5;
#endif

		EventClear(JPEG_LOAD_DATA_EVENT_ID1, BIT4);
		cdu->CduIc |= BIT8;//finish interrupt
		SystemIntEna(IM_JPEG);

	cdu->CduOp = 1;


				DWORD ret;
				DWORD LoadEvent;
				ret = EventWaitWithTO(JPEG_LOAD_DATA_EVENT_ID1, BIT4, OS_EVENT_OR, &LoadEvent, 200);
				TurnOffCDUClk();
				//mpDebugPrint("----Event arrive-----");
				if(ret != PASS)
				{
					//mpDebugPrint("Wait JPEG_LOAD_DATA_EVENT Time Out");

					return 0;
				}else{
						//mpDebugPrint("Have finished early");

						//cdu->CduIc &= ~BIT0;
						return (DWORD) ((cdu->CduCmps >> 2));
				}

#endif
	return 0;/*JPEG_ENCODE not Enable*/
}






// input  : ST_IMGWIN *pWin   : Include the information of image(start sddress, width, height and offset).
//          BYTE *pbTrgBuffer : The target buffer.
// output : The encode length, if 0 mease encode fail.
DWORD Img2Jpeg(BYTE *pbTrgBuffer, ST_IMGWIN *pWin)
{
	return Img2Jpeg_WithQTable(pbTrgBuffer, pWin, Standard_QT_NUM-1);/*use QT num 7*/
}

/*
g_jpegEcodeQualityLevel
0: the same as: "r_hight   0" defined in recode.h
1: the same as: "r_midlle  1" defined in recode.h
2: the same as: "r_low      2" defined in recode.h
only for Table 8
dynamic QT
*/
void Set_jpegEcodeQualityLevel(BYTE setVal)
{
	MP_DEBUG2("%s : setVal =%d #####", __FUNCTION__, setVal);
	g_jpegEcodeQualityLevel = setVal;
}

BYTE Get_jpegEcodeQualityLevel(void)
{
	return g_jpegEcodeQualityLevel;
}

 //dynamic Qt
void JpegEncQTableInit(void)
{

#if 0	
   BYTE CompressRatio=25;
   WORD quality_test,temp;
   int i,j;

   BYTE getQalityLevel;
 
   BYTE u08QtEcon[2][64]=\
   {
      {
         8, 5, 5, 8, 12, 18, 24, 29,
         5, 5, 7, 9, 12, 27, 29, 26,
         7, 7, 8, 12, 18, 26, 33, 26,
         7, 8, 11, 13, 24, 40, 38, 29,
         8, 11, 17, 26, 31, 51, 48, 37,
        12, 17, 26, 30, 38, 48, 53, 43,
        24, 30, 37, 41, 48, 56, 56, 47,
        34, 43, 44, 46, 52, 47, 48, 47
      },
      {
         8, 8, 12, 22, 47, 47, 47, 47, 
         8, 11, 12, 31, 47, 47, 47, 47,
        12, 12, 26, 47, 47, 47, 47, 47,
        22, 31, 47, 47, 47, 47, 47, 47,
        47, 47, 47, 47, 47, 47, 47, 47,
        47, 47, 47, 47, 47, 47, 47, 47,
        47, 47, 47, 47, 47, 47, 47, 47,
        47, 47, 47, 47, 47, 47, 47, 47
      }
   };

   getQalityLevel = Get_jpegEcodeQualityLevel();

   switch(getQalityLevel)
   {
     case JPEG_ENC_HighLevel:	 	
		MP_DEBUG1("%s :JPEG_ENC_HighLevel.", __FUNCTION__);
	 	CompressRatio = 40;
       break;

     case JPEG_ENC_LowLevel:
	 	MP_DEBUG1("%s :JPEG_ENC_LowLevel.", __FUNCTION__);
	 	CompressRatio = 20;
       break;
	   
   	 case JPEG_ENC_MedlleLevel: /*normal level*/
     default:
	 	CompressRatio = 30;
	 	MP_DEBUG1("%s :JPEG_ENC_MedlleLevel", __FUNCTION__);
   }
   
   if(CompressRatio>100) CompressRatio = 100;            

   if(CompressRatio<50)
     quality_test = 5000 / CompressRatio;
   else
     quality_test = 200 - CompressRatio*2;

   /*0*/
     for (j = 0; j < 64; j++)
     {
#if 0
		 
  	 StandardQt[8][j+1] = u08QtEcon[0][j];		 
#else	 	
        temp = (WORD)(((WORD)u08QtEcon[0][j] * quality_test) + 50) / 100;
                                
        if(temp > 255) temp = 255;
        StandardQt[8][j+1]  = temp;
#endif		
     }

   /*1*/
     for (j = 0; j < 64; j++)
     {
#if 0
		 
  	 StandardQt[8][j+1+65] = u08QtEcon[1][j];		 
#else	 	
        temp = (WORD)(((WORD)u08QtEcon[1][j] * quality_test) + 50) / 100;
                                
        if(temp > 255) temp = 255;
        StandardQt[8][j+1+65]  = temp;
#endif		
     }

#endif


	 
}



DWORD Img2Jpeg_WithQTable(BYTE *const pbOrgTrgBuffer, ST_IMGWIN *pWin, BYTE bQualityTable)
{
        MP_DEBUG1("## %s ##", __FUNCTION__);
	DWORD fileLength = 0;
	BYTE * trg, * src;
	DWORD count;
	DWORD ret;
	ST_IMGWIN tmpWin;
	BYTE bQualityTable_set;
	BYTE *orgTrgBufferPtr;
	BYTE *pbTrgBuffer;


   if(bQualityTable == 8)
   {
     JpegEncQTableInit();
   }
#if JPEG_ENCODE
	orgTrgBufferPtr = pbOrgTrgBuffer;
	pbTrgBuffer     = pbOrgTrgBuffer;

		MP_DEBUG2("%s: before enc pbTrgBuffer =0x%x", __FUNCTION__, pbTrgBuffer);
	
		if (bQualityTable <= Standard_QT_NUM)
			{
				bQualityTable_set = bQualityTable;
			}
			else
			{
				bQualityTable_set = Standard_QT_NUM;
				MP_DEBUG2("-E- %s:Wrong bQualityTable. bQualityTable = %d", __FUNCTION__, bQualityTable);
				MP_DEBUG2("-E- %s: Sellect QT num =%d", __FUNCTION__, bQualityTable_set);
				
			}


    tmpWin.pdwStart = pWin->pdwStart;
	tmpWin.dwOffset = pWin->dwOffset;
	tmpWin.wWidth = pWin->wWidth & 0xfff0;			// make sure the width is a multiple of 16
	tmpWin.wHeight = pWin->wHeight & 0xfff8;		// make sure the height is a multiple of 8

 	memcpy((BYTE *)(bJpegHeader + 6), &StandardQt[bQualityTable_set][0], 130);              

	///////////////////////////////////////////////////////////
    //
    // fill the header to target buffer
    //
    ///////////////////////////////////////////////////////////
    memcpy(pbTrgBuffer, (BYTE *) &bJpegHeader, JPEG_HEADER_LENGTH - JPEG_HEARDER_SOS_LENGTH);
    memcpy(pbTrgBuffer + 6, &StandardQt[bQualityTable_set][0], 130);

    trg = pbTrgBuffer + JPEG_HEADER_OFFSET_X;

    *trg = tmpWin.wHeight >> 8;
    *(trg + 1) = tmpWin.wHeight;
    *(trg + 2) = tmpWin.wWidth >> 8;
    *(trg + 3) = tmpWin.wWidth;

    fileLength = (JPEG_HEADER_LENGTH - JPEG_HEARDER_SOS_LENGTH);
    pbTrgBuffer += (JPEG_HEADER_LENGTH - JPEG_HEARDER_SOS_LENGTH);

	MP_DEBUG2("%s:1. pbTrgBuffer =0x%x", __FUNCTION__, pbTrgBuffer);
	MP_DEBUG2("%s:1. fileLength = %d", __FUNCTION__, fileLength);
	///////////////////////////////////////////////////////////
    //
    // SOS marker
    //
    ///////////////////////////////////////////////////////////
    memcpy(pbTrgBuffer, (BYTE *) &bJpegHeader[JPEG_HEADER_LENGTH - JPEG_HEARDER_SOS_LENGTH], JPEG_HEARDER_SOS_LENGTH);
    pbTrgBuffer += JPEG_HEARDER_SOS_LENGTH;
    fileLength += JPEG_HEARDER_SOS_LENGTH;

	MP_DEBUG2("%s:2. pbTrgBuffer =0x%x", __FUNCTION__, pbTrgBuffer);
	MP_DEBUG2("%s:2. fileLength = %d", __FUNCTION__, fileLength);

	///////////////////////////////////////////////////////////
    //
    //  Encode JPEG
    //
    ///////////////////////////////////////////////////////////

    ret = JpegEncode(pbTrgBuffer, &tmpWin, bQualityTable_set);

	if (!ret)
	{
		mpDebugPrint("Encode JPEG fail !!!");
		
		return 0;
	}
   	pbTrgBuffer += ret;
   	fileLength += ret;

	MP_DEBUG2("%s:3. pbTrgBuffer =0x%x", __FUNCTION__, pbTrgBuffer);
	MP_DEBUG2("%s:3. fileLength = %d", __FUNCTION__, fileLength);

    MP_DEBUG2("#### orgTrgBufferPtr =0x%x,pbTrgBuffer = 0x%x",orgTrgBufferPtr,pbTrgBuffer);
	*(pbTrgBuffer)++ = 0xFF;
    *(pbTrgBuffer)++ = 0xD9;
    fileLength += 2;

	MP_DEBUG2("%s:4. pbTrgBuffer =0x%x", __FUNCTION__, pbTrgBuffer);
	MP_DEBUG2("%s:4. fileLength = %d", __FUNCTION__, fileLength);

	trg = orgTrgBufferPtr + JPEG_HEADER_OFFSET_X;
	*trg = tmpWin.wHeight >> 8;
	*(trg + 1) = tmpWin.wHeight;
	*(trg + 2) = tmpWin.wWidth >> 8;
	*(trg + 3) = tmpWin.wWidth;

#endif
        MP_DEBUG1("###### fileLength =%d",fileLength);
	return (fileLength);	
}






/*--------------------------------------------------------------------------*/

#if OPEN_EXIF
/*Frank Lin add*/
/*Exif add into JPEG APP1 Tag.*/
//Frank Lin add





DWORD EncodeSubIFD(BYTE *trg_in, ST_EXIF *psEXIF, DWORD EXIFStart)
{


    MP_DEBUG2("EncodeSubIFD trg_in=0x%x,EXIFStart=0x%x",trg_in,EXIFStart);

	MP_DEBUG("psEXIF->SubIFDTag[0]=0x%4x",psEXIF->SubIFDTag[0]);



    DWORD SubIFDCountByte=0;

	DWORD i;
	//WORD dwJohn, dwMary;
	DWORD SubIFDEncodeCountNum=0;

    BYTE  *NoOfSubIFD0Addr;
	DWORD SubIFDStart;
	BYTE  *SubIFDValue;
	BYTE  *ZeroIFDValue;
    BYTE  *trg=trg_in;

	DWORD OffsetTemp=NULL;
	DWORD GetValue;
	WORD  SubIFDTagTemp;
	WORD  SubIFDTypeTemp;
	DWORD SubIFDCountTemp;
	DWORD SubIFDValueTemp;

	BYTE *GetASCIIChar;
	DWORD Valuei;
	DWORD Gi;
	DWORD MakerNoteCountByte = 0;
	


    SubIFDCountByte=0;
	SubIFDStart=trg_in;
	NoOfSubIFD0Addr=SubIFDStart;
	
	*trg=0xff;  trg++;/*num of SubIFD TAG ,update later*/
	*trg=0xff;  trg++;/*update later*/


    MP_DEBUG("----------SubIFD -----------------------");
	MP_DEBUG1("psEXIF->NoOfSubIFD=%d",psEXIF->NoOfSubIFD);

	SubIFDValue=SubIFDStart+2+(12* psEXIF->NoOfSubIFD)+4;/*2bytes=num of SubIFD TAG ,4=Last IFD End(0x00_00_00_00)*/

   
    BYTE *trg_IFD=NULL;
    BYTE *trg_Value=NULL;
    trg_IFD=trg;
    trg_Value=SubIFDValue;

	
	for(i=0 ; i<psEXIF->NoOfSubIFD; i++)
    {
       SubIFDTypeTemp=psEXIF->SubIFDType[i];
   
	   switch(SubIFDTypeTemp)
	   {
         //trg_IFD+=12
	     //trg_Value++

		case EXIF_TYPE_USIGNED_BYTE:
			 SubIFDTagTemp=psEXIF->SubIFDTag[i];
             MP_DEBUG2("Encode EXIF[%d] TAG=0x%4x",i,SubIFDTagTemp);
			
		     *trg_IFD    =(BYTE)((SubIFDTagTemp & 0xff00)>>8);        
             *(trg_IFD+1)=(BYTE)(SubIFDTagTemp & 0xff); 
			 *(trg_IFD+2)=0x00;        /*Type 0x0001 BYTE*/
             *(trg_IFD+3)=0x01;

			 SubIFDCountTemp=psEXIF->SubIFDCount[i];
			 MP_DEBUG1("SubIFDCountTemp=0x%x",SubIFDCountTemp);
			 *(trg_IFD+4)=(BYTE)((SubIFDCountTemp & 0xff000000)>>24);        /*Count*/
	         *(trg_IFD+5)=(BYTE)((SubIFDCountTemp & 0x00ff0000)>>16);        
             *(trg_IFD+6)=(BYTE)((SubIFDCountTemp & 0x0000ff00)>>8);         
		     *(trg_IFD+7)=(BYTE)(SubIFDCountTemp & 0x000000ff);

             SubIFDValueTemp=psEXIF->SubIFDValue1[i][0];
			 *(trg_IFD+8)=(BYTE)(SubIFDValueTemp & 0xff);        /*value */
             *(trg_IFD+9)= 0x00;         
             *(trg_IFD+10)=0x00;       /*padding*/
             *(trg_IFD+11)=0x00;
			 
			 trg_IFD+=12;
		     SubIFDEncodeCountNum++;
		break;

		case EXIF_TYPE_ASCII_STRING:
			 SubIFDTagTemp=psEXIF->SubIFDTag[i];
             MP_DEBUG2("Encode EXIF[%d] TAG=0x%4x",i,SubIFDTagTemp);
			
		     *trg_IFD    =(BYTE)((SubIFDTagTemp & 0xff00)>>8);        
             *(trg_IFD+1)=(BYTE)(SubIFDTagTemp & 0xff); 
			 *(trg_IFD+2)=0x00;        /*Type 0x0002 ASCII*/
             *(trg_IFD+3)=0x02;

             SubIFDCountTemp=psEXIF->SubIFDCount[i];
			 
			 if((SubIFDCountTemp>CAMERAINFOSIZE) || (SubIFDCountTemp<=2) )
			 {
               SubIFDCountTemp=CAMERAINFOSIZE;
			 }
             *(trg_IFD+4)=(BYTE)((SubIFDCountTemp & 0xff000000)>>24);        /*Count 1*/
	         *(trg_IFD+5)=(BYTE)((SubIFDCountTemp & 0x00ff0000)>>16);         
             *(trg_IFD+6)=(BYTE)((SubIFDCountTemp & 0x0000ff00)>>8);         
		     *(trg_IFD+7)=(BYTE)(SubIFDCountTemp& 0x000000ff);

			 OffsetTemp=trg_Value-EXIFStart;
			 *(trg_IFD+8)=(BYTE)((OffsetTemp & 0xff000000)>>24);        /*Value Offset */
             *(trg_IFD+9)=(BYTE)((OffsetTemp & 0x00ff0000)>>16);          
             *(trg_IFD+10)=(BYTE)((OffsetTemp & 0x0000ff00)>>8);  
             *(trg_IFD+11)=(BYTE)(OffsetTemp & 0x000000ff);

			 MP_DEBUG2("psEXIF->string[%d]=%s",i,psEXIF->SubIFDString[i]);
			 
			 GetASCIIChar=psEXIF->SubIFDString[i];
			 for(Valuei=0;Valuei<CAMERAINFOSIZE-1;Valuei++)
             	{
			    	*trg_Value =*(GetASCIIChar+Valuei);
					trg_Value++;
             	}
			    *trg_Value = 0x00;/*add NULL */
				trg_Value++;

			 trg_IFD+=12;
		     SubIFDEncodeCountNum++;
			 
		break;
		
		case EXIF_TYPE_USIGNED_SHORT:	
             SubIFDTagTemp=psEXIF->SubIFDTag[i];
             MP_DEBUG2("##################  Encode SubIFD EXIF[%d] TAG=0x%4x",i,SubIFDTagTemp);
			
		     *trg_IFD    =(BYTE)((SubIFDTagTemp & 0xff00)>>8);        
             *(trg_IFD+1)=(BYTE)(SubIFDTagTemp & 0xff); 
			 *(trg_IFD+2)=0x00;        /*Type 0x0003 short*/
             *(trg_IFD+3)=0x03;

             SubIFDCountTemp=psEXIF->SubIFDCount[i];
			 MP_DEBUG1("################## SubIFDCountTemp=0x%x",SubIFDCountTemp);
			 *(trg_IFD+4)=(BYTE)((SubIFDCountTemp & 0xff000000)>>24);        /*Count*/
	         *(trg_IFD+5)=(BYTE)((SubIFDCountTemp & 0x00ff0000)>>16);        
             *(trg_IFD+6)=(BYTE)((SubIFDCountTemp & 0x0000ff00)>>8);         
		     *(trg_IFD+7)=(BYTE)(SubIFDCountTemp & 0x000000ff);

             SubIFDValueTemp=psEXIF->SubIFDValue1[i][0];
			 MP_DEBUG("##################SubIFDValueTemp=0x%x",SubIFDValueTemp);
			 *(trg_IFD+8)=(BYTE)((SubIFDValueTemp & 0xff00)>>8);        /*value */
             *(trg_IFD+9)=(BYTE)(SubIFDValueTemp & 0x00ff);         
             *(trg_IFD+10)=0x00;       /*padding*/
             *(trg_IFD+11)=0x00;
			
			 trg_IFD+=12;
		     SubIFDEncodeCountNum++;
		 break;

		 case EXIF_TYPE_USIGNED_LONG:
		 	SubIFDTagTemp=psEXIF->SubIFDTag[i];
			if(SubIFDTagTemp==0xa005)/*Skip ExifInteroperabilityOffset*/
				break;
             MP_DEBUG2("##################  Encode SubIFD EXIF[%d] TAG=0x%4x",i,SubIFDTagTemp);
			
		     *trg_IFD    =(BYTE)((SubIFDTagTemp & 0xff00)>>8);        
             *(trg_IFD+1)=(BYTE)(SubIFDTagTemp & 0xff); 
			 *(trg_IFD+2)=0x00;        /*Type 0x0004 long*/
             *(trg_IFD+3)=0x04;

             SubIFDCountTemp=psEXIF->SubIFDCount[i];
			 MP_DEBUG1("################## SubIFDCountTemp=0x%x",SubIFDCountTemp);
			 *(trg_IFD+4)=(BYTE)((SubIFDCountTemp & 0xff000000)>>24);        /*Count*/
	         *(trg_IFD+5)=(BYTE)((SubIFDCountTemp & 0x00ff0000)>>16);        
             *(trg_IFD+6)=(BYTE)((SubIFDCountTemp & 0x0000ff00)>>8);         
		     *(trg_IFD+7)=(BYTE)(SubIFDCountTemp & 0x000000ff);

             SubIFDValueTemp=psEXIF->SubIFDValue1[i][0];
			 MP_DEBUG("##################SubIFDValueTemp=0x%x",SubIFDValueTemp);
			 *(trg_IFD+8)=(BYTE)((SubIFDValueTemp & 0xff000000)>>24);        /*value  LONG*/
             *(trg_IFD+9)=(BYTE)((SubIFDValueTemp & 0x00ff0000)>>16);        
             *(trg_IFD+10)=(BYTE)((SubIFDValueTemp & 0x0000ff00)>>8);
             *(trg_IFD+11)=(BYTE)(SubIFDValueTemp & 0x000000ff);
			
			 trg_IFD+=12;
		     SubIFDEncodeCountNum++;
		 break;

         case EXIF_TYPE_RATIONAL:
		 	 SubIFDTagTemp=psEXIF->SubIFDTag[i];
             MP_DEBUG2("Encode EXIF[%d] TAG=0x%4x",i,SubIFDTagTemp);
			
		     *trg_IFD    =(BYTE)((SubIFDTagTemp & 0xff00)>>8);        
             *(trg_IFD+1)=(BYTE)(SubIFDTagTemp & 0xff); 
			 *(trg_IFD+2)=0x00;        /*Type 0x0005 Rational*/
             *(trg_IFD+3)=0x05;

			 SubIFDCountTemp=psEXIF->SubIFDCount[i];
			 
			 if(SubIFDCountTemp>6) 
			 {
               SubIFDCountTemp=6;
			 }
             *(trg_IFD+4)=(BYTE)((SubIFDCountTemp & 0xff000000)>>24);        /*Count 1*/
	         *(trg_IFD+5)=(BYTE)((SubIFDCountTemp & 0x00ff0000)>>16);         
             *(trg_IFD+6)=(BYTE)((SubIFDCountTemp & 0x0000ff00)>>8);         
		     *(trg_IFD+7)=(BYTE)(SubIFDCountTemp& 0x000000ff);

			 OffsetTemp=trg_Value-EXIFStart;
    	     *(trg_IFD+8)=(BYTE)((OffsetTemp & 0xff000000)>>24);        /*Value Offset */
             *(trg_IFD+9)=(BYTE)((OffsetTemp & 0x00ff0000)>>16);          
             *(trg_IFD+10)=(BYTE)((OffsetTemp & 0x0000ff00)>>8);  
             *(trg_IFD+11)=(BYTE)(OffsetTemp & 0x000000ff);

			 for(Gi=0;Gi<SubIFDCountTemp;Gi++)
             {
               GetValue=psEXIF->SubIFDValue1[i][Gi];
			   *trg_Value=(BYTE) ((GetValue & 0xff000000)>>24);    trg_Value++; 
			   *trg_Value=(BYTE) ((GetValue & 0x00ff0000)>>16);    trg_Value++; 
			   *trg_Value=(BYTE) ((GetValue & 0x0000ff00)>>8);     trg_Value++; 
			   *trg_Value=(BYTE) ( GetValue & 0x000000ff);         trg_Value++; 

			   GetValue=psEXIF->SubIFDValue2[i][Gi];
			   *trg_Value++ =(BYTE) ((GetValue & 0xff000000)>>24);   
			   *trg_Value++ =(BYTE) ((GetValue & 0x00ff0000)>>16);   
			   *trg_Value++ =(BYTE) ((GetValue & 0x0000ff00)>>8);    
			   *trg_Value++ =(BYTE) ( GetValue & 0x000000ff);
			   
             }
			  trg_IFD+=12;
			  SubIFDEncodeCountNum++;	 
		 break;

         case EXIF_TYPE_SIGNED_BYTE:
			 SubIFDTagTemp=psEXIF->SubIFDTag[i];
             MP_DEBUG2("Encode EXIF[%d] TAG=0x%4x",i,SubIFDTagTemp);
			
		     *trg_IFD    =(BYTE)((SubIFDTagTemp & 0xff00)>>8);        
             *(trg_IFD+1)=(BYTE)(SubIFDTagTemp & 0xff); 
			 *(trg_IFD+2)=0x00;        /*Type 0x0006 SIGNED_BYTE*/
             *(trg_IFD+3)=0x06;

			 SubIFDCountTemp=1;/*psEXIF->SubIFDCount[i];*/
			 MP_DEBUG1("SubIFDCountTemp=0x%x",SubIFDCountTemp);
			 *(trg_IFD+4)=(BYTE)((SubIFDCountTemp & 0xff000000)>>24);        /*Count*/
	         *(trg_IFD+5)=(BYTE)((SubIFDCountTemp & 0x00ff0000)>>16);        
             *(trg_IFD+6)=(BYTE)((SubIFDCountTemp & 0x0000ff00)>>8);         
		     *(trg_IFD+7)=(BYTE)(SubIFDCountTemp & 0x000000ff);

             SubIFDValueTemp=psEXIF->SubIFDValue1[i][0];
			 *(trg_IFD+8)=(BYTE)(SubIFDValueTemp & 0xff);        /*value */
             *(trg_IFD+9)= 0x00;         
             *(trg_IFD+10)=0x00;       /*padding*/
             *(trg_IFD+11)=0x00;
			 
			 trg_IFD+=12;
		     SubIFDEncodeCountNum++;
		break;

		 case EXIF_TYPE_UNDEFINED:
		 	SubIFDTagTemp=psEXIF->SubIFDTag[i];
			switch(SubIFDTagTemp)
			{
              case 0x9000:/*ExifVersion TAG*/	
             
               MP_DEBUG2("##################  Encode SubIFD EXIF[%d] TAG=0x%4x",i,SubIFDTagTemp);
			
		       *trg_IFD    =(BYTE)((SubIFDTagTemp & 0xff00)>>8);        
               *(trg_IFD+1)=(BYTE)(SubIFDTagTemp & 0xff); 
			   *(trg_IFD+2)=0x00;        /*Type 0x0007 undefined*/
               *(trg_IFD+3)=0x07;

               SubIFDCountTemp=4;/*psEXIF->SubIFDCount[i];*/
			   MP_DEBUG1("################## SubIFDCountTemp=0x%x",SubIFDCountTemp);
			   *(trg_IFD+4)=(BYTE)((SubIFDCountTemp & 0xff000000)>>24);        /*Count*/
	           *(trg_IFD+5)=(BYTE)((SubIFDCountTemp & 0x00ff0000)>>16);        
               *(trg_IFD+6)=(BYTE)((SubIFDCountTemp & 0x0000ff00)>>8);         
		       *(trg_IFD+7)=(BYTE)(SubIFDCountTemp & 0x000000ff);

               SubIFDValueTemp=psEXIF->SubIFDValue1[i][0];
		  	   MP_DEBUG("##################SubIFDValueTemp=0x%x",SubIFDValueTemp);
			   *(trg_IFD+8)=(BYTE)((SubIFDValueTemp & 0xff000000)>>24);        /*value 0210=V2.1 */
               *(trg_IFD+9)=(BYTE)((SubIFDValueTemp & 0x00ff0000)>>16);        
               *(trg_IFD+10)=(BYTE)((SubIFDValueTemp & 0x0000ff00)>>8);  
               *(trg_IFD+11)=(BYTE)( SubIFDValueTemp & 0x000000ff);  
			
			   trg_IFD+=12;
		       SubIFDEncodeCountNum++;
		      break;

			  case 0x9101:/*ComponentsConfiguration*/
			  	MP_DEBUG2("##################  Encode SubIFD EXIF[%d] TAG=0x%4x",i,SubIFDTagTemp);
			
		       *trg_IFD    =(BYTE)((SubIFDTagTemp & 0xff00)>>8);        
               *(trg_IFD+1)=(BYTE)(SubIFDTagTemp & 0xff); 
			   *(trg_IFD+2)=0x00;        /*Type 0x0007 undefined*/
               *(trg_IFD+3)=0x07;

               SubIFDCountTemp=4;/*psEXIF->SubIFDCount[i];*/
			   MP_DEBUG1("################## SubIFDCountTemp=0x%x",SubIFDCountTemp);
			   *(trg_IFD+4)=(BYTE)((SubIFDCountTemp & 0xff000000)>>24);        /*Count*/
	           *(trg_IFD+5)=(BYTE)((SubIFDCountTemp & 0x00ff0000)>>16);        
               *(trg_IFD+6)=(BYTE)((SubIFDCountTemp & 0x0000ff00)>>8);         
		       *(trg_IFD+7)=(BYTE)(SubIFDCountTemp & 0x000000ff);

                /*format : dose not exist=0x00; Y=0x01; Cb=0x02;Cr=0x03; Red=0x04; Green=0x05; Blue=0x06 */
               SubIFDValueTemp=psEXIF->SubIFDValue1[i][0];
		  	   MP_DEBUG("##################SubIFDValueTemp=0x%x",SubIFDValueTemp);
			   *(trg_IFD+8)=(BYTE)((SubIFDValueTemp & 0xff000000)>>24);       
               *(trg_IFD+9)=(BYTE)((SubIFDValueTemp & 0x00ff0000)>>16);        
               *(trg_IFD+10)=(BYTE)((SubIFDValueTemp & 0x0000ff00)>>8);  
               *(trg_IFD+11)=(BYTE)( SubIFDValueTemp & 0x000000ff);  
			
			   trg_IFD+=12;
		       SubIFDEncodeCountNum++;
			   break;

			  case 0xa000:/*TAG FlashPixeVersion V1.0=0100*/	
             
               MP_DEBUG2("##################  Encode SubIFD EXIF[%d] TAG=0x%4x",i,SubIFDTagTemp);
			
		       *trg_IFD    =(BYTE)((SubIFDTagTemp & 0xff00)>>8);        
               *(trg_IFD+1)=(BYTE)(SubIFDTagTemp & 0xff); 
			   *(trg_IFD+2)=0x00;        /*Type 0x0007 undefined*/
               *(trg_IFD+3)=0x07;

               SubIFDCountTemp=4;/*psEXIF->SubIFDCount[i];*/
			   MP_DEBUG1("################## SubIFDCountTemp=0x%x",SubIFDCountTemp);
			   *(trg_IFD+4)=(BYTE)((SubIFDCountTemp & 0xff000000)>>24);        /*Count*/
	           *(trg_IFD+5)=(BYTE)((SubIFDCountTemp & 0x00ff0000)>>16);        
               *(trg_IFD+6)=(BYTE)((SubIFDCountTemp & 0x0000ff00)>>8);         
		       *(trg_IFD+7)=(BYTE)(SubIFDCountTemp & 0x000000ff);

               SubIFDValueTemp=psEXIF->SubIFDValue1[i][0];
		  	   MP_DEBUG("##################SubIFDValueTemp=0x%x",SubIFDValueTemp);
			   *(trg_IFD+8)=(BYTE)((SubIFDValueTemp & 0xff000000)>>24);        /*TAG FlashPixeVersion V1.0=0100*/	
               *(trg_IFD+9)=(BYTE)((SubIFDValueTemp & 0x00ff0000)>>16);        
               *(trg_IFD+10)=(BYTE)((SubIFDValueTemp & 0x0000ff00)>>8);  
               *(trg_IFD+11)=(BYTE)( SubIFDValueTemp & 0x000000ff);  
			
			   trg_IFD+=12;
		       SubIFDEncodeCountNum++;
		      break;

			  case 0xa300:/*TAG FileSource 0xa300*/	
             
               MP_DEBUG2("##################  Encode SubIFD EXIF[%d] TAG=0x%4x",i,SubIFDTagTemp);
			
		       *trg_IFD    =(BYTE)((SubIFDTagTemp & 0xff00)>>8);        
               *(trg_IFD+1)=(BYTE)(SubIFDTagTemp & 0xff); 
			   *(trg_IFD+2)=0x00;        /*Type 0x0007 undefined*/
               *(trg_IFD+3)=0x07;

               SubIFDCountTemp=4;/*psEXIF->SubIFDCount[i];*/
			   MP_DEBUG1("################## SubIFDCountTemp=0x%x",SubIFDCountTemp);
			   *(trg_IFD+4)=(BYTE)((SubIFDCountTemp & 0xff000000)>>24);        /*Count*/
	           *(trg_IFD+5)=(BYTE)((SubIFDCountTemp & 0x00ff0000)>>16);        
               *(trg_IFD+6)=(BYTE)((SubIFDCountTemp & 0x0000ff00)>>8);         
		       *(trg_IFD+7)=(BYTE)(SubIFDCountTemp & 0x000000ff);

               SubIFDValueTemp=psEXIF->SubIFDValue1[i][0];
		  	   MP_DEBUG("##################SubIFDValueTemp=0x%x",SubIFDValueTemp);
			   *(trg_IFD+8)=(BYTE)((SubIFDValueTemp & 0xff000000)>>24);        /*TAG FlashPixeVersion V1.0=0100*/	
               *(trg_IFD+9)=(BYTE)((SubIFDValueTemp & 0x00ff0000)>>16);        
               *(trg_IFD+10)=(BYTE)((SubIFDValueTemp & 0x0000ff00)>>8);  
               *(trg_IFD+11)=(BYTE)( SubIFDValueTemp & 0x000000ff);  
			
			   trg_IFD+=12;
		       SubIFDEncodeCountNum++;
		      break;

			  case 0xa301:/*TAG SceneType 0xa301*/	
             
               MP_DEBUG2("##################  Encode SubIFD EXIF[%d] TAG=0x%4x",i,SubIFDTagTemp);
			
		       *trg_IFD    =(BYTE)((SubIFDTagTemp & 0xff00)>>8);        
               *(trg_IFD+1)=(BYTE)(SubIFDTagTemp & 0xff); 
			   *(trg_IFD+2)=0x00;        /*Type 0x0007 undefined*/
               *(trg_IFD+3)=0x07;

               SubIFDCountTemp=4;/*psEXIF->SubIFDCount[i];*/
			   MP_DEBUG1("################## SubIFDCountTemp=0x%x",SubIFDCountTemp);
			   *(trg_IFD+4)=(BYTE)((SubIFDCountTemp & 0xff000000)>>24);        /*Count*/
	           *(trg_IFD+5)=(BYTE)((SubIFDCountTemp & 0x00ff0000)>>16);        
               *(trg_IFD+6)=(BYTE)((SubIFDCountTemp & 0x0000ff00)>>8);         
		       *(trg_IFD+7)=(BYTE)(SubIFDCountTemp & 0x000000ff);

               SubIFDValueTemp=psEXIF->SubIFDValue1[i][0];
		  	   MP_DEBUG("##################SubIFDValueTemp=0x%x",SubIFDValueTemp);
			   *(trg_IFD+8)=(BYTE)((SubIFDValueTemp & 0xff000000)>>24);        /*TAG FlashPixeVersion V1.0=0100*/	
               *(trg_IFD+9)=(BYTE)((SubIFDValueTemp & 0x00ff0000)>>16);        
               *(trg_IFD+10)=(BYTE)((SubIFDValueTemp & 0x0000ff00)>>8);  
               *(trg_IFD+11)=(BYTE)( SubIFDValueTemp & 0x000000ff);  
			
			   trg_IFD+=12;
		       SubIFDEncodeCountNum++;
		      break;

#if (MPO_ENCODE == ENABLE)
  			  case 0x927c:
			  	if(psEXIF->NoOfMakerNote != 0)
			  	{
				  /*if  NoOfMakerNote !=0 : it needs to encode 0x927c TAG*/

				  *trg_IFD  =0x92;         /*TAG 0x927C*/
                  *(trg_IFD+1)=0x7C;     
		          *(trg_IFD+2)=0x00;        /*Type 0x0007 Undefined*/
                  *(trg_IFD+3)=0x07;     

			      *(trg_IFD+4)=0x00;        /*Count= 1*/
	              *(trg_IFD+5)=0x00;         
                  *(trg_IFD+6)=0x00;         
		          *(trg_IFD+7)=0x01;

			   
			      OffsetTemp=trg_Value-EXIFStart; 
							  
			      MP_DEBUG1(" OffsetTemp=0x%x",OffsetTemp);
			      *(trg_IFD+8)=(BYTE)((OffsetTemp & 0xff000000)>>24);		 /*Value Offset */
			      *(trg_IFD+9)=(BYTE)((OffsetTemp & 0x00ff0000)>>16);		   
			      *(trg_IFD+10)=(BYTE)((OffsetTemp & 0x0000ff00)>>8);  
			      *(trg_IFD+11)=(BYTE)(OffsetTemp & 0x000000ff);


				  MakerNoteCountByte = EncodeMakerNote(trg_Value, psEXIF);
			      trg_Value += MakerNoteCountByte;

			      trg_IFD+=12;
			      SubIFDEncodeCountNum++;
  			   }
              break;
#endif
			  
			  
			  default:
			  	MP_DEBUG1("Encode EXIF. Not supported TAG = 0x%4x",SubIFDTagTemp);
			}			 
		 break;

		 case EXIF_TYPE_SRATIONAL:
		 	 SubIFDTagTemp=psEXIF->SubIFDTag[i];
             MP_DEBUG2("Encode EXIF[%d] TAG=0x%4x",i,SubIFDTagTemp);
			
		     *trg_IFD    =(BYTE)((SubIFDTagTemp & 0xff00)>>8);        
             *(trg_IFD+1)=(BYTE)(SubIFDTagTemp & 0xff); 
			 *(trg_IFD+2)=0x00;        /*Type 0x000A SRational*/
             *(trg_IFD+3)=0x0A;

			 SubIFDCountTemp=psEXIF->SubIFDCount[i];
			 
			 if(SubIFDCountTemp>6) 
			 {
               SubIFDCountTemp=6;
			 }
             *(trg_IFD+4)=(BYTE)((SubIFDCountTemp & 0xff000000)>>24);        /*Count 1*/
	         *(trg_IFD+5)=(BYTE)((SubIFDCountTemp & 0x00ff0000)>>16);         
             *(trg_IFD+6)=(BYTE)((SubIFDCountTemp & 0x0000ff00)>>8);         
		     *(trg_IFD+7)=(BYTE)(SubIFDCountTemp& 0x000000ff);

			 OffsetTemp=trg_Value-EXIFStart;
    	     *(trg_IFD+8)=(BYTE)((OffsetTemp & 0xff000000)>>24);        /*Value Offset */
             *(trg_IFD+9)=(BYTE)((OffsetTemp & 0x00ff0000)>>16);          
             *(trg_IFD+10)=(BYTE)((OffsetTemp & 0x0000ff00)>>8);  
             *(trg_IFD+11)=(BYTE)(OffsetTemp & 0x000000ff);

			 for(Gi=0;Gi<SubIFDCountTemp;Gi++)
             {
               GetValue=psEXIF->SubIFDValue1[i][Gi];
			   *trg_Value=(BYTE) ((GetValue & 0xff000000)>>24);    trg_Value++; 
			   *trg_Value=(BYTE) ((GetValue & 0x00ff0000)>>16);    trg_Value++; 
			   *trg_Value=(BYTE) ((GetValue & 0x0000ff00)>>8);     trg_Value++; 
			   *trg_Value=(BYTE) ( GetValue & 0x000000ff);         trg_Value++; 

			   GetValue=psEXIF->SubIFDValue2[i][Gi];
			   *trg_Value++ =(BYTE) ((GetValue & 0xff000000)>>24);   
			   *trg_Value++ =(BYTE) ((GetValue & 0x00ff0000)>>16);   
			   *trg_Value++ =(BYTE) ((GetValue & 0x0000ff00)>>8);    
			   *trg_Value++ =(BYTE) ( GetValue & 0x000000ff);
			   
             }
			  trg_IFD+=12;
			  SubIFDEncodeCountNum++;
		 break;
		 

		 default:
		     MP_DEBUG2("####Skip SubIFD TAG[%d]=0x%4x",i,psEXIF->SubIFDTag[i]);
			 MP_DEBUG3("####Skip SubIFD type[%d]=0x4%x,SubIFDTypeTemp=0x%x",i,psEXIF->SubIFDType[i],SubIFDTypeTemp);
	   }
	}

   trg=trg_IFD;
   SubIFDValue=trg_Value;


	 /*Last IFD End*/
   *trg=0x00;         trg++;
   *trg=0x00;         trg++;
   *trg=0x00;         trg++;
   *trg=0x00;         trg++; 
     
    MP_DEBUG1("ffjjj 0910  SubIFDEncodeCountNum=%d",SubIFDEncodeCountNum);
	
    *NoOfSubIFD0Addr=(BYTE) ((SubIFDEncodeCountNum & 0xff00)>>8);
	*(NoOfSubIFD0Addr+1)=(BYTE) (SubIFDEncodeCountNum & 0x00ff);
	

    SubIFDCountByte=SubIFDValue-SubIFDStart;
	MP_DEBUG1("SubIFDCountByte=%d",SubIFDCountByte);
	MP_DEBUG("-------------------------------------");
	
    return SubIFDCountByte;
}

#endif

#if ((OPEN_EXIF == ENABLE) && (MPO_ENCODE == ENABLE))
DWORD EncodeMakerNote(BYTE *trg_in, ST_EXIF *psEXIF)
{
	DWORD MakerNoteStart;
	BYTE  *trg;
	BYTE  *trg_IFD;
	BYTE  *CountOfTagPtr;
	DWORD MakerNoteCountByte = 0;

	WORD  MakerNoteTagTemp;
	WORD  MakerNoteTypeTemp;
	DWORD MakerNoteCountTemp;
	DWORD MakerNoteValueTemp;
	int i;

	BYTE *trg_Value;
	DWORD GetValue;
	DWORD MakerNoteValue;
	DWORD OffsetTemp;

	WORD  MakerNoteEncodeNum = 0;

	trg = trg_in;
	MakerNoteStart = trg_in; 
	MP_DEBUG1("## %s ##", __FUNCTION__);
	MP_DEBUG1("MakerNoteStart =0x%x", MakerNoteStart);

    *trg = 0x46; trg++; /*F*/
	*trg = 0x55; trg++; /*U*/
	*trg = 0x4A; trg++; /*J*/
	*trg = 0x49; trg++; /*I*/
	*trg = 0x46; trg++; /*F*/
	*trg = 0x49; trg++; /*I*/
	*trg = 0x4C; trg++; /*L*/
	*trg = 0x4D; trg++; /*M*/

	/*count = 12 ,0x0000c*/
	*trg = 0x00; trg++; /*0*/
	*trg = 0x00; trg++; /*0*/
	*trg = 0x00; trg++; /*0*/
	*trg = 0x0C; trg++; /*C*/

	//MakerNoteCountByte += 12;

    CountOfTagPtr = trg;
	*trg = 0x00; trg++; /*this is the number of tag. it will be updated later.*/
	*trg = 0x00; trg++; /*this is the number of tag. it will be updated later.*/
	//MakerNoteCountByte += 2;

	trg_IFD = trg;

	trg_Value = MakerNoteStart+12+2+(12*psEXIF->NoOfMakerNote)+4;
				/*12 byte: Fujifilm000c, 2 bytes: CountOfTag, 12*CountOfTag, 4 bytes :0x00000000*/

	for(i=0 ; i<psEXIF->NoOfMakerNote; i++)
    {
		MP_DEBUG1("----  %d  ----", i);
		MakerNoteTagTemp  = psEXIF->MakerNoteTag[i];
		MakerNoteTypeTemp = psEXIF->MakerNoteType[i];

		MP_DEBUG2("- %d - MakerNoteTagTemp =0x%x", i, MakerNoteTagTemp);

		switch(MakerNoteTypeTemp)
	   {
		 case EXIF_TYPE_SRATIONAL:
		 	
			 MP_DEBUG2("Encode MakerNote[%d] TAG=0x%4x",i,MakerNoteTagTemp);
			
			 *trg_IFD    =(BYTE)((MakerNoteTagTemp & 0xff00)>>8);        
             *(trg_IFD+1)=(BYTE)(MakerNoteTagTemp & 0xff); 
			 *(trg_IFD+2)=0x00;        /*Type 0x000A SRational*/
             *(trg_IFD+3)=0x0A;

			 MakerNoteCountTemp = psEXIF->MakerNoteCount[i];
			 *(trg_IFD+4)=(BYTE)((MakerNoteCountTemp & 0xff000000)>>24);		 /*Count 1*/
			 *(trg_IFD+5)=(BYTE)((MakerNoteCountTemp & 0x00ff0000)>>16);		   
			 *(trg_IFD+6)=(BYTE)((MakerNoteCountTemp & 0x0000ff00)>>8);		  
			 *(trg_IFD+7)=(BYTE)(MakerNoteCountTemp & 0x000000ff);

			 OffsetTemp = trg_Value-MakerNoteStart;
	     	 *(trg_IFD+8)=(BYTE)((OffsetTemp & 0xff000000)>>24);		/*Value Offset */
			 *(trg_IFD+9)=(BYTE)((OffsetTemp & 0x00ff0000)>>16);		  
			 *(trg_IFD+10)=(BYTE)((OffsetTemp & 0x0000ff00)>>8);  
			 *(trg_IFD+11)=(BYTE)(OffsetTemp & 0x000000ff);

			 GetValue=psEXIF->MakerNoteValue1[i][0];
			 *trg_Value=(BYTE) ((GetValue & 0xff000000)>>24);    trg_Value++; 
			 *trg_Value=(BYTE) ((GetValue & 0x00ff0000)>>16);    trg_Value++; 
			 *trg_Value=(BYTE) ((GetValue & 0x0000ff00)>>8);     trg_Value++; 
			 *trg_Value=(BYTE) ( GetValue & 0x000000ff);         trg_Value++; 

			 GetValue=psEXIF->MakerNoteValue2[i][0];
			 *trg_Value++ =(BYTE) ((GetValue & 0xff000000)>>24);   
			 *trg_Value++ =(BYTE) ((GetValue & 0x00ff0000)>>16);   
			 *trg_Value++ =(BYTE) ((GetValue & 0x0000ff00)>>8);    
			 *trg_Value++ =(BYTE) ( GetValue & 0x000000ff);
				 
		     trg_IFD += 12;
			 MakerNoteEncodeNum ++;
		   break;
		
	     default:
		 	MP_DEBUG1("Skip not supported MakerNote TAG = 0x%x", MakerNoteTagTemp);
	   }		
	}

     trg=trg_IFD;
     MakerNoteValue=trg_Value;
	 /*Last IFD End*/
     *trg=0x00;         trg++;
     *trg=0x00;         trg++;
     *trg=0x00;         trg++;
     *trg=0x00;         trg++;
    
	*CountOfTagPtr     = (BYTE)((MakerNoteEncodeNum & 0xff00)>>8);
    *(CountOfTagPtr+1) = (BYTE)(MakerNoteEncodeNum & 0x00ff);

	MakerNoteCountByte = MakerNoteValue - MakerNoteStart;
	MP_DEBUG1("--MakerNoteCountByte =%d", MakerNoteCountByte);

	return MakerNoteCountByte;
}
#endif


#if OPEN_EXIF
DWORD JpegEncodeAPP1Exif(BYTE *trg_in, ST_EXIF *psEXIF)
{
  /*APP1:  Segment type(2bytes)+Length(2bytes)+data[(Length-2) bytes]*/

  BYTE *trg=trg_in;
   
   DWORD APP1LengthCount=0;

   BYTE *APP1Start;	
   DWORD EXIFStart;
   DWORD ZeroIFDValueStart;
   BYTE  *ZeroIFDValue;
   DWORD ZeroIFDOffset;
  
   DWORD SubIDFValueStart;
   DWORD SubIFDCountByte;
  

   BYTE  *NoOfIFD0Addr;

   WORD  ZeroIFDTag;
   WORD  ZeroIFDType;
   DWORD IFD0EncodeCountNum;
   DWORD EXIFCountByte;
   DWORD ValueCountByte;
   DWORD GetValue;

   DWORD i;
   DWORD Valuei;
   WORD dwJohn, dwMary;

   WORD  IFD0TagTemp;
   WORD  IFD0TypeTemp;
   DWORD IFD0CountTemp;
   DWORD IFD0ValueTemp;
   DWORD Gi;
   
   

   APP1Start=trg;
  
   *trg=0xff;         trg++;
   *trg=MARKER_APP1;  trg++;
   *trg=0x99;/*error value,update later*/    trg++;
   *trg=0x99;/*error value,update later*/    trg++;

   /*-------------Identifier=EXIF-----------------*/
   *trg=0x45;/*E*/    trg++;
   *trg=0x78;/*X*/    trg++;
   *trg=0x69;/*I*/    trg++;
   *trg=0x66;/*F*/    trg++;
   /*Padding 2bytes*/  
   *trg=0x00;         trg++;
   *trg=0x00;         trg++;
   APP1LengthCount=8;

   /*-------------Big endian:MM=0x4D 0x4D----EXIF Start here----*/
//   EXIFCountByte=0;/*Inital*/
   ValueCountByte=0;
   
   EXIFStart=trg;/*EXIF cout byte from here.*/
   *trg=0x4D;         trg++;/*Big endian*/
   *trg=0x4D;         trg++;
   /*0x00 0x2A*/
   *trg=0x00;         trg++;
   *trg=0x2A;         trg++;

   /*Offset of 0th IFD */
   *trg=0x00;         trg++;
   *trg=0x00;         trg++;
   *trg=0x00;         trg++;
   *trg=0x08;         trg++;

   /*------------ 0th IFD  -----------------------*/
   /*number of Directory Entries*/
   NoOfIFD0Addr=trg;
   *trg=0xff;         trg++;/*update later ,(IFD0EncodeCount & 0xff00 )>>8*/
   *trg=0xff;         trg++;/*update later ,(IFD0EncodeCount & 0xff)*/

   IFD0EncodeCountNum=0;

 
  ZeroIFDValueStart=EXIFStart+10+(12*psEXIF->NoOfZeroIFD)+4;/*10bytes =4+4+2,4=Last IFD End(0x00_00_00_00)*/


  // encode EXIF ZeroIFD
             BYTE *trg_IFD=NULL;
             BYTE *trg_Value=NULL;
			 DWORD trg_ValueAddrTemp=NULL;
			 DWORD OffsetTemp=NULL;
			 BYTE *GetASCIIChar;
             trg_IFD=trg;
             trg_Value=ZeroIFDValueStart;
			 
			 
	MP_DEBUG("------$$$ Before encode EXIF IFD-----");
	MP_DEBUG1("psEXIF->NoOfZeroIFD=%d",psEXIF->NoOfZeroIFD);
	MP_DEBUG1("IFD0EncodeCountNum=%d",IFD0EncodeCountNum);
	MP_DEBUG1("IFD base addr=0x%x",trg_IFD);
	MP_DEBUG1("Value base addr=0x%x",trg_Value);

			 
    for(i=0 ; i<psEXIF->NoOfZeroIFD ; i++)
    {

	    
         //dwJohn=psEXIF->IFD0Tag[i];
         IFD0TypeTemp=psEXIF->IFD0Type[i];
         
		 switch(IFD0TypeTemp)
		 {
		    case EXIF_TYPE_USIGNED_BYTE:
			 IFD0TagTemp=psEXIF->IFD0Tag[i];
             MP_DEBUG2("Encode EXIF[%d] TAG=0x%4x",i,IFD0TagTemp);
			
		     *trg_IFD    =(BYTE)((IFD0TagTemp & 0xff00)>>8);        
             *(trg_IFD+1)=(BYTE)(IFD0TagTemp & 0xff); 
			 *(trg_IFD+2)=0x00;        /*Type 0x0001 BYTE*/
             *(trg_IFD+3)=0x01;

			 IFD0CountTemp=psEXIF->IFD0Count[i];
			 //if(IFD0CountTemp>6) 
			// {
             //  IFD0CountTemp=6;
			 //}
			 IFD0CountTemp=1;//only one
			 MP_DEBUG1("IFD0CountTemp=0x%x",IFD0CountTemp);
			 *(trg_IFD+4)=(BYTE)((IFD0CountTemp & 0xff000000)>>24);        /*Count*/
	         *(trg_IFD+5)=(BYTE)((IFD0CountTemp & 0x00ff0000)>>16);        
             *(trg_IFD+6)=(BYTE)((IFD0CountTemp & 0x0000ff00)>>8);         
		     *(trg_IFD+7)=(BYTE)(IFD0CountTemp & 0x000000ff);

            // for(Gi=0;Gi<IFD0CountTemp;Gi++)
             //{
			 IFD0ValueTemp=psEXIF->IFD0Value1[i][0];
			 *(trg_IFD+8)=(BYTE)(IFD0ValueTemp & 0xff);        /*value */
             *(trg_IFD+9)= 0x00;         
             *(trg_IFD+10)=0x00;       /*padding*/
             *(trg_IFD+11)=0x00;
             //}

			 trg_IFD+=12;
		     IFD0EncodeCountNum++;
			break;
			 
            case EXIF_TYPE_ASCII_STRING:
		   	 IFD0TagTemp=psEXIF->IFD0Tag[i];
             MP_DEBUG2("Encode EXIF[%d] TAG=0x%4x",i,IFD0TagTemp);
			
		     *trg_IFD    =(BYTE)((IFD0TagTemp & 0xff00)>>8);        
             *(trg_IFD+1)=(BYTE)(IFD0TagTemp & 0xff); 
			 *(trg_IFD+2)=0x00;        /*Type 0x0002 ASCII*/
             *(trg_IFD+3)=0x02;

             IFD0CountTemp=psEXIF->IFD0Count[i];
			 
			 if((IFD0CountTemp>CAMERAINFOSIZE) || (IFD0CountTemp<=2) )
			 {
               IFD0CountTemp=CAMERAINFOSIZE;
			 }
             *(trg_IFD+4)=(BYTE)((IFD0CountTemp & 0xff000000)>>24);        /*Count 1*/
	         *(trg_IFD+5)=(BYTE)((IFD0CountTemp & 0x00ff0000)>>16);         
             *(trg_IFD+6)=(BYTE)((IFD0CountTemp & 0x0000ff00)>>8);         
		     *(trg_IFD+7)=(BYTE)(IFD0CountTemp& 0x000000ff);

			 OffsetTemp=trg_Value-EXIFStart;
			 *(trg_IFD+8)=(BYTE)((OffsetTemp & 0xff000000)>>24);        /*Value Offset */
             *(trg_IFD+9)=(BYTE)((OffsetTemp & 0x00ff0000)>>16);          
             *(trg_IFD+10)=(BYTE)((OffsetTemp & 0x0000ff00)>>8);  
             *(trg_IFD+11)=(BYTE)(OffsetTemp & 0x000000ff);


             MP_DEBUG2("psEXIF->string[%d]=%s",i,psEXIF->IFD0String[i]);
			 
			 GetASCIIChar=psEXIF->IFD0String[i];
			 for(Valuei=0;Valuei<CAMERAINFOSIZE-1;Valuei++)
             	{
			    	*trg_Value =*(GetASCIIChar+Valuei);
					trg_Value++;
             	}
			    *trg_Value = 0x00;/*add NULL */
				trg_Value++;

			 trg_IFD+=12;
		     IFD0EncodeCountNum++;
		   break;
		   
           case EXIF_TYPE_USIGNED_SHORT:	
             IFD0TagTemp=psEXIF->IFD0Tag[i];
             MP_DEBUG2("Encode EXIF[%d] TAG=0x%4x",i,IFD0TagTemp);
			
		     *trg_IFD    =(BYTE)((IFD0TagTemp & 0xff00)>>8);        
             *(trg_IFD+1)=(BYTE)(IFD0TagTemp & 0xff); 
			 *(trg_IFD+2)=0x00;        /*Type 0x0003 short*/
             *(trg_IFD+3)=0x03;

             IFD0CountTemp=psEXIF->IFD0Count[i];
			 MP_DEBUG1("IFD0CountTemp=0x%x",IFD0CountTemp);
			 IFD0CountTemp=1;//only one
			 *(trg_IFD+4)=(BYTE)((IFD0CountTemp & 0xff000000)>>24);        /*Count*/
	         *(trg_IFD+5)=(BYTE)((IFD0CountTemp & 0x00ff0000)>>16);        
             *(trg_IFD+6)=(BYTE)((IFD0CountTemp & 0x0000ff00)>>8);         
		     *(trg_IFD+7)=(BYTE)(IFD0CountTemp & 0x000000ff);

             IFD0ValueTemp=psEXIF->IFD0Value1[i][0];
			 *(trg_IFD+8)=(BYTE)((IFD0ValueTemp & 0xff00)>>8);        /*value */
             *(trg_IFD+9)=(BYTE)(IFD0ValueTemp & 0x00ff);         
             *(trg_IFD+10)=0x00;       /*padding*/
             *(trg_IFD+11)=0x00;
			
			 trg_IFD+=12;
		     IFD0EncodeCountNum++;
		   break;

           case EXIF_TYPE_RATIONAL:
             IFD0TagTemp=psEXIF->IFD0Tag[i];
             MP_DEBUG2("Encode EXIF[%d] TAG=0x%4x",i,IFD0TagTemp);
			
		     *trg_IFD    =(BYTE)((IFD0TagTemp & 0xff00)>>8);        
             *(trg_IFD+1)=(BYTE)(IFD0TagTemp & 0xff); 
			 *(trg_IFD+2)=0x00;        /*Type 0x0005 Rational*/
             *(trg_IFD+3)=0x05;

             IFD0CountTemp=psEXIF->IFD0Count[i];
			 
			 if(IFD0CountTemp>6) 
			 {
               IFD0CountTemp=6;
			 }
             *(trg_IFD+4)=(BYTE)((IFD0CountTemp & 0xff000000)>>24);        /*Count 1*/
	         *(trg_IFD+5)=(BYTE)((IFD0CountTemp & 0x00ff0000)>>16);         
             *(trg_IFD+6)=(BYTE)((IFD0CountTemp & 0x0000ff00)>>8);         
		     *(trg_IFD+7)=(BYTE)(IFD0CountTemp& 0x000000ff);

             OffsetTemp=trg_Value-EXIFStart;
    	     *(trg_IFD+8)=(BYTE)((OffsetTemp & 0xff000000)>>24);        /*Value Offset */
             *(trg_IFD+9)=(BYTE)((OffsetTemp & 0x00ff0000)>>16);          
             *(trg_IFD+10)=(BYTE)((OffsetTemp & 0x0000ff00)>>8);  
             *(trg_IFD+11)=(BYTE)(OffsetTemp & 0x000000ff);

             for(Gi=0;Gi<IFD0CountTemp;Gi++)
             {
               GetValue=psEXIF->IFD0Value1[i][Gi];
			   *trg_Value=(BYTE) ((GetValue & 0xff000000)>>24);    trg_Value++; 
			   *trg_Value=(BYTE) ((GetValue & 0x00ff0000)>>16);    trg_Value++; 
			   *trg_Value=(BYTE) ((GetValue & 0x0000ff00)>>8);     trg_Value++; 
			   *trg_Value=(BYTE) ( GetValue & 0x000000ff);         trg_Value++; 

			   GetValue=psEXIF->IFD0Value2[i][Gi];
			   *trg_Value++ =(BYTE) ((GetValue & 0xff000000)>>24);   
			   *trg_Value++ =(BYTE) ((GetValue & 0x00ff0000)>>16);   
			   *trg_Value++ =(BYTE) ((GetValue & 0x0000ff00)>>8);    
			   *trg_Value++ =(BYTE) ( GetValue & 0x000000ff);
			   
             }
			 
			 trg_IFD+=12;
			 IFD0EncodeCountNum++;	 
 		   break;


		   case EXIF_TYPE_USIGNED_LONG:
		   	 IFD0TagTemp=psEXIF->IFD0Tag[i];
             MP_DEBUG2("Encode EXIF[%d] TAG=0x%4x",i,IFD0TagTemp);
		   	 if(IFD0TagTemp==0x8769)/*Check if ExifOffset TAG or not*/
		   	 {
		   	   *trg_IFD    =0x87;         /*TAG 0x010e*/
               *(trg_IFD+1)=0x69;     
		       *(trg_IFD+2)=0x00;        /*Type 0x0004 unsigned long*/
               *(trg_IFD+3)=0x04;     

			   *(trg_IFD+4)=0x00;        /*Count= 1*/
	           *(trg_IFD+5)=0x00;         
               *(trg_IFD+6)=0x00;         
		       *(trg_IFD+7)=0x01;

			   OffsetTemp=trg_Value-EXIFStart; 
			   
			   MP_DEBUG1("ffffffffff OffsetTemp=0x%x",OffsetTemp);
               *(trg_IFD+8)=(BYTE)((OffsetTemp & 0xff000000)>>24);        /*Value Offset */
               *(trg_IFD+9)=(BYTE)((OffsetTemp & 0x00ff0000)>>16);          
               *(trg_IFD+10)=(BYTE)((OffsetTemp & 0x0000ff00)>>8);  
               *(trg_IFD+11)=(BYTE)(OffsetTemp & 0x000000ff);


               SubIFDCountByte=EncodeSubIFD(trg_Value, psEXIF, EXIFStart);
			 
               trg_Value+=SubIFDCountByte;

			   trg_IFD+=12;
			   IFD0EncodeCountNum++;
		   	   
		   	 }
		   	 
		   break;

                 	   
		   default:
		     MP_DEBUG2("####Skip TAG[%d]=0x%4x",i,IFD0TypeTemp);
		   	
		 }    
	

    }

	trg=trg_IFD;
	ZeroIFDValue=trg_Value;
    MP_DEBUG("-------- After Encode EXIF -----------");
	MP_DEBUG1("IFD0EncodeCountNum=%d",IFD0EncodeCountNum);
	MP_DEBUG1("trg_IFD addr=0x%x", trg_IFD);
	MP_DEBUG1("trg_Value addr =0x%x",trg_Value);

	



   #if 0 //Frank Lin Tag ok sample
     IFD0EncodeCountNum=1;
    /*------TAG---------------------------*/
    *trg=0x01;         trg++;/*TAG 0x0112*/
    *trg=0x12;         trg++;
	*trg=0x00;         trg++;/*Type 0x0003 short*/
    *trg=0x03;         trg++;

    *trg=0x00;         trg++;/*Count 1*/
	*trg=0x00;         trg++;
    *trg=0x00;         trg++;
    *trg=0x01;         trg++;

    *trg=0x00;         trg++;/*TAG 0x0213*/
    *trg=0x03;         trg++;
	*trg=0x00;         trg++;/*padding*/
    *trg=0x00;         trg++;
	EXIFCountByte+=12;
	/*---------------------------------*/
	#endif

   /*Last IFD End*/
   *trg=0x00;         trg++;
   *trg=0x00;         trg++;
   *trg=0x00;         trg++;
   *trg=0x00;         trg++;
  // EXIFCountByte+=4;



EXIFCountByte=ZeroIFDValue-EXIFStart;
MP_DEBUG1("Total EXIFCountByte=%d",EXIFCountByte);






  
//#error frank lin here 0908
   /*---------update the lengh value into APP1-------*/
   //APP1LengthCount
   
     APP1LengthCount += EXIFCountByte;

MP_DEBUG1("before APP1LengthCount=0x%x",APP1LengthCount);

   *(APP1Start+2)=(BYTE)((APP1LengthCount & 0xff00)>>8);/*update length*/
   *(APP1Start+3)=(BYTE)(APP1LengthCount & 0xff);/*update length */

   MP_DEBUG2("*(APP1Start+2)=0x%x, *(APP1Start+3)=0x%x",*(APP1Start+2), *(APP1Start+3));

   MP_DEBUG2("APP1LengthCount=%d,EXIFCountByte=%d",APP1LengthCount,EXIFCountByte);
   /*---------update the number of ZeroIFD ----------*/
   //IFD0EncodeCountNum
   MP_DEBUG1("ffjjj 0909  IFD0EncodeCountNum=%d",IFD0EncodeCountNum);
   *NoOfIFD0Addr=(BYTE) (IFD0EncodeCountNum & 0xff00)>>8;
   *(NoOfIFD0Addr+1)=(BYTE) (IFD0EncodeCountNum & 0xff);
   	
   
   
  MP_DEBUG1("ffff=====0x%x",trg);

  return (APP1LengthCount+2);/*add 2 byte(APP1 Marker)*/
}

/*MARKER_APP1*/

#endif//End EXIF



/*--------------------------------------------------------------------*/
//Add by Frank Lin
//Frank Lin for Encode with EXIF


#if OPEN_EXIF
DWORD Img2JpegWithEXIF(BYTE *pbTrgBuffer, ST_IMGWIN *pWin,ST_EXIF *psEXIF)
{
	return Img2JpegWithEXIF_WithQTable( pbTrgBuffer, pWin, psEXIF, Standard_QT_NUM);
}


DWORD Img2JpegWithEXIF_WithQTable(BYTE *const pbOrgTrgBuffer, ST_IMGWIN *pWin,ST_EXIF *psEXIF, BYTE bQualityTable)
{
	/*Add by Frank Lin ,for EXIF*/
	DWORD ExifAddLength=0;
		
	DWORD fileLength = 0;
	BYTE * trg, * src;
	DWORD count;
	DWORD ret;
	ST_IMGWIN tmpWin;
	BYTE bQualityTable_set;
	BYTE *orgTrgBufferPtr;
	BYTE *pbTrgBuffer;
 
#if JPEG_ENCODE
	orgTrgBufferPtr = pbOrgTrgBuffer;
	pbTrgBuffer     = pbOrgTrgBuffer;
	
	if((bQualityTable <= Standard_QT_NUM) && ((bQualityTable >= 0)))
	{
		bQualityTable_set = bQualityTable;
	}
	else
	{
		bQualityTable_set = Standard_QT_NUM;
		MP_ALERT("-E- %s:Wrong bQualityTable. bQualityTable = %d", __FUNCTION__, bQualityTable);
		MP_ALERT("-E- %s: Sellect QT num =%d", __FUNCTION__, bQualityTable_set);
		
	}

	
    tmpWin.pdwStart = pWin->pdwStart;
	tmpWin.dwOffset = pWin->dwOffset;
	tmpWin.wWidth = pWin->wWidth & 0xfff0;			// make sure the width is a multiple of 16
	tmpWin.wHeight = pWin->wHeight & 0xfff8;		// make sure the height is a multiple of 8

 	memcpy((BYTE *)(bJpegHeader + 6), &StandardQt[bQualityTable_set][0], 130);              

	///////////////////////////////////////////////////////////
    //
    // fill the header to target buffer
    //
    ///////////////////////////////////////////////////////////
    memcpy(pbTrgBuffer, (BYTE *) &bJpegHeader, JPEG_HEADER_LENGTH - JPEG_HEARDER_SOS_LENGTH);
    memcpy(pbTrgBuffer + 6, &StandardQt[bQualityTable_set][0], 130);

    trg = pbTrgBuffer + JPEG_HEADER_OFFSET_X;

    *trg = tmpWin.wHeight >> 8;
    *(trg + 1) = tmpWin.wHeight;
    *(trg + 2) = tmpWin.wWidth >> 8;
    *(trg + 3) = tmpWin.wWidth;

    fileLength = JPEG_HEADER_LENGTH - JPEG_HEARDER_SOS_LENGTH;
    pbTrgBuffer += JPEG_HEADER_LENGTH - JPEG_HEARDER_SOS_LENGTH;

	///////////////////////////////////////////////////////////
	//
	// fill the EXIF header to target buffer
	//
	///////////////////////////////////////////////////////////

    ExifAddLength = JpegEncodeAPP1Exif(pbTrgBuffer, psEXIF);
	pbTrgBuffer   += ExifAddLength;
	fileLength    += ExifAddLength;

	///////////////////////////////////////////////////////////
    //
    // SOS marker
    //
    ///////////////////////////////////////////////////////////
    memcpy(pbTrgBuffer, (BYTE *) &bJpegHeader[JPEG_HEADER_LENGTH - JPEG_HEARDER_SOS_LENGTH], JPEG_HEARDER_SOS_LENGTH);
    pbTrgBuffer += JPEG_HEARDER_SOS_LENGTH;
    fileLength += JPEG_HEARDER_SOS_LENGTH;
   
	///////////////////////////////////////////////////////////
    //
    //  Encode JPEG
    //
    ///////////////////////////////////////////////////////////
	
    ret = JpegEncode(pbTrgBuffer, &tmpWin, bQualityTable_set);

	if (!ret)
	  {
		mpDebugPrint("Encode JPEG fail !!!");
		
		return 0;
	  }
   	pbTrgBuffer += ret;
   	fileLength += ret;

    MP_DEBUG2("#### orgTrgBufferPtr =0x%x,pbTrgBuffer = 0x%x",orgTrgBufferPtr,pbTrgBuffer);
	*(pbTrgBuffer)++ = 0xFF;
    *(pbTrgBuffer)++ = 0xD9;
    fileLength += 2;


	trg = orgTrgBufferPtr + JPEG_HEADER_OFFSET_X;
	*trg = tmpWin.wHeight >> 8;
	*(trg + 1) = tmpWin.wHeight;
	*(trg + 2) = tmpWin.wWidth >> 8;
	*(trg + 3) = tmpWin.wWidth;

#endif
	return (fileLength);
	  }

#endif




	
	


/*---------------------------------------------------------------------------------*/


#pragma alignvar(4)
volatile CHAIN stNoteChain;
static void BinToAscii(DWORD dwValue, BYTE *pbBuffer, BYTE bLeng)
{
#if JPEG_ENCODE

	BYTE i;
	DWORD dwTemp;

	dwTemp = dwValue;
	for(i=bLeng; i>0; i--)
	{
		if(dwTemp%10)
			*(pbBuffer + i - 1) = 0x30 + dwTemp%10;
		else
			*(pbBuffer + i - 1) = 0x30;

		dwTemp = dwTemp/10;
	}
#endif

}



SWORD SaveBuf2File(BYTE *pbBufffer, DWORD dwFileSize, STREAM * stSourceHandle, BYTE bTrgDrvID)
{
#if JPEG_ENCODE
	DRIVE *pTrgdrv;
	FDB sNode;
	BYTE i;
	DWORD contT, dwTempLNC;
	WORD *wTmpFCLName;

	FDB *FCnode;
	DWORD dwLNameChged;
	FDB sTempFnode;

	if (!SystemCardPresentCheck(bTrgDrvID))
	{
		MP_ALERT("%s: -E- card not present !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}

	pTrgdrv = DriveChange(bTrgDrvID);

	if (DirReset(pTrgdrv) == FS_SUCCEED)
	{
		if (DirFirst(pTrgdrv) != ABNORMAL_STATUS)
		{
			//get file node
			FCnode = &sTempFnode;
			dwTempLNC = stSourceHandle->Drv->CacheBufPoint->LongNameFdbCount;
			memcpy((BYTE *) FCnode, (BYTE *) stSourceHandle->Drv->Node, sizeof(FDB));

			wTmpFCLName = (WORD *) ext_mem_malloc(MAX_L_NAME_LENG * 2);
			if (wTmpFCLName == NULL)
			{
				MP_ALERT("%s: -E- malloc fail", __FUNCTION__);
				return ABNORMAL_STATUS;
			}
			memset((BYTE *) wTmpFCLName, 0, MAX_L_NAME_LENG * 2);

			dwLNameChged = 0;
			if (dwTempLNC)
				BuildNewLName(pTrgdrv, (DRIVE *) stSourceHandle->Drv, wTmpFCLName, &dwLNameChged);

// if long name changed, get the new short name
// because we only add "COPY(XX)_" to long name, and they are all ASCII code
// so, only get the low byte data
			if (dwLNameChged)
			{
				for (i = 0; i < 6; i++)
					FCnode->Name[i] = (BYTE) wTmpFCLName[i];

				FCnode->Name[6] = 0x7e;	// " ~ "
				FCnode->Name[7] = 0x31;	// " 1 "
			}

			BuildNewFDB(pTrgdrv, FCnode);

			stSourceHandle->Drv->CacheBufPoint->LongNameFdbCount = dwTempLNC;
			if (stSourceHandle->Drv->CacheBufPoint->LongNameFdbCount)
			{
				if (LongNameCopy(pTrgdrv, wTmpFCLName, FCnode) != FS_SUCCEED)
				{
					MP_ALERT("%s: LongNameCopy() failed !", __FUNCTION__);
					ext_mem_free(wTmpFCLName);
					return ABNORMAL_STATUS;
				}
			}

			ext_mem_free(wTmpFCLName);

			dwFileSize = (dwFileSize + 511) & 0xfffffe00;
			FCnode->Size = LoadAlien32(&dwFileSize);
			FCnode->Attribute = FDB_ARCHIVE;

			if (FdbCopy(pTrgdrv, FCnode) != FS_SUCCEED)
			{
				MP_ALERT("%s: FdbCopy() failed !", __FUNCTION__);
				return ABNORMAL_STATUS;
			}

			contT = DriveNewClusGet(pTrgdrv);                 // get a new cluster for this new chain
			if (contT == 0xffffffff)
			{
				MP_ALERT("%s: DriveNewClusGet() failed => disk full !", __FUNCTION__);
				return DISK_FULL;
			}

			if (pTrgdrv->FatWrite(pTrgdrv, contT, 0xffffffff) != FS_SUCCEED)  // set chain terminator
			{
				MP_ALERT("%s: pTrgdrv->FatWrite() failed !", __FUNCTION__);
				return ABNORMAL_STATUS;
			}

	#if FS_REENTRANT_API
			/* a free cluster is allocated now => force to write FAT cache and FSInfo to device right away !
			 * note: this is important to sync FAT and free cluster info for concurrent multiple File I/O API invoking case.
			 */
			if (DriveRefresh(pTrgdrv) != FS_SUCCEED)
			{
				MP_ALERT("%s: DriveRefresh() failed !", __FUNCTION__);
				return ABNORMAL_STATUS;
			}
	#endif

			ChainInit((CHAIN *)(&stNoteChain.Start), contT, 0);       // initialize the new chain

			// extend the new chain to expected size
			if (ChainChangeSize(pTrgdrv, (CHAIN *)(&stNoteChain.Start), dwFileSize) != FS_SUCCEED)
			{
				MP_ALERT("%s: ChainChangeSize() failed => disk full !", __FUNCTION__);
				return DISK_FULL;
			}

			// reset chains point
			ChainSeekSet((CHAIN *)(&stNoteChain.Start));

			contT = dwFileSize >> 9;

			if (ChainWrite(pTrgdrv, (CHAIN *)(&stNoteChain.Start), pbBufffer, contT) != FS_SUCCEED)
			{
				MP_ALERT("%s: ChainWrite() failed !", __FUNCTION__);
				return ABNORMAL_STATUS;
			}

			contT = stNoteChain.Start;
			SaveAlien16((WORD *)(&pTrgdrv->Node->StartHigh), (WORD) (contT >> 16));
			SaveAlien16((WORD *)(&pTrgdrv->Node->StartLow), (WORD) (contT));
			DriveRefresh(pTrgdrv);
			return FS_SUCCEED;
		}
	}
#endif //JPEG_ENCODE
	return ABNORMAL_STATUS;
}


SWORD JpegEncodeCopy(ST_SEARCH_INFO * pSrcSearchInfo, BYTE bSrcDrvID, BYTE bTrgDrvID)
{
	DWORD dwTest, i;
	ST_IMGWIN *tWin;
	BYTE *pbBuffer;
	DRIVE * pSrcDrv;
	STREAM *pSrcHandle;
	DWORD dwLngCnt = 0;
	SWORD ret;
	ST_FILE_BROWSER *psBrowser;


	if (bSrcDrvID == bTrgDrvID)
		return FS_SUCCEED;

	if (pSrcSearchInfo == NULL)
	{
		// get current image file
		psBrowser = (ST_FILE_BROWSER *)&g_psSystemConfig->sFileBrowser;
		if (psBrowser->dwImgAndMovCurIndex >= psBrowser->dwImgAndMovTotalFile)
			return NULL;

		pSrcSearchInfo = &psBrowser->sImgAndMovFileList[psBrowser->dwImgAndMovCurIndex];
	}
	tWin = Idu_GetNextWin();

//	ret = ImageDraw(tWin, 0);
//	if (ret != PASS)
//		return ret;

	/* pdwTemp = (BYTE *)mem_malloc((pWin->wWidth* 2) * (pWin->wHeight + 16)); //(DWORD *)SystemGetMemAddr(JPEG_SOURCE_MEM_ID);
	memset(pdwTemp, 0, 512*1024);
	dwTest = 0x80000>> 2;	// clear 512KB area for jpeg data
	for (i=0; i<dwTest; i++)
	{
		*pdwTemp = 0;
		pdwTemp++;
	}
	*/
	pbBuffer = (BYTE *)ext_mem_malloc(128*1024);

	memset(pbBuffer, 0, 128*1024);
	dwTest = Img2Jpeg(pbBuffer, tWin);	

	if (!dwTest)
		return ABNORMAL_STATUS;
	pSrcDrv = DriveGet(bSrcDrvID);
	FileGetLongName(pSrcDrv, pSrcSearchInfo, (BYTE *) (&pSrcDrv->LongName[0]), &dwLngCnt, 128);
	pSrcHandle = FileListOpen(pSrcDrv, pSrcSearchInfo);
	ret = SaveBuf2File(pbBuffer, dwTest, pSrcHandle, bTrgDrvID);
	FileClose(pSrcHandle);
	DriveChange(bSrcDrvID);

	return ret;
}

#if 0//Not test copy from FAE Fuji project
#if JPEG_ENCODE_QUALITY_SELECT
WORD GetJpegTableQuality(BYTE bIndex)
{   return stQualityTable[bIndex].wQuality; }
WORD GetJpegTableMinFactor(BYTE bIndex)
{   return stQualityTable[bIndex].wMinFactor; }
WORD GetJpegTableMaxFactor(BYTE bIndex)
{   return stQualityTable[bIndex].wMaxFactor; }
WORD GetJpegTableAvrFactor(BYTE bIndex)
{   return stQualityTable[bIndex].wAvrFactor; }
#endif
#endif



#if ((MPO_ENCODE == ENABLE) && (MPO == ENABLE))

extern const DWORD EXIF_DATA_TYPE_LENGTH[EXIF_MAX_TYPES_MPO];

//////////////////////////////////////////////////////
//
// MP Extensions Index Ifd
//
//////////////////////////////////////////////////////

/*
#define DEFAULT_NO_OF_IMAGES            2


static const BYTE defaultMpfVersion[5] = {"0100"};
static const DWORD defaultNoOfImages = DEFAULT_NO_OF_IMAGES;
static const ST_MPO_ENTRY defaultMpEntry[DEFAULT_NO_OF_IMAGES] = {
    {
        0x00020002,         // Image Attribute
        0x00000000,         // Image Size
        0x00000000,         // Image Offset
        0x0000,             // Image1 Entry Number
        0x0000              // Image2 Entry Number
    },
    {
        0x00020002,         // Image Attribute
        0x00000000,         // Image Size
        0x00000000,         // Image Offset
        0x0000,             // Image1 Entry Number
        0x0000              // Image2 Entry Number
    }
};


static const ST_JPG_EXIF_TAG_INFO stDefaultMpIndexIfd[] = {
    {TAG_MPF_VERSION,       EXIF_TYPE_UNDEFINED_MPO,    4,  (BYTE *) &defaultMpfVersion, 0},
    {TAG_NUMBER_OF_IMAGES,  EXIF_TYPE_LONG_MPO,         1,  (BYTE *) &defaultNoOfImages, 0},
    {TAG_MP_ENTRY,          EXIF_TYPE_UNDEFINED_MPO,    32, (BYTE *) &defaultMpEntry, 0}
};
*/

#define DEFAULT_NO_OF_IMAGES            2
#define DEFAULT_MPF_VERSION           0x30313030   /*Version = 0100*/

static const DWORD defaultSrational[1] = {0};
 
static const ST_JPG_EXIF_TAG_INFO stDefaultMpIndexIfd[] = {
    {TAG_MPF_VERSION,       EXIF_TYPE_UNDEFINED_MPO,    4,  /* &defaultMpfVersion*/
		{
			(DEFAULT_MPF_VERSION&0xFF000000)>>24,
			(DEFAULT_MPF_VERSION&0x00FF0000)>>16,
			(DEFAULT_MPF_VERSION&0x0000FF00)>>8,
			(DEFAULT_MPF_VERSION&0x000000FF)>>0,
		},
		{0},
		{0},
	0},

#if 1
    {TAG_NUMBER_OF_IMAGES,  EXIF_TYPE_LONG_MPO,         1,  
		{
			0x00,
			0x00,
			0x00,
			0x02
		},
		{0},
		{0},
	0},
#endif	

    {TAG_MP_ENTRY,          EXIF_TYPE_UNDEFINED_MPO,    32, /*(BYTE *) &defaultMpEntry*/
		{ 
        	0x00,0x02,0x00,0x02,//0x00020002,         // Image Attribute
        	0x00,0x00,0x00,0x00,//0x00000000,         // Image Size
        	0x00,0x00,0x00,0x00,//0x00000000,         // Image Offset
        	0x00,0x00,			//0x0000,             // Image1 Entry Number
        	0x00,0x00,			//0x0000,              // Image2 Entry Number
    
        	0x00,0x02,0x00,0x02,//0x00020002,         // Image Attribute
        	0x00,0x00,0x00,0x00,//0x00000000,         // Image Size
        	0x00,0x00,0x00,0x00,//0x00000000,         // Image Offset
        	0x00,0x00,			//0x0000,             // Image1 Entry Number
        	0x00,0x00			//0x0000,              // Image2 Entry Number
    	}, 
		{0},
		{0},
	0}
			
};

static ST_JPG_EXIF_TAG_INFO *userMpIndexIfdPtr = (ST_JPG_EXIF_TAG_INFO *) &stDefaultMpIndexIfd;
static WORD userNoOfMpIndexTag = sizeof(stDefaultMpIndexIfd) / sizeof(ST_JPG_EXIF_TAG_INFO);

void JpegMpIndexIfdInstall(ST_JPG_EXIF_TAG_INFO *mpIndexIfdPtr, WORD noOfTags)
{
    userMpIndexIfdPtr = mpIndexIfdPtr;
    userNoOfMpIndexTag = noOfTags;
}

void JpegMpIndexIfdInstall_UseDefaultInfo(void)
{
    userMpIndexIfdPtr = (ST_JPG_EXIF_TAG_INFO *) &stDefaultMpIndexIfd;
    userNoOfMpIndexTag = sizeof(stDefaultMpIndexIfd) / sizeof(ST_JPG_EXIF_TAG_INFO);
}


void JpegMpIndexIfdUninstall(void)
{
    userMpIndexIfdPtr = (ST_JPG_EXIF_TAG_INFO *) &stDefaultMpIndexIfd;
    userNoOfMpIndexTag = sizeof(stDefaultMpIndexIfd) / sizeof(ST_JPG_EXIF_TAG_INFO);
}

//////////////////////////////////////////////////////
//
// MP Extensions Attr Ifd
//
//////////////////////////////////////////////////////

static const BYTE test[8] = {0,0,0,1,0x00,0x00,0x00,0x88};

static const ST_JPG_EXIF_TAG_INFO stDefaultMpAttrIfd[] = {
    // Add default table to here
};

static ST_JPG_EXIF_TAG_INFO *userMpAttrIfdPtr = (ST_JPG_EXIF_TAG_INFO *) &stDefaultMpAttrIfd;
static WORD userNoOfMpAttrTag = sizeof(stDefaultMpAttrIfd) / sizeof(ST_JPG_EXIF_TAG_INFO);

void JpegMpAttrIfdInstall(ST_JPG_EXIF_TAG_INFO *mpAttrIfdPtr, WORD noOfTags)
{
    userMpAttrIfdPtr = mpAttrIfdPtr;
    userNoOfMpAttrTag = noOfTags;
}



void JpegMpAttrIfdUninstall(void)
{
    userMpAttrIfdPtr = (ST_JPG_EXIF_TAG_INFO *) &stDefaultMpAttrIfd;
    userNoOfMpAttrTag = sizeof(stDefaultMpAttrIfd) / sizeof(ST_JPG_EXIF_TAG_INFO);
}


ST_JPG_EXIF_TAG_INFO *Decode_MpIndexIfd_ptr = NULL;
WORD g_NoOfMpoIndexIfd = 0;

DWORD MPO_Info_Allocate(void)
{
	extern BYTE bUseDecodedMPOInof_Flag; 

	Decode_MpIndexIfd_ptr = (ST_JPG_EXIF_TAG_INFO *)ext_mem_malloc(sizeof(ST_JPG_EXIF_TAG_INFO) * MAX_NUM_OF_MpIndexIfd);

	JpegMpIndexIfdInstall(Decode_MpIndexIfd_ptr, 0);
	//JpegMpAttrIfdInstall(Decode_MpIndexIfd_ptr, 3);

	bUseDecodedMPOInof_Flag = USE_DECODED_MPO_INFO;
	g_NoOfMpoIndexIfd       = 0;
}

void Set_MPO_NumOfTag(void)
{
	MP_DEBUG1("-Num of MPO Index = %d",g_NoOfMpoIndexIfd);
	userNoOfMpIndexTag = g_NoOfMpoIndexIfd;
}

void Release_MPO_NumOfTag(void)
{
	userNoOfMpIndexTag = 0;
}


DWORD MPO_Info_Free(void)
{
	extern BYTE bUseDecodedMPOInof_Flag;
	if(Decode_MpIndexIfd_ptr != NULL)
	{
	 	ext_mem_free(Decode_MpIndexIfd_ptr);
		Decode_MpIndexIfd_ptr = NULL;
	}

	JpegMpIndexIfdUninstall();
	//	JpegMpAttrIfdUninstall();

	g_NoOfMpoIndexIfd = 0;
	bUseDecodedMPOInof_Flag = USE_DEFAULT_MPO_INFO;
}



DWORD JpegEncPutMpExtensionInfo(BYTE *streamBufPtr,
                                ST_JPG_EXIF_TAG_INFO *jpgMpIndexIfdPtr, WORD noOfMpIndexTag,
                                ST_JPG_EXIF_TAG_INFO *jpgMpAttrIfdPtr, WORD noOfMpAttrTag,
                                DWORD *retMpHeaderPos, DWORD *retMpEntryValueOffset)
{
    BYTE *fp;
    WORD count;
    BYTE *mpIndexNextIfdPtr;
    BYTE *app2SavePos, *headSavePos, *saveAddrPtr;
    DWORD infoLength, offset;
    ST_JPG_EXIF_TAG_INFO *tmpJpgMpExtIfdPtr;
    DWORD tmpCount;

    MP_DEBUG("JpegEncPutMpExtensionInfo -");

	MP_DEBUG1("####--->noOfMpIndexTag =%d",noOfMpIndexTag);

    if (!jpgMpIndexIfdPtr || !noOfMpIndexTag)
    {
        mpDebugPrint("No any MP Extensions Info will be put !!!!");

        return 0;
    }

    fp = streamBufPtr;

    // APP2
    // APP2 marker
    SPUTC(fp, 0xFF);
    SPUTC(fp, 0xE2);
    // APP2 Length
    app2SavePos = fp;
    fp += 2;

    //////////////////////////////////////////////////////////
    //
    //  MP Extensions Header
    //
    //////////////////////////////////////////////////////////
    SPUTC(fp, 'M');     // Identifier
    SPUTC(fp, 'P');
    SPUTC(fp, 'F');
    SPUTC(fp, 0x00);

    // Byte Order
    headSavePos = fp;

    if (retMpHeaderPos)
    {
        *retMpHeaderPos = (DWORD) headSavePos - (DWORD) streamBufPtr;
        MP_DEBUG("MP Extensions's offset = %d", *retMpHeaderPos);
    }

    SPUTC(fp, EXIF_MOTO_ORDER >> 8);
    SPUTC(fp, EXIF_MOTO_ORDER & 0xFF);
    // TIFF ID
    SPUTC(fp, EXIF_TIFF_ID >> 8);
    SPUTC(fp, EXIF_TIFF_ID & 0xFF);
    // IFD0 offset, word boundary
    SPUTC(fp, 0x00);
    SPUTC(fp, 0x00);
    SPUTC(fp, 0x00);
    SPUTC(fp, 0x08);

    //////////////////////////////////////////////////////////
    //
    //  MP Index IFD
    //
    //////////////////////////////////////////////////////////
    // Number of MP Index IFD entry
    SPUTC(fp, noOfMpIndexTag >> 8);
    SPUTC(fp, noOfMpIndexTag & 0xFF);
    tmpJpgMpExtIfdPtr = jpgMpIndexIfdPtr;

    MP_DEBUG("Put MP Index IFD -");

    for (count = 0; count < noOfMpIndexTag; count++, tmpJpgMpExtIfdPtr++)
    {
        MP_DEBUG("\r\nMP Extensions's Tag = 0x%04X", tmpJpgMpExtIfdPtr->u16Tag);

        SPUTC(fp, tmpJpgMpExtIfdPtr->u16Tag >> 8);
        SPUTC(fp, tmpJpgMpExtIfdPtr->u16Tag & 0xFF);

        MP_DEBUG("MP Extensions's Data Type = 0x%02X", tmpJpgMpExtIfdPtr->u16DataType);
        SPUTC(fp, tmpJpgMpExtIfdPtr->u16DataType >> 8);
        SPUTC(fp, tmpJpgMpExtIfdPtr->u16DataType & 0xFF);

        MP_DEBUG("MP Extensions's Data Length = 0x%08X", tmpJpgMpExtIfdPtr->u32DataLength);
        SPUTC(fp, (tmpJpgMpExtIfdPtr->u32DataLength >> 24) & 0xFF);
        SPUTC(fp, (tmpJpgMpExtIfdPtr->u32DataLength >> 16) & 0xFF);
        SPUTC(fp, (tmpJpgMpExtIfdPtr->u32DataLength >> 8) & 0xFF);
        SPUTC(fp, tmpJpgMpExtIfdPtr->u32DataLength & 0xFF);

        if ((tmpJpgMpExtIfdPtr->u32DataLength * (DWORD) EXIF_DATA_TYPE_LENGTH[tmpJpgMpExtIfdPtr->u16DataType]) > 4)
        {
            MP_DEBUG("Marked - Unsaved position at 0x%X", fp);

            tmpJpgMpExtIfdPtr->unsavedFlag = (DWORD) fp;
            fp += 4;
        }
        else
        {
            SPUTC(fp, tmpJpgMpExtIfdPtr->u08BufferPtr[0]);
            SPUTC(fp, tmpJpgMpExtIfdPtr->u08BufferPtr[1]);
            SPUTC(fp, tmpJpgMpExtIfdPtr->u08BufferPtr[2]);
            SPUTC(fp, tmpJpgMpExtIfdPtr->u08BufferPtr[3]);
            tmpJpgMpExtIfdPtr->unsavedFlag = 0;
        }
    }

    // Next IFD offset
    mpIndexNextIfdPtr = fp;
    fp += 4;

    //////////////////////////////////////////////////////////
    //
    //  MP Index IFD Value
    //
    //////////////////////////////////////////////////////////
    tmpJpgMpExtIfdPtr = jpgMpIndexIfdPtr;

    MP_DEBUG("Put MP Index IFD Value -");

	//MP_ALERT("##########jpgMpIndexIfdPtr->Value1[0] =%d ",jpgMpIndexIfdPtr->Value1[0]);;

    for (count = 0; count < noOfMpIndexTag; count++, tmpJpgMpExtIfdPtr++)
    {
        if (tmpJpgMpExtIfdPtr->unsavedFlag != 0)
        {
            offset = (DWORD) fp - (DWORD) headSavePos;
            saveAddrPtr = (BYTE *) tmpJpgMpExtIfdPtr->unsavedFlag;

            MP_DEBUG("tmpJpgMpExtIfdPtr is 0x%X", &tmpJpgMpExtIfdPtr);
            MP_DEBUG("Unsaved position at 0x%X", tmpJpgMpExtIfdPtr->unsavedFlag);
            MP_DEBUG("Unsaved position at 0x%X", (DWORD) saveAddrPtr);
            MP_DEBUG("offset is 0x%X", offset);

            SPUTC(saveAddrPtr, (offset >> 24) & 0xFF);
            SPUTC(saveAddrPtr, (offset >> 16) & 0xFF);
            SPUTC(saveAddrPtr, (offset >> 8) & 0xFF);
            SPUTC(saveAddrPtr, offset & 0xFF);

            infoLength = (tmpJpgMpExtIfdPtr->u32DataLength * (DWORD) EXIF_DATA_TYPE_LENGTH[tmpJpgMpExtIfdPtr->u16DataType]);

            // Record position of MP Entry's value
            if ((tmpJpgMpExtIfdPtr->u16Tag == TAG_MP_ENTRY) && retMpEntryValueOffset)
            {
                MP_DEBUG("MP Entry Value's offset = %d", offset);

                *retMpEntryValueOffset = offset;
            }

            MP_DEBUG("\r\nMP Extensions's Tag = 0x%04X", tmpJpgMpExtIfdPtr->u16Tag);
            MP_DEBUG("Data Length is %d", infoLength);

            for (tmpCount = 0; tmpCount < infoLength; tmpCount++)
                SPUTC(fp, tmpJpgMpExtIfdPtr->u08BufferPtr[tmpCount]);
        }
    }

    MP_DEBUG("Put MP Next IFD -");

    if (jpgMpAttrIfdPtr && noOfMpAttrTag)
        offset = (DWORD) fp - (DWORD) headSavePos;
    else
        offset = 0;

	MP_DEBUG1("--->mpIndexNextIfdPtr =0x%x",mpIndexNextIfdPtr);
	MP_DEBUG2("%s:offset =0x%x", __FUNCTION__, offset);

    SPUTC(mpIndexNextIfdPtr, (offset >> 24) & 0xFF);
    SPUTC(mpIndexNextIfdPtr, (offset >> 16) & 0xFF);
    SPUTC(mpIndexNextIfdPtr, (offset >> 8) & 0xFF);
    SPUTC(mpIndexNextIfdPtr, offset & 0xFF);

    //////////////////////////////////////////////////////////
    //
    //  MP Attribute IFD
    //
    //////////////////////////////////////////////////////////
    MP_DEBUG("Put MP Attribute IFD -");

    if (jpgMpAttrIfdPtr && noOfMpAttrTag)
    {
        // Number of MP Attribute IFD entry
        SPUTC(fp, noOfMpAttrTag >> 8);
        SPUTC(fp, noOfMpAttrTag & 0xFF);
        tmpJpgMpExtIfdPtr = jpgMpAttrIfdPtr;

        for (count = 0; count < noOfMpAttrTag; count++, tmpJpgMpExtIfdPtr++)
        {
            SPUTC(fp, tmpJpgMpExtIfdPtr->u16Tag >> 8);
            SPUTC(fp, tmpJpgMpExtIfdPtr->u16Tag & 0xFF);
            SPUTC(fp, tmpJpgMpExtIfdPtr->u16DataType >> 8);
            SPUTC(fp, tmpJpgMpExtIfdPtr->u16DataType & 0xFF);
            SPUTC(fp, (tmpJpgMpExtIfdPtr->u32DataLength >> 24) & 0xFF);
            SPUTC(fp, (tmpJpgMpExtIfdPtr->u32DataLength >> 16) & 0xFF);
            SPUTC(fp, (tmpJpgMpExtIfdPtr->u32DataLength >> 8) & 0xFF);
            SPUTC(fp, tmpJpgMpExtIfdPtr->u32DataLength & 0xFF);

            if ((tmpJpgMpExtIfdPtr->u32DataLength * (DWORD) EXIF_DATA_TYPE_LENGTH[tmpJpgMpExtIfdPtr->u16DataType]) > 4)
            {
                tmpJpgMpExtIfdPtr->unsavedFlag = (DWORD) fp;
                fp += 4;
            }
            else
            {
                SPUTC(fp, tmpJpgMpExtIfdPtr->u08BufferPtr[0]);
                SPUTC(fp, tmpJpgMpExtIfdPtr->u08BufferPtr[1]);
                SPUTC(fp, tmpJpgMpExtIfdPtr->u08BufferPtr[2]);
                SPUTC(fp, tmpJpgMpExtIfdPtr->u08BufferPtr[3]);
                tmpJpgMpExtIfdPtr->unsavedFlag = 0;
            }
        }

        // Next IFD offset
        SPUTC(fp, 0x00);
        SPUTC(fp, 0x00);
        SPUTC(fp, 0x00);
        SPUTC(fp, 0x00);

        //////////////////////////////////////////////////////////
        //
        //  MP Attribute IFD Value
        //
        //////////////////////////////////////////////////////////
        MP_DEBUG("Put MP Attribute IFD Value -");

        tmpJpgMpExtIfdPtr = jpgMpAttrIfdPtr;

        for (count = 0; count < noOfMpAttrTag; count++, tmpJpgMpExtIfdPtr++)
        {
            if (tmpJpgMpExtIfdPtr->unsavedFlag != 0)
            {
                saveAddrPtr = (BYTE *) tmpJpgMpExtIfdPtr->unsavedFlag;
                offset = (DWORD) fp - (DWORD) headSavePos;

                SPUTC(saveAddrPtr, (offset >> 24) & 0xFF);
                SPUTC(saveAddrPtr, (offset >> 16) & 0xFF);
                SPUTC(saveAddrPtr, (offset >> 8) & 0xFF);
                SPUTC(saveAddrPtr, offset & 0xFF);

                infoLength = tmpJpgMpExtIfdPtr->u32DataLength * (DWORD) EXIF_DATA_TYPE_LENGTH[tmpJpgMpExtIfdPtr->u16DataType];

                for (tmpCount = 0; tmpCount < infoLength; tmpCount++)
                    SPUTC(fp, tmpJpgMpExtIfdPtr->u08BufferPtr[tmpCount]);
            }
        }
    }
    else
        MP_DEBUG("No MP Attribute !!!");

    //////////////////////////////////////////////////////////
    //
    //  End of MP Extensions
    //
    //////////////////////////////////////////////////////////
    infoLength = (DWORD) fp - (DWORD) app2SavePos;

    SPUTC(app2SavePos, infoLength >> 8);
    SPUTC(app2SavePos, infoLength & 0xFF);
    infoLength += 2;    // with 0xFFE2

    if (infoLength > 64000)
    {
        mpDebugPrint("MP Extensions Info exceed to 64KB !!!");

        return 0;
    }

    MP_DEBUG("streamBufPtr = 0x%08X", (DWORD) streamBufPtr);
    MP_DEBUG("end of streamBufPtr = 0x%08X", (DWORD) fp);
    MP_DEBUG("APP2 Length = %d", infoLength - 2);
    MP_DEBUG("Total APP2 Length = %d", infoLength);

    return infoLength;
}



// input  : ST_IMGWIN *pWin   : Include the information of image(start sddress, width, height and offset).
//          	BYTE *pbTrgBuffer : The target buffer.
//		ST_EXIF *psEXIF:EXIF information. If psEXIF == NULL, encode without EXIF. 
//		BYTE bQualityTable : select encode JPEG quality.
// output : The encode length, if 0 mease encode fail.
DWORD Img2Mpo(BYTE *pbTrgBufferPtrIn, DWORD dwTrgBufferLen, ST_IMGWIN *pLWin, ST_IMGWIN *pRWin, ST_EXIF *psEXIF, ST_EXIF *psEXIF_R, BYTE bQualityTable)
{
#if JPEG_ENCODE

    DWORD fileLength = 0;
    DWORD count;
    DWORD ret, appLength;
    ST_IMGWIN tmpLWin, tmpRWin;
    BYTE *orgTrgBufferPtr;
    DWORD mpHeaderPos, mpEntryValueOffset;
    BYTE *trg;

	BYTE *pbTrgBuffer;

    orgTrgBufferPtr = pbTrgBufferPtrIn;
	pbTrgBuffer     = pbTrgBufferPtrIn;

    tmpLWin.pdwStart = pLWin->pdwStart;
    tmpLWin.dwOffset = pLWin->dwOffset;
    tmpLWin.wWidth = pLWin->wWidth & 0xfff0;        // make sure the width is a multiple of 16
    tmpLWin.wHeight = pLWin->wHeight & 0xfff8;      // make sure the height is a multiple of 8

    tmpRWin.pdwStart = pRWin->pdwStart;
    tmpRWin.dwOffset = pRWin->dwOffset;
    tmpRWin.wWidth = pRWin->wWidth & 0xfff0;        // make sure the width is a multiple of 16
    tmpRWin.wHeight = pRWin->wHeight & 0xfff8;      // make sure the height is a multiple of 8

    if (dwTrgBufferLen < JPEG_HEADER_LENGTH)
    {
        mpDebugPrint("A. Exceed to target buffer length !!!");

        return 0;
    }

    ///////////////////////////////////////////////////////////
    //
    // fill the header to target buffer
    //
    ///////////////////////////////////////////////////////////

    memcpy(pbTrgBuffer, (BYTE *) &bJpegHeader, JPEG_HEADER_LENGTH - 14);
    memcpy(pbTrgBuffer + 6, &StandardQt[bQualityTable][0], 130);

    trg = pbTrgBuffer + JPEG_HEADER_OFFSET_X;

    *trg = tmpLWin.wHeight >> 8;
    *(trg + 1) = tmpLWin.wHeight;
    *(trg + 2) = tmpLWin.wWidth >> 8;
    *(trg + 3) = tmpLWin.wWidth;

    fileLength = JPEG_HEADER_LENGTH - 14;
    pbTrgBuffer += JPEG_HEADER_LENGTH - 14;

    ///////////////////////////////////////////////////////////
    //
    //  EXIF Tag
    //
    ///////////////////////////////////////////////////////////
	if(psEXIF != NULL)
	{
#if OPEN_EXIF //EXIF TAG	
		MP_DEBUG1("%s: ## MPO Encode with EXIF ##", __FUNCTION__);
		appLength = JpegEncodeAPP1Exif(pbTrgBuffer, psEXIF);

		if (appLength)
    	{
        	fileLength += appLength;

        	if (dwTrgBufferLen < fileLength)
        	{
            	MP_ALERT("%s: Exceed to target buffer length !!!", __FUNCTION__);

            	return 0;
       		}

        	pbTrgBuffer += appLength;
    	}
#else
	MP_ALERT("%s: Encode without EXIF.", __FUNCTION__);		
#endif		
	}
	else
	{
		MP_ALERT("%s: waring. psEXIF == NULL.", __FUNCTION__);
	}

    ///////////////////////////////////////////////////////////
    //
    //  MP Extensions Tag
    //
    ///////////////////////////////////////////////////////////
    appLength = JpegEncPutMpExtensionInfo(pbTrgBuffer,
                                          userMpIndexIfdPtr, userNoOfMpIndexTag,
                                          userMpAttrIfdPtr, userNoOfMpAttrTag,
                                          &mpHeaderPos, &mpEntryValueOffset);
    mpHeaderPos += fileLength;
    mpEntryValueOffset += mpHeaderPos;

    if (appLength)
    {
        fileLength += appLength;

        if (dwTrgBufferLen < fileLength)
        {
            mpDebugPrint("B1. Exceed to target buffer length !!!");

            return 0;
        }

        pbTrgBuffer += appLength;
    }
    ///////////////////////////////////////////////////////////
    //
    // SOS marker
    //
    ///////////////////////////////////////////////////////////
    memcpy(pbTrgBuffer, (BYTE *) &bJpegHeader[JPEG_HEADER_LENGTH - 14], 14);
    pbTrgBuffer += 14;
    fileLength += 14;
    ///////////////////////////////////////////////////////////
    //
    //  Encode Left FB
    //
    ///////////////////////////////////////////////////////////
    {
		MP_DEBUG1("## Encode Left Win JPEG ## bQualityTable =%d",bQualityTable);
	    ret = JpegEncode(pbTrgBuffer, &tmpLWin, bQualityTable);

        if (!ret)
        {
            mpDebugPrint("Encode L-Win fail !!!");

            return 0;
        }

        MP_DEBUG("Left Win's stream size is %d", ret);
        pbTrgBuffer += ret;
        *(pbTrgBuffer)++ = 0xFF;
        *(pbTrgBuffer)++ = 0xD9;
        ret += 2;
        fileLength += ret;

        if (dwTrgBufferLen < fileLength)
        {
            mpDebugPrint("C. Exceed to target buffer length !!!");

            return 0;
        }
    }
    ///////////////////////////////////////////////////////////
    //
    //  Write back MP Entry - Image Data Offset
    //
    ///////////////////////////////////////////////////////////
    {
        DWORD tmpImageDataOffset = fileLength - mpHeaderPos;
        DWORD tmpMpEntryValueOffset;

        MP_DEBUG("Write back MP Entry - Image Data Offset");

        // MP Entry1 - Image Data Size
        tmpMpEntryValueOffset = mpEntryValueOffset + 4;

        *(orgTrgBufferPtr + tmpMpEntryValueOffset)     = (BYTE) (ret >> 24);
        *(orgTrgBufferPtr + tmpMpEntryValueOffset + 1) = (BYTE) ((ret >> 16) & 0xFF);
        *(orgTrgBufferPtr + tmpMpEntryValueOffset + 2) = (BYTE) ((ret >> 8) & 0xFF);
        *(orgTrgBufferPtr + tmpMpEntryValueOffset + 3) = (BYTE) (ret & 0xFF);

        // MP Entry2 - Image Data Offset
        tmpMpEntryValueOffset = mpEntryValueOffset + sizeof(ST_MPO_ENTRY) + 8;

        *(orgTrgBufferPtr + tmpMpEntryValueOffset)     = (BYTE) (tmpImageDataOffset >> 24);
        *(orgTrgBufferPtr + tmpMpEntryValueOffset + 1) = (BYTE) ((tmpImageDataOffset >> 16) & 0xFF);
        *(orgTrgBufferPtr + tmpMpEntryValueOffset + 2) = (BYTE) ((tmpImageDataOffset >> 8) & 0xFF);
        *(orgTrgBufferPtr + tmpMpEntryValueOffset + 3) = (BYTE) (tmpImageDataOffset & 0xFF);
    }
    ///////////////////////////////////////////////////////////
    //
    //  Encode Right FB
    //
    ///////////////////////////////////////////////////////////
	MP_DEBUG1("## Encode Right Win JPEG ##- bQualityTable =%d",bQualityTable);

	if(psEXIF_R != NULL)
	{
		ret = Img2JpegWithEXIF_WithQTable(pbTrgBuffer, &tmpRWin, psEXIF_R, bQualityTable);
	}
	else
	{
		ret = Img2Jpeg_WithQTable( pbTrgBuffer,  &tmpRWin, bQualityTable);
	}
	
    if (!ret)
    {
        mpDebugPrint("Encode R-Win fail !!!");

        return 0;
    }

    MP_DEBUG("Right Win's stream size is %d", ret);

    fileLength += ret;
    //pbTrgBuffer += ret;

    if (dwTrgBufferLen < fileLength)
    {
        mpDebugPrint("D. Exceed to target buffer length !!!");

        return 0;
    }

    ///////////////////////////////////////////////////////////
    //
    //  Write back MP Entry2 - Image Data Size
    //
    ///////////////////////////////////////////////////////////
    {
        DWORD tmpMpEntryValueOffset;

        MP_DEBUG("Write back MP Entry2 - Image Data Size");

        tmpMpEntryValueOffset = mpEntryValueOffset + sizeof(ST_MPO_ENTRY) + 4;

		MP_DEBUG1("--> mpEntryValueOffset =0x%x",mpEntryValueOffset);
		MP_DEBUG1("--> tmpMpEntryValueOffset =0x%x",tmpMpEntryValueOffset);
		MP_DEBUG1("--> org trgbuf prt =0x%x",orgTrgBufferPtr);
		MP_DEBUG1("--> mp2 entry addr =0x%x",orgTrgBufferPtr + tmpMpEntryValueOffset);


        *(orgTrgBufferPtr + tmpMpEntryValueOffset)     = (BYTE) (ret >> 24);
        *(orgTrgBufferPtr + tmpMpEntryValueOffset + 1) = (BYTE) ((ret >> 16) & 0xFF);
        *(orgTrgBufferPtr + tmpMpEntryValueOffset + 2) = (BYTE) ((ret >> 8) & 0xFF);
        *(orgTrgBufferPtr + tmpMpEntryValueOffset + 3) = (BYTE) (ret & 0xFF);
    }
		MP_DEBUG1("----->fileLength =%d",fileLength);
    return fileLength;

#else

    return 0;

#endif      // #if JPEG_ENCODE
}


int ResizeDecBuf_toTrgBuf(ST_IMGWIN *DecBufWin,WORD wSrcImgWidth, WORD wSrcImgHeight , WORD wGetwImageType, ST_IMGWIN *TrgBufWin, WORD wSmartCopyWantWidth, WORD wSmartCopyWantHeight)
{
	int iErrorCode = PASS;

	DWORD dwGetFinalBufWidth;
	DWORD dwGetFinalBufHeight;

	DWORD wGetWantWidth,wGetWantHeight; 
	DWORD wGetSrcImgWidth,wGetSrcImgHeight;

	DWORD RatioX1000,RatioY1000,UsedRatio1000;
	DWORD wImgWidth1000;
	DWORD wImgHeight1000;
	DWORD dwGetTrgBufWidth1;
	DWORD dwGetTrgBufHeight1;

	DWORD dwTrgBufX;
	DWORD dwTrgBufY;
	DWORD dwSrcImgX;
	DWORD dwSrcImgY;
	
	DWORD dwSrcImageOwnRatio;
	BYTE  bHoriFlag;

	ST_IMGWIN *GetDecBufWin;
	ST_IMGWIN *GetTrgBufWin;

	WORD GetImgType;
	

	MP_DEBUG1("###DecBufWin->wWidth =%d",DecBufWin->wWidth);
	MP_DEBUG1("###DecBufWin->wHeight =%d",DecBufWin->wHeight);
	MP_DEBUG1("###wSrcImgWidth =%d",wSrcImgWidth);
	MP_DEBUG1("###wSrcImgHeight =%d",wSrcImgHeight);
	MP_DEBUG1("###wGetwImageType =%d",wGetwImageType);
	MP_DEBUG1("###-TrgBufWin->wWidth=%d",TrgBufWin->wWidth);
	MP_DEBUG1("###-TrgBufWin->wHeight=%d",TrgBufWin->wHeight);

	MP_DEBUG1("###-wSmartCopyWantWidth =%d",wSmartCopyWantWidth);
	MP_DEBUG1("###-wSmartCopyWantHeight =%d",wSmartCopyWantHeight);

	wGetWantWidth     = wSmartCopyWantWidth;
	wGetWantHeight    = wSmartCopyWantHeight;
	wGetSrcImgWidth   = wSrcImgWidth;
	wGetSrcImgHeight = wSrcImgHeight;
	GetDecBufWin      = DecBufWin;
	GetTrgBufWin      = TrgBufWin;
	GetImgType 		  = 422;//wGetwImageType;

	if(GetDecBufWin->pdwStart == NULL)
    {
		MP_ALERT("%s:GetDecBufWin = NULL!!",__FUNCTION__ );
		return FAIL;
    }

	if(GetTrgBufWin->pdwStart == NULL)
	{
		MP_ALERT("%s:GetTrgBufWin = NULL!!",__FUNCTION__ );
		return FAIL;
	}

	if((wGetWantWidth <=0) || (wGetWantHeight <= 0))
	{
		MP_ALERT("%s:Target Width or Heigh <= 0 ", __FUNCTION__);
		return FAIL;
	}

	if((GetDecBufWin->wWidth <= 0) || (GetDecBufWin->wHeight<= 0) || (wGetSrcImgWidth <= 0) || (wGetSrcImgHeight <= 0))
	{
		MP_ALERT("%s: Input parameter is NULL!!",__FUNCTION__);
		return FAIL;
	}
	
	 
	if(iErrorCode == PASS)
	{
		
	  /*desging: dwGetTrgBufWidth1 > dwGetTrgBufHeight1*/
	  /*|-------|*/
	  /*|             |*/
	  /*|-------|*/
	  /*here : check the dwGetTrgBufWidth1 and dwGetTrgBufHeight1*/
      if(wGetWantWidth >= wGetWantHeight)
      {
	  	dwGetTrgBufWidth1  = wGetWantWidth;
	    dwGetTrgBufHeight1 = wGetWantHeight;
		MP_DEBUG("Find wGetWantWidth > wGetWantHeight");
      }
	  else
	  {
	  	/*Find g_wTrgBufHeight > g_wTrgBufWidth. Need chage*/
	  	dwGetTrgBufWidth1  = wGetWantHeight;
	    dwGetTrgBufHeight1 = wGetWantWidth;
		MP_DEBUG("Find wGetWantHeight > wGetWantWidth.");
		MP_DEBUG("Change:dwGetTrgBufWidth1=H, dwGetTrgBufHeight1=W");
	  } 

	  
      bHoriFlag = 1;// 1=horizontal , 0=vertical
      if(wGetSrcImgWidth >= wGetSrcImgHeight)
      {
	  	bHoriFlag = 1;//horizontal
	  	dwTrgBufX = dwGetTrgBufWidth1;
	    dwTrgBufY = dwGetTrgBufHeight1;
		dwSrcImgX = wGetSrcImgWidth;
		dwSrcImgY = wGetSrcImgHeight;
      }
	  else
	  {
	  	bHoriFlag = 0;//vertical, reverse Width, and height
	  	dwTrgBufX = dwGetTrgBufHeight1;
	    dwTrgBufY = dwGetTrgBufWidth1; 
		dwSrcImgX = wGetSrcImgHeight;
		dwSrcImgY = wGetSrcImgWidth;
	  }
	  	

	  if((wGetSrcImgWidth * wGetSrcImgHeight) <= (dwTrgBufX * dwTrgBufY))
	  {
	  	MP_DEBUG1("%s: Here is small photo",__FUNCTION__);

		RatioX1000   = (dwTrgBufX * 1000) / wGetSrcImgWidth /*dwSrcImgX*/  ;
	    RatioY1000   = (dwTrgBufY * 1000) / wGetSrcImgHeight/*dwSrcImgY*/ ;

        MP_DEBUG2("Src Img %d x %d", dwSrcImgX, dwSrcImgY);
        MP_DEBUG2("Trg Buf %d x %d", dwTrgBufX, dwTrgBufY);
        MP_DEBUG1("RatioX (x1000)= %d", RatioX1000);
		MP_DEBUG1("RatioY (x1000)= %d", RatioY1000);

		if(RatioX1000 <= RatioY1000) /*small to big ,sellect smaller*/
		{
			UsedRatio1000 = RatioX1000 - 1;
		}
		else
		{
			UsedRatio1000 = RatioY1000 - 1;
		}

		
		dwGetFinalBufWidth   = (wGetSrcImgWidth * UsedRatio1000 ) / 1000;
  	    dwGetFinalBufHeight  = (wGetSrcImgHeight * UsedRatio1000) / 1000;

		MP_DEBUG2("%s: Smaller UsedRatio(x1000) = %d", __FUNCTION__ , UsedRatio1000);
		MP_DEBUG3("%s: GetFinal(not cut 16 W=%d ,H=%d)", __FUNCTION__ , dwGetFinalBufWidth, dwGetFinalBufHeight);
	
	  }
	  else /*-------------Big Photo -----------------------*/
	  {
	  	
        wImgWidth1000   = wGetSrcImgWidth  * 1000;//<<10;
	    wImgHeight1000  = wGetSrcImgHeight * 1000;//<<10;
  	
        RatioX1000   = wImgWidth1000 / dwTrgBufX;
	    RatioY1000   = wImgHeight1000 / dwTrgBufY;

        MP_DEBUG1("wImgWidth1000  = %d",wImgWidth1000);
	    MP_DEBUG1("wImgHeight1000 = %d",wImgHeight1000);
	    MP_DEBUG1("dwTrgBufX=%d",dwTrgBufX);
	    MP_DEBUG1("dwTrgBufY=%d",dwTrgBufY);
        MP_DEBUG1("RatioX *1000=%d",RatioX1000);
        MP_DEBUG1("RatioY *1000=%d",RatioY1000);
		
		if(RatioX1000 >= RatioY1000)
		{
			UsedRatio1000 = RatioX1000 + 1;
		}
		else
		{
			UsedRatio1000 = RatioY1000 + 1;
		}

        MP_DEBUG1("UsedRatio(x1000) = %d",UsedRatio1000);
		
		dwGetFinalBufWidth   = wImgWidth1000  / UsedRatio1000;
  	    dwGetFinalBufHeight  = wImgHeight1000 / UsedRatio1000;
	  }
 

	  #if 0 /*only for debug */
      /*Fit the wanted size*/
	  dwGetFinalBufWidth  = wGetWantWidth; 
	  dwGetFinalBufHeight = wGetWantHeight;
	  #endif

      dwGetFinalBufWidth  = ALIGN_CUT_16(dwGetFinalBufWidth);
	  dwGetFinalBufHeight = ALIGN_CUT_16(dwGetFinalBufHeight);

 
      MP_DEBUG1("#UsedRatio=%d",UsedRatio1000);
	  MP_DEBUG1("dwGetFinalBufWidth=%d",dwGetFinalBufWidth);
      MP_DEBUG1("dwGetFinalBufHeight=%d",dwGetFinalBufHeight);

	  GetTrgBufWin->wWidth  = (WORD)dwGetFinalBufWidth;
	  GetTrgBufWin->wHeight = (WORD)dwGetFinalBufHeight;
      GetTrgBufWin->dwOffset = (DWORD) ((GetTrgBufWin->wWidth)<<1);
	
	  MP_DEBUG1("=== GetDecBufWin->pdwStart =0x%x",GetDecBufWin->pdwStart);
	  MP_DEBUG1("=== GetDecBufWin->dwOffset =%d",GetDecBufWin->dwOffset);
	  MP_DEBUG1("=== GetDecBufWin->wWidth =%d",GetDecBufWin->wWidth);
	  MP_DEBUG1("=== GetDecBufWin->wHeight =%d",GetDecBufWin->wHeight);

	  Ipu_ImageScaling(GetDecBufWin, GetTrgBufWin, 0, 0, GetDecBufWin->wWidth, GetDecBufWin->wHeight,0, 0,
	    GetTrgBufWin->wWidth, GetTrgBufWin->wHeight, 0);
	}

return PASS;
	
}


#endif /*MPO_ENCODE*/

#endif /*#if JPEG_ENCODE*/
