/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"
#include "sensor.h"
#include "peripheral.h"



#if 0//((SENSOR_ENABLE == ENABLE) && defined(SENSOR_TYPE_CVBS_INPUT))
/*set video decode tvp5150*/

extern int SensorInputWidth;
extern int SensorInputHeight;


#if 1 //use SW i2c

#define IIC_FLAG_NO_ACK		1
#define IIC_FLAG_ACK		2
#define IIC_FLAG_START		3
#define IIC_FLAG_STOP		4
#define IIC_FLAG_IGNORE		5

#define WAIT_TIME_IIC_DELAY	5//10//35  //20 //5
#define WAIT_TIME_WAIT_ACK	70//200  //160 //40

//--------------------------------------------------------
BYTE ACK1=1, ACK2=1, ACK3=1;

//////////////////////////////////////////
// software simulate i2c base
//////////////////////////////////////////
#define TVP5150_DEVICE_ADDRESS   0xBA//0xBA//0xb8//0xBA//

#define TVP5150_DEVICE_ADDRESS_READ (0xBB)//0xbb//0xb9//0xBB

#define __tvp5150_i2c_data_high()	(g_psGpio->Gpdat0 |= 0x00000002)
#define __tvp5150_i2c_data_low()		(g_psGpio->Gpdat0 &= 0xfffffffd)
#define __tvp5150_i2c_data_output()	(g_psGpio->Gpdat0 |= 0x00020000)
#define __tvp5150_i2c_data_input()	(g_psGpio->Gpdat0 &= 0xfffdffff)

#define __tvp5150_i2c_clk_high()		(g_psGpio->Gpdat0 |= 0x00000001)
#define __tvp5150_i2c_clk_low()		(g_psGpio->Gpdat0 &= 0xfffffffe)
#define __tvp5150_i2c_clk_output()	(g_psGpio->Gpdat0 |= 0x00010000)
#define __tvp5150_i2c_clk_input()	(g_psGpio->Gpdat0 &= 0xfffeffff)


#define __tvp5150_i2c_read_data()   ((g_psGpio->Gpdat0 & 0x00000002)>>1)
#define __tvp5150_i2c_all_output()	(g_psGpio->Gpdat0 |= 0x00030000)
#define __tvp5150_i2c_set_gpio_mode() (g_psGpio->Gpcfg0 = (g_psGpio->Gpcfg0 & 0xfffcfffc) )


static void tvp5150_i2c_data_high(void)
{
	__tvp5150_i2c_data_high();		IODelay(WAIT_TIME_IIC_DELAY);
}
static void tvp5150_i2c_data_low(void)
{
	__tvp5150_i2c_data_low();		IODelay(WAIT_TIME_IIC_DELAY);
}
static void tvp5150_i2c_data_output(void)
{
	__tvp5150_i2c_data_output();		IODelay(WAIT_TIME_IIC_DELAY);
}
static void tvp5150_i2c_data_input(void)
{
	__tvp5150_i2c_data_input();		IODelay(WAIT_TIME_IIC_DELAY);
}
static void tvp5150_i2c_clk_high(void)
{
	__tvp5150_i2c_clk_high();		IODelay(WAIT_TIME_IIC_DELAY);
}
static void tvp5150_i2c_clk_low(void)
{
	__tvp5150_i2c_clk_low();			IODelay(WAIT_TIME_IIC_DELAY);
}
static void tvp5150_i2c_clk_output(void)
{
	__tvp5150_i2c_clk_output();		IODelay(WAIT_TIME_IIC_DELAY);
}
static void tvp5150_i2c_clk_input(void)
{
	__tvp5150_i2c_clk_input();		IODelay(WAIT_TIME_IIC_DELAY);
}


static BYTE SW_TVP5150_IIC_SetValue(BYTE data1, BYTE condition)
{
    BYTE mask;
    BYTE ret;
	
    __tvp5150_i2c_set_gpio_mode();	// set to GPIO function mode
    __tvp5150_i2c_all_output();		// GP0, GP1 output

	if(condition == IIC_FLAG_START)
	{
		//start condition
		tvp5150_i2c_clk_high();
		tvp5150_i2c_data_high();
		tvp5150_i2c_data_low();
		tvp5150_i2c_clk_low();
	}
	tvp5150_i2c_clk_low();
	mask = 0x80;
	while(mask > 0)
	{
		if((mask & data1) == mask)
			tvp5150_i2c_data_high();
		else 
			tvp5150_i2c_data_low();
		tvp5150_i2c_clk_high();
		tvp5150_i2c_clk_low();
		mask >>= 1;
	}
	
	//cs4349_i2c_data_high();
	__tvp5150_i2c_data_input();		// data = input direction
	IODelay(WAIT_TIME_WAIT_ACK);   //grad
	tvp5150_i2c_clk_high();
	//IODelay(40);  //grad mark
	IODelay(10);
	if(__tvp5150_i2c_read_data())
	{
		char tmpbuf[64];
		UartOutText("I2C ACK error,");
		sprintf(tmpbuf,"data=0x%02x,condition=%d \r\n",data1,condition);
		UartOutText(tmpbuf);
		//__asm("break 100");
		ret = IIC_FLAG_NO_ACK;
	}
	else
	{
		//UartOutText("A");
		__tvp5150_i2c_all_output();		// GP0, GP1 output
		ret = IIC_FLAG_ACK;
		//__cs4349_i2c_all_output();	// GP0, GP1 output
    }
        
    tvp5150_i2c_clk_low();
	tvp5150_i2c_data_low();   //along add 20100325 avoid data is high due to not stop
	IODelay(WAIT_TIME_WAIT_ACK);   //grad add

    if(condition == IIC_FLAG_STOP){
        tvp5150_i2c_clk_high();
        tvp5150_i2c_data_high();
    }

    return ret; //IIC_ACK;
}


static BYTE SW_CS4349_IIC_GetValue_NoStop()		// get value, but no stop bit
{
    BYTE data, i;
    
    __tvp5150_i2c_set_gpio_mode();	//set to GPIO function mode
    __tvp5150_i2c_data_input();		// DATA = input
    __tvp5150_i2c_clk_output();		// CLK = output

	tvp5150_i2c_clk_low();
    data = 0;
    for(i = 0; i < 8; i++){
        data <<= 1;
        //cs4349_i2c_clk_low();
        tvp5150_i2c_clk_high();
        data |= __tvp5150_i2c_read_data();
		tvp5150_i2c_clk_low();
    }
    // ACK
    tvp5150_i2c_clk_low();
	IODelay(WAIT_TIME_WAIT_ACK);
    __tvp5150_i2c_all_output();		// GP0, GP1 output
    //cs4349_i2c_data_low(); // ACK    
    tvp5150_i2c_data_low(); // ACK    grad
    tvp5150_i2c_clk_high();
	tvp5150_i2c_clk_low();
	IODelay(WAIT_TIME_WAIT_ACK);
    return data;
}



static BYTE SW_TVP5150_IIC_GetValue()
{
    BYTE data, i;
    
    __tvp5150_i2c_set_gpio_mode();	//set to GPIO function mode
    __tvp5150_i2c_data_input();  // DATA = input
    __tvp5150_i2c_clk_output();  // CLK = output

    tvp5150_i2c_clk_low();
    data = 0;
    for(i = 0; i < 8; i++){
        data <<= 1;        
       // cs4349_i2c_clk_low();
        tvp5150_i2c_clk_high();
        data |= __tvp5150_i2c_read_data();
        tvp5150_i2c_clk_low();		
    }
    // ACK
    tvp5150_i2c_clk_low();
	IODelay(WAIT_TIME_WAIT_ACK);
    __tvp5150_i2c_all_output();  // GP0, GP1 output
    //cs4349_i2c_data_low(); // ACK    

//frank mask    cs4349_i2c_data_high(); // ACK    grad
//frank mask     cs4349_i2c_clk_high();
	
    //__cs4349_i2c_data_input();  // GP1 = input
    
    
    tvp5150_i2c_clk_low();
	tvp5150_i2c_data_low();   //along add 20100325 avoid data is high due to not stop
	IODelay(WAIT_TIME_WAIT_ACK);
    tvp5150_i2c_clk_high();
    tvp5150_i2c_data_high();
    
    return data;
}


static BYTE TVP5150_setRegister(BYTE addr, BYTE data)
{
	ACK1 = SW_TVP5150_IIC_SetValue(TVP5150_DEVICE_ADDRESS, IIC_FLAG_START); 
	ACK2 = SW_TVP5150_IIC_SetValue(addr, IIC_FLAG_IGNORE);
	ACK3 = SW_TVP5150_IIC_SetValue(data, IIC_FLAG_STOP);
}
    

static BYTE TVP5150_readRegister(BYTE addr)
{
	BYTE value;
	ACK1 = SW_TVP5150_IIC_SetValue(TVP5150_DEVICE_ADDRESS, IIC_FLAG_START);
	ACK2 = SW_TVP5150_IIC_SetValue(addr, IIC_FLAG_STOP);
	//__cs4349_i2c_data_high();
	ACK3 = SW_TVP5150_IIC_SetValue((TVP5150_DEVICE_ADDRESS_READ) | 0x01, IIC_FLAG_START);
	value = SW_TVP5150_IIC_GetValue();
	return value;
}


#endif

void Set_CVBS_Input_resolution(void)
{
  SensorInputWidth  = CVBS_getInputWidth();
  SensorInputHeight = CVBS_getInputHeight()<<1 ;
}

void TVP5150_setting(void)
{
  Gpio_Config2GpioFunc(GPIO_GPIO_0, GPIO_OUTPUT_MODE, GPIO_DATA_HIGH, 1);
  Gpio_Config2GpioFunc(GPIO_GPIO_1, GPIO_OUTPUT_MODE, GPIO_DATA_HIGH, 1);



  //H sync set FGPIO[29] to function 2
  g_psGpio->Fgpcfg[1] &= (~(BIT29|BIT13));
  g_psGpio->Fgpcfg[1] |=(BIT29);

  //V sync set FGPIO[32] to function 2
  g_psGpio->Fgpcfg[2] &= (~(BIT16|BIT0));
  g_psGpio->Fgpcfg[2] |=(BIT16);

  //set Pclk to function 2
  g_psGpio->Fgpcfg[0] &= (~(BIT30|BIT14));
  g_psGpio->Fgpcfg[0] |=(BIT30);

  //set Y0
  g_psGpio->Fgpcfg[0] &= (~(BIT31|BIT15));
  g_psGpio->Fgpcfg[0] |=(BIT31);

  //set Y1-5
  g_psGpio->Fgpcfg[1] &= (~(0x061f061f));
  g_psGpio->Fgpcfg[1] |=(0x061f0000);


  MP_ALERT("g_psGpio->Fgpcfg[0] =0x%x",g_psGpio->Fgpcfg[0]);
  MP_ALERT("g_psGpio->Fgpcfg[1] =0x%x",g_psGpio->Fgpcfg[1]);
  MP_ALERT("g_psGpio->Fgpcfg[2] =0x%x",g_psGpio->Fgpcfg[2]);
  MP_ALERT("g_psGpio->Fgpcfg[3] =0x%x",g_psGpio->Fgpcfg[3]);


  Gpio_Config2GpioFunc(GPIO_FGPIO_27, GPIO_OUTPUT_MODE, GPIO_DATA_HIGH, 1);
  MP_ALERT("#### SET FGPIO27 High ####");
  //IODelay(1000);

  TVP5150_setRegister( 0x00, 0x02);//default 0x02
  TVP5150_setRegister( 0x03, 0x6f);//default: 0x6d
  TVP5150_setRegister( 0x06, 0x00); 		// color killer
  TVP5150_setRegister( 0x08, 0x4C);
  TVP5150_setRegister( 0x09, 96);			// brightness
  TVP5150_setRegister( 0x0A, 128 + 50); 	// saturation
  TVP5150_setRegister( 0x0C, 120);			// contrast
  TVP5150_setRegister( 0x0d, 0);
  TVP5150_setRegister( 0x0E, 3);
  TVP5150_setRegister( 0x0f, 0x02);

  //frank add  fine tuning       
  TVP5150_setRegister( 0x16, 0x60);//H sync postion
            			
		
#if 1 //debug
  MP_ALERT("TVP5150 read[0x00] =0x%x ###", TVP5150_readRegister(0x00));

  MP_ALERT("TVP5150 read[0x03] =0x%x ###", TVP5150_readRegister(0x03));
  MP_ALERT("TVP5150 read[0x06] =0x%x ###", TVP5150_readRegister(0x06));
  MP_ALERT("TVP5150 read[0x08] =0x%x ###", TVP5150_readRegister(0x08));
  MP_ALERT("TVP5150 read[0x09] =0x%x ###", TVP5150_readRegister(0x09));
  MP_ALERT("TVP5150 read[0x0a] =0x%x ###", TVP5150_readRegister(0x0a));
  MP_ALERT("TVP5150 read[0x0c] =0x%x ###", TVP5150_readRegister(0x0c));
  MP_ALERT("TVP5150 read[0x0d] =0x%x ###", TVP5150_readRegister(0x0d));
  MP_ALERT("TVP5150 read[0x0e] =0x%x ###", TVP5150_readRegister(0x0e));
  MP_ALERT("TVP5150 read[0x0f] =0x%x ###", TVP5150_readRegister(0x0f));

  MP_ALERT("TVP5150 read[0x11] =0x%x ###", TVP5150_readRegister(0x11));
  MP_ALERT("TVP5150 read[0x12] =0x%x ###", TVP5150_readRegister(0x12));
  MP_ALERT("TVP5150 read[0x13] =0x%x ###", TVP5150_readRegister(0x13));
  MP_ALERT("TVP5150 read[0x14] =0x%x ###", TVP5150_readRegister(0x14));
  MP_ALERT("TVP5150 read[0x16] =0x%x ###", TVP5150_readRegister(0x16));
  MP_ALERT("TVP5150 read[0x18] =0x%x ###", TVP5150_readRegister(0x18));
  MP_ALERT("TVP5150 read[0x19] =0x%x ###", TVP5150_readRegister(0x19));
  MP_ALERT("TVP5150 read[0x84] =0x%x ###", TVP5150_readRegister(0x84));
  MP_ALERT("TVP5150 read[0x85] =0x%x ###", TVP5150_readRegister(0x85));
#endif
}


#endif



