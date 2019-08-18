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
* Filename      : index.h
* Programmers   : 
* Created       : 
* Descriptions  : 
*******************************************************************************
*/
#ifndef __INDEX_H
#define __INDEX_H


///@mainpage FileSystem
///
///The FileSystem module is a software module designed for MagicPixel's iPlay platform.\n
///It provides the API interfaces for file system operations, include drive, directory, and file operations.\n
///And these API interfaces are conceptually divided to some layers.\n\n
///Currently, we only support Microsoft FAT12, FAT16 abd FAT32 file systems.\n\n
///Note that the FileSystem APIs of MagicPixel's iPlay platform is proprietarily designed and 
///therefore not very compliant to the File I/O API interfaces in POSIX standard or C standard library.\n
///While porting code from Unix/Linux open source code or other OS to MagicPixel's iPlay platform,
///cares should be taken for the different usage detail of APIs between these platforms. \n\n
///All rights are reserved for MagicPixel. \n\n
///
///@image html FS_sublayers.gif
///



///
///@defgroup FS_Wrapper   High-level wrapper functions for MagicPixel native file system functions
///High-level wrapper functions for the native MagicPixel file system functions.\n
///The wrapper functions in this group are intended to be similar as the File I/O functions in C standard
///library or in POSIX standard.\n\n
///Note that it is still very weak and not ready in this group.
/// 



///
///@defgroup FILE   File access
///File-related operations.\n\n
///The functions in this group are mainly for the normal file operations, including
///open/read/write/close/create/delete/search, ... etc.\n
///The file operations are based on stream file I/O concept, i.e. any file is a randomly accessible sequence of bytes.\n
///To reduce complexity, any stream file is bidirectional, i.e. any opened file can be written or read.\n\n
///The functions in this group are conceptually higher-level than other groups in our FileSystem module.
/// 



///
///@defgroup DIRECTORY   Directory and FDB node access
///Directory-specific operations and directory entry (i.e. FDB node) related operations.\n\n
///The term "node" represents the basic unit in a directory. A "node" is also called as a "directory entry".
///It could be a file or a subdirectory.\n
///In FAT12/16/32 file systems, the FDB (file description block) is a 32-byte structure designed to store the information of a node.\n
///The information within a FDB node includes the 8.3 format short file name, file attribute, create and last access date/time, \n
///file size and the starting cluster number of the chain of that file or directory. \n\n
///To support long filename and Unicode, Microsoft introduced another 32-byte FDB type called "Long name FDB" accompanying with \n
///the original short name FDB. Depending on the filename length of a file/directory, it may have only exact one short name FDB node \n
///or several FDB nodes (one short name FDB with several long name FDB nodes) for that file/directory. \n\n
///We keep information in the DRIVE structure of each storage drive to track the current working directory and current
///working FDB node on the drive.\n
///The functions in this group can then operate based on this current working directory and FDB node.
///
///The diagrams below show the short name FDB and long name FDB structures: \n\n
///@image html FS_short_name_FDB.gif
/// \n\n
///@image html FS_long_name_FDB.gif
///



///
///@defgroup CHAIN   Chain access
///Chain-level operations.\n\n
///The so-called "chain" is a chain of data clusters allocated by a file or by a directory. The chain is like a 
///"singly linked list" because of the essential design of the FAT table in FAT12/16/32 file systems.\n
///The chain-level operations mainly process the chain data or the seeking in a chain.\n\n
/// 
///The diagram below is a typical storage device layout of FAT file system: \n\n
///@image html FS_FAT_Drive_layout.gif
///



///
///@defgroup DRIVE   Drive access
///Drive-level and drive-related operations.\n\n
///A 'DRIVE' structure is a software object that is used to support all the file system functions for an enabled storage device drive.\n
///We maintain an internal DRIVE table for keeping all the DRIVE status information of all supported storage devices in our system.\n  
///When an supported storage device drive is plugged in and detected, a DRIVE entry will be allocated from the DRIVE table 
///for that device drive if the partition type or file system format of that storage drive is FAT12, FAT16 or FAT32.\n\n
///The drive-level operations are the lowest level operations in our FileSystem module, and it is the interface to
///underlying driver modules of storage devices.
/// 



///
///@defgroup FILE_BROWSER   File Browser operations for GUI
///Operations for GUI File Browser which is for presentation of file lists on the GUI.
/// 



//Close this special group because it is not suitable as public API for Doxygen document
#ifdef DOXYGEN_SHOW_INTERNAL_USAGE_API
///
///@defgroup For_VideoPlayback   Variant of CHAIN group operations for video playback only
///Variant functions of the CHAIN group operations for video playback only.
/// 
#endif



///
///@defgroup DATA_STRUCT   Data structures
///Define data structure types used in the file system.
///



///
///@defgroup FS_VALUE_DEF   Constants and return/error codes
///Define constants used in the file system and return/error codes of the file system operations.
///



///
///@defgroup FS_CONST   Constants used in the file system
///@ingroup  FS_VALUE_DEF
///Define the constants used in the file system.
///



///
///@defgroup FS_ERROR_CODE   Return/error codes of the file system operations
///@ingroup  FS_VALUE_DEF
///Define the return or error codes of the file system operations.
///



///
///@defgroup FS_SampleCode   Examples / Sample codes
///Some examples or sample codes for the usage of some file system API functions.
///



///
///@defgroup FS_Sample1   Sample code 1 
///@ingroup  FS_SampleCode
///@htmlinclude   FS_Sample1.html
///
///@defgroup FS_Sample2   Sample code 2 
///@ingroup  FS_SampleCode
///@htmlinclude   FS_Sample2.html
///
///@defgroup FS_Sample3   Sample code 3 
///@ingroup  FS_SampleCode
///@htmlinclude   FS_Sample3.html
///
///@defgroup FS_Sample4   Sample code 4 
///@ingroup  FS_SampleCode
///@htmlinclude   FS_Sample4.html
///



#endif //__INDEX_H
