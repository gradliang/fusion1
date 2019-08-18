
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

/*
// Include section
*/
#include "global612.h"
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#include "mpTrace.h"
#include "Mcard.h"
#include "McardApi.h"
#include "peripheral.h"
#include "Uti.h"
#include "mp650_spi.h"


#define SPI_WRITE_PROTECT_ENABLE       1
/*
// Constant declarations
*/
#ifndef SPI_RESERVED_SIZE
    #define SPI_RESERVED_SIZE       0x10000            // reserve 64KB for bootrom
#endif


// define SPI status bit
#define SPI_WRITE_IN_PROGRESS		0x01
#define SPI_WRITE_ENABLE			0x02
#define SPI_PROTECT_LEVEL_0			0x04
#define SPI_PROTECT_LEVEL_1			0x08
#define SPI_PROTECT_LEVEL_2			0x10
#define SPI_STATUS_WRITE_PROTECT	0x80
// define SPI command
#define SPI_CMD_RDID				0x9F        		// read ID
#define SPI_CMD_RDSR				0x05		        // read status
#define SPI_CMD_WRSR				0x01		        // write status
#define SPI_CMD_WREN				0x06		        // write enable
#define SPI_CMD_WRDI				0x04		        // write disable
#define SPI_CMD_READ				0x03		        // read
#define SPI_CMD_FREAD				0x0B		        // fast read
#define SPI_CMD_PP					0x02		        // page program
#define SPI_CMD_CE                  0xC7                // chip erase
#define SPI_CMD_BE                  0xd8                // block 64KB erase
#define SPI_CMD_SE                  0x20                // sector 4KB erase

#define SPI_WRITE_PAGE_SIZE			0x100		        // for page program size

#define SPI_RW_4K                   0x1000
#define SPI_RW_64K                  0x10000
#define SPI_HW_LIMIT_SIZE           0x400000

/*****************RDID***********************/
/*      Memory density           ID         */
/*          256KB               0x12        */
/*          512KB               0x13        */
/*          1MB                 0x14        */
/*          2MB                 0x15        */
/*          4MB                 0x16        */
/*          8MB                 0x17        */
/*****************RDID***********************/
typedef struct
{
    DWORD dwChipSize;
    //DWORD dwStorageSize;
    DWORD dwCodeRsvdSize;
}ST_SPI_INFO;

static ST_SPI_INFO stSpiInfo;
static BYTE RdSzFlag = 0;


/*
// Variable declarations
*/
static BYTE SPI_Size = 0;
static BYTE bDescriptor[] = "SPI";      // for what???

/*************************Function prototype*******************************************/
static SDWORD spi_wait_module_ready(DWORD);
static SDWORD spi_rd_flash_status(BYTE*);
static SDWORD spi_erase(BYTE, DWORD, SWORD);
static SWORD spi_format(void);
static SDWORD erase_storage(void);
static SDWORD spi_wr_flash_status(BYTE newStatus);

/*************************Function phototype*******************************************/

static SDWORD spi_wait_module_ready(DWORD timeout)
{
	SPI *spi = (SPI *)SPI_BASE;
	DWORD i;

	for(i=0; i < timeout; i++)
	{
		if((spi->SPI_CFG & 0x00000001) == 0)
			return PASS;
	}

	MP_ALERT("Wait SPI module ready Fail");
	return FAIL;

}



void spi_module_reset(void)
{
    BIU *regBiuPtr = (BIU *) BIU_BASE;
    BYTE tmp;

    // Reset SPI
    regBiuPtr->BiuArst |= ARST_SPI;
    regBiuPtr->BiuArst &= ~ARST_SPI;
    for (tmp = 0; tmp < 0x10; tmp++);
    regBiuPtr->BiuArst |= ARST_SPI;
}



void spi_deselect(void)
{
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
    BIU* biu = (BIU*)BIU_BASE;
    CLOCK* ckg = (CLOCK*)CLOCK_BASE;
#if FAST_WRITE_FLASH
    biu->BIU_STRAP_CFG &= ~(BIT1|BIT0) ;
#else
    biu->BIU_STRAP_CFG &= ~BIT1 ;
#endif
    ckg->Clkss_EXT2 &= ~BIT6;                                               // Select the IO group for SPI IO.
#endif
}



// data : fgpio[6..9]
// WP   : fgpio10
// Hold : fgpio11
// CLK  : fgpio12
// CS   : fgpio19
void spi_pin_config(void)
{
	SPI *spi = (SPI *)SPI_BASE;
	GPIO *Gpio = (GPIO *) (GPIO_BASE);
	DWORD memClk;
	DWORD spiClkDiv;
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
    BIU* biu = (BIU*)BIU_BASE ;
    CLOCK* ckg = (CLOCK*)CLOCK_BASE;
#if FAST_WRITE_FLASH
    biu->BIU_STRAP_CFG |= (BIT1|BIT0) ;
#else
    biu->BIU_STRAP_CFG |= BIT1 ;
#endif
    ckg->Clkss_EXT2 |= BIT6;                                                // Select the IO group for SPI IO.

	Gpio->Fgpcfg[0] &= 0xFCBEFCBE;
	Gpio->Fgpcfg[1] &= 0x7FBF7FBF;
#endif

    memClk = Clock_MemFreqGet();                                            // can be fine tuned if higher speed is required
    if(memClk >= 160000000)                                                 // if MemClk >= 160MHz, SPI CLK = MemClk / 8
        spiClkDiv = 0x00000300;
    else if(memClk >= 120000000 && memClk < 160000000)                      // if MemClk >= 120MHz, SPI CLK = MemClk / 6
        spiClkDiv = 0x00000200;
    else if(memClk >= 60000000 && memClk < 120000000)                       // if MemClk >= 60MHz, SPI CLK = MemClk / 4
        spiClkDiv = 0x00000200;                                             // it seems MemClk should be 6 times of SPI CLK. Pending.....20091226
    else
        spiClkDiv = 0;

	spi->SPI_TIME |= (spiClkDiv | 0x00000012);
	spi->SPI_CFG = BIT0;

}



// Read SPI_RX0 , SPI_RX1 to buffer, can't read more than 8 bytes once
static void spi_copy_data(BYTE *Buffer, DWORD size)
{
	DWORD i;
	SPI *spi = (SPI *)SPI_BASE;
	BYTE *tmpBuffer;
	register DWORD tmp0, tmp1;

	tmpBuffer = Buffer;

	tmp0 = spi->SPI_RX0;

	if(size < 4)
	{
		for(i = 0 ; i < size ; i++)
		{
			*tmpBuffer = (BYTE)(tmp0 >> ((3 - i) << 3));
			tmpBuffer++;
		}
	}
	else
	{
		for(i = 0 ; i < 4 ; i++)
		{
			*tmpBuffer = (BYTE)(tmp0 >> ((3 - i) << 3));
			tmpBuffer++;
		}

		tmp0 = spi->SPI_RX1;

		for(i = 0 ; i < (size - 4) ; i++)
		{
			*tmpBuffer = (BYTE)(tmp0 >> ((3 - i) << 3));
			tmpBuffer++;
		}

	}

}



static SDWORD spi_rd_flash_status(BYTE *Status)
{
	SPI *spi = (SPI *)SPI_BASE;

	spi->SPI_TX0 = (SPI_CMD_RDSR << 24);

	spi->SPI_CFG = 0x01010001;

	if(spi_wait_module_ready(0x1000000) != PASS)
	{
	    MP_ALERT("SPI read status fail");
		return FAIL;
	}

	*Status = (BYTE)(spi->SPI_RX0 >> 24);

	return PASS;

}


static SDWORD spi_wr_flash_status(BYTE newStatus)
{//return;
#if SPI_WRITE_PROTECT_ENABLE
	SPI *spi = (SPI *)SPI_BASE;
	spi->SPI_TX0 = (SPI_CMD_WRSR << 24) | ((DWORD) newStatus) << 16;
	spi->SPI_CFG = 0x02000001;

	if(spi_wait_module_ready(0x1000000) != PASS)
	{
	    MP_ALERT("SPI write status fail");
		return FAIL;
	}
//mpDebugPrint("spi_wr_flash_status=%x",(BYTE)(spi->SPI_RX0 >> 24));
#endif
	return PASS;

}
static SDWORD spi_set_flash_wt_enable(void)
{
	SPI *spi = (SPI *)SPI_BASE;

	spi->SPI_TX0 = (SPI_CMD_WREN << 24);
	spi->SPI_CFG = 0x01000001;

	if(spi_wait_module_ready(0x1000000) != PASS)
	{
	    MP_ALERT("SPI set write enable fail");
		return FAIL;
    }

	return PASS;

}



static SDWORD spi_set_flash_wt_disable(void)
{
#if SPI_WRITE_PROTECT_ENABLE
	SPI *spi = (SPI *)SPI_BASE;

	spi->SPI_TX0 = (SPI_CMD_WRDI << 24);
	spi->SPI_CFG = 0x01000001;

	if(spi_wait_module_ready(0x1000000) != PASS)
	{
	    MP_ALERT("SPI set write disable fail");
		return FAIL;
    }
#endif
	return PASS;

}



SDWORD spi_read_id(void)
{
	SPI *spi = (SPI *)SPI_BASE;
	DWORD tmp;

	spi_pin_config();

	spi->SPI_TX0 = (SPI_CMD_RDID << 24);
	spi->SPI_CFG = 0x01030001;

	if(spi_wait_module_ready(0x10000) != PASS)
	{
		spi_deselect();
		return FAIL;
	}

	tmp = spi->SPI_RX0;

    switch((tmp >> 8) & 0x000000FF)
    {
        case 0x14:
            stSpiInfo.dwChipSize = 0x100000;
        break;
        case 0x15:
            stSpiInfo.dwChipSize = 0x200000;
        break;
        case 0x16:
            stSpiInfo.dwChipSize = 0x400000;
        break;
        case 0x17:
            stSpiInfo.dwChipSize = 0x800000;
        break;
        case 0x18:
            stSpiInfo.dwChipSize = 0x1000000;
        break;
        default:
            MP_DEBUG("Unsupport SPI size!!");
            stSpiInfo.dwChipSize = 0x0000000;
            //__asm("break 100");
        break;
    }
    MP_DEBUG("spi size = 0x%x", stSpiInfo.dwChipSize);

	spi_deselect();
	return tmp;
}

SDWORD spi_rx_read(BYTE *Buffer, DWORD size, DWORD addr)
{
	SPI *spi = (SPI *)SPI_BASE;
	DWORD tmpAddr, tmpSize, tmpBuffer, i;

	if(size == 0)
		return PASS;

	spi_pin_config();

	tmpAddr = addr;
	tmpSize = size;
	tmpBuffer = (DWORD)Buffer;

	while(tmpSize)
	{
		if(tmpSize > 8)
			i = 8;
		else
			i = tmpSize;

		spi->SPI_TX0 = (SPI_CMD_READ << 24) | (tmpAddr & 0x00FFFFFF);

		spi->SPI_CFG = 0x04000001 | (i << 16);

		if(spi_wait_module_ready(0x1000000) != PASS)
		{
			spi_deselect();
			return FAIL;
		}

		spi_copy_data((BYTE *)tmpBuffer, i);

		tmpAddr += i;
		tmpBuffer += i;
		tmpSize -= i;

	}

	spi_deselect();

	return PASS;

}



SDWORD spi_tx_write(BYTE *buffer, DWORD size, DWORD addr)
{
	BYTE status;
    DWORD timeout, i;
    SPI* spi = (SPI*)SPI_BASE;

	spi_pin_config();


	while(size)
	{
		if(spi_set_flash_wt_enable() != PASS)
		{
			spi_deselect();
			return FAIL;
		}
		timeout = 0x100000;
		while(timeout)
		{
			if(spi_rd_flash_status(&status) != PASS)
			{
				spi_deselect();
				return FAIL;
			}

			if(status & SPI_WRITE_ENABLE)
				break;

			timeout--;
		}

		if(timeout == 0)
		{
			spi_deselect();
			return FAIL;
		}
		spi->SPI_TX0 = (SPI_CMD_PP << 24) | (addr & 0x00FFFFFF);

		if(size >= 4)
		{
			i = 4;
			spi->SPI_TX1 = *(DWORD*)buffer;
		}
		else
        {
			i = size;
			spi->SPI_TX1 = *(DWORD*)buffer & (0xFFFFFFFF << (i << 3));
		}

		spi->SPI_CFG = 0x00000001 | ((4 + i) << 24);

		addr += i;
		size -= i;
		buffer += 4;

		if(spi_wait_module_ready(0x1000000) != PASS)
		{
			spi_deselect();
			return FAIL;
		}

		timeout = 0x10000;
		while(timeout)
		{
			if(spi_rd_flash_status(&status) != PASS)
			{
				spi_deselect();
				return FAIL;
			}

			if((status & SPI_WRITE_IN_PROGRESS) == 0)
				break;

			timeout--;
		}

		if(timeout == 0)
		{
			spi_deselect();
			return FAIL;
		}
	}

    spi_deselect();
    return PASS;
}



SDWORD spi_isp_tx_wt(BYTE *Buffer, DWORD size, DWORD addr)
{
	SPI* spi = (SPI*)SPI_BASE;
	DWORD tmpAddr, tmpSize, i;
	BYTE* tmpBuffer;
	DWORD timeout;
	DWORD retVal = PASS;
	BYTE eraseCnt;

	if(size == 0)
		return PASS;

    BYTE* memBuf = (BYTE*)ext_mem_malloc(SPI_RW_4K);
    if(memBuf == NULL)
    {
        MP_ALERT("Not enough memory for SPI");
        return FAIL;
    }

	spi_pin_config();

    tmpBuffer = Buffer;

    if(addr & 0xFFF != 0)
    {
        tmpAddr =addr & 0xFFFFF000;
        tmpSize = addr & 0x00000FFF;

        spi_rx_read(memBuf, SPI_RW_4K, tmpAddr);

        spi_sector_erase(tmpAddr, 1);
        for(i = 0; i < (SPI_RW_4K - tmpSize) && i < size; i++)
        {
            *(memBuf + tmpSize + i) = *tmpBuffer;
            tmpBuffer++;
        }
        if(spi_tx_write(memBuf, SPI_RW_4K, tmpAddr) == FAIL)
        {
        	ext_mem_free(memBuf);
            return FAIL;
        }

        tmpAddr += SPI_RW_4K;
        if(size < SPI_RW_4K)
            tmpSize = 0;
        else
            tmpSize = size - (SPI_RW_4K - tmpSize);
    }
    else
    {
    	tmpAddr = addr;
    	tmpSize = size;
    }

    while(tmpSize)
    {
        if(tmpSize > SPI_RW_64K)
        {
            spi_block_erase(tmpAddr, 1);
            if(spi_tx_write(tmpBuffer, SPI_RW_64K, tmpAddr) == FAIL)
            {
                retVal = FAIL;
                break;
            }

            tmpBuffer += SPI_RW_64K;
            tmpSize -= SPI_RW_64K;
            tmpAddr += SPI_RW_64K;
        }
        else
        {
            if(tmpSize < SPI_RW_4K)
            {
                spi_rx_read(memBuf, SPI_RW_4K, tmpAddr);
                for(i = 0 ; i < tmpSize; i++)
                {
                    *(memBuf + i) = *tmpBuffer;
                    tmpBuffer++;
                }
                spi_sector_erase(tmpAddr, 1);
                if(spi_tx_write(memBuf, SPI_RW_4K, tmpAddr) == FAIL)
                {
                    retVal = FAIL;
                }
                break;
            }
            spi_sector_erase(tmpAddr, 1);
            if(spi_tx_write(tmpBuffer, SPI_RW_4K, tmpAddr) == FAIL)
            {
                retVal = FAIL;
                break;
            }

            tmpBuffer += SPI_RW_4K;
            tmpSize -= SPI_RW_4K;
            tmpAddr += SPI_RW_4K;
        }
    }

	ext_mem_free(memBuf);

	spi_deselect();
	return PASS;

}



SDWORD spi_rcmd(BYTE *Buffer, DWORD size, DWORD addr)
{
	SPI *spi = (SPI *)SPI_BASE;
	DWORD tmpSize;
	BYTE *tmpAddr, *tmpBuffer, i;

	if(size == 0)
		return PASS;

	spi_pin_config();

	tmpAddr = (BYTE *)(addr + CODE_FLASH_BASE);
	tmpSize = size;
	tmpBuffer = Buffer;

	spi->SPI_RCMD = (SPI_CMD_FREAD << 24) | 0x00030100;                 // Default fast read command, 3 address bytes, 1 dummy byte, single port.
	spi->SPI_CFG = 0x00000001;

	if(spi_wait_module_ready(0x1000000) != PASS)
	{
		spi_deselect();
		return FAIL;
	}

	while(tmpSize)
	{
		*tmpBuffer = *tmpAddr;
		tmpAddr++;
		tmpBuffer++;
		tmpSize--;

	}

	spi_deselect();
	return PASS;

}



SDWORD spi_isp_wcmd(BYTE *Buffer, DWORD size, DWORD addr)
{
	SPI *spi = (SPI *)SPI_BASE;
	DWORD tmpSize, i, WrSize;
	BYTE *tmpBuffer, *memBuf;
	DWORD tmpAddr;
	DWORD timeout;
	DWORD retVal = PASS;
	BYTE status;

	if(size == 0)
		return PASS;

    memBuf = (BYTE*)ext_mem_malloc(SPI_RW_4K);
    if(memBuf == NULL)
    {
        mpDebugPrint("Not enough memory for SPI");
        return FAIL;
    }

	spi_pin_config();

	tmpBuffer = Buffer;

    if(addr & 0xFFF != 0)                                               // not in  4KB address offset
    {
        tmpAddr = addr & 0xFFFFF000;
        tmpSize = addr & 0x00000FFF;
        spi_rcmd(memBuf, SPI_RW_4K, tmpAddr);
        spi_sector_erase(tmpAddr, 1);
        for(i = 0; i < (SPI_RW_4K - tmpSize) && i < size; i++)
        {
            *(memBuf + tmpSize + i) = *tmpBuffer;
            tmpBuffer++;
        }
        if(spi_wcmd(memBuf, SPI_RW_4K, tmpAddr) == FAIL)
        {
        	ext_mem_free(memBuf);
            return FAIL;
        }

        tmpAddr += SPI_RW_4K;
        if(size < SPI_RW_4K)
            tmpSize = 0;
        else
            tmpSize = size - (SPI_RW_4K - tmpSize);
    }
    else
    {
    	tmpAddr = addr;
    	tmpSize = size;

    }

    //eraseCnt = (tmpSize % 4096 ? (tmpSize >> 12) + 1 : tmpSize >> 12 );

    while(tmpSize)
    {
        if(tmpSize > SPI_RW_64K)
        {
            spi_block_erase(tmpAddr, 1);
            if(spi_wcmd(tmpBuffer, SPI_RW_64K, tmpAddr) == FAIL)
            {
                retVal = FAIL;
                break;
            }

            tmpBuffer += SPI_RW_64K;
            tmpSize -= SPI_RW_64K;
            tmpAddr += SPI_RW_64K;
        }
        else
        {
            if(tmpSize < SPI_RW_4K)
            {
                spi_rcmd(memBuf, SPI_RW_4K, tmpAddr);
                for(i = 0 ; i < tmpSize; i++)
                {
                    *(memBuf + i) = *tmpBuffer;
                    tmpBuffer++;
                }
                spi_sector_erase(tmpAddr, 1);

                if(spi_wcmd(memBuf, SPI_RW_4K, tmpAddr) == FAIL)
                {
                    retVal = FAIL;
                }
                break;
            }
            spi_sector_erase(tmpAddr, 1);
            if(spi_wcmd(tmpBuffer, SPI_RW_4K, tmpAddr) == FAIL)
            {
                retVal = FAIL;
                break;
            }

            tmpBuffer += SPI_RW_4K;
            tmpSize -= SPI_RW_4K;
            tmpAddr += SPI_RW_4K;
        }
    }

	ext_mem_free(memBuf);

	spi_deselect();
	return retVal;

}



SDWORD spi_wcmd(BYTE *Buffer, DWORD size, DWORD addr)
{
	SPI *spi = (SPI *)SPI_BASE;
	DWORD tmpSize, i, WrSize;
	BYTE *tmpBuffer, *tmpAddr;
	DWORD timeout;
	BYTE status;

	if(size == 0)
		return PASS;

	spi_pin_config();

	tmpAddr = (BYTE *)(addr + CODE_FLASH_BASE);
	tmpSize = size;
	tmpBuffer = Buffer;

	while(tmpSize)
	{

		if(tmpSize > SPI_WRITE_PAGE_SIZE)
			WrSize = SPI_WRITE_PAGE_SIZE;
		else
			WrSize = tmpSize;

		tmpSize -= WrSize;

		if(spi_set_flash_wt_enable() != PASS)
		{
			spi_deselect();
			return FAIL;
		}

		timeout = 0x10000;
		while(timeout)
		{
			if(spi_rd_flash_status(&status) != PASS)
			{
				spi_deselect();
				return FAIL;
			}

			if(status & SPI_WRITE_ENABLE)
				break;

			timeout--;
		}

		if(timeout == 0)
		{
			spi_deselect();
			return FAIL;
		}

		spi->SPI_WCMD = (SPI_CMD_PP << 24) | 0x00030000;                    // Default page program command, 3 address bytes, single port.
		spi->SPI_CFG = BIT0;

		while(WrSize)
		{
			*tmpAddr = *tmpBuffer;
			tmpBuffer++;
			tmpAddr++;
			WrSize--;
		}

		timeout = 0x10000;
		while(timeout)
		{
			if(spi_rd_flash_status(&status) != PASS)
			{
				spi_deselect();
			    return FAIL;
			}

			if((status & SPI_WRITE_IN_PROGRESS) == 0)
				break;

			timeout--;
		}

		if(timeout == 0)
		{
			spi_deselect();
			return FAIL;
    	}
	}

	spi_deselect();
	return PASS;

}



SDWORD spi_chip_erase(void)
{
    return spi_erase(SPI_CMD_CE, 0, 1);
}



SDWORD spi_block_erase(DWORD addr, DWORD cnt)
{
    return spi_erase(SPI_CMD_BE, addr, cnt);
}


// addr : erased physical address(Not Sector number).
SDWORD spi_sector_erase(DWORD addr, DWORD cnt)
{
    return spi_erase(SPI_CMD_SE, addr, cnt);
}



// erase type : block(64KB) or sector(4KB) erase
// addr : erased physical address(Not Sector number). Can be any address value, but the sector or block of that address will be all erased.
// cnt : numbers of sector or block to be erased.
static SDWORD spi_erase(BYTE eraseType, DWORD addr, SWORD cnt)
{
	SPI *spi = (SPI *)SPI_BASE;
	DWORD timeout;
	BYTE status;

    if(cnt <= 0)
        return;
	spi_pin_config();

	if(spi_set_flash_wt_enable() != PASS)
	{
		spi_deselect();
		return FAIL;
	}
	timeout = 0x10000;
	while(timeout)
	{
		if(spi_rd_flash_status(&status) != PASS)
		{
			spi_deselect();
			return FAIL;
		}

		if(status & SPI_WRITE_ENABLE)
			break;

		timeout--;
	}
	if(timeout == 0)
	{
	    MP_ALERT("SPI write status not ready");
		spi_deselect();
		return FAIL;
	}

    for(cnt ; cnt > 0 ; cnt--)
    {
        if(eraseType == SPI_CMD_CE)
        {
        	spi->SPI_TX0 = (eraseType << 24);
        	spi->SPI_CFG = 0x01000001;
        }
        else
        {
        	spi->SPI_TX0 = (eraseType << 24) | (addr & 0x00FFFFFF);
        	spi->SPI_CFG = 0x04000001;
        }

    	if(spi_wait_module_ready(0x1000000) != PASS)
    	{
    		spi_deselect();
    		return FAIL;
    	}

    	timeout = 0x1000000;
    	while(timeout)
    	{
    		if(spi_rd_flash_status(&status) != PASS)
    		{
    			spi_deselect();
    			return FAIL;
    		}

    		if((status & SPI_WRITE_IN_PROGRESS) == 0)       // erase done
    			break;

    		timeout--;
    	}

    	if(timeout == 0)
    	{
		    MP_ALERT("erase time out");
        	spi_deselect();
    		return FAIL;
    	}

    	if(eraseType == SPI_CMD_SE)
    	    addr += SPI_RW_4K;                              // add 4KB
    	else
    	    if(eraseType == SPI_CMD_BE)
    	        addr += SPI_RW_64K;                         // add 64KB

    }
    spi_deselect();
	return PASS;

}



DWORD spi_get_size(void)
{
    return stSpiInfo.dwChipSize;
}



/*
void spi_init(void)
{
	//SPI *spi = (SPI *)SPI_BASE;

	spi_module_reset();
    //spi_pin_config();
    spi_read_id();

}
*/


/********************************************Device level****************************************************/
static SDWORD spi_logic_read(DWORD buf, DWORD sectors, DWORD lba)
{
    DWORD phyAddr = 0;

    if(((sectors + lba) << 9) > (stSpiInfo.dwChipSize - stSpiInfo.dwCodeRsvdSize))
    {
        MP_ALERT("-E- SPI read overflow sectors=%d, lba=%d",sectors, lba);
        return FAIL;
    }

    if(stSpiInfo.dwChipSize > SPI_HW_LIMIT_SIZE)
    {
        DWORD tol_sz = (sectors + lba) << 9;

        if(lba < 0x2000 && tol_sz >= SPI_HW_LIMIT_SIZE)                                 // 0x2000 = 0x400000 >> 9
        {
            DWORD read_sz2 = tol_sz % SPI_HW_LIMIT_SIZE;
            DWORD read_sz1 = (sectors << 9) - read_sz2;
            DWORD tmpLba = lba;

            MP_DEBUG("read_sz1=%d,read_sz2=%d,lba=%d",read_sz1,read_sz2,lba);
            if(spi_rx_read((BYTE*)buf, read_sz1, tmpLba << 9))
            {
                MP_ALERT("spi rx read error");
                return FAIL;
            }

            if(spi_rx_read((BYTE*)(buf + read_sz1), read_sz2, (stSpiInfo.dwCodeRsvdSize | SPI_HW_LIMIT_SIZE)))
            {
                MP_ALERT("spi rx read error");
                return FAIL;
            }
        }
        else
        {
            phyAddr = lba << 9;
            if(phyAddr >= SPI_HW_LIMIT_SIZE)
                phyAddr += stSpiInfo.dwCodeRsvdSize;                        // offset code size
            MP_DEBUG("R-phyAddr=0x%x",phyAddr);
            if(spi_rx_read((BYTE*)buf, (sectors << 9), phyAddr))
            {
                MP_ALERT("spi rx read error");
                return FAIL;
            }
        }
    }
    else
    {
        phyAddr = (lba << 9) + stSpiInfo.dwCodeRsvdSize;
        if(spi_rcmd((BYTE*)buf, (sectors << 9), phyAddr))
        {
            MP_ALERT("spi rcmd error");
            return FAIL;
        }
    }

    return PASS;
}



static SDWORD spi_logic_write(DWORD buf, DWORD secCnt, DWORD lba)
{
    BYTE* tmpBuff;
    DWORD AddrOf4KBRange = 0;
    DWORD phyAddr = 0;
    DWORD ret = PASS;

    if(((secCnt + lba) << 9) > (stSpiInfo.dwChipSize - stSpiInfo.dwCodeRsvdSize))
    {
        MP_ALERT("-E- SPI write overflow secCnt=%d, lba=%d",secCnt, lba);
        return FAIL;
    }

    tmpBuff = (BYTE*)ext_mem_malloc(SPI_RW_4K);                                  //allocate for write purpose

    if(tmpBuff == NULL)
    {
        MP_ALERT("Not enough memory for SPI!");
        return FAIL;
    }

    mmcp_memset(tmpBuff, 0, SPI_RW_4K);

    if(stSpiInfo.dwChipSize > SPI_HW_LIMIT_SIZE)
    {
        phyAddr = lba << 9;

        if(phyAddr >= SPI_HW_LIMIT_SIZE)
        {
            phyAddr += stSpiInfo.dwCodeRsvdSize;                                            // offset code size
        }

        AddrOf4KBRange = phyAddr & 0xFFFFF000;                                               // get 4K range
        if(spi_rx_read((BYTE*)tmpBuff, SPI_RW_4K, AddrOf4KBRange))                              // read 4KB out
        {
            ext_mem_free(tmpBuff);
            return FAIL;
        }
    }
    else
    {
        phyAddr = stSpiInfo.dwCodeRsvdSize + (lba << 9);
        AddrOf4KBRange = phyAddr & 0xFFFFF000;
        if(spi_rcmd((BYTE*)tmpBuff, SPI_RW_4K, AddrOf4KBRange))
        {
            ext_mem_free(tmpBuff);
            return FAIL;
        }
    }

    BYTE* ptrBuff = tmpBuff;
    DWORD bufOffset = (DWORD)phyAddr & ~0xFFFFF000;
    ptrBuff += bufOffset;

    while(secCnt)
    {
        MP_DEBUG("secCnt=%d,lba=%d,AddrOf4KBRange=0x%x",secCnt,lba,AddrOf4KBRange);

        mmcp_memcpy(ptrBuff, (BYTE*)buf, SECTOR_SIZE);
        secCnt--;
        if(secCnt == 0)
        {
            MP_DEBUG("write1");
            spi_sector_erase(AddrOf4KBRange, 1);                                            // erase before writing
            if(stSpiInfo.dwChipSize > SPI_HW_LIMIT_SIZE)
            {
#if 0
                if(AddrOf4KBRange < SPI_HW_LIMIT_SIZE || AddrOf4KBRange >= 0x800000)
                {
                    if(spi_tx_write(tmpBuff, SPI_RW_4K, AddrOf4KBRange) != PASS)                     // write 4KB back
                    {
                        ret = FAIL;
                        break;
                    }
                }
                else
                {
                    if(spi_wcmd(tmpBuff, SPI_RW_4K, AddrOf4KBRange) != PASS)                  // write 4KB back
                    {
                        ret = FAIL;
                        break;
                    }
                }
#else
                if(spi_tx_write(tmpBuff, SPI_RW_4K, AddrOf4KBRange) != PASS)                     // write 4KB back
                {
                    ret = FAIL;
                    break;
                }
#endif
            }
            else
            {
                if(spi_wcmd(tmpBuff, SPI_RW_4K, AddrOf4KBRange) != PASS)                  // write 4KB back
                {
                    ret = FAIL;
                    break;
                }
            }
        }

        ptrBuff += SECTOR_SIZE;                                                             // point to next 512B
        if((secCnt > 0) && (ptrBuff == (tmpBuff + SPI_RW_4K)))                               // over 4KB range
        {
            MP_DEBUG("write2");

            if(spi_sector_erase(AddrOf4KBRange, 1) != PASS)                                         // erase before writing
            {
                ret = FAIL;
                break;
            }

            if(stSpiInfo.dwChipSize > SPI_HW_LIMIT_SIZE)
            {
#if 0
                if(AddrOf4KBRange < SPI_HW_LIMIT_SIZE || AddrOf4KBRange >= 0x800000)
                {
                    if(spi_tx_write(tmpBuff, SPI_RW_4K, AddrOf4KBRange) != PASS)                     // write 4KB back
                    {
                        ret = FAIL;
                        break;
                    }
                    AddrOf4KBRange += SPI_RW_4K;
                    if(AddrOf4KBRange == SPI_HW_LIMIT_SIZE)
                        AddrOf4KBRange |= stSpiInfo.dwCodeRsvdSize;

                    if(spi_rx_read(tmpBuff, SPI_RW_4K, AddrOf4KBRange) != PASS)                            // read next 4KB out
                    {
                        ret = FAIL;
                        break;
                    }
                }
                else
                {
                    if(spi_wcmd(tmpBuff, SPI_RW_4K, AddrOf4KBRange) != PASS)                  // write 4KB back
                    {
                        ret = FAIL;
                        break;
                    }
                    AddrOf4KBRange += SPI_RW_4K;
                    if(spi_rcmd(tmpBuff, SPI_RW_4K, AddrOf4KBRange) != PASS)                         // read next 4KB out
                    {
                        ret = FAIL;
                        break;
                    }
                }
 #else
                if(spi_tx_write(tmpBuff, SPI_RW_4K, AddrOf4KBRange) != PASS)                     // write 4KB back
                {
                    ret = FAIL;
                    break;
                }
                AddrOf4KBRange += SPI_RW_4K;
                if(AddrOf4KBRange == SPI_HW_LIMIT_SIZE)
                    AddrOf4KBRange |= stSpiInfo.dwCodeRsvdSize;

                if(spi_rx_read(tmpBuff, SPI_RW_4K, AddrOf4KBRange) != PASS)                            // read next 4KB out
                {
                    ret = FAIL;
                    break;
                }

 #endif
            }
            else
            {
                if(spi_wcmd(tmpBuff, SPI_RW_4K, AddrOf4KBRange) != PASS)                  // write 4KB back
                {
                    ret = FAIL;
                    break;
                }
                AddrOf4KBRange += SPI_RW_4K;
                if(spi_rcmd(tmpBuff, SPI_RW_4K, AddrOf4KBRange) != PASS)                         // read next 4KB out
                {
                    ret = FAIL;
                    break;
                }
            }
            ptrBuff = tmpBuff;                                                              // point to next 4KB data
        }
        buf += SECTOR_SIZE;                                                                 // buf offset 512B
    }

    ext_mem_free(tmpBuff);

    return ret;
}



static void spi_cmd_process(void* pMcardDev)
{
	ST_MCARD_MAIL *sMcardRMail;
	register ST_MCARD_DEV* pDev = (ST_MCARD_DEV*) pMcardDev;
    BYTE* buffer;


    TimerDelay(500);

	sMcardRMail = pDev->sMcardRMail;

	switch(sMcardRMail->wCmd)
	{

    	case SECTOR_READ_CMD:     // 512Bytes
    	    MP_DEBUG("SPI_READ_CMD");
            if(spi_logic_read(sMcardRMail->dwBuffer, sMcardRMail->dwBlockCount, sMcardRMail->dwBlockAddr))
            {
                MP_DEBUG("spi read cmd error");
            }
		break;

    	case SECTOR_WRITE_CMD:    // 512Bytes
            MP_DEBUG("SPI WRITE_PAGE_CMD");
#if SPI_WRITE_PROTECT_ENABLE
 		spi_wr_enable();
#endif
            if(spi_logic_write(sMcardRMail->dwBuffer, sMcardRMail->dwBlockCount, sMcardRMail->dwBlockAddr))
            {
                MP_DEBUG("spi write cmd error");
            }
#if SPI_WRITE_PROTECT_ENABLE
 		spi_wr_disable();
#endif
		break;

    	case RAW_FORMAT_CMD:
            MP_DEBUG("SPI FORMAT_CMD");
    		pDev->swStatus = spi_format();
		break;

    	default:
    		MP_DEBUG("-E- INVALID CMD");
		break;
	}
	spi_deselect();

}



static SDWORD erase_storage(void)
{
    DWORD erase64k;
    erase64k = (stSpiInfo.dwChipSize - stSpiInfo.dwCodeRsvdSize) >> 16;
    if((stSpiInfo.dwChipSize - stSpiInfo.dwCodeRsvdSize) % 0x10000)
        erase64k++;

    if((stSpiInfo.dwChipSize - stSpiInfo.dwCodeRsvdSize) > SPI_HW_LIMIT_SIZE)
    {
        if(spi_block_erase(0, 64))                                                         // 0 ~ 0x3FFFFF
        {
            MP_ALERT("storage erase fail 1.");
            return FAIL;
        }
        if(spi_block_erase((stSpiInfo.dwCodeRsvdSize + SPI_HW_LIMIT_SIZE), erase64k - 64))         //
        {
            MP_ALERT("storage erase fail 2.");
            return FAIL;
        }
    }
    else
    {
        if(spi_block_erase(stSpiInfo.dwCodeRsvdSize, erase64k))                           //
        {
            MP_ALERT("storage erase fail 3.");
            return FAIL;
        }
    }
}



// assume FAT16
static SWORD spi_format(void)
{
	BYTE* btSecBuf;
	BYTE* tmpBuf;
    DWORD tmpVal1, tmpVal2, FATSz;
    WORD tolSecs = (WORD)((stSpiInfo.dwChipSize - stSpiInfo.dwCodeRsvdSize) >> 9);

    erase_storage();

    btSecBuf = (BYTE *)ext_mem_malloc(512);
    if(btSecBuf == NULL)
    {
        MP_ALERT("No enoght memory");
        __asm("break 100");
    }
    btSecBuf = (BYTE*)((DWORD)btSecBuf | 0xA0000000);

    tmpBuf = (BYTE *)ext_mem_malloc(512);
    if(tmpBuf == NULL)
    {
        MP_ALERT("No enough memory");
        __asm("break 100");
    }
    tmpBuf = (BYTE*)((DWORD)tmpBuf | 0xA0000000);

    mmcp_memset(btSecBuf, 0, 512);
    mmcp_memset(tmpBuf, 0, 512);

    btSecBuf[0] = 0xEB;                                     // BS_jmpBoot[0] : 0xEB is the more frequently used format.
    btSecBuf[1] = 0x58;                                     // BS_jmpBoot[1] : Any 8-bit value is allowed in that byte.
    btSecBuf[2] = 0x90;                                     // BS_jmpBoot[2] : NOP.
    btSecBuf[3] = 'M';                                      // BS_OEMName : Formatted the volume system.
    btSecBuf[4] = 'P';
    btSecBuf[5] = 'X';
    btSecBuf[6] = 'S';
    btSecBuf[7] = 'P';
    btSecBuf[8] = 'I';
    btSecBuf[9] = '0';
    btSecBuf[10] = '0';
    btSecBuf[11] = 0x00;                                    // BPB_BytesPerSec : Assume sector = 512B
    btSecBuf[12] = 0x02;
    btSecBuf[13] = 0x08;                                    // BPB_SecPerClus : Assume cluster = 8 sectors. Can't be greater than 32KB.
    btSecBuf[14] = 0x01;                                    // BPB_RsvdSecCnt : For FAT12 and FAT16 volumns, this value should
    btSecBuf[15] = 0x00;                                    //                  never be anything other than 1.
    btSecBuf[16] = 0x02;                                    // BPB_NumFATs : This field should always contain the value 2 for
                                                            //               any FAT volume of any type.
    btSecBuf[17] = 0x00;                                    // BPB_RootEntCnt : For maximum compatibility, FAT16 volumes should
    btSecBuf[18] = 0x02;                                    //                  use the value 512.
    btSecBuf[19] = (BYTE)tolSecs;                           // BPB_TotSec16 : This count includes the count of all sectors in all four
    btSecBuf[20] = (BYTE)(tolSecs >> 8);                    //                regions of the volume.
    btSecBuf[21] = 0xF8;                                    // BPB_Media : 0xF8 is the standard value for "fixed"(non-removable) media.

//***Do not spend too much time trying to figure out whay this math works.*****//
    tmpVal1 = (stSpiInfo.dwChipSize - stSpiInfo.dwCodeRsvdSize)
              - (1 + 32);
    tmpVal2 = (256 * btSecBuf[13]) + btSecBuf[16];
    FATSz = (tmpVal1 + (tmpVal2 -1)) / tmpVal2;
//*****************************************************************************//
    btSecBuf[22] = (BYTE)FATSz;                             // BPB_FATSz16 : This field is the FAT16 16-bit count of sectors occupied by
    btSecBuf[23] = (BYTE)(FATSz >> 8);                      //               ONE FAT.
    btSecBuf[54] = 'F';                                     // BS_FilSysType : FAT16
    btSecBuf[55] = 'A';                                     //
    btSecBuf[56] = 'T';                                     //
    btSecBuf[57] = '1';                                     //
    btSecBuf[58] = '6';                                     //
    btSecBuf[59] = ' ';                                     //
    btSecBuf[60] = ' ';                                     //
    btSecBuf[61] = ' ';                                     //
    btSecBuf[510] = 0x55;                                   // Signature of boot sector end.
    btSecBuf[511] = 0xAA;

    BYTE  fatFst2Field[4] = {0xF8, 0xFF, 0xFF, 0xFF};       // For FAT first two fields initial value.

    if(stSpiInfo.dwChipSize > SPI_HW_LIMIT_SIZE)
    {
        spi_tx_write(btSecBuf, 512, 0);                     // Write boot sector at address 0.
        spi_tx_write(fatFst2Field, 4, 512);                 // FAT1 is at sector 1 => address 0x200.
        spi_tx_write(fatFst2Field, 4, (1 + FATSz) << 9);    // FAT2 is at sector (1 + FATSz) => address (1 + FATSz) * 512
        IODelay(100);
        spi_rx_read(tmpBuf, 512, 0);
    }
    else
    {
        spi_wcmd(btSecBuf, 512, stSpiInfo.dwCodeRsvdSize);
        spi_wcmd(fatFst2Field, 4, (stSpiInfo.dwCodeRsvdSize + 512));                        // FAT1 is at sector 1 => address (stSpiInfo.dwCodeRsrvdSize + 512).
        spi_wcmd(fatFst2Field, 4, (stSpiInfo.dwCodeRsvdSize + 512 + (FATSz << 9)));         // FAT2 is at sector (1 + FATSz) => address (stSpiInfo.dwCodeRsrvdSize + 512 + (FATSz << 9))
        IODelay(100);
        spi_rcmd(tmpBuf, 512, stSpiInfo.dwCodeRsvdSize);
    }

    for(tmpVal1 = 0; tmpVal1 < 512; tmpVal1++)              // Read back for checking.
    {
        if(btSecBuf[tmpVal1] != tmpBuf[tmpVal1])
        {
            MP_ALERT("Format error!!");
            __asm("break 100");
        }
    }

	return PASS;
}



void spi_dev_init(ST_MCARD_DEV* sDev)
{

    sDev->pbDescriptor = "SPI";
	sDev->wMcardType = DEV_SPI_FLASH;
	sDev->wSectorSize = 512;
	sDev->wSectorSizeExp = 9;
	sDev->Flag.Installed = 1;
    sDev->Flag.Present = 1;
    sDev->Flag.Detected = 1;
	sDev->CommandProcess = spi_cmd_process;

	spi_module_reset();
	if(spi_read_id())                                      // get spi chip size
	   RdSzFlag = 1;

//#if (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_SPI))
//#if SPI_STORAGE_ENABLE
    stSpiInfo.dwCodeRsvdSize = SPI_RESERVED_SIZE;
//#else
//    stSpiInfo.dwCodeRsvdSize = 0;
//#endif
    sDev->dwCapacity = (stSpiInfo.dwChipSize - stSpiInfo.dwCodeRsvdSize) >> 9;
    if(sDev->dwCapacity == 0)
    {
        MP_ALERT("-ERROR- SPI storage size is small then 512B.");
        __asm("break 100");
    }
    MP_ALERT("sDev->dwCapacity=0x%x", sDev->dwCapacity);

}



#if (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_SPI))

static void spi_isp_cmd_process(void* pMcardDev)
{
	ST_MCARD_MAIL *sMcardRMail;
	register ST_MCARD_DEV* pDev = (ST_MCARD_DEV*) pMcardDev;
    BYTE* buffer;

    //TimerDelay(500);

	sMcardRMail = pDev->sMcardRMail;

	switch(sMcardRMail->wCmd)
	{
    	case ISP_READ_CMD:
    	    MP_DEBUG("SPI_ISP_READ_CMD");
            if(stSpiInfo.dwChipSize > SPI_HW_LIMIT_SIZE)
            {
                //if(spi_rx_read((BYTE*)sMcardRMail->dwBuffer, sMcardRMail->dwBlockCount, sMcardRMail->dwBlockAddr))
                if(spi_rx_read((BYTE*)sMcardRMail->dwBuffer, sMcardRMail->dwBlockCount, (sMcardRMail->dwBlockAddr | SPI_HW_LIMIT_SIZE)))
                {
                    MP_DEBUG("spi isp read cmd error");
                }
            }
            else
            {
                if(spi_rcmd((BYTE*)sMcardRMail->dwBuffer, sMcardRMail->dwBlockCount, sMcardRMail->dwBlockAddr))
                {
                    MP_DEBUG("spi isp read cmd error");
                }
            }
		break;

    	case ISP_WRITE_CMD:
    	    MP_DEBUG("SPI_ISP_WRITE_CMD");
#if SPI_WRITE_PROTECT_ENABLE
 		spi_wr_enable();
#endif
            if(stSpiInfo.dwChipSize > SPI_HW_LIMIT_SIZE)
            {
                MP_DEBUG("spi_isp_tx_wt()");
                if(spi_isp_tx_wt((BYTE*)sMcardRMail->dwBuffer, sMcardRMail->dwBlockCount, (sMcardRMail->dwBlockAddr | SPI_HW_LIMIT_SIZE)))
                {
                    MP_DEBUG("spi isp write cmd error");
                }
            }
            else
            {
                MP_DEBUG("spi_isp_wcmd()");
                if(spi_isp_wcmd((BYTE*)sMcardRMail->dwBuffer, sMcardRMail->dwBlockCount, sMcardRMail->dwBlockAddr))
                {
                    MP_DEBUG("spi isp write cmd error");
                }
            }
#if SPI_WRITE_PROTECT_ENABLE
 		spi_wr_disable();
#endif

		break;
    	default:
    		MP_DEBUG("-E- INVALID CMD for SPI ISP");
		break;
	}
	spi_deselect();
}



void spi_isp_dev_init(ST_MCARD_DEV* sDev)
{
	sDev->pbDescriptor = "SPI ISP";
	sDev->wMcardType = DEV_SPI_FLASH_ISP;
	sDev->Flag.Installed = 1;
	sDev->CommandProcess = spi_isp_cmd_process;

    if(!RdSzFlag)
    {
    	spi_module_reset();
    	spi_read_id();                                                                         // get spi chip size
    }

	stSpiInfo.dwCodeRsvdSize = SPI_RESERVED_SIZE;
	if((stSpiInfo.dwChipSize - stSpiInfo.dwCodeRsvdSize) % 0x10000)
	{
	    MP_ALERT("ERROR : Storage size should be 65536 mutilpes");
	    __asm("break 100");
	}
    sDev->dwCapacity = stSpiInfo.dwChipSize - stSpiInfo.dwCodeRsvdSize;

}
#endif

void spi_wr_enable()
{
#if SPI_WRITE_PROTECT_ENABLE
	BYTE status;
	DWORD ticktime;
	spi_pin_config();

	spi_set_flash_wt_enable();
	spi_wr_flash_status(0x00);
	spi_set_flash_wt_disable();
	ticktime=GetSysTime();
	while(1)
	{
		spi_rd_flash_status(&status);
		IODelay(1);
		if (SystemGetElapsedTime(ticktime)>500)
			break;
		if(status & 0x03)
		{
			IODelay(1);
		}
		else
			break;
	}

#endif
}

void spi_wr_disable()
{
#if SPI_WRITE_PROTECT_ENABLE
	BYTE status;
	DWORD ticktime;
	spi_pin_config();

	spi_set_flash_wt_enable();
	spi_wr_flash_status(0x9c);
	spi_set_flash_wt_disable();
	ticktime=GetSysTime();
	while(1)
	{
		spi_rd_flash_status(&status);
		if (SystemGetElapsedTime(ticktime)>500)
			break;
		if(status & 0x03)
		{
			IODelay(10);
		}
		else
			break;
	}

#endif
}


#if ENCRYPT_BY_SPI_FLASH
#define SPI_READ_BYTES						8
#define SPI_ENCRYPT_ADDR        				(0x00100000-0x10)// 0x00290000
#if ENCRYPT_WRITE_FLASH
#define ENCRYPT_DEBUG						1
#else
#define ENCRYPT_DEBUG						0
#endif

static DWORD st_dwSpiEncryptBuffer[SPI_READ_BYTES/4];

void EncryptData(DWORD *dwInBuffer,DWORD *dwOutBuffer)
{
	DWORD i;
	BYTE *pbBufer;

#if ((UVC_PRODUCT_ID==1)||(UVC_PRODUCT_ID==2)||(UVC_PRODUCT_ID==3))
	dwOutBuffer[0]=dwInBuffer[0]^0x68393745;
#else
	dwOutBuffer[0]=dwInBuffer[0]^0x68293745;// For kang UVC
#endif
	if (dwOutBuffer[0]>0x7fffffff)
		dwOutBuffer[0]-=0x7fffffff;
	else
		dwOutBuffer[0]+=0x7fffffff;


#if ((UVC_PRODUCT_ID==1)||(UVC_PRODUCT_ID==2)||(UVC_PRODUCT_ID==3))
	dwOutBuffer[1]=dwInBuffer[1]^0x61393748;
#else
	dwOutBuffer[1]=dwInBuffer[1]^0x61293748;// For kang UVC
#endif
	if (dwOutBuffer[1]>0x6fffffff)
		dwOutBuffer[1]-=0x6fffffff;
	else
		dwOutBuffer[1]+=0x6fffffff;

}

void spi_unique_id_read()
{
	SPI *spi = (SPI *)SPI_BASE;
	DWORD i,dwSetupEncrypt[2],dwUnique[2];


	spi_pin_config();


	i = SPI_READ_BYTES;

	spi->SPI_TX0 = (0x4b << 24);
	spi->SPI_TX1 = 0;

	spi->SPI_CFG = 0x05000001 | (i << 16);

	if(spi_wait_module_ready(0x1000000) != PASS)
	{
		spi_deselect();
		mpDebugPrint("-----read-----Check FAIL!!!");
		while(1);
		return FAIL;
	}

	spi_copy_data((BYTE *)dwUnique, i);



	spi_deselect();

#if ENCRYPT_DEBUG
	//mpDebugPrint("----------spi_unique_id_read :");
	for (i=0;i<2;i++)
	mpDebugPrint("%08x",dwUnique[i]);
	mpDebugPrint("");
#endif

	EncryptData((DWORD *)&dwUnique[0],(DWORD *)&st_dwSpiEncryptBuffer[0]);
	//GetEncryptData((BYTE *)&dwSetupEncrypt[0],SPI_READ_BYTES);
	SPI_ReadEncryptData((BYTE *)&dwSetupEncrypt[0]);
	if ((dwSetupEncrypt[0]==st_dwSpiEncryptBuffer[0]) && (dwSetupEncrypt[1]==st_dwSpiEncryptBuffer[1]))
	{
		#if ENCRYPT_DEBUG
		mpDebugPrint("----------spi check ok!!!");
		#endif
	}
	else
	{
		
#if ENCRYPT_WRITE_FLASH
	mpDebugPrint("-Check FAIL-in write mode.");
	if (GetGPIOValue((VGPIO | GPIO_20)))
		while(1);
#else
		mpDebugPrint("----------Check FAIL!!!");
		while(1);
#endif
	}

}

#if 0
SWORD SetEncryptData(BYTE *bBuffer,DWORD dwSize)
{
	DWORD dwTmpData[2],i;
	BYTE *pbBufer;

	if (dwSize!=SPI_READ_BYTES)
	{
		mpDebugPrint("!!!SetEncryptData size error!!!");
		return FAIL;
	}

	EncryptData((DWORD *)&st_dwSpiEncryptBuffer[0],(DWORD *)&dwTmpData[0]);

	mpDebugPrint("----------spi_unique_id write :");
	for (i=0;i<2;i++)
		mpDebugPrint("%08x",dwTmpData[i]);
	mpDebugPrint("");

	pbBufer=(BYTE *)&dwTmpData[0];
	for (i=0;i<8;i++)
	{
		bBuffer[i]=pbBufer[i];
	}

	
	
	return PASS;
}
#endif

SDWORD SPI_WriteEncryptData()
{
	DWORD dwReadData[2],i;
	BYTE *pbBufer;
	SDWORD sdwRet;

	spi_wr_enable();
#if ENCRYPT_DEBUG
	mpDebugPrint("----------%s:",__FUNCTION__);
	for (i=0;i<2;i++)
		mpDebugPrint("%08x",st_dwSpiEncryptBuffer[i]);
	mpDebugPrint("");
#endif
	pbBufer=(BYTE *)&st_dwSpiEncryptBuffer[0];

	sdwRet=spi_isp_tx_wt(pbBufer,8,SPI_ENCRYPT_ADDR);
	//sdwRet=IspWrite(DEV_SPI_FLASH_ISP, SPI_ENCRYPT_ADDR, pbBufer, 0);
#if ENCRYPT_DEBUG
	if (sdwRet==PASS)
		mpDebugPrint("----------%s OK!",__FUNCTION__);
	else
		mpDebugPrint("----------%s FAIL!",__FUNCTION__);
#endif
	spi_wr_disable();
	SPI_ReadEncryptData(&dwReadData[0]);
	if ((dwReadData[0]!=st_dwSpiEncryptBuffer[0])||(dwReadData[1]!=st_dwSpiEncryptBuffer[1]))
	{
		mpDebugPrint("----WriteEncrypt verify FAIL!");
		sdwRet=FAIL;
	}
	return sdwRet;
}

SDWORD SPI_ReadEncryptData(DWORD *pdwbuffer)
{
	DWORD i;
	BYTE *pbBufer;
	SDWORD sdwRet;


	pdwbuffer[1]=0;
	pbBufer=(BYTE *)pdwbuffer;

	//sdwRet=IspRead(DEV_SPI_FLASH_ISP, SPI_ENCRYPT_ADDR, pbBufer, 8);
	sdwRet=spi_rx_read(pbBufer,8,SPI_ENCRYPT_ADDR);
#if ENCRYPT_DEBUG
	if (sdwRet==PASS)
		mpDebugPrint("----------%s OK!",__FUNCTION__);
	else
		mpDebugPrint("----------%s FAIL!",__FUNCTION__);

	//mpDebugPrint("----------%s :",__FUNCTION__);
	for (i=0;i<2;i++)
		mpDebugPrint("%08x",pdwbuffer[i]);
	mpDebugPrint("");
#endif	
	
	return sdwRet;
}


#endif



#endif


