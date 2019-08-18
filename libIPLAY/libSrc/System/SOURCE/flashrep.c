/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "mpTrace.h"

#include "iplaysysconfig.h"
#include "flashrep.h"


#if (BOOTUP_TYPE == BOOTUP_TYPE_NOR && CHIP_VER_MSB == CHIP_VER_615)

#define SEC_SIZE        0x10000 //64k bytes

// ****** stardard ******
void _new(FLASHREP *);
void _free(FLASHREP *);
WORD _getSecNum(FLASHREP *, DWORD);
WORD _getMaxSecNum(FLASHREP *);
BOOL _eraseSector(FLASHREP *, WORD);

// ****** DEVI_MX29LV800T ******
void MX29LV800T_new(FLASHREP *);
WORD MX29LV800T_getSecNum(FLASHREP *, DWORD);
BOOL MX29LV800T_eraseSector(FLASHREP *, WORD);

// ****** DEVI_MX29LV800B ******
void MX29LV800B_new(FLASHREP *);
WORD MX29LV800B_getSecNum(FLASHREP *, DWORD);
BOOL MX29LV800B_eraseSector(FLASHREP *, WORD);

// ****** DEVI_MX29LV160T ******
void MX29LV160T_new(FLASHREP *);
WORD MX29LV160T_getSecNum(FLASHREP *, DWORD);
BOOL MX29LV160T_eraseSector(FLASHREP *, WORD);

// ****** DEVI_MX29LV160B ******
void MX29LV160B_new(FLASHREP *);
WORD MX29LV160B_getSecNum(FLASHREP *, DWORD);
BOOL MX29LV160B_eraseSector(FLASHREP *, WORD);

// ****** DEVI_MX29LV320CB ******
void MX29LV320CB_new(FLASHREP *);
WORD MX29LV320CB_getSecNum(FLASHREP *, DWORD);
BOOL MX29LV320CB_eraseSector(FLASHREP *, WORD);


// ****** DEVI_MX29LV320CB ******
void MX29L64V0B_new(FLASHREP*);


void _new(FLASHREP * in_pFlashRep)
{
    in_pFlashRep->tag = 0;
    in_pFlashRep->maxSecNum = 0;
    in_pFlashRep->free = _free;
    in_pFlashRep->getSecNum = _getSecNum;
    in_pFlashRep->getMaxSecNum = _getMaxSecNum;
    in_pFlashRep->eraseSector = _eraseSector;
}



void _free(FLASHREP * in_pFlashRep)
{
    in_pFlashRep->tag = 0;
    in_pFlashRep->maxSecNum = 0;
    in_pFlashRep->free = NULL;
    in_pFlashRep->getSecNum = NULL;
    in_pFlashRep->getMaxSecNum = NULL;
    in_pFlashRep->eraseSector = NULL;
}



WORD _getSecNum(FLASHREP * in_pFlashRep, DWORD indISPSize)
{
    WORD secNum = 0;

    secNum = (indISPSize) / SEC_SIZE + (((indISPSize) % SEC_SIZE) ? 1 : 0);	// four bytes for size

    return secNum;
}



WORD _getMaxSecNum(FLASHREP * in_pFlashRep)
{
    return in_pFlashRep->maxSecNum;
}



BOOL _eraseSector(FLASHREP * in_pFlashRep, WORD inSecNo)
{
    // do nothing here
    return 1;
}



WORD NewFlashRep(FLASHREP * in_pFlashRep, WORD inFlashType)
{
    WORD result = 0;			//no error

    switch (inFlashType)
    {
    case DEVI_MX29LV800T:
        MX29LV800T_new(in_pFlashRep);
        break;

    case DEVI_MX29LV800B:
        MX29LV800B_new(in_pFlashRep);
        break;

    case DEVI_MX29LV160T:
        MX29LV160T_new(in_pFlashRep);
        break;

    case DEVI_MX29LV160B:
        MX29LV160B_new(in_pFlashRep);
        break;

    case DEVI_MX29LV320CB:
        MX29LV320CB_new(in_pFlashRep);
        break;

    case DEVI_MX29LV640B:
        MX29LV640B_new(in_pFlashRep);
        break;

    default:
        MP_ALERT("Not available flash type");
        result = 1;     //not supported
        break;
    }

    return result;
}



// ****** DEVI_MX29LV800T ******
void MX29LV800T_new(FLASHREP * in_pFlashRep)
{
    in_pFlashRep->tag = DEVI_MX29LV800T;
    in_pFlashRep->maxSecNum = 18;
    in_pFlashRep->free = _free;
    in_pFlashRep->getSecNum = MX29LV800T_getSecNum;
    in_pFlashRep->getMaxSecNum = _getMaxSecNum;
    in_pFlashRep->eraseSector = MX29LV800T_eraseSector;
}



WORD MX29LV800T_getSecNum(FLASHREP * in_pFlashRep, DWORD indISPSize)
{
    WORD secNum = 0;

    secNum = (indISPSize) / SEC_SIZE + (((indISPSize) % SEC_SIZE) ? 1 : 0);	// four bytes for size

    if (secNum == 15)
    {
        secNum = (indISPSize & 0x8000) ?
                ((indISPSize & 0x4000) ? 18 : ((indISPSize & 0x2000) ? 17 : 16)) : 15;
    }
    else if (secNum >= 16)
        secNum += 3;

    return secNum;
}



BOOL MX29LV800T_eraseSector(FLASHREP * in_pFlashRep, WORD inSecNo)
{
    DWORD SA;
    BOOL result = 0;

    switch (inSecNo)
    {
    case 0:
        mpDebugPrint("Protection Area!\r\n");

        return (1);         // Not Success !! But re-erase un-necessary!!
    case 16:
        SA = 0xF8000;
        break;
    case 17:
        SA = 0xFA000;
        break;
    case 18:
        SA = 0xFC000;
        break;
    default:
        SA = inSecNo * 0x10000;
        break;
    }

    result = ISP_NorSecErase(SA);

    return (result);
}



// ****** DEVI_MX29LV800B ******
void MX29LV800B_new(FLASHREP * in_pFlashRep)
{
    in_pFlashRep->tag = DEVI_MX29LV800B;
    in_pFlashRep->maxSecNum = 18;
    in_pFlashRep->free = _free;
    in_pFlashRep->getSecNum = MX29LV800B_getSecNum;
    in_pFlashRep->getMaxSecNum = _getMaxSecNum;
    in_pFlashRep->eraseSector = MX29LV800B_eraseSector;
}



WORD MX29LV800B_getSecNum(FLASHREP * in_pFlashRep, DWORD indISPSize)
{
    WORD secNum = 0;

    //secNum = (indISPSize)/SEC_SIZE + (((indISPSize)%SEC_SIZE)?1:0);
    secNum = (indISPSize) / SEC_SIZE;

    if (secNum == 0)
    {
        secNum = (indISPSize & 0x8000) ?
                 3 : ((indISPSize & 0x4000) ? ((indISPSize & 0x2000) ? 2 : 1) : 0);
    }
    else if (secNum >= 1)
        secNum += 3;

    return secNum;
}



WORD MX29LV320CB_getSecNum(FLASHREP * in_pFlashRep, DWORD indISPSize)
{
    WORD secNum = 0;
    DWORD temp;

    //secNum = (indISPSize)/SEC_SIZE + (((indISPSize)%SEC_SIZE)?1:0);
    secNum = (indISPSize) / SEC_SIZE;
    if (secNum == 0)
    {
        temp = indISPSize & 0xF000;

        switch (temp)
        {
        case 0xF000:
        case 0xE000:
            secNum = 7;
            break;
        case 0xD000:
        case 0xC000:
            secNum = 6;
            break;
        case 0xB000:
        case 0xA000:
            secNum = 5;
            break;
        case 0x9000:
        case 0x8000:
            secNum = 4;
            break;
        case 0x7000:
        case 0x6000:
            secNum = 3;
            break;
        case 0x5000:
        case 0x4000:
            secNum = 2;
            break;
        case 0x3000:
        case 0x2000:
            secNum = 1;
            break;
        default:
            secNum = 0;
            break;
        }
    }
    else if (secNum >= 1)
        secNum += 7;

    return secNum;
}



WORD MX29LV640B_getSecNum(FLASHREP* in_pFlashRep, DWORD indISPSize)
{
    WORD	secNum = 0;
    DWORD temp;

    //secNum = (indISPSize)/SEC_SIZE + (((indISPSize)%SEC_SIZE)?1:0);
    secNum = indISPSize / SEC_SIZE;

    if(secNum == 0)
    {
        temp = indISPSize & 0xF000;

        switch(temp)
        {
        case 0xF000:
        case 0xE000:
            secNum = 7;
            break;
        case 0xD000:
        case 0xC000:
            secNum = 6;
            break;
        case 0xB000:
        case 0xA000:
            secNum = 5;
            break;
        case 0x9000:
        case 0x8000:
            secNum = 4;
            break;
        case 0x7000:
        case 0x6000:
            secNum = 3;
            break;
        case 0x5000:
        case 0x4000:
            secNum = 2;
            break;
        case 0x3000:
        case 0x2000:
            secNum = 1;
            break;
        default:
            secNum = 0;
            break;
        }
    }
    else if(secNum >= 1)
        secNum += 7;

    return secNum;
}



BOOL MX29LV800B_eraseSector(FLASHREP * in_pFlashRep, WORD inSecNo)
{
    DWORD SA;
    BOOL result = 0;

    switch (inSecNo)
    {
    case 0:
        mpDebugPrint("Protection Area!\r\n");
        return TRUE;        // Not Success !! But re-erase un-necessary!!
    case 1:
    case 2:
    case 3:
        SA = (inSecNo - 1) * 0x2000 + 0x4000;
        break;
    default:
        SA = (inSecNo - 3) * 0x10000;
        break;
    }

    result = ISP_NorSecErase(SA);

    return (result);
}



BOOL MX29LV320CB_eraseSector(FLASHREP * in_pFlashRep, WORD inSecNo)
{
    DWORD SA;
    BOOL result = 0;

    switch (inSecNo)
    {
    case 0:
    case 1:
        mpDebugPrint("Protection Area!\r\n");
        return TRUE;        // Not Success !! But re-erase un-necessary!!
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        SA = (inSecNo - 2) * 0x2000 + 0x4000;
        break;
    default:
        SA = (inSecNo - 7) * 0x10000;
        break;
    }

    result = ISP_NorSecErase(SA);

    return (result);
}



// ****** DEVI_MX29LV160T ******
void MX29LV160T_new(FLASHREP * in_pFlashRep)
{
    in_pFlashRep->tag = DEVI_MX29LV160T;
    in_pFlashRep->maxSecNum = 34;
    in_pFlashRep->free = _free;
    in_pFlashRep->getSecNum = MX29LV160T_getSecNum;
    in_pFlashRep->getMaxSecNum = _getMaxSecNum;
    in_pFlashRep->eraseSector = MX29LV160T_eraseSector;
}



WORD MX29LV160T_getSecNum(FLASHREP * in_pFlashRep, DWORD indISPSize)
{
    WORD secNum = 0;

    secNum = (indISPSize) / SEC_SIZE + (((indISPSize) % SEC_SIZE) ? 1 : 0);	// four bytes for size

    if (secNum == 31)
    {
        secNum = (indISPSize & 0x8000) ?
                 ((indISPSize & 0x4000) ? 34 : ((indISPSize & 0x2000) ? 33 : 32)) : 31;
    }
    else if (secNum >= 32)
        secNum += 3;

    return secNum;
}



BOOL MX29LV160T_eraseSector(FLASHREP * in_pFlashRep, WORD inSecNo)
{
    DWORD SA;
    BOOL result = 0;

    switch (inSecNo)
    {
    case 0:
        UartOutText("Protection Area!\r\n");
        return (1);     // Not Success !! But re-erase un-necessary!!
    case 32:
        SA = 0x1F8000;
        break;
    case 33:
        SA = 0x1FA000;
        break;
    case 34:
        SA = 0x1FC000;
        break;
    default:
        SA = inSecNo * 0x10000;
        break;
    }

    result = ISP_NorSecErase(SA);

    return (result);
}



// ****** DEVI_MX29LV160B ******
void MX29LV160B_new(FLASHREP * in_pFlashRep)
{
    in_pFlashRep->tag = DEVI_MX29LV160B;
    in_pFlashRep->maxSecNum = 34;
    in_pFlashRep->free = _free;
    in_pFlashRep->getSecNum = MX29LV800B_getSecNum;
    in_pFlashRep->getMaxSecNum = _getMaxSecNum;
    in_pFlashRep->eraseSector = MX29LV800B_eraseSector;
}



// ****** DEVI_MX29LV320CB ******
void MX29LV320CB_new(FLASHREP *in_pFlashRep)
{
    in_pFlashRep->tag = DEVI_MX29LV320CB;
    in_pFlashRep->maxSecNum = 71;
    in_pFlashRep->free = _free;
    in_pFlashRep->getSecNum = MX29LV320CB_getSecNum;
    in_pFlashRep->getMaxSecNum = _getMaxSecNum;
    in_pFlashRep->eraseSector = MX29LV320CB_eraseSector;
}



// ****** DEVI_MX29LV640B ******
void MX29LV640B_new(FLASHREP *in_pFlashRep)
{
    in_pFlashRep->tag = DEVI_MX29LV640B;
    in_pFlashRep->maxSecNum = 135;
    in_pFlashRep->free = _free;
    in_pFlashRep->getSecNum = MX29LV320CB_getSecNum;
    in_pFlashRep->getMaxSecNum = _getMaxSecNum;
    in_pFlashRep->eraseSector = MX29LV320CB_eraseSector;
}

#endif

