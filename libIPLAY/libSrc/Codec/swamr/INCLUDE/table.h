/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2005, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
* Filename:    Table.h
*
* Programmer:    Honda Hsu
*                MPX E310 division
*
* Created: 08/10/2005
*
* Description: Header file for table.c
*        
* Change History (most recent first):
*     <1>     08/10/2005    Honda Hsu  first file
****************************************************************
*/
#define len_dico1_lsf_3 768  //Word16
#define len_dico2_lsf_3 1536  //Word16
#define len_mr515_3_lsf 512  //Word16
#define len_table_gain_lowrates 256 //Word16
#define len_log2_table 33 //Word16
#define len_lsp_lsf_table 65 //Word16
#define len_lsp_lsf_slope 64 //Word16
#define len_pow2_table 33 //Word16
#define len_lsp_init_data M //Word16
#define len_sqrt_l_table 49 //Word16
#define len_inv_sqrt_table 49 //Word16
#define len_ph_imp_low 40 //Word16
#define len_ph_imp_mid 40 //Word16
#define len_grid 61 //Word16
#define len_inter_36_inter_6 25 //Word16, depend on  UP_SAMP_MAX, L_INTER_SRCH
#define len_pred_lt_inter_6 61 //Word16, depend on  UP_SAMP_MAX, L_INTER10
#define len_lag_h 10 //Word16
#define len_lag_l 10 //Word16
#define len_gray  8   //Word16
#define len_dgray 8   //Word16
#define len_startPos 16   //Word16
#define len_startPos1 2 //Word16  
#define len_startPos2 4 //Word16
#define len_gamma1 M //Word16   
#define len_gamma2 M //Word16
#define len_trackTable 20 //Word16
#define len_pred 4 //Word16
#define len_table_gain_MR475 1024 //Word16
#define len_window_200_40 L_WINDOW  //Word16


void InitTable();
void RemoveTable();
void RemoveTable_ENC515();
void RemoveTable_ENC102();
unsigned long sram_alloc_NoClearSRAM(unsigned long size);


void RemoveWord16(
  Word16 *dest,      //(i/o) destination address
  Word16 src[],         //(i)source address
  Word32 length    //(i)remove length 
);

void RemoveConstWord16(
  Word16 *dest,      //(i/o) destination address
  const Word16 src[],         //(i)source address
  Word32 length    //(i)remove length 
);

void LoadDecTable(Speech_Decode_FrameState *state, int mode);





