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

pGlyphContainer_LinkedList GlyphContainerLinkedList_Create(pGlyphContainer pGlyphCon)
{
	pGlyphContainer_LinkedList pList = (pGlyphContainer_LinkedList)MEMORY_MALLOC(sizeof(stGlyphContainer_LinkedList));
	//
	pList->next = NULL;
	pList->index = 0;
	pList->pGlyphCon = (pGlyphCon == NULL)?NULL:pGlyphCon;
	pList->AlignDummyLength = 0;
	pList->pAlignDummy = NULL;
	//
	return pList;
}

int GlyphContainerLinkedList_Length(pGlyphContainer_LinkedList pList)
{
	int length = 0;
	pGlyphContainer_LinkedList curr = pList;
	//
	while(curr != NULL)
	{
		length++;
		curr = curr->next;
	}
	//
	return length;
}

void GlyphContainerLinkedList_Release(pGlyphContainer_LinkedList* ppGlyphConList)
{
	if ((*ppGlyphConList) != NULL)
	{
		pGlyphContainer_LinkedList pCurr = (*ppGlyphConList);
		pGlyphContainer_LinkedList pNext = NULL;

		while(pCurr != NULL)
		{
			pNext = pCurr->next;
			if (pCurr->pGlyphCon != NULL)
			{
				GlyphContainer_Release(&(pCurr->pGlyphCon));
			}

			//Release Align Dummy
			if ((*ppGlyphConList)->pAlignDummy != NULL)
			{
				MEMORY_FREE((*ppGlyphConList)->pAlignDummy);
			}

			//
			MEMORY_FREE(pCurr);
			pCurr = pNext;
		}
	}
	(*ppGlyphConList) = NULL;
}

pGlyphContainer_LinkedList GlyphContainerLinkedList_GetTail(pGlyphContainer_LinkedList pList)
{
	pGlyphContainer_LinkedList pTail = pList;
	//
	if (pList == NULL)
		return NULL;
	//
	while(pTail->next != NULL)
	{
		pTail = pTail->next;
	}

	//
	return pTail;
}

int GlyphContainerLinkedList_Size(pGlyphContainer_LinkedList pList, int bAddDummy4BytesAlign)
{
	int size = 0;

	if (pList == NULL)	return -1;
	if (pList->pGlyphCon == NULL)	return -1;
	if (pList->pGlyphCon->pFontBitmap == NULL)	return -1;

	size += sizeof(stGlyphContainer_LinkedList);
	size += sizeof(stGlyphContainer);
	size += pList->pGlyphCon->FontBitmapSize;
	size += pList->AlignDummyLength;


	return size;
}

int GlyphContainerLinkedList_SearchGlyphFeature(pGlyphContainer_LinkedList pList, pGlyphContainerFeature pFeature)
{
	int i=0;
	pGlyphContainer_LinkedList pList_temp = pList;
	int index = -1;
	int index2 = -1;
	//
	if (pList == NULL)
		return -1;
	if (pFeature == NULL)
		return -1;
	//
	while(pList_temp != NULL)
	{
		if (pList_temp->pGlyphCon == NULL)
			return -1;

		index++;
		//if (GlyphContainer_CompareInfo(pList->pGlyphCon, pGlyphCon) == 1)
		if (GlyphContainerFeature_Compare(&(pList_temp->pGlyphCon->Feature), pFeature) == 1)
		{
			index2 = index;
			break;
		}
		pList_temp = pList_temp->next;

	}

	return index2;
}

int GlyphContainerLinkedList_SearchGlyphCon(pGlyphContainer_LinkedList pList, pGlyphContainer pGlyphCon)
{
	int i=0;
	pGlyphContainer_LinkedList pList_temp = pList;
	int index = -1;
	int index2 = -1;
	//
	if (pList == NULL)
		return -1;
	if (pGlyphCon == NULL)
		return -1;
	//
	while(pList_temp != NULL)
	{
		index++;
		//if (GlyphContainer_CompareInfo(pList->pGlyphCon, pGlyphCon) == 1)
		if (GlyphContainerFeature_Compare(&(pList_temp->pGlyphCon->Feature), &(pGlyphCon->Feature)) == 1)
		{
			index2 = index;
			break;
		}
		pList_temp = pList_temp->next;
	}

	return index2;
}

int GlyphContainerLinkedList_AddUnit2(pGlyphContainer_LinkedList* ppList, pGlyphContainer pGlyphCon)
{
	int result = 0;
	pGlyphContainer_LinkedList pList2 = NULL;
	int index = -1;

	//
	if (pGlyphCon == NULL)
		return 0;
	//
	if ((*ppList) == NULL)
	{
		(*ppList) = GlyphContainerLinkedList_Create(pGlyphCon);
		result = 1;
	}
	else
	{
		index = GlyphContainerLinkedList_SearchGlyphCon((*ppList), pGlyphCon);
		if (index < 0)
		{
			//pList2 = GlyphContainerLinkedList_Create(pGlyphCon);
			//(*ppList)->Funcs.pFuncGlyphContainerLinkedList_Concatenate((*ppList), pList2);
			GlyphContainerLinkedList_AddUnit2(ppList, pGlyphCon);
			result = 1;
		}
	}
	//
	return result;
}

int GlyphContainerLinkedList_MoveUnit(pGlyphContainer_LinkedList* ppList_Source, pGlyphContainer_LinkedList pList_Target)
{
	int result = 0;
	int size_temp = 0;
	//
	size_temp = sizeof(stGlyphContainer_LinkedList);
	memcpy(pList_Target, *ppList_Source, size_temp);
	pList_Target->pGlyphCon = (pGlyphContainer)((unsigned char*)pList_Target + size_temp);
	//
	size_temp = sizeof(stGlyphContainer);
	memcpy(pList_Target->pGlyphCon, (*ppList_Source)->pGlyphCon, size_temp);
	pList_Target->pGlyphCon->pFontBitmap = ((unsigned char*)pList_Target->pGlyphCon + size_temp);
	//

	//pList_Target->next = (pGlyphContainer_LinkedList)((unsigned char*)pList_Target->pGlyphCon->pFontBitmap + pList_Target->pGlyphCon->FontBitmapSize);
	memcpy(pList_Target->pGlyphCon->pFontBitmap, (*ppList_Source)->pGlyphCon->pFontBitmap, pList_Target->pGlyphCon->FontBitmapSize);
	pList_Target->next = NULL;
	//
	pList_Target->pAlignDummy = (unsigned char*)pList_Target->pGlyphCon->pFontBitmap + pList_Target->pGlyphCon->FontBitmapSize;
	memcpy(pList_Target->pAlignDummy, (*ppList_Source)->pAlignDummy, pList_Target->AlignDummyLength);
	//
	GlyphContainerLinkedList_Release(ppList_Source);

	return result;
}

//int GlyphContainerLinkedList_AddUnit(pGlyphContainer_LinkedList* ppList, pGlyphContainerFeature pGlyphFeature, int bAddDummy4BytesAlign)

#if 0
int GlyphContainerLinkedList_AddUnit(pCacheManager pCM, pGlyphContainerFeature pGlyphFeature, int bAddDummy4BytesAlign)
{
	FT_Error      error;
	FT_UInt glyph_index = 0;
	FT_GlyphSlot slot = pGlyphFeature->pFace->glyph;
	int result = 0;
	pGlyphContainer pGlyphCon = NULL;
	pGlyphContainer_LinkedList pList2 = NULL;
	int index;
	//
	if (pCM->pMemory == NULL)
	{
		index = GlyphContainerLinkedList_SearchGlyphFeature(pCM->pCacheList, pGlyphFeature);
	}
	else
	{
		index = GlyphContainerLinkedList_SearchGlyphFeature((pGlyphContainer_LinkedList)(pCM->pMemory), pGlyphFeature);
	}

	if (index >= 0)
	{
		return result;
	}
	//
	pGlyphCon = GlyphContainer_Create(pGlyphFeature);

	glyph_index = FT_Get_Char_Index( pGlyphFeature->pFace, pGlyphFeature->CharCode );
	error = FT_Set_Char_Size(pGlyphFeature->pFace, 0, pGlyphFeature->FontSize*64, pGlyphFeature->Resolution.x, pGlyphFeature->Resolution.y);
	error = FT_Load_Glyph(pGlyphFeature->pFace, glyph_index, FT_LOAD_DEFAULT);
	error = FT_Render_Glyph(pGlyphFeature->pFace->glyph, FT_RENDER_MODE_NORMAL);
	pGlyphCon->FontBitmapSize = (slot->metrics.width>>6) * (slot->metrics.height>>6);
	memcpy(&(pGlyphCon->Metrics), &(slot->metrics), sizeof(FT_Glyph_Metrics));
	pGlyphCon->pFontBitmap = (unsigned char*)MEMORY_CALLOC(sizeof(unsigned char), pGlyphCon->FontBitmapSize);

	memcpy((pGlyphCon->pFontBitmap), (slot->bitmap.buffer), pGlyphCon->FontBitmapSize);




	if (pCM->pCacheList == NULL)
	{
		int a = 0;
		//
		pCM->pCacheList = GlyphContainerLinkedList_Create(pGlyphCon);
		pCM->pCacheList->Size = GlyphContainerLinkedList_Size(pCM->pCacheList, bAddDummy4BytesAlign);



		result = 1;
	}
	else
	{
		pList2 = GlyphContainerLinkedList_GetTail(pCM->pCacheList);
		pList2->next = GlyphContainerLinkedList_Create(pGlyphCon);
		pList2->next->Size = GlyphContainerLinkedList_Size(pList2->next, bAddDummy4BytesAlign);
		result = 1;
	}

	// Align Dummy
	if (bAddDummy4BytesAlign > 0)
	{
		pCM->pCacheList->AlignDummyLength = (4 - pCM->pCacheList->Size % 4);
		if (pCM->pCacheList->AlignDummyLength != 0)
		{
			pCM->pCacheList->pAlignDummy = (unsigned char*)MEMORY_CALLOC(sizeof(unsigned char), pCM->pCacheList->AlignDummyLength);
			pCM->pCacheList->Size += pCM->pCacheList->AlignDummyLength;
		}
		memset(pCM->pCacheList->pAlignDummy, 0xFF, pCM->pCacheList->AlignDummyLength);
	}



	//
	return result;
}
#else
int GlyphContainerLinkedList_AddUnit(pCacheManager pCM, pGlyphContainerFeature pGlyphFeature, int bAddDummy4BytesAlign)
{
	FT_Error      error;
	FT_UInt glyph_index = 0;
	FT_GlyphSlot slot;
	int result = 0;
	pGlyphContainer pGlyphCon = NULL;
	pGlyphContainer_LinkedList pList2 = NULL;
	int index;
	//FT_Glyph glyph;
	FT_Matrix   matrix;                 /* transformation matrix */
	FT_Vector delta;
	
	//int Height, Width,horiBearingX, horiBearingY, horiAdvance, vertBearingX, vertBearingY, vertAdvance ;
	
	//
	if (pCM->pMemory == NULL)
	{
		index = GlyphContainerLinkedList_SearchGlyphFeature(pCM->pCacheList, pGlyphFeature);
	}
	else
	{
		index = GlyphContainerLinkedList_SearchGlyphFeature((pGlyphContainer_LinkedList)(pCM->pMemory), pGlyphFeature);
	}

	if (index >= 0)
	{
		return result;
	}
	//
	pGlyphCon = GlyphContainer_Create(pGlyphFeature);

	glyph_index = FT_Get_Char_Index( pGlyphFeature->pFace, pGlyphFeature->CharCode );
	error = FT_Set_Char_Size(pGlyphFeature->pFace, 0, pGlyphFeature->FontSize*64, pGlyphFeature->Resolution.x, pGlyphFeature->Resolution.y);
	pGlyphCon->glyph_index = glyph_index;
	error = FT_Load_Glyph(pGlyphFeature->pFace, glyph_index, FT_LOAD_DEFAULT);
		
	delta.x = 0;
       delta.y = 0;

	if (pGlyphFeature->FontRotation== 0) 
	{	
		matrix.xx = 0x10000L;
  		matrix.xy = 0;
  		matrix.yx = 0;
  		matrix.yy = 0x10000L;
		FT_Set_Transform( pGlyphFeature->pFace, /* target face object */
 							&matrix, /* pointer to 2x2 matrix */
 							0); /* pointer to 2d vector */
	}
	else if(pGlyphFeature->FontRotation == 90)
	{
		matrix.xx = (FT_Fixed)(0L);
  		matrix.xy = (FT_Fixed)(-0x10000L);
  		matrix.yx = (FT_Fixed)(0x10000L);
  		matrix.yy = (FT_Fixed)(0L);
		FT_Set_Transform( pGlyphFeature->pFace, /* target face object */
 							&matrix, /* pointer to 2x2 matrix */
 							0); /* pointer to 2d vector */
	}
	else if(pGlyphFeature->FontRotation == 180)
	{
		matrix.xx =  (FT_Fixed)(-0x10000L);
		matrix.xy =  (FT_Fixed)(0L); 
		matrix.yx =  (FT_Fixed)(0L);  
		matrix.yy =  (FT_Fixed)(-0x10000L);
		FT_Set_Transform( pGlyphFeature->pFace, /* target face object */
 							&matrix, /* pointer to 2x2 matrix */
 							0); /* pointer to 2d vector */
	}
	else if(pGlyphFeature->FontRotation == 270)
	{
		matrix.xx = (FT_Fixed)(0L);
  		matrix.xy = (FT_Fixed)(0x10000L);
  		matrix.yx = (FT_Fixed)(-0x10000L);
  		matrix.yy = (FT_Fixed)(0L);
		FT_Set_Transform( pGlyphFeature->pFace, /* target face object */
 							&matrix, /* pointer to 2x2 matrix */
 							0); /* pointer to 2d vector */
	}

	slot = pGlyphFeature->pFace->glyph;
		
	error = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);

	pGlyphCon->FontBitmapSize = (slot->metrics.width>>6) * (slot->metrics.height>>6);
	memcpy(&(pGlyphCon->Metrics), &(slot->metrics), sizeof(FT_Glyph_Metrics));
	memcpy(&(pGlyphCon->Slot), &slot, sizeof(FT_GlyphSlot));
		
	pGlyphCon->pFontBitmap = (unsigned char*)MEMORY_CALLOC(sizeof(unsigned char), pGlyphCon->FontBitmapSize);

	memcpy((pGlyphCon->pFontBitmap), (slot->bitmap.buffer), pGlyphCon->FontBitmapSize);
#if 0
	mpDebugPrint("slot->advance->x = %d, slot->advance->y =%d",slot->advance.x>>6 ,slot->advance.y>>6);

	Width = (pGlyphCon->Metrics.width>>6);
	Height = (pGlyphCon->Metrics.height>>6);
       horiBearingX = (pGlyphCon->Metrics.horiBearingX>>6);
	horiBearingY = (pGlyphCon->Metrics.horiBearingY>>6);
	horiAdvance = (pGlyphCon->Metrics.horiAdvance>>6);

	vertBearingX = (pGlyphCon->Metrics.vertBearingX>>6);
	vertBearingY = (pGlyphCon->Metrics.vertBearingY>>6);
	vertAdvance = (pGlyphCon->Metrics.vertAdvance>>6);

	mpDebugPrint("CharCode = 0x%x ,H = %d, W = %d, hX = %d, hY = %d, hA = %d, vX = %d,  vY = %d, vA = %d",
		                       pGlyphFeature->CharCode, Height, Width,horiBearingX, horiBearingY, horiAdvance, 
		                       							vertBearingX,vertBearingY,vertAdvance);
#endif

	if (pCM->pCacheList == NULL)
	{
		int a = 0;
		//
		pCM->pCacheList = GlyphContainerLinkedList_Create(pGlyphCon);
		pCM->pCacheList->Size = GlyphContainerLinkedList_Size(pCM->pCacheList, bAddDummy4BytesAlign);



		result = 1;
	}
	else
	{
		pList2 = GlyphContainerLinkedList_GetTail(pCM->pCacheList);
		pList2->next = GlyphContainerLinkedList_Create(pGlyphCon);
		pList2->next->Size = GlyphContainerLinkedList_Size(pList2->next, bAddDummy4BytesAlign);
		result = 1;
	}

	// Align Dummy
	if (bAddDummy4BytesAlign > 0)
	{
		pCM->pCacheList->AlignDummyLength = (4 - pCM->pCacheList->Size % 4);
		if (pCM->pCacheList->AlignDummyLength != 0)
		{
			pCM->pCacheList->pAlignDummy = (unsigned char*)MEMORY_CALLOC(sizeof(unsigned char), pCM->pCacheList->AlignDummyLength);
			pCM->pCacheList->Size += pCM->pCacheList->AlignDummyLength;
		}
		memset(pCM->pCacheList->pAlignDummy, 0xFF, pCM->pCacheList->AlignDummyLength);
	}



	//
	return result;
}

#endif

int GlyphContainerLinkedList_AddUnitArray (pCacheManager pCM, stPrintTTFConfig* pttf_config, FT_Face pFace, long* charcodeArray, int length, int bAddDummy4BytesAlign)
{
	int i = 0;
	int compare;
	int count = 0;
	//
	//static stGlyphContainerFeature feature;
	stGlyphContainerFeature feature;
	memset(&feature,0,sizeof(stGlyphContainerFeature)); //cj add
		
	for (i = 0; i<length; i++)
	{
		feature.CharCode = charcodeArray[i];
		feature.FontSize = pttf_config->StringData.FontSize;
		feature.pFace = pFace;
		feature.RenderMode = pttf_config->Rendering;
		feature.Resolution.x = pttf_config->ImagePlaneData.DeviceResolution.x;
		feature.Resolution.y = pttf_config->ImagePlaneData.DeviceResolution.y;
		feature.FontRotation  = pttf_config->FontRotation;
		compare = GlyphContainerLinkedList_AddUnit(pCM, &feature, bAddDummy4BytesAlign);

		if (compare == 1)
				count++;
	}
	return count;
}

pGlyphContainer_LinkedList GlyphContainerLinkedList_IndexAt(pGlyphContainer_LinkedList pList, int index)
{
	pGlyphContainer_LinkedList pResult = pList;
	int i = 0;
	//
	if (pResult == NULL)
		return NULL;
	//
	while(pResult)
	{
		if (index == 0)
		{
			break;
		}
		index --;
		pResult = pResult->next;
	}
	//
	if (index != 0)
		pResult = NULL;

	//
	return pResult;
}

int GlyphContainerLinkedList_Concatenate(pGlyphContainer_LinkedList pList_Head, pGlyphContainer_LinkedList pList_Tail)
{
	int result = 1;
	//
	if (pList_Head == NULL)
		return 0;
	if (pList_Tail == NULL)
		return 0;
	//
	((pGlyphContainer_LinkedList)GlyphContainerLinkedList_GetTail(pList_Head))->next = pList_Tail;
	//
	return result;
}


