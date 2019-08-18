#include "global612.h"
#include "mpTrace.h"

#ifndef SHIFTJIS_H_C95864CBEE4F161B8299FF15EB167F18
#define SHIFTJIS_H_C95864CBEE4F161B8299FF15EB167F18

#if CHARSET_SHIFT_JIS_ENABLE

BOOL shift_jis_isLeadByte(BYTE ch);
int shift_jis_ToUTF16(const BYTE* pbByteString, WORD* pwWordBuffer, DWORD dwWordBufSize, WORD wDefaultChar);	
int shift_jis_FromUTF16(const WORD* pwWordString, BYTE* pbByteBuffer, DWORD dwByteBufSize, BYTE bDefaultChar);

#endif	//
#endif	//SHIFTJIS_H_C95864CBEE4F161B8299FF15EB167F18

