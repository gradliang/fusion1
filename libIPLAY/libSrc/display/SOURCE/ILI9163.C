
#include "global612.h"
#include "../../../libSrc/Peripheral/include/hal_gpio.h"
#include "../../../libSrc/Peripheral/include/gpio_MP650.h"
//#include "rainbow.h"


#define CSX      22			// VGPIO_22
#define DCX      26			// VGPIO_26
#define WRX      25			// VGPIO_25
#define RDX      12			// GPIO_12
//--DB[0~7] : VGPIO 0~7 --//
#define DB_NUMBER  8        // Data pins
//-- set command choice -- //
#define GPIO_mode  1        // 1: use GPIO to set command or data  0: use hardware I80 to set command
//-- set write frame type -- //
#define Method1  0          // use GPIO to write all frame data
#define Method2  0          // use hardware I80 to write all frame data
#define Method3  1          // use WRx to simulate I80 to write all frame data
//--test write/read control --//
#define testRW   0

int counter = 1;

void ILI9163_SPI_Wait()
{
	IODelay(10);
}


void ILI9163_SetVGPIOValue(BYTE gpio_num, BYTE value)
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
  	gpval = 0xffffffff & (~(1<<gpio_num_shift)) ;
	if(gpio_num<15)
  	{
		vGpio->Vgpdat0 &= gpval;
	}
	else
	{
		vGpio->Vgpdat1 &= gpval;

	}

  }
}


void ILI9163_SetGPIOValue(BYTE gpio_num, BYTE value)
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


BYTE ILI9163_GetVGPIOValue(BYTE gpio_num)
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


BYTE ILI9163_GetGPIOValue(BYTE gpio_num)
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

void ILI9163_Switch(BYTE altfunc)
{
    register GPIO *gpio = (GPIO *) (GPIO_BASE);

    if(altfunc == 0)
    {
       //---Change GPIOcfg to set Memory write ---// 
	    gpio->Vgpcfg0 = 0xFF00FF00 ;	//gpio 0~7 select IDU function 0
   	    gpio->Vgpcfg1 = 0x00000000 ;	//gpio 22 25 26 select IDU function 0
    }
	else 
	{
	   //---Change GPIOcfg to set write frame---// 
	    gpio->Vgpcfg0 = 0x000000FF;	//gpio 0~7 select IDU function 1
	    gpio->Vgpcfg1 = 0x02000200;	//gpio 22 26 select IDU function 0 gpio25 select IDU function 3
	}

}

#if (GPIO_mode == 1)

void ILI9163_LCD_WRITE_COMMAND(BYTE bCommand)
{
    BYTE i;
	register GPIO *Gpio;
	Gpio = (GPIO *)(GPIO_BASE);
	
	ILI9163_SetVGPIOValue(DCX,0);
		  IODelay(1);

	ILI9163_SetVGPIOValue(WRX,0);
		  IODelay(1);

	for( i = 0 ; i < DB_NUMBER ; i++)
	{
	  if((bCommand & 0x80))
	  {
	  	ILI9163_SetVGPIOValue((DB_NUMBER - 1 - i), 1);
	  }
	  else
	  	ILI9163_SetVGPIOValue((DB_NUMBER - 1 - i), 0);

      bCommand<<=1; 
	  
	  IODelay(1);	  
	}

	ILI9163_SetVGPIOValue(WRX, 1);
		  IODelay(1);

	ILI9163_SetVGPIOValue(DCX, 1);
		  IODelay(1);

}


void ILI9163_LCD_WRITE_DATA(BYTE bData)
{
    BYTE i;
	register GPIO *Gpio;
	Gpio = (GPIO *)(GPIO_BASE);

	
	ILI9163_SetVGPIOValue(DCX,1);
		  IODelay(1);

	ILI9163_SetVGPIOValue(WRX,0);
		  IODelay(1);

	for( i = 0 ; i < DB_NUMBER ; i++ )
	{
	  if(( bData & 0x80 ))
	  {
	    //mpDebugPrint("&");
	  	ILI9163_SetVGPIOValue((DB_NUMBER - 1 - i),1);
	  }
	  else
	  	ILI9163_SetVGPIOValue((DB_NUMBER - 1 - i),0);

      bData<<=1;
	  
	  IODelay(1);	  
	}

	ILI9163_SetVGPIOValue(WRX, 1);
		  IODelay(1);

    ILI9163_SetVGPIOValue(DCX, 0);
		  IODelay(1);
}


void ILI9163_LCD_READ_DATA()
{
    BYTE i;
	WORD value = 0x0000;
	register GPIO *Gpio;
	Gpio = (GPIO *)(GPIO_BASE);
	
	ILI9163_SetVGPIOValue(DCX, 1);
		  IODelay(1);

    ILI9163_SetVGPIOValue(WRX, 1);
		  IODelay(1);

	ILI9163_SetGPIOValue(RDX, 0);
		  IODelay(1);

	for( i = 0 ; i < DB_NUMBER ; i++ )
	{
	
	  	if(ILI9163_GetVGPIOValue((DB_NUMBER - 1 - i)))
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
	  
	  IODelay(1);	  
	  
	}

    mpDebugPrint("value = 0x%x", value);
	  
    ILI9163_SetVGPIOValue(DCX, 0);
		  IODelay(1);

}

void LCD_EnterSleep_ILI9163(void)
{
	ILI9163_SetVGPIOValue(CSX,0);
	IODelay(1);

	ILI9163_LCD_WRITE_COMMAND(0x10); // Memory write

	ILI9163_SetVGPIOValue(CSX,1);
	IODelay(1);
}


void LCD_ExitSleep_ILI9163(void)
{
	ILI9163_SetVGPIOValue(CSX,0);
	IODelay(1);

	ILI9163_LCD_WRITE_COMMAND(0x11); // Memory write

	ILI9163_SetVGPIOValue(CSX,1);
	IODelay(1);
}

void LCD_MemoryWrite_ILI9163(void)
{
	 ILI9163_SetVGPIOValue(CSX,0);
	 IODelay(1);

	 ILI9163_LCD_WRITE_COMMAND(0x2C); // Memory write

	 //ILI9163_SetVGPIOValue(CSX,1);
	 //IODelay(1);
}

void LCD_DisplayOn_ILI9163(void)
{
	 ILI9163_SetVGPIOValue(CSX,0);
	 IODelay(1);

	 ILI9163_LCD_WRITE_COMMAND(0x29); // DisplayOn

	 ILI9163_SetVGPIOValue(CSX,1);
	 IODelay(1);
}

#else

void ILI9163_LCD_WRITE_COMMAND(BYTE bCommand)
{
    BYTE i;
	register IDU *idu = (IDU *)(IDU_BASE);
	//register GPIO *Gpio;
	//Gpio = (GPIO *)(GPIO_BASE);

	//Gpio->Vgpcfg0 = 0x00FF00FF;	//gpio 0~7 select IDU function 3
	//Gpio->Vgpcfg1 = 0x06400640;	//gpio 22 25 26 select IDU function 3

	//idu->Mpu_Ctrl |= BIT31;
	
	//idu->Mpu_Cmd |= bCommand;

	//mpDebugPrint("idu->Mpu_Cmd = 0x%x",idu->Mpu_Cmd);

	//idu->Mpu_Ctrl &= ~BIT31;

}


void ILI9163_LCD_WRITE_DATA(BYTE bData)
{
    BYTE i;
	register IDU *idu = (IDU *)(IDU_BASE);
	//register GPIO *Gpio;
	//Gpio = (GPIO *)(GPIO_BASE);
	
	//Gpio->Vgpcfg0 = 0x00FF00FF;	//gpio 0~7 select IDU function 3
	//Gpio->Vgpcfg1 = 0x06400640;	//gpio 22 25 26 select IDU function 3


	//idu->Mpu_Ctrl |= BIT31;
	
    //idu->Mpu_Par|= bData;

	//idu->Mpu_Ctrl &= ~BIT31;

}
#endif

#ifdef Method1

void LCD_EnterPartial_ILI9163(WORD imgSizeY)
{
    WORD StartY, EndY;

	LCD_WRITE_COMMAND(0x30);

    StartY = (128 - imgSizeY)/2 ;
	EndY = 128 - StartY - 1;

	//---Y Star point
	ILI9163_LCD_WRITE_DATA( (StartY  & 0xFF00)>>8 );
	ILI9163_LCD_WRITE_DATA( (StartY  & 0x00FF) );
	//---Y End point
	ILI9163_LCD_WRITE_DATA( (EndY  & 0xFF00)>>8 );
	ILI9163_LCD_WRITE_DATA( (EndY  & 0x00FF) );

	ILI9163_LCD_WRITE_COMMAND(0x12);
	
}

void LCD_ExitPartial_ILI9163()
{

    ILI9163_LCD_WRITE_COMMAND(0x28);
	ILI9163_LCD_WRITE_COMMAND(0x13);
	
}
void LCD_WRITE_PIX(WORD imgSizeX, WORD imgSizeY, DWORD *imgData)
{
     int i;
	 WORD StartY, EndY;

	 if( imgSizeX < 128 || imgSizeY < 128)
     {
        LCD_EnterPartial_ILI9163(imgSizeY);
		
		StartY = (128 - imgSizeY)/2 ;
		EndY = 128 - StartY - 1;
     }
	 else 
	 {
        LCD_ExitPartial_ILI9163();
	 
	    StartY = 0;
		EndY = imgSizeY;
	 }

	 ILI9163_LCD_WRITE_COMMAND(0x2B);
	 //---Y Star point
     ILI9163_LCD_WRITE_DATA( (StartY  & 0xFF00)>>8 );
	 ILI9163_LCD_WRITE_DATA( (StartY  & 0x00FF) );
	 //---Y End point
	 ILI9163_LCD_WRITE_DATA( (EndY  & 0xFF00)>>8 );
	 ILI9163_LCD_WRITE_DATA( (EndY  & 0x00FF) );

     ILI9163_LCD_WRITE_COMMAND(0x2A);
	 //---X Star point
     ILI9163_LCD_WRITE_DATA(0x00);
	 ILI9163_LCD_WRITE_DATA(0x00);
	 //---X End point
	 ILI9163_LCD_WRITE_DATA( (imgSizeX & 0xFF00)>>8 );
	 ILI9163_LCD_WRITE_DATA( (imgSizeX & 0x00FF) );

	 ILI9163_LCD_WRITE_COMMAND(0x2C);

	 for( i = 0 ; i < imgSizeX*imgSizeY*3/4 ; i++ )
	 {
	    //LCD_WRITE_DATA(0xFC);
		//LCD_WRITE_DATA(0x00);
		//LCD_WRITE_DATA(0x00);
		ILI9163_LCD_WRITE_DATA((imgData[i] & 0xFC000000)>>24 );
		ILI9163_LCD_WRITE_DATA((imgData[i] & 0x00FC0000)>>16 );
		ILI9163_LCD_WRITE_DATA((imgData[i] & 0x0000FC00)>>8  ); 
		ILI9163_LCD_WRITE_DATA((imgData[i] & 0x000000FC)); 
	 }

     
	 ILI9163_LCD_WRITE_COMMAND(0x29); // Display On

	 
}

void LCD_SLIDESHOW_ILI9163(void)
{
   int i,j;
   i = 0;
   j = 0;
 
   while(i)
   {
    //LCD_WRITE_PIX(128, 128, rainbow);
    IODelay(961);
    //LCD_WRITE_PIX(128, 128, motor);
    IODelay(961);
    //LCD_WRITE_PIX(128, 128, Ta);
    IODelay(961);
    //LCD_WRITE_PIX(128, 128, Tb);
    IODelay(961);
    //LCD_WRITE_PIX(128, 128, Tc);
    IODelay(961);
    //LCD_WRITE_PIX(128, 128, Te);
    IODelay(961);
    //LCD_WRITE_PIX(128, 128, Tf);
    IODelay(961);
    //LCD_WRITE_PIX(128, 128, Tg);
    IODelay(961);
    //LCD_WRITE_PIX(128, 128, Th);
    IODelay(961);
    //LCD_WRITE_PIX(128, 128, Ti);
    IODelay(961);
   }
}

#endif


void ILI9163_Init(void)
{
    register GPIO *gpio = (GPIO *) (GPIO_BASE);
	register IDU *idu = (IDU *)(IDU_BASE);
	register CHANNEL *idu_dma = (CHANNEL *) (DMA_IDU_BASE);
	
    #if(GPIO_mode == 1)
	gpio->Vgpcfg0 = (gpio->Vgpcfg0 & 0xFF00FF00) | 0x00000000;	//gpio 0~7 select IDU function 0
	gpio->Vgpcfg1 = (gpio->Vgpcfg1 & 0xF64FF64F) | 0x00000000;	//gpio 22 25 26 select IDU function 0

	ILI9163_SetVGPIOValue(CSX,1);
	ILI9163_SetVGPIOValue(DCX,1);
	ILI9163_SetVGPIOValue(WRX,1);
	#else

	gpio->Vgpcfg0 = 0x00FF00FF;	//gpio 0~7 select IDU function 3
	gpio->Vgpcfg1 = 0x0FC00FC0;	//gpio 22 25 26 select IDU function 3
	//gpio->Vgpcfg1 = 0x0F800F80;	//gpio 22 25 26 select IDU function 3

	#endif
	
    mpDebugPrint("gpio->Vgpcfg0 = 0x%x",gpio->Vgpcfg0);
    mpDebugPrint("gpio->Vgpcfg1 = 0x%x",gpio->Vgpcfg1);
	
	mpDebugPrint("~~~~~~~~~~ILI9163_Init");
	{
#if testRW
    #if(GPIO_mode == 1)
       //-----below codes are for testing that read/write can actually work-----// 
        ILI9163_SetVGPIOValue(CSX,0);
		IODelay(1);

        ILI9163_LCD_WRITE_COMMAND(0x3A); 	
		ILI9163_LCD_WRITE_DATA(0x66);

		ILI9163_SetVGPIOValue(CSX,1);
		IODelay(1);

	    ILI9163_SetVGPIOValue(CSX,0);
		IODelay(1);
		
		ILI9163_LCD_WRITE_COMMAND(0x0C); 	  
		ILI9163_LCD_READ_DATA();
		ILI9163_LCD_WRITE_COMMAND(0x0C);
		ILI9163_LCD_READ_DATA();
		

		ILI9163_SetVGPIOValue(CSX,1);
		IODelay(1);		
	  //--------------------------------------------------------------//
	  #endif
#endif

     #if (GPIO_mode == 1)
       ILI9163_SetVGPIOValue(CSX,0);
		IODelay(1);
     #endif
	 
		ILI9163_LCD_WRITE_COMMAND(0x11); //Exit Sleep

		IODelay(20);

        ILI9163_LCD_WRITE_COMMAND(0x3A); //Set Color Format
        ILI9163_LCD_WRITE_DATA(0xE6);

		//ILI9163_LCD_WRITE_COMMAND(0x20); //Display inversion off

		//ILI9163_LCD_WRITE_COMMAND(0x36);
        //ILI9163_LCD_WRITE_DATA(0x08);


    #ifdef Method1
        ILI9163_LCD_WRITE_COMMAND(0x36);
        ILI9163_LCD_WRITE_DATA(0xC8);

        ILI9163_LCD_WRITE_COMMAND(0xB1);
		ILI9163_LCD_WRITE_DATA(0x11);
        ILI9163_LCD_WRITE_DATA(0x14);

		ILI9163_LCD_WRITE_COMMAND(0xB3);
		ILI9163_LCD_WRITE_DATA(0x11);
        ILI9163_LCD_WRITE_DATA(0x14);
		
        LCD_SLIDESHOW_ILI9163();
    #endif
       
		ILI9163_LCD_WRITE_COMMAND(0x36);
        ILI9163_LCD_WRITE_DATA(0xC8);

        
        ILI9163_LCD_WRITE_COMMAND(0xB1);
		ILI9163_LCD_WRITE_DATA(0x07);
        ILI9163_LCD_WRITE_DATA(0x20);
          //ILI9163_LCD_WRITE_DATA(0x11);
          //ILI9163_LCD_WRITE_DATA(0x14);



        ILI9163_LCD_WRITE_COMMAND(0x29);

	#if (GPIO_mode == 1)
        ILI9163_SetVGPIOValue(CSX,1);
		IODelay(1);
     #endif


	
	}

}

#ifdef Method3
void ILI9163_WRITE_FRAME_PERIOD()
{
    register GPIO *gpio = (GPIO *) (GPIO_BASE);
	register IDU *idu = (IDU *)(IDU_BASE);
	register CHANNEL *idu_dma = (CHANNEL *) (DMA_IDU_BASE);
	register CHANNEL *osd_dma = (CHANNEL *) (DMA_OSD_BASE);
	

    //ILI9163_SetVGPIOValue(CSX,0);
	//IODelay(1);
	 
	ILI9163_SetVGPIOValue(DCX,1);
	IODelay(1);

    osd_dma->Control |= BIT0;
	idu->IduCtrl0 |= BIT12;
	idu_dma->Control |= BIT0;
	idu->IduCtrl0 |= BIT0;
	
	
	while((idu->RST_Ctrl_Bypass & 0x18000000) != 0x10000000);
	while((idu->RST_Ctrl_Bypass & 0x18000000) == 0x10000000);

	idu->RST_Ctrl_Bypass = 0x00000a00 ;			// IDU reset per frame

	//IODelay(100);
	idu->IduCtrl0 &= ~BIT12;
	idu->IduCtrl0 &= ~BIT0;
	
	 
	ILI9163_SetVGPIOValue(DCX,0);
    IODelay(1);

	ILI9163_SetVGPIOValue(CSX,1);
	IODelay(1);
	
}

void ILI9163_WRITE_FRAME()
{
    register GPIO *gpio = (GPIO *) (GPIO_BASE);
	register IDU *idu = (IDU *)(IDU_BASE);
	register CHANNEL *idu_dma = (CHANNEL *) (DMA_IDU_BASE);
	register CHANNEL *osd_dma = (CHANNEL *) (DMA_OSD_BASE);
	
    //if(PANEL_ID != PANEL_ILI9163C)
		//return;

	//mpDebugPrint("~~~~~~~~~~ILI9163_WRITE_FRAME");

	//mpDebugPrint("DMA_A:0x%x",idu_dma->StartA);
	//mpDebugPrint("DMA_B:0x%x",idu_dma->StartB);
	//mpDebugPrint("idu->IduCtrl0= 0x%x",idu->IduCtrl0);
	//mpDebugPrint("osd_dma->Control= 0x%x",osd_dma->Control);
    IntDisable();
	
	
	idu->IduCtrl0 &= ~BIT12;
	osd_dma->Control &= ~BIT0;
	idu->IduCtrl0 &= ~BIT0;
	idu_dma->Control &= ~BIT0;
	

    //---Change GPIOcfg to set Memory write ---// 
    ILI9163_Switch(0);
	 
    LCD_MemoryWrite_ILI9163();

	ILI9163_SPI_Wait();

    //---Change GPIOcfg to set write frame---// 
	ILI9163_Switch(1);

    ILI9163_WRITE_FRAME_PERIOD();

    //---Change GPIOcfg to set Display on ---// 
	 ILI9163_Switch(0);

     //LCD_DisplayOn_ILI9163();

	 IntEnable();

	
}

void ILI9163_test(DWORD *ImgData)
{
    register GPIO *gpio = (GPIO *) (GPIO_BASE);
	register IDU *idu = (IDU *)(IDU_BASE);
	register CHANNEL *idu_dma = (CHANNEL *) (DMA_IDU_BASE);

	 idu->IduCtrl0 &= ~BIT0;
	 idu_dma->Control &= ~BIT0;

	 // -- Set  Memory write-- //
	 ILI9163_SetVGPIOValue(CSX,0);
		IODelay(1);

	 ILI9163_LCD_WRITE_COMMAND(0x2C); // Memory write

	 ILI9163_SetVGPIOValue(CSX,1);
		IODelay(1);

	 ILI9163_SPI_Wait();

	 //PaintFrame(ImgData);

     // -- Start to write frame -- //
	 gpio->Vgpcfg0 = 0x000000FF;	//gpio 0~7 select IDU function 1
	 gpio->Vgpcfg1 = 0x02000200;	//gpio 22 26 select IDU function 0

     ILI9163_SetVGPIOValue(CSX,0);
		IODelay(1);
	 
	 ILI9163_SetVGPIOValue(DCX,1);
		  IODelay(1);

	 idu_dma->Control |= BIT0;
	 idu->IduCtrl0 |= BIT0;

	 while((idu->RST_Ctrl_Bypass & 0x18000000) != 0x10000000);
	 while((idu->RST_Ctrl_Bypass & 0x18000000) == 0x10000000);
	 idu->IduCtrl0 &= ~BIT0;	 
	 

	 ILI9163_SetVGPIOValue(DCX,0);
		  IODelay(1);

	 ILI9163_SetVGPIOValue(CSX,1);
		IODelay(1);

    //---Change GPIOcfg to set Display on ---// 

	gpio->Vgpcfg0 = 0xFF00FF00 ;	//gpio 0~7 select IDU function 0
	gpio->Vgpcfg1 = 0x00000000 ;	//gpio 22 25 26 select IDU function 0

    ILI9163_SetVGPIOValue(CSX,0);
		IODelay(1);

    ILI9163_LCD_WRITE_COMMAND(0x29); // Display On

	ILI9163_SetVGPIOValue(CSX,1);
		IODelay(1);
	
    //__asm("break 100");
}

#endif