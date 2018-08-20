/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   cleanflight - https://github.com/cleanflight
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

#ifndef _GPS_H_
#define _GPS_H_

#include <inttypes.h>

struct gpsdata_t
{
  int32_t longitude;              // degrees * 1.000.000
  int32_t latitude;               // degrees * 1.000.000
  uint8_t fix;
  uint8_t numSat;
  uint32_t packetCount;
  uint32_t errorCount;
  uint16_t altitude;              // altitude in 0.1m
  uint16_t speed;                 // speed in 0.1m/s
  uint16_t groundCourse;          // degrees * 10
};

extern gpsdata_t gpsData;
void gpsWakeup();

void gpsSendFrame(const char * frame);

#endif // _GPS_H_
