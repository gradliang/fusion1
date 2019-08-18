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


#include "CacheManager.h"

BYTE* CacheManager_CacheMemoryGetRoom(pCacheManager pCM, pGlyphContainer_LinkedList pList)
{
	BYTE* pAddr = NULL;
	int room = 0;
	int list_size = 0;

	//Nothing in the cache memory
	if (((pGlyphContainer_LinkedList)pCM->pMemory)->pGlyphCon == NULL)
	{
		return (pCM->pMemory);
	}

	//Get the size of the list unit
	list_size = GlyphContainerLinkedList_Size(pList, pCM->bAddDummy4BytesAlign);

	//If middle gap exists, fill it first, then end gap
	if (pCM->Middle_Gap_Start == NULL)
	{
		//room = (BYTE*)pCM->pMemory + pCM->MemorySize -  (BYTE*)(pCM->End_Gap_Start);
		if (pCM->End_Gap_Length > list_size)
		{
			return pCM->End_Gap_Start;
		}
	}
	else
	{
		//if (pCM->Middle_Gap_Length < list_size)	return NULL;
		if (pCM->Middle_Gap_Length > list_size)
		{
			return pCM->Middle_Gap_Start;
		}

	}

	return NULL;
}

int CacheManager_CacheMemoryEnough(pCacheManager pCM, pGlyphContainer_LinkedList pList)
{
	int result = 1;
	int room = 0;
	int list_size = 0;

	//Nothing in the cache memory
	if (((pGlyphContainer_LinkedList)pCM->pMemory)->pGlyphCon == NULL)
	{
		room = pCM->MemorySize;
		return 1;
	}

	//
	list_size = GlyphContainerLinkedList_Size(pList, pCM->bAddDummy4BytesAlign);
	if (pCM->Middle_Gap_Start == NULL)
	{
		room = (BYTE*)pCM->pMemory + pCM->MemorySize -  (BYTE*)GlyphContainerLinkedList_GetTail((pGlyphContainer_LinkedList)pCM->pMemory);
		if (room < list_size)	return -1;
	}
	else
	{
		if (pCM->Middle_Gap_Length < list_size)	return -1;
	}

	return 1;

}

pCacheManager CacheManager_Create(stPrintTTFConfig* pttf_config, FT_Face pFace, long* pCharcodeArray, int ArrayLength)
{
	pCacheManager pCM = (pCacheManager)MEMORY_MALLOC(sizeof(stCacheManager));

	//
	pCM->pCacheList = NULL;
	pCM->pMemory = NULL;


	//
	if ((pttf_config != NULL)&&(pFace != NULL)&&(pCharcodeArray != NULL)&&(ArrayLength != NULL))
	{
		GlyphContainerLinkedList_AddUnitArray(pCM, pttf_config, pFace, pCharcodeArray, ArrayLength, pCM->bAddDummy4BytesAlign);
		//
		CacheManager_InitCacheMemory(pCM, pttf_config);
	}
	else
	{
		pCM->pCacheList = NULL;
	}

	
	//
	return pCM;
}

int CacheManager_ResetCacheMemory(pCacheManager pCM)
{
	pCM->Middle_Gap_Start = NULL;
	pCM->Middle_Gap_Length = 0;
	pCM->End_Gap_Start = pCM->pMemory;
	pCM->End_Gap_Length = pCM->MemorySize;

	memset(pCM->pMemory, 0x00, pCM->MemorySize);

	return 0;
}

int CacheManager_InitCacheMemory(pCacheManager pCM, stPrintTTFConfig* pttf_config)
{
	int i = 0;
	//
	if ((pCM == NULL) || (pttf_config == NULL))
	{
		return -1;
	}
	//
	if (pCM->pMemory != NULL)
	{
		return -1 ;
	}
	//
	if (pttf_config->CacheSetting.CacheSpaceStyle == CACHE_SPACE_STYLE_STATIC_ALLOCATED)
	{
		pCM->MemorySize = pttf_config->CacheSetting.MemorySize;

		if (pttf_config->CacheSetting.pMemory == NULL)
		{
#ifdef PC_DEVELOPMENT
			pCM->pMemory = (BYTE*)MEMORY_CALLOC(sizeof(BYTE), pCM->MemorySize);
#endif
#ifdef TARGET_DEVELOPMENT
			pCM->pMemory = (BYTE*)MEMORY_MALLOC(pCM->MemorySize);
#endif
		}
		else
		{
			pCM->pMemory = pttf_config->CacheSetting.pMemory;
		}

		//Zero out memory
		memset(pCM->pMemory, 0x00, pCM->MemorySize);
	}
	//
	pCM->Middle_Gap_Start = NULL;
	pCM->Middle_Gap_Length = 0;
	pCM->End_Gap_Start = pCM->pMemory;
	pCM->End_Gap_Length = pCM->MemorySize;
	//
	pCM->bAddDummy4BytesAlign = pttf_config->CacheSetting.bAddDummy4BytesAlign;


	return 1;
}

void CacheManager_Release(pCacheManager* ppCM)
{
	if ((*ppCM)->pCacheList != NULL)
	{
		GlyphContainerLinkedList_Release(&((*ppCM)->pCacheList));
	}

	if ((*ppCM)->pMemory != NULL)
	{
		MEMORY_FREE((*ppCM)->pMemory);
	}
	//
	MEMORY_FREE((*ppCM));
	(*ppCM) = NULL;
}

int CacheManager_VerifyCacheMemory(pCacheManager pCM)
{
	int sum = 0;
	pGlyphContainer_LinkedList pHead0 = NULL;
	pGlyphContainer_LinkedList pHead = (pGlyphContainer_LinkedList)(pCM->pMemory);

	if (pHead->pGlyphCon == NULL)
	{
		if (pCM->Middle_Gap_Start != NULL)
			return -1;
		if (pCM->Middle_Gap_Length != 0)
			return -2;
		if (pCM->End_Gap_Length != pCM->MemorySize)
			return -3;
		if (pCM->End_Gap_Start != pCM->pMemory)
			return -4;
	}
	else
	{
		while(pHead != NULL)
		{
			if ((((BYTE*)pHead + pHead->Size) != (BYTE*)pHead->next) && (pHead->next != NULL))
			{
				if (((BYTE*)pHead + pHead->Size) != pCM->Middle_Gap_Start)
					return -5;
				if (((BYTE*)pHead + pHead->Size + pCM->Middle_Gap_Length) != (BYTE*)pHead->next)
					return -6;
			}

			pHead0 = pHead;
			pHead = pHead->next;
		}

		pHead = pHead0;

		if ((BYTE*)pHead + pHead->Size != pCM->End_Gap_Start)
			return -7;
		if ((BYTE*)pHead + pHead->Size + pCM->End_Gap_Length != pCM->pMemory + pCM->MemorySize)
			return -8;
	}

	return 1;
}

int CacheManager_CacheReplaceUnit(pCacheManager pCM, pGlyphContainer_LinkedList* ppList)
{
	int result = 0;
	pGlyphContainer_LinkedList pKick = NULL;
	pGlyphContainer_LinkedList pPrior = NULL;
	pGlyphContainer_LinkedList pAfter = NULL;
	int i = 0;
	//
	
	//result = CacheManager_VerifyCacheMemory(pCM);
	// Check middle gap status
	if (pCM->Middle_Gap_Start == NULL)
	{
		int size_1 = GlyphContainerLinkedList_Size(*ppList, pCM->bAddDummy4BytesAlign);
		int size_2 = 0;
		int count = 0;
		//Calculate number of list units needed to be kicked
		while(size_2 < size_1)
		{
			size_2 += GlyphContainerLinkedList_Size(GlyphContainerLinkedList_IndexAt((pGlyphContainer_LinkedList)(pCM->pMemory), count), pCM->bAddDummy4BytesAlign);
			count++;
		}
		//
		pAfter = GlyphContainerLinkedList_IndexAt((pGlyphContainer_LinkedList)(pCM->pMemory), count);
		//
		memset((BYTE*)pCM->pMemory, 0x00, (BYTE*)pAfter - (BYTE*)pCM->pMemory);
		/*
		for (i=0; i<(BYTE*)pAfter - (BYTE*)pCM->pMemory; i++)
		{
			((BYTE*)pCM->pMemory)[i] = 0x00;
		}
		*/
		//
		GlyphContainerLinkedList_MoveUnit(ppList, (pGlyphContainer_LinkedList)pCM->pMemory);
		//
		((pGlyphContainer_LinkedList)pCM->pMemory)->next = pAfter;
		//
		pCM->Middle_Gap_Start = (BYTE*)(pCM->pMemory)+GlyphContainerLinkedList_Size((pGlyphContainer_LinkedList)pCM->pMemory, pCM->bAddDummy4BytesAlign);
		pCM->Middle_Gap_Length = (BYTE*)pAfter - (BYTE*)pCM->Middle_Gap_Start;
		//
		int aaa = 1;
	}
	else
	{
		int size_1 = GlyphContainerLinkedList_Size(*ppList, pCM->bAddDummy4BytesAlign);
		int size_2 = pCM->Middle_Gap_Length;
		int count = 0;
		pGlyphContainer_LinkedList pAfterMiddleGap = (pGlyphContainer_LinkedList)pCM->pMemory;

		//
	//	result = CacheManager_VerifyCacheMemory(pCM);
		//Get the unit following the middle gap and the prior one
		while ((BYTE*)pAfterMiddleGap < pCM->Middle_Gap_Start)
		{
			pPrior = pAfterMiddleGap;
			pAfterMiddleGap = pAfterMiddleGap->next;
		}
	//	result = CacheManager_VerifyCacheMemory(pCM);//**********************

		// Middle gap is enough?
		if (size_2 > size_1)	//yes
		{
			pAfter = pAfterMiddleGap;

		}
		else					// no
		{
			int bNewTail = 0;
			// Calculate number of list units needed to be kicked
			pAfter = (pGlyphContainer_LinkedList)(pAfterMiddleGap);
			while (size_2 < size_1)
			{
				size_2 += GlyphContainerLinkedList_Size(pAfter, pCM->bAddDummy4BytesAlign);
				if (pAfter == NULL)
				{
					break;
				}
				else
				{
					pAfter = pAfter->next;
				}
				count++;
			}
			//All units after the middle gap will be deleted, so merge to one end gap
			if (pAfter == NULL)	//<====
			{
				memset((BYTE*)pAfterMiddleGap, 0x00, (BYTE*)pCM->pMemory + pCM->MemorySize - (BYTE*)pAfterMiddleGap);
				pPrior->next = NULL;
			}
			else
			{
				memset(pCM->Middle_Gap_Start, 0x00, size_2);
			}
		}

		GlyphContainerLinkedList_MoveUnit(ppList, (pGlyphContainer_LinkedList)pCM->Middle_Gap_Start);
		//

		pPrior->next = (pGlyphContainer_LinkedList)pCM->Middle_Gap_Start;

		pPrior->next->next = pAfter;



		//set middle gap info
		//
		if (pAfter == NULL)
		{
			pCM->Middle_Gap_Start = NULL;
			pCM->Middle_Gap_Length = 0;
			pCM->End_Gap_Start = (BYTE*)pPrior->next + pPrior->next->Size;
			pCM->End_Gap_Length = pCM->pMemory + pCM->MemorySize - pCM->End_Gap_Start;
		}
		else
		{
			pCM->Middle_Gap_Start = ((BYTE*)pPrior->next)+GlyphContainerLinkedList_Size((pGlyphContainer_LinkedList)pPrior->next, pCM->bAddDummy4BytesAlign);
			pCM->Middle_Gap_Length = (BYTE*)pAfter - pCM->Middle_Gap_Start;
		}
		//
		//result = CacheManager_VerifyCacheMemory(pCM);

	}



	//
	return result;
}

int CacheManager_AddUnit(pCacheManager pCM, pGlyphContainerFeature pGlyphFeature, int bAddDummy4BytesAlign)
{
	int result = 0;
	int bCacheMemoryEnough = 0;
	pGlyphContainer_LinkedList pPrior = NULL;
	pGlyphContainer_LinkedList pAfter = NULL;
	int i = 0;
	//
	if (pCM->pMemory == NULL)
	{
		result = GlyphContainerLinkedList_AddUnit(pCM, pGlyphFeature, bAddDummy4BytesAlign);
	}
	else
	{
		//////////////////// Move cache list to cache memory ////////////////////

		pGlyphContainer_LinkedList Addr_to_add = NULL;
		pGlyphContainer_LinkedList Current_Tail = GlyphContainerLinkedList_GetTail((pGlyphContainer_LinkedList)pCM->pMemory);
		int bInsertEnd = 0;
		//
		//Generate a list unit
		if (pCM->pCacheList != NULL)
		{
			int aaa = 1;
		}
		GlyphContainerLinkedList_AddUnit(pCM, pGlyphFeature, bAddDummy4BytesAlign);
		//
		if (Current_Tail->pGlyphCon == NULL)
		{
			Current_Tail = NULL;
		}

		// Check and get the address in the cache memory to move in
		Addr_to_add = (pGlyphContainer_LinkedList)CacheManager_CacheMemoryGetRoom(pCM, pCM->pCacheList);

		////// No room for the new unit
		// Insert in End or Middle?
		if (Current_Tail == NULL)	//Nothing in the cache memory   ===> End
		{
			bInsertEnd = 1;
		}
		else
		{
			bInsertEnd =((pGlyphContainer_LinkedList)Addr_to_add > Current_Tail)? 1 : 0;
		}

		// Move the unit to cache memory
		if (Addr_to_add == NULL)	//No enough room for the new unit
		{
			CacheManager_CacheReplaceUnit(pCM, &(pCM->pCacheList));
		}
		else   
		{
			//
			GlyphContainerLinkedList_MoveUnit(&(pCM->pCacheList), Addr_to_add);
			//
			
			// Update cache memory maintain information
			if (bInsertEnd == 1)
			{
				pCM->End_Gap_Start = (BYTE*)(Addr_to_add) + GlyphContainerLinkedList_Size(Addr_to_add, pCM->bAddDummy4BytesAlign);
				pCM->End_Gap_Length = (BYTE*)pCM->pMemory + pCM->MemorySize - (BYTE*)pCM->End_Gap_Start;
				if (Current_Tail != NULL)
				{
					Current_Tail->next = Addr_to_add;
				}
				if (pCM->pCacheList != NULL)
				{
					int aaa = 1;
				}

			}
			else
			{
				int bNewTail = 0;
				//
				pPrior = (pGlyphContainer_LinkedList)pCM->pMemory;

				//int i = GlyphContainerLinkedList_Size((pGlyphContainer_LinkedList)pCM->pMemory);
				for (i=0; i<GlyphContainerLinkedList_Size((pGlyphContainer_LinkedList)pCM->pMemory, pCM->bAddDummy4BytesAlign); i++)
				{
					if (pPrior == NULL)
					{
						bNewTail = 1;
						break;
					}

					if ((BYTE*)pPrior->next > pCM->Middle_Gap_Start)
					{
						pAfter = pPrior->next;
						break;
					}
					pPrior = pPrior->next;
				}
				pPrior->next = Addr_to_add;
				Addr_to_add->next = pAfter;
				//
				if (bNewTail == 1)
				{
					pCM->Middle_Gap_Start = NULL;
					pCM->Middle_Gap_Length = 0;
					//
					pCM->End_Gap_Start = (BYTE*)Addr_to_add + GlyphContainerLinkedList_Size(Addr_to_add, pCM->bAddDummy4BytesAlign);
					pCM->End_Gap_Length = ((BYTE*)pCM->pMemory + pCM->MemorySize) - pCM->End_Gap_Start;
					if (pCM->pCacheList != NULL)
					{
						int aaa = 1;
					}

				}
				else
				{
					pCM->Middle_Gap_Start = (BYTE*)Addr_to_add + GlyphContainerLinkedList_Size(Addr_to_add, pCM->bAddDummy4BytesAlign);
					pCM->Middle_Gap_Length = (BYTE*)(Addr_to_add->next) - pCM->Middle_Gap_Start;
					if (pCM->pCacheList != NULL)
					{
						int aaa = 1;
					}
				}
				

			}
		}
	}
	//////////////
	if (pCM->pCacheList != NULL)
	{
		int aaa = 1;
	}

	result = CacheManager_VerifyCacheMemory(pCM);
	if (result < 0)
	{
		CacheManager_ResetCacheMemory(pCM);
		CacheManager_AddUnit(pCM, pGlyphFeature, bAddDummy4BytesAlign);
	}
	//
	return result;
}



int CacheManager_AddUnitArray(pCacheManager pCM, stPrintTTFConfig* pttf_config, FT_Face pFace, long* pCharcodeArray, int length)
{
	int result = 0;
	stGlyphContainerFeature feature;
	int i = 0;
	pGlyphContainer_LinkedList SearchTarget = NULL;
	//
	CacheManager_InitCacheMemory(pCM, pttf_config);
	//
	SearchTarget = (pCM->pMemory == NULL)?(pCM->pCacheList):(pGlyphContainer_LinkedList)(pCM->pMemory);
	//
	 if (length>500) // Test Fix
	 	length=500;
	 
	for (i = 0; i<length; i++)
	{
		feature.CharCode = pCharcodeArray[i];
		feature.FontSize = pttf_config->StringData.FontSize;
		feature.pFace = pFace;
		feature.RenderMode = pttf_config->Rendering;
		feature.Resolution.x = pttf_config->ImagePlaneData.DeviceResolution.x;
		feature.Resolution.y = pttf_config->ImagePlaneData.DeviceResolution.y;
		feature.FontRotation = pttf_config->FontRotation; //cj add for rotation
		
		if (pCM->pCacheList != NULL)
		{
			int aaa = 1;
		}
		
		if (GlyphContainerLinkedList_SearchGlyphFeature(SearchTarget, &feature)<0)
		{
			//result = CacheManager_VerifyCacheMemory(pCM);
			result = CacheManager_AddUnit(pCM, &feature, pCM->bAddDummy4BytesAlign);
		}
	}

	//result = CacheManager_VerifyCacheMemory(pCM);
	
	return result;
}

int CacheManager_CacheListLength(pCacheManager pCM)
{
	int i = 0;
	int count = 0;
	pGlyphContainer_LinkedList pTemp = NULL;
	/////////////////////////
	if (pCM == NULL)
	{
		return -1;
	}

	//

	pTemp = pCM->pCacheList;

	while (pTemp != NULL)
	{
		count++;
		pTemp = pTemp->next;
	}

	return count;
}

pGlyphContainer CacheManager_RequestUnitMetrics(pCacheManager pCM, pGlyphContainerFeature pGlyphConFeature, WHILE_CACHE_MISS_DO WhileCacheMissDo)
{
	int index = -1;
	pGlyphContainer pGlyphCon_Temp = NULL;
	//

	index = GlyphContainerLinkedList_SearchGlyphFeature(pCM->pCacheList, pGlyphConFeature);
	if (index<0)
	{
		if (WhileCacheMissDo == WHILE_CACHE_MISS_DO_IGNORE)
		{
			return NULL;
		}
		if (WhileCacheMissDo == WHILE_CACHE_MISS_DO_ADD)
		{
			CacheManager_AddUnit(pCM, pGlyphConFeature, pCM->bAddDummy4BytesAlign);
			index = GlyphContainerLinkedList_SearchGlyphFeature(pCM->pCacheList, pGlyphConFeature);
		}
	}
	//
	pGlyphCon_Temp = ((pGlyphContainer_LinkedList)GlyphContainerLinkedList_IndexAt(pCM->pCacheList, index))->pGlyphCon;
	//
	return pGlyphCon_Temp;
}


pGlyphContainer CacheManager_RequestUnitGlyphBMP(pCacheManager pCM, pGlyphContainerFeature pGlyphConFeature, WHILE_CACHE_MISS_DO WhileCacheMissDo)
{
	int index = -1;
	pGlyphContainer pGlyphCon_Temp = NULL;
	//
	index = GlyphContainerLinkedList_SearchGlyphFeature(pCM->pCacheList, pGlyphConFeature);
	if ((index<0) && (WhileCacheMissDo == WHILE_CACHE_MISS_DO_ADD))
	{
		if (pCM->pMemory == NULL)
		{
			GlyphContainerLinkedList_AddUnit(pCM, pGlyphConFeature, pCM->bAddDummy4BytesAlign);
			index = GlyphContainerLinkedList_SearchGlyphFeature((pGlyphContainer_LinkedList)pCM->pCacheList, pGlyphConFeature);
			pGlyphCon_Temp = ((pGlyphContainer_LinkedList)GlyphContainerLinkedList_IndexAt(pCM->pCacheList, index))->pGlyphCon;
		}
		else
		{
			

			index = GlyphContainerLinkedList_SearchGlyphFeature((pGlyphContainer_LinkedList)((pGlyphContainer_LinkedList)(pCM->pMemory)), pGlyphConFeature);
			if (index < 0)
			{
				CacheManager_AddUnit(pCM, pGlyphConFeature, pCM->bAddDummy4BytesAlign);
			}
			index = GlyphContainerLinkedList_SearchGlyphFeature((pGlyphContainer_LinkedList)((pGlyphContainer_LinkedList)(pCM->pMemory)), pGlyphConFeature);

			pGlyphCon_Temp = ((pGlyphContainer_LinkedList)GlyphContainerLinkedList_IndexAt((pGlyphContainer_LinkedList)(pCM->pMemory), index))->pGlyphCon;
		}
	}

	//
	return pGlyphCon_Temp;
}


void Test(stPrintTTFConfig* pttf_config, FT_Face pFace)
{
	long str7[] = {'M', 'g'};
	int  str7_length = 2;
	stPrintTTFConfig ttf_config;
	//
	ttf_config.StringData.FontSize = 8;
	ttf_config.ImagePlaneData.DeviceResolution.x = 150;
	ttf_config.ImagePlaneData.DeviceResolution.y = 150;
}
