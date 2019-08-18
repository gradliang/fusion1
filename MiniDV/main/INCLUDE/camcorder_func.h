#ifndef CAMCORDER_FUNC_H
#define CAMCORDER_FUNC_H

#include "utiltypedef.h"
#include "fs.h"
#include "record.h"

typedef enum {
    CAMCORDER_RESOLUTION_CIF,    // 352 x 240 @ 30
    CAMCORDER_RESOLUTION_VGA_24,    // 640 x 480 @ 24
    CAMCORDER_RESOLUTION_VGA_25,    // 640 x 480 @ 25
    CAMCORDER_RESOLUTION_VGA,    // 640 x 480 @ 30
    CAMCORDER_RESOLUTION_480P,  // 720 x 480 @ 30
    CAMCORDER_RESOLUTION_800x480,  // 800 x 480 @ 20
    CAMCORDER_RESOLUTION_SVGA,  // 800 x 600 @ 20
    CAMCORDER_RESOLUTION_XGA,   // 1024 x 768 @ 30
    CAMCORDER_RESOLUTION_720P, // 1280 x 720 @ 24    
    CAMCORDER_RESOLUTION_MAX
} E_CAMCORDER_TIMING;

SDWORD Camcorder_PreviewStart(E_CAMCORDER_TIMING recordTiming);
SDWORD Camcorder_PreviewStop(void);
SDWORD Camcorder_RecordStart(BOOL infiniteRecordMode);
SDWORD Camcorder_RecordStop(void);
SDWORD Camcorder_RecordPause(void);
SDWORD Camcorder_RecordResume(void);

#if RECORD_AUDIO
SDWORD Audio_RecordStart(BOOL infiniteRecordMode);
SDWORD Audio_RecordStop(void);
#endif

#endif

