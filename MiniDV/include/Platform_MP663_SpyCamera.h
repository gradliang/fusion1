#ifndef PLATFORM_H
#define PLATFORM_H

#include "flagdefine.h"
#include "corelib.h"

#define ENJOY_FLASHLIGHT_PROJECT	1
// -------------------------------------------------
// System config
// -------------------------------------------------
#define CHIP_VER                (CHIP_VER_660 | CHIP_VER_B)
//#define STD_BOARD_VER           MP663_128BGA_16M_SDRAM        // Chip Version A
#define STD_BOARD_VER           MP663_144BGA_32M_SDRAM          // Chip Version B

// SerialRGB 320x240
//#define TCON_ID                 TCON_RGB8SerialMode_320x240
//#define PANEL_ID                PANEL_A025DL02
// Non panel
#define TCON_ID                 TCON_NONE // TCON_RGB24Mode_320x240 //TCON_NONE
#define PANEL_ID                PANEL_NONE // PANEL_A025DL02 //PANEL_NONE

#define	PRODUCT_FW_VERSION      "MP663_SpyCamera "
#define	PRODUCT_MODEL           "Spy Camera "
//#define Support_EPD

#define ISP_FUNC_ENABLE         ENABLE
#define ISP_FILENAME            "MP660"  /* note: length of ASCII string <= 8 to meet 8.3 short name (extension name is always "ISP") */

//#define BOOTUP_TYPE             BOOTUP_TYPE_NAND
#define BOOTUP_TYPE             BOOTUP_TYPE_SPI
//#define BOOTUP_TYPE             BOOTUP_TYPE_SD
//#define BOOTUP_TYPE             BOOTUP_TYPE_NOISP

#define WRITE_BOOTROM_CODE_TO_SPI   0
#define TF_DISK_ENCRYPT             0

#if (BOOTUP_TYPE == BOOTUP_TYPE_NAND)
#define SYSTEM_DRIVE            SYSTEM_DRIVE_ON_NAND
#define SYS_DRV_SIZE            4                       // unit is 256 KB
#define TOOL_DRV_SIZE           0                       // unit is 256 KB
#define DEFAULT_RES_ADDR        0x00600000              // start address of MPRS, that is the size of MPAP region
#define NAND_RESERVED_SIZE      (10 * 1024 * 1024)      // reserved 10MB for raw data type (ex: ISP)
#elif (BOOTUP_TYPE == BOOTUP_TYPE_SPI)
#define SYSTEM_DRIVE            SYSTEM_DRIVE_ON_SPI
#define SYS_DRV_SIZE            2                       // unit is 256 KB
#if TF_DISK_ENCRYPT
#define TOOL_DRV_SIZE           32
#else
#define TOOL_DRV_SIZE           0                       // SPI not support tool drive (Drive ID number issue)
#endif
//Need be adjusted according to code size, current setting is for 32Mb SPI
#define SPI_RESERVED_SIZE       0x001F0000              // reserved 3MB size for AP code and resource. This value should be 65536 (0x00010000) mutiples.
#define SPI_APCODE_SADDR        0x00010000              // first 64KB is reserved for boot code and setting table. This value should be 65536 mutiples
#define SPI_RESOCE_SADDR        0x00170000              // reserved 768KB for AP code , 192KB for resource. This value should be 65536 mutiples
#elif (BOOTUP_TYPE == BOOTUP_TYPE_SD)
#define SYSTEM_DRIVE            SYSTEM_DRIVE_ON_SD
#define SYS_DRV_SIZE            128                     // unit is 256 KB
#define TOOL_DRV_SIZE           0//8                       // unit is 256 KB
#else
#define SYSTEM_DRIVE            SYSTEM_DRIVE_NONE
#define SYS_DRV_SIZE            0                       // unit is 256 KB
#define TOOL_DRV_SIZE           0                       // unit is 256 KB
#undef ISP_FUNC_ENABLE
#define ISP_FUNC_ENABLE         DISABLE
#endif

#define EXFAT_ENABLE                0
#define CARD_DETECT_FUNC_ENABLE     0                   // 0: For embedded MicroSD case, no card detect function, 1: For normal SD
#define CARD_PROTECT_FUNC_ENABLE    0                   // 0: For embedded MicroSD case, no write protect function, 1: For normal SD

/* enable this if want to use clusters Allocation Bitmap mechanism for FAT12/16/32 file systems to speed up free clusters searching */
#define ENABLE_ALLOC_BITMAP_FOR_FAT121632   1

//Main crystal clock, in Hz/Sec
#define MAIN_CRYSTAL_CLOCK      12000000    // For real chip

//-------------------------------------------------
//  Debug Com Port for HUART
//-------------------------------------------------
// Depend on PCB schematic
// For MP615, always using DEBUG_PORT_HUART_A (UART and HUART are same interrupt index)
#define MP_TRACE_DEBUG_PORT     DEBUG_PORT_HUART_A

// HUART-A pin multiplexer elect, Only for MP65x/66x serial
// For MP65x serial
// HUARTA_PIN_MULTIPLEXER - ENABLE : Using KGPIO (when I2CM is enabled, using KGPIO-1/2)
//                        - DISABLE : Using GP (when I2CM is disabled, using GP-0/1)
// For MP66x serial
// HUARTA_PIN_MULTIPLEXER - ENABLE : Using UGPIO-0/1 (when I2CM is enabled, using UGPIO-0/1)
//                        - DISABLE : Using GP (when I2CM is disabled, using GP-0/1)
//
#if (STD_BOARD_VER == MP663_128BGA_16M_SDRAM)
#define HUARTA_PIN_MULTIPLEXER  DISABLE
#else
#define HUARTA_PIN_MULTIPLEXER  DISABLE  //ENABLE
#endif

//------------------------------------------------------------------
// Function
//------------------------------------------------------------------
#if NETWARE_ENABLE == ENABLE
#define YOUGOTPHOTO	1             /* support YouGotPhoto.com */
#define HAVE_LWIP	1             /* enable LWIP */
#define HAVE_WPA_SUPPLICANT 1  /* enable WPA_SUPPLICANT */
#if Make_NETSTREAM
#define HAVE_NETSTREAM 1
#else
#define HAVE_NETSTREAM 0
#endif
#define HAVE_CURL 1	           /* enable CURL */
#define HAVE_FRAMECHANNEL 1
#define HAVE_VTUNER 1
#define HAVE_FLICKR	1
#define HAVE_GCE 0
#define NET_UPNP 1
#define HAVE_SHUTTERFLY	0
#define HAVE_FRAMEIT 1
#define HAVE_SNAPFISH 1
#define HAVE_UPNP_MEDIA_SERVER 1
#define HAVE_YOUTUBE 0
#define HAVE_YOUKU3G 0
#define HAVE_FTP_SERVER        0

//USB_WIFI 0:NONE 1:RALINK 2:REALTEK 3:ATHEROS
#define USB_WIFI             3
#define Write_To_NAND 0
#define UART_EDGE			DISABLE  	// EDGE modem
#define PPP_ENABLE			DISABLE
#define USB_ARCH_URB		DISABLE

#define DM9KS_ETHERNET_ENABLE	0

#define POP3_TEST               0

#define HAVE_FACEBOOK 0

#define HAVE_HTTP_SERVER        0
#if ( HAVE_UPNP_MEDIA_SERVER == 1 || NET_UPNP == 1 ) && ( Make_UPNP_SDK == 0 )
#error "\n!!!Make_UPNP_SDK SHOULD BE SET TO 1!!!\n"
#endif
#endif

//------------------------------------------------------------------
// Touch controller
//------------------------------------------------------------------
#define TOUCH_CONTROLLER_ENABLE         DISABLE
#if (TOUCH_CONTROLLER_ENABLE)
    #define TOUCH_CONTROLLER            TOUCH_PANEL_DRIVER_AK4183 //TOUCH_PANEL_DRIVER_A043VL01
    #define TOUCH_CONTROLLER_TYPE       TOUCH_PANEL_TYPE_RESISTIVE //TOUCH_PANEL_TYPE_CAPACITIVE
    #define TOUCH_CONTROLLER_INT_PIN    GPIO_03
#endif

//------------------------------------------------------------------
// BT
//------------------------------------------------------------------
#if MAKE_BLUETOOTH == 1
#include "btsetting.h"
#endif

//------------------------------------------------------------------
// Font
//------------------------------------------------------------------
#define BMP_FONT_ENABLE             1

//If you have Bitmap data use
#define FONT_BIG5                   0   //Traditional Chinese
#define FONT_GB2                    0   //Simpific Chinese
#define FONT_JIS                    0   //Japanese
#define FONT_KSC                    0   //Korean
#define BITMAP_DATA                 (FONT_BIG5 | FONT_GB2 | FONT_JIS | FONT_KSC)
#define FONT_MAX_INDEX              1   // total font data size
#define DEFAULT_FONT_SIZE           12

#define UNKNOWN_CHAR                0x3F  // the unknown character is display "?" = 0x3F

// Modify the font structure to modularize
#define NEWFONTSTRCUCTURE           0

#if BITMAP_DATA
// Using the dynamic load data from NAND flash (for NAND only) Lighter 20090821
// Load all table and font data to DRAM if DYNALOADFONTDATA "0"
#define DYNALOADFONTDATA            1
// The font data is generate by "Get Bitmap Fonts"
#endif

#define FONTSHADOW                  0
#define FONTCOLORCHANGE             0

#if FONTCOLORCHANGE
// the display step is double of FONTCOLORCHANGESTEP
// suggest the FONTCOLORCHANGESTEP is DEFAULT_FONT_SIZE/2 or DEFAULT_FONT_SIZE/2
// need to check the structure "ST_FONT" to be 4 byte align
#define FONTCOLORCHANGESTEP         8
#define SETFONTCOLORSTEP            0
#if SETFONTCOLORSTEP
#define FONTCOLOR0      0xff0000        // Data is RRGGBB
#define FONTCOLOR1      0xffff00        // Data is RRGGBB
#define FONTCOLOR2      0x00ff00        // Data is RRGGBB
#define FONTCOLOR3      0x00ffff        // Data is RRGGBB
#define FONTCOLOR4      0x0000ff        // Data is RRGGBB
#define FONTCOLOR5      0xffffff        // Data is RRGGBB
#endif
#endif

#if NEWFONTSTRCUCTURE

//If you have more another kind font size for ASCII
#define MULTI_ASCI_FONT_DATA        0
#if MULTI_ASCI_FONT_DATA
#define MULTI_ASCI_FONT_SIZE        24
#endif

#if DYNALOADFONTDATA
// Only use one font at the same moment (for NAND only) Lighter 20090909
// For save the SDRAM, but only can use the one font data
// If you have different font size in the same language, you can not use this flag
#define USINGONEFONTBUFFER          1

// 0x31000=200KB for one font using (data + table),
// the minimum size is (U+N)table size + temp + ?K,
// the ?K is the cache,
// the temp = (Font_Size < MCARD_SECTOR_SIZE)? (MCARD_SECTOR_SIZE << 1):(((Font_Size/MCARD_SECTOR_SIZE)+2)*MCARD_SECTOR_SIZE);
// normally temp = MCARD_SECTOR_SIZE * 2 = 1K
// the ONE_FONT_BUFFER suggest is 200KB
// If the table + font data < ONE_FONT_BUFFER, load all date to DRAM
#define ONE_FONT_BUFFER             0x31000

// Enable the cache buffer for cache Dynamic load data from NAND flash
#define ENABLE_FONT_CACHE           1

#if USINGONEFONTBUFFER
#define FONT_MEMORY_BUF     ONE_FONT_BUFFER
#else
#define FONT_MEMORY_BUF     (ONE_FONT_BUFFER*FONT_MAX_INDEX)    // Max buffer for font
#endif

#else  // DYNALOADFONTDATA

#define FONT_MEMORY_BUF     ((FONT_BIG5 * 0x118000) + (FONT_GB2 * 0xAE000) + (FONT_JIS * 0x9F000) + (FONT_KSC * 0xED000))

#endif  // DYNALOADFONTDATA

#else  // NEWFONTSTRCUCTURE
#define FONT_MEMORY_BUF     ((FONT_BIG5 * 0x104000) + (FONT_GB2 * 0xA0000) + (FONT_JIS * 0x98000) + (FONT_KSC * 0x4C000))
#endif  // NEWFONTSTRCUCTURE

//Please find Lighter to get new RomPacker and Compress AP 0915
//For used four font datas (T-Chinese, S-Chinese, Japen, Korea)
#define DATA_COMPRESSED             0

// -------------------------------------------------
// memory map
// -------------------------------------------------

//#define MEMORY_SIZE_AUTO_DETECT     0
#define MEMORY_SIZE                 (16 * 1024 * 1024)
//#define MEMORY_SIZE                 (64 * 1024 * 1024)
#define OS_BUF_SIZE                 (512 * 1024)
#define USB_DEVICE_BUF_SIZE         (256 * 1024)
#define USB_HOST_BUF_SIZE           (256 * 1024)
#define USB_OTG_BUF_SIZE            (200 * 1024)
// xpg file size + 100k,  20070517 - xpg thumb dynamic allocate with mem_malloc
#define XPG_BUF_SIZE                (0 * 1024)        // support WiFI + BT Xpg file, Sync with MP656

#define FONT_TTF_MEM_BUF_SIZE       (0*1024 * 1024)     //replace the FONT_TTF_MEM_BUF_SIZE 6M to ext_mem_malloc by CJ

#if BITMAP_DATA
#define FONT_BUF_SIZE           FONT_MEMORY_BUF
#else
#define FONT_BUF_SIZE           (0 * 1024 * 1024)
#endif
//For Network Driver and LWIP use
#if Make_USB == AR2524_WIFI
#define NETPOOL_BUF_SIZE        (128 * 4864)
#else
#define NETPOOL_BUF_SIZE        (2560 * 512)
#endif

//#define STRING_BUF_SIZE         (32 * 1024)             //This data is base on MLST size
#define STRING_BUF_SIZE         (0 * 1024)             //This data is base on MLST size

#define EXT_MEMORY_LEAK_DEBUG	0
// -------------------------------------------------
// Image
// -------------------------------------------------
#define IMAGE_COPY_BY_IPU           1
#define NEW_SCALER                  1

//JPEG
#define IMAGE_SW_DECODER            1 //disable this flag won't reduce codesize
#define MAX_PROGRESSIVE_RESOLUTION  (4096 * 4096) //width*hight
#define JPEG444_SW_DECODE           0 //force JPEG 444 format && resizeratio over 3 decode by SW method when only photo view, thumb will be invalid
#define SLIDESHOW_MAX_JPEG_WIDTH    8192
#define SLIDESHOW_MAX_JPEG_HEIGHT   8192
#define SLIDESHOW_MAX_RESOLUTION    64 //M-pixel

// BMP
#define BMP                         0

//GIF
#define GIF                         0

//=============================================================
// MEMORY_SIZE = 32MB
// 1. NON-BLUETOOTH 13MB (3300 * 1024) //width * hight * RGB888
// 2. BLUETOOTH     10MB (2048 * 1024) //width * hight * 4
//
// MEMORY_SIZE = 16MB
// 1. NON-BLUETOOTH 6MB (1500 * 1024) //width * hight * 4
// 2. BLUETOOTH     4MB (1024 * 1024) //width * hight * 4
//
// MEMORY_SIZE = 8MB
// 1. NON-BLUETOOTH 3MB (800 * 600) //width * hight * 4
// 2. BLUETOOTH     1MB (500 * 480) //width * hight * 4
//=============================================================
#define MAX_GIF_RESOLUTION          (13 * 1024 * 1024)
#define GIF_DECODE_BUFFER_SIZE      (768 * 1024)

//#define DYNAMIC_GIF_ENABLE          DISABLE

//PNG
#define PNG                         0
//=============================================================
// MEMORY_SIZE = 32MB
// 1. NON-BLUETOOTH 13MB (3300 * 1024) //width * hight * RGB888
// 2. BLUETOOTH     10MB (2048 * 1024) //width * hight * 4
// PNG_DECODE_BUFFER_SIZE    (4 * 1024 * 1024)
//
// MEMORY_SIZE = 16MB
// 1. NON-BLUETOOTH 6MB (1024 * 1024) //width * hight * 4
// 2. BLUETOOTH     4MB (1024 * 768) //width * hight * 4
// PNG_DECODE_BUFFER_SIZE    (2 * 1024 * 1024)
//
// MEMORY_SIZE = 8MB
// 1. NON-BLUETOOTH 3MB (800 * 600) //width * hight * 4
// 2. BLUETOOTH     1MB (500 * 480) //width * hight * 4
// Cannot support PNG because decode buffer is larger than heap memory
// PNG_DECODE_BUFFER_SIZE     ((1 * 1024 * 1024) + (800 * 1024))
///=============================================================
#define MAX_PNG_RESOLUTION          (13 * 1024 * 1024)
#define PNG_DECODE_BUFFER_SIZE      (4 * 1024 * 1024)

// TIFF
#define TIFF 0

//MPO
#define MPO 0

// -------------------------------------------------
// display - idu & osd
// -------------------------------------------------
#define OSD_ENABLE                  0
#define DYNAMIC_OSD                 0
#define OSD_TIMEBAR                 1
#define IDU_BIT_OFFSET              1
#define OSD_BIT_WIDTH               4
#define ThreeD_ENABLE               0

#if (OSD_BIT_WIDTH == 2)
#define OSD_BIT_OFFSET              2
#elif (OSD_BIT_WIDTH == 4)
#define OSD_BIT_OFFSET              1
#elif (OSD_BIT_WIDTH == 8)
#define OSD_BIT_OFFSET              0
#else
    #error - OSD BIT WIDTH NOT SUPPORT
#endif

//flag define for LCD BackLight Control
#define LCD_BRIGHTNESS_SW           0   //Brightness control by SW
#define LCD_BRIGHTNESS_HW1          1   //Brightness control by PWM3
#define LCD_BRIGHTNESS_HW2          2   //Brightness control by DC2DC

#define INTERNAL_DTD                DISABLE      //Enable internal DC2DC for panel
#define BRIGHTNESS_CTRL             LCD_BRIGHTNESS_SW
#define IDU_CHANGEWIN_MODE          1       // 0: Polling, 1: SW toggle

// -------------------------------------------------
// colors
// -------------------------------------------------
//for movie background color
#define BGCOLOR_BLACK               0
#define BGCOLOR_RED                 1
#define BGCOLOR_GREEN               2
#define BGCOLOR_BLUE                3
#define BGCOLOR_GREY                4
#define BGCOLOR_WHITE               5

#define MOVIE_MIN_BGCOLOR           10
#define MOVIE_MAX_BGCOLOR           240
#define MOVIE_CURR_BGCOLOR          BGCOLOR_BLACK

#define SLIDE_SHOW_MIN_COLOR        MOVIE_MIN_BGCOLOR
#define SLIDE_SHOW_MAX_COLOR        MOVIE_MAX_BGCOLOR
#define SLIDE_SHOW_CURR_COLOR       BGCOLOR_BLUE

#define XPG_LIST_FONT_COLOR         0
#define XPG_LIST_TIME_FONT_COLOR    3

//-------------------------------------------------
// video
//-------------------------------------------------
// VIDEO_ENABLE -- 0: disable video except MJPEG, 1: enable video except MJPEG
#define VIDEO_ENABLE                0
// MJPEG_ENABLE -- 0: disable MJPEG decoder, 1: enable MJPEG decoder
#define MJPEG_ENABLE                1
// display video subtitle when video is played in full screen mode
#define DISPLAY_VIDEO_SUBTITLE      0

// -------------------------------------------------
// audio
// -------------------------------------------------
#define AUDIO_ON					1
#define AUDIO_DAC                   DAC_ALC5621//DAC_WM8904//DAC_ALC5621 // DAC_ALC5621  //DAC_ES7240//DAC_WM8960
#define AUDIO_DAC_USING_PLL2		ENABLE//DISABLE //default: DISABLE; DISABLE: audio dac uses PLL3, ENABLE: audio dac uses PLL2 (first check if audio dac supports PLL2)

#define HAVE_AMP_MUTE 				0
#define LYRIC_ENABLE                0
#define AK4387_on_PQFP              DISABLE
#define AK4387_on_BGA               DISABLE //default

//#define MOVIE_VIDEO_FF_FRAMES       100     //video FF frames
#define MOVIE_VIDEO_FF_SECONDS      2       //video FF seconds
#define MOVIE_VIDEO_FB_SECONDS      -MOVIE_VIDEO_FF_SECONDS	//video FB seconds
#define MOVIE_AUDIO_FF_SECONDS      10      //audio FF seconds
//#define MOVIE_VIDEO_FB_FRAMES       -100    //video FF frames
#define MOVIE_AUDIO_FB_SECONDS      -MOVIE_AUDIO_FF_SECONDS //audio FF seconds
#define RECORD_AUDIO                ENABLE
#define SOFT_RECORD                 DISABLE   //If need record and playback at the same time
#define AMR_RECORD                  DISABLE
#define AMR_RECORDING				DISABLE

// -------------------------------------------------
// audio UI
// -------------------------------------------------
#define MULTIPAGES_AUDIO            DISABLE

// -------------------------------------------------
// Sensor
// -------------------------------------------------
#define SENSOR_ENABLE               ENABLE
#define VirtualSensorEnable         0
//#define SENSOR_TYPE_OV5642
//#define SENSOR_TYPE_OV2643
#define SENSOR_TYPE_OV2643_JH43A
//#define SENSOR_TYPE_S5K6AAFX13
//#define SENSOR_TYPE_MAGIC_SENSOR
//#define  SENSOR_TYPE_NT99140              ENABLE

#if (TCON_ID != TCON_NONE)	
#define	SENSOR_WITH_DISPLAY         ENABLE
#endif

#if (STD_BOARD_VER == MP663_128BGA_16M_SDRAM)
#define SENSOR_GPIO_SSEN            (VGPIO | GPIO_14)
#define SENSOR_GPIO_RESET           (VGPIO | GPIO_18)
#elif (STD_BOARD_VER == MP663_144BGA_32M_SDRAM)
#define SENSOR_GPIO_SSEN            (VGPIO | GPIO_23)
#define SENSOR_GPIO_RESET           (VGPIO | GPIO_24)
#else
#define SENSOR_GPIO_SSEN            GPIO_NULL
#define SENSOR_GPIO_RESET           GPIO_NULL
#endif

#define RECORD_ENABLE               ENABLE              //You must enable SENSOR_ENABLE befor enabing thsi flag

#if ((SENSOR_ENABLE == ENABLE) && (defined(SENSOR_TYPE_MAGIC_SENSOR)))
	#define MAGIC_SENSOR_INTERFACE_ENABLE
	#define MAGIC_SHOP_ENABLE
#endif

// -------------------------------------------------
// ui & gpio
// -------------------------------------------------
#define CARD_INSERT_STABLE_TIME     300 // msec

///////////////////////////////////////////////////////////////////////
// IICM         IICM_PIN_USING_PGPIO    IICM_PIN_USING_VGPIO
// GP0/1        DISABLE                 DISABLE
// PGPIO0/1     ENABLE                  x
// VGPIO23/24   DISABLE                 ENABLE
///////////////////////////////////////////////////////////////////////
#define I2C_FUNCTION                I2C_HW_MASTER_MODE      // default
#define IICM_PIN_USING_PGPIO        DISABLE

#if (STD_BOARD_VER == MP663_128BGA_16M_SDRAM)
#define IICM_PIN_USING_VGPIO        ENABLE
#elif (STD_BOARD_VER == MP663_144BGA_32M_SDRAM)
#define IICM_PIN_USING_VGPIO        DISABLE
#endif

#define IR_ENABLE                   0
#define REMOTE_CONTRLLER            REMOTE_NONE

#define GPIO_MS_PWCTRL              GPIO_NULL
#define GPIO_BL_CTRL                GPIO_NULL
#define GPIO_AC97_RESET             GPIO_NULL
#define GPIO_AMPMUTE                GPIO_NULL

#define KEY_NULL                    0
#define KEY_UP                      1
#define KEY_DOWN                    2
#define KEY_LEFT                    3
#define KEY_RIGHT                   4
#define KEY_MENU                    5
#define KEY_SOURCE                  6
#define KEY_EXIT                    7
#define KEY_BACKWARD                9
#define KEY_FORWARD                 10
#define KEY_STOP                    11
#define KEY_VOLUP                   12
#define KEY_VOLDOWN                 13
#define KEY_ENTER                   14
#define KEY_POWER                   15
#define KEY_SETUP                   16
#define KEY_INFO                    17
#define KEY_BRIGHTNESS              18
#define KEY_REPEAT                  19
#define KEY_MUTE                    20
#define KEY_PREVIOUS                21
#define KEY_NEXT                    22
#define KEY_PIC_MP3                 23
#define KEY_ZOOM                    24
#define KEY_ROTATE                  25
#define KEY_CALENDAR                26
#define KEY_CLOCK                   27

#define KEY_NEXTMODE                30
#define KEY_PHOTOMODE               31
#define KEY_MUSICMODE               32
#define KEY_VIDEOMODE               33
#define KEY_SELECT                  34
#define KEY_CHANGEUI                35
#define KEY_TV_O_P                  36
#define KEY_PAUSE_RESUME            37

#define KEY_ONOFF					38
#define KEY_CAMERA					39
#define KEY_RECORD					40
#define KEY_MODE					41
#define KEY_SET_VGA					42
#define KEY_SET_720P				43

#define KEY_ALARM                   127

#define KEY_PP                      KEY_ENTER

#define MOTION_DETECT				0
// -------------------------------------------------
// USB
// -------------------------------------------------

#define USBOTG_CLK_FROM_GPIO        DISABLE     // USB clock source from GPIO, for MP65x VerC or later only, for MP66x VerB or later only

#define SC_USBDEVICE    ENABLE
#define SC_USBHOST      ENABLE//DISABLE

#define SC_USBOTG_0    ENABLE
#define SC_USBOTG_1    DISABLE

#if SC_USBHOST
#define BT_USB_PORT                 USBOTG0
#define WIFI_USB_PORT               USBOTG0
#define WEBCAM_USB_PORT             USBOTG0
#define USBOTG_HOST_PTP             ENABLE
#define USBOTG_WEB_CAM              DISABLE
#if MAKE_BLUETOOTH
#if (BT_PROFILE_TYPE & BT_HF)
#define USBOTG_HOST_ISOC            ENABLE
#endif
#else
#define USBOTG_HOST_ISOC            DISABLE
#endif
#define USBOTG_HOST_CDC             DISABLE
#define USBOTG_HOST_HID             DISABLE // HID Class for KeyBoard
#define USBOTG_HOST_EYE             DISABLE // Eye Pattern Test
#define USBOTG_HOST_USBIF           DISABLE // Press "VolumnDown" Key into USB-IF Embedded Host Electrical Test
#define USBOTG_HOST_DESC            ENABLE
#define USBOTG_HOST_TEST            DISABLE // Host Compare Write/Read Pattern & Performance
#define USBOTG_HOST_DATANG          DISABLE // USBOTG_HOST_CDC enable, USBOTG_HOST_DATANG must be disable
#else
#define USBOTG_HOST_PTP             DISABLE
#define USBOTG_WEB_CAM              DISABLE
#define USBOTG_HOST_ISOC            DISABLE
#define USBOTG_HOST_CDC             DISABLE
#define USBOTG_HOST_HID             DISABLE
#define USBOTG_HOST_EYE             DISABLE
#define USBOTG_HOST_USBIF           DISABLE
#define USBOTG_HOST_DESC            DISABLE
#define USBOTG_HOST_TEST            DISABLE
#define USBOTG_HOST_DATANG          DISABLE // USBOTG_HOST_CDC enable, USBOTG_HOST_DATANG must be disable
#endif

#if USBOTG_HOST_ISOC
#undef USB_OTG_BUF_SIZE
#define USB_OTG_BUF_SIZE (384 * 1024)
#endif

#if SC_USBDEVICE
#define USBOTG_DEVICE_HOST              ENABLE   // PC would read another USB-IF Host PenDrive
#define USBOTG_DEVICE_EXTERN            DISABLE   // MPX Side Monitor
#define USBOTG_DEVICE_EXTERN_SAMSUNG    DISABLE   // Samsung Side Monitor
#define USBOTG_DEVICE_MSDC_TECO         DISABLE   // TECO MSDC Vendor Cmd
#define WEB_CAM_DEVICE                  ENABLE
#define USBOTG_DEVICE_PROTECTION        ENABLE  // conform by key from AP to display storage
#else
#define USBOTG_DEVICE_HOST              DISABLE
#define USBOTG_DEVICE_EXTERN            DISABLE
#define USBOTG_DEVICE_EXTERN_SAMSUNG    DISABLE
#define USBOTG_DEVICE_MSDC_TECO         DISABLE
#define WEB_CAM_DEVICE                  DISABLE
#define USBOTG_DEVICE_PROTECTION        DISABLE
#endif

#if (USBOTG_DEVICE_EXTERN_SAMSUNG == ENABLE)
    #define USBOTG_DEVICE_EXTERN        DISABLE
#endif

/* === Show Debug Message on Terminal screen via USB cable === */
// 1. Set USBDEVICE_CDC_DEBUG ENABLE
// 2. Install MPXCDC driver for PC
// 3. Open PC Terminal AP [MUST after DPF Plug-in PC]
//     Terminal:Port/depend on USB device Rate/115200 Data/8 Stop/1 Parity/none
// 4. Card In/Out Action and Message will be showed on PC Terminal AP screen
#if (MP_TRACE_DEBUG_PORT == DEBUG_PORT_USB_CDC)
#define USBDEVICE_CDC_DEBUG     ENABLE //OFF
#else
#define USBDEVICE_CDC_DEBUG     DISABLE //OFF
#endif

#if !(SC_USBDEVICE)
#undef USBDEVICE_CDC_DEBUG
#define USBDEVICE_CDC_DEBUG DISABLE
#endif

#if SC_USBHOST
/// the switch of the USB_HOST_ID1 device
#define USB_HOST_ID1_ENABLE         1
/// the switch of the USB_HOST_ID2 device
#define USB_HOST_ID2_ENABLE         1
/// the switch of the USB_HOST_ID3 device
#define USB_HOST_ID3_ENABLE         1
/// the switch of the USB_HOST_ID4 device
#define USB_HOST_ID4_ENABLE         1
/// the switch of the USB_HOST_PTP device
#define USB_HOST_PTP_ENABLE         1
/// the switch of the USB_HOST_ID1 device
#define USBOTG1_HOST_ID1_ENABLE         1
/// the switch of the USB_HOST_ID2 device
#define USBOTG1_HOST_ID2_ENABLE         1
/// the switch of the USB_HOST_ID3 device
#define USBOTG1_HOST_ID3_ENABLE         1
/// the switch of the USB_HOST_ID4 device
#define USBOTG1_HOST_ID4_ENABLE         1
/// the switch of the USB_HOST_PTP device
#define USBOTG1_HOST_PTP_ENABLE         1
#else
/// the switch of the USB_HOST_ID1 device
#define USB_HOST_ID1_ENABLE         0
/// the switch of the USB_HOST_ID2 device
#define USB_HOST_ID2_ENABLE         0
/// the switch of the USB_HOST_ID3 device
#define USB_HOST_ID3_ENABLE         0
/// the switch of the USB_HOST_ID4 device
#define USB_HOST_ID4_ENABLE         0
/// the switch of the USB_HOST_PTP device
#define USB_HOST_PTP_ENABLE         0
/// the switch of the USB_HOST_ID1 device
#define USBOTG1_HOST_ID1_ENABLE         0
/// the switch of the USB_HOST_ID2 device
#define USBOTG1_HOST_ID2_ENABLE         0
/// the switch of the USB_HOST_ID3 device
#define USBOTG1_HOST_ID3_ENABLE         0
/// the switch of the USB_HOST_ID4 device
#define USBOTG1_HOST_ID4_ENABLE         0
/// the switch of the USB_HOST_PTP device
#define USBOTG1_HOST_PTP_ENABLE         0
#endif
/// the switch of the NAND flash device

#define NAND_ENABLE                 0

#if  NAND_ENABLE
#define NAND_ID                     COMMON_NAND
#endif

/// the switch of the SmartMedia device
#define SM_ENABLE                   0
/// the switch of the XD device
#define XD_ENABLE                   0
/// the switch of the Memory Stick device
#define MS_ENABLE                   0
/// the switch of the SD and MMC device
#define SD_MMC_ENABLE               1
///
#define SD2_ENABLE                  0
/// the switch of the CompactFlash device
#define CF_ENABLE                   0
/// the switch of the HardDisk device
#define HD_ENABLE                   0
/// the switch of the SPI nor flash device
#define SPI_STORAGE_ENABLE          0                // 1 : For AP code and user data; 0 : Only for AP code.

#define MCARD_POWER_CTRL            0

#if MCARD_POWER_CTRL
#define XD_PW_GPIO                  23
#define MS_PW_GPIO                  23
#define SDMMC_PW_GPIO               23
#define SD2_PW_GPIO	                23
#define CF_PW_GPIO                  23
#endif

#define USB_WIFI_ENABLE             NETWARE_ENABLE
#define HAVE_USB_MODEM              Make_MODEM  	/* USB modem */


#define SD_MMC_LUN_NUM              LUN_NUM_0
#define XD_LUN_NUM                  NULL_LUN_NUM
#define MS_LUN_NUM                  NULL_LUN_NUM
#define CF_LUN_NUM                  NULL_LUN_NUM
#define SM_LUN_NUM                  NULL_LUN_NUM
#define SD2_LUN_NUM                 NULL_LUN_NUM
#define NAND_LUN_NUM                NULL_LUN_NUM
#define USB_LUN_NUM                 NULL_LUN_NUM
#define SPI_LUN_NUM                 NULL_LUN_NUM

// MAX_LUN : PC get MAX LUN via connecting with DPF(as USB Device)
#if (SYSTEM_DRIVE != SYSTEM_DRIVE_NONE)
    #define SYSTEM_DRIVE_LUN_NUM    NULL_LUN_NUM // LUN_NUM_1
    #define MAX_LUN                 1  // 2
#else
    #define SYSTEM_DRIVE_LUN_NUM    NULL_LUN_NUM
    #define MAX_LUN                 1
#endif

#if TOOL_DRV_SIZE
    #if ((BOOTUP_TYPE == BOOTUP_TYPE_NAND) && NAND_ENABLE)
    #define TOOL_DRIVE_LUN_NUM      NAND_LUN_NUM
    #elif ((BOOTUP_TYPE == BOOTUP_TYPE_SD) && SD_MMC_ENABLE)
    #define TOOL_DRIVE_LUN_NUM      SD_MMC_LUN_NUM
    #elif ((BOOTUP_TYPE == BOOTUP_TYPE_SPI) && SPI_STORAGE_ENABLE)
    #define TOOL_DRIVE_LUN_NUM      SPI_LUN_NUM
    #elif (BOOTUP_TYPE == BOOTUP_TYPE_SPI)
    #define TOOL_DRIVE_LUN_NUM      SD_MMC_LUN_NUM
    #else
    #define TOOL_DRIVE_LUN_NUM      NULL_LUN_NUM
    #endif
#else
    #define TOOL_DRIVE_LUN_NUM      NULL_LUN_NUM
#endif

#define GP_VENDER_INF               "MPX     "          // 8 letters

#define GP_PRODUCT_ID               "Spy Camera      "  // 16 letters
#define GP_PRODUCT_REV              "0.01"              // 4 letters n.nn

#define GP_PRODUCT_ID_LUN_0         "Memory Slot - 1 "  // 16 letters
#define GP_PRODUCT_ID_LUN_1         "Memory Slot - 2 "  // 16 letters
#define GP_PRODUCT_ID_LUN_2         "Memory Slot - 3 "  // 16 letters
#define GP_PRODUCT_ID_LUN_3         "Memory Slot - 4 "  // 16 letters
#define GP_PRODUCT_ID_LUN_4         "Memory Slot - 5 "  // 16 letters
#define GP_PRODUCT_ID_LUN_5         "Memory Slot - 6 "  // 16 letters
#define GP_PRODUCT_ID_LUN_6         "Memory Slot - 7 "  // 16 letters

#define USER_DRV_LABEL              "ENJOY"
#define TOOL_DRV_LABEL              "PASSWORD"
//------------------------------------------------------------------
// xpg
//------------------------------------------------------------------
#define XPG_ROLE_USE_CACHE          1   // set for analog clock every just 1 second to draw by role cache
#define XPG_USE_YUV444              0  // Must YUV444_ENABLE = 1
#define XPG_MULTI_SEARCH            1  // When [EXIT] from page "Card", if true, then show multi .XPG to select

#define SCREEN_TABLE_TOTAL_NUM      1
#define XPG_LOAD_FROM_FLASH         0
#define MUSIC_LIST_ARTIST_ENABLE    0
//#define ERROR_HANDLER_ENABLE        0
#define PAGE_EVENT_ENABLE           0
//#define MENUITEM_EVENT_ENABLE       0

#define BLANK_WIDTH     (DEFAULT_FONT_SIZE * 7 + 10)//For xpgUpdateTimeBar() use
#define BLANK_HEIGHT    (DEFAULT_FONT_SIZE + 4)

#define XPG_ONSCREEN_DEBUG_OSD      0
#define XPG_ONSCREEN_DEBUG_W        400
#define XPG_ONSCREEN_DEBUG_H        16
#define XPG_ONSCREEN_DEBUG_X        (DISP_WIDTH - XPG_ONSCREEN_DEBUG_W)
#define XPG_ONSCREEN_DEBUG_Y        8
#define XPG_ONSCREEN_DEBUG_C        0x00008080

// xpg List
#define XPG_LIST_WIDTH_BY_LISTFRMAE 0
#define XPG_LIST_WITH_INFO          1


// xpg Thumb
#define THUMB_FILENAME_ENABLE       0
#define THUMB_COUNT                 30 //6 // to fix Bugzilla 86
#define THUMB_WIDTH                 232
#define THUMB_HEIGHT                188

// xpg Icon
#define XPG_KEY_ICON_ENABLE     1
#define ICON_BUFFER_SIZE        (64*1024)

#define XPG_ICON_X    32
#define XPG_ICON_Y    (DISP_HEIGHT - 100)


#define SPRITE_TYPE_BUTTON              1
#define SPRITE_TYPE_THUMB               2
#define SPRITE_TYPE_THUMBFRAME          3
#define SPRITE_TYPE_LIST                4
#define SPRITE_TYPE_LISTFRAME           5
#define SPRITE_TYPE_ICON                6
#define SPRITE_TYPE_LISTICON            7
#define SPRITE_TYPE_CARD                8
#define SPRITE_TYPE_TEXT                9
#define SPRITE_TYPE_SCROLLBAR           10
#define SPRITE_TYPE_SCROLLBARBUTTON     11
#define SPRITE_TYPE_TIMEBAR             12
#define SPRITE_TYPE_TIMEBARSLIDER       13
#define SPRITE_TYPE_FILECON             14
#define SPRITE_TYPE_CARDLIGHT           15
#define SPRITE_TYPE_MODEICON            16
#define SPRITE_TYPE_MODELIGHT           17
#define SPRITE_TYPE_LANGUAGE            18
#define SPRITE_TYPE_POPMENU             19
#define SPRITE_TYPE_MENUITEM            20
#define SPRITE_TYPE_MULTILANGUAGE       21
#define SPRITE_TYPE_SETUPICON           22
#define SPRITE_TYPE_MUSICICON           23
#define SPRITE_TYPE_CHECKBOX            24
#define SPRITE_TYPE_SLIDESHOWINDEXTOTAL 25
#define SPRITE_TYPE_MUSICTIME           26
#define SPRITE_TYPE_MUSICTOTALTIME      27
#define SPRITE_TYPE_FREESPACE           29
#define SPRITE_TYPE_CURFILEPREVIEW      30
#define SPRITE_TYPE_CURFILENAME         31
#define SPRITE_TYPE_CURFILESIZE         32
#define SPRITE_TYPE_CURFILEDATE         33
#define SPRITE_TYPE_CURFILEINDEX        34
#define SPRITE_TYPE_CURFILETYPE         35
#define SPRITE_TYPE_CURPAGEINDEX        36
#define SPRITE_TYPE_CURIMAGESIZE        37
#define SPRITE_TYPE_CURCARDNAME         38
#define SPRITE_TYPE_FILETOTALCOUNT      39
#define SPRITE_TYPE_ICONANI             40

#define SPRITE_TYPE_FLASHICON2          47 // type47
#define SPRITE_TYPE_FLASHICON           48 // type48=FlashIcon

#define SPRITE_TYPE_PLAYMODE            52
#define SPRITE_TYPE_CRAWICON            15  // need this one for CRAWICON slide effect
#if  EREADER_ENABLE
#define SPRITE_TYPE_EREADER             69
#endif

#define SPRITE_TYPE_NOSHOW              81
#define SPRITE_TYPE_UNLIGHTITEM         82
#define SPRITE_TYPE_LIGHTITEM           83

#define SPRITE_TYPE_ITEMTEXT            84
#define SPRITE_TYPE_PAGE                85
#define SPRITE_TYPE_SCROLL              86
#define SPRITE_TYPE_TITLE               87
#define SPRITE_TYPE_MUSICINFO           88

#define SPRITE_TYPE_CONTINUOUS              90
#define SPRITE_TYPE_SLEEP	            91
#define SPRITE_TYPE_MDETECT           92


#define XPG_ICON_MASK                   0
#define XPG_ICON_PHOTO                  1
#define XPG_ICON_MUSIC                  2
#define XPG_ICON_VIDEO                  3
#define XPG_ICON_PLAY                   4
#define XPG_ICON_PAUSE                  5
#define XPG_ICON_STOP                   6
#define XPG_ICON_FORWARD                7
#define XPG_ICON_BACKWARD               8
#define XPG_ICON_UP                     9
#define XPG_ICON_DOWN                   10
#define XPG_ICON_RIGHT                  11
#define XPG_ICON_LEFT                   12
#define XPG_ICON_SETUP                  13
#define XPG_ICON_SLIDESHOW              14
#define XPG_ICON_ROTATE                 15
#define XPG_ICON_HOURGLASS              16
#define XPG_ICON_CHECKBOX               17
#define XPG_ICON_ZOOMIN                 18
#define XPG_ICON_ZOOMOUT                19
#define XPG_ICON_BRIGHTNESS             20
#define XPG_ICON_COTRAST                21
#define XPG_ICON_INVALIDKEY             22
#define XPG_ICON_VOLUME                 23
#define XPG_ICON_MUTE                   24
#define XPG_ICON_ENTER                  25
#define XPG_ICON_EXIT                   26
#define XPG_ICON_CARD                   27

#define XPG_IMG_MENUTOP                 28
#define XPG_IMG_MENUMID                 29
#define XPG_IMG_MENUBTM                 30
#define XPG_IMG_MENUSEL                 31

#define XPG_PHOTO_FOLDER                32 // ??? - not match
#define XPG_PHOTO_UP                    33
#define XPG_IMAGE_FOLDER                34
#define XPG_IMAGE_UP                    35
// xpgAudioFunc.c, xpgUpdatemusicIcon:
#define XPG_IMAGE_AUDIO                 36

#define XPG_IMAGE_PAUSE                 36
#define XPG_IMAGE_STOP                  36
#define XPG_IMAGE_PLAY                  36

#define XPG_IMAGE_VIDEO                 37

#if BLUETOOTH == ENABLE
#define XPG_IMAGE_PAIRENABLE            32
#define XPG_IMAGE_CONNECTENABLE         33
#define XPG_IMAGE_MISCELLAUEOUS         34
#define XPG_IMAGE_COMPUTER              35
#define XPG_IMAGE_PHONE                 36
#define XPG_IMAGE_LAN_ACCESS_POINT      37
#define XPG_IMAGE_BTAUDIO               38
#define XPG_IMAGE_PERIPHERAL            39
#define XPG_IMAGE_IMAGING               40
#define XPG_IMAGE_UNCLASSIFIED          41
#endif

#define SSIndexBCount                   200

//xpg page define
#define XPG_PAGE_PAGE0          0
#define XPG_PAGE_LOGO           1
#define XPG_PAGE_MAIN           2
#define XPG_PAGE_Setup          3
#define XPG_PAGE_SetupItem      4
#define XPG_PAGE_DateSet        16
#define XPG_PAGE_SystemInfo     6
#define XPG_PAGE_Mode_Music     7
#define XPG_PAGE_Mode_Video     8
#define XPG_PAGE_Mode_Photo     9
#define XPG_PAGE_Preview        4 //update
#define XPG_PAGE_RecordVideo    11
#define XPG_PAGE_PhotoCapture   12

#define XPG_PAGE_PhotoViewer    14
#define XPG_PAGE_VideoViewer    15

#define XPG_PAGE_MaxId          10

//------------------------------
//  Movie display values
//------------------------------
#define AUTO_PLAY_VIDEO_ENABLE      1
#define ANTI_TEARING_ENABLE         0

#define VIDEO_SHOW_TIMER_ENABLE     0

//for play movie display time
#define MOVIE_DISPLAY_TIME_START_X          70  //60
#define MOVIE_DISPLAY_TIME_START_Y          30  //915
#define MOVIE_DISPLAY_TOTALTIME_START_X     400  //60
#define MOVIE_DISPLAY_TOTALTIME_START_Y     30  //915
#define MOVIE_DISPLAY_TIME_WIDTH            255 //osd area position
#define MOVIE_DISPLAY_TIME_HEIGHT           30

#ifndef SGBOARD
#define MOVIE_DISPLAY_TOTAL_TIME_START_X    (640-44-144)  //240
#define MOVIE_DISPLAY_TOTAL_TIME_START_Y    40    //915
#define MOVIE_DISPLAY_TOTAL_TIME_WIDTH      144
#define MOVIE_DISPLAY_TOTAL_TIME_HEIGHT     34
#else
#define MOVIE_DISPLAY_TOTAL_TIME_START_X    (640 - 0 - 144)
#define MOVIE_DISPLAY_TOTAL_TIME_START_Y    40
#define MOVIE_DISPLAY_TOTAL_TIME_WIDTH      235
#define MOVIE_DISPLAY_TOTAL_TIME_HEIGHT     54
#endif

// for play movie display time in 1080i
#define MOVIE_DISPLAY_TIME_1080_START_X     (1920 - 100 - 144)
#define MOVIE_DISPLAY_TIME_1080_START_Y     170
#define MOVIE_DISPLAY_TIME_1080_WIDTH       144
#define MOVIE_DISPLAY_TIME_1080_HEIGHT      34

#define MOVIE_DISPLAY_TOTAL_TIME_1080_START_X   (1920 - 60 - 144)
#define MOVIE_DISPLAY_TOTAL_TIME_1080_START_Y   60
#define MOVIE_DISPLAY_TOTAL_TIME_1080_WIDTH     144
#define MOVIE_DISPLAY_TOTAL_TIME_1080_HEIGHT    34


//------------------------------
//  file values
//------------------------------

#define FILE_SORT_ENABLE        0
#define COPY_CANCEL_ENABLE      0

#define AUTO_CLEAR_UP_FILE_SYSTEM       1
//#define SEARCH_TYPE     LOCAL_SEARCH_INCLUDE_FOLDER  /* GUI folder enabled */
#define SEARCH_TYPE     GLOBAL_SEARCH                /* GUI folder disabled */


//for ST_FILE_BROWSER_TAG use
#define FILE_LIST_SIZE                  1024
#define FAVOR_HISTORY_SIZE              ((FILE_LIST_SIZE >> 5) + 1)
//#define AUDIO_BUFFER_SIZE               2000
//#define IMG_AND_MOV_BUFFER_SIZE         2000

//------------------------------
//  language values
//------------------------------
#define LANGUAGE_ENGLISH        0
#define LANGUAGE_GERMANY        1
#define LANGUAGE_SPANISH        2
#define LANGUAGE_FRENCH         3
#define LANGUAGE_ITALY          4
#define LANGUAGE_DUTCH          5
#define LANGUAGE_POLISH         6
#define LANGUAGE_PORTUGEES      7
#define LANGUAGE_RUSSIAN        8
#define LANGUAGE_SWEDISH        9
#define LANGUAGE_TURKISH        10
#define LANGUAGE_S_CHINESE      11
#if FONT_BIG5
#define LANGUAGE_T_CHINESE      12
#define LANGUAGE_JAPAN          13
#define LANGUAGE_KOREA          14
#define LANGUAGE_TOTAL_NUM      15
#else
#define LANGUAGE_JAPAN          12
#define LANGUAGE_KOREA          13
#define LANGUAGE_TOTAL_NUM      14
#endif


#if ((STD_BOARD_VER == MP656_SS_8)||(STD_BOARD_VER == MP656_SS_10))
#define LANG_COUNT              14  //English-Germany-Spanish-French-Italian-Dutch-Polish-Portuguees-Russian-Swedish-Turkish-SChinese-Japanese-Korean
#elif (STD_BOARD_VER == MP652_216LQFP_32M_DDR)
#define LANG_COUNT              1   //English-Germany-Spanish-French-Italian-Dutch-Polish-Portuguees-Russian-Swedish-Turkish-SChinese-TChinese-Japanese-Korean
#endif //INCLUDE_BIG5

#define SETUP_DISPLAY_STYLE     SETUP_MIX_STYLE
#define SETUP_FOCUS             4
#define SETUP_DISABLE           2
#define SETUP_NORMAL            1
#define SETUP_SUB               2
#define SETUP_LINE              14
#define SETUP_CHAR              3
#define SETUP_SHADOW            15

#define OSD_CLR_BLUE             				1
#define OSD_CLR_RED             				8
#define OSD_COLOR_WHITE             		14
#define OSD_COLOR_BLACK             		15

#define OSD_FADE_IN             0
#define OSD_FADE_OUT            1
#define OSD_BLEND_MAX           6

//OSD ICON INDEX
#define OSDICON_Battery_0               	1
#define OSDICON_Battery_1               	2
#define OSDICON_Battery_2               	3
#define OSDICON_Battery_3               	4
#define OSDICON_Battery_4               	5
#define OSDICON_DC              					6
#define OSDICON_Down               			7
#define OSDICON_Left               				8
#define OSDICON_Rec               				9
#define OSDICON_Right               			10
#define OSDICON_Slider               			11
#define OSDICON_Sliders               		12
#define OSDICON_Trash               			13
#define OSDICON_Up               				14
#define OSDICON_ZoomIn               		15
#define OSDICON_ZoomOut               	16
#define OSDICON_Lock     			          	17
#define OSDICON_Play			               	18
#define OSDICON_Pause     			          	19
#define OSDICON_FB     			       		20
#define OSDICON_FF     			       		21
//#define OSDICON_Stop     			          	20
//#define OSDICON_Delete     			       21
#define OSDICON_Delete 			       		22
#define OSDICON_Cancel 			       		23
#define OSDICON_NoSD 			       		24
#define OSDICON_UsbOn 			       		25
#define OSDICON_UsbOff 			       		26

//OSD ICON position
//OSD ICON position--status
#if (FOOT_56_SUB_MODE==8)
#define DISPLAY_POS_X                			580
#else
#define DISPLAY_POS_X                			430
#endif
#define DISPLAY_POS_DX                		70
#define DISPLAY_POS_Y                			10
#define DISPLAY_POS_DY                		22
//OSD ICON position--action
#define OSD_ICON_ACTION_POS_X		10
#define OSD_ICON_ACTION_POS_Y		10
#define OSD_ICON_ACTION_POS_W		100
#define OSD_ICON_ACTION_POS_H		70


//------------------------------
// Setup Table on SPI
//------------------------------
#if (BOOTUP_TYPE == BOOTUP_TYPE_SPI)
#define SETUP_TABLE_DIRECT_SPI  1
#define SETUP_TABLE_SPI_ADDR    SPI_RESERVED_SIZE
#endif

//------------------------------
//  Image & image player values
//------------------------------

#define ENJOY_USE_GPIO_KEY      1


//------------------------------
//  Other values
//------------------------------
#define SCREENSAVER_ENABLE      0
#define MPTEST_FLAG             0 //ON
#define RTC_ENABLE              1
#define RTC_AUTO_CORRECT_EN     1
#define RTC_AUTO_CORRECT_VALUE  1
#define RTC_RESET_YEAR          2000           // 1970/1/1 00:00:00
#define RTC_RESET_MONTH         1
#define RTC_RESET_DATE          1

//------------------------------
//  Auto-check
//------------------------------
//
#define PHOTO_PAGE_LIST_COUNT   6
#define MUSIC_PAGE_LIST_COUNT   8

//
#define FONT_ID_TAHOMA19        1
#define FONT_ID_YAHEI19         2
#define FONT_ID_LibMono17       3
#define FONT_ID_DotumChe18      4

#define EXT_FONT_YAHEI_18       0


#endif  //PLATFORM_H


