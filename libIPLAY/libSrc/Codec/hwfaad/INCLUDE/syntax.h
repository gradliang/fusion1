/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      syntax.h
*
* Programmer:    Brenda Li
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: 
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Brenda Li    first file
****************************************************************
*/
#ifndef __SYNTAX_H__
#define __SYNTAX_H__

#include "decoder.h"
#include "bits.h"

#define LC         2

/* Bitstream */
#define LEN_SE_ID 3
#define LEN_TAG   4
#define LEN_BYTE  8

#define EXT_FIL            0
#define EXT_FILL_DATA      1
#define EXT_DATA_ELEMENT   2
#define EXT_DYNAMIC_RANGE 11
#define ANC_DATA           0

/* Syntax elements */
#define ID_SCE 0x0
#define ID_CPE 0x1
#define ID_CCE 0x2
#define ID_LFE 0x3
#define ID_DSE 0x4
#define ID_PCE 0x5
#define ID_FIL 0x6
#define ID_END 0x7

#define ONLY_LONG_SEQUENCE   0x0
#define LONG_START_SEQUENCE  0x1
#define EIGHT_SHORT_SEQUENCE 0x2
#define LONG_STOP_SEQUENCE   0x3

#define ZERO_HCB       0
#define FIRST_PAIR_HCB 5
#define ESC_HCB        11
#define QUAD_LEN       4
#define PAIR_LEN       2
#define NOISE_HCB      13
#define INTENSITY_HCB2 14
#define INTENSITY_HCB  15

int8_t GASpecificConfig_faad(bitfile *ld, mp4AudioSpecificConfig *mp4ASC,
                        program_config *pce);

BYTE hw_adts_frame(adts_header *adts, bitfile *ld);
void hw_get_adif_header(adif_header *adif, bitfile *ld);
void decode_sce_lfe_faad(faacDecHandle hDecoder, faacDecFrameInfo *hInfo, bitfile *ld,
                    BYTE id_syn_ele);
void decode_cpe_faad(faacDecHandle hDecoder, faacDecFrameInfo *hInfo, bitfile *ld,
                BYTE id_syn_ele);
void hw_raw_data_block(faacDecHandle hDecoder, faacDecFrameInfo *hInfo,
                    bitfile *ld, program_config *pce);


/* static functions */
static BYTE single_lfe_channel_element(faacDecHandle hDecoder, bitfile *ld,
                                          BYTE channel);
static BYTE channel_pair_element(faacDecHandle hDecoder, bitfile *ld,
                                    BYTE channel);
static uint16_t data_stream_element(faacDecHandle hDecoder, bitfile *ld);
static BYTE program_config_element(program_config *pce, bitfile *ld);
static BYTE fill_element(faacDecHandle hDecoder, bitfile *ld);
static BYTE individual_channel_stream(faacDecHandle hDecoder, element *ele,
                                         bitfile *ld, ic_stream *ics, BYTE scal_flag,
                                         int16_t *spec_data);
static BYTE ics_info(faacDecHandle hDecoder, ic_stream *ics, bitfile *ld,
                        BYTE common_window);
static BYTE section_data(faacDecHandle hDecoder, ic_stream *ics, bitfile *ld);
static BYTE scale_factor_data(faacDecHandle hDecoder, ic_stream *ics, bitfile *ld);
static void gain_control_data(bitfile *ld, ic_stream *ics);
static BYTE spectral_data(faacDecHandle hDecoder, ic_stream *ics, bitfile *ld,
                             int16_t *spectral_data);
static BYTE pulse_data(ic_stream *ics, pulse_info *pul, bitfile *ld);
static void tns_data(ic_stream *ics, tns_info *tns, bitfile *ld);
static BYTE adts_fixed_header(adts_header *adts, bitfile *ld);
static void adts_variable_header(adts_header *adts, bitfile *ld);
static void adts_error_check(adts_header *adts, bitfile *ld);

#endif
