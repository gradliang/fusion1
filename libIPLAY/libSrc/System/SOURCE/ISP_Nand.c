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

#ifndef NAND_RESERVED_SIZE
#define NAND_RESERVED_SIZE          (10 * 1024 * 1024)
#endif

#if (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_NAND))

static void ispNandInit(void);
static DWORD NandIspGetBlockGap(DWORD dwTag);

#define ISP_TO_ORIGINAL_FIRST   0
#define DRV_ACCESS_SIZE         512

typedef struct{
    DWORD dwTag;                // Tag of block
    DWORD dwAddr;               // The start BYTE address of block
} NAND_ISP_AREA;

#define MAX_AREA_NUM           16
NAND_ISP_AREA IspAreaArray[MAX_AREA_NUM];

static DWORD SecondApAddr, SecondResAddr;

static DWORD nandReservedRegionSize = NAND_RESERVED_SIZE;
DWORD ISP_GetNandReservedSize(void)
{
    MP_DEBUG("%s: nandReservedRegionSize = 0x%08X(%dMB)", __func__, nandReservedRegionSize, nandReservedRegionSize/1024/1024);
    return nandReservedRegionSize;
}


void ISP_SetNandReservedSize(DWORD revSize)
{
    MP_DEBUG("%s: revSize = 0x%08X(%dMB)", __func__, revSize, revSize/1024/1024);
    nandReservedRegionSize = revSize;
}


void checkNandIspRegion(DWORD tag)
{
    MP_DEBUG("%s: tag = 0x%X(%c%c%c%c)", __func__, tag, tag >> 24, tag >> 16,tag >> 8, tag);
    int apCodePart1, apCodePart2;

    apCodePart1 = NandCodeCheck(tag, 0);
    apCodePart2 = NandCodeCheck(tag, 1);
    MP_DEBUG("%s: apCodePart1 = %d, apCodePart2 = %d", __func__, apCodePart1, apCodePart2);
    
    if ((apCodePart1 == FAIL) || (apCodePart2 == FAIL))
    {
        if ((apCodePart1 == FAIL) && (apCodePart2 == PASS))
        {
            MP_ALERT("--E-- 1st %c%c%c%c code is bad !!!\r\nRecorver 1st %c%c%c%c from 2nd %c%c%c%c !!!",
                     tag >> 24, tag >> 16,tag >> 8, tag,
                     tag >> 24, tag >> 16,tag >> 8, tag,
                     tag >> 24, tag >> 16,tag >> 8, tag);
            NandCodeBackup(tag, 1);
        }
        else if ((apCodePart1 == PASS) && (apCodePart2 == FAIL))
        {
            MP_ALERT("--E-- 2nd %c%c%c%c is bad !!!\r\nRecorver 2nd %c%c%c%c  from 1st %c%c%c%c !!!",
                     tag >> 24, tag >> 16,tag >> 8, tag,
                     tag >> 24, tag >> 16,tag >> 8, tag,
                     tag >> 24, tag >> 16,tag >> 8, tag);
            NandCodeBackup(tag, 0);
        }
        else //if ((apCodePart1 == FAIL) && (apCodePart2 == FAIL))
        {
            MP_ALERT("--E-- No %c%c%c%c infomation in NAND !!!", tag >> 24, tag >> 16,tag >> 8, tag);
        }
    }
    else
    {
        MP_ALERT("NAND ISP of %c%c%c%c is fine .", tag >> 24, tag >> 16,tag >> 8, tag);
    }
    MP_DEBUG("%s ---> Exit", __func__);
}


void xpgDumpNandIspBlock()
{
    MP_DEBUG("%s: SecondApAddr = 0x%X, SecondResAddr = 0x%08X", __func__, SecondApAddr, SecondResAddr);
    DWORD i;
    for(i=0; i<MAX_AREA_NUM; i++)
    {
        if(IspAreaArray[i].dwAddr > 0)
            MP_DEBUG("   - IspAreaArray[%02d].dwTag = 0x%08X, IspAreaArray[%02d].dwAddr = 0x%08X", i, IspAreaArray[i].dwTag, i, IspAreaArray[i].dwAddr);
    }
}


// AP code start address is fixed at block 1 (block 0 is reserved for bootup ?)
void ISP_NandInit(DWORD apCodeAreaSize)
{
    MP_DEBUG("%s: apCodeAreaSize = 0x%08X(%dMB)", __func__, apCodeAreaSize, apCodeAreaSize/1024/1024);

    ispNandInit();
    //xpgDumpNandIspBlock(); // verify

    ISP_NandBlockRegister(RES_TAG, apCodeAreaSize);
    //xpgDumpNandIspBlock(); // verify
    
    MP_DEBUG("%s ---> Exit", __func__);
}

//------------------------------------------------------------------------------
// UserBlock related function - don't know for what use ?!
/*
DWORD ISP_GetUserBlockSize(DWORD dwTag)
{
    MP_DEBUG("%s: dwTag = 0x%X", __func__, dwTag);
    return NandIspGetBlockGap(dwTag);
}


DWORD ISP_GetUserBlock(BYTE *buf, DWORD size, DWORD tag)
{
    MP_DEBUG("%s: size = %d, tag = 0x%X", __func__, size, tag);
    return IspFunc_Read(buf, size, tag);
}


int ISP_UpdateUserBlock(BYTE *buf, DWORD size, DWORD tag)
{
    MP_DEBUG("%s: size = %d, tag = 0x%X", __func__, size, tag);
    return IspFunc_Write(buf, size, tag);
}
*/
//------------------------------------------------------------------------------


static void ispNandInit(void)
{
    MP_DEBUG("%s", __func__);
    DWORD i;

    for(i=0; i<MAX_AREA_NUM; i++)
    {
        IspAreaArray[i].dwTag = NULL;
        IspAreaArray[i].dwAddr = 0;
    }

    SecondApAddr = 0;
    SecondResAddr = 0;
}


// Register one block to code area. This block is not same as NAND capacity's Block.
// If the block has been registered, it will not change the original setting and return fail.
int ISP_NandBlockRegister(DWORD dwTag, DWORD dwAddr)
{
    MP_DEBUG("%s: Register one block area to code(0x%X).", __func__, dwTag);
    MP_DEBUG("%s: The area from Block1_or_Sector1024_or_512KB to dwAddr(0x%08X)(%dMB).", __func__, dwAddr, dwAddr/1024/1024);
    //MP_DEBUG("%s: dwTag = 0x%X, dwAddr = 0x%08X", __func__, dwTag, dwAddr);
    DWORD i;

    if(dwAddr >= nandReservedRegionSize)
    {
        MP_ALERT("--E-- %s fail address(0x%X) exceed ISP max region(0x%X)! Stop! Need to check!", __func__, dwAddr, nandReservedRegionSize);
        while(1);
    }

    IntDisable();

    for(i=0; i<MAX_AREA_NUM; i++)
    {
        if(IspAreaArray[i].dwTag == dwTag)
        {
            MP_DEBUG("i = %d, IspAreaArray[%d].dwTag had found, stop register Block!", i, i); 
            IntEnable();
            return FAIL;
        }
    }

    for(i=0; i<MAX_AREA_NUM; i++)
    {
        if(IspAreaArray[i].dwTag == NULL)
        {
            MP_DEBUG("%s: IspAreaArray[%d].dwTag == NULL, Register Block!", __func__, i);  
            IspAreaArray[i].dwAddr = (dwAddr & 0x0fffffff);
            IspAreaArray[i].dwTag = dwTag;
            MP_DEBUG("%s: IspAreaArray[%d].dwTag = 0x%08X, IspAreaArray[%d].dwAddr = 0x%08X", __func__, i, IspAreaArray[i].dwTag, i, IspAreaArray[i].dwAddr); 
            IntEnable();
            return PASS;
        }
    }

    IntEnable();
    return FAIL;
}


// Unregister one block from code area.
// Please note that, the function call do not clear the block data, you need to clear the block data
// before your unregister.
void UnRegisterNandIspBlock(DWORD dwTag)
{
    MP_DEBUG("%s: dwTag = 0x%X", __func__, dwTag);
    DWORD i;

    IntDisable();

    for(i=0; i<MAX_AREA_NUM; i++)
    {
        if(IspAreaArray[i].dwTag == dwTag)
        {
            IspAreaArray[i].dwAddr = 0;
            IspAreaArray[i].dwTag = NULL;
        }
    }

    IntEnable();
}


// Get the start address of specific block by the block tag.
DWORD NandIspGetStartAddr(DWORD dwTag)
{
    MP_DEBUG("%s: dwTag = 0x%X", __func__, dwTag);
    DWORD i;
    for(i=0; i<MAX_AREA_NUM; i++)
    {
        if(IspAreaArray[i].dwTag == dwTag)
            return IspAreaArray[i].dwAddr;
    }

    return NULL;
}


// Get the distance from the specific block to next block
static DWORD NandIspGetBlockGap(DWORD dwTag)
{
    //MP_DEBUG("%s: dwTag = 0x%X", __func__, dwTag);
    DWORD dwStartAddr, dwNextAddr, i;
    DWORD SectorNumberPerBlock;

    IspGetInfo(DEV_NAND_ISP, &SectorNumberPerBlock, 0, 0);
    //MP_DEBUG("%s: SectorNumberPerBlock = %d", __func__, SectorNumberPerBlock);
    dwStartAddr = 0;
    dwNextAddr = nandReservedRegionSize;
    //MP_DEBUG("%s: dwNextAddr = NAND_RESERVED_SIZE = 0x%08X(%d)", __func__, dwNextAddr, dwNextAddr);
    
    if(dwTag == AP_TAG)
    {
        dwStartAddr = SectorNumberPerBlock * DRV_ACCESS_SIZE;
        //MP_DEBUG("%s dwTag == AP_TAG: dwStartAddr = 0x%08X(%d)", __func__, dwStartAddr, dwStartAddr);
    }
    else
    {
        for(i=0; i<MAX_AREA_NUM; i++)
        {
            //MP_DEBUG("for each block, IspAreaArray[%d].dwTag = 0x%X", i, IspAreaArray[i].dwTag);
            if(IspAreaArray[i].dwTag == dwTag)
            {
                dwStartAddr = IspAreaArray[i].dwAddr;
                //MP_DEBUG("Tag Match!i = %d, IspAreaArray[%d].dwTag = 0x%08X, IspAreaArray[%d].dwAddr = 0x%08X", i, i, IspAreaArray[i].dwTag, i, IspAreaArray[i].dwAddr);
            }
        }
        //MP_DEBUG("%s dwTag isn't AP_TAG: dwStartAddr = 0x%08X(%d)", __func__, dwStartAddr, dwStartAddr);
    }
    
    if(dwStartAddr == 0)
    {
        MP_ALERT("not registered tag! return 0!");
        return 0;
    }

    // Find the tag's Gap
    for(i=0; i<MAX_AREA_NUM; i++)
    {
        if(IspAreaArray[i].dwTag != NULL)
        {
            //MP_DEBUG("%s: IspAreaArray[%d].dwTag = 0x%X != NULL", __func__, i, IspAreaArray[i].dwTag);
            if((IspAreaArray[i].dwAddr > dwStartAddr) && (IspAreaArray[i].dwAddr < dwNextAddr))
            {
                dwNextAddr = IspAreaArray[i].dwAddr;
                //MP_DEBUG("%s: re-config dwNextAddr = 0x%08X(%d)", __func__, dwNextAddr, dwNextAddr);
            }
        }
    }

    //MP_DEBUG("%s ---> Exit, return tagGap = dwNextAddr - dwStartAddr = 0x%08X(%d)", __func__, dwNextAddr - dwStartAddr, dwNextAddr - dwStartAddr);
    return dwNextAddr - dwStartAddr;
}


static SDWORD NandBlockFind(DWORD tag)
{
    //MP_DEBUG("%s: tag = 0x%X", __func__, tag);
    DWORD SectorNumberPerBlock, i;
    DWORD IspLimitSector;
    DWORD CurSector;
    DWORD *buf;

    IspGetInfo(DEV_NAND_ISP, &SectorNumberPerBlock, 0, 0);
    //MP_DEBUG("%s: IspGetInfo() get SectorNumberPerBlock = 0x%X(%d)", __func__, SectorNumberPerBlock, SectorNumberPerBlock);
    CurSector = SectorNumberPerBlock;
    IspLimitSector = nandReservedRegionSize / DRV_ACCESS_SIZE; // DRV_ACCESS_SIZE = 512
    //MP_DEBUG("%s: nandReservedRegionSize = 0x%08X(%dMB), IspLimitSector = 0x%08X(%d)", __func__, nandReservedRegionSize, nandReservedRegionSize/1024/1024, IspLimitSector, IspLimitSector);

    if (tag == AP_TAG)
    {
        CurSector = SectorNumberPerBlock;   // Alway reserved block 0
    }
    else
    {
        CurSector = -1;

        for(i=0; i<MAX_AREA_NUM; i++)
        {
            if(IspAreaArray[i].dwTag == tag)
                CurSector = IspAreaArray[i].dwAddr/DRV_ACCESS_SIZE;
        }
    }
    //MP_DEBUG("%s: CurSector = %d", __func__, CurSector);
    
    if (CurSector != -1)    // truncate to meet Block align
    {
        CurSector = ((CurSector + SectorNumberPerBlock - 1) / SectorNumberPerBlock) * SectorNumberPerBlock;
        //MP_DEBUG("%s: CurSector != -1, do truncate. CurSector = %d", __func__, CurSector);
    }

    if (CurSector >= IspLimitSector)
    {
        //MP_ALERT("%c%c%c%c CurSector %d is IspLimitSector %d!", tag>>24, (tag>>16)&0xff, (tag>>8)&0xff, tag&0xff, CurSector, IspLimitSector);
        CurSector = -1;
    }

    //MP_DEBUG("%s ---> Exit", __func__);
    
    return CurSector;
}


static SDWORD NandBlankBlockFind()
{
    MP_DEBUG("%s", __func__);
    DWORD SectorNumberPerBlock, sectNrPerPage, pageSize;
    DWORD IspLimit;
    DWORD CurSector;
    DWORD *buf;
    DWORD i, j;
    int ret = -1;

    IspGetInfo(DEV_NAND_ISP, &SectorNumberPerBlock, 0, &pageSize);
    sectNrPerPage = pageSize / DRV_ACCESS_SIZE;
    IspLimit = NandIspGetStartAddr(RES_TAG) / DRV_ACCESS_SIZE;
    CurSector = nandReservedRegionSize / DRV_ACCESS_SIZE;
    buf = (DWORD *) ext_mem_malloc(pageSize);

    if (!buf)
    {
        MP_ALERT("%s - malloc fail", __FUNCTION__);
        return FAIL;
    }

    buf = (DWORD *) ((DWORD) buf | BIT29);

    while ((CurSector > IspLimit) && (ret == -1))
    {
        if (!IspBlockIsBad(DEV_NAND_ISP, CurSector))
        {
            for (i = 0 ; i < SectorNumberPerBlock ; i += sectNrPerPage)
            {
                IspRead(DEV_NAND_ISP, CurSector + i, (BYTE *) buf, sectNrPerPage);

                for (j = 0 ; j < (pageSize >> 2) ; j++)
                {
                    if (buf[j] != 0xffffffff)   // not blank sector
                        break;
                }

                if (j < (pageSize >> 2))
                    break;
            }

            if (i >= SectorNumberPerBlock)
                ret = CurSector;
        }

        CurSector -= SectorNumberPerBlock;
    }

    ext_mem_free(buf);

    return ret;
}


DWORD NandIspRealRead(BYTE *buf, DWORD size, DWORD StartAddr)
{
    //MP_DEBUG("%s: size = %d, StartAddr = 0x%X", __func__, size, StartAddr);
    DWORD SectorNumberPerBlock, sectNrPerPage, pageSize;
    DWORD i = 0, j;
    BYTE *tmp;

    IspGetInfo(DEV_NAND_ISP, &SectorNumberPerBlock, 0, &pageSize);
    sectNrPerPage = pageSize / DRV_ACCESS_SIZE;
    //MP_DEBUG("%s: read from %d, size=%d", __FUNCTION__, StartAddr, size);

    tmp = (BYTE *) ext_mem_malloc(pageSize);

    if (!tmp)
    {
        MP_ALERT("%s - malloc fail", __FUNCTION__);
        return 0;
    }

    tmp = (BYTE *) ((DWORD) tmp | BIT29);

    while (i < size)
    {
        if (!IspBlockIsBad(DEV_NAND_ISP, StartAddr))
        {
            MP_DEBUG("sector %d", StartAddr);

            for (j = 0 ; j < SectorNumberPerBlock; j += sectNrPerPage)
            {
                if ((i + pageSize) > size)
                {
                    int k = size - i;

                    IspRead(DEV_NAND_ISP, StartAddr + j, tmp, sectNrPerPage);
                    memcpy((BYTE *) &buf[i], tmp, k);
                    i += k;

                    break;
                }
                else
                {
                    IspRead(DEV_NAND_ISP, StartAddr + j, &buf[i], sectNrPerPage);
                    i += pageSize;
                }
            }
        }

        StartAddr += SectorNumberPerBlock;
    }

    ext_mem_free(tmp);

    return i;
}


static DWORD NandIspRead(BYTE *buf, DWORD size, DWORD tag)
{
    //MP_DEBUG("%s: size = %d, tag = 0x%X", __func__, size, tag);
    SDWORD ret;

    ret = NandBlockFind(tag);

    if (ret == -1)
    {
        MP_ALERT("%s(tag=%x): not found!", __FUNCTION__, tag);
        return 0;
    }
    else
        return NandIspRealRead(buf, size, (DWORD)ret);
}


static DWORD NandIspRealWrite(BYTE *src, DWORD size, DWORD StartSec, DWORD EndSec)
{
    MP_DEBUG("%s: size = %d, StartSec = 0x%08X(%d), EndSec = 0x%08X(%d)", __func__, size, StartSec, StartSec, EndSec, EndSec);
    DWORD SectorNumberPerBlock;
    DWORD i = 0, j, k, VerifySize;
    BYTE *verifyBuf, *srcBuf;

    IspGetInfo(DEV_NAND_ISP, &SectorNumberPerBlock, 0, 0);
    DWORD blksz = DRV_ACCESS_SIZE * SectorNumberPerBlock;
    BYTE *tmp = (BYTE *) ext_mem_malloc(blksz << 1);

    if (!tmp)
    {
        MP_ALERT("%s - malloc fail", __FUNCTION__);
        return 0;
    }

    tmp = (BYTE *) ((DWORD) tmp | BIT29);
    verifyBuf = (BYTE *) (tmp + blksz);

    MP_DEBUG("%s: write to StartSec(0x%08X), size=%u", __FUNCTION__, StartSec, size);

    while (i < size)
    {
        //MP_DEBUG("while i(%d) < size", i);
        if (!IspBlockIsBad(DEV_NAND_ISP, StartSec))     // is this block available?
        {
            if ((i + blksz) > size)     // size of data to write is less than a block size
            {
                //for (j = 0 ; j < SectorNumberPerBlock ; j++) // read whole block
                //    IspRead(DEV_NAND_ISP, StartSecAddr + j, &tmp[DRV_ACCESS_SIZE * j], 1);
                IspRead(DEV_NAND_ISP, StartSec, tmp, SectorNumberPerBlock);
                memcpy(tmp, &src[i], size - i);
                srcBuf = tmp;
            }
            else
                srcBuf = &(src[i]);

            if (IspEraseBlk(DEV_NAND_ISP, StartSec, 1) == PASS) // erase it
            {
                //MP_DEBUG("sector %d", StartSec);

                if ((i + blksz) > size)
                {
                    IspWrite(DEV_NAND_ISP, StartSec, tmp, 1);
                    VerifySize = size - i;
                }
                else
                {
                    IspWrite(DEV_NAND_ISP, StartSec, &src[i], 1);
                    VerifySize = blksz;
                }

                // verify
                //for (j = 0 ; j < SectorNumberPerBlock ; j++) // read whole block
                //    IspRead(DEV_NAND_ISP, StartSecAddr + j, &verifyBuf[DRV_ACCESS_SIZE * j], 1);
                IspRead(DEV_NAND_ISP, StartSec, verifyBuf, SectorNumberPerBlock);

                for (k = 0; k < VerifySize; k++)
                {
                    // If verify Error, erase the block (bad block)
                    if (verifyBuf[k] != srcBuf[k])
                    {
                        IspEraseBlk(DEV_NAND_ISP, StartSec, 1);
                        break;
                    }
                }

                // Verify OK
                if (k == VerifySize)
                    i += blksz;
            }
        }
        else
            MP_ALERT("ISP Bad Block at sector %d", StartSec);

        StartSec += SectorNumberPerBlock;
        //MP_DEBUG("StartSec += SectorNumberPerBlock, StartSec = 0x%08X(%d), i = %d", StartSec, StartSec, i);
        
        // Check if over range
        if (StartSec >= EndSec && i < size)
        {
            MP_ALERT("--E-- %s: over range - StartSec(0x%08X or %d) > EndSec(0x%08X or %d)", __func__, StartSec, StartSec, EndSec, EndSec);
            i = 0;
            break;
        }
    }

    ext_mem_free(tmp);

    return i;
}


static SDWORD NandIspWrite(BYTE *src, DWORD size, DWORD tag)
{
    //MP_DEBUG("%s: size = %d, tag = 0x%X", __func__, size, tag);
    DWORD startSector, endSector;
    SDWORD ret;

    ret = NandBlockFind(tag);
    MP_DEBUG("%s: after NandBlockFind(tag), startSector = 0x%08X(%d)", __func__, ret, ret);
    
    if (ret == -1)
    {
        if ((tag == USER_SET_TAG) || (tag == FACTORY_SET_TAG))
        {
            ret = NandBlankBlockFind();
            mpDebugPrintN("0x%08X", tag);

            if (ret == -1)
                MP_ALERT("--E-- %s: no place to store!", __FUNCTION__);
            else
                MP_ALERT("--E-- %s: new location is %d", __FUNCTION__, ret);
        }
    }

    if (ret == -1)
    {
        MP_ALERT("%s(tag=%x): not found!", __FUNCTION__, tag);
    }
    else
    {
        startSector = (DWORD) ret;
        endSector = NandIspGetBlockGap(tag);

        if (endSector == 0)
        {
            MP_ALERT("--E-- Not registered tag");
        }
        
        MP_DEBUG("%s(tag=%x): write to startSector = 0x%08X(%d), size=%dKB", __func__, tag, startSector, startSector, size / 1000);

        endSector /= DRV_ACCESS_SIZE;
        endSector += startSector;
        ret = NandIspRealWrite(src, size, startSector, endSector);
    }

    return ret;
}



int IspFunc_Write(BYTE *data, DWORD len, DWORD tag)
{
    //MP_DEBUG("%s: len = %d, tag = 0x%X", __func__, len, tag);
    SDWORD ret = 0;

    DWORD dwGap = NandIspGetBlockGap(tag);
    if (dwGap < len)
    {
        MP_ALERT("--E-- Tag Gap(0x%X or %d) < len(0x%X or %d), ISP update will overlay other block !!", dwGap, dwGap, len, len);
        return FAIL;
    }
    else
    {
    //--------------------------------------
    // prepare Backup MPAP/MPRS Start Sector 
        if (tag == AP_TAG)
            SecondApAddr = 0;
        else
            SecondResAddr = 0;
    //---------------------------------------

        ret = NandIspWrite(data, len, tag);
        //MP_DEBUG("%s: after NandispWrite, ret = 0x%08X(%d)", __func__, ret, ret);
        
        if (ret >= len)
            NandCodeBackup(tag, 0);
    }
    
    //MP_DEBUG("%s ---> Exit", __func__);

    return (ret < len) ? FAIL : PASS;
}


DWORD IspFunc_Read(BYTE *buf, DWORD size, DWORD tag)
{
    MP_DEBUG("%s: size = %d, tag = 0x%X", __func__, size, tag);
    return NandIspRead(buf, size, tag);
}


DWORD IspFunc_ReadAP(BYTE *buf, DWORD size)
{
    MP_DEBUG("%s: size = 0x%X", __func__, size);
    return NandIspRead(buf, size, AP_TAG);
}


DWORD IspFunc_ReadRES(BYTE *buf, DWORD size)
{
    MP_DEBUG("%s: size = 0x%X", __func__, size);
    return NandIspRead(buf, size, RES_TAG);
}


BYTE *IspFunc_ReadMPAP(DWORD dwTag, BYTE *pbTarget, DWORD dwSize)
{
    MP_DEBUG("---------------------------------------------------------------");
    MP_DEBUG("%s: dwTag = 0x%X, dwSize = 0x%X", __func__, dwTag, dwSize);
    DWORD SectAddr;
    DWORD SectorNumberPerBlock, sectNrPerPage, pageSize;
    DWORD i, j;
    DWORD *buffer;
    DWORD *dst = (DWORD *)pbTarget;
    DWORD pos;
    DWORD gap = 0;

    IspGetInfo(DEV_NAND_ISP, &SectorNumberPerBlock, 0, &pageSize);
    sectNrPerPage = pageSize / DRV_ACCESS_SIZE;
    SectAddr = NandBlockFind(AP_TAG);
    MP_DEBUG("%s: NandBlockFind(AP_TAG) return SectAddr = %d", __func__, SectAddr);
    
    buffer = (DWORD *) ext_mem_malloc(pageSize);
    if (!buffer)
    {
        mpDebugPrint("%s - malloc fail", __FUNCTION__);
        return NULL;
    }
    buffer = (DWORD *) ((DWORD) buffer | BIT29);

    while (IspBlockIsBad(DEV_NAND_ISP, SectAddr))
        SectAddr += SectorNumberPerBlock;

    IspRead(DEV_NAND_ISP, SectAddr, (BYTE *) buffer, sectNrPerPage);
    MP_DEBUG("%s: after ispRead(), SectAddr = %d", __func__, SectAddr);

    MP_DEBUG("buffer[0..3] = 0x%08X 0x%08X 0x%08X 0x%08X", buffer[0], buffer[1], buffer[2], buffer[3]);
    MP_DEBUG("buffer[4..7] = 0x%08X 0x%08X 0x%08X 0x%08X", buffer[4], buffer[5], buffer[6], buffer[7]);
    MP_DEBUG("buffer[8.11] = 0x%08X 0x%08X 0x%08X 0x%08X", buffer[8], buffer[9], buffer[10], buffer[11]);
    
    //if (buffer[0] == AP_TAG)
    {
        DWORD nr_res = 1;
        DWORD dwcnt = pageSize >> 2;            // DWORD count
        MP_DEBUG("dwcnt = %d", dwcnt);
        //j = 10;
        //pos = (j << 2) + (nr_res << 3);         // byte count

        //for (i = 0 ; i < nr_res ; i++)          // finding
        {
            //if (buffer[j] == res_type)
            //    break;

            pos += buffer[1];
            j += 2;
        }

        if (buffer[0] == dwTag && buffer[2] == dwSize)    // found
        {
            //if (gap)
            //    SectAddr -= gap;

            MP_DEBUG("read %c%c%c%c from %d, size=%d", dwTag>>24, dwTag>>16, dwTag>>8, dwTag, SectAddr, dwSize);
            j = pos >> 2;
            dwSize >>= 2;   // DWORD size

            while (dwSize--)
            {
                if (j >= dwcnt)    // out of buffer, read next sector from Nandflash
                {
                    SDWORD sectnr, gap;

                    sectnr = (j / dwcnt) * sectNrPerPage;

                    while (sectnr >= 0)
                    {
                        gap = SectAddr % SectorNumberPerBlock;

                        if (gap != 0)
                        {
                            if (sectnr >= SectorNumberPerBlock)
                            {
                                SectAddr += gap;
                                sectnr -= gap;
                            }
                        }

                        while (IspBlockIsBad(DEV_NAND_ISP, SectAddr))
                            SectAddr += SectorNumberPerBlock;

                        if (sectnr >= SectorNumberPerBlock)
                        {
                            SectAddr += SectorNumberPerBlock;
                            sectnr -= SectorNumberPerBlock;
                        }
                        else
                        {
                            break;
                        }
                    }

                    SectAddr += sectnr;
                    j = j % dwcnt;

                    while (IspBlockIsBad(DEV_NAND_ISP, SectAddr))
                        SectAddr += SectorNumberPerBlock;

                    IspRead(DEV_NAND_ISP, SectAddr, (BYTE *) buffer, sectNrPerPage);
                }

                *dst++ = buffer[j++];
            }
        }
        else
        {
            mpDebugPrint("%s() not find %c%c%c%c", __FUNCTION__, dwTag>>24, dwTag>>16, dwTag>>8, dwTag);
            pbTarget = NULL;
        }
    }

    ext_mem_free(buffer);
    MP_DEBUG("---------------------------------------------------------------");
    return pbTarget;
}



BYTE *IspFunc_ReadRESOURCE(DWORD res_type, BYTE *pbTarget, DWORD dwSize)
{
    MP_DEBUG("%s: res_type = %d, dwSize = 0x%X", __func__, res_type, dwSize);
    DWORD SectAddr;
    DWORD SectorNumberPerBlock, sectNrPerPage, pageSize;
    DWORD i, j;
    DWORD *buffer;
    DWORD *dst = (DWORD *)pbTarget;
    DWORD pos;
    DWORD gap = 0;

    IspGetInfo(DEV_NAND_ISP, &SectorNumberPerBlock, 0, &pageSize);
    sectNrPerPage = pageSize / DRV_ACCESS_SIZE;
    SectAddr = NandBlockFind(RES_TAG);
    buffer = (DWORD *) ext_mem_malloc(pageSize);

    if (!buffer)
    {
        mpDebugPrint("%s - malloc fail", __FUNCTION__);

        return NULL;
    }

    buffer = (DWORD *) ((DWORD) buffer | BIT29);

    while (IspBlockIsBad(DEV_NAND_ISP, SectAddr))
        SectAddr += SectorNumberPerBlock;

    //mpDebugPrint("%s: SectAddr = %d", __FUNCTION__, SectAddr);
    IspRead(DEV_NAND_ISP, SectAddr, (BYTE *) buffer, sectNrPerPage);
    //WORD *pw = &buffer[0], *pwStart;
    //pwStart = pw;
    //mpDebugPrint("0x%08X: 0x%04X, 0x%04X, 0x%04X, 0x%04X 0x%04X, 0x%04X, 0x%04X, 0x%04X", pw-pwStart, *pw++, *pw++, *pw++, *pw++, *pw++, *pw++, *pw++, *pw++);
    //mpDebugPrint("0x%08X: 0x%04X, 0x%04X, 0x%04X, 0x%04X 0x%04X, 0x%04X, 0x%04X, 0x%04X", pw-pwStart, *pw++, *pw++, *pw++, *pw++, *pw++, *pw++, *pw++, *pw++);
    
    if (buffer[0] == RES_TAG)
    {
        DWORD nr_res = buffer[9];
        DWORD dwcnt = pageSize >> 2;            // DWORD cout

        j = 10;
        pos = (j << 2) + (nr_res << 3);         // byte count

        for (i = 0 ; i < nr_res ; i++)          // finding
        {
            if (j >= dwcnt)                     // out of buffer, read next page from Nandflash
            {
                //MP_ALERT("j = %d, dwcnt = %d", j, dwcnt);
                gap = (j / dwcnt) * sectNrPerPage;
                SectAddr += gap;
                j = j % dwcnt;

                while (IspBlockIsBad(DEV_NAND_ISP, SectAddr))
                {
                    gap = SectorNumberPerBlock;
                    SectAddr += gap;
                }

                //mpDebugPrint("%s: SectAddr = %d", __FUNCTION__, SectAddr);
                IspRead(DEV_NAND_ISP, SectAddr, (BYTE *) buffer, sectNrPerPage);
                //WORD *pw = &buffer[0], *pwStart;
                //pwStart = pw;
                //mpDebugPrint("0x%08X: 0x%04X, 0x%04X, 0x%04X, 0x%04X 0x%04X, 0x%04X, 0x%04X, 0x%04X", pw-pwStart, *pw++, *pw++, *pw++, *pw++, *pw++, *pw++, *pw++, *pw++);
                //mpDebugPrint("0x%08X: 0x%04X, 0x%04X, 0x%04X, 0x%04X 0x%04X, 0x%04X, 0x%04X, 0x%04X", pw-pwStart, *pw++, *pw++, *pw++, *pw++, *pw++, *pw++, *pw++, *pw++);
                
            }

            if (buffer[j] == res_type)
                break;

            pos += buffer[j + 1];
            j += 2;
        }

        if (buffer[j] == res_type && buffer[j + 1] == dwSize)    // found
        {
            //MP_ALERT("gap = %d, SectAddr = %d", gap, SectAddr);
            if (gap)
            {
                SectAddr -= gap;
                //MP_ALERT("if(gap), SectAddr = %d", SectAddr);
            }

            //MP_ALERT("read %c%c%c%c from (0x%X)%d, size=%d", res_type>>24, res_type>>16, res_type>>8, res_type, SectAddr, pos, dwSize);
            j = pos >> 2;
            //MP_ALERT("pos = %d, j = %d, dwcnt = %d", pos, j, dwcnt);
            dwSize >>= 2;   // DWORD size
            //MP_ALERT("dwSize = %d", dwSize);
            while (dwSize--)
            {
                //MP_ALERT("while dwSize = %d", dwSize);
                if (j >= dwcnt)    // out of buffer, read next sector from Nandflash
                {
                    SDWORD sectnr, gap;

                    sectnr = (j / dwcnt) * sectNrPerPage;
                    //MP_ALERT("j = %d, dwcnt = %d, sectnr = %d", j, dwcnt, sectnr);
                    while (sectnr >= 0)
                    {
                        gap = SectAddr % SectorNumberPerBlock;
                        //MP_ALERT("gap = %d", gap);
                        if (gap != 0)
                        {
                            if (sectnr >= SectorNumberPerBlock)
                            {
                                SectAddr += gap;
                                //MP_ALERT("SectAddr += gap; SectAddr = %d", SectAddr);
                                sectnr -= gap;
                            }
                        }

                        while (IspBlockIsBad(DEV_NAND_ISP, SectAddr))
                            SectAddr += SectorNumberPerBlock;

                        if (sectnr >= SectorNumberPerBlock)
                        {
                            SectAddr += SectorNumberPerBlock;
                            //MP_ALERT("SectAddr += SectorNumberPerBlock; SectAddr = %d", SectAddr);
                            sectnr -= SectorNumberPerBlock;
                        }
                        else
                        {
                            break;
                        }
                    }

                    SectAddr += sectnr;
                    //MP_ALERT("SectAddr += sectnr; SectAddr = %d", SectAddr);
                    j = j % dwcnt;

                    while (IspBlockIsBad(DEV_NAND_ISP, SectAddr))
                        SectAddr += SectorNumberPerBlock;

                    IspRead(DEV_NAND_ISP, SectAddr, (BYTE *) buffer, sectNrPerPage);
                    //WORD *pw = &buffer[0], *pwStart;
                    //pwStart = pw;
                    //mpDebugPrint("0x%08X: 0x%04X, 0x%04X, 0x%04X, 0x%04X 0x%04X, 0x%04X, 0x%04X, 0x%04X", pw-pwStart, *pw++, *pw++, *pw++, *pw++, *pw++, *pw++, *pw++, *pw++);
                    //mpDebugPrint("0x%08X: 0x%04X, 0x%04X, 0x%04X, 0x%04X 0x%04X, 0x%04X, 0x%04X, 0x%04X", pw-pwStart, *pw++, *pw++, *pw++, *pw++, *pw++, *pw++, *pw++, *pw++);
                    
                }

                *dst++ = buffer[j++];
            }
        }
        else
        {
            mpDebugPrint("%s() not find %c%c%c%c", __FUNCTION__, res_type>>24, res_type>>16, res_type>>8, res_type);
            pbTarget = NULL;
        }
    }

    ext_mem_free(buffer);
    
    return pbTarget;
}


DWORD IspFunc_GetMPAPSize(DWORD dwTag)
{
    MP_DEBUG("---------------------------------------------------------------");
    MP_DEBUG("%s: dwTag = 0x%X", __func__, dwTag);
    DWORD SectAddr;
    DWORD SectorNumberPerBlock, sectNrPerPage, pageSize;
    DWORD i = 0, j;
    DWORD *buffer;
    DWORD ret = 0;

    IspGetInfo(DEV_NAND_ISP, &SectorNumberPerBlock, 0, &pageSize);
    MP_DEBUG("%s: pageSize = %d", __func__, pageSize); // NAND pageSize = 4096
    sectNrPerPage = pageSize / DRV_ACCESS_SIZE;
    SectAddr = NandBlockFind(AP_TAG);
    MP_DEBUG("%s: NandBlockFind(AP_TAG) return SectAddr = %d", __func__, SectAddr);
    buffer = (DWORD *) ext_mem_malloc(pageSize);
    if (!buffer)
    {
        mpDebugPrint("%s - malloc fail", __FUNCTION__);
        return 0;
    }
    buffer = (DWORD *) ((DWORD) buffer | BIT29);

    while (IspBlockIsBad(DEV_NAND_ISP, SectAddr))
        SectAddr += SectorNumberPerBlock;

    MP_DEBUG("%s: Check BadBlock, SectAddr = %d", __func__, SectAddr);
    
    IspRead(DEV_NAND_ISP, SectAddr, (BYTE *) buffer, sectNrPerPage);
    SectAddr += sectNrPerPage;
    MP_DEBUG("%s: after ispRead(), SectAddr = %d", __func__, SectAddr);

    MP_DEBUG("buffer[0..3] = 0x%08X 0x%08X 0x%08X 0x%08X", buffer[0], buffer[1], buffer[2], buffer[3]);
    MP_DEBUG("buffer[4..7] = 0x%08X 0x%08X 0x%08X 0x%08X", buffer[4], buffer[5], buffer[6], buffer[7]);
    MP_DEBUG("buffer[8.11] = 0x%08X 0x%08X 0x%08X 0x%08X", buffer[8], buffer[9], buffer[10], buffer[11]);
    if (buffer[0] == dwTag)
    {
        ret = buffer[2];
    }

    ext_mem_free(buffer);
    MP_DEBUG("---------------------------------------------------------------");
    return ret;
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
    MP_DEBUG("%s: dwTag = 0x%X", __func__, dwTag);
    DWORD SectAddr;
    DWORD SectorNumberPerBlock, sectNrPerPage, pageSize;
    DWORD i = 0, j;
    DWORD *buffer;
    DWORD ret = 0;

    IspGetInfo(DEV_NAND_ISP, &SectorNumberPerBlock, 0, &pageSize);
    sectNrPerPage = pageSize / DRV_ACCESS_SIZE;
    SectAddr = NandBlockFind(RES_TAG);
    MP_DEBUG("%s: NandBlockFind(dwTag=0x%X) return SectAddr = %d", __func__, dwTag, SectAddr);
    buffer = (DWORD *) ext_mem_malloc(pageSize);
    if (!buffer)
    {
        mpDebugPrint("%s - malloc fail", __FUNCTION__);
        return 0;
    }

    buffer = (DWORD *) ((DWORD) buffer | BIT29);

    while (IspBlockIsBad(DEV_NAND_ISP, SectAddr))
        SectAddr += SectorNumberPerBlock;

    IspRead(DEV_NAND_ISP, SectAddr, (BYTE *) buffer, sectNrPerPage);
    SectAddr += sectNrPerPage;

    if (buffer[0] == RES_TAG)
    {
        DWORD nr_res = buffer[9];
        MP_DEBUG("%s: nr_res = %d", __func__, nr_res);
        j = 10;

        for (i = 0 ; i < nr_res ; i++)
        {
            if (j >= (pageSize >> 2))       // out of buffer, read next page from Nandflash
            {
                j = 0;

                while (IspBlockIsBad(DEV_NAND_ISP, SectAddr))
                    SectAddr += SectorNumberPerBlock;

                IspRead(DEV_NAND_ISP, SectAddr, (BYTE *) buffer, sectNrPerPage);
                SectAddr += sectNrPerPage;
            }

            if (buffer[j] == dwTag)
            {
                ret = buffer[j+1];
                break;
            }

            j += 2;
        }
    }

    ext_mem_free(buffer);

    return ret;
}   //end of NorGetResourceSize


int IspFunc_WriteUST(BYTE *pdwTempMen, DWORD buf_size)
{
    MP_DEBUG("%s: buf_size = 0x%X", __func__, buf_size);
    int iRet = NandIspWrite((BYTE *)((DWORD)pdwTempMen| 0xA0000000), buf_size, USER_SET_TAG);

    return (iRet - buf_size);
}


DWORD IspFunc_ReadUST(BYTE *pdwTempMen, DWORD buf_size)
{
    MP_DEBUG("%s: buf_size = 0x%X", __func__, buf_size);
    return NandIspRead((BYTE *)((DWORD)pdwTempMen| 0xA0000000), buf_size, USER_SET_TAG);
}


int IspFunc_WriteFST(BYTE *pdwTempMen, DWORD buf_size)
{
    MP_DEBUG("%s: buf_size = 0x%X", __func__, buf_size);
    SDWORD iRet = NandIspWrite((BYTE *) pdwTempMen, buf_size, FACTORY_SET_TAG);

    return (iRet - buf_size);
}


DWORD IspFunc_ReadFST(BYTE *pdwTempMen, DWORD buf_size)
{
    MP_DEBUG("%s: buf_size = 0x%X", __func__, buf_size);
    return NandIspRead((BYTE *)((DWORD)pdwTempMen| 0xA0000000), buf_size, FACTORY_SET_TAG);
}


int NandEraseCodeArea(DWORD ValidOnly)	// this will not erase boot sectors
{
    MP_DEBUG("%s: ValidOnly = 0x%X", __func__, ValidOnly);
    DWORD SecterPerBlk, BlkNr;
    DWORD i;

    mpDebugPrint("Erase Nandflash System Code blocks...");

    //SystemDeviceRawFormat(NAND_ISP, FALSE);
    //SystemDeviceRawFormat(NAND, FALSE);

    IspGetInfo(DEV_NAND_ISP, &SecterPerBlk, &BlkNr, 0);
    BlkNr = nandReservedRegionSize / (SecterPerBlk * DRV_ACCESS_SIZE);
    i = 1;  // block 0 is reserved

    for (; i < BlkNr ; i++)
    {
        if ((i&0x03) == 0)
            mpDebugPrint("");

        mpDebugPrintN("[%08X]:", i);

        if (!ValidOnly || !IspBlockIsBad(DEV_NAND_ISP, i * SecterPerBlk))
        {
            if (IspEraseBlk(DEV_NAND_ISP, i * SecterPerBlk, 1) == PASS)
                mpDebugPrintN("Erased, ");
            else
                mpDebugPrintN("Failed, ");
        }
        else
        {
            mpDebugPrintN("BadBlk, ");
        }
    }

    mpDebugPrint("");

    return PASS;
}



int NandEraseFatArea(DWORD ValidOnly)
{
    MP_DEBUG("%s: ValidOnly = 0x%X", __func__, ValidOnly);
    DWORD SecterPerBlk, BlkNr;
    DWORD i;

    mpDebugPrint("Erase Nandflash file system blocks...");
    IspGetInfo(DEV_NAND_ISP, &SecterPerBlk, &BlkNr, 0);
    i = nandReservedRegionSize / (SecterPerBlk * DRV_ACCESS_SIZE);

    for (; i < BlkNr ; i++)
    {
        if ((i&0x03) == 0)
            mpDebugPrint("");

        mpDebugPrintN("[%08X]:", i);

        if (!ValidOnly || !IspBlockIsBad(DEV_NAND_ISP, i * SecterPerBlk))
        {
            if (IspEraseBlk(DEV_NAND_ISP, i * SecterPerBlk, 1) == PASS)
                mpDebugPrintN("Erased, ");
            else
                mpDebugPrintN("Failed, ");
        }
        else
        {
            mpDebugPrintN("BadBlk, ");
        }
    }

    mpDebugPrint("");

    return PASS;
}



int NandEraseAll(DWORD ValidOnly)
{
    MP_DEBUG("%s: ValidOnly = 0x%X", __func__, ValidOnly);
    NandEraseCodeArea(ValidOnly);
    NandEraseFatArea(ValidOnly);

    return PASS;
}


// Only AP and Resource area contain 2 blocks in one code area
// return value  -1: not registered area, -2: Second block not found
static SDWORD Nand2ndBlockFind(DWORD tag)
{
    //MP_DEBUG("%s: tag = 0x%X", __func__, tag);
    SDWORD startSecAddr, endSecAddr;
    DWORD i;
    DWORD *SecBlkAddr;
    DWORD *buf, FstBlkFound;
    DWORD SectorNumberPerBlock, sectNrPerPage, pageSize;

    if ((tag != AP_TAG) && (tag != RES_TAG))
    {
        MP_ALERT("--E-- %s: Wrong tag", __FUNCTION__);

        return -1;
    }

    startSecAddr = NandBlockFind(tag);
    //MP_DEBUG("%s: startSecAddr = %d", __func__, startSecAddr);
    if (startSecAddr == -1)
    {
        MP_ALERT("--E-- %s: Not registered area ", __FUNCTION__);
        return -1;
    }

    if (tag == AP_TAG)
        SecBlkAddr = &SecondApAddr;
    else
        SecBlkAddr = &SecondResAddr;

    // If second block found, just return the value;
    if (*SecBlkAddr != 0)
        return *SecBlkAddr;

    endSecAddr = startSecAddr + (NandIspGetBlockGap(tag) / DRV_ACCESS_SIZE);
    //MP_DEBUG("%s: endSecAddr = %d", __func__, endSecAddr);
    IspGetInfo(DEV_NAND_ISP, &SectorNumberPerBlock, 0, &pageSize);
    sectNrPerPage = pageSize / DRV_ACCESS_SIZE;

    buf = (DWORD *) ext_mem_malloc(pageSize);

    if (buf == NULL)
    {
        MP_ALERT("--E-- %s: Out of memory !!!", __FUNCTION__);
        return -2;
    }

    buf = (DWORD *) ((DWORD) buf | BIT29);
    FstBlkFound = 0;

    for (i = startSecAddr; i < endSecAddr; i += SectorNumberPerBlock)
    {
        if (!IspBlockIsBad(DEV_NAND_ISP, i))
        {
            IspRead(DEV_NAND_ISP, i, (BYTE *) buf, sectNrPerPage);

            if (buf[0] == tag)
            {
                if (FstBlkFound)
                {
                    ext_mem_free(buf);
                    *SecBlkAddr = i;

                    return i;
                }
                else
                {
                    DWORD offset;

                    offset = (buf[2] + DRV_ACCESS_SIZE - 1) / DRV_ACCESS_SIZE;  // secotr number
                    offset = (offset + SectorNumberPerBlock - 1) / SectorNumberPerBlock;        // block number
                    i += offset * SectorNumberPerBlock;                                 // move to the start of last block of 1st area

                    FstBlkFound = 1;
                }
            }
        }
    }

    ext_mem_free(buf);

    return -2;
}


// Do check sum check of specific code block and index
// checkNandIspRegion(DWORD tag) :
//int apCodePart1, apCodePart2;
//    apCodePart1 = NandCodeCheck(tag, 0);
//    apCodePart2 = NandCodeCheck(tag, 1);
int NandCodeCheck(DWORD tag, DWORD Index)
{
    MP_DEBUG("%s: tag = 0x%X, Index = %d", __func__, tag, Index);
    SDWORD startSec, maxSec;
    DWORD i, j, k;
    DWORD *tmp, ChkSum, Size;
    DWORD SectorNumberPerBlock, sectNrPerPage, pageSize;

    if (Index == 0)
    {
        startSec = NandBlockFind(tag);
        MP_DEBUG("%s: after NandBlockFind, tag(0x%X) startSec = %d", __func__, tag, startSec);
    }
    else
    {
        startSec = Nand2ndBlockFind(tag);
        MP_DEBUG("%s: after Nand2ndBlockFind, tag(0x%X) startSec = %d", __func__, tag, startSec);
    }

    // Not registered block or block not found
    if (startSec < 0)
    {
        MP_ALERT("--E-- %s: Index-%d Source area is not registered block or block not found", __FUNCTION__, Index);
        return FAIL;
    }

    IspGetInfo(DEV_NAND_ISP, &SectorNumberPerBlock, 0, &pageSize);
    //MP_DEBUG("%s: after IspGetInfo(), pageSize = %d", __func__, pageSize);
    sectNrPerPage = pageSize / DRV_ACCESS_SIZE;
    tmp = (DWORD *) ext_mem_malloc(pageSize);

    if (!tmp)
    {
        mpDebugPrint("%s - malloc fail", __FUNCTION__);
        return FAIL;
    }

    tmp = (DWORD *) ((DWORD) tmp | BIT29);
    maxSec = startSec + (NandIspGetBlockGap(tag) / DRV_ACCESS_SIZE);
    MP_DEBUG("%s: maxSecr = %d", __func__, maxSec);
    i = 0;

    while (startSec < maxSec)
    {
        //MP_DEBUG("while(startSec = %d)", startSec);
        if (!IspBlockIsBad(DEV_NAND_ISP, startSec))
        {
            for (j = 0 ; j < SectorNumberPerBlock; j += sectNrPerPage)
            {
                //MP_DEBUG("for j = %d", j);
                IspRead(DEV_NAND_ISP, startSec + j, (BYTE *) tmp, sectNrPerPage);

                // first sector
                if (i == 0)
                {
                    MP_DEBUG("tmp[0] = 0x%X", tmp[0]);
                    if (tmp[0] != tag)
                    {
                        MP_ALERT("--E-- tmp[0] = 0x%X, tmp[0] != tag, %s: Tag-%c%c%c%c error !!!!", tmp[0], __FUNCTION__,
                                  tag >> 24, (tag >> 16) & 0xFF, (tag >> 8) & 0xFF, tag & 0xFF);

                        ext_mem_free(tmp);

                        return FAIL;
                    }
                    MP_DEBUG("first sector pass! tmp[0] = 0x%X, tmp[0] == tag, get Size and Checksum", tmp[0]);
                    MP_DEBUG("Size = 0x%X, ChkSum = 0x%X", tmp[2], tmp[4]);

                    Size = tmp[2];
                    ChkSum = tmp[4];

                    for (k = 8 ; k < (pageSize >> 2); k++)
                        ChkSum += tmp[k];

                    Size = Size - (pageSize - 32);
                }
                else
                {
                    if (Size <= pageSize)
                    {
                        for (k = 0 ; k < (Size >> 2); k++)
                            ChkSum += tmp[k];

                        ext_mem_free(tmp);

                        if (ChkSum == 0)
                            return PASS;

                        MP_ALERT("--E-- %s: Tag-%c%c%c%c check sum fail !!!!", __FUNCTION__,
                                  tag >> 24, (tag >> 16) & 0xFF, (tag >> 8) & 0xFF, tag & 0xFF);

                        return FAIL;

                    }
                    else
                    {
                        for (k = 0 ; k < (pageSize >> 2); k++)
                            ChkSum += tmp[k];

                        Size -= pageSize;
                    }
                }

                i++;
            }
        }
        else
            MP_DEBUG("BadSector at sector %d", startSec);

        startSec += SectorNumberPerBlock;
    }

    MP_ALERT("--E-- %s: Tag-%c%c%c%c out of boundary !!!!", __FUNCTION__,
              tag >> 24, (tag >> 16) & 0xFF, (tag >> 8) & 0xFF, tag & 0xFF);

    ext_mem_free(tmp);

    return FAIL;
}


// Backup specific code type source block to target block
int NandCodeBackup(DWORD tag, DWORD SrcIndex)
{
    MP_DEBUG("%s: tag = 0x%X, SrcIndex = %d", __func__, tag, SrcIndex);
    SDWORD startSecAddr, tarStartSecAddr, maxSecAddr;
    DWORD i, j, k;
    DWORD *tmp, Size, *Buf = 0;
    DWORD SectorNumberPerBlock, sectNrPerPage, pageSize;

    if ((tag != AP_TAG) && (tag != RES_TAG))
    {
        MP_ALERT("--E-- %s: Tag-0x%08X not support backup !!!", __FUNCTION__);

        return FAIL;
    }

    // Get source data
    if (SrcIndex == 0)
        startSecAddr = NandBlockFind(tag);
    else
        startSecAddr = Nand2ndBlockFind(tag);

    // Source area is not registered block or block not found
    if (startSecAddr < 0)
    {
        MP_ALERT("--E-- %s: Source area is not registered block or block not found", __FUNCTION__);
        return FAIL;
    }

    MP_DEBUG("Source addr = 0x%08X", startSecAddr * DRV_ACCESS_SIZE);
    IspGetInfo(DEV_NAND_ISP, &SectorNumberPerBlock, 0, &pageSize);
    sectNrPerPage = pageSize / DRV_ACCESS_SIZE;
    tmp = (DWORD *) ext_mem_malloc(pageSize);

    if (!tmp)
    {
        mpDebugPrint("%s - malloc fail", __FUNCTION__);
        return FAIL;
    }

    tmp = (DWORD *) ((DWORD) tmp | BIT29);
    maxSecAddr = startSecAddr + (NandIspGetBlockGap(tag) / DRV_ACCESS_SIZE);
    i = 0;

    while (startSecAddr < maxSecAddr)
    {
        if (!IspBlockIsBad(DEV_NAND_ISP, startSecAddr))
        {
            for (j = 0; j < SectorNumberPerBlock; j += sectNrPerPage)
            {
                IspRead(DEV_NAND_ISP, startSecAddr + j, (BYTE *) tmp, sectNrPerPage);

                if (i == 0)
                {
                    if (tmp[0] != tag)
                    {
                        ext_mem_free(tmp);
                        return FAIL;
                    }

                    Size = tmp[2] + 32;
                    Buf = (DWORD *) ext_mem_malloc(Size + pageSize);

                    if (Buf == NULL)
                    {
                        mpDebugPrint("%s - malloc fail", __FUNCTION__);
                        ext_mem_free((void *) tmp);

                        return FAIL;
                    }

                    memcpy((BYTE *) Buf, (BYTE *) tmp, pageSize);
                    ext_mem_free(tmp);
                    tmp = Buf + (pageSize >> 2);
                }
                else
                {
                    tmp += pageSize >> 2;
                }

                i++;

                // Check if read end
                if ((i * pageSize) >= Size)
                {
                    startSecAddr += SectorNumberPerBlock;

                    goto END_SRC_READ;
                }
            }
        }

        startSecAddr += SectorNumberPerBlock;
    }

    ext_mem_free(Buf);
    MP_ALERT("--E-- Out of boundary !!!");
    return FAIL;

END_SRC_READ:
    // Now, StartAddr point to the start of last block
    // Write target data
    if (SrcIndex == 0)
    {
        tarStartSecAddr = Nand2ndBlockFind(tag);
        maxSecAddr = NandBlockFind(tag) + (NandIspGetBlockGap(tag) / DRV_ACCESS_SIZE);
    }
    else
    {
        tarStartSecAddr = NandBlockFind(tag);
        maxSecAddr = Nand2ndBlockFind(tag);
    }

    // Not registered block or block not found
    if (tarStartSecAddr < 0)
    {
        MP_DEBUG("Not registered block or block not found -");

        // Not registered block
        if (SrcIndex != 0)
        {
            ext_mem_free(Buf);
            MP_ALERT("--E-- %s: First data area not found !!!", __FUNCTION__);

            return FAIL;
        }
        else                    // Not created second block, Creat one
        {
            i = (Size + DRV_ACCESS_SIZE - 1) / DRV_ACCESS_SIZE;         // How many sector needed
            i = ((i + SectorNumberPerBlock - 1) / SectorNumberPerBlock) * SectorNumberPerBlock; // How many sector needed (block boundry)
            startSecAddr += SectorNumberPerBlock;                               // Point StartAddr to the start address of next block

            // Check if there is enough space for second area
            if (maxSecAddr < (startSecAddr + i))
            {
                ext_mem_free(Buf);
                MP_ALERT("--E-- %s: Not enough space for second area !!!", __FUNCTION__);

                return FAIL;
            }

            if (maxSecAddr > (startSecAddr + i + SectorNumberPerBlock))
                tarStartSecAddr = startSecAddr + SectorNumberPerBlock;
            else
                tarStartSecAddr = startSecAddr;

            // set second address
            if (tag == AP_TAG)
                SecondApAddr = tarStartSecAddr;
            else
                SecondResAddr = tarStartSecAddr;
        }
    }

    MP_DEBUG("Second addr = 0x%08X", tarStartSecAddr * DRV_ACCESS_SIZE);

    if (NandIspRealWrite((BYTE *) Buf, Size, (DWORD) tarStartSecAddr, maxSecAddr) < Size)
    {
        ext_mem_free(Buf);
        return FAIL;
    }

    ext_mem_free(Buf);
    return PASS;
}



#endif



#if 0   // nand content dump for engineer purpose
void ReadRawDataFromNand()
{
    MP_DEBUG("%s", __func__);
    DRIVE *drv;

    SystemDeviceInit(SD_MMC);
    DriveAdd(SD_MMC);
    drv = DriveChange(SD_MMC);
    DirReset(drv);
    DirFirst(drv);

    if (CreateFile(drv, "NandImg2", "bin") != PASS)
        mpDebugPrint("NandImg.bin can not be created!");
    else
    {
        DWORD PageSz, PagePerBlk, BlkNr, SectNr;
        DWORD tmp, i, j;
        STREAM *fptr;
        BYTE *rawbuffer;
        BYTE info[32];

        DriveRefresh(drv);
        fptr = FileOpen(drv);
        GetNandRawGeometry(&PageSz, &PagePerBlk);
        GetNandGeometry(&BlkNr, &tmp);
        SectNr = PageSz / 512;
        rawbuffer = (BYTE *)((DWORD)ext_mem_malloc(PageSz*PagePerBlk) | 0xA0000000);
        mpDebugPrint("Write nand content to SD as file:");

        for (i = 0 ; i < 16 ; i++)
        {
            sprintf(info, "block %d", i);
            UartOutText(info);
            SetMcardClock(48000);
            McardNandActive();

            for (j = 0 ; j < PagePerBlk ; j++)
            {
                NandRawPageRead((i*PagePerBlk+j)*SectNr, &rawbuffer[j*PageSz]);
                PutUartChar('.');
            }

            McardNandInactive();
            FileWrite(fptr, rawbuffer, PageSz*PagePerBlk);
            mpDebugPrint("ok");
        }

        FileClose(fptr);
        ext_mem_free(rawbuffer);
        mpDebugPrint("finished!(%d bytes)", PageSz * PagePerBlk * BlkNr);
    }
}



void WriteRawDataToNand()
{
    MP_DEBUG("%s", __func__);
    DRIVE *drv;

    SystemDeviceInit(SD_MMC);
    DriveAdd(SD_MMC);
    drv = DriveChange(SD_MMC);
    DirReset(drv);
    DirFirst(drv);

    if (FileSearch(drv, "NANDIMG", "BIN", E_FILE_TYPE) != PASS)
        mpDebugPrint("NandImg.bin can not be found!");
    else
    {
        DWORD PageSz, PagePerBlk, BlkNr, SectNr, SectPerBlk;
        DWORD i, j;
        STREAM *fptr;
        BYTE *rawbuffer;
        BYTE info[32];

        fptr = FileOpen(drv);
        GetNandRawGeometry(&PageSz, &PagePerBlk);
        GetNandGeometry(&BlkNr, &SectPerBlk);
        SectNr = PageSz / 512;
        rawbuffer = (BYTE *)((DWORD)ext_mem_malloc(PageSz*PagePerBlk) | 0xA0000000);
        mpDebugPrint("Write Nand content by file in SD:");

        for (i = 0 ; i < 16 ; i++)
        {
            sprintf(info, "block %d", i);
            UartOutText(info);
            FileRead(fptr, rawbuffer, PageSz*PagePerBlk);
            SetMcardClock(48000);
            McardNandActive();
            NandBlockErase(i*SectPerBlk);

            for (j = 0 ; j < PagePerBlk ; j++)
            {
                NandRawPageWrite((i*PagePerBlk+j)*SectNr, &rawbuffer[j*PageSz]);
                PutUartChar('.');
            }

            McardNandInactive();
            mpDebugPrint("ok");
        }

        FileClose(fptr);
        ext_mem_free(rawbuffer);
        mpDebugPrint("finished!(%d bytes)", PageSz * PagePerBlk * BlkNr);
    }
}
#endif

