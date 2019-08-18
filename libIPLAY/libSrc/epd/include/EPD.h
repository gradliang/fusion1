#ifndef _EPD_H
#define _EDP_H
#ifdef Support_EPD
void IDU_EPD_send_data(BYTE data);
void IDU_EPD_DMA_buff_SET(ST_IMGWIN *win);
void IDU_EPD_send_buff_start();
void IDU_EPD_Screen_Clear();
void IDU_EPD_2GateBlock_updata(ST_IMGWIN *win,BYTE *Y_data,BYTE *Y_updata,int x1,int y1,int x2,int y2, int nFrameCount);
void IDU_EPD_8GateBlock_updata(ST_IMGWIN *win,BYTE *Y_data,BYTE *Y_updata,int x1,int y1,int x2,int y2);
void IDU_EPD_16GateBlock_updata(ST_IMGWIN *win,BYTE *Y_data,BYTE *Y_updata,int x1,int y1,int x2,int y2);
void IDU_EPD_Block_Clear(ST_IMGWIN *win,BYTE *Y_data,int x1,int y1,int x2,int y2);
void Idu_EPDPrintString(ST_IMGWIN * trgWin, BYTE * string, WORD startX, WORD startY);
void EPDmpPrintMessage( ST_IMGWIN *pWin,const char *cMsg,WORD x,WORD y);
void IDU_EPD_Screen2Gate_ALL(ST_IMGWIN *win,BYTE *Y_data, int nFrameCount);
void IDU_EPD_Screen2Gate_Draw(ST_IMGWIN *win, int nFrameCount);
void IDU_EPD_Screen2Gate_Reflash(ST_IMGWIN *win,BYTE *Y_data, int nFrameCount);
void IDU_EPD_Screen8Gate_ALL(ST_IMGWIN *win,BYTE *Y_data);
void IDU_EPD_Screen8Gate_Draw(ST_IMGWIN *win);
void IDU_EPD_Screen8Gate_Reflash(ST_IMGWIN *win,BYTE *Y_data);
void IDU_EPD_Screen16Gate_ALL(ST_IMGWIN *win,BYTE *Y_data);
void IDU_EPD_Screen16Gate_Draw(ST_IMGWIN *win);
void IDU_EPD_Screen16Gate_Reflash(ST_IMGWIN *win,BYTE *Y_data);
void prepare_data2(ST_IMGWIN * win, BYTE * Y_data,int nType);
void prepare_data8(ST_IMGWIN * win, BYTE * Y_data,int nType);
void prepare_data16(ST_IMGWIN * win, BYTE * Y_data,int nType);
void IDU_EPD_Free_Output_Buf();
WORD* IDU_EPD_Get_Output_Buf();
void IDU_EPD_Set_Output_Buf(WORD* pBuf);
void IDU_EPD_Init();

#endif
#endif

