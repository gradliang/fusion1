#include "global612.h"
#include "mpTrace.h"

#ifndef BIG5_H_0232178AA847C9F498431878CA9B80CD
#define BIG5_H_0232178AA847C9F498431878CA9B80CD

#if CHARSET_BIG5_ENABLE

BOOL big5_isLeadByte(BYTE ch);
int big5_ToUTF16(const BYTE* pbByteString, WORD* pwWordBuffer, DWORD dwWordBufSize, WORD wDefaultChar);	
int big5_FromUTF16(const WORD* pwWordString, BYTE* pbByteBuffer, DWORD dwByteBufSize, BYTE bDefaultChar);

#endif	//
#endif	//BIG5_H_0232178AA847C9F498431878CA9B80CD

