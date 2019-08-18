/*
*******************************************************************************
*                           Magic Pixel Inc.                                  *
*                  Copyright (c) 2007 -, All Rights Reserved                  *
*                                                                             *
* TS module driver.                                                           *
*                                                                             *
* File : Tc_ICN8502.c                                                          *
* By :                                                                        *
*                                                                             *
*                                                                             *
* Description : ICN8502 touch screen controller driver                     *
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

#if ((TOUCH_CONTROLLER_ENABLE == ENABLE) && (TOUCH_CONTROLLER == TOUCH_PANEL_DRIVER_ICN8502))
#include "mpTrace.h"
#include "os.h"
#include "taskid.h"
#include "peripheral.h"
#include "tc_driver.h"
#include "Tc_ICN8502.h"

/*************************************************************************/
/***               Public Variable Declaration                         ***/
/*************************************************************************/

/*************************************************************************/
/***               Internally Visible Constants and Static Data        ***/
/*************************************************************************/
/* ICN8502 command define */

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

static SDWORD ICN8502_change_sensitivity(DWORD, DWORD, DWORD);



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
		
		i_ret = I2CM_Read_Length_BustMode(ICN8502_I2C_ADDRESS, auc_i2c_write_buf, 4, 
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

		i_ret = I2CM_Read_Length_BustMode(ICN8502_I2C_ADDRESS, auc_i2c_write_buf, 4, 
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
	i_ret = I2CM_Read_Length_BustMode(ICN8502_I2C_ADDRESS, auc_i2c_write_buf, 4, 
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
			I2CM_WtReg8Data8(ICN8502_I2C_ADDRESS, 0xbc, FT_UPGRADE_AA);
		else
			I2CM_WtReg8Data8(ICN8502_I2C_ADDRESS, 0xfc, FT_UPGRADE_AA);
		TaskSleep(upgradeinfo.delay_aa);

		/*write 0x55 to register 0xfc */
		if (DEVICE_IC_TYPE == IC_FT6208)
			I2CM_WtReg8Data8(ICN8502_I2C_ADDRESS, 0xbc, FT_UPGRADE_55);
		else
			I2CM_WtReg8Data8(ICN8502_I2C_ADDRESS, 0xfc, FT_UPGRADE_55);

		TaskSleep(upgradeinfo.delay_55);
		/*********Step 2:Enter upgrade mode *****/
		auc_i2c_write_buf[0] = FT_UPGRADE_55;
		auc_i2c_write_buf[1] = FT_UPGRADE_AA;
		do {
			i++;
			i_ret = I2CM_Write_BustMode(ICN8502_I2C_ADDRESS, auc_i2c_write_buf, 2);
			TaskSleep(5);
		} while (i_ret < 0 && i < 5);


		/*********Step 3:check READ-ID***********************/

		
		TaskSleep(upgradeinfo.delay_readid);
		auc_i2c_write_buf[0] = 0x90;
		auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] =
			0x00;
		I2CM_Read_Length_BustMode(ICN8502_I2C_ADDRESS, auc_i2c_write_buf, 4, reg_val, 2);


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

	I2CM_Read_Length_BustMode(ICN8502_I2C_ADDRESS, auc_i2c_write_buf, 1, reg_val, 1);


	/*Step 4:erase app and panel paramenter area*/
	MP_DEBUG("Step 4:erase app and panel paramenter area\n");
	auc_i2c_write_buf[0] = 0x61;
	I2CM_Write_BustMode(ICN8502_I2C_ADDRESS, auc_i2c_write_buf, 1);	/*erase app area */
	TaskSleep(upgradeinfo.delay_earse_flash);
	/*erase panel parameter area */
	auc_i2c_write_buf[0] = 0x63;
	I2CM_Write_BustMode(ICN8502_I2C_ADDRESS, auc_i2c_write_buf, 1);
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
		
		I2CM_Write_BustMode(ICN8502_I2C_ADDRESS, packet_buf, FTS_PACKET_LENGTH + 6);
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

		I2CM_Write_BustMode(ICN8502_I2C_ADDRESS, packet_buf, temp + 6);
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
		I2CM_Write_BustMode(ICN8502_I2C_ADDRESS, packet_buf, 7);
		TaskSleep(20);
	}


	/*********Step 6: read out checksum***********************/
	/*send the opration head */
	MP_DEBUG("Step 6: read out checksum\n");
	auc_i2c_write_buf[0] = 0xcc;
	I2CM_Read_Length_BustMode(ICN8502_I2C_ADDRESS, auc_i2c_write_buf, 1, reg_val, 1);
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
	I2CM_Write_BustMode(ICN8502_I2C_ADDRESS, auc_i2c_write_buf, 1);
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

	uc_tp_fm_ver=I2CM_RdReg8Data8(ICN8502_I2C_ADDRESS, FT5x0x_REG_FW_VER);
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

    ICN8502_get_point(&data);

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
        tcMessage.status = TC_INT;

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
 ** NAME:           ICN8502_get_value
 **
 ** PARAMETERS:     val[]:the value from chip, cnt:value count
 **
 ** RETURN VALUES:  The result of sample
 **
 ** DESCRIPTION:    get cnt samples average value, and do not consider max and min
 **
 ****************************************************************************/
static WORD ICN8502_get_value(WORD val[], WORD cnt)
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
 ** NAME:           ICN8502_Sleep
 **
 ** PARAMETERS:     sleep: TRUE or FALSE
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Enter power saving mode of ICN8502
 **
 ****************************************************************************/
 BOOL Tc_status=0;
static SDWORD ICN8502_Sleep(BYTE sleep)
{
    BYTE cmd;
    if(sleep)
    {
		Tc_status=0;
        cmd = ICN8502_I2C_CMD_SLEEP;
        I2CM_Write_BustMode(ICN8502_I2C_ADDRESS, &cmd, 1);
        Gpio_IntDisable(gpioIntNum);
    }
    else
    {
		Tc_status=1;
        cmd = ICN8502_I2C_CMD_INIT;
        I2CM_Write_BustMode(ICN8502_I2C_ADDRESS, &cmd, 1);
        Gpio_IntEnable(gpioIntNum);
    }

    return 0;
}

/****************************************************************************
 **
 ** NAME:           ICN8502_Check_Intterupt
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Check interrupt pin of ICN8502
 **
 ****************************************************************************/
static SDWORD ICN8502_Check_Interrupt(void)
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
 ** NAME:           ICN8502_get_point
 **
 ** PARAMETERS:     data: the sapce of data to return
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    The procedure of getting data from ICN8502.
 **
 ****************************************************************************/
#if 1
static SDWORD ICN8502_get_point(Tc_point * data)
{
#if 0
	WORD x, y, xh,xl,yh,yl;
#else
	BYTE dataBuf[5];
#endif
	if (Tc_status==0) return 0;
//		Gpio_IntDisable(gpioIntNum);        //avoid interrupt when getting data
#if 0		
		xh=I2CM_RdReg8Data8(ICN8502_I2C_ADDRESS,0x03);
		xl=I2CM_RdReg8Data8(ICN8502_I2C_ADDRESS,0x04);
		yh=I2CM_RdReg8Data8(ICN8502_I2C_ADDRESS,0x05);
		yl=I2CM_RdReg8Data8(ICN8502_I2C_ADDRESS,0x06);
		x = (xh&0x0F)<<8|xl;
		y = (yh&0x0F)<<8|yl;
		data->x1=x;
		data->y1=y;
		data->reserved=xh&0xf0;
		//mpDebugPrint("x=%d,y=%d,status=%x",x,y,xh&0xf0);
#endif

		if(I2CM_Read_BustMode(ICN8502_I2C_ADDRESS, 0x1003, 1, &dataBuf[0],1) == PASS)
		{
			//mpDebugPrint("dataBuf[0]=%d,",dataBuf[0]);		

               }
		if(I2CM_Read_BustMode(ICN8502_I2C_ADDRESS, 0x1004, 1, &dataBuf[1],1) == PASS)
		{
			//mpDebugPrint("dataBuf[1]=%d,",dataBuf[1]);
			data->x1=((dataBuf[1]&0xff)<<8)|(dataBuf[0]&0xff);
               }
		if(I2CM_Read_BustMode(ICN8502_I2C_ADDRESS, 0x1005, 1, &dataBuf[2],1) == PASS)
		{
			//mpDebugPrint("dataBuf[2]=%d,",dataBuf[2]);		
               }
		if(I2CM_Read_BustMode(ICN8502_I2C_ADDRESS, 0x1006, 1, &dataBuf[3],1) == PASS)
		{
			data->y1=(dataBuf[3]&0xFF)<<8|dataBuf[2];
			//mpDebugPrint("dataBuf[3]=%d,",dataBuf[3]);		
               }
		if(I2CM_Read_BustMode(ICN8502_I2C_ADDRESS, 0x1008, 1, &dataBuf[4],1) == PASS)
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
			
			//mpDebugPrint("reserved=%d,",dataBuf[4]);		
			//mpDebugPrint("x=%d,y=%d,status=%x",data->x1,data->y1,data->reserved);		
               }
	//	Gpio_IntConfig(gpioIntNum, GPIO_ACTIVE_LOW, TC_TRIGGER_MODE);
		//Gpio_IntEnable(gpioIntNum);

		return 0;

}
#else
static SDWORD ICN8502_get_point(Tc_point * data)
{
    BYTE i;
    WORD val[SAMPLE_ARRAY_SIZE];
    WORD x, y, z1, z2,xh,xl,yh,yl,x_ori,y_ori;
    WORD cmd[8],dataBuf[8];
    //mpDebugPrint("SampleBucket.num =%d",SampleBucket.num);
    if ( SampleBucket.num < SAMPLE_ARRAY_SIZE )
    {
        if (!ICN8502_Check_Interrupt())
        {
            Gpio_IntDisable(gpioIntNum);        //avoid interrupt when getting data
            do
            {
                /*I2CM_ReadData_BustMode(ICN8502_I2C_ADDRESS, dataBuf, 8);
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
                xh=I2CM_RdReg8Data8(ICN8502_I2C_ADDRESS,0x03);
		  xl=I2CM_RdReg8Data8(ICN8502_I2C_ADDRESS,0x04);
                yh=I2CM_RdReg8Data8(ICN8502_I2C_ADDRESS,0x05);
		  yl=I2CM_RdReg8Data8(ICN8502_I2C_ADDRESS,0x06);
                x = (xh&0x0F)<<8|xl;
                y = (yh&0x0F)<<8|yl;

	 	  //mpDebugPrint("###xh is %x,xl is %x,  yh is%x,  yl is%x",xh&0x0F,xl,yh&0x0F,yl);
        	  //mpDebugPrint("###x_ori is %d,  y_ori is %d, x=%d,y=%d",x_ori,y_ori,x,y);
		  #else
		  //SW_ICN8502_I2CM_Read_BustMode(ICN8502_I2C_ADDRESS,dataBuf,8);
		  I2CM_Read_BustMode(ICN8502_I2C_ADDRESS, 0x03,0,dataBuf, 4);
                //I2CM_ReadData_BustMode(ICN8502_I2C_ADDRESS, dataBuf, 8);

				
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
            if (ICN8502_Check_Interrupt())
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
        xh=I2CM_RdReg8Data8(ICN8502_I2C_ADDRESS,0x03);
        xl=I2CM_RdReg8Data8(ICN8502_I2C_ADDRESS,0x04);
        yh=I2CM_RdReg8Data8(ICN8502_I2C_ADDRESS,0x05);
        yl=I2CM_RdReg8Data8(ICN8502_I2C_ADDRESS,0x06);
        x = (xh&0x0F)<<8|xl;
        y = (yh&0x0F)<<8|yl;
	#else
	 /*I2CM_ReadData_BustMode(ICN8502_I2C_ADDRESS, &cmd, 8);
        xh=cmd[3];//I2CM_RdReg8Data8(ICN8502_I2C_ADDRESS,0x03);
        xl=cmd[4];//I2CM_RdReg8Data8(ICN8502_I2C_ADDRESS,0x04);
        yh=cmd[5];//I2CM_RdReg8Data8(ICN8502_I2C_ADDRESS,0x05);
        yl=cmd[6];//I2CM_RdReg8Data8(ICN8502_I2C_ADDRESS,0x06);
        x = (xh&0x0F)<<8|xl;
        y = (yh&0x0F)<<8|yl;*/
        I2CM_ReadData_BustMode(ICN8502_I2C_ADDRESS, dataBuf, 8);
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
    
    data->x1 = ICN8502_get_value(val, SampleBucket.num);

    for(i = 0; i < SampleBucket.num; i++)
        val[i] = SampleBucket.SampleAry[i].y;
    data->y1 = ICN8502_get_value(val, SampleBucket.num);
    Gpio_IntEnable(gpioIntNum);

    return 0;
}

#endif

/****************************************************************************
 **
 ** NAME:           ICN8502_change_sensitivity
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
static SDWORD ICN8502_change_sensitivity(DWORD x_sen, DWORD y_sen, DWORD z_sen)
{
    return 0;
}



/****************************************************************************
 **
 ** NAME:           ICN8502_Init
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Init ICN8502. and Setting the mode of interrupt pin.
 **
 ****************************************************************************/
static SDWORD ICN8502_Init(void)
{
    MP_DEBUG("__%s__", __FUNCTION__);

    SDWORD       status;
    DWORD cmd;
    u08IntModeFlag = INT_LOW_LEVEL;
	Gpio_DataSet(TOUCH_RESET_PIN, GPIO_DATA_LOW, 1);
	IODelay(1000);
	Gpio_DataSet(TOUCH_RESET_PIN, GPIO_DATA_HIGH, 1);
    gpioNum = GetGPIOPinIndex(TOUCH_CONTROLLER_INT_PIN);


    //BYTE cmd = ICN8502_I2C_CMD_INIT;
    //I2CM_Write_BustMode(ICN8502_I2C_ADDRESS, &cmd, 1);
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
    stTcDriver->u08Name = "ICN8502 Touch screen device driver";
    stTcDriver->TcInit = ICN8502_Init;
    stTcDriver->TcSleep = ICN8502_Sleep;
    stTcDriver->TcGetData = ICN8502_get_point;
    stTcDriver->TcCheckInterrupt = ICN8502_Check_Interrupt;
    stTcDriver->TcChangeSensitivity = ICN8502_change_sensitivity;


    return 0;
}


#endif //#if (TOUCH_CONTROLLER == TOUCH_PANEL_DRIVER_ICN8502)


