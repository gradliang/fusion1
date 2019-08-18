#ifndef UI_FILE_SYSTEM_H
#define UI_FILE_SYSTEM_H

#include "utiltypedef.h"
#include "fs.h"

STREAM *UI_FileSystem_AutoNameFileCreate(WORD *folderPath, WORD *fileNamePreFix, BYTE *extendFileName);
SDWORD UI_FileSystem_EarliestFileRemove(WORD *folderPath, WORD *fileNamePreFix, BYTE *extendFileName);
#endif

