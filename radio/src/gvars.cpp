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

#if defined(PCBSTD)
int16_t getGVarFieldValue(int16_t x, int16_t min, int16_t max)
{
  if (GV_IS_GV_VALUE(x, min, max)) {
    int8_t idx = GV_INDEX_CALCULATION(x, max);
    int8_t mul = 1;

    if (idx < 0) {
      idx = -1-idx;
      mul = -1;
    }

    x = GVAR_VALUE(idx, -1) * mul;
  }

  return limit(min, x, max);
}

void setGVarValue(uint8_t idx, int8_t value)
{
  if (GVAR_VALUE(idx, -1) != value) {
    SET_GVAR_VALUE(idx, -1, value);
  }
}
#else
uint8_t gvarDisplayTimer = 0;
uint8_t gvarLastChanged = 0;

uint8_t getGVarFlightMode(uint8_t fm, uint8_t gv) // TODO change params order to be consistent!
{
  for (uint8_t i=0; i<MAX_FLIGHT_MODES; i++) {
    if (fm == 0) return 0;
    int16_t val = GVAR_VALUE(gv, fm);
    if (val <= GVAR_MAX) return fm;
    uint8_t result = val-GVAR_MAX-1;
    if (result >= fm) result++;
    fm = result;
  }
  return 0;
}

int16_t getGVarValue(int8_t gv, int8_t fm)
{
  int8_t mul = 1;
  if (gv < 0) {
    gv = -1-gv;
    mul = -1;
  }
  return GVAR_VALUE(gv, getGVarFlightMode(fm, gv)) * mul;
}

int32_t getGVarValuePrec1(int8_t gv, int8_t fm)
{
  int8_t mul;
  uint8_t prec = g_model.gvars[abs((int)gv)].prec;    // explicit cast to `int` needed, othervise gv is promoted to double!
  if (prec == 0)
    mul = 10;
  else
    mul = 1;

  if (gv < 0) {
    gv = -1-gv;
    mul = -mul;
  }
  return GVAR_VALUE(gv, getGVarFlightMode(fm, gv)) * mul;
}

void setGVarValue(uint8_t gv, int16_t value, int8_t fm)
{
  fm = getGVarFlightMode(fm, gv);
  if (GVAR_VALUE(gv, fm) != value) {
    SET_GVAR_VALUE(gv, fm, value);
  }
}

int16_t getGVarFieldValue(int16_t val, int16_t min, int16_t max, int8_t fm)
{
  if (GV_IS_GV_VALUE(val, min, max)) {
    int8_t gv = GV_INDEX_CALCULATION(val, max);
    val = getGVarValue(gv, fm);
  }
  return limit(min, val, max);
}

int32_t getGVarFieldValuePrec1(int16_t val, int16_t min, int16_t max, int8_t fm)
{
  if (GV_IS_GV_VALUE(val, min, max)) {
    int8_t gv = GV_INDEX_CALCULATION(val, max);
    val = getGVarValuePrec1(gv, fm);
  }
  else {
    val *= 10;
  }
  return limit<int>(min*10, val, max*10);
}
#endif
