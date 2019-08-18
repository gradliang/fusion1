#ifndef __ISPFUNC_H_
#define __ISPFUNC_H_

#include "iplaysysconfig.h"
#include "system.h"

///@defgroup    ISP_MODULE

/*
///@defgroup    ISP_MODULE     ISP API description.
*/
///iPlay can do in system programming only if ISP_FUNC_ENABLE has been enable. ISP will update AP code
///,resource and setup table respectively according to AP_TAG, RS_TAG, USER_SET_TAG or FACTORY_SET_TAG.
///


///@ingroup     ISP_MODULE
///@brief       Update AP code or resource to on board storage.
///
///@param       handle     ISP file handle.
///
///@retval      return PASS if succeed, otherwise return FAIL
///
extern int ISP_UpDateCode(STREAM *handle);

///
///@ingroup ISP_MODULE
///@brief   Get the size of specific resource
///
///@param   dwTag     the tag of resource to find
///
///@retval  If the resource found, return the size of the resource, else return 0.
///
extern DWORD ISP_GetResourceSize(DWORD dwTag);

///
///@ingroup     ISP_MODULE
///@brief       Get the start address of specific resource.
///
///@param       dwTag       the tag of resource.
///@param       *pbTarget   the buffer to read in resource.
///@param       dwSize      the size of resource.
///
///@retval      If the resource found, return the address, else return 0.
///
///@remark      The function call just search resource in on board storage.
///             If we want to access it through DMA channel, we must move
///             these data stream to sdram before access.
///
extern BYTE *ISP_GetResource(DWORD dwTag, BYTE *pbTarget, DWORD dwSize);

///
///@ingroup     ISP_MODULE
///@brief       check whether setup table has been written back or not.
///
///@retval      return PASS if succeed, otherwise return FAIL
///
///@remark      If setup table had been chagned and g_bAniFlag is not zero, this function will
///             be added to UI timer process and every 100ms write back setup table. If IdleFlag
///             is small than 4, this function will be added to UI timer process and every 10ms
///             write back setup table.
///
extern int ISP_ChkWriteBackSetup(void);

///
///@ingroup     ISP_MODULE
///@brief       write setup table to on board storage.
///
///@retval      return PASS if succeed, otherwise return FAIL
///
///@remark
///
int ISP_WriteBackSetup(void);

///
///@ingroup     ISP_MODULE
///@brief       read user setup table from on board storage.
///
///@retval      return PASS if succeed, otherwise return FAIL
///
///@remark
///
int ISP_ReadSetup(void);

///
///@ingroup     ISP_MODULE
///@brief       check AP code in on board storage.
///
///@retval      return PASS if check sum is zero, otherwise return FAIL
///
///@remark
///
SWORD ISP_ChksumVerify(void);

#endif

