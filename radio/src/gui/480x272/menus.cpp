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

MenuHandlerFunc menuHandlers[5];
event_t menuEvent = 0;
uint8_t menuVerticalPositions[4];
uint8_t menuLevel = 0;

void popMenu()
{
  assert(menuLevel>0);
  menuLevel = menuLevel-1;
  menuEvent = EVT_ENTRY_UP;
  AUDIO_KEY_PRESS();
  TRACE("popMenu(%d)", menuLevel);
}

void chainMenu(MenuHandlerFunc newMenu)
{
  menuHandlers[menuLevel] = newMenu;
  menuEvent = EVT_ENTRY;
  AUDIO_KEY_PRESS();
  TRACE("chainMenu(%d, %p)", menuLevel, newMenu);
}

void pushMenu(MenuHandlerFunc newMenu)
{
  killEvents(KEY_ENTER);

  if (menuLevel == 0) {
    if (newMenu == menuRadioSetup)
      menuVerticalPositions[0] = 1;
    if (newMenu == menuModelSetup)
      menuVerticalPositions[0] = 0;
  }
  else {
    menuVerticalPositions[menuLevel] = menuVerticalPosition;
  }

  menuLevel++;

  assert(menuLevel < DIM(menuHandlers));

  menuHandlers[menuLevel] = newMenu;
  menuEvent = EVT_ENTRY;
  AUDIO_KEY_PRESS();
  TRACE("pushMenu(%d, %p)", menuLevel, newMenu);
}

void readModelNotes()
{
  LED_ERROR_BEGIN();

  strcpy(s_text_file, MODELS_PATH "/");
  char *buf = strcat_currentmodelname(&s_text_file[sizeof(MODELS_PATH)]);
  strcpy(buf, TEXT_EXT);
  if (!isFileAvailable(s_text_file)) {
    char *buf = strAppendFilename(&s_text_file[sizeof(MODELS_PATH)], g_eeGeneral.currModelFilename, LEN_MODEL_FILENAME);
    strcpy(buf, TEXT_EXT);
  }

  clearKeyEvents();
  event_t event = EVT_ENTRY;
  while (event != EVT_KEY_FIRST(KEY_EXIT)) {
    lcdRefreshWait();
    lcdClear();
    menuTextView(event);
    event = getEvent();
    lcdRefresh();
    wdt_reset();
  }

  LED_ERROR_END();
}

bool menuModelNotes(event_t event)
{
  if (event == EVT_ENTRY) {
    strcpy(s_text_file, MODELS_PATH "/");
    char *buf = strcat_currentmodelname(&s_text_file[sizeof(MODELS_PATH)]);
    strcpy(buf, TEXT_EXT);
    if (!isFileAvailable(s_text_file)) {
      char *buf = strAppendFilename(&s_text_file[sizeof(MODELS_PATH)], g_eeGeneral.currModelFilename, LEN_MODEL_FILENAME);
      strcpy(buf, TEXT_EXT);
    }
  }
  return menuTextView(event);
}

void pushModelNotes()
{
  pushMenu(menuModelNotes);
}
