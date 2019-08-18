/*
*******************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based 
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2004, Magic Pixel Inc, Hsinchu, Taiwan
*
* All rights reserved. Magic Pixel's source code is an unpublished work and the 
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in 
* deciphering, decoding, reverse engineering or in ay way altering the source 
* code is strictly prohibited, unless the prior written consent of Magic 
* Pixel is obtained.
* 
* 
* Filename		: usbotg_sensorcfg.c
* Programmer(s)	: Morse
* Created Date	: 2010/05/06 
* Description   : USB Device UVC/UAC Class sensor interface
******************************************************************************** 
*/
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#include "typedef.h"
#include "usbotg_device_vac.h"

#if (SC_USBDEVICE && WEB_CAM_DEVICE)

/////////////////////////////////////////////////////////////////////////////
typedef enum
{
    IDU_GRAPHIC_WINDOW  = 0,
    IDU_IMAGE_WINDOW    = 1,
}ENUM_IDU_WINDOW_TYPE;

/////////////////////////////////////////////////////////////////////////////


#define API_VIDEO_PREVIEW_START                  0x00000001
#define API_VIDEO_PREVIEW_STOP                   0x00000002
#define API_VIDEO_RECORD_START                   0x00000003
#define API_VIDEO_RECORD_STOP                    0x00000004
#define API_VIDEO_CLIP_OPEN                      0x00000005
#define API_VIDEO_CLIP_CLOSE                     0x00000006
#define API_VIDEO_PLAY_START                     0x00000007
#define API_VIDEO_PLAY_STOP                      0x00000008
#define API_VIDEO_PROGRESS_SET                   0x00000009
#define API_VIDEO_PROGRESS_GET                   0x0000000a
#define API_VIDEO_FORWARD_SET                    0x0000000b
#define API_VIDEO_IMAGE_WINDOW_REDRAW            0x0000000c
#define API_VIDEO_DIGITAL_ZOOM_SET               0x0000000d
#define API_VIDEO_RECORD_PAUSE                   0x0000000e
#define API_VIDEO_RECORD_RESUME                  0x0000000f
#define API_VIDEO_CON_PREVIEW_START              0x00000010
#define API_VIDEO_CON_PREVIEW_STOP               0x00000011
#define API_VIDEO_CON_TRANSMISSION_START         0x00000012
#define API_VIDEO_CON_TRANSMISSION_STOP          0x00000013
#define API_VIDEO_CON_VISUAL_CODEC_INIT          0x00000014
#define API_VIDEO_CON_VISUAL_ENCODE_TYPE_SET     0x00000015
#define API_VIDEO_CON_VISUAL_DECODE_TYPE_SET     0x00000016
#define API_VIDEO_CON_VISUAL_ENCODE_PARAM_CHANGE 0x00000017
#define API_VIDEO_CON_VISUAL_CODEC_VOL_SET       0x00000018
#define API_VIDEO_CON_VISUAL_STREAM_FILL         0x00000019
#define API_VIDEO_CON_VISUAL_BUFFER_MEASURE      0x0000001a
#define API_VIDEO_CON_VISUAL_STREAM_DECODE       0x0000001b
#define API_VIDEO_CON_SOUND_ENCODE_TYPE_SET      0x0000001c
#define API_VIDEO_PROGRESS_FRAME_BY_FRAME        0x0000001d
#define API_VIDEO_QUICK_CLIP_OPEN                0x00000020
#define API_VIDEO_VISUAL_FRAME_SET               0x00000021
#define API_VIDEO_VISUAL_FRAME_APPLY             0x00000022
#define API_VIDEO_TIMESTAMP_FONT_SET             0x00000023
#define API_VIDEO_TIMESTAMP_FONT_CLEAN           0x00000024
#define API_VIDEO_TIMESTAMP_REGION_SET           0x00000025
#define API_VIDEO_TIMESTAMP_ENABLE               0x00000026

#define AUDIO_SAMPLERATE_8K     0 // AUDIO_RECORD_8K_SR
#define AUDIO_SAMPLERATE_16K    1 // AUDIO_RECORD_16K_SR
#define AUDIO_SAMPLERATE_24K    2 // AUDIO_RECORD_24K_SR

#define ERR_VIDEO_BASE                          0x80040000
#define ERR_VIDEO_INVALID_PARAMETER             (9 + ERR_VIDEO_BASE)


typedef struct
{
    U08 Byte0;
    U08 Byte1;
    U08 Byte2;
    U08 Byte3;
} ST_BYTE_STRC;

U16 UtilByteSwap16(void* in)
{
    register U16 out;

    out = ((ST_BYTE_STRC*)in)->Byte1;
    out = out << 8;
    out += ((ST_BYTE_STRC*)in)->Byte0;

    return out;
}

S32 mpx_AudioCodecVolumnSet(U32 path, U32 vol)
{
#if REMOVE
    U08 regaddr;
    U32 sum;
    regaddr=0x14;
    if ( vol > MAX_VOLUMN_VALUE )
        vol = MAX_VOLUMN_VALUE;
	
	vSPI_Write(regaddr, (U08)4);   // i2s

    if ( (path & SPEAKER_OUTPUT) || (path & HEADPHONE_OUTPUT) )
    {
    	  sum = (vol * VOLUMN_OUTPUT_STEPS) / MAX_VOLUMN_VALUE + VOLUMN_OUTPUT_STEP_BASE;
    	  PCMInitDACTab[0] = (U08) sum;
        //PCMInitDACTab[0] = PCMInitDACTab[1] = (U08)(vol * VOLUMN_OUTPUT_STEPS / MAX_VOLUMN_VALUE + VOLUMN_OUTPUT_STEP_BASE);
        // Adjust speakers volume
            regaddr=0x10;
       vSPI_Write(regaddr, (U08)PCMInitDACTab[0]);//L channel
       regaddr=0x11;
       vSPI_Write(regaddr, (U08)PCMInitDACTab[1]);//R channel
       
       DPrintf("PCM1753 volume SET [ R16 0x%x, R17 0x%x ]", (U08)PCMInitDACTab[0], (U08)PCMInitDACTab[1]);
//        vSPI_Write(0x10, 10);//R channel
//        vSPI_Write(0x11, 10);//R channel
    }

    return NO_ERR;
#endif
}

S32 UsbdPipeStallSet(U08 pipenum)
{
    //REMOVE if(stUsbdDriver.UsbdEpStallSet)
    {
       //REMOVE stUsbdDriver.UsbdEpStallSet(stPipeInfo.stPipes[pipenum].u08EpNum); 
       //REMOVE stPipeInfo.stPipes[pipenum].u08Stalled = 1;       
    }
    //REMOVE else
        //REMOVE DPrintf("want to set stall, but driver not provide");
    return 0;
}

void SsSysCfgBrightnessSet(U08 u08Value)
{
    //REMOVE  stSystemConfig.u08Brightness = u08Value;
}

void SsSysCfgContrastSet(U08 u08Value)
{
    //REMOVE  stSystemConfig.u08Contrast = u08Value;
}

U08 SsSysCfgBrightnessGet(void)
{
    //REMOVE  return stSystemConfig.u08Brightness;
}

U08 SsSysCfgContrastGet(void)
{
    //REMOVE  return stSystemConfig.u08Contrast;
}



///
///@ingroup hal_IPU
///@brief   IpuSetBrightnessLevelInf
///
void IpuSetBrightnessLevelInf(U08 Brightness)
{
#ifdef CHIP_TYPE_MP322
    REG_POINT(regIpuPtr, IPU);

    SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_PROC_ORDER, 0);
    SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_EN, 1);
    SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_CC_EN, 0);
    SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_GAIN_EN, 0);

    switch (Brightness)
    {
    case 1: // bright -32
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 480);
        break;
    case 2: // bright -24
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 488);
        break;
    case 3: // bright -16
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 496);
        break;
    case 4: // bright -8
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 504);
        break;
    case 5: // bright 0
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 0);
        break;
    case 6: // bright 8
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 8);
        break;
    case 7: // bright 16
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 16);
        break;
    case 8: // bright 24
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 24);
        break;
    case 9: // bright 32
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 32);
        break;
    case 10:// bright 40
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 40);
        break;
    }
#endif
}



///
///@ingroup hal_IPU
///@brief   IpuSetContrastLevelInf
///
void IpuSetContrastLevelInf(U08 contrast)
{
    //REMOVE REG_POINT(regIpuPtr, IPU);

#ifdef REMOVE//CHIP_TYPE_MP321D
    regIpuPtr->IP_YGM = 1;

    switch (contrast)
    {
    case 0: // Gamma = 0.33
        regIpuPtr->IP_YGMT_0 = (0 << 16) | 40;
        regIpuPtr->IP_YGMT_1 = (50 << 16) | 63;
        regIpuPtr->IP_YGMT_2 = (72 << 16) | 80;
        regIpuPtr->IP_YGMT_3 = (91 << 16) | 101;
        regIpuPtr->IP_YGMT_4 = (108 << 16) | 115;
        regIpuPtr->IP_YGMT_5 = (121 << 16) | 128;
        regIpuPtr->IP_YGMT_6 = (130 << 16) | 144;
        regIpuPtr->IP_YGMT_7 = (182 << 16) | 255;
        break;
    case 1: // Gamma = 0.38
        regIpuPtr->IP_YGMT_0 = (0 << 16) | 33;
        regIpuPtr->IP_YGMT_1 = (43 << 16) | 57;
        regIpuPtr->IP_YGMT_2 = (66 << 16) | 74;
        regIpuPtr->IP_YGMT_3 = (87 << 16) | 97;
        regIpuPtr->IP_YGMT_4 = (106 << 16) | 114;
        regIpuPtr->IP_YGMT_5 = (121 << 16) | 128;
        regIpuPtr->IP_YGMT_6 = (131 << 16) | 149;
        regIpuPtr->IP_YGMT_7 = (189 << 16) | 255;
        break;
    case 2: // Gamma = 0.45
        regIpuPtr->IP_YGMT_0 = (0 << 16) | 26;
        regIpuPtr->IP_YGMT_1 = (36 << 16) | 49;
        regIpuPtr->IP_YGMT_2 = (59 << 16) | 67;
        regIpuPtr->IP_YGMT_3 = (81 << 16) | 93;
        regIpuPtr->IP_YGMT_4 = (102 << 16) | 111;
        regIpuPtr->IP_YGMT_5 = (119 << 16) | 128;
        regIpuPtr->IP_YGMT_6 = (134 << 16) | 156;
        regIpuPtr->IP_YGMT_7 = (196 << 16) | 255;
        break;
    case 3: // Gamma = 0.55
        regIpuPtr->IP_YGMT_0 = (0 << 16) | 18;
        regIpuPtr->IP_YGMT_1 = (27 << 16) | 40;
        regIpuPtr->IP_YGMT_2 = (50 << 16) | 59;
        regIpuPtr->IP_YGMT_3 = (73 << 16) | 86;
        regIpuPtr->IP_YGMT_4 = (98 << 16) | 108;
        regIpuPtr->IP_YGMT_5 = (118 << 16) | 128;
        regIpuPtr->IP_YGMT_6 = (138 << 16) | 164;
        regIpuPtr->IP_YGMT_7 = (204 << 16) | 255;
        break;
    case 4: // Gamma = 0.71
        regIpuPtr->IP_YGMT_0 = (0 << 16) | 10;
        regIpuPtr->IP_YGMT_1 = (17 << 16) | 28;
        regIpuPtr->IP_YGMT_2 = (38 << 16) | 47;
        regIpuPtr->IP_YGMT_3 = (63 << 16) | 77;
        regIpuPtr->IP_YGMT_4 = (91 << 16) | 103;
        regIpuPtr->IP_YGMT_5 = (116 << 16) | 128;
        regIpuPtr->IP_YGMT_6 = (146 << 16) | 176;
        regIpuPtr->IP_YGMT_7 = (213 << 16) | 255;
        break;
    case 5: // Gamma = 1.0
        regIpuPtr->IP_YGMT_0 = (0 << 16) | 4;
        regIpuPtr->IP_YGMT_1 = (8 << 16) | 16;
        regIpuPtr->IP_YGMT_2 = (24 << 16) | 32;
        regIpuPtr->IP_YGMT_3 = (48 << 16) | 64;
        regIpuPtr->IP_YGMT_4 = (80 << 16) | 96;
        regIpuPtr->IP_YGMT_5 = (112 << 16) | 128;
        regIpuPtr->IP_YGMT_6 = (160 << 16) | 192;
        regIpuPtr->IP_YGMT_7 = (224 << 16) | 255;
        break;
    case 6: // Gamma = 1.4
        regIpuPtr->IP_YGMT_0 = (0 << 16) | 1;
        regIpuPtr->IP_YGMT_1 = (2 << 16) | 6;
        regIpuPtr->IP_YGMT_2 = (12 << 16) | 18;
        regIpuPtr->IP_YGMT_3 = (32 << 16) | 48;
        regIpuPtr->IP_YGMT_4 = (66 << 16) | 85;
        regIpuPtr->IP_YGMT_5 = (106 << 16) | 128;
        regIpuPtr->IP_YGMT_6 = (175 << 16) | 205;
        regIpuPtr->IP_YGMT_7 = (231 << 16) | 255;
        break;
    case 7: // Gamma = 1.8
        regIpuPtr->IP_YGMT_0 = (0 << 16) | 0;
        regIpuPtr->IP_YGMT_1 = (0 << 16) | 3;
        regIpuPtr->IP_YGMT_2 = (6 << 16) | 10;
        regIpuPtr->IP_YGMT_3 = (22 << 16) | 36;
        regIpuPtr->IP_YGMT_4 = (55 << 16) | 76;
        regIpuPtr->IP_YGMT_5 = (101 << 16) | 128;
        regIpuPtr->IP_YGMT_6 = (187 << 16) | 214;
        regIpuPtr->IP_YGMT_7 = (236 << 16) | 255;
        break;
    case 8: // Gamma = 2.2
        regIpuPtr->IP_YGMT_0 = (0 << 16) | 0;
        regIpuPtr->IP_YGMT_1 = (0 << 16) | 1;
        regIpuPtr->IP_YGMT_2 = (3 << 16) | 6;
        regIpuPtr->IP_YGMT_3 = (14 << 16) | 28;
        regIpuPtr->IP_YGMT_4 = (45 << 16) | 68;
        regIpuPtr->IP_YGMT_5 = (96 << 16) | 128;
        regIpuPtr->IP_YGMT_6 = (195 << 16) | 221;
        regIpuPtr->IP_YGMT_7 = (239 << 16) | 255;
        break;
    case 9: // Gamma = 2.6
        regIpuPtr->IP_YGMT_0 = (0 << 16) | 0;
        regIpuPtr->IP_YGMT_1 = (0 << 16) | 0;
        regIpuPtr->IP_YGMT_2 = (1 << 16) | 3;
        regIpuPtr->IP_YGMT_3 = (10 << 16) | 21;
        regIpuPtr->IP_YGMT_4 = (38 << 16) | 61;
        regIpuPtr->IP_YGMT_5 = (91 << 16) | 128;
        regIpuPtr->IP_YGMT_6 = (202 << 16) | 225;
        regIpuPtr->IP_YGMT_7 = (242 << 16) | 255;
        break;
    case 10:// Gamma = 3.0
    default:
        regIpuPtr->IP_YGMT_0 = (0 << 16) | 0;
        regIpuPtr->IP_YGMT_1 = (0 << 16) | 0;
        regIpuPtr->IP_YGMT_2 = (0 << 16) | 2;
        regIpuPtr->IP_YGMT_3 = (6 << 16) | 16;
        regIpuPtr->IP_YGMT_4 = (31 << 16) | 54;
        regIpuPtr->IP_YGMT_5 = (87 << 16) | 128;
        regIpuPtr->IP_YGMT_6 = (208 << 16) | 229;
        regIpuPtr->IP_YGMT_7 = (243 << 16) | 255;
        break;
    }
#elif REMOVE

    switch (contrast)
    {
    case 0: // gain 0.55, offset 58
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_PROC_ORDER, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_CC_EN, 0);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_GAIN_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL2, YUV_GAIN_Y, 35);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 58);
        break;
    case 1: // gain 0.62, offset 48
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_PROC_ORDER, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_CC_EN, 0);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_GAIN_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL2, YUV_GAIN_Y, 40);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 48);
        break;
    case 2: // gain 0.69, offset 40
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_PROC_ORDER, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_CC_EN, 0);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_GAIN_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL2, YUV_GAIN_Y, 47);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 44);
        break;
    case 3: // gain 0.76, offset 30
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_PROC_ORDER, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_CC_EN, 0);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_GAIN_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL2, YUV_GAIN_Y, 49);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 30);
        break;
    case 4: // gain 0.84, offset 20
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_PROC_ORDER, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_CC_EN, 0);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_GAIN_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL2, YUV_GAIN_Y, 54);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 20);
        break;
    case 5: // offset 0, slope=gain 1x
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_PROC_ORDER, 0);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_CC_EN, 0);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_GAIN_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 0);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL2, YUV_GAIN_Y, 64);
        break;
    case 6: // offset -20, slope=gain 1.18
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_PROC_ORDER, 0);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_CC_EN, 0);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_GAIN_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 492);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL2, YUV_GAIN_Y, 76);
        break;
    case 7: // offset -30, gain 1.31
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_PROC_ORDER, 0);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_CC_EN, 0);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_GAIN_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 482);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL2, YUV_GAIN_Y, 84);
        break;
    case 8: // offset -40, gain 1.46
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_PROC_ORDER, 0);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_CC_EN, 0);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_GAIN_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 472);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL2, YUV_GAIN_Y, 93);
        break;
    case 9: // offset -48, gain 1.60
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_PROC_ORDER, 0);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_CC_EN, 0);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_GAIN_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 464);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL2, YUV_GAIN_Y, 102);
        break;
    case 10:// offset -58, gain 1.83
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_PROC_ORDER, 0);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_CC_EN, 0);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_GAIN_EN, 1);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL0, YUV_OFFSET_Y, 454);
        SET_REG_BITS(regIpuPtr->IP_YUV_CTRL2, YUV_GAIN_Y, 117);
        break;
    default:
        break;
    }
#endif
}





U08 *GetUsbdBuf(U32 len)
{
#if REMOVE
#ifdef NEW_USB_BUF
    U08 i;
    void *buf;

    while(1)
    {
        mpx_SemaphoreWait(UsbdIntBufSemphoreId);
        if(bufnum < MAX_UBUF)
        {
            for(i = 0 ; i < MAX_UBUF; i ++)
            {
                if(!UsbdIntBuf[i])
                {
                    if(BiosGetMemorySize() > 0x1000000) //the usb dma must be using the memory address < 16MB
                    {
                        if((buf = mpx_ZoneMalloc(2, len)) == (void *)0)
                        {
                            mpx_SoleZoneExtend();
                            if((buf = mpx_ZoneMalloc(2, len)) == (void *)0)
                                buf = mpx_ZoneMalloc(3, len);
                        }
                    }
                    else
                        buf = mpx_Malloc(len);
                    if(!buf)    //not enough memory, wait other buf release
                    {
                        DPrintf("Not enough Ubuf");
                        break;
                    }

                    bufnum++;
                    UsbdIntBuf[i] = buf;
                    //DPrintf("bufa = %x, %x", bufnum, UsbdIntBuf[i]);

                    mpx_SemaphoreRelease(UsbdIntBufSemphoreId);
                    return buf;
                }
                
            }
            
        }
        mpx_SemaphoreRelease(UsbdIntBufSemphoreId);
        
        mpx_SemaphoreWait(waitUsbdBufSemaphoreId);
        IntDisable();
        waitUsbdBufTaskId = mpx_TaskIdGet();
        mpx_TaskSleep();
        mpx_SemaphoreRelease(waitUsbdBufSemaphoreId);
    }
#else
    U08 i;
    void *buf;

    mpx_SemaphoreWait(UsbdIntBufSemphoreId);

    for(i = 0 ; i < MAX_UBUF; i ++)
    {
        if(!UsbdIntBuf[i])
        {
            if(BiosGetMemorySize() > 0x1000000) //the usb dma must be using the memory address < 16MB
            {
                if((buf = mpx_ZoneMalloc(2, len)) == (void *)0)
                {
                    mpx_SoleZoneExtend();
                    if((buf = mpx_ZoneMalloc(2, len)) == (void *)0)
                        buf = mpx_ZoneMalloc(3, len);

                    if(buf == (void *)0)
                        BREAK_POINT();
                }
            }
            else
                buf = mpx_Malloc(len);

            UsbdIntBuf[i] = buf;
            bufnum++;
            //DPrintf("bufa = %x, %x", bufnum, UsbdIntBuf[i]);
            return UsbdIntBuf[i];
        }
        
    }
    DPrintf("no avaliable usb buf");
    BREAK_POINT();
    return 0;
#endif    
#elif REMOVE
    static U08 shit = 1;
    static U08 *shitbuf;

    if(shit)
    {
        shit = 0;
        shitbuf = mpx_Malloc(1024*128);
    }

    return shitbuf;
    #endif
}

U08 VideoActionMsgIdGet()
{
    //REMOVE return u08VideoActMsgId;
}



U08 VideoReturnMsgIdGet()
{
    //REMOVE return u08VideoActReturnMsgId;
}

S32 mpx_VideoConVisualEncodedTypeSet(U08 codecType, U08 windowType, U08 encQuality, U08 encResolution, U08 nextFrameType, U08 skipFrames, BOOL boolErrResillence, BOOL boolRVLC, S32 (*handle)(U08*, U32))
{
#if REMOVE
    UNION_VIDEO_API_MESSAGE message;

    message.VideoConVisualEncodeTypeSetMsg.u32ActionId = API_VIDEO_CON_VISUAL_ENCODE_TYPE_SET;
    message.VideoConVisualEncodeTypeSetMsg.u08CodecType = codecType;
    message.VideoConVisualEncodeTypeSetMsg.u08WindowType = windowType;
    message.VideoConVisualEncodeTypeSetMsg.u08EncodedQuality = encQuality;
    message.VideoConVisualEncodeTypeSetMsg.u08EncodedResolution = encResolution;
    message.VideoConVisualEncodeTypeSetMsg.u08NextFrameType= nextFrameType;
    message.VideoConVisualEncodeTypeSetMsg.u08SkipFrames= skipFrames;
    message.VideoConVisualEncodeTypeSetMsg.boolErrResillence = boolErrResillence;
    message.VideoConVisualEncodeTypeSetMsg.boolRVLC = boolRVLC;
    message.VideoConVisualEncodeTypeSetMsg.callBackHandle = (void*)handle;
    mpx_MessageSend((U08)VideoActionMsgIdGet(), (U08 *)&message, sizeof(message.VideoConVisualEncodeTypeSetMsg));
    mpx_MessageReceive((U08)VideoReturnMsgIdGet(), (U08 *)&message);

    return message.VideoConVisualEncodeTypeSetRetMsg.s32Response;
#endif
}


S32 mpx_VideoFunctionsStartup()
{
#if REMOVE
    if(!VideoInitialed)
    {
        DPrintf("Startup Video ISR & Tasks");

        SsSysSensorParameterInit();
        Mp4IsrInit();
        //IpuIsrInit();
        VideoInitialed = TRUE;

        return VideoTasksStartup();
    }
    return 0;
#endif
}

S32 mpx_AudioFunctionsStartup()
{
#if REMOVE
    I2cSemaphoreCreate();

    return AudioTasksStartup();
#endif
}

S32 mpx_GdcPanelVisualSizeGet(ENUM_IDU_WINDOW_TYPE eWindowType, U16 *pu16horizontal, U16 *pu16vertical)
{
#if REMOVE
    U16     u16frameh, u16framev;

    if(!pu16horizontal || !pu16vertical)
        return  ERR_NULL_OBJECT;

    u16frameh = pstLcdControlObject[u08ActivePanelID]->stLcdInfo.u16PanelW;
    u16framev = pstLcdControlObject[u08ActivePanelID]->stLcdInfo.u16PanelH;
    if (mpx_GdcWindowRatioChanged(eWindowType))
    {
        *pu16horizontal = u16framev;
        *pu16vertical = u16frameh;
    }
    else
    {
        *pu16horizontal = u16frameh;
        *pu16vertical = u16framev;
    }

    return NO_ERR;
#endif
}




S32 mpx_SysCfgImageWindowSet(ST_IP_IMAGE_POINT stPos, ST_IP_IMAGE_SIZE stSize)
{
#if REMOVE
    U16 panelWidth, panelHeight;
    U16 temp;

    mpx_GdcPanelVisualSizeGet(IDU_IMAGE_WINDOW, &panelWidth, &panelHeight);

    temp = stPos.u16H & 0x0001;
    stPos.u16H &= 0xFFFE;
    stSize.u16H += temp;

    temp = stPos.u16V & 0x0001;
    stPos.u16V &= 0xFFFE;
    stSize.u16V += temp;

    stSize.u16H = (stSize.u16H + 1) & 0xFFFE;
    stSize.u16V = (stSize.u16V + 1) & 0xFFFE;

    // limit the setting width and height from over the panel range
    if ((stPos.u16H + stSize.u16H) > panelWidth)
    {
        stSize.u16H = (panelWidth - stPos.u16H) & 0xFFFE;
    }

    if ((stPos.u16V + stSize.u16V) > panelHeight)
    {
        stSize.u16V = (panelHeight - stPos.u16V) & 0xFFFE;
    }

    stSystemConfig.stImageWindowPos = stPos;
    stSystemConfig.stImageWindowSize = stSize;

    return NO_ERR;
#endif
}

S32 mpx_VideoConPreviewStart()
{
#if REMOVE
    UNION_VIDEO_API_MESSAGE message;

    SensorModulePowerOn();

    SensorModuleStatus = TRUE;

    message.VideoConPreviewStartMsg.u32ActionId = API_VIDEO_CON_PREVIEW_START;

    mpx_MessageSend((U08)VideoActionMsgIdGet(), (U08 *)&message, sizeof(message.VideoConPreviewStartMsg));
    mpx_MessageReceive((U08)VideoReturnMsgIdGet(), (U08 *)&message);

    return message.VideoConPreviewStartRetMsg.s32Response;
#endif
}

S32 mpx_VideoConVisualCodecInit(U08 *codecType, U08 *encQuality, U08 *encResolution, U08 *decResolution)
{
    UNION_VIDEO_API_MESSAGE message;

    message.VideoConVisualCodecInitMsg.u32ActionId = API_VIDEO_CON_VISUAL_CODEC_INIT;
    mpx_MessageSend((U08)VideoActionMsgIdGet(), (U08 *)&message, sizeof(message.VideoConVisualCodecInitMsg));
    mpx_MessageReceive((U08)VideoReturnMsgIdGet(), (U08 *)&message);

    *codecType = message.VideoConVisualCodecInitRetMsg.u08CodecType;
    *encQuality = message.VideoConVisualCodecInitRetMsg.u08EncodedQuality;
    *encResolution = message.VideoConVisualCodecInitRetMsg.u08EncodedResolution;
    *decResolution = message.VideoConVisualCodecInitRetMsg.u08DecodedResolution;

    return message.VideoConVisualCodecInitRetMsg.s32Response;
}

void AudioCodecInit(BOOL codecAct)
{
#if REMOVE
    ST_AUDIO_FREQ_CONFIG pllConfig;

#if defined ( CHIP_TYPE_MP322 )

    pllConfig.bEmbededCodec = 1;
    pllConfig.bAdcDataType = 0;
    pllConfig.bOutClkConstant = 0;
    pllConfig.bNoMclkOutput = 0;            // GPIO0 => 0: Output, 1: Input
    pllConfig.u32MultiSampleRate = 256;     // KHz (12MHz)
    pllConfig.bAutoGanClk = 1;
    pllConfig.bSwAgcEnable = 1;
    pllConfig.u16SwAgcGainLevel = 20;

    if( mpx_AudioFrequencySet(&pllConfig, NULL) )
    {
        DPrintf("Audio PLL Config setting error!!");
        BREAK_POINT();
    }

#elif defined(CHIP_TYPE_MP321D)

    pllConfig.bEmbededCodec = 0;            // 0 : using External ADC for Mic input
    pllConfig.bAdcDataType = 1;
    pllConfig.bOutClkConstant = 1;
    pllConfig.bNoMclkOutput = 0;            // GPIO0 => 0: Output, 1: Input
    pllConfig.u32OutClockRate = 12000;      // KHz (12MHz)
    pllConfig.bAutoGanClk = 1;
    pllConfig.bSwAgcEnable = 1;
    pllConfig.u16SwAgcGainLevel = 20;

    if(mpx_AudioFrequencySet(&pllConfig,NULL))
    {
        DPrintf("Audio PLL Config setting error!!");
        BREAK_POINT();
    }
#endif

    AudioI2sInfSet(codecAct);
#endif
}

S32 mpx_AudioCodecInit(void)
{
#if REMOVE
#if defined ( CHIP_TYPE_MP322 )
    I2sPowerUp();
    EmbededCodecAnalogPathReset();
#endif

    return NO_ERR;
#endif
}

S32 mpx_AudioCodecAdcEnable(U32 sampleRate)
{
#if REMOVE
#if defined ( CHIP_TYPE_MP322 )
    DPrintf("Codec ADC enable ...");
    EmbededCodecHighPassFilter(0);      //Enable: 0
    EmbededCodecAnalogMuteSet(MIC_INPUT|AUX_INPUT|ADC_OUTPUT|SPEAKER_OUTPUT|HEADPHONE_OUTPUT, TRUE);
    EmbededCodecAnalogPathSet(MIC_INPUT, ADC_OUTPUT, TRUE);
    EmbededCodecAnalogMuteSet(MIC_INPUT|ADC_OUTPUT, FALSE);
    EmbededCodecAnalogGainAdjust(ADC_OUTPUT, 0xf); // 0x1 ~ 0xf => 0 ~ 21db
    EmbededCodecAnalogGainAdjust(AUX_INPUT, 0x1);  // 0x1 ~ 0xf => 0 ~ 21db
    EmbededCodecAnalogGainAdjust(MIC_INPUT, 0xf);  // 0x1 ~ 0xf => 0 ~ 21db
#endif

    return NO_ERR;
#endif
}

S32 mpx_VideoConSoundEncodedTypeSet(U08 codecType, U08 sampleRateIdx, S32 (*handle)(U08*, U32, BOOL))
{
    UNION_VIDEO_API_MESSAGE message;

    message.VideoConSoundEncodeTypeSetMsg.u32ActionId = API_VIDEO_CON_SOUND_ENCODE_TYPE_SET;
    message.VideoConSoundEncodeTypeSetMsg.u08CodecType = codecType;
    if ( sampleRateIdx > AUDIO_SAMPLERATE_24K )
        return ERR_VIDEO_INVALID_PARAMETER;
    message.VideoConSoundEncodeTypeSetMsg.u08SampleRateIdx = sampleRateIdx;
    message.VideoConSoundEncodeTypeSetMsg.callBackHandle = (void*)handle;
    mpx_MessageSend((U08)VideoActionMsgIdGet(), (U08 *)&message, sizeof(message.VideoConSoundEncodeTypeSetMsg));
    mpx_MessageReceive((U08)VideoReturnMsgIdGet(), (U08 *)&message);

    return message.VideoConSoundEncodeTypeSetRetMsg.s32Response;
}




void SensorModulePowerOff(void)
{
#if REMOVE
    if (stSensorRec.u08SensorNum == 0)
        return;

    switch (stSensorRec.s08ActiveSensor)
    {
    case 0:
        #ifdef PRI_SENSOR_PWR
        if (PRI_SENSOR_PWR != 0xFFFFFFFF)
        {
            if (PRI_SENSOR_PWR_POL == 0)
                mpx_GpioDataSet(PRI_SENSOR_PWR, 1);
            else
                mpx_GpioDataSet(PRI_SENSOR_PWR, 0);
        }
        #endif
        break;

    case 1:
        #ifdef SEC_SENSOR_PWR
        if (SEC_SENSOR_PWR != 0xFFFFFFFF)
        {
            if (SEC_SENSOR_PWR_POL == 0)
                mpx_GpioDataSet(SEC_SENSOR_PWR, 1);
            else
                mpx_GpioDataSet(SEC_SENSOR_PWR, 0);
        }
        #endif
        break;

    case 2:
        #ifdef THIRD_SENSOR_PWR
        if (THIRD_SENSOR_PWR != 0xFFFFFFFF)
        {
            if (THIRD_SENSOR_PWR_POL == 0)
                mpx_GpioDataSet(THIRD_SENSOR_PWR, 1);
            else
                mpx_GpioDataSet(THIRD_SENSOR_PWR, 0);
        }
        #endif
        break;

    default:
        DPrintf("SensorModulePowerOff - Wrong Active Sensor ID");

        return;
        break;
    }

    if (flashDriverProc)
    {
        flashDriverProc->chipEnable(FALSE);
    }
#endif
}


S32 UsbdPipeWrite(U08 pipenum, ST_PIPE_MESSAGE *pmsg)
{
#if 0
    ST_DRIVER_MESSAGE dmsg;

    if(!stUsbdDriver.UsbdSendStart(stPipeInfo.stPipes[pipenum].u08EpNum, pmsg->u32Len, pmsg->data, FALSE))
    {
        mpx_MessageReceive(stPipeInfo.stPipes[pipenum].u08PipeMessageId , (U08*)&dmsg);

        pmsg->u32Event = dmsg.u32Event;
            
        if(pmsg->u32Event == EVENT_DATA_SEND_DONE)
        {
            return 0x00;
        }
        else if(pmsg->u32Event == EVENT_BUS_RESET || pmsg->u32Event == EVENT_SOFT_DISCON_CON || 
            pmsg->u32Event == EVENT_SUSPEND || pmsg->u32Event == EVENT_DISCONNECT)
            return 0x00;
        else
        {
            DPrintf("pipe %2d write error u32Event = 0x%x", pipenum, dmsg.u32Event);
        }
    }
#endif

    return 0xff;
}

U32 HwTimerResolutionGet(U08 index)
{
    //return (U32)u32TimerResolution[index];
}

S32 UsbdPipeFrameNumGet(U16 *frameNum, U08 *uFrameNum)
{
    //return stUsbdDriver.UsbdFrameNumGet(frameNum, uFrameNum);
}

S32 UsbdPipeStop(U08 pipenum)
{
    U08 ep;
        
    //ep = stPipeInfo.stPipes[pipenum].u08EpNum;   
    //stUsbdDriver.UsbdEpDeReg(ep);
    //stUsbdDriver.UsbdEpDis(ep);
    return 0;
}

S32 UsbdPipeStart(U08 pipenum, U16 maxpktsize)
{
    U08 ep;
    U08 eptype;
    
    //ep = stPipeInfo.stPipes[pipenum].u08EpNum;
    //eptype = stPipeInfo.stPipes[pipenum].u08EpType;
    //stPipeInfo.stPipes[pipenum].u16MaxPacketSize = maxpktsize;

    //stUsbdDriver.UsbdEpReg(eptype, ep, maxpktsize);
    return 0;
}



typedef U32 FILE_HANDLE;

///
///@ingroup api_File
///@brief   If the node pointed by the *wPath is exsit, open it else create the file.
///
///@param   *wPath      the uni-code path of open file
///@param   attr        reserved
///@param   mode        reserved
///
///@retval  ERR_FS_FILE_OPERATION_FAIL  Mean the path of *wPath is wrong when the @b create paramter is FALSE or
///                                     create file fail when the @b create paramter is TRUE.
///@retval  ERR_FS_FILE_HANDLE_FULL     Mean the file handle in system is not enough.
///@retval  ERR_FS_STORAGE_FULL         Mean the storage is full.
///
///@remark  the *wPath can be an absolute path or a relative path
///
S32 mpx_FileCreateOpen(U16 *wPath, U32 attr, U32 mode)
{
#if REMOVE
    ST_DRIVE *drv;
    U16 *fileName;
    U16 *dirPath;
    U16 tmpWchar;
    S32 retCode;
    U32 modeValue;

    drv = DriveGet(DriveCurIdGet());
    DRIVE_CHECK;
#if 1
    if(mode & FILE_CREATE_AUTONAME)
    {
        return  mpx_FileAutoNameCreate(wPath);
    }
    else
    {
        U32 posId;

        modeValue = mode & FILE_CREATE_OPEN_MASK;

        posId = mpx_DirNodePositionGet();
        retCode = mpx_DirNodeLocate(wPath);
        if((retCode == NO_ERR) && (modeValue == FILE_CREATE_ALWAYS))
        {
            mpx_DirFileDelete();
            mpx_DirNodePositionSet(posId);
            retCode = ERR_FS_NODE_NOT_FOUND;
        }
        mpx_DirNodePositionRelease(posId);

        if(retCode == NO_ERR)
        {//file found
            if((modeValue == FILE_CREATE_OPEN) || (modeValue == FILE_OPEN_ONLY))
            {//open
                return mpx_FileOpenEx(mode);
            }
            else
            //create fail
                return ERR_FS_DUPLICATED_NAME;
        }
        else if((retCode == ERR_FS_NODE_NOT_FOUND))
        {//file not found
            if((modeValue == FILE_CREATE_OPEN) || (modeValue == FILE_CREATE_ONLY) || (modeValue == FILE_CREATE_ALWAYS))
            {//create
                //separate path and file name
                fileName = dirPath = wPath;
                while(*wPath)
                {
                    if((*wPath == '/') || (*wPath == '\\'))
                        fileName = wPath+1;
                    wPath++;
                }
                if(fileName > dirPath+1)
                {
                    if(*(fileName-2) == '/' || *(fileName-2) == '\\')   //the case "//xxx.jpg"
                        mpx_DirRootChange();
                    else
                    {
                        tmpWchar = *(fileName-1);
                        *(fileName-1) = 0;
                        retCode = mpx_DirPathMake(dirPath);
                        if((retCode != NO_ERR) && (retCode != ERR_FS_DUPLICATED_NAME))
                        {
                            *(fileName-1) = tmpWchar;
                            return ERR_FS_FILE_OPERATION_FAIL;
                        }
                        DirSubChange(drv);
                        *(fileName-1) = tmpWchar;
                    }
                }
                else if(fileName == (dirPath+1))    //may be the case "/xxx.jpg"
                    mpx_DirRootChange();
                    //the wPath is only file name

                if(!*fileName)
                {
                    DPrintf("no file name");
                    return ERR_FS_FILE_OPERATION_FAIL;
                }
                retCode = mpx_FileCreate(fileName);
                if(retCode > 0)
                    mpx_DirNodeAttributeSet(attr);
                return retCode;
            }
            else
            //open fail
                return ERR_FS_NODE_NOT_FOUND;
        }
        else
            //path string error
            return retCode;
    }
#else
    if(mode & FILE_CREATE_OPEN)
    {
        fileName = dirPath = wPath;
        while(*wPath)
        {
            if((*wPath == '/') || (*wPath == '\\'))
                fileName = wPath+1;
            wPath++;
        }
        if(fileName > dirPath+1)    //may be the case "/xxx.jpg"
        {
            tmpWchar = *(fileName-1);
            *(fileName-1) = 0;
            retCode = mpx_DirPathMake(dirPath);
            if(retCode != NO_ERR && retCode != ERR_FS_DUPLICATED_NAME)
                return ERR_FS_FILE_OPERATION_FAIL;
            DirSubChange(drv);
            *(fileName-1) = tmpWchar;
        }
        //else
            //the wPath is only file name

        if(!*fileName)
            return ERR_FS_FILE_OPERATION_FAIL;
        if(DirLocate(drv, fileName))
            return mpx_FileCreate(fileName);
        else
            return mpx_FileOpen();
    }
    else
    {
        if(DirLocate(drv, wPath))    //file is not exsit
            return ERR_FS_FILE_OPERATION_FAIL;
        else
            return mpx_FileOpen();
    }
#endif
#endif
}

///
///@ingroup api_File
///@brief   Get the file size.
///
///@param   handle  Specify the opened file
///@param   *size   Get the file size
///
///@retval  ERR_FS_FILE_HANDLE_INVALID  The input handle parameter is not a valide file handle.
///
S32 mpx_FileSizeGet(FILE_HANDLE handle, U32 *size)
{
#if REMOVE
    ST_STREAM *fileStream;

    fileStream = GetHandleByIdx(handle);
    if(fileStream->Drv && fileStream)
    {
        *size = FileSizeGet(fileStream);
        return NO_ERR;
    }
    else
        return ERR_FS_FILE_HANDLE_INVALID;
#endif
}




///
///@ingroup api_File
///@brief   Reads size bytes of data.
///
///@param   handle  Specify the opened file
///@param   *buffer The buffer to get the data that read from the specified file
///@param   size    The size that wanted to read
///
///@retval  0~2^32-1    The size of file read.
///
U32 mpx_FileRead(FILE_HANDLE handle, U08 *buffer, U32 size)
{
#if REMOVE
    ST_STREAM *fileStream;
    U32 len;

    fileStream = GetHandleByIdx(handle);
    if(fileStream->Drv && fileStream)
    {
    #ifdef FS_RW_MEASURE
        U32 t;
        if(measureF == handle)
        {
            t = TimerRegister();
        }
    #endif

        if(fileStream->Stream.ReadStreaming.pu08StartBufAddr)
            len = FileStreamRead(fileStream, buffer, size);
        else
        {
            if(fileStream->BindingStreaming)
            {
                len = FileStreamRead(fileStream, buffer, size);
                if(len != size)
                {
                    len = FileRead(fileStream, buffer, size);
                    //DPrintf("Miss");
                }
                else
                {
                    //DPrintf("Hit");
                }
            }
            else
                len = FileRead(fileStream, buffer, size);
        }

    #ifdef FS_RW_MEASURE
        if(measureF == handle)
            DPrintf("T:%d",TimerRelease(t));
    #endif

        return len;
    }
    else
    {
        DPrintf("read: err handle num");
        return 0;
    }
#endif
}




///
///@ingroup api_File
///@brief   Conslusion the access of an opened file specified by the handle.
///
///@param   handle  Specify the opened file
///
///@retval  ERR_FS_FILE_HANDLE_INVALID  The input handle parameter is invalid.
///
S32 mpx_FileClose(FILE_HANDLE handle)
{
#if REMOVE
    ST_STREAM *fileStream;

#ifdef FS_RW_MEASURE
    if(measureF == handle)
        measureF = 0;
#endif

    fileStream = GetHandleByIdx(handle);
    if(fileStream->Drv && fileStream)
    {
        if(fileStream->u32Flag.IsHostFile)  //because the commad handle field is U08 type
            mpx_LogicFileHandleRelease(fileStream->u32CacheBufSize);
        FileClose(fileStream);
        return NO_ERR;
    }
    else
        return ERR_FS_FILE_HANDLE_INVALID;
#endif
}

///
///@ingroup     TIMER
///@brief       get current count value of desired Timer
///
///@param   u08TmrSelect    the desired timer index
///
///@retval	the desired timer time current value
///
U32 HwTimerCurValueGet(U08 u08TmrSelect)
{
    //ST_TIMER_REG *pstTmr;

    //pstTmr = pstTmr1 + u08TmrSelect;
    //return(pstTmr->TM_V);
}

#endif



