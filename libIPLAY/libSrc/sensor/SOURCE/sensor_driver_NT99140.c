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

#if ((SENSOR_ENABLE == ENABLE) && defined(SENSOR_TYPE_NT99140))
#if 1       // Software I2C

#define SENSOR_NT99140_DEVICE_ADDRESS   (0x54>>1)
#define IIC_FLAG_NO_ACK     1
#define IIC_FLAG_ACK        2
#define IIC_FLAG_START      3
#define IIC_FLAG_STOP       4
#define IIC_FLAG_IGNORE     5

#define WAIT_TIME_IIC_DELAY 10
#define WAIT_TIME_WAIT_ACK  50

//--------------------------------------------------------
static BYTE ACK1=1, ACK2=1, ACK3=1, ACK4=1;

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

#elif (PRODUCT_PCBA==PCBA_MAIN_BOARD_V12)

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
        data |= __sensor_nt99140_i2c_read_data();
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
        data |= __sensor_nt99140_i2c_read_data();
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
    //SemaphoreWait(SW_I2C_SEMA_ID);
    ACK1 = SW_sensor_nt99140_IIC_SetValue(SENSOR_NT99140_DEVICE_ADDRESS<<1, IIC_FLAG_START); 
    ACK2 = SW_sensor_nt99140_IIC_SetValue((addr>>8)&0xff, IIC_FLAG_IGNORE);
    ACK3 = SW_sensor_nt99140_IIC_SetValue(addr&0xff, IIC_FLAG_IGNORE);
    ACK4 = SW_sensor_nt99140_IIC_SetValue(data, IIC_FLAG_STOP);
    //SemaphoreRelease(SW_I2C_SEMA_ID);
}

static BYTE sensor_nt99140_readRegister(WORD addr)
{
    BYTE value1 = 0;
    //SemaphoreWait(SW_I2C_SEMA_ID);
    ACK1 = SW_sensor_nt99140_IIC_SetValue(SENSOR_NT99140_DEVICE_ADDRESS<<1, IIC_FLAG_START);
    ACK2 = SW_sensor_nt99140_IIC_SetValue((addr>>8)&0xff, IIC_FLAG_IGNORE);
    ACK2 = SW_sensor_nt99140_IIC_SetValue(addr&0xff, IIC_FLAG_IGNORE);
    __sensor_nt99140_i2c_data_high();
    ACK3 = SW_sensor_nt99140_IIC_SetValue((SENSOR_NT99140_DEVICE_ADDRESS<<1) | 0x01, IIC_FLAG_START);
    value1 = SW_sensor_nt99140_IIC_GetValue();
    //SemaphoreRelease(SW_I2C_SEMA_ID);
    return value1;
}

#endif


#if ((SENSOR_ENABLE == ENABLE) && defined(SENSOR_TYPE_NT99140))

extern BYTE sensor_mode;
extern BYTE sensor_IMGSIZE_ID;

static void NT99140_NightMode();

int boIsNightMode = 0;              // 0: day, 1: overcast, 2: night
#if (PRODUCT_UI==UI_WELDING)
static BYTE st_bhaveSetted[2] = {0};
#else
static BYTE st_bhaveSetted=0;
#endif
static void NT99140_Initial_Setting()
{
#if (PRODUCT_UI==UI_WELDING)
	if (Sensor_CurChannel_Get()>1)
			return;
    if (st_bhaveSetted[Sensor_CurChannel_Get()])
        return;
    st_bhaveSetted[Sensor_CurChannel_Get()] = TRUE;
#else
	if (st_bhaveSetted)
		return;
	st_bhaveSetted=1;
#endif
    MP_DEBUG("First time call function %s()", __FUNCTION__);
#if 1//99142

sensor_nt99140_setRegister(0x3069, 0x01);
sensor_nt99140_setRegister(0x306A, 0x03);
sensor_nt99140_setRegister(0x3100, 0x03);
sensor_nt99140_setRegister(0x3101, 0x00);
sensor_nt99140_setRegister(0x3102, 0x0A);
sensor_nt99140_setRegister(0x3103, 0x00);
sensor_nt99140_setRegister(0x3105, 0x03);
sensor_nt99140_setRegister(0x3106, 0x04);
sensor_nt99140_setRegister(0x3107, 0x20);
sensor_nt99140_setRegister(0x3108, 0x00);
sensor_nt99140_setRegister(0x3109, 0x02);
sensor_nt99140_setRegister(0x307D, 0x01);
sensor_nt99140_setRegister(0x310A, 0x05);
sensor_nt99140_setRegister(0x310C, 0x00);
sensor_nt99140_setRegister(0x310D, 0x80);
sensor_nt99140_setRegister(0x3110, 0x33);
sensor_nt99140_setRegister(0x3111, 0x59);
sensor_nt99140_setRegister(0x3112, 0x44);
sensor_nt99140_setRegister(0x3113, 0x66);
sensor_nt99140_setRegister(0x3114, 0x66);
sensor_nt99140_setRegister(0x311D, 0x40);
sensor_nt99140_setRegister(0x3127, 0x01);
sensor_nt99140_setRegister(0x3129, 0x44);
sensor_nt99140_setRegister(0x3136, 0x59);
sensor_nt99140_setRegister(0x313F, 0x02);
sensor_nt99140_setRegister(0x30A0, 0x03);
sensor_nt99140_setRegister(0x30A1, 0x23);
sensor_nt99140_setRegister(0x30A2, 0x70);
sensor_nt99140_setRegister(0x30A3, 0x01);
sensor_nt99140_setRegister(0x303E, 0x00);
sensor_nt99140_setRegister(0x303F, 0x32);
sensor_nt99140_setRegister(0x3051, 0x3A);
sensor_nt99140_setRegister(0x3052, 0x0F);
sensor_nt99140_setRegister(0x3055, 0x00);
sensor_nt99140_setRegister(0x3056, 0x28);
sensor_nt99140_setRegister(0x305F, 0x33);
sensor_nt99140_setRegister(0x308B, 0x26);
sensor_nt99140_setRegister(0x308C, 0x20);
sensor_nt99140_setRegister(0x308D, 0x38);
sensor_nt99140_setRegister(0x308E, 0x3A);
sensor_nt99140_setRegister(0x308F, 0x0D);
sensor_nt99140_setRegister(0x324F, 0x00);
sensor_nt99140_setRegister(0x3210, 0x10);
sensor_nt99140_setRegister(0x3211, 0x10);
sensor_nt99140_setRegister(0x3212, 0x10);
sensor_nt99140_setRegister(0x3213, 0x10);
sensor_nt99140_setRegister(0x3214, 0x0C);
sensor_nt99140_setRegister(0x3215, 0x0C);
sensor_nt99140_setRegister(0x3216, 0x0C);
sensor_nt99140_setRegister(0x3217, 0x0C);
sensor_nt99140_setRegister(0x3218, 0x0C);
sensor_nt99140_setRegister(0x3219, 0x0C);
sensor_nt99140_setRegister(0x321A, 0x0C);
sensor_nt99140_setRegister(0x321B, 0x0C);
sensor_nt99140_setRegister(0x321C, 0x0B);
sensor_nt99140_setRegister(0x321D, 0x0B);
sensor_nt99140_setRegister(0x321E, 0x0B);
sensor_nt99140_setRegister(0x321F, 0x0B);
sensor_nt99140_setRegister(0x3230, 0x03);
sensor_nt99140_setRegister(0x3231, 0x00);
sensor_nt99140_setRegister(0x3232, 0x00);
sensor_nt99140_setRegister(0x3233, 0x03);
sensor_nt99140_setRegister(0x3234, 0x00);
sensor_nt99140_setRegister(0x3235, 0x00);
sensor_nt99140_setRegister(0x3236, 0x00);
sensor_nt99140_setRegister(0x3237, 0x00);
sensor_nt99140_setRegister(0x3238, 0x18);
sensor_nt99140_setRegister(0x3239, 0x18);
sensor_nt99140_setRegister(0x323A, 0x18);
sensor_nt99140_setRegister(0x3241, 0x44);
sensor_nt99140_setRegister(0x3243, 0xC3);
sensor_nt99140_setRegister(0x3244, 0x00);
sensor_nt99140_setRegister(0x3245, 0x00);
sensor_nt99140_setRegister(0x324F, 0x00);
sensor_nt99140_setRegister(0x3250, 0x28);
sensor_nt99140_setRegister(0x3251, 0x20);
sensor_nt99140_setRegister(0x3252, 0x2F);
sensor_nt99140_setRegister(0x3253, 0x20);
sensor_nt99140_setRegister(0x3254, 0x33);
sensor_nt99140_setRegister(0x3255, 0x26);
sensor_nt99140_setRegister(0x3256, 0x25);
sensor_nt99140_setRegister(0x3257, 0x19);
sensor_nt99140_setRegister(0x3258, 0x44);
sensor_nt99140_setRegister(0x3259, 0x35);
sensor_nt99140_setRegister(0x325A, 0x24);
sensor_nt99140_setRegister(0x325B, 0x16);
sensor_nt99140_setRegister(0x325C, 0xA0);
sensor_nt99140_setRegister(0x3262, 0x00);
sensor_nt99140_setRegister(0x3268, 0x01);
sensor_nt99140_setRegister(0x3270, 0x00);
sensor_nt99140_setRegister(0x3271, 0x0C);
sensor_nt99140_setRegister(0x3272, 0x18);
sensor_nt99140_setRegister(0x3273, 0x32);
sensor_nt99140_setRegister(0x3274, 0x44);
sensor_nt99140_setRegister(0x3275, 0x54);
sensor_nt99140_setRegister(0x3276, 0x70);
sensor_nt99140_setRegister(0x3277, 0x88);
sensor_nt99140_setRegister(0x3278, 0x9D);
sensor_nt99140_setRegister(0x3279, 0xB0);
sensor_nt99140_setRegister(0x327A, 0xCF);
sensor_nt99140_setRegister(0x327B, 0xE2);
sensor_nt99140_setRegister(0x327C, 0xEF);
sensor_nt99140_setRegister(0x327D, 0xF7);
sensor_nt99140_setRegister(0x327E, 0xFF);
sensor_nt99140_setRegister(0x3290, 0x77);
sensor_nt99140_setRegister(0x3292, 0x73);
sensor_nt99140_setRegister(0x3297, 0x03);
sensor_nt99140_setRegister(0x32B0, 0x46);
sensor_nt99140_setRegister(0x32B1, 0xBB);
sensor_nt99140_setRegister(0x32B2, 0x14);
sensor_nt99140_setRegister(0x32B3, 0xA0);
sensor_nt99140_setRegister(0x32B4, 0x20);
sensor_nt99140_setRegister(0x32B8, 0x06);
sensor_nt99140_setRegister(0x32B9, 0x06);
sensor_nt99140_setRegister(0x32BC, 0x3C);
sensor_nt99140_setRegister(0x32BD, 0x04);
sensor_nt99140_setRegister(0x32BE, 0x04);
sensor_nt99140_setRegister(0x32CB, 0x14);
sensor_nt99140_setRegister(0x32CC, 0x70);
sensor_nt99140_setRegister(0x32CD, 0xA0);
sensor_nt99140_setRegister(0x32F1, 0x05);
sensor_nt99140_setRegister(0x32F2, 0x80);
sensor_nt99140_setRegister(0x32FC, 0x00);
sensor_nt99140_setRegister(0x3302, 0x00);
sensor_nt99140_setRegister(0x3303, 0x4F);
sensor_nt99140_setRegister(0x3304, 0x00);
sensor_nt99140_setRegister(0x3305, 0x9F);
sensor_nt99140_setRegister(0x3306, 0x00);
sensor_nt99140_setRegister(0x3307, 0x0F);
sensor_nt99140_setRegister(0x3308, 0x07);
sensor_nt99140_setRegister(0x3309, 0xB0);
sensor_nt99140_setRegister(0x330A, 0x07);
sensor_nt99140_setRegister(0x330B, 0x04);
sensor_nt99140_setRegister(0x330C, 0x01);
sensor_nt99140_setRegister(0x330D, 0x4E);
sensor_nt99140_setRegister(0x330E, 0x01);
sensor_nt99140_setRegister(0x330F, 0x06);
sensor_nt99140_setRegister(0x3310, 0x06);
sensor_nt99140_setRegister(0x3311, 0xEE);
sensor_nt99140_setRegister(0x3312, 0x00);
sensor_nt99140_setRegister(0x3313, 0x0B);
sensor_nt99140_setRegister(0x3326, 0x09);//0x16
sensor_nt99140_setRegister(0x3327, 0x01);
sensor_nt99140_setRegister(0x3332, 0x80);
sensor_nt99140_setRegister(0x3360, 0x0A);
sensor_nt99140_setRegister(0x3361, 0x1F);
sensor_nt99140_setRegister(0x3362, 0x26);
sensor_nt99140_setRegister(0x3363, 0x31);
sensor_nt99140_setRegister(0x3364, 0x0B);
sensor_nt99140_setRegister(0x3365, 0x04);
sensor_nt99140_setRegister(0x3366, 0x08);
sensor_nt99140_setRegister(0x3367, 0x0C);
sensor_nt99140_setRegister(0x3368, 0x38);
sensor_nt99140_setRegister(0x3369, 0x28);
sensor_nt99140_setRegister(0x336B, 0x20);
sensor_nt99140_setRegister(0x336D, 0x00);
sensor_nt99140_setRegister(0x336E, 0x00);
sensor_nt99140_setRegister(0x3370, 0x00);
sensor_nt99140_setRegister(0x3371, 0x20);
sensor_nt99140_setRegister(0x3372, 0x2F);
sensor_nt99140_setRegister(0x3374, 0x3F);
sensor_nt99140_setRegister(0x3375, 0x18);
sensor_nt99140_setRegister(0x3376, 0x14);
sensor_nt99140_setRegister(0x3378, 0x10);
sensor_nt99140_setRegister(0x3379, 0x04);
sensor_nt99140_setRegister(0x337A, 0x06);
sensor_nt99140_setRegister(0x337C, 0x06);
sensor_nt99140_setRegister(0x33A0, 0x50);
sensor_nt99140_setRegister(0x33A1, 0x78);
sensor_nt99140_setRegister(0x33A2, 0x10);
sensor_nt99140_setRegister(0x33A3, 0x20);
sensor_nt99140_setRegister(0x33A4, 0x00);
sensor_nt99140_setRegister(0x33A5, 0x74);
sensor_nt99140_setRegister(0x33A7, 0x04);
sensor_nt99140_setRegister(0x33A8, 0x00);
sensor_nt99140_setRegister(0x33A9, 0x00);
sensor_nt99140_setRegister(0x33AA, 0x02);
sensor_nt99140_setRegister(0x33AC, 0x02);
sensor_nt99140_setRegister(0x33AD, 0x02);
sensor_nt99140_setRegister(0x33AE, 0x02);
sensor_nt99140_setRegister(0x33B0, 0x02);
sensor_nt99140_setRegister(0x33B1, 0x00);
sensor_nt99140_setRegister(0x33B4, 0x48);
sensor_nt99140_setRegister(0x33B5, 0x44);
sensor_nt99140_setRegister(0x33B6, 0xA0);
sensor_nt99140_setRegister(0x33B9, 0x03);
sensor_nt99140_setRegister(0x33BD, 0x00);
sensor_nt99140_setRegister(0x33BE, 0x08);
sensor_nt99140_setRegister(0x33BF, 0x10);
sensor_nt99140_setRegister(0x33C0, 0x01);
sensor_nt99140_setRegister(0x3700, 0x04);
sensor_nt99140_setRegister(0x3701, 0x0F);
sensor_nt99140_setRegister(0x3702, 0x1A);
sensor_nt99140_setRegister(0x3703, 0x32);
sensor_nt99140_setRegister(0x3704, 0x42);
sensor_nt99140_setRegister(0x3705, 0x51);
sensor_nt99140_setRegister(0x3706, 0x6B);
sensor_nt99140_setRegister(0x3707, 0x81);
sensor_nt99140_setRegister(0x3708, 0x94);
sensor_nt99140_setRegister(0x3709, 0xA6);
sensor_nt99140_setRegister(0x370A, 0xC2);
sensor_nt99140_setRegister(0x370B, 0xD4);
sensor_nt99140_setRegister(0x370C, 0xE0);
sensor_nt99140_setRegister(0x370D, 0xE7);
sensor_nt99140_setRegister(0x370E, 0xEF);
sensor_nt99140_setRegister(0x3710, 0x07);
sensor_nt99140_setRegister(0x371E, 0x02);
sensor_nt99140_setRegister(0x371F, 0x02);
sensor_nt99140_setRegister(0x3800, 0x00);
sensor_nt99140_setRegister(0x3813, 0x07);
sensor_nt99140_setRegister(0x3028, 0x02);


#else
    if (0)//(boIsNightMode == 0) 
    {
        sensor_nt99140_setRegister(0x32F0,0x01);
        sensor_nt99140_setRegister(0x3024,0x00);
        sensor_nt99140_setRegister(0x3028,0x05); 
        sensor_nt99140_setRegister(0x3029,0x02); 
        sensor_nt99140_setRegister(0x302a,0x00); 
        sensor_nt99140_setRegister(0x3336,0x14);
        sensor_nt99140_setRegister(0x3210,0x04);  //lsc
        sensor_nt99140_setRegister(0x3211,0x04);
        sensor_nt99140_setRegister(0x3212,0x04);
        sensor_nt99140_setRegister(0x3213,0x04);
        sensor_nt99140_setRegister(0x3214,0x04);
        sensor_nt99140_setRegister(0x3215,0x04);
        sensor_nt99140_setRegister(0x3216,0x04);
        sensor_nt99140_setRegister(0x3217,0x04);
        sensor_nt99140_setRegister(0x3218,0x04);
        sensor_nt99140_setRegister(0x3219,0x04);
        sensor_nt99140_setRegister(0x321A,0x04);
        sensor_nt99140_setRegister(0x321B,0x04);
        sensor_nt99140_setRegister(0x321C,0x04);
        sensor_nt99140_setRegister(0x321D,0x04);
        sensor_nt99140_setRegister(0x321E,0x04);
        sensor_nt99140_setRegister(0x321F,0x04);
        sensor_nt99140_setRegister(0x3230,0x1D);
        sensor_nt99140_setRegister(0x3231,0x00);
        sensor_nt99140_setRegister(0x3232,0x04);
        sensor_nt99140_setRegister(0x3233,0x30);    
        sensor_nt99140_setRegister(0x3234,0x42);
        sensor_nt99140_setRegister(0x3270,0x00);  //gamma 
        sensor_nt99140_setRegister(0x3271,0x0e);
        sensor_nt99140_setRegister(0x3272,0x1a);
        sensor_nt99140_setRegister(0x3273,0x31);
        sensor_nt99140_setRegister(0x3274,0x4B);
        sensor_nt99140_setRegister(0x3275,0x5D);
        sensor_nt99140_setRegister(0x3276,0x7E);
        sensor_nt99140_setRegister(0x3277,0x94);
        sensor_nt99140_setRegister(0x3278,0xA6);
        sensor_nt99140_setRegister(0x3279,0xB6);
        sensor_nt99140_setRegister(0x327A,0xcc);
        sensor_nt99140_setRegister(0x327B,0xde);
        sensor_nt99140_setRegister(0x327C,0xeb);
        sensor_nt99140_setRegister(0x327D,0xF6);
        sensor_nt99140_setRegister(0x327E,0xFF);
        //sensor_nt99140_setRegister(0x3302,0x00);  //CC
        //sensor_nt99140_setRegister(0x3303,0x3f);
        //sensor_nt99140_setRegister(0x3304,0x00);
        //sensor_nt99140_setRegister(0x3305,0x88);
        //sensor_nt99140_setRegister(0x3306,0x00);
        //sensor_nt99140_setRegister(0x3307,0x38);
        //sensor_nt99140_setRegister(0x3308,0x07);
        //sensor_nt99140_setRegister(0x3309,0xe0);
        //sensor_nt99140_setRegister(0x330a,0x06);
        //sensor_nt99140_setRegister(0x330b,0xad);
        //sensor_nt99140_setRegister(0x330c,0x01);
        //sensor_nt99140_setRegister(0x330d,0x74);
        //sensor_nt99140_setRegister(0x330e,0x00);
        //sensor_nt99140_setRegister(0x330f,0xf5);
        //sensor_nt99140_setRegister(0x3310,0x07);
        //sensor_nt99140_setRegister(0x3311,0x33);
        //sensor_nt99140_setRegister(0x3312,0x07);
        //sensor_nt99140_setRegister(0x3313,0xd8);

        //[CC: Saturation:130%]
        //sensor_nt99140_setRegister(0x3302,0x00);
        //sensor_nt99140_setRegister(0x3303,0x04);
        //sensor_nt99140_setRegister(0x3304,0x00);
        //sensor_nt99140_setRegister(0x3305,0xFF);
        //sensor_nt99140_setRegister(0x3306,0x07);
        //sensor_nt99140_setRegister(0x3307,0xFD);
        //sensor_nt99140_setRegister(0x3308,0x07);
        //sensor_nt99140_setRegister(0x3309,0xEE);
        //sensor_nt99140_setRegister(0x330A,0x06);
        //sensor_nt99140_setRegister(0x330B,0x65);
        //sensor_nt99140_setRegister(0x330C,0x01);
        //sensor_nt99140_setRegister(0x330D,0xAD);
        //sensor_nt99140_setRegister(0x330E,0x01);
        //sensor_nt99140_setRegister(0x330F,0x06);
        //sensor_nt99140_setRegister(0x3310,0x07);
        //sensor_nt99140_setRegister(0x3311,0x01);
        //sensor_nt99140_setRegister(0x3312,0x07);
        //sensor_nt99140_setRegister(0x3313,0xF9);

        //[CC: Saturation:100%]
        sensor_nt99140_setRegister(0x3302,0x00);
        sensor_nt99140_setRegister(0x3303,0x1C);
        sensor_nt99140_setRegister(0x3304,0x00);
        sensor_nt99140_setRegister(0x3305,0xC8);
        sensor_nt99140_setRegister(0x3306,0x00);
        sensor_nt99140_setRegister(0x3307,0x1C);
        sensor_nt99140_setRegister(0x3308,0x07);
        sensor_nt99140_setRegister(0x3309,0xE9);
        sensor_nt99140_setRegister(0x330A,0x06);
        sensor_nt99140_setRegister(0x330B,0xDF);
        sensor_nt99140_setRegister(0x330C,0x01);
        sensor_nt99140_setRegister(0x330D,0x38);
        sensor_nt99140_setRegister(0x330E,0x00);
        sensor_nt99140_setRegister(0x330F,0xC6);
        sensor_nt99140_setRegister(0x3310,0x07);
        sensor_nt99140_setRegister(0x3311,0x3F);
        sensor_nt99140_setRegister(0x3312,0x07);
        sensor_nt99140_setRegister(0x3313,0xFC);


        sensor_nt99140_setRegister(0x3326,0x03);
        sensor_nt99140_setRegister(0x3327,0x0a);
        sensor_nt99140_setRegister(0x3328,0x0a);
        sensor_nt99140_setRegister(0x3329,0x06);
        sensor_nt99140_setRegister(0x332a,0x06);
        sensor_nt99140_setRegister(0x332b,0x1c);
        sensor_nt99140_setRegister(0x332c,0x1c);
        sensor_nt99140_setRegister(0x332d,0x00);
        sensor_nt99140_setRegister(0x332e,0x1d);
        sensor_nt99140_setRegister(0x332f,0x1f);
        sensor_nt99140_setRegister(0x3040,0x04);  //ABLC
        sensor_nt99140_setRegister(0x3041,0x02);
        sensor_nt99140_setRegister(0x3042,0xFF);
        sensor_nt99140_setRegister(0x3043,0x14);
        sensor_nt99140_setRegister(0x3052,0xd0);
        sensor_nt99140_setRegister(0x3057,0x80);
        sensor_nt99140_setRegister(0x3058,0x00);
        sensor_nt99140_setRegister(0x3059,0x2F);
        sensor_nt99140_setRegister(0x305f,0x22);
        sensor_nt99140_setRegister(0x32b0,0x00);  //AE
        sensor_nt99140_setRegister(0x32b1,0x90);
        sensor_nt99140_setRegister(0x32BB,0x0b);
        sensor_nt99140_setRegister(0x32bd,0x10);
        sensor_nt99140_setRegister(0x32be,0x05);
        sensor_nt99140_setRegister(0x32bf,0x4a);
        sensor_nt99140_setRegister(0x32c0,0x40);
        sensor_nt99140_setRegister(0x32C3,0x08);
        sensor_nt99140_setRegister(0x32c5,0x24);
        sensor_nt99140_setRegister(0x32cd,0x01);
        sensor_nt99140_setRegister(0x32d3,0x00);
        sensor_nt99140_setRegister(0x32f6,0x0c);
        sensor_nt99140_setRegister(0x3118,0xF2);
        sensor_nt99140_setRegister(0x3119,0xF2);
        sensor_nt99140_setRegister(0x311A,0x13);
        sensor_nt99140_setRegister(0x3106,0x01);
        sensor_nt99140_setRegister(0x3108,0x55);
        sensor_nt99140_setRegister(0x3105,0x41);
        sensor_nt99140_setRegister(0x3112,0x21);
        sensor_nt99140_setRegister(0x3113,0x55);
        sensor_nt99140_setRegister(0x3114,0x05);
        sensor_nt99140_setRegister(0x3300,0x30);  //sharpness & denoise
        sensor_nt99140_setRegister(0x3301,0x80);
        sensor_nt99140_setRegister(0x3324,0x07);
        sensor_nt99140_setRegister(0x3320,0x18);
        sensor_nt99140_setRegister(0x3335,0x00);
        sensor_nt99140_setRegister(0x3200,0x3e);     
        sensor_nt99140_setRegister(0x3201,0x3f); 
        sensor_nt99140_setRegister(0x3331,0x03);
        sensor_nt99140_setRegister(0x3332,0x40);

        sensor_nt99140_setRegister(0x3343,0xc0);    //auto control  
        sensor_nt99140_setRegister(0x333b,0x14);
        sensor_nt99140_setRegister(0x333c,0x14);
        sensor_nt99140_setRegister(0x333d,0x18);
        sensor_nt99140_setRegister(0x333f,0x80);
        sensor_nt99140_setRegister(0x3340,0x78);
        sensor_nt99140_setRegister(0x3341,0x60);
        sensor_nt99140_setRegister(0x3344,0x28);
        sensor_nt99140_setRegister(0x3345,0x30);
        sensor_nt99140_setRegister(0x3346,0x3f);
        sensor_nt99140_setRegister(0x3348,0x30);
        sensor_nt99140_setRegister(0x3349,0x40);
        sensor_nt99140_setRegister(0x334a,0x30);
        sensor_nt99140_setRegister(0x334b,0x00);
        sensor_nt99140_setRegister(0x334d,0x15);
        sensor_nt99140_setRegister(0x329b,0x01);
        sensor_nt99140_setRegister(0x32a1,0x01);
        sensor_nt99140_setRegister(0x32a2,0x40);
        sensor_nt99140_setRegister(0x32a3,0x01);
        sensor_nt99140_setRegister(0x32a4,0xc0);
        sensor_nt99140_setRegister(0x32a5,0x01);
        sensor_nt99140_setRegister(0x32a6,0x40);
        sensor_nt99140_setRegister(0x32a7,0x02);
        sensor_nt99140_setRegister(0x32a8,0x10);
        sensor_nt99140_setRegister(0x32a9,0x11);
        sensor_nt99140_setRegister(0x3054,0x05);
    
        #if 1
        //[edge table] --- winson 2012-04-09
        sensor_nt99140_setRegister(0x305f, 0x33);
        //sensor_nt99140_setRegister(0x32c5, 0x24);
        sensor_nt99140_setRegister(0x32c5, 0x18);
        sensor_nt99140_setRegister(0x32f2, 0x8a);
        sensor_nt99140_setRegister(0x3401, 0x58);
        sensor_nt99140_setRegister(0x3348, 0x30);
        sensor_nt99140_setRegister(0x3108, 0xff);
        sensor_nt99140_setRegister(0x3300, 0x30);
        sensor_nt99140_setRegister(0x3301, 0x90);
        sensor_nt99140_setRegister(0x333b, 0x12);
        sensor_nt99140_setRegister(0x333c, 0x14);
        sensor_nt99140_setRegister(0x333d, 0x18);
        sensor_nt99140_setRegister(0x333f, 0x80);
        sensor_nt99140_setRegister(0x3340, 0x78);
        sensor_nt99140_setRegister(0x3341, 0x78);
        sensor_nt99140_setRegister(0x3344, 0x30);
        sensor_nt99140_setRegister(0x3345, 0x3f);
        sensor_nt99140_setRegister(0x3346, 0x3f);
        sensor_nt99140_setRegister(0x3331, 0x02);
        sensor_nt99140_setRegister(0x3332, 0x80);
        sensor_nt99140_setRegister(0x3326, 0x03);
        sensor_nt99140_setRegister(0x3327, 0x0a);
        sensor_nt99140_setRegister(0x3328, 0x0a);
        sensor_nt99140_setRegister(0x3329, 0x06);
        sensor_nt99140_setRegister(0x332a, 0x06);
        sensor_nt99140_setRegister(0x332b, 0x1c);
        sensor_nt99140_setRegister(0x332c, 0x1c);
        sensor_nt99140_setRegister(0x332d, 0x00);
        sensor_nt99140_setRegister(0x332e, 0x1d);
        sensor_nt99140_setRegister(0x332f, 0x1f);
        //sensor_nt99140_setRegister(0x3060, 0x01);
        //sensor_nt99140_setRegister(0x3021, 0x16);     // Enable Output now
    
        //NT99140_NightMode();
        #endif

    }
    else
    {
        sensor_nt99140_setRegister(0x32F0, 0x01);
        sensor_nt99140_setRegister(0x3028, 0x05);
        sensor_nt99140_setRegister(0x3029, 0x02);
        sensor_nt99140_setRegister(0x302a, 0x00);
        sensor_nt99140_setRegister(0x3336, 0x14);

        sensor_nt99140_setRegister(0x3069, 0x02);
        sensor_nt99140_setRegister(0x306a, 0x02);

        /*
        //[Gamma_12]
        sensor_nt99140_setRegister(0x3270, 0x10);
        sensor_nt99140_setRegister(0x3271, 0x18);
        sensor_nt99140_setRegister(0x3272, 0x20);
        sensor_nt99140_setRegister(0x3273, 0x38);
        sensor_nt99140_setRegister(0x3274, 0x4E);
        sensor_nt99140_setRegister(0x3275, 0x63);
        sensor_nt99140_setRegister(0x3276, 0x87);
        sensor_nt99140_setRegister(0x3277, 0xA2);
        sensor_nt99140_setRegister(0x3278, 0xB8);
        sensor_nt99140_setRegister(0x3279, 0xCA);
        sensor_nt99140_setRegister(0x327A, 0xE3);
        sensor_nt99140_setRegister(0x327B, 0xF0);
        sensor_nt99140_setRegister(0x327C, 0xF8);
        sensor_nt99140_setRegister(0x327D, 0xFD);
        sensor_nt99140_setRegister(0x327E, 0xFF);
        */
        
        //[Gamma PC-cam new2]
        sensor_nt99140_setRegister(0x3270, 0x00);
        sensor_nt99140_setRegister(0x3271, 0x08);
        sensor_nt99140_setRegister(0x3272, 0x18);
        sensor_nt99140_setRegister(0x3273, 0x38);
        sensor_nt99140_setRegister(0x3274, 0x4e);
        sensor_nt99140_setRegister(0x3275, 0x64);
        sensor_nt99140_setRegister(0x3276, 0x82);
        sensor_nt99140_setRegister(0x3277, 0x98);
        sensor_nt99140_setRegister(0x3278, 0xa9);
        sensor_nt99140_setRegister(0x3279, 0xb8);
        sensor_nt99140_setRegister(0x327A, 0xcf);
        sensor_nt99140_setRegister(0x327B, 0xe2);
        sensor_nt99140_setRegister(0x327C, 0xf0);
        sensor_nt99140_setRegister(0x327D, 0xfa);
        sensor_nt99140_setRegister(0x327E, 0xff);

        //[Color_calibration]
        sensor_nt99140_setRegister(0x3302, 0x00); 
        sensor_nt99140_setRegister(0x3303, 0x3C); 
        sensor_nt99140_setRegister(0x3304, 0x00); 
        sensor_nt99140_setRegister(0x3305, 0xA6); 
        sensor_nt99140_setRegister(0x3306, 0x00); 
        sensor_nt99140_setRegister(0x3307, 0x1C); 
        sensor_nt99140_setRegister(0x3308, 0x07); 
        sensor_nt99140_setRegister(0x3309, 0xE1); 
        sensor_nt99140_setRegister(0x330A, 0x06); 
        sensor_nt99140_setRegister(0x330B, 0x9B); 
        sensor_nt99140_setRegister(0x330C, 0x01); 
        sensor_nt99140_setRegister(0x330D, 0x84); 
        sensor_nt99140_setRegister(0x330E, 0x00); 
        sensor_nt99140_setRegister(0x330F, 0xEF); 
        sensor_nt99140_setRegister(0x3310, 0x07); 
        sensor_nt99140_setRegister(0x3311, 0x69); 
        sensor_nt99140_setRegister(0x3312, 0x07); 
        sensor_nt99140_setRegister(0x3313, 0xA9); 

        sensor_nt99140_setRegister(0x32D4, 0x02);       //0x04
        sensor_nt99140_setRegister(0x32D5, 0x04);       //0x08

        sensor_nt99140_setRegister(0x32DB, 0x90); 
        sensor_nt99140_setRegister(0x3300, 0x3f);   //0x20
        sensor_nt99140_setRegister(0x3301, 0xdc); 
        sensor_nt99140_setRegister(0x3024, 0x00); 
              
        sensor_nt99140_setRegister(0x3040, 0x04); 
           
        sensor_nt99140_setRegister(0x3041, 0x02); 
        sensor_nt99140_setRegister(0x3042, 0xFF); 
        sensor_nt99140_setRegister(0x3043, 0x14); 
        sensor_nt99140_setRegister(0x3052, 0xd0); 
        sensor_nt99140_setRegister(0x3057, 0x80); 
        sensor_nt99140_setRegister(0x3058, 0x00); 
        sensor_nt99140_setRegister(0x3059, 0x2F); 
        sensor_nt99140_setRegister(0x305f, 0x44); 
               
        sensor_nt99140_setRegister(0x32b0, 0x00); 
        sensor_nt99140_setRegister(0x32b1, 0xdc); 
        sensor_nt99140_setRegister(0x32b2, 0x00); 
        sensor_nt99140_setRegister(0x32b3, 0x80); 
        sensor_nt99140_setRegister(0x32b4, 0x00); 
        sensor_nt99140_setRegister(0x32b5, 0xb0); 
        sensor_nt99140_setRegister(0x32b6, 0xa9); 
             
        sensor_nt99140_setRegister(0x32BB, 0x0b); 
        sensor_nt99140_setRegister(0x32bd, 0x10); 
        sensor_nt99140_setRegister(0x32be, 0x05); 
        sensor_nt99140_setRegister(0x32bf, 0x4A); 
        sensor_nt99140_setRegister(0x32c0, 0x40); 
        sensor_nt99140_setRegister(0x32C3, 0x08); 
        sensor_nt99140_setRegister(0x32c5, 0x30); //0x21
        sensor_nt99140_setRegister(0x32c6, 0x0a); 
        sensor_nt99140_setRegister(0x32c7, 0x31); 
        sensor_nt99140_setRegister(0x32cd, 0x02); 
        sensor_nt99140_setRegister(0x32d3, 0x00); 
        sensor_nt99140_setRegister(0x32f6, 0x0c); 
        sensor_nt99140_setRegister(0x3324, 0x00); 
        sensor_nt99140_setRegister(0x3320, 0x28); //0x15    //0x30 
        sensor_nt99140_setRegister(0x3335, 0x01); 
        sensor_nt99140_setRegister(0x3336, 0x10); 
        sensor_nt99140_setRegister(0x3337, 0x18); 
        sensor_nt99140_setRegister(0x3118, 0xF2); 
        sensor_nt99140_setRegister(0x3119, 0xF2); 
        sensor_nt99140_setRegister(0x311A, 0x13); 
        sensor_nt99140_setRegister(0x3106, 0x01); 
        sensor_nt99140_setRegister(0x3108, 0x55); 
        sensor_nt99140_setRegister(0x3105, 0x41); 
        sensor_nt99140_setRegister(0x3112, 0x21); 
        sensor_nt99140_setRegister(0x3113, 0x55); 
        sensor_nt99140_setRegister(0x3114, 0x05); 

        //[Auto_Ctrl]
        sensor_nt99140_setRegister(0x3343, 0x60);  //0xc0
        sensor_nt99140_setRegister(0x333b, 0x20);
        sensor_nt99140_setRegister(0x333c, 0x28);
        sensor_nt99140_setRegister(0x333d, 0x38);
        sensor_nt99140_setRegister(0x333f, 0x80);
        sensor_nt99140_setRegister(0x3340, 0x80);
        sensor_nt99140_setRegister(0x3341, 0x60);

        sensor_nt99140_setRegister(0x3344, 0x18);
        sensor_nt99140_setRegister(0x3345, 0x2d);
        sensor_nt99140_setRegister(0x3346, 0x30);
        sensor_nt99140_setRegister(0x3348, 0x38);
        sensor_nt99140_setRegister(0x3349, 0x40);
        sensor_nt99140_setRegister(0x334a, 0x30);
        sensor_nt99140_setRegister(0x334b, 0x00);
        sensor_nt99140_setRegister(0x334d, 0x15);

        sensor_nt99140_setRegister(0x32bc, 0x36);     //0x38
        
        //;AE/WB Ini
        sensor_nt99140_setRegister(0x3012, 0x01);
        sensor_nt99140_setRegister(0x3013, 0x80);
                  
        sensor_nt99140_setRegister(0x3290, 0x01);
        sensor_nt99140_setRegister(0x3291, 0xA5);
        sensor_nt99140_setRegister(0x3296, 0x01);
        sensor_nt99140_setRegister(0x3297, 0x4C);

        //;WB limit
        sensor_nt99140_setRegister(0x329b, 0x01);
        sensor_nt99140_setRegister(0x32a1, 0x01);
        sensor_nt99140_setRegister(0x32a2, 0x40);
        sensor_nt99140_setRegister(0x32a3, 0x01);
        sensor_nt99140_setRegister(0x32a4, 0xc0);
        sensor_nt99140_setRegister(0x32a5, 0x01);
        sensor_nt99140_setRegister(0x32a6, 0x40);
        sensor_nt99140_setRegister(0x32a7, 0x02);
        sensor_nt99140_setRegister(0x32a8, 0x10);
        sensor_nt99140_setRegister(0x32a9, 0x11);
        sensor_nt99140_setRegister(0x3054, 0x04);

        sensor_nt99140_setRegister(0x3300, 0x30);    //0x3f     //0x20        
        sensor_nt99140_setRegister(0x3301, 0xbc);       
        sensor_nt99140_setRegister(0x3331, 0x04);
        sensor_nt99140_setRegister(0x3332, 0x40);  
        sensor_nt99140_setRegister(0x3200, 0x3f);  
        sensor_nt99140_setRegister(0x3201, 0x3f);  
    }
#endif
}

static void NT99140_NightMode()
{
    sensor_nt99140_setRegister(0x3300, 0x30);
    sensor_nt99140_setRegister(0x3301, 0x90);
    //[Gamma]
    sensor_nt99140_setRegister(0x3270, 0x00); 
    sensor_nt99140_setRegister(0x3271, 0x0f);
    sensor_nt99140_setRegister(0x3272, 0x1e);
    sensor_nt99140_setRegister(0x3273, 0x31);
    sensor_nt99140_setRegister(0x3274, 0x4B);
    sensor_nt99140_setRegister(0x3275, 0x5D);
    sensor_nt99140_setRegister(0x3276, 0x7E);
    sensor_nt99140_setRegister(0x3277, 0x94);
    sensor_nt99140_setRegister(0x3278, 0xA6);
    sensor_nt99140_setRegister(0x3279, 0xB6);
    sensor_nt99140_setRegister(0x327A, 0xcc);
    sensor_nt99140_setRegister(0x327B, 0xde);
    sensor_nt99140_setRegister(0x327C, 0xeb);
    sensor_nt99140_setRegister(0x327D, 0xF6);
    sensor_nt99140_setRegister(0x327E, 0xFF);
    
    //[Color_calibration]
    sensor_nt99140_setRegister(0x3302, 0x00);
    sensor_nt99140_setRegister(0x3303, 0x3C);
    sensor_nt99140_setRegister(0x3304, 0x00);
    sensor_nt99140_setRegister(0x3305, 0xA6);
    sensor_nt99140_setRegister(0x3306, 0x00);
    sensor_nt99140_setRegister(0x3307, 0x1C);
    sensor_nt99140_setRegister(0x3308, 0x07);
    sensor_nt99140_setRegister(0x3309, 0xE1);
    sensor_nt99140_setRegister(0x330A, 0x06);
    sensor_nt99140_setRegister(0x330B, 0x9B);
    sensor_nt99140_setRegister(0x330C, 0x01);
    sensor_nt99140_setRegister(0x330D, 0x84);
    sensor_nt99140_setRegister(0x330E, 0x00);
    sensor_nt99140_setRegister(0x330F, 0xEF);
    sensor_nt99140_setRegister(0x3310, 0x07);
    sensor_nt99140_setRegister(0x3311, 0x69);
    sensor_nt99140_setRegister(0x3312, 0x07);
    sensor_nt99140_setRegister(0x3313, 0xA9);
    
    sensor_nt99140_setRegister(0x32c5, 0x28); //max gain
    sensor_nt99140_setRegister(0x32bb, 0x1b); 
    //SensorSetReg_NT99140(0x3025, 0x00); //Test pattern: disable

}


static void NT99140_set_176x144(void)
{
    NT99140_Initial_Setting(); 
    sensor_nt99140_setRegister(0x32e0, 0x00); 
    sensor_nt99140_setRegister(0x32e1, 0xb0); 
    sensor_nt99140_setRegister(0x32e2, 0x00); 
    sensor_nt99140_setRegister(0x32e3, 0x90); 
    sensor_nt99140_setRegister(0x32e4, 0x04); 
    sensor_nt99140_setRegister(0x32e5, 0x07); 
    sensor_nt99140_setRegister(0x32e6, 0x04); 
    sensor_nt99140_setRegister(0x32e7, 0x08); 
    sensor_nt99140_setRegister(0x3028, 0x15); 
    sensor_nt99140_setRegister(0x3029, 0x00); 
    sensor_nt99140_setRegister(0x302a, 0x04); 
    sensor_nt99140_setRegister(0x3022, 0x24); 
    sensor_nt99140_setRegister(0x3023, 0x24); 
    sensor_nt99140_setRegister(0x3002, 0x00); 
    sensor_nt99140_setRegister(0x3003, 0xcc); 
    sensor_nt99140_setRegister(0x3004, 0x00); 
    sensor_nt99140_setRegister(0x3005, 0x04); 
    sensor_nt99140_setRegister(0x3006, 0x04); 
    sensor_nt99140_setRegister(0x3007, 0x3b); 
    sensor_nt99140_setRegister(0x3008, 0x02); 
    sensor_nt99140_setRegister(0x3009, 0xd3); 
    sensor_nt99140_setRegister(0x300a, 0x05); 
    sensor_nt99140_setRegister(0x300b, 0x5f); 
    sensor_nt99140_setRegister(0x300c, 0x03); 
    sensor_nt99140_setRegister(0x300d, 0x20); 
    sensor_nt99140_setRegister(0x300e, 0x03); 
    sensor_nt99140_setRegister(0x300f, 0x70); 
    sensor_nt99140_setRegister(0x3010, 0x02); 
    sensor_nt99140_setRegister(0x3011, 0xd0); 
    sensor_nt99140_setRegister(0x32b0, 0x00); 
    sensor_nt99140_setRegister(0x32b1, 0x00); 
    sensor_nt99140_setRegister(0x32b2, 0x00); 
    sensor_nt99140_setRegister(0x32b3, 0xb4); 
    sensor_nt99140_setRegister(0x32b4, 0x00); 
    sensor_nt99140_setRegister(0x32b5, 0x64); 
    sensor_nt99140_setRegister(0x32b6, 0x99); 
    sensor_nt99140_setRegister(0x32bb, 0x1b); 
    sensor_nt99140_setRegister(0x32bc, 0x40); 
    sensor_nt99140_setRegister(0x32c1, 0x03); 
    sensor_nt99140_setRegister(0x32c2, 0xc0); 
    sensor_nt99140_setRegister(0x32c8, 0x78); 
    sensor_nt99140_setRegister(0x32c9, 0x64); 
    sensor_nt99140_setRegister(0x32c4, 0x00); 
    sensor_nt99140_setRegister(0x3201, 0x7f); 
    sensor_nt99140_setRegister(0x3021, 0x06); 
    sensor_nt99140_setRegister(0x3060, 0x01); 
}

static void NT99140_setQVGA_320x240_30FPS(void)
{
    NT99140_Initial_Setting(); 
    sensor_nt99140_setRegister(0x32e0, 0x01); 
    sensor_nt99140_setRegister(0x32e1, 0x40); 
    sensor_nt99140_setRegister(0x32e2, 0x00); 
    sensor_nt99140_setRegister(0x32e3, 0xf0); 
    sensor_nt99140_setRegister(0x32e4, 0x02); 
    sensor_nt99140_setRegister(0x32e5, 0x02); 
    sensor_nt99140_setRegister(0x32e6, 0x02); 
    sensor_nt99140_setRegister(0x32e7, 0x03); 
    sensor_nt99140_setRegister(0x3028, 0x15); 
    sensor_nt99140_setRegister(0x3029, 0x00); 
    sensor_nt99140_setRegister(0x302a, 0x04); 
    sensor_nt99140_setRegister(0x3022, 0x24); 
    sensor_nt99140_setRegister(0x3023, 0x24); 
    sensor_nt99140_setRegister(0x3002, 0x00); 
    sensor_nt99140_setRegister(0x3003, 0xa4); 
    sensor_nt99140_setRegister(0x3004, 0x00); 
    sensor_nt99140_setRegister(0x3005, 0x04); 
    sensor_nt99140_setRegister(0x3006, 0x04); 
    sensor_nt99140_setRegister(0x3007, 0x63); 
    sensor_nt99140_setRegister(0x3008, 0x02); 
    sensor_nt99140_setRegister(0x3009, 0xd3); 
    sensor_nt99140_setRegister(0x300a, 0x05); 
    sensor_nt99140_setRegister(0x300b, 0x5f); 
    sensor_nt99140_setRegister(0x300c, 0x03); 
    sensor_nt99140_setRegister(0x300d, 0x20); 
    sensor_nt99140_setRegister(0x300e, 0x03); 
    sensor_nt99140_setRegister(0x300f, 0xc0); 
    sensor_nt99140_setRegister(0x3010, 0x02); 
    sensor_nt99140_setRegister(0x3011, 0xd0); 
    sensor_nt99140_setRegister(0x32b0, 0x00); 
    sensor_nt99140_setRegister(0x32b1, 0x00); 
    sensor_nt99140_setRegister(0x32b2, 0x00); 
    sensor_nt99140_setRegister(0x32b3, 0xdc); 
    sensor_nt99140_setRegister(0x32b4, 0x00); 
    sensor_nt99140_setRegister(0x32b5, 0x64); 
    sensor_nt99140_setRegister(0x32b6, 0x99); 
    sensor_nt99140_setRegister(0x32bb, 0x1b); 
    sensor_nt99140_setRegister(0x32bc, 0x40); 
    sensor_nt99140_setRegister(0x32c1, 0x03); 
    sensor_nt99140_setRegister(0x32c2, 0xc0); 
    sensor_nt99140_setRegister(0x32c8, 0x78); 
    sensor_nt99140_setRegister(0x32c9, 0x64); 
    sensor_nt99140_setRegister(0x32c4, 0x00); 
    sensor_nt99140_setRegister(0x3201, 0x7f); 
    sensor_nt99140_setRegister(0x3021, 0x06); 
    sensor_nt99140_setRegister(0x3060, 0x01);  
}

static void NT99140_setVGA_640x480_30FPS(void)
{
    MP_DEBUG1("---## %s ##---", __FUNCTION__);
	NT99140_setVGA_640x480();
	return;
}

static void NT99140_setSVGA_800x480(void)
{
    /*for capture preview (Initial setting)*/
    MP_DEBUG1("---## %s ##---", __FUNCTION__);
	NT99140_Initial_Setting();

#if 1//!SHOW_CENTER
	sensor_nt99140_setRegister(0x32F1, 0x01);//BW 
#endif
#if 1 	 
//MCLK:      12.00MHz 
//PCLK:      48.00MHz 
//Size:      800x480 
//FPS:       20.00~20.02 
//Line:      1586 
//Frame:      756 
//Flicker:   50Hz 
			sensor_nt99140_setRegister(0x32BB, 0x67);  //AE Start
			sensor_nt99140_setRegister(0x32BF, 0x60); 
			sensor_nt99140_setRegister(0x32C0, 0x64); 
			sensor_nt99140_setRegister(0x32C1, 0x64); 
			sensor_nt99140_setRegister(0x32C2, 0x64); 
			sensor_nt99140_setRegister(0x32C3, 0x00); 
			sensor_nt99140_setRegister(0x32C4, 0x30); 
			sensor_nt99140_setRegister(0x32C5, 0x30); 
			sensor_nt99140_setRegister(0x32C6, 0x30); 
			sensor_nt99140_setRegister(0x32D3, 0x00); 
			sensor_nt99140_setRegister(0x32D4, 0x97); 
			sensor_nt99140_setRegister(0x32D5, 0x72); 
			sensor_nt99140_setRegister(0x32D6, 0x00); 
			sensor_nt99140_setRegister(0x32D7, 0x7E); 
			sensor_nt99140_setRegister(0x32D8, 0x6F);  //AE End
			sensor_nt99140_setRegister(0x32E0, 0x03);  //Scale Start
			sensor_nt99140_setRegister(0x32E1, 0x20); 
			sensor_nt99140_setRegister(0x32E2, 0x01); 
			sensor_nt99140_setRegister(0x32E3, 0xE0); 
			sensor_nt99140_setRegister(0x32E4, 0x00); 
			sensor_nt99140_setRegister(0x32E5, 0x9A); 
			sensor_nt99140_setRegister(0x32E6, 0x00); 
			sensor_nt99140_setRegister(0x32E7, 0x80);  //Scale End
			sensor_nt99140_setRegister(0x32F0, 0x01);  //Output Format
			sensor_nt99140_setRegister(0x3200, 0x7E);  //Mode Control
			sensor_nt99140_setRegister(0x3201, 0x7D);  //Mode Control
			sensor_nt99140_setRegister(0x302A, 0x80);  //PLL Start
			sensor_nt99140_setRegister(0x302C, 0x0F); 
			sensor_nt99140_setRegister(0x302D, 0x01);  //PLL End
			sensor_nt99140_setRegister(0x3022, 0x00);  //Timing Start
			sensor_nt99140_setRegister(0x300A, 0x06); 
			sensor_nt99140_setRegister(0x300B, 0x32); 
			sensor_nt99140_setRegister(0x300C, 0x02); 
			sensor_nt99140_setRegister(0x300D, 0xF4);  //Timing End
			sensor_nt99140_setRegister(0x320A, 0x4D); 
			sensor_nt99140_setRegister(0x3021, 0x02); 
			sensor_nt99140_setRegister(0x3060, 0x01); 

#else
//MCLK:      12.00MHz 
//PCLK:      72.00MHz 
//Size:      800x480 
//FPS:       30.00~30.02 
//Line:      1586 
//Frame:      756 
//Flicker:   50Hz 
			sensor_nt99140_setRegister(0x32BB, 0x67);  //AE Start
			sensor_nt99140_setRegister(0x32BF, 0x60); 
			sensor_nt99140_setRegister(0x32C0, 0x5A); 
			sensor_nt99140_setRegister(0x32C1, 0x5A); 
			sensor_nt99140_setRegister(0x32C2, 0x5A); 
			sensor_nt99140_setRegister(0x32C3, 0x00); 
			sensor_nt99140_setRegister(0x32C4, 0x30); 
			sensor_nt99140_setRegister(0x32C5, 0x30); 
			sensor_nt99140_setRegister(0x32C6, 0x30); 
			sensor_nt99140_setRegister(0x32D3, 0x00); 
			sensor_nt99140_setRegister(0x32D4, 0xE3); 
			sensor_nt99140_setRegister(0x32D5, 0x7C); 
			sensor_nt99140_setRegister(0x32D6, 0x00); 
			sensor_nt99140_setRegister(0x32D7, 0xBC); 
			sensor_nt99140_setRegister(0x32D8, 0x77);  //AE End
			sensor_nt99140_setRegister(0x32E0, 0x03);  //Scale Start
			sensor_nt99140_setRegister(0x32E1, 0x20); 
			sensor_nt99140_setRegister(0x32E2, 0x01); 
			sensor_nt99140_setRegister(0x32E3, 0xE0); 
			sensor_nt99140_setRegister(0x32E4, 0x00); 
			sensor_nt99140_setRegister(0x32E5, 0x9A); 
			sensor_nt99140_setRegister(0x32E6, 0x00); 
			sensor_nt99140_setRegister(0x32E7, 0x80);  //Scale End
			sensor_nt99140_setRegister(0x32F0, 0x01);  //Output Format
			sensor_nt99140_setRegister(0x3200, 0x7E);  //Mode Control
			sensor_nt99140_setRegister(0x3201, 0x7D);  //Mode Control
			sensor_nt99140_setRegister(0x302A, 0x80);  //PLL Start
			sensor_nt99140_setRegister(0x302C, 0x17); 
			sensor_nt99140_setRegister(0x302D, 0x01);  //PLL End
			sensor_nt99140_setRegister(0x3022, 0x00);  //Timing Start
			sensor_nt99140_setRegister(0x300A, 0x06); 
			sensor_nt99140_setRegister(0x300B, 0x32); 
			sensor_nt99140_setRegister(0x300C, 0x02); 
			sensor_nt99140_setRegister(0x300D, 0xF4);  //Timing End
			sensor_nt99140_setRegister(0x320A, 0x4D); 
			sensor_nt99140_setRegister(0x3021, 0x02); 
			sensor_nt99140_setRegister(0x3060, 0x01); 
#endif
}

static void NT99140_setSVGA_800x600(void)
{
    /*for capture preview (Initial setting)*/
    MP_DEBUG1("---## %s ##---", __FUNCTION__);
	NT99140_Initial_Setting();

#if 1//!SHOW_CENTER
	sensor_nt99140_setRegister(0x32F1, 0x01);//BW 
#endif

//MCLK:      12.00MHz 
//PCLK:      48.00MHz 
//Size:      800x600 
//FPS:       20.18~20.18 
//Line:      1586 
//Frame:      750 
//Flicker:   50Hz 
sensor_nt99140_setRegister(0x32BB, 0x67);  //AE Start
sensor_nt99140_setRegister(0x32BF, 0x60); 
sensor_nt99140_setRegister(0x32C0, 0x5A); 
sensor_nt99140_setRegister(0x32C1, 0x5A); 
sensor_nt99140_setRegister(0x32C2, 0x5A); 
sensor_nt99140_setRegister(0x32C3, 0x00); 
sensor_nt99140_setRegister(0x32C4, 0x30); 
sensor_nt99140_setRegister(0x32C5, 0x30); 
sensor_nt99140_setRegister(0x32C6, 0x30); 
sensor_nt99140_setRegister(0x32D3, 0x00); 
sensor_nt99140_setRegister(0x32D4, 0x97); 
sensor_nt99140_setRegister(0x32D5, 0x72); 
sensor_nt99140_setRegister(0x32D6, 0x00); 
sensor_nt99140_setRegister(0x32D7, 0x7E); 
sensor_nt99140_setRegister(0x32D8, 0x6F);  //AE End
sensor_nt99140_setRegister(0x32E0, 0x03);  //Scale Start
sensor_nt99140_setRegister(0x32E1, 0x20); 
sensor_nt99140_setRegister(0x32E2, 0x02); 
sensor_nt99140_setRegister(0x32E3, 0x58); 
sensor_nt99140_setRegister(0x32E4, 0x00); 
sensor_nt99140_setRegister(0x32E5, 0x33); 
sensor_nt99140_setRegister(0x32E6, 0x00); 
sensor_nt99140_setRegister(0x32E7, 0x33);  //Scale End
sensor_nt99140_setRegister(0x32F0, 0x01);  //Output Format
sensor_nt99140_setRegister(0x3200, 0x7E);  //Mode Control
sensor_nt99140_setRegister(0x3201, 0x7D);  //Mode Control
sensor_nt99140_setRegister(0x302A, 0x80);  //PLL Start
sensor_nt99140_setRegister(0x302C, 0x0F); 
sensor_nt99140_setRegister(0x302D, 0x01);  //PLL End
sensor_nt99140_setRegister(0x3022, 0x00);  //Timing Start
sensor_nt99140_setRegister(0x300A, 0x06); 
sensor_nt99140_setRegister(0x300B, 0x32); 
sensor_nt99140_setRegister(0x300C, 0x02); 
sensor_nt99140_setRegister(0x300D, 0xEE);  //Timing End
sensor_nt99140_setRegister(0x320A, 0x24); 
sensor_nt99140_setRegister(0x3021, 0x02); 
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
#if 0

//MCLK:      12.00MHz 
//PCLK:      36.00MHz 
//Size:      640x480 
//FPS:       10.00~10.01 
//Line:      1586 
//Frame:     1134 
//Flicker:   50Hz 
sensor_nt99140_setRegister(0x32BB, 0x67);  //AE Start
sensor_nt99140_setRegister(0x32BF, 0x60); 
sensor_nt99140_setRegister(0x32C0, 0x74); 
sensor_nt99140_setRegister(0x32C1, 0x74); 
sensor_nt99140_setRegister(0x32C2, 0x74); 
sensor_nt99140_setRegister(0x32C3, 0x00); 
sensor_nt99140_setRegister(0x32C4, 0x2B); 
sensor_nt99140_setRegister(0x32C5, 0x30); 
sensor_nt99140_setRegister(0x32C6, 0x30); 
sensor_nt99140_setRegister(0x32D3, 0x00); 
sensor_nt99140_setRegister(0x32D4, 0x71); 
sensor_nt99140_setRegister(0x32D5, 0x6C); 
sensor_nt99140_setRegister(0x32D6, 0x00); 
sensor_nt99140_setRegister(0x32D7, 0x5F); 
sensor_nt99140_setRegister(0x32D8, 0x67);  //AE End
sensor_nt99140_setRegister(0x32E0, 0x02);  //Scale Start
sensor_nt99140_setRegister(0x32E1, 0x80); 
sensor_nt99140_setRegister(0x32E2, 0x01); 
sensor_nt99140_setRegister(0x32E3, 0xE0); 
sensor_nt99140_setRegister(0x32E4, 0x00); 
sensor_nt99140_setRegister(0x32E5, 0x80); 
sensor_nt99140_setRegister(0x32E6, 0x00); 
sensor_nt99140_setRegister(0x32E7, 0x80);  //Scale End
sensor_nt99140_setRegister(0x32F0, 0x11);  //Output Format
sensor_nt99140_setRegister(0x3200, 0x7E);  //Mode Control
sensor_nt99140_setRegister(0x3201, 0x7D);  //Mode Control
sensor_nt99140_setRegister(0x302A, 0x80);  //PLL Start
sensor_nt99140_setRegister(0x302C, 0x11); 
sensor_nt99140_setRegister(0x302D, 0x02);  //PLL End
sensor_nt99140_setRegister(0x3022, 0x00);  //Timing Start
sensor_nt99140_setRegister(0x300A, 0x06); 
sensor_nt99140_setRegister(0x300B, 0x32); 
sensor_nt99140_setRegister(0x300C, 0x04); 
sensor_nt99140_setRegister(0x300D, 0x6E);  //Timing End
sensor_nt99140_setRegister(0x320A, 0x38); 
sensor_nt99140_setRegister(0x3101, 0x00); 
sensor_nt99140_setRegister(0x302A, 0x80); 
sensor_nt99140_setRegister(0x3025, 0x02); 
sensor_nt99140_setRegister(0x3025, 0x01); 
sensor_nt99140_setRegister(0x3021, 0x02);
sensor_nt99140_setRegister(0x3060, 0x01); 
sensor_nt99140_setRegister(0x3025, 0x00); 

#elif 0
//MCLK:      12.00MHz 
//PCLK:      48.00MHz 
//Size:      640x480 
//FPS:       20.00~20.02 
//Line:      1586 
//Frame:      756 
//Flicker:   50Hz 
			sensor_nt99140_setRegister(0x32BB, 0x67);  //AE Start
			sensor_nt99140_setRegister(0x32BF, 0x60); 
			sensor_nt99140_setRegister(0x32C0, 0x64); 
			sensor_nt99140_setRegister(0x32C1, 0x64); 
			sensor_nt99140_setRegister(0x32C2, 0x64); 
			sensor_nt99140_setRegister(0x32C3, 0x00); 
			sensor_nt99140_setRegister(0x32C4, 0x2B); 
			sensor_nt99140_setRegister(0x32C5, 0x30); 
			sensor_nt99140_setRegister(0x32C6, 0x30); 
			sensor_nt99140_setRegister(0x32D3, 0x00); 
			sensor_nt99140_setRegister(0x32D4, 0x97); 
			sensor_nt99140_setRegister(0x32D5, 0x72); 
			sensor_nt99140_setRegister(0x32D6, 0x00); 
			sensor_nt99140_setRegister(0x32D7, 0x7E); 
			sensor_nt99140_setRegister(0x32D8, 0x6F);  //AE End
			sensor_nt99140_setRegister(0x32E0, 0x02);  //Scale Start
			sensor_nt99140_setRegister(0x32E1, 0x80); 
			sensor_nt99140_setRegister(0x32E2, 0x01); 
			sensor_nt99140_setRegister(0x32E3, 0xE0); 
			sensor_nt99140_setRegister(0x32E4, 0x00); 
			sensor_nt99140_setRegister(0x32E5, 0x80); 
			sensor_nt99140_setRegister(0x32E6, 0x00); 
			sensor_nt99140_setRegister(0x32E7, 0x80);  //Scale End
			sensor_nt99140_setRegister(0x32F0, 0x11);  //Output Format
			sensor_nt99140_setRegister(0x3200, 0x7E);  //Mode Control
			sensor_nt99140_setRegister(0x3201, 0x7D);  //Mode Control
			sensor_nt99140_setRegister(0x302A, 0x80);  //PLL Start
			sensor_nt99140_setRegister(0x302C, 0x0F); 
			sensor_nt99140_setRegister(0x302D, 0x01);  //PLL End
			sensor_nt99140_setRegister(0x3022, 0x00);  //Timing Start
			sensor_nt99140_setRegister(0x300A, 0x06); 
			sensor_nt99140_setRegister(0x300B, 0x32); 
			sensor_nt99140_setRegister(0x300C, 0x02); 
			sensor_nt99140_setRegister(0x300D, 0xF4);  //Timing End
			sensor_nt99140_setRegister(0x320A, 0x38); 
			sensor_nt99140_setRegister(0x3101, 0x00); 
			sensor_nt99140_setRegister(0x302A, 0x80); 
			sensor_nt99140_setRegister(0x3025, 0x02); 
			sensor_nt99140_setRegister(0x3025, 0x01); 
			sensor_nt99140_setRegister(0x3021, 0x02);
			sensor_nt99140_setRegister(0x3060, 0x01); 
			sensor_nt99140_setRegister(0x3025, 0x00); 


#else
//MCLK:      12.00MHz 
//PCLK:      72.00MHz 
//Size:      640x480 
//FPS:       30.00~30.02 
//Line:      1586 
//Frame:      756 
//Flicker:   50Hz 
			sensor_nt99140_setRegister(0x32BB, 0x67);  //AE Start
			sensor_nt99140_setRegister(0x32BF, 0x60); 
			sensor_nt99140_setRegister(0x32C0, 0x5A); 
			sensor_nt99140_setRegister(0x32C1, 0x5A); 
			sensor_nt99140_setRegister(0x32C2, 0x5A); 
			sensor_nt99140_setRegister(0x32C3, 0x00); 
			sensor_nt99140_setRegister(0x32C4, 0x2B); 
			sensor_nt99140_setRegister(0x32C5, 0x30); 
			sensor_nt99140_setRegister(0x32C6, 0x30); 
			sensor_nt99140_setRegister(0x32D3, 0x00); 
			sensor_nt99140_setRegister(0x32D4, 0xE3); 
			sensor_nt99140_setRegister(0x32D5, 0x7C); 
			sensor_nt99140_setRegister(0x32D6, 0x00); 
			sensor_nt99140_setRegister(0x32D7, 0xBC); 
			sensor_nt99140_setRegister(0x32D8, 0x77);  //AE End
			sensor_nt99140_setRegister(0x32E0, 0x02);  //Scale Start
			sensor_nt99140_setRegister(0x32E1, 0x80); 
			sensor_nt99140_setRegister(0x32E2, 0x01); 
			sensor_nt99140_setRegister(0x32E3, 0xE0); 
			sensor_nt99140_setRegister(0x32E4, 0x00); 
			sensor_nt99140_setRegister(0x32E5, 0x80); 
			sensor_nt99140_setRegister(0x32E6, 0x00); 
			sensor_nt99140_setRegister(0x32E7, 0x80);  //Scale End
			sensor_nt99140_setRegister(0x32F0, 0x11);  //Output Format
			sensor_nt99140_setRegister(0x3200, 0x7E);  //Mode Control
			sensor_nt99140_setRegister(0x3201, 0x7D);  //Mode Control
			sensor_nt99140_setRegister(0x302A, 0x80);  //PLL Start
			sensor_nt99140_setRegister(0x302C, 0x17); 
			sensor_nt99140_setRegister(0x302D, 0x01);  //PLL End
			sensor_nt99140_setRegister(0x3022, 0x00);  //Timing Start
			sensor_nt99140_setRegister(0x300A, 0x06); 
			sensor_nt99140_setRegister(0x300B, 0x32); 
			sensor_nt99140_setRegister(0x300C, 0x02); 
			sensor_nt99140_setRegister(0x300D, 0xF4);  //Timing End
			sensor_nt99140_setRegister(0x320A, 0x38); 
			sensor_nt99140_setRegister(0x3101, 0x00); 
			sensor_nt99140_setRegister(0x302A, 0x80); 
			sensor_nt99140_setRegister(0x3025, 0x02); 
			sensor_nt99140_setRegister(0x3025, 0x01); 
			sensor_nt99140_setRegister(0x3021, 0x02);
			sensor_nt99140_setRegister(0x3060, 0x01); 
			sensor_nt99140_setRegister(0x3025, 0x00); 
#endif

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


static void NT99140_set_720P_1280x720(void)
{
    NT99140_Initial_Setting(); 

#if 1//!SHOW_CENTER
	sensor_nt99140_setRegister(0x32F1, 0x01);//BW 
#endif

#if 0    
    if (boIsNightMode == 0) 
    {
        sensor_nt99140_setRegister(0x3028, 0x15); 
        sensor_nt99140_setRegister(0x3029, 0x00); 
        sensor_nt99140_setRegister(0x302a, 0x04); 
        sensor_nt99140_setRegister(0x3022, 0x24); 
        sensor_nt99140_setRegister(0x3023, 0x24); 
        sensor_nt99140_setRegister(0x3002, 0x00); 
        sensor_nt99140_setRegister(0x3003, 0x04); 
        sensor_nt99140_setRegister(0x3004, 0x00); 
        sensor_nt99140_setRegister(0x3005, 0x04); 
        sensor_nt99140_setRegister(0x3006, 0x05); 
        sensor_nt99140_setRegister(0x3007, 0x03); 
        sensor_nt99140_setRegister(0x3008, 0x02); 
        sensor_nt99140_setRegister(0x3009, 0xd3); 
        sensor_nt99140_setRegister(0x300a, 0x06); 
        sensor_nt99140_setRegister(0x300b, 0xb7); 
        sensor_nt99140_setRegister(0x300c, 0x02); 
        sensor_nt99140_setRegister(0x300d, 0xff); 
        sensor_nt99140_setRegister(0x300e, 0x05); 
        sensor_nt99140_setRegister(0x300f, 0x00); 
        sensor_nt99140_setRegister(0x3010, 0x02); 
        sensor_nt99140_setRegister(0x3011, 0xd0); 
        sensor_nt99140_setRegister(0x32b0, 0x00); 
        sensor_nt99140_setRegister(0x32b1, 0x00); 
        sensor_nt99140_setRegister(0x32b2, 0x01); 
        sensor_nt99140_setRegister(0x32b3, 0x7c); 
        sensor_nt99140_setRegister(0x32b4, 0x00); 
        sensor_nt99140_setRegister(0x32b5, 0x64); 
        sensor_nt99140_setRegister(0x32b6, 0x99); 
        sensor_nt99140_setRegister(0x32bb, 0x1b); 
        sensor_nt99140_setRegister(0x32bc, 0x40); 
        sensor_nt99140_setRegister(0x32c1, 0x03); 
        sensor_nt99140_setRegister(0x32c2, 0x00); 
        sensor_nt99140_setRegister(0x32c8, 0x60); 
        sensor_nt99140_setRegister(0x32c9, 0x50); 
        sensor_nt99140_setRegister(0x32c4, 0x00); 
        sensor_nt99140_setRegister(0x3201, 0x3f); 
        sensor_nt99140_setRegister(0x3021, 0x16); 
        sensor_nt99140_setRegister(0x3060, 0x01);

        sensor_nt99140_setRegister(0x3320, 0x28);
    }
    else if (boIsNightMode == 1) 
    {
    //[1280x720_15_25_Fps]  MCLK_12M  PCLK_75M
		sensor_nt99140_setRegister(0x3300, 0x70);
		sensor_nt99140_setRegister(0x3301, 0x40);
		sensor_nt99140_setRegister(0x3331, 0x08);
		sensor_nt99140_setRegister(0x3332, 0x40);
		sensor_nt99140_setRegister(0x3200, 0x3e);      // Mode_Control_0
		sensor_nt99140_setRegister(0x3201, 0x3f);      // Mode_Control_1
		
		sensor_nt99140_setRegister(0x3028, 0x18); 
		sensor_nt99140_setRegister(0x3029, 0x00); 
		sensor_nt99140_setRegister(0x302a, 0x04); 
		sensor_nt99140_setRegister(0x3022, 0x24); 
		sensor_nt99140_setRegister(0x3023, 0x24); 
		sensor_nt99140_setRegister(0x3002, 0x00); 
		sensor_nt99140_setRegister(0x3003, 0x04); 
		sensor_nt99140_setRegister(0x3004, 0x00); 
		sensor_nt99140_setRegister(0x3005, 0x04); 
		sensor_nt99140_setRegister(0x3006, 0x05); 
		sensor_nt99140_setRegister(0x3007, 0x03); 
		sensor_nt99140_setRegister(0x3008, 0x02); 
		sensor_nt99140_setRegister(0x3009, 0xd3); 
		sensor_nt99140_setRegister(0x300a, 0x07); 
		sensor_nt99140_setRegister(0x300b, 0xd0); 
		sensor_nt99140_setRegister(0x300c, 0x02); 
		sensor_nt99140_setRegister(0x300d, 0xee); 
		sensor_nt99140_setRegister(0x300e, 0x05); 
		sensor_nt99140_setRegister(0x300f, 0x00); 
		sensor_nt99140_setRegister(0x3010, 0x02); 
		sensor_nt99140_setRegister(0x3011, 0xd0); 
		sensor_nt99140_setRegister(0x32b0, 0x00); 
		sensor_nt99140_setRegister(0x32b1, 0x00); 
		sensor_nt99140_setRegister(0x32b2, 0x01); 
		sensor_nt99140_setRegister(0x32b3, 0x7c); 
		sensor_nt99140_setRegister(0x32b4, 0x00); 
		sensor_nt99140_setRegister(0x32b5, 0x64); 
		sensor_nt99140_setRegister(0x32b6, 0x99); 
		sensor_nt99140_setRegister(0x32bb, 0x1b); 
		sensor_nt99140_setRegister(0x32bc, 0x40); 
		sensor_nt99140_setRegister(0x32c1, 0x24); 
		sensor_nt99140_setRegister(0x32c2, 0xe2); 
		sensor_nt99140_setRegister(0x32c8, 0x5e); 
		sensor_nt99140_setRegister(0x32c9, 0x4e); 
		sensor_nt99140_setRegister(0x32c4, 0x00); 
		sensor_nt99140_setRegister(0x3201, 0x3f); 
		sensor_nt99140_setRegister(0x3021, 0x16); 
		sensor_nt99140_setRegister(0x3060, 0x01); 

        sensor_nt99140_setRegister(0x3320, 0x32);
        
    }
    else
#endif
    {
        #if 0
        //1280x720_15fps_to_25fps_PCLK_68M_Tline_1830  
        sensor_nt99140_setRegister(0x3028, 0x21); 
        sensor_nt99140_setRegister(0x3029, 0x20); 
        sensor_nt99140_setRegister(0x302a, 0x04); 
        sensor_nt99140_setRegister(0x3022, 0x24); 
        sensor_nt99140_setRegister(0x3023, 0x24); 
        sensor_nt99140_setRegister(0x3002, 0x00); 
        sensor_nt99140_setRegister(0x3003, 0x04); 
        sensor_nt99140_setRegister(0x3004, 0x00); 
        sensor_nt99140_setRegister(0x3005, 0x04); 
        sensor_nt99140_setRegister(0x3006, 0x05); 
        sensor_nt99140_setRegister(0x3007, 0x03); 
        sensor_nt99140_setRegister(0x3008, 0x02); 
        sensor_nt99140_setRegister(0x3009, 0xd3); 
        sensor_nt99140_setRegister(0x300a, 0x07); 
        sensor_nt99140_setRegister(0x300b, 0x26); 
        sensor_nt99140_setRegister(0x300c, 0x02); 
        sensor_nt99140_setRegister(0x300d, 0xe7); 
        sensor_nt99140_setRegister(0x300e, 0x05); 
        sensor_nt99140_setRegister(0x300f, 0x00); 
        sensor_nt99140_setRegister(0x3010, 0x02); 
        sensor_nt99140_setRegister(0x3011, 0xd0); 
        sensor_nt99140_setRegister(0x32b0, 0x00); 
        sensor_nt99140_setRegister(0x32b1, 0x00); 
        sensor_nt99140_setRegister(0x32b2, 0x01); 
        sensor_nt99140_setRegister(0x32b3, 0x7c); 
        sensor_nt99140_setRegister(0x32b4, 0x00); 
        sensor_nt99140_setRegister(0x32b5, 0x64); 
        sensor_nt99140_setRegister(0x32b6, 0x99); 
        sensor_nt99140_setRegister(0x32bb, 0x1b); 

        //sensor_nt99140_setRegister(0x32c5, 0x40);     //0x2f
        sensor_nt99140_setRegister(0x3343, 0xc0); 
        //{0x32bc, 0x40); 
        sensor_nt99140_setRegister(0x32c1, 0x24); 
        sensor_nt99140_setRegister(0x32c2, 0xd6); 
        sensor_nt99140_setRegister(0x32c8, 0x5d); 
        sensor_nt99140_setRegister(0x32c9, 0x4d); 
        sensor_nt99140_setRegister(0x3201, 0x3f); 
        sensor_nt99140_setRegister(0x3021, 0x16); 
        sensor_nt99140_setRegister(0x3060, 0x01); 
        #endif

        #if 0
        //[1280x720_15.01_Fps_PCLK_48M]
        sensor_nt99140_setRegister(0x3028, 0x0f); 
        sensor_nt99140_setRegister(0x3029, 0x00); 
        sensor_nt99140_setRegister(0x302a, 0x04); 
        sensor_nt99140_setRegister(0x3022, 0x24); 
        sensor_nt99140_setRegister(0x3023, 0x24); 
        sensor_nt99140_setRegister(0x3002, 0x00); 
        sensor_nt99140_setRegister(0x3003, 0x04); 
        sensor_nt99140_setRegister(0x3004, 0x00); 
        sensor_nt99140_setRegister(0x3005, 0x04); 
        sensor_nt99140_setRegister(0x3006, 0x05); 
        sensor_nt99140_setRegister(0x3007, 0x03); 
        sensor_nt99140_setRegister(0x3008, 0x02); 
        sensor_nt99140_setRegister(0x3009, 0xd3); 
        sensor_nt99140_setRegister(0x300a, 0x06); 
        sensor_nt99140_setRegister(0x300b, 0x83); 
        sensor_nt99140_setRegister(0x300c, 0x03); 
        sensor_nt99140_setRegister(0x300d, 0xbf); 
        sensor_nt99140_setRegister(0x300e, 0x05); 
        sensor_nt99140_setRegister(0x300f, 0x00); 
        sensor_nt99140_setRegister(0x3010, 0x02); 
        sensor_nt99140_setRegister(0x3011, 0xd0); 
        sensor_nt99140_setRegister(0x32b0, 0x00); 
        sensor_nt99140_setRegister(0x32b1, 0x00); 
        sensor_nt99140_setRegister(0x32b2, 0x01); 
        sensor_nt99140_setRegister(0x32b3, 0x7c); 
        sensor_nt99140_setRegister(0x32b4, 0x00); 
        sensor_nt99140_setRegister(0x32b5, 0x64); 
        sensor_nt99140_setRegister(0x32b6, 0x99); 
        sensor_nt99140_setRegister(0x32bb, 0x1b); 
        sensor_nt99140_setRegister(0x32bc, 0x40); 
        sensor_nt99140_setRegister(0x32c1, 0x23); 
        sensor_nt99140_setRegister(0x32c2, 0xbf); 
        sensor_nt99140_setRegister(0x32c8, 0x48); 
        sensor_nt99140_setRegister(0x32c9, 0x3c); 
        sensor_nt99140_setRegister(0x32c4, 0x00); 
        sensor_nt99140_setRegister(0x3201, 0x3f); 
        sensor_nt99140_setRegister(0x3021, 0x16); 
        sensor_nt99140_setRegister(0x3060, 0x01); 
        #endif

        #if 0
        //[1280x720__FPS_10_TO_15_PCLK_40M]
        sensor_nt99140_setRegister(0x3028, 0x13);
        sensor_nt99140_setRegister(0x3029, 0x02);
        sensor_nt99140_setRegister(0x302a, 0x00);
        sensor_nt99140_setRegister(0x3022, 0x24);
        sensor_nt99140_setRegister(0x3023, 0x24);
        sensor_nt99140_setRegister(0x3002, 0x00);
        sensor_nt99140_setRegister(0x3003, 0x04);
        sensor_nt99140_setRegister(0x3004, 0x00);
        sensor_nt99140_setRegister(0x3005, 0x04);
        sensor_nt99140_setRegister(0x3006, 0x05);
        sensor_nt99140_setRegister(0x3007, 0x03);
        sensor_nt99140_setRegister(0x3008, 0x02);
        sensor_nt99140_setRegister(0x3009, 0xd3);
        sensor_nt99140_setRegister(0x300a, 0x06);
        sensor_nt99140_setRegister(0x300b, 0x83);
        sensor_nt99140_setRegister(0x300c, 0x03);
        sensor_nt99140_setRegister(0x300d, 0x1f);
        sensor_nt99140_setRegister(0x300e, 0x05);
        sensor_nt99140_setRegister(0x300f, 0x00);
        sensor_nt99140_setRegister(0x3010, 0x02);
        sensor_nt99140_setRegister(0x3011, 0xd0);
        sensor_nt99140_setRegister(0x32b0, 0x00);
        sensor_nt99140_setRegister(0x32b1, 0x00);
        sensor_nt99140_setRegister(0x32b2, 0x01);
        sensor_nt99140_setRegister(0x32b3, 0x7c);
        sensor_nt99140_setRegister(0x32b4, 0x00);
        sensor_nt99140_setRegister(0x32b5, 0x64);
        sensor_nt99140_setRegister(0x32b6, 0x99);
        sensor_nt99140_setRegister(0x32bb, 0x1b);
        sensor_nt99140_setRegister(0x32bc, 0x40);
        sensor_nt99140_setRegister(0x32c1, 0x24);
        sensor_nt99140_setRegister(0x32c2, 0xaf);
        sensor_nt99140_setRegister(0x32c8, 0x3c);
        sensor_nt99140_setRegister(0x32c9, 0x32);
        sensor_nt99140_setRegister(0x32c4, 0x00);
        sensor_nt99140_setRegister(0x3201, 0x3f);
        //sensor_nt99140_setRegister(0x3025, 0x02); //Test pattern: color bars
        sensor_nt99140_setRegister(0x3021, 0x16);
        sensor_nt99140_setRegister(0x3060, 0x01);
        #endif
		#if 0
        //[1280x720_FPS_10_TO_15_PCLK_38M]
        sensor_nt99140_setRegister(0x3028, 0x12); 
        sensor_nt99140_setRegister(0x3029, 0x02); 
        sensor_nt99140_setRegister(0x302a, 0x00); 
        sensor_nt99140_setRegister(0x3022, 0x24); 
        sensor_nt99140_setRegister(0x3023, 0x24); 
        sensor_nt99140_setRegister(0x3002, 0x00); 
        sensor_nt99140_setRegister(0x3003, 0x04); 
        sensor_nt99140_setRegister(0x3004, 0x00); 
        sensor_nt99140_setRegister(0x3005, 0x04); 
        sensor_nt99140_setRegister(0x3006, 0x05); 
        sensor_nt99140_setRegister(0x3007, 0x03); 
        sensor_nt99140_setRegister(0x3008, 0x02); 
        sensor_nt99140_setRegister(0x3009, 0xd3); 
        sensor_nt99140_setRegister(0x300a, 0x06); 
        sensor_nt99140_setRegister(0x300b, 0x83); 
        sensor_nt99140_setRegister(0x300c, 0x02); 
        sensor_nt99140_setRegister(0x300d, 0xf7); 
        sensor_nt99140_setRegister(0x300e, 0x05); 
        sensor_nt99140_setRegister(0x300f, 0x00); 
        sensor_nt99140_setRegister(0x3010, 0x02); 
        sensor_nt99140_setRegister(0x3011, 0xd0); 
        sensor_nt99140_setRegister(0x32b0, 0x00); 
        sensor_nt99140_setRegister(0x32b1, 0x00); 
        sensor_nt99140_setRegister(0x32b2, 0x01); 
        sensor_nt99140_setRegister(0x32b3, 0x7c); 
        sensor_nt99140_setRegister(0x32b4, 0x00); 
        sensor_nt99140_setRegister(0x32b5, 0x64); 
        sensor_nt99140_setRegister(0x32b6, 0x99); 
        sensor_nt99140_setRegister(0x32bb, 0x1b); 
        sensor_nt99140_setRegister(0x32bc, 0x40); 
        sensor_nt99140_setRegister(0x32c1, 0x24); 
        sensor_nt99140_setRegister(0x32c2, 0x73); 
        sensor_nt99140_setRegister(0x32c8, 0x39); 
        sensor_nt99140_setRegister(0x32c9, 0x2f); 
        sensor_nt99140_setRegister(0x32c4, 0x00); 
        sensor_nt99140_setRegister(0x3201, 0x3f); 
        sensor_nt99140_setRegister(0x3021, 0x16); 
        sensor_nt99140_setRegister(0x3060, 0x01); 

        sensor_nt99140_setRegister(0x3320, 0x24);
		#endif

#if 1
//MCLK:      12.00MHz 
//PCLK:      36.00MHz 
//Size:      1280x720 
//FPS:       15.00~15.01 
//Line:      1586 
//Frame:      756 
//Flicker:   50Hz 
sensor_nt99140_setRegister(0x32BB, 0x67);  //AE Start
sensor_nt99140_setRegister(0x32BF, 0x60); 
sensor_nt99140_setRegister(0x32C0, 0x6A); 
sensor_nt99140_setRegister(0x32C1, 0x6A); 
sensor_nt99140_setRegister(0x32C2, 0x6A); 
sensor_nt99140_setRegister(0x32C3, 0x00); 
sensor_nt99140_setRegister(0x32C4, 0x2B); 
sensor_nt99140_setRegister(0x32C5, 0x30); 
sensor_nt99140_setRegister(0x32C6, 0x30); 
sensor_nt99140_setRegister(0x32D3, 0x00); 
sensor_nt99140_setRegister(0x32D4, 0x71); 
sensor_nt99140_setRegister(0x32D5, 0x6C); 
sensor_nt99140_setRegister(0x32D6, 0x00); 
sensor_nt99140_setRegister(0x32D7, 0x5F); 
sensor_nt99140_setRegister(0x32D8, 0x67);  //AE End
sensor_nt99140_setRegister(0x32F0, 0x01);  //Output Format
sensor_nt99140_setRegister(0x3200, 0x7E);  //Mode Control
sensor_nt99140_setRegister(0x3201, 0x3D);  //Mode Control
sensor_nt99140_setRegister(0x302A, 0x80);  //PLL Start
sensor_nt99140_setRegister(0x302C, 0x11); 
sensor_nt99140_setRegister(0x302D, 0x02);  //PLL End
sensor_nt99140_setRegister(0x3022, 0x00);  //Timing Start
sensor_nt99140_setRegister(0x300A, 0x06); 
sensor_nt99140_setRegister(0x300B, 0x32); 
sensor_nt99140_setRegister(0x300C, 0x02); 
sensor_nt99140_setRegister(0x300D, 0xF4);  //Timing End
sensor_nt99140_setRegister(0x320A, 0x00); 
sensor_nt99140_setRegister(0x3101, 0x00); 
sensor_nt99140_setRegister(0x302A, 0x80); 
sensor_nt99140_setRegister(0x3025, 0x02); 
sensor_nt99140_setRegister(0x3025, 0x01); 
sensor_nt99140_setRegister(0x3021, 0x02);
sensor_nt99140_setRegister(0x3060, 0x01); 
sensor_nt99140_setRegister(0x3025, 0x00); 


#else
//MCLK:      12.00MHz 
//PCLK:      36.00MHz 
//Size:      1280x720 
//FPS:       10.00~10.01 
//Line:      1586 
//Frame:     1134 
//Flicker:   50Hz 
sensor_nt99140_setRegister(0x32BB, 0x67);  //AE Start
sensor_nt99140_setRegister(0x32BF, 0x60); 
sensor_nt99140_setRegister(0x32C0, 0x74); 
sensor_nt99140_setRegister(0x32C1, 0x74); 
sensor_nt99140_setRegister(0x32C2, 0x74); 
sensor_nt99140_setRegister(0x32C3, 0x00); 
sensor_nt99140_setRegister(0x32C4, 0x2B); 
sensor_nt99140_setRegister(0x32C5, 0x30); 
sensor_nt99140_setRegister(0x32C6, 0x30); 
sensor_nt99140_setRegister(0x32D3, 0x00); 
sensor_nt99140_setRegister(0x32D4, 0x71); 
sensor_nt99140_setRegister(0x32D5, 0x6C); 
sensor_nt99140_setRegister(0x32D6, 0x00); 
sensor_nt99140_setRegister(0x32D7, 0x5F); 
sensor_nt99140_setRegister(0x32D8, 0x67);  //AE End
sensor_nt99140_setRegister(0x32F0, 0x01);  //Output Format
sensor_nt99140_setRegister(0x3200, 0x7E);  //Mode Control
sensor_nt99140_setRegister(0x3201, 0x3D);  //Mode Control
sensor_nt99140_setRegister(0x302A, 0x80);  //PLL Start
sensor_nt99140_setRegister(0x302C, 0x11); 
sensor_nt99140_setRegister(0x302D, 0x02);  //PLL End
sensor_nt99140_setRegister(0x3022, 0x00);  //Timing Start
sensor_nt99140_setRegister(0x300A, 0x06); 
sensor_nt99140_setRegister(0x300B, 0x32); 
sensor_nt99140_setRegister(0x300C, 0x04); 
sensor_nt99140_setRegister(0x300D, 0x6E);  //Timing End
sensor_nt99140_setRegister(0x320A, 0x00); 
sensor_nt99140_setRegister(0x3101, 0x00); 
sensor_nt99140_setRegister(0x302A, 0x80); 
sensor_nt99140_setRegister(0x3025, 0x02); 
sensor_nt99140_setRegister(0x3025, 0x01); 
sensor_nt99140_setRegister(0x3021, 0x02);
sensor_nt99140_setRegister(0x3060, 0x01); 
sensor_nt99140_setRegister(0x3025, 0x00); 
#endif


    }
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

#if 0
BOOL IsPreviewOrRecordMode();

void NT99140_UpdateDayNightMode()
{
    int boNight;
    static int lastNightValue = 0;
    DWORD H, L, gain;
    DWORD dnvalue;

    if (!haveSetted)
        return;

    if (!IsPreviewOrRecordMode())
        return;
    H = sensor_nt99140_readRegister(0x3012);
    L = sensor_nt99140_readRegister(0x3013);
    gain = sensor_nt99140_readRegister(0x301D);
    dnvalue = ((H << 8) | L ) * gain;
    //mpDebugPrint("dnvalue = %d, H = %d, L = %d, gain = %d", dnvalue, H, L, gain);

    ////if (dnvalue > 70000)
    //if (dnvalue > 105000)
    //    boNight = 1;
    //else
    //    boNight = 0;

    if (dnvalue > 120000)
        boNight = 2;
    else if (dnvalue > 50000 && dnvalue <= 120000)
        boNight = 1;
    else
        boNight = 0;

    if (lastNightValue != boNight)
    {
        mpDebugPrint("dnv diff,oldmode=%d, mode=%d", lastNightValue, boNight);
        lastNightValue = boNight;
        return;
    }
    //////////////////////////////
    if (boIsNightMode == boNight)
        return;

    mpDebugPrint("dnv=%d", dnvalue);
    boIsNightMode = boNight;

    EventSet(UI_EVENT, EVENT_SWITCH_DAY_NIGHT);
    
}

void NT99140_SwitchDayNightMode()
{
    if (boIsNightMode == 2)
        mpDebugPrint("switch to night mode");
    else if (boIsNightMode == 1)
        mpDebugPrint("switch to overcast mode");
    else
        mpDebugPrint("switch to day mode");

    if (SetupCamcorderTimingGet() == CAMCORDER_RESOLUTION_VGA)
    {
        NT99140_setVGA_640x480_30FPS();
    }
    else
    {
        NT99140_set_720P_1280x720();
    }
}
#endif

#endif

#endif


