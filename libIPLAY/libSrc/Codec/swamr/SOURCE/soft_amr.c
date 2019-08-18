#include "global612.h"
#include "mptrace.h"
#include "avTypeDef.h"
#include "debug.h"

#include "typedef_amr.h"
#include "cnst.h"
#include "cnst_vad.h"
#include "dtx_struct.h"
#include "amrnb_struct.h"
#include "amrnb_function.h"

// Below header files looks useless
/*
#include "basic_op.h"
#include "amrglbtable.h"
#include "table.h"
#include "amrglb.h"
#include "dtx_function.h"
*/


/*------------------------------*
 * AMR storage format           *
 *  +------------------+        *
 *  | Header           |        *
 *  +------------------+        *
 *  | Speech frame 1   |        *
 *  +------------------+        *
 *  : ...              :        *
 *  +------------------+        *
 *  | Speech frame n   |        *
 *  +------------------+        *
 *------------------------------*/

//#define honda_measure

#define AMR_MAGIC_NUMBER "#!AMR\n"
//#define AMR_MC_MAGIC_NUMBER "#!AMR_MC1.0\n"
//#define AMR_WB_MAGIC_NUMBER "#!AMR-WB\n"
//#define AMR_WB_MC_MAGIC_NUMBER "#!AMR-WB_MC1.0\n"

#define AMR_MAGIC_NUMBER_LENGTH        6
#define AMR_MC_MAGIC_NUMBER_LENGTH     12
#define AMR_WB_MAGIC_NUMBER_LENGTH     9
#define AMR_WB_MC_MAGIC_NUMBER_LENGTH  15
#define MAX_PACKED_SIZE  (MAX_SERIAL_SIZE / 8 + 2)
#define SERIAL_FRAMESIZE (1 + MAX_SERIAL_SIZE + 5)

#define AMR_CACHE_SIZE   512

unsigned int cod_rd_ind;	// Current buffer position indicator
unsigned int cod_wr_size;	// data cache size
unsigned char *chartmp;
unsigned char old_toc;
int amr_end = 0;

//Frame size of AMR modes
#if 1	// AMR IF1 frame size 
static int packed_size[16] = {12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0};
#else	// AMR IF2 frame size
static int packed_size[16] = {13, 14, 16, 18, 19, 21, 26, 31, 6, 0, 0, 0, 0, 0, 0, 0};
#endif

extern Audio_dec  Media_data;
extern int left_channel;
extern int right_channel;


static ad_info_t info = {
	"Magic Pixel AMR audio decoder",
	"MP",
	"A'rpi",
	"MP...",
	"based on S250...."
};

static int init(sh_audio_t * sh);
static int preinit(sh_audio_t * sh);
static void uninit(sh_audio_t * sh);
static int control(sh_audio_t * sh, int cmd, void *arg, ...);
static int decode_audio(sh_audio_t * sh, unsigned char *buffer, int minlen, int maxlen);

int dataStreamInit()
{
	amr_end = 0;
	cod_rd_ind = 0;
	Seek(Media_data.dec_fp, SEEK_SET);
	cod_wr_size = FileRead(Media_data.dec_fp, (BYTE*)chartmp, AMR_CACHE_SIZE);

	if(cod_wr_size == 0)
		return -1;
}

int dataread(unsigned char *serial, int datalen)
{
	int i, j = 0;
	unsigned int remainder = cod_wr_size - cod_rd_ind, readCount;

	if(datalen == 0)
		return 0;

	if( remainder <= datalen )
	{
		readCount = remainder;
	}
	else
	{
		readCount = datalen;
	}
	for( i=0; i<readCount; i++ )
	{
		serial[i] = chartmp[cod_rd_ind++];
	}

	remainder = datalen - readCount;

	if(cod_rd_ind >= cod_wr_size)
	{
		if(amr_end && remainder){
//mpDebugPrint("hehe..........");
			return -1;
		}

		cod_wr_size = FileRead(Media_data.dec_fp, (unsigned char *)chartmp, AMR_CACHE_SIZE);
		if(cod_wr_size != AMR_CACHE_SIZE){
			amr_end = 1;
mpDebugPrint("amr_end....................");
		}
//		if(cod_wr_size == 0 || cod_wr_size < 0) 
//			return -1;

		cod_rd_ind = 0;
	}

	if( remainder != 0 )
	{
		for( j=0; j < remainder; j++ )
		{
			serial[i + j] = chartmp[cod_rd_ind++];
		}
	}
	if(amr_end)
		mpDebugPrint("%d/%d", cod_rd_ind, cod_wr_size);

	//mpDebugPrint("data size: %d", i + j);
	return i + j;
}

static int preinit(sh_audio_t * sh)
{
	sh->audio_out_minsize = 2 * 4608;
	sh->audio_in_minsize = 4096;
     
	return 1;
}

int MagicPixel_amr_init(int file_size, int *ch,int *srate,int *frame_size, unsigned int *bitrate, int *total_time)
{
	unsigned char header[20];
	unsigned char ft;
	int framenum, res;
	int ret = 0;
	char magic[8];

	//Init input value
	*ch			= 0;
	*srate		= 0;
	*total_time = 0;
	*frame_size = 0;
	*bitrate	= 0;

	old_toc = 0xff;
	cod_rd_ind = 0;

	FileRead(Media_data.dec_fp, magic, 8);
	if(strncmp(magic, AMR_MAGIC_NUMBER, AMR_MAGIC_NUMBER_LENGTH))
	{
		mpDebugPrint("Invalid AMR File header magic number");
		ret = -1;
	}

	if( file_size <= AMR_MAGIC_NUMBER_LENGTH ){
		mpDebugPrint("file size is not large enough");
		ret = -1;
	}

	if(ret < 0)
	{
		Seek(Media_data.dec_fp, SEEK_SET);
		return -1;
	}


	Seek(Media_data.dec_fp, SEEK_SET);
	FileRead(Media_data.dec_fp, header, 20);
	Seek(Media_data.dec_fp, SEEK_SET);

	ft = (header[AMR_MAGIC_NUMBER_LENGTH] >> 3) & 0x0F;
	framenum = (file_size - AMR_MAGIC_NUMBER_LENGTH) / (packed_size[ft] + 1);  // AMR Frame Number
	*ch = 1;
	*srate = 8000;
	*total_time = framenum * 20;
	res = *total_time % 1000;
	*total_time /= 1000;

	if(res > 500)
		*total_time += 1;

	*frame_size = 160;
	*bitrate = (file_size - AMR_MAGIC_NUMBER_LENGTH) / *total_time * 8000;
	chartmp = NULL;

	return ret;
}

static int init(sh_audio_t * sh)
{
	// initial Progress Console
	unsigned int dwFileSize;
	unsigned char header[20];
	unsigned char ft;
	int framenum, res;
	char magic[8];

	old_toc = 0xff;

	mpDebugPrint("amr init...");
	MagicPixel_AMR_SRAM_Init((char *)0xb8000000, 0x6000);
	if(MagicPixel_AMRDEC_Init())
		return -1;

	chartmp = (unsigned char *)mem_malloc(AMR_CACHE_SIZE);
	if(!chartmp)
	{
		mpDebugPrint("amr decoder memory allocation fail");
		return -1;
	}

	dataStreamInit();
	dataread(magic, AMR_MAGIC_NUMBER_LENGTH);

	if (strncmp(magic, AMR_MAGIC_NUMBER, AMR_MAGIC_NUMBER_LENGTH))
	{
		mpDebugPrint("Invalid AMR File header magic number");
		return -1;
	}

	dwFileSize = FileSizeGet(Media_data.dec_fp);
	if ( dwFileSize <= AMR_MAGIC_NUMBER_LENGTH ){
		mpDebugPrint("file size is not large enough");
		return -1;
	}

	Seek(Media_data.dec_fp, SEEK_SET);
	FileRead(Media_data.dec_fp, header, 20);
	Seek(Media_data.dec_fp, SEEK_SET);
	

	ft = (header[AMR_MAGIC_NUMBER_LENGTH] >> 3) & 0x0F;
	framenum = (dwFileSize - AMR_MAGIC_NUMBER_LENGTH) / (packed_size[ft] + 1);  // AMR Frame Number

	Media_data.ch            = 1;
	Media_data.srate         = 8000;
	Media_data.play_time     = 0;
	Media_data.total_time    = framenum * 20;
	res = Media_data.total_time % 1000;
	Media_data.total_time    /= 1000;
	if(res > 500)
	   Media_data.total_time += 1;
	Media_data.frame_size    = 160;
	Media_data.bitrate = (dwFileSize - AMR_MAGIC_NUMBER_LENGTH) * 8000 / Media_data.total_time;

	sh->channels = Media_data.ch;
	sh->samplerate = Media_data.srate;
	sh->i_bps = Media_data.bitrate ;
	sh->samplesize = 2;
	
	cod_wr_size = FileRead(Media_data.dec_fp, (BYTE*)chartmp, AMR_CACHE_SIZE);
	cod_rd_ind = AMR_MAGIC_NUMBER_LENGTH;

	return 0;
}

static int decode_audio(sh_audio_t * sh, unsigned char *buf, int minlen, int maxlen)
{
	int ret, len = 0;
	unsigned char toc, ft;
	unsigned int ERRORHeaderNumber = 0;
	unsigned char packed_bits[MAX_PACKED_SIZE];

	//	mpDebugPrint("AMR decode start");
	while (len < minlen )  // && len + 4608 <= maxlen)
	{
		if(dataread (&toc, 1) == 1)
		{
			if ( old_toc != toc )
			{
				old_toc = toc;
			}

			/* read rest of the frame based on ToC byte */
			//q  = (toc >> 2) & 0x01;
			ft = (toc >> 3) & 0x0F;
			//mpDebugPrint("AMR ft: %2d, toc: %2x", ft, toc);

#ifdef    honda_measure
			if (first != 0)
				mpDebugPrint("AMR MODE= %d",ft);
#endif

			if ( ft > 8 && toc != 0x7c )
			{
				//mpDebugPrint("Undefine AMR ft: %2d, toc: %2x", ft, toc);
				ERRORHeaderNumber++;
				if(ERRORHeaderNumber > 512)
				{
					break;
				}
				continue;
			}

			if ( dataread(packed_bits, packed_size[ft]) <= 0 ){
				ret = -1;
				break;
			}

			//++frame;
			MagicPixel_AMRDEC_decode(toc, packed_bits, (short *)buf);
			Media_data.play_time += 20;
			ret = 0;
		}
		else{
			ret = -1;
			break;
		}
		len += 2 * Media_data.frame_size;
		buf += 2 * Media_data.frame_size;

		TaskYield();
	}

//	mpDebugPrint("AMR decode End : %d", ret);

	if(ret)
		return -1;
	else
		return len ? len : -1;
}

static void uninit(sh_audio_t * sh)
{
	mem_free((void*)chartmp);
	amr_end = 0;
	mpDebugPrint("soft_amr end");
}

static int control(sh_audio_t * sh, int cmd, void *arg, ...)
{
}

static int resync(unsigned int second)
{
	return MagicPixel_AMR_resync(second);
}


// If sucess return zero,
// Else error code is returned.
int MagicPixel_AMR_resync(DWORD msec)
{
	int ret = 0;
	int frameCnt = 0, i;
	unsigned char packed_bits[MAX_PACKED_SIZE];
	unsigned char toc, ft;

	frameCnt = msec / 20;

	// Method 1 (performance is poor)
	dataStreamInit();

	for(i = 0; i < frameCnt; i++){
		if(dataread (&toc, 1) == 1)
		{
			ft = (toc >> 3) & 0x0F;

			if ( ft > 8 && toc != 0x7c )
			{
				continue;
			}
			//mpDebugPrint("	%d", packed_size[ft]);
			if (dataread(packed_bits, packed_size[ft]) <= 0){
				ret = -1;
				break;
			}
			ret = 0;
		}
		else{
			ret = -1;
			break;
		}

		TaskYield();
	}

	Media_data.play_time = msec;

	return ret;
}

ad_functions_t software_amr = {
	&info,
	preinit,
	init,
	uninit,
	control,
	decode_audio,
	resync
};

