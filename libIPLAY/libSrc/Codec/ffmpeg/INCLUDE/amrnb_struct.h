/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0                
*                                REL-4 Version 4.1.0                
*
********************************************************************************
*
*      File             : lpc.h
*      Purpose          : 2 LP analyses centered at 2nd and 4th subframe
*                         for mode 12.2. For all other modes a
*                         LP analysis centered at 4th subframe is 
*                         performed.
*
********************************************************************************
*/
#ifndef lpc_h
#define lpc_h  


#define grid_points 60
#define LTPG_MEM_SIZE 5 /* number of stored past LTP coding gains + 1 */

typedef struct {
    Word16 onset;                   /* onset state,                   Q0  */
    Word16 prev_alpha;              /* previous adaptor output,       Q15 */
    Word16 prev_gc;                 /* previous code gain,            Q1  */

    Word16 ltpg_mem[LTPG_MEM_SIZE]; /* LTP coding gain history,       Q13 */
                                    /* (ltpg_mem[0] not used for history) */
} GainAdaptState;
 
typedef struct {
   Word16 past_qua_en[4];         /* normal MA predictor memory,         Q10 */
                                  /* (contains 20*log10(qua_err))            */
   Word16 past_qua_en_MR122[4];   /* MA predictor memory for MR122 mode, Q10 */
                                  /* (contains log2(qua_err))                */
} gc_predState;
typedef struct {
    Word16 sf0_exp_gcode0;
    Word16 sf0_frac_gcode0;
    Word16 sf0_exp_target_en;
    Word16 sf0_frac_target_en;
    Word16 sf0_exp_coeff[5];
    Word16 sf0_frac_coeff[5];
    Word16 *gain_idx_ptr;
    
    gc_predState     gc_predSt;
    gc_predState     gc_predUnqSt;
    GainAdaptState   adaptSt;
} gainQuantState;


typedef struct {
   Word16 old_T0_med;
   Word16 ada_w;
   Word16 wght_flg; 
} pitchOLWghtState;

 
typedef struct {
   /* Past LSPs */
   Word16 lsp_old[M];
   Word16 lsp_old_q[M];
   /* Quantization state */
   Word16 past_rq[M];    /* Past quantized prediction error, Q15 */
} lspState;

typedef struct {

   /* counters */
   Word16 count;
   
   /* gain history Q11 */
   Word16 gp[N_FRAME];
   
} tonStabState;
typedef struct {
   /* Speech vector */
   Word16 old_speech[L_TOTAL];
   Word16 *speech, *p_window, *p_window_12k2;
   Word16 *new_speech;             /* Global variable */
   
   /* Weight speech vector */
   Word16 old_wsp[L_FRAME + PIT_MAX];
   Word16 *wsp;

   /* OL LTP states */
   Word16 old_lags[5];
   Word16 ol_gain_flg[2];

   /* Excitation vector */
   Word16 old_exc[L_FRAME + PIT_MAX + L_INTERPOL];
   Word16 *exc;

   /* Zero vector */
   Word16 ai_zero[L_SUBFR + MP1];
   Word16 *zero;

   /* Impulse response vector */
   Word16 *h1;
   Word16 hvec[L_SUBFR * 2];

   /* Substates */
   Word16 old_A[M + 1];     /* Last A(z) for case of unstable filter */
   lspState   lspSt;
   Word16 T0_prev_subframe;   /* integer pitch lag of previous sub-frame */
   gainQuantState  gainQuantSt;
   pitchOLWghtState pitchOLWghtSt;
   tonStabState tonStabSt;
   vadState vadSt;
   Flag dtx;
   dtx_encState dtx_encSt;

   /* Filter's memory */
   Word16 mem_syn[M], mem_w0[M], mem_w[M];
   Word16 mem_err[M + L_SUBFR], *error;
   Word16 sharp;
   Word16 mode;
   Word16 T0;
   Word16 T0_frac;
   Word16 gain_pit;
   Word16 gain_code;
   Word16 lsp_flag;
   Word16 gp_limit;            /* pitch gain limit value            */
   Word16 gain_pit_sf0;        /* Quantized pitch gain for sf0         */
   Word16 gain_code_sf0;       /* Quantized codebook gain for sf0      */
   Word16 *A, *Aq;             /* Pointer on A_t and Aq_t              */
} cod_amrState;


typedef struct {
  Word16 y2_hi;
  Word16 y2_lo;
  Word16 y1_hi;
  Word16 y1_lo;
  Word16 x0;
  Word16 x1;
} Pre_ProcessState;


typedef struct{
    Pre_ProcessState pre_state;
    cod_amrState   cod_amr_state;
    Flag dtx;
} Speech_Encode_FrameState;

typedef struct {
    Word16 sid_update_rate;  /* Send SID Update every sid_update_rate frame */
    Word16 sid_update_counter; /* Number of frames since last SID          */
    Word16 sid_handover_debt;  /* Number of extra SID_UPD frames to schedule*/
    Word16 prev_ft;
} sid_syncState;









//--------------------------------------------------------------------
//--------------------------------------------------------------------

#define PHDGAINMEMSIZE 5
#define PHDTHR1LTP     9830  /* 0.6 in Q14 */
#define PHDTHR2LTP     14746 /* 0.9 in Q14 */
#define ONFACTPLUS1    16384 /* 2.0 in Q13   */
#define ONLENGTH 2

typedef struct {
  Word16 gainMem[PHDGAINMEMSIZE];
  Word16 prevState;
  Word16 prevCbGain;
  Word16 lockFull;
  Word16 onset;
} ph_dispState;

typedef struct {
  Word16 pbuf[5];
  Word16 past_gain_pit;
  Word16 prev_gp;
} ec_gain_pitchState;
 
typedef struct {
  Word16 gbuf[5];
  Word16 past_gain_code;
  Word16 prev_gc;
} ec_gain_codeState;


typedef struct {
  Word16 past_r_q[M];   /* Past quantized prediction error, Q15 */
  Word16 past_lsf_q[M]; /* Past dequantized lsfs,           Q15 */
} D_plsfState;


#define EXPCONST          5243               /* 0.16 in Q15 */
typedef struct {
  Word16 lsp_meanSave[M];          /* Averaged LSPs saved for efficiency  */
} lsp_avgState;

#define L_CBGAINHIST 7
typedef struct{
   /* history vector of past synthesis speech energy */
   Word16 cbGainHistory[L_CBGAINHIST];
   
   /* state flags */
   Word16 hangVar;       /* counter; */
   Word16 hangCount;     /* counter; */

} Cb_gain_averageState;


#define L_ENERGYHIST 60
#define INV_L_FRAME 102


/* 2*(160*x)^2 / 65536  where x is FLP values 150,5 and 50 */
#define FRAMEENERGYLIMIT  17578         /* 150 */
#define LOWERNOISELIMIT      20         /*   5 */
#define UPPERNOISELIMIT    1953         /*  50 */
typedef struct{
   /* history vector of past synthesis speech energy */
   Word16 frameEnergyHist[L_ENERGYHIST];
   
   /* state flags */
   Word16 bgHangover;       /* counter; number of frames after last speech frame */

} Bgn_scdState;


typedef struct{
  /* Excitation vector */
  Word16 *exc;
  Word16 old_exc[L_SUBFR + PIT_MAX + L_INTERPOL];
   
  /* Lsp (Line spectral pairs) */
   /* Word16 lsp[M]; */      /* Used by CN codec */
  Word16 lsp_old[M];
 
  /* Filter's memory */
  Word16 mem_syn[M];

  /* pitch sharpening */
  Word16 sharp;
  Word16 old_T0;

  /* Memories for bad frame handling */
  Word16 prev_bf;
  Word16 prev_pdf;   
  Word16 state;
  Word16 excEnergyHist[9];

  /* Variable holding received ltpLag, used in background noise and BFI */
  Word16 T0_lagBuff;

  /* Variables for the source characteristic detector (SCD) */
  Word16 inBackgroundNoise;
  Word16 voicedHangover;
  Word16 ltpGainHistory[9];

  Bgn_scdState background_state;
  Word16 nodataSeed;
  
  Cb_gain_averageState Cb_gain_averState;
  lsp_avgState lsp_avg_st;
   
   D_plsfState lsfState;
   ec_gain_pitchState ec_gain_p_st;
   ec_gain_codeState ec_gain_c_st;  
   gc_predState pred_state;
   ph_dispState ph_disp_st;
   dtx_decState dtxDecoderState;
} Decoder_amrState;

typedef struct {
    Word16 past_gain;
} agcState;


typedef struct {
  Word16 mem_pre;          /* filter state */
} preemphasisState;

typedef struct{
  Word16 res2[L_SUBFR];
  Word16 mem_syn_pst[M];
  preemphasisState preemph_state;
  Word16 agc_state;
  Word16 synth_buf[M + L_FRAME];  
} Post_FilterState;


typedef struct {
   Word16 y2_hi;
   Word16 y2_lo;
   Word16 y1_hi;
   Word16 y1_lo;
   Word16 x0;
   Word16 x1;
} Post_ProcessState; 

typedef struct{
  Decoder_amrState  decoder_amrState;
  Post_FilterState  post_state;
  Post_ProcessState postHP_state;
  Word16 prev_mode;
  unsigned short tablesaddr;
  char tablemode; 
//  int complexityCounter;   /* Only for complexity computation            */
} Speech_Decode_FrameState;
#endif
