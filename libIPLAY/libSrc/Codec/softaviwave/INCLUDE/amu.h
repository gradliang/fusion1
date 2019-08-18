
typedef struct {
   unsigned short tag; 
   unsigned short ch;
}MPXWave; 


//return value: the size of output "short" data  
//if it is -1, not support format
int MPXWaveDecoder(MPXWave *mpxw, unsigned char *audibuf, short *pcmbuf, unsigned long ilen);

void MagicPixel_WAV_fileseek_callback(int file_ptr);
int MagicPixel_WAV_bitstream_callback(unsigned char *buf, int len,int *fend_flag);
unsigned char *MagicPixel_WAV_malloc(unsigned int size);
void MagicPixel_WAV_free(unsigned char *buf);
int MagicPixel_wav_init(int file_size,int *ch,int *srate,int *bitrate,int *total_time,int *sample_size, int *format,int *blockalign,unsigned int *waveoffset);

