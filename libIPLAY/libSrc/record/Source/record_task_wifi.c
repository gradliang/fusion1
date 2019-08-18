#define LOCAL_DEBUG_ENABLE  0
#include "global612.h"
#include "mpTrace.h"
#include "os.h"
#include "taskid.h"
#include "devio.h"
#include "sensor.h"
#include "record.h"


//#include "camcorder_func.h"
#include "ui.h"



/*---state---*/

char *S_Preview_state       	        ="Preview_state        ";
char *S_Recording_state     	        ="Recording_state      ";
char *S_Recording_stop_state            ="Recording_stop_state ";
char *S_Recording_pause_state           ="Recording_pause_state";
char *S_Preview_stop_state              ="Preview_stop_state   ";
char *S_Rec_StandBy_state               ="StandBy_state        ";
char *S_ExceptionCheck_state            ="ExceptionCheck_state ";
char *S_Unknown_state                   ="Unknown_state        ";

/*---op_code---*/

char *S_VIDEO_REC_OP_PREVIEW_START      ="Preview_Start        ";
char *S_VIDEO_REC_OP_RECORDING_START    ="Recording_Sstart     ";
char *S_VIDEO_REC_OP_RECORDING_STOP     ="Recording_Stop       ";
char *S_VIDEO_REC_OP_PREVIEW_STOP       ="Preview_Stop         ";
char *S_VIDEO_REC_OP_RECORDING_PAUSE    ="Recording_Pause      ";
char *S_VIDEO_REC_OP_RECORDING_RESUME   ="Recording_Resume     ";
char *S_VIDEO_REC_OP_RECORDING_EXCEPTION="Recording_Exception  ";
char *S_VIDEO_REC_OP_Unknown            ="Unknown              ";

extern int Need_changWin;


#if (RECORD_ENABLE)
void RecordTask_State(BYTE apievent);
SWORD SetResolution();
#define REC_IMGSIZE  (1280 * 720 * 2 + 4096)
//#define DoubleBuffer_open
#define MOTION_DETECTION 0
#define EN_JPGSIZE     (1024 * 90 * 6)
#define AUDIOSAMPLELEN 1024
DWORD fps_record=6;
//static 
	record_argument rec_argument;
DWORD    VIDEO_WIDTH =   1280;
DWORD    VIDEO_HEIGHT =  720;
 BYTE *tBuf;
//=====================
STREAM *shandle_jpg=NULL;
STREAM *shandle_jpg1=NULL;



//=====================

//DWORD rec_fileopen=0,FileOpenFinsh=0;;
//===overlay============
#if 0
ST_SYSTEM_TIME * curr_time1;
ST_IMGWIN  srcMainWin;
ST_IMGWIN  srcSubWin;
ST_IMGWIN  trgWin;
STREAM *shandle1;
STREAM *shandle2;
STREAM *shandle3;
BYTE *pBuffer1,*pBuffer2,*fileBuffer;
BYTE *pBuffe_[15];
BYTE *timestamp;

DWORD iw=0;
DWORD iw1=0;
DWORD rtc=0;
DWORD rtc_counter=0;


char *outName_Main = "yellow.yuv";
char outName_Src[]= "0_YUV422.yuv";
char outName_Trg[]= "0.bin";
#endif
//===overlay============

#if (MOTION_DETECTION == 0)
//STREAM *shandle1;
//int aud_1=0;
//DRIVE *sDrv1;
 volatile int recordTaskState = Rec_StandBy_state;

void RecordVideoTask();
void AUDIO_TASK();
void record_op_control();
void record_panel_task();
static BYTE *audio_buf;
//static DWORD Audio_len;

extern ST_IMGWIN SensorInWin[3];
extern int DoubleBuffer;
DWORD file_size_avi =0;
DWORD cache_size_avi=0;
int ExceptionCheck=1;
int no_movie=1;
int count_num=0;
	

DWORD writepcm(BYTE *buf, DWORD len, BYTE hciSize)
{
    DWORD audioStreamInfo[2];
	audio_buf=buf;
//	Audio_len=len;
    audioStreamInfo[0] = (DWORD) buf;
    audioStreamInfo[1] = len;
	//mpDebugPrint("---buf=--%0x%0x%0x---",buf[0],buf[1],buf[2]);
   // mpDebugPrint("len=%d",len);
    //EventSet(AUDIO_ID, BIT0);
    if (MessageDrop(AUDIO_STREAM_MSG_ID, (BYTE *) &audioStreamInfo, 8) != OS_STATUS_OK){
        MP_ALERT("--E-- AUDIO_STREAM_MSG_ID msg drop !!!");
    }
}


SWORD RecordArgument(record_argument *p)
{
    SWORD ret=0;
    //=====================
    rec_argument.resolution=p->resolution     ;//  =r_720p;
    rec_argument.fps=p->fps            ;// =r_fps_24;
    rec_argument.handle=p->handle         ;// =handle;
    rec_argument.movie_length=p->movie_length   ;// =10;
    rec_argument.quantification=p->quantification ;// =r_midlle;
    rec_argument.RecordingAudio=p->RecordingAudio ;// =r_open;
	rec_argument.image_size=p->image_size;
	rec_argument.driveIndex=p->driveIndex;

	return ret;
}
record_argument RecordArgumentGet(void)
{
    return rec_argument;
}
SWORD recordTaskInit()
{
	
    SWORD ret;
    SWORD ret1;
    EventCreate(REC_SENDMSG_EVENT,(OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);
    EventCreate(AUDIO_ID,(OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);
    SemaphoreCreate(RECORD_STREAM_CACHE_SEMA_ID, OS_ATTR_PRIORITY, 1);
    SemaphoreCreate(RECORD_opcode_SEMA_ID, OS_ATTR_PRIORITY, 1);
    MessageCreate(AUDIO_STREAM_MSG_ID, OS_ATTR_FIFO, 640);
    EventCreate(SENSOR_IPW_FRAME_END_EVENT_ID, (OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);
    ret = TaskCreate (RECORD_CNT_TASK, RecordVideoTask, ISR_PRIORITY + 1, 0x4000);
    if(ret != OS_STATUS_OK){
        MP_ALERT("video task crate failed..");
        __asm("break 100");
    }
    ret1= TaskCreate (AVI_AUDIO_TASK, AUDIO_TASK, ISR_PRIORITY, 0x4000);
    if(ret1 != OS_STATUS_OK){
        MP_ALERT("Audio task crate failed..");
        __asm("break 100");
    }

    if(!aviStreamStartup()){
        mpDebugPrint("Avi stream startup fail...");
        while(1);
    }
    TaskSleep(10);
    ret1 = TaskStartup(AVI_AUDIO_TASK);
    if (ret1 != OS_STATUS_OK){
        mpDebugPrint("avi stream task initiates fail....");
        return -1;
    }
    ret1 = TaskStartup(RECORD_CNT_TASK);
    if (ret1 != OS_STATUS_OK){
        mpDebugPrint("avi_audio stream task initiates fail....");
        return -1;
    }
    return ret;
}



SWORD recopcontrolInit()
{
    EventCreate(REC_ERR_EVENT,(OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);
    MessageCreate(REC_OP_MSG_ID, OS_ATTR_FIFO, 640);

    //EventCreate(REC_OP_EVENT_ID,(OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE), 0);
    TaskCreate (REC_OP_CONTROL_TASK, record_op_control,CONTROL_PRIORITY, 0x4000);

#if ((TCON_ID != TCON_NONE) && (SENSOR_WITH_DISPLAY == ENABLE))	
	TaskCreate (RECORD_PANEL_TASK, record_panel_task,CONTROL_PRIORITY, 0x4000);
#endif
	//==========
    //==========
    recordTaskInit();
    TaskStartup(REC_OP_CONTROL_TASK);

#if ((TCON_ID != TCON_NONE) && (SENSOR_WITH_DISPLAY == ENABLE))	
	TaskStartup(RECORD_PANEL_TASK);
#endif
}
///////////////////////////////////////////////////////////
//                                    AUDIO              //
//                                                       //
///////////////////////////////////////////////////////////
void AUDIO_TASK()
{
    DWORD audioStreamInfo[2];
    //Audio_OpenInputDevice(8000, 1, 1, writepcm, 1024);
    while(1)
    {
        MessageReceive(AUDIO_STREAM_MSG_ID, (BYTE *) &audioStreamInfo);
        //PutUartCharDrop('A');
        //audio_buf = (BYTE *) audioStreamInfo[0];
        //Audio_len = audioStreamInfo[1];

        //if(audio_buf[0]==0 &&audio_buf[5]==0 &&audio_buf[11]==0 &&audio_buf[20]==0 &&audio_buf[21]==0 )
        //mpDebugPrint("====Write back:0===");
        //UartOutText(" a ");
        AVIEN_fillPcm((BYTE *) audioStreamInfo[0]);
       // EventSet(AUDIO_ID, BIT0);
    }
}
//===============================
#define MAX_FILE_SIZE               3900000     // KB

void TimeEndREC()
{
    static DWORD preRecordSize = 0;
    DWORD currRecordSize;
    DWORD diskSize;
    BYTE a = VIDEO_REC_OP_RECORDING_EXCEPTION;
	
	if (1== SystemCardPresentCheck(rec_argument.driveIndex))
		{

    currRecordSize = file_size_avi*cache_size_avi;
    if (currRecordSize > MAX_FILE_SIZE)
    {
        file_size_avi = 0;
        mpDebugPrint("-E- file size over");

        EventSet(UI_EVENT, EVENT_DISK_FULL);
            SysTimerProcRemove(TimeEndREC);
    }

    diskSize = DriveFreeSizeGet(DriveGet(DriveCurIdGet())) / 1024;  // Sector
    diskSize = diskSize * DriveSetcorSizeGet(DriveGet(DriveCurIdGet())) / 1024;

    if (diskSize < 10)  // unit is MB
    {
        mpDebugPrint("-E- disk full in Recording");
 
		EventSet(UI_EVENT, EVENT_DISK_FULL);
		SysTimerProcRemove(TimeEndREC);
    }

    if (((currRecordSize >> 10) / 100) != preRecordSize)
    {
        preRecordSize = (currRecordSize >> 10) / 100;

        mpDebugPrintN("%dMB ", currRecordSize >> 10);
    }

	}

}


void record_op_control()
{
    BYTE api_event = 7;

    while (1)
    {
  
        MessageReceive(REC_OP_MSG_ID,(BYTE *) &api_event);
        RecordTask_State(api_event);
    }
}

//static BYTE rec_opcode=0;
//int REC_err=0;
//int firstone=0;

///////////////////////////////////////////////
//#define __DEBUG_RECORD_FRAME_RATE

#ifdef __DEBUG_RECORD_FRAME_RATE
// For debug only
#define HISTORY_OF_JPEG_POWER       5
#define HISTORY_OF_JPEG_FARME       (1 << HISTORY_OF_JPEG_POWER)
static DWORD startTime = 0, elapsedTime;
//ST_SYSTEM_TIME * curr_time1;
static DWORD frameNumber = 0;
static DWORD encodeFaileTimes = 0;
static DWORD encodeDropTimes = 0;
static DWORD jpegSize[HISTORY_OF_JPEG_FARME];
#endif

///////////////////////////////////////////////
//============
unsigned int picDecodeNumber1 = 0;
char fname[18];   
char extname[]="jpg";
int sensor_start_o=0;
int record_time=0;
void RecordTask_State(BYTE apievent)
{
    int opcode=99;
    int ret=0;
    DWORD REC_SENDMSG;
	mpDebugPrint("///////////////////////////////////////");
	mpDebugPrint("/ 							       ");
	ShowState(recordTaskState);
	Showinstruction(apievent);
	mpDebugPrint("/ 								   ");
	mpDebugPrint("///////////////////////////////////////");
    switch (recordTaskState)
    { 
    	case Rec_StandBy_state:
        	switch (apievent)
        	{
				
        		case VIDEO_REC_OP_PREVIEW_START:
            		tBuf = (BYTE *)ext_mem_malloc(EN_JPGSIZE);
            		if (!tBuf)__asm("break 100");
					tBuf = (BYTE *)((DWORD)tBuf | 0xa0000000);
            		recordTaskState = Preview_state;
					TestFunction();
					mpDebugPrint("///////////////////////////////////////");
			        mpDebugPrint("/  sensor_fps=%2d                     /",rec_argument.fps);
					SetResolution();
					mpDebugPrint("///////////////////////////////////////");
					SetSensorInterfaceMode(MODE_RECORD);
					SetSensorFrameRate(rec_argument.fps);
					SetImageSize(rec_argument.image_size);
					MP_DEBUG("##debug## API_Sensor_Initial() start");	
                    SetMultiBufFlag(rec_argument.sensor_buffer_num);
					API_Sensor_Initial();
					MP_DEBUG("##debug## API_Sensor_Initial() finish");	
            		EventSet(REC_ERR_EVENT,BIT0);
            		break;
				default:
					mpDebugPrint("-E- (ERROR)No api function in standBy state");
            		EventSet(REC_ERR_EVENT,BIT1);
            		break;
        	}
			break;
//=====Preview=======================================
		case Preview_state:
        	switch (apievent)
        	{
        		case VIDEO_REC_OP_RECORDING_START:
					record_time=0;
				record_time=GetSysTime();
				
					mpDebugPrint("No:%d/////////////////////////////////////",no_movie);
					sensor_start_o=1;
					MP_DEBUG("##debug## create file start");
            		AVIEN_createfile(rec_argument.handle,EN_JPGSIZE, AUDIOSAMPLELEN, 8000, 8);
					MP_DEBUG("##debug## create file finish");
#ifdef __DEBUG_RECORD_FRAME_RATE
            		frameNumber = 0;
            		encodeFaileTimes = 0;
            		encodeDropTimes = 0;
            		memset((BYTE *) &jpegSize, 0, sizeof(DWORD) * HISTORY_OF_JPEG_FARME);
#endif
					

					if(rec_argument.RecordingAudio==0)//r_open=0 r_close=1
					{
					mpDebugPrint("##audio open recording");
					MP_DEBUG("##debug## Audio_OpenInputDevice start");
                    IntDisable();
					Audio_OpenInputDevice(8000, 1, 1, writepcm, 1024, 0);
					IntEnable();
					MP_DEBUG("##debug## Audio_OpenInputDevice finish");
					MP_DEBUG("##debug## WaveInputStart() start");
					WaveInputStart();
					MP_DEBUG("##debug## WaveInputStart() finish");
        			}
					else
					{
					mpDebugPrint("##audio close recording");	
					}
#ifdef __DEBUG_RECORD_FRAME_RATE
            		startTime = GetSysTime();
#endif
            		recordTaskState = Recording_state;
            		EventSet(REC_ERR_EVENT,BIT0);
					MP_DEBUG("##debug## SysTimerProcAdd() start");
            		SysTimerProcAdd(1000,TimeEndREC,0);
					MP_DEBUG("##debug## SysTimerProcAdd() finish");
#if (MOTION_DETECT == ENABLE)
				  	/*Motion detection need to reset after video recoding.*/
		 			MotionDetectionReset();
#endif
            		break;
        		case VIDEO_REC_OP_PREVIEW_STOP:
            		recordTaskState=Rec_StandBy_state;
            		ext_mem_free(tBuf);
					MP_DEBUG("##debug## API_Sensor_Stop() start");
            		API_Sensor_Stop();
					MP_DEBUG("##debug## API_Sensor_Stop() finish");
            		EventSet(REC_ERR_EVENT, BIT0);
            		break;

        		default:
					mpDebugPrint("-E-(ERROR)No api function in Preview_state");
            		EventSet(REC_ERR_EVENT,BIT1);
            		break;
        	}
        	break;
//=====Recording============================================
    	case Recording_state:
        	switch (apievent)
        	{
				case VIDEO_REC_OP_RECORDING_EXCEPTION:
					MP_DEBUG("##debug## SysTimerProcRemove(TimeEndREC) start");
					SysTimerProcRemove(TimeEndREC);
					MP_DEBUG("##debug## SysTimerProcRemove(TimeEndREC) finish");
					MP_DEBUG("##debug## WaveInputEnd() start");
					WaveInputEnd();
					MP_DEBUG("##debug## WaveInputEnd() finish");
					MP_DEBUG("##debug## Audio_CloseInputDevice() start");
        			Audio_CloseInputDevice();
					MP_DEBUG("##debug## Audio_CloseInputDevice() finish");
					MP_DEBUG("##debug## ExceptionHandle() start");
					ExceptionHandle();
					MP_DEBUG("##debug## ExceptionHandle() finish");
					if(1== SystemCardPresentCheck(rec_argument.driveIndex))
					{
						MP_DEBUG("##debug## send EVENT_CARD_FATAL_ERROR");
        				EventSet(UI_EVENT, EVENT_CARD_FATAL_ERROR);
					}
					recordTaskState = ExceptionCheck_state;
					break;	
        		case VIDEO_REC_OP_RECORDING_STOP:
					sensor_start_o=0;
            		recordTaskState=Recording_stop_state;
					mpDebugPrint("No:%d/////RECORDING_STOP:///time=%d//////////",no_movie++,SystemGetElapsedTime(record_time)/1000);
					MP_DEBUG("##debug## WaveInputEnd() start");
					if(rec_argument.RecordingAudio==0)//r_open=0 r_close=1
					{
					WaveInputEnd();
					}
					MP_DEBUG("##debug## WaveInputEnd() finish");
#ifdef __DEBUG_RECORD_FRAME_RATE
            		elapsedTime = SystemGetElapsedTime(startTime);
#endif
					MP_DEBUG("##debug## SysTimerProcRemove(TimeEndREC) start");
            		SysTimerProcRemove(TimeEndREC);
					MP_DEBUG("##debug## SysTimerProcRemove(TimeEndREC) finish");		
            		mpDebugPrint("##debug## AVIEN_closefile() start");
            		AVIEN_closefile();
					mpDebugPrint("##debug## AVIEN_closefile() finish");
					mpDebugPrint("##debug## Audio_CloseInputDevice() start");
                    if(rec_argument.RecordingAudio==0)//r_open=0 r_close=1
					{
					Audio_CloseInputDevice();
                    }
					mpDebugPrint("##debug## Audio_CloseInputDevice() finish");
#ifdef __DEBUG_RECORD_FRAME_RATE
					//mpDebugPrint("Record Frame Rate = %dfs, %u/%u", frameNumber/ (elapsedTime / 1000), frameNumber, elapsedTime);
            		mpDebugPrint("Fail Record Frame %u", encodeFaileTimes);
            		mpDebugPrint("Drop Record Frame %u", encodeDropTimes);
#endif
					recordTaskState = Preview_state;
					EventSet(REC_ERR_EVENT,BIT0);   
            		break;
        		case VIDEO_REC_OP_RECORDING_PAUSE:
					MP_DEBUG("##debug## WaveInputEnd() start");
					WaveInputEnd();
					MP_DEBUG("##debug## WaveInputEnd() finish");
            		recordTaskState = Recording_pause_state;
					EventSet(REC_ERR_EVENT, BIT0);
					break;
        		default:
				mpDebugPrint("-E- (ERROR)No api function in Recording state");
            	EventSet(REC_ERR_EVENT,BIT1);
            	break;
        }
        break;
//=====Recording_pause============================================
    	case Recording_pause_state:
        	switch (apievent)
        	{
        		case VIDEO_REC_OP_RECORDING_RESUME:
            		recordTaskState=Recording_state;
					MP_DEBUG("##debug## WaveInputStart() start");
            		WaveInputStart();
					MP_DEBUG("##debug## WaveInputStart() finish");
					EventSet(REC_ERR_EVENT, BIT0);
					break;

        		default:
            		EventSet(REC_ERR_EVENT,BIT1);
					mpDebugPrint("-E- (ERROR)No api function in Recording_pause_state");
            		break;
        	}
        	break;
//=====ExceptionCheck_state============================================
    	case ExceptionCheck_state:
			mpDebugPrint("##debug## ExceptionCheck_state");
        	switch (apievent)
        	{
				Showinstruction(apievent);
        		case VIDEO_REC_OP_RECORDING_STOP:
					ExceptionCheck = 1;
					recordTaskState = Preview_state;
					EventSet(REC_ERR_EVENT,BIT0);
            		break;
        		default:
					mpDebugPrint("-E- (ERROR)No api function in ExceptionCheck_state");
            		EventSet(REC_ERR_EVENT,BIT1);
            		break;
        	}
        	break;
    }
}

//===============================
int ce=0,sce1=0;
extern int sce;
extern BYTE MultiBufFlag;
int now=0,ok=0;

#if SHOW_FRAME_TEST
extern int connect_socket;
#endif

void RecordVideoTask()
{int temp=0;
    int retVal;
    int CurrImgBufEncoderIndex=0;

    mpDebugPrint("RecordVideoTask startup!");
    #define AUDIOSAMPLELEN 1024
    DWORD SensorEvent;
    DWORD iSize = 0;

#if SHOW_FRAME_TEST
	//VGA temp photo buffer use for WiFi send Queue
	BYTE *photoBuf = (BYTE *)ext_mem_malloc(EN_JPGSIZE);
	if(photoBuf == NULL)
	{
		mpDebugPrint("[%s] alloc ext memory failed!", __func__);
		BREAK_POINT();
	}
	else	//alloc success, set address of photoBuf to non-cache address
		photoBuf = (BYTE *)((DWORD)photoBuf | 0xa0000000);
#endif	

    EventClear(SENSOR_IPW_FRAME_END_EVENT_ID, ~BIT0);

    while(1)
    {
		//EventClear(SENSOR_IPW_FRAME_END_EVENT_ID, ~BIT0);

		EventWait(SENSOR_IPW_FRAME_END_EVENT_ID, BIT0, OS_EVENT_OR, &SensorEvent);
#if SHOW_FRAME_TEST
		iSize = ImageFile_Encode_Img2Jpeg(tBuf, &SensorInWin[temp]);
	    	iSize = (iSize + 1) & 0xFFFFFFFE;
		//mpDebugPrint("b imgSize = %u\n", iSize);
		if(Conn(connect_socket) && (connect_socket > 0))
		{
			MP_DEBUG("imgSize = %u\n", iSize);
			mmcp_memcpy(photoBuf, tBuf, iSize);
			miniDV_Send_Queue(photoBuf, iSize);
			TaskYield();
		}
#else
     SensorApplication();
#endif
    }
}

#if ((TCON_ID != TCON_NONE) && (SENSOR_WITH_DISPLAY == ENABLE))	
void record_panel_task()
{
 DWORD SensorEvent;
 IPU *ipu = (IPU *) IPU_BASE;
 ST_IMGWIN *win1;
 int waitflg;
 IDU *idu = (IDU *)IDU_BASE; 
 DWORD DisplayWinStartAddr;
 WORD PanelW,PanelH;
 extern WORD SensorWindow_PosX;
 extern WORD SensorWindow_PosY;
 extern BYTE SensorWindow_setFlag;
 
 win1 = Idu_GetCurrWin();
 PanelW = win1->wWidth;
 PanelH = win1->wHeight;
 
 while(1)
 { 
  	EventWait(SENSOR_IPW_FRAME_END_EVENT_ID, BIT1, OS_EVENT_OR, &SensorEvent);
   
#if ((IDU_CHANGEWIN_MODE == 1)) //&& ((SENSOR_TO_FB_RGB888_TYPE != ENABLE)) )
     // 0: Polling, 1: SW toggle 
	/*for small panel*/
	waitflg = 1; 
	while(waitflg)  
	{
  		if((idu->RST_Ctrl_Bypass & 0x18000000)!=0x10000000)
  		{
   			waitflg = 0;
  		}
	}
#endif

	if(Get_Display_flag() == ENABLE)
	{
		MP_DEBUG("change win");
  		win1 = Idu_GetNextWin();
       
        Idu_ChgWin(win1);

	    #if (TCON_ID == TCON_I80Mode_128x128)
		   //mpDebugPrint("ILI9163");
		    ILI9163_WRITE_FRAME();
		    //TimerDelay(5000);
        #endif

/*
	     #if (TCON_ID == TCON_I80Mode_128x128)
		    //mpDebugPrint("ILI9163");
		    ILI9163_WRITE_FRAME();
         #else
		    //mpDebugPrint("Others");
			Idu_ChgWin(win1);
         #endif
*/
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE)
		ipu->Ipu_reg_F0 &= ~BIT6;/*Open IPW1 write path*/
#else
		ipu->Ipu_reg_F0 &= ~BIT7;/*Open IPW2 write path*/
#endif
	}
	else
	{ 
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE)
		ipu->Ipu_reg_F0 |= BIT6;/*Close IPW1 write path*/
#else
		ipu->Ipu_reg_F0 |= BIT7;/*Close IPW2 write path*/
#endif
	}
  
	Need_changWin = 0;
	
	
 }
}
#endif

SWORD SetResolution()
{
	
	switch (rec_argument.resolution)
    {
    case RESOLUTION_QCIF_60:
		count_num=7;
		fps_record=60;
		VIDEO_WIDTH=176;
  		VIDEO_HEIGHT=144;
		mpDebugPrint("/ AVI_Resolution =QCIF                /");
		mpDebugPrint("/    AVI_fps     =%2d                 /",fps_record);
		mpDebugPrint("/ VIDEO_WIDTH    =%3d         		/",VIDEO_WIDTH);
		mpDebugPrint("/ VIDEO_HEIGHT   =%3d         		/",VIDEO_HEIGHT);
		break;
    case RESOLUTION_QVGA_60:
		count_num=6;
		fps_record=60;
		VIDEO_WIDTH=320;
  		VIDEO_HEIGHT=240;
		mpDebugPrint("/ AVI_Resolution=QVGA                 /");
		mpDebugPrint("/ AVI_fps=%2d                         /",fps_record);
		mpDebugPrint("/ VIDEO_WIDTH=%3d 		            /",VIDEO_WIDTH);
		mpDebugPrint("/ VIDEO_HEIGHT=%3d 		            /",VIDEO_HEIGHT);
		break;
    case RESOLUTION_CIF_60:
		count_num=5;
		fps_record=60;
		VIDEO_WIDTH=352;
  		VIDEO_HEIGHT=288;
		mpDebugPrint("/ AVI_Resolution=QCIF           /");
		mpDebugPrint("/ AVI_fps=%2d           /",fps_record);
		mpDebugPrint("/ VIDEO_WIDTH=%3d 		  /",VIDEO_WIDTH);
		mpDebugPrint("/ VIDEO_HEIGHT=%3d 		  /",VIDEO_HEIGHT);
		break;
	case RESOLUTION_CIF_30:
		count_num=5;
		fps_record=30;
		VIDEO_WIDTH=352;
  		VIDEO_HEIGHT=288;
		mpDebugPrint("/ AVI_Resolution=CIF           /");
		mpDebugPrint("/ AVI_fps=%2d           /",fps_record);
		mpDebugPrint("/ VIDEO_WIDTH=%3d 		  /",VIDEO_WIDTH);
		mpDebugPrint("/ VIDEO_HEIGHT=%3d 		  /",VIDEO_HEIGHT);
		break;
    case RESOLUTION_VGA_60:
		count_num=4;
		fps_record=60;
		VIDEO_WIDTH=640;
  		VIDEO_HEIGHT=480;
		mpDebugPrint("/ AVI_Resolution =VGA");
		mpDebugPrint("/ AVI_fps        =%2d",fps_record);
		mpDebugPrint("/ VIDEO_WIDTH    =%3d",VIDEO_WIDTH);
		mpDebugPrint("/ VIDEO_HEIGHT   =%3d",VIDEO_HEIGHT);
		break;
    case RESOLUTION_VGA_30:
		count_num=3;
		fps_record=30;
		VIDEO_WIDTH=640;
  		VIDEO_HEIGHT=480;
		mpDebugPrint("/ AVI_Resolution =VGA");
		mpDebugPrint("/   AVI_fps      =%2d",fps_record);
		mpDebugPrint("/  VIDEO_WIDTH   =%3d",VIDEO_WIDTH);
		mpDebugPrint("/  VIDEO_HEIGHT  =%3d",VIDEO_HEIGHT);
		break;
	case RESOLUTION_VGA_16:
		count_num=3;
		fps_record=16;
		VIDEO_WIDTH=640;
  		VIDEO_HEIGHT=480;
		mpDebugPrint("/ AVI_Resolution =VGA");
		mpDebugPrint("/   AVI_fps      =%2d",fps_record);
		mpDebugPrint("/  VIDEO_WIDTH   =%3d",VIDEO_WIDTH);
		mpDebugPrint("/  VIDEO_HEIGHT  =%3d",VIDEO_HEIGHT);
		break;
    case RESOLUTION_D1_60:
		count_num=3;
		fps_record=60;
		VIDEO_WIDTH=720;
  		VIDEO_HEIGHT=480;
		mpDebugPrint("/ AVI_Resolution = D1");
		mpDebugPrint("/   AVI_fps      =%2d",fps_record);
		mpDebugPrint("/ VIDEO_WIDTH    =%3d",VIDEO_WIDTH);
		mpDebugPrint("/ VIDEO_HEIGHT   =%3d",VIDEO_HEIGHT);
		break;
    case RESOLUTION_D1_50:
		count_num=3;
		fps_record=50;
		VIDEO_WIDTH=720;
  		VIDEO_HEIGHT=480;
		mpDebugPrint("/ AVI_Resolution = D1");
		mpDebugPrint("/   AVI_fps      =%2d",fps_record);
		mpDebugPrint("/ VIDEO_WIDTH    =%3d",VIDEO_WIDTH);
		mpDebugPrint("/ VIDEO_HEIGHT   =%3d",VIDEO_HEIGHT);
		break;
	case RESOLUTION_480P_30:
		count_num=3;
		fps_record=30;
		VIDEO_WIDTH=720;
  		VIDEO_HEIGHT=480;
		mpDebugPrint("/ AVI_Resolution =480P");
		mpDebugPrint("/   AVI_fps      =%2d",fps_record);
		mpDebugPrint("/ VIDEO_WIDTH    =%3d",VIDEO_WIDTH);
		mpDebugPrint("/ VIDEO_HEIGHT   =%3d",VIDEO_HEIGHT);
		break;
	case RESOLUTION_XGA_30:
		count_num=3;
		fps_record=30;
		VIDEO_WIDTH=1024;
  		VIDEO_HEIGHT=768;
		mpDebugPrint("/ AVI_Resolution =XGA");
		mpDebugPrint("/   AVI_fps      =%2d",fps_record);
		mpDebugPrint("/ VIDEO_WIDTH    =%3d",VIDEO_WIDTH);
		mpDebugPrint("/ VIDEO_HEIGHT   =%3d",VIDEO_HEIGHT);
		break;
    case RESOLUTION_720P_24:
		count_num=2;
		fps_record=24;
		VIDEO_WIDTH=1280;
  		VIDEO_HEIGHT=720;
		mpDebugPrint("/ AVI_Resolution=720P");
		mpDebugPrint("/   AVI_fps     =%2d",fps_record);
		mpDebugPrint("/ VIDEO_WIDTH   =%3d",VIDEO_WIDTH);
		mpDebugPrint("/ VIDEO_HEIGHT  =%3d",VIDEO_HEIGHT);
		break;
    case RESOLUTION_720P_20:
		count_num=2;
		fps_record=20;
		VIDEO_WIDTH=1280;
  		VIDEO_HEIGHT=720;
		mpDebugPrint("/ AVI_Resolution=720P");
		mpDebugPrint("/   AVI_fps     =%2d",fps_record);
		mpDebugPrint("/ VIDEO_WIDTH   =%3d",VIDEO_WIDTH);
		mpDebugPrint("/ VIDEO_HEIGHT  =%3d",VIDEO_HEIGHT);
		break;
    case RESOLUTION_720P_16:
		count_num=2;
		fps_record=16;
		VIDEO_WIDTH=1280;
  		VIDEO_HEIGHT=720;
		mpDebugPrint("/ AVI_Resolution=720P");
		mpDebugPrint("/   AVI_fps     =%2d",fps_record);
		mpDebugPrint("/ VIDEO_WIDTH   =%3d",VIDEO_WIDTH);
		mpDebugPrint("/ VIDEO_HEIGHT  =%3d",VIDEO_HEIGHT);
		break;
    case RESOLUTION_720P_6:
		count_num=2;
		fps_record=6;
		VIDEO_WIDTH=1280;
  		VIDEO_HEIGHT=720;
		mpDebugPrint("/ AVI_Resolution=720P           /");
		mpDebugPrint("/ AVI_fps=%2d           /",fps_record);
		mpDebugPrint("/ VIDEO_WIDTH=%3d 		  /",VIDEO_WIDTH);
		mpDebugPrint("/ VIDEO_HEIGHT=%3d 		  /",VIDEO_HEIGHT);
		break;
		}
	return 0;
}

#else

#endif
void ShowState(int state)
{   	
	switch (state)
    {
	case 0:
	mpDebugPrint("/       state= Preview_state        /");
	break;
	
	case 1:
	mpDebugPrint("/       state= Recording_state	  /");
	break;
	
	case 2:
	mpDebugPrint("/       state= Recording_stop_state /");
	break;

	case 3:
	mpDebugPrint("/       state= Recording_pause_state/");
	break;

	case 4:
	mpDebugPrint("/       state= Preview_stop_state   /"); 
	break;

	case 5:
	mpDebugPrint("/       state= StandBy_state 	      /");
	break;

	case 6:
	mpDebugPrint("/       state= ExceptionCheck_state /");
	break;

	default:	
	mpDebugPrint("/       state= Unknown_state 	      /");
	break;

	}
	
}

void Showinstruction(int instruction)
{   	
	
	
	switch (instruction)
    {
	case 0:
	mpDebugPrint("/ instruction= Preview_Start 	      /");
	break;
	case 1:
	mpDebugPrint("/ instruction= Recording_Sstart     /");
	break;
	case 2:
	mpDebugPrint("/ instruction= Recording_Stop	      /");
	break;
	case 3:
	mpDebugPrint("/ instruction= Preview_Stop		  /");
	break;
	case 4:
	mpDebugPrint("/ instruction= Recording_Pause      /"); 
	break;
	case 5:
	mpDebugPrint("/ instruction= Recording_Resume     /");
	break;
	case 6:
	mpDebugPrint("/ instruction= Recording_Exception  /");
	break;
	default:
	mpDebugPrint("/ instruction= Unknown			  /");
	break;
	}
	
}



int RecordTaskStattusGet(void)
{
    return recordTaskState;
}


#endif

