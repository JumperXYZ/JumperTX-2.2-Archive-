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

#ifndef _GUI_H_
#define _GUI_H_

#include "gui_common.h"
#include "lcd.h"
#include "menus.h"
#include "popups.h"

#define HEADER_LINE                    0
#define HEADER_LINE_COLUMNS

#define DEFAULT_SCROLLBAR_X            (LCD_W-1)
#define NUM_BODY_LINES                 (LCD_LINES-1)
#define MENU_HEADER_HEIGHT             FH
#define MENU_INIT_VPOS                 0

#define MODEL_BITMAP_WIDTH             64
#define MODEL_BITMAP_HEIGHT            32
#define MODEL_BITMAP_SIZE              BITMAP_BUFFER_SIZE(MODEL_BITMAP_WIDTH, MODEL_BITMAP_HEIGHT)
#define LOAD_MODEL_BITMAP()            loadModelBitmap(g_model.header.bitmap, modelBitmap)

#define CURVE_SIDE_WIDTH               (LCD_H/2)
#define CURVE_CENTER_X                 (LCD_W-CURVE_SIDE_WIDTH-2)
#define CURVE_CENTER_Y                 (LCD_H/2)

#define MIXES_2ND_COLUMN               (18*FW)

#define MENUS_SCROLLBAR_WIDTH          2

inline void drawFieldLabel(coord_t x, coord_t y, const char * str)
{
  lcdDrawText(0, y, str);
}

extern uint8_t modelBitmap[MODEL_BITMAP_SIZE];
bool loadModelBitmap(char * name, uint8_t * bitmap);

struct MenuItem {
  const char * name;
  const MenuHandlerFunc action;
};

// Temporary no highlight
extern uint8_t noHighlightCounter;
#define NO_HIGHLIGHT()        (noHighlightCounter > 0)
#define START_NO_HIGHLIGHT()  do { noHighlightCounter = 25; } while(0)

void drawCheckBox(coord_t x, coord_t y, uint8_t value, LcdFlags attr);
void drawSlider(coord_t x, coord_t y, uint8_t value, uint8_t max, uint8_t attr);
void drawSplash();
void drawSecondSplash();
void drawScreenIndex(uint8_t index, uint8_t count, uint8_t attr);
void drawVerticalScrollbar(coord_t x, coord_t y, coord_t h, uint16_t offset, uint16_t count, uint8_t visible);
void displayMenuBar(const MenuItem * menu, int index);
void drawProgressBar(const char * label, int num, int den);
void drawGauge(coord_t x, coord_t y, coord_t w, coord_t h, int32_t val, int32_t max);
void drawColumnHeader(const char * const * headers, uint8_t index);
void drawStick(coord_t centrex, int16_t xval, int16_t yval);

void drawAlertBox(const char * title, const char * text, const char * action);
#define ALERT_SOUND_ARG , uint8_t sound
void showAlertBox(const char * title, const char * text, const char * action, uint8_t sound);

void doMainScreenGraphics();

typedef uint16_t FlightModesType;

extern int8_t checkIncDec_Ret;  // global helper vars

#define EDIT_SELECT_MENU               -1
#define EDIT_SELECT_FIELD              0
#define EDIT_MODIFY_FIELD              1
#define EDIT_MODIFY_STRING             2
extern int8_t s_editMode; // global editmode

// checkIncDec flags
#define EE_GENERAL                     0x01
#define EE_MODEL                       0x02
#define NO_INCDEC_MARKS                0x04
#define INCDEC_SWITCH                  0x08
#define INCDEC_SOURCE                  0x10
#define INCDEC_REP10                   0x40
#define NO_DBLKEYS                     0x80

#define INCDEC_DECLARE_VARS(f)         uint8_t incdecFlag = (f); IsValueAvailable isValueAvailable = NULL
#define INCDEC_SET_FLAG(f)             incdecFlag = (f)
#define INCDEC_ENABLE_CHECK(fn)        isValueAvailable = fn
#define CHECK_INCDEC_PARAM(event, var, min, max) checkIncDec(event, var, min, max, incdecFlag, isValueAvailable)

// mawrow special values
#define TITLE_ROW                      ((uint8_t)-1)
#define HIDDEN_ROW                     ((uint8_t)-2)

struct CheckIncDecStops
{
  const int count;
  const int stops[];
  int min() const
  {
    return stops[0];
  }
  int max() const
  {
    return stops[count-1];
  }
  bool contains(int value) const
  {
    for (int i=0; i<count; ++i) {
      int stop = stops[i];
      if (value == stop)
        return true;
      else if (value < stop)
        return false;
    }
    return false;
  }
};

extern const CheckIncDecStops &stops100;
extern const CheckIncDecStops &stops1000;
extern const CheckIncDecStops &stopsSwitch;

#define INIT_STOPS(var, ...)                                        \
  const int _ ## var[] = { __VA_ARGS__ };                           \
  const CheckIncDecStops &var  = (const CheckIncDecStops&)_ ## var;
#define CATEGORY_END(val)                                          \
  (val), (val+1)

int checkIncDec(event_t event, int val, int i_min, int i_max, unsigned int i_flags=0, IsValueAvailable isValueAvailable=NULL, const CheckIncDecStops &stops=stops100);
#define checkIncDecModel(event, i_val, i_min, i_max) checkIncDec(event, i_val, i_min, i_max, EE_MODEL)
#define checkIncDecModelZero(event, i_val, i_max) checkIncDec(event, i_val, 0, i_max, EE_MODEL)
#define checkIncDecGen(event, i_val, i_min, i_max) checkIncDec(event, i_val, i_min, i_max, EE_GENERAL)

#define CHECK_INCDEC_MODELVAR(event, var, min, max) \
  var = checkIncDecModel(event, var, min, max)

#define CHECK_INCDEC_MODELVAR_ZERO(event, var, max) \
  var = checkIncDecModelZero(event, var, max)

#define CHECK_INCDEC_MODELVAR_CHECK(event, var, min, max, check) \
  var = checkIncDec(event, var, min, max, EE_MODEL, check)

#define CHECK_INCDEC_MODELVAR_ZERO_CHECK(event, var, max, check) \
  var = checkIncDec(event, var, 0, max, EE_MODEL, check)

#define AUTOSWITCH_ENTER_LONG() (attr && event==EVT_KEY_LONG(KEY_ENTER))
#define CHECK_INCDEC_SWITCH(event, var, min, max, flags, available) \
  var = checkIncDec(event, var, min, max, (flags)|INCDEC_SWITCH, available)
#define CHECK_INCDEC_MODELSWITCH(event, var, min, max, available) \
  CHECK_INCDEC_SWITCH(event, var, min, max, EE_MODEL, available)

#define CHECK_INCDEC_MODELSOURCE(event, var, min, max) \
  var = checkIncDec(event, var, min, max, EE_MODEL|INCDEC_SOURCE|NO_INCDEC_MARKS, isSourceAvailable)

#define CHECK_INCDEC_GENVAR(event, var, min, max) \
  var = checkIncDecGen(event, var, min, max)

#define NAVIGATION_LINE_BY_LINE  0x40
#define CURSOR_ON_LINE()         (menuHorizontalPosition<0)

#define CHECK_FLAG_NO_SCREEN_INDEX   1
void check(const char * title, event_t event, uint8_t curr, const MenuHandlerFunc *menuTab, uint8_t menuTabSize, const pm_uint8_t *horTab, uint8_t horTabMax, vertpos_t maxrow, uint8_t flags=0);
void check_simple(const char *title, event_t event, uint8_t curr, const MenuHandlerFunc *menuTab, uint8_t menuTabSize, vertpos_t maxrow);
void check_submenu_simple(const char * title, event_t event, uint8_t maxrow);

void title(const pm_char * s);
#define TITLE(str) title(str)

#define MENU_TAB(...) const uint8_t mstate_tab[] = __VA_ARGS__

#define MENU_CHECK(title, tab, menu, lines_count) \
  check(title, event, menu, tab, DIM(tab), mstate_tab, DIM(mstate_tab)-1, lines_count)

#define MENU_CHECK_FLAGS(title, tab, menu, flags, lines_count) \
  check(title, event, menu, tab, DIM(tab), mstate_tab, DIM(mstate_tab)-1, lines_count, flags)

#define MENU(title, tab, menu, lines_count, ...) \
  MENU_TAB(__VA_ARGS__); \
  MENU_CHECK(title, tab, menu, lines_count)

#define MENU_FLAGS(title, tab, menu, flags, lines_count, ...) \
  MENU_TAB(__VA_ARGS__); \
  MENU_CHECK_FLAGS(title, tab, menu, flags, lines_count)

#define SIMPLE_MENU_FLAGS(title, tab, menu, flags, lines_count, ...) \
  check(title, event, menu, tab, DIM(tab), NULL, 0, lines_count, flags)

#define SIMPLE_MENU(title, tab, menu, lines_count) \
  check_simple(title, event, menu, tab, DIM(tab), lines_count)

#define SUBMENU_NOTITLE(lines_count, ...) { \
  MENU_TAB(__VA_ARGS__); \
  check(NULL, event, 0, NULL, 0, mstate_tab, DIM(mstate_tab)-1, lines_count); \
  }

#define SUBMENU(title, lines_count, ...) \
  MENU_TAB(__VA_ARGS__); \
  check(title, event, 0, NULL, 0, mstate_tab, DIM(mstate_tab)-1, lines_count)

#define SIMPLE_SUBMENU_NOTITLE(lines_count) \
  check_submenu_simple(NULL, event, lines_count);

#define SIMPLE_SUBMENU(title, lines_count) \
  check_submenu_simple(title, event, lines_count)

typedef int choice_t;

choice_t editChoice(coord_t x, coord_t y, const pm_char *label, const pm_char *values, choice_t value, choice_t min, choice_t max, LcdFlags attr, event_t event);
uint8_t editCheckBox(uint8_t value, coord_t x, coord_t y, const pm_char *label, LcdFlags attr, event_t event);
swsrc_t editSwitch(coord_t x, coord_t y, swsrc_t value, LcdFlags attr, event_t event);

#define ON_OFF_MENU_ITEM(value, x, y, label, attr, event) value = editCheckBox(value, x, y, label, attr, event)

#if defined(GVARS)
  void drawGVarName(coord_t x, coord_t y, int8_t index, LcdFlags flags=0);
  void drawGVarValue(coord_t x, coord_t y, uint8_t gvar, gvar_t value, LcdFlags flags=0);
  int16_t editGVarFieldValue(coord_t x, coord_t y, int16_t value, int16_t min, int16_t max, LcdFlags attr, uint8_t editflags, event_t event);
  #define GVAR_MENU_ITEM(x, y, v, min, max, lcdattr, editflags, event) editGVarFieldValue(x, y, v, min, max, lcdattr, editflags, event)
  #define displayGVar(x, y, v, min, max) GVAR_MENU_ITEM(x, y, v, min, max, 0, 0, 0)
#else
  #define GVAR_MENU_ITEM(x, y, v, min, max, lcdattr, editflags, event) editGVarFieldValue(x, y, v, min, max, lcdattr, event)
  int16_t editGVarFieldValue(coord_t x, coord_t y, int16_t value, int16_t min, int16_t max, LcdFlags attr, event_t event);
  #define displayGVar(x, y, v, min, max) lcdDrawNumber(x, y, v)
#endif

void gvarWeightItem(coord_t x, coord_t y, MixData * md, LcdFlags attr, event_t event);

extern uint8_t s_curveChan;
void editCurveRef(coord_t x, coord_t y, CurveRef & curve, event_t event, LcdFlags flags);

extern uint8_t editNameCursorPos;
void editName(coord_t x, coord_t y, char * name, uint8_t size, event_t event, uint8_t active, LcdFlags attr=ZCHAR);
void editSingleName(coord_t x, coord_t y, const pm_char * label, char * name, uint8_t size, event_t event, uint8_t active);

uint8_t editDelay(coord_t y, event_t event, uint8_t attr, const char * str, uint8_t delay);
#define EDIT_DELAY(y, event, attr, str, delay) editDelay(y, event, attr, str, delay)

#define WARNING_TYPE_ASTERISK          0
#define WARNING_TYPE_CONFIRM           1
#define WARNING_TYPE_INPUT             2

void copySelection(char * dst, const char * src, uint8_t size);

extern const pm_char * warningText;
extern const pm_char * warningInfoText;
extern uint8_t         warningInfoLength;
extern uint8_t         warningResult;
extern uint8_t         warningType;

#define COPY_MODE 1
#define MOVE_MODE 2
extern uint8_t s_copyMode;
extern int8_t s_copySrcRow;
extern int8_t s_copyTgtOfs;
extern uint8_t s_currIdx;
extern uint8_t s_maxLines;
extern uint8_t s_copySrcIdx;
extern uint8_t s_copySrcCh;
extern int8_t s_currCh;

uint8_t getExposCount();
void deleteExpo(uint8_t idx);
void insertExpo(uint8_t idx);

uint8_t getMixesCount();
void deleteMix(uint8_t idx);
void insertMix(uint8_t idx);

#define STATUS_LINE_LENGTH             32
extern char statusLineMsg[STATUS_LINE_LENGTH];
void showStatusLine();
void drawStatusLine();

#define TEXT_FILENAME_MAXLEN           40
extern char s_text_file[TEXT_FILENAME_MAXLEN];
void menuTextView(event_t event);
void pushMenuTextView(const char *filename);
void pushModelNotes();
void readModelNotes();

void menuChannelsView(event_t event);

#define LABEL(...) (uint8_t)-1

#if defined(ROTARY_ENCODER_NAVIGATION)
#define CURSOR_MOVED_LEFT(event)       (event==EVT_ROTARY_LEFT)
#define CURSOR_MOVED_RIGHT(event)      (event==EVT_ROTARY_RIGHT)
#else
#define CURSOR_MOVED_LEFT(event)       (EVT_KEY_MASK(event) == KEY_LEFT)
#define CURSOR_MOVED_RIGHT(event)      (EVT_KEY_MASK(event) == KEY_RIGHT)
#endif

#define REPEAT_LAST_CURSOR_MOVE()      { if (CURSOR_MOVED_LEFT(event) || CURSOR_MOVED_RIGHT(event)) putEvent(event); else menuHorizontalPosition = 0; }
#define MOVE_CURSOR_FROM_HERE()        if (menuHorizontalPosition > 0) REPEAT_LAST_CURSOR_MOVE()

#define MENU_FIRST_LINE_EDIT           (menuTab ? (MAXCOL((uint16_t)0) >= HIDDEN_ROW ? (MAXCOL((uint16_t)1) >= HIDDEN_ROW ? 2 : 1) : 0) : 0)
#define POS_HORZ_INIT(posVert)         ((COLATTR(posVert) & NAVIGATION_LINE_BY_LINE) ? -1 : 0)
#define EDIT_MODE_INIT                 0 // TODO enum

typedef int (*FnFuncP) (int x);
void drawFunction(FnFuncP fn, uint8_t offset=0);

void onSourceLongEnterPress(const char *result);

uint8_t switchToMix(uint8_t source);

extern coord_t scrollbar_X;
#define SET_SCROLLBAR_X(x) scrollbar_X = (x);

extern const pm_uchar sticks[] PROGMEM;

#if defined(FLIGHT_MODES)
void displayFlightModes(coord_t x, coord_t y, FlightModesType value);
FlightModesType editFlightModes(coord_t x, coord_t y, event_t event, FlightModesType value, uint8_t attr);
#else
#define displayFlightModes(...)
#endif

#define IS_MAIN_VIEW_DISPLAYED()       menuHandlers[0] == menuMainView
#define IS_TELEMETRY_VIEW_DISPLAYED()  menuHandlers[0] == menuViewTelemetryFrsky
#define IS_OTHER_VIEW_DISPLAYED()      (menuHandlers[0] == menuMainViewChannelsMonitor || menuHandlers[0] == menuChannelsView)

#endif // _GUI_H_
