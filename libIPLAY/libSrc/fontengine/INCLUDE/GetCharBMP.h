#ifndef __GETCHARBMP_H__
#define __GETCHARBMP_H__

#include "global612.h"
#include "display.h"

#if 0
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

typedef enum  Font_Data_At_
{
  Font_Data_At_Memory = 0,
  Font_Data_At_File
} Font_Data_At;

typedef enum  Display_Mode_
{
  Display_Mode_YYCBCR = 0,
  Display_Mode_RGB
} Display_Mode;


//Map to FT_Render_Mode_
typedef enum  RENDERING_
{
  RENDERING_NORMAL = 0,
  RENDERING_LIGHT,
  RENDERING_MONO,
  RENDERING_LCD,
  RENDERING_LCD_V,
  RENDERING_MAX
} RENDERING;

typedef enum  ENCODING_
{
  ENCODING_NONE = 0,
  ENCODING_UNICODE,
  ENCODING_MS_SYMBOL,
  ENCODING_ADOBE_LATIN_1 ,
  ENCODING_OLD_LATIN_2,
  ENCODING_SJIS,
  ENCODING_GB2312,
  ENCODING_BIG5,
  ENCODING_WANSUNG,
  ENCODING_JOHAB,
  ENCODING_ADOBE_STANDARD,
  ENCODING_ADOBE_EXPERT,
  ENCODING_ADOBE_CUSTOM,
  ENCODING_APPLE_ROMAN
} ENCODING;



typedef struct
{
  ST_IMGWIN* pDisplay;
  DWORD*     string;
  int        string_length;
  int        fontsize;
  RENDERING  rendering;
  ENCODING   encoding;
  int        x;
  int        y;
  int        count;
  unsigned int Font_Y;
  unsigned int Font_Cb;
  unsigned int Font_Cr;

  unsigned int Rectangle_Y;
  unsigned int Rectangle_Cb;
  unsigned int Rectangle_Cr;

  int          Rectangle_ON;
  
  int Rectangle_X_Min;
  int Rectangle_X_Max;
  int Rectangle_Y_Min;
  int Rectangle_Y_Max;
  
} ST_PRINT_TTF_DATA;


#endif
