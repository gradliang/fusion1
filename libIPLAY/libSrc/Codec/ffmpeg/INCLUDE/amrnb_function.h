/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0                
*                                REL-4 Version 4.1.0                
*
********************************************************************************
*
*      File             : levinson.h
*      Purpose          : Levinson-Durbin algorithm in double precision.
*                       : To compute the LP filter parameters from the
*                       : speech autocorrelations.
*
********************************************************************************
*/
#ifndef levinson_h
#define levinson_h  

 
#ifdef dohoming 
void Levinson_reset(Word16 *st);
#endif
int Levinson (
    Word16 *st,
    Word16 Rh[],       /* i : Rh[m+1] Vector of autocorrelations (msb) */
    Word16 Rl[],       /* i : Rl[m+1] Vector of autocorrelations (lsb) */
    Word16 A[],        /* o : A[m]    LPC coefficients  (m = 10)       */
    Word16 rc[]        /* o : rc[4]   First 4 reflection coefficients  */
);

int lpc(
    Word16 *st,     /* i/o: State struct                */
    Word16 mode,   /* i  : coder mode                  */
    Word16 x[],       /* i  : Input signal           Q15  */
    Word16 x_12k2[],  /* i  : Input signal (EFR)     Q15  */
    Word16 a[]        /* o  : predictor coefficients Q12  */
);


int lsp_init (lspState **st);
void lsp_reset (lspState *st);
int lsp(lspState *st,        /* i/o : State struct                            */
        Word16 req_mode,  /* i   : requested coder mode                    */
        Word16 used_mode, /* i   : used coder mode                         */        
        Word16 az[],         /* i/o : interpolated LP parameters Q12          */
        Word16 azQ[],        /* o   : quantization interpol. LP parameters Q12*/
        Word16 lsp_new[],    /* o   : new lsp vector                          */ 
        Word16 **anap        /* o   : analysis parameters                     */
        );


void Q_plsf_3(
    Word16 *st,    /* i/o: state struct                             */
    Word16 mode,     /* i  : coder mode                               */
    Word16 *lsp1,       /* i  : 1st LSP vector                      Q15  */
    Word16 *lsp1_q,     /* o  : quantized 1st LSP vector            Q15  */
    Word16 *indice,     /* o  : quantization indices of 3 vectors   Q0   */
    Word16 *pred_init_i /* o  : init index for MA prediction in DTX mode */
);

void Q_plsf_5 (
    Word16 *st,
    Word16 *lsp1,      /* i  : 1st LSP vector,                     Q15 */
    Word16 *lsp2,      /* i  : 2nd LSP vector,                     Q15 */   
    Word16 *lsp1_q,    /* o  : quantized 1st LSP vector,           Q15 */
    Word16 *lsp2_q,    /* o  : quantized 2nd LSP vector,           Q15 */
    Word16 *indice     /* o  : quantization indices of 5 matrices, Q0  */
);

int ol_ltp(
    cod_amrState *cst,
    Word16 mode,           /* i   : coder mode                              */
    Word16 wsp[],         /* i   : signal used to compute the OL pitch, Q0 */
                          /*       uses signal[-pit_max] to signal[-1]     */
    Word16 *T_op,         /* o   : open loop pitch lag,                 Q0 */
    Word16 old_lags[],    /* i   : history with old stored Cl lags         */
    Word16 ol_gain_flg[], /* i   : OL gain flag                            */
    Word16 idx           /* i   : index                                   */
);

void p_ol_wgh_init (pitchOLWghtState *s);
#ifdef dohoming 
void p_ol_wgh_reset (pitchOLWghtState *st);
#endif


Word16 Pitch_ol_wgh (     /* o   : open loop pitch lag                            */
    cod_amrState *cst,
    Word16 signal[],      /* i   : signal used to compute the open loop pitch     */
                          /*       signal[-pit_max] to signal[-1] should be known */
    Word16 pit_min,       /* i   : minimum pitch lag                              */
    Word16 pit_max,       /* i   : maximum pitch lag                              */
    Word16 L_frame,       /* i   : length of frame to compute pitch               */
    Word16 old_lags[],    /* i   : history with old stored Cl lags                */
    Word16 ol_gain_flg[], /* i   : OL gain flag                                   */
    Word16 idx           /* i   : index                                          */
    );


Word16 Pitch_fr (        /* o   : pitch period (integer)                    */
    cod_amrState *cst,   
    Word16 mode,      /* i   : codec mode                                */
    Word16 T_op[],       /* i   : open loop pitch lags                      */
    Word16 exc[],        /* i   : excitation buffer                      Q0 */
    Word16 xn[],         /* i   : target vector                          Q0 */
    Word16 i_subfr,      /* i   : subframe offset                           */
    Word16 *resu3,       /* o   : subsample resolution 1/3 (=1) or 1/6 (=0) */
    Word16 *ana_index    /* o   : index of encoding                         */
);

int cl_ltp (
    cod_amrState *st,   
    Word16 mode,      /* i   : coder mode                                */
    Word16 frameOffset,  /* i   : Offset to subframe                        */
    Word16 res2[],       /* i/o : Long term prediction residual          Q0 */
    Word16 xn2[],        /* o   : Target vector for codebook search      Q0 */
    Word16 y1[],         /* o   : Filtered adaptive excitation           Q0 */
    Word16 g_coeff[],    /* o   : Correlations between xn, y1, & y2         */
    Word16 **anap       /* o   : Analysis parameters                       */
);


#ifdef dohoming
void gainQuant_reset (gainQuantState *st);
#endif



int gainQuant(
    cod_amrState *cst,  
    Word16 mode,       /* i   : coder mode                        */
    Word16 exc[],         /* i   : LTP excitation (unfiltered), Q0   */
    Word16 code[],        /* i   : CB innovation (unfiltered),  Q13  */
                          /*       (unsharpened for MR475)           */
    Word16 xn[],          /* i   : Target vector.                    */
    Word16 xn2[],         /* i   : Target vector.                    */
    Word16 even_subframe, /* i   : even subframe indicator flag      */
    Word16 **anap         /* o   : Index of quantization             */
);

 
void gc_pred_reset (gc_predState *st);


void
gc_pred_copy(
    gc_predState *st_src,  /* i : State struct                           */
    gc_predState *st_dest  /* o : State struct                           */
);

void gc_pred(
    gc_predState *st,   /* i/o: State struct                           */
    Word16 mode,     /* i  : AMR mode                               */
    Word16 *code,       /* i  : innovative codebook vector (L_SUBFR)   */
                        /*      MR122: Q12, other modes: Q13           */
    Word16 *exp_gcode0, /* o  : exponent of predicted gain factor, Q0  */
    Word16 *frac_gcode0,/* o  : fraction of predicted gain factor  Q15 */
    Word16 *exp_en,     /* o  : exponent of innovation energy,     Q0  */
                        /*      (only calculated for MR795)            */
    Word16 *frac_en     /* o  : fraction of innovation energy,     Q15 */
                        /*      (only calculated for MR795)            */
);


void gc_pred_update(
    gc_predState *st,      /* i/o: State struct                     */
    Word16 qua_ener_MR122, /* i  : quantized energy for update, Q10 */
                           /*      (log2(qua_err))                  */
    Word16 qua_ener        /* i  : quantized energy for update, Q10 */
                           /*      (20*log10(qua_err))              */
);

void gc_pred_average_limited(
    gc_predState *st,       /* i: State struct                    */
    Word16 *ener_avg_MR122, /* o: averaged quantized energy,  Q10 */
                            /*    (log2(qua_err))                 */
    Word16 *ener_avg        /* o: averaged quantized energy,  Q10 */
                            /*    (20*log10(qua_err))             */
);


void gain_adapt(
    GainAdaptState *st,  /* i  : state struct                  */
    Word16 ltpg,         /* i  : ltp coding gain (log2()), Q   */
    Word16 gain_cod,     /* i  : code gain,                Q13 */
    Word16 *alpha        /* o  : gain adaptation factor,   Q15 */
);

void Set_zero (
    Word16 x[],        /* (o)  : vector to clear                            */
    Word16 L           /* (i)  : length of vector                           */
);

void Log2 (
    Word32 L_x,        /* (i) : input value                                 */
    Word16 *exponent,  /* (o) : Integer part of Log2.   (range: 0<=val<=30) */
    Word16 *fraction   /* (o) : Fractional part of Log2. (range: 0<=val<1)*/
);

void Log2_norm (
    Word32 L_x,         /* (i) : input value (normalized)                    */
    Word16 exp,         /* (i) : norm_l (L_x)                                */
    Word16 *exponent,   /* (o) : Integer part of Log2.   (range: 0<=val<=30) */
    Word16 *fraction    /* (o) : Fractional part of Log2. (range: 0<=val<1)  */
);

Word16 Qua_gain(                   /* o  : index of quantization.                 */ 
    cod_amrState *cst,           
    Word16 mode,         /* i  : AMR mode                               */
    Word16 exp_gcode0,      /* i  : predicted CB gain (exponent),      Q0  */
    Word16 frac_gcode0,     /* i  : predicted CB gain (fraction),      Q15 */
    Word16 frac_coeff[],    /* i  : energy coeff. (5), fraction part,  Q15 */
    Word16 exp_coeff[],     /* i  : energy coeff. (5), exponent part,  Q0  */
                            /*      (frac_coeff and exp_coeff computed in  */
                            /*       calc_filt_energies())                 */
    Word16 *qua_ener_MR122, /* o  : quantized energy error,            Q10 */
                            /*      (for MR122 MA predictor update)        */
    Word16 *qua_ener        /* o  : quantized energy error,            Q10 */
                            /*      (for other MA predictor update)        */
);

int pre_big(
    cod_amrState *st,           
    Word16 mode,            /* i  : coder mode                             */
    const Word16 gamma1[],     /* i  : spectral exp. factor 1                 */
    const Word16 gamma1_12k2[],/* i  : spectral exp. factor 1 for EFR         */
    const Word16 gamma2[],     /* i  : spectral exp. factor 2                 */
    Word16 A_t[],              /* i  : A(z) unquantized, for 4 subframes, Q12 */
    Word16 frameOffset        /* i  : Start position in speech vector,   Q0  */
);

int subframePreProc(
    cod_amrState *st,
    int i_subfr,
    Word16 mode,            /* i  : coder mode                            */
    Word16 *A,                 /* i  : A(z) unquantized for the 4 subframes  */
    Word16 *Aq,                /* i  : A(z)   quantized for the 4 subframes  */
    Word16 *mem_w0,            /* i  : memory of weighting filter            */
    Word16 xn[],               /* o  : target vector for pitch search        */
    Word16 res2[]             /* o  : long term prediction residual         */
);
void Pred_lt_3or6 (
    Word16 exc[],     /* in/out: excitation buffer                         */
    Word16 T0,        /* input : integer pitch lag                         */
    Word16 frac,      /* input : fraction of lag                           */
    Word16 L_subfr,   /* input : subframe size                             */
    Word16 flag3      /* input : if set, upsampling rate = 3 (6 otherwise) */
);
int subframePostProc(
    cod_amrState *st, 
    Word16 mode,   /* i   : coder mode                            */
    Word16 i_subfr,   /* i   : Subframe nr                           */
    Word16 synth[],   /* i   : Local snthesis                        */
    Word16 xn[],      /* i   : Target vector for pitch search        */
    Word16 code[],    /* i   : Fixed codebook exitation              */
//    Word16 y1[],      /* i   : Filtered adaptive exitation           */
    Word16 y2[]      /* i   : Filtered fixed codebook excitation    */
);

#if OPT_475
int subframePostProcMR475(
    cod_amrState *st, 
    Word16 mode,   /* i   : coder mode                            */
    Word16 i_subfr,   /* i   : Subframe nr                           */
    Word16 gain_pit,  /* i   : Pitch gain                       Q14  */
    Word16 gain_code, /* i   : Decoded innovation gain               */
    Word16 synth[],   /* i   : Local snthesis                        */
    Word16 xn[],      /* i   : Target vector for pitch search        */
    Word16 code[],    /* i   : Fixed codebook exitation              */
//    Word16 y1[],      /* i   : Filtered adaptive exitation           */
    Word16 y2[],      /* i   : Filtered fixed codebook excitation    */
    Word16 *mem_syn,  /* i/o : memory of synthesis filter            */
    Word16 *mem_w0,   /* o   : memory of weighting filter            */
    Word16 *sharp     /* o   : pitch sharpening value                */
);
#endif

int cbsearch(
             cod_amrState *st, 
             Word16 x[],    /* i : target vector, Q0                       */
             Word16 res2[], /* i : Long term prediction residual, Q0       */
             Word16 code[], /* o : Innovative codebook, Q13                */
             Word16 y[],    /* o : filtered fixed codebook excitation, Q12 */
             Word16 **anap, /* o : Signs of the pulses                     */
             Word16 mode,/* i : coder mode                              */
             Word16 subNr)  /* i : subframe number                         */;   /* i : subframe number                        */
void Convolve (
    Word16 x[],        /* (i)  : input vector                               */
    Word16 h[],        /* (i)  : impulse response                           */
    Word16 y[]        /* (o)  : output vector                              */
);
 

Word16 check_lsp(tonStabState *st, /* i/o : State struct            */
                 Word16 *lsp       /* i   : unquantized LSP's       */
);
Word16 check_gp_clipping(tonStabState *st, /* i/o : State struct            */
                         Word16 g_pitch    /* i   : pitch gain              */
);
void update_gp_clipping(tonStabState *st, /* i/o : State struct            */
                        Word16 g_pitch    /* i   : pitch gain              */
);


void cor_h_x (
    Word16 h[],     /* (i) : impulse response of weighted synthesis filter */
    Word16 x[],     /* (i) : target                                        */
    Word16 dn[],    /* (o) : correlation between target and h[]            */
    Word16 sf       /* (i) : scaling factor: 2 for 12.2, 1 for 7.4         */
);

void cor_h_x2 (
    Word16 h[],     /* (i) : impulse response of weighted synthesis filter */
    Word16 x[],     /* (i) : target                                        */
    Word16 dn[],    /* (o) : correlation between target and h[]            */
    Word16 sf,      /* (i) : scaling factor: 2 for 12.2, 1 for 7.4         */
    Word16 nb_track,/* (i) : the number of ACB tracks                      */
    Word16 step     /* (i) : step size from one pulse position to the next
                             in one track                                  */
);

void cor_h (
    Word16 h[],     /* (i) : impulse response of weighted synthesis filter */
    Word16 sign[],      /* (i) : sign of d[n]                              */
    Word16 rr[][L_CODE] /* (o) : matrix of autocorrelation                 */
);


void set_sign(Word16 dn[],   /* i/o : correlation between target and h[]    */
              Word16 sign[], /* o   : sign of dn[]                          */
              Word16 dn2[],  /* o   : maximum of correlation in each track. */
              Word16 n       /* i   : # of maximum correlations in dn2[]    */
);

void set_sign12k2 (
    Word16 dn[],      /* i/o : correlation between target and h[]         */
    Word16 cn[],      /* i   : residual after long term prediction        */
    Word16 sign[],    /* o   : sign of d[n]                               */
    Word16 pos_max[], /* o   : position of maximum correlation            */
    Word16 nb_track,  /* i   : number of tracks tracks                    */        
    Word16 ipos[],    /* o   : starting position for each pulse           */
    Word16 step       /* i   : the step size in the tracks                */        
);

void search_10and8i40 (
    Word16 nbPulse,      /* i : nbpulses to find                       */
    Word16 step,         /* i :  stepsize                              */
    Word16 nbTracks,     /* i :  nbTracks                              */
    Word16 dn[],         /* i : correlation between target and h[]     */
    Word16 rr[][L_CODE], /* i : matrix of autocorrelation              */
    Word16 ipos[],       /* i : starting position for each pulse       */
    Word16 pos_max[],    /* i : position of maximum of dn[]            */
    Word16 codvec[]      /* o : algebraic codebook vector              */
);
Word16 code_2i40_9bits(
    Word16 subNr,       /* i : subframe number                               */
    Word16 x[],         /* i : target vector                                 */
    Word16 h[],         /* i : impulse response of weighted synthesis filter */
                        /*     h[-L_subfr..-1] must be set to zero.          */
    Word16 T0,          /* i : Pitch lag                                     */
    Word16 pitch_sharp, /* i : Last quantized pitch gain                     */
    Word16 code[],      /* o : Innovative codebook                           */
    Word16 y[],         /* o : filtered fixed codebook excitation            */
    Word16 * sign       /* o : Signs of 2 pulses                             */
);



Word16 code_2i40_11bits(
    Word16 x[], /* i : target vector                                 */
    Word16 h[], /* i : impulse response of weighted synthesis filter */
                /*     h[-L_subfr..-1] must be set to zero.          */
    Word16 T0,  /* i : Pitch lag                                     */
    Word16 pitch_sharp, /* i : Last quantized pitch gain             */
    Word16 code[],      /* o : Innovative codebook                   */
    Word16 y[],         /* o : filtered fixed codebook excitation    */
    Word16 * sign       /* o : Signs of 2 pulses                     */
);

void code_10i40_35bits (
    Word16 x[],        /* (i)   : target vector                             */
    Word16 cn[],       /* (i)   : residual after long term prediction       */
    Word16 h[],        /* (i)   : impulse response of weighted synthesis
                                  filter                                    */
    Word16 cod[],      /* (o)   : algebraic (fixed) codebook excitation     */
    Word16 y[],        /* (o)   : filtered fixed codebook excitation        */
    Word16 indx[]      /* (o)   : index of 10 pulses (sign + position)      */
);

Word16 code_3i40_14bits(
    Word16 x[], /* (i)   : target vector                                 */
    Word16 h[], /* (i)   : impulse response of weighted synthesis filter */
                /*         h[-L_subfr..-1] must be set to zero.          */
    Word16 T0,  /* (i)   : Pitch lag                                     */
    Word16 pitch_sharp, /* (i)   : Last quantized pitch gain             */
    Word16 code[],      /* (o)   : Innovative codebook                   */
    Word16 y[],         /* (o)   : filtered fixed codebook excitation    */
    Word16 * sign       /* (o)   : Signs of 3 pulses                     */
);






Word16 q_gain_pitch (   /* Return index of quantization                      */
    Word16 mode,     /* i  : AMR mode                                     */
    Word16 gp_limit,    /* i  : pitch gain limit                             */
    Word16 *gain,       /* i/o: Pitch gain (unquant/quant),              Q14 */
    Word16 gain_cand[], /* o  : pitch gain candidates (3),   MR795 only, Q14 */ 
    Word16 gain_cind[]  /* o  : pitch gain cand. indices (3),MR795 only, Q0  */ 
);
 
Word16 G_pitch     (    /* o : Gain of pitch lag saturated to 1.2       */
    Word16 mode,     /* i : AMR mode                                 */
    Word16 xn[],        /* i : Pitch target.                            */
    Word16 y1[],        /* i : Filtered adaptive codebook.              */
    Word16 g_coeff[],   /* i : Correlations need for gain quantization.
                               (7.4 only). Pass NULL if not needed      */
    Word16 L_subfr      /* i : Length of subframe.                      */
);

Word16 gmed_n (   /* o : index of the median value (0...N-1)      */
    Word16 ind[], /* i : Past gain values                         */
    Word16 n      /* i : The number of gains; this routine        */
                  /*     is only valid for a odd number of gains  */ 
);


void Lsf_lsp (
    Word16 lsf[],      /* (i)    : lsf[m] normalized (range: 0.0<=val<=0.5) */
    Word16 lsp[],      /* (o)    : lsp[m] (range: -1<=val<1)                */
    Word16 m           /* (i)    : LPC order                                */
);
void Lsp_lsf (
    Word16 lsp[],      /* (i)    : lsp[m] (range: -1<=val<1)                */
    Word16 lsf[],      /* (o)    : lsf[m] normalized (range: 0.0<=val<=0.5) */
    Word16 m           /* (i)    : LPC order                                */
);
void Reorder_lsf (
    Word16 *lsf,       /* (i/o)  : vector of LSFs   (range: 0<=val<=0.5)    */
    Word16 min_dist,   /* (i)    : minimum required distance                */
    Word16 n           /* (i)    : LPC order                                */
);

void Copy (
    const Word16 x[],  /* i : input vector (L)    */
    Word16 y[],        /* o : output vector (L)   */
    Word16 L           /* i : vector length       */
);
 
void calc_unfilt_energies(
    Word16 res[],     /* i  : LP residual,                               Q0  */
    Word16 exc[],     /* i  : LTP excitation (unfiltered),               Q0  */
    Word16 code[],    /* i  : CB innovation (unfiltered),                Q13 */
    Word16 gain_pit,  /* i  : pitch gain,                                Q14 */
    Word16 L_subfr,   /* i  : Subframe length                                */

    Word16 frac_en[], /* o  : energy coefficients (3), fraction part,    Q15 */
    Word16 exp_en[],  /* o  : energy coefficients (3), exponent part,    Q0  */
    Word16 *ltpg      /* o  : LTP coding gain (log2()),                  Q13 */
);


void calc_filt_energies(
    Word16 mode,     /* i  : coder mode                                   */
    Word16 xn[],        /* i  : LTP target vector,                       Q0  */
    Word16 xn2[],       /* i  : CB target vector,                        Q0  */
    Word16 Y2[],        /* i  : Filtered innovative vector,              Q12 */
    Word16 frac_coeff[],/* o  : energy coefficients (5), fraction part,  Q15 */
    Word16 exp_coeff[], /* o  : energy coefficients (5), exponent part,  Q0  */
    Word16 *cod_gain_frac,/* o: optimum codebook gain (fraction part),   Q15 */
    Word16 *cod_gain_exp  /* o: optimum codebook gain (exponent part),   Q0  */
);


void calc_target_energy(
    Word16 xn[],     /* i: LTP target vector,                       Q0  */
    Word16 *en_exp,  /* o: optimum codebook gain (exponent part),   Q0  */
    Word16 *en_frac  /* o: optimum codebook gain (fraction part),   Q15 */
);

void MR475_update_unq_pred(
    gc_predState *pred_st, /* i/o: gain predictor state struct            */
    Word16 exp_gcode0,     /* i  : predicted CB gain (exponent),      Q0  */
    Word16 frac_gcode0,    /* i  : predicted CB gain (fraction),      Q15 */
    Word16 cod_gain_exp,   /* i  : optimum codebook gain (exponent),  Q0  */
    Word16 cod_gain_frac   /* i  : optimum codebook gain (fraction),  Q15 */
);


Word16 MR475_gain_quant(              /* o  : index of quantization.                 */
    gc_predState *pred_st,     /* i/o: gain predictor state struct            */
      
                               /* data from subframe 0 (or 2) */
    Word16 sf0_exp_gcode0,     /* i  : predicted CB gain (exponent),      Q0  */
    Word16 sf0_frac_gcode0,    /* i  : predicted CB gain (fraction),      Q15 */
    Word16 sf0_exp_coeff[],    /* i  : energy coeff. (5), exponent part,  Q0  */
    Word16 sf0_frac_coeff[],   /* i  : energy coeff. (5), fraction part,  Q15 */
                               /*      (frac_coeff and exp_coeff computed in  */
                               /*       calc_filt_energies())                 */
    Word16 sf0_exp_target_en,  /* i  : exponent of target energy,         Q0  */
    Word16 sf0_frac_target_en, /* i  : fraction of target energy,         Q15 */
      
                               /* data from subframe 1 (or 3) */
    Word16 sf1_code_nosharp[], /* i  : innovative codebook vector (L_SUBFR)   */
                               /*      (whithout pitch sharpening)            */
    Word16 sf1_exp_gcode0,     /* i  : predicted CB gain (exponent),      Q0  */
    Word16 sf1_frac_gcode0,    /* i  : predicted CB gain (fraction),      Q15 */
    Word16 sf1_exp_coeff[],    /* i  : energy coeff. (5), exponent part,  Q0  */
    Word16 sf1_frac_coeff[],   /* i  : energy coeff. (5), fraction part,  Q15 */
                               /*      (frac_coeff and exp_coeff computed in  */
                               /*       calc_filt_energies())                 */
    Word16 sf1_exp_target_en,  /* i  : exponent of target energy,         Q0  */
    Word16 sf1_frac_target_en, /* i  : fraction of target energy,         Q15 */

    Word16 gp_limit,           /* i  : pitch gain limit                       */

    Word16 *sf0_gain_pit,      /* o  : Pitch gain,                        Q14 */
    Word16 *sf0_gain_cod,      /* o  : Code gain,                         Q1  */
      
    Word16 *sf1_gain_pit,      /* o  : Pitch gain,                        Q14 */
    Word16 *sf1_gain_cod       /* o  : Code gain,                         Q1  */
);

void MR795_gain_quant(
    GainAdaptState *adapt_st, /* i/o: gain adapter state structure       */
    Word16 res[],             /* i  : LP residual,                  Q0   */
    Word16 exc[],             /* i  : LTP excitation (unfiltered),  Q0   */
    Word16 code[],            /* i  : CB innovation (unfiltered),   Q13  */
    Word16 frac_coeff[],      /* i  : coefficients (5),             Q15  */
    Word16 exp_coeff[],       /* i  : energy coefficients (5),      Q0   */
                              /*      coefficients from calc_filt_ener() */
    Word16 exp_code_en,       /* i  : innovation energy (exponent), Q0   */
    Word16 frac_code_en,      /* i  : innovation energy (fraction), Q15  */
    Word16 exp_gcode0,        /* i  : predicted CB gain (exponent), Q0   */
    Word16 frac_gcode0,       /* i  : predicted CB gain (fraction), Q15  */
    Word16 L_subfr,           /* i  : Subframe length                    */
    Word16 cod_gain_frac,     /* i  : opt. codebook gain (fraction),Q15  */
    Word16 cod_gain_exp,      /* i  : opt. codebook gain (exponent), Q0  */
    Word16 gp_limit,          /* i  : pitch gain limit                   */
    Word16 *gain_pit,         /* i/o: Pitch gain (unquant/quant),   Q14  */
    Word16 *gain_cod,         /* o  : Code gain,                    Q1   */
    Word16 *qua_ener_MR122,   /* o  : quantized energy error,       Q10  */
                              /*      (for MR122 MA predictor update)    */
    Word16 *qua_ener,         /* o  : quantized energy error,       Q10  */
                              /*      (for other MA predictor update)    */
    Word16 **anap             /* o  : Index of quantization              */
                              /*      (first gain pitch, then code pitch)*/
);


Word16 code_4i40_17bits(
    Word16 x[], /* (i)   : target vector                                 */
    Word16 h[], /* (i)   : impulse response of weighted synthesis filter */
                /*         h[-L_subfr..-1] must be set to zero.          */
    Word16 T0,  /* (i)   : Pitch lag                                     */
    Word16 pitch_sharp, /* (i)   : Last quantized pitch gain             */
    Word16 code[],      /* (o)   : Innovative codebook                   */
    Word16 y[],         /* (o)   : filtered fixed codebook excitation    */
    Word16 * sign       /* (o)   : Signs of 4 pulses                     */
);


void code_8i40_31bits (
    Word16 x[],        /* i : target vector                                  */
    Word16 cn[],       /* i : residual after long term prediction            */
    Word16 h[],        /* i : impulse response of weighted synthesis
                              filter                                         */
    Word16 cod[],      /* o : algebraic (fixed) codebook excitation          */
    Word16 y[],        /* o : filtered fixed codebook excitation             */
    Word16 indx[]      /* o : 7 Word16, index of 8 pulses (signs+positions)  */
    );
Word16 G_code (        /* out      : Gain of innovation code.               */
    Word16 xn[],       /* in       : target vector                          */
    Word16 y2[]        /* in       : filtered inovation vector              */
);
Word16 q_gain_code (        /* o  : quantization index,            Q0  */
    Word16 mode,         /* i  : AMR mode                           */
    Word16 exp_gcode0,      /* i  : predicted CB gain (exponent),  Q0  */
    Word16 frac_gcode0,     /* i  : predicted CB gain (fraction),  Q15 */
    Word16 *gain,           /* i/o: quantized fixed codebook gain, Q1  */
    Word16 *qua_ener_MR122, /* o  : quantized energy error,        Q10 */
                            /*      (for MR122 MA predictor update)    */
    Word16 *qua_ener        /* o  : quantized energy error,        Q10 */
                            /*      (for other MA predictor update)    */
);

Word32 Div_32 (Word32 L_num, Word16 denom_hi, Word16 denom_lo);
Word16 Autocorr (
    Word16 x[],        /* (i)    : Input signal (L_WINDOW)             */
    Word16 m,          /* (i)    : LPC order                           */
    Word16 r_h[],      /* (o)    : Autocorrelations  (msb)  (MP1)      */
    Word16 r_l[],      /* (o)    : Autocorrelations  (lsb)  (MP1)      */
    const Word16 wind[]/* (i)    : window for LPC analysis. (L_WINDOW) */
);

void Int_lpc_1and3 (
    Word16 lsp_old[],  /* i : LSP vector at the 4th subfr. of past frame (M) */
    Word16 lsp_mid[],  /* i : LSP vector at the 2nd subfr. of
                              present frame (M)                              */
    Word16 lsp_new[],  /* i : LSP vector at the 4th subfr. of
                              present frame (M)                              */
    Word16 Az[]        /* o : interpolated LP parameters in all subfr.
                              (AZ_SIZE)                                      */
);

void Int_lpc_1and3_2 (
    Word16 lsp_old[],  /* i : LSP vector at the 4th subfr. of past frame (M) */
    Word16 lsp_mid[],  /* i : LSP vector at the 2nd subframe of
                             present frame (M)                                  */
    Word16 lsp_new[],  /* i : LSP vector at the 4th subframe of
                             present frame (M)                                  */
    Word16 Az[]        /* o :interpolated LP parameters
                             in subframes 1 and 3 (AZ_SIZE)                     */
);


void Int_lpc_1to3 (
    Word16 lsp_old[], /* i : LSP vector at the 4th SF of past frame (M)      */
    Word16 lsp_new[], /* i : LSP vector at the 4th SF of present frame (M)   */
    Word16 Az[]       /* o : interpolated LP parameters in all SFs (AZ_SIZE) */
);

void Int_lpc_1to3_2 (
    Word16 lsp_old[],  /* i : LSP vector at the 4th SF of past frame (M) */
    Word16 lsp_new[],  /* i : LSP vector at the 4th SF present frame (M) */
    Word16 Az[]        /* o :interpolated LP parameters in SFs 1, 2, 3 
                             (AZ_SIZE)                                   */
);

void Az_lsp (
    Word16 a[],        /* (i)  : predictor coefficients (MP1)              */
    Word16 lsp[],      /* (o)  : line spectral pairs (M)                   */
    Word16 old_lsp[]   /* (i)  : old lsp[] (in case not found 10 roots) (M)*/
);

Word16 Pitch_ol (      /* o   : open loop pitch lag                         */
    cod_amrState *cst,
    Word16 mode,    /* i   : coder mode                                  */
    Word16 signal[],   /* i   : signal used to compute the open loop pitch  */
                       /*    signal[-pit_max] to signal[-1] should be known */
    Word16 pit_min,    /* i   : minimum pitch lag                           */
    Word16 pit_max,    /* i   : maximum pitch lag                           */
    Word16 L_frame,    /* i   : length of frame to compute pitch            */
    Word16 idx        /* i   : frame index                                 */
);
 
void comp_corr (Word16 scal_sig[],  /* i   : scaled signal.                     */
                Word16 L_frame,     /* i   : length of frame to compute pitch   */
                Word16 lag_max,     /* i   : maximum lag                        */
                Word16 lag_min,     /* i   : minimum lag                        */
                Word32 corr[]       /* o   : correlation of selected lag        */
                );

Word32 Inv_sqrt (      /* (o) : output value   (range: 0<=val<1)            */
    Word32 L_x           /* (i) : input value    (range: 0<=val<=7fffffff)    */
);

Word16 Interpol_3or6 (  /* (o)  : interpolated value                        */
    Word16 *x,          /* (i)  : input vector                              */
    Word16 frac,        /* (i)  : fraction  (-2..2 for 3*, -3..3 for 6*)    */
    Word16 flag3        /* (i)  : if set, upsampling rate = 3 (6 otherwise) */
);

Word16
Enc_lag3(                /* o  : Return index of encoding     */
    Word16 T0,           /* i  : Pitch delay                          */
    Word16 T0_frac,      /* i  : Fractional pitch delay               */
    Word16 T0_prev,      /* i  : Integer pitch delay of last subframe */
    Word16 T0_min,       /* i  : minimum of search range              */
    Word16 T0_max,       /* i  : maximum of search range              */
    Word16 delta_flag,   /* i  : Flag for 1st (or 3rd) subframe       */
    Word16 flag4         /* i  : Flag for encoding with 4 bits        */
);

Word16 Enc_lag6 (        /* o  : Return index of encoding             */
    Word16 T0,           /* i  : Pitch delay                          */
    Word16 T0_frac,      /* i  : Fractional pitch delay               */
    Word16 T0_min,       /* i  : minimum of search range              */
    Word16 delta_flag    /* i  : Flag for 1st (or 3rd) subframe       */
);
 
void Weight_Ai (
    Word16 a[],        /* (i)  : a[m+1]  LPC coefficients   (m=10)          */
    const Word16 fac[],/* (i)  : Spectral expansion factors.                */
    Word16 a_exp[]     /* (o)  : Spectral expanded LPC coefficients         */
);

void Residu (
    Word16 a[],        /* (i)  : prediction coefficients                    */
    Word16 x[],        /* (i)  : speech signal                              */
    Word16 y[],        /* (o)  : residual signal                            */
    Word16 lg          /* (i)  : size of filtering                          */
);
 
void Syn_filt (
    Word16 a[],        /* (i)  : a[m+1] prediction coefficients   (m=10)    */
    Word16 x[],        /* (i)  : input signal                               */
    Word16 y[],        /* (o)  : output signal                              */
    Word16 lg,         /* (i)  : size of filtering                          */
    Word16 mem[],      /* (i/o): memory associated with this filtering.     */
    Word16 update      /* (i)  : 0=no update, 1=update of memory.           */
);

Word32 Pow2 (          /* (o) : result       (range: 0<=val<=0x7fffffff)    */
    Word16 exponent,   /* (i) : Integer part.      (range: 0<=val<=30)      */
    Word16 fraction    /* (i) : Fractional part.  (range: 0.0<=val<1.0)     */
);

void Lsf_wt (
    Word16 *lsf,         /* input : LSF vector                  */
    Word16 *wf);         /* output: square of weighting factors */
 
Word32 sqrt_l_exp (/* o : output value,                        Q31 */
    Word32 L_x,    /* i : input value,                         Q31 */
    Word16 *exp    /* o : right shift to be applied to result, Q0  */
);

int sid_sync_init (sid_syncState **st);
int sid_sync_reset (sid_syncState *st);
int sid_sync_set_handover_debt (sid_syncState *st, /* i/o: sid_sync state  */
                                Word16 debtFrames);
void sid_sync(sid_syncState *st , /* i/o: sid_sync state      */
              Word16 mode,
              Word16 *tx_frame_type); 


 
int Pre_Process (
    Pre_ProcessState *st,
    Word16 signal[],   /* Input/output signal                               */
    Word16 lg          /* Lenght of signal                                  */
);

void Prm2bits (
    Word16 mode,    /* i : AMR mode */
    Word16 prm[],      /* input : analysis parameters                       */
    Word16 bits[]      /* output: serial bits                               */
);


//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

void Dec_gain(
    gc_predState *pred_state, /* i/o: MA predictor state           */
    Word16 mode,           /* i  : AMR mode                     */
    Word16 index,             /* i  : index of quantization.       */
    Word16 code[],            /* i  : Innovative vector.           */
    Word16 evenSubfr,         /* i  : Flag for even subframes      */     
    Word16 * gain_pit,        /* o  : Pitch gain.                  */
    Word16 * gain_cod         /* o  : Code gain.                   */
);

void decode_2i40_9bits(
    Word16 subNr,  /* i : subframe number                          */
    Word16 sign,   /* i : signs of 2 pulses.                       */
    Word16 index,  /* i : Positions of the 2 pulses.               */
    Word16 cod[]   /* o : algebraic (fixed) codebook excitation    */
);

void decode_2i40_11bits(
    Word16 sign,   /* i : signs of 2 pulses.                       */
    Word16 index,  /* i : Positions of the 2 pulses.               */
    Word16 cod[]   /* o : algebraic (fixed) codebook excitation    */
);

void decode_3i40_14bits(
    Word16 sign,   /* i : signs of 3 pulses.                       */
    Word16 index,  /* i : Positions of the 3 pulses.               */
    Word16 cod[]   /* o : algebraic (fixed) codebook excitation    */
);


int agc (Word16 *st,      /* i/o : agc state                         */
    Word16 *sig_in,    /* i   : postfilter input signal, (l_trm)  */
    Word16 *sig_out,   /* i/o : postfilter output signal, (l_trm) */
    Word16 agc_fac,    /* i   : AGC factor                        */
    Word16 l_trm       /* i   : subframe size                     */
);

void agc2 (
    Word16 *sig_in,    /* i   : postfilter input signal   */
    Word16 *sig_out,   /* i/o : postfilter output signal  */
    Word16 l_trm       /* i   : subframe size             */
);

void Bits2prm (
    Word16 mode,
    Word16 bits[],   /* input : serial bits, (244 + bfi)               */
    Word16 prm[]     /* output: analysis parameters, (57+1 parameters) */
); 


void D_plsf_reset (D_plsfState *st);
int D_plsf_5 (
    D_plsfState *st,  /* i/o: State variables                            */
    Word16 bfi,       /* i  : bad frame indicator (set to 1 if a bad    
                              frame is received)                         */
    Word16 *indice,   /* i  : quantization indices of 5 submatrices, Q0  */
    Word16 *lsp1_q,   /* o  : quantized 1st LSP vector (M)           Q15 */
    Word16 *lsp2_q    /* o  : quantized 2nd LSP vector (M)           Q15 */
);
void D_plsf_3(
    D_plsfState *st,  /* i/o: State struct                               */
    Word16 mode,   /* i  : coder mode                                 */
    Word16 bfi,       /* i  : bad frame indicator (set to 1 if a         */
                      /*      bad frame is received)                     */
    Word16 * indice,  /* i  : quantization indices of 3 submatrices, Q0  */
    Word16 * lsp1_q   /* o  : quantized 1st LSP vector,              Q15 */
);
void Init_D_plsf_3(D_plsfState *st,  /* i/o: State struct                */
         Word16 index      /* i  : past_rq_init[] index [0, 7] */
);

void ec_gain_code_reset (
    ec_gain_codeState *state
);



void ec_gain_code (
    ec_gain_codeState *st,    /* i/o : State struct                     */
    gc_predState *pred_state, /* i/o : MA predictor state               */
    Word16 state,             /* i   : state of the state machine       */
    Word16 *gain_code         /* o   : decoded innovation gain          */
);
void ec_gain_code_update (
    ec_gain_codeState *st,    /* i/o : State struct                     */
    Word16 bfi,               /* i   : flag: frame is bad               */
    Word16 prev_bf,           /* i   : flag: previous frame was bad     */
    Word16 *gain_code         /* i/o : decoded innovation gain          */
);

int ec_gain_pitch_init (
    ec_gain_pitchState **state
);

void ec_gain_pitch_reset (
    ec_gain_pitchState *state
);



void ec_gain_pitch (
    ec_gain_pitchState *st, /* i/o : state variables                   */
    Word16 state,           /* i   : state of the state machine        */
    Word16 *gain_pitch      /* o   : pitch gain (Q14)                  */
);

void ec_gain_pitch_update (
    ec_gain_pitchState *st, /* i/o : state variables                   */
    Word16 bfi,             /* i   : flag: frame is bad               */
    Word16 prev_bf,         /* i   : flag: previous frame was bad     */
    Word16 *gain_pitch      /* i/o : pitch gain                        */
);


Word16 Cb_gain_average (
   Cb_gain_averageState *st, /* i/o : State variables for CB gain avergeing   */
   Word16 mode,           /* i   : AMR mode                                */
   Word16 gain_code,         /* i   : CB gain                              Q1 */
   Word16 lsp[],             /* i   : The LSP for the current frame       Q15 */
   Word16 lspAver[],         /* i   : The average of LSP for 8 frames     Q15 */
   Word16 bfi,               /* i   : bad frame indication flag               */
   Word16 prev_bf,           /* i   : previous bad frame indication flag      */
   Word16 pdfi,              /* i   : potential degraded bad frame ind flag   */
   Word16 prev_pdf,          /* i   : prev pot. degraded bad frame ind flag   */
   Word16 inBackgroundNoise, /* i   : background noise decision               */
   Word16 voicedHangover     /* i   : # of frames after last voiced frame     */
);

void Lsp_Az (
    Word16 lsp[],      /* (i)    : line spectral frequencies                */
    Word16 a[]         /* (o)    : predictor coefficients (order = 10)      */
);

int Speech_Encode_Frame_init (Speech_Encode_FrameState **st,
                              Flag dtx,
                              char *id);
void Speech_Encode_Frame_reset (Speech_Encode_FrameState *st);

int Speech_Encode_Frame (
    Speech_Encode_FrameState *st, /* i/o : encoder states         */
    Word16 *new_speech,           /* i   : input speech           */
    Word16 *serial,Word16 amr_mode               /* o   : serial bit stream      */
);

Word16 PackBits(
    Word16 used_mode,       /* i : actual AMR mode             */
    Word16 mode,            /* i : requested AMR (speech) mode */
    Word16 fr_type,  /* i : frame type                  */
    Word16 bits[],             /* i : serial bits                 */
    UWord8 packed_bits[]       /* o : sorted&packed bits          */
);



void lsp_avg (
    lsp_avgState *st,     /* i/o : State struct                 Q15 */
    Word16 *lsp           /* i   : LSP vector                   Q15 */
);



Word16 Bgn_scd (Bgn_scdState *st,      /* i : State variables for bgn SCD         */
                Word16 ltpGainHist[],  /* i : LTP gain history                    */
                Word16 speech[],       /* o : synthesis speech frame              */
                Word16 *voicedHangover /* o : # of frames after last voiced frame */
);
 

void ph_disp_lock (ph_dispState *state);
void ph_disp_release (ph_dispState *state);

void ph_disp (
      ph_dispState *state, /* i/o     : State struct                     */
      Word16 mode,      /* i       : codec mode                       */
      Word16 x[],          /* i/o Q0  : in:  LTP excitation signal       */
                           /*           out: total excitation signal     */
      Word16 cbGain,       /* i   Q1  : Codebook gain                    */
      Word16 ltpGain,      /* i   Q14 : LTP gain                         */
      Word16 inno[],       /* i   Q13 : Innovation vector (Q12 for 12.2) */
      Word16 pitch_fac,    /* i   Q14 : pitch factor used to scale the
                                        LTP excitation (Q13 for 12.2)    */
      Word16 tmp_shift     /* i   Q0  : shift factor applied to sum of   
                                        scaled LTP ex & innov. before
                                        rounding                         */
);

void Dec_lag3(Word16 index,     /* i : received pitch index                 */
              Word16 T0_min,    /* i : minimum of search range              */
              Word16 T0_max,    /* i : maximum of search range              */
              Word16 i_subfr,   /* i : subframe flag                        */
              Word16 T0_prev,   /* i : integer pitch delay of last subframe
                                       used in 2nd and 4th subframes        */
              Word16 * T0,      /* o : integer part of pitch lag            */ 
              Word16 * T0_frac, /* o : fractional part of pitch lag         */
              Word16 flag4      /* i : flag for encoding with 4 bits        */
              );

void Dec_lag6 (
    Word16 index,      /* input : received pitch index           */
    Word16 pit_min,    /* input : minimum pitch lag              */
    Word16 pit_max,    /* input : maximum pitch lag              */
    Word16 i_subfr,    /* input : subframe flag                  */
    Word16 *T0,        /* in/out: integer part of pitch lag      */
    Word16 *T0_frac    /* output: fractional part of pitch lag   */
);

Word16 d_gain_pitch (      /* return value: gain (Q14)                */
    Word16 mode,        /* i : AMR mode                            */
    Word16 index           /* i   : index of quantization             */
);
void d_gain_code (
    gc_predState *pred_state, /* i/o : MA predictor state               */
    Word16 mode,           /* i   : AMR mode                         */
    Word16 index,             /* i   : received quantization index      */
    Word16 code[],            /* i   : innovation codevector            */
    Word16 *gain_code         /* o   : decoded innovation gain          */
);
 

int preemphasis (
    preemphasisState *st, /* (i/o): preemphasis filter state                  */
    Word16 *signal,    /* (i/o): input signal overwritten by the output     */
    Word16 g,          /* (i)  : preemphasis coefficient                    */
    Word16 L           /* (i)  : size of filtering                          */
);

Word16 pseudonoise (
    Word32 *shift_reg, /* i/o : Old CN generator shift register state */
    Word16 no_bits     /* i   : Number of bits                        */
);

void build_CN_code (
    Word32 *seed,         /* i/o : Old CN generator shift register state */
    Word16 cod[]          /* o   : Generated CN fixed codebook vector    */
);

void build_CN_param (
    Word16 *seed,              /* i/o : Old CN generator shift register state */
    Word16 n_param,            /* i : number of params     */  
    const char param_size_table[], /* i : size of params       */   
    Word16 parm[]              /* o   : CN Generated Params*/
);


void Post_Filter_reset (Post_FilterState *st);
int Post_Filter (
    Post_FilterState *st, /* i/o : post filter states                        */
    Word16 mode,       /* i   : AMR mode                                  */
    Word16 *syn,          /* i/o : synthesis speech (postfiltered is output) */
    Word16 *Az_4          /* i   : interpolated LPC parameters in all subfr. */
);

void Decoder_amr_reset (Decoder_amrState *st,Word16 mode);


int Decoder_amr (
    Decoder_amrState *st,  /* i/o : State variables                       */
    Word16 parm[],         /* i   : vector of synthesis parameters
                                    (PRM_SIZE)                            */
    Word16 synth[],        /* o   : synthesis speech (L_FRAME)            */
    Word16 A_t[]           /* o   : decoded LP filter in 4 subframes
                                    (AZ_SIZE)                             */
);

void GetLibVersion(char **str);


int Speech_Decode_Frame_reset (Speech_Decode_FrameState *st);

int Speech_Decode_Frame (
    Speech_Decode_FrameState *st, /* io: post filter states                */
    Word16 *serial,               /* i : serial bit stream                 */
    Word16 *synth                 /* o : synthesis speech (postfiltered    */
                                  /*     output)                           */
);


Word16 UnpackBits (
    Word8  q,              /* i : Q-bit (i.e. BFI)        */
   Word16 ft,             /* i : frame type (i.e. mode)  */
    UWord8 packed_bits[],  /* i : sorted & packed bits    */
   Word16 *mode,       /* o : mode information        */
    Word16 bits[]          /* o : serial bits             */
);



int Post_Process (
    Post_ProcessState *st,  /* i/o : post process state                   */
    Word16 signal[],        /* i/o : signal                               */
    Word16 lg               /* i   : lenght of signal                     */
    );
void dec_10i40_35bits (
    Word16 index[],    /* (i)   : index of 10 pulses (sign+position)        */
    Word16 cod[]       /* (o)   : algebraic (fixed) codebook excitation     */
);

void decode_4i40_17bits(
    Word16 sign,   /* i : signs of 4 pulses.                       */
    Word16 index,  /* i : Positions of the 4 pulses.               */
    Word16 cod[]   /* o : algebraic (fixed) codebook excitation    */
);

void Int_lsf (
    Word16 lsf_old[], /* i : LSF vector at the 4th SF of past frame          */
    Word16 lsf_new[], /* i : LSF vector at the 4th SF of present frame       */
    Word16 i_subfr,   /* i : Pointer to current sf (equal to 0,40,80 or 120) */
    Word16 lsf_out[]  /* o : interpolated LSF parameters for current sf      */
);
Word16 Ex_ctrl (Word16 excitation[],   /*i/o: Current subframe excitation   */
                Word16 excEnergy,      /* i : Exc. Energy, sqrt(totEx*totEx)*/
                Word16 exEnergyHist[], /* i : History of subframe energies  */
                Word16 voicedHangover, /* i : # of fr. after last voiced fr.*/
                Word16 prevBFI,        /* i : Set i previous BFI            */
                Word16 carefulFlag     /* i : Restrict dymamic in scaling   */
);

Word16 decoder_homing_frame_test (Word16 input_frame[], Word16 mode);
Word16 decoder_homing_frame_test_first (Word16 input_frame[], Word16 mode);
void A_Refl(
   Word16 a[],       /* i   : Directform coefficients */
   Word16 refl[]      /* o   : Reflection coefficients */
);

void dec_8i40_31bits (
    Word16 index[],    /* i : index of 8 pulses (sign+position)         */
    Word16 cod[]       /* o : algebraic (fixed) codebook excitation     */
);




void cod_amr_init (cod_amrState *s, Flag dtx);
void cod_amr_reset (cod_amrState *st);
int cod_amr_first(cod_amrState *st,     /* i/o : State struct            */
                  Word16 new_speech[]   /* i   : speech input (L_FRAME)  */
);
int cod_amr(cod_amrState *st,         /* i/o : State struct                 */
            Word16 new_speech[],      /* i   : speech input (L_FRAME)       */
            Word16 ana[],             /* o   : Analysis parameters          */
            Word16 synth[]           /* o   : Local synthesis              */
);

Word16 encoder_homing_frame_test (Word16 input_frame[]);

void MagicPixel_AMR_SRAM_Init(char *start, int size);
void MagicPixel_AMRENC_SRAM_Init(void);
int MagicPixel_AMRDEC_Init(void);
int MagicPixel_AMRENC_Init(Flag dtx);
Word16 MagicPixel_AMRENC_Encode(Word16 amr_mode,Word16 **serial,Word16 *new_speech,UWord8 **packeted_bits);
void MagicPixel_AMRDEC_decode(int toc,UWord8*packed_bits,Word16 *synth);

#endif
