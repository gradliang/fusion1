#ifndef _IDU_TEST_H
#define _IDU_TEST_H


#include "displayStructure.h"

//***********************************************************************
//                                          define
//***********************************************************************
#ifndef IDU_CHANGEWIN_MODE
    #define IDU_CHANGEWIN_MODE          0       // 0: Polling, 1: SW toggle
#endif

// from idu.h
#define DISP_TYPE_D_SUB                 6
#define DISP_TYPE_DVI_666               7
#define DISP_TYPE_DVI_888               8
#define DISP_TYPE_COMPONENT             9
#define DISP_TYPE_COMPOSITE             10
#define DISP_TYPE_S_VIDEO               11
#define DISP_TYPE_CCIR601               12
#define DISP_TYPE_CCIR656               13
#define DISP_TYPE_LCD                   14
#define DISP_TYPE_RGB24                 15
#define DISP_TYPE_RGB_A                 16
#define DISP_TYPE_RGB18                 17
#define	DISP_TYPE_LVDS_SINGLE_666				18
#define	DISP_TYPE_LVDS_SINGLE_888				19
#define	DISP_TYPE_LVDS_DOUBLE_666				20
#define	DISP_TYPE_LVDS_DOUBLE_888				21
#define DISP_TYPE_RGB8_SERIAL           22
#define DISP_TYPE_ITU709               23
#define DISP_TYPE_I80                 24

// video format type
#if 0
#define OSD_COLOR_RED                           9
#define OSD_COLOR_GREEN                         10
#define OSD_COLOR_BLUE                          11
#define OSD_COLOR_YELLOW                        12
#define OSD_COLOR_PINK                          13
#define OSD_COLOR_BABYBLUE                      14
#define OSD_BLENDING                            (-6)
#endif
// IDU_EVENT_ID
#define IDU_CHANGE_WIN_FINISHED                 BIT0
#define IDU_FRAME_END_A                         BIT1
#define IDU_FRAME_END_B                         BIT2
#define IDU_FRAME_END_A2                        BIT3
#define IDU_FRAME_END_B2                        BIT4

#define IDU_FONT_YUVCOLOR_DEFAULT_WHITE                       0xffff8080
#define IDU_FONT_YUVCOLOR_BLACK                       						0x00008080
#define IDU_FONT_YUVCOLOR_GREY                       						0xc0c08080

#if (OSD_BIT_WIDTH == 8)//Mason 20080603, move from platform file,Wendy copy 20080916
#define OSD_BIT_OFFSET  0
#define OSD_MASK        0x7f
#elif (OSD_BIT_WIDTH == 4)
#define OSD_BIT_OFFSET  1
#define OSD_MASK        0xf
#elif (OSD_BIT_WIDTH == 2)
#define OSD_BIT_OFFSET  2
#define OSD_MASK        0x3
#endif

#define PRINT_DIRECTION_UP			0
#define PRINT_DIRECTION_DOWN		1
#define PRINT_DIRECTION_LEFT		2
#define PRINT_DIRECTION_RIGHT		3

#define ROTATE_RIGHT_0     0
#define ROTATE_RIGHT_90    1
#define ROTATE_RIGHT_180   2
#define ROTATE_RIGHT_270   3


#define Idu_PrintString(trgWin, string, startX, startY, UnicodeFlag,wWidth) Idu_PrintStringWithSize(trgWin, string, startX, startY, UnicodeFlag,wWidth, 0, DEFAULT_FONT_SIZE)
//#define Idu_GetStringWidth(pbString, boUnicodeFlag) Idu_GetStringWidthWithSize(pbString, boUnicodeFlag, DEFAULT_FONT_SIZE)
#define Idu_OsdPutStr(psWin, pbString, startX, startY, bIndex) Idu_OsdPutStrWithSize(psWin, pbString, startX, startY, bIndex, 0, 0, DEFAULT_FONT_SIZE)


extern ST_IMGWIN *psCurrWin, *psNextWin;
extern ST_IMGWIN sWin[2];				// used for real screen buffer
extern ST_TCON g_mstTCON[];
extern ST_PANEL g_mstPanel[];
extern BYTE g_bPrevBlending;

extern char *PANEL_ID_NAME[];
extern char *TCON_ID_NAME[];


;//***********************************************************************
//                                      Function
//***********************************************************************
ST_IMGWIN *Idu_GetNextWin();
ST_IMGWIN *Idu_GetCurrWin();
ST_IMGWIN *Idu_GetBuffWin();
void Idu_ChgWin(ST_IMGWIN *);
void Idu_ChgWinWaitFinish(ST_IMGWIN * ImgWin, BOOL needWaitFinish);
void Idu_SetWinClipRegion(ST_IMGWIN *pWin, WORD wLeft, WORD wTop, WORD wRight, WORD wBottom);
void Idu_ResetWinClipRegion(ST_IMGWIN *psWin);
//void Idu_SetFontColor(BYTE R, BYTE G, BYTE B);
//WORD Idu_PrintString(ST_IMGWIN *, BYTE *, WORD, WORD, BYTE, WORD);
WORD Idu_PrintStringWithSize(ST_IMGWIN *, BYTE *, WORD, WORD, BYTE, WORD, WORD, BYTE);
void Idu_Draw1BitBitmap(ST_IMGWIN *pWin, WORD x, WORD y, WORD w, WORD h, BYTE *pImgBuffer, BYTE bColor);
//WORD Idu_GetStringWidth(BYTE *pbString, BYTE boUnicodeFlag);
WORD Idu_GetStringWidthWithSize(BYTE *pbString, BYTE boUnicodeFlag, BYTE Font_Size);
WORD Idu_GetStringWidth(BYTE *pbString, BYTE UnicodeFlag);
BYTE Idu_GetStringHeight(BYTE *pbString, BYTE boUnicodeFlag);
void Idu_InitPrintPosition(ST_IMGWIN *, WORD, WORD);
void Idu_PrintLyrics(ST_IMGWIN *, BYTE *, BYTE);
void Idu_PrintMovieTime(DWORD);
void Idu_PaintWin(ST_IMGWIN *ImgWin, DWORD dwColor);
void Idu_PaintWinArea(ST_IMGWIN *ImgWin, WORD x, WORD y, WORD w, WORD h, DWORD dwColor);
void Idu_ChangeResolution(DWORD, DWORD, BYTE);
BOOL Idu_SetHighResolution();
void Idu_SetImageGamma(void);

WORD Idu_PrintStringCenter(ST_IMGWIN * trgWin, BYTE * string, WORD startX, WORD startY, BYTE UnicodeFlag, WORD wWidth);
WORD Idu_PrintStringRight(ST_IMGWIN * trgWin, BYTE * string, WORD startX, WORD startY, BYTE UnicodeFlag);

void Idu_FontColorSet(BYTE R, BYTE G, BYTE B);

ST_OSDWIN *Idu_GetOsdWin();
void Idu_OsdErase();
void Idu_OsdOnOff(BYTE);
///
///@ingroup group_OSD
///@brief   Paint area on OSD win
///
///@param   WORD startX    Horizontal starting address
///@param   WORD startY    Vertical starting address
///@param   WORD SizeX     Horizontal size
///@param   WORD SizeY     Vertical size
///@param   BYTE b_PaletteIndex          Color index of the color to paint
///
///@retval  NONE
///
///@remark
///
void Idu_OsdPaintArea(WORD startX, WORD startY, WORD SizeX, WORD SizeY, BYTE b_PaletteIndex);
//void Idu_OSDInit(DWORD *buff, int width, int height, BYTE bInterlace, BYTE bitWidth);
void Idu_OSDInit(DWORD * pdwBuffer, int width, int height, BYTE bInterlace, BYTE bitWidth, BOOL ClearOsd);
///
///@ingroup group_OSD
///@brief   Load palette for OSD win
///
///@param   BYTE b_Blending    Blending
///
///@retval  TRUE if success, FALSE if fail
///
///@remark
///
void Idu_OsdSetDefaultPalette(void);
void DmaIduIsr();
void Idu_RunPAL();
void Idu_RunNTSC();
void Idu_OsdDrawBitmap4Bit(WORD x, WORD y, WORD w, WORD h, BYTE *pImgBuffer, BYTE bColor);
void Idu_OsdDrawImage(WORD x, WORD y, WORD w, WORD h, BYTE *pImgBuffer);
void Idu_OsdPutRectangle(ST_OSDWIN *, WORD, WORD, WORD, WORD, WORD, BYTE);
//void Idu_OsdPutStr(ST_OSDWIN *, BYTE *, WORD, WORD, BYTE);
void Idu_OsdPutStrWithSize(ST_OSDWIN * psWin, BYTE * pbString, WORD startX, WORD startY, BYTE bIndex, BYTE UnicodeFlag, WORD wWidth, BYTE Font_Size);
void Idu_OSDPrint(ST_OSDWIN *, BYTE *, WORD, WORD, BYTE);
void Idu_SetLargeFont(BYTE flag);
void Idu_Display_CCIR();
void Idu_Display_COMPOSITE();
void Idu_Display_COMPOSITENTSC();
void Idu_Display_COMPOSITEPAL();
void Idu_Display_TVIN();
BOOL Idu_GetIs444Mode(void);

void Idu_WaitBufferEnd();

#if 0//OSD_BMP
int Osd_DrawBmpByHandle(STREAM* handle, DWORD X, DWORD Y, BOOL boCoverPalette, BYTE blend, BOOL boEnableTransprt);
int Osd_DrawBmpByBuffer(const BYTE* buffer, DWORD dwBufSize, DWORD X, DWORD Y, BOOL boCoverPalette, BYTE blend, BOOL boEnableTransprt);
int OSD_LoadPaletteFromBmpFile(STREAM* handle, BYTE blend);
int OSD_LoadPaletteFromBmpBuffer(const BYTE* buffer, DWORD dwBufSize, BYTE blend);
#endif


#endif



