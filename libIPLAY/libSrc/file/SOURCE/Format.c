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
* Filename      : Format.c
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
#include "global612.h"
#include "mptrace.h"
#include "exFAT.h"
#include "fat.h"
#include "file.h"
#include "chain.h"
#include "dir.h"
#include "index.h"
#include "devio.h"
#include "os.h"
#include "taskid.h"


#define BPB_BytsPerSec               Mcard_GetSectorSize(drv->DevID)
#define BPB_SecPerTrk                63
#define BPB_NumHeads                 255
#define BPB_NumFATs                  2

#define BPB_RsvdSecCnt_Fat32         32   /* for FAT32, BPB_RsvdSecCnt is typically 32 */
#define BPB_RsvdSecCnt_Fat1216       1    /* for FAT12 and FAT16, BPB_RsvdSecCnt should never be anything other than 1 */

#define BPB_RootEntCnt_Fat32         0    /* for FAT32, BPB_RootEntCnt must be 0 */
#define BPB_RootEntCnt_Fat1216       512  /* for FAT12 and FAT16, BPB_RootEntCnt should use the value 512 */

#define START_CLUSTER                2
#define FIRST_SECTOR_OF_CLUSTER(Nr)  ((Nr - START_CLUSTER) * BPB_SecPerClus + FirstDataSector)


static DWORD FatEntriesCnt, DataSectorsCnt;
static DWORD FirstDataSector, RootDirSectors;
static BYTE BPB_SecPerClus;
static DWORD BPB_FATSz32, BPB_FATSz16;
static DWORD HiddSecCnt_BeforeThePartition; /* count of sectors before boot sector of the partition */



static void SectorToCHS(DWORD sector_nr, BYTE * ret)
{
    BYTE sector, head;
    WORD cylinder;


    if (ret == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return;
    }

    sector = (sector_nr % BPB_SecPerTrk) + 1;
    head = sector_nr / BPB_SecPerTrk;
    cylinder = head / BPB_NumHeads;

    ret[0] = head;
    ret[1] = ((cylinder >> 2) & 0xC0) | sector;
    ret[2] = cylinder & 0xff;
}



static void SFNCopy(BYTE * dst, BYTE * src, DWORD seqno)  // short file name copy
{
#define TO_UPPER(x)    ((((x) >= 'a') && ((x) <= 'z'))? ((x) - 'a' + 'A'):(x))
    DWORD cpl;
    BYTE *ptr = dst;
    BYTE *ext;
    int i;


    if ((src == NULL) || (dst == NULL))
        return;
    else if ((src[0] == 0) || (src[0] == ' '))
        return;

    MpMemSet(dst, ' ', 11);

    if ((src[0] == '.') && (src[1] == 0))
        dst[0] = '.';
    else if ((src[0] == '.') && (src[1] == '.') && (src[2] == 0))
        dst[0] = dst[1] = '.';
    else
    {
        cpl = StringLength08(src);

        for (i = 0; i < 8; i++)
        {
            if ((*src == '.') || (i >= cpl))
                break;
            ptr[i] = TO_UPPER(*src);
            src++;
        }

        if (i < cpl)
        {
            ext = src;
            for (i = StringLength08(ext); i >= 0; i--)
            {
                if (ext[i] == '.')
                {
                    ext = &ext[i+1];
                    break;
                }
            }
            for (i = 8; i < 11; i++)
            {
                ptr[i] = TO_UPPER(*ext);
                ext++;
            }
        }

        i = 7;
        ptr = dst;
        while (seqno)
        {
            ptr[i] = '0' + (seqno % 10);
            i--;
            seqno /= 10;
            if (seqno == 0)
                ptr[i] = '~';
        }
    }
}



static void InitSectorSignature_55AA(BYTE * buf)
{
    if (buf == NULL)
    {
        MP_ALERT("%s: Error! NULL buffer pointer !", __FUNCTION__);
        return;
    }

    MpMemSet(buf, 0, 512);
    buf[SIGNATURE_OFFSET] = 0x55;
    buf[SIGNATURE_OFFSET + 1] = 0xAA;
}



static SWORD BuildInitMBR(BYTE * sector_buf, DWORD HiddSec_cnt, DWORD partition1_SectorNr, DWORD partition2_SectorNr, DWORD partition3_SectorNr, DWORD partition4_SectorNr)
{
    PARTITION partition[4] = { {0, 0, 0, 0, 0x0b, 0, 0, 0, 0x00000000, 0x00000000},  //initialize 1st partition, default partition type = 0x0b (FAT32)
                               {0, 0, 0, 0, 0x00, 0, 0, 0, 0x00000000, 0x00000000},  //initialize 2nd partition, default partition type = 0x00
                               {0, 0, 0, 0, 0x00, 0, 0, 0, 0x00000000, 0x00000000},  //initialize 3rd partition, default partition type = 0x00
                               {0, 0, 0, 0, 0x00, 0, 0, 0, 0x00000000, 0x00000000},  //initialize 4th partition, default partition type = 0x00
                             };


    if (sector_buf == NULL)
    {
        MP_ALERT("%s: Error! NULL buffer pointer !", __FUNCTION__);
        return FAIL;
    }

    if (partition1_SectorNr == 0)
    {
        MP_ALERT("%s: Error! Size of first partition is zero !", __FUNCTION__);
        return FAIL;
    }

    if (partition2_SectorNr > 0)
        partition[1].Type = 0x0b; //default partition type = 0x0b (FAT32)
    else  // (partition2_SectorNr == 0)
    {
        if ((partition3_SectorNr > 0) || (partition4_SectorNr > 0))
        {
            MP_ALERT("%s: Error! Size of each partition is unreasonable !", __FUNCTION__);
            return FAIL;
        }
    }

    if (partition3_SectorNr > 0)
        partition[2].Type = 0x0b; //default partition type = 0x0b (FAT32)
    else  // (partition3_SectorNr == 0)
    {
        if (partition4_SectorNr > 0)
        {
            MP_ALERT("%s: Error! Size of each partition is unreasonable !", __FUNCTION__);
            return FAIL;
        }
    }

    if (partition4_SectorNr > 0)
        partition[3].Type = 0x0b; //default partition type = 0x0b (FAT32)

    /* prepare MBR sector buffer content */
    InitSectorSignature_55AA(sector_buf);

    /* 1st partition info */
    SectorToCHS(HiddSec_cnt, (BYTE *)&(partition[0].res1));  //lagacy CHS address of first block in partition
    SectorToCHS(HiddSec_cnt + partition1_SectorNr - 1, (BYTE *)&(partition[0].res4));  //lagacy CHS address of last block in partition
    partition[0].Start = byte_swap_of_dword(HiddSec_cnt);  //LBA of the first sector in this partition
    partition[0].Size = byte_swap_of_dword(partition1_SectorNr);  //length (number of blocks) of this partion

    /* 2nd partition info */
    if (partition2_SectorNr > 0)
    {
        SectorToCHS(HiddSec_cnt + partition1_SectorNr, (BYTE *)&(partition[1].res1));  //lagacy CHS address of first block in partition
        SectorToCHS(HiddSec_cnt + partition1_SectorNr + partition2_SectorNr - 1, (BYTE *)&(partition[1].res4));  //lagacy CHS address of last block in partition
        partition[1].Start = byte_swap_of_dword(HiddSec_cnt + partition1_SectorNr);  //LBA of the first sector in this partition
        partition[1].Size = byte_swap_of_dword(partition2_SectorNr);  //length (number of blocks) of this partion
    }

    /* 3rd partition info */
    if (partition3_SectorNr > 0)
    {
        SectorToCHS(HiddSec_cnt + partition1_SectorNr + partition2_SectorNr, (BYTE *)&(partition[2].res1));  //lagacy CHS address of first block in partition
        SectorToCHS(HiddSec_cnt + partition1_SectorNr + partition2_SectorNr + partition3_SectorNr - 1, (BYTE *)&(partition[2].res4));  //lagacy CHS address of last block in partition
        partition[2].Start = byte_swap_of_dword(HiddSec_cnt + partition1_SectorNr + partition2_SectorNr);  //LBA of the first sector in this partition
        partition[2].Size = byte_swap_of_dword(partition3_SectorNr);  //length (number of blocks) of this partion
    }

    /* 4th partition info */
    if (partition4_SectorNr > 0)
    {
        SectorToCHS(HiddSec_cnt + partition1_SectorNr + partition2_SectorNr + partition3_SectorNr, (BYTE *)&(partition[3].res1));  //lagacy CHS address of first block in partition
        SectorToCHS(HiddSec_cnt + partition1_SectorNr + partition2_SectorNr + partition3_SectorNr + partition4_SectorNr - 1, (BYTE *)&(partition[3].res4));  //lagacy CHS address of last block in partition
        partition[3].Start = byte_swap_of_dword(HiddSec_cnt + partition1_SectorNr + partition2_SectorNr + partition3_SectorNr);  //LBA of the first sector in this partition
        partition[3].Size = byte_swap_of_dword(partition4_SectorNr);  //length (number of blocks) of this partion
    }

    MpMemCopy(&sector_buf[PARTITION_OFFSET], (BYTE *) partition, sizeof(PARTITION) * 4);
    return PASS;
}



static SWORD UpdatePartitionTypeinMBR(DRIVE * drv, FS_TYPE fs_type)
{
    PARTITION *partition;
    BYTE *sector_buf;
    BYTE partition_idx, partition_type;
    DWORD partition_start_lba, partition_SectorNr, partition_blockSize;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return FAIL;
    }

    if (GetDrvPartitionInfoFromMBR(drv->DrvIndex, &partition_idx, &partition_type, &partition_start_lba, &partition_SectorNr, &partition_blockSize) != PASS)
    {
        MP_ALERT("%s: Error! GetDrvPartitionInfoFromMBR() failed !", __FUNCTION__);
        return FAIL;
    }

    sector_buf = (BYTE *) ext_mem_malloc(BPB_BytsPerSec);
    if (sector_buf == NULL)
    {
        MP_ALERT("%s: malloc fail !!", __FUNCTION__);
        return FAIL;
    }
    sector_buf = (BYTE *) ((DWORD) sector_buf | BIT29);

    /* read MBR */
    Mcard_DeviceRead(drv, sector_buf, 0, 1);

    if (*(WORD *) (sector_buf + SIGNATURE_OFFSET) != COMMON_SIGNATURE)  /* check MBR sector signature (2 bytes: offset 510, 511) */
    {
        MP_ALERT("%s: -I- (MBR) Signature 0x55AA not found.", __FUNCTION__);
        ext_mem_free(sector_buf);
        return FAIL;
    }

    partition = (PARTITION *) (sector_buf + PARTITION_OFFSET);

    /* update partition type of this partition only */
    if (fs_type == FS_TYPE_FAT32)
        partition[partition_idx].Type = 0x0b;
    else if (fs_type == FS_TYPE_FAT16)
    {
        if (((partition_SectorNr * 512) >> 20) > 32) //FAT16 with size over 32MB
            partition[partition_idx].Type = 0x06;
        else //FAT16 with size less than 32MB
            partition[partition_idx].Type = 0x04;
    }
    else if (fs_type == FS_TYPE_FAT12)
        partition[partition_idx].Type = 0x01;
#if EXFAT_ENABLE
    else if (fs_type == FS_TYPE_exFAT)
    {
  #if EXFAT_WRITE_ENABLE
        MP_ALERT("%s: To-Do: how to process exFAT Write operations ??", __FUNCTION__);
        return FAIL; //not supported yet
  #else
        MP_ALERT("%s: -I- exFAT Write operations are not supported !", __FUNCTION__);
        return FAIL;
  #endif
    }
#endif

    MpMemCopy(&sector_buf[PARTITION_OFFSET], (BYTE *) partition, sizeof(PARTITION) * 4);
    Mcard_DeviceWrite(drv, sector_buf, 0, 1); /* write back MBR to update partition table */
    ext_mem_free(sector_buf);

    return PASS;
}



static SWORD BuildPartitionBPB(DRIVE * drv, BYTE * sector_buf, DWORD partition_start_lba, DWORD total_SectorNr, FS_TYPE fs_type)
{
    DWORD tmp;
    ST_SYSTEM_TIME  sys_time;
    WORD file_date, file_time;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return FAIL;
    }

    if (sector_buf == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return FAIL;
    }

    FatEntriesCnt = 0;
    FirstDataSector = 0;
    RootDirSectors = 0;
    BPB_SecPerClus = 64; /* here, initial big value. Final value adopted for BPB_SecPerClus will be tried while building BPB info */
    BPB_FATSz32 = 0, BPB_FATSz16 = 0;
    HiddSecCnt_BeforeThePartition = partition_start_lba;

    InitSectorSignature_55AA(sector_buf);
    BPB_SecPerClus <<= 1; // initial value for later do-while loops to choose suitable BPB_SecPerClus value

    /* The following calculations are according to FAT32 Spec page 21 */
    if (fs_type == FS_TYPE_FAT32)
    {
        RootDirSectors = 0;
        do
        {
            BPB_SecPerClus >>= 1;
            if (BPB_SecPerClus == 0)
            {
                MP_ALERT("%s: Cluster number cannot meet FAT32 spec ! => FAT32 formatting aborted !", __FUNCTION__);
                return FAIL;
            }

            if ((BPB_SecPerClus * BPB_BytsPerSec) > (32 * 1024))
                continue;

            tmp = (256 * BPB_SecPerClus + BPB_NumFATs)/ 2;
            BPB_FATSz32 = ((total_SectorNr - (BPB_RsvdSecCnt_Fat32 + RootDirSectors)) + (tmp - 1))/ tmp;
            DataSectorsCnt = total_SectorNr - (BPB_RsvdSecCnt_Fat32 + (BPB_NumFATs * BPB_FATSz32) + RootDirSectors);
        } while ((DataSectorsCnt / BPB_SecPerClus) < 65525);
    }
    else if (fs_type == FS_TYPE_FAT16)
    {
        RootDirSectors = ((BPB_RootEntCnt_Fat1216 * 32) + (BPB_BytsPerSec - 1))/ BPB_BytsPerSec;
        do
        {
            BPB_SecPerClus >>= 1;
            if (BPB_SecPerClus == 0)
            {
                MP_ALERT("%s: Cluster number cannot meet FAT16 spec ! => FAT16 formatting aborted !", __FUNCTION__);
                return FAIL;
            }

            if ((BPB_SecPerClus * BPB_BytsPerSec) > (32 * 1024))
                continue;

            tmp = (256 * BPB_SecPerClus) + BPB_NumFATs;
            BPB_FATSz16 = ((total_SectorNr - (BPB_RsvdSecCnt_Fat1216 + RootDirSectors)) + (tmp - 1))/ tmp;
            DataSectorsCnt = total_SectorNr - (BPB_RsvdSecCnt_Fat1216 + (BPB_NumFATs * BPB_FATSz16) + RootDirSectors);
        } while (((DataSectorsCnt / BPB_SecPerClus) < 4085) || ((DataSectorsCnt / BPB_SecPerClus) >= 65525));
    }
    else if (fs_type == FS_TYPE_FAT12)
    {
        RootDirSectors = ((BPB_RootEntCnt_Fat1216 * 32) + (BPB_BytsPerSec - 1))/ BPB_BytsPerSec;
        BPB_SecPerClus = 0; // initial value for later do-while loops to choose suitable BPB_SecPerClus value
        do
        {
            if (BPB_SecPerClus == 0)
                BPB_SecPerClus = 1;
            else
                BPB_SecPerClus <<= 1;

            if ((BPB_SecPerClus * BPB_BytsPerSec) > (32 * 1024))
            {
                MP_ALERT("%s: Cluster number cannot meet FAT12 spec ! => FAT12 formatting aborted !", __FUNCTION__);
                return FAIL;
            }

            tmp = (256 * BPB_SecPerClus) + BPB_NumFATs;
            BPB_FATSz16 = ((total_SectorNr - (BPB_RsvdSecCnt_Fat1216 + RootDirSectors)) + (tmp - 1))/ tmp;
            DataSectorsCnt = total_SectorNr - (BPB_RsvdSecCnt_Fat1216 + (BPB_NumFATs * BPB_FATSz16) + RootDirSectors);
        } while ((DataSectorsCnt / BPB_SecPerClus) >= 4085);
    }
#if EXFAT_ENABLE
    else if (fs_type == FS_TYPE_exFAT)
    {
  #if EXFAT_WRITE_ENABLE
        MP_ALERT("%s: To-Do: how to process exFAT Write operations ??", __FUNCTION__);
        return FAIL; //not supported yet
  #else
        MP_ALERT("%s: -I- exFAT Write operations are not supported !", __FUNCTION__);
        return FAIL;
  #endif
    }
#endif

    sector_buf[0] = 0xEB;  // BS_jmpBoot
    sector_buf[1] = 0x58;
    sector_buf[2] = 0x90;
    MpMemCopy(&sector_buf[3], "MSDOS5.0", 8);  // BS_OEMName
    sector_buf[11] = BPB_BytsPerSec & 0xff; // BPB_BytsPerSec
    sector_buf[12] = BPB_BytsPerSec >> 8;
    sector_buf[13] = BPB_SecPerClus;  // BPB_SecPerClus

    if (fs_type == FS_TYPE_FAT32)
    {
        sector_buf[14] = BPB_RsvdSecCnt_Fat32 & 0xff;  // BPB_RsvdSecCnt
        sector_buf[15] = (BPB_RsvdSecCnt_Fat32 >> 8) & 0xff;
    }
    else if ((fs_type == FS_TYPE_FAT12) || (fs_type == FS_TYPE_FAT16))
    {
        sector_buf[14] = 0x01;  // BPB_RsvdSecCnt
        sector_buf[15] = 0x00;
    }

    sector_buf[16] = BPB_NumFATs;  // BPB_NumFATs

    if (fs_type == FS_TYPE_FAT32)
    {
        sector_buf[17] = BPB_RootEntCnt_Fat32 & 0xff;  // BPB_RootEntCnt
        sector_buf[18] = (BPB_RootEntCnt_Fat32 >> 8) & 0xff;
    }
    else if ((fs_type == FS_TYPE_FAT12) || (fs_type == FS_TYPE_FAT16))
    {
        sector_buf[17] = BPB_RootEntCnt_Fat1216 & 0xff;  // BPB_RootEntCnt
        sector_buf[18] = (BPB_RootEntCnt_Fat1216 >> 8) & 0xff;
    }

    sector_buf[19] = 0x00;  // BPB_TotSec16
    sector_buf[20] = 0x00;
    sector_buf[21] = 0xF8;  // BPB_Media

    if (fs_type == FS_TYPE_FAT32)
    {
        sector_buf[22] = 0x00;  // BPB_FATSz16
        sector_buf[23] = 0x00;
    }
    else if ((fs_type == FS_TYPE_FAT12) || (fs_type == FS_TYPE_FAT16))
    {
        sector_buf[22] = BPB_FATSz16 & 0xff;  // BPB_FATSz16
        sector_buf[23] = (BPB_FATSz16 >> 8) & 0xff;
    }

    sector_buf[24] = BPB_SecPerTrk & 0xff;  // BPB_SecPerTrk
    sector_buf[25] = (BPB_SecPerTrk >> 8) & 0xff;
    sector_buf[26] = BPB_NumHeads & 0xff;   // BPB_NumHeads
    sector_buf[27] = (BPB_NumHeads >> 8) & 0xff;
    sector_buf[28] = HiddSecCnt_BeforeThePartition & 0xff;    // BPB_HiddSec
    sector_buf[29] = (HiddSecCnt_BeforeThePartition >> 8) & 0xff;
    sector_buf[30] = (HiddSecCnt_BeforeThePartition >> 16) & 0xff;
    sector_buf[31] = (HiddSecCnt_BeforeThePartition >> 24) & 0xff;
    sector_buf[32] = total_SectorNr & 0xff;        // BPB_TotSec32
    sector_buf[33] = (total_SectorNr >> 8) & 0xff;
    sector_buf[34] = (total_SectorNr >> 16) & 0xff;
    sector_buf[35] = (total_SectorNr >> 24) & 0xff;

    if (fs_type == FS_TYPE_FAT32)
    {
        sector_buf[36] = BPB_FATSz32 & 0xff;  // BPB_FATSz32
        sector_buf[37] = (BPB_FATSz32 >> 8) & 0xff;
        sector_buf[38] = (BPB_FATSz32 >> 16) & 0xff;
        sector_buf[39] = (BPB_FATSz32 >> 24) & 0xff;

        FatEntriesCnt = BPB_FATSz32 * BPB_BytsPerSec / 4;
        FirstDataSector = HiddSecCnt_BeforeThePartition + BPB_RsvdSecCnt_Fat32 + (BPB_NumFATs * BPB_FATSz32) + RootDirSectors;

        sector_buf[44] = START_CLUSTER;  // BPB_RootClus
        sector_buf[48] = 1;              // BPB_FSInfo
        sector_buf[50] = 6;              // BPB_BkBootSec
        sector_buf[66] = 0x29;           // BPB_BootSig

        SystemTimeGet(&sys_time);
        file_date = FileSetDate_for_FATxx(sys_time.u16Year, sys_time.u08Month, sys_time.u08Day);
        file_time = FileSetTime_for_FATxx(sys_time.u08Hour, sys_time.u08Minute, sys_time.u08Second);
        tmp = ((DWORD) file_time) | (((DWORD) file_date) << 16);
        tmp = byte_swap_of_dword(tmp);
        MpMemCopy(&sector_buf[67], &tmp, 4);             // BS_VolID

        strncpy(&sector_buf[71], "NO NAME    ", 11);  // BS_VolLab
        strncpy(&sector_buf[82], "FAT32   ", 8);      // BS_FilSysType
    }
    else if ((fs_type == FS_TYPE_FAT16) || (fs_type == FS_TYPE_FAT12))
    {
        if (fs_type == FS_TYPE_FAT16)
            FatEntriesCnt = BPB_FATSz16 * BPB_BytsPerSec / 2;
        else if (fs_type == FS_TYPE_FAT12)
            FatEntriesCnt = (BPB_FATSz16 * BPB_BytsPerSec) * 2 / 3;

        FirstDataSector = HiddSecCnt_BeforeThePartition + BPB_RsvdSecCnt_Fat1216 + (BPB_NumFATs * BPB_FATSz16) + RootDirSectors;

        sector_buf[38] = 0x29;  // BPB_BootSig

        SystemTimeGet(&sys_time);
        file_date = FileSetDate_for_FATxx(sys_time.u16Year, sys_time.u08Month, sys_time.u08Day);
        file_time = FileSetTime_for_FATxx(sys_time.u08Hour, sys_time.u08Minute, sys_time.u08Second);
        tmp = ((DWORD) file_time) | (((DWORD) file_date) << 16);
        tmp = byte_swap_of_dword(tmp);
        MpMemCopy(&sector_buf[39], &tmp, 4);             // BS_VolID

        strncpy(&sector_buf[43], "NO NAME    ", 11);  // BS_VolLab
        if (fs_type == FS_TYPE_FAT16)
            strncpy(&sector_buf[54], "FAT16   ", 8);  // BS_FilSysType
        else
            strncpy(&sector_buf[54], "FAT12   ", 8);
    }

    return PASS;
}



static void BuildPartitionFSInfo(BYTE * sector_buf, DWORD free_nr, DWORD next)
{
    if (sector_buf == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return;
    }

    InitSectorSignature_55AA(sector_buf);
    *((DWORD *)&sector_buf[0]) = byte_swap_of_dword(0x41615252);    // FSI_LeadSig
    *((DWORD *)&sector_buf[484]) = byte_swap_of_dword(0x61417272);  // FSI_StruSig
    *((DWORD *)&sector_buf[488]) = byte_swap_of_dword(free_nr);     // FSI_Free_Count
    *((DWORD *)&sector_buf[492]) = byte_swap_of_dword(next);        // FSI_Nxt_Free
}



static void SetFAT32Entry(DRIVE * drv, BYTE * buffer, DWORD Nr, DWORD cluster)
{
    DWORD fatOffset, lba, idx;
    DWORD *fat; //DWORD type (32-bit) for FAT32 entries


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return;
    }

    if (buffer == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return;
    }

    fatOffset = ((HiddSecCnt_BeforeThePartition + BPB_RsvdSecCnt_Fat32) * BPB_BytsPerSec);
    lba = (fatOffset + (Nr << 2)) / BPB_BytsPerSec;
    idx = Nr % (BPB_BytsPerSec >> 2);
    fat = (DWORD *) buffer; //DWORD type (32-bit) for FAT32 entries

    Mcard_DeviceRead(drv, buffer, lba, 1);
    fat[idx] = byte_swap_of_dword((fat[idx] & 0xF0000000) | (cluster & 0x0FFFFFFF));
    Mcard_DeviceWrite(drv, buffer, lba, 1);
    if (BPB_NumFATs > 1)
        Mcard_DeviceWrite(drv, buffer, lba + BPB_FATSz32, 1);
}



static void SetFAT16Entry(DRIVE * drv, BYTE * buffer, DWORD Nr, DWORD cluster)
{
    DWORD fatOffset, lba, idx;
    WORD *fat; //WORD type (16-bit) for FAT16 entries


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return;
    }

    if (buffer == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return;
    }

    fatOffset = ((HiddSecCnt_BeforeThePartition + BPB_RsvdSecCnt_Fat1216) * BPB_BytsPerSec);
    lba = (fatOffset + (Nr << 1)) / BPB_BytsPerSec;
    idx = Nr % (BPB_BytsPerSec >> 1);
    fat = (WORD *) buffer; //WORD type (16-bit) for FAT16 entries

    Mcard_DeviceRead(drv, buffer, lba, 1);
    fat[idx] = byte_swap_of_word(cluster & 0xFFFF);
    Mcard_DeviceWrite(drv, buffer, lba, 1);
    if (BPB_NumFATs > 1)
        Mcard_DeviceWrite(drv, buffer, lba + BPB_FATSz16, 1);
}



static void SetFAT12Entry(DRIVE * drv, BYTE * buffer, DWORD Nr, DWORD cluster)
{
    WORD  mask, byte_offset;
    DWORD fatOffset, lba, sector;
    BYTE *point;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return;
    }

    if (buffer == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer !", __FUNCTION__);
        return;
    }

    byte_offset = Nr + (Nr >> 1); // byte_offset = cluster_No. * 1.5
    fatOffset = ((HiddSecCnt_BeforeThePartition + BPB_RsvdSecCnt_Fat1216) * BPB_BytsPerSec);
    lba = (fatOffset + byte_offset) / BPB_BytsPerSec;

    sector = byte_offset / BPB_BytsPerSec;

    byte_offset = byte_offset & (BPB_BytsPerSec - 1); // limit the byte offset within a sector size
    cluster = cluster & 0xfff; //content (FAT entry value): 0 ~ 0xFFF in FAT12

    Mcard_DeviceRead(drv, buffer, lba, 1);
    point = (BYTE *) ((BYTE *)buffer + byte_offset);

    // if cluster_No. is odd then left shift a nibble
    mask = 0xf000;
    if (Nr & 1)
    {
        cluster <<= 4;
        mask = 0x000f;
    }
    *point = (*point & mask) | cluster;

    point++;
    byte_offset++;
    mask >>= 8;
    cluster >>= 8;
    if (byte_offset == BPB_BytsPerSec)
    {
        Mcard_DeviceWrite(drv, buffer, lba, 1);
        if (BPB_NumFATs > 1)
            Mcard_DeviceWrite(drv, buffer, lba + BPB_FATSz16, 1);

        lba++;
        Mcard_DeviceRead(drv, buffer, lba, 1);
        point = (BYTE *) buffer;
    }
    *point = (*point & mask) | cluster;

    Mcard_DeviceWrite(drv, buffer, lba, 1);
    if (BPB_NumFATs > 1)
        Mcard_DeviceWrite(drv, buffer, lba + BPB_FATSz16, 1);
}



static SWORD BuildFatDirRoot(DRIVE * drv, BYTE * name, FS_TYPE fs_type)
{
    SWORD ret = PASS;
    BYTE *sector_buf;
    FDB  *fdb;
    DWORD written_sector_count, i;
	

    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return FAIL;
    }

    sector_buf = (BYTE *) ext_mem_malloc(BPB_BytsPerSec);
    if (sector_buf == NULL)
    {
        MP_ALERT("%s: malloc fail !!", __FUNCTION__);
        return FAIL;
    }

    if ((name != NULL) && (name[0] != 0)) /* volume label specified */
    {
        ST_SYSTEM_TIME  sys_time;
        WORD file_date, file_time;

        MpMemSet(sector_buf, 0, BPB_BytsPerSec);

        fdb = (FDB *) sector_buf;
        fdb->Attribute = FDB_LABEL;

        // fill in the create date/time and modified date
        SystemTimeGet(&sys_time);
        file_date = FileSetDate_for_FATxx(sys_time.u16Year, sys_time.u08Month, sys_time.u08Day);
        file_time = FileSetTime_for_FATxx(sys_time.u08Hour, sys_time.u08Minute, sys_time.u08Second);
        fdb->CreateTime = file_time;
        fdb->CreateDate = file_date;
        fdb->AccessDate = file_date;
        fdb->ModifyTime = file_time;
        fdb->ModifyDate = file_date;

        SFNCopy(fdb->Name, name, 0);
        if (fdb->Name[0] == 0xE5)
            fdb->Name[0] = 0x05;

        /* write to 1st sector of Root Dir for setting volume label */
        if (fs_type == FS_TYPE_FAT32)
            Mcard_DeviceWrite(drv, sector_buf, FIRST_SECTOR_OF_CLUSTER(START_CLUSTER), 1);
        else if ((fs_type == FS_TYPE_FAT16) || (fs_type == FS_TYPE_FAT12))
            Mcard_DeviceWrite(drv, sector_buf, (FirstDataSector - RootDirSectors), 1);
#if EXFAT_ENABLE
        else if (fs_type == FS_TYPE_exFAT)
        {
    #if EXFAT_WRITE_ENABLE
            MP_ALERT("%s: To-Do: how to process exFAT Write operations ??", __FUNCTION__);
            return FAIL; //not supported yet
    #else
            MP_ALERT("%s: -I- exFAT Write operations are not supported !", __FUNCTION__);
            return FAIL;
    #endif
        }
#endif

        written_sector_count = 1;
    }
    else
        written_sector_count = 0;

    MpMemSet(sector_buf, 0, BPB_BytsPerSec);

    /* note: RootDirSectors is count of sectors within Root Dir in FAT12/16; but RootDirSectors is always 0 in FAT32 */
    if (fs_type == FS_TYPE_FAT32)
    {
        MP_DEBUG("%s: FAT32: BPB_FATSz32 = %lu, BPB_RsvdSecCnt_Fat32 = %lu, FirstDataSector = %lu, RootDirSectors = %lu, BPB_SecPerClus = %lu", __FUNCTION__, BPB_FATSz32, BPB_RsvdSecCnt_Fat32, FirstDataSector, RootDirSectors, BPB_SecPerClus);

        for (i = written_sector_count; i < BPB_SecPerClus; i++)
            Mcard_DeviceWrite(drv, sector_buf, FIRST_SECTOR_OF_CLUSTER(START_CLUSTER) + i, 1);

        SetFAT32Entry(drv, sector_buf, START_CLUSTER, EOC_FAT32); // end of cluster of FAT32 Root Dir
    }
    else if ((fs_type == FS_TYPE_FAT16) || (fs_type == FS_TYPE_FAT12))
    {
        if (fs_type == FS_TYPE_FAT16)
        {
            MP_DEBUG("%s: FAT16: BPB_FATSz16 = %lu, BPB_RsvdSecCnt_Fat1216 = %lu, FirstDataSector = %lu, RootDirSectors = %lu, BPB_SecPerClus = %lu", __FUNCTION__, BPB_FATSz16, BPB_RsvdSecCnt_Fat1216, FirstDataSector, RootDirSectors, BPB_SecPerClus);
        }
        else if (fs_type == FS_TYPE_FAT12)
        {
            MP_DEBUG("%s: FAT12: BPB_FATSz16 = %lu, BPB_RsvdSecCnt_Fat1216 = %lu, FirstDataSector = %lu, RootDirSectors = %lu, BPB_SecPerClus = %lu", __FUNCTION__, BPB_FATSz16, BPB_RsvdSecCnt_Fat1216, FirstDataSector, RootDirSectors, BPB_SecPerClus);
        }

        for (i = written_sector_count; i < RootDirSectors; i++)
            Mcard_DeviceWrite(drv, sector_buf, (FirstDataSector - RootDirSectors) + i, 1);
    }
#if EXFAT_ENABLE
    /* To-do: check if any difference in DirRoot of exFAT need to process ... */
    else if (fs_type == FS_TYPE_exFAT)
    {
  #if EXFAT_WRITE_ENABLE
        MP_ALERT("%s: To-Do: how to process exFAT Write operations ??", __FUNCTION__);
        return FAIL; //not supported yet
  #else
        MP_ALERT("%s: -I- exFAT Write operations are not supported !", __FUNCTION__);
        return FAIL;
  #endif
    }
#endif

    ext_mem_free(sector_buf);

    return ret;
}



///
///@ingroup DRIVE
///@brief   Format a storage drive to FAT32 file system format with specified volume label.
///
///@param   drv       The drive to format. \n\n
///
///@param   label     The 8.3 short filename string to be the volume label of the drive. \n
///                   NULL pointer or "" (NULL string) is also acceptable.
///
///@retval  PASS      Format drive successfully. \n\n
///@retval  FAIL      Format drive unsuccessfully.
///
///@remark    This function will not invoke DriveAdd() to add file system info of the drive to DRIVE table. \n
///           The application task needs to call DriveAdd() for this drive manually after it is formatted.
///
SWORD Fat32_Format(DRIVE * drv, char * label)
{
    DRIVE_PHY_DEV_ID  phyDevID;
    SWORD ret = PASS;
    BYTE  *sector_buf;
    BYTE  partition_idx, partition_type;
    DWORD partition_start_lba, partition_SectorNr, partition_blockSize;
    BOOL  without_MBR = FALSE;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return FAIL;
    }

    phyDevID = DriveIndex2PhyDevID(drv->DrvIndex); /* avoid logical drive index ID is misused */
    if (phyDevID == DEV_NULL)
    {
        MP_ALERT("%s: Error! Invalid device ID !", __FUNCTION__);
        return FAIL;
    }
    else if ((phyDevID == DEV_USB_WIFI_DEVICE) || \
             (phyDevID == DEV_CF_ETHERNET_DEVICE) || \
             (phyDevID == DEV_USB_ETHERNET_DEVICE) || \
             (phyDevID == DEV_USB_WEBCAM))
    {
        MP_ALERT("%s: Error! The device specified is not a storage device !", __FUNCTION__);
        return FAIL;
    }

    /* delete this single drive from Drive[] table */
    SingleDriveDelete(drv->DrvIndex);

    /* read MBR partition table to get partition info of the partition drive */
    ret = GetDrvPartitionInfoFromMBR(drv->DrvIndex, &partition_idx, &partition_type, &partition_start_lba, &partition_SectorNr, &partition_blockSize);
    if (ret == FS_SCAN_FAIL)
    {
        MP_ALERT("%s: No MBR sector found on the drive ...", __FUNCTION__);
        without_MBR = TRUE;
        partition_start_lba = 0;
        partition_SectorNr = Mcard_GetCapacity(phyDevID);
    }
    else if (ret == FAIL)
    {
        MP_ALERT("%s: Error! GetDrvPartitionInfoFromMBR() failed !", __FUNCTION__);
        return FAIL;
    }
    else
        without_MBR = FALSE;

    sector_buf = (BYTE *) ext_mem_malloc(BPB_BytsPerSec);
    if (sector_buf == NULL)
    {
        MP_ALERT("%s: malloc fail !!", __FUNCTION__);
        return FAIL;
    }
    sector_buf = (BYTE *) ((DWORD) sector_buf | BIT29);

    if (partition_SectorNr < 65525) /* here, check count of clusters for FAT32 by using worst case, i.e. 1 cluster == 1 sector */
    {
        MP_ALERT("%s: Cluster number (%d) is less than 65525, not compliant to FAT32 !!", __FUNCTION__, partition_SectorNr);
        ext_mem_free(sector_buf);
        return FAIL;
    }
    else
    {
        drv->Flag.ReadOnly = 0; /* For writing to the drive, force to clear Read-Only flag in file system layer first */

        if (without_MBR == FALSE)
        {
            /* update partition type and write back to MBR */
            if (UpdatePartitionTypeinMBR(drv, FS_TYPE_FAT32) != PASS)
            {
                MP_ALERT("%s: UpdatePartitionTypeinMBR() failed !", __FUNCTION__);
                ext_mem_free(sector_buf);
                return FAIL;
            }
        }

        /* write Partition boot sector */
        ret = BuildPartitionBPB(drv, sector_buf, partition_start_lba, partition_SectorNr, FS_TYPE_FAT32);
        if (ret == FAIL)
        {
            MP_ALERT("%s: BuildPartitionBPB() failed => Formatting failed !!", __FUNCTION__);
            ext_mem_free(sector_buf);
            return FAIL;
        }
        Mcard_DeviceWrite(drv, sector_buf, partition_start_lba, 1);

        MP_ALERT("\t Sector size: %d bytes", BPB_BytsPerSec);
        MP_ALERT("\t Sectors per cluster: %d", BPB_SecPerClus);
        MP_ALERT("\t Total clusters: %d", DataSectorsCnt/BPB_SecPerClus);

        /* backup of partition boot sector */
        Mcard_DeviceWrite(drv, sector_buf, partition_start_lba+6, 1);

        // write FS Info
        BuildPartitionFSInfo(sector_buf, FatEntriesCnt - 3, 3);
        Mcard_DeviceWrite(drv, sector_buf, partition_start_lba+1, 1);

        // backup of FS Info
        Mcard_DeviceWrite(drv, sector_buf, partition_start_lba+7, 1);

        // reserved sector
        InitSectorSignature_55AA(sector_buf);
        Mcard_DeviceWrite(drv, sector_buf, partition_start_lba+2, 1);

        // backup of reserved sector
        Mcard_DeviceWrite(drv, sector_buf, partition_start_lba+8, 1);

        /* clean FAT tables */
        DWORD sz = (FatEntriesCnt * 4)/BPB_BytsPerSec, pos = (partition_start_lba + BPB_RsvdSecCnt_Fat32);
        DWORD tmp_sz = (1024 * 1024)/BPB_BytsPerSec; /* size of some sectors to erase/overwrite FAT region sectors */
        ext_mem_free(sector_buf);

        tmp_sz = ((sz > tmp_sz)? tmp_sz : sz); /* make sure (tmp_sz <= FAT table size), in sectors */

        for ( ; tmp_sz > 0; tmp_sz--)	// find available memory
        {
            sector_buf = (BYTE *) ext_mem_malloc(tmp_sz * BPB_BytsPerSec);
            if (sector_buf)
                break;
        }
        if (sector_buf == NULL)
        {
            MP_ALERT("%s: malloc fail !!", __FUNCTION__);
            return FAIL;
        }
        else
        {
            MpMemSet(sector_buf, 0x00, tmp_sz * BPB_BytsPerSec);
            while (sz)
            {
                tmp_sz = ((sz > tmp_sz)? tmp_sz : sz); /* make sure (tmp_sz <= remaining FAT table size), in sectors */
                Mcard_DeviceWrite(drv, sector_buf, pos, tmp_sz);
                pos += tmp_sz;
                sz = (sz > tmp_sz) ? (sz - tmp_sz) : 0;
            }
            // set default cluster entry 0 and 1
            SetFAT32Entry(drv, sector_buf, 0, CLUSTER0_FAT32);
            SetFAT32Entry(drv, sector_buf, 1, EOC_FAT32);

            ret = BuildFatDirRoot(drv, label, FS_TYPE_FAT32);
            if (ret == FAIL)
            {
                MP_ALERT("%s: BuildFatDirRoot() failed => Formatting failed !!", __FUNCTION__);
            }
            else
            {
                MP_ALERT("%s: Formatting finished.", __FUNCTION__);
            }

            ext_mem_free(sector_buf);
        }
    }

    return ret;
}



///
///@ingroup DRIVE
///@brief   Format a storage drive to FAT16 file system format with specified volume label.
///
///@param   drv       The drive to format. \n\n
///
///@param   label     The 8.3 short filename string to be the volume label of the drive. \n
///                   NULL pointer or "" (NULL string) is also acceptable.
///
///@retval  PASS      Format drive successfully. \n\n
///@retval  FAIL      Format drive unsuccessfully.
///
///@remark    This function will not invoke DriveAdd() to add file system info of the drive to DRIVE table. \n
///           The application task needs to call DriveAdd() for this drive manually after it is formatted.
///
SWORD Fat16_Format(DRIVE * drv, char * label)
{
    DRIVE_PHY_DEV_ID  phyDevID;
    SWORD ret = PASS;
    BYTE  *sector_buf;
    BYTE  partition_idx, partition_type;
    DWORD partition_start_lba, partition_SectorNr, partition_blockSize;
    BOOL  without_MBR = FALSE;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return FAIL;
    }

    phyDevID = DriveIndex2PhyDevID(drv->DrvIndex); /* avoid logical drive index ID is misused */
    if (phyDevID == DEV_NULL)
    {
        MP_ALERT("%s: Error! Invalid device ID !", __FUNCTION__);
        return FAIL;
    }
    else if ((phyDevID == DEV_USB_WIFI_DEVICE) || (phyDevID == DEV_CF_ETHERNET_DEVICE))
    {
        MP_ALERT("%s: Error! The device specified is not a storage device !", __FUNCTION__);
        return FAIL;
    }

    /* delete this single drive from Drive[] table */
    SingleDriveDelete(drv->DrvIndex);

    /* read MBR partition table to get partition info of the partition drive */
    ret = GetDrvPartitionInfoFromMBR(drv->DrvIndex, &partition_idx, &partition_type, &partition_start_lba, &partition_SectorNr, &partition_blockSize);
    if (ret == FS_SCAN_FAIL)
    {
        MP_ALERT("%s: No MBR sector found on the drive ...", __FUNCTION__);
        without_MBR = TRUE;
        partition_start_lba = 0;
        partition_SectorNr = Mcard_GetCapacity(phyDevID);
    }
    else if (ret == FAIL)
    {
        MP_ALERT("%s: Error! GetDrvPartitionInfoFromMBR() failed !", __FUNCTION__);
        return FAIL;
    }
    else
        without_MBR = FALSE;

    sector_buf = (BYTE *) ext_mem_malloc(BPB_BytsPerSec);
    if (sector_buf == NULL)
    {
        MP_ALERT("%s: malloc fail !!", __FUNCTION__);
        return FAIL;
    }
    sector_buf = (BYTE *) ((DWORD) sector_buf | BIT29);

    if ((partition_SectorNr / 64) >= 65525) /* here, check count of clusters for FAT16 by using worst case, i.e. 1 cluster == 64 sectors */
    {
        MP_ALERT("%s: Cluster number (%d) >= 65525, not compliant to FAT16 !!", __FUNCTION__, partition_SectorNr);
        ext_mem_free(sector_buf);
        return FAIL;
    }
    else if (partition_SectorNr < 4085) /* here, check count of clusters for FAT16 by using worst case, i.e. 1 cluster == 1 sector */
    {
        MP_ALERT("%s: Cluster number (%d) < 4085, not compliant to FAT16 !!", __FUNCTION__, partition_SectorNr);
        ext_mem_free(sector_buf);
        return FAIL;
    }
    else
    {
        drv->Flag.ReadOnly = 0; /* For writing to the drive, force to clear Read-Only flag in file system layer first */

        if (without_MBR == FALSE)
        {
            /* update partition type and write back to MBR */
            if (UpdatePartitionTypeinMBR(drv, FS_TYPE_FAT16) != PASS)
            {
                MP_ALERT("%s: UpdatePartitionTypeinMBR() failed !", __FUNCTION__);
                ext_mem_free(sector_buf);
                return FAIL;
            }
        }

        /* write Partition boot sector */
        ret = BuildPartitionBPB(drv, sector_buf, partition_start_lba, partition_SectorNr, FS_TYPE_FAT16);
        if (ret == FAIL)
        {
            MP_ALERT("%s: BuildPartitionBPB() failed => Formatting failed !!", __FUNCTION__);
            ext_mem_free(sector_buf);
            return FAIL;
        }
        Mcard_DeviceWrite(drv, sector_buf, partition_start_lba, 1);

        MP_ALERT("\t Sector size: %d bytes", BPB_BytsPerSec);
        MP_ALERT("\t Sectors per cluster: %d", BPB_SecPerClus);
        MP_ALERT("\t Total clusters: %d", DataSectorsCnt/BPB_SecPerClus);

        /* clean FAT tables */
        DWORD sz = (FatEntriesCnt * 2)/BPB_BytsPerSec, pos = (partition_start_lba + BPB_RsvdSecCnt_Fat1216);
        DWORD tmp_sz = (1024 * 1024)/BPB_BytsPerSec; /* size of some sectors to erase/overwrite FAT region sectors */
        ext_mem_free(sector_buf);

        tmp_sz = ((sz > tmp_sz)? tmp_sz : sz); /* make sure (tmp_sz <= FAT table size), in sectors */

        for ( ; tmp_sz > 0; tmp_sz--)	// find available memory
        {
            sector_buf = (BYTE *) ext_mem_malloc(tmp_sz * BPB_BytsPerSec);
            if (sector_buf)
                break;
        }
        if (sector_buf == NULL)
        {
            MP_ALERT("%s: malloc fail !!", __FUNCTION__);
            return FAIL;
        }
        else
        {
            MpMemSet(sector_buf, 0x00, tmp_sz * BPB_BytsPerSec);
            while (sz)
            {
                tmp_sz = ((sz > tmp_sz)? tmp_sz : sz); /* make sure (tmp_sz <= remaining FAT table size), in sectors */
                Mcard_DeviceWrite(drv, sector_buf, pos, tmp_sz);
                pos += tmp_sz;
                sz = (sz > tmp_sz) ? (sz - tmp_sz) : 0;
            }
            // set default cluster entry 0 and 1
            SetFAT16Entry(drv, sector_buf, 0, CLUSTER0_FAT16);
            SetFAT16Entry(drv, sector_buf, 1, EOC_FAT16);

            ret = BuildFatDirRoot(drv, label, FS_TYPE_FAT16);
            if (ret == FAIL)
            {
                MP_ALERT("%s: BuildFatDirRoot() failed => Formatting failed !!", __FUNCTION__);
            }
            else
            {
                MP_ALERT("%s: Formatting finished.", __FUNCTION__);
            }

            ext_mem_free(sector_buf);
        }
    }

    return ret;
}



///
///@ingroup DRIVE
///@brief   Format a storage drive to FAT12 file system format with specified volume label.
///
///@param   drv       The drive to format. \n\n
///
///@param   label     The 8.3 short filename string to be the volume label of the drive. \n
///                   NULL pointer or "" (NULL string) is also acceptable.
///
///@retval  PASS      Format drive successfully. \n\n
///@retval  FAIL      Format drive unsuccessfully.
///
///@remark    This function will not invoke DriveAdd() to add file system info of the drive to DRIVE table. \n
///           The application task needs to call DriveAdd() for this drive manually after it is formatted.
///
SWORD Fat12_Format(DRIVE * drv, char * label)
{
    DRIVE_PHY_DEV_ID  phyDevID;
    SWORD ret = PASS;
    BYTE  *sector_buf;
    BYTE  partition_idx, partition_type;
    DWORD partition_start_lba, partition_SectorNr, partition_blockSize;
    BOOL  without_MBR = FALSE;


    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return FAIL;
    }

    phyDevID = DriveIndex2PhyDevID(drv->DrvIndex); /* avoid logical drive index ID is misused */
    if (phyDevID == DEV_NULL)
    {
        MP_ALERT("%s: Error! Invalid device ID !", __FUNCTION__);
        return FAIL;
    }
    else if ((phyDevID == DEV_USB_WIFI_DEVICE) || (phyDevID == DEV_CF_ETHERNET_DEVICE))
    {
        MP_ALERT("%s: Error! The device specified is not a storage device !", __FUNCTION__);
        return FAIL;
    }

    /* delete this single drive from Drive[] table */
    SingleDriveDelete(drv->DrvIndex);

    /* read MBR partition table to get partition info of the partition drive */
    ret = GetDrvPartitionInfoFromMBR(drv->DrvIndex, &partition_idx, &partition_type, &partition_start_lba, &partition_SectorNr, &partition_blockSize);
    if (ret == FS_SCAN_FAIL)
    {
        MP_ALERT("%s: No MBR sector found on the drive ...", __FUNCTION__);
        without_MBR = TRUE;
        partition_start_lba = 0;
        partition_SectorNr = Mcard_GetCapacity(phyDevID);
    }
    else if (ret == FAIL)
    {
        MP_ALERT("%s: Error! GetDrvPartitionInfoFromMBR() failed !", __FUNCTION__);
        return FAIL;
    }
    else
        without_MBR = FALSE;

    sector_buf = (BYTE *) ext_mem_malloc(BPB_BytsPerSec);
    if (sector_buf == NULL)
    {
        MP_ALERT("%s: malloc fail !!", __FUNCTION__);
        return FAIL;
    }
    sector_buf = (BYTE *) ((DWORD) sector_buf | BIT29);

    if ((partition_SectorNr / 64) >= 4085) /* here, check count of clusters for FAT12 by using worst case, i.e. 1 cluster == 64 sectors */
    {
        MP_ALERT("%s: Cluster number (%d) >= 4085, not compliant to FAT12 !!", __FUNCTION__, partition_SectorNr);
        ext_mem_free(sector_buf);
        return FAIL;
    }
    else
    {
        drv->Flag.ReadOnly = 0; /* For writing to the drive, force to clear Read-Only flag in file system layer first */

        if (without_MBR == FALSE)
        {
            /* update partition type and write back to MBR */
            if (UpdatePartitionTypeinMBR(drv, FS_TYPE_FAT12) != PASS)
            {
                MP_ALERT("%s: UpdatePartitionTypeinMBR() failed !", __FUNCTION__);
                ext_mem_free(sector_buf);
                return FAIL;
            }
        }

        /* write Partition boot sector */
        ret = BuildPartitionBPB(drv, sector_buf, partition_start_lba, partition_SectorNr, FS_TYPE_FAT12);
        if (ret == FAIL)
        {
            MP_ALERT("%s: BuildPartitionBPB() failed => Formatting failed !!", __FUNCTION__);
            ext_mem_free(sector_buf);
            return FAIL;
        }
        Mcard_DeviceWrite(drv, sector_buf, partition_start_lba, 1);

        MP_ALERT("\t Sector size: %d bytes", BPB_BytsPerSec);
        MP_ALERT("\t Sectors per cluster: %d", BPB_SecPerClus);
        MP_ALERT("\t Total clusters: %d", DataSectorsCnt/BPB_SecPerClus);

        /* clean FAT tables */
        DWORD sz = (FatEntriesCnt * 3 / 2)/BPB_BytsPerSec, pos = (partition_start_lba + BPB_RsvdSecCnt_Fat1216);
        DWORD tmp_sz = (1024 * 1024)/BPB_BytsPerSec; /* size of some sectors to erase/overwrite FAT region sectors */
        ext_mem_free(sector_buf);

        tmp_sz = ((sz > tmp_sz)? tmp_sz : sz); /* make sure (tmp_sz <= FAT table size), in sectors */

        for ( ; tmp_sz > 0; tmp_sz--)	// find available memory
        {
            sector_buf = (BYTE *) ext_mem_malloc(tmp_sz * BPB_BytsPerSec);
            if (sector_buf)
                break;
        }
        if (sector_buf == NULL)
        {
            MP_ALERT("%s: malloc fail !!", __FUNCTION__);
            return FAIL;
        }
        else
        {
            MpMemSet(sector_buf, 0x00, tmp_sz * BPB_BytsPerSec);
            while (sz)
            {
                tmp_sz = ((sz > tmp_sz)? tmp_sz : sz); /* make sure (tmp_sz <= remaining FAT table size), in sectors */
                Mcard_DeviceWrite(drv, sector_buf, pos, tmp_sz);
                pos += tmp_sz;
                sz = (sz > tmp_sz) ? (sz - tmp_sz) : 0;
            }
            // set default cluster entry 0 and 1
            SetFAT12Entry(drv, sector_buf, 0, CLUSTER0_FAT12);
            SetFAT12Entry(drv, sector_buf, 1, EOC_FAT12);

            ret = BuildFatDirRoot(drv, label, FS_TYPE_FAT12);
            if (ret == FAIL)
            {
                MP_ALERT("%s: BuildFatDirRoot() failed => Formatting failed !!", __FUNCTION__);
            }
            else
            {
                MP_ALERT("%s: Formatting finished.", __FUNCTION__);
            }

            ext_mem_free(sector_buf);
        }
    }

    return ret;
}



#if EXFAT_ENABLE
 #if 0 //disable this Doxygen comment before implement completely
///
///@ingroup DRIVE
///@brief   Format a storage drive to exFAT file system format with specified volume label.
///
///@param   drv       The drive to format. \n\n
///@param   label     The string to be the volume label of the drive.
///
///@retval  PASS      Format drive successfully. \n\n
///@retval  FAIL      Format drive unsuccessfully.
///
 #endif
SWORD exFAT_Format(DRIVE * drv, char * label)
{
    DRIVE_PHY_DEV_ID  phyDevID;

    if (drv == NULL)
    {
        MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
        return FAIL;
    }

    phyDevID = DriveIndex2PhyDevID(drv->DrvIndex); /* avoid logical drive index ID is misused */
    if (phyDevID == DEV_NULL)
    {
        MP_ALERT("%s: Error! Invalid device ID !", __FUNCTION__);
        return FAIL;
    }
    else if ((phyDevID == DEV_USB_WIFI_DEVICE) || (phyDevID == DEV_CF_ETHERNET_DEVICE))
    {
        MP_ALERT("%s: Error! The device specified is not a storage device !", __FUNCTION__);
        return FAIL;
    }

    /* delete this single drive from Drive[] table */
    SingleDriveDelete(drv->DrvIndex);

  #if EXFAT_WRITE_ENABLE
    drv->Flag.ReadOnly = 0; /* for writing, force to clear Read-Only flag in file system layer first */

    MP_ALERT("%s: To-Do: how to process exFAT Write operations ??", __FUNCTION__);
    return FAIL; //not supported yet
  #else
    MP_ALERT("%s: -I- exFAT Write operations are not supported !", __FUNCTION__);
    return FAIL;
  #endif
}
#endif



///
///@ingroup DRIVE
///@brief   Format a storage drive to FAT32 or FAT16 or FAT12 file system format with specified volume label. \n
///         This function will try to format as FAT32 first, and then try FAT16 if FAT32 formatting failed. And then
///         try FAT12 if FAT16 formatting also failed. \n
///         This function is suitable for GUI or other App to use, that it does not care FAT12 or FAT16 or FAT32
///         file system type is finally formatted on the drive.
///
///@param   drive_index   Drive index ID of the storage device to be formatted. \n\n
///
///@param   label     The 8.3 short filename string to be the volume label of the drive. \n
///                   NULL pointer or "" (NULL string) is also acceptable.
///
///@retval  PASS      Format drive successfully. \n\n
///@retval  FAIL      Format drive unsuccessfully.
///
///@remark    This function will invoke DriveAdd() internally to add file system info of the drive to DRIVE table
///           after the formatting procedure.
///
SWORD Drive_Formatting(E_DRIVE_INDEX_ID drive_index, char * label)
{
    SWORD ret = PASS;

    if (ret = Fat32_Format(DriveGet(drive_index), label) != PASS)
        if (ret = Fat16_Format(DriveGet(drive_index), label) != PASS)
            if (ret = Fat12_Format(DriveGet(drive_index), label) != PASS)
                MP_ALERT("%s: (DrvId = %d) FAT32/FAT16/FAT12 formatting all failed !", __FUNCTION__, drive_index);

    SingleDriveDelete(drive_index);

    /* after drive formatting, DriveAdd() again */
    DriveAdd(drive_index);

    return ret;
}



///
///@ingroup DRIVE
///@brief   Partition the space of a storage device to a single partition or up to two partitions totally. \n\n
///
///@param   drive_index        Drive index ID of a storage device to be partitioned. \n\n
///
///@param   partition2_size    Size of reserved space on the device to be partitioned for 2nd partition. (unit: number of 256 KB). \n
///                            Set this parameter to 0 if you don't want to have 2nd partition. \n\n
///
///@retval  PASS      Partition disk successfully. \n\n
///@retval  FAIL      Partition disk unsuccessfully.
///
///@remark    1. This function will overwrite the content of partition table in MBR sector (physical sector 0) of the device. \n\n
///
///           2. This function invokes DriveDelete() internally to delete/reset file system info of all drives of the device from DRIVE table.
///
SWORD DiskPartitioning_WithMaxTwoPartitions(E_DRIVE_INDEX_ID drive_index, DWORD partition2_size)
{
#define PART2_SIZE_UNIT  (256 * 1024) /* unit: 256 KB */
    DRIVE_PHY_DEV_ID  phyDevID;
    SWORD ret = PASS;
    BYTE  *sector_buf;
    DWORD SectorSize, HiddSec_cnt;
    DWORD total_SectorNr, partition1_SectorNr, partition2_SectorNr, partition1_StartSector, partition2_StartSector;
    DRIVE *tmp_drv; /* to be parameter for Mcard_DeviceRead()/Mcard_DeviceWrite() */
    DWORD tmp_MB_size;


    phyDevID = DriveIndex2PhyDevID(drive_index); /* avoid logical drive index ID is misused */
    if (phyDevID == DEV_NULL)
    {
        MP_ALERT("%s: Error! Invalid device ID !", __FUNCTION__);
        return FAIL;
    }
    else if ((phyDevID == DEV_USB_WIFI_DEVICE) || (phyDevID == DEV_CF_ETHERNET_DEVICE))
    {
        MP_ALERT("%s: Error! The device specified is not a storage device !", __FUNCTION__);
        return FAIL;
    }

    DriveDelete(drive_index); /* delete/reset file system info of all drives of the device from DRIVE table */

    SectorSize = Mcard_GetSectorSize(phyDevID);
    if (SectorSize == 0)
    {
        MP_ALERT("%s: Error! Sector size = 0 ! phyDevID = %d", __FUNCTION__, phyDevID);
        return FAIL;
    }
    else
    {
        MP_ALERT("%s: Sector size = %u bytes", __FUNCTION__, SectorSize);
        if (SectorSize % 512) /* sector size should be multiples of 512-bytes */
        {
            MP_ALERT("%s: Error! Sector size not multiples of 512 => Invalid ! phyDevID = %d", __FUNCTION__, phyDevID);
            return FAIL;
        }
    }

    total_SectorNr = Mcard_GetCapacity(phyDevID);
    MP_DEBUG("%s: total_SectorNr = %u", __FUNCTION__, total_SectorNr);
    if (total_SectorNr == 0)
    {
        MP_ALERT("%s: Error! Count of total sectors of the device = 0 !", __FUNCTION__);
        return FAIL;
    }

    sector_buf = (BYTE *) ext_mem_malloc(SectorSize);
    if (sector_buf == NULL)
    {
        MP_ALERT("%s: malloc fail !!", __FUNCTION__);
        return FAIL;
    }
    sector_buf = (BYTE *) ((DWORD) sector_buf | BIT29);

    /* check value first to avoid (total_SectorNr * SectorSize) exceeding 4GB (upper bound of 32-bit) */
    if (total_SectorNr >= (1 << 20))
    {
        HiddSec_cnt = Default_BPB_HiddSec;
        tmp_MB_size = ((total_SectorNr >> 20) * SectorSize);
        MP_ALERT("%s: Total storage capacity = %d MB", __FUNCTION__, tmp_MB_size);
    }
    else
    {
        tmp_MB_size = MB_SIZE(total_SectorNr * SectorSize);
        if (tmp_MB_size < 2) /* 2 MB */
        {
            HiddSec_cnt = Small_BPB_HiddSec;
            MP_ALERT("%s: Total storage capacity = %d KB", __FUNCTION__, KB_SIZE(total_SectorNr * SectorSize));
        }
        else
        {
            HiddSec_cnt = Default_BPB_HiddSec;
            MP_ALERT("%s: Total storage capacity = %d MB", __FUNCTION__, tmp_MB_size);
        }
    }

    /* prepare for Mcard_DeviceRead()/Mcard_DeviceWrite(): only some fields of DRIVE are needed */
    tmp_drv = DriveGet(drive_index);
    tmp_drv->DevID = phyDevID;

    tmp_drv->Flag.ReadOnly = 0; /* For writing to the drive, force to clear Read-Only flag in file system layer first */

    /* calculate the exponent of 2 for bytes-per-sector */
    tmp_drv->bSectorExp = 0;
    DWORD tmp_value = SectorSize; /* avoid modifying SectorSize value */
    while (tmp_value != 1)
    {
        tmp_value >>= 1;
        tmp_drv->bSectorExp++;
    }

    partition2_SectorNr = (partition2_size * (PART2_SIZE_UNIT/SectorSize));
    partition1_SectorNr = total_SectorNr - HiddSec_cnt - partition2_SectorNr;
    partition1_StartSector = HiddSec_cnt;
    partition2_StartSector = HiddSec_cnt + partition1_SectorNr;

    if (partition2_SectorNr >= (total_SectorNr - HiddSec_cnt))
    {
        MP_ALERT("%s: Error! Specified size for 2nd partition is too big !", __FUNCTION__);
        return FAIL;
    }

    if (partition2_size > 0)
    {
        MP_ALERT("%s: total_SectorNr = %d, partition1_SectorNr = %d, partition2_SectorNr = %d", __FUNCTION__, total_SectorNr, partition1_SectorNr, partition2_SectorNr);
        MP_ALERT("%s: partition_1 start LBA = %d, partition_2 start LBA = %d", __FUNCTION__, partition1_StartSector, partition2_StartSector);
    }
    else
    {
        MP_ALERT("%s: total_SectorNr = %d, partition1_SectorNr = %d, partition_1 start LBA = %d", __FUNCTION__, total_SectorNr, partition1_SectorNr, partition1_StartSector);
    }

    if (partition2_SectorNr > 0) /* To make two partitions on the device */
    {
        /* check value first to avoid (partition1_SectorNr * SectorSize) exceeding 4GB (upper bound of 32-bit) */
        if (partition1_SectorNr >= (1 << 20))
        {
            tmp_MB_size = ((partition1_SectorNr >> 20) * SectorSize);
            MP_ALERT("%s: partition_1 size = %d MB", __FUNCTION__, tmp_MB_size);
        }
        else
        {
            tmp_MB_size = MB_SIZE(partition1_SectorNr * SectorSize);
            if (tmp_MB_size < 2) /* 2 MB */
            {
                MP_ALERT("%s: partition_1 size = %d KB", __FUNCTION__, KB_SIZE(partition1_SectorNr * SectorSize));
            }
            else
            {
                MP_ALERT("%s: partition_1 size = %d MB", __FUNCTION__, tmp_MB_size);
            }
        }

        /* check value first to avoid (partition2_SectorNr * SectorSize) exceeding 4GB (upper bound of 32-bit) */
        if (partition2_SectorNr >= (1 << 20))
        {
            tmp_MB_size = ((partition2_SectorNr >> 20) * SectorSize);
            MP_ALERT("%s: partition_2 size = %d MB", __FUNCTION__, tmp_MB_size);
        }
        else
        {
            tmp_MB_size = MB_SIZE(partition2_SectorNr * SectorSize);
            if (tmp_MB_size < 2) /* 2 MB */
            {
                MP_ALERT("%s: partition_2 size = %d KB", __FUNCTION__, KB_SIZE(partition2_SectorNr * SectorSize));
            }
            else
            {
                MP_ALERT("%s: partition_2 size = %d MB", __FUNCTION__, tmp_MB_size);
            }
        }

        /* write MBR */
        BuildInitMBR(sector_buf, HiddSec_cnt, partition1_SectorNr, partition2_SectorNr, 0, 0);
        ret = Mcard_DeviceWrite(tmp_drv, sector_buf, 0, 1);

  #if 0 /* note: we should not erase boot sector of the partition => let Format_XXX() to do that !! */
        /* erase Partition boot sector */
        MpMemSet(sector_buf, 0, SectorSize);
        ret = Mcard_DeviceWrite(tmp_drv, sector_buf, partition1_StartSector, 1);
        ret = Mcard_DeviceWrite(tmp_drv, sector_buf, partition2_StartSector, 1);
  #endif
    }
    else /* only single partition on the device */
    {
        /* check value first to avoid (total_SectorNr * SectorSize) exceeding 4GB (upper bound of 32-bit) */
        if (total_SectorNr >= (1 << 20))
        {
            tmp_MB_size = ((partition1_SectorNr >> 20) * SectorSize);
            MP_ALERT("%s: Only single partition, partition size = %d MB, start LBA = %d", __FUNCTION__, tmp_MB_size, partition1_StartSector);
        }
        else
        {
            tmp_MB_size = MB_SIZE(partition1_SectorNr * SectorSize);
            if (tmp_MB_size < 2) /* 2 MB */
            {
                MP_ALERT("%s: Only single partition, partition size = %d KB, start LBA = %d", __FUNCTION__, KB_SIZE(partition1_SectorNr * SectorSize), partition1_StartSector);
            }
            else
            {
                MP_ALERT("%s: Only single partition, partition size = %d MB, start LBA = %d", __FUNCTION__, tmp_MB_size, partition1_StartSector);
            }
        }

        /* write MBR */
        BuildInitMBR(sector_buf, HiddSec_cnt, partition1_SectorNr, 0, 0, 0);
        ret = Mcard_DeviceWrite(tmp_drv, sector_buf, 0, 1);

  #if 0 /* note: we should not erase boot sector of the partition => let Format_XXX() to that !! */
        /* erase Partition boot sector */
        MpMemSet(sector_buf, 0, SectorSize);
        ret = Mcard_DeviceWrite(tmp_drv, sector_buf, partition1_StartSector, 1);
  #endif
    }

    /* prepare return value */
    if (ret != FS_SUCCEED)
    {
        ret = FAIL;
        MP_ALERT("%s: Partitioning failed !!", __FUNCTION__);
    }
    else
    {
        ret = PASS;
        MP_ALERT("%s: Partitioning finished.", __FUNCTION__);
    }

    ext_mem_free(sector_buf);
    return ret;
}



///
///@ingroup DRIVE
///@brief   Allocate and create a new disk partition from the left space of the specified storage device. Then update the partition table in MBR. \n
///         Previously existing partitions will not be destroied by this function invocation. \n\n
///
///@param   phyDevID                 [IN] The physical device ID of the storage device to be partitioned. \n\n
///
///@param   new_partition_size       [IN] Size (unit: KB) of disk space on the device to be allocated for the new partition.\n
///                                       Setting this parameter to ALLOC_ALL_LEFT_DISK_SPACE means to allocate all left disk space for the new partition. \n
///                                       If value of this parameter is greater than left disk space, then just return FAIL. \n\n
///
///@param   op_type                  [IN] Operation types for how to creating a new disk partition.\n
///                                       Valid values are: \n
///                                         E_ERASE_OLD_PARTITION_TABLE_FIRST   - to erase old partition table first before creating a new disk partition; \n
///                                         E_BEHIND_EXISTING_PARTITION_ENTRIES - to create a new disk partition directly behind existing partitions. \n\n
///
///@param   left_unused_space_size   [OUT] Size (unit: KB) of left disk space on the device behind all existing partitions if disk partitioning successfully.\n\n
///
///@retval  PASS      Create new disk partition successfully. \n\n
///@retval  FAIL      Create new disk partition unsuccessfully.
///
///@remark    1. If value of 'new_partition_size' parameter is greater than left disk space, then just return FAIL. \n
///              If value of 'new_partition_size' parameter is set to ALLOC_ALL_LEFT_DISK_SPACE, then allocate all left disk space for the new partition. \n\n
///
///           2. This function invokes DriveDelete() internally to delete all partition drives of the specified device from DRIVE table. \n\n
///
SWORD CreateNewDiskPartition(E_DEVICE_ID phyDevID, DWORD new_partition_size, DISK_PARTITIONING_OP_TYPE op_type, DWORD * left_unused_space_size)
{
#define PARTITION_ID_UNKNOWN  0xFF
    SWORD ret = PASS;
    BYTE  *sector_buf;
    E_DRIVE_INDEX_ID drv_p0_index;
    DRIVE *tmp_drv; /* to be parameter for Mcard_DeviceRead()/Mcard_DeviceWrite() */
    PARTITION *partition;
    PARTITION partition_buf[4]; /* for Partition Table content */
    BOOL  f_have_MBR = TRUE;
    DWORD SectorSize, HiddSec_cnt, total_SectorNr, tmp_MB_size;
    DWORD left_space_SectorNr;
    BYTE  Id_of_new_partition = PARTITION_ID_UNKNOWN; /* partition index for the new partition to be created */
    DWORD  partitionStartLba, partitionSectorNr;
    BYTE   i;


    if ((phyDevID == DEV_NULL) || (phyDevID >= MAX_DEVICE_DRV))
    {
        MP_ALERT("%s: Error! Invalid device ID !", __FUNCTION__);
        return FAIL;
    }
    else if ((phyDevID == DEV_USB_WIFI_DEVICE) || \
             (phyDevID == DEV_CF_ETHERNET_DEVICE) || \
             (phyDevID == DEV_USB_ETHERNET_DEVICE) || \
             (phyDevID == DEV_USB_WEBCAM))
    {
        MP_ALERT("%s: Error! The device specified is not a storage device !", __FUNCTION__);
        return FAIL;
    }

    if (new_partition_size == 0)
    {
        MP_ALERT("%s: Error! Specified partition size is 0 (sectors) !", __FUNCTION__);
        return FAIL;
    }

    if ((op_type != E_ERASE_OLD_PARTITION_TABLE_FIRST) && (op_type != E_BEHIND_EXISTING_PARTITION_ENTRIES))
    {
        MP_ALERT("%s: Error! Invalid 'op_type' parameter specified !", __FUNCTION__);
        return FAIL;
    }

    if (left_unused_space_size == NULL)
    {
        MP_ALERT("%s: Error! NULL pointer for the 'left_unused_space_size' parameter !", __FUNCTION__);
        return FAIL;
    }

    drv_p0_index = PhyDevID2DriveIndex(phyDevID);
    DriveDelete(drv_p0_index); /* delete/reset file system info of all drives of the device from DRIVE table */

    SectorSize = Mcard_GetSectorSize(phyDevID);
    if (SectorSize == 0)
    {
        MP_ALERT("%s: Error! Sector size = 0 ! phyDevID = %d", __FUNCTION__, phyDevID);
        return FAIL;
    }
    else
    {
        MP_ALERT("%s: Sector size = %u bytes", __FUNCTION__, SectorSize);
        if (SectorSize % 512) /* sector size should be multiples of 512-bytes */
        {
            MP_ALERT("%s: Error! Sector size not multiples of 512 => Invalid ! phyDevID = %d", __FUNCTION__, phyDevID);
            return FAIL;
        }
    }
    partitionSectorNr = (new_partition_size * 1024) / SectorSize;  /* calculate needed count of sectors */

    total_SectorNr = Mcard_GetCapacity(phyDevID);
    if (total_SectorNr == 0)
    {
        MP_ALERT("%s: Error! Count of total sectors of the device = 0 !", __FUNCTION__);
        return FAIL;
    }

    /* check value first to avoid (total_SectorNr * SectorSize) exceeding 4GB (upper bound of 32-bit) */
    if (total_SectorNr >= (1 << 20))
    {
        HiddSec_cnt = Default_BPB_HiddSec;
        tmp_MB_size = ((total_SectorNr >> 20) * SectorSize);
        MP_ALERT("%s: Total storage capacity = %d MB", __FUNCTION__, tmp_MB_size);
    }
    else
    {
        tmp_MB_size = MB_SIZE(total_SectorNr * SectorSize);
        if (tmp_MB_size < 2) /* 2 MB */
        {
            HiddSec_cnt = Small_BPB_HiddSec;
            MP_ALERT("%s: Total storage capacity = %d KB", __FUNCTION__, KB_SIZE(total_SectorNr * SectorSize));
        }
        else
        {
            HiddSec_cnt = Default_BPB_HiddSec;
            MP_ALERT("%s: Total storage capacity = %d MB", __FUNCTION__, tmp_MB_size);
        }
    }

    /* prepare for Mcard_DeviceRead()/Mcard_DeviceWrite(): only some fields of DRIVE are needed */
    tmp_drv = DriveGet(drv_p0_index);
    tmp_drv->DevID = phyDevID;

    tmp_drv->Flag.ReadOnly = 0; /* For writing to the drive, force to clear Read-Only flag in file system layer first */

    /* calculate the exponent of 2 for bytes-per-sector */
    tmp_drv->bSectorExp = 0;
    DWORD tmp_value = SectorSize; /* avoid modifying SectorSize value */
    while (tmp_value != 1)
    {
        tmp_value >>= 1;
        tmp_drv->bSectorExp++;
    }

    sector_buf = (BYTE *) ext_mem_malloc(SectorSize);
    if (sector_buf == NULL)
    {
        MP_ALERT("%s: malloc fail !!", __FUNCTION__);
        return FAIL;
    }
    sector_buf = (BYTE *) ((DWORD) sector_buf | BIT29);

    /* read MBR */
    Mcard_DeviceRead(tmp_drv, sector_buf, 0, 1);

    if (*(WORD *) (sector_buf + SIGNATURE_OFFSET) != COMMON_SIGNATURE)  /* check MBR sector signature (2 bytes: offset 510, 511) */
    {
        MP_ALERT("%s: -I- Signature 0x55AA not found => no MBR.", __FUNCTION__);
        f_have_MBR = FALSE;
    }
    else  /* check whether if sector 0 is actually a boot sector */
    {
        /* check first 3 bytes for boot sector BS_jmpBoot pattern */
        if ( ((sector_buf[0] == 0xEB) && (sector_buf[2] == 0x90)) || (sector_buf[0] == 0xE9) )
        {
            MP_ALERT("%s: (Boot Sector) BS_jmpBoot pattern found => sector 0 is actually a boot sector => no MBR.", __FUNCTION__);
            f_have_MBR = FALSE;
        }
        else
        {
            if (op_type == E_ERASE_OLD_PARTITION_TABLE_FIRST)
                f_have_MBR = FALSE; /* force to erase MBR first */
            else if (op_type == E_BEHIND_EXISTING_PARTITION_ENTRIES)
                f_have_MBR = TRUE;
        }
    }

    if (! f_have_MBR)
    {
        Id_of_new_partition = 0; /* the 1st partition */
        left_space_SectorNr = total_SectorNr - HiddSec_cnt;

        MpMemSet(sector_buf, 0, SectorSize);
        if (new_partition_size == ALLOC_ALL_LEFT_DISK_SPACE)
        {
            /* total only single one partition */
            partitionSectorNr = left_space_SectorNr;
            MP_DEBUG("%s: allocate all left disk space (%lu sectors) for the new partition ...", __FUNCTION__, left_space_SectorNr);
        }
        else
        {
            if (partitionSectorNr > left_space_SectorNr)
            {
                MP_ALERT("%s: -E- Unused disk space is less than the specified partition size !", __FUNCTION__);
                ext_mem_free(sector_buf);
                return FAIL;
            }
            else if (partitionSectorNr <= left_space_SectorNr)
            {
                /* currently, only 1st partition is to be created */
                MP_DEBUG("%s: allocate (%lu sectors) for the new partition ...", __FUNCTION__, partitionSectorNr);
            }
        }

        /* prepare MBR sector content */
        BuildInitMBR(sector_buf, HiddSec_cnt, partitionSectorNr, 0, 0, 0);

        *left_unused_space_size = KB_SIZE((left_space_SectorNr - partitionSectorNr) * SectorSize); /* unit: KB */
        partitionStartLba = HiddSec_cnt;
    }
    else
    {
        DWORD entry_SectorNr;
        partition = (PARTITION *) (sector_buf + PARTITION_OFFSET);
        MpMemCopy((BYTE *) &partition_buf[0], (BYTE *) partition, sizeof(PARTITION) * 4);

        HiddSec_cnt = byte_swap_of_dword(partition_buf[0].Start); /* real count of hidden sectors before the 1st partition */
        left_space_SectorNr = total_SectorNr - HiddSec_cnt;

        for (i = 0; i < 4; i++)
        {
            partitionStartLba = byte_swap_of_dword(partition_buf[i].Start);  //LBA of the first sector in this partition
            entry_SectorNr = byte_swap_of_dword(partition_buf[i].Size);  //length (number of blocks) of this partion
            MP_DEBUG("%s: Partition[%d] start sector = %lu, partition type = %u, count of sectors = %lu", __FUNCTION__, i, partitionStartLba, (BYTE) partition_buf[i].Type, entry_SectorNr);

            if (((BYTE) partition_buf[i].Active != 0x80) && ((BYTE) partition_buf[i].Active != 0x00))
            {
                MP_DEBUG("%s: Invalid Active (Bootable) flag (0x%02x) of this partition table entry.", __FUNCTION__, (BYTE) partition_buf[i].Active);

                /* treat this invalid partition table entry as free space ?? */
                Id_of_new_partition = i;
            }

            switch ((BYTE) partition_buf[i].Type)
            {
                case 1:     // FAT12 type
                case 4:     // FAT16 type
                case 6:     // FAT16 type
                case 14:    // FAT16 type
                case 11:    // FAT32 type
                case 12:    // FAT32 type
                case 5:     // DOS extended type
                case 15:    // DOS extended type
                case 7:     // NTFS, OS/2 HPFS type or exFAT type
                    MP_DEBUG("%s: known partition type (%u) ...", __FUNCTION__, (BYTE) partition_buf[i].Type);
                    if (entry_SectorNr > left_space_SectorNr)
                    {
                        MP_ALERT("%s: partition size setting (%lu) is too big => Invalid !", __FUNCTION__, entry_SectorNr);
                        /* treat this invalid partition table entry as free space */
                        Id_of_new_partition = i;
                    }
                    else
                    {
                        left_space_SectorNr -= entry_SectorNr;
                    }
                    break;

                case 0:     // unused partition table entry
                    MP_DEBUG("%s: partition type 0x00 => Unused partition table entry ...", __FUNCTION__);
                    Id_of_new_partition = i;
                    break;

                default:    // Unknown or not supported partition type, or bad partition table ?
                    MP_DEBUG("%s: Unknown or not supported partition type (%d) ...", __FUNCTION__, (BYTE) partition_buf[i].Type);
                    /* treat this invalid partition table entry as free space */
                    Id_of_new_partition = i;
                    break;
            }

            if (Id_of_new_partition != PARTITION_ID_UNKNOWN)
                break;
        }

        if (Id_of_new_partition == PARTITION_ID_UNKNOWN)
        {
            MP_ALERT("%s: All partition table entries are already allocated => Cannot create new partition !", __FUNCTION__);
            ext_mem_free(sector_buf);
            return FAIL;
        }
        else
        {
            partition_buf[Id_of_new_partition].Active = 0x00;
            partition_buf[Id_of_new_partition].Type = 0x0b; //default partition type = 0x0b (FAT32)

            if (new_partition_size == ALLOC_ALL_LEFT_DISK_SPACE)
            {
                partitionSectorNr = left_space_SectorNr;
                MP_DEBUG("%s: allocate all left disk space (%lu sectors) for the new partition ...", __FUNCTION__, left_space_SectorNr);
            }
            else
            {
                if (partitionSectorNr > left_space_SectorNr)
                {
                    MP_ALERT("%s: -E- Unused disk space is less than the specified partition size !", __FUNCTION__);
                    ext_mem_free(sector_buf);
                    return FAIL;
                }
                else if (partitionSectorNr <= left_space_SectorNr)
                {
                    MP_DEBUG("%s: allocate (%lu sectors) for the new partition ...", __FUNCTION__, partitionSectorNr);
                }
            }

            *left_unused_space_size = KB_SIZE((left_space_SectorNr - partitionSectorNr) * SectorSize); /* unit: KB */

            partition_buf[Id_of_new_partition].Size = byte_swap_of_dword(partitionSectorNr);  //length (number of blocks) of this partion

            /* calculate start LBA of this new partition */
            partitionStartLba = HiddSec_cnt;
            for (i = 0; i < Id_of_new_partition; i++)
                partitionStartLba += byte_swap_of_dword(partition_buf[i].Size);
            partition_buf[Id_of_new_partition].Start = byte_swap_of_dword(partitionStartLba);  //LBA of the first sector in this partition

            SectorToCHS(partitionStartLba, (BYTE *) &(partition_buf[Id_of_new_partition].res1));  //lagacy CHS address of first block in partition
            SectorToCHS(partitionStartLba + partitionSectorNr - 1, (BYTE *) &(partition_buf[Id_of_new_partition].res4));  //lagacy CHS address of last block in partition

            /* clear subsequent partition table entries */
            for (i = (Id_of_new_partition + 1); i < 4; i++)
                MpMemSet((BYTE *) &partition_buf[i], 0, sizeof(PARTITION));
        }

        /* put new partition table content into the MBR sector buffer */
        MpMemCopy((BYTE *) &sector_buf[PARTITION_OFFSET], (BYTE *) &partition_buf[0], sizeof(PARTITION) * 4);    	
    }

    /* write MBR sector to disk */
    ret = Mcard_DeviceWrite(tmp_drv, sector_buf, 0, 1);
    if (ret != FS_SUCCEED)
    {
        MP_ALERT("%s: Write failed => disk partitioning failed !", __FUNCTION__);
        ext_mem_free(sector_buf);
        return FAIL;
    }

  #if 0 /* note: we should not erase boot sector of the partition => let Format_XXX() to that !! */
    /* erase corresponding boot sector of this new partition */
    MpMemSet(sector_buf, 0, SectorSize);
    ret = Mcard_DeviceWrite(tmp_drv, sector_buf, partitionStartLba, 1);
    if (ret != FS_SUCCEED)
    {
        ret = FAIL;
        MP_ALERT("%s: Write failed => disk partitioning failed !", __FUNCTION__);
    }
    else
    {
        ret = PASS;
        MP_ALERT("%s: disk partitioning successfully.", __FUNCTION__);
    }
  #else
    ret = PASS;
    MP_ALERT("%s: disk partitioning successfully.", __FUNCTION__);  
  #endif

    ext_mem_free(sector_buf);
    return ret;
}



MPX_KMODAPI_SET(Fat32_Format);
MPX_KMODAPI_SET(Fat16_Format);
MPX_KMODAPI_SET(Fat12_Format);
#if EXFAT_ENABLE
MPX_KMODAPI_SET(exFAT_Format);
#endif
