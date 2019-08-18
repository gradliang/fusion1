#include "global612.h"
#include "mpTrace.h"

#ifndef CP1250_H_CF510AA9AADE3154B1C73DBCF3CC4751
#define CP1250_H_CF510AA9AADE3154B1C73DBCF3CC4751

#if CHARSET_CP1250_ENABLE

BOOL cp1250_isLeadByte(BYTE ch);
int cp1250_ToUTF16(const BYTE* pbByteString, WORD* pwWordBuffer, DWORD dwWordBufSize, WORD wDefaultChar);	
int cp1250_FromUTF16(const WORD* pwWordString, BYTE* pbByteBuffer, DWORD dwByteBufSize, BYTE bDefaultChar);

#endif	//
#endif	//CP1250_H_CF510AA9AADE3154B1C73DBCF3CC4751

