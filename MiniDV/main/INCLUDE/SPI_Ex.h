
#include "global612.h"
#include "mpTrace.h"
#ifndef SPI_EX_H_124457658776435345345
#define SPI_EX_H_124457658776435345345  1

/////////
/////////
//    SPI Flash �洢�ļ���˼·:
//    ��SPI Flash���CODE��ʣ��ռ�����������ļ��Ĺ���
//    ��ʣ��ռ�ֳ�ҳ,ÿ4KBһ��ҳ
/////////

////////
//    ҳID�ĺ���:
//      0xfffd:   �ļ���β��־
//      0xfffe:   ��ҳ������ (����SPI Flash�Ĵ洢��Χ)
//      0xffff:   ��ҳΪ��,δ��ʹ��
//      ����ֵ:   �ļ�����һ��ҳ��ID
////////

////////
//    flag �ı�־����:
//      0x00:         ��Ŀ¼��Ϊ��,����ʹ��;
//      0x01:         ɾ����־,��Ŀ¼���ָ�����ļ���ɾ��,����ʹ��;
//      0x02:         ��Ŀ¼��ǿ�,���ļ�����
//      0x03:         ��Ŀ¼���ʾ�ļ�,ϵͳ���� (���ڵ�0��Ŀ¼������,Ϊ�Ѹ�ʽ����־)
////////

#define SPI_MAX_FILE_COUNT              1024
#define SPI_BLOCK_SIZE                  (64*1024)           // 64 KB per block
#define SPI_PAGE_SIZE                   (4*1024)            // 4 KB per page
#define SPI_FS_INFO_ADDR                SPI_FS_START_ADDR
#define SPI_FS_DATA_ADDR                (SPI_FS_INFO_ADDR + SPI_BLOCK_SIZE)

struct __SPI_FILE_ENTRY_
{
    ST_SYSTEM_TIME  time;
    DWORD           filesize;
    WORD            start_page_id;
    BYTE            flag;
    BYTE            filetype;
};
typedef struct __SPI_FILE_ENTRY_ SPI_FILE_ENTRY;

#define SPI_FILE_TYPE_FORMATED_FLAG     0x09
#define SPI_FILE_TYPE_JPG_FILE          0x55
#define SPI_FILE_TYPE_BIN_FILE          0x00
#define SPI_FILE_TYPE_NONE              0xff

#define ERR_SPIFS_UNKNOWN                   (-1)
#define ERR_SPIFS_OUT_OF_RANGE              (-2)
#define ERR_SPIFS_READ_ERROR                (-3)
#define ERR_SPIFS_WRITE_ERROR               (-4)
#define ERR_SPIFS_UNINIT                    (-5)
#define ERR_SPIFS_OUT_OF_FILECOUNT          (-6)
#define ERR_SPIFS_NO_EMPTY_ENTRY            (-7)
#define ERR_SPIFS_NO_ENOUGH_SPACE           (-8)
#define ERR_SPIFS_NO_FREE_PAGE              (-9)


struct __SPI_FAT_TABLE_
{
    SPI_FILE_ENTRY  fentry[SPI_MAX_FILE_COUNT];
    WORD            tables[(SPI_BLOCK_SIZE-(SPI_MAX_FILE_COUNT*sizeof(SPI_FILE_ENTRY)))/sizeof(WORD)];
};

typedef struct __SPI_FAT_TABLE_ SPI_FAT_TABLE;

#define SPI_FS_PAGE_EOF             0xfffd          //�ļ���β��־
#define SPI_FS_PAGE_NOT_EXIST       0xfffe          //��ҳ������ (����SPI Flash�Ĵ洢��Χ)
#define SPI_FS_PAGE_NULL            0xffff          //��ҳΪ��,δ��ʹ��
                                                    //����ֵ:       �ļ�����һ��ҳ��ID
                                                    
#define SPI_FS_ENTRY_NULL           0x00            //��Ŀ¼��Ϊ��,����ʹ��;
#define SPI_FS_ENTRY_EXIST_FILE     0x02            //��Ŀ¼��ǿ�,���ļ�����
#define SPI_FS_ENTRY_FORMATED       0x03            //��Ŀ¼���ʾ�ļ�,ϵͳ���� (���ڵ�0��Ŀ¼������,Ϊ�Ѹ�ʽ����־

DWORD spifs_GetTotalDataSpaceSize();
WORD spifs_GetTotalPageCount();
WORD spifs_GetFreePageCount();
WORD spifs_GetFreePageCount2();
int spifs_ReadFileInfoBlock(BYTE * buffer);
int spifs_WriteFileInfoBlock(BYTE * buffer);
int spifs_WriteBackFATTable();
int spifs_ReadPage(WORD page_id, BYTE * buffer);
int spifs_WritePage(WORD page_id, BYTE * buffer);
int spifs_FileSystemInit();
int spifs_Format();
//
DWORD spifs_GetFileSize(DWORD dwFileIndex);
DWORD spifs_ReadFileToBuffer(DWORD dwFileIndex, BYTE* buffer, int *pError);
DWORD spifs_WriteBufferToNewFile(DWORD* pdwFileIndex, BYTE* buffer, DWORD dwBuffSize, BYTE filetype, int *pError);
DWORD spifs_DeleteFile(DWORD dwFileIndex, BOOL isWriteBackFAT);


#endif  //SPI_EX_H_124457658776435345345




