
#define LOCAL_DEBUG_ENABLE 0


#include "global612.h"
#include "mpTrace.h"
//#include "text.h"

U16 mpx_Utf8ToUnicode(void ** string)
{
    U08 *utf8_point, word_count;
    U16 unicode;

    utf8_point = *string;
    unicode = *utf8_point;
    utf8_point++;
    
    if (unicode < 0x80)
    {
        *string = utf8_point;
        return unicode;
    }
    
    if ((unicode & 0xf0) == 0xe0)
        word_count = 2;
    else if ((unicode & 0xe0) == 0xc0)
    {
        unicode &= 0x1f;
        word_count = 1;
    }
    
    while (word_count)
    {
        unicode <<= 6;
        unicode += *utf8_point;
        unicode -= 0x80;
        utf8_point++;
        word_count--;
    }
    
    *string = utf8_point;

    return unicode;
}

///
///@ingroup UTILITY
///@brief   Utility for converting the UTF-8 string to UTF-16 Unicode string.
///
///@param   target    Target UTF-16 Unicode string.
///@param   source    Source UTF-8 string with 0x00 null terminator.
///
///@return  Target string pointer.
///
///@remark  This utility assumes that the source is a limited null-terminated string. A string
///         without a null at the end of the string will cause unpreditable result.
///
U16 *mpx_UtilUtf8ToUnicodeU16(U16 * target, U08 * source)
{
    U16 *result;
    U08 i, *temp_str = source;

    result = target;
    i = 0;
    while (*temp_str)
    {
        target[i++] = mpx_Utf8ToUnicode(&temp_str); //note: mpx_Utf8ToUnicode will increment the pointer value to next byte/word !
    }
    target[i] = 0; //null terminator

    return result;
}

