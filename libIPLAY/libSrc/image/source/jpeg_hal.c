  
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0


#define CDU_650_MAX_CHASE_TARGET_WIDTH 800
/*CDU_650_MAX_CHASE_TARGET_WIDTH Follow IPU_650_MAX_SCALE_TARGET_SIZE*/

#define CDU_650_MAX_CHASE_TARGET_WIDTH_444 500
/*For JPEG 444*/


/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
//#include "image.h"
//#include "mpapi.h"
#include "Jpeg.h"
#include "taskid.h"


#define JPEG_DECODE_TIMEOUT_CNT     40000000

#define JPEG_CHASING_BUFFER_SIZE 16*1024
#define JPEG_CHASING_BUFFER_COUNT 24
#if ((CHIP_VER & 0xffff0000) == CHIP_VER_660)
#define	JPEG_SRC_BUF_SIZE_650	512*1024
#else
#define	JPEG_SRC_BUF_SIZE_650	1024*1024
#endif

//#define SW_DECODE_JPEG
#if OUTPUT_JPEG
extern BOOL DUMPMEMORY;
#endif

#if CUT_IMAGE_HEIGHT_ENALBE
static WORD st_wDecodeWinHeight=0;
#endif

#if MPO
#pragma alignvar(4)
extern ST_JPEG_AddFor_Mpo *g_psCurJpeg_Mpo;
#endif

#if (OPEN_EXIF)

#pragma alignvar(4)
ST_EXIF *g_psEXIFEnc=NULL;

#pragma alignvar(4)
FILE_EXIF_INFO_TYPE *g_psEXIF_Info_ForUI=NULL;
#endif

#pragma alignvar(4)
extern WORD g_wDecodedImageType;


#pragma alignvar(4)
ST_JPEG g_sJpeg[2];
ST_JPEG *g_psCurJpeg = &g_sJpeg[0];

#pragma alignvar(4)
ST_MPX_STREAM g_sJpegStream;

#pragma alignvar(4)
BYTE baMinTable[64], baSymbolTable[336];
WORD waBaseTable[64];

BYTE baMinPatchTable[]={
0x00,0x00,0x04,0x0A,0x1A,0x3A,0x78,0xF8,
0xF6,0xF6,0xF6,0xF4,0xF0,0xE0,0xC0,0x82,
0x00,0x00,0x02,0x0E,0x1E,0x3E,0x7E,0xFE,
0xFE,0xFE,0xFC,0xF8,0xF0,0xE0,0xC0,0x80,
0x00,0x00,0x04,0x0A,0x18,0x38,0x78,0xF6,
0xF4,0xF6,0xF6,0xF4,0xF0,0xE0,0xC2,0x88,
0x00,0x00,0x06,0x0E,0x1E,0x3E,0x7E,0xFE,
0xFE,0xFE,0xFE,0xFE,0xFC,0xF8,0xF0,0xE0
};

WORD waBasePatchTable[]={
0x0000,0x0000,0xFFFE,0xFFF9,0xFFEC,0xFFCF,0xFF93,0xFF17,
0xFE1C,0xFC21,0xF826,0xF02C,0xE034,0xC044,0x8064,0x00A3,
0x00A2,0x00A2,0x00A1,0x009A,0x008B,0x006C,0x002D,0xFFAE,
0xFEAF,0xFCB0,0xF8B2,0xF0B6,0xE0BE,0xC0CE,0x80EE,0x012E,
0x00AE,0x00AE,0x00AC,0x00A7,0x009B,0x007F,0x0043,0xFFC8,
0xFECE,0xFCD3,0xF8D8,0xF0DE,0xE0E6,0xC0F6,0x8115,0x0151,
0x00A2,0x00A2,0x009F,0x0098,0x0089,0x006A,0x002B,0xFFAC,
0xFEAD,0xFCAE,0xF8AF,0xF0B0,0xE0B2,0xC0B6,0x80BE,0x00CE
};

BYTE baSymbolPatchTable[]={
0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,
0x21,0x31,0x41,0x06,0x13,0x51,0x61,0x07,
0x22,0x71,0x14,0x32,0x81,0x91,0xA1,0x08,
0x23,0x42,0xB1,0xC1,0x15,0x52,0xD1,0xF0,
0x24,0x33,0x62,0x72,0x82,0x09,0x0A,0x16,
0x17,0x18,0x19,0x1A,0x25,0x26,0x27,0x28,
0x29,0x2A,0x34,0x35,0x36,0x37,0x38,0x39,
0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,
0x4A,0x53,0x54,0x55,0x56,0x57,0x58,0x59,
0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,
0x6A,0x73,0x74,0x75,0x76,0x77,0x78,0x79,
0x7A,0x83,0x84,0x85,0x86,0x87,0x88,0x89,
0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,
0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,
0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,
0xB7,0xB8,0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,
0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,
0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xE1,0xE2,
0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,
0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,
0xF9,0xFA,0x00,0x11,0x22,0x33,0x44,0x55,
0x66,0x77,0x88,0x99,0xAA,0xBB,0x00,0x01,
0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,
0x12,0x41,0x51,0x07,0x61,0x71,0x13,0x22,
0x32,0x81,0x08,0x14,0x42,0x91,0xA1,0xB1,
0xC1,0x09,0x23,0x33,0x52,0xF0,0x15,0x62,
0x72,0xD1,0x0A,0x16,0x24,0x34,0xE1,0x25,
0xF1,0x17,0x18,0x19,0x1A,0x26,0x27,0x28,
0x29,0x2A,0x35,0x36,0x37,0x38,0x39,0x3A,
0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,
0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,
0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,
0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,
0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,
0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,
0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,
0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,
0xB7,0xB8,0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,
0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,
0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xE2,0xE3,
0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF2,
0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA
};

typedef struct
{
//	IMAGEFILE *psImage;
//	ST_IMGWIN *psNextWin;
//	ST_MPX_STREAM *psStream;
	ST_IMGWIN sDCWin;
	ST_IMGWIN sPTWin;
	ST_IMGWIN sRAWin;
	
	BYTE * pbChasingBuffer;
	DWORD dwChasingBufferSize;
	
	BYTE * pbDChasingBuffer;
	DWORD dwDChasingBufferSize;

	BYTE * pbReArrangeBuffer;
	DWORD dwReArrangeBufferSize;	

	WORD wDCWidth;
	WORD wDCHeight;

	DWORD dwDCWidthLineOffset;

	WORD wTargetWidth;
	WORD wTargetHeight;

	DWORD dwCurrentChasingStep;
	DWORD dwTargetLineOffset;

	BYTE bDCCoEfficient;
	BYTE bDChasingStep;
	BYTE bSegmentCoEfficient;
	BYTE bDCIndex;
	
	WORD wSegmentWidth;
	WORD wSectionHeight;

	WORD wSegmentCount;
	WORD wSectionCount;

	WORD wSegmentResidue;
	WORD wSectionResidue;

	DWORD dwDCSegmentLineOffset;
	DWORD dwDCSegmentResidueOffset;
	DWORD dwDCSegmentDownSizeOffset;
	DWORD dwDCSegmentResidueDownSizeOffset;
	
	WORD wTargetSegmentWidth;
	WORD wTargetSectionHeight;

	WORD wTargetSegmentResidue;
	WORD wTargetSectionResidue;

	DWORD dwTargetSegmentOffset;
	DWORD dwTargetSegmentResidueOffset;

	WORD wCurrentSegment;
	WORD wCurrentSection;

	WORD wDCSegmentWidth;
	WORD wDCSectionHeight;

	WORD wDCSegmentResidue;
	WORD wDCSectionResidue;

	BYTE bSegmentResidueExist;
	BYTE bDCCellHeight;
	BYTE bDChasingBucket;
	BYTE Reservedbyte;
	
} DC_CONTROLLER;

//static DC_CONTROLLER g_sDC_Controller;
//static DC_CONTROLLER * gp_sDC_Controller = &g_sDC_Controller;
static DC_CONTROLLER * gp_sDC_Controller=NULL;
static BYTE CDU_Isr_Cnt = 0 ;

#if MJPEG_ENABLE
extern BYTE g_bVideoResume;
#endif

BYTE JpegDecoder444=0;
BYTE SetFixedRatio444_422Flag=0;

DC_CONTROLLER *ImageGetDC_Controller()
{
	return gp_sDC_Controller;
}

void Jpg_DC_Controller_init()
{
    gp_sDC_Controller = (DC_CONTROLLER *)ext_mem_malloc(sizeof(DC_CONTROLLER)+32); 
//	ext_mem_register( (sizeof(DC_CONTROLLER)+32), "Jpg_DC_Controller_init() in Jpeg_hal.c", gp_sDC_Controller);//byAlexWang 27apr2007 m2project
	memset(gp_sDC_Controller, 0, sizeof(DC_CONTROLLER));
}

void Jpg_DC_Controller_Free()
{
	if (gp_sDC_Controller) ext_mem_free(gp_sDC_Controller);
	gp_sDC_Controller=NULL;
}

//extern const BYTE baJpegHeader[JPEG_HEADER_LENGTH];

//BYTE CduWaitReady;

//***************************************************************************************************************//

//void SetCduWaitRdy(BYTE value)
//{
//	CduWaitReady = value;
//}


BYTE CduCheckEnabled()
{
	return (((CLOCK *) (CLOCK_BASE))->MdClken & BIT12);	//if cdu clock enable
}


void CduDecodeEnable()
{
	((CLOCK *) (CLOCK_BASE))->MdClken |= BIT12;	//cdu clock enable

	((CHANNEL *) (DMA_JVLC_BASE))->Control = 0;
	((CHANNEL *) (DMA_JMCU_BASE))->Control = 0;

	((CDU *) (CDU_BASE))->CduOp = 0;

	BIU *biu = (BIU *) (BIU_BASE);
	//biu->BiuArst &= 0xffffffef;
	//biu->BiuArst |= 0x00000010;
	biu->BiuSrst &= 0xffffffef;
	biu->BiuSrst |= 0x00000010;
}


void CduDisable()
{
	((CLOCK *) (CLOCK_BASE))->MdClken &= ~BIT12;	//cdu clock disable

	((CHANNEL *) (DMA_JVLC_BASE))->Control = 0;
	((CHANNEL *) (DMA_JMCU_BASE))->Control = 0;

	((CDU *) (CDU_BASE))->CduOp = 0;
}


//***************************************************************************************************************//

WORD JpegDelay(WORD count)
{
	volatile WORD i, j;

	count <<= 1;
	for (i = 0; i < count; i++)
	{
		for (j = 0; j < count; j++);
	}
	return j;
}

#define mSetValToTable(x) if(Clock_CpuFreqGet() == CLOCK_108M_PLL2CFG){(*dwpTableAddr = (x));__asm("nop");__asm("nop");__asm("nop");__asm("nop");}else{(*dwpTableAddr = (x));}


//--------------------------------------------------------------------------------------------
int Jpg_Marker_SOF0(ST_MPX_STREAM *psStream, ST_JPEG *psJpeg)
{
	DWORD dwJohn;
	// bypass 2 bytes - the 'frame header length'
	// bypass 1 byte -  the 'precision', assume to 8 bits	
	mpxStreamSkip(psStream, 3);

	psJpeg->wImageHeight = mpxStreamReadWord(psStream);	// get frame height
	psJpeg->wImageWidth = mpxStreamReadWord(psStream);	// get frame width		
	MP_DEBUG2("Jpg_Marker_SOF0 w %d, h %d", psJpeg->wImageWidth, psJpeg->wImageHeight);
	
	dwJohn = mpxStreamGetc(psStream);	// get 'number of component'
	
	//Griffy++
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	if(dwJohn == 1)	
	{
		mpxStreamSkip(psStream, 1);	// skip C1
		dwJohn = mpxStreamGetc(psStream);	// get H1:V1
		dwJohn = (dwJohn << 8) + mpxStreamGetc(psStream);
		if(dwJohn != 0x1100)
		{
				return NOT_SUPPORTED_VIDEO_FORMAT;
		}
		psJpeg->bMONO = 1 ;
		psJpeg->dwCduControl |= VIDEO_444;
		psJpeg->wImageType = 444;
		MP_DEBUG("MONO Y-only Photo");	
		return PASS;
	}
	else if(dwJohn == 4)
	{
		//mpDebugPrint("4-Component");
		psJpeg->bCMYK = 1 ;
		//return NOT_SUPPORTED_VIDEO_FORMAT;
	}
#endif
	//Griffy--

	if (dwJohn != 3 && dwJohn != 4)
	{
		MP_DEBUG1("number of component %d != 3", dwJohn);
		psJpeg->bProgressive |= dwJohn << 3;
		return NOT_SUPPORTED_VIDEO_FORMAT;
	}

	mpxStreamSkip(psStream, 1);	// skip C1
	dwJohn = mpxStreamGetc(psStream);	// get H1:V1
	mpxStreamSkip(psStream, 2);	// skip Tq1 and C2
	dwJohn = (dwJohn << 8) + mpxStreamGetc(psStream);	// get H2:V2
	mpxStreamSkip(psStream, 2);	// skip Tq2 and C3
	dwJohn = (dwJohn << 8) + mpxStreamGetc(psStream);	// get H3:V3

	if (dwJohn == 0x00211111)
	{
		psJpeg->dwCduControl |= VIDEO_422;
		psJpeg->wImageType = 422;
	}
	else if (dwJohn == 0x00221111)
	{
		psJpeg->dwCduControl |= VIDEO_420;
		psJpeg->wImageType = 420;
	}
	else if(dwJohn == 0x00111111)
	{
		if(psJpeg->bCMYK != 1)
		{
			psJpeg->dwCduControl |= VIDEO_444;
			psJpeg->wImageType = 444;
		}
		else
		{
			psJpeg->dwCduControl |= VIDEO_422;
			psJpeg->wImageType = 422;
		}
#if JPEG444_SW_DECODE
                MP_DEBUG3("wImageWidth =%d, wImageHeight =%d, psJpeg->bJResizeRatio =%d", psJpeg->wImageWidth, psJpeg->wImageHeight, psJpeg->bJResizeRatio);
                //MP_DEBUG4("wImageWidth= %d, wImageHeight =%d, bScaleDown =%d, iErrorCode =%d", psImage->wImageWidth, psImage->wImageHeight, psImage->bScaleDown, psImage->iErrorCode);
                if ((psJpeg->wImageWidth > 4096) && (psJpeg->wImageHeight > 4096) && ((psJpeg->bDecodeMode & 0x3f) == IMG_DECODE_PHOTO))
                    psJpeg->bProgressive = 4;
#endif
	}
	else if (dwJohn == 0x00121111 && (psJpeg->bDecodeMode & 0x3f) != IMG_DECODE_MJPEG)
	{
		//special format
		psJpeg->dwCduControl |= VIDEO_422;
		
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
		psJpeg->b422V = 1;
		psJpeg->wImageType = 422;		//422V for MP650
#else
		
		psJpeg->blNintyDegreeFlag = 1;
		psJpeg->wImageType = 422;
#endif
		
              MP_ALERT("JPEG blNintyDegreeFlag !!");
	}
	else
	{
		MP_DEBUG(" NOT_SUPPORTED_VIDEO_FORMAT");
		return NOT_SUPPORTED_VIDEO_FORMAT;	// return error code
	}
	return PASS;
}			
//--------------------------------------------------------------------------------------------
int Jpg_Marker_DHT(ST_MPX_STREAM * psStream, ST_JPEG * psJpeg)
{
	DWORD dwJohn, dwMary, dwPaul, dwCodes;
	BYTE baLength[16], bTableId;
	DWORD blCounter, blIndex;
	
	dwJohn = mpxStreamReadWord(psStream);	// get segment length
	DWORD dwEndPoint = mpxStreamTell(psStream) + dwJohn - 2; // bpsStream + dwJohn - 2;
	do
	{
		if (psStream->eof)return FAIL;
		
		bTableId = mpxStreamGetc(psStream);	// get Tc:Th
		//MP_DEBUG1("Huffmun table id %d", bTableId);
		if (bTableId == 0x00)
		{				// if first DC table
			//MP_DEBUG(" first DC table");
			dwMary = 16;	// base blIndex for MinTable and BaseTable
			dwPaul = 162;	// base blIndex for SymbolTable
			psJpeg->dc1 = 1;
		}
		else if (bTableId == 0x01)
		{				// if second DC table
			//MP_DEBUG(" second DC table");
			dwMary = 48;
			dwPaul = 162;
			psJpeg->dc2 = 1;
		}
		else if (bTableId == 0x10)
		{				// if first AC table
			//MP_DEBUG(" first AC table");
			dwMary = 0;
			dwPaul = 0;
			psJpeg->ac1 = 1;
		}
		else if (bTableId == 0x11)
		{				// if second AC table
			//MP_DEBUG(" second AC table");
			dwMary = 32;
			dwPaul = 174;
			psJpeg->ac2 = 1;
		}else if((bTableId & 0x0f) > 3)
		{
			/*bTableId : Bit[3:0]   0..3, otherwise error*/
			/*bTableId : Bit[4]      0=DC table, 1=AC table*/
			/*bTableId : Bit[5..7]  not used , must be 0*/
			MP_ALERT("%s: Invalid bTableId (Bit[3:0] > 3 )= 0x%x", __FUNCTION__, bTableId);
			return FAIL;
		}
		else if((bTableId & 0xE0) != 0)
		{
			/*bTableId : Bit[5..7]  not used , must be 0*/
			MP_ALERT("%s: Invalid bTableId = 0x%x", __FUNCTION__, bTableId);
			return FAIL;
		}
		else
		{			
			MP_DEBUG2("%s: Waring bTableId = 0x%x", __FUNCTION__, bTableId);
		}

		mpxStreamRead(baLength, 1, 16, psStream);  // get each code length

		dwCodes = 0;	// initialize huffman code
		for (blIndex = 0; blIndex < 16; blIndex++)
		{
			baMinTable[dwMary] = (BYTE) dwCodes;
			waBaseTable[dwMary] = dwPaul - dwCodes;
			dwMary++;

			for (blCounter = 0; blCounter < baLength[blIndex]; blCounter++)
			{
				dwJohn = mpxStreamGetc(psStream);	// get symbol value

				// fill to symbol table, put symbol at high nibble for DC1
				if (bTableId == 0x00)
					baSymbolTable[dwPaul] |= (dwJohn & 0x0f);
				else if (bTableId == 0x01)
					baSymbolTable[dwPaul] |= ((dwJohn & 0x0f) << 4);
				else
					baSymbolTable[dwPaul] = dwJohn;

				dwCodes++;
				dwPaul++;
			}

			dwCodes <<= 1;
		}
	}
	while (mpxStreamTell(psStream) < dwEndPoint);	// if not go the end of segment
	return PASS;		
}

//--------------------------------------------------------------------------------------------
int Jpg_Marker_DQT(ST_MPX_STREAM * psStream, ST_JPEG * psJpeg)
{
	DWORD dwJohn, dwMary, dwPaul;
	DWORD blCounter;
	DWORD *dwpTableAddr;
	
	CDU *cdu = (CDU *) (CDU_BASE);

	dwJohn = mpxStreamReadWord(psStream);	// get segment length
	dwJohn -= 2;		// dwJohn contain the valid data length
	
	while (dwJohn)
	{
		if (psStream->eof) return FAIL;
		
		dwPaul = mpxStreamGetc(psStream);
		dwMary = dwPaul & 0x0f;	// dwMary contain the table ID
		//MP_DEBUG1("MARKER_DQT id %d", dwMary);
		dwPaul >>= 4;	// dwPaul contain the precision

		if (dwMary < 2)
		{
			if (dwMary == 0) psJpeg->boWithDQ |= 0x01;  
			else if (dwMary == 1) psJpeg->boWithDQ |= 0x10;				
			
			dwpTableAddr = (DWORD *) (&cdu->CduQt[0] + (dwMary << 6));

			for (blCounter = 0; blCounter < 64; blCounter++)
			{
				if (dwPaul)
				{
					dwMary = mpxStreamReadWord(psStream);
				}		// 16-bits precision
				else
				{
					dwMary = mpxStreamGetc(psStream);
				}		// 8-bits precision

				mSetValToTable(dwMary);
				//*dwpTableAddr = dwMary;       // fill it to CDU table directly
				dwpTableAddr++;       
			}
		}
		else if (dwMary >= 2)
		{
			// 128 : 16-bits precision - 64 : 8-bits precision
			mpxStreamSkip(psStream, dwPaul ? 128 : 64);
		}

		dwJohn -= 65;
		if (dwPaul)	dwJohn -= 64;	// if 16-bits precision, subtract more 64 bytes
	}	

	return PASS;
}
//--------------------------------------------------------------------------------------------
#if OPEN_EXIF

//Frank Lin add 20100916
void GetSubIFDExif(ST_MPX_STREAM *psStream, ST_EXIF *psEXIF, DWORD EXIFOffset, DWORD SubIFDOffset, BYTE LittleEndian)
{
      /*---------Sub IFD--------------------------------------------------------------*/
      /*Frank Lin adds*/
	  //go to Sub IFD 
	  //SubIFDOffset=psEXIF->SubIFDOffset;
	  
	  /*SubIFD-----Directory Entry                                         */
	  /*          Tag    = 2bytes                                                 */
	  /*          Type  = 2bytes                                                 */
	  /*          Count = 4bytes                                                 */
	  /*          value or offset=1~4bytes(depend on Type)         */

	  DWORD dwJohn, dwMary;
      DWORD Count,ValueOffset;
      WORD NoOfInterOp,Type;
	  WORD i;
	  
	  DWORD RationalTemp;/*Frank Lin add*/
	  DWORD SubTagStart=0;

	  DWORD SubIFDCountNoOfDE=0;/*DE : Directory Entry*/
	  BYTE  GetValue[4];
	  DWORD Value,ValueTemp;
	  DWORD Gi;/*number of rational group*/


	  MP_DEBUG1("SubIFDOffset=0x%x",SubIFDOffset);
	  
		mpxStreamSeek(psStream, EXIFOffset+SubIFDOffset , SEEK_SET);
		DWORD SubIFDStart = mpxStreamTell(psStream);
		MP_DEBUG1("$$$ SubIFDStart = 0x%x", SubIFDStart);
	  
        if(LittleEndian) NoOfInterOp = mpxStreamReadWord_le(psStream);
		else NoOfInterOp = mpxStreamReadWord(psStream);
		MP_DEBUG("SubIFD NoOfInterOp = %x", NoOfInterOp);

		SubTagStart=EXIFOffset+SubIFDOffset+2;/*2Byte = Number of Directory Entries*/
		MP_DEBUG1("SubTagStart=0x%x",SubTagStart);
  

       MP_DEBUG1("fff NoOfInterOp=%d",NoOfInterOp);


		
        for(i = 0; i < NoOfInterOp; i++){
          MP_DEBUG1("SubIFD------ %d -----",i);
          if(SubIFDCountNoOfDE>NUM_OF_SubIFDTAG)
          {
            break;
           //Type=SKIP_TYPE;/*MAX = NUM_OF_IFD0TAG*/
          }
			
		  //go nth and get tag
          mpxStreamSeek(psStream, SubTagStart + i * 12, SEEK_SET);

          if(LittleEndian)
          {
            dwJohn = mpxStreamReadWord_le(psStream);
		    Type   = mpxStreamReadWord_le(psStream);
		    Count  = mpxStreamReadDWord_le(psStream);
          }
	      else
	      {
	        dwJohn = mpxStreamReadWord(psStream); 
		    Type = mpxStreamReadWord(psStream);
		    Count = mpxStreamReadDWord(psStream);
	      }			

		  MP_DEBUG1("SubIFD tag value = 0x%04x", dwJohn);
		  MP_DEBUG1("SubIFD Type=0x%x",Type);
          
		  psEXIF->SubIFDTag[SubIFDCountNoOfDE]=dwJohn;
		  psEXIF->SubIFDType[SubIFDCountNoOfDE]=Type;		  
		  
          switch(Type)
		  {
		    case EXIF_TYPE_USIGNED_BYTE:/*Frank Lin,not test*/
			  	MP_DEBUG3("*BYTE*-------BYTE type: IFD0 TAG=0x%4x,Type=%d,Count=0x%x",dwJohn,Type,Count);
				  	  mpxStreamRead(GetValue, 1, 1, psStream);
					  Value=(BYTE)(GetValue[0]);
                MP_DEBUG3("--------GetValue0=0x%x,GetValue1=0x%x,Value=0x%x",GetValue[0],GetValue[1],Value);

				  psEXIF->SubIFDCount[SubIFDCountNoOfDE]=Count;
			      psEXIF->SubIFDValue1[SubIFDCountNoOfDE][0]=Value; 
				  SubIFDCountNoOfDE++;
				  mpxStreamSkip(psStream, 3);/*Skip 3 bytes*/
			break;

		    case EXIF_TYPE_ASCII_STRING:
				
			  	  Count = (Count>CAMERAINFOSIZE)? CAMERAINFOSIZE:Count;
			      MP_DEBUG1("*ASCII* ASCII_STRING Type: Count = 0x%x", Count);
				  if(Count <= 4){ //the value is recorded, may be < 4?
					  	mpxStreamRead(psEXIF->SubIFDString[SubIFDCountNoOfDE], 1, Count, psStream);
						mpxStreamSkip(psStream, 4 - Count);
		          }
				  else{                       
						if(LittleEndian) ValueOffset = mpxStreamReadDWord_le(psStream);
						else ValueOffset = mpxStreamReadDWord(psStream);
						
						mpxStreamSeek(psStream, EXIFOffset + ValueOffset, SEEK_SET);
						mpxStreamRead(psEXIF->SubIFDString[SubIFDCountNoOfDE], 1, Count, psStream);
					  }
				 MP_DEBUG2("????? psEXIF->SubIFDType[%d]=0x%x",SubIFDCountNoOfDE,psEXIF->SubIFDType[SubIFDCountNoOfDE]);	  
				  psEXIF->SubIFDCount[SubIFDCountNoOfDE]=Count;
				  SubIFDCountNoOfDE++;
				  
			break;

            case EXIF_TYPE_USIGNED_SHORT:
				
			  	  MP_DEBUG3("*SHORT*-------Short type: IFD0 TAG=0x%4x,Type=%d,Count=0x%x",dwJohn,Type,Count);
                  if(LittleEndian) 
				  	{
				  	  mpxStreamRead(GetValue, 1, 2, psStream);
					  Value=(DWORD)((GetValue[1]<<8)+(GetValue[0]));
                  	}
				  else
				  	{
				  	  mpxStreamRead(GetValue, 1, 2, psStream);
					  Value=(DWORD)((GetValue[0]<<8)+(GetValue[1]));
				  	}
                  MP_DEBUG3("--------GetValue0=0x%x,GetValue1=0x%x,Value=0x%x",GetValue[0],GetValue[1],Value);
				  psEXIF->SubIFDCount[SubIFDCountNoOfDE]=Count;
			      psEXIF->SubIFDValue1[SubIFDCountNoOfDE][0]=Value; 
				  MP_DEBUG1("SubIFDCountNoOfDE=0x%x",SubIFDCountNoOfDE);
				  
				  SubIFDCountNoOfDE++;
				  mpxStreamSkip(psStream, 2);
			break;

			//#define USIGNED_LONG    4
			case EXIF_TYPE_USIGNED_LONG:
				  if(dwJohn==0xa005)/*Skip ExifInteroperabilibtyOffset*/
				  	  {
				  	    mpxStreamSkip(psStream, 4);//SubIFDptr += 4;
				  	    break;
				  	  }
				  
			  	  MP_DEBUG3("*SHORT*-------Short type: IFD0 TAG=0x%4x,Type=%d,Count=0x%x",dwJohn,Type,Count);
                  if(LittleEndian) 
				  	{
				  	  mpxStreamRead(GetValue, 1, 4, psStream);
					  Value=(DWORD)((GetValue[3]<<24)+(GetValue[2]<<16)+(GetValue[1]<<8)+(GetValue[0]));
                  	}
				  else
				  	{
				  	  mpxStreamRead(GetValue, 1, 4, psStream);
					  Value=(DWORD)((GetValue[0]<<24)+(GetValue[1]<<16)+(GetValue[2]<<8)+(GetValue[3]));
				  	}
                  MP_DEBUG3("--------GetValue0=0x%x,GetValue1=0x%x,Value=0x%x",GetValue[0],GetValue[1],Value);
				  psEXIF->SubIFDCount[SubIFDCountNoOfDE]=Count;
			      psEXIF->SubIFDValue1[SubIFDCountNoOfDE][0]=Value; 
				  MP_DEBUG1("SubIFDCountNoOfDE=0x%x",SubIFDCountNoOfDE);
				  
				  SubIFDCountNoOfDE++;
				  mpxStreamSkip(psStream, 2);
			break;

			  
			  
 			case EXIF_TYPE_RATIONAL:             
				MP_DEBUG("*Rational*");
			  	if(LittleEndian) ValueOffset = mpxStreamReadDWord_le(psStream);
						else ValueOffset = mpxStreamReadDWord(psStream);
                MP_DEBUG1(" ValueOffset=0x%x",ValueOffset);

                if(Count>6)/*number of rational group *2 */
					Count=6;
				psEXIF->SubIFDCount[SubIFDCountNoOfDE]=Count;
				
                for(Gi=0;Gi<(Count);Gi++)
                {
				
				  mpxStreamSeek(psStream, EXIFOffset + ValueOffset/*+(Gi<<2)*/, SEEK_SET);
					  if(LittleEndian) RationalTemp = mpxStreamReadDWord_le(psStream);
						  else RationalTemp = mpxStreamReadDWord(psStream);
				  psEXIF->SubIFDValue1[SubIFDCountNoOfDE][/*Gi*/0]=RationalTemp;

				  if(LittleEndian) RationalTemp = mpxStreamReadDWord_le(psStream);
					  	  else RationalTemp = mpxStreamReadDWord(psStream);
				  psEXIF->SubIFDValue2[SubIFDCountNoOfDE][/*Gi*/0]=RationalTemp;
                }

			  	SubIFDCountNoOfDE++;
			break;

			case EXIF_TYPE_UNDEFINED:
				
				switch(dwJohn)
				{
				  case 0x9000:/*TAG ExifVersion*/
				    ValueOffset = mpxStreamReadDWord(psStream);
					//psEXIF->SubIFDCount[SubIFDCountNoOfDE]=1;	
					psEXIF->SubIFDValue1[SubIFDCountNoOfDE][0]=ValueOffset;
					MP_DEBUG1("Exif Version=0x%4x",psEXIF->SubIFDValue1[SubIFDCountNoOfDE][0]);
				  	SubIFDCountNoOfDE++;
				  break;

				  case 0x9101:/*TAG ComponentsConfiguration*/
					ValueOffset = mpxStreamReadDWord(psStream);
					//psEXIF->SubIFDCount[SubIFDCountNoOfDE]=1;	
					psEXIF->SubIFDValue1[SubIFDCountNoOfDE][0]=ValueOffset;
					MP_DEBUG1("EComponentsConfiguration=0x%4x",psEXIF->SubIFDValue1[SubIFDCountNoOfDE][0]);
				  	SubIFDCountNoOfDE++;
				  break;

				  case 0xa000:/*TAG FlashPixVersion*/ /*FlashPix verion V1.0=0100*/
				  	 ValueOffset = mpxStreamReadDWord(psStream);
					//psEXIF->SubIFDCount[SubIFDCountNoOfDE]=1;	
					psEXIF->SubIFDValue1[SubIFDCountNoOfDE][0]=ValueOffset;
					MP_DEBUG1("FlashPixVersion=0x%4x",psEXIF->SubIFDValue1[SubIFDCountNoOfDE][0]);
				  	SubIFDCountNoOfDE++;
				  break;

				  case 0xa300:/*TAG FileSource 0xa300*/
				  	ValueOffset = mpxStreamReadDWord(psStream);
					//psEXIF->SubIFDCount[SubIFDCountNoOfDE]=1;	
					psEXIF->SubIFDValue1[SubIFDCountNoOfDE][0]=ValueOffset;
					MP_DEBUG1("FlashPixVersion=0x%4x",psEXIF->SubIFDValue1[SubIFDCountNoOfDE][0]);
				  	SubIFDCountNoOfDE++;
				  break;

				  case 0xa301:/*TAG FileSource 0xa300*/
				  	ValueOffset = mpxStreamReadDWord(psStream);
					//psEXIF->SubIFDCount[SubIFDCountNoOfDE]=1;	
					psEXIF->SubIFDValue1[SubIFDCountNoOfDE][0]=ValueOffset;
					MP_DEBUG1("FlashPixVersion=0x%4x",psEXIF->SubIFDValue1[SubIFDCountNoOfDE][0]);
				  	SubIFDCountNoOfDE++;
				  break;

#if MPO//THREE_D_PROJECT //for FAE Three D project smart copy
				  case 0x927C:	//Maker Note 				  	
					if(LittleEndian) ValueOffset = mpxStreamReadDWord_le(psStream);
						else ValueOffset = mpxStreamReadDWord(psStream);
					MP_DEBUG1("ValueOffset = 0x%x", ValueOffset);			
					MP_DEBUG1("EXIFOffset  = 0x%x", EXIFOffset);
					mpxStreamSeek(psStream, EXIFOffset + ValueOffset, SEEK_SET);					
					Jpg_GetEXIF_MakerNote(psStream, psEXIF,LittleEndian);

					SubIFDCountNoOfDE++;
				  break;
#endif
				  
				  default:
				  	MP_DEBUG1("JPEG Decode NOT support TAG=0x%4x",dwJohn);
				}
				
			break;

			//SRATIONAL
			case EXIF_TYPE_SRATIONAL:
              
				MP_DEBUG("*Rational*");
			  	if(LittleEndian) ValueOffset = mpxStreamReadDWord_le(psStream);
						else ValueOffset = mpxStreamReadDWord(psStream);
                MP_DEBUG1(" ValueOffset=0x%x",ValueOffset);

                if(Count>6)/*number of rational group *2 */
					Count=6;
				psEXIF->SubIFDCount[SubIFDCountNoOfDE]=Count;
				
                for(Gi=0;Gi<(Count);Gi++)
                {
				
				  mpxStreamSeek(psStream, EXIFOffset + ValueOffset/*+(Gi<<2)*/, SEEK_SET);
					  if(LittleEndian) RationalTemp = mpxStreamReadDWord_le(psStream);
						  else RationalTemp = mpxStreamReadDWord(psStream);
				  psEXIF->SubIFDValue1[SubIFDCountNoOfDE][/*Gi*/0]=RationalTemp;

				  if(LittleEndian) RationalTemp = mpxStreamReadDWord_le(psStream);
					  	  else RationalTemp = mpxStreamReadDWord(psStream);
				  psEXIF->SubIFDValue2[SubIFDCountNoOfDE][/*Gi*/0]=RationalTemp;
                }

			  	SubIFDCountNoOfDE++;
			break;


			case EXIF_TYPE_SKIP_TYPE:
		    default:
				    MP_DEBUG("*SKIP defaul*");
					mpxStreamSkip(psStream, 4);//SubIFDptr += 4;
          }
		}


		psEXIF->NoOfSubIFD=SubIFDCountNoOfDE;
		MP_DEBUG1("#### End SubIFD psEXIF->NoOfSubIFD=%d",psEXIF->NoOfSubIFD);

		
			
      /*---------End Sub IFD----------------------------------------------------------*/
}

void GetSubIFDExif_to_UI(ST_MPX_STREAM *psStream, FILE_EXIF_INFO_TYPE *psEXIF_to_UI, DWORD EXIFOffset, DWORD SubIFDOffset, BYTE LittleEndian)
{
  MP_DEBUG1("--  %s -- ", __FUNCTION__ );
	
  /*SubIFD-----Directory Entry					 */
  /* 		 Tag	= 2bytes					 */
  /* 		 Type  = 2bytes 					*/
  /* 		 Count = 4bytes 					*/
  /* 		 value or offset=1~4bytes(depend on Type)		  */
	
  DWORD dwJohn, dwMary;
  DWORD Count,ValueOffset;
  WORD NoOfInterOp,Type;
  WORD i;
		 
  DWORD RationalTemp;/*Frank Lin add*/
  DWORD SubTagStart=0;
	
  DWORD SubIFDCountNoOfDE=0;/*DE : Directory Entry*/
  BYTE  GetValue[4];
  DWORD Value,ValueTemp;
  DWORD Gi;/*number of rational group*/
  DWORD SubIFDValue1_Temp;/*numerator */
  DWORD SubIFDValue2_Temp;/*denominator */
		
  MP_DEBUG1("SubIFDOffset=0x%x",SubIFDOffset);
		 
  mpxStreamSeek(psStream, EXIFOffset+SubIFDOffset , SEEK_SET);
  DWORD SubIFDStart = mpxStreamTell(psStream);
  MP_DEBUG1("$$$ SubIFDStart = 0x%x", SubIFDStart);
		 
  if(LittleEndian) NoOfInterOp = mpxStreamReadWord_le(psStream);
	else NoOfInterOp = mpxStreamReadWord(psStream);
  MP_DEBUG("SubIFD NoOfInterOp = %x", NoOfInterOp);
	
  SubTagStart = EXIFOffset + SubIFDOffset + 2;/*2Byte = Number of Directory Entries*/
  MP_DEBUG1("SubTagStart=0x%x",SubTagStart);
	 
  MP_DEBUG1("fff NoOfInterOp=%d",NoOfInterOp);
	
	
		   
  for(i = 0; i < NoOfInterOp; i++){
    MP_DEBUG1("SubIFD------ %d -----",i);
     
	//go nth and get tag
	mpxStreamSeek(psStream, SubTagStart + i * 12, SEEK_SET);
	
	if(LittleEndian)
	{
	  dwJohn = mpxStreamReadWord_le(psStream);
	  Type   = mpxStreamReadWord_le(psStream);
	  Count  = mpxStreamReadDWord_le(psStream);
	}
	else
	{
	  dwJohn = mpxStreamReadWord(psStream); 
	  Type   = mpxStreamReadWord(psStream);
	  Count  = mpxStreamReadDWord(psStream);
	}		   
	
	MP_DEBUG1("SubIFD tag value = 0x%04x", dwJohn);
    MP_DEBUG1("SubIFD Type=0x%x",Type);
			 		 
	switch(Type)
	{
      case EXIF_TYPE_RATIONAL:   
		MP_DEBUG("*Rational*");
        if((dwJohn != 0x829a) && (dwJohn != 0x829d) && (dwJohn != 0x9205) && (dwJohn != 0x920a))
        {
		  break;
        }
				 
		if(LittleEndian) ValueOffset = mpxStreamReadDWord_le(psStream);
			else ValueOffset = mpxStreamReadDWord(psStream);
        MP_DEBUG1(" ValueOffset=0x%x",ValueOffset);
				
	    mpxStreamSeek(psStream, EXIFOffset + ValueOffset/*+(Gi<<2)*/, SEEK_SET);
		if(LittleEndian) RationalTemp = mpxStreamReadDWord_le(psStream);
			else RationalTemp = mpxStreamReadDWord(psStream);
		SubIFDValue1_Temp = RationalTemp;

		if(LittleEndian) RationalTemp = mpxStreamReadDWord_le(psStream);
		  	else RationalTemp = mpxStreamReadDWord(psStream);
		SubIFDValue2_Temp =RationalTemp;

        switch(dwJohn)
        {  
		  case 0x829a:/*ExpusureTime TAG = 0x829a*/
			psEXIF_to_UI->ExposureTimeN = SubIFDValue1_Temp;
			psEXIF_to_UI->ExposureTimeD = SubIFDValue2_Temp;
			psEXIF_to_UI->ExposureTime_flag= 1;
		    break;

		  case 0x829d:/*FNumber*/
			psEXIF_to_UI->FNumberN = SubIFDValue1_Temp;
			psEXIF_to_UI->FNumberD = SubIFDValue2_Temp;
			psEXIF_to_UI->FNumber_flag = 1;
			break;

		  case 0x9205:/*MaxApertureValue*/
			psEXIF_to_UI->MaxApertureValueN = SubIFDValue1_Temp;
			psEXIF_to_UI->MaxApertureValueD = SubIFDValue2_Temp;
			psEXIF_to_UI->MaxApertureValue_flag = 1;
			break;
 
		  case 0x920a: /*FocalLength TAG=0x920a*/
			psEXIF_to_UI->FocalLengthN = SubIFDValue1_Temp;
			psEXIF_to_UI->FocalLengthD = SubIFDValue2_Temp;
			psEXIF_to_UI->FocalLength_flag = 1;
			break;
					
		  default:
			MP_DEBUG1("*SKIP * TAG=0x%4x", dwJohn);
        }

      break;/*END EXIF_TYPE_RATIONAL*/

      case EXIF_TYPE_USIGNED_SHORT:
	    MP_DEBUG3("*SHORT*-------Short type: IFD0 TAG=0x%4x,Type=%d,Count=0x%x",dwJohn,Type,Count);
		if((dwJohn != 0x8822) && (dwJohn != 0x8827) && (dwJohn != 0x9207) && (dwJohn != 0x9209)) 
		{
		  MP_DEBUG1("*SKIP defaul* TAG=0x%4x", dwJohn);
		  mpxStreamSkip(psStream, 4);//SubIFDptr += 4;
		  break;
		}
        if(LittleEndian) 
		{
		  mpxStreamRead(GetValue, 1, 2, psStream);
		  Value = (DWORD)((GetValue[1]<<8) + (GetValue[0]));
        }
		else
		{
		  mpxStreamRead(GetValue, 1, 2, psStream);
		  Value = (DWORD)((GetValue[0]<<8) + (GetValue[1]));
		}
        MP_DEBUG4("--------TAG= 0x%x GetValue0=0x%x,GetValue1=0x%x,Value=0x%x",dwJohn,GetValue[0],GetValue[1],Value);

		switch(dwJohn)
        {
		  case 0x8822:/*ExposureProgram*/
			psEXIF_to_UI->ExposureProgram = Value;
			psEXIF_to_UI->ExposureProgram_flag = 1;
			break;

		  case 0x8827:/*ISOSpeedRatings*/
			psEXIF_to_UI->ISOSpeedRatings = Value;
			psEXIF_to_UI->ISOSpeedRatings_flag = 1;
			break;

		  case 0x9207:/*MeteringMode*/
			psEXIF_to_UI->MeteringMode = Value;
			psEXIF_to_UI->MeteringMode_flag = 1;
			break;

          case 0x9209:/*Flash*/
			psEXIF_to_UI->Flash = Value;
			psEXIF_to_UI->Flash_flag = 1;
			break;
					  

		  default:
			MP_DEBUG1("*SKIP * TAG=0x%4x", dwJohn);
		}
				 
		mpxStreamSkip(psStream, 2);
      break;/*End EXIF_TYPE_USIGNED_SHORT*/
	  
	  case EXIF_TYPE_UNDEFINED:
		if(dwJohn == 0x9000)/*TAG ExifVersion*/
		{
		  mpxStreamRead(psEXIF_to_UI->ExifVersion, 1, 4, psStream);
          psEXIF_to_UI->ExifVersion[4] = NULL;
		  psEXIF_to_UI->ExifVersion_flag = 1;
		  MP_DEBUG1("Exif Version=%s",psEXIF_to_UI->ExifVersion);
		}
		else
		{
		  MP_DEBUG1("*SKIP defaul* TAG=0x%4x", dwJohn);
		  mpxStreamSkip(psStream, 4);//SubIFDptr += 4;
		}
      break;/*End  EXIF_TYPE_UNDEFINED*/

	  case EXIF_TYPE_ASCII_STRING:
		if(dwJohn == 0x9003) /*DateTimeOriginal*/
		{
		  MP_DEBUG1("*ASCII* ASCII_STRING Type: Count = 0x%x", Count);
				                           
		  if(LittleEndian) ValueOffset = mpxStreamReadDWord_le(psStream);
			else ValueOffset = mpxStreamReadDWord(psStream);
						
		  mpxStreamSeek(psStream, EXIFOffset + ValueOffset, SEEK_SET);
		  mpxStreamRead(psEXIF_to_UI->DateTimeOriginal, 1, 19, psStream);
					 
		  psEXIF_to_UI->DateTimeOriginal[19]= NULL;
		  psEXIF_to_UI->DateTimeOriginal_flag= 1;
		}
		else
		{
		  MP_DEBUG1("*SKIP defaul* TAG=0x%4x", dwJohn);
		  mpxStreamSkip(psStream, 4);//SubIFDptr += 4;
		}
      break;
				  
	  default:
		MP_DEBUG1("*SKIP defaul* TAG=0x%4x", dwJohn);
	    mpxStreamSkip(psStream, 4);//SubIFDptr += 4;
    }			 
  }
	
}



void Jpg_GetEXIF(ST_MPX_STREAM *psStream, ST_EXIF *psEXIF)
{
    DWORD ZeroIFDoffset, Count, ValueOffset, EXIFOffset, APP1Offset, TagOffset;
    WORD dwJohn, dwMary, APP1Length, Type, NoOfInterOp, i,j;
    BYTE LittleEndian;

	DWORD IFD0DecodeCountNum=0;
	DWORD Value,ValueTemp;
	BYTE  GetValue[4];
	DWORD RationalTemp;/*Frank Lin add*/
	DWORD SubIFDOffset=0;
	DWORD Gi;

    LittleEndian = 0;

    APP1Offset = mpxStreamTell(psStream);
    APP1Length = mpxStreamReadWord(psStream); 
    MP_DEBUG("APP1Offset = 0x%08x", APP1Offset);
    MP_DEBUG("APP1Length = 0x%08x", APP1Length);
    
    dwJohn = mpxStreamReadWord(psStream);
    dwMary = mpxStreamReadWord(psStream);
    if((dwJohn == 0x4578) && (dwMary == 0x6966)) //check EXIF
    {
		mpxStreamSkip(psStream, 2); //padding 0x0000

		EXIFOffset = mpxStreamTell(psStream);
		MP_DEBUG1("EXIFOffset = 0x%08x", EXIFOffset);
		
		dwJohn = mpxStreamReadWord(psStream);
		if(dwJohn == 0x4949) LittleEndian = TRUE;

		mpxStreamSkip(psStream, 2); //pass 0x002A
		
		if(LittleEndian) ZeroIFDoffset = mpxStreamReadDWord_le(psStream);
		else ZeroIFDoffset = mpxStreamReadDWord(psStream);
		MP_DEBUG1("ZeroIFDoffset = 0x%08x", ZeroIFDoffset);
        
		mpxStreamSeek(psStream, EXIFOffset + ZeroIFDoffset , SEEK_SET);

		if(LittleEndian) NoOfInterOp = mpxStreamReadWord_le(psStream);
		else NoOfInterOp = mpxStreamReadWord(psStream);
		MP_DEBUG("NoOfInterOp = %x", NoOfInterOp);

        if (NoOfInterOp == 0xd8ff && LittleEndian)//Jasmine 071019: if special exif
            NoOfInterOp = 0;
		
		//go to 0thIFD 
		mpxStreamSeek(psStream, ZeroIFDoffset - 8, SEEK_CUR);
		DWORD TagStart = mpxStreamTell(psStream);
		MP_DEBUG1("TagStart = 0x%08x", TagStart);


    MP_DEBUG1("Total IFD0 Tag=%d",NoOfInterOp);
	MP_DEBUG1("Before IFD0DecodeCountNum=%d",IFD0DecodeCountNum);


		for(i = 0; i <= NoOfInterOp; i++){
			//go nth and get tag
			if(i == 0)
			mpxStreamSeek(psStream, EXIFOffset + ZeroIFDoffset + i * 12 + 2, SEEK_SET);
			else{
			mpxStreamSeek(psStream, EXIFOffset + ZeroIFDoffset + i * 12, SEEK_SET);
			mpxStreamSkip(psStream, 2);
			}

            MP_DEBUG("----------------------------------------------");
			
			TagOffset = mpxStreamTell(psStream);
			MP_DEBUG1("tag address = 0x%08x", TagOffset);
			if(LittleEndian) dwJohn = mpxStreamReadWord_le(psStream);
			else dwJohn = mpxStreamReadWord(psStream); 
			MP_DEBUG1("tag value = 0x%04x", dwJohn);        
			
			if(LittleEndian) Type = mpxStreamReadWord_le(psStream);
			else Type = mpxStreamReadWord(psStream);
			MP_DEBUG1("Tag type=0x%x",Type);

			if(LittleEndian) Count = mpxStreamReadDWord_le(psStream);
			else Count = mpxStreamReadDWord(psStream);
			MP_DEBUG1("Tag count=0x%x",Count);

			psEXIF->IFD0Tag[IFD0DecodeCountNum]=dwJohn;/*TAG*/
			psEXIF->IFD0Type[IFD0DecodeCountNum]=Type;
			
            switch(Type)
			{
			  case EXIF_TYPE_USIGNED_BYTE:/*Frank Lin,not test*/
			  	MP_DEBUG3("-------BYTE type: IFD0 TAG=0x%4x,Type=%d,Count=0x%x",dwJohn,Type,Count);
				  	  mpxStreamRead(GetValue, 1, 1, psStream);
					  Value=(BYTE)(GetValue[0]);
                MP_DEBUG3("--------GetValue0=0x%x,GetValue1=0x%x,Value=0x%x",GetValue[0],GetValue[1],Value);

				  psEXIF->IFD0Count[IFD0DecodeCountNum]=1/*Count*/;
			      psEXIF->IFD0Value1[IFD0DecodeCountNum][0]=Value; 
				  IFD0DecodeCountNum++;
				  mpxStreamSkip(psStream, 3);/*Skip 3 bytes*/
			  break;
			
			  case EXIF_TYPE_ASCII_STRING:
			  	  Count = (Count>CAMERAINFOSIZE)? CAMERAINFOSIZE:Count;
			      MP_DEBUG1("ASCII_STRING Type: Count = 0x%x", Count);
				  if(Count <= 4){ //the value is recorded, may be < 4?
					  	mpxStreamRead(psEXIF->IFD0String[IFD0DecodeCountNum], 1, Count, psStream);
						mpxStreamSkip(psStream, 4 - Count);
		          }
				  else{                       
						if(LittleEndian) ValueOffset = mpxStreamReadDWord_le(psStream);
						else ValueOffset = mpxStreamReadDWord(psStream);
						
						mpxStreamSeek(psStream, EXIFOffset + ValueOffset, SEEK_SET);
						mpxStreamRead(psEXIF->IFD0String[IFD0DecodeCountNum], 1, Count, psStream);
					  	MP_DEBUG1("Make = %s", psEXIF->IFD0String);
					  }
				  psEXIF->IFD0Count[IFD0DecodeCountNum]=Count;
				  IFD0DecodeCountNum++;	
			  break;
			  
			  case EXIF_TYPE_USIGNED_SHORT:
			  	  MP_DEBUG3("-------Short type: IFD0 TAG=0x%4x,Type=%d,Count=0x%x",dwJohn,Type,Count);
                  if(LittleEndian) 
				  	{
				  	  mpxStreamRead(GetValue, 1, 2, psStream);
					  Value=(DWORD)((GetValue[1]<<8)+(GetValue[0]));
                  	}
				  else
				  	{
				  	  mpxStreamRead(GetValue, 1, 2, psStream);
					  Value=(DWORD)((GetValue[0]<<8)+(GetValue[1]));
				  	}
                  MP_DEBUG3("--------GetValue0=0x%x,GetValue1=0x%x,Value=0x%x",GetValue[0],GetValue[1],Value);

				  psEXIF->IFD0Count[IFD0DecodeCountNum]=1/*Count*/;
			      psEXIF->IFD0Value1[IFD0DecodeCountNum][0]=Value; 
				  IFD0DecodeCountNum++;
				  mpxStreamSkip(psStream, 2);
			  break;


              case EXIF_TYPE_USIGNED_LONG:
			  	if(dwJohn==0x8769)/*ExifOffset*/
			  	{
			  	  if(LittleEndian) SubIFDOffset = mpxStreamReadDWord_le(psStream);
						else SubIFDOffset = mpxStreamReadDWord(psStream);
                //      psEXIF->SubIFDOffset=SubIFDOffset;
				//	MP_DEBUG1("$$$$ SubIFDOffset=0x%x",psEXIF->SubIFDOffset);	

					GetSubIFDExif(psStream,psEXIF,EXIFOffset,SubIFDOffset,LittleEndian);

					psEXIF->IFD0Count[IFD0DecodeCountNum]=1/*Count*/;
			        psEXIF->IFD0Value1[IFD0DecodeCountNum][0]=SubIFDOffset;
					IFD0DecodeCountNum++;
			  	}
				else
				{
				  
				  mpxStreamSkip(psStream, 4);//ZeroIFDptr += 10;
			  	  MP_DEBUG3("IFD0 Skip TAG=0x%4x,Type=%d,Count=0x%x",dwJohn,Type,Count);
     			}

			  break;
 

              case EXIF_TYPE_RATIONAL:
			  	if(LittleEndian) ValueOffset = mpxStreamReadDWord_le(psStream);
						else ValueOffset = mpxStreamReadDWord(psStream);
                MP_DEBUG1(" ValueOffset=0x%x",ValueOffset);

                if(Count>6)/*number of rational group *2 */
					Count=6;
				psEXIF->SubIFDCount[IFD0DecodeCountNum]=Count;
				
				MP_DEBUG1("Count=%d",Count);
                for(Gi=0;Gi<(Count);Gi++)
                {
				mpxStreamSeek(psStream, EXIFOffset + ValueOffset+(Gi<<2), SEEK_SET);
					if(LittleEndian) RationalTemp = mpxStreamReadDWord_le(psStream);
						else RationalTemp = mpxStreamReadDWord(psStream);
				psEXIF->IFD0Value1[IFD0DecodeCountNum][Gi]=RationalTemp;

				MP_DEBUG2("(((((((9 value=0x%x,RationalTemp=0x%x",psEXIF->IFD0Value1[IFD0DecodeCountNum][Gi],RationalTemp);

				if(LittleEndian) RationalTemp = mpxStreamReadDWord_le(psStream);
						else RationalTemp = mpxStreamReadDWord(psStream);
				psEXIF->IFD0Value2[IFD0DecodeCountNum][Gi]=RationalTemp;
                }
				psEXIF->IFD0Count[IFD0DecodeCountNum]=Count;
			  	IFD0DecodeCountNum++;  
			  break;
			  			  
			  default:
			  	mpxStreamSkip(psStream, 4);//ZeroIFDptr += 10;
			  	MP_DEBUG3("**** IFD0 Skip TAG=0x%4x,Type=%d,Count=0x%x",dwJohn,Type,Count);
			  	
            }
			
		}


       psEXIF->NoOfZeroIFD=IFD0DecodeCountNum;
       MP_DEBUG1("After IFD0DecodeCountNum=%d",IFD0DecodeCountNum);
  
		
	}
	mpxStreamSeek(psStream, APP1Offset, SEEK_SET);
	DWORD tempAddr;
	tempAddr = mpxStreamTell(psStream);
	MP_DEBUG("tempAddr = 0x%08x", tempAddr);
}


//--------------------------------------------------------------------------------------------
void Jpg_PrintEXIF(ST_EXIF *psEXIF)
{

    DWORD i;
	MP_DEBUG("== Print EXIF  ==");
	MP_DEBUG1("psEXIF->NoOfZeroIFD=%d",psEXIF->NoOfZeroIFD);
	for(i=0;i< psEXIF->NoOfZeroIFD;i++)
	{ 
	  MP_DEBUG1("---- i=%d----",i);
	  MP_DEBUG1("IFD0Tag=0x%4x",psEXIF->IFD0Tag[i]);
	  MP_DEBUG1("IFD0Type=0x%4x",psEXIF->IFD0Type[i]);
	  MP_DEBUG1("IFD0Count=0x%x",psEXIF->IFD0Count[i]);
      MP_DEBUG1("IFD0Value1=0x%x",psEXIF->IFD0Value1[i][0]);
	  MP_DEBUG1("IFD0Value2=0x%x",psEXIF->IFD0Value2[i][0]);
	  MP_DEBUG1("psEXIF->IFD0String=\"%s\"",psEXIF->IFD0String[i]);
	}
	MP_DEBUG("###### SubIFD ######");
    MP_DEBUG1("psEXIF->NoOfSubIFD=%d",psEXIF->NoOfSubIFD);
	for(i=0;i< psEXIF->NoOfSubIFD;i++)
	{
	 MP_DEBUG1("---- i=%d----",i);
	  MP_DEBUG1("SubIFDTag=0x%4x",psEXIF->SubIFDTag[i]);
	  MP_DEBUG1("SubIFDType=0x%4x",psEXIF->SubIFDType[i]);
	  MP_DEBUG1("SubIFDCount=0x%x",psEXIF->SubIFDCount[i]);
      MP_DEBUG1("SubIFDValue1=0x%x",psEXIF->SubIFDValue1[i][0]);
	  MP_DEBUG1("SubIFDValue2=0x%x",psEXIF->SubIFDValue2[i][0]);
	  MP_DEBUG1("psEXIF->SubIFD0String=\"%s\"",psEXIF->SubIFDString[i]);
	}

#if 0//Frank Lin close
	MP_DEBUG("-----------------EXIF-----------------------");
	MP_DEBUG1("Make = %s", psEXIF->MakeValue);
	MP_DEBUG1("Model = %s", psEXIF->ModelValue);
	MP_DEBUG1("DateTime = %s", psEXIF->DateTimeValue);
	MP_DEBUG1("Orientation = %d", psEXIF->OrientationValue);
	
	MP_DEBUG1("Artist = %s", psEXIF->ArtistValue);
	MP_DEBUG1("CopyRight = %s", psEXIF->CopyRightValue);
	MP_DEBUG1("ImageDescription = %s", psEXIF->ImageDescriptionValue);
	MP_DEBUG1("UserComment = %s", psEXIF->UserCommentValue);
	
	MP_DEBUG("-----------------EXIF end-----------------------");
#endif	
	
}



/*------Get EXIF for UI-------*/
void Jpg_GetEXIF_to_UI(ST_MPX_STREAM *psStream, FILE_EXIF_INFO_TYPE *psEXIF_to_UI)
{
  MP_DEBUG1("-### %s ###-", __FUNCTION__ );
  DWORD ZeroIFDoffset, Count, ValueOffset, EXIFOffset, APP1Offset, TagOffset;
  WORD dwJohn, dwMary, APP1Length, Type, NoOfInterOp, i,j;
  BYTE LittleEndian;

  DWORD IFD0DecodeCountNum=0;
  DWORD Value,ValueTemp;
  BYTE  GetValue[4];
  DWORD RationalTemp;/*Frank Lin add*/
  DWORD SubIFDOffset=0;
  DWORD Gi;

  memset(psEXIF_to_UI, 0, sizeof(FILE_EXIF_INFO_TYPE));
  LittleEndian = 0;

  APP1Offset = mpxStreamTell(psStream);
  APP1Length = mpxStreamReadWord(psStream); 
  MP_DEBUG("APP1Offset = 0x%08x", APP1Offset);
  MP_DEBUG("APP1Length = 0x%08x", APP1Length);
    
  dwJohn = mpxStreamReadWord(psStream);
  dwMary = mpxStreamReadWord(psStream);
  if((dwJohn == 0x4578) && (dwMary == 0x6966)) //check EXIF
  {
	mpxStreamSkip(psStream, 2); //padding 0x0000

	EXIFOffset = mpxStreamTell(psStream);
	MP_DEBUG1("EXIFOffset = 0x%08x", EXIFOffset);
		
	dwJohn = mpxStreamReadWord(psStream);
	if(dwJohn == 0x4949) LittleEndian = TRUE;

	mpxStreamSkip(psStream, 2); //pass 0x002A
		
	if(LittleEndian) ZeroIFDoffset = mpxStreamReadDWord_le(psStream);
	else ZeroIFDoffset = mpxStreamReadDWord(psStream);
	MP_DEBUG1("ZeroIFDoffset = 0x%08x", ZeroIFDoffset);

	if(LittleEndian) NoOfInterOp = mpxStreamReadWord_le(psStream);
	else NoOfInterOp = mpxStreamReadWord(psStream);
	MP_DEBUG("NoOfInterOp = %x", NoOfInterOp);

    if (NoOfInterOp == 0xd8ff && LittleEndian)//Jasmine 071019: if special exif
       NoOfInterOp = 0;
		
	//go to 0thIFD 
	mpxStreamSeek(psStream, ZeroIFDoffset - 8, SEEK_CUR);
	DWORD TagStart = mpxStreamTell(psStream);
	MP_DEBUG1("TagStart = 0x%08x", TagStart);

    MP_DEBUG1("Total IFD0 Tag=%d",NoOfInterOp);
	MP_DEBUG1("Before IFD0DecodeCountNum=%d",IFD0DecodeCountNum);


	for(i = 0; i < NoOfInterOp; i++){
	  //go nth and get tag
	  if(i == 0)
	  {
		mpxStreamSeek(psStream, EXIFOffset + ZeroIFDoffset + i * 12 + 2, SEEK_SET);
	  }
	  else{
		mpxStreamSeek(psStream, EXIFOffset + ZeroIFDoffset + i * 12, SEEK_SET);
		mpxStreamSkip(psStream, 2);
	  }

      MP_DEBUG("----------------------------------------------");
			
	  TagOffset = mpxStreamTell(psStream);
	  MP_DEBUG1("tag address = 0x%08x", TagOffset);
	  if(LittleEndian) dwJohn = mpxStreamReadWord_le(psStream);
	  else dwJohn = mpxStreamReadWord(psStream); 
	  MP_DEBUG1("tag value = 0x%04x", dwJohn);        
			
	  if(LittleEndian) Type = mpxStreamReadWord_le(psStream);
	  else Type = mpxStreamReadWord(psStream);
	  MP_DEBUG1("Tag type=0x%x",Type);

	  if(LittleEndian) Count = mpxStreamReadDWord_le(psStream);
	  else Count = mpxStreamReadDWord(psStream);
	  MP_DEBUG1("Tag count=0x%x",Count);

      /*----------------------------------------*/
      switch(dwJohn)
      {
		case 0x010f: /*Make TAG = 0x010f */
		  Count = (Count>EXIF_STRING_LENGTH)? EXIF_STRING_LENGTH:Count;
		  MP_DEBUG1("ASCII_STRING Type: Count = 0x%x", Count);
		  if(Count <= 4){ //the value is recorded, may be < 4?
			mpxStreamRead(psEXIF_to_UI->Make, 1, Count, psStream);
			mpxStreamSkip(psStream, 4 - Count);
		  }
		  else{                       
			if(LittleEndian) ValueOffset = mpxStreamReadDWord_le(psStream);
				else ValueOffset = mpxStreamReadDWord(psStream);
						
			mpxStreamSeek(psStream, EXIFOffset + ValueOffset, SEEK_SET);
			mpxStreamRead(psEXIF_to_UI->Make, 1, Count, psStream);
			psEXIF_to_UI->Make[EXIF_STRING_LENGTH-1] = NULL;
			MP_DEBUG1("Make = %s", psEXIF_to_UI->Make);
		  }
		  psEXIF_to_UI->Make_flag = 1;
	    break;
  
        case 0x0110:/*Model TAG = 0x0110*/ 
		  Count = (Count>EXIF_STRING_LENGTH)? EXIF_STRING_LENGTH:Count;
		  MP_DEBUG1("ASCII_STRING Type: Count = 0x%x", Count);
		  if(Count <= 4){ //the value is recorded, may be < 4?
			mpxStreamRead(psEXIF_to_UI->Model, 1, Count, psStream);
			mpxStreamSkip(psStream, 4 - Count);
		  }
		  else{                       
			if(LittleEndian) ValueOffset = mpxStreamReadDWord_le(psStream);
				else ValueOffset = mpxStreamReadDWord(psStream);
						
			mpxStreamSeek(psStream, EXIFOffset + ValueOffset, SEEK_SET);
			mpxStreamRead(psEXIF_to_UI->Model, 1, Count, psStream);
			psEXIF_to_UI->Model[EXIF_STRING_LENGTH-1] = NULL;
		    MP_DEBUG1("Model = %s", psEXIF_to_UI->Model);
		  }
		  psEXIF_to_UI->Model_flag = 1;
		break;

        case 0x0112: /*Orientation TAG = 0x0112*/
		  MP_DEBUG3("-------Short type: IFD0 TAG=0x%4x,Type=%d,Count=0x%x",dwJohn,Type,Count);
          if(LittleEndian) 
		  {
			mpxStreamRead(GetValue, 1, 2, psStream);
			Value=(DWORD)((GetValue[1]<<8)+(GetValue[0]));
          }
		  else
		  {
		    mpxStreamRead(GetValue, 1, 2, psStream);
			Value=(DWORD)((GetValue[0]<<8)+(GetValue[1]));
		  }
          MP_DEBUG3("--------GetValue0=0x%x,GetValue1=0x%x,Value=0x%x",GetValue[0],GetValue[1],Value);

		  psEXIF_to_UI->Orientation      = Value;
		  psEXIF_to_UI->Orientation_flag = 1;
		  mpxStreamSkip(psStream, 2);	  
		break;

		case 0x8769:/*ExifOffset TAG = 0x879*/
                
		  if(LittleEndian) SubIFDOffset = mpxStreamReadDWord_le(psStream);
			else SubIFDOffset = mpxStreamReadDWord(psStream);
               
		  MP_DEBUG1("$$$$ SubIFDOffset=0x%x",SubIFDOffset);	

          GetSubIFDExif_to_UI(psStream, psEXIF_to_UI, EXIFOffset, SubIFDOffset, LittleEndian);	
		break;
				
		default:
		  mpxStreamSkip(psStream, 4);//ZeroIFDptr += 10;
	      MP_DEBUG3("**** IFD0 Skip TAG=0x%4x,Type=%d,Count=0x%x",dwJohn,Type,Count);					
      }
      /*------------------------------------------*/		
	}/*end for loop*/

	 MP_DEBUG1("After IFD0DecodeCountNum=%d",IFD0DecodeCountNum);
	
  }
	mpxStreamSeek(psStream, APP1Offset, SEEK_SET);
	DWORD tempAddr;
	tempAddr = mpxStreamTell(psStream);
	MP_DEBUG("tempAddr = 0x%08x", tempAddr);
}




void Jpg_EXIF_Extractions(ST_MPX_STREAM *psStream, ST_JPEG *psJpeg)//byAlexWang 17dec2007
{
#if 0 //Frank Lin 
	ST_EXIF *psEXIF=NULL;
	if ((psJpeg->bDecodeMode & 0x3f) == IMG_DECODE_PHOTO) 
	{
		psEXIF = (ST_EXIF *)ext_mem_malloc(sizeof(ST_EXIF)+32); 
		if (!psEXIF) return;
//		ext_mem_register( (sizeof(ST_EXIF)+32), "Jpg_EXIF_Extractions() in Jpeg_hal.c", psEXIF);//byAlexWang 27apr2007 m2project
		memset(psEXIF, 0, sizeof(ST_EXIF));
		Jpg_GetEXIF(psStream, psEXIF);
		Jpg_PrintEXIF(psEXIF);
		ext_mem_free(psEXIF);
     }
#else
   if(g_psEXIFEnc!=NULL)
   {
	 Jpg_GetEXIF(psStream, g_psEXIFEnc);
  #if 0 //for debug only
	 Jpg_PrintEXIF(g_psEXIFEnc);
  #endif
   }
   else if(g_psEXIF_Info_ForUI != NULL)
   {
   	/*get EXIF to UI*/
    MP_DEBUG("$$$EXIF Decode EXIF:g_psEXIFEnc is NULL.");
	MP_DEBUG("$$$EXIF Decode EXIF:g_psEXIF_Info_ForUI is not NULL.");
	Jpg_GetEXIF_to_UI(psStream, g_psEXIF_Info_ForUI);
   }
   else
   {
   	MP_DEBUG("$$$EXIF Decode EXIF:g_psEXIFEnc is NULL.");
	MP_DEBUG("$$$EXIF Decode EXIF:g_psEXIF_Info_ForUI is NULL.");
   }
#endif
}



SDWORD JepgExifTagReportInfo(ST_EXIF_INFO *sImageDecEXIFinfo,ST_EXIF *psEXIFptr)
{
    SDWORD iErrorCode;
	ST_EXIF *psEXIF;
	DWORD i;

	WORD TagTemp;
	psEXIF=psEXIFptr;

    if( psEXIFptr == NULL )
    {		
		MP_ALERT("%s:psEXIFptr is NULL!!", __FUNCTION__);
		return FAIL;
    }

       memset(sImageDecEXIFinfo, 0, sizeof(ST_EXIF_INFO));   
	
	for(i=0;i<psEXIF->NoOfZeroIFD;i++)
	{
		TagTemp=psEXIF->IFD0Tag[i];
		switch(TagTemp)
		{
            case 0x0112:/*Orientation*/
				sImageDecEXIFinfo->OrientationValue=psEXIF->IFD0Value1[i][0];
				MP_DEBUG1("$$$ OrientationValue=%d",sImageDecEXIFinfo->OrientationValue);
				break;
			case 0x0132:/*TAG DateTime*/
                memcpy( &(sImageDecEXIFinfo->DateTimeValue[0]),&(psEXIF->IFD0String[i][0]),CAMERAINFOSIZE);
				MP_DEBUG1("$$$ DateTimeValue=%s",sImageDecEXIFinfo->DateTimeValue);
				break;
			default:
				MP_DEBUG1("$$$ SKIP TAG 0x%4x",TagTemp);
		}
	}
	
    for(i=0;i<psEXIF->NoOfSubIFD;i++)
	{
		TagTemp=psEXIF->SubIFDTag[i];
		switch(TagTemp)
		{
			case 0x9003:/*DateTimeOrignal*/
				memcpy( &(sImageDecEXIFinfo->DateTimeOriginalValue[0]),&(psEXIF->SubIFDString[i][0]),CAMERAINFOSIZE);
                MP_DEBUG1("$$$ DateTimeOriginalValue=%s",sImageDecEXIFinfo->DateTimeOriginalValue);
				break;
			default:
				MP_DEBUG1("$$$ SKIP TAG 0x%4x",TagTemp);	 	
		}
    }
	

	iErrorCode=PASS;
	
	return iErrorCode;
}





#endif//EXIF
//--------------------------------------------------------------------------------------------
int Jpg_Parse_Marker(ST_MPX_STREAM *psStream, ST_JPEG *psJpeg)
{
	BYTE bMarker = 0;
	DWORD dwJohn, dwMary, dwPaul;	// some helper valuables for temporary used
	DWORD *dwpTableAddr;
	int iErrCode = 0;

	DWORD GetSeekPos = 0;
	WORD  Length     = 0;	

#if MPO
        g_psCurJpeg_Mpo->dwFirstApp2Pos = 0;
#endif
	MP_DEBUG2("psStream %08x %d", (DWORD)psStream, psStream->pos + psStream->buf_pos);
	
	// read and check the first two byte for SOI bMarker
	if ((psJpeg->bDecodeMode & 0x3f) != IMG_DECODE_MJPEG) 
	{
	    dwJohn = mpxStreamReadWord(psStream);
	    if (dwJohn != 0xffd8)
	    {
		    MP_DEBUG1("-E- %04x !! head mismatch, NOT JPEG FILE!! \r\n", dwJohn);
		    return NOT_JPEG_FILE;
	    }
	}
//	MP_DEBUG1("Jpg_Parse_Marker %02x", psJpeg->bDecodeMode);			
	
	while (bMarker != MARKER_EOI && !psStream->eof)
	{
		// bypass all bytes except the 0xff to find the bMarker
		if (mpxStreamSearchMarker(psStream, MARKER_FIRST) == FAIL) 
			break;
		do
		{
		bMarker = mpxStreamGetc(psStream);	// get the second byte of bMarker
		}while(bMarker == MARKER_FIRST);
	
		if (bMarker == 0) continue;
		//if ((psJpeg->bDecodeMode & 0x3f) == IMG_DECODE_THUMB1)
		//	MP_DEBUG2("%08x marker %02x", psStream->buf_pos + psStream->pos, bMarker);
		
		switch (bMarker)
		{
		MP_DEBUG1("Jpg_Parse_Marker %02x", bMarker);			

		case MARKER_SOF0:  // 0xc0
			MP_DEBUG("MARKER_SOF0");		
			iErrCode = Jpg_Marker_SOF0(psStream, psJpeg);
			if (iErrCode != PASS)
				return iErrCode;
			break;

		case MARKER_DHT:  //0xc4 
			MP_DEBUG("MARKER_DHT");				
			psJpeg->dwDhtPos[psJpeg->bDhtHit] = mpxStreamTell(psStream);
			mpxStreamReadAndSkipLength(psStream);
			//Jpg_Marker_DHT(psStream, psJpeg);			
			psJpeg->bDhtHit++;
			break;

		case MARKER_EOI: // 0xd9
			psJpeg->dwEoiPoint = mpxStreamTell(psStream);
			MP_DEBUG1("MARKER_EOI %08x", psJpeg->dwEoiPoint);
			break;
			
		case MARKER_SOS:  // 0xda
			mpxStreamReadAndSkipLength(psStream);
			psJpeg->dwEcsStart = mpxStreamTell(psStream);
			MP_DEBUG1("MARKER_SOS %08x", psJpeg->dwEcsStart);
			bMarker = MARKER_EOI;
			break;
			
		case MARKER_DQT:  // 0xdb
			MP_DEBUG("MARKER_DQT");
			if(psJpeg->bDqtHit < 2)
			{
			  psJpeg->dwDqtPos[psJpeg->bDqtHit] = mpxStreamTell(psStream);
			  mpxStreamReadAndSkipLength(psStream);
			  psJpeg->bDqtHit++;
			}
			else
			{
			  mpxStreamReadAndSkipLength(psStream);
			}
			/* psJpeg->bDqtHit must be less than 2.*/
			/* If psJpeg->bDqtHit =>2 , need check MJpeg . MJpeg will fail. */
			//Jpg_Marker_DQT(psStream, psJpeg);
						
			break;

		case MARKER_DRI:  // 0xdd			
			mpxStreamSkip(psStream, 2);	// skip the Lr
			dwJohn = mpxStreamReadWord(psStream);
			MP_DEBUG("MARKER_DRI %d", dwJohn);			
			if(dwJohn > 0) {
			    psJpeg->dwCduDri = dwJohn;
			    //((CDU *) (CDU_BASE))->CduDri = dwJohn - 1;
			    psJpeg->dwCduControl |= 0x00000010L;
			}
			break;

			// these markers will be ignored
		case MARKER_RST0:  // 0xd0
		case MARKER_RST1:
		case MARKER_RST2:
		case MARKER_RST3:
		case MARKER_RST4:
		case MARKER_RST5:
		case MARKER_RST6:
		case MARKER_RST7:	// 0xd7
			break;

		case MARKER_APP1:
			MP_DEBUG("MARKER_APP1");

			GetSeekPos = mpxStreamTell(psStream);
			Length = mpxStreamReadWord(psStream); 
					 
			dwJohn = mpxStreamReadWord(psStream);
			dwMary = mpxStreamReadWord(psStream);

			mpxStreamSeek(psStream, GetSeekPos , SEEK_SET); 
							
			if((dwJohn == 0x4578) && (dwMary == 0x6966)) //check EXIF
			{
				MP_DEBUG("%s: supported TAG 0x%x EXIF", __FUNCTION__ , bMarker);
				psJpeg->dwApp1Pos = mpxStreamTell(psStream);
			    MP_DEBUG("dwApp1Pos = 0x%08x", psJpeg->dwApp1Pos);
			#if OPEN_EXIF
/*			        if ((psJpeg->bDecodeMode & 0x3f) == IMG_DECODE_PHOTO) {
			        memset(&g_sEXIF, 0, sizeof(g_sEXIF));
			        g_psEXIF = &g_sEXIF;
			        Jpg_GetEXIF(psStream, g_psEXIF);
			        Jpg_PrintEXIF();
			        }
*/
			    Jpg_EXIF_Extractions(psStream,psJpeg);
			#else
//			        if ((psJpeg->bDecodeMode & 0x3f) == IMG_DECODE_THUMB) {
				    //return Jpg_Marker_APP1(psStream, psJpeg);
				    //return PASS;
//			        } 		
			#endif
			}
			else
			{
				MP_DEBUG("%s: Not Support TAG 0x%x", __FUNCTION__, bMarker);
			}
			mpxStreamReadAndSkipLength(psStream);
			break;

#if MPO
        case MARKER_APP2: //Add for MPO

		    GetSeekPos = mpxStreamTell(psStream);
		    Length = mpxStreamReadWord(psStream);
    		dwJohn = mpxStreamReadDWord(psStream);
		    MP_DEBUG("%s: APP2 dwJohn =0x%x", __FUNCTION__, dwJohn);

		    mpxStreamSeek(psStream, GetSeekPos , SEEK_SET); 

		    if(dwJohn == 0x4d504600) //  //"MPF"  MP_FORMAT_ID
		    {
	          g_psCurJpeg_Mpo->dwFirstApp2Pos = mpxStreamTell(psStream);
	          MP_DEBUG("%s: supported TAG 0x%x MPF", __FUNCTION__ , bMarker);
            }
		    else
		    {
		       MP_DEBUG("%s: Not supported TAG 0x%x", __FUNCTION__ , bMarker);
		    }
	        mpxStreamReadAndSkipLength(psStream);
			//MP_ALERT("----g_psCurJpeg_Mpo->dwFirstApp2Pos = 0x%x " , g_psCurJpeg_Mpo->dwFirstApp2Pos);
	    break;
#else
        case MARKER_APP2: 
			mpxStreamReadAndSkipLength(psStream);
			break;
#endif
			
			// these markers will be ignored and bypassed
		case MARKER_APP0:  // 0xe0		
		case MARKER_APP3:
		case MARKER_APP4:
		case MARKER_APP5:
		case MARKER_APP6:
		case MARKER_APP7:
		case MARKER_APP8:
		case MARKER_APP9:
		case MARKER_APP10:
		case MARKER_APP11:
		case MARKER_APP12:
		case MARKER_APP13:
		case MARKER_APP14:
		case MARKER_APP15:  // 0xef
		case MARKER_JPG0:	// 0xf0
		case MARKER_JPG1:
		case MARKER_JPG2:
		case MARKER_JPG3:
		case MARKER_JPG4:
		case MARKER_JPG5:
		case MARKER_JPG6:
		case MARKER_JPG7:
		case MARKER_JPG8:
		case MARKER_JPG9:
		case MARKER_JPG10:
		case MARKER_JPG11:
		case MARKER_JPG12:
		case MARKER_JPG13:
		case MARKER_COM:    // 0xfe
			mpxStreamReadAndSkipLength(psStream);
			break;
		case MARKER_SOF2:  // 0xc2  /* Progressive, Huffman */
			psJpeg->bProgressive = 1;
			//mpxStreamReadAndSkipLength(psStream);
			iErrCode = Jpg_Marker_SOF0(psStream, psJpeg);
			break;		
			// these markers are not supported
		case MARKER_SOF1:

		case MARKER_SOF3:
		case MARKER_SOF5:
		case MARKER_SOF6:
		case MARKER_SOF7:
		case MARKER_SOF9:
		case MARKER_SOF10:
		case MARKER_SOF11:
		case MARKER_SOF13:
		case MARKER_SOF14:
		case MARKER_SOF15:
		case MARKER_JPG:
			//not supported frame type JPG
		case MARKER_DAC:
			//not supported frame type DAC
		default:
			MP_DEBUG("NOT_SUPPORTED_FRAME_TYPE %x", bMarker);
			return NOT_SUPPORTED_FRAME_TYPE;	// return error code
		}
	}
	

	if (psJpeg->dwEoiPoint == 0)
		psJpeg->dwEoiPoint = psStream->file_len;
	
	if (psJpeg->dwEcsStart == 0 || psJpeg->dwEoiPoint == 0)
	{
		//Start/End not found error
		MP_DEBUG(" FILE_EXTRACT_FAIL");
		return FILE_EXTRACT_FAIL;
	}
	MP_DEBUG2("Jpg_Parse_Marker w %d, h %d", psJpeg->wImageWidth, psJpeg->wImageHeight);

	MP_DEBUG("Jpeg parse marker pass");
	return PASS;
}

//--------------------------------------------------------------------------------------------
int Jpg_Thumb_Parse_Marker(ST_MPX_STREAM *psStream, ST_JPEG *psJpeg)
{
	BYTE bMarker = 0, Stop_Search = 0;
	DWORD dwJohn ;
	int iErrCode = 0;
	
	MP_DEBUG("Jpg_Thumb_Parse_Marker");
	if ((psJpeg->bDecodeMode & 0x3f) != IMG_DECODE_MJPEG) 
	{
	    dwJohn = mpxStreamReadWord(psStream);
	    if (dwJohn != 0xffd8)
	    {
		    MP_ALERT("-E- %04x !! head mismatch, NOT JPEG FILE!! \r\n", dwJohn);
		    return NOT_JPEG_FILE;
	    }
	}
	
	//while (bMarker != MARKER_EOI && !psStream->eof)
	while (!psStream->eof && !Stop_Search)
	{
		// bypass all bytes except the 0xff to find the bMarker
		if (mpxStreamSearchMarker(psStream, MARKER_FIRST) == FAIL) 
			break;
		do
		{
			bMarker = mpxStreamGetc(psStream);	// get the second byte of bMarker
		}while(bMarker == MARKER_FIRST);
	
		if (bMarker == 0) continue;
		
		
		switch (bMarker)
		{
			case MARKER_SOF0:  // 0xc0
				MP_DEBUG("MARKER_SOF0");		
				iErrCode = Jpg_Marker_SOF0(psStream, psJpeg);
				if (iErrCode != PASS)
					return iErrCode;
			break;
			
			case MARKER_DHT:  //0xc4 
				MP_DEBUG("MARKER_DHT");		
				psJpeg->dwDhtPos[psJpeg->bDhtHit] = mpxStreamTell(psStream);
				mpxStreamReadAndSkipLength(psStream);
				//Jpg_Marker_DHT(psStream, psJpeg);			
				psJpeg->bDhtHit++;
			break;

			case MARKER_EOI: // 0xd9
				psJpeg->dwEoiPoint = mpxStreamTell(psStream);
				MP_DEBUG("MARKER_EOI %08x", psJpeg->dwEoiPoint);
				Stop_Search = 1 ;
			break;
			
			case MARKER_SOS:  // 0xda
				mpxStreamReadAndSkipLength(psStream);
				psJpeg->dwEcsStart = mpxStreamTell(psStream);
				MP_DEBUG("MARKER_SOS %08x", psJpeg->dwEcsStart);
				bMarker = MARKER_EOI;
			break;
			
			case MARKER_DQT:  // 0xdb
				MP_DEBUG("MARKER_DQT");				
				psJpeg->dwDqtPos[psJpeg->bDqtHit] = mpxStreamTell(psStream);
				mpxStreamReadAndSkipLength(psStream);
				//Jpg_Marker_DQT(psStream, psJpeg);
				psJpeg->bDqtHit++;			
			break;

			case MARKER_DRI:  // 0xdd			
				mpxStreamSkip(psStream, 2);	// skip the Lr
				dwJohn = mpxStreamReadWord(psStream);
				MP_DEBUG("MARKER_DRI %d", dwJohn);			
				if(dwJohn > 0) {
			    psJpeg->dwCduDri = dwJohn;
			    //((CDU *) (CDU_BASE))->CduDri = dwJohn - 1;
			    psJpeg->dwCduControl |= 0x00000010L;
				}
			break;
			
			default:
				MP_DEBUG("NOT_SUPPORTED_FRAME_TYPE %x", bMarker);
		}
	}
	MP_DEBUG("Thumb w %d, h %d", psJpeg->wImageWidth, psJpeg->wImageHeight);
	
	return PASS ;
}

//--------------------------------------------------------------------------------------------
int Jpg_InitTables(ST_JPEG *psJpeg)
{
	int i;
	MP_DEBUG("Jpg_InitTables");
	if ((psJpeg->dc1 == 1) && (psJpeg->dc2 == 0))
	{
		for (i = 0; i < 16; i++)
		{
			baMinTable[i + 48] = baMinTable[i + 16];
			waBaseTable[i + 48] = waBaseTable[i + 16];
		}
		for (i = 0; i < 12; i++)
			baSymbolTable[i + 162] = (baSymbolTable[i + 162] << 4) | baSymbolTable[i + 162];
	}

	if ((psJpeg->dc1 == 0) && (psJpeg->dc2 == 1))
	{
		for (i = 0; i < 16; i++)
		{
			baMinTable[i + 16] = baMinTable[i + 48];
			waBaseTable[i + 16] = waBaseTable[i + 48];
		}
		for (i = 0; i < 12; i++)
			baSymbolTable[i + 162] = (baSymbolTable[i + 162] >> 4) | baSymbolTable[i + 162];
	}

	if ((psJpeg->ac1 == 1) && (psJpeg->ac2 == 0))
	{
		for (i = 0; i < 16; i++)
		{
			baMinTable[i + 32] = baMinTable[i];
			waBaseTable[i + 32] = waBaseTable[i];
		}
		for (i = 0; i < 162; i++)
			baSymbolTable[i + 174] = baSymbolTable[i];
	}

	if ((psJpeg->ac1 == 0) && (psJpeg->ac2 == 1))
	{
		for (i = 0; i < 16; i++)
		{
			baMinTable[i] = baMinTable[i + 32];
			waBaseTable[i] = waBaseTable[i + 32];
		}
		for (i = 0; i < 162; i++)
			baSymbolTable[i] = baSymbolTable[i + 174];
	}

	
}
//--------------------------------------------------------------------------------------------
void Jpg_CheckQtTable(ST_JPEG *psJpeg)
{
	volatile DWORD dwJohn;
	DWORD *dwpTableAddr;
	DWORD *dwpSrcTableAddr;
	int i;
	CDU *cdu = (CDU *) (CDU_BASE);
	
	// check if miss QT table
	if (psJpeg->boWithDQ == 0x10) 
	{		
		dwpSrcTableAddr = (DWORD *) (&cdu->CduQt[0] + (1 << 6));
		dwpTableAddr = (DWORD *) (&cdu->CduQt[0] + (0 << 6));
		
		for (i = 0; i < 64; i++)
		{
			dwJohn = *dwpSrcTableAddr; 
			*dwpTableAddr = dwJohn;   // fill it to CDU table directly
			dwpTableAddr++;
			dwpSrcTableAddr++;
		}
	}		
	
	if (psJpeg->boWithDQ == 0x01) 
	{	
		dwpSrcTableAddr = (DWORD *) (&cdu->CduQt[0] + (0 << 6));
		dwpTableAddr = (DWORD *) (&cdu->CduQt[0] + (1 << 6));

		for (i = 0; i < 64; i++)
		{
			dwJohn = *dwpSrcTableAddr; 
			*dwpTableAddr = dwJohn;   // fill it to CDU table directly
			dwpTableAddr++;
			dwpSrcTableAddr++;
		}
	}	
}

//--------------------------------------------------------------------------------------------
void Jpg_SetHummanTable(BYTE *pbMinTable, WORD *pwBaseTable, BYTE *pbSymbolTable)
{	
	volatile DWORD dwValue;
	DWORD dwJohn;
	int i;	

	CDU *cdu = (CDU *) (CDU_BASE);
	DWORD *dwpTableAddr = (DWORD *) (&cdu->CduHdmt[3]);
	BYTE *bpAuxPoint = pbMinTable;
	

	// fill the Huffman MIN table	
	for (i = 0; i < 4; i++)
	{
		dwJohn = *bpAuxPoint & 0x01;
		bpAuxPoint++;
		dwJohn = (dwJohn << 2) | (*bpAuxPoint & 0x03);
		bpAuxPoint++;
		dwJohn = (dwJohn << 3) | (*bpAuxPoint & 0x07);
		bpAuxPoint++;
		dwValue = (dwJohn >> 2);
		*dwpTableAddr = dwValue;
		dwpTableAddr--;

		dwJohn = (dwJohn << 4) | (*bpAuxPoint & 0x0f);
		bpAuxPoint++;
		dwJohn = (dwJohn << 5) | (*bpAuxPoint & 0x1f);
		bpAuxPoint++;
		dwJohn = (dwJohn << 6) | (*bpAuxPoint & 0x3f);
		bpAuxPoint++;
		dwJohn = (dwJohn << 7) | (*bpAuxPoint & 0x7f);
		bpAuxPoint++;
		dwValue = ((dwJohn << 8) | *bpAuxPoint);
		bpAuxPoint++;
		*dwpTableAddr = dwValue;
		dwpTableAddr--;

		dwJohn = *bpAuxPoint;
		bpAuxPoint++;
		dwJohn = (dwJohn << 8) | *bpAuxPoint;
		bpAuxPoint++;
		dwJohn = (dwJohn << 8) | *bpAuxPoint;
		bpAuxPoint++;
		dwValue = ((dwJohn << 8) | *bpAuxPoint);
		bpAuxPoint++;
		*dwpTableAddr = dwValue;
		dwpTableAddr--;

		dwJohn = *bpAuxPoint;
		bpAuxPoint++;
		dwJohn = (dwJohn << 8) | *bpAuxPoint;
		bpAuxPoint++;
		dwJohn = (dwJohn << 8) | *bpAuxPoint;
		bpAuxPoint++;
		dwValue = ((dwJohn << 8) | *bpAuxPoint);
		*dwpTableAddr = dwValue;
		bpAuxPoint++;
		dwpTableAddr += 7;
		JpegDelay(1);
	}


	// fill the Huffman Base table
	dwpTableAddr = (DWORD *) (&cdu->CduHdbt[0]);
	bpAuxPoint = (BYTE *) pwBaseTable;
	for (i = 0; i < 64; i++)
	{
		dwValue = (DWORD) ((*((WORD *) bpAuxPoint)) & 0x01ff);
		*dwpTableAddr = dwValue;
		dwpTableAddr++;
		bpAuxPoint += 2;
		JpegDelay(1);
	}

	// fill the Huffman Symbol table
	dwpTableAddr = (DWORD *) (&cdu->CduHet[0]);
	bpAuxPoint = pbSymbolTable;
	for (i = 0; i < 336; i++)
	{
		dwValue = (DWORD) (*bpAuxPoint);
		*dwpTableAddr = dwValue;	
		dwpTableAddr++;
		bpAuxPoint++;
		JpegDelay(1);
	}	
}

//--------------------------------------------------------------------------------------------

#if ( SET_RATIO_BY_BUF_WIDTH )
/*RatioByTrgBufWidthFlag, inital value must be ZERO.  */
/* 0 = Skip  to set ResizeRatio by TrgBufWidth*/
/* 1 =  */
#pragma alignvar(4)
int  g_Retresult = FAIL;
WORD g_wTrgBufWidth;
WORD g_wTrgBufHeight;
WORD g_wFinalBufWidth;
WORD g_wFinalBufHeight;

BYTE g_bRatioByTrgBufWidthFlag = 0; 
BYTE *g_BufOutPtr;
BYTE f1;
BYTE f2;



        
int Jpg_OpenRatio_ByTrgBufWidth(BYTE *bBufOutPtr, WORD wTrgBufWidth, WORD wTrgBufHeight )
{
	if(bBufOutPtr == NULL)
	{
		g_bRatioByTrgBufWidthFlag = 0;
		g_BufOutPtr = NULL;
		MP_ALERT("%s:g_BufOutPtr = NULL!!",__FUNCTION__ );
		return FAIL;
	}
	g_bRatioByTrgBufWidthFlag = 1;
	g_wTrgBufWidth  = wTrgBufWidth;
	g_wTrgBufHeight = wTrgBufHeight;
	g_BufOutPtr     = bBufOutPtr;

	return PASS;
}

#if 0
int Jpg_CloseRatio_ByTrgBufWidth(WORD *wFinalBufWidth, WORD *wFinalBufHeight)
{
	g_bRatioByTrgBufWidthFlag = 0; 
	*wFinalBufWidth  = g_wFinalBufWidth;
	*wFinalBufHeight = g_wFinalBufHeight;

	MP_DEBUG1("wFinalBufWidth=%d",*wFinalBufWidth);
	
	return g_Retresult;
}
#else
int Jpg_CloseRatio_ByTrgBufWidth(WORD *wFinalBufWidth, WORD *wFinalBufHeight)
{
  g_bRatioByTrgBufWidthFlag = 0; 
 
  if((g_wFinalBufWidth <= 0) || (g_wFinalBufHeight <= 0))
  {
    MP_ALERT("%s: g_wFinalBufWidth or g_wFinalBufHeight <= ZERO",__FUNCTION__);
	goto Error;
  }
 
  if(g_Retresult == FAIL)
  {
    goto Error;
  }
  else
  {
    *wFinalBufWidth   = g_wFinalBufWidth;
    *wFinalBufHeight  = g_wFinalBufHeight;
     //g_Retresult = FAIL; /*Check it is PASS,  so change it to FAIL. return Value = PASS*/
    return PASS;
  } 
 
 Error:
  g_wFinalBufWidth  = 0;/*If FAIL, set W =0 , H =0*/
  g_wFinalBufHeight = 0;
  *wFinalBufWidth   = g_wFinalBufWidth;
  *wFinalBufHeight  = g_wFinalBufHeight;
  //g_Retresult = FAIL;
  return FAIL;
}


#endif

#if 1  //For Smart copy Version2
/*Start: For Smart copy Version2 ---------------------------------------------*/
int GetResizeTrgBuf(BYTE *Bufin, WORD wSrcWidth, WORD wSrcHeight,WORD wSrcImgWidth, WORD wSrcImgHeight ,WORD wGetwImageType)
{
	ST_IMGWIN * srcWin;
	ST_IMGWIN * trgWin;
	int iErrorCode = PASS;

	DWORD RatioX1000,RatioY1000,UsedRatio1000;

	DWORD wImgWidth1000;
	DWORD wImgHeight1000;
	DWORD dwGetTrgBufWidth1;
	DWORD dwGetTrgBufHeight1;
	DWORD dwTrgBufX;
	DWORD dwTrgBufY;
	DWORD dwGetFinalBufWidth;
	DWORD dwGetFinalBufHeight;

	DWORD dwSrcImgX;
	DWORD dwSrcImgY;
	
	DWORD dwSrcImageOwnRatio;
	BYTE  bHoriFlag;

    MP_DEBUG1("---Bufin         = 0x%x", Bufin);
    MP_DEBUG1("---wSrcWidth     = %d", wSrcWidth);
	MP_DEBUG1("---wSrcHeight    = %d", wSrcHeight);
	MP_DEBUG1("---wGetwImageType= %d", wGetwImageType);
	MP_DEBUG1("---wImgWidth1000=%d", wImgWidth1000);
	MP_DEBUG1("---wImgHeight1000=%d", wImgHeight1000);
	MP_DEBUG1("----g_wTrgBufWidth=%d",g_wTrgBufWidth);
	MP_DEBUG1("----g_wTrgBufHeight=%d",g_wTrgBufHeight);

    if(Bufin == NULL)
    {
		MP_ALERT("%s:Bufin = NULL!!",__FUNCTION__ );
		return FAIL;
    }

	if((g_wTrgBufWidth <=0) || (g_wTrgBufHeight <= 0))
	{
		MP_ALERT("%s:Target Width or Heigh <= 0 ", __FUNCTION__);
		return FAIL;
	}

	if((wSrcWidth <= 0) || (wSrcHeight <= 0) || (wSrcImgWidth <= 0) || (wSrcImgHeight <= 0))
	{
		MP_ALERT("%s: Input parameter is NULL!!",__FUNCTION__);
		return FAIL;
	}
		
    srcWin =(ST_IMGWIN *) ext_mem_malloc(sizeof(ST_IMGWIN));
    trgWin =(ST_IMGWIN *) ext_mem_malloc(sizeof(ST_IMGWIN));

	if((srcWin == NULL) || (trgWin == NULL) || (g_BufOutPtr == NULL))
	{
		MP_ALERT("%s:Alloct memory FAIL!!",__FUNCTION__ );
		iErrorCode = FAIL;
		goto Lable_Error;
	}

	if(iErrorCode == PASS)
	{
	  /*desging: dwGetTrgBufWidth1 > dwGetTrgBufHeight1*/
	  /*|-------|*/
	  /*|             |*/
	  /*|-------|*/
	  /*here : check the dwGetTrgBufWidth1 and dwGetTrgBufHeight1*/
      if(g_wTrgBufWidth >= g_wTrgBufHeight)
      {
	  	dwGetTrgBufWidth1  = g_wTrgBufWidth;
	    dwGetTrgBufHeight1 = g_wTrgBufHeight;
		MP_DEBUG("Find g_wTrgBufWidth > g_wTrgBufHeight");
      }
	  else
	  {
	  	/*Find g_wTrgBufHeight > g_wTrgBufWidth. Need chage*/
	  	dwGetTrgBufWidth1  = g_wTrgBufHeight;
	    dwGetTrgBufHeight1 = g_wTrgBufWidth;
		MP_DEBUG("Find g_wTrgBufHeight > g_wTrgBufWidth.");
		MP_DEBUG("Change:dwGetTrgBufWidth1=H, dwGetTrgBufHeight1=W");
	  } 
	  
      bHoriFlag = 1;// 1=horizontal , 0=vertical
      if(wSrcImgWidth >= wSrcImgHeight)
      {
	  	bHoriFlag = 1;//horizontal
	  	dwTrgBufX = dwGetTrgBufWidth1;
	    dwTrgBufY = dwGetTrgBufHeight1;
		dwSrcImgX = wSrcImgWidth;
		dwSrcImgY = wSrcImgHeight;
      }
	  else
	  {
	  	bHoriFlag = 0;//vertical, reverse Width, and height
	  	dwTrgBufX = dwGetTrgBufHeight1;
	    dwTrgBufY = dwGetTrgBufWidth1; 
		dwSrcImgX = wSrcImgHeight;
		dwSrcImgY = wSrcImgWidth;
	  }
	  


	  if((wSrcImgWidth * wSrcImgHeight) <= (dwTrgBufX * dwTrgBufY))
	  {
	  	MP_DEBUG1("%s: Here is small photo",__FUNCTION__);

		RatioX1000   = (dwTrgBufX * 1000) / wSrcImgWidth /*dwSrcImgX*/  ;
	    RatioY1000   = (dwTrgBufY * 1000) / wSrcImgHeight/*dwSrcImgY*/ ;

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

		
		dwGetFinalBufWidth   = (wSrcImgWidth * UsedRatio1000 ) / 1000;
  	    dwGetFinalBufHeight  = (wSrcImgHeight * UsedRatio1000) / 1000;

		MP_DEBUG2("%s: Smaller UsedRatio(x1000) = %d", __FUNCTION__ , UsedRatio1000);
		MP_DEBUG3("%s: GetFinal(not cut 16 W=%d ,H=%d)", __FUNCTION__ , dwGetFinalBufWidth, dwGetFinalBufHeight);
	
	  }
	  else /*-------------Big Photo -----------------------*/
	  {
        wImgWidth1000   = wSrcImgWidth  * 1000;//<<10;
	    wImgHeight1000  = wSrcImgHeight * 1000;//<<10;
	  	
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

      dwGetFinalBufWidth  = ALIGN_CUT_16(dwGetFinalBufWidth);
	  dwGetFinalBufHeight = ALIGN_CUT_16(dwGetFinalBufHeight);

	  g_wFinalBufWidth = dwGetFinalBufWidth;//ALIGN_16(g_wFinalBufWidth);
	  g_wFinalBufHeight = dwGetFinalBufHeight;//ALIGN_16(g_wFinalBufHeight);
 
      MP_DEBUG1("UsedRatio=%d",UsedRatio1000);
	  MP_DEBUG1("g_wFinalBufWidth=%d",g_wFinalBufWidth);
      MP_DEBUG1("g_wFinalBufHeight=%d",g_wFinalBufHeight);
	  	
	  /*SRC Buf*/	
	  memset(srcWin, 0, sizeof(ST_IMGWIN));
	  memset(trgWin, 0, sizeof(ST_IMGWIN));
			
	  srcWin->pdwStart  = Bufin;
	  srcWin->dwOffset  = (wSrcWidth) * 2;
	  srcWin->wHeight   =	wSrcHeight;
	  srcWin->wWidth	 =	wSrcWidth;
	  srcWin->wX		 = 0;
	  srcWin->wY		 = 0;
	  srcWin->wType	 = wGetwImageType;
	
	  trgWin->pdwStart = g_BufOutPtr;
	  trgWin->dwOffset = g_wFinalBufWidth * 2;
	  trgWin->wHeight  =	g_wFinalBufHeight;
	  trgWin->wWidth	 =	g_wFinalBufWidth;
	  trgWin->wX		 = 0;
	  trgWin->wY		 = 0;
	  trgWin->wType	 = wGetwImageType;
					
	  Ipu_ImageScaling(srcWin, trgWin, 0, 0, wSrcWidth, wSrcHeight, 0, 0, g_wFinalBufWidth, g_wFinalBufHeight, 0);
	}

Lable_Error:	
    if(srcWin != NULL)
	  ext_mem_free(srcWin);
	if(trgWin != NULL)
  	  ext_mem_free(trgWin);

	return iErrorCode;
}
/*End:For Smart copy Version2---------------------------------*/
#endif


#endif
/*----------------------------------------------------------*/

BYTE GetRatioOver(DWORD dwImgWidth,DWORD dwTargetWidth)
{
	DWORD temp;
	DWORD dwModValue;
	DWORD dwRatioSrc2Trg;

    dwModValue = dwImgWidth % dwTargetWidth;
	if(dwModValue != 0)
	  dwRatioSrc2Trg = (dwImgWidth/dwTargetWidth)+1;//franklin debug //+1;
	else
	  dwRatioSrc2Trg = (dwImgWidth/dwTargetWidth);

    if(dwRatioSrc2Trg > 8) // 1/8
    {
	  temp = dwRatioSrc2Trg;
    }
	else
	{
	  temp = (dwImgWidth/dwTargetWidth);
	}

	MP_DEBUG3("%s:dwRatioSrc2Trg=%d, GetRatio=%d",  __FUNCTION__,dwRatioSrc2Trg, temp);

	if(temp<=1)
	  return 0;
	else if(temp<=2)
	  return 1;
	else if(temp<=4)
	  return 2;
	else if(temp<=8)
	  return 3;
	else if(temp<=16)
	  return 4;
	else if(temp<=32)
	  return 5;
	else if(temp<=64)
	  return 6;
	else
	  return 7;	
}



// if width or height more than 4k, resize 1/4
// else if more than 2k, resize 1/2
// else not resize

BYTE Jpg_GetResizeRatio(WORD w, WORD h, DWORD dwTargetSize)
{	
    register struct ST_IMAGE_PLAYER_TAG *sImagePlayer = &(g_psSystemConfig->sImagePlayer);

#if CHASE_IN_DECODE
	if (GetNeedChase())
		return 0;
#endif
#if CUT_IMAGE_HEIGHT_ENALBE
	if (st_wDecodeWinHeight)
		return 0;
#endif
    dwTargetSize >>= 1;

    if (ALIGN_16(w) * (h) <= dwTargetSize)
    {
    	if (sImagePlayer->dwZoomInitFlag)
    		return 0;
		else if (h > 4000) // h too long scale down more, because most of time screen height <= 600
			return 2;
		else if (h > 2000)
			return 1;
		else
			return 0;   

    }
    else if (((w >> 1) * (h >> 1)) <= dwTargetSize)
    {
		
        if((w >> 1)>CDU_650_MAX_CHASE_TARGET_WIDTH)
        {
		   return GetRatioOver(w,CDU_650_MAX_CHASE_TARGET_WIDTH);
        }
		else
		{
	       return 1;
		}
    }
    else if (((w >> 2) * (h >> 2)) <= dwTargetSize)
    {
		
        if((w >> 2)>CDU_650_MAX_CHASE_TARGET_WIDTH)
        {
		   return GetRatioOver(w,CDU_650_MAX_CHASE_TARGET_WIDTH);
        }
		else
		{
	       return 2;
		}
    }
    else if (((w >> 3) * (h >> 3)) <= dwTargetSize)
    {
		
        if((w >> 3)>CDU_650_MAX_CHASE_TARGET_WIDTH)
        {
		   return GetRatioOver(w,CDU_650_MAX_CHASE_TARGET_WIDTH);
        }
		else
		{
	       return 3;
		}
    }   
    else if (((w >> 4) * (h >> 4)) <= dwTargetSize)
	{
		
        if((w >> 4)>CDU_650_MAX_CHASE_TARGET_WIDTH)
        {
		   return GetRatioOver(w,CDU_650_MAX_CHASE_TARGET_WIDTH);
        }
		else
		{
	       return 4;
		}
	}
    else if (((w >> 5) * (h >> 5)) <= dwTargetSize)
	{
		
        if((w >> 5)>CDU_650_MAX_CHASE_TARGET_WIDTH)
        {
		   return GetRatioOver(w,CDU_650_MAX_CHASE_TARGET_WIDTH);
        }
		else
		{
	       return 5;
		}
	}
    else if (((w >> 6) * (h >> 6)) <= dwTargetSize)
	{
        if((w >> 6)>CDU_650_MAX_CHASE_TARGET_WIDTH)
        {
		   return GetRatioOver(w,CDU_650_MAX_CHASE_TARGET_WIDTH);
        }
		else
		{
	       return 6;
		}
	}
    else if (((w >> 7) * (h >> 7)) <= dwTargetSize)
    {
        if((w >> 7)>CDU_650_MAX_CHASE_TARGET_WIDTH)
        {
		   return GetRatioOver(w,CDU_650_MAX_CHASE_TARGET_WIDTH);
        }
		else
		{
	       return 7;
		}
    } 
    MP_ALERT("-E-  can't scale down over 256");
    return 8;
}



#if (YUV444_ENABLE)

BYTE Jpg_GetResizeRatio_444(WORD w, WORD h, DWORD dwTargetSize)
{	
    register struct ST_IMAGE_PLAYER_TAG *sImagePlayer = &(g_psSystemConfig->sImagePlayer);

    dwTargetSize = dwTargetSize / 3;

    if (ALIGN_16(w) * (h) <= dwTargetSize)
    {
    	if (sImagePlayer->dwZoomInitFlag)
    		return 0;
		else if (h > 4000) // h too long scale down more, because most of time screen height <= 600
			return 2;
		else if (h > 2000)
			return 1;
		else
			return 0;   

    }
    else if (((w >> 1) * (h >> 1)) <= dwTargetSize)
    {
		
        if((w >> 1)>CDU_650_MAX_CHASE_TARGET_WIDTH_444)
        {
		   return GetRatioOver(w,CDU_650_MAX_CHASE_TARGET_WIDTH_444);
        }
		else
		{
	       return 1;
		}
    }
    else if (((w >> 2) * (h >> 2)) <= dwTargetSize)
    {
		
        if((w >> 2)>CDU_650_MAX_CHASE_TARGET_WIDTH_444)
        {
		   return GetRatioOver(w,CDU_650_MAX_CHASE_TARGET_WIDTH_444);
        }
		else
		{
	       return 2;
		}
    }
    else if (((w >> 3) * (h >> 3)) <= dwTargetSize)
    {
		
        if((w >> 3)>CDU_650_MAX_CHASE_TARGET_WIDTH_444)
        {
		   return GetRatioOver(w,CDU_650_MAX_CHASE_TARGET_WIDTH_444);
        }
		else
		{
	       return 3;
		}
    }   
    else if (((w >> 4) * (h >> 4)) <= dwTargetSize)
	{
		
        if((w >> 4)>CDU_650_MAX_CHASE_TARGET_WIDTH_444)
        {
		   return GetRatioOver(w,CDU_650_MAX_CHASE_TARGET_WIDTH_444);
        }
		else
		{
	       return 4;
		}
	}
    else if (((w >> 5) * (h >> 5)) <= dwTargetSize)
	{
		
        if((w >> 5)>CDU_650_MAX_CHASE_TARGET_WIDTH_444)
        {
		   return GetRatioOver(w,CDU_650_MAX_CHASE_TARGET_WIDTH_444);
        }
		else
		{
	       return 5;
		}
	}
    else if (((w >> 6) * (h >> 6)) <= dwTargetSize)
	{
        if((w >> 6)>CDU_650_MAX_CHASE_TARGET_WIDTH_444)
        {
		   return GetRatioOver(w,CDU_650_MAX_CHASE_TARGET_WIDTH_444);
        }
		else
		{
	       return 6;
		}
	}
    else if (((w >> 7) * (h >> 7)) <= dwTargetSize)
    {
        if((w >> 7)>CDU_650_MAX_CHASE_TARGET_WIDTH_444)
        {
		   return GetRatioOver(w,CDU_650_MAX_CHASE_TARGET_WIDTH_444);
        }
		else
		{
	       return 7;
		}
    } 
    MP_ALERT("-E-  can't scale down over 256");
    return 8;
}
#endif



//--------------------------------------------------------------------------------------------
int Jpg_CheckImageSize(ST_JPEG *psJpeg, DWORD dwTargetSize)
{
	DWORD dwJohn;
	WORD wMcuHeightMask, wMcuWidthMask;
	WORD wCellUnitSize;
	WORD wCduWidth, wCduHeight, wCduResidue;
	volatile WORD wTargetWidth, wTargetHeight;
		
	// get max vertical sampling factor and max horizontal sampling factor
	// and save them in mask type
	dwJohn = psJpeg->dwCduControl & VFMT_MASK;
 
	if (dwJohn == VIDEO_422)
	{
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))

		if(psJpeg->b422V)
		{
			wMcuHeightMask = 15;
			wMcuWidthMask = 7;
		}
		
#else
		if (psJpeg->blNintyDegreeFlag)
		{
			wMcuHeightMask = 15;
			wMcuWidthMask = 7;
		}
#endif
		else
		{
			wMcuHeightMask = 7;
			wMcuWidthMask = 15;
		}

		wCellUnitSize = 8;
	}
	else if (dwJohn == VIDEO_420)
	{
		wMcuHeightMask = 15;
		wMcuWidthMask = 15;
		wCellUnitSize = 16;
	}
	else
	{
		wMcuHeightMask = 7;
		wMcuWidthMask = 7;
		wCellUnitSize = 8;
	}

	wTargetWidth = psJpeg->wImageWidth;
	wTargetHeight = psJpeg->wImageHeight;
	
	// convert the actual image height and width to be the multiples of MCU size
	if (wTargetHeight & wMcuHeightMask)
		wTargetHeight = (wTargetHeight | wMcuHeightMask) + 1;

	if (wTargetWidth & wMcuWidthMask)
		wTargetWidth = (wTargetWidth | wMcuWidthMask) + 1;
/*
	dwJohn = wTargetWidth * wTargetHeight * 2;
	if (dwJohn > 0x8000000)
	{
		MP_DEBUG1("Pixels count can't exceed 64M %d", dwJohn);
		return NOT_SUPPORTED_FRAME_SIZE;				// pixels count can't exceed 64M
	}
*/

	if (psJpeg->blNintyDegreeFlag)
	{
		psJpeg->blSpecialSizeFlag = 1;
		wTargetWidth <<= 1;
		wTargetHeight >>= 1;
	}
	MP_DEBUG("format %d, wMcuWidthMask %d, wMcuHeightMask %d, wCellUnitSize %d", psJpeg->dwCduControl & VFMT_MASK, wMcuWidthMask, wMcuHeightMask, wCellUnitSize);
	
	MP_DEBUG("wTargetWidth %d, wTargetHeight %d", wTargetWidth, wTargetHeight);
/*	
	if (wTargetWidth > 0x2000 || wTargetHeight > 0x2000)
	{							//8k x 8k
		psJpeg->blSpecialSizeFlag = 1;
		wCduWidth = 0x2000;
		wCduHeight = dwJohn >> 13;
		wCduHeight = wCduHeight - (wCduHeight & (wCellUnitSize - 1));

		dwJohn = dwJohn - wCduWidth * wCduHeight;

		wCduResidue = dwJohn / wCellUnitSize;
	} 
	else 
*/	{
		wCduWidth = wTargetWidth;
		wCduHeight = wTargetHeight;
		wCduResidue = 0;	
	}
	
/*	 
	if (wCduWidth > 0x2000 || wCduHeight > 0x2000)
	{							//8kx8k
		//check image size error
		MP_DEBUG("Image too big");
		return NOT_SUPPORTED_FRAME_SIZE;
	}
*/
	if (wTargetWidth == 0 || wTargetHeight == 0) {
		//check image size error
		MP_DEBUG("Image size = 0");
		return NOT_SUPPORTED_FRAME_SIZE;
	}
	
	if (psJpeg->blNintyDegreeFlag)
	{
		wTargetWidth >>= 1;
		wTargetHeight <<= 1;
	}

	psJpeg->wTargetWidth = wTargetWidth;
	psJpeg->wTargetHeight = wTargetHeight;
	
	psJpeg->wCduWidth = wCduWidth;
	psJpeg->wCduHeight = wCduHeight;

	psJpeg->wCduResidue = wCduResidue;

	psJpeg->wCellSize = wCellUnitSize;

#if (YUV444_ENABLE)
	if(JpegDecoder444 == 1)
	{
#if (SET_FIXED_RATIO_444_422)		
		if(SetFixedRatio444_422Flag != 1)
		{
			if(psJpeg->wImageType==444)
				psJpeg->bJResizeRatio = Jpg_GetResizeRatio_444(wCduWidth, wCduHeight, dwTargetSize);
			else			
				psJpeg->bJResizeRatio = Jpg_GetResizeRatio(wCduWidth, wCduHeight, dwTargetSize);
		}else
		{			
			psJpeg->bJResizeRatio = Jpg_GetResizeRatio_444(wCduWidth, wCduHeight, dwTargetSize);
		}
#else		
		if(psJpeg->wImageType==444)
			psJpeg->bJResizeRatio = Jpg_GetResizeRatio_444(wCduWidth, wCduHeight, dwTargetSize);
		else			
			psJpeg->bJResizeRatio = Jpg_GetResizeRatio(wCduWidth, wCduHeight, dwTargetSize);
#endif		
	}
	else
	{
		psJpeg->bJResizeRatio = Jpg_GetResizeRatio(wCduWidth, wCduHeight, dwTargetSize);
	}
#else
	psJpeg->bJResizeRatio = Jpg_GetResizeRatio(wCduWidth, wCduHeight, dwTargetSize);
#endif
	if (psJpeg->bJResizeRatio >= 8) return NOT_SUPPORTED_FRAME_SIZE;
/*
	if((psJpeg->dwCduControl & VFMT_MASK) == VIDEO_444)		
		if (psJpeg->bJResizeRatio == 3)
			return FAIL;
*/
	return PASS;
}

//--------------------------------------------------------------------------------------------
void Jpg_SetDecodedSize(ST_JPEG *psJpeg)
{
	// Set jpeg global variables by resize result
	BYTE bJResizeRatio = psJpeg->bJResizeRatio;
	BYTE tmp = psJpeg->wImageWidth & 1;
	MP_DEBUG("##psJpeg->wTargetWidth=%d", psJpeg->wTargetWidth);
	MP_DEBUG("##psJpeg->wImageWidth=%d", psJpeg->wImageWidth);
	
	//Set Decode width
	psJpeg->wTargetWidth >>= bJResizeRatio;
	psJpeg->wTargetHeight >>= bJResizeRatio;
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	if(bJResizeRatio > 3)
	{
		psJpeg->wTargetWidth = ALIGN_2(psJpeg->wImageWidth >> bJResizeRatio) ;
	}
	else
	{
		if(psJpeg->wTargetWidth & 0x1)
		{
			psJpeg->wTargetWidth++ ;	//	align 2 
		}
	}
#else
	if(psJpeg->wTargetWidth & 0x1)
	{
		psJpeg->wTargetWidth++ ;	//	align 2 
	}
#endif
	//Set valid image width
	psJpeg->wRealTargetWidth = psJpeg->wImageWidth >> bJResizeRatio;
	psJpeg->wRealTargetHeight = psJpeg->wImageHeight >> bJResizeRatio;
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	if(bJResizeRatio > 3)
	{
		psJpeg->wRealTargetWidth = ALIGN_2(psJpeg->wImageWidth >> bJResizeRatio) ;
	}
	else
	{
		if (psJpeg->wImageWidth-(psJpeg->wRealTargetWidth<<bJResizeRatio))
		{ 
			psJpeg->wRealTargetWidth++;
		}
	}
#else	
	if (psJpeg->wImageWidth-(psJpeg->wRealTargetWidth<<bJResizeRatio))
		psJpeg->wRealTargetWidth++;
#endif
	psJpeg->wCduWidth >>= bJResizeRatio;	// Alex Test, 060401
	psJpeg->wCduHeight >>= bJResizeRatio;
	//if ((psJpeg->bDecodeMode & 0x3f) != IMG_DECODE_MJPEG) {
		//psJpeg->wTargetWidth = psJpeg->wCduWidth;
		//psJpeg->wTargetHeight = psJpeg->wCduHeight;		
	//}
	
	psJpeg->wCellSize >>= bJResizeRatio;

	//prevent 16-divisible image width that isn't really 16-divisible because of no decimal places
	if ((bJResizeRatio) && ((psJpeg->wImageWidth & 0xf) == 0) && tmp)
		psJpeg->wImageWidth += 2;

#if (YUV444_ENABLE)
	if(JpegDecoder444 == 1)
	{
		if(psJpeg->wImageType == 444)
		{
			psJpeg->wTargetWidth = ALIGN_4(psJpeg->wTargetWidth);
		}
	}
#endif

}


void Jpg_SetRotateDecodedSize(ST_JPEG *psJpeg)
{
	BYTE bJResizeRatio = psJpeg->bJResizeRatio;
	WORD temp_width = 0 ;
	//Set Decode width
	temp_width = psJpeg->wTargetWidth ;
	psJpeg->wTargetWidth = psJpeg->wTargetHeight >> bJResizeRatio ;
	psJpeg->wTargetHeight = temp_width >> bJResizeRatio ;
	if(bJResizeRatio > 3)
	{
		psJpeg->wTargetWidth = ALIGN_2(psJpeg->wImageHeight >> bJResizeRatio) ;
	}
	else
	{
		if(psJpeg->wTargetWidth & 0x1)
			psJpeg->wTargetWidth++ ;	//	align 2 
	}
	
	//Set valid image width
	psJpeg->wRealTargetWidth = psJpeg->wImageHeight >> bJResizeRatio;
	psJpeg->wRealTargetHeight = psJpeg->wImageWidth >> bJResizeRatio;
	if(bJResizeRatio > 3)
	{
		psJpeg->wRealTargetWidth = ALIGN_2(psJpeg->wImageHeight >> bJResizeRatio) ;
	}
	else
	{
		if(psJpeg->wImageHeight-(psJpeg->wRealTargetWidth<<bJResizeRatio))
		{ 
			psJpeg->wRealTargetWidth++;
		}
	}
	
	temp_width = psJpeg->wCduWidth ;
	psJpeg->wCduWidth = psJpeg->wCduHeight >> bJResizeRatio;	
	psJpeg->wCduHeight = temp_width >> bJResizeRatio;

}


//--------------------------------------------------------------------------------------------
int Jpg_PollEvent(ST_JPEG *psJpeg)
{
	//if ((psJpeg->bDecodeMode & 0x3f) == IMG_DECODE_SLIDE)
	{
		if (Polling_Event())
		{
			TurnOffIPUClk();
			return FAIL;
		}
		//if (!(g_bAniFlag & ANI_SLIDE) )
		//return FAIL;  //byAlexWang 13jun2007 m2project

	}
	return PASS;
}

//--------------------------------------------------------------------------------------------

void Jpg_CheckComplete()
{
	if (CduCheckEnabled()) 
		Jpg_Cdu_Wait(JPEG_DECODE_TIMEOUT_CNT, 1);
}	

//--------------------------------------------------------------------------------------------
int Jpg_Cdu_Wait(DWORD dwTimeoutCount, BOOL boTaskYield)
{
	CDU *cdu = (CDU *) (CDU_BASE);
	
	while (dwTimeoutCount--)
	{
		if (cdu->CduIc & 0x1)				
			return PASS;
		
		if (boTaskYield)
			TaskYield();
		
		//if (Jpg_PollEvent(NULL)) 
		//	return FAIL; 
	}
    
	if (cdu->CduIc & 0x1)				
		return PASS;

	MP_ALERT("-E-Jpeg:JPEG_DECODE_TIMEOUT\r\n");	
	return JPEG_DECODE_TIMEOUT; 
}

int Jpg_Cdu_Wait_Time(DWORD dwTimeout, BOOL boTaskYield)
{
	CDU *cdu = (CDU *) (CDU_BASE);

	DWORD currTime = GetSysTime();
	while (SystemGetElapsedTime(currTime) < dwTimeout)
	{
		if (cdu->CduIc & 0x1)				
			return PASS;
		
		if (boTaskYield)
			TaskYield();
		
		//if (Jpg_PollEvent(NULL)) 
		//	return FAIL; 
	}
    
	if (cdu->CduIc & 0x1)				
		return PASS;

	MP_ALERT("-E-Jpeg:JPEG_DECODE_TIMEOUT\r\n");	
	return JPEG_DECODE_TIMEOUT; 
}

//--------------------------------------------------------------------------------------------
#if ((CHIP_VER & 0xffff0000) == CHIP_VER_615 || (CHIP_VER & 0xffff0000) == CHIP_VER_612)
int Jpg_Cdu_Decode_One(ST_MPX_STREAM *psStream, ST_JPEG *psJpeg, BYTE *pbTarget)
{
	DWORD dwStartPoint = psJpeg->dwEcsStart + (DWORD)psStream->buffer;
	DWORD dwEndPoint = psJpeg->dwEoiPoint + (DWORD)psStream->buffer;

	//if (dwEndPoint & 0x7) dwEndPoint += 8 - (dwEndPoint & 0x7);
	
	MP_DEBUG2("Jpg_Cdu_Decode_One dwStartPoint %08x dwEndPoint %08x", dwStartPoint, dwEndPoint);	
	
    psJpeg->dwCduControl &= ~(0x00000300);
	psJpeg->dwCduControl |= (psJpeg->dwEcsStart & 3) << 8;	// get CDU_DBS

	//MP_DEBUG("Jpg_Cdu_Init");	
	CDU *cdu = (CDU *) (CDU_BASE);
	cdu->CduIc = 0;
	cdu->CduSize = ((psJpeg->wCduHeight - 1) << 16) | (psJpeg->wCduWidth - 1);
	cdu->CduC = psJpeg->dwCduControl | ((psJpeg->bJResizeRatio & 0x03) << 2);

	CHANNEL *Ch_Jvlc = (CHANNEL *) (DMA_JVLC_BASE);
	Ch_Jvlc->StartA = dwStartPoint & 0xfffffffc;
	Ch_Jvlc->EndA = dwEndPoint;
	Ch_Jvlc->Control = 1;

	BYTE bJResizeRatio = psJpeg->bJResizeRatio;
	WORD wResizeWidth;
	WORD wResizeHeight;	

	if ((psJpeg->bDecodeMode & 0x3f) == IMG_DECODE_MJPEG)
	{
		wResizeWidth = psJpeg->wTargetWidth >> bJResizeRatio;
		wResizeHeight = psJpeg->wTargetHeight>> bJResizeRatio;		
	}
	else
	{
		wResizeWidth = psJpeg->wCduWidth >> bJResizeRatio;
		wResizeHeight = psJpeg->wCduHeight >> bJResizeRatio;		
	}
		
	DWORD downSizeOffset = (((wResizeWidth + 15) & 0xfff0) - wResizeWidth) + psJpeg->dwOffset;	
	CHANNEL *Ch_JMCU = (CHANNEL *) (DMA_JMCU_BASE);
	Ch_JMCU->StartA = (DWORD) pbTarget;
	Ch_JMCU->EndA = (DWORD) pbTarget + (wResizeHeight * ((wResizeWidth + downSizeOffset) << 1)) - 1;	
	Ch_JMCU->LineCount = (downSizeOffset << 17) + ((wResizeWidth << 1) - 1);
	Ch_JMCU->Control = 5;

	// enable CDU operation
	cdu->CduOp = 1;
	
//	if(CduWaitReady)
    if(((psJpeg->bDecodeMode & 0x3f) != IMG_DECODE_MJPEG) ||
	   (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PREVIEW)
	  ) 
	    Jpg_Cdu_Wait(JPEG_DECODE_TIMEOUT_CNT, 1);
///	else
		return 0;

}
#endif

BYTE *g_pbChaseBuffer = NULL;
//--------------------------------------------------------------------------------------------
int Jpg_Cdu_Decode_Chase(ST_MPX_STREAM *psStream, ST_JPEG *psJpeg, BYTE *pbTarget)
{
	DWORD i, n, h;
	BYTE *pBuffer = NULL;	

	MP_DEBUG("Jpg_Cdu_Chase file size %d, eoipoint 0x%08x", mpxStreamGetSize(psStream), psJpeg->dwEoiPoint);	


	BYTE bJResizeRatio = psJpeg->bJResizeRatio;
	WORD wResizeWidth;
	WORD wResizeHeight;	

	if ((psJpeg->bDecodeMode & 0x3f) == IMG_DECODE_MJPEG)
	{
		wResizeWidth = psJpeg->wTargetWidth >> bJResizeRatio;
		wResizeHeight = psJpeg->wTargetHeight >> bJResizeRatio;		
	}
	else
	{
		//if (psJpeg->wTargetWidth> 8192) return Jpg_Cdu_Decode_DChase_Wild_and_High2(psStream, psJpeg, pbTarget);
		//if (psJpeg->wTargetHeight > 8192) return Jpg_Cdu_Decode_DChase_High(psStream, psJpeg, pbTarget);
		if(psJpeg->wTargetWidth>8192 || psJpeg->wTargetHeight > 8192) return FAIL;
		if (bJResizeRatio>2) return Jpg_Cdu_Decode_DChase(psStream, psJpeg, pbTarget);	
		wResizeWidth = psJpeg->wCduWidth >> bJResizeRatio;
		wResizeHeight = psJpeg->wCduHeight >> bJResizeRatio;	
	}

	CDU *cdu = (CDU *) (CDU_BASE);
	cdu->CduIc = 0;
	cdu->CduSize = ((psJpeg->wCduHeight - 1) << 16) | (psJpeg->wCduWidth - 1);
	cdu->CduC = psJpeg->dwCduControl | ((psJpeg->bJResizeRatio & 0x03) << 2);
	MP_DEBUG("psJpeg->wImageType=%d, psJpeg->dwCduControl= 0x%08x, psJpeg->bJResizeRatio= 0x%08x", psJpeg->wImageType, psJpeg->dwCduControl, psJpeg->bJResizeRatio);
	
	psJpeg->dwOffset=(DWORD)((DWORD)(psJpeg->wCduWidth) - (DWORD)(wResizeWidth << bJResizeRatio));

	MP_DEBUG("psJpeg->wOriginalWidth %d, psJpeg->wOriginalHeight %d \n\r bJResizeRatio  %d", psJpeg->wOriginalWidth, psJpeg->wOriginalHeight,bJResizeRatio);	
	MP_DEBUG("psJpeg->wImageWidth %d, psJpeg->wImageHeight %d", psJpeg->wImageWidth, psJpeg->wImageHeight);	
	
	DWORD dwLineOffset = (ALIGN_16(wResizeWidth + psJpeg->dwOffset)) << 1;	
	DWORD downSizeOffset =  dwLineOffset - (wResizeWidth << 1);	
	
	CHANNEL *Ch_JMCU = (CHANNEL *) (DMA_JMCU_BASE);
	CHANNEL *Ch_Jvlc = (CHANNEL *) (DMA_JVLC_BASE);

	DWORD dwStartPoint = psJpeg->dwEcsStart;
	DWORD dwTotalSize = psJpeg->dwEoiPoint - dwStartPoint;

	DWORD dwSize_1 = JPEG_CHASING_BUFFER_SIZE; // the buffer size could be different for photo size
	DWORD dwSrcBufferSize;
    
         dwSrcBufferSize = dwSize_1 * JPEG_CHASING_BUFFER_COUNT; //byAlexWang 27jun2007 m2project
		
	     switch((int) psJpeg->wCduWidth / 512)
	     {
		    case 6:

		    case 5:
			    i=3;
			    break;
		    case 4:

		    case 3:
		    case 2:	
			    i=3;
			    break;
		    case 1:
		    case 0:
			    i=4;
			    break;
		    default:
			    i=3;
      }   
	if(psJpeg->wCellSize == 16)	
		i = 4;


	h = 1 << i;
	n = wResizeHeight >> i;			
	n++;
	
	DWORD t;	
	DWORD dwPos = 0;
	DWORD dwDecodeSize = 0;

	if (dwSrcBufferSize >= dwTotalSize) dwSrcBufferSize = dwTotalSize;
	if ((psJpeg->bDecodeMode & 0x3f) != IMG_DECODE_MJPEG)
	   pBuffer = (BYTE *)ext_mem_malloc(dwSrcBufferSize + 256);
	else {
	   if (g_pbChaseBuffer == NULL) g_pbChaseBuffer = (BYTE *)ext_mem_malloc(dwSrcBufferSize + 256);
	   pBuffer = g_pbChaseBuffer;
	}
	
	if (pBuffer == NULL){
		MP_ALERT("alloc pBuffer fail");
		return FAIL;
	}
	psJpeg->dwCduControl |= ((DWORD)pBuffer & 0x00000003) << 8;	// get CDU_DBS
	pBuffer = (BYTE *)((DWORD)pBuffer | 0xa0000000);
	//memset(pBuffer, 0, dwSrcBufferSize);
	
	DWORD dwReadSize;

	if (mpxStreamSeek(psStream, dwStartPoint, SEEK_SET) == FAIL) {
		MP_DEBUG("-E- mpxStreamSeek(psStream, dwStartPoint, SEEK_SET) == FAIL");
		ext_mem_free((BYTE *)((DWORD)pBuffer & ~0x20000000));
		return FAIL;
	}
	dwReadSize = mpxStreamRead(pBuffer, 1, dwSrcBufferSize, psStream);
	
	Ch_Jvlc->StartA = (DWORD)pBuffer;
	Ch_Jvlc->EndA = (DWORD)pBuffer + dwSrcBufferSize - 1;
	Ch_Jvlc->Control = 3;
	
	dwStartPoint = 0;
	DWORD dwPrevCurrent = Ch_Jvlc->StartA & 0xfffffff;

	DWORD dwTargetStart = (DWORD) psJpeg->pbTarget;
	DWORD dwTargetEnd = (DWORD) psJpeg->pbTarget + dwLineOffset * wResizeHeight - downSizeOffset;//+ psJpeg->dwTargetSize;
	int iErrorCount = 1;  //byAlexWang 12jun2007 m2project
	MP_DEBUG("dwLineOffset is %d, h is %d, n is %d", dwLineOffset, h, n);		

	for (i = 0; i < n; i++) 
	{		
	   MP_DEBUG("Loop i:%d", i);
		if (wResizeHeight < h) h = wResizeHeight;	
		//MP_DEBUG1("decode pos %08x", dwStartPoint);		
//		MP_DEBUG1("decode pos dwStartPoint 0x%08x", dwStartPoint);		
//		MP_DEBUG("wResizeHeight is %d, \n\r i is %d, n is 0x%08x", wResizeHeight, i, n);		
//		MP_DEBUG("dwReadSize is 0x%08x, \n\r pbTarget is 0x%08x, dwPrevCurrent is 0x%08x", dwReadSize,  pbTarget, dwPrevCurrent);		
	
		Ch_JMCU->StartA = (DWORD) pbTarget;
		Ch_JMCU->EndA = (DWORD) pbTarget + (h * dwLineOffset) - downSizeOffset - 1;	
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
		Ch_JMCU->LineCount = (0 << 16) + ((wResizeWidth << 1) - 1);
#else
		Ch_JMCU->LineCount = (downSizeOffset << 16) + ((wResizeWidth << 1) - 1);
#endif		
		MP_DEBUG("Ch_JMCU->StartA=0x%08x, Ch_JMCU->LineCount=0x%08x", Ch_JMCU->StartA, Ch_JMCU->LineCount);
		if (Ch_JMCU->StartA >= dwTargetEnd) break;
		if (Ch_JMCU->EndA >= dwTargetEnd) Ch_JMCU->EndA = dwTargetEnd - 1;
		
		Ch_JMCU->Control = 5;
		
		// enable CDU operation
		cdu->CduOp = 1;		

		while (!psStream->eof && (dwReadSize + dwSize_1 < dwStartPoint + dwSrcBufferSize))
		{
			//MP_DEBUG2("read %d, decode point %d", (dwReadSize + dwSize_1) % dwSrcBufferSize, dwStartPoint % dwSrcBufferSize);

			TaskYield();
			if(Jpg_PollEvent(psJpeg))//byAlexWang 12jun2007 m2project
			{
				i=n+30;
				iErrorCount=30;
				break;
			}
			TaskYield();

			dwPos = dwReadSize % dwSrcBufferSize;
			DWORD dwSize = mpxStreamRead(pBuffer + dwPos, 1, dwSize_1, psStream);	
			TaskYield();

			dwReadSize += dwSize;
			if (dwSize < dwSize_1) break; //psStream->eof = 1;
			if (dwSize == 0) psStream->eof = 1;
			//MP_DEBUG1("read size %8x",  dwSize);
			TaskYield();
		}

		t = JPEG_DECODE_TIMEOUT_CNT << 1;	//byAlexWang 12jun2007 m2project
		DWORD new_pos = dwPrevCurrent;
		DWORD t1 = 0;
		while (!(Ch_JMCU->Control & BIT16) && t--)
		{				
			if (g_bAniFlag & ANI_AUDIO) TaskYield();

			if(Jpg_PollEvent(psJpeg))
			{
				i=n+40;
				iErrorCount=40;
				break;
			}

			if (t1 >= JPEG_DECODE_TIMEOUT_CNT/*>>3 0x7fff*/) {  // avoid decode error and always redecode same position
				if (Ch_Jvlc->Current == new_pos)
				{  
					Ch_JMCU->Control = 0;
					iErrorCount=50;
					i = n+50; 
					break; 
				}
				else 
				{
					new_pos = Ch_Jvlc->Current;
					t1 = 0;
				}
			}
			t1++;
		}		
		
		
		Ch_JMCU->Control = 0;

		if (t == 0) break;		

		if (Ch_Jvlc->Current >= dwPrevCurrent)
			dwDecodeSize = Ch_Jvlc->Current - dwPrevCurrent;
		else
			dwDecodeSize = Ch_Jvlc->Current + dwSrcBufferSize - dwPrevCurrent;

		if ((dwDecodeSize == 0)&&(i<(n-3))) {
			iErrorCount++;
			i--;
			if (iErrorCount > 20) break;
			MP_DEBUG("dwDecodeSize %d, current %08x, dwReadSize %08x, dwStartPoint %08x", dwDecodeSize, Ch_Jvlc->Current, dwReadSize, dwStartPoint);					
		} 
		else
		{
			pbTarget += (h * dwLineOffset);
			iErrorCount = 1;
			dwStartPoint += dwDecodeSize;		
			dwPrevCurrent = Ch_Jvlc->Current;		
			wResizeHeight -= h;
            
			if (wResizeHeight <= 0)
			{
				iErrorCount=0;//byAlexWang 12jun2007 m2project
				break;
			}		
		}

		if(Jpg_PollEvent(psJpeg)) //byAlexWang 12jun2007 m2project
		{
			iErrorCount=60;
			break;
		}
		if (((i & 0xf) == 0) && ((psJpeg->bDecodeMode & 0x3f) == IMG_DECODE_SLIDE))
			TaskYield();
	}  // end of for loop
	CduDisable();
	if ((psJpeg->bDecodeMode & 0x3f) != IMG_DECODE_MJPEG)
	ext_mem_free((BYTE *)((DWORD)pBuffer & ~0x20000000));
		MP_DEBUG("at the end  wResizeHeight is %d, i is %d, n is %d", wResizeHeight, i, n);		

#if OUTPUT_JPEG
	if ((psJpeg->bDecodeMode & 0x3f) == IMG_DECODE_PHOTO)

	if(DUMPMEMORY){
		static DRIVE *sDrv;
		static STREAM *shandle;
		static int ret;
		int len = 0;
		WORD width, height;
		width = psJpeg->wTargetWidth >> bJResizeRatio;
		if(width & 0xfff0)
			width = (width + 15) & 0xfff0;
		height = psJpeg->wTargetHeight >> bJResizeRatio;
		len = width * height *2;

		UartOutText("creat jpg.bin");
		UartOutText("width");
		UartOutValue(width, 4);
		UartOutText("height");
		UartOutValue(height, 4);
	    sDrv=DriveGet(SD_MMC);
        ret=CreateFile(sDrv, "jpg", "bin");
        if (ret) UartOutText("create file fail\r\n");
        shandle=FileOpen(sDrv);
        if(!shandle) UartOutText("open file fail\r\n");

		ret=FileWrite(shandle, psJpeg->pbTarget, len);
        if(!ret) UartOutText("write file fail\r\n");

        FileClose(shandle);
        UartOutText("\n\rfile close\n\r");
	}	
#endif
  //iErrorCount = 0 ;
	return iErrorCount;
}

//--------------------------------------------------------------------------------------------
int Jpg_Decoder_CduDecode(ST_JPEG *psJpeg)
{	
	int i;
	ST_MPX_STREAM *psStream;

	MP_ASSERT(psJpeg != NULL);
	MP_ASSERT(psJpeg->psStream != NULL);
	
	psStream = psJpeg->psStream;	
	
	
	MP_DEBUG("Jpg_Decoder_CduDecode");
	Jpg_CheckComplete();  // check cdu status before decode

	if (psJpeg->bDhtHit)
		memset(baSymbolTable, 0, 336);

	CduDecodeEnable();
        if (g_bAniFlag & ANI_SLIDE)
            Polling_Event();
	if (psJpeg->dwCduDri > 0) 
	{
		((CDU *) (CDU_BASE))->CduDri = psJpeg->dwCduDri - 1;
		psJpeg->dwCduControl |= 0x00000010L;
	}
        if (g_bAniFlag & ANI_SLIDE)
            Polling_Event();
	for (i = 0; i < psJpeg->bDqtHit; i++) {
		if (mpxStreamSeek(psStream, psJpeg->dwDqtPos[i], SEEK_SET) == FAIL)
			return FAIL;
		if (Jpg_Marker_DQT(psStream, psJpeg) == FAIL)
			return FAIL;
	}
        if (g_bAniFlag & ANI_SLIDE)
            Polling_Event();
	for (i = 0; i < psJpeg->bDhtHit; i++) {
		if (mpxStreamSeek(psStream, psJpeg->dwDhtPos[i], SEEK_SET) == FAIL)
			return FAIL;
		if (Jpg_Marker_DHT(psStream, psJpeg) == FAIL)
			return FAIL;
	}
        if (g_bAniFlag & ANI_SLIDE)
            Polling_Event();
	Jpg_CheckQtTable(psJpeg);	
	if ((psJpeg->bDecodeMode & 0x3f) != IMG_DECODE_MJPEG) 
	{
		Jpg_InitTables(psJpeg);	
		//Jpg_CheckQtTable(psJpeg);
		//if (Jpg_PollEvent(psJpeg)) return FAIL;
	}	
	
	MP_DEBUG("check image size");
	if (Jpg_CheckImageSize(psJpeg, psJpeg->dwTargetSize))
	{
		MP_DEBUG("NOT_SUPPORTED_FRAME_SIZE");
		psJpeg->iErrorCode = NOT_SUPPORTED_FRAME_SIZE;
		return psJpeg->iErrorCode;
	}
        if (g_bAniFlag & ANI_SLIDE)
            Polling_Event();
	if (psJpeg->bDhtHit)
		Jpg_SetHummanTable(baMinTable, waBaseTable, baSymbolTable);
	else
		Jpg_SetHummanTable(baMinPatchTable, waBasePatchTable, baSymbolPatchTable);

	if ((psJpeg->bDecodeMode & 0x3f) == IMG_DECODE_SLIDE)
		if (Jpg_PollEvent(psJpeg)) return FAIL;

#if CHASE_IN_DECODE
	if (GetNeedChase())
		psJpeg->iErrorCode = Jpg_Cdu_Decode_650(psStream, psJpeg, psJpeg->pbTarget);
	else
#endif
	if ((psStream->fd != NULL) && (psJpeg->bDecodeMode & IMG_DECODE_CHASE))
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
		psJpeg->iErrorCode = Jpg_Cdu_Decode_650(psStream, psJpeg, psJpeg->pbTarget);
#else
		psJpeg->iErrorCode = Jpg_Cdu_Decode_Chase(psStream, psJpeg, psJpeg->pbTarget);
#endif
	else
		psJpeg->iErrorCode = Jpg_Cdu_Decode_One(psStream, psJpeg, psJpeg->pbTarget);
        if (g_bAniFlag & ANI_SLIDE)
            Polling_Event();
  if(psJpeg->bRotate == 0 || psJpeg->bRotate == 2)	// 0 degree or 180 degree
		Jpg_SetDecodedSize(psJpeg);
	else if(psJpeg->bRotate == 1 || psJpeg->bRotate == 3)		// 90 degree or 270 degree
		Jpg_SetRotateDecodedSize(psJpeg);

	MP_DEBUG("Jpg_CduDecode done iErrorCode is %d",psJpeg->iErrorCode);
	return psJpeg->iErrorCode;		
}


//--------------------------------------------------------------------------------------------
ST_JPEG *Jpg_OpenJpegBuffer(BYTE  *pbSource, BYTE *pbTarget, DWORD dwSourceSize, DWORD dwTargetSize, BYTE bMode)
{
	int iRet;
	ST_JPEG *psJpeg;
	ST_MPX_STREAM *psStream;

	MP_DEBUG1("source buffer size 0x%08x", dwSourceSize);
	//MP_DEBUG2("head tag %2x%2x", pbSource[0], pbSource[1]);

	psStream = &g_sJpegStream; 
	
	iRet = mpxStreamOpenBuffer(psStream, pbSource, dwSourceSize);

	if (iRet == FAIL) return NULL;
		
	psJpeg = Jpg_Decoder_Init(psStream, pbTarget, dwTargetSize, bMode, 0);

	return psJpeg;
}


//--------------------------------------------------------------------------------------------
void Jpg_CloseJpegFile(ST_JPEG *psJpeg)
{
	if (psJpeg->psStream) {
		mpxStreamClose(psJpeg->psStream);	
	}
}

//----------------------------------------------------------------------------

WORD Jpg_GetTargetWidth()
{
	return g_psCurJpeg->wTargetWidth;
}

WORD Jpg_GetTargetHeight()
{
	return g_psCurJpeg->wTargetHeight;
}

WORD Jpg_GetImageWidth()
{
	return g_psCurJpeg->wImageWidth;
}

WORD Jpg_GetImageHeight()
{
	return g_psCurJpeg->wImageHeight;
}



DWORD Jpg_GetImageFrameWidth()
{
	return ((g_psCurJpeg->wCduWidth)<<1);
}


WORD Jpg_GetRealTargetWidth()
{
 return g_psCurJpeg->wRealTargetWidth;
} 
 
WORD Jpg_GetReadTargetHeight()
{
 return g_psCurJpeg->wRealTargetHeight;
}


//static BYTE MJ_INIT;
#if MJPEG_ENABLE
BYTE MJ_INIT;
#if MJPEG_TOGGLE
static ST_IMGWIN g_sDecodeWin;
#endif
BYTE Mjpeg_toggle = 0;
BYTE Mjpeg_toggle_r = 0;
BYTE *Mjpeg_Tar = NULL;
BYTE *Mjpeg_Tar_r = NULL;
DWORD Mjpeg_TarSize;
extern int decoder_initialized;

//--------------------------------------------------------------------------------------------
int Mjpeg_Alloc_Target_Buffer(DWORD size, BYTE **pbTarget, BYTE **pbTarget_r)
{
	mpDebugPrint(" ===Mjpeg_Alloc_Target_Buffer %d",size);
    *pbTarget = (BYTE *) ImageAllocTargetBuffer(size);
	if (NULL == *pbTarget)
		return FAIL;
	
    *pbTarget = (BYTE *) ((DWORD) *pbTarget | 0xA0000000);
	
    #if THREE_D_PROJECT	
    *pbTarget_r = (BYTE *) ImageAllocTargetRightBuffer(size);
    if (NULL == *pbTarget_r)
		return FAIL;
	*pbTarget_r = (BYTE *) ((DWORD) *pbTarget_r | 0xA0000000);
	#endif

	return PASS;
}

ST_JPEG *Mjpeg_Decode_Init(ST_MPX_STREAM *psStream, BYTE * pbTarget, BYTE * pbTarget_3D, DWORD dwTargetSize)
{
	MP_DEBUG(" === Mjpeg_Decode_Init");
	#if THREE_D_PROJECT
	g_psCurJpeg = Jpg_Decoder_Init(psStream, pbTarget, pbTarget_3D, dwTargetSize, IMG_DECODE_MJPEG | IMG_DECODE_CHASE, 0);
	#else
	g_psCurJpeg = Jpg_Decoder_Init(psStream, pbTarget, dwTargetSize, IMG_DECODE_MJPEG | IMG_DECODE_CHASE, 0);
	#endif
	#if MJPEG_TOGGLE
	g_sDecodeWin.pdwStart = (DWORD *)((DWORD)g_psCurJpeg->pbTarget | 0xA0000000);
	#endif
	WORD wd = g_psCurJpeg->wTargetWidth;
	
	if (wd != ALIGN_16(wd)) {
		MP_DEBUG("target width not 16 aligned");
	}

	#if MJPEG_TOGGLE
	g_sDecodeWin.dwOffset = ALIGN_16(wd) << 1;
	g_sDecodeWin.wWidth = g_psCurJpeg->wTargetWidth;
	g_sDecodeWin.wHeight = g_psCurJpeg->wTargetHeight;	
    
	
    MP_DEBUG("g_sDecodeWin pdwstart = 0x%08x, dwOffset = %d", g_sDecodeWin.pdwStart, g_sDecodeWin.dwOffset);
    MP_DEBUG("              Width = %d, height =%d", g_sDecodeWin.wWidth, g_sDecodeWin.wHeight);
    #endif
		mpDebugPrint("dwTargetSize = %u", dwTargetSize);
	//g_psCurJpeg->pbTarget = (BYTE *) ((DWORD)ImageAllocTargetBuffer((dwTargetSize *2) + 256) | 0xA0000000);
	if (NULL == ((DWORD)g_psCurJpeg->pbTarget & (~0xA0000000)))
	{
        g_psCurJpeg->iErrorCode = NOT_SUPPORTED_FRAME_SIZE;
	}
	else
	{
	    g_psCurJpeg->iErrorCode = 0;
	}
	Mjpeg_Tar = g_psCurJpeg->pbTarget;
	Mjpeg_TarSize = g_psCurJpeg->dwTargetSize;
    MJ_INIT=0;
	Mjpeg_toggle = 0;
    #if 0//for test 
    int i;
    for (i = 0; i < 100000 && !psStream->eof; i++)
        if (Mjpeg_Decode_Frame(g_psCurJpeg) == FAIL) break;
    #endif    
	return g_psCurJpeg;
}



int Mjpeg_Decode_Frame(ST_JPEG *psJpeg)
{
	ST_MPX_STREAM *psStream;
	g_psCurJpeg = psJpeg;
	MP_DEBUG(" === Mjpeg_Decode_Frame");
    psJpeg->bDqtHit = 0; //temp
    psJpeg->bDhtHit = 0; //temp
	psStream = psJpeg->psStream;
	BYTE bMarker = 0;
	do {
		if (mpxStreamSearchMarker(psStream, MARKER_FIRST) == FAIL) 
			break;
	
		bMarker = mpxStreamGetc(psStream);	// get the second byte of bMarker			
		if (MARKER_FIRST == bMarker)
		{
		    bMarker = mpxStreamGetc(psStream);
		}
		
	} while (bMarker != MARKER_SOI && !psStream->eof);
	
	if (psStream->eof) return FAIL;
	psJpeg->iErrorCode = Jpg_Parse_Marker(psJpeg->psStream, psJpeg);
	
//#if 1//if(MJ_INIT)		
if(!(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PREVIEW))
{
    //mpDebugPrint("MJ_INIT = %d", MJ_INIT);
    #if 0
    if (0 == MJ_INIT)
    {
	    Mjpeg_Tar = psJpeg->pbTarget;
	    Mjpeg_TarSize = psJpeg->dwTargetSize;
    }	
	#endif
	if (psJpeg->iErrorCode == 0)
	{
	    #if 0
		switch (Mjpeg_toggle)
		{
		    case 0:
			   psJpeg->pbTarget = (BYTE *)((DWORD) Mjpeg_Tar);
			break;
			
		    case 1:
			   psJpeg->pbTarget = (BYTE *)((DWORD) (Mjpeg_Tar + psJpeg->dwTargetSize));
			break;

			case 2:
			   psJpeg->pbTarget = (BYTE *)((DWORD) (Mjpeg_Tar + psJpeg->dwTargetSize * 2));
            break;
		}	
		#endif
//		SetCduWaitRdy(1);
		SemaphoreWait(JPEG_CDU_SEMA_ID);
		Jpg_Decoder_CduDecode(psJpeg);		
		SemaphoreRelease(JPEG_CDU_SEMA_ID);
	}

    #if MJPEG_TOGGLE
	if(!Mjpeg_toggle)
	{	    
		g_sDecodeWin.pdwStart = (DWORD *)((DWORD)(Mjpeg_Tar + psJpeg->dwTargetSize));
        Mjpeg_toggle = 1;
	}
	else
	{
		g_sDecodeWin.pdwStart = (DWORD *)Mjpeg_Tar;		
        Mjpeg_toggle = 0;
	}
	#endif
	    
	
#if 0 // move to vd_mjpeg.c   	
	psJpeg->pbTarget = Mjpeg_Tar;
	psJpeg->dwTargetSize = Mjpeg_TarSize;
#endif	

#if MJPEG_TOGGLE
	g_sDecodeWin.dwOffset = psJpeg->wRealTargetWidth << 1;
	g_sDecodeWin.wWidth = psJpeg->wRealTargetWidth;
	g_sDecodeWin.wHeight = psJpeg->wRealTargetHeight;
#endif	
}	
//#else
else
{	
    MJ_INIT=1;
    if (psJpeg->iErrorCode == 0)
		Jpg_Decoder_CduDecode(psJpeg);	

#if MJPEG_TOGGLE
	g_sDecodeWin.pdwStart = (DWORD *) psJpeg->pbTarget;
	g_sDecodeWin.dwOffset = psJpeg->wRealTargetWidth << 1;//640 << 1;
	g_sDecodeWin.wWidth = psJpeg->wRealTargetWidth;//640;
	g_sDecodeWin.wHeight = psJpeg->wRealTargetHeight;//480;
#endif	
}    
//#endif

#if MJPEG_ENABLE 
#if MJPEG_TOGGLE
    if(!MJ_INIT)
    {
        MJ_INIT=1;
	    mpPaintWin(Idu_GetNextWin(), 0x00008080);
		Idu_ChgWin(Idu_GetNextWin());
    }
    else
    {
       static ST_SCA_PARM sScaParm;
	   static ST_IMAGEINFO src, trg;
	   ST_IMGWIN srcwin, tmpwin;
	   int full_window;
	
	   ST_IMGWIN * trgwin = Idu_GetCurrWin();
		//ImageDraw_FitToFull(&g_sDecodeWin, Idu_GetNextWin());
	   #if (CHIP_VER & 0xffff0000) != CHIP_VER_615

		WORD wScaleFactorWidth;
		WORD wScaleFactorHeight;
		WORD wScaledWidth;
		WORD wScaledHeight;
		WORD wStartX;
		WORD wStartY;

		wScaleFactorWidth = ( (DWORD) trgwin->wWidth << 10 ) / g_psCurJpeg->wImageHeight;
		wScaleFactorHeight = ( (DWORD) trgwin->wHeight << 10 ) / g_psCurJpeg->wImageWidth;

		if (wScaleFactorWidth < wScaleFactorHeight)
		{
			wScaledWidth = trgwin->wWidth;
			wScaledHeight = (g_psCurJpeg->wImageHeight * wScaleFactorWidth) >> 10;
			wStartX = 0;
			wStartY = (trgwin->wHeight - wScaledHeight) >> 1;
		}
		else
		{
			wScaledWidth = (g_psCurJpeg->wImageWidth * wScaleFactorHeight) >> 10;
			wScaledHeight = trgwin->wHeight;
			wStartX = (trgwin->wWidth - wScaledWidth) >> 1;
			wStartY = 0;
		}

		//mpDebugPrint("ScaleFactor: width = %d, height = %d", wScaleFactorWidth, wScaleFactorHeight);
		//mpDebugPrint("video_mpi->display_width = %d, video_mpi->display_height = %d", g_psCurJpeg->wImageWidth, g_psCurJpeg->wImageHeight);

		srcwin.pdwStart = g_psCurJpeg->pbTarget;
		srcwin.dwOffset = g_psCurJpeg->wImageWidth << 1;
		srcwin.wWidth = g_psCurJpeg->wImageWidth;
		srcwin.wHeight = g_psCurJpeg->wImageHeight;
		srcwin.wType = 411;

//		Ipu_ImageScaling(&srcwin, trgwin, wScaledWidth, wScaledHeight, wStartX, wStartY);

        
	    //Ipu_ImageScaling(&srcwin, trgwin, 800, 600, 0, 0);
		Ipu_ImageScaling(&srcwin, trgwin, 0, 0, &srcwin.wWidth, &srcwin.wHeight, 0, 0, trgwin->wWidth, trgwin->wHeight, 1);
#else	// (CHIP_VER & 0xffff0000) != CHIP_VER_650

		src.dwPointer = (DWORD *) g_psCurJpeg->pbTarget;

		if ((src.wWidth != g_psCurJpeg->wImageWidth) || (src.wHeight != g_psCurJpeg->wImageHeight) || (trg.dwPointer != trgwin->pdwStart)
		 || (trg.wWidth != trgwin->wWidth) || (trg.wHeight != trgwin->wHeight))
		{
			if (g_psCurJpeg->wImageWidth < g_psCurJpeg->wImageHeight)
				full_window = 0;
			else if ((g_psCurJpeg->wImageWidth < (trgwin->wWidth >> 2))) // || (video_mpi->display_height < (trgwin->wHeight >> 2)))
				full_window = 0;
			else
				full_window = 1;
			if (full_window)
			{
				trg.dwPointer = trgwin->pdwStart;
				trg.wWidth = trgwin->wWidth;
				trg.wHeight = trgwin->wHeight;
				trg.dwOffset = 0;
			}
			else
			{
				tmpwin.wWidth = trgwin->wWidth;
				tmpwin.wHeight = trgwin->wHeight;
				srcwin.wWidth = g_psCurJpeg->wImageWidth;
				srcwin.wHeight = g_psCurJpeg->wImageHeight;
				VideoAdjustDisplaySize(&tmpwin, &srcwin);

				trg.dwPointer = trgwin->pdwStart + ((tmpwin.wY * (trgwin->dwOffset>>1) + tmpwin.wX)>>1);
				trg.wWidth = tmpwin.wWidth;
				trg.wHeight = tmpwin.wHeight;
				trg.dwOffset = tmpwin.dwOffset;
			}

			src.wWidth = g_psCurJpeg->wImageWidth;
			src.wHeight = g_psCurJpeg->wImageHeight;

			src.dwOffset = 0;
			
			ImgGetResizeParam(&src, &trg, &sScaParm);
		}
MP_TRACE_TIME_START();
		ImgIpuScaling(&sScaParm, &src, &trg);
MP_TRACE_TIME_PRINT("IPU", 0);

#endif	// (CHIP_VER & 0xffff0000) == CHIP_VER_650
	} 
//	SetCduWaitRdy(1);		
	//Idu_ChgWin(Idu_GetNextWin());
#endif 
#endif
    //__asm("break 100");
	return psJpeg->iErrorCode;
}


#if MJPEG_TOGGLE
int Mjpeg_Decode_First_Frame_Only(ST_JPEG *psJpeg, DWORD dwX, DWORD dwY, DWORD dwWidth, DWORD dwHeight)
{
	ST_MPX_STREAM *psStream;
	g_psCurJpeg = psJpeg;
	MP_DEBUG(" === Mjpeg_Decode_First_Frame_Only");
    psJpeg->bDqtHit = 0; //temp
    psJpeg->bDhtHit = 0; //temp
	psStream = psJpeg->psStream;
	BYTE bMarker = 0;
	do {
		if (mpxStreamSearchMarker(psStream, MARKER_FIRST) == FAIL) 
			break;
	
		bMarker = mpxStreamGetc(psStream);	// get the second byte of bMarker			
	} while (bMarker != MARKER_SOI && !psStream->eof);
	
	if (psStream->eof) return FAIL;
	psJpeg->iErrorCode = Jpg_Parse_Marker(psJpeg->psStream, psJpeg);
	
    if (psJpeg->iErrorCode == 0)
    {
		SemaphoreWait(JPEG_CDU_SEMA_ID);
		Jpg_Decoder_CduDecode(psJpeg);	
		SemaphoreRelease(JPEG_CDU_SEMA_ID);
    }

	g_sDecodeWin.pdwStart = (DWORD *) psJpeg->pbTarget;
	g_sDecodeWin.dwOffset = psJpeg->wRealTargetWidth << 1;//640 << 1;
	g_sDecodeWin.wWidth = psJpeg->wRealTargetWidth;//640;
	g_sDecodeWin.wHeight = psJpeg->wRealTargetHeight;//480;

  //ImageDraw_FitToPreview(&g_sDecodeWin, Idu_GetCurrWin()); // Jonny 20090401 for VideoPreview
  //ImageDraw_FitToPreview(&g_sDecodeWin, Idu_GetCurrWin(), dwX, dwY, dwWidth, dwHeight); 
  Ipu_ImageScaling(&g_sDecodeWin, Idu_GetCurrWin(), 0, 0, g_sDecodeWin.wWidth, g_sDecodeWin.wHeight, dwX, dwY, dwX+dwWidth, dwY+dwHeight, 0);
  //ImageDraw_FitToFull(&g_sDecodeWin, Idu_GetCurrWin());
    
	    
//	SetCduWaitRdy(1);		
	
	return psJpeg->iErrorCode;
}
#endif

void Mjpeg_Decode_Stop(void)
{
    CduDecodeEnable();
}

int Mjpeg_Decode_Close(ST_JPEG *psJpeg)
{
     ImageReleaseAllBuffer();
    if (g_pbChaseBuffer) ext_mem_free(g_pbChaseBuffer);
    g_pbChaseBuffer = NULL;
    return 0;
    
}
#endif

//--------------------------------------------------------------------------------------------

int Img_Jpeg2ImgBuf(BYTE * pbSource, BYTE * pbTarget, BYTE bMode, DWORD dwSourceSize, DWORD dwTargetSize)
{
	ST_JPEG *psJpeg;
		
    MP_DEBUG("%s:%d", __func__, __LINE__);
	g_psCurJpeg = psJpeg = Jpg_OpenJpegBuffer(pbSource, pbTarget, dwSourceSize, dwTargetSize, bMode);

	if (psJpeg == NULL)
	{
        MP_DEBUG("%s:%d: psJpeg->iErrorCode %d", __func__, __LINE__, psJpeg->iErrorCode);
		return FAIL;
	}
	
	if (psJpeg->iErrorCode == 0)
	{
		SemaphoreWait(JPEG_CDU_SEMA_ID);
		Jpg_Decoder_CduDecode(psJpeg);
		SemaphoreRelease(JPEG_CDU_SEMA_ID);
	}
	
    MP_DEBUG("%s:%d: psJpeg->iErrorCode %d", __func__, __LINE__, psJpeg->iErrorCode);
	Jpg_CloseJpegFile(psJpeg);	
	
    MP_DEBUG("%s:%d: psJpeg->iErrorCode %d", __func__, __LINE__, psJpeg->iErrorCode);
	return psJpeg->iErrorCode;
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
int Jpg_Cdu_Decode_DChase(ST_MPX_STREAM *psStream, ST_JPEG *psJpeg, BYTE *pbTargetBuffer)
{
	DWORD i, n, h, dwFileSize;
	BYTE *pBuffer = NULL;	
	DC_CONTROLLER * pDCC = ImageGetDC_Controller();
	CDU *cdu = (CDU *) (CDU_BASE);
	BYTE bJResizeRatio = psJpeg->bJResizeRatio;
	WORD wResizeWidth,wResizeHeight;
	
	dwFileSize=mpxStreamGetSize(psStream);
	MP_DEBUG("Jpg_Cdu_DChase file size %08x, eoipoint %08x", dwFileSize, psJpeg->dwEoiPoint);	

	pDCC->wTargetWidth= (psJpeg->wCduWidth >> bJResizeRatio);
	pDCC->wTargetHeight= psJpeg->wCduHeight >> bJResizeRatio;
	pDCC->dwTargetLineOffset = ((ALIGN_16(pDCC->wTargetWidth ) << 1 ));	
	MP_DEBUG("pDCC->wTargetWidth %d, pDCC->wTargetHeight %d \n\r pDCC->dwTargetLineOffset %d, psJpeg->wCduWidth  %d", pDCC->wTargetWidth, pDCC->wTargetHeight,pDCC->dwTargetLineOffset,psJpeg->wCduWidth);	
	MP_DEBUG("psJpeg->wOriginalWidth %d, psJpeg->wOriginalHeight %d \n\r pDCC->dwTargetLineOffset %d, psJpeg->wCduWidth  %d", psJpeg->wOriginalWidth, psJpeg->wOriginalHeight,pDCC->dwTargetLineOffset,psJpeg->wCduWidth);	
	
	if (bJResizeRatio>2) 
	{
		pDCC->bDCCoEfficient=bJResizeRatio-2;
		pDCC->bDChasingStep=1<<(pDCC->bDCCoEfficient);
		pDCC->dwCurrentChasingStep=0;
		bJResizeRatio=2;
	}
	else
	{
		return FAIL;
	}
	
	wResizeWidth =( psJpeg->wCduWidth >> bJResizeRatio);
	wResizeHeight = psJpeg->wCduHeight >> bJResizeRatio;
	psJpeg->dwOffset=0;
	MP_DEBUG("psJpeg->dwOffset %d, wResizeWidth %d", psJpeg->dwOffset, wResizeWidth);	
	MP_DEBUG("psJpeg->wCduWidth %d, wResizeWidth << bJResizeRatio %d", psJpeg->wCduWidth, wResizeWidth << bJResizeRatio);	
	
	psJpeg->dwOffset=(DWORD)((DWORD)(psJpeg->wCduWidth) - (DWORD)(wResizeWidth << bJResizeRatio));
	MP_DEBUG("psJpeg->dwOffset %d, wResizeHeight %d", psJpeg->dwOffset, wResizeHeight);	

	
	cdu->CduIc = 0;
	cdu->CduSize = ((psJpeg->wCduHeight - 1) << 16) | (psJpeg->wCduWidth - 1);
	cdu->CduC = psJpeg->dwCduControl | ((bJResizeRatio & 0x03) << 2);	
	
	DWORD dwLineOffset = ALIGN_16(((DWORD)wResizeWidth + psJpeg->dwOffset)) << 1;	
	DWORD downSizeOffset =  dwLineOffset - ((DWORD)wResizeWidth << 1);	
	MP_DEBUG("dwLineOffset %d, downSizeOffset %d", dwLineOffset, downSizeOffset);	
	CHANNEL *Ch_JMCU = (CHANNEL *) (DMA_JMCU_BASE);
	CHANNEL *Ch_Jvlc = (CHANNEL *) (DMA_JVLC_BASE);


	DWORD dwStartPoint = psJpeg->dwEcsStart;
	DWORD dwTotalSize = psJpeg->dwEoiPoint - dwStartPoint;

	DWORD dwSize_1 = JPEG_CHASING_BUFFER_SIZE; // the buffer size could be different for photo size
	DWORD dwSrcBufferSize = dwSize_1 * JPEG_CHASING_BUFFER_COUNT; //byAlexWang 27jun2007 m2project

		
	switch((int) psJpeg->wCduWidth / 512)
	{
		case 6:

		case 5:
			i=3;
			break;
		case 4:

		case 3:
		case 2:	
			i=3;
			break;
		case 1:
		case 0:
			i=4;
			break;
		default:
			i=2;
	}	
	
	if(psJpeg->wCellSize == 16)	
		i = 3;
	if ((dwFileSize > 40*1024*1024)&&(psJpeg->wCellSize == 8))
		i=1;
	h = 1 << i;
	n = wResizeHeight >> i;		
	n++;
	pDCC->bDCIndex=0;
	pDCC->wDCHeight = (h>(pDCC->bDChasingStep))?h:pDCC->bDChasingStep;
	pDCC->wDCWidth = dwLineOffset >>1;

	DWORD t;	
	DWORD dwPos = 0;
	DWORD dwDecodeSize = 0;

	pDCC->dwDChasingBufferSize = (pDCC->wDCWidth) *  (pDCC->wDCHeight) * 2;
	pDCC->pbDChasingBuffer=(BYTE *)ext_mem_malloc(pDCC->dwDChasingBufferSize + 256);

	if (dwSrcBufferSize >= dwTotalSize) dwSrcBufferSize = dwTotalSize;
	
	   pBuffer = (BYTE *)ext_mem_malloc(dwSrcBufferSize + 4096);
	
	if ((pBuffer == NULL)||(pDCC->pbDChasingBuffer == NULL))
	{
		ext_mem_free((BYTE *)((DWORD)pBuffer & ~0x20000000));
		ext_mem_free((BYTE *)((DWORD)(pDCC->pbDChasingBuffer)& ~0x20000000));		
		MP_ALERT("alloc pBuffer fail");
		return FAIL;
	}

	pDCC->pbChasingBuffer = pBuffer;
	pDCC->dwChasingBufferSize = dwSrcBufferSize;
	
	psJpeg->dwCduControl |= ((DWORD)pBuffer & 0x00000003) << 8;	// get CDU_DBS
	pBuffer = (BYTE *)((DWORD)pBuffer | 0xa0000000);
	pDCC->pbDChasingBuffer = (BYTE *)((DWORD)pDCC->pbDChasingBuffer | 0xa0000000);
	//memset(pBuffer, 0, dwSrcBufferSize);
	
	DWORD dwReadSize;

	if (mpxStreamSeek(psStream, dwStartPoint, SEEK_SET) == FAIL) {
		MP_DEBUG("-E- mpxStreamSeek(psStream, dwStartPoint, SEEK_SET) == FAIL");
		ext_mem_free((BYTE *)((DWORD)pBuffer & ~0x20000000));
		ext_mem_free((BYTE *)((DWORD)(pDCC->pbDChasingBuffer)& ~0x20000000));		
		return FAIL;
	}
	dwReadSize = mpxStreamRead(pBuffer, 1, dwSrcBufferSize, psStream);
	
	Ch_Jvlc->StartA = (DWORD)pBuffer;
	Ch_Jvlc->EndA = (DWORD)pBuffer + dwSrcBufferSize - 1;
	Ch_Jvlc->Control = 3;
	
	dwStartPoint = 0;
	DWORD dwPrevCurrent = Ch_Jvlc->StartA & 0xfffffff;

#if 0   
	WORD bar_w = Idu_GetCurrWin()->wWidth  >> 2;
	bar_w /= n;
	bar_w++;
#endif	
	DWORD dwTargetStart = (DWORD) pDCC->pbDChasingBuffer; //psJpeg->pbTarget;
//	DWORD dwTargetEnd = (DWORD) psJpeg->pbTarget + dwLineOffset * wResizeHeight - downSizeOffset;//+ psJpeg->dwTargetSize;
	DWORD dwTargetEnd = (DWORD) pDCC->pbDChasingBuffer + dwLineOffset * (pDCC->wDCHeight) - downSizeOffset;
	int iErrorCount = 1;  //byAlexWang 12jun2007 m2project
	BYTE *pbTarget= pDCC->pbDChasingBuffer;
	
	MP_DEBUG("pBuffer %08x, dwSrcBufferSize %d", pBuffer, dwSrcBufferSize);	
	MP_DEBUG("dwTargetStart %08x, dwTargetEnd %08x", dwTargetStart, dwTargetEnd);	
	MP_DEBUG("pbTarget %08x, downSizeOffset %d", pbTarget, downSizeOffset);	
	MP_DEBUG("dwTargetStart is 0x%08x, dwTargetEnd is 0x%08x, Target size used %d,\n\r wResizeWidth is %d, pBuffer is 0x%08x", dwTargetStart, dwTargetEnd,dwTargetEnd- dwTargetStart ,wResizeWidth, pBuffer);		
	MP_DEBUG("dwLineOffset is %d, h is %d, n is %d", dwLineOffset, h, n);		
	MP_DEBUG("downSizeOffset 0x%08x", downSizeOffset);		
	
	for (i = 0; i < n; i++) 
	{		
		if (wResizeHeight < h) h = wResizeHeight;	
		//MP_DEBUG1("decode pos %08x", dwStartPoint);		

//		MP_DEBUG1("decode pos dwStartPoint 0x%08x", dwStartPoint);		
//		MP_DEBUG("wResizeHeight is %d, \n\r i is %d, n is 0x%08x", wResizeHeight, i, n);		
//		MP_DEBUG("dwReadSize is 0x%08x, \n\r pbTarget is 0x%08x, dwPrevCurrent is 0x%08x", dwReadSize,  pbTarget, dwPrevCurrent);		
	
		Ch_JMCU->StartA = (DWORD) pbTarget;
		Ch_JMCU->EndA = (DWORD) pbTarget + (h * dwLineOffset) - downSizeOffset - 1;	
		Ch_JMCU->LineCount = ((downSizeOffset) << 16) + ((wResizeWidth << 1) - 1);
//	MP_DEBUG("Ch_JMCU->StartA %08x, dwTargetEnd %08x", Ch_JMCU->StartA, dwTargetEnd);	

		if (Ch_JMCU->StartA >= dwTargetEnd) break;
		if (Ch_JMCU->EndA >= dwTargetEnd) Ch_JMCU->EndA = dwTargetEnd - 1;
		
		Ch_JMCU->Control = 5;
		
		// enable CDU operation
		cdu->CduOp = 1;		

		while (!psStream->eof && dwReadSize + dwSize_1 < dwStartPoint + dwSrcBufferSize)
		{
			//MP_DEBUG2("read %d, decode point %d", (dwReadSize + dwSize_1) % dwSrcBufferSize, dwStartPoint % dwSrcBufferSize);

			TaskYield();
			if(Jpg_PollEvent(psJpeg))//byAlexWang 12jun2007 m2project
			{
				i=n+30;
				iErrorCount=30;
				break;
			}
			TaskYield();

			dwPos = dwReadSize % dwSrcBufferSize;
			DWORD dwSize = mpxStreamRead(pBuffer + dwPos, 1, dwSize_1, psStream);	
			TaskYield();

			dwReadSize += dwSize;
			if (dwSize < dwSize_1) break; //psStream->eof = 1;
			if (dwSize == 0) psStream->eof = 1;
			//MP_DEBUG1("read size %8x",  dwSize);
			TaskYield();
		}

		t = JPEG_DECODE_TIMEOUT_CNT << 1;	//byAlexWang 12jun2007 m2project
		DWORD new_pos = dwPrevCurrent;
		DWORD t1 = 0;
		while (!(Ch_JMCU->Control & BIT16) && t--)
		{				
			if (g_bAniFlag & ANI_AUDIO) TaskYield();

			if(Jpg_PollEvent(psJpeg))
			{
				i=n+40;
				iErrorCount=40;
				break;
			}

			if (t1 >= JPEG_DECODE_TIMEOUT_CNT>>3/*0x7fff*/) {  // avoid decode error and always redecode same position
				if (Ch_Jvlc->Current == new_pos)
				{  
					Ch_JMCU->Control = 0;
					iErrorCount=50;
					i = n+50; 
					break; 
				}
				else 
				{
					new_pos = Ch_Jvlc->Current;
					t1 = 0;
				}
			}
			t1++;
		}		
		
		
		Ch_JMCU->Control = 0;

		if (t == 0) break;		

		if (Ch_Jvlc->Current >= dwPrevCurrent)
			dwDecodeSize = Ch_Jvlc->Current - dwPrevCurrent;
		else
			dwDecodeSize = Ch_Jvlc->Current + dwSrcBufferSize - dwPrevCurrent;

		if ((dwDecodeSize == 0)&&(i<(n-3))) {
			i--;
			iErrorCount++;
			if (iErrorCount > 20) break;
			MP_DEBUG("dwDecodeSize %d, current %08x, dwReadSize %08x, dwStartPoint %08x", dwDecodeSize, Ch_Jvlc->Current, dwReadSize, dwStartPoint);					
		} 
		else
		{
			pbTarget += (h * dwLineOffset);
			iErrorCount = 1;
			dwStartPoint += dwDecodeSize;		
			dwPrevCurrent = Ch_Jvlc->Current;		
			wResizeHeight -= h;
			pDCC->bDCIndex+=h;
			
			BYTE * pbTargetStart, * pbDCStart,j=0;
			while (pDCC->bDCIndex >= pDCC->bDChasingStep)
			{
				pbTargetStart=pbTargetBuffer+(pDCC->dwCurrentChasingStep)*(pDCC->dwTargetLineOffset);
				ImgWinInit(&(pDCC->sPTWin), pbTargetStart, 1, (pDCC->dwTargetLineOffset)>>1);
//				pDCC->sPTWin.dwOffset=pDCC->dwTargetLineOffset;
				
				pbDCStart = pDCC->pbDChasingBuffer + j * ((DWORD)pDCC->bDChasingStep) * dwLineOffset;
				ImgWinInit(&(pDCC->sDCWin), pbDCStart, 1, (pDCC->wDCWidth));
				//image_scale(&(pDCC->sDCWin), &(pDCC->sPTWin),0,0,pDCC->wDCWidth,1,0,0,(pDCC->dwTargetLineOffset)>>1,1);
				Ipu_ImageScaling(&(pDCC->sDCWin), &(pDCC->sPTWin),0,0,pDCC->wDCWidth,1,0,0,(pDCC->dwTargetLineOffset)>>1,1,0);
				
				pDCC->bDCIndex -= pDCC->bDChasingStep;
				j++;
				pDCC->dwCurrentChasingStep++;
				if(pDCC->bDCIndex ==0)
				{
					pbTarget= pDCC->pbDChasingBuffer;
//					MP_DEBUG("pbTargetStart %08x, pDCC->dwCurrentChasingStep %08x\n\r pbDCStart %08x, (pDCC->dwTargetLineOffset)>>1 %08x", pbTargetStart, pDCC->dwCurrentChasingStep, pbDCStart,(pDCC->dwTargetLineOffset)>>1);					
					
				}
			}

			if (wResizeHeight <= 0) 
			{
				iErrorCount=0;//byAlexWang 12jun2007 m2project
				break;
			}		
		}

		

		if(Jpg_PollEvent(psJpeg)) //byAlexWang 12jun2007 m2project
		{
			iErrorCount=60;
			break;
		}
		if (((i & 0xf) == 0) && ((psJpeg->bDecodeMode & 0x3f) == IMG_DECODE_SLIDE))
			TaskYield();
	}
		MP_DEBUG("at the end  wResizeHeight is %d, i is %d, n is %d", wResizeHeight, i, n);		
	CduDisable();
	if ((psJpeg->bDecodeMode & 0x3f) != IMG_DECODE_MJPEG)
	ext_mem_free((BYTE *)((DWORD)pBuffer & ~0x20000000));
	ext_mem_free((BYTE *)((DWORD)(pDCC->pbDChasingBuffer)& ~0x20000000));
#if OUTPUT_JPEG	
	DUMPMEMORY = TRUE;
#endif
#if OUTPUT_JPEG
	if ((psJpeg->bDecodeMode & 0x3f) == IMG_DECODE_PHOTO)

	if(DUMPMEMORY){
		static DRIVE *sDrv;
		static STREAM *shandle;
		static int ret;
		int len = 0;
		WORD width, height;
		width = psJpeg->wTargetWidth >> psJpeg->bJResizeRatio;
		if(width & 0xfff0)
			width = (width + 15) & 0xfff0;
		height = psJpeg->wTargetHeight >> psJpeg->bJResizeRatio;
		len = width * height *2;

		UartOutText("creat jpg.bin");
		UartOutText("width");
		UartOutValue(width, 4);
		UartOutText("height");
		UartOutValue(height, 4);
	    sDrv=DriveGet(SD_MMC);
        ret=CreateFile(sDrv, "jpg", "bin");
        if (ret) UartOutText("create file fail\r\n");
        shandle=FileOpen(sDrv);
        if(!shandle) UartOutText("open file fail\r\n");

		ret=FileWrite(shandle, psJpeg->pbTarget, len);
        if(!ret) UartOutText("write file fail\r\n");

        FileClose(shandle);
        UartOutText("\n\rfile close\n\r");
	}	
#endif

	return iErrorCount;
}

//--------------------------------------------------------------------------------------------
static void SpecialSizeRecover(ST_JPEG *psJpeg, DWORD * dwpTarget, DWORD * dwpSource)
{
	WORD height, width;
	WORD i, j, k, l, m, chroma_index;
	BYTE *sptr, *dptr, *tmpsrc, *tmptrg;
	BYTE *Ydptr, *Cbdptr, *Crdptr;
	WORD offset; 
	WORD block_no_of_height, block_no_of_width, line, chroma, ratio;
	MP_DEBUG("SpecialSizeRecover");
	sptr = (BYTE *)dwpSource;
	tmpsrc = (BYTE *)((DWORD)dwpSource - 0x10000);
	
	dptr = tmptrg = (BYTE *)dwpTarget;

	width = ALIGN_16(psJpeg->wCduWidth) /2;
	height = psJpeg->wRealTargetHeight;
	MP_DEBUG2("width = %d, height = %d", width, height);
	
	block_no_of_height = 16;
	block_no_of_width = 8;
	line = 8;
	chroma = 4;

	block_no_of_height >>= psJpeg->bJResizeRatio;
	block_no_of_width >>= psJpeg->bJResizeRatio;
	line >>= psJpeg->bJResizeRatio;
	chroma >>= psJpeg->bJResizeRatio;

	Ydptr = (BYTE *)ext_mem_malloc((width/2) * block_no_of_height *2);
	if(Ydptr == NULL)
		MP_DEBUG("Ydptr is NULL");
	
	Cbdptr = (BYTE *)ext_mem_malloc((width/2) * block_no_of_height);
	if(Cbdptr == NULL)
		MP_DEBUG("Cbdptr is NULL");
	
	Crdptr = (BYTE *)ext_mem_malloc((width/2) * block_no_of_height);
	if(Crdptr == NULL)
		MP_DEBUG("Crdptr is NULL");

	MP_DEBUG1("ratio = %d", psJpeg->bJResizeRatio);

	for(i=0; i<height/block_no_of_height; i++)  //i = 0, 1, 2, ...  //16, 8, 4...
	{
		for(j=0; j<line; j++)
		{
			for(k=0; k<width/block_no_of_width; k++)
			{
				chroma_index=0 ;
				for(l=0; l<chroma; l++)
				{
					*(Ydptr + ((j)*width+k*block_no_of_width+l*2)) = *sptr++;
					*(Ydptr + ((j)*width+k*block_no_of_width+l*2+1)) = *sptr++;
					if(chroma_index%2==0)
					{
						*(Cbdptr + ((j*2)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = *sptr++;
						*(Crdptr + ((j*2)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = *sptr++;
					}
					else
					{
						*(Cbdptr + ((j*2)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = *sptr++;
						*(Crdptr + ((j*2)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = *sptr++;
	
						*(Cbdptr + ((j*2+1)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = 
							*(Cbdptr + ((j*2)*width/2+k*(block_no_of_width/2)+ chroma_index/2));
						*(Crdptr + ((j*2+1)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = 
							*(Crdptr + ((j*2)*width/2+k*(block_no_of_width/2)+ chroma_index/2));
					}
					chroma_index++ ;
				}
				for(l=0; l<chroma; l++)
				{
					*(Ydptr + ((j+line)*width+k*block_no_of_width+l*2)) = *sptr++;
					*(Ydptr + ((j+line)*width+k*block_no_of_width+l*2+1)) = *sptr++;
					if(chroma_index%2==0)
					{
						*(Cbdptr + ((j*2)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = *sptr++;
						*(Crdptr + ((j*2)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = *sptr++;
					}
					else
					{
						*(Cbdptr + ((j*2)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = *sptr++;
						*(Crdptr + ((j*2)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = *sptr++;
						
						*(Cbdptr + ((j*2+1)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = 
							*(Cbdptr + ((j*2)*width/2+k*(block_no_of_width/2)+ chroma_index/2));
						*(Crdptr + ((j*2+1)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = 
							*(Crdptr + ((j*2)*width/2+k*(block_no_of_width/2)+ chroma_index/2));
					}
					chroma_index++ ;
				}
						TaskYield();

			}
					TaskYield();

		}
		TaskYield();

		//arrange 8 line, move data to target	
		//## write file
		for(m=0; m<block_no_of_height; m++) //16, 8, 4...
		{
			for(j=0; j<width/2; j++)
			{
				*dptr++ = *(Ydptr +(m*width+j*2));
				*dptr++ = *(Ydptr + (m*width+j*2+1));
				*dptr++ = *(Cbdptr + (m*width/2+j));
				*dptr++ = *(Crdptr + (m*width/2+j));
			}
			TaskYield();
		}		
		TaskYield();
		WORD offset = ALIGN_16(width) - width;

		for(j =0; j < block_no_of_height; j++){
			for(k = 0; k < width * 2; k++)
			{
				*tmpsrc++ = *tmptrg++;
			}
			tmpsrc += offset * 2;
			TaskYield();
		}
		dptr = tmptrg = (BYTE *)dwpTarget; //move to org trg point
		TaskYield();

	}
	
	ext_mem_free(Ydptr);
	ext_mem_free(Cbdptr);
	ext_mem_free(Crdptr);
}
//--------------------------------------------------------------------------------------------
static void FixSpecialImage(ST_JPEG *psJpeg, BYTE *pbTarget)
{
	BYTE *pbAllocBuffer, *pbTempBuffer;
	WORD wd, ht;
	DWORD dwSize;
	MP_DEBUG("FixSpecialImage");
	wd = ALIGN_16(psJpeg->wRealTargetWidth);
	ht = 16 >> psJpeg->bJResizeRatio;//psJpeg->wRealTargetHeight;
	dwSize = wd * ht * 2;
	
	ImageReleaseSourceBuffer();
	MP_DEBUG("Jpg_FixSpecialImage %d, %d", dwSize, psJpeg->dwTargetSize);

	pbAllocBuffer = (BYTE *)ext_mem_malloc(dwSize + 4096);

	if (pbAllocBuffer == NULL){
	MP_ALERT("pbAllocBuffer alloc fail");
		return;	
	}
	pbTempBuffer = (BYTE *)((DWORD)pbAllocBuffer | 0xa0000000);
	memset(pbTempBuffer, 0x80, dwSize);
	pbTarget = (BYTE *)((DWORD)pbTarget | 0xa0000000);
	TaskYield();
	MP_DEBUG("pbTarget %x, pbTempBuffer %x", pbTarget, pbTempBuffer);

	SpecialSizeRecover(psJpeg, (DWORD *) pbTempBuffer, (DWORD *) pbTarget);

#if OUTPUT_JPEG
		if(DUMPMEMORY){
		static DRIVE *sDrv;
		static STREAM *shandle;
		static int ret;
		int len = 0;
		WORD width, height;
		width = ALIGN_16(psJpeg->wCduWidth) / 2;
		height = psJpeg->wTargetHeight;
		len = width * height *2;

		UartOutText("creat jpg_422v.bin");
		UartOutText("\r\nwidth");
		UartOutValue(width, 4);
		UartOutText("\r\nheight");
		UartOutValue(height, 4);
	    sDrv=DriveGet(SD_MMC);
        ret=CreateFile(sDrv, "jpg_422v", "bin");
        if (ret) UartOutText("create file fail\r\n");
        shandle=FileOpen(sDrv);
        if(!shandle) UartOutText("open file fail\r\n");

	psJpeg->pbTarget = (BYTE *)((DWORD)psJpeg->pbTarget - 0x10000);
		
		ret=FileWrite(shandle, psJpeg->pbTarget, len);
        if(!ret) UartOutText("write file fail\r\n");

        FileClose(shandle);
        UartOutText("\n\rfile close\n\r");
	}
#endif

	ext_mem_free(pbAllocBuffer);
}

//--------------------------------------------------------------------------------------------

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
void Reset_CDU_ISR_Count()
{
	CDU_Isr_Cnt = 0 ;
	EventClear(JPEG_LOAD_DATA_EVENT_ID1, BIT0);
	EventClear(JPEG_LOAD_DATA_EVENT_ID1, BIT1);
	EventClear(JPEG_LOAD_DATA_EVENT_ID1, BIT2);
	EventClear(JPEG_LOAD_DATA_EVENT_ID1, BIT3);
}

void InsertDummyByte(BYTE *source, DWORD buff_size)
{
		BYTE *dummy ;
    dummy = source + buff_size -2;
		if(*dummy == 0xff && *(dummy+1) == 0xd9)
		{
			MP_DEBUG("Dummy");
			*dummy = 0x80 ;
			dummy++ ;
			*dummy = 0xff ;
			dummy++ ;
			*dummy = 0xd9 ;
		}
}

int WaitBuffEmpty(BYTE BuffNum)
{
		DWORD t=0, TimeOut;
		int ret = 1 ;
		CDU *cdu = (CDU *) (CDU_BASE);
		TimeOut = (JPEG_DECODE_TIMEOUT_CNT>>1);
		while(t<TimeOut)
		{
			if(BuffNum==0 && !(cdu->JVLC_BUFSZ & 0x10))
			{
				mpDebugPrint("Buffer 0 decode finish");
				ret = 0 ;
				break ;
			}
			else if(BuffNum==1 && !(cdu->JVLC_BUFSZ & 0x20))
			{
				mpDebugPrint("Buffer 1 decode finish");
				ret = 0 ;
				break ;
			}
			else if(BuffNum==2 && !(cdu->JVLC_BUFSZ & 0x40))
			{
				mpDebugPrint("Buffer 2 decode finish");
				ret = 0 ;
				break ;
			}
			else if(BuffNum==3 && !(cdu->JVLC_BUFSZ & 0x80))
			{
				mpDebugPrint("Buffer 3 decode finish");
				ret = 0 ;
				break ;
			}
			t++ ;
		}
		if(t>=TimeOut)	mpDebugPrint("Wait buffer empty time out");
		return ret ;
}

BYTE Choose_Buff_Size(DWORD size)
{
		if(size == (1<<15))	//32KB
			return 0 ;
		else if(size == (1<<16))	//64KB
			return 1 ;
		else if(size == (1<<17))	//128KB
			return 2 ;
		else if(size == (1<<18))	//256KB
			return 3 ;
		else if(size == (1<<19))	//512KB
			return 4 ;
		else if(size == (1<<20))	//1MB
			return 5 ;
		else if(size == (1<<21))	//2MB
			return 6 ;
		else if(size == (1<<22))	//4MB
			return 7 ;
		else if(size == (1<<23))	//8MB
			return 8 ;
}

WORD Adjust_RotateMCU(BYTE rotate, WORD type, BYTE resize, BYTE flag_422V)
{
		WORD MCU = 0 ;
		if(type == 444)
		{
#if (YUV444_ENABLE)
           if(JpegDecoder444 == 1)
           {
				MCU = 24 ;
           }
		   else
		   {
		   	    MCU = 16 ;
		   }
#else
				MCU = 16 ;
#endif
		}
		else if(type == 422 && flag_422V == 0)
		{
			MP_DEBUG("422H");
			if(rotate == 1)			//Clockwise
				MCU = 16 ;
			else if(rotate == 2)	//Upsidedown
				MCU = 32 ;
		}
		else if(type == 422 && flag_422V == 1)
		{
			MP_DEBUG("422V");
			if(rotate == 1)			//Clockwise
				MCU = 32 ;
			else if(rotate == 2)	//Upsidedown
				MCU = 16 ;
		}
		else if(type == 420)
		{
			MP_DEBUG("420");
			MCU = 32 ;
		}
		return (MCU >> resize) ;
}

void SetJpegDecoder444(void)
{
	JpegDecoder444=1;
	SetFixedRatio444_422Flag = 1;
}
void SetJpegDecoder422(void)
{
	JpegDecoder444=0;
	SetFixedRatio444_422Flag = 0;
}
	
#if CHASE_IN_DECODE||SET_DECODE_OFFSET_ENALBE
static WORD st_wDecodeWinOffset=0;
void SetDecodeWinOffset(WORD wDecodeWinOffset)
{
	st_wDecodeWinOffset=ALIGN_4(wDecodeWinOffset);
}
#endif
#if CUT_IMAGE_HEIGHT_ENALBE
void SetDecodeWinHeight(WORD wDecodeWinHeight)
{
	st_wDecodeWinHeight=wDecodeWinHeight;
}
#endif
int Jpg_Cdu_Decode_One(ST_MPX_STREAM *psStream, ST_JPEG *psJpeg, BYTE *pbTarget)
{
		DWORD dwStartPoint, dwEndPoint ;
		DWORD Jmcu_Fbwidth = 0;
		BYTE bJResizeRatio ;
		WORD wResizeWidth;
		WORD wResizeHeight;	
		CDU *cdu = (CDU *) (CDU_BASE);
		CHANNEL *Ch_JMCU = (CHANNEL *) (DMA_JMCU_BASE);
		CHANNEL *Ch_Jvlc = (CHANNEL *) (DMA_JVLC_BASE);
		
		MP_DEBUG("CDU Decode_One_650\r\n");
		dwStartPoint = psJpeg->dwEcsStart + (DWORD)psStream->buffer;
	  dwEndPoint = psJpeg->dwEoiPoint + (DWORD)psStream->buffer;
	  psJpeg->dwCduControl &= ~(0x00000300);
		psJpeg->dwCduControl |= (psJpeg->dwEcsStart & 3) << 8;	// get CDU_DBS
		bJResizeRatio = psJpeg->bJResizeRatio;
		if ((psJpeg->bDecodeMode & 0x3f) == IMG_DECODE_MJPEG)
		{
			wResizeWidth = psJpeg->wTargetWidth >> bJResizeRatio;
			wResizeHeight = psJpeg->wTargetHeight>> bJResizeRatio;		
		}
		else
		{
			wResizeWidth = psJpeg->wCduWidth >> bJResizeRatio;
			wResizeHeight = psJpeg->wCduHeight >> bJResizeRatio;		
		}


#if YUV444_ENABLE
		if((psJpeg->wImageType==444)&&(JpegDecoder444))
		{
				Jmcu_Fbwidth = wResizeWidth * 3 ;
				if(Jmcu_Fbwidth & 0x3)
					Jmcu_Fbwidth = ALIGN_4(Jmcu_Fbwidth) ;
				g_wDecodedImageType = 444;

		}else{		
				Jmcu_Fbwidth = wResizeWidth * 2 ;
				if(Jmcu_Fbwidth & 0x3)
					Jmcu_Fbwidth = ALIGN_4(Jmcu_Fbwidth) ;
				g_wDecodedImageType = 422;
		}
#else
				Jmcu_Fbwidth = wResizeWidth * 2 ;
				if(Jmcu_Fbwidth & 0x3)
					Jmcu_Fbwidth = ALIGN_4(Jmcu_Fbwidth) ;
				g_wDecodedImageType = 422;
#endif

#if CHASE_IN_DECODE||SET_DECODE_OFFSET_ENALBE
				if (st_wDecodeWinOffset>Jmcu_Fbwidth)
					Jmcu_Fbwidth=st_wDecodeWinOffset;
#endif		

		
		cdu->CduIc = 0;
#if CUT_IMAGE_HEIGHT_ENALBE
		if (st_wDecodeWinHeight)
			cdu->CduSize = ((st_wDecodeWinHeight - 1) << 16) | (psJpeg->wCduWidth - 1);
		else
#endif
		cdu->CduSize = ((psJpeg->wCduHeight - 1) << 16) | (psJpeg->wCduWidth - 1);
		cdu->CduC = psJpeg->dwCduControl | ((psJpeg->bJResizeRatio & 0x03) << 2);
		
#if (YUV444_ENABLE)
        if(JpegDecoder444 != 1)
        {
			if(psJpeg->wImageType==444)
			{
			  cdu->CduC |= 0x00001800;
			}
        }
#else
		if(psJpeg->wImageType==444)
			cdu->CduC |= 0x00001800;
#endif
		
		cdu->CduC |= 0x00004000;
		cdu->CduC &= 0xffffdfff;

		#if EREADER_ENABLE //  fix bug for The_story_of_King_Arthur_and_his_knights.epub
			if(psJpeg->bMONO == 1)
			{
				MP_ALERT("Jpg_Cdu_Decode_One MONO photo");
				cdu->CduC |= 0x00000800;
				cdu->CduC &= 0xffff8fff;	// One component and not Raw_FMT
			}
		#endif


		if(psJpeg->bMONO == 1)
		{
				MP_ALERT("Jpg_Cdu_Decode_One MONO photo");
				cdu->CduC |= 0x00000800;
				cdu->CduC &= 0xffff8fff;	// One component and not Raw_FMT
		}
		if(psJpeg->b422V == 1)
		{
				MP_ALERT("Jpg_Cdu_Decode_One Cdu b422V");
				cdu->CduC |= 0x000000c0;
		}
		if(psJpeg->bCMYK == 1)
		{
				MP_ALERT("Jpg_Cdu_Decode_One Decode CMYK");
				cdu->CduC &= 0xfffff7ff;	// Raw data mode
				cdu->CduC |= 0x00001000;	// Raw data mode
				cdu->CduC |= 0x00006000;	// Four component
				cdu->CduCmp0 = 0x00000000;
				cdu->CduCmp3 = 0x00000000;
		}

		

		if(psJpeg->wImageType==444)
			cdu->CduCmp0 = 0x00000000;
		else if(psJpeg->wImageType==422)
			cdu->CduCmp0 = 0x00000010;
		else if(psJpeg->wImageType==420)
			cdu->CduCmp0 = 0x00000030;
		cdu->CduCmp1 = 0x00000007;
		cdu->CduCmp2 = 0x00000007;
		
		cdu->CduC |= 0x00008000;	// enable dummy byte
		cdu->JVLC_DMASA = dwStartPoint & 0xfffffffc; ;
		//cdu->JVLC_BUFSZ |= ((Choose_Buff_Size(JPEG_SRC_BUF_SIZE_650)) << 4);
		cdu->JVLC_BUFSZ |= ((Choose_Buff_Size(JPEG_SRC_BUF_SIZE_650)) << 8);
		cdu->JVLC_BUFSZ |= 0x00000001 ; 
		cdu->JMCU_DMASA = (DWORD)pbTarget;
		cdu->JMCU_FBWID = Jmcu_Fbwidth;
		MP_DEBUG("Jmcu_Fbwidth = %d", Jmcu_Fbwidth);
		MP_DEBUG("cdu->JVLC_DMASA=0x%08x, cdu->JVLC_BUFSZ=0x%08x", cdu->JVLC_DMASA, cdu->JVLC_BUFSZ);
		MP_DEBUG("cdu->JMCU_DMASA=0x%08x, cdu->JMCU_FBWID=0x%08x", cdu->JMCU_DMASA, cdu->JMCU_FBWID);
		
		Ch_Jvlc->Control |= 0x00000001 ;
		Ch_JMCU->Control |= 0x00000001 ;
		//cdu->JVLC_BUFSZ |= 0x0000000c ;
		cdu->JVLC_BUFSZ |= 0x00000030 ;
		cdu->CduOp = 1;
		
		if(((psJpeg->bDecodeMode & 0x3f) != IMG_DECODE_MJPEG)                      || 
			(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PREVIEW) 			
			#if MJPEG_ENABLE
			                                                                       ||
			g_bVideoResume                                                         ||               
			(1 == decoder_initialized)
			#endif
		  )
		{ 
			//Jpg_Cdu_Wait(JPEG_DECODE_TIMEOUT_CNT, 1);
			#if MJPEG_ENABLE
			g_bVideoResume = 0;
			#endif
#if USBCAM_IN_ENABLE
          Jpg_Cdu_Wait_Time(64, 1);
#else
      if((psJpeg->wCduWidth*psJpeg->wCduWidth)<=384000)
          Jpg_Cdu_Wait_Time(32, 1);
      else   
  	      Jpg_Cdu_Wait_Time((psJpeg->wCduWidth*psJpeg->wCduWidth / ( 12000 / JPEG_EXTRA_WAIT_TIME)), 1);/*384000 / 32 =12000*/
#endif
			CduDisable();
		}
	
		return 0 ;
}


int Jpg_Cdu_Decode_650(ST_MPX_STREAM *psStream, ST_JPEG *psJpeg, BYTE *pbTarget)
{
		BYTE bJResizeRatio, NeedChasing = 0;
		BYTE *pBuffer = NULL, *pHalfBuffer = NULL, *pQuadFirst = NULL, *pQuadThird = NULL, *pChasingBuffer = NULL;
		WORD wResizeWidth, wResizeHeight;	
		DWORD dwStartPoint, dwTotalSize, dwRemainSize;
		DWORD dwSrcBufferSize, dwReadSize;
		DWORD Jmcu_Fbwidth = 0, loading_times = 0 ;
		int iErrorCount = 1, i = 0;
		DWORD t=0, decode_done=0, WaitBufEmpty_Fail = 0 , ReadCardFail = 0, ret;
		DWORD LoadEvent;
		WORD  Auto_Chase_Img_Type = 422; 
		
		CDU *cdu = (CDU *) (CDU_BASE);
		CHANNEL *Ch_JMCU = (CHANNEL *) (DMA_JMCU_BASE);
		CHANNEL *Ch_Jvlc = (CHANNEL *) (DMA_JVLC_BASE);
		DMA* dma = (DMA*)DMA_BASE ;
		
		MP_DEBUG("Jpg_Cdu_Decode_Chase_650");
		MP_DEBUG("File size %d, eoipoint 0x%08x", mpxStreamGetSize(psStream), psJpeg->dwEoiPoint);	
		
		MP_DEBUG("psJpeg->bJResizeRatio %d", psJpeg->bJResizeRatio);

#if CHASE_IN_DECODE
		NeedChasing=GetNeedChase();
#endif
		bJResizeRatio = psJpeg->bJResizeRatio;
		if (bJResizeRatio>3) 
		{
			MP_ALERT("CDU resize ratio over 3, need Chasing");
			NeedChasing = 1 ;
			bJResizeRatio = 3 ;
			//return Jpg_Cdu_Auto_Chase_650(psStream, psJpeg, pbTarget);
		}
		if ((psJpeg->bDecodeMode & 0x3f) == IMG_DECODE_MJPEG)
		{
				wResizeWidth = psJpeg->wTargetWidth >> bJResizeRatio;
				wResizeHeight = psJpeg->wTargetHeight >> bJResizeRatio;		
		}
		else
		{
			wResizeWidth = psJpeg->wCduWidth >> bJResizeRatio;
			wResizeHeight = psJpeg->wCduHeight >> bJResizeRatio;	
		}
		
		dwStartPoint = psJpeg->dwEcsStart;
		dwTotalSize = psJpeg->dwEoiPoint - dwStartPoint;	// Image total size
		MP_DEBUG("## dwTotalSize=%d", dwTotalSize); 				
    dwSrcBufferSize = JPEG_SRC_BUF_SIZE_650; 
		if ((psJpeg->bDecodeMode & 0x3f) != IMG_DECODE_MJPEG)	//static image
	   		pBuffer = (BYTE *)ext_mem_malloc(dwSrcBufferSize + 256);
		else	//Mjpeg
		{
	  		if (g_pbChaseBuffer == NULL) g_pbChaseBuffer = (BYTE *)ext_mem_malloc(dwSrcBufferSize + 256);
	   		pBuffer = g_pbChaseBuffer;
		}
		if (pBuffer == NULL)
		{
				MP_ALERT("alloc pBuffer fail");
				return FAIL;
		}
		psJpeg->dwCduControl |= ((DWORD)pBuffer & 0x00000003) << 8;	// get CDU_DBS
		pBuffer = (BYTE *)((DWORD)pBuffer | 0xa0000000);
		if (mpxStreamSeek(psStream, dwStartPoint, SEEK_SET) == FAIL) 
		{
				MP_DEBUG("-E- mpxStreamSeek(psStream, dwStartPoint, SEEK_SET) == FAIL");
				ext_mem_free((BYTE *)((DWORD)pBuffer & ~0x20000000));
				return FAIL;
		}
		//Load Jpeg source
		if(dwTotalSize > dwSrcBufferSize)	
    {
    		MP_DEBUG("Need to load image source many times, dwTotalSize=%d", dwTotalSize);
				dwReadSize = mpxStreamRead(pBuffer, 1, dwSrcBufferSize, psStream);
    		loading_times = (dwTotalSize - dwSrcBufferSize) / (dwSrcBufferSize>>2) ;
    		//mpDebugPrint("Multi loading_times = %d", loading_times);
    		if((dwTotalSize - dwSrcBufferSize)%(dwSrcBufferSize>>2))
    			loading_times++ ;
    		dwRemainSize = dwTotalSize - dwSrcBufferSize ; 
    }
    else if(dwSrcBufferSize >= dwTotalSize) 
    {
    	MP_DEBUG("JPG check, dwTotalSize=%d", dwTotalSize);
    	dwReadSize = mpxStreamRead(pBuffer, 1, dwTotalSize, psStream);
    	InsertDummyByte(pBuffer, dwReadSize);
    	loading_times = 0 ;
    	MP_DEBUG("JPG check, dwReadSize=%d", dwReadSize);
    }
    MP_DEBUG("loading_times = %d", loading_times);
    pQuadFirst = pBuffer + (JPEG_SRC_BUF_SIZE_650>>2) ;
		pHalfBuffer = pBuffer + (JPEG_SRC_BUF_SIZE_650>>1) ;
		pQuadThird = pBuffer + (JPEG_SRC_BUF_SIZE_650>>1) + (JPEG_SRC_BUF_SIZE_650>>2) ;
		//Enable Auto Chasing or not
		if(NeedChasing == 1)
		{
			dma->JPG2SCLCtl1 |= 0x00000001 ;
			dma->JPG2SCLCtl0 |= 0x00000010 ;
			//mpDebugPrint("dma->JPG2SCLCtl0=%x, dma->JPG2SCLCtl1=%x, JPG2SCLCtl0 addr %x, JPG2SCLCtl1 addr %x", dma->JPG2SCLCtl0, dma->JPG2SCLCtl1, &dma->JPG2SCLCtl0, &dma->JPG2SCLCtl1);

						
#if (YUV444_ENABLE)
			if(JpegDecoder444 == 1)
			{
				pChasingBuffer = (BYTE *)ext_mem_malloc((((ALIGN_2(psJpeg->wCduWidth>>bJResizeRatio)) * 48)<<1) + 256); //need 48 lines = 16*3
			}
			else
			{
				pChasingBuffer = (BYTE *)ext_mem_malloc((((ALIGN_2(psJpeg->wCduWidth>>bJResizeRatio)) * 32)<<1) + 256); //need 32 lines
			}
						
#else
			pChasingBuffer = (BYTE *)ext_mem_malloc((((ALIGN_2(psJpeg->wCduWidth>>bJResizeRatio)) * 32)<<1) + 256); //need 32 lines
#endif

			
			if (pChasingBuffer == NULL)
			{
				MP_ALERT("alloc pChasingBuffer fail, not enough memory for chasing");
				return FAIL;
			}
#if (YUV444_ENABLE)
			if(JpegDecoder444 == 1)
			{
				Auto_Chase_Img_Type = psJpeg->wImageType;
			}
			else
			{
				Auto_Chase_Img_Type = 422;
			}
#else
			Auto_Chase_Img_Type = psJpeg->wImageType;
#endif			
#if CHASE_IN_DECODE
			Chase_Scaling(pChasingBuffer, pbTarget, psJpeg->wImageWidth, psJpeg->wImageHeight, ((ST_IMGWIN  *)Idu_GetNextWin())->wWidth,((ST_IMGWIN  *)Idu_GetNextWin())->wHeight, Auto_Chase_Img_Type);
#else
			Auto_Chase_Scaling(pChasingBuffer, pbTarget, psJpeg->wImageWidth, psJpeg->wImageHeight, psJpeg->wCduWidth, psJpeg->bJResizeRatio, Auto_Chase_Img_Type);
			//Auto_Chase_Scaling(pChasingBuffer, pbTarget, psJpeg->wCduWidth, psJpeg->wCduHeight, psJpeg->wCduWidth, psJpeg->bJResizeRatio, psJpeg->wImageType);//TYChen fix
#endif
			TimerDelay(700);// Frank Lin add for bug353
		}
		//CDU register ++
		cdu->CduIc = 0;
		cdu->CduC = 0 ;
		if(psJpeg->bCMYK == 1)
			cdu->CduSize = ((psJpeg->wCduHeight - 1) << 16) | ((psJpeg->wCduWidth<<2) - 1);
		else
			cdu->CduSize = ((psJpeg->wCduHeight - 1) << 16) | (psJpeg->wCduWidth - 1);
		//cdu->CduC = psJpeg->dwCduControl | ((psJpeg->bJResizeRatio & 0x03) << 2);
		cdu->CduC = psJpeg->dwCduControl | ((bJResizeRatio & 0x03) << 2);
		//cdu->CduC |= 0x00008000;	// enable dummy byte
		MP_DEBUG("psJpeg->wImageType=%d, psJpeg->dwCduControl= 0x%08x, psJpeg->bJResizeRatio= 0x%08x", psJpeg->wImageType, psJpeg->dwCduControl, psJpeg->bJResizeRatio);
#if (YUV444_ENABLE)
        if(JpegDecoder444 != 1)
        {
			if(psJpeg->wImageType==444)
				cdu->CduC |= 0x00001800;
        }
#else
		if(psJpeg->wImageType==444)
				cdu->CduC |= 0x00001800;
#endif
		cdu->CduC |= 0x00004000;
		cdu->CduC &= 0xffffdfff;
		if(psJpeg->wImageType==444)
				cdu->CduCmp0 = 0x00000000;
		else if(psJpeg->wImageType==422)
				cdu->CduCmp0 = 0x00000010;
		else if(psJpeg->wImageType==420)
				cdu->CduCmp0 = 0x00000030;
		cdu->CduCmp1 = 0x00000007;
		cdu->CduCmp2 = 0x00000007;
		if(psJpeg->bMONO == 1)
		{
				MP_ALERT("MONO photo");
				cdu->CduC |= 0x00000800;
				cdu->CduC &= 0xffff8fff;	// One component and not Raw_FMT
		}
		if(psJpeg->b422V == 1)
		{
				MP_ALERT("Cdu b422V");
				cdu->CduC |= 0x000000c0;
		}
		if(psJpeg->bCMYK == 1)
		{
				MP_ALERT("Decode CMYK");
				cdu->CduC &= 0xfffff7ff;	// Raw data mode
				cdu->CduC |= 0x00001000;	// Raw data mode
				cdu->CduC |= 0x00006000;	// Four component
				cdu->CduCmp0 = 0x00000000;
				cdu->CduCmp3 = 0x00000000;
		}
		if(psJpeg->bRotate)
		{
			MP_ALERT("Decode Rotate");
			cdu->CduC |= ((psJpeg->bRotate) << 16);
		}
		MP_DEBUG("Cdu setting for 650\r\n");
		MP_DEBUG("cdu->CduC = 0x%08x", cdu->CduC);
		MP_DEBUG("cdu->CduSize = 0x%08x", cdu->CduSize);
		MP_DEBUG("cdu->CduCmp0 = 0x%08x", cdu->CduCmp0);
		MP_DEBUG("cdu->CduCmp1 = 0x%08x", cdu->CduCmp1);
		MP_DEBUG("cdu->CduCmp2 = 0x%08x", cdu->CduCmp2);
		//CDU register --

		// Calculate JMCU Width in byte unit
		MP_DEBUG("wResizeWidth=%d", wResizeWidth);
		// Rotate 180 degree or Rotate 0 degree
		if(psJpeg->bRotate == 0 || psJpeg->bRotate == 2)		
		{
			if(psJpeg->wImageType==444)
			{
#if YUV444_ENABLE
	     	if(JpegDecoder444 == 1)
	     	{
				if(wResizeWidth%4 != 0)
					Jmcu_Fbwidth = ALIGN_4(wResizeWidth) * 3 ;
				else 
					Jmcu_Fbwidth = wResizeWidth * 3 ;

				g_wDecodedImageType = 444;
	     	}
			else
			{
			 	if(wResizeWidth%2 != 0)
					Jmcu_Fbwidth = ALIGN_2(wResizeWidth) * 2 ;
				else 
					Jmcu_Fbwidth = wResizeWidth * 2 ;

				g_wDecodedImageType = 422;
			}
#else
				if(wResizeWidth%2 != 0)
					Jmcu_Fbwidth = ALIGN_2(wResizeWidth) * 2 ;
				else 
					Jmcu_Fbwidth = wResizeWidth * 2 ;

				g_wDecodedImageType = 422;
#endif
			}
			else	
			{
				if(wResizeWidth%2 != 0)
					Jmcu_Fbwidth = ALIGN_2(wResizeWidth) * 2 ;
				else 
					Jmcu_Fbwidth = wResizeWidth * 2 ;
				if(psJpeg->bCMYK == 1)
					Jmcu_Fbwidth = Jmcu_Fbwidth << 1 ;
			}
		}
		// Rotate 90 degree or Rotate -90 degree
		if(psJpeg->bRotate == 1 || psJpeg->bRotate == 3)		
		{
			if(psJpeg->wImageType==444)
			{
#if YUV444_ENABLE
				if(JpegDecoder444)
				{
				  if(wResizeHeight%4 != 0)
					Jmcu_Fbwidth = ALIGN_4(wResizeHeight) * 3 ;
				  else 
					Jmcu_Fbwidth = wResizeHeight * 3 ;

				  g_wDecodedImageType = 444;
				
				}else{
					if(wResizeHeight%2 != 0)
						Jmcu_Fbwidth = ALIGN_2(wResizeHeight) * 2 ;
					else 
						Jmcu_Fbwidth = wResizeHeight * 2 ;

					g_wDecodedImageType = 422;
				}

#else
				if(wResizeHeight%2 != 0)
					Jmcu_Fbwidth = ALIGN_2(wResizeHeight) * 2 ;
				else 
					Jmcu_Fbwidth = wResizeHeight * 2 ;

				g_wDecodedImageType = 422;
#endif
			}
			else	
			{
				if(wResizeWidth%2 != 0)
					Jmcu_Fbwidth = ALIGN_2(wResizeHeight) * 2 ;
				else 
					Jmcu_Fbwidth = wResizeHeight * 2 ;
			}
		}
		
#if CHASE_IN_DECODE
				if (st_wDecodeWinOffset>Jmcu_Fbwidth)
					Jmcu_Fbwidth=st_wDecodeWinOffset;
#endif		
		// CDU DMA
		cdu->JVLC_DMASA = (DWORD)pBuffer ;
		cdu->JVLC_BUFSZ |= ((Choose_Buff_Size(JPEG_SRC_BUF_SIZE_650)) << 8);
		cdu->JVLC_BUFSZ |= 0x00000001 ; 
		if(NeedChasing == 1)
			cdu->JMCU_DMASA = (DWORD)pChasingBuffer;
		else
			cdu->JMCU_DMASA = (DWORD)pbTarget;
		// rotate	: Write DMA adjust
		if(psJpeg->bRotate == 1)
			cdu->JMCU_DMASA = cdu->JMCU_DMASA + (wResizeHeight * 2) - Adjust_RotateMCU(psJpeg->bRotate, psJpeg->wImageType, bJResizeRatio, psJpeg->b422V);
		else if(psJpeg->bRotate == 2)	
			cdu->JMCU_DMASA = cdu->JMCU_DMASA + (wResizeHeight * wResizeWidth *2) - Adjust_RotateMCU(psJpeg->bRotate, psJpeg->wImageType, bJResizeRatio, psJpeg->b422V) ;	
		else if(psJpeg->bRotate == 3)	
			cdu->JMCU_DMASA = cdu->JMCU_DMASA + (wResizeHeight * wResizeWidth *2) - (wResizeHeight << 1) ;
		cdu->JMCU_FBWID = Jmcu_Fbwidth;
		Ch_Jvlc->Control |= 0x00000001 ;
		Ch_JMCU->Control |= 0x00000001 ;
		
		MP_DEBUG("pBuffer=0x%08x", pBuffer);
		MP_DEBUG("pQuadFirst=0x%08x", pQuadFirst);
		MP_DEBUG("pHalfBuffer=0x%08x", pHalfBuffer);
		MP_DEBUG("pQuadThird=0x%08x", pQuadThird);
		MP_DEBUG("Jmcu_Fbwidth = %d", Jmcu_Fbwidth);
		MP_DEBUG("cdu->JVLC_DMASA=0x%08x, cdu->JVLC_BUFSZ=0x%08x", cdu->JVLC_DMASA, cdu->JVLC_BUFSZ);
		MP_DEBUG("cdu->JMCU_DMASA=0x%08x, cdu->JMCU_FBWID=0x%08x", cdu->JMCU_DMASA, cdu->JMCU_FBWID);
		// enable CDU operation
		//cdu->JVLC_BUFSZ |= 0x000000c0 ;
		cdu->JVLC_BUFSZ |= 0x000000f0 ;
		cdu->JVLC_BUFSZ |= 0x00000002 ;	// use 4 buffer
		// Interrupt setting
		Reset_CDU_ISR_Count();
		cdu->CduIc |= 0x00008000 ;
		SystemIntEna(IM_JPEG);
		cdu->CduOp = 1;		
		
		//Polling CDU BUF_VLD
		
		for(i=0 ; i<loading_times ; i++)
		{

           if(Jpg_PollEvent(psJpeg))
           {
		   	 WaitBufEmpty_Fail =1;
		   	 break;
           }
			
			MP_DEBUG("loading_times > 0");
			if(i%4==0)	// Buff 0 done
			{
				//ret = WaitBuffEmpty(0);
				MP_DEBUG("Event wait %d", i);
				ret = EventWaitWithTO(JPEG_LOAD_DATA_EVENT_ID1, BIT0, OS_EVENT_OR, &LoadEvent, 800);
				MP_DEBUG("Event arrive %d", i);
				if(ret)
				{	
					if(cdu->CduIc&0x1)
					{
						MP_ALERT("Have finished early");
						break ;
					}
					else
					{
						MP_ALERT("Wait JPEG_LOAD_DATA_EVENT Time Out");
						WaitBufEmpty_Fail = 1 ;
						break;
					}
				}
//				mpDebugPrint("Load Src data to Buff 0");
				if(dwRemainSize>=(dwSrcBufferSize>>2))
				{
					dwReadSize = mpxStreamRead(pBuffer, 1, (dwSrcBufferSize>>2), psStream);
					if(dwReadSize != (dwSrcBufferSize>>2))
					{
						MP_ALERT("Read Card fail");
						ReadCardFail = 1 ;
						break ;
					}
					dwRemainSize = dwRemainSize - (dwSrcBufferSize>>2) ;
				}
				else
				{
					dwReadSize = mpxStreamRead(pBuffer, 1, dwRemainSize, psStream);
					if(dwReadSize != dwRemainSize)
					{
						MP_ALERT("Read Card fail");
						ReadCardFail = 1 ;
						break ;
					}
					InsertDummyByte(pBuffer, dwReadSize);
				}
				MP_DEBUG("Load finish");
				cdu->JVLC_BUFSZ |= 0x10 ;
			}
			else if(i%4==1)	// Buff 1 done
			{
				//ret = WaitBuffEmpty(1);
				MP_DEBUG("Event wait %d", i);
				ret = EventWaitWithTO(JPEG_LOAD_DATA_EVENT_ID1, BIT1, OS_EVENT_OR, &LoadEvent, 800);
				MP_DEBUG("Event arrive %d", i);
				if(ret)
				{	
					if(cdu->CduIc&0x1)
					{
						MP_ALERT("Have finished early");
						break ;
					}
					else
					{
						MP_ALERT("Wait JPEG_LOAD_DATA_EVENT Time Out");
						WaitBufEmpty_Fail = 1 ;
						break;
					}
				}
//				mpDebugPrint("Load Src data to Buff 1");
				if(dwRemainSize>=(dwSrcBufferSize>>2))
				{
					dwReadSize = mpxStreamRead(pQuadFirst, 1, (dwSrcBufferSize>>2), psStream);
					if(dwReadSize != (dwSrcBufferSize>>2))
					{
						MP_ALERT("Read Card fail");
						ReadCardFail = 1 ;
						break ;
					}	
					dwRemainSize = dwRemainSize - (dwSrcBufferSize>>2) ;
				}
				else
				{
					dwReadSize = mpxStreamRead(pQuadFirst, 1, dwRemainSize, psStream);
					if(dwReadSize != dwRemainSize)
					{
						MP_ALERT("Read Card fail");
						ReadCardFail = 1 ;
						break ;
					}
					InsertDummyByte(pQuadFirst, dwReadSize);
				}
				MP_DEBUG("Load finish");
				cdu->JVLC_BUFSZ |= 0x20 ;
			}
			else if(i%4==2)	// Buff 2 done
			{
				//ret = WaitBuffEmpty(2);
				MP_DEBUG("Event wait %d", i);
				ret = EventWaitWithTO(JPEG_LOAD_DATA_EVENT_ID1, BIT2, OS_EVENT_OR, &LoadEvent, 800);
				MP_DEBUG("Event arrive %d", i);
				if(ret)
				{	
					if(cdu->CduIc&0x1)
					{
						MP_ALERT("Have finished early");
						break ;
					}
					else
					{
						MP_ALERT("Wait JPEG_LOAD_DATA_EVENT Time Out");
						WaitBufEmpty_Fail = 1 ;
						break;
					}
				}
//				mpDebugPrint("Load Src data to Buff 2");
				if(dwRemainSize>=(dwSrcBufferSize>>2))
				{
					dwReadSize = mpxStreamRead(pHalfBuffer, 1, (dwSrcBufferSize>>2), psStream);
					if(dwReadSize != (dwSrcBufferSize>>2))
					{
						MP_ALERT("Read Card fail");
						ReadCardFail = 1 ;
						break ;
					}	
					dwRemainSize = dwRemainSize - (dwSrcBufferSize>>2) ;
				}
				else
				{
					dwReadSize = mpxStreamRead(pHalfBuffer, 1, dwRemainSize, psStream);
					if(dwReadSize != dwRemainSize)
					{
						MP_ALERT("Read Card fail");
						ReadCardFail = 1 ;
						break ;
					}
					InsertDummyByte(pHalfBuffer, dwReadSize);
				}
				MP_DEBUG("Load finish");
				cdu->JVLC_BUFSZ |= 0x40 ;
			}
			else if(i%4==3)	// Buff 3 done
			{
				//ret = WaitBuffEmpty(3);
				MP_DEBUG("Event wait %d", i);
				ret = EventWaitWithTO(JPEG_LOAD_DATA_EVENT_ID1, BIT3, OS_EVENT_OR, &LoadEvent, 800);
				MP_DEBUG("Event arrive %d", i);
				if(ret)
				{	
					if(cdu->CduIc&0x1)
					{
						MP_ALERT("Have finished early");
						break ;
					}
					else
					{
						MP_ALERT("Wait JPEG_LOAD_DATA_EVENT Time Out");
						WaitBufEmpty_Fail = 1 ;
						break;
					}
				}
//				mpDebugPrint("Load Src data to Buff 3");
				if(dwRemainSize>=(dwSrcBufferSize>>2))
				{
					dwReadSize = mpxStreamRead(pQuadThird, 1, (dwSrcBufferSize>>2), psStream);
					if(dwReadSize != (dwSrcBufferSize>>2))
					{
						MP_ALERT("Read Card fail");
						ReadCardFail = 1 ;
						break ;
					}
					dwRemainSize = dwRemainSize - (dwSrcBufferSize>>2) ;
				}
				else
				{
					dwReadSize = mpxStreamRead(pQuadThird, 1, dwRemainSize, psStream);
					if(dwReadSize != dwRemainSize)
					{
						MP_ALERT("Read Card fail");
						ReadCardFail = 1 ;
						break ;
					}
					InsertDummyByte(pQuadThird, dwReadSize);
				}
				MP_DEBUG("Load finish");
				cdu->JVLC_BUFSZ |= 0x80 ;
			}
		}
		SystemIntDis(IM_JPEG);
		
		MP_DEBUG("check finish");
		//Polling CDU finish
		while(t<(JPEG_DECODE_TIMEOUT_CNT) && (WaitBufEmpty_Fail != 1) && (ReadCardFail != 1))
		{
			if(( t % 500) == 0)
			  TaskYield();/*When checking CduIc, it shares CPU resource to another task*/
			
			if(cdu->CduIc&0x1)
			{
				MP_DEBUG("Decode Finish");
				decode_done = 1 ;
				iErrorCount = 0 ;
				break ;
			}
			t++;
		}
		if(decode_done != 1)
		{
			MP_ALERT("CDU decode fail");
			iErrorCount = 100 ;
		}
		
		Ch_JMCU->Control = 0;
		CduDisable();
		if ((psJpeg->bDecodeMode & 0x3f) != IMG_DECODE_MJPEG)
				ext_mem_free((BYTE *)((DWORD)pBuffer & ~0x20000000));
		if(NeedChasing == 1)
		{
				ext_mem_free((BYTE *)((DWORD)pChasingBuffer & ~0x20000000));
				dma->JPG2SCLCtl0 = 0x00000001 ;		// after chasing need to reset
				dma->JPG2SCLCtl0 = 0x00000000 ;
		}

        if(g_wDecodedImageType == 444)
        {
	  	  MP_DEBUG1("%s: JPEG decode, Decoded buffer is 444.", __FUNCTION__);
	  	  psJpeg->wImageType = 444;
        }
	    else
	    {
	  	  MP_DEBUG1("%s: JPEG decode, Decoded buffer is 422.", __FUNCTION__);
		  psJpeg->wImageType = 422;
	    }
	
		if(0)
		{		
		static DRIVE *sDrv;
		static STREAM *shandle;
		static int ret1;
		int len = 0;
		WORD width, height;
		width = 544;
		//if(width & 0xfff0)
		//width = (width + 15) & 0xfff0;
		height = 800;
		len = width * height *2;

					UartOutText("creat jpg.bin");
					UartOutText("width");
					UartOutValue(width, 4);
					UartOutText("height");
					UartOutValue(height, 4);
	    		sDrv=DriveGet(SD_MMC);
    			ret1=CreateFile(sDrv, "jpg", "bin");
        	if (ret1) UartOutText("create file fail\r\n");
        	shandle=FileOpen(sDrv);
        	if(!shandle) UartOutText("open file fail\r\n");

					ret1=FileWrite(shandle, pbTarget, len);
        	if(!ret1) UartOutText("write file fail\r\n");
        	FileClose(shandle);
        	UartOutText("\n\rfile close\n\r");		
		}			
		
		return iErrorCount;
}


void CduIsr()
{
	CDU *cdu = (CDU *) (CDU_BASE);
if (cdu->CduIc & BIT0)
{
cdu->CduIc &= ~BIT0;
EventSet(JPEG_LOAD_DATA_EVENT_ID1, BIT4);
}
	
	if(CDU_Isr_Cnt == 0)
	{
		cdu->CduIc &= ~0x00000080 ;
		EventSet(JPEG_LOAD_DATA_EVENT_ID1, BIT0);
		MP_DEBUG("S0");
	}
	else if(CDU_Isr_Cnt == 1)
	{
		cdu->CduIc &= ~0x00000080 ;
		EventSet(JPEG_LOAD_DATA_EVENT_ID1, BIT1);
		MP_DEBUG("S1");
	}
	else if(CDU_Isr_Cnt == 2)
	{
		cdu->CduIc &= ~0x00000080 ;
		EventSet(JPEG_LOAD_DATA_EVENT_ID1, BIT2);
		MP_DEBUG("S2");
	}
	else if(CDU_Isr_Cnt == 3)
	{
		cdu->CduIc &= ~0x00000080 ;
		EventSet(JPEG_LOAD_DATA_EVENT_ID1, BIT3);
		MP_DEBUG("S3");
	}
		
	CDU_Isr_Cnt = (CDU_Isr_Cnt+1)%4 ;
}

#endif

