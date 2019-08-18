
#include "global612.h"
#include "../../../libSrc/Peripheral/include/hal_gpio.h"
#include "../../../libSrc/Peripheral/include/gpio_MP650.h"
#include "car.h"

//#if (TCON_ID == TCON_RGBSerialMode_240x400)


#define SPI_NCS      6			//GPIO_6
#define SPI_SDI      6			//UGPIO_6
#define SPI_SCL      0			//GPIO_0
#define SPI_SDO      7			//UGPIO_7
#define GRAME_BYTE_SIZE   (240*400*16)/8
#define SPI_CLOCK_NUMBER  8
#define LCDSPITEST   0

void ILI9327_SPI_Wait()
{
	IODelay(10);
}

void ILI9327_SetGPIOValue(BYTE gpio_num, BYTE value)
{
	register GPIO *Gpio;
	Gpio = (GPIO *)(GPIO_BASE);
	DWORD gpval = 0 ;
	BYTE gpio_num_shift;
    Gpio->Gpdat0 |= BIT16<<gpio_num;
	gpio_num_shift = gpio_num;
  if(value == 1)
  {
		Gpio->Gpdat0 |= (1<<gpio_num_shift) ;
  }
  else if(value == 0)
  {
  	gpval = 0xffffffff & (~(1<<gpio_num_shift)) ;
	Gpio->Gpdat0 &= gpval;
  }
}

void ILI9327_SetUGPIOValue(BYTE gpio_num, BYTE value)
{
	register GPIO *Gpio;
	Gpio = (GPIO *)(GPIO_BASE);
	DWORD gpval = 0 ;
	BYTE gpio_num_shift;
    Gpio->Ugpdat |= BIT16<<gpio_num;
	gpio_num_shift = gpio_num;
  if(value == 1)
  {
		Gpio->Ugpdat |= (1<<gpio_num_shift) ;
  }
  else if(value == 0)
  {
  	gpval = 0xffffffff & (~(1<<gpio_num_shift)) ;
	Gpio->Ugpdat &= gpval;

  }
}

void ILI9327_SetVGPIOValue(BYTE gpio_num, BYTE value)
{
	register GPIO *Gpio;
	Gpio = (GPIO *)(GPIO_BASE);
    //Set GPIODIR OUTPUT
    if(gpio_num> 15)
		Gpio->Vgpdat1 |= BIT16<<(gpio_num-15);
	else	
    	Gpio->Vgpdat0 |= BIT16<<gpio_num;
	
  //mpDebugPrint("Gpio->Gpdat0 = %x", Gpio->Gpdat0);
  //mpDebugPrint("Or value = 0x%08x", vGpio->Vgpdat0);
  if(value == 1)
  {
    if(gpio_num> 15)
		Gpio->Vgpdat1 |= BIT0<<(gpio_num-15);
	else	
    	Gpio->Vgpdat0 |= BIT0<<gpio_num;
  }
  else if(value == 0)
  {
  	//mpDebugPrint("gpio_num = %d", gpio_num);
  	//mpDebugPrint("gpval = 0x%08x", gpval);
    if(gpio_num> 15)
		Gpio->Vgpdat1 &= ~(gpio_num-15);
	else	
    	Gpio->Vgpdat0 &= ~gpio_num;
  }

	//mpDebugPrint("F value = 0x%08x", Gpio->Gpdat0);
}


void LCD_WRITE_COMMAND(BYTE bCommand)
{
    BYTE i;
	
	ILI9327_SetUGPIOValue(SPI_SDI,0);
		  IODelay(1);

	ILI9327_SetGPIOValue(SPI_SCL,1);
		  IODelay(1);

	ILI9327_SetUGPIOValue(SPI_SDI,0);
		  IODelay(1);

	ILI9327_SetGPIOValue(SPI_SCL,0);
		  IODelay(1);



	for(i=0;i<SPI_CLOCK_NUMBER;i++)
	{
	  //mpDebugPrint("i %d",i);
	  if(bCommand&(BIT7>>i))
	  {
	  	ILI9327_SetUGPIOValue(SPI_SDI,1);
	  }
	  else
	  	ILI9327_SetUGPIOValue(SPI_SDI,0);
	  IODelay(1);
	  
	  ILI9327_SetGPIOValue(SPI_SCL,1);
	  	  IODelay(1);

	  if(bCommand&(BIT7>>i))
	  	ILI9327_SetUGPIOValue(SPI_SDI,1);
	  else
	  	{
	  	  //mpDebugPrint("i=%x",i);
	  	ILI9327_SetUGPIOValue(SPI_SDI,0);
	  	}
	  IODelay(1);
	  
	  ILI9327_SetGPIOValue(SPI_SCL,0);
	  IODelay(1);

	}
	//ILI9327_SetGPIOValue(SPI_SCL,1);


}

void LCD_WRITE_DATA(BYTE bData)
{
    BYTE i;
	ILI9327_SetUGPIOValue(SPI_SDI,1);
		  IODelay(1);

	ILI9327_SetGPIOValue(SPI_SCL,1);
		  IODelay(1);

	ILI9327_SetUGPIOValue(SPI_SDI,1);
		  IODelay(1);

	ILI9327_SetGPIOValue(SPI_SCL,0);
		  IODelay(1);



	for(i=0;i<SPI_CLOCK_NUMBER;i++)
	{
	  if(bData&(BIT7>>i))
	  	ILI9327_SetUGPIOValue(SPI_SDI,1);
	  else
	  	ILI9327_SetUGPIOValue(SPI_SDI,0);
	  IODelay(1);
	  
	  ILI9327_SetGPIOValue(SPI_SCL,1);
	  IODelay(1);

      if(bData&(BIT7>>i))
	  	ILI9327_SetUGPIOValue(SPI_SDI,1);
	  else
	  	{
	  	  //mpDebugPrint("i=%x",i);
	  	ILI9327_SetUGPIOValue(SPI_SDI,0);
	  	}
	  IODelay(1);
	  
	  ILI9327_SetGPIOValue(SPI_SCL,0);
	  	  IODelay(1);


	}
	//ILI9327_SetGPIOValue(SPI_SCL,0);

}

void LCD_EnterSleep_ILI9327(void)
{
	LCD_WRITE_COMMAND(0x10);
}
void LCD_ExitSleep_ILI9327(void)
{
	LCD_WRITE_COMMAND(0x11);
}
void ILI9327_Init(void)
{
    BYTE read = 0;
	int i=0,j=0;
	WORD imagex = 452;
    WORD imagey = 149;

	ILI9327_SetGPIOValue(SPI_NCS,0);
	ILI9327_SetGPIOValue(SPI_SCL,0);

	mpDebugPrint("~~~~~~~~~~ILI9327_Init");
	{
		LCD_WRITE_COMMAND(0xE9);
		LCD_WRITE_DATA (0x20);
		LCD_WRITE_COMMAND(0x11); //Exit Sleep
		IODelay(10);
		LCD_WRITE_COMMAND(0xD1);
		LCD_WRITE_DATA (0x00);
		LCD_WRITE_DATA (0x6B);
		LCD_WRITE_DATA (0x19);
		LCD_WRITE_COMMAND(0xD0);
		LCD_WRITE_DATA (0x07);
		LCD_WRITE_DATA (0x01);
		LCD_WRITE_DATA(0x90);	//0x04
		LCD_WRITE_COMMAND(0x36);//set access format
		//LCD_WRITE_DATA (0x08);
		LCD_WRITE_DATA (0x40);
		LCD_WRITE_COMMAND(0x3a);//set pixel format
		LCD_WRITE_DATA (0x55);
		LCD_WRITE_COMMAND(0xC1);
		LCD_WRITE_DATA (0x10);
		LCD_WRITE_DATA (0x10);
		LCD_WRITE_DATA (0x02);
		LCD_WRITE_DATA (0x02);
		LCD_WRITE_COMMAND(0xC0); //Set Default Gamma
		LCD_WRITE_DATA (0x00);
		LCD_WRITE_DATA (0x31);
		LCD_WRITE_DATA (0x00);
		LCD_WRITE_DATA (0x00);
		LCD_WRITE_DATA (0x01);
		LCD_WRITE_DATA (0x02);
		LCD_WRITE_COMMAND(0xC5); //Set frame rate
		LCD_WRITE_DATA (0x02);
		LCD_WRITE_COMMAND(0xD2); //power setting
		LCD_WRITE_DATA (0x01);
		LCD_WRITE_DATA (0x44);
		LCD_WRITE_COMMAND(0xC8); //Set Gamma
		LCD_WRITE_DATA (0x04);
		LCD_WRITE_DATA (0x67);
		LCD_WRITE_DATA (0x35);
		LCD_WRITE_DATA (0x04);
		LCD_WRITE_DATA (0x08);
		LCD_WRITE_DATA (0x06);
		LCD_WRITE_DATA (0x24);
		LCD_WRITE_DATA (0x01);
		LCD_WRITE_DATA (0x37);
		LCD_WRITE_DATA (0x40);
		LCD_WRITE_DATA (0x03);
		LCD_WRITE_DATA (0x10);
		LCD_WRITE_DATA (0x08);
		LCD_WRITE_DATA (0x80);
		LCD_WRITE_DATA (0x00);
		LCD_WRITE_COMMAND(0x29); //display on
		//====set RAM====//
		LCD_WRITE_COMMAND(0x2A); //Set col
		LCD_WRITE_DATA (0x00);
		LCD_WRITE_DATA (0x00);
		LCD_WRITE_DATA (0x00);
		LCD_WRITE_DATA (0xEF);
		LCD_WRITE_COMMAND(0x2B); //Set page
		LCD_WRITE_DATA (0x00);
		LCD_WRITE_DATA (0x00);
		LCD_WRITE_DATA (0x01);
		LCD_WRITE_DATA (0x8F);
		//LCD_WRITE_DATA (0xAF);
		LCD_WRITE_COMMAND(0x2C); 
#if	(LCDSPITEST == 0)
		LCD_WRITE_COMMAND(0xB4); //choose RGB interface(add 2011.8.16),default CPU interface
		LCD_WRITE_DATA (0x10);
#endif		
	}
#if	LCDSPITEST
#if	1
	for(i=0;i<GRAME_BYTE_SIZE;i+=2)
	{
	        
	        if(i<(GRAME_BYTE_SIZE/3))
	        {
	 			LCD_WRITE_DATA(0xF8);//Red 0xf800
				LCD_WRITE_DATA(0x00);
	        }else if(i<((GRAME_BYTE_SIZE/3)*2))
			{
				LCD_WRITE_DATA(0x07);//Green 0x07E0
				LCD_WRITE_DATA(0xE0);
			}else
			{
				LCD_WRITE_DATA(0x00);//Bule 0x001F
				LCD_WRITE_DATA(0x1F);
			}

	}
#else
	for(i=0;i<GRAME_BYTE_SIZE;i+=2)
	{
	 LCD_WRITE_DATA((sample_240X400[i]&0xFF00)>>8);
	 LCD_WRITE_DATA((sample_240X400[i]&0x00FF));
	}
#endif
#endif
}


//#endif



