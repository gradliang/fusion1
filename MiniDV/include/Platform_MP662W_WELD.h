#ifndef PLATFORM_H
#define PLATFORM_H

#include "flagdefine.h"
#include "corelib.h"


//-- corelib.h need check MAKE_XPG_PLAYER & AUDIO_ON

//--产品总开关
//#define PRODUCT_ID									(PCBA_MAIN_BOARD_V10|PANEL_800x600|UI_SURFACE)
//#define PRODUCT_ID									(PCBA_MAIN_BOARD_V11|PANEL_LVDS_1024x600|UI_SURFACE)
#define PRODUCT_ID									(PCBA_MAIN_BOARD_V12|PANEL_800x480|UI_WELDING)
//#define PRODUCT_ID									(PCBA_MAIN_BOARD_V20|PANEL_800x480|UI_WELDING)


//common function
#define RESET_EXCEPTION  								1
#define UPDATE_BY_USB_ENABLE						1

//#define CAMCODERRECORD_OLDMODE  				1

//hardware define

// hardware sub function


//software sub funtion
#if (PRODUCT_UI==UI_WELDING)
#define USBCAM_IN_ENABLE									0// // 1->normal jpg+yuy2  2->only yuy2 640*480
#define  TSPI_ENBALE              								1//ENABLE

//---For test machine
#define		SHOW_CENTER											0  //4 在显示屏中间固定画个十字架及两条水平线，看画面是否居中及水平，用于在装摄像头的夹具上
#define		TEST_DISPLAY_PANEL							0 //4  显示出几个像素，用于在装摄像头的夹具上
#define		TEST_PLANE												0 //4  测试光纤安装水平度程序
#define		TEST_TWO_LED										0 //4  测试照射光纤的两个LED灯的亮度
//#define		VAUTO_PLAY_VIDEO									1
#define		PROC_SENSOR_DATA_MODE					0
#define		IPW_FAST_MODE										1
#endif

//#define FILE_IN_RESOURCE										1

#if USBCAM_IN_ENABLE
#define USBCAM_TECH_MODE									1
#define USBCAM_TMP_SHARE_BUFFER					1
#define USBCAM_DEBUG_ISR_LOST						0
#define ISOC_QUEUE_DYNAMIC        						1
#if ISOC_QUEUE_DYNAMIC
#define ISOC_EVERY_QUEUE_SIZE_FIXED			1 // 0 or 1
#endif
#define CHASE_IN_DECODE										0  // 0 is better
#if !CHASE_IN_DECODE
#define SET_DECODE_OFFSET_ENALBE					1
#define CUT_IMAGE_HEIGHT_ENALBE					1// instead of TCON_RGB24Mode_1280x600
#endif
#define USBCAM_CHECK_INPUT_SET						1
#endif

// -------------------------------------------------
// System config
// -------------------------------------------------
#define CHIP_VER                (CHIP_VER_660 | CHIP_VER_A)
//#define STD_BOARD_VER           MP662_144TQFP_32M_MINIDV // MP660_144TQFP_32M_SDRAM

#if (PRODUCT_PANEL==PANEL_640x480)
#define TCON_ID                 TCON_RGB24Mode_640x480
#define PANEL_ID               PANEL_LQ104V1DG51
#elif (PRODUCT_PANEL==PANEL_800x480)
#define TCON_ID                 TCON_RGB24Mode_800x480
#define PANEL_ID               PANEL_AT080TN03
#elif (PRODUCT_PANEL==PANEL_I_800x480)
#define TCON_ID                 TCON_INTERNAL_800x480
#define PANEL_ID               PANEL_AT102TN03
#elif (PRODUCT_PANEL==PANEL_800x600)
#define TCON_ID                 TCON_RGB24Mode_800x600
#define PANEL_ID               PANEL_A080SN01
#elif (PRODUCT_PANEL==PANEL_1280x720)
#define TCON_ID                 TCON_RGB24Mode_1280x720
#define PANEL_ID               PANEL_A080SN01
#elif (PRODUCT_PANEL==PANEL_NO_DISPLAY)
#define TCON_ID                 TCON_NONE
#define PANEL_ID                PANEL_NONE
#elif (PRODUCT_PANEL==PANEL_480x800)
#define TCON_ID                 TCON_RGB24Mode_480x800
#define PANEL_ID               PANEL_RELAX040
#elif (PRODUCT_PANEL==PANEL_LVDS_1024x600)
#define TCON_ID                 TCON_RGB24Mode_1024x600
#define PANEL_ID               PANEL_A089SW01

#else
#error "\n!!!Please select panel!!!\n"
#endif


#define	PRODUCT_MODEL           "Mini-DV "
#define	SVN_VERSION             "10841" 

//#define Support_EPD

#define ISP_FUNC_ENABLE         ENABLE
#define ISP_FILENAME            "MP660"  /* note: length of ASCII string <= 8 to meet 8.3 short name (extension name is always "ISP") */

//#define BOOTUP_TYPE             BOOTUP_TYPE_SPI
#define BOOTUP_TYPE				BOOTUP_TYPE_NAND
//#define BOOTUP_TYPE             BOOTUP_TYPE_SD
//#define BOOTUP_TYPE             BOOTUP_TYPE_NOISP

#if (BOOTUP_TYPE == BOOTUP_TYPE_NAND)
#define SYSTEM_DRIVE            SYSTEM_DRIVE_ON_NAND
#define SYS_DRV_SIZE            50                      // unit is 256 KB
#define TOOL_DRV_SIZE           0                       // unit is 256 KB
#define DEFAULT_RES_ADDR        0x00600000              // start address of MPRS, that is the size of MPAP region
#define NAND_RESERVED_SIZE      (10 * 1024 * 1024)      // reserved 10MB for raw data type (ex: ISP)
#elif (BOOTUP_TYPE == BOOTUP_TYPE_SPI)
#define SYSTEM_DRIVE            SYSTEM_DRIVE_ON_SPI//SYSTEM_DRIVE_ON_SPI//SYSTEM_DRIVE_NONE
#define SYS_DRV_SIZE            2                       // unit is 256 KB
#define TOOL_DRV_SIZE           1                       // unit is 256 KB
//Need be adjusted according to code size
#define SPI_APCODE_SADDR        0x00010000              // first 64KB is reserved for boot code. This value should be 65536 mutiples
#if MAKE_XPG_PLAYER
#define SPI_RESOCE_SADDR        0x00250000              // reserved for AP cod. This value should be 65536 mutiples //0x00260000
#define SPI_RESERVED_SIZE       0x00300000              // reserved size for AP code and resource. This value should be 65536 mutiples.//0x002C0000
#else
#define SPI_RESOCE_SADDR        0x00200000              // reserved for AP cod. This value should be 65536 mutiples //0x00260000
#define SPI_RESERVED_SIZE       0x00300000              // reserved size for AP code and resource. This value should be 65536 mutiples.//0x002C0000
#endif

#if 1
#define SETUP_TABLE_DIRECT_SPI  1
#define SETUP_TABLE_SPI_ADDR    SPI_RESERVED_SIZE
#define SPI_FS_START_ADDR       (SETUP_TABLE_SPI_ADDR + 64*1024)
#define SPI_FLASH_SIZE          (4*1024*1024)           // 4M Bytes SPI Flash (4*1024*1024)
#endif

#elif (BOOTUP_TYPE == BOOTUP_TYPE_SD)
#define SYSTEM_DRIVE            SYSTEM_DRIVE_ON_SD
#define SYS_DRV_SIZE            128                     // unit is 256 KB
#define TOOL_DRV_SIZE           8                       // unit is 256 KB
#else
#define SYSTEM_DRIVE            SYSTEM_DRIVE_NONE
#define SYS_DRV_SIZE            0                       // unit is 256 KB
#define TOOL_DRV_SIZE           0                       // unit is 256 KB
#undef ISP_FUNC_ENABLE
#define ISP_FUNC_ENABLE         DISABLE
#endif

#define SYS_DRV_LABEL           "SYS"
#define USER_DRV_LABEL          "USER"

#define EXFAT_ENABLE                					0
#define CARD_DETECT_FUNC_ENABLE     		1                   // 0: For embedded MicroSD case, no card detect function, 1: For normal SD
#define CARD_PROTECT_FUNC_ENABLE    	0                   // 0: For embedded MicroSD case, no write protect function, 1: For normal SD

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
#define HUARTA_PIN_MULTIPLEXER  ENABLE

//------------------------------------------------------------------
// Function
//------------------------------------------------------------------

//------------------------------------------------------------------
// Touch controller
//------------------------------------------------------------------
#define TOUCH_CONTROLLER_ENABLE         ENABLE
#define TOUCH_DIRECT_TO_GUI         			ENABLE

  #define TOUCH_CONTROLLER            TOUCH_PANEL_DRIVER_GT911
  #define TOUCH_CONTROLLER_TYPE       TOUCH_PANEL_TYPE_CAPACITIVE
#if (PRODUCT_PCBA==PCBA_MAIN_BOARD_V20)
  #define TOUCH_CONTROLLER_INT_PIN    GPIO_03//GPIO_02
  #define TOUCH_RESET_PIN		(PGPIO | GPIO_01)
#else
  #define TOUCH_CONTROLLER_INT_PIN    GPIO_03//GPIO_02
  #define TOUCH_RESET_PIN		GPIO_02//GPIO_FGPIO_20
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
#define BMP_FONT_ENABLE             				1
#define EXT_FONT_FORMAT_ENABLE      		0

#define DEFAULT_FONT_SIZE           				12
#define UNKNOWN_CHAR                				0x3F  // the unknown character is display "?" = 0x3F

#if BMP_FONT_ENABLE
#define FONT_ID_TAHOMA19        1
//#define FONT_ID_YAHEI19         2
//#define FONT_ID_LibMono17       3
#define FONT_ID_DotumChe18      4
#define FONT_ID_HeiTi19         5
#define FONT_ID_HeiTi16         6
#define FONT_ID_HeiTi10         7
#define FONT_ID_ARIAL_36        8
#endif

#if EXT_FONT_FORMAT_ENABLE
#define EXT_FONT_YAHEI_18       0
#endif

// -------------------------------------------------
// memory map
// -------------------------------------------------

#define MEMORY_SIZE_AUTO_DETECT     0
#define MEMORY_SIZE                 (16 * 1024 * 1024)
#define OS_BUF_SIZE                 (512 * 1024)
#define USB_DEVICE_BUF_SIZE         (256 * 1024)
#define USB_HOST_BUF_SIZE           (256 * 1024)
#define USB_OTG_BUF_SIZE            (200 * 1024)
// xpg file size + 100k,  20070517 - xpg thumb dynamic allocate with mem_malloc
#if MAKE_XPG_PLAYER
#define XPG_BUF_SIZE                (1024 * 1024)
#else
#define XPG_BUF_SIZE                0
#endif

#define EXT_MEMORY_LEAK_DEBUG	0
// -------------------------------------------------
// Image
// -------------------------------------------------

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
#if (MPO == ENABLE)
#define MPO_ENCODE DISABLE
#endif

// -------------------------------------------------
// display - idu & osd
// -------------------------------------------------
#define OSD_ENABLE                  		1
#if OSD_ENABLE
#define IDU_BIT_OFFSET              	1
#define OSD_BIT_WIDTH               	4

#define OSD_BMP                     		0
#define OSD_TRANSPARENT_RGB         0x00000000

#define OSD_DISPLAY_CACHE       	0
#define OSD_LINE_NUM       			20

#if (OSD_BIT_WIDTH == 2)
#define OSD_BIT_OFFSET              2
#elif (OSD_BIT_WIDTH == 4)
#define OSD_BIT_OFFSET              1
#elif (OSD_BIT_WIDTH == 8)
#define OSD_BIT_OFFSET              0
#else
    #error - OSD BIT WIDTH NOT SUPPORT
#endif

#define OSD_COLOR_BLACK             		1
#define OSD_COLOR_WHITE             		2
#define OSD_COLOR_RED             			3
#define OSD_COLOR_GREEN             		4
#define OSD_COLOR_BLUE             			5

#endif

//flag define for LCD BackLight Control
#define LCD_BRIGHTNESS_SW           0   //Brightness control by SW
#define LCD_BRIGHTNESS_HW1          1   //Brightness control by PWM3
#define LCD_BRIGHTNESS_HW2          2   //Brightness control by DC2DC

#define INTERNAL_DTD                DISABLE      //Enable internal DC2DC for panel
#define BRIGHTNESS_CTRL             LCD_BRIGHTNESS_SW
#define IDU_CHANGEWIN_MODE          0       // 0: Polling, 1: SW toggle(好像会分屏)

// -------------------------------------------------
// colors
// -------------------------------------------------
//for movie background color

//-------------------------------------------------
// video
//-------------------------------------------------
// VIDEO_ENABLE -- 0: disable video except MJPEG, 1: enable video except MJPEG
#define VIDEO_ENABLE                0
// MJPEG_ENABLE -- 0: disable MJPEG decoder, 1: enable MJPEG decoder
#define MJPEG_ENABLE                0
// display video subtitle when video is played in full screen mode
#define DISPLAY_VIDEO_SUBTITLE      0

// -------------------------------------------------
// audio
// -------------------------------------------------
#define AUDIO_DAC                   DAC_NONE
//#define AUDIO_ON									(AUDIO_DAC!=DAC_NONE)
#if (AUDIO_ON && (AUDIO_DAC==DAC_NONE))||(!AUDIO_ON && (AUDIO_DAC!=DAC_NONE))
#error "\n!!!	Check AUDIO_ON&AUDIO_DAC set !!!\n"
#endif
#define AUDIO_DAC_USING_PLL2		ENABLE //ENABLE //DISABLE //default: DISABLE; DISABLE: audio dac uses PLL3, ENABLE: audio dac uses PLL2 (first check if audio dac supports PLL2)

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
#define RECORD_AUDIO                DISABLE
#define SOFT_RECORD                 DISABLE   //If need record and playback at the same time
#define AMR_RECORD                  DISABLE

// -------------------------------------------------
// audio UI
// -------------------------------------------------
#define MULTIPAGES_AUDIO            DISABLE

// -------------------------------------------------
// Sensor
// -------------------------------------------------
#define SENSOR_ENABLE               ENABLE
//#define VirtualSensorEnable         0
//#define SENSOR_TYPE_OV5642
//#define SENSOR_TYPE_OV2643
//#define SENSOR_TYPE_S5K6AAFX13
//#define SENSOR_TYPE_MAGIC_SENSOR             

//#define SENSOR_TYPE_OV2643_JH43A
//#define SENSOR_TYPE_CVBS_INPUT   ENABLE


#if (TCON_ID != TCON_NONE)
#define	SENSOR_WITH_DISPLAY         					ENABLE
//#define IPW1_DISABLE						 						ENABLE//Fast input mode
//#define USE_IPW1_DISPLAY_MOTHOD     			ENABLE
//#define  SENSOR_TYPE_NT99140              			ENABLE // really is 99142
#define  SENSOR_TYPE_NT99141              			ENABLE
#if 1//PROC_SENSOR_DATA_MODE
#define  SENSOR_WIN_NUM              						2
#else
#define  SENSOR_WIN_NUM              						1
#endif

#if (SENSOR_WIN_NUM == 2)
#define  DISPLAY_IN_ONE_WIN              						0
#endif

#endif

#if (PRODUCT_PCBA==PCBA_MAIN_BOARD_V10)
#define SENSOR_GPIO_RESET           (KGPIO | GPIO_01)
#endif

#if 0//SENSOR_ENABLE
#define RECORD_ENABLE               ENABLE              //You must enable SENSOR_ENABLE befor enabing thsi flag
#endif
#if RECORD_ENABLE
#define AUTO_GET_RECORD_FPS_TIMER               											10000
#endif

// -------------------------------------------------
// ui & gpio
// -------------------------------------------------
#define CARD_INSERT_STABLE_TIME     300 // msec

///////////////////////////////////////////////////////////////////////
// IICM         IICM_PIN_USING_PGPIO    IICM_PIN_USING_VGPIO
// GP0/1        DISABLE                 DISABLE
// PGPIO0/1     ENABLE                  DISABLE
// VGPIO23/24   DISABLE                 ENABLE
///////////////////////////////////////////////////////////////////////
#if HW_IIC_MASTER_ENABLE
#define I2C_FUNCTION                I2C_HW_MASTER_MODE      // default
#define IICM_PIN_USING_PGPIO        DISABLE
#define IICM_PIN_USING_VGPIO        DISABLE
#else
#define I2C_FUNCTION                I2C_DISABLE
#endif

#define IR_ENABLE                   			DISABLE
//#define REMOTE_CONTRLLER            REMOTE_YBD_4WIN

#define GPIO_MS_PWCTRL              GPIO_NULL
#if (PRODUCT_PCBA==PCBA_MAIN_BOARD_V10)
#define GPIO_BL_CTRL                	(PGPIO | GPIO_01)// MP662 PIN114
#elif (PRODUCT_PCBA==PCBA_MAIN_BOARD_V11||PRODUCT_PCBA==PCBA_MAIN_BOARD_V12||PRODUCT_PCBA==PCBA_MAIN_BOARD_V20)
#define GPIO_BL_CTRL                	(PGPIO | GPIO_03)// MP662 PIN115
#endif
#define GPIO_AMPMUTE                GPIO_NULL

// -------------------------------------------------
// minidv
// -------------------------------------------------
#define CyclicRecording DISABLE//ENABLE
#define LowBattery DISABLE//ENABLE

// ----------------------------------------------------

#define USBOTG_CLK_FROM_GPIO        DISABLE     // USB clock source from GPIO, for MP65x VerC or later only, for MP66x VerB or later only

#define USBOTG_DEVICE_SIDC  DISABLE

#define SC_USBDEVICE    DISABLE//ENABLE
#define SC_USBHOST      DISABLE//ENABLE

#define SC_USBOTG_0    DISABLE//ENABLE
#define SC_USBOTG_1    DISABLE

#if SC_USBHOST
#define BT_USB_PORT                 USBOTG0
#define WIFI_USB_PORT               USBOTG0
#define WEBCAM_USB_PORT             USBOTG0
#define USBOTG_HOST_PTP             ENABLE
#if USBCAM_IN_ENABLE
#define USBOTG_WEB_CAM              ENABLE
#else
#define USBOTG_WEB_CAM              DISABLE
#endif
#if MAKE_BLUETOOTH
#if (BT_PROFILE_TYPE & BT_HF)
#define USBOTG_HOST_ISOC            ENABLE
#endif
#elif USBCAM_IN_ENABLE
#define USBOTG_HOST_ISOC            ENABLE
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
#define USB_OTG_BUF_SIZE (1184 * 1024)
#endif

#if SC_USBDEVICE
#define USBOTG_DEVICE_HOST              DISABLE //ENABLE   // PC would read another USB-IF Host PenDrive
#define USBOTG_DEVICE_EXTERN            DISABLE   // MPX Side Monitor
#define USBOTG_DEVICE_EXTERN_SAMSUNG    DISABLE   // Samsung Side Monitor
#define USBOTG_DEVICE_MSDC_TECO         DISABLE   // TECO MSDC Vendor Cmd
#define WEB_CAM_DEVICE                  DISABLE
#define USBOTG_DEVICE_PROTECTION        DISABLE  // conform by key from AP to display storage
#define USB_PRINT_PHOTO                 DISABLE
#define USBOTG_DEVICE_SIDC              DISABLE //ENABLE
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

#if (BOOTUP_TYPE == BOOTUP_TYPE_NAND)
#define NAND_ENABLE                 1
#endif

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
#define NAND_LUN_NUM                LUN_NUM_1
#define USB_LUN_NUM                 NULL_LUN_NUM
#define SPI_LUN_NUM                  NULL_LUN_NUM

// MAX_LUN : PC get MAX LUN via connecting with DPF(as USB Device)
#if (SYSTEM_DRIVE != SYSTEM_DRIVE_NONE)
    #define SYSTEM_DRIVE_LUN_NUM    LUN_NUM_2
    #define MAX_LUN                 3
#else
    #define SYSTEM_DRIVE_LUN_NUM    NULL_LUN_NUM
    #define MAX_LUN                 2
#endif

#if TOOL_DRV_SIZE
    #if ((BOOTUP_TYPE == BOOTUP_TYPE_NAND) && NAND_ENABLE)
    #define TOOL_DRIVE_LUN_NUM      NAND_LUN_NUM
    #elif ((BOOTUP_TYPE == BOOTUP_TYPE_SD) && SD_MMC_ENABLE)
    #define TOOL_DRIVE_LUN_NUM      SD_MMC_LUN_NUM
    #else
    #define TOOL_DRIVE_LUN_NUM      NULL_LUN_NUM
    #endif
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

//------------------------------------------------------------------
// xpg
//------------------------------------------------------------------
#define SCREEN_TABLE_TOTAL_NUM      1
#if MAKE_XPG_PLAYER
#define XPG_ROLE_USE_CACHE          0   // set for analog clock every just 1 second to draw by role cache

// xpg Thumb
//#define THUMB_COUNT                 6
//#define THUMB_WIDTH                 160
//#define THUMB_HEIGHT                120

// xpg Icon
//#define XPG_KEY_ICON_ENABLE     0
//#define ICON_BUFFER_SIZE        (64*1024)
//#define XPG_ICON_X    32
//#define XPG_ICON_Y    (DISP_HEIGHT - 100)


#define XPG_ROLE_TEXT_BLACK_0              0 //0-9
#define XPG_ROLE_TEXT_WHITE_0              10 //10-19

#define XPG_ROLE_BACKGROUND_0               0
#define XPG_ROLE_ICON_MASK_0                1
#define XPG_ROLE_SMALL_BUTTON_MASK          2
#define XPG_ROLE_SMALL_BUTTON_ICON          3
#define XPG_ROLE_SMALL_BUTTON_LIGHT         4
#define XPG_ROLE_CLOSE_ICON_MASK            5
#define XPG_ROLE_CLOSE_ICON_OLD             6
#define XPG_ROLE_ADD_MINUS                  7
#define XPG_ROLE_ADD_MASK                   8
#define XPG_ROLE_MINUS_MASK                 9
#define XPG_ROLE_UP_ICON                    10
#define XPG_ROLE_UP_MASK                    11
#define XPG_ROLE_DOWN_ICON                  12
#define XPG_ROLE_DOWN_MASK                  13
#define XPG_ROLE_CANCEL_ICON                14
#define XPG_ROLE_CANCEL_MASK                15
#define XPG_ROLE_OK_ICON                    16
#define XPG_ROLE_OK_MASK                    17
#define XPG_ROLE_LONG_LIST_DARK             18
#define XPG_ROLE_LONG_LIST_LIGHT            19
#define XPG_ROLE_LONG_LIST_MASK             20
#define XPG_ROLE_KEYBOARD_0                 21
#define XPG_ROLE_KEYBOARD_1                 22
#define XPG_ROLE_KEYBOARD_2                 23
#define XPG_ROLE_KEYBOARD_3                 24
#define XPG_ROLE_KEYBOARD_4                 25
#define XPG_ROLE_KEYBOARD_5                 26
#define XPG_ROLE_KEYBOARD_6                 27
#define XPG_ROLE_KEYBOARD_7                 28
#define XPG_ROLE_KEYBOARD_8                 29
#define XPG_ROLE_KEYBOARD_9                 30
#define XPG_ROLE_KEYBOARD_NULL              31
#define XPG_ROLE_KEYBOARD_BACKSPACE         32
#define XPG_ROLE_KEYBOARD_LEFT_MASK         33
#define XPG_ROLE_KEYBOARD_RIGHT_MASK        34
#define XPG_ROLE_BUTTON_OK_ICON             35
#define XPG_ROLE_BUTTON_CANCEL_ICON         36
#define XPG_ROLE_BUTTON_MASK                37

#define XPG_ROLE_ICON_YJR_OFF               38
#define XPG_ROLE_ICON_OPM_OFF               49
#define XPG_ROLE_ICON_YJR_ON                50
#define XPG_ROLE_ICON_OPM_ON                61

#define XPG_ROLE_BACK_ICON                  62
#define XPG_ROLE_BACK_ICON_MASK             63
#define XPG_ROLE_CLOSE_ICON                 64
#define XPG_ROLE_ADD                        65
#define XPG_ROLE_MINUS                      66
#define XPG_ROLE_OK_BUTTON                  67
#define XPG_ROLE_OK_BUTTON_MASK             68
#define XPG_ROLE_CANCEL_BUTTON              69  
#define XPG_ROLE_CANCEL_BUTTON_MASK         70
#define XPG_ROLE_LONG2_LIST_DARK            71
#define XPG_ROLE_LONG2_LIST_LIGHT           72
#define XPG_ROLE_LONG2_LIST_MASK            73
#define XPG_ROLE_MID2_LIST_DARK             74
#define XPG_ROLE_MID2_LIST_LIGHT            75
#define XPG_ROLE_MID2_LIST_MASK             76
#define XPG_ROLE_STATUS_ICON_AUTOFUSION     77
#define XPG_ROLE_STATUS_ICON_CLOUD_MODE     78
#define XPG_ROLE_STATUS_ICON_CLOUD_OFF      79
#define XPG_ROLE_STATUS_ICON_AUTO_OFF       80
#define XPG_ROLE_STATUS_ICON_SMART_BL       81
#define XPG_ROLE_MAIN_ERROR_ICON       					82
#define XPG_ROLE_CLOSE_ICON_NEW_MASK       		83




#define SPRITE_TYPE_HILIGHTFRAME            5
#define SPRITE_TYPE_ICON                    12
#define SPRITE_TYPE_LIGHT_ICON              13
#define SPRITE_TYPE_DARK_ICON               14
#define SPRITE_TYPE_MASK                    15
#define SPRITE_TYPE_CLOSE_ICON              19
#define SPRITE_TYPE_TEXT                    20
#define SPRITE_TYPE_DIALOG                  23
#define SPRITE_TYPE_SCROLL                  26
#define SPRITE_TYPE_FRAME                  		27
#define SPRITE_TYPE_ROLE                 		28
#define SPRITE_TYPE_REPEATICON          29

//xpg page define




// dialog define
#define Dialog_Toast                        1           // 文字弹出
#define Dialog_ReSuGuan                     2           // 热塑管设置对话框
#define Dialog_ModifyNumber                 3           // 修改数字对话框(加热温度设置对话框, 加热时间设置对话框)
#define Dialog_ShutdownTime                 4           // 关机时间对话框
#define Dialog_SetBrightness                5           // 设置亮度对话框
#define Dialog_SetSound                     6           // 
#define Dialog_SetTime                      7           // 设置时间对话框
#define Dialog_SetDate                      8           // 设置日期对话框
#define Dialog_SetDateFormat                9           // 设置日期格式对话框
#define Dialog_SetLang                      10          // 
#define Dialog_SetPassword1                 11          // 设置密码对话框
#define Dialog_SetPassword2                 12          // 设置密码对话框2
#define Dialog_CheckPassword                13          // 检查密码对话框
#define Dialog_EditValue                    14          // 输入
#define Dialog_Value                        15          // 值显示
#define Dialog_About                        16          // 关于本机
#define Dialog_Times                        17          // 电极棒信息
#define Dialog_TempInfo                     18          // 温度信息
#define Dialog_BatInfo                      19          // 电池信息
#define Dialog_MainPageError                      20          //4 主界面出现的错误提示框
#define Dialog_PowerOnCheckHirePassword                      21          //4 开机确认租借密码
#define Dialog_PowerOnCheckOpenPassword                      22          //4 开机确认开机密码
#define Dialog_Note_ForgetHirePassword                      23          //4 忘记租借密码
#define Dialog_Note_ForgetOpenPassword                      24          //4 忘记开机密码




#define XPG_DIALOG_EXTRA_SPRITE_ENABLE      1
#define DIALOG_PAGE_NAME                    "Dialog"

#endif

//------------------------------
//  Movie display values
//------------------------------
#define VIDEO_SHOW_TIMER_ENABLE     0
#if 0
#define AUTO_PLAY_VIDEO_ENABLE      1
#define ANTI_TEARING_ENABLE         0


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
#endif

//------------------------------
//  file values
//------------------------------

#define FILE_SORT_ENABLE        1
#define COPY_CANCEL_ENABLE      0

//#define SEARCH_TYPE     LOCAL_SEARCH_INCLUDE_FOLDER  /* GUI folder enabled */
#define SEARCH_TYPE     GLOBAL_SEARCH                /* GUI folder disabled */


//for ST_FILE_BROWSER_TAG use
#define FILE_LIST_SIZE                  		512
//#define FAVOR_HISTORY_SIZE              1//((FILE_LIST_SIZE >> 5) + 1)
//#define AUDIO_BUFFER_SIZE               2000
//#define IMG_AND_MOV_BUFFER_SIZE         2000

//------------------------------
//  language values
//------------------------------
#define LANGUAGE_S_CHINESE      0
#define LANGUAGE_ENGLISH          1
//#define LANGUAGE_T_CHINESE      2
#define LANGUAGE_TOTAL_NUM     2  //  3
//------------------------------
//  Image & image player values
//------------------------------

//------------------------------
//   Charset define
//------------------------------
#if 0
#define CHARSET_CP1250_ENABLE           0		// Central Europe
#define CHARSET_CP1251_ENABLE           0		// Russian
#define CHARSET_CP1252_ENABLE           0		// English/French/Spanish/German/Portuguese/Italian
#define CHARSET_GBK_ENABLE              0		// Chinese Simplified
#define CHARSET_BIG5_ENABLE             0		// Chinese Traditional
#define CHARSET_CP949_ENABLE            0		// Korean
#define CHARSET_SHIFT_JIS_ENABLE        0		// Japanese
#endif

//------------------------------
//  Other values
//------------------------------
//#define RTC_ENABLE              1
#if RTC_ENABLE
#define RTC_AUTO_CORRECT_EN     1
#define RTC_AUTO_CORRECT_VALUE  1
#define RTC_RESET_YEAR          2000           // 1970/1/1 00:00:00
#define RTC_RESET_MONTH         1
#define RTC_RESET_DATE          1
#define RTC_MAX_YEAR          							2100
#endif
//------------------------------
//  Auto-check
//------------------------------


#endif  //PLATFORM_H

