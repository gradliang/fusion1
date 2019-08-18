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
* Filename      : McardApi.h
* Programmer(s) : 
* Created       : 
* Descriptions  :
*******************************************************************************
*/
#ifndef __MCARD_API_H
#define __MCARD_API_H

///
///@mainpage iPlay Mcard Device System
///
///"Mcard" means memory card which used flash memory as physical storage. The
///iPlay Mcard is actually a storage device driver system which supports many
///kinds of storage device including memory cards, hard drive and even flash
///memory directly.
///
///Mcard provide a uniform interface to access to different device driver such
///as SD/MMC/MS/xD/CF/Nand/Nor/HDD. Each of them are reigstered in Mcard device
///task. Mcard device task is a dedicated task serviced at high priority
///(DRIVER_PRIORITY). To change priority, please modify taskid.h.
///
///To Communicate with this task, the only way is go through with Mcard APIs.
///Every APIs are implement as synchronized function which will never return
///before either finished or failed.

/*
// Include section 
*/
#include "utiltypedef.h"
#include "devio.h"


#define SD_TYPE_SDSC  0x01
#define SD_TYPE_SDHC  0x02
#define SD_TYPE_MMC   0x03
//#define SD_TYPE_SDIO  0x04
#define SD_TYPE_SDXC  0x05

/*
// Functions for external
*/

///
///@defgroup MCARD_SYS Init and ISR
///
///When system startup, Mcard device system could be initialed after OS and Memory
///management module ready. And it should be initialed before system start accessing to
///any of Mcard device. Here also provide two interrupt service routine for callback.


///
///@ingroup MCARD_SYS
///@brief   Initial all available device drivers and create Mcard device task.
///
///@remark  The Mcard_Init function installs designate device driver,
///                 and then create Mcard device task to current system.
///
///@param   cardCtl    Used to config some mcard subsystem setting
///                            Bit0 : If this bit is 1, disable card detecting function
///                            Bit1 : If this bit is 1, disable write protecting function 
///
void Mcard_Init(BYTE cardCtl);

///
///@ingroup MCARD_SYS
///@brief   Mcard interrupt service routine
///
///@remark  Most of device driver need interrupt from chip to check whether
///         the hardware job is finished or not.
///                  
///
void McardIsr();
	
///
///@ingroup MCARD_SYS
///@brief   Mcard DMA interrupt service routine
///
///@remark  Actually, this is reserved for future. No device driver need it
///         currently.
///                  
///
void DmaMcardIsr();

///
///@ingroup MCARD_SYS
///@brief   Register a callback function for card detection
///
///@remark  When card insert or remove, func_ptr will be called and
///			two parameter be passed. 1st parameter is TRUE/FALSE for
///			insert/remove. 2nd one is card's device ID.
///                  
///
void RegisterCardDetectCB(void (*func_ptr)(DWORD, DWORD));

///
///@ingroup MCARD_SYS
///@brief   Register a callback function for card fatal error
///
///@remark  When a card has a fatal error(It was based on specific card), func_ptr will be called
///			Parameter one is card's device ID.
///                  
///
void RegisterCardFatalErrorCB(void (*func_ptr)(DRIVE_PHY_DEV_ID));

///
///@defgroup MCARD_INFO Get/set information
///
///There are many kinds of information in Mcard device system. These functions
///return quickly no matter get or set informations. Most of them just deal
///with Mcard device system's internal variables from RAM.

///
///@ingroup MCARD_INFO
///@brief   Get the internal information structure of the Mcard device driver
///
///
///@retval  struct ST_MCARD_DEVS_TAG *   the same as ST_MCARD_DEVS *, is a pointer
///										of a structure
/// 
///@remark  This function returns the pointer of the Mcard device driver's internal
///			data structure. This is for USB host only
///
struct ST_MCARD_DEVS_TAG *GetMcardDevTag4Usb (void);

///
///@defgroup MCARD_INFO Get/set information
///
///There are many kinds of information in Mcard device system. These functions
///return quickly no matter get or set informations. Most of them just deal
///with Mcard device system's internal variables from RAM.

///
///@ingroup MCARD_INFO
///@brief   Get the internal information structure of the Mcard device driver
///
///@param   bMcardID    ID number of the Mcard device driver
///
///@retval  struct ST_MCARD_DEV_TAG *   the same as ST_MCARD_DEV *, is a pointer
///										of a structure
/// 
///@remark  This function returns the pointer of the Mcard device driver's internal
///			data structure.
///
struct ST_MCARD_DEV_TAG *Mcard_GetDevice(E_DEVICE_ID bMcardID);

///
///@ingroup MCARD_INFO
///@brief   Get the description of the Mcard device driver
///
///@param   bMcardID    ID number of the Mcard device driver
///
///@retval  BYTE*   the address of description which is a null-terminal ascii string.
/// 
///@remark  This function returns the pointer of the description according to
///         the bMcardID. 
///
BYTE *Mcard_GetDescriptor(E_DEVICE_ID bMcardID);

///
///@ingroup MCARD_INFO
///@brief   Get the count of the installed device driver. 
///
///@remark  This function returns the count of the installed device driver.
///                  
///
BYTE Mcard_GetDeviceNum();

///
///@ingroup MCARD_INFO
///@brief   Get the capcity of the Mcard device driver
///
///@param   bMcardID    ID number of the Mcard device driver
///
///@retval  DWORD   The number of total sectors that are available
///					on the storage device.
/// 
///@remark  This function returns the total sectors which are available 
///			for file system to access.
/// 
DWORD Mcard_GetCapacity(E_DEVICE_ID bMcardID);

///
///@ingroup MCARD_INFO
///@brief   Using lun number to get current working device driver's ID
///
///@param   bLunNum    lun number
///
///@retval  BYTE   ID number of Mcard device driver
/// 
///@remark  This function returns the current working device driver's ID
///			number according to lun number.
/// 
E_DEVICE_ID Mcard_CurLunGetCardID(BYTE bLunNum);

///
///@ingroup MCARD_INFO
///@brief   Using device driver's ID to get lun number
///
///@param   bMcardID    ID number of the Mcard device driver
///
///@retval  BYTE   lun number
/// 
///@remark  This function returns the lun number according specific device
///			driver's ID number.
/// 
BYTE Mcard_CurCardIDGetLun(E_DEVICE_ID bMcardID);

///
///@ingroup MCARD_INFO
///@brief   Using device driver's ID to get renew count.
///
///@param   bMcardID    ID number of the Mcard device driver
///
///@retval  WORD   renew count
/// 
///@remark  This function returns the renew count according specific device
///			driver's ID number. The "renew" means how many times this device
///			have been identified.
/// 
WORD Mcard_GetRenewCounter(E_DEVICE_ID bMcardID);

///
///@ingroup MCARD_INFO
///@brief   Using device driver's ID to get sector size.
///
///@param   bMcardID    ID number of the Mcard device driver
///
///@retval  WORD   count of bytes per sector
/// 
///@remark  This function returns the sector size according specific device
///			driver's ID number. The sector size is the actually access unit
///			of this Mcard device.
/// 
WORD Mcard_GetSectorSize(E_DEVICE_ID bMcardID);

///
///@ingroup MCARD_INFO
///@brief   Using device driver's ID to check whether the device is available
///			or not.
///
///@param   bMcardID    ID number of the Mcard device driver
///
///@retval  BYTE   TRUE or FALSE for available or not.
/// 
///@remark  This function returns whether the Mcard device is available or not after
///			identified.
/// 
BYTE Mcard_GetFlagPresent(E_DEVICE_ID bMcardID);

///
///@ingroup MCARD_INFO
///@brief   check all device is available or not. if one device presents, it will return true.
///
///@param   bMcardID    ID number of the Mcard device driver
///
///@retval  BYTE   TRUE or FALSE for available or not.
/// 
///@remark  This function returns whether all of Mcard device is available or not after
///			identified.
/// 
BYTE Mcard_GetAllFlagPresent( void );

///
///@ingroup MCARD_INFO
///@brief   Using device driver's ID to check whether be able to write
///
///@param   bMcardID    ID number of the Mcard device driver
///
///@retval  BYTE   TRUE means read only, FALSE means could be written.
/// 
///@remark  This function returns whether the Mcard device is read only or not.
///			If it is read only device, Mcard_DeviceWrite will always do nothing
///			and return FAIL soon.
/// 
BYTE Mcard_GetFlagReadOnly(E_DEVICE_ID bMcardID);

///
///@ingroup MCARD_INFO
///@brief   Using device driver's ID to check whether device inserted
///
///@param   bMcardID    ID number of the Mcard device driver
///
///@retval  BYTE   TRUE means deivce inserted, FALSE means doesn't.
/// 
///@remark  This function returns whether the storage is inserted or not.
///			This result is according to detect pin only. The Mcard device inserted
///			doesn't mean the Mcard device present.
/// 
BYTE Mcard_GetDetected(E_DEVICE_ID bMcardID);

///
///@ingroup MCARD_INFO
///@brief   Install a Mcard device driver by its ID.
///
///@param   bMcardID    ID number of the Mcard device driver
///
///@remark  This function is for USB host device.
/// 
//void DeviceEnableByMcardId(E_DEVICE_ID bMcardID);

///
///@ingroup MCARD_INFO
///@brief   set lun number to specific device driver
///
///@param   bMcardID	ID number of the Mcard device driver
///@param   lun			lun number for USB device identify
///
/// 
///@remark  This function sets lun number to specific device driver according to
///			its ID number. The lun number is meaningless to Mcard system but meaningful
///			to USB device system.
///
void Mcard_DeviceLunSet(E_DEVICE_ID bMcardID, BYTE lun);

///
///@ingroup MCARD_INFO
///@brief   configure the necessary pins for Mcard system to operate.
///
///@param   pintype		another ID difine in global612.h
///@param   nr			pin numbers
///@param   *pins			array of pin number
///
/// 
///@remark  This function delivers pin numbers. Mcard module will physical operate on
///			these pins by Hardware purpose.
///
void Mcard_DeviceFgpioConfig(BYTE pintype, BYTE nr, const BYTE *pins);

///
///@ingroup MCARD_INFO
///@brief   configure SD card special pins
///
///@param   WP_pin		SD's write protection pin
///@param   Data_pin	the start pin number of SD's data pins
///
/// 
///@remark  This function configure the special pin of SD card.
///
void Mcard_DeviceSdConfig(BYTE WP_pin, BYTE Data_pin);

///
///@ingroup MCARD_INFO
///@brief   configure available Mcard working clock.
///
/// 
///@remark  This function will pre-generate a table of available Mcard working clock
///			before Mcard device is actual working.
///
void Mcard_DeviceClkConfig();


///
///@ingroup MCARD_INFO
///@brief initial card detect gpio (MS, XD, CF) when define the MCARD_POWER_CTRL,
///          It's unnecessary to call without card power control.
///
///@param None
///
///@retval None
///
///@remark The function will be called otherwise  the chip can't detect any card in socket
void McardCardDetectionEnable( void);

///
///@defgroup MCARD_ACCESS Device access
///
///To access to Mcard device actually, functions will send Mailbox to Mcard device task.
///After Mailbox sent, functions will track Mail that cause current task sleep until
///Mcard device task release Mail back. That means a task try to access Mcard device actually
///will become a sleeping task before access finished.

///
///@ingroup MCARD_ACCESS
///@brief turn on card power with gpio
///
///@param DevId card device ID 
///
///@retval none
///
///@remark
void McardBootUpPowerOff();

///
///@ingroup MCARD_ACCESS
///@brief   Power on and identify the specific Mcard device.
///
///@param   bMcardID    ID number of the Mcard device driver
///
///@retval  SWORD		Identification successful or not
/// 
///@remark  This function will power on Mcard device and start identifying.
///			Always call this function after Mcard device was detected.
///
SWORD Mcard_DeviceInit(E_DEVICE_ID bMcardID);

///
///@ingroup MCARD_ACCESS
///@brief   Power off Mcard device and reset Mcard device driver
///
///@param   bMcardID    ID number of the Mcard device driver
///
///@retval  SWORD		power off successful or not
/// 
///@remark  This function will power off and clean internal variables of Mcard
///			device driver no matter the Mcard device is remove from slot or not.
///
SWORD Mcard_DeviceRemove(E_DEVICE_ID bMcardID);

///
///@ingroup MCARD_ACCESS
///@brief   provide a low level format(erase)
///
///@param   bMcardID    ID number of the Mcard device driver
///@param   deepVerify  Erase with Write/Read verification on each bit
///
///@retval  SWORD		low level format successful or not
/// 
///@remark  This function will provide a low level format. But not all Mcard devices
///			could support low level format function. It depends on physical function.
///
SWORD Mcard_DeviceRawFormat(E_DEVICE_ID bMcardID, BYTE deepVerify);

///
///@ingroup MCARD_ACCESS
///@brief   read data from Mcard device to RAM buffer
///
///@param   drv		DRIVE information from file system layer
///@param   buffer	a start address of the read target buffer, address should be
///					aligned to DWORD
///@param   lba		sector address
///@param   len		amount of sector count
///
///@retval  SWORD	Read finished with problem or not
/// 
///@remark  This function read data from Mcard device to RAM buffer. Mcard device driver
///			will read from sector number "lba" of Mcard device and read out "len" sectors.
///			Sector size is from Mcard_GetSectorSize.
///
SWORD Mcard_DeviceRead(DRIVE *drv, BYTE *buffer, DWORD lba, DWORD len);

///
///@ingroup MCARD_ACCESS
///@brief   write data from RAM buffer to Mcard device
///
///@param   drv		DRIVE information from file system layer
///@param   buffer	a start address of the source buffer, address should be
///					aligned to DWORD
///@param   lba		sector address
///@param   len		amount of sector count
///
///@retval  SWORD	Read finished with problem or not
/// 
///@remark  This function write data from RAM buffer to Mcard device. Mcard device driver
///			will write from sector number "lba" of Mcard device and write "len" sectors.
///			Sector size is from Mcard_GetSectorSize.
///
SWORD Mcard_DeviceWrite(DRIVE *drv, BYTE *buffer, DWORD lba, DWORD len);

///
///@ingroup MCARD_ACCESS
///@brief   To know the progress of asynchronizing job.
///
///@param   AsyncID	ID of asynchronizing job
///
///@retval  DWORD	sector counts of asynchronizing progress, 0xffffffff means
///					the job already been finished.
/// 
///@remark  To reduce waiting time, it would be better to check progress
///			at the same even if the job is not finished.
///
DWORD Mcard_DeviceAsyncProgress(BYTE AsyncID);

///
///@ingroup MCARD_ACCESS
///@brief   wait until asynchronized job finished
///
///@param   AsyncID	ID of asynchronizing job
///
///@retval  DWORD	execute result of asynchronized job
/// 
///@remark  This function will cause the task yield until the Mcard task
///			finished the specific asynchronized job.
///
SWORD Mcard_DeviceAsyncWait(BYTE AsyncID);

///
///@ingroup MCARD_ACCESS
///@brief   Asynchronized low level format
///
///@param   bMcardID    ID number of the Mcard device driver
///@param   deepVerify  Erase with Write/Read verification on each bit
///
///@retval  SWORD		ID of asynchronizing job
/// 
///@remark  This function will provide an asynchronized low level format. But not
///			all Mcard devices could support low level format function. It depends
///			on physical function.
///
BYTE Mcard_DeviceAsyncRawFormat(E_DEVICE_ID bMcardID, BYTE deepVerify);

///
///@ingroup MCARD_ACCESS
///@brief   Asynchronized read data from Mcard device to RAM buffer
///
///@param   drv		DRIVE information from file system layer
///@param   buffer	a start address of the read target buffer, address should be
///					aligned to DWORD
///@param   lba		sector address
///@param   len		amount of sector count
///
///@retval  SWORD	ID of asynchronizing job
/// 
///@remark  This function read data from Mcard device to RAM buffer. Mcard device driver
///			will read from sector number "lba" of Mcard device and read out "len" sectors.
///			Sector size is from Mcard_GetSectorSize.
///
BYTE Mcard_DeviceAsyncRead(DRIVE *drv, BYTE *buffer, DWORD lba, DWORD len);

///
///@ingroup MCARD_ACCESS
///@brief   Asynchronized write data from RAM buffer to Mcard device
///
///@param   drv		DRIVE information from file system layer
///@param   buffer	a start address of the source buffer, address should be
///					aligned to DWORD
///@param   lba		sector address
///@param   len		amount of sector count
///
///@retval  SWORD	ID of asynchronizing job
/// 
///@remark  This function write data from RAM buffer to Mcard device. Mcard device driver
///			will write from sector number "lba" of Mcard device and write "len" sectors.
///			Sector size is from Mcard_GetSectorSize.
///
BYTE Mcard_DeviceAsyncWrite(DRIVE *drv, BYTE *buffer, DWORD lba, DWORD len);
#if (ISP_FUNC_ENABLE == ENABLE)
///
///@ingroup MCARD_ACCESS
///@brief   read raw data from Nand flash to RAM buffer
///
///@param   bDrvID	nand driver ID
///@param   lba		physical sector address
///@param   buffer	a start address of the read target buffer, address should be
///					aligned to DWORD
///@param   size	number of sectors to read
///
///@retval  SWORD	Read finished with problem or not
/// 
///@remark  This function read raw data from Nand flash to RAM buffer. Nand driver
///			will read from sector number "lba" of Nand and read out 1 sector.
///			Sector size is always be 512. Nand driver always has one more 
///			abstract layer to access actual physical address. This function will ignore
///			abstract layer and access to physical layer directly.
///
SWORD IspRead(E_DEVICE_ID bDrvID, DWORD lba, BYTE *buffer, DWORD size);

///
///@ingroup MCARD_ACCESS
///@brief   write raw data from RAM buffer to Nand
///
///@param   bDrvID	nand driver ID
///@param   buffer	a start address of the source buffer, address should be
///					aligned to DWORD
///@param   size	number of blocks to be written
///@param   lba		physical sector address
///
///@retval  SWORD	Read finished with problem or not
/// 
///@remark  This function write raw data from RAM buffer to Nand. Nand driver will
///			write from sector number "lba" of Nand and write 1 block.
///			block size could be calculated by (SectorPerBlock x 512) which SectorPerBlock
///			is from IspGetInfo(). Nand driver always has one more abstract layer to
///			access actual physical address. This function will ignore
///			abstract layer and access to physical layer directly.
///
SWORD IspWrite(E_DEVICE_ID bDrvID, DWORD lba, BYTE *buffer, DWORD size);
#endif

#if ((ISP_FUNC_ENABLE == ENABLE) && (BOOTUP_TYPE == BOOTUP_TYPE_NAND))
///
///@ingroup MCARD_ACCESS
///@brief   check block valid or not
///
///@param   bNandID	nand driver ID
///@param   lba		physical sector address
///
///@retval  SWORD	TRUE means the block is bad one.
/// 
///@remark  This function use physical sector address to check whether the address is
///			located at bad block.
///
SWORD IspBlockIsBad(E_DEVICE_ID bNandID, DWORD lba);

///
///@ingroup MCARD_ACCESS
///@brief   get nand flash storage geometry
///
///@param   bNandID	nand driver ID
///@param   *SectorPerBlk	how many sector per block
///@param   *blknr			how many block in this nand flash
///@param   *pageSize       how many bytes per one page
/// 
///@remark  This function returns physical geometry of nand flash.
///
void IspGetInfo(E_DEVICE_ID bNandID, DWORD *SectorPerBlk, DWORD *blknr, DWORD *pageSize);

///
///@ingroup MCARD_ACCESS
///@brief   erase a nand flash block
///
///@param   bNandID	nand driver ID
///@param   lba		physical sector address
///@param   nr		block number
///
///@retval  SWORD	TRUE means the block is erased successful.
/// 
///@remark  This function will erase "nr" blocks from "lba" sector address.
///
BYTE IspEraseBlk(E_DEVICE_ID bNandID, DWORD lba, DWORD nr);
#endif

///
///@ingroup MCARD_SYS
///@brief   This function is used to query the SD card type.
///
///@retval  BYTE	The SD type of SD card slot.
///
///@remark  This function will return the SD card type on the sd card slot 1.
///

BYTE Mcard_TypeSDCard(void);

#endif  //__MCARD_H

