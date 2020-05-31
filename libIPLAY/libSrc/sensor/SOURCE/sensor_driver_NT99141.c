/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 1

#include "global612.h"
#include "mpTrace.h"
#include "sensor.h"
#include "ui.h"
#include "TaskId.h"
//#include "..\..\MiniDV\main\include\camcorder_func.h"

#if ((SENSOR_ENABLE == ENABLE) && defined(SENSOR_TYPE_NT99141))
#if 1       // Software I2C

#define SENSOR_NT99140_DEVICE_ADDRESS   (0x54)
#define IIC_FLAG_NO_ACK     1
#define IIC_FLAG_ACK        2
#define IIC_FLAG_START      3
#define IIC_FLAG_STOP       4
#define IIC_FLAG_IGNORE     5

#define WAIT_TIME_IIC_DELAY 10
#define WAIT_TIME_WAIT_ACK  50

//--------------------------------------------------------
//static BYTE ACK1=1, ACK2=1, ACK3=1, ACK4=1;

//////////////////////////////////////////
// software simulate i2c base
//////////////////////////////////////////
#if PRODUCT_PSN611
#define __sensor_nt99140_i2c_data_high()        (g_psGpio->Gpdat0 |= 0x00000002)
#define __sensor_nt99140_i2c_data_low()         (g_psGpio->Gpdat0 &= 0xfffffffd)
#define __sensor_nt99140_i2c_data_output()      (g_psGpio->Gpdat0 |= 0x00020000)
#define __sensor_nt99140_i2c_data_input()       (g_psGpio->Gpdat0 &= 0xfffdffff)

#define __sensor_nt99140_i2c_clk_high()         (g_psGpio->Gpdat0 |= 0x00000001)
#define __sensor_nt99140_i2c_clk_low()          (g_psGpio->Gpdat0 &= 0xfffffffe)
#define __sensor_nt99140_i2c_clk_output()       (g_psGpio->Gpdat0 |= 0x00010000)
#define __sensor_nt99140_i2c_clk_input()        (g_psGpio->Gpdat0 &= 0xfffeffff)

#define __sensor_nt99140_i2c_read_data()        ((g_psGpio->Gpdat0 & 0x00000002)>>1)
#define __sensor_nt99140_i2c_all_output()       (g_psGpio->Gpdat0 |= 0x00030000)
#define __sensor_nt99140_i2c_set_gpio_mode()    (g_psGpio->Gpcfg0 = (g_psGpio->Gpcfg0 & 0xfffcfffc))
#elif (PRODUCT_PCBA==PCBA_MAIN_BOARD_V10)
//PGPIO 0  --SDA1
#define __sensor_nt99140_i2c_data_high()        (g_psGpio->Pgpdat |= 0x00000001)//(g_psGpio->Gpdat0 |= 0x00000002)
#define __sensor_nt99140_i2c_data_low()         (g_psGpio->Pgpdat &= 0xfffffffe)
#define __sensor_nt99140_i2c_data_output()      (g_psGpio->Pgpdat |= 0x00010000)
#define __sensor_nt99140_i2c_data_input()       (g_psGpio->Pgpdat &= 0xfffeffff)
#define __sensor_nt99140_i2c_read_data()        (g_psGpio->Pgpdat & 0x00000001)
//FGPIO 24
#define __sensor_nt99140_i2c_clk_high()         (g_psGpio->Fgpdat[1] |= 0x00000100)
#define __sensor_nt99140_i2c_clk_low()          (g_psGpio->Fgpdat[1] &= 0xfffffeff)
#define __sensor_nt99140_i2c_clk_output()       (g_psGpio->Fgpdat[1] |= 0x01000000)
#define __sensor_nt99140_i2c_clk_input()        (g_psGpio->Fgpdat[1] &= 0xfeffffff)

static  void __sensor_nt99140_i2c_all_output()       //(g_psGpio->Gpdat0 |= 0x00030000)
{
	__sensor_nt99140_i2c_data_output();
	__sensor_nt99140_i2c_clk_output();
}
static  void __sensor_nt99140_i2c_set_gpio_mode()    //(g_psGpio->Gpcfg0 = (g_psGpio->Gpcfg0 & 0xfffcfffc))
{
//PGPIO 0  --SDA1
		g_psGpio->Pgpcfg &=  0xfffeffff;
		g_psGpio->Pgpcfg |=  0x00000001; // PGPIO0-3.alt.func 1 is GPIO
//PGPIO 2  --SDA2
		g_psGpio->Pgpcfg &=  0xfffbffff;
		g_psGpio->Pgpcfg |=  0x00000004;
//FGPIO 24
		g_psGpio->Fgpcfg[1] &=  0xfefffeff;
}

#elif (PRODUCT_PCBA==PCBA_MAIN_BOARD_V11)

//FGPIO 24  --SDA1  FGPIO 42  --SDA2
void __sensor_nt99140_i2c_data_high()
{
	if (Sensor_CurChannel_Get())
	{
		g_psGpio->Fgpdat[2] |= 0x00000400; // BIT10
	}
	else
	{
		g_psGpio->Fgpdat[1] |= 0x00000100;//BIT8
	}
}
void __sensor_nt99140_i2c_data_low()
{
	if (Sensor_CurChannel_Get())
	{
		g_psGpio->Fgpdat[2] &= 0xfffffbff;
	}
	else
	{
		g_psGpio->Fgpdat[1] &= 0xfffffeff;
	}
}
void __sensor_nt99140_i2c_data_output()
{
	if (Sensor_CurChannel_Get())
	{
		g_psGpio->Fgpdat[2] |= 0x04000000;
	}
	else
	{
		g_psGpio->Fgpdat[1] |= 0x01000000;
	}
}
void __sensor_nt99140_i2c_data_input()
{
	if (Sensor_CurChannel_Get())
	{
		g_psGpio->Fgpdat[2] &= 0xfbffffff;
	}
	else
	{
		g_psGpio->Fgpdat[1] &= 0xfeffffff;
	}
}
DWORD __sensor_nt99140_i2c_read_data()
{
	if (Sensor_CurChannel_Get())
	{
		return (g_psGpio->Fgpdat[2] & 0x00000400);
	}
	else
	{
		return (g_psGpio->Fgpdat[1] & 0x00000100);
	}
}

//FGPIO 27
#define __sensor_nt99140_i2c_clk_high()         (g_psGpio->Fgpdat[1] |= 0x00000800)
#define __sensor_nt99140_i2c_clk_low()          (g_psGpio->Fgpdat[1] &= 0xfffff7ff)
#define __sensor_nt99140_i2c_clk_output()       (g_psGpio->Fgpdat[1] |= 0x08000000)
#define __sensor_nt99140_i2c_clk_input()        (g_psGpio->Fgpdat[1] &= 0xf7ffffff)

static  void __sensor_nt99140_i2c_all_output()       //(g_psGpio->Gpdat0 |= 0x00030000)
{
	__sensor_nt99140_i2c_data_output();
	__sensor_nt99140_i2c_clk_output();
}
static  void __sensor_nt99140_i2c_set_gpio_mode()    //(g_psGpio->Gpcfg0 = (g_psGpio->Gpcfg0 & 0xfffcfffc))
{
//FGPIO 24  --SDA1
		g_psGpio->Fgpcfg[1] &=  0xfefffeff;
//FGPIO 42  --SDA2
		g_psGpio->Fgpcfg[2] &=  0xfbfffbff;
//FGPIO 27
		g_psGpio->Fgpcfg[1] &=  0xf7fff7ff;
}

#elif (PRODUCT_PCBA==PCBA_MAIN_BOARD_V12||PRODUCT_PCBA==PCBA_MAIN_BOARD_V20)

//FGPIO 24  --SDA1  FGPIO 42  --SDA2
void __sensor_nt99140_i2c_data_high()
{
	if (Sensor_CurChannel_Get())
	{
		g_psGpio->Fgpdat[2] |= 0x00000400; // BIT10
	}
	else
	{
		g_psGpio->Fgpdat[1] |= 0x00000100;//BIT8
	}
}
void __sensor_nt99140_i2c_data_low()
{
	if (Sensor_CurChannel_Get())
	{
		g_psGpio->Fgpdat[2] &= 0xfffffbff;
	}
	else
	{
		g_psGpio->Fgpdat[1] &= 0xfffffeff;
	}
}
void __sensor_nt99140_i2c_data_output()
{
	if (Sensor_CurChannel_Get())
	{
		g_psGpio->Fgpdat[2] |= 0x04000000;
	}
	else
	{
		g_psGpio->Fgpdat[1] |= 0x01000000;
	}
}
void __sensor_nt99140_i2c_data_input()
{
	if (Sensor_CurChannel_Get())
	{
		g_psGpio->Fgpdat[2] &= 0xfbffffff;
	}
	else
	{
		g_psGpio->Fgpdat[1] &= 0xfeffffff;
	}
}
DWORD __sensor_nt99140_i2c_read_data()
{
	if (Sensor_CurChannel_Get())
	{
		return (g_psGpio->Fgpdat[2] & 0x00000400);
	}
	else
	{
		return (g_psGpio->Fgpdat[1] & 0x00000100);
	}
}

//FGPIO 27--SCL1     PGPIO 0  --SCL2
void __sensor_nt99140_i2c_clk_high()
{
	if (Sensor_CurChannel_Get())
		g_psGpio->Pgpdat |= 0x00000001;
	else
		g_psGpio->Fgpdat[1] |= 0x00000800;
}
void __sensor_nt99140_i2c_clk_low()
{
	if (Sensor_CurChannel_Get())
		g_psGpio->Pgpdat &= 0xfffffffe;
	else
		g_psGpio->Fgpdat[1] &= 0xfffff7ff;
}
void __sensor_nt99140_i2c_clk_output()
{
	if (Sensor_CurChannel_Get())
		g_psGpio->Pgpdat |= 0x00010000;
	else
		g_psGpio->Fgpdat[1] |= 0x08000000;
}
void __sensor_nt99140_i2c_clk_input()
{
	if (Sensor_CurChannel_Get())
		g_psGpio->Pgpdat &= 0xfffeffff;
	else
		g_psGpio->Fgpdat[1] &= 0xf7ffffff;
}

static  void __sensor_nt99140_i2c_all_output()       //(g_psGpio->Gpdat0 |= 0x00030000)
{
	__sensor_nt99140_i2c_data_output();
	__sensor_nt99140_i2c_clk_output();
}
static  void __sensor_nt99140_i2c_set_gpio_mode()    //(g_psGpio->Gpcfg0 = (g_psGpio->Gpcfg0 & 0xfffcfffc))
{
//FGPIO 24  --SDA1
		g_psGpio->Fgpcfg[1] &=  0xfefffeff;
//FGPIO 42  --SDA2
		g_psGpio->Fgpcfg[2] &=  0xfbfffbff;
//FGPIO 27 --SCL1
		g_psGpio->Fgpcfg[1] &=  0xf7fff7ff;
//PGPIO 0  --SCL2
		g_psGpio->Pgpcfg &=  0xfffeffff;
		g_psGpio->Pgpcfg |=  0x00000001; // PGPIO0-3.alt.func 1 is GPIO
}

	void __sensor_nt99140_i2c_all_input()
	{
	__sensor_nt99140_i2c_data_input();
	__sensor_nt99140_i2c_clk_input();
	}

#endif

static void sensor_nt99140_i2c_data_high(void)
{
    __sensor_nt99140_i2c_data_high();       IODelay(WAIT_TIME_IIC_DELAY>>1);
}
static void sensor_nt99140_i2c_data_low(void)
{
    __sensor_nt99140_i2c_data_low();        IODelay(WAIT_TIME_IIC_DELAY>>1);
}
static void sensor_nt99140_i2c_data_output(void)
{
    __sensor_nt99140_i2c_data_output();     IODelay(WAIT_TIME_IIC_DELAY);
}
static void sensor_nt99140_i2c_data_input(void)
{
    __sensor_nt99140_i2c_data_input();      IODelay(WAIT_TIME_IIC_DELAY);
}
static void sensor_nt99140_i2c_clk_high(void)
{
    __sensor_nt99140_i2c_clk_high();        IODelay(WAIT_TIME_IIC_DELAY);
}
static void sensor_nt99140_i2c_clk_low(void)
{
    __sensor_nt99140_i2c_clk_low();         IODelay(WAIT_TIME_IIC_DELAY);
}
static void sensor_nt99140_i2c_clk_output(void)
{
    __sensor_nt99140_i2c_clk_output();      IODelay(WAIT_TIME_IIC_DELAY);
}
static void sensor_nt99140_i2c_clk_input(void)
{
    __sensor_nt99140_i2c_clk_input();       IODelay(WAIT_TIME_IIC_DELAY);
}

static void sensor_nt99140_i2c_wait_data_Ack(DWORD dwDelay)
{

	while (dwDelay--)
	{
		if (!__sensor_nt99140_i2c_read_data())
			break;
	}
}

void sensor_nt99140_GPIO_Init()
{
    __sensor_nt99140_i2c_set_gpio_mode();
    __sensor_nt99140_i2c_all_output();
	__sensor_nt99140_i2c_clk_high() ;
	__sensor_nt99140_i2c_data_high();

}

static BYTE SW_sensor_nt99140_IIC_SetValue(BYTE data1, BYTE condition)
{
    BYTE mask;
    BYTE ret;
    
    __sensor_nt99140_i2c_set_gpio_mode();   // set to GPIO function mode
    __sensor_nt99140_i2c_all_output();      // GP0, GP1 output

    if(condition == IIC_FLAG_START)
    {
        sensor_nt99140_i2c_clk_high();
        sensor_nt99140_i2c_data_high();
        sensor_nt99140_i2c_data_low();
        sensor_nt99140_i2c_clk_low();
    }
    sensor_nt99140_i2c_clk_low();
    mask = 0x80;
    while(mask > 0)
    {
        if (mask & data1)
            sensor_nt99140_i2c_data_high();
        else 
            sensor_nt99140_i2c_data_low();
        sensor_nt99140_i2c_clk_high();
        sensor_nt99140_i2c_clk_low();
        mask >>= 1;
    }
    
    //sensor_nt99140_i2c_data_high();
    __sensor_nt99140_i2c_data_input();      // data = input direction
    sensor_nt99140_i2c_wait_data_Ack(WAIT_TIME_WAIT_ACK);   //
    sensor_nt99140_i2c_clk_high();
    if(__sensor_nt99140_i2c_read_data())
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
        __sensor_nt99140_i2c_all_output();      // GP0, GP1 output
        ret = IIC_FLAG_ACK;
        //__sensor_nt99140_i2c_all_output();    // GP0, GP1 output
    }
        
    sensor_nt99140_i2c_clk_low();
    sensor_nt99140_i2c_data_low();
   // IODelay(WAIT_TIME_WAIT_ACK);   //

    if(condition == IIC_FLAG_STOP){
        sensor_nt99140_i2c_clk_high();
        sensor_nt99140_i2c_data_high();
    IODelay(WAIT_TIME_WAIT_ACK);   //
    }

    return ret; //IIC_ACK;
}

static BYTE SW_sensor_nt99140_IIC_GetValue_NoStop()     // get value, but no stop bit
{
    BYTE data, i;
    
    __sensor_nt99140_i2c_set_gpio_mode();   //set to GPIO function mode
    __sensor_nt99140_i2c_data_input();      // DATA = input
    __sensor_nt99140_i2c_clk_output();      // CLK = output

    sensor_nt99140_i2c_clk_low();
    data = 0;
    for(i = 0; i < 8; i++){
        data <<= 1;
        //sensor_nt99140_i2c_clk_low();
        sensor_nt99140_i2c_clk_high();
		if ( __sensor_nt99140_i2c_read_data())
        	data |= 0x01;
        sensor_nt99140_i2c_clk_low();
    }
    // ACK
    sensor_nt99140_i2c_clk_low();
    IODelay(WAIT_TIME_WAIT_ACK);
    __sensor_nt99140_i2c_all_output();      // GP0, GP1 output
    //sensor_nt99140_i2c_data_low(); // ACK    
    sensor_nt99140_i2c_data_low(); // ACK    
    sensor_nt99140_i2c_clk_high();
    sensor_nt99140_i2c_clk_low();
    IODelay(WAIT_TIME_WAIT_ACK);
    return data;
}

static BYTE SW_sensor_nt99140_IIC_GetValue()
{
    BYTE data, i;
    
    __sensor_nt99140_i2c_set_gpio_mode();   //set to GPIO function mode
    __sensor_nt99140_i2c_data_input();  // DATA = input
    __sensor_nt99140_i2c_clk_output();  // CLK = output

    sensor_nt99140_i2c_clk_low();
    data = 0;
    for(i = 0; i < 8; i++){
        data <<= 1;        
       // sensor_nt99140_i2c_clk_low();
        sensor_nt99140_i2c_clk_high();
		if ( __sensor_nt99140_i2c_read_data())
        	data |= 0x01;
        sensor_nt99140_i2c_clk_low();       
    }
    // ACK
    sensor_nt99140_i2c_clk_low();
    IODelay(WAIT_TIME_WAIT_ACK);
    __sensor_nt99140_i2c_all_output();  // GP0, GP1 output
    //sensor_nt99140_i2c_data_low(); // ACK    
    sensor_nt99140_i2c_data_high(); // ACK    
    sensor_nt99140_i2c_clk_high();
    
    //__sensor_nt99140_i2c_data_input();  // GP1 = input
    

    // end
    sensor_nt99140_i2c_clk_low();
    sensor_nt99140_i2c_data_low();
    IODelay(WAIT_TIME_WAIT_ACK);
    sensor_nt99140_i2c_clk_high();
    sensor_nt99140_i2c_data_high();
    
    return data;
}

static BYTE sensor_nt99140_setRegister(WORD addr, BYTE data)
{
    BYTE ACK1,ACK2,ACK3,ACK4;

    //SemaphoreWait(SW_I2C_SEMA_ID);
    ACK1 = SW_sensor_nt99140_IIC_SetValue(SENSOR_NT99140_DEVICE_ADDRESS, IIC_FLAG_START); 
    ACK2 = SW_sensor_nt99140_IIC_SetValue((addr>>8)&0xff, IIC_FLAG_IGNORE);
    ACK3 = SW_sensor_nt99140_IIC_SetValue(addr&0xff, IIC_FLAG_IGNORE);
    ACK4 = SW_sensor_nt99140_IIC_SetValue(data, IIC_FLAG_STOP);
    //SemaphoreRelease(SW_I2C_SEMA_ID);
}

static BYTE sensor_nt99140_readRegister(WORD addr)
{
    BYTE value= 0,ACK1,ACK2,ACK3,ACK4;
    //SemaphoreWait(SW_I2C_SEMA_ID);
    ACK1 = SW_sensor_nt99140_IIC_SetValue(SENSOR_NT99140_DEVICE_ADDRESS, IIC_FLAG_START);
    ACK2 = SW_sensor_nt99140_IIC_SetValue((addr>>8)&0xff, IIC_FLAG_IGNORE);
    ACK3 = SW_sensor_nt99140_IIC_SetValue(addr&0xff, IIC_FLAG_STOP);
  //  __sensor_nt99140_i2c_data_high();
    ACK4 = SW_sensor_nt99140_IIC_SetValue(SENSOR_NT99140_DEVICE_ADDRESS | 0x01, IIC_FLAG_START);
    value = SW_sensor_nt99140_IIC_GetValue();
    //MP_DEBUG("ACK %p %p %p %p",ACK1,ACK2,ACK3,value);
    //SemaphoreRelease(SW_I2C_SEMA_ID);
    return value;
}

#endif

SWORD sensor_NT99140_CheckID(void)
{
	WORD wID;
	BYTE bChanel=Sensor_CurChannel_Get();
    CLOCK *clock = (CLOCK *)CLOCK_BASE;

	//Global_Sensor_Initial_NT99140();
    Local_HW_MCLK_Set();
    Sensor_Pin_Set();
    clock->Clkss2 &=~(BIT15|BIT16|BIT17|BIT18);
    clock->MdClken |= 0x00000c00 ;  //enable clock to sensor and clock for sensor_in
	Sensor_ChangeIO_Init();
	Sensor_Channel_Set(0); //->g_bDisplayMode=0x81; down sensor near to panel connect
	Local_Sensor_GPIO_Reset();
	wID=sensor_nt99140_readRegister(0x3000);
	wID<<=8;
	wID|=sensor_nt99140_readRegister(0x3001);
    //MP_DEBUG("sensor_NT99140_CheckID %d:%p",Sensor_CurChannel_Get(),wID);
	wID>>=4;
	if (wID !=0x141)
		return FAIL;
	Sensor_Channel_Set(1); //->g_bDisplayMode=0x81; down sensor near to panel connect
	IODelay(100);
	Local_Sensor_GPIO_Reset();
	//Drive_Sensor_NT99140();
	wID=sensor_nt99140_readRegister(0x3000);
	wID<<=8;
	wID|=sensor_nt99140_readRegister(0x3001);
    MP_DEBUG("sensor_NT99140_CheckID %d:%p",Sensor_CurChannel_Get(),wID);
	wID>>=4;
	Sensor_Channel_Set(bChanel);
	if (wID !=0x141)
		return FAIL;
	return PASS;
}


extern BYTE sensor_mode;
extern BYTE sensor_IMGSIZE_ID;

static void NT99140_NightMode();

//int boIsNightMode = 0;              // 0: day, 1: overcast, 2: night
#if (PRODUCT_UI==UI_WELDING)
//static BYTE st_bhaveSetted[2] = {0};
#else
//static BYTE st_bhaveSetted=0;
#endif

static void NT99140_Initial_Setting()
{
#if (PRODUCT_UI==UI_WELDING)
	if (Sensor_CurChannel_Get()>1)
			return;
  //  if (st_bhaveSetted[Sensor_CurChannel_Get()])
  //      return;
   // st_bhaveSetted[Sensor_CurChannel_Get()] = TRUE;
#else
//	if (st_bhaveSetted)
//		return;
//	st_bhaveSetted=1;
#endif
    MP_DEBUG("First time call function NT99141_Initial_Setting");
//nt99141 initial
	sensor_nt99140_setRegister(0x3069,0x01);
	sensor_nt99140_setRegister(0x306a,0x05);
	sensor_nt99140_setRegister(0x3109,0x04);
	sensor_nt99140_setRegister(0x3040,0x04);
	sensor_nt99140_setRegister(0x3041,0x02);
	sensor_nt99140_setRegister(0x3042,0xFF);
	sensor_nt99140_setRegister(0x3043,0x08);
	sensor_nt99140_setRegister(0x3052,0xE0);
	sensor_nt99140_setRegister(0x305F,0x33);
	sensor_nt99140_setRegister(0x3100,0x07);
	sensor_nt99140_setRegister(0x3106,0x03);
	sensor_nt99140_setRegister(0x3105,0x01);
	sensor_nt99140_setRegister(0x3108,0x05);
	sensor_nt99140_setRegister(0x3110,0x22);
	sensor_nt99140_setRegister(0x3111,0x57);
	sensor_nt99140_setRegister(0x3112,0x22);
	sensor_nt99140_setRegister(0x3113,0x55);
	sensor_nt99140_setRegister(0x3114,0x05);
	sensor_nt99140_setRegister(0x3135,0x00);
	sensor_nt99140_setRegister(0x32F0,0x01);
	sensor_nt99140_setRegister(0x3290,0x01);
	sensor_nt99140_setRegister(0x3291,0x80);
	sensor_nt99140_setRegister(0x3296,0x01);
	sensor_nt99140_setRegister(0x3297,0x73);
	sensor_nt99140_setRegister(0x3250,0x80);
	sensor_nt99140_setRegister(0x3251,0x03);
	sensor_nt99140_setRegister(0x3252,0xFF);
	sensor_nt99140_setRegister(0x3253,0x00);
	sensor_nt99140_setRegister(0x3254,0x03);
	sensor_nt99140_setRegister(0x3255,0xFF);
	sensor_nt99140_setRegister(0x3256,0x00);
	sensor_nt99140_setRegister(0x3257,0x50);
	sensor_nt99140_setRegister(0x3270,0x00);
	sensor_nt99140_setRegister(0x3271,0x09);
	sensor_nt99140_setRegister(0x3272,0x12);
	sensor_nt99140_setRegister(0x3273,0x23);
	sensor_nt99140_setRegister(0x3274,0x34);
	sensor_nt99140_setRegister(0x3275,0x45);
	sensor_nt99140_setRegister(0x3276,0x64);
	sensor_nt99140_setRegister(0x3277,0x86);
	sensor_nt99140_setRegister(0x3278,0xA4);
	sensor_nt99140_setRegister(0x3279,0xBC);
	sensor_nt99140_setRegister(0x327A,0xDC);
	sensor_nt99140_setRegister(0x327B,0xF0);
	sensor_nt99140_setRegister(0x327C,0xFA);
	sensor_nt99140_setRegister(0x327D,0xFE);
	sensor_nt99140_setRegister(0x327E,0xFF);
	sensor_nt99140_setRegister(0x3302,0x00);
	sensor_nt99140_setRegister(0x3303,0x40);
	sensor_nt99140_setRegister(0x3304,0x00);
	sensor_nt99140_setRegister(0x3305,0x96);
	sensor_nt99140_setRegister(0x3306,0x00);
	sensor_nt99140_setRegister(0x3307,0x29);
	sensor_nt99140_setRegister(0x3308,0x07);
	sensor_nt99140_setRegister(0x3309,0xBA);
	sensor_nt99140_setRegister(0x330A,0x06);
	sensor_nt99140_setRegister(0x330B,0xF5);
	sensor_nt99140_setRegister(0x330C,0x01);
	sensor_nt99140_setRegister(0x330D,0x51);
	sensor_nt99140_setRegister(0x330E,0x01);
	sensor_nt99140_setRegister(0x330F,0x30);
	sensor_nt99140_setRegister(0x3310,0x07);
	sensor_nt99140_setRegister(0x3311,0x16);
	sensor_nt99140_setRegister(0x3312,0x07);
	sensor_nt99140_setRegister(0x3313,0xBA);
	sensor_nt99140_setRegister(0x3326,0x04);
	sensor_nt99140_setRegister(0x3327,0x04);
	sensor_nt99140_setRegister(0x3328,0x04);
	sensor_nt99140_setRegister(0x3329,0x1E);
	sensor_nt99140_setRegister(0x332A,0x08);
	sensor_nt99140_setRegister(0x332B,0x1E);
	sensor_nt99140_setRegister(0x332C,0x08);
	sensor_nt99140_setRegister(0x332D,0x1E);
	sensor_nt99140_setRegister(0x332E,0x1E);
	sensor_nt99140_setRegister(0x332F,0x1E);
    sensor_nt99140_setRegister(0x32F2,0xA0);
	sensor_nt99140_setRegister(0x32FC,0xB0);
	sensor_nt99140_setRegister(0x32F6,0x0F);
	sensor_nt99140_setRegister(0x32F9,0x42);
	sensor_nt99140_setRegister(0x32FA,0x24);
	sensor_nt99140_setRegister(0x3325,0x4A);
	sensor_nt99140_setRegister(0x3330,0x00);
	sensor_nt99140_setRegister(0x3331,0x0A);
	sensor_nt99140_setRegister(0x3332,0xFF);
	sensor_nt99140_setRegister(0x3338,0x30);
	sensor_nt99140_setRegister(0x3339,0x84);
	sensor_nt99140_setRegister(0x333A,0x48);
	sensor_nt99140_setRegister(0x333F,0x07);
	sensor_nt99140_setRegister(0x3360,0x10);
	sensor_nt99140_setRegister(0x3361,0x18);
	sensor_nt99140_setRegister(0x3362,0x1f);
	sensor_nt99140_setRegister(0x3363,0x37);
	sensor_nt99140_setRegister(0x3364,0x80);
	sensor_nt99140_setRegister(0x3365,0x80);
	sensor_nt99140_setRegister(0x3366,0x68);
	sensor_nt99140_setRegister(0x3367,0x60);
	sensor_nt99140_setRegister(0x3368,0x80);
	sensor_nt99140_setRegister(0x3369,0x70);
	sensor_nt99140_setRegister(0x336A,0x60);
	sensor_nt99140_setRegister(0x336B,0x50);
	sensor_nt99140_setRegister(0x336C,0x00);
	sensor_nt99140_setRegister(0x336D,0x20);
	sensor_nt99140_setRegister(0x336E,0x1C);
	sensor_nt99140_setRegister(0x336F,0x18);
	sensor_nt99140_setRegister(0x3370,0x10);
	sensor_nt99140_setRegister(0x3371,0x38);
	sensor_nt99140_setRegister(0x3372,0x3C);
	sensor_nt99140_setRegister(0x3373,0x3F);
	sensor_nt99140_setRegister(0x3374,0x3F);
	sensor_nt99140_setRegister(0x338A,0x34);
	sensor_nt99140_setRegister(0x338B,0x7F);
	sensor_nt99140_setRegister(0x338C,0x10);
	sensor_nt99140_setRegister(0x338D,0x23);
	sensor_nt99140_setRegister(0x338E,0x7F);
	sensor_nt99140_setRegister(0x338F,0x14);
	sensor_nt99140_setRegister(0x3375,0x0A);
	sensor_nt99140_setRegister(0x3376,0x0C);
	sensor_nt99140_setRegister(0x3377,0x10);
	sensor_nt99140_setRegister(0x3378,0x14);
	sensor_nt99140_setRegister(0x3012,0x02);
	sensor_nt99140_setRegister(0x3013,0xD0);
	sensor_nt99140_setRegister(0x32BB, 0x87); 
	sensor_nt99140_setRegister(0x32C4, 0x10); 	
#if 0	
	sensor_nt99140_setRegister(0x32B8, 0x24);     
	sensor_nt99140_setRegister(0x32B9, 0x1C);     
	sensor_nt99140_setRegister(0x32BC, 0x20);     
	sensor_nt99140_setRegister(0x32BD, 0x22);     
	sensor_nt99140_setRegister(0x32BE, 0x1E);   
#else
	sensor_nt99140_setRegister(0x32B8, 0x2D); 
	sensor_nt99140_setRegister(0x32B9, 0x23); 
	sensor_nt99140_setRegister(0x32BC, 0x28); 
	sensor_nt99140_setRegister(0x32BD, 0x2B); 
	sensor_nt99140_setRegister(0x32BE, 0x25); 		
#endif       
        
        
}       
        
static void NT99140_NightMode()
{
   

}


static void NT99140_set_176x144(void)
{
        NT99140_Initial_Setting(); 
	sensor_nt99140_setRegister(0x32BF, 0x60); 
	sensor_nt99140_setRegister(0x32C0, 0x64); 
	sensor_nt99140_setRegister(0x32C1, 0x64); 
	sensor_nt99140_setRegister(0x32C2, 0x64); 
	sensor_nt99140_setRegister(0x32C3, 0x00); 

	sensor_nt99140_setRegister(0x32C5, 0x20); 
	sensor_nt99140_setRegister(0x32C6, 0x20); 
	sensor_nt99140_setRegister(0x32C7, 0x00); 
	sensor_nt99140_setRegister(0x32C8, 0xBB); 
	sensor_nt99140_setRegister(0x32C9, 0x64); 
	sensor_nt99140_setRegister(0x32CA, 0x84); 
	sensor_nt99140_setRegister(0x32CB, 0x84); 
	sensor_nt99140_setRegister(0x32CC, 0x84); 
	sensor_nt99140_setRegister(0x32CD, 0x84); 
	sensor_nt99140_setRegister(0x32DB, 0x77); 
	sensor_nt99140_setRegister(0x32E0, 0x00); 
	sensor_nt99140_setRegister(0x32E1, 0xB0); 
	sensor_nt99140_setRegister(0x32E2, 0x00); 
	sensor_nt99140_setRegister(0x32E3, 0x90); 
	sensor_nt99140_setRegister(0x32E4, 0x04); 
	sensor_nt99140_setRegister(0x32E5, 0x7C); 
	sensor_nt99140_setRegister(0x32E6, 0x04); 
	sensor_nt99140_setRegister(0x32E7, 0x08); 
	sensor_nt99140_setRegister(0x3200, 0x3E); 
	sensor_nt99140_setRegister(0x3201, 0x0F); 
	sensor_nt99140_setRegister(0x3028, 0x18); 
	sensor_nt99140_setRegister(0x3029, 0x02); 
	sensor_nt99140_setRegister(0x302A, 0x00); 
	sensor_nt99140_setRegister(0x3022, 0x24); 
	sensor_nt99140_setRegister(0x3023, 0x24); 
	sensor_nt99140_setRegister(0x3002, 0x00); 
	sensor_nt99140_setRegister(0x3003, 0xA4); 
	sensor_nt99140_setRegister(0x3004, 0x00); 
	sensor_nt99140_setRegister(0x3005, 0x04); 
	sensor_nt99140_setRegister(0x3006, 0x04); 
	sensor_nt99140_setRegister(0x3007, 0x63); 
	sensor_nt99140_setRegister(0x3008, 0x02); 
	sensor_nt99140_setRegister(0x3009, 0xD3); 
	sensor_nt99140_setRegister(0x300A, 0x05); 
	sensor_nt99140_setRegister(0x300B, 0x3C); 
	sensor_nt99140_setRegister(0x300C, 0x03); 
	sensor_nt99140_setRegister(0x300D, 0xA4); 
	sensor_nt99140_setRegister(0x300E, 0x03); 
	sensor_nt99140_setRegister(0x300F, 0xC0); 
	sensor_nt99140_setRegister(0x3010, 0x02); 
	sensor_nt99140_setRegister(0x3011, 0xD0); 


	sensor_nt99140_setRegister(0x3201, 0x7F); 
	sensor_nt99140_setRegister(0x3021, 0x06); 
	sensor_nt99140_setRegister(0x3060, 0x01); 
}

static void NT99140_setQVGA_320x240_30FPS(void)
{               
	sensor_nt99140_setRegister(0x32BF, 0x60); 
	sensor_nt99140_setRegister(0x32C0, 0x64); 
	sensor_nt99140_setRegister(0x32C1, 0x64); 
	sensor_nt99140_setRegister(0x32C2, 0x64); 
	sensor_nt99140_setRegister(0x32C3, 0x00); 

	sensor_nt99140_setRegister(0x32C5, 0x20); 
	sensor_nt99140_setRegister(0x32C6, 0x20); 
	sensor_nt99140_setRegister(0x32C7, 0x00); 
	sensor_nt99140_setRegister(0x32C8, 0xBB); 
	sensor_nt99140_setRegister(0x32C9, 0x64); 
	sensor_nt99140_setRegister(0x32CA, 0x84); 
	sensor_nt99140_setRegister(0x32CB, 0x84); 
	sensor_nt99140_setRegister(0x32CC, 0x84); 
	sensor_nt99140_setRegister(0x32CD, 0x84); 
	sensor_nt99140_setRegister(0x32DB, 0x77); 
	sensor_nt99140_setRegister(0x32E0, 0x01); 
	sensor_nt99140_setRegister(0x32E1, 0x40); 
	sensor_nt99140_setRegister(0x32E2, 0x00); 
	sensor_nt99140_setRegister(0x32E3, 0xF0); 
	sensor_nt99140_setRegister(0x32E4, 0x02); 
	sensor_nt99140_setRegister(0x32E5, 0x02); 
	sensor_nt99140_setRegister(0x32E6, 0x02); 
	sensor_nt99140_setRegister(0x32E7, 0x03); 
	sensor_nt99140_setRegister(0x3200, 0x3E); 
	sensor_nt99140_setRegister(0x3201, 0x0F); 
	sensor_nt99140_setRegister(0x3028, 0x18); 
	sensor_nt99140_setRegister(0x3029, 0x02); 
	sensor_nt99140_setRegister(0x302A, 0x00); 
	sensor_nt99140_setRegister(0x3022, 0x24); 
	sensor_nt99140_setRegister(0x3023, 0x24); 
	sensor_nt99140_setRegister(0x3002, 0x00); 
	sensor_nt99140_setRegister(0x3003, 0xA4); 
	sensor_nt99140_setRegister(0x3004, 0x00); 
	sensor_nt99140_setRegister(0x3005, 0x04); 
	sensor_nt99140_setRegister(0x3006, 0x04); 
	sensor_nt99140_setRegister(0x3007, 0x63); 
	sensor_nt99140_setRegister(0x3008, 0x02); 
	sensor_nt99140_setRegister(0x3009, 0xD3); 
	sensor_nt99140_setRegister(0x300A, 0x05); 
	sensor_nt99140_setRegister(0x300B, 0x3C); 
	sensor_nt99140_setRegister(0x300C, 0x03); 
	sensor_nt99140_setRegister(0x300D, 0xA4); 
	sensor_nt99140_setRegister(0x300E, 0x03); 
	sensor_nt99140_setRegister(0x300F, 0xC0); 
	sensor_nt99140_setRegister(0x3010, 0x02); 
	sensor_nt99140_setRegister(0x3011, 0xD0); ; 
	sensor_nt99140_setRegister(0x3201, 0x7F); 
	sensor_nt99140_setRegister(0x3021, 0x06); 
	sensor_nt99140_setRegister(0x3060, 0x01);               
}   
                               
static void NT99140_setVGA_640x480_30FPS(void)
{                              
    MP_DEBUG1("---## %s ##---", __FUNCTION__);
	NT99140_setVGA_640x480();
	return;                
}                              

void NT99141_ScalerTo_800x480(void)
{
    /*for capture preview (Initial setting)*/
    MP_DEBUG1("---## %s ##---", __FUNCTION__);
	             
#if 0
//[YUYV_800x480_10.00_10.01_Fps_PCLK48Mhz]
sensor_nt99140_setRegister(0x32BF, 0x60); 
sensor_nt99140_setRegister(0x32C0, 0x74); 
sensor_nt99140_setRegister(0x32C1, 0x74); 
sensor_nt99140_setRegister(0x32C2, 0x74); 
sensor_nt99140_setRegister(0x32C3, 0x00); 
                                 
sensor_nt99140_setRegister(0x32C5, 0x20); 
sensor_nt99140_setRegister(0x32C6, 0x20); 
sensor_nt99140_setRegister(0x32C7, 0x00); 
sensor_nt99140_setRegister(0x32C8, 0x98); 
sensor_nt99140_setRegister(0x32C9, 0x74); 
sensor_nt99140_setRegister(0x32CA, 0x94); 
sensor_nt99140_setRegister(0x32CB, 0x94); 
sensor_nt99140_setRegister(0x32CC, 0x94); 
sensor_nt99140_setRegister(0x32CD, 0x94); 
sensor_nt99140_setRegister(0x32DB, 0x72); 
sensor_nt99140_setRegister(0x32E0, 0x03); 
sensor_nt99140_setRegister(0x32E1, 0x20); 
sensor_nt99140_setRegister(0x32E2, 0x01); 
sensor_nt99140_setRegister(0x32E3, 0xE0); 
sensor_nt99140_setRegister(0x32E4, 0x00); 
sensor_nt99140_setRegister(0x32E5, 0x80); 
sensor_nt99140_setRegister(0x32E6, 0x00); 
sensor_nt99140_setRegister(0x32E7, 0x80); 
sensor_nt99140_setRegister(0x3200, 0x3E); 
sensor_nt99140_setRegister(0x3201, 0x0F); 
sensor_nt99140_setRegister(0x3028, 0x0F); 
sensor_nt99140_setRegister(0x3029, 0x00); 
sensor_nt99140_setRegister(0x302A, 0x04); 
sensor_nt99140_setRegister(0x3022, 0x24); 
sensor_nt99140_setRegister(0x3023, 0x24); 
sensor_nt99140_setRegister(0x3002, 0x00); 
sensor_nt99140_setRegister(0x3003, 0x2C); 
sensor_nt99140_setRegister(0x3004, 0x00); 
sensor_nt99140_setRegister(0x3005, 0x04); 
sensor_nt99140_setRegister(0x3006, 0x04); 
sensor_nt99140_setRegister(0x3007, 0xDB); 
sensor_nt99140_setRegister(0x3008, 0x02); 
sensor_nt99140_setRegister(0x3009, 0xD3); 
sensor_nt99140_setRegister(0x300A, 0x06); 
sensor_nt99140_setRegister(0x300B, 0x2C); 
sensor_nt99140_setRegister(0x300C, 0x05); 
sensor_nt99140_setRegister(0x300D, 0xEE); 
sensor_nt99140_setRegister(0x300E, 0x04); 
sensor_nt99140_setRegister(0x300F, 0xB0); 
sensor_nt99140_setRegister(0x3010, 0x02); 
sensor_nt99140_setRegister(0x3011, 0xD0); 

#if 1//AE_ON
sensor_nt99140_setRegister(0x3201, 0x5F); 
sensor_nt99140_setRegister(0x3012, 0x02); 
sensor_nt99140_setRegister(0x3013, 0x5C); 
#else
sensor_nt99140_setRegister(0x3201, 0x7F); 
#endif
sensor_nt99140_setRegister(0x3021, 0x06); 
sensor_nt99140_setRegister(0x3060, 0x01); 
#endif

#if 0
//[YUYV_800x480_20.00_20.01_FpsPCLK48Mhz]
sensor_nt99140_setRegister(0x32BF, 0x60); 
sensor_nt99140_setRegister(0x32C0, 0x64); 
sensor_nt99140_setRegister(0x32C1, 0x64); 
sensor_nt99140_setRegister(0x32C2, 0x64); 
sensor_nt99140_setRegister(0x32C3, 0x00); 
 
sensor_nt99140_setRegister(0x32C5, 0x20); 
sensor_nt99140_setRegister(0x32C6, 0x20); 
sensor_nt99140_setRegister(0x32C7, 0x00); 
sensor_nt99140_setRegister(0x32C8, 0x98); 
sensor_nt99140_setRegister(0x32C9, 0x64); 
sensor_nt99140_setRegister(0x32CA, 0x84); 
sensor_nt99140_setRegister(0x32CB, 0x84); 
sensor_nt99140_setRegister(0x32CC, 0x84); 
sensor_nt99140_setRegister(0x32CD, 0x84); 
sensor_nt99140_setRegister(0x32DB, 0x72); 
sensor_nt99140_setRegister(0x32E0, 0x03); 
sensor_nt99140_setRegister(0x32E1, 0x20); 
sensor_nt99140_setRegister(0x32E2, 0x01); 
sensor_nt99140_setRegister(0x32E3, 0xE0); 
sensor_nt99140_setRegister(0x32E4, 0x00); 
sensor_nt99140_setRegister(0x32E5, 0x80); 
sensor_nt99140_setRegister(0x32E6, 0x00); 
sensor_nt99140_setRegister(0x32E7, 0x80); 
sensor_nt99140_setRegister(0x3200, 0x3E); 
sensor_nt99140_setRegister(0x3201, 0x0F); 
sensor_nt99140_setRegister(0x3028, 0x0F); 
sensor_nt99140_setRegister(0x3029, 0x00); 
sensor_nt99140_setRegister(0x302A, 0x04); 
sensor_nt99140_setRegister(0x3022, 0x24); 
sensor_nt99140_setRegister(0x3023, 0x24); 
sensor_nt99140_setRegister(0x3002, 0x00); 
sensor_nt99140_setRegister(0x3003, 0x2C); 
sensor_nt99140_setRegister(0x3004, 0x00); 
sensor_nt99140_setRegister(0x3005, 0x04); 
sensor_nt99140_setRegister(0x3006, 0x04); 
sensor_nt99140_setRegister(0x3007, 0xDB); 
sensor_nt99140_setRegister(0x3008, 0x02); 
sensor_nt99140_setRegister(0x3009, 0xD3); 
sensor_nt99140_setRegister(0x300A, 0x06); 
sensor_nt99140_setRegister(0x300B, 0x2C); 
sensor_nt99140_setRegister(0x300C, 0x02); 
sensor_nt99140_setRegister(0x300D, 0xF7); 
sensor_nt99140_setRegister(0x300E, 0x04); 
sensor_nt99140_setRegister(0x300F, 0xB0); 
sensor_nt99140_setRegister(0x3010, 0x02); 
sensor_nt99140_setRegister(0x3011, 0xD0); 

#if 1//AE_OFF
sensor_nt99140_setRegister(0x3201, 0x5F); 
sensor_nt99140_setRegister(0x3012, 0x03); 
sensor_nt99140_setRegister(0x3013, 0x80); 
#else
sensor_nt99140_setRegister(0x3201, 0x7F); 
#endif
sensor_nt99140_setRegister(0x3021, 0x06); 
sensor_nt99140_setRegister(0x3060, 0x01); 
#endif

#if 1
//[YUYV_800x480_30.00_30.03_Fps_PCLK70Mhz]

sensor_nt99140_setRegister(0x32BF, 0x60); 
sensor_nt99140_setRegister(0x32C0, 0x5A); 
sensor_nt99140_setRegister(0x32C1, 0x5A); 
sensor_nt99140_setRegister(0x32C2, 0x5A); 
sensor_nt99140_setRegister(0x32C3, 0x00); 

sensor_nt99140_setRegister(0x32C5, 0x20); 
sensor_nt99140_setRegister(0x32C6, 0x20); 
sensor_nt99140_setRegister(0x32C7, 0x00); 
sensor_nt99140_setRegister(0x32C8, 0xDF); 
sensor_nt99140_setRegister(0x32C9, 0x5A); 
sensor_nt99140_setRegister(0x32CA, 0x7A); 
sensor_nt99140_setRegister(0x32CB, 0x7A); 
sensor_nt99140_setRegister(0x32CC, 0x7A); 
sensor_nt99140_setRegister(0x32CD, 0x7A); 
sensor_nt99140_setRegister(0x32DB, 0x7B); 
sensor_nt99140_setRegister(0x32E0, 0x03); 
sensor_nt99140_setRegister(0x32E1, 0x20); 
sensor_nt99140_setRegister(0x32E2, 0x01); 
sensor_nt99140_setRegister(0x32E3, 0xE0); 
sensor_nt99140_setRegister(0x32E4, 0x00); 
sensor_nt99140_setRegister(0x32E5, 0x80); 
sensor_nt99140_setRegister(0x32E6, 0x00); 
sensor_nt99140_setRegister(0x32E7, 0x80); 
sensor_nt99140_setRegister(0x3200, 0x3E); 
sensor_nt99140_setRegister(0x3201, 0x0F); 
sensor_nt99140_setRegister(0x3028, 0x2E); 
sensor_nt99140_setRegister(0x3029, 0x10); 
sensor_nt99140_setRegister(0x302A, 0x04); 
if (Sensor_CurChannel_Get())
	sensor_nt99140_setRegister(0x3022, 0x26); 
else
	sensor_nt99140_setRegister(0x3022, 0x24); 
sensor_nt99140_setRegister(0x3023, 0x24); 
sensor_nt99140_setRegister(0x3002, 0x00); 
sensor_nt99140_setRegister(0x3003, 0x2C); 
sensor_nt99140_setRegister(0x3004, 0x00); 
sensor_nt99140_setRegister(0x3005, 0x04); 
sensor_nt99140_setRegister(0x3006, 0x04); 
sensor_nt99140_setRegister(0x3007, 0xDB); 
sensor_nt99140_setRegister(0x3008, 0x02); 
sensor_nt99140_setRegister(0x3009, 0xD3); 
sensor_nt99140_setRegister(0x300A, 0x06); 
sensor_nt99140_setRegister(0x300B, 0x2C); 
sensor_nt99140_setRegister(0x300C, 0x02); 
sensor_nt99140_setRegister(0x300D, 0xe7); //E7
sensor_nt99140_setRegister(0x300E, 0x04); 
sensor_nt99140_setRegister(0x300F, 0xB0); 
sensor_nt99140_setRegister(0x3010, 0x02); 
sensor_nt99140_setRegister(0x3011, 0xD0); 

//sensor_nt99140_setRegister(0x3201, 0x7F); 
#if 1//AE_OFF
sensor_nt99140_setRegister(0x3201, 0x5F); 
sensor_nt99140_setRegister(0x3012, 0x05); 
sensor_nt99140_setRegister(0x3013, 0x80); // 0x80
#else
sensor_nt99140_setRegister(0x3201, 0x7F); 
#endif

sensor_nt99140_setRegister(0x3021, 0x06); 
sensor_nt99140_setRegister(0x3060, 0x01); 

#endif                                           
}

void NT99141_Center_800x480(void)
{
    /*for capture preview (Initial setting)*/
    MP_DEBUG1("---## %s ##---", __FUNCTION__);

//--以下三组数据为按1:1取sensor中间的800*480数据出来，不做缩放
#if 1
//[YUYV_800x480_40.00_40.04_Fps]
 
sensor_nt99140_setRegister(0x32BF, 0x60); 
sensor_nt99140_setRegister(0x32C0, 0x54); 
sensor_nt99140_setRegister(0x32C1, 0x54); 
sensor_nt99140_setRegister(0x32C2, 0x54); 
sensor_nt99140_setRegister(0x32C3, 0x00); 
sensor_nt99140_setRegister(0x32C4, 0x20); 
sensor_nt99140_setRegister(0x32C5, 0x20); 
sensor_nt99140_setRegister(0x32C6, 0x20); 
sensor_nt99140_setRegister(0x32C7, 0x40); 
sensor_nt99140_setRegister(0x32C8, 0x31); 
sensor_nt99140_setRegister(0x32C9, 0x54); 
sensor_nt99140_setRegister(0x32CA, 0x74); 
sensor_nt99140_setRegister(0x32CB, 0x74); 
sensor_nt99140_setRegister(0x32CC, 0x74); 
sensor_nt99140_setRegister(0x32CD, 0x74); 
sensor_nt99140_setRegister(0x32DB, 0x83); 
sensor_nt99140_setRegister(0x3200, 0x3E); 
sensor_nt99140_setRegister(0x3201, 0x0F); 
sensor_nt99140_setRegister(0x3028, 0x17); 
sensor_nt99140_setRegister(0x3029, 0x00); 
sensor_nt99140_setRegister(0x302A, 0x04); 
sensor_nt99140_setRegister(0x3022, 0x24); 
sensor_nt99140_setRegister(0x3023, 0x24); 
sensor_nt99140_setRegister(0x3002, 0x00); 
sensor_nt99140_setRegister(0x3003, 0xF4); 
sensor_nt99140_setRegister(0x3004, 0x00); 
sensor_nt99140_setRegister(0x3005, 0x7C); 
sensor_nt99140_setRegister(0x3006, 0x04); 
sensor_nt99140_setRegister(0x3007, 0x13); 
sensor_nt99140_setRegister(0x3008, 0x02); 
sensor_nt99140_setRegister(0x3009, 0x5B); 
sensor_nt99140_setRegister(0x300A, 0x04); 
sensor_nt99140_setRegister(0x300B, 0x9C); 
sensor_nt99140_setRegister(0x300C, 0x02); 
sensor_nt99140_setRegister(0x300D, 0xFA); 
sensor_nt99140_setRegister(0x300E, 0x03); 
sensor_nt99140_setRegister(0x300F, 0x20); 
sensor_nt99140_setRegister(0x3010, 0x01); 
sensor_nt99140_setRegister(0x3011, 0xE0); 
sensor_nt99140_setRegister(0x3012, 0x02); 
sensor_nt99140_setRegister(0x3013, 0x5C);
sensor_nt99140_setRegister(0x3201, 0x3F); 
sensor_nt99140_setRegister(0x3021, 0x06); 
sensor_nt99140_setRegister(0x3060, 0x01); 
#endif

#if 0
//[YUYV_800x480_50.00_50.01_Fps]

sensor_nt99140_setRegister(0x32BF, 0x60); 
sensor_nt99140_setRegister(0x32C0, 0x50); 
sensor_nt99140_setRegister(0x32C1, 0x50); 
sensor_nt99140_setRegister(0x32C2, 0x50); 
sensor_nt99140_setRegister(0x32C3, 0x00); 
sensor_nt99140_setRegister(0x32C4, 0x20); 
sensor_nt99140_setRegister(0x32C5, 0x20); 
sensor_nt99140_setRegister(0x32C6, 0x20); 
sensor_nt99140_setRegister(0x32C7, 0x40); 
sensor_nt99140_setRegister(0x32C8, 0x31); 
sensor_nt99140_setRegister(0x32C9, 0x50); 
sensor_nt99140_setRegister(0x32CA, 0x70); 
sensor_nt99140_setRegister(0x32CB, 0x70); 
sensor_nt99140_setRegister(0x32CC, 0x70); 
sensor_nt99140_setRegister(0x32CD, 0x70); 
sensor_nt99140_setRegister(0x32DB, 0x83); 
sensor_nt99140_setRegister(0x3200, 0x3E); 
sensor_nt99140_setRegister(0x3201, 0x0F); 
sensor_nt99140_setRegister(0x3028, 0x17); 
sensor_nt99140_setRegister(0x3029, 0x00); 
sensor_nt99140_setRegister(0x302A, 0x04); 
sensor_nt99140_setRegister(0x3022, 0x24); 
sensor_nt99140_setRegister(0x3023, 0x24); 
sensor_nt99140_setRegister(0x3002, 0x00); 
sensor_nt99140_setRegister(0x3003, 0xF4); 
sensor_nt99140_setRegister(0x3004, 0x00); 
sensor_nt99140_setRegister(0x3005, 0x7C); 
sensor_nt99140_setRegister(0x3006, 0x04); 
sensor_nt99140_setRegister(0x3007, 0x13); 
sensor_nt99140_setRegister(0x3008, 0x02); 
sensor_nt99140_setRegister(0x3009, 0x5B); 
sensor_nt99140_setRegister(0x300A, 0x04); 
sensor_nt99140_setRegister(0x300B, 0x9C); 
sensor_nt99140_setRegister(0x300C, 0x02); 
sensor_nt99140_setRegister(0x300D, 0x62); 
sensor_nt99140_setRegister(0x300E, 0x03); 
sensor_nt99140_setRegister(0x300F, 0x20); 
sensor_nt99140_setRegister(0x3010, 0x01); 
sensor_nt99140_setRegister(0x3011, 0xE0); 
sensor_nt99140_setRegister(0x3012, 0x02); 
sensor_nt99140_setRegister(0x3013, 0x5C);
sensor_nt99140_setRegister(0x3201, 0x3F); 
sensor_nt99140_setRegister(0x3021, 0x06); 
sensor_nt99140_setRegister(0x3060, 0x01); 
#endif

#if 0
//[YUYV_800x480_60.00_60.06_Fps]

sensor_nt99140_setRegister(0x32BF, 0x60); 
sensor_nt99140_setRegister(0x32C0, 0x4A); 
sensor_nt99140_setRegister(0x32C1, 0x4B); 
sensor_nt99140_setRegister(0x32C2, 0x4B); 
sensor_nt99140_setRegister(0x32C3, 0x00); 
sensor_nt99140_setRegister(0x32C4, 0x20); 
sensor_nt99140_setRegister(0x32C5, 0x20); 
sensor_nt99140_setRegister(0x32C6, 0x20); 
sensor_nt99140_setRegister(0x32C7, 0x40); 
sensor_nt99140_setRegister(0x32C8, 0x31); 
sensor_nt99140_setRegister(0x32C9, 0x4B); 
sensor_nt99140_setRegister(0x32CA, 0x6B); 
sensor_nt99140_setRegister(0x32CB, 0x6B); 
sensor_nt99140_setRegister(0x32CC, 0x6B); 
sensor_nt99140_setRegister(0x32CD, 0x6A); 
sensor_nt99140_setRegister(0x32DB, 0x83); 
sensor_nt99140_setRegister(0x3200, 0x3E); 
sensor_nt99140_setRegister(0x3201, 0x0F); 
sensor_nt99140_setRegister(0x3028, 0x17); 
sensor_nt99140_setRegister(0x3029, 0x00); 
sensor_nt99140_setRegister(0x302A, 0x04); 
sensor_nt99140_setRegister(0x3022, 0x24); 
sensor_nt99140_setRegister(0x3023, 0x24); 
sensor_nt99140_setRegister(0x3002, 0x00); 
sensor_nt99140_setRegister(0x3003, 0xF4); 
sensor_nt99140_setRegister(0x3004, 0x00); 
sensor_nt99140_setRegister(0x3005, 0x7C); 
sensor_nt99140_setRegister(0x3006, 0x04); 
sensor_nt99140_setRegister(0x3007, 0x13); 
sensor_nt99140_setRegister(0x3008, 0x02); 
sensor_nt99140_setRegister(0x3009, 0x5B); 
sensor_nt99140_setRegister(0x300A, 0x04); 
sensor_nt99140_setRegister(0x300B, 0x9C); 
sensor_nt99140_setRegister(0x300C, 0x01); 
sensor_nt99140_setRegister(0x300D, 0xFC); 
sensor_nt99140_setRegister(0x300E, 0x03); 
sensor_nt99140_setRegister(0x300F, 0x20); 
sensor_nt99140_setRegister(0x3010, 0x01); 
sensor_nt99140_setRegister(0x3011, 0xE0); 
sensor_nt99140_setRegister(0x3012, 0x02); 
sensor_nt99140_setRegister(0x3013, 0x5C);
sensor_nt99140_setRegister(0x3201, 0x3F); 
sensor_nt99140_setRegister(0x3021, 0x06); 
sensor_nt99140_setRegister(0x3060, 0x01); 
#endif

}

static void NT99140_setSVGA_800x480(void)
{
    /*for capture preview (Initial setting)*/
    MP_DEBUG1("---## %s ##---", __FUNCTION__);
	NT99140_Initial_Setting();

#if 1//!SHOW_CENTER
	sensor_nt99140_setRegister(0x32F1, 0x01);//BW 
#endif

	if (Sensor_GetPicMode())
		NT99141_Center_800x480();
	else
		NT99141_ScalerTo_800x480();

}                                          
                                           
static void NT99140_setSVGA_800x600(void)  
{                                          
    /*for capture preview (Initial setting)*/
    MP_DEBUG1("---## %s ##---", __FUNCTION__);
	NT99140_Initial_Setting();         
                                           
#if 1//!SHOW_CENTER                        
	sensor_nt99140_setRegister(0x32F1, 0x01);//BW 
#endif
//[YUYV_800x600_30.00_30.00_Fps_PCLK64Mhz]

sensor_nt99140_setRegister(0x32BF, 0x60); 
sensor_nt99140_setRegister(0x32C0, 0x5A); 
sensor_nt99140_setRegister(0x32C1, 0x5A); 
sensor_nt99140_setRegister(0x32C2, 0x5A); 
sensor_nt99140_setRegister(0x32C3, 0x00); 

sensor_nt99140_setRegister(0x32C5, 0x20); 
sensor_nt99140_setRegister(0x32C6, 0x20); 
sensor_nt99140_setRegister(0x32C7, 0x00); 
sensor_nt99140_setRegister(0x32C8, 0xEF); 
sensor_nt99140_setRegister(0x32C9, 0x5A); 
sensor_nt99140_setRegister(0x32CA, 0x7A); 
sensor_nt99140_setRegister(0x32CB, 0x7A); 
sensor_nt99140_setRegister(0x32CC, 0x7A); 
sensor_nt99140_setRegister(0x32CD, 0x7A); 
sensor_nt99140_setRegister(0x32DB, 0x7D); 
sensor_nt99140_setRegister(0x32E0, 0x03); 
sensor_nt99140_setRegister(0x32E1, 0x20); 
sensor_nt99140_setRegister(0x32E2, 0x02); 
sensor_nt99140_setRegister(0x32E3, 0x58); 
sensor_nt99140_setRegister(0x32E4, 0x00); 
sensor_nt99140_setRegister(0x32E5, 0x33); 
sensor_nt99140_setRegister(0x32E6, 0x00); 
sensor_nt99140_setRegister(0x32E7, 0x33); 
sensor_nt99140_setRegister(0x3200, 0x3E); 
sensor_nt99140_setRegister(0x3201, 0x0F); 
sensor_nt99140_setRegister(0x3028, 0x3F); 
sensor_nt99140_setRegister(0x3029, 0x20); 
sensor_nt99140_setRegister(0x302A, 0x04); 
sensor_nt99140_setRegister(0x3022, 0x24); 
sensor_nt99140_setRegister(0x3023, 0x24); 
sensor_nt99140_setRegister(0x3002, 0x00); 
sensor_nt99140_setRegister(0x3003, 0xA4); 
sensor_nt99140_setRegister(0x3004, 0x00); 
sensor_nt99140_setRegister(0x3005, 0x04); 
sensor_nt99140_setRegister(0x3006, 0x04); 
sensor_nt99140_setRegister(0x3007, 0x63); 
sensor_nt99140_setRegister(0x3008, 0x02); 
sensor_nt99140_setRegister(0x3009, 0xD3); 
sensor_nt99140_setRegister(0x300A, 0x05); 
sensor_nt99140_setRegister(0x300B, 0x3C); 
sensor_nt99140_setRegister(0x300C, 0x03); 
sensor_nt99140_setRegister(0x300D, 0x1C); 
sensor_nt99140_setRegister(0x300E, 0x03); 
sensor_nt99140_setRegister(0x300F, 0xC0); 
sensor_nt99140_setRegister(0x3010, 0x02); 
sensor_nt99140_setRegister(0x3011, 0xD0); 

sensor_nt99140_setRegister(0x3201, 0x7F); 
sensor_nt99140_setRegister(0x3021, 0x06); 
sensor_nt99140_setRegister(0x3060, 0x01); 

			
}

void SetBWmodeOnOff(BYTE bOn)
{
	if (bOn)
		sensor_nt99140_setRegister(0x32F1, 0x01);//BW 
	else
		sensor_nt99140_setRegister(0x32F1, 0); 
}

static void NT99140_setVGA_640x480(void)
{
    /*for capture preview (Initial setting)*/
    MP_DEBUG("---## %s ##--", __FUNCTION__);
	NT99140_Initial_Setting();

#if 1//(PRODUCT_UI==UI_WELDING)
	//sensor_nt99140_setRegister(0x32F1, 0x01);//BW 
#endif
//MCLK:      12.00MHz 
//PCLK:      50.00MHz 
//Size:      640x480 
//FPS:       20.00~320.02 


//Flicker:   50Hz 
		sensor_nt99140_setRegister(0x32BF, 0x60);  //AE Start
		sensor_nt99140_setRegister(0x32C0, 0x64); 
		sensor_nt99140_setRegister(0x32C1, 0x64); 
		sensor_nt99140_setRegister(0x32C2, 0x64); 
		sensor_nt99140_setRegister(0x32C3, 0x00); 
 
		sensor_nt99140_setRegister(0x32C5, 0x20); 
		sensor_nt99140_setRegister(0x32C6, 0x20); 
		sensor_nt99140_setRegister(0x32C7, 0x00); 
		sensor_nt99140_setRegister(0x32C8, 0xBB); 
		sensor_nt99140_setRegister(0x32C9, 0x64); 
		sensor_nt99140_setRegister(0x32CA, 0x84); 
		sensor_nt99140_setRegister(0x32CB, 0x84); 
		sensor_nt99140_setRegister(0x32CC, 0x84); 
		sensor_nt99140_setRegister(0x32CD, 0x84);  //AE End
		sensor_nt99140_setRegister(0x32DB, 0x77);  //Scale Start
		sensor_nt99140_setRegister(0x32E0, 0x02); 
		sensor_nt99140_setRegister(0x32E1, 0x80); 
		sensor_nt99140_setRegister(0x32E2, 0x01); 
		sensor_nt99140_setRegister(0x32E3, 0xE0); 
		sensor_nt99140_setRegister(0x32E4, 0x00); 
		sensor_nt99140_setRegister(0x32E5, 0x80); 
		sensor_nt99140_setRegister(0x32E6, 0x00);  //Scale End
		sensor_nt99140_setRegister(0x32E7, 0x80);  //Output Format
		sensor_nt99140_setRegister(0x3200, 0x3E);  //Mode Control
		sensor_nt99140_setRegister(0x3201, 0x0F);  //Mode Control
		sensor_nt99140_setRegister(0x3028, 0x18);  //PLL Start
		sensor_nt99140_setRegister(0x3029, 0x02); 
		sensor_nt99140_setRegister(0x302A, 0x00);  //PLL End
		sensor_nt99140_setRegister(0x3022, 0x24);  //Timing Start
		sensor_nt99140_setRegister(0x3023, 0x24); 
		sensor_nt99140_setRegister(0x3002, 0x00); 
		sensor_nt99140_setRegister(0x3003, 0xA4); 
		sensor_nt99140_setRegister(0x3004, 0x00);  //Timing End
		sensor_nt99140_setRegister(0x3005, 0x04); 
		sensor_nt99140_setRegister(0x3006, 0x04); 
		sensor_nt99140_setRegister(0x3007, 0x63); 
		sensor_nt99140_setRegister(0x3008, 0x02); 
		sensor_nt99140_setRegister(0x3009, 0xD3); 
		sensor_nt99140_setRegister(0x300A, 0x05);
		sensor_nt99140_setRegister(0x300B, 0x3C); 
		sensor_nt99140_setRegister(0x300C, 0x03);                       
                sensor_nt99140_setRegister(0x300D, 0xA4);
                sensor_nt99140_setRegister(0x300E, 0x03);
                sensor_nt99140_setRegister(0x300F, 0xC0);
                sensor_nt99140_setRegister(0x3010, 0x02);
                sensor_nt99140_setRegister(0x3011, 0xD0);

                sensor_nt99140_setRegister(0x3201, 0x7F);
                sensor_nt99140_setRegister(0x3021, 0x06);
                sensor_nt99140_setRegister(0x3060, 0x01);
}                                          
                                           
static void NT99140_setQXGA_2048x1536(void)
{                                          
    MP_ALERT("## %s ##", __FUNCTION__);     
                                            
    MP_ALERT("%s: not ready", __FUNCTION__);
    __asm("break 100");                     
}


static void NT99140_set_352x240_30FPS(void)
{
    MP_ALERT("## %s ##", __FUNCTION__);
    MP_ALERT("%s: Not Support!!", __FUNCTION__);
    __asm("break 100");
}


static void NT99140_set_1024x600(void)
{
    NT99140_Initial_Setting(); 

#if 1//!SHOW_CENTER
	sensor_nt99140_setRegister(0x32F1, 0x01);//BW 
#endif

//[YUYV_1024x600_25.00_25.00_Fps_PCLK64Mhz]

sensor_nt99140_setRegister(0x32BF, 0x60); 
sensor_nt99140_setRegister(0x32C0, 0x60); 
sensor_nt99140_setRegister(0x32C1, 0x60); 
sensor_nt99140_setRegister(0x32C2, 0x60); 
sensor_nt99140_setRegister(0x32C3, 0x00); 

sensor_nt99140_setRegister(0x32C5, 0x20); 
sensor_nt99140_setRegister(0x32C6, 0x20); 
sensor_nt99140_setRegister(0x32C7, 0x00); 
sensor_nt99140_setRegister(0x32C8, 0xC7); 
sensor_nt99140_setRegister(0x32C9, 0x60); 
sensor_nt99140_setRegister(0x32CA, 0x80); 
sensor_nt99140_setRegister(0x32CB, 0x80); 
sensor_nt99140_setRegister(0x32CC, 0x80); 
sensor_nt99140_setRegister(0x32CD, 0x80); 
sensor_nt99140_setRegister(0x32DB, 0x78); 
sensor_nt99140_setRegister(0x32E0, 0x04); 
sensor_nt99140_setRegister(0x32E1, 0x00); 
sensor_nt99140_setRegister(0x32E2, 0x02); 
sensor_nt99140_setRegister(0x32E3, 0x58); 
sensor_nt99140_setRegister(0x32E4, 0x00); 
sensor_nt99140_setRegister(0x32E5, 0x33); 
sensor_nt99140_setRegister(0x32E6, 0x00); 
sensor_nt99140_setRegister(0x32E7, 0x33); 
sensor_nt99140_setRegister(0x3200, 0x3E); 
sensor_nt99140_setRegister(0x3201, 0x0F); 
sensor_nt99140_setRegister(0x3028, 0x3F); 
sensor_nt99140_setRegister(0x3029, 0x20); 
sensor_nt99140_setRegister(0x302A, 0x04); 
sensor_nt99140_setRegister(0x3022, 0x24); 
sensor_nt99140_setRegister(0x3023, 0x24); 
sensor_nt99140_setRegister(0x3002, 0x00); 
sensor_nt99140_setRegister(0x3003, 0x1E); 
sensor_nt99140_setRegister(0x3004, 0x00); 
sensor_nt99140_setRegister(0x3005, 0x04); 
sensor_nt99140_setRegister(0x3006, 0x04); 
sensor_nt99140_setRegister(0x3007, 0xE9); 
sensor_nt99140_setRegister(0x3008, 0x02); 
sensor_nt99140_setRegister(0x3009, 0xD3); 
sensor_nt99140_setRegister(0x300A, 0x06); 
sensor_nt99140_setRegister(0x300B, 0x48); 
sensor_nt99140_setRegister(0x300C, 0x03); 
sensor_nt99140_setRegister(0x300D, 0x1C); 
sensor_nt99140_setRegister(0x300E, 0x04); 
sensor_nt99140_setRegister(0x300F, 0xCC); 
sensor_nt99140_setRegister(0x3010, 0x02); 
sensor_nt99140_setRegister(0x3011, 0xD0); 

sensor_nt99140_setRegister(0x3201, 0x7F); 
sensor_nt99140_setRegister(0x3021, 0x06); 
sensor_nt99140_setRegister(0x3060, 0x01); 


	
}



static void NT99140_set_720P_1280x720(void)
{
    NT99140_Initial_Setting(); 

#if 1//!SHOW_CENTER
	sensor_nt99140_setRegister(0x32F1, 0x01);//BW 
#endif

//[YUYV_1280x720_25.00_25.00_Fps_PCLK64Mhz] 

sensor_nt99140_setRegister(0x32BF, 0x60); 
sensor_nt99140_setRegister(0x32C0, 0x60); 
sensor_nt99140_setRegister(0x32C1, 0x60); 
sensor_nt99140_setRegister(0x32C2, 0x60); 
sensor_nt99140_setRegister(0x32C3, 0x00); 

sensor_nt99140_setRegister(0x32C5, 0x20); 
sensor_nt99140_setRegister(0x32C6, 0x20); 
sensor_nt99140_setRegister(0x32C7, 0x00); 
sensor_nt99140_setRegister(0x32C8, 0xC1); 
sensor_nt99140_setRegister(0x32C9, 0x60); 
sensor_nt99140_setRegister(0x32CA, 0x80); 
sensor_nt99140_setRegister(0x32CB, 0x80); 
sensor_nt99140_setRegister(0x32CC, 0x80); 
sensor_nt99140_setRegister(0x32CD, 0x80); 
sensor_nt99140_setRegister(0x32DB, 0x78); 
sensor_nt99140_setRegister(0x3200, 0x3E); 
sensor_nt99140_setRegister(0x3201, 0x0F); 
sensor_nt99140_setRegister(0x3028, 0x3F); 
sensor_nt99140_setRegister(0x3029, 0x20); 
sensor_nt99140_setRegister(0x302A, 0x04); 
sensor_nt99140_setRegister(0x3022, 0x24); 
sensor_nt99140_setRegister(0x3023, 0x24); 
sensor_nt99140_setRegister(0x3002, 0x00); 
sensor_nt99140_setRegister(0x3003, 0x04); 
sensor_nt99140_setRegister(0x3004, 0x00); 
sensor_nt99140_setRegister(0x3005, 0x04); 
sensor_nt99140_setRegister(0x3006, 0x05); 
sensor_nt99140_setRegister(0x3007, 0x03); 
sensor_nt99140_setRegister(0x3008, 0x02); 
sensor_nt99140_setRegister(0x3009, 0xD3); 
sensor_nt99140_setRegister(0x300A, 0x06); 
sensor_nt99140_setRegister(0x300B, 0x7C); 
sensor_nt99140_setRegister(0x300C, 0x03); 
sensor_nt99140_setRegister(0x300D, 0x03); 
sensor_nt99140_setRegister(0x300E, 0x05); 
sensor_nt99140_setRegister(0x300F, 0x00); 
sensor_nt99140_setRegister(0x3010, 0x02); 
sensor_nt99140_setRegister(0x3011, 0xD0); 

sensor_nt99140_setRegister(0x3201, 0x1F); 
sensor_nt99140_setRegister(0x3021, 0x06); 
sensor_nt99140_setRegister(0x3060, 0x01); 
	
}


void Drive_Sensor_NT99140(void)
{

        ////I2CM_FreqChg(0x54, 300000);             // grad mark
        sensor_nt99140_GPIO_Init();

    //    if(sensor_mode==MODE_RECORD)
        {
                if(sensor_IMGSIZE_ID==SIZE_QVGA_320x240)
                        NT99140_setQVGA_320x240_30FPS();
                else  if(sensor_IMGSIZE_ID==SIZE_CIF_352x240)
                        NT99140_set_352x240_30FPS();
                else   if(sensor_IMGSIZE_ID==SIZE_VGA_640x480)
                        NT99140_setVGA_640x480_30FPS();
                else   if(sensor_IMGSIZE_ID==SIZE_SVGA_800x480)
                        NT99140_setSVGA_800x480();
                else   if(sensor_IMGSIZE_ID==SIZE_SVGA_800x600)
                        NT99140_setSVGA_800x600();
                else 
                {//default 720p
                        NT99140_set_720P_1280x720();
                }
        }
#if 0//CAMCODERRECORD_OLDMODE
        else if(sensor_mode==MODE_CAPTURE)
        {
            //frank lin capture flow
            MP_ALERT("%s:  capture preview mode (720p)", __FUNCTION__); 
            NT_Capture_preview();
        }
        else if(sensor_mode==MODE_PCCAM)
        {
            if (sensor_IMGSIZE_ID==SIZE_720P_1280x720)
                NT99140_set_720P_1280x720();
            else if (sensor_IMGSIZE_ID==SIZE_VGA_640x480)
                NT99140_setVGA_640x480_30FPS();
            else if (sensor_IMGSIZE_ID==SIZE_QVGA_320x240)
                NT99140_setQVGA_320x240_30FPS();
            else
                NT99140_set_176x144();
        }
        else
#endif
        {
    //        NT99140_set_720P_1280x720();
        }

}



#endif


