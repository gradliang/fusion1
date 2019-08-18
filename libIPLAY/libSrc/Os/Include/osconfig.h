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
* Filename      : osconfig.h
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/
///
///@defgroup    CONFIG  Configuration
///
///@{
///
/// All the options defined here should be considered carefully before re-compiling this operating
/// system. Smaller value of these options will allocate smaller size of memory at link time. The
/// designer should make the decision how large these number should be.
///


#ifndef	__OS_CONFIG_H
#define __OS_CONFIG_H


/// Indicate that the SDRAM must be accessed by bank interlaced
#define OS_SDRAM_INTERLACE          1

/// Define the maximal priority number of each task
#define OS_MAX_PRIORITY             16

/// Define the maximal number of task that can be created simultaneously
/// ID is start from 0 to (OS_MAX_NUMBER_OF_TASK - 1)
#if NETWARE_ENABLE
#define OS_MAX_NUMBER_OF_TASK       64 //52
#else
#define OS_MAX_NUMBER_OF_TASK       46 //42 //32
#endif

///@brief   Define the maximal number of object that can be created semultaneously,
///         these objects include the Event Flag, Semaphore, Mailbox and Message Buffer.
#if NETWARE_ENABLE
#if Make_USB == AR9271_WIFI
#define OS_MAX_NUMBER_OF_OBJECT     254 //112//96//32
#elif Make_USB == REALTEK_RTL8188CU || Make_USB == REALTEK_RTL8188EUS
#define OS_MAX_NUMBER_OF_OBJECT     254 //224
#else
#define OS_MAX_NUMBER_OF_OBJECT     162 //112//96//32
#endif
#else
#if USBOTG_WEB_CAM
#define OS_MAX_NUMBER_OF_OBJECT     120 //110 //80//32
#else
#define OS_MAX_NUMBER_OF_OBJECT     116 //110 //80//32
#endif
#endif // #if NETWARE_ENABLE


/// Define the maximal number of mailtag that can be used for the mailbox function.
#define OS_MAX_NUMBER_OF_MAILTAG    32

/// Define the maximal number of time handlers that can be used in timer
#define OS_MAX_NUMBER_OF_TIMER      8

#define OS_MAX_NUMBER_OF_IDLE_FUNC  16

#define OS_MAX_NUMBER_OF_EXCEPTION_TAG  30

#endif	// __OS_CONFIG_H

///@}
