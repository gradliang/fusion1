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
* 
* Filename		: usbotg_device_msdc.c
* Programmer(s)	: Cloud Wu
* Created Date	: 2008/08/19 
* Description   : MSDC (Mass Storage Device Class) 
*               : USB Device <---> DriverID(FileSystem) <---> DeviceID(MCard)
******************************************************************************** 
*/
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#include "usbotg_std.h"
#include "usbotg_device.h"
#include "usbotg_device_msdc.h"
#include "usbotg_ctrl.h"
#include "usbotg_device_protection.h"
#include "usbotg_device_msdc_teco.h"
 
#if SC_USBDEVICE
/*
// Constant declarations
*/

#define CBW_LEN      31
#define CSW_LEN      13
#define MS_WRITE_PROTECT            0x80
#define DESCRIPTOR_CODE_DEFINITION	0x02000000 // Formatted Media - Current media capacity
#define TOTAL_SECTORS               0x2000
#define BLOCK_LEN                   0x200
#define EXP_VAL                     9
#define SENSE_KEY_LEN               18

#define CBW_SIGNATE                 0x55534243
#define CSW_SIGNATE                 0x55534253
#define CSW_STATUS_SCSI_PASS        0x00
#define CSW_STATUS_SCSI_FAIL        0x01
#define CSW_STATUS_PHASE_ERROR      0x02

#define DISABL_ACCSS            0x00    // access disable                  
#define ENABL_ACCSS             0x01    // access enable                   
#define UNLOAD                  0x02    // unload                          
#define LOAD                    0x03    // load                            

#define     NO_ASC              0x00

// valid
#define SENSE_VALID             0x80    // Sense data is valid as SCSI2     
#define SENSE_INVALID           0x00    // Sense data is invalid as SCSI2   

// error code
#define CUR_ERR                 0x70    // current error                    
#define DEF_ERR                 0x70    // specific command error           

// sense key
#define ILI                     0x20    // ILI bit is on                    

#define NO_SENSE                0x00    // not exist sense key              
#define RECOVER_ERR             0x01    // Target/Logical unit is recoverd  
#define NOT_READY               0x02    // Logical unit is not ready        
#define MEDIA_ERR               0x03    // medium/data error                
#define HW_ERR                  0x04    // hardware error                   
#define ILGAL_REQ               0x05    // CDB/parameter/identify msg error 
#define UNIT_ATTENTION          0x06    // unit attention condition occur   
#define DAT_PRTCT               0x07    // read/write is desable            
#define BLNC_CHK                0x08    // find blank/DOF in read           
                                        // write to unblank area            
#define CPY_ABRT                0x0a    // Copy/Compare/Copy&Verify illgal  
#define ABRT_CMD                0x0b    // Target make the command in error 
#define EQUAL                   0x0c    // Search Data end with Equal       
#define VLM_OVRFLW              0x0d    // Some data are left in buffer     
#define MISCMP                  0x0e    // find inequality                  

#define READ_ERR                -1
#define WRITE_ERR               -2


// sense key Infomation
#define SNSKEYINFO_LEN          3       // length of sense key infomation

#define SKSV                    0x80
#define CDB_ILLEGAL             0x40

// ASC
#define ASC_NO_INFO             0x00
#define ASC_MISCMP              0x1d
#define ASC_INVLD_CDB           0x24
#define ASC_INVLD_PARA          0x26
#define ASC_LU_NOT_REDEAY       0x04
#define ASC_WRITE_ERR           0x0c
#define ASC_READ_ERR            0x11
#define ASC_LOAD_EJCT_ERR       0x53

// ASQC
#define ASCQ_NO_INFO            0x00
#define ASCQ_MISCMP             0x00
#define ASCQ_INVLD_CDB          0x00
#define ASCQ_INVLD_PARA         0x02
#define ASCQ_LU_NOT_REDEAY      0x02
#define ASCQ_WRITE_ERR          0x02
#define ASCQ_READ_ERR           0x00
#define ASCQ_LOAD_EJCT_ERR      0x00

//====== SENSE KEY DEFINITION ========================
// ------- Sense Key ---------------------------------
#define SenseKey0       0x0     // No sense
#define SenseKey1       0x1
#define SenseKey2       0x2
#define SenseKey3       0x3
#define SenseKey4       0x4
#define SenseKey5       0x5
#define SenseKey6       0x6
#define SenseKey7       0x7
#define SenseKey8       0xb
// ------ Sense Code ---------------------------------
#define SenseCode0      0x0
#define SenseCode4      0x4     // Invalid field in command packet
#define SenseCode24     0x24    // Invalid field in command packet
#define SenseCode27     0x27    // Write Protect Media
#define SenseCode28     0x28    // Media Change
#define SenseCode29     0x29    // Power On Reset or Bus Device Reset Occurred
#define SenseCode3A     0x3A    // Mediumn not present
#define SenseCode53     0x53    // Media Removal Prevented
// ------ Sense Code Qualifier -----------------------
#define SenseCodeQ0     0x0     // First qualifier of Sense code
#define SenseCodeQ1     0x1     // First qualifier of Sense code
#define SenseCodeQ2     0x2     // First qualifier of Sense code
#define SenseCodeQ4     0x4     // First qualifier of Sense code

/*-----------------------------------
    Mode Select Parameter
-----------------------------------*/
#define     DBD                 0x08
#define     MD_SENSE_PAGE_CODE  0x3f
#define     PAGE_CONTROL        0xc0
#define     DPO                 0x10
#define     FUA                 0x08
#define     RELADR              0x01
#define     ALL_PARA_PAGE       0x3f
#define     ALL_PARA_SIZE       124
#define     DNSTY               0x00

/*---- Block Descrptor ----*/
#define BLKNUM_LEN              3       /* length of Block number           */
#define BLKLEN_LEN              3       /* length of Block length           */
//#define BLK_DSCRPT_MAX          1       /* Block descriptor max number      */

/*---- page control ----*/
#define CURRENT                 0x00    /* current parameter                */
#define CHANGABLE               0x01    /* changable parameter              */
#define DEFAULT                 0x02    /* default parameter                */
#define SAVED                   0x03    /* saved parameter                  */

/*---- page code ----*/
#define PAGE_MAX                30      /* Parameter page max size          */
//#define PARA_PAGE_MAX           9       /* Parameter page max number        */

#define RWERR_RECVRY_PAGE       0x01    /* read/write error                 */
#define DECNCT_RECNCT_PAGE      0x02    /* disconnect/reconnect parameter   */
#define FORMT_PAGE              0x03    /* format parameter                 */
#define VRFYERR_RECVRY_PAGE     0x07    /* verify error recovery parameter  */
#define CACHE_CNTRL_PAGE        0x08    /* cache contorol parmeter          */
#define PERPHRA_PAGE            0x09    /* peripheral parameter             */
#define CNTRL_MODE_PAGE         0x0a    /* contorol mode parameter          */
#define MEDIA_TYPE_PAGE         0x0b    /* medium type parameter            */
#define NOTCH_PERTITION_PAGE    0x0c    /* notch/partition parameter        */

/*---- page code length ----*/
#define RWERR_RECVRY_LEN        0x0a
#define DECNCT_RECNCT_LEN       0x0e
#define FORMT_LEN               0x16
#define VRFYERR_RECVRY_LEN      0x0a
#define CACHE_CNTRL_LEN         0x0a
#define PERPHRAL_DEV_LEN        0x06
#define CNTRL_MODE_LEN          0x06
#define MEDIA_TYPE_LEN          0x06
#define NOTCH_PERTITION_LEN     0x16

/*---- read/write error recovery parameter ----*/
#define AWRE                0x00    /* Automatic Wwrite Reallocation Enable */
#define ARRE                0x00    /* Automatic Read Reallocation Enabke   */
#define TB                  0x00    /* Trahsfer Block                       */
#define RC                  0x00    /* Read Continuous                      */
#define EER                 0x00    /* Enable Early Recovery                */
#define PER                 0x00    /* Post Error                           */
#define DTE                 0x00    /* Disable Transfer on Error            */
#define DCR                 0x01    /* Disable Correction                   */
#define R_RETRY_MAX         1
#define DAT_CRECT_LEN       0
#define HEAD_OFFSET         0
#define DAT_STROV_OFFSET    0
#define W_RETRY_MAX         1
#define RECVRY_TIME         100

/*---- Desconnect/Reconnect parameter ----*/
#define BUF_FUL_RATIO       0x80
#define BUF_EMP_RATIO       0x80
#define BUS_INACT_LIM       0x00
#define DESCNCT_TIME_LIM    0x00
#define CNCT_TIME_LIM       0x00
#define BURST_SIZE          0x00
#define DTDC                0x00

/*---- Desconnect/Reconnect parameter ----*/
#define VERFY_RETRY_MAX     1
#define VERFY_RETRY_LEN     0
#define VERFY_RCVRY_LIM     100

#define DRCT_ACCESS_DEV         0x00    /* Direct Access Device             */
#define RMB_DISC                0x80    /* The Device is Removable          */
#define ANSI_SCSI2              0x02    /* Based on ANSI-SCSI2              */
#define SCSI                    0x00    /* Interface ID                     */

#define USBOTG_DEVICE_MAX_SECTOR (USB_OTG_BUF_SIZE>>9)


enum 
{
    kStopUnitCondition          = 0x0001,
    kMediaChangeCondition       = 0x0002,
    kWriteProtectCondition      = 0x0004,
    kInvalidCommandCondition    = 0x0008,

    kMediaRemovalPreventedCondition = 0x0010,
    kBecomingReadyCondition         = 0x0020,
    kMediaNotPresentCondition       = 0x0040,
    //kCheckConditionMask         = 0xFFFF
};


/*
// Structure declarations
*/

/////////////////////////////////////////////////
// SCSI COMMAND STRUCTURE
/////////////////////////////////////////////////
/*
typedef struct {
    BYTE   OperCode;
    BYTE   Reserved[4];
    BYTE   CtlByte;
} USB_TEST_UNIT_READY;
*/
typedef struct {
    BYTE   OperCode;
    BYTE   Lun;
    BYTE   Reserved[2];
    BYTE   AllocLength;
    BYTE   CtlByte;
    BYTE   Reserved1[2];
} USB_REQUEST_SENSE;

typedef struct {
    BYTE   OperCode;
    BYTE   Lun;             // the first 3 bits is LUN
    BYTE   Reserved1[2];
    BYTE   AllocLength;
    BYTE   Reserved2[3];
} USB_INQUIRY;

/*
typedef struct {
    BYTE   OperCode;
    BYTE   PfSp;
    BYTE   Reserved[2];
    BYTE   ParListLength;
    BYTE   CtlByte;
} USB_MODE_SELECT6;

typedef struct {
    BYTE   OperCode;
    BYTE   Dbd;
    BYTE   PcPgCode;
    BYTE   Reserved;
    BYTE   AllocLength;
    BYTE   CtlByte;
} USB_MODE_SENSE6;
*/

typedef struct {
    BYTE   OperCode;
    BYTE   Immed;
    BYTE   Reserved1[2];
    BYTE   LoEjStart;
    BYTE   CtlByte;
    BYTE   Reserved2[2];
} USB_START_STOP_UNIT;

typedef struct {
    BYTE   OperCode;
    BYTE   Lun;
    BYTE   Reserved1[2];
    BYTE   Prevent;
    BYTE   CtlByte;
    BYTE   Reserved2[2];
} USB_MEDIUM_REMOVAL;
/*
typedef struct {
    BYTE   OperCode;
    BYTE   RelAdr;
    BYTE   BlkAddr[4];
    BYTE   Reserved[2];
    BYTE   Pmi;
    BYTE   CtlByte;
} USB_READ_CAPACITY;
*/
typedef struct {
    BYTE   OperCode;
    BYTE   DFRel;
    BYTE   BlkAddr[4];
    BYTE   Reserved1;
    BYTE   BlkCnt[2];
    BYTE   CtlByte;
    BYTE   Reserved2[2];
} USB_READ10;

typedef struct {
    BYTE   OperCode;
    BYTE   DFRel;
    BYTE   BlkAddr[4];
    BYTE   Reserved1;
    BYTE   BlkCnt[2];
    BYTE   CtlByte;
    BYTE   Reserved2[2];
} USB_WRITE10;

typedef struct {
    BYTE   OperCode;
    BYTE   DBRel;
    BYTE   BlkAddr[4];
    BYTE   Reserved1;
    BYTE   BlkCnt[2];
    BYTE   CtlByte;
    BYTE   Reserved2[2];
} USB_VERIFY10;

typedef struct {
    BYTE   OperCode;
    BYTE   Dbd;
    BYTE   PcPgCode;
    BYTE   Reserved1[4];
    BYTE   AllocLength[2];
    BYTE   CtlByte;
    BYTE   Reserved2[2];
} USB_MODE_SENSE10;

typedef struct {
    BYTE   PageCode;                    // Code of the Page       
    BYTE   PageLen;                     // Length of the Page     
    BYTE   RecvryWay;                   // means of recovery      
    BYTE   RretryCnt;                   // read retry counter     
    BYTE   CorctLen;                    // length of data correct 
    BYTE   HeadOfst;                    // head offset            
    BYTE   DatStrbOfst;                 // data strobe offset     
    BYTE   Reserved1;                   // Reserved area          
    BYTE   WretryCnt;                   // write retry counter    
    BYTE   Reserved2;                   // Reserved area          
    BYTE   TimeLmt[2];                  // recovery time limit    
} RWERR_RECVRY;

typedef struct {
    BYTE   PageCode;                    // Code of the Page                 
    BYTE   PageLen;                     // Length of the Page               
    BYTE   BufFulRatio;                 // buffer full ratio                
    BYTE   BufEmpRatio;                 // buffer empty ratio               
    BYTE   BusInactLim[2];              // bus inactivity limit             
    BYTE   DiscnctTmLim[2];             // disconnect time limit           
    BYTE   CnctTmLim[2];                // connect time limit               
    BYTE   MaxBrstSize[2];              // max burst size                   
    BYTE   dtdc;                        // DTDC                             
                                        //(Data Transfer Disconnect Control)
    BYTE   Reserved1;                   // Reserved area                    
    BYTE   Reserved2;                   // Reserved area                    
    BYTE   Reserved3;                   // Reserved area                    
} DECNCT_RECNCT;


typedef struct {
    BYTE   PageCode;            // Code of the Page                  
    BYTE   PageLen;             // Length of the Page               
    BYTE   TrckNum[2];          // truck number per zone            
    BYTE   ChgSecNum[2];        // change secter number per zone    
    BYTE   ChgTrckNum[2];       // change truck number per zone     
    BYTE   ChgTrckLogNum[2];
                                // change truck number logical unit 
    BYTE   SecNum[2];           // secter number per zone           
    BYTE   DatNum[2];           // deta number per physical sector  
    BYTE   intrleav[2];         // interleave factor                
    BYTE   TrckSkew[2];         // truck skew factor                
    BYTE   SrndrSkew[2];        // serinder skew factor             
    BYTE   charcter;            // characteristic of medium         
    BYTE   Reserved1;           // Reserved area                    
    BYTE   Reserved2;           // Reserved area                    
    BYTE   Reserved3;           // Reserved area                    
} FORMT;

// Verify Error Recovery Parameter
typedef struct {
    BYTE   PageCode;            // Code of the Page          
    BYTE   PageLen;             // Length of the Page        
    BYTE   RecvryWay;           // means of recovery         
    BYTE   RetryCnt;            // read retry counter        
    BYTE   CorctLen;            // length of data correct    
    BYTE   Reserved1;           // Reserved area             
    BYTE   Reserved2;           // Reserved area             
    BYTE   Reserved3;           // Reserved area             
    BYTE   Reserved4;           // Reserved area             
    BYTE   Reserved5;           // Reserved area             
    BYTE   TimeLmt[2];          // recovery time limit       
} VRFYERR_RECVRY;

// Cache Control Parameter
typedef struct {
    BYTE   PageCode;            // Code of the Page                 
    BYTE   PageLen;             // Length of the Page               
    BYTE   CntrlWay;            // means of cache control           
    BYTE   Priority;            // keeping priority of r/wdeta      
    BYTE   DsablPrftchLen[2];   // disable prefetch transfer length 
    BYTE   MimPrftch[2];        // minimum prefetch                 
    BYTE   MaxPrftch[2];        // maximum prefetch                 
    BYTE   MaxPrftchCelng[2];   // maximum prefetch ceiling         
} CACHE_CNTRL;

// Peripheral Device Parameter
typedef struct {
    BYTE   PageCode;            // Code of the Page    
    BYTE   PageLen;             // Length of the Page  
    BYTE   IntrfaceID[2];       // interface ID        
    BYTE   Reserved1;           // Reserved area       
    BYTE   Reserved2;           // Reserved area       
    BYTE   Reserved3;           // Reserved area       
    BYTE   Reserved4;           // Reserved area       
} PRPHRL_DEV;

// Contorol Mode Parameter 
typedef struct {
    BYTE   PageCode;            // Code of the Page                
    BYTE   PageLen;             // Length of the Page              
    BYTE   rlec;                // RLEC                            
                                // (Report Log Exception Condition)
    BYTE   QueData;             // Queue algorithm & QErr & DQue   
    BYTE   TrgtStat;            // status of target                
                                // byte7 : EECA                    
                                // byte6-3 : reserved              
                                // byte2 : RAENP                   
                                // byte1 : EAENP                   
    BYTE   Reserved;            // reserved area                   
    BYTE   RdyAenHldTime[2];    // ready AEN hold off time         
} CNTRL_MODE;

// Medium Type Parameter 
typedef struct {
    BYTE   PageCode;           // Code of the Page       
    BYTE   PageLen;            // Length of the Page     
    BYTE   Reserved1;          // Reserved area          
    BYTE   Reserved2;          // Reserved area          
    BYTE   MediaType1;         // supported mudium type  
    BYTE   MediaType2;         // supported mudium type  
    BYTE   MediaType3;         // supported mudium type  
    BYTE   MediaType4;         // supported mudium type  
} MEDIA_TYPE;

// Notch Pertition Prameter 
typedef struct {
    BYTE   PageCode;            // Code of the Page                 
    BYTE   PageLen;             // Length of the Page               
    BYTE   NdLpn;               // notched device & logical/physcal 
    BYTE   Reserved1;           // Reserved area                    
    BYTE   MaxNtchNum[2];       // maxmum notch number              
    BYTE   ActvNtch[2];         // active notch                     
    BYTE   NtchStrtPos[4];      // position of notch start          
    BYTE   NtchEndPos[4];       // position of notch end            
    BYTE   NtchParaPage[8];     // Notch parameter page             
} NOTCH_PERTITION;
/*-----------------------------------
    Read Capacity
-----------------------------------*/

typedef struct {            // Little endian
    DWORD   LastLBA;        // Last logical block address
    DWORD   BlockLength;    // Block length in bytes(MSB first for command return )
    DWORD   BLenLSB;        // LSB first - block length
    BYTE    nClass;         // CFCard - '0'
                            // InternalFlash - '1'
    WORD    StsFlag;        // StsFlag = MediumReady '2'
                            // StsFlag = MediumError '1'
                            // StsFlag = MediumEmpty '4'
                            // StsFlag = MediumBusy  '8' ,opposite - MediaIdle
    BYTE    bReserved;
} MediaInfoStr;

typedef struct {                // Little endian
    BYTE    DeviceType;          // Peripheral device type(bit0~4)
    BYTE    RMB;                // Removable media - bit7='1'
    BYTE	Version;            // Bit0~2 : ANSI Version '0x00'
                                // Bit3~5 : ECMA Version '0x00' for UFI device
                                // Bit6~7 : ISO Version  '0x00' for UFI device
    BYTE    RespDataFormate;    // Bit0~3 : '0x01' for UFI device, (0x00 for HD device ?)
    BYTE    AdditionLen;        // '0x1F' for UFI device, (0x00 for HD device ? )
    BYTE    Reserved[3];        // Reserved
    BYTE    VendorInf[8];       // Vendor information(byte 8~15)
    BYTE    ProductId[16];      // Product identification(byte 16~31)
    BYTE    ProductRev[4];      // Product revision level(4bytes: n.nn)
} DeviceInformationStr;

typedef struct {
    BYTE    Reserved[3] ;
    BYTE    CapacityListLength ;
    DWORD   LastLBA;        // Last logical block address
    DWORD   BlockLength;	// Block length in bytes(MSB first for command return )
} CapacityDescriptor ;	     

static BYTE gpVendorInf[] = GP_VENDER_INF;             // 8 letters
//BYTE    gpProductId[] = GP_PRODUCT_ID;        // 16 letters
static BYTE gpProductRev[] = GP_PRODUCT_REV;           // 4 letters n.nn

static BYTE gpProductId_Lun_0[] = GP_PRODUCT_ID_LUN_0; // 16 letters
static BYTE gpProductId_Lun_1[] = GP_PRODUCT_ID_LUN_1; // 16 letters
static BYTE gpProductId_Lun_2[] = GP_PRODUCT_ID_LUN_2; // 16 letters
static BYTE gpProductId_Lun_3[] = GP_PRODUCT_ID_LUN_3; // 16 letters
static BYTE gpProductId_Lun_4[] = GP_PRODUCT_ID_LUN_4; // 16 letters
static BYTE gpProductId_Lun_5[] = GP_PRODUCT_ID_LUN_5; // 16 letters
static BYTE gpProductId_Lun_6[] = GP_PRODUCT_ID_LUN_6; // 16 letters

/*
// Macro declarations
*/


/*
// Static function prototype
*/
static void USBDeviceInfSetup ( WHICH_OTG eWhichOtg );
static WORD ScsiCommandProcess ( BYTE **hData,
                                 BYTE *pCmd,
                                 BYTE lun,
                                 DWORD *pData_residue,
                                 DWORD expect_tx_length,
                                 WHICH_OTG eWhichOtg);
static WORD ReadDataFromDrive ( USB_READ10 *pRead,
                                BYTE lun,
                                DWORD sent_data_length,
                                WHICH_OTG eWhichOtg);
static WORD WriteDataIntoDrive( USB_WRITE10 *pWrite, 
                                BYTE lun,
                                DWORD lba,
                                WORD cnt,
                                WHICH_OTG eWhichOtg);
/*
// Definition of internal functions
*/
//#define PROC_SIZE (1024*1024)
void UsbMsdcInit(WHICH_OTG eWhichOtg)
{
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
	PUSB_DEVICE_DESC psUsbDevDesc = (PUSB_DEVICE_DESC)UsbOtgDevDescGet(eWhichOtg);
	PST_USB_OTG_DEVICE psUsbOtgDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet (eWhichOtg);
    BYTE i=0;

    MP_DEBUG("USBOTG%d MsdcInit", eWhichOtg);

    psUsbDevDesc->dwBulkTxRxCounter = 0;
    psUsbDevDesc->pbBulkTxRxData = (BYTE *)((DWORD)UsbOtgBufferGet(eWhichOtg)| 0xa0000000);//((DWORD)g_USBBUF | 0xa0000000);
    //gpBulkTxRxData = 0;
    psUsbOtgDev->eUsbTransactionState = STATE_CBW;
    psUsbDevMsdc->psCbw = (PCBW)((DWORD)(psUsbDevMsdc->psCbw)|0xa0000000);
    psUsbDevMsdc->psCsw = (PCSW)((DWORD)(psUsbDevMsdc->psCsw)| 0xa0000000);
    memset((BYTE*)psUsbDevMsdc->psCbw, 0, CBW_LEN);
    memset((BYTE*)psUsbDevMsdc->psCsw, 0, CSW_LEN);
    psUsbDevMsdc->psCsw->u32Signature = CSW_SIGNATE;	// Init u32Signature
    USBDeviceInfSetup(eWhichOtg);
    for (i = 0; i < MSDC_MAX_LUN; i++)
    {
        psUsbDevMsdc->boIsMediumRemoval[i] = FALSE;
        psUsbDevMsdc->wCheckCondition[i] = 0;
		//g_bLunDriveID[i] = Mcard_CurLunGetCardID(i);
        psUsbDevMsdc->boMediaNotPresent[i] = FALSE;
        psUsbDevMsdc->boIsPrevent[i] = FALSE;
    }
}

BOOL MsdcCheckIfReadWriteData(WHICH_OTG eWhichOtg)
{
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);

    return psUsbDevMsdc->boIsReadWriteData;
}

/*
// Definition of local functions 
*/
TRANSACTION_STATE eUsbMsDataInStall(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES  psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
    TRANSACTION_STATE eState = STATE_CSW;

    if ((psUsbDevMsdc->psCbw->u8CB[0] == SCSI_READ_12)||
        (psUsbDevMsdc->psCbw->u8CB[0] == SCSI_READ_10)||
        (psUsbDevMsdc->psCbw->u8CB[0] == SCSI_INQUIRY)||
        (psUsbDevMsdc->psCbw->u8CB[0] == SCSI_READ_CAPACITY)||
#if USBOTG_DEVICE_MSDC_TECO
        (psUsbDevMsdc->psCbw->u8CB[0] == SCSI_VENDOR_TECO_CMD)||
#endif
        (psUsbDevMsdc->psCbw->u8CB[0] == SCSI_READ_FORMAT_CAPACITIES))
    {
        MP_DEBUG("eUsbMsDataInStall:Stall EP1");
        mUsbOtgEPinStallSet(EP1);
        return eState;
    }

    MP_DEBUG("eUsbMsDataInStall:STATE_CBW");
    eState = STATE_CBW;
    
    return eState;
}

TRANSACTION_STATE eUsbMsDataOutStall(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    TRANSACTION_STATE eState = STATE_DATA_OUT_STALL;

    if (mUsbOtgEPoutStallST(EP2))
    {
        eState = STATE_DATA_OUT_STALL;
        MP_DEBUG("eUMDOS_SDOS");
    }
    else
    {
        eState = STATE_CSW;
        MP_DEBUG("eUMDOS_CSW");
    }

    return eState;
}
///////////////////////////////////////////////////////////////////////////////
//		eUsbMsSendCsw()
//		Description: Send out the CSW structure to PC
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
TRANSACTION_STATE eUsbMsSendCsw(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES  psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
    TRANSACTION_STATE eState = STATE_CBW;

    // Send CSW to F0 via DBUS;
    // howCSW();
    if ((psUsbDevMsdc->psCbw->u8CB[0] == SCSI_READ_12)||
        (psUsbDevMsdc->psCbw->u8CB[0] == SCSI_READ_10)||
        (psUsbDevMsdc->psCbw->u8CB[0] == SCSI_INQUIRY)||
        (psUsbDevMsdc->psCbw->u8CB[0] == SCSI_READ_CAPACITY)||
#if USBOTG_DEVICE_MSDC_TECO
        (psUsbDevMsdc->psCbw->u8CB[0] == SCSI_VENDOR_TECO_CMD)||
#endif
        (psUsbDevMsdc->psCbw->u8CB[0] == SCSI_READ_FORMAT_CAPACITIES))
    {
        if (mUsbOtgEPinStallST(EP1) == TRUE)
        {
            eState = STATE_CSW;
            return eState;
        }
    }
    
    psUsbDevMsdc->psCsw->u32DataResidue = BYTE_SWAP_OF_DWORD(psUsbDevMsdc->psCsw->u32DataResidue);	
    BOOL  ret = TRUE;
    //ret = vOTGFxWr((BYTE *)(gpCsw), CSW_LEN);
    ret = bOTGCxFxWrRd(FOTG200_DMA2FIFO0,DIRECTION_IN,(BYTE *)(psUsbDevMsdc->psCsw),CSW_LEN,eWhichOtg);
    if (ret == FALSE)
    {
        mpDebugPrint("2Disconnect while reading");
    }

	mUsbOtgFIFODone(FIFO_BULK_IN);

    return eState;
}

static void USBDeviceInfSetup( WHICH_OTG eWhichOtg )
{    
    PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);

    psUsbDevMsdc->sDeviceInf.DeviceType = 0x0;
    psUsbDevMsdc->sDeviceInf.RMB = 0x80;
    psUsbDevMsdc->sDeviceInf.Version = 0x0;    

    psUsbDevMsdc->sDeviceInf.RespDataFormate = 0x1; // 2 - SCSI
    psUsbDevMsdc->sDeviceInf.AdditionLen = 0x1f;    // 0x20

    memcpy(&psUsbDevMsdc->sDeviceInf.VendorInf[0],    &gpVendorInf[0],    sizeof(psUsbDevMsdc->sDeviceInf.VendorInf));
    //memcpy(&gDeviceInf.ProductId[0],    &gpProductId[0],    sizeof(gDeviceInf.ProductId));
    memcpy(&psUsbDevMsdc->sDeviceInf.ProductRev[0],   &gpProductRev[0],   sizeof(psUsbDevMsdc->sDeviceInf.ProductRev));
}


///////////////////////////////////////////////////////////////////////////////
//		eUsbProcessCbw()
//		Description: Process the CBW
//		input: none
//		output: TRANSACTION_STATE
//      note: have knew the data lenght is 31
///////////////////////////////////////////////////////////////////////////////
TRANSACTION_STATE eUsbProcessCbw(WORD u16FIFOByteCount, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
	PUSB_DEVICE_DESC psUsbDevDesc = (PUSB_DEVICE_DESC)UsbOtgDevDescGet(eWhichOtg);
    DWORD   u16count = 0;
    TRANSACTION_STATE eState = STATE_IDLE;	
    BOOL ret = TRUE;	
    WORD wMaxPacketSize = 0;

    if (mUsbOtgHighSpeedST())
        wMaxPacketSize = HS_MAX_PACKET_SIZE;
    else
        wMaxPacketSize = FS_MAX_PACKET_SIZE;

	//ret = vOTGFxRd((BYTE *)(gpCbw), u16FIFOByteCount);  //calvin diff
    ret = bOTGCxFxWrRd(FOTG200_DMA2FIFO1,DIRECTION_OUT,(BYTE *)(psUsbDevMsdc->psCbw),u16FIFOByteCount,eWhichOtg);
    if (ret == FALSE)
        return STATE_CSW;
	
    if(psUsbDevMsdc->psCbw->u32Signature != CBW_SIGNATE)
    {
        MP_DEBUG("not CBW");
        if (u16FIFOByteCount == wMaxPacketSize) 
        {
            eState = STATE_CBW;
        }
        else
        {
            eState = STATE_DATA_OUT_STALL;
        }
    }
	else
	{
        DWORD expect_tx_length = 0;
        expect_tx_length = BYTE_SWAP_OF_DWORD(psUsbDevMsdc->psCbw->u32DataTransferLength);
        psUsbDevMsdc->psCsw->u8Status = ScsiCommandProcess (	&psUsbDevDesc->pbBulkTxRxData,
                                                (BYTE*)&(psUsbDevMsdc->psCbw->u8CB[0]),
                                                (BYTE)psUsbDevMsdc->psCbw->u8LUN,
                                                (DWORD*)&psUsbDevMsdc->psCsw->u32DataResidue, 
                                                expect_tx_length,
                                                eWhichOtg);
		psUsbDevDesc->pbBulkTxRxData = (BYTE*)((DWORD)psUsbDevDesc->pbBulkTxRxData | 0xa0000000);
		psUsbDevMsdc->psCsw->u32Tag = psUsbDevMsdc->psCbw->u32Tag; // pass Tag from CBW to CSW	

        if ((psUsbDevMsdc->psCsw->u8Status == STS_CMD_FAILED)||(psUsbDevMsdc->psCsw->u8Status == STS_PHASE_ERR))
        {
            BYTE    cmd;
            cmd = psUsbDevMsdc->psCbw->u8CB[0];

            switch (cmd)
            {
                case SCSI_READ_12:
                case SCSI_INQUIRY:
                case SCSI_READ_CAPACITY:
                case SCSI_READ_FORMAT_CAPACITIES:
#if USBOTG_DEVICE_MSDC_TECO
                case SCSI_VENDOR_TECO_CMD:
#endif
                {
                    eState = STATE_DATA_IN_STALL;
            	}
                break;
                
                case SCSI_WRITE_12:
                {
                    eState = STATE_DATA_OUT_STALL;
                }
                break;
                
                case SCSI_READ_10:
                {
                    if ((psUsbDevMsdc->psCsw->u32DataResidue != expect_tx_length)||(expect_tx_length == 0))
                    {
                        if(psUsbDevMsdc->psCsw->u32DataResidue < expect_tx_length)
                        {
                            psUsbDevMsdc->psCsw->u32DataResidue = expect_tx_length - psUsbDevMsdc->psCsw->u32DataResidue;
                        }
                        else if (psUsbDevMsdc->psCsw->u32DataResidue > expect_tx_length)
                        {
                            psUsbDevMsdc->psCsw->u32DataResidue = psUsbDevMsdc->psCsw->u32DataResidue - expect_tx_length;
                        }
                        else
                        {
                            psUsbDevMsdc->psCsw->u32DataResidue = expect_tx_length;
                        }
                    }
                            
                    eState = STATE_DATA_IN_STALL;
                }
                break;
                
                case SCSI_WRITE_10:
                {
                    if((psUsbDevMsdc->psCsw->u32DataResidue < expect_tx_length)||(psUsbDevMsdc->psCsw->u8Status == STS_PHASE_ERR))
                    {
                        eState = STATE_DATA_OUT_STALL;
                    }
                    else if (psUsbDevMsdc->psCbw->u8Flags == 0x00)
                    {
                        eState = STATE_CB_DATA_OUT;
                    }
                }
                break;

                case SCSI_TEST_UNIT_READY:
                {
                    if (psUsbDevMsdc->psCsw->u32DataResidue > 0)
                    {
                        eState = STATE_DATA_OUT_STALL;
                    }
                    else
                    {
                        eState = STATE_CSW;
                    }
                }
                break;
                
                default:
                {
                    eState = STATE_CSW;
                }
                break;
            }
            
            return eState;
        }		
		
        if (psUsbDevMsdc->psCsw->u8Status == STS_STALL)
        {
            psUsbDevMsdc->psCsw->u8Status = STS_CMD_FAILED;
            eState = STATE_CSW;
        }
        else if (psUsbDevMsdc->psCsw->u32DataResidue == 0)
        {
            eState = STATE_CSW;
        }
        else if (psUsbDevMsdc->psCbw->u8Flags == 0x00)
        {
            eState = STATE_CB_DATA_OUT;
        }
        else
        {
            eState = STATE_CB_DATA_IN;
        }
	}
	return eState;
}

static WORD ReadDataFromDrive(USB_READ10 *pRead, BYTE lun, DWORD sent_data_length, WHICH_OTG eWhichOtg)
{
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
	PUSB_DEVICE_DESC psUsbDevDesc = (PUSB_DEVICE_DESC)UsbOtgDevDescGet(eWhichOtg);
    DWORD	lba;
    WORD    ret = STS_GOOD;
    WORD	cnt;
    SWORD   swRet = 0;
    BYTE    bDriveId; // From Drive

    lba = BYTE_TO_DWORD(pRead->BlkAddr[0],
                        pRead->BlkAddr[1],
                        pRead->BlkAddr[2],
                        pRead->BlkAddr[3]
                        );
    cnt = BYTE_TO_WORD(pRead->BlkCnt[0], pRead->BlkCnt[1]);

    if (sent_data_length > 0)
    {
        lba += (sent_data_length>>EXP_VAL);//change to data sector count
        cnt -= (sent_data_length>>EXP_VAL);//change to data sector count
        //mpDebugPrint("change:L = %d;C = %d", lba, cnt);
    }
    
    bDriveId = SystemDriveIdGetByLun(lun);
    if ((bDriveId != 0) && SystemCardPresentCheck(bDriveId))
    {
        DWORD sector_read = 0;
        if (cnt > USBOTG_DEVICE_MAX_SECTOR)
            sector_read = USBOTG_DEVICE_MAX_SECTOR;
        else
            sector_read = cnt;

        psUsbDevDesc->dwBulkTxRxCounter = sector_read<<EXP_VAL;//change to byte count
        
        //swRet = Mcard_DeviceRead(DriveGet(bDeviceId), (BYTE*)UsbOtgBufferGet(eWhichOtg), lba, sector_read);
        swRet = SystemDriveReadByLun(lun, (BYTE*)UsbOtgBufferGet(eWhichOtg), lba, sector_read);
        if (swRet != 0)
        {
            MP_DEBUG("read fail!!");
            ret = STS_CMD_FAILED;
        }
    }
    else
    {
        psUsbDevMsdc->wCheckCondition[lun] = kMediaNotPresentCondition;
        ret = STS_CMD_FAILED;
    }

    return ret;
}

static WORD WriteDataIntoDrive(USB_WRITE10 *pWrite, BYTE lun, DWORD lba, WORD cnt, WHICH_OTG eWhichOtg)
{
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
	PUSB_DEVICE_DESC psUsbDevDesc = (PUSB_DEVICE_DESC)UsbOtgDevDescGet(eWhichOtg);
    WORD    ret = STS_GOOD;
    SWORD   swRet = 0;
    BYTE    bDriveId; // From Drive

    bDriveId = SystemDriveIdGetByLun(lun);
    if ((bDriveId != 0) && SystemCardPresentCheck(bDriveId))     
    {
        //swRet = Mcard_DeviceWrite(DriveGet(bDeviceId), (BYTE*)psUsbDevDesc->pbBulkTxRxData, lba, cnt);
        swRet = SystemDriveWriteByLun(lun, (BYTE*)psUsbDevDesc->pbBulkTxRxData, lba, cnt);
        if(swRet != 0)  // write fail
        {
              MP_DEBUG("write fail !");
              psUsbDevMsdc->psCsw->u8Status = STS_CMD_FAILED;
              ret = STS_CMD_FAILED;
        }
    }
    else
    {
        psUsbDevMsdc->wCheckCondition[lun] = kMediaNotPresentCondition;
        psUsbDevMsdc->psCsw->u8Status = STS_CMD_FAILED;
        ret = STS_CMD_FAILED;
    }                 

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
//      eUsbMsDataOut()
//		Description: Process the data output stage
//		input: none
//		output: TRANSACTION_STATE
///////////////////////////////////////////////////////////////////////////////
TRANSACTION_STATE eUsbMsDataOut(WORD u16FIFOByteCount, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
	PUSB_DEVICE_DESC psUsbDevDesc = (PUSB_DEVICE_DESC)UsbOtgDevDescGet(eWhichOtg);
    DWORD lba = 0;
    DWORD   chk_err = 0;
    DWORD   len = 0;
    WORD    byteCount;
    WORD  cnt = 0;
    TRANSACTION_STATE eState = STATE_CSW;
    SWORD   err = 0;
    volatile BYTE bIntLevel2;
    BYTE    lun = 0;
    BOOL ret = TRUE;
    BYTE    cmd;
 
    cmd = psUsbDevMsdc->psCbw->u8CB[0]; 
    
    #if USBOTG_DEVICE_MSDC_TECO
    if(cmd == SCSI_VENDOR_TECO_CMD)
    {
        VendorTecoDataOut(psUsbDevDesc->pbBulkTxRxData, BYTE_SWAP_OF_DWORD(psUsbDevMsdc->psCbw->u32DataTransferLength), psUsbDevMsdc->psCbw->u8LUN , eWhichOtg);
        return STATE_CSW;
    }
    #endif  


    psUsbDevMsdc->boIsReadWriteData = TRUE;
    // Get byteCount Bytes data from F1 via DBUS;

    ret = bOTGCxFxWrRd(FOTG200_DMA2FIFO1,DIRECTION_OUT,(BYTE*)((DWORD)psUsbDevDesc->pbBulkTxRxData + psUsbDevMsdc->dwRxByteCnt),u16FIFOByteCount,eWhichOtg);
    if (ret == FALSE)
        return eState;  

    #if USBOTG_DEVICE_PROTECTION
    if(cmd == SCSI_VENDOR_PROTECTION_CMD)
    {
        VendorProtectionData((USB_PROTECTION_DATA*)psUsbDevDesc->pbBulkTxRxData, BYTE_SWAP_OF_DWORD(psUsbDevMsdc->psCbw->u32DataTransferLength), psUsbDevMsdc->psCbw->u8LUN , eWhichOtg);
        return STATE_CSW;
    }
    #endif    

    psUsbDevMsdc->dwRxByteCnt += u16FIFOByteCount;
    psUsbDevMsdc->psCsw->u32DataResidue -= u16FIFOByteCount;
    //mpDebugPrint("1W:r = %d; t = %d",gpCsw->u32DataResidue, psUsbDevMsdc->dwRxByteCnt);

    
    if (cmd == SCSI_WRITE_10)
    {
        USB_WRITE10 *pWrite;
        pWrite = (USB_WRITE10*)&(psUsbDevMsdc->psCbw->u8CB[0]);                   
        lba = BYTE_TO_DWORD(pWrite->BlkAddr[0],
                            pWrite->BlkAddr[1],
                            pWrite->BlkAddr[2],
                            pWrite->BlkAddr[3]
                            );
        cnt = BYTE_TO_WORD(pWrite->BlkCnt[0], pWrite->BlkCnt[1]);
        if (psUsbDevMsdc->psCsw->u32DataResidue == 0)
        {
            WORD ret_val = STS_GOOD;
            ret_val = WriteDataIntoDrive ((USB_WRITE10*)&(psUsbDevMsdc->psCbw->u8CB[0]),
                                           psUsbDevMsdc->psCbw->u8LUN,
                                           lba,
                                           cnt,
                                           eWhichOtg);
            if (ret_val != STS_GOOD)
            {
                mpDebugPrint("error in WriteDataIntoDrive");
            }
            
            psUsbDevMsdc->boIsReadWriteData = FALSE;
            psUsbDevMsdc->dwRxByteCnt = 0;
            return eState;
        }
        
    }
    
    while(psUsbDevMsdc->psCsw->u32DataResidue > 0)
    {
        if (UsbOtgDeviceCheckIdPin(eWhichOtg) == FALSE)
        {
            MP_DEBUG("1Disconnect while writting");
            eState = STATE_IDLE;
            break;
        }
        else
        {
                bIntLevel2 = mUsbOtgIntSrc1Rd() &~ mUsbOtgIntSrc1MaskRd();
                if ((bIntLevel2 & BIT3) || (bIntLevel2 & BIT2))
                {
                    BOOL ret = TRUE;
                    byteCount = mUsbOtgFIFOOutByteCount(FIFO_BULK_OUT);
                    // Get byteCount Bytes data from F1 via DBUS;
                ret = bOTGCxFxWrRd(FOTG200_DMA2FIFO1,DIRECTION_OUT,(BYTE*)((DWORD)psUsbDevDesc->pbBulkTxRxData + psUsbDevMsdc->dwRxByteCnt),byteCount,eWhichOtg);
                    if (ret == FALSE)
                        break;

                //gBulkTxRxCounter += byteCount;
                psUsbDevMsdc->dwRxByteCnt += byteCount;
                    psUsbDevMsdc->psCsw->u32DataResidue -= byteCount;
                }

            if (cmd == SCSI_WRITE_10)
            {
                if (((psUsbDevMsdc->dwRxByteCnt == USB_OTG_BUF_SIZE) && (psUsbDevMsdc->psCsw->u32DataResidue > 0)) ||
                    (psUsbDevMsdc->psCsw->u32DataResidue == 0))
                { // write data to Drive
                    if ((psUsbDevMsdc->dwRxByteCnt == USB_OTG_BUF_SIZE) && (psUsbDevMsdc->psCsw->u32DataResidue > 0))
                    {
                        psUsbDevMsdc->dwWriteLba = lba;
                        psUsbDevMsdc->wSectorCnt = USBOTG_DEVICE_MAX_SECTOR;
                        //mpDebugPrint("W:over");
                    }
                    else if (psUsbDevMsdc->psCsw->u32DataResidue == 0)
                    {
                        psUsbDevMsdc->dwWriteLba = lba;
                        psUsbDevMsdc->wSectorCnt = cnt;
                        //mpDebugPrint("W:end");
                    }
                    else
                    {
                        mpDebugPrint("W:???");
                    }
                        
                    WORD ret_val = STS_GOOD;
                    ret_val = WriteDataIntoDrive ((USB_WRITE10*)&(psUsbDevMsdc->psCbw->u8CB[0]),
                                                   psUsbDevMsdc->psCbw->u8LUN,
                                                   psUsbDevMsdc->dwWriteLba,
                                                   psUsbDevMsdc->wSectorCnt,
                                                   eWhichOtg);
                    if (ret_val != STS_GOOD)
                    {
                        mpDebugPrint("error in WriteDataIntoDrive");
                        break;
                    }

                    if ((psUsbDevMsdc->dwRxByteCnt == USB_OTG_BUF_SIZE) && (psUsbDevMsdc->psCsw->u32DataResidue > 0))
                    {
                        lba += USBOTG_DEVICE_MAX_SECTOR;
                        cnt -= USBOTG_DEVICE_MAX_SECTOR;
                    }
                    else
                    {
                        psUsbDevMsdc->dwWriteLba = 0;
                        psUsbDevMsdc->wSectorCnt = 0;
                    }
                    psUsbDevMsdc->dwRxByteCnt = 0;
                }
            }
    	}
    }
    
    psUsbDevMsdc->boIsReadWriteData = FALSE;
    //gBulkTxRxCounter = 0;
    psUsbDevMsdc->dwRxByteCnt = 0;
    return eState;
}

///////////////////////////////////////////////////////////////////////////////
//      UsbMsDataIn()
//		Description: Process the data intput stage
//		input: none
//		output: TRANSACTION_STATE
//////////////////////////////{
/////////////////////////////////////////////////
TRANSACTION_STATE eUsbMsDataIn(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
	PUSB_DEVICE_DESC psUsbDevDesc = (PUSB_DEVICE_DESC)UsbOtgDevDescGet(eWhichOtg);
    BYTE* pData;
    WORD u16count;
    TRANSACTION_STATE eState = STATE_CSW;
    BOOL  ret = TRUE;

    psUsbDevMsdc->boIsReadWriteData = TRUE;
    while(psUsbDevMsdc->psCsw->u32DataResidue > 0)
    {
        if (UsbOtgDeviceCheckIdPin(eWhichOtg) == FALSE)
        {
            MP_DEBUG("1Disconnect while reading");
            eState = STATE_IDLE;
            break;
        }
        //else if (mUsbFIFOEmptyByte0Rd(FIFO_BULK_IN))
        else
        {
            MP_DEBUG("R:%d",psUsbDevMsdc->psCsw->u32DataResidue);
            if(psUsbDevMsdc->psCsw->u32DataResidue > mUsbOtgEPinMxPtSz(EP1))
                u16count = mUsbOtgEPinMxPtSz(EP1);
            else
                u16count = psUsbDevMsdc->psCsw->u32DataResidue;
            
            // Send u16FIFOByteCount Bytes data to F0 via DBUS;
            if (psUsbDevDesc->pbBulkTxRxData != 0)
            {
                psUsbDevDesc->pbBulkTxRxData = (BYTE*)((DWORD)psUsbDevDesc->pbBulkTxRxData | 0xa0000000);
                ret = bOTGCxFxWrRd(FOTG200_DMA2FIFO0,DIRECTION_IN,(BYTE*)((DWORD)psUsbDevDesc->pbBulkTxRxData + psUsbDevMsdc->dwTxByteCnt),u16count,eWhichOtg);
                if (ret == FALSE)
                {
                    mpDebugPrint("2Disconnect while reading");
                    break;
                }
                psUsbDevMsdc->dwTxByteCnt += u16count;
            }

            psUsbDevMsdc->psCsw->u32DataResidue -= u16count;
            psUsbDevDesc->dwBulkTxRxCounter -= u16count;
            BYTE    cmd;
            cmd = psUsbDevMsdc->psCbw->u8CB[0];
            if (cmd == SCSI_READ_10)
            {
                if ((psUsbDevDesc->dwBulkTxRxCounter == 0) && (psUsbDevMsdc->psCsw->u32DataResidue > 0))
                { // read remain data from Drive
                    WORD ret_val = STS_GOOD;
                    ret = ReadDataFromDrive ( (USB_READ10*)&(psUsbDevMsdc->psCbw->u8CB[0]),
                                               psUsbDevMsdc->psCbw->u8LUN,
                                               psUsbDevMsdc->dwTxByteCnt,
                                               eWhichOtg);
                    if (ret != STS_GOOD)
                    {
                        mpDebugPrint("error in ReadDataFromDrive");
                        break;
                    }
                    
                    psUsbDevMsdc->dwTxByteCnt = 0;
                }
                else
                {
                    ;//UartOutText("R End");
                }
            }
        }
    }

    psUsbDevMsdc->boIsReadWriteData = FALSE;
    psUsbDevMsdc->dwTxByteCnt = 0;
    mUsbOtgFIFODone(FIFO_BULK_IN);  // OK
    return eState;
}

void UsbSenseDataInit(BYTE lun, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
    PREQUEST_SENSE_STR pUSBRequestSense = (PREQUEST_SENSE_STR)&psUsbDevMsdc->sRequestSense[lun];

    pUSBRequestSense->ErrorCode         = SENSE_INVALID;
    pUSBRequestSense->Reserved1         = 0;
    pUSBRequestSense->SenseKey          = NO_SENSE;
    pUSBRequestSense->Information[0]    = 0;
    pUSBRequestSense->Information[1]    = 0;
    pUSBRequestSense->Information[2]    = 0;
    pUSBRequestSense->Information[3]    = 0;
    pUSBRequestSense->AddSenseLen       = 0;
    pUSBRequestSense->VendorSpec[0]     = 0;
    pUSBRequestSense->VendorSpec[1]     = 0;
    pUSBRequestSense->VendorSpec[2]     = 0;
    pUSBRequestSense->VendorSpec[3]     = 0;
    pUSBRequestSense->AddSenseCode      = 0;
    pUSBRequestSense->AddSenseCodeQ     = 0;
    pUSBRequestSense->Fru               = 0;
    
    memset( pUSBRequestSense->SnsKeyInfo, 0, SNSKEYINFO_LEN );
}

void UsbSenseKeySetup ( BYTE errCode,
                        BYTE senseKey,
                        DWORD info,
                        BYTE senseCode,
                        BYTE senseCodeQ,
                        BYTE snsKeyInfo0,
                        WORD snsKeyInfo1,
                        BYTE lun,
                        WHICH_OTG eWhichOtg)
{
	PUSB_DEVICE_MSDC psUsbDevMsdc     = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
    PREQUEST_SENSE_STR pUSBRequestSense = (PREQUEST_SENSE_STR)&psUsbDevMsdc->sRequestSense[lun];

    memset((BYTE*) pUSBRequestSense, 0, SENSE_KEY_LEN); // JL, 03032003

    pUSBRequestSense->ErrorCode = errCode;//0x70;
    pUSBRequestSense->Reserved1 = 0x00;
    pUSBRequestSense->SenseKey  = senseKey;

    pUSBRequestSense->Information[0] = (info & 0xff000000) >> 24;
    pUSBRequestSense->Information[1] = (info & 0x00ff0000) >> 16;
    pUSBRequestSense->Information[2] = (info & 0x0000ff00) >> 8;
    pUSBRequestSense->Information[3] = (info & 0x000000ff);

    pUSBRequestSense->AddSenseLen =  0xa;//SENSE_KEY_LEN - 7;//0xa;
    pUSBRequestSense->VendorSpec[0] = 0x0;
    pUSBRequestSense->VendorSpec[1] = 0x0;
    pUSBRequestSense->VendorSpec[2] = 0x0;
    pUSBRequestSense->VendorSpec[3] = 0x0;

    pUSBRequestSense->AddSenseCode = senseCode;
    pUSBRequestSense->AddSenseCodeQ = senseCodeQ;

    pUSBRequestSense->Fru = 0;
    
    if ( snsKeyInfo0 != 0 )
    {
        pUSBRequestSense->SnsKeyInfo[0] = SKSV | snsKeyInfo0;
        pUSBRequestSense->SnsKeyInfo[1] = (snsKeyInfo1 & 0xf0) >> 8;
        pUSBRequestSense->SnsKeyInfo[2] = snsKeyInfo1 & 0x0f;
    }
}

//////////////////////////////////////////////////////////////////////////
//SCSI Command Mode10    process
/////////////////////////////////////////////////////////////////////////

void USBStrgInitModeParaHdr(BYTE lun, WHICH_OTG eWhichOtg)
{
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);

    psUsbDevMsdc->sModeParaHdr10.ModeParaLen[1] = \
        (sizeof(MODE_PARA_HDR10) - 1)
        + sizeof(BLK_DSCRPT) * BLK_DSCRPT_MAX
        + sizeof(RWERR_RECVRY)
        + sizeof(DECNCT_RECNCT)
        + sizeof(FORMT)
        + sizeof(VRFYERR_RECVRY)
        + sizeof(CACHE_CNTRL)
        + sizeof(PRPHRL_DEV)
        + sizeof(CNTRL_MODE)
        + sizeof(MEDIA_TYPE)
        + sizeof(NOTCH_PERTITION);
    psUsbDevMsdc->sModeParaHdr10.MediaType = DRCT_ACCESS_DEV;
    psUsbDevMsdc->sModeParaHdr10.DevPara = 0;
    psUsbDevMsdc->sModeParaHdr10.BlkDscrptLen[0] = 0;
    psUsbDevMsdc->sModeParaHdr10.BlkDscrptLen[1] = sizeof(BLK_DSCRPT) * BLK_DSCRPT_MAX;

    if (SystemGetFlagReadOnly(SystemDriveIdGetByLun(lun)))
    {
        psUsbDevMsdc->sModeParaHdr10.DevPara = MS_WRITE_PROTECT;
    }
    else
    {
        psUsbDevMsdc->sModeParaHdr10.DevPara = 0;
    }

}

void USBStrgInitBlkDscrpt(WHICH_OTG eWhichOtg)
{
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
    DWORD   i = 0;

    for ( i = 0; i <  BLK_DSCRPT_MAX; i++ )
    {
        psUsbDevMsdc->sBlkDscrpt[i].DnstyCode = DNSTY;
        psUsbDevMsdc->sBlkDscrpt[i].BlkNum[0] = 0;
        psUsbDevMsdc->sBlkDscrpt[i].BlkNum[1] = 0;
        psUsbDevMsdc->sBlkDscrpt[i].BlkNum[2] = 0;

        psUsbDevMsdc->sBlkDscrpt[i].BlkLen[0] = 0;
        psUsbDevMsdc->sBlkDscrpt[i].BlkLen[1] = 0;
        psUsbDevMsdc->sBlkDscrpt[i].BlkLen[2] = 0;
    }

}


void USBStrgInitRWErrRecvry( RWERR_RECVRY *paraPage )
{
    paraPage->PageCode      = RWERR_RECVRY_PAGE;
    paraPage->PageLen       = RWERR_RECVRY_LEN;
    paraPage->RecvryWay     = AWRE | ARRE | TB | RC | EER | PER | DTE | DCR;
    paraPage->RretryCnt     = R_RETRY_MAX;
    paraPage->CorctLen      = DAT_CRECT_LEN;
    paraPage->HeadOfst      = HEAD_OFFSET;
    paraPage->DatStrbOfst   = DAT_STROV_OFFSET;
    paraPage->WretryCnt     = W_RETRY_MAX;
    paraPage->TimeLmt[0]    = RECVRY_TIME;
    paraPage->TimeLmt[1]    = 0;
}

void USBStrgInitDecnctRecnct( DECNCT_RECNCT *paraPage )
{
    paraPage->PageCode          = DECNCT_RECNCT_PAGE;
    paraPage->PageLen           = DECNCT_RECNCT_LEN;
    paraPage->BufFulRatio       = BUF_FUL_RATIO;
    paraPage->BufEmpRatio       = BUF_EMP_RATIO;
    paraPage->BusInactLim[0]    = BUS_INACT_LIM;
    paraPage->BusInactLim[1]    = BUS_INACT_LIM;
    paraPage->DiscnctTmLim[0]   = DESCNCT_TIME_LIM;
    paraPage->DiscnctTmLim[1]   = DESCNCT_TIME_LIM;
    paraPage->CnctTmLim[0]      = CNCT_TIME_LIM;
    paraPage->CnctTmLim[1]      = CNCT_TIME_LIM;
    paraPage->MaxBrstSize[0]    = BURST_SIZE;
    paraPage->MaxBrstSize[1]    = BURST_SIZE;
    paraPage->dtdc              = DTDC;
}

void USBStrgInitFormat( FORMT *paraPage )
{
    paraPage->PageCode          = FORMT_PAGE;
    paraPage->PageLen           = FORMT_LEN;
    paraPage->TrckNum[0]        = 0;
    paraPage->TrckNum[1]        = 0;
    paraPage->ChgSecNum[0]      = 0;
    paraPage->ChgSecNum[1]      = 0;
    paraPage->ChgTrckLogNum[0]  = 0;
    paraPage->ChgTrckLogNum[1]  = 0;
    paraPage->ChgTrckNum[0]     = 0;
    paraPage->ChgTrckNum[1]     = 0;
    paraPage->SecNum[0]         = 0;
    paraPage->SecNum[1]         = 0;
    paraPage->DatNum[0]         = 0;
    paraPage->DatNum[1]         = 0;
    paraPage->intrleav[0]       = 0;
    paraPage->intrleav[1]       = 0;
    paraPage->TrckSkew[0]       = 0;
    paraPage->TrckSkew[1]       = 0;
    paraPage->SrndrSkew[0]      = 0;
    paraPage->SrndrSkew[1]      = 0;
    paraPage->charcter          = 0x20; // RMB
}

void USBStrgInitVrfyErrRecvry( VRFYERR_RECVRY *paraPage )
{
    paraPage->PageCode      = VRFYERR_RECVRY_PAGE;
    paraPage->PageLen       = VRFYERR_RECVRY_LEN;
    paraPage->RecvryWay     = EER | PER | DTE | DCR;
    paraPage->RetryCnt      = VERFY_RETRY_MAX;
    paraPage->CorctLen      = VERFY_RETRY_LEN;
    paraPage->TimeLmt[0]    = VERFY_RCVRY_LIM;
    paraPage->TimeLmt[1]    = 0;
}

void USBStrgInitCasheCntrl( CACHE_CNTRL *paraPage )
{
    paraPage->PageCode          = CACHE_CNTRL_PAGE;
    paraPage->PageLen           = CACHE_CNTRL_LEN;
    paraPage->CntrlWay          = 0;
    paraPage->Priority          = 0;
    paraPage->DsablPrftchLen[0] = 0;
    paraPage->DsablPrftchLen[1] = 0;
    paraPage->MimPrftch[0]      = 0;
    paraPage->MimPrftch[1]      = 0;
    paraPage->MaxPrftch[0]      = 0;
    paraPage->MaxPrftch[1]      = 0;
    paraPage->MaxPrftchCelng[0] = 0;
    paraPage->MaxPrftchCelng[1] = 0;
}

void USBStrgInitPerphral( PRPHRL_DEV *paraPage )
{
    paraPage->PageCode      = PERPHRA_PAGE;
    paraPage->PageLen       = PERPHRAL_DEV_LEN;
    paraPage->IntrfaceID[0] = SCSI;
    paraPage->IntrfaceID[1] = SCSI;

    paraPage->Reserved1     = 0;
    paraPage->Reserved2     = 0;
    paraPage->Reserved3     = 0;
    paraPage->Reserved4     = 0;
}

void USBStrgInitCntrlMode( CNTRL_MODE *paraPage )
{
    paraPage->PageCode          = CNTRL_MODE_PAGE;
    paraPage->PageLen           = CNTRL_MODE_LEN;
    paraPage->rlec              = 0;
    paraPage->QueData           = 0;
    paraPage->TrgtStat          = 0;
    paraPage->Reserved          = 0;
    paraPage->RdyAenHldTime[0]  = 0;
    paraPage->RdyAenHldTime[1]  = 0;
}

void USBStrgInitMediaType( MEDIA_TYPE *paraPage )
{
    paraPage->PageCode      = MEDIA_TYPE_PAGE;
    paraPage->PageLen       = MEDIA_TYPE_LEN;
    paraPage->Reserved1     = 0;
    paraPage->Reserved2     = 0;
    paraPage->MediaType1    = 0;
    paraPage->MediaType2    = 0;
    paraPage->MediaType3    = 0;
    paraPage->MediaType4    = 0;
}

 void USBStrgInitNotchPertition( NOTCH_PERTITION *paraPage )
{
    paraPage->PageCode      = NOTCH_PERTITION_PAGE;
    paraPage->PageLen       = NOTCH_PERTITION_LEN;
    paraPage->NdLpn         = 0;
    paraPage->Reserved1     = 0;
    paraPage->MaxNtchNum[0] = 0;
    paraPage->MaxNtchNum[1] = 0;
    paraPage->ActvNtch[0]   = 0;
    paraPage->ActvNtch[1]   = 0;

    paraPage->NtchStrtPos[0] = 0;
    paraPage->NtchStrtPos[1] = 0;
    paraPage->NtchStrtPos[2] = 0;
    paraPage->NtchStrtPos[3] = 0;

    paraPage->NtchEndPos[0] = 0;
    paraPage->NtchEndPos[1] = 0;
    paraPage->NtchEndPos[2] = 0;
    paraPage->NtchEndPos[3] = 0;

    paraPage->NtchParaPage[0] = 0;
    paraPage->NtchParaPage[1] = 0;
    paraPage->NtchParaPage[2] = 0;
    paraPage->NtchParaPage[3] = 0;
    paraPage->NtchParaPage[4] = 0;
    paraPage->NtchParaPage[5] = 0;
    paraPage->NtchParaPage[6] = 0;
    paraPage->NtchParaPage[7] = 0;

}

void UsbStrgInitPara(BYTE lun, WHICH_OTG eWhichOtg)
{
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
    DWORD i = 0;

    USBStrgInitModeParaHdr(lun, eWhichOtg);
    USBStrgInitBlkDscrpt(eWhichOtg);

    USBStrgInitRWErrRecvry  ( (RWERR_RECVRY *)&psUsbDevMsdc->sParaPage[i++] );
    USBStrgInitDecnctRecnct ( (DECNCT_RECNCT *)&psUsbDevMsdc->sParaPage[i++] );
    USBStrgInitFormat       ( (FORMT *)&psUsbDevMsdc->sParaPage[i++] );
    USBStrgInitVrfyErrRecvry( (VRFYERR_RECVRY *)&psUsbDevMsdc->sParaPage[i++] );
    USBStrgInitCasheCntrl   ( (CACHE_CNTRL *)&psUsbDevMsdc->sParaPage[i++] );
    USBStrgInitPerphral     ( (PRPHRL_DEV *)&psUsbDevMsdc->sParaPage[i++] );
    USBStrgInitCntrlMode    ( (CNTRL_MODE *)&psUsbDevMsdc->sParaPage[i++] );
    USBStrgInitMediaType    ( (MEDIA_TYPE *)&psUsbDevMsdc->sParaPage[i++] );
    USBStrgInitNotchPertition((NOTCH_PERTITION *)&psUsbDevMsdc->sParaPage[i++] );
}

///////////////////////////////////////////////////////////////////////////
//SCSI   Command Processing
//////////////////////////////////////////////////////////////////////////
WORD Read10(BYTE** hData, USB_READ10 *pRead, BYTE lun, DWORD* pData_residue, DWORD expect_tx_length, WHICH_OTG eWhichOtg)
{
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
    DWORD	lba;
    WORD	cnt;
    WORD    ret = STS_GOOD;
    BYTE    id;

    lba = BYTE_TO_DWORD(pRead->BlkAddr[0],
                        pRead->BlkAddr[1],
                        pRead->BlkAddr[2],
                        pRead->BlkAddr[3]
                        );
    cnt = BYTE_TO_WORD(pRead->BlkCnt[0], pRead->BlkCnt[1]);
    //*hData = (BYTE *)(g_VirtualStorageBuffer +(lba<<9));
    //id = Mcard_CurLunGetCardID(lun);
    *pData_residue = cnt << EXP_VAL;
 
    if((*pData_residue != expect_tx_length)||(expect_tx_length == 0))   //expect_tx_length is total length in this transfer scsi command
    {
        if(psUsbDevMsdc->psCsw->u32DataResidue > expect_tx_length)
            ret = STS_PHASE_ERR;
        else
            ret = STS_CMD_FAILED;
    }
    else 
    {
        ret = ReadDataFromDrive(pRead, lun, 0, eWhichOtg);
    }
    *hData = (BYTE *)UsbOtgBufferGet(eWhichOtg);//g_USBBUF;

    return ret;
}


WORD Write10(BYTE** hData, USB_WRITE10 *pWrite, BYTE lun, DWORD* pData_residue, DWORD expect_tx_length, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)&psUsbOtg->sUsbDev.sMsdc;
    DWORD   lba;
    WORD    cnt;
    WORD    ret = STS_GOOD;
    BYTE    bDriveId; // From Drive
    
    lba = BYTE_TO_DWORD(pWrite->BlkAddr[0],
                        pWrite->BlkAddr[1],
                        pWrite->BlkAddr[2],
                        pWrite->BlkAddr[3]
                        );
    cnt = BYTE_TO_WORD(pWrite->BlkCnt[0], pWrite->BlkCnt[1]);
    *hData = (BYTE *)UsbOtgBufferGet(eWhichOtg);//g_USBBUF;
    *pData_residue = cnt * BLOCK_LEN;
    if( *pData_residue < expect_tx_length )
    {  
            mUsbOtgEPoutStallSet(EP2);
            *pData_residue = (expect_tx_length - *pData_residue);
            ret = STS_CMD_FAILED;
    }
    else if (*pData_residue > expect_tx_length)
    {
            if(expect_tx_length > 0)
                mUsbOtgEPoutStallSet(EP2);

            ret = STS_PHASE_ERR;
    }
    else
    {
        bDriveId = SystemDriveIdGetByLun(lun);
        if( SystemGetFlagReadOnly(bDriveId))
        {
            psUsbDevMsdc->wCheckCondition[lun] = kWriteProtectCondition;
    	    ret = STS_CMD_FAILED;
        }
    }

    return ret;
}



WORD TestUnitReady(BYTE** hData, BYTE lun, DWORD* pData_residue, DWORD expect_tx_length, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)&psUsbOtg->sUsbDev.sMsdc;
    WORD    ret = STS_GOOD;
    BYTE    i = 0;
    
    *hData = 0;
    *pData_residue = 0;
    if( expect_tx_length != 0)
    { 
          mUsbOtgEPoutStallSet(EP2);
          *pData_residue = expect_tx_length;
          return STS_CMD_FAILED;
    }

    if ( (SystemDriveIdGetByLun(lun) != 0) && (SystemCardPresentCheck(SystemDriveIdGetByLun(lun))) )
    {
        if (psUsbDevMsdc->boMediaNotPresent[lun] == TRUE)
        {
            psUsbDevMsdc->wCheckCondition[lun] = kBecomingReadyCondition;
            psUsbDevMsdc->boMediaNotPresent[lun] = FALSE;
            ret = STS_CMD_FAILED;
        }
        else
        {
            ret = STS_GOOD;
        }
    }
    else
    {
        ret = STS_CMD_FAILED;
        psUsbDevMsdc->boMediaNotPresent[lun] = TRUE;
        psUsbDevMsdc->wCheckCondition[lun] = kMediaNotPresentCondition;
    }
    
    return ret;
}


WORD Inquiry( BYTE** hData, USB_INQUIRY *pCdb, BYTE lun, DWORD* pData_residue, DWORD expect_tx_length, WHICH_OTG eWhichOtg)
{
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);

    switch(lun)
    {
        case LUN_NUM_0:
            memcpy(&psUsbDevMsdc->sDeviceInf.ProductId[0], &gpProductId_Lun_0[0], sizeof(psUsbDevMsdc->sDeviceInf.ProductId));
        break;
        case LUN_NUM_1:
            memcpy(&psUsbDevMsdc->sDeviceInf.ProductId[0], &gpProductId_Lun_1[0], sizeof(psUsbDevMsdc->sDeviceInf.ProductId));
        break;
        case LUN_NUM_2:
            memcpy(&psUsbDevMsdc->sDeviceInf.ProductId[0], &gpProductId_Lun_2[0], sizeof(psUsbDevMsdc->sDeviceInf.ProductId));
        break;
        case LUN_NUM_3:
            memcpy(&psUsbDevMsdc->sDeviceInf.ProductId[0], &gpProductId_Lun_3[0], sizeof(psUsbDevMsdc->sDeviceInf.ProductId));
        break;
        case LUN_NUM_4:
            memcpy(&psUsbDevMsdc->sDeviceInf.ProductId[0], &gpProductId_Lun_4[0], sizeof(psUsbDevMsdc->sDeviceInf.ProductId));
        break;
        case LUN_NUM_5:
            memcpy(&psUsbDevMsdc->sDeviceInf.ProductId[0], &gpProductId_Lun_5[0], sizeof(psUsbDevMsdc->sDeviceInf.ProductId));
        break;
        case LUN_NUM_6:
            memcpy(&psUsbDevMsdc->sDeviceInf.ProductId[0], &gpProductId_Lun_6[0], sizeof(psUsbDevMsdc->sDeviceInf.ProductId));
        break;
        default:
            memcpy(&psUsbDevMsdc->sDeviceInf.ProductId[0], "NULL", sizeof(psUsbDevMsdc->sDeviceInf.ProductId));
        break;
    }

    *hData = (BYTE*)&psUsbDevMsdc->sDeviceInf;
    if((pCdb->AllocLength > sizeof(STANDARD_INQUIRY))&&(expect_tx_length > sizeof(STANDARD_INQUIRY)))
    {
        *pData_residue = expect_tx_length;
        return STS_CMD_FAILED;
    }
    
    if ( pCdb->AllocLength < sizeof(STANDARD_INQUIRY) )
    {
        *pData_residue = pCdb->AllocLength;
    }
    else
    {
        *pData_residue = sizeof( psUsbDevMsdc->sDeviceInf );
    }

    return( STS_GOOD );
}

BYTE ReadCapacity( BYTE** hData, BYTE lun, DWORD* pData_residue, WHICH_OTG eWhichOtg)
{
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
    DWORD   dev_cnt = 0;
    WORD    ret = STS_GOOD;

    if ( (SystemDriveIdGetByLun(lun) != 0) && (SystemCardPresentCheck(SystemDriveIdGetByLun(lun))) )
    {
        //dev_cnt = Mcard_GetMaxLun();

        //if(dev_cnt == 0)   //if device count is zero than capacity is zero
        //    dev_cnt = 1;
        //else
        
        //psUsbDevMsdc->sMediaInfo.LastLBA = Mcard_GetCapacity(Mcard_CurLunGetCardID(lun)) - 1;
        psUsbDevMsdc->sMediaInfo.LastLBA = SystemDriveTotalSectorGetByLun(lun) - 1;

        //psUsbDevMsdc->sMediaInfo.BlockLength = BLOCK_LEN;
        psUsbDevMsdc->sMediaInfo.BlockLength = SystemDriveSectorSizeGetByLun(lun);
        *hData = (BYTE*)&psUsbDevMsdc->sMediaInfo;
        *pData_residue = 8;
    }
    else
    {
        psUsbDevMsdc->wCheckCondition[lun] = kMediaNotPresentCondition;
        memset(&psUsbDevMsdc->sMediaInfo, 0, 8);
        *hData = (BYTE*)&psUsbDevMsdc->sMediaInfo;
        *pData_residue = 8;
        ret = STS_CMD_FAILED;
       /* 
        UsbStallEp(BULK_IN_EP, TRUE);
        *hData = 0;
        *pData_residue = 0;	
        gCheckCondition[lun] = kMediaNotPresentCondition;
        ret = STS_STALL;
       */
    }
    
    return ret;
}


WORD ReadFormatCapacities ( BYTE** hData, BYTE lun, DWORD* pData_residue, WHICH_OTG eWhichOtg)
{
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
    DWORD   dev_cnt = 0;
    WORD    ret = STS_GOOD;

    if ( (SystemDriveIdGetByLun(lun) != 0) && (SystemCardPresentCheck(SystemDriveIdGetByLun(lun))) )
    {
        //dev_cnt = Mcard_GetMaxLun();
        //if(dev_cnt== 0)   //if device count is zero than capacity is zero
        //    dev_cnt = 0;
        //else

        memset((BYTE*)&psUsbDevMsdc->sCapacityDescriptor, 0, sizeof(psUsbDevMsdc->sCapacityDescriptor));
        psUsbDevMsdc->sCapacityDescriptor.CapacityListLength = 8 ;
        //psUsbDevMsdc->sCapacityDescriptor.LastLBA = Mcard_GetCapacity(Mcard_CurLunGetCardID(lun)); // Number of Blocks
        psUsbDevMsdc->sCapacityDescriptor.NumOfBlock = SystemDriveTotalSectorGetByLun(lun); // Number of Blocks

        //psUsbDevMsdc->sCapacityDescriptor.BlockLength = BLOCK_LEN;
        psUsbDevMsdc->sCapacityDescriptor.BlockLength = SystemDriveSectorSizeGetByLun(lun); // Block Length     
        psUsbDevMsdc->sCapacityDescriptor.BlockLength |= DESCRIPTOR_CODE_DEFINITION;
        *hData = (BYTE*) &psUsbDevMsdc->sCapacityDescriptor;
        *pData_residue = sizeof(psUsbDevMsdc->sCapacityDescriptor);
    }
    else
    {
        psUsbDevMsdc->wCheckCondition[lun] = kMediaNotPresentCondition;
        memset(&psUsbDevMsdc->sMediaInfo, 0, 8);
        *hData = (BYTE*)&psUsbDevMsdc->sMediaInfo;
        *pData_residue = 8;	
        ret = STS_CMD_FAILED;
       /* 
        UsbStallEp(BULK_IN_EP, TRUE);
        *hData = 0;
        *pData_residue = 0;	
        gCheckCondition[lun] = kMediaNotPresentCondition;
        ret = STS_STALL;
       */
    }
    
    return ret;
}


WORD ModeSense6 (BYTE** hData, BYTE lun, DWORD* pData_residue, WHICH_OTG eWhichOtg)
{
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);

    psUsbDevMsdc->sModeParaHdr6.ModeParaLen = 3;
    psUsbDevMsdc->sModeParaHdr6.MediaType = 0;
    
    if (SystemGetFlagReadOnly(SystemDriveIdGetByLun(lun)))// ||
        //gIsMediumRemoval[lun])
    {
        psUsbDevMsdc->sModeParaHdr6.DevPara = MS_WRITE_PROTECT;
    }
    else
    {
        psUsbDevMsdc->sModeParaHdr6.DevPara = 0;
    }
    
    psUsbDevMsdc->sModeParaHdr6.BlkDscrptLen = 0;
    *hData = (BYTE*) &psUsbDevMsdc->sModeParaHdr6;
    *pData_residue = 4;
    return  STS_GOOD;
}

WORD Verify10(BYTE** hData, BYTE lun, USB_VERIFY10 *pVerify, WHICH_OTG eWhichOtg)
{
    // procee later
    return( STS_GOOD );
}


WORD MeadiumRemoval(BYTE** hData, USB_MEDIUM_REMOVAL *pCmd, BYTE lun, DWORD* pData_residue, WHICH_OTG eWhichOtg)
{
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
    BYTE	ret = STS_GOOD;


    *hData = 0;
    *pData_residue = 0;

    if ( pCmd->Prevent == 1 )  // Prevent
    {
        psUsbDevMsdc->boIsMediumRemoval[lun] = TRUE;
        //gCheckCondition[lun] = kInvalidCommandCondition;
        psUsbDevMsdc->boIsPrevent[lun] = TRUE;
        UsbSenseKeySetup (  CUR_ERR,
                            SenseKey5,
                            0,
                            SenseCode24,
                            SenseCodeQ0,
                            0,
                            0,
                            lun,
                            eWhichOtg);
        ret = STS_CMD_FAILED;
    }
    else  // Allow
    {
        psUsbDevMsdc->boIsMediumRemoval[lun] = FALSE;
        UsbSenseKeySetup (  CUR_ERR,
                            SenseKey0,
                            0,
                            SenseCode0,
                            SenseCodeQ0,
                            0,
                            0,
                            lun,
                            eWhichOtg);
        ret = STS_GOOD;
    }

    return ret;
}


BYTE StartStopUnit (BYTE** hData, USB_START_STOP_UNIT *pStartStopCmd, BYTE lun, DWORD* pData_residue, WHICH_OTG eWhichOtg)
{
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
    BYTE   ret = STS_GOOD;
    BYTE   sts = STS_GOOD; // for Mac OS X 10.2


    *hData = 0;
    *pData_residue = 0;
    
    if ( pStartStopCmd->Immed & 0x01 )
    {
        sts = STS_SENDED;
    }

    switch ( pStartStopCmd->LoEjStart )
    {
        case DISABL_ACCSS:
        case LOAD:
            ret = STS_GOOD;
            break;

        case ENABL_ACCSS:
            psUsbDevMsdc->wCheckCondition[lun] = 0;
            ret = STS_GOOD;
            break;

        case UNLOAD:
            if (psUsbDevMsdc->boIsMediumRemoval[lun])
            {
                psUsbDevMsdc->wCheckCondition[lun] = kMediaRemovalPreventedCondition;
                ret = STS_CMD_FAILED;
            }
            else
            {
             //   MP_DEBUG("UNLOAD");
                psUsbDevMsdc->wCheckCondition[lun] = kStopUnitCondition;
                ret = STS_GOOD;
            }
           break;

        default:
            ret = STS_CMD_FAILED;
          break;
    }

    /*---- already sended Status Byte ----*/
    if ( (sts == STS_SENDED) && (ret == STS_GOOD) )
        return sts;

	return ret;
}

WORD ModeSense10 (BYTE** hData, USB_MODE_SENSE10 *pModeSense, BYTE lun, DWORD* pData_residue, WHICH_OTG eWhichOtg)
{
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
    MODE_PARA_HDR10 *pModeParaHdr10 = (MODE_PARA_HDR10 *)&psUsbDevMsdc->sModeParaHdr10;
    DWORD   i = 0, j = 0, k = 0;
    WORD    datasize;
    WORD    allocLength;
    WORD    paraListLength;
    WORD    modeParaLen;
    WORD    blkDscrptLen;
    BYTE    dbd;
    BYTE    pageCode;
    BYTE    buff[140];
    BYTE nFifoFlag ;

    UsbStrgInitPara(lun, eWhichOtg);

    dbd = (pModeSense->Dbd & DBD) >> 3;

    blkDscrptLen = pModeParaHdr10->BlkDscrptLen[0];
    blkDscrptLen <<= 8;
    blkDscrptLen |= pModeParaHdr10->BlkDscrptLen[1];

    pageCode = pModeSense->PcPgCode & MD_SENSE_PAGE_CODE;

    allocLength = pModeSense->AllocLength[0];
    allocLength <<= 8;
    allocLength |= pModeSense->AllocLength[1];


    if ( !allocLength )
    {
        return( STS_GOOD );
    }

    paraListLength = sizeof(MODE_PARA_HDR10);

    memcpy( &buff[0], pModeParaHdr10, sizeof(MODE_PARA_HDR10) );
    k = 8;
    if ( !dbd )
    {
        paraListLength += sizeof(BLK_DSCRPT);
        memcpy( &buff[8], &psUsbDevMsdc->sBlkDscrpt[0], sizeof(BLK_DSCRPT) );
        k = 16;
    }

    for( i = 0; i < PARA_PAGE_MAX; i++ )
    {
        if ( pageCode == psUsbDevMsdc->sParaPage[i].PageCode || pageCode == ALL_PARA_PAGE )
        {
            memcpy( &buff[k+j], &psUsbDevMsdc->sParaPage[i], psUsbDevMsdc->sParaPage[i].PageLen+2 );
            paraListLength += (psUsbDevMsdc->sParaPage[i].PageLen + 2);
            j += (psUsbDevMsdc->sParaPage[i].PageLen + 2);
        }
    }

    if ( allocLength > paraListLength )         /* decide send data size    */
    {
        datasize = paraListLength;
    }
    else
    {
        datasize = allocLength;
    }

    modeParaLen = datasize - 2;
    pModeParaHdr10->ModeParaLen[1] = (BYTE)modeParaLen;
    pModeParaHdr10->ModeParaLen[0] = (BYTE)(modeParaLen>>8);


    memcpy( *hData, (BYTE*)&pModeParaHdr10->ModeParaLen[0], modeParaLen );
    //memcpy( *hData, &buff[0], modeParaLen );
    *pData_residue = datasize;
    return STS_GOOD;
}


BYTE RequestSense (BYTE** hData, USB_REQUEST_SENSE *pRequestSense, BYTE lun, DWORD* pData_residue, WHICH_OTG eWhichOtg)
{
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
    PREQUEST_SENSE_STR pUSBRequestSense = (PREQUEST_SENSE_STR)&psUsbDevMsdc->sRequestSense[lun];
    BYTE    ret = STS_GOOD;
    BYTE    allocLength;

    *hData = (BYTE*)&psUsbDevMsdc->sRequestSense[lun];
    allocLength = pRequestSense->AllocLength;
    if ( allocLength == 0x00 )
    {
        UsbSenseDataInit(lun, eWhichOtg);
        return ret;
    }

    // set & send SenseData 
    if ( pUSBRequestSense->ErrorCode & SENSE_INVALID )       // no valid sense data
    {
        pUSBRequestSense->SenseKey = NO_SENSE;
        pUSBRequestSense->AddSenseCode = NO_ASC;
    }

    switch ( psUsbDevMsdc->wCheckCondition[lun] )
    {
        case kStopUnitCondition:
        case kMediaNotPresentCondition:
            UsbSenseKeySetup (  CUR_ERR,
                                SenseKey2,
                                0,
                                SenseCode3A,  // Media not present
                                SenseCodeQ0,
                                0,
                                0,
                                lun,
                                eWhichOtg);
        break;

        case kMediaChangeCondition:
        case kBecomingReadyCondition:
            UsbSenseKeySetup (  CUR_ERR,
                                SenseKey6,
                                0,
                                SenseCode28,  // Media Change
                                SenseCodeQ0,
                                0,
                                0,
                                lun,
                                eWhichOtg);
        break;

        case kWriteProtectCondition:
            UsbSenseKeySetup (  CUR_ERR,
                                SenseKey7,
                                0,
                                SenseCode27, // Write Protect
                                SenseCodeQ0,
                                0,
                                0,
                                lun,
                                eWhichOtg);
        break;

        case kInvalidCommandCondition:
            UsbSenseKeySetup(   CUR_ERR,
                                SenseKey5,
                                0,
                                SenseCode24, // Invalid Field In Data Packet
                                SenseCodeQ0,
                                CDB_ILLEGAL,
                                1,
                                lun,
                                eWhichOtg);
        break;

        case kMediaRemovalPreventedCondition:
            UsbSenseKeySetup(   CUR_ERR,
                                SenseKey5,
                                0,
                                SenseCode53, // Media Removal Prevented
                                SenseCodeQ2,
                                CDB_ILLEGAL,
                                1,
                                lun,
                                eWhichOtg);
        break;
/*
        case kBecomingReadyCondition:
            gCheckCondition[lun] = 0;
            UsbSenseKeySetup(   CUR_ERR,
                                SenseKey6,
                                0,
                                SenseCode28, // Media Changed
                                SenseCodeQ0,
                                0,
                                0,
                                lun,
                                eWhichOtg);
        break;
*/
        default:
            UsbSenseKeySetup (  CUR_ERR,
                                SenseKey0,
                                0,
                                SenseCode0,
                                SenseCodeQ0,
                                0,
                                0,
                                lun,
                                eWhichOtg);
        break;
    }

    if(psUsbDevMsdc->boIsPrevent[lun])
    {
        //case kInvalidCommandCondition:
        UsbSenseKeySetup(   CUR_ERR,
                            SenseKey5,
                            0,
                            SenseCode24, // Invalid Field In Data Packet
                            SenseCodeQ0,
                            CDB_ILLEGAL,
                            1,
                            lun,
                            eWhichOtg);
        //break;
        psUsbDevMsdc->boIsPrevent[lun] = FALSE;
    }

    *pData_residue = allocLength;

    if (psUsbDevMsdc->wCheckCondition[lun] != kStopUnitCondition)
    {
        psUsbDevMsdc->wCheckCondition[lun] = 0;
    }
    
    return ret;
}


//scsi command  process function
static WORD ScsiCommandProcess(BYTE** hData,
                        BYTE* pCmd,
                        BYTE lun,
                        DWORD* pData_desidue,
                        DWORD expect_tx_length,
                        WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES  psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
    WORD sts = STS_GOOD;
    BYTE i = 0;

    switch ( *pCmd )
    {
        case SCSI_READ_10: 
            MP_DEBUG("-USBOTG%d- SCSI_READ_10 Lun:%d", eWhichOtg, lun);
            sts =  Read10 (hData, (USB_READ10*) pCmd, lun, pData_desidue, expect_tx_length, eWhichOtg);
        break;

        case SCSI_WRITE_10:
            MP_DEBUG("-USBOTG%d- SCSI_WRITE_10 Lun:%d", eWhichOtg, lun);
            sts = Write10 (hData, (USB_WRITE10*) pCmd, lun, pData_desidue, expect_tx_length, eWhichOtg);
        break;

        case SCSI_TEST_UNIT_READY:
            MP_DEBUG("-USBOTG%d- SCSI_TEST_UNIT_READY Lun:%d", eWhichOtg, lun);
            //UartOutText(".");
            sts = TestUnitReady (hData, lun, pData_desidue, expect_tx_length, eWhichOtg);	
        break;

        case SCSI_REQUEST_SENSE:
            MP_DEBUG("-USBOTG%d- SCSI_REQUEST_SENSE Lun:%d", eWhichOtg, lun);
            sts = RequestSense (hData, (USB_REQUEST_SENSE *) pCmd, lun, pData_desidue, eWhichOtg);
        break;

        case SCSI_INQUIRY:
            MP_DEBUG("-USBOTG%d- SCSI_INQUIRY Lun:%d", eWhichOtg, lun);
            sts = Inquiry (hData, (USB_INQUIRY *) pCmd, lun, pData_desidue, expect_tx_length, eWhichOtg);
        break;

        //case SCSI_MODE_SELECT6:
        //    MP_DEBUG("kSCSI_MODE_SELECT6");
            //sts = USB2_ModeSelect6 ( (SCSI_MODE_SELECT6 *)&pUSBBulkOnlyCmd->OpCode );
        break;

        case SCSI_MODE_SENSE_6:
            MP_DEBUG("-USBOTG%d- SCSI_MODE_SENSE_6 Lun:%d", eWhichOtg, lun);
            sts = ModeSense6 (hData, lun, pData_desidue, eWhichOtg);
        break;

        case SCSI_START_STOP_UNIT:
            MP_DEBUG("-USBOTG%d- SCSI_START_STOP_UNIT Lun:%d", eWhichOtg, lun);
            sts =  StartStopUnit (hData, (USB_START_STOP_UNIT *) pCmd, lun, pData_desidue, eWhichOtg);
        break;


        case SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL:
            MP_DEBUG("-USBOTG%d- SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL Lun:%d", eWhichOtg, lun);
            sts = MeadiumRemoval (hData, (USB_MEDIUM_REMOVAL *) pCmd, lun, pData_desidue, eWhichOtg);	
        break;

        case SCSI_READ_CAPACITY:
            MP_DEBUG("-USBOTG%d- SCSI_READ_CAPACITY Lun:%d", eWhichOtg, lun);
            sts = ReadCapacity (hData, lun, pData_desidue, eWhichOtg);
        break;

        case SCSI_VERIFY_10:
            MP_DEBUG("-USBOTG%d- SCSI_VERIFY_10 Lun:%d", eWhichOtg, lun);
            sts = Verify10 (hData, lun, (USB_VERIFY10*) pCmd, eWhichOtg);
        break;

        case SCSI_MODE_SENSE_10:
            MP_DEBUG("-USBOTG%d- SCSI_MODE_SENSE_10 Lun:%d", eWhichOtg, lun);
            sts = ModeSense10 (hData, (USB_MODE_SENSE10*)pCmd, lun, pData_desidue, eWhichOtg);
        break ;


        case SCSI_READ_FORMAT_CAPACITIES:
            MP_DEBUG("-USBOTG%d- SCSI_READ_FORMAT_CAPACITIES Lun:%d", eWhichOtg, lun);
            sts = ReadFormatCapacities (hData, lun, pData_desidue, eWhichOtg);
        break;

        #if USBOTG_DEVICE_PROTECTION
        case SCSI_VENDOR_PROTECTION_CMD:
            sts = VendorProtectionCmd(hData, (USB_PROTECTION_CMD*) pCmd,lun, pData_desidue, eWhichOtg);
            break;
        #endif

        #if USBOTG_DEVICE_MSDC_TECO
        case SCSI_VENDOR_TECO_CMD:
            sts = VendorTecoCmd(hData, (USB_TECO_CMD*) pCmd,lun, pData_desidue, eWhichOtg);
            break;
        #endif
        
        default:
            if (*pCmd == SCSI_WRITE_12) 
            {
                mUsbOtgEPoutStallSet(EP2); // OK
                //mUsbEPoutStallSet(EP2);
                //mUsbOtgEPoutStallSet(OTG_FIFO_BULK_OUT);
                MP_DEBUG("-USBOTG%d- SCSI_WRITE_12 Lun:%d", eWhichOtg, lun);
            }
            if (*pCmd == SCSI_READ_12)
            {
                MP_DEBUG("-USBOTG%d- SCSI_READ_12 Lun:%d", eWhichOtg, lun);
            }
            
            MP_DEBUG("-USBOTG%d- SCSI_UNKNOW_COMMAND_OpCode 0x%x  Lun:%d", eWhichOtg, *pCmd, lun);
            psUsbDevMsdc->wCheckCondition[lun] = kInvalidCommandCondition;
            UsbSenseKeySetup(   CUR_ERR,
                                ILGAL_REQ,
                                0,
                                ASC_INVLD_CDB,
                                ASCQ_INVLD_CDB,
                                CDB_ILLEGAL,
                                1,
                                lun,
                                eWhichOtg);
            sts = STS_CMD_FAILED;
        break;
    }

    if(sts != STS_GOOD)
        MP_DEBUG("-USBOTG%d- Cmd:0x%x Lun:%d Fail!!", eWhichOtg, *pCmd, lun);
        
    if ( *pCmd != SCSI_REQUEST_SENSE )
    {
        switch ( psUsbDevMsdc->wCheckCondition[lun] )
        {
            case kStopUnitCondition:
                if (*pCmd != SCSI_START_STOP_UNIT)
                {
                    // After stop unit, return failed for only the command TestUnitReady
                    if (*pCmd == SCSI_TEST_UNIT_READY)
                    {
                        MP_DEBUG("-USBOTG%d- Condition:StopUnit Lun:%d", eWhichOtg, lun);
                        sts = STS_CMD_FAILED;
                    }
                }
                break;

            case kMediaChangeCondition:
            case kMediaRemovalPreventedCondition:
                MP_DEBUG("-USBOTG%d- Condition:MediaChange/MediaRemoval Lun:%d", eWhichOtg, lun);
                sts = STS_CMD_FAILED;
                break;

            case kWriteProtectCondition:
            {
                if (    *pCmd == SCSI_WRITE_10 ||
                        *pCmd == SCSI_VERIFY_10 )
                {
                    MP_DEBUG("-USBOTG%d- Condition:WriteProtect Lun:%d", eWhichOtg, lun);
                    sts = STS_CMD_FAILED;
                }
            }
                break;

            //case kBecomingReadyCondition:
            case kInvalidCommandCondition:
                    MP_DEBUG("-USBOTG%d- Condition:InvalidCommand Lun:%d", eWhichOtg, lun);
                    sts = STS_CMD_FAILED;
               /* 
                UsbStallEp(BULK_IN_EP, TRUE);
                sts = STS_STALL;
                */

            default:
                break;
        }
    }

    
     return sts;
}

// Change the USB condition via UI Card Event (Card-In/Out)
void USBCardStsByUI(BYTE bDriveId, BYTE bCardIn, WHICH_OTG eWhichOtg)
{
	PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
    BYTE bLun = SystemDriveLunNumGet(bDriveId);

    if(bCardIn) // Card - In
    {
        psUsbDevMsdc->wCheckCondition[bLun] = kMediaChangeCondition; //kBecomingReadyCondition;
        psUsbDevMsdc->boMediaNotPresent[bLun] = FALSE;
    }
    else  // Card - Out
    {
        psUsbDevMsdc->boMediaNotPresent[bLun] = TRUE;
        psUsbDevMsdc->wCheckCondition[bLun] = kMediaNotPresentCondition;
    }
}

#endif   //USBOTG

