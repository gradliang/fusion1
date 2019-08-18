
#include "global612.h"
#include "mpTrace.h"

#ifndef CHARSET_INT_H_C3B23803081067B84DE0501F7B18DB9A
#define CHARSET_INT_H_C3B23803081067B84DE0501F7B18DB9A

WORD ConvertCharByTab(WORD srcChar, const unsigned char* pbMapTable, const unsigned short* pwConvertTable);
int common_ToUTF16(const BYTE* pbByteString, WORD* pwWordBuffer, DWORD dwWordBufSize, WORD wDefaultChar, WORD (*b2w)(WORD));
int common_FromUTF16(const WORD* pwWordString, BYTE* pbByteBuffer, DWORD dwByteBufSize, BYTE bDefaultChar,  WORD (*w2b)(WORD));


#endif	//CHARSET_INT_H_C3B23803081067B84DE0501F7B18DB9A


