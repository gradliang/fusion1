/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      aviheader.c
*
* Programmer:    Deming Li
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description:   AVI format file header
*        
* Change History (most recent first):
*     <1>     03/30/2005    Deming Li    first file
****************************************************************
*/

#include "global612.h"
#if VIDEO_ON

#include "app_interface.h"
#include "stream.h"
#include "demux_stream.h"
#include "demux.h"
#include "stheader.h"

#include "avi/aviheader.h"

#define MIN(a,b) (((a)<(b))?(a):(b))


static MainAVIHeader avih;

//added by jackyang 20050607
AVI_Content avicontent;

//add end jackyang 20050607

extern DWORD  g_dwTotalSeconds;


extern void print_avih(MainAVIHeader * h);
extern void print_avih_flags(MainAVIHeader * h);
extern void print_strh(AVIStreamHeader * h);
extern void print_wave_header(WAVEFORMATEX * h);
extern void print_video_header(BITMAPINFOHEADER * h);
extern void print_index(AVIINDEXENTRY * idx, int idx_size);
static int readmpg4profile(demuxer_t * demuxer, DWORD dwLen);

int avi_idx_cmp(const void *elem1, const void *elem2)
{
	register unsigned int a = AVI_IDX_OFFSET((AVIINDEXENTRY *) elem1);
	register unsigned int b = AVI_IDX_OFFSET((AVIINDEXENTRY *) elem2);

	return (a > b) - (b > a);
}

static int odml_get_vstream_id(int id, unsigned char res[])
{
	unsigned char *p = (unsigned char *) &id;

	id = le2me_32(id);

	if (p[2] == 'd')
	{
		if (res)
		{
			res[0] = p[0];
			res[1] = p[1];
		}
		return 1;
	}
	return 0;
}

int read_avi_header(demuxer_t * demuxer, int index_mode)
{
	sh_audio_t *sh_audio = NULL;
	sh_video_t *sh_video = NULL;
	sh_video_t *sh_video_3D = NULL;
	int stream_id = -1;
	int idxfix_videostream = 0;
	int idxfix_divx = 0;
	avi_priv_t *priv = demuxer->priv;
	unsigned int list_end = 0;
	int junk=0;

//---- AVI header:
	priv->idx_size = 0;
	priv->audio_streams = 0;

	//while (!junk)	
	while (1)
	{
		if (stream_eof(demuxer->stream))
		{
			break;
		}	

		int id = stream_read_dword_le(demuxer->stream);
		unsigned chunksize, size2;
		static int last_fccType = 0;
		char *hdr = NULL;

		// Imply -forceidx if -saveidx is specified
		// if (index_file_save)
		//  index_mode = 2;
		//
		if (id == mmioFOURCC('L', 'I', 'S', 'T'))
		{
			unsigned len = stream_read_dword_le(demuxer->stream);	// list size

			id = stream_read_dword_le(demuxer->stream);	// list type
			if (len >= 4)
			{
				len -= 4;
				list_end = stream_tell(demuxer->stream) + ((len + 1) & (~1));
			}
			else
			{
				list_end = 0;
			}
			if (id == listtypeAVIMOVIE)
			{
				// found MOVI header
				if (!demuxer->movi_start)
					demuxer->movi_start = stream_tell(demuxer->stream);
				demuxer->movi_end = stream_tell(demuxer->stream) + len;
				if (demuxer->stream->end_pos > demuxer->movi_end)
					demuxer->movi_end = demuxer->stream->end_pos;
				if (index_mode == -2 || index_mode == 2 || index_mode == 0)
					break;		// reading from non-seekable source (stdin) or forced index or no index forced
				if (list_end > 0)
				{
					if(idxfix_divx > 0)
					readmpg4profile(demuxer, len);
					int ret = stream_seek(demuxer->stream, list_end);	// skip movi
					if (ret == 0)	break;
				}
				list_end = 0;
			}

			continue;
		}

		size2 = stream_read_dword_le(demuxer->stream);
		chunksize = (size2 + 1) & (~1);
		switch (id)
		{
			// Indicates where the subject of the file is archived
		case mmioFOURCC('I', 'A', 'R', 'L'):
			hdr = "Archival Location";
			break;
			// Lists the artist of the original subject of the file;
			// for example, "Michaelangelo."
		case mmioFOURCC('I', 'A', 'R', 'T'):
			/*get author name, by jackyang 20050607 */
			if (MAX_AUTHOR_LEN > size2)
			{
				stream_read(demuxer->stream, avicontent.szAuthor, size2);
			}
			/*add end jackyang 20050607 */
			hdr = "Artist";
			break;
			// Lists the name of the person or organization that commissioned
			// the subject of the file; for example "Pope Julian II."
		case mmioFOURCC('I', 'C', 'M', 'S'):
			hdr = "Commissioned";
			break;
			// Provides general comments about the file or the subject
			// of the file. If the comment is several sentences long, end each
			// sentence with a period. Do not include new-line characters.
		case mmioFOURCC('I', 'C', 'M', 'T'):
			/*get comment, jackyang 20050607 */
			if (MAX_DESCRIP_LEN > size2)
			{
				stream_read(demuxer->stream, avicontent.szDescription, size2);
			}
			/*add end jackyang 20050607 */
			hdr = "Comments";
			break;
			// Records the copyright information for the file; for example,
			// "Copyright Encyclopedia International 1991." If there are multiple
			// copyrights, separate them by semicolon followed by a space.
		case mmioFOURCC('I', 'C', 'O', 'P'):
			/*get copyright jackyang 20050607 */
			if (MAX_COPYRIGHT_LEN > size2)
			{
				stream_read(demuxer->stream, avicontent.szCopyright, size2);
			}
			/*add end 20050607 */
			hdr = "Copyright";
			break;
			// Describes whether an image has been cropped and, if so, how it
			// was cropped; for example, "lower-right corner."
		case mmioFOURCC('I', 'C', 'R', 'D'):
			hdr = "Creation Date";
			break;
			// Describes whether an image has been cropped and, if so, how it
			// was cropped; for example, "lower-right corner."
		case mmioFOURCC('I', 'C', 'R', 'P'):
			hdr = "Cropped";
			break;
			// Specifies the size of the original subject of the file; for
			// example, "8.5 in h, 11 in w."
		case mmioFOURCC('I', 'D', 'I', 'M'):
			hdr = "Dimensions";
			break;
			// Stores dots per inch setting of the digitizer used to
			// produce the file, such as "300."
		case mmioFOURCC('I', 'D', 'P', 'I'):
			hdr = "Dots Per Inch";
			break;
			// Stores the of the engineer who worked on the file. If there are
			// multiple engineers, separate the names by a semicolon and a blank;
			// for example, "Smith, John; Adams, Joe."
		case mmioFOURCC('I', 'E', 'N', 'G'):
			hdr = "Engineer";
			break;
			// Describes the original work, such as "landscape,", "portrait,"
			// "still liefe," etc.
		case mmioFOURCC('I', 'G', 'N', 'R'):
			hdr = "Genre";
			break;
			// Provides a list of keywords that refer to the file or subject of the
			// file. Separate multiple keywords with a semicolon and a blank;
			// for example, "Seattle, aerial view; scenery."
		case mmioFOURCC('I', 'K', 'E', 'Y'):
			hdr = "Keywords";
			break;
			// ILGT - Describes the changes in the lightness settings on the digitizer
			// required to produce the file. Note that the format of this information
			// depends on the hardware used.
		case mmioFOURCC('I', 'L', 'G', 'T'):
			hdr = "Lightness";
			break;
			// IMED - Decribes the original subject of the file, such as
			// "computer image," "drawing," "lithograph," and so on.
		case mmioFOURCC('I', 'M', 'E', 'D'):
			hdr = "Medium";
			break;
			// INAM - Stores the title of the subject of the file, such as
			// "Seattle from Above."
		case mmioFOURCC('I', 'N', 'A', 'M'):
			/*get file name, jackyang 20050607 */
			if (MAX_TITLE_LEN > size2)
			{
				stream_read(demuxer->stream, avicontent.szTitle, size2);
			}
			/*add end jackyang 20050607 */
			hdr = "Name";
			break;
			// IPLT - Specifies the number of colors requested when digitizing
			// an image, such as "256."
		case mmioFOURCC('I', 'P', 'L', 'T'):
			hdr = "Palette Setting";
			break;
			// IPRD - Specifies the name of title the file was originally intended
			// for, such as "Encyclopedia of Pacific Northwest Geography."
		case mmioFOURCC('I', 'P', 'R', 'D'):
			hdr = "Product";
			break;
			// ISBJ - Decsribes the contents of the file, such as
			// "Aerial view of Seattle."
		case mmioFOURCC('I', 'S', 'B', 'J'):
			hdr = "Subject";
			break;
			// ISFT - Identifies the name of the software packages used to create the
			// file, such as "Microsoft WaveEdit"
		case mmioFOURCC('I', 'S', 'F', 'T'):
			hdr = "Software";
			break;
			// ISHP - Identifies the change in sharpness for the digitizer
			// required to produce the file (the format depends on the hardware used).
		case mmioFOURCC('I', 'S', 'H', 'P'):
			hdr = "Sharpness";
			break;
			// ISRC - Identifies the name of the person or organization who
			// suplied the original subject of the file; for example, "Try Research."
		case mmioFOURCC('I', 'S', 'R', 'C'):
			hdr = "Source";
			break;
			// ISRF - Identifies the original form of the material that was digitized,
			// such as "slide," "paper," "map," and so on. This is not necessarily
			// the same as IMED
		case mmioFOURCC('I', 'S', 'R', 'F'):
			hdr = "Source Form";
			break;
			// ITCH - Identifies the technician who digitized the subject file;
			// for example, "Smith, John."
		case mmioFOURCC('I', 'T', 'C', 'H'):
			hdr = "Technician";
			break;
		case mmioFOURCC('I', 'S', 'M', 'P'):
			hdr = "Time Code";
			break;
		case mmioFOURCC('I', 'D', 'I', 'T'):
			hdr = "Digitization Time";
			break;

		case ckidAVIMAINHDR:	// read 'avih'
			stream_read(demuxer->stream, (char *) &avih, MIN(size2, sizeof(avih)));
			le2me_MainAVIHeader(&avih);	// swap to machine endian
			chunksize -= MIN(size2, sizeof(avih));

			
			//g_dwTotalSeconds = (avih.dwTotalFrames * avih.dwMicroSecPerFrame + 500000) / 1000000;
		
			//      if(verbose>0) print_avih(&avih); // else print_avih_flags(&avih);
			break;
		case ckidSTREAMHEADER:
			{					// read 'strh'
				AVIStreamHeader h;

//    //MP_DEBUG("stream header  found\r\n");
				stream_read(demuxer->stream, (char *) &h, MIN(size2, sizeof(h)));
				le2me_AVIStreamHeader(&h);	// swap to machine endian
				chunksize -= MIN(size2, sizeof(h));
				++stream_id;
				if (h.fccType == streamtypeVIDEO)
				{
 					g_dwTotalSeconds = (DWORD) ((h.dwLength * h.dwScale  + 0.5 * h.dwRate) / h.dwRate);
					//MP_DEBUG("video stream found\r\n");
					if(sh_video == NULL)
					{
						sh_video = new_sh_video(demuxer, stream_id);  
						memcpy(&sh_video->video, &h, sizeof(h));
					  demuxer->video->id = stream_id;
					}
					else if(sh_video_3D == NULL) // two video streams
					{
						sh_video_3D = new_sh_video_3D(demuxer, stream_id);  
						memcpy(&sh_video_3D->video, &h, sizeof(h));            
					  demuxer->video_3D->id = stream_id;
					}
					//MP_DEBUG1("sh_video = ",sh_video,8);
				}
				else if (h.fccType == streamtypeAUDIO)
				{
					//MP_DEBUG("Audio stream found\r\n");
					sh_audio = new_sh_audio(demuxer, stream_id);
					//MP_DEBUG1("sh_audio = ",sh_audio,8);
					memcpy(&sh_audio->audio, &h, sizeof(h));
					//Fix.2004.12.09: when type is audio,set sample_format.  joshua,bruce
					//Fix.2004.12.11: change sample_format to sample_size.  joshua
					//sh_audio->sample_format = h.dwSampleSize;
					sh_audio->samplesize = h.dwSampleSize;
					demuxer->audio->id = stream_id;
				}

				last_fccType = h.fccType;
				//      if(verbose>=1) print_strh(&h);
				//Fix.2004.12.09: move this code to if(h.fccType==streamtypeAUDIO).  joshua,bruce
				//sh_audio->sample_format = h.dwSampleSize;
				break;
			}
			/////////////////////////////////////////////////////
		case mmioFOURCC('i', 'n', 'd', 'x'):
			{
				uint32_t i;
				unsigned msize = 0;
				avisuperindex_chunk *s;

				priv->suidx_size++;
				priv->suidx =
					(avisuperindex_chunk *) mem_reallocm(priv->suidx,
														 priv->suidx_size *
														 sizeof(avisuperindex_chunk));
				s = &priv->suidx[priv->suidx_size - 1];

				chunksize -= 24;
				memcpy(s->fcc, "indx", 4);
				s->dwSize = size2;
				s->wLongsPerEntry = stream_read_word_le(demuxer->stream);
				s->bIndexSubType = stream_read_char(demuxer->stream);
				s->bIndexType = stream_read_char(demuxer->stream);
				s->nEntriesInUse = stream_read_dword_le(demuxer->stream);
				*(uint32_t *) s->dwChunkId = stream_read_dword_le(demuxer->stream);
				stream_read(demuxer->stream, (char *) s->dwReserved, 3 * 4);
				memset(s->dwReserved, 0, 3 * 4);

				//print_avisuperindex_chunk(s);

				msize = sizeof(uint32_t) * s->wLongsPerEntry * s->nEntriesInUse;
				s->aIndex = (avisuperindex_entry *) mem_malloc(msize);
				memset(s->aIndex, 0, msize);
				s->stdidx =
					(avistdindex_chunk *) mem_malloc(s->nEntriesInUse * sizeof(avistdindex_chunk));
				memset(s->stdidx, 0, s->nEntriesInUse * sizeof(avistdindex_chunk));

				// now the real index of indices
				for (i = 0; i < s->nEntriesInUse; i++)
				{
					chunksize -= 16;
					s->aIndex[i].qwOffset = stream_read_dword_le(demuxer->stream) & 0xffffffff;
					s->aIndex[i].qwOffset |=
						((uint64_t) stream_read_dword_le(demuxer->stream) & 0xffffffff) << 32;
					s->aIndex[i].dwSize = stream_read_dword_le(demuxer->stream);
					s->aIndex[i].dwDuration = stream_read_dword_le(demuxer->stream);
					//mp_msg (MSGT_HEADER, MSGL_V, "ODML (%.4s): [%d] 0x%016llx 0x%04lx %ld\n", 
					//      (s->dwChunkId), i,
					//      (uint64_t)s->aIndex[i].qwOffset, s->aIndex[i].dwSize, s->aIndex[i].dwDuration);
				}

				break;
			}
			/////////////////////////////////////////////////////
		case ckidSTREAMFORMAT:
			{					// read 'strf'
				if (last_fccType == streamtypeVIDEO)
				{
				  sh_video_t * sh_video_ptr;
          if(sh_video->bih == NULL)
				    sh_video_ptr = sh_video;
				  else
				    sh_video_ptr = sh_video_3D;
					//sh_video->bih= (BITMAPINFOHEADER*)calloc((chunksize<sizeof(BITMAPINFOHEADER))?sizeof(BITMAPINFOHEADER):chunksize,1);
					sh_video_ptr->bih =
						(BITMAPINFOHEADER *) mem_malloc((chunksize < sizeof(BITMAPINFOHEADER)) ?
														sizeof(BITMAPINFOHEADER) : chunksize);
					memset(sh_video_ptr->bih, 0,
						   (chunksize <
							sizeof(BITMAPINFOHEADER)) ? sizeof(BITMAPINFOHEADER) : chunksize);
					//CHK_MALLOC(sh_video->bih, "calloc sh_video->bih fail");
//        sh_video->bih=malloc(chunksize); memset(sh_video->bih,0,chunksize);
					stream_read(demuxer->stream, (char *) sh_video_ptr->bih, chunksize);
					le2me_BITMAPINFOHEADER(sh_video_ptr->bih);	// swap to machine endian
					// fixup MS-RLE header (seems to be broken for <256 color files)
					if (sh_video_ptr->bih->biCompression <= 1 && sh_video_ptr->bih->biSize == 40)
						sh_video_ptr->bih->biSize = chunksize;
					//        if(verbose>=1) print_video_header(sh_video->bih);
					chunksize = 0;
//        sh_video->fps=(float)sh_video->video.dwRate/(float)sh_video->video.dwScale;
//        sh_video->frametime=(float)sh_video->video.dwScale/(float)sh_video->video.dwRate;
//        if(demuxer->video->id==-1) demuxer->video->id=stream_id;
					// IdxFix:
					idxfix_videostream = stream_id;
					switch (sh_video_ptr->bih->biCompression)
					{
					case mmioFOURCC('M', 'P', 'G', '4'):
					case mmioFOURCC('m', 'p', 'g', '4'):
					case mmioFOURCC('D', 'I', 'V', '1'):
						idxfix_divx = 3;	// we can fix keyframes only for divx coded files!
						break;
					case mmioFOURCC('D', 'I', 'V', '3'):
					case mmioFOURCC('d', 'i', 'v', '3'):
					case mmioFOURCC('D', 'I', 'V', '4'):
					case mmioFOURCC('d', 'i', 'v', '4'):
					case mmioFOURCC('D', 'I', 'V', '5'):
					case mmioFOURCC('d', 'i', 'v', '5'):
					case mmioFOURCC('D', 'I', 'V', '6'):
					case mmioFOURCC('d', 'i', 'v', '6'):
					case mmioFOURCC('M', 'P', '4', '3'):
					case mmioFOURCC('m', 'p', '4', '3'):
					case mmioFOURCC('M', 'P', '4', '2'):
					case mmioFOURCC('m', 'p', '4', '2'):
					case mmioFOURCC('D', 'I', 'V', '2'):
					case mmioFOURCC('A', 'P', '4', '1'):
						idxfix_divx = 1;	// we can fix keyframes only for divx coded files!
						break;
					case mmioFOURCC('D', 'I', 'V', 'X'):
					case mmioFOURCC('d', 'i', 'v', 'x'):
					case mmioFOURCC('D', 'X', '5', '0'):
        				case mmioFOURCC('X', 'V', 'I', 'D'):
        				case mmioFOURCC('x', 'v', 'i', 'd'):
        				case mmioFOURCC('F', 'M', 'P', '4'):
        				case mmioFOURCC('f', 'm', 'p', '4'):
						idxfix_divx = 2;	// we can fix keyframes only for divx coded files!
						break;
					}
				}
				else if (last_fccType == streamtypeAUDIO)
				{
				    #if (0 == PURE_VIDEO)
					unsigned wf_size =
						chunksize < sizeof(WAVEFORMATEX) ? sizeof(WAVEFORMATEX) : chunksize;
					//sh_audio->wf = (WAVEFORMATEX*)calloc(wf_size,1);
					sh_audio->wf = (WAVEFORMATEX *) mem_malloc(wf_size);
					memset(sh_audio->wf, 0, wf_size);
					//CHK_MALLOC(sh_audio->wf, "calloc sh_audio->wf fail");
//        sh_audio->wf=malloc(chunksize); memset(sh_audio->wf,0,chunksize);
					stream_read(demuxer->stream, (char *) sh_audio->wf, chunksize);
					le2me_WAVEFORMATEX(sh_audio->wf);
					if (sh_audio->wf->cbSize != 0 &&
						wf_size < sizeof(WAVEFORMATEX) + sh_audio->wf->cbSize)
					{
						//sh_audio->wf=(WAVEFORMATEX*)realloc(sh_audio->wf, sizeof(WAVEFORMATEX)+sh_audio->wf->cbSize);
						sh_audio->wf =
							(WAVEFORMATEX *) mem_reallocm(sh_audio->wf,
														  sizeof(WAVEFORMATEX) +
														  sh_audio->wf->cbSize);
						//CHK_MALLOC(sh_audio->wf, "realloc sh_audio->wf fail");
					}
					chunksize = 0;
					//  if(verbose>=1) print_wave_header(sh_audio->wf);
					sh_audio->samplerate = (int) sh_audio->wf->nSamplesPerSec;
					sh_audio->channels = sh_audio->wf->nChannels;
					++priv->audio_streams;					
                    //        if(demuxer->audio->id==-1) demuxer->audio->id=stream_id;
                    #endif
				}
				break;
			}
    		case mmioFOURCC('v', 'p', 'r', 'p'): 
    			{
#if 0			
				VideoPropHeader *vprp = mem_malloc(chunksize);
				unsigned int i;
				stream_read(demuxer->stream, (void*)vprp, chunksize);
				le2me_VideoPropHeader(vprp);
				chunksize -= sizeof(*vprp)-sizeof(vprp->FieldInfo);
				chunksize /= sizeof(VIDEO_FIELD_DESC);
				if (vprp->nbFieldPerFrame > chunksize) 
				{
	    			vprp->nbFieldPerFrame = chunksize;
				}
				chunksize = 0;
				for (i=0; i<vprp->nbFieldPerFrame; i++) 
				{
					le2me_VIDEO_FIELD_DESC(&vprp->FieldInfo[i]);
				}
				if (sh_video) {
				sh_video->aspect = GET_AVI_ASPECT(vprp->dwFrameAspectRatio);
				}
				mem_free(vprp);
#endif
				stream_skip(demuxer->stream, chunksize);
				chunksize = 0;
				break;
    			}
    		case mmioFOURCC('d', 'm', 'l', 'h'): 
    			{
				// dmlh 00 00 00 04 frms
				unsigned int total_frames = stream_read_dword_le(demuxer->stream);
				stream_skip(demuxer->stream, chunksize-4);
				chunksize = 0;
    				break;
    			}
		case ckidAVINEWINDEX:
			if (demuxer->movi_end > stream_tell(demuxer->stream))
				demuxer->movi_end = stream_tell(demuxer->stream);	// fixup movi-end
			if (index_mode)
			{
				int i;

				priv->idx_size = size2 >> 4;
				priv->idx = (void *) mem_malloc(priv->idx_size << 4);
				//CHK_MALLOC(priv->idx, "read_avi_header failed");
//      MP_DPF("\nindex to %p !!!!! (priv=%p)\n",priv->idx,priv);
				if(priv->idx==NULL)
				{
					return -1;
				}

				stream_read(demuxer->stream, (char *) priv->idx, priv->idx_size << 4);
				for (i = 0; i < priv->idx_size; i++)	// swap index to machine endian
					le2me_AVIINDEXENTRY((AVIINDEXENTRY *) priv->idx + i);
				chunksize -= priv->idx_size << 4;
				//  if(verbose>=2) print_index(priv->idx,priv->idx_size);
				break;
			}
			/* added May 2002 */
		case mmioFOURCC('R', 'I', 'F', 'F'):
			{
				char riff_type[4];

				stream_read(demuxer->stream, riff_type, sizeof(riff_type));
				if (strncmp(riff_type, "AVIX", sizeof(riff_type)))
					;
				else
				{
					/*
					 * We got an extended AVI header, so we need to switch to
					 * ODML to get seeking to work, provided we got indx chunks
					 * in the header (suidx_size > 0).
					 */
					if (priv->suidx_size > 0)
						priv->isodml = 1;
				}
				chunksize = 0;
				list_end = 0;	/* a new list will follow */
				break;
			}
		case ckidAVIPADDING:
			{
			stream_skip(demuxer->stream, chunksize);
			chunksize = 0;
			break;
			}
		case ckidSTREAMNAME:
		case ckidSTREAMHANDLERDATA:
			{
			stream_skip(demuxer->stream, chunksize);
			chunksize = 0;
			break;			
			}	
		default:		// junk at the end of file
			{
			junk=1;
			break;
			}
		}


		if (hdr)
		{
			if (size2 == 3)
				chunksize = 1;	// empty
			else
			{
				char buf[256];
				int len = (size2 < 250) ? size2 : 250;

				stream_read(demuxer->stream, buf, len);
				chunksize -= len;
				buf[len] = 0;
			}
		}
#if 0
		if (list_end > 0 && chunksize + stream_tell(demuxer->stream) == list_end)
			list_end = 0;
#endif
		if (list_end > 0 && chunksize + stream_tell(demuxer->stream) > list_end)
		{
			stream_seek(demuxer->stream, list_end);
			list_end = 0;
		}
		else if (chunksize > 0)
			stream_skip(demuxer->stream, chunksize);
		else if ((int) chunksize < 0)
		{
		}

	}

	//if (priv->suidx_size > 0 && priv->idx_size == 0)
	if (priv->suidx_size > 0)
	{
		/*
		 * No NEWAVIINDEX, but we got an OpenDML index.
		 */
		priv->isodml = 1;
	}

	if (priv->isodml && (index_mode == -1 || index_mode == 0))
	{
		int i, j, k;
		int safety = 1000;

		avisuperindex_chunk *cx;
		AVIINDEXENTRY *idx;


		if (priv->idx_size)
			mem_free(priv->idx);
		priv->idx_size = 0;
		priv->idx_offset = 0;
		priv->idx = NULL;



		// read the standard indices
		for (cx = &priv->suidx[0], i = 0; i < priv->suidx_size; cx++, i++)
		{
			stream_reset(demuxer->stream);
			for (j = 0; j < cx->nEntriesInUse; j++)
			{
				int ret1, ret2;

				memset(&cx->stdidx[j], 0, 32);
				ret1 = stream_seek(demuxer->stream, (unsigned int) cx->aIndex[j].qwOffset);
				ret2 = stream_read(demuxer->stream, (char *) &cx->stdidx[j], 32);
				if (ret1 != 1 || ret2 != 32 || cx->stdidx[j].nEntriesInUse == 0)
				{
					// this is a broken file (probably incomplete) let the standard
					// gen_index routine handle this
					priv->isodml = 0;
					priv->idx_size = 0;
					//mp_msg(MSGT_HEADER, MSGL_WARN,
					//  "AVI: ODML: Broken (incomplete?) file detected. Will use traditional index\n");
					goto freeout;
				}

				le2me_AVISTDIDXCHUNK(&cx->stdidx[j]);
				//print_avistdindex_chunk(&cx->stdidx[j]);
				priv->idx_size += cx->stdidx[j].nEntriesInUse;
				cx->stdidx[j].aIndex =
					(avistdindex_entry *) mem_malloc(cx->stdidx[j].nEntriesInUse *
													 sizeof(avistdindex_entry));
				stream_read(demuxer->stream, (char *) cx->stdidx[j].aIndex,
							cx->stdidx[j].nEntriesInUse * sizeof(avistdindex_entry));
				for (k = 0; k < cx->stdidx[j].nEntriesInUse; k++)
					le2me_AVISTDIDXENTRY(&cx->stdidx[j].aIndex[k]);

				cx->stdidx[j].dwReserved3 = 0;

			}
		}

		/*
		 * We convert the index by translating all entries into AVIINDEXENTRYs
		 * and sorting them by offset.  The result should be the same index
		 * we would get with -forceidx.
		 */

		idx = (AVIINDEXENTRY *) priv->idx = mem_malloc(priv->idx_size * sizeof(AVIINDEXENTRY));

		for (cx = priv->suidx; cx != &priv->suidx[priv->suidx_size]; cx++)
		{
			avistdindex_chunk *sic;

			for (sic = cx->stdidx; sic != &cx->stdidx[cx->nEntriesInUse]; sic++)
			{
				avistdindex_entry *sie;

				for (sie = sic->aIndex; sie != &sic->aIndex[sic->nEntriesInUse]; sie++)
				{
					uint64_t off = sic->qwBaseOffset + sie->dwOffset - 8;

					memcpy(&idx->ckid, sic->dwChunkId, 4);
					idx->dwChunkOffset = off;
					idx->dwFlags = (off >> 32) << 16;
					idx->dwChunkLength = sie->dwSize & 0x7fffffff;
					idx->dwFlags |= (sie->dwSize & 0x80000000) ? 0x0 : AVIIF_KEYFRAME;	// bit 31 denotes !keyframe
					idx++;
				}
			}
		}
		qsort(priv->idx, priv->idx_size, sizeof(AVIINDEXENTRY), avi_idx_cmp);

		/*
		   Hack to work around a "wrong" index in some divx odml files
		   (processor_burning.avi as an example)
		   They have ##dc on non keyframes but the ix00 tells us they are ##db.
		   Read the fcc of a non-keyframe vid frame and check it.
		 */

		{
			uint32_t id;
			uint32_t db = 0;

			stream_reset(demuxer->stream);

			// find out the video stream id. I have seen files with 01db.
			for (idx = &((AVIINDEXENTRY *) priv->idx)[0], i = 0; i < priv->idx_size; i++, idx++)
			{
				unsigned char res[2];

				if (odml_get_vstream_id(idx->ckid, res))
				{
					db = mmioFOURCC(res[0], res[1], 'd', 'b');
					break;
				}
			}

			// find first non keyframe
			for (idx = &((AVIINDEXENTRY *) priv->idx)[0], i = 0; i < priv->idx_size; i++, idx++)
			{
				if (!(idx->dwFlags & AVIIF_KEYFRAME) && idx->ckid == db)
					break;
			}
			if (i < priv->idx_size && db)
			{
				stream_seek(demuxer->stream, AVI_IDX_OFFSET(idx));
				id = stream_read_dword_le(demuxer->stream);
				if (id && id != db)	// index fcc and real fcc differ? fix it.
					for (idx = &((AVIINDEXENTRY *) priv->idx)[0], i = 0; i < priv->idx_size;
						 i++, idx++)
					{
						if (!(idx->dwFlags & AVIIF_KEYFRAME) && idx->ckid == db)
							idx->ckid = id;
					}
			}
		}

		//if (verbose>=2) print_index(priv->idx, priv->idx_size);

		demuxer->movi_end = demuxer->stream->end_pos;

	  freeout:

		// free unneeded stuff
		cx = &priv->suidx[0];
		do
		{
			for (j = 0; j < cx->nEntriesInUse; j++)
				if (cx->stdidx[j].nEntriesInUse)
					mem_free(cx->stdidx[j].aIndex);
			mem_free(cx->stdidx);

		}
		while (cx++ != &priv->suidx[priv->suidx_size - 1]);
		mem_free(priv->suidx);

	}


  gen_index:
	if (index_mode >= 2 || (priv->idx_size == 0 && index_mode == 1))
	{
		// build index for file:
		stream_reset(demuxer->stream);
		stream_seek(demuxer->stream, demuxer->movi_start);

		priv->idx_pos = 0;
		priv->idx_size = 0;
		priv->idx = NULL;

		while (1)
		{
			int id;
			unsigned int len;
			unsigned int skip;
			AVIINDEXENTRY *idx;
			unsigned int c;

			demuxer->filepos = stream_tell(demuxer->stream);
			if (demuxer->filepos >= demuxer->movi_end && demuxer->movi_start < demuxer->movi_end)
				break;
			id = stream_read_dword_le(demuxer->stream);
			len = stream_read_dword_le(demuxer->stream);
			if (id == mmioFOURCC('L', 'I', 'S', 'T') || id == mmioFOURCC('R', 'I', 'F', 'F'))
			{
				id = stream_read_dword_le(demuxer->stream);	// list or RIFF type
				continue;
			}
			if (stream_eof(demuxer->stream))
				break;
			if (!id || avi_stream_id(id) == 100)
				goto skip_chunk;	// bad ID (or padding?)

			if (priv->idx_pos >= priv->idx_size)
			{
//      priv->idx_size+=32;
				priv->idx_size += 1024;	// +16kB
				priv->idx =
					(void *) mem_reallocm(priv->idx, priv->idx_size * sizeof(AVIINDEXENTRY));
				if (!priv->idx)
				{
					priv->idx_pos = 0;
					break;
				}				// error!
			}
			idx = &((AVIINDEXENTRY *) priv->idx)[priv->idx_pos++];
			idx->ckid = id;
			idx->dwFlags = AVIIF_KEYFRAME;	// FIXME
			idx->dwChunkOffset = (unsigned long) demuxer->filepos;
			idx->dwChunkLength = len;

			c = stream_read_dword(demuxer->stream);

			// Fix keyframes for DivX files:
			if (idxfix_divx)
				if (avi_stream_id(id) == idxfix_videostream)
				{
					switch (idxfix_divx)
					{
					case 3:
						c = stream_read_dword(demuxer->stream) << 5;	//skip 32+5 bits for m$mpeg4v1
					case 1:
						if (c & 0x40000000)
							idx->dwFlags = 0;
						break;	// divx 3
					case 2:
						if (c == 0x1B6)
							idx->dwFlags = 0;
						break;	// divx 4
					}
				}

			// update status line:
			{
				static unsigned int lastpos;
				unsigned int pos;
				unsigned int len = demuxer->movi_end - demuxer->movi_start;

				if (len)
				{
					pos = 100 * (demuxer->filepos - demuxer->movi_start) / len;	// %
				}
				else
				{
					pos = (demuxer->filepos - demuxer->movi_start) >> 20;	// MB
				}
				if (pos != lastpos)
				{
					lastpos = pos;
				}
			}
		  skip_chunk:
			skip = (len + 1) & (~1UL);	// total bytes in this chunk
			stream_seek(demuxer->stream, 8 + demuxer->filepos + skip);
		}
		priv->idx_size = priv->idx_pos;

	}
	return 0;
}

int readmpg4profile(demuxer_t * demuxer, DWORD dwLen)
{
	stream_t * stream = demuxer->stream;
	sh_video_t* sh_video = demuxer->sh_video;
	uint32_t dwRead = 0;
	uint32_t dwSize = 0;
	int i;
	int nStreamId = 0;
	int nTag[4];
	if(NULL == sh_video || NULL == stream) return 0;
	while( dwRead < dwLen )
	{
		for( i=0; i<4; i++ )
			nTag[i] = stream_read_char(stream);
		dwSize = stream_read_dword_le(stream);
		dwSize += (dwSize&1);
		dwRead += 8;

	    nStreamId = (nTag[2] - '0') * 10 + (nTag[3] - '0');
        if(    nTag[2] >= '0' && nTag[2] <= '9'
            && nTag[3] >= '0' && nTag[3] <= '9'
            && nTag[0] == 'i' && nTag[1] == 'x'
          )
		{
			stream_seek(stream, dwSize);
			dwRead += dwSize;
			continue;
        }

		nStreamId = (nTag[0] - '0') * 10 + (nTag[1] - '0');
		if(      nTag[0] >= '0' && nTag[0] <= '9'
			&&   nTag[1] >= '0' && nTag[1] <= '9'
			&& ((nTag[2] == 'd' && nTag[3] == 'c') || 
			(    nTag[2] == 'w' && nTag[3] == 'b') || 
			(    nTag[2] == 'd' && nTag[3] == 'b') ||
			(    nTag[2] == '_' && nTag[3] == '_')) ) 
		{
			int nFind = 0;
			for( i=0; i<4; i++ )
				nTag[i] = stream_read_char(stream);
			uint32_t dwSeek = 4;
			while( !(nTag[0]==0x00 && nTag[1]==0x00 && nTag[2]==0x01 && nTag[3]==0xb0) && dwSeek<dwSize )
			{
				for( i=1; i<4; i++ )
					nTag[i-1] = nTag[i];
				nTag[3] = stream_read_char(stream);
				dwSeek ++;
			}
			if( dwSeek < dwSize )
			{
				sh_video->mp4_profile = stream_read_char(stream);
				dwSeek++;
				nFind = 1;
			}
			dwRead += dwSeek;
			if( nFind || dwRead > 512*1024 )
				break;
			continue;
		}
		if( !stream_seek(stream, dwSize) )
			return 0;
		dwRead += dwSize ;
	}
	if( !stream_seek(stream, dwLen-dwRead) )
		return 0;
	return 1;
}

#undef MIN
#endif
