
#include "global612.h"
#include "mpTrace.h"
#include "xpg.h"
#include "xpgString.h"

static const BYTE* xpgString_English[] = 
{
	"",
        
	"VIDEO RECORDER",
	"CAPTURE",
	"M-DETECT",
	"VIDEO",
	"PHOTO",
	"MUSIC",
	"SETUP",

    "Beginning",
    "Record Time",
    "Photo Number",
    "M-Detect",
    "M-Detect Sensitivity",
	"Time and date",
	"Language",
	"Format",
	"System Info",
	"Default Setting",
    "General Setting",

    "Camcorder",
    "Photo",
    "30 Seconds",
    "60 Seconds",
    "90 Seconds",
    "120 Seconds",
    "1 pc",
    "3 pc",
    "5 pc",
    "Low Sensitivity",
    "Normal Sensitivity",
    "High Sensitivity",
    "English",
    "Chinese",
    "NO",
    "YES",
    "VER",
    "NAND Flash",
    "Used",
    "Unused",
    "SD Card",

    "Name",
    "Singer",
    "Album",
    "Bit Rate",
    "Length",
    "Progress",
    "Vol",

    "Camcorder Preview",
    "Camera Preview",
	
};

#include "xpgString_SChinese.txt"

int langid = 0;
const BYTE** xpgString = xpgString_SChinese;   //xpgString_English;

const BYTE* getstr(DWORD str_id)
{
	if (str_id >= Str_MAX_ID)
		return "";
	return xpgString[str_id];
}
#if 0
BYTE ENG_FontList[] = {
	0,
	FONT_ID_TAHOMA19,
	FONT_ID_TAHOMA19,
};

BYTE SChinese_FontList[] = {
	0,
	FONT_ID_TAHOMA19,
	FONT_ID_YAHEI19,
};
static BYTE * fontlist = ENG_FontList;
#endif
void ChangeLanguage(int lang)
{
	langid = lang;
	if (lang == LANGUAGE_ENGLISH) {
		xpgString = xpgString_English;
		//fontlist = ENG_FontList;
	}
	else if (lang == LANGUAGE_S_CHINESE) {
		xpgString = xpgString_SChinese;
		//fontlist = SChinese_FontList;
	}
	else {
		xpgString = xpgString_English;
		//fontlist = ENG_FontList;
	}
}

void IduSetFontSize(DWORD fontsize)
{
	if (fontsize == 0)
		fontsize = 1;
	//SetCurrIduFontID(fontlist[fontsize]);
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////






