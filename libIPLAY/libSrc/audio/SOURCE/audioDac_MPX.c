#define LOCAL_DEBUG_ENABLE 1

/*
// Include section 
*/
#include "global612.h"
#include "audio_hal.h"
#include "mpTrace.h"
#include "peripheral.h"


#if (AUDIO_DAC == DAC_INTERNAL)
/**
 ***************************************************************
 *
 * Config M6xx clock and gpio
 *
 *  Input  : none.
 *
 *  Output : none. 
 ***************************************************************
*/

void MPXDAC_uninit()
{
	register AIU *aiu;
	aiu = (AIU *) (AIU_BASE);
	//AIU_PLAYBACK_IN_DISABLE();
}

void MPXDAC_setSamplerate(WORD samplerate)
{
	register CLOCK *audio_clock;
	register AIU *aiu;
	aiu = (AIU *) (AIU_BASE);
	audio_clock = (CLOCK *) (CLOCK_BASE);
	int samplerate_range = 0;
	int PLL2CLOCK = (Clock_PllFreqGet(CLOCK_PLL2_INDEX)/1000000);
	
	if (AUDIO_DAC_USING_PLL2)
	{
		if ( (PLL2CLOCK == 96) || (PLL2CLOCK == 120) || (PLL2CLOCK == 138) || (PLL2CLOCK == 153))
			mpDebugPrint("MPXDAC(DAC_INTERNAL) using PLL2 Clock: %d", PLL2CLOCK);
		else
		{
			goto not_supported_samplerate;
		}
	}
	else
		mpDebugPrint("MPXDAC(DAC_INTERNAL) using PLL3");

	mpDebugPrint("MPXDAC(DAC_INTERNAL) Samplerate: %d", samplerate);
	//note: AIU_SET_LRCLOCK_LEN is fixed, always (0xFF);

	if (samplerate < 20000 )
	{
		if (samplerate < 9900 )
			samplerate_range = 1;//((samplerate > 100 )  && (samplerate < 9900 ))
		else
			samplerate_range = 2;//((samplerate >= 9900 )  && (samplerate < 20000 ))
	}
	else
	{
		if (samplerate < 40000 )
			samplerate_range = 3;//((samplerate >= 20000 )  && (samplerate < 40000 ))
		else
			samplerate_range = 4;//((samplerate >= 40000 )  && (samplerate < 100000 ))
	}
	
	switch( samplerate_range )
	{
	case 1 :
		if ((samplerate > 100 )  && (samplerate < 900 ))	// Set 0.8khz status
		{
			#if !AUDIO_DAC_USING_PLL2
					audio_clock->PLL3Cfg = 0x0077230C;
					audio_clock->AudClkC = 0x00001620;//0.80605khz
			#else
				goto not_supported_samplerate;
			#endif
		}
		else if ((samplerate > 900 )  && (samplerate < 1800))	// Set 1.6khz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x00771714;
				audio_clock->AudClkC = 0x00001440;//1.640khz
			#else
				goto not_supported_samplerate;
			#endif
		}
		else if ((samplerate > 1800)  && (samplerate < 2700))	// Set 2.4khz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x00770F14;
				audio_clock->AudClkC = 0x00001440;//2.460khz
			#else
				goto not_supported_samplerate;
			#endif
		}
		else if ((samplerate > 2700)  && (samplerate < 3600))	// Set 3.2khz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x00771423;
				audio_clock->AudClkC = 0x00001440;//3.214khz
			#else
				goto not_supported_samplerate;
			#endif
		}
		else if ((samplerate > 3600)  && (samplerate < 4500))	// Set 4khz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x0077142C;
				audio_clock->AudClkC = 0x00001440;//4.017khz
			#else
				goto not_supported_samplerate;
			#endif
		}
		else if ((samplerate > 4500)  && (samplerate < 5400))	// Set 4.8khz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x00771435;
				audio_clock->AudClkC = 0x00001440;//4.821khz
			#else
				goto not_supported_samplerate;
			#endif
		}
		else if ((samplerate > 5400)  && (samplerate < 6300))	// Set 5.6khz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x0077143E;
				audio_clock->AudClkC = 0x00001440;//5.625khz
			#else
				goto not_supported_samplerate;
			#endif
		}
		else if ((samplerate > 6300)  && (samplerate < 7000))	// Set 6.4khz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x00771447;
				audio_clock->AudClkC = 0x00001440;//6.428khz
			#else
				goto not_supported_samplerate;
			#endif
		}
		else if ((samplerate > 7000)  && (samplerate < 7900))	// Set 7.8kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x007718c7;
				//audio_clock->AudClkC = 0x00001120;
				audio_clock->AudClkC = 0x00001570;//7.812khz
			#else
				if (PLL2CLOCK == 138)
					audio_clock->AudClkC = 0x00001469;//7.7khz
				else
					goto not_supported_samplerate;
			#endif
		}
		else if ( (samplerate > 7900)  && (samplerate < 8900))	//# Set 8kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x007718cc;
				//audio_clock->AudClkC = 0x00001120;
				audio_clock->AudClkC = 0x00001570;//8.007khz
			#else
				if 		(PLL2CLOCK == 96)
					audio_clock->AudClkC = 0x000013b8;//7.8125khz
				else if (PLL2CLOCK == 120)
					audio_clock->AudClkC = 0x000010bc;//120/5/12/1/256=7.8125khz
				else if (PLL2CLOCK == 138)
					audio_clock->AudClkC = 0x000015a8;//8.167khz
				else if (PLL2CLOCK == 153)
					audio_clock->AudClkC = 0x000014e8;//153/1/15/5/256=7.968875khz
				else
					goto not_supported_samplerate;
			#endif
		}
		else if ( (samplerate > 8900)  && (samplerate < 9900))	// Set 9kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x007715ca;
				//audio_clock->AudClkC = 0x00001120;
				audio_clock->AudClkC = 0x00001570;//9.011khz
			#else
				goto not_supported_samplerate;
			#endif
		}
		break;

	case 2:
		if ( (samplerate >= 9900)  && (samplerate < 10900))	// Set 10.125kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x007718ab;
				//audio_clock->AudClkC = 0x00001120;
				audio_clock->AudClkC = 0x00001560;//10.103khz
			#else
				goto not_supported_samplerate;
			#endif
		}
		else if((samplerate >= 10900) && (samplerate < 11300))	//# Set 11.025kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x0077635D;
				//audio_clock->PLL3Cfg = 0x007718cc;
				audio_clock->AudClkC = 0x00001030;
			#else 
				if 		(PLL2CLOCK == 96 )
					audio_clock->AudClkC = 0x00001468;//10.714khz
				else if (PLL2CLOCK == 120)
					audio_clock->AudClkC = 0x0000116A;//120/3/7/2/256=11.160khz
				else if (PLL2CLOCK == 138)
					audio_clock->AudClkC = 0x0000117A;//11.23khz
				else if (PLL2CLOCK == 153)
					audio_clock->AudClkC = 0x00001588;//153/1/9/6/256=11.067khz
				else
					goto not_supported_samplerate;
			#endif
		}
		else if((samplerate >= 11300) && (samplerate < 12300))	//# Set 12kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x007718cc;
				audio_clock->AudClkC = 0x00001370;
			#else
				if 		(PLL2CLOCK == 96 )
					audio_clock->AudClkC = 0x0000137a;//11.718khz
				else if (PLL2CLOCK == 120)
					audio_clock->AudClkC = 0x000010ca;//120/3/13/1/256=12.019khz
				else if (PLL2CLOCK == 138)
					audio_clock->AudClkC = 0x0000124a;//138/3/5/3/256=11.97khz
				else if (PLL2CLOCK == 153)
					audio_clock->AudClkC = 0x00001498;//153/1/10/5/256=11.95khz
				else
					goto not_supported_samplerate;
			#endif
		}
		else if((samplerate >= 12300) && (samplerate < 14000))	// Set 12.8kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x007710c2;
				audio_clock->AudClkC = 0x00001560;//12.801khz
			#else
				goto not_supported_samplerate;
			#endif
		}
		else if((samplerate >= 14000) && (samplerate < 15000))	// Set 14.4kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x00770CA7;
				audio_clock->AudClkC = 0x00001560;//14.423khz
			#else
				goto not_supported_samplerate;
			#endif
		}
		else if((samplerate >= 15000) && (samplerate < 17000))	// #Set 16kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x0077D74A;
				audio_clock->AudClkC = 0x00001000;//16.276khz
			#else
				if 		(PLL2CLOCK == 96 )
					audio_clock->AudClkC = 0x000011b8;//15.625khz
				else if (PLL2CLOCK == 120)
					audio_clock->AudClkC = 0x000010e9;//120/2/15/1/256=15.625khz
				else if (PLL2CLOCK == 138)
					audio_clock->AudClkC = 0x00001339;//16.84khz
				else if (PLL2CLOCK == 153)
					audio_clock->AudClkC = 0x000012b8;//153/1/12/3/256=16.601khz
				else
					goto not_supported_samplerate;
			#endif
		}
		else if((samplerate >= 17000) && (samplerate < 18000))	// Set 17.64kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x0077D952;
				audio_clock->AudClkC = 0x00001000;//17.846khz		
			#else
				goto not_supported_samplerate;
			#endif
		}
		else if((samplerate >= 18000) && (samplerate < 20000))	// Set 19.845kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x0077DA5C;
				audio_clock->AudClkC = 0x00001000;//19.905khz		
			#else
				goto not_supported_samplerate;
			#endif
		}
		break;
		
	case 3:
		if((samplerate >= 20000) && (samplerate < 23000))	// #Set 22.05 kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x0077635D;
				//audio_clock->PLL3Cfg = 0x007718cc;
				audio_clock->AudClkC = 0x00001010;//22.031khz
			#else
				if 		(PLL2CLOCK == 96 )
					audio_clock->AudClkC = 0x000010f8;//23.4375khz
				else if (PLL2CLOCK == 120)
					audio_clock->AudClkC = 0x0000106a;//120/3/7/1/256=22.3214khz
				else if (PLL2CLOCK == 138)
					audio_clock->AudClkC = 0x0000131a;//138/3/2/4/256=22.46khz
				else if (PLL2CLOCK == 153)
					audio_clock->AudClkC = 0x00001288;//153/1/9/3/256=22.135khz
				else
					goto not_supported_samplerate;
			#endif
		}
		else if((samplerate >= 23000) && (samplerate < 28000))	// #Set 24kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x007718cc;
				audio_clock->AudClkC = 0x00001170;	//24.023khz	
			#else
				if 		(PLL2CLOCK == 96 )
					audio_clock->AudClkC = 0x000010f8;//23.4375khz
				else if (PLL2CLOCK == 120)
					audio_clock->AudClkC = 0x00001198;//120/1/10/2/256=23.4375khz
				else if (PLL2CLOCK == 138)
					audio_clock->AudClkC = 0x000011a8;//138/1/11/2/256=24.5khz
				else if (PLL2CLOCK == 153)
					audio_clock->AudClkC = 0x00001448;//153/1/5/5/256=23.906khz
				else
					goto not_supported_samplerate;
			#endif
		}
		else if((samplerate >= 28000) && (samplerate < 30000))	// Set 28.8kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x007714ce;
				audio_clock->AudClkC = 0x00001170;	//28.878khz	
			#else
				goto not_supported_samplerate;
			#endif
		}
		else if((samplerate >= 30000) && (samplerate < 33000))	// #Set 32kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x0077D795;
				audio_clock->AudClkC = 0x00001000;//32.552khz
			#else
				if 		(PLL2CLOCK == 96 )
					audio_clock->AudClkC = 0x000010b8;//31.25khz
				else if (PLL2CLOCK == 120)
					audio_clock->AudClkC = 0x000010e8;//31.25khz
				else if (PLL2CLOCK == 138)
					audio_clock->AudClkC = 0x00001178;//33.69khz
				else if (PLL2CLOCK == 153)
					audio_clock->AudClkC = 0x00001188;//33.203khz
				else
					goto not_supported_samplerate;
			#endif
		}
		else if((samplerate >= 33000) && (samplerate < 36000))	// Set 35.28kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x0077D6A1;
				audio_clock->AudClkC = 0x00001000;//35.319khz
			#else
				goto not_supported_samplerate;
			#endif
		}
		else if((samplerate >= 36000) && (samplerate < 40000))	// Set 38.4kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x0077D6AF;
				audio_clock->AudClkC = 0x00001000;//38.372khz
			#else
				goto not_supported_samplerate;
			#endif
		}
		break;
		
	case 4:
		if((samplerate >= 40000) && (samplerate < 46000))	// #Set 44.1kHz status
		{			
			#if (!AUDIO_DAC_USING_PLL2)
				audio_clock->PLL3Cfg = 0x0077645E;
				//audio_clock->PLL3Cfg = 0x007718cc;
				//audio_clock->AudClkC = 0x000010B8;
				audio_clock->AudClkC = 0x00001000;//44.09khz
			#else
				if 		(PLL2CLOCK == 96 )
					audio_clock->AudClkC = 0x00001088;//96/1/9/1/256=41.66khz
				else if (PLL2CLOCK == 120)
					audio_clock->AudClkC = 0x000010a8;//120/1/12/1/256=42.61khz
				else if (PLL2CLOCK == 138)
					audio_clock->AudClkC = 0x00001158;//138/1/6/2/256=44.92khz
				else if (PLL2CLOCK == 153)
					audio_clock->AudClkC = 0x000010d8;//153/1/14/1/256=42.689khz
				else
					goto not_supported_samplerate;
			#endif
		}
		else if((samplerate >= 46000) && (samplerate < 55000))	// #Set 48kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x007718cc;
				audio_clock->AudClkC = 0x00001070;//48.046khz
			#else
				if 		(PLL2CLOCK == 96 )
					audio_clock->AudClkC = 0x00001078;//46.875khz
				else if (PLL2CLOCK == 120)
					audio_clock->AudClkC = 0x00001098;//120/1/10/1/256=46.875khz
				else if (PLL2CLOCK == 138)
					audio_clock->AudClkC = 0x000010a8;//138/1/11/1/256=49khz
				else if (PLL2CLOCK == 153)
					audio_clock->AudClkC = 0x000010b8;//153/1/12/1/256=49.80khz
				else
					goto not_supported_samplerate;
			#endif
		}
		else if((samplerate >= 55000) && (samplerate < 64000))	// Set 57.6kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x007718D6;
				audio_clock->AudClkC = 0x00001060;//57.589khz
			#else
				goto not_supported_samplerate;
			#endif
		}
		else if((samplerate >= 64000) && (samplerate < 70000))	// Set 67.2kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x007712BE;
				audio_clock->AudClkC = 0x00001060;//67.316khz
			#else
				goto not_supported_samplerate;
			#endif
		}
		else if((samplerate >= 70000) && (samplerate < 80000))	// Set 76.8kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x007710C2;
				audio_clock->AudClkC = 0x00001060;//76.811khz
			#else
				goto not_supported_samplerate;
			#endif
		}
		else if((samplerate >= 80000) && (samplerate < 89000))	// Set 86.4kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x00770CA7;
				audio_clock->AudClkC = 0x00001060;//86.538khz
			#else
				goto not_supported_samplerate;
			#endif
		}
		else if((samplerate >= 90000) && (samplerate < 97000))	// #Set 96kHz status
		{
			#if !AUDIO_DAC_USING_PLL2
				audio_clock->PLL3Cfg = 0x007718cc;
				audio_clock->AudClkC = 0x00001030;//96.09khz
			#else
				if 		(PLL2CLOCK == 96 )
					audio_clock->AudClkC = 0x00001038;//93.75khz
				else if (PLL2CLOCK == 120)
					audio_clock->AudClkC = 0x00001048;//120/1/3/2/256=93.75khz
				else if (PLL2CLOCK == 138)
					audio_clock->AudClkC = 0x00001128;//138/1/3/2/256=89.84khz
				else if (PLL2CLOCK == 153)
					audio_clock->AudClkC = 0x00001058;//153/1/6/1/256=99.609khz
				else
					goto not_supported_samplerate;
			#endif
		}
		break;
		
		
	case 0 :
	default :
	not_supported_samplerate:
		if (AUDIO_DAC_USING_PLL2)
		{
			MP_ALERT("MPXDAC(DAC_INTERNAL) doesn't support PLL2 %d, only supports PLL2: 96, 120, 138, 153.", PLL2CLOCK);
		}
		MP_ALERT("Samplerate %d is not supported by IIS.(Only supports 8K--96K )", samplerate, PLL2CLOCK);
		break;

}
#if !AUDIO_DAC_USING_PLL2	
		g_psClock->Clkss_EXT1 &= 0xfffffff0;
		g_psClock->Clkss_EXT1 |= (BIT3);	// Using PLL3/1 to be clock source
		g_psClock->ClkCtrl &= ~0x01000000;		// Disable PLL3
		g_psClock->ClkCtrl |= 0x01000000;		// enable PLL3
#else
		g_psClock->Clkss_EXT1 &= 0xfffffff0; //for using PLL2, eddy 2011.11.02
#endif


#if (HAVE_AMP_MUTE)	
	g_psAiu->AiuCtl    |= 0x18000000;	//RAM_RESET
	//g_psAiu->AiuTopanaCtl |= 0x00008000;
	TimerDelay(1000);

	g_psAiu->AiuCtl |= 0x20000000;

	g_psAiu->AiuTopanaCtl =  0x00200000;
	g_psAiu->AiuDacCtl = 0x00000013;
	
	while(!(g_psAiu->AiuTopanaCtl & 0x00400000));

	g_psAiu->AiuTopanaCtl &=  ~0x00200000;
	g_psAiu->AiuTopanaCtl |=   0x00008200;
#endif
	
	AIUCLOCK_DISABLE();
	AIU_MAINCLOCK_ENABLE();
	//MPXDAC_setVolume(0);
	//MPXDAC_Set_OutputSource();
	//g_psAiu->AiuDacCtl = 0x00000013; //why set 0x00000013? Bit2 is reserved
	//MPXDAC_Set_OutputSource();
	//TimerDelay(8);
	//IODelay(200);
	/*	while(1)
			{
				if (((g_psAiu->AiuDacCtl>>3) & 0x1))
					break;
			}
		g_psAiu->AiuTopanaCtl &= ~0x00200000;
	*/
}


/*
 *	This function is used to set volume degree of internal dac
 *
 *  MPX supports line buffer only since mp650 series
 *
 * @retval = 0 error
 * @retval = 1 success
 */
BYTE MPXDAC_setVolume(WORD wVol)
{
	register AIU *aiu;
	aiu = (AIU *) (AIU_BASE);
	WORD iVol;
	MP_DEBUG("MPXDAC(DAC_INTERNAL) volume: %d",wVol);
	iVol =wVol;
	if (iVol >= 15)
		iVol = 15;
	if (wVol == 1)
		iVol = 1;
	//aiu->AiuVolCtl=(DWORD)(VolumeSequence[g_bVolumeIndex]<<16) + (DWORD)(VolumeSequence[(wVol & 0x3f)]<<8)+ (DWORD)VolumeSequence[(wVol & 0x3f)]; //headphone
	aiu->AiuGainCtl=iVol;//(DWORD)((g_bVolumeIndex == 0x10) ? (0xf) : g_bVolumeIndex);//line buffer volume
	return 1;
}

void AIUCfg_M6xxDAC(void)
{
	//g_psClock->ClkCtrl |= 0x01000000;	// enable PLL3
	//g_psClock->ClkCtrl &= 0xfdffff7f;	// from XIN
	//g_psClock->ClkCtrl &= 0xfdffffff;	// from XIN		//Pin 7 can not be off in MP650, more checking is needed

#if 0	// Temporarily mark by C.W
	if(g_psBiu->BiuChid == (CHIP_VER_620 | CHIP_VER_B))
		g_psClock->PLL3Cfg = (PLL3_CONFIG_FS_22K_DIV_1 | 0x00080000);//by samplerate
	else	
		g_psClock->PLL3Cfg = PLL3_CONFIG_FS_22K_DIV_1;//by samplerate
#endif
	g_psClock->PLL3Cfg = 0x0077D74A;
	g_psClock->AudClkC = 0x00001120;

#if !AUDIO_DAC_USING_PLL2	
	g_psClock->Clkss_EXT1 &= 0xfffffff0;
	g_psClock->Clkss_EXT1 |= (BIT3);	// Using PLL3/1 to be clock source
	//g_psClock->Clkss_EXT1 |= 0x00000008;	  //for PLL3 and enable
	g_psClock->ClkCtrl &= ~0x01000000;	 // Disable PLL3
	g_psClock->ClkCtrl |= 0x01000000;	 // enable PLL3
#else
	g_psClock->Clkss_EXT1 &= 0xfffffff0; //for using PLL2, eddy 2011.11.02
#endif

	AIUCLOCK_DISABLE();
	AIU_MAINCLOCK_ENABLE();

#if ((CHIP_VER & 0xFFFF0000) == CHIP_VER_615)
	g_psClock->MdClken |= 0x00000001;	
#else	
	g_psClock->Clkss_EXT2 |= 0x00040000;	//Using since mp650 series
#endif
	
	
	//Gpio_Config2GpioFunc(GPIO_GPIO_7, GPIO_OUTPUT_MODE,0, 1);
	//Gpio_Config2GpioFunc(GPIO_GPIO_6, GPIO_INPUT_MODE,1, 1);//If it has mute circuit, open it.

	// Dissable AIU_DMA
	g_psDmaAiu->Control = 0x00000000;
	

	g_psAiu->AiuCtl    |= 0x18000000;	//RAM_RESET
	//g_psAiu->AiuTopanaCtl |= 0x00008000;
	TimerDelay(1000);

	g_psAiu->AiuCtl |= 0x20000000;

	g_psAiu->AiuTopanaCtl =  0x00200000;
	g_psAiu->AiuDacCtl = 0x00000013;
	
	while(!(g_psAiu->AiuTopanaCtl & 0x00400000));

	g_psAiu->AiuTopanaCtl &=  ~0x00200000;
	g_psAiu->AiuTopanaCtl |=   0x00008200;
}


void  MPXDAC_Set_OutputSource(void)
{
#if LINE_BUFFER
	{
		g_psAiu->AiuTopanaCtl = 0x00008200;//line buffer enable
		g_psAiu->AiuGainCtl   = (DWORD)((g_bVolumeIndex==0x10)?(0xf) :g_bVolumeIndex);//line buffer volume
	}   
#else
		g_psAiu->AiuTopanaCtl = 0x00001001;//speaker enable
#endif
	g_psAiu->AiuTopanaCtl = 0x00008200;//line buffer enable
	g_psAiu->AiuGainCtl   = 0;//g_bVolumeIndex;//0x00000001;
}

inline void AudioInit_M6xxDAC(void)
{
	register INTERRUPT *interrupt;

	AIUCfg_M6xxDAC();
	interrupt = (INTERRUPT *)(INT_BASE);
	interrupt->MiMask |= 0x00000800;    //enable AUDIO module interrupt
	MP_DEBUG("MPXDAC(DAC_INTERNAL) init...");
}
int AudioRecInit_MPX()
{
	MP_ALERT("Not Support Record function");
}
HAL_AUDIODAC_T _audioDAC_Internal =
{
	AudioInit_M6xxDAC,
	AudioRecInit_MPX,	
	MPXDAC_uninit,
	MPXDAC_setSamplerate,
	NULL,
	MPXDAC_setVolume
};
#endif

