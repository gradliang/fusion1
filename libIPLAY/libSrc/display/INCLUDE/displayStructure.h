#ifndef DISPLAY_STRUCT_H
#define DISPLAY_STRUCT_H

#include "iplaysysconfig.h"

//***********************************************************************
//    display structure
//***********************************************************************
///
///@ingroup     ImageDecoder
///@brief       Image file data sructure
typedef struct{
    DWORD   *pdwStart;          /* frame buffer start address of image */
    DWORD   dwOffset;           /* frame buffer width in byte unit */
    WORD    wHeight;            /* working area height in pixel */
    WORD    wWidth;             /* working area width in pixel */
    WORD    wX;                    /* Start pixel to display */
    WORD    wY;                    /* Start pixel to display */
    WORD    wClipLeft;
    WORD    wClipRight;
    WORD    wClipTop;
    WORD    wClipBottom;
    WORD    wAxisX;
    WORD    wAxisY;
    WORD    wType;
} ST_IMGWIN;


typedef struct{
    DWORD   *pdwStart;          /* frame buffer start address of image */
    DWORD   dwBufferSize;
    WORD    wHeight;            /* frame buffer height in pixel */
    WORD    wWidth;             /* frame buffer width in pixel */
    WORD    wX;                     /* Start pixel to display */
    WORD    wY;                     /* Start pixel to display */
    BYTE    boPainted;
    BYTE    boOn;
    BYTE    bDepth;
    BYTE    bInterlace;
} ST_OSDWIN;


typedef struct{
    WORD    wX;                     /* Start pixel to put character */
    WORD    wY;                     /* Start pixel to put character */
    BYTE    bFontColor;         /* font color. image: Y, OSD: index */
    BYTE    bFontCb;
    BYTE    bFontCr;
    BYTE    bTextGap;           /* Gap between character */
    WORD    wWidth;               /* The font display area */
    WORD    wDisplayWidth;  /* The display font width */
	//BYTE 	bHeight;		// Character Height
	//BYTE 	bWidth;			//Character Width
	BYTE bReserve[2];

	BYTE	bBitsPerPixel; 		// Every Pixel used bit number on this font data
	BYTE	bAlign;        		// Every character line data align byte number
	BYTE	bFontSize; 			// The Font display size
	SBYTE	bYOffset;				// The offset of Y, for fine turn the display base line
	BYTE	bAngle;
	BYTE	bPrintDirection;
} ST_FONT;

//******Mason 20061018 start******//
typedef struct{
    WORD wHSync;    //Reg : TvHCtrl0
    WORD wHBkPorch;
    WORD wHFrPorch;
    WORD wVSync;    //Reg : TvVCtrl0
    WORD wVBkPorch;
    WORD wVFrPorch;
}ST_TV_SYNC;
typedef struct{
    BYTE bId;           //ID, should be unique
    BYTE bPixelMode;    //Stripe / Delta / Digital
    BYTE bReserve[2];
    WORD wWidthInPix;   //Real dot num of panel
    WORD wHeightInPix;
    WORD wWidthInLen;   //Length of panel  // 16:9 or 4:3
    WORD wHeightInLen;
} ST_PANEL;
typedef struct{
    BYTE bId;           //ID, should be unique
    BYTE bBusType;      //DVI, COMPOSITE, etc...
    BYTE bClockSource;  //Generate clock from PLL1, PLL2, or Auto-select
    BYTE bInterlace;    //Interlace / Prograssive
    WORD wIduClock;     //Clock of IDU module, in 100KHz/Sec
    WORD wPixClock;     //Clock of bus, in 100KHz/Sec (Maybe be IDU2 clock or TV clock)
    WORD wWidth;        //Pixel number from MP612 to TCON
    WORD wHeight;
    DWORD dwPLL2Clk;    //Special PLL2 clock, default as 0
    ST_TV_SYNC stTVSync;
} ST_TCON;

#endif

