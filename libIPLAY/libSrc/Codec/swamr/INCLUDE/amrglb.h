/*
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0                
*                                REL-4 Version 4.1.0                
*
*****************************************************************************
*
*      File             : frame.h
*      Purpose          : Declaration of received and transmitted frame types
*
*****************************************************************************
*/
#ifndef amrglb_h
#define amrglb_h  
//"$Id $"


extern Word16 *scaled_signal;
extern Word32 *corr;
// cor_h.c
extern Word16 *h2;
extern Word32 *y32;
//az_lsp.c
extern Word16 *f1, *f2;
//syn_filt.c
extern Word16 *tmp, *tmp2;
// pre_big.c, spreproc.c
extern Word16 *Ap1;
extern Word16 *Ap2;
// pitch_fr.c
extern Word16 *excf, *scaled_excf, *corr_v;
// autocorr.c
extern Word16 *y;

//extern Speech_Encode_FrameState *mem_Speech_Encode_FrameState;
//extern Pre_ProcessState *mem_Pre_ProcessState;
//extern cod_amrState *mem_cod_amrState;
extern Word16 *parm, *syn;
//extern Word16 *new_speech; 
extern Word16 *lsp_new, *lsp_new_q, *lsp_mid, *lsp_mid_q;
extern Word16 *A_t, *Aq_t;
extern Word16 *xn, *xn2, *code, *y1, *y2, *res, *res2;
extern Word16 *xn_sf0, *y2_sf0, *code_sf0, *h1_sf0;
extern Word16 *ipos, *pos_max, *codvec;
extern Word16 *dn, *sign, *dn2, *dn_sign, *_sign;
extern Word16 (*rr) [L_CODE];
extern Word16 *linear_signs;
extern Word16 *linear_codewords;
extern Word16 *scal_y2, *scaled_y1;
extern Word16 *frac_coeff, *exp_coeff;
extern Word16 *lspt;
extern Word16 *Ah, *Al, *Anh, *Anl;
extern Word16 *rc, *rLow, *rHigh;
extern Word32 *lf1, *lf2;
extern Word16 *inno_sav, *ps_poss;
extern Word16 *Ap3, *Ap4;
extern Word16 *lsf1, *lsf2, *wf1, *wf2, *lsf_p, *lsf_r1, *lsf_r2, *lsf1_q, *lsf2_q, *temp_r1, *temp_p, *lsf1_r, *lsf2_r;
extern Word16 *coeff, *coeff_lo, *exp_max;
extern Word16 *frac_en, *exp_en, *g_pitch_cand, *g_pitch_cind;
extern Word16 *rrv;
extern Word16 *level;
extern Word16 *gCoeff;
extern Word16 *mem_syn_save, *mem_w0_save, *mem_err_save, *T_op;

//Honda : assign scratch RAM
extern Pre_ProcessState *Ptr_Pre_ProcessState;
extern cod_amrState *Ptr_cod_amrState;
//extern clLtpState *Ptr_clLtpState;
extern lspState *Ptr_lspState;
//extern Q_plsfState *Ptr_Q_plsfState;
extern gainQuantState *Ptr_gainQuantState;
extern gc_predState *Ptr_gc_predState0;
extern gc_predState *Ptr_gc_predState1;
extern GainAdaptState *Ptr_GainAdaptState;
extern pitchOLWghtState *Ptr_pitchOLWghtState;
extern tonStabState *Ptr_tonStabState;
extern vadState1 *Ptr_vadState1;
extern dtx_encState *Ptr_dtx_encState;
//extern LevinsonState *Ptr_LevinsonState;
//extern lpcState *Ptr_lpcState;
extern Speech_Encode_FrameState *Ptr_Speech_Encode_FrameState;

/* gloabl variable */
extern Word16 amrdec_mode;
extern Word16 amrenc_used_mode;
extern Word16 amrdec_frame_type;

#endif
