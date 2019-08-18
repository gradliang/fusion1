/*
*******************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2004, Magic Pixel Inc, Hsinchu, Taiwan
*
* All rights reserved. Magic Pixel's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in
* deciphering, decoding, reverse engineering or in ay way altering the source
* code is strictly prohibited, unless the prior written consent of Magic
* Pixel is obtained.
*
* Filename      : fileapp.c
* Programmers   : 
* Created       :
* Descriptions  :
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

/*
// Include section
*/
#include "mpTrace.h"
#include "fileapp.h"

/*
*******************************************************************************
*        LOCAL DATA TYPES
*******************************************************************************
*/

BYTE * ClearDirList[] = {
    "/",
    "VIDEO",
    "AUDIO",
};

/*
*******************************************************************************
*        LOCAL VARIABLE
*******************************************************************************
*/


/*
*******************************************************************************
*        LOCAL FUNCTION PROTOTYPES
*******************************************************************************
*/
static int DeleteBadFile(DRIVE * drv);
static int BadFileChainFree(DRIVE * drv, CHAIN * chain);

/*
*******************************************************************************
*        GLOBAL FUNCTIONS
*******************************************************************************
*/

#if AUTO_CLEAR_UP_FILE_SYSTEM
static int BadFileChainFree(DRIVE * drv, CHAIN * chain)
{
	DWORD size, clusterSize, current, next;

	if (drv == NULL)
	{
		MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
		return INVALID_DRIVE;
	}

	if (chain == NULL)
	{
		MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}

	if (drv->FreeClusters == 0)  /* maybe FAT table of this drive has not been scanned yet */
		DriveFreeClustersCountGet(drv); /* force to do ScanFat() for FAT12/16/32 drive */

  #if (ENABLE_ALLOC_BITMAP_FOR_FAT121632)
	if (! drv->Flag.AllocBitmapReady)
	{
		MP_ALERT("%s: Error! AllocBitmap table still not ready !", __FUNCTION__);
		return ABNORMAL_STATUS;
	}
  #endif

	size = chain->Size;
	clusterSize = 1 << (drv->ClusterExp + drv->bSectorExp);
	current = chain->Start;

	while ((current >= 2) && (current < (drv->TotalClusters + 2)))
	{
		if (! SystemCardPresentCheck(drv->DrvIndex))
		{
			MP_ALERT("%s: Card not present !", __FUNCTION__);
			return DRIVE_ACCESS_FAIL;
		}

        //mpDebugPrint("free %d", current);

		next = drv->FatRead(drv, current);

		if (drv->StatusCode == FS_SCAN_FAIL)
		{
			MP_ALERT("%s: drv->FatRead() failed !", __FUNCTION__);
			return DRIVE_ACCESS_FAIL;
		}

		drv->FatWrite(drv, current, 0);
		current = next;
		drv->FreeClusters++;
		if (size < clusterSize) /* this should be impossible, except that FAT table is corrupt */
		{
			chain->Size = 0;
			//break;
		}
		else
			size = size - clusterSize;

		if (current == FAT_READ_END_OF_CHAIN) /* reach EOC (end of chain) */
			break;
	}

	return FS_SUCCEED;
}


static int DeleteBadFile(DRIVE * drv)
{
    volatile FDB *node;
    LONG_NAME *lnode;
    CHAIN chain;
    DWORD counter;
    int ret;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return INVALID_DRIVE;
    }

    node = (volatile FDB *) (drv->Node);
    ChainInit(&chain, (LoadAlien16((void *) (&node->StartHigh)) << 16) + LoadAlien16((void *) &node->StartLow), LoadAlien32((void *) &node->Size));

    if (BadFileChainFree(drv, &chain) != FS_SUCCEED)
    {
        MP_ALERT("%s: ChainFree() failed !", __FUNCTION__);
        return ABNORMAL_STATUS;
    }

    node->Name[0] = 0xe5;
    drv->Flag.DirCacheChanged = 1;

    counter = 0;
    while (1)
    {
        counter++;

        ret = PreviousNode(drv);
        if (ret == ABNORMAL_STATUS)
        {
            return ABNORMAL_STATUS;
        }
        else if (ret == BEGIN_OF_DIR)
        {
            counter--;
            break;
        }

        node = (FDB *) (drv->Node);
        lnode = (LONG_NAME *) (drv->Node);

        if (lnode->Attribute1 == 0x0f)
            node->Name[0] = 0xe5;
        else
            break;

        drv->Flag.DirCacheChanged = 1;
    }

    while (counter)
    {
        if (NextNode(drv) != FS_SUCCEED)
        {
            return ABNORMAL_STATUS;
        }
        counter--;
    }

    

    if (DriveRefresh(drv) != FS_SUCCEED)
        return ABNORMAL_STATUS;
    return FS_SUCCEED;
}


void AutoClearUpFileSystem(BYTE drvid)
{
    DRIVE * drv;
    int ret;
    int dircnt;

    mpDebugPrint("%s()", __FUNCTION__);
    
    if (!SystemCardPresentCheck(drvid))
        return;
    
    drv = DriveGet(drvid);
    if (drv == NULL) {
        mpDebugPrint("--E--  driver is NULL.");
        return -1;
    }

    for (dircnt = 0; dircnt < sizeof(ClearDirList)/sizeof(BYTE*); dircnt++)
    {
        const BYTE * pbDirName;
        ret = DirReset(drv);
        if (ret != FS_SUCCEED) {
            mpDebugPrint("--E--  DirReset() Fail.");
            return -1;
        }
        ret = DirFirst(drv);
        if (ret != FS_SUCCEED) {
            mpDebugPrint("--E--  DirFirst() Fail.");
            return -1;
        }
        
        pbDirName = ClearDirList[dircnt];
        if (strcmp(pbDirName, "/"))         // if not ROOT folder
        {
            const BYTE * SubDirName;
            if (pbDirName[0] == '/')
                SubDirName = &pbDirName[1];
            else
                SubDirName = pbDirName;

            ret = FileSearch(drv, SubDirName, "", E_DIR_TYPE);
            if (ret != FS_SUCCEED) {
                mpDebugPrint("--E--  Cannot find \"%s\" folder, EXIT.", SubDirName);
                continue;
            }
            
            ret = CdSub(drv);
            if (ret != FS_SUCCEED) {
                mpDebugPrint("--E--  Cannot enter \"%s\" folder, EXIT.", SubDirName);
                continue;
            }
            
            ret = DirFirst(drv);
            if (ret != FS_SUCCEED) {
                mpDebugPrint("--E-- \"%s\" DirFirst() Fail.", SubDirName);
                continue;
            }

        }
        mpDebugPrint("clear bad files in folder \"%s\"", ClearDirList[dircnt]);
        
        ///////////////////////////////////////////////////////
        while (1)
        {
            BYTE * pExt;
            BOOL boExtMatch;
            
            pExt = drv->Node->Extension;
            boExtMatch = FALSE;
            
            if (pExt[0] == 'A' && pExt[1] == 'V' && pExt[2] == 'I')
                boExtMatch = TRUE;
            if (pExt[0] == 'W' && pExt[1] == 'A' && pExt[2] == 'V')
                boExtMatch = TRUE;
            if (pExt[0] == 'T' && pExt[1] == ' ' && pExt[2] == ' ')
                boExtMatch = TRUE;
            if (pExt[0] == 'T' && pExt[1] == 'M' && pExt[2] == 'P')
                boExtMatch = TRUE;

            if (boExtMatch && drv->Node->Size == 0)
            {
                ///////////////////   get full name
                int j;
                BYTE bBaseName[12];
                BYTE bExtName[4];
                BYTE bFullName[16];
                strncpy(bBaseName, drv->Node->Name, 8);
                bBaseName[8] = 0;
                strncpy(bExtName, drv->Node->Extension, 3);
                bExtName[3] = 0;
                for (j = 0; j < 8; j++)
                    if (bBaseName[j] == ' ')
                        bBaseName[j] = 0;
                for (j = 0; j < 3; j++)
                    if (bExtName[j] == ' ')
                        bExtName[j] = 0;
                sprintf(bFullName, "%s.%s", bBaseName, bExtName);
                ///////////////////     delete bad file
                DeleteBadFile(drv);
                ///////////////////
                mpDebugPrint("delete bad file \"%s\" finish.", bFullName);
                //////////////////
            }
L_next_node:
            if (DirNext(drv) != FS_SUCCEED)
            {
                //MP_DEBUG("%s: file not found.\n", __FUNCTION__);
                break;
            }
        }
    }

    DirReset(drv);
    
    mpDebugPrint("%s() finish.", __FUNCTION__);
}
#endif

