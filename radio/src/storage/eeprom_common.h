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

#define EEPROM_MIN_MODEL_SIZE          256

uint16_t eeLoadModelData(uint8_t id);
uint16_t eeLoadGeneralSettingsData();

bool eeModelExists(uint8_t id);
void eeLoadModel(uint8_t id);
bool eeConvert();
void ConvertModel(int id, int version);
uint8_t eeFindEmptyModel(uint8_t id, bool down);
void selectModel(uint8_t sub);

#if defined(CPUARM)
  extern ModelHeader modelHeaders[MAX_MODELS];
  void eeLoadModelHeader(uint8_t id, ModelHeader *header);
  void eeLoadModelHeaders();
#else
  #define eeLoadModelHeaders()
#endif

void storageReadRadioSettings();
void storageReadCurrentModel();

