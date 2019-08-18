//#include "stdafx_dec.h"
#include "typedef.h"
#include "cnst.h"
#include "cnst_vad.h"
#include "dtx_struct.h"
#include "amrnb_struct.h"
#include "amrnb_function.h"


void InitTable();

#define NB_PULSE  10  
#define NMAX 9

Word16 amrdec_mode;
Word16 amrdec_frame_type; 


/* alloc pointer define */

Word16 *f1, *f2;
Word16 *tmp, *tmp2;
Word16 *Ap1;
Word16 *Ap2;
Word16 *excf, *scaled_excf, *corr_v;
Word16 *y;
Word16 *parm, *syn;
Word16 *lsp_new, *lsp_mid; 
Word16 *A_t; 
Word16 *xn, *xn2, *code, *y1, *y2;
Word16 *ipos, *pos_max, *codvec;
Word16 *dn, *sign, *dn2, *dn_sign, *_sign;
Word16 *linear_signs;
Word16 *linear_codewords;
Word16 *scal_y2, *scaled_y1;
Word16 *frac_coeff, *exp_coeff;
Word16 *lspt;
Word16 *Ah, *Al, *Anh, *Anl;
Word16 *rc, *rLow, *rHigh;
Word32 *lf1, *lf2;
Word16 *inno_sav, *ps_poss;
Word16 *Ap3, *Ap4;
Word16 *lsf1, *lsf2, *wf1, *wf2, *lsf_p, *lsf_r1, *lsf_r2, *lsf1_q, *lsf2_q, *temp_r1, *temp_p, *lsf1_r, *lsf2_r;
Word16 *coeff, *coeff_lo, *exp_max;
Word16 *frac_en, *exp_en, *g_pitch_cand, *g_pitch_cind;
Word16 *rrv;
Word16 *level;
Word16 *gCoeff;
Word16 *mem_syn_save, *mem_w0_save, *mem_err_save, *T_op;

Speech_Decode_FrameState *Ptr_Speech_Decode_FrameState;

//The following is for encoder
/*
Word16 *scaled_signal;
Word32 *corr;
Word16 *h2;
Word32 *y32;
Word16 *lsp_new_q;
Word16 *lsp_mid_q;
Word16 *Aq_t;
Word16 *res, *res2;
Word16 *xn_sf0, *y2_sf0, *code_sf0, *h1_sf0;
Word16 (*rr) [L_CODE];
Speech_Encode_FrameState *Ptr_Speech_Encode_FrameState;
sid_syncState *Ptr_sid_syncState;

//Table
Word16 *trackTable;
Word16 *gamma1;
Word16 *gamma2; 
Word16 *startPos1;
Word16 *startPos2; 
Word16 *lag_h;
Word16 *lag_l;  
Word16 *inter_36_inter_6;  
Word16 *grid;    
Word16 *gamma1_12k2;    
Word16 *corrweight;
*/

//Honda : remove table scratch RAM
Word16 *dico1_lsf_3;
Word16 *dico2_lsf_3;
Word16 *mr515_3_lsf;
Word16 *table_gain_lowrates;
Word16 *log2_table;
Word16 *lsp_lsf_table;
Word16 *lsp_lsf_slope;
Word16 *pow2_table;
Word16 *lsp_init_data;
Word16 *sqrt_l_table;
Word16 *inv_sqrt_table;
Word16 *ph_imp_low;
Word16 *ph_imp_mid;


Word16 *pred_lt_inter_6;

Word16 *gray; 
Word16 *dgray;
Word16 *startPos;

                           
   
                            
Word16 *pred;                    
Word16 *table_gain_MR475;     
Word16 *window_200_40;    
 
Word16 *q_plsf_3_mean_lsf;
Word16 *pred_fac;
Word16 *past_rq_init;
Word16 *table_gain_highrates;
Word16 *dico3_lsf_3;

Word16 *qua_gain_pitch;
Word16 *qua_gain_code;
Word16 *ph_imp_low_MR795;
Word16 *ph_imp_mid_MR795;
Word16 *mr795_1_lsf;
Word16 *mean_lsf;
Word16 *dico1_lsf;
Word16 *dico2_lsf;
Word16 *dico3_lsf;
Word16 *dico4_lsf;
Word16 *dico5_lsf;

                                  
/* simulate sram buffer */
//#define poolsize 16*1024  
unsigned long sramsize;   
char *sram;


unsigned long sramaddr;

// size: number of byte,
// will be a 32bit/4byte alignment

unsigned long sram_alloc(unsigned long size)
{
	unsigned long alloc ,*addr ,alloc_end;
	alloc = sramaddr + (unsigned long) sram;
	sramaddr += size;
	if((sramaddr&3) != 0) sramaddr = ((sramaddr >> 2) +1 ) << 2;		// 32bit/ 4byte alignment
//Honda: Clear scratch RAM	
	alloc_end = sramaddr+ (unsigned long) sram;;
	for (addr= (unsigned long *)alloc; addr < (unsigned long *)alloc_end; addr++)
		*addr = 0;
	return alloc;
}

#if (!AMR_ENCODE_ENABLE)
void AMRDEC_init_sram(void)
{
	unsigned long size;
	sramaddr = 0; // init sram buffer start address to offset 0
	
	size = (M / 2 + 1) * sizeof(Word16);
	f1 = (Word16 *) sram_alloc(size);
	
	size = (M / 2 + 1) * sizeof(Word16);
	f2 = (Word16 *) sram_alloc(size);

	size = (80) * sizeof(Word16);
	tmp = (Word16 *) sram_alloc(size);

	size = (NMAX) * sizeof(Word16);
	tmp2 = (Word16 *) sram_alloc(size);

	size = (MP1) * sizeof(Word16);
	Ap1 = (Word16 *) sram_alloc(size);

	size = (MP1) * sizeof(Word16);
	Ap2 = (Word16 *) sram_alloc(size);

   size = (40) * sizeof(Word16);          //corr_v must be between Ap2 and y
	corr_v = (Word16 *) sram_alloc(size);

	size = (L_WINDOW) * sizeof(Word16);
	y = (Word16 *) sram_alloc(size);

	size = (MAX_PRM_SIZE + 1) * sizeof(Word16);
	parm = (Word16 *) sram_alloc(size);

	size = (L_FRAME) * sizeof(Word16);
	syn = (Word16 *) sram_alloc(size);

	size = (M) * sizeof(Word16);
	lsp_new = (Word16 *) sram_alloc(size);

	size = (M) * sizeof(Word16);
	lsp_mid = (Word16 *) sram_alloc(size);

	size = ((MP1) * 4) * sizeof(Word16);
	A_t = (Word16 *) sram_alloc(size);

	size = (L_SUBFR) * sizeof(Word16);
	xn = (Word16 *) sram_alloc(size);

	size = (L_SUBFR) * sizeof(Word16);
	xn2 = (Word16 *) sram_alloc(size);

	size = (L_SUBFR) * sizeof(Word16);
	code = (Word16 *) sram_alloc(size);

	size = (L_SUBFR) * sizeof(Word16);
	y1 = (Word16 *) sram_alloc(size);

	size = (L_SUBFR) * sizeof(Word16);
	y2 = (Word16 *) sram_alloc(size);

	size = (NB_PULSE) * sizeof(Word16);
	ipos = (Word16 *) sram_alloc(size);

	size = (NB_TRACK) * sizeof(Word16);
	pos_max = (Word16 *) sram_alloc(size);

	size = (NB_PULSE) * sizeof(Word16);
	codvec = (Word16 *) sram_alloc(size);

	size = (L_CODE) * sizeof(Word16);
	dn = (Word16 *) sram_alloc(size);

	size = (L_CODE) * sizeof(Word16);
	sign = (Word16 *) sram_alloc(size);

	size = (L_CODE) * sizeof(Word16);
	dn2 = (Word16 *) sram_alloc(size);

	size = (L_CODE) * sizeof(Word16);
	dn_sign = (Word16 *) sram_alloc(size);

	size = (NB_TRACK_MR102) * sizeof(Word16);
	linear_signs = (Word16 *) sram_alloc(size);

	size = (NB_PULSE) * sizeof(Word16);
	linear_codewords = (Word16 *) sram_alloc(size);

	size = (NB_PULSE) * sizeof(Word16);
	_sign = (Word16 *) sram_alloc(size);

	size = (5) * sizeof(Word16);
	frac_coeff = (Word16 *) sram_alloc(size);
	
	size = (5) * sizeof(Word16);
	exp_coeff = (Word16 *) sram_alloc(size);
	
	size = (M) * sizeof(Word16);
	lspt = (Word16 *) sram_alloc(size);

	size = (M+1) * sizeof(Word16);
	Ah = (Word16 *) sram_alloc(size);
	
	size = (M+1) * sizeof(Word16);
	Al = (Word16 *) sram_alloc(size);
	
	size = (M+1) * sizeof(Word16);
	Anh = (Word16 *) sram_alloc(size);
	
	size = (M+1) * sizeof(Word16);
	Anl = (Word16 *) sram_alloc(size);

	size = (4) * sizeof(Word16);
	rc = (Word16 *) sram_alloc(size);
	
	size = (MP1) * sizeof(Word16);
	rLow = (Word16 *) sram_alloc(size);
	
	size = (MP1) * sizeof(Word16);
	rHigh = (Word16 *) sram_alloc(size);

	size = (6) * sizeof(Word32);
	lf1 = (Word32 *) sram_alloc(size);
	
	size = (6) * sizeof(Word32);
	lf2 = (Word32 *) sram_alloc(size);

	size = (MP1) * sizeof(Word16);
	Ap3 = (Word16 *) sram_alloc(size);
	
	size = (MP1) * sizeof(Word16);
	Ap4 = (Word16 *) sram_alloc(size);

	size = (M) * sizeof(Word16);
	lsf1 = (Word16 *) sram_alloc(size);

	size = (M) * sizeof(Word16);
	lsf2 = (Word16 *) sram_alloc(size);

	size = (M) * sizeof(Word16);
	wf1 = (Word16 *) sram_alloc(size);

	size = (M) * sizeof(Word16);
	wf2 = (Word16 *) sram_alloc(size);

	size = (M) * sizeof(Word16);
	lsf_p = (Word16 *) sram_alloc(size);

	size = (M) * sizeof(Word16);
	lsf_r1 = (Word16 *) sram_alloc(size);

	size = (M) * sizeof(Word16);
	lsf_r2 = (Word16 *) sram_alloc(size);

	size = (M) * sizeof(Word16);
	lsf1_q = (Word16 *) sram_alloc(size);

	size = (M) * sizeof(Word16);
	lsf2_q = (Word16 *) sram_alloc(size);

	size = (M) * sizeof(Word16);
	temp_r1 = (Word16 *) sram_alloc(size);

	size = (M) * sizeof(Word16);
	temp_p = (Word16 *) sram_alloc(size);

	size = (M) * sizeof(Word16);
	lsf1_r = (Word16 *) sram_alloc(size);

	size = (M) * sizeof(Word16);
	lsf2_r = (Word16 *) sram_alloc(size);

	size = (10) * sizeof(Word16);
	coeff = (Word16 *) sram_alloc(size);

	size = (10) * sizeof(Word16);
	coeff_lo = (Word16 *) sram_alloc(size);

	size = (10) * sizeof(Word16);
	exp_max = (Word16 *) sram_alloc(size);

	size = (4) * sizeof(Word16);
	frac_en = (Word16 *) sram_alloc(size);

	size = (4) * sizeof(Word16);
	exp_en = (Word16 *) sram_alloc(size);

	size = (3) * sizeof(Word16);
	g_pitch_cand = (Word16 *) sram_alloc(size);

	size = (3) * sizeof(Word16);
	g_pitch_cind = (Word16 *) sram_alloc(size);

	size = (COMPLEN) * sizeof(Word16);
	level = (Word16 *) sram_alloc(size);

	size = (6) * sizeof(Word16);
	gCoeff = (Word16 *) sram_alloc(size);

	size = (M) * sizeof(Word16);
	mem_syn_save = (Word16 *) sram_alloc(size);

	size = (M) * sizeof(Word16);
	mem_w0_save = (Word16 *) sram_alloc(size);

	size = (M) * sizeof(Word16);
	mem_err_save = (Word16 *) sram_alloc(size);

	size = (L_FRAME/L_FRAME_BY2) * sizeof(Word16);
	T_op = (Word16 *) sram_alloc(size);

	scaled_y1 = dn;

	scal_y2 = dn2;

	excf = dn;

	scaled_excf = dn2;

	rrv = dn2;

   inno_sav = dn;

	ps_poss = dn2;

  

  size = sizeof(Speech_Decode_FrameState);
  Ptr_Speech_Decode_FrameState = (Speech_Decode_FrameState *) sram_alloc(size);

  return;
};



void MagicPixel_AMR_SRAM_Init(char *start, int size)
{
   sramsize = (unsigned long)size;   
   sram = start;
   AMRDEC_init_sram();
   Ptr_Speech_Decode_FrameState->tablesaddr=(unsigned short)sramaddr;
   Ptr_Speech_Decode_FrameState->tablemode = -1;
   InitTable();
}

#endif

