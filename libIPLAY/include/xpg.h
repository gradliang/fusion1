//---------------------------------------------------------------------------

#ifndef xpg_MAIN_H__
#define xpg_MAIN_H__

#include "..\..\libIPLAY\libsrc\xpg\include\xpg.h"

typedef struct {
    WORD boChanged;
    WORD wCount;        //total device count
    WORD wIndex;        //current index without mod wListCount
    WORD wListCount;    //this if for how many icon/or bars in single page ex:6 for photo , 5 for BT,video , audio
    WORD wListIndex;    //current list index after mod wListCount
    WORD wListFirstIndex;//usually 0,5,10...etc
    ST_SEARCH_INFO *pCurSearchInfo;
    DWORD   dwCurFileSize;
    WORD    wCurImageWidth;
    WORD    wCurImageHeight;
    DWORD dwOpMode;
    BYTE bCurFileType;
    BYTE bCurDriveId;
    BYTE bReserved[2];

} STXPGBROWSER;

#endif

