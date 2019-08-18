#ifndef __HAL_DMA_C
#define __HAL_DMA_C

#include "iplaysysconfig.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))

///@defgroup    MMCP_MODULE     Memory memory copy module.
///These APIs are used for memory copy to memory by using DMA.


///@ingroup     MMCP_MODULE
///@brief       Using ISR scheme to copy block of memory.
///
///@param       *destAddr     Pointer to the destination array where content is to be copied.
///@param       *srcAddr      Pointer to the source of data to be copied.
///@param       lens          Number of bytes to copy.
///
///@retval      PASS          Memory to memory copy successfully.
///@retval      FAIL          Memory to memory copy fail. The reason is OS event time out.
///
///@remark      Copy the values to lens bytes from the location pointed by srcAddr directly to the memory
///             block pointed by destAddr.
///
SDWORD mmcp_memcpy(BYTE* const destAddr, const BYTE* const srcAddr, DWORD lens);

///@ingroup     MMCP_MODULE
///@brief       Using polling scheme to copy block of memory.
///
///@param       *destAddr     Pointer to the destination array where content is to be copied.
///@param       *srcAddr      Pointer to the source of data to be copied.
///@param       lens          Number of bytes to copy.
///
///@retval      PASS          Memory to memory copy successfully.
///@retval      FAIL          Memory to memory copy fail. The reason is OS event time out.
///
///@remark      Copy the values to lens bytes from the location pointed by srcAddr directly to the memory
///             block pointed by destAddr.
///
SDWORD mmcp_memcpy_polling(BYTE* const destAddr, const BYTE* const srcAddr, DWORD lens);

///@ingroup     MMCP_MODULE
///@brief       Fill block of memory by a 32-bit value.
///
///@param       *addr       Byte pointer to the block of memory to fill.
///@param       value       32 bits value to be set.
///@param       lens        Number of bytes to be set to the specified value.
///
///@retval      PASS        Memory set to value successfully.
///@retval      FAIL        Memory set to value fail. The reason is OS event time out.
///
///@remark      Set the lens bytes of specified value to the memory block pointed by addr.
///
SDWORD mmcp_memset_u32(BYTE* addr, DWORD value, DWORD lens);

///@ingroup     MMCP_MODULE
///@brief       Fill block of memory by a 8-bit value.
///
///@param       *addr       Byte pointer to the block of memory to fill.
///@param       value       8 bits value to be set.
///@param       lens        Number of bytes to be set to the specified value.
///
///@retval      PASS        Memory set to value successfully.
///@retval      FAIL        Memory set to value fail. The reason is OS event time out.
///
///@remark      Set the lens bytes of specified value to the memory block pointed by addr.
///
SDWORD mmcp_memset(BYTE* addr, BYTE value, DWORD lens);

///@ingroup     MMCP_MODULE
///@brief       Using ISR scheme to copy block of memory.
///
///@param       *srcAddr    Pointer to the source of data to be copied.
///@param       *destAddr   Pointer to the destination array where content is to be copied.
///@param       row      box height to be copied is byte.
///@param       col       box width to be cpied is byte.
///@param       sLineOffset The line offset of source buffer, unit is byte.
///@param       dLineOffset The line offset of destination buffer, unit is byte.
///
///@retval      PASS        Memory to memory copy successfully.
///@retval      FAIL        Memory to memory copy fail. The reason is OS event time out.
///
///@remark      Copy the values to lens bytes from the location pointed by srcAddr directly to the memory
///             block pointed by destAddr.
///
SDWORD mmcp_block(const BYTE * const srcAddr, BYTE * const destAddr, DWORD row, DWORD col, DWORD sLineOffset, DWORD dLineOffset);

SDWORD mmcp_block_polling(const BYTE * const srcAddr, BYTE * const destAddr, DWORD row, DWORD col, DWORD sLineOffset, DWORD dLineOffset);

#else

#define mmcp_memcpy                     memcpy
#define mmcp_memcpy_polling             memcpy
#define mmcp_memset_u32                 memset
#define mmcp_memset                     memset

#endif


#endif

