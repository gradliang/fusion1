/*
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0                
*                                REL-4 Version 4.1.0                
*
*****************************************************************************
*
*      File             : cnst.h
*      Purpose          : Speech codec constant parameters
*                       :  (encoder, decoder, and postfilter)
*
*****************************************************************************
*/
#ifndef cnst_h
#define cnst_h  
//"$Id $"

#define L_TOTAL      320       /* Total size of speech buffer.             */
#define L_WINDOW     240       /* Window size in LP analysis               */
#define L_FRAME      160       /* Frame size                               */
#define L_FRAME_BY2  80        /* Frame size divided by 2                  */
#define L_SUBFR      40        /* Subframe size                            */
#define L_CODE       40        /* codevector length                        */
#define NB_TRACK     5         /* number of tracks                         */
#define STEP         5         /* codebook step size                       */
#define NB_TRACK_MR102  4      /* number of tracks mode mr102              */
#define STEP_MR102      4      /* codebook step size mode mr102            */
#define M            10        /* Order of LP filter                       */
#define MP1          (M+1)     /* Order of LP filter + 1                   */
#define LSF_GAP      205       /* Minimum distance between LSF after quan-
                                  tization; 50 Hz = 205                    */
#define LSP_PRED_FAC_MR122 21299 /* MR122 LSP prediction factor (0.65 Q15) */
#define AZ_SIZE       (4*M+4)  /* Size of array of LP filters in 4 subfr.s */
#define PIT_MIN_MR122 18       /* Minimum pitch lag (MR122 mode)           */
#define PIT_MIN       20       /* Minimum pitch lag (all other modes)      */
#define PIT_MAX       143      /* Maximum pitch lag                        */
#define L_INTERPOL    (10+1)   /* Length of filter for interpolation       */
#define L_INTER_SRCH  4        /* Length of filter for CL LTP search
                                  interpolation                            */
        
#define MU       26214         /* Factor for tilt compensation filter 0.8  */
#define AGC_FAC  29491         /* Factor for automatic gain control 0.9    */
        
#define L_NEXT       40        /* Overhead in LP analysis                  */
#define SHARPMAX  13017        /* Maximum value of pitch sharpening        */
#define SHARPMIN  0            /* Minimum value of pitch sharpening        */
                                                                          
                                                                          
#define MAX_PRM_SIZE    57     /* max. num. of params                      */
#define MAX_SERIAL_SIZE 244    /* max. num. of serial bits                 */
                                                                          
#define GP_CLIP   15565        /* Pitch gain clipping = 0.95               */
#define N_FRAME   7            /* old pitch gains in average calculation   */

#define EHF_MASK 0x0008        /* encoder homing frame pattern             */


#define MR475  0
#define MR515  1         
#define MR59   2
#define MR67   3
#define MR74   4
#define MR795  5
#define MR102  6
#define MR122  7        
#define MRDTX  8
#define N_MODES   9

#define BIT_0      0
#define BIT_1      1

#define PRMNO_MR475 17
#define PRMNO_MR515 19
#define PRMNO_MR59  19
#define PRMNO_MR67  19
#define PRMNO_MR74  19
#define PRMNO_MR795 23
#define PRMNO_MR102 39
#define PRMNO_MR122 57
#define PRMNO_MRDTX 5

/* number of parameters to first subframe */
#define PRMNOFSF_MR475 7
#define PRMNOFSF_MR515 7
#define PRMNOFSF_MR59  7
#define PRMNOFSF_MR67  7
#define PRMNOFSF_MR74  7
#define PRMNOFSF_MR795 8
#define PRMNOFSF_MR102 12
#define PRMNOFSF_MR122 18


#define NB_QUA_PITCH 16
#define NB_QUA_CODE 32
#define PAST_RQ_INIT_SIZE 8

#define MR515_3_SIZE  128
#define MR795_1_SIZE  512


#define MR475_VQ_SIZE 256

#define VQ_SIZE_HIGHRATES 128
/* table used in 'low' rates: MR475, MR515, MR59 */
#define VQ_SIZE_LOWRATES 64


#define RX_SPEECH_GOOD     0
#define RX_SPEECH_DEGRADED 1
#define RX_ONSET        2
#define RX_SPEECH_BAD      3
#define RX_SID_FIRST    4
#define RX_SID_UPDATE      5
#define RX_SID_BAD         6
#define RX_NO_DATA         7
#define RX_N_FRAMETYPES    8

#define TX_SPEECH_GOOD     0
#define TX_SID_FIRST    1
#define TX_SID_UPDATE      2
#define TX_NO_DATA         3
#define TX_SPEECH_DEGRADED 4
#define TX_SPEECH_BAD      5
#define TX_SID_BAD         6
#define TX_ONSET        7
#define TX_N_FRAMETYPES    8






#define SERIAL_FRAMESIZE (1+MAX_SERIAL_SIZE+5)
#define AMR_MAGIC_NUMBER "#!AMR\n"
#define MAX_PACKED_SIZE (MAX_SERIAL_SIZE / 8 + 2)




#endif
