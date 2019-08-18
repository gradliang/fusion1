/********************************************
*
*lyric parsing and showing implementation file
*create: 2005/10/21
*updated: 2010/07/09 by Eddyson
*
********************************************/
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#include "global612.h"

#if LYRIC_ENABLE

#define LOCAL_DEBUG_ENABLE 0

#include <fcntl.h>
#include "mpTrace.h"
#include "lrc.h"
#include <unistd.h>
#include <string.h>
#include "devio.h"
#include "util.h"
#include "stream.h"

//#ifndef xpg__H__
#include "xpg.h"
//#endif

//#ifndef STREAM_TYPE_H
#include "stream_type.h"
//#endif

//#ifndef _SUB_COMMON_H_
#include "sub_common.h"
//#endif

int filelen = 0;
char *pfilecontent = NULL;

//int timescale = 0;
char *artist = NULL;
char *title = NULL;
char *album = NULL;
char *editor = NULL;
int offset = 0;

PLRC_TIMETAG lrc_timetags = NULL;
int cur_timetag = 0;
int total_timetag = 0;

PLRC_LINE lrc_line = NULL;
int cur_lrc_line_part = 0;
int total_lrc_line_parts = 0;

//Display position of lyrics
WORD x_lrc = 10; //x-axis point to show the first line of lyric
WORD y_lrc = 50; //y-axis point to show the first line of lyric
int line_dist = 25;
int cur_line = 0;
int total_line = 5; //number of lines to display

int lrc_encoding_format = -1 ;
int MusicLyricState = 0;


/********************************************/
extern XPG_FUNC_PTR* g_pXpgFuncPtr;
extern BYTE __attribute__ ((aligned(4))) SubStreamBuffer[4096];
extern BYTE xpgGetSetupMusicLyricState();



/********************************************/
void Lrc_destroy();
void Line_destroy();
static void parsing_idtags(char *pcontent);
static int parsing_oneline(char *pline);
static void parsing_timetags(char *pcontent);
static char *get_tag_string(char *ptag, char *pcontent);
static char *get_artist(char *pcontent);
static char *get_title(char *pcontent);
static char *get_album(char *pcontent);
static int get_offset(char *pcontent);
static char *get_editor(char *pcontent);
void quicksort_timetags(int left, int right);
void sub_trail_return_char_lrc(char *s);
char *sub_wcscat_lrc(char* s1, char* s2);
int lrc_pos_seek(DWORD ms, BOOL forward);
void Lrc_Line_Cut_ASCII_and_GBK_BIG5(char *pline);
void Lrc_Line_Merge_GB_BIG5_To_Uni(char *pline, char *lrc);
int lrc_auto_detect (stream_t* st);
U16 *GB2312ToUnicode(U16 * target, U16 * source);
U16 GBKGB2312ToUnicode(void ** string);
U16 *GBToUnicode(U16 * target, U16 * source);
U16 GBKToUnicode(void ** string);


//static void sort_timetags(void); //bubblesort timetags for lyrics, note by Eddyson 2010.07.09
/*******************************************/



/********************************************
*FUNCTION    : Lrc_init
*ARGUMENT    : char* pfilename
*FILE FORMAT : .LRC
*RETURN VALUE: 0, success; -1, fail to parsing the lyric file
*COMMENTS    : open and parsing the lyric file, initialize the variables
********************************************/
int Lrc_init(STREAM * pLrcHandle)	//, Lrc_showproc cbshow)
{
	MP_DEBUG1("%s", __func__);
	MusicLyricState = xpgGetSetupMusicLyricState(); // 0 for GBK, 1 for Big5, 2 for Unicode
	MP_DEBUG1("-I- Music Lyric State %x", MusicLyricState);

	cur_timetag = 0;
	total_timetag = 0;
	offset = 0;
	lrc_timetags = NULL;
	char *pfilecontenttemp = NULL;
	int pfilecontenttemp_len=0;



#if 0
	int i = 0;
	int j = 0;
	bool bfind = false;

//  WORD* pname = (WORD*)pfilename;
	STREAM *pstream = NULL;
	DRIVE *pdrv = NULL;

	pdrv = DriveChange(CF);
	if ((DirReset(pdrv) != FS_SUCCEED) && (DirFirst(pdrv) != FS_SUCCEED))
	{
		return -1;
	}

	for (; i < 65535; i++)
	{
		pstream = FileOpen(pdrv);
		if (!pstream)
			return -1;
		if ((pdrv->Node->Extension[0] == 'L')
			&& (pdrv->Node->Extension[1] == 'R') && (pdrv->Node->Extension[2] == 'C'))
		{
			break;
		}
		FileClose(pstream);
		pstream = NULL;
		if (DirNext(pdrv) != FS_SUCCEED)
		{
			return -1;
		}
	}
	if (pstream == NULL)
	{
		return -1;
	}
#endif

	if (pLrcHandle == NULL)
	{
		return -1;
	}

	filelen = FileSizeGet(pLrcHandle);

#if 0 //original 2010.07.28, only support ASCII and UTF8
	
		pfilecontent = (char *) mem_malloc(filelen + 1);
	
		if (NULL == pfilecontent)
		{
			return -1;
		}
		memset(pfilecontent, 0, filelen + 1);
		FileRead(pLrcHandle, pfilecontent, filelen);
	
#endif

	if (MusicLyricState == 0) // for GBK
	{
		lrc_encoding_format = CHAR_ENCODING_GBK;
	}
	else if (MusicLyricState == 1) // for BIG5
	{
		lrc_encoding_format = CHAR_ENCODING_BIG5;
	}
	else if (MusicLyricState == 2) // for Unicode
	{
		SeekSet(pLrcHandle);
		stream_t *st = sub_stream_create(pLrcHandle);
		lrc_encoding_format = lrc_auto_detect((stream_t *)st);  //auto detect unicode format
		//MP_DEBUG1("-I- lrc_encoding_format = %d", lrc_encoding_format);
		mem_free(st);
		SeekSet(pLrcHandle);
	}



#if 1
	switch(lrc_encoding_format)
	{
	case CHAR_ENCODING_GBK:
	case CHAR_ENCODING_GB_18030:
	case CHAR_ENCODING_GB_2312:
		if (lrc_encoding_format == CHAR_ENCODING_GBK)
			MP_DEBUG("-I- CHAR_ENCODING_GBK");

	case CHAR_ENCODING_BIG5:
		if (lrc_encoding_format == CHAR_ENCODING_BIG5)
			MP_DEBUG("-I- CHAR_ENCODING_BIG5");

	case CHAR_ENCODING_UTF_8:
		if (lrc_encoding_format == CHAR_ENCODING_UTF_8)
			MP_DEBUG("-I- CHAR_ENCODING_UTF_8");

		pfilecontent = (char *) mem_malloc(filelen + 1);
		if (NULL == pfilecontent)
		{
			return -1;
		}
		memset(pfilecontent, 0, filelen + 1);
		FileRead(pLrcHandle, pfilecontent, filelen);
		
		break;

	case CHAR_ENCODING_UTF_16_LE:
		if (lrc_encoding_format == CHAR_ENCODING_UTF_16_LE)
			MP_DEBUG("-I- CHAR_ENCODING_UTF_16_LE");
		
	case CHAR_ENCODING_UTF_16_BE:
		if (lrc_encoding_format == CHAR_ENCODING_UTF_16_BE)
			MP_DEBUG("-I- CHAR_ENCODING_UTF_16_BE");

		pfilecontenttemp = (char *) mem_malloc(filelen + 2);
		if (NULL == pfilecontenttemp)
		{
			return -1;
		}
		memset(pfilecontenttemp, 0, filelen + 2);
				
		pfilecontent = (char *) mem_malloc(filelen);
		if (NULL == pfilecontent)
		{
			return -1;
		}
		memset(pfilecontent, 0, filelen);
		
		pfilecontenttemp_len = filelen;
		
		SeekSet(pLrcHandle);
		FileRead(pLrcHandle, pfilecontenttemp, pfilecontenttemp_len);

		filelen = lrc_utf16_to_utf8((uint8_t*)pfilecontent, (int*) filelen, (const uint8_t *) pfilecontenttemp,(int ) &pfilecontenttemp_len ,lrc_encoding_format,0);

		mem_free(pfilecontenttemp);

		break;		

	case CHAR_ENCODING_UTF_32_BE:
	case CHAR_ENCODING_UTF_32_LE:
	case CHAR_ENCODING_UTF_7:
	case CHAR_ENCODING_UTF_1:
	case CHAR_ENCODING_UTF_EBCDIC:
	case CHAR_ENCODING_SCSU:
	case CHAR_ENCODING_BOCU_1:
	
	case -1:
	default:
		MP_DEBUG("Not supported lrc encoding format");
		break;
	}

#endif 

	parsing_idtags(pfilecontent);
	parsing_timetags(pfilecontent);

	mem_free(pfilecontent);

	FileClose(pLrcHandle);

	return 0;

}

/**********************************************
*
*	get id tages from the lyric file, 
*	such as artist, title, album, editor, display offset
*
**********************************************/
static void parsing_idtags(char *pcontent)
{
	//MP_DEBUG1("%s", __func__);
	artist = get_artist(pcontent);
	title = get_title(pcontent);
	album = get_album(pcontent);
	offset = get_offset(pcontent);
	editor = get_editor(pcontent);
}
static int parsing_oneline(char *pline)
{
    //MP_DEBUG1("%s", __func__);

	char *lb = NULL;
	char *rb = NULL;
	char *colon = NULL;;
	char *sp = NULL;
	int lrc_pos = 0;
	char *lrc = NULL;
	int itime = 0;
	int istrlen = 0;
	char* utf16_lrc = NULL;
	char* lrc_pointer;
	BOOL flag_spc = FALSE;
	BOOL flag_no_spc = FALSE;
	int i;

	if (NULL != (lb = strchr(pline, '['))
		&& NULL != (colon = strchr(lb, ':')) && NULL != (rb = strchr(colon, ']'))) // find first characteres ('[',':' and ']') and time, later store the lyric of the line, note by Eddyson
	{
		/*
		MP_DEBUG1("pline %s",pline);
		MP_DEBUG1("[ %s",lb);
		MP_DEBUG1(": %s",colon);
		MP_DEBUG1("] %s",rb);
		*/

		PLRC_TIMETAG p =
			(PLRC_TIMETAG) mem_reallocm(lrc_timetags, sizeof(LRC_TIMETAG) * (cur_timetag + 1));

		if (p != 0)
		{
			lrc_timetags = p;
			lrc_timetags[cur_timetag].bfilled = false;
			lrc_timetags[cur_timetag].lyric = NULL;
			itime = atoi(lb + 1) * 60 * 1000;
			itime += atoi(colon + 1) * 1000;
			//itime += (int)(atof(colon+1)*1000);
			sp = strchr(colon, '.');
			if ((sp != NULL) && (sp < rb))
			{
				switch (rb - sp - 1)
				{
				case 3:
					itime += atoi(sp + 1);
					break;
				case 2:
					itime += atoi(sp + 1) * 10; //Calculate miliseconds, note by Eddyson
					break;
				case 1:
					itime += atoi(sp + 1) * 100;
					break;
				default:
					break;
				}
			}

			if ((itime - offset) >= 0 )
			{
			   lrc_timetags[cur_timetag].time = itime - offset;
			   MP_DEBUG1("lrc_timetags[cur_timetag].time %d",lrc_timetags[cur_timetag].time);
			   MP_DEBUG1("cur_timetag %d", cur_timetag);
			   cur_timetag++;
			}
			
		}

		parsing_oneline(rb + 1);
	}
	else
	{
		lrc_pos = cur_timetag - 1;
		while ((lrc_pos >= 0) && !lrc_timetags[lrc_pos].bfilled)
		{
			if (pline[strlen(pline) - 1] == '\r')
			{
				istrlen = strlen(pline) - 1;
			}
			else
			{
				istrlen = strlen(pline);
			}
			
			if (lrc_pos == 0) //avoid the first line to be an empty line
				{
					for (i = 0; i <= istrlen; i++)
					{
						//MP_DEBUG2("pline[%d] = %x",i ,pline[i] );
						if (pline[0] == 0x20)
							{
								flag_spc = TRUE;
							}
						if (pline[i] != 0x20 && pline[i] != 0x0 && pline[i] != 0xd)
							{
								flag_no_spc = TRUE;
							}
					}
				if (flag_spc==TRUE && flag_no_spc==FALSE)
				{
					lrc_timetags[lrc_pos].bfilled = false;
					cur_timetag--;
					return;
				}
			}				
			
			#if 0 //original 2010.07.28, only support ASCII and UTF8

			lrc = (char *) mem_malloc(istrlen*2 + 1);
							
			//utf16_lrc = (char *) mem_malloc(istrlen*2 + 1);

			sub_trail_return_char_lrc(pline);
			
			memset(lrc, 0, istrlen*2 + 1);
			//memset(utf16_lrc, 0, istrlen*2 + 1);
			utf16_lrc = strncpy(lrc, pline, istrlen);
			#endif

			if (istrlen!=0)
			{
			switch(lrc_encoding_format)
				{
				case CHAR_ENCODING_UTF_8:
				case CHAR_ENCODING_UTF_16_LE:
				case CHAR_ENCODING_UTF_16_BE:
					lrc = (char *) mem_malloc(istrlen*4 + 2 + 2);
					utf16_lrc = (char *) mem_malloc(istrlen*4 + 2 + 2);
					
					sub_trail_return_char_lrc(pline);
					
					memset(lrc, 0, istrlen*4 + 2 + 2);
					memset(utf16_lrc, 0, istrlen*4 + 2 + 2);

					mpx_UtilUtf8ToUnicodeU16((U16 *) lrc, (U08 *) pline); // Convert UTF8 to UTF16BE
					break;

				case CHAR_ENCODING_GBK:
				case CHAR_ENCODING_GB_2312:
				case CHAR_ENCODING_GB_18030:
				case CHAR_ENCODING_BIG5:

					lrc = (char *) mem_malloc(128);
					utf16_lrc = (char *) mem_malloc(istrlen*4 + 2 + 2);
					
					sub_trail_return_char_lrc(pline);
					
					memset(lrc, 0, istrlen*4 + 2 + 2);
					memset(utf16_lrc, 0, istrlen*4 + 2 + 2);

					Lrc_Line_Cut_ASCII_and_GBK_BIG5((char *)pline);
//					mem_reallocm(lrc,128);

					Lrc_Line_Merge_GB_BIG5_To_Uni((char *)pline, (char *)lrc);


					break;
					
				case CHAR_ENCODING_UTF_32_BE:
				case CHAR_ENCODING_UTF_32_LE:
				case CHAR_ENCODING_UTF_7:
				case CHAR_ENCODING_UTF_1:
				case CHAR_ENCODING_UTF_EBCDIC:
				case CHAR_ENCODING_SCSU:
				case CHAR_ENCODING_BOCU_1:
					
				case -1:
				default:
					break;
				}
			}
			else
			{
				lrc = (char *) mem_malloc(istrlen*4 + 2 + 2);
				utf16_lrc = (char *) mem_malloc(istrlen*4 + 2 + 2);
				memset(lrc, 0, istrlen*4 + 2 + 2);
				memset(utf16_lrc, 0, istrlen*4 + 2 + 2);
			}

			//MP_DEBUG1("lrc= %s",lrc);

			utf16_lrc[0] = 0xFF;
			utf16_lrc[1] = 0xFE;
			
			sub_wcscat(utf16_lrc, lrc);
			if (istrlen == 0)
				utf16_lrc[3] = '\0';


			//if (utf16_lrc == NULL)
				//cur_timetag--;
			lrc_timetags[lrc_pos].lyric = utf16_lrc;
			lrc_timetags[lrc_pos].bfilled = true;
			lrc_timetags[lrc_pos].bdisplaying = false;
			lrc_pos--;
			
			if (lrc !=NULL)
				lrc = NULL;

			if (utf16_lrc != NULL)
				utf16_lrc= NULL;
			
			mem_free(lrc);
			mem_free(utf16_lrc);

		}
	}

	return 1;
}

static void parsing_timetags(char *pcontent)
{
    MP_DEBUG1("%s", __func__);

	char *pline_start = pcontent;
	char *pline_end = strchr(pcontent, '\n');
	//char *pline_end = sub_stristr(pcontent, '\n'); // Avi subtitle function
	char *pline = NULL;
	int ilen = 0,i=0;

	while (pline_end != NULL)
	{
		ilen = pline_end - pline_start;
		pline = (char *) mem_malloc(ilen + 1);
		if (NULL == pline)
		{
			break;
		}
		memset(pline, 0, ilen + 1);
		strncpy(pline, pline_start, ilen);
		parsing_oneline(pline);
		if (cur_timetag != 0)
		{
			lrc_timetags[cur_timetag - 1].bfilled = 1;
		}
		mem_free(pline);
		pline_start = pline_end + 1;
		pline_end = strchr(pline_end + 1, '\n');
		//pline_end = sub_stristr(pline_end + 1, '\n'); // Avi subtitle function
	}
	if (pline_start != NULL)
	{
		parsing_oneline(pline_start);
	}
	total_timetag = cur_timetag;
	//sort_timetags(); //Bubblesort function for lyric
	quicksort_timetags(0,total_timetag - 1);//Quicksort function for lyric, by Eddyson 2010.07.09

	//check lyric sequence after sorting
	MP_DEBUG1("-I- total_timetag = %d",total_timetag); //cur_timetag start counting from zero
	#if 0
	for (cur_timetag = 0; cur_timetag < total_timetag; cur_timetag++)
	{
		MP_DEBUG("-----");
		MP_DEBUG1("cur_timetag = %d",cur_timetag);
		MP_DEBUG1("timetag = %d",lrc_timetags[cur_timetag].time);
		MP_DEBUG1("lyric = %s",lrc_timetags[cur_timetag].lyric);
		//MP_DEBUG1("lyric = %x",lrc_timetags[cur_timetag].lyric[3]);	
		MP_DEBUG("-----");
	}
	#endif
	cur_timetag = 0;

}

//Separate ASCII and Chinese characters
void Lrc_Line_Cut_ASCII_and_GBK_BIG5(char *pline) 
{
	MP_DEBUG1("%s", __func__);
	
	int jj;
	int lrc_size;
	BOOL ascii_flag = FALSE;
	BOOL not_ascii_flag = FALSE;

	lrc_size = strlen(pline);
	cur_lrc_line_part = 0;
	total_lrc_line_parts = 0;

	PLRC_LINE p = (PLRC_LINE) mem_reallocm(lrc_line, sizeof(LRC_LINE) * (lrc_size + 1));
	lrc_line = p;
	lrc_line[cur_lrc_line_part].bfilled = FALSE;
	
	MP_DEBUG1("-I- Number of bytes of the line = %d",lrc_size);

    for (jj = 0; jj <= lrc_size; jj++) // Search how many ASCII and Big5/GBK  parts, and count pointers
    {
		/*
		MP_DEBUG1("====jj=%d====",jj);
		MP_DEBUG2("pline[%d] = %x",jj,pline[jj]);
		MP_DEBUG2("not_ascii_flag = %d, ascii_flag = %d",not_ascii_flag,ascii_flag);
		MP_DEBUG1("total_lrc_line_parts = %d",total_lrc_line_parts);
		*/


		if (((pline[jj] >= 0x20)&&(pline[jj] <= 0x40))||((pline[jj] == 0x5B)&&(pline[jj] <= 0x60))||((pline[jj] == 0x7B)&&(pline[jj] <= 0x7F)))
		{
			if ((not_ascii_flag == 1)&&(lrc_line[cur_lrc_line_part].start_pt + 1 < jj))
			{
				if (((jj-1) - lrc_line[cur_lrc_line_part].start_pt + 1)%2 == 0) //detect if is the 2nd byte of GBK/BIG5
				{
					
					MP_DEBUG1("GBK/BIG5 before special character at byte %d",(jj - 1));
					lrc_line[cur_lrc_line_part].end_pt = jj - 1;
					lrc_line[cur_lrc_line_part].bfilled = TRUE;
					memset(lrc_line[cur_lrc_line_part].line_part,0,128);
					cur_lrc_line_part++;
					not_ascii_flag = 0;
						
				}
			}

			if ((ascii_flag == 1)) //Find ASCII end point
			{
				if  ((jj-1==0)||(((pline[jj-1] >= 0x41) && (pline[jj-1] <= 0x5A))||((pline[jj-1] >= 0x61) && (pline[jj-1] <= 0x7A))	)
					|| (((pline[jj-2] >= 0x41) && (pline[jj-2] <= 0x5A))||((pline[jj-2] >= 0x61) && (pline[jj-2] <= 0x7A))))
				{
					MP_DEBUG1("ASCII before special character at byte %d",(jj - 1));
					ascii_flag = 0;
					lrc_line[cur_lrc_line_part].end_pt = jj - 1;
					lrc_line[cur_lrc_line_part].bfilled = TRUE;
					memset(lrc_line[cur_lrc_line_part].line_part,0,128);
					cur_lrc_line_part++;
				}
			}			

		if ( (not_ascii_flag == 0)&&(ascii_flag == 0) )
		{
			MP_DEBUG1("Special character at byte %d ", jj);
			lrc_line[cur_lrc_line_part].seq_num = cur_lrc_line_part;
			lrc_line[cur_lrc_line_part].start_pt = jj;
			lrc_line[cur_lrc_line_part].ascii_flag = TRUE;
			lrc_line[cur_lrc_line_part].end_pt = jj;
			lrc_line[cur_lrc_line_part].bfilled = TRUE;
			memset(lrc_line[cur_lrc_line_part].line_part,0,128);
			cur_lrc_line_part++;

		}
		}

		
		if (((pline[jj] >= 0x41) && (pline[jj] <= 0x5A))||((pline[jj] >= 0x61) && (pline[jj] <= 0x7A)))//Find ASCII start point
		{

			if (not_ascii_flag == 1) //Find Big5/GBK end point
			{
				if ((jj - lrc_line[cur_lrc_line_part].start_pt + 1)%2 != 0) //detect if is the 2nd byte of GBK/BIG5
				{
					
					MP_DEBUG1("BIG5 or GBK end point at byte %d",(jj-1));
					lrc_line[cur_lrc_line_part].end_pt = jj - 1;
					lrc_line[cur_lrc_line_part].bfilled = TRUE;
					//MP_DEBUG1("BIG5orGBK End Point = %d",lrc_line[cur_lrc_line_part].end_pt);
					memset(lrc_line[cur_lrc_line_part].line_part,0,128);
					cur_lrc_line_part++;
					not_ascii_flag = FALSE;
					
				}
				
			}
			if ( ((ascii_flag == 0)) && (not_ascii_flag == 0) )//Find ASCII start point
			{
				MP_DEBUG1("ASCII start point at byte %d",jj);

				ascii_flag = 1;
				lrc_line[cur_lrc_line_part].seq_num = cur_lrc_line_part;
				lrc_line[cur_lrc_line_part].start_pt = jj;
				lrc_line[cur_lrc_line_part].ascii_flag = TRUE;
				//MP_DEBUG1("ASCII Start Point = %d",lrc_line[cur_lrc_line_part].start_pt);
				
			}
		}
		else if ((pline[jj] < 0x20) || (pline[jj] > 0x7F)) //Big5 or GBK
		{

			if ((ascii_flag == 1)) //Find ASCII end point
			{
				if ((jj-1==0) || 
					(((pline[jj-1] >= 0x41) && (pline[jj-1] <= 0x5A))||((pline[jj-1] >= 0x61) && (pline[jj-1] <= 0x7A))))
				{
					MP_DEBUG1("ASCII end point at byte %d",(jj-1));
					ascii_flag = 0;
					lrc_line[cur_lrc_line_part].end_pt = jj - 1;
					lrc_line[cur_lrc_line_part].bfilled = TRUE;
					//MP_DEBUG1("ASCII End Point = %d",lrc_line[cur_lrc_line_part].end_pt);
					memset(lrc_line[cur_lrc_line_part].line_part,0,128);
					cur_lrc_line_part++;
				}

				else
				{
					lrc_line[cur_lrc_line_part].ascii_flag = FALSE;
					ascii_flag = 0;
				}
			}

			if ((not_ascii_flag == 0)) //Find Big5 or GBK start point
			{
				MP_DEBUG1("BIG5 or GBK start point at byte %d",jj);

				lrc_line[cur_lrc_line_part].seq_num = cur_lrc_line_part;
				lrc_line[cur_lrc_line_part].start_pt = jj;
				lrc_line[cur_lrc_line_part].ascii_flag = FALSE;
				not_ascii_flag = 1;
			}
		}

		total_lrc_line_parts = cur_lrc_line_part;

		if (jj == lrc_size)
		{
			MP_DEBUG1("Last point at byte %d",jj);
			lrc_line[cur_lrc_line_part].end_pt = jj;
			break;
		}
		
		//MP_DEBUG2("not_ascii_flag = %d, ascii_flag = %d",not_ascii_flag,ascii_flag);
		//MP_DEBUG1("====jj=%d====",jj);
		
    }

MP_DEBUG1("-I- total_lrc_line_parts (starts from 0) = %d", total_lrc_line_parts+1);


cur_lrc_line_part = 0;
for (cur_lrc_line_part = 0; cur_lrc_line_part <= total_lrc_line_parts; cur_lrc_line_part++) //Cut the ASCII and Big5/GBK  parts
{
	/*
	MP_DEBUG1("====cur_lrc_line_part = %d",jj);
	MP_DEBUG1("start pt = %d",lrc_line[jj].start_pt);
	MP_DEBUG1("end pt = %d",lrc_line[jj].end_pt);
	*/

	for (jj = lrc_line[cur_lrc_line_part].start_pt; jj <= lrc_line[cur_lrc_line_part].end_pt; jj++)
	{
		lrc_line[cur_lrc_line_part].line_part[jj - lrc_line[cur_lrc_line_part].start_pt] = pline[jj];
		/*
		MP_DEBUG2("line_part[%d]: %x",jj,lrc_line[cur_lrc_line_part].line_part[jj - lrc_line[cur_lrc_line_part].start_pt]);
		MP_DEBUG2("pline[%d]: %x",jj,pline[jj]);
		MP_DEBUG2("pline[%d]: %s",jj,pline[jj]);
		MP_DEBUG2("line_part[%d]: %s",cur_lrc_line_part,lrc_line[cur_lrc_line_part].line_part);
		*/
	}	
}

#if 0 //check each byte
cur_lrc_line_part=0;
int kk;
for (kk =cur_lrc_line_part; kk<=total_lrc_line_parts; kk++)
{	
	MP_DEBUG1("==kk=%d==",kk);
	MP_DEBUG1("seq_num %d",lrc_line[kk].seq_num);
	MP_DEBUG1("start_pt %d",lrc_line[kk].start_pt);
	MP_DEBUG1("end_pt %d",lrc_line[kk].end_pt);
	MP_DEBUG1("ascii_flag %d",lrc_line[kk].ascii_flag);
	MP_DEBUG("--");
	MP_DEBUG2("line[%d].line_part: %x",kk,lrc_line[kk].line_part);
	MP_DEBUG2("line[%d].line_part: %s",kk,lrc_line[kk].line_part);
	for (jj = lrc_line[kk].start_pt; jj <= lrc_line[kk].end_pt; jj++)
	{
		MP_DEBUG3("line[%d].line_part[%d]: %x",kk,jj,lrc_line[kk].line_part[jj]);
		MP_DEBUG2("pline[%d]: %x",jj,pline[jj]);	
	}
	MP_DEBUG("--");
	MP_DEBUG1("==kk=%d==",kk);
}
#endif

}


//Convert GBK/Big5/ASCII to UTF16 and merge all parts
void Lrc_Line_Merge_GB_BIG5_To_Uni(char *pline, char *lrc) 
{
	MP_DEBUG1("%s", __func__);
	char *add_line_part1;
	char *add_line_part2;
	//char line_part_tmp[128];
	char *line_part_tmp;
	int count_line_size = 0;
	int count_line_size_tmp =0;
	line_part_tmp = (char *)mem_malloc(128);

	for (cur_lrc_line_part = 0; cur_lrc_line_part <= total_lrc_line_parts; cur_lrc_line_part++) // Convert each part to UTF16
	{
		//MP_DEBUG1("cur_lrc_line_part = %d",cur_lrc_line_part);
		if (!lrc_line[cur_lrc_line_part].ascii_flag) //BIG5 to UTF16
		{
			MP_DEBUG("GBK/BIG5 to UTF16");
			count_line_size_tmp = (lrc_line[cur_lrc_line_part].end_pt - lrc_line[cur_lrc_line_part].start_pt + 1);
			memset(line_part_tmp, 0, 128);

			memcpy(line_part_tmp, lrc_line[cur_lrc_line_part].line_part, count_line_size_tmp);
			memset(lrc_line[cur_lrc_line_part].line_part, 0, 128);

			if (MusicLyricState == 0)
			{
				MP_DEBUG("-I- GBK to UTF16-BE");
				//GB2312ToUnicode((U16 *) lrc_line[cur_lrc_line_part].line_part, (U16 *) line_part_tmp);
				GBToUnicode((U16 *)lrc_line[cur_lrc_line_part].line_part, (U16 *)line_part_tmp);
			}
			else if (MusicLyricState == 1)
			{
				MP_DEBUG("-I- BIG5 to UTF16-BE");
				mpx_UtilBig5ToUnicodeU16( (U16 *) lrc_line[cur_lrc_line_part].line_part, (U16 *) line_part_tmp);
			}

			count_line_size = count_line_size + count_line_size_tmp;
			//MP_DEBUG1("count_line_size = %d", count_line_size);			
		}
		
		else if (lrc_line[cur_lrc_line_part].ascii_flag) //ASCII to UTF16
		{
			MP_DEBUG("ASCII to UTF16");
			
			count_line_size_tmp = (lrc_line[cur_lrc_line_part].end_pt - lrc_line[cur_lrc_line_part].start_pt) + 1;
			memset(line_part_tmp, 0, 128);

			memcpy(line_part_tmp, lrc_line[cur_lrc_line_part].line_part, count_line_size_tmp);
			memset(lrc_line[cur_lrc_line_part].line_part, 0, 128);
			
			mpx_UtilUtf8ToUnicodeU16( (U16 *) lrc_line[cur_lrc_line_part].line_part, (U08 *) line_part_tmp);

			count_line_size = count_line_size + count_line_size_tmp;
			//MP_DEBUG1("count_line_size = %d", count_line_size);
		}

	}

	mem_free(line_part_tmp);

	add_line_part1 = (char *) mem_malloc(count_line_size + 2);
	memset(add_line_part1, 0, count_line_size + 2);
	add_line_part1 = lrc_line[0].line_part;


	if (total_lrc_line_parts > 0)
	{

		for (cur_lrc_line_part = 1; cur_lrc_line_part <= total_lrc_line_parts; cur_lrc_line_part++)
		{
			count_line_size_tmp = (lrc_line[cur_lrc_line_part].end_pt - lrc_line[cur_lrc_line_part].start_pt) + 1;
			add_line_part2 = (char *) mem_malloc(count_line_size + 2);

			memset(add_line_part2, 0, count_line_size + 2);
			add_line_part2 = lrc_line[cur_lrc_line_part].line_part;
			sub_wcscat(add_line_part1, add_line_part2);

		}
		mem_free(add_line_part2);

	}

	mem_reallocm(lrc, count_line_size*2 + 2);
	memset(lrc, 0, count_line_size*2 + 2);
	memcpy(lrc, add_line_part1, count_line_size*2 + 2);
	mem_free(add_line_part1);

}


static char *get_tag_string(char *ptag, char *pcontent)
{
    //MP_DEBUG1("%s", __func__);
	char *pline_start = pcontent;
	char *pline_end = strchr(pcontent, '\n');
	int ilen = 0;
	char *rb = 0;
	char *strline = NULL;
	char *ret = NULL;

	while (pline_end != NULL)
	{
		ilen = pline_end - pline_start;
		strline = (char *) mem_malloc(ilen + 1);
		if (strline == NULL)
		{
			break;
		}
		memset(strline, 0, ilen + 1);
		strncpy(strline, pline_start, ilen);
		if ((strline == strstr(strline, ptag)) && (NULL != (rb = strchr(strline, ']'))))
		{
			ret = (char *) mem_malloc(rb - strline - strlen(ptag) + 1);
			if (NULL == ret)
			{
				break;
			}
			memset(ret, 0, rb - strline - strlen(ptag) + 1);
			strncpy(ret, strline + strlen(ptag), rb - strline - strlen(ptag));
			mem_free(strline);
			strcpy(pline_start, pline_end + 1);
			break;
		}
		mem_free(strline);
		pline_start = pline_end + 1;
		pline_end = strchr(pline_end + 1, '\n');
	}
	return ret;
}

static char *get_artist(char *pcontent)
{
	char *art = NULL;

	art = get_tag_string("[ar:", pcontent);
	if (NULL == art)
	{
		art = get_tag_string("[AR:", pcontent);
	}
	return art;
}

static char *get_title(char *pcontent)
{
	char *ti = NULL;

	ti = get_tag_string("[ti:", pcontent);
	if (NULL == ti)
	{
		ti = get_tag_string("[TI:", pcontent);
	}
	return ti;
}

static char *get_album(char *pcontent)
{
	char *al = NULL;

	al = get_tag_string("[al:", pcontent);
	if (NULL == al)
	{
		al = get_tag_string("[AL:", pcontent);
	}
	return al;
}

static char *get_editor(char *pcontent)
{
	char *by = NULL;

	by = get_tag_string("[by:", pcontent);
	if (NULL == by)
	{
		by = get_tag_string("[BY:", pcontent);
	}
	return by;
}

static int get_offset(char *pcontent)
{
	char *stroffset = get_tag_string("[offset:", pcontent);
	int ioffset = 0;

	if (stroffset)
	{
		ioffset = atoi(stroffset);
		mem_free(stroffset);
	}
	return ioffset;
}

/*********************************************
*FUNCTION: Lrc_destroy
*ARGUMENT: void
*RETURN VALUE: void
*COMMENTS: release the resource 
**********************************************/
void Lrc_destroy()
{
    MP_DEBUG1("%s", __func__);

	if (artist != NULL)
	{
		mem_free(artist);
		artist = NULL;
	}
	if (title != NULL)
	{
		mem_free(title);
		title = NULL;
	}
	if (album != NULL)
	{
		mem_free(album);
		album = NULL;
	}
	if (editor != NULL)
	{
		mem_free(editor);
		editor = NULL;
	}
	int i = 0;

	if (lrc_timetags != NULL)
	{
		while (lrc_timetags[i].bfilled && i < total_timetag)
		//while (i < total_timetag)
		{
			if (lrc_timetags[i].lyric != NULL)
			{
				//MP_DEBUG1("--i = %d",i);
				mem_free(lrc_timetags[i].lyric);
				lrc_timetags[i].lyric = NULL;
			}
			lrc_timetags[i].bfilled = false;
			lrc_timetags[i].bdisplaying = NULL;
			i++;
		}
		//mem_free(lrc_timetags);
		lrc_timetags = NULL;
	}
	total_timetag = 0;
	Line_destroy();
}

void Line_destroy()
{
    MP_DEBUG1("%s", __func__);

	int i = 0;
	if (lrc_line != NULL)
	{
	while (lrc_line[i].bfilled && i < total_lrc_line_parts)
	{
		if (lrc_line[i].line_part != NULL)
		{
			mem_free(lrc_line[i].line_part);
			//lrc_line[i].line_part= NULL;
		}
		lrc_line[i].bfilled = false;
		i++;
	}
	}
	//mem_free(lrc_timetags);
	lrc_line = NULL;
	total_lrc_line_parts = 0;
}


/*****************************************
*FUNCTION: Lrc_show
*ARGUMENT: int time
*RETURN VALUE: pointer to the string of lyric
*COMMENT: retreive the lyric string of most near to and below time 
*                 if not found, it will return NULL
*****************************************/
char *Lrc_show(int time)
{
    MP_DEBUG1("%s", __func__);

	if (lrc_timetags == NULL)
	{
		return NULL;
	}
	bool bRev = false;

	cur_timetag = cur_timetag > total_timetag ? total_timetag : cur_timetag;
	if (time < lrc_timetags[cur_timetag].time)
	{
		bRev = 1;
	}
	if (bRev)
	{
		while (cur_timetag > 0)
		{
			lrc_timetags[cur_timetag].bdisplaying = false;
			cur_timetag--;
			if (time >= lrc_timetags[cur_timetag].time)
			{
				return lrc_timetags[cur_timetag].lyric;
			}
		}
	}
	else
	{
		while (cur_timetag < total_timetag - 1)
		{
			if (lrc_timetags[cur_timetag + 1].time > time)
			{
				return lrc_timetags[cur_timetag].lyric;
			}
			lrc_timetags[cur_timetag].bdisplaying = false;
			cur_timetag++;
		}
		if (cur_timetag == total_timetag - 1)
		{
			return lrc_timetags[cur_timetag].lyric;
		}
	}

	return NULL;

}


char *Find_cur_timetag(DWORD time)
{
	MP_DEBUG1("%s", __func__);
	if (lrc_timetags == NULL)
	{
		return NULL;
	}
	bool bRev = false;

	cur_timetag = cur_timetag > total_timetag ? total_timetag : cur_timetag;
	if (time < lrc_timetags[cur_timetag].time)
	{
		bRev = 1;
	}
	if (bRev)
	{
		while (cur_timetag > 0)
		{
			lrc_timetags[cur_timetag].bdisplaying = false;
			cur_timetag--;
			if (time >= lrc_timetags[cur_timetag].time)
			{
				return cur_timetag;
			}
		}
	}
	else
	{
		while (cur_timetag < total_timetag - 1)
		{
			if (lrc_timetags[cur_timetag + 1].time > time)
			{
				return cur_timetag;
			}
			lrc_timetags[cur_timetag].bdisplaying = false;
			cur_timetag++;
		}
		if (cur_timetag == total_timetag - 1)
		{
			return cur_timetag;
		}
	}
	return NULL;

}



//bubblesort lyrics, note by Eddyson 2010.07.09
static void sort_timetags(void)
{
	//MP_DEBUG1("%s", __func__);
	int i = 0;
	int j = i;

	for (; i < total_timetag - 1; i++)
	{
		for (j = i + 1; j < total_timetag; j++)
		{
			if (lrc_timetags[j].time < lrc_timetags[i].time)
			{
				LRC_TIMETAG tmp;

				memcpy(&tmp, &lrc_timetags[i], sizeof(LRC_TIMETAG));
				memcpy(&lrc_timetags[i], &lrc_timetags[j], sizeof(LRC_TIMETAG));
				memcpy(&lrc_timetags[j], &tmp, sizeof(LRC_TIMETAG));

			}
		}
	}
} 


//Quicksort lyrics by timetags, by Eddyson 2010.07.09
void quicksort_timetags(int left, int right)
{
    //MP_DEBUG1("%s", __func__);

	int i, j;
	int x;
	LRC_TIMETAG tmp;

	i = left; j = right;
	x = lrc_timetags[(left+right)/2].time;

	do
	{
		while((lrc_timetags[i].time < x) && (i < right))
			i++;
		while((lrc_timetags[j].time > x) && (j > left))
			j--;
		if(i <= j)
		{
			memcpy(&tmp, &lrc_timetags[i], sizeof(LRC_TIMETAG));
			memcpy(&lrc_timetags[i], &lrc_timetags[j], sizeof(LRC_TIMETAG));
		  	memcpy(&lrc_timetags[j], &tmp, sizeof(LRC_TIMETAG));
	      	i++; j--;
    	}
	} while(i <= j);
	if(left < j) quicksort_timetags(left, j);
	if(i < right) quicksort_timetags(i, right);
}


/* Remove leading and trailing \r, function copied from sub_common.c */
void sub_trail_return_char_lrc(char *s) 
{
    //MP_DEBUG1("%s", __func__);
	int i = 0;
	while (s[i] == '\r') ++i;
	if (i) strcpy(s, s + i);
	i = strlen(s) - 1;
	while (i > 0 && s[i] == '\r') s[i--] = '\0';
}


//for forward/rewind function, but not used
int lrc_pos_seek(DWORD ms, BOOL forward) 
{
    MP_DEBUG1("%s", __func__);
	//__asm("break 100");
	MP_DEBUG2("lrc_pos_seek ms = %d, forward = %d", ms, forward);
	cur_timetag = Lrc_show((int) ms);
	MP_DEBUG2("lrec_pos_seek_cur_timetag=%d, time=%d",cur_timetag,lrc_timetags[cur_timetag].time);
	
    if (forward)
    {
        //mpDebugPrint("g_SubNodeTail = 0x%x", g_SubNodeTail);
        while (lrc_timetags[cur_timetag].time < (lrc_timetags[cur_timetag].time + ms))
        {
            //mpDebugPrint("g_SubNodeCur = 0x%x, g_SubNodeCur->s_sub.start = %d, ms = %d", g_SubNodeCur, g_SubNodeCur->s_sub.start, ms);
            if (lrc_timetags[cur_timetag].bdisplaying)
            {
                lrc_timetags[cur_timetag].bdisplaying = FALSE;

                if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgUpdateOsdLrc)
                {
					for (cur_line = 0; cur_line < total_line; cur_line++)
					{	
					MP_DEBUG("forward clean");
					g_pXpgFuncPtr->xpgUpdateOsdLrc("",x_lrc, y_lrc + (line_dist*cur_line));
					}
                }
		
            }	
			if (lrc_timetags[cur_timetag].time != lrc_timetags[total_timetag].time)
               cur_timetag++;
			else
			   break;	
        } 
    }
	else // backward
	{
	    //BOOL match = FALSE;
        MP_DEBUG2("lrc_pos_seek ms = %d, backward = %d", ms, forward);
	    while (lrc_timetags[cur_timetag].time > (lrc_timetags[cur_timetag].time + ms))
        {
            
            if (lrc_timetags[cur_timetag].bdisplaying)
            {
                lrc_timetags[cur_timetag].bdisplaying = FALSE;

                if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgUpdateOsdLrc)
                {
					for (cur_line = 0; cur_line < total_line; cur_line++)
					{	
						MP_DEBUG("backward clean");
						MP_DEBUG2("cur_timetag=%d, time=%d",cur_timetag,lrc_timetags[cur_timetag].time);
						g_pXpgFuncPtr->xpgUpdateOsdLrc("",x_lrc, y_lrc + (line_dist*cur_line));
					}
                }
            }	
			if (lrc_timetags[cur_timetag].time != lrc_timetags[total_timetag].time)
               cur_timetag--;

			else
				break;
			//mpDebugPrint("g_SubNodeCur = 0x%x, g_SubNodeCur->s_sub.start = %d, ms = %d", g_SubNodeCur, g_SubNodeCur->s_sub.start, ms);
        } 

		//if ((g_SubNodeCur->s_sub.end < ms) && (g_SubNodeCur != g_SubNodeTail))
		//   g_SubNodeCur = g_SubNodeCur->p_next;
	}
	
	return PASS;
}

//detects lyrics of format: utf8, utf16be, utf16le
int lrc_auto_detect (stream_t* st) 
{
    MP_DEBUG1("%s", __func__);
    char line[SUB_LINE_LEN+1];
    int i,j = 0;
    char p;
	int utf16_source = 0; //0 = UTF-8/ASCII/other, 1 = UTF-16-LE, 2 = UTF-16-BE
    int utf16_dst = 0; // the function only supports converting to UTF8
	int num;
	//int uses_time = 0;
	//int	lrc_encoding_format;
	lrc_encoding_format = -1;


	for  (utf16_source = 0; utf16_source >= 0 && utf16_source < 11; utf16_source++)
	{

		//for (utf16_dst = 0; utf16_dst >= 0 && utf16_dst < 11; utf16_dst++) 
		{
			j=0;
			while (j < 50) 
			{
				j++;

				MP_DEBUG2("utf16_source= %d, utf16_dst= %d", utf16_source, utf16_dst);

				if (sub_read_line(st, line, SUB_LINE_LEN, (E_CHAR_ENCODING) utf16_source, (E_CHAR_ENCODING) utf16_dst)!=NULL)
				{

					MP_DEBUG1("line=%s",line);
					num = sscanf(line, "[%d:%d.%d]",	  &i, &i, &i);
					MP_DEBUG1("num=%d",num);
					
					if (num == 3)
					{
						MP_DEBUG1("-I- lrc encoding format = %d", utf16_source );
						//return SUB_MP3_LYRIC;
						lrc_encoding_format = utf16_source;
						return lrc_encoding_format;
					}

				}
			}

			sub_stream_reset(st);
	        sub_stream_seek_long(st, 0);
			if (lrc_encoding_format != -1)
			break;
		}

	}
	
	return -1;  // too many bad lines
}

U16 *GB2312ToUnicode(U16 * target, U16 * source)
{
	MP_DEBUG1("%s", __func__);
	U16 code, *result, *ptr_UTF16;
    U08 *ptr_ch;

//*source=0xd39b;
    result = target;
    
    ptr_UTF16 = source;
    while (*ptr_UTF16)
    {
        ptr_ch = (U08 *) ptr_UTF16;
        code = mpx_Gb2312ToUnicode(&ptr_ch);  

        *target = code;
		
        ptr_UTF16++;
        target++;
    }
    *target = 0;

    return result;
}

U16 GBKGB2312ToUnicode(void ** string)
{
	MP_DEBUG1("%s", __func__);
	U16 code;
    U08 *gb_point;
    gb_point = *string;

	code = GBKToUnicode((void **) &gb_point);
    if (!code)
		code = mpx_Gb2312ToUnicode((void **) &gb_point);

	return code;
}


U16 *GBToUnicode(U16 * target, U16 * source)
{
	MP_DEBUG1("%s", __func__);
	U16 code, *result, *ptr_UTF16;
    U08 *ptr_ch;

    result = target;
    ptr_UTF16 = source;
    while (*ptr_UTF16)
    {
		ptr_ch = (U16 *) ptr_UTF16;
		code = GBKGB2312ToUnicode(&ptr_ch);

        *target = code;
		
        ptr_UTF16++;
        target++;
    }
    *target = 0;

    return result;
}

U16 GBKToUnicode(void ** string)
{
	MP_DEBUG1("%s", __func__);
	U16 *gb_point;
    WORD gb, index;
	int pos, unicode;
	//BYTE high_byte, low_byte;
	
    gb_point = *string;
    gb = *gb_point;

	/*
	high_byte = gb_point[0];
	low_byte = gb_point[1];
	MP_DEBUG2("high_byte = %x \nlow_byte = %x", high_byte, low_byte );
	if ((!((0x81<=((high_byte)&0xff)) && (((high_byte)&0xff)<=0xfe))) ||
		(!((0x40<=((low_byte)&0xff)) && (((low_byte)&0xff)<=0xfe))))
	{
		MP_DEBUG("An invalid GBK character found, 0x%x%x", high_byte, low_byte);
		return 0;
	} */
	
	pos = (((gb)>>8)-0x81)*192 + (((gb)&0x00FF)-0x40);
	unicode = gbk2uni[pos];

	return unicode;

}

#endif

