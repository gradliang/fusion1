/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "taskid.h"
#include "ipu.h"

#define FIX_POINT(a) ((a) << 8)
#define FIX_POINT_R(a) ((a) >> 8)

extern BYTE overlay_enable;
int sce=0;
extern int now,ok;

static BYTE g_bOverlay_Use_Fillwall;
static ST_OVERLAY_PARAM * g_Overlay_param;
static ST_IMGWIN * g_SubWin;
void Set_Bypass_Overlay_Param(ST_IMGWIN * srcSubWin, ST_OVERLAY_PARAM * sOverlay_param)
{
	g_SubWin = srcSubWin;
	g_Overlay_param = sOverlay_param;
}
void Clear_Bypass_Overlay_Param()
{
	g_SubWin = NULL;
}


#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
#define IPU_650_MAX_SCALE_TARGET_SIZE 800
#define IPU_650_TRG_EXTRA_POINT	2
#define TOTAL_IPU_650_TRG_EXTRA_POINT 	(IPU_650_TRG_EXTRA_POINT * 2)

typedef struct
{
	ST_SCA_PARM sScaParm;
	WORD wSourceImageWidth;
	WORD wSourceImageHeight;
	WORD wSourceFilterWidth;
	WORD wSourceFilterHeight;
	WORD wSourceOutterWidth;	//align to IPR limit (2 points)

	WORD wTargetWidth;
	WORD wTargetHeight;

	WORD wTargetImageWidth;
	WORD wTargetImageHeight;

	WORD wOverlayStartX;
	WORD wOverlayStartY;

	BYTE wIPR_Drop_First_Point;
	BYTE bNeedInitValue;
	BYTE bMPV;
} SCALER_PARA, * PSCALER_PARA;

SCALER_PARA g_stScalerPara;
PSCALER_PARA g_pstScalerPara = &g_stScalerPara ;

PSCALER_PARA GetScalerPara()
{
	return g_pstScalerPara;
}

#if VIDEO_ON
static int volatile video_frame_cnt = 0 ;
int Get_Video_Frame_Cnt()
{

	IODelay(5);
	return video_frame_cnt ;
}

void Reset_Video_Frame_Cnt()
{
	video_frame_cnt = 0 ;
}
#endif


int MP650_FPGA_Change_DMA_Priority_IPU(void)
{
	DMA* dma = (DMA*)DMA_BASE ;
	dma->FDMACTL_EXT1 = 0x01033F01 ; //wait cycles: 3F
//	dma->FDMACTL_EXT1 = 0x01030001; //wait cycles: 0
}


// MP650 subsample ratio 1/2, 1/3, 1/4, 1/5, 1/6, 1/7, 1/8
/*
static BYTE GetSubSampleRatio_650(register WORD wSrc, register WORD wTrg)
{
	register BYTE i;
	if (wSrc > wTrg)
	{
		if ((wTrg * 8) <= wSrc)
			return 7 ;
		if ((wTrg<<1) > wSrc)
			return 0 ;
		for (i = 2; i < 9; i++)
		{
			if ((wTrg * (i+1)) >= wSrc)
			{
				if( (wTrg * (i+1)) ==  wSrc)
					return i ;
				else
				return (i-1);
			}
		}
	}
	return 0;
}
*/
static BYTE GetSubSampleRatio_650(register WORD wSrc, register WORD wTrg)
{
	register BYTE i;
	if (wSrc > wTrg)
	{
		if ((wTrg * 8) <= wSrc)
			return 7 ;
		else if((wTrg * 4) <= wSrc)
			return 3 ;
		else if ((wTrg * 2) <= wSrc)
			return 1 ;
		else
			return 0 ;
	}
	return 0;
}

//MP650 calculate Scaling Factor
static WORD GetSF_650(DWORD dwSrc, DWORD dwTrg, BYTE H_or_V)
{
	PSCALER_PARA pSC = GetScalerPara();
	ST_SCA_PARM *psScaParm;
	psScaParm = &pSC->sScaParm;

	WORD SF ;
	if (H_or_V==0)
		dwSrc = dwSrc/(psScaParm->wHSubRatio+1);
	else if (H_or_V==1)
		dwSrc = dwSrc/(psScaParm->wVSubRatio+1);

	if (dwSrc%dwTrg == 0)
		SF = 0 ;
	else
	{
		if (H_or_V==1)
		{
			SF = (((dwSrc-1) << 15) / (dwTrg-1)) ;
		}
		else
			SF = (((dwSrc-1) << 15) / (dwTrg-1)) ;
	}
	return SF ;
}

//MP650 Initial Offset Factor for Multi-part Scaling
static WORD Get_Offset_SF_650(DWORD dwSrc, DWORD dwTrg)
{
	WORD SF ;
	DWORD tmp ;
	PSCALER_PARA pSC = GetScalerPara();
	ST_SCA_PARM *psScaParm;
	psScaParm = &pSC->sScaParm;

	MP_DEBUG("Get_Offset_SF_650 dwSrc=%d, dwTrg=%d", dwSrc, dwTrg);
	dwSrc = dwSrc>>psScaParm->wHSubRatio ;
	tmp = (((dwSrc-1)*IPU_650_MAX_SCALE_TARGET_SIZE<<15)/(dwTrg-1));
	MP_DEBUG("tmp=%d", tmp);
	tmp = tmp>>15 ;
	MP_DEBUG("tmp=%d", tmp);
	//SF = ((((dwSrc-1)*IPU_650_MAX_SCALE_TARGET_SIZE)<<15)/(dwTrg-1)) - (tmp<<15) ;
	SF = ((dwSrc-1)<<15)/(dwTrg-1) ;
	MP_DEBUG("Offset SF=%d", SF);
	SF = SF*IPU_650_MAX_SCALE_TARGET_SIZE - (tmp<<15) ;
	MP_DEBUG("Offset SF=%d", SF);
	return SF ;
}

void ipu_reset()
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;
	BIU *biu;
	biu = (BIU *) BIU_BASE;
	CHANNEL *dmaR, *dmaW, *IDUdma;
	dmaW = (CHANNEL *) DMA_SCW_BASE;
	dmaR = (CHANNEL *) DMA_SCR_BASE;

	IODelay(1);
	dmaR->Control = 0x00000000;
	dmaW->Control = 0x00000000;
	// asynchronous IPU reset
	biu->BiuArst &= 0xfffffffb;
	IODelay(1);
	biu->BiuArst |= 0x00000004;
	SystemIntDis(IM_IPU);

	//Print_IPU_Register();
	MP_DEBUG("IPU Reset");
}

static DWORD GetDMA_Width(WORD Srcwd, WORD flag)
{
	DWORD align_width = Srcwd ;
	MP_DEBUG("GetDMA_Width flag=%d", flag);
	if (flag == 1) // 444
		align_width = ALIGN_8(Srcwd) ;
	else if (flag == 2) // 422
		align_width = ALIGN_16(Srcwd) ;
	else if (flag == 3) // 420
		align_width = ALIGN_16(Srcwd) ;
	else if (flag == 5) // 42V
		align_width = ALIGN_8(Srcwd) ;
	else if (flag == 4) // MPV
	{
		if (Srcwd >= 720)
			align_width = 1024 ;
	}

	return align_width ;
}

static WORD GetTrgStartPoint_NonFilter(WORD run, WORD Max_Trg_Width)
{
	DWORD	point ;
	if (run == 0)
		return 0 ;
	else
		point = (Max_Trg_Width * run) ;
	return point ;
}

static WORD GetTrgStartPoint(WORD run, WORD Max_Trg_Width)
{
	DWORD	point ;
	if (run == 0)
		return 0 ;
	else
		//point = ((Max_Trg_Width * run) - (IPU_650_TRG_EXTRA_POINT * 2 * run)) ;
		//point = ((Max_Trg_Width * run) - run) ;
		point = (Max_Trg_Width * run) ;
	return point ;
}

static WORD GetSrcStartPoint(WORD SrcWidth, WORD TrgWidth, WORD Trg_point)
{
	DWORD point, tmp ;
	PSCALER_PARA pSC = GetScalerPara();
	ST_SCA_PARM *psScaParm;

	psScaParm = &pSC->sScaParm;

	if (psScaParm->wHSubRatio != 0)
	{
		tmp = ((((SrcWidth>>psScaParm->wHSubRatio)-1)<<15)/(TrgWidth-1)*(Trg_point)) ;
		point = (tmp>>15);
		point = point <<psScaParm->wHSubRatio ;
	}
	else
	{
		tmp = (((SrcWidth-1)<<15)/(TrgWidth-1)*(Trg_point)) ;
		point = (tmp>>15);
	}

	return point ;
}

static WORD GetSrcEndPoint(WORD SrcWidth, WORD TrgWidth, WORD Trg_point)
{
	DWORD tmp = 0 ;
	WORD point = 0 ;
	PSCALER_PARA pSC = GetScalerPara();
	ST_SCA_PARM *psScaParm;

	MP_DEBUG("SrcWidth=%d, TrgWidth=%d, Trg_point=%d", SrcWidth, TrgWidth, Trg_point);

	psScaParm = &pSC->sScaParm;
	tmp = ((((SrcWidth>>psScaParm->wHSubRatio)-1)*(Trg_point))*100/(TrgWidth-1))<<psScaParm->wHSubRatio ;
	if (tmp%100)
		point = (tmp/100) + 1 ;
	else
		point = (tmp/100) ;

	return point ;
}

static int Get650ScaParm(ST_SCA_PARM * psScaParm, WORD SrcWidth, WORD SrcHeight, WORD TrgWidth, WORD TrgHeight)
{
	psScaParm->wHUp = 0;
	psScaParm->wVUp = 0;
	psScaParm->wHSubRatio = GetSubSampleRatio_650(SrcWidth, TrgWidth);
	psScaParm->wVSubRatio = GetSubSampleRatio_650(SrcHeight, TrgHeight);
	psScaParm->wHSF = GetSF_650(SrcWidth, TrgWidth, 0);
	psScaParm->wVSF = GetSF_650(SrcHeight, TrgHeight, 1);
	if (SrcWidth < TrgWidth)
		psScaParm->wHUp = 1 ;
	if (SrcHeight < TrgHeight)
		psScaParm->wVUp = 1 ;
	MP_DEBUG("psScaParm  wHSF %d wHSubRatio %d  wVSF %d wVSubRatio %d",
	         psScaParm->wHSF,
	         psScaParm->wHSubRatio,
	         psScaParm->wVSF,
	         psScaParm->wVSubRatio);
}



extern volatile int recordTaskState;
void ScalerIsr()
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;
	INTERRUPT *isr = (INTERRUPT *) INT_BASE;
	DWORD val = 0 ;
	
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650)||((CHIP_VER & 0xffff0000) == CHIP_VER_660))

#if IPW_FAST_MODE
	//--BIT6->IPW2  BIT7->IPW1
	if (ipu->Ipu_reg_1C & 0x000000c0)
	{
		val=ipu->Ipu_reg_1C;
		ipu->Ipu_reg_1C = val & 0xffffffc0;
		//stop IPW1 IPW2
		ipu->Ipu_reg_F0 |= 0x000000c0; 
		if (recordTaskState)
				ProcIpwIsr(val);
	}
#else
#if (SENSOR_ENABLE == ENABLE)
	//extern BYTE Sensor_Mode;
	//if(Sensor_Mode)
	{

/*******
ipu->Ipu_reg_F0 = 0x20000000|Overlay|Iprfrom|Ssclfrom;

IPW1:
	ipu->Ipu_reg_A2	= 0x00000000;//((DWORD) psSource->dwPointer| 0xA0000000); // IPR  input DMA addr
	ipu->Ipu_reg_1A = (psSource->dwOffset - (pSC->wSourceOutterWidth<<1)); //IPR1 offset

	ipu->Ipu_reg_A3	= ((DWORD) SensorInWin[0].pdwStart| 0xA0000000); //IPW output DMA saddr (in byte unit)
	ipu->Ipu_reg_102 = ((SensorInputHeight-1)<<16)+(SensorInputWidth-1) ;// Height & Width
	ipu->Ipu_reg_1B = (win_sensor->wWidth - SensorWindow_Width)<<1; //IPW1 offset

    ipu->Ipu_reg_21 = (vsub_ratio<<8) | hsub_ratio;
    ipu->Ipu_reg_22 = width_ratio  << 16;
    ipu->Ipu_reg_23 = height_ratio << 16;

IPW2:
	ipu->Ipu_reg_F2 = ((DWORD) win_sensor->pdwStart| 0xA0000000);	//IPW2 DMA saddr (in byte unit)  address start
	ipu->Ipu_reg_F6 = 480-1 ;//H
	ipu->Ipu_reg_F7 = 800-1 ;	//W
	ipu->Ipu_reg_F1 = (PanelW - 800)<<1; // offset

	ipu->Ipu_reg_F4 = (1<<31)+(height_ratio<<16)+width_ratio ;//scaling

*/
		//mpDebugPrintN(" %08x ", ipu->Ipu_reg_1C); // 0000c0cc  0000c04c 

		// IPW2		Bit 1
		if (ipu->Ipu_reg_1C & BIT6)
		{
			//mpDebugPrint("1 ipu->Ipu_reg_1C=0x%08x", ipu->Ipu_reg_1C);
			val = ipu->Ipu_reg_1C & 0xffffff00 ;
			val |= 0x00000040 ;
			ipu->Ipu_reg_1C = val ;			
			//mpDebugPrint("W2");

#if 0//(PRODUCT_FUNC==FUN_OC)
			static DWORD st_dwIpw2Cnt=0;
			static DWORD dwStartTime = 0;

			if ((SystemGetElapsedTime(dwStartTime)>1000))
			{
				dwStartTime = GetSysTime();
				mpDebugPrint("IPW2:%d",st_dwIpw2Cnt);
				st_dwIpw2Cnt=0;
			}
			st_dwIpw2Cnt++;
#endif


		ipu->Ipu_reg_F0 |= BIT7;/*Close IPW2 write path*/
#if 0//(USE_IPW1_DISPLAY_MOTHOD == ENABLE) 
		EventSet(SENSOR_IPW_FRAME_END_EVENT_ID, BIT0); //RecordVideo
#else
		EventSet(SENSOR_IPW_FRAME_END_EVENT_ID, BIT1);//display
#endif

		}

		// IPW1		Bit0
		if (ipu->Ipu_reg_1C & BIT7)
		{
			val = ipu->Ipu_reg_1C & 0xffffff00 ;
			val |= 0x00000080 ;
			ipu->Ipu_reg_1C = val ;

			//mpDebugPrint("W1");
#if IPW1_DISABLE
			ipu->Ipu_reg_F0 |= BIT6;/*Close IPW1 write path*/						  
			mpDebugPrint("IPW1-Close");

#else

			if(overlay_enable)
					ipu->Ipu_reg_10 |= 0x01000000 ;//overlay
			ipu->Ipu_reg_F0 |= BIT6;/*Close IPW1 write path*/						  
	#if 0//(USE_IPW1_DISPLAY_MOTHOD == ENABLE) 
			EventSet(SENSOR_IPW_FRAME_END_EVENT_ID, BIT1); //display
	#else
			EventSet(SENSOR_IPW_FRAME_END_EVENT_ID, BIT0);
	#endif
#endif

		}
  
		EventSet(SCALING_FINISH_EVENT_ID, BIT0);
		return;

	}
#endif
#endif



#if VIDEO_ON
	video_frame_cnt = video_frame_cnt + 1 ;
	MP_DEBUG("video_frame_cnt=%d", video_frame_cnt);
#endif
	ipu->Ipu_reg_1C &= ~0x10000000 ;
	ipu->Ipu_reg_1C |= 0x10000000 ;

#endif

	EventSet(SCALING_FINISH_EVENT_ID, BIT0);

}

void SetScaleForZoom(WORD SrcWidth, WORD SrcHeight, WORD TrgWidth, WORD TrgHeight)
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;
	ST_SCA_PARM *psScaParm,sScaParm;

mpDebugPrint("------- SetScaleForZoom : src %d*%d ->trg %d,%d",SrcWidth,SrcHeight,TrgWidth,TrgHeight);
	psScaParm = &sScaParm;
	Get650ScaParm(psScaParm, SrcWidth, SrcHeight, TrgWidth, TrgHeight);
		// IPU Subsample
	ipu->Ipu_reg_21 = (psScaParm->wVSubRatio << 8) + psScaParm->wHSubRatio ;
	// IPU Scaling factor
	if (psScaParm->wHSF == 0)
		ipu->Ipu_reg_22 = 0x80000000 ;
	else
		ipu->Ipu_reg_22 = (psScaParm->wHSF << 16) + 0;
	if (psScaParm->wVSF == 0)
		ipu->Ipu_reg_23 = 0x80000000 ;
	else
		ipu->Ipu_reg_23 = (psScaParm->wVSF << 16) ;

}

static int ipu_scale_650_nonCsc(ST_IMAGEINFO * psSource, ST_IMAGEINFO * psTarget, WORD type_flag, BYTE filter_flag)
{
	IPU *ipu;
	CHANNEL *dmaR, *dmaW;
	BIU *biu;
	CLOCK *clock;
	CHANNEL *dma;
	IMAGEEFFECT Value;
	register IDU *idu = (IDU *) IDU_BASE;
	dma = (CHANNEL *) (DMA_IDU_BASE);
	clock = (CLOCK *) CLOCK_BASE;
	ipu = (IPU *) IPU_BASE;
	dmaR = (CHANNEL *) DMA_SCR_BASE;
	dmaW = (CHANNEL *) DMA_SCW_BASE;

	ST_SCA_PARM *psScaParm;
	PSCALER_PARA pSC = GetScalerPara();
	psScaParm = &pSC->sScaParm;
	WORD H_Offset_SF = 0 ;
	DWORD FinishEvent;
	int ret = FAIL, t = 0;
	int i,j ;

	mmcp_memset(&Value,0,sizeof(IMAGEEFFECT));
	//memset(&Value,0,sizeof(IMAGEEFFECT));
	if (pSC->bNeedInitValue == 1)
	{
		if (filter_flag)
			H_Offset_SF = Get_Offset_SF_650((pSC->wSourceImageWidth + TOTAL_IPU_650_TRG_EXTRA_POINT), (pSC->wTargetImageWidth + TOTAL_IPU_650_TRG_EXTRA_POINT));
		else
			H_Offset_SF = Get_Offset_SF_650(pSC->wSourceImageWidth, pSC->wTargetImageWidth);
	}

	ipu_reset();
	EventClear(SCALING_FINISH_EVENT_ID, ~BIT0);


	MP_DEBUG("Img_GetCDU_DecodeWidth=%d", Img_GetCDU_DecodeWidth());
	//IPR DMA
	ipu->Ipu_reg_A2	= ((DWORD) psSource->dwPointer| 0xA0000000);
	if (type_flag==444)
	{
#if	YUV444_ENABLE
        // TY Miao
		ipu->Ipu_reg_1A = (psSource->dwOffset - (pSC->wSourceOutterWidth*3));
#else
		ipu->Ipu_reg_1A = (psSource->dwOffset - (pSC->wSourceOutterWidth<<1));
#endif
	}
	else
	{
		ipu->Ipu_reg_1A = (psSource->dwOffset - (pSC->wSourceOutterWidth<<1));
	}
	if (pSC->bMPV) // MPV mode
	{
		ipu->Ipu_reg_1A = 0 ;
		ipu->Ipu_reg_A0 = ipu->Ipu_reg_A2 + (ALIGN_16(psSource->wWidth) * ALIGN_16(psSource->wHeight));
		ipu->Ipu_reg_A1 = 0 ;
	}

	//IPW DMA
	ipu->Ipu_reg_A3	= ((DWORD) psTarget->dwPointer| 0xA0000000);
	if (type_flag==444)
#if	YUV444_ENABLE
		ipu->Ipu_reg_1B = ((psTarget->wWidth - pSC->wTargetWidth)*3);
#else
		ipu->Ipu_reg_1B = ((psTarget->wWidth - pSC->wTargetWidth)<<1);
#endif
	else
		ipu->Ipu_reg_1B = ((psTarget->wWidth - pSC->wTargetWidth)<<1);
	MP_DEBUG("psTarget->wWidth=%d, pSC->wTargetWidth=%d", psTarget->wWidth, pSC->wTargetWidth);
	// Fill IPU Registers ++
	// IPR
	if (pSC->bMPV) // MPV mode
	{
		ipu->Ipu_reg_11 = (((ALIGN_16(pSC->wSourceFilterWidth) - 1) & 0xfff) << 16) + ((pSC->wSourceFilterHeight - 1) & 0xfff);
	}
	else
	{
		/*if(pSC->wIPR_Drop_First_Point == 0)
			ipu->Ipu_reg_11 = (((pSC->wSourceOutterWidth - 1) & 0xfff) << 16) + ((pSC->wSourceFilterHeight - 1) & 0xfff);
		else
			ipu->Ipu_reg_11 = (((pSC->wSourceFilterWidth - 1) & 0xfff) << 16) + ((pSC->wSourceFilterHeight - 1) & 0xfff);
		*/
		//polun modify MP650/MP660 support Width 16bit
		if(pSC->wIPR_Drop_First_Point == 0)
			ipu->Ipu_reg_11 = ((pSC->wSourceOutterWidth - 1) << 16) + (pSC->wSourceFilterHeight - 1);
		else
			ipu->Ipu_reg_11 = ((pSC->wSourceFilterWidth - 1) << 16) + (pSC->wSourceFilterHeight - 1);
	}
	ipu->Ipu_reg_10 &= 0xfffffcff ; // Clear Bit8 and Bit9
	ipu->Ipu_reg_10 |= (pSC->wIPR_Drop_First_Point<<8) ;
	// IPU DMA OUT Direction
	ipu->Ipu_reg_12 = 0x01000000 ;  //to dram
	// IPU DMA OUT Format
	if (type_flag == 444) // Jpeg 444 format
	{
#if YUV444_ENABLE
		ipu->Ipu_reg_12 |= 0x00010000 ;	//444 format out
#endif
	}
	// Window before IPW
	ipu->Ipu_reg_14 = 0x01000000 ;	//Enable bit 24
	if (filter_flag)
	{
		ipu->Ipu_reg_15 = (IPU_650_TRG_EXTRA_POINT << 16) + (pSC->wTargetHeight + IPU_650_TRG_EXTRA_POINT - 1) ;
		ipu->Ipu_reg_16 = (IPU_650_TRG_EXTRA_POINT << 16) + (pSC->wTargetWidth + IPU_650_TRG_EXTRA_POINT - 1) ;
	}
	else	//non_filter
	{
		ipu->Ipu_reg_15 = (0 << 16) + (pSC->wTargetHeight - 1) ;
		ipu->Ipu_reg_16 = (0 << 16) + (pSC->wTargetWidth - 1) ;
	}

	// Scaling path:IPR->SCL->YUV->IPW
	ipu->Ipu_reg_18 = 0x00070207 ;
	ipu->Ipu_reg_19 = 0x07030000 ;

	// Turn on IPU Frame End
#if ((CHIP_VER & 0xffff0000) == CHIP_VER_660)
	ipu->Ipu_reg_F0 |= 0x20000000 ;
	ipu->Ipu_reg_1C = 0x00008000 ;
#elif (((CHIP_VER & 0xffff0000) == CHIP_VER_650))
	ipu->Ipu_reg_1C = 0x11000000 ;
#endif
	SystemIntEna(IM_IPU);
	// IPU Subsample
	ipu->Ipu_reg_21 = (psScaParm->wVSubRatio << 8) + psScaParm->wHSubRatio ;
	// IPU Scaling factor
	if (psScaParm->wHSF == 0)
		ipu->Ipu_reg_22 = 0x80000000 ;
	else
		ipu->Ipu_reg_22 = (psScaParm->wHSF << 16) + H_Offset_SF;
	if (psScaParm->wVSF == 0)
		ipu->Ipu_reg_23 = 0x80000000 ;
	else
		ipu->Ipu_reg_23 = (psScaParm->wVSF << 16) ;
	// Edge processing and S/C value
	//ipu->Ipu_reg_26 = 0x03000000 ;
	//ipu->Ipu_reg_27 = 0x00020202 ;
	// HBIL_back
	if (psScaParm->wHUp)		// Scaling up
		ipu->Ipu_reg_20 = 0x00000001 ;
	//for pid....
	#if DEMO_PID
	ipu->Ipu_reg_20 |= 0x00000002 ;		//double line buffer
	//double line buffer need to disable filter
	ipu->Ipu_reg_26 |= 0x03000001 ;
	ipu->Ipu_reg_27 |= 0x01000000 ;
	#endif
	//for pid....
	// IPR Source Type
	MP_DEBUG("type_flag=%d", type_flag);
	if (type_flag == 444) // Jpeg 444 format
	{
		mpDebugPrint("This photo is 444 type");
		ipu->Ipu_reg_10 &= 0xfffcffff ;
#if YUV444_ENABLE
		ipu->Ipu_reg_10 |= 0x00020000 ;
#else
		ipu->Ipu_reg_10 |= 0x00010000 ;
#endif
	}
	else if (pSC->bMPV) //MPV mode
	{
		MP_DEBUG("MPV Mode scale to DRAM");
		ipu->Ipu_reg_10 &= 0xfffcffff ;
		ipu->Ipu_reg_10 |= 0x00104000 ; //Use PP line buffer
	}
	else
		ipu->Ipu_reg_10 |= 0x00010000 ;

	//Print_IPU_Register();
	// Enable IPU clock and DMA
	clock->MdClken |= BIT6;
	clock->MdClken |= BIT5;
	clock->MdClken |= BIT7;
	dmaR->Control = 0x00000001;
	dmaW->Control = 0x00000001;
	// Trigger IPU
	ipu->Ipu_reg_10 |= 0x01000000 ;
	// Fill IPU Registers --
	IODelay(3);
	//Wait scaling finish event
	ret = EventWaitWithTO(SCALING_FINISH_EVENT_ID, BIT0, OS_EVENT_OR, &FinishEvent, 8000);
	if (ret)
		MP_ALERT("Scaling Time out");
#if 0
	// Polling IPU Finish
	while (t < MAX_IPU_WAIT_TIME)
	{
		if (ipu->Ipu_reg_1C & 0x00010000)
		{
			MP_DEBUG("650 IPU Scaling Done");
			ret = 0 ;
			break;
		}
		t++;
	}
	MP_DEBUG("Time t==%d", t);
	if (t >= MAX_IPU_WAIT_TIME)
	{
		ret = FAIL ;
		mpDebugPrint("Scaling Time out");
	}
#endif
	IODelay(3);
	return ret ;
}




static int ipu_scale_650(ST_IMAGEINFO * psSource, ST_IMAGEINFO * psTarget, WORD type_flag, BYTE filter_flag)
{
	IPU *ipu;
	CHANNEL *dmaR, *dmaW;
	BIU *biu;
	CLOCK *clock;
	CHANNEL *dma;
	IMAGEEFFECT Value;
	register IDU *idu = (IDU *) IDU_BASE;
	dma = (CHANNEL *) (DMA_IDU_BASE);
	clock = (CLOCK *) CLOCK_BASE;
	ipu = (IPU *) IPU_BASE;
	dmaR = (CHANNEL *) DMA_SCR_BASE;
	dmaW = (CHANNEL *) DMA_SCW_BASE;

	ST_SCA_PARM *psScaParm;
	PSCALER_PARA pSC = GetScalerPara();
	psScaParm = &pSC->sScaParm;
	WORD H_Offset_SF = 0 ;
	DWORD FinishEvent;
	int ret = FAIL, t = 0;
	int i,j ;

	mmcp_memset(&Value,0,sizeof(IMAGEEFFECT));
	//memset(&Value,0,sizeof(IMAGEEFFECT));
	if (pSC->bNeedInitValue == 1)
	{
		if (filter_flag)
			H_Offset_SF = Get_Offset_SF_650((pSC->wSourceImageWidth + TOTAL_IPU_650_TRG_EXTRA_POINT), (pSC->wTargetImageWidth + TOTAL_IPU_650_TRG_EXTRA_POINT));
		else
			H_Offset_SF = Get_Offset_SF_650(pSC->wSourceImageWidth, pSC->wTargetImageWidth);
	}

	ipu_reset();
	EventClear(SCALING_FINISH_EVENT_ID, ~BIT0);


	MP_DEBUG("Img_GetCDU_DecodeWidth=%d", Img_GetCDU_DecodeWidth());
	//IPR DMA
	ipu->Ipu_reg_A2	= ((DWORD) psSource->dwPointer| 0xA0000000);
	if (type_flag==444)
	{
#if	YUV444_ENABLE
        // TY Miao
		ipu->Ipu_reg_1A = (psSource->dwOffset - (pSC->wSourceOutterWidth*3));
#else
		ipu->Ipu_reg_1A = (psSource->dwOffset - (pSC->wSourceOutterWidth<<1));
#endif
	}
	else
	{
		ipu->Ipu_reg_1A = (psSource->dwOffset - (pSC->wSourceOutterWidth<<1));
	}
	if (pSC->bMPV) // MPV mode
	{
		//ipu->Ipu_reg_1A = 0 ;
		ipu->Ipu_reg_1A = ((psSource->dwOffset - (ALIGN_16(pSC->wSourceFilterWidth)<<1))/2);
		ipu->Ipu_reg_A0 = ipu->Ipu_reg_A2 + (ALIGN_16(psSource->wWidth) * ALIGN_16(psSource->wHeight));
		//ipu->Ipu_reg_A1 = 0 ;
		ipu->Ipu_reg_A1 = ((psSource->dwOffset - (ALIGN_16(pSC->wSourceFilterWidth)<<1))/2);
	}

	//IPW DMA
	ipu->Ipu_reg_A3	= ((DWORD) psTarget->dwPointer| 0xA0000000);
	if (type_flag==444)
#if	YUV444_ENABLE
		ipu->Ipu_reg_1B = ((psTarget->wWidth - pSC->wTargetWidth)*3);
#else
		//ipu->Ipu_reg_1B = ((psTarget->wWidth - pSC->wTargetWidth)<<1);
		ipu->Ipu_reg_1B = (psTarget->dwOffset-(pSC->wTargetWidth<<1));
#endif
	else
		//ipu->Ipu_reg_1B = ((psTarget->wWidth - pSC->wTargetWidth)<<1);
		ipu->Ipu_reg_1B = (psTarget->dwOffset-(pSC->wTargetWidth<<1));
	MP_DEBUG("psTarget->wWidth=%d, pSC->wTargetWidth=%d", psTarget->wWidth, pSC->wTargetWidth);
	// Fill IPU Registers ++
	// IPR
	if (pSC->bMPV) // MPV mode
	{
		ipu->Ipu_reg_11 = (((ALIGN_16(pSC->wSourceFilterWidth) - 1) & 0xfff) << 16) + ((pSC->wSourceFilterHeight - 1) & 0xfff);
	}
	else
	{
		/*if(pSC->wIPR_Drop_First_Point == 0)
			ipu->Ipu_reg_11 = (((pSC->wSourceOutterWidth - 1) & 0xfff) << 16) + ((pSC->wSourceFilterHeight - 1) & 0xfff);
		else
			ipu->Ipu_reg_11 = (((pSC->wSourceFilterWidth - 1) & 0xfff) << 16) + ((pSC->wSourceFilterHeight - 1) & 0xfff);
		*/
		//polun modify MP650/MP660 support Width 16bit
		if(pSC->wIPR_Drop_First_Point == 0)
			ipu->Ipu_reg_11 = ((pSC->wSourceOutterWidth - 1) << 16) + (pSC->wSourceFilterHeight - 1);
		else
			ipu->Ipu_reg_11 = ((pSC->wSourceFilterWidth - 1) << 16) + (pSC->wSourceFilterHeight - 1);

	}
	ipu->Ipu_reg_10 &= 0xfffffcff ; // Clear Bit8 and Bit9
	ipu->Ipu_reg_10 |= (pSC->wIPR_Drop_First_Point<<8) ;
	// IPU DMA OUT Direction
	ipu->Ipu_reg_12 = 0x01000000 ;  //to dram
	// IPU DMA OUT Format
	if (type_flag == 444) // Jpeg 444 format
	{
#if YUV444_ENABLE
		ipu->Ipu_reg_12 |= 0x00010000 ;	//444 format out
#endif
	}
	// Window before IPW
	ipu->Ipu_reg_14 = 0x01000000 ;	//Enable bit 24
	if (filter_flag)
	{
		ipu->Ipu_reg_15 = (IPU_650_TRG_EXTRA_POINT << 16) + (pSC->wTargetHeight + IPU_650_TRG_EXTRA_POINT - 1) ;
		ipu->Ipu_reg_16 = (IPU_650_TRG_EXTRA_POINT << 16) + (pSC->wTargetWidth + IPU_650_TRG_EXTRA_POINT - 1) ;
	}
	else	//non_filter
	{
		ipu->Ipu_reg_15 = (0 << 16) + (pSC->wTargetHeight - 1) ;
		ipu->Ipu_reg_16 = (0 << 16) + (pSC->wTargetWidth - 1) ;
	}
	// In the same Win & Non-filter
	// Scaling path:IPR->SCL->YUV->IPW
	//ipu->Ipu_reg_18 = 0x00070207 ;
	//ipu->Ipu_reg_19 = 0x07030000 ;
	{
	//ycbcr in->scl->csc0->RGBp->csc1->yuvp->ycbcr out TYChen Fix
	ipu->Ipu_reg_18 = 0x00010301 ;
	ipu->Ipu_reg_19 = 0x03030000 ;
	Ipu_CSC0Set();
	Ipu_CSC1Set();
	/*******************************Check Effect***********************************/
	mpGetIPUEffectValue(&Value);
	if( Value.Edge_Enable || Value.Blur_Enable||Value.Sketch_Enable||Value.BW_Enable
	||Value.Noise_Enable || Value.Bilevel_Enable || Value.Reverse_Enable||Value.CM_Enable
	||Value.Y_Offset_Enable||Value.Y_Gain_Enable ||Value.CbCrOffset_Enable||Value.SEHG_Enable
	||Value.Crayon_Enable)
	{
		if (Value.Edge_Enable)
			Ipu_SetEdgeFumc(2, ((Value.EdgeGain > 3) ? (Value.EdgeGain - 3 ):0), ((Value.EdgeGain > 3) ? 3 : (Value.EdgeGain & 0x03)));
		if (Value.Blur_Enable)
			Ipu_SetBlur(Value.BlurGain);
		if (Value.Sketch_Enable)
				Ipu_Sketch(Value.SketchMode);
		if (Value.BW_Enable)
		{
			Ipu_SetIpuGlobalEnable(1, 0, 0);
			Ipu_SetBlackWitheEffect( 1, 128, 128 , 1 , 1, 1);
		}
		if ( Value.Noise_Enable || Value.Bilevel_Enable || Value.Reverse_Enable)
		{
			Ipu_SetIpuGlobalEnable(1, 0, 0);
			Ipu_SetSEBIANEffect(Value.Noise_Enable, Value.NoiseGain, Value.Bilevel_Enable,
								Value.Bilevel_cbralsoGain, Value.BilevelGain ,Value.Reverse_Enable);//SEBIN2
		}

		if (Value.CM_Enable)
		{
			Ipu_SetIpuGlobalEnable(1, 0, 0);
			Ipu_SetColorMixEffect(1, 0, 0, 0, 0, Value.CM_Cb_Mode, Value.CM_Cr_Mode);// Cb = 127 ; Cr = -128	//CM5
		}

		if (Value.Y_Offset_Enable)//ok
		{
			Ipu_SetIpuGlobalEnable(1, 0, 0);
			Ipu_SetBrightness(Value.Y_Offset);
		}

		if (Value.Y_Gain_Enable)//ok
		{
			Ipu_SetIpuGlobalEnable(1, 0, 0);
			Ipu_SetContrast(Value.Y_Gain,0);
		}

		if (Value.CbCrOffset_Enable)//ok
		{
			Ipu_SetIpuGlobalEnable(1, 0, 0);
			Ipu_SetCbCrOffset(Value.Cb_Offset,Value.Cr_Offset);
		}

		if (Value.SEHG_Enable)
		{
			Ipu_SetIpuGlobalEnable(1, 0, 0);
			Ipu_SetHighGainEffect(Value.SEHG_Enable, Value.SEHG_Offset, Value.SEHG_Gain, 0);
		}

		if (Value.Crayon_Enable)
		{
			Ipu_Crayon();
		}

		if (Value.IpuGamma_Enable)//ok
		{
			Ipu_SetIpuGlobalEnable(1, 0, 0);//SE=>SP
			Ipu_SetYGammaEnable(Value.IpuGamma_Enable,Value.IpuGamma_Step);
			Ipu_SetYGammaTable(&Value.IpuGammaValue);
		}
	}
	else
	{
		/*******************************Check Effect***********************************/
  //TYChen add:IPU Clock about SP->CS(MDCLKEN),must set MDCLKEN[7]=1,IPU_REG_58 is normal
	static int Ipu_reg_58_RegisterDefaultFlag=0;
	if(Ipu_reg_58_RegisterDefaultFlag==0)
	{
			if(clock->MdClken&BIT7)
			{
					for(i=0;i<32;i++)
					  for(j=0;j<32;j++)
					{
					//mpDebugPrint("addr=0x%x=0x%x",i*32+j,ipu->Ipu_reg_58);
					  	ipu->Ipu_reg_58=((i*32+j)|0x0000)<<16;
					}
					Ipu_reg_58_RegisterDefaultFlag=1;
			}
	}

//	if (test285flag>0)
//		Ipu_RGBp_CS_Set();
/*
	if(HueSATflag>0)
	{
			ipu->Ipu_reg_3E = 0x00010100; //SP->CS must set CS register default value
			Ipu_SetHueSaturation(g_psSystemConfig->sScreenSetting.bHue-8,g_psSystemConfig->sScreenSetting.bSaturation-8);
	}
*/
	}
		}
	// Turn on IPU Frame End
#if ((CHIP_VER & 0xffff0000) == CHIP_VER_660)
	ipu->Ipu_reg_F0 |= 0x20000000 ;
	ipu->Ipu_reg_1C = 0x00008000 ;
#elif (((CHIP_VER & 0xffff0000) == CHIP_VER_650))
	ipu->Ipu_reg_1C = 0x11000000 ;
#endif
	SystemIntEna(IM_IPU);
	// IPU Subsample
	ipu->Ipu_reg_21 = (psScaParm->wVSubRatio << 8) + psScaParm->wHSubRatio ;
	// IPU Scaling factor
	if (psScaParm->wHSF == 0)
		ipu->Ipu_reg_22 = 0x80000000 ;
	else
		ipu->Ipu_reg_22 = (psScaParm->wHSF << 16) + H_Offset_SF;
	if (psScaParm->wVSF == 0)
		ipu->Ipu_reg_23 = 0x80000000 ;
	else
		ipu->Ipu_reg_23 = (psScaParm->wVSF << 16) ;
	// Edge processing and S/C value
	//ipu->Ipu_reg_26 = 0x03000000 ;
	//ipu->Ipu_reg_27 = 0x00020202 ;
	// HBIL_back
	if (psScaParm->wHUp)		// Scaling up
		ipu->Ipu_reg_20 = 0x00000001 ;
	//for pid....
	#if DEMO_PID
	ipu->Ipu_reg_20 |= 0x00000002 ;		//double line buffer
	//double line buffer need to disable filter
	ipu->Ipu_reg_26 |= 0x03000001 ;
	ipu->Ipu_reg_27 |= 0x01000000 ;
	#endif
	//for pid....
	// IPR Source Type
	MP_DEBUG("type_flag=%d", type_flag);
	// scaling timeout
	// scaling pixel 3x?
	if((ipu->Ipu_reg_11 & 0x0000FFFF) == 2)
	ipu->Ipu_reg_11 |= 0x00000003 ;
	// scaling pixel ?x2
	if((ipu->Ipu_reg_11 & 0xFFFF0000) == 0x00020000)
	ipu->Ipu_reg_11 |= 0x00030000 ;
	// scaling pixel 2x?
	if((ipu->Ipu_reg_11 & 0x0000FFFF) == 1)
	ipu->Ipu_reg_11 |= 0x00000003 ;
	// scaling pixel ?x2
	if((ipu->Ipu_reg_11 & 0xFFFF0000) == 0x00010000)
	ipu->Ipu_reg_11 |= 0x00030000 ;
	// scaling pixel 1x?
	if((ipu->Ipu_reg_11 & 0x0000FFFF) == 0)
	{
		ipu->Ipu_reg_11 |= 0x00000003 ;
		ipu->Ipu_reg_23 = 0x00010000 ;
	}
	// scaling pixel ?x2
	if((ipu->Ipu_reg_11 & 0xFFFF0000) == 0)
	{
		ipu->Ipu_reg_11 |= 0x00030000 ;
		ipu->Ipu_reg_22 = 0x00010000 ;
	}
    //scaling pixel timeout end
	if (type_flag == 444) // Jpeg 444 format
	{
		mpDebugPrint("This photo is 444 type");
		ipu->Ipu_reg_10 &= 0xfffcffff ;
#if YUV444_ENABLE
		ipu->Ipu_reg_10 |= 0x00020000 ;
#else
		ipu->Ipu_reg_10 |= 0x00010000 ;
#endif
	}
	else if (pSC->bMPV) //MPV mode
	{
		MP_DEBUG("MPV Mode scale to DRAM");
		ipu->Ipu_reg_10 &= 0xfffcffff ;
		ipu->Ipu_reg_10 |= 0x00104000 ; //Use PP line buffer
	}
	else
		ipu->Ipu_reg_10 |= 0x00010000 ;

#if 0
//--------------------------------------------------
//For IPU setting from PC tool, added by Vanessa+
//--------------------------------------------------
  #define SRAM_ADR        0xb8000000
  #define PC_TOOL_ADR    (0xb8000000+0x0000040)
  #define CbCr_32x32_ADR (0xb8000000+0x0000300)

  typedef struct{
      volatile DWORD tool_used;
      volatile DWORD tool_test;
  } TOOL;

  typedef struct{
      volatile DWORD reg_58;
  } REG_58;

  IPU *pc_tool_ipu;
  TOOL *tool;
  REG_58 *ptr_reg_58;
  DWORD tmp;

    //--- UVmax
    ipu->Ipu_reg_B0 = 0x00000000;
    ipu->Ipu_reg_B1 = 0x2a57e24d;
    ipu->Ipu_reg_B2 = 0xd6a91eb3;
    ipu->Ipu_reg_B3 = 0x05d9011f;
    ipu->Ipu_reg_B4 = 0x02ca016b;
    ipu->Ipu_reg_B5 = 0x01200895;
    ipu->Ipu_reg_B6 = 0x0358016b;
    ipu->Ipu_reg_B7 = 0x012005b6;
    ipu->Ipu_reg_B8 = 0x016d02c2;
    ipu->Ipu_reg_B9 = 0x08c50120;
    ipu->Ipu_reg_BA = 0x016d034c;

  tool = (TOOL *) SRAM_ADR;
 	pc_tool_ipu = (IPU *) PC_TOOL_ADR;

 	tmp = tool->tool_used & 0x80000000;
 	if(tmp == 0x80000000){
 		  //RGB Gains
 		  tmp = tool->tool_used & 0x00000001;
 		  if(tmp) 	ipu->Ipu_reg_82 = pc_tool_ipu->Ipu_reg_82;

      //HSL
 		  tmp = tool->tool_used & 0x00000002;
 		  if(tmp){
 		  	ipu->Ipu_reg_60 = pc_tool_ipu->Ipu_reg_60;

 		  	tmp = tool->tool_used & 0x00010000;  //HH_On
 		  	if(tmp){
 		  		ipu->Ipu_reg_61 = pc_tool_ipu->Ipu_reg_61;
 		  		ipu->Ipu_reg_62 = pc_tool_ipu->Ipu_reg_62;
 		  		ipu->Ipu_reg_63 = pc_tool_ipu->Ipu_reg_63;
 		  		ipu->Ipu_reg_64 = pc_tool_ipu->Ipu_reg_64;
 		  		ipu->Ipu_reg_65 = pc_tool_ipu->Ipu_reg_65;
 		  		ipu->Ipu_reg_66 = pc_tool_ipu->Ipu_reg_66;
 		  	}

 		  	tmp = tool->tool_used & 0x00020000;  //HS_On
 		  	if(tmp){
 		  		ipu->Ipu_reg_67 = pc_tool_ipu->Ipu_reg_67;
 		  		ipu->Ipu_reg_68 = pc_tool_ipu->Ipu_reg_68;
 		  		ipu->Ipu_reg_69 = pc_tool_ipu->Ipu_reg_69;
 		  		ipu->Ipu_reg_6A = pc_tool_ipu->Ipu_reg_6A;
 		  		ipu->Ipu_reg_6B = pc_tool_ipu->Ipu_reg_6B;
 		  		ipu->Ipu_reg_6C = pc_tool_ipu->Ipu_reg_6C;
 		  	}

 		  	tmp = tool->tool_used & 0x00040000;  //SW_On
 		  	if(tmp){
 		  		ipu->Ipu_reg_6D = pc_tool_ipu->Ipu_reg_6D;
 		  		ipu->Ipu_reg_6E = pc_tool_ipu->Ipu_reg_6E;
 		  		ipu->Ipu_reg_6F = pc_tool_ipu->Ipu_reg_6F;
 		  		ipu->Ipu_reg_70 = pc_tool_ipu->Ipu_reg_70;
 		  		ipu->Ipu_reg_71 = pc_tool_ipu->Ipu_reg_71;
 		  	}
 		  }

      //Y Gamma Curve
 		  tmp = tool->tool_used & 0x00000004;
 		  if(tmp){
 		  	ipu->Ipu_reg_31 = pc_tool_ipu->Ipu_reg_31;
 		  	ipu->Ipu_reg_32 = pc_tool_ipu->Ipu_reg_32;
 		  	ipu->Ipu_reg_33 = pc_tool_ipu->Ipu_reg_33;
 		  	ipu->Ipu_reg_34 = pc_tool_ipu->Ipu_reg_34;
 		  	ipu->Ipu_reg_35 = pc_tool_ipu->Ipu_reg_35;
 		  	ipu->Ipu_reg_36 = pc_tool_ipu->Ipu_reg_36;
 		  	ipu->Ipu_reg_37 = pc_tool_ipu->Ipu_reg_37;
 		  	ipu->Ipu_reg_38 = pc_tool_ipu->Ipu_reg_38;
 		  	ipu->Ipu_reg_39 = pc_tool_ipu->Ipu_reg_39;
 		  	ipu->Ipu_reg_3A = pc_tool_ipu->Ipu_reg_3A;
 		  	ipu->Ipu_reg_B0 = pc_tool_ipu->Ipu_reg_B0;
 		  }

      //YCbCr Gains & Offsets & rotation
 		  tmp = tool->tool_used & 0x00000008;
 		  if(tmp){
 		  	ipu->Ipu_reg_30 = pc_tool_ipu->Ipu_reg_30;
 		  	ipu->Ipu_reg_3B = pc_tool_ipu->Ipu_reg_3B;
 		  	ipu->Ipu_reg_3C = pc_tool_ipu->Ipu_reg_3C;
 		  	ipu->Ipu_reg_3D = pc_tool_ipu->Ipu_reg_3D;
 		  	ipu->Ipu_reg_B0 = pc_tool_ipu->Ipu_reg_B0;
 		  }
/*
      //CbCr 32x32 Matrix
 		  tmp = tool->tool_used & 0x00000010;
 		  if(tmp){
 		  	ipu->Ipu_reg_B0 = pc_tool_ipu->Ipu_reg_B0;
 		  	ipu->Ipu_reg_57 = pc_tool_ipu->Ipu_reg_57;
 		  	for (i=0x300; i<=0x12fc; i+=4){
 		  		 	ptr_reg_58 = (REG_58 *) (CbCr_32x32_ADR+i);
 		  	    ipu->Ipu_reg_58 = ptr_reg_58->reg_58;
 		    }
 		  }
*/
   }
//modification end, Vanessa-
#endif

	//Print_IPU_Register();
	// Enable IPU clock and DMA
	clock->MdClken |= BIT6;
	clock->MdClken |= BIT5;
	clock->MdClken |= BIT7;
	dmaR->Control = 0x00000001;
	dmaW->Control = 0x00000001;
	// Trigger IPU
	ipu->Ipu_reg_10 |= 0x01000000 ;
	// Fill IPU Registers --
	IODelay(3);
	//Wait scaling finish event
	ret = EventWaitWithTO(SCALING_FINISH_EVENT_ID, BIT0, OS_EVENT_OR, &FinishEvent, 8000);
	if (ret)
		MP_ALERT("Scaling Time out");
#if 0
	// Polling IPU Finish
	while (t < MAX_IPU_WAIT_TIME)
	{
		if (ipu->Ipu_reg_1C & 0x00010000)
		{
			MP_DEBUG("650 IPU Scaling Done");
			ret = 0 ;
			break;
		}
		t++;
	}
	MP_DEBUG("Time t==%d", t);
	if (t >= MAX_IPU_WAIT_TIME)
	{
		ret = FAIL ;
		mpDebugPrint("Scaling Time out");
	}
#endif
	IODelay(3);
	return ret ;
}



static int image_non_scale650(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD tx1, WORD ty1, BYTE filter)
{
	ST_IMAGEINFO s, t;
	DWORD *srcPtr, *trgPtr;
	int ret ;

	PSCALER_PARA pSC = GetScalerPara();
	ST_SCA_PARM *psScaParm;
	psScaParm = &pSC->sScaParm;
	//source
	s.wWidth = srcWin->wWidth ;
	s.wHeight = srcWin->wHeight;
	s.dwOffset = srcWin->dwOffset;

	//target
	t.wWidth = trgWin->wWidth ;
	t.wHeight = trgWin->wHeight ;
	//t.dwOffset = (trgWin->wWidth - IPU_650_MAX_SCALE_TARGET_SIZE) << 1 ;

	if (srcWin->wType == 444)
	{
#if YUV444_ENABLE
		s.dwPointer = (DWORD) ((srcWin->pdwStart + (((sy1 * srcWin->dwOffset) + sx1 * 3) >> 2)));
		t.dwPointer = trgWin->pdwStart + (((ty1 * trgWin->dwOffset) + tx1 * 3) >> 2);
#else
		s.dwPointer = (DWORD*) ((srcWin->pdwStart + (((sy1 * srcWin->dwOffset) + sx1 * 2) >> 2)));
		t.dwPointer = trgWin->pdwStart + (((ty1 * trgWin->dwOffset) + tx1 * 2) >> 2);
#endif
	}
	else
	{
		s.dwPointer = (DWORD*) ((srcWin->pdwStart + (((sy1 * srcWin->dwOffset) + sx1 * 2) >> 2)));
		t.dwPointer = trgWin->pdwStart + (((ty1 * trgWin->dwOffset) + tx1 * 2) >> 2);
	}

	ret = ipu_scale_650_nonCsc(&s, &t, srcWin->wType, filter);

	if (ret!=0)
	{
		mpDebugPrint("Source w%d, h%d, offset%d", s.wWidth, s.wHeight, s.dwOffset);
		mpDebugPrint("Target w%d, h%d, offset%d", t.wWidth, t.wHeight, t.dwOffset);
	}
	MP_DEBUG("image_sliceone_scale650 done, ret=%d", ret);
	return ret ;
}



static int image_sliceone_scale650(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD tx1, WORD ty1, BYTE filter)
{
	ST_IMAGEINFO s, t;
	DWORD *srcPtr, *trgPtr;
	int ret ;

	PSCALER_PARA pSC = GetScalerPara();
	ST_SCA_PARM *psScaParm;
	psScaParm = &pSC->sScaParm;
	//source
	s.wWidth = srcWin->wWidth ;
	s.wHeight = srcWin->wHeight;
	s.dwOffset = srcWin->dwOffset;

	//target
	t.wWidth = trgWin->wWidth ;
	t.wHeight = trgWin->wHeight ;
	//t.dwOffset = (trgWin->wWidth - IPU_650_MAX_SCALE_TARGET_SIZE) << 1 ;
	t.dwOffset = trgWin->dwOffset;

	if (srcWin->wType == 444)
	{
#if YUV444_ENABLE
		s.dwPointer = (DWORD) ((srcWin->pdwStart + (((sy1 * srcWin->dwOffset) + sx1 * 3) >> 2)));
		t.dwPointer = trgWin->pdwStart + (((ty1 * trgWin->dwOffset) + tx1 * 3) >> 2);
#else
		s.dwPointer = (DWORD*) ((srcWin->pdwStart + (((sy1 * srcWin->dwOffset) + sx1 * 2) >> 2)));
		t.dwPointer = trgWin->pdwStart + (((ty1 * trgWin->dwOffset) + tx1 * 2) >> 2);
#endif
	}
	else
	{
		s.dwPointer = (DWORD*) ((srcWin->pdwStart + (((sy1 * srcWin->dwOffset) + sx1 * 2) >> 2)));
		t.dwPointer = trgWin->pdwStart + (((ty1 * trgWin->dwOffset) + tx1 * 2) >> 2);
	}
	if (pSC->bMPV) // MPV mode
	{
		s.dwPointer = (DWORD*) ((srcWin->pdwStart + (((sy1 * srcWin->dwOffset) + sx1) >> 2)));
	}

	ret = ipu_scale_650(&s, &t, srcWin->wType, filter);
	if (ret!=0)
	{
		mpDebugPrint("Source w%d, h%d, offset%d", s.wWidth, s.wHeight, s.dwOffset);
		mpDebugPrint("Target w%d, h%d, offset%d", t.wWidth, t.wHeight, t.dwOffset);
	}
	MP_DEBUG("image_sliceone_scale650 done, ret=%d", ret);
	return ret ;
}

//#define MAX_IPU_WAIT_TIME 500000
#define MAX_IPU_WAIT_TIME 0x500000  //modify for MP650/MP660 suppor big size image

int ipu_overlay_bypass_scale_650(ST_IMGWIN * srcWin, ST_IMGWIN * srcSubWin, ST_IMGWIN * trgWin, BYTE mpv_flag, WORD tx1, WORD ty1, BYTE non_colorkey_level, BYTE colorkey_enable, BYTE colorkey_level, BYTE colorkey_Y, BYTE colorkey_Cb, BYTE colorkey_Cr)
{
	IPU *ipu;
	CHANNEL *dmaR, *dmaW, *overlay;
	BIU *biu;
	CLOCK *clock;
	clock = (CLOCK *) CLOCK_BASE;
	ipu = (IPU *) IPU_BASE;
	dmaR = (CHANNEL *) DMA_SCR_BASE;
	dmaW = (CHANNEL *) DMA_SCW_BASE;
	overlay = (CHANNEL *) DMA_THMW_BASE;

	ST_SCA_PARM *psScaParm;
	PSCALER_PARA pSC = GetScalerPara();
	psScaParm = &pSC->sScaParm;

	ipu_reset();
	//IPR DMA
	ipu->Ipu_reg_A2	= ((DWORD) srcWin->pdwStart| 0xA0000000);

	if(mpv_flag)
		ipu->Ipu_reg_1A = ALIGN_16(srcWin->wWidth) - ALIGN_4(srcWin->wWidth);
	else
		ipu->Ipu_reg_1A = 0 ;
	if(mpv_flag)
	{
		ipu->Ipu_reg_A0 = ipu->Ipu_reg_A2 + (ALIGN_16(srcWin->wWidth) * ALIGN_16(srcWin->wHeight));
		ipu->Ipu_reg_A1 = ALIGN_16(srcWin->wWidth) - ALIGN_4(srcWin->wWidth);

	}
	ipu->Ipu_reg_A4 = ((DWORD) srcSubWin->pdwStart);
	ipu->Ipu_reg_1F = 0 ; //Overlay win offset

	// Fill IPU Registers ++
	// IPR
	//ipu->Ipu_reg_11 = (((srcWin->wWidth - 1) & 0xfff) << 16) + ((srcWin->wHeight - 1) & 0xfff);
	//polun modify MP650/MP660 support Width 16bit
	ipu->Ipu_reg_11 = ((srcWin->wWidth - 1) << 16) + (srcWin->wHeight - 1);
	ipu->Ipu_reg_10 &= 0xfffffcff ; // Clear Bit8 and Bit9

	ipu->Ipu_reg_1D = 0x00010000 ;
	ipu->Ipu_reg_1E = ((srcSubWin->wWidth-1)<<16) + (srcSubWin->wHeight-1) ;
	// IPU DMA OUT Direction
	ipu->Ipu_reg_12 = 0x00000000 ;  //to IDU
	if(srcWin->wType == 444)
	{
	#if YUV444_ENABLE
		ipu->Ipu_reg_12 = 0x00010000 ;  //to IDU
	#endif
	}
	// Window before IPW
	ipu->Ipu_reg_14 = 0x01000000 ;	//Enable bit 24
	ipu->Ipu_reg_15 = (0 << 16) + (pSC->wTargetHeight - 1) ;
	ipu->Ipu_reg_16 = (0 << 16) + (pSC->wTargetWidth - 1) ;
	// Scaling path
	ipu->Ipu_reg_18 = 0x00070707 ;
	ipu->Ipu_reg_19 = 0x07020000 ;
	// Turn on IPU Frame End
	ipu->Ipu_reg_1C = 0x11000000 ;
	// IPU Subsample
	ipu->Ipu_reg_21 = (psScaParm->wVSubRatio << 8) + psScaParm->wHSubRatio ;
	// IPU Scaling factor
	if (psScaParm->wHSF == 0)
		ipu->Ipu_reg_22 = 0x80000000 ;
	else
		ipu->Ipu_reg_22 = (psScaParm->wHSF << 16);
	if (psScaParm->wVSF == 0)
		ipu->Ipu_reg_23 = 0x80000000 ;
	else
		ipu->Ipu_reg_23 = (psScaParm->wVSF << 16) ;
	// HBIL_back
	if (psScaParm->wHUp)		// Scaling up
		ipu->Ipu_reg_20 = 0x00000001 ;
	ipu->Ipu_reg_20 |= 0x00000002 ;		//double line buffer
	//double line buffer need to disable filter
	ipu->Ipu_reg_26 |= 0x03000001 ;
	ipu->Ipu_reg_27 |= 0x01000000 ;
	// Video auto start
	ipu->Ipu_reg_10 |= 0x10000000 ;
	//MPV
	if (mpv_flag)
	{
		ipu->Ipu_reg_10 &= 0xfffcffff ;
		ipu->Ipu_reg_10 |= 0x00104000 ; //Use PP line buffer
	}
	else
	{
		ipu->Ipu_reg_10 &= 0xfffcffff ;
		if(srcWin->wType == 444)
		{
		#if YUV444_ENABLE
			ipu->Ipu_reg_10 |= 0x00020000 ;
		#else
			ipu->Ipu_reg_10 |= 0x00010000 ;
		#endif
		}
		else
		{
		ipu->Ipu_reg_10 |= 0x00010000 ;
	}
	}

	//Overlay win settings ++
	if (colorkey_enable)
		ipu->Ipu_reg_90 = (1<<28) + (1<<24) + (colorkey_Y<<16) + (colorkey_Cb<<8) + colorkey_Cr ;
	else
		ipu->Ipu_reg_90 = 0x10000000 ;
	ipu->Ipu_reg_91 = ((ty1&0xfff) << 16) + (tx1&0xfff);
	// Non-Color Key weight for main and sub win
	if (non_colorkey_level == 0)	// Sub win 100 %, Main win 0%
	{
		ipu->Ipu_reg_92 = 0x00000000 ;
		ipu->Ipu_reg_93 = 0x01000000 ;
	}
	else if (non_colorkey_level == 1)	// Sub win 50 %, Main win 50%
	{
		ipu->Ipu_reg_92 = 0x00800000 ;
		ipu->Ipu_reg_93 = 0x00800000 ;
	}
	else if (non_colorkey_level == 2)	// Sub win 0 %, Main win 100%
	{
		ipu->Ipu_reg_92 = 0x01000000 ;
		ipu->Ipu_reg_93 = 0x00000000 ;
	}
	// Color Key weight for main and sub win
	if (colorkey_enable)
	{
		if (colorkey_level == 0)	// Sub win 100 %, Main win 0%
		{
			ipu->Ipu_reg_93 = ipu->Ipu_reg_93 + 0x00000100 ;
		}
		else if (colorkey_level == 1)	// Sub win 50 %, Main win 50%
		{
			ipu->Ipu_reg_92 = ipu->Ipu_reg_92 + 0x00000080 ;
			ipu->Ipu_reg_93 = ipu->Ipu_reg_93 + 0x00000080 ;
		}
		else if (colorkey_level == 2)	// Sub win 0 %, Main win 100%
		{
			ipu->Ipu_reg_92 = ipu->Ipu_reg_92 + 0x00000100 ;
		}
	}
	//Fill wallpaper enable
	ipu->Ipu_reg_94 = 0x00000000 ;
	//Overlay win settings --
	//Print_IPU_Register();
	// DMA Enable
	clock->MdClken |= BIT6;
	dmaR->Control = 0x00000001;
	dmaW->Control = 0x00000001;
	overlay->Control |= 0x00000001;
	//Print_IPU_Register();
	Idu_DMA2Bypass();				// must after DMA enable  before IPU trigger
	MP650_FPGA_Change_DMA_Priority_IPU();
	// Trigger IPU
	ipu->Ipu_reg_10 |= 0x01000000 ;
	IDU_Show_Original_Color(); //recovering IDU_Show_BG_Color_Black() called in MovieTask_Play();
	return 0 ;
}

static int ipu_bypass_scale_650(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, BYTE mpv_flag)
{
	IPU *ipu;
	CHANNEL *dmaR, *dmaW, *overlay;
	BIU *biu;
	CLOCK *clock;
	clock = (CLOCK *) CLOCK_BASE;
	ipu = (IPU *) IPU_BASE;
	dmaR = (CHANNEL *) DMA_SCR_BASE;
	dmaW = (CHANNEL *) DMA_SCW_BASE;
	overlay = (CHANNEL *) DMA_THMW_BASE;

	ST_SCA_PARM *psScaParm;
	PSCALER_PARA pSC = GetScalerPara();
	psScaParm = &pSC->sScaParm;

	ipu_reset();
	//IPR DMA
	ipu->Ipu_reg_A2	= ((DWORD) srcWin->pdwStart| 0xA0000000);

	if(mpv_flag)
		ipu->Ipu_reg_1A = ALIGN_16(srcWin->wWidth) - ALIGN_4(srcWin->wWidth);
	else
		ipu->Ipu_reg_1A = 0 ;
	if(mpv_flag)
	{
		ipu->Ipu_reg_A0 = ipu->Ipu_reg_A2 + (ALIGN_16(srcWin->wWidth) * ALIGN_16(srcWin->wHeight));
		ipu->Ipu_reg_A1 = ALIGN_16(srcWin->wWidth) - ALIGN_4(srcWin->wWidth);

	}

	// Fill IPU Registers ++
	// IPR
	//ipu->Ipu_reg_11 = (((srcWin->wWidth - 1) & 0xfff) << 16) + ((srcWin->wHeight - 1) & 0xfff);
	//polun modify MP650/MP660 support Width 16bit

	//when vide movie height eaual plane height may be to lead to ipuisr shut down  
	if(trgWin->wHeight==srcWin->wHeight)
		ipu->Ipu_reg_11 = ((srcWin->wWidth - 1) << 16) + (srcWin->wHeight - 0);
	else
		ipu->Ipu_reg_11 = ((srcWin->wWidth - 1) << 16) + (srcWin->wHeight - 1);	
    
	ipu->Ipu_reg_10 &= 0xfffffcff ; // Clear Bit8 and Bit9
	// IPU DMA OUT Direction
	ipu->Ipu_reg_12 = 0x00000000 ;  //to IDU
	if(srcWin->wType == 444)
	{
	#if YUV444_ENABLE
		ipu->Ipu_reg_12 = 0x00010000 ;  //to IDU
	#endif
	}
	// Window before IPW
	ipu->Ipu_reg_14 = 0x01000000 ;	//Enable bit 24
	ipu->Ipu_reg_15 = (0 << 16) + (pSC->wTargetHeight - 1) ;
	ipu->Ipu_reg_16 = (0 << 16) + (pSC->wTargetWidth - 1) ;
	// Scaling path
	ipu->Ipu_reg_18 = 0x00070707 ;
	ipu->Ipu_reg_19 = 0x07020000 ;
	// Turn on IPU Frame End
	ipu->Ipu_reg_1C = 0x11000000 ;
	// IPU Subsample
	ipu->Ipu_reg_21 = (psScaParm->wVSubRatio << 8) + psScaParm->wHSubRatio ;
	// IPU Scaling factor
	if (psScaParm->wHSF == 0)
		ipu->Ipu_reg_22 = 0x80000000 ;
	else
		ipu->Ipu_reg_22 = (psScaParm->wHSF << 16);
	if (psScaParm->wVSF == 0)
		ipu->Ipu_reg_23 = 0x80000000 ;
	else
		ipu->Ipu_reg_23 = (psScaParm->wVSF << 16) ;
	// HBIL_back
	if (psScaParm->wHUp)		// Scaling up
		ipu->Ipu_reg_20 = 0x00000001 ;
	ipu->Ipu_reg_20 |= 0x00000002 ;		//double line buffer
	//double line buffer need to disable filter
	ipu->Ipu_reg_26 |= 0x03000001 ;
	ipu->Ipu_reg_27 |= 0x01000000 ;
	// Video auto start
	ipu->Ipu_reg_10 |= 0x10000000 ;
	//MPV
	if (mpv_flag)
	{
		ipu->Ipu_reg_10 &= 0xfffcffff ;
		ipu->Ipu_reg_10 |= 0x00104000 ; //Use PP line buffer
	}
	else
	{
		ipu->Ipu_reg_10 &= 0xfffcffff ;
		if(srcWin->wType == 444)
		{
		#if YUV444_ENABLE
			ipu->Ipu_reg_10 |= 0x00020000 ;
		#else
			ipu->Ipu_reg_10 |= 0x00010000 ;
		#endif
		}
		else
		{
			ipu->Ipu_reg_10 |= 0x00010000 ;
		}
	}

	//Overlay win settings ++
	ipu->Ipu_reg_90 = 0x30000000 ;
	// Sub win 100 %, Main win 0%
	ipu->Ipu_reg_92 = 0x00000000 ;
	ipu->Ipu_reg_93 = 0x01000000 ;
	//Fill wallpaper enable
	ipu->Ipu_reg_94 = 0x10000000 ;
	//Wallpaper settings
	ipu->Ipu_reg_1D = 0x00010000 ;
	struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;
	if((pstScreen->wInnerWidth > trgWin->wWidth) || (pstScreen->wInnerHeight > trgWin->wHeight))
	{
	ipu->Ipu_reg_91 = ((((pstScreen->wInnerHeight-trgWin->wHeight)>>1)&0xfff) << 16) + (((pstScreen->wInnerWidth-trgWin->wWidth)>>1)&0xfff);
	//ipu->Ipu_reg_91 = ((0&0xfff) << 16) + (((pstScreen->wInnerWidth-trgWin->wWidth)>>1)&0xfff);
	ipu->Ipu_reg_1E = ((pstScreen->wInnerWidth-1)<<16) + (pstScreen->wInnerHeight-1) ;
		g_bOverlay_Use_Fillwall = 1;
	}
	else
	{
	ipu->Ipu_reg_91 = ((pSC->wOverlayStartY&0xfff) << 16) + (pSC->wOverlayStartX&0xfff);
	ipu->Ipu_reg_1E = ((trgWin->wWidth-1)<<16) + (trgWin->wHeight-1) ;
		g_bOverlay_Use_Fillwall = 0;
	}
	ipu->Ipu_reg_A4 = 0x00808000 ;	// background use black color
	//Overlay win settings --
	//Print_IPU_Register();
	// DMA Enable
	clock->MdClken |= BIT6;
	dmaR->Control = 0x00000001;
	dmaW->Control = 0x00000001;
	overlay->Control |= 0x00000001;
	//Print_IPU_Register();
	#if 0
	Idu_DMA2Bypass();				// must after DMA enable  before IPU trigger
	//Idu_Chg_Bypass444_Mode();
	MP650_FPGA_Change_DMA_Priority_IPU();
    #else  //for YUV444 or video on low speed storage into bypass mode have fliker 
    register IDU *idu = (IDU *)IDU_BASE;
    int waitflg;
    waitflg=1;
	Idu_DMA2Bypass();				// must after DMA enable  before IPU trigger
	MP650_FPGA_Change_DMA_Priority_IPU();
    while(waitflg)
    {
      if((idu->RST_Ctrl_Bypass & 0x18000000)!=0x10000000)
      {
          waitflg=0;
      }
    }
	#endif
	// Trigger IPU
	ipu->Ipu_reg_10 |= 0x01000000 ;
    #if ((PANEL_ID == PANEL_HDMI_720P)||(PANEL_ID == PANEL_ADV7393))
    IODelay(500);
    #endif
	IDU_Show_Original_Color(); //recovering IDU_Show_BG_Color_Black() called in MovieTask_Play();
	return 0 ;
}

static int Bypass_scale(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2,
                        WORD tx1, WORD ty1, WORD tx2, WORD ty2, BYTE mpv_flag)
{
	int ret ;
	WORD Src_W, Src_H, Trg_W, Trg_H ;
	PSCALER_PARA pSC = GetScalerPara();
	ST_SCA_PARM *psScaParm;
	psScaParm = &pSC->sScaParm;
	//mmcp_memset((BYTE*)pSC, 0, sizeof(SCALER_PARA));
	memset((BYTE*)pSC, 0, sizeof(SCALER_PARA));
	if ((sx1 >= sx2) || (sy1 >= sy2) || (tx1 >= tx2) || (ty1 >= ty2))
	{
		MP_ALERT("-E- Bypass_scale region error %d->%d %d->%d %d->%d %d->%d", sx1, sx2, sy1, sy2, tx1, tx2, ty1, ty2);
		return FAIL;
	}
	Src_W = sx2 - sy1 ;
	Src_H = sy2 - sy1 ;
	Trg_W = tx2 - tx1 ;
	Trg_H = ty2 - ty1 ;
	Get650ScaParm(psScaParm, Src_W, Src_H, Trg_W, Trg_H);
	pSC->wTargetWidth = Trg_W ;
	pSC->wTargetHeight = Trg_H ;
	pSC->wOverlayStartX = tx1 ;
	pSC->wOverlayStartY = ty1 ;
	
#if DEMO_PID
	if(g_SubWin != NULL)
		ret = ipu_overlay_bypass_scale_650(srcWin,g_SubWin,trgWin,mpv_flag,g_Overlay_param->tx1,g_Overlay_param->ty1,g_Overlay_param->non_colorkey_level,
			g_Overlay_param->colorkey_level,	g_Overlay_param->colorkey_enable,g_Overlay_param->colorkey_Y,g_Overlay_param->colorkey_Cb,g_Overlay_param->colorkey_Cr);
	else
		ret = ipu_bypass_scale_650(srcWin, trgWin, mpv_flag);
#else
	ret = ipu_bypass_scale_650(srcWin, trgWin, mpv_flag);
#endif
	return ret ;
}

static int ipu_overlay_position_adjust(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD tx1, WORD ty1, WORD tx2, WORD ty2)
{
	ST_IMAGEINFO s, t;

	//source
	s.wWidth = srcWin->wWidth ;
	s.wHeight = srcWin->wHeight;
	//target
	t.wWidth = trgWin->wWidth ;
	t.wHeight = trgWin->wHeight ;
	//Adjust start address
	//s.dwPointer = (DWORD) ((srcWin->pdwStart + (((sy1 * srcWin->dwOffset) + sx1 * 2) >> 2)));
	t.dwPointer = trgWin->pdwStart + (((ty1 * trgWin->dwOffset) + tx1 * 2) >> 2);
}

int image_scale_mpv_bypass(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2,
                           WORD tx1, WORD ty1, WORD tx2, WORD ty2)
{
	if (sx2 > srcWin->wWidth)  sx2 = srcWin->wWidth ;
	if (sy2 > srcWin->wHeight) sy2 = srcWin->wHeight;
	if (tx2 > trgWin->wWidth)  tx2 = trgWin->wWidth ;
	if (ty2 > trgWin->wHeight) ty2 = trgWin->wHeight;

	if ((sx1 >= sx2) || (sy1 >= sy2) || (tx1 >= tx2) || (ty1 >= ty2))
	{
		MP_ALERT("mpv_bypass: -E- scale region error %d->%d %d->%d %d->%d %d->%d", sx1, sx2, sy1, sy2, tx1, tx2, ty1, ty2);
		return FAIL;
	}
	if (srcWin->dwOffset == 0)
	{
		MP_ALERT("-E- srcWin->dwOffset == 0");
		srcWin->dwOffset = ALIGN_16(srcWin->wWidth) << 1;
	}
	if (trgWin->dwOffset == 0)
	{
		MP_ALERT("-E- trgWin->dwOffset == 0");
		trgWin->dwOffset = trgWin->wWidth << 1;
	}

	//return scale_once_non_filter(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2, 1);
	return Bypass_scale(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2, 1);

}

static int scale_once_non_filter(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2,
                                 WORD tx1, WORD ty1, WORD tx2, WORD ty2, BYTE MPV)
{
	WORD sXdiff, sYdiff, tXdiff, tYdiff;
	DWORD src_x, dst_x, src_y, dst_y, src_dx, dst_dx ;
	WORD Src_Start_Point = 0, Src_End_Point = 0, Trg_Start_Point = 0, Trg_End_Point = 0 ;
	WORD rest = 0 ;
	int i ;

	PSCALER_PARA pSC = GetScalerPara();
	ST_SCA_PARM *psScaParm;
	psScaParm = &pSC->sScaParm;
	mmcp_memset((BYTE*)pSC, 0, sizeof(SCALER_PARA));
	//memset((BYTE*)pSC, 0, sizeof(SCALER_PARA));

	sXdiff = (sx2 - sx1);
	sYdiff = (sy2 - sy1);
	tXdiff = (tx2 - tx1);
	tYdiff = (ty2 - ty1);
	MP_DEBUG("src_w=%d, src_h=%d, trg_w=%d, trg_h=%d", sXdiff, sYdiff, tXdiff, tYdiff);

	src_x = sx1;
	dst_x = tx1;
	src_y = sy1;
	dst_y = ty1;

	pSC->wSourceImageWidth = sXdiff ;
	pSC->wSourceImageHeight = sYdiff ;
	pSC->wTargetImageWidth = tXdiff ;
	pSC->wTargetImageHeight = tYdiff ;

	Get650ScaParm(psScaParm, sXdiff, sYdiff, tXdiff, tYdiff);

	pSC->bNeedInitValue = 0 ;

	//Trg_Start_Point = GetTrgStartPoint_NonFilter(i, IPU_650_MAX_SCALE_TARGET_SIZE);
	//Src_Start_Point = GetSrcStartPoint(sXdiff, tXdiff, Trg_Start_Point);
	//Trg_End_Point = Trg_Start_Point + (IPU_650_MAX_SCALE_TARGET_SIZE-1) ;
	//Src_End_Point = GetSrcEndPoint(sXdiff, tXdiff, Trg_End_Point);
	MP_DEBUG("Trg Start=%d, Trg End=%d, Src Start=%d, Src End=%d", Trg_Start_Point, Trg_End_Point, Src_Start_Point, Src_End_Point);
	pSC->wTargetWidth = tXdiff ;
	pSC->wSourceFilterWidth = sXdiff ;
	pSC->wSourceOutterWidth = ALIGN_2(pSC->wSourceFilterWidth);
	if (pSC->wSourceOutterWidth > pSC->wSourceFilterWidth)
		pSC->wIPR_Drop_First_Point = pSC->wSourceOutterWidth - pSC->wSourceFilterWidth ;
	pSC->wSourceFilterHeight = sYdiff ;
	pSC->wTargetHeight = tYdiff ;

	if (MPV)	pSC->bMPV = 1 ;
	else	pSC->bMPV = 0 ;
	if (image_sliceone_scale650(srcWin, trgWin, src_x, src_y, dst_x, dst_y, 0))
	{
		pSC->bMPV = 0 ;
		return FAIL;
	}
	pSC->bMPV = 0 ;
}


static int non_scale_non_filter(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2,
                                 WORD tx1, WORD ty1, WORD tx2, WORD ty2, BYTE MPV)
{
	WORD sXdiff, sYdiff, tXdiff, tYdiff;
	DWORD src_x, dst_x, src_y, dst_y, src_dx, dst_dx ;
	WORD Src_Start_Point = 0, Src_End_Point = 0, Trg_Start_Point = 0, Trg_End_Point = 0 ;
	WORD rest = 0 ;
	int i ;

	PSCALER_PARA pSC = GetScalerPara();
	ST_SCA_PARM *psScaParm;
	psScaParm = &pSC->sScaParm;
	mmcp_memset((BYTE*)pSC, 0, sizeof(SCALER_PARA));
	//memset((BYTE*)pSC, 0, sizeof(SCALER_PARA));

	sXdiff = (sx2 - sx1);
	sYdiff = (sy2 - sy1);
	tXdiff = (tx2 - tx1);
	tYdiff = (ty2 - ty1);
	MP_DEBUG("src_w=%d, src_h=%d, trg_w=%d, trg_h=%d", sXdiff, sYdiff, tXdiff, tYdiff);

	src_x = sx1;
	dst_x = tx1;
	src_y = sy1;
	dst_y = ty1;

	pSC->wSourceImageWidth = sXdiff ;
	pSC->wSourceImageHeight = sYdiff ;
	pSC->wTargetImageWidth = tXdiff ;
	pSC->wTargetImageHeight = tYdiff ;

	Get650ScaParm(psScaParm, sXdiff, sYdiff, tXdiff, tYdiff);

	pSC->bNeedInitValue = 0 ;

	//Trg_Start_Point = GetTrgStartPoint_NonFilter(i, IPU_650_MAX_SCALE_TARGET_SIZE);
	//Src_Start_Point = GetSrcStartPoint(sXdiff, tXdiff, Trg_Start_Point);
	//Trg_End_Point = Trg_Start_Point + (IPU_650_MAX_SCALE_TARGET_SIZE-1) ;
	//Src_End_Point = GetSrcEndPoint(sXdiff, tXdiff, Trg_End_Point);
	MP_DEBUG("Trg Start=%d, Trg End=%d, Src Start=%d, Src End=%d", Trg_Start_Point, Trg_End_Point, Src_Start_Point, Src_End_Point);
	pSC->wTargetWidth = tXdiff ;
	pSC->wSourceFilterWidth = sXdiff ;
	pSC->wSourceOutterWidth = ALIGN_2(pSC->wSourceFilterWidth);
	if (pSC->wSourceOutterWidth > pSC->wSourceFilterWidth)
		pSC->wIPR_Drop_First_Point = pSC->wSourceOutterWidth - pSC->wSourceFilterWidth ;
	pSC->wSourceFilterHeight = sYdiff ;
	pSC->wTargetHeight = tYdiff ;

	if (MPV)	pSC->bMPV = 1 ;
	else	pSC->bMPV = 0 ;
	if (image_non_scale650(srcWin, trgWin, src_x, src_y, dst_x, dst_y, 0))
	{
		pSC->bMPV = 0 ;
		return FAIL;
	}
	pSC->bMPV = 0 ;
}


int image_scale_mpv2mem(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2,
                        WORD tx1, WORD ty1, WORD tx2, WORD ty2)
{
	if (sx2 > srcWin->wWidth)  sx2 = srcWin->wWidth ;
	if (sy2 > srcWin->wHeight) sy2 = srcWin->wHeight;
	if (tx2 > trgWin->wWidth)  tx2 = trgWin->wWidth ;
	if (ty2 > trgWin->wHeight) ty2 = trgWin->wHeight;

	if ((sx1 >= sx2) || (sy1 >= sy2) || (tx1 >= tx2) || (ty1 >= ty2))
	{
		MP_ALERT("mpv2mem: -E- scale region error %d->%d %d->%d %d->%d %d->%d", sx1, sx2, sy1, sy2, tx1, tx2, ty1, ty2);
		return FAIL;
	}
	if (srcWin->dwOffset == 0)
	{
		MP_ALERT("-E- srcWin->dwOffset == 0");
		srcWin->dwOffset = ALIGN_16(srcWin->wWidth) << 1;
	}
	if (trgWin->dwOffset == 0)
	{
		MP_ALERT("-E- trgWin->dwOffset == 0");
		trgWin->dwOffset = trgWin->wWidth << 1;
	}
    #if (DEMO_PID!=1)
    if ((sx2- sx1) > (tx2 - tx1) && (tx2 - tx1)> IPU_650_MAX_SCALE_TARGET_SIZE)
    {
	return multi_scale_mpv_non_filter(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2);
	}
	else
	#endif
	return scale_once_non_filter(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2, 1);

}

int image_scale_mjpg_bypass(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2,
                            WORD tx1, WORD ty1, WORD tx2, WORD ty2)
{
	if (sx2 > srcWin->wWidth)  sx2 = srcWin->wWidth ;
	if (sy2 > srcWin->wHeight) sy2 = srcWin->wHeight;
	if (tx2 > trgWin->wWidth)  tx2 = trgWin->wWidth ;
	if (ty2 > trgWin->wHeight) ty2 = trgWin->wHeight;

	if ((sx1 >= sx2) || (sy1 >= sy2) || (tx1 >= tx2) || (ty1 >= ty2))
	{
		MP_ALERT("mjpg_bypass : -E- scale region error %d->%d %d->%d %d->%d %d->%d", sx1, sx2, sy1, sy2, tx1, tx2, ty1, ty2);
		return FAIL;
	}
	if (srcWin->dwOffset == 0)
	{
		MP_ALERT("-E- srcWin->dwOffset == 0");
		srcWin->dwOffset = ALIGN_16(srcWin->wWidth) << 1;
	}
	if (trgWin->dwOffset == 0)
	{
		MP_ALERT("-E- trgWin->dwOffset == 0");
		trgWin->dwOffset = trgWin->wWidth << 1;
	}

	//return scale_once_non_filter(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2, 1);
	return Bypass_scale(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2, 0);

}
static int multi_scale_non_filter(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2,
                                  WORD tx1, WORD ty1, WORD tx2, WORD ty2)
{
	WORD sXdiff, sYdiff, tXdiff, tYdiff;
	DWORD src_x, dst_x, src_y, dst_y, src_dx, dst_dx ;
	WORD Src_Start_Point = 0, Src_End_Point = 0, Trg_Start_Point = 0, Trg_End_Point = 0 ;
	WORD scale_times = 0 ;
	WORD rest = 0 ;
	int i ;
	BYTE bYUV444 = 0;

	PSCALER_PARA pSC = GetScalerPara();
	ST_SCA_PARM *psScaParm;
	psScaParm = &pSC->sScaParm;
	mmcp_memset((BYTE*)pSC, 0, sizeof(SCALER_PARA));
	//memset((BYTE*)pSC, 0, sizeof(SCALER_PARA));

	sXdiff = (sx2 - sx1);
	sYdiff = (sy2 - sy1);
	tXdiff = (tx2 - tx1);
	tYdiff = (ty2 - ty1);
	MP_DEBUG("multi_scale_non_filter src_w=%d, src_h=%d, trg_w=%d, trg_h=%d", sXdiff, sYdiff, tXdiff, tYdiff);

	src_x = sx1;
	dst_x = tx1;
	src_y = sy1;
	dst_y = ty1;

	pSC->wSourceImageWidth = sXdiff ;
	pSC->wSourceImageHeight = sYdiff ;
	pSC->wTargetImageWidth = tXdiff ;
	pSC->wTargetImageHeight = tYdiff ;

	Get650ScaParm(psScaParm, sXdiff, sYdiff, tXdiff, tYdiff);

	scale_times = tXdiff / IPU_650_MAX_SCALE_TARGET_SIZE ;
	if (tXdiff % IPU_650_MAX_SCALE_TARGET_SIZE)
		rest = tXdiff - (IPU_650_MAX_SCALE_TARGET_SIZE*scale_times);

	if(srcWin->wType == 444)
	{
	#if YUV444_ENABLE
		bYUV444 = 1;
	#endif
	}

	pSC->bNeedInitValue = 0 ;
	for (i=0 ; i< scale_times ; i++)
	{
		Trg_Start_Point = GetTrgStartPoint_NonFilter(i, IPU_650_MAX_SCALE_TARGET_SIZE);
		Src_Start_Point = GetSrcStartPoint(sXdiff, tXdiff, Trg_Start_Point);
		Trg_End_Point = Trg_Start_Point + (IPU_650_MAX_SCALE_TARGET_SIZE-1) ;
		Src_End_Point = GetSrcEndPoint(sXdiff, tXdiff, Trg_End_Point);
		MP_DEBUG("Trg Start=%d, Trg End=%d, Src Start=%d, Src End=%d", Trg_Start_Point, Trg_End_Point, Src_Start_Point, Src_End_Point);
		pSC->wTargetWidth = IPU_650_MAX_SCALE_TARGET_SIZE ;
		pSC->wSourceFilterWidth = (Src_End_Point - Src_Start_Point) + 1 ;
		if(bYUV444)
			pSC->wSourceOutterWidth = ALIGN_4(pSC->wSourceFilterWidth);
		else
			pSC->wSourceOutterWidth = ALIGN_2(pSC->wSourceFilterWidth);
		//if(pSC->wSourceOutterWidth > pSC->wSourceFilterWidth)
		//	pSC->wIPR_Drop_First_Point = pSC->wSourceOutterWidth - pSC->wSourceFilterWidth ;
		pSC->wSourceFilterHeight = sYdiff ;
		pSC->wTargetHeight = tYdiff ;

		if (i!=0)	pSC->bNeedInitValue = 1 ;
		src_x = Src_Start_Point + sx1;
		dst_x = Trg_Start_Point + tx1;
		if (image_sliceone_scale650(srcWin, trgWin, src_x, src_y, dst_x, dst_y, 0))
			return FAIL;
	}
	// Last part
	//rest = 0 ;
	MP_DEBUG("rest=%d", rest);
	if (rest)
	{
		pSC->wIPR_Drop_First_Point = 0;
		if(bYUV444)
		{
			Trg_Start_Point = ALIGN_CUT_4(GetTrgStartPoint_NonFilter(scale_times, IPU_650_MAX_SCALE_TARGET_SIZE));
			Trg_End_Point = ALIGN_CUT_4(tXdiff - 1);
		}
		else
		{
			Trg_Start_Point = GetTrgStartPoint_NonFilter(scale_times, IPU_650_MAX_SCALE_TARGET_SIZE);
			Trg_End_Point = tXdiff - 1 ;
		}
		Src_Start_Point = GetSrcStartPoint(sXdiff, tXdiff, Trg_Start_Point);
		Src_End_Point = GetSrcEndPoint(sXdiff, tXdiff, Trg_End_Point);
		//mpDebugPrint("non Last part Trg Start=%d, Trg End=%d, Src Start=%d, Src End=%d", Trg_Start_Point, Trg_End_Point, Src_Start_Point, Src_End_Point);
		pSC->wSourceFilterWidth = (Src_End_Point - Src_Start_Point) + 1 ;
		if(bYUV444)
		{
			pSC->wTargetWidth = ALIGN_CUT_4(Trg_End_Point - Trg_Start_Point + 1);
			pSC->wSourceOutterWidth = ALIGN_4(pSC->wSourceFilterWidth);
		}
		else
		{
			pSC->wTargetWidth = Trg_End_Point - Trg_Start_Point + 1 ;
			pSC->wSourceOutterWidth = ALIGN_2(pSC->wSourceFilterWidth);
		}
		MP_DEBUG("wSourceOutterWidth=%d, wSourceFilterWidth=%d", pSC->wSourceOutterWidth, pSC->wSourceFilterWidth);
		if (pSC->wSourceOutterWidth > pSC->wSourceFilterWidth)
		{
			MP_DEBUG("Need to drop src point");
			pSC->wIPR_Drop_First_Point = pSC->wSourceOutterWidth - pSC->wSourceFilterWidth ;
		}
		else	pSC->wIPR_Drop_First_Point = 0 ;
		pSC->wSourceFilterHeight = sYdiff ;
		pSC->wTargetHeight = tYdiff ;

		pSC->bNeedInitValue = 1 ;
		if(bYUV444)
		{
			src_x = ALIGN_CUT_4(Src_Start_Point + sx1);
			dst_x = ALIGN_CUT_4(Trg_Start_Point + tx1);
		}
		else
		{
			src_x = Src_Start_Point + sx1;
			dst_x = Trg_Start_Point + tx1;
		}
		MP_DEBUG("src_x=%d", src_x);
		if (image_sliceone_scale650(srcWin, trgWin, src_x, src_y, dst_x, dst_y, 0))
			return FAIL;
	}

}

static int multi_scale_mpv_non_filter(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2,
                                  WORD tx1, WORD ty1, WORD tx2, WORD ty2)
{
	WORD sXdiff, sYdiff, tXdiff, tYdiff;
	DWORD src_x, dst_x, src_y, dst_y, src_dx, dst_dx ;
	WORD Src_Start_Point = 0, Src_End_Point = 0, Trg_Start_Point = 0, Trg_End_Point = 0 ;
	WORD scale_times = 0 ;
	WORD rest = 0 ;
	int i ;
	BYTE bYUV444 = 0;

	PSCALER_PARA pSC = GetScalerPara();
	ST_SCA_PARM *psScaParm;
	psScaParm = &pSC->sScaParm;
	mmcp_memset((BYTE*)pSC, 0, sizeof(SCALER_PARA));
	//memset((BYTE*)pSC, 0, sizeof(SCALER_PARA));

	sXdiff = (sx2 - sx1);
	sYdiff = (sy2 - sy1);
	tXdiff = (tx2 - tx1);
	tYdiff = (ty2 - ty1);
	MP_DEBUG("multi_scale_non_filter src_w=%d, src_h=%d, trg_w=%d, trg_h=%d", sXdiff, sYdiff, tXdiff, tYdiff);

	src_x = sx1;
	dst_x = tx1;
	src_y = sy1;
	dst_y = ty1;

	pSC->wSourceImageWidth = sXdiff ;
	pSC->wSourceImageHeight = sYdiff ;
	pSC->wTargetImageWidth = tXdiff ;
	pSC->wTargetImageHeight = tYdiff ;

	Get650ScaParm(psScaParm, sXdiff, sYdiff, tXdiff, tYdiff);

	scale_times = tXdiff / IPU_650_MAX_SCALE_TARGET_SIZE ;
	if (tXdiff % IPU_650_MAX_SCALE_TARGET_SIZE)
		rest =tXdiff - (IPU_650_MAX_SCALE_TARGET_SIZE*scale_times);

	if(srcWin->wType == 444)
	{
	#if YUV444_ENABLE
		bYUV444 = 1;
	#endif
	}

	pSC->bNeedInitValue = 0 ;
	for (i=0 ; i< scale_times ; i++)
	{
		Trg_Start_Point = GetTrgStartPoint_NonFilter(i, IPU_650_MAX_SCALE_TARGET_SIZE);
		Src_Start_Point = GetSrcStartPoint(sXdiff, tXdiff, Trg_Start_Point);
		Trg_End_Point = Trg_Start_Point + (IPU_650_MAX_SCALE_TARGET_SIZE-1) ;
		Src_End_Point = GetSrcEndPoint(sXdiff, tXdiff, Trg_End_Point);
		MP_DEBUG("Trg Start=%d, Trg End=%d, Src Start=%d, Src End=%d", Trg_Start_Point, Trg_End_Point, Src_Start_Point, Src_End_Point);
		pSC->wTargetWidth = IPU_650_MAX_SCALE_TARGET_SIZE ;
		pSC->wSourceFilterWidth = (Src_End_Point - Src_Start_Point) + 1 ;
		if(bYUV444)
			pSC->wSourceOutterWidth = ALIGN_4(pSC->wSourceFilterWidth);
		else
			pSC->wSourceOutterWidth = ALIGN_2(pSC->wSourceFilterWidth);

		//if(pSC->wSourceOutterWidth > pSC->wSourceFilterWidth)
		//	pSC->wIPR_Drop_First_Point = pSC->wSourceOutterWidth - pSC->wSourceFilterWidth ;
		pSC->wSourceFilterHeight = sYdiff ;
		pSC->wTargetHeight = tYdiff ;

		if (i!=0)	pSC->bNeedInitValue = 1 ;
		src_x = Src_Start_Point + sx1;
		dst_x = Trg_Start_Point + tx1;
	    pSC->bMPV = 1 ;
	    if (image_sliceone_scale650(srcWin, trgWin, src_x, src_y, dst_x, dst_y, 0))
	    {
		   pSC->bMPV = 0 ;
		   return FAIL;
	    }
	    pSC->bMPV = 0 ;
	}
	// Last part
	//rest = 0 ;
	MP_DEBUG("rest=%d", rest);
	if (rest)
	{
		pSC->wIPR_Drop_First_Point = 0;
		if(bYUV444)
		{
			Trg_Start_Point = ALIGN_CUT_4(GetTrgStartPoint_NonFilter(scale_times, IPU_650_MAX_SCALE_TARGET_SIZE));
			Trg_End_Point = ALIGN_CUT_4(tXdiff - 1);
		}
		else
		{
			Trg_Start_Point = GetTrgStartPoint_NonFilter(scale_times, IPU_650_MAX_SCALE_TARGET_SIZE);
			Trg_End_Point = tXdiff - 1 ;
		}
		Src_Start_Point = GetSrcStartPoint(sXdiff, tXdiff, Trg_Start_Point);
		Src_End_Point = GetSrcEndPoint(sXdiff, tXdiff, Trg_End_Point);
		//mpDebugPrint("non Last part Trg Start=%d, Trg End=%d, Src Start=%d, Src End=%d", Trg_Start_Point, Trg_End_Point, Src_Start_Point, Src_End_Point);
		pSC->wSourceFilterWidth = (Src_End_Point - Src_Start_Point) + 1 ;
		if(bYUV444)
		{
			pSC->wTargetWidth = ALIGN_CUT_4(Trg_End_Point - Trg_Start_Point + 1);
			pSC->wSourceOutterWidth = ALIGN_4(pSC->wSourceFilterWidth);
		}
		else
		{
			pSC->wTargetWidth = Trg_End_Point - Trg_Start_Point + 1 ;
			pSC->wSourceOutterWidth = ALIGN_2(pSC->wSourceFilterWidth);
		}
		MP_DEBUG("wSourceOutterWidth=%d, wSourceFilterWidth=%d", pSC->wSourceOutterWidth, pSC->wSourceFilterWidth);
		if (pSC->wSourceOutterWidth > pSC->wSourceFilterWidth)
		{
			MP_DEBUG("Need to drop src point");
			pSC->wIPR_Drop_First_Point = pSC->wSourceOutterWidth - pSC->wSourceFilterWidth ;
		}
		else	pSC->wIPR_Drop_First_Point = 0 ;
		pSC->wSourceFilterHeight = sYdiff ;
		pSC->wTargetHeight = tYdiff ;

		pSC->bNeedInitValue = 1 ;
		if(bYUV444)
		{
			src_x = ALIGN_CUT_4(Src_Start_Point + sx1);
			dst_x = ALIGN_CUT_4(Trg_Start_Point + tx1);
		}
		else
		{
			src_x = Src_Start_Point + sx1;
			dst_x = Trg_Start_Point + tx1;
		}
		MP_DEBUG("src_x=%d", src_x);
	    pSC->bMPV = 1 ;
	    if (image_sliceone_scale650(srcWin, trgWin, src_x, src_y, dst_x, dst_y, 0))
	    {
		   pSC->bMPV = 0 ;
		   return FAIL;
	    }
	    pSC->bMPV = 0 ;
	}

}


int image_scale_non_filter(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2,
                           WORD tx1, WORD ty1, WORD tx2, WORD ty2, BYTE Photo_Scaling)
{
	int ret = -1 ;

	if (sx2 > srcWin->wWidth)  sx2 = srcWin->wWidth ;
	if (sy2 > srcWin->wHeight) sy2 = srcWin->wHeight;
	if (tx2 > trgWin->wWidth)  tx2 = trgWin->wWidth ;
	if (ty2 > trgWin->wHeight) ty2 = trgWin->wHeight;


	if ((sx1 >= sx2) || (sy1 >= sy2) || (tx1 >= tx2) || (ty1 >= ty2))
	{
		MP_ALERT("non_filter: -E- scale region error %d->%d %d->%d %d->%d %d->%d", sx1, sx2, sy1, sy2, tx1, tx2, ty1, ty2);
		return FAIL;
	}

	if (srcWin->dwOffset == 0)
	{
		MP_ALERT("-E- srcWin->dwOffset == 0");
		srcWin->dwOffset = ALIGN_16(srcWin->wWidth) << 1;
	}
	if (trgWin->dwOffset == 0)
	{
		MP_ALERT("-E- trgWin->dwOffset == 0");
		trgWin->dwOffset = trgWin->wWidth << 1;
	}

	if (((sx2-sx1)>=(tx2-tx1)) && ((tx2-tx1) > IPU_650_MAX_SCALE_TARGET_SIZE)) // if target size > 720, scale twice
	{
		MP_DEBUG("Scaling Down Target size greater than IPU_650_MAX_SCALE_TARGET_SIZE, need to scale twice ");
		ret = multi_scale_non_filter(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2);
		return ret ;
	}
	/*else if (((sx2-sx1)==(tx2-tx1)) && ((sy2-sy1)==(ty2-ty1)))//In the equal win &
	{
		mpDebugPrint("~~~~~~~~~~~~~~~~~~~~~~~~~Non Scaling~~");
		ret = non_scale_non_filter(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2, 0);
		return ret ;
	}*/
	else if (((sx2-sx1)>=(tx2-tx1)) && ((tx2-tx1) <= IPU_650_MAX_SCALE_TARGET_SIZE))
	{
		MP_DEBUG("Scaling down, but just scale once");
		ret = scale_once_non_filter(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2, 0);
		return ret ;
	}
	else if (((sx2-sx1)<(tx2-tx1)) && ((sx2-sx1)<= IPU_650_MAX_SCALE_TARGET_SIZE))
	{
		MP_DEBUG("Scaling up, just need to scale once");
		ret = scale_once_non_filter(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2, 0);
		return ret ;
	}
	else if (((sx2-sx1)<(tx2-tx1)) && ((sx2-sx1) > IPU_650_MAX_SCALE_TARGET_SIZE))
	{
		MP_DEBUG("Scaling up, but need to scale twice");
		ret = multi_scale_non_filter(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2);
		return ret ;
	}
	else
	{
		MP_DEBUG("Not in rules : Non_filter");
	}
}

#if 1 // jeffery 20100426, later may add to std code 
//   
//   alpha : 0 ~ 256
//
//  	pixelTrg = pixelMain * (alpha /256) + pixelSub * (( 1 -alpha) /256);
// 

int ipu_overlay_650_blend(ST_IMGWIN * srcMainWin, ST_IMGWIN * srcSubWin, ST_IMGWIN * trgWin, WORD tx1, WORD ty1,  unsigned int alpha)
{
	IPU *ipu;
	CHANNEL *dmaR, *dmaW, *overlay;
	BIU *biu;
	CLOCK *clock;
	clock = (CLOCK *) CLOCK_BASE;
	ipu = (IPU *) IPU_BASE;
	dmaR = (CHANNEL *) DMA_SCR_BASE;
	dmaW = (CHANNEL *) DMA_SCW_BASE;
	overlay = (CHANNEL *) DMA_THMW_BASE;
	int ret, t = 0 ;

	ipu_reset();
	//IPR DMA
	ipu->Ipu_reg_A2	= ((DWORD) srcMainWin->pdwStart| 0xA0000000);		//Main win start addr
	ipu->Ipu_reg_1A = 0 ;		//Main win offset
	ipu->Ipu_reg_A4 = ((DWORD) srcSubWin->pdwStart| 0xA0000000);		//Overlay win start addr
	ipu->Ipu_reg_1F = 0 ;		//Overlay win offset
	//IPW DMA
	ipu->Ipu_reg_A3	= ((DWORD) trgWin->pdwStart| 0xA0000000);
	ipu->Ipu_reg_1B = 0 ;
	// IPR
	//Main win
	ipu->Ipu_reg_11 = (((srcMainWin->wWidth - 1) & 0xfff) << 16) + ((srcMainWin->wHeight - 1) & 0xfff);
	ipu->Ipu_reg_10 &= 0xfffffcff ; // Clear Bit8 and Bit9
	//Sub win
	ipu->Ipu_reg_1D = 0x00010000 ;
	ipu->Ipu_reg_1E = ((srcSubWin->wWidth-1)<<16) + (srcSubWin->wHeight-1) ;
	// IPU DMA OUT Direction
	ipu->Ipu_reg_12 = 0x01000000 ;  //to dram
	// Window before IPW
	ipu->Ipu_reg_14 = 0x01000000 ;	//Enable bit 24
	ipu->Ipu_reg_15 = (0 << 16) + (srcMainWin->wHeight - 1) ;
	ipu->Ipu_reg_16 = (0 << 16) + (srcMainWin->wWidth - 1) ;
	//Overlay win settings
#if 0	
	if (colorkey_enable)
		ipu->Ipu_reg_90 = (1<<28) + (1<<24) + (colorkey_Y<<16) + (colorkey_Cb<<8) + colorkey_Cr ;
	else
#endif		
		ipu->Ipu_reg_90 = 0x10000000 ;

	ipu->Ipu_reg_91 = ((ty1&0xfff) << 16) + (tx1&0xfff);
	// Non-Color Key weight for main and sub win
#if 1
//	int alpha;	
//	alpha =  (f_alpha * 256 + 0.5); 
	
	if (alpha > 256) 
		alpha = 256;
	else if (alpha < 0)
		alpha = 0;

//	mpDebugPrint("alpha = %d" , alpha);

	ipu->Ipu_reg_92 = ((alpha) << 16) ; 			// non_colorkey
	ipu->Ipu_reg_93 = ((256 - alpha) << 16);		// non_colorkey



#else
	if (non_colorkey_level == 0)	// Sub win 100 %, Main win 0%
	{
		ipu->Ipu_reg_92 = 0x00000000 ;
		ipu->Ipu_reg_93 = 0x01000000 ;
	}
	else if (non_colorkey_level == 1)	// Sub win 50 %, Main win 50%
	{
		ipu->Ipu_reg_92 = 0x00800000 ;
		ipu->Ipu_reg_93 = 0x00800000 ;
	}
	else if (non_colorkey_level == 2)	// Sub win 0 %, Main win 100%
	{
		ipu->Ipu_reg_92 = 0x01000000 ;
		ipu->Ipu_reg_93 = 0x00000000 ;
	}

	// Color Key weight for main and sub win
	if (colorkey_enable)
	{
		if (colorkey_level == 0)	// Sub win 100 %, Main win 0%
		{
			ipu->Ipu_reg_93 = ipu->Ipu_reg_93 + 0x00000100 ;
		}
		else if (colorkey_level == 1)	// Sub win 50 %, Main win 50%
		{
			ipu->Ipu_reg_92 = ipu->Ipu_reg_92 + 0x00000080 ;    // mainWin * alpha	   ROUND(float_alpha * 256)		     
			ipu->Ipu_reg_93 = ipu->Ipu_reg_93 + 0x00000080 ;    // subWin  * (1 -alpha) 
		}
		else if (colorkey_level == 2)	// Sub win 0 %, Main win 100%
		{
			ipu->Ipu_reg_92 = ipu->Ipu_reg_92 + 0x00000100 ;
		}
	}
#endif	

	// Scaling path
	ipu->Ipu_reg_18 = 0x07070707 ;
	ipu->Ipu_reg_19 = 0x07000000 ;
	// Turn on IPU Frame End
	ipu->Ipu_reg_1C = 0x01000000 ;
	// Main win use 422 format
	ipu->Ipu_reg_10 |= 0x00010000 ;
	// Enable IPU clock and DMA
	clock->MdClken |= BIT6;
	dmaR->Control = 0x00000001;
	dmaW->Control = 0x00000001;
	overlay->Control |= 0x00000001;
	//Print_IPU_Register();
	// Trigger IPU
	ipu->Ipu_reg_10 |= 0x01000000 ;
	IODelay(50);
	// Polling IPU Finish
	while (t < MAX_IPU_WAIT_TIME)
	{
		if (ipu->Ipu_reg_1C & 0x00010000)
		{
			MP_DEBUG("650 IPU Scaling Done");
			ret = 0 ;
			break;
		}
		t++;
	}
	MP_DEBUG("Time t==%d", t);
	if (t >= MAX_IPU_WAIT_TIME )
	{
		ret = FAIL ;
		mpDebugPrint("Scaling Time out");
	}

	return ret ;
}
#endif

static int ipu_overlay_650(ST_IMGWIN * srcMainWin, ST_IMGWIN * srcSubWin, ST_IMGWIN * trgWin, WORD tx1, WORD ty1, BYTE non_colorkey_level, BYTE colorkey_enable, BYTE colorkey_level, BYTE colorkey_Y, BYTE colorkey_Cb, BYTE colorkey_Cr)
{
	IPU *ipu;
	CHANNEL *dmaR, *dmaW, *overlay;
	BIU *biu;
	CLOCK *clock;
	clock = (CLOCK *) CLOCK_BASE;
	ipu = (IPU *) IPU_BASE;
	dmaR = (CHANNEL *) DMA_SCR_BASE;
	dmaW = (CHANNEL *) DMA_SCW_BASE;
	overlay = (CHANNEL *) DMA_THMW_BASE;
	int ret, t = 0 ;

	ipu_reset();
	//IPR DMA
	ipu->Ipu_reg_A2	= ((DWORD) srcMainWin->pdwStart| 0xA0000000);		//Main win start addr
	ipu->Ipu_reg_1A = 0 ;		//Main win offset
	ipu->Ipu_reg_A4 = ((DWORD) srcSubWin->pdwStart| 0xA0000000);		//Overlay win start addr
	ipu->Ipu_reg_1F = 0 ;		//Overlay win offset
	//IPW DMA
	ipu->Ipu_reg_A3	= ((DWORD) trgWin->pdwStart| 0xA0000000);
	ipu->Ipu_reg_1B = 0 ;
	// IPR
	//Main win
	//ipu->Ipu_reg_11 = (((srcMainWin->wWidth - 1) & 0xfff) << 16) + ((srcMainWin->wHeight - 1) & 0xfff);
	//polun modify MP650/MP660 support Width 16bit
	ipu->Ipu_reg_11 = ((srcMainWin->wWidth - 1) << 16) + (srcMainWin->wHeight - 1);
	ipu->Ipu_reg_10 &= 0xfffffcff ; // Clear Bit8 and Bit9
	//Sub win
	ipu->Ipu_reg_1D = 0x00010000 ;
	ipu->Ipu_reg_1E = ((srcSubWin->wWidth-1)<<16) + (srcSubWin->wHeight-1) ;
	// IPU DMA OUT Direction
	ipu->Ipu_reg_12 = 0x01000000 ;  //to dram
	// Window before IPW
	ipu->Ipu_reg_14 = 0x01000000 ;	//Enable bit 24
	ipu->Ipu_reg_15 = (0 << 16) + (srcMainWin->wHeight - 1) ;
	ipu->Ipu_reg_16 = (0 << 16) + (srcMainWin->wWidth - 1) ;
	//Overlay win settings
	if (colorkey_enable)
		ipu->Ipu_reg_90 = (1<<28) + (1<<24) + (colorkey_Y<<16) + (colorkey_Cb<<8) + colorkey_Cr ;
	else
		ipu->Ipu_reg_90 = 0x10000000 ;

	ipu->Ipu_reg_91 = ((ty1&0xfff) << 16) + (tx1&0xfff);
	// Non-Color Key weight for main and sub win
	if (non_colorkey_level == 0)	// Sub win 100 %, Main win 0%
	{
		ipu->Ipu_reg_92 = 0x00000000 ;
		ipu->Ipu_reg_93 = 0x01000000 ;
	}
	else if (non_colorkey_level == 1)	// Sub win 50 %, Main win 50%
	{
		ipu->Ipu_reg_92 = 0x00800000 ;
		ipu->Ipu_reg_93 = 0x00800000 ;
	}
	else if (non_colorkey_level == 2)	// Sub win 0 %, Main win 100%
	{
		ipu->Ipu_reg_92 = 0x01000000 ;
		ipu->Ipu_reg_93 = 0x00000000 ;
	}
	// Color Key weight for main and sub win
	if (colorkey_enable)
	{
		if (colorkey_level == 0)	// Sub win 100 %, Main win 0%
		{
			ipu->Ipu_reg_93 = ipu->Ipu_reg_93 + 0x00000100 ;
		}
		else if (colorkey_level == 1)	// Sub win 50 %, Main win 50%
		{
			ipu->Ipu_reg_92 = ipu->Ipu_reg_92 + 0x00000080 ;
			ipu->Ipu_reg_93 = ipu->Ipu_reg_93 + 0x00000080 ;
		}
		else if (colorkey_level == 2)	// Sub win 0 %, Main win 100%
		{
			ipu->Ipu_reg_92 = ipu->Ipu_reg_92 + 0x00000100 ;
		}
	}
	// Scaling path
	ipu->Ipu_reg_18 = 0x07070707 ;
	ipu->Ipu_reg_19 = 0x07000000 ;
	// Turn on IPU Frame End
	ipu->Ipu_reg_1C = 0x01000000 ;
	// Main win use 422 format
	ipu->Ipu_reg_10 |= 0x00010000 ;
	// Enable IPU clock and DMA
	clock->MdClken |= BIT6;
	dmaR->Control = 0x00000001;
	dmaW->Control = 0x00000001;
	overlay->Control |= 0x00000001;
	//Print_IPU_Register();
	// Trigger IPU
	ipu->Ipu_reg_10 |= 0x01000000 ;
	IODelay(50);
	// Polling IPU Finish
	while (t < MAX_IPU_WAIT_TIME)
	{
		if (ipu->Ipu_reg_1C & 0x00010000)
		{
			MP_DEBUG("650 IPU Scaling Done");
			ret = 0 ;
			break;
		}
		t++;
	}
	MP_DEBUG("Time t==%d", t);
	if (t >= MAX_IPU_WAIT_TIME)
	{
		ret = FAIL ;
		mpDebugPrint("Scaling Time out");
	}

	return ret ;
}

int image_overlay(ST_IMGWIN * srcMainWin, ST_IMGWIN * srcSubWin, ST_IMGWIN * trgWin, WORD tx1, WORD ty1, BYTE non_colorkey_level, BYTE colorkey_enable, BYTE colorkey_level, BYTE colorkey_Y, BYTE colorkey_Cb, BYTE colorkey_Cr)
{
	DWORD MainWidth, MainHeight, OverlayWidth, OverlayHeight ;
	int ret = 1 ;

	MainWidth = srcMainWin->wWidth ;
	MainHeight = srcMainWin->wHeight;
	OverlayWidth = srcSubWin->wWidth ;
	OverlayHeight = srcSubWin->wHeight;

	if ((tx1 + OverlayWidth>MainWidth) || (ty1 + OverlayHeight>MainHeight))
	{
		mpDebugPrint("Out of target region");
		mpDebugPrint("tx1=%d, ty1=%d", tx1, ty1);
		return FAIL;
	}

	//ret = ipu_overlay_position_adjust(srcWin, trgWin, tx1, ty1, tx2, ty2);
	ret = ipu_overlay_650(srcMainWin, srcSubWin, trgWin, tx1, ty1, non_colorkey_level, colorkey_enable, colorkey_level, colorkey_Y, colorkey_Cb, colorkey_Cr);

	return ret ;
}

int Bypass_Overlay(ST_IMGWIN * srcMainWin, ST_IMGWIN * srcSubWin, ST_IMGWIN * trgWin,  BYTE mpv_flag, WORD tx1, WORD ty1, BYTE non_colorkey_level, BYTE colorkey_enable, BYTE colorkey_level, BYTE colorkey_Y, BYTE colorkey_Cb, BYTE colorkey_Cr)
{
	DWORD MainWidth, MainHeight, OverlayWidth, OverlayHeight ;
	int ret = 1 ;

	MainWidth = srcMainWin->wWidth ;
	MainHeight = srcMainWin->wHeight;
	OverlayWidth = srcSubWin->wWidth ;
	OverlayHeight = srcSubWin->wHeight;

	if ((tx1 + OverlayWidth>MainWidth) || (ty1 + OverlayHeight>MainHeight))
	{
		mpDebugPrint("Out of target region");
		mpDebugPrint("tx1=%d, ty1=%d", tx1, ty1);
		return FAIL;
	}

	ret = ipu_overlay_bypass_scale_650(srcMainWin, srcSubWin, trgWin, mpv_flag, tx1, ty1, non_colorkey_level, colorkey_enable, colorkey_level, colorkey_Y, colorkey_Cb, colorkey_Cr);
	return ret ;
}

static int ipu_fill_mem(DWORD * StartAddr, BYTE Value_0, BYTE Value_1, BYTE Value_2, BYTE Value_3, WORD len)
{
	IPU *ipu;
	CHANNEL *dmaR, *dmaW, *overlay;
	BIU *biu;
	CLOCK *clock;
	clock = (CLOCK *) CLOCK_BASE;
	ipu = (IPU *) IPU_BASE;
	dmaR = (CHANNEL *) DMA_SCR_BASE;
	dmaW = (CHANNEL *) DMA_SCW_BASE;
	int ret, t = 0 ;

	ipu_reset();
	ipu->Ipu_reg_90 = (Value_0<<16) + (Value_1<<8) + (Value_2);
	ipu->Ipu_reg_94 = (Value_3<<16);
	ipu->Ipu_reg_91 = (len);
	//IPW DMA
	ipu->Ipu_reg_A3	= (DWORD)StartAddr;
	// Turn on IPU Frame End
	ipu->Ipu_reg_1C = 0x01000000 ;
	// Enable IPU clock and DMA
	clock->MdClken |= BIT6;
	dmaW->Control = 0x00000001;

	mpDebugPrint("ipu->Ipu_reg_91=0x%08x", ipu->Ipu_reg_91);
	// Trigger
	ipu->Ipu_reg_94 |= 0x01000000 ;
	// Polling IPU Finish
	while (t < MAX_IPU_WAIT_TIME)
	{
		if (ipu->Ipu_reg_1C & 0x00010000)
		{
			MP_DEBUG("Fill Value Done");
			ret = 0 ;
			break;
		}
		t++;
	}
	mpDebugPrint("Fill Value Time t==%d", t);
	if (t >= MAX_IPU_WAIT_TIME)
	{
		ret = FAIL ;
		mpDebugPrint("Fill Value out");
	}

	return ret ;
}
#if VIDEO_ON
int image_FillMem(DWORD * StartAddr, BYTE Value_0, BYTE Value_1, BYTE Value_2, BYTE Value_3, WORD len)
{
	int ret = 1 ;
	ret = ipu_fill_mem(StartAddr, Value_0, Value_1, Value_2, Value_3, len);
	return ret ;
}

void Video_Update_Frame(ST_IMGWIN * NewFrame, BYTE mpv_flag)
{
	IPU *ipu;
	ipu = (IPU *) IPU_BASE;
	register IDU *idu = (IDU *)IDU_BASE;


	SystemIntDis(IM_IPU);
	Reset_Video_Frame_Cnt();
	MP_DEBUG("ipu->Ipu_reg_1C=0x%08x", ipu->Ipu_reg_1C);
	ipu->Ipu_reg_A2	= ((DWORD) NewFrame->pdwStart| 0xA0000000);
	if (mpv_flag)
	{
		ipu->Ipu_reg_A0 = ipu->Ipu_reg_A2 + (ALIGN_16(NewFrame->wWidth) * ALIGN_16(NewFrame->wHeight));
	}
    #if ((PANEL_ID != PANEL_HDMI_720P)&&(PANEL_ID != PANEL_ADV7393))
	if(g_bOverlay_Use_Fillwall == 0)
	{
	idu->RST_Ctrl_Bypass |= 0x00005500 ;			// IDU reset one frame
	idu->RST_Ctrl_Bypass &= ~0x00005500 ;
	}
    #endif 
	SystemIntEna(IM_IPU);

	//while(Get_Video_Frame_Cnt()<1);

}
#endif

#if 1
static int multi_scale(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2,
                       WORD tx1, WORD ty1, WORD tx2, WORD ty2)
{
	WORD sXdiff, sYdiff, tXdiff, tYdiff;
	DWORD src_x, dst_x, src_y, dst_y, src_dx, dst_dx ;
	WORD Src_Start_Point = 0, Src_End_Point = 0, Trg_Start_Point = 0, Trg_End_Point = 0 ;
	WORD scale_times = 0 ;
	WORD rest = 0 ;
	int i ;
	BYTE bYUV444 = 0;

	PSCALER_PARA pSC = GetScalerPara();
	ST_SCA_PARM *psScaParm;
	psScaParm = &pSC->sScaParm;
	mmcp_memset((BYTE*)pSC, 0, sizeof(SCALER_PARA));
	//memset((BYTE*)pSC, 0, sizeof(SCALER_PARA));

	sXdiff = (sx2 - sx1);
	sYdiff = (sy2 - sy1);
	tXdiff = (tx2 - tx1);
	tYdiff = (ty2 - ty1);

	src_x = sx1;
	dst_x = tx1;
	src_y = sy1;
	dst_y = ty1;

	pSC->wSourceImageWidth = sXdiff ;
	pSC->wSourceImageHeight = sYdiff ;
	pSC->wTargetImageWidth = tXdiff ;
	pSC->wTargetImageHeight = tYdiff ;

	Get650ScaParm(psScaParm, sXdiff, sYdiff, tXdiff+TOTAL_IPU_650_TRG_EXTRA_POINT, tYdiff+TOTAL_IPU_650_TRG_EXTRA_POINT);

	scale_times = tXdiff / IPU_650_MAX_SCALE_TARGET_SIZE ;
	if (tXdiff % IPU_650_MAX_SCALE_TARGET_SIZE)
		rest = tXdiff - (IPU_650_MAX_SCALE_TARGET_SIZE*scale_times);

	if(srcWin->wType == 444)
	{
	#if YUV444_ENABLE
		bYUV444 = 1;
	#endif
	}

	pSC->bNeedInitValue = 0 ;
	for (i=0 ; i< scale_times ; i++)
	{
		Trg_Start_Point = GetTrgStartPoint(i, IPU_650_MAX_SCALE_TARGET_SIZE);
		Src_Start_Point = GetSrcStartPoint(sXdiff, (tXdiff+TOTAL_IPU_650_TRG_EXTRA_POINT), Trg_Start_Point);
		Trg_End_Point = Trg_Start_Point + IPU_650_MAX_SCALE_TARGET_SIZE + TOTAL_IPU_650_TRG_EXTRA_POINT -1 ;
		Src_End_Point = GetSrcEndPoint(sXdiff, (tXdiff+TOTAL_IPU_650_TRG_EXTRA_POINT), Trg_End_Point);
		mpDebugPrint("Trg Start=%d, Trg End=%d, Src Start=%d, Src End=%d", Trg_Start_Point, Trg_End_Point, Src_Start_Point, Src_End_Point);
		pSC->wTargetWidth = IPU_650_MAX_SCALE_TARGET_SIZE ;
		pSC->wSourceFilterWidth = (Src_End_Point - Src_Start_Point) + 1 ;
		if(bYUV444)
			pSC->wSourceOutterWidth = ALIGN_4(pSC->wSourceFilterWidth);
		else
			pSC->wSourceOutterWidth = ALIGN_2(pSC->wSourceFilterWidth);
		if (pSC->wSourceOutterWidth > pSC->wSourceFilterWidth)
			pSC->wIPR_Drop_First_Point = pSC->wSourceOutterWidth - pSC->wSourceFilterWidth ;
		else	pSC->wIPR_Drop_First_Point = 0 ;
		pSC->wSourceFilterHeight = sYdiff ;
		pSC->wTargetHeight = tYdiff ;

		if (i!=0)	pSC->bNeedInitValue = 1 ;
		src_x = Src_Start_Point + sx1;
		//dst_x = Trg_Start_Point + tx1;
		dst_x = i*IPU_650_MAX_SCALE_TARGET_SIZE ;
		if (image_sliceone_scale650(srcWin, trgWin, src_x, src_y, dst_x, dst_y, 1))
			return FAIL;
	}
	// Last part
	//resr = 0 ;
	if (rest)
	{
		pSC->wIPR_Drop_First_Point = 0;
		if(bYUV444)
		{
			Trg_Start_Point = ALIGN_CUT_4(GetTrgStartPoint(scale_times, IPU_650_MAX_SCALE_TARGET_SIZE));
			Trg_End_Point = ALIGN_CUT_4(tXdiff + TOTAL_IPU_650_TRG_EXTRA_POINT - 1);
		}
		else
		{
			Trg_Start_Point = GetTrgStartPoint(scale_times, IPU_650_MAX_SCALE_TARGET_SIZE);
			Trg_End_Point = tXdiff + TOTAL_IPU_650_TRG_EXTRA_POINT - 1 ;
		}
		Src_Start_Point = GetSrcStartPoint(sXdiff, (tXdiff+TOTAL_IPU_650_TRG_EXTRA_POINT), Trg_Start_Point);
		Src_End_Point = GetSrcEndPoint(sXdiff, (tXdiff+TOTAL_IPU_650_TRG_EXTRA_POINT), Trg_End_Point);
		mpDebugPrint("Last part Trg Start=%d, Trg End=%d, Src Start=%d, Src End=%d", Trg_Start_Point, Trg_End_Point, Src_Start_Point, Src_End_Point);
		pSC->wSourceFilterWidth = (Src_End_Point - Src_Start_Point) + 1 ;
		if(bYUV444)
		{
			pSC->wTargetWidth = ALIGN_CUT_4(rest);
			pSC->wSourceOutterWidth = ALIGN_4(pSC->wSourceFilterWidth);
		}
		else
		{
			pSC->wTargetWidth = rest ;
			pSC->wSourceOutterWidth = ALIGN_2(pSC->wSourceFilterWidth);
		}
		if (pSC->wSourceOutterWidth > pSC->wSourceFilterWidth)
			pSC->wIPR_Drop_First_Point = pSC->wSourceOutterWidth - pSC->wSourceFilterWidth ;
		pSC->wSourceFilterHeight = sYdiff ;
		pSC->wTargetHeight = tYdiff ;

		pSC->bNeedInitValue = 1 ;
		if(bYUV444)
		{
			src_x = ALIGN_CUT_4(Src_Start_Point + sx1);
			dst_x = ALIGN_CUT_4(Trg_Start_Point + tx1);
		}
		else
		{
			src_x = Src_Start_Point + sx1;
			dst_x = Trg_Start_Point + tx1;
		}
		if (image_sliceone_scale650(srcWin, trgWin, src_x, src_y, dst_x, dst_y, 1))
			return FAIL;
	}

}
#endif


static int scale_once(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2,
                      WORD tx1, WORD ty1, WORD tx2, WORD ty2)
{
	WORD sXdiff, sYdiff, tXdiff, tYdiff;
	DWORD src_x, dst_x, src_y, dst_y, src_dx, dst_dx ;
	WORD Src_Start_Point = 0, Src_End_Point = 0, Trg_Start_Point = 0, Trg_End_Point = 0 ;
	WORD scale_times = 0 ;
	WORD rest = 0 ;
	int i ;

	PSCALER_PARA pSC = GetScalerPara();
	ST_SCA_PARM *psScaParm;
	psScaParm = &pSC->sScaParm;
	mmcp_memset((BYTE*)pSC, 0, sizeof(SCALER_PARA));
	//memset((BYTE*)pSC, 0, sizeof(SCALER_PARA));

	sXdiff = (sx2 - sx1);
	sYdiff = (sy2 - sy1);
	tXdiff = (tx2 - tx1);
	tYdiff = (ty2 - ty1);
	MP_DEBUG("scale_once : src_w=%d, src_h=%d, trg_w=%d, trg_h=%d", sXdiff, sYdiff, tXdiff, tYdiff);

	src_x = sx1;
	dst_x = tx1;
	src_y = sy1;
	dst_y = ty1;

	pSC->wSourceImageWidth = sXdiff ;
	pSC->wSourceImageHeight = sYdiff ;
	pSC->wTargetImageWidth = tXdiff ;
	pSC->wTargetImageHeight = tYdiff ;

	Get650ScaParm(psScaParm, sXdiff, sYdiff, tXdiff+TOTAL_IPU_650_TRG_EXTRA_POINT, tYdiff+TOTAL_IPU_650_TRG_EXTRA_POINT);
	pSC->bNeedInitValue = 0 ;

	Trg_Start_Point = 0;
	Src_Start_Point = GetSrcStartPoint(sXdiff, (tXdiff+TOTAL_IPU_650_TRG_EXTRA_POINT), Trg_Start_Point);
	Trg_End_Point = Trg_Start_Point + tXdiff + TOTAL_IPU_650_TRG_EXTRA_POINT -1 ;
	Src_End_Point = GetSrcEndPoint(sXdiff, (tXdiff+TOTAL_IPU_650_TRG_EXTRA_POINT), Trg_End_Point);
	MP_DEBUG("Trg Start=%d, Trg End=%d, Src Start=%d, Src End=%d", Trg_Start_Point, Trg_End_Point, Src_Start_Point, Src_End_Point);
	pSC->wTargetWidth = tXdiff ;
	pSC->wSourceFilterWidth = (Src_End_Point - Src_Start_Point) + 1 ;
	pSC->wSourceOutterWidth = ALIGN_2(pSC->wSourceFilterWidth);
	if (pSC->wSourceOutterWidth > pSC->wSourceFilterWidth)
		pSC->wIPR_Drop_First_Point = pSC->wSourceOutterWidth - pSC->wSourceFilterWidth ;
	pSC->wSourceFilterHeight = sYdiff ;
	pSC->wTargetHeight = tYdiff ;

	src_x = Src_Start_Point + sx1;
	dst_x = Trg_Start_Point + tx1;
	if (image_sliceone_scale650(srcWin, trgWin, src_x, src_y, dst_x, dst_y, 1))
		return FAIL;
}

int image_scale(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2,
                WORD tx1, WORD ty1, WORD tx2, WORD ty2)
{
	BYTE HW_Chasing_enable = 0 ;

	if (sx2 > srcWin->wWidth)  sx2 = srcWin->wWidth ;
	if (sy2 > srcWin->wHeight) sy2 = srcWin->wHeight;
	if (tx2 > trgWin->wWidth)  tx2 = trgWin->wWidth ;
	if (ty2 > trgWin->wHeight) ty2 = trgWin->wHeight;
	MP_DEBUG("image_scale");
	if ((sx1 >= sx2) || (sy1 >= sy2) || (tx1 >= tx2) || (ty1 >= ty2))
	{
		MP_ALERT("scale: -E- scale region error %d->%d %d->%d %d->%d %d->%d", sx1, sx2, sy1, sy2, tx1, tx2, ty1, ty2);
		return FAIL;
	}

	if (srcWin->dwOffset == 0)
	{
		MP_ALERT("-E- srcWin->dwOffset == 0");
		srcWin->dwOffset = ALIGN_16(srcWin->wWidth) << 1;
	}
	if (trgWin->dwOffset == 0)
	{
		MP_ALERT("-E- trgWin->dwOffset == 0");
		trgWin->dwOffset = trgWin->wWidth << 1;
	}
	MP_DEBUG("srcWin->dwOffset = %d, trgWin->dwOffset = %d", srcWin->dwOffset, trgWin->dwOffset);

	if (((sx2-sx1)>=(tx2-tx1)) && ((tx2-tx1) > IPU_650_MAX_SCALE_TARGET_SIZE)) // if target size > 720, scale twice
	{
		MP_DEBUG("Scaling Down Target size greater than 720, need to scale twice ");
		return multi_scale(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2);
	}
	else if (((sx2-sx1)>=(tx2-tx1)) && ((tx2-tx1) <= IPU_650_MAX_SCALE_TARGET_SIZE))
	{
		MP_DEBUG("Scaling down, but just scale once");
		return scale_once(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2);
	}
	else if ((sx2-sx1) <= IPU_650_MAX_SCALE_TARGET_SIZE)
	{
		MP_DEBUG("Panel size less than 720, just only scale once ");
		//return scale_once(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2, HW_Chasing_enable);
		return scale_once(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2);
	}
	else
	{
		MP_DEBUG("Not in rules");
	}
}

int Auto_Chase_Scaling(BYTE *pbSource, BYTE *pbTarget, WORD ImageWidth, WORD ImageHeight, WORD JPGWidth, BYTE ResizeRatio, WORD ImageType)
{
	IPU *ipu;
	CHANNEL *dmaR, *dmaW;
	BIU *biu;
	CLOCK *clock;
	CHANNEL *dma;
	register IDU *idu = (IDU *) IDU_BASE;
	dma = (CHANNEL *) (DMA_IDU_BASE);
	clock = (CLOCK *) CLOCK_BASE;
	ipu = (IPU *) IPU_BASE;
	dmaR = (CHANNEL *) DMA_SCR_BASE;
	dmaW = (CHANNEL *) DMA_SCW_BASE;
	DWORD TrgWidth, TrgHeight ;


	PSCALER_PARA pSC = GetScalerPara();
	ST_SCA_PARM *psScaParm;
	psScaParm = &pSC->sScaParm;
	mmcp_memset((BYTE*)pSC, 0, sizeof(SCALER_PARA));
	//memset((BYTE*)pSC, 0, sizeof(SCALER_PARA));

	TrgWidth = ALIGN_2(ImageWidth>>ResizeRatio) ;
	TrgHeight = (ImageHeight>>ResizeRatio) ;

	MP_DEBUG("ImageWidth=%d, ImageHeight=%d", ImageWidth, ImageHeight);
	Get650ScaParm(psScaParm, ALIGN_CUT_2(ImageWidth>>3), ImageHeight>>3, TrgWidth, TrgHeight);

	MP_DEBUG("Chasing Src_W=%d, Src_H=%d", ImageWidth>>3, ImageHeight>>3);
	MP_DEBUG("Chasing Trg_W=%d, Trg_H=%d", TrgWidth, TrgHeight);
	ipu_reset();
	//IPR DMA
	ipu->Ipu_reg_A2	= ((DWORD) pbSource| 0xA0000000);
	if (ImageType==444)
	{
#if	YUV444_ENABLE
		ipu->Ipu_reg_1A = ((ALIGN_4(JPGWidth>>3) - (ImageWidth>>3))*3);
#else
		ipu->Ipu_reg_1A = ((ALIGN_2(JPGWidth>>3) - (ALIGN_CUT_2(ImageWidth>>3)))<<1);
#endif
	}
	else
	{
		ipu->Ipu_reg_1A = ((ALIGN_2(JPGWidth>>3) - (ALIGN_CUT_2(ImageWidth>>3)))<<1);
	}
	//IPW DMA
	ipu->Ipu_reg_A3	= ((DWORD) pbTarget| 0xA0000000);
	ipu->Ipu_reg_1B = 0 ;

	// Fill IPU Registers ++
	// IPR
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	//polun modify MP650/MP660 support Width 16bit
	ipu->Ipu_reg_11 = ((ALIGN_CUT_2(ImageWidth>>3) - 1) << 16) + ((ImageHeight>>3) - 1);
#else
	ipu->Ipu_reg_11 = (((ALIGN_CUT_2(ImageWidth>>3) - 1) & 0xfff) << 16) + (((ImageHeight>>3) - 1) & 0xfff);
#endif
	ipu->Ipu_reg_10 &= 0xfffffcff ; // Clear Bit8 and Bit9
	// IPU DMA OUT Direction
	ipu->Ipu_reg_12 = 0x01000000 ;  //to dram
	// IPU DMA OUT Format
	if (ImageType == 444) // Jpeg 444 format
	{
#if YUV444_ENABLE
		ipu->Ipu_reg_12 |= 0x00010000 ;	//444 format out
#endif
	}
	// Window before IPW
	ipu->Ipu_reg_14 = 0x01000000 ;	//Enable bit 24
	//Always non_filter
	{
		ipu->Ipu_reg_15 = (0 << 16) + (TrgHeight - 1) ;
		ipu->Ipu_reg_16 = (0 << 16) + (TrgWidth - 1) ;
	}

	// Scaling path
	ipu->Ipu_reg_18 = 0x00070707 ;
	ipu->Ipu_reg_19 = 0x07020000 ;
	// Turn on IPU Frame End
	ipu->Ipu_reg_1C = 0x01000000 ;
	// IPU Scaling factor
	int nResizeRatio = ResizeRatio-3;
	if (psScaParm->wHSF == 0)
	{
		ipu->Ipu_reg_22 = 0x80000000 ;
		if(nResizeRatio >= 3)
			psScaParm->wHSubRatio = 7;
		else if(nResizeRatio == 2)
			psScaParm->wHSubRatio = 3;
		else if(nResizeRatio == 1)
			psScaParm->wHSubRatio = 1;
		else
			psScaParm->wHSubRatio = 0;
	}
	else
		ipu->Ipu_reg_22 = (psScaParm->wHSF << 16) ;

	//if (psScaParm->wVSF == 0)
		ipu->Ipu_reg_23 = 0x80000000 ;		// V only subsample as chasing
	//else
	//	ipu->Ipu_reg_23 = (psScaParm->wVSF << 16) ;
	// Edge processing and S/C value
	// IPU Subsample
	ipu->Ipu_reg_21 = (psScaParm->wVSubRatio << 8) + psScaParm->wHSubRatio ;
	ipu->Ipu_reg_26 = 0x03000000 ;
	ipu->Ipu_reg_27 = 0x00020202 ;
	// HBIL_back
	if (psScaParm->wHUp)		// Scaling up
		ipu->Ipu_reg_20 = 0x00000001 ;
	//ipu->Ipu_reg_20 |= 0x00000002 ;
	// IPR Source Type
	mpDebugPrint("type_flag=%d", ImageType);
	if (ImageType == 444) // Jpeg 444 format
	{
		mpDebugPrint("This photo is 444 type");
		ipu->Ipu_reg_10 &= 0xfffcffff ;
#if YUV444_ENABLE
		ipu->Ipu_reg_10 |= 0x00020000 ;
#else
		ipu->Ipu_reg_10 |= 0x00010000 ;
#endif
	}
	else
		ipu->Ipu_reg_10 |= 0x00010000 ;

	//Print_IPU_Register();
	// Enable IPU clock and DMA
	clock->MdClken |= BIT6;
	dmaR->Control = 0x00000001;
	dmaW->Control = 0x00000001;
	// Trigger IPU
	ipu->Ipu_reg_10 |= 0x01000000 ;
	// Fill IPU Registers --
	IODelay(5);

	return 0 ;
}

#if CHASE_IN_DECODE
int Chase_Scaling(BYTE *pbSource, BYTE *pbTarget, WORD ImageWidth, WORD ImageHeight, WORD TrgWidth, WORD TrgHeight, WORD ImageType)
{
	IPU *ipu;
	CHANNEL *dmaR, *dmaW;
	BIU *biu;
	CLOCK *clock;
	CHANNEL *dma;
	register IDU *idu = (IDU *) IDU_BASE;
	dma = (CHANNEL *) (DMA_IDU_BASE);
	clock = (CLOCK *) CLOCK_BASE;
	ipu = (IPU *) IPU_BASE;
	dmaR = (CHANNEL *) DMA_SCR_BASE;
	dmaW = (CHANNEL *) DMA_SCW_BASE;


	PSCALER_PARA pSC = GetScalerPara();
	ST_SCA_PARM *psScaParm;
	psScaParm = &pSC->sScaParm;
	mmcp_memset((BYTE*)pSC, 0, sizeof(SCALER_PARA));


	MP_DEBUG("ImageWidth=%d, ImageHeight=%d", ImageWidth, ImageHeight);
	Get650ScaParm(psScaParm, ALIGN_CUT_2(ImageWidth), ImageHeight, TrgWidth, TrgHeight);

	MP_DEBUG("Chasing Src_W=%d, Src_H=%d", ImageWidth, ImageHeight);
	MP_DEBUG("Chasing Trg_W=%d, Trg_H=%d", TrgWidth, TrgHeight);
	ipu_reset();
	//IPR DMA
	ipu->Ipu_reg_A2	= ((DWORD) pbSource| 0xA0000000);
	ipu->Ipu_reg_1A = 0;
	//IPW DMA
	ipu->Ipu_reg_A3	= ((DWORD) pbTarget| 0xA0000000);
	ipu->Ipu_reg_1B = 0 ;

	// Fill IPU Registers ++
	// IPR
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	//polun modify MP650/MP660 support Width 16bit
	ipu->Ipu_reg_11 = ((ALIGN_CUT_2(ImageWidth) - 1) << 16) + ((ImageHeight) - 1);
#else
	ipu->Ipu_reg_11 = (((ALIGN_CUT_2(ImageWidth) - 1) & 0xfff) << 16) + (((ImageHeight) - 1) & 0xfff);
#endif
	ipu->Ipu_reg_10 &= 0xfffffcff ; // Clear Bit8 and Bit9
	// IPU DMA OUT Direction
	ipu->Ipu_reg_12 = 0x01000000 ;  //to dram
	// IPU DMA OUT Format
	if (ImageType == 444) // Jpeg 444 format
	{
#if YUV444_ENABLE
		ipu->Ipu_reg_12 |= 0x00010000 ;	//444 format out
#endif
	}
	// Window before IPW
	ipu->Ipu_reg_14 = 0x01000000 ;	//Enable bit 24
	//Always non_filter
	{
		ipu->Ipu_reg_15 = (0 << 16) + (TrgHeight - 1) ;
		ipu->Ipu_reg_16 = (0 << 16) + (TrgWidth - 1) ;
	}

	// Scaling path
	ipu->Ipu_reg_18 = 0x00070707 ;
	ipu->Ipu_reg_19 = 0x07020000 ;
	// Turn on IPU Frame End
	ipu->Ipu_reg_1C = 0x01000000 ;
	// IPU Scaling factor
	if (psScaParm->wHSF == 0)
	{
		ipu->Ipu_reg_22 = 0x80000000 ;
		psScaParm->wHSubRatio = 0;
	}
	else
		ipu->Ipu_reg_22 = (psScaParm->wHSF << 16) ;

	//if (psScaParm->wVSF == 0)
		ipu->Ipu_reg_23 = 0x80000000 ;		// V only subsample as chasing
	//else
	//	ipu->Ipu_reg_23 = (psScaParm->wVSF << 16) ;
	// Edge processing and S/C value
	// IPU Subsample
	ipu->Ipu_reg_21 = (psScaParm->wVSubRatio << 8) + psScaParm->wHSubRatio ;
	ipu->Ipu_reg_26 = 0x03000000 ;
	ipu->Ipu_reg_27 = 0x00020202 ;
	// HBIL_back
	if (psScaParm->wHUp)		// Scaling up
		ipu->Ipu_reg_20 = 0x00000001 ;
	//ipu->Ipu_reg_20 |= 0x00000002 ;
	// IPR Source Type
	//mpDebugPrint("type_flag=%d", ImageType);
	if (ImageType == 444) // Jpeg 444 format
	{
		mpDebugPrint("This photo is 444 type");
		ipu->Ipu_reg_10 &= 0xfffcffff ;
#if YUV444_ENABLE
		ipu->Ipu_reg_10 |= 0x00020000 ;
#else
		ipu->Ipu_reg_10 |= 0x00010000 ;
#endif
	}
	else
		ipu->Ipu_reg_10 |= 0x00010000 ;

	//Print_IPU_Register();
	// Enable IPU clock and DMA
	clock->MdClken |= BIT6;
	dmaR->Control = 0x00000001;
	dmaW->Control = 0x00000001;
	// Trigger IPU
	ipu->Ipu_reg_10 |= 0x01000000 ;
	// Fill IPU Registers --
	IODelay(5);

	return 0 ;
}
#endif

int Bypass_Overlay_Show_Main_Win()
{
	register IPU *ipu = (IPU *)IPU_BASE;

	// Sub win 0 %, Main win 100%
	ipu->Ipu_reg_92 = 0x01000000 ;
	ipu->Ipu_reg_93 = 0x00000000 ;

}


int Bypass_Show_WallPaper()
{
	register IPU *ipu = (IPU *)IPU_BASE;

	// Sub win 0 %, Main win 100%
	ipu->Ipu_reg_92 = 0x01000000 ;
	ipu->Ipu_reg_93 = 0x00000000 ;

}


#endif	//#if ((CHIP_VER & 0xffff0000) == CHIP_VER_650)
