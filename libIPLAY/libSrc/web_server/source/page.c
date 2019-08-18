/*
 * Page.c -- Support for page retrieval.
 *
 * Copyright (c) GoAhead Software Inc., 1995-2000. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 * $Id: page.c,v 1.3 2002/10/24 14:44:50 bporter Exp $
 */

/******************************** Description *********************************/

/*
 *	This module provides page retrieval handling. It provides support for
 *	reading web pages from file systems and has expansion for ROMed web
 *	pages.
 */

/********************************* Includes ***********************************/
#include	"wsIntrn.h"
#include	"net_sys.h"


/*********************************** Code *************************************/
/*
 *	Open a web page. lpath is the local filename. path is the URL path name.
 */
 
static STREAM *web_shandle = NULL;
static DWORD web_shandle_idx;
int websPageOpen(webs_t wp, char_t *lpath, char_t *path, int mode, int perm)
{
    BYTE *nowpage = "/nowplaying.jpg";
	
	//mpDebugPrint("websPageOpen %s",lpath);
	a_assert(websValid(wp));

#ifdef WEBS_PAGE_ROM
	DRIVE *w_sDrv;
	int ret,i,j,cp=0,out = 0;
	BYTE name[16];
	BYTE extname[8];
	
	if(strncmp(lpath,nowpage,strlen(nowpage)) == 0)
	{
		NowPlaying_ReadUnlock();
		NowPlaying_GetReadFileName(name, sizeof name, extname);
		NowPlaying_ReadLock();
	}
	else
	{
	   if(strlen(lpath)>13)//"/"+file name + "." +extname= 13
	   {
	      //mpDebugPrint("%s %d > 13",lpath,strlen(lpath));
	   	  goto ROM;
	   }
	   
	   memset(name,0x00,16);
	   memset(extname,0x00,8);
	   
	   for(i=0,j=0;i<strlen(lpath);i++)
	   {
	     if(lpath[i]=='/')
	     {   
	        i++;
		 	cp = 1;
	     }
		 
		 switch(cp)
		 {
		     case 1:
			 	if(lpath[i]=='.')
			 	{
			 	  cp=2;
				  j=0;
				  break;
			 	}
			 	name[j]=lpath[i];
			 	j++;
				if(j>8)
					out = 1;
					
			 	break;
			 case 2:
			 	extname[j] = lpath[i];
				j++;
			 	break;
			 	
		 }
	     
	   }
       //mpDebugPrint("name %s",name);
       //mpDebugPrint("extname %s",extname);
	   
	}
		
	
	w_sDrv=DriveGet(SD_MMC);//SD_MMC//NAND
	
    if((FS_SUCCEED == FileSearch(w_sDrv, name, extname, E_FILE_TYPE)) && (!out))
    {
        if(web_shandle!=NULL)
			FileClose(web_shandle);
		web_shandle = FileOpen(w_sDrv);
		wp->docfd = 1;
#if DM9KS_ETHERNET_ENABLE
		DriveChange(CF_ETHERNET_DEVICE);
#else
		DriveChange(USB_WIFI_DEVICE);
#endif

		return wp->docfd;
    }
	else	
	{
#if DM9KS_ETHERNET_ENABLE
		DriveChange(CF_ETHERNET_DEVICE);
#else
		DriveChange(USB_WIFI_DEVICE);
#endif

ROM:		
		return websRomPageOpen(wp, path, mode, perm);
	}
#else
	return (wp->docfd = gopen(lpath, mode, perm));
#endif /* WEBS_PAGE_ROM */
}

/******************************************************************************/
/*
 *	Close a web page
 */

void websPageClose(webs_t wp)
{
    //BYTE *nowpage = "//nowplaying.jpg";

	a_assert(websValid(wp));

#ifdef WEBS_PAGE_ROM
	DRIVE *w_sDrv;

	//if(!strncmp(wp->lpath,nowpage,strlen(nowpage)))
		
	if(web_shandle!= NULL)
	{
	   NowPlaying_ReadUnlock();
	   NowPlaying_ReleaseReadFileName();
	   w_sDrv=DriveGet(SD_MMC);//SD_MMC//NAND
	   if (FileClose(web_shandle) != FS_SUCCEED)
	     mpDebugPrint("FileClose %s FALSE!!",web_shandle);
	   else
	   	{
	   	   //mpDebugPrint("FileClose %x",web_shandle);
	  	   web_shandle = NULL;
	   	}
#if DM9KS_ETHERNET_ENABLE
		DriveChange(CF_ETHERNET_DEVICE);
#else
		DriveChange(USB_WIFI_DEVICE);
#endif

	}
	else
		websRomPageClose(wp->docfd);
#else
	if (wp->docfd >= 0) {
		close(wp->docfd);
		wp->docfd = -1;
	}
#endif
}

/******************************************************************************/
/*
 *	Stat a web page lpath is the local filename. path is the URL path name.
 */

int websPageStat(webs_t wp, char_t *lpath, char_t *path, websStatType* sbuf)
{
#ifdef WEBS_PAGE_ROM
    //BYTE *nowpage = "/nowplaying.jpg";
	DRIVE *w_sDrv;

    //if(!strncmp(lpath,nowpage,strlen(nowpage)))
		
	if(web_shandle!=NULL)
    {
	    w_sDrv=DriveGet(SD_MMC);//SD_MMC//NAND
        sbuf->size = FileSizeGet(web_shandle);
		//mpDebugPrint("sbuf->size %d",sbuf->size);
		DriveChange(USB_WIFI_DEVICE);
		return 0;
    }
    else
	return websRomPageStat(path, sbuf);
#else
	gstat_t	s;

	if (gstat(lpath, &s) < 0) {
		return -1;
	}
	sbuf->size = s.st_size;
	sbuf->mtime = s.st_mtime;
	sbuf->isDir = s.st_mode & S_IFDIR;
	return 0;
#endif
}

/******************************************************************************/
/*
 *	Is this file a directory?
 */

int websPageIsDirectory(char_t *lpath)
{
#ifdef WEBS_PAGE_ROM
	websStatType	sbuf;

	if (websRomPageStat(lpath, &sbuf) >= 0) {
		return(sbuf.isDir);
	} else {
		return 0;
	}
#else
	gstat_t sbuf;

	if (gstat(lpath, &sbuf) >= 0) {
		return(sbuf.st_mode & S_IFDIR);
	} else {
		return 0;
	}
#endif
}


/******************************************************************************/
/*
 *	Read a web page. Returns the number of _bytes_ read.
 *	len is the size of buf, in bytes.
 */

int websPageReadData(webs_t wp, char *buf, int nBytes)
{
    //BYTE *nowpage = "//nowplaying.jpg";
	int len;

#ifdef WEBS_PAGE_ROM
	DRIVE *w_sDrv;

	a_assert(websValid(wp));

	//if(!strncmp(wp->lpath,nowpage,strlen(nowpage)))
	if(web_shandle!=NULL)
	{
	    //mpDebugPrint("websPageReadData %s",wp->lpath);
		w_sDrv=DriveGet(SD_MMC);//SD_MMC//NAND
		len = FileRead(web_shandle,buf, nBytes);
		web_shandle_idx = FilePosGet(web_shandle);
		DriveChange(USB_WIFI_DEVICE);
		return len;
	}
	else	
		return websRomPageReadData(wp, buf, nBytes);
#else
	a_assert(websValid(wp));
	return read(wp->docfd, buf, nBytes);
#endif
}

/******************************************************************************/
/*
 *	Move file pointer offset bytes.
 */

void websPageSeek(webs_t wp, long offset)
{
    //BYTE *nowpage = "//nowplaying.jpg";
	//mpDebugPrint("websPageSeek");
	a_assert(websValid(wp));

#ifdef WEBS_PAGE_ROM
	DRIVE *w_sDrv;

	//if(!strncmp(wp->lpath,nowpage,strlen(nowpage)))
		
	if(web_shandle!=NULL)
	{
		w_sDrv=DriveGet(SD_MMC);//SD_MMC//NAND
		//if (FS_SUCCEED != Seek(web_shandle, web_shandle_idx))
		if (FS_SUCCEED != Seek(web_shandle, offset))
		{
			mpDebugPrint("Seek FALSE!!\n");
		}
		
		DriveChange(USB_WIFI_DEVICE);
	}	
    else
		websRomPageSeek(wp, offset, SEEK_CUR);
#else
	lseek(wp->docfd, offset, SEEK_CUR);
#endif
}

/******************************************************************************/


