#ifndef CAMERA_FUNC_H
#define CAMERA_FUNC_H

#include "utiltypedef.h"
#include "fs.h"
#include "record.h"

#define MIN_FREE_DISK_SPACE             10//500       // MB

typedef enum {
    CAMERA_RESOLUTION_640x480,
    CAMERA_RESOLUTION_1024x768,
    CAMERA_RESOLUTION_1280x1024,
    CAMERA_RESOLUTION_1600x1200,
    CAMERA_RESOLUTION_2048x1536,
    CAMERA_RESOLUTION_2560x1920,
    CAMERA_RESOLUTION_MAX
} E_CAMERA_RESOLUTION;

SDWORD Camera_PreviewStart(E_CAMERA_RESOLUTION resolution);
SDWORD Camera_PreviewStop();
SDWORD Camera_Capture(E_CAMERA_RESOLUTION resolution);
#endif

