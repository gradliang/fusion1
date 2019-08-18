/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      syntax.c
*
* Programmer:    Brenda Li
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description:   Advanced Audio (AAC) Decoder
*        	 Reads the AAC bitstream as defined in 14496-3 (MPEG-4 Audio)
* Change History (most recent first):
*     <1>     03/30/2005    Brenda Li   first file
****************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"

#include "common.h"
#include "structs.h"

#include <stdlib.h>
#include <string.h>

#include "decoder.h"
#include "syntax.h"
#include "huffman.h"
#include "bits.h"
#include "pulse.h"

#include "Debug.h"
#include "PcmOutput.h"
#include "codec.h"

#define DEBUGDEC
#define DEBUGVAR(A,B,C)

extern BYTE video_flag;//1120 Rebecca
//function writer:Brenda 
//time:2005-3-9
/* Table 4.4.1 */
///////////////////////////////////Move from Specrec.c 2005.05.18////////////////////

static uint8_t num_swb_1024_window[] = {
	41, 41, 47, 49, 49, 51, 47, 47, 43, 43, 43, 40
};

static uint8_t num_swb_128_window[] = {
	12, 12, 12, 14, 14, 14, 15, 15, 15, 15, 15, 15
};

static uint16_t swb_offset_1024_96[] = {
	0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56,
	64, 72, 80, 88, 96, 108, 120, 132, 144, 156, 172, 188, 212, 240,
	276, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024
};

static uint16_t swb_offset_128_96[] = {
	0, 4, 8, 12, 16, 20, 24, 32, 40, 48, 64, 92, 128
};

static uint16_t swb_offset_1024_64[] = {
	0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56,
	64, 72, 80, 88, 100, 112, 124, 140, 156, 172, 192, 216, 240, 268,
	304, 344, 384, 424, 464, 504, 544, 584, 624, 664, 704, 744, 784, 824,
	864, 904, 944, 984, 1024
};

static uint16_t swb_offset_128_64[] = {
	0, 4, 8, 12, 16, 20, 24, 32, 40, 48, 64, 92, 128
};


static uint16_t swb_offset_1024_48[] = {
	0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 48, 56, 64, 72,
	80, 88, 96, 108, 120, 132, 144, 160, 176, 196, 216, 240, 264, 292,
	320, 352, 384, 416, 448, 480, 512, 544, 576, 608, 640, 672, 704, 736,
	768, 800, 832, 864, 896, 928, 1024
};

static uint16_t swb_offset_128_48[] = {
	0, 4, 8, 12, 16, 20, 28, 36, 44, 56, 68, 80, 96, 112, 128
};

static uint16_t swb_offset_1024_32[] = {
	0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 48, 56, 64, 72,
	80, 88, 96, 108, 120, 132, 144, 160, 176, 196, 216, 240, 264, 292,
	320, 352, 384, 416, 448, 480, 512, 544, 576, 608, 640, 672, 704, 736,
	768, 800, 832, 864, 896, 928, 960, 992, 1024
};

static uint16_t swb_offset_1024_24[] = {
	0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 52, 60, 68,
	76, 84, 92, 100, 108, 116, 124, 136, 148, 160, 172, 188, 204, 220,
	240, 260, 284, 308, 336, 364, 396, 432, 468, 508, 552, 600, 652, 704,
	768, 832, 896, 960, 1024
};

static uint16_t swb_offset_128_24[] = {
	0, 4, 8, 12, 16, 20, 24, 28, 36, 44, 52, 64, 76, 92, 108, 128
};

static uint16_t swb_offset_1024_16[] = {
	0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 100, 112, 124,
	136, 148, 160, 172, 184, 196, 212, 228, 244, 260, 280, 300, 320, 344,
	368, 396, 424, 456, 492, 532, 572, 616, 664, 716, 772, 832, 896, 960, 1024
};

static uint16_t swb_offset_128_16[] = {
	0, 4, 8, 12, 16, 20, 24, 28, 32, 40, 48, 60, 72, 88, 108, 128
};

static uint16_t swb_offset_1024_8[] = {
	0, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 172,
	188, 204, 220, 236, 252, 268, 288, 308, 328, 348, 372, 396, 420, 448,
	476, 508, 544, 580, 620, 664, 712, 764, 820, 880, 944, 1024
};

static uint16_t swb_offset_128_8[] = {
	0, 4, 8, 12, 16, 20, 24, 28, 36, 44, 52, 60, 72, 88, 108, 128
};

static uint16_t *swb_offset_1024_window[] = {
	swb_offset_1024_96,			/* 96000 */
	swb_offset_1024_96,			/* 88200 */
	swb_offset_1024_64,			/* 64000 */
	swb_offset_1024_48,			/* 48000 */
	swb_offset_1024_48,			/* 44100 */
	swb_offset_1024_32,			/* 32000 */
	swb_offset_1024_24,			/* 24000 */
	swb_offset_1024_24,			/* 22050 */
	swb_offset_1024_16,			/* 16000 */
	swb_offset_1024_16,			/* 12000 */
	swb_offset_1024_16,			/* 11025 */
	swb_offset_1024_8			/* 8000  */
};

static uint16_t *swb_offset_128_window[] = {
	swb_offset_128_96,			/* 96000 */
	swb_offset_128_96,			/* 88200 */
	swb_offset_128_64,			/* 64000 */
	swb_offset_128_48,			/* 48000 */
	swb_offset_128_48,			/* 44100 */
	swb_offset_128_48,			/* 32000 */
	swb_offset_128_24,			/* 24000 */
	swb_offset_128_24,			/* 22050 */
	swb_offset_128_16,			/* 16000 */
	swb_offset_128_16,			/* 12000 */
	swb_offset_128_16,			/* 11025 */
	swb_offset_128_8			/* 8000  */
};

#define bit_set(A, B) ((A) & (1<<(B)))

DWORD MPA_WAIT_TIME = 0;
void aac_fill_regs(faacDecHandle hDecoder,
				   ic_stream * ics1,
				   ic_stream * ics2,
				   int16_t * spec_data1, int16_t * spec_data2, int8_t pair_channel)
{
	MPA *mpa;
	int w, i, j, f;
	int x, n = 0;
	CHANNEL *rdma;
	DWORD tmp = 0;

	mpa = (MPA *) MPA_BASE;
//1+++++++++++left channel+++++++++++++
	tmp = (ics1->window_sequence << 6) + (ics1->window_shape << 3);
	+(hDecoder->window_shape_prev[0] << 2);
	mpa->BlkType = (mpa->BlkType & 0x33) | (tmp & 0xcc);

	mpa->NumSwbL = ics1->num_swb;
	mpa->MsMaskPresentL = ics1->ms_mask_present;
	mpa->NumWindowGroupsL = ics1->num_window_groups;
	mpa->MaxSfbL = ics1->max_sfb;
	tmp = 0;
	tmp = ics1->window_group_length[0] << 21;
	tmp += ics1->window_group_length[1] << 18;
	tmp += ics1->window_group_length[2] << 15;
	tmp += ics1->window_group_length[3] << 12;
	tmp += ics1->window_group_length[4] << 9;
	tmp += ics1->window_group_length[5] << 6;
	tmp += ics1->window_group_length[6] << 3;
	tmp += ics1->window_group_length[7];
//??Spec didn't give the order of win sequence
	mpa->WinGrpLenL = tmp;
	mpa->PnsUsedL = ics1->noise_used;
	
	if (!video_flag)//when play vedio ,close TNS to avoid lacks of bandwidth
	mpa->TnsUsedL = ics1->tns_data_present;
	else
	mpa->TnsUsedL = 0;//??ics1->tns_data_present;
	
	if (ics1->tns_data_present)
	{
		for (w = 0; w < 8; w++)
		{
			x <<= 3;
			x |= ((ics1->tns.n_filt[w] & 0x3) << 1) | (ics1->tns.coef_res[w] & 0x1);
			//x |= ((ics1->tns.n_filt[w] & 0x3) << 1) | (ics1->tns.n_filt[w] & 0x1);
		}
		mpa->TnsL = x;
		x = 0;
		i = 0;
		for (w = 0; w < ics1->num_windows; w++)
		{
			for (f = 0; f < ics1->tns.n_filt[w]; f++)
			{
				x = ((ics1->tns.coef_compress[w][f] & 0x1) << 25)
					| ((ics1->tns.direction[w][f] & 0x1) << 24)
					| ((ics1->tns.order[w][f] & 0xf) << 20)
					| ((ics1->tns.size[w][f] & 0x3ff) << 10) | (ics1->tns.start[w][f] & 0x3ff);
				mpa->TnsL_N[i][0] = x;
				i++;
				x = 0;
			}
		}

		n = ics1->tns.n_filt[0] + ics1->tns.n_filt[1]
			+ ics1->tns.n_filt[2] + ics1->tns.n_filt[3]
			+ ics1->tns.n_filt[4] + ics1->tns.n_filt[5] + ics1->tns.n_filt[6] + ics1->tns.n_filt[7];
		if (n < 8)
			for (i = 0; i < 8 - n; i++)
				mpa->TnsL_N[i][0] = 0;	//keep the left wins' TnsL_N all '0'
		j = 0;
		for (w = 0; w < ics1->num_windows; w++)
		{
			for (f = 0; f < ics1->tns.n_filt[w]; f++)
			{
				for (i = 0; i < 6; i++)
					x |= (ics1->tns.coef[w][f][i] & 0xf) << ((i % 6) * 4);
				mpa->TnsL_N[j][1] = x;
				x = 0;
				for (i = 6; i < 12; i++)
					x |= (ics1->tns.coef[w][f][i] & 0xf) << ((i % 6) * 4);
				mpa->TnsL_N[j][2] = x;
				x = 0;
				j++;
			}
		}
		if (n < 8)
		{						//keep the left wins' TnsL_N all '0'
			for (i = 0; i < 8 - n; i++)
			{
				mpa->TnsL_N[j][1] = x;
				mpa->TnsL_N[j][2] = x;
				j++;
			}
		}
	}
	if (pair_channel)
		mpa->Mono = 0;
	else
		mpa->Mono = 1;

	mpa->Voffset = 0;
	mpa->Bypass = 0;
	mpa->Op_Mode = 4;

	mpa->MbRst = 1;
	for (i = 0; i < 120; i++)
		mpa->ScalefacMb = ics1->scale_factors[i];

	//MISC - ms_used
	mpa->MbRst = 1;
	for (i = 0; i < 8; i++)
	{
		x = ics1->ms_used[i * 16 + 0] << 0 | ics1->ms_used[i * 16 + 1] << 1
			| ics1->ms_used[i * 16 + 2] << 2 | ics1->ms_used[i * 16 + 3] << 3
			| ics1->ms_used[i * 16 + 4] << 4 | ics1->ms_used[i * 16 + 5] << 5
			| ics1->ms_used[i * 16 + 6] << 6 | ics1->ms_used[i * 16 + 7] << 7
			| ics1->ms_used[i * 16 + 8] << 8 | ics1->ms_used[i * 16 + 9] << 9
			| ics1->ms_used[i * 16 + 10] << 10 | ics1->ms_used[i * 16 + 11] << 11
			| ics1->ms_used[i * 16 + 12] << 12 | ics1->ms_used[i * 16 + 13] << 13
			| ics1->ms_used[i * 16 + 14] << 14 | ics1->ms_used[i * 16 + 15] << 15;
		mpa->SideinfoMb = x;
		x = 0;
	}
	//MISC - swb_offset
	for (i = 0; i < 52; i++)
		mpa->SideinfoMb = ics1->swb_offset[i];
	//MISC - sect_sfb_offset
	mpa->MbRst = 1;
	for (i = 0; i < 120; i++)
		mpa->SectsfbMb = ics1->sect_sfb_offset[i];
	//sfb_cb
	mpa->MbRst = 1;
	if (pair_channel == 1)
		for (i = 0; i < 120; i++)
			mpa->SfbcbMb = ((ics2->sfb_cb[i] << 4) | ics1->sfb_cb[i]);
	else
		for (i = 0; i < 120; i++)
			mpa->SfbcbMb = ics1->sfb_cb[i];

	rdma = (CHANNEL *) DMA_MPAR_BASE;
	rdma->Control = 0;
	//Gregxu 2005.5.20
	rdma->StartA = (DWORD) spec_data1;
	rdma->EndA = (DWORD) ((BYTE *) spec_data1 + 2048 - 1);
	mpa->MpaCtrl = 0x11;		//start decoding and busy goes to 1
	rdma->Control = 1;


//1+++++++++++right channel+++++++++++++
	if (pair_channel == 1)
	{
		////////////////////////
		DWORD Dtick, Dtpc, Dtmv;

		get_cur_timeL(&Dtick, &Dtpc, &Dtmv);
		while (1)
		{
			if (mpa->MpaStatus == MPA_SEQ_CH1_FILL)
				break;
			TaskYield();
		}
		MPA_WAIT_TIME += get_elapsed_timeL(Dtick, Dtpc, Dtmv);
		/////////////////////////

		//aac_mode: 0-long window, 1-short window
		tmp = 0;
		tmp = (ics2->window_sequence * 16) + (ics2->window_shape * 2);
		+(hDecoder->window_shape_prev[1]);
		mpa->BlkType = (mpa->BlkType & 0xcc) | (tmp & 0x33);
		mpa->NumSwbR = ics2->num_swb;
		mpa->MsMaskPresentR = ics2->ms_mask_present;
		mpa->NumWindowGroupsR = ics2->num_window_groups;
		mpa->MaxSfbR = ics2->max_sfb;
		tmp = 0;
		tmp = ics2->window_group_length[0] << 21;
		tmp += ics2->window_group_length[1] << 18;
		tmp += ics2->window_group_length[2] << 15;
		tmp += ics2->window_group_length[3] << 12;
		tmp += ics2->window_group_length[4] << 9;
		tmp += ics2->window_group_length[5] << 6;
		tmp += ics2->window_group_length[6] << 3;
		tmp += ics2->window_group_length[7];
		mpa->WinGrpLenR = tmp;
		mpa->PnsUsedR = ics2->noise_used;
		
		if (!video_flag)//when play vedio ,close TNS to avoid lacks of bandwidth
		mpa->TnsUsedR = ics2->tns_data_present;
		else
        mpa->TnsUsedR =0;//?? ics2->tns_data_present;

		if (ics2->tns_data_present)
		{
			x = 0;
			for (w = 0; w < 8; w++)
			{
				x <<= 3;
				x |= ((ics2->tns.n_filt[w] & 0x3) << 1) | (ics2->tns.coef_res[w] & 0x1);
				//x |= ((ics2->tns.n_filt[w] & 0x3) << 1) | (ics2->tns.n_filt[w] & 0x1);
			}
			mpa->TnsR = x;
			x = 0;
			i = 0;
			for (w = 0; w < ics2->num_windows; w++)
			{
				for (f = 0; f < ics2->tns.n_filt[w]; f++)
				{
					x = ((ics2->tns.coef_compress[w][f] & 0x1) << 25)
						| ((ics2->tns.direction[w][f] & 0x1) << 24)
						| ((ics2->tns.order[w][f] & 0xf) << 20)
						| ((ics2->tns.size[w][f] & 0x3ff) << 10) | (ics2->tns.start[w][f] & 0x3ff);
					mpa->TnsR_N[i][0] = x;
					i++;
					x = 0;
				}
			}
			n = ics2->tns.n_filt[0] + ics2->tns.n_filt[1]
				+ ics2->tns.n_filt[2] + ics2->tns.n_filt[3]
				+ ics2->tns.n_filt[4] + ics2->tns.n_filt[5]
				+ ics2->tns.n_filt[6] + ics2->tns.n_filt[7];

			if (n < 8)
				for (i = 0; i < 8 - n; i++)
					mpa->TnsR_N[i][0] = 0;

			j = 0;
			for (w = 0; w < ics2->num_windows; w++)
			{
				for (f = 0; f < ics2->tns.n_filt[w]; f++)
				{
					for (i = 0; i < 6; i++)
					{
						x |= (ics2->tns.coef[w][f][i] & 0xf) << ((i % 6) * 4);
					}
					mpa->TnsR_N[j][1] = x;
					x = 0;
					for (i = 6; i < 12; i++)
					{
						x |= (ics2->tns.coef[w][f][i] & 0xf) << ((i % 6) * 4);
					}
					mpa->TnsR_N[j][2] = x;
					x = 0;
					j++;
				}
			}
			if (n < 8)
			{
				for (i = 0; i < 8 - n; i++)
				{
					mpa->TnsR_N[j][1] = x;
					mpa->TnsR_N[j][2] = x;
					j++;
				}
			}
		}

		//scale_factor
		mpa->MbRst = 0x100;
		//mpa->MbRst = 0x1;
		for (i = 0; i < 120; i++)
			mpa->ScalefacMb = ics2->scale_factors[i];
		//MISC - swb_offset
		mpa->MbRst = 0x10;
		//mpa->MbRst = 0x1;
		for (i = 0; i < 52; i++)
			mpa->SideinfoMb = ics2->swb_offset[i];
		//MISC - sect_sfb_offset
		mpa->MbRst = 0x100;
		//mpa->MbRst = 0x1;
		for (i = 0; i < 120; i++)
			mpa->SectsfbMb = ics2->sect_sfb_offset[i];
		rdma = (CHANNEL *) DMA_MPAR_BASE;
		rdma->Control = 0;
		//Gregxu 2005.5.20
		rdma->StartA = (DWORD) spec_data2;
		rdma->EndA = (DWORD) ((BYTE *) spec_data2 + 2048 - 1);
		mpa->MpaCtrl = 0x11;	//start decoding and busy goes to 1
		rdma->Control = 1;
	}
}

/* 4.5.2.3.4 */
/*
  - determine the number of windows in a window_sequence named num_windows
  - determine the number of window_groups named num_window_groups
  - determine the number of windows in each group named window_group_length[g]
  - determine the total number of scalefactor window bands named num_swb for
    the actual window type
  - determine swb_offset[swb], the offset of the first coefficient in
    scalefactor window band named swb of the window actually used
  - determine sect_sfb_offset[g][section],the offset of the first coefficient
    in section named section. This offset depends on window_sequence and
    scale_factor_grouping and is needed to decode the spectral_data().
*/
uint8_t window_grouping_info_faad(faacDecHandle hDecoder, ic_stream * ics)
{
	uint8_t i, g;

	uint8_t sf_index = hDecoder->sf_index;

	switch (ics->window_sequence)
	{
	case ONLY_LONG_SEQUENCE:
	case LONG_START_SEQUENCE:
	case LONG_STOP_SEQUENCE:
		ics->num_windows = 1;
		ics->num_window_groups = 1;
		ics->window_group_length[ics->num_window_groups - 1] = 1;
		ics->num_swb = num_swb_1024_window[sf_index];

		/* preparation of sect_sfb_offset for long blocks */
		/* also copy the last value! */
		for (i = 0; i < ics->num_swb; i++)
		{
			ics->sect_sfb_offset[i] = swb_offset_1024_window[sf_index][i];
			ics->swb_offset[i] = swb_offset_1024_window[sf_index][i];
		}
		ics->sect_sfb_offset[ics->num_swb] = 1024;
		ics->swb_offset[ics->num_swb] = 1024;
		return 0;
	case EIGHT_SHORT_SEQUENCE:
		ics->num_windows = 8;
		ics->num_window_groups = 1;
		ics->window_group_length[ics->num_window_groups - 1] = 1;
		ics->num_swb = num_swb_128_window[sf_index];

		for (i = 0; i < ics->num_swb; i++)
			ics->swb_offset[i] = swb_offset_128_window[sf_index][i];
		ics->swb_offset[ics->num_swb] = 128;

		for (i = 0; i < ics->num_windows - 1; i++)
		{
			if (bit_set(ics->scale_factor_grouping, 6 - i) == 0)
			{
				ics->num_window_groups += 1;
				ics->window_group_length[ics->num_window_groups - 1] = 1;
			}
			else
			{
				ics->window_group_length[ics->num_window_groups - 1] += 1;
			}
		}

		/* preparation of sect_sfb_offset for short blocks */
		for (g = 0; g < ics->num_window_groups; g++)
		{
			uint16_t width;
			uint8_t sect_sfb = 0;
			uint16_t offset = 0;

			for (i = 0; i < ics->num_swb; i++)
			{
				if (i + 1 == ics->num_swb)
				{
					width = 128 - swb_offset_128_window[sf_index][i];
				}
				else
				{
					width = swb_offset_128_window[sf_index][i + 1] -
						swb_offset_128_window[sf_index][i];
				}
				width *= ics->window_group_length[g];
				ics->sect_sfb_offset[g * 15 + sect_sfb++] = offset;
				offset += width;
			}
			ics->sect_sfb_offset[g * 15 + sect_sfb] = offset;
		}
		return 0;
	default:
		return 1;
	}
}

///////////////////////////////////Move from Specrec.c 2005.05.18////////////////////
int8_t GASpecificConfig_faad(bitfile * ld, mp4AudioSpecificConfig * mp4ASC, program_config * pce_out)
{
	program_config pce;

	/* 1024 or 960 */
	mp4ASC->frameLengthFlag = faad_get1bit(ld
										   DEBUGVAR(1, 138, "GASpecificConfig_faad(): FrameLengthFlag"));
	mp4ASC->dependsOnCoreCoder = faad_get1bit(ld
											  DEBUGVAR(1, 139,
													   "GASpecificConfig_faad(): DependsOnCoreCoder"));
	if (mp4ASC->dependsOnCoreCoder == 1)
	{
		faad_getbits(ld, 14 DEBUGVAR(1, 140, "GASpecificConfig_faad(): CoreCoderDelay"));
	}

	faad_get1bit(ld DEBUGVAR(1, 141, "GASpecificConfig_faad(): ExtensionFlag"));
	if (mp4ASC->channelsConfiguration == 0)
	{
		program_config_element(&pce, ld);
		//mp4ASC->channelsConfiguration = pce.channels;

		if (pce_out != NULL)
			memcpy(pce_out, &pce, sizeof(program_config));

		/*
		   if (pce.num_valid_cc_elements)
		   return -3;
		 */
	}
	return 0;
}

/* Table 4.4.2 */
/* An MPEG-4 Audio decoder is only required to follow the Program
   Configuration Element in GASpecificConfig_faad(). The decoder shall ignore
   any Program Configuration Elements that may occur in raw data blocks.
   PCEs transmitted in raw data blocks cannot be used to convey decoder
   configuration information.
*/
uint8_t program_config_element(program_config * pce, bitfile * ld)
{
	uint8_t i;

	memset(pce, 0, sizeof(program_config));

	pce->channels = 0;

	pce->element_instance_tag = (uint8_t) faad_getbits(ld, 4
													   DEBUGVAR(1, 10,
																"program_config_element(): element_instance_tag"));

	pce->object_type = (uint8_t) faad_getbits(ld, 2
											  DEBUGVAR(1, 11,
													   "program_config_element(): object_type"));
	pce->sf_index =
		(uint8_t) faad_getbits(ld, 4 DEBUGVAR(1, 12, "program_config_element(): sf_index"));
	pce->num_front_channel_elements =
		(uint8_t) faad_getbits(ld,
							   4 DEBUGVAR(1, 13,
										  "program_config_element(): num_front_channel_elements"));
	pce->num_side_channel_elements =
		(uint8_t) faad_getbits(ld,
							   4 DEBUGVAR(1, 14,
										  "program_config_element(): num_side_channel_elements"));
	pce->num_back_channel_elements =
		(uint8_t) faad_getbits(ld,
							   4 DEBUGVAR(1, 15,
										  "program_config_element(): num_back_channel_elements"));
	pce->num_lfe_channel_elements =
		(uint8_t) faad_getbits(ld,
							   2 DEBUGVAR(1, 16,
										  "program_config_element(): num_lfe_channel_elements"));
	pce->num_assoc_data_elements =
		(uint8_t) faad_getbits(ld,
							   3 DEBUGVAR(1, 17,
										  "program_config_element(): num_assoc_data_elements"));
	pce->num_valid_cc_elements =
		(uint8_t) faad_getbits(ld,
							   4 DEBUGVAR(1, 18,
										  "program_config_element(): num_valid_cc_elements"));

	pce->mono_mixdown_present = faad_get1bit(ld
											 DEBUGVAR(1, 19,
													  "program_config_element(): mono_mixdown_present"));
	if (pce->mono_mixdown_present == 1)
	{
		pce->mono_mixdown_element_number = (uint8_t) faad_getbits(ld, 4
																  DEBUGVAR(1, 20,
																		   "program_config_element(): mono_mixdown_element_number"));
	}

	pce->stereo_mixdown_present = faad_get1bit(ld
											   DEBUGVAR(1, 21,
														"program_config_element(): stereo_mixdown_present"));
	if (pce->stereo_mixdown_present == 1)
	{
		pce->stereo_mixdown_element_number = (uint8_t) faad_getbits(ld, 4
																	DEBUGVAR(1, 22,
																			 "program_config_element(): stereo_mixdown_element_number"));
	}

	pce->matrix_mixdown_idx_present = faad_get1bit(ld
												   DEBUGVAR(1, 23,
															"program_config_element(): matrix_mixdown_idx_present"));
	if (pce->matrix_mixdown_idx_present == 1)
	{
		pce->matrix_mixdown_idx = (uint8_t) faad_getbits(ld, 2
														 DEBUGVAR(1, 24,
																  "program_config_element(): matrix_mixdown_idx"));
		pce->pseudo_surround_enable =
			faad_get1bit(ld DEBUGVAR(1, 25, "program_config_element(): pseudo_surround_enable"));
	}

	for (i = 0; i < pce->num_front_channel_elements; i++)
	{
		pce->front_element_is_cpe[i] = faad_get1bit(ld
													DEBUGVAR(1, 26,
															 "program_config_element(): front_element_is_cpe"));
		pce->front_element_tag_select[i] =
			(uint8_t) faad_getbits(ld,
								   4 DEBUGVAR(1, 27,
											  "program_config_element(): front_element_tag_select"));

		if (pce->front_element_is_cpe[i] & 1)
		{
			pce->cpe_channel[pce->front_element_tag_select[i]] = pce->channels;
			pce->num_front_channels += 2;
			pce->channels += 2;
		}
		else
		{
			pce->sce_channel[pce->front_element_tag_select[i]] = pce->channels;
			pce->num_front_channels++;
			pce->channels++;
		}
	}

	for (i = 0; i < pce->num_side_channel_elements; i++)
	{
		pce->side_element_is_cpe[i] = faad_get1bit(ld
												   DEBUGVAR(1, 28,
															"program_config_element(): side_element_is_cpe"));
		pce->side_element_tag_select[i] =
			(uint8_t) faad_getbits(ld,
								   4 DEBUGVAR(1, 29,
											  "program_config_element(): side_element_tag_select"));

		if (pce->side_element_is_cpe[i] & 1)
		{
			pce->cpe_channel[pce->side_element_tag_select[i]] = pce->channels;
			pce->num_side_channels += 2;
			pce->channels += 2;
		}
		else
		{
			pce->sce_channel[pce->side_element_tag_select[i]] = pce->channels;
			pce->num_side_channels++;
			pce->channels++;
		}
	}

	for (i = 0; i < pce->num_back_channel_elements; i++)
	{
		pce->back_element_is_cpe[i] = faad_get1bit(ld
												   DEBUGVAR(1, 30,
															"program_config_element(): back_element_is_cpe"));
		pce->back_element_tag_select[i] =
			(uint8_t) faad_getbits(ld,
								   4 DEBUGVAR(1, 31,
											  "program_config_element(): back_element_tag_select"));

		if (pce->back_element_is_cpe[i] & 1)
		{
			pce->cpe_channel[pce->back_element_tag_select[i]] = pce->channels;
			pce->channels += 2;
			pce->num_back_channels += 2;
		}
		else
		{
			pce->sce_channel[pce->back_element_tag_select[i]] = pce->channels;
			pce->num_back_channels++;
			pce->channels++;
		}
	}

	for (i = 0; i < pce->num_lfe_channel_elements; i++)
	{
		pce->lfe_element_tag_select[i] = (uint8_t) faad_getbits(ld, 4
																DEBUGVAR(1, 32,
																		 "program_config_element(): lfe_element_tag_select"));

		pce->sce_channel[pce->lfe_element_tag_select[i]] = pce->channels;
		pce->num_lfe_channels++;
		pce->channels++;
	}

	for (i = 0; i < pce->num_assoc_data_elements; i++)
		pce->assoc_data_element_tag_select[i] = (uint8_t) faad_getbits(ld, 4
																	   DEBUGVAR(1, 33,
																				"program_config_element(): assoc_data_element_tag_select"));

	for (i = 0; i < pce->num_valid_cc_elements; i++)
	{
		pce->cc_element_is_ind_sw[i] = faad_get1bit(ld
													DEBUGVAR(1, 34,
															 "program_config_element(): cc_element_is_ind_sw"));
		pce->valid_cc_element_tag_select[i] =
			(uint8_t) faad_getbits(ld,
								   4 DEBUGVAR(1, 35,
											  "program_config_element(): valid_cc_element_tag_select"));
	}

	hw_faad_byte_align(ld);

	pce->comment_field_bytes = (uint8_t) faad_getbits(ld, 8
													  DEBUGVAR(1, 36,
															   "program_config_element(): comment_field_bytes"));

	for (i = 0; i < pce->comment_field_bytes; i++)
	{
		pce->comment_field_data[i] = (uint8_t) faad_getbits(ld, 8
															DEBUGVAR(1, 37,
																	 "program_config_element(): comment_field_data"));
	}
	pce->comment_field_data[i] = 0;

	return 0;
}

void decode_sce_lfe_faad(faacDecHandle hDecoder,
					faacDecFrameInfo * hInfo, bitfile * ld, uint8_t id_syn_ele)
{
	uint8_t channels = hDecoder->fr_channels;

	if (channels + 1 > MAX_CHANNELS)
	{
		hInfo->error = 12;
		return;
	}
	if (hDecoder->fr_ch_ele + 1 > MAX_SYNTAX_ELEMENTS)
	{
		hInfo->error = 13;
		return;
	}

	hInfo->error = single_lfe_channel_element(hDecoder, ld, channels);

	hDecoder->fr_channels++;
	hDecoder->fr_ch_ele++;
}

void decode_cpe_faad(faacDecHandle hDecoder, faacDecFrameInfo * hInfo, bitfile * ld, uint8_t id_syn_ele)
{
	uint8_t channels = hDecoder->fr_channels;

	if (channels + 2 > MAX_CHANNELS)
	{
		hInfo->error = 12;
		return;
	}
	if (hDecoder->fr_ch_ele + 1 > MAX_SYNTAX_ELEMENTS)
	{
		hInfo->error = 13;
		return;
	}

	hInfo->error = channel_pair_element(hDecoder, ld, channels);

	hDecoder->fr_channels += 2;
	hDecoder->fr_ch_ele++;
}

void hw_raw_data_block(faacDecHandle hDecoder, faacDecFrameInfo * hInfo,
					bitfile * ld, program_config * pce)
{
	uint8_t id_syn_ele;

	hDecoder->fr_channels = 0;
	hDecoder->fr_ch_ele = 0;

	/* Table 4.4.3: raw_data_block() */
	while ((id_syn_ele = (uint8_t) faad_getbits(ld, LEN_SE_ID
												DEBUGVAR(1, 4,
														 "faacDecDecode(): id_syn_ele"))) != ID_END)
	{
		switch (id_syn_ele)
		{
		case ID_SCE:
			decode_sce_lfe_faad(hDecoder, hInfo, ld, id_syn_ele);
			if (hInfo->error > 0)
				return;
			break;
		case ID_CPE:
			decode_cpe_faad(hDecoder, hInfo, ld, id_syn_ele);
			if (hInfo->error > 0)
				return;
			break;
		case ID_LFE:
			decode_sce_lfe_faad(hDecoder, hInfo, ld, id_syn_ele);
			if (hInfo->error > 0)
				return;
			break;
		case ID_CCE:			/* not implemented yet, but skip the bits */
			hInfo->error = 6;
			if (hInfo->error > 0)
				return;
			break;
		case ID_DSE:
			data_stream_element(hDecoder, ld);
			break;
		case ID_PCE:
			if ((hInfo->error = program_config_element(pce, ld)) > 0)
				return;
			break;
		case ID_FIL:
			/* one sbr_info describes a channel_element not a channel! */
			if ((hInfo->error = fill_element(hDecoder, ld)) > 0)
				return;
			break;
		}
	}
	hw_faad_byte_align(ld);

	return;
}

extern int n_frames;
int AACDmaOut[2][512] = { 0 * 1024 };

static inline void Big2Little(int16_t * array)	//,int len), len = 1024
{
	int i, j;
	int16_t tmp;

	for (i = 0, j = 0; i < 512; i++, j += 2)
	{
		tmp = array[j];
		array[j] = array[j + 1];
		array[j + 1] = tmp;
	}
}

int Faad_Inited = 0;

extern BOOL boScalerBusy, boMPABusy;
/* Table 4.4.4 and */
/* Table 4.4.9 */
static uint8_t single_lfe_channel_element(faacDecHandle hDecoder, bitfile * ld, uint8_t channel)
{
	uint8_t retval = 0;
	element sce = { 0 };
	ic_stream *ics = &(sce.ics1);
	int16_t spec_data[1024] = { 0 };
	MPA *mpa;
	int *pAACDmaOut0;

	mpa = (MPA *) MPA_BASE;

	pAACDmaOut0 = (int *) (((long) (&AACDmaOut[0][0]) & 0x1fffffffL) | 0xa0000000L);
	if (Faad_Inited == 0)
	{
		CHANNEL *wdma;

		wdma = (CHANNEL *) DMA_MPAW_BASE;
		wdma->Control = 0;
		//Gregxu 2005.5.20  
		wdma->StartA = (DWORD) pAACDmaOut0;
		wdma->EndA = (DWORD) ((BYTE *) pAACDmaOut0 + 2048 - 1);
		wdma->Control = 0x4003;
	}
	/*sce.element_instance_tag(uint8_t) */ faad_getbits(ld, LEN_TAG
														DEBUGVAR(1, 38,
																 "single_lfe_channel_element(): element_instance_tag"));

	sce.channel = channel;
	sce.paired_channel = -1;

	{
		retval = individual_channel_stream(hDecoder, &sce, ld, ics, 0, spec_data);
		if (retval > 0)
			return retval;
	}

	/* noiseless coding is done, spectral reconstruction is done now */
	//Big2Little(spec_data,1024);
	//Big2Little(spec_data);
	if (hDecoder->sample_buffer != NULL)
	{
		MPA *mpa;
		DWORD Dtick, Dtpc, Dtmv;

		mpa = (MPA *) MPA_BASE;

		get_cur_timeL(&Dtick, &Dtpc, &Dtmv);
		while (mpa->MpaStatus != MPA_SEQ_IDLE)    TaskYield();
		MPA_WAIT_TIME += get_elapsed_timeL(Dtick, Dtpc, Dtmv);

#if 0
		PcmOutput(1, 1024, pAACDmaOut0, NULL, hDecoder->sample_buffer);
#else
		if (n_frames < 10)
		{
			memset(hDecoder->sample_buffer, 0, 1 * 1024 * 2);
		}
		else
		{
			PcmOutput(1, 1024, pAACDmaOut0, NULL, hDecoder->sample_buffer);
		}
#endif
	}

	while ((boScalerBusy == TRUE))    TaskYield();
	boMPABusy = TRUE;
	aac_fill_regs(hDecoder, ics, NULL, spec_data, NULL, 0);
	hDecoder->window_shape_prev[0] = ics->window_shape;
	Faad_Inited = 1;
	n_frames++;
	//while(mpa->MpaStatus != MPA_SEQ_IDLE);
	boMPABusy = FALSE;
	return 0;
}
static uint8_t channel_pair_element(faacDecHandle hDecoder, bitfile * ld, uint8_t channels)
{
	int16_t spec_data1[1024] = { 0 };
	int16_t spec_data2[1024] = { 0 };
	element cpe = { 0 };
	ic_stream *ics1 = &(cpe.ics1);
	ic_stream *ics2 = &(cpe.ics2);
	uint8_t result;
	int *pAACDmaOut0, *pAACDmaOut1;

	pAACDmaOut0 = (int *) (((long) (&AACDmaOut[0][0]) & 0x1fffffffL) | 0xa0000000L);
	pAACDmaOut1 = (int *) (((long) (&AACDmaOut[1][0]) & 0x1fffffffL) | 0xa0000000L);
	if (Faad_Inited == 0)
	{
		CHANNEL *wdma;

		wdma = (CHANNEL *) DMA_MPAW_BASE;
		wdma->Control = 0;
		//Gregxu 2005.5.20  
		wdma->StartA = (DWORD) pAACDmaOut0;
		wdma->EndA = (DWORD) ((BYTE *) pAACDmaOut0 + 2048 - 1);
		wdma->StartB = (DWORD) pAACDmaOut1;
		wdma->EndB = (DWORD) ((BYTE *) pAACDmaOut1 + 2048 - 1);
		wdma->Control = 0x400b;
	}
	cpe.channel = channels;
	cpe.paired_channel = channels + 1;

	/*cpe.element_instance_tag = (uint8_t) */ faad_getbits(ld, LEN_TAG
														   DEBUGVAR(1, 39,
																	"channel_pair_element(): element_instance_tag"));

	if ((cpe.common_window = faad_get1bit(ld
										  DEBUGVAR(1, 40,
												   "channel_pair_element(): common_window"))) & 1)
	{
		/* both channels have common ics information */
		if ((result = ics_info(hDecoder, ics1, ld, cpe.common_window)) > 0)
			return result;

		ics1->ms_mask_present = (uint8_t) faad_getbits(ld, 2
													   DEBUGVAR(1, 41,
																"channel_pair_element(): ms_mask_present"));
		if (ics1->ms_mask_present == 1)
		{
			uint8_t g, sfb;

			for (g = 0; g < ics1->num_window_groups; g++)
			{
				for (sfb = 0; sfb < ics1->max_sfb; sfb++)
				{
					ics1->ms_used[g * 15 + sfb] = faad_get1bit(ld
															   DEBUGVAR(1, 42,
																		"channel_pair_element(): faad_get1bit"));
				}
			}
		}

		memcpy(ics2, ics1, sizeof(ic_stream));
	}
	else
	{
		ics1->ms_mask_present = 0;
	}

	if ((result = individual_channel_stream(hDecoder, &cpe, ld, ics1, 0, spec_data1)) > 0)
	{
		return result;
	}

	if ((result = individual_channel_stream(hDecoder, &cpe, ld, ics2, 0, spec_data2)) > 0)
	{
		return result;
	}

	/* noiseless coding is done, spectral reconstruction is done now */
	//Big2Little(spec_data1);//,1024);
	//Big2Little(spec_data2);//,1024);

	if (hDecoder->sample_buffer != NULL)
	{
		MPA *mpa;
		DWORD Dtick, Dtpc, Dtmv;

		mpa = (MPA *) MPA_BASE;
		get_cur_timeL(&Dtick, &Dtpc, &Dtmv);
		while (mpa->MpaStatus != MPA_SEQ_IDLE)    TaskYield();
		MPA_WAIT_TIME += get_elapsed_timeL(Dtick, Dtpc, Dtmv);

#if 0
		PcmOutput(2, 1024, pAACDmaOut0, pAACDmaOut1, hDecoder->sample_buffer);
#else
		if (n_frames < 10)
		{
			memset(hDecoder->sample_buffer, 0, 2 * 1024 * 2);
		}
		else
		{
			PcmOutput(2, 1024, pAACDmaOut0, pAACDmaOut1, hDecoder->sample_buffer);
		}

#endif
	}

	while ((boScalerBusy == TRUE))    TaskYield();
	boMPABusy = TRUE;
	aac_fill_regs(hDecoder, ics1, ics2, spec_data1, spec_data2, 1);
	hDecoder->window_shape_prev[0] = ics1->window_shape;
	hDecoder->window_shape_prev[1] = ics2->window_shape;

	Faad_Inited = 1;
	n_frames++;
	boMPABusy = FALSE;
	return 0;
}

/* Table 4.4.6 */
static uint8_t ics_info(faacDecHandle hDecoder, ic_stream * ics, bitfile * ld,
						uint8_t common_window)
{
	uint8_t retval = 0;

	/* ics->ics_reserved_bit = */ faad_get1bit(ld
											   DEBUGVAR(1, 43, "ics_info(): ics_reserved_bit"));
	ics->window_sequence = (uint8_t) faad_getbits(ld, 2
												  DEBUGVAR(1, 44, "ics_info(): window_sequence"));
	ics->window_shape = faad_get1bit(ld DEBUGVAR(1, 45, "ics_info(): window_shape"));

	if (ics->window_sequence == EIGHT_SHORT_SEQUENCE)
	{
		ics->max_sfb = (uint8_t) faad_getbits(ld, 4 DEBUGVAR(1, 46, "ics_info(): max_sfb (short)"));
		ics->scale_factor_grouping = (uint8_t) faad_getbits(ld, 7
															DEBUGVAR(1, 47,
																	 "ics_info(): scale_factor_grouping"));
	}
	else
	{
		ics->max_sfb = (uint8_t) faad_getbits(ld, 6 DEBUGVAR(1, 48, "ics_info(): max_sfb (long)"));
	}

	/* get the grouping information */
	if ((retval = window_grouping_info_faad(hDecoder, ics)) > 0)
		return retval;

	/* should be an error */
	/* check the range of max_sfb */
	if (ics->max_sfb > ics->num_swb)
		return 16;

	if (ics->window_sequence != EIGHT_SHORT_SEQUENCE)
	{
		/*predictor_data_present */ faad_get1bit(ld
												 DEBUGVAR(1, 49,
														  "ics_info(): predictor_data_present"));
	}

	return retval;
}

/* Table 4.4.7 */
static uint8_t pulse_data(ic_stream * ics, pulse_info * pul, bitfile * ld)
{
	uint8_t i;

	pul->number_pulse = (uint8_t) faad_getbits(ld, 2 DEBUGVAR(1, 56, "pulse_data(): number_pulse"));
	pul->pulse_start_sfb = (uint8_t) faad_getbits(ld, 6
												  DEBUGVAR(1, 57, "pulse_data(): pulse_start_sfb"));

	/* check the range of pulse_start_sfb */
	if (pul->pulse_start_sfb > ics->num_swb)
		return 16;

	for (i = 0; i < pul->number_pulse + 1; i++)
	{
		pul->pulse_offset[i] = (uint8_t) faad_getbits(ld, 5
													  DEBUGVAR(1, 58,
															   "pulse_data(): pulse_offset"));
		pul->pulse_amp[i] =
			(uint8_t) faad_getbits(ld, 4 DEBUGVAR(1, 59, "pulse_data(): pulse_amp"));
	}

	return 0;
}

/* Table 4.4.10 */
static uint16_t data_stream_element(faacDecHandle hDecoder, bitfile * ld)
{
	uint8_t byte_aligned;
	uint16_t i, count;

	/* element_instance_tag = */ faad_getbits(ld, LEN_TAG
											  DEBUGVAR(1, 60,
													   "data_stream_element(): element_instance_tag"));
	byte_aligned = faad_get1bit(ld DEBUGVAR(1, 61, "data_stream_element(): byte_aligned"));
	count = (uint16_t) faad_getbits(ld, 8 DEBUGVAR(1, 62, "data_stream_element(): count"));
	if (count == 255)
	{
		count += (uint16_t) faad_getbits(ld, 8
										 DEBUGVAR(1, 63, "data_stream_element(): extra count"));
	}
	if (byte_aligned)
		hw_faad_byte_align(ld);

	for (i = 0; i < count; i++)
	{
		uint8_t data = faad_getbits(ld, LEN_BYTE
									DEBUGVAR(1, 64, "data_stream_element(): data_stream_byte"));
	}

	return count;
}

/* Table 4.4.11 */
static uint8_t fill_element(faacDecHandle hDecoder, bitfile * ld)
{
	uint16_t count;

	count = (uint16_t) faad_getbits(ld, 4 DEBUGVAR(1, 65, "fill_element(): count"));
	if (count == 15)
	{
		count += (uint16_t) faad_getbits(ld, 8 DEBUGVAR(1, 66, "fill_element(): extra count")) - 1;
	}

	if (count > 0)
	{
		while (count > 0)
		{
			faad_getbits(ld, 8 DEBUGVAR(1, 66, "fill_element(): extra count"));
			count--;
		}
	}

	return 0;
}

/* Table 4.4.24 */
static uint8_t individual_channel_stream(faacDecHandle hDecoder, element * ele,
										 bitfile * ld, ic_stream * ics, uint8_t scal_flag,
										 int16_t * spec_data)
{
	uint8_t result;

	ics->global_gain = (uint8_t) faad_getbits(ld, 8
											  DEBUGVAR(1, 67,
													   "individual_channel_stream(): global_gain"));

	if (!ele->common_window && !scal_flag)
	{
		if ((result = ics_info(hDecoder, ics, ld, ele->common_window)) > 0)
			return result;
	}

	if ((result = section_data(hDecoder, ics, ld)) > 0)
		return result;

	if ((result = scale_factor_data(hDecoder, ics, ld)) > 0)
		return result;

	if (!scal_flag)
	{
		/**
         **  NOTE: It could be that pulse data is available in scalable AAC too,
         **        as said in Amendment 1, this could be only the case for ER AAC,
         **        though. (have to check this out later)
         **/
		/* get pulse data */
		if ((ics->pulse_data_present = faad_get1bit(ld
													DEBUGVAR(1, 68,
															 "individual_channel_stream(): pulse_data_present")))
			& 1)
		{
			if ((result = pulse_data(ics, &(ics->pul), ld)) > 0)
				return result;
		}

		/* get tns data */
		if ((ics->tns_data_present = faad_get1bit(ld
												  DEBUGVAR(1, 69,
														   "individual_channel_stream(): tns_data_present")))
			& 1)
		{
			tns_data(ics, &(ics->tns), ld);
		}

		/* get gain control data */
		if (( /*gain_control_data_present */ faad_get1bit(ld
														  DEBUGVAR(1, 70,
																   "individual_channel_stream(): gain_control_data_present")))
			& 1)
		{
			return 1;
		}
	}

	/* decode the spectral data */
	if ((result = spectral_data(hDecoder, ics, ld, spec_data)) > 0)
	{
		return result;
	}

	/* pulse coding reconstruction */
	if (ics->pulse_data_present)
	{
		if (ics->window_sequence != EIGHT_SHORT_SEQUENCE)
		{
			if ((result = pulse_decode_2(ics, spec_data, 1024)) > 0)
				return result;
		}
		else
		{
			return 2;			/* pulse coding not allowed for short blocks */
		}
	}

	return 0;
}

/* Table 4.4.25 */
static uint8_t section_data(faacDecHandle hDecoder, ic_stream * ics, bitfile * ld)
{
	uint8_t g;
	uint8_t sect_esc_val, sect_bits;

	if (ics->window_sequence == EIGHT_SHORT_SEQUENCE)
		sect_bits = 3;
	else
		sect_bits = 5;
	sect_esc_val = (1 << sect_bits) - 1;

	for (g = 0; g < ics->num_window_groups; g++)
	{
		uint8_t k = 0;
		uint8_t i = 0;

		while (k < ics->max_sfb)
		{
			uint8_t sfb;
			uint8_t sect_len_incr;
			uint16_t sect_len = 0;
			uint8_t sect_cb_bits = 4;

			/* if "faad_getbits" detects error and returns "0", "k" is never
			   incremented and we cannot leave the while loop */
			if ((ld->error != 0) || (ld->no_more_reading))
				return 14;
			ics->sect_cb[g * 15 + i] = (uint8_t) faad_getbits(ld, sect_cb_bits
															  DEBUGVAR(1, 71,
																	   "section_data(): sect_cb"));

			if (ics->sect_cb[g * 15 + i] == NOISE_HCB)
				ics->noise_used = 1;

			sect_len_incr = (uint8_t) faad_getbits(ld, sect_bits
												   DEBUGVAR(1, 72,
															"section_data(): sect_len_incr"));
			while ((sect_len_incr == sect_esc_val)	/* &&
													   (k+sect_len < ics->max_sfb) */ )
			{
				sect_len += sect_len_incr;
				sect_len_incr = (uint8_t) faad_getbits(ld, sect_bits
													   DEBUGVAR(1, 72,
																"section_data(): sect_len_incr"));
			}

			sect_len += sect_len_incr;

			ics->sect_start[g * 15 + i] = k;
			ics->sect_end[g * 15 + i] = k + sect_len;

			if (k + sect_len >= 8 * 15)
				return 15;
			if (i >= 8 * 15)
				return 15;

			for (sfb = k; sfb < k + sect_len; sfb++)
				ics->sfb_cb[g * 15 + sfb] = ics->sect_cb[g * 15 + i];
			k += sect_len;
			i++;
		}
		ics->num_sec[g] = i;
	}
	return 0;
}

/*
 *  decode_scale_factors()
 *   decodes the scalefactors from the bitstream
 */
/*
 * All scalefactors (and also the stereo positions and pns energies) are
 * transmitted using Huffman coded DPCM relative to the previous active
 * scalefactor (respectively previous stereo position or previous pns energy,
 * see subclause 4.6.2 and 4.6.3). The first active scalefactor is
 * differentially coded relative to the global gain.
 */
static uint8_t decode_scale_factors(ic_stream * ics, bitfile * ld)
{
	uint8_t g, sfb;
	int16_t t;
	int8_t noise_pcm_flag = 1;

	int16_t scale_factor = ics->global_gain;
	int16_t is_position = 0;
	int16_t noise_energy = ics->global_gain - 90;

	for (g = 0; g < ics->num_window_groups; g++)
	{
		for (sfb = 0; sfb < ics->max_sfb; sfb++)
		{
			switch (ics->sfb_cb[g * 15 + sfb])

			{
			case ZERO_HCB:		/* zero book */
				ics->scale_factors[g * 15 + sfb] = 0;
				break;
			case INTENSITY_HCB:	/* intensity books */
			case INTENSITY_HCB2:

				/* decode intensity position */
				t = hw_huffman_scale_factor(ld);
				if (t < 0)
					return 9;
				is_position += (t - 60);
				ics->scale_factors[g * 15 + sfb] = is_position;

				break;
			case NOISE_HCB:	/* noise books */

				/* decode noise energy */
				if (noise_pcm_flag)
				{
					noise_pcm_flag = 0;
					t = (int16_t) faad_getbits(ld, 9
											   DEBUGVAR(1, 73,
														"scale_factor_data(): first noise")) - 256;
				}
				else
				{
					t = hw_huffman_scale_factor(ld);
					if (t < 0)
						return 9;
					t -= 60;
				}
				noise_energy += t;
				ics->scale_factors[g * 15 + sfb] = noise_energy;

				break;
			default:			/* spectral books */

				/* decode scale factor */
				t = hw_huffman_scale_factor(ld);
				if (t < 0)
					return 9;
				scale_factor += (t - 60);
				if (scale_factor < 0 || scale_factor > 255)
					return 4;
				ics->scale_factors[g * 15 + sfb] = scale_factor;

				break;
			}
		}
	}

	return 0;
}

/* Table 4.4.26 */
static uint8_t scale_factor_data(faacDecHandle hDecoder, ic_stream * ics, bitfile * ld)
{
	return decode_scale_factors(ics, ld);
}

/* Table 4.4.27 */
static void tns_data(ic_stream * ics, tns_info * tns, bitfile * ld)
{
	uint8_t w, filt, i, start_coef_bits, coef_bits;
	uint8_t n_filt_bits = 2;
	uint8_t length_bits = 6;
	uint8_t order_bits = 5;

	if (ics->window_sequence == EIGHT_SHORT_SEQUENCE)
	{
		n_filt_bits = 1;
		length_bits = 4;
		order_bits = 3;
	}

	for (w = 0; w < ics->num_windows; w++)
	{
		tns->n_filt[w] = (uint8_t) faad_getbits(ld, n_filt_bits
												DEBUGVAR(1, 74, "tns_data(): n_filt"));

		if (tns->n_filt[w])
		{
			if ((tns->coef_res[w] = faad_get1bit(ld DEBUGVAR(1, 75, "tns_data(): coef_res"))) & 1)
			{
				start_coef_bits = 4;
			}
			else
			{
				start_coef_bits = 3;
			}
		}

		for (filt = 0; filt < tns->n_filt[w]; filt++)
		{
			tns->length[w][filt] = (uint8_t) faad_getbits(ld, length_bits
														  DEBUGVAR(1, 76, "tns_data(): length"));
			tns->order[w][filt] = (uint8_t) faad_getbits(ld, order_bits
														 DEBUGVAR(1, 77, "tns_data(): order"));
			if (tns->order[w][filt])
			{
				tns->direction[w][filt] = faad_get1bit(ld DEBUGVAR(1, 78, "tns_data(): direction"));
				tns->coef_compress[w][filt] = faad_get1bit(ld
														   DEBUGVAR(1, 79,
																	"tns_data(): coef_compress"));

				coef_bits = start_coef_bits - tns->coef_compress[w][filt];
				for (i = 0; i < tns->order[w][filt]; i++)
				{
					tns->coef[w][filt][i] = (uint8_t) faad_getbits(ld, coef_bits
																   DEBUGVAR(1, 80,
																			"tns_data(): coef"));
				}
			}
		}
	}
}

/* Table 4.4.29 */
static uint8_t spectral_data(faacDecHandle hDecoder, ic_stream * ics, bitfile * ld,
							 int16_t * spectral_data)
{
	int8_t i;
	uint8_t g;
	int16_t *sp;
	uint16_t k, p = 0;
	uint8_t groups = 0;
	uint8_t sect_cb;
	uint8_t result;

	sp = spectral_data;

	for (g = 0; g < ics->num_window_groups; g++)
	{
		p = groups * 128;

		for (i = 0; i < ics->num_sec[g]; i++)
		{
			sect_cb = ics->sect_cb[g * 15 + i];
			switch (sect_cb)
			{
			case ZERO_HCB:
			case NOISE_HCB:
			case INTENSITY_HCB:
			case INTENSITY_HCB2:
				p += (ics->sect_sfb_offset[g * 15 + ics->sect_end[g * 15 + i]] -
					  ics->sect_sfb_offset[g * 15 + ics->sect_start[g * 15 + i]]);

				break;
			default:
				for (k = ics->sect_sfb_offset[g * 15 + ics->sect_start[g * 15 + i]];
					 k < ics->sect_sfb_offset[g * 15 + ics->sect_end[g * 15 + i]]; k += 4)
				{
					sp = spectral_data + p;

					if ((result = hw_huffman_spectral_data(sect_cb, ld, sp)) > 0)
						return result;
					if (sect_cb >= FIRST_PAIR_HCB)
					{
						if ((result = hw_huffman_spectral_data(sect_cb, ld, sp + 2)) > 0)
							return result;
					}
					p += 4;
				}
				break;
			}
		}
		groups += ics->window_group_length[g];
	}

	return 0;
}

/* Table 1.A.2 */
void hw_get_adif_header(adif_header * adif, bitfile * ld)
{
	uint8_t i;

	/* adif_id[0] = */ faad_getbits(ld, 8
									DEBUGVAR(1, 106, "hw_get_adif_header(): adif_id[0]"));
	/* adif_id[1] = */ faad_getbits(ld, 8
									DEBUGVAR(1, 107, "hw_get_adif_header(): adif_id[1]"));
	/* adif_id[2] = */ faad_getbits(ld, 8
									DEBUGVAR(1, 108, "hw_get_adif_header(): adif_id[2]"));
	/* adif_id[3] = */ faad_getbits(ld, 8
									DEBUGVAR(1, 109, "hw_get_adif_header(): adif_id[3]"));
	if (faad_get1bit(ld DEBUGVAR(1, 110, "hw_get_adif_header(): copyright_id_present")))
	{
		for (i = 0; i < 72 / 8; i++)
		{
			adif->copyright_id[i] = (int8_t) faad_getbits(ld, 8
														  DEBUGVAR(1, 111,
																   "hw_get_adif_header(): copyright_id"));
		}
		adif->copyright_id[i] = 0;
	}
	/*original_copy */ faad_get1bit(ld
									DEBUGVAR(1, 112, "hw_get_adif_header(): original_copy"));
	/*adif->home */ faad_get1bit(ld
								 DEBUGVAR(1, 113, "hw_get_adif_header(): home"));
	adif->bitstream_type = faad_get1bit(ld DEBUGVAR(1, 114, "hw_get_adif_header(): bitstream_type"));
	adif->bitrate = faad_getbits(ld, 23 DEBUGVAR(1, 115, "hw_get_adif_header(): bitrate"));
	adif->num_program_config_elements = (uint8_t) faad_getbits(ld, 4
															   DEBUGVAR(1, 116,
																		"hw_get_adif_header(): num_program_config_elements"));

	for (i = 0; i < adif->num_program_config_elements + 1; i++)
	{
		if (adif->bitstream_type == 0)
		{
			/*adif_buffer_fullness */ faad_getbits(ld, 20
												   DEBUGVAR(1, 117,
															"hw_get_adif_header(): adif_buffer_fullness"));
		}

		program_config_element(&adif->pce[i], ld);
	}
}

/* Table 1.A.5 */
uint8_t hw_adts_frame(adts_header * adts, bitfile * ld)
{
	/* faad_byte_align(ld); */
	if (adts_fixed_header(adts, ld))
		return 5;
	adts_variable_header(adts, ld);
	adts_error_check(adts, ld);

	return 0;
}

/* Table 1.A.6 */
static uint8_t adts_fixed_header(adts_header * adts, bitfile * ld)
{
	uint16_t i;
	uint8_t sync_err = 1;

	/* try to recover from sync errors */
	for (i = 0; i < 768; i++)
	{
		adts->syncword = (uint16_t) faad_showbits(ld, 12);
		if (adts->syncword != 0xFFF)
		{
			faad_getbits(ld, 8 DEBUGVAR(0, 0, ""));
		}
		else
		{
			sync_err = 0;
			faad_getbits(ld, 12 DEBUGVAR(1, 118, "adts_fixed_header(): syncword"));
			break;
		}
	}
	if (sync_err)
		return 5;

	adts->id = faad_get1bit(ld DEBUGVAR(1, 119, "adts_fixed_header(): id"));
	adts->layer = (uint8_t) faad_getbits(ld, 2 DEBUGVAR(1, 120, "adts_fixed_header(): layer"));
	adts->protection_absent = faad_get1bit(ld
										   DEBUGVAR(1, 121,
													"adts_fixed_header(): protection_absent"));
	adts->profile = (uint8_t) faad_getbits(ld, 2 DEBUGVAR(1, 122, "adts_fixed_header(): profile"));
	adts->sf_index = (uint8_t) faad_getbits(ld, 4
											DEBUGVAR(1, 123, "adts_fixed_header(): sf_index"));
	adts->private_bit = faad_get1bit(ld DEBUGVAR(1, 124, "adts_fixed_header(): private_bit"));
	adts->channel_configuration = (uint8_t) faad_getbits(ld, 3
														 DEBUGVAR(1, 125,
																  "adts_fixed_header(): channel_configuration"));
	adts->original = faad_get1bit(ld DEBUGVAR(1, 126, "adts_fixed_header(): original"));
	/*home */ faad_get1bit(ld
						   DEBUGVAR(1, 127, "adts_fixed_header(): home"));

	if (adts->old_format == 1)
	{
		/* Removed in corrigendum 14496-3:2002 */
		if (adts->id == 0)
		{
			adts->emphasis = (uint8_t) faad_getbits(ld, 2
													DEBUGVAR(1, 128,
															 "adts_fixed_header(): emphasis"));
		}
	}

	return 0;
}

/* Table 1.A.7 */
static void adts_variable_header(adts_header * adts, bitfile * ld)
{
	adts->copyright_identification_bit = faad_get1bit(ld
													  DEBUGVAR(1, 129,
															   "adts_variable_header(): copyright_identification_bit"));
	adts->copyright_identification_start =
		faad_get1bit(ld DEBUGVAR(1, 130, "adts_variable_header(): copyright_identification_start"));
	adts->aac_frame_length =
		(uint16_t) faad_getbits(ld,
								13 DEBUGVAR(1, 131, "adts_variable_header(): aac_frame_length"));
	adts->adts_buffer_fullness =
		(uint16_t) faad_getbits(ld,
								11 DEBUGVAR(1, 132,
											"adts_variable_header(): adts_buffer_fullness"));
	adts->no_raw_data_blocks_in_frame =
		(uint8_t) faad_getbits(ld,
							   2 DEBUGVAR(1, 133,
										  "adts_variable_header(): no_raw_data_blocks_in_frame"));
}

/* Table 1.A.8 */
static void adts_error_check(adts_header * adts, bitfile * ld)
{
	if (adts->protection_absent == 0)
	{
		adts->crc_check = (uint16_t) faad_getbits(ld, 16
												  DEBUGVAR(1, 134,
														   "adts_error_check(): crc_check"));
	}
}
