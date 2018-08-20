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

enum MixFields {
  MIX_FIELD_NAME,
  MIX_FIELD_SOURCE,
  MIX_FIELD_WEIGHT,
  MIX_FIELD_OFFSET,
  MIX_FIELD_TRIM,
  CASE_CURVES(MIX_FIELD_CURVE)
  CASE_FLIGHT_MODES(MIX_FIELD_FLIGHT_MODE)
  MIX_FIELD_SWITCH,
  MIX_FIELD_WARNING,
  MIX_FIELD_MLTPX,
  MIX_FIELD_DELAY_UP,
  MIX_FIELD_DELAY_DOWN,
  MIX_FIELD_SLOW_UP,
  MIX_FIELD_SLOW_DOWN,
  MIX_FIELD_COUNT
};

void drawOffsetBar(uint8_t x, uint8_t y, MixData * md)
{
  const int gaugeWidth = 33;
  const int gaugeHeight = 6;

  int offset = GET_GVAR(MD_OFFSET(md), GV_RANGELARGE_NEG, GV_RANGELARGE, mixerCurrentFlightMode);
  int weight = GET_GVAR(MD_WEIGHT(md), GV_RANGELARGE_NEG, GV_RANGELARGE, mixerCurrentFlightMode);
  int barMin = offset - weight;
  int barMax = offset + weight;
  if (y > 15) {
    lcdDrawNumber(x-((barMin >= 0) ? 2 : 3), y-6, barMin, TINSIZE|LEFT);
    lcdDrawNumber(x+gaugeWidth+1, y-6, barMax, TINSIZE|RIGHT);
  }
  if (weight < 0) {
    barMin = -barMin;
    barMax = -barMax;
  }
  if (barMin < -101)
    barMin = -101;
  if (barMax > 101)
    barMax = 101;
  lcdDrawHorizontalLine(x-2, y, gaugeWidth+2, DOTTED);
  lcdDrawHorizontalLine(x-2, y+gaugeHeight, gaugeWidth+2, DOTTED);
  lcdDrawSolidVerticalLine(x-2, y+1, gaugeHeight-1);
  lcdDrawSolidVerticalLine(x+gaugeWidth-1, y+1, gaugeHeight-1);
  if (barMin <= barMax) {
    int8_t right = (barMax * gaugeWidth) / 200;
    int8_t left = ((barMin * gaugeWidth) / 200)-1;
    lcdDrawSolidFilledRect(x+gaugeWidth/2+left, y+2, right-left, gaugeHeight-3);
  }
  lcdDrawSolidVerticalLine(x+gaugeWidth/2-1, y, gaugeHeight+1);
  if (barMin == -101) {
    for (uint8_t i=0; i<3; ++i) {
      lcdDrawPoint(x+i, y+4-i);
      lcdDrawPoint(x+3+i, y+4-i);
    }
  }
  if (barMax == 101) {
    for (uint8_t i=0; i<3; ++i) {
      lcdDrawPoint(x+gaugeWidth-8+i, y+4-i);
      lcdDrawPoint(x+gaugeWidth-5+i, y+4-i);
    }
  }
}

void menuModelMixOne(event_t event)
{
  MixData * md2 = mixAddress(s_currIdx) ;
  putsChn(PSIZE(TR_MIXER)*FW+FW, 0, md2->destCh+1,0);
  
  SUBMENU(STR_MIXER, MIX_FIELD_COUNT, {0, 0, 0, 0, 0, CASE_CURVES(1) CASE_FLIGHT_MODES((MAX_FLIGHT_MODES-1) | NAVIGATION_LINE_BY_LINE) 0, 0 /*, ...*/});
  
  int8_t sub = menuVerticalPosition;
  int8_t editMode = s_editMode;

  for (int k=0; k<LCD_LINES-1; k++) {
    coord_t y;
    if (k >= LCD_LINES-1)
      y = 1 + (k-LCD_LINES+2)*FH;
    else
      y = 1 + (k+1)*FH;
    int8_t i = k;

    i += menuVerticalOffset;

    LcdFlags attr = (sub==i ? (editMode>0 ? BLINK|INVERS : INVERS) : 0);
    switch(i) {
      case MIX_FIELD_NAME:
        editSingleName(MIXES_2ND_COLUMN, y, STR_MIXNAME, md2->name, sizeof(md2->name), event, attr);
        break;

      case MIX_FIELD_SOURCE:
        lcdDrawTextAlignedLeft(y, NO_INDENT(STR_SOURCE));
        drawSource(MIXES_2ND_COLUMN, y, md2->srcRaw, STREXPANDED|attr);
        if (attr) CHECK_INCDEC_MODELSOURCE(event, md2->srcRaw, 1, MIXSRC_LAST);
        break;

      case MIX_FIELD_WEIGHT:
        lcdDrawTextAlignedLeft(y, STR_WEIGHT);
        gvarWeightItem(MIXES_2ND_COLUMN, y, md2, attr|LEFT, event);
        break;

      case MIX_FIELD_OFFSET:
      {
        lcdDrawTextAlignedLeft(y, NO_INDENT(STR_OFFSET));
        u_int8int16_t offset;
        MD_OFFSET_TO_UNION(md2, offset);
        offset.word = GVAR_MENU_ITEM(MIXES_2ND_COLUMN, y, offset.word, GV_RANGELARGE_OFFSET_NEG, GV_RANGELARGE_OFFSET, attr|LEFT, 0, event);
        MD_UNION_TO_OFFSET(offset, md2);
        drawOffsetBar(MIXES_2ND_COLUMN+22, y, md2);
        break;
      }

      case MIX_FIELD_TRIM:
        lcdDrawTextAlignedLeft(y, STR_TRIM);
        drawCheckBox(MIXES_2ND_COLUMN, y, !md2->carryTrim, attr);
        if (attr) md2->carryTrim = !checkIncDecModel(event, !md2->carryTrim, 0, 1);
        break;

#if defined(CURVES)
      case MIX_FIELD_CURVE:
        lcdDrawTextAlignedLeft(y, STR_CURVE);
        editCurveRef(MIXES_2ND_COLUMN, y, md2->curve, event, attr);
        break;
#endif

#if defined(FLIGHT_MODES)
      case MIX_FIELD_FLIGHT_MODE:
        lcdDrawTextAlignedLeft(y, STR_FLMODE);
        md2->flightModes = editFlightModes(MIXES_2ND_COLUMN, y, event, md2->flightModes, attr);
        break;
#endif

      case MIX_FIELD_SWITCH:
        md2->swtch = editSwitch(MIXES_2ND_COLUMN, y, md2->swtch, attr, event);
        break;

      case MIX_FIELD_WARNING:
        drawFieldLabel(MIXES_2ND_COLUMN, y, STR_MIXWARNING);
        if (md2->mixWarn)
          lcdDrawNumber(MIXES_2ND_COLUMN, y, md2->mixWarn, attr|LEFT);
        else
          lcdDrawText(MIXES_2ND_COLUMN, y, STR_OFF, attr);
        if (attr) CHECK_INCDEC_MODELVAR_ZERO(event, md2->mixWarn, 3);
        break;

      case MIX_FIELD_MLTPX:
        md2->mltpx = editChoice(MIXES_2ND_COLUMN, y, STR_MULTPX, STR_VMLTPX, md2->mltpx, 0, 2, attr, event);
        break;

      case MIX_FIELD_DELAY_UP:
        md2->delayUp = EDIT_DELAY(0, y, event, attr, STR_DELAYUP, md2->delayUp);
        break;

      case MIX_FIELD_DELAY_DOWN:
        md2->delayDown = EDIT_DELAY(0, y, event, attr, STR_DELAYDOWN, md2->delayDown);
        break;

      case MIX_FIELD_SLOW_UP:
        md2->speedUp = EDIT_DELAY(0, y, event, attr, STR_SLOWUP, md2->speedUp);
        break;

      case MIX_FIELD_SLOW_DOWN:
        md2->speedDown = EDIT_DELAY(0, y, event, attr, STR_SLOWDOWN, md2->speedDown);
        break;
    }
  }
}
