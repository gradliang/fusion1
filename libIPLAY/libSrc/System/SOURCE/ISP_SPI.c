/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"

#if (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_SPI) && (CHIP_VER_MSB != CHIP_VER_615))

#include "devio.h"
#include "mcardapi.h"

#define RES_TAG_SIZE                0x20
#define SPI_4MB_OFFSET              0x400000            // offset 4MB due to hardware limitation

/*********************** RESERVED 64KB FOR BOOT CODE AND SYSTEM SETTING *******************************/
#define SPI_SETTAB_OFFSET           0x8000              // size = 0x8000 ~ 0xDFFF bytes
#define SPI_UST_OFFSET              0xE000              // size = 0xE000 ~ 0xEFFF bytes
#define SPI_FST_OFFSET              0xF000              // size = 0xF000 ~ 0xFFFF bytes
/******************************************************************************************************/

DWORD IspFunc_Read(BYTE* buffer, DWORD len , DWORD tag)
{
    DWORD tagAddr;
    DWORD spiSz = spi_get_size();
    MP_DEBUG("-spi size = 0x%08X", spiSz);

    if(spiSz > 0x400000)
    {
        switch(tag)
        {
            case AP_TAG:
                tagAddr = SPI_APCODE_SADDR | SPI_4MB_OFFSET;
            break;
            case RES_TAG:
                tagAddr = SPI_RESOCE_SADDR | SPI_4MB_OFFSET;
            break;
            case FACTORY_SET_TAG:
                tagAddr = SPI_FST_OFFSET | SPI_4MB_OFFSET;
            break;
            case USER_SET_TAG:
                tagAddr = SPI_UST_OFFSET | SPI_4MB_OFFSET;
            break;
            default:
                MP_ALERT("Wrong tag");
                return 1;
            break;
        }
    }
    else
    {
        switch(tag)
        {
            case AP_TAG:
                tagAddr = SPI_APCODE_SADDR;
            break;
            case RES_TAG:
                tagAddr = SPI_RESOCE_SADDR;
            break;
            case FACTORY_SET_TAG:
                tagAddr = SPI_FST_OFFSET;
            break;
            case USER_SET_TAG:
                tagAddr = SPI_UST_OFFSET;
            break;
            default:
                MP_ALERT("Wrong tag");
                return 1;
            break;
        }

    }

    if(IspRead(DEV_SPI_FLASH_ISP, tagAddr, buffer, len))
    {
        MP_ALERT("IspFunc_Read fail");
        return 1;
    }

    return 0;

}



int IspFunc_Write(BYTE *data, DWORD len, DWORD tag)
{
    DWORD i;
    DWORD tagAddr;
    DWORD spiSz;

    switch(tag)
    {
    case AP_TAG:
        if(len > (SPI_RESOCE_SADDR - SPI_APCODE_SADDR))
        {
            MP_ALERT("--E-- %s: AP Code sizes are too big!!!", __FUNCTION__);

            return FAIL;
        }
        break;

    case RES_TAG:
        if(len > SPI_RESERVED_SIZE - SPI_RESOCE_SADDR)
        {
            MP_ALERT("--E-- Resource sizes are too big!!!");

            return FAIL;
        }
        break;

    case USER_SET_TAG:
    case FACTORY_SET_TAG:
        if(len > 0x1000)
        {
            MP_ALERT("--E-- UST/FST sizes are too big!!!");

            return FAIL;
        }
        break;

    default:
        MP_ALERT("--E-- %s: tag = %x : Wrong tag name!", __FUNCTION__, tag);

        return FAIL;
        break;
    }

    spiSz = spi_get_size();
    MP_DEBUG("spi size = 0x%08X", spiSz);

    if (spiSz > 0x400000)
    {
        switch (tag)
        {
        case AP_TAG:
            tagAddr = SPI_APCODE_SADDR | SPI_4MB_OFFSET;
            break;

        case RES_TAG:
            tagAddr = SPI_RESOCE_SADDR | SPI_4MB_OFFSET;
            break;

        case FACTORY_SET_TAG:
            tagAddr = SPI_FST_OFFSET | SPI_4MB_OFFSET;
            break;

        case USER_SET_TAG:
            tagAddr = SPI_UST_OFFSET | SPI_4MB_OFFSET;
            break;

        default:
            break;
        }
    }
    else
    {
        switch(tag)
        {
        case AP_TAG:
            tagAddr = SPI_APCODE_SADDR;
            break;

        case RES_TAG:
            tagAddr = SPI_RESOCE_SADDR;
            break;

        case FACTORY_SET_TAG:
            tagAddr = SPI_FST_OFFSET;
            break;

        case USER_SET_TAG:
            tagAddr = SPI_UST_OFFSET;
            break;

        default:
            break;
        }
    }

    if(spiSz - tagAddr < len)
    {
        MP_ALERT("SPI size is too small!!");
        MP_ALERT("--E-- SPI size is too small!!");

        return FALSE;
    }

    if (IspWrite(DEV_SPI_FLASH_ISP, tagAddr, data, len))
    {
        MP_ALERT("--E-- %s Fail", __FUNCTION__);
        return FALSE;
    }

    return PASS;
}



DWORD IspFunc_ReadAP(BYTE *buf, DWORD size)
{
    return IspFunc_Read(buf, size, AP_TAG);
}

#define   RESOURCE_HEAD_LENTH				512
DWORD IspFunc_ReadRES(BYTE *buf, DWORD size)
{
    return IspFunc_Read(buf, size, RES_TAG);
}

BYTE* IspFunc_ReadRESOURCE(DWORD res_type, BYTE *pbTarget, DWORD dwSize)
{
    DWORD i;
    DWORD resStarAddr;
    DWORD resNum;
    DWORD resSize;
    DWORD resOffset;
    DWORD *tmpBuf;
    DWORD *buf;

    buf = (DWORD *) ext_mem_malloc(RESOURCE_HEAD_LENTH);                     // assume 256Bytes can include all resource tags.

    tmpBuf = buf;
    if (!tmpBuf)
    {
        MP_ALERT("\r\nCan't allocate memory for SPI get resource size buffer");
        __asm("break 100");

        return pbTarget;
    }

    tmpBuf = (DWORD *) ((DWORD) tmpBuf | 0x20000000);
    if(IspFunc_Read((BYTE *) tmpBuf, RESOURCE_HEAD_LENTH, RES_TAG))
    {
        MP_ALERT("Can't read RES_TAG");
        return 0;
    }

    resNum = *(tmpBuf + 9);
    if(resNum == 0xFFFFFFFF)
    {
        MP_ALERT("Wrong resource number");
        return 0;
    }
    resStarAddr = SPI_RESOCE_SADDR + RES_TAG_SIZE + ((resNum + 1) << 3) ;           // resource data start address
    tmpBuf += 10;                                                                   // offset to first resource tag
    resOffset = 0;

    for( i = 0; i < resNum; i++, tmpBuf += 2)                                       // get resource offset
    {
        if (*tmpBuf == res_type)
            break;

        resOffset += *(tmpBuf + 1);
    }

    resStarAddr += resOffset;
    ext_mem_free(buf);

    if(i == resNum)
    {
        MP_ALERT("Can't find %c%c%c%c", res_type >> 24, res_type >> 16, res_type >> 8, res_type);
        return FALSE;
    }


    if (IspRead(DEV_SPI_FLASH_ISP, resStarAddr, pbTarget, dwSize))
    {
        MP_ALERT("IspFunc_ReadRESOURCE fail!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        //return FALSE;
    }

    return pbTarget;    //???
}



/*
Resource header : MPRS(4 bytes)                     + StarAddr(4 bytes) + {ResTotalSize + 16Bytes}(4 bytes)     + Version(4 bytes)  +
                  CkSum?(4 bytes)                   + Reserve0(4 bytes) + Reserve1(4 bytes)                     + Reserve2(4 bytes) +
                  {ResTotalSize + 8 * (ResNum + 1)}(4 bytes) + ResNum(4 bytes)   + Res1Tag(4 bytes)                      + Res1Size(4 bytes)  +
                  Res2Tag(4 bytes) + Res2Size(4 bytes) + .....................................

*/
DWORD IspFunc_GetRESOURCESize(DWORD dwTag)
{
    DWORD i;
    DWORD resNum;
    DWORD resSize = 0;
    DWORD* buf;
    DWORD* tmpBuf;

    buf = (DWORD*) ext_mem_malloc(RESOURCE_HEAD_LENTH);                             // assume 256Bytes can include all resource tags.
    tmpBuf = buf;
    if(!tmpBuf)
    {
        MP_ALERT("Can't allocate memory for SPI get resource size buffer");
        __asm("break 100");
    }

    tmpBuf = (DWORD *) ((DWORD) tmpBuf | BIT29);
    IspFunc_Read((BYTE*)tmpBuf, RESOURCE_HEAD_LENTH, RES_TAG);

    resNum = *(tmpBuf + 9);
    tmpBuf += 10;                                                       // offset to first resource tag

    for(i = 0 ; i < resNum ; i++ , tmpBuf += 2)                         // get resource offset
    {
        if(*tmpBuf == dwTag)
        {
            resSize = *(tmpBuf + 1);
            break;
        }
    }
    ext_mem_free(buf);

    if(i == resNum)
    {
        MP_ALERT("Can't find %c%c%c%c", dwTag >> 24, dwTag >> 16, dwTag >> 8, dwTag);
        return FALSE;
    }

    return resSize;

}



int IspFunc_WriteUST(BYTE *pdwTempMen, DWORD buf_size)
{
    return IspFunc_Write(pdwTempMen, buf_size, USER_SET_TAG);
}



DWORD IspFunc_ReadUST(BYTE *pdwTempMen, DWORD buf_size)
{
    return IspFunc_Read(pdwTempMen, buf_size, USER_SET_TAG);
}



int IspFunc_WriteFST(BYTE *pdwTempMen, DWORD buf_size)
{
    return IspFunc_Write(pdwTempMen, buf_size, FACTORY_SET_TAG);
}



DWORD IspFunc_ReadFST(BYTE *pdwTempMen, DWORD buf_size)
{
    return IspFunc_Read(pdwTempMen, buf_size, FACTORY_SET_TAG);
}

#endif	// #if (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_SPI))

