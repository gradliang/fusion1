
#include "global612.h"
#include "../../../libSrc/Peripheral/include/hal_gpio.h"
#include "../../../libSrc/Peripheral/include/gpio_MP650.h"



#define SPI_CS      12			//GPIO_12
#define SPI_DI      1			//GPIO_1
#define SPI_CLK     0			//GPIO_0
//--DB[0~5]  --//
#define DB_NUMBER  8


void ILI9342_SPI_Wait()
{
	IODelay(10);
}

void ILI9342_SetGPIOValue(BYTE gpio_num, BYTE value)
{
  register GPIO *Gpio;
  Gpio = (GPIO *)(GPIO_BASE);
  DWORD gpval = 0 ;
  BYTE gpio_num_shift;

  if(gpio_num<15)
  {
		Gpio->Gpdat0 |= (1<<(gpio_num+16));	// set output
  }
  else
  {
		Gpio->Gpdat1 |= (1<<(gpio_num));	// set output
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
		Gpio->Gpdat0 |= (1<<gpio_num_shift) ;
	}
	else
	{
		Gpio->Gpdat1 |= (1<<gpio_num_shift) ;

	}
  }
  else if(value == 0)
  {
  	gpval = 0xffffffff & (~(1<<gpio_num_shift)) ;
	
	if(gpio_num<15)
  	{
		Gpio->Gpdat0 &= gpval;
	}
	else
	{
		Gpio->Gpdat1 &= gpval;
	}

  }
}


BYTE ILI9342_GetVGPIOValue(BYTE gpio_num)
{
  BYTE value;
  DWORD gpval0 = 0 ;
  DWORD gpval1 = 0 ;
  BYTE gpio_num_shift;
  register GPIO *Gpio;
  Gpio = (GPIO *)(GPIO_BASE);

  if(gpio_num<15)
  {
		Gpio->Vgpdat0 &= ~(1<<(gpio_num+16));	// set input
  }
  else
  {
		Gpio->Vgpdat1 &= ~(1<<(gpio_num));	// set input
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
    if(Gpio->Vgpdat0 & gpval0)
		value = 1; 
	else 
		value = 0; 
  }
  else
  {
	if(Gpio->Vgpdat1 & gpval0)
		value = 1; 
	else 
		value = 0; 
  }
  return value;
}


BYTE ILI9342_GetGPIOValue(BYTE gpio_num)
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

void LCD_WRITE_COMMAND_ILI9342(BYTE bCommand)
{
    BYTE i;
	register GPIO *Gpio;
	Gpio = (GPIO *)(GPIO_BASE);
	
	ILI9163_SetGPIOValue(SPI_CS,0);

    //---use SDI as DCX---//

	ILI9163_SetGPIOValue(SPI_CLK,0);
		  IODelay(1);

	ILI9163_SetGPIOValue(SPI_DI,0);

	ILI9342_SPI_Wait();
	ILI9163_SetGPIOValue(SPI_CLK,1);
	ILI9342_SPI_Wait();
	//---------------------

	for( i = 0 ; i < DB_NUMBER ; i++)
	{
	   ILI9163_SetGPIOValue(SPI_CLK,0);
		  IODelay(1);
	
	  if((bCommand & 0x80))
	  {
	  	ILI9163_SetGPIOValue(SPI_DI, 1);
	  }
	  else
	  	ILI9163_SetGPIOValue(SPI_DI, 0);

      bCommand<<=1; 
      ILI9342_SPI_Wait();
	  ILI9163_SetGPIOValue(SPI_CLK,1);
	  ILI9342_SPI_Wait();
	  
	}


	ILI9163_SetGPIOValue(SPI_CS, 1);
	ILI9342_SPI_Wait();

}


void LCD_WRITE_DATA_ILI9342(BYTE bData)
{
    BYTE i;
	register GPIO *Gpio;
	Gpio = (GPIO *)(GPIO_BASE);

	
	ILI9163_SetGPIOValue(SPI_CS,0);
		  IODelay(1);

    //---use SDI as DCX---//

	ILI9163_SetGPIOValue(SPI_CLK,0);
		  IODelay(1);

	ILI9163_SetGPIOValue(SPI_DI,1);

	ILI9342_SPI_Wait();
	ILI9163_SetGPIOValue(SPI_CLK,1);
	ILI9342_SPI_Wait();
	//---------------------
	

	for( i = 0 ; i < DB_NUMBER ; i++ )
	{

      ILI9163_SetGPIOValue(SPI_CLK,0);
		  IODelay(1);
	
	  if(( bData & 0x80 ))
	  {
	    //mpDebugPrint("&");
	  	ILI9163_SetGPIOValue(SPI_DI,1);
	  }
	  else
	  	ILI9163_SetGPIOValue(SPI_DI,0);

      bData<<=1;

	  ILI9342_SPI_Wait();
	  ILI9163_SetGPIOValue(SPI_CLK,1);
	  ILI9342_SPI_Wait();	  
	}

    ILI9163_SetGPIOValue(SPI_CS, 1);
		  IODelay(1);
}


void LCD_READ_DATA_ILI9342(BYTE bCommand , BYTE r_count)
{
    BYTE i,j;
	WORD value = 0x0000;
	register GPIO *Gpio;
	Gpio = (GPIO *)(GPIO_BASE);
	

	ILI9163_SetGPIOValue(SPI_CS,0);

    //---use SDI as DCX---//

	ILI9163_SetGPIOValue(SPI_CLK,0);
		  IODelay(1);

	ILI9163_SetGPIOValue(SPI_DI,0);

	ILI9342_SPI_Wait();
	ILI9163_SetGPIOValue(SPI_CLK,1);
	ILI9342_SPI_Wait();
	//---------------------


	for( i = 0 ; i < DB_NUMBER ; i++)
	{
	   ILI9163_SetGPIOValue(SPI_CLK,0);
		  IODelay(1);
	
	  if((bCommand & 0x80))
	  {
	  	ILI9163_SetGPIOValue(SPI_DI, 1);
	  }
	  else
	  	ILI9163_SetGPIOValue(SPI_DI, 0);

      bCommand<<=1; 
      ILI9342_SPI_Wait();
	  ILI9163_SetGPIOValue(SPI_CLK,1);
	  ILI9342_SPI_Wait();
	  
	}


   if(r_count > 1) //not only read 8bits but read 24 or 32 bits
   {
     ILI9163_SetGPIOValue(SPI_CLK,0);
		  //IODelay(1);
     ILI9342_SPI_Wait();
	 ILI9163_SetGPIOValue(SPI_CLK,1);
	 ILI9342_SPI_Wait();
   }

   for( j = 0 ; j < r_count ; j++ )
   {
	for( i = 0 ; i < DB_NUMBER ; i++ )
	{

	    ILI9163_SetGPIOValue(SPI_CLK, 0);
		  IODelay(1);
	
	  	if(ILI9163_GetGPIOValue(SPI_DI))
	  	{
	  	  //mpDebugPrint("*");
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

	  ILI9342_SPI_Wait();
	  ILI9163_SetGPIOValue(SPI_CLK,1);
	  ILI9342_SPI_Wait();	
	}
   mpDebugPrint("value[%d] = 0x%x", j ,value);
}

    ILI9163_SetGPIOValue(SPI_CS, 1);
		  IODelay(1);

}

void LCD_EnterSleep_ILI9342(void)
{
	LCD_WRITE_COMMAND(0x10);
}


void LCD_ExitSleep_ILI9342(void)
{
	LCD_WRITE_COMMAND(0x11);
}


void ILI9342_Init(void)
{
    register GPIO *gpio = (GPIO *) (GPIO_BASE);
	register IDU *idu = (IDU *)(IDU_BASE);
	register CHANNEL *idu_dma = (CHANNEL *) (DMA_IDU_BASE);
	
	ILI9163_SetGPIOValue(SPI_CS,1);
	ILI9163_SetGPIOValue(SPI_CLK,1);

	mpDebugPrint("~~~~~~~~~~ILI9342_Init");
	{
		LCD_WRITE_COMMAND_ILI9342(0x01);//set reset
	
        LCD_WRITE_COMMAND_ILI9342(0xB9);//set EXTC
        LCD_WRITE_DATA_ILI9342(0xFF);
        LCD_WRITE_DATA_ILI9342(0x93);
        LCD_WRITE_DATA_ILI9342(0x42);

		LCD_WRITE_COMMAND_ILI9342(0x21); //set Display Inversion

		LCD_WRITE_COMMAND_ILI9342(0xF6); //set RM & RIM RGB interface
        LCD_WRITE_DATA_ILI9342(0x01); 
		LCD_WRITE_DATA_ILI9342(0x00); 
		LCD_WRITE_DATA_ILI9342(0x03);

	    LCD_WRITE_COMMAND_ILI9342(0xB0); //set Sync mode
        LCD_WRITE_DATA_ILI9342(0x60);//Sync mode

		//--- Set Panel back and front porch for panel ---//
		//--- and remember to divide the vale by 3 for our signal---//
		LCD_WRITE_COMMAND_ILI9342(0xB5);	
		LCD_WRITE_DATA_ILI9342(0x0A); 
		LCD_WRITE_DATA_ILI9342(0x03);  
		LCD_WRITE_DATA_ILI9342(0x0A);  //(0x0A)
		LCD_WRITE_DATA_ILI9342(0xDA);  //(0xDA)	

		LCD_WRITE_COMMAND_ILI9342(0x11); //Exit Sleep

		IODelay(10);

		LCD_WRITE_COMMAND_ILI9342(0x29); //Display on

        //--- Read data--- //
		LCD_READ_DATA_ILI9342(0x09, 4); //read dispaly status	
		LCD_READ_DATA_ILI9342(0x04, 3); //read display ID
		LCD_READ_DATA_ILI9342(0x0C, 1); //read display Pixel Format
        LCD_READ_DATA_ILI9342(0x0E, 1); //read display signal mode
	}


}





