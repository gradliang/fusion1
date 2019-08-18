/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "Display.h"
#include "mpapi.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif

#if (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_SD))

#define RES_TAG_SIZE                0x20
#define USER_SET_TAG_FILENAME       "/user.res"
#define FACTORY_SET_TAG_FILENAME    "/factory.res"
static STREAM *resFileHandle = NULL;

static STREAM *ispResourceFileOpen(void)
{
    E_DRIVE_INDEX_ID currDriveId, sysDriveId = SYS_DRV_ID;
    DRIVE *drive;
    BYTE fileName[32];

    MP_DEBUG("%s -", __FUNCTION__);

    if (resFileHandle)
        return resFileHandle;

    currDriveId = DriveCurIdGet();

    if (sysDriveId == NULL_DRIVE)
    {
        MP_ALERT("--E-- %s: Invalid System Drive ID defined ! => use current drive instead ...", __FUNCTION__);
        sysDriveId =  DriveCurIdGet();
    }

    drive = DriveChange(sysDriveId);

    if (drive == NULL)
    {
        MP_ALERT("--E-- %s: Change to %s fail !!!", __FUNCTION__, DriveIndex2DrvName(sysDriveId));

        return NULL;
    }

    sprintf(fileName, "/%s.res", ISP_FILENAME);
    resFileHandle = PathAPI__Fopen(fileName, "rb");

    if (!resFileHandle)
        MP_ALERT("--E-- %s: %s file open fail !!!", __FUNCTION__, fileName);

    DriveChange(currDriveId);

    return resFileHandle;
}



// AP code start address is fixed at block 1 (block 0 is reserved for bootup)
int IspFunc_Write(BYTE *data, DWORD len, DWORD tag)
{
    DRIVE *drive;
    STREAM *handle;
    E_DRIVE_INDEX_ID *drvID, currDriveId, sysDriveId = SYS_DRV_ID;
    PATH_TARGET_TYPE *entrytype;
    BYTE fileName[32];
    int ret = PASS;

    MP_DEBUG("%s -", __FUNCTION__);

    switch (tag)
    {
    case AP_TAG:
    case RES_TAG:
    case USER_SET_TAG:
    case FACTORY_SET_TAG:
        break;

    default:
        MP_ALERT("--E-- %s: Wrong Tag - %4s!!!", __FUNCTION__, &tag);
        return FAIL;
    }

    currDriveId = DriveCurIdGet();

    if (sysDriveId == NULL_DRIVE)
    {
        MP_ALERT("--E-- %s: Invalid System Drive ID defined ! => use current drive instead ...", __FUNCTION__);
        sysDriveId =  DriveCurIdGet();
    }

    drive = DriveChange(sysDriveId);

    if (drive == NULL)
    {
        MP_ALERT("--E-- %s: Change to %s fail !!!", __FUNCTION__, DriveIndex2DrvName(sysDriveId));

        return FAIL;
    }

    switch (tag)
    {
    case AP_TAG:
        sprintf(fileName, "/%s.bin", ISP_FILENAME);
        // Drop header part
        data += 32;
        len -= 32;
        break;

    case RES_TAG:
        if (resFileHandle)
            FileClose(resFileHandle);

        resFileHandle = NULL;
        sprintf(fileName, "/%s.res", ISP_FILENAME);
        break;

    case USER_SET_TAG:
        memcpy(fileName, USER_SET_TAG_FILENAME, strlen(USER_SET_TAG_FILENAME));
        break;

    //case FACTORY_SET_TAG:
    default:
        memcpy(fileName, FACTORY_SET_TAG_FILENAME, strlen(FACTORY_SET_TAG_FILENAME));
        break;
    }

    handle = PathAPI__Fopen(fileName, "wb");

    if (!handle)
    {
        MP_ALERT("--E-- %s: %s file open fail !!!", __FUNCTION__, fileName);
        DriveChange(currDriveId);

        return FAIL;
    }

    if (File_ReleaseSpace_For_OverwriteContent(handle) != FS_SUCCEED)
    {
        MP_ALERT("--E-- %s: File_ReleaseSpace_For_OverwriteContent fail !!!", __FUNCTION__);
        DeleteFile(handle);
        DriveChange(currDriveId);

        return FAIL;
    }

    if (FileWrite(handle, data, len) != len)
    {
        MP_ALERT("--E-- %s: File write eror !!!", __FUNCTION__);
        ret = FAIL;
    }

    FileClose(handle);
    DriveChange(currDriveId);

    return ret;
}



DWORD IspFunc_Read(BYTE *buf, DWORD size, DWORD tag)
{
    DRIVE *drive;
    STREAM *handle;
    E_DRIVE_INDEX_ID *drvID, currDriveId, sysDriveId = SYS_DRV_ID;
    PATH_TARGET_TYPE *entrytype;
    BYTE fileName[32];
    int ret = PASS;

    MP_DEBUG("%s -", __FUNCTION__);

    switch (tag)
    {
    case AP_TAG:
    case RES_TAG:
    case USER_SET_TAG:
    case FACTORY_SET_TAG:
        break;

    default:
        MP_ALERT("--E-- %s: Wrong Tag - %4s!!!", __FUNCTION__, &tag);
        return FAIL;
    }

    currDriveId = DriveCurIdGet();

    if (sysDriveId == NULL_DRIVE)
    {
        MP_ALERT("--E-- %s: Invalid System Drive ID defined ! => use current drive instead ...", __FUNCTION__);
        sysDriveId =  DriveCurIdGet();
    }

    drive = DriveChange(sysDriveId);

    if (drive == NULL)
    {
        MP_ALERT("--E-- %s: Change to %s fail !!!", __FUNCTION__, DriveIndex2DrvName(sysDriveId));

        return FAIL;
    }

    switch (tag)
    {
    case AP_TAG:
        sprintf(fileName, "/%s.bin", ISP_FILENAME);
        break;

    case RES_TAG:
        if (resFileHandle)
            FileClose(resFileHandle);

        resFileHandle = NULL;
        sprintf(fileName, "/%s.res", ISP_FILENAME);
        break;

    case USER_SET_TAG:
        memcpy(fileName, USER_SET_TAG_FILENAME, strlen(USER_SET_TAG_FILENAME));
        break;

    //case FACTORY_SET_TAG:
    default:
        memcpy(fileName, FACTORY_SET_TAG_FILENAME, strlen(FACTORY_SET_TAG_FILENAME));
        break;
    }

    handle = PathAPI__Fopen(fileName, "rb");

    if (!handle)
    {
        MP_ALERT("--E-- %s: %s file open fail !!!", __FUNCTION__, fileName);
        DriveChange(currDriveId);

        return FAIL;
    }

    if (FileRead(handle, (BYTE *) buf, size) == 0)
    {
        MP_ALERT("--E-- %s: File read eror !!!", __FUNCTION__);

        ret = FAIL;
    }

    FileClose(handle);
    DriveChange(currDriveId);

    return ret;
}



DWORD IspFunc_ReadAP(BYTE *buf, DWORD size)
{
    return IspFunc_Read(buf, size, AP_TAG);
}



DWORD IspFunc_ReadRES(BYTE *buf, DWORD size)
{
    return IspFunc_Read(buf, size, RES_TAG);
}



#define RES_STREAM_BUFFER_SIZE          1024

BYTE *IspFunc_ReadRESOURCE(DWORD res_type, BYTE *pbTarget, DWORD dwSize)
{
    STREAM *handle;
    DWORD i;
    DWORD resStarAddr;
    DWORD resNum;
    DWORD resSize;
    DWORD resOffset;
    DWORD *tmpBuf;
    DWORD *buf;

    MP_DEBUG("%s -", __FUNCTION__);

    handle = ispResourceFileOpen();

    if (!handle)
    {
        MP_ALERT("--E-- open resource file fail !!!");
        return NULL;
    }

    Fseek(handle, 0, SEEK_SET);
    buf = (DWORD *) ext_mem_malloc(RES_STREAM_BUFFER_SIZE);
    tmpBuf = buf;

    if (!tmpBuf)
    {
        MP_ALERT("--E-- %s: Can't allocate memory !!!", __FUNCTION__);

        return NULL;
    }

    tmpBuf = (DWORD *) ((DWORD) tmpBuf | BIT29);

    if (FileRead(handle, (BYTE *) tmpBuf, RES_STREAM_BUFFER_SIZE) == 0)
    {
        MP_ALERT("--E-- %s: Can't read RES_TAG", __FUNCTION__);

        return NULL;
    }

    resNum = *(tmpBuf + 9);

    if (resNum == 0xFFFFFFFF)
    {
        MP_ALERT("--E-- %s: Wrong resource number !!!", __FUNCTION__);

        return NULL;
    }

    resStarAddr = RES_TAG_SIZE + ((resNum + 1) << 3);                           // resource data start address
    tmpBuf += 10;                                                               // offset to first resource tag
    resOffset = 0;

    for( i = 0; i < resNum; i++, tmpBuf += 2)                                       // get resource offset
    {
        if (*tmpBuf == res_type)
            break;

        resOffset += *(tmpBuf + 1);
    }

    resStarAddr += resOffset;
    ext_mem_free(buf);

    if (i == resNum)
    {
        MP_ALERT("--E-- Can't find %c%c%c%c", res_type >> 24, res_type >> 16, res_type >> 8, res_type);

        return NULL;
    }

    if (Fseek(handle, resStarAddr, SEEK_SET) != FS_SUCCEED)
    {
        MP_ALERT("--E-- %s: Seek to %u error !!!", __FUNCTION__, resStarAddr);

        return NULL;
    }

    if (FileRead(handle, pbTarget, dwSize) != dwSize)
    {
        MP_ALERT("--E-- %s: Read resource fail !!!", __FUNCTION__);

        return NULL;
    }

    return pbTarget;
}



///
///@ingroup MAIN
///@brief   Get the size of specific resource
///
///@param   DWORD dwTag  the tag of resource to find
///
///@retval  If the resource found, return the size of the resource, else return 0.
///
DWORD IspFunc_GetRESOURCESize(DWORD dwTag)
{
    STREAM *handle;
    DWORD i;
    DWORD resNum;
    DWORD resSize = 0;
    DWORD* buf;
    DWORD* tmpBuf;

    MP_DEBUG("%s -", __FUNCTION__);

    handle = ispResourceFileOpen();

    if (!handle)
    {
        MP_ALERT("--E-- %s: open resource file fail !!!", __FUNCTION__);
        return 0;
    }

    Fseek(handle, 0, SEEK_SET);
    buf = (DWORD *) ext_mem_malloc(RES_STREAM_BUFFER_SIZE);

    if (!buf)
    {
        MP_ALERT("--E-- %s: Can't allocate memory !!!", __FUNCTION__);

        return 0;
    }

    tmpBuf = (DWORD *) ((DWORD) buf | BIT29);

    if (FileRead(handle, (BYTE *) tmpBuf, RES_STREAM_BUFFER_SIZE) == 0)
    {
        MP_ALERT("--E-- %s: Can't read RES_TAG", __FUNCTION__);

        return 0;
    }

    resNum = *(tmpBuf + 9);
    tmpBuf += 10;                                                       // offset to first resource tag

    for (i = 0 ; i < resNum ; i++, tmpBuf += 2)                         // get resource offset
    {
        if (*tmpBuf == dwTag)
        {
            resSize = *(tmpBuf + 1);
            break;
        }
    }

    ext_mem_free((void*) buf);

    if (i == resNum)
    {
        MP_ALERT("--E-- %s: Can't find %c%c%c%c", __FUNCTION__, dwTag >> 24, dwTag >> 16, dwTag >> 8, dwTag);

        return 0;
    }

    MP_DEBUG("Tag-%c%c%c%c size is %u", dwTag >> 24, dwTag >> 16, dwTag >> 8, dwTag, resSize);

    return resSize;
}



int IspFunc_WriteUST(BYTE *pdwTempMen, DWORD buf_size)
{
    MP_DEBUG("%s -", __FUNCTION__);
    return IspFunc_Write(pdwTempMen, buf_size, USER_SET_TAG);
}



DWORD IspFunc_ReadUST(BYTE *pdwTempMen, DWORD buf_size)
{
    MP_DEBUG("%s -", __FUNCTION__);
    return IspFunc_Read(pdwTempMen, buf_size, USER_SET_TAG);
}



int IspFunc_WriteFST(BYTE *pdwTempMen, DWORD buf_size)
{
    MP_DEBUG("%s -", __FUNCTION__);
    return IspFunc_Write(pdwTempMen, buf_size, FACTORY_SET_TAG);
}



DWORD IspFunc_ReadFST(BYTE *pdwTempMen, DWORD buf_size)
{
    MP_DEBUG("%s -", __FUNCTION__);
    return IspFunc_Read(pdwTempMen, buf_size, FACTORY_SET_TAG);
}

#endif

