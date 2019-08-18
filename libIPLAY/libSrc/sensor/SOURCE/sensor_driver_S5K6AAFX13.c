/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"



#if ((SENSOR_ENABLE == ENABLE) && defined(SENSOR_TYPE_S5K6AAFX13))

#define I2C_LoopMode   BIT6
#define I2C_NoRstart   BIT5
#define I2C_nonACK     BIT4
#define I2C_16bitmode  BIT3
#define I2C_sus        BIT2
#define I2C_done       BIT1
#define I2C_WaitNext   BIT0


#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))

static void setsensorreg(void)
{
		IPU *ipu = (IPU *) IPU_BASE;
    //I2C_Write_Addr8_Value8(0x43>>1, 0x11, 0x40);//01
    #if 0
		I2C_Write_Addr8_Value8(0x43>>1, 0x11, 0);

    I2C_Write_Addr8_Value8(0x43>>1, 0x3a, 0x04);
    I2C_Write_Addr8_Value8(0x43>>1, 0x12, 0x00);
    I2C_Write_Addr8_Value8(0x43>>1, 0x17, 0x13);
    I2C_Write_Addr8_Value8(0x43>>1, 0x18, 0x01);
    I2C_Write_Addr8_Value8(0x43>>1, 0x32, 0xb6);
    I2C_Write_Addr8_Value8(0x43>>1, 0x19, 0x02);
    I2C_Write_Addr8_Value8(0x43>>1, 0x1a, 0x7a);
    I2C_Write_Addr8_Value8(0x43>>1, 0x03, 0x0a);
    I2C_Write_Addr8_Value8(0x43>>1, 0x0c, 0x00);
    I2C_Write_Addr8_Value8(0x43>>1, 0x3e, 0x00);
    I2C_Write_Addr8_Value8(0x43>>1, 0x70, 0x3a);
    I2C_Write_Addr8_Value8(0x43>>1, 0x71, 0x35);
    I2C_Write_Addr8_Value8(0x43>>1, 0x72, 0x11);
    I2C_Write_Addr8_Value8(0x43>>1, 0x73, 0xf0);
    I2C_Write_Addr8_Value8(0x43>>1, 0xa2, 0x02);

    I2C_Write_Addr8_Value8(0x43>>1, 0x13, 0xe0);
    I2C_Write_Addr8_Value8(0x43>>1, 0x00, 0x00);
    I2C_Write_Addr8_Value8(0x43>>1, 0x10, 0x00);
    I2C_Write_Addr8_Value8(0x43>>1, 0x0d, 0x40);
    I2C_Write_Addr8_Value8(0x43>>1, 0x14, 0x18);    // maximum AGC value: 4x
    I2C_Write_Addr8_Value8(0x43>>1, 0xa5, 0x05);
    I2C_Write_Addr8_Value8(0x43>>1, 0xab, 0x07);

    I2C_Write_Addr8_Value8(0x43>>1, 0x24, 0xa5);
    I2C_Write_Addr8_Value8(0x43>>1, 0x25, 0x33);
    I2C_Write_Addr8_Value8(0x43>>1, 0x26, 0xe3);
    I2C_Write_Addr8_Value8(0x43>>1, 0x9f, 0xba);    // High reference luminance
    I2C_Write_Addr8_Value8(0x43>>1, 0xa0, 0xaa);    // Low reference luminance
    //I2C_Write_Addr8_Value8(0x43>>1, 0x9f, 0xd8);    // High reference luminance
    //I2C_Write_Addr8_Value8(0x43>>1, 0xa0, 0xc8);    // Low reference luminance

    I2C_Write_Addr8_Value8(0x43>>1, 0xa1, 0x03);
    I2C_Write_Addr8_Value8(0x43>>1, 0xa6, 0xd0);
    I2C_Write_Addr8_Value8(0x43>>1, 0xa7, 0xd0);
    I2C_Write_Addr8_Value8(0x43>>1, 0xa8, 0xf0);
    I2C_Write_Addr8_Value8(0x43>>1, 0xa9, 0xb0);
    I2C_Write_Addr8_Value8(0x43>>1, 0xaa, 0x94);
    I2C_Write_Addr8_Value8(0x43>>1, 0x13, 0xe5);

    I2C_Write_Addr8_Value8(0x43>>1, 0x0e, 0x61);
    I2C_Write_Addr8_Value8(0x43>>1, 0x0f, 0x4b);
    I2C_Write_Addr8_Value8(0x43>>1, 0x16, 0x02);
    //I2C_Write_Addr8_Value8(0x43>>1, 0x1e, 0x07);//mirror and flip
    I2C_Write_Addr8_Value8(0x43>>1, 0x1e, 0x07|0x10);

    I2C_Write_Addr8_Value8(0x43>>1, 0x21, 0x02);
    I2C_Write_Addr8_Value8(0x43>>1, 0x22, 0x91);
    I2C_Write_Addr8_Value8(0x43>>1, 0x29, 0x07);
    I2C_Write_Addr8_Value8(0x43>>1, 0x33, 0x0b);
    I2C_Write_Addr8_Value8(0x43>>1, 0x35, 0x0b);
    I2C_Write_Addr8_Value8(0x43>>1, 0x37, 0x1d);
    I2C_Write_Addr8_Value8(0x43>>1, 0x38, 0x71);
    I2C_Write_Addr8_Value8(0x43>>1, 0x39, 0x2a);
    I2C_Write_Addr8_Value8(0x43>>1, 0x3c, 0x78);
    I2C_Write_Addr8_Value8(0x43>>1, 0x4d, 0x40);
    I2C_Write_Addr8_Value8(0x43>>1, 0x4e, 0x20);
    I2C_Write_Addr8_Value8(0x43>>1, 0x69, 0x00);
    I2C_Write_Addr8_Value8(0x43>>1, 0x6b, 0x0a);//4a mike
    I2C_Write_Addr8_Value8(0x43>>1, 0x74, 0x19);
    I2C_Write_Addr8_Value8(0x43>>1, 0x8d, 0x4f);
    I2C_Write_Addr8_Value8(0x43>>1, 0x8e, 0x00);
    I2C_Write_Addr8_Value8(0x43>>1, 0x8f, 0x00);
    I2C_Write_Addr8_Value8(0x43>>1, 0x90, 0x00);
    I2C_Write_Addr8_Value8(0x43>>1, 0x91, 0x00);
    I2C_Write_Addr8_Value8(0x43>>1, 0x96, 0x00);
    I2C_Write_Addr8_Value8(0x43>>1, 0x9a, 0x80);
    I2C_Write_Addr8_Value8(0x43>>1, 0xb0, 0x84);
    I2C_Write_Addr8_Value8(0x43>>1, 0xb1, 0x0c);
    I2C_Write_Addr8_Value8(0x43>>1, 0xb2, 0x0e);
    I2C_Write_Addr8_Value8(0x43>>1, 0xb3, 0x82);
    I2C_Write_Addr8_Value8(0x43>>1, 0xb8, 0x0a);

    I2C_Write_Addr8_Value8(0x43>>1, 0x43, 0x14);
    I2C_Write_Addr8_Value8(0x43>>1, 0x44, 0xf0);
    I2C_Write_Addr8_Value8(0x43>>1, 0x45, 0x34);
    I2C_Write_Addr8_Value8(0x43>>1, 0x46, 0x58);
    I2C_Write_Addr8_Value8(0x43>>1, 0x47, 0x28);
    I2C_Write_Addr8_Value8(0x43>>1, 0x48, 0x3a);
    I2C_Write_Addr8_Value8(0x43>>1, 0x59, 0x88);
    I2C_Write_Addr8_Value8(0x43>>1, 0x5a, 0x88);
    I2C_Write_Addr8_Value8(0x43>>1, 0x5b, 0x44);
    I2C_Write_Addr8_Value8(0x43>>1, 0x5c, 0x67);
    I2C_Write_Addr8_Value8(0x43>>1, 0x5d, 0x49);
    I2C_Write_Addr8_Value8(0x43>>1, 0x5e, 0x0e);
    I2C_Write_Addr8_Value8(0x43>>1, 0x6c, 0x0a);
    I2C_Write_Addr8_Value8(0x43>>1, 0x6d, 0x55);
    I2C_Write_Addr8_Value8(0x43>>1, 0x6e, 0x11);
    I2C_Write_Addr8_Value8(0x43>>1, 0x6f, 0x9f);    // 0x9e for advance AWB
    I2C_Write_Addr8_Value8(0x43>>1, 0x6a, 0x40);
    I2C_Write_Addr8_Value8(0x43>>1, 0x01, 0x40);
    I2C_Write_Addr8_Value8(0x43>>1, 0x02, 0x40);
    I2C_Write_Addr8_Value8(0x43>>1, 0x13, 0xe7);

 	 /* Saturation level +1 */
   	I2C_Write_Addr8_Value8(0x43>>1, 0x4f, 0x99);
   	I2C_Write_Addr8_Value8(0x43>>1, 0x50, 0x99);
   	I2C_Write_Addr8_Value8(0x43>>1, 0x51, 0x00);
   	I2C_Write_Addr8_Value8(0x43>>1, 0x52, 0x28);
   	I2C_Write_Addr8_Value8(0x43>>1, 0x53, 0x70);
   	I2C_Write_Addr8_Value8(0x43>>1, 0x54, 0x99);
   	I2C_Write_Addr8_Value8(0x43>>1, 0x58, 0x9e);

	   //Sharpness
    //I2C_Write_Addr8_Value8(0x43>>1, 0x41, 0x08);
    //I2C_Write_Addr8_Value8(0x43>>1, 0x3f, 0x00);
    I2C_Write_Addr8_Value8(0x43>>1, 0x41, 0x38);
    I2C_Write_Addr8_Value8(0x43>>1, 0x3f, 0x04);

    I2C_Write_Addr8_Value8(0x43>>1, 0x75, 0x03);//02
    I2C_Write_Addr8_Value8(0x43>>1, 0x76, 0xe1);
    I2C_Write_Addr8_Value8(0x43>>1, 0x4c, 0x00);
    I2C_Write_Addr8_Value8(0x43>>1, 0x77, 0x03);//03
    I2C_Write_Addr8_Value8(0x43>>1, 0x3d, 0xc2);
    I2C_Write_Addr8_Value8(0x43>>1, 0x4b, 0x09);
    I2C_Write_Addr8_Value8(0x43>>1, 0xc9, 0x20);
    I2C_Write_Addr8_Value8(0x43>>1, 0x41, 0x38);
    I2C_Write_Addr8_Value8(0x43>>1, 0x56, 0x40);

    I2C_Write_Addr8_Value8(0x43>>1, 0x34, 0x11);
    I2C_Write_Addr8_Value8(0x43>>1, 0x3b, 0x12);    // Enable 50/60 auto detection
    I2C_Write_Addr8_Value8(0x43>>1, 0xa4, 0x88);
    I2C_Write_Addr8_Value8(0x43>>1, 0x96, 0x00);
    I2C_Write_Addr8_Value8(0x43>>1, 0x97, 0x30);
    I2C_Write_Addr8_Value8(0x43>>1, 0x98, 0x20);
    I2C_Write_Addr8_Value8(0x43>>1, 0x99, 0x30);
    I2C_Write_Addr8_Value8(0x43>>1, 0x9a, 0x84);
    I2C_Write_Addr8_Value8(0x43>>1, 0x9b, 0x29);
    I2C_Write_Addr8_Value8(0x43>>1, 0x9c, 0x03);
    I2C_Write_Addr8_Value8(0x43>>1, 0x92, 0x66);//mike
    I2C_Write_Addr8_Value8(0x43>>1, 0x9d, 0x4c);//99
    I2C_Write_Addr8_Value8(0x43>>1, 0x9e, 0x3f);//7f
    I2C_Write_Addr8_Value8(0x43>>1, 0x78, 0x04);

    I2C_Write_Addr8_Value8(0x43>>1, 0x79, 0x01);
    I2C_Write_Addr8_Value8(0x43>>1, 0xc8, 0xf0);
    I2C_Write_Addr8_Value8(0x43>>1, 0x79, 0x0f);
    I2C_Write_Addr8_Value8(0x43>>1, 0xc8, 0x00);
    I2C_Write_Addr8_Value8(0x43>>1, 0x79, 0x10);
    I2C_Write_Addr8_Value8(0x43>>1, 0xc8, 0x7e);
    I2C_Write_Addr8_Value8(0x43>>1, 0x79, 0x0b);
    I2C_Write_Addr8_Value8(0x43>>1, 0xc8, 0x01);
    I2C_Write_Addr8_Value8(0x43>>1, 0x79, 0x0c);
    I2C_Write_Addr8_Value8(0x43>>1, 0xc8, 0x0f);
    I2C_Write_Addr8_Value8(0x43>>1, 0x79, 0x0d);
    I2C_Write_Addr8_Value8(0x43>>1, 0xc8, 0x20);
    I2C_Write_Addr8_Value8(0x43>>1, 0x79, 0x09);
    I2C_Write_Addr8_Value8(0x43>>1, 0xc8, 0x80);
    I2C_Write_Addr8_Value8(0x43>>1, 0x79, 0x02);
    I2C_Write_Addr8_Value8(0x43>>1, 0xc8, 0xc0);
    I2C_Write_Addr8_Value8(0x43>>1, 0x79, 0x03);
    I2C_Write_Addr8_Value8(0x43>>1, 0xc8, 0x40);
    I2C_Write_Addr8_Value8(0x43>>1, 0x79, 0x05);
    I2C_Write_Addr8_Value8(0x43>>1, 0xc8, 0x30);
    I2C_Write_Addr8_Value8(0x43>>1, 0x79, 0x26);

    // Lens correction
    I2C_Write_Addr8_Value8(0x43>>1, 0x64, 0x05);
    I2C_Write_Addr8_Value8(0x43>>1, 0x65, 0x00);
    I2C_Write_Addr8_Value8(0x43>>1, 0x94, 0x05);
    I2C_Write_Addr8_Value8(0x43>>1, 0x95, 0x07);
    I2C_Write_Addr8_Value8(0x43>>1, 0x66, 0x05);
    // Dummy 4x
    I2C_Write_Addr8_Value8(0x43>>1, 0xa4, 0x89);
    I2C_Write_Addr8_Value8(0x43>>1, 0xa1, 0x03);
   	//offset
   	I2C_Write_Addr8_Value8(0x43>>1, 0xb1, 0x0c);// ;[2] enble
   	I2C_Write_Addr8_Value8(0x43>>1, 0xb3, 0x7c);// ;lower limit +0x80
   	I2C_Write_Addr8_Value8(0x43>>1, 0xb5, 0x04);// ;Stable Range
   	I2C_Write_Addr8_Value8(0x43>>1, 0xbe, 0x48);// ;Blue channel
   	I2C_Write_Addr8_Value8(0x43>>1, 0xbf, 0x48);// ;Red channel
   	I2C_Write_Addr8_Value8(0x43>>1, 0xc0, 0x4a);// ;Gb channel
   	I2C_Write_Addr8_Value8(0x43>>1, 0xc1, 0x4a);// ;Gr channel

   	//set sensor mode

		I2C_Write_Addr8_Value8(0x43>>1, 0x45, 0x34);
		I2C_Write_Addr8_Value8(0x43>>1, 0x46, 0x58);
		I2C_Write_Addr8_Value8(0x43>>1, 0x47, 0x28);
		I2C_Write_Addr8_Value8(0x43>>1, 0x48, 0x3a);
		I2C_Write_Addr8_Value8(0x43>>1, 0x5a, 0x88);
		I2C_Write_Addr8_Value8(0x43>>1, 0x5b, 0x44);
		I2C_Write_Addr8_Value8(0x43>>1, 0x5c, 0x67);
		I2C_Write_Addr8_Value8(0x43>>1, 0x5d, 0x49);
		I2C_Write_Addr8_Value8(0x43>>1, 0x3b, 0x12);


   	I2C_Write_Addr8_Value8(0x43>>1, 0x2d, 0x00);
   	I2C_Write_Addr8_Value8(0x43>>1, 0x2e, 0x00);
#else

		TimerDelay(100);	// Wait100mSec
		TimerDelay(10);	// Wait10mSec
		// Start user init script

		// End user init script

		///////////////////////////////////////////
		//clk Settings

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01B8,	0x5DC0);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01BA,	0x0000);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01C6,	0x0001);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01CC,	0x1B58);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01CE,	0x36B0);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01D0,	0x36B0);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01E0,	0x0001);


		TimerDelay(100);

		///////////////////////////////////////////
		//PREVIEW CONFIGURATION 0 (SXGA, YUV, 24fps)

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0242	,0x0500);	//0320 					//1280
		mpDebugPrint("--------------------------");
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0244	,0x02D0);	//0258					//720
		mpDebugPrint("--------------------------");
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0246	,0x0005);	//YUV
		mpDebugPrint("--------------------------");
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x024E	,0x0000);
		mpDebugPrint("--------------------------");
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0248	,0x36B0);
		mpDebugPrint("--------------------------");
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x024A	,0x36B0);
		mpDebugPrint("--------------------------");
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x024C	,0x0042);
		mpDebugPrint("--------------------------");
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0252	,0x0002);
		mpDebugPrint("--------------------------");
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0250	,0x0002);
		mpDebugPrint("--------------------------");
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0254	,0x029A);
		mpDebugPrint("--------------------------");
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0256	,0x0000);



		////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////
		//PREVIEW
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x021C	,0x0000);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0220	,0x0001);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01F8	,0x0001);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x021E	,0x0001);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01F0	,0x0001);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01F2	,0x0001);

		////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////
		// change InPut

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x020A	,0x0500);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x020C	,0x02D0);

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01FA	,0x0500);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01FC	,0x02D0);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01FE	,0x0000);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0200	,0x0098);

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x021A	,0x0001);

		TimerDelay(1000);

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0254	,0x01A0);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x021E	,0x0001);

#endif
}


static void sensorSET(void)//mclk=24 max frame rate=15
{


		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0400,  0x007F);	//Auto 7Fh, manual:5Fh
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x03DC,  0x0000);	// 0-disable 1-50 2-60
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x03DE,  0x0000);
		///////////////////////////////////////////
		//clk Settings
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01B8,  0x5DC0);

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01BA,  0x0000);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01C6,  0x0002);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01CC,  0x1964);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01CE,  0x1964);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01D0,  0x1964);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01D2,  0x1770);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01D4,  0x2EE0);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01D6,  0x2EE0);

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01E0,  0x0001);

		IODelay(100);

		//PREVIEW CONFIGURATION 1 (SXGA, YUV, 15fps)
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0268,  0x0500);	// 0320 					//1280
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x026A,  0x0400);	//0258					//1024
		//I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x026A,  720);

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x026C,  0x0005);	//YUV
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0274,  0x0001);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x026E,  0x2EE0);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0270,  0x2EE0);

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0272,  0x0042);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0278,  0x0002);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0276,  0x0000);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x027A,  0x0535);	//029a
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x027C,  0x029A);	//0535

		///////////////////////////////////////////

		////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////
		//PREVIEW
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x021C,  0x0001);	// 0: fixed sxga 1: dynamic sxga 2: fixed vga 3: dynamic vga

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0220,  0x0001);

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01F8,  0x0001);

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x021E,  0x0001);

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01F0,  0x0001);

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01F2,  0x0001);

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01E0,  0x0001);

}

static void sensorSET720p(void)//mclk=24 max frame rate=24
{
		//clk Settings
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01B8,  0x5DC0);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01BA,  0x0000);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01C6,  0x0001);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01CC,  0x1B58);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01CE,  0x36B0);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01D0,  0x36B0);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01E0,  0x0001);

		//p100
		//IODelay(100);
		SensorWait(100);
		///////////////////////////////////////////
		//PREVIEW CONFIGURATION 0 (SXGA, YUV, 24fps)

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0242,  0x0500);	//0320 					//1280
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0244,  0x02D0);	//0258					//720
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0246,  0x0005);	//YUV
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x024E,  0x0000);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0248,  0x36B0);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x024A,  0x36B0);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x024C,  0x0042);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0252,  0x0002);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0250,  0x0002);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0254,  0x029A);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0256,  0x0000);
		////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////
		//PREVIEW
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x021C,  0x0000);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0220,  0x0001);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01F8,  0x0001);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x021E,  0x0001);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01F0,  0x0001);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01F2,  0x0001);

		////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////
		// change InPut

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x020A,  0x0500);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x020C,  0x02D0);

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01FA,  0x0500);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01FC,  0x02D0);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01FE,  0x0000);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0200,  0x0098);

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x021A,  0x0001);

		//p1000
		//IODelay(1000);
		SensorWait(1000);

		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0254,  0x01A0);
		I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x021E,  0x0001);

}

void Drive_Sensor_S5K6AAFX13(void)
{
	//I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0xfcfc,  0x7000);

	// Analog Settings
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0xfcfc,  0x7000);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x1102,  0x0000);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x1108,  0x0090);

	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0xfcfc,  0xd000);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0xF40C,  0x0060);

	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0xfcfc,  0x7000);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x11B6,  0x0020);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x11B8,  0x0010);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x11BA,  0x0008);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x11BC,  0x0004);

	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x119C,  0x0040);

	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x07B6,  0x0000);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x07B8,  0x0002);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x07BA,  0x0003);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x07BC,  0x0006);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x07BE,  0x000C);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x07C0,  0x0018);


	//p10	// Wait10mSec
	//IODelay(10);
	//IODelay(10);
	SensorWait(10);

	//sensorSET();
	sensorSET720p();

	return;
///////////////////////////////////////////
//clk Settings
#if 0  //input mclk=24 vsync<10
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01B8,  0x5DC0);//input mclk 24mhz
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01BA,  0x0000);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01C6,  0x0002);

	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01CC,  0x05DC);

	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01CE,  0x05DC);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01D0,  0x05DC);


	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01D2,  0x05DC);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01D4,  0x05DC);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01D6,  0x05DC);

	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01E0,  0x0001);
#endif

#if 0 //input mclk=24 vsync=10
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01B8,  0x4E20);//input mclk 20mhz
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01BA,  0x0000);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01C6,  0x0002);

	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01CC,  0x4E20>>2);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01CE,  0x4E20>>1);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01D0,  0x4E20>>1);


	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01D2,  0x05DC);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01D4,  0x05DC);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01D6,  0x05DC);

	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01E0,  0x0001);
#endif

	//p100
	IODelay(100);
	IODelay(100);
	///////////////////////////////////////////
	//PREVIEW CONFIGURATION 0 (1280x800, Raw10)
	               /* */
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0242,  0x0500);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0244,  0x0320);
	//I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0246,  0x0007);	//bayer
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0246,  0x0005);	//YUV


	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x024E,  0x0000);//use index 0 slock

	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0248,  0x1770);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x024A,  0x0000);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x024C,  0x0042);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0252,  0x0002);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0250,  0x0002);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0254,  0x029a);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0256,  0x0000);

	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////
	//PREVIEW
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x021C,  0x0000);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0220,  0x0001);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01F8,  0x0001);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x021E,  0x0001);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01F0,  0x0001);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01F2,  0x0001);
	return;
	//p100
	IODelay(100);
	IODelay(100);
	// change InPut

	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x020A,  0x0500);//preview window width
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x020C,  0x0400);//preview window height

	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01FA,  0x0500);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01FC,  0x0400);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x01FE,  0x0000);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0200,  0x0000);

	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x021A,  0x0001);

	//p1000
	IODelay(2000);


	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0400,  0x0080);	// Disable AE, AWB, AFC, AFIT

	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x03DC,  0x0000);	// Disable flicker quantum

	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x03DE,  0x0001);



	// Disable blocks for power consumption
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0ABC,  0xFFFF);	//afit_pConstBaseVals[15] // Disparity, Despeckle, Denoise, Demosaic, Sharpening, Noise Mixer
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0ABe,  0xFFFF);	//afit_pConstBaseVals[16]
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0Ac0,  0xFFFF);	//afit_pConstBaseVals[17]

	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0xfcfc,  0xd000);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x4500,  0x0001);	// PreGamma
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x4700,  0x0001);	// PostGamma
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x6200,  0x0001);	// PreLum
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x6100,  0x0001);	// Pre_x3
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x6500,  0x0001);	// PostGamma8
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x6600,  0x0001);	// CCM
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x6412,  0x0001);	// Demosaic - bypass via fit
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x4C00,  0x0001);	// Denoise - bypass via fit
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x6900,  0x0001);	// Dsmix - bypass via fit
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x4B00,  0x0001);	// Despeckle - bypass via fit
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x4900,  0x0001);	// Disparity - bypass via fit
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x6800,  0x0001);	// RGBGamma
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x4100,  0x0001);	// GRAS
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x6700,  0x0001);	// Uvdenoise - bypass via fit
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x6B30,  0x0001);	// RGB2YUV
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x4400,  0x0001);	// byr_wb
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x7200,  0x0001);	// RGBCrop
	//6A00 0001 // Noise Mixer - bypass via fit


	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0xfcfc,  0x7000);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x0400,  0x0080);	//Manual AE

	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x03C6,  0x0CE4);	//shutter()
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x03CA,  0x0001);
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x03CC,  0x0100);	//Analog gain (0400:X1, 0800:X2)
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0x03CE,  0x0001);

}
#endif

#endif



