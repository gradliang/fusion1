/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "display.h"
#include "os.h"


//**********************************************************************************
// IODelay
// This routine delay io time
//
// INPUT:
//  WORD nDelay - delay count
// OUTPUT:
//  NONE
//**********************************************************************************
void IODelay(WORD nDelay)
{
	volatile DWORD i, j;

	for (i = 0; i < nDelay; i++)
		for (j = 0; j < nDelay; j++);
}

DWORD GetBitByOffset(DWORD *pdwStrptr, DWORD dwOffset)	//Mason 20060731  //Get value of specific bit
{
    pdwStrptr += (dwOffset >> 5);
    return ((*pdwStrptr & (1 << (dwOffset & 0x1f))) >> (dwOffset & 0x1f));
}

void SetBitByOffset(DWORD *pdwStrptr, DWORD dwOffset, DWORD dwValue)	//Mason 20060731  //Set value of specific bit
{
    dwValue &= 0x1;
    pdwStrptr += (dwOffset >> 5);
    if (dwValue)
        *pdwStrptr |= (1 << (dwOffset & 0x1f));
    else
        *pdwStrptr &= ~(1 << (dwOffset & 0x1f));
}



//extern void decode_more_audio(void);
void WaitMs(DWORD ms)
{
    DWORD Curr_MS, Target_MS, remainMs;

    if ((SysTimerStatusGet() == FALSE) || (ms < 1))
        return;

    Curr_MS = GetSysTime();
    Target_MS = Curr_MS + ms;

    if (Target_MS < Curr_MS)
    {
        remainMs = Curr_MS;

        while (Curr_MS > remainMs)
        {
            #if AUDIO_ON
            decode_more_audio();
			#endif
            Curr_MS = GetSysTime();
        }
    }

    while (Curr_MS < Target_MS)
    {
        #if AUDIO_ON
        decode_more_audio();
		#endif
        Curr_MS = GetSysTime();
    }
}



void TimerDelay(DWORD dwDelayTime)
{
    DWORD dwCurTime;
#if 0
    if ((SysTimerStatusGet() == FALSE) || (ContextStatusGet() != 0))
    {   // In ISR or system timer not active yet
        IODelay(dwDelayTime);

        return;
    }
#endif
    dwCurTime = GetSysTime();

    while (SystemGetElapsedTime(dwCurTime) < dwDelayTime)
    {
        TaskYield();
    }
}



void MpMemCopy(BYTE *tar, BYTE *src, DWORD length)
{
    DWORD *pdwSrc, *pdwTar, dwTmp;
    WORD *pwSrc, *pwTar;

    if (length < 4)
    {
        for (dwTmp = 0; dwTmp < length; dwTmp++)
            tar[dwTmp] = src[dwTmp];

        return;
    }

    if ( ( ((DWORD)src & 0x00000003) == 0 ) && ( ((DWORD) tar & 0x00000003) == 0 ))
    {   // DWORD boundry
        pdwSrc = (DWORD *) src;
        pdwTar = (DWORD *) tar;

        for (dwTmp = length; dwTmp >= 4; dwTmp -= 4)
        {
            *pdwTar = *pdwSrc;
            pdwTar++;
            pdwSrc++;
        }

        length = dwTmp;
        tar = (BYTE *) pdwTar;
        src = (BYTE *) pdwSrc;
    }
    else if ((((DWORD)src & 0x00000001)==0) && (((DWORD)tar & 0x00000001)==0))
    {   // WORD boundry
        pwSrc = (WORD *) src;
        pwTar = (WORD *) tar;

        for (dwTmp = length; dwTmp >= 2; dwTmp -= 2)
        {
            *pwTar = *pwSrc;
            pwTar++;
            pwSrc++;
        }

        if (dwTmp)
            *((BYTE *) pwTar) = *((BYTE *) pwSrc);

        return;
    }

    for (dwTmp = 0; dwTmp < length; dwTmp++)
        tar[dwTmp] = src[dwTmp];
}



void MpMemSet(BYTE *tar, BYTE value, DWORD length)
{
    DWORD tmpAddr = (DWORD) tar;
    DWORD remLen = length;
    DWORD count;
    BYTE remainder;
    BYTE i;

    if (!tar || (length == 0))
    {
        MP_ALERT("--E-- %s - Null pointer or zero length !!!", __FUNCTION__);
        __asm("break 100");

        return;
    }

    if (length < 4)
    {
        for (i = 0; i < length; i++)
            tar[i] = value;

        return;
    }

    tmpAddr = (DWORD) tar;
    remLen = length;
    remainder = tmpAddr & 0x03;

    if (remainder)
    {   // address is not 4 bytes alignment.
        i = 4 - remainder;
        remLen -= i;

        while (i--)
        {
            *tar = value;
            tar++;
        }

        remainder = 0;
    }

    if (remLen & 0x03)
    {   // remain length is not multiplex of 4
        remainder = remLen & 0x03;
        remLen -= remainder;
    }

    if (remLen)
    {
        DWORD newValue = value;

        newValue |= (newValue << 24) | (newValue << 16) | (newValue << 8);
        tmpAddr = (DWORD) tar;
        tar += remLen;

        for (count = 0; count < (remLen >> 2); count++)
            ((DWORD *) tmpAddr)[count] = newValue;
    }

    while (remainder--)
    {
        *tar = value;
        tar++;
    }
}



void MpMemSetDWORD(DWORD *tar, DWORD value, DWORD byteCount)
{
    DWORD dwTmp;

    if (!tar || !byteCount)
    {
        MP_ALERT("--E-- %s - Null pointer or zero length !!!", __FUNCTION__);
        __asm("break 100");

        return;
    }

    if ((((DWORD) tar) & 0x00000003) != 0)
    {
        MP_ALERT("--E-- %s - address is not 4 bytes alignment !!!", __FUNCTION__);
        __asm("break 100");

        return;
    }

    for (dwTmp = 0; dwTmp < (byteCount >> 2); dwTmp++)
        tar[dwTmp] = value;
}


#if MPO
BOOL UtilStringCompareWoCase08( const BYTE *a, const BYTE *b, WORD length)
{
    BYTE aa, bb;

    while ( *a && *b && length)
    {
        aa = *a;
        bb = *b;

        // 'a' - 'A' = 0x61 - 0x41 = 0x20
        if ( (aa >= 'a') && (aa <= 'z') )
            aa -= 0x20;

        if ( (bb >= 'a') && (bb <= 'z') )
            bb -= 0x20;

//        mpDebugPrint("aa=%c bb=%c", aa, bb);

        if ( aa != bb )
            return 0;

        a++;
        b++;
        length--;
    }

    return 1;
}
#endif



