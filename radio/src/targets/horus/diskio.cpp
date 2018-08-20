/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x 
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include <string.h>
#include "opentx.h"
#include "targets/common/arm/stm32/sdio_sd.h"

/*-----------------------------------------------------------------------*/
/* Lock / unlock functions                                               */
/*-----------------------------------------------------------------------*/
#if !defined(BOOT)
static OS_MutexID ioMutex;
uint32_t ioMutexReq = 0, ioMutexRel = 0;
int ff_cre_syncobj (BYTE vol, _SYNC_t *mutex)
{
  *mutex = ioMutex;
  return 1;
}

int ff_req_grant (_SYNC_t mutex)
{
  ioMutexReq += 1;
  return CoEnterMutexSection(mutex) == E_OK;
}

void ff_rel_grant (_SYNC_t mutex)
{
  ioMutexRel += 1;
  CoLeaveMutexSection(mutex);
}

int ff_del_syncobj (_SYNC_t mutex)
{
  return 1;
}
#endif


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
  BYTE drv                                /* Physical drive nmuber (0..) */
)
{
  DSTATUS stat = 0;

  /* Supports only single drive */
  if (drv)
  {
    stat |= STA_NOINIT;
  }

  /*-------------------------- SD Init ----------------------------- */
  SD_Error res = SD_Init();
  if (res != SD_OK)
  {
    TRACE("SD_Init() failed: %d", res);
    stat |= STA_NOINIT;
  }

  TRACE("SD card info:");
  TRACE("sectors: %u", (uint32_t)(SDCardInfo.CardCapacity / 512));
  TRACE("type: %u", (uint32_t)(SDCardInfo.CardType));
  TRACE("EraseGrSize: %u", (uint32_t)(SDCardInfo.SD_csd.EraseGrSize));
  TRACE("EraseGrMul: %u", (uint32_t)(SDCardInfo.SD_csd.EraseGrMul));
  TRACE("ManufacturerID: %u", (uint32_t)(SDCardInfo.SD_cid.ManufacturerID));

  return(stat);
}

DWORD scratch[BLOCK_SIZE / 4] __DMA;

/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
  BYTE drv                /* Physical drive nmuber (0..) */
)
{
  DSTATUS stat = 0;

  if (SD_Detect() != SD_PRESENT)
    stat |= STA_NODISK;

  // STA_NOTINIT - Subsystem not initailized
  // STA_PROTECTED - Write protected, MMC/SD switch if available

  return(stat);
}

uint32_t sdReadRetries = 0;

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */


DRESULT disk_read_dma(BYTE drv, BYTE * buff, DWORD sector, UINT count)
{
  // this functions assumes that buff is properly aligned and in the right RAM segment for DMA
  DRESULT res;
  SD_Error Status;
  SDTransferState State;
  for (int retry=0; retry<3; retry++) {
    res = RES_OK;
    if (count == 1) {
      Status = SD_ReadBlock(buff, sector, BLOCK_SIZE); // 4GB Compliant
    }
    else {
      Status = SD_ReadMultiBlocks(buff, sector, BLOCK_SIZE, count); // 4GB Compliant
    }
    if (Status == SD_OK) {
      Status = SD_WaitReadOperation(200*count); // Check if the Transfer is finished
      while ((State = SD_GetStatus()) == SD_TRANSFER_BUSY); // BUSY, OK (DONE), ERROR (FAIL)
      if (State == SD_TRANSFER_ERROR)  {
        TRACE("State=SD_TRANSFER_ERROR, c: %u", sector, (uint32_t)count);
        res = RES_ERROR;
      }
      else if (Status != SD_OK) {
        TRACE("Status(WaitRead)=%d, s:%u c: %u", Status, sector, (uint32_t)count);
        res = RES_ERROR;
      }
    }
    else {
      TRACE("Status(ReadBlock)=%d, s:%u c: %u", Status, sector, (uint32_t)count);
      res = RES_ERROR;
    }
    if (res == RES_OK) break;
    sdReadRetries += 1;
  }
  return res;
}

DRESULT __disk_read(BYTE drv, BYTE * buff, DWORD sector, UINT count)
{
  // If unaligned, do the single block reads with a scratch buffer.
  // If aligned and single sector, do a single block read.
  // If aligned and multiple sectors, try multi block read.
  //    If multi block read fails, try single block reads without
  //    an intermediate buffer (move trough the provided buffer)

  // TRACE("disk_read %d %p %10d %d", drv, buff, sector, count);
  if (SD_Detect() != SD_PRESENT) {
    TRACE("SD_Detect() != SD_PRESENT");
    return RES_NOTRDY;
  }

  DRESULT res = RES_OK;
  if (count == 0) return res;

  if ((DWORD)buff < 0x20000000 || ((DWORD)buff & 3)) {
    // buffer is not aligned, use scratch buffer that is aligned
    TRACE("disk_read bad alignment (%p)", buff);
    while (count--) {
      res = disk_read_dma(drv, (BYTE *)scratch, sector++, 1);
      if (res != RES_OK) break;
      memcpy(buff, scratch, BLOCK_SIZE);
      buff += BLOCK_SIZE;
    }
    return res;
  }

  res = disk_read_dma(drv, buff, sector, count);
  if (res != RES_OK && count > 1) {
    // multi-read failed, try reading same sectors, one by one
    TRACE("disk_read() multi-block failed, trying single block reads...");
    while (count--) {
      res = disk_read_dma(drv, buff, sector++, 1);
      if (res != RES_OK) break;
      buff += BLOCK_SIZE;
    }
  }
  return res;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT __disk_write(
  BYTE drv,                       /* Physical drive nmuber (0..) */
  const BYTE *buff,               /* Data to be written */
  DWORD sector,                   /* Sector address (LBA) */
  UINT count                      /* Number of sectors to write (1..255) */
)
{
  SD_Error Status;
  DRESULT res = RES_OK;

  // TRACE("disk_write %d %p %10d %d", drv, buff, sector, count);

  if (SD_Detect() != SD_PRESENT)
    return(RES_NOTRDY);

  if ((DWORD)buff < 0x20000000 || ((DWORD)buff & 3)) {
    TRACE("disk_write bad alignment (%p)", buff);
    while(count--) {
      memcpy(scratch, buff, BLOCK_SIZE);

      res = __disk_write(drv, (BYTE *)scratch, sector++, 1);

      if (res != RES_OK)
        break;

      buff += BLOCK_SIZE;
    }
    return(res);
  }

  if (count == 1) {
    Status = SD_WriteBlock((uint8_t *)buff, sector, BLOCK_SIZE); // 4GB Compliant
  }
  else {
    Status = SD_WriteMultiBlocks((uint8_t *)buff, sector, BLOCK_SIZE, count); // 4GB Compliant
  }

  if (Status == SD_OK) {
    SDTransferState State;

    Status = SD_WaitWriteOperation(500*count); // Check if the Transfer is finished

    while((State = SD_GetStatus()) == SD_TRANSFER_BUSY); // BUSY, OK (DONE), ERROR (FAIL)

    if ((State == SD_TRANSFER_ERROR) || (Status != SD_OK)) {
      TRACE("__disk_write() err, st:%d,%d, s:%u c: %u", Status, State, sector, (uint32_t)count);
      res = RES_ERROR;
    }
  }
  else {
    res = RES_ERROR;
  }

  // TRACE("result=%d", res);
  return res;
}
#endif /* _READONLY */

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
  BYTE drv,               /* Physical drive nmuber (0..) */
  BYTE ctrl,              /* Control code */
  void *buff              /* Buffer to send/receive control data */
)
{
  DRESULT res;

  if (drv) return RES_PARERR;

  res = RES_ERROR;

  switch (ctrl) {
    case GET_SECTOR_COUNT : /* Get number of sectors on the disk (DWORD) */
      // use 512 for sector size, SDCardInfo.CardBlockSize is not sector size and can be 1024 for 2G SD cards!!!!
      *(DWORD*)buff = SDCardInfo.CardCapacity / BLOCK_SIZE;
      res = RES_OK;
      break;

    case GET_SECTOR_SIZE :  /* Get R/W sector size (WORD) */
      *(WORD*)buff = BLOCK_SIZE;   // force sector size. SDCardInfo.CardBlockSize is not sector size and can be 1024 for 2G SD cards!!!!
      res = RES_OK;
      break;

    case GET_BLOCK_SIZE :   /* Get erase block size in unit of sector (DWORD) */
      // TODO verify that this is the correct value
      *(DWORD*)buff = (uint32_t)SDCardInfo.SD_csd.EraseGrSize * (uint32_t)SDCardInfo.SD_csd.EraseGrMul;
      res = RES_OK;
      break;

    case CTRL_SYNC:
      while (SD_GetStatus() == SD_TRANSFER_BUSY); /* Complete pending write process (needed at _FS_READONLY == 0) */
      res = RES_OK;
      break;

    default:
      res = RES_OK;
      break;

  }

  return res;
}

// TODO everything here should not be in the driver layer ...

FATFS g_FATFS_Obj __DMA;    // initialized in boardInit()
#if defined(LOG_TELEMETRY)
FIL g_telemetryFile = {};
#endif

#if defined(BOOT)
void sdInit(void)
{
  if (f_mount(&g_FATFS_Obj, "", 1) == FR_OK) {
    f_chdir("/");
  }
}
#else
void sdInit()
{
  TRACE("sdInit");
  
  ioMutex = CoCreateMutex();
  if (ioMutex >= CFG_MAX_MUTEX) {
    // sd error
    return;
  }
  sdMount();
}

void sdMount()
{
  TRACE("sdMount");
  
  diskCache.clear();
  
  if (f_mount(&g_FATFS_Obj, "", 1) == FR_OK) {
    // call sdGetFreeSectors() now because f_getfree() takes a long time first time it's called
    sdGetFreeSectors();

#if defined(LOG_TELEMETRY)
    f_open(&g_telemetryFile, LOGS_PATH "/telemetry.log", FA_OPEN_ALWAYS | FA_WRITE);
    if (f_size(&g_telemetryFile) > 0) {
      f_lseek(&g_telemetryFile, f_size(&g_telemetryFile)); // append
    }
#endif
  }
  else {
    TRACE("f_mount() failed");
  }
}

void sdDone()
{
  TRACE("sdDone");
  
  if (sdMounted()) {
    audioQueue.stopSD();
#if defined(LOG_TELEMETRY)
    f_close(&g_telemetryFile);
#endif
    f_mount(NULL, "", 0); // unmount SD
  }
}
#endif

uint32_t sdMounted()
{
  return g_FATFS_Obj.fs_type != 0;
}

uint32_t sdIsHC()
{
  return true; // TODO (CardType & CT_BLOCK);
}

uint32_t sdGetSpeed()
{
  return 330000;
}
