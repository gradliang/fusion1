

typedef struct{
    volatile U32 b_code1;
    volatile U32 b_code2;
} TOOL;

typedef struct{
unsigned char src_start_bit;
unsigned char src_end_bit;
unsigned short reg_addr;
unsigned char start_bit;
unsigned char end_bit;
} BITS_ASSIGN;

#ifdef SENSOR_TYPE_MAGIC_SENSOR
    extern BYTE *start_addr_PC_tool;
#endif

  IPU *pc_tool_ipu;
  TOOL *tool;
  U32 tmp, tmp1, tmp2;

BITS_ASSIGN aec_expo_regs[4], aec_gain_regs[4], aec_adj_regs[4], aec_vts_regs[4];
unsigned char cnt_aec_expo_regs, cnt_aec_gain_regs, cnt_aec_adj_regs, cnt_aec_vts_regs;

U32 device_addr;

  typedef struct{
      unsigned short addr;
      unsigned short value;
      BYTE delay;
  } SENSOR_REGS;

	
	/*------ Define bits type ------*/
	#define   I2C_TYPE_BIT8    0
	#define   I2C_TYPE_BIT16   1




	/*I2C write function*/
	SDWORD (*Sensor_I2C_WriteFunc) (BYTE DevID, WORD wReg, WORD bData);	  

	extern SDWORD I2CM_WtReg16Data8(BYTE DevID, WORD wReg, BYTE bData);
	extern SDWORD I2CM_WtReg8Data8(BYTE DevID, BYTE bReg, BYTE bData);


	/*I2C read function*/
	SDWORD (*Sensor_I2C_ReadFunc) (BYTE DevID_7Bit, WORD RegAddr);

	extern SDWORD I2C_Read_Addr16_Value8_LoopMode(BYTE DevID_7Bit, WORD RegAddr);

	#define MAGIC_SENSOR_DEBUG 0
	//#define MAGIC_SENSOR_DEBUG 1

	#define MAGIC_SHOP_DEBUG 0
	//#define MAGIC_SHOP_DEBUG 1
	
	/*------ Data format address ------*/

#if (MAGIC_SENSOR_DEBUG)
	/*Only debug */
	/*debug using "start_addr_PC_tool_mem"*/
	#define I2C_TYPE						 0x0
	#define SENSOR_SLAVE_DEVICE_WRITE_ADDR	 0x8
	#define PREVIEW_RESOLUTION               0X4
	#define RECORD_SETTING_BASE 			 0xC

	/*Debug using "start_addr_PC_tool_mem_capture_prev"*/
	#define CAPTURE_PREVIEW_SETTING_BASE	 0X0 /*525 capture prev debug mem*/

	/*debug using "start_addr_PC_tool_mem_capture_flow"*/
	#define MAPPING_OF_GAIN               0X0
	#define MAPPING_OF_EXPOSURE           0XC
	#define MAPPING_OF_ADJ_MAXLINE        0X28
	#define MAPPING_OF_VTS                0x3C         
	#define MAPPING_OF_AGC                0x48
	#define MAPPING_OF_AEC                0x54
	#define CAPTURE_SETTING_BASE          0x60 
#else
	/*Sensor tool : Field Definitions of Scratch Memeory in iPlay*/
	#define I2C_TYPE						 0x00000004
	#define SENSOR_SLAVE_DEVICE_WRITE_ADDR	 0x00000008
	#define PREVIEW_RESOLUTION               0X00000010
	#define MAPPING_OF_AEC                   0x00000030
	#define MAPPING_OF_AGC                   0x0000003C
	#define MAPPING_OF_VTS                   0x00000048
	#define MAPPING_OF_EXPOSURE              0X00000054	
	#define MAPPING_OF_GAIN                  0X00000078
	#define MAPPING_OF_ADJ_MAXLINE           0X0000009C
	#define CAPTURE_PREVIEW_SETTING_BASE	 0X00000100
	#define CAPTURE_SETTING_BASE             0x00001100  
	#define RECORD_SETTING_BASE 			 0x00002100		
#endif



#if (MAGIC_SHOP_DEBUG) //for Magic Shop debug only
	/*Debug using 'start_addr_PC_tool_mem_ColorIPU"*/
	#define MAGIC_SHOP_COLOR_MANAGER      0x00
#else
	#define MAGIC_SHOP_COLOR_MANAGER      0x00003100	
#endif










