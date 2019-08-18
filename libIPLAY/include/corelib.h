#ifndef __CORE_LIBRARY_H__
#define __CORE_LIBRARY_H__

#include "flagdefine.h"

// WARNING!!!!
//
// You can not  write any comments at the tail of define flag.
// It will effect makefile's judge
//
// -------------------------------------------------
// UI Framework
// -------------------------------------------------
#define MAKE_XPG_PLAYER     1

// -------------------------------------------------
// DISPLAY
// -------------------------------------------------
#define YUV444_ENABLE 0
#if YUV444_ENABLE
#define SET_FIXED_RATIO_444_422     0
#endif


#define EDITINFO_SAVETOJPEG			0
#if EDITINFO_SAVETOJPEG
#define ROTATE_OP				1
#define FRAMES_OP				2
#define EFFECT_OP				3
#endif
// -------------------------------------------------
// VIDEO
// -------------------------------------------------
// MJPEG_TOGGLE -- 0: MJPEG decoder use VIDEO_DECODE_TASK, MJPEG decoder use MJPEG_DATAIO_TASK
#define MJPEG_TOGGLE        0

#define PURE_VIDEO 0
//#if (STD_BOARD_VER == MP650_FPGA)
//Read the next video packet while HW decoder is running
#define READ_VIDEO_ON_DECODING 1
//Make (but not force) decoding audio start while HW decoder is running
#define DECODE_AUDIO_ON_DECODING_VIDEO 0
//#endif

// display video drop information in full screen mode
#define DISPLAY_VIDEO_DROP_INFO_FULL  0

// display video drop information in draw thumb mode
#define DISPLAY_VIDEO_DROP_INFO_THUMB 0

/* enable this if want to use file chain clusters caching for speed up video performance */
#define ENABLE_VIDEO_FILE_CHAIN_CLUSTERS_CACHING  0

// display flat video frame in preview mode
#define PREVIEW_FLAT_VIDEO_FRAME 1

//individual video codec enable
//include MPEG-1 & MPEG-2
#define	VCODEC_MPEG12_ENABLE	1
//include MPEG-4 & H.263
#define	VCODEC_MPEG4_ENABLE		1
#define	VCODEC_H264_ENABLE		1


//turn on bypass to dma mode whe video file play to pause
#define DISPLAY_PAUSE_BYPASS2DMA 0

// 0: can only display video with panel full size, 1: can display video with any size
#define DISPLAY_VIDEO_FILLWALL_ENABLE 0

// if DISPLAY_VIDEO_RATIO set to 0, display panel full size
// if DISPLAY_VIDEO_RATIO set to 1, display origne size
// if DISPLAY_VIDEO_RATIO set to 2, display 16:9
// if DISPLAY_VIDEO_RATIO set to 3, display 4:3
// if DISPLAY_VIDEO_RATIO set to 4, display origne scale  panel full size

#define DISPLAY_VIDEO_RATIO 0

//chang mode FullScreen to Thumb or Thumb to FullScreen
#define DISPLAY_CHANGMODE   1

// -------------------------------------------------
// AUDIO
// -------------------------------------------------


/*	**** Attention ****
 *
 * Please read comments before you change flags here or call help to audio software engineer !!!!!
 *
 */

#define AUDIO_ON			0
//Only one MP3 codec can be opened below, Iplaysysconfig.h is needed to change too
//SWMP3
#define MP3_SW_AUDIO		1
//MADMP3
#define MP3_MAD_AUDIO		0
//HWMP3
#define MP3_HW_CODEC		0

//Only one AAC codec can be opened below, Iplaysysconfig.h is needed to change too
#define AAC_FAAD_AUDIO		0
#define AAC_SW_AUDIO		0

#define WMA_AV				0
#define AC3_AV				0


#define WMA_ENABLE			0
#define OGG_ENABLE			0
#define RAM_AUDIO_ENABLE	0
#define AC3_AUDIO_ENABLE	0
#define WAV_ENABLE			1

#define AMR_ENABLE			0
// Open this flag for activating AMR encoder
#define AMR_ENCODE_ENABLE	0
#define AUDIO_SW_VOLUME_DOWN 0



/*************************************************************************************/

// -------------------------------------------------
// BLUETOOTH
// -------------------------------------------------
#define MAKE_BLUETOOTH			0
//please define BT USB port in Platform_MP6xx.h.

// -------------------------------------------------
// WIFI
// -------------------------------------------------
#define NETWARE_ENABLE			0

//WIFI UTILITY 0:NONE 1:WPA
#define Make_WPA 				0
//WIFI PROTECTED SETUP
#define Make_WPS 				0
// -------------------------------------------------
// NETWORK FUNCTION
// -------------------------------------------------
//NETWORK TCP/IP 0:NONE 1:LWIP
#define Make_LWIP 				0
//NETWORK HTTP 0:NONE 1:ENABLE
#define Make_CURL 				0
//NETWORK
#define Make_OPENSSL 			0
//UPNP
#define Make_UPNP_SDK 			0
//Netstream
#define Make_NETSTREAM 			0
//SMTP
#define Make_SMTP 				0
//ADHOC
#define Make_ADHOC 				0

#define Make_RTSP 				0
//-------------------------------------------------
// ETHERNET INTERFACE
//-------------------------------------------------
#define Make_DM9KS_ETHERNET		0
#define Make_DM9621_ETHERNET	0
//-------------------------------------------------
// WIFI INTERFACE
//-------------------------------------------------
//SDIO 0:NONE 1:MARVELL 2:REALTEK
#define Make_SDIO 				0
//USB_WIFI 0:NONE 1:RALINK 2:REALTEK 3:AR2524 4:RALINK+AR2524 5:AR9271 7:REALTEK8188CU
#define Make_USB	 			0
//please define WIFI USB port in Platform_MP6xx.h.

//-------------------------------------------------
// USB MODEM INTERFACE
//-------------------------------------------------
#define Make_MODEM		0

//-------------------------------------------------
// osip/eXosip libraries
//-------------------------------------------------
#define Make_OSIP		0

//-------------------------------------------------
// sk_buff optimization
//-------------------------------------------------
//#define SKB_OPT
#undef SKB_OPT

//-------------------------------------------------
// Ebook  INTERFACE if EREADER_ENABLE disable, set 0 to UNRTF/ANTIWORD/LRF/ZLIB/EPUB/PDF/DUMP_PIC/...
//-------------------------------------------------
#define EREADER_ENABLE		0

// Use TTF font or Bitmap font - only one can 1 at one time!!!
#define Make_TTF		            0 //True type font engine - default
//#define READERDISPLAY_ENABLE		0 //USE FONT  BITMAP for reader

#define Make_TXT		0
#define Make_EPUB		0
#define Make_PDF		0
#define BOOK_DUMP_PIC	0	//0: no picture dump   1: dump as a file    2: dump as offset

#define DEFAULT_MULTI_LANGUAGE   0

// -------------------------------------------------
// Misc
// -------------------------------------------------
#define DEMO_AUTO  0
// For Test console
#define Make_TESTCONSOLE 	0
#define Make_DIAGNOSTIC_TC 	0

#define HW_IIC_MASTER_ENABLE        0
#define HW_IIC_SLAVE_ENABLE         0

#define Make_EMBEDDED_RTC           1

//Make_RELEASE ------- 0: full   1: Internal release   2: BSP release
#define Make_RELEASE		0

#endif //__CORE_LIBRARY_H__

