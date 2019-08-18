/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      movfmt.h
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
#ifndef _movfmt_h
#define	_movfmt_h


#define BE_16(x) (((unsigned char *)(x))[0] <<  8 | \
		  ((unsigned char *)(x))[1])
#define BE_32(x) (((unsigned char *)(x))[0] << 24 | \
		  ((unsigned char *)(x))[1] << 16 | \
		  ((unsigned char *)(x))[2] <<  8 | \
		  ((unsigned char *)(x))[3])

#define char2short(x,y)	BE_16(&(x)[(y)])
#define char2int(x,y) 	BE_32(&(x)[(y)])

typedef struct {
    unsigned long long pts; // decode time
    unsigned int size;
    off_t pos;	  //sample offset
} mov_sample_t;

typedef struct {
    unsigned int sample; // number of the first sample in the chunk
    unsigned int size;   // number of samples in the chunk
    int desc;            // for multiple codecs mode - not used
    off_t pos;
} mov_chunk_t;

//stco
typedef struct {
    unsigned int first;	//chunk first use this table	
    unsigned int spc;	//sample per chunk
    unsigned int sdid;	//sample description id
} mov_chunkmap_t;

typedef struct {
    unsigned int num;
    unsigned int dur;
} mov_durmap_t;

typedef struct {
    unsigned int dur;		//duration of this segment (movie time)
    unsigned int pos;		//start  time of this segment (media scale) 
    int speed;			//media rate
    //
    int frames;			// frame number in dur
    int start_sample;		//first sample respect to pos 
    int start_frame;		//first start frame of this segment time not necessary equal to start_sample
    int pts_offset;			//offset between  movie time and pos,that is, pos-movie start time(usually 0)
} mov_editlist_t;

#define MOV_TRAK_UNKNOWN 0
#define MOV_TRAK_VIDEO 1
#define MOV_TRAK_AUDIO 2
#define MOV_TRAK_FLASH 3
#define MOV_TRAK_GENERIC 4
#define MOV_TRAK_CODE 5

/*structure of mov file content, jackyang 20050607*/
typedef struct _MOV_Content_
{
	char szName[MAX_TITLE_LEN];
	char szAuthor[MAX_AUTHOR_LEN];
	char szCopyright[MAX_COPYRIGHT_LEN];
	char szRating[MAX_RATING_LEN];
	char szComment[MAX_DESCRIP_LEN];
}MOV_Content, *PMOV_Content;
/*added end jackyang 20050607*/
typedef struct {
    int type;               //type=0 -ok ;type=1-track->timescale=0
    unsigned int i1;		//when type=1,i1 is movie total time
    unsigned int i2;		// not use	
	float        f1;        // not use
	float        f2;        // not use
} mov_exception;

typedef struct {
    int id;
    int type;			//audio,video or generic
    off_t pos;		//if (!samplesize)record single one sample that is read ,else record the chunk  that is read currently 
    //
    unsigned int media_handler;
    unsigned int data_handler;
    //
    int timescale;			//media time scale
    unsigned int length;
    int samplesize;  // 0 implies various size for each sample see "stsz"
    int duration;    // 0 = variable
    int width,height; // for video
    unsigned int fourcc;
    unsigned int nchannels;
    unsigned int samplebytes;
    //"tkhd"  track data
    int tkdata_len;  		 	//size of track chunk 
    unsigned char* tkdata;		// entire track chunk data
    
	//"stsd" stream data
	int stdata_len;  			//entire table size, assume stsd Number of entries=1,exclude size and Fourcc
    unsigned char* stdata;		// stsd table
    //
    unsigned char* stream_header;	//esds.decoderConfig ofr mp4
    int stream_header_len; 		// if >0, this header should be sent before the 1st frame  ,trak->stream_header_len = esds.decoderConfigLen

	//"stsz"
    int samples_size;				//Entries of sample size 
    mov_sample_t* samples;
	
	//"stco" chunk offset
    int chunks_size;				//Entries of chunk
    mov_chunk_t* chunks;
	
	//"stsc" smaples to chunk
    int chunkmap_size;			//bad named!!!  renamed as Entries of stsc table
    mov_chunkmap_t* chunkmap;
	
	//"stts" time to sample table
    int durmap_size;				//bad named!!!  renamed as Number of Entries
    mov_durmap_t* durmap;
	
	//"stss"
    int keyframes_size;			//bad named!!!  renamed as Number of keyframes
    unsigned int* keyframes;

	//
    int editlist_size;				//entries number
    mov_editlist_t* editlist;
    int editlist_pos;
    //
    void* desc; // image/sound/etc description (pointer to ImageDescription etc)
} mov_track_t;

#define MOV_MAX_TRACKS 256

//whole movie meta
typedef struct {
    off_t moov_start;
    off_t moov_end;
    off_t mdat_start;
    off_t mdat_end;
    int track_db;		//record how many tracks
    mov_track_t* tracks[MOV_MAX_TRACKS];
    int timescale; // movie timescale
    int duration;  // movie duration (in movie timescale units)
} mov_priv_t;

#define MOV_FOURCC(a,b,c,d) ((a<<24)|(b<<16)|(c<<8)|(d))


#endif
