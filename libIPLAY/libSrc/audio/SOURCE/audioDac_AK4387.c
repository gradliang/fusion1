/**
* FileName     AudioDac_AK4387.c
*
* Auther       abel yeh
* Date         2004.11.04
*
* Description  AKM 4387 dec driver
*
*/
#define LOCAL_DEBUG_ENABLE 0

/*// Include section
*/#include "global612.h"#include "mpTrace.h"#include "audio_hal.h"

#if (AUDIO_DAC == DAC_AK4387)

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "audio_hal.h"

/***************************************************************
*          SET GPIO 12, 13, 14,15
*                 12 : CLK
*                 13 : CDTI
*                 14 : CS
*                 15 : RST
****************************************************************/
#if (AK4387_on_PQFP)
#define Set_MP615_RST_HIGH()		g_psGpio->Gpdat0  |= 0x80008000;//set pin15 up
#define Set_MP615_RST_LOW()			g_psGpio->Gpdat0 &= 0xffff7fff;//set pin15 down
#define Set_MP615_CS_HIGH()			g_psGpio->Gpdat0  |= 0x40004000;//set pin14 up
#define Set_MP615_CS_LOW()			g_psGpio->Gpdat0 &= 0xffffbfff;//set pin14 down
#define Set_MP615_CLK_HIGH()		g_psGpio->Gpdat0  |= 0x10001000;//set pin12 up
#define Set_MP615_CLK_LOW()			g_psGpio->Gpdat0 &= 0xffffefff;//set pin12 down
#define Set_MP615_CDTI_HIGH()		g_psGpio->Gpdat0  |= 0x20002000;//set pin13 up
#define Set_MP615_CDTI_LOW()		g_psGpio->Gpdat0 &= 0xffffdfff;//set pin13 down

#define Set_MP615_CDTI_Output()		g_psGpio->Gpdat0 |= 0x20000000		// GPIO 13 output
#define Set_MP615_CDTI_Input()		g_psGpio->Gpdat0 &= 0xDFFFFFFF		// GPIO 13 input

#elif (AK4387_on_BGA)
#if (STD_BOARD_VER == IPLAYB5_E_HD_0641MD03)
//Only for AUO  Griffy ++
#if AUO_USE
#define Set_MP615_RST_HIGH()		g_psGpio->Gpdat1  |= 0x40000000;//set pin30 up
#define Set_MP615_RST_LOW()			g_psGpio->Gpdat1 &= 0xbfffffff;//set pin30 down
#define Set_MP615_CS_HIGH()			g_psGpio->Gpdat1  |= 0x00000002;//set pin17 up
#define Set_MP615_CS_LOW()			g_psGpio->Gpdat1 &= 0xfffffffd;//set pin17 down
#define Set_MP615_CLK_HIGH()		g_psGpio->Gpdat1  |= 0x00000004;//set pin18 up
#define Set_MP615_CLK_LOW()			g_psGpio->Gpdat1 &= 0xfffffffb;//set pin18 down
#define Set_MP615_CDTI_HIGH()		g_psGpio->Gpdat1  |= 0x00000008;//set pin19 up
#define Set_MP615_CDTI_LOW()		g_psGpio->Gpdat1 &= 0xfffffff7;//set pin19 down
#define Set_MP615_CDTI_Output()		g_psGpio->Gpdat1 |= 0x00080000		// FGPIO 19 output
#define Set_MP615_CDTI_Input()		g_psGpio->Gpdat1 &= 0xFFF7FFFF		// FGPIO 19 input
//Only for AUO  Griffy --
#else
#define Set_MP615_RST_HIGH()			g_psGpio->Gpdat1  |= 0x40004000;
#define Set_MP615_RST_LOW()				g_psGpio->Gpdat1  &= 0xffffbfff;
#define Set_MP615_CS_HIGH()		        g_psGpio->Ugpdat  |= 0x00020002;
#define Set_MP615_CS_LOW()		        g_psGpio->Ugpdat  &= 0xffffff0d;
#define Set_MP615_CLK_HIGH()		  	g_psGpio->Gpdat0  |= 0x00400040;
#define Set_MP615_CLK_LOW()		  		g_psGpio->Gpdat0  &= 0xffffffbf;
#define Set_MP615_CDTI_HIGH()		  	g_psGpio->Gpdat0  |= 0x00800080;
#define Set_MP615_CDTI_LOW()		  	g_psGpio->Gpdat0  &= 0xffffff7f;
#define Set_MP615_CDTI_Output()		  	g_psGpio->Gpdat0  |= 0x00800000
#define Set_MP615_CDTI_Input()		  	g_psGpio->Gpdat0  &= 0xFF7FFFFF
#endif  //#if AUO_USE

#else
#define Set_MP615_RST_HIGH()		g_psGpio->Gpdat1  |= 0x00000040;//set pin22 up
#define Set_MP615_RST_LOW()			g_psGpio->Gpdat1 &= 0xffffffbf;//set pin22 down
#define Set_MP615_CS_HIGH()			g_psGpio->Fgpdat[3]  |= 0x00000001;//set pin48 up
#define Set_MP615_CS_LOW()			g_psGpio->Fgpdat[3] &= 0xfffffffe;//set pin48 down
#define Set_MP615_CLK_HIGH()		g_psGpio->Fgpdat[3]  |= 0x00000002;//set pin49 up
#define Set_MP615_CLK_LOW()			g_psGpio->Fgpdat[3] &= 0xfffffffd;//set pin49 down
#define Set_MP615_CDTI_HIGH()		g_psGpio->Fgpdat[3]  |= 0x00000004;//set pin50 up
#define Set_MP615_CDTI_LOW()		g_psGpio->Fgpdat[3] &= 0xfffffffb;//set pin50 down

#define Set_MP615_CDTI_Output()		g_psGpio->Fgpdat[3] |= 0x00040000		// FGPIO 50 output
#define Set_MP615_CDTI_Input()		g_psGpio->Fgpdat[3] &= 0xFFFBFFFF		// FGPIO 50 input
#endif
#endif

#define MP615_DELAY()				__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");

#define read_MP615_CDTI(buffer)		buffer = g_psGpio->Gpdat0		// GPIO13 read input data

void AK4387GPIO_Init(void);
BYTE AK4387SetAddr(WORD regaddr);

/**
 ***************************************************************
 *
 * Config AK4387 clock and gpio
 *
 *  Input  : none.
 *
 *  Output : none.
 ***************************************************************
*/
void AIUCfg_AK4387(void)
{

	g_psClock->ClkCtrl |= 0x01000000;	// enable PLL3

	if(g_psBiu->BiuChid == (CHIP_VER_620 | CHIP_VER_B))
	 	g_psClock->PLL3Cfg = (PLL3_CONFIG_FS_22K_DIV_1 | 0x00080000);//by samplerate
	else
		g_psClock->PLL3Cfg = PLL3_CONFIG_FS_22K_DIV_1;//by samplerate

	g_psClock->AudClkC = 0x00001310;	//for PLL3  /2 =MCLK/4=ACLK
	g_psClock->Clkss_EXT1 |= (BIT3);    // Using PLL3/1 to be clock source
	//g_psClock->Clkss_EXT1 = 0x00000008;	//for PLL3/1 and enable

	// Bit_Clock from AK4387 Codec
	//g_psClock->MdClken &= 0xFFFeFFFF;	//CKE_AUDB disable,  c cke_ac97 disable
	//g_psClock->MdClken |= 0x00018000;	// 2 CKE_AC97 enable,1: CKE_AUDB enable  CKE_AUDM enable
	AIUCLOCK_DISABLE();
	AIU_MAINCLOCK_ENABLE();

	// Disable AIU_DMA
	g_psDmaAiu->Control = 0x00000000;

	// Configure AGPIO to AUDIO Mode
	g_psGpio->Agpcfg = 0x0000001F;
	//MP612 AIU GPIO configuration

	AK4387GPIO_Init();
	Set_MP615_RST_HIGH();//power on mode

}//end of AIUCfg_AK4387


void AK4387GPIO_Init()
{
#if (AK4387_on_PQFP) // PQFP
	g_psGpio->Gpcfg0 &= 0x0fff0fff;	//set gp12-gp15 as default function
//	g_psGpio->Gpcfg1 &= 0x0fff;//??
	g_psGpio->Gpdat0  |= 0xf0000000;	//Output DIRECTION_IN

#elif (AK4387_on_BGA)
#if (STD_BOARD_VER == IPLAYB5_E_HD_0641MD03)
	g_psGpio->Gpcfg1  &= 0xbfffbfff;		//set gp30 as default function
	g_psGpio->Gpdat1  |= 0x40000000;		//Output
//Only for AUO  Griffy ++
#if AUO_USE
	g_psGpio->Gpcfg1  &= 0xfff1fff1;		//set gp17~19 as default function
	g_psGpio->Gpdat1  |= 0x800e0000;		//Output
#else
//Only for AUO  Griffy --
	g_psGpio->Gpcfg0  &= 0xff1fff1f;		//set gp5~7 as default function
	g_psGpio->Gpdat0  |= 0x00e00000;		//Output
#endif

	g_psGpio->Ugpcfg  &= 0xfffdfffd;		//Ugp1 as default function
	g_psGpio->Ugpdat  |= 0x00020000;		//Output
#else
	g_psGpio->Gpcfg1	&= 0xffbfffbf;		//set gp22 as default function
	g_psGpio->Gpdat1	|= 0x00400000;		//Output DIRECTION_IN
	g_psGpio->Fgpcfg[3]	&= 0xfff8fff8;		//set gp48-gp50 as default function
	g_psGpio->Fgpdat[3]	|= 0x00070000;		//Output DIRECTION_IN
#endif
#endif
}


void MP615_SendStart(void)
{
	Set_MP615_CDTI_Output();
	MP615_DELAY();
	Set_MP615_CDTI_HIGH();
	Set_MP615_CLK_HIGH();
	Set_MP615_CS_HIGH();
	MP615_DELAY();
	Set_MP615_CS_LOW();
	MP615_DELAY();
	Set_MP615_CLK_LOW();
	MP615_DELAY();
	Set_MP615_CDTI_LOW();
}

void MP615_SendStop(void)
{
	Set_MP615_CLK_HIGH();
	MP615_DELAY();
	Set_MP615_CS_HIGH();
}

BYTE AK4387RegWrite(WORD regaddr, WORD data)
{
	WORD temp;

	if (regaddr < 0x05){	 //0x05 is the invalid address
		MP615_SendStart();
		Set_MP615_CDTI_Output();//always output data

		Set_MP615_CDTI_LOW();//C1 always 0
		MP615_DELAY();
		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		Set_MP615_CLK_LOW();

		Set_MP615_CDTI_HIGH();//C0 always 1
		MP615_DELAY();
		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		Set_MP615_CLK_LOW();

		Set_MP615_CDTI_HIGH();//Write  Mode
		MP615_DELAY();
		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		Set_MP615_CLK_LOW();

		AK4387SetAddr(regaddr);

		if(data & 0x80)
			Set_MP615_CDTI_HIGH()//D7
		else
			Set_MP615_CDTI_LOW();//D7

		MP615_DELAY();
		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		Set_MP615_CLK_LOW();

		if(data & 0x40)
			Set_MP615_CDTI_HIGH()//D6
		else
			Set_MP615_CDTI_LOW();//D6

		MP615_DELAY();
		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		Set_MP615_CLK_LOW();

		if(data & 0x20)
			Set_MP615_CDTI_HIGH()//D5
		else
			Set_MP615_CDTI_LOW();//D5

		MP615_DELAY();
		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		Set_MP615_CLK_LOW();

		if(data & 0x10)
			Set_MP615_CDTI_HIGH()//D4
		else
			Set_MP615_CDTI_LOW();//D4

		MP615_DELAY();
		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		Set_MP615_CLK_LOW();

		if(data & 0x08)
			Set_MP615_CDTI_HIGH()//D3
		else
			Set_MP615_CDTI_LOW();//D3

		MP615_DELAY();
		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		Set_MP615_CLK_LOW();

		if(data & 0x04)
			Set_MP615_CDTI_HIGH()//D2
		else
			Set_MP615_CDTI_LOW();//D2

		MP615_DELAY();
		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		Set_MP615_CLK_LOW();

		if(data & 0x02)
			Set_MP615_CDTI_HIGH()//D1
		else
			Set_MP615_CDTI_LOW();//D1

		MP615_DELAY();
		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		Set_MP615_CLK_LOW();

		if(data & 0x01)
			Set_MP615_CDTI_HIGH()//D0
		else
			Set_MP615_CDTI_LOW();//D0

		MP615_DELAY();
		Set_MP615_CLK_HIGH();
		MP615_DELAY();

		MP615_SendStop();//at the end after send data
	}

	return TRUE;
}//end of AK4387RegWrite



/*******if want to use, Please check************/
BYTE AK4387RegRead(DWORD regaddr, DWORD value)
{
	DWORD temp;
	DWORD data;

	if (regaddr < 0x05){	 //0x05 is the invalid address
		MP615_SendStart();
		// Set_MP615_CS_LOW();  //at the beginning b4 send data
		//Set_MP615_CLK_LOW(); //at the beginning b4 send data
		Set_MP615_CDTI_Output();//for address output

		Set_MP615_CDTI_LOW();//C1 always 0
		MP615_DELAY();
		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		Set_MP615_CLK_LOW();

		Set_MP615_CDTI_HIGH();//C0 always 1
		MP615_DELAY();
		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		Set_MP615_CLK_LOW();

		Set_MP615_CDTI_LOW();// read Mode
		MP615_DELAY();
		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		Set_MP615_CLK_LOW();

		AK4387SetAddr(regaddr);

		Set_MP615_CDTI_Input();//for data read

		value=0x0;
		data=0x0;
		temp=0x0;
		temp = g_psGpio->Gpdat0;

		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		temp = g_psGpio->Gpdat0;
		Set_MP615_CLK_LOW();
		MP615_DELAY();

		data =(temp & 0x00002000)>>13;
		value += (data << 7);//read D7

		temp=0x0;


		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		temp = g_psGpio->Gpdat0;
		Set_MP615_CLK_LOW();
		MP615_DELAY();
		//   temp = g_psGpio->Gpdat0;
		data =(temp & 0x00002000)>>13;
		value += (data << 6);;//read D6

		temp=0x0;

		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		temp = g_psGpio->Gpdat0;
		Set_MP615_CLK_LOW();
		MP615_DELAY();
		//  temp = g_psGpio->Gpdat0;
		data =(temp & 0x00002000)>>13;
		value += (data << 5);//read D5

		temp=0x0;

		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		temp = g_psGpio->Gpdat0;
		Set_MP615_CLK_LOW();
		MP615_DELAY();
		//   temp = g_psGpio->Gpdat0;
		data =(temp & 0x00002000)>>13;
		value += (data << 4);//read D4

		temp=0x0;

		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		temp = g_psGpio->Gpdat0;
		Set_MP615_CLK_LOW();
		MP615_DELAY();
		//  temp = g_psGpio->Gpdat0;
		data =(temp & 0x00002000)>>13;
		value += (data << 3);//read D3

		temp=0x0;

		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		temp = g_psGpio->Gpdat0;
		Set_MP615_CLK_LOW();
		MP615_DELAY();
		// temp = g_psGpio->Gpdat0;
		data =(temp & 0x00002000)>>13;
		value += (data << 2);//read D2

		temp=0x0;

		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		temp = g_psGpio->Gpdat0;
		Set_MP615_CLK_LOW();
		MP615_DELAY();
		// temp = g_psGpio->Gpdat0;
		data =(temp & 0x00002000)>>13;
		value += (data << 1);;//read D1

		temp=0x0;

		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		temp = g_psGpio->Gpdat0;
		Set_MP615_CLK_LOW();
		MP615_DELAY();
		//  temp = g_psGpio->Gpdat0;
		data =(temp & 0x00002000)>>13;
		value += data;//read D0

		Set_MP615_CS_HIGH();
	}
	else
		return FALSE;

	return TRUE;
}//end of AK4387RegRead


BYTE AK4387SetAddr(WORD regaddr)
{
	int i = 5;
	while (i--)
	{
		if(regaddr & 0x10)
			Set_MP615_CDTI_HIGH()//A4
		else
			Set_MP615_CDTI_LOW();

		MP615_DELAY();
		Set_MP615_CLK_HIGH();
		MP615_DELAY();
		Set_MP615_CLK_LOW();
		regaddr <<= 1;
	}
}

void AudioClose_AK4387(void)
{
    AK4387RegWrite(0x01, 0x01);
    MP615_DELAY();
    Set_MP615_RST_LOW();//power off mode
}

/**
 * AK4387 Volume : 0 = Max Volume , 0x3f = Mute
 *
 * @retval = 0 error
 * @retval = 1 success
 */
int DAC_AK4387_SetVolume(WORD wVol)
{
	register AIU *aiu;
	aiu = (AIU *) (AIU_BASE);

	if ((aiu->AiuCtl >> 30) == 0)//check GAI
	{
		g_bVolumeIndex |= 0x40;
		return 1;
	}

	// Master Volume Register, 6-bit resolution
	AK4387RegWrite(0x03, VolumeSequence[wVol]) ;
	AK4387RegWrite(0x04, VolumeSequence[wVol]);
}


/**
 ***************************************************************
 *
 * Init AK4387 codec
 *
 *  Input  : none
 *
 *  Output : none
 ***************************************************************
*/

void  AK4387CodecInit(void)
{
	//mpDebugPrint("AK4387 init!!");
	AK4387GPIO_Init();

	Set_MP615_RST_HIGH();	//power on mode

	g_bVolumeIndex &= 0x3f;
	AK4387RegWrite(0x00, 0x03);	// Using 16bit LSB Justified
//	AK4387RegWrite(0x01, 0x02);	// Default setting as AK4387
//	AK4387RegWrite(0x02, 0x00);	// Default setting as AK4387
	AK4387RegWrite(0x03, VolumeSequence[g_bVolumeIndex]);
	AK4387RegWrite(0x04, VolumeSequence[g_bVolumeIndex]);

	AIU_PLAYBACK_GAI_INIT();
	//g_psAiu->AiuCtl  = 0x60800040; //0x6080f070;	//Enable GAI_EN, FIFO_CLR, PLY_EN, FSH_MASK
#if (STD_BOARD_VER == MP650_FPGA)
	AIU_SET_GENERAL_WAVEFORM();
	AIU_SET_LRCLOCK_LEN(0x1f);
#else
	AIU_SET_GENERAL_WAVEFORM();
	AIU_SET_LRCLOCK_LEN(0x3f);
#endif
}

/**
 ***************************************************************
 *
 * Init IIS AK4387 codec and AIU module
 *
 *  Input  : none
 *
 *  Output : none
 ***************************************************************
 */
inline void AudioInit_AK4387(void)
{
	register INTERRUPT *interrupt;

	AIUCfg_AK4387();

	interrupt = (INTERRUPT *)(INT_BASE);
	interrupt->MiMask |= 0x00000800;		//enable AUDIO module interrupt
}

HAL_AUDIODAC_T _audioDAC_AK4387 =
{
	AudioInit_AK4387,
	NULL,
	AudioClose_AK4387,
	NULL,
	NULL,
	DAC_AK4387_SetVolume
};
#endif

