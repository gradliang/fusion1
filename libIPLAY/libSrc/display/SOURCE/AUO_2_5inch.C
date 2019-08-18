#include "global612.h"
#include "../../../libSrc/Peripheral/include/hal_gpio.h"
#include "../../../libSrc/Peripheral/include/gpio_MP650.h"

//--for Read Register setting--//
#define Check_Read_Value  


#if (TCON_ID == TCON_RGB8SerialMode_320x240)

//#define UP052_320_mode 0

#define SPI_CS      25			//VGPIO_25
#define SPI_DI      26			//VGPIO_26
#define SPI_CLK     27			//VGPIO_27
#define SPI_DO      23			//GPIO_23

void AUO_SPI_Wait()
{
	IODelay(10);
}

#if 0
void SetVGPIOValue(BYTE gpio_num, BYTE value)
{
	register GPIO *vGpio;
  vGpio = (GPIO *)(GPIO_BASE);
  DWORD gpval = 0 ;

  //mpDebugPrint("High Low = %d", value);
  //mpDebugPrint("Or value = 0x%08x", vGpio->Vgpdat0);
  if(value == 1)
  	vGpio->Vgpdat0 |= (1<<gpio_num) ;
  else if(value == 0)
  {
  	//mpDebugPrint("gpio_num = %d", gpio_num);
  	gpval = 0xffffffff & (~(1<<gpio_num)) ;
  	//mpDebugPrint("gpval = 0x%08x", gpval);
  	vGpio->Vgpdat0 &= gpval ;
  }

	//mpDebugPrint("F value = 0x%08x", vGpio->Vgpdat0);
}
#else
void SetVGPIOValue(BYTE gpio_num, BYTE value)
{
	register GPIO *vGpio;
  vGpio = (GPIO *)(GPIO_BASE);
  DWORD gpval = 0 ;
  BYTE gpio_num_shift;


  if(gpio_num<15)
  {
		vGpio->Vgpdat0 |= (1<<(gpio_num+16));	// set output
  }
  else
  {
		vGpio->Vgpdat1 |= (1<<(gpio_num));	// set output
  }

  if(gpio_num>15)
  {
	 gpio_num_shift = gpio_num-16;
  }
  else
  {
	 gpio_num_shift = gpio_num;
  }
  //mpDebugPrint("High Low = %d", value);
  //mpDebugPrint("Or value = 0x%08x", vGpio->Vgpdat0);
  if(value == 1)
  {
	if(gpio_num<15)
  	{
		vGpio->Vgpdat0 |= (1<<gpio_num_shift) ;
	}
	else
	{
		vGpio->Vgpdat1 |= (1<<gpio_num_shift) ;

	}
  }
  else if(value == 0)
  {
  	//mpDebugPrint("gpio_num = %d", gpio_num);
  	gpval = 0xffffffff & (~(1<<gpio_num_shift)) ;
  	//mpDebugPrint("gpval = 0x%08x", gpval);
	if(gpio_num<15)
  	{
		vGpio->Vgpdat0 &= gpval;
	}
	else
	{
		vGpio->Vgpdat1 &= gpval;

	}

  }

	//mpDebugPrint("F value = 0x%08x", vGpio->Vgpdat0);
}

#endif


BYTE GetVGPIOValue(BYTE gpio_num)
{
	register GPIO *vGpio;
  vGpio = (GPIO *)(GPIO_BASE);
  DWORD gpval0 = 0 ;
  DWORD gpval1 = 0 ;
  BYTE gpio_num_shift;
  

  if(gpio_num>15)
  {
	 gpio_num_shift = gpio_num-16;
  }
  else
  {
	 gpio_num_shift = gpio_num;
  }

  gpval0 = (1<<gpio_num_shift) ;
  gpval1 = 0xffffffff & (~(1<<gpio_num_shift)) ;
  
  
  if(gpio_num<15)
  {
        if(vGpio->Vgpdat0 & gpval0)
			return 1;
		else if(vGpio->Vgpdat0 & gpval1)
			return 0;
  }
  else
  {
		if(vGpio->Vgpdat1 & gpval0)
			return 1;
		else if(vGpio->Vgpdat1 & gpval1)
			return 0;

   }
  
}

WORD AUO_SPI_Read(BYTE bAddr)
{
	BYTE i, j = 0 ;//, value=0;
	WORD value;
    

	for (i = 0; i < 5; i++)
	{
		SetVGPIOValue(SPI_CLK, 0);
		
		if (bAddr & 0x10)
		{
			SetVGPIOValue(SPI_DI, 1);
		}
		else
		{
			SetVGPIOValue(SPI_DI, 0);
		}

		bAddr <<= 1;
		AUO_SPI_Wait();
		SetVGPIOValue(SPI_CLK, 1);
		AUO_SPI_Wait();
	}
	for (i = 0; i < 11; i++)		// 8 ~ 15 Data
	{
		SetVGPIOValue(SPI_CLK, 0);

		if (i > 0)
		{
			j = (value << 1);
			value = j;

			if (GetVGPIOValue(SPI_DO))
				value |= 0x1;
		}
		SetVGPIOValue(SPI_CLK, 1);
		AUO_SPI_Wait();
	}
	SetVGPIOValue(SPI_CS, 0);
	SetVGPIOValue(SPI_DI, 0);
	
	return value;
}

void AUO_SPI_Write(BYTE bAddr, BYTE bData)
{
	BYTE i;
	register GPIO *vGpio;
  vGpio = (GPIO *)(GPIO_BASE);

	mpDebugPrint("### bData = %x, bAddr = %x", bData, bAddr);
	IODelay(10);
	SetVGPIOValue(SPI_CS, 0);

	for (i = 0; i < 8; i++)
	{
		SetVGPIOValue(SPI_CLK, 0);
		if (bAddr & 0x80)
		{
			SetVGPIOValue(SPI_DI, 1);
		}
		else
		{
			SetVGPIOValue(SPI_DI, 0);
		}

		bAddr <<= 1;
		AUO_SPI_Wait();
		SetVGPIOValue(SPI_CLK, 1);
		AUO_SPI_Wait();
	}
	for (i = 0; i < 8; i++)
	{
		SetVGPIOValue(SPI_CLK, 0);
		if (bData & 0x80)
			SetVGPIOValue(SPI_DI, 1);
		else
			SetVGPIOValue(SPI_DI, 0);

		bData <<= 1;
		AUO_SPI_Wait();
		SetVGPIOValue(SPI_CLK, 1);
		AUO_SPI_Wait();
	}

	SetVGPIOValue(SPI_CS, 1);
	SetVGPIOValue(SPI_DI, 0);
	AUO_SPI_Wait();
	AUO_SPI_Wait();
}


void AUO_2_5inchInit(void)
{
	register GPIO *sGpio;
    sGpio = (GPIO *)(GPIO_BASE);
    ST_PANEL *pstPanel = g_psSystemConfig->sScreenSetting.pstPanel;
	mpDebugPrint("~~~~~~~~~~AUO_2_5inchInit");
    register GPIO *gpio = (GPIO *) (GPIO_BASE);
	//gpio->Vgpcfg0 = 0x000000ff;	//gpio 0~7 select IDU function I
	//gpio->Vgpdat0 = 0x70000000;	// set out
//	gpio->Vgpdat0 = 0x00000000;
//	gpio->Vgpdat1 = 0x00000000;
	if(SPI_CS<15)
	{
		gpio->Vgpdat0 |= (1<<(SPI_CS+16));	// set out
	}
	else
	{
		gpio->Vgpdat1 |= (1<<(SPI_CS));	// set out
	}
	if(SPI_DI<15)
	{
		gpio->Vgpdat0 |= (1<<(SPI_DI+16));	// set out
	}
	else
	{
		gpio->Vgpdat1 |= (1<<(SPI_DI));	// set out
	}
	if(SPI_CLK<15)
	{
		gpio->Vgpdat0 |=(1<<(SPI_CLK+16));	// set out
	}
	else
	{
		gpio->Vgpdat1 |= (1<<(SPI_CLK));	// set out
	}

	SetVGPIOValue(SPI_CLK, 1);
	SetVGPIOValue(SPI_CS, 1);
	WaitMs(52);
	#if 0 //UP052_320_mode //320 mode
	AUO_SPI_Write(0x05, 0x16);
	AUO_SPI_Write(0x04, 0x1b);
	AUO_SPI_Write(0x08, 0xc0);
	//AUO_SPI_Write(0x01, 0x58); //test pattern
	AUO_SPI_Write(0x05, 0x47);
    #else
    // up-051 mode
	AUO_SPI_Write(0x05, 0x16);
	AUO_SPI_Write(0x04, 0x0b);
	AUO_SPI_Write(0x07, 0xd9);
	AUO_SPI_Write(0x08, 0xc0); //test pattern
	AUO_SPI_Write(0x05, 0x47);
    
	
	#endif
}


#endif



#if (TCON_ID == TCON_RGB8SerialMode_480x240)  


#define SPI_CS      12			//GPIO_12
#define SPI_DI      1			//GPIO_1
#define SPI_CLK     0			//GPIO_0


void SPI_Wait()
{
	IODelay(10);
}

void SetGGPIOValue(BYTE gpio_num, BYTE value)
{
	register GPIO *vGpio;
  vGpio = (GPIO *)(GPIO_BASE);
  DWORD gpval = 0 ;
  BYTE gpio_num_shift;

  if(gpio_num<15)
  {
		vGpio->Gpdat0 |= (1<<(gpio_num+16));	// set output
  }
  else
  {
		vGpio->Gpdat1 |= (1<<(gpio_num));	// set output
  }
	
  if(gpio_num>15)
  {
	 gpio_num_shift = gpio_num-16;
  }
  else
  {
	 gpio_num_shift = gpio_num;
  }

  if(value == 1)
  {
	if(gpio_num<15)
  	{
		vGpio->Gpdat0 |= (1<<gpio_num_shift) ;
	}
	else
	{
		vGpio->Gpdat1 |= (1<<gpio_num_shift) ;

	}
  }
  else if(value == 0)
  {
  	gpval = 0xffffffff & (~(1<<gpio_num_shift)) ;
	
	if(gpio_num<15)
  	{
		vGpio->Gpdat0 &= gpval;
	}
	else
	{
		vGpio->Gpdat1 &= gpval;

	}

  }

	//mpDebugPrint("F value = 0x%08x", vGpio->Vgpdat0);
}


BYTE GetGGPIOValue(BYTE gpio_num)
{
  BYTE value;
  DWORD gpval0 = 0 ;
  DWORD gpval1 = 0 ;
  BYTE gpio_num_shift;
  register GPIO *Gpio;
  Gpio = (GPIO *)(GPIO_BASE);

  if(gpio_num<15)
  {
		Gpio->Gpdat0 &= ~(1<<(gpio_num+16));	// set input
  }
  else
  {
		Gpio->Gpdat1 &= ~(1<<(gpio_num));	// set input
  }

  
  if(gpio_num>15)
  {
	 gpio_num_shift = gpio_num-16;
  }
  else
  {
	 gpio_num_shift = gpio_num;
  }

  gpval0 = (1<<gpio_num_shift) ;
  gpval1 = 0xffffffff & (~(1<<gpio_num_shift)) ;
   
  if(gpio_num < 15)
  {
    if(Gpio->Gpdat0 & gpval0)
		value = 1; 
	else 
		value = 0; 
  }
  else
  {
	if(Gpio->Gpdat1 & gpval0)
		value = 1; 
	else 
		value = 0; 
  }

  return value;
 
}

WORD SPI_Read(BYTE bAddr)
{
	BYTE i ;//, value=0;
	BYTE m = 0x80 ;
	BYTE tempdata ;
	WORD j = 0 ;
    WORD value = 0x0000 ;
	register GPIO *Gpio;
    Gpio = (GPIO *)(GPIO_BASE);
    bAddr = (bAddr<<4) | 0x8 ;
	
	IODelay(10);
	SetGGPIOValue(SPI_CS, 0);

	for (i = 0; i < 5; i++)
	{
		SetGGPIOValue(SPI_CLK, 0);
		
		if (bAddr & m)
		{
			SetGGPIOValue(SPI_DI, 1);
		}
		else
		{
			SetGGPIOValue(SPI_DI, 0);
		}

		m >>= 1;
		SPI_Wait();
		SetGGPIOValue(SPI_CLK, 1);
		SPI_Wait();
	}

    //if(SPI_DI < 15)
	    //Gpio->Gpdat0 &= ~(1<<(SPI_DI+16));	// set input
    //else
		//Gpio->Gpdat1 &= ~(1<<(SPI_DI));	// set input
   
	
	for ( ; i < 16 ; i++)		// 0 ~ 15 Data
	{
		SetGGPIOValue(SPI_CLK, 0);

		if (GetGGPIOValue(SPI_DI))
		{
		  if(value & 0xff)
		   {
		    value <<= 1 ;
			value |= 0x1;
		   }
		  else
		  	value |= 0x1;
		}
		else
		{
		   if(value & 0xff)
		   {
		    value <<= 1 ;
			value &= 0xff;
		   }
		   else
			value &= 0xfe;
		}
		
		SPI_Wait();
		SetGGPIOValue(SPI_CLK, 1);
		SPI_Wait();

	}

#ifdef Check_Read_Value
    mpDebugPrint("value_end = 0x%x",value);
#endif
	
	SetGGPIOValue(SPI_CS, 1);


	return value;
}


void SPI_Write(BYTE bAddr, WORD bData)
{
	BYTE i ;
	BYTE m = 0x80 ;
	register GPIO *Gpio;
    Gpio = (GPIO *)(GPIO_BASE);
    bAddr = (bAddr<<4) | 0x0 ;

	IODelay(10);
	SetGGPIOValue(SPI_CS, 0);

	for (i = 0; i < 5; i++)
	{
		SetGGPIOValue(SPI_CLK, 0);
		
		if (bAddr & m)
			SetGGPIOValue(SPI_DI, 1);
		else
			SetGGPIOValue(SPI_DI, 0);

		m >>= 1;
		SPI_Wait();
		SetGGPIOValue(SPI_CLK, 1);
		SPI_Wait();
	}

	for (i = 0; i < 11; i++)
	{
		SetGGPIOValue(SPI_CLK, 0);
		
		if (bData & 0x400)
			SetGGPIOValue(SPI_DI, 1);
		else
			SetGGPIOValue(SPI_DI, 0);

		bData <<= 1;
		SPI_Wait();
		SetGGPIOValue(SPI_CLK, 1);
		SPI_Wait();
	}
	
	SetGGPIOValue(SPI_CS, 1);
	SetGGPIOValue(SPI_DI, 1);
	SPI_Wait();
	SPI_Wait();
}

void ORISE_Init(void)
{
    BYTE i;
    ST_PANEL *pstPanel = g_psSystemConfig->sScreenSetting.pstPanel;
	mpDebugPrint("~~~~~~~~~~ORISE_Init");
    register GPIO *gpio = (GPIO *) (GPIO_BASE);


	SetGGPIOValue(SPI_CLK, 1);
	SetGGPIOValue(SPI_CS, 1);

	WaitMs(52);

	SPI_Write(0x06 , 0x2);
	SPI_Read(0x06);

/*
	DWORD time1,time2;
	time1=GetSysTime();
	mpDebugPrint("xxxxxxxxxxxxxxxxxx");
	time2=GetSysTime();
	mpDebugPrint("1:%d 2:%d",time1,time2);
*/
}

#endif

#if (TCON_ID == TCON_RGB24Mode_FUJI_320x240)  


#define SPI_CS      2			//PGPIO_2
#define SPI_DI      0			//PGPIO_0
#define SPI_CLK     3			//PGPIO_3
#define SPI_Chg     1			//PGPIO_1

void SPI_Wait()
{
	IODelay(10);
}


void SetPGPIOValue(BYTE gpio_num, BYTE value)
{
	register GPIO *Gpio;
  Gpio = (GPIO *)(GPIO_BASE);
  DWORD gpval = 0 ;
  BYTE gpio_num_shift;

  Gpio->Pgpdat |= (1<<(gpio_num+16));	// set output

  gpio_num_shift = gpio_num;

  if(value == 1)
  {
	Gpio->Pgpdat |= (1<<gpio_num_shift) ;
  }
  else if(value == 0)
  {
  	gpval = 0xffffffff & (~(1<<gpio_num_shift)) ;

	Gpio->Pgpdat &= gpval;
  }
	
}


BYTE GetPGPIOValue(BYTE gpio_num)
{
  BYTE value;
  DWORD gpval0 = 0 ;
  DWORD gpval1 = 0 ;
  BYTE gpio_num_shift;
  register GPIO *Gpio;
  Gpio = (GPIO *)(GPIO_BASE);

  Gpio->Pgpdat &= ~(1<<(gpio_num+16));	// set input

  gpio_num_shift = gpio_num;

  gpval0 = (1<<gpio_num_shift) ;
  gpval1 = 0xffffffff & (~(1<<gpio_num_shift)) ;
   

  if(Gpio->Pgpdat & gpval0)
		value = 1; 
  else 
		value = 0; 
 
  return value;
 
}

BYTE Write_Command(BYTE OAddr)
{
     OAddr = (OAddr<<1|0x1); //--0x1 for write command
	 return OAddr;
}

BYTE Read_Command(BYTE OAddr)
{
     OAddr = (OAddr<<1|0x0); //--0x0 for write command
	 return OAddr;
}

WORD SPI_Read(BYTE bAddr)
{
	BYTE i ;
	BYTE m = 0x80 ;
	BYTE tempdata ;
	WORD j = 0 ;
    WORD value = 0x0000 ;
	register GPIO *Gpio;
    Gpio = (GPIO *)(GPIO_BASE);
    bAddr = (bAddr<<1);
	
	IODelay(10);
	SetPGPIOValue(SPI_CS, 0);

	for (i = 0; i < 7; i++)
	{
		SetPGPIOValue(SPI_CLK, 0);
		
		if (bAddr & m)
		{
			SetPGPIOValue(SPI_DI, 1);
		}
		else
		{
			SetPGPIOValue(SPI_DI, 0);
		}

		m >>= 1;
		SPI_Wait();
		SetPGPIOValue(SPI_CLK, 1);
		SPI_Wait();
	}

    //if(SPI_DI < 15)
	    //Gpio->Gpdat0 &= ~(1<<(SPI_DI+16));	// set input
    //else
		//Gpio->Gpdat1 &= ~(1<<(SPI_DI));	// set input
   
	
	for ( ; i < 16 ; i++)		// 0 ~ 15 Data
	{
		SetPGPIOValue(SPI_CLK, 0);

		if (GetPGPIOValue(SPI_DI))
		{
		  if(value & 0xff)
		   {
		    value <<= 1 ;
			value |= 0x1;
		   }
		  else
		  	value |= 0x1;
		}
		else
		{
		   if(value & 0xff)
		   {
		    value <<= 1 ;
			value &= 0xff;
		   }
		   else
			value &= 0xfe;
		}
		
		SPI_Wait();
		SetPGPIOValue(SPI_CLK, 1);
		SPI_Wait();

	}
	
#ifdef Check_Read_Value
    mpDebugPrint("value_end = 0x%x",value);
#endif

	SetPGPIOValue(SPI_CS, 1);


	return value;
}


void SPI_Write(BYTE bAddr, WORD bData)
{
	BYTE i ;
	BYTE m = 0x80 ;
	register GPIO *Gpio;
    Gpio = (GPIO *)(GPIO_BASE);
    bAddr = (bAddr<<1);

	IODelay(10);
	SetPGPIOValue(SPI_CS, 0);

	for (i = 0; i < 7; i++)
	{
		SetPGPIOValue(SPI_CLK, 0);
		
		if (bAddr & m)
			SetPGPIOValue(SPI_DI, 1);
		else
			SetPGPIOValue(SPI_DI, 0);

		m >>= 1;
		SPI_Wait();
		SetPGPIOValue(SPI_CLK, 1);
		SPI_Wait();
	}

	for (i = 0; i < 9; i++)
	{
		SetPGPIOValue(SPI_CLK, 0);
		
		if (bData & 0x100)
			SetPGPIOValue(SPI_DI, 1);
		else
			SetPGPIOValue(SPI_DI, 0);

		bData <<= 1;
		SPI_Wait();
		SetPGPIOValue(SPI_CLK, 1);
		SPI_Wait();
	}
	
	SetPGPIOValue(SPI_CS, 1);
	SetPGPIOValue(SPI_DI, 1);
	SPI_Wait();
	SPI_Wait();
}


void NOVA_Init(void)
{
    BYTE i;
    ST_PANEL *pstPanel = g_psSystemConfig->sScreenSetting.pstPanel;
	mpDebugPrint("~~~~~~~~~~NOVA_Init");
	register GPIO *Gpio = (GPIO *) (GPIO_BASE);

	Gpio->Pgpcfg |= 0x0000000F;

    SetPGPIOValue(SPI_CLK, 0);
	SetPGPIOValue(SPI_CS, 0);
	SetPGPIOValue(SPI_DI, 0);

	SPI_Wait();

	SPI_Write( Write_Command(0x03) , 0x8C); 
	//SPI_Read(  Read_Command(0x03) );


}

#endif
