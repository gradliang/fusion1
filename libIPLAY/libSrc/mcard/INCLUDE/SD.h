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
* Filename      : sd.h
* Programmer(s) : 
* Created       : 
* Descriptions  :
*******************************************************************************
*/
#ifndef __SD_H
#define __SD_H
/*
// Include section 
*/

#define SD_TYPE			1
#define SDHC_TYPE		2
#define MMC_TYPE		3
#define SDIO_TYPE		4
#define SDXC_TYPE		5

#define DBUS_1BIT		0x0
#define DBUS_4BIT		0x1
#define DBUS_8BIT		0x2

#define NORMAL_SPEED	0
#define HIGH_SPEED		1

#define OP_RADTC		0x4
#define OP_WADTC		0x2
#define OP_NADTC		0x1

#define CMD0			(0  | (OP_NADTC << 8))	// GO_IDLE_STATE
#define CMD1			(1  | (OP_NADTC << 8))	// SEND_OP_COND
#define CMD2			(2  | (OP_NADTC << 8))	// ALL_SEND_CID
#define CMD3			(3  | (OP_NADTC << 8))	// SEND_RELATIVE_ADDR
#define MMC_CMD6		(6  | (OP_NADTC << 8))	// SWITCH_FUNCTION
#define SD_CMD6			(6  | (OP_RADTC << 8))	// SWITCH_FUNCTION
#define CMD7			(7  | (OP_NADTC << 8))	// SELECT_CARD
#define CMD8			(8  | (OP_NADTC << 8))	// SEND_IF_COND
#define CMD9			(9  | (OP_NADTC << 8))	// SEND_CSD
#define CMD12			(12 | (OP_NADTC << 8))	// STOP_TRANSMISSION
#define CMD13			(13 | (OP_NADTC << 8))	// SEND_STATUS
#define CMD16			(16 | (OP_NADTC << 8))	// SET_BLOCKLEN
#define CMD17			(17 | (OP_RADTC << 8))	// READ_SINGLE_BLOCK
#define CMD18			(18 | (OP_RADTC << 8))	// READ_MULTIPLE_BLOCK
#define CMD24			(24 | (OP_WADTC << 8))	// WRITE_SINGLE_BLOCK
#define CMD25			(25 | (OP_WADTC << 8))	// WRITE_MULTIPLE_BLOCK
#define CMD55			(55 | (OP_NADTC << 8))	// ACMD
#define ACMD6			(6  | (OP_NADTC << 8))	// SET_BUS_WIDTH
#define ACMD22			(22 | (OP_NADTC << 8))  // SEND_NUM_WR_BLOCKS
#define ACMD41			(41 | (OP_NADTC << 8))	// SD_APP_OP_COND
#define ACMD51			(51 | (OP_RADTC << 8))	// SEND_SCR

// SD status
#define NULL_STATE		0
#define STANDBY_STATE	3
#define TRANSFER_STATE	4
#define RECEIVE_STATE	6
#define PROGRAM_STATE	7

///
///@defgroup SD_MMC SD and MMC
///@ingroup CONSTANT
///@{
/// The device's function executes commands without any problem.
#define SD_PASS				0
#define BAD_CRC7			-1
/// Wait for response's signal fail.
#define RES_TIMEOUT			-2
/// When reading data, card had no response with previous command.
#define READ_TIMEOUT		-3
/// When writing data, card had no response with previous command.
#define WRITE_TIMEOUT		-4
#define TRANSFER_FAIL		-5
#define PROGRAM_FAIL		-6
/// When reading data, the data had failed the CRC examination
#define BAD_CRC16			-7
#define CARD_NOT_SUPPORT	-8
/// Unknown or general error encounter.
#define GENERAL_FAIL		-9
///@}
/*
// Function prototype 
*/

void SdHost_DataConfig(DWORD data_nr);
void SdHost_WpConfig(DWORD);
void SdHost_SetHostBlkLen(DWORD length);
void SdHost_GetResponse(DWORD *Resp);
void SdHost_SendCmd(WORD wCommand, DWORD dwArgument);
SWORD SdHost_WaitCardResponse();
void SdHost_PhyTransfer(DWORD dwBufferAddress, DWORD dwSize, DWORD dwDirection);
void SdHost_DmaReset();
SWORD SdHost_WaitTransfer();
void McardSdFreeDataPin(void);
void McardSdActive(DWORD CardType, DWORD BusWidth, DWORD HiSpeed);
void McardSdInactive(DWORD cardtype);
BYTE SdHost_GetCardWP();
#endif  // __SD_H

