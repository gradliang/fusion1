/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2005, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:    agent.h
*
* Programmer:    Programmer
*                MPX E120 division
*
* Created: xx/xx/2005
*
* Description: The purpose, functionality and role in whole system
*              (ex: 321/612/323) ...
*
*
* Change History (most recent first):
*     <1>     xx/xx/2005    Programmer    first file
****************************************************************
*/
#ifndef __AGENT_H
#define __AGENT_H

#include "UtilTypeDef.h"

void HandlerEnable(BOOL en);
void DebugPrintString(BYTE * string);
DWORD IsGdbConnect(void);
void DebugPrint(char *file, int line, char *fmt, ...);

#endif  // __AGENT_H

