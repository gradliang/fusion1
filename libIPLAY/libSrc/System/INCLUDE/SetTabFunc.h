#ifndef __SETTABLEFUNC_H__
#define __SETTABLEFUNC_H__

#include "iplaysysconfig.h"
//#include "system.h"


#define SETTING_TAG_MAX_LENGTH  10

#define SETTING_HEAD_VLIDE      0x80
#define SETTING_HEAD_TAG        0x40
#define SETTING_HEAD_LINKED     0x20


///
/// @brief Data structure of the basic unit "Segment" for the system setting value
///
typedef struct
{
    BYTE Head;       ///< [7] valid(1)/in-valid(0)
                    ///< [6] with tag(1)/without tag(0)
                    ///<     if this segment is the first segment of a setting record and go with
                    ///<     a tag string then this bit should be set
                    ///< [5] with-link(1)/terminator(0)
                    ///<     when 0 if this is the last segment of a setting record
                    ///<     when 1, Head(3:0):SilRbc(7:0) will contain a 12-bits index value
                    ///<     indicating to the next segment.

    BYTE SilRbc;     ///< Next segment index link if Head[5] is set
                    ///< Residue byte count if Head[5] is not set

    BYTE Data[14];   ///< storage for the setting value
} ST_SETTING_SEGMENT;

#define SETTING_TABLE_SIZE          0x1000 //4KB  //0x4000 //16KB
#define SETTING_SEGMENT_COUNT       (SETTING_TABLE_SIZE/sizeof(ST_SETTING_SEGMENT))


SDWORD Sys_SettingTableOpen(void);
SDWORD Sys_SettingTableClose(void);
void Sys_SettingTableDump(void);
SDWORD Sys_SettingTableGet(BYTE *tag, void *dataBufPtr, DWORD dataBuffLen);
S32 Sys_SettingTablePut(U08 *tag, void *value, U32 size);
void Sys_SettingTableClean();


#endif

