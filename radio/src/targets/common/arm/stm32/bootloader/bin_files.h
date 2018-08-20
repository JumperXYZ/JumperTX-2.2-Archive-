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

#ifndef _bin_files_h_
#define _bin_files_h_

#include "opentx.h"

enum MemoryType {
  MEM_FLASH,
  MEM_EEPROM
};

#if defined(EEPROM)
#define getBinaryPath(mt)  ((mt == MEM_FLASH) ? FIRMWARES_PATH : EEPROMS_PATH)
#else
#define getBinaryPath(mt)  (FIRMWARES_PATH)
#endif

#define MAX_NAMES_ON_SCREEN   6
#define MAX_BIN_FILES         (MAX_NAMES_ON_SCREEN+1)

// Size of the block read when checking / writing BIN files
#define BLOCK_LEN 4096

// File info struct while browsing files on SD card
struct BinFileInfo {
    TCHAR        name[_MAX_LFN + 1];
    unsigned int size;
};

// File info storage while browsing files on SD card
extern BinFileInfo binFiles[MAX_BIN_FILES];

// Block buffer used when checking / writing BIN files
extern uint8_t Block_buffer[BLOCK_LEN];

// Bytes read into the Block_buffer
extern UINT    BlockCount;

// Open directory for EEPROM / firmware files
FRESULT openBinDir(MemoryType mt);

// Fetch file names and sizes into binFiles,
// starting at the provided index.
// Only files ending with ".bin" (case-insensitive)
// will be considered.
unsigned int fetchBinFiles(unsigned int index);

// Open file indexed in binFiles and read the first BLOCK_LEN bytes
// Bootloader is skipped in firmware files
FRESULT openBinFile(MemoryType mt, unsigned int index);

struct VersionTag
{
    char        flavour[8];
    const char* version;
};

// Can be called right after openBinFile() to extract the version information
// from a firmware file
void extractFirmwareVersion(VersionTag* tag);

// Read the next BLOCK_LEN bytes into 'Block_buffer'
// Check 'BlockCount' for # of bytes read
FRESULT readBinFile();

// Close the previously opened file
FRESULT closeBinFile();

#endif
