/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "xpg.h"
#include "xpgUtil.h"
#include "xpgMovie.h"

#include "taskid.h" // for EXCEPTION_MSG_ID

//----------------------------------------------------------------------------
///
///@ingroup xpgMovie
///@brief   Init pstMovie parameters
///
///@param   pstMovie - pointer of Movie
///
///@return
///
void xpgMovieInit(register STXPGMOVIE * pstMovie)
{
	memset(pstMovie, 0, sizeof(STXPGMOVIE));

	pstMovie->m_wScreenWidth = 240;
	pstMovie->m_wScreenHeight = 320;
	pstMovie->m_dwBkgColor = 0xfffff;
	pstMovie->m_dwTransColor = 0xff0000;
	pstMovie->m_wMaxSprites = 128;

	pstMovie->m_wSpriteDataLen = 12;
	pstMovie->m_wRoleCount = 0;

	// init chunk header length & position
	pstMovie->m_wImageHeaderLen = 24;
	pstMovie->m_wPageHeaderLen = 16;
	pstMovie->m_wScriptHeaderLen = 12;
	pstMovie->m_wSpriteDataLen = 20;
	pstMovie->m_dwImageHeaderPos = 0;
	pstMovie->m_dwPageHeaderPos = 0;
	pstMovie->m_dwScriptHeaderPos = 0;
	pstMovie->m_wPageCount = 0;
	pstMovie->m_wScriptCount = 0;
	pstMovie->m_pstListFrame = NULL;
	pstMovie->m_pstThumbFrame = NULL;
	pstMovie->m_pstButtonFrame = NULL;
	pstMovie->m_dwListCount = 0;
	pstMovie->m_dwThumbCount = 0;
	pstMovie->m_dwCardCount = 0;
	pstMovie->m_dwThumbBufferCount = 0;
	pstMovie->m_dwThumbBufferSize = 0;
	pstMovie->m_dwThumbColumns = 4;

}

//---------------------------------------------------------------------------
///
///@ingroup xpgMovie
///@brief   Load specified Movie's akk pages
///
///@param   pstMovie - pointer of Movie
///
///@param   iPage - working page index
///
///@return
///

void xpgMovieLoadPage(register STXPGMOVIE * pstMovie, DWORD iPage)
{
    MP_DEBUG("xpgMovieLoadPage iPage = %d", iPage);
    
    DWORD dwOffset;
    if(iPage > 0) // in boot up, system will load page 0 first and then xpgMemoryOffset not yet set
    {
        dwOffset = xpgMemoryGetOffsetSave();
        MP_DEBUG("%s: dwXpgMemOffsetSave = 0x%X", __func__, dwOffset);
        xpgMemorySetOffset(dwOffset);
    } 
    
	DWORD i, j;
	DWORD iSpriteCount;
	STXPGPAGE *pstPage = NULL;
	pstPage = &(pstMovie->m_astPage[iPage]);	//xpgGetPage( pstMovie, iPage );

	pstMovie->m_pstPrevPage = pstMovie->m_pstCurPage;
	pstMovie->m_pstCurPage = pstPage;
	pstMovie->m_wCurPage = pstPage->m_wIndex;

	iSpriteCount = pstPage->m_wSpriteCount;
	pstMovie->m_dwSpriteCount = iSpriteCount;
	pstMovie->m_wFrameX = 0;
	pstMovie->m_wFrameY = 0;
	pstMovie->m_dwListCount = 0;
	pstMovie->m_dwThumbCount = 0;
	//pstMovie->m_dwCardCount = 0;
	pstMovie->m_wAniCount = 0;
	pstMovie->m_pstListFrame = NULL;
	pstMovie->m_pstThumbFrame = NULL;
	pstMovie->m_boPopMenu = false;
	pstMovie->m_bMenuItemCount = 0;
	pstMovie->m_bMenuItemIndex = 0;

	pstMovie->m_wMenuLeft = 0;
	pstMovie->m_wMenuTop = 0;
	pstMovie->m_wMenuRight = 0;
	pstMovie->m_wMenuBottom = 0;
#if XPG_LIST_COUNT
	for (i = 0; i < XPG_LIST_COUNT; i++)
		memset(&(pstMovie->m_astList[i]), 0, sizeof(STXPGLIST));
#endif
#if XPG_THUMB_COUNT
	for (i = 0; i < XPG_THUMB_COUNT; i++)
	{
		pstMovie->m_astThumb[i].m_pstSprite = NULL;
	}
#endif
#if XPG_SCROLLBAR_COUNT
	for (i = 0; i < XPG_SCROLLBAR_COUNT; i++)
	{
		pstMovie->m_pstScrollBar[i] = NULL;
		pstMovie->m_pstScrollButton[i] = NULL;
	}
#endif
	MP_DEBUG("\tiSpriteCount = %d", iSpriteCount);

	for (i = 0; i < iSpriteCount; i++)
	{
	    MP_DEBUG("\ti=%d(i<iSpriteCount)", i);
		memset(&pstMovie->m_astSprite[i], 0, sizeof(STXPGSPRITE));  //Mason 20060619  //From Athena
		xpgSpriteCopy(pstMovie, &(pstMovie->m_astSprite[i]), &(pstPage->m_astSprite[i]));
		
	}

#if XPG_DIALOG_EXTRA_SPRITE_ENABLE
    //mpDebugPrint("g_isDialogPage = %d", g_isDialogPage);
    if (g_isDialogPage)
    {
        DWORD dialogSpriteExtraCount = getCurDialogExtraSpriteCount();
        XPGEXTRASPRITE * extraSpriteList = getCurDialogExtraSpriteList();
        mpDebugPrint("dialogSpriteExtraCount = %d, extraSpriteList = %x", dialogSpriteExtraCount, extraSpriteList);
        if (extraSpriteList)
        {
            for (j = 0; j < dialogSpriteExtraCount; j++) 
            {
                memset(&pstMovie->m_astSprite[i + j], 0, sizeof(STXPGSPRITE));
    		    xpgExtraSpriteCopy(&(pstMovie->m_astSprite[i + j]), &(extraSpriteList[j]));
            }
            pstMovie->m_dwSpriteCount += dialogSpriteExtraCount;
        }
    }   
#endif
    

#if XPG_LIST_COUNT
	if (pstMovie->m_pstListFrame != NULL)
	{
		STXPGSPRITE *pstFirst = pstMovie->m_astList[0].m_pstSprite;

		if (pstFirst != NULL)
		{
			pstMovie->m_wFrameX = pstMovie->m_pstListFrame->m_wPx - pstFirst->m_wPx;
			pstMovie->m_wFrameY = pstMovie->m_pstListFrame->m_wPy - pstFirst->m_wPy;
		}
	}
#endif
#if XPG_THUMB_COUNT
	if (pstMovie->m_pstThumbFrame != NULL)
	{
		STXPGSPRITE *pstFirst = pstMovie->m_astThumb[0].m_pstSprite;

		if (pstFirst != NULL)
		{
			pstMovie->m_wFrameX = pstMovie->m_pstThumbFrame->m_wPx - pstFirst->m_wPx;
			pstMovie->m_wFrameY = pstMovie->m_pstThumbFrame->m_wPy - pstFirst->m_wPy;
		}

		// caculate thumb column
		if (pstMovie->m_dwThumbCount > 0)
		{
			pstMovie->m_dwThumbColumns = 0;
			for (i = 1; i < pstMovie->m_dwThumbCount; i++)
			{
				STXPGSPRITE *pstSprite0 = pstMovie->m_astThumb[i - 1].m_pstSprite;
				STXPGSPRITE *pstSprite1 = pstMovie->m_astThumb[i].m_pstSprite;

				if (pstSprite1->m_wPy > pstSprite0->m_wPy && pstSprite1->m_wPx < pstSprite0->m_wPx)
				{
					pstMovie->m_dwThumbColumns = i;
					break;
				}
			}
		}
	}
#endif

}

//---------------------------------------------------------------------------
///
///@ingroup xpgMovie
///@brief   Load Movie's global page(page 0)'s commands
///
///@param   pstMovie - pointer of Movie
///
///@return
///
void xpgMovieLoadGlobalCommand(register STXPGMOVIE * pstMovie)
{
	register DWORD i, j;
	STXPGPAGE *pstPage = NULL;
	DWORD dwCmdCount;

	MP_DEBUG("xpgMovieLoadGlobalCommand");
#if XPG_GLOBAL_CMD_COUNT
	pstPage = &(pstMovie->m_astPage[0]);	//xpgGetPage( pstMovie, iPage );

	dwCmdCount = pstPage->m_dwCmdCount;
	if (dwCmdCount>XPG_GLOBAL_CMD_COUNT)
		dwCmdCount=XPG_GLOBAL_CMD_COUNT;
	pstMovie->m_dwGlobalCmdCount = dwCmdCount;
	pstMovie->m_dwCmdCount = dwCmdCount;

	for (i = 0; i < dwCmdCount; i++)
	{
		for (j = 0; j < 6; j++)  // add key event and action param - 07.24.2006 Athena
		{
			pstMovie->m_dwCommand[i][j] = pstPage->m_wCommand[(i * 6) + j]; // 20070517 - command count defined by xpg file
		}
	}
#else
	pstMovie->m_dwGlobalCmdCount = 0;
#endif
}

//---------------------------------------------------------------------------
///
///@ingroup xpgMovie
///@brief   Load specified page's commands
///
///@param   pstMovie - pointer of Movie
///
///@param   iPage - specified page index
///@return
///
void xpgMovieLoadPageCommand(register STXPGMOVIE * pstMovie, int iPage)
{
	register DWORD i, j;
	STXPGPAGE *pstPage = NULL;
	DWORD dwCmdCount;
	DWORD dwFirstCmd;

	pstPage = &(pstMovie->m_astPage[iPage]);

	dwCmdCount = pstPage->m_dwCmdCount;
	dwFirstCmd = pstMovie->m_dwGlobalCmdCount;
	pstMovie->m_dwCmdCount = dwFirstCmd + dwCmdCount;

	for (i = 0; i < dwCmdCount; i++)
	{
		for (j = 0; j < 6; j++) // add key event and action param - 07.24.2006 Athena
		{
			pstMovie->m_dwCommand[dwFirstCmd + i][j] = pstPage->m_wCommand[(i * 6) + j]; // 20070517 - command count defined by xpg file
		}
	}
}


//-----------------------------------------------------------------------------
///
///@ingroup xpgMovie
///@brief   Find specified sprite type and index
///
///@param   pstMovie - pointer of Movie
///
///@param   dwType - specified sprite type
///
///@param   dwIndex - specified index
///@return
///
STXPGSPRITE *xpgSpriteFindType(register STXPGMOVIE * pstMovie, DWORD dwType, DWORD dwIndex)
{
	register STXPGSPRITE *pstSprite;
	register int i;
	register int iSpriteCount = pstMovie->m_dwSpriteCount;

	for (i = 0; i < iSpriteCount; i++)
	{
		pstSprite = &(pstMovie->m_astSprite[i]);
		if (dwType == pstSprite->m_dwType && dwIndex == pstSprite->m_dwTypeIndex)
		{
			return pstSprite;
		}
	}
	return NULL;
}
//---------------------------------------------------------------------------
///
///@ingroup xpgMovie
///@brief   Search specified page name's page
///
///@param   name - specified page name
///
///@return  STXPGPAGE - selected page
///
STXPGPAGE *xpgMovieSearchPage(const char *name)
{
	register DWORD iPage;
	register DWORD dwSearchKey;

	dwSearchKey = xpgHash(name);
	for (iPage = 1; iPage < g_pstXpgMovie->m_wPageCount; iPage++)
	{
		STXPGPAGE *pstPage = &(g_pstXpgMovie->m_astPage[iPage]);

		if (dwSearchKey == pstPage->m_dwHashKey)
		{
			return pstPage;
		}
	}
	return NULL;
}

///
///@brief   Init Movie ObjRole
///
///@param   pstMovie - pointer of Movie
///
void xpgMovieInitObjRole(register STXPGMOVIE * pstMovie)
{
    MP_DEBUG("xpgMovieInitObjRole");
    int i;

    // allocate object roles by xpg page 0 sprite objects
    MP_DEBUG("pstMovie->m_dwSpriteCount %d", pstMovie->m_dwSpriteCount);
    if (pstMovie->m_dwSpriteCount == 0)
        return;

    if (pstMovie->m_pstObjRole == NULL)
        pstMovie->m_pstObjRole = (STXPGROLE **)xpgMalloc(pstMovie->m_dwSpriteCount * 4);

    for (i = 0; i < pstMovie->m_dwSpriteCount; i++)
    {
        pstMovie->m_pstObjRole[i] = pstMovie->m_astSprite[i].m_pstRole;  // only init pointer
    }
}


//-----------------------------------------------------------------------------
