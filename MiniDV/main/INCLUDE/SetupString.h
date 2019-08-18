
/*
 You need to add 

#include "slideEffect.h"
#include "Setup.h"

When you include this SetupString.h
*/


// 
// jeffery 20100514
// -- move to "ML_StringDefine.h" 
//
#include "ML_StringDefine.h"


extern BYTE *Str_NullStrTab[2];
extern BYTE *Str_FullScreenTab[2];
extern BYTE *Str_PhotoTransitionTab[2];
extern BYTE *Str_TransitionEffectTab[SETUP_MENU_TRANSITION_RANDOM+1];
extern BYTE *Str_SlideIntervalTab[3];
extern BYTE *Str_SlideRptTab[3];
extern BYTE *Str_SlideShuffleTab[2];
extern BYTE *Str_VideoSettingsTab[3];  //Str_VideoSettingsTab[2];
extern BYTE *Str_VideoRepeatTab[3] ;
extern BYTE *Str_VideoForwardTab[5];
extern BYTE *Str_AVRepeatTab[3];
extern BYTE *Str_MusicLyricTab[3];
extern BYTE *Str_FileConfirm[3];
extern BYTE *Str_SystemSettingTab[10];
extern BYTE *Str_SpecialEffectTab[8];
extern BYTE *Str_DisplayChromaTab[2];
extern BYTE *Str_ColorManagementTab[7];
extern BYTE *Str_CropMode[5];
#if (BT_XPG_UI == ENABLE)
extern BYTE *Str_BlueToothTab[2];
#endif

extern BYTE *Str_USBDModeTab[3];
extern BYTE *Str_ChangeResTab[4];
extern BYTE *Str_MyFavorTab[6];
/* note: total (MAX_DRIVE_NUM - 1) items in XXX_DeviceArryTab[], pls refer to and meet the drive ID list order in drive.h */
extern BYTE *Str_DeviceArryTab[MAX_DRIVE_NUM - 1];
extern BYTE *Str_MainSetupStrTab[MAIN_ARRY_SIZE];

//=============================================================================================
extern BYTE **Str_MainSetupArry[MAIN_ARRY_SIZE];

extern BYTE *Str_FileStrArry[22];


typedef struct{
	BYTE *Language_String;
	WORD Language_Index;
	WORD Table_Index;
}ST_LANGUAGE_TABLE;


