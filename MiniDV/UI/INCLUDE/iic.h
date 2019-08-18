/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2005, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:    iic.h
*
* Programmer:    Joe.H
*                MPX E320 division
*
* Created: 9/5/2005
*
* Description:
* Version : 0001
*
****************************************************************
*/
#ifndef __IIC_H
#define __IIC_H

/*
// Include section
*/
#include "global612.h"
#include "peripheral.h"

/*
// Constant declarations
*/
/*
//user-defined IIC event
#define IIC_ERROR        			0
#define IIC_UP								1
#define IIC_DOWN							2
#define IIC_LEFT							3
#define IIC_RIGHT							4
#define IIC_MENU							5
#define IIC_SOURCE						6
#define IIC_EXIT							7
#define IIC_PP								8
#define IIC_BACKWARD					9
#define IIC_FORWARD		 				10
#define IIC_STOP				 			11
#define IIC_VOLUP			 				12
#define IIC_VOLDOWN		 				13
#define IIC_ENTER		 					14
#define IIC_POWER							15
#define IIC_SETUP							16
#define IIC_INFO							17
#define IIC_BRIGHTNESS				18
#define IIC_REPEAT						19
#define IIC_MUTE							20
#define IIC_PREVIOUS					21
#define IIC_NEXT							22
#define IIC_PIC_MP3						23
#define IIC_ZOOM							24
#define IIC_ROTATE						25
#define IIC_PAL_NTSC					26
*/

void UI_I2CS_WriteIsr(void);
void UI_I2CS_ReadIsr(void);

#endif // __IIC_H

