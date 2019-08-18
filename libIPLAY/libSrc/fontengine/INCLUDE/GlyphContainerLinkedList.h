#ifndef __GLYPHCONTAINERLINKEDLIST_H__
#define __GLYPHCONTAINERLINKEDLIST_H__

#include "Basic_Struct.h"
#include "Basic_Enum.h"
#include "PrintTTFConfig.h"

//
////////////////////////////////////////////////////////////////////////
//
typedef struct stCacheManager_* pCacheManager;

typedef struct stGlyphContainer_LinkedList_* pGlyphContainer_LinkedList;



typedef struct stGlyphContainer_LinkedList_
{
    int							index;
    pGlyphContainer				pGlyphCon;
	pGlyphContainer_LinkedList	next;
	int							Size;
	unsigned char*				pAlignDummy;
	int							AlignDummyLength;
	//
	//stGlyphContainerLinkedListFuncs Funcs;
}stGlyphContainer_LinkedList, *pstGlyphContainer_LinkedList;


//Function prototype
pGlyphContainer_LinkedList GlyphContainerLinkedList_Create(pGlyphContainer pGlyphCon);
int GlyphContainerLinkedList_Length(pGlyphContainer_LinkedList pList);
void GlyphContainerLinkedList_Release(pGlyphContainer_LinkedList* ppGlyphConList);
pGlyphContainer_LinkedList GlyphContainerLinkedList_GetTail(pGlyphContainer_LinkedList pList);
int GlyphContainerLinkedList_Concatenate(pGlyphContainer_LinkedList pList_Head, pGlyphContainer_LinkedList pList_Tail);
int GlyphContainerLinkedList_SearchGlyphCon(pGlyphContainer_LinkedList pList, pGlyphContainer pGlyphCon);
int GlyphContainerLinkedList_SearchGlyphFeature(pGlyphContainer_LinkedList pList, pGlyphContainerFeature pFeature);
//int GlyphContainerLinkedList_AddUnit(pGlyphContainer_LinkedList* ppList, pGlyphContainerFeature pGlyphFeature, int bAddDummy4BytesAlign);
int GlyphContainerLinkedList_AddUnit(pCacheManager pCM, pGlyphContainerFeature pGlyphFeature, int bAddDummy4BytesAlign);
int GlyphContainerLinkedList_MoveUnit(pGlyphContainer_LinkedList* ppList_Source, pGlyphContainer_LinkedList pList_Target);
int GlyphContainerLinkedList_Size(pGlyphContainer_LinkedList pList, int bAddDummy4BytesAlign);
int GlyphContainerLinkedList_AddUnitArray(pCacheManager pCM, stPrintTTFConfig* pttf_config, FT_Face pFace, long* charcodeArray, int length, int bAddDummy4BytesAlign);


int GlyphContainerLinkedList_Length(pGlyphContainer_LinkedList pList);

pGlyphContainer_LinkedList GlyphContainerLinkedList_IndexAt(pGlyphContainer_LinkedList pList, int index);


#endif	//__GLYPHCONTAINERLINKEDLIST_H__

