/*
*******************************************************************************
*                           Magic Pixel Inc.                                  *
*                  Copyright (c) 2007 -, All Rights Reserved                  *
*                                                                             *
* TS module driver.                                                           *
*                                                                             *
* File : Tc_FT5306.c                                                          *
* By :                                                                        *
*                                                                             *
*                                                                             *
* Description : FT5306 touch screen controller driver                     *
*                                                                             *
* History :                                                                   *
*                                                                             *
*******************************************************************************
*/

/*************************************************************************/
/***                        Include Files                              ***/
/*************************************************************************/

#define LOCAL_DEBUG_ENABLE 1

#include "global612.h"

#if 0 //use SW i2c

#define IIC_FLAG_NO_ACK		1
#define IIC_FLAG_ACK		2
#define IIC_FLAG_START		3
#define IIC_FLAG_STOP		4
#define IIC_FLAG_IGNORE		5

#define WAIT_TIME_IIC_DELAY	5//10//35  //20 //5
#define WAIT_TIME_WAIT_ACK	70//200  //160 //40

//--------------------------------------------------------
static BYTE ACK1=1, ACK2=1, ACK3=1;

//////////////////////////////////////////
// software simulate i2c base
//////////////////////////////////////////

#define __tvp5150_i2c_data_high()	(g_psGpio->Fgpdat[1] |= 0x00000400)//FGPIO_26 MP662PIN77
#define __tvp5150_i2c_data_low()		(g_psGpio->Fgpdat[1] &= 0xfffffbff)
#define __tvp5150_i2c_data_output()	(g_psGpio->Fgpdat[1] |= 0x04000000)
#define __tvp5150_i2c_data_input()	(g_psGpio->Fgpdat[1] &= 0xfbffffff)

#define __tvp5150_i2c_clk_high()		(g_psGpio->Fgpdat[1] |= 0x00000200) //FGPIO_25 MP662PIN76
#define __tvp5150_i2c_clk_low()		(g_psGpio->Fgpdat[1] &= 0xfffffdff)
#define __tvp5150_i2c_clk_output()	(g_psGpio->Fgpdat[1] |= 0x02000000)
#define __tvp5150_i2c_clk_input()	(g_psGpio->Fgpdat[1] &= 0xfdffffff)


#define __tvp5150_i2c_read_data()   ((g_psGpio->Fgpdat[1] & 0x00000400)>>10)
//#define __tvp5150_i2c_all_output()	(g_psGpio->Gpdat0 |= 0x00040000)
#define __tvp5150_i2c_set_gpio_mode() (g_psGpio->Fgpcfg[1] = (g_psGpio->Fgpcfg[1] & 0xf9fff9ff) )

void __tvp5150_i2c_all_output(void)
{
	__tvp5150_i2c_data_output();
	__tvp5150_i2c_clk_output();
}
/*
void __tvp5150_i2c_set_gpio_mode(void)
{
	g_psGpio->Gpcfg0 = (g_psGpio->Gpcfg0 & 0xfffbfffb);
	GetGPIOValue(FGPIO|GPIO_11);
}
*/
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
	//IODelay(10);
	IODelay(WAIT_TIME_WAIT_ACK); //Tech use
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
		//UartOutText("ACK OK");
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


static BYTE TVP5150_setRegister(BYTE devid,BYTE addr, BYTE data)
{
	ACK1 = SW_TVP5150_IIC_SetValue(devid, IIC_FLAG_START); 
	ACK2 = SW_TVP5150_IIC_SetValue(addr, IIC_FLAG_IGNORE);
	ACK3 = SW_TVP5150_IIC_SetValue(data, IIC_FLAG_STOP);
}
    

static BYTE TVP5150_readRegister(BYTE devid,BYTE addr)
{
	BYTE value;
	ACK1 = SW_TVP5150_IIC_SetValue(devid, IIC_FLAG_START);
	ACK2 = SW_TVP5150_IIC_SetValue(addr, IIC_FLAG_STOP);
	//IODelay(WAIT_TIME_WAIT_ACK);
	ACK3 = SW_TVP5150_IIC_SetValue((devid | 0x01), IIC_FLAG_START);
	value = SW_TVP5150_IIC_GetValue();
	return value;
}

static SDWORD I2CM_Read_BustMode(BYTE DevID, WORD regAddr, BYTE regAddr16, BYTE *inDataBuf, WORD readLength)
{
    M_I2C_REG *regI2cPtr = (M_I2C_REG *) I2C_MR_BASE;
    SDWORD status = PASS;
    DWORD i, dataLength;
    BOOL semaLocked = FALSE;
    BYTE tmpRegAddr[2];

    //i2cm_resource_lock(&semaLocked);
    MP_DEBUG("-----%s to DevID, total %dbytes", __FUNCTION__, DevID, readLength);

    if (regAddr16)
    {
        dataLength = 2;
        tmpRegAddr[0] = regAddr >> 8;
        tmpRegAddr[1] = regAddr & 0xFF;
    }
    else
    {
        dataLength = 1;
        tmpRegAddr[0] = regAddr & 0xFF;
    }


    //i2cm_PowerCtrl(ENABLE);
    DevID &= 0xFE;
    //regI2cPtr->I2C_MReg60 = ((DWORD) DevID) << 24;
    //regI2cPtr->I2C_MReg61 = i2cm_TimingTableSearch(DevID);
   // regI2cPtr->I2C_MReg62 = I2C_SCL_FROM_EXT | I2C_LOOP_MODE | I2C_NO_RSTART | I2C_16BIT_MODE;

	SW_TVP5150_IIC_SetValue(DevID, IIC_FLAG_START);

    for (i = 0; i < dataLength; i++)
    {
       // regI2cPtr->I2C_MReg63 = ((DWORD) tmpRegAddr[i]) << 24;
		
		SW_TVP5150_IIC_SetValue(tmpRegAddr[i], IIC_FLAG_IGNORE);
       // i2cm_BusyWiat();
      //  regI2cPtr->I2C_MReg64 |= BIT0;

    }

    //if (status == PASS)
    {
        //regI2cPtr->I2C_MReg60 = BIT24 | (((DWORD) DevID) << 24);        // read operation
        //regI2cPtr->I2C_MReg61 = i2cm_TimingTableSearch(DevID);
        //regI2cPtr->I2C_MReg62 = I2C_SCL_FROM_EXT | I2C_LOOP_MODE | I2C_NO_RSTART | I2C_16BIT_MODE;
	SW_TVP5150_IIC_SetValue((DevID|0x01), IIC_FLAG_START);

        for (i = 0; i < readLength; i++)
        {
            //i2cm_BusyWiat();
            //regI2cPtr->I2C_MReg64 |= BIT0;

            inDataBuf[i] = SW_TVP5150_IIC_GetValue();;
        }

        //regI2cPtr->I2C_MReg62 = I2C_SCL_FROM_EXT | I2C_LOOP_MODE | I2C_NO_RSTART;
        //i2cm_BusyWiat();
        //regI2cPtr->I2C_MReg64 |= BIT0;
        //i2cm_WaitAck();
    }

    return status;
}

static SDWORD I2CM_WtReg8Data8(BYTE devid,BYTE addr, BYTE data)
{
    mpDebugPrint("__%s__ devid=%x addr=%d", __FUNCTION__,devid,addr);
	return TVP5150_setRegister(devid,addr,data);
}

static SDWORD I2CM_RdReg8Data8(BYTE DevID, BYTE reg)
{
	return TVP5150_readRegister(DevID,reg);
}


#endif


#if ((TOUCH_CONTROLLER_ENABLE == ENABLE) && (TOUCH_CONTROLLER == TOUCH_PANEL_DRIVER_FT5306))
#include "mpTrace.h"
#include "os.h" 
#include "taskid.h"
#include "peripheral.h"
#include "tc_driver.h"
#include "touchCtrller.h"
#include "Ui.h"

#define     TOUCH_PANEL_KIND 2   //->soft  0->default
#if (TOUCH_PANEL_KIND==2)//(TOUCH_MAX_X,TOUCH_MIN_Y) (TOUCH_MIN_X,TOUCH_MAX_Y)
WORD wTcMinX=35,wTcMinY=215,wTcMaxX=2050,wTcMaxY=2050;
//WORD wTcMinX=48,wTcMinY=358,wTcMaxX=3953,wTcMaxY=3953;
#endif
#define  TOUCH_MIN_COUNT 6// 4
#define  TOUCH_MAX_COUNT 20
#define  TOUCH_CRACK_COUNT 200
#define  TOUCH_TOLERANCE_X 10
#define  TOUCH_TOLERANCE_Y 10
static BYTE s_bIRQ_Level=0;
static WORD s_wTouchX[TOUCH_MAX_COUNT],s_wTouchY[TOUCH_MAX_COUNT];
static DWORD s_dwTouchCnt=0;
static DWORD gpioNum, gpioIntNum;
#if 0
void GetTouchValue()
{
g_psSetupMenu->wsTcMinX=wTcMinX;
g_psSetupMenu->wsTcMinY=wTcMinY;
g_psSetupMenu->wsTcMaxX=wTcMaxX;
g_psSetupMenu->wsTcMaxY=wTcMaxY;
}#define TOUCH_IRQ_PIN			GPIO_GPIO_7

#endif
#if 1//!defined(HW_IIC_MASTER_ENABLE)||(HW_IIC_MASTER_ENABLE == 0)
//sch T_REST GPIO3 PIN51  //SDA
//TGPIO0 GPIO7 PIN69       //WAKE
//TGPIO1 FGPIO17 PIN90   //INT CHECK_OK
//TGPIO2 FGPIO18 PIN91   //SCL
//TGPIO3 FGPIO19 PIN92
#define TIMER_CHECK_TOUCH_INT 1
#define SW_IIC_CLK_PIN			    GPIO_FGPIO_25//GPIO_FGPIO_18 //GPIO_FGPIO_17
#define SW_IIC_DATA_PIN		      GPIO_FGPIO_26//GPIO_FGPIO_17 //GPIO_FGPIO_18
//#define Touch_PENIRQ      				GPIO_GPIO_2//GPIO_GPIO_7
#define TOUCH_RESET_PIN			GPIO_FGPIO_20//GPIO_GPIO_3//GPIO_GPIO_5

#define TOUCH_CONTROLLER_INT_PIN    GPIO_02//GPIO_07// GPIO_03


#define IIC_IO_DELAY 1
#define WAIT_TIME_WAIT_ACK 50//125
#define Touch_Receive_Buf_Len  7 //40
BYTE TouchData_Buffer[Touch_Receive_Buf_Len];


//--------------------------------------------------------
void SetTouchResetPin()
{
	Gpio_Config2GpioFunc(TOUCH_RESET_PIN, GPIO_OUTPUT_MODE, GPIO_DATA_HIGH, 1);
}

//--------------------------------------------------------
//////////////////////////////////////////
// software IICM
//////////////////////////////////////////
static void _I2CM_Delay(DWORD dwTime)
{
	IODelay(dwTime*IIC_IO_DELAY);
}
#define CLK_LOW                     0
#define CLK_HIGH                     1
#define DATA_LOW                     2
#define DATA_HIGH                     3
#define DATA_INPUT                     4
#define DATA_OUTPUT                   5
static void I2CM_Set_Line_Level(DWORD value)
{
if (value == CLK_LOW)
Gpio_DataSet(SW_IIC_CLK_PIN, 0, 1);
else if ((value == CLK_HIGH))
Gpio_DataSet(SW_IIC_CLK_PIN, 1, 1);
else if ((value == DATA_LOW))
Gpio_DataSet(SW_IIC_DATA_PIN, 0, 1);
else if ((value == DATA_HIGH))
Gpio_DataSet(SW_IIC_DATA_PIN, 1, 1);
//IODelay(IIC_IO_DELAY);
}
static void I2CM_Set_Data_Direction(DWORD value)
{
if (value==DATA_INPUT)
Gpio_Config2GpioFunc(SW_IIC_DATA_PIN, GPIO_INPUT_MODE, 0, 1);
else //DATA_OUTPUT
Gpio_Config2GpioFunc(SW_IIC_DATA_PIN, GPIO_OUTPUT_MODE, 0, 1);
}
static BYTE sw_iic_read_data()
{
WORD data= 0;
Gpio_DataGet(SW_IIC_DATA_PIN, &data, 1);
if (data)
return 1;
else
return 0;
}
static void sw_iic_config_write_func()
{
Gpio_Config2GpioFunc(SW_IIC_CLK_PIN, GPIO_OUTPUT_MODE, 0, 1);
Gpio_Config2GpioFunc(SW_IIC_DATA_PIN, GPIO_OUTPUT_MODE, 0, 1);
}
static void   Start(void)
{
sw_iic_config_write_func();
I2CM_Set_Line_Level(DATA_HIGH);           //起始?件的?据信?
_I2CM_Delay(2);
I2CM_Set_Line_Level(CLK_HIGH);           //起始?件??信?
_I2CM_Delay(5);                   //信?建立??> 4.7us
I2CM_Set_Line_Level(DATA_LOW);           //起始信?
_I2CM_Delay(5);
I2CM_Set_Line_Level(CLK_LOW);           //?住I2C??,   准??送或者接受?据
_I2CM_Delay(2);
}
static void   Stop(void)
{
I2CM_Set_Data_Direction(DATA_OUTPUT);
I2CM_Set_Line_Level(DATA_LOW);             //?束?件的?据信?
_I2CM_Delay(3);
I2CM_Set_Line_Level(CLK_HIGH);             //?束?件的??信?
_I2CM_Delay(5);
I2CM_Set_Line_Level(DATA_HIGH);             //?束信?
_I2CM_Delay(4);
}
void   Ack(void)
{
        I2CM_Set_Line_Level(DATA_LOW);
_I2CM_Delay(2);
        I2CM_Set_Line_Level(CLK_HIGH);
_I2CM_Delay(5);
        I2CM_Set_Line_Level(CLK_LOW);
_I2CM_Delay(2);
}
void   NoAck(void)
{
I2CM_Set_Line_Level(DATA_HIGH);
_I2CM_Delay(2);
I2CM_Set_Line_Level(CLK_HIGH);
_I2CM_Delay(5);
I2CM_Set_Line_Level(CLK_LOW);
_I2CM_Delay(2);
}
BYTE  GetAck(void)
{
	BYTE i,bData;
	I2CM_Set_Data_Direction(DATA_INPUT);
	_I2CM_Delay(5); // 2
	I2CM_Set_Line_Level(CLK_HIGH);
	_I2CM_Delay(3);
	for (i=0;i<WAIT_TIME_WAIT_ACK;i++)
	{
	if (!(bData   =   sw_iic_read_data()))
	break;
	_I2CM_Delay(2);
	}
	if (bData)
	mpDebugPrint(" !!! GetAck error ");
	I2CM_Set_Line_Level(CLK_LOW);
	_I2CM_Delay(2);
	I2CM_Set_Data_Direction(DATA_OUTPUT);
	return bData;
}
void   Write_I2c(BYTE   date)
{
BYTE i;
for(i   =   0;   i   <   8;   i++)
{
	if (date & 0x80)  //送?据到?据?上
	I2CM_Set_Line_Level(DATA_HIGH);
	else
	I2CM_Set_Line_Level(DATA_LOW);
	date<<=1;
	_I2CM_Delay(2);
	I2CM_Set_Line_Level(CLK_HIGH);                                     //置??信??高?平,使?据有效
	_I2CM_Delay(5);
	I2CM_Set_Line_Level(CLK_LOW);
	_I2CM_Delay(2);
	}
}
BYTE   Read_I2c(void)
{
	BYTE   i;
	BYTE   byte   =   0;
	I2CM_Set_Data_Direction(DATA_INPUT);
	_I2CM_Delay(2);
	for(i   =   0;   i   <   8;   i++)
	{
		I2CM_Set_Line_Level(CLK_LOW);                       //置???低?平,准?接受?据
		_I2CM_Delay(5);
		I2CM_Set_Line_Level(CLK_HIGH);                     //置???高?平,使?据??据有效
		_I2CM_Delay(2);
		byte<<=1;
		byte|=sw_iic_read_data();
		_I2CM_Delay(2);
		I2CM_Set_Line_Level(CLK_LOW);
	}
	I2CM_Set_Data_Direction(DATA_OUTPUT);
	_I2CM_Delay(1);
	return (byte);
}
void   Write_Date(BYTE DevID,BYTE StartAddress,BYTE *date,BYTE bytes)
{
BYTE   i   =   0;
if (StartAddress)
{
Start();
Write_I2c(DevID&0xfe);
GetAck();
}
Write_I2c(StartAddress);
GetAck();
for(i = 0;i < bytes; i++)
{
Write_I2c(*date);
GetAck();
date++;
}
Stop();
_I2CM_Delay(20);
}
void   Read_Date(BYTE DevID,BYTE StartAddress,BYTE *date,BYTE bytes)
{
BYTE   i;
if (StartAddress)
{
Start();
Write_I2c(DevID&0xfe);
GetAck();
Write_I2c(StartAddress);
GetAck();
Stop();
_I2CM_Delay(10);
}
Start();
Write_I2c(DevID|0x01);
GetAck();
for   (i   =   0;   i   <   bytes;   i++)
{
*date   =   Read_I2c();
Ack();
//NoAck();
date++;
}
NoAck();
Stop();
_I2CM_Delay(20);
}
#endif
void Start_I2c()
{
Start();
}
void Stop_I2c()
{
Stop();
}
void Ack_I2c(BYTE date)
{
if (date)
NoAck();
else
Ack();
}
BYTE SendByte(BYTE date)
{
	Write_I2c(date);
	return (GetAck()==0);
}
BYTE RcvByte(void)
{
	return Read_I2c();
}


#if 1

static WORD sample_x, sample_y,LastX=0,LastY=0;
void I2C_ReadTouchData() //from datasheet
{
	UartOutText("I");
ST_TC_MSG tcMessage;

	WORD i;
	BYTE tp_point;

	Gpio_IntDisable(gpioIntNum);
#if 0
	Start_I2c();
	if(!SendByte(0x70)) //send slave address，bus write
	{
		mpDebugPrint("---%s--- 0x70 ACK ERROR",__FUNCTION__);
		goto THE_END; //no ack retrun
	}
	if(!SendByte(0x00)) //send command
	{
		mpDebugPrint("---%s--- 0xF9 ACK ERROR",__FUNCTION__);
		goto THE_END; //no ack retrun
	}
	
	Stop_I2c();
#endif	
	Start_I2c();
	if(!SendByte(0x71)) //send slave address，bus read
	{
		mpDebugPrint("---%s--- 0x71 ACK ERROR",__FUNCTION__);
		goto THE_END; //no ack retrun
	}
	
	for(i=0;i< Touch_Receive_Buf_Len;i++)
	{
		TouchData_Buffer[i]=RcvByte(); //receive data

			if (i == Touch_Receive_Buf_Len -1)
				Ack_I2c(1); 
			else
				Ack_I2c(0); 
		
	}
	
	Stop_I2c();
#if 0
	mpDebugPrint("--=%d",i,TouchData_Buffer[0]);

#else


	tp_point =TouchData_Buffer[2] & 0x0f;
	//MP_ALERT("t point =%x", tp_point);
	if(tp_point == 0) // take release
	{
		#if 0
		sample_x =(short)(TouchData_Buffer[3] & 0x0F)<<8 | (short)TouchData_Buffer[4] ;
		sample_y =(short)(TouchData_Buffer[5] & 0x0F)<<8 | (short)TouchData_Buffer[6] ;
		mpDebugPrint("x=%d,y=%d",sample_x,sample_y);
		#elif 1 //my sample
		sample_y =(short)(TouchData_Buffer[3] & 0x0F)<<8 | (short)TouchData_Buffer[4] ; //revert x or y
		sample_x =(short)(TouchData_Buffer[5] & 0x0F)<<8 | (short)TouchData_Buffer[6] ;
		if (sample_x==LastX && sample_y==LastY)
			goto THE_END;
		LastY=sample_y;
		LastX=sample_x;
		for (i=0;i<Touch_Receive_Buf_Len;i++)
			mpDebugPrint("--- [%d]=%d",i,TouchData_Buffer[i]);
		mpDebugPrint("x1=%d,y1=%d",sample_x,sample_y);
		#else
		SWORD swX=(short)(TouchData_Buffer[3] & 0x0F)<<8 | (short)TouchData_Buffer[4] ; 
		SWORD swY =(short)(TouchData_Buffer[5] & 0x0F)<<8 | (short)TouchData_Buffer[6] ;
		mpDebugPrint("swX=%d,swY=%d",swX,swY);
		swX-=12;
		swY-=12;
		if (swX<0)
			swX=0;
		if (swY<0)
			swY=0;
		sample_x=swX*1023/1260;
		if (sample_x>1023)
			sample_x=1023;
		sample_y=swY*599/780;
		if (sample_y>599)
			sample_y=599;
		#endif

		//ISR_TP_LOCK();					
		tcMessage.status = TC_DOWN;
		MessageDrop(TcGetMsgId(), (BYTE *) &tcMessage, sizeof(ST_TC_MSG));
		//Ui_TimerProcAdd(3000, TouchIntEnable);
	}
#endif

THE_END:		
	Ui_TimerProcAdd(1000, TouchIntEnable);

}
#endif
/*
void I2C_ReadTouchData()
{
	UartOutText("I");
	MP_ALERT("%d",I2C_ReadTouchDataByte(0x02) );
	MP_ALERT("x1=%d",((I2C_ReadTouchDataByte(0x03) & 0x0F)<<8 ) | I2C_ReadTouchDataByte(0x04)  );
	MP_ALERT("y1=%d",((I2C_ReadTouchDataByte(0x05) & 0x0F)<<8 ) | I2C_ReadTouchDataByte(0x06)  );
}
*/

void TouchIntEnable()
{
	//Gpio_IntConfig(gpioIntNum, GPIO_ACTIVE_LOW, TC_TRIGGER_MODE);
	Gpio_IntEnable(gpioIntNum);
}

static SDWORD _get_point(Tc_point * data) // fiill data->x,data->y,data->reserved
{

data->x1 = (sample_x );//1024/1200=18/150 = 64/75
data->y1 = (sample_y );
//MP_ALERT("_get_point, x=%d,y=%d, x'=%d,y'=%d",sample_x,sample_y,data->x1,data->y1);
//	ISP_TP_UNLOCK();

return 0;


}

void TouchPanelInit()
{
         MP_ALERT("TouchPanelInit");
		 
	ResetTouchPanel(); //Reset before touch
	 Gpio_Config2GpioFunc(SW_IIC_CLK_PIN, GPIO_INPUT_MODE, 1, 1);
	 Gpio_Config2GpioFunc(SW_IIC_DATA_PIN, GPIO_INPUT_MODE, 1, 1);// for 7D
	 //Gpio_Config2GpioFunc(SW_IIC_DATA_PIN, GPIO_OUTPUT_MODE, 1, 1);// for 10D
}

void ResetTouchPanel()
{
	mpDebugPrint("\n---TP PullDown reset pin");
//	Gpio_Config2GpioFunc(GPIO_FGPIO_17, GPIO_OUTPUT_MODE, GPIO_DATA_HIGH, 1);// PullUp data pin
	Gpio_Config2GpioFunc(TOUCH_RESET_PIN, GPIO_OUTPUT_MODE, GPIO_DATA_LOW, 1);// reset TP ic
	TimerDelay(2000);
	mpDebugPrint("\n---TP PullUp reset pin");
	//Gpio_Config2GpioFunc(GPIO_FGPIO_17, GPIO_OUTPUT_MODE, GPIO_DATA_HIGH, 1);// PullUp data pin
	Gpio_Config2GpioFunc(TOUCH_RESET_PIN, GPIO_OUTPUT_MODE, GPIO_DATA_HIGH, 1);

}

/****************************************************************************
 **
 ** NAME:           NT11003_Init
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:     tp_Init. and Setting the mode of interrupt pin.
 **
 ****************************************************************************/
static SDWORD tp_Init(void)
{
    MP_DEBUG("__%s__", __FUNCTION__);
//E_GPIO_GROUP_INDEX gpioNum;
//E_GPIO_INT_GROUP_INDEX gpioIntNum;
    SDWORD status;

    //u08IntModeFlag = INT_LOW_LEVEL;
    gpioNum = GetGPIOPinIndex(TOUCH_CONTROLLER_INT_PIN);
    Gpio_Config2GpioFunc(gpioNum, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1);
    gpioIntNum = GetGpioIntIndexByGpioPinNum(gpioNum);
    Gpio_IntConfig(gpioIntNum, GPIO_ACTIVE_LOW, GPIO_EDGE_TRIGGER);
    Gpio_IntCallbackFunRegister(gpioIntNum, I2C_ReadTouchData);
    TouchPanelInit();
    TouchIntEnable();
  //  SetGPIOValue(GPIO_02, 1);
//    memset(&SampleBucketNt1103,0,sizeof(SampleBucketNt1103));

    return 0;
}

static SDWORD FT5306_Sleep(BYTE sleep)
{
}

static SDWORD FT5306_Check_Interrupt(void)
{
    WORD       data;

    Gpio_DataGet(gpioNum, &data, 1);
    if(data)
    {
        return 1;       // no touch
    }
    else
    {
        return 0;       // touch
    }
}

static SDWORD FT5306_change_sensitivity(DWORD x_sen, DWORD y_sen, DWORD z_sen)
{
    return 0;
}


/****************************************************************************
 **
 ** NAME:           TcDriverInit
 **
 ** PARAMETERS:     stTcDriver
 **
 ** RETURN VALUES:  result
 **
 ** DESCRIPTION:    constructor of TS driver.
 **
 ****************************************************************************/
SDWORD TcDriverInit(ST_TC_DRIVER * stTcDriver)
{
    stTcDriver->u08Name = "tp driver";
    stTcDriver->TcInit = tp_Init;
    stTcDriver->TcSleep = FT5306_Sleep;
    stTcDriver->TcGetData = _get_point;
    stTcDriver->TcCheckInterrupt = FT5306_Check_Interrupt;
    stTcDriver->TcChangeSensitivity = FT5306_change_sensitivity;

    return 0;
}


#if 0
SWORD GetTouchXYData()
{
WORD i,wX,wY;
I2C_ReadTouchData();
for (i=0;i<5;i++)
{
if (s_dwTouchCnt >= TOUCH_MAX_COUNT)
break;
wX=TouchData_Buffer[5+i*4];
wX=(wX<<8)+TouchData_Buffer[6+i*4];
wY=TouchData_Buffer[7+i*4];
wY=(wY<<8)+TouchData_Buffer[8+i*4];
mpDebugPrint("------ wX=%d wY=%d",wX,wY);
if (wX<wTcMinX)
wX=wTcMinX;
if (wX>wTcMaxX)
wX=wTcMaxX;
if (wY<wTcMinY)
wY=wTcMinY;
if (wY>wTcMaxY)
wY=wTcMaxY;
     s_wTouchX[s_dwTouchCnt] =(wTcMaxX-wX)*(((ST_IMGWIN *)Idu_GetCurrWin())->wWidth)/(wTcMaxX-wTcMinX);
    s_wTouchY[s_dwTouchCnt] = (wY-wTcMinY)*(((ST_IMGWIN *)Idu_GetCurrWin())->wHeight)/(wTcMaxY-wTcMinY);
s_dwTouchCnt++;
}
}
 #endif
#endif


#if 0//((TOUCH_CONTROLLER_ENABLE == ENABLE) && (TOUCH_CONTROLLER == TOUCH_PANEL_DRIVER_FT5306))
#include "mpTrace.h"
#include "os.h"
#include "taskid.h"
#include "peripheral.h"
#include "tc_driver.h"
#include "Tc_ft5306.h"

/*************************************************************************/
/***               Public Variable Declaration                         ***/
/*************************************************************************/

/*************************************************************************/
/***               Internally Visible Constants and Static Data        ***/
/*************************************************************************/
/* ft5306 command define */

#define INT_LOW_LEVEL           0
#define INT_HIGH_LEVEL          1

#define TC_INT_USING_EDGE_TRIGGER
#ifdef TC_INT_USING_EDGE_TRIGGER
#define TC_TRIGGER_MODE     GPIO_EDGE_TRIGGER
#else
#define TC_TRIGGER_MODE     GPIO_LEVEL_TRIGGER
#endif


static BYTE u08IntModeFlag;

#define SAMPLE_ARRAY_SIZE 	5
typedef struct
{
    WORD x;
    WORD y;
} _SampleAry;

typedef struct
{
    _SampleAry SampleAry[SAMPLE_ARRAY_SIZE];
    BYTE index;
    BYTE num;
} _SampleBucket;

static _SampleBucket SampleBucket;

static DWORD gpioNum, gpioIntNum;

static SDWORD FT5306_change_sensitivity(DWORD, DWORD, DWORD);



struct Upgrade_Info {
	WORD delay_aa;		/*delay of write FT_UPGRADE_AA */
	WORD delay_55;		/*delay of write FT_UPGRADE_55 */
	BYTE upgrade_id_1;	/*upgrade id 1 */
	BYTE upgrade_id_2;	/*upgrade id 2 */
	WORD delay_readid;	/*delay of read id */
	WORD delay_earse_flash; /*delay of earse flash*/
};


static void fts_get_upgrade_info(struct Upgrade_Info *upgrade_info)
{
	switch (DEVICE_IC_TYPE) {
	case IC_FT5X06:
		upgrade_info->delay_55 = FT5X06_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5X06_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5X06_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5X06_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5X06_UPGRADE_READID_DELAY;
		upgrade_info->delay_earse_flash = FT5X06_UPGRADE_EARSE_DELAY;
		break;
	case IC_FT5606:
		upgrade_info->delay_55 = FT5606_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5606_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5606_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5606_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5606_UPGRADE_READID_DELAY;
		upgrade_info->delay_earse_flash = FT5606_UPGRADE_EARSE_DELAY;
		break;
	case IC_FT5316:
		upgrade_info->delay_55 = FT5316_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5316_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5316_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5316_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5316_UPGRADE_READID_DELAY;
		upgrade_info->delay_earse_flash = FT5316_UPGRADE_EARSE_DELAY;
		break;
	case IC_FT6208:
		upgrade_info->delay_55 = FT6208_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT6208_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT6208_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT6208_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT6208_UPGRADE_READID_DELAY;
		upgrade_info->delay_earse_flash = FT6208_UPGRADE_EARSE_DELAY;
		break;
	default:
		break;
	}
}
int fts_ctpm_fw_read_app(BYTE *pbt_buf,
			  DWORD dw_lenth)
{
	DWORD packet_number;
	DWORD j = 0;
	DWORD temp;
	DWORD lenght = 0;
	BYTE *pReadBuf = NULL;
	BYTE auc_i2c_write_buf[10];
	int i_ret;

	dw_lenth = dw_lenth - 2;

	pReadBuf = malloc(dw_lenth + 1);
	packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
	auc_i2c_write_buf[0] = 0x03;
	auc_i2c_write_buf[1] = 0x00;

	/*Read flash*/
	for (j = 0; j < packet_number; j++) {
		temp = j * FTS_PACKET_LENGTH;
		auc_i2c_write_buf[2] = (BYTE) (temp >> 8);
		auc_i2c_write_buf[3] = (BYTE) temp;
		
		i_ret = I2CM_Read_Length_BustMode(FT5306_I2C_ADDRESS, auc_i2c_write_buf, 4, 
			pReadBuf+lenght, FTS_PACKET_LENGTH);
		if (i_ret < 0)
			return ERR;
		TaskSleep(FTS_PACKET_LENGTH / 6 + 1);
		lenght += FTS_PACKET_LENGTH;
	}

	if ((dw_lenth) % FTS_PACKET_LENGTH > 0) {
		temp = packet_number * FTS_PACKET_LENGTH;
		auc_i2c_write_buf[2] = (BYTE) (temp >> 8);
		auc_i2c_write_buf[3] = (BYTE) temp;
		temp = (dw_lenth) % FTS_PACKET_LENGTH;

		i_ret = I2CM_Read_Length_BustMode(FT5306_I2C_ADDRESS, auc_i2c_write_buf, 4, 
			pReadBuf+lenght, temp);
		if (i_ret < 0)
			return -ERR;
		TaskSleep(FTS_PACKET_LENGTH / 6 + 1);
		lenght += temp;
	}

	/*read the last six byte */
	temp = 0x6ffa + j;
	auc_i2c_write_buf[2] = (BYTE) (temp >> 8);
	auc_i2c_write_buf[3] = (BYTE) temp;
	temp = 6;
	i_ret = I2CM_Read_Length_BustMode(FT5306_I2C_ADDRESS, auc_i2c_write_buf, 4, 
		pReadBuf+lenght, temp);
	if (i_ret < 0)
		return ERR;
	TaskSleep(FTS_PACKET_LENGTH / 6 + 1);
	lenght += temp;


	/*read app from flash and compart*/
	for (j=0; j<dw_lenth-2; j++) {
		if(pReadBuf[j] != pbt_buf[j]) {
			mem_free(pReadBuf);
			return ERR;
		}
	}

	mem_free(pReadBuf);
	return 0;
}
int fts_ctpm_fw_upgrade(BYTE *pbt_buf,
			  DWORD dw_lenth)
{
	BYTE reg_val[2] = {0};
	DWORD i = 0;
	DWORD packet_number;
	DWORD j;
	DWORD temp;
	DWORD lenght;
	BYTE packet_buf[FTS_PACKET_LENGTH + 6];
	BYTE auc_i2c_write_buf[10];
	BYTE bt_ecc;
	int i_ret;
	struct Upgrade_Info upgradeinfo;

	fts_get_upgrade_info(&upgradeinfo);
	
	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
		/*********Step 1:Reset  CTPM *****/
		/*write 0xaa to register 0xfc */
		if (DEVICE_IC_TYPE == IC_FT6208)
			I2CM_WtReg8Data8(FT5306_I2C_ADDRESS, 0xbc, FT_UPGRADE_AA);
		else
			I2CM_WtReg8Data8(FT5306_I2C_ADDRESS, 0xfc, FT_UPGRADE_AA);
		TaskSleep(upgradeinfo.delay_aa);

		/*write 0x55 to register 0xfc */
		if (DEVICE_IC_TYPE == IC_FT6208)
			I2CM_WtReg8Data8(FT5306_I2C_ADDRESS, 0xbc, FT_UPGRADE_55);
		else
			I2CM_WtReg8Data8(FT5306_I2C_ADDRESS, 0xfc, FT_UPGRADE_55);

		TaskSleep(upgradeinfo.delay_55);
		/*********Step 2:Enter upgrade mode *****/
		auc_i2c_write_buf[0] = FT_UPGRADE_55;
		auc_i2c_write_buf[1] = FT_UPGRADE_AA;
		do {
			i++;
			i_ret = I2CM_Write_BustMode(FT5306_I2C_ADDRESS, auc_i2c_write_buf, 2);
			TaskSleep(5);
		} while (i_ret < 0 && i < 5);


		/*********Step 3:check READ-ID***********************/

		
		TaskSleep(upgradeinfo.delay_readid);
		auc_i2c_write_buf[0] = 0x90;
		auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] =
			0x00;
		I2CM_Read_Length_BustMode(FT5306_I2C_ADDRESS, auc_i2c_write_buf, 4, reg_val, 2);


		if (reg_val[0] == upgradeinfo.upgrade_id_1
			&& reg_val[1] == upgradeinfo.upgrade_id_2) {
			//dev_MP_DEBUG(&client->dev, "[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",
				//reg_val[0], reg_val[1]);
			MP_DEBUG3("[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x i=%d\n",
				reg_val[0], reg_val[1],i);
			break;
		} else {
			MP_DEBUG3("[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x i=%d\n",
				reg_val[0], reg_val[1],i);
		}
	}
	if (i >= FTS_UPGRADE_LOOP)
		return ERR;
	auc_i2c_write_buf[0] = 0xcd;

	I2CM_Read_Length_BustMode(FT5306_I2C_ADDRESS, auc_i2c_write_buf, 1, reg_val, 1);


	/*Step 4:erase app and panel paramenter area*/
	MP_DEBUG("Step 4:erase app and panel paramenter area\n");
	auc_i2c_write_buf[0] = 0x61;
	I2CM_Write_BustMode(FT5306_I2C_ADDRESS, auc_i2c_write_buf, 1);	/*erase app area */
	TaskSleep(upgradeinfo.delay_earse_flash);
	/*erase panel parameter area */
	auc_i2c_write_buf[0] = 0x63;
	I2CM_Write_BustMode(FT5306_I2C_ADDRESS, auc_i2c_write_buf, 1);
	TaskSleep(100);

	/*********Step 5:write firmware(FW) to ctpm flash*********/
	bt_ecc = 0;
	MP_DEBUG("Step 5:write firmware(FW) to ctpm flash\n");

	dw_lenth = dw_lenth - 8;
	packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
	packet_buf[0] = 0xbf;
	packet_buf[1] = 0x00;

	for (j = 0; j < packet_number; j++) {
		temp = j * FTS_PACKET_LENGTH;
		packet_buf[2] = (BYTE) (temp >> 8);
		packet_buf[3] = (BYTE) temp;
		lenght = FTS_PACKET_LENGTH;
		packet_buf[4] = (BYTE) (lenght >> 8);
		packet_buf[5] = (BYTE) lenght;

		for (i = 0; i < FTS_PACKET_LENGTH; i++) {
			packet_buf[6 + i] = pbt_buf[j * FTS_PACKET_LENGTH + i];
			bt_ecc ^= packet_buf[6 + i];
		}
		
		I2CM_Write_BustMode(FT5306_I2C_ADDRESS, packet_buf, FTS_PACKET_LENGTH + 6);
		TaskSleep(FTS_PACKET_LENGTH / 6 + 1);
		//MP_DEBUG("write bytes:0x%04x\n", (j+1) * FTS_PACKET_LENGTH);
		//delay_qt_ms(FTS_PACKET_LENGTH / 6 + 1);
	}

	if ((dw_lenth) % FTS_PACKET_LENGTH > 0) {
		temp = packet_number * FTS_PACKET_LENGTH;
		packet_buf[2] = (BYTE) (temp >> 8);
		packet_buf[3] = (BYTE) temp;
		temp = (dw_lenth) % FTS_PACKET_LENGTH;
		packet_buf[4] = (BYTE) (temp >> 8);
		packet_buf[5] = (BYTE) temp;

		for (i = 0; i < temp; i++) {
			packet_buf[6 + i] = pbt_buf[packet_number * FTS_PACKET_LENGTH + i];
			bt_ecc ^= packet_buf[6 + i];
		}

		I2CM_Write_BustMode(FT5306_I2C_ADDRESS, packet_buf, temp + 6);
		TaskSleep(20);
	}


	/*send the last six byte */
	for (i = 0; i < 6; i++) {
		temp = 0x6ffa + i;
		packet_buf[2] = (BYTE) (temp >> 8);
		packet_buf[3] = (BYTE) temp;
		temp = 1;
		packet_buf[4] = (BYTE) (temp >> 8);
		packet_buf[5] = (BYTE) temp;
		packet_buf[6] = pbt_buf[dw_lenth + i];
		bt_ecc ^= packet_buf[6];
		I2CM_Write_BustMode(FT5306_I2C_ADDRESS, packet_buf, 7);
		TaskSleep(20);
	}


	/*********Step 6: read out checksum***********************/
	/*send the opration head */
	MP_DEBUG("Step 6: read out checksum\n");
	auc_i2c_write_buf[0] = 0xcc;
	I2CM_Read_Length_BustMode(FT5306_I2C_ADDRESS, auc_i2c_write_buf, 1, reg_val, 1);
	if (reg_val[0] != bt_ecc) {
		MP_DEBUG2( "[FTS]--ecc error! FW=%02x bt_ecc=%02x\n",
					reg_val[0],
					bt_ecc);
		return ERR;
	}

	/*read app from flash and compare*/
	MP_DEBUG("Read flash and compare\n");
#if 0
	if(fts_ctpm_fw_read_app(pbt_buf, dw_lenth+6) < 0) {
		MP_DEBUG("[FTS]--app from flash is not equal to app.bin\n");
		return ERR;
	}
#endif	
	/*********Step 7: reset the new FW***********************/
	MP_DEBUG("Step 7: reset the new FW\n");
	auc_i2c_write_buf[0] = 0x07;
	I2CM_Write_BustMode(FT5306_I2C_ADDRESS, auc_i2c_write_buf, 1);
	TaskSleep(300);	/*make sure CTP startup normally */

	return 0;
}
int fts_ctpm_fw_upgrade_with_i_file()
{
	BYTE *pbt_buf = NULL;
	int i_ret;
	int fw_len = sizeof(CTPM_FW);

	/*judge the fw that will be upgraded
	* if illegal, then stop upgrade and return.
	*/
	if (fw_len < 8 || fw_len > 32 * 1024) {
		MP_DEBUG1( "%s:FW length error\n", __func__);
		return ERR;
	}

	if ((CTPM_FW[fw_len - 8] ^ CTPM_FW[fw_len - 6]) == 0xFF
		&& (CTPM_FW[fw_len - 7] ^ CTPM_FW[fw_len - 5]) == 0xFF
		&& (CTPM_FW[fw_len - 3] ^ CTPM_FW[fw_len - 4]) == 0xFF) {
		/*FW upgrade */
		pbt_buf = CTPM_FW;
		/*call the upgrade function */
		i_ret = fts_ctpm_fw_upgrade(pbt_buf, sizeof(CTPM_FW));
		if (i_ret != 0)
			MP_DEBUG1("%s:upgrade failed. err.\n",
					__func__);
#ifdef AUTO_CLB
		else
			fts_ctpm_auto_clb(client);	/*start auto CLB */
#endif
	} else {
		MP_DEBUG1( "%s:FW format error\n", __func__);
		return ERR;
	}

	return i_ret;
}
BYTE fts_ctpm_get_i_file_ver(void)
{
	WORD ui_sz;
	ui_sz = sizeof(CTPM_FW);
	if (ui_sz > 2)
		return CTPM_FW[ui_sz - 2];

	return 0x00;	/*default value */
}


int fts_ctpm_auto_upgrade()
{
	BYTE uc_host_fm_ver = FT5x0x_REG_FW_VER;
	BYTE uc_tp_fm_ver;
	int i_ret;

	uc_tp_fm_ver=I2CM_RdReg8Data8(FT5306_I2C_ADDRESS, FT5x0x_REG_FW_VER);
	mpDebugPrint("uc_tp_fm_ver=%x",uc_tp_fm_ver);
	uc_host_fm_ver = fts_ctpm_get_i_file_ver();
	if (/*the firmware in touch panel maybe corrupted */
		uc_tp_fm_ver == FT5x0x_REG_FW_VER ||
		/*the firmware in host flash is new, need upgrade */
	     uc_tp_fm_ver < uc_host_fm_ver
	    ) {
		TaskSleep(100);
		MP_DEBUG2( "[FTS] uc_tp_fm_ver = 0x%x, uc_host_fm_ver = 0x%x",
				uc_tp_fm_ver, uc_host_fm_ver);
		i_ret = fts_ctpm_fw_upgrade_with_i_file();
		if (i_ret == 0)	{
			TaskSleep(300);
			uc_host_fm_ver = fts_ctpm_get_i_file_ver();
			MP_DEBUG1( "[FTS] upgrade to new version 0x%x\n",
					uc_host_fm_ver);
		} else {
			MP_DEBUG1("[FTS] upgrade failed ret=%d.\n", i_ret);
			return ERR;
		}
	}

	return 0;
}

#ifndef FOR_TOUCH_CONTROLLRT_TASK
void read_data_test(void)
{
    Tc_point data;

    FT5306_get_point(&data);

    mpDebugPrintN("x = %d, y = %d\r\n", data.x1, data.y1);
}
#endif


void TouchIntEnable()
{
	//Gpio_IntConfig(gpioIntNum, GPIO_ACTIVE_LOW, TC_TRIGGER_MODE);
        Gpio_IntEnable(gpioIntNum);

}
/****************************************************************************
 **
 ** NAME:           TcDebounceTimerIsr
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  return msg ID
 **
 ** DESCRIPTION:    Isr of Tc Debounce timer.
 **
 ****************************************************************************/
static void TcDebounceTimerIsr(WORD noUse)
{
    ST_TC_MSG tcMessage;
	static DWORD touchintTime=0;
	static WORD i;
        Gpio_IntDisable(gpioIntNum);
        //Gpio_IntConfig(gpioIntNum, GPIO_ACTIVE_LOW, TC_TRIGGER_MODE);
#if 0        
	if (SystemGetElapsedTime(touchintTime)>2000)
		i=0;
	else
		i++;
	touchintTime=GetSysTime();
	mpDebugPrintN("i=%d\r\n",i);
	if (i>1000)
	{
		Ui_TimerProcAdd(2000, TouchIntEnable);
		return;
	}
#endif	
        tcMessage.status = TC_DOWN;//TC_INT;

   	#define FOR_TOUCH_CONTROLLRT_TASK
    #ifdef FOR_TOUCH_CONTROLLRT_TASK
        MessageDrop(TcGetMsgId(), (BYTE *) & tcMessage, 1);
        //Gpio_IntEnable(gpioIntNum);
	Ui_TimerProcAdd(20, TouchIntEnable);
    #else
        read_data_test();
        //Gpio_IntEnable(gpioIntNum);
	Ui_TimerProcAdd(1, TouchIntEnable);

    #endif
}



/****************************************************************************
 **
 ** NAME:           FT5306_get_value
 **
 ** PARAMETERS:     val[]:the value from chip, cnt:value count
 **
 ** RETURN VALUES:  The result of sample
 **
 ** DESCRIPTION:    get cnt samples average value, and do not consider max and min
 **
 ****************************************************************************/
static WORD FT5306_get_value(WORD val[], WORD cnt)
{
    DWORD i, ret = 0, max = 0, min = 0xffffffff;

    if ( cnt <= 3 )
    {
        for(i = 0; i < cnt; i++)
            ret += val[i];

        ret /= cnt;
    }
    else
    {
        for(i = 0; i < cnt; i++)
        {
            ret += val[i];
            if(val[i] > max)
                max = val[i];
            if(val[i] < min)
                min = val[i];
        }
        ret = (ret - (max + min)) / (cnt - 2);
    }

    return ret;
}

/****************************************************************************
 **
 ** NAME:           TcIsr
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    The interrupt service routine of Touch screen controller.
 **                 Set low level trigger when pen not be touched.
 **                 Set high level trigger when pen be touched.
 **
 ****************************************************************************/

/****************************************************************************
 **
 ** NAME:           FT5306_Sleep
 **
 ** PARAMETERS:     sleep: TRUE or FALSE
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Enter power saving mode of FT5306
 **
 ****************************************************************************/
 BOOL Tc_status=0;
static SDWORD FT5306_Sleep(BYTE sleep)
{
    BYTE cmd;
    if(sleep)
    {
		Tc_status=0;
        cmd = FT5306_I2C_CMD_SLEEP;
        I2CM_Write_BustMode(FT5306_I2C_ADDRESS, &cmd, 1);
        Gpio_IntDisable(gpioIntNum);
    }
    else
    {
		Tc_status=1;
        cmd = FT5306_I2C_CMD_INIT;
        I2CM_Write_BustMode(FT5306_I2C_ADDRESS, &cmd, 1);
        Gpio_IntEnable(gpioIntNum);
    }

    return 0;
}

/****************************************************************************
 **
 ** NAME:           FT5306_Check_Intterupt
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Check interrupt pin of FT5306
 **
 ****************************************************************************/
static SDWORD FT5306_Check_Interrupt(void)
{
    WORD       data;

    Gpio_DataGet(gpioNum, &data, 1);
    if(data)
    {
        return 1;       // no touch
    }
    else
    {
        return 0;       // touch
    }
}

/****************************************************************************
 **
 ** NAME:           FT5306_get_point
 **
 ** PARAMETERS:     data: the sapce of data to return
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    The procedure of getting data from FT5306.
 **
 ****************************************************************************/
#if 1
static SDWORD FT5306_get_point(Tc_point * data)
{
#if 0
	WORD x, y, xh,xl,yh,yl;
#else
	BYTE dataBuf[5];
#endif
	if (Tc_status==0) return 0;
//		Gpio_IntDisable(gpioIntNum);        //avoid interrupt when getting data
#if 0		
		xh=I2CM_RdReg8Data8(FT5306_I2C_ADDRESS,0x03);
		xl=I2CM_RdReg8Data8(FT5306_I2C_ADDRESS,0x04);
		yh=I2CM_RdReg8Data8(FT5306_I2C_ADDRESS,0x05);
		yl=I2CM_RdReg8Data8(FT5306_I2C_ADDRESS,0x06);
		x = (xh&0x0F)<<8|xl;
		y = (yh&0x0F)<<8|yl;
		data->x1=x;
		data->y1=y;
		data->reserved=xh&0xf0;
		//mpDebugPrint("x=%d,y=%d,status=%x",x,y,xh&0xf0);
#endif

		if(I2CM_Read_BustMode(FT5306_I2C_ADDRESS, 0x1003, 1, &dataBuf[0],1) == PASS)
		{
			//mpDebugPrint("dataBuf[0]=%d,",dataBuf[0]);		

               }
		if(I2CM_Read_BustMode(FT5306_I2C_ADDRESS, 0x1004, 1, &dataBuf[1],1) == PASS)
		{
			//mpDebugPrint("dataBuf[1]=%d,",dataBuf[1]);
			data->x1=((dataBuf[1]&0xff)<<8)|(dataBuf[0]&0xff);
               }
		if(I2CM_Read_BustMode(FT5306_I2C_ADDRESS, 0x1005, 1, &dataBuf[2],1) == PASS)
		{
			//mpDebugPrint("dataBuf[2]=%d,",dataBuf[2]);		
               }
		if(I2CM_Read_BustMode(FT5306_I2C_ADDRESS, 0x1006, 1, &dataBuf[3],1) == PASS)
		{
			data->y1=(dataBuf[3]&0xFF)<<8|dataBuf[2];
			//mpDebugPrint("dataBuf[3]=%d,",dataBuf[3]);		
               }
		if(I2CM_Read_BustMode(FT5306_I2C_ADDRESS, 0x1008, 1, &dataBuf[4],1) == PASS)
		{
			switch(dataBuf[4])
			{
				case 1:
					data->reserved=TC_DOWN;
				break;
				case 2:
					data->reserved=TC_POLLING;
				break;
				case 4:
					data->reserved=TC_UP;
				break;
				default:
					data->reserved=dataBuf[4];
				break;
	
			}
			
			//mpDebugPrint("dataBuf[4]=%d,",dataBuf[4]);		
			//mpDebugPrint("x=%d,y=%d,status=%x",data->x1,data->y1,data->reserved);		
               }
	//	Gpio_IntConfig(gpioIntNum, GPIO_ACTIVE_LOW, TC_TRIGGER_MODE);
		//Gpio_IntEnable(gpioIntNum);

		return 0;

}
#else
static SDWORD FT5306_get_point(Tc_point * data)
{
    BYTE i;
    WORD val[SAMPLE_ARRAY_SIZE];
    WORD x, y, z1, z2,xh,xl,yh,yl,x_ori,y_ori;
    WORD cmd[8],dataBuf[8];
    //mpDebugPrint("SampleBucket.num =%d",SampleBucket.num);
    if ( SampleBucket.num < SAMPLE_ARRAY_SIZE )
    {
        if (!FT5306_Check_Interrupt())
        {
            Gpio_IntDisable(gpioIntNum);        //avoid interrupt when getting data
            do
            {
                /*I2CM_ReadData_BustMode(FT5306_I2C_ADDRESS, dataBuf, 8);
                {
                    x = ((dataBuf[3]&0x0F)<<8)|dataBuf[4];
                    y = ((dataBuf[5]&0x0F)<<8)|dataBuf[4];
                }*/
		  //mpDebugPrint("dataBuf[5]=0x%x",dataBuf[5]);
                //mpDebugPrint("Device mode is %d",dataBuf[0]);
		  //mpDebugPrint("Gest ID is %d",dataBuf[1]);
                //mpDebugPrint("%d Touch Points",dataBuf[2]);

		  //mpDebugPrint("###xh is %x,xl is %x,  yh is%x,  yl is%x",xh&0x0F,xl,yh&0x0F,yl);
                //mpDebugPrint("###x_ori is %d,  y_ori is%d",(xh&0x0F)<<8|xl,(yh&0x0F)<<8|yl);
		  #if 1
                xh=I2CM_RdReg8Data8(FT5306_I2C_ADDRESS,0x03);
		  xl=I2CM_RdReg8Data8(FT5306_I2C_ADDRESS,0x04);
                yh=I2CM_RdReg8Data8(FT5306_I2C_ADDRESS,0x05);
		  yl=I2CM_RdReg8Data8(FT5306_I2C_ADDRESS,0x06);
                x = (xh&0x0F)<<8|xl;
                y = (yh&0x0F)<<8|yl;

	 	  //mpDebugPrint("###xh is %x,xl is %x,  yh is%x,  yl is%x",xh&0x0F,xl,yh&0x0F,yl);
        	  //mpDebugPrint("###x_ori is %d,  y_ori is %d, x=%d,y=%d",x_ori,y_ori,x,y);
		  #else
		  //SW_FT5306_I2CM_Read_BustMode(FT5306_I2C_ADDRESS,dataBuf,8);
		  I2CM_Read_BustMode(FT5306_I2C_ADDRESS, 0x03,0,dataBuf, 4);
                //I2CM_ReadData_BustMode(FT5306_I2C_ADDRESS, dataBuf, 8);

				
                {
		      if(dataBuf[5]>2)
			  dataBuf[5]--;
                    x = ((dataBuf[3]&0x0F)<<8)|dataBuf[4];
                    y = ((dataBuf[5]&0x0F)<<8)|dataBuf[4];
                }
		  #endif
		  //mpDebugPrint("###??? x is %d,  y is%d dataBuf[5]=%x",x,y,dataBuf[5]);
                SampleBucket.SampleAry[SampleBucket.index].x = x;
                SampleBucket.SampleAry[SampleBucket.index].y = y;
                SampleBucket.index++;
                if (SampleBucket.index >= SAMPLE_ARRAY_SIZE)
                    SampleBucket.index = 0;
            } while(++SampleBucket.num < SAMPLE_ARRAY_SIZE);
            if (FT5306_Check_Interrupt())
            {
                Gpio_IntConfig(gpioIntNum, GPIO_ACTIVE_LOW, TC_TRIGGER_MODE);
                u08IntModeFlag = INT_LOW_LEVEL;
                SampleBucket.num = 0;//Clear number to zero
                SampleBucket.index = 0;
                Gpio_IntEnable(gpioIntNum);

                return 1;
            }
        }
        else
            return 1;
    }
    else
    {
        Gpio_IntDisable(gpioIntNum);        //avoid interrupt when getting data
        


      #if 1
        xh=I2CM_RdReg8Data8(FT5306_I2C_ADDRESS,0x03);
        xl=I2CM_RdReg8Data8(FT5306_I2C_ADDRESS,0x04);
        yh=I2CM_RdReg8Data8(FT5306_I2C_ADDRESS,0x05);
        yl=I2CM_RdReg8Data8(FT5306_I2C_ADDRESS,0x06);
        x = (xh&0x0F)<<8|xl;
        y = (yh&0x0F)<<8|yl;
	#else
	 /*I2CM_ReadData_BustMode(FT5306_I2C_ADDRESS, &cmd, 8);
        xh=cmd[3];//I2CM_RdReg8Data8(FT5306_I2C_ADDRESS,0x03);
        xl=cmd[4];//I2CM_RdReg8Data8(FT5306_I2C_ADDRESS,0x04);
        yh=cmd[5];//I2CM_RdReg8Data8(FT5306_I2C_ADDRESS,0x05);
        yl=cmd[6];//I2CM_RdReg8Data8(FT5306_I2C_ADDRESS,0x06);
        x = (xh&0x0F)<<8|xl;
        y = (yh&0x0F)<<8|yl;*/
        I2CM_ReadData_BustMode(FT5306_I2C_ADDRESS, dataBuf, 8);
        {
            if(dataBuf[5]>2)
	     dataBuf[5]--;
            x = ((dataBuf[3]&0x0F)<<8)|dataBuf[4];
            y = ((dataBuf[5]&0x0F)<<8)|dataBuf[4];
        }
	#endif
	 SampleBucket.SampleAry[SampleBucket.index].x = x;
        SampleBucket.SampleAry[SampleBucket.index].y = y;
        SampleBucket.index++;
        if ( SampleBucket.index >= SAMPLE_ARRAY_SIZE )
            SampleBucket.index = 0;
    }
    //mpDebugPrint("SampleBucket.num===%d",SampleBucket.num);
    for(i = 0; i < SampleBucket.num; i++)
        val[i] = SampleBucket.SampleAry[i].x;
    
    data->x1 = FT5306_get_value(val, SampleBucket.num);

    for(i = 0; i < SampleBucket.num; i++)
        val[i] = SampleBucket.SampleAry[i].y;
    data->y1 = FT5306_get_value(val, SampleBucket.num);
    Gpio_IntEnable(gpioIntNum);

    return 0;
}

#endif

/****************************************************************************
 **
 ** NAME:           FT5306_change_sensitivity
 **
 ** PARAMETERS:     x_sen : Sensitivity of X-axix .
 **                 y_sen : Sensitivity of Y-axix .
 **                 z_sen : Sensitivity of Z-axix .
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Change sensitivity of touch panel
 **
 ****************************************************************************/
static SDWORD FT5306_change_sensitivity(DWORD x_sen, DWORD y_sen, DWORD z_sen)
{
    return 0;
}



/****************************************************************************
 **
 ** NAME:           FT5306_Init
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Init FT5306. and Setting the mode of interrupt pin.
 **
 ****************************************************************************/
static SDWORD FT5306_Init(void)
{
    MP_DEBUG("__%s__", __FUNCTION__);
    SDWORD       status;
    DWORD cmd;


    u08IntModeFlag = INT_LOW_LEVEL;
	Gpio_DataSet(TOUCH_RESET_PIN, GPIO_DATA_LOW, 1);
	IODelay(1000);
	Gpio_DataSet(TOUCH_RESET_PIN, GPIO_DATA_HIGH, 1);
    gpioNum = GetGPIOPinIndex(TOUCH_CONTROLLER_INT_PIN);


    //BYTE cmd = FT5306_I2C_CMD_INIT;
    //I2CM_Write_BustMode(FT5306_I2C_ADDRESS, &cmd, 1);
	fts_ctpm_auto_upgrade();

    Gpio_Config2GpioFunc(gpioNum, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1);
    gpioIntNum = GetGpioIntIndexByGpioPinNum(gpioNum);
    Gpio_IntConfig(gpioIntNum, GPIO_ACTIVE_LOW, TC_TRIGGER_MODE);
    Gpio_IntCallbackFunRegister(gpioIntNum, TcDebounceTimerIsr);

    Gpio_IntEnable(gpioIntNum);
//    SystemIntEna(IM_GPIO);
    return 0;
}



/****************************************************************************
 **
 ** NAME:           TcDriverInit
 **
 ** PARAMETERS:     stTcDriver
 **
 ** RETURN VALUES:  result
 **
 ** DESCRIPTION:    constructor of TS driver.
 **
 ****************************************************************************/
SDWORD TcDriverInit(ST_TC_DRIVER * stTcDriver)
{
    stTcDriver->u08Name = "FT5306 Touch screen device driver";
    stTcDriver->TcInit = FT5306_Init;
    stTcDriver->TcSleep = FT5306_Sleep;
    stTcDriver->TcGetData = FT5306_get_point;
    stTcDriver->TcCheckInterrupt = FT5306_Check_Interrupt;
    stTcDriver->TcChangeSensitivity = FT5306_change_sensitivity;


    return 0;
}


#endif //#if (TOUCH_CONTROLLER == TOUCH_PANEL_DRIVER_FT5306)


