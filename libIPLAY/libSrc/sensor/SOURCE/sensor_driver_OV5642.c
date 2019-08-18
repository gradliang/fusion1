/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"
#include "sensor.h"


#if ((SENSOR_ENABLE == ENABLE) && defined(SENSOR_TYPE_OV5642))

extern BYTE sensor_mode;
extern BYTE sensor_IMGSIZE_ID;
extern BYTE sensor_FrameRate;

extern BYTE *g_bImageData_Buf;

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
static void ov_set_176x144(void)
{

I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3103,  0x93);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3008,  0x82);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3017,  0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3018,  0xfc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3615,  0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3000,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3001,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3002,  0x5c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3003,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3004,  0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3005,  0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3006,  0x43);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3007,  0x37);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3011,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3012,  0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3010,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x460c,  0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3815,  0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x370c,  0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3602,  0xfc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3612,  0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3634,  0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3613,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3605,  0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3621,  0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3622,  0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3604,  0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3603,  0xa7);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3603,  0x27);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x4000,  0x21);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x401d,  0x22);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3600,  0x54);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3605,  0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3606,  0x3f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3c01,  0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5000,  0x4f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5020,  0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5181,  0x79);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5182,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5185,  0x22);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5197,  0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5001,  0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5500,  0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5504,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5505,  0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5080,  0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x300e,  0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x4610,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x471d,  0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x4708,  0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3808,  0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3809,  0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380a,  0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380b,  0xe0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380e,  0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380f,  0xd0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x501f,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5000,  0x4f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x4300,  0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3503,  0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3501,  0x73);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3502,  0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x350b,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3503,  0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3824,  0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3825,  0xb0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3501,  0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3502,  0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x350b,  0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380c,  0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380d,  0x2a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380e,  0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380f,  0xe8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a0d,  0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a0e,  0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3818,  0xc1);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3705,  0xdb);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x370a,  0x81);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3801,  0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3621,  0xc7);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3801,  0x50);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3803,  0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3827,  0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3810,  0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3804,  0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3805,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5682,  0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5683,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3806,  0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3807,  0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5686,  0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5687,  0xbc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a00,  0x78);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a1a,  0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a13,  0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a18,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a19,  0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a08,  0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a09,  0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a0a,  0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a0b,  0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x350c,  0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x350d,  0xd0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3500,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3501,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3502,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x350a,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x350b,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3503,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x528a,  0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x528b,  0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x528c,  0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x528d,  0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x528e,  0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x528f,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5290,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5292,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5293,  0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5294,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5295,  0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5296,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5297,  0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5298,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5299,  0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x529a,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x529b,  0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x529c,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x529d,  0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x529e,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x529f,  0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a0f,  0x3c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a10,  0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a1b,  0x3c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a1e,  0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a11,  0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a1f,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3030,  0x2b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a02,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a03,  0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a04,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a14,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a15,  0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a16,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a00,  0x78);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a08,  0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a09,  0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a0a,  0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a0b,  0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a0d,  0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a0e,  0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5193,  0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x589b,  0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x589a,  0xc5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x4001,  0x42);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x401c,  0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x528a,  0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x528b,  0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x528c,  0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x528d,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x528e,  0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x528f,  0x28);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5290,  0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5292,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5293,  0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5294,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5295,  0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5296,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5297,  0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5298,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5299,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x529a,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x529b,  0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x529c,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x529d,  0x28);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x529e,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x529f,  0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5282,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5300,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5301,  0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5302,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5303,  0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x530c,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x530d,  0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x530e,  0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x530f,  0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5310,  0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5311,  0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5308,  0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5309,  0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5304,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5305,  0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5306,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5307,  0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5314,  0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5315,  0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5319,  0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5316,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5317,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5318,  0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5380,  0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5381,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5382,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5383,  0x4e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5384,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5385,  0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5386,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5387,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5388,  0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5389,  0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x538a,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x538b,  0x31);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x538c,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x538d,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x538e,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x538f,  0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5390,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5391,  0xab);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5392,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5393,  0xa2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5394,  0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5480,  0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5481,  0x21);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5482,  0x36);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5483,  0x57);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5484,  0x65);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5485,  0x71);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5486,  0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5487,  0x87);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5488,  0x91);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5489,  0x9a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x548a,  0xaa);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x548b,  0xb8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x548c,  0xcd);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x548d,  0xdd);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x548e,  0xea);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x548f,  0x1d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5490,  0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5491,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5492,  0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5493,  0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5494,  0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5495,  0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5496,  0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5497,  0xb8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5498,  0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5499,  0x86);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x549a,  0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x549b,  0x5b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x549c,  0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x549d,  0x3b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x549e,  0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x549f,  0x1c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54a0,  0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54a1,  0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54a2,  0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54a3,  0xed);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54a4,  0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54a5,  0xc5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54a6,  0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54a7,  0xa5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54a8,  0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54a9,  0x6c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54aa,  0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54ab,  0x41);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54ac,  0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54ad,  0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54ae,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54af,  0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54b0,  0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54b1,  0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54b2,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54b3,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54b4,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54b5,  0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54b6,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x54b7,  0xdf);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5402,  0x3f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5403,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3406,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5180,  0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5181,  0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5182,  0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5183,  0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5184,  0x25);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5185,  0x24);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5186,  0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5187,  0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5188,  0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5189,  0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x518a,  0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x518b,  0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x518c,  0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x518d,  0x44);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x518e,  0x3d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x518f,  0x58);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5190,  0x46);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5191,  0xf8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5192,  0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5193,  0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5194,  0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5195,  0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5196,  0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5197,  0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5198,  0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5199,  0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x519a,  0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x519b,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x519c,  0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x519d,  0x82);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x519e,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5025,  0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a0f,  0x38);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a10,  0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a1b,  0x3a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a1e,  0x2e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a11,  0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a1f,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5688,  0xa6);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5689,  0x6a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x568a,  0xea);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x568b,  0xae);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x568c,  0xa6);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x568d,  0x6a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x568e,  0x62);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x568f,  0x26);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5583,  0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5584,  0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5580,  0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5000,  0xcf);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5800,  0x27);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5801,  0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5802,  0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5803,  0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5804,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5805,  0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5806,  0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5807,  0x2f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5808,  0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5809,  0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x580a,  0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x580b,  0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x580c,  0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x580d,  0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x580e,  0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x580f,  0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5810,  0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5811,  0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5812,  0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5813,  0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5814,  0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5815,  0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5816,  0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5817,  0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5818,  0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5819,  0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x581a,  0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x581b,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x581c,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x581d,  0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x581e,  0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x581f,  0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5820,  0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5821,  0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5822,  0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5823,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5824,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5825,  0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5826,  0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5827,  0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5828,  0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5829,  0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x582a,  0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x582b,  0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x582c,  0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x582d,  0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x582e,  0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x582f,  0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5830,  0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5831,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5832,  0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5833,  0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5834,  0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5835,  0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5836,  0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5837,  0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5838,  0x32);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5839,  0x1f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x583a,  0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x583b,  0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x583c,  0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x583d,  0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x583e,  0x26);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x583f,  0x53);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5840,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5841,  0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5842,  0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5843,  0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5844,  0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5845,  0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5846,  0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5847,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5848,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5849,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x584a,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x584b,  0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x584c,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x584d,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x584e,  0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x584f,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5850,  0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5851,  0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5852,  0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5853,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5854,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5855,  0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5856,  0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5857,  0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5858,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5859,  0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x585a,  0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x585b,  0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x585c,  0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x585d,  0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x585e,  0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x585f,  0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5860,  0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5861,  0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5862,  0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5863,  0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5864,  0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5865,  0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5866,  0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5867,  0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5868,  0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5869,  0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x586a,  0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x586b,  0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x586c,  0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x586d,  0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x586e,  0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x586f,  0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5870,  0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5871,  0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5872,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5873,  0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5874,  0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5875,  0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5876,  0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5877,  0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5878,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5879,  0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x587a,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x587b,  0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x587c,  0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x587d,  0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x587e,  0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x587f,  0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5880,  0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5881,  0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5882,  0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5883,  0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5884,  0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5885,  0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5886,  0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5887,  0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3710,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3632,  0x51);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3702,  0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3703,  0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3704,  0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x370b,  0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x370d,  0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3631,  0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3632,  0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3606,  0x24);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3620,  0x96);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5785,  0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3a13,  0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3600,  0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3604,  0x48);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3606,  0x1b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x370d,  0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x370f,  0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3709,  0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3823,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5007,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5009,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5011,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5013,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x519e,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5086,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5087,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5088,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5089,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x302b,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3621,  0x87);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3808,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3809,  0xb0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380a,  0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380b,  0x90);//

}
static void ov_setQVGA_320x240_30FPS(void)
{
//@@ QVGA(YUV) 30fps
//100 99 320 240
//100 98 0 0
//100 97 320 240
//;
//;OV5642 setting Version History
//;
//;date 07/11/2009
//;--2nth release of OV5642 Rev1D(BL) setting.

I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3103,0x93);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3008,0x82);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3017,0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3018,0xfc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3615,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3000,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3001,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3002,0x5c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3003,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3004,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3005,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3006,0x43);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3007,0x37);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3011,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3012,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3010,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460c,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3815,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370c,0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3602,0xfc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3612,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3634,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3613,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3605,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3622,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3603,0xa7);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3603,0x27);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4000,0x21);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x401d,0x22);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3600,0x54);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3605,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x3f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3c01,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0x4f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5020,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5181,0x79);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5182,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5185,0x22);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5197,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5001,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5500,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5504,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5505,0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5080,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x300e,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4610,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471d,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4708,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3808,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3809,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380a,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380b,0xe0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,0xd0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x501f,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0x4f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4300,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x73);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3824,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3825,0xb0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380c,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380d,0x2a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,0xe8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0d,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0e,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3818,0xc1);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3705,0xdb);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370a,0x81);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3801,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0xc7);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3801,0x50);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3803,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3827,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3810,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3804,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3805,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5682,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5683,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3806,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3807,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5686,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5687,0xbc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a00,0x78);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1a,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a13,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a18,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a19,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a08,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a09,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0a,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0b,0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350c,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350d,0xd0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3500,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528a,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528c,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528d,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528e,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5290,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5292,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5293,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5294,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5295,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5296,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5297,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5298,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5299,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529b,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529d,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529f,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0f,0x3c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a10,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1b,0x3c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1e,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a11,0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3030,0x2b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a02,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a03,0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a04,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a14,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a15,0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a16,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a00,0x78);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a08,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a09,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0a,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0b,0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0d,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0e,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5193,0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589a,0xc5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4001,0x42);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x401c,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528a,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528c,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528d,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528e,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528f,0x28);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5290,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5292,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5293,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5294,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5295,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5296,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5297,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5298,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5299,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529b,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529d,0x28);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529f,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5282,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5300,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5301,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5302,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5303,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530d,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530e,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530f,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5310,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5311,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5308,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5309,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5304,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5305,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5306,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5307,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5314,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5315,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5319,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5316,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5317,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5318,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5380,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5381,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5382,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5383,0x4e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5384,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5385,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5386,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5387,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5388,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5389,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538b,0x31);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538d,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538f,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5390,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5391,0xab);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5392,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5393,0xa2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5394,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5480,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5481,0x21);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5482,0x36);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5483,0x57);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5484,0x65);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5485,0x71);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5486,0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5487,0x87);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5488,0x91);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5489,0x9a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548a,0xaa);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548b,0xb8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548c,0xcd);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548d,0xdd);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548e,0xea);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548f,0x1d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5490,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5491,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5492,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5493,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5494,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5495,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5496,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5497,0xb8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5498,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5499,0x86);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549a,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549b,0x5b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549c,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549d,0x3b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549e,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549f,0x1c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a0,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a1,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a2,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a3,0xed);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a4,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a5,0xc5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a6,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a7,0xa5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a8,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a9,0x6c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54aa,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ab,0x41);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ac,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ad,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ae,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54af,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b0,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b1,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b2,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b3,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b4,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b5,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b6,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b7,0xdf);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5402,0x3f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5403,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3406,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5180,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5181,0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5182,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5183,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5184,0x25);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5185,0x24);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5186,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5187,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5188,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5189,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518a,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518b,0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518c,0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518d,0x44);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518e,0x3d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518f,0x58);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5190,0x46);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5191,0xf8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5192,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5193,0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5194,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5195,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5196,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5197,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5198,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5199,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519a,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519c,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519d,0x82);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5025,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0f,0x38);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a10,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1b,0x3a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1e,0x2e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a11,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5688,0xa6);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5689,0x6a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568a,0xea);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568b,0xae);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568c,0xa6);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568d,0x6a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568e,0x62);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568f,0x26);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5583,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5584,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5580,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0xcf);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5800,0x27);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5801,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5802,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5803,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5804,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5805,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5806,0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5807,0x2f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5808,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5809,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580a,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580b,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580c,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580d,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580e,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580f,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5810,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5811,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5812,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5813,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5814,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5815,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5816,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5817,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5818,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5819,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581a,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581d,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581e,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581f,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5820,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5821,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5822,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5823,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5824,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5825,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5826,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5827,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5828,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5829,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582a,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582c,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582d,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582e,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582f,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5830,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5831,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5832,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5833,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5834,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5835,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5836,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5837,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5838,0x32);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5839,0x1f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583a,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583b,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583c,0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583d,0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583e,0x26);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583f,0x53);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5840,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5841,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5842,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5843,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5844,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5845,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5846,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5847,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5848,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5849,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584a,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584b,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584c,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584d,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584e,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5850,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5851,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5852,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5853,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5854,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5855,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5856,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5857,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5858,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5859,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585a,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585b,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585c,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585d,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585e,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585f,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5860,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5861,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5862,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5863,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5864,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5865,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5866,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5867,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5868,0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5869,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586a,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586b,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586c,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586d,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586e,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586f,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5870,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5871,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5872,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5873,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5874,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5875,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5876,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5877,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5878,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5879,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587a,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587b,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587c,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587d,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587e,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587f,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5880,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5881,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5882,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5883,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5884,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5885,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5886,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5887,0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3710,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3632,0x51);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3702,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3703,0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3704,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370b,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370d,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3631,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3632,0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x24);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3620,0x96);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5785,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a13,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3600,0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x48);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x1b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370d,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370f,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3709,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3823,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5007,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5009,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5011,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5013,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5086,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5087,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5088,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5089,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x302b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0x87);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3808,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3809,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380b,0xf0);//

}
static void ov_setVGA_640x480_30FPS(void)
{
//@@ VGA(YUV) 30fps


I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3103,0x93);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3008,0x82);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3017,0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3018,0xfc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3615,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3000,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3001,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3002,0x5c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3003,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3004,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3005,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3006,0x43);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3007,0x37);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3011,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3012,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3010,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460c,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3815,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370c,0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3602,0xfc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3612,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3634,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3613,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3605,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3622,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3603,0xa7);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3603,0x27);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4000,0x21);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x401d,0x22);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3600,0x54);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3605,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x3f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3c01,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0x4f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5020,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5181,0x79);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5182,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5185,0x22);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5197,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5001,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5500,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5504,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5505,0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5080,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x300e,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4610,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471d,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4708,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3808,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3809,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380a,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380b,0xe0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,0xd0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x501f,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0x4f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4300,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x73);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3824,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3825,0xb0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380c,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380d,0x2a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,0xe8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0d,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0e,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3818,0xc1);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3705,0xdb);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370a,0x81);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3801,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0xc7);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3801,0x50);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3803,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3827,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3810,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3804,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3805,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5682,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5683,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3806,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3807,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5686,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5687,0xbc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a00,0x78);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1a,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a13,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a18,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a19,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a08,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a09,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0a,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0b,0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350c,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350d,0xd0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3500,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528a,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528c,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528d,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528e,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5290,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5292,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5293,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5294,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5295,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5296,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5297,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5298,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5299,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529b,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529d,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529f,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0f,0x3c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a10,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1b,0x3c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1e,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a11,0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3030,0x2b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a02,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a03,0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a04,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a14,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a15,0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a16,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a00,0x78);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a08,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a09,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0a,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0b,0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0d,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0e,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5193,0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589a,0xc5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4001,0x42);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x401c,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528a,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528c,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528d,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528e,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528f,0x28);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5290,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5292,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5293,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5294,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5295,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5296,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5297,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5298,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5299,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529b,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529d,0x28);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529f,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5282,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5300,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5301,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5302,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5303,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530d,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530e,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530f,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5310,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5311,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5308,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5309,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5304,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5305,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5306,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5307,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5314,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5315,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5319,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5316,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5317,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5318,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5380,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5381,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5382,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5383,0x4e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5384,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5385,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5386,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5387,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5388,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5389,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538b,0x31);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538d,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538f,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5390,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5391,0xab);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5392,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5393,0xa2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5394,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5480,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5481,0x21);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5482,0x36);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5483,0x57);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5484,0x65);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5485,0x71);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5486,0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5487,0x87);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5488,0x91);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5489,0x9a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548a,0xaa);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548b,0xb8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548c,0xcd);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548d,0xdd);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548e,0xea);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548f,0x1d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5490,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5491,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5492,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5493,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5494,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5495,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5496,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5497,0xb8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5498,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5499,0x86);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549a,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549b,0x5b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549c,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549d,0x3b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549e,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549f,0x1c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a0,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a1,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a2,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a3,0xed);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a4,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a5,0xc5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a6,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a7,0xa5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a8,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a9,0x6c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54aa,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ab,0x41);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ac,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ad,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ae,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54af,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b0,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b1,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b2,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b3,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b4,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b5,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b6,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b7,0xdf);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5402,0x3f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5403,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3406,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5180,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5181,0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5182,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5183,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5184,0x25);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5185,0x24);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5186,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5187,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5188,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5189,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518a,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518b,0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518c,0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518d,0x44);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518e,0x3d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518f,0x58);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5190,0x46);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5191,0xf8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5192,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5193,0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5194,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5195,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5196,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5197,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5198,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5199,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519a,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519c,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519d,0x82);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5025,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0f,0x38);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a10,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1b,0x3a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1e,0x2e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a11,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5688,0xa6);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5689,0x6a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568a,0xea);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568b,0xae);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568c,0xa6);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568d,0x6a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568e,0x62);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568f,0x26);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5583,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5584,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5580,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0xcf);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5800,0x27);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5801,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5802,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5803,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5804,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5805,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5806,0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5807,0x2f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5808,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5809,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580a,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580b,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580c,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580d,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580e,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580f,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5810,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5811,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5812,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5813,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5814,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5815,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5816,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5817,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5818,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5819,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581a,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581d,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581e,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581f,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5820,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5821,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5822,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5823,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5824,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5825,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5826,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5827,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5828,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5829,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582a,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582c,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582d,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582e,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582f,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5830,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5831,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5832,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5833,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5834,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5835,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5836,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5837,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5838,0x32);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5839,0x1f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583a,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583b,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583c,0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583d,0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583e,0x26);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583f,0x53);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5840,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5841,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5842,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5843,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5844,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5845,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5846,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5847,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5848,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5849,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584a,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584b,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584c,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584d,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584e,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5850,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5851,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5852,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5853,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5854,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5855,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5856,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5857,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5858,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5859,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585a,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585b,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585c,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585d,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585e,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585f,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5860,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5861,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5862,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5863,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5864,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5865,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5866,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5867,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5868,0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5869,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586a,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586b,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586c,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586d,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586e,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586f,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5870,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5871,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5872,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5873,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5874,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5875,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5876,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5877,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5878,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5879,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587a,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587b,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587c,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587d,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587e,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587f,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5880,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5881,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5882,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5883,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5884,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5885,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5886,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5887,0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3710,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3632,0x51);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3702,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3703,0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3704,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370b,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370d,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3631,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3632,0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x24);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3620,0x96);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5785,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a13,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3600,0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x48);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x1b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370d,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370f,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3709,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3823,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5007,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5009,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5011,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5013,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5086,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5087,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5088,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5089,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x302b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0x87);//

I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a00,0x78);//

}
static void ov_setVGA_640x480(void)
{


/*
@@ VGA(YUV) 15fps
100 99 640 480
100 98 0 0
100 97 640 480
;
;OV5642 setting Version History
;
;date 07/11/2009
;--2nth release of OV5642 Rev1D(BL) setting.

*/
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3103,0x93);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3008,0x82);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3017,0x7f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3018,0xfc);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3810,0xc2);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3615,0xf0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3000,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3001,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3002,0x5c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3003,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3004,0xff);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3005,0xff);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3006,0x43);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3007,0x37);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3011,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3010,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460c,0x22);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3815,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370c,0xa0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3602,0xfc);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3612,0xff);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3634,0xc0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3613,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3605,0x7c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0x09);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3622,0x60);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x40);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3603,0xa7);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3603,0x27);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4000,0x21);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x401d,0x22);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3600,0x54);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3605,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x3f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3c01,0x80);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0x4f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5020,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5181,0x79);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5182,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5185,0x22);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5197,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5001,0xff);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5500,0x0a);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5504,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5505,0x7f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5080,0x08);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x300e,0x18);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4610,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471d,0x05);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4708,0x06);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3808,0x02);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3809,0x80);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380a,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380b,0xe0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,0x07);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,0xd0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x501f,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0x4f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4300,0x30);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x07);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x73);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x80);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x07);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3824,0x11);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x1e);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x80);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x7f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380c,0x0c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380d,0x80);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,0x03);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,0xe8);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0d,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0e,0x03);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3818,0xc1);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3705,0xdb);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370a,0x81);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3801,0x80);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0x87);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3801,0x50);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3803,0x08);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3827,0x08);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3810,0x40);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3804,0x05);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3805,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5682,0x05);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5683,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3806,0x03);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3807,0xc0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5686,0x03);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5687,0xbc);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a00,0x78);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1a,0x05);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a13,0x30);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a18,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a19,0x7c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a08,0x12);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a09,0xc0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0a,0x0f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0b,0xa0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350c,0x07);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350d,0xd0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3500,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350a,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528a,0x02);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528b,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528c,0x08);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528d,0x08);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528e,0x08);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528f,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5290,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5292,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5293,0x02);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5294,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5295,0x02);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5296,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5297,0x02);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5298,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5299,0x02);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529a,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529b,0x02);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529c,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529d,0x02);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529e,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529f,0x02);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0f,0x3c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a10,0x30);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1b,0x3c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1e,0x30);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a11,0x70);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1f,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3030,0x2b);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a02,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a03,0x7d);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a04,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a14,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a15,0x7d);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a16,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a00,0x78);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a08,0x09);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a09,0x60);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0a,0x07);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0b,0xd0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0d,0x08);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0e,0x06);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5193,0x70);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589b,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589a,0xc5);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x401e,0x20);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4001,0x42);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x401c,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528a,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528b,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528c,0x08);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528d,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528e,0x20);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528f,0x28);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5290,0x30);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5292,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5293,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5294,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5295,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5296,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5297,0x08);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5298,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5299,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529a,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529b,0x20);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529c,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529d,0x28);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529e,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529f,0x30);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5282,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5300,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5301,0x20);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5302,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5303,0x7c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530c,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530d,0x0c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530e,0x20);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530f,0x80);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5310,0x20);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5311,0x80);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5308,0x20);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5309,0x40);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5304,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5305,0x30);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5306,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5307,0x80);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5314,0x08);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5315,0x20);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5319,0x30);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5316,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5317,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5318,0x02);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5380,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5381,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5382,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5383,0x4e);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5384,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5385,0x0f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5386,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5387,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5388,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5389,0x15);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538a,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538b,0x31);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538c,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538d,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538e,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538f,0x0f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5390,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5391,0xab);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5392,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5393,0xa2);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5394,0x08);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5480,0x14);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5481,0x21);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5482,0x36);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5483,0x57);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5484,0x65);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5485,0x71);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5486,0x7d);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5487,0x87);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5488,0x91);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5489,0x9a);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548a,0xaa);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548b,0xb8);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548c,0xcd);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548d,0xdd);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548e,0xea);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548f,0x1d);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5490,0x05);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5491,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5492,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5493,0x20);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5494,0x03);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5495,0x60);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5496,0x02);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5497,0xb8);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5498,0x02);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5499,0x86);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549a,0x02);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549b,0x5b);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549c,0x02);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549d,0x3b);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549e,0x02);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549f,0x1c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a0,0x02);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a1,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a2,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a3,0xed);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a4,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a5,0xc5);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a6,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a7,0xa5);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a8,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a9,0x6c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54aa,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ab,0x41);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ac,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ad,0x20);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ae,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54af,0x16);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b0,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b1,0x20);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b2,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b3,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b4,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b5,0xf0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b6,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b7,0xdf);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5402,0x3f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5403,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3406,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5180,0xff);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5181,0x52);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5182,0x11);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5183,0x14);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5184,0x25);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5185,0x24);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5186,0x06);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5187,0x08);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5188,0x08);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5189,0x7c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518a,0x60);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518b,0xb2);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518c,0xb2);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518d,0x44);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518e,0x3d);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518f,0x58);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5190,0x46);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5191,0xf8);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5192,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5193,0x70);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5194,0xf0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5195,0xf0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5196,0x03);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5197,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5198,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5199,0x12);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519a,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519b,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519c,0x06);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519d,0x82);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519e,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5025,0x80);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0f,0x38);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a10,0x30);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1b,0x3a);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1e,0x2e);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a11,0x60);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1f,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5688,0xa6);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5689,0x6a);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568a,0xea);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568b,0xae);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568c,0xa6);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568d,0x6a);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568e,0x62);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568f,0x26);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5583,0x40);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5584,0x40);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5580,0x02);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0xcf);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5800,0x27);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5801,0x19);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5802,0x12);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5803,0x0f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5804,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5805,0x15);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5806,0x1e);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5807,0x2f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5808,0x15);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5809,0x0d);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580a,0x0a);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580b,0x09);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580c,0x0a);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580d,0x0c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580e,0x12);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580f,0x19);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5810,0x0b);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5811,0x07);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5812,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5813,0x03);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5814,0x03);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5815,0x06);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5816,0x0a);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5817,0x0f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5818,0x0a);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5819,0x05);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581a,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581b,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581c,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581d,0x03);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581e,0x08);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581f,0x0c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5820,0x0a);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5821,0x05);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5822,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5823,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5824,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5825,0x03);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5826,0x08);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5827,0x0c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5828,0x0e);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5829,0x08);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582a,0x06);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582b,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582c,0x05);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582d,0x07);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582e,0x0b);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582f,0x12);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5830,0x18);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5831,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5832,0x0c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5833,0x0a);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5834,0x0b);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5835,0x0e);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5836,0x15);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5837,0x19);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5838,0x32);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5839,0x1f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583a,0x18);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583b,0x16);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583c,0x17);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583d,0x1e);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583e,0x26);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583f,0x53);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5840,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5841,0x0f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5842,0x0d);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5843,0x0c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5844,0x0e);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5845,0x09);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5846,0x11);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5847,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5848,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5849,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584a,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584b,0x0e);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584c,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584d,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584e,0x11);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584f,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5850,0x0f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5851,0x0c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5852,0x0f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5853,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5854,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5855,0x0f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5856,0x0e);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5857,0x0b);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5858,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5859,0x0d);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585a,0x0d);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585b,0x0c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585c,0x0c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585d,0x0c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585e,0x0b);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585f,0x0c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5860,0x0c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5861,0x0c);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5862,0x0d);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5863,0x08);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5864,0x11);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5865,0x18);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5866,0x18);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5867,0x19);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5868,0x17);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5869,0x19);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586a,0x16);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586b,0x13);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586c,0x13);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586d,0x12);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586e,0x13);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586f,0x16);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5870,0x14);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5871,0x12);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5872,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5873,0x11);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5874,0x11);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5875,0x16);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5876,0x14);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5877,0x11);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5878,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5879,0x0f);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587a,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587b,0x14);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587c,0x13);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587d,0x12);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587e,0x11);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587f,0x11);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5880,0x12);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5881,0x15);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5882,0x14);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5883,0x15);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5884,0x15);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5885,0x15);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5886,0x13);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5887,0x17);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3710,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3632,0x51);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3702,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3703,0xb2);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3704,0x18);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370b,0x40);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370d,0x03);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3631,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3632,0x52);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x24);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3620,0x96);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5785,0x07);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a13,0x30);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3600,0x52);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x48);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x1b);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370d,0x0b);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370f,0xc0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3709,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3823,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5007,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5009,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5011,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5013,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519e,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5086,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5087,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5088,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5089,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x302b,0x00);

}

static void OV_setQXGA_2048x1536(void)
{
	MP_ALERT("======== %s ========", __FUNCTION__);
	ov_set2M_1600x1200();
	
	//	@@ 5M to QXGA Key  QXGA 2048กั1536
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3406,	0x01);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3003,	0x00);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3005,	0xFF);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3006,	0xFF);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3007,	0x3F);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3011,	0x10);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3012,	0x00);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x350C,	0x07);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x350D,	0xD0);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3602,	0xE4);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3612,	0xAC);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3613,	0x44);// 
//	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3621,	0x29);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3622,	0x60);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3623,	0x22);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3604,	0x60);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3705,	0xDA);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x370A,	0x80);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x370D,	0x03);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3801,	0x8A);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3803,	0x0A);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3804,	0x0A);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3805,	0x20);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3806,	0x07);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3807,	0x98);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3808,	0x0A);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3809,	0x20);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380A,	0x07);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380B,	0x98);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380C,	0x0C);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380D,	0x80);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380E,	0x07);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380F,	0xD0);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3824,	0x11);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3825,	0xAC);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3827,	0x0A);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3A08,	0x09);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3A09,	0x60);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3A0A,	0x07);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3A0B,	0xD0);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3A0D,	0x10);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3A0E,	0x0D);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3A1A,	0x04);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x460B,	0x35);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x471D,	0x00);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x4713,	0x03);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5001,	0xFF);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x589B,	0x00);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x589A,	0xC0);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x4407,	0x04);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x589B,	0x00);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x589A,	0xC0);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3002,	0x1C);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x460C,	0x20);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x471C,	0xD0);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x4721,	0x01);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3815,	0x01);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x501F,	0x00);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5002,	0xE0);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x4300,	0x30);// 
	//I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3818,	0x81);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3810,	0xC2);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3010,	0x70);// 
																 
	//;QXGA 													 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3800,	0x01);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3801,	0x8A);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3802,	0x00);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3803,	0x0A);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3804,	0x0A);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3805,	0x20);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3806,	0x07);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3807,	0x98);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3808,	0x08);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x3809,	0x00);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380A,	0x06);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380B,	0x00);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380C,	0x0C);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380D,	0x80);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380E,	0x07);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x380F,	0xD0);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5001,	0x7F);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5680,	0x00);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5681,	0x00);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5682,	0x0A);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5683,	0x20);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5684,	0x00);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5685,	0x00);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5686,	0x07);// 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,  0x5687,	0x98);// 
}
static void ov_set2M_1600x1200(void)
{
//@@ OV5642 UXGA YUV initial setting
//100 99 1600 1200
//100 98 0 0
//100 97 640 480


I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3103,0x93);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3008,0x82);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3017,0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3018,0xfc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3810,0xc2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3615,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3000,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3001,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3002,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3003,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3004,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3030,0x2b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3011,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3010,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3622,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3709,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4000,0x21);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x401d,0x22);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3600,0x54);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3605,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x3f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3c01,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x300d,0x22);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3623,0x22);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0x4f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5020,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5181,0x79);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5182,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5185,0x22);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5197,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5500,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5504,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5505,0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5080,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x300e,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4610,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471d,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4708,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370c,0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3808,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3809,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380a,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380b,0x98);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380c,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380d,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,0xd0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5687,0x94);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x501f,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0x4f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5001,0xcf);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4300,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4300,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460b,0x35);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471d,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3002,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3002,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4713,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471c,0x50);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4721,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4402,0x90);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460c,0x22);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3815,0x44);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x73);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3818,0xc8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3801,0x88);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3824,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a00,0x78);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1a,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a13,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a18,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a19,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a08,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a09,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0a,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0b,0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350c,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350d,0xd0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0d,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0e,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3500,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0f,0x3c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a10,0x32);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1b,0x3c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1e,0x32);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a11,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1f,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3030,0x2b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a02,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a03,0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a04,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a14,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a15,0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a16,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a00,0x78);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a08,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a09,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0a,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0b,0xd0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0d,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0e,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4407,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5193,0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589a,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4001,0x42);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x401c,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3825,0xac);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3827,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528a,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528c,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528d,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528e,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528f,0x28);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5290,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5292,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5293,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5294,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5295,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5296,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5297,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5298,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5299,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529b,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529d,0x28);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529f,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5282,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5300,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5301,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5302,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5303,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530d,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530e,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530f,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5310,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5311,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5308,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5309,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5304,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5305,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5306,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5307,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5314,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5315,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5319,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5316,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5317,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5318,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5380,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5381,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5382,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5383,0x4e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5384,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5385,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5386,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5387,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5388,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5389,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538b,0x31);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538d,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538f,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5390,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5391,0xab);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5392,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5393,0xa2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5394,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5480,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5481,0x21);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5482,0x36);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5483,0x57);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5484,0x65);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5485,0x71);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5486,0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5487,0x87);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5488,0x91);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5489,0x9a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548a,0xaa);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548b,0xb8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548c,0xcd);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548d,0xdd);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548e,0xea);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548f,0x1d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5490,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5491,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5492,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5493,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5494,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5495,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5496,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5497,0xb8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5498,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5499,0x86);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549a,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549b,0x5b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549c,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549d,0x3b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549e,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549f,0x1c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a0,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a1,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a2,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a3,0xed);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a4,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a5,0xc5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a6,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a7,0xa5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a8,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a9,0x6c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54aa,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ab,0x41);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ac,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ad,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ae,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54af,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b0,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b1,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b2,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b3,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b4,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b5,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b6,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b7,0xdf);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5402,0x3f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5403,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3406,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5180,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5181,0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5182,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5183,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5184,0x25);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5185,0x24);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5186,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5187,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5188,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5189,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518a,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518b,0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518c,0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518d,0x44);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518e,0x3d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518f,0x58);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5190,0x46);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5191,0xf8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5192,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5193,0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5194,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5195,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5196,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5197,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5198,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5199,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519a,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519c,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519d,0x82);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5025,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0f,0x38);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a10,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1b,0x3a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1e,0x2e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a11,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5688,0xa6);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5689,0x6a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568a,0xea);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568b,0xae);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568c,0xa6);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568d,0x6a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568e,0x62);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568f,0x26);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5583,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5584,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5580,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0xcf);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5800,0x27);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5801,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5802,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5803,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5804,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5805,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5806,0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5807,0x2f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5808,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5809,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580a,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580b,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580c,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580d,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580e,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580f,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5810,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5811,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5812,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5813,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5814,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5815,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5816,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5817,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5818,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5819,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581a,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581d,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581e,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581f,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5820,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5821,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5822,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5823,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5824,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5825,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5826,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5827,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5828,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5829,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582a,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582c,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582d,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582e,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582f,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5830,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5831,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5832,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5833,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5834,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5835,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5836,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5837,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5838,0x32);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5839,0x1f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583a,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583b,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583c,0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583d,0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583e,0x26);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583f,0x53);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5840,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5841,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5842,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5843,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5844,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5845,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5846,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5847,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5848,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5849,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584a,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584b,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584c,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584d,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584e,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5850,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5851,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5852,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5853,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5854,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5855,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5856,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5857,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5858,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5859,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585a,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585b,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585c,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585d,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585e,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585f,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5860,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5861,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5862,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5863,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5864,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5865,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5866,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5867,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5868,0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5869,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586a,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586b,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586c,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586d,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586e,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586f,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5870,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5871,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5872,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5873,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5874,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5875,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5876,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5877,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5878,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5879,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587a,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587b,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587c,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587d,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587e,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587f,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5880,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5881,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5882,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5883,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5884,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5885,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5886,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5887,0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3710,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3632,0x51);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3702,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3703,0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3704,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370b,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370d,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3631,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3632,0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x24);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3620,0x96);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5785,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a13,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3600,0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x48);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x1b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370d,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370f,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3709,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3823,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5007,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5009,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5011,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5013,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5086,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5087,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5088,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5089,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x302b,0x00);//
//;YUV key setting
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460c,0x20);//; auto pclk
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460b,0x37);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471c,0xd0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471d,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3815,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3818,0xc0);//;
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x501f,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4300,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3002,0x1c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4721,0x01);//

I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3824,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3825,0xB0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3826,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3827,0x0C);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3804,0x0A);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3805,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3806,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3807,0x98);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3808,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3809,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380A,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380B,0xB0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380C,0x0C);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380D,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380E,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380F,0xD0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5001,0x7F);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5680,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5681,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5682,0x0A);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5683,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5684,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5685,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5686,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5687,0x98);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3010,0x70);//
}

static void ov_set5M_2560x1920(void)
{
//@@ 5M Key
//100 99 2592 1944
//100 98 6 6

	ov_set2M_1600x1200();

	//@@ OV5642 Capture 5M Key setting
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3406,0x01);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3003,0x00);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3005,0xFF);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3006,0xFF);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3007,0x3F);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3011,0x10);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3012,0x00);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350C,0x07);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350D,0xD0);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3602,0xE4);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3612,0xAC);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3613,0x44);//
	//I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0x09 29 ;Mirror & Flip
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3622,0x60);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3623,0x22);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x60);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3705,0xDA);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370A,0x80);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370D,0x03);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3801,0x8A);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3803,0x0A);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3804,0x0A);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3805,0x20);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3806,0x07);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3807,0x98);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3808,0x0A);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3809,0x20);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380A,0x07);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380B,0x98);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380C,0x0C);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380D,0x80);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380E,0x07);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380F,0xD0);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3824,0x11);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3825,0xAC);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3827,0x0A);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A08,0x09);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A09,0x60);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A0A,0x07);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A0B,0xD0);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A0D,0x10);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A0E,0x0D);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A1A,0x04);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460B,0x35);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471D,0x00);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4713,0x03);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5001,0xFF);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589B,0x00);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589A,0xC0);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4407,0x04);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589B,0x00);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589A,0xC0);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3002,0x1C);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460C,0x20);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471C,0xD0);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4721,0x01);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3815,0x01);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x501F,0x00);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5002,0xE0);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4300,0x30);//
	//I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3818,0xc0 A0 ;Mirror&Flip
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3810,0xC2);//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3010,0x70);//
    //I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3818,0xc1);//
}


static void ov_set_352x240_30FPS(void)
{
//@@ QVGA(YUV) 30fps
//100 99 320 240
//100 98 0 0
//100 97 320 240
//;
//;OV5642 setting Version History
//;
//;date 07/11/2009
//;--2nth release of OV5642 Rev1D(BL) setting.

I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3103,0x93);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3008,0x82);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3017,0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3018,0xfc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3615,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3000,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3001,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3002,0x5c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3003,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3004,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3005,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3006,0x43);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3007,0x37);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3011,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3012,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3010,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460c,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3815,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370c,0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3602,0xfc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3612,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3634,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3613,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3605,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3622,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3603,0xa7);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3603,0x27);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4000,0x21);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x401d,0x22);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3600,0x54);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3605,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x3f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3c01,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0x4f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5020,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5181,0x79);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5182,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5185,0x22);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5197,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5001,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5500,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5504,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5505,0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5080,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x300e,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4610,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471d,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4708,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3808,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3809,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380a,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380b,0xe0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,0xd0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x501f,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0x4f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4300,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x73);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3824,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3825,0xb0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380c,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380d,0x2a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,0xe8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0d,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0e,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3818,0xc1);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3705,0xdb);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370a,0x81);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3801,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0xc7);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3801,0x50);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3803,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3827,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3810,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3804,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3805,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5682,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5683,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3806,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3807,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5686,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5687,0xbc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a00,0x78);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1a,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a13,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a18,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a19,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a08,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a09,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0a,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0b,0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350c,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350d,0xd0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3500,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528a,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528c,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528d,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528e,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5290,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5292,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5293,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5294,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5295,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5296,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5297,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5298,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5299,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529b,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529d,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529f,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0f,0x3c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a10,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1b,0x3c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1e,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a11,0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3030,0x2b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a02,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a03,0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a04,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a14,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a15,0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a16,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a00,0x78);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a08,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a09,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0a,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0b,0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0d,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0e,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5193,0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589a,0xc5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4001,0x42);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x401c,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528a,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528c,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528d,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528e,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528f,0x28);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5290,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5292,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5293,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5294,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5295,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5296,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5297,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5298,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5299,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529b,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529d,0x28);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529f,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5282,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5300,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5301,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5302,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5303,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530d,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530e,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530f,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5310,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5311,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5308,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5309,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5304,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5305,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5306,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5307,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5314,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5315,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5319,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5316,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5317,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5318,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5380,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5381,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5382,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5383,0x4e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5384,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5385,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5386,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5387,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5388,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5389,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538b,0x31);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538d,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538f,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5390,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5391,0xab);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5392,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5393,0xa2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5394,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5480,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5481,0x21);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5482,0x36);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5483,0x57);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5484,0x65);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5485,0x71);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5486,0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5487,0x87);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5488,0x91);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5489,0x9a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548a,0xaa);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548b,0xb8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548c,0xcd);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548d,0xdd);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548e,0xea);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548f,0x1d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5490,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5491,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5492,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5493,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5494,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5495,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5496,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5497,0xb8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5498,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5499,0x86);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549a,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549b,0x5b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549c,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549d,0x3b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549e,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549f,0x1c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a0,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a1,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a2,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a3,0xed);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a4,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a5,0xc5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a6,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a7,0xa5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a8,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a9,0x6c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54aa,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ab,0x41);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ac,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ad,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ae,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54af,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b0,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b1,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b2,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b3,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b4,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b5,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b6,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b7,0xdf);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5402,0x3f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5403,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3406,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5180,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5181,0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5182,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5183,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5184,0x25);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5185,0x24);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5186,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5187,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5188,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5189,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518a,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518b,0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518c,0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518d,0x44);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518e,0x3d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518f,0x58);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5190,0x46);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5191,0xf8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5192,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5193,0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5194,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5195,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5196,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5197,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5198,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5199,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519a,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519c,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519d,0x82);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5025,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0f,0x38);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a10,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1b,0x3a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1e,0x2e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a11,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5688,0xa6);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5689,0x6a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568a,0xea);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568b,0xae);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568c,0xa6);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568d,0x6a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568e,0x62);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568f,0x26);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5583,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5584,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5580,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0xcf);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5800,0x27);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5801,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5802,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5803,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5804,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5805,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5806,0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5807,0x2f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5808,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5809,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580a,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580b,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580c,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580d,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580e,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580f,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5810,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5811,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5812,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5813,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5814,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5815,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5816,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5817,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5818,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5819,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581a,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581d,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581e,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581f,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5820,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5821,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5822,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5823,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5824,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5825,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5826,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5827,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5828,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5829,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582a,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582c,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582d,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582e,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582f,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5830,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5831,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5832,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5833,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5834,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5835,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5836,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5837,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5838,0x32);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5839,0x1f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583a,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583b,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583c,0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583d,0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583e,0x26);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583f,0x53);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5840,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5841,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5842,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5843,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5844,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5845,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5846,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5847,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5848,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5849,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584a,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584b,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584c,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584d,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584e,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5850,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5851,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5852,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5853,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5854,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5855,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5856,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5857,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5858,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5859,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585a,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585b,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585c,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585d,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585e,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585f,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5860,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5861,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5862,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5863,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5864,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5865,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5866,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5867,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5868,0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5869,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586a,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586b,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586c,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586d,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586e,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586f,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5870,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5871,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5872,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5873,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5874,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5875,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5876,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5877,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5878,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5879,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587a,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587b,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587c,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587d,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587e,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587f,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5880,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5881,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5882,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5883,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5884,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5885,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5886,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5887,0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3710,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3632,0x51);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3702,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3703,0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3704,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370b,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370d,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3631,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3632,0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x24);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3620,0x96);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5785,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a13,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3600,0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x48);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x1b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370d,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370f,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3709,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3823,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5007,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5009,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5011,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5013,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5086,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5087,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5088,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5089,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x302b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0x87);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3808,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3809,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380b,0xf0);//

}


static void ov_set_720P_1280x720(void)
{
		 /*
		@@ JPG_720P 30fps
		100 99 1280 720
		100 98 6 6
		100 97 640 480
		;
		;OV5642 setting Version History
		;
		;date 07/11/2009
		;--2nth release of OV5642 Rev1D(BL) setting.
		*/

		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3103,0x93);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3008,0x82);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3017,0x7f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3018,0xfc);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3810,0xc2);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3615,0xf0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3000,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3001,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3002,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3003,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3004,0xff);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3030,0x2b);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3011,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3010,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x60);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3622,0x60);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0x09);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3709,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4000,0x21);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x401d,0x22);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3600,0x54);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3605,0x04);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x3f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3c01,0x80);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x300d,0x22);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3623,0x22);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0x4f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5020,0x04);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5181,0x79);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5182,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5185,0x22);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5197,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5500,0x0a);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5504,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5505,0x7f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5080,0x08);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x300e,0x18);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4610,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471d,0x05);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4708,0x06);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370c,0xa0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3808,0x0a);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3809,0x20);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380a,0x07);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380b,0x98);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380c,0x0c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380d,0x80);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,0x07);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,0xd0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5687,0x94);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x501f,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0x4f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5001,0xcf);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4300,0x30);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4300,0x30);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460b,0x35);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471d,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3002,0x0c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3002,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4713,0x03);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471c,0x50);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4721,0x02);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4402,0x90);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460c,0x22);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3815,0x44);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x07);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x73);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x80);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3818,0xc8);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3801,0x88);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3824,0x11);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a00,0x78);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1a,0x04);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a13,0x30);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a18,0x00);//

		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a19,0x7c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a08,0x12);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a09,0xc0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0a,0x0f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0b,0xa0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350c,0x07);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350d,0xd0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0d,0x08);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0e,0x06);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3500,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350a,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0f,0x3c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a10,0x32);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1b,0x3c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1e,0x32);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a11,0x80);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1f,0x20);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3030,0x2b);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a02,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a03,0x7d);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a04,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a14,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a15,0x7d);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a16,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a00,0x78);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a08,0x09);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a09,0x60);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0a,0x07);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0b,0xd0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0d,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0e,0x0d);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4407,0x04);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5193,0x70);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589b,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589a,0xc0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4001,0x42);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x401c,0x06);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3825,0xac);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3827,0x0c);//



		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528a,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528b,0x04);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528c,0x08);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528d,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528e,0x20);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528f,0x28);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5290,0x30);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5292,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5293,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5294,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5295,0x04);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5296,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5297,0x08);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5298,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5299,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529a,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529b,0x20);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529c,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529d,0x28);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529e,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529f,0x30);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5282,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5300,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5301,0x20);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5302,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5303,0x7c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530c,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530d,0x0c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530e,0x20);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530f,0x80);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5310,0x20);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5311,0x80);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5308,0x20);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5309,0x40);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5304,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5305,0x30);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5306,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5307,0x80);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5314,0x08);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5315,0x20);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5319,0x30);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5316,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5317,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5318,0x02);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5380,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5381,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5382,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5383,0x4e);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5384,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5385,0x0f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5386,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5387,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5388,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5389,0x15);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538a,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538b,0x31);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538c,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538d,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538e,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538f,0x0f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5390,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5391,0xab);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5392,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5393,0xa2);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5394,0x08);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5480,0x14);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5481,0x21);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5482,0x36);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5483,0x57);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5484,0x65);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5485,0x71);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5486,0x7d);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5487,0x87);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5488,0x91);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5489,0x9a);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548a,0xaa);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548b,0xb8);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548c,0xcd);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548d,0xdd);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548e,0xea);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548f,0x1d);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5490,0x05);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5491,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5492,0x04);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5493,0x20);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5494,0x03);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5495,0x60);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5496,0x02);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5497,0xb8);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5498,0x02);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5499,0x86);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549a,0x02);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549b,0x5b);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549c,0x02);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549d,0x3b);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549e,0x02);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549f,0x1c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a0,0x02);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a1,0x04);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a2,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a3,0xed);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a4,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a5,0xc5);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a6,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a7,0xa5);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a8,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a9,0x6c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54aa,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ab,0x41);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ac,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ad,0x20);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ae,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54af,0x16);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b0,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b1,0x20);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b2,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b3,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b4,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b5,0xf0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b6,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b7,0xdf);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5402,0x3f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5403,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3406,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5180,0xff);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5181,0x52);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5182,0x11);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5183,0x14);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5184,0x25);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5185,0x24);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5186,0x06);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5187,0x08);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5188,0x08);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5189,0x7c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518a,0x60);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518b,0xb2);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518c,0xb2);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518d,0x44);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518e,0x3d);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518f,0x58);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5190,0x46);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5191,0xf8);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5192,0x04);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5193,0x70);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5194,0xf0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5195,0xf0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5196,0x03);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5197,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5198,0x04);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5199,0x12);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519a,0x04);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519b,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519c,0x06);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519d,0x82);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519e,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5025,0x80);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0f,0x38);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a10,0x30);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1b,0x3a);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1e,0x2e);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a11,0x60);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1f,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5688,0xa6);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5689,0x6a);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568a,0xea);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568b,0xae);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568c,0xa6);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568d,0x6a);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568e,0x62);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568f,0x26);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5583,0x40);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5584,0x40);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5580,0x02);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0xcf);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5800,0x27);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5801,0x19);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5802,0x12);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5803,0x0f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5804,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5805,0x15);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5806,0x1e);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5807,0x2f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5808,0x15);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5809,0x0d);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580a,0x0a);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580b,0x09);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580c,0x0a);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580d,0x0c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580e,0x12);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580f,0x19);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5810,0x0b);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5811,0x07);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5812,0x04);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5813,0x03);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5814,0x03);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5815,0x06);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5816,0x0a);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5817,0x0f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5818,0x0a);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5819,0x05);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581a,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581b,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581c,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581d,0x03);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581e,0x08);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581f,0x0c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5820,0x0a);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5821,0x05);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5822,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5823,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5824,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5825,0x03);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5826,0x08);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5827,0x0c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5828,0x0e);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5829,0x08);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582a,0x06);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582b,0x04);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582c,0x05);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582d,0x07);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582e,0x0b);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582f,0x12);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5830,0x18);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5831,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5832,0x0c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5833,0x0a);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5834,0x0b);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5835,0x0e);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5836,0x15);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5837,0x19);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5838,0x32);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5839,0x1f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583a,0x18);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583b,0x16);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583c,0x17);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583d,0x1e);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583e,0x26);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583f,0x53);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5840,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5841,0x0f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5842,0x0d);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5843,0x0c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5844,0x0e);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5845,0x09);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5846,0x11);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5847,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5848,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5849,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584a,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584b,0x0e);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584c,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584d,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584e,0x11);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584f,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5850,0x0f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5851,0x0c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5852,0x0f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5853,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5854,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5855,0x0f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5856,0x0e);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5857,0x0b);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5858,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5859,0x0d);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585a,0x0d);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585b,0x0c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585c,0x0c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585d,0x0c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585e,0x0b);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585f,0x0c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5860,0x0c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5861,0x0c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5862,0x0d);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5863,0x08);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5864,0x11);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5865,0x18);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5866,0x18);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5867,0x19);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5868,0x17);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5869,0x19);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586a,0x16);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586b,0x13);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586c,0x13);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586d,0x12);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586e,0x13);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586f,0x16);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5870,0x14);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5871,0x12);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5872,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5873,0x11);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5874,0x11);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5875,0x16);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5876,0x14);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5877,0x11);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5878,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5879,0x0f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587a,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587b,0x14);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587c,0x13);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587d,0x12);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587e,0x11);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587f,0x11);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5880,0x12);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5881,0x15);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5882,0x14);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5883,0x15);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5884,0x15);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5885,0x15);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5886,0x13);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5887,0x17);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3710,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3632,0x51);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3702,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3703,0xb2);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3704,0x18);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370b,0x40);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370d,0x03);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3631,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3632,0x52);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x24);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3620,0x96);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5785,0x07);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a13,0x30);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3600,0x52);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x48);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x1b);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370d,0x0b);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370f,0xc0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3709,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3823,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5007,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5009,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5011,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5013,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519e,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5086,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5087,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5088,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5089,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x302b,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x07);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3011,0x10);//PLL DIV


		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350c,0x02);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350d,0xe4);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0xc9);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370a,0x81);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3803,0x08);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3804,0x05);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3805,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3806,0x02);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3807,0xd0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3808,0x05);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3809,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380a,0x02);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380b,0xd0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380c,0x08);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380d,0x72);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,0x02);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,0xe4);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3810,0xc0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3818,0xc9);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x381c,0x10);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x381d,0xa0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x381e,0x05);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x381f,0xb0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3820,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3821,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3824,0x11);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a08,0x1b);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a09,0xc0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0a,0x17);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0b,0x20);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0d,0x02);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0e,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x401c,0x04);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5682,0x05);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5683,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5686,0x02);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5687,0xcc);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5001,0x7f);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589b,0x06);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589a,0xc5);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x00);//

		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3010,0x10);//

		//;@@ JPG2YUV
		//100 98 0 0

		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460c,0x20);//; auto pclk
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460b,0x37);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471c,0xd0);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471d,0x05);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3815,0x01);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3818,0xd0);//;
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x501f,0x00);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4300,0x30);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3002,0x1c);//
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4721,0x01);//

		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3818,0xc1);//

		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,1380>>8);//dummy
		I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,1380&0xFF);//

		switch(sensor_FrameRate)//(VIDEO_RECORDING_FPS)
		{
			case 16:
				//frame rate=16  add dummy
				I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,1380>>8);//dummy
				I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,1380&0xFF);//
				break;
			case 20:
				//frame rate=20 PLL DIV
				//I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3011,0x08);//PLL DIV
				//I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3010,0x20);//PLL DIV
				//frame rate=20  add dummy
				I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,1110>>8);//dummy
				I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,1110&0xFF);//
				break;
			case 24:
				//frame rate=24  add dummy
				I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,0x03);//dummy
				I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,0x9d);//
				break;
			default:
				//frame rate<10  DIV PLL
				I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3011,0x10);//PLL DIV
				//I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3010,0x20);//PLL DIV
				break;
		}

}


static void ov_set_720x480_30FPS(void)
{
//@@ QVGA(YUV) 30fps
//100 99 320 240
//100 98 0 0
//100 97 320 240
//;
//;OV5642 setting Version History
//;
//;date 07/11/2009
//;--2nth release of OV5642 Rev1D(BL) setting.

I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3103,0x93);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3008,0x82);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3017,0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3018,0xfc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3615,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3000,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3001,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3002,0x5c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3003,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3004,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3005,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3006,0x43);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3007,0x37);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3011,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3012,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3010,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460c,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3815,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370c,0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3602,0xfc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3612,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3634,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3613,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3605,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3622,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3603,0xa7);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3603,0x27);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4000,0x21);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x401d,0x22);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3600,0x54);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3605,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x3f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3c01,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0x4f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5020,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5181,0x79);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5182,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5185,0x22);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5197,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5001,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5500,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5504,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5505,0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5080,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x300e,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4610,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471d,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4708,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3808,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3809,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380a,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380b,0xe0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,0xd0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x501f,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0x4f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4300,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x73);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3824,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3825,0xb0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380c,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380d,0x2a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,0xe8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0d,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0e,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3818,0xc1);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3705,0xdb);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370a,0x81);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3801,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0xc7);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3801,0x50);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3803,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3827,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3810,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3804,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3805,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5682,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5683,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3806,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3807,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5686,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5687,0xbc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a00,0x78);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1a,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a13,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a18,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a19,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a08,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a09,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0a,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0b,0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350c,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350d,0xd0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3500,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528a,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528c,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528d,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528e,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5290,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5292,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5293,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5294,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5295,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5296,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5297,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5298,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5299,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529b,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529d,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529f,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0f,0x3c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a10,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1b,0x3c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1e,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a11,0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3030,0x2b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a02,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a03,0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a04,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a14,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a15,0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a16,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a00,0x78);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a08,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a09,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0a,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0b,0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0d,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0e,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5193,0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589a,0xc5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4001,0x42);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x401c,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528a,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528c,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528d,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528e,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528f,0x28);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5290,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5292,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5293,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5294,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5295,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5296,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5297,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5298,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5299,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529b,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529d,0x28);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529f,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5282,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5300,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5301,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5302,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5303,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530d,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530e,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530f,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5310,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5311,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5308,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5309,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5304,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5305,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5306,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5307,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5314,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5315,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5319,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5316,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5317,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5318,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5380,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5381,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5382,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5383,0x4e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5384,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5385,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5386,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5387,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5388,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5389,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538b,0x31);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538d,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538f,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5390,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5391,0xab);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5392,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5393,0xa2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5394,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5480,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5481,0x21);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5482,0x36);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5483,0x57);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5484,0x65);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5485,0x71);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5486,0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5487,0x87);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5488,0x91);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5489,0x9a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548a,0xaa);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548b,0xb8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548c,0xcd);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548d,0xdd);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548e,0xea);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548f,0x1d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5490,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5491,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5492,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5493,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5494,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5495,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5496,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5497,0xb8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5498,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5499,0x86);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549a,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549b,0x5b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549c,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549d,0x3b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549e,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549f,0x1c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a0,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a1,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a2,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a3,0xed);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a4,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a5,0xc5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a6,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a7,0xa5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a8,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a9,0x6c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54aa,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ab,0x41);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ac,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ad,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ae,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54af,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b0,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b1,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b2,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b3,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b4,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b5,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b6,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b7,0xdf);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5402,0x3f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5403,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3406,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5180,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5181,0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5182,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5183,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5184,0x25);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5185,0x24);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5186,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5187,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5188,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5189,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518a,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518b,0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518c,0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518d,0x44);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518e,0x3d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518f,0x58);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5190,0x46);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5191,0xf8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5192,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5193,0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5194,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5195,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5196,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5197,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5198,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5199,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519a,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519c,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519d,0x82);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5025,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0f,0x38);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a10,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1b,0x3a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1e,0x2e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a11,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5688,0xa6);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5689,0x6a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568a,0xea);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568b,0xae);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568c,0xa6);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568d,0x6a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568e,0x62);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568f,0x26);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5583,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5584,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5580,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0xcf);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5800,0x27);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5801,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5802,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5803,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5804,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5805,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5806,0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5807,0x2f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5808,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5809,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580a,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580b,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580c,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580d,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580e,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580f,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5810,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5811,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5812,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5813,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5814,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5815,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5816,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5817,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5818,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5819,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581a,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581d,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581e,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581f,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5820,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5821,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5822,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5823,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5824,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5825,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5826,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5827,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5828,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5829,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582a,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582c,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582d,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582e,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582f,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5830,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5831,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5832,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5833,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5834,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5835,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5836,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5837,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5838,0x32);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5839,0x1f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583a,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583b,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583c,0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583d,0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583e,0x26);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583f,0x53);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5840,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5841,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5842,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5843,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5844,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5845,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5846,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5847,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5848,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5849,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584a,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584b,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584c,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584d,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584e,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5850,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5851,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5852,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5853,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5854,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5855,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5856,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5857,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5858,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5859,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585a,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585b,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585c,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585d,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585e,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585f,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5860,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5861,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5862,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5863,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5864,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5865,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5866,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5867,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5868,0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5869,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586a,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586b,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586c,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586d,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586e,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586f,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5870,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5871,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5872,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5873,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5874,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5875,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5876,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5877,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5878,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5879,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587a,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587b,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587c,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587d,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587e,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587f,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5880,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5881,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5882,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5883,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5884,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5885,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5886,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5887,0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3710,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3632,0x51);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3702,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3703,0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3704,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370b,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370d,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3631,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3632,0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x24);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3620,0x96);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5785,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a13,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3600,0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x48);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x1b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370d,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370f,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3709,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3823,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5007,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5009,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5011,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5013,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5086,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5087,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5088,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5089,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x302b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0x87);//


I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3808,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3809,0xD0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380a,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380b,0xe0);//

}

static void ov_setXGA_1024x768_30FPS(void)
{


I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3103,0x93);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3008,0x82);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3017,0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3018,0xfc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3615,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3000,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3001,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3002,0x5c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3003,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3004,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3005,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3006,0x43);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3007,0x37);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3011,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3012,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3010,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460c,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3815,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370c,0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3602,0xfc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3612,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3634,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3613,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3605,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3622,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3603,0xa7);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3603,0x27);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4000,0x21);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x401d,0x22);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3600,0x54);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3605,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x3f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3c01,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0x4f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5020,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5181,0x79);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5182,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5185,0x22);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5197,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5001,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5500,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5504,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5505,0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5080,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x300e,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4610,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471d,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4708,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3808,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3809,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380a,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380b,0xe0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,0xd0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x501f,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0x4f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4300,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x73);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3824,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3825,0xb0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x7f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380c,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380d,0x2a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380e,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380f,0xe8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0d,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0e,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3818,0xc1);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3705,0xdb);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370a,0x81);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3801,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0xc7);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3801,0x50);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3803,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3827,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3810,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3804,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3805,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5682,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5683,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3806,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3807,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5686,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5687,0xbc);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a00,0x78);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1a,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a13,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a18,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a19,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a08,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a09,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0a,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0b,0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350c,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350d,0xd0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3500,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528a,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528c,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528d,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528e,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5290,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5292,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5293,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5294,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5295,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5296,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5297,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5298,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5299,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529b,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529d,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529f,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0f,0x3c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a10,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1b,0x3c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1e,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a11,0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3030,0x2b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a02,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a03,0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a04,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a14,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a15,0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a16,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a00,0x78);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a08,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a09,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0a,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0b,0xa0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0d,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0e,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5193,0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589a,0xc5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4001,0x42);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x401c,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528a,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528c,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528d,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528e,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x528f,0x28);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5290,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5292,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5293,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5294,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5295,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5296,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5297,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5298,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5299,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529b,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529d,0x28);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x529f,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5282,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5300,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5301,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5302,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5303,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530d,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530e,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x530f,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5310,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5311,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5308,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5309,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5304,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5305,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5306,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5307,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5314,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5315,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5319,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5316,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5317,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5318,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5380,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5381,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5382,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5383,0x4e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5384,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5385,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5386,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5387,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5388,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5389,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538a,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538b,0x31);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538d,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x538f,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5390,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5391,0xab);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5392,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5393,0xa2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5394,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5480,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5481,0x21);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5482,0x36);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5483,0x57);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5484,0x65);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5485,0x71);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5486,0x7d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5487,0x87);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5488,0x91);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5489,0x9a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548a,0xaa);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548b,0xb8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548c,0xcd);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548d,0xdd);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548e,0xea);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x548f,0x1d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5490,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5491,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5492,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5493,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5494,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5495,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5496,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5497,0xb8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5498,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5499,0x86);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549a,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549b,0x5b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549c,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549d,0x3b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549e,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x549f,0x1c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a0,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a1,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a2,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a3,0xed);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a4,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a5,0xc5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a6,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a7,0xa5);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a8,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54a9,0x6c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54aa,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ab,0x41);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ac,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ad,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54ae,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54af,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b0,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b1,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b2,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b3,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b4,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b5,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b6,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x54b7,0xdf);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5402,0x3f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5403,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3406,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5180,0xff);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5181,0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5182,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5183,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5184,0x25);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5185,0x24);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5186,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5187,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5188,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5189,0x7c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518a,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518b,0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518c,0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518d,0x44);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518e,0x3d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x518f,0x58);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5190,0x46);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5191,0xf8);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5192,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5193,0x70);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5194,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5195,0xf0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5196,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5197,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5198,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5199,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519a,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519c,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519d,0x82);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5025,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a0f,0x38);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a10,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1b,0x3a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1e,0x2e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a11,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a1f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5688,0xa6);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5689,0x6a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568a,0xea);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568b,0xae);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568c,0xa6);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568d,0x6a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568e,0x62);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x568f,0x26);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5583,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5584,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5580,0x02);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5000,0xcf);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5800,0x27);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5801,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5802,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5803,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5804,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5805,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5806,0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5807,0x2f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5808,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5809,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580a,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580b,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580c,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580d,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580e,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x580f,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5810,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5811,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5812,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5813,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5814,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5815,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5816,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5817,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5818,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5819,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581a,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581c,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581d,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581e,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x581f,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5820,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5821,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5822,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5823,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5824,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5825,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5826,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5827,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5828,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5829,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582a,0x06);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582b,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582c,0x05);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582d,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582e,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x582f,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5830,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5831,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5832,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5833,0x0a);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5834,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5835,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5836,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5837,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5838,0x32);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5839,0x1f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583a,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583b,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583c,0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583d,0x1e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583e,0x26);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x583f,0x53);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5840,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5841,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5842,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5843,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5844,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5845,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5846,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5847,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5848,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5849,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584a,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584b,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584c,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584d,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584e,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x584f,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5850,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5851,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5852,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5853,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5854,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5855,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5856,0x0e);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5857,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5858,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5859,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585a,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585b,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585c,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585d,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585e,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x585f,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5860,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5861,0x0c);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5862,0x0d);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5863,0x08);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5864,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5865,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5866,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5867,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5868,0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5869,0x19);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586a,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586b,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586c,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586d,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586e,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x586f,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5870,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5871,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5872,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5873,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5874,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5875,0x16);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5876,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5877,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5878,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5879,0x0f);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587a,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587b,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587c,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587d,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587e,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x587f,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5880,0x12);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5881,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5882,0x14);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5883,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5884,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5885,0x15);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5886,0x13);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5887,0x17);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3710,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3632,0x51);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3702,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3703,0xb2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3704,0x18);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370b,0x40);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370d,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3631,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3632,0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x24);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3620,0x96);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5785,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3a13,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3600,0x52);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x48);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3606,0x1b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370d,0x0b);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370f,0xc0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3709,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3823,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5007,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5009,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5011,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5013,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x519e,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5086,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5087,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5088,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5089,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x302b,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0x87);//


I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3808,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3809,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380a,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380b,0x00);//

}


DWORD ov_Sensor_GetAvgLuminance(void)
{
		SDWORD value=0;
		value=I2C_Read_Addr16_Value8_LoopMode(0x78>>1,  0x5690);
		return (DWORD)value;
}





void static OV_CaptureKey_VGA_640x480(void)
{
    MP_ALERT("-- %s --", __FUNCTION__);
 											
															
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3406, 0x01);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3003, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3005, 0xFF);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3006, 0xFF);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3007, 0x3F);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3011, 0x10);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3012, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x350C, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x350D, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3602, 0xE4);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3612, 0xAC);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3613, 0x44);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3621, 0x09);//29
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3622, 0x60);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3623, 0x22);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3604, 0x60);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3705, 0xDA);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x370A, 0x80);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x370D, 0x03);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3801, 0x8A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3803, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3804, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3805, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3806, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3807, 0x98);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3808, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3809, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380A, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380B, 0x98);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380C, 0x0C);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380D, 0x80);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380E, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380F, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3824, 0x11);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3825, 0xAC);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3827, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A08, 0x09);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A09, 0x60);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A0A, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A0B, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A0D, 0x10);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A0E, 0x0D);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A1A, 0x04);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x460B, 0x35);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x471D, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x4713, 0x03);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5001, 0xFF);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x589B, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x589A, 0xC0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x4407, 0x04);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x589B, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x589A, 0xC0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3002, 0x1C);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x460C, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x471C, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x4721, 0x01);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3815, 0x01);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x501F, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5002, 0xE0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x4300, 0x30);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3818, 0xc0);//a0
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3810, 0xC2);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3010, 0x70);
	//; 													
	//;SXGA 												
	//														
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3800, 0x1 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3801, 0x8A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3802, 0x0 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3803, 0xA );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3804, 0xA );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3805, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3806, 0x7 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3807, 0x98);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3808, 0x02 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3809, 0x80 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380A, 0x01 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380B, 0xE0 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380C, 0xC );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380D, 0x80);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380E, 0x7 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380F, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5001, 0x7F);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5680, 0x0 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5681, 0x0 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5682, 0xA );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5683, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5684, 0x0 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5685, 0x0 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5686, 0x7 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5687, 0x98);

}




void static OV_CaptureKey_XGA_1024x768(void)
{
    MP_ALERT("-- %s --", __FUNCTION__);
 											
															
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3406, 0x01);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3003, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3005, 0xFF);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3006, 0xFF);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3007, 0x3F);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3011, 0x10);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3012, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x350C, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x350D, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3602, 0xE4);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3612, 0xAC);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3613, 0x44);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3621, 0x09);//29
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3622, 0x60);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3623, 0x22);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3604, 0x60);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3705, 0xDA);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x370A, 0x80);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x370D, 0x03);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3801, 0x8A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3803, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3804, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3805, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3806, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3807, 0x98);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3808, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3809, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380A, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380B, 0x98);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380C, 0x0C);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380D, 0x80);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380E, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380F, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3824, 0x11);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3825, 0xAC);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3827, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A08, 0x09);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A09, 0x60);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A0A, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A0B, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A0D, 0x10);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A0E, 0x0D);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A1A, 0x04);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x460B, 0x35);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x471D, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x4713, 0x03);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5001, 0xFF);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x589B, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x589A, 0xC0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x4407, 0x04);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x589B, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x589A, 0xC0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3002, 0x1C);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x460C, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x471C, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x4721, 0x01);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3815, 0x01);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x501F, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5002, 0xE0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x4300, 0x30);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3818, 0xc0);//a0
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3810, 0xC2);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3010, 0x70);
	//; 													
	//;SXGA 												
	//														
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3800, 0x1 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3801, 0x8A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3802, 0x0 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3803, 0xA );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3804, 0xA );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3805, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3806, 0x7 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3807, 0x98);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3808, 0x04 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3809, 0x00 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380A, 0x03 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380B, 0x00 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380C, 0xC );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380D, 0x80);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380E, 0x7 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380F, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5001, 0x7F);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5680, 0x0 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5681, 0x0 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5682, 0xA );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5683, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5684, 0x0 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5685, 0x0 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5686, 0x7 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5687, 0x98);

}


void static OV_CaptureKey_SXGA_1280x1024(void)
{
    MP_ALERT("-- %s --", __FUNCTION__);
	//@@  5M to SXGA Key  SXGA: ก@1,280x1,024				
	//100 99 1280 960										
	//100 98 6 6											
															
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3406, 0x01);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3003, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3005, 0xFF);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3006, 0xFF);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3007, 0x3F);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3011, 0x10);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3012, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x350C, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x350D, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3602, 0xE4);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3612, 0xAC);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3613, 0x44);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3621, 0x09);//29
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3622, 0x60);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3623, 0x22);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3604, 0x60);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3705, 0xDA);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x370A, 0x80);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x370D, 0x03);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3801, 0x8A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3803, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3804, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3805, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3806, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3807, 0x98);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3808, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3809, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380A, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380B, 0x98);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380C, 0x0C);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380D, 0x80);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380E, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380F, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3824, 0x11);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3825, 0xAC);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3827, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A08, 0x09);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A09, 0x60);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A0A, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A0B, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A0D, 0x10);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A0E, 0x0D);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A1A, 0x04);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x460B, 0x35);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x471D, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x4713, 0x03);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5001, 0xFF);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x589B, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x589A, 0xC0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x4407, 0x04);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x589B, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x589A, 0xC0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3002, 0x1C);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x460C, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x471C, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x4721, 0x01);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3815, 0x01);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x501F, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5002, 0xE0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x4300, 0x30);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3818, 0xc0);//a0
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3810, 0xC2);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3010, 0x70);
	//; 													
	//;SXGA 												
	//														
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3800, 0x1 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3801, 0x8A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3802, 0x0 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3803, 0xA );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3804, 0xA );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3805, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3806, 0x7 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3807, 0x98);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3808, 0x5 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3809, 0x0 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380A, 0x4 );//3//
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380B, 0x0);//c
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380C, 0xC );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380D, 0x80);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380E, 0x7 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380F, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5001, 0x7F);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5680, 0x0 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5681, 0x0 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5682, 0xA );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5683, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5684, 0x0 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5685, 0x0 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5686, 0x7 );
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5687, 0x98);

}



void static OV_CaptureKey_UXGA_1600x1200(void)
{

MP_ALERT("-- %s --", __FUNCTION__);
//	@@ 5M to 2M Key UXGA: ก@1,600x1,200
//100 99 1600 1200
//100 98 6 6

I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3406,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3003,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3005,0xFF);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3006,0xFF);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3007,0x3F);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3011,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3012,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350C,0x07);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350D,0xD0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3602,0xE4);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3612,0xAC);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3613,0x44);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0x09);//29
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3622,0x60);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3623,0x22);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x60);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3705,0xDA);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370A,0x80);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370D,0x03);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3801,0x8A);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3803,0x0A);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3804,0x0A);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3805,0x20);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3806,0x07);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3807,0x98);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3808,0x0A);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3809,0x20);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380A,0x07);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380B,0x98);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380C,0x0C);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380D,0x80);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380E,0x07);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380F,0xD0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3824,0x11);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3825,0xAC);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3827,0x0A);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A08,0x09);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A09,0x60);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A0A,0x07);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A0B,0xD0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A0D,0x10);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A0E,0x0D);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A1A,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460B,0x35);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471D,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4713,0x03);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5001,0xFF);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589B,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589A,0xC0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4407,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589B,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589A,0xC0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3002,0x1C);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460C,0x20);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471C,0xD0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4721,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3815,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x501F,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5002,0xE0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4300,0x30);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3818,0xc0);//80
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3810,0xC2);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3010,0x70);


//;
//;UXGA
//;
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3800,0x01);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3801,0x8A);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3802,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3803,0x0A);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3804,0x0A);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3805,0x20);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3806,0x07);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3807,0x98);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3808,0x06);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3809,0x40);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380A,0x04);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380B,0xB0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380C,0x0C);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380D,0x80);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380E,0x07);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380F,0xD0);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5001,0x7F);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5680,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5681,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5682,0x0A);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5683,0x20);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5684,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5685,0x00);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5686,0x07);
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5687,0x98);

}



void OV_CaptureKey_QXGA_2048x1536(void)
{
	MP_ALERT("-----%s", __FUNCTION__);

	
	//@@ 5M to QXGA Key  QXGA 2048กั1536						
	//100 99 2048 1536										
	//100 98 6 6												
															
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3406, 0x01);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3003, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3005, 0xFF);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3006, 0xFF);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3007, 0x3F);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3011, 0x10);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3012, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x350C, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x350D, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3602, 0xE4);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3612, 0xAC);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3613, 0x44);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3621, 0x09);//29
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3622, 0x60);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3623, 0x22);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3604, 0x60);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3705, 0xDA);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x370A, 0x80);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x370D, 0x03);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3801, 0x8A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3803, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3804, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3805, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3806, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3807, 0x98);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3808, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3809, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380A, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380B, 0x98);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380C, 0x0C);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380D, 0x80);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380E, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380F, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3824, 0x11);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3825, 0xAC);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3827, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A08, 0x09);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A09, 0x60);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A0A, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A0B, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A0D, 0x10);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A0E, 0x0D);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3A1A, 0x04);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x460B, 0x35);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x471D, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x4713, 0x03);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5001, 0xFF);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x589B, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x589A, 0xC0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x4407, 0x04);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x589B, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x589A, 0xC0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3002, 0x1C);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x460C, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x471C, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x4721, 0x01);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3815, 0x01);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x501F, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5002, 0xE0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x4300, 0x30);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3818, 0xc0);//80
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3810, 0xC2);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3010, 0x70);
															
	//;QXGA													
															
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3800, 0x01);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3801, 0x8A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3802, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3803, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3804, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3805, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3806, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3807, 0x98);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3808, 0x08);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3809, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380A, 0x06);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380B, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380C, 0x0C);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380D, 0x80);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380E, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380F, 0xD0);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5001, 0x7F);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5680, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5681, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5682, 0x0A);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5683, 0x20);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5684, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5685, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5686, 0x07);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x5687, 0x98);

    
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3808, 0x08);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x3809, 0x00);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380A, 0x06);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1, 0x380B, 0x00);


	
}



static void OV_CaptureKey_5M_2560x1920(void)
{
	MP_ALERT("-----%s", __FUNCTION__);

//OV5642_5M key_YUV_3.75fps_24MHz_20101027ฑH.txt
//@@ 5M Key
//100 99 2592 1944
//100 98 6 6

I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3406,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3003,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3005,0xFF);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3006,0xFF);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3007,0x3F);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3011,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3012,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350C,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350D,0xD0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3602,0xE4);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3612,0xAC);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3613,0x44);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3621,0x09);//;29	;Mirror & Flip
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3622,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3623,0x22);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3604,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3705,0xDA);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370A,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x370D,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3801,0x8A);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3803,0x0A);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3804,0x0A);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3805,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3806,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3807,0x98);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3808,0x0A);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3809,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380A,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380B,0x98);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380C,0x0C);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380D,0x80);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380E,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x380F,0xD0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3824,0x11);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3825,0xAC);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3827,0x0A);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A08,0x09);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A09,0x60);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A0A,0x07);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A0B,0xD0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A0D,0x10);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A0E,0x0D);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3A1A,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460B,0x35);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471D,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4713,0x03);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5001,0xFF);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589B,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589A,0xC0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4407,0x04);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589B,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x589A,0xC0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3002,0x1C);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x460C,0x20);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x471C,0xD0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4721,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3815,0x01);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x501F,0x00);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x5002,0xE0);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x4300,0x30);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3818,0xc0);//;A0	;Mirror&Flip
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3810,0xC2);//
I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3010,0x70);//

	
}




//frank lin capture flow
void OV_Capture_KeyStting(BYTE IMGSIZE_ID)//(BYTE IMGSIZE_ID)
{

  switch(IMGSIZE_ID)
  {
  	case SIZE_VGA_640x480:
         OV_CaptureKey_VGA_640x480();		
		break;
		
    case SIZE_XGA_1024x768:
		OV_CaptureKey_XGA_1024x768();
		break;
	
    case SIZE_SXGA_1280x1024:
		OV_CaptureKey_SXGA_1280x1024();
		break;
	
  	case SIZE_2M_1600x1200:
		OV_CaptureKey_UXGA_1600x1200();
		break;

	case SIZE_QXGA_2048x1536:
		OV_CaptureKey_QXGA_2048x1536();
		break;

	case SIZE_5M_2560x1920:
		OV_CaptureKey_5M_2560x1920();
		break;

	default:
		MP_ALERT("%s: Not support capture size.", __FUNCTION__);
  }
	
}


void OV_Capture_preview(void)
{
	ov_setVGA_640x480();
}



int Sensor_Capture_OV5642(STREAM *fileHandle, BYTE Capture_IMGSIZE_ID)
{
	MP_DEBUG1("###### %s ######", __FUNCTION__);
	int iErrorCode;
	extern int SensorInputWidth;
    extern int SensorInputHeight;

	//---- OV caputure flow ----
	int Capture_Framerate = 3000;//depend on  Capture setting
	int Lines_10ms;
	int Capture_MaxLines;
	int Preview_FrameRate = 3000;////depend on  Preview setting
	int Preview_Maxlines;
	
	DWORD ulCapture_Exposure;
	DWORD ulCapture_Exposure_Gain;
	DWORD ulPreviewExposure;
	DWORD iCapture_Gain;

	BYTE Gain;
	BYTE ExposureLow;
	BYTE ExposureMid;
	BYTE ExposureHigh;
	BYTE PreviewMaxlineHigh;
	BYTE PreviewMaxlineLow;
	
	BYTE CaptureMaxlineHigh;
	BYTE CaptureMaxlineLow;
	
	DWORD GetValue;

	extern int CaptureCloseDisplay_Flag;

	IPU *ipu = (IPU *) IPU_BASE;

	
	//stop AEC/AGC 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x07);
	
	//read the register 0x350*
	GetValue = I2C_Read_Addr16_Value8_LoopMode(0x78>>1, 0x350b);
	Gain = (BYTE) (GetValue & 0xff);
	
	GetValue = I2C_Read_Addr16_Value8_LoopMode(0x78>>1, 0x3502);
	ExposureLow = (BYTE) (GetValue & 0xff);
	
	GetValue = I2C_Read_Addr16_Value8_LoopMode(0x78>>1, 0x3501);
	ExposureMid = (BYTE) (GetValue & 0xff);
	
	GetValue = I2C_Read_Addr16_Value8_LoopMode(0x78>>1, 0x3500);
	ExposureHigh = (BYTE) (GetValue & 0xff);
	   
	GetValue = I2C_Read_Addr16_Value8_LoopMode(0x78>>1, 0x350c);
	PreviewMaxlineHigh = (BYTE) (GetValue & 0xff);
	
	GetValue = I2C_Read_Addr16_Value8_LoopMode(0x78>>1, 0x350d);
	PreviewMaxlineLow = (BYTE) (GetValue & 0xff);

	switch(Capture_IMGSIZE_ID)
	{
		case SIZE_5M_2560x1920:
	   		sensor_IMGSIZE_ID = SIZE_5M_2560x1920;//SIZE_2M_1600x1200;
	   		SensorInputWidth  = 2560;
	   		SensorInputHeight = 1920;
	   		break;

		case SIZE_QXGA_2048x1536:
			sensor_IMGSIZE_ID  = SIZE_QXGA_2048x1536;
	   		SensorInputWidth   = 2048;
	   		SensorInputHeight  = 1536;
			break;

		case SIZE_2M_1600x1200:
	   		sensor_IMGSIZE_ID = SIZE_2M_1600x1200;
	   		SensorInputWidth  = 1600;
	   		SensorInputHeight = 1200;
	   		break;

		case SIZE_SXGA_1280x1024:
		   	sensor_IMGSIZE_ID = SIZE_SXGA_1280x1024;
	   		SensorInputWidth  = 1280;
	   		SensorInputHeight = 1024;
	   		break;

       	case SIZE_XGA_1024x768:
	   		sensor_IMGSIZE_ID = SIZE_XGA_1024x768;
	   		SensorInputWidth  = 1024;
	   		SensorInputHeight = 768;
	   		break;

	    case SIZE_VGA_640x480:
	   		sensor_IMGSIZE_ID = SIZE_VGA_640x480;
	   		SensorInputWidth  = 640;
	   		SensorInputHeight = 480;
	   		break;
	   
		default:
			MP_ALERT("%s:Not Support capture IMGSIZE. ID=%d", __FUNCTION__,Capture_IMGSIZE_ID);
			goto Error;
	}

    
    /*Before change to big resolution need to free the older memory*/
	/*If Usign double buffer need to add free memory here.*/
    extern ST_IMGWIN SensorInWin[3];
	ipu->Ipu_reg_100 &= ~BIT0; //close SenInEn
	while(1)
	{
		if((ipu->Ipu_reg_100&BIT0)==0)//ฝTฉwฆณผgถiฅh
			break;
	}

    if(sensor_mode==MODE_CAPTURE)
	{
		if(g_bImageData_Buf != NULL)
		{
			ext_mem_free(g_bImageData_Buf);
			g_bImageData_Buf        = NULL;
			SensorInWin[0].pdwStart = NULL;
		}
    }
	else
	{
		MP_ALERT("%s:Error Not caputre mode (sensor_mode =%d)", __FUNCTION__, sensor_mode);
		goto Error;
	}

	
    /*During capture , disable the display path*/	
	CaptureCloseDisplay_Flag = 1;
		Set_Display_flag(DISABLE);/*close IPW2*/

		TimerDelay(1000);/*delay time for auto AE adjust */

		//change resolution from VGA to ....
		OV_Capture_KeyStting(sensor_IMGSIZE_ID);//input ImageSize

	   	Sensor_Run_capture();
	   	Global_Sensor_Run();
		
	CaptureCloseDisplay_Flag = 0;


    //stop AEC/AGC 
	//I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x07);
	
	
	//read 0x350c 0x350d   
	GetValue = I2C_Read_Addr16_Value8_LoopMode(0x78>>1, 0x350c);
	CaptureMaxlineLow = (BYTE) (GetValue & 0xff);
	   
	GetValue = I2C_Read_Addr16_Value8_LoopMode(0x78>>1, 0x350d);
	CaptureMaxlineLow = (BYTE) (GetValue & 0xff);
	
	Preview_Maxlines = 256*PreviewMaxlineHigh + PreviewMaxlineLow;
	Capture_MaxLines = 256*CaptureMaxlineHigh + CaptureMaxlineLow;
	
	if(1)//(m_60Hz==true)
	{
	 Lines_10ms = Capture_Framerate*Capture_MaxLines/12000;
	}
	else
	{
	 Lines_10ms = Capture_Framerate * Capture_MaxLines/10000;
	}

	//Lines_10ms *= 6;//franklin test
	ulPreviewExposure = ((DWORD)(ExposureHigh))<<12 ;
	ulPreviewExposure += ((DWORD)ExposureMid)<<4 ;
	ulPreviewExposure += (ExposureLow >>4);
	
	if(0 == Preview_Maxlines ||0 ==Preview_FrameRate ||0== Lines_10ms)
	{
	  goto Error;
	}
	
    ulCapture_Exposure =(ulPreviewExposure*(Capture_Framerate)*(Capture_MaxLines))/(((Preview_Maxlines)*(Preview_FrameRate)));
	iCapture_Gain = (Gain & 0x0f) + 16;
	if (Gain & 0x10)
	{
	  iCapture_Gain = iCapture_Gain << 1;
	}
	if (Gain & 0x20)
	{
	  iCapture_Gain = iCapture_Gain << 1;
	}
	if (Gain & 0x40)
	{
	  iCapture_Gain = iCapture_Gain << 1;
	}
	if (Gain & 0x80)
	{
	  iCapture_Gain = iCapture_Gain << 1;
	}
	ulCapture_Exposure_Gain = ulCapture_Exposure * iCapture_Gain;
	if(ulCapture_Exposure_Gain < ((LONG)(Capture_MaxLines)*16))
	{
	  ulCapture_Exposure = ulCapture_Exposure_Gain/16;
	  if (ulCapture_Exposure > Lines_10ms)
	  {
		ulCapture_Exposure /= Lines_10ms;
		ulCapture_Exposure *= Lines_10ms;
	  }
	}
	else
	{
		ulCapture_Exposure = Capture_MaxLines;
	} 
	
	if(ulCapture_Exposure == 0)
	{
	  ulCapture_Exposure = 1;
	}
	iCapture_Gain = (ulCapture_Exposure_Gain*2/ulCapture_Exposure + 1)/2;
	
    ExposureLow = ((BYTE)ulCapture_Exposure)<<4;
	ExposureMid = (BYTE)(ulCapture_Exposure >> 4) & 0xff;
	ExposureHigh = (BYTE)(ulCapture_Exposure >> 12);
	  
	#if 0 
	  ulCapture_Exposure_end=ulCapture_Exposure;
	  iCapture_Gain_end=iCapture_Gain;
	#endif 
    MP_DEBUG1("-----### ulCapture_Exposure=%d",ulCapture_Exposure);
    MP_DEBUG1("-----### iCapture_Gain=%d", iCapture_Gain);
	
	Gain = 0;
	if (iCapture_Gain > 31)
	{
	  Gain |= 0x10;
	  iCapture_Gain = iCapture_Gain >> 1;
	}
	if (iCapture_Gain > 31)
	{
	  Gain |= 0x20;
	  iCapture_Gain = iCapture_Gain >> 1;
	}
	if (iCapture_Gain > 31)
	{
	  Gain |= 0x40;
      iCapture_Gain = iCapture_Gain >> 1;
	}
	if (iCapture_Gain > 31)
	{
	  Gain |= 0x80;
	  iCapture_Gain = iCapture_Gain >> 1;
	}
	if (iCapture_Gain > 16)
	{
	  Gain |= ((iCapture_Gain -16) & 0x0f);
	}
	if(Gain==0x10)
	 {Gain=0x11;}
	
	//write the gain and exposoure to 0x350*
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x350b,Gain);  
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3502,ExposureLow);
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3501,ExposureMid);  
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3500,ExposureHigh);
	  
    //---- OV caputure flow End---- 

	iErrorCode = Sensor_Capture_Encode(fileHandle, SensorInputWidth, SensorInputHeight,sensor_IMGSIZE_ID);	
    /*After here the big memory for capture. the memory is free after Sensor_Capture_Encode*/

	if(iErrorCode == PASS)
	{
        MP_ALERT("%s:Capture end!! (Successful)", __FUNCTION__);
	}
	else
	{
		MP_ALERT("%s:Capture end!! (Fail)", __FUNCTION__);
	}
	
    sensor_IMGSIZE_ID = SIZE_VGA_640x480;
	SensorInputWidth  = 640;
	SensorInputHeight = 480;

	//start AEC/AGC 
	I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3503,0x00);

	
	//----Back to preview mode  ------
	OV_Capture_preview();
	Set_Display_flag(ENABLE);/*Open IPW2*/
	Sensor_Run();
	Global_Sensor_Run();
	
	ipu->Ipu_reg_100 |= BIT0; //SenInEn

	//----Back to preview mode  ------
	

	return iErrorCode;

Error:
	//API_Sensor_Stop();
	/*When exit caputer mod, need to call API_Sensor_Stop() to free preview memoy */
	return FAIL;
	
}





void Drive_Sensor_OV5642(void)
{

		I2CM_FreqChg(0x78, 300000);
		/*
		SDWORD value=0;
		while(1)
		{
				//I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x3103,0x93);
				value=I2C_Read_Addr16_Value8_LoopMode(0x78>>1,  0x300B);
				mpDebugPrint("-------------value=0x%X",value);
		IODelay(50);
				//IODelay(4000);
		}*/
/**/
		if(sensor_mode==MODE_RECORD)
		{
				if(sensor_IMGSIZE_ID==SIZE_CIF_352x240)
				{
						ov_set_352x240_30FPS();
				}else	if(sensor_IMGSIZE_ID==SIZE_VGA_640x480)
				{
						ov_setVGA_640x480_30FPS();
				}else if(sensor_IMGSIZE_ID==SIZE_480P_720x480)
				{
						ov_set_720x480_30FPS();
				}else if(sensor_IMGSIZE_ID==SIZE_XGA_1024x768)
				{
						ov_setXGA_1024x768_30FPS();
				}else{//default 720p
						ov_set_720P_1280x720();
				}


		}else if(sensor_mode==MODE_CAPTURE)
		{
            //frank lin capture flow
            MP_ALERT("########  capture preview mode (VGA)------"); 
			OV_Capture_preview();

		}else if(sensor_mode==MODE_PCCAM)
		{
				if(sensor_IMGSIZE_ID==SIZE_QVGA_320x240)
						ov_setQVGA_320x240_30FPS();
				else
						ov_set_176x144();
		}else
		{
		ov_set_720P_1280x720();
		}


//I2C_Write_Addr16_Value8_LoopMode(0x78>>1,0x503D,0x80);//sensor color bar
		

}
#endif

#endif



