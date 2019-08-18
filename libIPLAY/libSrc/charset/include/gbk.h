#include "global612.h"
#include "mpTrace.h"

#ifndef GBK_H_3454C3C4D649FFD7BC1E683064709B00
#define GBK_H_3454C3C4D649FFD7BC1E683064709B00

#if CHARSET_GBK_ENABLE

BOOL gbk_isLeadByte(BYTE ch);
int gbk_ToUTF16(const BYTE* pbByteString, WORD* pwWordBuffer, DWORD dwWordBufSize, WORD wDefaultChar);	
int gbk_FromUTF16(const WORD* pwWordString, BYTE* pbByteBuffer, DWORD dwByteBufSize, BYTE bDefaultChar);

#endif	//
#endif	//GBK_H_3454C3C4D649FFD7BC1E683064709B00

