/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      codec.h
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: 
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Joshua Lu    first file
****************************************************************
*/

#ifndef __CODEC_H__
#define __CODEC_H__


//SRAM GROUP STATUS
#define SRMGP_CH1_DMA_IDLE  	0x000000
#define SRMGP_CH1_DMA_READ 	0x010000
#define SRMGP_CH1_DMA_WRITE 	0x020000
#define SRMGP_CH1_DMA_RD_WAIT 0x030000
#define SRMGP_CH1_DMA_END		0x100000
#define SRMGP_CH0_DMA_IDLE		0x000000
#define SRMGP_CH0_DMA_READ		0x000001
#define SRMGP_CH0_DMA_WRITE 	0x000002
#define SRMGP_CH0_DMA_RD_WAIT 0x000003
#define SRMGP_CH0_DMA_END		0x000010

// mp3aac main sequnecer state definition
// The mp3aac status register contains one of this values below:
#define  MPA_SEQ_IDLE          0
#define  MPA_SEQ_CH0_FILL      1
#define  MPA_SEQ_CH1_FILL      2
#define  MPA_SEQ_CH0_WAIT      3
#define  MPA_SEQ_STDEC         4
#define  MPA_SEQ_CH0_PNS       5
#define  MPA_SEQ_CH1_PNS       6
#define  MPA_SEQ_CH0_TNS       7
#define  MPA_SEQ_CH1_TNS       8
#define  MPA_SEQ_CH0_IMDCT     9
#define  MPA_SEQ_CH1_IMDCT     10
#define  MPA_SEQ_CH0_SFB       11
#define  MPA_SEQ_CH1_SFB       12

// mp3aac error numbers. 
// the firmware returns one of them when write registers failed.
// the application will get this error numbers.
#define  MPA_ERROR_NONE          0
#define  MPA_ERROR_CH0_FILL      1
#define  MPA_ERROR_CH0_WAIT      2
#define  MPA_ERROR_CH1_FILL      3
#define  MPA_ERROR_STDEC         4
#define  MPA_ERROR_CH0_PNS       5
#define  MPA_ERROR_CH1_PNS       6
#define  MPA_ERROR_CH0_TNS       7
#define  MPA_ERROR_CH1_TNS       8
#define  MPA_ERROR_CH0_IMDCT     9
#define  MPA_ERROR_CH1_IMDCT     10
#define  MPA_ERROR_CH0_SFB       11
#define  MPA_ERROR_CH1_SFB       12

//Control register's bits defination
#define MPA_CTRL_START   0x1
#define MPA_CTRL_ENABLE  0x10
#define MPA_CTRL_RESET   0x100
#define MPA_CTRL_GRANULE 0x1000


/* --- hardware interface --- */
#define DMA_OP					0x00000001L
#define DMA_CYC					0x00000002L
#define DMA_LC					0x00000004L
#define DMA_DBL					0x00000008L
#define DMA_DEC					0x00000010L
#define DMA_LC_DEC				0x00000020L


//CTRL_REG
#define SW_RESET	0x0L
#define SOF			0x1L 
#define SEL_IQ_TBL	0x2L
//

//PARA_REG
#define MB_H_E			0
#define MB_H_S			5
#define MB_V_E			8
#define MB_V_S			13
#define MPEG4_B			16
#define QUANT_TYPE_E	17
#define QUANT_TYPE_S	18
#define ALTER_ZIGZAG_B	19
#define SHORT_HEADER_B	20
#define DIVX3_B			21

//STATUS_REG
#define	COE_FIFO_FULL		0
#define COE_FIFO_EMPTY	1	
#define MV_FIFO_FULL		2
#define MV_FIFO_EMPTY		3	
#define PIXEL_FIFO_FULL	4	
#define PIXEL_FIFO_EMPTY	5
#define REF_FIFO_FULL		6	
#define REF_FIFO_EMPTY	7	
#define MV_WR_EN			8 
#define COE_WR_EN			9  

//COE_FIFO_REG  :MB COEFF DATA
#define COEFF_LAST_B	18
#define COEFF_RUN_E		12	
#define COEFF_RUN_S		17
#define COEFF_LEVEL_E	0
#define COEFF_LEVEL_S	11

//COE_FIFO_REG	:MB PARAM
#define ACPRED_FLAG_B 	0
#define QUANT_E 		1
#define QUANT_S			6
#define INTER_FLAG_B	7	
#define NO_CODED_E		8
#define NO_CODED_S		13
#define RESYNC_B		14

//MV_FIFO_REG	:MV_PARAM
#define FW_MC_B			0
#define BW_MC_B			1
#define ROUNDING_B		31

#define MB_POS_Y_S		31
#define MB_POS_Y_E		16

#define MB_POS_X_S		15
#define MB_POS_X_E		0

#define MVP_Y_S		31
#define MVP_Y_E		16

#define MVP_X_S		15
#define MVP_X_E		0





#define GETBIT(x,y) (((x)&(1<<(y)))>>(y)) 
#define GETBITS(x,s,e) (((x)&((UF<<((s)+1))^(UF<<(e))))>>(e))

#define GET_REG_BIT(reg,bit)		(((*((int32_t *)(reg)))&(0x1L<<bit))>>bit)
#define SET_REG_BIT_0(reg,bit)		((*((int32_t *)(reg)))&(~(0x1L<<bit)))
#define SET_REG_BIT_1(reg,bit)		((*((int32_t *)(reg)))|(0x1L<<bit))




#define UF 0xffffffff
#define SWAP(_T_,A,B)    { _T_ tmp = A; A = B; B = tmp; }

#define I_FRAME 1
#define P_FRAME 2
#define B_FRAME 3
#define INTER 1
#define INTRA 0


#endif	// __CODEC_H__

