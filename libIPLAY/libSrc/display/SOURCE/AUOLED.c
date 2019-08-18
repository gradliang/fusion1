
#include "global612.h"
#include "mptrace.h"

//#if ((PANEL_ID == PANEL_A070VW04) || (PANEL_ID == PANEL_A080SN01))	// serapis 11/09
#if (PANEL_ID == PANEL_A080SN01)	// serapis 11/09
#define DELAYS()       __asm("nop");__asm("nop");__asm("nop");__asm("nop");

#define Set_CSB_HIGH()		g_psGpio->Gpdat1 |= 0x00000040    // GPIO 22 = High (Chip select)
#define Set_CSB_LOW()		g_psGpio->Gpdat1 &= 0xFFFFFFBF    //  GPIO 22 = Low (Chip select)
#define Set_PSDA_HIGH()		g_psGpio->Gpdat1 |= 0x00000020    // GPIO 21 = High (Serial data)
#define Set_PSDA_LOW()		g_psGpio->Gpdat1 &= 0xFFFFFFDF    // GPIO 21 = Low (Serial data)
#define Set_PSDA_Output()		g_psGpio->Gpdat1 |= 0x00200000    // GPIO 21 = output (Serial data Output)
#define Set_PSDA_Input()		g_psGpio->Gpdat1 &= 0xFFDFFFFF    // GPIO 21 = input (Serial data Input)
#define Get_PSDA()			(g_psGpio->Gpdat1 & 0x00000020)   // GPIO 21 = input (Serial data Get)
#define Set_PSCL_HIGH()		g_psGpio->Gpdat1 |= 0x00000010    // GPIO 20 = High (Serial clock)
#define Set_PSCL_LOW()		g_psGpio->Gpdat1 &= 0xFFFFFFEF    // GPIO 20 = Low (Serial clock)


//void AUOLED_Init(BYTE bInch);		//serapis 04/10
void AUOLEDWtReg(WORD wData);	//serapis 04/10
WORD AUOLEDRdReg(WORD wAddr);	//serapis 04/10

static void AUOLEDRegInit(void)
{
	//g_psGpio->Gpcfg1 &= 0xFFCBFFCB;     //GPIO 18/20/21 = GPIO
	//g_psGpio->Gpdat1 |= 0x00340034;     //GPIO 18/20/21 = Output/High

	//g_psGpio->Fgpcfg[1] &= 0xfffefffe;		//serapis 11/07 : LCD_CSB : general GPIO
	//g_psGpio->Fgpdat[1] |= 0x00010001;		//serapis 11/07 : LCD_CSB : output high

	g_psGpio->Gpcfg1 &= 0xff8fff8f;			//serapis 11/08 : GPIO 0/1 general GPIO
	g_psGpio->Gpdat1 |= 0x00700070;		//serapis 11/08 : GPIO 0/1 output/high
}

void AUOLED_Init(BYTE bInch)		//0:7inch, 1:8inch
{
        AUOLEDRegInit();
		
	if (bInch == 0)	// 7inch
		AUOLEDWtReg(0x02D3);			//R0 : U/D=0, SHL=1, GRB=1, STB=1 (D10/D9 = 01b)
		//AUOLEDWtReg(0x0293);			//R0 : U/D=0, SHL=1, GRB=1, STB=1 (D10/D9 = 01b), old version DITH(D6)=0
	else// if (bInch == 1) // 8inch
		AUOLEDWtReg(0x04D3);			//R0 : U/D=0, SHL=1, GRB=1, STB=1 (D10/D9 = 10b)
		//AUOLEDWtReg(0x04E3);			//R0 : U/D=1, SHL=0, GRB=1, STB=1 (D10/D9 = 10b) : invert vertical/horizontal
		
	AUOLEDWtReg(0x116F);			//R1 : VCOM_M=01, VCOM_LVL=2Fh
	AUOLEDWtReg(0x2080);			//R2 : HDL=80h
	AUOLEDWtReg(0x3008);			//R3 : VDL=1000b
	AUOLEDWtReg(0x419F);			//R4 : 
	AUOLEDWtReg(0x61CE);			//R6 : EnGB12=1, EnGB11=1, EnGB10=1, EnGB5=1, EnGB4=1, EnGB3=1

#if 0
	//////////////////////////////////////////serapis test : 11/01
	DELAYS();
	DELAYS();
	DELAYS();
	DELAYS();
	DELAYS();
	DELAYS();

	WORD wRegValue = 0;
	wRegValue = AUOLEDRdReg(0x0800);	//R0
	MP_DEBUG1("R0 = 0x%.4x", wRegValue);
	wRegValue = AUOLEDRdReg(0x1800);	//R1
	MP_DEBUG1("R1 = 0x%.4x", wRegValue);	
	wRegValue = AUOLEDRdReg(0x2800);	//R2
	MP_DEBUG1("R2 = 0x%.4x", wRegValue);	
	wRegValue = AUOLEDRdReg(0x3800);	//R3
	MP_DEBUG1("R3 = 0x%.4x", wRegValue);	
	wRegValue = AUOLEDRdReg(0x4800);	//R4
	MP_DEBUG1("R4 = 0x%.4x", wRegValue);	
	wRegValue = AUOLEDRdReg(0x6800);	//R6
	MP_DEBUG1("R6 = 0x%.4x", wRegValue);
	//////////////////////////////////////////serapis test : 11/01
#endif
}

WORD AUOLEDRdReg(WORD wAddr)
{
	BYTE		bBit = 0;
	WORD	wRet = 0;
	WORD	wWriteAddr=0;

	wWriteAddr = wAddr;

	Set_CSB_LOW();				//chip select low
	DELAYS();

	Set_PSDA_Output();
	for(bBit = 0; bBit < 6; bBit++)		//first, write 6bit (addr. 4bit + read bit + dummy 1bit)
	{
		Set_PSCL_LOW();			//clock low
		if(wWriteAddr & 0x8000)
                Set_PSDA_HIGH();		//data high
		else
                Set_PSDA_LOW();			//data low
            	DELAYS();
            	Set_PSCL_HIGH();			//clock high
            	DELAYS();

		wWriteAddr = wWriteAddr << 1;		// 1bit left-shift
	}
	DELAYS();

	Set_PSDA_Input();
	for(bBit = 0; bBit < 10; bBit++)	//second, read 10bit (D9 ~ D0)
	{
		Set_PSCL_LOW();			//clock low
		wRet |=Get_PSDA()>>5;
		Set_PSCL_HIGH();			//clock high

		wRet = wRet << 1;		// 1bit left-shift
	}

	Set_PSDA_Output();
	Set_PSDA_HIGH();			//data high
        Set_CSB_HIGH();				//chip select high

	wRet = wRet >> 1;		// 1bit right-shift
	
	return	wRet;
}

void AUOLEDWtReg(WORD wData)
{
	BYTE		bBit = 0;
	WORD	wWriteData=0;

	wWriteData = wData;
	
        Set_CSB_LOW();				//chip select low
	DELAYS();

	Set_PSDA_Output();
	for(bBit = 0; bBit < 16; bBit++)
	{
        	Set_PSCL_LOW();			//clock low
		if(wWriteData & 0x8000)
                	Set_PSDA_HIGH();	//data high
		else
                	Set_PSDA_LOW();		//data low
            	DELAYS();
            	Set_PSCL_HIGH();			//clock high
            	DELAYS();

		wWriteData = wWriteData << 1;		// 1bit left-shift
	}
	
        Set_PSDA_HIGH();			//data high
        Set_CSB_HIGH();				//chip select high
}
#endif


