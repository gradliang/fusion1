#include "global612.h"
#include "mpTrace.h"

#if (TCON_ID == TCON_HX8817A)	//Mason 2006/4/17

#define SPI_DI      21			//GPIO_21
#define SPI_CLK     20			//GPIO_20
#define SPI_CS      22			//GPIO_22
#define SPI_DO      23			//GPIO_23

void HIMAX_SPI_Delay(DWORD i)
{
	while (i != 0)
		i--;
}

void HIMAX_SPI_Wait()
{
	HIMAX_SPI_Delay(10);
}

void HIMAX_SPI_Write(BYTE bAddr, BYTE bData)
{
	BYTE i;

	SetGPIOValue(SPI_DI, 1);
	SetGPIOValue(SPI_CLK, 1);
	SetGPIOValue(SPI_CS, 0);
	HIMAX_SPI_Wait();
	HIMAX_SPI_Wait();

	SetGPIOValue(SPI_CLK, 1);
	SetGPIOValue(SPI_CS, 1);
	HIMAX_SPI_Wait();
	HIMAX_SPI_Wait();

	SetGPIOValue(SPI_CLK, 0);
	SetGPIOValue(SPI_DI, 0);
	HIMAX_SPI_Wait();
	SetGPIOValue(SPI_CLK, 1);
	HIMAX_SPI_Wait();

	for (i = 0; i < 5; i++)		// 2 ~ 6 Data
	{
		SetGPIOValue(SPI_CLK, 0);
		if (bAddr & 0x10)
			SetGPIOValue(SPI_DI, 1);
		else
			SetGPIOValue(SPI_DI, 0);

		bAddr <<= 1;
		HIMAX_SPI_Wait();
		SetGPIOValue(SPI_CLK, 1);
		HIMAX_SPI_Wait();
	}
	for (i = 0; i < 8; i++)		// 7 ~ 14 Data
	{
		SetGPIOValue(SPI_CLK, 0);
		if (bData & 0x80)
			SetGPIOValue(SPI_DI, 1);
		else
			SetGPIOValue(SPI_DI, 0);

		bData <<= 1;
		HIMAX_SPI_Wait();
		SetGPIOValue(SPI_CLK, 1);
		HIMAX_SPI_Wait();
	}
	SetGPIOValue(SPI_CLK, 0);
	HIMAX_SPI_Wait();
	SetGPIOValue(SPI_CLK, 1);
	HIMAX_SPI_Wait();
	SetGPIOValue(SPI_CS, 0);
	SetGPIOValue(SPI_DI, 0);
}

void HIMAX_SPI_Read(BYTE bAddr)
{
	BYTE i, j = 0, value=0;

	SetGPIOValue(SPI_DI, 1);
	SetGPIOValue(SPI_CLK, 1);
	SetGPIOValue(SPI_CS, 0);
	HIMAX_SPI_Wait();
	HIMAX_SPI_Wait();

	SetGPIOValue(SPI_CLK, 1);
	SetGPIOValue(SPI_CS, 1);
	HIMAX_SPI_Wait();
	HIMAX_SPI_Wait();

	SetGPIOValue(SPI_CLK, 0);
	SetGPIOValue(SPI_DI, 1);
	HIMAX_SPI_Wait();
	SetGPIOValue(SPI_CLK, 1);
	HIMAX_SPI_Wait();

	for (i = 0; i < 5; i++)		// 3 ~ 7 Data
	{
		SetGPIOValue(SPI_CLK, 0);
		if (bAddr & 0x10)
			SetGPIOValue(SPI_DI, 1);
		else
			SetGPIOValue(SPI_DI, 0);

		bAddr <<= 1;
		HIMAX_SPI_Wait();
		SetGPIOValue(SPI_CLK, 1);
		HIMAX_SPI_Wait();
	}
	for (i = 0; i < 9; i++)		// 8 ~ 15 Data
	{
		SetGPIOValue(SPI_CLK, 0);
		HIMAX_SPI_Wait();

		if (i > 0)
		{
			j = (value << 1);
			value = j;

			if (GetGPIOValue(SPI_DO))
				value |= 0x1;
		}
		SetGPIOValue(SPI_CLK, 1);
		HIMAX_SPI_Wait();
	}
	SetGPIOValue(SPI_CS, 0);
	SetGPIOValue(SPI_DI, 0);
	return value;
}

BYTE HIMAX_Table_AU56[] = {
	0x02, 0xfb, 0x03, 0x00, 0x04, 0x00, 0x05, 0x00, 0x06, 0x4f,
	0x07, 0x06, 0x08, 0x13, 0x09, 0x1c, 0x0a, 0x11, 0x0b, 0x15,
	0x0c, 0x0a, 0xff, 0xff
};
//Mason 20060717 updated	//Currently for H/W I2C only
void HIMAX_WtRegFromTable(BYTE *SettingTable)
{
        BYTE TablePointer=0,RegNumber=0,RegValue=0;
        RegNumber=*(SettingTable+TablePointer);
        RegValue=*(SettingTable+TablePointer+1);
        while(!((RegNumber==0xff)&&(RegValue==0xff)))
        {
                HIMAX_SPI_Write(RegNumber, RegValue);
                TablePointer+=2;
                RegNumber=*(SettingTable+TablePointer);
                RegValue=*(SettingTable+TablePointer+1);
        }
}

void HIMAX_Init (void)
{
    DWORD dwGpioMap;
    register GPIO *sGpio;
    sGpio = (GPIO *)(GPIO_BASE);
    dwGpioMap = ((1 << (SPI_DI & 0xf)) | (1 << (SPI_CLK & 0xf)) | (1 << (SPI_CS & 0xf)) | (1 << (SPI_DO & 0xf)));

    sGpio->Gpcfg1 &= 0xff0fff0f;//(((~dwGpioMap) << 4) | (~dwGpioMap));
    sGpio->Gpdat1 |= 0x00f00000;//((~dwGpioMap) << 4);
    HIMAX_WtRegFromTable(&HIMAX_Table_AU56);
}
#endif
