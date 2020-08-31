/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include "diskio.h" /* FatFs lower layer API */
#include "ff.h"
#include "sdcard.h"
#include "w25qxx.h"

/* Definitions of physical drive number for each drive */
#define SD_CARD 0
#define SPI_FLASH 1

#define FLASH_SECTOR_SIZE 512
#define FLASH_BLOCK_SIZE 8

uint32_t FLASH_SECTOR_COUNT = 2048 * 6;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(BYTE pdrv) { return 0; }

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(BYTE pdrv) {
    uint8_t res = 0;
    uint32_t spi_index = 3;
    switch (pdrv) {
    case SD_CARD:
        if (sd_init() == 0)
            res = 0;
        break;
    case SPI_FLASH:
        if (w25qxx_init(spi_index, 0) == 0)
            res = 0;
        break;
    default:
        res = 1;
    }
    if (res)
        return STA_NOINIT;
    else
        return 0;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count) {
    uint8_t res = 0;
    if (!count)
        return RES_PARERR;
    switch (pdrv) {
    case SD_CARD:
        res = sd_read_sector_dma((BYTE *)buff, sector, count);
        while (res) {
            sd_init();
            res = sd_read_sector_dma((BYTE *)buff, sector, count);
        }
        break;
    case SPI_FLASH:
        for (; count > 0; count--) {
            w25qxx_read_data(sector * FLASH_SECTOR_SIZE, (BYTE *)buff,
                             FLASH_SECTOR_SIZE, W25QXX_STANDARD);
            sector++;
            buff += FLASH_SECTOR_SIZE;
        }
        res = 0;
        break;
    default:
        res = 1;
        break;
    }
    if (res == 0x00)
        return RES_OK;
    else
        return RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count) {
    uint8_t res = 0;
    if (!count)
        return RES_PARERR;
    switch (pdrv) {
    case SD_CARD:
        res = sd_write_sector_dma((BYTE *)buff, sector, count);
        while (res) {
            sd_init();
            res = sd_write_sector_dma((BYTE *)buff, sector, count);
        }
        break;
    case SPI_FLASH:
        for (; count > 0; count--) {
            w25qxx_write_data(sector * FLASH_SECTOR_SIZE, (BYTE *)buff,
                              FLASH_SECTOR_SIZE);
            sector++;
            buff += FLASH_SECTOR_SIZE;
        }
        res = 0;
        break;
    default:
        res = 1;
        break;
    }
    if (res == 0x00)
        return RES_OK;
    else
        return RES_ERROR;
    return RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
    DRESULT res;
    if (pdrv == SD_CARD) // SD卡
    {
        switch (cmd) {
        case CTRL_SYNC:
            res = RES_OK;
            break;
        case GET_SECTOR_SIZE:
            *(DWORD *)buff = (cardinfo.SD_csd.DeviceSize + 1) << 10;
            res = RES_OK;
            break;
        case GET_BLOCK_SIZE:
            *(WORD *)buff = cardinfo.CardBlockSize;
            res = RES_OK;
            break;
        case GET_SECTOR_COUNT:
            *(DWORD *)buff = cardinfo.CardBlockSize;
            res = RES_OK;
            break;
        default:
            res = RES_PARERR;
            break;
        }
    } else if (pdrv == SPI_FLASH) //外部FLASH
    {
        switch (cmd) {
        case CTRL_SYNC:
            res = RES_OK;
            break;
        case GET_SECTOR_SIZE:
            *(WORD *)buff = FLASH_SECTOR_SIZE;
            res = RES_OK;
            break;
        case GET_BLOCK_SIZE:
            *(WORD *)buff = FLASH_BLOCK_SIZE;
            res = RES_OK;
            break;
        case GET_SECTOR_COUNT:
            *(DWORD *)buff = FLASH_SECTOR_COUNT;
            res = RES_OK;
            break;
        default:
            res = RES_PARERR;
            break;
        }
    } else
        res = RES_ERROR; //其他的不支持
    return res;
}
