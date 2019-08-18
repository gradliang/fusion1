//---------------------------------------------------------------------------

/*
 	define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#include "xpg.h"
#include "xpgUtil.h"

#define XPG_LOG_FILE    0 // start from xpgReadMovie()
STREAM *xpglogHandle = NULL;
BYTE bDataBuf[80];

static XPGBOOL check_tag(XPGFILE *fp, const char *tag, int n)
{
	int i;
	char buf[8];

	xpgFileRead(buf, n, 1, fp);

	for (i = 0; i < n; i++)
	{
		if (buf[i] != tag[i])
		{
			return false;
		}
	}
	return true;
}
//---------------------------------------------------------------------------
static void _swap_buffer_endian(unsigned char *buffer, int size)
{
    unsigned char b[4];
    int i;
	int n = size % 4;

	if (n > 0)
		size += 4 - n;

	for (i = 0; i < size; i += 4)
	{
		b[0] = buffer[i+3];
		b[1] = buffer[i+2];
		b[2] = buffer[i+1];
		b[3] = buffer[i+0];
		buffer[i+3] = b[3];
		buffer[i+2] = b[2];
		buffer[i+1] = b[1];
		buffer[i+0] = b[0];
	}
}
//----------------------------------------------------------------------------
///
///@ingroup xpgReader
///@brief   Read next dword from file
///
///@param   fp - file pointer   
/// 
///@return  DWORD - the return DWORD value
/// 
static DWORD xpgReadNextDWord(XPGFILE * fp)
{
	DWORD dwValue;

	xpgFileRead(&dwValue, 4, 1, fp);
	return dwValue;
}
//----------------------------------------------------------------------------
///
///@ingroup xpgReader
///@brief   Read next word from file
///
///@param   fp - file pointer   
/// 
///@return  WORD - the return WORD value
/// 
static WORD xpgReadNextWord(XPGFILE * fp)
{
	WORD wValue;

	xpgFileRead(&wValue, 2, 1, fp);
	return wValue;
}

//----------------------------------------------------------------------------
static WORD xpgReadNextByte(XPGFILE * fp)
{
	BYTE value;

	xpgFileRead(&value, 1, 1, fp);

	//MP_DEBUG1("%04x ", value);
	return value;
}
//----------------------------------------------------------------------------
static WORD xpgReadString(XPGFILE * fp, char *buffer, int size)
{
    MP_DEBUG("xpgReadString(size=%d)", size);
	xpgFileRead(buffer, size, 1, fp);
	//if (size >= 4)
    //swap_buffer_endian(buffer, size - (size % 4));
	//MP_DEBUG1("%04x ", value);
	MP_DEBUG("buffer=%s", buffer);
	// re-adjust String size length from 4 align to real length
	if(*(buffer+size-3) == 0x00) 
        size -= 3;
	else if(*(buffer+size-2) == 0x00) 
        size -= 2;
	else if(*(buffer+size-1) == 0x00) 
        size -= 1;
    MP_DEBUG("After adjust, size = %d", size);
	return size;
}

//----------------------------------------------------------------------------
///
///@ingroup xpgReader
///@brief   Read Movie header
///
///@param   fp - file pointer   
/// 
///@param   pstMovie - pointer of Movie
///@return  true or false
/// 
DWORD startPage = 0;
DWORD xpgGetStartpage()
{
    return startPage;
} 
XPGBOOL xpgReadMovieHeader(register STXPGMOVIE * pstMovie, XPGFILE * fp)
{
	if (fp == NULL)
		return false;

	//temp skip header info
	/* fwrite( "xpg0", 1, 4, fp );
	   fwrite( "1000", 1, 4, fp ); // version = 1.0.0
	   fwrite( "magicpixel      ", 1, 16, fp );
	   fwrite( "xpgconverter    ", 1, 16, fp );
	 */
	DWORD dwValue = 0;
	xpgFileSeek(fp, 4, SEEK_SET);
	pstMovie->m_dwVersion = xpgReadNextDWord(fp);
	mpDebugPrint("pstMovie->m_dwVersion = %d(If BuilderV5 then m_dwVersion >= 5000)", pstMovie->m_dwVersion);
	if (pstMovie->m_dwVersion == 0x30303031) pstMovie->m_dwVersion = 1000; 
	if (pstMovie->m_dwVersion >= 6000)
    {
        MP_ALERT("pstMovie->m_dwVersion >= 6000, just return!"); 
        return;
    } 

	xpgFileSeek(fp, 40, SEEK_SET);
	pstMovie->m_wScreenWidth = xpgReadNextDWord(fp);
	pstMovie->m_wScreenHeight = xpgReadNextDWord(fp);

    xpgFileSeek(fp, 52, SEEK_SET);
	BOOL boMenu = xpgReadNextDWord(fp);
    //mpDebugPrint("boMenu = 0x%X", boMenu);
    if(boMenu == 1)
    {
        DWORD dwMenuDataPos = xpgReadNextDWord(fp);
        DWORD dwMenuDataSize = xpgReadNextDWord(fp);
        mpDebugPrint("dwMenuDataPos  = 0x%X", dwMenuDataPos);
        mpDebugPrint("dwMenuDataSize = 0x%X", dwMenuDataSize);
        xpgFileSeek(fp, dwMenuDataPos, SEEK_SET);
        BYTE *xpgMenuString = (BYTE *) ext_mem_mallocEx(dwMenuDataSize, __FILE__, __LINE__);
        xpgFileRead(xpgMenuString, 1, dwMenuDataSize, fp);
        mpDebugPrint("xpgMenuString = %s", xpgMenuString);
        mpDebugPrint(" ");
        // TO DO - call menuXmlParser xpgMenuXmlParser(); // output fotmat - TO DO
#if 0   // source in \editor\fmMenu.cpp, XML2MenuTree()       
        while (Node != NULL) 
        {
             tn = tv->Items->AddChild(tn, Node->Attributes["text"]);
             NodeID[gnIdPos] = tn->ItemId;
             MenuStruct[gnIdPos].text = Node->Attributes["text"];
             MenuStruct[gnIdPos].imageIdx = StrToInt(Node->Attributes["imageIdx"]);
             MenuStruct[gnIdPos].enable = StrToInt(Node->Attributes["enable"]);
             MenuStruct[gnIdPos].chkState = StrToInt(Node->Attributes["chkState"]);
             MenuStruct[gnIdPos].chkOnIdx = StrToInt(Node->Attributes["chkOnIdx"]);
             MenuStruct[gnIdPos].chkOffIdx = StrToInt(Node->Attributes["chkOffIdx"]);
             MenuStruct[gnIdPos].chkOnIdx = MenuStruct[0].chkOnIdx;
             MenuStruct[gnIdPos].chkOffIdx = MenuStruct[0].chkOffIdx;
             MenuStruct[gnIdPos].bgNormalIdx = StrToInt(Node->Attributes["bgNormalIdx"]);
             MenuStruct[gnIdPos].bgSelectedIdx = StrToInt(Node->Attributes["bgSelectedIdx"]);
             MenuStruct[gnIdPos].menuTopIdx = StrToInt(Node->Attributes["menuTopIdx"]);
             MenuStruct[gnIdPos].menuBottomIdx = StrToInt(Node->Attributes["menuBottomIdx"]);
             MenuStruct[gnIdPos].entryIdx = StrToInt(Node->Attributes["entryIdx"]);
             MenuStruct[gnIdPos].funIdx = StrToInt(Node->Attributes["funIdx"]);
        }
#endif             
    }
    
    xpgFileSeek(fp, 256, SEEK_SET);
	BOOL boMultiLanguage = xpgReadNextDWord(fp);
    //mpDebugPrint("boMultiLanguage = 0x%X", boMultiLanguage);
    if(boMultiLanguage == 1)
    {
        DWORD dwCountryNumber = xpgReadNextDWord(fp);
        DWORD dwStringNumber = xpgReadNextDWord(fp);
        DWORD dwMultiLanguagePos = xpgReadNextDWord(fp);
        DWORD dwMultiLanguageSize = xpgReadNextDWord(fp);
        mpDebugPrint("dwCountryNumber = 0x%X(%d)", dwCountryNumber, dwCountryNumber);
        mpDebugPrint("dwStringNumber  = 0x%X(%d)", dwStringNumber,  dwStringNumber);
        mpDebugPrint("dwMultiLanguagePos  = 0x%X", dwMultiLanguagePos);
        mpDebugPrint("dwMultiLanguageSize = 0x%X", dwMultiLanguageSize);
        xpgFileSeek(fp, dwMultiLanguagePos, SEEK_SET);
        BYTE *xpgMultiLanguage = (BYTE *) ext_mem_mallocEx(dwMultiLanguageSize, __FILE__, __LINE__);
        xpgFileRead(xpgMultiLanguage, 1, dwMultiLanguageSize, fp);
        BYTE *p = xpgMultiLanguage;
        int j;
        for(j=0; j<dwStringNumber; j++)
        {
            //mpDebugPrint("j=%d", j);
            WORD wLength = (*p)+*(p+1)*256;
            //mpDebugPrint("wLength = %d(0x%X)", wLength, *p);
            p += 2;
            //mpDebugPrint("0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X", *(p+0), *(p+1), *(p+2), *(p+3), *(p+4), *(p+5), *(p+6), *(p+7));
            p += wLength * 2; // 2 - unicode(2 bytes), next String
        }
    }
    
	xpgFileSeek(fp, 64, SEEK_SET);
	pstMovie->m_wMaxSprites = xpgReadNextDWord(fp);
	pstMovie->m_wSpriteDataLen = xpgReadNextDWord(fp);

	xpgFileSeek(fp, 76, SEEK_SET);

	pstMovie->m_wRoleCount = xpgReadNextDWord(fp);	//Image Number
	pstMovie->m_wImageHeaderLen = xpgReadNextDWord(fp);
	pstMovie->m_dwImageHeaderPos = xpgReadNextDWord(fp);

	pstMovie->m_wPageCount = xpgReadNextDWord(fp);
	pstMovie->m_wPageHeaderLen = xpgReadNextDWord(fp);
	pstMovie->m_dwPageHeaderPos = xpgReadNextDWord(fp);

	pstMovie->m_wScriptCount = xpgReadNextDWord(fp);
	pstMovie->m_wScriptHeaderLen = xpgReadNextDWord(fp);
	pstMovie->m_dwScriptHeaderPos = xpgReadNextDWord(fp);

	//pstMovie->m_astSprite = xpgCalloc(sizeof(STXPGSPRITE), pstMovie->m_wMaxSprites);
	
	xpgFileSeek(fp, 72, SEEK_SET);
	startPage = xpgReadNextDWord(fp);
	if( !(startPage > 0 && startPage <= pstMovie->m_wPageCount) )
	   startPage = 0;
	else
	   mpDebugPrint("Movie's startPage = %d", startPage);

	return true;
}

//----------------------------------------------------------------------------
/*  format	            1	0	jpg, png, gif¡K. Or other flag
    Pixel depth	        1	1
    Pixel type	        4	2	444, 555, 888¡K
    width	            2	6
    height	            2	8
    Image Data Length	4	10
    Image Data Position	4	14
    len = 18
    
    curRole->m_iFormat = fgetc(fp);
    curRole->m_cBpp = fgetc(fp);
    curRole->m_iType = fgetc(fp);
    curRole->m_iQuality = fgetc(fp);

    //int iPixelType = 0x888;
    int iType = 0;
    fread(&(iType), 4, 1, fp);
    curRole->m_boOSD = iType & 1;
    curRole->m_boTrans = iType & 0x10000000;
*/
///
///@ingroup xpgReader
///@brief   Read Role header and set pstRole
///
///@param   pstMovie - pointer of Movie
///
///@param   fp - file pointer   
///
///@param   pstRole - pointer of Role
/// 
///@return  true of false
/// 
XPGBOOL xpgReadRoleHeader(register STXPGMOVIE * pstMovie, XPGFILE * fp, STXPGROLE * pstRole)
{
    MP_DEBUG("xpgReadRoleHeader");
	DWORD dwPixelType;

	if (fp == NULL)
		return false;

	DWORD dwFormat;

	dwFormat = xpgReadNextDWord(fp);
	//dwPixelType = 0x888;
	pstRole->m_bImageType = (dwFormat >> 0) & 0xff;	//IF_JPEG
	pstRole->m_bBitDepth = (dwFormat >> 8) & 0xff;
	pstRole->m_bImageType = (dwFormat >> 16) & 0xff;
	MP_DEBUG("pstRole->m_bImageType = 0x%X", pstRole->m_bImageType);
	MP_DEBUG("pstRole->m_bBitDepth = 0x%X", pstRole->m_bBitDepth);

	//pstRole->m_bQuality = (dwFormat >> 24) & 0xff;
	//if (pstRole->m_bBitDepth != 24)
	//  MP_DEBUG2("role fmt %x %d", dwFormat, pstRole->m_bBitDepth);

	dwFormat = xpgReadNextDWord(fp);
	pstRole->m_bType = dwFormat & 0xff;

	pstRole->m_wWidth = xpgReadNextDWord(fp);
	pstRole->m_wHeight = xpgReadNextDWord(fp);
	pstRole->m_dwFilePos = xpgReadNextDWord(fp);
	pstRole->m_dwDataLen = xpgReadNextDWord(fp);
#if XPG_LOG_FILE
    memset(bDataBuf, 0, sizeof(bDataBuf));
    mp_sprintf(bDataBuf, "w = %03d, h = %03d", pstRole->m_wWidth, pstRole->m_wHeight);
    bDataBuf[16]=0x0d;
    bDataBuf[17]=0x0a;
    FileWrite(xpglogHandle, bDataBuf, StringLength08(bDataBuf));
#endif		
    MP_DEBUG("pstRole->m_wWidth = %d, pstRole->m_wHeight = %d", pstRole->m_wWidth, pstRole->m_wHeight);
    // v5.0.1.1 add role ox, oy
	if (pstMovie->m_dwVersion >= 5011) {
	    xpgReadNextDWord(fp);
		pstRole->m_wOx = xpgReadNextDWord(fp);
		pstRole->m_wOy = xpgReadNextDWord(fp);
	}
	if (pstMovie->m_dwVersion < 2150) {
		pstRole->m_bBitDepth = 24;
		pstRole->m_bImageType = 0;
	}
	return true;
}

//-----------------------------------------------------------------------
///
///@ingroup xpgReader
///@brief   Read Role image data
///
///@param   pstMovie - pointer of Movie
///
///@param   fp - file pointer   
///
///@param   pstRole - pointer of Role
/// 
///@return  true or false
/// 
XPGBOOL xpgReadRoleMask(STXPGMOVIE * pstMovie, XPGFILE * fp, STXPGROLE * pstRole)
{
    MP_DEBUG("xpgReadRoleMask");
	if (pstRole->m_bBitDepth == 32)
	{
        char tag[4];
		unsigned long mask_pos = ALIGN_4(pstRole->m_dwFilePos + (pstRole->m_dwDataLen));

        xpgFileSeek(fp, mask_pos, SEEK_SET);
        xpgFileRead(tag, 1, 4, fp);

		/* check if with mask tag "MASK" */
		if (tag[3]=='M' && tag[2]=='A' && tag[1]=='S' && tag[0]=='K')
		{
			//MP_DEBUG("read mask image tag ok");
			/* read data size */
			int dwSize = xpgReadNextDWord(fp);

			if (dwSize > 0)
			{
				//TRACE("mask");
				/* allocation and clear buffer */
				int len = ALIGN_4(dwSize);
				pstRole->m_pMaskImage = (BYTE *)xpgMalloc(len);
				memset(pstRole->m_pMaskImage, 0, len);

				/* read image data */
                if (0 != xpgFileRead(pstRole->m_pMaskImage, 1, dwSize, fp))
                    return false;

                int count = dwSize / 4;
#if 0//(SDL_BYTEORDER == 1234)
                if (dwSize % 4) count++;
                swap_buffer_endian(pstRole->m_pMaskImage, 4, count);
#endif
			}
			return true;
		}
	}
	return false;
}

XPGBOOL xpgReadRoleData(register STXPGMOVIE * pstMovie, XPGFILE * fp, STXPGROLE * pstRole, WORD iRole)
{
	DWORD dwSize = pstRole->m_dwDataLen;
/*	if (iRole >= 563 && iRole <= 569)
	{
	    mpDebugPrint("iRole = %d", iRole);
        //mpDebugPrint("m_dwDataLen(dwSize) = 0x%X", dwSize);
        mpDebugPrint("m_wWidth = %d", pstRole->m_wWidth);
        mpDebugPrint("m_wHeight= %d", pstRole->m_wHeight);
    } */
    
	if (dwSize == 0)
		return false;
	xpgFileSeek(fp, pstRole->m_dwFilePos, SEEK_SET);

#if 1
		//MP_DEBUG("xpgReadRoleData");
		pstRole->m_pImage = (BYTE *) xpgMalloc(dwSize);
		if (pstRole->m_pImage == NULL)
		{
            return false;
        }
		//if (dwSize != xpgFileRead(pstRole->m_pImage, 1, dwSize, fp))
		if (0 != xpgFileRead(pstRole->m_pImage, 1, dwSize, fp))
		{
			return false;
		}
#else
	pstRole->m_pImage = (BYTE *) (fp->pbBuffer + pstRole->m_dwFilePos);
#endif
        if (pstRole->m_bBitDepth == 32)
        {
            xpgReadRoleMask(pstMovie, fp, pstRole);
        }

	return true;

}

//-----------------------------------------------------------------------
///
///@ingroup xpgReader
///@brief   Read Sprite data
///
///@param   pstMovie - pointer of Movie
///
///@param   fp - file pointer   
///
///@param   pstSprite - pointer of Sprite
/// 
///@return  true or false
/// 
XPGBOOL xpgReadPageSpriteData(register STXPGMOVIE * pstMovie, XPGFILE * fp, STXPGPAGESPRITE * pstSprite, WORD iSprite)
{
    MP_DEBUG("\t%s", __FUNCTION__);
/*
Layer	    2	0
X	        2	2
Y	        2	4
Image Index	2	6
ink	        4	8
*/
	//WORD x, y;
	//WORD iRole;

	if (fp == NULL)
		return false;

	pstSprite->m_wLayer = xpgReadNextDWord(fp);
	pstSprite->m_wPx = xpgReadNextDWord(fp);
	if(pstSprite->m_wPx & 1)
	   pstSprite->m_wPx += 1; // prevent careless sprite odd x position of Exception 1003 hang error
	pstSprite->m_wPy = xpgReadNextDWord(fp);
	pstSprite->m_wRole = xpgReadNextDWord(fp);
    //MP_DEBUG("\tpstSprite->m_wPx = %d, pstSprite->m_wPy = %d", pstSprite->m_wPx, pstSprite->m_wPy);
    //MP_DEBUG("\tpstSprite->m_wRole = %d", pstSprite->m_wRole);
	//if (iRole >= pstMovie->m_dwRoleCount) iRole = 0;

	//xpgSpriteSetRole(pstSprite, pstMovie->ppstRole[iRole);
	//xpgSpriteMoveTo(pstSprite, x, y);    

	pstSprite->m_dwType = xpgReadNextDWord(fp);
	pstSprite->m_dwHashKey = xpgReadNextDWord(fp);
	//pstSprite->m_dwInkValue), 3, 1, fp);
DWORD m_dwTypeIndex = (pstSprite->m_dwType & 0x000000ff);
#if XPG_LOG_FILE
    memset(bDataBuf, 0, sizeof(bDataBuf));
    mp_sprintf(bDataBuf, "m_wRole = %04d", pstSprite->m_wRole);
    bDataBuf[14]=0x0d;
    bDataBuf[15]=0x0a;
    FileWrite(xpglogHandle, bDataBuf, StringLength08(bDataBuf));
    memset(bDataBuf, 0, sizeof(bDataBuf));
    mp_sprintf(bDataBuf, "m_dwType = %03d", pstSprite->m_dwType>>16);
    bDataBuf[14]=0x0d;
    bDataBuf[15]=0x0a;
    FileWrite(xpglogHandle, bDataBuf, StringLength08(bDataBuf));
    memset(bDataBuf, 0, sizeof(bDataBuf));
    mp_sprintf(bDataBuf, "m_dwTypeIndex = %04d", m_dwTypeIndex);
    bDataBuf[20]=0x0d;
    bDataBuf[21]=0x0a;
    FileWrite(xpglogHandle, bDataBuf, StringLength08(bDataBuf));
    memset(bDataBuf, 0, sizeof(bDataBuf));
    mp_sprintf(bDataBuf, "m_wPx = %03d, m_wPy = %03d", pstSprite->m_wPx, pstSprite->m_wPy);
    bDataBuf[24]=0x0d;
    bDataBuf[25]=0x0a;
    FileWrite(xpglogHandle, bDataBuf, StringLength08(bDataBuf));
#endif			
#if 0	
	WORD wCurPage = pstMovie->m_wCurPage;
	if(wCurPage == 37) // 0
	{
	    //mpDebugPrint("wCurPage = %d", wCurPage);
	    mpDebugPrint("iSprite = %d", iSprite);
	    mpDebugPrint("\tpstSprite->m_wRole = %d", pstSprite->m_wRole);
	    mpDebugPrint("\tpstSprite->m_dwType      = %d", pstSprite->m_dwType>>16);
        mpDebugPrint("\tpstSprite->m_dwTypeIndex = %d", pstSprite->m_dwType & 0x000000ff);
        mpDebugPrint("\tpstSprite->m_wPx = %d, pstSprite->m_wPy = %d", pstSprite->m_wPx, pstSprite->m_wPy);
        //mpDebugPrint("\tpstSprite->m_dwHashKey = 0x%X", pstSprite->m_dwHashKey);
        // until now, you can not get sprite width and height
        STXPGROLE * pstRole = &(pstMovie->m_astRole[pstSprite->m_wRole]);
        mpDebugPrint("\twidth = %d, height = %d", pstRole->m_wWidth, pstRole->m_wHeight);
    }
#endif
    // TODO - check when will pstSprite->m_dwType separate to pstSprite->m_dwType and pstSprite->m_dwTypeIndex
    // in xpgSprite.c, xpgSpriteCopy() (from PAGESPRITE to SPRITE) will do
    //      DWORD x, y;
	//     x = pstSrc->m_wPx;
	//     y = pstSrc->m_wPy;
	//     xpgSpriteMoveTo(pstDst, x, y);
	// and
    //     pstDst->m_wLayer = pstSrc->m_wLayer;
	//     pstDst->m_dwInk = pstSrc->m_dwInk;
	//     pstDst->m_dwHashKey = pstSrc->m_dwHashKey;
	//     pstDst->m_dwType = (pstSrc->m_dwType >> 16) & 0xffff;
	//     pstDst->m_dwTypeIndex = pstSrc->m_dwType & 0xffff;
	return true;
}

//-----------------------------------------------------------------------
XPGBOOL xpgReadSpriteFrame(STXPGMOVIE * pstMovie, XPGFILE *fp, STXPGPAGESPRITE *pSprite)
{
    MP_DEBUG("\txpgReadSpriteFrame");
    int i, iFrame, iAct, iActionCount;
    xpg_sprite_frame_t *pFrame;

	/* read pstSprite total frame count */
	pSprite->m_iFrameCount = xpgReadNextDWord(fp);
	pSprite->m_iKeyFrameCount = xpgReadNextDWord(fp);
    MP_DEBUG("\tpSprite->m_iFrameCount = %d", pSprite->m_iFrameCount);
    MP_DEBUG("\tpSprite->m_iKeyFrameCount = %d", pSprite->m_iKeyFrameCount);
	/* setting and allocate frames */
    xpgSpriteSetFrameCount(pSprite, pSprite->m_iFrameCount);
    xpgSpriteSetKeyFrameCount(pSprite, pSprite->m_iKeyFrameCount);

	if (pSprite->m_iKeyFrameCount > 0)
	{
		/* read pstSprite frame */
		for (i = 0; i < pSprite->m_iKeyFrameCount; i++)
		{
		    MP_DEBUG("\ti=%d(i<pSprite->m_iKeyFrameCount)", i);
		    iFrame = xpgReadNextDWord(fp);
		    MP_DEBUG("\tiFrame=%d", iFrame);
			pFrame = &(pSprite->m_astFrame[i]);
            pFrame->m_iFrame = iFrame;

            iActionCount = xpgReadNextDWord(fp);
            MP_DEBUG("\tiActionCount=%d", iActionCount);
			pFrame->m_iAction = xpgReadNextDWord(fp);
            MP_DEBUG("\tpFrame->m_iAction=0x%X(0x80-Angle, 0x40-Blend)", pFrame->m_iAction);
            if(iActionCount <= 0)
                break;
			/* set action data array */
			pFrame->m_alValue = (long *)xpgMalloc(sizeof(long) * iActionCount);
			//assert (pFrame->m_alValue != NULL);

			/* read action value array */
			for (iAct = 0; iAct < iActionCount; iAct++)
			{
			    MP_DEBUG("\tiAct=%d(iAct < iActionCount)", iAct);
				pFrame->m_alValue[iAct] = xpgReadNextDWord(fp);
				MP_DEBUG("\tpFrame->m_alValue[iAct=%d]=0x%X", iAct, pFrame->m_alValue[iAct]);
			}

			/* read frame tag */
			if (pFrame->m_iAction & ACT_FRAME_TAG)
			{
				short len;

				/* read tag length */
				len = xpgReadNextDWord(fp);

				char buf[18];
				memset(buf, 0, 18);
				xpgFileRead(buf, 1, len, fp);
                /* TODO : save buf to frame tag */
				/* set tag string */
				//pFrame->m_FrameTag = AnsiString(buf);
				//mpDebugPrint("FrameTag buf = %s", buf);
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------
/* 2 bytes length, text */
XPGBOOL xpgReadSpriteText(STXPGMOVIE * pstMovie, XPGFILE * fp, STXPGPAGESPRITE * pstSprite)
{
    MP_DEBUG("xpgReadSpriteText()");
    pstSprite->m_wTextLen = xpgReadNextDWord(fp);
    MP_DEBUG("pstSprite->m_wTextLen = %d", pstSprite->m_wTextLen);
    if (pstSprite->m_wTextLen == 0)
        return true;

    pstSprite->m_Text = (char *)xpgMalloc(pstSprite->m_wTextLen);
    int size = xpgReadString(fp, pstSprite->m_Text, pstSprite->m_wTextLen);
    MP_DEBUG("pstSprite->m_Text = %s", pstSprite->m_Text);
    MP_DEBUG("size = %d", size);
    
    if(size < pstSprite->m_wTextLen && size > (pstSprite->m_wTextLen - 4))
    {
        pstSprite->m_wTextLen = size;
        MP_DEBUG("After adjust Text size to %d", pstSprite->m_wTextLen);
    }

    return true;
}

//-----------------------------------------------------------------------
XPGBOOL xpgReadSpriteExtraData(STXPGMOVIE * pstMovie, XPGFILE * fp, STXPGPAGESPRITE * pstSprite)
{
    MP_DEBUG("\txpgReadSpriteExtraData");
	//assert(fp != NULL);
    int i;
	unsigned long extra_chunk_start = 0;
	unsigned long extra_chunk_end = 0;
	unsigned long extra_chunk_size = 0;
	unsigned long pos = 0;
	int chunk_count = 0;
    char tag[5];

	extra_chunk_size = xpgReadNextDWord(fp);
	chunk_count = xpgReadNextDWord(fp);
    MP_DEBUG1("\textra_chunk_size = %d", extra_chunk_size);
    MP_DEBUG1("\tchunk_count = %d", chunk_count);
	extra_chunk_start = fp->dwPos;
	extra_chunk_end = extra_chunk_start + extra_chunk_size;

	if (extra_chunk_size == 0 || chunk_count == 0)
	{
		xpgFileSeek( fp, extra_chunk_end, SEEK_SET );
		return true;
	}
    /* chunk count -> 4 bytes */
	for (i = 0; i < chunk_count; i++)
	{
	    MP_DEBUG1("\ti=%d(i<chunk_count)", i);
		/* check tag */
		xpgFileRead(&(tag), 1, 4, fp);
		tag[4]=0;
        MP_DEBUG("\ttag=%s", tag);
		if (tag[0] == 'T' && tag[1] == 'X' && tag[2] == 'E' && tag[3] == 'T')
		{
			xpgReadSpriteText(pstMovie, fp, pstSprite);
		}
		else if (tag[0] == 'F' && tag[1] == 'I' && tag[2] == 'N' && tag[3] == 'A')
		{
			xpgReadSpriteFrame(pstMovie, fp, pstSprite);
		}
	}

	/* seek forward to chunk end for next write */
	xpgFileSeek( fp, extra_chunk_end, SEEK_SET );

	return true;
}
//---------------------------------------------------------------------------
///
///@ingroup xpgReader
///@brief   Read Page Header
///
///@param   pstMovie - pointer of Movie
///
///@param   fp - file pointer   
///
///@param   pstPage - pointer of Page
/// 
///@return  true or false
/// 
XPGBOOL xpgReadPageHeader(register STXPGMOVIE * pstMovie, XPGFILE * fp, STXPGPAGE * pstPage)
{
/*
Page Index	                2	0
Page Script Index	        2	2
Sprite Number	            2	4
Sprite Data Start Position	4	6
*/
	if (fp == NULL)
		return false;

	pstPage->m_wIndex = xpgReadNextDWord(fp) & 0xff;
	pstPage->m_dwScriptFilePos = xpgReadNextDWord(fp);
	//pstPage->m_wSpriteCount = xpgReadNextDWord(fp) & 0xff;
	pstPage->m_wSpriteCount = xpgReadNextDWord(fp);
	pstPage->m_dwSpriteFilePos = xpgReadNextDWord(fp);
	pstPage->m_dwHashKey = xpgReadNextDWord(fp);

	return true;
}

//----------------------------------------------------------------------------
int xpgReadPageCommand(STXPGMOVIE * pstMovie, XPGFILE * fp, STXPGPAGE * pstPage)
{
    MP_DEBUG("xpgReadPageCommand");
	// Read Script Data
	WORD wCurPage = pstMovie->m_wCurPage;
	int i, iCmd;
	WORD wTemp;  
	if (pstPage->m_dwScriptFilePos > 0
		&& pstPage->m_dwScriptFilePos < pstMovie->m_dwFileSize)
	{
		xpgFileSeek(fp, pstPage->m_dwScriptFilePos, SEEK_SET);
		//DWORD curFilePos = xpgFileTell(fp);
		//mpDebugPrint("curFilePos = 0x%X", curFilePos);
		pstPage->m_dwCmdCount = xpgReadNextDWord(fp) & 0xff;

		//if (pstPage->m_dwCmdCount > XPG_COMMAND_COUNT)
		//	pstPage->m_dwCmdCount = XPG_COMMAND_COUNT;

        MP_DEBUG("pstPage->m_dwCmdCount %d", pstPage->m_dwCmdCount);
		if (pstPage->m_dwCmdCount > 0)
		{
			pstPage->m_wCommand = (WORD *)xpgMalloc(pstPage->m_dwCmdCount * 6 * 2); // 20070517 - command count defined by xpg file

			// add key event and action param - 07.24.2006 Athena
			for (iCmd = 0, i = 0; iCmd < pstPage->m_dwCmdCount; iCmd++, i += 6)
			{
				pstPage->m_wCommand[i+3] = xpgReadNextWord(fp);	// event index
				pstPage->m_wCommand[i+0] = xpgReadNextWord(fp);	// button index
				wTemp = xpgReadNextWord(fp);
				pstPage->m_wCommand[i+1] = xpgReadNextWord(fp);	// page hash key
				pstPage->m_wCommand[i+4] = xpgReadNextWord(fp);	// act parameter
				pstPage->m_wCommand[i+2] = xpgReadNextWord(fp);	// action hash key
				 /*   if(wCurPage == 1) // 0
    	           {
    	             mpDebugPrint("\tiCmd = %d", iCmd);
                     mpDebugPrint("\tpstPage->m_wCommand[i+3] = 0x%X(event)", pstPage->m_wCommand[i+3]);  
                     mpDebugPrint("\tpstPage->m_wCommand[i+0] = 0x%X(button)", pstPage->m_wCommand[i+0]);
                     mpDebugPrint("\tpstPage->m_wCommand[i+1] = 0x%X(page hash key)", pstPage->m_wCommand[i+1]);
                     mpDebugPrint("\tpstPage->m_wCommand[i+2] = 0x%X(action hash key)", pstPage->m_wCommand[i+2]);
                     DWORD dwAction = pstPage->m_wCommand[i+2];
                     mpDebugPrint("\tdwAction = %d", dwAction);
                     extern STACTFUNC *xpgActionFunctions; // in xpgUi.c 
                     mpDebugPrint("\txpgActionFunctions[dwAction].hCommand = 0x%X", xpgActionFunctions[dwAction].hCommand); 
    	           } */  
			}
		}
	}
}    


//----------------------------------------------------------------------------
int xpgReadPage(STXPGMOVIE * pstMovie, XPGFILE * fp, int iPage)
{
    int i, iSprite, iSpriteCount;
    DWORD lPos;
    STXPGPAGE *pstPage = NULL;
    STXPGPAGESPRITE *pstSprite = NULL;
    short with_extra_data = 0;

    // Read Page Header
    pstMovie->m_wCurPage = iPage;
    MP_DEBUG1("xpgReadPage %d", iPage);
    lPos = pstMovie->m_dwPageHeaderPos;
    lPos += (iPage * pstMovie->m_wPageHeaderLen);
    xpgFileSeek(fp, lPos, SEEK_SET);

    pstPage = &(pstMovie->m_astPage[iPage]);
    memset(pstPage, 0, sizeof(STXPGPAGE));
    xpgReadPageHeader(pstMovie, fp, pstPage);

    //iSpriteCount = 4;
    if (pstPage->m_dwSpriteFilePos > 0
        && pstPage->m_dwSpriteFilePos < pstMovie->m_dwFileSize)
    {
        xpgFileSeek(fp, pstPage->m_dwSpriteFilePos, SEEK_SET);

        if (pstPage->m_wSpriteCount > XPG_SPRITE_COUNT)
        {
			mpDebugPrint("!!!--- %d overflow: really:%d define:%d", iPage,pstPage->m_wSpriteCount , XPG_SPRITE_COUNT);
            pstPage->m_wSpriteCount = XPG_SPRITE_COUNT;
        }

        iSpriteCount = pstPage->m_wSpriteCount;
#if XPG_LOG_FILE
    memset(bDataBuf, 0, sizeof(bDataBuf));
    mp_sprintf(bDataBuf, "iSpriteCount = %02d", pstPage->m_wSpriteCount);
    bDataBuf[17]=0x0d;
    bDataBuf[18]=0x0a;
    FileWrite(xpglogHandle, bDataBuf, StringLength08(bDataBuf));
#endif	            
        MP_DEBUG("iSpriteCount = %d", iSpriteCount);
        if(iSpriteCount > 0)
        {
            pstPage->m_astSprite =
                (STXPGPAGESPRITE *) xpgCalloc(sizeof(STXPGPAGESPRITE), iSpriteCount);
    
            // Read Sprite Data
            for (iSprite = 0; iSprite < iSpriteCount; iSprite++)
            {
                pstSprite = &(pstPage->m_astSprite[iSprite]);
                memset(pstSprite, 0, sizeof(STXPGPAGESPRITE));
#if XPG_LOG_FILE
    memset(bDataBuf, 0, sizeof(bDataBuf));
    mp_sprintf(bDataBuf, "iSprite = %02d", iSprite);
    bDataBuf[12]=0x0d;
    bDataBuf[13]=0x0a;
    FileWrite(xpglogHandle, bDataBuf, StringLength08(bDataBuf));
#endif                 
                MP_DEBUG("\tiSprite=%d(iSprite < iSpriteCount)", iSprite);
                xpgReadPageSpriteData(pstMovie, fp, pstSprite, iSprite);
            }
        }

        // Read Sprite Extra Data
        if (check_tag(fp, "RTXE", 4))
        {
            with_extra_data = xpgReadNextDWord(fp);
            MP_DEBUG("with_extra_data = %d", with_extra_data);
            
            // read extra data here
            for (i = 0; i < with_extra_data; i++)
            {
                iSprite = xpgReadNextDWord(fp);
                MP_DEBUG("iSprite = 0x%08X(%d)", iSprite, iSprite);
                if(iSprite < iSpriteCount)
                {
                    pstSprite = &(pstPage->m_astSprite[iSprite]);
                    xpgReadSpriteExtraData(pstMovie, fp, pstSprite);
                }
            }
        }

        xpgReadPageCommand(pstMovie, fp, pstPage);
    }
}

//----------------------------------------------------------------------------
/*
XPG File Header
	name	length	Position	Description
	xpg	                4	0
	version	            4	4	1010 = v1.01
	Company name	    16	8	magicpixel
	Editor name	        16	24	"Magicbuilder, xpgConverter"
	Screen Width	    2	40
	Screen Height	    2	42
	Background Color 	6	44	RGB
	Transparent Color	6	50
	Maximun SpriteLayer	2	56
	Sprite Data Length	2	58
	Reserve	            16	60
	Image Number	    2	76
	Image Header Len	2	78
	Image Header Pos	4	80
	Page Number	        2	84
	Page Header Len	    2	86
	Page Header Pos	    4	88
	Script Number	    2	92
    Script Header Len	2	94
    Script Header Pos	4	96
*/
//----------------------------------------------------------------------------
///
///@ingroup xpgReader
///@brief   Read Movie
///
///@param   pstMovie - pointer of Movie
///
///@return  true or false
/// 
XPGBOOL xpgReadMovie(register STXPGMOVIE * pstMovie)
{
    MP_DEBUG("xpgReadMovie");
	XPGFILE *fp = pstMovie->m_pFileHandle;
	STXPGROLE *pstRole = NULL;
	STXPGPAGE *pstPage = NULL;
	STXPGPAGESPRITE *pstSprite = NULL;

	DWORD lPos = 0;
	int iRole = 0, iPage = 0, iSprite = 0;
	int iSpriteCount = 0;
	int iRoleCount = 0, iPageCount = 0;
	int i, iCmd;

	XPGBOOL boNoErr = true;
#if XPG_LOG_FILE
    // Open xpg.log
    // CreateFile() supports both DOS 8.3 format and long filename format 
    mpDebugPrint("%s: Create xpg.log....", __func__);
    DRIVE *drv = DriveGet(19);
    if (CreateFile(drv, "xpg", "log") != FS_SUCCEED)
    {
        MP_ALERT("%s: Create xpg.log failed, file not created !", __func__);
        return 0;
    }
    MP_ALERT("%s: Create xpg.log OK, file created !", __func__);
        
    xpglogHandle = FileOpen(drv);
    if (xpglogHandle == NULL)
    {
        MP_ALERT("%s: FileOpen() xpg.log failed !", __func__);
        return 0;
    }
    MP_ALERT("%s: FileOpen() xpg.log OK !", __func__);
#endif
	boNoErr = xpgReadMovieHeader(pstMovie, fp);

	if (boNoErr)
	{
		//if (pstMovie->m_wRoleCount > XPG_ROLE_COUNT) // remove at 20070517 - role count defined by xpg file
		//	pstMovie->m_wRoleCount = XPG_ROLE_COUNT;

		iRoleCount = pstMovie->m_wRoleCount;
#if XPG_LOG_FILE
    memset(bDataBuf, 0, sizeof(bDataBuf));
    mp_sprintf(bDataBuf, "iRoleCount = %04d", iRoleCount);
    bDataBuf[17]=0x0d;
    bDataBuf[18]=0x0a;
    FileWrite(xpglogHandle, bDataBuf, StringLength08(bDataBuf));
#endif			
        MP_DEBUG("iRoleCount %d", iRoleCount);
		pstMovie->m_astRole = (STXPGROLE *)xpgCalloc(sizeof(STXPGROLE), iRoleCount); // 20070517 - role count defined by xpg file

		// Read Role Data
		for (iRole = 0; iRole < iRoleCount; iRole++)
		{
#if XPG_LOG_FILE
    memset(bDataBuf, 0, sizeof(bDataBuf));
    mp_sprintf(bDataBuf, "iRole = %04d", iRole);
    bDataBuf[12]=0x0d;
    bDataBuf[13]=0x0a;
    FileWrite(xpglogHandle, bDataBuf, StringLength08(bDataBuf));
#endif		
		    MP_DEBUG("iRole = %d", iRole);
			// Read Role Header
			lPos = pstMovie->m_dwImageHeaderPos + iRole * pstMovie->m_wImageHeaderLen;
			xpgFileSeek(fp, lPos, SEEK_SET);

			pstRole = &(pstMovie->m_astRole[iRole]);
			//xpgRoleInit(pstRole);
			xpgReadRoleHeader(pstMovie, fp, pstRole);
			pstRole->m_wIndex = iRole;

			//xpgFileSeek( fp, pstRole->m_dwFilePos, SEEK_SET );
			xpgReadRoleData(pstMovie, fp, pstRole, iRole);
		}

		//if (pstMovie->m_wPageCount > XPG_PAGE_COUNT) // remove at 20070517 - page count defined by xpg file
		//	pstMovie->m_wPageCount = XPG_PAGE_COUNT;

		//Read Pages & Sprites
		iPageCount = pstMovie->m_wPageCount;
#if XPG_LOG_FILE
    memset(bDataBuf, 0, sizeof(bDataBuf));
    mp_sprintf(bDataBuf, "iPageCount = %03d", iPageCount);
    bDataBuf[16]=0x0d;
    bDataBuf[17]=0x0a;
    FileWrite(xpglogHandle, bDataBuf, StringLength08(bDataBuf));
#endif				
        MP_DEBUG("iPageCount %d", iPageCount);
		pstMovie->m_astPage = (STXPGPAGE *)xpgCalloc(sizeof(STXPGPAGE), iPageCount); // 20070517 - page count defined by xpg file
		
		for (iPage = 0; iPage < iPageCount; iPage++)
		{
		    //mpDebugPrint("iPage = %d", iPage);
#if XPG_LOG_FILE
    memset(bDataBuf, 0, sizeof(bDataBuf));
    mp_sprintf(bDataBuf, "iPage = %03d", iPage);
    bDataBuf[11]=0x0d;
    bDataBuf[12]=0x0a;
    FileWrite(xpglogHandle, bDataBuf, StringLength08(bDataBuf));
#endif			    
			xpgReadPage(pstMovie, fp, iPage);
		}

	}

#if XPG_LOG_FILE
    FileClose(xpglogHandle);
#endif	
	return boNoErr;
}

//----------------------------------------------------------------------------
#ifdef XPG_LOAD_FROM_FILE
///
///@ingroup xpgReader
///@brief   Load Movie file
///
///@param   pstMovie - pointer of Movie
///
///@param   filename - specified file name
/// 
///@return  true or false
/// 
XPGBOOL xpgLoadMovie(register STXPGMOVIE * pstMovie, const char *filename)
{
    MP_DEBUG("xpgLoadMovie");
	XPGFILE *fp = NULL;
	XPGBOOL boNoErr = true;

	if ((fp = xpgFileOpen(filename, "r")) == NULL)
		return false;

	pstMovie->m_pFileHandle = fp;
	pstMovie->m_dwFileSize = xpgFileGetSize(fp);
	boNoErr = xpgReadMovie(pstMovie);

	xpgFileClose(fp);
	return boNoErr;
}
#else
//----------------------------------------------------------------------------
///
///@ingroup xpgReader
///@brief   Load Movie from file handle
///
///@param   pstMovie - pointer of Movie
///
///@param   pdwBuffer - pointer of Buffer   
///
///@param   dwSize - specified buffer size
/// 
///@return  true or false
/// 
XPGBOOL xpgLoadMovie(register STXPGMOVIE * pstMovie, DWORD * pdwBuffer, DWORD dwSize)
{
    MP_DEBUG("xpgLoadMovie dwSize = 0x%X(%d)", dwSize, dwSize);
	XPGFILE *fp = NULL;
	XPGBOOL boNoErr = true;

	if ((fp = xpgFileOpen(pdwBuffer, dwSize)) == NULL)
		return false;

	pstMovie->m_pFileHandle = fp;
	pstMovie->m_dwFileSize = fp->dwSize;

	boNoErr = xpgReadMovie(pstMovie);

	xpgFileClose(fp);
	return boNoErr;
}
#endif
//-----------------------------------------------------------------------
