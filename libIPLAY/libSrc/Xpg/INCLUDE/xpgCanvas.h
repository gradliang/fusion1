
#ifndef __XPG_CANVAS_H_
#define __XPG_CANVAS_H_


#include "xpgDIB.h"

#define PIXEL256    1
#define PIXEL4444   2
#define PIXEL444    3
#define PIXEL888    4
#define PIXEL8888   5

#define KEY_COLOR 0x100f
#define COLOR_BLACK 0x000
#define COLOR_DARK  0x111
#define COLOR_WHITE 0xFFF
#define COLOR_BRIGHT 0xEEE
#define COLOR_YELLOW 0xFF0
#define COLOR_RED	 0xf00
#define COLOR_BLUE	 0x00f
#define COLOR_GREEN  0x0F0

#define IMG_START_ADDR 6

#define COPY_PUT	0
#define AND_PUT		1
#define OR_PUT		2
#define XOR_PUT		3
#define NOT_PUT		4
#define TRANS_PUT	5
#define COLOR_PUT	6
#define TRANS_NOT_PUT	7
#define SOLID_PUT		8
#define BLEND_PUT		9
#define ADD_PUT			10
#define DARKEN_PUT		11
#define LIGHTEN_PUT		12
#define ADD_VALUE_PUT	13

#define FLIPH_PUT (1 << 8)
#define FLIPV_PUT (1 << 7)

#define CLIPMAP_W 40
#define CLIPMAP_H 30

#define RGB444_R(c) (((c) >> 8) & 0xf)
#define RGB444_G(c) (((c) >> 4) & 0xf)
#define RGB444_B(c) ((c) & 0xf)
#define ARGB4444_A(c) (((c) >> 12) & 0xf)
/*
#define RGB888_R(c) (((c) >> 0) & 0xff)
#define RGB888_G(c) (((c) >> 8) & 0xff)
#define RGB888_B(c) (((c) >> 16) & 0xff)
#define ARGB8888_A(c) (((c) >> 24) & 0xff)
*/
//extern XPGPIXEL gPal[];
extern DWORD screen_width;
extern DWORD screen_height;

/*
class xpgCanvass {
	WORD DWORD iWidth;
	WORD DWORD iHeight;
	BYTE cFormat;
	XPGPIXEL *canvas;	
	BOOL boNeedRefresh;
	BOOL boCheckClip;

	//xpgCanvass();
	//~xpgCanvass();
};
*/

XPGPIXEL*	xpgGetPixelAddress				( XPGPIXEL img[], DWORD x, DWORD y );
XPGPIXEL*	xpgAllocImageBuffer	( DWORD width, DWORD height, DWORD type );

XPGPIXEL*	get_dgw_canvas_for_refresh  ();	//-/
XPGPIXEL*	get_dgw_canvas		    ();	//-/



void 	xpgOpenCanvas			();
void 	xpgCloseCanvas   		();

void 	xpgSetVisibleRange		( DWORD lt, DWORD tp, DWORD rt, DWORD bm );

void 	xpgSetCanvasPalette		(BYTE *pPalette);
void    xpgClearCanvas           ();


// canvas clip
void 	xpgSetCheckClip			(BOOL check);
void 	xpgSetCanvasClip			(DWORD left, DWORD right, DWORD top, DWORD bottom);
void 	xpgAddCanvasClip			(DWORD left, DWORD right, DWORD top, DWORD bottom);
void 	xpgGetCanvasClip			(DWORD *left, DWORD *right, DWORD *top, DWORD *bottom);
void 	xpgClearCanvasClip		();


#endif /* __XPG_CANVAS_H_ */
