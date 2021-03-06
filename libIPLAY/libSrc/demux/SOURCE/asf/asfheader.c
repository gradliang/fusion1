/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      asfheader.c
*
* Programmer:    Greg Xu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description:   ASF format file header
*        
* Change History (most recent first):
*     <1>     03/30/2005    Greg Xu    first file
****************************************************************
*/

#include "global612.h"
#if VIDEO_ON
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/

#include "mptrace.h"
#include "stream.h"
#include "demux_stream.h"
#include "demux.h"
#include "stheader.h"

#include "bswap.h"
#include "asf.h"

#ifdef ARCH_X86
#define	ASF_LOAD_GUID_PREFIX(guid)	(*(uint32_t *)(guid))
#else
#define	ASF_LOAD_GUID_PREFIX(guid)	\
	((guid)[3] << 24 | (guid)[2] << 16 | (guid)[1] << 8 | (guid)[0])
#endif

#define ASF_GUID_PREFIX_audio_stream	0xF8699E40
#define ASF_GUID_PREFIX_video_stream	0xBC19EFC0
#define ASF_GUID_PREFIX_audio_conceal_none 0x49f1a440
#define ASF_GUID_PREFIX_audio_conceal_interleave 0xbfc3cd50
#define ASF_GUID_PREFIX_header		0x75B22630
#define ASF_GUID_PREFIX_data_chunk	0x75b22636
#define ASF_GUID_PREFIX_index_chunk	0x33000890
#define ASF_GUID_PREFIX_stream_header	0xB7DC0791
#define ASF_GUID_PREFIX_header_2_0	0xD6E229D1
#define ASF_GUID_PREFIX_file_header	0x8CABDCA1
#define	ASF_GUID_PREFIX_content_desc	0x75b22633
#define	ASF_GUID_PREFIX_stream_group	0x7bf875ce


static ASF_header_t asfh;
static ASF_obj_header_t objh;
static ASF_file_header_t fileh;
static ASF_stream_header_t streamh;
static ASF_content_description_t contenth;


//added by jackyang 20050607
ASF_Content asfcontent;

//add end jackyang 20050607

unsigned char *asf_packet = NULL;
int asf_scrambling_h = 1;
int asf_scrambling_w = 1;
int asf_scrambling_b = 1;
int asf_packetsize = 0;
double asf_packetrate = 0;
int asf_movielength = 0;
extern DWORD audio_delay_time;

//int i;

// the variable string is modify in this function
void pack_asf_string(char *string, int length)
{
	int i, j;

	if (string == NULL)
		return;
	for (i = 0, j = 0; i < length && string[i] != '\0'; i += 2, j++)
	{
		string[j] = string[i];
	}
	string[j] = '\0';
}

// the variable string is modify in this function
void print_asf_string(const char *name, char *string, int length)
{
	pack_asf_string(string, length);
}

static char *asf_chunk_type(unsigned char *guid)
{
	static char tmp[60];
	char *p;
	int i;

	switch (ASF_LOAD_GUID_PREFIX(guid))
	{
	case ASF_GUID_PREFIX_audio_stream:
		return "guid_audio_stream";
	case ASF_GUID_PREFIX_video_stream:
		return "guid_video_stream";
	case ASF_GUID_PREFIX_audio_conceal_none:
		return "guid_audio_conceal_none";
	case ASF_GUID_PREFIX_audio_conceal_interleave:
		return "guid_audio_conceal_interleave";
	case ASF_GUID_PREFIX_header:
		return "guid_header";
	case ASF_GUID_PREFIX_data_chunk:
		return "guid_data_chunk";
	case ASF_GUID_PREFIX_index_chunk:
		return "guid_index_chunk";
	case ASF_GUID_PREFIX_stream_header:
		return "guid_stream_header";
	case ASF_GUID_PREFIX_header_2_0:
		return "guid_header_2_0";
	case ASF_GUID_PREFIX_file_header:
		return "guid_file_header";
	case ASF_GUID_PREFIX_content_desc:
		return "guid_content_desc";
	default:
		strcpy(tmp, "unknown guid ");
		p = tmp + strlen(tmp);
		for (i = 0; i < 16; i++)
		{
			if ((1 << i) & ((1 << 4) | (1 << 6) | (1 << 8)))
				*p++ = '-';
			sprintf(p, "%02x", guid[i]);
			p += 2;
		}
		return tmp;
	}
}

int asf_check_header(demuxer_t * demuxer)
{
	unsigned char asfhdrguid[16] =
		{ 0x30, 0x26, 0xB2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE,
		0x6C
	};
	stream_read(demuxer->stream, (char *) &asfh, sizeof(asfh));	// header obj
	le2me_ASF_header_t(&asfh);	// swap to machine endian
//  for(i=0;i<16;i++) MP_DPF(" %02X",temp[i]);MP_DPF("\n");
//  for(i=0;i<16;i++) MP_DPF(" %02X",asfhdrguid[i]);MP_DPF("\n");
	if (memcmp(asfhdrguid, asfh.objh.guid, 16))
	{
		return 0;				// not ASF guid
	}
	if (asfh.cno > 256)
	{
		return 0;				// invalid header???
	}
	return 1;
}

extern void print_wave_header(WAVEFORMATEX * h);
extern void print_video_header(BITMAPINFOHEADER * h);
int read_asf_header(demuxer_t * demuxer)
{   
	static unsigned char buffer[2048];
	uint32_t *streams = NULL;
	int audio_streams = 0;
	int video_streams = 0;
	uint16_t stream_count = 0;
	int best_video = -1;
	int best_audio = -1;
	ASF_PRIV_T *priv = demuxer->priv;

    MP_DEBUG("enter read_asf_header\r\n");

	while (!stream_eof(demuxer->stream))
	{
		unsigned int pos, endpos;

		pos = stream_tell(demuxer->stream);
		stream_read(demuxer->stream, (char *) &objh, sizeof(objh));
		le2me_ASF_obj_header_t(&objh);
		
		if (stream_eof(demuxer->stream))
			break;				// EOF
		//modified by gregxu 2004.12.23, objh.size is 64 bits value, how to handle?
		{
			uint64_t objhsize = 0;

			memcpy(&(objhsize), &(objh.size), sizeof(uint64_t));

			endpos = pos + objhsize;

		}
//  for(i=0;i<16;i++) MP_DPF("%02X ",objh.guid[i]);
		//MP_DPF("0x%08X  [%s] %d\n",pos, asf_chunk_type(objh.guid),(int) objh.size);


		switch (ASF_LOAD_GUID_PREFIX(objh.guid))
		{
		    case ASF_GUID_PREFIX_stream_header:
            MP_DEBUG("ASF_GUID_PREFIX_stream_header");

			stream_read(demuxer->stream, (char *) &streamh, sizeof(streamh));
			le2me_ASF_stream_header_t(&streamh);
			// if(verbose>0){
			//   mp_msg(MSGT_HEADER,MSGL_V,"stream type: %s\n",asf_chunk_type(streamh.type));
			//mp_msg(MSGT_HEADER,MSGL_V,"stream concealment: %s\n",asf_chunk_type(streamh.concealment));
			//mp_msg(MSGT_HEADER,MSGL_V,"type: %d bytes,  stream: %d bytes  ID: %d\n",(int)streamh.type_size,(int)streamh.stream_size,(int)streamh.stream_no);
			//mp_msg(MSGT_HEADER,MSGL_V,"unk1: %lX  unk2: %X\n",(unsigned long)streamh.unk1,(unsigned int)streamh.unk2);
			//mp_msg(MSGT_HEADER,MSGL_V,"FILEPOS=0x%X\n",stream_tell(demuxer->stream));
			//}
			{
				uint32_t streamhtype_size = 0;
				uint32_t streamhstream_size = 0;

				memcpy(&(streamhtype_size), &(streamh.type_size), sizeof(uint32_t));
				memcpy(&(streamhstream_size), &(streamh.stream_size), sizeof(uint32_t));

				if (streamhtype_size > 2048 || streamhstream_size > 2048)
				{
					//mp_msg(MSGT_HEADER,MSGL_FATAL,"FATAL: header size bigger than 2048 bytes (%d,%d)!\n"
					//    "Please contact mplayer authors, and upload/send this file.\n",
					//(int)streamhtype_size,(int)streamhstream_size);
					return 0;
				}
			}
			// type-specific data:
			{
				uint32_t streamhtype_size = 0;

				memcpy(&(streamhtype_size), &(streamh.type_size), sizeof(uint32_t));

				stream_read(demuxer->stream, (char *) buffer, streamhtype_size);
			}
			switch (ASF_LOAD_GUID_PREFIX(streamh.type))
			{
			    case ASF_GUID_PREFIX_audio_stream:
				{
                    MP_DEBUG("ASF_GUID_PREFIX_audio_stream");
					sh_audio_t *sh_audio = new_sh_audio(demuxer, streamh.stream_no & 0x7F);

					++audio_streams;
					{
						uint32_t streamhtype_size = 0;

						memcpy(&(streamhtype_size), &(streamh.type_size), sizeof(uint32_t));

//        sh_audio->wf=(WAVEFORMATEX*)calloc((streamhtype_size<sizeof(WAVEFORMATEX))?sizeof(WAVEFORMATEX):streamhtype_size,1);
						sh_audio->wf =
							(WAVEFORMATEX *) mem_malloc((streamhtype_size < sizeof(WAVEFORMATEX)) ?
														sizeof(WAVEFORMATEX) : streamhtype_size);
						memset(sh_audio->wf, 0,
							   (streamhtype_size <
								sizeof(WAVEFORMATEX)) ? sizeof(WAVEFORMATEX) : streamhtype_size);
						memcpy(sh_audio->wf, buffer, streamhtype_size);
					}
					le2me_WAVEFORMATEX(sh_audio->wf);
					// if(verbose>=1) print_wave_header(sh_audio->wf);
					if (ASF_LOAD_GUID_PREFIX(streamh.concealment) ==
						ASF_GUID_PREFIX_audio_conceal_interleave)
					{
						{
							uint32_t streamhstream_size = 0;

							memcpy(&(streamhstream_size), &(streamh.stream_size), sizeof(uint32_t));

							stream_read(demuxer->stream, (char *) buffer, streamhstream_size);
						}
						asf_scrambling_h = buffer[0];
						asf_scrambling_w = (buffer[2] << 8) | buffer[1];
						asf_scrambling_b = (buffer[4] << 8) | buffer[3];
						asf_scrambling_w /= asf_scrambling_b;
					}
					else
					{
						asf_scrambling_b = asf_scrambling_h = asf_scrambling_w = 1;
					}
					//mp_msg(MSGT_HEADER,MSGL_V,"ASF: audio scrambling: %d x %d x %d\n",asf_scrambling_h,asf_scrambling_w,asf_scrambling_b);
					if (demuxer->audio->id==-1) demuxer->audio->id = (streamh.stream_no & 0x7F);

					break;
				}
				
			    case ASF_GUID_PREFIX_video_stream:
				{
					MP_DEBUG("ASF_GUID_PREFIX_video_stream");
					sh_video_t *sh_video = new_sh_video(demuxer, streamh.stream_no & 0x7F);
					unsigned int len = 0;

					{
						uint32_t streamhtype_size = 0;

						memcpy(&(streamhtype_size), &(streamh.type_size), sizeof(uint32_t));

						len = streamhtype_size - (4 + 4 + 1 + 2);
					}
					++video_streams;
//        sh_video->bih=malloc(chunksize); memset(sh_video->bih,0,chunksize);

// //MP_DEBUG("ASF_GUID_PREFIX_video_stream\r\n");
// DebugBreak();

					//sh_video->bih=(BITMAPINFOHEADER*)calloc((len<sizeof(BITMAPINFOHEADER))?sizeof(BITMAPINFOHEADER):len,1);
					sh_video->bih =
						(BITMAPINFOHEADER *) mem_malloc((len < sizeof(BITMAPINFOHEADER)) ?
														sizeof(BITMAPINFOHEADER) : len);
					memset(sh_video->bih, 0,
						   (len < sizeof(BITMAPINFOHEADER)) ? sizeof(BITMAPINFOHEADER) : len);
					memcpy(sh_video->bih, &buffer[4 + 4 + 1 + 2], len);
					le2me_BITMAPINFOHEADER(sh_video->bih);
					//sh_video->fps=(float)sh_video->video.dwRate/(float)sh_video->video.dwScale;
					//sh_video->frametime=(float)sh_video->video.dwScale/(float)sh_video->video.dwRate;
					// if(verbose>=1) print_video_header(sh_video->bih);
					//asf_video_id=streamh.stream_no & 0x7F;
					if (demuxer->video->id==-1) demuxer->video->id = (streamh.stream_no & 0x7F);

// //MP_DEBUG("ASF_GUID_PREFIX_video_stream OK\r\n");
// DebugBreak();

					break;
				}
			}
			// stream-specific data:
			// stream_read(demuxer->stream,(char*) buffer,streamh.stream_size);
			break;
//  case ASF_GUID_PREFIX_header_2_0: return "guid_header_2_0";

			case ASF_GUID_PREFIX_file_header:	// guid_file_header

                 MP_DEBUG("ASF_GUID_PREFIX_file_header");
// DebugBreak();

			    stream_read(demuxer->stream, (char *) &fileh, sizeof(fileh));
			    le2me_ASF_file_header_t(&fileh);
			//mp_msg(MSGT_HEADER,MSGL_V,"ASF: packets: %d  flags: %d  pack_size: %d  frame_size: %d\n",(int)fileh.packets,(int)fileh.flags,(int)fileh.packetsize,(int)fileh.frame_size);
			//mp_msg(MSGT_HEADER,MSGL_V,"ASF: packets: %d  flags: %d  max_packet_size: %d  min_packet_size: %d  max_bitrate: %d  preroll: %d\n",(int)fileh.num_packets,(int)fileh.flags,(int)fileh.min_packet_size,(int)fileh.max_packet_size,(int)fileh.max_bitrate,(int)fileh.preroll);
			{
				uint32_t filehmax_packet_size = 0;
				uint32_t filehmax_bitrate = 0;
				uint64_t filehsend_duration = 0;

				memcpy(&(filehmax_packet_size), &(fileh.max_packet_size), sizeof(uint32_t));
				memcpy(&(filehmax_bitrate), &(fileh.max_bitrate), sizeof(uint32_t));
				memcpy(&(filehsend_duration), &(fileh.send_duration), sizeof(uint64_t));

				asf_packetsize = filehmax_packet_size;
				//asf_packet=(unsigned char*)malloc(asf_packetsize); // !!!
				asf_packet = (unsigned char *) mem_malloc(asf_packetsize);	// !!!
				//CHK_MALLOC(asf_packet, "read_asf_header failed");

				asf_packetrate = filehmax_bitrate / 8.0 / (double) asf_packetsize;
				asf_movielength = filehsend_duration / 10000000LL;
				audio_delay_time = fileh.preroll;
			}
			break;

			case ASF_GUID_PREFIX_data_chunk:	// guid_data_chunk
            {
                MP_DEBUG("ASF_GUID_PREFIX_data_chunk");
			    demuxer->movi_start = stream_tell(demuxer->stream) + 26;
			    demuxer->movi_end = endpos;			
			    MP_DEBUG("Found movie at 0x%X - 0x%X",(int)demuxer->movi_start,(int)demuxer->movi_end);				
			}	
			break;

            case ASF_GUID_PREFIX_index_chunk:
			{
				ASF_SIMPLE_INDEX_OBJ_T simple_idx_obj;
				int i;
				ASF_INDEX_ENTRY_T* idx;		
				
				MP_DEBUG("ASF_GUID_PREFIX_index_chunk"); 				
				stream_read(demuxer->stream, (char *) &simple_idx_obj, sizeof(simple_idx_obj));
			    le2me_ASF_simple_index_obj_t(&simple_idx_obj);
				priv->idx_size = simple_idx_obj.idx_entries_cnt;
				idx = (ASF_INDEX_ENTRY_T *) priv->idx = (void *) mem_malloc(priv->idx_size * sizeof(ASF_INDEX_ENTRY_T));
				for (i=0; i<priv->idx_size; i++)
				{
				    idx->packet_count  = stream_read_dword_le(demuxer->stream);
					idx->packet_number = stream_read_word_le(demuxer->stream);
					idx++;
				}																
            }
			break;

		    case ASF_GUID_PREFIX_content_desc:	// Content description
			{
				char *string = NULL;

                MP_DEBUG("ASF_GUID_PREFIX_content_desc");

				stream_read(demuxer->stream, (char *) &contenth, sizeof(contenth));
				le2me_ASF_content_description_t(&contenth);
				
				// extract the title
				if (contenth.title_size != 0)
				{
					string = (char *) mem_malloc(contenth.title_size);
					////CHK_MALLOC(string, "read_asf_header failed");
					MP_ASSERT(string);
					stream_read(demuxer->stream, string, contenth.title_size);

				}
				// extract the author 
				if (contenth.author_size != 0)
				{
					string = (char *) mem_reallocm((void *) string, contenth.author_size);
					stream_read(demuxer->stream, string, contenth.author_size);

				}
				// extract the copyright
				if (contenth.copyright_size != 0)
				{
					string = (char *) mem_reallocm((void *) string, contenth.copyright_size);
					stream_read(demuxer->stream, string, contenth.copyright_size);

				}
				// extract the comment
				if (contenth.comment_size != 0)
				{
					string = (char *) mem_reallocm((void *) string, contenth.comment_size);
					stream_read(demuxer->stream, string, contenth.comment_size);
				}
				// extract the rating
				if (contenth.rating_size != 0)
				{
					string = (char *) mem_reallocm((void *) string, contenth.rating_size);
					stream_read(demuxer->stream, string, contenth.rating_size);

					// if(verbose>0)
					//  print_asf_string(" Rating: ", string, contenth.rating_size);

				}
				
				mem_free(string);
				break;
			}
			
		    case ASF_GUID_PREFIX_stream_group:
			{
				uint16_t stream_id, i;
				uint32_t max_bitrate;
				char *object = NULL, *ptr = NULL;

                MP_DEBUG("ASF_GUID_PREFIX_stream_group");

				{
					uint64_t objhsize = 0;

					memcpy(&(objhsize), &(objh.size), sizeof(uint64_t));

					object = (char *) mem_malloc(objhsize);

					if (object == NULL)
					{
						MP_ALERT("Memory allocation failed");
						return 0;
					}
					stream_read(demuxer->stream, object, objhsize);

					// FIXME: We need some endian handling below...
					ptr = object;
					stream_count = le2me_16(*(uint16_t *) ptr);
					ptr += sizeof(uint16_t);
					if (stream_count > 0)
					{
						streams = (uint32_t *) mem_malloc(2 * stream_count * sizeof(uint32_t));
						MP_ASSERT(streams);
					}
					// MP_DPF(" stream count=[0x%x][%u]\n", stream_count, stream_count );
					for (i = 0; i < stream_count && ptr < ((char *) object + objhsize); i++)
					{
						stream_id = le2me_16(*(uint16_t *) ptr);
						ptr += sizeof(uint16_t);
						memcpy(&max_bitrate, ptr, sizeof(uint32_t));	// workaround unaligment bug on sparc
						max_bitrate = le2me_32(max_bitrate);
						ptr += sizeof(uint32_t);
						// MP_DPF("   stream id=[0x%x][%u]\n", stream_id, stream_id );
						// MP_DPF("   max bitrate=[0x%x][%u]\n", max_bitrate, max_bitrate );
						streams[2 * i] = stream_id;
						streams[2 * i + 1] = max_bitrate;
					}
					// MP_DPF("============ ASF Stream group == END ===\n");
					mem_free(object);
				}
				break;
			}
		}						// switch GUID

		//if (ASF_LOAD_GUID_PREFIX(objh.guid) == ASF_GUID_PREFIX_data_chunk)
		//	break;				// movi chunk

		if (!stream_seek(demuxer->stream, endpos))
			break;
	}							// while EOF


	if (streams)
	{
		uint32_t vr = 0, ar = 0, i;

#ifdef MPLAYER_NETWORK
		if (demuxer->stream->streaming_ctrl != NULL)
		{
			if (demuxer->stream->streaming_ctrl->bandwidth != 0
				&& demuxer->stream->streaming_ctrl->data != NULL)
			{
				best_audio =
					((asf_http_streaming_ctrl_t *) demuxer->stream->streaming_ctrl->data)->audio_id;
				best_video =
					((asf_http_streaming_ctrl_t *) demuxer->stream->streaming_ctrl->data)->video_id;
			}
		}
		else
#endif
			for (i = 0; i < stream_count; i++)
			{
				uint32_t id = streams[2 * i];
				uint32_t rate = streams[2 * i + 1];

				if (demuxer->v_streams[id] && rate > vr)
				{
					vr = rate;
					best_video = id;
				}
				else if (demuxer->a_streams[id] && rate > ar)
				{
					ar = rate;
					best_audio = id;
				}
			}
		mem_free(streams);
	}

//mp_msg(MSGT_HEADER,MSGL_V,"ASF: %d audio and %d video streams found\n",audio_streams,video_streams);
	if (!audio_streams)
		demuxer->audio->id = -2;	// nosound
	else if (best_audio > 0 && demuxer->audio->id == -1)
		demuxer->audio->id = best_audio;
	if (!video_streams)
	{
		if (!audio_streams)
		{
			//mp_msg(MSGT_HEADER,MSGL_ERR,"ASF: no audio or video headers found - broken file?\n");
			return 0;
		}
		demuxer->video->id = -2;	// audio-only
	}
	else if (best_video > 0 && demuxer->video->id == -1)
		demuxer->video->id = best_video;

#if 0
	if (verbose)
	{
		MP_DPF("ASF duration: %d\n", (int) fileh.duration);
		MP_DPF("ASF start pts: %d\n", (int) fileh.start_timestamp);
		MP_DPF("ASF end pts: %d\n", (int) fileh.end_timestamp);
	}
#endif





	return 1;
}
#endif
