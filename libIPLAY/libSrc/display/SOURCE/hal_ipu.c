
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"

#include "timer.h"

#include "taskid.h"
#include "hal_ipu.h"


#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))

#define LUT_Count 16
/**
 * @ingroup Color
 * @brief Ipu Set Hue and Saturation
 * @param Hue_value:-8~+7  default=0
 * @param Saturation_value:-8~+7  default=0 
 */
void Ipu_SetHueSaturation(int Hue_value,int Saturation_value)
{
		IPU *ipu = (IPU *)IPU_BASE;

		//GainLUT[*]:Saturation*256
		//saturation MAX 2X scale 1.1
		DWORD SaturationGainLUTx256[LUT_Count]= 
		{
		    119,// -8	0.466x
		    131,// -7	0.513x
		    144,// -6	0.564x
		    159,// -5	0.621x
		    175,// -4	0.683x
		    192,// -3	0.75x
		    211,// -2	0.826x
		    233,// -1	0.91x
		    256,// 0	1x
		    282,// 1	1.1x
		    310,// 2	1.21x
		    341,// 3	1.33x
		    375,// 4	1.464x
		    412,// 5	1.61x
		    453,// 6	1.771x
		    499// 7 1.948x
		};
		//GainLUT[*]:Hue*256
		//hue MAX 1.5 scale 1.06
		DWORD HueGainLUTx256[LUT_Count+1]= 
		{
		    160,// -8	0.6x
		    170,// -7	0.627x
		    180,// -6	0.665x
		    191,// -5	0.7x
		    203,// -4	0.75x
		    215,// -3	0.79x
		    228,// -2	0.84x
		    242,// -1	0.94x
		    256,// 0	1x
		    271,// 1	1.06x
		    288,// 2	1.12x
		    305,// 3	1.19x
		    323,// 4	1.26x
		    343,// 5	1.34x
		    363,// 6	1.42x
		    385,// 7  1.5x
		    408  //8
		};



		long TotalCbGainx256,TotalCrGainx256;
		int i;


		if(Saturation_value>7)Saturation_value=7;
		if(Saturation_value<-8)Saturation_value=-8;
		if(Hue_value>7)Hue_value=7;
		if(Hue_value<-8)Hue_value=-8;	


		//Hue_value=Hue_value+8;
		
		TotalCbGainx256=(SaturationGainLUTx256[Saturation_value+8]*HueGainLUTx256[Hue_value+8])>>8;
		TotalCrGainx256=(SaturationGainLUTx256[Saturation_value+8]*HueGainLUTx256[(-1*Hue_value)+8])>>8;
		
/*
		//ycbcr in->scl->yuvp->csc1->rgbp->csc0->ycbcr out
		ipu->Ipu_reg_18 = 0x00020203 ;
		ipu->Ipu_reg_19 = 0x02010000 ;

		//Ipu_CSC0Set();
		ipu->Ipu_reg_40 = 0x00010000;// UV minus 128 after 3x3 matrix mult
		ipu->Ipu_reg_41 = 0x004C0096;//R = 4D ; G = 95
		ipu->Ipu_reg_42 = 0x001D0000;//B = 1D ; OFF = 0 	
		ipu->Ipu_reg_43 = 0x07D407AA;//R = -2C = >2`=>0x7D4 ; G = 0x7AA
		ipu->Ipu_reg_44 = 0x00820000;//B = 82 ; OFF = 7F
		ipu->Ipu_reg_45 = 0x00820793;//R = 82 ; G = -6D => 2`=> 0x793 
		ipu->Ipu_reg_46 = 0x07EB0000;//B = -15 = >2`=>0x7EB ; OFF = 7F

		//Ipu_CSC1Set();
		ipu->Ipu_reg_47 = 0x01000000;// UV add 128 after 3x3 matrix mult
		ipu->Ipu_reg_48 = 0x01000000;// Y = 1 , Cb = 0 
		ipu->Ipu_reg_49 = 0x015E0000;// Cr = 1.371 = 0x15E , Offset = 0 	
		ipu->Ipu_reg_4A = 0x010007AA;// Y = 1 , Cb =  -0x56 => 2`=> 0x7AA
		ipu->Ipu_reg_4B = 0x074E0000;// Cr = -0xB2 => 2`=> 0x74E, Offset = 0
		ipu->Ipu_reg_4C = 0x010001BB;// Y = 1 , Cb = 1.732 = 0x1BB  
		ipu->Ipu_reg_4D = 0x00000000;// Cr = 0, Offset = 0
*/
		ipu->Ipu_reg_3E = 0x00010100; //SP->CS


		//CbCrGain
		//ipu->Ipu_reg_3C=0xC0C00000;//CbCrGain  3X
		//RegisterCode=TotalCbGainx256*64
		ipu->Ipu_reg_3C=((TotalCbGainx256>>2)<<24)+((TotalCrGainx256>>2)<<16);


//ClearHueSATflag();		
//IPUCbCrSwitch=0;
}

/**
 * @ingroup SpecialEffect
 * @brief   Sketch
 * 
 *
 */ 
void Ipu_Sketch(BOOL level)
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;		
	#if 0
	ipu->Ipu_reg_30 = 0x00A02000;
	BOOL level = 1;//
	if(level)//high
		ipu->Ipu_reg_26 = 0x00010100;
	else//low
		ipu->Ipu_reg_26 = 0x01010100;
	ipu->Ipu_reg_51 = 0x01808000;
	ipu->Ipu_reg_56 = 0x01010100;
	#else
	/*
	ipu->Ipu_reg_30=0x00A02000;

	ipu->Ipu_reg_26 = 0x01010100;
	ipu->Ipu_reg_51 = 0x01808000;
	ipu->Ipu_reg_56 = 0x01010100;
	*/
	ipu->Ipu_reg_26 = 0x00010100;
	ipu->Ipu_reg_51 = 0x01808000;
	ipu->Ipu_reg_56 = 0x01010100;	

	#endif
}

void Ipu_Crayon(void)
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;		
	ipu->Ipu_reg_26 = 0x00010100;
	ipu->Ipu_reg_51 = 0x00808000;	
	ipu->Ipu_reg_56 = 0x01010100;	
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ CSC ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// YCbCr -> RGB
// R = ( Y + 1.371 ( Cr - 128 )) 
// G = ( Y - 0.698 ( Cr - 128 ) - 0.336 ( Cb - 128 )) 
// B = ( Y + 1.732 ( Cr - 128 )) 
//=>DEC
// R = ( Y + 350 ( Cr - 128 )) / 256 
// G = ( Y - 178 ( Cr - 128 ) - 86 ( Cb - 128 )) / 256 
// B = ( Y + 443 ( Cb - 128 )) / 256 
//=>HEX
// R = ( Y + 0x15E ( Cr - 128 )) / 256 
// G = ( Y - 0xB2 ( Cr - 128 ) - 0x56 ( Cb - 128 ) ) / 256 
// B = ( Y + 0x1BB ( Cb - 128 )) / 256 
// Example : Ipu_CSC0Set();
int Ipu_CSC0Set()//YCbCr -> RGB
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;	
	ipu->Ipu_reg_40 = 0x01000000;// UV add 128 after 3x3 matrix mult
	ipu->Ipu_reg_41 = 0x01000000;// Y = 1 , Cb = 0 
	ipu->Ipu_reg_42 = 0x015E0000;// Cr = 1.371 = 0x15E , Offset = 0 	
	ipu->Ipu_reg_43 = 0x010007AA;// Y = 1 , Cb =  -0x56 => 2`=> 0x7AA
	ipu->Ipu_reg_44 = 0x074E0000;// Cr = -0xB2 => 2`=> 0x74E, Offset = 0
	ipu->Ipu_reg_45 = 0x010001BB;// Y = 1 , Cb = 1.732 = 0x1BB  
	ipu->Ipu_reg_46 = 0x00000000;// Cr = 0, Offset = 0
	return  NO_ERR;	
}
// RGB -> YCbCr 
// Y = ( 0.299R + 0.587G + 0.114B )
// Cb = ( -0.172R - 0.339G + 0.511B )
// Cr = ( 0.511R - 0.428G - 0.083B ) 
//=>DEC
// Y = ( 76R + 149G + 29B ) / 256 + 0
// Cb = ( -44R - 86G + 130B ) / 256 + 128
// Cr = ( 130R - 109G - 21B ) / 256 + 128
//=>HEX
// Y = ( 0x4DR + 0x95G + 0x1DB ) / 256 + 0
// Cb = ( -0x2CR - 0x56G + 0x82B ) / 256 + 128
// Cr = ( 0x82R - 0x6DG - 0x15B ) / 256 + 128
// Example : Ipu_CSC1Set();
int Ipu_CSC1Set()// RGB -> YCbCr
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;	

	ipu->Ipu_reg_47 = 0x00010000;// UV minus 128 after 3x3 matrix mult
	ipu->Ipu_reg_48 = 0x004C0096;//R = 4D ; G = 95
	ipu->Ipu_reg_49 = 0x001D0000;//B = 1D ; OFF = 0 	
	ipu->Ipu_reg_4A = 0x07D407AA;//R = -2C = >2`=>0x7D4 ; G = 0x7AA
	ipu->Ipu_reg_4B = 0x00820000;//B = 82 ; OFF = 7F
	ipu->Ipu_reg_4C = 0x00820793;//R = 82 ; G = -6D => 2`=> 0x793 
	ipu->Ipu_reg_4D = 0x07EB0000;//B = -15 = >2`=>0x7EB ; OFF = 7F
	return  NO_ERR;	
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~SCL EDGE~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Title : AutoEdge 
// USED :
//
// use_p1low = 1					   use_p1low = 0
//            |    blur   | alpha   //            |    non    |   low_alpha     
// edge thr   |-----------|------   // edge thr   |-----------|------------
// -----------|    non    |         // -----------|    non    | 
// edge mhigh |-----------|         // edge mhigh |-----------|
// -----------| edge gain | alpha	// -----------| edge gain |   alpha
// edge mlow  |-----------|			// edge mlow  |-----------|
// -----------|    non    |    		// -----------|    non    | 
//
//example : Ipu_SetAutoEdgeFumc(0, 1, 50, 100, 550, 100, 3, 100);//

int Ipu_SetAutoEdgeFumc(char SelEdgeMode, char use_p1low, char edge_threshold, char edge_Mhight , char edge_MLow, 
	                    char Y_low_alpha, char EdgeGain, char YC_AlphaValue )
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;		
	ipu->Ipu_reg_24 |= (((DWORD)edge_threshold << 16)|(DWORD)edge_MLow); 
	ipu->Ipu_reg_25 |= (((DWORD)edge_Mhight << 16)|((DWORD)EdgeGain << 8)|(DWORD)use_p1low); 
	ipu->Ipu_reg_26 |= (DWORD)SelEdgeMode << 24 ;
	ipu->Ipu_reg_27 |= (((DWORD)YC_AlphaValue << 16)|((DWORD)YC_AlphaValue << 8)|(DWORD)Y_low_alpha);  
	return  NO_ERR;	
}

// Title : Edge Func 
// USED 
// Edge Level
// light  |   middle  |  heavy
// alpha  |   Edge    | alpha + Edge
/*/
//*/

int Ipu_SetEdgeFumc(char SelEdgeMode, char EdgeGain, char YC_AlphaValue )
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;		
	ipu->Ipu_reg_25 |= (DWORD)EdgeGain << 8 ; 	
	//ipu->Ipu_reg_26 |= (DWORD)SelEdgeMode << 24 ;
	ipu->Ipu_reg_26 = 0x02000000;
	ipu->Ipu_reg_27 |= (((DWORD)YC_AlphaValue << 16)|((DWORD)YC_AlphaValue << 8));  
	return  NO_ERR;	
}

// Title : Blur Func 
// USED 
// Blur Level
// light -->  heavy
// Blur 1     Blur 0
// Example : Ipu_SetBlur(0);//Blur0
// Example : Ipu_SetBlur(1);//Blur1	  

int Ipu_SetBlur(char BlurValue)
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;		
	
	ipu->Ipu_reg_26 = 0x01000000;
	//ipu->Ipu_reg_26 |= 0x01000000;
	ipu->Ipu_reg_25 |= (DWORD)BlurValue << 10;
	return  NO_ERR;	
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~YUV SP~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~offset~~~~~~~~~~~~
//Ipu_SetIpuGlobalEnable(1, 0, 0);
// offset+60
//Ipu_SetBrightness(0x3c);
// offset-60
//Ipu_SetBrightness(0x1c4);
//*/
int Ipu_SetBrightness(DWORD YOffsetValue)
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;	
	YOffsetValue &= 0x01ff; 
	ipu->Ipu_reg_30 |= ((DWORD)YOffsetValue << 16);
	MP_DEBUG("ipu->Ipu_reg_30=0x%08x", ipu->Ipu_reg_30);
	return  NO_ERR;
}


// gain 2X	  
//Ipu_SetContrast(2, 0);
// gain 4X	  
//Ipu_SetContrast(4, 0);
// gain 0.XX	  
//Ipu_SetContrast(0, 30);
int Ipu_SetContrast(BYTE Yintgain , BYTE Yfloatgain) 
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;	
	DWORD Ygain = 0;

	if(Yintgain > 7)
		Yintgain = 7; 

	if(Yfloatgain > 31)
		Yfloatgain = 31; 
	
	Ygain = (((DWORD)Yintgain << 5)|((DWORD)Yfloatgain));
	ipu->Ipu_reg_30 &= 0xffff0000; 
	ipu->Ipu_reg_30 |= (DWORD)Ygain << 8;

	return  NO_ERR;	
}

// CbCrOffset 
//Ipu_SetCbCrOffset(0x21, 0x21);//Cb = - 32 ; Cr = - 32; //CbCr1
//Ipu_SetCbCrOffset(0x1f, 0x1f);//Cb = + 32 ; Cr = + 32;//CbCr2

int Ipu_SetCbCrOffset(BYTE CbOffsetValue,BYTE CrOffsetValue)
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;	
	
	CrOffsetValue &= 0x003f; 
	CbOffsetValue &= 0x003f; 	
	MP_DEBUG("CbOffsetValue =0x%08x", CbOffsetValue);
	MP_DEBUG("CrOffsetValue =0x%08x", CrOffsetValue);	

	ipu->Ipu_reg_3B |= ((DWORD)CbOffsetValue << 24)|((DWORD)CrOffsetValue << 16);
	MP_DEBUG("ipu->Ipu_reg_3B=0x%08x", ipu->Ipu_reg_3B);
	return  NO_ERR;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~YUV SE~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
// Title : Ipu Global Config 
// USED 
// yuvse_en | yuvcs_en | yuvsp_ahead |  Ans
//    1     |     0    |      0      | SE => SP
//    0     |     0    |      0      |    SP
//    0     |     1    |      0      | CS => SP
//    0     |     1    |      1      | SE => CS
*/
int Ipu_SetIpuGlobalEnable(BOOL yuvse_en,BOOL yuvcs_en,BOOL yuvsp_ahead)
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;
	if(yuvse_en==1 && yuvcs_en==1 && yuvsp_ahead==1)
		return  FAIL;
	
	ipu->Ipu_reg_3E |= ((DWORD)yuvse_en<<24)|((DWORD)yuvcs_en<<16)|((DWORD)yuvsp_ahead<<8);
	MP_DEBUG("ipu->Ipu_reg_3E=0x%08x", ipu->Ipu_reg_3E);
	return  NO_ERR;
}


/**
 * @ingroup SpecialEffect
 * @brief   SetBlackWitheEffect
 * 
 * @param  CbCrBWFixValue_en\n
 * @param  Cb_BWFixValue\n
 * @param  Cr_BWFixValue\n
 * @param  sebw_en\n
 * @param  Cb_BWGain\n
 * @param  Cr_BWGain\n
 *
 *  Title : Black & White Effect
 *  State : YcbCr Color Mag         
 * ~~~~~~~Black & White~~~~~~~
 *  Ipu_SetBlackWitheEffect( 1, 128, 128 , 1 , 1, 1);
 *
 */ 
int Ipu_SetBlackWitheEffect(BOOL CbCrBWFixValue_en,BYTE Cb_BWFixValue,BYTE Cr_BWFixValue,
                            BOOL sebw_en,BYTE Cb_BWGain,BYTE Cr_BWGain)
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;	
	if(Cr_BWGain>128)
		Cr_BWGain = 127;

	if(Cb_BWGain>128)
		Cb_BWGain = 127;

	ipu->Ipu_reg_50 |= (((DWORD)sebw_en<<24)|((DWORD)Cb_BWGain<<16)|((DWORD)Cr_BWGain<<8));
	ipu->Ipu_reg_51 |= (((DWORD)CbCrBWFixValue_en<<24)|((DWORD)Cb_BWFixValue<<16)|((DWORD)Cr_BWFixValue<<8));
	MP_DEBUG("ipu->Ipu_reg_50=0x%08x", ipu->Ipu_reg_50);	
	MP_DEBUG("ipu->Ipu_reg_51=0x%08x", ipu->Ipu_reg_51);
	return  NO_ERR;
}

/*
// Title : HG ( high Gain & high offset) 
// State : YcbCr Color Mag
// USED 
// Contrast  Effect 
*/
// HG ( high Gain & high offset)	  
//~~~~~~~HG_offset~~~~~~~
// offset+60
//Ipu_SetHighGainEffect(1, 60, 1, 1);
// offset-60
//Ipu_SetHighGainEffect(1, 0x1c4, 1, 1);
// HG_gain 1.14X
//Ipu_SetHighGainEffect(1, 0, 1, 15);
// HG_gain 4X	  
//Ipu_SetHighGainEffect(1, 0, 4, 0);

int Ipu_SetHighGainEffect(BOOL HG_en,DWORD HG_offset,BYTE HG_intgain , BYTE HG_floatgain)
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;	
	DWORD HG_gain = 0;
	HG_offset &= 0x01ff;

	MP_DEBUG("offset=0x%08x", HG_offset);
	MP_DEBUG("HG_gain=0x%08x", HG_gain);
	if(HG_floatgain > 15)
		HG_floatgain = 15;
	HG_gain = (((DWORD)HG_intgain << 4)|((DWORD)HG_floatgain));
	
	ipu->Ipu_reg_52 |= ((DWORD)HG_en<<28)|((DWORD)HG_offset<<16)|((DWORD)HG_gain<<0);
	MP_DEBUG("ipu->Ipu_reg_52=0x%08x", ipu->Ipu_reg_52);
	return  NO_ERR;	
}

/**
 * @ingroup SpecialEffect
 * @brief   SetColorMixEffect
 * 
 * @param  CM_en\n
 * @param  Y_gain\n
 * @param  Y_fixGain_en\n
 * @param  Y_fixGain\n
 * @param  Cbr_reverse\n
 * @param  Cb_Tone\n
 * @param  Cr_Tone\n
 
 * Title : Color Mix + CbCr (CM) 
 * State : YcbCr Color Mag 
 * USED  
 * Color Mix Effect  
 * Cb_Tone & Cr_Tone is 2`complement 
 * 
 *~~~~~~~Color Mix + CbCr (CM)~~~~~~~ 
 *Ipu_SetColorMixEffect(1, 0, 0, 0, 0, 0x80, 0x7f);// Cb = -128 ; Cr = 127  	//CM1   \n
 *Ipu_SetColorMixEffect(1, 0, 0, 0, 0, 0x80, 0x00);// Cb = -128 ; Cr = 0		//CM2     \n
 *Ipu_SetColorMixEffect(1, 0, 0, 0, 0, 0x80, 0x80);// Cb = -128; Cr = -128	//CM3     \n
 *Ipu_SetColorMixEffect(1, 0, 0, 0, 0, 0x00, 0x80);// Cb =  0  ; Cr = -128		//CM4   \n
 *Ipu_SetColorMixEffect(1, 0, 0, 0, 0, 0x7f, 0x80);// Cb = 127 ; Cr = -128	//CM5     \n
 *Ipu_SetColorMixEffect(1, 0, 0, 0, 0, 0x7f, 0x00);// Cb = 127 ; Cr = 0		//CM6       \n
 *Ipu_SetColorMixEffect(1, 0, 0, 0, 0, 0x7f, 0x7f);// Cb = 127 ; Cr = 127		//CM7     \n
 *Ipu_SetColorMixEffect(1, 0, 0, 0, 0, 0x00, 0x7f);// Cb =  0  ; Cr = 127		//CM8     \n
 *Ipu_SetColorMixEffect(1, 0, 0, 0, 0, 0x00, 0x00);// Cb =  0  ; Cr = 0	  		//CM0   
 */ 

int Ipu_SetColorMixEffect(BOOL CM_en,BYTE Y_gain,BOOL Y_fixGain_en,BYTE Y_fixGain,BOOL Cbr_reverse,BYTE Cb_Tone,BYTE Cr_Tone)  
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;	

	MP_DEBUG("Cb_Tone=0x%08x", Cb_Tone);
	MP_DEBUG("Cr_Tone=0x%08x", Cr_Tone);

	ipu->Ipu_reg_53 |= (((DWORD)CM_en<<24)|((DWORD)Y_gain<<16)|((DWORD)Y_fixGain_en<<8)|((DWORD)Y_fixGain));
	ipu->Ipu_reg_54 |= (((DWORD)Cbr_reverse<<24)|((DWORD)Cb_Tone<<16)|((DWORD)Cr_Tone<<8));
	MP_DEBUG("ipu->Ipu_reg_53=0x%08x", ipu->Ipu_reg_53);
	MP_DEBUG("ipu->Ipu_reg_54=0x%08x", ipu->Ipu_reg_54);
	return  NO_ERR;
}

/**
 * @ingroup SpecialEffect
 * @brief   SetSEBIANEffect
 * 
 * @param  Noise_en\n
 * @param  Noise_level\n
 * @param  Bilevel_en\n
 * @param  Bilevel_cbralso\n
 * @param  Bilevel\n
 *
 * Title : SEBIAN (bit level and noise)
 * State : YcbCr Color Mag
 * USED 
 * Color Mix Effect 
 *~~~~~~~SEBIAN (bit level and noise)	~~~~~~~
 *Ipu_SetSEBIANEffect(1, 3, 0, 1, 3);//SEBIN1
 *Ipu_SetSEBIANEffect(1, 3, 1, 1, 3);//SEBIN2
 *  
 */ 

int Ipu_SetSEBIANEffect(BOOL Noise_en,BYTE Noise_level,BOOL Bilevel_en,BOOL Bilevel_cbralso,BYTE Bilevel,BOOL RevPixel)  
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;	
	if( Noise_level > 3)
		Noise_level = 3;
	if( Bilevel > 3)
		Bilevel = 3;

	ipu->Ipu_reg_55 |= (((DWORD)Noise_en<<28)|((DWORD)Noise_level<<24)|
						((DWORD)Bilevel_en<<20)|((DWORD)Bilevel_cbralso<<16)|
						((DWORD)Bilevel<<12)|((DWORD)RevPixel<<8));
	MP_DEBUG("ipu->Ipu_reg_55=0x%08x", ipu->Ipu_reg_55);
	return  NO_ERR;
}



int Ipu_SetSpecialFumc(char SelEdgeMode, BOOL specialmoge, BOOL sensingmode)
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;		

	ipu->Ipu_reg_26 |= ((DWORD)SelEdgeMode << 24 )|((DWORD)specialmoge << 16)|((DWORD)sensingmode << 8);
	ipu->Ipu_reg_56 = 0x01000000;
	
	MP_DEBUG("ipu->Ipu_reg_26=0x%08x", ipu->Ipu_reg_3B);
	return  NO_ERR;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~IPU GAMMA~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	  	
/*/example
//gamma = 0.45
unsigned char IPUGammaTable45[34]= {0x20,0x35,0x49,0x58,
									0x64,0x6F,0x78,0x81,
									0x89,0x90,0x97,0x9E,
									0xA4,0xAA,0xB0,0xB6,
									0xBB,0xC0,0xC5,0xCA,
									0xCF,0xD3,0xD8,0xDC,
									0xE0,0xE5,0xE9,0xED,
									0xF1,0xF4,0xF8,0xFC,
									0xff};
//gamma = 1
unsigned char IPUGammaTable1[34]= {0x08,0x10,
								   0x18,0x20,0x28,0x30,
								   0x38,0x40,0x48,0x50,
								   0x58,0x60,0x68,0x70,
								   0x78,0x80,0x88,0x90,
								   0x98,0xA0,0xA8,0xB0,
								   0xB8,0xC0,0xC8,0xD0,
								   0xD8,0xE0,0xE8,0xF0,
								   0xF8,0xFF};
//gamma = 2.3
unsigned char IPUGammaTable23[34]= {0x00,0x00,0x00,
									0x01,0x02,0x03,0x05,
									0x07,0x0A,0x0D,0x11,
									0x15,0x1A,0x20,0x26,
									0x2C,0x33,0x3B,0x44,
								  	0x4D,0x56,0x61,0x6C,
								  	0x77,0x84,0x91,0x9E,
								  	0xAD,0xBC,0xCC,0xDC,
								  	0xED,0xff};
//gamma = s1
unsigned char IPUGammaTableS1[34]= {0x10,0x20,0x2D,0x37,
									0x3F,0x46,0x4E,0x55,
									0x5A,0x5C,0x5E,0x60,
									0x62,0x64,0x66,0x69,
									0x6B,0x6E,0x71,0x75,
									0x7A,0x80,0x87,0x8E,
									0x96,0x9F,0xA7,0xB2,
									0xBE,0xCD,0xDC,0xEB,
									0xFF};
//gamma = s2
unsigned char IPUGammaTableS2[34]= {0x00,0x00,0x00,0x01,
									0x02,0x05,0x07,0x0A,
									0x0D,0x11,0x17,0x1E,
									0x27,0x31,0x3C,0x48,
									0x59,0x6E,0x82,0x96,
									0xA5,0xB4,0xC3,0xCF,
									0xD7,0xDE,0xE4,0xE8,
									0xEC,0xF0,0xF5,0xFA,
									0xFF};
//*/
//example : Ipu_SetYGammaEnable(1, 0);
int Ipu_SetYGammaEnable(BOOL GammaEnable,BOOL SelLevel ) 
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;	
	ipu->Ipu_reg_31 |= ((DWORD)GammaEnable<<24)|((DWORD)SelLevel<<16);
	MP_DEBUG("ipu->Ipu_reg_31=0x%08x", ipu->Ipu_reg_31);
	return  NO_ERR;
}


//example : Ipu_SetYGammaTable(IPUGammaTableS2);
int Ipu_SetYGammaTable(DWORD *GammaValue)
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;	

	int k;
	for( k = 0 ; k < 8 ; k++ )		
		{
		*((DWORD*)(&ipu->Ipu_reg_32 + k))  = ((GammaValue[k*4]<<24)|(GammaValue[k*4+1]<<16)|(GammaValue[k*4+2]<<8)|(GammaValue[k*4+3])); 		 
		//mpDebugPrint("*((DWORD*)(&ipu->Ipu_reg_32 + k))  =0x%08x",*((DWORD*)(&ipu->Ipu_reg_32 + k))  );
		}
	ipu->Ipu_reg_3A = GammaValue[32]<<24;
	
	#if 0 //def mic_debug	
	mpDebugPrint("ipu->Ipu_reg_32=0x%08x", ipu->Ipu_reg_32);
	mpDebugPrint("ipu->Ipu_reg_33=0x%08x", ipu->Ipu_reg_33);
	mpDebugPrint("ipu->Ipu_reg_34=0x%08x", ipu->Ipu_reg_34);
	mpDebugPrint("ipu->Ipu_reg_35=0x%08x", ipu->Ipu_reg_35);
	mpDebugPrint("ipu->Ipu_reg_36=0x%08x", ipu->Ipu_reg_36);
	mpDebugPrint("ipu->Ipu_reg_37=0x%08x", ipu->Ipu_reg_37);
	mpDebugPrint("ipu->Ipu_reg_38=0x%08x", ipu->Ipu_reg_38);
	mpDebugPrint("ipu->Ipu_reg_39=0x%08x", ipu->Ipu_reg_39);
	mpDebugPrint("ipu->Ipu_reg_3A=0x%08x", ipu->Ipu_reg_3A);
	#endif	
	return  NO_ERR;
}
#endif



