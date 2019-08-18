/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      demux_mp3.c
*
* Programmer:    Deming Li
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: MP3 format file demux implementation 
*        
* Change History (most recent first):
*     <1>     03/30/2005    Deming Li    first file
****************************************************************
*/
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
//#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "global612.h"
//#include "mpTrace.h"
#include "mp3_hdr.h"
#include "id3_tag.h"

#define HDR_SIZE 4
#define MP3_TYPE 1

static int tabsel_123[2][3][16] = { //Version, Layer, Bitrate
	{{0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0}, //Version 1
	 {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,0},
	 {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0}},

	{{0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0},   //Version 2
	 {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0},
	 {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0}}
};

static long freqs[9] = { 44100, 48000, 32000,	// MPEG 1.0
	22050, 24000, 16000,		// MPEG 2.0
	11025, 12000, 8000
};								// MPEG 2.5

extern BYTE *Bitstream_Buffer;

unsigned int mad_seek_buffer(unsigned int pos)
{
	unsigned int cur_pos;

	cur_pos = MagicPixel_MP3_MAD_getposition_callback();
	if ((cur_pos - pos) <= MP3_BITSTREAM_CACHE) {
		SetCachePtr(pos + MP3_BITSTREAM_CACHE - cur_pos);
	} else {
		cur_pos = pos % MP3_BITSTREAM_CACHE;
		MagicPixel_MP3_MAD_fileseek_callback(pos - cur_pos);
		Fill_Cache_Buffer(MP3_TYPE);
		SetCachePtr(cur_pos);
	}
	return 1;
}

unsigned int mad_read_buffer(unsigned char *data, int length)
{
	int start, end, bytes, remain;
	unsigned int bytes_read = 0;

	start = GetCachePtr();
	end = GetCacheEndPtr();
	bytes = end - start;

	if (length > bytes) {
		remain = length - bytes;
		memcpy(data, &Bitstream_Buffer[start], bytes);
		bytes_read = bytes;
		Fill_Cache_Buffer(MP3_TYPE);
		bytes_read += mad_read_buffer(data + bytes, remain);
	} else {
		memcpy(data, &Bitstream_Buffer[start], length);
		SetCachePtr(start + length);
		bytes_read = length;
	}
	return bytes_read;
}

void mad_skip_buffer(int length)
{
	int start, end, bytes, remain;

	start = GetCachePtr();
	end = GetCacheEndPtr();
	bytes = end - start;

	if (length > bytes) {
		remain = length - bytes;
		Fill_Cache_Buffer(MP3_TYPE);
		mad_skip_buffer(remain);
	} else {
		SetCachePtr(start + length);
	}
}

/*
 * return frame size or -1 (bad frame)
 */
int mp_get_mp3_header_BitRate(unsigned char* hbuf,int* chans, int* srate, int* bitrate)
{
    int layer,stereo,ssize,lsf,framesize,padding,bitrate_index,sampling_frequency;
    unsigned long newhead = 
      hbuf[0] << 24 |
      hbuf[1] << 16 |
      hbuf[2] <<  8 |
      hbuf[3];
     
#if 1
    if ((newhead & 0xffff0000) == 0xffff0000) {
        return -1;
    }
    // head_check:
    if( (newhead & 0xffe00000) != 0xffe00000 ){
	return -1;
    }
#endif

	if ((layer = (4 - ((newhead >> 17) & 3))) == 4) {
		return -1;
	}

    sampling_frequency = ((newhead>>10)&0x3);  // valid: 0..2
    
    if(sampling_frequency==3){
	return -1;
    }

    if( newhead & ((long)1<<20) ) {
      // MPEG 1.0 (lsf==0) or MPEG 2.0 (lsf==1)
      lsf = (newhead & ((long)1<<19)) ? 0x0 : 0x1;
      sampling_frequency += (lsf*3);
    } else {
      // MPEG 2.5
      lsf = 1;
      sampling_frequency += 6;
    }

    bitrate_index = ((newhead>>12)&0xf);  // valid: 1..14
   
    padding   = ((newhead>>9)&0x1);
    
    stereo    = ( (((newhead>>6)&0x3)) == 3) ? 1 : 2;

    if(lsf)
      ssize = (stereo == 1) ? 9 : 17;
    else
      ssize = (stereo == 1) ? 17 : 32;
    if(!((newhead>>16)&0x1)) ssize += 2; // CRC

    *bitrate = tabsel_123[lsf][layer - 1][bitrate_index];
    framesize = *bitrate * 144000;

    if(!framesize){
		return -1;
    }

    framesize /= freqs[sampling_frequency]<<lsf;
    framesize += padding;

    if(*srate==0) 
        *srate = freqs[sampling_frequency];
    else if(*srate != freqs[sampling_frequency])
        return -1;

    if(chans) *chans = stereo;
    
    return framesize;
}

//decode first frame to check VBR or CBR
int mp_decode_first_frame(unsigned char* hbuf, int* sframe,  int* srate , int* layer, int* mpeg2)
{
    uint8_t hdr[4];
    int ssize, stereo, lsf, padding, bitrate_index,sampling_frequency;
    unsigned long newhead = 
      hbuf[0] << 24 |
      hbuf[1] << 16 |
      hbuf[2] <<  8 |
      hbuf[3];

    // head_check:
    if( (newhead & 0xffe00000) != 0xffe00000 ){
	return -1;
    }
    #if 0
    if((4-((newhead>>17)&3))!=3)  //01 : Layer III
    { 
	return -1;
    }
    #else
    *layer=4-((newhead>>17)&3);
    if(*layer == 4)
        return -1;
    #endif

    sampling_frequency = ((newhead>>10)&0x3);  // valid: 0..2
    if(sampling_frequency==3){
	return -1;
    }

    if (newhead & ((long)1<<20) ) {      // MPEG 1.0 (lsf==0) or MPEG 2.0/2.5 (lsf==1)
      *mpeg2=1;//not mpeg2.5
      lsf = (newhead & ((long)1<<19)) ? 0x0 : 0x1;
      sampling_frequency += (lsf*3);
    } else {      // MPEG 2.5
      *mpeg2=0;
      lsf = 1;
      sampling_frequency += 6;
    }

    stereo    = ( (((newhead>>6)&0x3)) == 3) ? 1 : 2;     //1:Mono  2:Stereo, Joint stereo, Dual channel

    if(lsf){
      ssize = (stereo == 1) ? 9 : 17;
      *sframe = 576;
    }else{ //MPEG1
      ssize = (stereo == 1) ? 17 : 32;
      *sframe = 1152;
    }

    //get sampling rate value
    *srate =  freqs[sampling_frequency];
    mad_skip_buffer(ssize);
    //stream_skip(s, ssize);
    mad_read_buffer(hdr, 4);
    //stream_read(s, (char *)hdr,4);
    if((hdr[0] == 'X' && hdr[1] =='i' && hdr[2] == 'n' && hdr[3] == 'g') ||(hdr[0] == 'I' && hdr[1] =='n' && hdr[2] == 'f' && hdr[3] == 'o'))
    {
        mad_read_buffer(hdr, 4);
        //stream_read(s, (char *)hdr,4);
        if(hdr[3]&0x01){
            mad_read_buffer(hdr, 4);
            //stream_read(s, (char *)hdr,4);
            return mp_get_vbr_framenum(hdr);
        }
    }
    else if (hdr[0] == 'V' && hdr[1] =='B' && hdr[2] == 'R' && hdr[3] == 'I')
    {
        mad_skip_buffer(10);  // skip VbriVersion(2), VbriDelay(2), VbriQuality(2), VbriStreamBytes(4)
        mad_read_buffer(hdr, 4);   // VbriStreamFrames
        return mp_get_vbr_framenum(hdr);
    }
    else
        return 0;
    
    
}

//skip uncorrect bytes and search next sync(ffa)
//return num of skip bytes
int mp_search_mp3_sync(unsigned char * hbuf, int* freq)
{
    int frame_len;
    int n=0, step, next_chans, bitrate;
    
    //while(n < 50000 && ! s->eof) {                 // 50000 can skip wrong ID3 file length and search ffa
      while(n < 50000 && ! GetFileEndFlag()) {  
        step = 1;
        
        frame_len=mp_get_mp3_header_BitRate(hbuf, &next_chans, freq, &bitrate);
        
        if (frame_len > 0)
                return n;
                
        if(step < 4)
            memmove(hbuf,&hbuf[step], 4-step);   //4:HDR_SIZE
        mad_read_buffer(&hbuf[4 - step], step);
        //stream_read(s, &hbuf[4 - step], step); 
        n++;                                                 
    }
    
    return -1;  //not MP3 file
}

//calculate frame number of VBR
unsigned int mp_get_vbr_framenum(unsigned char * hbuf)
{
    return (unsigned int)(hbuf[0] << 24 | hbuf[1] << 16 | hbuf[2] <<  8 | hbuf[3]);

}

int get_id3_length(uint8_t* hbuf)
{
	return (hbuf[0] << 21) | (hbuf[1] << 14) | (hbuf[2] << 7) | hbuf[3];
}


//demux_mp3_check_type()--------------------------------------------------------------------
int TAG_len, ID3_len, st_pos, frame_len, mp3_chans, mp3_freq, TotalTime;
static BYTE seq_frames=0;//calculate the sequential 3 correct frames
static int get_mp3_info(int file_size,int *mpeg2,int *layer,int *SamplesPerFrame)
{
	uint8_t hdr[HDR_SIZE];
	int ID3_end_pos = 0, skip_bytes = 0,  SamplingRate;
	unsigned int Num_of_frame = 0;
	int ret;
	//initial global variables
	st_pos = 0;
	frame_len = 0;
	mp3_freq = 0;
	TAG_len = 0;
	ID3_len = 0;
	TotalTime = 0;
	seq_frames=0;

	/***check ID3v1***/
	MagicPixel_MP3_MAD_fileseek_callback(file_size-128);
	//stream_seek(stream, (stream->end_pos - 128));

	ret = Fill_Cache_Buffer(MP3_TYPE);
	mad_read_buffer(hdr,4);
	//stream_read(stream, (char *) hdr, 4);
	if (hdr[0] == 'T' && hdr[1] == 'A' && hdr[2] == 'G')
		TAG_len = 128;

	MagicPixel_MP3_MAD_fileseek_callback(0);
	//	stream_seek(stream, 0);
	/***end ID3v1***/
	Fill_Cache_Buffer(MP3_TYPE);
	mad_read_buffer(hdr,HDR_SIZE);
	//stream_read(stream, hdr, HDR_SIZE);

	/***check ID3v2 or ID3v2.3***/
	if (hdr[0] == 'I' && hdr[1] == 'D' && hdr[2] == '3' && (hdr[3] >= 2))
	{
		mad_skip_buffer(2);
		//stream_skip(stream, 2);
		mad_read_buffer(hdr,4);
		//stream_read(stream, (char *) hdr, 4);
		
		ID3_end_pos = get_id3_length(hdr);
        
        mad_skip_buffer(ID3_end_pos-10);//
		//stream_skip(stream, ID3_end_pos);
		ID3_len = ID3_end_pos;//+10 ;	//the end address of ID3 frame

        mad_read_buffer(hdr,4);
		//stream_read(stream, (char *) hdr, 4);
		
		//check ID3 frame length, not a sync word->search next
		while(seq_frames < 5)
        {
			if ((mp_get_mp3_header_BitRate(hdr, &mp3_chans, &mp3_freq, &MP3_Bitrate)) == -1)
			{
			    seq_frames=0;
				skip_bytes = mp_search_mp3_sync(hdr, &mp3_freq);
                
				if (skip_bytes < 0)
					return -1;
				else
					ID3_len += skip_bytes;
                
                
			}
            seq_frames++; 
            }
		}
		else if ((mp_get_mp3_header_BitRate(hdr, &mp3_chans, &mp3_freq, &MP3_Bitrate)) == -1)
		{
			skip_bytes = mp_search_mp3_sync(hdr, &mp3_freq);
			if (skip_bytes < 0)
				return -1;
		else
			ID3_len += skip_bytes;
	}

	//The current position is at first frame
	int current_pos = GetCurPos(MP3_TYPE);
	//int current_pos = FilePosGet(stream)-BITSTREAM_CACHE+Cache_Ptr;
    
	st_pos = current_pos - HDR_SIZE;

	//get VBR or CBR
    
	Num_of_frame = mp_decode_first_frame(hdr, SamplesPerFrame, &SamplingRate,layer,  mpeg2);

	if (Num_of_frame > 0)
	{						//VBR MP3
		TotalTime = Num_of_frame * (*SamplesPerFrame) / SamplingRate;
		if(TotalTime == 0)
		    TotalTime = 1;
        
        mad_seek_buffer(current_pos);
		//stream_seek(stream, current_pos);
		return 1;
	}
	else if (Num_of_frame == -1)	//Unknow
    {
                  
		return -1;
	}
	else if (Num_of_frame == 0)
	{						//May be CBR or VBR
	    mad_seek_buffer(current_pos);
		//stream_seek(stream, current_pos);
        
		/***check some header data ***/
		if ((frame_len = mp_get_mp3_header_BitRate(hdr, &mp3_chans, &mp3_freq, &MP3_Bitrate)) > 0)
		{
			if (check_mp3_header(file_size, st_pos, frame_len, mp3_chans, mp3_freq) == 0)	// if success
				return 1;	//DEMUX_MP3 check successfully
				else
					return -1;
			}
		}
        
	return  -1;	//UNKNOWN
}

//--------------------------------------------------------------------------------------------------


//weixun 2005.09.27 
/* 
* This function is used to check the current mp3 header 
* whether it is valid. We first assume the current mp3 
* header is ok. Then this function tries to find 50 headers.
* If all headers are ok, then the current mp3 header is 
* considered the very first header in the mp3 file.
return -1: fail, return 0: success
*/
int check_mp3_header(int FileSize, int firstframe_st_pos, int firstframe_len, int mp3_chans,
					 int mp3_freq)


{
	uint8_t hdr[HDR_SIZE];
	int next_mp3_chans, BitRate, tmp_BitRate, VBR = FALSE, next_st_pos = 0, current_pos =
		0, skip_bytes = 0;;
	int nextframe_len = firstframe_len;
	DWORD SumofBitrate = 0, Count = 0;

        current_pos = GetCurPos(MP3_TYPE);
	//current_pos = stream_tell(stream);
	MP3_Bitrate = 0;
	tmp_BitRate = 0;
	next_st_pos = firstframe_st_pos + firstframe_len;

	//read first frame and store tmp_BitRate to cmp CBR or VBR
	mad_seek_buffer( firstframe_st_pos);
	//stream_seek(stream, firstframe_st_pos);
	mad_read_buffer(hdr, 4);
	//stream_read(stream, (char *) hdr, 4);
	mp_get_mp3_header_BitRate(hdr, &next_mp3_chans, &mp3_freq, &BitRate);
	tmp_BitRate = BitRate;
	SumofBitrate += BitRate;	//add first frame to sum
	Count = 1;
    mad_seek_buffer( current_pos);
	//stream_seek(stream, current_pos);

	// first, read 20` MP3 frame => CBR return
	// otherwise read all MP3 frame(over spec MP3)=>overspec VBR
	while (1)
	{
        if (mad_seek_buffer( next_st_pos) == 0)    
		//if (stream_seek(stream, next_st_pos) == 0)
		{
			//MP_DEBUG("Seek MP3 EOF-NO MIND\n");
			break;				// break when eof
		}

        mad_read_buffer(hdr, 4);
		//stream_read(stream, (char *) hdr, 4);

		if ((mp_get_mp3_header_BitRate(hdr, &next_mp3_chans, &mp3_freq, &BitRate)) == -1)
		{
			skip_bytes = mp_search_mp3_sync(hdr, &mp3_freq);
			if (skip_bytes < 0)
			{
				UartOutText("Search SYNC EOF-NO MIND\n");
				break;
			}
			else
				next_st_pos += skip_bytes;
		}

		nextframe_len = mp_get_mp3_header_BitRate(hdr, &next_mp3_chans, &mp3_freq, &BitRate);
		if ((nextframe_len < 0))
		{
			UartOutText("MP3 Header ERROR\n");
            mad_seek_buffer(current_pos);
			//stream_seek(stream, current_pos);
			break;
		}
		SumofBitrate += BitRate;
		Count++;
		next_st_pos += nextframe_len;

		//determine bitrate type : CBR(0) or VBR(1)
		if (Count <= 20 && tmp_BitRate != BitRate)
			VBR = 1;

		if (Count == 20 && VBR == 0)	//CBR File
			break;
	}

	MP3_Bitrate = SumofBitrate * 1000 / Count;
	TotalTime = (FileSize - TAG_len - ID3_len) / (MP3_Bitrate >> 3);
	//TotalTime = (stream->file_len - TAG_len - ID3_len) / MP3_Bitrate;
	if(TotalTime == 0)
	    TotalTime = 1;
    mad_seek_buffer(current_pos);
    //stream_seek(stream, current_pos);
	return 0;
}

static int read_string(unsigned char *buffer, char *ending, int max_len)
{
	int len = 0, ending_len = strlen(ending);
	unsigned int cur_pos;

	cur_pos = GetCurPos(MP3_TYPE);
	while (len < max_len) {
		mad_read_buffer(buffer + len, 1);
		len++;
		if ((len >= ending_len) && (strncmp(buffer + len - ending_len, ending, ending_len) == 0))
			break;
	}

	if (len == max_len) {
		mad_seek_buffer(cur_pos);
		return -1;
	} else {
		*(buffer + len - ending_len) = '\0';
		return 0;
	}
}

static int get_shoutcast_stream_info(int file_length, struct mad_file_info *info)
{
	int result;
	unsigned char item[16], content[128], hdr[HDR_SIZE];
	unsigned int data_start;

	mad_skip_buffer(12);
	while (1) {
		if (read_string(item, ":", 16) == -1)
			return -1;
		data_start = GetCurPos(MP3_TYPE);
		mad_read_buffer(content, 1);
		if (strncmp(content, " ", 1) != 0)
			mad_seek_buffer(data_start);
		if (read_string(content, "\r\n", 128) == -1)
			return -1;

		if (strncmp(item, "content-type", 12) == 0) {
			if (strncmp(content, "audio/mpeg", 10) != 0)
				return -1;
		} else if (strncmp(item, "icy-metaint", 11) == 0) {
			info->block_size = atoi(content);
		} else if (strncmp(item, "icy-br", 6) == 0) {
			info->bitrate = atoi(content);
		}

		data_start = GetCurPos(MP3_TYPE);
		mad_read_buffer(item, 2);
		if (strncmp(item, "\r\n", 2) != 0)
			mad_seek_buffer(data_start);
		else
			break;
	}

	data_start = GetCurPos(MP3_TYPE);
	file_length -= data_start;
	mad_read_buffer(hdr, 4);
	if ((result = mp_search_mp3_sync(hdr, &mp3_freq)) != -1) {   /* try to skip redundant data */
		data_start += result;
		file_length -= result;
		mad_seek_buffer(data_start);

		mad_read_buffer(hdr, HDR_SIZE);
		mp_get_mp3_header_BitRate(hdr, &mp3_chans, &mp3_freq, &MP3_Bitrate);
		mad_seek_buffer(data_start);
		info->skip_bytes = result;
		result = 0;
	}

	if (info->bitrate)
		MP3_Bitrate = info->bitrate;	/* should be the same */
	TotalTime = (file_length << 3) / (MP3_Bitrate * 1000);	 /* roughly calculate */
	if (!info->block_size)
		info->shoutcast = 0;

	return result;
}

static int id3_v1_parse(id3v1_tag_t *tag, int file_length)
{
	int result;
	unsigned char genre, track, buffer[10];

	MagicPixel_MP3_MAD_fileseek_callback(file_length - 128);
	result = Fill_Cache_Buffer(MP3_TYPE);
	mad_read_buffer(buffer, 3);

	if (strncmp(buffer, "TAG", 3) == 0) {	/* ID3 tag version 1 */
		mad_read_buffer(tag->title, 30);
		mad_read_buffer(tag->artist, 30);
		mad_read_buffer(tag->album, 30);
		mad_read_buffer(tag->year, 4);
		mad_read_buffer(tag->comment, 30);
		mad_read_buffer(&genre, 1);
		track = 0;
		if (tag->comment[28] == 0 && tag->comment[29] != 0)
			track = tag->comment[29];
		sprintf((char *)&tag->genre, "%d", genre);
		sprintf((char *)&tag->track, "%d", track);
		return 0;
	}
	return -1;
}

static void id3_v2_read_text_frame(char **frame, unsigned int size)
{
	*frame = (char *)libmad_malloc(size);
	if (*frame != NULL) {
		mad_read_buffer(*frame, size - 1);
		(*frame)[size - 1] = 0;
	} else {
		mad_skip_buffer(size - 1);
	}
}

static void id3_v2_read_comment(char **comment, unsigned int size)
{
	unsigned char buffer[10];
	unsigned int language;

	if (size <= 6) {
		mad_skip_buffer(size - 1);
		return;
	}

	*comment = (char *)libmad_malloc(size - 4);
	if (*comment != NULL) {
		mad_read_buffer(buffer, 3);
		language = (buffer[0] << 16) | (buffer[1] << 8) | buffer[2];
		mad_skip_buffer(1);		/* skip content descriptors */
		mad_read_buffer(*comment, 1);
		if ((*comment)[0] != 0) {
			mad_read_buffer(*comment + 1, size - 6);	/* read actual text string */
		} else {
			mad_read_buffer(*comment, size - 6);	/* read actual text string */
			(*comment)[size - 6] = 0;
		}
		(*comment)[size - 5] = 0;
	} else {
		mad_skip_buffer(size - 1);
	}
}

static int id3_v2_parse(id3v2_tag_t *tag, unsigned int version, unsigned int flags, unsigned int tagsize)
{
	unsigned char buffer[10];
	unsigned int frame_id, frame_size, text_encoding;
	unsigned int extended_header_size;
	char **ptr;
	int remain = tagsize, i;

	/* free previously allocated memory */
	ptr = (char **)&tag->title;
	for (i = 0; i < sizeof(id3v2_tag_t); i += 4, ptr++) {
		if (*ptr) {
			libmad_free(*ptr);
			*ptr = NULL;
		}
	}

	if (version == 0x0200) {	/* ID3 tag version 2 */
		do {
			mad_read_buffer(buffer, 7);
			frame_id = (buffer[0] << 16) | (buffer[1] << 8) | buffer[2];
			frame_size = (buffer[3] << 16) | (buffer[4] << 8) | buffer[5];
			if ((frame_id == 0) || (frame_size == 0) || ((frame_size + 6) > remain))
				return 0;
			text_encoding = buffer[6];

			switch (frame_id) {
			case TT2:
				id3_v2_read_text_frame((char **)&tag->title, frame_size);
				break;
			case TP1:
				id3_v2_read_text_frame((char **)&tag->artist, frame_size);
				break;
			case TAL:
				id3_v2_read_text_frame((char **)&tag->album, frame_size);
				break;
			case TYE:
				id3_v2_read_text_frame((char **)&tag->year, frame_size);
				break;
			case COM:
				id3_v2_read_comment((char **)&tag->comment, frame_size);
				break;
			case TCO:
				id3_v2_read_text_frame((char **)&tag->genre, frame_size);
				break;
			case TRK:
				id3_v2_read_text_frame((char **)&tag->track, frame_size);
				break;
			default:
				mad_skip_buffer(frame_size - 1);
				break;
			}
			remain -= frame_size + 6;
		} while (remain > 6);
	} else if (version == 0x0300) {		/* ID3 tag version 2.3.0 */
		do {
			if (flags & 0x40) {		/* the header is followed by an extended header */
				mad_read_buffer(buffer, 4);
				extended_header_size = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
				mad_skip_buffer(extended_header_size);
			}
			mad_read_buffer(buffer, 11);
			frame_id = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
			frame_size = (buffer[4] << 24) | (buffer[5] << 16) | (buffer[6] << 8) | buffer[7];
			if ((frame_id == 0) || (frame_size == 0) || ((frame_size + 10) > remain))
				return 0;
			/* skip flags */
			text_encoding = buffer[10];

			switch (frame_id) {
			case TIT2:
				id3_v2_read_text_frame((char **)&tag->title, frame_size);
				break;
			case TPE1:
				id3_v2_read_text_frame((char **)&tag->artist, frame_size);
				break;
			case TALB:
				id3_v2_read_text_frame((char **)&tag->album, frame_size);
				break;
			case TYER:
				id3_v2_read_text_frame((char **)&tag->year, frame_size);
				break;
			case COMM:
				id3_v2_read_comment((char **)&tag->comment, frame_size);
				break;
			case TCON:
				id3_v2_read_text_frame((char **)&tag->genre, frame_size);
				break;
			case TRCK:
				id3_v2_read_text_frame((char **)&tag->track, frame_size);
				break;
			default:
				mad_skip_buffer(frame_size - 1);
				break;
			}
			remain -= frame_size + 10;
		} while (remain > 10);
	} else {
		return -1;
	}
	return 0;
}

static id3v1_tag_t id3v1_tag;
static id3v2_tag_t id3v2_tag;

static void get_id3_tag(int file_length, char **song_singer, char **song_name, char **song_comment,
                 char **song_album, char **song_genre, char **song_year, char **song_track)
{
	int result;
	unsigned char buffer[10];

	memset(&id3v1_tag, 0, sizeof(id3v1_tag_t));
	*song_name = (char *)&id3v1_tag.title;
	*song_singer = (char *)&id3v1_tag.artist;
	*song_comment = (char *)&id3v1_tag.comment;
	*song_album = (char *)&id3v1_tag.album;
	*song_genre = (char *)&id3v1_tag.genre;
	*song_year  = (char *)&id3v1_tag.year;
	*song_track = (char *)&id3v1_tag.track;

	/* check ID3v2 or ID3v2.3 */
	MagicPixel_MP3_MAD_fileseek_callback(0);
	result = Fill_Cache_Buffer(MP3_TYPE);
	mad_read_buffer(buffer, 10);

	if (strncmp(buffer, "ID3", 3) != 0) {	/* no ID3 tag version 2 or 2.3.0 */
		result = id3_v1_parse(&id3v1_tag, file_length);		/* parse ID3v1 */
	} else {	/* ID3 tag version 2 or 2.3.0 */
		unsigned int version, flags, tagsize;

		version = (buffer[3] << 8) | buffer[4];
		flags = buffer[5];
		tagsize = (buffer[6] << 21) | (buffer[7] << 14) |
			(buffer[8] <<  7) | (buffer[9] <<  0);
		result = id3_v2_parse(&id3v2_tag, version, flags, tagsize);

		if (result != -1) {
			if (id3v2_tag.title)	*song_name = id3v2_tag.title;
			if (id3v2_tag.artist)	*song_singer = id3v2_tag.artist;
			if (id3v2_tag.comment)	*song_comment = id3v2_tag.comment;
			if (id3v2_tag.album)	*song_album = id3v2_tag.album;
			if (id3v2_tag.genre)	*song_genre = id3v2_tag.genre;
			if (id3v2_tag.year)		*song_year  = id3v2_tag.year;
			if (id3v2_tag.track)	*song_track = id3v2_tag.track;
		}
	}
}

struct mad_file_info mp3info;

int MagicPixel_mp3_mad_init(int file_size,int *ch,int *srate,int *frame_size,int *bitrate,int *total_time,
                        char **song_singer,char **song_name,char **song_comment,char **song_album,char **song_genre,
                        char **song_year,char **song_track,int *mpeg2,int *layer,BYTE filebrowser)
{  
	mpDebugPrint("------  MagicPixel mp3 mad decoder  ------");

	unsigned char header[10];
	int result;

	memset(&mp3info, 0, sizeof(struct mad_file_info));
	Init_BitstreamCache(MP3_TYPE);
	MagicPixel_MP3_MAD_fileseek_callback(0);
	result = Fill_Cache_Buffer(MP3_TYPE);
	mad_read_buffer(header, 10);
	if (strncmp(header, "ICY 200 OK", 10) == 0)
		mp3info.shoutcast = 1;
	mad_seek_buffer(0);

	if (mp3info.shoutcast) {
		result = get_shoutcast_stream_info(file_size, &mp3info);
	} else {
		result = get_mp3_info(file_size, mpeg2, layer, frame_size);
		if (filebrowser == 1) {
			if (result != -1) {
				get_id3_tag(file_size, song_singer, song_name, song_comment,
							song_album, song_genre, song_year, song_track);
				result = 0;
			}
		} else {
			if (result != -1)	result = 0;
		}
		mad_seek_buffer(st_pos + frame_len);
	}

	*ch = mp3_chans;
	*bitrate = MP3_Bitrate;
	*srate = mp3_freq;
	*total_time = TotalTime;

	return result;
}

