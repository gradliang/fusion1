#ifndef dtx_function_include
#define dtx_function_include
 

#ifdef dohoming
void dtx_enc_reset (dtx_encState *st);
#endif 

void dtx_enc_exit (dtx_encState **st);
 

#if OPT_DTX
int dtx_enc(dtx_encState *st,        /* i/o : State struct                    */
            Word16 computeSidFlag,   /* i   : compute SID                     */
       Word16 *qSt,        /* i/o : Qunatizer state struct          */
            gc_predState* predState, /* i/o : State struct                    */
       Word16 **anap            /* o   : analysis parameters             */
       );
#endif

int dtx_buffer(dtx_encState *st,   /* i/o : State struct                    */
          Word16 lsp_new[],   /* i   : LSP vector                      */
          Word16 speech[]     /* i   : speech samples                  */
          );

Word16 tx_dtx_handler(dtx_encState *st,       /* i/o : State struct          */
                      Word16 vadFlag,         /* i   : vad control variable  */
                      Word16 *usedMode     /* o   : mode changed or not   */
                      );


Word16   vad2 (Word16 *farray_ptr, vadState2 *st);
int   vad2_init (vadState2 **st);
int   vad2_reset (vadState2 *st);

void  r_fft (Word16 *farray_ptr);
void  LTP_flag_update (vadState2 *st, Word16 mode);


#if OPT_DTX

void vad_complex_detection_update (vadState1 *st,      /* i/o : State struct     */
                                   Word16 best_corr_hp /* i   : best Corr Q15    */
                                   );

void vad_tone_detection (vadState1 *st, /* i/o : State struct            */
                         Word32 t0,     /* i   : autocorrelation maxima  */
                         Word32 t1      /* i   : energy                  */
                         );

void vad_tone_detection_update (
                vadState1 *st,             /* i/o : State struct              */
                Word16 one_lag_per_frame   /* i   : 1 if one open-loop lag is
                                              calculated per each frame,
                                              otherwise 0                     */
                );

void vad_pitch_detection (vadState1 *st,  /* i/o : State struct                  */
                          Word16 lags[]   /* i   : speech encoder open loop lags */
                          );

Word16 vad1 (vadState1 *st,  /* i/o : State struct                      */
            Word16 in_buf[]  /* i   : samples of the input frame 
                                inbuf[159] is the very last sample,
                                incl lookahead                          */
            );

#endif

#ifdef dohoming  
void vad1_reset (vadState1 *st);
#endif

#if OPT_DTX
Word16 hp_max (   
    Word32 corr[],      /* i   : correlation vector.                      */
    Word16 scal_sig[],  /* i   : scaled signal.                           */
    Word16 L_frame,     /* i   : length of frame to compute pitch         */
    Word16 lag_max,     /* i   : maximum lag                              */
    Word16 lag_min,     /* i   : minimum lag                              */
    Word16 *cor_hp_max  /* o   : max high-pass filtered norm. correlation */
    );
#endif



void dtx_dec_reset (dtx_decState *st);


int dtx_dec(
   dtx_decState *st,                /* i/o : State struct                    */
   Word16 mem_syn[],                /* i/o : AMR decoder state               */
   D_plsfState* lsfState,           /* i/o : decoder lsf states              */
   gc_predState* predState,         /* i/o : prediction states               */
   Cb_gain_averageState* averState, /* i/o : CB gain average states          */
   Word16 new_state,     /* i   : new DTX state                   */    
   Word16 mode,                  /* i   : AMR mode                        */
   Word16 parm[],                   /* i   : Vector of synthesis parameters  */
   Word16 synth[],                  /* o   : synthesised speech              */
   Word16 A_t[]                     /* o   : decoded LP filter in 4 subframes*/
   );

void dtx_dec_activity_update(dtx_decState *st,
                             Word16 lsf[],
                             Word16 frame[]);

Word16 rx_dtx_handler(dtx_decState *st,           /* i/o : State struct */
                                 Word16 frame_type /* i   : Frame type   */
                                 );




































































#endif

