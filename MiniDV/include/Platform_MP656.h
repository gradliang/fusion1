#ifndef PLATFORM_H
#define PLATFORM_H

#include "flagdefine.h"
#include "corelib.h"

// -------------------------------------------------
// System config
// -------------------------------------------------
#define CHIP_VER                (CHIP_VER_650 | CHIP_VER_B)
#define STD_BOARD_VER           MP656_216LQFP_32M_DDR

// 19"
//#define TCON_ID                 TCON_LVDSMode_1280x1024 // 39
//#define PANEL_ID                PANEL_LTM190E1L01 // 1280x1024
// Demoset 7"
#define TCON_ID                 TCON_LVDSMode_1024x600
#define PANEL_ID                PANEL_A089SW01
// CPT 9"
//#define TCON_ID                 TCON_LVDSMode_1366x768_CPT
//#define PANEL_ID                PANEL_CLAA101

#define	PRODUCT_FW_VERSION      "MP656_Demo "
#define	PRODUCT_MODEL           "Demo "
#define	SVN_VERSION             "10191" 

#define ISP_FUNC_ENABLE         ENABLE
#define ISP_FILENAME            "MP650"  /* note: length of ASCII string <= 8 to meet 8.3 short name (extension name is always "ISP") */

#define BOOTUP_TYPE             BOOTUP_TYPE_NAND
//#define BOOTUP_TYPE             BOOTUP_TYPE_SPI
//#define BOOTUP_TYPE             BOOTUP_TYPE_NOISP

#if (BOOTUP_TYPE == BOOTUP_TYPE_NAND)
#define SYSTEM_DRIVE            SYSTEM_DRIVE_ON_NAND
#define SYS_DRV_SIZE            4                       // unit is 256 KB
#define DEFAULT_RES_ADDR        0x00A00000              // start address of MPRS, that is the size of MPAP region
#define NAND_RESERVED_SIZE      (16 * 1024 * 1024)      // reserved 16MB for raw data type (ex: ISP)
#elif (BOOTUP_TYPE == BOOTUP_TYPE_SPI)
#define SYSTEM_DRIVE            SYSTEM_DRIVE_NONE
#define SYS_DRV_SIZE            0                       // unit is 256 KB
#define TOOL_DRV_SIZE           0                       // SPI not support tool drive (Drive ID number issue)
//Need be adjusted according to code size, current setting is for 32Mb SPI
#define SPI_RESERVED_SIZE       (8 * 1024 * 1024)       // reserved 8MB size for AP code and resource. This value should be 65536 (0x00010000) mutiples.
#define SPI_APCODE_SADDR        0x00010000              // first 64KB is reserved for boot code and setting table. This value should be 65536 mutiples
#define SPI_RESOCE_SADDR        0x00600000              // reserved 5768KB for AP code , 2048KB for resource. This value should be 65536 mutiples
#else
#define SYSTEM_DRIVE            SYSTEM_DRIVE_NONE
#define SYS_DRV_SIZE            0                       // unit is 256 KB
#endif

/* enable this if want to use clusters Allocation Bitmap mechanism for FAT12/16/32 file systems to speed up free clusters searching */
#define ENABLE_ALLOC_BITMAP_FOR_FAT121632   0

//Main crystal clock, in Hz/Sec
#define MAIN_CRYSTAL_CLOCK      12000000    // For real chip

//-------------------------------------------------
//  Debug Com Port for HUART
//-------------------------------------------------
// Depend on PCB schematic
// For MP615, always using DEBUG_PORT_HUART_A (UART and HUART are same interrupt index)
#define MP_TRACE_DEBUG_PORT     DEBUG_PORT_HUART_B

// HUART-A pin multiplexer elect, Only for MP65x/66x serial
// For MP65x serial
// HUARTA_PIN_MULTIPLEXER - ENABLE : Using KGPIO (when I2CM is enabled, using KGPIO-1/2)
//                        - DISABLE : Using GP (when I2CM is disabled, using GP-0/1)
// For MP66x serial
// HUARTA_PIN_MULTIPLEXER - ENABLE : Using UGPIO-0/1 (when I2CM is enabled, using UGPIO-0/1)
//                        - DISABLE : Using GP (when I2CM is disabled, using GP-0/1)
//
#define HUARTA_PIN_MULTIPLEXER  DISABLE

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
#define HAVE_YOUTUBE 1
#define HAVE_YOUKU3G 0
#define HAVE_FTP_SERVER        0

//USB_WIFI 0:NONE 1:RALINK 2:REALTEK 3:ATHEROS
#define USB_WIFI             3
#define Write_To_NAND 0
#define UART_EDGE			DISABLE  	// EDGE modem
#define PPP_ENABLE			DISABLE
#define USB_ARCH_URB		DISABLE

#define DM9KS_ETHERNET_ENABLE	0
#define DM9621_ETHERNET_ENABLE	0

#define POP3_TEST               0

#define HAVE_FACEBOOK 0

#define HAVE_HTTP_SERVER        0

#define P2P_TEST        0

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
#define DEFAULT_FONT_SIZE           18

#define UNKNOWN_CHAR                0x3F  // the unknown character is display "?" = 0x3F

// Modify the font structure to modularize
#define NEWFONTSTRCUCTURE           1

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
#define ONE_FONT_BUFFER            (256 * 1024)//0x31000

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
#define MEMORY_SIZE_AUTO_DETECT     1
#define MEMORY_SIZE                 (32 * 1024 * 1024)
//#define MEMORY_SIZE                 (64 * 1024 * 1024)
#define OS_BUF_SIZE                 (1 * 1024 * 1024)
#define USB_DEVICE_BUF_SIZE         (256 * 1024)
#define USB_HOST_BUF_SIZE           (256 * 1024)
#define USB_OTG_BUF_SIZE            (200 * 1024)
// xpg file size + 100k,  20070517 - xpg thumb dynamic allocate with mem_malloc
#define XPG_BUF_SIZE                ((6000 + 100) * 1024)       // support WiFI + BT Xpg file, SXGA

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

#define STRING_BUF_SIZE         (32 * 1024)             //This data is base on MLST size

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
#define BMP                         1

//GIF
#define GIF                         1

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
#define PNG                         1
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
#define TIFF 1

//MPO
#define MPO 0
#if (MPO == ENABLE)
#define MPO_ENCODE DISABLE
#endif



// -------------------------------------------------
// display - idu & osd
// -------------------------------------------------
#define OSD_ENABLE                  1
#define DYNAMIC_OSD                 0
#define OSD_TIMEBAR                 1
#define IDU_BIT_OFFSET              1
#define OSD_BIT_WIDTH               8
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

#if (CHIP_VER_LSB == CHIP_VER_A)
#define INTERNAL_DTD                DISABLE     //Disable internal DC2DC for panel
#define BRIGHTNESS_CTRL             LCD_BRIGHTNESS_HW1
#else
#define INTERNAL_DTD                ENABLE      //Enable internal DC2DC for panel
#define BRIGHTNESS_CTRL             LCD_BRIGHTNESS_HW2
#endif

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
#define VIDEO_ENABLE                1
// MJPEG_ENABLE -- 0: disable MJPEG decoder, 1: enable MJPEG decoder
#define MJPEG_ENABLE                1
// display video subtitle when video is played in full screen mode
#define DISPLAY_VIDEO_SUBTITLE      1
#define	VIDEO_COLOR_TRANSFORM_IDU_WITH_PP	0	//not finished, do NOT open
#define	VIDEO_SCALAR_IDU_WITH_PP			0	//not finished, do NOT open

//------------------------------
//  MOVIE_EVENT_DEBUG
//------------------------------
#define MOVIE_EVENT_DEBUG_AVSYNC	(1<<0)
#define	MOVIE_EVENT_DEBUG_EVENT		(1<<1)
#define	MOVIE_EVENT_DEBUG_FUNC		(1<<2)
#define	MOVIE_EVENT_DEBUG_OTHER		(1<<3)
#define	MOVIE_EVENT_DEBUG			0	//(MOVIE_EVENT_DEBUG_AVSYNC|MOVIE_EVENT_DEBUG_EVENT|MOVIE_EVENT_DEBUG_FUNC)



// -------------------------------------------------
// audio
// -------------------------------------------------
#define AUDIO_ON			1
#define AUDIO_DAC                   DAC_WM8961//DAC_WM8961
#define AUDIO_DAC_USING_PLL2		DISABLE //default: DISABLE; DISABLE: audio dac uses PLL3, ENABLE: audio dac uses PLL2 (first check if audio dac supports PLL2)

#define HAVE_AMP_MUTE 				0
#define LYRIC_ENABLE                0
#define AK4387_on_PQFP              DISABLE
#define AK4387_on_BGA               ENABLE //default

//#define MOVIE_VIDEO_FF_FRAMES       100     //video FF frames
#define MOVIE_VIDEO_FF_SECONDS      2       //video FF seconds
#define MOVIE_VIDEO_FB_SECONDS      -MOVIE_VIDEO_FF_SECONDS	//video FB seconds
#define MOVIE_AUDIO_FF_SECONDS      10      //audio FF seconds
//#define MOVIE_VIDEO_FB_FRAMES       -100    //video FF frames
#define MOVIE_AUDIO_FB_SECONDS      -MOVIE_AUDIO_FF_SECONDS //audio FF seconds
#define RECORD_AUDIO                DISABLE
#define SOFT_RECORD                 DISABLE   //If need record and playback at the same time
#define AMR_RECORD                  DISABLE

// -------------------------------------------------
// audio UI
// -------------------------------------------------
#define MULTIPAGES_AUDIO            ENABLE

// -------------------------------------------------
// Sensor
// -------------------------------------------------
#define	SENSOR_ENABLE               DISABLE
#define RECORD_ENABLE               DISABLE             //You must enable SENSOR_ENABLE befor enabing thsi flag

// -------------------------------------------------
// ui & gpio
// -------------------------------------------------
#define CARD_INSERT_STABLE_TIME     300 // msec

#define I2C_FUNCTION                I2C_HW_MASTER_MODE      // default
#define IICM_PIN_USING_PGPIO        ENABLE

#define IR_ENABLE                   1
#define REMOTE_CONTRLLER            REMOTE_MPX_STANDARD_24KEYS

#define GPIO_MS_PWCTRL              GPIO_NULL
#define GPIO_BL_CTRL                (NORMAL_GPIO | GPIO_05)
#define GPIO_AC97_RESET             GPIO_NULL

#if (STD_BOARD_VER == MP656_216LQFP_32M_DDR)
#define GPIO_AMPMUTE                (OGPIO | GPIO_00)
#else   // Socket board
#define GPIO_AMPMUTE                (NORMAL_GPIO | GPIO_06)
#endif

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

#define KEY_ALARM                   127

#define KEY_PP                      KEY_ENTER

// -------------------------------------------------
// USB
// -------------------------------------------------

#define USBOTG_CLK_FROM_GPIO        DISABLE     // USB clock source from GPIO, for MP65x VerC or later only, for MP66x VerB or later only

#define SC_USBDEVICE    ENABLE
#define SC_USBHOST      ENABLE

#define SC_USBOTG_0    ENABLE
#define SC_USBOTG_1    ENABLE

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
#define WEB_CAM_DEVICE                  DISABLE
#define USBOTG_DEVICE_PROTECTION        DISABLE  // conform by key from AP to display storage
#define USB_PRINT_PHOTO                 DISABLE
#define USBOTG_DEVICE_SIDC              ENABLE
#else
#define USBOTG_DEVICE_HOST              DISABLE
#define USBOTG_DEVICE_EXTERN            DISABLE
#define USBOTG_DEVICE_EXTERN_SAMSUNG    DISABLE
#define USBOTG_DEVICE_MSDC_TECO         DISABLE
#define WEB_CAM_DEVICE                  DISABLE
#define USBOTG_DEVICE_PROTECTION        DISABLE
#define USB_PRINT_PHOTO                 DISABLE
#define USBOTG_DEVICE_SIDC              DISABLE
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

#define NAND_ENABLE                 1

#if  NAND_ENABLE
#define NAND_ID                     COMMON_NAND
#endif

/// the switch of the SmartMedia device
#define SM_ENABLE                   0
/// the switch of the XD device
#define XD_ENABLE                   1
/// the switch of the Memory Stick device
#define MS_ENABLE                   1
/// the switch of the SD and MMC device
#define SD_MMC_ENABLE               1
///
#define SD2_ENABLE                  1
/// the switch of the CompactFlash device
#define CF_ENABLE                   1
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
#define XD_LUN_NUM                  LUN_NUM_0
#define MS_LUN_NUM                  LUN_NUM_0
#define CF_LUN_NUM                  LUN_NUM_1
#define SM_LUN_NUM                  NULL_LUN_NUM
#define SD2_LUN_NUM                 LUN_NUM_2
#define NAND_LUN_NUM                LUN_NUM_3

#if USBOTG_DEVICE_HOST
#define USB_LUN_NUM                 LUN_NUM_4
#endif

// MAX_LUN : PC get MAX LUN via connecting with DPF(as USB Device)
#if (SYSTEM_DRIVE != 0)
    #if USBOTG_DEVICE_HOST
    #define SYSTEM_DRIVE_LUN_NUM    LUN_NUM_5
    #define MAX_LUN            6
    #else
    #define SYSTEM_DRIVE_LUN_NUM    LUN_NUM_4
    #define MAX_LUN            5
    #endif
#else
    #define SYSTEM_DRIVE_LUN_NUM    NULL_LUN_NUM

    #if USBOTG_DEVICE_HOST
    #define MAX_LUN            5
    #else
    #define MAX_LUN            4
    #endif
#endif

#define GP_VENDER_INF               "MPX     "          // 8 letters

#define GP_PRODUCT_ID               "iPlay           "  // 16 letters
#define GP_PRODUCT_REV              "0.00"              // 4 letters n.nn

#define GP_PRODUCT_ID_LUN_0         "Memory Slot - 1 "  // 16 letters
#define GP_PRODUCT_ID_LUN_1         "Memory Slot - 2 "  // 16 letters
#define GP_PRODUCT_ID_LUN_2         "Memory Slot - 3 "  // 16 letters
#define GP_PRODUCT_ID_LUN_3         "Memory Slot - 4 "  // 16 letters
#define GP_PRODUCT_ID_LUN_4         "Memory Slot - 5 "  // 16 letters
#define GP_PRODUCT_ID_LUN_5         "Memory Slot - 6 "  // 16 letters
#define GP_PRODUCT_ID_LUN_6         "Memory Slot - 7 "  // 16 letters

//------------------------------------------------------------------
// xpg
//------------------------------------------------------------------
#define XPG_STD_DPF_GUI             1   // only for project STD_DPF or Public Board 
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
#define SPRITE_TYPE_NOSHOWSPRITE		28
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

#define FILE_SORT_ENABLE        1
#define COPY_CANCEL_ENABLE      0

//#define SEARCH_TYPE     LOCAL_SEARCH_INCLUDE_FOLDER  /* GUI folder enabled */
#define SEARCH_TYPE     GLOBAL_SEARCH                /* GUI folder disabled */

/* SONY DCF (Digital Camera File System) rules support. When enabled, use customized behaviors. So, disable it by default. And only enabled by FAE in Platform_XXXX.h */
#define SONY_DCF_ENABLE         0

#if (SONY_DCF_ENABLE)
  #define FILE_LIST_SIZE               10000  /* must enough for 9,999 DCF objects */
#else
  #define FILE_LIST_SIZE               4000
#endif

#define FAVOR_HISTORY_SIZE              ((FILE_LIST_SIZE >> 5) + 1)   /* 1 bit for each file list entry */
//#define AUDIO_BUFFER_SIZE               2000
//#define IMG_AND_MOV_BUFFER_SIZE         2000

//------------------------------
//  language values
//------------------------------
#if 0
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
#else
enum{

	LANGUAGE_ENGLISH,      // 0
	LANGUAGE_GERMANY,     //  1
	LANGUAGE_SPANISH,     //  2
	LANGUAGE_FRENCH,       //  3
	LANGUAGE_ITALY,          // 4
	LANGUAGE_DUTCH,         //  5
	LANGUAGE_POLISH,        // 6
	LANGUAGE_PORTUGEES, // 7
	LANGUAGE_RUSSIAN,     // 8
	LANGUAGE_SWEDISH,    // 9
	LANGUAGE_TURKISH,    // 10
#if FONT_GB2
	LANGUAGE_S_CHINESE,
#endif
#if FONT_BIG5
	LANGUAGE_T_CHINESE,
#endif
#if FONT_JIS
	LANGUAGE_JAPAN,
#endif
#if FONT_KSC
	LANGUAGE_KOREA,
#endif
	LANGUAGE_TOTAL_NUM

};

#endif

#if ((STD_BOARD_VER == MP656_SS_8)||(STD_BOARD_VER == MP656_SS_10))
#define LANG_COUNT              14  //English-Germany-Spanish-French-Italian-Dutch-Polish-Portuguees-Russian-Swedish-Turkish-SChinese-Japanese-Korean
#elif (STD_BOARD_VER == MP652_216LQFP_32M_DDR)
#define LANG_COUNT              1   //English-Germany-Spanish-French-Italian-Dutch-Polish-Portuguees-Russian-Swedish-Turkish-SChinese-TChinese-Japanese-Korean
#else
#define LANG_COUNT              1
#endif //INCLUDE_BIG5

#define SETUP_DISPLAY_STYLE     SETUP_MIX_STYLE
#define SETUP_FOCUS             4
#define SETUP_DISABLE           2
#define SETUP_NORMAL            1
#define SETUP_SUB               2
#define SETUP_LINE              14
#define SETUP_CHAR              3
#define SETUP_SHADOW            15
#define OSD_FADE_IN             0
#define OSD_FADE_OUT            1
#define OSD_BLEND_MAX           6

//------------------------------
//  Image & image player values
//------------------------------


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


#endif  //PLATFORM_H

