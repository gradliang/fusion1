#include "global612.h"
#include "mpTrace.h"

#ifndef CP1251_H_FC626D5D7F51152F21587CDC84A815CC
#define CP1251_H_FC626D5D7F51152F21587CDC84A815CC

#if CHARSET_CP1251_ENABLE

BOOL cp1251_isLeadByte(BYTE ch);
int cp1251_ToUTF16(const BYTE* pbByteString, WORD* pwWordBuffer, DWORD dwWordBufSize, WORD wDefaultChar);	
int cp1251_FromUTF16(const WORD* pwWordString, BYTE* pbByteBuffer, DWORD dwByteBufSize, BYTE bDefaultChar);

#endif	//
#endif	//CP1251_H_FC626D5D7F51152F21587CDC84A815CC

