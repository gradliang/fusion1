
#include "global612.h"
#include "mpTrace.h"

#ifndef CP1252_H_8589DA28EFFF895F1E81536EA38CDAF3
#define CP1252_H_8589DA28EFFF895F1E81536EA38CDAF3

#if CHARSET_CP1252_ENABLE

BOOL cp1252_isLeadByte(BYTE ch);
int cp1252_ToUTF16(const BYTE* pbByteString, WORD* pwWordBuffer, DWORD dwWordBufSize, WORD wDefaultChar);	
int cp1252_FromUTF16(const WORD* pwWordString, BYTE* pbByteBuffer, DWORD dwByteBufSize, BYTE bDefaultChar);

#endif	//
#endif	//CP1252_H_8589DA28EFFF895F1E81536EA38CDAF3


