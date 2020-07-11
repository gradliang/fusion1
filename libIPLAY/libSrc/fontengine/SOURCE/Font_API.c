/*        
// define this module show debug message or not,  0 : disable, 1 : enable

*/                              
                       
#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"
#include "devio.h"
#include "display.h"
#include "mpapi.h"
#include "flagdefine.h"
#include "Taskid.h"
#include <FontEngine.h>
#include <ctype.h>
#define printf mpDebugPrint //cj modify 011310
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//The following is identical between target and host
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

#include "Compactmetrics_PreGenData.h"

FT_Library    library;
FT_Face       face;
pstCacheManager pCM = NULL;
DWORD longjmpNumber =0;

void mpx_longjmp(jmp_buf env, int val)
{
	mpDebugPrint(" ERROR(memory over write) ?? longjmpNumber=%d",longjmpNumber);
	longjmpNumber++;
	return;
}
int mpx_setjmp(jmp_buf env)
{
	return 0;
}

//================================================
//System depedent functions
#if 0//def PC_DEVELOPMENT

const char* ttf_file_name = "C:\\Font_1.ttf";

DWORD RGB2YUV(BYTE R, BYTE G, BYTE B)
{
	DWORD Y, Cb, Cr;

    Y = ((306 * R) + (601 * G) + (117 * B)) >> 10;
    Cb = ((-173 * R) + (-339 * G) + (512 * B) + 131072) >> 10;
    Cr = ((512 * R) + (-429 * G) + (-83 * B) + 131072) >> 10;

	return (Y << 24) | (Y << 16) | (Cb << 8) | Cr;
}
void DebugPrintLine(char* fmt,...)
{
}
#endif

#ifdef TARGET_DEVELOPMENT
//void open( void ){};

#if 0
int Font_Data_At_File_Get_List(BYTE bMcardId, DWORD dwSearchCount)
{
	DRIVE *drv;
	BYTE  pCurExt[3];
	BYTE  *pCurName;
	uint  i, j;
    char  *pFontFileNameExt = "ttf";
	drv = DriveChange(bMcardId);
	int count = 0;

	// the code must be the first file of the selected card
	if (DirReset(drv) != FS_SUCCEED)
		return 0x0;
	if (DirFirst(drv) != FS_SUCCEED)
		return 0x0;


	for (i = 0; i < dwSearchCount; i++)
	{
		STREAM *handle = FileOpen(drv);

		if (!handle)
			return 0x0;

		strcpy(pCurExt, (BYTE*)drv->Node->Extension);

		for (j=0; j<3; j++)
		{
			pCurExt[j] = tolower(pCurExt[j]);
		}

		if (!strncmp(pFontFileNameExt, pCurExt, 3))
		{
			hFontFile = handle;
			pFontFileMemBuffer_Size = FileSizeGet(hFontFile);
			pFontFileMemBuffer_Start = (unsigned char*)SystemGetMemAddr(FONT_TTF_BUF_MEM_ID);
			FileRead(hFontFile, pFontFileMemBuffer_Start, pFontFileMemBuffer_Size);

			FileClose(handle);
			count++;

            return 1;
		}

		FileClose(handle);
		if (DirNext(drv) != FS_SUCCEED)
			return 0x0;
	}

	return 0;
}
#else
int Font_Data_At_File_Get_List(BYTE bMcardId, DWORD dwSearchCount)
{
    MP_DEBUG("%s: bMcardId = %d, dwSearchCount = %d", __func__, bMcardId, dwSearchCount);
	STREAM *handle;

	handle = FileExtSearchOpen(bMcardId, "ttf", dwSearchCount);
	if (handle == NULL)

	{
	   MP_ALERT("bMcardId(%d) search *.ttf fail!", bMcardId);
	   return -1;  
    }

	hFontFile = handle;
	pFontFileMemBuffer_Size = FileSizeGet(hFontFile);
	pFontFileMemBuffer_Start = ext_mem_malloc(sizeof(char)*pFontFileMemBuffer_Size);
	FileRead(hFontFile, pFontFileMemBuffer_Start, pFontFileMemBuffer_Size);
	FileClose(handle);

    return 1;
}
#endif

int OpenFontFileMemBuffer(DWORD ResourceTag)
{
  	pFontFileMemBuffer_Size = ISP_GetResourceSize(ResourceTag);
	if(pFontFileMemBuffer_Size == 0)
		return -1;

	pFontFileMemBuffer_Start = (BYTE *)ext_mem_malloc(pFontFileMemBuffer_Size + 4096);
	if(pFontFileMemBuffer_Start == NULL)
		return -1;

	pdwStream = (DWORD *) ((DWORD)pFontFileMemBuffer_Start | 0xa0000000);
	pFontFileMemBuffer_Size = ISP_GetResource(ResourceTag, (DWORD)pdwStream);

	return 0;
}

#endif
//================================================

void message_font_bmp()
{
	int i, j;
	for (i=0; i<face->glyph->bitmap.rows; i++)
	{
		for (j=0; j<face->glyph->bitmap.width; j++)
		{
			int data = face->glyph->bitmap.buffer[i*face->glyph->bitmap.width + j];
			((int)((data+1)/32) ==0) ? printf(" ") : printf("%d", ((int)((data+1)/32)));
		}
		printf("\r\n");
	}
}
void message_font_bmp_data()
{
	printf("face->glyph->bitmap.pitch = %d\r\n", face->glyph->bitmap.pitch);
	printf("face->glyph->bitmap.rows = %d\r\n", face->glyph->bitmap.rows);
	printf("face->glyph->bitmap.width = %d\r\n", face->glyph->bitmap.width);
	printf("face->glyph->bitmap.buffer = 0x%08x\r\n", face->glyph->bitmap.buffer);
	printf("face->glyph->bitmap_left = %d\r\n", face->glyph->bitmap_left);
	printf("face->glyph->bitmap_top = %d\r\n", face->glyph->bitmap_top);
}

void message_font_memory()
{
	int i;
	printf("face->glyph->bitmap.pitch = %d\r\n", face->glyph->bitmap.pitch);
	printf("face->glyph->bitmap.rows = %d\r\n", face->glyph->bitmap.rows);
	printf("face->glyph->bitmap.width = %d\r\n", face->glyph->bitmap.width);
	printf("face->glyph->bitmap.buffer = 0x%08x\r\n", face->glyph->bitmap.buffer);

	for (i=0; i<(face->glyph->bitmap.rows * face->glyph->bitmap.pitch); i++)
	{
		if ((i) % 16 == 0)	printf("0x%08x -- ", &(face->glyph->bitmap.buffer[i]));
		printf("%02x", face->glyph->bitmap.buffer[i]);
		if ((i+1) %  4 == 0)	printf(" ");
		if ((i+1) % 16 == 0)	printf("\r\n");
	}
	printf("\r\n");
}


#if 1
int DisplayDrawGlyphBMP_YYCBCR_V(stPrintTTFConfig* ttf_data, stPoint pt, pGlyphContainer pGlyphCon)
{
	int   rt;
	int   i, j;
	int   HeadOdd, TailOdd;
	BYTE* pDispProc = NULL;	//Display memory start address to be processed
	//int   two_order=0, nTmp;
	BYTE  mask0L, mask0R;
	BYTE  mask1a, mask1b;
	BYTE  *pData_BlendBMP_Piece_a = NULL, *pData_BlendBMP_Piece_b = NULL;
	BYTE  data_temp_a, data_temp_b;
	int   w0, w1;
	int   loop_bond_start, loop_bond_end, loop_step;

	BYTE* pGlyphBMPBuffer = pGlyphCon->pFontBitmap;
	FT_Pos  width;
    	FT_Pos  height;
	stColorData ColorData;
	//
	Translate_Color(ttf_data->ColorParameter_Glyph.Data, ttf_data->ColorParameter_Glyph.Format, &ColorData, ttf_data->ImagePlaneData.ImagePlaneMode);

	mask0R = ((0x1 << 8) - 1);

	mask0L = mask0R << (8 - 8);

	HeadOdd = (pt.x&0x1) ? 1:0;

	width = pGlyphCon->Metrics.width>>6;
	height = pGlyphCon->Metrics.height>>6;

	TailOdd = ((pt.x+ height) & 0x1) ? 1:0;

	for (i = 0; i<width; i++)
	{
		if (((pt.y+i)<0)||((pt.y+i)>=ttf_data->ImagePlaneData.ImagePlaneSize.y))
			continue;

		//While draw over region edge
		if ((ttf_data->PrintCoordinate.PrintRegionMode == PRINT_REGION_MODE_PRINT_REGION)&&
			((pt.y+i)>ttf_data->PrintCoordinate.PrintRegion.pt_2.y)&&
			(ttf_data->PrintCoordinate.NotDrawOverBorder && NOT_DRAW_OVER_BORDER_BOTTOM))
		{
			continue;
		}

		//pData_BlendBMP_Piece_a = &(pGlyphBMPBuffer[pGlyphBMPData->GlyphBMPMetrics.Pitch*i]);
		pData_BlendBMP_Piece_a = &(pGlyphBMPBuffer[height*i]);

		//pData_BlendBMP_Piece_b = (pGlyphBMPData->GlyphBMPMetrics.BitPerPixel == 8)?&(pGlyphBMPBuffer[pGlyphBMPData->GlyphBMPMetrics.Pitch*i + 1]):&(pGlyphBMPBuffer[pGlyphBMPData->GlyphBMPMetrics.Pitch*i]);
		pData_BlendBMP_Piece_b = &(pGlyphBMPBuffer[height*i + 1]);

		if (HeadOdd)
		{
			if ((pt.x>=0) && (pt.x<ttf_data->ImagePlaneData.ImagePlaneSize.x))
			{
				pDispProc = (BYTE*)ttf_data->ImagePlaneData.ImagePlane + (ttf_data->ImagePlaneData.ImagePlaneSize.x*(pt.y+i) + pt.x)*2;
				//data_temp_a = (*pData_BlendBMP_Piece_a & mask0L) >> (8 - pGlyphBMPData->GlyphBMPMetrics.BitPerPixel);
				data_temp_a = (*pData_BlendBMP_Piece_a & mask0L);
				// Y Y
				//w1 = ((1<<pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)-1);
				w1 = ((1<<8)-1);

				*(pDispProc-1) = ((ColorData.color_1*data_temp_a) + (*(pDispProc-1))*(w1-data_temp_a))/w1;
				// CB, CR
				w0 = data_temp_a;
				//w1 = (((1<<pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)-1)<<1);
				w1 = (((1<<8)-1)<<1);
				*(pDispProc  ) = (ColorData.color_2*w0 + (*(pDispProc  ))*(w1-w0))/w1;
				*(pDispProc+1) = (ColorData.color_3*w0 + (*(pDispProc+1))*(w1-w0))/w1;
			}
			//if (pGlyphBMPData->GlyphBMPMetrics.BitPerPixel == 8)
				pData_BlendBMP_Piece_a++;
				pData_BlendBMP_Piece_b++;
		}

			loop_bond_start = (HeadOdd)?1:0;
			//loop_bond_end   = (TailOdd)?(pGlyphBMPData->GlyphBMPMetrics.Width-1):pGlyphBMPData->GlyphBMPMetrics.Width;
			loop_bond_end   = (TailOdd)?(height-1):(height);
			loop_step       = 2;
		//
		for ( j = loop_bond_start; j < loop_bond_end; j += loop_step)
		{
			if (((pt.x+j)>=0) && ((pt.x+j)<ttf_data->ImagePlaneData.ImagePlaneSize.x))
			{
				pDispProc = (BYTE*)ttf_data->ImagePlaneData.ImagePlane + (ttf_data->ImagePlaneData.ImagePlaneSize.x*(pt.y+i) + pt.x)*2 + j*2;

				//Get Mask and Data
				mask1a = mask0L >> (j%8);
				//mask1b = mask0L >> ((j+pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)%8);
				mask1b = mask0L >> ((j+8)%8);

				//data_temp_a = (pGlyphBMPData->GlyphBMPMetrics.BitPerPixel == 8)?(*pData_BlendBMP_Piece_a):((*pData_BlendBMP_Piece_a & mask1a) >> (8-pGlyphBMPData->GlyphBMPMetrics.BitPerPixel-(j%8)));
				data_temp_a = (*pData_BlendBMP_Piece_a);
				//data_temp_b = (pGlyphBMPData->GlyphBMPMetrics.BitPerPixel == 8)?(*pData_BlendBMP_Piece_b):(*pData_BlendBMP_Piece_b & mask1b) >> (8-pGlyphBMPData->GlyphBMPMetrics.BitPerPixel-((j+pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)%8)) ;
				data_temp_b = (*pData_BlendBMP_Piece_b);

				// Blending
				if ((data_temp_a) || (data_temp_b))
				{
					// Y Y
					//w1 = ((1<<pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)-1);
					w1 = ((1<<8)-1);
					*(pDispProc  ) = ((ColorData.color_1*data_temp_a) + (*(pDispProc  ))*(w1-data_temp_a))/w1;
					*(pDispProc+1) = ((ColorData.color_1*data_temp_b) + (*(pDispProc+1))*(w1-data_temp_b))/w1;
					// CB, CR
					w0 = data_temp_a+data_temp_b;
					//w1 = (((1<<pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)-1)<<1);
					w1 = (((1<<8)-1)<<1);
					*(pDispProc+2) = (ColorData.color_2*w0 + (*(pDispProc+2))*(w1-w0))/w1;
					*(pDispProc+3) = (ColorData.color_3*w0 + (*(pDispProc+3))*(w1-w0))/w1;
				}
			}
			//
			// Move Data Address
			//if (pGlyphBMPData->GlyphBMPMetrics.BitPerPixel == 8)

				pData_BlendBMP_Piece_a += 2;
				pData_BlendBMP_Piece_b += 2;
		}

		//if ((pGlyphBMPData->GlyphBMPMetrics.BitPerPixel == 8) && (TailOdd) && ((pt.x+pGlyphBMPData->GlyphBMPMetrics.Width-1)< ttf_data->ImagePlaneData.ImagePlaneSize.x))
		if ((8 == 8) && (TailOdd) && ((pt.x+height-1)< ttf_data->ImagePlaneData.ImagePlaneSize.x))
		{
			data_temp_a = (*pData_BlendBMP_Piece_a);
			data_temp_b = 0;
			//pDispProc = (BYTE*)ttf_data->ImagePlaneData.ImagePlane + (ttf_data->ImagePlaneData.ImagePlaneSize.x*(pt.y+i) + pt.x)*2 + (pGlyphBMPData->GlyphBMPMetrics.Width-1)*2;
			pDispProc = (BYTE*)ttf_data->ImagePlaneData.ImagePlane + (ttf_data->ImagePlaneData.ImagePlaneSize.x*(pt.y+i) + pt.x)*2 + (height-1)*2;
			// Y Y
			//w1 = ((1<<pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)-1);
			w1 = ((1<<8)-1);
			*(pDispProc  ) = ((ColorData.color_1*data_temp_a) + (*(pDispProc  ))*(w1-data_temp_a))/w1;
			*(pDispProc+1) = ((ColorData.color_1*data_temp_b) + (*(pDispProc+1))*(w1-data_temp_b))/w1;
			// CB, CR
			w0 = data_temp_a+data_temp_b;
			//w1 = (((1<<pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)-1)<<1);
			w1 = (((1<<8)-1)<<1);
			*(pDispProc+2) = (ColorData.color_2*w0 + (*(pDispProc+2))*(w1-w0))/w1;
			*(pDispProc+3) = (ColorData.color_3*w0 + (*(pDispProc+3))*(w1-w0))/w1;
			//
		}
		rt = 0;
	}
	//-----
	return rt;
}


#else

int DisplayDrawGlyphBMP_YYCBCR_V(stPrintTTFConfig* ttf_data, stPoint pt, pGlyphContainer pGlyphCon)
{
	int   rt;
	int   i, j;
	int   HeadOdd, TailOdd;
	BYTE* pDispProc = NULL;	//Display memory start address to be processed
	int   two_order=0, nTmp;
	BYTE  mask0L, mask0R;
	BYTE  mask1a, mask1b;
	BYTE  *pData_BlendBMP_Piece_a = NULL, *pData_BlendBMP_Piece_b = NULL;
	BYTE  data_temp_a, data_temp_b;
	int   w0, w1;
	int   loop_bond_start, loop_bond_end, loop_step;

	BYTE* pGlyphBMPBuffer = pGlyphCon->pFontBitmap;

	stColorData ColorData;
	//
	Translate_Color(ttf_data->ColorParameter_Glyph.Data, ttf_data->ColorParameter_Glyph.Format, &ColorData, ttf_data->ImagePlaneData.ImagePlaneMode);

	for (i = 0; i<(pGlyphCon->Metrics.width>>6); i++)
	{
		pData_BlendBMP_Piece_a = &(pGlyphBMPBuffer[(pGlyphCon->Metrics.height>>6)*i]);

		pData_BlendBMP_Piece_b = &(pGlyphBMPBuffer[(pGlyphCon->Metrics.height>>6)*i + 1]);

		loop_bond_end   = pGlyphCon->Metrics.height>>6;
		loop_step       = 2;
		//
		for ( j = loop_bond_start; j < loop_bond_end; j += loop_step)
		{
			if (((pt.x+j)>=0) && ((pt.x+j)<ttf_data->ImagePlaneData.ImagePlaneSize.x))
			{
				pDispProc = (BYTE*)ttf_data->ImagePlaneData.ImagePlane + (ttf_data->ImagePlaneData.ImagePlaneSize.x*(pt.y+i) + pt.x)*2 + j*2;

				//Get Mask and Data
				mask1a = mask0L >> (j%8);
				//mask1b = mask0L >> ((j+pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)%8);
				mask1b = mask0L >> ((j+8)%8);

				//data_temp_a = (pGlyphBMPData->GlyphBMPMetrics.BitPerPixel == 8)?(*pData_BlendBMP_Piece_a):((*pData_BlendBMP_Piece_a & mask1a) >> (8-pGlyphBMPData->GlyphBMPMetrics.BitPerPixel-(j%8)));
				data_temp_a = (*pData_BlendBMP_Piece_a);
				//data_temp_b = (pGlyphBMPData->GlyphBMPMetrics.BitPerPixel == 8)?(*pData_BlendBMP_Piece_b):(*pData_BlendBMP_Piece_b & mask1b) >> (8-pGlyphBMPData->GlyphBMPMetrics.BitPerPixel-((j+pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)%8)) ;
				data_temp_b = (*pData_BlendBMP_Piece_b);

				// Blending
				if ((data_temp_a) || (data_temp_b))
				{
					// Y Y
					//w1 = ((1<<pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)-1);
					w1 = ((1<<8)-1);
					*(pDispProc  ) = ((ColorData.color_1*data_temp_a) + (*(pDispProc  ))*(w1-data_temp_a))/w1;
					*(pDispProc+1) = ((ColorData.color_1*data_temp_b) + (*(pDispProc+1))*(w1-data_temp_b))/w1;
					// CB, CR
					w0 = data_temp_a+data_temp_b;
					//w1 = (((1<<pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)-1)<<1);
					w1 = (((1<<8)-1)<<1);
					*(pDispProc+2) = (ColorData.color_2*w0 + (*(pDispProc+2))*(w1-w0))/w1;
					*(pDispProc+3) = (ColorData.color_3*w0 + (*(pDispProc+3))*(w1-w0))/w1;
				}
			}
			pData_BlendBMP_Piece_a += 2;
			pData_BlendBMP_Piece_b += 2;

		}
		rt = 0;
	}
	//-----
	return rt;
}




#endif

int DisplayDrawGlyphBMP_YYCBCR_H(stPrintTTFConfig* ttf_data, stPoint pt, pGlyphContainer pGlyphCon)
{
	int   rt;
	int   i, j;
	int   HeadOdd, TailOdd;
	BYTE* pDispProc = NULL;	//Display memory start address to be processed
	int   two_order=0, nTmp;
	BYTE  mask0L, mask0R;
	BYTE  mask1a, mask1b;
	BYTE  *pData_BlendBMP_Piece_a = NULL, *pData_BlendBMP_Piece_b = NULL;
	BYTE  data_temp_a, data_temp_b;
	int   w0, w1;
	int   loop_bond_start, loop_bond_end, loop_step;
	//BYTE* pGlyphBMPBuffer = (BYTE*)pGlyphCon->Glyph->;
	//BYTE* pGlyphBMPBuffer = ((FT_BitmapGlyph)pGlyphCon->Glyph)->bitmap.buffer;
	BYTE* pGlyphBMPBuffer = pGlyphCon->pFontBitmap;

	stColorData ColorData;
	//
	Translate_Color(ttf_data->ColorParameter_Glyph.Data, ttf_data->ColorParameter_Glyph.Format, &ColorData, ttf_data->ImagePlaneData.ImagePlaneMode);

	//
	//nTmp = pGlyphCon->GlyphBMPMetrics.BitPerPixel;
	nTmp = 8;

	while( nTmp >>= 1)
		two_order++;

	//if ((0x1<<two_order) != pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)
	if ((0x1<<two_order) != 8)
		return (-1);

	//mask0R = ((0x1 << pGlyphBMPData->GlyphBMPMetrics.BitPerPixel) - 1);
	mask0R = ((0x1 << 8) - 1);
	//mask0L = mask0R << (8 - pGlyphBMPData->GlyphBMPMetrics.BitPerPixel);
	mask0L = mask0R << (8 - 8);

	HeadOdd = (pt.x&0x1) ? 1:0;
	//TailOdd = ((pt.x+pGlyphBMPData->GlyphBMPMetrics.Width)&0x1) ? 1:0;
	TailOdd = ((pt.x+(pGlyphCon->Metrics.width>>6))&0x1) ? 1:0;

	//for (i = 0; i<pGlyphBMPData->GlyphBMPMetrics.Height; i++)
	for (i = 0; i<(pGlyphCon->Metrics.height>>6); i++)
	{
		if (((pt.y+i)<0)||((pt.y+i)>=ttf_data->ImagePlaneData.ImagePlaneSize.y))
			continue;

		//While draw over region edge
		if ((ttf_data->PrintCoordinate.PrintRegionMode == PRINT_REGION_MODE_PRINT_REGION)&&
			((pt.y+i)>ttf_data->PrintCoordinate.PrintRegion.pt_2.y)&&
			(ttf_data->PrintCoordinate.NotDrawOverBorder && NOT_DRAW_OVER_BORDER_BOTTOM))
		{
			continue;
		}

		//pData_BlendBMP_Piece_a = &(pGlyphBMPBuffer[pGlyphBMPData->GlyphBMPMetrics.Pitch*i]);
		pData_BlendBMP_Piece_a = &(pGlyphBMPBuffer[(pGlyphCon->Metrics.width>>6)*i]);

		//pData_BlendBMP_Piece_b = (pGlyphBMPData->GlyphBMPMetrics.BitPerPixel == 8)?&(pGlyphBMPBuffer[pGlyphBMPData->GlyphBMPMetrics.Pitch*i + 1]):&(pGlyphBMPBuffer[pGlyphBMPData->GlyphBMPMetrics.Pitch*i]);
		pData_BlendBMP_Piece_b = &(pGlyphBMPBuffer[(pGlyphCon->Metrics.width>>6)*i + 1]);

		if (HeadOdd)
		{
			if ((pt.x>=0) && (pt.x<ttf_data->ImagePlaneData.ImagePlaneSize.x))
			{
				pDispProc = (BYTE*)ttf_data->ImagePlaneData.ImagePlane + (ttf_data->ImagePlaneData.ImagePlaneSize.x*(pt.y+i) + pt.x)*2;
				//data_temp_a = (*pData_BlendBMP_Piece_a & mask0L) >> (8 - pGlyphBMPData->GlyphBMPMetrics.BitPerPixel);
				data_temp_a = (*pData_BlendBMP_Piece_a & mask0L);
				// Y Y
				//w1 = ((1<<pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)-1);
				w1 = ((1<<8)-1);

				*(pDispProc-1) = ((ColorData.color_1*data_temp_a) + (*(pDispProc-1))*(w1-data_temp_a))/w1;
				// CB, CR
				w0 = data_temp_a;
				//w1 = (((1<<pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)-1)<<1);
				w1 = (((1<<8)-1)<<1);
				*(pDispProc  ) = (ColorData.color_2*w0 + (*(pDispProc  ))*(w1-w0))/w1;
				*(pDispProc+1) = (ColorData.color_3*w0 + (*(pDispProc+1))*(w1-w0))/w1;
			}
			//if (pGlyphBMPData->GlyphBMPMetrics.BitPerPixel == 8)
			if (8 == 8)
			{
				pData_BlendBMP_Piece_a++;
				pData_BlendBMP_Piece_b++;
			}

		}

		//if (pGlyphBMPData->GlyphBMPMetrics.BitPerPixel == 8)
		if (8 == 8)
		{
			loop_bond_start = (HeadOdd)?1:0;
			//loop_bond_end   = (TailOdd)?(pGlyphBMPData->GlyphBMPMetrics.Width-1):pGlyphBMPData->GlyphBMPMetrics.Width;
			loop_bond_end   = (TailOdd)?((pGlyphCon->Metrics.width>>6)-1):(pGlyphCon->Metrics.width>>6);
			loop_step       = 2;
		}
		else
		{
			//loop_bond_start = (HeadOdd)?(1*pGlyphBMPData->GlyphBMPMetrics.BitPerPixel):0;
			loop_bond_start = (HeadOdd)?(1*8):0;
			//loop_bond_end   = ((TailOdd)?(pGlyphBMPData->GlyphBMPMetrics.Width-1):pGlyphBMPData->GlyphBMPMetrics.Width);
			loop_bond_end   = ((TailOdd)?((pGlyphCon->Metrics.width>>6)-1):(pGlyphCon->Metrics.width>>6));
			//loop_step       = pGlyphBMPData->GlyphBMPMetrics.BitPerPixel*2;
			loop_step       = 8*2;
		}

		//
		for ( j = loop_bond_start; j < loop_bond_end; j += loop_step)
		{
			if (((pt.x+j)>=0) && ((pt.x+j)<ttf_data->ImagePlaneData.ImagePlaneSize.x))
			{
				pDispProc = (BYTE*)ttf_data->ImagePlaneData.ImagePlane + (ttf_data->ImagePlaneData.ImagePlaneSize.x*(pt.y+i) + pt.x)*2 + j*2;

				//Get Mask and Data
				mask1a = mask0L >> (j%8);
				//mask1b = mask0L >> ((j+pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)%8);
				mask1b = mask0L >> ((j+8)%8);

				//data_temp_a = (pGlyphBMPData->GlyphBMPMetrics.BitPerPixel == 8)?(*pData_BlendBMP_Piece_a):((*pData_BlendBMP_Piece_a & mask1a) >> (8-pGlyphBMPData->GlyphBMPMetrics.BitPerPixel-(j%8)));
				data_temp_a = (*pData_BlendBMP_Piece_a);
				//data_temp_b = (pGlyphBMPData->GlyphBMPMetrics.BitPerPixel == 8)?(*pData_BlendBMP_Piece_b):(*pData_BlendBMP_Piece_b & mask1b) >> (8-pGlyphBMPData->GlyphBMPMetrics.BitPerPixel-((j+pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)%8)) ;
				data_temp_b = (*pData_BlendBMP_Piece_b);

				// Blending
				if ((data_temp_a) || (data_temp_b))
				{
					// Y Y
					//w1 = ((1<<pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)-1);
					w1 = ((1<<8)-1);
					*(pDispProc  ) = ((ColorData.color_1*data_temp_a) + (*(pDispProc  ))*(w1-data_temp_a))/w1;
					*(pDispProc+1) = ((ColorData.color_1*data_temp_b) + (*(pDispProc+1))*(w1-data_temp_b))/w1;
					// CB, CR
					w0 = data_temp_a+data_temp_b;
					//w1 = (((1<<pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)-1)<<1);
					w1 = (((1<<8)-1)<<1);
					*(pDispProc+2) = (ColorData.color_2*w0 + (*(pDispProc+2))*(w1-w0))/w1;
					*(pDispProc+3) = (ColorData.color_3*w0 + (*(pDispProc+3))*(w1-w0))/w1;
				}
			}
			//
			// Move Data Address
			//if (pGlyphBMPData->GlyphBMPMetrics.BitPerPixel == 8)
			if (8 == 8)
			{
				pData_BlendBMP_Piece_a += 2;
				pData_BlendBMP_Piece_b += 2;
			}
			else
			{
				//if (!(mask1a >> (pGlyphBMPData->GlyphBMPMetrics.BitPerPixel*2))) pData_BlendBMP_Piece_a++;
				if (!(mask1a >> (2))) pData_BlendBMP_Piece_a++;
				//if (!(mask1b >> (pGlyphBMPData->GlyphBMPMetrics.BitPerPixel*2))) pData_BlendBMP_Piece_b++;
				if (!(mask1b >> (2))) pData_BlendBMP_Piece_b++;
			}
		}

		//if ((pGlyphBMPData->GlyphBMPMetrics.BitPerPixel == 8) && (TailOdd) && ((pt.x+pGlyphBMPData->GlyphBMPMetrics.Width-1)< ttf_data->ImagePlaneData.ImagePlaneSize.x))
		if ((8 == 8) && (TailOdd) && ((pt.x+(pGlyphCon->Metrics.width>>6)-1)< ttf_data->ImagePlaneData.ImagePlaneSize.x))
		{
			data_temp_a = (*pData_BlendBMP_Piece_a);
			data_temp_b = 0;
			//pDispProc = (BYTE*)ttf_data->ImagePlaneData.ImagePlane + (ttf_data->ImagePlaneData.ImagePlaneSize.x*(pt.y+i) + pt.x)*2 + (pGlyphBMPData->GlyphBMPMetrics.Width-1)*2;
			pDispProc = (BYTE*)ttf_data->ImagePlaneData.ImagePlane + (ttf_data->ImagePlaneData.ImagePlaneSize.x*(pt.y+i) + pt.x)*2 + ((pGlyphCon->Metrics.width>>6)-1)*2;
			// Y Y
			//w1 = ((1<<pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)-1);
			w1 = ((1<<8)-1);
			*(pDispProc  ) = ((ColorData.color_1*data_temp_a) + (*(pDispProc  ))*(w1-data_temp_a))/w1;
			*(pDispProc+1) = ((ColorData.color_1*data_temp_b) + (*(pDispProc+1))*(w1-data_temp_b))/w1;
			// CB, CR
			w0 = data_temp_a+data_temp_b;
			//w1 = (((1<<pGlyphBMPData->GlyphBMPMetrics.BitPerPixel)-1)<<1);
			w1 = (((1<<8)-1)<<1);
			*(pDispProc+2) = (ColorData.color_2*w0 + (*(pDispProc+2))*(w1-w0))/w1;
			*(pDispProc+3) = (ColorData.color_3*w0 + (*(pDispProc+3))*(w1-w0))/w1;
			//
		}
		rt = 0;
	}
	//-----
	return rt;
}


int DisplayDrawGlyphBMP_RGB(stPrintTTFConfig* ttf_data, stPoint pt, pGlyphContainer pGlyphCon)
{
	int   rt = 0;
	int   i, j;
	BYTE* pDispProc = NULL;	//Display memory start address to be processed
	BYTE  GlyphPixelValue = 0;
	BYTE  ColorR,ColorG,ColorB;
	//BYTE* pGlyphBMPBuffer = ((FT_BitmapGlyph)pGlyphCon->Glyph)->bitmap.buffer;
	BYTE* pGlyphBMPBuffer = pGlyphCon->pFontBitmap;
	int GlyphBMPHeight = (pGlyphCon->Metrics.height>>6);
	int GlyphBMPWidth = (pGlyphCon->Metrics.width>>6);
	//

	if (ttf_data->ColorParameter_Glyph.Format != COLOR_FORMAT_RGB)
	{
		return -1;
	}
	if (ttf_data->Rendering != FT_RENDER_MODE_NORMAL)
	{
		return -1;
	}

	for (i=0; i<GlyphBMPHeight; i++)
	{
		if (((pt.y+i)<0) || ((pt.y+i)>=ttf_data->ImagePlaneData.ImagePlaneSize.y))
			continue;

		//While draw over region edge
		if ((ttf_data->PrintCoordinate.PrintRegionMode == PRINT_REGION_MODE_PRINT_REGION)&&
			((pt.y+i)>ttf_data->PrintCoordinate.PrintRegion.pt_2.y)&&
			(ttf_data->PrintCoordinate.NotDrawOverBorder && NOT_DRAW_OVER_BORDER_BOTTOM))
		{
			continue;
		}



		for (j=0; j<GlyphBMPWidth; j++)
		{
			if (((pt.x+j)>=0)&&((pt.x+j)<ttf_data->ImagePlaneData.ImagePlaneSize.x))
			{

				if (pt.x+j >= ttf_data->ImagePlaneData.ImagePlaneSize.x)	continue;
				if (pt.y+i >= ttf_data->ImagePlaneData.ImagePlaneSize.y)	continue;
				if (pt.x+j < 0)	continue;
				if (pt.y+i < 0)	continue;

				GlyphPixelValue = (BYTE)(pGlyphBMPBuffer[i*GlyphBMPWidth + j] * (ttf_data->ColorParameter_Glyph.Alpha));

				ColorR = (BYTE)(((int)ttf_data->ColorParameter_Glyph.Data.color_1 * (int)GlyphPixelValue) / 255);
				ColorG = (BYTE)(((int)ttf_data->ColorParameter_Glyph.Data.color_2 * (int)GlyphPixelValue) / 255);
				ColorB = (BYTE)(((int)ttf_data->ColorParameter_Glyph.Data.color_3 * (int)GlyphPixelValue) / 255);

				if(ttf_data->BlendingOn == 0)
				{
					ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3] = GlyphPixelValue;
					ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3 + 1] = GlyphPixelValue;
					ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3 + 2] = GlyphPixelValue;
				}
				else	//Blending
				{
					//BYTE k1 = ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3];
					unsigned char k2 = ttf_data->ImagePlaneData.ImagePlane[125106];
					BYTE k = ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3] * ((255-GlyphPixelValue)/255) + (255*GlyphPixelValue)/255;
					ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3] =     (ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3]     * (255-GlyphPixelValue))/255 + (255*ColorB)/255;
					ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3 + 1] = (ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3 + 1] * (255-GlyphPixelValue))/255 + (255*ColorG)/255;
					ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3 + 2] = (ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3 + 2] * (255-GlyphPixelValue))/255 + (255*ColorR)/255;
				}
			}
		}
	}
	return rt;
}


void Print_String_To_Image(stPrintTTFConfig* ttf_data, stPoint pt)
{
	BYTE a;
	int i, j;
	for (i=0; i<face->glyph->bitmap.rows; i++)
	{
		for (j=0; j<face->glyph->bitmap.width; j++)
		{
			if ((pt.x>=0)&&(pt.x<ttf_data->ImagePlaneData.ImagePlaneSize.x)&&(pt.y>=0)&&(pt.y<ttf_data->ImagePlaneData.ImagePlaneSize.y))
			{
				a = face->glyph->bitmap.buffer[i*face->glyph->bitmap.width + j];

				if(0)
				{
					ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3] = a;
					ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3 + 1] = a;
					ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3 + 2] = a;
				}
				else	//Blending
				{
					BYTE k = ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3] * ((255-a)/255) + (255*a)/255;
					ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3] = (ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3] * (255-a))/255 + (255*a)/255;
					ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3 + 1] = (ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3 + 1] * (255-a))/255 + (255*a)/255;
					ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3 + 2] = (ttf_data->ImagePlaneData.ImagePlane[((pt.y+i)*ttf_data->ImagePlaneData.ImagePlaneSize.x + (pt.x+j))*3 + 2] * (255-a))/255 + (255*a)/255;
				}
			}
		}

	}
}



#if 0
int Get_Char_BMP2(stPrintTTFConfig* ttf_data, unsigned int charcode, pGlyphContainer pGlyphCon)
{
	int rt = 0;
	FT_UInt       glyph_index = 0;
	//

	error = FT_Set_Char_Size(
					        face,    /* handle to face object           */
				            0,       /* char_width in 1/64th of points  */
							ttf_data->StringData.FontSize*64,   /* char_height in 1/64th of points */
							ttf_data->ImagePlaneData.DeviceResolution.x,     /* horizontal device resolution    */
							ttf_data->ImagePlaneData.DeviceResolution.y );   /* vertical device resolution      */



    glyph_index = FT_Get_Char_Index( face, charcode );
	pGlyphBMPData->CharCode = charcode;
	pGlyphBMPData->GlyphIndex = glyph_index;
    //error = FT_Load_Char( face, charcode, FT_LOAD_RENDER );
	error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);

	//----------------
#if 0
	  GlyphBMPData_From_FT_Glyph(ttf_data, pGlyphBMPData);
#else
    error = FT_Render_Glyph( face->glyph, ttf_data->Rendering );

  	pGlyphBMPData->FontSize					          = ttf_data->StringData.FontSize;
    pGlyphBMPData->GlyphBMPMetrics.Width              = face->glyph->bitmap.width;
    pGlyphBMPData->GlyphBMPMetrics.Height             = face->glyph->bitmap.rows;
    pGlyphBMPData->GlyphBMPMetrics.Pitch              = face->glyph->bitmap.pitch;
    pGlyphBMPData->GlyphBMPMetrics.AdvancePixel       = (face->glyph->advance.x >> 6);
    pGlyphBMPData->GlyphBMPMetrics.HoriBearingPixelX  = (face->glyph->metrics.horiBearingX >> 6);
    pGlyphBMPData->GlyphBMPMetrics.HoriBearingPixelY  = (face->glyph->metrics.horiBearingY >> 6);
    pGlyphBMPData->GlyphBMPMetrics.ShiftLeft          = face->glyph->bitmap_left;
    pGlyphBMPData->GlyphBMPMetrics.ShiftTop           = face->glyph->bitmap_top;

  	//pGlyphBMPData->Buffer = NEW_MEMORY(BYTE, pGlyphBMPData->GlyphBMPMetrics.Height*pGlyphBMPData->GlyphBMPMetrics.Pitch);
	//pGlyphBMPData->Buffer = NEW_MEMORY_ARRAY(BYTE, pGlyphBMPData->GlyphBMPMetrics.Height*pGlyphBMPData->GlyphBMPMetrics.Pitch)
	pGlyphBMPData->pBuffer = (BYTE*)MEMORY_CALLOC(pGlyphBMPData->GlyphBMPMetrics.Height*pGlyphBMPData->GlyphBMPMetrics.Pitch, sizeof(BYTE));
  	memcpy(pGlyphBMPData->pBuffer, face->glyph->bitmap.buffer, pGlyphBMPData->GlyphBMPMetrics.Height*pGlyphBMPData->GlyphBMPMetrics.Pitch);
#endif
  	//
  	pGlyphBMPData->GlyphBMPMetrics.BitPerPixel = -1;
  	if (ttf_data->Rendering == FT_RENDER_MODE_MONO)	pGlyphBMPData->GlyphBMPMetrics.BitPerPixel = 1;
  	if (ttf_data->Rendering == FT_RENDER_MODE_NORMAL)	pGlyphBMPData->GlyphBMPMetrics.BitPerPixel = 8;

  	return rt;
}

int Get_Char_BMP(stPrintTTFConfig* ttf_data, unsigned int charcode, pGlyphContainer pGlyphBMPData)
{

  FT_GlyphSlot  slot;
  FT_UInt       glyph_index = 0;
  FT_Matrix     matrix;                 /* transformation matrix */

  //---------------
  error = FT_Select_Charmap(face, ttf_data->Encoding);

  slot = face->glyph;

#ifdef BASIC_CODE

	error = FT_Set_Char_Size(
					        face,    /* handle to face object           */
				            0,       /* char_width in 1/64th of points  */
							ttf_data->StringData.FontSize*64,   /* char_height in 1/64th of points */
							ttf_data->ImagePlaneData.DeviceResolution.x,     /* horizontal device resolution    */
							ttf_data->ImagePlaneData.DeviceResolution.y );   /* vertical device resolution      */


  /* retrieve glyph index from character code */
  glyph_index = FT_Get_Char_Index( face, charcode );


  if ( use_kerning && previous && glyph_index )
  {
  	//FT_Kerning_Mode kerning = FT_KERNING_DEFAULT;
  	FT_Kerning_Mode kerning = FT_KERNING_UNFITTED;
  	//FT_Kerning_Mode kerning = FT_KERNING_UNSCALED;
    FT_Get_Kerning( face, previous, glyph_index, kerning, &delta );
	//pen_x += delta.x >> 6;
  }


// FONT_ROTATE_0
// FONT_ROTATE_90
// FONT_ROTATE_190
// FONT_ROTATE_270
#define FONT_ROTATE_0
//#define FONT_ROTATE_90

#ifdef FONT_ROTATE_0
  matrix.xx = (FT_Fixed)(0x10000L);
  matrix.xy = (FT_Fixed)(0.0);
  matrix.yx = (FT_Fixed)(0.0);
  matrix.yy = (FT_Fixed)(0x10000L);
#endif
#ifdef FONT_ROTATE_90
  matrix.xx = (FT_Fixed)(0L);
  matrix.xy = (FT_Fixed)(-0x10000L);
  matrix.yx = (FT_Fixed)(0x10000L);
  matrix.yy = (FT_Fixed)(0L);
#endif
  /*
  //angle         = ( 25.0 / 360 ) * 3.14159 * 2;      // use 25 degrees
  matrix.xx = (FT_Fixed)( cos( 0.436332 ) * 0x10000L );
  matrix.xy = (FT_Fixed)(-sin( 0.436332 ) * 0x10000L );
  matrix.yx = (FT_Fixed)( sin( 0.436332 ) * 0x10000L );
  matrix.yy = (FT_Fixed)( cos( 0.436332 ) * 0x10000L );
  */


  //pen.x = 20 * 64;
  //pen.y = 20 * 64;
  //FT_Set_Transform( face, &matrix, &pen );
  //FT_Set_Transform( face, &matrix, NULL );

  /* load glyph image into the slot (erase previous one) */
  error = FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT );
  if (error) return -1;

  /* convert to a bitmap */
  error = FT_Render_Glyph( face->glyph, ttf_data->Rendering );

  if (error) return -1;

#else
#ifdef REFINED_CODE
 	//mpDebugPrint("Get_Char_BMP() : REFINED_CODE");

  /* load glyph image into the slot (erase previous one) */
  error = FT_Load_Char( face, c, FT_LOAD_RENDER );
  if (error) return -1;
#endif
#endif

  pGlyphBMPData->GlyphBMPMetrics.Width              = slot->bitmap.width;
  pGlyphBMPData->GlyphBMPMetrics.Height             = slot->bitmap.rows;
  pGlyphBMPData->GlyphBMPMetrics.Pitch              = slot->bitmap.pitch;
  pGlyphBMPData->GlyphBMPMetrics.AdvancePixel       = (face->glyph->advance.x >> 6);
  pGlyphBMPData->GlyphBMPMetrics.HoriBearingPixelX  = (face->glyph->metrics.horiBearingX >> 6);
  pGlyphBMPData->GlyphBMPMetrics.HoriBearingPixelY  = (face->glyph->metrics.horiBearingY >> 6);
  pGlyphBMPData->GlyphBMPMetrics.ShiftLeft          = slot->bitmap_left;
  pGlyphBMPData->GlyphBMPMetrics.ShiftTop           = slot->bitmap_top;


  previous = glyph_index;

  //pGlyphBMPData->Buffer = NEW_MEMORY(BYTE, pGlyphBMPData->GlyphBMPMetrics.Pitch * pGlyphBMPData->GlyphBMPMetrics.Height)
  //pGlyphBMPData->Buffer = NEW_MEMORY_ARRAY(BYTE, pGlyphBMPData->GlyphBMPMetrics.Height*pGlyphBMPData->GlyphBMPMetrics.Pitch)
  pGlyphBMPData->pBuffer =  (BYTE*)MEMORY_CALLOC(pGlyphBMPData->GlyphBMPMetrics.Height*pGlyphBMPData->GlyphBMPMetrics.Pitch, sizeof(BYTE));
  memcpy(pGlyphBMPData->pBuffer, slot->bitmap.buffer, slot->bitmap.rows * slot->bitmap.pitch);

  return 0;
}
#endif

int DisplayDrawGlyphBMP(stPrintTTFConfig* ttf_data, stPoint pt, pGlyphContainer pGlyphCon)
{
	int rt = 0;
	//-----------

	switch(ttf_data->ImagePlaneData.ImagePlaneMode)
	{
		case DISPLAY_MODE_YYCBCR:
			if((ttf_data->FontRotation ==0) ||(ttf_data->FontRotation ==180))
					rt = DisplayDrawGlyphBMP_YYCBCR_H(ttf_data, pt , pGlyphCon);
			else if((ttf_data->FontRotation ==90) ||(ttf_data->FontRotation ==270))
					rt = DisplayDrawGlyphBMP_YYCBCR_V(ttf_data, pt , pGlyphCon);
			break;

		case DISPLAY_MODE_RGB:
			rt = DisplayDrawGlyphBMP_RGB(ttf_data, pt , pGlyphCon);
			break;

		default:
			rt = -1;
			break;
	}

	//-----------
	return rt;

}


int DisplayDrawRectangle_YYCBCR(stImagaPlaneParameter* ImagaPlaneParameter, stColorParameter* pColorParameter, stRectangle rec)
{
	int   HeadOdd, TailOdd;
	BYTE* pDispProc = NULL;	//Display memory start address to be processed
	int   i;
	stColorData ColorData;
	//

	Translate_Color(pColorParameter->Data, pColorParameter->Format, &ColorData, ImagaPlaneParameter->ImagePlaneMode);

	//
	HeadOdd = (rec.pt_1.x&0x1) ? 1:0;
	TailOdd = (rec.pt_2.x&0x1) ? 1:0;

	for (i = (((rec.pt_1.x&0x1) == 0)?(rec.pt_1.x):(rec.pt_1.x+1)); i<=(((rec.pt_2.x&0x1) == 0)?(rec.pt_2.x):(rec.pt_2.x-1)); i += 2)
	{
		if ((i<0) || (i>=ImagaPlaneParameter->ImagePlaneSize.x))
			continue;

		if ((rec.pt_1.y >= 0) && (rec.pt_1.y < ImagaPlaneParameter->ImagePlaneSize.y))
		{
			pDispProc = (BYTE*)ImagaPlaneParameter->ImagePlane + (ImagaPlaneParameter->ImagePlaneSize.x*rec.pt_1.y + i)*2;
			*(pDispProc  ) = ColorData.color_1;
			*(pDispProc+1) = ColorData.color_1;
			*(pDispProc+2) = ColorData.color_2;
			*(pDispProc+3) = ColorData.color_3;
		}
		if ((rec.pt_2.y >= 0) && (rec.pt_2.y < ImagaPlaneParameter->ImagePlaneSize.y))
		{
			pDispProc = (BYTE*)ImagaPlaneParameter->ImagePlane + (ImagaPlaneParameter->ImagePlaneSize.x*rec.pt_2.y + i)*2;
			*(pDispProc  ) = ColorData.color_1;
			*(pDispProc+1) = ColorData.color_1;
			*(pDispProc+2) = ColorData.color_2;
			*(pDispProc+3) = ColorData.color_3;
		}
	}

	for (i=rec.pt_1.y; i<=rec.pt_2.y; i++)
	{
		if ((rec.pt_1.x >= 0) && (rec.pt_1.x < ImagaPlaneParameter->ImagePlaneSize.x))
		{
			pDispProc = (BYTE*)ImagaPlaneParameter->ImagePlane + (ImagaPlaneParameter->ImagePlaneSize.x*i + ((rec.pt_1.x>>1)<<1))*2;
			((rec.pt_1.x&0x1) == 1)? (*(pDispProc+1) = ColorData.color_1) : (*(pDispProc  ) = ColorData.color_1);
			*(pDispProc+2) = ColorData.color_2;
			*(pDispProc+3) = ColorData.color_3;
		}
		if ((rec.pt_2.x >= 0) && (rec.pt_2.x < ImagaPlaneParameter->ImagePlaneSize.x))
		{
			pDispProc = (BYTE*)ImagaPlaneParameter->ImagePlane + (ImagaPlaneParameter->ImagePlaneSize.x*i + ((rec.pt_2.x>>1)<<1))*2;
			((rec.pt_2.x&0x1) == 1)? (*(pDispProc+1) = ColorData.color_1) : (*(pDispProc  ) = ColorData.color_1);
			*(pDispProc+2) = ColorData.color_2;
			*(pDispProc+3) = ColorData.color_3;
		}
	}


	return 0;
}


//int CheckInside(int x, int y)
int CheckInside(stPoint pt, stRectangle rec)
{
	NormalizeRectangle(&rec);
	if ((pt.x<rec.pt_1.x) || (pt.x>rec.pt_2.x))	return -1;
	if ((pt.y<rec.pt_1.y) || (pt.y>rec.pt_2.y))	return -1;

	return 0;
}


int DisplayDrawPixel_RGB(stImagaPlaneParameter* pImagaPlaneParameter, stColorParameter* pColorParameter, stPoint pt)
{
	stRectangle rec;
	rec.pt_1.x = rec.pt_1.y = 0;
	rec.pt_2.x = pImagaPlaneParameter->ImagePlaneSize.x-1;
	rec.pt_2.y = pImagaPlaneParameter->ImagePlaneSize.y-1;

	if (CheckInside(pt, rec)<0)	return -1;

	pImagaPlaneParameter->ImagePlane[(pt.y*pImagaPlaneParameter->ImagePlaneSize.x + pt.x)*3] = pColorParameter->Data.color_3;
	pImagaPlaneParameter->ImagePlane[(pt.y*pImagaPlaneParameter->ImagePlaneSize.x + pt.x)*3 + 1] = pColorParameter->Data.color_2;
	pImagaPlaneParameter->ImagePlane[(pt.y*pImagaPlaneParameter->ImagePlaneSize.x + pt.x)*3 + 2] = pColorParameter->Data.color_1;
	return 0;
}

int DisplayDrawPixel_YYCBCR(stImagaPlaneParameter* pImagaPlaneParameter, stColorParameter* pColorParameter, stPoint pt)
{
	int Pixel_Index = 0;
	int ByteY_Index = 0;
	int ByteCB_Index = 0;
	int ByteCR_Index = 0;
	stRectangle rec;
	stColorData ColorData;
	//
	Translate_Color(pColorParameter->Data, pColorParameter->Format, &ColorData, pImagaPlaneParameter->ImagePlaneMode);
	rec.pt_1.x = rec.pt_1.y = 0;
	rec.pt_2.x = pImagaPlaneParameter->ImagePlaneSize.x-1;
	rec.pt_2.y = pImagaPlaneParameter->ImagePlaneSize.y-1;
	if (CheckInside(pt, rec)<0)	return -1;
	//
	Pixel_Index = pt.x + pImagaPlaneParameter->ImagePlaneSize.x * pt.y ;
	ByteY_Index = ((Pixel_Index >> 1) <<2 ) + (Pixel_Index - ((Pixel_Index >> 1) <<1 ));
	ByteCB_Index = ((Pixel_Index >> 1) <<2 ) + 2;
	ByteCR_Index = ((Pixel_Index >> 1) <<2 ) + 3;
	//
	pImagaPlaneParameter->ImagePlane[ByteY_Index] = ColorData.color_1;
	pImagaPlaneParameter->ImagePlane[ByteCB_Index] = ColorData.color_2;
	pImagaPlaneParameter->ImagePlane[ByteCR_Index] = ColorData.color_3;


	return 0;
}

int DisplayDrawLine_YYCBCR(stImagaPlaneParameter* pImagaPlaneParameter, stColorParameter* pColorParameter, stPoint pt1, stPoint pt2)
{
	int i=0;
	int y3 = 0;
	int b=0;
	stPoint pt;
	stRectangle rec;
	//
	rec.pt_1.x = rec.pt_1.y = 0;
	rec.pt_2.x = pImagaPlaneParameter->ImagePlaneSize.x-1;
	rec.pt_2.y = pImagaPlaneParameter->ImagePlaneSize.y-1;
	//
	if ((pt1.x>pImagaPlaneParameter->ImagePlaneSize.x) && (pt2.x>pImagaPlaneParameter->ImagePlaneSize.x)) return -1;
	if ((pt1.y>pImagaPlaneParameter->ImagePlaneSize.y) && (pt2.y>pImagaPlaneParameter->ImagePlaneSize.y)) return -1;
	if ((pt1.x<0) && (pt2.x<0)) return -1;
	if ((pt1.y<0) && (pt2.y<0)) return -1;


	if (pt1.x == pt2.x)
	{
		for (i = ((pt2.y>pt1.y)?pt1.y:pt2.y); i<=((pt2.y>pt1.y)?pt2.y:pt1.y); i++)
		{
			pt.x = pt1.x; pt.y = i;
			if (CheckInside(pt, rec) < 0)
			{
				b = -1;
				continue;
			}
			DisplayDrawPixel_YYCBCR(pImagaPlaneParameter, pColorParameter, pt);
		}
	}
	else
	{
		for (i = pt1.x; i<=pt2.x; i++)
		{
			y3 = ((pt2.y-pt1.y)*(i-pt1.x)) / (pt2.x-pt1.x) + pt1.y;
			pt.x = i; pt.y = y3;
			if (CheckInside(pt, rec) < 0)
			{
				b = -1;
				continue;
			}

			DisplayDrawPixel_YYCBCR(pImagaPlaneParameter, pColorParameter, pt);
		}
	}


	return b;
}

int DisplayDrawLine_RGB(stImagaPlaneParameter* pImagaPlaneParameter, stColorParameter* pColorParameter, stPoint pt1, stPoint pt2)
{
	int i=0;
	int y3 = 0;
	int b=0;
	stPoint pt;
	stRectangle rec;
	//
	rec.pt_1.x = rec.pt_1.y = 0;
	rec.pt_2.x = pImagaPlaneParameter->ImagePlaneSize.x-1;
	rec.pt_2.y = pImagaPlaneParameter->ImagePlaneSize.y-1;
	//
	if ((pt1.x>pImagaPlaneParameter->ImagePlaneSize.x) && (pt2.x>pImagaPlaneParameter->ImagePlaneSize.x)) return -1;
	if ((pt1.y>pImagaPlaneParameter->ImagePlaneSize.y) && (pt2.y>pImagaPlaneParameter->ImagePlaneSize.y)) return -1;
	if ((pt1.x<0) && (pt2.x<0)) return -1;
	if ((pt1.y<0) && (pt2.y<0)) return -1;


	if (pt1.x == pt2.x)
	{
		for (i = ((pt2.y>pt1.y)?pt1.y:pt2.y); i<=((pt2.y>pt1.y)?pt2.y:pt1.y); i++)
		{
			pt.x = pt1.x;
			pt.y = i;
			if (CheckInside(pt, rec) < 0)
			{
				b = -1;
				continue;
			}
			DisplayDrawPixel_RGB(pImagaPlaneParameter, pColorParameter, pt);
		}
	}
	else
	{
		for (i = pt1.x; i<=pt2.x; i++)
		{
			stRectangle rec;
			rec.pt_1.x = rec.pt_1.y = 0;
			rec.pt_2.x = pImagaPlaneParameter->ImagePlaneSize.x-1;
			rec.pt_2.y = pImagaPlaneParameter->ImagePlaneSize.y-1;
			y3 = ((pt2.y-pt1.y)*(i-pt1.x)) / (pt2.x-pt1.x) + pt1.y;
			pt.x = i;
			pt.y = y3;
			if (CheckInside(pt, rec) < 0)
			{
				b = -1;
				continue;
			}

			DisplayDrawPixel_RGB(pImagaPlaneParameter, pColorParameter, pt);
		}
	}


	return b;
}

int DisplayDrawLine(stImagaPlaneParameter* ImagaPlaneParameter, stColorParameter* pColorParameter, stPoint pt1, stPoint pt2)
{
	int rt = 0;
	//-----------

	switch(ImagaPlaneParameter->ImagePlaneMode)
	{
		case DISPLAY_MODE_YYCBCR:
			rt = DisplayDrawLine_YYCBCR(ImagaPlaneParameter, pColorParameter, pt1, pt2);
			break;
		case DISPLAY_MODE_RGB:
			rt = DisplayDrawLine_RGB(ImagaPlaneParameter, pColorParameter, pt1, pt2);
			break;
		default:
			rt = -1;
			break;

	}

	//-----------
	return rt;

}

int DisplayDrawRectangle_RGB(stImagaPlaneParameter* ImagaPlaneParameter, stColorParameter* pColorParameter, stRectangle rec)
{
	int rt = 0;
	stPoint pt1, pt2;
	stColorData ColorData;
	Translate_Color(pColorParameter->Data, pColorParameter->Format, &ColorData, ImagaPlaneParameter->ImagePlaneMode);

	/*
	DisplayDrawLine_RGB(x1, y1, x2, y1, ttf_config->ColorParameter_Rectangle.Data.color_1, ttf_config->ColorParameter_Rectangle.Data.color_2, ttf_config->ColorParameter_Rectangle.Data.color_3);
	DisplayDrawLine_RGB(x1, y2, x2, y2, ttf_config->ColorParameter_Rectangle.Data.color_1, ttf_config->ColorParameter_Rectangle.Data.color_2, ttf_config->ColorParameter_Rectangle.Data.color_3);
	DisplayDrawLine_RGB(x1, y1, x1, y2, ttf_config->ColorParameter_Rectangle.Data.color_1, ttf_config->ColorParameter_Rectangle.Data.color_2, ttf_config->ColorParameter_Rectangle.Data.color_3);
	DisplayDrawLine_RGB(x2, y1, x2, y2, ttf_config->ColorParameter_Rectangle.Data.color_1, ttf_config->ColorParameter_Rectangle.Data.color_2, ttf_config->ColorParameter_Rectangle.Data.color_3);
	*/
	pt1.x = rec.pt_1.x; pt1.y = rec.pt_1.y; pt2.x = rec.pt_2.x; pt2.y = rec.pt_1.y;
	DisplayDrawLine_RGB(ImagaPlaneParameter, pColorParameter, pt1, pt2);
	pt1.x = rec.pt_1.x; pt1.y = rec.pt_2.y; pt2.x = rec.pt_2.x; pt2.y = rec.pt_2.y;
	DisplayDrawLine_RGB(ImagaPlaneParameter, pColorParameter, pt1, pt2);
	pt1.x = rec.pt_1.x; pt1.y = rec.pt_1.y; pt2.x = rec.pt_1.x; pt2.y = rec.pt_2.y;
	DisplayDrawLine_RGB(ImagaPlaneParameter, pColorParameter, pt1, pt2);
	pt1.x = rec.pt_2.x; pt1.y = rec.pt_1.y; pt2.x = rec.pt_2.x; pt2.y = rec.pt_2.y;
	DisplayDrawLine_RGB(ImagaPlaneParameter, pColorParameter, pt1, pt2);

	//DisplayDrawLine_RGB(ttf_config->NextPos.x, ttf_config->NextPos.y-20, ttf_config->NextPos.x, ttf_config->NextPos.y+20, 0, 255, 0);

	return 0;
}

int  Swap_int(int* a, int* b)
{
	int rt = 0;
	int temp = 0;
	temp = *a;
	*a = *b;
	*b = temp;

	return rt;
}

int  NormalizeRectangle(stRectangle* rec)
{
	int rt = 0;
	//
	if (rec->pt_1.x > rec->pt_2.x)
		Swap_int(&rec->pt_1.x, &rec->pt_2.x);
	if (rec->pt_1.y > rec->pt_2.y)
		Swap_int(&rec->pt_1.y, &rec->pt_2.y);
	//
	return rt;
}

int DisplayDrawRectangle(stImagaPlaneParameter* ImagaPlaneParameter, stColorParameter* pColorParameter, stRectangle rec)
{
	int rt = 0;

	NormalizeRectangle(&rec);

	switch(ImagaPlaneParameter->ImagePlaneMode)
	{
		case DISPLAY_MODE_YYCBCR:
			rt = DisplayDrawRectangle_YYCBCR(ImagaPlaneParameter, pColorParameter, rec);
			break;
		case DISPLAY_MODE_RGB:
			rt = DisplayDrawRectangle_RGB(ImagaPlaneParameter, pColorParameter, rec);
			break;
		default:
			rt = -1;
			break;

	}
	//-----------
	return 0;
}


#if 0
int ClearGlyph_List()
{
	unsigned int i;
	for (i=0; i<num_glyphs; i++)
	{
		FT_Done_Glyph(Glyph_List[i]);
	}
	RELEASE_MEMORY(Glyph_List);
	RELEASE_MEMORY(Glyph_Pos_List);

	return 0;
}

int GetStringGlyphList(stPrintTTFConfig* ttf_data)
{

	int rt, i;
	FT_GlyphSlot  slot;
	FT_UInt       glyph_index = 0;
	FT_Vector     pen;                    /* untransformed origin  */
	double        angle;
	FT_UInt       previous_glyph_index;
	FT_Vector     delta;
	//-------------------------

	Glyph_List = NEW_MEMORY(FT_Glyph, ttf_data->StringData.StrLength);
	Glyph_Pos_List = NEW_MEMORY(FT_Vector, ttf_data->StringData.StrLength);

	pen.x = ttf_data->PrintPos.x;
	pen.y = ttf_data->PrintPos.y;
	num_glyphs = 0;
	previous_glyph_index = 0;
	String_bbox.xMax = String_bbox.yMax = 0;
	String_bbox.xMin = ttf_data->ImagePlaneData.ImagePlaneSize.x;
	String_bbox.yMin = ttf_data->ImagePlaneData.ImagePlaneSize.y;
	ttf_data->OutputData.StrHBearingYPixel = 0;

	delta.x = delta.y = 0;
	//---------

    use_kerning = FT_HAS_KERNING( face );

	error = FT_Select_Charmap(face, ttf_data->Encoding);

	slot = face->glyph;



	error = FT_Set_Char_Size(
					        face,    /* handle to face object           */
				            0,       /* char_width in 1/64th of points  */
							ttf_data->StringData.FontSize*64,   /* char_height in 1/64th of points */
				            300,     /* horizontal device resolution    */
							300 );   /* vertical device resolution      */

	for (i=0; i<ttf_data->StringData.StrLength; i++)
	{
		glyph_index = FT_Get_Char_Index(face, ttf_data->StringData.Str[i]);

		/*
		if (use_kerning && previous_glyph_index && glyph_index)
		{
			FT_Vector delta;
			FT_Get_Kerning(face, previous_glyph_index, glyph_index, FT_KERNING_DEFAULT, &delta);
			pen.x += delta.x >> 6;
			pen.y += delta.y >> 6;
		}
		*/
		Glyph_Pos_List[num_glyphs].x = pen.x;
		Glyph_Pos_List[num_glyphs].y = pen.y;

		error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
		if (error)	continue;

		error = FT_Get_Glyph(face->glyph, &Glyph_List[num_glyphs]);
		if (error)	continue;

		Glyph_Pos_List[num_glyphs].x +=  face->glyph->metrics.horiBearingX >> 6;
		Glyph_Pos_List[num_glyphs].y -=  face->glyph->metrics.horiBearingY >> 6;

		previous_glyph_index = glyph_index;
		num_glyphs++;

		pen.x += (slot->advance.x >> 6) + (delta.x >> 6);


		//Get bbox of the string
		if (i==0)
		{
			ttf_data->OutputData.StrHBearingXPixelFirstGlyph = (face->glyph->metrics.horiBearingX >> 6);
			String_bbox.xMin = Glyph_Pos_List[i].x;
		}
		if (Glyph_Pos_List[i].y < String_bbox.yMin)	String_bbox.yMin = Glyph_Pos_List[i].y;
		if (i == (ttf_data->StringData.StrLength - 1))
			String_bbox.xMax = Glyph_Pos_List[num_glyphs-1].x + (face->glyph->metrics.width >> 6);
		if ((Glyph_Pos_List[i].y + (face->glyph->metrics.height >> 6)) > String_bbox.yMax)
			String_bbox.yMax = (Glyph_Pos_List[i].y + (face->glyph->metrics.height >> 6));
		if ((face->glyph->metrics.horiBearingY >> 6) > ttf_data->OutputData.StrHBearingYPixel)
			ttf_data->OutputData.StrHBearingYPixel = (face->glyph->metrics.horiBearingY >> 6);

		ttf_data->OutputData.NextPos.x = Glyph_Pos_List[i].x + (face->glyph->advance.x >> 6);
		ttf_data->OutputData.NextPos.y = Glyph_Pos_List[i].y + (face->glyph->metrics.horiBearingY >> 6) + (face->glyph->advance.y >> 6);
	}

	//DisplayDrawLine(ttf_config->PrintPos.x, ttf_config->PrintPos.y-20, ttf_config->PrintPos.x, ttf_config->PrintPos.y+20, 0, 0, 255);

	//DisplayDrawLine(ttf_config->PrintPos.x + pen.x, ttf_config->PrintPos.y-20, ttf_config->PrintPos.x + pen.x, ttf_config->PrintPos.y+20, 255, 0, 0);

	//---------------


	return 0;
}


int RenderPrintStringGlyphList(stPrintTTFConfig* ttf_data)
{
	unsigned int n;
	int rt = 0;
	stPoint pt;
	/* compute start pen position in 26.6 cartesian pixels */
	for ( n = 0; n < num_glyphs; n++ )
	{
		FT_Glyph   image;
		FT_Vector  pen;
		stGlyphBMPData GlyphBMPData;

		Init_stGlyphBMPData(&GlyphBMPData);


		image = Glyph_List[n];

		error = FT_Glyph_To_Bitmap( &image, ttf_data->Rendering, &pen, 0 );

		FT_BitmapGlyph bit = (FT_BitmapGlyph)image;

		GlyphBMPData.GlyphBMPMetrics.Width = bit->bitmap.width;
		GlyphBMPData.GlyphBMPMetrics.Height = bit->bitmap.rows;
		GlyphBMPData.GlyphBMPMetrics.Pitch = bit->bitmap.pitch;

		GlyphBMPData.Buffer = NEW_MEMORY(BYTE, GlyphBMPData.GlyphBMPMetrics.Height*GlyphBMPData.GlyphBMPMetrics.Pitch);
		memcpy(GlyphBMPData.Buffer, bit->bitmap.buffer, GlyphBMPData.GlyphBMPMetrics.Height*GlyphBMPData.GlyphBMPMetrics.Pitch);


		pt.x = Glyph_Pos_List[n].x; pt.y = Glyph_Pos_List[n].y;
		DisplayDrawGlyphBMP(ttf_data, pt, &GlyphBMPData);

		FT_Done_Glyph(image);
		Release_stGlyphBMPData(&GlyphBMPData);
	}



	return rt;
}


#endif

int Init_stPoint(stPoint* pPoint)
{
	int rt = 0;
	pPoint->x = 0;
	pPoint->y = 0;
	return rt;
}


int GetNextDelimiterIndex(stStrParameter* StringData, int index_now, int* NextDelimiterIndex)
{
	int rt = 0;
	int i, j;

	//
	*NextDelimiterIndex = -1;

	if ((index_now < 0) || (index_now >= StringData->StrLength))
		return -1;

	if(StringData->Str[index_now]>0xff)
		{
			*NextDelimiterIndex = index_now;
			return rt;
		}

	//
	for (i = index_now; i<StringData->StrLength; i++)
	{
		for (j = 0; j<StringData->DelimiterListLength; j++)
		{
			if ((StringData->Str[i] != StringData->DelimiterList[j])&&(StringData->Str[i]<0xff))
				continue;

			*NextDelimiterIndex = i;
			return rt;
		}
	}
	//
	rt = -1;
	return rt;
}

int GetNextSubStringLength(stPrintTTFConfig* ttf_data, int index_now, int* NextSubStringLength)
{
	int rt = 0;

	//
	switch(ttf_data->GlyphSetMode)
	{
		case (GLYPH_SET_MODE_GLYPH):
			{
				if ((index_now <0) || (index_now >= ttf_data->StringData.StrLength))
				{
					*NextSubStringLength = -1;
					rt = -1;
					break;
				}
				*NextSubStringLength = 1;
			}
			break;
		case (GLYPH_SET_MODE_DELIMITER)://Auto determine  DELIMITER and GLYPH
			{
				int NextDelimiter;
				rt = GetNextDelimiterIndex(&ttf_data->StringData, index_now, &NextDelimiter);
				*NextSubStringLength = (rt == 0)?(NextDelimiter - index_now):(ttf_data->StringData.Str_Index_Stop - index_now+1);
				if (NextDelimiter == index_now) *NextSubStringLength = 1;
				return rt;
			}
			break;
		case (GLYPH_SET_MODE_WHOLESTRING):
			{
				*NextSubStringLength = (ttf_data->StringData.Str_Index_Stop - ttf_data->StringData.Str_Index_Start + 1);
				return rt;
			}
			break;
		default:
			{
			}
			break;
	}
	//
	return rt;

}

int PrintString_ReverseOrder(stPrintTTFConfig* pttf_data)
{
	int rt = 0;
	int index = pttf_data->StringData.Str_Index_Start;
	stPoint next_pos = pttf_data->PrintCoordinate.PrintPos;
	stPoint pt0, pt1;
	stGlyphContainerFeature feature;
	pGlyphContainer pGlyphCon_Temp = NULL;

	int BearingX, BearingY, Advance, Height, Width;
	int vBearingX, vBearingY, vAdvance;

	//
	feature.FontSize = pttf_data->StringData.FontSize;
	feature.pFace = face;
	feature.RenderMode = pttf_data->Rendering;
	feature.Resolution.x = pttf_data->ImagePlaneData.DeviceResolution.x;
	feature.Resolution.y = pttf_data->ImagePlaneData.DeviceResolution.y;
	feature.FontRotation= pttf_data->FontRotation;
	//
	pt0.x = pttf_data->PrintCoordinate.PrintPos.x;
	pt0.y = pttf_data->PrintCoordinate.PrintPos.y;

	//Process "Glyph Set" one by one
	for (index = pttf_data->StringData.Str_Index_Start; index <= pttf_data->StringData.Str_Index_Stop; index++)
	{
		MP_DEBUG("index = %d start = %d, stop = %d",index,pttf_data->StringData.Str_Index_Start,pttf_data->StringData.Str_Index_Stop);

		BYTE* pGlyphBMPBuffer = NULL;
		//
		feature.CharCode = pttf_data->StringData.Str[index];
		if (pCM->pCacheList != NULL)
		{
			int aaa = 1;
		}

		pGlyphCon_Temp = CacheManager_RequestUnitGlyphBMP(pCM, &feature, WHILE_CACHE_MISS_DO_ADD);

		Width = (pGlyphCon_Temp->Metrics.width>>6);
		Height = (pGlyphCon_Temp->Metrics.height>>6);
		BearingX = (pGlyphCon_Temp->Metrics.horiBearingX>>6);
		BearingY = (pGlyphCon_Temp->Metrics.horiBearingY>>6);
		Advance = (pGlyphCon_Temp->Metrics.horiAdvance>>6);
		vBearingX = (pGlyphCon_Temp->Metrics.vertBearingX>>6);
		vBearingY = (pGlyphCon_Temp->Metrics.vertBearingY>>6);
		vAdvance = (pGlyphCon_Temp->Metrics.vertAdvance>>6);


		if((pttf_data->FontRotation == 0) || (pttf_data->FontRotation == 180))
		{
		//
		pt1.y = pt0.y - BearingY;
		pt1.x = pt0.x + BearingX;

		//Draw glyph on the image plane
		DisplayDrawGlyphBMP(pttf_data, pt1, pGlyphCon_Temp);//<=============

		if(Width != 0)
			pt0.x += BearingX+Width;
		else
			pt0.x += Advance>>1;
		//Get next pos
		next_pos.x = pt0.x;
		next_pos.y = pt0.y;
		}
		else if ((pttf_data->FontRotation == 90) || (pttf_data->FontRotation == 270))
		{
	#if 	1 //fix
		pt1.y = (pttf_data->PrintCoordinate.PrintRegion.pt_2.y-pt0.y) - Width-(vAdvance-Width)/2;
		pt1.x = pt0.x +vBearingY;
	#else
		//pt1.y = 400 +56 - ( pt0.y - Width-vBearingX);
		//pt1.y = pttf_data->PrintCoordinate.PrintRegion.pt_2.y-( pt0.y - Width-vBearingX);
		pt1.y = pttf_data->PrintCoordinate.PrintRegion.pt_2.y-( pt0.y + Width+vBearingX);
		pt1.x = pt0.x + vBearingY;
	#endif	

		MP_DEBUG("(pt1.x ,pt1,y)= (%d,%d)  pt0.y =%d,  Width =%d ,  vBearingX =%d",pt1.x,pt1.y,pt0.y,Width, vBearingX);
		//Draw glyph on the image plane
		DisplayDrawGlyphBMP(pttf_data, pt1, pGlyphCon_Temp);//<=============

		#if 1

		if(Width != 0)
			pt0.x +=  vBearingY+Height;
		else
			pt0.x +=  vAdvance>>1;

		#else
		
		//pt0.x += Advance;
		if(Width != 0)
			pt0.x += BearingX+Width;
		else
			pt0.x += Advance >>1;
		#endif
			
		//Get next pos
		next_pos.x = pt0.x;
		next_pos.y = pt0.y;
		}
		//
	}
	pttf_data->OutputData.NextPos = next_pos;

	return rt;
}

int PrintString_Immediately(stPrintTTFConfig* pttf_data)
{
	int rt = 0;
	int index = pttf_data->StringData.Str_Index_Start;
	stPoint next_pos = pttf_data->PrintCoordinate.PrintPos;
	stPoint pt0, pt1;
	//static stGlyphContainerFeature feature;
	stGlyphContainerFeature feature;
	pGlyphContainer pGlyphCon_Temp = NULL;

	int BearingX, BearingY, Advance, Height, Width;
	int vBearingX, vBearingY, vAdvance;

	//
	memset(&feature,0,sizeof(stGlyphContainerFeature)); //cj add
	feature.FontSize = pttf_data->StringData.FontSize;
	feature.pFace = face;
	feature.RenderMode = pttf_data->Rendering;
	feature.Resolution.x = pttf_data->ImagePlaneData.DeviceResolution.x;
	feature.Resolution.y = pttf_data->ImagePlaneData.DeviceResolution.y;
	feature.FontRotation= pttf_data->FontRotation;
	//
	pt0.x = pttf_data->PrintCoordinate.PrintPos.x;
	pt0.y = pttf_data->PrintCoordinate.PrintPos.y;


	//Process "Glyph Set" one by one
	for (index = pttf_data->StringData.Str_Index_Start; index <= pttf_data->StringData.Str_Index_Stop; index++)
	{

		BYTE* pGlyphBMPBuffer = NULL;
		//
		feature.CharCode = pttf_data->StringData.Str[index];
#if 1
		pGlyphCon_Temp = CacheManager_RequestUnitGlyphBMP(pCM, &feature, WHILE_CACHE_MISS_DO_ADD);
#endif
		Width = (pGlyphCon_Temp->Metrics.width>>6);
		Height = (pGlyphCon_Temp->Metrics.height>>6);
		BearingX = (pGlyphCon_Temp->Metrics.horiBearingX>>6);
		BearingY = (pGlyphCon_Temp->Metrics.horiBearingY>>6);
		Advance = (pGlyphCon_Temp->Metrics.horiAdvance>>6);
		vBearingX = (pGlyphCon_Temp->Metrics.vertBearingX>>6);
		vBearingY = (pGlyphCon_Temp->Metrics.vertBearingY>>6);
		vAdvance = (pGlyphCon_Temp->Metrics.vertAdvance>>6);

		if((pttf_data->FontRotation == 0) || (pttf_data->FontRotation == 180))
		{

		//
		pt1.y = pt0.y - BearingY;
		pt1.x = pt0.x + BearingX;

		//Draw glyph on the image plane
		DisplayDrawGlyphBMP(pttf_data, pt1, pGlyphCon_Temp);//<=============

		if(Width != 0)
			pt0.x += BearingX+Width;
		else
			pt0.x += Advance>>1;

		//Get next pos
		next_pos.x = pt0.x;
		next_pos.y = pt0.y;

		}
		else if ((pttf_data->FontRotation == 90) || (pttf_data->FontRotation == 270))
		{

		//mpDebugPrint(" PPP CharCode =%x W=%d h=%d Bx=%d By=%d Ad=%d vBx=%d vBy=%d vAdvance=%d",feature.CharCode,Width,Height,BearingX,BearingY,Advance,vBearingX,vBearingY,vAdvance);
		pt1.y = pt0.y - Width-(vAdvance-Width)/2;
		pt1.x = pt0.x +vBearingY;

		//Draw glyph on the image plane
		DisplayDrawGlyphBMP(pttf_data, pt1, pGlyphCon_Temp);//<=============

		if(Width != 0)
			pt0.x +=  vBearingY+Height;
		else
			pt0.x +=  vAdvance>>1;

		//Get next pos
		next_pos.x = pt0.x;
		next_pos.y = pt0.y;
		}
		//
	}

	pttf_data->OutputData.NextPos = next_pos;

	return rt;
}

int FE_LayoutString(stPrintTTFConfig* ttf_data)
{
	int result = 0;
	stPrintTTFConfig* pConfigTemp = stPrintTTFConfig_Copy(ttf_data);
	//
	stPoint pos;
	int index = pConfigTemp->StringData.Str_Index_Start;
	int index2 = 0;
	int continuous_not_cropped = 1;
	int Y_Bottom = 0, Y_Bottom_temp, LastGlyphIndexAtFirstCroppedLine_Candidate, LastChangeLineGlyphIndex_Old = -1;
	stPoint original_pos, print_pos, next_pos;
	stRectangle Print_Border = pConfigTemp->PrintCoordinate.PrintRegion;
	//stGlyphContainerFeature feature;
	//pGlyphContainer pGlyphCon_Temp = NULL;
	MEASURE_MODE Measure_Mode_temp = pConfigTemp->MeasureMode;
	int bOutOfBorder = 0;
	int TempBearingY=0;
	int hhh=0,www=0,Debug_Output_Position=0; //test bug


	//Normalize config
	stPrintTTFConfig_Normalize(pConfigTemp);
	//Reset output data
	stPrintTTFConfig_ResetOutputData(pConfigTemp);
	//
	pConfigTemp->MeasureMode = MEASURE_MODE_STRING;
	original_pos = pConfigTemp->PrintCoordinate.PrintPos;
	print_pos = pConfigTemp->PrintCoordinate.PrintPos;
	next_pos = pConfigTemp->PrintCoordinate.PrintPos;
	Print_Border = pConfigTemp->PrintCoordinate.PrintRegion;
	pConfigTemp->OutputData.Count = 0;
	//
	pos.x = pConfigTemp->PrintCoordinate.PrintPos.x;
	pos.y = pConfigTemp->PrintCoordinate.PrintPos.y;
	//
	print_pos = pConfigTemp->PrintCoordinate.PrintPos;
	//
	//
	#if 0
	feature.FontSize = pConfigTemp->StringData.FontSize;
	feature.pFace = face;
	feature.RenderMode = pConfigTemp->Rendering;
	feature.Resolution.x = pConfigTemp->ImagePlaneData.DeviceResolution.x;
	feature.Resolution.y = pConfigTemp->ImagePlaneData.DeviceResolution.y;
	#endif

	//////////////////////////////////////////////////////////////////////////////////////
	//Process "Glyph Set" one by one
	while(index <= pConfigTemp->StringData.Str_Index_Stop)
	{
		int NextSubStringLength;
		stPoint pt0 = original_pos;
		int StringStartTemp1 = pConfigTemp->StringData.Str_Index_Start;
		int StringStopTemp2 = pConfigTemp->StringData.Str_Index_Stop;
		//
		GetNextSubStringLength(pConfigTemp, index, &NextSubStringLength);
		pConfigTemp->StringData.Str_Index_Start = index;
		pConfigTemp->StringData.Str_Index_Stop = index + NextSubStringLength - 1;
		TaskYield();

////////////////////////////// chih Kuo add
		if(pConfigTemp->StringData.Str[index]==0xa) //ignore 0x0a and jump line
		{
			print_pos.x =pConfigTemp->PrintCoordinate.PrintRegion.pt_1.x;
			print_pos.y += pConfigTemp->PrintCoordinate.LineHeightPixel;
			//
			pConfigTemp->PrintCoordinate.PrintPos.x = print_pos.x;
			pConfigTemp->PrintCoordinate.PrintPos.y = print_pos.y;
			pConfigTemp->OutputData.NextPos.x = pConfigTemp->PrintCoordinate.PrintPos.x;
			pConfigTemp->OutputData.NextPos.y = pConfigTemp->PrintCoordinate.PrintPos.y;

			//Record the index the of last glyph at the last line in the region
			if ((continuous_not_cropped == 0)&&(pConfigTemp->OutputData.LastGlyphIndexAtFirstCroppedLine == -1))
			{
				pConfigTemp->OutputData.LastGlyphIndexAtFirstCroppedLine = LastGlyphIndexAtFirstCroppedLine_Candidate;
			}

			//Out of Rectangle?
			if (print_pos.y > Print_Border.pt_2.y)
			{
				pConfigTemp->OutputData.LastChangeLineGlyphIndex = index + NextSubStringLength - 1;
				bOutOfBorder = 1;
				break;
			}

			if (bOutOfBorder == 0)
			{
				pConfigTemp->OutputData.Count += NextSubStringLength;
			}

			//Record the index the of last glyph at the last not-cropped line
			if (continuous_not_cropped == 1)
			{
				LastChangeLineGlyphIndex_Old = pConfigTemp->OutputData.LastChangeLineGlyphIndex;
				pConfigTemp->OutputData.LastChangeLineGlyphIndex = index + NextSubStringLength - 1;
			}

			//////////////////
			index += NextSubStringLength;
			pConfigTemp->StringData.Str_Index_Start = StringStartTemp1;
			pConfigTemp->StringData.Str_Index_Stop = StringStopTemp2;
			print_pos.x = pConfigTemp->OutputData.NextPos.x;
			print_pos.y = pConfigTemp->OutputData.NextPos.y;
			pConfigTemp->PrintCoordinate.PrintPos.x = pConfigTemp->OutputData.NextPos.x;
			pConfigTemp->PrintCoordinate.PrintPos.y = pConfigTemp->OutputData.NextPos.y;

		}
		else  // fix
		{
//////////////////////////////

			MeasureString(pConfigTemp);

			if(pConfigTemp->OutputData.StrHBearingYPixel>TempBearingY)//fix
				TempBearingY=pConfigTemp->OutputData.StrHBearingYPixel;

		//	result = CacheManager_VerifyCacheMemory(pCM);

			//Change line or not? Shift the rectangle of the string while changing line
			if ((print_pos.x + pConfigTemp->OutputData.StrHBearingXPixelFirstGlyph + pConfigTemp->OutputData.RectangleSize.x-1) >= Print_Border.pt_2.x)
			{
				print_pos.x = pConfigTemp->PrintCoordinate.SpecifiedChangeLineX;
				print_pos.y += pConfigTemp->PrintCoordinate.LineHeightPixel;
				//print_pos.y += pConfigTemp->OutputData.StrHBearingYPixel;
				//
				pConfigTemp->PrintCoordinate.PrintPos.x = print_pos.x;
				pConfigTemp->PrintCoordinate.PrintPos.y = print_pos.y;
				pConfigTemp->OutputData.NextPos.x = pConfigTemp->PrintCoordinate.PrintPos.x + pConfigTemp->OutputData.RectangleSize.x;
				pConfigTemp->OutputData.NextPos.y = pConfigTemp->PrintCoordinate.PrintPos.y;

				//Record the index the of last glyph at the last line in the region
				if ((continuous_not_cropped == 0)&&(pConfigTemp->OutputData.LastGlyphIndexAtFirstCroppedLine == -1))
				{
					pConfigTemp->OutputData.LastGlyphIndexAtFirstCroppedLine = LastGlyphIndexAtFirstCroppedLine_Candidate;
				}
				//Out of Rectangle?
				if (print_pos.y > Print_Border.pt_2.y)
				{
					bOutOfBorder = 1;
					break;
				}

			}

			if (pConfigTemp->PrintStringOn == 1) //test bug
				{
					www=pConfigTemp->OutputData.NextPos.x;
					hhh=pConfigTemp->OutputData.NextPos.y;
				}

			//Print out the bitmaps of the glyph set
			if (pConfigTemp->PrintStringOn == 1)
			{
				if(pConfigTemp->RotationFontOrderDirection == 1)
					PrintString_ReverseOrder(pConfigTemp);
				else
					PrintString(pConfigTemp);
			}

			if (bOutOfBorder == 0)
			{
				pConfigTemp->OutputData.Count += NextSubStringLength;
			}

			//Record the index the of last glyph at the last not-cropped line
			if (continuous_not_cropped == 1)
			{
				LastChangeLineGlyphIndex_Old = pConfigTemp->OutputData.LastChangeLineGlyphIndex;
				pConfigTemp->OutputData.LastChangeLineGlyphIndex = index + NextSubStringLength - 1;
			}
			//Cropped?
			Y_Bottom_temp = pConfigTemp->PrintCoordinate.PrintPos.y - pConfigTemp->OutputData.StrHBearingYPixel + pConfigTemp->OutputData.RectangleSize.y;
			Y_Bottom = (Y_Bottom_temp>Y_Bottom)?Y_Bottom_temp:Y_Bottom;
			if (Y_Bottom > Print_Border.pt_2.y)
			{
				LastGlyphIndexAtFirstCroppedLine_Candidate = index + NextSubStringLength - 1;

				if (continuous_not_cropped>0)
				{
					continuous_not_cropped = 0;
					pConfigTemp->OutputData.LastChangeLineGlyphIndex = LastChangeLineGlyphIndex_Old;
				}
			}

			//////////////////
			index += NextSubStringLength;
			pConfigTemp->StringData.Str_Index_Start = StringStartTemp1;
			pConfigTemp->StringData.Str_Index_Stop = StringStopTemp2;
			print_pos.x = pConfigTemp->OutputData.NextPos.x;
			print_pos.y = pConfigTemp->OutputData.NextPos.y;
			pConfigTemp->PrintCoordinate.PrintPos.x = pConfigTemp->OutputData.NextPos.x;
			pConfigTemp->PrintCoordinate.PrintPos.y = pConfigTemp->OutputData.NextPos.y;

			if ((pConfigTemp->PrintStringOn == 1)&&(Debug_Output_Position==1)) //test bug
			{
				if(www!=pConfigTemp->OutputData.NextPos.x)
					mpDebugPrint("==>> FE_LayoutString ERROR  END XX  NextPos.x=%d  www=%d  index=%d",pConfigTemp->OutputData.NextPos.x,www,index);
				if(hhh!=pConfigTemp->OutputData.NextPos.y)
					mpDebugPrint("==>> FE_LayoutString  ERROR  END  YY  hhh=%d  ->OutputData.NextPos.y=%d  index=%d",hhh,pConfigTemp->OutputData.NextPos.y,index);
			}

		}
	}

	//Draw print region
	if (pConfigTemp->PrintRegionOn > 0)
	{
		DisplayDrawRectangle(&(pConfigTemp->ImagePlaneData), &(pConfigTemp->ColorParameter_PrintRegion), pConfigTemp->PrintCoordinate.PrintRegion);
	}

	pConfigTemp->OutputData.StrHBearingYPixel=TempBearingY;

	//////////////////////////////////////////////////////////////////////////////////////
	memcpy(&ttf_data->OutputData, &pConfigTemp->OutputData, sizeof(stOutputData));
	stPrintTTFConfig_Release(&pConfigTemp);
	//

	return result;
}

int PrintString(stPrintTTFConfig* pttf_data)
{
	int rt = 0;

	PrintString_Immediately(pttf_data);

	return rt;
}

int Translate_Color(stColorData ColorDataIn, int ColorIn_Format, stColorData* ColorDataOut, int ColorOut_Format)
{
	int rt = 0;
	//
	if (ColorIn_Format == ColorOut_Format)
	{
		ColorDataOut->color_1 = ColorDataIn.color_1;
		ColorDataOut->color_2 = ColorDataIn.color_2;
		ColorDataOut->color_3 = ColorDataIn.color_3;
	}

	if ((ColorIn_Format == COLOR_FORMAT_RGB) && (ColorOut_Format == COLOR_FORMAT_YYCBCR))
	{
		ColorDataOut->color_1 = (RGB2YUV(ColorDataIn.color_1, ColorDataIn.color_2, ColorDataIn.color_3) & 0x00FF0000) >> 16;
		ColorDataOut->color_2 = (RGB2YUV(ColorDataIn.color_1, ColorDataIn.color_2, ColorDataIn.color_3) & 0x0000FF00) >>  8;
		ColorDataOut->color_3 = (RGB2YUV(ColorDataIn.color_1, ColorDataIn.color_2, ColorDataIn.color_3) & 0x000000FF) >>  0;
	}
	//
	return rt;

}


int FE_Init(int Font_File_Source_Option)
{
    MP_DEBUG("%s", __func__);
	FT_Error      error;
	int rt = 0;

#ifdef TARGET_DEVELOPMENT

    //////////////////////////////////////////////////////////////////
    //
    //  Search TTF file
    //
    //////////////////////////////////////////////////////////////////
    //Font_Data_At_File_Get_List(SD_MMC, 512); //CJ  add to Load the default TTF Font
    if (Font_Data_At_File_Get_List(SD_MMC, 512) == 1)
        MP_ALERT("Load TTF file from SD !!");
#if CF_ENABLE
    else if (Font_Data_At_File_Get_List(CF, 512) == 1)
        MP_ALERT("Load TTF file from CF !!");
#endif
#if (SYSTEM_DRIVE)
    else if (Font_Data_At_File_Get_List(SYS_DRV_ID, 512) == 1)
        MP_ALERT("Load TTF file from system drive !!");
#endif
#if NAND_ENABLE
    else if (Font_Data_At_File_Get_List(NAND, 512) == 1)
        MP_ALERT("Load TTF file from NAND !!");
#endif
    else
    {
        MP_ALERT("--E-- Load TTF file error !!\n\r");
        __asm("break 100");
    }

    //////////////////////////////////////////////////////////////////
    //
    //
    //
    //////////////////////////////////////////////////////////////////
	if (Font_File_Source_Option == FONT_DATA_AT_MEMORY)
	{
		rt = OpenFontFileMemBuffer(/*Font File Tag*/0x464F4E54);
		error = FT_Init_FreeType( &library );
		if (error > 0)	rt = -1;
		error = FT_New_Memory_Face( library, /*BASE*/(pFontFileMemBuffer_Start), /*SIZE*/pFontFileMemBuffer_Size, 0, &face );
		if (error > 0)	rt = -1;
	}
	if (Font_File_Source_Option == FONT_DATA_AT_FILE)
	{
		error = FT_Init_FreeType( &library );
		if (error > 0)	rt = -1;
		error = FT_New_Memory_Face( library, /*BASE*/pFontFileMemBuffer_Start, /*SIZE*/pFontFileMemBuffer_Size, 0, &face );
		if (error > 0)	rt = -1;
	}
#endif

#ifdef PC_DEVELOPMENT
	error = FT_Init_FreeType( &library );
	if (error > 0)	rt = -1;
	//error = FT_New_Face( library, "c:\\kaiu.ttf", 0, &face );
	//if (error > 0)	rt = -1;
	//error = FT_New_Face( library, "c:\\Font_1.TTF", 0, &face );
	error = FT_New_Face( library, ttf_file_name, 0, &face );

	if (error > 0)	rt = -1;
#endif

	//Initialize CacheManager
	//pCM = CacheManager_Create(pttf_config, pFace, str, str_length);
	pCM = CacheManager_Create(NULL, NULL, NULL, 0x0);

	return rt;
}
int FE_Release()
{
	if (pCM != NULL)
	{
		CacheManager_Release(&pCM);
	}
	FT_Done_Face( face );
	FT_Done_FreeType( library );
	ext_mem_free(pFontFileMemBuffer_Start);
	pFontFileMemBuffer_Start = NULL;
	return 0;
}


int  stPrintTTFConfig_Init(stPrintTTFConfig* pConfig)
{
	int result = 1;
	//
	if (pConfig == NULL)
		return 0;
	//
	pConfig->BaselineOn = 0;
	pConfig->BlendingOn = 0;

	pConfig->CacheSetting.CacheOn = 0;
	pConfig->CacheSetting.Capacity = 0;
	pConfig->ColorParameter_Baseline.Alpha = 0;
	pConfig->ColorParameter_Baseline.Data.color_1 = 0;
	pConfig->ColorParameter_Baseline.Data.color_2 = 0;
	pConfig->ColorParameter_Baseline.Data.color_3 = 0;
	pConfig->ColorParameter_Baseline.Format = COLOR_FORMAT_YYCBCR;

	pConfig->ColorParameter_Glyph.Alpha = 0;
	pConfig->ColorParameter_Glyph.Data.color_1 = 0;
	pConfig->ColorParameter_Glyph.Data.color_2 = 0;
	pConfig->ColorParameter_Glyph.Data.color_3 = 0;
	pConfig->ColorParameter_Glyph.Format = COLOR_FORMAT_YYCBCR;

	pConfig->ColorParameter_PrintRegion.Alpha = 0;
	pConfig->ColorParameter_PrintRegion.Data.color_1 = 0;
	pConfig->ColorParameter_PrintRegion.Data.color_2 = 0;
	pConfig->ColorParameter_PrintRegion.Data.color_3 = 0;
	pConfig->ColorParameter_PrintRegion.Format = COLOR_FORMAT_YYCBCR;


	pConfig->ColorParameter_Rectangle.Alpha = 0;
	pConfig->ColorParameter_Rectangle.Data.color_1 = 0;
	pConfig->ColorParameter_Rectangle.Data.color_2 = 0;
	pConfig->ColorParameter_Rectangle.Data.color_3 = 0;
	pConfig->ColorParameter_Rectangle.Format = COLOR_FORMAT_YYCBCR;

	pConfig->Encoding = FT_ENCODING_UNICODE;
	pConfig->GlyphSetMode = GLYPH_SET_MODE_DELIMITER;

	pConfig->ImagePlaneData.DeviceResolution.x = 0;
	pConfig->ImagePlaneData.DeviceResolution.y = 0;
	pConfig->ImagePlaneData.ImagePlane = NULL;
	pConfig->ImagePlaneData.ImagePlaneMode = DISPLAY_MODE_YYCBCR;
	pConfig->ImagePlaneData.ImagePlaneSize.x = 0;
	pConfig->ImagePlaneData.ImagePlaneSize.y = 0;

	pConfig->MeasureMode = MEASURE_MODE_STRING;

	pConfig->OutputData.Count = 0;
	pConfig->OutputData.LastChangeLineGlyphIndex = -1;
	pConfig->OutputData.LastNotCroppedGlyphIndex = -1;
	pConfig->OutputData.LastGlyphIndexAtFirstCroppedLine = -1;
	pConfig->OutputData.NextPos.x = 0;
	pConfig->OutputData.NextPos.y = 0;
	pConfig->OutputData.RectangleSize.x = 0;
	pConfig->OutputData.RectangleSize.y = 0;
	pConfig->OutputData.StrHBearingXPixelFirstGlyph = 0;
	pConfig->OutputData.StrHBearingYPixel = 0;

	pConfig->PrintCoordinate.ChangeLineXAlignMode = CHANGE_LINE_X_ALIGN_MODE_FIRST_LINE;
	pConfig->PrintCoordinate.CoordinateOriginMode = COORDINATE_ORIGIN_MODE_IMAGE_PLANE;
	pConfig->PrintCoordinate.LineHeightPixel = 0;
	pConfig->PrintCoordinate.NotDrawOverBorder = 0;
	pConfig->PrintCoordinate.PrintPos.x = 0;
	pConfig->PrintCoordinate.PrintPos.y = 0;
	pConfig->PrintCoordinate.PrintRegion.pt_1.x = 0;
	pConfig->PrintCoordinate.PrintRegion.pt_1.y = 0;
	pConfig->PrintCoordinate.PrintRegion.pt_2.x = 0;
	pConfig->PrintCoordinate.PrintRegion.pt_2.y = 0;
	pConfig->PrintCoordinate.PrintRegionMode = PRINT_REGION_MODE_IMAGE_PLANE;
	pConfig->PrintCoordinate.SpecifiedChangeLineX = 0;

	pConfig->PrintRegionOn = 0;
	pConfig->PrintStringOn = 0;
	pConfig->RectangleOn = 0;
	pConfig->Rendering = FT_RENDER_MODE_NORMAL;

	pConfig->StringData.DelimiterList = NULL;
	pConfig->StringData.DelimiterListLength = 0;
	pConfig->StringData.FontSize = 0;
	pConfig->StringData.Str = NULL;
	pConfig->StringData.Str_Index_Start = 0;
	pConfig->StringData.Str_Index_Stop = 0;
	pConfig->StringData.StrLength = 0;
	pConfig->StringData.StrLineHeightCompensate = 0;
	pConfig->StringData.StrLineHeightRef = NULL;
	pConfig->StringData.StrLineHeightRefLength = 0;

	pConfig->FontRotation = 0;
	pConfig->RotationFontOrderDirection=0;
	//

	//error = FT_Init_FreeType( &library );
	//error = FT_Init_FreeType( &(pConfig->FontFace.Library) );
	//if (error > 0)	result = -1;

	//if (error > 0)	rt = -1;
	//error = FT_New_Face( pConfig->FontFace.Library, "c:\\Font_1.TTF", 0, &(pConfig->FontFace.Face) );




	//
	return result;
}

#if 1
stPrintTTFConfig* stPrintTTFConfig_Copy(stPrintTTFConfig* pSource)
{
	int rt = 0;
	long* Str_Temp = NULL;

	stPrintTTFConfig* pTarget = (stPrintTTFConfig*)MEMORY_MALLOC(sizeof(stPrintTTFConfig));
	//
	if (pSource == NULL)
		return NULL;
	memset(pTarget,0,sizeof(stPrintTTFConfig));

	memcpy(pTarget, pSource, sizeof(stPrintTTFConfig));
	//
	Str_Temp = (long*)ext_mem_malloc(sizeof(long)*pTarget->StringData.StrLength);
	pTarget->StringData.Str = Str_Temp;
	memcpy(pTarget->StringData.Str, pSource->StringData.Str, sizeof(long) * pTarget->StringData.StrLength);

	Str_Temp = (long*)ext_mem_malloc(sizeof(long)*pTarget->StringData.DelimiterListLength);
	pTarget->StringData.DelimiterList = Str_Temp;
	memcpy(pTarget->StringData.DelimiterList, pSource->StringData.DelimiterList, sizeof(long) * pTarget->StringData.DelimiterListLength);

	Str_Temp = (long*)ext_mem_malloc(sizeof(long)*pTarget->StringData.StrLineHeightRefLength);
	pTarget->StringData.StrLineHeightRef = Str_Temp;
	memcpy(pTarget->StringData.StrLineHeightRef, pSource->StringData.StrLineHeightRef, sizeof(long) * pTarget->StringData.StrLineHeightRefLength);

	/*
	pTarget->StringData.Str = (long*)MEMORY_CALLOC(pTarget->StringData.StrLength, sizeof(long));
	memcpy(pTarget->StringData.Str, pSource->StringData.Str, sizeof(long) * pTarget->StringData.StrLength);
	pTarget->StringData.DelimiterList = (long*)MEMORY_CALLOC(pTarget->StringData.DelimiterListLength, sizeof(long));
	memcpy(pTarget->StringData.DelimiterList, pSource->StringData.DelimiterList, sizeof(long) * pTarget->StringData.DelimiterListLength);
	*/
	//
	return pTarget;
}
#else
stPrintTTFConfig* stPrintTTFConfig_Copy(stPrintTTFConfig* pSource)
{
	int rt = 0;
	long* Str_Temp = NULL;
	stPrintTTFConfig* pTarget = (stPrintTTFConfig*)MEMORY_MALLOC(sizeof(stPrintTTFConfig));
	//
	if (pSource == NULL)
		return NULL;

	memcpy(pTarget, pSource, sizeof(stPrintTTFConfig));
	//
	Str_Temp = (long*)MEMORY_CALLOC(pTarget->StringData.StrLength, sizeof(long));
	pTarget->StringData.Str = Str_Temp;
	memcpy(pTarget->StringData.Str, pSource->StringData.Str, sizeof(long) * pTarget->StringData.StrLength);

	Str_Temp = (long*)MEMORY_CALLOC(pTarget->StringData.DelimiterListLength, sizeof(long));
	pTarget->StringData.DelimiterList = Str_Temp;
	memcpy(pTarget->StringData.DelimiterList, pSource->StringData.DelimiterList, sizeof(long) * pTarget->StringData.DelimiterListLength);

	Str_Temp = (long*)MEMORY_CALLOC(pTarget->StringData.StrLineHeightRefLength, sizeof(long));
	pTarget->StringData.StrLineHeightRef = Str_Temp;
	memcpy(pTarget->StringData.StrLineHeightRef, pSource->StringData.StrLineHeightRef, sizeof(long) * pTarget->StringData.StrLineHeightRefLength);

	/*
	pTarget->StringData.Str = (long*)MEMORY_CALLOC(pTarget->StringData.StrLength, sizeof(long));
	memcpy(pTarget->StringData.Str, pSource->StringData.Str, sizeof(long) * pTarget->StringData.StrLength);
	pTarget->StringData.DelimiterList = (long*)MEMORY_CALLOC(pTarget->StringData.DelimiterListLength, sizeof(long));
	memcpy(pTarget->StringData.DelimiterList, pSource->StringData.DelimiterList, sizeof(long) * pTarget->StringData.DelimiterListLength);
	*/
	//
	return pTarget;
}
#endif

int stPrintTTFConfig_Release(stPrintTTFConfig** ppConfig)
{
	int rt = 0;
	//
	if ((*ppConfig) == NULL)
		return 0;

	MEMORY_FREE((*ppConfig)->StringData.Str);

	MEMORY_FREE((*ppConfig)->StringData.DelimiterList);

	MEMORY_FREE((*ppConfig)->StringData.StrLineHeightRef);

	MEMORY_FREE((*ppConfig));

	ppConfig = 0x0;

	//
	return rt;
}

int  stPrintTTFConfig_ResetOutputData(stPrintTTFConfig* pConfig)
{
	if (pConfig == NULL)
		return 0;
	pConfig->OutputData.Count = 0;
	pConfig->OutputData.LastChangeLineGlyphIndex = 0;
	pConfig->OutputData.LastGlyphIndexAtFirstCroppedLine = 0;
	pConfig->OutputData.LastNotCroppedGlyphIndex = 0;
	pConfig->OutputData.NextPos.x = 0;
	pConfig->OutputData.NextPos.y = 0;
	pConfig->OutputData.RectangleSize.x = 0;
	pConfig->OutputData.RectangleSize.y = 0;
	pConfig->OutputData.StrHBearingXPixelFirstGlyph = 0;
	pConfig->OutputData.StrHBearingYPixel = 0;

	return 1;
}

int  stPrintTTFConfig_Normalize(stPrintTTFConfig* pConfig)
{
	int result = 1;
	//
	if (pConfig == NULL)
	{
		return 0;
	}
	//////////////////////
	switch(pConfig->PrintCoordinate.ChangeLineXAlignMode)
	{
	case CHANGE_LINE_X_ALIGN_MODE_FIRST_LINE:
		{
			if (pConfig->PrintCoordinate.CoordinateOriginMode == COORDINATE_ORIGIN_MODE_PRINT_REGION)
			{
				pConfig->PrintCoordinate.SpecifiedChangeLineX = pConfig->PrintCoordinate.PrintPos.x + pConfig->PrintCoordinate.PrintRegion.pt_1.x;
			}
			if (pConfig->PrintCoordinate.CoordinateOriginMode == COORDINATE_ORIGIN_MODE_IMAGE_PLANE)
			{
				pConfig->PrintCoordinate.SpecifiedChangeLineX = pConfig->PrintCoordinate.PrintPos.x;
			}
			pConfig->PrintCoordinate.ChangeLineXAlignMode = CHANGE_LINE_X_ALIGN_MODE_SPECIFIED_X;
		}
		break;
	case CHANGE_LINE_X_ALIGN_MODE_PRINT_REGION:
		{
			if (pConfig->PrintCoordinate.CoordinateOriginMode == COORDINATE_ORIGIN_MODE_PRINT_REGION)
			{
				pConfig->PrintCoordinate.SpecifiedChangeLineX = pConfig->PrintCoordinate.PrintRegion.pt_1.x;
			}
			if (pConfig->PrintCoordinate.CoordinateOriginMode == COORDINATE_ORIGIN_MODE_IMAGE_PLANE)
			{
				pConfig->PrintCoordinate.SpecifiedChangeLineX = pConfig->PrintCoordinate.PrintRegion.pt_1.x;
			}
			pConfig->PrintCoordinate.ChangeLineXAlignMode = CHANGE_LINE_X_ALIGN_MODE_SPECIFIED_X;
		}
		break;
	case CHANGE_LINE_X_ALIGN_MODE_SPECIFIED_X:
		{
			if (pConfig->PrintCoordinate.CoordinateOriginMode == COORDINATE_ORIGIN_MODE_PRINT_REGION)
			{
				pConfig->PrintCoordinate.SpecifiedChangeLineX += pConfig->PrintCoordinate.PrintRegion.pt_1.x ;
			}
			if (pConfig->PrintCoordinate.CoordinateOriginMode == COORDINATE_ORIGIN_MODE_IMAGE_PLANE)
			{
			}
		}
		break;
	default :
		{
		}
		break;

	}

	//////////////////////
	if (pConfig->PrintCoordinate.CoordinateOriginMode == COORDINATE_ORIGIN_MODE_PRINT_REGION)
	{
		pConfig->PrintCoordinate.PrintPos.x += pConfig->PrintCoordinate.PrintRegion.pt_1.x;
		pConfig->PrintCoordinate.PrintPos.y += pConfig->PrintCoordinate.PrintRegion.pt_1.y;
		//
		pConfig->PrintCoordinate.CoordinateOriginMode = COORDINATE_ORIGIN_MODE_IMAGE_PLANE;
	}
	//////////////////////
	if (pConfig->PrintCoordinate.PrintRegionMode == PRINT_REGION_MODE_IMAGE_PLANE)
	{
		pConfig->PrintCoordinate.PrintRegion.pt_1.x = 0;
		pConfig->PrintCoordinate.PrintRegion.pt_1.y = 0;
		pConfig->PrintCoordinate.PrintRegion.pt_2.x = pConfig->ImagePlaneData.ImagePlaneSize.x-1;
		pConfig->PrintCoordinate.PrintRegion.pt_2.y = pConfig->ImagePlaneData.ImagePlaneSize.y-1;
		pConfig->PrintCoordinate.PrintRegionMode = PRINT_REGION_MODE_PRINT_REGION;
	}


	//
	return result;
}


#if 0
pGlyphContainer_LinkedList MeasureString(stPrintTTFConfig* ttf_data)
{
	int index = ttf_data->StringData.Str_Index_Start;
	int rt = 0;
	int n1, n2;
	int length = 0;
	//
	stPoint pos;
	pGlyphContainer_LinkedList Head = NULL;
	pGlyphContainer_LinkedList Curr = NULL;
	//
	pos.x = ttf_data->PrintCoordinate.PrintPos.x;
	pos.y = ttf_data->PrintCoordinate.PrintPos.y;
	ttf_data->OutputData.RectangleSize.x = 0;
	ttf_data->OutputData.RectangleSize.y = 0;

	if ((ttf_data->StringData.Str_Index_Start < 0)||
		(ttf_data->StringData.Str_Index_Stop < 0)||
		(ttf_data->StringData.Str_Index_Start>=ttf_data->StringData.StrLength)||
		(ttf_data->StringData.Str_Index_Stop>=ttf_data->StringData.StrLength))
	{
		return NULL;
	}

	//---------------------------------
	//Add string to the cache manager
	length = CacheManager_AddUnitArray(pCM, ttf_data, face,
							           (ttf_data->StringData.Str + ttf_data->StringData.Str_Index_Start),
							           (ttf_data->StringData.Str_Index_Stop - ttf_data->StringData.Str_Index_Start + 1)
							           );




	//


	while(index <= ttf_data->StringData.Str_Index_Stop)
	{
		pGlyphContainer pGlyphCon_Temp = NULL;
		CACHE_MANAGER_FUNC_RESULT result = CACHE_MANAGER_FUNC_RESULT_SUCCESSFUL;
		stGlyphContainerFeature feature;
		//
		GlyphContainerFeature_Config(&feature, ttf_data, face,ttf_data->StringData.Str[index]);


		if (Curr == NULL)
		{
			Curr = GlyphContainerLinkedList_Create(NULL);
			Head = Curr;
		}

		Curr->pGlyphCon = GlyphContainer_Create(&feature);
		//Get_Char_BMP2(ttf_data, ttf_data->StringData.Str[index], Curr->pGlyphCon);

		Curr->index = index;
		//----------------
		if (index==ttf_data->StringData.Str_Index_Start)
		{
			ttf_data->OutputData.StrHBearingXPixelFirstGlyph = (Curr->pGlyphCon->Metrics.horiBearingX>>6);
			ttf_data->OutputData.StrHBearingYPixel = (Curr->pGlyphCon->Metrics.horiBearingY>>6);
			if (index == ttf_data->StringData.Str_Index_Stop)
			{
				ttf_data->OutputData.RectangleSize.x = (Curr->pGlyphCon->Metrics.width>>6);
			}
			else
			{
				ttf_data->OutputData.RectangleSize.x = (Curr->pGlyphCon->Metrics.horiAdvance>>6) - (Curr->pGlyphCon->Metrics.horiBearingX>>6);
			}
			//
			n1 = (pos.y-(Curr->pGlyphCon->Metrics.horiBearingY>>6));
			n2 = (pos.y-(Curr->pGlyphCon->Metrics.horiBearingY>>6)+(Curr->pGlyphCon->Metrics.height>>6));
		}


		//Decide output data and rectangle
		if ((Curr->pGlyphCon->Metrics.horiBearingY>>6) > ttf_data->OutputData.StrHBearingYPixel)
			ttf_data->OutputData.StrHBearingYPixel = (Curr->pGlyphCon->Metrics.horiBearingY>>6);

		if ((pos.y-(Curr->pGlyphCon->Metrics.horiBearingY>>6)) < n1)
			n1 = (pos.y-(Curr->pGlyphCon->Metrics.horiBearingY>>6));
		if ((pos.y-(Curr->pGlyphCon->Metrics.horiBearingY>>6)+(pos.y-(Curr->pGlyphCon->Metrics.height>>6)) > n2))
			n2 = (pos.y-(Curr->pGlyphCon->Metrics.horiBearingY>>6)+(Curr->pGlyphCon->Metrics.height>>6));

		ttf_data->OutputData.RectangleSize.y = n2 - n1;
		//
		if ((index > ttf_data->StringData.Str_Index_Start) && (index < ttf_data->StringData.Str_Index_Stop))
		{
			ttf_data->OutputData.RectangleSize.x += (Curr->pGlyphCon->Metrics.horiAdvance>>6);
		}
		//
		if ((index != ttf_data->StringData.Str_Index_Start) && (index == ttf_data->StringData.Str_Index_Stop))
		{
			ttf_data->OutputData.RectangleSize.x += (Curr->pGlyphCon->Metrics.horiBearingX>>6) + (Curr->pGlyphCon->Metrics.width>>6);
		}

		if ((Curr->pGlyphCon->Metrics.horiBearingY>>6) > ttf_data->OutputData.StrHBearingYPixel)
			ttf_data->OutputData.StrHBearingYPixel = (Curr->pGlyphCon->Metrics.horiBearingY>>6);
		//
		pos.x += (Curr->pGlyphCon->Metrics.horiAdvance>>6);
		ttf_data->OutputData.NextPos.x = pos.x;
		ttf_data->OutputData.NextPos.y = pos.y;

		//
		index++;
		if (index<=ttf_data->StringData.Str_Index_Stop)
		{
			Curr->next = GlyphContainerLinkedList_Create(NULL);
			Curr = Curr->next;
		}
		//
		if (pGlyphCon_Temp != NULL)
		{
			GlyphContainer_Release(&pGlyphCon_Temp);
		}
	}
	//


	//

	return Head;
}
#endif

int MeasureString_RectangleFilled(stPrintTTFConfig* ttf_data)
{
	int result = 0;
	stPrintTTFConfig* pConfigTemp = stPrintTTFConfig_Copy(ttf_data);
	//
	stPoint pos;
	int index = pConfigTemp->StringData.Str_Index_Start;
	int continuous_not_cropped = 1;
	int Y_Bottom = 0, Y_Bottom_temp, LastGlyphIndexAtFirstCroppedLine_Candidate, LastChangeLineGlyphIndex_Old = -1;
	stPoint original_pos, print_pos, next_pos;
	stRectangle Print_Border = pConfigTemp->PrintCoordinate.PrintRegion;
	//Normalize config
	stPrintTTFConfig_Normalize(pConfigTemp);
	//
	pConfigTemp->MeasureMode = MEASURE_MODE_STRING;
	original_pos = pConfigTemp->PrintCoordinate.PrintPos;
	print_pos = pConfigTemp->PrintCoordinate.PrintPos;
	next_pos = pConfigTemp->PrintCoordinate.PrintPos;
	Print_Border = pConfigTemp->PrintCoordinate.PrintRegion;
	//
	pos.x = pConfigTemp->PrintCoordinate.PrintPos.x;
	pos.y = pConfigTemp->PrintCoordinate.PrintPos.y;
	//
	print_pos = pConfigTemp->PrintCoordinate.PrintPos;
	//////////////////////////////////////////////////////////////////////////////////////
	//Process "Glyph Set" one by one
	while(index <= pConfigTemp->StringData.Str_Index_Stop)
	{
		int NextSubStringLength;
		stPoint pt0 = original_pos;
		int StringStartTemp1 = pConfigTemp->StringData.Str_Index_Start;
		int StringStopTemp2 = pConfigTemp->StringData.Str_Index_Stop;
		//
		GetNextSubStringLength(pConfigTemp, index, &NextSubStringLength);
		pConfigTemp->StringData.Str_Index_Start = index;
		pConfigTemp->StringData.Str_Index_Stop = index + NextSubStringLength - 1;

		MeasureString(pConfigTemp);

		//Change line or not? Shift the rectangle of the string while changing line
		if ((print_pos.x + pConfigTemp->OutputData.StrHBearingXPixelFirstGlyph + pConfigTemp->OutputData.RectangleSize.x-1) >= Print_Border.pt_2.x)
		{
			print_pos.x = pConfigTemp->PrintCoordinate.SpecifiedChangeLineX;
			print_pos.y += pConfigTemp->PrintCoordinate.LineHeightPixel;
			//
			pConfigTemp->PrintCoordinate.PrintPos.x = print_pos.x;
			pConfigTemp->PrintCoordinate.PrintPos.y = print_pos.y;
			pConfigTemp->OutputData.NextPos.x = pConfigTemp->PrintCoordinate.PrintPos.x + pConfigTemp->OutputData.RectangleSize.x;
			pConfigTemp->OutputData.NextPos.y = pConfigTemp->PrintCoordinate.PrintPos.y;
			//Record the index the of last glyph at the last line in the region
			if ((continuous_not_cropped == 0)&&(pConfigTemp->OutputData.LastGlyphIndexAtFirstCroppedLine == -1))
			{
				pConfigTemp->OutputData.LastGlyphIndexAtFirstCroppedLine = LastGlyphIndexAtFirstCroppedLine_Candidate;
			}
		}

		//Record the index the of last glyph at the last not-cropped line
		if (continuous_not_cropped == 1)
		{
			LastChangeLineGlyphIndex_Old = pConfigTemp->OutputData.LastChangeLineGlyphIndex;
			pConfigTemp->OutputData.LastChangeLineGlyphIndex = index + NextSubStringLength - 1;
		}


		//Cropped?
		Y_Bottom_temp = pConfigTemp->PrintCoordinate.PrintPos.y - pConfigTemp->OutputData.StrHBearingYPixel + pConfigTemp->OutputData.RectangleSize.y;
		Y_Bottom = (Y_Bottom_temp>Y_Bottom)?Y_Bottom_temp:Y_Bottom;
		if (Y_Bottom > Print_Border.pt_2.y)
		{
			LastGlyphIndexAtFirstCroppedLine_Candidate = index + NextSubStringLength - 1;

			if (continuous_not_cropped>0)
			{
				continuous_not_cropped = 0;
				pConfigTemp->OutputData.LastChangeLineGlyphIndex = LastChangeLineGlyphIndex_Old;
			}
		}


		//////////////////
		index += NextSubStringLength;
		pConfigTemp->StringData.Str_Index_Start = StringStartTemp1;
		pConfigTemp->StringData.Str_Index_Stop = StringStopTemp2;
		print_pos.x = pConfigTemp->OutputData.NextPos.x;
		print_pos.y = pConfigTemp->OutputData.NextPos.y;
		pConfigTemp->PrintCoordinate.PrintPos.x = pConfigTemp->OutputData.NextPos.x;
		pConfigTemp->PrintCoordinate.PrintPos.y = pConfigTemp->OutputData.NextPos.y;
	}


	//////////////////////////////////////////////////////////////////////////////////////
	memcpy(&ttf_data->OutputData, &pConfigTemp->OutputData, sizeof(stOutputData));
	stPrintTTFConfig_Release(&pConfigTemp);
	//
	return result;
}

int MeasureString_String(stPrintTTFConfig* ttf_data)
{
#if EREADER_ENABLE
	SemaphoreWait(TYPESETTING_SEMA_ID);
#endif
	int result = 1;
	int index = ttf_data->StringData.Str_Index_Start;
	int rt = 0;
	int n1, n2;
	int length = 0;
	stPoint pos;
	int Endindex=ttf_data->StringData.Str_Index_Stop; //Test Fix

	//Pre-set data
	pos.x = ttf_data->PrintCoordinate.PrintPos.x;
	pos.y = ttf_data->PrintCoordinate.PrintPos.y;
	ttf_data->OutputData.RectangleSize.x = 0;
	ttf_data->OutputData.RectangleSize.y = 0;
	//
	if(Endindex-index>500) //Test Fix
		Endindex=index+500;

	while(index <= Endindex)
	{
		//TaskYield();
		pGlyphContainer pGlyphCon_Temp = NULL;
		//CACHE_MANAGER_FUNC_RESULT result = CACHE_MANAGER_FUNC_RESULT_SUCCESSFUL;
		//static stGlyphContainerFeature feature;
		stGlyphContainerFeature feature;
		int BearingX, BearingY, Advance, Height, Width,vBearingX,vBearingY,vAdvance;

		MP_DEBUG("ttf_data->StringData.Str=%x ttf_data->StringData.Str=%x",ttf_data->StringData.Str+index,*(ttf_data->StringData.Str+index));

		//
		memset(&feature,0,sizeof(stGlyphContainerFeature)); //cj add
		GlyphContainerFeature_Config(&feature, ttf_data, face,ttf_data->StringData.Str[index]);
		//
		//pGlyphCon_Temp = CacheManager_RequestUnitMetrics(pCM, &feature, WHILE_CACHE_MISS_DO_ADD);
		pGlyphCon_Temp = CacheManager_RequestUnitGlyphBMP(pCM, &feature, WHILE_CACHE_MISS_DO_ADD);
		if (pGlyphCon_Temp == 0x0)
		{
			GlyphContainerLinkedList_AddUnit(pCM, &feature, pCM->bAddDummy4BytesAlign);
			pGlyphCon_Temp = CacheManager_RequestUnitGlyphBMP(pCM, &feature, WHILE_CACHE_MISS_DO_ADD);
		}

		Width = (pGlyphCon_Temp->Metrics.width>>6);
		Height = (pGlyphCon_Temp->Metrics.height>>6);
		BearingX = (pGlyphCon_Temp->Metrics.horiBearingX>>6);
		BearingY = (pGlyphCon_Temp->Metrics.horiBearingY>>6);
		Advance = (pGlyphCon_Temp->Metrics.horiAdvance>>6);
		vBearingX = (pGlyphCon_Temp->Metrics.vertBearingX>>6);
		vBearingY = (pGlyphCon_Temp->Metrics.vertBearingY>>6);
		vAdvance = (pGlyphCon_Temp->Metrics.vertAdvance>>6);
	
		if((ttf_data->FontRotation == 0) || (ttf_data->FontRotation == 180))
		{ 
			//mpDebugPrint(" 000 CharCode=%x W=%d h=%d Bx=%d By=%d Ad=%d vBx=%d vBy=%d vAdvance=%d",feature.CharCode,Width,Height,BearingX,BearingY,Advance,vBearingX,vBearingY,vAdvance);

			if (index==ttf_data->StringData.Str_Index_Start)
			{
				ttf_data->OutputData.StrHBearingXPixelFirstGlyph = BearingX;
				ttf_data->OutputData.StrHBearingYPixel = BearingY;
			#if 0
				if (index == ttf_data->StringData.Str_Index_Stop)
				{
					ttf_data->OutputData.RectangleSize.x = Width;
				}
				else
				{		
					if(Width != 0)
						ttf_data->OutputData.RectangleSize.x =  Advance - BearingX;
					else
						ttf_data->OutputData.RectangleSize.x = Advance>>1- BearingX;
					//ttf_data->OutputData.RectangleSize.x = Advance - BearingX;
				}
			#endif
				//
				n1 = (pos.y-BearingY);
				n2 = (pos.y-BearingY+Height);
			}

			//Decide output data and rectangle
			if (BearingY > ttf_data->OutputData.StrHBearingYPixel)
				ttf_data->OutputData.StrHBearingYPixel = BearingY;

			if ((pos.y - BearingY) < n1)
				n1 = (pos.y-BearingY);

			if ((pos.y - BearingY +  Height) > n2)
				n2 = (pos.y-BearingY+Height);

			ttf_data->OutputData.RectangleSize.y = n2 - n1;
#if 0
		//
		if ((index > ttf_data->StringData.Str_Index_Start) && (index < ttf_data->StringData.Str_Index_Stop))
		{
			ttf_data->OutputData.RectangleSize.x += Advance;
		}
		//
		if ((index != ttf_data->StringData.Str_Index_Start) && (index == ttf_data->StringData.Str_Index_Stop))
		{
			ttf_data->OutputData.RectangleSize.x += BearingX + Width;
		}
#endif
			if (BearingY > ttf_data->OutputData.StrHBearingYPixel)
				ttf_data->OutputData.StrHBearingYPixel = BearingY;

			//
			//CJ modify
			//pos.x += Advance;
			
			if(Width != 0)
				{
					pos.x += BearingX+Width;
					ttf_data->OutputData.RectangleSize.x+= BearingX+Width;
				}
			else
				{
					pos.x += Advance>>1;
					ttf_data->OutputData.RectangleSize.x+= Advance>>1;
				}
		}
		else if((ttf_data->FontRotation == 90) || (ttf_data->FontRotation == 270))
		{

			if (index==ttf_data->StringData.Str_Index_Start)
				{
					ttf_data->OutputData.StrHBearingXPixelFirstGlyph = vBearingY;
					ttf_data->OutputData.StrHBearingYPixel = vAdvance;

					n1 = (pos.y-vAdvance);
					n2 = (pos.y);
				}


			//Decide output data and rectangle
			if (vAdvance > ttf_data->OutputData.StrHBearingYPixel)
				ttf_data->OutputData.StrHBearingYPixel = vAdvance;

			 if((pos.y - vAdvance) < n1)
				n1 = (pos.y-vAdvance);

			n2 = pos.y;
			ttf_data->OutputData.RectangleSize.y = n2 - n1;

			if (vAdvance > ttf_data->OutputData.StrHBearingYPixel)
				ttf_data->OutputData.StrHBearingYPixel = vAdvance;

			if(Width != 0)
				{
					pos.x += Height+vBearingY;
					ttf_data->OutputData.RectangleSize.x+= Height+vBearingY;
				}
			else
				{
					pos.x += vAdvance>>1;
					ttf_data->OutputData.RectangleSize.x+= vAdvance>>1;
				}
		}

		
		ttf_data->OutputData.NextPos.x = pos.x;
		ttf_data->OutputData.NextPos.y = pos.y;

		index++;

	}
#if EREADER_ENABLE
	SemaphoreRelease(TYPESETTING_SEMA_ID);
#endif
	return result;
}

#if 0
int MeasureString_String_V(stPrintTTFConfig* ttf_data)
{
	int result = 1;
	int index = ttf_data->StringData.Str_Index_Start;
	int rt = 0;
	int n1, n2;
	int length = 0;
	stPoint pos;
	int Endindex=ttf_data->StringData.Str_Index_Stop; //Test Fix
       int BearingX, BearingY, Advance, Height, Width;
	int vBearingX, vBearingY, vAdvance;

	//Pre-set data
	pos.x = ttf_data->PrintCoordinate.PrintPos.x;
	pos.y = ttf_data->PrintCoordinate.PrintPos.y;
	ttf_data->OutputData.RectangleSize.x = 0;
	ttf_data->OutputData.RectangleSize.y = 0;
	//
	if(Endindex-index>500) //Test Fix
		Endindex=index+500;

	while(index <= Endindex)
	{
	//if(Polling_Event())
		TaskYield();
		pGlyphContainer pGlyphCon_Temp = NULL;
		//CACHE_MANAGER_FUNC_RESULT result = CACHE_MANAGER_FUNC_RESULT_SUCCESSFUL;
		stGlyphContainerFeature feature;
		//int BearingX, BearingY, Advance, Height, Width;
		int bTemp = 0;

		MP_DEBUG("ttf_data->StringData.Str=%x ttf_data->StringData.Str=%x",ttf_data->StringData.Str+index,*(ttf_data->StringData.Str+index));
		//
		if (ttf_data->CompactMetricsOn > 0)
		{
			bTemp =CompactMetricsArray_Query(ttf_data->StringData.FontSize, ttf_data->StringData.Str[index], &Width, &Height, &Advance, &BearingX, &BearingY);
		}

		if (bTemp < 1)
		{
		GlyphContainerFeature_Config(&feature, ttf_data, face,ttf_data->StringData.Str[index]);
		//
		//pGlyphCon_Temp = CacheManager_RequestUnitMetrics(pCM, &feature, WHILE_CACHE_MISS_DO_ADD);
		pGlyphCon_Temp = CacheManager_RequestUnitGlyphBMP(pCM, &feature, WHILE_CACHE_MISS_DO_ADD);
		if (pGlyphCon_Temp == 0x0)
		{
			GlyphContainerLinkedList_AddUnit(pCM, &feature, pCM->bAddDummy4BytesAlign);
			pGlyphCon_Temp = CacheManager_RequestUnitGlyphBMP(pCM, &feature, WHILE_CACHE_MISS_DO_ADD);
		}

		//
		//cj add for vertical 05192010
		//BearingX = (pGlyphCon_Temp->Metrics.horiBearingX>>6);
		//BearingY = (pGlyphCon_Temp->Metrics.horiBearingY>>6);
		//Advance = (pGlyphCon_Temp->Metrics.horiAdvance>>6);

             	vBearingX = (pGlyphCon_Temp->Metrics.vertBearingX>>6);
		vBearingY = (pGlyphCon_Temp->Metrics.vertBearingY>>6);
		vAdvance = (pGlyphCon_Temp->Metrics.vertAdvance>>6);

		Width = (pGlyphCon_Temp->Metrics.width>>6);
		Height  = (pGlyphCon_Temp->Metrics.height>>6);

		//mpDebugPrint("BearingX =%d ,BearingY = %d,Advance = %d, vBearingX= %d, vBearingY = %d, vAdvance = %d,Width= %d, Height = %d",
		//BearingX, BearingY, Advance, vBearingX, vBearingY, vAdvance, Width, Height);

		}
		//----------------
		if (index==ttf_data->StringData.Str_Index_Start)
		{
			ttf_data->OutputData.StrHBearingXPixelFirstGlyph = BearingX;

			ttf_data->OutputData.StrHBearingYPixel = BearingY;

			if (index == ttf_data->StringData.Str_Index_Stop)
			{
				ttf_data->OutputData.RectangleSize.x = Width;
			}
			else
			{
				ttf_data->OutputData.RectangleSize.x = Advance - BearingX;
			}
			//
			n1 = (pos.y-BearingY);
			n2 = (pos.y-BearingY+Height);
		}


		//Decide output data and rectangle
		//if (BearingY > ttf_data->OutputData.StrHBearingYPixel)
		//	ttf_data->OutputData.StrHBearingYPixel = BearingY;

		if ((pos.y - BearingY) < n1)
			n1 = (pos.y-BearingY);

		if ((pos.y - BearingY +  Height) > n2)
			n2 = (pos.y-BearingY+Height);

		ttf_data->OutputData.RectangleSize.y = n2 - n1;
		//
		if ((index > ttf_data->StringData.Str_Index_Start) && (index < ttf_data->StringData.Str_Index_Stop))
		{
			ttf_data->OutputData.RectangleSize.x += Advance;
		}
		//
		if ((index != ttf_data->StringData.Str_Index_Start) && (index == ttf_data->StringData.Str_Index_Stop))
		{
			ttf_data->OutputData.RectangleSize.x += BearingX + Width;
		}

		if (BearingY > ttf_data->OutputData.StrHBearingYPixel)
			ttf_data->OutputData.StrHBearingYPixel = BearingY;
		//
		pos.x += Advance;

		ttf_data->OutputData.NextPos.x = pos.x;
		ttf_data->OutputData.NextPos.y = pos.y;
		//
		index++;
	}
	return result;
}
#endif

int MeasureString(stPrintTTFConfig* ttf_data)
{
int result = 1;
#if 0 //fix flow 20100712
	//
	int index = ttf_data->StringData.Str_Index_Start;
	int rt = 0;
	int length = 0;
	stPoint pos;
	//stPrintTTFConfig ConfigTemp;

	//Pre-set data
	pos.x =ttf_data->PrintCoordinate.PrintPos.x;
	pos.y = ttf_data->PrintCoordinate.PrintPos.y;
	ttf_data->OutputData.RectangleSize.x = 0;
	ttf_data->OutputData.RectangleSize.y = 0;

	//Check TTF config data
	if ((ttf_data->StringData.Str_Index_Start < 0)||
		(ttf_data->StringData.Str_Index_Stop < 0)||
		(ttf_data->StringData.Str_Index_Start>=ttf_data->StringData.StrLength)||
		(ttf_data->StringData.Str_Index_Stop>=ttf_data->StringData.StrLength)||
		(ttf_data->StringData.Str_Index_Start > ttf_data->StringData.Str_Index_Stop)
		)
	{
	mpDebugPrint("===> MeasureString   ERROR ====================");
		return 0;
	}

	//Add string to cache manager
	length = CacheManager_AddUnitArray(pCM, ttf_data, face,
                                       (long*)(ttf_data->StringData.Str + ttf_data->StringData.Str_Index_Start),
                                       (ttf_data->StringData.Str_Index_Stop - ttf_data->StringData.Str_Index_Start + 1));

	//Get line height
	if (ttf_data->PrintCoordinate.LineHeightPixel <= 0)
	{
		if ((ttf_data->StringData.StrLineHeightRef != NULL)&&
			(ttf_data->StringData.StrLineHeightRefLength >0))
		{
			stPrintTTFConfig* pConfigTemp = stPrintTTFConfig_Copy(ttf_data);
			//
			//replace string
			MEMORY_FREE(pConfigTemp->StringData.Str);
			pConfigTemp->StringData.Str = (long*)MEMORY_CALLOC(ttf_data->StringData.StrLineHeightRefLength, sizeof(long));
			memcpy(pConfigTemp->StringData.Str, ttf_data->StringData.StrLineHeightRef, sizeof(long) * ttf_data->StringData.StrLineHeightRefLength);
			pConfigTemp->StringData.StrLength = ttf_data->StringData.StrLineHeightRefLength;
			pConfigTemp->StringData.Str_Index_Start = 0;
			pConfigTemp->StringData.Str_Index_Stop = pConfigTemp->StringData.StrLength - 1;
			MeasureString_String(pConfigTemp);
			//ttf_data->PrintCoordinate.LineHeightPixel = pConfigTemp->OutputData.RectangleSize.y + pConfigTemp->StringData.StrLineHeightCompensate;
			//
			stPrintTTFConfig_Release(&pConfigTemp);
		}
	}
#endif
	//
	switch(ttf_data->MeasureMode)
	{
		case MEASURE_MODE_STRING:
			{
				MeasureString_String(ttf_data);
			}
			break;
		case MEASURE_MODE_RECTANGLE_FILLED:
			{
				MeasureString_RectangleFilled(ttf_data);
			}
			break;
		default:
			{
			}
			break;
	}

	//
	return result;
}

void test_x(stPrintTTFConfig* pttf_data)
{
	int i = 0;
	pGlyphContainer pGlyphCon = NULL;
	stGlyphContainerFeature feature;
	//
	for (i = pttf_data->StringData.Str_Index_Start; i<=pttf_data->StringData.Str_Index_Stop; i++)
	{
		GlyphContainerFeature_Config(&feature, pttf_data, face, pttf_data->StringData.Str[i]);
		pGlyphCon = CacheManager_RequestUnitGlyphBMP(pCM, &feature, WHILE_CACHE_MISS_DO_ADD);
	}
}

int FE_MeasureString(stPrintTTFConfig* pttf_data)
{
	int rt = 0;
#if  1 //fix flow 20100712
	//
	int length = 0;
	stPoint pos;
	long  * Str_temp;
	long StrLineHeightRef[] = {0x20,0x9b4f,'M', 'g',};
	int  StrLineHeightRef_length = 4,temp_start=0,temp_stop=0,strlen_temp=0;
	int StrLineHeightCompensate=4;

	//Pre-set data
	pos.x =pttf_data->PrintCoordinate.PrintPos.x;
	pos.y = pttf_data->PrintCoordinate.PrintPos.y;
	pttf_data->OutputData.RectangleSize.x = 0;
	pttf_data->OutputData.RectangleSize.y = 0;

	//Check TTF config data
	if ((pttf_data->StringData.Str_Index_Start < 0)||
		(pttf_data->StringData.Str_Index_Stop < 0)||
		(pttf_data->StringData.Str_Index_Start>=pttf_data->StringData.StrLength)||
		(pttf_data->StringData.Str_Index_Stop>=pttf_data->StringData.StrLength)||
		(pttf_data->StringData.Str_Index_Start > pttf_data->StringData.Str_Index_Stop)
		)
	{
		mpDebugPrint("===> FE_MeasureString   ERROR ====================");
		return 0;
	}

	CacheManager_InitCacheMemory(pCM, pttf_data);

	Str_temp=pttf_data->StringData.Str ;
	strlen_temp=pttf_data->StringData.StrLength;
	temp_start=pttf_data->StringData.Str_Index_Start;
	temp_stop=pttf_data->StringData.Str_Index_Stop ;

	pttf_data->StringData.Str=StrLineHeightRef;
	pttf_data->StringData.StrLength =StrLineHeightRef_length;
	pttf_data->StringData.Str_Index_Start = 0;
	pttf_data->StringData.Str_Index_Stop =pttf_data->StringData.StrLength- 1;

	MeasureString_String(pttf_data);

	pttf_data->PrintCoordinate.LineHeightPixel = pttf_data->OutputData.RectangleSize.y + StrLineHeightCompensate;
	pttf_data->OutputData.StrHBearingYPixel = pttf_data->OutputData.StrHBearingYPixel+4;

	pttf_data->StringData.Str=Str_temp;
	pttf_data->StringData.StrLength =strlen_temp;
	pttf_data->StringData.Str_Index_Start =temp_start;
	pttf_data->StringData.Str_Index_Stop =temp_stop;

#endif

	//MeasureString(pttf_data);//fix flow 20100712
	//rt = CacheManager_VerifyCacheMemory(pCM);

	return rt;
}

int FE_PrintString(stPrintTTFConfig* pttf_data)
{
	int rt = 0;

	PrintString(pttf_data);

	return rt;
}

void Test1()
{
	int rt = 0;
	FT_Error      error;
	//Test(pttf_config, face);
	FT_Glyph glyph1;
	FT_UInt glyph_index = FT_Get_Char_Index( face, 'A' );
	FT_Vector v;
	//--------------------------
	printf("123");
	error = FT_Init_FreeType( &library );
	if (error > 0)	rt = -1;
	error = FT_New_Face( library, "c:\\Font_1.TTF", 0, &face );
	if (error > 0)	rt = -1;

	v.x = v.y = 0;
	error = FT_Set_Char_Size(face, 0, 20*64, 300, 300 );
	error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
#if 1
	error = FT_Get_Glyph(face->glyph, &glyph1);
	error = FT_Glyph_To_Bitmap(&glyph1, FT_RENDER_MODE_NORMAL, &v, 0);
#else
	error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	error = FT_Get_Glyph(face->glyph, &glyph1);
#endif
	FT_Done_Glyph(glyph1);
	FT_Done_Face( face );
	FT_Done_FreeType( library );

}

void FE_Test2(int charcode, int size, int res, int* width, int* height, int* adv, int* bearX, int* bearY)
{
	FT_Error      error;
	FT_UInt glyph_index = FT_Get_Char_Index( face, charcode );
	error = FT_Set_Char_Size(face, 0, size*64, res, res );
	error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
	error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	//
	*width = ((face->glyph->metrics.width)>>6);
	*height = ((face->glyph->metrics.height)>>6);
	*adv = ((face->glyph->metrics.horiAdvance)>>6);
	*bearX = ((face->glyph->metrics.horiBearingX)>>6);
	*bearY = ((face->glyph->metrics.horiBearingY)>>6);
}


int CompactMetricsArray_Query(int size, int charcode, int* width, int* height, int* adv, int* bearX, int* bearY)
{
	int i=0;
	int k1 = -1;
	int k2 = -1;
	int offset = -1;
	//
	for (i=0; i<Compactmetrics_SizeData_SizeArrayLength; i++)
	{
		if (Compactmetrics_SizeData_SizeArray[i] == size)
		{
			k1 = i;
			break;
		}
	}
	if (k1 == -1)
	{
		*width = *height = *adv = *bearX = *bearY = -1;
		return -1;
	}
	//
	if ((charcode > Compactmetrics_SizeData_Charcode_End) || (charcode < Compactmetrics_SizeData_Charcode_Start))
	{
		*width = *height = *adv = *bearX = *bearY = -1;
		return -1;
	}

	k2 = charcode-Compactmetrics_SizeData_Charcode_Start;

	offset = (k1*(Compactmetrics_SizeData_Charcode_End-Compactmetrics_SizeData_Charcode_Start+1) + k2)*(Compactmetrics_SizeData_Unit_Length);
	*width = Compactmetrics_PreGenData[offset];
	*height = Compactmetrics_PreGenData[offset+1];
	*adv = Compactmetrics_PreGenData[offset+2];
	*bearX = Compactmetrics_PreGenData[offset+3];
	*bearY = Compactmetrics_PreGenData[offset+4];

	return 1;
}

FT_Face GetFace()
{
	return face;
}

stPrintTTFConfig* stPrintTTFConfig_Create()
{
	stPrintTTFConfig* pConfig = (stPrintTTFConfig*)ext_mem_malloc(sizeof(stPrintTTFConfig));
	stPrintTTFConfig_Init(pConfig);
	return pConfig;
}

pGlyphContainer FT_GetGlyphBmp(stPrintTTFConfig *pttf_data)
{
	int index;

	stGlyphContainerFeature feature;
	pGlyphContainer pGlyphCon_Temp;
	pGlyphContainer_LinkedList SearchTarget;

	memset(&feature,0,sizeof(stGlyphContainerFeature)); //cj add

	feature.FontSize = pttf_data->StringData.FontSize;
	feature.pFace = face;
	feature.RenderMode = pttf_data->Rendering;
	feature.Resolution.x = pttf_data->ImagePlaneData.DeviceResolution.x;
	feature.Resolution.y = pttf_data->ImagePlaneData.DeviceResolution.y;
	feature.CharCode = *pttf_data->StringData.Str;
	feature.FontRotation= pttf_data->FontRotation; //cj add

	CacheManager_InitCacheMemory(pCM, pttf_data);
	SearchTarget = (pCM->pMemory == NULL)?(pCM->pCacheList):(pGlyphContainer_LinkedList)(pCM->pMemory);

	if (GlyphContainerLinkedList_SearchGlyphFeature(SearchTarget, &feature)<0)
	{
		//CacheManager_VerifyCacheMemory(pCM);
		CacheManager_AddUnit(pCM, &feature, pCM->bAddDummy4BytesAlign);
	}

	pGlyphCon_Temp = CacheManager_RequestUnitGlyphBMP(pCM, &feature, WHILE_CACHE_MISS_DO_ADD);
	return pGlyphCon_Temp;
}

STREAM * ttf_fopen(unsigned char device_id,char *filename)
{
	STREAM *shandle;
	DRIVE *drv;
	SWORD swRet;
	char *ptr,*file;
	int ret;

	ptr = strchr(filename,'.');
	file = filename;
	*(file+ (ptr -filename)) = '\0';

	drv = DriveChange(device_id);
	if (DirReset(drv) != FS_SUCCEED)
		return NULL;

	ret = FileSearch(drv, file, "ttf",E_FILE_TYPE);
	if (ret != FS_SUCCEED)
	{
		return NULL;
	}
	else
	{
		shandle = (STREAM *)FileOpen(drv);
		if(!shandle)
			return NULL;
	}
	return shandle;
}

int FE_FontTTF_Init(char device_ID,char *ttffile,int fontsize,int FileType)
{
       STREAM *fp;
	FT_Error      error;
	int rt = 0;
	//DWORD ret;
#if 0
	if(pFontFileMemBuffer_Start)
	{
		ext_mem_free(pFontFileMemBuffer_Start);
		pFontFileMemBuffer_Start = NULL;
	}
#endif
	switch(FileType)
	{
		case FONT_DATA_AT_MEMORY:
			//memcpy(pFontFileMemBuffer_Start,ttffile,fontsize);
			pFontFileMemBuffer_Start =ttffile;
			pFontFileMemBuffer_Size = fontsize;

			error = FT_Init_FreeType( &library );
			if (error > 0)
				rt = -1;
			error = FT_New_Memory_Face( library, /*BASE*/pFontFileMemBuffer_Start, /*SIZE*/pFontFileMemBuffer_Size, 0, &face );

			if (error > 0)
				rt = -1;
			break;

  		case FONT_DATA_AT_FILE:

			fp = (STREAM *)ttf_fopen(device_ID,ttffile);
			if(!fp)
				return -1;

			fontsize = FileSizeGet(fp);
			if(fontsize == 0)
				return -1;

			pFontFileMemBuffer_Start = ext_mem_malloc(sizeof(char)*fontsize);
			pFontFileMemBuffer_Size = fontsize;
			FileRead(fp, pFontFileMemBuffer_Start, pFontFileMemBuffer_Size);
                     FileClose(fp);
			error = FT_Init_FreeType( &library );
			if (error > 0)
				rt = -1;
			error = FT_New_Memory_Face( library, /*BASE*/pFontFileMemBuffer_Start, /*SIZE*/pFontFileMemBuffer_Size, 0, &face );
			if (error > 0)
				rt = -1;
			break;
	}

	//ret = FT_HAS_VERTICAL( face );

	//mpDebugPrint(" Has vertical = %x",ret);

	//Initialize CacheManager
	pCM = CacheManager_Create(NULL, NULL, NULL, 0x0);

	return rt;
}

int FE_FontTTF_test()
{
	int rt =0;
	char *font = "Font_1.tff";

	rt = FE_FontTTF_Init(SD_MMC,font,0,FONT_DATA_AT_FILE);
	if(rt<0)
	{
		pFontFileMemBuffer_Start = NULL;
		pFontFileMemBuffer_Size = 0;
		mpDebugPrint("Please reload TTF file for Freetype");
	}
	else
	{
		mpDebugPrint("The default TTF file is %s",font);
	}
	return rt;
}

#if 1
int FE_MPX_Drawfont(char* string,int FontSize,ST_IMGWIN* pDisplay ,int StartX,int StartY,int R,int G,int B,int FontRotation)
{
	stPrintTTFConfig *ttf_config= (stPrintTTFConfig*)ext_mem_malloc(sizeof(stPrintTTFConfig));
	int str_length = 0,len=0;
	long* str=NULL;
	WORD *wTemp=NULL;
	
	long DelimiterList[] = {',', ';', ' ', '.', '?', ':', '/'};
	int  DelimiterListLength = 7;
	long str7[] = {'M', 'g'};
	//
#if 1		
	len=strlen(string);
	str= (long *)ext_mem_malloc(sizeof(long)*len);
		while(len--)
		{
			str[str_length]=string[str_length];
			str_length++;
		}
#else
	len=strlen(string);
	mpDebugPrint(" 22 string=%x len=%d ",string,len);
	wTemp = (WORD *)ext_mem_malloc(sizeof(WORD)*len+2);
	memset(wTemp,0,sizeof(WORD)*len+2);
	len = EreaderUTF82Unicode2(string,wTemp);
	mpDebugPrint(" 33  EreaderUTF82Unicode2 wTemp=%x len=%d ",wTemp,len);
	str= (long *)ext_mem_malloc(sizeof(long)*len);
	memset(str,0,sizeof(long)*len);

	while(len--)
		{
			str[str_length]=wTemp[str_length];
			str_length++;
		}
	ext_mem_free(wTemp);
#endif

	//mpDebugPrint("str=%x str_length=%d ",str,str_length);
	//mpDebugPrint(" FE_MPX_Drawfont StartX=%d StartY=%d ",StartX,StartY);
	//mpDebugPrint(" FE_MPX_Drawfont R=%d G=%d B=%d",R,G,B);

	stPrintTTFConfig_Init(ttf_config);	

	ttf_config->ImagePlaneData.ImagePlaneMode = COLOR_FORMAT_YYCBCR;
	ttf_config->ColorParameter_Glyph.Format = COLOR_FORMAT_RGB;
	ttf_config->ColorParameter_Glyph.Data.color_1 = R;
	ttf_config->ColorParameter_Glyph.Data.color_2 = G;
	ttf_config->ColorParameter_Glyph.Data.color_3 = B;
	ttf_config->ColorParameter_Glyph.Alpha=0; //??
	ttf_config->ColorParameter_Rectangle.Format = COLOR_FORMAT_RGB;
	ttf_config->ColorParameter_Rectangle.Data.color_1 = 255;
	ttf_config->ColorParameter_Rectangle.Data.color_2 = 0;
	ttf_config->ColorParameter_Rectangle.Data.color_3 = 0;
	ttf_config->ColorParameter_Baseline.Format = COLOR_FORMAT_RGB;
	ttf_config->ColorParameter_Baseline.Data.color_1 = 255;
	ttf_config->ColorParameter_Baseline.Data.color_2 = 0;
	ttf_config->ColorParameter_Baseline.Data.color_3 = 255;
	ttf_config->ColorParameter_PrintRegion.Format = COLOR_FORMAT_RGB;
	ttf_config->ColorParameter_PrintRegion.Data.color_1 = 255;
	ttf_config->ColorParameter_PrintRegion.Data.color_2 = 255;
	ttf_config->ColorParameter_PrintRegion.Data.color_3 = 0;
	ttf_config->GlyphSetMode = GLYPH_SET_MODE_GLYPH;
	//ttf_config->GlyphSetMode = GLYPH_SET_MODE_DELIMITER;
	//ttf_config.GlyphSetMode = GLYPH_SET_MODE_WHOLESTRING;
	ttf_config->RectangleOn = 0;
	ttf_config->BaselineOn = 0;
	ttf_config->PrintRegionOn = 0;
	ttf_config->BlendingOn = 1;
	//ttf_config.Rendering = FT_RENDER_MODE_NORMAL;
	ttf_config->Rendering =FT_RENDER_MODE_MONO;
	
	ttf_config->Encoding = FT_ENCODING_UNICODE;
	ttf_config->StringData.Str = str;
	ttf_config->StringData.StrLength = str_length;
	ttf_config->StringData.DelimiterList = DelimiterList;
	ttf_config->StringData.DelimiterListLength = DelimiterListLength;
	ttf_config->StringData.FontSize = FontSize;
	
	ttf_config->StringData.Str_Index_Start = 0;
	ttf_config->StringData.Str_Index_Stop = ttf_config->StringData.StrLength - 1;
	ttf_config->StringData.StrLineHeightRef = str7;
	ttf_config->StringData.StrLineHeightCompensate = 2;
	ttf_config->StringData.StrLineHeightRefLength = sizeof(str7) / sizeof(long);
	ttf_config->CacheSetting.CacheOn = 1;
	ttf_config->CacheSetting.Capacity = 0;
	ttf_config->CacheSetting.CacheSpaceStyle = CACHE_SPACE_STYLE_STATIC_ALLOCATED;
//	ttf_config.CacheSetting.CacheSpaceStyle = CACHE_SPACE_STYLE_RANDOM;
	ttf_config->CacheSetting.MemorySize = 0x60000;
//	ttf_config.CacheSetting.MemorySize = 0x000400;
	ttf_config->CacheSetting.pMemory = NULL;
	ttf_config->CacheSetting.bAddDummy4BytesAlign = 1;
	//----------------------------------
	//Init FontRotation
	ttf_config->FontRotation=FontRotation;
	ttf_config->RotationFontOrderDirection=0;

	//Init imageplane

	//Set Image Plane related parameters
	ttf_config->ImagePlaneData.ImagePlane = pDisplay->pdwStart;
	ttf_config->ImagePlaneData.ImagePlaneSize.x = pDisplay->wWidth;
	ttf_config->ImagePlaneData.ImagePlaneSize.y = pDisplay->wHeight;
	ttf_config->ImagePlaneData.DeviceResolution.x = 72;
	ttf_config->ImagePlaneData.DeviceResolution.y = 72;

	//Measure the string
	ttf_config->PrintCoordinate.PrintRegion.pt_1.x =0;
	ttf_config->PrintCoordinate.PrintRegion.pt_1.y = 0;
	ttf_config->PrintCoordinate.PrintRegion.pt_2.x = pDisplay->wWidth;
	ttf_config->PrintCoordinate.PrintRegion.pt_2.y = pDisplay->wHeight;

	//Print in image-plane or print-region
	//ttf_config.PrintCoordinate.PrintRegionMode = PRINT_REGION_MODE_IMAGE_PLANE;
	ttf_config->PrintCoordinate.PrintRegionMode = PRINT_REGION_MODE_PRINT_REGION;
	//The origin is on the upper left corner of image-plane or print region	
	ttf_config->PrintCoordinate.CoordinateOriginMode = COORDINATE_ORIGIN_MODE_IMAGE_PLANE;
	ttf_config->MeasureMode = MEASURE_MODE_STRING;

	ttf_config->PrintCoordinate.PrintPos.x = StartX;
	ttf_config->PrintCoordinate.PrintPos.y = StartY;
	
	FE_MeasureString(ttf_config);

	//Decide X pos alignment while changing line.
	ttf_config->PrintCoordinate.ChangeLineXAlignMode = CHANGE_LINE_X_ALIGN_MODE_SPECIFIED_X;
	ttf_config->PrintCoordinate.SpecifiedChangeLineX = ttf_config->PrintCoordinate.PrintRegion.pt_1.x;

	//Not draw over border
	ttf_config->PrintCoordinate.NotDrawOverBorder = NOT_DRAW_OVER_BORDER_TOP | NOT_DRAW_OVER_BORDER_BOTTOM | NOT_DRAW_OVER_BORDER_LEFT | NOT_DRAW_OVER_BORDER_RIGHT;
	//ttf_config.PrintCoordinate.NotDrawOverBorder = NOT_DRAW_OVER_BORDER_TOP;
	//ttf_config.PrintCoordinate.NotDrawOverBorder = 0;

	ttf_config->PrintStringOn = 1;
	FE_LayoutString(ttf_config);
	ext_mem_free(ttf_config->StringData.Str );
	ext_mem_free(ttf_config);

	//---------------------------------------
	return 0;
}
#endif
