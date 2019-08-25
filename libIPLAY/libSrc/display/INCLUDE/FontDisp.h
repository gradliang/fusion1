#ifndef __FONTDISP__H__
#define __FONTDISP__H__

#include "utiltypedef.h"
#include "iPlaySysconfig.h"

#if BMP_FONT_ENABLE

typedef struct
{
	DWORD 	Data_Tag;		// Font Data Tag
	DWORD 	Uni_Tag;		// Font Unicode Table Tag
	DWORD 	Ansi_Tag;		// Font Ansi-code Table Tag
	BYTE	Lang_Index;		// This font language index
	SBYTE	bYOffset;		// The offset of Y, for fine turn the display base line
	BYTE	bRev[2];
}FontLangIndexTag;


void FontInit();

#endif

void Font_Draw(ST_IMGWIN *psWin, ST_FONT *sFont, WORD *str, BYTE UnicodeFlag);
void Font_GetStrWidth(ST_FONT *sFont, WORD *str, BYTE UnicodeFlag);
BYTE ImgPutCharFake(ST_FONT *sFont, WORD wData, BYTE MappingTable);


#endif

