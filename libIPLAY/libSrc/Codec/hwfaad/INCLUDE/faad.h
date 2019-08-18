/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      faad.h
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

#ifndef __AACDEC_H__
#define __AACDEC_H__

#ifndef FAADAPI
#define FAADAPI
#endif

/* object types for AAC */
#define LC         2

/* library output formats */
#define FAAD_FMT_16BIT  1

/* A decode call can eat up to FAAD_MIN_STREAMSIZE bytes per decoded channel,
   so at least so much bytes per channel should be available in this stream */
#define FAAD_MIN_STREAMSIZE 768 /* 6144 bits/channel */
#define MAX_CHANNELS        2 


typedef struct mp4AudioSpecificConfig
{
    /* Audio Specific Info */
    unsigned char objectTypeIndex;
    unsigned char samplingFrequencyIndex;
    unsigned long samplingFrequency;
    unsigned char channelsConfiguration;

    /* GA Specific Info */
    unsigned char frameLengthFlag;
    unsigned char dependsOnCoreCoder;
} mp4AudioSpecificConfig;

typedef struct faacDecConfiguration
{
    unsigned char defObjectType;
    unsigned long defSampleRate;
    unsigned char outputFormat;
    unsigned char useOldADTSFormat;
} faacDecConfiguration, *faacDecConfigurationPtr;

typedef struct faacDecFrameInfo
{
    unsigned long bytesconsumed;
    unsigned long samples;
    unsigned char error;
} faacDecFrameInfo;

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
 
faacDecHandle FAADAPI faacDecOpen();

/* Init the library based on info from the AAC file (ADTS/ADIF) */
long FAADAPI faacDecInit(faacDecHandle hDecoder,
                        unsigned char *buffer,
                        unsigned long buffer_size,
                        unsigned long *samplerate,
                        unsigned char *channels);

/* Init the library using a DecoderSpecificInfo */
char FAADAPI faacDecInit2(faacDecHandle hDecoder, unsigned char *pBuffer,
                         unsigned long SizeOfDecoderSpecificInfo,
                         unsigned long *samplerate, unsigned char *channels);

void FAADAPI faacDecClose(faacDecHandle hDecoder);

void* FAADAPI faacDecDecode(faacDecHandle hDecoder,
                            faacDecFrameInfo *hInfo,
                            unsigned char *buffer,
                            unsigned long buffer_size);

char FAADAPI AudioSpecificConfig(unsigned char *pBuffer,
                                 unsigned long buffer_size,
                                 mp4AudioSpecificConfig *mp4ASC);

#endif
