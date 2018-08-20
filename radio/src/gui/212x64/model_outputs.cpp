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

enum LimitsItems {
  ITEM_LIMITS_CH_NAME,
  ITEM_LIMITS_OFFSET,
  ITEM_LIMITS_MIN,
  ITEM_LIMITS_MAX,
  ITEM_LIMITS_DIRECTION,
#if defined(CURVES)
  ITEM_LIMITS_CURVE,
#endif
#if defined(PPM_CENTER_ADJUSTABLE)
  ITEM_LIMITS_PPM_CENTER,
#endif
#if defined(PPM_LIMITS_SYMETRICAL)
  ITEM_LIMITS_SYMETRICAL,
#endif
  ITEM_LIMITS_COUNT,
  ITEM_LIMITS_MAXROW = ITEM_LIMITS_COUNT-1
};

#define LIMITS_NAME_POS           4*FW
#define LIMITS_OFFSET_POS         14*FW+4
#define LIMITS_MIN_POS            20*FW-3
#if defined(PPM_CENTER_ADJUSTABLE)
  #define LIMITS_DIRECTION_POS    20*FW-3
    #define LIMITS_MAX_POS          24*FW+2
    #define LIMITS_REVERT_POS       25*FW-1
    #define LIMITS_CURVE_POS        27*FW-1
    #define LIMITS_PPM_CENTER_POS   34*FW
#else
  #define LIMITS_DIRECTION_POS    21*FW
  #define LIMITS_MAX_POS          26*FW
  #define LIMITS_REVERT_POS       27*FW
  #define LIMITS_CURVE_POS        32*FW-3
#endif

#define LIMITS_MIN_MAX_OFFSET 1000
#define CONVERT_US_MIN_MAX(x) (((x)*1280)/250)

#if defined(PPM_UNIT_US)
  #define SET_MIN_MAX(x, val)   x = ((val)*250)/128
  #define MIN_MAX_DISPLAY(x)    CONVERT_US_MIN_MAX(x)
#else
  #define MIN_MAX_DISPLAY(x)    (x)
  #define SET_MIN_MAX(x, val)   x = (val)
#endif

void onLimitsMenu(const char *result)
{
  int ch = menuVerticalPosition;

  if (result == STR_RESET) {
    LimitData *ld = limitAddress(ch);
    ld->min = 0;
    ld->max = 0;
    ld->offset = 0;
    ld->ppmCenter = 0;
    ld->revert = false;
    ld->curve = 0;
    storageDirty(EE_MODEL);
  }
  else if (result == STR_COPY_STICKS_TO_OFS) {
    copySticksToOffset(ch);
    storageDirty(EE_MODEL);
  }
  else if (result == STR_COPY_TRIMS_TO_OFS) {
    copyTrimsToOffset(ch);
    storageDirty(EE_MODEL);
  }
}

void menuModelLimits(event_t event)
{
  int sub = menuVerticalPosition;

  if (sub < MAX_OUTPUT_CHANNELS) {
#if defined(PPM_CENTER_ADJUSTABLE) || defined(PPM_UNIT_US)
    lcdDrawNumber(13*FW, 0, PPM_CH_CENTER(sub)+channelOutputs[sub]/2, RIGHT);
    lcdDrawText(13*FW, 0, STR_US);
#else
    lcdDrawNumber(13*FW, 0, calcRESXto1000(channelOutputs[sub]), PREC1|RIGHT);
#endif
  }

  MENU(STR_MENULIMITS, menuTabModel, MENU_MODEL_OUTPUTS, MAX_OUTPUT_CHANNELS+1, { NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, NAVIGATION_LINE_BY_LINE|ITEM_LIMITS_MAXROW, 0 });

  if (sub<MAX_OUTPUT_CHANNELS && menuHorizontalPosition>=0) {
    drawColumnHeader(STR_LIMITS_HEADERS, menuHorizontalPosition);
  }

  for (int i=0; i<NUM_BODY_LINES; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    uint8_t k = i+menuVerticalOffset;

    if (k==MAX_OUTPUT_CHANNELS) {
      // last line available - add the "copy trim menu" line
      uint8_t attr = (sub==MAX_OUTPUT_CHANNELS) ? INVERS : 0;
      lcdDrawText(CENTER_OFS, y, STR_TRIMS2OFFSETS, NO_HIGHLIGHT() ? 0 : attr);
      if (attr) {
        s_editMode = 0;
        if (event==EVT_KEY_LONG(KEY_ENTER)) {
          START_NO_HIGHLIGHT();
          killEvents(event);
          moveTrimsToOffsets(); // if highlighted and menu pressed - move trims to offsets
        }
      }
      return;
    }

    LimitData *ld = limitAddress(k);

    int16_t v = (ld->revert) ? -LIMIT_OFS(ld) : LIMIT_OFS(ld);
    char swVal = '-';  // '-', '<', '>'
    if ((channelOutputs[k] - v) > 50) swVal = (ld->revert ? 127 : 126); // Switch to raw inputs?  - remove trim!
    if ((channelOutputs[k] - v) < -50) swVal = (ld->revert ? 126 : 127);
    lcdDrawChar(LIMITS_DIRECTION_POS, y, swVal);

    int limit = (g_model.extendedLimits ? LIMIT_EXT_MAX : 1000);

    putsChn(0, y, k+1, (sub==k && menuHorizontalPosition < 0) ? INVERS : 0);
    if (sub==k && menuHorizontalPosition < 0 && event==EVT_KEY_LONG(KEY_ENTER) && !READ_ONLY()) {
      killEvents(event);
      POPUP_MENU_ADD_ITEM(STR_RESET);
      POPUP_MENU_ADD_ITEM(STR_COPY_TRIMS_TO_OFS);
      POPUP_MENU_ADD_ITEM(STR_COPY_STICKS_TO_OFS);
      POPUP_MENU_START(onLimitsMenu);
    }

    for (int j=0; j<ITEM_LIMITS_COUNT; j++) {
      LcdFlags attr = ((sub==k && menuHorizontalPosition==j) ? ((s_editMode>0) ? BLINK|INVERS : INVERS) : 0);
      uint8_t active = (attr && s_editMode>0) ;
      if (active) STICK_SCROLL_DISABLE();
      switch(j)
      {
        case ITEM_LIMITS_CH_NAME:
          editName(LIMITS_NAME_POS, y, ld->name, sizeof(ld->name), event, attr);
          break;

        case ITEM_LIMITS_OFFSET:
          if (GV_IS_GV_VALUE(ld->offset, -1000, 1000) || (attr && event == EVT_KEY_LONG(KEY_ENTER))) {
            ld->offset = GVAR_MENU_ITEM(LIMITS_OFFSET_POS, y, ld->offset, -1000, 1000, RIGHT|attr|PREC1, 0, event);
            break;
          }

#if defined(PPM_UNIT_US)
          lcdDrawNumber(LIMITS_OFFSET_POS, y, ((int32_t)ld->offset*128) / 25, attr|PREC1|RIGHT);
#else
          lcdDrawNumber(LIMITS_OFFSET_POS, y, ld->offset, attr|PREC1|RIGHT);
#endif
          if (active) {
            ld->offset = checkIncDec(event, ld->offset, -1000, 1000, EE_MODEL, NULL, stops1000);
          }
          else if (attr && event==EVT_KEY_LONG(KEY_MENU)) {
            copySticksToOffset(k);
            s_editMode = 0;
          }
          break;

        case ITEM_LIMITS_MIN:
          if (GV_IS_GV_VALUE(ld->min, -GV_RANGELARGE, GV_RANGELARGE) || (attr && event == EVT_KEY_LONG(KEY_ENTER))) {
            ld->min = GVAR_MENU_ITEM(LIMITS_MIN_POS, y, ld->min, -LIMIT_EXT_MAX, LIMIT_EXT_MAX, attr|PREC1|RIGHT, 0, event);
            break;
          }
          lcdDrawNumber(LIMITS_MIN_POS, y, MIN_MAX_DISPLAY(ld->min-LIMITS_MIN_MAX_OFFSET), attr|PREC1|RIGHT);
          if (active) ld->min = LIMITS_MIN_MAX_OFFSET + checkIncDec(event, ld->min-LIMITS_MIN_MAX_OFFSET, -limit, 0, EE_MODEL, NULL, stops1000);
          break;

        case ITEM_LIMITS_MAX:
          if (GV_IS_GV_VALUE(ld->max, -GV_RANGELARGE, GV_RANGELARGE) || (attr && event == EVT_KEY_LONG(KEY_ENTER))) {
            ld->max = GVAR_MENU_ITEM(LIMITS_MAX_POS, y, ld->max, -LIMIT_EXT_MAX, LIMIT_EXT_MAX, attr|PREC1|RIGHT, 0, event);
            break;
          }
          lcdDrawNumber(LIMITS_MAX_POS, y, MIN_MAX_DISPLAY(ld->max+LIMITS_MIN_MAX_OFFSET), attr|PREC1|RIGHT);
          if (active) ld->max = -LIMITS_MIN_MAX_OFFSET + checkIncDec(event, ld->max+LIMITS_MIN_MAX_OFFSET, 0, +limit, EE_MODEL, NULL, stops1000);
          break;

        case ITEM_LIMITS_DIRECTION:
        {
#if defined(PPM_CENTER_ADJUSTABLE)
          lcdDrawChar(LIMITS_REVERT_POS, y, ld->revert ? 127 : 126, attr);
#else
          lcdDrawTextAtIndex(LIMITS_REVERT_POS, y, STR_MMMINV, ld->revert, attr);
#endif
          if (active) {
            CHECK_INCDEC_MODELVAR_ZERO(event, ld->revert, 1);
          }
          break;
        }

#if defined(CURVES)
        case ITEM_LIMITS_CURVE:
          drawCurveName(LIMITS_CURVE_POS, y, ld->curve, attr);
          if (attr && event==EVT_KEY_LONG(KEY_ENTER) && ld->curve>0) {
            s_curveChan = (ld->curve<0 ? -ld->curve-1 : ld->curve-1);
            pushMenu(menuModelCurveOne);
          }
          if (active) {
            CHECK_INCDEC_MODELVAR(event, ld->curve, -MAX_CURVES, +MAX_CURVES);
          }
          break;
#endif

#if defined(PPM_CENTER_ADJUSTABLE)
        case ITEM_LIMITS_PPM_CENTER:
          lcdDrawNumber(LIMITS_PPM_CENTER_POS, y, PPM_CENTER+ld->ppmCenter, attr|RIGHT);
          if (active) {
            CHECK_INCDEC_MODELVAR(event, ld->ppmCenter, -PPM_CENTER_MAX, +PPM_CENTER_MAX);
          }
          break;
#endif

#if defined(PPM_LIMITS_SYMETRICAL)
        case ITEM_LIMITS_SYMETRICAL:
          lcdDrawChar(LCD_W-FW-MENUS_SCROLLBAR_WIDTH, y, ld->symetrical ? '=' : '\306', attr);
          if (active) {
            CHECK_INCDEC_MODELVAR_ZERO(event, ld->symetrical, 1);
          }
          break;
#endif
      }
    }
  }
}
