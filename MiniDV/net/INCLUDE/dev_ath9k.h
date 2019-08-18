/*
*******************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based 
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2010, Magic Pixel Inc, Hsinchu, Taiwan
*
* All rights reserved. Magic Pixel's source code is an unpublished work and the 
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in 
* deciphering, decoding, reverse engineering or in ay way altering the source 
* code is strictly prohibited, unless the prior written consent of Magic 
* Pixel is obtained.
* 
* Created       : 2010/11/01 
* Description   : Copy of some defines from AR9271 driver code.
******************************************************************************** 
*/

#ifndef __DEV_AR9271_H__
#define __DEV_AR9271_H__ 

/**
 * enum ath_debug_level - atheros wireless debug level
 *
 * @ATH_DBG_RESET: reset processing
 * @ATH_DBG_QUEUE: hardware queue management
 * @ATH_DBG_EEPROM: eeprom processing
 * @ATH_DBG_CALIBRATE: periodic calibration
 * @ATH_DBG_INTERRUPT: interrupt processing
 * @ATH_DBG_REGULATORY: regulatory processing
 * @ATH_DBG_ANI: adaptive noise immunitive processing
 * @ATH_DBG_XMIT: basic xmit operation
 * @ATH_DBG_BEACON: beacon handling
 * @ATH_DBG_CONFIG: configuration of the hardware
 * @ATH_DBG_FATAL: fatal errors, this is the default, DBG_DEFAULT
 * @ATH_DBG_PS: power save processing
 * @ATH_DBG_HWTIMER: hardware timer handling
 * @ATH_DBG_BTCOEX: bluetooth coexistance
 * @ATH_DBG_ANY: enable all debugging
 *
 * The debug level is used to control the amount and type of debugging output
 * we want to see. Each driver has its own method for enabling debugging and
 * modifying debug level states -- but this is typically done through a
 * module parameter 'debug' along with a respective 'debug' debugfs file
 * entry.
 */
enum ATH_DEBUG {
	ATH_DBG_RESET		= 0x00000001,
	ATH_DBG_QUEUE		= 0x00000002,
	ATH_DBG_EEPROM		= 0x00000004,
	ATH_DBG_CALIBRATE	= 0x00000008,
	ATH_DBG_INTERRUPT	= 0x00000010,
	ATH_DBG_REGULATORY	= 0x00000020,
	ATH_DBG_ANI		= 0x00000040,
	ATH_DBG_XMIT		= 0x00000080,
	ATH_DBG_BEACON		= 0x00000100,
	ATH_DBG_CONFIG		= 0x00000200,
	ATH_DBG_FATAL		= 0x00000400,
	ATH_DBG_PS		= 0x00000800,
	ATH_DBG_HWTIMER		= 0x00001000,
	ATH_DBG_BTCOEX		= 0x00002000,
	ATH_DBG_WMI		= 0x00004000,
	ATH_DBG_ANY		= 0xffffffff
};

#define ATH_DBG_DEFAULT (ATH_DBG_FATAL)


#endif // __DEV_AR9271_H__

