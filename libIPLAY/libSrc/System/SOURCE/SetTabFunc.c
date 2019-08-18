/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "fs.h"
#include "settabfunc.h"
#include "taskid.h"

#if !SETUP_SAVE_IN_UST

#define SETTING_TBL_CONFIRM         0x80800000

#define SETTABLE_PATH_1             "setting1"
#define SETTABLE_PATH_2             "setting2"
#define SETTABLE_EXT                "sys"

static volatile BOOL sttbSemaCreated = FALSE;
static ST_SETTING_SEGMENT  SettingTable[SETTING_SEGMENT_COUNT];
static BOOL boolTableChange = FALSE;
static STREAM* setFile;
static BOOL fileOpenedFlag = 0;

//****** Function prototype **************
static void Sys_SettingTableDelete(BYTE*);


///
///@brief Tag string compare utility
///
///@param input     Input tag string point
///@param inner     The tag string that to be compared
///@param length    The length of the tag string
///
///@retval  0       The input tag equal to the inner tag or the input tag is a null string.
///@retval  <>0     The input tag not equal to the inner tag.
///
///@remark  The tag string compare is provided because the tag string is different to the
///         regular string. It will not be termivated by a null charactor and will have the
///         fixed size length.
///
static SDWORD tagCompare(BYTE *input, BYTE *inner, BYTE length)
{
    //register DWORD counter;
    register SDWORD result = 0;

    if(!(*input))   length = 0;     // if the input string is null, then always match

    while(length)
    {
        if((*input == *inner))      // equal and not zero
        {
            if(!(*input))
                break;
            input++;
            inner++;
            length--;

            continue;
        }
        else if(*input > *inner)
            result = 1;
        else
            result = -1;

        break;
    }

    return result;
}



static ST_SETTING_SEGMENT *tagFind(BYTE *tag)
{
    ST_SETTING_SEGMENT *segment;
    DWORD table_index;

    segment = &SettingTable[0];
    table_index = SETTING_SEGMENT_COUNT;

    while(table_index)
    {
        if((segment->Head & SETTING_HEAD_VLIDE) && (segment->Head & SETTING_HEAD_TAG))
        {
            if(!tagCompare(tag, segment->Data, SETTING_TAG_MAX_LENGTH))
                return segment;
        }

        segment++;
        table_index--;
    }

    return 0;
}



//mpx_SettingTableOpen() must run first
//static ST_SETTING_SEGMENT *tagDump(void)
static SDWORD tagDump(void)
{
    ST_SETTING_SEGMENT *segment;
    DWORD table_index;

    segment = &SettingTable[0];
    table_index = SETTING_SEGMENT_COUNT;

    while(table_index)
    {
        if((segment->Head & SETTING_HEAD_VLIDE) && (segment->Head & SETTING_HEAD_TAG))
        {
            if(segment->Head & SETTING_HEAD_LINKED)
                MP_ALERT("[%d] = %s, \tnext link = %d segment", table_index, segment->Data, segment->SilRbc);
            else
                MP_ALERT("[%d] = %s, \tlength = %d bytes", table_index, segment->Data, segment->SilRbc);
        }

        segment++;
        table_index--;
    }

    return 0;
}



static void validSegmentSet(BYTE *buf, DWORD table_index)
{
    DWORD byte_idx, bit_idx;

    byte_idx = table_index >> 3;
    bit_idx = 7 - (table_index & 0x7);
    buf[byte_idx] |= (1 << bit_idx);
}



static void garbageCollection(void)
{
    ST_SETTING_SEGMENT *segment;
    BYTE *buf;
    DWORD bufSize = SETTING_SEGMENT_COUNT >> 3;
    DWORD table_index, index, i;

    buf = (BYTE*)ext_mem_malloc(bufSize);
    memset(buf, 0, bufSize);

    table_index = 0;
    segment = &SettingTable[0];
    while(table_index < SETTING_SEGMENT_COUNT)
    {
        if((segment->Head & (SETTING_HEAD_VLIDE|SETTING_HEAD_TAG)) == (SETTING_HEAD_VLIDE|SETTING_HEAD_TAG))
        {
            validSegmentSet(buf, table_index);
            while(segment->Head & SETTING_HEAD_LINKED)
            {
                index = (*(WORD*)&segment->Head) & 0x0fff;
                if(index >= SETTING_SEGMENT_COUNT)
                {
                    MP_ALERT("Seg %d has error link when garbage collection", ((DWORD)segment - (DWORD)SettingTable)/SETTING_TABLE_SIZE);
                    break;
                }
                validSegmentSet(buf, index);
                segment = &SettingTable[index];
            }
        }
        else if(!(segment->Head & SETTING_HEAD_VLIDE))
            validSegmentSet(buf, table_index);

        table_index++;
        segment = &SettingTable[table_index];
    }

    //Free lost segment
    for(table_index = 0; table_index < SETTING_SEGMENT_COUNT; table_index += 8)
    {
        i = table_index >> 3;
        for(index = 0; index < 8; index++)
        {
            if(!(buf[i] & 0x80))
            {
                MP_ALERT("seg %5d is lost tag", table_index + index);
                segment = &SettingTable[table_index + index];
                *(WORD*)&segment->Head = 0; //clear Head and SilRbc
            }

            buf[i] <<= 1;
        }
    }

    ext_mem_free(buf);
}



static void sttb_sema_create(void)
{
    if (SemaphoreCreate(STTB_SEMA_ID, OS_ATTR_FIFO, 1) != OS_STATUS_OK)
    {
        mpDebugPrint("STTB semaphore create fail!!!");

        __asm("break 100");
    }

    memset((BYTE *) &SettingTable, 0, sizeof(SettingTable));
    sttbSemaCreated = TRUE;
}



///
///@ingroup SYSTEM
///@brief   Open setting1.sys or setting2.sys
///
///
///@param
///
///@return  PASS : setting1.sys or setting2.sys was opened successfuly.
///         FAIL : setting1.sys or setting2.sys can't be created
///@remark  This function should be run before other setting table functions or the value to put or get might wrong.
///
///
///
SDWORD Sys_SettingTableOpen(void)
{
#if SETUP_TABLE_DIRECT_SPI
    DWORD confirm, value, updateValue;
    DWORD *ptr32;
    SDWORD retVal = PASS;

    if (sttbSemaCreated == FALSE)
        sttb_sema_create();

    if (fileOpenedFlag)
    {
        MP_ALERT("--E-- %s - Previous open has not been closed", __FUNCTION__);
        return retVal;
    }
    
    SemaphoreWait(STTB_SEMA_ID);

    spi_rcmd(SettingTable, SETTING_TABLE_SIZE, SETUP_TABLE_SPI_ADDR);

    confirm = *((DWORD*)(SettingTable));
    value = *(((DWORD*)(SettingTable))+1);
    mpDebugPrint("confirm = 0x%08x, value = 0x%08x", confirm, value);

    if (confirm == SETTING_TBL_CONFIRM) {
        updateValue = value + 1;
    }
    else {
        updateValue = 0;
        memset(SettingTable, 0, SETTING_TABLE_SIZE);
    mpDebugPrint("updateValue = %d", updateValue);
    }

    fileOpenedFlag = 1;
    ptr32 = (DWORD *) SettingTable;
    ptr32[0] = 0xcfffffff;
    ptr32[1] = updateValue;
    ptr32[2] = 0xffffffff;
    ptr32[3] = 0xffffffff;

    return retVal;

#else

    DRIVE *sysDrv;
    BYTE  sysDrvId = SYS_DRV_ID;
    DWORD confirm_1, confirm_2;
    DWORD value_1, value_2, updateValue;
    DWORD *ptr32;
    SDWORD retVal = PASS;
    DRIVE_PHY_DEV_ID phyDevID = DriveIndex2PhyDevID(sysDrvId);

    //if ((phyDevID != DEV_NAND) && (phyDevID != DEV_SPI_FLASH))
    if (sysDrvId == NULL_DRIVE)
    {
        MP_ALERT("--E-- %s: Invalid System Drive ID (= %d) defined ! => use current drive instead ...", __FUNCTION__, SYS_DRV_ID);
        sysDrvId = DriveCurIdGet(); /* use current drive */

        if (sysDrvId == NULL_DRIVE)
        {
            MP_ALERT("%s: --E-- Current drive is NULL !!! Abort !!!", __FUNCTION__);
            return FALSE;
        }

        MP_ALERT("Using current drive-%s !!!", DriveIndex2DrvName(sysDrvId));
    }

    if (sttbSemaCreated == FALSE)
        sttb_sema_create();

    if (fileOpenedFlag)
    {
        MP_ALERT("--E-- %s - Previous open has not been closed", __FUNCTION__);
        return retVal;
    }

    SemaphoreWait(STTB_SEMA_ID);

    {
        STREAM* file_1 = NULL;
        STREAM* file_2 = NULL;
        DWORD fileSize;
        LONG u64Tmp;
        BOOL checkTimes = 1;

STTAB_OPEN_START:
        sysDrv = DriveGet(sysDrvId);
		DirReset(sysDrv);

        // setting1.sys
        if (FileSearch(sysDrv, SETTABLE_PATH_1, SETTABLE_EXT, E_FILE_TYPE) != FS_SUCCEED)
        {
            MP_DEBUG("setting1.sys is not found in drive-%s !! Create it now.", DriveIndex2DrvName(sysDrvId));
            if (CreateFile(sysDrv, SETTABLE_PATH_1, SETTABLE_EXT) != FS_SUCCEED)
            {
                MP_ALERT("-E- setting1.sys in drive-%s can't be created!!!", DriveIndex2DrvName(sysDrvId));
                retVal = FAIL;
                goto STTAB_OPEN_END;
            }
        }

        file_1 = FileOpen(sysDrv);
        fileSize = FileSizeGet(file_1);
        if (fileSize < SETTING_TABLE_SIZE)
        {
            ptr32 = (DWORD *) SettingTable;
            memset((BYTE *) ptr32, 0xff, 16);
            if (FileWrite(file_1, (BYTE *) SettingTable, SETTING_TABLE_SIZE) != SETTING_TABLE_SIZE)
            {
                retVal = FAIL;
                goto STTAB_OPEN_END;
            }
        }

        // setting2.sys
        if (FileSearch(sysDrv, SETTABLE_PATH_2, SETTABLE_EXT, E_FILE_TYPE) != FS_SUCCEED)
        {
            MP_DEBUG("setting2.sys is not found in drive-%s !! Create it now.", DriveIndex2DrvName(sysDrvId));
            if (CreateFile(sysDrv, SETTABLE_PATH_2, SETTABLE_EXT) != FS_SUCCEED)
            {
                MP_ALERT("-E- setting1.sys can't be created!!!");
                retVal = FAIL;
                goto STTAB_OPEN_END;
            }
        }

        file_2 = FileOpen(sysDrv);
        fileSize = FileSizeGet(file_2);
        if (fileSize < SETTING_TABLE_SIZE)
        {
            ptr32 = (DWORD *) SettingTable;
            memset((BYTE *) ptr32, 0xff, 16);
            if (FileWrite(file_2, (BYTE *) SettingTable, SETTING_TABLE_SIZE) != SETTING_TABLE_SIZE)
            {
                retVal = FAIL;
                goto STTAB_OPEN_END;
            }
        }

        Fseek(file_1, 0, SEEK_SET);
        if (FileRead(file_1, (BYTE *) &u64Tmp, 8) != 8)
        {
            retVal = FAIL;
            goto STTAB_OPEN_END;
        }
        Fseek(file_1, 0, SEEK_SET);
        confirm_1 = u64Tmp >> 32;
        value_1 = (DWORD) u64Tmp;

        Fseek(file_2, 0, SEEK_SET);
        if (FileRead(file_2, (BYTE *) &u64Tmp, 8) != 8)
        {
            retVal = FAIL;
            goto STTAB_OPEN_END;
        }
        Fseek(file_2, 0, SEEK_SET);
        confirm_2 = u64Tmp >> 32;
        value_2 = (DWORD) u64Tmp;
    mpDebugPrint("confirm_1 = 0x%x value_1=%d  confirm_2 = 0x%x value_2=%d", confirm_1,value_1,confirm_2,value_2);

        if ((confirm_1 == SETTING_TBL_CONFIRM) && (confirm_2 == SETTING_TBL_CONFIRM))
        {
            if (value_1 > value_2)
            {
                setFile = file_2;
                updateValue = value_1 + 1;
                FileRead(file_1, (BYTE*)SettingTable, SETTING_TABLE_SIZE);
                FileClose(file_1);
            }
            else
            {
                setFile = file_1;
                updateValue = value_2 + 1;
                FileRead(file_2, (BYTE*)SettingTable, SETTING_TABLE_SIZE);
                FileClose(file_2);
            }
        }
        else if (confirm_1 == SETTING_TBL_CONFIRM)
        {
            setFile = file_2;
            updateValue = value_1 + 1;
            FileRead(file_1, (BYTE *) SettingTable, SETTING_TABLE_SIZE);
            FileClose(file_1);
        }
        else if (confirm_2 == SETTING_TBL_CONFIRM)
        {
            setFile = file_1;
            updateValue = value_2 + 1;
            FileRead(file_2, (BYTE *) SettingTable, SETTING_TABLE_SIZE);
            FileClose(file_2);
        }
        else
        {
            setFile = file_1;
            updateValue = 0;
            FileClose(file_2);
            memset(SettingTable, 0, SETTING_TABLE_SIZE);
        }

        fileOpenedFlag = 1;
        ptr32 = (DWORD *) SettingTable;
        ptr32[0] = 0xcfffffff;
        ptr32[1] = updateValue;
        ptr32[2] = 0xffffffff;
        ptr32[3] = 0xffffffff;

STTAB_OPEN_END:
        if (retVal != PASS)
        {
            SemaphoreRelease(STTB_SEMA_ID);
            if (file_1 != NULL)
                FileClose(file_1);
            if (file_2 != NULL)
                FileClose(file_2);
        }
    }

    return retVal;

#endif
}



SDWORD Sys_SettingTableClose(void)
{
#if SETUP_TABLE_DIRECT_SPI

    DWORD confirm_value = SETTING_TBL_CONFIRM;
    SDWORD retVal = PASS;

    if(fileOpenedFlag)     // what if fileOpenedFlag = NULL
    {
        if(boolTableChange)
        {
            DWORD newbuff[sizeof(SettingTable)/4];

            memcpy(newbuff, SettingTable, sizeof(SettingTable));
            newbuff[0] = confirm_value;

            spi_block_erase(SETUP_TABLE_SPI_ADDR, 1);
            spi_wcmd(newbuff, SETTING_TABLE_SIZE, SETUP_TABLE_SPI_ADDR);
    mpDebugPrint("Sys_SettingTableClose confirm = 0x%08x, value = 0x%08x", newbuff[0], newbuff[1]);

            boolTableChange = FALSE;
        }

        fileOpenedFlag = 0;
        SemaphoreRelease(STTB_SEMA_ID);
    }

    return retVal;

#else

    DWORD confirm_value = SETTING_TBL_CONFIRM;
    SDWORD retVal = PASS;

    if(fileOpenedFlag)     // what if fileOpenedFlag = NULL
    {
        if(boolTableChange)
        {
            if(FileWrite(setFile, (BYTE*)SettingTable, SETTING_TABLE_SIZE) != SETTING_TABLE_SIZE)
            {
                retVal = FAIL;
                goto STTAB_CLOSE_END;
            }

            if(retVal = Fseek(setFile, 0, SEEK_SET) != FS_SUCCEED)
                goto STTAB_CLOSE_END;

            if (FileWrite(setFile, (BYTE *) &confirm_value, 4) != 4)
            {
                retVal = FAIL;
                goto STTAB_CLOSE_END;
            }

            boolTableChange = FALSE;
        }

        if(retVal = FileClose(setFile) != FS_SUCCEED)
        {
            MP_ALERT("--E-- setting.sys can't be closed!!!");
            goto STTAB_CLOSE_END;
        }

        fileOpenedFlag = 0;

STTAB_CLOSE_END:
        SemaphoreRelease(STTB_SEMA_ID);

    }

    return retVal;

#endif
}



void Sys_SettingTableDump(void)
{
    tagDump();
}



SDWORD Sys_SettingTableGet(BYTE *tag, void *dataBufPtr, DWORD dataBuffLen)
{
    ST_SETTING_SEGMENT *segment;
    DWORD data_length;
    BYTE *suspect;
    SDWORD return_length;
    DWORD index;

    //data_length = UtilStringLength08(tag);
    data_length = strlen(tag);

    if (data_length > SETTING_TAG_MAX_LENGTH)
        return 0;

    if (!dataBufPtr)
        return 0;

    return_length = 0;
    segment = tagFind(tag);

    if (segment)
    {
        if (data_length < SETTING_TAG_MAX_LENGTH)
            data_length++;

        suspect = &segment->Data[data_length];
        data_length = 14 - data_length;

        while (segment->Head & SETTING_HEAD_LINKED)
        {
            if (data_length > dataBuffLen)
            {
                MP_ALERT("--E1-- Tag-%s - data length exceed to target buffer length !!!", __FUNCTION__, tag);

                return 0;
            }

            memcpy(dataBufPtr, suspect, data_length);
            return_length += data_length;
            dataBuffLen -= data_length;
            dataBufPtr += data_length;
            index = (*(WORD *)&segment->Head)&0x0fff;

            if (index >= SETTING_SEGMENT_COUNT)
            {
                MP_ALERT("--E-- Seg-%d has error link when get setting", ((DWORD)segment - (DWORD)SettingTable)/SETTING_TABLE_SIZE);

                break;
            }

            segment = &SettingTable[index];
            suspect = &segment->Data[0];
            data_length = 14;
        }

        data_length = segment->SilRbc;

        if (data_length > dataBuffLen)
        {
            MP_ALERT("--E2-- Tag-%s - data length exceed to target buffer length !!!", __FUNCTION__, tag);

            return 0;
        }

        memcpy(dataBufPtr, suspect, data_length);
        return_length += data_length;
    }

    return return_length;
}



SDWORD Sys_SettingTablePut(BYTE *tag, void *value, DWORD size)
{

    ST_SETTING_SEGMENT *segment, *prev_segment;
    DWORD table_index, data_length;
    BYTE *suspect;
    SDWORD return_length;
    BOOL first = TRUE, garbageCol = FALSE;

    data_length = strlen(tag);
    if((data_length > SETTING_TAG_MAX_LENGTH) || !size)
    {
        MP_ALERT("tag name is too long ( > 10) or size = 0.");
        return 0;
    }

    boolTableChange = TRUE;
    return_length = 0;

    //find the tag and release its segment
    Sys_SettingTableDelete(tag);

PUT_SETTING_VALUE:
    //find empty segment and write setting value
    table_index = 0;
    segment = &SettingTable[0];
    while(size && table_index < SETTING_SEGMENT_COUNT)
    {
        if(!(segment->Head & SETTING_HEAD_VLIDE))
        {
            if(TRUE == first)
            {
                segment->Head |= (SETTING_HEAD_TAG | SETTING_HEAD_VLIDE);

                //write tag
                if(data_length == SETTING_TAG_MAX_LENGTH)
                    data_length = 10;
                else
                    data_length++;
                memcpy(segment->Data, tag, data_length);

                suspect = &segment->Data[data_length];
                first = FALSE;

                data_length = 14 - data_length;
            }
            else
            {
                *(WORD *)(&prev_segment->Head) |= ((SETTING_HEAD_LINKED << 8) | table_index);

                segment->Head |= SETTING_HEAD_VLIDE;
                suspect = &segment->Data[0];
                data_length = 14;
            }

            //write data
            if(data_length >= size)
            {
                segment->SilRbc = size;
                data_length = size;
            }
            memcpy(suspect, value, data_length);

            return_length += data_length;
            prev_segment = segment;
            value += data_length;
            size -= data_length;
        }

        segment++;
        table_index++;


    }

    if(size)
    {
        if(!garbageCol)
        {
            MP_ALERT("Garbage collection");
            garbageCollection();
            garbageCol = TRUE;
            goto PUT_SETTING_VALUE;
        }
        else
        {
            boolTableChange = FALSE;
            MP_ALERT("Setting table is overflow");
        }
    }


    return return_length;
}



void Sys_SettingTableClean()
{
    //memset(((DWORD)SettingTable+sizeof(ST_SETTING_SEGMENT)), 0, SETTING_TABLE_SIZE - sizeof(ST_SETTING_SEGMENT));
    memset((BYTE *) &(SettingTable[1]), 0, SETTING_TABLE_SIZE - sizeof(ST_SETTING_SEGMENT));
}



#if 1
static void Sys_SettingTableDelete(BYTE *tag)
{
    ST_SETTING_SEGMENT *segment;

    segment = tagFind(tag);
    if(segment)
    {
        WORD index;
        MP_DEBUG("Clean original setting");

        while(1)
        {
            if(!(segment->Head & SETTING_HEAD_LINKED))
            {
                *(WORD *)&segment->Head = 0; //clear Head and SilRbc
                break;
            }

			index = (*(WORD *)&segment->Head) & 0x0fff;
			*(WORD *)&segment->Head = 0; //clear Head and SilRbc
            if(index > SETTING_SEGMENT_COUNT)
            {
                MP_ALERT("Seg %d has error link when delete setting", ((DWORD)segment - (DWORD)SettingTable)/SETTING_TABLE_SIZE);
                break;
            }
            segment = &SettingTable[index];
        }
    }
}
#else
SDWORD mpx_SettingTableDelete(BYTE *tag)
{
    ST_SETTING_SEGMENT *segment;
    DWORD data_length;
    SDWORD return_length;

    return_length = 0;
    data_length = UtilStringLength08(tag);
    if(data_length <= SETTING_TAG_MAX_LENGTH)
    {
        segment = tagFind(tag);
        if(segment)
        {
            WORD index;

            if(data_length < SETTING_TAG_MAX_LENGTH)
                data_length++;
            data_length = 14 - data_length;

            while(segment->Head & SETTING_HEAD_LINKED)
            {
                return_length += data_length;

                index = (*(WORD *)&segment->Head)&0x0fff;
                *(WORD *)&segment->Head = 0; //clear Head and SilRbc
                segment = &SettingTable[index];
                data_length = 14;
            }

            data_length -= segment->SilRbc;
            return_length += data_length;
            *(WORD *)&segment->Head = 0; //clear Head and SilRbc
        }
    }

    MP_ALERT("Setting value delete:%3d", return_length);
    return return_length;
}
#endif


/*
void ISP_SettingTableDump(void)
{
    tagDump();
}



void ISP_SettingTableOpen(void)
{
    IspFunc_ReadUST((BYTE*)SettingTable, SETTING_TABLE_SIZE);

}



void ISP_SettingTableClose(void)
{
    IspFunc_WriteUST((BYTE*)SettingTable, SETTING_TABLE_SIZE);
}
*/
#endif

