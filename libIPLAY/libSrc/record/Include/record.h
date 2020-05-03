
#ifndef __G_ARGUMENT_RECORD_H
#define __G_ARGUMENT_RECORD_H

#include "utiltypedef.h"
#include "fs.h"
/*--Resolution--*/
#define r_720p   0
#define r_VGA    1
#define r_QCIF   2
#define r_QVGA   3
#define r_CIF    4
#define r_D1     5


/*--fps--*/
#define r_fps_60  60
#define r_fps_50  50
#define r_fps_30  30
#define r_fps_24  24
#define r_fps_20  20
#define r_fps_16  16
#define r_fps_12  12
#define r_fps_6   6

/*--quantification--*/
#define r_hight   0
#define r_midlle  1
#define r_low     2
/*--RecordingAudio--*/
#define r_open    0
#define r_close   1
/*---state---*/
#define Rec_StandBy_state       0
#define Recording_state     	1
#define Recording_stop_state    2
#define Recording_pause_state   3
#define Preview_stop_state      4
#define Preview_state       	5
#define ExceptionCheck_state    6

#define AudioRecording_state    7
#define AudioRecording_stop_state    8

/*---op_code---*/
#define VIDEO_REC_OP_PREVIEW_START      0
#define VIDEO_REC_OP_RECORDING_START    1
#define VIDEO_REC_OP_RECORDING_STOP     2
#define VIDEO_REC_OP_PREVIEW_STOP       3
#define VIDEO_REC_OP_RECORDING_PAUSE    4
#define VIDEO_REC_OP_RECORDING_RESUME   5
#define VIDEO_REC_OP_RECORDING_EXCEPTION    6

#define AUDIO_REC_OP_RECORDING_READY    8
#define AUDIO_REC_OP_RECORDING_START    9
#define AUDIO_REC_OP_RECORDING_STOP      10
#define AUDIO_REC_OP_RECORDING_EXCEPTION    11

#define SIZE_QCIF_176x144 0
#define SIZE_QVGA_320x240 1
#define SIZE_CIF_352x240  2
#define SIZE_VGA_640x480  3
#define SIZE_480P_720x480 4
#define SIZE_SVGA_800x600 5
#define SIZE_XGA_1024x768 6
#define SIZE_720P_1280x720 7
#define SIZE_SXGA_1280x1024 8
#define SIZE_2M_1600x1200 9
#define SIZE_QXGA_2048x1536 10
#define SIZE_5M_2560x1920 11
#define SIZE_SVGA_800x480 12

//===============================
typedef enum {
    RESOLUTION_QCIF_60,
    RESOLUTION_CIF_30,
    RESOLUTION_480P_30,
    RESOLUTION_800x600_15FPS,
    RESOLUTION_800x600,
    RESOLUTION_XGA_30,
    RESOLUTION_QVGA_60,
    RESOLUTION_CIF_60,
    RESOLUTION_VGA_60,
    RESOLUTION_VGA_30,
    RESOLUTION_VGA_26,
    RESOLUTION_VGA_25,
    RESOLUTION_VGA_24,
    RESOLUTION_VGA_16,
    RESOLUTION_D1_60,
    RESOLUTION_D1_50,
    RESOLUTION_720P_24,
    RESOLUTION_720P_20,
    RESOLUTION_720P_16,
    RESOLUTION_720P_6,
} E_RESOLUTION_REC;




typedef struct{
WORD  resolution;
STREAM *handle;
WORD   fps;
WORD   image_size;
WORD  quantification;
WORD  movie_length;
WORD  RecordingAudio;//r_open=0 r_close=1
//WORD  sensor_buffer_num;
E_DRIVE_INDEX_ID driveIndex;
}record_argument;

record_argument RecordArgumentGet(void);
int RecordTaskStattusGet(void);
//void SensorApplication(void);

#endif


