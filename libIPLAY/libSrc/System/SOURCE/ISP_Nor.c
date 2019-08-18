/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "bios.h"
#include "fs.h"

#if (BOOTUP_TYPE == BOOTUP_TYPE_NOR && CHIP_VER_MSB == CHIP_VER_615)

#define NOR_8MB             0
#define CODE_FLASH_BASE     0xbfc00000
#define OFFSET_SETTAB       0x00004000
#if BLUETOOTH == ENABLE
#define OFFSET_BTTAB        0x00006000  //for BlueTooth Device Database
#define OFFSET_APP0         0x00008000
#else
#define OFFSET_APP0         0x00006000
#endif
#define OFFSET_RES          0x00190000

#define SEARCH_SIZE         0x00002000  //Address : 8k bytes
#define ROM_ADDR_RANG       0x003fffff  // 4  M bytes
#define ROM_LIMIT           (CODE_FLASH_BASE + ROM_ADDR_RANG)

static DWORD AddrShift = 0;


static void nop(DWORD num)
{
    DWORD xi;

    for (xi = 0; xi < num; xi++)
        __asm("nop");
}



void ISP_NorFlashWt(DWORD addr, WORD dat)
{
    addr += CODE_FLASH_BASE;

    if (addr > 0xBFFFFFFF)
        addr -= 0x800000;

    *(WORD volatile *) addr = dat;
}



WORD ISP_NorFlashRd(DWORD addr)
{
    addr += CODE_FLASH_BASE;

    if (addr > 0xBFFFFFFF)
        addr -= 0x800000;

    return (*(WORD volatile *) addr);
}



void ISP_NorFlashReset(void)
{
    Idu_WaitBufferEnd();
    ISP_NorFlashWt(0x000, 0xF0);
    nop(10);
}



void ISP_NorFlashDevCodeGet(WORD * deviceCode)
{
    ISP_NorFlashWt(0xaaa, 0xaa);
    ISP_NorFlashWt(0x554, 0x55);
    ISP_NorFlashWt(0xaaa, 0x90);
    *deviceCode = ISP_NorFlashRd(0x02);

    ISP_NorFlashReset();
}



void ISP_NorManuCodeGet(WORD * manuCode)
{
    ISP_NorFlashWt(0xaaa, 0xaa);
    ISP_NorFlashWt(0x554, 0x55);
    ISP_NorFlashWt(0xaaa, 0x90);

    *manuCode = ISP_NorFlashRd(0x00);

    ISP_NorFlashReset();
}



DWORD *ISP_NorBlockFind(DWORD tag)
{
    DWORD *pdwRes = (DWORD *) CODE_FLASH_BASE;
    DWORD *found = pdwRes;

#if NOR_8MB //NOR 8MB
    NORFLASH64MBINIT
    NORFLASH64MBLOW
#endif
    AddrShift = 0;

    while ((DWORD)pdwRes < ROM_LIMIT)
    {
        if (*pdwRes == tag)
            found = pdwRes;

        pdwRes += SEARCH_SIZE / 4;
    }

    pdwRes = found;

#if NOR_8MB //NOR 8MB
    if (*pdwRes != RES_TAG)
    {
        AddrShift = (ROM_ADDR_RANG + 1);
        NORFLASH64MBHIGH
        pdwRes = (DWORD *)CODE_FLASH_BASE;
        found = pdwRes;

        while ((DWORD)pdwRes < ROM_LIMIT)
        {
            if (*pdwRes == tag)
                found = pdwRes;

            pdwRes += SEARCH_SIZE / 4;
        }

        pdwRes = found;
    }
#endif

    return pdwRes;
}



BOOL ISP_NorFlashWrite(DWORD saddr, DWORD eaddr, WORD * cData)
{
    DWORD curAddr, cFatalAddr;
    BYTE fatalCnt = 0;
    BOOL finished = TRUE;
    DWORD timer = 0, cntTimer = 0;

    //16K bytes
    if (saddr < OFFSET_SETTAB)
        return FALSE;           // Sector0 is protected. DO NOT PROGRAMMING !!

    curAddr = saddr;
    cFatalAddr = saddr;

    Idu_WaitBufferEnd();

    do
    {
        ISP_NorFlashWt(0xaaa, 0xaa);
        ISP_NorFlashWt(0x554, 0x55);
        ISP_NorFlashWt(0xaaa, 0xa0);
        ISP_NorFlashWt(curAddr, *cData);

        //Delay 12usec is necessary
        nop(0x30);      // This number is erperimental result, depends on FLASH type

        if (ISP_NorFlashRd(curAddr) != (*cData))	// wait for Q7 = Q7
        {
            timer = 0;

            while (ISP_NorFlashRd(curAddr) != (*cData))	// wait for Q7 = Q7
            {
                if (32 <= timer)
                {

                    if (curAddr == cFatalAddr)
                    {
                        if (fatalCnt++ == 0x20)
                        {
                            //mpDebugPrint("TIME OUT !! FatalError\r\n");
                            curAddr = eaddr;    // terminate the programming
                            finished = FALSE;
                        }
                    }
                    else
                    {
                        cFatalAddr = curAddr;
                        fatalCnt = 0;
                    }

                    nop(0x2000);

                    //don't write again
                    curAddr -= 2;   // Half-WORD Programming
                    cData--;
                    //*/
                    ISP_NorFlashReset();
                    break;
                }

                timer++;
            }
        }

        curAddr += 2;               // Half-WORD Programming
        cData++;

        if (0x10000 == cntTimer)
        {
            //mpDebugPrint("ProgIndx@");
            //UartOutValue(curAddr, 8);
            //mpDebugPrint("\r\n");
            cntTimer = 0;
        }

        cntTimer++;
    } while (curAddr < eaddr);

    //ISP_NorFlashReset();
    nop(0x20);

    return finished;
}



BOOL VerifyFlashWrite(DWORD saddr, DWORD eaddr, WORD * cData)
{
    DWORD curAddr;
    BOOL passVerification = TRUE;
    DWORD cntTimer = 0;

    //MP_DEBUG("VerifyFlashWrite");

    if (saddr < OFFSET_SETTAB)
    {
        return FALSE;       // Sector0 is protected. DO NOT PROGRAMMING !!
    }

    curAddr = saddr;
    Idu_WaitBufferEnd();

    do
    {
        if (ISP_NorFlashRd(curAddr) != (*cData))
        {
            //MP_DEBUG("verification FatalError");
            //MP_DEBUG1("curAddr :%d", curAddr);
            passVerification = FALSE;
        }

        curAddr += 2;       // Half-WORD Programming
        cData++;

        if (0x20000 == cntTimer)
        {
            //MP_DEBUG1("VerifyIndx@ curAddr :%d", curAddr);
            cntTimer = 0;
        }

        cntTimer++;
    } while (curAddr < eaddr);

    return passVerification;
}



//single sector erase
BOOL ISP_NorSecErase(DWORD SA)
{
    DWORD timer = 0;
    WORD clearData = 0;

    Idu_WaitBufferEnd();

    ISP_NorFlashWt(0xaaa, 0xaa);
    ISP_NorFlashWt(0x554, 0x55);
    ISP_NorFlashWt(0xaaa, 0x80);
    ISP_NorFlashWt(0xaaa, 0xaa);
    ISP_NorFlashWt(0x554, 0x55);
    ISP_NorFlashWt(SA, 0x30);

    while (ISP_NorFlashRd(SA) != 0xffff)    // Check First WORD = 0xFFFF
    {
        if (timer >= 0x5000)
        {
            mpDebugPrint("ISP_NorSecErase:FAILED");
            return (0);
        }
        else
        {
            nop(0x1000);
        }

        timer++;
    }

    //FlashReset();
    return TRUE;
}



///
///@ingroup ISP_NOR
///@brief Update application code of NOR flash
///
///@param   BYTE *buffer  point of new application code
///@param   DWORD size    size of application code to program.
///
///@retval  PASS if success, else FAIL.
///
///@remark  Because the NOR flash is word mode, so the size must be
///         multiple of 2
///
static int SetNewApplication(BYTE * buffer, DWORD offset, DWORD size, DWORD range)
{
#if ISP_FUNC_ENABLE
    WORD deviceCode, result;
    DWORD i;
    FLASHREP flashRep;
    DWORD secnum1, secnum2, update_cnt;

    mpDebugPrint("%s ", __FUNCTION__);
    ISP_NorFlashDevCodeGet(&deviceCode);

    if (NewFlashRep(&flashRep, deviceCode & 0xFF))
    {   //not supported device id
        mpDebugPrint("Non-support flash chip!");

        return FAIL;
    }

    secnum1 = flashRep.getSecNum(&flashRep, offset);
    secnum2 = flashRep.getSecNum(&flashRep, range);
    update_cnt = (secnum2 - secnum1) / 3;	// update 4 time blindly

    //Sector Erase
    mpDebugPrint("SectorErase ...");

    for (i = secnum1; i <= secnum2; i++)
    {
        //if ((i % update_cnt) == 1)
        //    UpdateUI();

        if (!flashRep.eraseSector(&flashRep, i))
        {
            flashRep.eraseSector(&flashRep, i);
        }
    }

    //UpdateUI();
    mpDebugPrint("NorFlash Write ...");
    result = ISP_NorFlashWrite(offset, offset + size, (WORD *) buffer);

    if (result)
        mpDebugPrint("-->success!");
    else
        mpDebugPrint("-->failed!");

    mpDebugPrint("Verify...");
#if NOR_8MB //8MB
    NORFLASH64MBINIT
    NORFLASH64MBLOW
#endif

    BYTE *src = (BYTE *)buffer, *dst = (BYTE *)(CODE_FLASH_BASE+offset);

    for (i = 0 ; i < size ; i++)
    {
        if ((DWORD)dst > ROM_LIMIT)
        {
#if NOR_8MB //8MB
            NORFLASH64MBHIGH
            dst = (BYTE *)CODE_FLASH_BASE;
#else
            mpDebugPrint("Out of Norflash!(2)");
#endif
        }

        if (*src++ != *dst++)
            break;
    }

    if (i < size)
        mpDebugPrint("failed!(%d, %d), 0x%x", i, size, dst);
    else
        mpDebugPrint("success!");

    if (!result)
    {
        flashRep.free(&flashRep);

        return FAIL;
    }

    //mpDebugPrint("Start to verify ...");
/*
    result = VerifyFlashWrite(offset, offset + size, (WORD *) buffer);

    if (result)
    {
        mpDebugPrint("NorFlashVerify success!");
    }

    if (!result)
    {
        flashRep.free(&flashRep);
        return FAIL;
    }
*/

    flashRep.free(&flashRep);
#endif

    return PASS;
}   //end of SetNewApplication



///
///@ingroup MAIN
///@brief   Update code to NOR flash
///
///@param   DRIVE *drv  The drive contain code to uodate
///
///@retval  PASS if success, else FAIL.
///
///@remark  Because the NOR flash is word mode, so the size must be
///         mltiple of 2
///
int IspFunc_Write(BYTE *data, DWORD len, DWORD tag)
{
    int ret = 0;
    DWORD dwoffset, range;

    if (tag == AP_TAG)
    {
        dwoffset = OFFSET_APP0;
        range = OFFSET_RES - 1;
    }
    else if (tag == RES_TAG)
    {
        dwoffset = OFFSET_RES;
        range = OFFSET_RES + len;
    }
    else
        return FAIL;

    IntDisable();
    ret = SetNewApplication(data, dwoffset, len, range);
    IntEnable();

    return ret;
}   //end of NorUpDateCode



DWORD IspFunc_ReadAP(BYTE *buf, DWORD size)
{
    DWORD i, ret = PASS;
    DWORD *ptr = (DWORD *)(CODE_FLASH_BASE + OFFSET_APP0);

#if NOR_8MB     //NOR 8MB
    NORFLASH64MBINIT
    NORFLASH64MBLOW
#endif

    for (i = 0 ; i < size ; i++)
        buf[i] = ptr[i];

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
    DWORD *pdwRes, dwNumOfEntry, i;
    BYTE *pbOffset;

    pdwRes = ISP_NorBlockFind(RES_TAG);

    if (*pdwRes == RES_TAG)
    {
        pdwRes += 9;    // 32 byte sector header and 4 byte size

        if (!AddrShift && (DWORD)pdwRes > ROM_LIMIT)
        {
#if NOR_8MB //NOR 8MB
            NORFLASH64MBHIGH
            pdwRes = (DWORD *)(CODE_FLASH_BASE + (DWORD)pdwRes - ROM_LIMIT - 1);
            AddrShift = (ROM_ADDR_RANG + 1);
#else
            mpDebugPrint("Out of Norflash!(2)");
#endif
        }

        dwNumOfEntry = *pdwRes++;

        for (i = 0; i < dwNumOfEntry; i++)
        {
            if (!AddrShift && (DWORD)pdwRes > ROM_LIMIT)
            {
#if NOR_8MB //NOR 8MB
                NORFLASH64MBHIGH
                pdwRes = (DWORD *)(CODE_FLASH_BASE + (DWORD)pdwRes - ROM_LIMIT - 1);
                AddrShift = (ROM_ADDR_RANG + 1);
#else
                mpDebugPrint("Out of Norflash!(3)");
#endif
            }

            if (*pdwRes == dwTag)
            {
                MP_DEBUG("%c%c%c%c size = %x", (dwTag>>24)&0xff, (dwTag>>16)&0xff, (dwTag>>8)&0xff, dwTag&0xff, *(pdwRes+1));

                return *(pdwRes + 1);
            }

            pdwRes += 2;
        }
    }

    mpDebugPrint("Can not find Tag %x", dwTag);

    return 0;
}   //end of NorGetResourceSize



///
///@ingroup MAIN
///@brief   Get the start address of specific resource
///
///@param   DWORD dwTag  the tag of resource to find
///
///@retval  If the resource found, return the address, else return 0.
///
///@remark  The function call just search resource in resource sector, and the
///         return address is the address of NOR flash. If we want to access it through
///         DMA channel, we must move these data stream to sdram before access.
///
static BYTE *IspFunc_GetRESOURCEAddr(DWORD dwTag)
{
    DWORD *pdwRes, dwNumOfEntry, i;
    BYTE *pbOffset;

    pdwRes = ISP_NorBlockFind(RES_TAG);

    if (*pdwRes == RES_TAG)
    {
        //mpDebugPrint("RES_TAG Found\r\n");
        pdwRes += 9;	// 32 bytes Resource sector header(Added in isp file) and 4 byte Resource sector header size

        if (!AddrShift && (DWORD)pdwRes > ROM_LIMIT)
        {
#if NOR_8MB //NOR 8MB
            NORFLASH64MBHIGH
            pdwRes = (DWORD *)(CODE_FLASH_BASE + (DWORD)pdwRes - ROM_LIMIT - 1);
            AddrShift = (ROM_ADDR_RANG + 1);
#else
            mpDebugPrint("Out of Norflash!(4)");
#endif
        }

        dwNumOfEntry = *pdwRes++;
        pbOffset = (BYTE *) ((DWORD) pdwRes + (dwNumOfEntry << 3));	// real resource data is at

        for (i = 0; i < dwNumOfEntry; i++)
        {
            if (!AddrShift && (DWORD)pdwRes > ROM_LIMIT)
            {
#if NOR_8MB //NOR 8MB
                NORFLASH64MBHIGH
                pdwRes = (DWORD *)(CODE_FLASH_BASE + (DWORD)pdwRes - ROM_LIMIT - 1);
                AddrShift = (ROM_ADDR_RANG + 1);
#else
                mpDebugPrint("Out of Norflash!(5)");
#endif
            }

            if (*pdwRes == dwTag)
            {
                MP_DEBUG("%c%c%c%c found at %x", (dwTag>>24)&0xff, (dwTag>>16)&0xff, (dwTag>>8)&0xff, dwTag&0xff, pbOffset+AddrShift);
                return (pbOffset + AddrShift);
            }

            pbOffset = (BYTE *) ((DWORD) pbOffset + *(pdwRes + 1));
            pdwRes += 2;
        }
    }

    return NULL;  // resource not found
    //return (BYTE *) pdwRes;
}   //end of NorGetResource



BYTE *IspFunc_ReadRESOURCE(DWORD res_type, BYTE *pbTarget, DWORD dwSize)
{
    DWORD *Source,*Target;
    BYTE *pbResource = IspFunc_GetRESOURCEAddr(res_type);

    if(pbResource == NULL)
        MP_ALERT("Warning : No Resource Data");

#if NOR_8MB //NOR 8MB
    NORFLASH64MBINIT

    if ((DWORD)pbResource > ROM_LIMIT)
    {
        pbResource -= (ROM_ADDR_RANG + 1);
        NORFLASH64MBHIGH
    }
    else
        NORFLASH64MBLOW
#endif
    Source = (DWORD *)pbResource;
    Target = (DWORD *)pbTarget;
    dwSize = (dwSize >> 2);

    MP_DEBUG("Read resource 0x%x, to 0x%x(Size 0x%x)",Source,Target,dwSize);

    while (dwSize)
    {
        if((DWORD)Source > ROM_LIMIT)
        {
#if NOR_8MB //NOR 8MB
            NORFLASH64MBHIGH
            Source = (DWORD *)CODE_FLASH_BASE;
#else
            mpDebugPrint("Out of Norflash!(1)");
#endif
        }

        *Target = *Source;
        dwSize--;
        Target++;
        Source++;
    }

    return (pbTarget);  // Return the original pbTarget address when enter this function
}



//
//	Write user setting table to Nor flash.
//
//
#pragma alignvar(4)
int IspFunc_WriteUST(BYTE *pdwTempMen, DWORD buf_size)
{
    int iRet = FAIL;
    WORD deviceCode;
    FLASHREP flashRep;

    ISP_NorFlashDevCodeGet(&deviceCode);

    if (!NewFlashRep(&flashRep, deviceCode & 0xFF))
    {
        //Idu_WaitBufferEnd();  //Mason mark 20061114, sector erase func will do this// 07.13.2006 Athena - avoid screen shift
        if (!flashRep.eraseSector(&flashRep, flashRep.getSecNum(&flashRep, OFFSET_SETTAB)));
        {
            if (ISP_NorFlashWrite(OFFSET_SETTAB, OFFSET_SETTAB + buf_size, (WORD *)pdwTempMen))  //serapis 08/02/19 : settings not saving bug fixed
            {
                //if (VerifyFlashWrite(OFFSET_SETTAB, OFFSET_SETTAB + 256, (WORD *)pdwTempMen))
                iRet = PASS;
            }
        }
    }

    flashRep.free(&flashRep);

    return iRet;
}



//
// Read user setting table from Nor flash.
//
//
DWORD IspFunc_ReadUST(BYTE *pdwTempMen, DWORD buf_size)
{
    volatile DWORD *ptr = (DWORD *) (CODE_FLASH_BASE + OFFSET_SETTAB);
    DWORD *dst = (DWORD *)pdwTempMen;

#if NOR_8MB //NOR 8MB
    NORFLASH64MBINIT
    NORFLASH64MBLOW
#endif

    buf_size >>= 2;

    while (buf_size--)
        *dst++ = *ptr++;
}



//
// Write factory setting table to Nor flash.
//
//
int IspFunc_WriteFST(BYTE *pdwTempMen, DWORD buf_size)
{
    DWORD *pdwRes;
    int iRet = FAIL;

    pdwRes = ISP_NorBlockFind(RES_TAG);

    if (*pdwRes == RES_TAG)
    {
        DWORD FST_Addr = ROM_LIMIT + NOR_8MB * (ROM_ADDR_RANG + 1) - 65535;	// 65536 is block size of MX29LV320B/MX29LV640B
        pdwRes += 2;

        if ((CODE_FLASH_BASE + OFFSET_RES + *pdwRes) < FST_Addr)	// last block is not used by RES.
        {
            WORD deviceCode;
            FLASHREP flashRep;

            ISP_NorFlashDevCodeGet(&deviceCode);

            if (!NewFlashRep(&flashRep, deviceCode & 0xFF))
            {
                //Idu_WaitBufferEnd();  //Mason mark 20061114, sector erase func will do this// 07.13.2006 Athena - avoid screen shift
                FST_Addr -= CODE_FLASH_BASE;

                if (!flashRep.eraseSector(&flashRep, flashRep.getSecNum(&flashRep, FST_Addr)));
                {
                    if (ISP_NorFlashWrite(FST_Addr, FST_Addr + buf_size, (WORD *)pdwTempMen))  //serapis 08/02/19 : settings not saving bug fixed
                    {
                        //if (VerifyFlashWrite(OFFSET_SETTAB, OFFSET_SETTAB + 256, (WORD *)pdwTempMen))
                        iRet = PASS;
                    }
                }
            }

            flashRep.free(&flashRep);
        }
        else
            mpDebugPrint("%s(): no enough space for FST", __FUNCTION__);
    }

    return iRet;
}



//
// Read factory setting table from Nor flash.
//
//
DWORD IspFunc_ReadFST(BYTE *pdwTempMen, DWORD buf_size)
{
    DWORD *FST_Addr = (DWORD *)(ROM_LIMIT - 65535); // 65536 is block size of MX29LV320B/MX29LV640B
    DWORD *dst = (DWORD *)pdwTempMen;

#if NOR_8MB //NOR 8MB
    NORFLASH64MBINIT
    NORFLASH64MBHIGH
#endif

    buf_size >>= 2;

    while (buf_size--)
        *dst++ = *FST_Addr++;

    return PASS;
}



#if BLUETOOTH == ENABLE
//BlueTooth Read&Write Nor
int ISP_WriteBTDevice(BYTE *data, uint16_t len)
{
#if ISP_FUNC_ENABLE
    int i, iRet = FAIL;
    WORD deviceCode;
    FLASHREP flashRep;
    DWORD *pdwTempMem;
    pdwTempMem = (DWORD *)data;

    if(len == 0)
    {
        /* un-initialized BlueTooth Table */
        /* copy BlueTooth Table Tag and verify tag */
        memcpy((BYTE *)pdwTempMem, (BYTE *)BT_TAG, 4);
        len += 4;
        memcpy((BYTE *)pdwTempMem + len, (BYTE *)SETUP_INIT_BIT, 4);
        len += 4;
    }

    IntDisable();
    ISP_NorFlashDevCodeGet(&deviceCode);

    if (!NewFlashRep(&flashRep, deviceCode & 0xFF))
    {
        if (flashRep.eraseSector(&flashRep, flashRep.getSecNum(&flashRep, 0x6000)))
        {
            if (ISP_NorFlashWrite(OFFSET_BTTAB, OFFSET_BTTAB + len, (WORD *)pdwTempMem))
            {
                if (VerifyFlashWrite(OFFSET_BTTAB, OFFSET_BTTAB + len, (WORD *)pdwTempMem))
                    iRet = PASS;
            }
        }
    }

    flashRep.free(&flashRep);
    IntEnable();

    return iRet;
#else
    return FAIL;
#endif
}



void ISP_ReadBTDevice(BYTE *trg, SWORD size, SWORD *fp)
{
    BYTE *pbTmpMem;
    DWORD *pdwTmpMem;

    /* NULL => Start to read BlueTooth Table in Nor */
    if (*fp == 0)
    {
        pdwTmpMem = (DWORD *) (CODE_FLASH_BASE + OFFSET_BTTAB);

        if(pdwTmpMem[0] != BT_TAG || pdwTmpMem[1] != BT_INIT_BIT)
        {
            /* initialize not yet*/
            *fp = -1;
            return;
        }
        else
            *fp +=8;
    }

    /* read data from offset *fp */
    pbTmpMem = (BYTE *)(CODE_FLASH_BASE + OFFSET_BTTAB);
    pbTmpMem += *fp;
    memcpy((BYTE *)trg, (BYTE *)pbTmpMem, size);
    *fp += size;
}
#endif


void EraseNOR()
{
    WORD deviceCode, i;
    FLASHREP flashRep;
    DWORD secnum1, secnum2;

    ISP_NorFlashDevCodeGet(&deviceCode);

    if (NewFlashRep(&flashRep, deviceCode & 0xFF))
    {   //not supported device id
        mpDebugPrint("Non-support flash chip!");

        return; //FAIL;
    }

    secnum1 = flashRep.getSecNum(&flashRep, OFFSET_APP0);
    secnum2 = flashRep.getSecNum(&flashRep, ROM_LIMIT);

    //Sector Erase
    mpDebugPrint("All SectorErase ...");

    for (i = secnum1; i <= secnum2; i++)
    {
        if (!flashRep.eraseSector(&flashRep, i))
        {
            flashRep.eraseSector(&flashRep, i);
        }
    }

    flashRep.free(&flashRep);
}

#endif  // #if (BOOTUP_TYPE == BOOTUP_TYPE_NOR)

