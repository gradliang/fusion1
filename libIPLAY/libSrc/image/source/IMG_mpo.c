/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

#include "mpo.h"
#include "taskid.h"

#if MPO //MPO

#define MPO_DEVELOP         0

#if MPO_DEVELOP
    #define MPO_DEBUG       mpDebugPrint
    #define MPO_BREAK       __asm("break 100");
#else
    #define MPO_DEBUG
    #define MPO_BREAK
#endif

#if (MPO_ENCODE == ENABLE)
/*
bUseDecodedMPOInof_Flag:
0: MPO encode with defualt information. 
1: MPO encode with the information of the decoded MPO
*/
BYTE bUseDecodedMPOInof_Flag = USE_DEFAULT_MPO_INFO; 
#endif

#define W_REVERSE(x)    ((((x>>8) & 0xff) | ((x & 0xff) << 8)) & 0xffff)
#define DW_REVERSE(x)   (((x>>24)&0x000000ff) | ((x>>8)&0x0000ff00) | (((x&0x0000ff00)<<8) & 0x00ff0000) | (((x&0x000000ff)<<24)&0xff000000))


#pragma alignvar(4)
ST_JPEG_AddFor_Mpo g_sJpeg_Mpo[2];
ST_JPEG_AddFor_Mpo *g_psCurJpeg_Mpo = &g_sJpeg_Mpo[0];

#pragma alignvar(4)
extern ST_JPEG g_sJpeg[2];
extern ST_JPEG *g_psCurJpeg;

const DWORD EXIF_DATA_TYPE_LENGTH[EXIF_MAX_TYPES_MPO] = {
    0,
    kByteCount,
    kAsciiCount,
    kShortCount,
    kLongCount,
    kRationalCount,
    kSByteCount,
    kUndefineCount,
    kSShortCount,
    kSlongCount,
    kSRationalCount,
    kFloatCount,
    kDoubleCount
};

/*---------------------------------------------------------------*/
int Mpo_Decoder_ImageFile_Init(IMAGEFILE *psImage)
{
	MP_DEBUG("Mpo_Decoder_ImageFile_Init");

	ST_JPEG *psJpeg = Mpo_Decoder_Init(psImage->psStream, psImage->pbTarget, psImage->dwTargetSize, psImage->bDecodeMode, psImage->dwDecodeOffset);

	if(psJpeg->iErrorCode != PASS)			// abel 20100222
		return psJpeg->iErrorCode;

	psImage->ImageDecodeImage = Mpo_Decoder_DecodeImage;	
    psImage->ImageDecodeThumb = Jpg_Decoder_DecodeThumb;  
    psImage->ImageDecodeImageRight = Mpo_Decoder_DecodeImage_Right;
    psImage->ImageDecodeImageClose = Mpo_Decoder_Finish;
	
	psJpeg->wTargetWidth  = psJpeg->wImageWidth;
	psJpeg->wTargetHeight = psJpeg->wImageHeight;

    //App1
    if (psJpeg->dwApp1Pos == 0) {
		if ((psImage->bDecodeMode & 0x3f) == IMG_DECODE_THUMB) {
			return FAIL;//byAlexWang 23apr2008
		}

		psImage->ImageDecodeThumb = Jpg_Decoder_DecodeImage;
	}

    #ifdef WITH_LIB_JPEG
	if ((psJpeg->bProgressive != 0) ||(psJpeg->bCMYK==1)) 
	{
  		MP_ALERT("psJpeg->bProgressive = %d !!", psJpeg->bProgressive);
			//psImage->ImageDecodeImage = Jpg_Decode_libJpeg;
			psImage->ImageDecodeImage = Libjpeg_DecodeImage;
			psImage->LibjpegCallback = LibjpegCallback_Check;
			//psImage->bDecodeMode |= IMG_DECODE_ONEPASS ;
      if (psJpeg->bProgressive == 1)
      {
#if IMAGE_SW_DECODER
      		if (psJpeg->wTargetWidth * psJpeg->wTargetHeight > MAX_PROGRESSIVE_RESOLUTION)
          {
 						MP_ALERT("progressive jpeg out of resolution psJpeg->wTargetWidth %d, psJpeg->wTargetHeight %d",psJpeg->wTargetWidth, psJpeg->wTargetHeight);
            psJpeg->iErrorCode = IMAGE_TYPE_JPEG_PROGRESSIVE;
            return psJpeg->iErrorCode;
					}
#else
          psJpeg->iErrorCode = IMAGE_TYPE_JPEG_PROGRESSIVE;
          return psJpeg->iErrorCode;
#endif
	}
        }
	//else  if ((psJpeg->dwCduControl & VFMT_MASK) == VIDEO_444)//Jasmine 071017: 444 format, image height cut 32
  //      psJpeg->wImageHeight -= 32;//moved by AlexWang 28nov2007
#endif

	psImage->psJpeg = (void *)psJpeg;
	psImage->bNityDegree = psJpeg->blNintyDegreeFlag;
	psImage->wImageWidth = psJpeg->wImageWidth;
	psImage->wImageHeight = psJpeg->wImageHeight;
	psImage->wTargetWidth = psJpeg->wTargetWidth;
	psImage->wTargetHeight = psJpeg->wTargetHeight;
	psImage->wThumbWidth = psJpeg->wThumbWidth;
	psImage->wThumbHeight = psJpeg->wThumbHeight;
	MP_DEBUG2("image %d x %d", psImage->wImageWidth, psImage->wImageHeight);
	//MP_DEBUG2("thumb W %d, tH %d", psImage->wThumbWidth, psImage->wThumbHeight);
	//MP_DEBUG2("target W %d, tH %d", psImage->wTargetWidth, psImage->wTargetHeight);

	return PASS; //psJpeg->iErrorCode;	
}


/*------------------------------------------------------*/
void Mpo_ParseAttrIFD(ST_MPX_STREAM* psStream, ST_JPEG *psJpeg, BOOL bigEndian, DWORD ifdStart)
{
    WORD entryCount = 0, tagId, dataType;
    DWORD dataLength, dataOffset;

    MPO_DEBUG("Mpo_ParseAttrIFD -");

    //ifd entry count
    entryCount = mpxStreamReadWord(psStream);
    if(!bigEndian)
        entryCount = W_REVERSE(entryCount);

    while (entryCount)
    {
        tagId = mpxStreamReadWord(psStream);
        dataType = mpxStreamReadWord(psStream);
        dataLength = mpxStreamReadDWord(psStream);
        dataOffset = mpxStreamReadDWord(psStream);

        if(!bigEndian)
        {
            tagId = W_REVERSE(tagId);
            dataType = W_REVERSE(dataType);
            dataLength = DW_REVERSE(dataLength);
            dataOffset = DW_REVERSE(dataOffset);
        }

        MPO_DEBUG("tag: %x, type: %d, length: %d, offset: %d", tagId, dataType, dataLength, dataOffset);
        entryCount--;
    }
}


/*-----------------------------------------------------*/
DWORD Mpo_ParseIndexIFD(ST_MPX_STREAM* psStream, ST_JPEG *psJpeg, BOOL bigEndian, DWORD ifdStart)
{
    WORD entryCount = 0, tagId, dataType;
    ST_MPO_DATA *mpoData;
    DWORD dataLength, dataOffset;
    DWORD i;
    DWORD nextIFDOffset = 0;
    BOOL supported = TRUE;

#if (MPO_ENCODE == ENABLE)
	DWORD GetValue = NULL;
	DWORD ValueOffset;
	DWORD CurOffset;

	extern ST_JPG_EXIF_TAG_INFO *Decode_MpIndexIfd_ptr;
	extern WORD g_NoOfMpoIndexIfd;
	ST_JPG_EXIF_TAG_INFO *Get_Decode_MpIndexIfd_ptr;

	Get_Decode_MpIndexIfd_ptr = Decode_MpIndexIfd_ptr;	
	g_NoOfMpoIndexIfd = 0;
#endif
    psJpeg->iErrorCode = 0;
	
    //ifd entry count
    entryCount = mpxStreamReadWord(psStream);

    if(!bigEndian)
        entryCount = W_REVERSE(entryCount);

    MPO_DEBUG("Num of Index IFD = %d", entryCount);

    mpoData = (ST_MPO_DATA *) ext_mem_malloc(sizeof(ST_MPO_DATA));

    if (!mpoData)
    {
        MPO_DEBUG("Out of memory at Mpo_ParseIndexIFD");
        psJpeg->iErrorCode = JPEG_FILE_ERROR;

        return 0;
    }

    while (entryCount)
    {
        tagId = mpxStreamReadWord(psStream);
        dataType = mpxStreamReadWord(psStream);
        dataLength = mpxStreamReadDWord(psStream);
        dataOffset = mpxStreamReadDWord(psStream);

        if(!bigEndian)
        {
            tagId = W_REVERSE(tagId);
            dataType = W_REVERSE(dataType);
            dataLength = DW_REVERSE(dataLength);
            dataOffset = DW_REVERSE(dataOffset);
        }

        MPO_DEBUG("tag: 0x%04X, type: %d, length: %d, offset: 0x%08X", tagId, dataType, dataLength, dataOffset);
        dataLength = EXIF_DATA_TYPE_LENGTH[dataType] * dataLength;

        switch(tagId)
        {
        case TAG_MPF_VERSION:
            MPO_DEBUG("TAG_MPF_VERSION");

            MPO_DEBUG("MPF version: 0x%X, %c%c%c%c", dataOffset,
													(dataOffset >> 24) & 0xff,
                                                     (dataOffset >> 16) & 0xff,
                                                     (dataOffset >> 8) & 0xff,
                                                     dataOffset & 0xff);
#if (MPO_ENCODE == ENABLE)
			if((bUseDecodedMPOInof_Flag == USE_DECODED_MPO_INFO) && (g_NoOfMpoIndexIfd < MAX_NUM_OF_MpIndexIfd))
			{
				Get_Decode_MpIndexIfd_ptr->u16Tag	      = 0xB000;
				Get_Decode_MpIndexIfd_ptr->u16DataType     = EXIF_TYPE_UNDEFINED_MPO;
				Get_Decode_MpIndexIfd_ptr->u32DataLength   = 4;
				
				Get_Decode_MpIndexIfd_ptr->u08BufferPtr[0] = (BYTE)(dataOffset >> 24) & 0xff;
				Get_Decode_MpIndexIfd_ptr->u08BufferPtr[1] = (BYTE)(dataOffset >> 16) & 0xff;
				Get_Decode_MpIndexIfd_ptr->u08BufferPtr[2] = (BYTE)(dataOffset >> 8) & 0xff;
				Get_Decode_MpIndexIfd_ptr->u08BufferPtr[3] = (BYTE)(dataOffset >> 0) & 0xff;
				
				Get_Decode_MpIndexIfd_ptr->Value1[0]       = 0;
				Get_Decode_MpIndexIfd_ptr->Value2[0]       = 0;
				Get_Decode_MpIndexIfd_ptr->unsavedFlag     = 0;
				
				Get_Decode_MpIndexIfd_ptr++;
				g_NoOfMpoIndexIfd++;
			}
#endif

        break;

        case TAG_NUMBER_OF_IMAGES:
            MPO_DEBUG("TAG_NUMBER_OF_IMAGES");

            if (dataLength > 4)
            {
                MPO_DEBUG("error data length of TAG_NUMBER_OF_IMAGES");
                supported = FALSE;
                //MPO_BREAK
            }

#if (MPO_ENCODE == ENABLE)
			if((bUseDecodedMPOInof_Flag == USE_DECODED_MPO_INFO) && (g_NoOfMpoIndexIfd < MAX_NUM_OF_MpIndexIfd))
			{
				/*for Smart copy is 2 images.*/
				Get_Decode_MpIndexIfd_ptr->u16Tag	      = 0xB001;
				Get_Decode_MpIndexIfd_ptr->u16DataType     = EXIF_TYPE_LONG_MPO;
				Get_Decode_MpIndexIfd_ptr->u32DataLength   = 1;
			
				Get_Decode_MpIndexIfd_ptr->u08BufferPtr[0] = (BYTE)(0x00000002 >> 24) & 0xff;
				Get_Decode_MpIndexIfd_ptr->u08BufferPtr[1] = (BYTE)(0x00000002 >> 16) & 0xff;
				Get_Decode_MpIndexIfd_ptr->u08BufferPtr[2] = (BYTE)(0x00000002 >> 8) & 0xff;
				Get_Decode_MpIndexIfd_ptr->u08BufferPtr[3] = (BYTE)(0x00000002 >> 0) & 0xff;
			
				Get_Decode_MpIndexIfd_ptr->Value1[0]       = 0;
				Get_Decode_MpIndexIfd_ptr->Value2[0]       = 0;
				Get_Decode_MpIndexIfd_ptr->unsavedFlag     = 0;
			
				Get_Decode_MpIndexIfd_ptr++;
				g_NoOfMpoIndexIfd++;
			}
#endif/*MPO_ENCODE == ENABLE*/
            mpoData->dwImageNumber = dataOffset;

            MPO_DEBUG("%d images in file", mpoData->dwImageNumber);
        break;

        case TAG_MP_ENTRY:
            MPO_DEBUG("TAG_MP_ENTRY");

            if(dataLength <= 4)
            {
                MPO_DEBUG("error data length of TAG_MP_ENTRY");
                supported = FALSE;
            }

#if (MPO_ENCODE == ENABLE)
			if((bUseDecodedMPOInof_Flag == USE_DECODED_MPO_INFO) && (g_NoOfMpoIndexIfd < MAX_NUM_OF_MpIndexIfd))
			{
				/*for Smart copy is 2 images.*/
				Get_Decode_MpIndexIfd_ptr->u16Tag	      = 0xB002;
				Get_Decode_MpIndexIfd_ptr->u16DataType     = EXIF_TYPE_UNDEFINED_MPO;
				Get_Decode_MpIndexIfd_ptr->u32DataLength   = 32;
			
				/*Just store the  "Image Attribute" of the original MPO*/
				ValueOffset = dataOffset + ifdStart;
				CurOffset = mpxStreamTell(psStream);
						
				mpxStreamSeek(psStream, ValueOffset, SEEK_SET);
			
				GetValue = mpxStreamReadDWord(psStream);
			
				if(!bigEndian)
				{
					GetValue = DW_REVERSE(GetValue);
				}
						
				MP_DEBUG1("IMG 1 =0x%x",GetValue);
						
				Get_Decode_MpIndexIfd_ptr->u08BufferPtr[0] = (BYTE)(GetValue >> 24) & 0xff;
				Get_Decode_MpIndexIfd_ptr->u08BufferPtr[1] = (BYTE)(GetValue >> 16) & 0xff;
				Get_Decode_MpIndexIfd_ptr->u08BufferPtr[2] = (BYTE)(GetValue >> 8) & 0xff;
				Get_Decode_MpIndexIfd_ptr->u08BufferPtr[3] = (BYTE)(GetValue >> 0) & 0xff;
			
				/*Image 2*/
				ValueOffset += 16;
				mpxStreamSeek(psStream, ValueOffset, SEEK_SET);
			
				GetValue = mpxStreamReadDWord(psStream);
			
				if(!bigEndian)
				{
					GetValue = DW_REVERSE(GetValue);
				}
				MP_DEBUG2("##### dataOffset =0x%x, ifdStart=0x%x",dataOffset,ifdStart);
				MP_DEBUG1("IMG 2=0x%x",GetValue);
						
			 	Get_Decode_MpIndexIfd_ptr->u08BufferPtr[16] = (BYTE)(GetValue >> 24) & 0xff;
				Get_Decode_MpIndexIfd_ptr->u08BufferPtr[17] = (BYTE)(GetValue >> 16) & 0xff;
				Get_Decode_MpIndexIfd_ptr->u08BufferPtr[18] = (BYTE)(GetValue >> 8) & 0xff;
				Get_Decode_MpIndexIfd_ptr->u08BufferPtr[19] = (BYTE)(GetValue >> 0) & 0xff;
									
				Get_Decode_MpIndexIfd_ptr->Value1[0]        = 0;
				Get_Decode_MpIndexIfd_ptr->Value2[0]        = 0;
				Get_Decode_MpIndexIfd_ptr->unsavedFlag      = 0;
			
				Get_Decode_MpIndexIfd_ptr++;
				g_NoOfMpoIndexIfd++;
				
				mpxStreamSeek(psStream, CurOffset, SEEK_SET);/*Restore */
			}
#endif/*(MPO_ENCODE == ENABLE)*/
			
            mpoData->dwMPEntryOffset = dataOffset + ifdStart;
        break;

        case TAG_IMAGE_UID_LIST:
            MPO_DEBUG("TAG_IMAGE_UID_LIST");

            if(dataLength <= 4)
            {
                MPO_DEBUG("error data length of TAG_IMAGE_UID_LIST");
                supported = FALSE;
            }

            mpoData->dwImageUIDOffset = dataOffset + ifdStart;
        break;

        case TAG_TOTAL_FRAMES:
            MPO_DEBUG("TAG_TOTAL_FRAMES");

            if(dataLength > 4)
            {
                MPO_DEBUG("error data length of TAG_TOTAL_FRAMES");
                supported = FALSE;
            }

            mpoData->dwTotalFrames = dataOffset;
            MPO_DEBUG("%d Frames in file", mpoData->dwTotalFrames);
        break;
        }

        entryCount--;
    }

    if (!supported)
    {
        psJpeg->iErrorCode = JPEG_FILE_ERROR;
        ext_mem_free(mpoData);

        return 0;
    }

    nextIFDOffset = mpxStreamReadDWord(psStream);

    if(!bigEndian)
        nextIFDOffset = DW_REVERSE(nextIFDOffset);

    if (nextIFDOffset)
        nextIFDOffset += ifdStart;

    mpoData->pEntry = (ST_MPO_ENTRY *) ext_mem_malloc(sizeof(ST_MPO_ENTRY) * mpoData->dwImageNumber);

    if (!mpoData->pEntry)
    {
        psJpeg->iErrorCode = JPEG_FILE_ERROR;
        ext_mem_free(mpoData);

        return 0;
    }

    mpxStreamSeek(psStream, mpoData->dwMPEntryOffset, SEEK_SET);

    for (i = 0; i < mpoData->dwImageNumber; i++)
    {
        mpoData->pEntry[i].dwAttribute = mpxStreamReadDWord(psStream);
        mpoData->pEntry[i].dwImageSize = mpxStreamReadDWord(psStream);
        mpoData->pEntry[i].dwOffset = mpxStreamReadDWord(psStream);
        mpoData->pEntry[i].wDependent1 = mpxStreamReadWord(psStream);
        mpoData->pEntry[i].wDependent2 = mpxStreamReadWord(psStream);

        if (!bigEndian)
        {
            mpoData->pEntry[i].dwAttribute = DW_REVERSE(mpoData->pEntry[i].dwAttribute);
            mpoData->pEntry[i].dwImageSize = DW_REVERSE(mpoData->pEntry[i].dwImageSize);
            mpoData->pEntry[i].dwOffset = DW_REVERSE(mpoData->pEntry[i].dwOffset);
            mpoData->pEntry[i].wDependent1 = W_REVERSE(mpoData->pEntry[i].wDependent1);
            mpoData->pEntry[i].wDependent2 = W_REVERSE(mpoData->pEntry[i].wDependent2);
        }

        mpoData->pEntry[i].dwOffset += ifdStart;

        MPO_DEBUG("  image %d", i);
        MPO_DEBUG("  attr: %x", mpoData->pEntry[i].dwAttribute);
        MPO_DEBUG("  size: %d", mpoData->pEntry[i].dwImageSize);
        MPO_DEBUG("  offset: 0x%X", mpoData->pEntry[i].dwOffset);
    }

#if (MPO_ENCODE == ENABLE)
	MP_DEBUG1("########## bUseDecodedMPOInof_Flag =%d",bUseDecodedMPOInof_Flag);
	MP_DEBUG1("########## g_NoOfMpoIndexIfd=%d",g_NoOfMpoIndexIfd);
#endif	
    g_psCurJpeg_Mpo->mpoPrivate = mpoData;
	
    return nextIFDOffset;
}


/*--------------------------------------------------------------*/
int Mpo_ParseAPP2(ST_MPX_STREAM* psStream, ST_JPEG *psJpeg)
{
    BOOL finish = FALSE;
    WORD fieldLength = 0;
    DWORD mpFormatId = 0;
    ST_MP_HEADER stMPHeader;
    BOOL bigEndian = TRUE;
    DWORD ifdStart = 0;

    MPO_DEBUG("\r\nMpo_ParseAPP2 -");

    //if (!psJpeg->dwFirstApp2Pos)
    if(!g_psCurJpeg_Mpo->dwFirstApp2Pos)
    {
        MPO_DEBUG("No any APP2 Information !!!");

        return FAIL;
    }

    mpxStreamSeek(psStream, g_psCurJpeg_Mpo->dwFirstApp2Pos/*psJpeg->dwFirstApp2Pos*/, SEEK_SET);

    fieldLength = mpxStreamReadWord(psStream);
    mpFormatId = mpxStreamReadDWord(psStream);

    if(mpFormatId != MP_FORMAT_ID)
    {
        MPO_DEBUG("Invalid MP Format ID");

        return FAIL;
    }
    else
    {
        DWORD offsetNextIFD = 0;

        ifdStart = mpxStreamTell(psStream);
        stMPHeader.dwMPEndian = mpxStreamReadDWord(psStream);
        stMPHeader.dwOffsetFirstIFD = mpxStreamReadDWord(psStream);

        if(stMPHeader.dwMPEndian == MP_LITTLE_ENDIAN)
        {
            MPO_DEBUG("little endian");
            bigEndian = FALSE;
        }
        else if(stMPHeader.dwMPEndian == MP_BIG_ENDIAN)
        {
            MPO_DEBUG("big endian");
            bigEndian = TRUE;
        }
        else
        {
            MPO_DEBUG("error");
            return FAIL;
        }

        if(!bigEndian)
            stMPHeader.dwOffsetFirstIFD = DW_REVERSE(stMPHeader.dwOffsetFirstIFD);

        //jump to IFD
        if(stMPHeader.dwOffsetFirstIFD < 8)
        {
            MPO_DEBUG("Error IFD offset");
            return FAIL;
        }

        
        stMPHeader.dwOffsetFirstIFD += ifdStart;
        mpxStreamSeek(psStream, stMPHeader.dwOffsetFirstIFD, SEEK_SET);

        stMPHeader.dwOffsetFirstIFD = Mpo_ParseIndexIFD(psStream, psJpeg, bigEndian, ifdStart);

        if (psJpeg->iErrorCode)
        {
            MPO_DEBUG("unsupported MPO");
            return FAIL;
        }

        if (stMPHeader.dwOffsetFirstIFD)
        {
            mpxStreamSeek(psStream, stMPHeader.dwOffsetFirstIFD, SEEK_SET);
            Mpo_ParseAttrIFD(psStream, psJpeg, bigEndian, ifdStart);
        }
    }

    return PASS;
}


/*--------------------------------------------------------------*/
ST_JPEG * Mpo_Decoder_Init(ST_MPX_STREAM *psStream, BYTE * pbTarget, DWORD dwTargetSize, BYTE bMode, DWORD dwOffset)
{
	ST_JPEG *psJpeg;

	if ((bMode & 0x3f) != IMG_DECODE_XPG)// && (bMode & 0x3f) != IMG_DECODE_THUMB)
		psJpeg = (ST_JPEG *)&g_sJpeg[0];
	else
		psJpeg = (ST_JPEG *)&g_sJpeg[1];

	g_psCurJpeg = psJpeg;
	//MP_DEBUG5("Jpg_Decode_Init %08x %08x %08x %d %d", pbTarget, psStream, dwTargetSize, bMode, dwOffset);

	memset(psJpeg, 0, sizeof(ST_JPEG));

    g_psCurJpeg_Mpo = &g_sJpeg_Mpo[0];
	memset(g_psCurJpeg_Mpo, 0, sizeof(ST_JPEG_AddFor_Mpo));

	psJpeg->pbTarget = pbTarget;
	psJpeg->pbSource = psStream->buffer;
	psJpeg->psStream = psStream;
	psJpeg->dwSourceSize = psStream->buf_max_size;
	psJpeg->dwTargetSize = dwTargetSize;
	psJpeg->bDecodeMode = bMode;
	psJpeg->dwOffset = dwOffset;
	psJpeg->dwCduControl = 0;

	if ((bMode & 0x3f) != IMG_DECODE_THUMB1)
	{
		mpxStreamSeek(psStream, 0, SEEK_SET);		
	}	
	
	psJpeg->iErrorCode = Jpg_Parse_Marker(psStream, psJpeg);		

    MPO_DEBUG("------------ Parse APP2-----");
      if (FAIL == Mpo_ParseAPP2(psStream, psJpeg))
      {
	  	MP_ALERT("%s: NO APP2!! " , __FUNCTION__ );
        psJpeg->iErrorCode = FAIL;
      }
	
	return psJpeg;
}

/*------------------------------------------------------------*/
int Mpo_Decoder_DecodeImage(IMAGEFILE *psImage)
{
	ST_JPEG *psJpeg;
	MP_ASSERT(psImage != NULL);
	MP_ASSERT(psImage->psJpeg != NULL);
	MP_DEBUG("Jpg_Decoder_DecodeImage");
	psJpeg = (ST_JPEG *)psImage->psJpeg;
	psJpeg->bDecodeMode = psImage->bDecodeMode;
	psJpeg->pbTarget = psImage->pbTarget;
	psJpeg->dwTargetSize = psImage->dwTargetSize;

	SemaphoreWait(JPEG_CDU_SEMA_ID);
	int iErrorCode = Jpg_Decoder_CduDecode(psJpeg);
    SemaphoreRelease(JPEG_CDU_SEMA_ID);

	psImage->bNityDegree = psJpeg->blNintyDegreeFlag;
	psImage->wImageWidth = psJpeg->wImageWidth;
	psImage->wImageHeight = psJpeg->wImageHeight;
	psImage->wTargetWidth = psJpeg->wTargetWidth;
	psImage->wTargetHeight = psJpeg->wTargetHeight;
	psImage->wRealTargetWidth = psJpeg->wRealTargetWidth;
	psImage->wRealTargetHeight = psJpeg->wRealTargetHeight;
	psImage->wThumbWidth = psJpeg->wThumbWidth;//psJpeg->wTargetWidth;
	psImage->wThumbHeight = psJpeg->wThumbHeight;//psJpeg->wTargetHeight;
	psImage->bScaleDown = psJpeg->bJResizeRatio;

	MP_DEBUG("Target W %d, H %d", psImage->wTargetWidth, psImage->wTargetHeight);
	MP_DEBUG("Real W %d, H %d", psImage->wRealTargetWidth, psImage->wRealTargetHeight);
	MP_DEBUG("Thumb W %d, H %d", psImage->wThumbWidth, psImage->wThumbHeight);
	MP_DEBUG("Scale down = %d", psImage->bScaleDown);

	return iErrorCode;
}

/*-------------------------------------------------------------*/
int Mpo_Decoder_DecodeImage_Right(IMAGEFILE *psImage)
{
    ST_JPEG *psJpeg;
	int iErrorCode;
	ST_MPO_DATA* mpoData = NULL;

	MP_DEBUG("Mpo_Decoder_DecodeImage_Right -");

	psJpeg = (ST_JPEG *)psImage->psJpeg;
	psJpeg->bDecodeMode = psImage->bDecodeMode;
	psJpeg->pbTarget = psImage->pbTarget;
	psJpeg->dwTargetSize = psImage->dwTargetSize;

	mpoData = (ST_MPO_DATA*)g_psCurJpeg_Mpo->mpoPrivate;
	MPO_DEBUG("%s-mpoData->pEntry[1].dwOffset=0x%x ", __FUNCTION__ , mpoData->pEntry[1].dwOffset);
    mpxStreamSeek(psImage->psStream, mpoData->pEntry[1].dwOffset, SEEK_SET);
    psJpeg->iErrorCode = Jpg_Parse_Marker(psImage->psStream, psJpeg);

	SemaphoreWait(JPEG_CDU_SEMA_ID);
	iErrorCode = Jpg_Decoder_CduDecode(psJpeg);
	SemaphoreRelease(JPEG_CDU_SEMA_ID);

	psImage->bNityDegree = psJpeg->blNintyDegreeFlag;
	psImage->wImageWidth = psJpeg->wImageWidth;
	psImage->wImageHeight = psJpeg->wImageHeight;
	psImage->wTargetWidth = psJpeg->wTargetWidth;
	psImage->wTargetHeight = psJpeg->wTargetHeight;
	psImage->wRealTargetWidth = psJpeg->wRealTargetWidth;
	psImage->wRealTargetHeight = psJpeg->wRealTargetHeight;
	psImage->wThumbWidth = psJpeg->wThumbWidth;//psJpeg->wTargetWidth;
	psImage->wThumbHeight = psJpeg->wThumbHeight;//psJpeg->wTargetHeight;
	psImage->bScaleDown = psJpeg->bJResizeRatio;

	return iErrorCode;
}


/*----------------------------------------------------------------*/
int Mpo_Decoder_Finish(IMAGEFILE* psImage)
{
    ST_JPEG *psJpeg = (ST_JPEG *)psImage->psJpeg;
    ST_MPO_DATA *mpoData = g_psCurJpeg_Mpo->mpoPrivate;
    if(g_psCurJpeg_Mpo->mpoPrivate)
    {
        if(mpoData->pEntry)
        {
            ext_mem_free(mpoData->pEntry);
            mpoData->pEntry = NULL;
        }

        ext_mem_free(g_psCurJpeg_Mpo->mpoPrivate);
        g_psCurJpeg_Mpo->mpoPrivate = NULL;
    }
}



/*---------------------------------------------------------------*/
/*For ImagePlayer API*/


BOOL ImageDraw_Check3DFile(BYTE* extension)
{
    if (UtilStringCompareWoCase08((BYTE *) &extension[1], "MPO", 4))
         return TRUE;
    else
        return FALSE;
}


BYTE ImageDraw_CheckSize_3D(WORD srcWidth, WORD srcHeight, DWORD check_size, BYTE bThumbnail, WORD dstWidth, WORD dstHeight)
{
	BYTE i = 0;
	ST_IMGWIN *ImgWin = NULL;
	DWORD image_size;
	WORD w,h;

	image_size = srcWidth * srcHeight * 2;

	if(bThumbnail)
	{
		for(i=0;i<8;i++)
		{
			if ((image_size>>(i*2)) <= check_size)
			break;
		}

		MP_DEBUG("ImageDraw_CheckSize_3D - Decoding a ThumbNail psImage->bScaleDown is %d", i);
		return i;
	}
	else
	{
		if((dstWidth == 0) || (dstHeight == 0))
		{
		    ImgWin = Idu_GetNextWin();//byAlexWang 23apr2008
    		w = ImgWin->wWidth;
    		h = ImgWin->wHeight;
        }
        else
        {
            w = dstWidth;
            h = dstHeight;
        }

		if ((w == 480) ||(w == 320))//byAlexWang 23apr2008 for better quality for a small panel
		{
			w=720;
			h=480;
		}

		for(i=7;i>0;i--)
		{
			if(((srcWidth>>i)>=w)||((srcHeight>>i)>=h))
			break;
		}

		while((image_size>>(i*2)) > check_size)
		    i++;

		MP_DEBUG("ImageDraw_CheckSize_3D - Decoding an Image psImage->bScaleDown is %d", i);
		return i;
	}

	return 0;
}


SWORD ImageDraw_Decode_3D_Init(IMAGEFILE *stImageFile,WORD wWidth, WORD wHeight, BYTE bMode)
{
    SDWORD curr;
	STREAM *psHandle;
	BYTE *pbSource;
	DWORD dwSourceSize;
	SDWORD iErrorCode = 0;
    IMAGEFILE *psImageFile = stImageFile;
	
    BYTE tempNameBuffer[32];
    ST_JPEG *psJpeg = NULL;
	BYTE _3DPhoto = 0;

	DWORD iFileIndex;
	iFileIndex = g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex;
	ST_SEARCH_INFO *pSearchInfo = &g_psSystemConfig->sFileBrowser.sImgAndMovFileList[iFileIndex]; 

    MP_DEBUG("ImageDraw_Decode_3D_Init -");

    ImageReleaseAllBuffer();


#if 1 //Frank Lin add ,for Slide show decoding

    if(PASS == FileBrowserGetFileName(pSearchInfo, tempNameBuffer, 32, psImageFile->bExtension))
    {
    	_3DPhoto = 1;
        MP_DEBUG("Name: %s%s", tempNameBuffer, psImageFile->bExtension);
        if(!ImageDraw_Check3DFile(psImageFile->bExtension))
            return FAIL;
    }
    else
        return FAIL;
#else
    _3DPhoto = 1;
#endif
	

    psHandle = (STREAM *)FileBrowser_GetCurImageFile(NULL);
    if(psHandle == NULL)
	    return FAIL;


#if MEM_2M
    dwSourceSize = 64 * 1024;
#else
    dwSourceSize = 256 * 1024;
#endif

    pbSource = ImageAllocSourceBuffer(dwSourceSize + 32);
    if(pbSource)
    {
        iErrorCode = ImageFile_Open(psImageFile, psHandle, pbSource, dwSourceSize, bMode);
        if(iErrorCode == PASS)
        {
	        WORD w, h;

	        w = ALIGN_16(psImageFile->wImageWidth);
	        h = ALIGN_16(psImageFile->wImageHeight);
	        if((0 == w) || (0 == h))
	            iErrorCode = FAIL;
	        else
	        {
	            DWORD dwSizeLimit = 0, dwFreeSpace = 0, dwReserveBuffer = 0, dwRotateBuffer = 0, dwChasingBuffer = 0;
	            ST_IMGWIN *ImgWin = Idu_GetCurrWin();

	            dwChasingBuffer=16*1024*16+512;
				if(_3DPhoto == 1)
	            	dwRotateBuffer=((ImgWin->wWidth *2)*(ImgWin->wHeight+16))+512;
				
	            if (((psImageFile->bNityDegree))||(((psImageFile->bDecodeMode & 0x3f) == IMG_DECODE_THUMB)&&(psImageFile->bFileFormat == IMAGE_TYPE_JPEG)))
		            dwChasingBuffer += 0x18000;

		        //dwReserveBuffer=(dwChasingBuffer>dwRotateBuffer)?dwChasingBuffer:dwRotateBuffer;

				dwReserveBuffer=(dwChasingBuffer>dwRotateBuffer)?dwChasingBuffer:dwRotateBuffer;
				dwReserveBuffer = dwReserveBuffer + ((w >>1)* h>>1);
				
				dwFreeSpace = ext_mem_get_free_space();

                if(dwFreeSpace < dwReserveBuffer)
                {
                    iErrorCode = FAIL;
                    goto ERROR_RETURN;
                }

                dwFreeSpace = ext_mem_get_free_space() - dwReserveBuffer;

                if (IMG_DECODE_SLIDE == bMode)
                    psImageFile->bScaleDown = ImageDraw_CheckSize_3D(w ,h , dwFreeSpace, 0, wWidth, wHeight);
                else
                    psImageFile->bScaleDown = ImageDraw_CheckSize_3D(w ,h , dwFreeSpace, 1, wWidth, wHeight);

	            if (psImageFile->bFileFormat == IMAGE_TYPE_JPEG)
	            {
		            psJpeg = (ST_JPEG *)(psImageFile->psJpeg);

                    if (psJpeg->bProgressive == 1)
                    {
                        iErrorCode = FAIL;
                        goto ERROR_RETURN;
                    }

            		if(psImageFile->bNityDegree)
                    {
                        if(psImageFile->bScaleDown == 5)
                        	psImageFile->bScaleDown = 4 ;
                        else if(psImageFile->bScaleDown > 5)
                        {
                            iErrorCode = FAIL;
                            goto ERROR_RETURN;
                        }
                    }

            	  // NEED FAE check----	if (g_psSystemConfig->sImagePlayer.bZoomInitFlag)
            	  // NEED FAE check----  	psImageFile->bScaleDown = ImageDraw_CheckSize_3D(w , h , dwFreeSpace, 1, wWidth, wHeight);

            		if (((psImageFile->bDecodeMode & 0x3f) == IMG_DECODE_THUMB2)&&(!psJpeg->bProgressive)&&(!psJpeg->blNintyDegreeFlag))
            	    	psImageFile->bScaleDown = ImageDraw_CheckSize_3D(w , h , 200*200*2, 1, wWidth, wHeight);

            		if ((psImageFile->bScaleDown >2) || (w > 8192) || (h > 8192))
            			Jpg_DC_Controller_init();

                    if (psImageFile->bScaleDown > 7)
                    {
                		iErrorCode = FAIL;
                	}
                	else
                	{
                    	if ((psImageFile->bFileFormat == IMAGE_TYPE_JPEG) || (psImageFile->bFileFormat == IMAGE_TYPE_BMP)||(psImageFile->bFileFormat == IMAGE_TYPE_TIFF))
       	                    dwSizeLimit = (ALIGN_16(w>>psImageFile->bScaleDown) * ((h>>psImageFile->bScaleDown)+4 )* 2) + 16384;

                        psImageFile->pbTarget = NULL;

                        if (((psImageFile->bNityDegree))||(((psImageFile->bDecodeMode & 0x3f) == IMG_DECODE_THUMB)&&(psImageFile->bFileFormat == IMAGE_TYPE_JPEG)))
                		{
                			dwSizeLimit = dwSizeLimit + 0x10000;
                			psImageFile->pbNityDegreeTarget = (BYTE *)ImageAllocTargetBuffer(dwSizeLimit);
                			if(psImageFile->pbNityDegreeTarget == NULL)
                			{
                				MP_DEBUG("NityDegreeTarget is NULL");
                				iErrorCode = ERR_MEM_MALLOC;
                			}
                			if (iErrorCode == PASS)
                			{
                    			psImageFile->pbTarget = (BYTE *)((DWORD)psImageFile->pbNityDegreeTarget + 0x10000);
                    			psImageFile->dwTargetSize = dwSizeLimit - 0x10000;

                    			MP_DEBUG("NityDegreeTarget = 0x%08x", psImageFile->pbNityDegreeTarget);
                    			MP_DEBUG("Target = 0x%08x", psImageFile->pbTarget);
                    			MP_DEBUG("SizeLimit = 0x%08x", dwSizeLimit);
                    			MP_DEBUG("TargetSize = 0x%08x", psImageFile->dwTargetSize);
                		    }
                		}
                		else
                		{
                	    	psImageFile->pbTarget = (BYTE *)ImageAllocTargetBuffer((dwSizeLimit < dwFreeSpace) ? dwSizeLimit : dwFreeSpace );
                			psImageFile->dwTargetSize = ImageGetTargetSize();
                			MP_DEBUG("Target size is %d", psImageFile->dwTargetSize);
                        }

                        if (psImageFile->pbTarget == NULL)
                        {
                    		iErrorCode = ERR_MEM_MALLOC;
                    	}

                    	if(iErrorCode)
                    	    ImageReleaseTargetBuffer();
                	}
                }
                else
                {
                	iErrorCode = FAIL;
                }
	        }

	    }
        else
            goto ERROR_RETURN;
    }
    else
    {
        goto ERROR_RETURN;
    }

    return PASS;

ERROR_RETURN:
    ImageReleaseSourceBuffer();
    return FAIL;
}


SWORD ImageDraw_Decode_3D_Finish(IMAGEFILE *stImageFile, BOOL bSlideShow)
{
    IMAGEFILE *psImageFile = stImageFile;
	
    ImageFile_Close(psImageFile);
	
    Jpg_DC_Controller_Free();

    //if(bSlideShow)
        ImageReleaseTargetBuffer();
    //NEED FAE Check-----else
    //NEED FAE Check-----    g_psSystemConfig->sImagePlayer.bZoomInitFlag = TRUE;
    return PASS;
}


#if MPO//THREE_D_PROJECT //for FAE Three D project smart copy
void Jpg_GetEXIF_MakerNote(ST_MPX_STREAM *psStream, ST_EXIF *psEXIF,BYTE LittleEndian)
{
	DWORD value1 = 0;
	DWORD value2 = 0;
	BYTE blockStatus = 0;
	/*-----------------------------------------*/
	DWORD MakerOffset, tmpOffset, ValueOffset;
	WORD dwJohn, dwMary, NoOfInterOp, i;
	static WORD faceCount;

	DWORD CountNoOfMakerNote = 0;

	MP_DEBUG1("---- %s ----", __FUNCTION__);

	MakerOffset = mpxStreamTell(psStream);
	//mpDebugPrint("MakerOffset = 0x%x", MakerOffset);
	MP_DEBUG1("MakerOffset =0x%x", MakerOffset);

	dwJohn = mpxStreamReadWord(psStream);
	dwMary = mpxStreamReadWord(psStream);
	
	MP_DEBUG2("%x%x", dwJohn, dwMary);

	if ((dwJohn != 0x4655) || (dwMary != 0x4A49))
		return ;

	dwJohn = mpxStreamReadWord(psStream);
	dwMary = mpxStreamReadWord(psStream);
	MP_DEBUG2("%x%x", dwJohn, dwMary);

	if ((dwJohn != 0x4649) || (dwMary != 0x4C4D))
		return ;
	mpxStreamSkip(psStream, 4);

	if(LittleEndian) NoOfInterOp = mpxStreamReadWord_le(psStream);
		else NoOfInterOp = mpxStreamReadWord(psStream);

    MP_DEBUG1("NoOfMakerNote =%d", NoOfInterOp);
	
	tmpOffset = mpxStreamTell(psStream);

	for (i = 0; i <= NoOfInterOp; i++)
	{
		MP_DEBUG1("-- %d --", i);
		
		mpxStreamSeek(psStream, tmpOffset + i * 12, SEEK_SET);
		if (LittleEndian) 
			dwJohn = mpxStreamReadWord_le(psStream);
		else 
			dwJohn = mpxStreamReadWord(psStream); 
			
		MP_DEBUG1("tag value = 0x%04x", dwJohn);
		switch (dwJohn)
		{
			case 0xB212:
			case 0xB211:
				mpxStreamSkip(psStream, 6);
				
				if( CountNoOfMakerNote < NUM_OF_MakerNote)
				{
				  psEXIF->MakerNoteTag[CountNoOfMakerNote]   = dwJohn;
				  psEXIF->MakerNoteType[CountNoOfMakerNote]  = EXIF_TYPE_SRATIONAL;
				  psEXIF->MakerNoteCount[CountNoOfMakerNote] = 1;
				}				
				if (LittleEndian) 
					dwJohn= ValueOffset = mpxStreamReadDWord_le(psStream);
				else 
					ValueOffset = mpxStreamReadDWord(psStream);

				//Offset to coorindate
				mpxStreamSeek(psStream, MakerOffset + ValueOffset, SEEK_SET);

				MP_DEBUG1("- ValueOffset =0x%x",ValueOffset);
				MP_DEBUG1("- MakerOffset + ValueOffset =0x%x", MakerOffset + ValueOffset);

				if (LittleEndian) 
					value1 = mpxStreamReadDWord_le(psStream);
				else 
					value1 = mpxStreamReadDWord(psStream);

				if (LittleEndian) 
					value2 = mpxStreamReadDWord_le(psStream);
				else 
					value2 = mpxStreamReadDWord(psStream);

				MP_DEBUG1("###### Value1 = 0x%x", value1);
				MP_DEBUG1("###### value2 = 0x%x", value2);
				if( CountNoOfMakerNote < NUM_OF_MakerNote)
				{
				  psEXIF->MakerNoteValue1[CountNoOfMakerNote][0] = value1;
				  psEXIF->MakerNoteValue2[CountNoOfMakerNote][0] = value2;
				  CountNoOfMakerNote++;
				}
				break;
			default:
				break;
		}
		if(blockStatus == 1)
			break;
	}

	MP_DEBUG1("for smart copy : CountNoOfMakerNote =%d", CountNoOfMakerNote);

	psEXIF->NoOfMakerNote = CountNoOfMakerNote;

}

#endif


#if (MPO_ENCODE == ENABLE)
/*
STREAM *fileHandle: file handle.
BYTE *filePrefix:  fine name .  ex. BYTE *filePrefix = {"MPEnc"};
ST_IMGWIN *pLWin: the left image to encode.
ST_IMGWIN *pRWin: the right image to encode.
ST_EXIF *psEXIF:   EXIF information. 
		case 1 : psEXIF == NULL, No EXIF information need to encode in MPO file.
		case 2:  psEXIF != NULL. Encode MPO with EXIF information.
BYTE bUseDecodedMPOInof:
		case 1: bUseDecodedMPOInof = USE_DEFAULT_MPO_INFO: Use the default TAG to encode MPO.
		case 2: bUseDecodedMPOInof = USE_DECODED_MPO_INFO (Smart Copy): Use the decoded TAG of the MPO to encode NEW MPO file.
BYTE bQualityTable: 0~ 6, 7(Standard QT)
   		Table 0 :Quality = 100 (Highest Qulity)
   		   to 
   		Table 6 : Quality = 20  (Lowest Qulity)
   		Table 7: Standard QT  (Lower Qulity)
*/
SDWORD Mpo_FileCreate(STREAM *fileHandle, BYTE *filePrefix, ST_IMGWIN *pLWin, ST_IMGWIN *pRWin, ST_EXIF *psEXIF, ST_EXIF *psEXIF_R, BYTE bUseDecodedMPOInof,BYTE bQualityTable)
{
	int ret = PASS;
	DWORD retCode;
	WORD width, height;
    BYTE *streamBuffer;
    DWORD streamBufSize;
	BYTE defFileName[6] = {"Fuji"};

	Set_MPO_NumOfTag();/*NEED!! set the number of Tag to encode*/
	
	if(bUseDecodedMPOInof == USE_DEFAULT_MPO_INFO)
	{
		/*USE_DEFAULT_MPO_INFO*/	
		JpegMpIndexIfdInstall_UseDefaultInfo();		
		bUseDecodedMPOInof_Flag = USE_DEFAULT_MPO_INFO;
	}
	

	MP_DEBUG("\r\nCreat MPO file -");

    streamBufSize = pLWin->wWidth * pLWin->wHeight * 4; /*the max size of two image(422 Type)*/
    streamBuffer = (BYTE *) ext_mem_malloc(streamBufSize);

    if (!streamBuffer)
    {
        mpDebugPrint("Stream buffer malloc fail !!!\r\n Need %d bytes !!!", streamBufSize);

        return FAIL;
    }

	/*----------- MPO encode -----------------*/

	MP_DEBUG("-- MPO encode with EXIF--");
	streamBufSize = Img2Mpo(streamBuffer, streamBufSize, pLWin, pRWin, psEXIF, psEXIF_R, bQualityTable);

	Release_MPO_NumOfTag();/*NEED!! reset to the initial value.*/


    if (!streamBufSize)
    {
        ext_mem_free(streamBuffer);

        return FAIL;
    }

    MP_DEBUG("MPO file size is %d", streamBufSize);

    if (!fileHandle)
    {
	    DRIVE *sDrv;

        sDrv = DriveGet(SD_MMC);

        if (!filePrefix)
            filePrefix = (BYTE *) &defFileName;

        ret = CreateFile(sDrv, (WORD *) filePrefix, "MPO");

        if (ret)
        {
            mpDebugPrint("Create MPO file fail !!!");
            ext_mem_free(streamBuffer);

            return (SDWORD) ret;
        }

        MP_DEBUG("Create %s.MPO file !!!", (BYTE *) filePrefix);

        fileHandle = FileOpen(sDrv);

        if (!fileHandle)
        {
            mpDebugPrint("Open MPO file fail !!!");
            ext_mem_free(streamBuffer);

            return FAIL;
        }
    }

	if (FileWrite(fileHandle, streamBuffer, streamBufSize ) == 0)
    {
        mpDebugPrint("Write MPO file fail !!!");
        ret = FAIL;
    }

    FileClose(fileHandle);
    ext_mem_free(streamBuffer);

	MP_DEBUG("End of Creat MPO file !!!\r\n");

    return ret;
}

#endif //MPO_ENCODE


#endif //MPO

