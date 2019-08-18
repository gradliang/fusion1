#ifndef __MP650_SPI_H__
#define __MP650_SPI_H__

#define CODE_FLASH_BASE             0xBFC00000
//#define SPI_ACCESS_SIZE             0x10000             // 64KB


///
///@ingroup     SPI_MODULE
///
///@brief       Read spi flash identification number and get flash capacity.
///
///@retval      Spi flash identification number.
///
SDWORD spi_read_id(void);

///
///@ingroup     SPI_MODULE
///
///@brief       Get spi flash size.
///
///@retval      Spi flash size.
///
DWORD spi_get_size(void);

///
///@ingroup     SPI_MODULE
///
///@brief       Erase spi by block(64KB).
///
///@param       addr        block address.
///@param       cnt         block numbers want to erase.
///
///@retval      if succeed return PASS, otherwise return FAIL.
///
///@remark      The block size of this function is 64KB. Users should check whether the block of the flash is 64KB.
///
SDWORD spi_block_erase(DWORD addr, DWORD cnt);

///
///@ingroup     SPI_MODULE
///
///@brief       Erase spi by sector(4KB).
///
///@param       addr        sector address.
///@param       cnt         sector numbers want to erase.
///
///@retval      if succeed return PASS, otherwise return FAIL.
///
SDWORD spi_sector_erase(DWORD addr, DWORD cnt);

///
///@ingroup     SPI_MODULE
///
///@brief       Write data into spi flash.
///
///@param       *Buffer     data to be writen.
///@param       size        data size.
///@param       addr        address to be writen.
///
///@retval      PASS        write done.
///@retval      FAIL        write fail.
///
///@remark      This function issues spi page program command(0x02) and writes all data.
///             However, this function can't write more than 4MB spi flash due to ic designed
///             limitation(0xBFC0000 ~ 0xBFFFFFFF).
///
SDWORD spi_wcmd(BYTE *Buffer, DWORD size, DWORD addr);

///
///@ingroup     SPI_MODULE
///
///@brief       Read spi data.
///
///@param       *Buffer     container for data.
///@param       size        data size.
///@param       addr        data address.
///
///@retval      PASS        read done.
///@retval      FAIL        read fail.
///
///@remark      This function issues spi fast read command(0x0B) and reads back all data at once.
///             However, this function can't read more than 4MB spi flash due to ic designed
///             limitation(0xBFC0000 ~ 0xBFFFFFFF).
///
SDWORD spi_rcmd(BYTE *Buffer, DWORD size, DWORD addr);

///
///@ingroup     SPI_MODULE
///
///@brief       Write data into spi flash.
///
///@param       *Buffer     data to be writen.
///@param       size        data size.
///@param       addr        address to be writen.
///
///@retval      PASS        write done.
///@retval      FAIL        write fail.
///
///@remark      This function issues spi page program command(0x02) and writes 4 bytes and then
///             issues command and writes 4 bytes data till writing all data. Because
///             issuing page program command need follow 3 address bytes, programer can write
///             whole spi flash and ignore ic designed limitation(0xBFC0000 ~ 0xBFFFFFFF).
///
SDWORD spi_tx_write(BYTE *Buffer, DWORD size, DWORD addr);

///@defgroup    SPI_MODULE      SPI flash driver
///
///@ingroup     SPI_MODULE
///
///@brief       Read spi flash data.
///
///@param       *Buffer     container for data.
///@param       size        data size.
///@param       addr        data address.
///
///@retval      PASS        read done.
///@retval      FAIL        read fail.
///
///@remark      This function issues spi read command(0x03) and reads back 8 bytes and then
///             issues command and reads back 8 bytes data till reading back all data. Because
///             issuing read command need follow 3 address bytes, programer can read whole spi
///             flash and ignore ic designed limitation(0xBFC0000 ~ 0xBFFFFFFF).
///
SDWORD spi_rx_read(BYTE *Buffer, DWORD size, DWORD addr);

///
///@ingroup     SPI_MODULE
///
///@brief       Set gpio pins to SPI module pins.
///
void spi_pin_config(void);

///
///@ingroup     SPI_MODULE
///
///@brief       Set SPI module pins to gpio pins.
///
void spi_deselect(void);

///
///@ingroup     SPI_MODULE
///
///@brief       Reset SPI module.
///
void spi_module_reset(void);

//void spi_dev_init(ST_MCARD_DEV* sDev);


#endif


