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

#ifndef _DUMP_H_
#define _DUMP_H_

#if defined(DEBUG) || defined(CLI)
void dumpStart(unsigned int size);
void dumpBody(const uint8_t * data, unsigned int size);
void dumpEnd();
void dump(const uint8_t * data, unsigned int size);
#else
#define dump(data, size)
#endif

#endif // _DUMP_H_
