/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      structs.h
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

#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#define MAX_CHANNELS        2 
#define MAX_SYNTAX_ELEMENTS 48

typedef struct
{
    BYTE element_instance_tag;
    BYTE object_type;
    BYTE sf_index;
    BYTE num_front_channel_elements;
    BYTE num_side_channel_elements;
    BYTE num_back_channel_elements;
    BYTE num_lfe_channel_elements;
    BYTE num_assoc_data_elements;
    BYTE num_valid_cc_elements;
    BYTE mono_mixdown_present;
    BYTE mono_mixdown_element_number;
    BYTE stereo_mixdown_present;
    BYTE stereo_mixdown_element_number;
    BYTE matrix_mixdown_idx_present;
    BYTE pseudo_surround_enable;
    BYTE matrix_mixdown_idx;
    BYTE front_element_is_cpe[16];
    BYTE front_element_tag_select[16];
    BYTE side_element_is_cpe[16];
    BYTE side_element_tag_select[16];
    BYTE back_element_is_cpe[16];
    BYTE back_element_tag_select[16];
    BYTE lfe_element_tag_select[16];
    BYTE assoc_data_element_tag_select[16];
    BYTE cc_element_is_ind_sw[16];
    BYTE valid_cc_element_tag_select[16];

    BYTE channels;

    BYTE comment_field_bytes;
    BYTE comment_field_data[257];

    /* extra added values */
    BYTE num_front_channels;
    BYTE num_side_channels;
    BYTE num_back_channels;
    BYTE num_lfe_channels;
    BYTE sce_channel[16];
    BYTE cpe_channel[16];
} program_config;

typedef struct
{
    uint16_t syncword;
    BYTE id;
    BYTE layer;
    BYTE protection_absent;
    BYTE profile;
    BYTE sf_index;
    BYTE private_bit;
    BYTE channel_configuration;
    BYTE original;
    BYTE emphasis;
    BYTE copyright_identification_bit;
    BYTE copyright_identification_start;
    uint16_t aac_frame_length;
    uint16_t adts_buffer_fullness;
    BYTE no_raw_data_blocks_in_frame;
    uint16_t crc_check;

    /* control param */
    BYTE old_format;
} adts_header;

typedef struct
{
    int8_t copyright_id[10];
    BYTE bitstream_type;
    uint32_t bitrate;
    BYTE num_program_config_elements;
    /* maximum of 16 PCEs */
    program_config pce[16];
} adif_header;

typedef struct
{
    BYTE number_pulse;
    BYTE pulse_start_sfb;
    BYTE pulse_offset[4];
    BYTE pulse_amp[4];
} pulse_info;

typedef struct
{
    BYTE n_filt[8];
    BYTE coef_res[8];
    BYTE length[8][4];
    BYTE order[8][4];
    BYTE direction[8][4];
    BYTE coef_compress[8][4];
    BYTE coef[8][4][32];
	uint16_t start[8][4];
	int16_t size[8][4];
	int8_t inc[8][4];
} tns_info;

typedef struct
{
    BYTE max_sfb;

    BYTE num_swb;
    BYTE num_window_groups;
    BYTE num_windows;
    BYTE window_sequence;
    BYTE window_group_length[8];
    BYTE window_shape;
    BYTE scale_factor_grouping;
    uint16_t sect_sfb_offset[120];
    uint16_t swb_offset[52];

    BYTE sect_cb[120];
    uint16_t sect_start[120];
    uint16_t sect_end[120];
    BYTE sfb_cb[120];
    BYTE num_sec[8]; /* number of sections in a group */

    BYTE global_gain;
    int16_t scale_factors[120];

    BYTE ms_mask_present;
    BYTE ms_used[128];

    BYTE noise_used;
    BYTE pulse_data_present;
    BYTE tns_data_present;
    pulse_info pul;
    tns_info tns;
} ic_stream; /* individual channel stream */

typedef struct
{
    BYTE ele_id;

    BYTE channel;
    int16_t paired_channel;

    BYTE common_window;

    ic_stream ics1;
    ic_stream ics2;
} element; /* syntax element (SCE, CPE, LFE) */

typedef struct mp4AudioSpecificConfig
{
    /* Audio Specific Info */
    BYTE objectTypeIndex;
    BYTE samplingFrequencyIndex;
    uint32_t samplingFrequency;
    BYTE channelsConfiguration;

    /* GA Specific Info */
    BYTE frameLengthFlag;
    BYTE dependsOnCoreCoder;
} mp4AudioSpecificConfig;

typedef struct faacDecConfiguration
{
    BYTE defObjectType;
    uint32_t defSampleRate;
    BYTE outputFormat;
    BYTE useOldADTSFormat;
} faacDecConfiguration, *faacDecConfigurationPtr;

typedef struct faacDecFrameInfo
{
    uint32_t bytesconsumed;
    uint32_t samples;
    BYTE error;
} faacDecFrameInfo;

typedef struct
{
    BYTE adts_header_present;
    BYTE adif_header_present;
    BYTE sf_index;
    BYTE object_type;
    uint32_t frame;
    BYTE fr_channels;
    BYTE fr_ch_ele;
    void *sample_buffer;
    BYTE window_shape_prev[MAX_CHANNELS];
    /* Program Config Element */
    program_config pce;
    faacDecFrameInfo faac_finfo;

    /* Configuration data */
    faacDecConfiguration config;
} faacDecStruct, *faacDecHandle;
#endif
