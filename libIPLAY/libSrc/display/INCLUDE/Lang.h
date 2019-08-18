#define	English				0
#define	French				1
#define	German				2
#define	Italian 			3
#define	Japanese			4
#define	Korean				5
#define	Portuguese			6
#define	Russian				7
#define	Spanish				8
#define	SChinese			9
#define	TChinese			10


#include "UtilTypedef.h"

#define Msg_Ok						1
#define Msg_Cancel					2 				
#define Msg_No						3					
#define Msg_Yes						4				
#define Msg_Card_Backup				5			
#define Msg_Empty					6				
#define Msg_Language_Selection		7	
#define Msg_ENGLISH					8				
#define Msg_FRENCH					9
#define Msg_GERMAN					10
#define Msg_ITALIAN					11			
#define Msg_JAPANESE				12
#define Msg_KOREAN					13			
#define Msg_PORTUGESE				14		
#define Msg_RUSSIAN					15			
#define Msg_SPANISH					16				
#define Msg_SIMCHINESE				17		
#define Msg_TRACHINESE				18			
#define Msg_Firmware_Update		19		
#define Msg_Drive_Quick_Format 		20	
#define Msg_Zoom					21			
#define Msg_Zoom_In					22	
#define Msg_Zoom_Out 				23			
#define Msg_Exit						24				
#define Msg_Not_Enough_Space		25	
#define Msg_No_Medium_Exist		26		
#define Msg_Format_Not_Support		27	


BYTE bCurrentLanguage;

typedef struct
{
	DWORD ID;
	BYTE     *Text;
} ST_LANG_TABLE;


ST_LANG_TABLE * Lang_GetCurrentTable();
BYTE * Lang_GetText(ST_LANG_TABLE *, DWORD);
void Lang_Init();
void Lang_SetGap(BYTE);
void Lang_SetImgColor(BYTE);
void Lang_SetOsdColor(BYTE);
void Lang_SetbFontLanguage(BYTE);
BYTE Lang_GetGap();
BYTE Lang_GetImgColor();
BYTE Lang_GetOsdColor();
BYTE Lang_GetFontLanguage();
WORD * Lang_GetFstPixel(WORD);
DWORD Lang_GetLocation(WORD);


