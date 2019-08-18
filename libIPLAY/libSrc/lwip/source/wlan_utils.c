#define LOCAL_DEBUG_ENABLE 0

#include <stdio.h>
#include <stdarg.h>
#include "global612.h"
#include "typedef.h"
#include "mpTrace.h"
#include "typedef.h"
#include "os_mp52x.h"
#include "os.h"
#include "ndebug.h"

/* 
 * ascii_to_hex
 *
 * Convert an ASCII string to a hexidecimal digit string.  For each character
 * in the ASCII string, convert each ASCII character to a two-digit hex number.
 * 
 * The <a> and <h> parameters can point to the same memory location.
 */
char * ascii_to_hex (char *a, char *h, int len)
{
    BYTE *ptr, *ptr_save, val;
    char *outp = h;
    int i;

    MP_DEBUG1("ascii_to_hex %d", len);
    ptr = (BYTE *)mpx_Malloc(len + 1);
    if (ptr == NULL)
        return NULL;
    ptr_save = ptr;
    strcpy(ptr, a);

    MP_DEBUG("ascii_to_hex 2");
    for (i=0; i < len; i++)
    {
        val = *ptr;
        val = (val >> 4) & 0x0f;
        if (val <= 9)
            *outp++ = val + '0';
        else
            *outp++ = val - 10 + 'a';

        val = *ptr++;
        val = val & 0x0f;
        if (val <= 9)
            *outp++ = val + '0';
        else
            *outp++ = val - 10 + 'a';
    }
    MP_DEBUG("ascii_to_hex 3");

    *outp = '\0';
    mpx_Free(ptr_save);
    MP_DEBUG("ascii_to_hex 4");
    return h;
}

/* 
 * hex_to_ascii
 *
 * Convert a hex digit string to ASCII string.  This function will convert each 
 * two-digit hex number back to an ASCII character.
 *
 * The <a> and <h> parameters can point to the same memory location.
 */
char * hex_to_ascii (char *h, char *a,int len)
{
    BYTE *ptr, *ptr_save, val, abyte;
    BYTE *outp = a;
    int i;
    ptr = (BYTE *)mpx_Malloc(len + 1);
    if (ptr == NULL)
        return NULL;
    ptr_save = ptr;
    strcpy(ptr, h);

    for (i=0; i<len;i++)
    {
        val = *ptr++;
        if (val <= '9')
            val = val - '0';
        else if (val <= 'f')
            val = val - 'a' + 10;
        else if (val <= 'F')
            val = val - 'A' + 10;
        else
            ;
        abyte = (abyte << 4) + (val & 0x0f);
        if (i & 1)
        {
            *outp++ = abyte;
            abyte = 0;
        }
    }

    *outp = '\0';

    mpx_Free(ptr_save);
    return a;
}

