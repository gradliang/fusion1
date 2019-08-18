#ifndef __PRINTTTFCONFIG_H__
#define __PRINTTTFCONFIG_H__

#include "Basic_Struct.h"
#include "Basic_Enum.h"

typedef struct
{
	COORDINATE_ORIGIN_MODE    CoordinateOriginMode;
	CHANGE_LINE_X_ALIGN_MODE  ChangeLineXAlignMode;
	PRINT_REGION_MODE         PrintRegionMode;
    stPoint                   PrintPos;            //Print start position
	int                       LineHeightPixel;
	stRectangle               PrintRegion;
	int                       SpecifiedChangeLineX;
	unsigned int              NotDrawOverBorder;
}stPrintCoordinate, *pstPrintCoordinate;

typedef struct
{
    COLOR_FORMAT   Format;
    stColorData    Data;
	float          Alpha;
}stColorParameter, *pstColorParameter;

typedef struct
{
	FT_Face              Face;
	FT_Library           Library;
}stFontFace, *pstFontFace;


typedef struct
{
    long*                Str;                 //Content of the string to print
    int                  StrLength;           //Count of characters in the string
    long*                StrLineHeightRef;       //
    int                  StrLineHeightRefLength; //
    int                  StrLineHeightCompensate;//
	int                  Str_Index_Start;     //Start print at which char
	int                  Str_Index_Stop;      //Stop print at which char
	long*                DelimiterList;
	int                  DelimiterListLength;
    int                  FontSize;            //Size of the character to print
    
}stStrParameter, *pstStrParameter;

typedef struct
{
    unsigned char*   ImagePlane;          //Pointer to display target memory
    stSize           ImagePlaneSize;      //Size of display target memory
    DISPLAY_MODE     ImagePlaneMode;	  //Mode of the image plane
	stSize           DeviceResolution;
}stImagaPlaneParameter, *pstImagaPlaneParameter;

typedef struct
{
	stSize             RectangleSize;               //[out] The size of the rectangle of the string
    int                Count;                       //[out] Number of characters printed
    stPoint            NextPos;                     //[out] Prepare start position for the next string
    int                StrHBearingYPixel;           //[out] The gap between highest pixel of the string and baseline
    int                StrHBearingXPixelFirstGlyph; //[out] The gap between x of origin and most left pixel of the string
	int                LastNotCroppedGlyphIndex;
	int                LastChangeLineGlyphIndex;
	int                LastGlyphIndexAtFirstCroppedLine; //
}stOutputData, *pstOutputData;

typedef struct
{
	stImagaPlaneParameter ImagePlaneData;
    MEASURE_MODE          MeasureMode;
    GLYPH_SET_MODE        GlyphSetMode;
	stStrParameter        StringData;
	stPrintCoordinate     PrintCoordinate;
    FT_Render_Mode        Rendering;           //Rendering setting
    FT_Encoding           Encoding;            //Encoding setting
    int                   BlendingOn;

    stColorParameter      ColorParameter_Glyph;       //The color format and data of the string
    stColorParameter      ColorParameter_Rectangle;   //The color format and data of the rectangle
    stColorParameter      ColorParameter_Baseline; //The color format and data of the rectangle
    stColorParameter      ColorParameter_PrintRegion; //The color format and data of the rectangle

    int                   RectangleOn;         //Draw a ractangle around the string , >0: yes, 0: no
    int                   BaselineOn;          //Draw the baseline of the string , >0: yes, 0: no
    int                   PrintRegionOn;          //Draw the baseline of the string , >0: yes, 0: no
    //int                   GlyphCacheOn;        //Enable cache function , >0: yes, 0: no
	int                   PrintStringOn;		//

	stCacheSetting        CacheSetting;
    //Output data
	stOutputData          OutputData;
	
	int                   CompactMetricsOn;
	int		     FontRotation;     //0,90,180,270 
	int		     RotationFontOrderDirection;     // 0:( Rotat 90,270) up to down 1: ( Rotat 90,270) down to up
} stPrintTTFConfig, *pstPrintTTFConfig;



#endif	//__PRINTTTFCONFIG_H__

