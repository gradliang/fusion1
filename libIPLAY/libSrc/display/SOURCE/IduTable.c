#include "global612.h"
#include "flagdefine.h"
#include "idu.h"

  //  rio -w b001e01c 000201e0 rio -w b001e018 000201e0
  // b001e010 00190014     b001e014 0064031f    b001e018 000a0009    b001e01c 000201e0    //20 25 100     9 10 2
// H b001e010 002d0013  0063031f    V:000800b 00020031f    H 20 46 100  V 11 8 2
// rio -w b001e000 08200011   08210011(ori)  rio -w b001e000 08300011

/*********maybe OK********/
//H  rio -w b001e010 00190014      b001e014 0001031f  //20+1  25+1  16+1
//V  rio -w b001e018 0008000c      b001e01c 000101e0  //12+1  8+1   16+1

//Under HSYNC & DE OK
// rio -w b001e010 00190014         b001e014 0001031f
//b001e018 00160010                    b001e01c 000201e0          
//--check OK
//b001e010 000f001e(ori-000f001d)    b001e014 00d1031f   (30,15,209 )    {31,16,210,13,23,22}
//b001e018 0016000c(ori-0009000c)   b001e01c 001501df    (12,22,21)

ST_TCON g_mstTCON[] = {//HSync, Hback, Hfront, VSync, Vback, Vfront
    //{TCON_RGB24Mode_800x600,DISP_TYPE_DVI_888, TCON_CLKSRC_AUTO, PROGRASSIVE, 390, 390,  800,  600, 0x00064d1a, {32, 80, 48, 2, 33, 10}},   // Lighter 11/06
    {TCON_MX88V430,     DISP_TYPE_CCIR656,   TCON_CLKSRC_AUTO, INTERLACE,   135, 270,  720,  480, BYPASS,     {2, 118, 18, 3, 18, 1}},
    {TCON_MX88V44,      DISP_TYPE_CCIR656,   TCON_CLKSRC_AUTO, INTERLACE,   135, 270,  720,  480, BYPASS,     {2, 118, 18, 3, 18, 1}},
    {TCON_HX8819A,      DISP_TYPE_DVI_666,   TCON_CLKSRC_AUTO, PROGRASSIVE, 333, 333,  800,  480, BYPASS,     {136, 40, 24, 6, 31, 23}},
    {TCON_LQ080V3DG01,  DISP_TYPE_DVI_888,   TCON_CLKSRC_AUTO, PROGRASSIVE, 270, 270,  640,  480, BYPASS,     {32, 80, 48, 2, 33, 10}},
    {TCON_MX88V431,     DISP_TYPE_CCIR656,   TCON_CLKSRC_AUTO, INTERLACE,   135, 270,  720,  480, 0x0002230f,     {2, 118, 18, 3, 18, 1}},          //griffy update from MP600
    {TCON_MX88V431_WQVGA,     DISP_TYPE_CCIR656,   TCON_CLKSRC_AUTO, PROGRASSIVE,   120, 240,  480,  240, BYPASS,     {2, 118, 18, 3, 18, 1}},  //griffy update from MP600
    {TCON_MX88V431_CCIR601,     DISP_TYPE_CCIR601,   TCON_CLKSRC_AUTO, INTERLACE,   135, 270,  720,  480, BYPASS,     {2, 118, 18, 3, 18, 1}},  //griffy update from MP600
    {TCON_BIT1201,     DISP_TYPE_CCIR656,   TCON_CLKSRC_AUTO, INTERLACE,   135, 270,  720,  480, BYPASS,     {2, 118, 18, 3, 18, 1}},
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
    {TCON_INTERNAL_800x600,      DISP_TYPE_DVI_666,   TCON_CLKSRC_AUTO, PROGRASSIVE, 400, 400,  800,  600, BYPASS,     {48, 40, 40, 3, 29, 13}},
    //{TCON_INTERNAL_800x480,      DISP_TYPE_DVI_666,   TCON_CLKSRC_AUTO, PROGRASSIVE, 450, 450,  800,  480, BYPASS,     {48, 40, 40, 3, 29, 13}},	//griffy add
    {TCON_INTERNAL_800x480,      DISP_TYPE_DVI_666,   TCON_CLKSRC_AUTO, PROGRASSIVE, 400, 400,  800,  480, BYPASS,     {48, 40, 40, 3, 29, 13}},        //griffy update from MP600
    {TCON_INTERNAL_480x640,      DISP_TYPE_DVI_666,   TCON_CLKSRC_AUTO, PROGRASSIVE, 200, 200,  480,  640, BYPASS,     {30, 52, 52, 6, 34, 34}},	//Polun add for EPD 
#else
    {TCON_INTERNAL,      DISP_TYPE_DVI_666,   TCON_CLKSRC_AUTO, PROGRASSIVE, 333, 333,  800,  480, BYPASS,     {48, 40, 40, 3, 29, 13}},        //griffy update from MP600
    {TCON_INTERNAL_800x600,      DISP_TYPE_DVI_666,   TCON_CLKSRC_AUTO, PROGRASSIVE, 333, 333,  800,  600, BYPASS,     {48, 40, 40, 3, 29, 13}},//griffy update from MP600
    {TCON_INTERNAL_852x480,      DISP_TYPE_DVI_666,   TCON_CLKSRC_AUTO, PROGRASSIVE, 333, 333,  852,  480,  0x0006240E,     {1, 180, 23, 1, 4, 40}},//griffy update from MP600
#endif
#if New_RGB24
    //{TCON_RGB24Mode_640x480,  DISP_TYPE_DVI_666,   TCON_CLKSRC_AUTO, PROGRASSIVE, 270, 270,  640,  480, 	BYPASS,     {48, 40, 41, 3, 29, 13}},
	{TCON_RGB24Mode_640x480,      DISP_TYPE_RGB24,   TCON_CLKSRC_AUTO, PROGRASSIVE, 270/*250*/, 270/*250*/,  640,  480, BYPASS,     {48, 40, 40, 3, 29, 13}}, // griffy add AUO 7" for MP652 demo board 
    #if (PANEL_ID == PANEL_Q08009)
    {TCON_RGB24Mode_800x600,  DISP_TYPE_RGB24,  TCON_CLKSRC_AUTO, PROGRASSIVE, 400, 400, 800, 600, BYPASS,     {1, 46, 210, 1, 23, 12}}, 
	#elif (PRODUCT_SUB_FUN==TCON_720P_FOR_600P)
	{TCON_RGB24Mode_800x600,  DISP_TYPE_RGB24,   TCON_CLKSRC_AUTO, PROGRASSIVE, 	270, 270,  1280,  720, 	BYPASS,     {32, 240, 94, 3, 10, 15}},
	#elif MINIDV_YBD_FUNCION
    {TCON_RGB24Mode_800x600,  DISP_TYPE_RGB24,   TCON_CLKSRC_AUTO, PROGRASSIVE,  270,   270,    800,    600, 	BYPASS,     {120, 40, 25, 3, 1, 24} },//{128, 88, 40, 3, 1, 24}  
		#else
    {TCON_RGB24Mode_800x600,  DISP_TYPE_RGB24,   TCON_CLKSRC_AUTO, PROGRASSIVE,  400/*360*/,   400/*360*/,    800,    600, 	BYPASS,     {120, 40, 25, 3, 1, 24} },//{128, 88, 40, 3, 1, 24}  
    #endif
    {TCON_RGB24Mode_800x600_3D,	DISP_TYPE_RGB24,	TCON_CLKSRC_AUTO,	PROGRASSIVE,	400,	400,	800,	600,	BYPASS,    	{48, 88, 112, 3, 12, 13}}, 
 //-----FUJI test-----//
	{TCON_RGB24Mode_FUJI_320x240,     DISP_TYPE_RGB24,   TCON_CLKSRC_AUTO, PROGRASSIVE,   64, 64,  320,  240,	0x01060F0E,	{10, 60, 18, 2, 11, 10}},  //For Qisda DAB_Jimmy
 //-----------------//
 // -----I80 panel-----//
	{TCON_I80Mode_128x128,	DISP_TYPE_I80,	TCON_CLKSRC_AUTO,	PROGRASSIVE,	180,	180,	128*3,	128*2,	BYPASS,    	{48, 40, 41, 3, 12, 13}}, //{48, 40, 41, 3, 12, 13}
 //-----------------//
 //	{TCON_RGB24Mode_320x240,     DISP_TYPE_RGB24,   TCON_CLKSRC_AUTO, PROGRASSIVE,   64, 64,  320,  240, BYPASS,/*0x01060F0E*/     {30, 39, 19, 3, 15, 4}},  //For ChiHsin   Griffy
	{TCON_RGB24Mode_320x240,     DISP_TYPE_RGB24,   TCON_CLKSRC_PLL2, PROGRASSIVE,   65, 65,  320,  240, 0x0006070E,    {30,39,29, 3, 15, 4}},//For ChiHsin  Wendy
  	{TCON_RGB24Mode_800x480,      DISP_TYPE_RGB24,   TCON_CLKSRC_AUTO, PROGRASSIVE, 200/*333*/, 200/*333*/,  800,  480, BYPASS,     {48, 40, 40, 3, 29, 13}}, // griffy add AUO 7" for MP652 demo board 
  	{TCON_RGB24Mode_480x800,      DISP_TYPE_RGB24,   TCON_CLKSRC_AUTO, PROGRASSIVE, 200/*333*/, 200/*333*/,  480,  800, BYPASS,     {48, 40, 40, 3, 29, 13}},
    {TCON_RGB24Mode_N_640x480,  DISP_TYPE_RGB24,   TCON_CLKSRC_AUTO, PROGRASSIVE, 280, 280,  640,  480, 	BYPASS,     {48, 40, 41, 3, 29, 13}},//NTSC
    {TCON_RGB24Mode_P_640x480,  DISP_TYPE_RGB24,   TCON_CLKSRC_AUTO, PROGRASSIVE, 230, 230,  640,  480, 	BYPASS,     {48, 40, 41, 3, 29, 13}},//PAL
  	{TCON_RGB24Mode_240x400,  DISP_TYPE_RGB24,   TCON_CLKSRC_PLL2, PROGRASSIVE,   135, 135,  240, 400, BYPASS,    {5, 180, 5, 2, 2, 4}}, 
  	{TCON_LVDSMode_1024x600, DISP_TYPE_LVDS_SINGLE_666,   TCON_CLKSRC_AUTO, PROGRASSIVE,   3150, 3150,  1024,  600, BYPASS,     {40, 100, 36, 3, 6, 16}}, //Add for 650 LVDS Griffy
  	{TCON_LVDSMode_1280x800, DISP_TYPE_LVDS_SINGLE_666,   TCON_CLKSRC_AUTO, PROGRASSIVE,   3500, 3500,  1280,  800, BYPASS,     {32, 100, 28, 3, 6, 14}}, //Add for 650 LVDS Griffy
  	{TCON_LVDSMode_1280x1024, DISP_TYPE_LVDS_DOUBLE_888,   TCON_CLKSRC_AUTO, PROGRASSIVE,   3500, 3500,  1280,  1024, BYPASS,     {50, 70, 40, 3, 38, 1}}, //Samsung 1280x1024 LVDS 2 Channel for MP656
  	{TCON_LVDSMode_1366x768, DISP_TYPE_LVDS_SINGLE_888,   TCON_CLKSRC_AUTO, PROGRASSIVE,   5000, 5000,  1366,  768, BYPASS,  {6, 70, 24, 3, 5, 1}},    //Samsung 1366x768 LVDS
  	{TCON_LVDSMode_1366x768_CPT, DISP_TYPE_LVDS_SINGLE_666,   TCON_CLKSRC_AUTO, PROGRASSIVE,   4620, 4620,  1366,  768, BYPASS,     {4, 40, 10, 3, 5, 1}}, //CPT 1366x768 LVDS
#if (PANEL_ID == PANEL_G220SW01)
	{TCON_LVDSMode_1680x1050, DISP_TYPE_LVDS_DOUBLE_888,   TCON_CLKSRC_AUTO, PROGRASSIVE,   3500, 3500,  1680,  1050, BYPASS,     {50, 70, 40, 3, 28, 1}}, //Samsung 1680x1050 LVDS 2 Channel for MP656
#else
	{TCON_LVDSMode_1680x1050, DISP_TYPE_LVDS_DOUBLE_666,   TCON_CLKSRC_AUTO, PROGRASSIVE,   3500, 3500,  1680,  1050, BYPASS,     {50, 70, 40, 3, 28, 1}}, //Samsung 1680x1050 LVDS 2 Channel for MP656
#endif
#if MINIDV_YBD_FUNCION
{TCON_RGB24Mode_1280x720,  DISP_TYPE_RGB24,   TCON_CLKSRC_AUTO, PROGRASSIVE, 	270, 270,  1280,  720, 	BYPASS,     {32, 240, 94, 3, 10, 15}},
#else
{TCON_RGB24Mode_1280x720,  DISP_TYPE_RGB24,   TCON_CLKSRC_AUTO, PROGRASSIVE, 	744, 744,  1280,  720, 	BYPASS,     {32, 240, 94, 3, 10, 15}},
#endif
#if TCON_RGB24Mode_1280x600
{TCON_RGB24Mode_1280x600,  DISP_TYPE_RGB24,   TCON_CLKSRC_AUTO, PROGRASSIVE, 	270, 270,  1280,  600, 	BYPASS,     {32, 240, 94, 3, 10, 15}},
#endif
        {TCON_ITU709Mode_1280x720, DISP_TYPE_ITU709,   TCON_CLKSRC_AUTO, PROGRASSIVE,   743, 743,  1280,  720, BYPASS,     {68,246, 50, 6, 12, 12}}, //for Ypbpr 720p 0x04002405,{72,242, 48, 6, 12, 12}     {22,260, 80, 2, 12, 10}{88,216, 65, 6, 12, 12}
	{TCON_LVDSMode_1440x900, DISP_TYPE_LVDS_DOUBLE_888,   TCON_CLKSRC_AUTO, PROGRASSIVE,   3115, 3115,  1440,  900, BYPASS,     {20, 100, 80, 4, 13, 13}}, //CHIMEI 1440x900 LVDS 2 Channel {40, 80, 120, 10, 30, 60}
    #if 0//UP052_320_mode
    {TCON_RGB8SerialMode_320x240, DISP_TYPE_RGB8_SERIAL, TCON_CLKSRC_AUTO, PROGRASSIVE, 245, 245,  320,  240, 0x03020700, {1, 51, 9, 1, 16, 1}},//{1, 55, 8, 1, 16, 1}}, //0x03020700
    #else
    {TCON_RGB8SerialMode_320x240, DISP_TYPE_RGB8_SERIAL, TCON_CLKSRC_AUTO, PROGRASSIVE, 245, 245,  320,  240, 0x03020700, {3, 72, 120, 3, 18, 3}}, //0x03020700 
	{TCON_RGB8SerialMode_320x240_ILI9342, DISP_TYPE_RGB8_SERIAL, TCON_CLKSRC_AUTO, PROGRASSIVE, 150, 150,  320,  240, 0x03020700, {3, 72, 10, 1, 1, 10}}, //{14, 65, 10, 1, 1, 10}
	{TCON_RGB8SerialMode_480x240, DISP_TYPE_RGB8_SERIAL, TCON_CLKSRC_AUTO, PROGRASSIVE, 270, 270,  480,  240,  0x03020700, {1, 86, 9, 1, 13, 9} }, //0x03020700  {1, 83, 8, 1, 13, 9}{9, 67, 23, 1, 13, 9}
    #endif
    {TCON_RGB24Mode_1024x768, DISP_TYPE_RGB24, TCON_CLKSRC_AUTO, PROGRASSIVE, 650, 650, 1024, 768, BYPASS, {136, 160, 24, 6, 29, 3}},//{40, 100, 36, 3, 6, 16}
    {TCON_RGB24Mode_1024x600, DISP_TYPE_RGB24, TCON_CLKSRC_AUTO, PROGRASSIVE, 440, 440, 1024, 600, BYPASS, {160, 130, 30, 19, 16, 3}},
#else
    {TCON_RGB24Mode_1024x600, DISP_TYPE_DVI_666,   TCON_CLKSRC_AUTO, PROGRASSIVE,   500, 500,  1024,  600, BYPASS,     {40, 100, 180, 3, 6, 16}}, //Add for AUO A089SW01 Griffy
    {TCON_RGB24Mode_800x480,  DISP_TYPE_RGB24,   TCON_CLKSRC_AUTO, PROGRASSIVE,   240, 240,  800,  480, BYPASS,     {48, 40, 40, 3, 29, 13}},    //Add for AUO A050VN01 Griffy
    //{TCON_RGB24Mode_800x480,  DISP_TYPE_RGB24,   TCON_CLKSRC_AUTO, PROGRASSIVE,   333, 333,  800,  480, BYPASS,     {128, 88, 40, 3, 21, 1}},    //Add for AUO A070VW04 Griffy
    {TCON_RGB24Mode_800x600,  DISP_TYPE_RGB24,   TCON_CLKSRC_PLL2, PROGRASSIVE,   400, 400,  800,  600, 0x01069f1a,     {128, 88, 40, 3, 24, 1}}, //Add for AUO G121SN01 Griffy
    {TCON_RGB24Mode_400x234,  DISP_TYPE_RGB24,   TCON_CLKSRC_PLL2, PROGRASSIVE,   80, 80,  400,  234, 0x01067f1a,     {1, 88, 32, 1, 16, 19}}, //Add for AUO C043GW01 Griffy
    {TCON_RGB24Mode_640x480,  DISP_TYPE_RGB24,   TCON_CLKSRC_AUTO, PROGRASSIVE,   135, 270,  640,  480, BYPASS,     {32, 80, 48, 2, 33, 10}},
    {TCON_RGB24Mode_1024x768, DISP_TYPE_RGB24, TCON_CLKSRC_AUTO, PROGRASSIVE, 333, 666, 1024, 768, BYPASS, {136, 160, 24, 6, 29, 3}},
    {TCON_RGB24Mode_1280x800, DISP_TYPE_RGB24, TCON_CLKSRC_AUTO, PROGRASSIVE, 333, 666, 1280, 800, BYPASS, {32, 72, 24, 3, 12, 1}},
#endif
    {TV_COMPOSITE_NTSC, DISP_TYPE_COMPOSITE, TCON_CLKSRC_AUTO, INTERLACE,   135, 270,  720,  480, BYPASS,     {2, 118, 18, 3, 18, 1}},
    {TV_COMPOSITE_PAL , DISP_TYPE_COMPOSITE, TCON_CLKSRC_AUTO, INTERLACE,   135, 270,  720,  576, BYPASS,     {10, 133, 1, 3, 19, 1}},
    {TV_S_VIDEO_NTSC,   DISP_TYPE_S_VIDEO,   TCON_CLKSRC_AUTO, INTERLACE,   135, 270,  720,  480, BYPASS,     {2, 118, 18, 3, 18, 1}},
    {TV_S_VIDEO_PAL ,   DISP_TYPE_S_VIDEO,   TCON_CLKSRC_AUTO, INTERLACE,   135, 270,  720,  576, BYPASS,     {10, 133, 1, 3, 19, 1}},
    {TV_COMPONENT_PAL , DISP_TYPE_COMPONENT, TCON_CLKSRC_AUTO, INTERLACE,   135, 270,  720,  576, BYPASS,     {10, 133, 1, 3, 19, 1}},
    {TV_COMPONENT_480I, DISP_TYPE_COMPONENT, TCON_CLKSRC_AUTO, INTERLACE,   135, 270,  720,  480, BYPASS,     {2, 118, 18, 3, 18, 1}},
    {TV_COMPONENT_480P, DISP_TYPE_COMPONENT, TCON_CLKSRC_AUTO, PROGRASSIVE, 270, 270,  720,  480, BYPASS,     {40, 95, 3, 10, 30, 5}},
    {TV_COMPONENT_720P, DISP_TYPE_COMPONENT, TCON_CLKSRC_PLL2, PROGRASSIVE, 743, 743, 1280,  720, 0x01065707,     {40, 230, 100, 5, 20, 5}},
    {TV_COMPONENT_1080I,DISP_TYPE_COMPONENT, TCON_CLKSRC_PLL2, INTERLACE,   743, 743, 1920, 1080, 0x01065707, {44, 150, 86, 3, 10, 9}},
    {TV_D_SUB_480P,     DISP_TYPE_D_SUB,     TCON_CLKSRC_AUTO, PROGRASSIVE, 270, 270,  720,  480, BYPASS,     {40, 95, 3, 10, 30, 5}},
    {TV_D_SUB_SVGA,     DISP_TYPE_D_SUB,     TCON_CLKSRC_AUTO, PROGRASSIVE, 400, 400,  800,  600, BYPASS,     {128, 100, 28, 4, 23, 1}},
    {TV_D_SUB_720P,     DISP_TYPE_D_SUB,     TCON_CLKSRC_PLL2, PROGRASSIVE, 743, 743, 1280,  720, 0x01065707, {40, 230, 100, 5, 20, 5}},
    {TV_D_SUB_XGA,      DISP_TYPE_D_SUB,     TCON_CLKSRC_PLL2, PROGRASSIVE, 743, 743, 1024,  768, 0x0002811a, {136, 160, 24, 6, 29, 3}},
    {TV_D_SUB_QVGA,     DISP_TYPE_D_SUB,     TCON_CLKSRC_PLL2, PROGRASSIVE,1080,1080, 1280,  960, BYPASS,     {112, 312, 96, 3, 36, 1}},
    {TV_D_SUB_SXGA,     DISP_TYPE_D_SUB,     TCON_CLKSRC_PLL2, PROGRASSIVE,1080,1080, 1280, 1024, BYPASS,     {112, 248, 48, 3, 38, 1}},
    {TCON_NONE, 0, 0, 0, 0, 0, 0, 0, 0, {0, 0, 0, 0, 0, 0}},  //This one must be the last one
};
ST_PANEL g_mstPanel[] = {
    {PANEL_TV_4_3 ,      BYPASS, 0, 0, BYPASS, BYPASS, 4,  3},
    {PANEL_TV_16_9,      BYPASS, 0, 0, BYPASS, BYPASS, 16, 9},
    {PANEL_DI618_560009, PANEL_MODE_STRIPE, 0, 0, 320, 234, 4,  3},
    {PANEL_DI618_680005, PANEL_MODE_DIGITAL, 0, 0, 800, 600, 4,  3},
    {PANEL_A070FW03V2,   PANEL_MODE_STRIPE, 0, 0,  480, 234,16,  9},
    {PANEL_LQ104V1DG51,  PANEL_MODE_STRIPE,  0, 0, 640, 480, 4,  3},
    {PANEL_LT070W02TMC4,PANEL_MODE_STRIPE,  0, 0, 480, 234, 16, 9},
    {PANEL_AT080TN03,    PANEL_MODE_DIGITAL, 0, 0, 800, 480,16,  9},
    {PANEL_LQ080V3DG01,  PANEL_MODE_DIGITAL, 0, 0, 640, 480, 4,  3},
    {PANEL_A080SN01,     PANEL_MODE_DIGITAL, 0, 0, 800, 600, 4,  3},
    {PANEL_A060FW02,        PANEL_MODE_STRIPE, 0, 0, 520, 288, 4,  3},
    {PANEL_AT056TN04, PANEL_MODE_STRIPE, 0, 0, 320, 234, 4, 3},
    {PANEL_A089SW01, PANEL_MODE_DIGITAL, 0, 0, 1024, 600, 16, 9},   // Add for AUO A089SW01 Griffy
    {PANEL_HT14x12, PANEL_MODE_DIGITAL, 0, 0, 1024, 768, 4, 3},
    {PANEL_A121EW02, PANEL_MODE_DIGITAL, 0, 0, 1280, 800, 16, 10},
    {PANEL_FG100250DSCWA, PANEL_MODE_DIGITAL, 0, 0, 852, 480, 16, 9},
    {PANEL_C043GW01, PANEL_MODE_STRIPE, 0, 0, 400, 234, 16, 9},
    {PANEL_CLAA035QVA01,  PANEL_MODE_STRIPE,  0, 0, 320, 240, 4,  3},   //For ChiHsin   Griffy
    {PANEL_LTM190E1L01,  PANEL_MODE_STRIPE,  0, 0, 1280, 1024, 4,  3},  //Samsung 1280x1024 LVDS 2 Channel for MP656
    {PANEL_HDMI_720P,  PANEL_MODE_STRIPE,  0, 0, 1280, 720, 4,  3},  //Samsung 1280x1024 LVDS 2 Channel for MP656
    {PANEL_ADV7393,  PANEL_MODE_STRIPE,  0, 0, 1280, 720, 16,  9},  //Ypbpr ADV7930
    {PANEL_B154SW01,  PANEL_MODE_STRIPE,  0, 0, 1680, 1050, 16,  9},  //Samsung 1680x1050 LVDS 2 Channel for MP656
    {PANEL_G220SW01,  PANEL_MODE_STRIPE,  0, 0, 1680, 1050, 16,  9},  //Samsung 1680x1050 LVDS 2 Channel for MP656
    {PANEL_LTA320AP07,  PANEL_MODE_STRIPE,  0, 0, 1366, 768, 16,  9}, //Samsung 1366x768 LVDS 1 Channel for MP656
    {PANEL_CLAA101,  PANEL_MODE_STRIPE,  0, 0, 1366, 768, 16,  9}, //CLAA101 1366x768 LVDS 1 Channel for MP656
    {PANEL_M190A1_L02,  PANEL_MODE_STRIPE,  0, 0, 1440, 900, 16,  9}, //for CHIMEI LVDS 1440x900
//-----FUJI test-----//
    {PANEL_NOVA_39XX,  PANEL_MODE_STRIPE,  0, 0, 320, 240, 4,  3}, 
//-----------------//
	#if 0//UP052_320_mode
	{PANEL_A025DL02,  PANEL_MODE_STRIPE,  0, 0, 320, 240, 4,  3},
	#else
	{PANEL_A025DL02,  PANEL_MODE_STRIPE,  0, 0, 320, 240, 4,  3},
	#endif
    {PANEL_ED024VC1,  PANEL_MODE_STRIPE, 0, 0, 480, 640,3,  4},
    {PANEL_Q08009,    PANEL_MODE_DIGITAL, 0, 0, 800, 600, 4,  3},//CHIMEI Q08009-602  / teco 
    {PANEL_ILI9327,    PANEL_MODE_DIGITAL, 0, 0, 240, 400,4,  3},//240X400
    {PANEL_OTA5182A,    PANEL_MODE_STRIPE, 0, 0, 480, 240, 4,  3},
    {PANEL_ILI9342,    PANEL_MODE_STRIPE, 0, 0, 320, 240, 4,  3},
    {PANEL_ILI9163C,    PANEL_MODE_DIGITAL, 0, 0, 128*3, 128*2, 4,  3},
    {PANEL_720_480,  PANEL_MODE_DIGITAL,  0, 0, 720, 480, 16,  9}, 
    {PANEL_AT102TN03,   PANEL_MODE_DIGITAL, 0, 0, 800, 480,16,9},    
    {PANEL_AT080TN42,  PANEL_MODE_DIGITAL, 0, 0, 800, 600, 4,  3},
	{PANEL_NONE, 0, 0, 0, 0,  0, 0},  //This one must be the last one
    {PANEL_NONE, 0, 0, 0, 0,  0, 0},  //This one must be the last one
	{PANEL_NONE, 0, 0, 0, 0,  0, 0},  //This one must be the last one
};
ST_SCREEN_TABLE g_mstScreenTable[SCREEN_TABLE_TOTAL_NUM] = {
#if (SCREEN_TABLE_TOTAL_NUM == 4)
    {TV_COMPONENT_1080I, TV_COMPONENT_480I, PANEL_TV_16_9, 0,  0,  0, ISP_TAG_XP1},
    {TV_COMPOSITE_NTSC,  BYPASS,            PANEL_TV_16_9, 0,  0,  0, ISP_TAG_XP2},
    {TV_S_VIDEO_NTSC,    BYPASS,            PANEL_TV_16_9, 0,  0,  0, ISP_TAG_XP2},
    {TV_D_SUB_XGA,       TV_D_SUB_480P,     PANEL_TV_16_9, 0,  0,  0, ISP_TAG_XP3},
#elif (SCREEN_TABLE_TOTAL_NUM == 2)
    {TCON_ID,       BYPASS,            PANEL_ID,  0,  0, 0, ISP_TAG_XPG},
    {TCON_ID2,       BYPASS,            PANEL_ID2,  0,  0, 0, ISP_TAG_XPG},
#else
    {TCON_ID,       BYPASS,            PANEL_ID,  0,  0, 0, ISP_TAG_XPG},
#endif
};

char *PANEL_ID_NAME[] = {"NONE_0",                  //0
	                  "NONE",                    // 1
	                  "NONE",                    // 2
	                  "DataImage_5.6",           // 3
	                  "DataImage_8_(800x600)",   // 4
	                  "AUO_7",                   // 5
                      "LG7(26 pin)",             // 6
                      "InnoLux_8_(800x480)",     // 7
                      "Sharp_8_(800x480)",       // 8
                      "AUO_6",                   // 9
                      "Sharp_10.4_(640x480)",    // 10
                      "Innolux_5.6_(320x234)",   // 11
                      "ht14x12-101_(1024x768)",  // 12
                      "AUO_12_WXGA_(1280x800)",  // 13
                      "DataImage_10.2_(852x480)",// 14
                      "AUO_LED_8_(800x600)",     // 15
                      "AUO_8.9_(1024x600)",      // 16
                      "AUO_4.3_(400x234)",       // 17
                      "CHIMEI_3.5_(320X240)",    // 18
                      "NONE",                    // 19
                      "AUO_(480x272)",           // 20
                      "HDMI_720P",               // 21
                      "Samsung_LVDS_(1680x1050)",// 22
                      "Samsung_LVDS_(1366x768)", // 23
                      "Samsung_LVDS_(1366x768)", // 24
                      "AUO_2.5_SRGB_(320x240)",  // 25
                      "EPD_(480x640)",           // 26
                      "CHIMEI_Q08009_602_TECO",  // 27
                      "CHIMEI_LVDS _(1440x900)", // 28
                      "AUO_LVDS_(1680x1050)",    // 29
                      "Ypbpr_ADI_ADV7393",       // 30
                      "ILI9327_RGB565_(240X400)",// 31
                      "ORISE_SRGB_(480X240)",    // 32
                      "ILI9163_I80_(128X128)",   // 33
                      "ILI9342_SRGB_(320X240)",   // 34
                      "NOVA_RGB24_(320X240)",     //35
                      "720X480",     //36
                      "800X480",     //37
                      "800X600",     //38
                      "",     //39
                      "",     //40
                      "",     //41
                      "",     //42
                      "",     //43
                      "",     //44
                      "",     //45
                      "",     //46
                      "",     //47
                      "",     //48
                      "",     //49
                      "",     //50
                      "",     //51
                      "",     //52
                      "",     //53
                      "",     //54
                      "",     //55
                      "",     //56
                      "",     //57
                      "128",     //58
                      "59",	//59
                      "60",	//60
                      "PANEL_RELAX040",		//61
                      "",		//62
                      };

char *TCON_ID_NAME[] = {
	                 "TCON_NONE"        ,                  //0 This one must be '0', others are sequence irrelative
                     "TV_COMPOSITE_NTSC",                  //01
                     "TV_COMPOSITE_PAL",                   //02
                     "TV_S_VIDEO_NTSC",                    //03 
                     "TV_S_VIDEO_PAL",                     //04
                     "TV_COMPONENT_PAL",                   //05
                     "TV_COMPONENT_480I",                  //06
                     "TV_COMPONENT_480P",                  //07
                     "TV_COMPONENT_720P",                  //08
                     "TV_COMPONENT_1080I",                 //09
                     "TV_D_SUB_480P",                      //10
                     "TV_D_SUB_720P",                      //11
                     "TV_D_SUB_XGA",                       //12 1024X768
                     "TV_D_SUB_SVGA",                      //13 800X600
                     "TV_D_SUB_QVGA",                      //14 1280X960
                     "TV_D_SUB_SXGA",                      //15 1280X1024
                     "TCON_MX88V44",                       //16
                     "TCON_HX8817A",                       //17
                     "TCON_MX88V430",                      //18
                     "TCON_MX88V430_WQVGA",                //19 480x234
                     "TCON_MX88V431",                      //20
                     "TCON_MX88V431_WQVGA",                //21 480x234
                     "TCON_MX88V431_CCIR601",              //22
                     "TCON_HX8819A",                       //23
                     "TCON_LQ080V3DG01",                   //24 Sharp 8"
                     "TCON_ARK1829",                       //25
                     "TCON_INTERNAL",                      //26
                     "TCON_INTERNAL_800x600",              //27
                     "TCON_RGB24Mode_640x480",             //28
                     "TCON_RGB24Mode_800x600",             //29
                     "TCON_RGB24Mode_1024x768",            //30
                     "TCON_RGB24Mode_1280x800",            //31
                     "TCON_INTERNAL_852x480",              //32
                     "TCON_RGB24Mode_1024x600",            //33
                     "TCON_RGB24Mode_800x480",             //34
                     "TCON_RGB24Mode_400x234",             //35
                     "TCON_RGB24Mode_320x240",             //36 for ChiMei3.5" 320X240
                     "TCON_INTERNAL_800x480",  	           //37 for CPT 800X480 on 650 demo board
                     "TCON_LVDSMode_1024x600",             //38 for CPT 1024X600 on 650 demo board
                     "TCON_LVDSMode_1280x1024",            //39
                     "TCON_RGB24Mode_800x600_3D",          //40
                     "TCON_LVDSMode_1280x800",             //41
                     "TCON_A040FL01",                      //42
                     "TCON_RGB24Mode_480x272",             //43
                     "TCON_RGB24Mode_1280x720",            //44
                     "TCON_LVDSMode_1680x1050",            //45 for samsung
                     "TCON_LVDSMode_1366x768",             //46 for samsung
                     "TCON_LVDSMode_1366x768_CPT",         //47 for CPT
                     "TCON_RGB8SerialMode_320x240",        //48
                     "TCON_INTERNAL_480x640",  	           //49 for EPD 480x640
                     "TCON_LVDSMode_1440x900",             //50
                     "TCON_ITU709Mode_1280x720",           //51
                     "TCON_RGB24Mode_240x400",             //52 for ILI9327
                     "TCON_RGB8SerialMode_480x240",        //53
                     "TCON_I80Mode_128x128",               //54
                     "TCON_RGB8SerialMode_320x240_ILI9342", //55
                     "TCON_RGB24Mode_FUJI_320x240", //56
                     "TCON_BIT1201", //57
                     "58", //58
                     "59", //59
                     "60", //60
                     "TCON_RGB24Mode_480x800", //61
                     };


					 
