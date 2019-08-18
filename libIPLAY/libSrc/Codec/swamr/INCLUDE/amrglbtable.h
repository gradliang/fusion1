/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2005, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
* Filename:    amrglbtable.h
*
* Programmer:    Honda Hsu
*                MPX E310 division
*
* Created: 08/10/2005
*
* Description: define external variable for removing table to scratch RAM
*        
* Change History (most recent first):
*     <1>     08/10/2005    Honda Hsu  first file
****************************************************************
*/
extern Word16 *dico1_lsf_3;
extern Word16 *dico2_lsf_3;
extern Word16 *mr515_3_lsf;
extern Word16 *table_gain_lowrates;
extern Word16 *log2_table;
extern Word16 *lsp_lsf_table;
extern Word16 *lsp_lsf_slope;
extern Word16 *pow2_table;
extern Word16 *lsp_init_data;
extern Word16 *sqrt_l_table;
extern Word16 * inv_sqrt_table;
extern Word16 *ph_imp_low;
extern Word16 *ph_imp_mid;
extern Word16 *grid;
extern Word16 *inter_36_inter_6;
extern Word16 *pred_lt_inter_6;  
extern Word16 *lag_h;   
extern Word16 *lag_l; 
extern Word16 *gray;       
extern Word16 *dgray;     
extern Word16 *startPos;
extern Word16 *startPos1;    
extern Word16 *startPos2;    
extern Word16 *gamma1;
extern Word16 *gamma2;
extern Word16 *trackTable;
extern Word16 *pred;    
extern Word16 *table_gain_MR475;
extern Word16 *window_200_40;   
extern Word16 *gamma1_12k2;  
extern Word16 *q_plsf_3_mean_lsf;
extern Word16 *pred_fac;
extern Word16 *past_rq_init;
extern Word16 *table_gain_highrates;
extern Word16 *dico3_lsf_3;
extern Word16 *corrweight;
extern Word16 *qua_gain_pitch;
extern Word16 *qua_gain_code;
extern Word16 *ph_imp_low_MR795; 
extern Word16 *ph_imp_mid_MR795;
extern Word16 *mr795_1_lsf;
extern Word16 *mean_lsf;
extern Word16 *dico1_lsf;
extern Word16 *dico2_lsf;
extern Word16 *dico3_lsf;
extern Word16 *dico4_lsf;
extern Word16 *dico5_lsf;


extern const Word16 window_160_80[];
extern const Word16 window_232_8[];


extern const char prmno[];
extern const char prmnofsf[];
extern const char bitno_MR475[];
extern const char bitno_MR515[];
extern const char bitno_MR59[];
extern const char bitno_MR67[];
extern const char bitno_MR74[];
extern const char bitno_MR795[];
extern const char bitno_MR102[];
extern const char bitno_MR122[];
extern const char bitno_MRDTX[];
extern const char *bitno[];


extern UWord8 toc_byte[];
extern Word16 unpacked_size[];
extern Word16 packed_size[];
extern Word16 unused_size[];
extern unsigned char sort_475[];
extern unsigned char sort_515[];
extern unsigned char sort_59[];
extern unsigned char sort_67[];
extern unsigned char sort_74[];
extern unsigned char sort_795[];
extern unsigned char sort_102[];
extern unsigned char sort_122[];
extern unsigned char sort_SID[];
extern unsigned char *sort_ptr[];







    //remove all table to "CommonTable.tab"


extern const Word16 dhf_MR475[PRMNO_MR475];
extern const Word16 dhf_MR515[PRMNO_MR515];
extern const Word16 dhf_MR59[PRMNO_MR59];
extern const Word16 dhf_MR67[PRMNO_MR67];
extern const Word16 dhf_MR74[PRMNO_MR74];
extern const Word16 dhf_MR795[PRMNO_MR795];
extern const Word16 dhf_MR102[PRMNO_MR102];
extern const Word16 dhf_MR122[PRMNO_MR122];
extern const Word16 *dhf[];





extern Word16 serial_buf[SERIAL_FRAMESIZE];


