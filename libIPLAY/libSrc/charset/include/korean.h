#include "global612.h"
#include "mpTrace.h"

#ifndef KOREAN_H_0B0B7BBB8F769979D0748E63FF901C9C
#define KOREAN_H_0B0B7BBB8F769979D0748E63FF901C9C

#if CHARSET_CP949_ENABLE

BOOL cp949_isLeadByte(BYTE ch);
int cp949_ToUTF16(const BYTE* pbByteString, WORD* pwWordBuffer, DWORD dwWordBufSize, WORD wDefaultChar);	
int cp949_FromUTF16(const WORD* pwWordString, BYTE* pbByteBuffer, DWORD dwByteBufSize, BYTE bDefaultChar);

#endif	//
#endif	//KOREAN_H_0B0B7BBB8F769979D0748E63FF901C9C

