/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      video_out.h
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: 
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Joshua Lu    first file
****************************************************************
*/

#ifndef __VIDEO_OUT__
#define __VIDEO_OUT__ 

// typedef unsigned long   DWORD;
#include "UtilTypeDef.h"

typedef struct vo_info_s
{
        /* driver name ("Matrox Millennium G200/G400" */
        const char *name;
        /* short name (for config strings) ("mga") */
        const char *short_name;
        /* author ("Aaron Holtzman <aholtzma@ess.engr.uvic.ca>") */
        const char *author;
        /* any additional comments */
        const char *comment;
} vo_info_t;

typedef struct vo_functions_s
{
        vo_info_t *info;
        /*
         * Preinitializes driver (real INITIALIZATION)
         *   arg - currently it's vo_subdevice
         *   returns: zero on successful initialization, non-zero on error.
         */
        uint32_t (*preinit)(const char *arg);
        /*
         * Initialize (means CONFIGURE) the display driver.
         * params:
         *   width,height: image source size
         *   d_width,d_height: size of the requested window size, just a hint
         *   fullscreen: flag, 0=windowd 1=fullscreen, just a hint
         *   title: window title, if available
         *   format: fourcc of pixel format
         * returns : zero on successful initialization, non-zero on error.
         */
        uint32_t (*config)(DWORD width, DWORD height, DWORD d_width,
                         DWORD d_height, DWORD fullscreen, char *title,
                         DWORD format);

        /*
         * Control interface
         */
        DWORD (*control)(DWORD request, void *data, ...);

        /*
         * Display a new RGB/BGR frame of the video to the screen.
         * params:
         *   src[0] - pointer to the image
         */
        DWORD (*draw_frame)(uint8_t *src[]);
        /*
         * Draw a planar YUV slice to the buffer:
         * params:
         *   src[3] = source image planes (Y,U,V)
         *   stride[3] = source image planes line widths (in bytes)
         *   w,h = width*height of area to be copied (in Y pixels)
         *   x,y = position at the destination image (in Y pixels)
         */
        DWORD (*draw_slice)(uint8_t *src[], int stride[], int w,int h, int x,int y);

        /*
         * Draws OSD to the screen buffer
         */
        void (*draw_osd)(void);

        /*
         * Blit/Flip buffer to the screen. Must be called after each frame!
         */
        void (*flip_page)(void);

        /*
         * This func is called after every frames to handle keyboard and
         * other events. It's called in PAUSE mode too!
         */
        void (*check_events)(void);

        /*
         * Closes driver. Should restore the original state of the system.
         */
        void (*uninit)(void);

} vo_functions_t;

typedef struct video_out {
  int   vo_pts;
  float vo_fps;
  vo_functions_t video_out;
} video_out_t;

extern video_out_t* init_best_video_out(void* sh_video);

#endif
