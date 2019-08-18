#ifndef __CACHEMANGER_H__
#define __CACHEMANGER_H__

#include "Basic_Struct.h"
#include "GlyphContainer.h"
#include "PrintTTFConfig.h"
#include "GlyphContainerLinkedList.h"

//
//typedef struct stCacheManager_* pCacheManager;


//
typedef struct stCacheManager_
{
	//Data
	pGlyphContainer_LinkedList pCacheList;
	//Memory
	unsigned char*	pMemory;
	int		MemorySize;
	//Gap mamagement
	unsigned char*	Middle_Gap_Start;
	int		Middle_Gap_Length;
	unsigned char*	End_Gap_Start;
	int		End_Gap_Length;
	int		bAddDummy4BytesAlign;
}stCacheManager, *pstCacheManager;


//Function prototype
pCacheManager CacheManager_Create(stPrintTTFConfig* pttf_config, FT_Face pFace, long* pCharcodeArray, int ArrayLength);
int CacheManager_InitCacheMemory(pCacheManager pCM, stPrintTTFConfig* pttf_config);
int CacheManager_ResetCacheMemory(pCacheManager pCM);
void CacheManager_Release(pCacheManager* ppCM);
int CacheManager_AddUnit(pCacheManager pCM, pGlyphContainerFeature pGlyphFeature, int bAddDummy4BytesAlign);
int CacheManager_VerifyCacheMemory(pCacheManager pCM);
int CacheManager_CacheReplaceUnit(pCacheManager pCM, pGlyphContainer_LinkedList* ppList);
int CacheManager_AddUnitArray(pCacheManager pCM, stPrintTTFConfig* pttf_config, FT_Face pFace, long* pCharcodeArray, int length);
pGlyphContainer CacheManager_RequestUnitMetrics(pCacheManager pCM, pGlyphContainerFeature pGlyphConFeature, WHILE_CACHE_MISS_DO WhileCacheMissDo);
pGlyphContainer CacheManager_RequestUnitGlyphBMP(pCacheManager pCM, pGlyphContainerFeature pGlyphConFeature, WHILE_CACHE_MISS_DO WhileCacheMissDo);
int CacheManager_CacheListLength(pCacheManager pCM);
int CacheManager_CacheMemoryEnough(pCacheManager pCM, pGlyphContainer_LinkedList pList);
unsigned char* CacheManager_CacheMemoryGetRoom(pCacheManager pCM, pGlyphContainer_LinkedList pList);




//Function pointers typedef for Cache


//
void Test(stPrintTTFConfig* pttf_config, FT_Face pFace);




#endif //__CACHEMANGER_H__
