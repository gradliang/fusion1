/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "fs.h"
#include "IspFunc.h"
#include "ui_timer.h"
#include "Setup.h"
#include "slideEffect.h"
#include "isp.h"
#include "mpapi.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif


#define SETUP_INIT_BIT          0x00000001


//DWORD gSetupMenuValue[SETTING_BUFFER_SIZE >> 2];

//extern BOOL bSetUpChg;


int ISP_UpDateCode(STREAM * handle)
{
#if ISP_FUNC_ENABLE
    int ret = PASS;
    BYTE info[64];
    DWORD rdsz = 0;
    DWORD hdr[8], chksum = 0;
    SDWORD filesz = FileSizeGet(handle);
    DWORD *buffer = NULL;
    DWORD *header = (DWORD *) ((DWORD) hdr | BIT29);
    DWORD total = filesz, perc = 0;

    mpPrintMessage(__FUNCTION__);

    while (filesz > 4)
    {
        rdsz = FileRead(handle, (BYTE *) header, 4);
        filesz -= rdsz;

        if (rdsz == 4 && (header[0] == AP_TAG || header[0] == RES_TAG))
        {
            filesz -= FileRead(handle, (BYTE *) &header[1], 28);

            if (filesz < header[2])
            {
                filesz = 0;	// end of
                mpPrintMessage("ISP file is not correctly");
                ret = FAIL;
            }
            else
            {
                DWORD len = (header[2] + 0x3) & (~0x3);
                MP_ALERT("tag=0x%X, size=%d, chksum=0x%X", header[0], header[2], header[4]);

                if (buffer)
                    ext_mem_free(buffer);

                buffer = (DWORD *) ext_mem_malloc(len + 32);

                if (buffer)
                {
                    DWORD i, nr = header[2] >> 2;
                    DWORD *ptr;

                    buffer = (DWORD *) (BIT29 | (DWORD) buffer);

                    ptr = &buffer[8];
                    memcpy((BYTE *) buffer, (BYTE *) header, 32);
                    chksum = header[4];
                    rdsz = FileRead(handle, (BYTE *) &buffer[8], len);

                    if (rdsz != len)
                    {
                        filesz = 0;     // end of
                        mpPrintMessage("FileRead failed!");
                        ret = FAIL;
                        break;
                    }
                    else
                    {
                        filesz -= rdsz;

                        // checksum verify
                        for (i = 0 ; i < nr ; i++)
                            chksum += ptr[i];

                        if (chksum)
                        {
                            sprintf(info, "%c%c%c%c checksum error!", header[0]>>24, header[0]>>16, header[0]>>8, header[0]);
                            mpPrintMessage(info);
                            ret = FAIL;
                            break;
                        }
                        else
                        {
                            sprintf(info, "%c%c%c%c found", header[0]>>24, header[0]>>16, header[0]>>8, header[0]);
                            mpPrintMessage(info);
                            // checksum is good, start ISP
                            ret = IspFunc_Write((BYTE *)buffer, len+32, header[0]);
								if (header[0] == AP_TAG)
								{
	                    			chksum = header[4];
									header[4]++;
									IspFunc_ReadAP((BYTE *) header, 32);
									DWORD dwNewSum=header[4];
									if (chksum!=dwNewSum)
									{
	            						MP_ALERT("--E-- ISP writer chksum error !! %p->%p",chksum,dwNewSum);
		                            ret = FAIL;
		                            break;
									}
								}
                            mpPrintMessage("\t->done!!");
                        }
                    }

                    ext_mem_free(buffer);
                    buffer = NULL;
                }
                else
                {
                    sprintf(info, "Out of memory!(need:%d)", len);
                    mpPrintMessage(info);
                }
            }
        }
        else
        {
            MP_ALERT("--E-- %s error !!",__FUNCTION__);

            if (rdsz != 4)
                MP_ALERT("--E-- File read error !!!");
            else if ((header[0] != AP_TAG) && (header[0] == RES_TAG))
                MP_ALERT("--E-- Wrong header tag 0x%08X !!!", header[0]);
        }
    }

    if (buffer)
        ext_mem_free(buffer);

    return ret;
#else

    return FAIL;

#endif
}       //end of ISP_UpDateCode



DWORD ISP_GetResourceSize(DWORD dwTag)
{
#if ISP_FUNC_ENABLE
    return IspFunc_GetRESOURCESize(dwTag);
#else
    return FAIL;
#endif
}       //end of ISP_GetResourceSize



BYTE* ISP_GetResource(DWORD dwTag, BYTE *pbTarget, DWORD dwSize)
{
#if ISP_FUNC_ENABLE
    return IspFunc_ReadRESOURCE(dwTag, pbTarget, dwSize);
#else
    return FAIL;
#endif
}       //end of ISP_GetResource


/*
int ISP_ChkWriteBackSetup(void)
{
#if ISP_FUNC_ENABLE

    WORD wIdleTime = GetIdleFlag();

    if (!ChkSetupChg())
    {
        SetIdleFlag(0);

        return PASS;
    }

    if (g_bAniFlag != 0)
    {
        AddTimerProc(100, ISP_ChkWriteBackSetup);

        return PASS;
    }

    if (wIdleTime < 4)
    {
        SetIdleFlag ((wIdleTime + 1));
        AddTimerProc(10, ISP_ChkWriteBackSetup);

        return PASS;
    }

    bSetUpChg = 0;
    SetIdleFlag(0);

    return (ISP_WriteBackSetup());
#else
    return FAIL;
#endif
}




static void ISP_GenerateSettingTable(DWORD *pdwTempMen)
{
    *(pdwTempMen + 7) = (DWORD) SETUP_INIT_BIT;     //Jasmine 4/26 CH: save setup init bit
    *(pdwTempMen + 8)  = (DWORD) g_bVolumeIndex;    //VOL_CHG_0317
    *(pdwTempMen + 9)  = (DWORD) g_psSetupMenu->bPhotoTransEffect;
    *(pdwTempMen + 10) = (DWORD) g_psSetupMenu->bPhotoFullScreen;
    *(pdwTempMen + 11) = (DWORD) g_psSetupMenu->bMusicRptState;
    *(pdwTempMen + 12) = (((DWORD) g_psSetupMenu->bMovieRptState) << 16) |
                          g_psSetupMenu->bVideoForwardSecond;
    *(pdwTempMen + 13) = (DWORD) g_psSetupMenu->bSlideInterval;
    *(pdwTempMen + 14) = (DWORD) g_psSetupMenu->bSlideTransition;
    *(pdwTempMen + 15) = (DWORD) g_psSetupMenu->bInfoFlag;
    *(pdwTempMen + 16) = (DWORD) g_psSetupMenu->bSlideShuffle;
    *(pdwTempMen + 17) = (DWORD) g_psSetupMenu->bSlideSeparate;
    *(pdwTempMen + 18) = (DWORD) g_psSetupMenu->bPhotoCrop;
    *(pdwTempMen + 19) = (DWORD) g_psSetupMenu->bContrast; //bGamma;
    *(pdwTempMen + 20) = (DWORD) g_psSetupMenu->bBrightness;
    *(pdwTempMen + 22) = (DWORD) g_psSetupMenu->bSaturation; //bColor;
    *(pdwTempMen + 21) = (DWORD) g_psSetupMenu->bHue; //bTint;
    *(pdwTempMen + 23) = (DWORD) GetCurFileSortingBasis();

    {
        COLOR_MANAGEMENT *colorManagement = GetColorManagement();
        DWORD color_Red     = (colorManagement[0].Hue << 16) | (colorManagement[0].Saturation);
        DWORD color_Skin    = (colorManagement[1].Hue << 16) | (colorManagement[1].Saturation);
        DWORD color_Yellow  = (colorManagement[2].Hue << 16) | (colorManagement[2].Saturation);
        DWORD color_Green   = (colorManagement[3].Hue << 16) | (colorManagement[3].Saturation);
        DWORD color_Cyan    = (colorManagement[4].Hue << 16) | (colorManagement[4].Saturation);
        DWORD color_Blue    = (colorManagement[5].Hue << 16) | (colorManagement[5].Saturation);
        DWORD color_Magenta = (colorManagement[6].Hue << 16) | (colorManagement[6].Saturation);

        *(pdwTempMen + 24) = (DWORD) color_Red;
        *(pdwTempMen + 25) = (DWORD) color_Skin;
        *(pdwTempMen + 26) = (DWORD) color_Yellow;
        *(pdwTempMen + 27) = (DWORD) color_Green;
        *(pdwTempMen + 28) = (DWORD) color_Cyan;
        *(pdwTempMen + 29) = (DWORD) color_Blue;
        *(pdwTempMen + 30) = (DWORD) color_Magenta;
    }
}


int ISP_ReadSetup(void)
{
#if ISP_FUNC_ENABLE
    DWORD *pdwTempMen;
    #if 1
        pdwTempMen = gSetupMenuValue;
    #else
        DWORD pTempMen[SETTING_BUFFER_SIZE >> 2];
        pdwTempMen = (DWORD *) ((DWORD) pTempMen | BIT29);
    #endif

    MP_DEBUG(" IspFunc_ReadUST() ");
    IspFunc_ReadUST((BYTE *) pdwTempMen, SETTING_BUFFER_SIZE);

    if (*pdwTempMen == USER_SET_TAG)
    {
        #if 1
            Recover_g_psSetupMenu();
        #else
            ISP_GetSysParamFromST(pdwTempMen);
        #endif
    }
    else    // user setting was not found
    {
        MP_ALERT("No user setting found, rebuild it");
        ISP_WriteBackSetup();
    }

    return PASS;

#else
    return FAIL;
#endif

}



int ISP_WriteBackSetup(void)
{
#if ISP_FUNC_ENABLE
    int iRet = FAIL;

    #if 1
        Update_gSetupMenuValue();
        iRet = IspFunc_WriteUST((BYTE *)gSetupMenuValue, SETTING_BUFFER_SIZE);
    #else
        DWORD SetUpBuffer[SETTING_BUFFER_SIZE >> 2];
        DWORD *pdwTempMen = (DWORD *) ((DWORD) SetUpBuffer | BIT29);
        ISP_GenerateSettingTable(pdwTempMen);
        // fill header
        *pdwTempMen = USER_SET_TAG;
        iRet = IspFunc_WriteUST((BYTE *)pdwTempMen, SETTING_BUFFER_SIZE);
    #endif
    MP_DEBUG("ISP_WriteBackSetup");
    bSetUpChg = 0;
    SetIdleFlag(0);

    return iRet;
#else
    return FAIL;
#endif

}



void ISP_GetSysParamFromST(DWORD *pdwTempMen)    // get system parameters from setting table
{
    DWORD VideoSettings = pdwTempMen[12];

    if ((pdwTempMen[7] & SETUP_INIT_BIT) == 0)
        return; //Jasmine 4/26 CH: if init bit not equal to 1, then use default value only

    if (pdwTempMen[8] <= VOLUME_DEGREE)
        g_bVolumeIndex = (BYTE) pdwTempMen[8];

    if (pdwTempMen[9] <= SETUP_MENU_OFF)
        g_psSetupMenu->bPhotoTransEffect = (BYTE) pdwTempMen[9];

    if (pdwTempMen[10] <= SETUP_MENU_OFF)
        g_psSetupMenu->bPhotoFullScreen = (BYTE) pdwTempMen[10];

    if (pdwTempMen[11] <= SETUP_MENU_MUSIC_REPEAT_OFF)
        g_psSetupMenu->bMusicRptState = (BYTE) pdwTempMen[11];

    //if (pdwTempMen[12] <= SETUP_MENU_MOVIE_REPEAT_OFF)
    //    psSetupMenu->bMovieRptState = (BYTE) pdwTempMen[12];
    if ((VideoSettings >> 16) <= SETUP_MENU_MUSIC_REPEAT_OFF)
        g_psSetupMenu->bMovieRptState = VideoSettings >> 16;

    if ((VideoSettings & 0x0007) <= SETUP_MENU_VIDEO_FORWARD_SECOND_32)
        g_psSetupMenu->bVideoForwardSecond = VideoSettings & 0x0007; // <=4

    if (pdwTempMen[13] <= SETUP_MENU_SLIDE_SHOW_INTERVAL_SLOW)  //Mason 20060619
        g_psSetupMenu->bSlideInterval = (BYTE) pdwTempMen[13];

    if (pdwTempMen[14] <= SETUP_MENU_TRANSITION_RANDOM)
        g_psSetupMenu->bSlideTransition = (BYTE) pdwTempMen[14];

    if (pdwTempMen[15] <= SETUP_MENU_OFF)
        g_psSetupMenu->bInfoFlag = (BOOL) pdwTempMen[15];

    if (pdwTempMen[16] <= SETUP_MENU_OFF)
        g_psSetupMenu->bSlideShuffle = (BYTE) pdwTempMen[16];

    if (pdwTempMen[17] <= SETUP_MENU_OFF)
        g_psSetupMenu->bSlideSeparate = (BOOL) pdwTempMen[17];

    if (pdwTempMen[18] <= SETUP_MENU_OFF)
        g_psSetupMenu->bPhotoCrop = (BOOL) pdwTempMen[18];

    if (pdwTempMen[19] <= 0xf)
        g_psSetupMenu->bContrast = (BYTE) pdwTempMen[19]; // bGamma

    if (pdwTempMen[20] <= 0xf)
        g_psSetupMenu->bBrightness = (BYTE) pdwTempMen[20];

    if (pdwTempMen[21] <= 0xf)
        g_psSetupMenu->bHue = (BYTE) pdwTempMen[21]; // bTint

    if (pdwTempMen[22] <= 0xf)
        g_psSetupMenu->bSaturation = (BYTE) pdwTempMen[22]; // bColor

    if (pdwTempMen[23] < FILE_SORTING_MAX)
        g_psSetupMenu->bFileSortBasis = (BYTE) pdwTempMen[23];

    SetCurFileSortingBasis((FILE_SORTING_BASIS_TYPE) pdwTempMen[23]);
    SetupMenuSyncToSysSetup();

    {
        DWORD color_Red     = pdwTempMen[24];
        DWORD color_Skin    = pdwTempMen[25];
        DWORD color_Yellow  = pdwTempMen[26];
        DWORD color_Green   = pdwTempMen[27];
        DWORD color_Cyan    = pdwTempMen[28];
        DWORD color_Blue    = pdwTempMen[29];
        DWORD color_Magenta = pdwTempMen[30];

        COLOR_MANAGEMENT *colorManagement = GetColorManagement();

        colorManagement[0].Hue        = color_Red >> 16;
        colorManagement[0].Saturation = color_Red & 0x00ff;
        colorManagement[1].Hue        = color_Skin >> 16;
        colorManagement[1].Saturation = color_Skin & 0x00ff;
        colorManagement[2].Hue        = color_Yellow >> 16;
        colorManagement[2].Saturation = color_Yellow & 0x00ff;
        colorManagement[3].Hue        = color_Green >> 16;
        colorManagement[3].Saturation = color_Green & 0x00ff;
        colorManagement[4].Hue        = color_Cyan >> 16;
        colorManagement[4].Saturation = color_Cyan & 0x00ff;
        colorManagement[5].Hue        = color_Blue >> 16;
        colorManagement[5].Saturation = color_Blue & 0x00ff;
        colorManagement[6].Hue        = color_Magenta >> 16;
        colorManagement[6].Saturation = color_Magenta & 0x00ff;

        Setup_ColorManagementAdjust();
    }
}
*/





SWORD ISP_ChksumVerify(void)
{
#if ISP_FUNC_ENABLE
    SWORD ret = PASS;
    DWORD *tmp;
    DWORD *ptr;
    DWORD i, len, chksum;

    tmp = (DWORD *) ext_mem_malloc(32);  // 32 bytes for temporary header buffer

    if (!tmp)
        return FAIL;

    tmp = (DWORD *) (BIT29 | (DWORD) tmp);

    IspFunc_ReadAP((BYTE *) tmp, 32);
    len = tmp[2];
    chksum = tmp[4];
    ptr = (DWORD *) ext_mem_malloc(len + 32);
        mpDebugPrint("ISP_ChksumVerify len=%d chksum=%p", len,chksum);

    if (!ptr)
    {
        ext_mem_free(tmp);

        return FAIL;
    }

    ptr = (DWORD *) (BIT29 | (DWORD) ptr);
    IspFunc_ReadAP((BYTE *) ptr, len + 32);

    for (i = 0 ; i < len>>2 ; i++)
        chksum += ptr[8+i];

    if (chksum)
    {
        mpDebugPrint("MPAP checksum failed!(%x)", chksum);
#if 0
        SystemDeviceInit(SD_MMC);
        DriveAdd(SD_MMC);
        DriveChange(SD_MMC);
        DirReset(DriveGet(SD_MMC));

        if (CreateFile(DriveGet(SD_MMC), "AP", "BIN"))
            mpDebugPrint("AP.BIN creation failed!");
        else
        {
            STREAM *fptr = FileOpen(DriveGet(SD_MMC));
            FileWrite(fptr, ptr, len+32);
            FileClose(fptr);
        }
#endif
        ret = FAIL;
    }
    else
    {
        mpDebugPrint("MPAP checksum ok!");
    }

    ext_mem_free(ptr);

    IspFunc_ReadRES(tmp, 32);
    len = tmp[2];
    chksum = tmp[4];
    ptr = (DWORD *)((DWORD)ext_mem_malloc(len+32) | 0xA0000000);
    IspFunc_ReadRES(ptr, len + 32);
    for (i = 0 ; i < len>>2 ; i++)
    chksum += ptr[8+i];

    if (chksum)
    {
        mpDebugPrint("MPRS checksum failed!(%x)", chksum);
        ret = FAIL;
    }
    else
    {
        mpDebugPrint("MPRS checksum ok!");
    }

    ext_mem_free(tmp);
    ext_mem_free(ptr);

    return ret;
#endif
}

