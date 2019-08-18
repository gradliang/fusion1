int AV_WMA_bitstream_callback(unsigned char *buf, int len);
void AV_WMA_newblock_callback();
int MagicPixel_WMA_Init(int FormatTag,int SamplingRate,int nChannel,int BytePerSec,int BlockSize
                              ,int EncodeOpt, void (*fn_bitstream_callback)(unsigned char*, int)
                              ,void (*fn_newblock_callback)());
int MagicPixel_WMA_Decode(int *cSamples, int eq_enable, int *pcm_buf[2][2]);
void MagicPixel_WMA_Post_Process(int mono_stereo,int stereo_reverse,int samples,int ch,int *pcm_buf[2][2],short *out_buf);
void MagicPixel_WMA_Post_Decode(int cSamples);
void MagicPixel_WMA_Resync();
int MagicPixel_WMA_Get_NewBlock(); 
