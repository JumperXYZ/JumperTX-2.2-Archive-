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

void menuChannelsView(event_t event)
{
  static bool longNames = false;
  static bool secondPage = false;
  static bool mixersView = false;
  uint8_t ch = 0;
  uint8_t wbar = (longNames ? 54 : 64);
  int16_t limits = 512 * 2;

#if defined(PPM_UNIT_PERCENT_PREC1)
  wbar -= 6;
#endif

  switch(event)
  {
    case EVT_KEY_BREAK(KEY_EXIT):
      popMenu();
      break;
    case EVT_KEY_FIRST(KEY_RIGHT):
    case EVT_KEY_FIRST(KEY_LEFT):
#if defined(ROTARY_ENCODER_NAVIGATION)
    case EVT_ROTARY_LEFT:
    case EVT_ROTARY_RIGHT:
#endif
      secondPage = !secondPage;
      break;
    case EVT_KEY_FIRST(KEY_ENTER):
      mixersView = !mixersView;
      break;
  }

  if (secondPage)
    ch = 16;

  if (mixersView)
    limits *= 2;  // this could be handled nicer, but slower, by checking actual range for this mixer
  else if (g_model.extendedLimits)
    limits *= LIMIT_EXT_PERCENT / 100;

  if (mixersView)
    lcdDrawTextAlignedCenter(0, MIXERS_MONITOR);
  else
    lcdDrawTextAlignedCenter(0, CHANNELS_MONITOR);

  lcdInvertLine(0);

  // Column separator
  lcdDrawSolidVerticalLine(LCD_W/2, FH, LCD_H-FH);

  for (uint8_t col=0; col < 2; col++) {
    const uint8_t x = col * LCD_W / 2 + 1;
    const uint8_t ofs = (col ? 0 : 1);

    // Channels
    for (uint8_t line=0; line < 8; line++) {
      const uint8_t y = 9 + line * 7;
      const int32_t val = mixersView ? ex_chans[ch] : channelOutputs[ch];
      const uint8_t lenLabel = ZLEN(g_model.limitData[ch].name);

      // Channel name if present, number if not
      if (lenLabel > 0) {
        if (lenLabel > 4)
          longNames = true;
        lcdDrawSizedText(x+1-ofs, y, g_model.limitData[ch].name, sizeof(g_model.limitData[ch].name), ZCHAR | SMLSIZE);
      }
      else {
        putsChn(x+1-ofs, y, ch+1, SMLSIZE);
      }

      // Value
#if defined(PPM_UNIT_US)
      lcdDrawNumber(x+LCD_W/2-3-wbar-ofs, y+1, PPM_CH_CENTER(ch)+val/2, TINSIZE|RIGHT);
#elif defined(PPM_UNIT_PERCENT_PREC1)
      lcdDrawNumber(x+LCD_W/2-3-wbar-ofs, y+1, calcRESXto1000(val), PREC1|TINSIZE|RIGHT);
#else
      lcdDrawNumber(x+LCD_W/2-3-wbar-ofs, y+1, calcRESXto1000(val)/10, TINSIZE|RIGHT);
#endif

      // Gauge
      drawGauge(x+LCD_W/2-3-wbar-ofs, y, wbar, 6, val, limits);

      ++ch;
    }
  }
}
