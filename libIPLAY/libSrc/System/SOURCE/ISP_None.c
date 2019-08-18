/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "Display.h"

#if (ISP_FUNC_ENABLE == DISABLE)

int IspFunc_Write(BYTE *data, DWORD len, DWORD tag)
{
    return FAIL;
}



DWORD IspFunc_ReadAP(BYTE *buf, DWORD size)
{
    return FAIL;
}



DWORD IspFunc_ReadRES(BYTE *buf, DWORD size)
{
    return FAIL;
}



BYTE *IspFunc_ReadRESOURCE(DWORD res_type, BYTE *pbTarget, DWORD dwSize)
{
    return NULL;
}



///
///@ingroup MAIN
///@brief   Get the size of specific resource
///
///@param   DWORD dwTag  the tag of resource to find
///
///@retval  If the resource found, return the size of the resource, else return 0.
///
DWORD IspFunc_GetRESOURCESize(DWORD dwTag)
{
    return 0;
}



int IspFunc_WriteUST(BYTE *pdwTempMen, DWORD buf_size)
{
    return 0;
}



DWORD IspFunc_ReadUST(BYTE *pdwTempMen, DWORD buf_size)
{
    return 0;
}



int IspFunc_WriteFST(BYTE *pdwTempMen, DWORD buf_size)
{
    return 0;
}



DWORD IspFunc_ReadFST(BYTE *pdwTempMen, DWORD buf_size)
{
    return 0;
}


#endif

