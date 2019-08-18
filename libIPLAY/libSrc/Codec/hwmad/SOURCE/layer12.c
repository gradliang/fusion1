/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      layer2.c
*
* Programmer:    Brenda Li
*                MPX E120 division
*
* Created: 	 03/30/2005
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
#include "mpTrace.h"


# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# include "global.h"

# include "fixed.h"
# include "bit.h"
//# include "stream.h"
# include "frame.h"
# include "layer12.h"

# include "PcmOutput.h"
#include "../../include/codec.h"

/*!
   @defgroup	LAYERI&II 	LayerI&II
    Define Mpeg LayerI&II decoder's subfunctions.
   @{
*/

/*
 * scalefactor table
 * used in both Layer I and Layer II decoding
 */

static mad_fixed_t const sf_table[64] = {
# include "sf_table.dat"
};

void fill_mp12_regs(unsigned int *pSpec, int layer, short ch, short gr);

/* --- Layer I ------------------------------------------------------------- */

/* linear scaling table */
static mad_fixed_t const linear_table[14] = {
	MAD_F(0x15555555),			/* 2^2  / (2^2  - 1) == 1.33333333333333 */
	MAD_F(0x12492492),			/* 2^3  / (2^3  - 1) == 1.14285714285714 */
	MAD_F(0x11111111),			/* 2^4  / (2^4  - 1) == 1.06666666666667 */
	MAD_F(0x10842108),			/* 2^5  / (2^5  - 1) == 1.03225806451613 */
	MAD_F(0x10410410),			/* 2^6  / (2^6  - 1) == 1.01587301587302 */
	MAD_F(0x10204081),			/* 2^7  / (2^7  - 1) == 1.00787401574803 */
	MAD_F(0x10101010),			/* 2^8  / (2^8  - 1) == 1.00392156862745 */
	MAD_F(0x10080402),			/* 2^9  / (2^9  - 1) == 1.00195694716243 */
	MAD_F(0x10040100),			/* 2^10 / (2^10 - 1) == 1.00097751710655 */
	MAD_F(0x10020040),			/* 2^11 / (2^11 - 1) == 1.00048851978505 */
	MAD_F(0x10010010),			/* 2^12 / (2^12 - 1) == 1.00024420024420 */
	MAD_F(0x10008004),			/* 2^13 / (2^13 - 1) == 1.00012208521548 */
	MAD_F(0x10004001),			/* 2^14 / (2^14 - 1) == 1.00006103888177 */
	MAD_F(0x10002000)			/* 2^15 / (2^15 - 1) == 1.00003051850948 */
};

///
///@ingroup LAYERI&II
///@brief   decode one requantized Layer I sample from a bitstream
///
///@param   struct mad_bitptr *ptr      The pointer to bitstream
///@param   unsigned int nb         number of bits of the compressed sample
///
///@return  mad_fix_t               the decoded sample 
///
static mad_fixed_t I_sample(struct mad_bitptr *ptr, unsigned int nb)
{
	mad_fixed_t sample;

	sample = mad_bit_read(ptr, nb);

	/* invert most significant bit, extend sign, then scale to fixed format */

	sample ^= 1 << (nb - 1);
	sample |= -(sample & (1 << (nb - 1)));

	sample <<= MAD_F_FRACBITS - (nb - 1);

	/* requantize the sample */

	/* s'' = (2^nb / (2^nb - 1)) * (s''' + 2^(-nb + 1)) */

	sample += MAD_F_ONE >> (nb - 1);

	return mad_f_mul(sample, linear_table[nb - 2]);

	/* s' = factor * s'' */
	/* (to be performed by caller) */
}
volatile unsigned int DmaInBuf[1024] = { 0 * 1024 };

///
///@ingroup LAYERI&II
///@brief   arrange the input data in certain order according to the need of hardware
///         and then the data will be sent to store in the SRAM through the DMA channel
///@param   unsigned int * pDmaInBuf        a pointer to a buffer for arranged data
///@param   unsigned short* pSpec           a pointer to the input data         
///@param   int layer                       Layer I, 1;Layer II, 2.         
///@param   short nch                       number of channels          
///
///@return  NULL
///

inline void ChPaireImerge_I(unsigned int *pDmaInBuf, unsigned short *pSpec, short nch)
{
	unsigned short *left_ch, *right_ch;
	int x, y;

	left_ch = pSpec;

	if (nch == 2)
	{
		right_ch = left_ch + 384;	//12*32;//384

		for (x = 0; x < 32; x++)
			for (y = 0; y < 12; y++)
			{
				pDmaInBuf[x * 32 + y] = (right_ch[x * 12 + y] << 16) + left_ch[x * 12 + y];
			}
	}
	else
	{
		for (x = 0; x < 32; x++)
			for (y = 0; y < 12; y++)
			{
				pDmaInBuf[x * 32 + y] = left_ch[x * 12 + y];
			}
	}
}

inline void ChPaireImerge_II(unsigned int *pDmaInBuf, unsigned short *pSpec, short nch)
{
	unsigned short *left_ch, *right_ch;
	int x, y;

	left_ch = pSpec;
	if (nch == 2)
	{
		right_ch = left_ch + 576;	//18*32;//576

		for (x = 0; x < 32; x++)
			for (y = 0; y < 18; y++)
			{
				DmaInBuf[x * 32 + y] = (right_ch[x * 18 + y] << 16) + left_ch[x * 18 + y];
			}
	}
	else
	{
		for (x = 0; x < 32; x++)
			for (y = 0; y < 18; y++)
			{
				DmaInBuf[x * 32 + y] = left_ch[x * 18 + y];
			}
	}
}

extern int n_frames;

int Layer1_Inited = 0;
int Layer2_Inited = 0;

/*
 * NAME:	layer->I()
 * DESCRIPTION:	decode a single Layer I frame
 */
///
///@ingroup LAYERI&II
///@brief   decode a single Layer I frame
///
///@param   struct mad_stream *stream       pointer to the bitstream
///@param   struct mad_frame *frame     pointer to a frame structer which is used to stored info                
///@param   unsigned char *buf              pointer to the decoded PCM data buffer      
///
///@return  int                         indicate whether this frame is decoded successfully
///
int mad_layer_I(struct mad_stream *stream, struct mad_frame *frame, unsigned char *buf)
{
	struct mad_header *header = &frame->header;
	unsigned int nch, bound, ch, s, sb, nb;
	unsigned char allocation[2][32], scalefactor[2][32];
	int *pDmaOut0, *pDmaOut1;
	unsigned int *pDmaInbuf;
	short gr=0, x, y;
	short is[2][384];
	int ok;
	MPA *mpa = (MPA *) MPA_BASE;

	nch = MAD_NCHANNELS(header);

	bound = 32;
	if (header->mode == MAD_MODE_JOINT_STEREO)
	{
		header->flags |= MAD_FLAG_I_STEREO;
		bound = 4 + header->mode_extension * 4;
	}

	/* check CRC word */

	if (header->flags & MAD_FLAG_PROTECTION)
	{
		header->crc_check =
			mad_bit_crc2(stream->ptr, 4 * (bound * nch + (32 - bound)), header->crc_check);

		if (header->crc_check != header->crc_target && !(frame->options & MAD_OPTION_IGNORECRC))
		{
			stream->error = MAD_ERROR_BADCRC;
			return -1;
		}
	}

	/* decode bit allocations */

	for (sb = 0; sb < bound; ++sb)
	{
		for (ch = 0; ch < nch; ++ch)
		{
			nb = mad_bit_read(&stream->ptr, 4);

			if (nb == 15)
			{
				stream->error = MAD_ERROR_BADBITALLOC;
				return -1;
			}

			allocation[ch][sb] = nb ? nb + 1 : 0;
		}
	}

	for (sb = bound; sb < 32; ++sb)
	{
		nb = mad_bit_read(&stream->ptr, 4);

		if (nb == 15)
		{
			stream->error = MAD_ERROR_BADBITALLOC;
			return -1;
		}

		allocation[0][sb] = allocation[1][sb] = nb ? nb + 1 : 0;
	}

	/* decode scalefactors */

	for (sb = 0; sb < 32; ++sb)
	{
		for (ch = 0; ch < nch; ++ch)
		{
			if (allocation[ch][sb])
			{
				scalefactor[ch][sb] = mad_bit_read(&stream->ptr, 6);

# if defined(OPT_STRICT)
				/*
				 * Scalefactor index 63 does not appear in Table B.1 of
				 * ISO/IEC 11172-3. Nonetheless, other implementations accept it,
				 * so we only reject it if OPT_STRICT is defined.
				 */
				if (scalefactor[ch][sb] == 63)
				{
					stream->error = MAD_ERROR_BADSCALEFACTOR;
					return -1;
				}
# endif
			}
		}
	}

	/* decode samples */

	for (s = 0; s < 12; ++s)
	{
		for (sb = 0; sb < bound; ++sb)
		{
			for (ch = 0; ch < nch; ++ch)
			{
				nb = allocation[ch][sb];
				frame->sbsample[ch][s][sb] = nb ?
					mad_f_mul(I_sample(&stream->ptr, nb), sf_table[scalefactor[ch][sb]]) : 0;
			}
		}

		for (sb = bound; sb < 32; ++sb)
		{
			if ((nb = allocation[0][sb]))
			{
				mad_fixed_t sample;

				sample = I_sample(&stream->ptr, nb);

				for (ch = 0; ch < nch; ++ch)
				{
					frame->sbsample[ch][s][sb] = mad_f_mul(sample, sf_table[scalefactor[ch][sb]]);
				}
			}
			else
			{
				for (ch = 0; ch < nch; ++ch)
					frame->sbsample[ch][s][sb] = 0;
			}
		}
	}
	//Gregxu 2005.5.20
	pDmaOut0 = (int *) (((long) (&frame->PcmSample[0][0]) & 0x1fffffffL) | 0xa0000000L);
	pDmaOut1 = (int *) (((long) (&frame->PcmSample[1][0]) & 0x1fffffffL) | 0xa0000000L);
	pDmaInbuf = (unsigned int *) (((long) (&DmaInBuf[0]) & 0x1fffffffL) | 0xa0000000L);
	if (Layer1_Inited == 0)
	{
		CHANNEL *wdma = (CHANNEL *) DMA_MPAW_BASE;

		wdma->Control = 0;
		wdma->StartA = (DWORD) pDmaOut0;
		wdma->EndA = (DWORD) ((BYTE *) pDmaOut0 + 768 - 1);
		if (nch == 2)
		{
			wdma->StartB = (DWORD) pDmaOut1;
			wdma->EndB = (DWORD) ((BYTE *) pDmaOut1 + 768 - 1);
			wdma->Control = 0x400b;
		}
		else
			wdma->Control = 0x4003;
	}
	if (Layer1_Inited == 1)
	{
		ok = PollRegister32(mpa->MpaStatus, MPA_SEQ_IDLE, 0x0000000F, 10 * 1000);
		if (!ok)
			return -1;
//      buf = PcmOutput(nch,384,pDmaOut0,pDmaOut1,buf);
		if (n_frames < 10)
		{
			memset(buf, 0, nch * 384 * 2);
		}
		else
		{
			PcmOutput(nch, 384, pDmaOut0, pDmaOut1, buf);
		}
		buf += 768 * nch;
	}

	for (ch = 0; ch < nch; ch++)
	{
		for (y = 0; y < 12; y++)
			for (x = 0; x < 32; x++)
				is[ch][y + x * 12] = (short) (frame->sbsample[ch][gr * 12 + y][x] >> 14);
	}
	ChPaireImerge_I(pDmaInbuf, (unsigned short *) is, nch);
	IntDisable();
	fill_mp12_regs(pDmaInbuf, MAD_LAYER_I, nch, gr);
	IntEnable();
	n_frames++;
	Layer1_Inited = 1;
	return 0;
}

/* --- Layer II ------------------------------------------------------------ */

/* possible quantization per subband table */
static struct
{
	unsigned int sblimit;
	unsigned char const offsets[30];
} const sbquant_table[5] = {
	/* ISO/IEC 11172-3 Table B.2a */
	{27, {7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 3, 3, 3, 3, 3,	/* 0 */
		  3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0}},
	/* ISO/IEC 11172-3 Table B.2b */
	{30, {7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 3, 3, 3, 3, 3,	/* 1 */
		  3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0}},
	/* ISO/IEC 11172-3 Table B.2c */
	{8, {5, 5, 2, 2, 2, 2, 2, 2}},	/* 2 */
	/* ISO/IEC 11172-3 Table B.2d */
	{12, {5, 5, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}},	/* 3 */
	/* ISO/IEC 13818-3 Table B.1 */
	{30, {4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1,	/* 4 */
		  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}}
};

/* bit allocation table */
static struct
{
	unsigned short nbal;
	unsigned short offset;
} const bitalloc_table[8] = {
	{2, 0},						/* 0 */
	{2, 3},						/* 1 */
	{3, 3},						/* 2 */
	{3, 1},						/* 3 */
	{4, 2},						/* 4 */
	{4, 3},						/* 5 */
	{4, 4},						/* 6 */
	{4, 5}						/* 7 */
};

/* offsets into quantization class table */
static unsigned char const offset_table[6][15] = {
	{0, 1, 16},					/* 0 */
	{0, 1, 2, 3, 4, 5, 16},		/* 1 */
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14},	/* 2 */
	{0, 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},	/* 3 */
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 16},	/* 4 */
	{0, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}	/* 5 */
};

/* quantization class table */
static struct quantclass
{
	unsigned short nlevels;
	unsigned char group;
	unsigned char bits;
	mad_fixed_t C;
	mad_fixed_t D;
} const qc_table[17] = {
# include "qc_table.dat"
};

///
///@ingroup LAYERI&II
///@brief   decode one requantized Layer II sample from a bitstream
///
///@param   struct mad_bitptr *ptr              The pointer to bitstream
///@param   struct quantclass const *quantclass     A pointer point to the quantization class table         
///
///@return  mad_fix_t output[3]                     the samples 
///
static
	void II_samples(struct mad_bitptr *ptr,
					struct quantclass const *quantclass, mad_fixed_t output[3])
{
	unsigned int nb, s, sample[3];

	if ((nb = quantclass->group))
	{
		unsigned int c, nlevels;

		/* degrouping */
		c = mad_bit_read(ptr, quantclass->bits);
		nlevels = quantclass->nlevels;

		for (s = 0; s < 3; ++s)
		{
			sample[s] = c % nlevels;
			c /= nlevels;
		}
	}
	else
	{
		nb = quantclass->bits;

		for (s = 0; s < 3; ++s)
			sample[s] = mad_bit_read(ptr, nb);
	}

	for (s = 0; s < 3; ++s)
	{
		mad_fixed_t requantized;

		/* invert most significant bit, extend sign, then scale to fixed format */

		requantized = sample[s] ^ (1 << (nb - 1));
		requantized |= -(requantized & (1 << (nb - 1)));

		requantized <<= MAD_F_FRACBITS - (nb - 1);

		/* requantize the sample */

		/* s'' = C * (s''' + D) */

		output[s] = mad_f_mul(requantized + quantclass->D, quantclass->C);

		/* s' = factor * s'' */
		/* (to be performed by caller) */
	}
}
static
	void II_samples_force_mono(struct mad_bitptr *ptr,
							   struct quantclass const *quantclass, mad_fixed_t output[3])
{
	unsigned int nb, s, sample[3];

	if ((nb = quantclass->group))
	{
		unsigned int c, nlevels;

		/* degrouping */
		c = mad_bit_read(ptr, quantclass->bits);
	}
	else
	{
		nb = quantclass->bits;

		for (s = 0; s < 3; ++s)
			sample[s] = mad_bit_read(ptr, nb);
	}
}
void fill_mp12_regs(unsigned int *pSpec, int layer, short nch, short gr)
{
	int i;
	MPA *mpa = (MPA *) MPA_BASE;
	CHANNEL *rdma;
	SRMGP *srm = (SRMGP *) SRMGP_BASE;

//FixAdd 2005.06.01 Brenda, for mx612 chip use(FPGA don't use).
	CLOCK *clk;

	clk = (CLOCK *) (CLOCK_BASE);
	mSetIntsram0Cks(INTSRAM0CKS_MEM_CLK);
	mSetIntsram1Cks(INTSRAM1CKS_MEM_CLK);
	mSetIntsram1Cks(INTSRAM2CKS_CPU_CLK);
	//clk->Clkss2 &= 0x00ffffff; //sel SRAM0/1 CLK as Memory;  
//FixAdd 2005.06.01 Brenda, for mx612 chip use.
	//Feed the layerI and layerII dequant data into the SRAM
	mpa->Bypass = 0x1f;			//Imdct;Tns;Pns;Stdec;Dequnt
	mpa->Op_Mode = layer;
#ifndef LAYERII_FORCE_MONO
	if (nch == 1)
		mpa->Mono = 1;
	else
		mpa->Mono = 0;
#else
	mpa->Mono = 1;
#endif
	//pol dma finishing


	rdma = (CHANNEL *) DMA_INTSRAM1_BASE;
	rdma->Control = 0;
	rdma->StartA = (DWORD) pSpec;
	rdma->EndA = (DWORD) ((BYTE *) pSpec + 1024 * 4);
	rdma->Control = 1;

	srm->StartAddr1 = 0x400;
	srm->EndAddr1 = 0x7ff;
	srm->DMACfg = 0x00730000;

#if 1
	while (1)
	{
		if (srm->DMACfg == 0x00720000)
		{
			break;
		}
	}
#else
	TaskYield();
#endif
//FixAdd 2005.06.01 Brenda, for mx612 chip use(FPGA don't use).
	mSetIntsram0Cks(INTSRAM0CKS_MPA_CLK);
	mSetIntsram1Cks(INTSRAM1CKS_MPA_CLK);
	//clk->Clkss2 |= 0x22000000;//sel SRAM0/1 CLK as MPA;
//FixAdd 2005.06.01 Brenda, for mx612 chip use(FPGA don't use).
	//Enable Mpa
	mpa->MpaCtrl = 0x11;

}

///
///@ingroup LAYERI&II
///@brief   decode a single Layer II frame
///
///@param   struct mad_stream *stream       pointer to the bitstream
///@param   struct mad_frame *frame     pointer to a frame structer which is used to stored info                
///@param   unsigned char *buf              pointer to the decoded PCM data buffer      
///
///@return  int                         indicate whether this frame is decoded successfully
///
#if 0
int mad_layer_II(struct mad_stream *stream, struct mad_frame *frame, unsigned char *buf)
{
	struct mad_header *header = &frame->header;
	struct mad_bitptr start;
	unsigned int index, sblimit, nbal, bound, s, sb;
	unsigned char const *offsets;
	unsigned char allocation[2][32], scfsi[2][32], scalefactor[2][32][3];
	mad_fixed_t samples[3];
	int ns;
	short x, y, nch, gr, ch;
	short is[2][576];
	int *pcm_ptr;
	short grmax;
	int *pDmaOut0, *pDmaOut1;
	unsigned int *pDmaInbuf;
	MPA *mpa = (MPA *) MPA_BASE;

	grmax = (frame->header.layer == MAD_LAYER_I ? 1 :
			 ((frame->header.layer == MAD_LAYER_III &&
			   (frame->header.flags & MAD_FLAG_LSF_EXT)) ? 1 : 2));

	nch = MAD_NCHANNELS(header);

	if (header->flags & MAD_FLAG_LSF_EXT)
		index = 4;
	else if (header->flags & MAD_FLAG_FREEFORMAT)
		goto freeformat;
	else
	{
		unsigned long bitrate_per_channel;

		bitrate_per_channel = header->bitrate;
		if (nch == 2)
		{
			bitrate_per_channel >>= 1;

# if defined(OPT_STRICT)
			/*
			 * ISO/IEC 11172-3 allows only single channel mode for 32, 48, 56, and
			 * 80 kbps bitrates in Layer II, but some encoders ignore this
			 * restriction. We enforce it if OPT_STRICT is defined.
			 */
			if (bitrate_per_channel <= 28000 || bitrate_per_channel == 40000)
			{
				stream->error = MAD_ERROR_BADMODE;
				return -1;
			}
# endif
		}
		else
		{						/* nch == 1 */
			if (bitrate_per_channel > 192000)
			{
				/*
				 * ISO/IEC 11172-3 does not allow single channel mode for 224, 256,
				 * 320, or 384 kbps bitrates in Layer II.
				 */
				stream->error = MAD_ERROR_BADMODE;
				return -1;
			}
		}

		if (bitrate_per_channel <= 48000)
			index = (header->samplerate == 32000) ? 3 : 2;
		else if (bitrate_per_channel <= 80000)
			index = 0;
		else
		{
		  freeformat:
			index = (header->samplerate == 48000) ? 0 : 1;
		}
	}

	sblimit = sbquant_table[index].sblimit;
	offsets = sbquant_table[index].offsets;

	bound = 32;
	if (header->mode == MAD_MODE_JOINT_STEREO)
	{
		header->flags |= MAD_FLAG_I_STEREO;
		bound = 4 + header->mode_extension * 4;
	}

	if (bound > sblimit)
		bound = sblimit;

	start = stream->ptr;

	/* decode bit allocations */

	for (sb = 0; sb < bound; ++sb)
	{
		nbal = bitalloc_table[offsets[sb]].nbal;

		for (ch = 0; ch < nch; ++ch)
			allocation[ch][sb] = mad_bit_read(&stream->ptr, nbal);
	}

	for (sb = bound; sb < sblimit; ++sb)
	{
		nbal = bitalloc_table[offsets[sb]].nbal;

		allocation[0][sb] = allocation[1][sb] = mad_bit_read(&stream->ptr, nbal);
	}

	/* decode scalefactor selection info */

	for (sb = 0; sb < sblimit; ++sb)
	{
		for (ch = 0; ch < nch; ++ch)
		{
			if (allocation[ch][sb])
				scfsi[ch][sb] = mad_bit_read(&stream->ptr, 2);
		}
	}

	/* check CRC word */

	if (header->flags & MAD_FLAG_PROTECTION)
	{
		header->crc_check =
			mad_bit_crc2(start, mad_bit_length(&start, &stream->ptr), header->crc_check);

		if (header->crc_check != header->crc_target && !(frame->options & MAD_OPTION_IGNORECRC))
		{
			stream->error = MAD_ERROR_BADCRC;
			return -1;
		}
	}

	/* decode scalefactors */

	for (sb = 0; sb < sblimit; ++sb)
	{
		for (ch = 0; ch < nch; ++ch)
		{
			if (allocation[ch][sb])
			{
				scalefactor[ch][sb][0] = mad_bit_read(&stream->ptr, 6);

				switch (scfsi[ch][sb])
				{
				case 2:
					scalefactor[ch][sb][2] = scalefactor[ch][sb][1] = scalefactor[ch][sb][0];
					break;

				case 0:
					scalefactor[ch][sb][1] = mad_bit_read(&stream->ptr, 6);
					/* fall through */

				case 1:
				case 3:
					scalefactor[ch][sb][2] = mad_bit_read(&stream->ptr, 6);
				}

				if (scfsi[ch][sb] & 1)
					scalefactor[ch][sb][1] = scalefactor[ch][sb][scfsi[ch][sb] - 1];

# if defined(OPT_STRICT)
				/*
				 * Scalefactor index 63 does not appear in Table B.1 of
				 * ISO/IEC 11172-3. Nonetheless, other implementations accept it,
				 * so we only reject it if OPT_STRICT is defined.
				 */
				if (scalefactor[ch][sb][0] == 63 ||
					scalefactor[ch][sb][1] == 63 || scalefactor[ch][sb][2] == 63)
				{
					stream->error = MAD_ERROR_BADSCALEFACTOR;
					return -1;
				}
# endif
			}
		}
	}

	/* decode samples */

	for (gr = 0; gr < 12; ++gr)
	{
#ifdef LAYERII_FORCE_MONO
		for (sb = 0; sb < bound; ++sb)
		{
			for (ch = 0; ch < nch; ++ch)
			{
				if ((index = allocation[ch][sb]))
				{
					index = offset_table[bitalloc_table[offsets[sb]].offset][index - 1];

					if (ch == 0)
					{
						II_samples(&stream->ptr, &qc_table[index], samples);
						for (s = 0; s < 3; ++s)
						{
							frame->sbsample[ch][3 * gr + s][sb] =
								mad_f_mul(samples[s], sf_table[scalefactor[ch][sb][gr / 4]]);
						}
					}
					else
					{
						II_samples_force_mono(&stream->ptr, &qc_table[index], samples);
					}
				}
				else
				{
					if (ch == 0)
						for (s = 0; s < 3; ++s)
							frame->sbsample[ch][3 * gr + s][sb] = 0;
				}
			}
		}

#else
		for (sb = 0; sb < bound; ++sb)
		{
			for (ch = 0; ch < nch; ++ch)
			{
				if ((index = allocation[ch][sb]))
				{
					index = offset_table[bitalloc_table[offsets[sb]].offset][index - 1];

					II_samples(&stream->ptr, &qc_table[index], samples);

					for (s = 0; s < 3; ++s)
					{
						frame->sbsample[ch][3 * gr + s][sb] =
							mad_f_mul(samples[s], sf_table[scalefactor[ch][sb][gr / 4]]);
					}
				}
				else
				{
					for (s = 0; s < 3; ++s)
						frame->sbsample[ch][3 * gr + s][sb] = 0;
				}
			}
		}
#endif
		for (sb = bound; sb < sblimit; ++sb)
		{
			if ((index = allocation[0][sb]))
			{
				index = offset_table[bitalloc_table[offsets[sb]].offset][index - 1];

				II_samples(&stream->ptr, &qc_table[index], samples);
#ifdef LAYERII_FORCE_MONO
				for (ch = 0; ch < 1; ++ch)
#else
				for (ch = 0; ch < nch; ++ch)
#endif
				{
					for (s = 0; s < 3; ++s)
					{
						frame->sbsample[ch][3 * gr + s][sb] =
							mad_f_mul(samples[s], sf_table[scalefactor[ch][sb][gr / 4]]);
					}
				}
			}
			else
			{
#ifdef LAYERII_FORCE_MONO
				for (ch = 0; ch < 1; ++ch)
#else
				for (ch = 0; ch < nch; ++ch)
#endif
				{
					for (s = 0; s < 3; ++s)
						frame->sbsample[ch][3 * gr + s][sb] = 0;
				}
			}
		}

#ifdef LAYERII_FORCE_MONO
		for (ch = 0; ch < 1; ++ch)
#else
		for (ch = 0; ch < nch; ++ch)
#endif
		{
			for (s = 0; s < 3; ++s)
			{
				for (sb = sblimit; sb < 32; ++sb)
					frame->sbsample[ch][3 * gr + s][sb] = 0;
			}
		}
	}

	//Gregxu 2005.5.20
	pDmaOut0 = (int *) (((long) (&frame->PcmSample[0][0]) & 0x1fffffffL) | 0xa0000000L);
	pDmaOut1 = (int *) (((long) (&frame->PcmSample[1][0]) & 0x1fffffffL) | 0xa0000000L);
	pDmaInbuf = (unsigned int *) (((long) (&DmaInBuf[0]) & 0x1fffffffL) | 0xa0000000L);
	gr = 0;
	for (; gr < grmax; gr++)
	{
#ifdef LAYERII_FORCE_MONO
		if (Layer2_Inited == 0)
		{
			CHANNEL *wdma = (CHANNEL *) DMA_MPAW_BASE;

			wdma->Control = 0;
			wdma->StartA = (DWORD) pDmaOut0;
			wdma->EndA = (DWORD) ((BYTE *) pDmaOut0 + 1152 - 1);
			wdma->Control = 0x4003;	//not enable DMA end interrupt
		}
		if (Layer2_Inited == 1)
		{
			while (1)
			{
				if (mpa->MpaStatus == MPA_SEQ_IDLE)
					break;
			}
			PcmOutput(1, 576, pDmaOut0, pDmaOut1, buf);
			buf += 1152;
		}
		for (y = 0; y < 18; y++)
			for (x = 0; x < 32; x++)
				pDmaInbuf[x * 32 + y] = (unsigned int) (frame->sbsample[0][gr * 18 + y][x] >> 14);
		fill_mp12_regs(pDmaInbuf, MAD_LAYER_II, 1, gr);
#else
		if (Layer2_Inited == 0)
		{
			CHANNEL *wdma = (CHANNEL *) DMA_MPAW_BASE;

			wdma->Control = 0;
			wdma->StartA = (DWORD) pDmaOut0;
			wdma->EndA = (DWORD) ((BYTE *) pDmaOut0 + 1152 - 1);
			if (nch == 2)
			{
				wdma->StartB = (DWORD) pDmaOut1;
				wdma->EndB = (DWORD) ((BYTE *) pDmaOut1 + 1152 - 1);
				wdma->Control = 0x400b;	//not enable DMA end interrupt
			}
			else
				wdma->Control = 0x4003;	//not enable DMA end interrupt
		}
		if (Layer2_Inited == 1)
		{
			while (1)
			{
				if (mpa->MpaStatus == MPA_SEQ_IDLE)
					break;
			}
			buf = PcmOutput(nch, 576, pDmaOut0, pDmaOut1, buf);
			if (n_frames < 100)
			{
				memset(buf, 0, nch * 576 * 2);
			}
			else
			{
				PcmOutput(nch, 576, pDmaOut0, pDmaOut1, buf);
			}
			buf += 1152 * nch;
		}
		for (ch = 0; ch < nch; ch++)
		{
			for (y = 0; y < 18; y++)
				for (x = 0; x < 32; x++)
					is[ch][y + x * 18] = (short) (frame->sbsample[ch][gr * 18 + y][x] >> 14);
		}
		ChPaireImerge_II(pDmaInbuf, (unsigned short *) is, nch);
		IntDisable();
		fill_mp12_regs(pDmaInbuf, MAD_LAYER_II, nch, gr);
		IntEnable();
#endif
	}
	n_frames++;
	Layer2_Inited = 1;

	return 0;
}

///@}

#else
//For LAYERII_FORCE_MONO case only
int mad_layer_II(struct mad_stream *stream, struct mad_frame *frame, unsigned char *buf)
{
	struct mad_header *header = &frame->header;
	struct mad_bitptr start;
	unsigned int index, sblimit, nbal, bound, s, sb;
	unsigned char const *offsets;
	unsigned char allocation[2][32], scfsi[2][32], scalefactor[2][32][3];
	mad_fixed_t samples[3];
	int ns;
	short x, y, nch, gr, ch;
	short is[2][576];
	int *pcm_ptr;
	short grmax;
	int *pDmaOut0, *pDmaOut1;
	unsigned int *pDmaInbuf;
	MPA *mpa = (MPA *) MPA_BASE;

	grmax = (frame->header.layer == MAD_LAYER_I ? 1 :
			 ((frame->header.layer == MAD_LAYER_III &&
			   (frame->header.flags & MAD_FLAG_LSF_EXT)) ? 1 : 2));

	nch = MAD_NCHANNELS(header);

	if (header->flags & MAD_FLAG_LSF_EXT)
		index = 4;
	else if (header->flags & MAD_FLAG_FREEFORMAT)
		goto freeformat;
	else
	{
		unsigned long bitrate_per_channel;

		bitrate_per_channel = header->bitrate;
		if (nch == 2)
		{
			bitrate_per_channel >>= 1;
		}
		else
		{						/* nch == 1 */
			if (bitrate_per_channel > 192000)
			{
				/*
				 * ISO/IEC 11172-3 does not allow single channel mode for 224, 256,
				 * 320, or 384 kbps bitrates in Layer II.
				 */
				stream->error = MAD_ERROR_BADMODE;
				return -1;
			}
		}

		if (bitrate_per_channel <= 48000)
			index = (header->samplerate == 32000) ? 3 : 2;
		else if (bitrate_per_channel <= 80000)
			index = 0;
		else
		{
		  freeformat:
			index = (header->samplerate == 48000) ? 0 : 1;
		}
	}

	sblimit = sbquant_table[index].sblimit;
	offsets = sbquant_table[index].offsets;

	bound = 32;
	if (header->mode == MAD_MODE_JOINT_STEREO)
	{
		header->flags |= MAD_FLAG_I_STEREO;
		bound = 4 + header->mode_extension * 4;
	}

	if (bound > sblimit)
		bound = sblimit;

	start = stream->ptr;

	/* decode bit allocations */

	for (sb = 0; sb < bound; ++sb)
	{
		nbal = bitalloc_table[offsets[sb]].nbal;

		for (ch = 0; ch < nch; ++ch)
			allocation[ch][sb] = mad_bit_read(&stream->ptr, nbal);
	}

	for (sb = bound; sb < sblimit; ++sb)
	{
		nbal = bitalloc_table[offsets[sb]].nbal;

		allocation[0][sb] = allocation[1][sb] = mad_bit_read(&stream->ptr, nbal);
	}

	/* decode scalefactor selection info */

	for (sb = 0; sb < sblimit; ++sb)
	{
		for (ch = 0; ch < nch; ++ch)
		{
			if (allocation[ch][sb])
				scfsi[ch][sb] = mad_bit_read(&stream->ptr, 2);
		}
	}

	/* check CRC word */
#if 0
	if (header->flags & MAD_FLAG_PROTECTION)
	{
		header->crc_check =
			mad_bit_crc2(start, mad_bit_length(&start, &stream->ptr), header->crc_check);

		if (header->crc_check != header->crc_target && !(frame->options & MAD_OPTION_IGNORECRC))
		{
			stream->error = MAD_ERROR_BADCRC;
			return -1;
		}
	}
#endif
	/* decode scalefactors */

	for (sb = 0; sb < sblimit; ++sb)
	{
		for (ch = 0; ch < nch; ++ch)
		{
			if (allocation[ch][sb])
			{
				scalefactor[ch][sb][0] = mad_bit_read(&stream->ptr, 6);

				switch (scfsi[ch][sb])
				{
				case 2:
					scalefactor[ch][sb][2] = scalefactor[ch][sb][1] = scalefactor[ch][sb][0];
					break;

				case 0:
					scalefactor[ch][sb][1] = mad_bit_read(&stream->ptr, 6);
					/* fall through */

				case 1:
				case 3:
					scalefactor[ch][sb][2] = mad_bit_read(&stream->ptr, 6);
				}

				if (scfsi[ch][sb] & 1)
					scalefactor[ch][sb][1] = scalefactor[ch][sb][scfsi[ch][sb] - 1];
			}
		}
	}

	/* decode samples */

	for (gr = 0; gr < 12; ++gr)
	{
#ifdef LAYERII_FORCE_MONO
		for (sb = 0; sb < bound; ++sb)
		{
			for (ch = 0; ch < nch; ++ch)
			{
				if ((index = allocation[ch][sb]))
				{
					index = offset_table[bitalloc_table[offsets[sb]].offset][index - 1];

					if (ch == 0)
					{
						II_samples(&stream->ptr, &qc_table[index], samples);
						for (s = 0; s < 3; ++s)
						{
							frame->sbsample[ch][3 * gr + s][sb] =
								mad_f_mul(samples[s], sf_table[scalefactor[ch][sb][gr / 4]]);
						}
					}
					else
					{
						II_samples_force_mono(&stream->ptr, &qc_table[index], samples);
					}
				}
				else
				{
					if (ch == 0)
						for (s = 0; s < 3; ++s)
							frame->sbsample[ch][3 * gr + s][sb] = 0;
				}
			}
		}
#else
		for (sb = 0; sb < bound; ++sb)
		{
			for (ch = 0; ch < nch; ++ch)
			{
				if ((index = allocation[ch][sb]))
				{
					index = offset_table[bitalloc_table[offsets[sb]].offset][index - 1];

					II_samples(&stream->ptr, &qc_table[index], samples);

					for (s = 0; s < 3; ++s)
					{
						frame->sbsample[ch][3 * gr + s][sb] =
							mad_f_mul(samples[s], sf_table[scalefactor[ch][sb][gr / 4]]);
					}
				}
				else
				{
					for (s = 0; s < 3; ++s)
						frame->sbsample[ch][3 * gr + s][sb] = 0;
				}
			}
		}
#endif
		for (sb = bound; sb < sblimit; ++sb)
		{
			if ((index = allocation[0][sb]))
			{
				index = offset_table[bitalloc_table[offsets[sb]].offset][index - 1];

				II_samples(&stream->ptr, &qc_table[index], samples);

				{
					for (s = 0; s < 3; ++s)
					{
						frame->sbsample[0][3 * gr + s][sb] =
							mad_f_mul(samples[s], sf_table[scalefactor[0][sb][gr / 4]]);
					}
				}
			}
			else
			{
				for (s = 0; s < 3; ++s)
					frame->sbsample[0][3 * gr + s][sb] = 0;
			}
		}

		for (s = 0; s < 3; ++s)
		{
			for (sb = sblimit; sb < 32; ++sb)
				frame->sbsample[0][3 * gr + s][sb] = 0;
		}
	}

	//Gregxu 2005.5.20
	pDmaOut0 = (int *) (((long) (&frame->PcmSample[0][0]) & 0x1fffffffL) | 0xa0000000L);
	pDmaOut1 = (int *) (((long) (&frame->PcmSample[1][0]) & 0x1fffffffL) | 0xa0000000L);
	pDmaInbuf = (unsigned int *) (((long) (&DmaInBuf[0]) & 0x1fffffffL) | 0xa0000000L);
	gr = 0;
	for (; gr < grmax; gr++)
	{
#ifdef LAYERII_FORCE_MONO
		if (Layer2_Inited == 0)
		{
			CHANNEL *wdma = (CHANNEL *) DMA_MPAW_BASE;

			wdma->Control = 0;
			wdma->StartA = (DWORD) pDmaOut0;
			wdma->EndA = (DWORD) ((BYTE *) pDmaOut0 + 1152 - 1);
			wdma->Control = 0x4003;	//not enable DMA end interrupt
		}
		if (Layer2_Inited == 1)
		{
			while (1)
			{
				if (mpa->MpaStatus == MPA_SEQ_IDLE)
					break;
			}
			if (n_frames < 50)
			{
				memset(buf, 0, nch * 576 * 2);
			}
			else
			{
				PcmOutput(1, 576, pDmaOut0, pDmaOut1, buf);
			}
			buf += 1152;
		}
		for (y = 0; y < 18; y++)
			for (x = 0; x < 32; x++)
				pDmaInbuf[x * 32 + y] = (unsigned int) (frame->sbsample[0][gr * 18 + y][x] >> 14);
		fill_mp12_regs(pDmaInbuf, MAD_LAYER_II, 1, gr);
#else
		if (Layer2_Inited == 0)
		{
			CHANNEL *wdma = (CHANNEL *) DMA_MPAW_BASE;

			wdma->Control = 0;
			wdma->StartA = (DWORD) pDmaOut0;
			wdma->EndA = (DWORD) ((BYTE *) pDmaOut0 + 1152 - 1);
			if (nch == 2)
			{
				wdma->StartB = (DWORD) pDmaOut1;
				wdma->EndB = (DWORD) ((BYTE *) pDmaOut1 + 1152 - 1);
				wdma->Control = 0x400b;	//not enable DMA end interrupt
			}
			else
				wdma->Control = 0x4003;	//not enable DMA end interrupt
		}
		if (Layer2_Inited == 1)
		{
			while (1)
			{
				if (mpa->MpaStatus == MPA_SEQ_IDLE)
					break;
			}
			buf = PcmOutput(nch, 576, pDmaOut0, pDmaOut1, buf);
			if (n_frames < 50)
			{
				memset(buf, 0, nch * 576 * 2);
			}
			else
			{
				PcmOutput(nch, 576, pDmaOut0, pDmaOut1, buf);
			}
			buf += 1152 * nch;
		}
		for (ch = 0; ch < nch; ch++)
		{
			for (y = 0; y < 18; y++)
				for (x = 0; x < 32; x++)
					is[ch][y + x * 18] = (short) (frame->sbsample[ch][gr * 18 + y][x] >> 14);
		}
		ChPaireImerge_II(pDmaInbuf, (unsigned short *) is, nch);
		IntDisable();
		fill_mp12_regs(pDmaInbuf, MAD_LAYER_II, nch, gr);
		IntEnable();
#endif
	}
	n_frames++;
	Layer2_Inited = 1;

	return 0;
}

#endif
