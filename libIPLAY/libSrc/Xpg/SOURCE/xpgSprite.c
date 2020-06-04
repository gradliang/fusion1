//---------------------------------------------------------------------------
/*
 	define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"
#include "xpg.h"
#include "xpgRole.h"
#include "xpgSprite.h"

//---------------------------------------------------------------------------
xpg_sprite_frame_t *xpgSpriteGetFrame(STXPGPAGESPRITE *pstSprite, int i)
{
    if (pstSprite == NULL || pstSprite->m_astFrame == NULL)
        return NULL;

    if (i > pstSprite->m_iFrameCount)
        return NULL;

    return &pstSprite->m_astFrame[i];
}

//---------------------------------------------------------------------------
void xpgSpriteSetFrameCount(STXPGPAGESPRITE *pstSprite, int n)
{
	if (pstSprite == NULL)
		return;
	pstSprite->m_iFrameCount = n;
}
//---------------------------------------------------------------------------
/* page pstSprite only allocate key frame array */
void xpgSpriteSetKeyFrameCount(STXPGPAGESPRITE *pstSprite, int n)
{
	if (pstSprite == NULL)
		return;
	pstSprite->m_iKeyFrameCount = n;
	pstSprite->m_astFrame = (xpg_sprite_frame_t *)xpgCalloc(sizeof(xpg_sprite_frame_t), n);
	if (pstSprite->m_astFrame)
	{
        memset(pstSprite->m_astFrame, 0, sizeof(xpg_sprite_frame_t) * n);
	}
}

//---------------------------------------------------------------------------
///
///@ingroup xpgSprite
///@brief   Set Role struct parameters
///
///@param   pstSprite - specified Sprite
///
///@param   pstRole - specified Role
///
void xpgSpriteSetRole(register STXPGSPRITE * pstSprite, register STXPGROLE * pstRole)
{
	if (pstSprite == NULL || pstRole == NULL)
		return;
	pstSprite->m_pstRole = pstRole;
	pstSprite->m_wRole = pstRole->m_wIndex;
	pstSprite->m_wWidth = pstRole->m_wWidth;
	pstSprite->m_wHeight = pstRole->m_wHeight;
	pstSprite->m_pImage = pstRole->m_pImage;
}

//---------------------------------------------------------------------------
///
///@ingroup xpgSprite
///@brief   Move sprite to input destination
///
///@param   pstSprite - specified Sprite
///
///@param   dstX - left
///
///@param   dstY - top
///
void xpgSpriteMoveTo(register STXPGSPRITE * pstSprite, register DWORD dstX, register DWORD dstY)
{
	if (pstSprite == NULL)
		return;
	pstSprite->m_wPx = dstX;
	pstSprite->m_wPy = dstY;
	pstSprite->m_wLeft = dstX;
	pstSprite->m_wTop = dstY;
	pstSprite->m_wRight = pstSprite->m_wLeft + pstSprite->m_wWidth - 1;
	pstSprite->m_wBottom = pstSprite->m_wTop + pstSprite->m_wHeight - 1;
}

//---------------------------------------------------------------------------
///
///@ingroup xpgSprite
///@brief   Move sprite to input position
///
///@param   pstSprite - specified Sprite
///
///@param   x - left
///
///@param   y - top
///
void xpgSpriteMove(register STXPGSPRITE * pstSprite, register DWORD x, register DWORD y)
{
	if (pstSprite == NULL)
		return;
	pstSprite->m_wPx += x;
	pstSprite->m_wPy += y;
	pstSprite->m_wLeft = pstSprite->m_wPx;
	pstSprite->m_wTop = pstSprite->m_wPy;
	pstSprite->m_wRight = pstSprite->m_wLeft + pstSprite->m_wWidth - 1;
	pstSprite->m_wBottom = pstSprite->m_wTop + pstSprite->m_wHeight - 1;
}

//---------------------------------------------------------------------------
///
///@ingroup xpgSprite
///@brief   Check Sprite inside
///
///@param   pstSprite - specified Sprite
///
///@param   x - left
///
///@param   y - top
///
///@retval  BOOL 
///
BOOL xpgSpriteInside(register STXPGSPRITE * pstSprite, register DWORD x, register DWORD y)
{
	return (x > pstSprite->m_wLeft && x < pstSprite->m_wRight &&
			y > pstSprite->m_wTop && y < pstSprite->m_wBottom);
}

#define unfixed(v) ((v) >> 12)
#define fixed(v)   ((v) * (1 << 12))

void xpgSpritePosFrameInbetween(STXPGSPRITE *pstSprite)
{
    MP_DEBUG("xpgSpritePosFrameInbetween()");
    int i, j;
	xpg_sprite_frame_t *pPrevKeyFrame = NULL;
	xpg_sprite_frame_t *pNextKeyFrame = NULL;

	for (i = 0; i < pstSprite->m_wFrameCount; i++)
	{
		xpg_sprite_frame_t *pFrame = &pstSprite->m_astFrame[i];

		/* position animation */
		if (i == 0 || (pFrame->m_iAction & ACT_POS))
		{
			/* search next key frame */
			pPrevKeyFrame = pFrame;
			pNextKeyFrame = NULL;
			for (j = i + 1 ; j < pstSprite->m_wFrameCount; j++)
			{
				pFrame = &pstSprite->m_astFrame[j];
				if (pFrame->m_iAction & ACT_POS)
				{
					pNextKeyFrame = pFrame;
					break;
				}
			}

			/* set in-between values */
			int n =  0;
			int dx = 0;
			int dy = 0;
			int x0 = pPrevKeyFrame->m_alValue[SET_POS_X];
			int y0 = pPrevKeyFrame->m_alValue[SET_POS_Y];
			int f0 = pPrevKeyFrame->m_iFrame;
			int f1 = pstSprite->m_wFrameCount;
            int fn;

			if (pNextKeyFrame != NULL)
			{
				f1 = pNextKeyFrame->m_iFrame;
				n = f1 - f0;

				if (n > 1)
				{
					dx = fixed(pNextKeyFrame->m_alValue[SET_POS_X] - x0) / (n);
					dy = fixed(pNextKeyFrame->m_alValue[SET_POS_Y] - y0) / (n);
				}
			}

			for (j = f0 + 1, fn = 1; j < f1; j++, fn++)
			{
				pFrame = &pstSprite->m_astFrame[j];
				if (pFrame)
				{
				    pFrame->m_iAction |= ACT_POS;
					pFrame->m_alValue[SET_POS_X] = x0 + unfixed(dx * fn);
					pFrame->m_alValue[SET_POS_Y] = y0 + unfixed(dy * fn);
				}
			}

			if (pNextKeyFrame == NULL)
			{
				break;
			}

			i += n-1;
			pPrevKeyFrame = pNextKeyFrame;
			pNextKeyFrame = NULL;
		}
	}
}
void xpgSpriteSizeFrameInbetween(STXPGSPRITE *pstSprite)
{
    MP_DEBUG("xpgSpriteSizeFrameInbetween");
    int i, j;
	xpg_sprite_frame_t *pPrevKeyFrame = NULL;
	xpg_sprite_frame_t *pNextKeyFrame = NULL;

	for (i = 0; i < pstSprite->m_wFrameCount; i++)
	{
		xpg_sprite_frame_t *pFrame = &pstSprite->m_astFrame[i];

		/* position animation */
		if (i == 0 || (pFrame->m_iAction & ACT_SIZE))
		{
			/* search next key frame */
			pPrevKeyFrame = pFrame;
			pNextKeyFrame = NULL;
			for (j = i + 1 ; j < pstSprite->m_wFrameCount; j++)
			{
				pFrame = &pstSprite->m_astFrame[j];
				if (pFrame->m_iAction & ACT_SIZE)
				{
					pNextKeyFrame = pFrame;
					break;
				}
			}

			/* set in-between values */
			int n =  0;
			int dw = 0;
			int dh = 0;
			int w0 = pPrevKeyFrame->m_alValue[SET_SIZE_W];
			int h0 = pPrevKeyFrame->m_alValue[SET_SIZE_H];
			int f0 = pPrevKeyFrame->m_iFrame;
			int f1 = pstSprite->m_wFrameCount;
            int fn;

			if (pNextKeyFrame != NULL)
			{
				f1 = pNextKeyFrame->m_iFrame;
				n = f1 - f0;

				if (n > 1)
				{
					dw = fixed(pNextKeyFrame->m_alValue[SET_SIZE_W] - w0) / (n);
					dh = fixed(pNextKeyFrame->m_alValue[SET_SIZE_H] - h0) / (n);
				}
			}

			for (j = f0 + 1, fn = 1; j < f1; j++, fn++)
			{
				pFrame = &pstSprite->m_astFrame[j];
				if (pFrame)
				{
				    pFrame->m_iAction |= ACT_SIZE;
					pFrame->m_alValue[SET_SIZE_W] = w0 + unfixed(dw * fn);
					pFrame->m_alValue[SET_SIZE_H] = h0 + unfixed(dh * fn);
				}
			}

			if (pNextKeyFrame == NULL)
			{
				break;
			}

			i += n-1;
			pPrevKeyFrame = pNextKeyFrame;
			pNextKeyFrame = NULL;
		}
	}
}
//------------------------------------------------------------------------
void xpgSpriteActionInbetween(STXPGSPRITE *pstSprite, DWORD dwAct)
{
    MP_DEBUG1("xpgSpriteActionInbetween dwAct=0x%X(6-SET_BLEND, 7-SET_ANGLE)", dwAct);
    int i, j;
	xpg_sprite_frame_t *pPrevKeyFrame = NULL;
	xpg_sprite_frame_t *pNextKeyFrame = NULL;
	DWORD dwActFlag = (1 << dwAct);
	MP_DEBUG("\tpstSprite->m_wFrameCount = %d, dwActFlag = 0x%X(0x80-Angle)", pstSprite->m_wFrameCount, dwActFlag);
	for (i = 0; i < pstSprite->m_wFrameCount; i++)
	{
	    MP_DEBUG("\ti=%d(i<pstSprite->m_wFrameCount)", i);
		xpg_sprite_frame_t *pFrame = &pstSprite->m_astFrame[i];
		MP_DEBUG("\tpFrame->m_iAction = 0x%X", pFrame->m_iAction);

		/* position animation */
		if (i == 0 || (pFrame->m_iAction & dwActFlag))
		{
			/* search next key frame */
			pPrevKeyFrame = pFrame;
			pNextKeyFrame = NULL;
			for (j = i + 1 ; j < pstSprite->m_wFrameCount; j++)
			{
				pFrame = &pstSprite->m_astFrame[j];
				if (pFrame->m_iAction & dwActFlag)
				{
					pNextKeyFrame = pFrame;
					break;
				}
			}
            MP_DEBUG("\tFind NextKeyFrame = %d", j);
			/* set in-between values */
			int n =  0;
			int dv = 0;
			int v0 = pPrevKeyFrame->m_alValue[dwAct];
			int f0 = pPrevKeyFrame->m_iFrame;
			int f1 = pstSprite->m_wFrameCount;
            int fn;

			if (pNextKeyFrame != NULL)
			{
				f1 = pNextKeyFrame->m_iFrame;
				n = f1 - f0;

				if (n > 1)
				{
					dv = (fixed(pNextKeyFrame->m_alValue[dwAct] - v0)) / (n);
				}
			}

			for (j = f0 + 1, fn = 1; j < f1; j++, fn++)
			{
				pFrame = &pstSprite->m_astFrame[j];
				if (pFrame)
				{
				    pFrame->m_iAction |= dwActFlag;
					pFrame->m_alValue[dwAct] = v0 + unfixed(dv * fn);
					//MP_DEBUG("\t\tpFrame->m_alValue[dwAct] = %d", pFrame->m_alValue[dwAct]);
				}
			}

			if (pNextKeyFrame == NULL)
			{
				break;
			}

			i += n-1;
			pPrevKeyFrame = pNextKeyFrame;
			pNextKeyFrame = NULL;
		}
	}
}

//---------------------------------------------------------------------------
void xpgSpriteFramesFree(STXPGMOVIE * pstMovie, STXPGSPRITE * pstDst, STXPGPAGESPRITE * pstSrc)
{
    int i, iFrame, n = pstSrc->m_iFrameCount;
    // TODO
    //xpgFree(pstDst->m_astFrame);
    i = 0;
	for (iFrame = 0; iFrame < n; iFrame++)
	{
        //xpgFree(pDstFrame->m_alValue);
    }
}
void xpgSpriteCopyFrames(STXPGMOVIE * pstMovie, STXPGSPRITE * pstDst,
				   STXPGPAGESPRITE * pstSrc)
{
    MP_DEBUG("xpgSpriteCopyFrames");
    int i, j, a, act, iFrame;
    int n = pstSrc->m_iFrameCount;
    DWORD dwActFlag = 0;
    xpg_sprite_frame_t *pSrcFrame, *pDstFrame, *pSrcNextFrame;

    n = pstSrc->m_iFrameCount;
	pstDst->m_wFrameCount = n;
	if(n <= 0)
        return;

	/* allocate dst frame array */
	pstDst->m_astFrame = (xpg_sprite_frame_t *)xpgCalloc(sizeof(xpg_sprite_frame_t), n);
	if (pstDst->m_astFrame)
	{
	    /* clear array content */
        memset(pstDst->m_astFrame, 0, sizeof(xpg_sprite_frame_t) * n);
	}

	/* copy from src key frame array to dst frame array */
	MP_DEBUG("\tn = pstSrc->m_iFrameCount = pstDst->m_wFrameCount = %d", n);

	i = 0;
	for (iFrame = 0; iFrame < n; iFrame++)
	{
	    /* get source frame */
        pSrcFrame = &pstSrc->m_astFrame[i];

        if (i >= pstSrc->m_iKeyFrameCount - 1)
            pSrcNextFrame = &pstSrc->m_astFrame[i];
        else
            pSrcNextFrame = &pstSrc->m_astFrame[i+1];

        /* get destination frame */
        pDstFrame = &pstDst->m_astFrame[iFrame];

        /* copy frame content */
        pDstFrame->m_iFrame = iFrame;
        if (pSrcFrame->m_iFrame == pDstFrame->m_iFrame)
            pDstFrame->m_iAction = pSrcFrame->m_iAction;
        else
            pDstFrame->m_iAction = 0;
        //mpDebugPrint("ACT_COUNT = %d", ACT_COUNT);    
        pDstFrame->m_alValue = (long *)xpgMalloc(sizeof(long) * ACT_COUNT);

        /* clear value array */
        memset(pDstFrame->m_alValue, 0, sizeof(long) * ACT_COUNT);

        /* copy and decompress action values */
        j = 0; /* j : source frame value index */
        act = pSrcFrame->m_iAction;
        for (a = 0; a < ACT_COUNT; a++)
        {
            if (act & (1 << a))
            {
                pDstFrame->m_alValue[a] = pSrcFrame->m_alValue[j];
                j++;
            }
        }

        /* check if with position animation for later do in-between */
        dwActFlag |= act;

        /* check if have to switch to next key frame */
        if (iFrame + 1 >= pSrcNextFrame->m_iFrame)
        {
            i++;
        }
	}

	if (dwActFlag & ACT_POS)
	{
        xpgSpritePosFrameInbetween(pstDst);
	}
	if (dwActFlag & ACT_ANGLE)
	{
		xpgSpriteActionInbetween(pstDst, SET_ANGLE);
	}
	if (dwActFlag & ACT_BLEND)
	{
		xpgSpriteActionInbetween(pstDst, SET_BLEND);
	}
	if (dwActFlag & ACT_SIZE)
	{
        xpgSpriteSizeFrameInbetween(pstDst);
	}

}

//---------------------------------------------------------------------------
///
///@ingroup xpgSprite
///@brief   Copy Sprite to destination
///
///@param   pstMovie - specified Movie
///
///@param   pstDst - specified Destination Sprite
///
///@param   pstSrc - specified source Sprite
///
void xpgSpriteCopy(register STXPGMOVIE * pstMovie, register STXPGSPRITE * pstDst,
				   register STXPGPAGESPRITE * pstSrc)
{
    //mpDebugPrint("xpgSpriteCopy()");
    pstDst->m_bFlag = 0;
	//pstDst->role_type = 0;
	pstDst->m_boExist = true;
//******************************************************************************		
	pstDst->m_boVisible = true;
// *** Default, Initial , all sprites'm_boVisible are set true,
// *** Except in later switch (pstDst->m_dwType) --> set false, that will set true by runtime
// *** Example, SPRITE_TYPE_ICON: // type6 --> which command icon 
// ***          SPRITE_TYPE_CARD: // type8 --> SystemCardPresentCheck
// ***          11: /// type11 - NoDraw
// ***          28: /// type28 - NoShowSprite
//******************************************************************************	
	
	pstDst->m_bScaleW = 100;
	pstDst->m_bScaleH = 100;
	pstDst->m_iGotoFrame = -1;
	pstDst->m_iCurLoopGoto = -1;
	pstDst->m_iCurLoopFrame = -1;
	pstDst->m_iGotoRepeat = -1;
	pstDst->m_iGotoRepeated = 0;

	DWORD iRole;

	iRole = pstSrc->m_wRole;
	if (iRole >= pstMovie->m_wRoleCount)
		iRole = 0;
	xpgSpriteSetRole(pstDst, &(pstMovie->m_astRole[iRole]));

	/* copy pstSprite frame count */
	pstDst->m_wCurFrame = 0;
    pstDst->m_wFrameCount = pstSrc->m_iFrameCount;
    /* copy and decode pstSprite frames and set frame in-between */
    if (pstSrc->m_iFrameCount > 0)
    {
        xpgSpriteCopyFrames(pstMovie, pstDst, pstSrc);
    }

    // *** copy from PAGESPRITE to SPRITE, ref xpgReader.c, xpgReadPageSpriteData()
    DWORD x, y;
	x = pstSrc->m_wPx;
	y = pstSrc->m_wPy;
	xpgSpriteMoveTo(pstDst, x, y);
	
	pstDst->m_wLayer = pstSrc->m_wLayer;
	pstDst->m_dwInk = pstSrc->m_dwInk;
	pstDst->m_dwHashKey = pstSrc->m_dwHashKey;
	pstDst->m_dwType = (pstSrc->m_dwType >> 16) & 0xffff;
	pstDst->m_dwTypeIndex = pstSrc->m_dwType & 0xffff;
	// *********************************************
	
	pstDst->m_wTextLen = pstSrc->m_wTextLen;
	pstDst->m_pText = pstSrc->m_Text;

#if 0
	register DWORD iTypeIndex = pstDst->m_dwTypeIndex;

	switch (pstDst->m_dwType)
	{
#if XPG_THUMB_COUNT
	case SPRITE_TYPE_THUMB: // type2
		if (iTypeIndex == 0xff)
			iTypeIndex = 0;
		if (iTypeIndex >= pstMovie->m_dwThumbCount)
		{
			pstMovie->m_dwThumbCount = iTypeIndex + 1;
			//pstMovie->m_dwThumbBufferCount = iTypeIndex + 1;
		}
		pstMovie->m_astThumb[iTypeIndex].m_pstSprite = pstDst;
		//STXPGTHUMB *pThumb = &(pstMovie->m_astThumb[iTypeIndex]);
		//if (pThumb->m_pImage == NULL)
		//  pThumb->m_pImage = xpgMalloc(pstDst->m_wWidth * pstDst->m_wHeight * 2);
		break;
	case SPRITE_TYPE_THUMBFRAME: // type3
		pstMovie->m_pstThumbFrame = pstDst;
		break;
#endif
#if XPG_LIST_COUNT
	case SPRITE_TYPE_LIST: // type4
		if (iTypeIndex >= pstMovie->m_dwListCount)
			pstMovie->m_dwListCount = iTypeIndex + 1;
		pstMovie->m_astList[iTypeIndex].m_pstSprite = pstDst;
		break;
	case SPRITE_TYPE_LISTFRAME: // type5
		pstMovie->m_pstListFrame = pstDst;
		break;
#endif
#if SPRITE_TYPE_ICON
	case SPRITE_TYPE_ICON: // type6
		pstDst->m_boVisible = false;
        break;
#endif
#if XPG_LIST_COUNT
	case SPRITE_TYPE_LISTICON: // type7
		pstMovie->m_astList[iTypeIndex].m_pstListIcon = pstDst;
		break;
#endif
#if XPG_CARD_COUNT
	case SPRITE_TYPE_CARD: // type8
		pstDst->m_boVisible = false; // runtime set true by CurCard & SystemPresentCheck
		if (iTypeIndex >= pstMovie->m_dwCardCount)
			pstMovie->m_dwCardCount = iTypeIndex + 1;
		break;
#endif
#if XPG_SCROLLBAR_COUNT
	case SPRITE_TYPE_SCROLLBAR: // type10
		pstDst->m_boVisible = false;
		pstMovie->m_pstScrollBar[iTypeIndex] = pstDst;
		break;
#endif
	}
#endif

}

/////////////////////////////////////////////////////////////////////////////
// dialog support
///////////////////////////

#define DIALOG_MAX_SPRITE_CNT       50

typedef struct {
    XPGEXTRASPRITE  spriteList[DIALOG_MAX_SPRITE_CNT];
    DWORD           spriteCount;
    int             dailogId;
    //char            backToPage[32];
	DWORD dwReturnPageIndex;
    ST_IMGWIN       backupWin;
}XPGDIALOGSTACK;

#define DIALOG_STACK_SIZE           3
static XPGDIALOGSTACK  dialogStacks[DIALOG_STACK_SIZE];
static DWORD dialogCount = 0;


void xpgExtraSpriteCopy(STXPGSPRITE * pstDst, XPGEXTRASPRITE * pstSrc)
{
    memset(pstDst, 0, sizeof(STXPGSPRITE));
    pstDst->m_dwType = pstSrc->m_dwType;
    pstDst->m_dwTypeIndex = pstSrc->m_dwTypeIndex;
    pstDst->m_bFlag = pstSrc->m_bFlag;
    pstDst->m_boVisible = true;
    pstDst->m_boExist = true;
    pstDst->m_wWidth = 16;
    pstDst->m_wHeight = 16;

    DWORD x, y;
    x = 0;
    y = 0;
    xpgSpriteMoveTo(pstDst, x, y);
}

DWORD getCurDialogExtraSpriteCount()
{
    DWORD curDialogIndex;
    if (dialogCount == 0 || dialogCount > DIALOG_STACK_SIZE)
        return 0;
    curDialogIndex = dialogCount - 1;
    XPGDIALOGSTACK * pstCurDialogInfo = &dialogStacks[curDialogIndex];
    return pstCurDialogInfo->spriteCount;
}

XPGEXTRASPRITE* getCurDialogExtraSpriteList()
{
    DWORD curDialogIndex;
    if (dialogCount == 0 || dialogCount > DIALOG_STACK_SIZE)
        return NULL;
    curDialogIndex = dialogCount - 1;
    XPGDIALOGSTACK * pstCurDialogInfo = &dialogStacks[curDialogIndex];
    return pstCurDialogInfo->spriteList;
}

int xpgAddDialog(int dialogId, DWORD dwReturnPageIndex, ST_IMGWIN* backupWin)
{
    DWORD curDialogIndex;
    if (dialogCount >= 3)
    {
        mpDebugPrint("Dialog count too large, have exist %d dialogs. ", dialogCount);
        return -1;
    }
    curDialogIndex = dialogCount;
    XPGDIALOGSTACK * pstCurDialogInfo = &dialogStacks[curDialogIndex];
    pstCurDialogInfo->dailogId = dialogId;
    pstCurDialogInfo->spriteCount = 0;
    pstCurDialogInfo->dwReturnPageIndex= dwReturnPageIndex;

	ST_IMGWIN *pCacheWin = &pstCurDialogInfo->backupWin;

    if (pCacheWin->pdwStart != NULL)
    {
        ext_mem_free(pCacheWin->pdwStart);
		pCacheWin->pdwStart = NULL;
    }

	if (backupWin !=NULL && backupWin->pdwStart !=NULL)
	{
		ImgWinInit(pCacheWin, NULL, backupWin->wHeight, backupWin->wWidth);
		pCacheWin->pdwStart = ext_mem_malloc(backupWin->wWidth * backupWin->wHeight * 2);
		mpCopyEqualWin(pCacheWin, backupWin);
	}
	
    dialogCount++;
    mpDebugPrint("xpgAddDialog OK, index = %d", curDialogIndex);
    return PASS;
}

int xpgDeleteDialog()
{
    DWORD curDialogIndex;
    XPGDIALOGSTACK * pstCurDialogInfo;
    
    if (dialogCount > 0)
    {
        curDialogIndex = dialogCount - 1;
        pstCurDialogInfo = &dialogStacks[curDialogIndex];
        ST_IMGWIN *pCacheWin = &pstCurDialogInfo->backupWin;
        if (pCacheWin->pdwStart != NULL)
        {
            ext_mem_free(pCacheWin->pdwStart);
    		pCacheWin->pdwStart = NULL;
        }
        
        dialogCount --;
    }
    mpDebugPrint("xpgDeleteDialog OK, dialogCount = %d", dialogCount);
    return PASS;
}

int xpgDeleteAllDialog()
{
    DWORD i;
    XPGDIALOGSTACK * pstCurDialogInfo;
    
    dialogCount = 0;
    mpDebugPrint("xpgDeleteAllDialog");
    for (i = 0; i < DIALOG_STACK_SIZE; i++)
    {
        pstCurDialogInfo = &dialogStacks[i];
        if (pstCurDialogInfo->backupWin.pdwStart != NULL)
        {
            ext_mem_free(pstCurDialogInfo->backupWin.pdwStart);
            pstCurDialogInfo->backupWin.pdwStart = NULL;
        }
    }
    return PASS;
}

int xpgGetCurrDialogTypeId()
{
    DWORD curDialogIndex;
    XPGDIALOGSTACK * pstCurDialogInfo;
    
    if (dialogCount == 0 || dialogCount > DIALOG_STACK_SIZE)
        return 0;
    curDialogIndex = dialogCount - 1;
    pstCurDialogInfo = &dialogStacks[curDialogIndex];
    return pstCurDialogInfo->dailogId;
}

ST_IMGWIN* xpgGetCurrDialogCacheWin()
{
    DWORD curDialogIndex;
    XPGDIALOGSTACK * pstCurDialogInfo;
    
    if (dialogCount == 0 || dialogCount > DIALOG_STACK_SIZE)
        return NULL;
    curDialogIndex = dialogCount - 1;
    pstCurDialogInfo = &dialogStacks[curDialogIndex];
    return & pstCurDialogInfo->backupWin;
}

DWORD xpgGetCurrDialogBackPage()
{
    DWORD curDialogIndex;
    XPGDIALOGSTACK * pstCurDialogInfo;
    
    if (dialogCount == 0 || dialogCount > DIALOG_STACK_SIZE)
        return 0;
    curDialogIndex = dialogCount - 1;
    pstCurDialogInfo = &dialogStacks[curDialogIndex];
    return pstCurDialogInfo->dwReturnPageIndex;
}


int xpgAddDialogSprite(WORD m_dwType, WORD m_dwTypeIndex, BYTE flag)
{
    DWORD curDialogIndex;
    XPGEXTRASPRITE * pstExtraSprite;
    
    if (dialogCount == 0 || dialogCount > DIALOG_STACK_SIZE)
    {
        mpDebugPrint("xpgAddDialogSprite error, dialogCount=%d", dialogCount);
        return -1;
    }
    curDialogIndex = dialogCount - 1;
    XPGDIALOGSTACK * pstCurDialogInfo = &dialogStacks[curDialogIndex];
    if (pstCurDialogInfo->spriteCount >= DIALOG_MAX_SPRITE_CNT)
    {
        mpDebugPrint("xpgAddDialogSprite error, spriteCount=%d", pstCurDialogInfo->spriteCount);
        return -1;
    }
    pstExtraSprite = &(pstCurDialogInfo->spriteList[pstCurDialogInfo->spriteCount]);
    pstExtraSprite->m_dwType = m_dwType;
    pstExtraSprite->m_dwTypeIndex = m_dwTypeIndex;
    pstExtraSprite->m_bFlag = flag;

    pstCurDialogInfo->spriteCount ++;
    return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////


void xpgSpriteSetTouchArea (STXPGSPRITE * pstSprite, WORD startX, WORD startY, WORD width, WORD height) 
{
    if (pstSprite == NULL)
        return;
    pstSprite->m_touchInfo.m_touchAreaX = startX;
    pstSprite->m_touchInfo.m_touchAreaY = startY;
    pstSprite->m_touchInfo.m_touchAreaW = width;
    pstSprite->m_touchInfo.m_touchAreaH = height;
    pstSprite->m_touchInfo.m_boEnable = 1;
}

void xpgSpriteEnableTouch(STXPGSPRITE * pstSprite)
{
    xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, pstSprite->m_wHeight);
}

void xpgSpriteDisableTouch(STXPGSPRITE * pstSprite)
{
    if (pstSprite == NULL)
        return;
    pstSprite->m_touchInfo.m_touchAreaX = 0;
    pstSprite->m_touchInfo.m_touchAreaY = 0;
    pstSprite->m_touchInfo.m_touchAreaW = 0;
    pstSprite->m_touchInfo.m_touchAreaH = 0;
    pstSprite->m_touchInfo.m_boEnable = 0;
}



//---------------------------------------------------------------------------






