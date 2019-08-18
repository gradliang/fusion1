#ifndef dtx_structure_include
#define dtx_structure_include




#define DTX_HIST_SIZE 8
#define DTX_ELAPSED_FRAMES_THRESH (24 + 7 -1)
#define DTX_HANG_CONST 7             /* yields eight frames of SP HANGOVER  */

/*
********************************************************************************
*                         DEFINITION OF DATA TYPES
********************************************************************************
*/
typedef struct {
   Word16 lsp_hist[M * DTX_HIST_SIZE];
   Word16 log_en_hist[DTX_HIST_SIZE];
   Word16 hist_ptr;
   Word16 log_en_index;
   Word16 init_lsf_vq_index;
   Word16 lsp_index[3];

   /* DTX handler stuff */
   Word16 dtxHangoverCount;
   Word16 decAnaElapsedCount;

} dtx_encState;

/* state variable */
typedef struct {
   
   Word16 bckr_est[COMPLEN];    /* background noise estimate                */
   Word16 ave_level[COMPLEN];   /* averaged input components for stationary */
                                /*    estimation                            */
   Word16 old_level[COMPLEN];   /* input levels of the previous frame       */
   Word16 sub_level[COMPLEN];   /* input levels calculated at the end of
                                      a frame (lookahead)                   */
   Word16 a_data5[3][2];        /* memory for the filter bank               */
   Word16 a_data3[5];           /* memory for the filter bank               */

   Word16 burst_count;          /* counts length of a speech burst          */
   Word16 hang_count;           /* hangover counter                         */
   Word16 stat_count;           /* stationary counter                       */

   /* Note that each of the following three variables (vadreg, pitch and tone)
      holds 15 flags. Each flag reserves 1 bit of the variable. The newest
      flag is in the bit 15 (assuming that LSB is bit 1 and MSB is bit 16). */
   Word16 vadreg;               /* flags for intermediate VAD decisions     */
   Word16 pitch;                /* flags for pitch detection                */
   Word16 tone;                 /* flags for tone detection                 */
   Word16 complex_high;         /* flags for complex detection              */
   Word16 complex_low;          /* flags for complex detection              */

   Word16 oldlag_count, oldlag; /* variables for pitch detection            */
 
   Word16 complex_hang_count;   /* complex hangover counter, used by VAD    */
   Word16 complex_hang_timer;   /* hangover initiator, used by CAD          */
    
   Word16 best_corr_hp;         /* FIP filtered value Q15                   */ 

   Word16 speech_vad_decision;  /* final decision                           */
   Word16 complex_warning;      /* complex background warning               */

   Word16 sp_burst_count;       /* counts length of a speech burst incl
                                   HO addition                              */
   Word16 corr_hp_fast;         /* filtered value                           */ 
} vadState1;

#define     YES      1
#define     NO    0
#define     ON    1
#define     OFF      0
#define     TRUE     1
#define     FALSE    0

#define         FRM_LEN                 80
#define         DELAY                   24
#define         FFT_LEN                 128

#define         NUM_CHAN                16
#define         LO_CHAN                 0
#define         HI_CHAN                 15

#define         UPDATE_THLD             35
#define         HYSTER_CNT_THLD         6
#define         UPDATE_CNT_THLD         50

#define     SHIFT_STATE_0     0     /* channel energy scaled as 22,9 */
#define     SHIFT_STATE_1     1     /* channel energy scaled as 27,4 */

#define     NOISE_FLOOR_CHAN_0   512      /* 1.0    scaled as 22,9 */
#define     MIN_CHAN_ENRG_0      32    /* 0.0625 scaled as 22,9 */
#define     MIN_NOISE_ENRG_0  32    /* 0.0625 scaled as 22,9 */
#define     INE_NOISE_0    8192     /* 16.0   scaled as 22,9 */
#define     FRACTIONAL_BITS_0 9     /* used as input to fn10Log10() */

#define     NOISE_FLOOR_CHAN_1   16    /* 1.0    scaled as 27,4 */
#define     MIN_CHAN_ENRG_1      1     /* 0.0625 scaled as 27,4 */
#define     MIN_NOISE_ENRG_1  1     /* 0.0625 scaled as 27,4 */
#define     INE_NOISE_1    256      /* 16.0   scaled as 27,4 */
#define     FRACTIONAL_BITS_1 4     /* used as input to fn10Log10() */

#define     STATE_1_TO_0_SHIFT_R (FRACTIONAL_BITS_1-FRACTIONAL_BITS_0)  /* state correction factor */
#define     STATE_0_TO_1_SHIFT_R (FRACTIONAL_BITS_0-FRACTIONAL_BITS_1)  /* state correction factor */

#define         HIGH_ALPHA              29491      /* 0.9 scaled as 0,15 */
#define         LOW_ALPHA               22938      /* 0.7 scaled as 0,15 */
#define         ALPHA_RANGE             (HIGH_ALPHA - LOW_ALPHA)
#define         DEV_THLD                7168    /* 28.0 scaled as 7,8 */

#define         PRE_EMP_FAC             (-26214)   /* -0.8 scaled as 0,15 */

#define         CEE_SM_FAC              18022      /* 0.55 scaled as 0,15 */
#define         ONE_MINUS_CEE_SM_FAC    14746      /* 0.45 scaled as 0,15 */

#define         CNE_SM_FAC              3277    /* 0.1 scaled as 0,15 */
#define         ONE_MINUS_CNE_SM_FAC    29491      /* 0.9 scaled as 0,15 */

#define         FFT_HEADROOM            2


typedef struct
{
   Word16 pre_emp_mem;
   Word16 update_cnt;
   Word16 hyster_cnt;
   Word16 last_update_cnt;
   Word16 ch_enrg_long_db[NUM_CHAN];   /* scaled as 7,8  */

   Word32 Lframe_cnt;
   Word32 Lch_enrg[NUM_CHAN]; /* scaled as 22,9 or 27,4 */
   Word32 Lch_noise[NUM_CHAN];   /* scaled as 22,9 */

   Word16 last_normb_shift;   /* last block norm shift count */

   Word16 tsnr;         /* total signal-to-noise ratio in dB (scaled as 7,8) */
   Word16 hangover;
   Word16 burstcount;
   Word16 fupdate_flag;    /* forced update flag from previous frame */
   Word16 negSNRvar;    /* Negative SNR variance (scaled as 7,8) */
   Word16 negSNRbias;      /* sensitivity bias from negative SNR variance (scaled as 15,0) */

   Word16 shift_state;     /* use 22,9 or 27,4 scaling for ch_enrg[] */

   Word32 L_R0;
   Word32 L_Rmax;
   Flag   LTP_flag;     /* Use to indicate the the LTP gain is > LTP_THRESH */

} vadState2;

#ifndef  VAD2
#define vadState vadState1
#else
#define vadState vadState2
#endif
#endif

#define SPEECH    0
#define DTX       1
#define DTX_MUTE  2

#define DTX_MAX_EMPTY_THRESH 50

/*
*****************************************************************************
*                         DEFINITION OF DATA TYPES
*****************************************************************************
*/

typedef struct {
   Word16 since_last_sid;
   Word16 true_sid_period_inv;
   Word16 log_en;
   Word16 old_log_en;
   Word32 L_pn_seed_rx; 
   Word16 lsp[M];
   Word16 lsp_old[M]; 
   
   Word16 lsf_hist[M*DTX_HIST_SIZE];
   Word16 lsf_hist_ptr;
   Word16 lsf_hist_mean[M*DTX_HIST_SIZE]; 
   Word16 log_pg_mean;
   Word16 log_en_hist[DTX_HIST_SIZE];
   Word16 log_en_hist_ptr;

   Word16 log_en_adjust;

   Word16 dtxHangoverCount;
   Word16 decAnaElapsedCount;

   Word16 sid_frame;       
   Word16 valid_data;          
   Word16 dtxHangoverAdded;
 
   Word16 dtxGlobalState;     /* contains previous state */
                                         /* updated in main decoder */ 

   Word16 data_updated;      /* marker to know if CNI data is ever renewed */ 

} dtx_decState;
