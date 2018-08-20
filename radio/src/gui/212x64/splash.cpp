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

#if defined(SPLASH)
const pm_uchar splashdata[] PROGMEM = {
  'S','P','S',0,
  #include "bitmaps/212x64/splash.lbm"
  'S','P','E',0 };
const pm_uchar * const splash_lbm = splashdata+4;

void drawSplash()
{
  lcdClear();
  lcdDrawBitmap(0, 0, splash_lbm);

#if MENUS_LOCK == 1
  if (readonly == false) {
    lcdDrawFilledRect((LCD_W-(sizeof(TR_UNLOCKED)-1)*FW)/2 - 9, 50, (sizeof(TR_UNLOCKED)-1)*FW+16, 11, SOLID, ERASE|ROUND);
    lcdDrawText((LCD_W-(sizeof(TR_UNLOCKED)-1)*FW)/2 , 53, STR_UNLOCKED);
  }
#endif

  lcdRefresh();
}
#endif

#if defined(SPLASH_FRSKY)
const pm_uchar splashdata2[] PROGMEM = {
  'S','F','S',0,
  #include "bitmaps/212x64/splash_frsky.lbm"
  'S','F','E',0 };
const pm_uchar * const splash2_lbm = splashdata2+4;

void drawSecondSplash()
{
  lcdClear();
  lcdDrawBitmap(0, 0, splash2_lbm);
  lcdRefresh();
}
#endif