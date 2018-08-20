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

#include "opentx.h"

void getCPUUniqueID(char * s)
{
#if defined(SIMU)
  uint32_t cpu_uid[3] = { 0x12345678, 0x55AA55AA, 0x87654321};
#else
  uint32_t * cpu_uid = (uint32_t *)0x1FFF7A10;
#endif
  char * tmp = strAppendUnsigned(s, cpu_uid[0], 8, 16);
  *tmp = ' ';
  tmp = strAppendUnsigned(tmp+1, cpu_uid[1], 8, 16);
  *tmp = ' ';
  strAppendUnsigned(tmp+1, cpu_uid[2], 8, 16);
}
