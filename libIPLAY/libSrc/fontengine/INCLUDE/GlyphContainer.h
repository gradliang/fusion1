#ifndef __GLYPHCONTAINER_H__
#define __GLYPHCONTAINER_H__

#include "Basic_Struct.h"
#include "Basic_Enum.h"
#include "PrintTTFConfig.h"

//
typedef enum  STGLYPHBMPDATA_CONTENT_
{
	STGLYPHBMPDATA_CONTENT_EMPTY,
	STGLYPHBMPDATA_CONTENT_GLYPH,
	STGLYPHBMPDATA_CONTENT_GLYPH_BITMAP
} STGLYPHBMPDATA_CONTENT;

//Result enum
typedef enum  _GLYPH_CONTAINER_FUNC_RESULT
{
	//SUCCESSFUL
	GLYPH_CONTAINER_FUNC_RESULT_SUCCESSFUL = 0x0,
	//ERROR
	GLYPH_CONTAINER_FUNC_RESULT_ERROR = 0xFF,
	GLYPH_CONTAINER_FUNC_RESULT_ERROR_CACHE_NULL = 0x1,
	GLYPH_CONTAINER_FUNC_RESULT_ERROR_FACE_NULL = 0x2
} GLYPH_CONTAINER_FUNC_RESULT;



//
typedef struct stGlyphContainer_* pGlyphContainer;
typedef struct stGlyphContainerInfo_* pGlyphContainerInfo;
typedef struct stGlyphContainerFeature_* pGlyphContainerFeature;


typedef struct stGlyphContainerFeature_
{
	FT_Face					pFace;
	unsigned int			FontSize;
	unsigned int			CharCode;
	stSize                  Resolution;
	FT_Render_Mode          RenderMode;
	int		     FontRotation;     //0,90,180,270 
}stGlyphContainerFeature, *pstGlyphContainerFeature;

typedef struct stGlyphContainerInfo_
{
	//unsigned int			GlyphIndex;
	unsigned int			ReferenceCount;
	//int						BitPerPixel;
}stGlyphContainerInfo, *pstGlyphContainerInfo;


typedef struct stGlyphContainer_
{
	stGlyphContainerInfo    Info;
	stGlyphContainerFeature Feature;
	FT_Glyph_Metrics		Metrics;
	unsigned char*		pFontBitmap;
	unsigned int			FontBitmapSize;
	FT_GlyphSlot 			Slot;    //cj add 04262010
	int                               glyph_index; //cj add 04262010
}stGlyphContainer, *pstGlyphContainer;



////Function prototype
//pGlyphContainerInfo
pGlyphContainerInfo GlyphContainerInfo_Create(stPrintTTFConfig* pttf_config, FT_Face pFace, unsigned int charcode);
int GlyphContainerFeature_Init(pGlyphContainer pGlyphCon, pGlyphContainerFeature pGlyphFeature);
int GlyphContainerFeature_Compare(pGlyphContainerFeature pGlyphFeature1, pGlyphContainerFeature pGlyphFeature2);
int GlyphContainerFeature_Config(pGlyphContainerFeature pGlyphFeature, stPrintTTFConfig* pttf_config, FT_Face pFace, long charcode);
int GlyphContainerInfo_Compare(pGlyphContainerFeature pGlyphFeature, stPrintTTFConfig* pttf_config, FT_Face pFace, long charcode);
//pGlyphContainer
pGlyphContainer GlyphContainer_Create(pGlyphContainerFeature pGlyphFeature);
void GlyphContainer_Release(pGlyphContainer* ppGlyphCon);
unsigned char* GlyphContainer_GetGlyphBMP(pGlyphContainer pGlyphCon);
FT_Glyph GlyphContainer_GetGlyph(pGlyphContainer pGlyphCon);
int GlyphContainer_CompareInfo(pGlyphContainer pGlyphCon1, pGlyphContainer pGlyphCon2);


int GlyphContainer_Init(pGlyphContainer pGlyphCon, stPrintTTFConfig* pttf_config, FT_Face pFace, long charcode);
//

#endif	//__GLYPHBMPDATA_H__

