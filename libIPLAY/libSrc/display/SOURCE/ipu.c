
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"

#include "idu.h"
#include "ipu.h"
#include "timer.h"

#include "taskid.h"


//***********************************************************************
//                              Variable
//***********************************************************************
ST_IMAGEINFO src, trg;

void TurnOnIPUClk()
{
	register CLOCK *clock;

	clock = (CLOCK *) (CLOCK_BASE);
	clock->MdClken |= 0x00001000L;
}


void TurnOffIPUClk()
{
	register CLOCK *clock;

	clock = (CLOCK *) (CLOCK_BASE);
	clock->MdClken &= 0xffffefffL;
}

void TurnOffIpuScalerClk()
{
	register CLOCK *clock;

	clock = (CLOCK *) (CLOCK_BASE);
	clock->MdClken &= ~BIT6;	//disable ipu scaler pixel clock
}


///
///@ingroup      SCALING
///@brief          wait HW scaling complete
///
///@param       dwTimeOutCount time out counter
///
///@retval        if transfer success return TRUE else FALSE
///

#if MJPEG_ENABLE
extern int MJPEG;
#endif

int inline Ipu_WaitComplete(DWORD dwTimeOutCount){
    volatile register IPU *ipu;    
    DWORD dwStartTime = SystemGetTimeStamp();
    
    ipu = (IPU *)IPU_BASE;
 
    while (1) {
//    while (dwTimeOutCount > SystemGetElapsedTime(dwStartTime)) {
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
        if(ipu->Ipu_reg_1C & 0x00010000)
#else
        if ((ipu->IpIc & 0x1))
#endif        
            return TRUE;
        #if MJPEG_ENABLE
		if (!MJPEG)    
		#endif	
		{
			TaskYield();
		}
    }
    return FALSE;
}


int Ipu_CheckComplete(){
 
	CLOCK *clock = (CLOCK *) CLOCK_BASE;
	if (!(clock->MdClken & BIT6)) return TRUE;
    
    IPU *ipu = (IPU *)IPU_BASE;
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
    if(ipu->Ipu_reg_1C & 0x00010000)
#else
    if ((ipu->IpIc & 0x1))
#endif 
    {
		  clock->MdClken &= ~BIT6;	//disable ipu scaler pixel clock
        return TRUE;
    }

    return FALSE;
}


#if 0//(((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
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

		ipu->Ipu_reg_3E = 0x00010100; //SP->CS


		//CbCrGain
		//ipu->Ipu_reg_3C=0xC0C00000;//CbCrGain  3X
		//RegisterCode=TotalCbGainx256*64
		ipu->Ipu_reg_3C=((TotalCbGainx256>>2)<<24)+((TotalCrGainx256>>2)<<16);


ClearHueSATflag();		
//IPUCbCrSwitch=0;
}

/**
 * @ingroup SpecialEffect
 * @brief   Sketch
 * 
 *
 */ 
void Ipu_Sketch()
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


#endif

