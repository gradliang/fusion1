#ifndef __SYSTEM_ISP_H_
#define __SYSTEM_ISP_H_

#include "iplaysysconfig.h"

///@defgroup    ISP_MODULE
#include "flashrep.h"


///@ingroup     ISP_MODULE
///@brief       0x4D504150 is MPAP ascii code.
#define AP_TAG          0x4D504150  // MPAP

///@ingroup     ISP_MODULE
///@brief       0x4D505253 is MPRS ascii code.
#define RES_TAG         0x4D505253  // MPRS

///@ingroup     ISP_MODULE
///@brief       0x4D505354 is MPST ascii code.
#define USER_SET_TAG    0x4D505354  // MPST

///@ingroup     ISP_MODULE
///@brief       0x4D504654 is MPFT ascii code.
#define FACTORY_SET_TAG 0x4D504654  // MPFT

#if BLUETOOTH == ENABLE
///@ingroup     ISP_MODULE
///@brief       0x4D504254 is MPBT ascii code.
#define BT_TAG          0x4D504254     //MPBT
#endif

#if ISP_FUNC_ENABLE
int IspFunc_Write(BYTE *data, DWORD len, DWORD tag);
DWORD IspFunc_ReadAP(BYTE *buf, DWORD size);
BYTE *IspFunc_ReadRESOURCE(DWORD res_type, BYTE *pbTarget, DWORD dwSize);
DWORD IspFunc_GetRESOURCESize(DWORD dwTag);
int IspFunc_WriteUST(BYTE *pdwTempMen, DWORD buf_size);
DWORD IspFunc_ReadUST(BYTE *pdwTempMen, DWORD buf_size);
int IspFunc_WriteFST(BYTE *pdwTempMen, DWORD buf_size);
DWORD IspFunc_ReadFST(BYTE *pdwTempMen, DWORD buf_size);

DWORD ISP_GetUserBlockSize(DWORD dwTag);
DWORD ISP_GetUserBlock(BYTE *buf, DWORD size, DWORD tag);
int ISP_UpdateUserBlock(BYTE *buf, DWORD size, DWORD tag);
#endif

#if (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_NAND))
int NandEraseAll(DWORD ValidOnly);
void ISP_NandInit(DWORD apCodeAreaSize);
int ISP_NandBlockRegister(DWORD dwTag, DWORD dwAddr);
void ISP_SetNandReservedSize(DWORD revSize);
DWORD IspFunc_Read(BYTE *buf, DWORD size, DWORD tag);
DWORD ISP_GetNandReservedSize(void);
#endif

#endif

