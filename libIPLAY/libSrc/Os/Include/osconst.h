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
* Filename      : osconst.h
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/
///
///@defgroup	CONST	Constants
///
///@{


#ifndef __OSCONST_H
#define __OSCONST_H

#define OS_STATE_NON_EXISTENT			0x00
#define OS_STATE_RUNNING				0x01
#define OS_STATE_READY					0x02
#define OS_STATE_WAITING				0x04
#define OS_STATE_SLEEPING				0x08
#define OS_STATE_DORMANT				0x10
#define OS_STATE_WAKEUP					0x20

#define OS_ATTR_DEFAULT					0x00
#define OS_ATTR_ACTIVE					0x02
#define OS_ATTR_FIFO					0x00
#define OS_ATTR_PRIORITY				0x01
#define OS_ATTR_WAIT_SINGLE				0x00
#define OS_ATTR_WAIT_MULTIPLE			0x02
#define OS_ATTR_EVENT_CLEAR				0x04


#define OS_MAIL_EMPTY					0x00
#define OS_MAIL_PACKED					0x01
#define OS_MAIL_DELIVERED				0x02
#define OS_MAIL_WAITED					0x03
#define OS_MAIL_RELEASED				0x04


#define OS_TIMER_FINE					0x04
#define OS_TIMER_NORMAL					0x10
#define OS_TIMER_ROUGH					0x40


#define OS_EVENT_AND					0
#define OS_EVENT_OR						1

#define OS_IDLE_FUNC_TERMINATE          -1

/// The API function execution complete with any probkem
#define OS_STATUS_OK					0

/// The specified ID is greater than the maximal number
#define OS_STATUS_INVALID_ID			-1

/// The OS specific memory heap is full
#define OS_STATUS_NO_MEMORY				-2

/// The reserve the attribute bit field has been fill
#define OS_STATUS_RESERVE_ATTR			-3

/// The specified parameters is wrong
#define OS_STATUS_WRONG_PARAM			-4

/// The specified ID has been allocated by others
#define OS_STATUS_ID_INUSED				-5

/// Some illegal status happen
#define OS_STATUS_ILLEGAL				-6

/// The polling result is fail or the polled condition not yet satisfied
#define OS_STATUS_POLLING_FAILURE		-7

/// The buffer that will be accessed is full
#define OS_STATUS_OVERFLOW				-8

/// The specified mailbox tag is greater then the maximal number
#define OS_STATUS_INVALID_TAG			-9

/// The specifed mailbox tag has been allocated others
#define OS_STATUS_TAG_INUSED			-10

/// The current state of the specified task is invalid for the attempted function
#define OS_STATUS_WRONG_STATE			-11

/// The current state of the specified task is waiting timeout
#define OS_STATUS_TIMEOUT				-12

#endif		// __OSCONST_H


///@}
