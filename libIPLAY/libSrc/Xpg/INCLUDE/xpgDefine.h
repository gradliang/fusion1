#ifndef __XPG_DEFINE_H__
#define __XPG_DEFINE_H__

#include "global612.h"
#define XPG_612

#if 0 //0227 rebecca to reduce the main memory
//#define XPG_SPRITE_COUNT	46 // FlashClock = background + Hour(0..14) + Minute(0..14) + Second(0..14) = 46
#define XPG_SPRITE_COUNT	151 // for WiFi page WifiKeypadSetup & MP650 PDPF ClockType5
#define XPG_COMMAND_COUNT 	32 // 20070517 - enlarge(more flexible), because command count only use at xpg movie structure
#define XPG_CARD_COUNT		12
#define XPG_SCROLLBAR_COUNT	4
#define XPG_LIST_COUNT		8  // MP650 PDPF ClockType5
#define XPG_THUMB_COUNT		THUMB_COUNT // in Platform_MP652.h and Platform_MP656.h, changed from 6 to 30 
#define XPG_MENULIST_COUNT  8
#define XPG_ICONANI_COUNT	8
#else
#define XPG_SPRITE_COUNT	200 //
#define XPG_GLOBAL_CMD_COUNT 	4 // page 0
#define XPG_COMMAND_COUNT 	8 // every page
//#define XPG_CARD_COUNT		12  // no use
//#define XPG_SCROLLBAR_COUNT	0
//#define XPG_LIST_COUNT		8  // MP650 PDPF ClockType5
//#define XPG_THUMB_COUNT		THUMB_COUNT
#define XPG_MENULIST_COUNT  8
#define XPG_ICONANI_COUNT	8
#endif

#endif

