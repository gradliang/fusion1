
#include "global612.h"
#include "slideEffect.h"
#include "Setup.h"
#include "SetupString.h"

/* actually, (String_NULL + 1) entries in this array to match the String_NULL enumeration in SetupString.h */
BYTE *Str_Multi[String_NULL+1];

BYTE *Str_English[String_NULL+1]=
{
	"English",                          // 0  
	"Yes",                              // 1  
	"No",                               // 2  
	"On",                               // 3  
	"Off",                              // 4  
	"All",                              // 5  
	"Full Screen",                      // 6  
	"Image Size",                       // 7  
	"Photo Transition",                 // 8  
	"Slideshow Effect ",                // 9  
	"Slideshow Speed ",                 // 10 
	"Slideshow Shuffle ",               // 11 
	"Slideshow Repeat ",                // 12 
	"Fade",                             // 13 
	"Shutter",                          // 14 
	"Cross Comb",                       // 15 
	"Mask",                             // 16 
	"Brick",                            // 17 
	"Dissolve",                         // 18 
	"Bar",                              // 19 
	"Expansion",                        // 20 
	"Silk",                             // 21 
	"Sketch",                           // 22 
	"Grid",                             // 23 
	"Scroll",                           // 24 
	"Ken Burns",                        // 25 
	"Bullentin Board",                  // 26 
	"3D Cube",                          // 27 
	"3D Flip",                          // 28 
	"3D Swap",                          // 29 
	"Push",                             // 30 
	"Multi Effect",                     // 31 
	"Random",                           // 32 
	"Fast",                             // 33 
	"Normal",                           // 34 
	"Slow",                             // 35 
	"Video Repeat",                     // 36 
	"Video Settings",                   // 37 
	"Music Repeat",                     // 38 
	"English",               			// 39 
	"Copy File",                        // 40 
	"Delete File",                      // 41 
	"Rotate",                           // 42 
	"Zoom",                             // 43 
	"Start Slide Show",                 // 44 
	"Reset Settings",                   // 45 
	"System Settings",                  // 46 
	"Save Setup",                       // 47 
	"PC/Printer/SideMonitor",           // 48 
	"Connect to PC",                    // 49 
	"Connect to Printer",               // 50 
	"Display / Chroma",                 // 51 
	"Brightness / Contrast",            // 52 
	"Saturation / Hue",                 // 53 
	"Color Management",                 // 54 
	"Output Select",                    // 55 
	"Refresh",                          // 56 
	"Show Device Base",                 // 57 
	"Delete Device",                    // 58 
	"Connect",                          // 59 
	"Service Search",                   // 60 
	"Match Device",                     // 61 
	"A2DP Connect",                     // 62 
	"A2DP Disonnect",                   // 63 
	"A2DP Config",                      // 64 
	"Hint",                             // 65 
	"OPush Hint",                       // 66 
	"OPush Disconnect",                 // 67 
	"Transfer File",                    // 68 
	"Format Nand",                      // 69 
	"Bluetooth",                        // 70 
	"FTPC Hint",                        // 71 
	"FTPS Hint",                        // 72 
	"A2DP Hint",                        // 73 
	"FTPC Disconnect",                  // 74 
	"FTPS Disconnect",                  // 75 
	"A2DP Disconnect",                  // 76 
	"FTP Pull Folder",                  // 77 
	"FTP Set Folder Forward",           // 78 
	"FTP Set Folder Backup",            // 79 
	"FTP Set Folder Root",              // 80 
	"FTP Create Folder",                // 81 
	"FTP Pull File",                    // 82 
	"FTP Push",                         // 83 
	"FTP delete",                       // 84 
	"Set Favorite",                     // 85 
	"Separate Windows",                 // 86 
	"Video Forward Second",             // 87 
	"Are you sure",                     // 88 
	"Rotate Full",                      // 89 
	"Default",                          // 90 
	"One",                              // 91 
	"Optimal 60%",                      // 92 
	"Optimal 70%",                      // 93 
	"Optimal 80%",                      // 94 
	"Optimal 90%",                      // 95 
	"Sharp",                            // 96 
	"Black White",                      // 97 
	"SEBIAN",                           // 98 
	"Color Mix",                        // 99 
	"Blur",                             // 100
	"Disable",                          // 101
	"Opush transfer init",              // 102
	"FTP-C Connect",                    // 103
	"FTP-S Connect",                    // 104
	"Red",                              // 105
	"Skin",                             // 106
	"Yellow",                           // 107
	"Green",                            // 108
	"Cyan",                             // 109
	"Blue",                             // 110
	"Magenta",                          // 111
	"System Info",                      // 112
	"File Sort By Name(Ascend)",        // 113
	"File Sort By Name(Descend)",       // 114
	"File Sort By Date(Ascend)",        // 115
	"File Sort By Date(Descend)",       // 116
	"File Sort By Size(Ascend)",        // 117
	"File Sort By Size(Descend)",       // 118
	"Original",                         // 119
	"Shuffle",                          // 120
	"Special Effect",                   // 121
	"Print Photo",                      // 122
	"Dynamic Select",                   // 123
	"Search Device",                    // 124
	"AV Service",                       // 125
	"Setting",     						// 126
	"TV - NTSC",                        // 127
	"TV - PAL",                         // 128
	"HDTV - 1080i",                     // 129
	"HDTV - 720p",                      // 130
	"S-Video",                          // 131
	"D-Sub",                            // 132
	"Panel",                            // 133
	"2",                                // 134
	"4",                                // 135
	"8",                                // 136
	"16",                               // 137
	"32",                               // 138
	"Done ",                            // 139
	"Target Invalid!",                  // 140
	"Fail!",                            // 141
	"Disk Full!",                       // 142
	"File Not Found!",                  // 143
	"Target Read Only!",                // 144
	"Cancel!",                          // 145
	"RedEyesRemove",                    // 146
	"FaceDetection",                    // 147
	//PRODUCT_MODEL,                      // 148
	//PRODUCT_FW_VERSION,                 // 149
	"MyFavor",                          // 150
	"MyFavor1 AddFile",                 // 151
	"MyFavor2 AddFile",                 // 152
	"MyFavor3 AddFile",                 // 153
	"MyFavor4 AddFile",                 // 154
	"MyFavor1 Reset",                   // 155
	"MyFavor2 Reset",                   // 156
	"MyFavor3 Reset",                   // 157
	"MyFavor4 Reset",                   // 158
	"MyFavor DeleteFile",               // 159
	"Connect to SideMonitor",           // 160
	"DynamicLighting",                  // 161
	"Music Lyric",                      // 162
	"GBK",                              // 163
	"BIG5",                             // 164
	"UNICODE",                          // 165
	"File Sort By EXIF(Ascend)",        // 166
	"File Sort By EXIF(Descend)",       // 167
	"",
};

BYTE * Get_MLString(WORD wIndex)
{
	int size = sizeof(Str_Multi) / sizeof(Str_Multi[0]); 
	
	if (wIndex >= size)
		return NULL; 
	
	return Str_Multi[wIndex]; 
}



BYTE *Str_NullStrTab[2];
BYTE *Str_FullScreenTab[2];
BYTE *Str_PhotoTransitionTab[2];
BYTE *Str_TransitionEffectTab[SETUP_MENU_TRANSITION_RANDOM+1];
BYTE *Str_SlideIntervalTab[3];
BYTE *Str_SlideRptTab[3];
BYTE *Str_SlideShuffleTab[2];
BYTE *Str_VideoSettingsTab[3]; //Str_VideoSettingsTab[2];
BYTE *Str_VideoRepeatTab[3] ;
BYTE *Str_VideoForwardTab[5];
BYTE *Str_AVRepeatTab[3];
BYTE *Str_MusicLyricTab[3];
BYTE *Str_FileConfirm[3];
BYTE *Str_SystemSettingTab[10];
BYTE *Str_SpecialEffectTab[8];
BYTE *Str_DisplayChromaTab[2];
BYTE *Str_ColorManagementTab[7];
BYTE *Str_CropMode[5];
#if (BT_XPG_UI == ENABLE)
        BYTE *Str_BlueToothTab[2];
#endif

BYTE *Str_USBDModeTab[3];
BYTE *Str_ChangeResTab[4];
BYTE *Str_MyFavorTab[6];

/* note: total (MAX_DRIVE_NUM - 1) items in XXX_DeviceArryTab[], pls refer to and meet the drive ID list order in drive.h */
BYTE *Str_DeviceArryTab[MAX_DRIVE_NUM - 1] = {
	"USB_HOST_ID1",
	"USB_HOST_ID2",
	"USB_HOST_ID3",
	"USB_HOST_ID4",
	"USB_HOST_PTP",
	"USBOTG1_HOST_ID1",
	"USBOTG1_HOST_ID2",
	"USBOTG1_HOST_ID3",
	"USBOTG1_HOST_ID4",
	"USBOTG1_HOST_PTP",
	"NAND_ISP",
	"NAND_PART1",
	"NAND_PART2",
	"NAND_PART3",
	"NAND_PART4",
	"SM",
	"XD",
	"MS",
	"SD_MMC_PART1",
	"SD_MMC_PART2",
	"SD_MMC_PART3",
	"SD2",
	"CF",
	"HD",
	"HD2",
	"HD3",
	"HD4",
	"SPI_FLASH_PART1",
	"SPI_FLASH_PART2",
	"SPI_FLASH_ISP",
	"SDIO",
	"USB_WIFI_DEVICE",
	"USB_PPP",
	"CF_ETHERNET_DEVICE"
};



//=============================================================================================
BYTE *Str_MainSetupStrTab[MAIN_ARRY_SIZE];
BYTE *Str_MainUSBDSetupStrTab[MAIN_USBD_ARRY_SIZE];

//=============================================================================================
BYTE **Str_MainSetupArry[MAIN_ARRY_SIZE]={
	Str_SlideShuffleTab,
#if (BT_XPG_UI == ENABLE)
	Str_BlueToothTab,
#else
	Str_NullStrTab,
#endif
	Str_FullScreenTab,
	Str_PhotoTransitionTab,
	Str_TransitionEffectTab,
	Str_SlideIntervalTab,
	Str_SlideShuffleTab,
	Str_VideoSettingsTab, // 7 - Str_AVRepeatTab,
#if SC_USBDEVICE	
	Str_USBDModeTab, // 8 - USBD mode
#else
    Str_NullStrTab,
#endif     
	Str_NullStrTab,
	Str_NullStrTab,
	Str_DeviceArryTab,
	Str_FileConfirm,
	Str_NullStrTab,
	Str_NullStrTab,
	Str_NullStrTab,
	Str_SystemSettingTab, 
	Str_NullStrTab,        // 17 - "Print Photo"
	Str_SpecialEffectTab,   // 18 - Str_FileConfirm,
	Str_DisplayChromaTab,  // 19 - Str_NullStrTab,
	Str_ColorManagementTab, // 20 - Str_NullStrTab,
	Str_MyFavorTab, // 21
};

BYTE *Str_FileStrArry[22];



BYTE LANGUAGE_ENGLISH_STRING[]="English";
BYTE LANGUAGE_GERMANY_STRING[]="Germany";
BYTE LANGUAGE_SPAIN_STRING[]="Spain";
BYTE LANGUAGE_FRENCH_STRING[]="French";
BYTE LANGUAGE_ITALY_STRING[]="Italy";
BYTE LANGUAGE_DUTCH_STRING[]="Dutch";
BYTE LANGUAGE_POLISH_STRING[]="Polish";
BYTE LANGUAGE_PORTUGAL_STRING[]="Portugal";
BYTE LANGUAGE_RUSSIAN_STRING[]="Russian";
BYTE LANGUAGE_SWEDISH_STRING[]="Swedish";
BYTE LANGUAGE_TURKISH_STRING[]="Turkish";
BYTE LANGUAGE_S_CHINESE_STRING[]="SChinese";
BYTE LANGUAGE_T_CHINESE_STRING[]="TChinese";
BYTE LANGUAGE_JAPAN_STRING[]="Japan";
BYTE LANGUAGE_KOREA_STRING[]="Korea";

ST_LANGUAGE_TABLE LanguageTable[LANGUAGE_TOTAL_NUM]=
{
	{LANGUAGE_S_CHINESE_STRING , LANGUAGE_S_CHINESE, 0},
#ifdef LANGUAGE_T_CHINESE
	{LANGUAGE_T_CHINESE_STRING , LANGUAGE_T_CHINESE, 0},
#endif
	{LANGUAGE_ENGLISH_STRING	, LANGUAGE_ENGLISH	, 0},
};


