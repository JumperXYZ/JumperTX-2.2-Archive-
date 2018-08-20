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

enum MenuModelHeliItems {
  ITEM_HELI_SWASHTYPE,
  ITEM_HELI_SWASHRING,
#if defined(VIRTUAL_INPUTS)
  ITEM_HELI_ELE,
  ITEM_HELI_ELE_WEIGHT,
  ITEM_HELI_AIL,
  ITEM_HELI_AIL_WEIGHT,
  ITEM_HELI_COL,
  ITEM_HELI_COL_WEIGHT,
#else
  ITEM_HELI_COLLECTIVE,
  ITEM_HELI_ELEDIRECTION,
  ITEM_HELI_AILDIRECTION,
  ITEM_HELI_COLDIRECTION,
#endif
  ITEM_HELI_MAX
};

#if LCD_W >= 212
#define MODEL_HELI_2ND_COLUMN          (LCD_W-17*FW-MENUS_SCROLLBAR_WIDTH)
#else
#define MODEL_HELI_2ND_COLUMN          (14*FW)
#endif

void menuModelHeli(event_t event)
{
  SIMPLE_MENU(STR_MENUHELISETUP, menuTabModel, MENU_MODEL_HELI, HEADER_LINE+ITEM_HELI_MAX);

  uint8_t sub = menuVerticalPosition - HEADER_LINE;

#if defined(VIRTUAL_INPUTS)
  for (uint8_t i=0; i<NUM_BODY_LINES; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    uint8_t k = i + menuVerticalOffset;
#else
  for (uint8_t k=0; k<ITEM_HELI_MAX; k++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + k*FH;
#endif
    LcdFlags blink = (s_editMode > 0 ? BLINK|INVERS : INVERS);
    LcdFlags attr = (sub == k ? blink : 0);

    switch (k) {
      case ITEM_HELI_SWASHTYPE:
        g_model.swashR.type = editChoice(MODEL_HELI_2ND_COLUMN, y, STR_SWASHTYPE, STR_VSWASHTYPE, g_model.swashR.type, 0, SWASH_TYPE_MAX, attr, event);
        break;

      case ITEM_HELI_SWASHRING:
        lcdDrawTextAlignedLeft(y, STR_SWASHRING);
        lcdDrawNumber(MODEL_HELI_2ND_COLUMN, y, g_model.swashR.value, LEFT|attr);
        if (attr) CHECK_INCDEC_MODELVAR_ZERO(event, g_model.swashR.value, 100);
        break;

#if defined(VIRTUAL_INPUTS)
      case ITEM_HELI_ELE:
        lcdDrawTextAlignedLeft(y, STR_ELEVATOR);
        drawSource(MODEL_HELI_2ND_COLUMN, y, g_model.swashR.elevatorSource, attr);
        if (attr) CHECK_INCDEC_MODELSOURCE(event, g_model.swashR.elevatorSource, 0, MIXSRC_LAST_CH);
        break;

      case ITEM_HELI_ELE_WEIGHT:
        lcdDrawText(INDENT_WIDTH, y, STR_WEIGHT);
        lcdDrawNumber(MODEL_HELI_2ND_COLUMN, y, g_model.swashR.elevatorWeight, LEFT|attr);
        if (attr) CHECK_INCDEC_MODELVAR(event, g_model.swashR.elevatorWeight, -100, 100);
        break;

      case ITEM_HELI_AIL:
        lcdDrawTextAlignedLeft(y, STR_AILERON);
        drawSource(MODEL_HELI_2ND_COLUMN, y, g_model.swashR.aileronSource, attr);
        if (attr) CHECK_INCDEC_MODELSOURCE(event, g_model.swashR.aileronSource, 0, MIXSRC_LAST_CH);
        break;

      case ITEM_HELI_AIL_WEIGHT:
        lcdDrawText(INDENT_WIDTH, y, STR_WEIGHT);
        lcdDrawNumber(MODEL_HELI_2ND_COLUMN, y, g_model.swashR.aileronWeight, LEFT|attr);
        if (attr) CHECK_INCDEC_MODELVAR(event, g_model.swashR.aileronWeight, -100, 100);
        break;

      case ITEM_HELI_COL:
        lcdDrawTextAlignedLeft(y, STR_COLLECTIVE);
        drawSource(MODEL_HELI_2ND_COLUMN, y, g_model.swashR.collectiveSource, attr);
        if (attr) CHECK_INCDEC_MODELSOURCE(event, g_model.swashR.collectiveSource, 0, MIXSRC_LAST_CH);
        break;

      case ITEM_HELI_COL_WEIGHT:
        lcdDrawText(INDENT_WIDTH, y, STR_WEIGHT);
        lcdDrawNumber(MODEL_HELI_2ND_COLUMN, y, g_model.swashR.collectiveWeight, LEFT|attr);
        if (attr) CHECK_INCDEC_MODELVAR(event, g_model.swashR.collectiveWeight, -100, 100);
        break;
#else
      case ITEM_HELI_COLLECTIVE:
        g_model.swashR.collectiveSource = editChoice(MODEL_HELI_2ND_COLUMN, y, STR_COLLECTIVE, NULL, g_model.swashR.collectiveSource, 0, MIXSRC_LAST_CH, attr, event);
        drawSource(MODEL_HELI_2ND_COLUMN, y, g_model.swashR.collectiveSource, attr);
        break;

      case ITEM_HELI_ELEDIRECTION:
        g_model.swashR.invertELE = editChoice(MODEL_HELI_2ND_COLUMN, y, STR_ELEDIRECTION, STR_MMMINV, g_model.swashR.invertELE, 0, 1, attr, event);
        break;

      case ITEM_HELI_AILDIRECTION:
        g_model.swashR.invertAIL = editChoice(MODEL_HELI_2ND_COLUMN, y, STR_AILDIRECTION, STR_MMMINV, g_model.swashR.invertAIL, 0, 1, attr, event);
        break;

      case ITEM_HELI_COLDIRECTION:
        g_model.swashR.invertCOL = editChoice(MODEL_HELI_2ND_COLUMN, y, STR_COLDIRECTION, STR_MMMINV, g_model.swashR.invertCOL, 0, 1, attr, event);
        break;
#endif
    }
  }
}
