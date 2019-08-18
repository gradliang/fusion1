/*
#include "stdafx.h"
#include "FontEngine.h"
*/

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

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//The following is identical between target and host
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

#include "GlyphContainer.h"

int GlyphContainerFeature_Config(pGlyphContainerFeature pGlyphFeature, stPrintTTFConfig* pttf_config, FT_Face pFace, long charcode)
{
	int result = 1;
	//
	if (pGlyphFeature == NULL)
		return 0;
	//
	pGlyphFeature->CharCode = charcode;
	pGlyphFeature->FontSize = pttf_config->StringData.FontSize;
	pGlyphFeature->pFace = pFace;
	pGlyphFeature->RenderMode = pttf_config->Rendering;
	pGlyphFeature->Resolution.x = pttf_config->ImagePlaneData.DeviceResolution.x;
	pGlyphFeature->Resolution.y = pttf_config->ImagePlaneData.DeviceResolution.y;
	pGlyphFeature->FontRotation= pttf_config->FontRotation;

	//
	return result;
}


int GlyphContainerFeature_Init(pGlyphContainer pGlyphCon, stGlyphContainerFeature* pGlyphFeature)
{
	int result = 1;
	//
	if (pGlyphCon == NULL)
		return 0;

	if (pGlyphFeature == NULL)
	{
		pGlyphCon->Feature.CharCode = 0;
		pGlyphCon->Feature.FontSize = 0;
		pGlyphCon->Feature.pFace = NULL;
		pGlyphCon->Feature.RenderMode = FT_RENDER_MODE_NORMAL;
		pGlyphCon->Feature.Resolution.x = 0;
		pGlyphCon->Feature.Resolution.y = 0;
		pGlyphCon->Feature.FontRotation=0;
	}
	else
	{
		pGlyphCon->Feature.CharCode = pGlyphFeature->CharCode;
		pGlyphCon->Feature.FontSize = pGlyphFeature->FontSize;
		pGlyphCon->Feature.pFace = pGlyphFeature->pFace;
		pGlyphCon->Feature.RenderMode = pGlyphFeature->RenderMode;
		pGlyphCon->Feature.Resolution.x = pGlyphFeature->Resolution.x;
		pGlyphCon->Feature.Resolution.y = pGlyphFeature->Resolution.y;
		pGlyphCon->Feature.FontRotation= pGlyphFeature->FontRotation;
	}

	//
	return result;
}

pGlyphContainer GlyphContainer_Create(stGlyphContainerFeature* pGlyphFeature)
{
	pstGlyphContainer pCon_temp = (pstGlyphContainer)MEMORY_MALLOC(sizeof(stGlyphContainer));
	//
	GlyphContainerFeature_Init(pCon_temp, pGlyphFeature);
	//
	pCon_temp->Metrics.height = 0;
	pCon_temp->Metrics.horiAdvance = 0;
	pCon_temp->Metrics.horiBearingX = 0;
	pCon_temp->Metrics.horiBearingY = 0;
	pCon_temp->Metrics.vertAdvance = 0;
	pCon_temp->Metrics.vertBearingX = 0;
	pCon_temp->Metrics.vertBearingY = 0;
	pCon_temp->Metrics.width = 0;
	//
	pCon_temp->Info.ReferenceCount = 0;
	//
	return pCon_temp;
}

void GlyphContainer_Release(pGlyphContainer* ppGlyphCon)
{
	if ((*ppGlyphCon) != NULL)
	{
		MEMORY_FREE((*ppGlyphCon)->pFontBitmap);
		MEMORY_FREE((*ppGlyphCon));
		(*ppGlyphCon ) = NULL;
	}
}

BYTE* GlyphContainer_GetGlyphBMP(pGlyphContainer pGlyphCon)
{
	if (pGlyphCon == NULL)
		return NULL;

	return pGlyphCon->pFontBitmap;
}


int GlyphContainerFeature_Compare(pGlyphContainerFeature pGlyphFeature1, pGlyphContainerFeature pGlyphFeature2)
{
	int result = 1;
	//
	if ((pGlyphFeature1 == NULL) || (pGlyphFeature2 == NULL))
		return 0;
	//
	if (pGlyphFeature1->CharCode != pGlyphFeature2->CharCode)	return 0;
	if (pGlyphFeature1->FontSize != pGlyphFeature2->FontSize)	return 0;
	if (pGlyphFeature1->pFace != pGlyphFeature2->pFace)	return 0;
	if (pGlyphFeature1->Resolution.x != pGlyphFeature2->Resolution.x)	return 0;
	if (pGlyphFeature1->Resolution.y != pGlyphFeature2->Resolution.y)	return 0;
	if (pGlyphFeature1->RenderMode != pGlyphFeature2->RenderMode)	return 0;
	if (pGlyphFeature1->FontRotation!= pGlyphFeature2->FontRotation)	return 0;

	//
	return result;
}


int GlyphContainer_CompareFeature(pGlyphContainer pGlyphCon1, pGlyphContainer pGlyphCon2)
{
	int result = 1;
	//
	if (pGlyphCon1 == NULL)
		return 0;
	if (pGlyphCon2 == NULL)
		return 0;
	//
	result = GlyphContainerFeature_Compare(&(pGlyphCon1->Feature), &(pGlyphCon2->Feature));

	//
	return result;
}





//--------------------------------------------------------------------



