#include "iplaysysconfig.h"


#ifndef Motion_Detection_Calc_threshold
	#define Motion_Detection_Calc_threshold 20
#endif

#ifndef NoMotion_threshold
	#define NoMotion_threshold 2
#endif

#ifndef NoMotion_count_threshold
	#define NoMotion_count_threshold        24*3
#endif

#ifndef NEW_MOTION_DECTION_DELAY_FRAME
	#define NEW_MOTION_DECTION_DELAY_FRAME 24*3
#endif

#ifndef MOTION_DETECTION_DELAY_FRAME_NUM
    #define MOTION_DECTION_DELAY_FRAME        100
#endif

#define MAX_sensor_IMGSIZE_ID SIZE_2M_1600x1200

//Max HW Scaler width = 1600
#define MAX_Scale_IMGSIZE_ID  SIZE_SVGA_800x600
#define MODE_PREVIEW 0
#define MODE_CAPTURE 1
#define MODE_RECORD  2
#define MODE_PCCAM   3

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

#define Format_YYUV 0
#define Format_YUYV 1

/*Define Sensor frame rate */
typedef enum {
	SENSOR_FRAME_RATE_30FPS = 30,
	SENSOR_FRAME_RATE_35FPS = 35,
	SENSOR_FRAME_RATE_40FPS = 40,
	SENSOR_FRAME_RATE_45FPS = 45,
	SENSOR_FRAME_RATE_50FPS = 50,
	SENSOR_FRAME_RATE_55FPS = 55,
	SENSOR_FRAME_RATE_60FPS = 60
} E_SENSOR_FRAME_RATE;


/*Define CVBS zoom step level*/
typedef enum {
  SENSOR_ZOOM_LEVEL_1_00 = 0,
  SENSOR_ZOOM_LEVEL_1_33 = 1,
  SENSOR_ZOOM_LEVEL_1_66 = 2,
  SENSOR_ZOOM_LEVEL_2_00 = 3,
} E_SENSOR_ZOOM_LEVEL;

//#if (SENSOR_USB_SHARE_MEM  == ENABLE)
struct SENSOR_SHARE_BUF {
  DWORD *Buf;
  BYTE Empty_Flag;/*0: full, 1=empty.*/
};



//void Global_HW_Path_Set(int PathSelect);
int API_Sensor_Initial(void);
void API_Sensor_Stop(void);
//void Global_Sensor_Run(ST_IMGWIN * trgWin);
int API_SetSensorWindow(WORD Win_PosX, WORD Win_PosY,WORD Win_Width, WORD Win_Height);
int API_Sensor_Capture(STREAM *fileHandle, BYTE Capture_IMGSIZE_ID);

#if(SENSOR_TYPE_CVBS_INPUT == ENABLE)
/*ex. 480i (720x480). need to set InputWidth=720, InputHeight =480*/

#endif
