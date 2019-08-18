#ifndef READER_FONTSTRUCT_H
#define READER_FONTSTRUCT_H

#include "iplaysysconfig.h"



typedef struct
{
	int x;
	int y;
}stPosition, *pstPosition;

typedef struct
{
	stPosition pt_Start;
	stPosition pt_End;
}stPrintRegion, *pPrintRegion;

typedef struct
{
	BYTE 	R;
	BYTE 	G;
	BYTE 	B;
	BYTE	bRev;
}stColor, *pstColor;


typedef struct
{
	int                DrawStrLineHigh;                       //[out] Number of characters printed
	int                DrawStrStartIndex;
	int                DrawStrEndIndex;
	stPosition      NextPos;                     //[out] Prepare start position for the next string

}PrintOutputData, *pPrintOutputData;


typedef struct
{
	int                  	Str_Index_Start;     //Start print at which char
	int                  	Str_Index_Stop;      //Stop print at which char
	int				ChangeLineXAlignPos ;
	stPosition			PrintPos;            //Print start position
	stPrintRegion		PrintRegion;
	stColor 			Color;
	ST_IMGWIN*		PWin;
	BYTE			Font_size;
	BYTE			UnicodeFlag;
	PrintOutputData	OutputData;

}Reader_IduStringConfig, *PReader_IduStringConfig;


#endif

