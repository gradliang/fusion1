/*
*******************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2004, Magic Pixel Inc, Hsinchu, Taiwan
*
* All rights reserved. Magic Pixel's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in
* deciphering, decoding, reverse engineering or in ay way altering the source
* code is strictly prohibited, unless the prior written consent of Magic
* Pixel is obtained.
*
* Filename      : ui_timer.h
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/
#ifndef __UI_TIMER_H
#define __UI_TIMER_H

/*
// Include section
*/

//#include "utilregfile.h" //temp_remove
//#include "bitsdefine.h"
#include "iplaysysconfig.h"
//#include "peripheral.h"
//#include "os.h"

/*
// Constant declarations
*/
#define UI_TIMER_TICK_PERIOD        20      // ms per tick


//////These macro are used by timer1 only for avsync.
/*
// Function prototype
*/
int Ui_TimerProcessCheck(void);
void Ui_TimerProcessHandle(void);
void Ui_TimerProcInit(void);
SWORD Ui_TimerProcAdd(register DWORD, register void (*)(void));
SWORD Ui_TimerProcRemove(register void (*)(void));

#define AddTimerProc            Ui_TimerProcAdd
#define RemoveTimerProc         Ui_TimerProcRemove

#endif  // __UI_TIMER_H

