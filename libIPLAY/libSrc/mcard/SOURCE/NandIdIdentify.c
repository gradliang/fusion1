#include "global612.h"
#if (((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660)))
#if (NAND_ENABLE || ISP_FUNC_ENABLE)

#include "mpTrace.h"
#include "Mcard.h"
#include "nand.h"

static WORD DeviceIdTable[][2] = {
	//[Id,   ChipSize(MB)]
	//large block
	/*64   MB*/    {0xf0,           64},
	/*128  MB*/    {0xd1,          128},
	/*128  MB*/    {0xf1,          128},
	/*256  MB*/    {0xda,          256},
	/*512  MB*/    {0xdc,          512},
	/*1024 MB*/    {0xd3,         1024},
	/*2048 MB*/    {0xd5,         2048},
	/*4096 MB*/    {0xd7,         4096},
	/*8192 MB*/    {0xd9,         8192},
	{   0,            0}
};

static SWORD NandTraditionId(BYTE *id, DWORD *nMB, DWORD *BlkSz, DWORD *PageSz,
									DWORD *SpareSz, DWORD *ecc_bit, DWORD *plane, DWORD *eccPerSize)
{
	DWORD i;

    if (id[2]&0x0c)   //MLC
    {
        *ecc_bit = 4;
    }
    else
    {
        *ecc_bit = 1;
    }
	*PageSz = (1<<(id[3] & 0x03)) * 1024;
	*SpareSz = (8 << ((id[3] >> 2) & 0x01)) * (*PageSz / 512);
	*BlkSz = 64 * 1024 << ((id[3] >> 4) & 0x03);	// block size
	*plane = 1 << ((id[4] >> 2) & 0x03);
	*eccPerSize = 512;
	i = 0;
	*nMB = 0;
	while(DeviceIdTable[i][0])
	{
		if (DeviceIdTable[i][0] == id[1])
		{
			*nMB = DeviceIdTable[i][1] * NAND_CE_PIN_NR;
			break;
		}
		i++;
	}

	return 0;
}

static SWORD NandHynixHSeries(BYTE *id, DWORD *nMB, DWORD *BlkSz, DWORD *PageSz,
									DWORD *SpareSz, DWORD *ecc_bit, DWORD *plane, DWORD *eccPerSize)
{
	DWORD i;
	DWORD blk  = ((id[3] & 0x80) >> 7) | ((id[3] & 0x30) >> 4);
	DWORD redt = ((id[3] & 0x40) >> 4) | ((id[3] & 0x0C) >> 2);

	*plane = 1 << ((id[4] >> 2) & 0x03);
	*PageSz  = (1 << (id[3] & 0x03)) * 2048;

	switch(blk)
	{
		case 0:
			*BlkSz = 128;
			break;
		case 1:
			*BlkSz = 256;
			break;
		case 2:
			*BlkSz = 512;
			break;
		case 3:
			*BlkSz = 768;
			break;
		case 4:
			*BlkSz = 1024;
			break;
		default:
			*BlkSz = 128;
			break;
	}
	*BlkSz = *BlkSz * 1024;  // Unit :  KB --> B

	switch(redt)
	{
		case 0:
			*SpareSz = 128;
			break;
		case 1:
			*SpareSz = 224;
			break;
		default:
			mpDebugPrint("spare area size is unknown...");
			*SpareSz = 128;
	}

    switch((id[4]&0x70) >> 4)
    {
		case 0:
			*ecc_bit = 1;
			break;
		case 1:
			*ecc_bit = 2;
			break;
		case 2:
			*ecc_bit = 4;
			break;
		case 3:
			*ecc_bit = 8;
			break;
		case 4:
			*ecc_bit = 12;
			break;
		case 5:
			*ecc_bit = 16;
			break;
		default:
			*ecc_bit = 1;
			break;
    }
	*eccPerSize = 512;

	i = 0;
	*nMB = 0;
	while(DeviceIdTable[i][0])
	{
		if (DeviceIdTable[i][0] == id[1])
		{
			*nMB = DeviceIdTable[i][1] * NAND_CE_PIN_NR;
			break;
		}
		i++;
	}
}

static SWORD NandHynixHBSeries(BYTE *id, DWORD *nMB, DWORD *BlkSz, DWORD *PageSz,
									DWORD *SpareSz, DWORD *ecc_bit, DWORD *plane, DWORD *eccPerSize)
{
	DWORD i;
	DWORD blk  = ((id[3] & 0x80) >> 5) | ((id[3] & 0x30) >> 4);
	DWORD redt = ((id[3] & 0x40) >> 4) | ((id[3] & 0x0C) >> 2);

	*plane = 1 << ((id[4] >> 2) & 0x03);
	*PageSz  = (1 << (id[3] & 0x03)) * 2048;

	switch(blk)
	{
		case 0:
			*BlkSz = 128;
			break;
		case 1:
			*BlkSz = 256;
			break;
		case 2:
			*BlkSz = 512;
			break;
		case 3:
			*BlkSz = 768;
			break;
		case 4:
			*BlkSz = 1024;
			break;
		case 5:
			*BlkSz = 2048;
			break;
		default:
			*BlkSz = 128;
			break;
	}
	*BlkSz = *BlkSz * 1024;  // Unit :  KB --> MB

	switch(redt)
	{
		case 0:
			*SpareSz = 128;
			break;
		case 1:
			*SpareSz = 224;
			break;
		case 2:
			*SpareSz = 448;
			break;
		default:
			mpDebugPrint("spare area size is unknown...");
			*SpareSz = 128;
	}

    switch((id[4]&0x70) >> 4)
    {
		case 0:
			*ecc_bit = 1;
			*eccPerSize = 512;
			break;
		case 1:
			*ecc_bit = 2;
			*eccPerSize = 512;
			break;
		case 2:
			*ecc_bit = 4;
			*eccPerSize = 512;
			break;
		case 3:
			*ecc_bit = 8;
			*eccPerSize = 512;
			break;
		case 4:
			*ecc_bit = 16;
			*eccPerSize = 512;
			break;
		case 5:
			*ecc_bit = 24;
			*eccPerSize = 2048;
			break;
		case 6:
			*ecc_bit = 24;
			*eccPerSize = 1024;
			break;
		case 7:
			*ecc_bit = 24;
			*eccPerSize = 1024;
			break;
		default:
			*ecc_bit = 1;
			*eccPerSize = 512;
			break;
    }

	i = 0;
	*nMB = 0;
	while(DeviceIdTable[i][0])
	{
		if (DeviceIdTable[i][0] == id[1])
		{
			*nMB = DeviceIdTable[i][1] * NAND_CE_PIN_NR;
			break;
		}
		i++;
	}
}

static SWORD NandHynixHBBGSeries(BYTE *id, DWORD *nMB, DWORD *BlkSz, DWORD *PageSz,
									DWORD *SpareSz, DWORD *ecc_bit, DWORD *plane, DWORD *eccPerSize)
{
	DWORD i;
	DWORD blk  = ((id[3] & 0x80) >> 5) | ((id[3] & 0x30) >> 4);
	DWORD redt = ((id[3] & 0x40) >> 4) | ((id[3] & 0x0C) >> 2);

	*plane = 1 << ((id[4] >> 2) & 0x03);
	*PageSz  = (1 << (id[3] & 0x03)) * 2048;

	switch(blk)
	{
		case 0:
			*BlkSz = 128;
			break;
		case 1:
			*BlkSz = 256;
			break;
		case 2:
			*BlkSz = 512;
			break;
		case 3:
			*BlkSz = 768;
			break;
		case 4:
			*BlkSz = 1024;
			break;
		case 5:
			*BlkSz = 2048;
			break;
		default:
			*BlkSz = 128;
			break;
	}
	*BlkSz = *BlkSz * 1024;  // Unit :  KB --> MB

	switch(redt)
	{
		case 0:
			*SpareSz = 128;
			break;
		case 1:
			*SpareSz = 224;
			break;
		case 2:
			*SpareSz = 448;
			break;
		case 3:
			*SpareSz = 64;
			break;
		case 4:
			*SpareSz = 32;
			break;
		case 5:
			*SpareSz = 16;
			break;
		case 6:
			*SpareSz = 640;
			break;
		default:
			mpDebugPrint("spare area size is unknown...");
			*SpareSz = 128;
	}

    switch((id[4]&0x70) >> 4)
    {
		case 0:
			*ecc_bit = 1;
			*eccPerSize = 512;
			break;
		case 1:
			*ecc_bit = 1;
			*eccPerSize = 512;
			break;
		case 2:
			*ecc_bit = 2;
			*eccPerSize = 512;
			break;
		case 3:
			*ecc_bit = 4;
			*eccPerSize = 512;
			break;
		case 4:
			*ecc_bit = 8;
			*eccPerSize = 512;
			break;
		case 5:
			*ecc_bit = 40;
			*eccPerSize = 1024;
			break;
		case 6:
			*ecc_bit = 32;
			*eccPerSize = 1024;
			break;
		case 7:
			*ecc_bit = 24;
			*eccPerSize = 1024;
			break;
		default:
			*ecc_bit = 1;
			*eccPerSize = 512;
			break;
    }

	i = 0;
	*nMB = 0;
	while(DeviceIdTable[i][0])
	{
		if (DeviceIdTable[i][0] == id[1])
		{
			*nMB = DeviceIdTable[i][1] * NAND_CE_PIN_NR;
			break;
		}
		i++;
	}
}

static SWORD NandHynixHCAGSeries(BYTE *id, DWORD *nMB, DWORD *BlkSz, DWORD *PageSz,
									DWORD *SpareSz, DWORD *ecc_bit, DWORD *plane, DWORD *eccPerSize)
{
	DWORD i;
	DWORD blk  = ((id[3] & 0x80) >> 5) | ((id[3] & 0x30) >> 4);
	DWORD redt = ((id[3] & 0x40) >> 4) | ((id[3] & 0x0C) >> 2);

	*plane = 1 << ((id[4] >> 2) & 0x03);
	*PageSz  = (1 << (id[3] & 0x03)) * 2048;

	switch(blk)
	{
		case 0:
			*BlkSz = 128;
			break;
		case 1:
			*BlkSz = 256;
			break;
		case 2:
			*BlkSz = 512;
			break;
		case 3:
			*BlkSz = 768;
			break;
		case 4:
			*BlkSz = 1024;
			break;
		case 5:
			*BlkSz = 2048;
			break;
		default:
			*BlkSz = 128;
			break;
	}
	*BlkSz = *BlkSz * 1024;  // Unit :  KB --> MB

	switch(redt)
	{
		case 0:
			*SpareSz = 128;
			break;
		case 1:
			*SpareSz = 224;
			break;
		case 2:
			*SpareSz = 448;
			break;
		case 3:
			*SpareSz = 64;
			break;
		case 4:
			*SpareSz = 32;
			break;
		case 5:
			*SpareSz = 16;
			break;
		case 6:
			*SpareSz = 640;
			break;
		default:
			mpDebugPrint("spare area size is unknown...");
			*SpareSz = 128;
	}

    switch((id[4]&0x70) >> 4)
    {
		case 1:
			*ecc_bit = 1;
			*eccPerSize = 512;
			break;
		case 2:
			*ecc_bit = 2;
			*eccPerSize = 512;
			break;
		case 3:
			*ecc_bit = 4;
			*eccPerSize = 512;
			break;
		case 4:
			*ecc_bit = 8;
			*eccPerSize = 512;
			break;
		case 5:
			*ecc_bit = 24;
			*eccPerSize = 512;
			break;
		case 6:
			*ecc_bit = 32;
			*eccPerSize = 1024;
			break;
		case 7:
			*ecc_bit = 40;
			*eccPerSize = 1024;
			break;
		default:
			*ecc_bit = 1;
			*eccPerSize = 512;
			break;
    }

	i = 0;
	*nMB = 0;
	while(DeviceIdTable[i][0])
	{
		if (DeviceIdTable[i][0] == id[1])
		{
			*nMB = DeviceIdTable[i][1] * NAND_CE_PIN_NR;
			break;
		}
		i++;
	}
}

static SWORD NandSamsungNewId(BYTE *id, DWORD *nMB, DWORD *BlkSz, DWORD *PageSz,
									DWORD *SpareSz, DWORD *ecc_bit, DWORD *plane, DWORD *eccPerSize)
{
	DWORD i;
	DWORD redt = ((id[3] & 0x40) >> 4) | ((id[3] & 0x0C) >> 2);

	*ecc_bit = 1 << ((id[4] & 0x70) >> 4);
	if(*ecc_bit == 32){
		*ecc_bit = 24;
		*eccPerSize = 1024;
	} else if(*ecc_bit == 64){
		*ecc_bit = 40;
		*eccPerSize = 1024;
	} else if(*ecc_bit == 128){
		*ecc_bit = 60;
		*eccPerSize = 1024;
	}

	*PageSz = 2048 << (id[3] & 0x03);
	*BlkSz = 128 * 1024 << (((id[3] & 0x80) >> 5) | ((id[3] & 0x30) >> 4));
	*plane = 1 << ((id[4] >> 2) & 0x03);

	switch(redt)
	{
		case 1 :
			*SpareSz = 128;
			break;

		case 2 :
			*SpareSz = 218;
			break;

		case 3 :
			*SpareSz = 400;
			break;

		case 4 :
			*SpareSz = 436;
			break;

		case 5 :
			*SpareSz = 512;
			break;

		case 6 :
			*SpareSz = 640;
			break;

		default:
			mpDebugPrint("spare area size is unknown...");
			*SpareSz = (16 * (*PageSz / 512));
	}

	i = 0;
	while(DeviceIdTable[i][0])
	{
		if (DeviceIdTable[i][0] == id[1])
		{
			*nMB = DeviceIdTable[i][1];
			break;
		}
		i++;
	}
}


static SWORD NandMicron12AAA(BYTE *id, DWORD *nMB, DWORD *BlkSz, DWORD *PageSz,
									DWORD *SpareSz, DWORD *ecc_bit, DWORD *plane, DWORD *eccPerSize)
{
	DWORD i;

	*ecc_bit = 12;
	*PageSz = (1<<(id[3] & 0x03)) * 1024;
	*SpareSz = 218;
	*BlkSz = 512 * 1024;	// block size
	*plane = *BlkSz;
	*eccPerSize = 1024;
	i = 0;
	*nMB = 0;
	while(DeviceIdTable[i][0])
	{
		if (DeviceIdTable[i][0] == id[1])
		{
			*nMB = DeviceIdTable[i][1] * NAND_CE_PIN_NR;
			break;
		}
		i++;
	}
}

static SWORD NandMicron12ABA(BYTE *id, DWORD *nMB, DWORD *BlkSz, DWORD *PageSz,
									DWORD *SpareSz, DWORD *ecc_bit, DWORD *plane, DWORD *eccPerSize)
{
	*ecc_bit = 12;
	*PageSz = 4 * 1024;
	*SpareSz = 28 * 8;
	*BlkSz = 256 * (*PageSz);	// block size
	*plane = *BlkSz;
	*eccPerSize = 1024;

	if(id[1] == 0x68)
		*nMB = 4 * 1024 * NAND_CE_PIN_NR;
	else if(id[1] == 0x88)
		*nMB = 8 * 1024 * NAND_CE_PIN_NR;
}

static SWORD NandToshiba(BYTE *id, DWORD *nMB, DWORD *BlkSz, DWORD *PageSz,
									DWORD *SpareSz, DWORD *ecc_bit, DWORD *plane, DWORD *eccPerSize)
{
	mpDebugPrint("Nand Toshiba ID");

	// Because Toshiba ID table provide no ECC information, we use hardcode strategy
#if 1	// This setting is used for TC58NVG4D2ETA00
	*ecc_bit    = 24;
	*PageSz     = 8 * 1024;
	*SpareSz    = 376;
	*BlkSz      = 128 * (*PageSz);	// block size
	*eccPerSize = 1024;
	*plane      = 1;
	*nMB        = 2048;
#endif
}

// This table is for hardcode testing only.
static SWORD NandHardCode(BYTE *id, DWORD *nMB, DWORD *BlkSz, DWORD *PageSz,
									DWORD *SpareSz, DWORD *ecc_bit, DWORD *plane, DWORD *eccPerSize)
{
	mpDebugPrint("Nand Hard code");
	*ecc_bit    = 24;
	*PageSz     = 8 * 1024;
	*SpareSz    = 640;
	*BlkSz      = 128 * (*PageSz);
	*eccPerSize = 1024;
	*plane      = 1;
	*nMB        = 4096;
	*nMB        = 8192;
}

void NandIdPaserFunInit(void **IdInfoGet)
{
#if (NAND_ID_TYPE == SAMSUNG_NEW)
	*IdInfoGet = (void *)NandSamsungNewId;

#elif (NAND_ID_TYPE == MICRON_12BITS_AAA)
	*IdInfoGet = (void *)NandMicron12AAA;

#elif (NAND_ID_TYPE == MICRON_12BITS_ABA)
	*IdInfoGet = (void *)NandMicron12ABA;

#elif (NAND_ID_TYPE == TOSHIBA_NAND)
	*IdInfoGet = (void *)NandToshiba;

#elif (NAND_ID_TYPE == HYNIX_H_SERIES)
	*IdInfoGet = (void *)NandHynixHSeries;

#elif (NAND_ID_TYPE == HYNIX_HB_SERIES)
	*IdInfoGet = (void *)NandHynixHBSeries;

#elif (NAND_ID_TYPE == HYNIX_HB_BG_SERIES)
	*IdInfoGet = (void *)NandHynixHBBGSeries;

#elif (NAND_ID_TYPE == HYNIX_HC_AG_SERIES)
	*IdInfoGet = (void *)NandHynixHCAGSeries;

#else
	*IdInfoGet = (void *)NandTraditionId;
#endif

//	*IdInfoGet = (void *)NandHardCode;
}


#endif
#endif

