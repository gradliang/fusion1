                                                                                                                                                                                                                                                                      
/*        
// define this module show debug message or not,  0 : disable, 1 : enable
*/                              
                       
#define LOCAL_DEBUG_ENABLE 0
  
#include "global612.h"
#include "mpTrace.h"
//#include "xpgFunc.h"
#include "ui.h"
#include "filebrowser.h"

#if  EREADER_ENABLE


#if READERDISPLAY_ENABLE

#include "../../typesetting/INCLUDE/typesetting.h"
#include "../../typesetting/INCLUDE/Typesetting_task.h"

void xpgCb_WordBTNUp(void)
{
	BYTE index;
	DWORD listcount;
	DWORD curindex, total;
	STXPGMOVIE *pstMovie = &g_stXpgMovie;
   	DWORD dwFileType;
	
	index = 0;
	xpgMenuListPrev();
}	

void	xpgCb_WordBTNDown(void)
{
	BYTE index;
	DWORD listcount;
	DWORD curindex, total;
	STXPGMOVIE *pstMovie = &g_stXpgMovie;
	DWORD dwFileType;
	index = 0;

	xpgMenuListNext();
}

void	xpgCb_WordBTNLeft(void)
{

}
void	xpgCb_WordBTNRight(void)
{

}

BYTE GetEbookFileType(ST_SEARCH_INFO *pSearchInfo)
{
	BYTE FileType;
	BYTE bExt[4];
	BYTE i;
	DWORD dwExt = 0;

	memcpy(bExt, pSearchInfo->bExt, 4);

	for (i = 0; i < 4; i ++)
	{
		dwExt <<= 8;
		dwExt |= toupper(bExt[i]);
	}
	
	dwExt &= 0xFFFFFF00;
	dwExt |= 0x0000002E;

	MP_DEBUG1("Ebook File Ext = 0x%x", dwExt);

	switch (dwExt)
	{

		case EXT_TXT:
			FileType = EBOOK_TYPE_TXT;
			break;
			
		default:
			FileType = EBOOK_TYPE_UNKNOWN;
			break;
	}

	return FileType;
}

 BYTE FileType2;
 DWORD TypesettingStatus,PageReadyStatus ;
 DWORD RPagetotalindex;
 SDWORD RPageCurindex;
 int ExitFlag,ExitTaskFlag1,ExitTaskFlag2;
 SDWORD CacheDecodePageIndex;

 void EGlobalInit()
 {
	TypesettingStatus =NoTypesetting;
	PageReadyStatus=NoReady;
	RPagetotalindex=0;
	RPageCurindex=0;
	ExitTaskFlag1=0;
	ExitTaskFlag2=0;
	ExitFlag=0;
	CacheDecodePageIndex=0;
	FileType2=0;

 }
void	xpgCb_WordBTNEnter(void)
{
	ST_SEARCH_INFO *pSearchInfo;	
	int iIndex;
	STREAM *sHandle;
	BYTE FileType;
	int mem;
	
	EGlobalInit();
	
	if (FileBrowserGetCurFileType() == FILE_OP_TYPE_FOLDER)
	{
		xpgSearchAndGotoPage("Mode_Reader");
		xpgCb_OpenFolder();
		return;
	}

	xpgSearchAndGotoPage("Reader_Viewer");
	xpgUpdateStage();
	iIndex = FileBrowserGetCurIndex();	
	
	pSearchInfo = (ST_SEARCH_INFO *)FileGetSearchInfo(iIndex);

	if (pSearchInfo == NULL)
	{
		xpgCb_GoPrevPage();
		return;
	}

	FileType = GetEbookFileType(pSearchInfo);

	StopRtcDisplay();
	ClearClock();
	
	FileType2=FileType;
	
	mem=ext_mem_get_free_space();
	mpDebugPrint("  ~~~~~~~~~~~~~~~~~~~~ xpgCb_WordBTNEnter  START  mem=%d  FileType =%d CacheNumber=%d",mem,FileType,CacheNumber);

	switch(FileType)
	{

#if Make_TXT
		case EBOOK_TYPE_TXT:
			sHandle = FileListOpen(DriveGet(DriveCurIdGet()), pSearchInfo);
			if (sHandle == NULL)
				return;

			main_txt(sHandle);	
			FileClose(sHandle);
			TypesettingStatus =EnterTypesetting;
			ReaderTypesettingWinInit();
			ReaderCacheMode_DisplayWinInit();
			TypesettingEvent_ReaderTxtTypesetting();
			TypesettingEvent2_ReaderTxtCacheWinCheck();
			
			break;		
#endif			
		default:
			xpgCb_GoPrevPage();
			break;
	}
}

void	xpgCb_WordBTNExit(void)
{
	//STREAM *sHandle;
	ST_SEARCH_INFO *pSearchInfo;
	BYTE FileType;
	int iIndex,mem;

	if(ExitFlag==1)
		{
			mpDebugPrint("----- Exit ExitFlag= %d",ExitFlag);
			return;
		}
	if(TypesettingStatus==NoTypesetting)
		{
			mem=ext_mem_get_free_space();
			mpDebugPrint(" ==>TypesettingStatus =%d EXIT mem=%d",TypesettingStatus,mem);
			//StartRtcDisplay();
			xpgCb_EnterWordMenu();
			return;
		}
	iIndex = FileBrowserGetCurIndex();	
	
	pSearchInfo = (ST_SEARCH_INFO *)FileGetSearchInfo(iIndex);

	if (pSearchInfo == NULL)
	{
		xpgCb_GoPrevPage();
		return;
	}

	FileType = GetEbookFileType(pSearchInfo);

	mpDebugPrint(" FileType <=========== %d",FileType);

	switch(FileType)
	{
#if Make_TXT
		case EBOOK_TYPE_TXT:

			txt_info_exit();
			ExitFlag=1;
						
			while(ExitFlag)
			{	
				TaskYield();

				if((ExitTaskFlag1==1)&&(ExitTaskFlag2==1))
					{
						ReaderCacheMode_WinFree();
						ReaderDisplayWinFreeForCacheMode();
						DebugReaderRecordFree();
						break ;
					}
			}
			break;
#endif			


			break;
		

	}
	TypesettingStatus =NoTypesetting;
	mem=ext_mem_get_free_space();
	mpDebugPrint("  ~~~End ~~EXIT mem=%d",mem);
	MemLeakChk(2);

	StartRtcDisplay();

	xpgCb_EnterWordMenu();
}

void xpgCb_EreaderZoom(void)
{
			


}

void	xpgCb_EreaderAutoSlideshow(void)
{

}

void xpgCb_EreaderChapterUp(void)
{

}	

void	xpgCb_EreaderChapterDown(void)
{

}
	

void	xpgCb_EreaderPageUp(void)
{
	CacheWinInfo *C_WinInfo=(CacheWinInfo *)ReaderCacheMode_GetInfo();
	int Cindex=0,xx=0,temp=0;

	RPageCurindex--;

	if(RPageCurindex<0)
	{
		RPageCurindex++;
		return ;
	}

	mpDebugPrint(" xpgCb_EreaderPageUp   RPageCurindex =%d CacheDecodePageIndex=%d",RPageCurindex,CacheDecodePageIndex);

	Cindex=RPageCurindex%CacheNumber;

	if(RPageCurindex+CacheNumber>RPagetotalindex)
	{
		while(C_WinInfo->Cachestatus[Cindex]!=CacheWinEnd) 
			TaskYield();
	
		Ereader_WinScaleCopy(C_WinInfo->EreaderCachewin[Cindex],Idu_GetCurrWin());
	}
	else
	{
		Ereader_WinScaleCopy(Idu_GetNextWin(),Idu_GetCurrWin());

		if(C_WinInfo->Cachestatus[Cindex]!=0)
			{
				while(C_WinInfo->Cachestatus[Cindex]!=CacheWinEnd) 
					TaskYield();
			}

		C_WinInfo->Cachestatus[Cindex]=CacheWinEnd;

		if((CacheNumber!=1)||(RPageCurindex+1!=RPagetotalindex))
			CacheDecodePageIndex--;

		Ereader_WinScaleCopy(Idu_GetNextWin(),C_WinInfo->EreaderCachewin[Cindex]);

		for(xx=0;xx<CacheNumber;xx++)
		{
			temp=(Cindex+xx)%CacheNumber;
			while(C_WinInfo->Cachestatus[temp]!=CacheWinEnd)
				TaskYield();
		}

		if(RPageCurindex-1>=0)
			{
			if(FileType2==EBOOK_TYPE_TXT)
				Reader_View(RPageCurindex-1,0);

			}
	}

}


void	xpgCb_EreaderPageDown(void)
{
	CacheWinInfo *C_WinInfo=(CacheWinInfo *)ReaderCacheMode_GetInfo();
	int Cindex=0,tempindex=RPageCurindex;
	
	RPageCurindex++;

	mpDebugPrint(" xpgCb_EreaderPageDown  RPageCurindex =%d RPagetotalindex=%d ",RPageCurindex,RPagetotalindex);

	if(RPageCurindex>RPagetotalindex)
		{
			RPageCurindex--;
			return ;
		}

	Cindex=RPageCurindex%CacheNumber;

	if(CacheNumber==1)
		C_WinInfo->Cachestatus[Cindex]=0;

	while(C_WinInfo->Cachestatus[Cindex]!=CacheWinEnd) 
		TaskYield();

	while(RPageCurindex+CacheNumber-1>RPagetotalindex)
		{
			TaskYield();

			if(TypesettingStatus==TypesettingChapterEnd)
				break;
		}

	if(tempindex+CacheNumber<=RPagetotalindex)
		{
			Ereader_WinScaleCopy(Idu_GetCurrWin(),Idu_GetNextWin());

			if(CacheNumber!=1)
				C_WinInfo->Cachestatus[tempindex%CacheNumber]=0;		
		}

	Ereader_WinScaleCopy(C_WinInfo->EreaderCachewin[Cindex],Idu_GetCurrWin());
}



#else //READERDISPLAY_ENABLE
//////////////////////////
////  TTF Use                  ////
//////////////////////////
//#include "wordfunc.h" 
#include "../../typesetting/INCLUDE/typesetting_task.h"
#include "../../../../libIPLAY/libSrc/epub/include/xmlepub.h"
#include "../../../../libIPLAY/include/PdfApi.h"
#include "../../../../libIPLAY/libSrc/pdb/include/pdbtool/pdb_mpx.h"
#include "../../../../libIPLAY/libSrc/fontengine/include/basic_enum.h"
#include "../../typesetting/INCLUDE/typesetting.h"

//extern ST_SYSTEM_CONFIG g_sSystemConfig;
//char g_sEPUB_FILENAME[256];
BYTE FileExt[5];   
BYTE FileName[256];

ST_WORD_BTN WORD_BTNFunc[]=
{
	//{XPG_MODE_WORD,Net_PhotoSet_GetCount,Net_PhotoSet_GetCurrentindex,Net_PhotoSet_SetListindex,Net_PhotoSet_SetFirstindex,Net_PhotoSet_SetCurrentindex},


};

void xpgCb_WordBTNUp(void)
{
	BYTE index;
	DWORD listcount;
	DWORD curindex, total;
	STXPGMOVIE *pstMovie = &g_stXpgMovie;
   	DWORD dwFileType;
	
	index = 0;
	xpgMenuListPrev();
}	

void	xpgCb_WordBTNDown(void)
{
	BYTE index;
	DWORD listcount;
	DWORD curindex, total;
	STXPGMOVIE *pstMovie = &g_stXpgMovie;
	DWORD dwFileType;
	index = 0;

	xpgMenuListNext();
}

void	xpgCb_WordBTNLeft(void)
{

}
void	xpgCb_WordBTNRight(void)
{

}

BYTE GetEbookFileType(ST_SEARCH_INFO *pSearchInfo)
{
	BYTE FileType;
	BYTE bExt[4];
	BYTE i;
	DWORD dwExt = 0;

	memcpy(bExt, pSearchInfo->bExt, 4);

	for (i = 0; i < 4; i ++)
	{
		dwExt <<= 8;
		dwExt |= toupper(bExt[i]);
	}
	
	dwExt &= 0xFFFFFF00;
	dwExt |= 0x0000002E;

	MP_DEBUG1("Ebook File Ext = 0x%x", dwExt);

	switch (dwExt)
	{
		case EXT_DOC:
			FileType = EBOOK_TYPE_DOC;
			break;
		case EXT_XML:
			FileType = EBOOK_TYPE_XML;
			break;
		case EXT_PDF:
			FileType = EBOOK_TYPE_PDF;
			break;
		case EXT_PGM:
			FileType = EBOOK_TYPE_PGM;
			break;

		case EXT_PPM:
			FileType = EBOOK_TYPE_PPM;
			break;			
		case EXT_EPUB:
			FileType = EBOOK_TYPE_EPUB;
			break;
	
	#if Make_UNRTF
		case EXT_RTF:
			FileType = EBOOK_TYPE_RTF;
			break;
	#endif
	#if Make_LRF
		case EXT_LRF:
			FileType = EBOOK_TYPE_LRF;
			break;
	#endif
	#if Make_PDB
		case EXT_PDB:
			FileType = EBOOK_TYPE_PDB;
			break;
	#endif		
		case EXT_TXT:
			FileType = EBOOK_TYPE_TXT;
			break;
			
		default:
			FileType = EBOOK_TYPE_UNKNOWN;
			break;
	}

	return FileType;
}

#define CacheMode_Enable 1 //For PDF mode 1:Cache Mode (NEW)  0: PPM mode(OLD)

 BYTE FileType2;
 DWORD TypesettingStatus,PageReadyStatus ;
 DWORD pagetotalindex;
 SDWORD pageCurindex;
 DWORD Zoompagetotalindex;
 SDWORD ZoompageCurindex;
 DWORD ZoomTypesettingStatus ;
 int ExitFlag,ExitTaskFlag1,ExitTaskFlag2;
 int chaptertotalindex,chapterCurindex;
 int TotalPage,Pdfcurpageindex,PdfMode,PageDirect;
 int ZoomFlag,ZoomPageDirect;
 BYTE EreaderSlideshow;
 SDWORD CacheDecodePageIndex;

 void EGlobalInit()
 {
	TypesettingStatus =NoTypesetting;
	PageReadyStatus=NoReady;
	chaptertotalindex=0;
 	chapterCurindex=0;
	pagetotalindex=0;
	pageCurindex=0;
	Pdfcurpageindex=0;
	PageDirect=0;
	ZoomFlag=0;
	ExitTaskFlag1=0;
	ExitTaskFlag2=0;
	ExitFlag=0;
	Zoompagetotalindex=0;
	ZoompageCurindex=0;
	PdfMode=1;
	ZoomPageDirect=0;
	ZoomTypesettingStatus=NoTypesetting;
	EreaderSlideshow=0;
	CacheDecodePageIndex=0;

 }
void	xpgCb_WordBTNEnter(void)
{
	ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
	
	char *argv1[] = {"Anti0409.exe","-x","db","testdoc.doc"};
	char pdb[5][32]= {"pdb.exe","-generic","unpack","","pdbout.txt"};
	char *argv[5];
	//---------------------------12345678901234567890123456789012// 
	//char *argv_m[]={"ppmpgm.exe","                                                        "};
	//ST_SYSTEM_CONFIG *psSysConfig; 
	ST_SEARCH_INFO *pSearchInfo;	
	int iIndex,status,i;
	STREAM *sHandle;
	BYTE FileType;
	static char tmp[256];
	WORD *str;
	SWORD sRet;
	int Width, Height, BufferAddress,mem;
	XML_Iterator_t *pIt;
	ST_IMGWIN *PpmSrcWin;

	EGlobalInit();
	
	if (FileBrowserGetCurFileType() == FILE_OP_TYPE_FOLDER)
	{
		//xpgGotoPage(g_pstMenuPage->m_wIndex);
		xpgSearchAndGotoPage("Mode_Reader");
		xpgCb_OpenFolder();
		return;
	}

	xpgSearchAndGotoPage("Reader_Viewer");
	//xpgUpdateStage();
	iIndex = FileBrowserGetCurIndex();	
	
	pSearchInfo = (ST_SEARCH_INFO *)FileGetSearchInfo(iIndex);

	if (pSearchInfo == NULL)
	{
		xpgCb_GoPrevPage();
		return;
	}
#if 0
	FileType = FileBrowserGetCurFileType();
#else
	FileType = GetEbookFileType(pSearchInfo);
	MP_DEBUG1("Ebook File Type = 0x%x", FileType);
#endif
	StopRtcDisplay();
	//Idu_OsdErase();
	ClearClock();
	
	FileType2=FileType;
	
	mem=ext_mem_get_free_space();
	mpDebugPrint("  ~~~~~~~~~~~~~~~~~~~~ xpgCb_WordBTNEnter  START  mem=%d  FileType =%d CacheNumber=%d",mem,FileType,CacheNumber);

	switch(FileType)
	{
#if Make_PDB	
		case EBOOK_TYPE_PDB:
			//xpgUpdateStage();
			//DisplayWinInit();			
			//type_test();
			sHandle = FileListOpen(DriveGet(DriveCurIdGet()), pSearchInfo);
			if (sHandle == NULL)
				return;
		
			word_xmltaginit();

			if(!FileBrowserGetFileName(pSearchInfo, FileName, 256, FileExt))
			{
				memset(pdb[3],0,32);
				sprintf(pdb[3],"%s%s",FileName,FileExt);
				argv[0] = pdb[0];
				argv[1] = pdb[1];
				argv[2] = pdb[2];
				argv[3] = pdb[3];
				argv[4] = pdb[4];
			}
			else
				return;
			
			main_pdb(sHandle,5,argv);	
			FileClose(sHandle);	
		
			word_xmltagfree();

			mem=ext_mem_get_free_space();
			mpDebugPrint("  ~~~~~~~~~~~~~~~~~~~~ xpgCb_WordBTNEnter  START  mem=%d",mem);

			//pdb_data_first(); //get the fist chapter <tag> link to typesetting
			FE_INIT();
			DisplayWinInit();
			//PDB_Start_Typesetting();

			TypesettingEvent_SetPage();
			TaskYield();
			break;		
#endif			
		case EBOOK_TYPE_PPM:
		case EBOOK_TYPE_PGM:					
			if(!FileBrowserGetFileName(pSearchInfo, FileName, 256, FileExt))
			{
				memset(tmp,0,256);
				sprintf(tmp,"ppmpgm.exe %s%s",FileName,FileExt);
			}
			else
				return;

			mpDebugPrint("FileName=%s  FileExt=%s",FileName,FileExt);
			sHandle = FileListOpen(DriveGet(DriveCurIdGet()), pSearchInfo);
			ppm2yuv_main2(sHandle);
			FileClose(sHandle);
			mem=ext_mem_get_free_space();
			mpDebugPrint("  PPM END~~~~~~~~~~~~~~~ xpgCb_WordBTNEnter  START  mem=%d",mem);

			break;
#if Make_ANTIWORD
		case EBOOK_TYPE_DOC:
			TypesettingStatus =EnterTypesetting;
			sHandle = FileListOpen(DriveGet(DriveCurIdGet()), pSearchInfo);
			if (sHandle == NULL)
				return;
			word_xmltaginit();
			main_word(sHandle,4,argv1);
		#if (BOOK_DUMP_PIC != 2)
			FileClose(sHandle);	
		#endif
			word_xml_Write2Mem_test(0);
			
			FE_INIT();
			DisplayWinInit();
			status = word_xmlinit(FileType);
			Typesettinginit(status);
		#if 0//(BOOK_DUMP_PIC == 1)
			Doc_DeletePicFile();	//Delete Dumped picture files
		#endif
			if(!status)
				return;			
			break;
#endif

#if Make_PDF	
		case EBOOK_TYPE_PDF:
		{
			//BYTE FileName[256], FileExt[5];
			int err=TRUE,ret=0;
#if CacheMode_Enable
			TypesettingStatus =EnterTypesetting;
			CacheMode_DisplayWinInit();
			ret=FE_INIT();

			#if 1 //debug
				if(ret <0)
				{
					mpDebugPrint("  ERROR xpgCb_WordBTNEnter ===>  Not find font ret=%d",ret);
					ExitTaskFlag1=1;
					ExitTaskFlag2=1;
					return ;
				}
			#endif
			
			mem=ext_mem_get_free_space();
			mpDebugPrint("  PDF CatchMode ===>>  START  mem=%d  TypesettingStatus=%d",mem,TypesettingStatus);	

			CacheWinInfo *C_WinInfo=(CacheWinInfo *)CacheMode_GetInfo();

			if(!FileBrowserGetFileName(pSearchInfo, FileName, 256, FileExt))			
			{
				TypesettingStatus=Pdf2PpmDecoding;
				err = pdf2ppm(FileName, PPM_FORMAT_COLOR, 1, &TotalPage, &BufferAddress, &Width, &Height);

				if(ExitFlag==1)
				{
					TypesettingStatus=Pdf2PpmEnd;
					return ;
				}

				mpDebugPrint("PDF  FileName =%s BufferAddress =%x TotalPage=%d",FileName,BufferAddress,TotalPage);
				mpDebugPrint("PDF   Width =%d Height=%d",Width,Height);

			}
			
			Pdfcurpageindex=1;
			CacheDecodePageIndex=1;

			if(err==FALSE)
				{
					PpmSrcWin=(ST_IMGWIN *)ppm2yuv_main(BufferAddress);		
					ST_IMGWIN *trg_win=(ST_IMGWIN *)Win_New(ALIGN_CUT_16(Idu_GetCurrWin()->wHeight), ALIGN_CUT_16(Idu_GetCurrWin()->wWidth));
					Ereader_WinScale(PpmSrcWin,trg_win);
					Win_Free(PpmSrcWin);
					PpmSrcWin=(ST_IMGWIN *)CacheMode_DisplayWinSelect(1,CacheDecodePageIndex%CacheNumber,0);
					Img_Rotate_PP(PpmSrcWin, trg_win, PP_ROTATION_RIGHT_90);
					Win_Free(trg_win);
					Ereader_WinScale(PpmSrcWin,Idu_GetCurrWin());
				}
			else
				{
					//ext_mem_free(BufferAddress);
					PpmSrcWin=(ST_IMGWIN *)CacheMode_DisplayWinSelect(1,CacheDecodePageIndex%CacheNumber,0);
					Idu_PaintWin(PpmSrcWin, 0x00008080);
					char* str="This Page Not Support !";
					FE_MPX_Drawfont(str,40,PpmSrcWin,200,Idu_GetNextWin()->wHeight/2,255,255,255,0);
					Ereader_WinScale(PpmSrcWin,Idu_GetCurrWin());
					mpDebugPrint("pdf2ppm  Page 1  ERROR!!");
				}
			
			if(CacheNumber>1)
				C_WinInfo->Cachestatus[1]=Pdf2PpmEnd;
			else
				{
					C_WinInfo->Cachestatus[0]=Pdf2PpmEnd;
					TypesettingStatus=Pdf2PpmEnd;
				}

			mem=ext_mem_get_free_space();
			mpDebugPrint("PDF   CacheNumber=%d   mem=%d",CacheNumber,mem);

		#if 0
			int xx=0;

			for(xx=1;xx<CacheNumber;xx++)
				{
					if(xx<TotalPage)
						{
							CacheDecodePageIndex++;
							CacheMode_PDF2CacheWinBuffer(CacheDecodePageIndex);
						}
					else
						C_WinInfo->Cachestatus[(xx+1)%CacheNumber]=Pdf2PpmEnd;
				}
		#endif
			TypesettingEvent_PdfCheckCatchWin();

//#endif
//#if	0	//PPM mode
#else //PPM mode
			TypesettingStatus =EnterTypesetting;
			ret=FE_INIT();
			PDF2PPMDisplayWinInit();	
			ClearClock();
			StopRtcDisplay();
				
			mem=ext_mem_get_free_space();
			mpDebugPrint("  PPM 111 ~~~~~~~~~~~~~~~ xpgCb_WordBTNEnter  START  mem=%d  TypesettingStatus=%d",mem,TypesettingStatus);
				
			#if 1 //debug
			if(ret <0)
				{
					mpDebugPrint("  ERROR xpgCb_WordBTNEnter ===>  Not find font ret=%d",ret);
					ExitTaskFlag1=1;
					ExitTaskFlag2=1;
					return ;
				}
			#endif
			
			if(!FileBrowserGetFileName(pSearchInfo, FileName, 256, FileExt))
			{
				TypesettingStatus=Pdf2PpmDecoding;
				err = pdf2ppm(FileName, PPM_FORMAT_COLOR, 1, &TotalPage, &BufferAddress, &Width, &Height);
				//FE_Release();
			}

			if(ExitFlag==1)
				{
					TypesettingStatus=Pdf2PpmEnd;
					return ;
				}
			mpDebugPrint("PDF  FileName =%s BufferAddress =%x TotalPage=%d",FileName,BufferAddress,TotalPage);
			mpDebugPrint("PDF   Width =%d Height=%d",Width,Height);

			if(err == FALSE)
			{		
				Pdfcurpageindex=1;			

				PpmSrcWin=(ST_IMGWIN *)ppm2yuv_main(BufferAddress);				

				ST_IMGWIN *trg_win=(ST_IMGWIN *)Win_New(ALIGN_CUT_16(Idu_GetCurrWin()->wHeight), ALIGN_CUT_16(Idu_GetCurrWin()->wWidth));

				Ereader_WinScale(PpmSrcWin,trg_win);

				mpDebugPrint("PDF  trg_win  Width =%d Height=%d",trg_win->wWidth,trg_win->wHeight);
				Win_Free(PpmSrcWin);

				Img_Rotate_PP(Idu_GetCurrWin(), trg_win, PP_ROTATION_RIGHT_90);
		
				Win_Free(trg_win);

				if(TotalPage>1)
					{
						Pdfcurpageindex++;
						TypesettingEvent_PdfPage2winbuffer();
					}
				else
					TypesettingStatus=Pdf2PpmEnd;

			}
#endif
#if 0 //XML MODE

			TypesettingStatus =EnterTypesetting;
			PdfMode=0;
			
			if(!FileBrowserGetFileName(pSearchInfo, FileName, 256, FileExt))
			{
				word_xmltaginit();
				err = pdf2xml(FileName, 8, &TotalPage);

			}

			if(err == FALSE)
			{
				FE_INIT();
				DisplayWinInit();

				Pdfcurpageindex=1;

				word_xml_Write2Mem_test(2);		
				status=word_xmlinit(FileType);
				PDF_DataInit ();

				PDF_Typesettinginit(status,FileType);
				

			}

			
#endif
			
		}
		break;
#endif
#if Make_UNRTF
		case EBOOK_TYPE_RTF:
			TypesettingStatus =EnterTypesetting;
			sHandle = FileListOpen(DriveGet(DriveCurIdGet()), pSearchInfo);
			if (sHandle == NULL)
				return; 
			word_xmltaginit();
			sRet = Unrtf(sHandle);
			FileClose(sHandle);
			if (sRet == FAIL)
				xpgCb_GoPrevPage();
			word_xml_Write2Mem_test(0);

			////FE_INIT();
			////DisplayWinInit();
			status = word_xmlinit(FileType);
			if(!status)
				return;
			break;
#endif

#if Make_TXT
		case EBOOK_TYPE_TXT:
			sHandle = FileListOpen(DriveGet(DriveCurIdGet()), pSearchInfo);
			if (sHandle == NULL)
				return;
#if EPUB_DEBUG			
			word_xmltaginit();
#endif
			main_txt(sHandle);	
			FileClose(sHandle);
#if EPUB_DEBUG			
			word_xmltagfree();
#endif
			mem=ext_mem_get_free_space();
			mpDebugPrint("  ~~~~~~~~~~~~~~~~~~~~ xpgCb_WordBTNEnter  START  mem=%d",mem);
		#if	0	
			FE_INIT();
			//FE_FontTTF_Init(SD_MMC,"kaiu.tff",0,FONT_DATA_AT_MEMORY);
			DisplayWinInit();

			TypesettingEvent_SetPage();
			TaskYield();
		#else //

#if 1

TypesettingEvent_SetPage();
#else
			FE_INIT();
			CacheMode_DisplayWinInit();
			DisplayWinInitForCacheMode();
			TypesettingEvent_SetPage();
			TypesettingEvent2_TXTCheckCatchWin();	
			#endif
		#endif
			
			break;		
#endif			
#if Make_EPUB			
		case EBOOK_TYPE_EPUB:
#if 	1	//CacheMode
			TypesettingStatus =EnterTypesetting;
			sHandle = FileListOpen(DriveGet(DriveCurIdGet()), pSearchInfo);

			if (sHandle == NULL)
				return; 			

			if(pSearchInfo->dwLongFdbCount >= 1)
    			{   	

#if EPUB_DEBUG				
				word_xmltaginit(); //prepare output file in memory 
#endif				
				Einfo_main(sHandle);
			
				pIt = (XML_Iterator_t *)xml_epub_parsing_First(); //get the fist chapter <tag> link to typesetting

#if EPUB_DEBUG
                //1. parsing all iterator(chapter) to debug file
                //2. No typesetting just epub xml parsing out for debug 
				while(pIt)
					pIt = xml_epub_parsing_next();
				
				xml_epub_metadata_test();  //output metadata info 
				xml_epub_get_Nav_test();   //output navdata info 
				xml_epub_tag_test();       //outpur all tag and content data 
				word_xml_Write2Mem_test(1);//0: no output; 1: output uart 2:output debug file
				word_xmltagfree();         //free all data of output file in memory         
#else 
				if(pIt != NULL)
				{
					CacheMode_DisplayWinInit();
					DisplayWinInitForCacheMode();
					Epub_TypesettingInit(pIt);
					TypesettingEvent2_EPUBCheckCatchWin();	
	
				}
#endif				
				FileClose(sHandle);
			}			

#endif

			break;

#endif
#if Make_LRF
		case EBOOK_TYPE_LRF:
			TypesettingStatus =EnterTypesetting;
			sHandle = FileListOpen(DriveGet(DriveCurIdGet()), pSearchInfo);
			if (sHandle == NULL)
				return; 
			word_xmltaginit();
			sRet = LrfParser(sHandle);
		#if (BOOK_DUMP_PIC != 2)
			FileClose(sHandle);
		#endif
			if (sRet == FAIL)
				xpgCb_GoPrevPage();
			word_xml_Write2Mem_test(0);
			////FE_INIT();
			////DisplayWinInit();
			status = word_xmlinit(FileType);
		#if 0//(BOOK_DUMP_PIC == 1)
			LRF_DeletePicFile();		//Delete Dumped picture files
		#endif
			if(!status)
				return;
			break;
#endif
		default:
			xpgCb_GoPrevPage();
			break;
	}
}

void	xpgCb_WordBTNExit(void)
{
	//STREAM *sHandle;
	ST_SEARCH_INFO *pSearchInfo;
	BYTE FileType;
	int iIndex,mem;

	if(ExitFlag==1)
		{
			mpDebugPrint("----- Exit ExitFlag= %d",ExitFlag);
			return;
		}
	if(TypesettingStatus==NoTypesetting)
		{
			mem=ext_mem_get_free_space();
			mpDebugPrint(" ==>TypesettingStatus =%d EXIT mem=%d",TypesettingStatus,mem);
			//StartRtcDisplay();
			xpgCb_EnterWordMenu();
			return;
		}
	iIndex = FileBrowserGetCurIndex();	
	
	pSearchInfo = (ST_SEARCH_INFO *)FileGetSearchInfo(iIndex);

	if (pSearchInfo == NULL)
	{
		xpgCb_GoPrevPage();
		return;
	}

	FileType = GetEbookFileType(pSearchInfo);

	//DisplayWinFree();
	//FE_Release();

	mpDebugPrint(" FileType <=========== %d",FileType);

	switch(FileType)
	{
#if Make_PDB
		case EBOOK_TYPE_PDB:
			ExitFlag=1;
			ExitTaskFlag1=1;
			ExitTaskFlag2=1;
			
			while(ExitFlag)
			{	
				TaskYield();

				if((ExitTaskFlag1==1)&&(ExitTaskFlag2==1))
				{
					#if 0
						pdb_info_exit();
						DisplayWinFree();
						FE_Release();
					#else
						User_mem_init();
					#endif
						break ;
					}
			}
			break;
#endif	
#if Make_TXT
		case EBOOK_TYPE_TXT:

			txt_info_exit();
			
			ExitFlag=1;
						
			while(ExitFlag)
			{	
				TaskYield();

				if((ExitTaskFlag1==1)&&(ExitTaskFlag2==1))
				{
					#if 0
						pdb_info_exit();
						DisplayWinFree();
						FE_Release();
					#else
						User_mem_init();
					#endif
						break ;
					}
			}
			break;
#endif			
#if Make_ANTIWORD	
		case  EBOOK_TYPE_DOC:
			word_xmlexit();
			break;
#endif			
		case  EBOOK_TYPE_XML:
			word_xmlexit();
			break;
#if Make_PDF			
              case EBOOK_TYPE_PDF:
		#if 1 //PPM mode

		ExitFlag=1;
	#if CacheMode_Enable
		while(ExitFlag)
			{	

				while(EreaderSlideshow==1)
					TaskYield();
				while(ExitTaskFlag1==0)
					TaskYield();
				TaskYield();		
				if((TypesettingStatus==Pdf2PpmEnd)||(TotalPage==1))
					{
						//FE_Release();
						User_mem_init();
						break ;
					}
			}
	#else
		while(ExitFlag)
			{	

				while(EreaderSlideshow==1)
					TaskYield();
				TaskYield();
				if((TypesettingStatus==Pdf2PpmEnd)||(TotalPage==1))
					{
						//FE_Release();
						User_mem_init();
						break ;
					}
			}
	#endif
		#else
			ExitFlag=1;
					
		while(ExitFlag)
			{	
				TaskYield();

				if((ExitTaskFlag1==1)&&(ExitTaskFlag2==1))
					{
					word_Pdfxmlexit();
					FE_Release();
					PDF_DataFree();
					DisplayWinFree();
					MemLeakChk(2);
						break ;
					}
			}
			word_Pdfxmlexit();
		#endif

			break;
#endif		
#if Make_UNRTF
	       case EBOOK_TYPE_RTF:
		   	word_xmlexit();
		   	break;
#endif			
#if Make_LRF
		case EBOOK_TYPE_LRF:
			word_xmlexit();
			break;
#endif
#if Make_EPUB
		case EBOOK_TYPE_EPUB:
			
#if EPUB_DEBUG 		
            xml_epub_info_exit();			
			Einfo_main_exit();				
#else
			ExitFlag=1;
			
			while(ExitFlag)
			{	
				TaskYield();

				if((ExitTaskFlag1==1)&&(ExitTaskFlag2==1))
				{
					while(EreaderSlideshow==1)
						TaskYield();
					#if 0
						xml_epub_info_exit();
						Einfo_main_exit();		
						DisplayWinFree();
						FE_Release();
						Epub_DataFree();
						MemLeakChk(2);
					#else
						User_mem_init();
					#endif
						break ;
				}
			}
#endif					
			break;
#endif		
		case EBOOK_TYPE_PPM:
		case EBOOK_TYPE_PGM:

			break;
		

	}
	TypesettingStatus =NoTypesetting;
	mem=ext_mem_get_free_space();
	mpDebugPrint("  ~~~End ~~EXIT mem=%d",mem);
	MemLeakChk(2);

	StartRtcDisplay();

	xpgCb_EnterWordMenu();
}

void xpgCb_EreaderZoom(void)
{
			
#if Make_PDF	
	if(FileType2==EBOOK_TYPE_PDF)
		{
		#if CacheMode_Enable//CacheMode
			PDF_CacheModeZoom();
		//#endif
		//#if 0
		#else //PPM mode
			if (TypesettingStatus ==Typesettingduring)
	  			return;

			ZoomFlag++;
				
			if(PageDirect==1)
				{
					Pdfcurpageindex+=2;
					PageDirect=0;
				}

			if(ZoomFlag==6)
				{
					//FE_Release();
					DisplayWinFree();
					//PDF_DataFree();
					PdfMode=1;
					
					PDF_Page2WinBuffer(Pdfcurpageindex-1);
					Idu_ChgWin(Idu_GetNextWin());

					if(Pdfcurpageindex>2)
						{
							PDF_Page2WinBuffer(Pdfcurpageindex-2);
							PDF_NextWin2Pdfwin1();
						}
					if(Pdfcurpageindex<TotalPage)
						PDF_Page2WinBuffer(Pdfcurpageindex);
					ZoomFlag=0;
				}
			else
				{
					if(ZoomFlag==1)
						{
							PdfMode=0;
							DisplayWinInit();
							PDF_DataInit ();
							//FE_INIT();	
							Ppm2Xml();
						}
					else
						{	
							Ppm2Xml();
						}
				}
		#endif
		}
#endif

#if Make_EPUB
		if(FileType2==EBOOK_TYPE_EPUB)
		{
		#if 1//CacheMode
			EPUB_CacheModeZoom();
		#endif
		
		#if	0

			if (ZoomTypesettingStatus ==ZoomTypesettingduring)
	  			return;
			if(PageReadyStatus==ZoomNoReady)
				return;
			if(PageReadyStatus==NoReady)
				return;

			ZoomFlag++;

			 if(ZoomFlag==6)
				{
					Epub_ZoomDataFree();	
					Zoompagetotalindex=0;
					ZoompageCurindex=0;
					PageReadyStatus=NoReady;
					Epub_reader_page_view(pageCurindex,0);
					Idu_ChgWin(Idu_GetNextWin());

					if(PageDirect==0)
						{
							if(pageCurindex>0)
								{
								Epub_reader_page_view(pageCurindex-1,0);
								CopyNextWinTo(1);
								}
							if(pageCurindex<pagetotalindex)	
								Epub_reader_page_view(pageCurindex+1,0);
						}
					else{

							if(pageCurindex<pagetotalindex)	
								{
								Epub_reader_page_view(pageCurindex+1,0);
								CopyNextWinTo(1);
								}
							
							if(pageCurindex>0)
								{
								Epub_reader_page_view(pageCurindex-1,0);
								//CopyNextWinTo(1);
								}
							ZoomPageDirect=0;
						}
					PageReadyStatus=Ready;
					ZoomFlag=0;

				}
			else	
				{
					if(ZoomPageDirect==1)
						ZoomPageDirect=0;

					Zoompagetotalindex=0;
					ZoompageCurindex=0;
					
					if(ZoomFlag==1)
						{	
							Epub_Zoom_typesetting(pageCurindex,ZoomFlag);
						}
					else
						{
							Epub_ZoomDataFree();
							Epub_Zoom_typesetting(pageCurindex,ZoomFlag);
						}

				}
		#endif		
		}
#endif

#if Make_TXT
	if(FileType2==EBOOK_TYPE_TXT)
		TXT_CacheModeZoom();

#endif

}

void	xpgCb_EreaderAutoSlideshow(void)
{
	if (EreaderSlideshow==1)
		return;
	else
		EreaderSlideshow=1;

#if Make_PDF
	if(FileType2==EBOOK_TYPE_PDF)
	{
		if(PdfMode==1)
			TypesettingEvent2_AutoSlideShow();
	}
#endif
	
#if Make_EPUB	
	 if(FileType2==EBOOK_TYPE_EPUB)
	{
		if(ZoomFlag==0)
			CacheModeAutoSlideshow();
	}
#endif

#if Make_TXT
	 if(FileType2==EBOOK_TYPE_TXT)
	{
		if(ZoomFlag==0)
			CacheModeAutoSlideshow();
	}
#endif
	
}

void xpgCb_EreaderChapterUp(void)
{
	EreaderSlideshow=0;//PDF stop AutoSlideShow use	
}	

void	xpgCb_EreaderChapterDown(void)
{
	xpgCb_EreaderAutoSlideshow();
#if 0
	if (EreaderSlideshow==1)
		return;
	else
		EreaderSlideshow=1;
#if Make_PDF
	if(FileType2==EBOOK_TYPE_PDF)
	{
		if(PdfMode==1)
			TypesettingEvent2_AutoSlideShow();
	}
#endif
	
#if Make_EPUB	
	 if(FileType2==EBOOK_TYPE_EPUB)
	{
		if(ZoomFlag==0)
			CacheModeAutoSlideshow();
	}
#endif

#if Make_TXT
	 if(FileType2==EBOOK_TYPE_TXT)
	{
		if(ZoomFlag==0)
			CacheModeAutoSlideshow();
	}
#endif

#endif
}
	

void	xpgCb_EreaderPageUp(void)
{
#if Make_PDF	
	if(FileType2==EBOOK_TYPE_PDF)
		PDF_EreaderPageUp();
	else if(FileType2==EBOOK_TYPE_EPUB)
#endif		
		{
#if Make_EPUB
			if (ZoomFlag==0)
				CacheModePageUp();
	 			//EreaderNormalPageUp();
			else
				EPUB_EreaderZoomPageup();
#endif
		}
#if Make_TXT
	if(FileType2==EBOOK_TYPE_TXT)
		{
			if (ZoomFlag==0)
				CacheModePageUp();
	 		else
				PT_EreaderZoomPageup();
		}
#endif	
}
	
void	xpgCb_EreaderPageDown(void)
{
#if Make_PDF	
	if(FileType2==EBOOK_TYPE_PDF)
		PDF_EreaderPageDown();
	else if(FileType2==EBOOK_TYPE_EPUB)
#endif		
		{
#if Make_EPUB
			if (ZoomFlag==0)
				CacheModePageDown();
	 			//EreaderNormalPageDown();
			else
				EPUB_EreaderZoomPageDown();
#endif
		}
#if Make_TXT
	if(FileType2==EBOOK_TYPE_TXT)
	{
		if (ZoomFlag==0)
			CacheModePageDown();
		else
			PT_EreaderZoomPageDown();
	}
#endif
}

int  CacheModePageDown(void)
{
	CacheWinInfo *C_WinInfo=(CacheWinInfo *)CacheMode_GetInfo();
	int Cindex=0,tempindex=pageCurindex;
	
	pageCurindex++;
	mpDebugPrint(" START  CacheModePageDown   pageCurindex =%d pagetotalindex=%d ",pageCurindex,pagetotalindex);
	//mpDebugPrint(" START  CacheDecodePageIndex=%d TypesettingStatus=%d",CacheDecodePageIndex,TypesettingStatus);

	if(pageCurindex>pagetotalindex)
	//if((pageCurindex>pagetotalindex)||((pageCurindex==pagetotalindex)&&(ExitTaskFlag1==0)))
		{
			pageCurindex--;
			return ;
		}

	Cindex=pageCurindex%CacheNumber;

	if(CacheNumber==1)
		C_WinInfo->Cachestatus[Cindex]=0;

	while(C_WinInfo->Cachestatus[Cindex]!=CacheWinEnd) 
		TaskYield();

	while(pageCurindex+CacheNumber-1>pagetotalindex)
		{
			TaskYield();
			if((TypesettingStatus==TypesettingChapterEnd)||(TypesettingStatus==ZoomTypesettingEnd))
				break;
		}

	if(tempindex+CacheNumber<=pagetotalindex)
		{
			Ereader_WinScaleCopy(Idu_GetCurrWin(),Idu_GetNextWin());
			if(CacheNumber!=1)
				C_WinInfo->Cachestatus[tempindex%CacheNumber]=0;		
		}

	Ereader_WinScaleCopy(C_WinInfo->EreaderCachewin[Cindex],Idu_GetCurrWin());

}


int  CacheModePageUp(void)
{
	CacheWinInfo *C_WinInfo=(CacheWinInfo *)CacheMode_GetInfo();
	int Cindex=0,xx=0,temp=0;

	pageCurindex--;

	if(pageCurindex<0)
	{
		pageCurindex++;
		return ;
	}

	mpDebugPrint(" CacheModePageDown   pageCurindex =%d CacheDecodePageIndex=%d",pageCurindex,CacheDecodePageIndex);
	Cindex=pageCurindex%CacheNumber;

	if(pageCurindex+CacheNumber>pagetotalindex)
	{
		while(C_WinInfo->Cachestatus[Cindex]!=CacheWinEnd) 
			TaskYield();
	
		Ereader_WinScaleCopy(C_WinInfo->EreaderCachewin[Cindex],Idu_GetCurrWin());
	}
	else
	{
		Ereader_WinScaleCopy(Idu_GetNextWin(),Idu_GetCurrWin());

		if(C_WinInfo->Cachestatus[Cindex]!=0)
			{
				while(C_WinInfo->Cachestatus[Cindex]!=CacheWinEnd) 
					TaskYield();
			}

		C_WinInfo->Cachestatus[Cindex]=CacheWinEnd;

		if((CacheNumber!=1)||(pageCurindex+1!=pagetotalindex))
			CacheDecodePageIndex--;

		Ereader_WinScaleCopy(Idu_GetNextWin(),C_WinInfo->EreaderCachewin[Cindex]);

		for(xx=0;xx<CacheNumber;xx++)
		{
			temp=(Cindex+xx)%CacheNumber;
			while(C_WinInfo->Cachestatus[temp]!=CacheWinEnd)
				TaskYield();
		}

		if(pageCurindex-1>=0)
			{
			#if Make_TXT
			if(FileType2==EBOOK_TYPE_TXT)
				PDBTXTCacheMode_PageView(pageCurindex-1,0);
			#endif
			#if Make_EPUB
			if(FileType2==EBOOK_TYPE_EPUB)
				Epub_reader_page_view(pageCurindex-1,0);
			#endif
			}
	}

}

int  CacheModeAutoSlideshow(void)
{
	mpDebugPrint("########### CacheModeAutoSlideshow ####");
	while(1)
		{
			while(pageCurindex>pagetotalindex)
				TaskYield();
			//TaskYield();
			if(ExitFlag==1)
				{
					EreaderSlideshow=0;
					break;
				}

			xpgCb_EreaderPageDown();
			TaskYield();
			if((pageCurindex==pagetotalindex)&&(TypesettingStatus==TypesettingChapterEnd))
				break;
			if((pageCurindex==pagetotalindex)&&(TypesettingStatus==ZoomTypesettingEnd))
				break;

			if((ExitFlag==1)||(EreaderSlideshow==0))
				{
					EreaderSlideshow=0;
					break;
				}
		}
	 EreaderSlideshow=0;
}
#if 0

void	EreaderNormalPageUp(void)
{
		MP_DEBUG(" xpgCb_EreaderPageUP Start  pageCurindex =%d ",pageCurindex);

		if(PageReadyStatus==NoReady)
			return;
		
		if (TypesettingStatus ==Typesettingduring)
			{	
				  if((pageCurindex-1)<0)
	  				return;
			}

		if (pagetotalindex==0)
			return;

		pageCurindex--;

		if(pageCurindex==0)
		{

			if(PageDirect==0)
				{
					Win1andNextwinCopy(1);
					Idu_ChgWin(Idu_GetNextWin());
				}
			else
				{
					Idu_ChgWin(Idu_GetNextWin());
					CopyNextWinTo(1);
				}
			if(PageDirect==0)
				PageDirect=1;
			return;
		}

		if(PageDirect==0)
			PageDirect=1;
		
		if (pageCurindex<0)
			{
				pageCurindex++;
				return;
				//pageCurindex=pageCurindex+pagetotalindex+1;
			}

		TypesettingEvent2_PrePage();

}


void	EreaderNormalPageDown(void)
{
		if(PageReadyStatus==NoReady)
			return;
		
		if (TypesettingStatus ==Typesettingduring)
			{
				if (pagetotalindex<(pageCurindex+1))
	  				return;
			}

		if (pagetotalindex==0)
			return;



		pageCurindex++;

		if(pageCurindex==pagetotalindex)
		{
		if(PageDirect==0)
			{
			Idu_ChgWin(Idu_GetNextWin());
			CopyNextWinTo(1);
			}
		else
			{
				Ereader_WinScaleCopy(Idu_GetCurrWin(), Idu_GetNextWin()) ;
				Win1andCurwinCopy(1);
				PageDirect=0;

			}
			return;
		}
		
		if(PageDirect==1)
			PageDirect=0;
		
		if(pageCurindex>pagetotalindex)
		{
			pageCurindex--;
			return;
			//pageCurindex=pageCurindex-pagetotalindex-1;
		}

		TypesettingEvent2_NextPage();

}


#endif
#if Make_TXT
int  TXT_CacheModeZoom(void)
{
	CacheWinInfo *C_WinInfo=(CacheWinInfo *)CacheMode_GetInfo();
	int xx=0,Tempindex=0;

	if (ZoomTypesettingStatus ==ZoomTypesettingduring)
	  	return;
	if(PageReadyStatus==ZoomNoReady)
		return;

	ZoomFlag++;
	mpDebugPrint("==  CCC  TXT_CacheModeZoom =%d pageCurindex=%d  pagetotalindex =%d ZoomFlag=%d ZoomPageDirect=%d PageDirect=%d === ",CacheDecodePageIndex,pageCurindex,pagetotalindex,ZoomFlag,ZoomPageDirect,PageDirect);
	 if(ZoomFlag==6)
		{
			int tempdecodeindex=0;

			ZoomDisplayWinFree();
			Zoompagetotalindex=0;
			ZoompageCurindex=0;

			if((pageCurindex+CacheNumber-1)<pagetotalindex)
				{
					for(tempdecodeindex=pageCurindex;tempdecodeindex<pageCurindex+CacheNumber;tempdecodeindex++)
						{
							if((tempdecodeindex>=CacheDecodePageIndex)||(tempdecodeindex<(CacheDecodePageIndex-CacheNumber+1)))
								{
									PDBTXTCacheMode_PageView(tempdecodeindex,1);
									C_WinInfo->Cachestatus[(tempdecodeindex%CacheNumber)]=CacheWinEnd;
								//	mpDebugPrint(" 111-1  tempdecodeindex =%d C_WinInfo->Cachestatus[tempdecodeindex%CacheNumber]=%d (CacheDecodePageIndex-CacheNumber+1)=%d",tempdecodeindex,C_WinInfo->Cachestatus[tempdecodeindex%CacheNumber],(CacheDecodePageIndex-CacheNumber+1));
								}
							//else
							//	mpDebugPrint(" 111-2  tempdecodeindex =%d C_WinInfo->Cachestatus[tempdecodeindex%CacheNumber]=%d (CacheDecodePageIndex-CacheNumber+1)=%d",tempdecodeindex,C_WinInfo->Cachestatus[tempdecodeindex%CacheNumber],(CacheDecodePageIndex-CacheNumber+1));

							if(tempdecodeindex==pageCurindex)
								Ereader_WinScale(C_WinInfo->EreaderCachewin[pageCurindex%CacheNumber],Idu_GetCurrWin());
						}

					if((pageCurindex+CacheNumber-1)==pagetotalindex)
						CacheDecodePageIndex=pagetotalindex;
					else
						CacheDecodePageIndex=tempdecodeindex;

					if(pageCurindex-1>=0)
						PDBTXTCacheMode_PageView(pageCurindex-1,0);
					//mpDebugPrint(" 111 END tempdecodeindex=%d CacheDecodePageIndex=%d",tempdecodeindex,CacheDecodePageIndex);
				}
			else
				{
					for(xx=pagetotalindex-CacheNumber+1;xx<=pagetotalindex;xx++)
						{
							Tempindex=(xx)%CacheNumber;
							if((xx>=CacheDecodePageIndex))
								{
									C_WinInfo->Cachestatus[Tempindex]=0;
									//mpDebugPrint(" 22 xx=%d C_WinInfo->Cachestatus[Tempindex=%d]=%d",xx,Tempindex,C_WinInfo->Cachestatus[Tempindex]);
								}
							//else
								//mpDebugPrint(" 22--2222  xx=%d C_WinInfo->Cachestatus[Tempindex=%d]=%d",xx,Tempindex,C_WinInfo->Cachestatus[Tempindex]);
						}

					if(C_WinInfo->Cachestatus[(pageCurindex%CacheNumber)]!=CacheWinEnd)
						{
							PDBTXTCacheMode_PageView(pageCurindex,1);
							C_WinInfo->Cachestatus[(pageCurindex%CacheNumber)]=CacheWinEnd;
						}

					Ereader_WinScale(C_WinInfo->EreaderCachewin[pageCurindex%CacheNumber],Idu_GetCurrWin());

					if(pagetotalindex-CacheNumber>0)
						PDBTXTCacheMode_PageView(pagetotalindex-CacheNumber,0);

					if(CacheDecodePageIndex<pagetotalindex-CacheNumber+1)
						CacheDecodePageIndex=pagetotalindex-CacheNumber+1;


				}
			
			PageReadyStatus=Ready;
			ZoomFlag=0;
			TypesettingStatus=ZoomTypesettingEnd;	
			TypesettingEvent2_TXTCheckCatchWin();
		}
	else	
		{
			if(ZoomPageDirect==1)
				ZoomPageDirect=0;

			Zoompagetotalindex=0;
			ZoompageCurindex=0;
			
			if(ZoomFlag==1)
				{	
					TypesettingStatus=CacheWinEnd;
					ZoomDisplayWinInit();
					PDBTXT_Zoom_Typesetting(pageCurindex,ZoomFlag);
				}
			else
				{
					PDBTXTT_ZoomDataFree();
					PDBTXT_Zoom_Typesetting(pageCurindex,ZoomFlag);
				}

		}

}



void	PT_EreaderZoomPageup(void)
{
	if(PageReadyStatus==ZoomNoReady)
		return;
	if (ZoomTypesettingStatus ==ZoomTypesettingduring)
		return;
		
	ZoompageCurindex--;

	if(ZoompageCurindex==0)
		{
			if(ZoomPageDirect==0)
				{
					Win1andNextwinCopy(1);
					Idu_ChgWin(Idu_GetNextWin());
				}
			else
				{
					Idu_ChgWin(Idu_GetNextWin());
					CopyNextWinTo(1);
				}
			if(ZoomPageDirect==0)
				ZoomPageDirect=1;
			return;
		}
	
	if(ZoomPageDirect==0)
		ZoomPageDirect=1;

	if (ZoompageCurindex<0)
		{
			if(pageCurindex<1)
				{
					ZoompageCurindex++;
						return ;
				}
			else
				{
					pageCurindex--;
					Zoompagetotalindex=0;
					ZoompageCurindex=0;	
					PDBTXTT_ZoomDataFree();
					PDBTXT_Zoom_Typesetting(pageCurindex,ZoomFlag);
					ZoompageCurindex=Zoompagetotalindex;
				}

				PDBTXT_Zoom_PageView(Zoompagetotalindex);
				Idu_ChgWin(Idu_GetNextWin());
				CopyNextWinTo(1);

				if ((Zoompagetotalindex)>0)
					PDBTXT_Zoom_PageView(Zoompagetotalindex-1);	
			
		}
	else
		TypesettingEvent2_ZoomPrePage();

}


void	PT_EreaderZoomPageDown(void)
{
	if(PageReadyStatus==ZoomNoReady)
		return;
	if (ZoomTypesettingStatus ==ZoomTypesettingduring)
		return;
		
	if (TypesettingStatus ==Typesettingduring)
		{
			if (pagetotalindex<(pageCurindex+1))
	  			return;
		}

	//mpDebugPrint("PT_EreaderZoomPageDown  Zoompagetotalindex =%d ZoompageCurindex=%d pagetotalindex=%d pageCurindex=%d",Zoompagetotalindex,ZoompageCurindex,pagetotalindex,pageCurindex);

	if(ZoomPageDirect==1)
		ZoomPageDirect=0;

	ZoompageCurindex++;

	if(ZoompageCurindex==Zoompagetotalindex)
		{
			Idu_ChgWin(Idu_GetNextWin());
			CopyNextWinTo(1);
			return;
		}

	if(ZoompageCurindex>Zoompagetotalindex)
		{
			if(pageCurindex>=pagetotalindex)
				{
					ZoompageCurindex--;
					return ;
				}
			else
				{
					ZoompageCurindex=0;
					Zoompagetotalindex=0;
					pageCurindex++;					
					PageReadyStatus=ZoomNoReady;
					PDBTXT_Zoom_Typesetting(pageCurindex,ZoomFlag);
				}
		}
	else
		TypesettingEvent2_ZoomNextPage();

}
#endif

#if Make_EPUB

void	EPUB_EreaderZoomPageup(void)
{
	if(PageReadyStatus==ZoomNoReady)
		return;
	if (ZoomTypesettingStatus ==ZoomTypesettingduring)
		return;
		
	ZoompageCurindex--;

	if(ZoompageCurindex==0)
		{
			if(ZoomPageDirect==0)
				{
					Win1andNextwinCopy(1);
					Idu_ChgWin(Idu_GetNextWin());
				}
			else
				{
					Idu_ChgWin(Idu_GetNextWin());
					CopyNextWinTo(1);
				}
			if(ZoomPageDirect==0)
				ZoomPageDirect=1;
			return;
		}
	
	if(ZoomPageDirect==0)
		ZoomPageDirect=1;

	if (ZoompageCurindex<0)
		{
			if(pageCurindex<1)
				{
					ZoompageCurindex++;
						return ;
				}
			else
				{
					pageCurindex--;
					Zoompagetotalindex=0;
					ZoompageCurindex=0;	
					Epub_ZoomDataFree();
					Epub_Zoom_typesetting(pageCurindex,ZoomFlag);
					ZoompageCurindex=Zoompagetotalindex;
				}

				Epub_page_ZoomView(Zoompagetotalindex);
				Idu_ChgWin(Idu_GetNextWin());
				CopyNextWinTo(1);

				if ((Zoompagetotalindex)>0)
					Epub_page_ZoomView(Zoompagetotalindex-1);	
			
		}
	else
		TypesettingEvent2_ZoomPrePage();

}


void	EPUB_EreaderZoomPageDown(void)
{
	if(PageReadyStatus==ZoomNoReady)
		return;
	if (ZoomTypesettingStatus ==ZoomTypesettingduring)
		return;
		
	if (TypesettingStatus ==Typesettingduring)
		{
			if (pagetotalindex<(pageCurindex+1))
	  			return;
		}


	if(ZoomPageDirect==1)
		ZoomPageDirect=0;

	ZoompageCurindex++;

	if(ZoompageCurindex==Zoompagetotalindex)
		{
			Idu_ChgWin(Idu_GetNextWin());
			CopyNextWinTo(1);
			return;
		}

	if(ZoompageCurindex>Zoompagetotalindex)
		{
			if(pageCurindex>=pagetotalindex)
				{
					ZoompageCurindex--;
					return ;
				}
			else
				{
					ZoompageCurindex=0;
					Zoompagetotalindex=0;
					pageCurindex++;					
					Epub_ZoomDataFree();
					PageReadyStatus=ZoomNoReady;
					Epub_Zoom_typesetting(pageCurindex,ZoomFlag);

				}
		}
	else
		TypesettingEvent2_ZoomNextPage();

}

int  EPUB_CacheModeZoom(void)
{
	CacheWinInfo *C_WinInfo=(CacheWinInfo *)CacheMode_GetInfo();
	int xx=0,Tempindex=0;

	if (ZoomTypesettingStatus ==ZoomTypesettingduring)
	  	return;
	if(PageReadyStatus==ZoomNoReady)
		return;

	ZoomFlag++;
	mpDebugPrint("=====  AAA  EPUB_CacheModeZoom =%d pageCurindex=%d  pagetotalindex =%d ZoomFlag=%d ZoomPageDirect=%d PageDirect=%d === ",CacheDecodePageIndex,pageCurindex,pagetotalindex,ZoomFlag,ZoomPageDirect,PageDirect);
	 if(ZoomFlag==6)
		{
			int tempdecodeindex=0;

			Epub_ZoomDataFree();	
			Zoompagetotalindex=0;
			ZoompageCurindex=0;

			if((pageCurindex+CacheNumber-1)<pagetotalindex)
				{
					for(tempdecodeindex=pageCurindex;tempdecodeindex<pageCurindex+CacheNumber;tempdecodeindex++)
						{
							if((tempdecodeindex>=CacheDecodePageIndex)||(tempdecodeindex<(CacheDecodePageIndex-CacheNumber+1)))
								{
									Epub_reader_page_view(tempdecodeindex,1);
									C_WinInfo->Cachestatus[(tempdecodeindex%CacheNumber)]=CacheWinEnd;
								//	mpDebugPrint(" 111-1  tempdecodeindex =%d C_WinInfo->Cachestatus[tempdecodeindex%CacheNumber]=%d (CacheDecodePageIndex-CacheNumber+1)=%d",tempdecodeindex,C_WinInfo->Cachestatus[tempdecodeindex%CacheNumber],(CacheDecodePageIndex-CacheNumber+1));
								}
							//else
							//	mpDebugPrint(" 111-2  tempdecodeindex =%d C_WinInfo->Cachestatus[tempdecodeindex%CacheNumber]=%d (CacheDecodePageIndex-CacheNumber+1)=%d",tempdecodeindex,C_WinInfo->Cachestatus[tempdecodeindex%CacheNumber],(CacheDecodePageIndex-CacheNumber+1));

							if(tempdecodeindex==pageCurindex)
								Ereader_WinScale(C_WinInfo->EreaderCachewin[pageCurindex%CacheNumber],Idu_GetCurrWin());
						}

					if((pageCurindex+CacheNumber-1)==pagetotalindex)
						CacheDecodePageIndex=pagetotalindex;
					else
						CacheDecodePageIndex=tempdecodeindex;

					if(pageCurindex-1>=0)
						Epub_reader_page_view(pageCurindex-1,0);
					//mpDebugPrint(" 111 END tempdecodeindex=%d CacheDecodePageIndex=%d",tempdecodeindex,CacheDecodePageIndex);
				}
			else
				{
					for(xx=pagetotalindex-CacheNumber+1;xx<=pagetotalindex;xx++)
						{
							Tempindex=(xx)%CacheNumber;
							if((xx>=CacheDecodePageIndex))
								{
									C_WinInfo->Cachestatus[Tempindex]=0;
									//mpDebugPrint(" 22 xx=%d C_WinInfo->Cachestatus[Tempindex=%d]=%d",xx,Tempindex,C_WinInfo->Cachestatus[Tempindex]);
								}
							//else
								//mpDebugPrint(" 22--2222  xx=%d C_WinInfo->Cachestatus[Tempindex=%d]=%d",xx,Tempindex,C_WinInfo->Cachestatus[Tempindex]);
						}

					if(C_WinInfo->Cachestatus[(pageCurindex%CacheNumber)]!=CacheWinEnd)
						{
							Epub_reader_page_view(pageCurindex,1);
							C_WinInfo->Cachestatus[(pageCurindex%CacheNumber)]=CacheWinEnd;
						}

					Ereader_WinScale(C_WinInfo->EreaderCachewin[pageCurindex%CacheNumber],Idu_GetCurrWin());

					if(pagetotalindex-CacheNumber>0)
						Epub_reader_page_view(pagetotalindex-CacheNumber,0);

					if(CacheDecodePageIndex<pagetotalindex-CacheNumber+1)
						CacheDecodePageIndex=pagetotalindex-CacheNumber+1;


				}
			
			PageReadyStatus=Ready;
			ZoomFlag=0;
			TypesettingStatus=ZoomTypesettingEnd;	
			TypesettingEvent2_EPUBCheckCatchWin();
		}
	else	
		{
			if(ZoomPageDirect==1)
				ZoomPageDirect=0;

			Zoompagetotalindex=0;
			ZoompageCurindex=0;
			
			if(ZoomFlag==1)
				{	
					TypesettingStatus=CacheWinEnd;
					ZoomDisplayWinInit();
					Epub_Zoom_typesetting(pageCurindex,ZoomFlag);
				}
			else
				{
					Epub_ZoomDataFree();
					Epub_Zoom_typesetting(pageCurindex,ZoomFlag);
				}

		}

}

#if 0

int  EPUB_CacheModePageDown(void)
{
	CacheWinInfo *C_WinInfo=(CacheWinInfo *)CacheMode_GetInfo();
	int Cindex=0,tempindex=pageCurindex;
	
	pageCurindex++;
	mpDebugPrint(" START == >> CatchModePageDown   pageCurindex =%d pagetotalindex=%d CacheDecodePageIndex=%d TypesettingStatus=%d",pageCurindex,pagetotalindex,CacheDecodePageIndex,TypesettingStatus);

	if(pageCurindex>pagetotalindex)
	//if((pageCurindex>pagetotalindex)||((pageCurindex==pagetotalindex)&&(ExitTaskFlag1==0)))
		{
			pageCurindex--;
			return ;
		}

	Cindex=pageCurindex%CacheNumber;

	if(CacheNumber==1)
		C_WinInfo->Cachestatus[Cindex]=0;

	while(C_WinInfo->Cachestatus[Cindex]!=CacheWinEnd) 
		TaskYield();

	while(pageCurindex+CacheNumber-1>pagetotalindex)
		{
			TaskYield();
			if((TypesettingStatus==TypesettingChapterEnd)||(TypesettingStatus==ZoomTypesettingEnd))
				break;
		}

	if(tempindex+CacheNumber<=pagetotalindex)
		{
			Ereader_WinScaleCopy(Idu_GetCurrWin(),Idu_GetNextWin());
			if(CacheNumber!=1)
				C_WinInfo->Cachestatus[tempindex%CacheNumber]=0;		
		}

	Ereader_WinScaleCopy(C_WinInfo->EreaderCachewin[Cindex],Idu_GetCurrWin());

}

int  EPUB_CacheModePageUp(void)
{
	CacheWinInfo *C_WinInfo=(CacheWinInfo *)CacheMode_GetInfo();
	int Cindex=0,xx=0,temp=0;

	pageCurindex--;

	if(pageCurindex<0)
	{
		pageCurindex++;
		return ;
	}

	mpDebugPrint(" CatchModePageUp ==  pageCurindex =%d CacheDecodePageIndex=%d",pageCurindex,CacheDecodePageIndex);
	Cindex=pageCurindex%CacheNumber;

	if(pageCurindex+CacheNumber>pagetotalindex)
	{
		while(C_WinInfo->Cachestatus[Cindex]!=CacheWinEnd) 
			TaskYield();
	
		Ereader_WinScaleCopy(C_WinInfo->EreaderCachewin[Cindex],Idu_GetCurrWin());
	}
	else
	{
		Ereader_WinScaleCopy(Idu_GetNextWin(),Idu_GetCurrWin());

		if(C_WinInfo->Cachestatus[Cindex]!=0)
			{
				while(C_WinInfo->Cachestatus[Cindex]!=CacheWinEnd) 
					TaskYield();
			}

		C_WinInfo->Cachestatus[Cindex]=CacheWinEnd;

		if((CacheNumber!=1)||(pageCurindex+1!=pagetotalindex))
			CacheDecodePageIndex--;

		Ereader_WinScaleCopy(Idu_GetNextWin(),C_WinInfo->EreaderCachewin[Cindex]);

		for(xx=0;xx<CacheNumber;xx++)
		{
			temp=(Cindex+xx)%CacheNumber;
			while(C_WinInfo->Cachestatus[temp]!=CacheWinEnd)
				TaskYield();
		}

		if(pageCurindex-1>=0)
			Epub_reader_page_view(pageCurindex-1,0);
	}

}

#endif

#endif

#if Make_PDF	
////PDFXML
int xpgCb_PdfPagechange()
{
	int pdffiletype=3;
	int err=TRUE,status;

	if (TypesettingStatus ==Typesettingduring)
		return;

	word_xmltaginit();
	err = pdf2xml(FileName, Pdfcurpageindex, &TotalPage);

	if (ExitFlag==1)
		{
			ExitTaskFlag1=1;
			return 0;
		}
	if(err == FALSE)
		{
			status=word_xmlinit(pdffiletype);
			TypesettingStatus = EnterTypesetting;
			PdfPagechange_TypesettingInit(status);
		}

}

int Ppm2Xml()
{
	int err,status,tempindex;

	while(1)
		{
			if(TypesettingStatus!=Pdf2PpmDecoding)
				break;
			TaskYield();
		}
	TypesettingStatus=Typesettingduring;

	word_xmltaginit();

	if(PageDirect==0)
		tempindex=Pdfcurpageindex-1;
	else
		tempindex=Pdfcurpageindex+1;

	if(TotalPage==1)
		tempindex=1; //??
	err = pdf2xml(FileName, tempindex, &TotalPage);

	if(err == FALSE)
		{
			//xpgUpdateStage();
			//Ereader_WinScaleCopy(Idu_GetCurrWin(), Idu_GetNextWin());
			pagetotalindex=0;
			pageCurindex=0;
			status=word_xmlinit(3);
			PDF_Typesettinginit(status);
		}

	return 0;
}
int	PDF_EreaderPageDown(void)
{
if(PdfMode==1)
	{	
#if CacheMode_Enable
		PDF_CacheModePageDown();			
#else
		if((TotalPage==1)||(TypesettingStatus==Pdf2PpmDecoding))
			return 0;

		if((TotalPage==2)&&(TypesettingStatus==Pdf2PpmEnd))
			{
				if(Pdfcurpageindex==2)
					{
						Pdfcurpageindex++;
						Idu_ChgWin(Idu_GetNextWin());
					}

				return 0;
			}

		Pdfcurpageindex++;

		if(Pdfcurpageindex==TotalPage+1)
			{	
				Idu_ChgWin(Idu_GetNextWin());
				PDF_NextWin2Pdfwin1();
				return 0;
			}

		if(Pdfcurpageindex>TotalPage)
			{		
				Pdfcurpageindex--;
				return 0;
			}

		if(PageDirect==1)///aaa
			{
				 if(Pdfcurpageindex==1)
					{
						Idu_ChgWin(Idu_GetNextWin());
						PDF_NextWin2Pdfwin1();
						Pdfcurpageindex=Pdfcurpageindex+2;
						TypesettingEvent_PdfPage2winbuffer();
					}
				else  if(Pdfcurpageindex==TotalPage-1)
					{
						PDF_PdfWin1Nextwin();
						Idu_ChgWin(Idu_GetNextWin());
						Pdfcurpageindex=Pdfcurpageindex+2;
					}
				else
					{
						PDF_PdfWin1Nextwin();
						Idu_ChgWin(Idu_GetNextWin());
						PDF_NextWin2Pdfwin1();
						Pdfcurpageindex=Pdfcurpageindex+2;
						TypesettingEvent_PdfPage2winbuffer();
					}
				PageDirect=0;///aaaa
			}
		else
			PDF_OnePageModeNextPage(Pdfcurpageindex);
#endif
	}
else
	{
		if(PageReadyStatus==NoReady)
			return;
		
		if (TypesettingStatus ==Typesettingduring)
			{
				if (pagetotalindex<(pageCurindex+1))
	  				return;
			}

		if (pagetotalindex==0)
			if(Pdfcurpageindex==TotalPage)
				return;

		if(PageDirect==1)
			{
				Pdfcurpageindex=Pdfcurpageindex+2;
				PageDirect=0;
			}

		pageCurindex++;

		if(pageCurindex==pagetotalindex)
			{
				Idu_ChgWin(Idu_GetNextWin());
				CopyNextWinTo(1);
				return;
			}

		if(pageCurindex>pagetotalindex)
			{
				if(Pdfcurpageindex>TotalPage)
					{
						pageCurindex--;
						return 0;
					}
				else
					{
						PDF_DataCla();
						pagetotalindex=0;
						pageCurindex=0;
						xpgCb_PdfPagechange();
						Pdfcurpageindex++;
					}
			}
		else
			TypesettingEvent2_NextPage();
	}

	return 0;

}


int	PDF_EreaderPageUp(void)
{
	int index;

if(PdfMode==1)
	{
	
#if CacheMode_Enable
		PDF_CacheModePageUp();
#else
		if((TotalPage==1)||(TypesettingStatus==Pdf2PpmDecoding))
			{
				return 0;

			}
		if((TotalPage==2)&&(TypesettingStatus==Pdf2PpmEnd))
			{
				if(Pdfcurpageindex==3)
					{
						Pdfcurpageindex--;
						Idu_ChgWin(Idu_GetNextWin());
					}

				return 0;
			}

		Pdfcurpageindex--;
		
		if(Pdfcurpageindex==0)
			{
				Idu_ChgWin(Idu_GetNextWin());
				return 0;
			}

		if(Pdfcurpageindex<1)
			{
				Pdfcurpageindex++;
				return 0;
			}

		if(PageDirect==0)
			{
				if(Pdfcurpageindex==TotalPage)
					{
						Idu_ChgWin(Idu_GetNextWin());
						PDF_NextWin2Pdfwin1();

						if(TotalPage>2)
							{
								Pdfcurpageindex=Pdfcurpageindex-2;
								WaitPpm2WinEnd();
								TypesettingEvent_PdfPage2winbuffer();
							}
					}	
				else if(Pdfcurpageindex==1)
					{
						Pdfcurpageindex=0;
					}
				else if(Pdfcurpageindex==2)
					{
						PDF_PdfWin1Nextwin();
						Idu_ChgWin(Idu_GetNextWin());
						Pdfcurpageindex=0;
					}
				else
					{
						WaitPpm2WinEnd();
						PDF_PdfWin1Nextwin();
						Idu_ChgWin(Idu_GetNextWin());
						PDF_NextWin2Pdfwin1();
						Pdfcurpageindex=Pdfcurpageindex-2;
						TypesettingEvent_PdfPage2winbuffer();
					}

				PageDirect=1;///aaa
			}
		else
			PDF_OnePageModeNextPage(Pdfcurpageindex);
#endif
	}
	else
	{
		if(PageReadyStatus==NoReady)
			return;
		
		if (TypesettingStatus ==Typesettingduring)
			{	
				  if((pageCurindex-1)<0)
	  				return;
			}

		if (pagetotalindex==0)
			if(Pdfcurpageindex==1)
			return;

		pageCurindex--;

		if(pageCurindex==0)
		{

			if(PageDirect==0)
				{
					PageDirect==1;
					Win1andNextwinCopy(1);
					Idu_ChgWin(Idu_GetNextWin());
				}
			else
				{
					Idu_ChgWin(Idu_GetNextWin());
					CopyNextWinTo(1);
				}
			if(PageDirect==0)
				PageDirect==1;
			return;
		}

		if(PageDirect==0)
			{
				PageDirect=1;
				Pdfcurpageindex=Pdfcurpageindex-2;
			}

		if (pageCurindex<0)
			{
				if(Pdfcurpageindex<1)
					{
						pageCurindex++;
						return 0;
					}
				else
					{
						PDF_DataCla();
						pagetotalindex=0;
						pageCurindex=0;
						xpgCb_PdfPagechange();
						Pdfcurpageindex--;
						pageCurindex=pagetotalindex;
					}
			}

		TypesettingEvent2_PrePage();
	
	}
	
	return 0;

	//mpDebugPrint(" xpgCb_EreaderPageDown   pageCurindex =%d ",pageCurindex);
}

int	PDF_CacheModePageDown(void)
{
	CacheWinInfo *C_WinInfo=(CacheWinInfo *)CacheMode_GetInfo();
	int Cindex=0,tempindex=Pdfcurpageindex;
	
	Pdfcurpageindex++;

	if(Pdfcurpageindex>TotalPage)
		{
			Pdfcurpageindex--;
			return ;
		}

	Cindex=Pdfcurpageindex%CacheNumber;

	if(CacheNumber==1)
		C_WinInfo->Cachestatus[Cindex]=0;

	while(C_WinInfo->Cachestatus[Cindex]!=Pdf2PpmEnd) 
		TaskYield();

	if(Pdfcurpageindex+CacheNumber-1<=TotalPage)
		{
			Ereader_WinScaleCopy(Idu_GetCurrWin(),Idu_GetNextWin());
			if(CacheNumber>1)
				C_WinInfo->Cachestatus[tempindex%CacheNumber]=0;
		}
	if(ExitFlag!=1)
		Ereader_WinScaleCopy(C_WinInfo->EreaderCachewin[Cindex],Idu_GetCurrWin());

	mpDebugPrint(" == >> PDF_CatchModePageDown  END Pdfcurpageindex =%d",Pdfcurpageindex);
}

int	PDF_CacheModePageUp(void)
{
	CacheWinInfo *C_WinInfo=(CacheWinInfo *)CacheMode_GetInfo();
	int Cindex=0,xx=0,temp=0;

	Pdfcurpageindex--;

	if(Pdfcurpageindex<=0)
	{
		Pdfcurpageindex++;
		return ;
	}

	Cindex=Pdfcurpageindex%CacheNumber;

	if(Pdfcurpageindex+CacheNumber>TotalPage)
	{
		while(C_WinInfo->Cachestatus[Cindex]!=Pdf2PpmEnd) 
			TaskYield();
		Ereader_WinScaleCopy(C_WinInfo->EreaderCachewin[Cindex],Idu_GetCurrWin());
	}
	else
	{
		Ereader_WinScaleCopy(Idu_GetNextWin(),Idu_GetCurrWin());

		if(C_WinInfo->Cachestatus[Cindex]!=0)
			{
				while(C_WinInfo->Cachestatus[Cindex]!=Pdf2PpmEnd) 
					TaskYield();
			}
		
		C_WinInfo->Cachestatus[Cindex]=Pdf2PpmEnd;
		CacheDecodePageIndex--;		
		Ereader_WinScaleCopy(Idu_GetNextWin(),C_WinInfo->EreaderCachewin[Cindex]);

		for(xx=0;xx<CacheNumber;xx++)
		{
			temp=(Cindex+xx)%CacheNumber;
			while(C_WinInfo->Cachestatus[temp]!=Pdf2PpmEnd)
				TaskYield();
		}

		if(Pdfcurpageindex-1>0)
			PDF_Page2WinBuffer(Pdfcurpageindex-1);
	}

	mpDebugPrint(" PDF_CatchModePageUp ====END  Pdfcurpageindex =%d CacheDecodePageIndex=%d",Pdfcurpageindex,CacheDecodePageIndex);

}

int	PDF_CacheModeZoom(void)
{
	CacheWinInfo *C_WinInfo=(CacheWinInfo *)CacheMode_GetInfo();
	int xx=0,Tempindex=0;
			
	if (TypesettingStatus ==Typesettingduring)
	  	return;

	ZoomFlag++;
	mpDebugPrint("=PDF_CacheModeZoom=   ZoomFlag=%d  CacheDecodePageIndex= %d   ==",ZoomFlag,CacheDecodePageIndex);

	if(ZoomFlag==1)
		{
			Tempindex=(CacheDecodePageIndex)%CacheNumber;

			while(C_WinInfo->Cachestatus[Tempindex]!=Pdf2PpmEnd)
				TaskYield();			
		}
			
	if(PageDirect==1)
		{
			Pdfcurpageindex+=2;
			PageDirect=0;
		}

	if(ZoomFlag==6)
		{
			DisplayWinFree();
			Pdfcurpageindex--;
			PdfMode=1;
			ExitTaskFlag1=0;

			if((Pdfcurpageindex+CacheNumber)<=TotalPage)
				{
					for(xx=0;xx<CacheNumber;xx++)
						{
							Tempindex=(Pdfcurpageindex+xx)%CacheNumber;
							if(((Pdfcurpageindex+xx)<=CacheDecodePageIndex)&&((Pdfcurpageindex+xx)>CacheDecodePageIndex-CacheNumber))
								C_WinInfo->Cachestatus[Tempindex]=Pdf2PpmEnd;
							else
								C_WinInfo->Cachestatus[Tempindex]=0;
						}

					if(C_WinInfo->Cachestatus[(Pdfcurpageindex)%CacheNumber]!=Pdf2PpmEnd)
						{
							CacheMode_PDF2CacheWinBuffer(Pdfcurpageindex);
							C_WinInfo->Cachestatus[(Pdfcurpageindex)%CacheNumber]=Pdf2PpmEnd;
						}

					Ereader_WinScale(C_WinInfo->EreaderCachewin[Pdfcurpageindex%CacheNumber],Idu_GetCurrWin());

					if(Pdfcurpageindex-1>0)
						PDF_Page2WinBuffer(Pdfcurpageindex-1);
					//if((CacheDecodePageIndex<Pdfcurpageindex)||(CacheDecodePageIndex-CacheNumber>Pdfcurpageindex+CacheNumber))
					CacheDecodePageIndex=Pdfcurpageindex;

				}
			else
				{
					for(xx=TotalPage-CacheNumber+1;xx<=TotalPage;xx++)
						{
							Tempindex=(xx)%CacheNumber;
							if((xx<=CacheDecodePageIndex))
								C_WinInfo->Cachestatus[Tempindex]=Pdf2PpmEnd;
							else
								C_WinInfo->Cachestatus[Tempindex]=0;
							mpDebugPrint(" 22 xx=%d C_WinInfo->Cachestatus[Tempindex=%d]=%d",xx,Tempindex,C_WinInfo->Cachestatus[Tempindex]);
						}

					if(C_WinInfo->Cachestatus[(Pdfcurpageindex)%CacheNumber]!=Pdf2PpmEnd)
						{
							CacheMode_PDF2CacheWinBuffer(Pdfcurpageindex);
							C_WinInfo->Cachestatus[(Pdfcurpageindex)%CacheNumber]=Pdf2PpmEnd;
						}

					Ereader_WinScale(C_WinInfo->EreaderCachewin[Pdfcurpageindex%CacheNumber],Idu_GetCurrWin());
							
					if(TotalPage-CacheNumber>0)
						PDF_Page2WinBuffer(TotalPage-CacheNumber);
							if(CacheDecodePageIndex<TotalPage-CacheNumber)
								CacheDecodePageIndex=TotalPage-CacheNumber+1;

				}
			ZoomFlag=0;
			TypesettingEvent_PdfCheckCatchWin();
		}
	else
		{
			if(ZoomFlag==1)
				{
					ExitTaskFlag1=1; 
					PdfMode=0;
					Pdfcurpageindex++;
					DisplayWinInit();
					PDF_DataInit ();
					Ppm2Xml();
				}
			else
				Ppm2Xml();
		}

}

void PDFCacheModeAutoSlideshow()
{
	mpDebugPrint("#### PDFAutoSlideshow ====>START ");
	while(Pdfcurpageindex<=TotalPage)
		{
			TaskYield();

			while((TypesettingStatus==Pdf2PpmDecoding))
				TaskYield();

			if(ExitFlag==1)
				break;

			xpgCb_EreaderPageDown();

			if((ExitFlag==1)||(EreaderSlideshow==0))
				break;
			if(Pdfcurpageindex==TotalPage)
				EreaderSlideshow=0;

		}
	EreaderSlideshow=0;

}

#endif //Make_PDF

#endif // READERDISPLAY_ENABLE

#endif //EREADER_ENABLE

