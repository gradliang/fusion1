/*
*******************************************************************************
*                          	Magic Pixel Inc. Inc.                         	 *
*                  Copyright (c) 2009 -, All Rights Reserved                  *
*                                                                             *
* Shell.                                                                      *
*                                                                             *
* File : tcons.h                                                              *
* By : Kevin Huang                                                            *
*                                                                             *
*                                                                             *
* Description : Header file of test console										 *
*                                                                             *
* History : 2009/03/18 File created.                                          *
*                                                                             *
*******************************************************************************
*/
#ifndef _TCONS_H
#define _TCONS_H
/*************************************************************************/
/***               Public Variable Declaration                         ***/
/*************************************************************************/
typedef void 				(*voidfun)();

#define NoCmd 			100
#define CmdStringLength 128
#define HISTORY_NO      5        
#define PromptLength   	20
#define PARA_NUM		10
#define PARA_LENGTH		20

#define SHELL_POLL 0

#define BT_UART_API
/*************************************************************************/
/***               			Functions definition        		   	   ***/
/*************************************************************************/
SDWORD Check32bit( BYTE * TestString );
SDWORD Check16bit( BYTE * TestString );
SDWORD Check8bit( BYTE * TestString );
void DumpMem8( DWORD iLength, volatile BYTE * StartAddress );
void DumpMem32( DWORD iLength, volatile DWORD * StartAddress );
DWORD StrToInt( BYTE * ptr );
SWORD endianTrans_short( SWORD source );
SDWORD endianTrans_long( SDWORD source );
BOOL mpxCheckComCount(void);
SBYTE mpxKeyGetc(void);
void tConsoleEnable(void);
void tConsoleDisable(void);
SDWORD Loader_init(SBYTE *fname, BOOL free, SDWORD argc, SBYTE *argv[]);
#endif

