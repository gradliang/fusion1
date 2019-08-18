


#include "global612.h"

#if (TCON_ID == TCON_MX88V44)	//Mason 2006/4/17
void MX88V44WtRegFromTable(BYTE DevID, BYTE * SettingTable, BYTE TVRegFlag)	//TvRegFlag =0:Normal =1:TV
{
	BYTE TablePointer = 0, RegNumber = 0, RegValue = 0;

	RegNumber = *(SettingTable + TablePointer);
	RegValue = *(SettingTable + TablePointer + 1);
	while (!((RegNumber == 0xff) && (RegValue == 0xff)))
	{
		if (TVRegFlag)
			I2CM_MX88V44_WtTVReg(DevID, RegNumber, RegValue);
		else
			I2CM_WtReg1(DevID, RegNumber, RegValue);
		TablePointer += 2;
		RegNumber = *(SettingTable + TablePointer);
		RegValue = *(SettingTable + TablePointer + 1);
	}
}
BYTE MX88V44_Table_TVInit[] = {
	0x3f, 0x00, 0x03, 0x00, 0x05, 0x64, 0x06, 0x8a, 0x07, 0x20, 0x11, 0xb9, 0x83, 0x89, 0xff, 0xff
};
BYTE MX88V44_Table_AU7[] = {
	0x00, 0xc0, 0x01, 0x3f, 0x03, 0x0d, 0x04, 0x0c, 0x06, 0xd8, 0x30, 0x48, 0x31, 0x12, 0x32, 0x00,
	0x33, 0x38,
	0x34, 0x00, 0x35, 0x30, 0x36, 0x01, 0x3b, 0x0d, 0x3d, 0x01, 0x3f, 0x0c, 0x41, 0x02, 0x43, 0x0a,
	0x45, 0xca, 0x46, 0x00,
	0x47, 0xd0, 0x48, 0x00, 0x49, 0x12, 0x4a, 0x00, 0x4b, 0x01, 0x4c, 0x00, 0x4d, 0x12, 0x4e, 0x00,
	0x53, 0x15, 0x55, 0x14,
	0x57, 0x15, 0x59, 0x14, 0x68, 0x10, 0x71, 0x09, 0x74, 0x6e, 0x75, 0x00, 0x76, 0x8a, 0x77, 0x00,
	0x78, 0x01, 0x79, 0x00,
	0x7a, 0xa3, 0x7b, 0x00, 0x7c, 0x00, 0x7d, 0x84, 0x7f, 0x80, 0xff, 0xff
};
BYTE MX88V44_Table_AU7_Contrast[] = {
	0x8a, 0x56, 0x0a, 0x00, 0x0b, 0x10, 0x11, 0xc0, 0x12, 0x70, 0x13, 0x70, 0x14, 0x70, 0x15, 0x78,
	0x16, 0x78, 0x17, 0x78,
	0xff, 0xff
};
BYTE MX88V44_Table_AU56_Contrast[] = {
	0x8a, 0x39, 0x0a, 0xb0, 0x0b, 0x1e, 0x11, 0xc0, 0x12, 0x80, 0x13, 0x80, 0x14, 0x80, 0x15, 0x70,
	0x16, 0x78, 0x17, 0x78,
	0xff, 0xff
};
BYTE MX88V44_Table_TVNTSC[] = {
	0x00, 0x00, 0x01, 0x01, 0x02, 0x6f, 0x0c, 0x8a, 0x18, 0x21, 0x19, 0xf0, 0x1a, 0x7c, 0x1b, 0x1f,
	0x2e, 0x82, 0x30, 0x22,
	0x31, 0x61, 0x39, 0x0a, 0x82, 0x42, 0x87, 0x10, 0xff, 0xff
};
void MX88V44Init(void)
{
	register GPIO *sGpio;
    sGpio = (GPIO *)(GPIO_BASE);
    ST_PANEL *pstPanel = g_psSystemConfig->sScreenSetting.pstPanel;
		
	sGpio->Gpdat0 &= 0xffffffef;
	IODelay(100);
	sGpio->Gpdat0 |= 0x00000010;
	IODelay(100);

    MX88V44WtRegFromTable(0x28, (BYTE *) (&MX88V44_Table_TVInit), 1);
    MX88V44WtRegFromTable(0x28, (BYTE *) (&MX88V44_Table_AU7), 0);

	if (pstPanel->wWidthInPix == 480 && pstPanel->wHeightInPix == 234)		
        MX88V44WtRegFromTable(0x28,&MX88V44_Table_AU7_Contrast,0);
	else if (pstPanel->wWidthInPix == 320 && pstPanel->wHeightInPix == 234)		
        MX88V44WtRegFromTable(0x28,&MX88V44_Table_AU56_Contrast,0);

	MX88V44WtRegFromTable(0x28, (BYTE *) (&MX88V44_Table_TVNTSC), 1);
}

BYTE *String;
BYTE CurRegFlag = 0, RegValue, BitFlag = 7, SetupLayer = 0xff;
    #define RegNum 11
    #define REG_POS_X	100
    #define REG_POS_Y	100
    #define VAL_POS_X	100
    #define VAL_POS_Y	150	

BYTE *Trans (BYTE Value)
{
	
    if (Value / 16 >= 10)
        *(String + 0) = (Value / 16) - 10 + 'a';
    else
        *(String + 0) = Value / 16 + '0';
    if (Value % 16 >= 10)
        *(String + 1) = (Value % 16) - 10 + 'a';
    else
        *(String + 1) = Value % 16 + '0';
    *(String + 2) = 0;
    return String;
}

void MX88V44_ChangeRatio(BYTE Flag)
{
	if(Flag)	// 4:3
	{
		I2CM_WtReg1(0x28,0x04,0x2a);
		I2CM_WtReg1(0x28,0x08,0x94);
		I2CM_WtReg1(0x28,0x0a,0xb3);
		I2CM_WtReg1(0x28,0x0b,0x16);
		I2CM_WtReg1(0x28,0x4d,0x6c);
		I2CM_WtReg1(0x28,0x53,0x18);
		I2CM_WtReg1(0x28,0x55,0x17);
		I2CM_WtReg1(0x28,0x57,0x18);
		I2CM_WtReg1(0x28,0x59,0x17);
		I2CM_WtReg1(0x28,0x69,0x1c);
		I2CM_WtReg1(0x28,0x6a,0x01);
		I2CM_WtReg1(0x28,0x70,0x06);
		I2CM_WtReg1(0x28,0x8a,0x80);
		I2CM_WtReg1(0x28,0x8b,0x3f);
		I2CM_WtReg1(0x28,0x45,0x8b);
		I2CM_WtReg1(0x28,0x47,0xc2);
		I2CM_WtReg1(0x28,0x68,0x1a);	
	}
	else		// 16:9
	{
		I2CM_WtReg1(0x28,0x04,0x0c);
		I2CM_WtReg1(0x28,0x08,0x00);
		I2CM_WtReg1(0x28,0x0a,0x00);
		I2CM_WtReg1(0x28,0x0b,0x10);
		I2CM_WtReg1(0x28,0x4d,0x12);
		I2CM_WtReg1(0x28,0x53,0x15);
		I2CM_WtReg1(0x28,0x55,0x14);
		I2CM_WtReg1(0x28,0x57,0x15);
		I2CM_WtReg1(0x28,0x59,0x14);
		I2CM_WtReg1(0x28,0x69,0x20);
		I2CM_WtReg1(0x28,0x6a,0x00);
		I2CM_WtReg1(0x28,0x70,0x00);
#ifdef WINBOARD7
		I2CM_WtReg1(0x28,0x8a ,0x56 );
#endif
#ifdef WINBOARD56
		I2CM_WtReg1(0x28,0x8a ,0x39 );
#endif
		I2CM_WtReg1(0x28,0x8b,0x09);
		I2CM_WtReg1(0x28,0x45,0xca);
		I2CM_WtReg1(0x28,0x47,0xd0);
		I2CM_WtReg1(0x28,0x68,0x10);
	}
}

#endif


