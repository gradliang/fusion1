//---------------------------------------------------------------------------

#include "xpg.h"
#include "xpgPage.h"

//---------------------------------------------------------------------------
void xpgPageInit(STXPGPAGE * pstPage)
{
}

//---------------------------------------------------------------------------
///
///@ingroup xpgPage
///
///@brief   Clear specified page's parameters
///
///@param   pstPage - specified page   
/// 
///@return  
/// 
void xpgPageRelease(STXPGPAGE * pstPage)
{
	xpgPageClear(pstPage);
}

//---------------------------------------------------------------------------
///
///@ingroup xpgPage
///@brief   Free specified sprite's memory and set specified page's sprite count = 0
///
///@param   pstPage - specified page   
/// 
///@return  
/// 
void xpgPageClear(STXPGPAGE * pstPage)
{
	xpgFree(pstPage->m_astSprite);
	pstPage->m_wSpriteCount = 0;
}

//---------------------------------------------------------------------------
///
///@ingroup xpgPage
///@brief   Get specified page's specified sprite
///
///@param   pstPage - specified page   
/// 
///@param   iSprite - sprite index
///@return  
/// 
STXPGPAGESPRITE *xpgPageGetSprite(STXPGPAGE * pstPage, DWORD iSprite)
{
	if (iSprite >= pstPage->m_wSpriteCount)
		return NULL;
	else
		return &(pstPage->m_astSprite[iSprite]);
}


//---------------------------------------------------------------------------
