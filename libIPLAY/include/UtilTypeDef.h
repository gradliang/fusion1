
#ifndef __TYPEDEF_H
#define __TYPEDEF_H

///
///@defgroup    TYPE_DEFINE             Type define
///

#include <sys/types.h>

///
///@ingroup     TYPE_DEFINE
///@brief       define \b LONG as <b>unsigned long long</b>
typedef unsigned long long  LONG;

#ifndef _DWORD
#define _DWORD
///
///@ingroup     TYPE_DEFINE
///@brief       define \b DWORD as <b>unsigned long</b>
typedef unsigned long       DWORD;
#endif

#ifndef _WORD
#define _WORD
///
///@ingroup     TYPE_DEFINE
///@brief       define \b WORD as <b>unsigned short</b>
typedef unsigned short      WORD;
#endif

#ifndef _BYTE
#define _BYTE
///
///@ingroup     TYPE_DEFINE
///@brief       define \b BYTE as <b>unsigned char</b>
typedef unsigned char       BYTE;
#endif

#ifndef _BOOL
//#define _BOOL
///
///@ingroup     TYPE_DEFINE
///@brief       define \b BOOL as <b>unsigned int</b>
typedef unsigned int        BOOL;
#endif

///
///@ingroup     TYPE_DEFINE
///@brief       define \b SLONG as <b>long long</b>
typedef long long           SLONG;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b SDWORD as \b long
typedef long                SDWORD;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b SWORD as \b short
typedef short               SWORD;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b SBYTE as \b char
typedef char                SBYTE;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b uint8_t as <b>unsigned char</b>
typedef unsigned char       uint8_t;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b uint16_t as <b>unsigned short</b>
typedef unsigned short      uint16_t;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b uint32_t as <b>unsigned long</b>
typedef unsigned long       uint32_t;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b uint64_t as <b>unsigned long long</b>
typedef unsigned long long  uint64_t;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b int8_t as <b>signed char</b>
typedef signed char         int8_t;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b int16_t as <b>signed short</b>
typedef signed short        int16_t;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b int32_t as <b>signed long</b>
typedef signed long         int32_t;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b int64_t as <b>signed long long</b>
typedef signed long long    int64_t;

typedef unsigned char      u8_t;
typedef signed char        s8_t;
typedef unsigned short    u16_t;
typedef signed   short    s16_t;
typedef unsigned long    u32_t;
typedef long                  s32_t;
typedef unsigned long long  u64_t;

#ifdef FALSE
#undef FALSE
#endif

///
///@ingroup     TYPE_DEFINE
///@brief       define \b FALSE as \b 0
#define FALSE 0

#ifdef TRUE
#undef TRUE
#endif

///
///@ingroup     TYPE_DEFINE
///@brief       define \b TRUE as \b 1
#define TRUE  1

#ifdef NULL
#undef NULL
#endif

///
///@ingroup     TYPE_DEFINE
///@brief       define \b NULL as \b 0
#define NULL  0

typedef struct{
    BYTE Y0;
    BYTE Y1;
    BYTE Cb;
    BYTE Cr;
} ST_PIXEL;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b U08 as <b>unsigned char</b>
typedef unsigned char       U08;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b U16 as <b>unsigned short</b>
typedef unsigned short      U16;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b U32 as <b>unsigned long</b>
typedef unsigned long       U32;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b U64 as <b>unsigned long long</b>
typedef unsigned long long  U64;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b S08 as <b>signed char</b>
typedef signed char         S08;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b S16 as <b>signed short</b>
typedef signed short        S16;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b S32 as <b>signed long</b>
typedef signed long         S32;

///
///@ingroup     TYPE_DEFINE
///@brief       define \b S64 as <b>signed long long</b>
typedef signed long long    S64;

#endif    // __TYPEDEF_H


