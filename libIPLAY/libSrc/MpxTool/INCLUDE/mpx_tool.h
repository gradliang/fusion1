#ifndef __MPX_TOOL_H
#define __MPX_TOOL_H

#include "utiltypedef.h"
#include "os.h"

#define TOOL_BUFFER_SIZE  12288

BYTE *start_addr_PC_tool;

void mpx_toolInit();
void mpx_toolReload(void);

#endif

