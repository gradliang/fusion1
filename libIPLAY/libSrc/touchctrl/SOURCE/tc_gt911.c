/*
*******************************************************************************
*                           Magic Pixel Inc.                                  *
*                  Copyright (c) 2007 -, All Rights Reserved                  *
*                                                                             *
* TS module driver.                                                           *
*                                                                             *
* File : Tc_GT911.c                                                          *
* By :                                                                        *
*                                                                             *
*                                                                             *
* Description : GT911 touch screen controller driver                     *
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

#if ((TOUCH_CONTROLLER_ENABLE == ENABLE) && (TOUCH_CONTROLLER == TOUCH_PANEL_DRIVER_GT911))
#include "mpTrace.h"
#include "os.h" 
#include "taskid.h"
#include "peripheral.h"
#include "tc_driver.h"
#include "touchCtrller.h"
#include "Ui.h"
#include "Tc_gt911.h"


#if 1 //use SW i2c

#define IIC_FLAG_NO_ACK		1
#define IIC_FLAG_ACK		2
#define IIC_FLAG_START		3
#define IIC_FLAG_STOP		4
#define IIC_FLAG_IGNORE		5

#define WAIT_TIME_IIC_DELAY	2//35  //20 //5
#define WAIT_TIME_WAIT_ACK	100//200  //160 //40
#define WAIT_TIME_WAIT_ACK_RETRY	10

//--------------------------------------------------------
BYTE ACK1=1, ACK2=1, ACK3=1;

//////////////////////////////////////////
// software simulate i2c base
//////////////////////////////////////////
#if (PRODUCT_PCBA==PCBA_MAIN_BOARD_V12)

#define __GT911_i2c_data_high()	(g_psGpio->Gpdat0 |= 0x00000001)
#define __GT911_i2c_data_low()		(g_psGpio->Gpdat0 &= 0xfffffffe)
#define __GT911_i2c_data_output()	(g_psGpio->Gpdat0 |= 0x00010000)
#define __GT911_i2c_data_input()	(g_psGpio->Gpdat0 &= 0xfffeffff)

#define __GT911_i2c_clk_high()		(g_psGpio->Gpdat0 |= 0x00000002)
#define __GT911_i2c_clk_low()		(g_psGpio->Gpdat0 &= 0xfffffffd)
#define __GT911_i2c_clk_output()	(g_psGpio->Gpdat0 |= 0x00020000)
#define __GT911_i2c_clk_input()	(g_psGpio->Gpdat0 &= 0xfffdffff)


#define __GT911_i2c_read_data()   (g_psGpio->Gpdat0 & 0x00000001)
#define __GT911_i2c_all_output()	(g_psGpio->Gpdat0 |= 0x00030000)
#define __GT911_i2c_set_gpio_mode() (g_psGpio->Gpcfg0 = (g_psGpio->Gpcfg0 & 0xfffcfffc) )

#else

#define __GT911_i2c_data_high()	(g_psGpio->Gpdat0 |= 0x00000002)
#define __GT911_i2c_data_low()		(g_psGpio->Gpdat0 &= 0xfffffffd)
#define __GT911_i2c_data_output()	(g_psGpio->Gpdat0 |= 0x00020000)
#define __GT911_i2c_data_input()	(g_psGpio->Gpdat0 &= 0xfffdffff)

#define __GT911_i2c_clk_high()		(g_psGpio->Gpdat0 |= 0x00000001)
#define __GT911_i2c_clk_low()		(g_psGpio->Gpdat0 &= 0xfffffffe)
#define __GT911_i2c_clk_output()	(g_psGpio->Gpdat0 |= 0x00010000)
#define __GT911_i2c_clk_input()	(g_psGpio->Gpdat0 &= 0xfffeffff)


#define __GT911_i2c_read_data()   ((g_psGpio->Gpdat0 & 0x00000002)>>1)
#define __GT911_i2c_all_output()	(g_psGpio->Gpdat0 |= 0x00030000)
#define __GT911_i2c_set_gpio_mode() (g_psGpio->Gpcfg0 = (g_psGpio->Gpcfg0 & 0xfffcfffc) )

#endif 

static void GT911_i2c_data_high(void)
{
	__GT911_i2c_data_high();		IODelay(WAIT_TIME_IIC_DELAY);
}
static void GT911_i2c_data_low(void)
{
	__GT911_i2c_data_low();		IODelay(WAIT_TIME_IIC_DELAY);
}
static void GT911_i2c_data_output(void)
{
	__GT911_i2c_data_output();		IODelay(WAIT_TIME_IIC_DELAY);
}
static void GT911_i2c_data_input(void)
{
	__GT911_i2c_data_input();		IODelay(WAIT_TIME_IIC_DELAY);
}
static void GT911_i2c_clk_high(void)
{
	__GT911_i2c_clk_high();		IODelay(WAIT_TIME_IIC_DELAY);
}
static void GT911_i2c_clk_low(void)
{
	__GT911_i2c_clk_low();			IODelay(WAIT_TIME_IIC_DELAY);
}
static void GT911_i2c_clk_output(void)
{
	__GT911_i2c_clk_output();		IODelay(WAIT_TIME_IIC_DELAY);
}
static void GT911_i2c_clk_input(void)
{
	__GT911_i2c_clk_input();		IODelay(WAIT_TIME_IIC_DELAY);
}


static BYTE SW_GT911_IIC_SetValue(BYTE data1, BYTE condition)
{
    BYTE mask;
    BYTE ret;
	
    __GT911_i2c_set_gpio_mode();	// set to GPIO function mode
    __GT911_i2c_all_output();		// GP0, GP1 output

	if(condition == IIC_FLAG_START)
	{
		//start condition
		GT911_i2c_clk_high();
		GT911_i2c_data_high();
		IODelay(WAIT_TIME_WAIT_ACK);
		GT911_i2c_data_low();
		IODelay(WAIT_TIME_WAIT_ACK);
	}
	GT911_i2c_clk_low();
	IODelay(WAIT_TIME_WAIT_ACK);
	mask = 0x80;
	while(mask > 0)
	{
		if((mask & data1) == mask)
			GT911_i2c_data_high();
		else 
			GT911_i2c_data_low();
		GT911_i2c_clk_high();
		GT911_i2c_clk_low();
		mask >>= 1;
	}
	
	//cs4349_i2c_data_high();
	__GT911_i2c_data_input();		// data = input direction
#ifdef WAIT_TIME_WAIT_ACK_RETRY
	IODelay(WAIT_TIME_WAIT_ACK>>2);
	mask=150;
	while (mask--)
	{
		IODelay(WAIT_TIME_WAIT_ACK_RETRY);
		if (__GT911_i2c_read_data()==0)
			break;
	}
	GT911_i2c_clk_high();
#else
	IODelay(WAIT_TIME_WAIT_ACK);
	GT911_i2c_clk_high();
	//IODelay(40);  //grad mark
	//IODelay(10);
	IODelay(WAIT_TIME_WAIT_ACK>>2); //Tech use
#endif
	if(__GT911_i2c_read_data())
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
		__GT911_i2c_all_output();		// GP0, GP1 output
		ret = IIC_FLAG_ACK;
		//__cs4349_i2c_all_output();	// GP0, GP1 output
    }
        
    GT911_i2c_clk_low();
	GT911_i2c_data_low();   //along add 20100325 avoid data is high due to not stop
	IODelay(WAIT_TIME_WAIT_ACK);   //grad add

    if(condition == IIC_FLAG_STOP){
        GT911_i2c_clk_high();
        GT911_i2c_data_high();
    }

    return ret; //IIC_ACK;
}

static BYTE SW_GT911_IIC_SetWord(WORD wData, BYTE condition)
{
    WORD mask;
    BYTE ret;
	
    __GT911_i2c_set_gpio_mode();	// set to GPIO function mode
    __GT911_i2c_all_output();		// GP0, GP1 output

	if(condition == IIC_FLAG_START)
	{
		//start condition
		GT911_i2c_clk_high();
		GT911_i2c_data_high();
		GT911_i2c_data_low();
		GT911_i2c_clk_low();
	}
	GT911_i2c_clk_low();
	mask = 0x8000;
	while(mask > 0)
	{
		if((mask & wData) == mask)
			GT911_i2c_data_high();
		else 
			GT911_i2c_data_low();
		GT911_i2c_clk_high();
		GT911_i2c_clk_low();
		mask >>= 1;
	}
	
	//cs4349_i2c_data_high();
	__GT911_i2c_data_input();		// data = input direction
#ifdef WAIT_TIME_WAIT_ACK_RETRY
	IODelay(WAIT_TIME_WAIT_ACK>>2);
	mask=150;
	while (mask--)
	{
		IODelay(WAIT_TIME_WAIT_ACK_RETRY);
		if (__GT911_i2c_read_data()==0)
			break;
	}
	GT911_i2c_clk_high();
#else
	IODelay(WAIT_TIME_WAIT_ACK);
	GT911_i2c_clk_high();
	//IODelay(40);  //grad mark
	//IODelay(10);
	IODelay(WAIT_TIME_WAIT_ACK>>2); //Tech use
#endif
	if(__GT911_i2c_read_data())
	{
		char tmpbuf[64];
		UartOutText("I2C ACK error,");
		sprintf(tmpbuf,"data=%p,condition=%d \r\n",wData,condition);
		UartOutText(tmpbuf);
		//__asm("break 100");
		ret = IIC_FLAG_NO_ACK;
	}
	else
	{
		//UartOutText("ACK OK");
		__GT911_i2c_all_output();		// GP0, GP1 output
		ret = IIC_FLAG_ACK;
		//__cs4349_i2c_all_output();	// GP0, GP1 output
    }
        
    GT911_i2c_clk_low();
	GT911_i2c_data_low();   //along add 20100325 avoid data is high due to not stop
	IODelay(WAIT_TIME_WAIT_ACK);   //grad add

    if(condition == IIC_FLAG_STOP){
        GT911_i2c_clk_high();
        GT911_i2c_data_high();
    }

    return ret; //IIC_ACK;
}

static BYTE SW_CS4349_IIC_GetValue_NoStop()		// get value, but no stop bit
{
    BYTE data, i;
    
    __GT911_i2c_set_gpio_mode();	//set to GPIO function mode
    __GT911_i2c_data_input();		// DATA = input
    __GT911_i2c_clk_output();		// CLK = output

	GT911_i2c_clk_low();
    data = 0;
    for(i = 0; i < 8; i++){
        data <<= 1;
        //cs4349_i2c_clk_low();
        GT911_i2c_clk_high();
        data |= __GT911_i2c_read_data();
		GT911_i2c_clk_low();
    }
    // ACK
    GT911_i2c_clk_low();
	IODelay(WAIT_TIME_WAIT_ACK);
    __GT911_i2c_all_output();		// GP0, GP1 output
    //cs4349_i2c_data_low(); // ACK    
    GT911_i2c_data_low(); // ACK    grad
    GT911_i2c_clk_high();
	GT911_i2c_clk_low();
	IODelay(WAIT_TIME_WAIT_ACK);
    return data;
}



static BYTE SW_GT911_IIC_GetValue()
{
    BYTE data, i;
    
    __GT911_i2c_set_gpio_mode();	//set to GPIO function mode
    __GT911_i2c_data_input();  // DATA = input
    __GT911_i2c_clk_output();  // CLK = output

    GT911_i2c_clk_low();
    data = 0;
    for(i = 0; i < 8; i++){
        data <<= 1;        
       // cs4349_i2c_clk_low();
        GT911_i2c_clk_high();
        data |= __GT911_i2c_read_data();
        GT911_i2c_clk_low();		
    }
    // ACK
    GT911_i2c_clk_low();
	IODelay(WAIT_TIME_WAIT_ACK);
    __GT911_i2c_all_output();  // GP0, GP1 output
    //cs4349_i2c_data_low(); // ACK    

//frank mask    cs4349_i2c_data_high(); // ACK    grad
//frank mask     cs4349_i2c_clk_high();
	
    //__cs4349_i2c_data_input();  // GP1 = input
    
    
    GT911_i2c_clk_low();
	GT911_i2c_data_low();   //along add 20100325 avoid data is high due to not stop
	IODelay(WAIT_TIME_WAIT_ACK);
    GT911_i2c_clk_high();
    GT911_i2c_data_high();
    
    return data;
}


static BYTE GT911_setRegister(BYTE addr, BYTE data)
{
	ACK1 = SW_GT911_IIC_SetValue(GT911_I2C_ADDRESS, IIC_FLAG_START); 
	ACK2 = SW_GT911_IIC_SetValue(addr, IIC_FLAG_IGNORE);
	ACK3 = SW_GT911_IIC_SetValue(data, IIC_FLAG_STOP);
}
    

static BYTE GT911_readRegister(BYTE addr)
{
	BYTE value;
	ACK1 = SW_GT911_IIC_SetValue(GT911_I2C_ADDRESS, IIC_FLAG_START);
	ACK2 = SW_GT911_IIC_SetValue(addr, IIC_FLAG_STOP);
	//__cs4349_i2c_data_high();
	ACK3 = SW_GT911_IIC_SetValue((GT911_I2C_ADDRESS) | 0x01, IIC_FLAG_START);
	value = SW_GT911_IIC_GetValue();
	return value;
}

static SWORD GT911_setRegisterBytes(BYTE *bBufer,BYTE lens)
{
	BYTE i;

	if (!lens)
		return FAIL;
	ACK1 = SW_GT911_IIC_SetValue(GT911_I2C_ADDRESS, IIC_FLAG_START); 
	//ACK2 = SW_GT911_IIC_SetValue(addr, IIC_FLAG_IGNORE);
	for (i=0;i<lens-1;i++)
	{
		ACK2 = SW_GT911_IIC_SetValue(bBufer[i], IIC_FLAG_IGNORE);
	}
	ACK3 = SW_GT911_IIC_SetValue(bBufer[i], IIC_FLAG_STOP);
	
	if (ACK1 != IIC_FLAG_ACK || ACK2 != IIC_FLAG_ACK || ACK3 != IIC_FLAG_ACK)
		return FAIL;
	return PASS;
}

static SWORD GT911_ReadRegisterBytes(BYTE addr,BYTE *bBufer,BYTE lens)
{
	BYTE i;

	if (!lens)
		return FAIL;
	ACK1 = SW_GT911_IIC_SetValue(GT911_I2C_ADDRESS, IIC_FLAG_START);
	ACK2 = SW_GT911_IIC_SetValue(addr, IIC_FLAG_STOP);
	ACK3 = SW_GT911_IIC_SetValue((GT911_I2C_ADDRESS) | 0x01, IIC_FLAG_START);
	for (i=0;i<lens-1;i++)
	{
		bBufer[i] = SW_CS4349_IIC_GetValue_NoStop();
	}
	bBufer[lens-1]=SW_GT911_IIC_GetValue();
	if (ACK1 != IIC_FLAG_ACK || ACK2 != IIC_FLAG_ACK || ACK3 != IIC_FLAG_ACK)
		return FAIL;
	return PASS;
}

static SWORD GT911_OnlyReadRegisterBytes(BYTE *bBufer,BYTE lens)
{
	BYTE i;

	if (!lens)
		return FAIL;
	ACK3 = SW_GT911_IIC_SetValue((GT911_I2C_ADDRESS) | 0x01, IIC_FLAG_START);
	for (i=0;i<lens-1;i++)
	{
		bBufer[i] = SW_CS4349_IIC_GetValue_NoStop();
	}
	bBufer[lens-1]=SW_GT911_IIC_GetValue();
	if (ACK3 != IIC_FLAG_ACK)
		return FAIL;
	return PASS;
}

#endif

//--------------------------------------------------------

BYTE I2CM_Touch_RdReg(BYTE DevID, BYTE reg)
{
	return GT911_readRegister(reg);
	//return I2CM_RdReg(DevID,reg);
}

BYTE I2CM_GT911_RdReg(WORD wReg)
{

	BYTE value;

	ACK1 = SW_GT911_IIC_SetValue(GT911_I2C_ADDRESS, IIC_FLAG_START);
	//ACK2 = SW_GT911_IIC_SetWord(wReg, IIC_FLAG_STOP);
	ACK2 = SW_GT911_IIC_SetValue(wReg>>8, IIC_FLAG_IGNORE);
	ACK2 = SW_GT911_IIC_SetValue(wReg&0xff, IIC_FLAG_STOP);
	ACK3 = SW_GT911_IIC_SetValue((GT911_I2C_ADDRESS) | 0x01, IIC_FLAG_START);
	value = SW_GT911_IIC_GetValue();
	return value;
}

SWORD I2CM_GT911_RdRegisters(WORD wReg,BYTE *bBufer,BYTE lens)
{

	ACK1 = SW_GT911_IIC_SetValue(GT911_I2C_ADDRESS, IIC_FLAG_START);
	//ACK2 = SW_GT911_IIC_SetWord(wReg, IIC_FLAG_STOP);
	ACK2 = SW_GT911_IIC_SetValue(wReg>>8, IIC_FLAG_IGNORE);
	ACK2 = SW_GT911_IIC_SetValue(wReg&0xff, IIC_FLAG_STOP);
	//ACK3 = SW_GT911_IIC_SetValue((GT911_I2C_ADDRESS) | 0x01, IIC_FLAG_START);

	return GT911_OnlyReadRegisterBytes(bBufer,lens);
}

SWORD I2CM_GT911_WtRegisters(BYTE *bBufer,BYTE lens)
{

	return GT911_setRegisterBytes(bBufer,lens);

}



#define  KEYRELEASEDELAY													5
static DWORD gpioNum, gpioIntNum;
static BYTE st_bInReadTouchData=0,st_bKeyPressed=0;
static WORD st_wTouchX, st_wTouchY;
#if 1
void TimerToKeyRelease(void)
{
	if (st_bKeyPressed)
	{
		st_bKeyPressed--;
	}
}

void GT911_I2C_ReadTouchData() //from datasheet
{
	if (st_bInReadTouchData)
		return;
	st_bInReadTouchData=1;
	//UartOutText("I");
	DWORD i,dwLen,x,y,w;
	BYTE mult_tp_id,buf[20];
	BYTE  end_cmd[3] = {GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF, 0};
	ST_TC_MSG tcMessage;

		if (!st_bKeyPressed)
		{
	//dwLen=I2CM_GT911_RdReg(GTP_READ_COOR_ADDR)&0x0f;
	//mpDebugPrint(" dwLen=%p",I2CM_GT911_RdReg(GTP_READ_COOR_ADDR));
	I2CM_GT911_RdRegisters(GTP_READ_COOR_ADDR,buf,8);
	dwLen=buf[0]&0x0f;
	//mpDebugPrint(" dwLen=%p",buf[0]);

	if (dwLen)
	{
		/*
		if (g_bTouchDevelop && dwLen>1)
		{
			dwLen=2;
			I2CM_GT911_RdRegisters((GTP_READ_COOR_ADDR + 8),&buf[8],8*(dwLen-1));
		}
		else
		*/
			dwLen=1;

		for (i = 0; i < dwLen; i++)
		{
			x= buf[i*8+2] | (buf[i*8+3] << 8);
			y= buf[i*8+4] | (buf[i*8+5] << 8);
			st_wTouchX=x;
			st_wTouchY=y;
			//st_bKeyPressed=KEYRELEASEDELAY;
			//mpDebugPrint(" %d :(%d,%d)",buf[i*8+1],st_wTouchX,st_wTouchY);
		}
		tcMessage.status = TC_DOWN;
		MessageDrop(TcGetMsgId(), (BYTE *) &tcMessage, sizeof(ST_TC_MSG));
	}
		}
		st_bKeyPressed=KEYRELEASEDELAY;



	if (I2CM_GT911_WtRegisters(end_cmd, 3) < 0)
	   mpDebugPrint("I2C write end_cmd error!");
	st_bInReadTouchData=0;
}
#endif

static SDWORD GT911_get_point(Tc_point * data) // fiill data->x,data->y,data->reserved
{

data->x1 = st_wTouchX;//1024/1200=18/150 = 64/75
data->y1 = st_wTouchY;
//MP_ALERT("_get_point, x=%d,y=%d, x'=%d,y'=%d",sample_x,sample_y,data->x1,data->y1);
//	ISP_TP_UNLOCK();

return 0;
}


void gtp_read_version(void)
{
    SWORD ret = -1;
    BYTE  buf[10] = {0};

	ret=I2CM_GT911_RdRegisters(GTP_REG_VERSION,buf,sizeof(buf)/sizeof(BYTE));
   // ret = gtp_i2c_read(client, buf, sizeof(buf));
    if (ret < 0)
    {
        mpDebugPrint("GTP read version failed");
        return ;
    }

	//*version = (buf[7] << 8) | buf[6];
	//    mpDebugPrint("IC Version: %02x%02x%02x%02x_%02x%02x", buf[2], buf[3], buf[4], buf[5], buf[7], buf[6]);
	mpDebugPrint("TOUCH IC Version: %02x%02x", buf[5],buf[4]);
	mpDebugPrint("MAX_X=%d MAX_Y=%d", (DWORD)(buf[9]<<8)|buf[8],(DWORD)(buf[7]<<8)|buf[6]);
}

void TouchIntEnable()
{
	//Gpio_IntConfig(gpioIntNum, GPIO_ACTIVE_LOW, TC_TRIGGER_MODE);
	Gpio_IntEnable(gpioIntNum);
}

void TouchIntDisable()
{
	//Gpio_IntConfig(gpioIntNum, GPIO_ACTIVE_LOW, TC_TRIGGER_MODE);
	Gpio_IntDisable(gpioIntNum);
}

void ResetTouchPanel()
{
	mpDebugPrint("ResetTouchPanel");

	SetGPIOValue(TOUCH_RESET_PIN,0);
	//TimerDelay(50);
	SetGPIOValue(TOUCH_CONTROLLER_INT_PIN,0);
	TimerDelay(50);

	SetGPIOValue(TOUCH_RESET_PIN,0);
	IODelay(100); //min = 100us
	SetGPIOValue(TOUCH_RESET_PIN,1);
	TimerDelay(1000); //min 5ms
	//SetGPIODirection(TOUCH_CONTROLLER_INT_PIN,GPIO_INPUT);
	GetGPIOValue(TOUCH_CONTROLLER_INT_PIN);
	IODelay(100); //min = 100us

}

void TouchPanelInit()
{
         MP_ALERT("TouchPanelInit");
		 
	ResetTouchPanel(); //Reset before touch
	 //Gpio_Config2GpioFunc(SW_IIC_CLK_PIN, GPIO_INPUT_MODE, 1, 1);
	// Gpio_Config2GpioFunc(SW_IIC_DATA_PIN, GPIO_INPUT_MODE, 1, 1);// for 7D
	 //Gpio_Config2GpioFunc(SW_IIC_DATA_PIN, GPIO_OUTPUT_MODE, 1, 1);// for 10D
}

/****************************************************************************
 **
 **
 ****************************************************************************/
static SDWORD GT911_tp_Init(void)
{
    MP_DEBUG("__%s__", __FUNCTION__);

    TouchPanelInit();
    gpioNum = GetGPIOPinIndex(TOUCH_CONTROLLER_INT_PIN);
    Gpio_Config2GpioFunc(gpioNum, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1);
    gpioIntNum = GetGpioIntIndexByGpioPinNum(gpioNum);
    Gpio_IntConfig(gpioIntNum, GPIO_ACTIVE_LOW, GPIO_EDGE_TRIGGER);
    Gpio_IntCallbackFunRegister(gpioIntNum, GT911_I2C_ReadTouchData);

	//TimerDelay(10);
	gtp_read_version();

    TouchIntEnable();
  //  SetGPIOValue(GPIO_02, 1);
//    memset(&SampleBucketNt1103,0,sizeof(SampleBucketNt1103));

    return 0;
}

static SDWORD GT911_Sleep(BYTE sleep)
{
	if (sleep)
		TouchIntDisable();
	else
		TouchIntEnable();
}

static SDWORD GT911_Check_Interrupt(void)
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

static SDWORD GT911_change_sensitivity(DWORD x_sen, DWORD y_sen, DWORD z_sen)
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
    stTcDriver->TcInit = GT911_tp_Init;
    stTcDriver->TcSleep = GT911_Sleep;
    stTcDriver->TcGetData = GT911_get_point;
    stTcDriver->TcCheckInterrupt = GT911_Check_Interrupt;
    stTcDriver->TcChangeSensitivity = GT911_change_sensitivity;

    return 0;
}


#endif




