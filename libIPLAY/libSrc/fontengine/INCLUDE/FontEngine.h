#ifndef __FONTENGINE_H__
#define __FONTENGINE_H__

//#define PC_DEVELOPMENT
#define TARGET_DEVELOPMENT
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//The following is identical between target and host
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

//----------------------------------

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_IMAGE_H
#include FT_GLYPH_H

#include "Basic_Struct.h"
#include "Basic_Enum.h"
#include "GlyphContainer.h"
#include "GlyphContainerLinkedList.h"
#include "CacheManager.h"
#include "PrintTTFConfig.h"
//----------------------------------
#define BASIC_CODE
//#define REFINED_CODE

#ifdef TARGET_DEVELOPMENT
#include "FontEngine.h"
#include "global612.h"
#include "mpTrace.h"
#include "devio.h"
#include "display.h"
#include "mpapi.h"
#include "flagdefine.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#endif




#define STGLYPHBMPDATA_FACENAME_LENGTH 16

#ifdef PC_DEVELOPMENT
#define MEMORY_MALLOC                  malloc
#define MEMORY_CALLOC                  calloc
#define MEMORY_FREE                    free
#define TIME_PING(TIME)                TIME = clock();
#define TIME_PONG(TIME)                {CString str;str.Format(L">>> Expanding Time = %d ms.\r\n", clock()-TIME);OutputDebugString(str);}
#define TIME_FORMAT                    long
#define DEBUG_PRINT_STRING             DebugPrintLine
#define NT_HEAP_MANAGER_FREEED_MEMORY  0xFEEEFEEE
#define PLATFORM_FREEED_MEMORY         NT_HEAP_MANAGER_FREEED_MEMORY

extern FT_Library    library;
extern FT_Face       face;

#endif

#ifdef TARGET_DEVELOPMENT
#define MEMORY_MALLOC                  ext_mem_malloc
#define MEMORY_CALLOC                  ext_mem_calloc
#define MEMORY_FREE                    ext_mem_free
#define TIME_PING(TIME)                TIME = GetSysTime();
#define TIME_PONG(TIME)                {mpDebugPrint(">>> Expanding Time = %d ms.", SystemGetElapsedTime(TIME));}
#define TIME_FORMAT                    DWORD
#define DEBUG_PRINT_STRING             mpDebugPrint
#define PLATFORM_FREEED_MEMORY         0x00000000
#define IS_FREE(MEMORY)                 ((MEMORY == NULL) || (MEMORY == PLATFORM_FREEED_MEMORY))

static long FontFileResourceTag = 0x464F4E54;
//static unsigned int *pFontFileMemBuffer_Start = NULL;
static unsigned char* pFontFileMemBuffer_Start = NULL;
static unsigned int pFontFileMemBuffer_Size;
static long *pdwStream = NULL;
static STREAM* hFontFile = NULL;

FT_Library    library;
FT_Face       face;

//#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
//#define memset          mmcp_memset
//#define memcpy          mmcp_memcpy
//#else
//#define memset          MpMemSet
//#define memcpy          MpMemCopy
//#endif

#endif



//----------------------------------
//Internal Function Prototype
void message_font_bmp();
void message_font_bmp_data();
void message_font_memory();
int  DisplayDrawGlyphBMP_YYCBCR_V(stPrintTTFConfig* ttf_data, stPoint pt, pGlyphContainer pGlyphCon);
int  DisplayDrawGlyphBMP_YYCBCR_H(stPrintTTFConfig* ttf_data, stPoint pt, pGlyphContainer pGlyphCon);
int  DisplayDrawGlyphBMP_RGB(stPrintTTFConfig* ttf_data, stPoint pt, pGlyphContainer pGlyphCon);
void Print_String_To_Image(stPrintTTFConfig* ttf_data, stPoint pt);
int  Get_Char_BMP(stPrintTTFConfig* ttf_data, unsigned int charcode, pGlyphContainer pGlyphBMPData);
int  Get_Char_BMP2(stPrintTTFConfig* ttf_data, unsigned int charcode, pGlyphContainer pGlyphCon);
int  DisplayDrawGlyphBMP(stPrintTTFConfig* ttf_data, stPoint pt, pGlyphContainer pGlyphBMPData);

int  CheckInside(stPoint pt, stRectangle rec);
int  DisplayDrawPixel_RGB(stImagaPlaneParameter* pImagaPlaneParameter, stColorParameter* pColorParameter, stPoint pt);
int  DisplayDrawPixel_YYCBCR(stImagaPlaneParameter* pImagaPlaneParameter, stColorParameter* pColorParameter, stPoint pt);
int  DisplayDrawLine_RGB(stImagaPlaneParameter* ImagaPlaneParameter, stColorParameter* pColorParameter, stPoint pt1, stPoint pt2);
int  DisplayDrawLine_YYCBCR(stImagaPlaneParameter* ImagaPlaneParameter, stColorParameter* pColorParameter, stPoint pt1, stPoint pt2);
int  DisplayDrawLine(stImagaPlaneParameter* ImagaPlaneParameter, stColorParameter* pColorParameter, stPoint pt1, stPoint pt2);
int  DisplayDrawRectangle_RGB(stImagaPlaneParameter* ImagaPlaneParameter, stColorParameter* pColorParameter, stRectangle rec);
int  DisplayDrawRectangle_YYCBCR(stImagaPlaneParameter* ImagaPlaneParameter, stColorParameter* pColorParameter, stRectangle rec);
int  DisplayDrawRectangle(stImagaPlaneParameter* ImagaPlaneParameter, stColorParameter* pColorParameter, stRectangle rec);
int  NormalizeRectangle(stRectangle* rec);
int  Swap_int(int* a, int* b);

int  ClearGlyph_List();
int  GetStringGlyphList(stPrintTTFConfig* ttf_data);
int  RenderPrintStringGlyphList(stPrintTTFConfig* ttf_data);
int  GetStringRectangle(FT_BBox*  bbox);

int  PrintString(stPrintTTFConfig* ttf_data);
int  PrintString_UseMeasurePregen(stPrintTTFConfig* ttf_data);
int  PrintString_Immediately(stPrintTTFConfig* ttf_data);

int  Init_stPoint(stPoint* pPoint);


int  Translate_Color(stColorData ColorDataIn, int ColorIn_Format, stColorData* ColorDataOut, int ColorOut_Format);

int  GetNextSubStringLength(stPrintTTFConfig* ttf_data, int index_now, int* NextSubStringLength);
int  GetNextDelimiterIndex(stStrParameter* StringData, int index_now, int* NextDelimiterIndex);


int  stPrintTTFConfig_Init(stPrintTTFConfig* pConfig);
int  stPrintTTFConfig_Release(stPrintTTFConfig** ppConfig);
stPrintTTFConfig* stPrintTTFConfig_Copy(stPrintTTFConfig* Source);
stPrintTTFConfig* stPrintTTFConfig_Create();
int  stPrintTTFConfig_Normalize(stPrintTTFConfig* pConfig);
int  stPrintTTFConfig_ResetOutputData(stPrintTTFConfig* pConfig);


int MeasureString(stPrintTTFConfig* ttf_data);
int MeasureString_String(stPrintTTFConfig* ttf_data);

int CompactMetricsArray_Query(int size, int charcode, int* width, int* height, int* adv, int* bearX, int* bearY);

/////////////////////////////////////////////////////////////
//External Function Prototype
int FE_LayoutString(stPrintTTFConfig* ttf_data);
int FE_LayoutString2(stPrintTTFConfig* ttf_data);
int FE_PrintString(stPrintTTFConfig* ttf_config);
int FE_Init(int Font_File_Source_Option);
int FE_Release();
int FE_MeasureString(stPrintTTFConfig* ttf_config);

#ifdef PC_DEVELOPMENT
DWORD RGB2YUV(BYTE R, BYTE G, BYTE B);
void DebugPrintLine(char* fmt,...);
void FE_Test1();
void FE_Test2(int charcode, int size, int res, int* width, int* height, int* adv, int* bearX, int* bearY);
#endif

#ifdef TARGET_DEVELOPMENT
int  Font_Data_At_File_Get_List(unsigned char bMcardId, DWORD dwSearchCount);
int  OpenFontFileMemBuffer(DWORD ResourceTag);
#endif

//---------------------------------
// RC4 Test
void RC4_Run();
int RC4_Test(unsigned char* Key, unsigned char* TextIn, unsigned char* TextOut, int BufferLength);
void output_array_data_hex(unsigned char* data, int length);
unsigned char rc4_output(unsigned char *S, unsigned int* i, unsigned int* j);
void rc4_init(unsigned char *S, unsigned char *key, unsigned int key_length);
void rc4_swap(unsigned char *s, unsigned int i, unsigned int j);
//----------------------------------

#endif  //__FONTENGINE_H__

