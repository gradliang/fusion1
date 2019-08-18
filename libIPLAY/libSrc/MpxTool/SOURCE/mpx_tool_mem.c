/* ========================================================================== */
/*                                                                            */
/*   mpx_tool_mem.c                                                           */
/*   (c) 2010 Author                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */
#define LOCAL_DEBUG_ENABLE  0

#include "global612.h"
#include "mpTrace.h"
#include "ui.h"
#include "taskid.h"
#include "mpx_tool.h"

#ifdef MAGIC_SENSOR_INTERFACE_ENABLE

BYTE *start_addr_PC_tool = NULL;

void mpx_toolInit()
{
    start_addr_PC_tool = (BYTE *) ext_mem_malloc(TOOL_BUFFER_SIZE);

    if (start_addr_PC_tool == NULL) {
         MP_ALERT("--E-- Out of memory for %s()", __FUNCTION__);
         __asm("break 100");
    }
    else {
        start_addr_PC_tool = (BYTE *) (((DWORD) start_addr_PC_tool) & ~BIT29);      // pointer to cached buffer
        memset(start_addr_PC_tool, 0, TOOL_BUFFER_SIZE);
        MP_ALERT("%s -\r\nAllocate memory successfully for %s()", __FUNCTION__, __FUNCTION__);
        MP_ALERT("... start_addr_PC_tool = 0x%08X ...", start_addr_PC_tool);
    }
}



void mpx_toolRelease()
{
    if (start_addr_PC_tool)
        ext_mem_free(start_addr_PC_tool);

    start_addr_PC_tool = NULL;
}



void mpx_toolReload(void)
{
    EventSet(UI_EVENT, EVENT_MPXTOOL_RELOAD);
}

#endif

