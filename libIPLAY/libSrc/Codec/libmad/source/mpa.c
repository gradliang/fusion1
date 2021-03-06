/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      layer3.c
*
* Programmer:    Brenda Li
*                MPX E120 division
*
* Created:   03/30/2005
*
* Description:   MPEG audio decoder library
*              
*        
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
#include "mptrace.h"


# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# include "global.h"

# include <stdlib.h>
# include <string.h>

# ifdef HAVE_ASSERT_H
#  include <assert.h>
# endif

# include "fixed.h"
# include "bit.h"
# include "stream.h"
# include "frame.h"
# include "huffman.h"
# include "layer3.h"
//# include "ad_internal.h"
//# include "filter_graph.h" 
# include "Sine_lw0_h.h"
# include "Sine_lw0_l.h"
# include "Sine_lw1_h.h"
# include "Sine_lw1_l.h"
# include "Kbd_lw0_h.h"
# include "Kbd_lw0_l.h"
# include "Kbd_lw1_h.h"
# include "Kbd_lw1_l.h"
# include "Sine_sw.h"
# include "Kbd_sw.h"
# include "PcmOutput.h"

//#include <math.h>   //must include this file. or the sin function will be failed.
#include "codec.h"

/* --- Layer III ----------------------------------------------------------- */
enum
{
	count1table_select = 0x01,
	scalefac_scale = 0x02,
	preflag = 0x04,
	mixed_block_flag = 0x08
};

enum
{
	I_STEREO = 0x1,
	MS_STEREO = 0x2
};

struct sideinfo
{
	unsigned int main_data_begin;
	unsigned int private_bits;

	unsigned char scfsi[2];

	struct granule
	{
		struct channel
		{
			/* from side info */
			unsigned short part2_3_length;
			unsigned short big_values;
			unsigned short global_gain;
			unsigned short scalefac_compress;

			unsigned char flags;
			unsigned char block_type;
			unsigned char table_select[3];
			unsigned char subblock_gain[3];
			unsigned char region0_count;
			unsigned char region1_count;

			/* from main_data */
			unsigned char scalefac[39];	/* scalefac_l and/or scalefac_s */
			unsigned char window_switching_flag;	//Added by Sunny Zeng
		} ch[2];
	} gr[2];
};

/*
 * scalefactor bit lengths
 * derived from section 2.4.2.7 of ISO/IEC 11172-3
 */
static struct
{
	unsigned char slen1;
	unsigned char slen2;
} const sflen_table[16] = {
	{0, 0}, {0, 1}, {0, 2}, {0, 3},
	{3, 0}, {1, 1}, {1, 2}, {1, 3},
	{2, 1}, {2, 2}, {2, 3}, {3, 1},
	{3, 2}, {3, 3}, {4, 2}, {4, 3}
};

/*
 * number of LSF scalefactor band values
 * derived from section 2.4.3.2 of ISO/IEC 13818-3
 */
static unsigned char const nsfb_table[6][3][4] = {
	{{6, 5, 5, 5},
	 {9, 9, 9, 9},
	 {6, 9, 9, 9}},

	{{6, 5, 7, 3},
	 {9, 9, 12, 6},
	 {6, 9, 12, 6}},

	{{11, 10, 0, 0},
	 {18, 18, 0, 0},
	 {15, 18, 0, 0}},

	{{7, 7, 7, 0},
	 {12, 12, 12, 0},
	 {6, 15, 12, 0}},

	{{6, 6, 6, 3},
	 {12, 9, 9, 6},
	 {6, 12, 9, 6}},

	{{8, 8, 5, 0},
	 {15, 12, 9, 0},
	 {6, 18, 9, 0}}
};

/*
 * MPEG-1 scalefactor band widths
 * derived from Table B.8 of ISO/IEC 11172-3
 */
static unsigned char const sfb_48000_long[] = {
	4, 4, 4, 4, 4, 4, 6, 6, 6, 8, 10,
	12, 16, 18, 22, 28, 34, 40, 46, 54, 54, 192
};

static unsigned char const sfb_44100_long[] = {
	4, 4, 4, 4, 4, 4, 6, 6, 8, 8, 10,
	12, 16, 20, 24, 28, 34, 42, 50, 54, 76, 158
};

static unsigned char const sfb_32000_long[] = {
	4, 4, 4, 4, 4, 4, 6, 6, 8, 10, 12,
	16, 20, 24, 30, 38, 46, 56, 68, 84, 102, 26
};

static unsigned char const sfb_48000_short[] = {
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6,
	6, 6, 6, 6, 6, 10, 10, 10, 12, 12, 12, 14, 14,
	14, 16, 16, 16, 20, 20, 20, 26, 26, 26, 66, 66, 66
};

static unsigned char const sfb_44100_short[] = {
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6,
	6, 6, 8, 8, 8, 10, 10, 10, 12, 12, 12, 14, 14,
	14, 18, 18, 18, 22, 22, 22, 30, 30, 30, 56, 56, 56
};

static unsigned char const sfb_32000_short[] = {
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6,
	6, 6, 8, 8, 8, 12, 12, 12, 16, 16, 16, 20, 20,
	20, 26, 26, 26, 34, 34, 34, 42, 42, 42, 12, 12, 12
};

static unsigned char const sfb_48000_mixed[] = {
	/* long */ 4, 4, 4, 4, 4, 4, 6, 6,
	/* short */ 4, 4, 4, 6, 6, 6, 6, 6, 6, 10,
	10, 10, 12, 12, 12, 14, 14, 14, 16, 16,
	16, 20, 20, 20, 26, 26, 26, 66, 66, 66
};

static unsigned char const sfb_44100_mixed[] = {
	/* long */ 4, 4, 4, 4, 4, 4, 6, 6,
	/* short */ 4, 4, 4, 6, 6, 6, 8, 8, 8, 10,
	10, 10, 12, 12, 12, 14, 14, 14, 18, 18,
	18, 22, 22, 22, 30, 30, 30, 56, 56, 56
};

static unsigned char const sfb_32000_mixed[] = {
	/* long */ 4, 4, 4, 4, 4, 4, 6, 6,
	/* short */ 4, 4, 4, 6, 6, 6, 8, 8, 8, 12,
	12, 12, 16, 16, 16, 20, 20, 20, 26, 26,
	26, 34, 34, 34, 42, 42, 42, 12, 12, 12
};

/*
 * MPEG-2 scalefactor band widths
 * derived from Table B.2 of ISO/IEC 13818-3
 */
static unsigned char const sfb_24000_long[] = {
	6, 6, 6, 6, 6, 6, 8, 10, 12, 14, 16,
	18, 22, 26, 32, 38, 46, 54, 62, 70, 76, 36
};

static unsigned char const sfb_22050_long[] = {
	6, 6, 6, 6, 6, 6, 8, 10, 12, 14, 16,
	20, 24, 28, 32, 38, 46, 52, 60, 68, 58, 54
};

# define sfb_16000_long  sfb_22050_long

static unsigned char const sfb_24000_short[] = {
	4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 6, 8,
	8, 8, 10, 10, 10, 12, 12, 12, 14, 14, 14, 18, 18,
	18, 24, 24, 24, 32, 32, 32, 44, 44, 44, 12, 12, 12
};

static unsigned char const sfb_22050_short[] = {
	4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 6, 6,
	6, 6, 8, 8, 8, 10, 10, 10, 14, 14, 14, 18, 18,
	18, 26, 26, 26, 32, 32, 32, 42, 42, 42, 18, 18, 18
};

static unsigned char const sfb_16000_short[] = {
	4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 6, 8,
	8, 8, 10, 10, 10, 12, 12, 12, 14, 14, 14, 18, 18,
	18, 24, 24, 24, 30, 30, 30, 40, 40, 40, 18, 18, 18
};

static unsigned char const sfb_24000_mixed[] = {
	/* long */ 6, 6, 6, 6, 6, 6,
	/* short */ 6, 6, 6, 8, 8, 8, 10, 10, 10, 12,
	12, 12, 14, 14, 14, 18, 18, 18, 24, 24,
	24, 32, 32, 32, 44, 44, 44, 12, 12, 12
};

static unsigned char const sfb_22050_mixed[] = {
	/* long */ 6, 6, 6, 6, 6, 6,
	/* short */ 6, 6, 6, 6, 6, 6, 8, 8, 8, 10,
	10, 10, 14, 14, 14, 18, 18, 18, 26, 26,
	26, 32, 32, 32, 42, 42, 42, 18, 18, 18
};

static unsigned char const sfb_16000_mixed[] = {
	/* long */ 6, 6, 6, 6, 6, 6,
	/* short */ 6, 6, 6, 8, 8, 8, 10, 10, 10, 12,
	12, 12, 14, 14, 14, 18, 18, 18, 24, 24,
	24, 30, 30, 30, 40, 40, 40, 18, 18, 18
};

/*
 * MPEG 2.5 scalefactor band widths
 * derived from public sources
 */
# define sfb_12000_long  sfb_16000_long
# define sfb_11025_long  sfb_12000_long

static unsigned char const sfb_8000_long[] = {
	12, 12, 12, 12, 12, 12, 16, 20, 24, 28, 32,
	40, 48, 56, 64, 76, 90, 2, 2, 2, 2, 2
};

# define sfb_12000_short  sfb_16000_short
# define sfb_11025_short  sfb_12000_short

static unsigned char const sfb_8000_short[] = {
	8, 8, 8, 8, 8, 8, 8, 8, 8, 12, 12, 12, 16,
	16, 16, 20, 20, 20, 24, 24, 24, 28, 28, 28, 36, 36,
	36, 2, 2, 2, 2, 2, 2, 2, 2, 2, 26, 26, 26
};

# define sfb_12000_mixed  sfb_16000_mixed
# define sfb_11025_mixed  sfb_12000_mixed

/* the 8000 Hz short block scalefactor bands do not break after
   the first 36 frequency lines, so this is probably wrong */
static unsigned char const sfb_8000_mixed[] = {
	/* long */ 12, 12, 12,
	/* short */ 4, 4, 4, 8, 8, 8, 12, 12, 12, 16, 16, 16,
	20, 20, 20, 24, 24, 24, 28, 28, 28, 36, 36, 36,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 26, 26, 26
};

static struct
{
	unsigned char const *l;
	unsigned char const *s;
	unsigned char const *m;
} const sfbwidth_table[9] = {
	{sfb_48000_long, sfb_48000_short, sfb_48000_mixed},
	{sfb_44100_long, sfb_44100_short, sfb_44100_mixed},
	{sfb_32000_long, sfb_32000_short, sfb_32000_mixed},
	{sfb_24000_long, sfb_24000_short, sfb_24000_mixed},
	{sfb_22050_long, sfb_22050_short, sfb_22050_mixed},
	{sfb_16000_long, sfb_16000_short, sfb_16000_mixed},
	{sfb_12000_long, sfb_12000_short, sfb_12000_mixed},
	{sfb_11025_long, sfb_11025_short, sfb_11025_mixed},
	{sfb_8000_long, sfb_8000_short, sfb_8000_mixed}
};

/*
 * scalefactor band preemphasis (used only when preflag is set)
 * derived from Table B.6 of ISO/IEC 11172-3
 */
static unsigned char const pretab[22] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 3, 3, 3, 2, 0
};


#include "tables.h"
#include "utilregfile.h"
#include "mpa_define.h"
#include "di.h"
/***************************Fill mp3 Registers********************************/
struct mp3_regs
{
	int is_bound[3];
	int is_stereo;
	int mix_bound;
	int version;
	int block_type[2][2];
	int mixed_block_flag[2][2];
};


extern int n_frames;

void dump_mpa_status(const char *prefix)
{
	MPA *mpa = (MPA *) MPA_BASE;
	char *status_str[] = { "Idle",
		"Ch0 Fill", "Ch1 Fill", "Ch0 Wait",
		"Stereo Decode",
		"Ch0 PNS", "Ch1 PNS", "Ch0 TNS", "Ch1 TNS",
		"Ch0 IMDCT", "Ch1 IMDCT", "Ch0 SYNFB", "Ch1 SYNFB",
	};
	int status_id = mpa->MpaStatus;

	UartOutText((BYTE *)prefix);
	UartOutText("(");
	if (status_id < sizeof(status_str) / sizeof(char *))
	{
		UartOutText(status_str[status_id]);
	}
	else
	{
		UartOutText("(unknown MPA status)");
	}
	UartOutText(")");
}

/*!
   @defgroup    LAYERIII    LayerIII
    Define Mp3 decoder's subfunctions.
   @{
*/
#if 0
static inline enum mad_error fill_mp3_regs(struct sideinfo si,
										   struct mad_header *header,
										   struct mp3_regs *regs,
										   unsigned int nch,
										   unsigned short is[576],
										   short gr, short ch) __attribute__ ((always_inline));
#endif

#if 0
static inline enum mad_error
fill_mp3_regs(struct sideinfo si,
			  struct mad_header *header,
			  struct mp3_regs *regs, unsigned int nch, unsigned short is[576], short gr, short ch)
{
	short *l_sfbptr, *s_sfbptr;
	int win;
	int sfb;
	int lines;
	int i, j;
	int max;
	int k;
	int temp_r;
	MPA *mpa;
	CHANNEL *rdma;
	unsigned short spec;
	int ok;
	

	//static unsigned short myis[576];
	mpa = (MPA *) MPA_BASE;

	regs->is_bound[0] = regs->is_bound[1] = regs->is_bound[2] = 0;
	regs->mix_bound = 0;
	regs->version = 0;

	if (ch == 0 && gr == 0)
	{
		if ((3 - header->mode) == JOINT_STEREO && (header->mode_extension & 0x2))
		{
			mpa->MsMode = 1;
		}
		else
		{
			mpa->MsMode = 0;
		}						//from header
		if ((3 - header->mode) == JOINT_STEREO && (header->mode_extension & 0x1))
		{
                         mpa->Ismsmixed=1;//06.08 rebecca modified
			regs->is_stereo = 1;
		}
	 else
        {
             mpa->Ismsmixed=0;//06.08 rebecca modified
            if( ((3 - header->mode) == JOINT_STEREO ) ||((3 - header->mode) == STEREO ) ||((3 - header->mode) == DUAL_CHANNEL) )//06.08 rebecca modified
                regs->is_stereo = 1; //06.08 rebecca modified
            else //06.08 rebecca modified
               regs->is_stereo = 0;
        }                       //from header
        mpa->Mono = (nch == 1) ? 1 : 0;
        mpa->Mp3Id = (header->flags & MAD_FLAG_LSF_EXT) ? 0 : 1;
        mpa->Voffset = 0;
        mpa->Op_Mode = 3;

    }

//[7:6]-gr0ch0;[5:4]-gr0ch1;[3:2]-gr1ch0;[1:0]-gr1ch1
	i = (3 - (gr << 1) - ch);
	temp_r = (mpa->BlkType) & (~(0x3 << (i << 1)));  //6 - gr * 4 - ch * 2
	mpa->BlkType = temp_r | (si.gr[gr].ch[ch].block_type & 0x3) << (i << 1);

	temp_r = (mpa->MixedBlkFlag) & (~(1 << i));
	mpa->MixedBlkFlag =
		temp_r | ((si.gr[gr].ch[ch].flags & mixed_block_flag) ? 1 : 0) << i;

	temp_r = (mpa->GlobalGain) & (~(0xff << (i << 3)));
	mpa->GlobalGain = temp_r | si.gr[gr].ch[ch].global_gain << (i << 3);

	temp_r = (mpa->ScalefacScale) & (~(1 << i));
	mpa->ScalefacScale =
		temp_r | ((si.gr[gr].ch[ch].flags & scalefac_scale) ? 1 : 0) << i;

	temp_r = (mpa->Preflag) & (~(1 << i));
	mpa->Preflag = ((si.gr[gr].ch[ch].flags & preflag) ? 1 : 0) << i;

	temp_r = (si.gr[0].ch[0].subblock_gain[0] << 15) +
		(si.gr[0].ch[0].subblock_gain[1] << 12) +
		(si.gr[0].ch[0].subblock_gain[2] << 9) +
		(si.gr[0].ch[1].subblock_gain[0] << 6) +
		(si.gr[0].ch[1].subblock_gain[1] << 3) + si.gr[0].ch[1].subblock_gain[2];
	mpa->SubGain0 = temp_r;

	temp_r = (si.gr[1].ch[0].subblock_gain[0] << 15) +
		(si.gr[1].ch[0].subblock_gain[1] << 12) +
		(si.gr[1].ch[0].subblock_gain[2] << 9) +
		(si.gr[1].ch[1].subblock_gain[0] << 6) +
		(si.gr[1].ch[1].subblock_gain[1] << 3) + si.gr[1].ch[1].subblock_gain[2];
	mpa->SubGain1 = temp_r;

	if (!(header->flags & MAD_FLAG_MPEG_2_5_EXT))
		regs->version = 2;
	if (!(header->flags & MAD_FLAG_LSF_EXT))
		regs->version += 1;
	mpa->Version = regs->version;

	if (regs->version == MPEG1)
	{
		l_sfbptr = l_sfb_tab[0][header->sample_freq];
		s_sfbptr = s_sfb_tab[0][header->sample_freq];
	}
	else if (regs->version == MPEG2)
	{
		l_sfbptr = l_sfb_tab[1][header->sample_freq];
		s_sfbptr = s_sfb_tab[1][header->sample_freq];
	}
	else
	{
		l_sfbptr = l_sfb_tab[2][header->sample_freq];
		s_sfbptr = s_sfb_tab[2][header->sample_freq];
	}

	if (regs->is_stereo && ch)
	{
		if (regs->block_type[gr][1] == 2)
		{						// short block
			for (win = 0; win < 3; win++)
			{
				sfb = 12;
				while (sfb >= 0)
				{
					lines = s_sfbptr[sfb + 1] - s_sfbptr[sfb];
					j = s_sfbptr[sfb] * 3 + (win + 1) * lines - 1;
					while ((lines > 0) && (is[j] == 0))
					{
						j--;
						lines--;
					}
					if (lines != 0)
						break;
					else
						sfb--;
				}
				regs->is_bound[win] = sfb + 1;
			}

			if (regs->mixed_block_flag[gr][1])
			{
				regs->mix_bound = -1;	// init to invalid value
				max = regs->is_bound[0];
				if (regs->is_bound[1] > max);
				max = regs->is_bound[1];
				if (regs->is_bound[2] > max);
				max = regs->is_bound[2];

				if (max <= 3)
				{				// if nonzero down to subband 0, 1
					regs->is_bound[0] = regs->is_bound[1] = regs->is_bound[2] = 3;
					j = 35;
					while ((j >= 0) && (is[j] == 0))
						j--;
					sfb = 0;	// search from sfb 0 upward
					while (j >= l_sfbptr[sfb])
						sfb++;
					regs->mix_bound = sfb;	// set to new value
				}
			}
		}
		else
		{						// long block
			j = 575;
			while ((j >= 0) && (is[j] == 0))
				j--;
			sfb = 21;
			while ((sfb >= 0) && (j < l_sfbptr[sfb]))
				sfb--;
			regs->is_bound[0] = sfb + 1;
		}
	}

	mpa->Ismsmixed += (regs->mix_bound << 1) ;

//      mpa->IsBound = (regs->is_bound[0]<<10)+(regs->is_bound[1]<<5)+(regs->is_bound[2]);   //Spec discribed 
	mpa->IsBound = (regs->is_bound[0]) + (regs->is_bound[1] << 5) + (regs->is_bound[2] << 10);	//Script
	//Write scale factors to a fixed address.

	/* Checking/Waiting MPA, according to MPA state machine */
	//6.13 rebecca mark begin
/*	ok = 1;
	if (ch == 0)
	{
		ok = PollRegister32(mpa->MpaStatus, MPA_SEQ_IDLE, 0x0F, 10 * 1000);
		if (!ok)
			UartOutText("(MPA must in Idle)");
	}
	else
	{
		ok = PollRegister32(mpa->MpaStatus, MPA_SEQ_CH1_FILL, 0x0F, 10 * 1000);
		if (!ok)
			UartOutText("(MPA must in Ch1_Fill)");
	}
	if (!ok)
	{
		dump_mpa_status("decode1");
		//return MAD_ERROR_DECODER_FAIL;
	}*/
//6.13 rebecca mark end
	/* Send audio data via MailBox */
	mpa->MbRst = 1;
	unsigned char *pScalefac;
	pScalefac = &si.gr[gr].ch[ch].scalefac[0];
	if (ch == 1)
	{
		for (j = 0; j < 22; j++)
			mpa->ScalefacMb = 0;
		for (j = 0; j < 22; j++)
			mpa->ScalefacMb = pScalefac[j];
		for (j = 0; j < 39; j++)
			mpa->ScalefacMb = 0;
		for (j = 0; j < 39; j++)
			mpa->ScalefacMb = pScalefac[j];
	}
	else
	{
		for (j = 0; j < 22; j++)
			mpa->ScalefacMb = pScalefac[j];
		for (j = 0; j < 22; j++)
			mpa->ScalefacMb = 0;
		for (j = 0; j < 39; j++)
			mpa->ScalefacMb = pScalefac[j];
		for (j = 0; j < 39; j++)
			mpa->ScalefacMb = 0;
	}
	//Start DMA in
#if 0
	for (i = 0; i < 576; i += 2)
	{
		myis[i] = is[i];
		myis[i + 1] = is[i + 1];
	}
#endif
	rdma = (CHANNEL *) DMA_MPAR_BASE;
	rdma->StartA = (DWORD) & is[0];
	rdma->EndA = (DWORD) ((BYTE *) & is[0] + 1152 - 1);
	rdma->Control = 1;
	if (gr)
		mpa->MpaCtrl = 0x1011;
	else
		mpa->MpaCtrl = 0x11;

	/* Waiting MPA, according to MPA state machine */
	ok = 1;
	extern DWORD pre_parse;
	if (pre_parse == 2) return MPA_ERROR_NONE;		
	
	if (ch == 0)
	{
		/* MPA transit from Idle => Ch0_Fill => Ch1_Fill. */
		/* we wait for Ch1_Fill directly. */
#if 0		
        if (ok)
        {
            ok = PollRegister32(mpa->MpaStatus, MPA_SEQ_CH0_FILL, 0x0F, 500*1000);
            if (!ok)
                UartOutText("(MPA Ch0 Fill fail)");
        }
#endif	
		if (regs->is_stereo && ok)
		{
		//ok = PollRegister32(mpa->MpaStatus, MPA_SEQ_CH1_FILL, 0x0F, 500 * 1000);
		//while(1)  if (mpa->MpaStatus==MPA_SEQ_CH1_FILL) break;
		IntEnable();//make tickcount can go on
		ok = PollReg(&mpa->MpaStatus, MPA_SEQ_CH1_FILL, 0x0F, 500 * 1000);
			if (!ok)
				UartOutText("(MPA Ch1 Fill fail)");
		}
	
	}
	else
	{
		/* MPA should transit from Ch1_Fill => Ch0_Wait => Stereo Decode => Ch0 IMDCT => Ch1 IMDCT => Ch0 SYNFB => Ch1 SYNFB => Idle */
		/* We wait for Idle directly. (Bug: can't get Ch0_SFB) */
//        if (ok)
//        {
//            ok = PollRegister32(mpa->MpaStatus, MPA_SEQ_CH0_WAIT, 0x0F, 20*1000);
//            if (!ok)
//                UartOutText("(MPA Ch0 Wait fail)");
//        }
//        if (ok)
//        {
//            ok = BusyPollRegister32(mpa->MpaStatus, MPA_SEQ_STDEC, 0x0F, 20*1000);
//            if (!ok)
//                UartOutText("(MPA Stereo Decode fail)");
//        }
//        if (ok)
//        {
//            ok = PollRegister32(mpa->MpaStatus, MPA_SEQ_CH0_IMDCT, 0x0F, 20*1000);
//            if (!ok)
//                UartOutText("(MPA Ch0 IMDCT fail)");
//        }
//        if (ok)
//        {
//            ok = PollRegister32(mpa->MpaStatus, MPA_SEQ_CH1_IMDCT, 0x0F, 20*1000);
//            if (!ok)
//                UartOutText("(MPA Ch1 IMDCT fail)");
//        }
		/* Bug: why can't get Ch0_SFB??? */
//        if (ok)
//        {
//          ok = BusyPollRegister32(mpa->MpaStatus, MPA_SEQ_CH0_SFB, 0x0F, 20*1000);
//            if (!ok)
//                UartOutText("(MPA Ch0 SFB fail)");
//        }
//        if (ok)
//        {
//            ok = PollRegister32(mpa->MpaStatus, MPA_SEQ_CH1_SFB, 0x0F, 20*1000);
//            if (!ok)
//                UartOutText("(MPA Ch1 SFB fail)");
//        }
		if (ok)
		{
			//ok = PollRegister32(mpa->MpaStatus, MPA_SEQ_IDLE, 0x0F, 500 * 1000);
			//while(1)  if (mpa->MpaStatus==MPA_SEQ_IDLE) break;
			IntEnable();//make tickcount can go on
			ok = PollReg(&mpa->MpaStatus, MPA_SEQ_IDLE, 0x0F, 500 * 1000);
			if (!ok)
				UartOutText("(MPA Idle fail)");
		}
	}
	if (!ok)
	{
		dump_mpa_status("decode2");
		return MAD_ERROR_DECODER_FAIL;
	}

	return MPA_ERROR_NONE;

}
#endif

# undef MASK
# undef MASK1BIT
DWORD mpareg[128];

//DWORD ScalefacMb[240];
//DWORD SectsfbMb[240];
//DWORD SfbcbMb[120];
//DWORD SideinfoMb[333];
//DWORD DiSwinMb[256];

void save_mpa_regs()
{
	int i;
	MPA *mpa;

	mpa = (MPA *) MPA_BASE;
	for (i = 0; i < 128; i++)
		mpareg[i] = (DWORD) (*((DWORD *) MPA_BASE + i));
#if 0
	{
		mpa->MbRst = 1;
		for (i = 0; i < 240; i++)
			ScalefacMb[i] = mpa->ScalefacMb;

		mpa->MbRst = 1;
		for (i = 0; i < 240; i++)
			SectsfbMb[i] = mpa->SectsfbMb;

		mpa->MbRst = 1;
		for (i = 0; i < 120; i++)
			SfbcbMb[i] = mpa->SfbcbMb;

		mpa->MbRst = 1;
		for (i = 0; i < 333; i++)
			SideinfoMb[i] = mpa->SideinfoMb;

		mpa->MbRst = 1;
		for (i = 0; i < 256; i++)
			DiSwinMb[i] = mpa->DiSwinMb;
	}
#endif

}
void reload_mpa_regs()
{
	int i;
	DWORD temp;
	MPA *mpa;

	mpa = (MPA *) MPA_BASE;

	for (i = 0; i < 128; i++)
		(volatile DWORD) (*((volatile DWORD *) MPA_BASE + i)) = mpareg[i];
	//memcpy((char *)MPA_BASE,mpareg,sizeof(MPA));
#if 0
	mpa->MbRst = 1;
	for (i = 0; i < 240; i++)
		mpa->ScalefacMb = ScalefacMb[i];

	mpa->MbRst = 1;
	for (i = 0; i < 240; i++)
		mpa->SectsfbMb = SectsfbMb[i];

	mpa->MbRst = 1;
	for (i = 0; i < 120; i++)
		mpa->SfbcbMb = SfbcbMb[i];

	mpa->MbRst = 1;
	for (i = 0; i < 333; i++)
		mpa->SideinfoMb = SideinfoMb[i];

	mpa->MbRst = 1;
	for (i = 0; i < 256; i++)
		mpa->DiSwinMb = DiSwinMb[i];
#endif
}


void MPAInit(int audio_format)
{
	DMA *dmactl;
	MPA *mpa;
	int i, j, k = 0;
	CLOCK *clk;

//FixAdd 2005.06.01 Brenda, for mx612 chip use(FPGA don't use).
	clk = (CLOCK *) (CLOCK_BASE);
	mClrMpaCks();
	mClrIntsram0Cks();
	mClrIntsram1Cks();
	mClrIntsram2Cks();

	switch (Clock_CpuFreqGet())
	{
	case CLOCK_108M_PLL2CFG:
		mSetMpaCks(MPACKS_PLL2_DIV_2);
		break;
	case CLOCK_96M_PLL1CFG:
	case CLOCK_108M_PLL1CFG:
	default:
		mSetMpaCks(MPACKS_PLL1_DIV_2);
		break;
	}

	mSetIntsram0Cks(INTSRAM0CKS_MPA_CLK);
	mSetIntsram1Cks(INTSRAM1CKS_MPA_CLK);
	mSetIntsram2Cks(INTSRAM2CKS_CPU_CLK);
	//clk->Clkss2 &= 0x00fffffc;  
	//clk->Clkss2 |= 0x22000001;  //sel SRAM0/1 CLK as MPA,MPA=pll1/2

	clk->MdClken |= 0x35; 
//FixAdd 2005.06.01 Brenda, for mx612 chip use.

	mpa = (MPA *) MPA_BASE;

//    dmactl = (DMA*)DMA_BASE;
//    dmactl->DmaCtl |= 4;
	mpa->MpaCtrl = 0x100;
	reload_mpa_regs();
	mpa->MpaCtrl = 0x10;
	mpa->Bypass = 0x0;

	switch (audio_format)
	{
	case 0:					//mp3, mp1, mp2
		{
			mpa->MbRst = 1;
			for (i = 0; i < 4; i++)
				for (j = 0; j < 64; j++)
					mpa->DiSwinMb = di[i][j];
			/* Load into Internal SRAM using Uncached address space */
			for (i = 0xafc01000; i < 0xafc04000; i++)
			{
				*((unsigned char *) i) = 0;
			}

			mpa->MbRst = 1;
			for (i = 0; i < 3; i++)
				for (j = 0; j < 3; j++)
					for (k = 0; k < 23; k++)
						mpa->SideinfoMb = l_sfb_tab[i][j][k];
			for (i = 0; i < 3; i++)
				for (j = 0; j < 3; j++)
					for (k = 0; k < 14; k++)
						mpa->SideinfoMb = s_sfb_tab[i][j][k];
			break;
		}
	case 1:					//aac
		{

			for (i = 0xafc00000; i < 0xafc04000; i++)
			{
				*((unsigned char *) i) = 0;
			}

			/* Load into Internal SRAM using Uncached address space */
			for (i = 0; i < 512; i++)
			{
				*((BYTE *) (0xafc00000 + i * 4 + 3)) = sine_lw0_l[i];
				*((unsigned char *) (0xafc00000 + i * 4 + 2)) = sine_lw0_h[i];
				*((unsigned char *) (0xafc00000 + i * 4 + 1)) = kbd_lw0_l[i];
				*((unsigned char *) (0xafc00000 + i * 4)) = kbd_lw0_h[i];
			}
			/* Load into Internal SRAM using Uncached address space */
			for (i = 0; i < 512; i++)
			{
				*((unsigned char *) (0xafc00800 + i * 4 + 3)) = sine_lw1_l[i];
				*((unsigned char *) (0xafc00800 + i * 4 + 2)) = sine_lw1_h[i];
				*((unsigned char *) (0xafc00800 + i * 4 + 1)) = kbd_lw1_l[i];
				*((unsigned char *) (0xafc00800 + i * 4)) = kbd_lw1_h[i];
			}
			for (i = 0xafc01000; i < 0xafc04000; i++)
			{
				*((unsigned char *) i) = 0;
			}

			mpa->MbRst = 1;
			for (j = 0; j < 2; j++)
			{
				for (i = 0; i < 64; i++)
					mpa->DiSwinMb = STable[j][i];
			}
			for (j = 0; j < 2; j++)
			{
				for (i = 0; i < 64; i++)
					mpa->DiSwinMb = KBDTable[j][i];
			}

			break;
		}
	case 2:
		{
			mpa->MbRst = 1;
			for (i = 0; i < 4; i++)
				for (j = 0; j < 64; j++)
					mpa->DiSwinMb = di[i][j];

			/* Load into Internal SRAM using Uncached address space */
			for (i = 0xafc01000; i < 0xafc04000; i++)
			{
				*((unsigned char *) i) = 0;
			}

		}
	}
	//DpString("MPAInit()");
}
static inline MPAReset()
{
	MPA *mpa;

	mpa = (MPA *) MPA_BASE;
	mpa->MpaCtrl = MPA_CTRL_RESET;
//    IODelay(10);
//    mpa->MpaCtrl &= ~MPA_CTRL_RESET;
//    IODelay(10);
	mpa->MpaCtrl = MPA_CTRL_ENABLE;
}

//Brenda for debug
extern int n_frames;
extern BOOL boScalerBusy, boMPABusy;

//extern filter_graph_t filter_graph;
extern int Layer3_Inited;
extern BYTE video_flag;
static
enum mad_error III_decodempa(struct mad_bitptr *ptr, struct mad_frame *frame,
			  struct sideinfo *si, unsigned int nch, unsigned char *buf)
{
	struct mad_header *header = &frame->header;
	struct mp3_regs regs;

	unsigned int sfreq;
	unsigned int sfreqi, ngr, gr;
	int *pDmaOut0,*pDmaOut1;
	int ok=0;
	int tempp;
MP_TRACE_LINE();
	CHANNEL *wdma;//rebecca 6.5
	wdma = (CHANNEL *)DMA_MPAW_BASE;//rebecca 6.5
	sfreq = header->samplerate;
	if (header->flags & MAD_FLAG_MPEG_2_5_EXT)
		sfreq *= 2;

	/* 48000 => 0, 44100 => 1, 32000 => 2,
	24000 => 3, 22050 => 4, 16000 => 5 */
	sfreqi = ((sfreq >>  7) & 0x000f) + ((sfreq >> 15) & 0x0001) - 8;

	if (header->flags & MAD_FLAG_MPEG_2_5_EXT)
		sfreqi += 3;

	/* scalefactors, Huffman decoding, requantization */

	ngr = (header->flags & MAD_FLAG_LSF_EXT) ? 1 : 2;

	for (gr = 0; gr < ngr; gr++) {

		struct granule *granule = &si->gr[gr];
		unsigned char const *sfbwidth[2];
		unsigned short xr[2][576];
		//unsigned int xr[2][288];
		int         i, j, l;
	    	int ns,len=0;
		unsigned int         nsamples;

		unsigned int ch;
		enum mad_error error;
		MPA *mpa;
		mpa= (MPA*)MPA_BASE;
		//point to the memory bank a*******, which won't be cached!!!
		//maybe caused by the GNU gcc don't support lexra mips mmu method.
		//IntDisable();
		pDmaOut0 = (int*)(((long)(&frame->PcmSample[0][0]) & 0x1fffffffL) | 0xa0000000L);
		pDmaOut1 = (int*)(((long)(&frame->PcmSample[1][0]) & 0x1fffffffL) | 0xa0000000L);
		if(Layer3_Inited ==0){
			////rebecca 6.5 CHANNEL *wdma;
			////rebecca 6.5 wdma = (CHANNEL *)DMA_MPAW_BASE;
			wdma->Control = 0;
			wdma->StartA =(DWORD)pDmaOut0;
			wdma->EndA = (DWORD)((BYTE*)pDmaOut0+1152-1);
			if(nch == 2){
				wdma->StartB = (DWORD)pDmaOut1;
				wdma->EndB = (DWORD)((BYTE*)pDmaOut1+1152-1);
				wdma->Control = 0x400b;//not enable DMA end interrupt
			}
			else
				wdma->Control = 0x4003;//not enable DMA end interrupt
		}
		//IntEnable();
#if 1		
		if(Layer3_Inited==1){
			ns = MAD_NSBSAMPLES(&frame->header);
			nsamples  = 32 * ns/ngr;
			
			while(1){
				if(mpa->MpaStatus == MPA_SEQ_IDLE) break;
			}
			
			if(n_frames <10)
			{
				memset(buf,0,nch*nsamples*2);
			}else{
 				PcmOutput(nch,nsamples,pDmaOut0,pDmaOut1,buf);
			}
			buf+=2*nch*nsamples;
		}
#endif		
		while ((boScalerBusy == TRUE))    TaskYield();
		boMPABusy = TRUE;
		for (ch = 0; ch < nch; ch++){
			struct channel *channel = &granule->ch[ch];
			unsigned int part2_length;

			sfbwidth[ch] = sfbwidth_table[sfreqi].l;
			if (channel->block_type == 2)
				sfbwidth[ch] = (channel->flags & mixed_block_flag) ?
					sfbwidth_table[sfreqi].m : sfbwidth_table[sfreqi].s;

			if (header->flags & MAD_FLAG_LSF_EXT) 
				part2_length = III_scalefactors_lsf(ptr, channel,
					    ch == 0 ? 0 : &si->gr[1].ch[1],
					    header->mode_extension);
			else 
	        		part2_length = III_scalefactors(ptr, channel, &si->gr[0].ch[ch],
					gr == 0 ? 0 : si->scfsi[ch]);

			error = III_huffdecode(ptr, xr, channel, sfbwidth[ch], part2_length);
			if (error){
				MPAReset();
				boMPABusy = FALSE;
				return error;
			}
			extern DWORD pre_parse;
			
			tempp=header->flags & MAD_FLAG_I_STEREO;
			tempp=header->flags &  MAD_FLAG_MS_STEREO;
			tempp=ch;

			if( (header->flags & MAD_FLAG_I_STEREO) && ch == 1 && !pre_parse)
			{
				//rebecca 6.05 begin
				//ok = PollRegister32(mpa->MpaStatus, MPA_SEQ_CH0_WAIT, 0x0F, 10*1000);
				ok = PollReg(&mpa->MpaStatus, MPA_SEQ_CH0_WAIT, 0x0F, 10*1000);
				//while(1)  if (mpa->MpaStatus== MPA_SEQ_CH0_WAIT) break;
				if (!ok)
				{
					UartOutText("(MPA must in ch0_wait)\r\n");
					MPAInit(0);
					wdma->Control = 0;
					wdma->StartA =(DWORD)pDmaOut0;
					wdma->EndA = (DWORD)((BYTE*)pDmaOut0+1152-1);
					if(nch == 2){
						wdma->StartB = (DWORD)pDmaOut1;
						wdma->EndB = (DWORD)((BYTE*)pDmaOut1+1152-1);
						wdma->Control = 0x400b;//not enable DMA end interrupt
					}
					else
						wdma->Control = 0x4003;//not enable DMA end interrupt
					error=MAD_ERROR_DECODER_FAIL;
					boMPABusy = FALSE;
					return error;
				} //rebecca 6.05 end
				  //rebecca 6.05   while(1){if(mpa->MpaStatus == MPA_SEQ_CH0_WAIT) break;}
			}
			if(video_flag)IntDisable();
			//rebecca 6.13 begin
			error=fill_mp3_regs(*si,header,&regs,nch,xr,gr,ch);
			if(error==MAD_ERROR_DECODER_FAIL)
			{
				MPAInit(0);
				wdma->Control = 0;
				wdma->StartA =(DWORD)pDmaOut0;
				wdma->EndA = (DWORD)((BYTE*)pDmaOut0+1152-1);
				if(nch == 2){
					wdma->StartB = (DWORD)pDmaOut1;
					wdma->EndB = (DWORD)((BYTE*)pDmaOut1+1152-1);
					wdma->Control = 0x400b;//not enable DMA end interrupt
				}
				else
					wdma->Control = 0x4003;//not enable DMA end interrupt
				error=MAD_ERROR_DECODER_FAIL;
				boMPABusy = FALSE;
				return error;
			}
			//rebecca 6.13 end
			if(video_flag){
				IntEnable();
			}
		}
	}
	n_frames++;
	Layer3_Inited = 1;
	boMPABusy = FALSE;
	return MAD_ERROR_NONE;
}


extern volatile DWORD TickCount;
inline int PollReg( volatile void *arg1,DWORD arg2, DWORD mask, DWORD us)
{
//DWORD us, volatile void *arg1, DWORD arg2, DWORD arg3

      DWORD   start_time = 0;
      DWORD   current_time = 0;

    if (us == 0)
        return 0;   /* fail */

    us = (us+100000-1) / 100000;   /* In MP612, each timestamp is 100,000 us */
    start_time = TickCount;//SystemGetTimeStamp();
    while (current_time < us)
    {
       
            if ( (*(volatile DWORD *)arg1 & (DWORD)mask) == ((DWORD)arg2 & (DWORD)mask) )
                break;
       
          //  TaskYield();
      if (start_time > TickCount)
	
		 current_time= 0xffffffff - start_time + TickCount;
	
	else
	
		current_time= TickCount -start_time;
      //  current_time = SystemGetElapsedTime(start_time);
       
    	}
    if (current_time >= us)
    {
        return 0;   /* fail */
    }
    
    return 1;   /* success */



}
///@}
