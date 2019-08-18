
#define LOCAL_DEBUG_ENABLE  0
#include "global612.h"
#include "mpTrace.h"
#include "charset.h"
#include "charset_int.h"

WORD ConvertCharByTab(WORD srcChar, const unsigned char* pbMapTable, const unsigned short* pwConvertTable)
{
	const WORD *pwStart;
	BYTE high8, low8, idx2;;
	high8 = srcChar >> 8;
	low8  = srcChar & 0xff;
	idx2 = pbMapTable[high8];
	if ( idx2 == 0xff )
		return 0xffff;
	pwStart = & pwConvertTable[idx2*256];
	return pwStart[low8];
}

int common_ToUTF16(const BYTE* pbByteString, WORD* pwWordBuffer, DWORD dwWordBufSize, WORD wDefaultChar, WORD (*b2w)(WORD))
{
	BYTE ch1, ch2, bEndFlag;
	WORD wSrcChar, wDstChar;
	int byteCnt, wordCnt;

	bEndFlag = FALSE;
	byteCnt = wordCnt = 0;
	while (1)
	{
		ch1 = pbByteString[byteCnt++];
		if ( ch1==0 )
			bEndFlag = TRUE;
		if ( isLeadByte(ch1) ) {
			ch2 = pbByteString[byteCnt++];
			if ( ch2==0 )
				bEndFlag = TRUE;
			wSrcChar = (ch1 << 8) | ch2;
		}
		else
			wSrcChar = ch1;
		wDstChar = b2w(wSrcChar);
		if ( wDstChar == 0xffff )
			wDstChar = wDefaultChar;
		if ( pwWordBuffer )
		{
			if ( wordCnt < (int)(dwWordBufSize-2) )
				pwWordBuffer[wordCnt++] = wDstChar;
			else
			{
				pwWordBuffer[wordCnt++] = wDstChar;
				pwWordBuffer[wordCnt++] = 0;
				bEndFlag = TRUE;
			}
		}
		else
		{
			wordCnt++;
		}

		if ( bEndFlag )
			break;
	}
	return wordCnt;
}

int common_FromUTF16(const WORD* pwWordString, BYTE* pbByteBuffer, DWORD dwByteBufSize, BYTE bDefaultChar,  WORD (*w2b)(WORD))
{
	BYTE bEndFlag;
	WORD wSrcChar, wDstChar;
	int byteCnt, wordCnt;

	bEndFlag = FALSE;
	byteCnt = wordCnt = 0;
	while (1)
	{
		wSrcChar = pwWordString[wordCnt++];
		if ( wSrcChar == 0 )
			bEndFlag = TRUE;
		wDstChar = w2b(wSrcChar);
		if ( wDstChar == 0xffff )
			wDstChar = bDefaultChar;
		if ( pbByteBuffer )
		{
			if ( wDstChar <= 0xff )
			{
				if ( byteCnt < (int)(dwByteBufSize-2) )
					pbByteBuffer[byteCnt++] = (BYTE)wDstChar;
				else
				{
					pbByteBuffer[byteCnt++] = (BYTE)wDstChar;
					pbByteBuffer[byteCnt++] = 0;
					bEndFlag = TRUE;
				}
			}
			else
			{
				if ( byteCnt < (int)(dwByteBufSize-3) )
				{
					pbByteBuffer[byteCnt++] = (BYTE)(wDstChar >> 8);
					pbByteBuffer[byteCnt++] = (BYTE)(wDstChar & 0xff);					
				}
				else
				{
					pbByteBuffer[byteCnt++] = (BYTE)(wDstChar >> 8);
					pbByteBuffer[byteCnt++] = (BYTE)(wDstChar & 0xff);
					pbByteBuffer[byteCnt++] = 0;
					bEndFlag = TRUE;
				}
			}
		}
		else
		{
			if ( wDstChar <= 0xff )
				byteCnt ++;
			else
				byteCnt += 2;
		}
		if ( bEndFlag )
			break;
	}
	return byteCnt;
}











