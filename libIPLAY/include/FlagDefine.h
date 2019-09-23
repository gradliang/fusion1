
#ifndef _MP650_FLAG__DEFINE_H__
#define _MP650_FLAG__DEFINE_H__

#define ENABLE              1
#define DISABLE             0

#define PASS                0
#define FAIL                -1
#define DECODE_FAIL         -2

#define SYS_PASS            PASS
#define SYS_FAIL            FAIL

#define HIGH_ON            1
#define LOW_ON             0



#define GAMMA_new           0
#define OPEN_EXIF           	0
#define OUTPUT_JPEG         0
#define New_RGB24           1
#define INTERNAL_SIMULATE   0

#define TCON_CLKSRC_AUTO    0   //Use for struct ST_TCON
#define TCON_CLKSRC_PLL1    1
#define TCON_CLKSRC_PLL2    2

#define BYPASS              0
//Wi-Fi module defines
#define MARVELL_WIFI        1                     /* SDIO */
#define REALTEK_WIFI        2                     /* SDIO */

#define RALINK_WIFI         1                     /* USB */
#define REALTEK_WIFI_USB    2                     /* USB */
#define AR2524_WIFI         3                     /* USB ZyDAS ZD1211B */
#define RALINK_AR2524_WIFI  4                     /* USB ZyDAS ZD1211B */
#define AR9271_WIFI  		5                     /* USB */
#define REALTEK_RTL8192SU   6                     /* USB */
#define REALTEK_RTL8188CU   7                     /* USB */
#define REALTEK_RTL8188EUS  8                     /* USB */
#define REALTEK_RTL8188E    8                     /* USB */

//Use for struct ST_TCON
#define TCON_NONE               0 //This one must be '0', others are sequence irrelative
#define TV_COMPOSITE_NTSC       1
#define TV_COMPOSITE_PAL        2
#define TV_S_VIDEO_NTSC         3
#define TV_S_VIDEO_PAL          4
#define TV_COMPONENT_PAL        5
#define TV_COMPONENT_480I       6
#define TV_COMPONENT_480P       7
#define TV_COMPONENT_720P       8
#define TV_COMPONENT_1080I      9
#define TV_D_SUB_480P           10
#define TV_D_SUB_720P           11
#define TV_D_SUB_XGA            12 //1024X768
#define TV_D_SUB_SVGA           13 //800X600
#define TV_D_SUB_QVGA           14 //1280X960
#define TV_D_SUB_SXGA           15  //1280X1024
#define TCON_MX88V44            16
#define TCON_HX8817A            17
#define TCON_MX88V430           18
#define TCON_MX88V430_WQVGA     19//480x234
#define TCON_MX88V431           20
#define TCON_MX88V431_WQVGA     21 //480x234
#define TCON_MX88V431_CCIR601   22
#define TCON_HX8819A            23
#define TCON_LQ080V3DG01        24 //Sharp 8"
#define TCON_ARK1829            25
#define TCON_INTERNAL           26
#define TCON_INTERNAL_800x600   27
#define TCON_RGB24Mode_640x480  28
#define TCON_RGB24Mode_800x600  29
#define TCON_RGB24Mode_1024x768 30
#define TCON_RGB24Mode_1280x800 31
#define TCON_INTERNAL_852x480   32
#define TCON_RGB24Mode_1024x600 33
#define TCON_RGB24Mode_800x480  34
#define TCON_RGB24Mode_400x234  35
#define TCON_RGB24Mode_320x240  36  //for ChiMei3.5" 320X240
#define TCON_INTERNAL_800x480  	37  //for CPT 800X480 on 650 demo board
#define TCON_LVDSMode_1024x600  38  //for CPT 1024X600 on 650 demo board
#define TCON_LVDSMode_1280x1024  39
#define TCON_RGB24Mode_800x600_3D  40
#define TCON_LVDSMode_1280x800  41
#define TCON_A040FL01           42
#define TCON_RGB24Mode_480x272  43
#define TCON_RGB24Mode_1280x720 44
#define TCON_LVDSMode_1680x1050 45  //for samsung
#define TCON_LVDSMode_1366x768  46  //for samsung
#define TCON_LVDSMode_1366x768_CPT 47 //for CPT
#define TCON_RGB8SerialMode_320x240 48
#define TCON_INTERNAL_480x640  	49 //for EPD 480x640
#define TCON_LVDSMode_1440x900 50
#define TCON_ITU709Mode_1280x720 51
#define TCON_RGB24Mode_240x400 52 //for ILI9327
#define TCON_RGB8SerialMode_480x240 53
#define TCON_I80Mode_128x128 54
#define TCON_RGB8SerialMode_320x240_ILI9342 55
#define TCON_RGB24Mode_FUJI_320x240 56

#define TCON_BIT1201           														57
#define TCON_RGB24Mode_1280x600 										58
#define TCON_RGB24Mode_N_640x480 									59
#define TCON_RGB24Mode_P_640x480 									60
#define TCON_RGB24Mode_480x800  										61



#define PANEL_NONE              0   //This one must be '0', others are sequence irrelative
#define PANEL_TV_4_3            1
#define PANEL_TV_16_9           2
#define PANEL_DI618_560009      3   //DataImage 5.6"
#define PANEL_DI618_680005      4   //DataImage 8" (800x600)
#define PANEL_A070FW03V2        5   //AUO 7"
#define PANEL_LT070W02TMC4      6   //LG 7"(26 pin)
#define PANEL_AT080TN03         7   //InnoLux 8" (800x480)
#define PANEL_LQ080V3DG01       8   //Sharp 8"
#define PANEL_A060FW02          9   //AU 6"
#define PANEL_LQ104V1DG51       10  //Sharp 10.4" (640x480)
#define PANEL_AT056TN04         11  //Innolux 5.6" (320x234)
#define PANEL_HT14x12           12  //ht14x12-101(1024x768)
#define PANEL_A121EW02          13  //AUO 12" WXGA (1280x800)
#define PANEL_FG100250DSCWA     14  //DataImage 10.2" (852x480)
#define PANEL_A080SN01          15  //AUO LED 8" 800x600
#define PANEL_A089SW01          16  //AUO 8.9" 1024x600
#define PANEL_C043GW01          17  //AUO 4.3" 400x234
#define PANEL_CLAA035QVA01      18  //for ChiMei3.5" 320X240
#define PANEL_LTM190E1L01      	19
#define PANEL_A040FL01          20  // for AUO 480x272
#define PANEL_HDMI_720P 		21
#define PANEL_B154SW01 		    22      //for samsung LVDS 1680x1050
#define PANEL_LTA320AP07 		23      //for samsung LVDS 1366x768
#define PANEL_CLAA101 		    24      //for samsung LVDS 1366x768
#define PANEL_A025DL02          25   //320x240
#define PANEL_ED024VC1          26   //EPD 480x640
#define PANEL_Q08009            27   //CHIMEI Q08009-602 / teco
#define PANEL_M190A1_L02		28		//for CHIMEI LVDS 1440x900
#define PANEL_G220SW01			29		//for AUO LVDS 1680x1050
#define PANEL_ADV7393			30		//for Ypbpr ADI ADV7393
#define PANEL_ILI9327			31		//240X400
#define PANEL_OTA5182A			32		//480X240
#define PANEL_ILI9163C			33		//132X160
#define PANEL_ILI9342			34		//320X240
#define PANEL_NOVA_39XX         35      //320X240
#define PANEL_720_480         36      //720X480
#define PANEL_AT102TN03         37      //800X480
#define PANEL_AT080TN42         38      //800X600
#define PANEL_RELAX040         	61      //480X800


///@ingroup USBOTG_INIT_API
///@brief   Enumeration constants of which one of usb otg.
typedef enum {
    USBOTG0,            ///< for USBOTG0
    USBOTG1,            ///< for USBOTG1
    USBOTG_NONE,        ///< it's initial value and total number of item of WHICH_OTG
}WHICH_OTG;


#define SYSTEM_DRIVE_NONE               0
#define SYSTEM_DRIVE_ON_NAND            1
#define SYSTEM_DRIVE_ON_SPI             2
#define SYSTEM_DRIVE_ON_SD              3

// select touch panel driver
#define TOUCH_PANEL_TYPE_RESISTIVE      0
#define TOUCH_PANEL_TYPE_CAPACITIVE     1

#define TOUCH_PANEL_DRIVER_AK4183           0
#define TOUCH_PANEL_DRIVER_A043VL01         1
#define TOUCH_PANEL_DRIVER_FT5306     	    2 //capacitive
#define TOUCH_PANEL_DRIVER_ICN8502     	    3
#define TOUCH_PANEL_DRIVER_GT911     	    4


#define ISP_TAG_XPG 0x53585047      //SXPG
#define ISP_TAG_XP1 0x53585031      //SXP1
#define ISP_TAG_XP2 0x53585032      //SXP2
#define ISP_TAG_XP3 0x53585033      //SXP3

#define PROGRASSIVE         0
#define INTERLACE           1

#define PANEL_MODE_DIGITAL  0   //Use for struct ST_PANEL
#define PANEL_MODE_STRIPE   1
#define PANEL_MODE_DELTA    2

#define I2C_DISABLE         0x0
#define I2C_HW_MASTER_MODE  0x1
#define I2C_HW_SLAVE_MODE   0x2
#define I2C_SW_MASTER_MODE  0x4
#define I2C_SW_SLAVE_MODE   0x8

#define DISPLAY_NOT_INIT                0
#define DISPLAY_INIT_LOW_RESOLUTION     1
#define DISPLAY_INIT_DEFAULT_RESOLUTION 2

#define CHIP_VER_612    0x06120000
#define CHIP_VER_615    0x06150000
#define CHIP_VER_600    0x06000000
#define CHIP_VER_620    0x06200000
#define CHIP_VER_630    0x06300000
#define CHIP_VER_650    0x06500000
#define CHIP_VER_660    0x06600000

#define CHIP_VER_A      0x00000000
#define CHIP_VER_B      0x00000001
#define CHIP_VER_C      0x00000002
#define CHIP_VER_D      0x00000006
#define CHIP_VER_FPGA   0x0000FFFF

#define CHIP_PACKAGE_BGA        0
#define CHIP_PACKAGE_PQFP       1
#define CHIP_PACKAGE_QFP_144    2
#define CHIP_PACKAGE_QFP_216A   3
#define CHIP_PACKAGE_QFP_216B   4

#define CHIP_VER_MSB            (CHIP_VER & 0xFFFF0000)
#define CHIP_VER_LSB            (CHIP_VER & 0x0000FFFF)

//for PRODUCT_ID
#define PRODUCT_UI            										(PRODUCT_ID & 0x000000FF) //BIT0-7   
#define PRODUCT_PCBA       										(PRODUCT_ID & 0x0000FF00) //BIT8-15
#define PRODUCT_PANEL       										(PRODUCT_ID & 0x00FF0000) //BIT16-23
#define PRODUCT_FUNC       										(PRODUCT_ID & 0xFF000000) //BIT24-31
//--PRODUCT_UI
#define	UI_NONE															0x00000000 // without xpg
#define UI_3_WINDOW            									0x00000001
#define UI_4_WINDOW            									0x00000002
#define UI_HXJ_1            												0x00000003 // wxj 201605  first ui for 8FT 5FT
#define UI_RELAX            												0x00000004
#define UI_WELDING           											0x00000005					// 20181203	two sensor+usb
#define UI_SURFACE           											0x00000008					//4  // ¶ËÃæ¼ì²â
//--PRODUCT_PCBA
#define PCBA_MP662_3_WIN       								0x00000100									
#define PCBA_MP662_4_WIN       								0x00000200									
#define PCBA_MP663_AV_USBCAM       						0x00000300							//MP663_AV_IN_OUT_20160124		
#define PCBA_MP662_AV_USBCAM       						0x00000400							//PCBA_MP662_AV_USBCAM 201607		
#define PCBA_MP662_RELAX       								0x00000500							// 201608		
#define PCBA_MAIN_BOARD_V10     							0x00000600							// 20181203	
#define PCBA_MAIN_BOARD_V11     							0x00000700							
#define PCBA_MAIN_BOARD_V12     							0x00000800							
#define PCBA_MAIN_BOARD_V20     							0x00000900							//Ò»Ìå°å
//--PRODUCT_PANEL
#define PANEL_NO_DISPLAY            							0x00000000
#define PANEL_640x480            									0x00010000
#define PANEL_800x480            									0x00020000
#define PANEL_800x600            									0x00030000
#define PANEL_1280x720            								0x00040000
#define PANEL_I_800x480            								0x00050000
#define PANEL_480x800            									0x00070000
#define PANEL_LVDS_1024x600            					0x00080000
//--PRODUCT_SUB_FUN
#define SF50_V10           												0x01000000
#define TCON_720P_FOR_600P           						0x02000000
//#define FUN_OC           													0x03000000					// 20181203	two sensor+usb


// for debug port select
#define DEBUG_PORT_HUART_A      0
#define DEBUG_PORT_HUART_B      1
#define DEBUG_PORT_USB_CDC      2
#define DEBUG_PORT_NONE         0xFF

// select audio dac
#define DAC_NONE		0
#define DAC_INTERNAL	1
#define DAC_AK4387		2
#define DAC_WS8956		3
#define DAC_AC97		4
#define DAC_WM8750L 	5
#define DAC_WM8961		6
#define HDMI_AD9889B	7
#define DAC_WM8960		8
#define DAC_CS4334		9
#define DAC_ES7240		10
#define DAC_WM8904		11
#define DAC_ES8328      12
#define DAC_ALC5621		13
#define DAC_ALC5633Q	14

// STD board version
#define MP612_NON_STD_SET                0
#define MP612_PQFP_MX430_0615MD01        1
#define MP612_BGA_HX8819_0627MD01        2
#define IPLAYB5_E_HD_0641MD03            3
#define MP650_FPGA                       4
#define MP652_216LQFP_32M_DDR            5
#define MP656_216LQFP_32M_DDR            6
#define MP661_128PQFP_16M_SDRAM          7
#define MP652_216LQFP_32M_DDR_SOCKET     8
#define MP660_144TQFP_32M_SDRAM          9
#define MP663_128BGA_16M_SDRAM          10
#define MP663_144BGA_32M_SDRAM          11
#define MP652_216LQFP_2LAYER            12
#define MP662_16M_SDRAM                 13
#define MP652_216LQFP_4LAYER 	        14 // Lj 2011/05/24 added
#define MP662_144TQFP_32M_MINIDV          15

#define MP662_YBD_CVBS_IN_OUT          20
#define MP662_YBD_FT102_RGB666          21
#define MP662_YBD_VGA_RGB666             22

// define bootup type
#define BOOTUP_TYPE_NOISP   0
#define BOOTUP_TYPE_NAND    1
#define BOOTUP_TYPE_NOR     2
#define BOOTUP_TYPE_SPI     3
#define BOOTUP_TYPE_SD      4       // T-Flash

// define Nand flash driver
#define NAND_SIMPLE_DRV     0
#define NAND_FTL_DRV        1

// from devio.h
#define LUN_NUM_0           0
#define LUN_NUM_1           1
#define LUN_NUM_2           2
#define LUN_NUM_3           3
#define LUN_NUM_4           4
#define LUN_NUM_5           5
#define LUN_NUM_6           6
#define NULL_LUN_NUM        0xff

//---------------------------------------------------------------------------------------
//--------   Supported NAND flash names   -----------------------------------------------
//---------------------------------------------------------------------------------------
// Hynix  (0x000 - 0x0ff)
// -- HY series (Old) --
#define HY27UG088G5M            0x000
#define HY27US08121B            0x001
#define HY27UT088G2M            0x002
#define HY27US08121A            0x003
#define HY27UH08AG5M            0x004
#define HY27UF081G2A            0x005
#define HY27UT084G2M            0x006
#define HY27UT084G2A            0x007
#define HY27US08281A            0x008
#define HY27US08561A            0x009
#define HY27UU08AG5M            0x00a
#define HY27UF084G2B            0x00b
#define HY27UF082G2A            0x00c
// -- H series (New) --
#define H27UAG8T2MTR            0x030
#define H27UAG8T2ATR            0X031
#define H27UBG8T2ATR            0x032

#define H27UAG8T2BTR			0x033

#define H27UBG8T2BTR            0x034

#define H27UAG8T2CTR            0x035

// Micron (0x100 - 0x1ff)
#define MICRON_29F8G08MAD       0x100
#define MICRON_29F16G1608MAA    0x101
#define MICRON_29F64G08CFABA    0x102
#define MICRON_29F128G08CJABA   0x103
#define MICRON_29F32G080CBAAA   0x104

// Toshiba (0x200 - 0x2ff)
#define TC58NVG0S3ETA00         0x200
#define TC58NVG4D2ETA00         0X201


// Samsung (0x300 - 0x3ff)
#define K9G4G08U0A              0x300
#define K9F2G08U0A              0x301
#define K9GAG08U0M              0x302
#define K9K8G08U0A              0x303
#define K9G8G08U0C              0x304
#define K9F1G08U0B              0x305
#define K9F1208U0M              0x306
#define K9F1G08U0A              0x307
#define K9G4G08U0B              0x308
#define K9LBG08U0D              0x309
#define K9F5608U0D              0X30a
#define K9GAG08U0D              0x30b
#define K9GAG08U0E              0x30c
#define K9GBG08U0A              0X30d
#define K9LCG08U0A              0x30e
#define K9GAG08U0F              0x30f

// MXIC (0x400 - 0x4ff)
#define MX30LF1G8AM				0x400
#define MX30LF1208AA			0x401
#define MX30LF1G08AA			0x402

// Other customer (0xf00 - 0xffe)
#define NAND128W3A2BN6          0xF00

// Default (0xfff)
#define COMMON_NAND             0xfff
//---------------------------------------------------------------------------------------

//OSD
//------------------------------
//  Setup values
//------------------------------
/*
We're currently using three types of setup menu, description as below:
1. IDU style: Get the setup item from XPG file and draw it on IDU buffer, 24 bit colo
   supported, extra buffer needed.
2. OSD style: Draw the geometrical image to OSD buffer to complete the setup menu, OSD
   index used, w/o extra buffer
3. MIX style: Get the setup item from XPG file and draw it's "shape" only. The color is
   indicated by appilication. The setup menu will be draw on OSD buffer. w/o extra
   buffer.
*/
#define SETUP_IDU_STYLE         0
#define SETUP_OSD_STYLE         1
#define SETUP_MIX_STYLE         2
//#define XPG_IDU_SETUP_ENABLE    0

#define    REMOTE_NONE                  0
#define    REMOTE_MPX_STANDARD_28KEYS   1
#define    REMOTE_MPX_STANDARD_24KEYS   2
#define    REMOTE_WA_15KEYS             3
#define    REMOTE_LK                    4
#define    REMOTE_CS_24KEYS             5
#define    REMOTE_SONY_RMT_DPF          6       // SIRC 20bits
#define    REMOTE_DPF_21KEYS            7       // Lj 2011/05/24 added
#define    REMOTE_MP662W_MINIDV         8       // Lj 2011/05/24 added

#define    PID_IR_CR2025                     21
#define    REMOTE_YBD_4WIN                     22

#define    REMOTE_PLATFORM              0xff

//-------------------------------------------------
//  For SS_DPF using
//-------------------------------------------------

#define	UI_SZ_480X234	0
#define	UI_SZ_800X480	1
#define	UI_SZ_800X600	2
#define	UI_SZ_1024X600	3

//******************************
// These items conflict with filebrowser.h 40 : enum...
//#define PHOTO_MODE 		0
//#define MUSIC_MODE 		1
//#define VIDEO_MODE 		2
//#define FILE_MODE 		3
//#define SETUP_MODE      4
//#define NUM_OF_MODE     5
//*******************************

#define SLIDESHOW	0
#define PAUSE		1
#define PREV			2
#define NEXT			3

#define RANDOM_DIRECTION	0
#define PUSH_LEFT_RIGHT	2
#define PUSH_RIGHT_LEFT	3


//Font
//------------------------------
//  Font Attribute values
//------------------------------
#define FONT_ATTR_NONE          0x00000000
#define FONT_ATTR_CENTERALIGN   0x00000001
//------------------------------
//  Font size values
//------------------------------
// For SS project
#define FONT_SIZE44 44	// 44x44 pixel
#define FONT_SIZE22 22	// 22x22 pixel
#define FONT_SIZE18 18	// 18x18 pixel

#define GPIO_00     (0)
#define GPIO_01     (1)
#define GPIO_02     (2)
#define GPIO_03     (3)
#define GPIO_04     (4)
#define GPIO_05     (5)
#define GPIO_06     (6)
#define GPIO_07     (7)
#define GPIO_08     (8)
#define GPIO_09     (9)
#define GPIO_10     (10)
#define GPIO_11     (11)
#define GPIO_12     (12)
#define GPIO_13     (13)
#define GPIO_14     (14)
#define GPIO_15     (15)

#define GPIO_16     (16)
#define GPIO_17     (17)
#define GPIO_18     (18)
#define GPIO_19     (19)
#define GPIO_20     (20)
#define GPIO_21     (21)
#define GPIO_22     (22)
#define GPIO_23     (23)
#define GPIO_24     (24)
#define GPIO_25     (25)
#define GPIO_26     (26)
#define GPIO_27     (27)
#define GPIO_28     (28)
#define GPIO_29     (29)
#define GPIO_30     (30)
#define GPIO_31     (31)

#define GPIO_32     (32)
#define GPIO_33     (33)
#define GPIO_34     (34)
#define GPIO_35     (35)
#define GPIO_36     (36)
#define GPIO_37     (37)
#define GPIO_38     (38)
#define GPIO_39     (39)
#define GPIO_40     (40)
#define GPIO_41     (41)
#define GPIO_42     (42)
#define GPIO_43     (43)
#define GPIO_44     (44)
#define GPIO_45     (45)
#define GPIO_46     (46)
#define GPIO_47     (47)

#define GPIO_48     (48)
#define GPIO_49     (49)
#define GPIO_50     (50)
#define GPIO_51     (51)
#define GPIO_52     (52)
#define GPIO_53     (53)
#define GPIO_54     (54)
#define GPIO_55     (55)
#define GPIO_56     (56)
#define GPIO_57     (57)
#define GPIO_58     (58)
#define GPIO_59     (59)
#define GPIO_60     (60)
#define GPIO_61     (61)
#define GPIO_62     (62)
#define GPIO_63     (63)

#define GPIO_INPUT  0
#define GPIO_OUTPUT 1

#define NORMAL_GPIO 0x0000
#define FGPIO       0x0100
#define UGPIO       0x0200
#define VGPIO       0x0300
#define AGPIO       0x0400
#define KGPIO       0x0500
#define OGPIO       0x0600
#define PGPIO       0x0700
#define MGPIO       0x0800
#define GPIO_NULL   0xFFFF

// IPU
#define NON_SCALING     0x00
#define SCALING_UP      0x02
#define SCALING_DOWN    0x01

#define FULL_SCALING    0

// IMAGE
#define IMG_DECODE_PHOTO                    0x00
#define IMG_DECODE_SLIDE                    0x01
#define IMG_DECODE_MJPEG                    0x02
#define IMG_DECODE_XPG                      0x03
#define IMG_DECODE_THUMB                    0x04
#define IMG_DECODE_THUMB1                   0x05
#define IMG_DECODE_THUMB2                   0x06
#define IMG_DECODE_ROTATE_CLOCKWISE         0x10
#define IMG_DECODE_ROTATE_UPSIDEDOWN        0x20
#define IMG_DECODE_ROTATE_COUNTERCLOCKWISE  0x30
#define IMG_DECODE_CHASE                    0x80
#define IMG_DECODE_ONEPASS                  0x40    // Progressive


// MP_API
#define EVENT_ENTER_MOVIE_PLAYER    (1<<0)
#define EVENT_EXIT_MOVIE_PLAYER     (1<<1)
#define EVENT_MOVIE_PLAY            (1<<2)
#define EVENT_START_AUDIODECODE     (1<<3)
#define	EVENT_MOVIE_CLOSE           (1<<4)

//MOVIE_STATUS
#define	EVENT_MOVIE_STATUS_CLOSED			(1<<0)
#define	EVENT_MOVIE_STATUS_NOT_START_YET	(1<<1)
#define	EVENT_MOVIE_STATUS_SEEK_COMPLETED	(1<<2)
#define	EVENT_MOVIE_STATUS_STOPED			(1<<3)


//AV_DECODER_EVENT
#define	EVENT_V_DECODER_START       (1<<0)
#define	EVENT_V_DECODER_EOF         (1<<1)
#define	EVENT_V_NOT_DECODING        (1<<3)
#define	EVENT_A_DECODER_START       (1<<4)
#define	EVENT_A_DECODER_EOF         (1<<5)
#define	EVENT_A_NOT_DECODING        (1<<7)
#define	EVENT_V_DECODER_HDRS_RDY	(1<<8)

//AVP_SYNC_EVENT
#define	EVENT_V_DECODED				(1<<0)
#define	EVENT_V_PLAYED				(1<<1)
#define	EVENT_V_WAIT_FOR_PLAY		(1<<2)
#define	EVENT_V_ENOUGH_TO_PLAY		(1<<3)
#define	EVENT_A_DECODED				(1<<4)
#define	EVENT_A_PLAYED				(1<<5)
#define	EVENT_A_START_PLAY			(1<<6)
#define	EVENT_AV_EOF				(1<<8)
#define	EVENT_V_WAIT_A				(1<<9)
#define	EVENT_AV_STOP				(1<<10)

//VIDEO_DATA_EVENT
#define	EVENT_VD_READ				(1<<0)
#define	EVENT_VD_READ_COMPLETED		(1<<1)



// the following definitions are used for indicate whether relative member of Media_Infor is useful
#define MOVIE_TotalTime_USEFUL      (1<<0)

//video special
#define MOVIE_ImageWidth_USEFUL     (1<<1)
#define MOVIE_ImageHeight_USEFUL    (1<<2)
#define MOVIE_TotalFrame_USEFUL     (1<<3)
#define MOVIE_FrameRate_USEFUL      (1<<4)
//video content
#define MOVIE_Title_USEFUL          (1<<5)
#define MOVIE_Author_USEFUL         (1<<6)
#define MOVIE_Copyright_USEFUL      (1<<7)
#define MOVIE_Rating_USEFUL         (1<<8)
#define MOVIE_Description_USEFUL    (1<<9)


//audio special
#define MOVIE_Bitrate_USEFUL        (1<<10)
#define MOVIE_SampleRate_USEFUL     (1<<11)
#define MOVIE_SampleSize_USEFUL     (1<<12)

//audio content
#define MOVIE_ID3_Title_USEFUL      (1<<13)
#define MOVIE_ID3_Artist_USEFUL     (1<<14)
#define MOVIE_ID3_Album_USEFUL      (1<<15)
#define MOVIE_ID3_Year_USEFUL       (1<<16)
#define MOVIE_ID3_Comment_USEFUL    (1<<17)
#define MOVIE_ID3_Track_USEFUL      (1<<18)
#define MOVIE_ID3_Genre_USEFUL      (1<<19)


//whether has video/audio relative infor
#define MOVIE_INFO_WITH_AUDIO       (1<<30)
#define MOVIE_INFO_WITH_VIDEO       (1<<31)


// the following instructions are used by the AP to control the movie
#define MOVIE_VIDEO_PLAY            1
#define MOVIE_VIDEO_STOP            2
#define MOVIE_VIDEO_FORWARD         3
#define MOVIE_VIDEO_BACKWARD        4
#define MOVIE_VIDEO_FAST_FORWARD    5
#define MOVIE_VIDEO_FAST_BACKWARD   6
#define MOVIE_VIDEO_TO_BEGIN        7
//#define MOVIE_VIDEO_TO_END          8
#define MOVIE_VIDEO_EJECT           9
#define MOVIE_VIDEO_PAUSE           10
#define MOVIE_VIDEO_CLOSE           11
#define MOVIE_VIDEO_CONTINUE        12
//#define MOVIE_VIDEO_SEEK            13
#define MOVIE_VIDEO_EXITED          14
#define MOVIE_VIDEO_ENTER           15
#define MOVIE_AUDIO_RESUME          16


// the following instructions are used by the movie driver itself
#define MOVIE_VIDEO_TRACKING        100
//#define MOVIE_VIDEO_SHOW          101


//--------------------------------------------
// xpg repaint flag -- g_boNeedRepaint
//--------------------------------------------
#define XPG_REPAINT_NONE            0 // g_boNeedRepaint = false
#define XPG_REPAINT_ALL             1 // g_boNeedRepaint = true
#define XPG_REPAINT_LIST            2 // in xpgBrowser.c, xpgMenuListSetIndex()
#define XPG_REPAINT_NOLIST          3 // xpgDrawSprite_List don't update


//Non XPG mode
#define 	NON_XPG_MODE_NULL			            				0
#define 	NON_XPG_MODE_AVIN			            				1
#define 	NON_XPG_MODE_WEBCAM			            			2
#define 	N0N_XPG_MODE_PHOTOVIEW	            			3
#define 	N0N_XPG_MODE_VIDEOVIEW	            			4
#define 	N0N_XPG_MODE_SETUP	            						5

// XPG
// xpg flag
#ifndef XPG_MODE_NULL
        // define main flow mode status ID
        #define 	XPG_MODE_NULL			            0
        #define 	XPG_MODE_CARDSEL		            1
        #define		XPG_MODE_MODESEL		            2
        #define		XPG_MODE_PHOTOMENU		            3
        #define		XPG_MODE_MUSICMENU		            4
        #define		XPG_MODE_VIDEOMENU		            5
        #define		XPG_MODE_FILEMENU		            6
        #define		XPG_MODE_PHOTOVIEW		            7
        #define		XPG_MODE_MUSICPLAYER	            8
        #define		XPG_MODE_VIDEOPLAYER	            9
        #define		XPG_MODE_SLIDESHOW		            10
        #define	    XPG_MODE_BLUETOOTH                  11  //for setupmenu <=18
        #define		XPG_MODE_SETUPMENU		            12
        #define		XPG_MODE_ENTERVIDEOPLAYER	        13
        #define		XPG_MODE_MUSICSETTING	            14
        #define 	XPG_MODE_TARGETCARDSEL              15
        #define 	XPG_MODE_COPYFILE			        16
        #define 	XPG_MODE_DELETEFILE		            17
        #define 	XPG_MODE_SETUP				        18
        #define		XPG_MODE_INFOMENU			        19
        #define		XPG_MODE_POPMENU			        20
        #define 	XPG_MODE_PHOTOZOOM		            21
        #define		XPG_MODE_PHOTOROTATE	            22
        #define   XPG_MODE_CALENDAR		                23 // CALENDARWHITE

        #define   XPG_MODE_CALPHOTO		                24
        #define   XPG_MODE_PRINTER                      25

        #define   XPG_MODE_NET_AP    			        26
        #define   XPG_MODE_NET_FUNC   		            27

        #define   XPG_MODE_NET_PC_SERVER	            28
        #define   XPG_MODE_NET_KEY     	 	            29
        #define   XPG_MODE_NET_PC_AUDIO                 30
        #define   XPG_MODE_NET_RSS                      31

        #define   XPG_MODE_NET_PC_PHOTO                 32
        #define   XPG_MODE_NET_FLICKR			        33
        #define   XPG_MODE_NET_PICASA 		            34
        #define   XPG_MODE_RSS_CHANNEL                  35  			// 1

        #define	  XPG_MODE_FLICKR_USER_LIST	            36  			// 1
        #define   XPG_MODE_PICASA_USER_LIST	            37          		// 1
        #define   XPG_MODE_YOUGOTPHOTO_USER_LIST	    38   		// 1
        #define   XPG_MODE_RSS_TITLE			        39

        #define   XPG_MODE_NET_FTPSERVER	            40
        #define   XPG_MODE_NET_GCE				        41
        #define   XPG_MODE_NET_YOUGOTPHOTO              42
        #define	  XPG_MODE_FLICKR_PHOTOSET	            43

        #define	  XPG_MODE_PICASA_PHOTOSET	            44
        #define	  XPG_MODE_YOUGOTPHOTO_PHOTOSET	        45
        #define	  XPG_MODE_INTERNET_RADIO		        46                  // 1
        #define	  XPG_MODE_IRADIO_STATION		        47

        #define	  XPG_MODE_IRADIO_PLS				    48
        #define	  XPG_MODE_FRAMECHANNEL                 49
        #define	  XPG_MODE_FRAMECHANNEL_USER_LIST	    50
        #define	  XPG_MODE_FRAMECHANNEL_CHANNELSET	    51

        #define   XPG_MODE_NW_WIFIKEYSetup              52
        #define   XPG_MODE_NW_DHCPSetup                 53
        #define 	XPG_MODE_VTUNER                     54
        //*******************************************************************

        #define 	XPG_MODE_USEREDIT   			    56
        #define	  XPG_MODE_UPNP_USER_LIST			    57
        #define	  XPG_MODE_UPNP_FILE_LIST			    58
        #define	  XPG_MODE_SHUTTERFLY        	        59
        #define   XPG_MODE_SHUTTERFLY_USER_LIST	        60
        #define	  XPG_MODE_NET_SHUTTERFLY   		    61
        #define	  XPG_MODE_SHUTTERFLY_PHOTOSET	        62

        #define 	XPG_MODE_FRAMEIT   				    63
        #define	  XPG_MODE_FRAMEIT_USER_LIST		    64
        #define   XPG_MODE_FRAMEIT_COLLECTIONSET        65
        #define   XPG_MODE_FRAMEIT_GETCLAIMTOKEN        66
        #define	  XPG_MODE_WPSSET	    				67

        //*******************************************************************
        #define	  XPG_MODE_SNAPFISH_USER_LIST	        68
        #define   XPG_MODE_SNAPFISH_LOGIN 		        69
        #define   XPG_MODE_SNAPFISH_PHOTOSET	        70
        #define   XPG_MODE_SNAPFISH                     71

        #define 	XPG_MODE_VTUNER_LOCATION            72
        #define 	XPG_MODE_GPRS_PLUGIN                74
        #define 	XPG_MODE_DLNA_1_5_DMR			    75

        #define		XPG_MODE_VIDEOPREVIEW	            81
        #define   XPG_MODE_DIGITCLOCK                   82
        #define   XPG_MODE_ANALOGCLOCK                  83
        #define   XPG_MODE_CALENDARBLACK                84
        #define   XPG_MODE_CALENDARWHITE                85
        #define   XPG_MODE_FLASHCLOCKBLACK              86
        #define   XPG_MODE_FLASHCLOCKWHITE              87
        #define   XPG_MODE_SYSTEMABOUT                  88
        #define   XPG_MODE_WEEKPHOTO                    89

        #define		XPG_MODE_YOUTUBEFAVOR	            95
        #define		XPG_MODE_YOUTUBEFAVORSHOW      	    96
        #define		XPG_MODE_YOUTUBETHUMB	            97
        #define		XPG_MODE_YOUTUBETHUMBSHOW           98
        #define		XPG_MODE_YOUTUBELOGIN	            99

	   #define 	XPG_MODE_WORD				100
       #define		XPG_MODE_YOUTUBETYPE	            101

       #define		XPG_MODE_MYFAVOR           102
       #define		XPG_MODE_NETAUDIO           103

       #define		XPG_MODE_YAHOOKIMO           104
       #define		XPG_MODE_YAHOOKIMO_STOCK     105

       #define		XPG_MODE_NETPHOTOVIEW        106
       #define		XPG_MODE_NETSLIDESHOW        107

       #define		XPG_MODE_3G     			108

       #define		XPG_MODE_YOUKUFAVOR	        111
       #define		XPG_MODE_YOUKUFAVORSHOW     112
       #define		XPG_MODE_YOUKUTHUMB	        113
       #define		XPG_MODE_YOUKUTHUMBSHOW     114
       #define		XPG_MODE_YOUKULOGIN	        115

       // ====== MINI-DV ====== //
       #define		XPG_MODE_PAUSE	            116
       #define		XPG_MODE_CAMERA	            117
       #define      XPG_MODE_PREVIEW            118

       //------------------------------------------------------
       #define		XPG_MODE_PHOTO_LOCAL_RECURSIVE_SEARCH   119 // in xpgDrawSprite.c, sprite type43=AniOnePhoto
       #define      XPG_MODE_RECORDING            120
       #define      XPG_MODE_CLOCK                121
        #define		XPG_MODE_MYFAVORPHOTO         123
	#define 	XPG_MODE_MAINMENU		124
        #define 	XPG_MODE_DELETEFILE_CONFIRM		            125
        #define 	XPG_MODE_DAILOG		            						126
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

#define KEY_CAM                     38
#define KEY_CAM_VIEW                     39

#define KEY_CONTRAST              			40
#define KEY_DISPLAYMODE               41

#define KEY_WINDOWMODE               42
#define KEY_DEL_WIN_1               		43
#define KEY_DEL_WIN_2               		44
#define KEY_DEL_WIN_3               		45
#define KEY_DEL_WIN_4               		46
#define KEY_DEL_PAGE               		47
#define KEY_DEL_ALL               			48

#define KEY_CAPTURE               			50
#define KEY_RECORD               			51
#define KEY_FREEZE_DISPLAY   			52
#define KEY_PLAY_PHOTO   					53
#define KEY_PLAY_VIDEO   					54
#define KEY_PLAYBACK   						55
#define KEY_LED   									56

//#define KEY_ALARM                   127
// number key start from 100 //for key in poker

#define KEY_PP                      KEY_ENTER


// for Key repeat and hold
#define XPG_EVENT_KEY_DOWN		0
#define XPG_EVENT_KEY_UP		1
#define XPG_EVENT_KEY_REPEAT	2
//#define XPG_EVENT_KEY_HOLD1		3
//#define XPG_EVENT_KEY_HOLD2		4
//#define XPG_EVENT_KEY_HOLD3		5
//#define XPG_EVENT_ENTERPAGE		6
//#define XPG_EVENT_EXITPAGE		7

// for Draw Role Different ink
#define XPG_INK_TRANS    10
#define XPG_INK_DARKEN   11
#define XPG_INK_WHITEN   12
#define XPG_INK_COLOR    13
#define XPG_INK_BLEND    14

#define ANI_AUDIO           BIT0
#define ANI_VIDEO           BIT1
#define ANI_SLIDE           BIT2
#define ANI_ICON		    BIT3
#define ANI_ScreenSaver     BIT4
#define ANI_SLIDE_EFFECT    BIT5
#define ANI_READY           BIT6
#define ANI_PAUSE           BIT7
#define ANI_VIDEO_PREVIEW   BIT8
#define ANI_FILEMUSIC               BIT9
#define ANI_PAGEMUSIC               BIT10
#define ANI_MULTIPAGES_FILEMUSIC    BIT11
#define ANI_MULTIPAGES_PAGEMUSIC    BIT12
// for MINI-DV
#define ANI_CAMRECORDING            BIT13
#define ANI_PREVIEW                 BIT14
#define ANI_CAPTURE                 BIT15
#define ANI_PLAYBACK                BIT16
#define ANI_DELETE                  BIT17
#define ANI_RESOLUTION              BIT18
#define ANI_PAUSERESUME              BIT19


#define ANI_AV (ANI_AUDIO | ANI_VIDEO| ANI_READY)

#define MOVIE_STATUS_NULL					0
#define MOVIE_STATUS_PLAYER					BIT0 //enter_movie_player status
#define MOVIE_STATUS_PLAY					BIT1 //move_play status
#define MOVIE_STATUS_STOP					BIT2
#define MOVIE_STATUS_FORWARD				BIT3
#define MOVIE_STATUS_BACKWARD				BIT4
#define MOVIE_STATUS_FAST_FORWARD			BIT5
#define MOVIE_STATUS_FAST_BACKWARD			BIT6
#define MOVIE_STATUS_TO_BEGIN				BIT7
#define MOVIE_STATUS_TO_END					BIT8
#define MOVIE_STATUS_EJECT					BIT9
#define MOVIE_STATUS_PAUSE					BIT10
#define MOVIE_STATUS_CLOSE					BIT11
#define MOVIE_STATUS_CONTINUE				BIT12
#define MOVIE_STATUS_SEEK					BIT13
#define MOVIE_STATUS_EXITED					BIT14
#define MOVIE_STATUS_PREVIEW				BIT15
#define MOVIE_STATUS_PARSING				BIT16
#define MOVIE_STATUS_DRAW_THUMB             BIT17
#define MOVIE_STATUS_FULL_SCREEN            BIT18

#define ALIGN_4096(a)       (((a) + 4095) & 0xfffff000)

#define ALIGN_256(a)        (((a) + 255) & 0xffffff00)
#define ALIGN_32(a)         (((a) + 31) & 0xffffffe0)

#define ALIGN_16(a)         (((a) + 15) & 0xfffffff0)
#define ALIGN_8(a)          (((a) + 7) & 0xfffffff8)

#define ALIGN_4(a)          (((a) + 3) & 0xfffffffc)
#define ALIGN_2(a)          (((a) + 1) & 0xfffffffe)

#define ALIGN_CUT_256(a)    ((a) & 0xffffff00)
#define ALIGN_CUT_128(a)     ((a) & 0xffffff80)
#define ALIGN_CUT_64(a)     ((a) & 0xffffffc0)
#define ALIGN_CUT_32(a)     ((a) & 0xffffffe0)
#define ALIGN_CUT_16(a)     ((a) & 0xfffffff0)
#define ALIGN_CUT_8(a)      ((a) & 0xfffffff8)
#define ALIGN_CUT_4(a)      ((a) & 0xfffffffc)
#define ALIGN_CUT_2(a)      ((a) & 0xfffffffe)

#endif  //_MP650_FLAG__DEFINE_H__

