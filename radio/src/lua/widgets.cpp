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

#include <ctype.h>
#include <stdio.h>
#include "opentx.h"
#include "bin_allocator.h"
#include "lua_api.h"

#define WIDGET_SCRIPTS_MAX_INSTRUCTIONS    (10000/100)
#define MANUAL_SCRIPTS_MAX_INSTRUCTIONS    (20000/100)
#define LUA_WARNING_INFO_LEN               64

lua_State *lsWidgets = NULL;
extern int custom_lua_atpanic(lua_State *L);

#define LUA_WIDGET_FILENAME                "/main.lua"
#define LUA_FULLPATH_MAXLEN                (LEN_FILE_PATH_MAX + LEN_SCRIPT_FILENAME + LEN_FILE_EXTENSION_MAX)  // max length (example: /SCRIPTS/THEMES/mytheme.lua)

void exec(int function, int nresults=0)
{
  if (lsWidgets == 0) return;

  if (function) {
    luaSetInstructionsLimit(lsWidgets, WIDGET_SCRIPTS_MAX_INSTRUCTIONS);
    lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, function);
    if (lua_pcall(lsWidgets, 0, nresults, 0) != 0) {
      TRACE("Error in theme  %s", lua_tostring(lsWidgets, -1));
      // TODO disable theme - revert back to default theme???
    }
  }
}

ZoneOption * createOptionsArray(int reference, uint8_t maxOptions)
{
  if (reference == 0) {
    // TRACE("createOptionsArray() no options");
    return NULL;
  }

  int count = 0;
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, reference);
  for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2); lua_pop(lsWidgets, 1)) {
    count++;
  }

  // TRACE("we have %d options", count);
  if (count > maxOptions) {
    count = maxOptions;
    // TRACE("limited to %d options", count);
  }

  ZoneOption * options = (ZoneOption *)malloc(sizeof(ZoneOption) * (count+1));
  if (!options) {
    return NULL;
  }

  PROTECT_LUA() {
    lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, reference);
    ZoneOption * option = options;
    for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2), count-- > 0; lua_pop(lsWidgets, 1)) {
      // TRACE("parsing option %d", count);
      luaL_checktype(lsWidgets, -2, LUA_TNUMBER); // key is number
      luaL_checktype(lsWidgets, -1, LUA_TTABLE); // value is table
      uint8_t field = 0;
      for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2) && field<5; lua_pop(lsWidgets, 1), field++) {
        switch (field) {
          case 0:
            luaL_checktype(lsWidgets, -2, LUA_TNUMBER); // key is number
            luaL_checktype(lsWidgets, -1, LUA_TSTRING); // value is string
            option->name = lua_tostring(lsWidgets, -1);
            // TRACE("name = %s", option->name);
            break;
          case 1:
            luaL_checktype(lsWidgets, -2, LUA_TNUMBER); // key is number
            luaL_checktype(lsWidgets, -1, LUA_TNUMBER); // value is number
            option->type = (ZoneOption::Type)lua_tointeger(lsWidgets, -1);
            // TRACE("type = %d", option->type);
            if (option->type > ZoneOption::Color) {
              // wrong type
              option->type = ZoneOption::Integer;
            }
            if (option->type == ZoneOption::Integer) {
              // set some sensible defaults (only Integer actually uses them)
              option->deflt.signedValue = 0;
              option->min.signedValue = -100;
              option->max.signedValue = 100;
            }
            break;
          case 2:
            luaL_checktype(lsWidgets, -2, LUA_TNUMBER); // key is number
            if (option->type == ZoneOption::Integer) {
              luaL_checktype(lsWidgets, -1, LUA_TNUMBER); // value is number
              option->deflt.signedValue = lua_tointeger(lsWidgets, -1);
              // TRACE("default signed = %d", option->deflt.signedValue);
            }
            else if (option->type == ZoneOption::Source ||
                     option->type == ZoneOption::TextSize ||
                     option->type == ZoneOption::Color) {
              luaL_checktype(lsWidgets, -1, LUA_TNUMBER); // value is number
              option->deflt.unsignedValue = lua_tounsigned(lsWidgets, -1);
              // TRACE("default unsigned = %u", option->deflt.unsignedValue);
            }
            else if (option->type == ZoneOption::Bool) {
              luaL_checktype(lsWidgets, -1, LUA_TNUMBER); // value is number
              option->deflt.boolValue = (lua_tounsigned(lsWidgets, -1) != 0);
              // TRACE("default bool = %d", (int)(option->deflt.boolValue));
            }
            else if (option->type == ZoneOption::String) {
              str2zchar(option->deflt.stringValue, lua_tostring(lsWidgets, -1), sizeof(option->deflt.stringValue));  // stringValue is ZCHAR
              // TRACE("default string = %s", lua_tostring(lsWidgets, -1));
            }
            break;
          case 3:
            if (option->type == ZoneOption::Integer) {
              luaL_checktype(lsWidgets, -2, LUA_TNUMBER); // key is number
              luaL_checktype(lsWidgets, -1, LUA_TNUMBER); // value is number
              option->min.signedValue = lua_tointeger(lsWidgets, -1);
            }
            break;
          case 4:
            if (option->type == ZoneOption::Integer) {
              luaL_checktype(lsWidgets, -2, LUA_TNUMBER); // key is number
              luaL_checktype(lsWidgets, -1, LUA_TNUMBER); // value is number
              option->max.signedValue = lua_tointeger(lsWidgets, -1);
            }
            break;
        }
      }
      option++;
    }
    option->name = NULL; // sentinel
  }
  else {
    TRACE("error in theme/widget options");
    free(options);
    return NULL;
  }
  UNPROTECT_LUA();
  return options;
}

class LuaTheme: public Theme
{
  friend void luaLoadThemeCallback();

  public:
    LuaTheme(const char * name, ZoneOption * options):
      Theme(name, options),
      loadFunction(0),
      drawBackgroundFunction(0),
      drawTopbarBackgroundFunction(0),
      drawAlertBoxFunction(0)
    {
    }

    virtual void load() const
    {
      luaLcdAllowed = true;
      exec(loadFunction);
    }

    virtual void drawBackground() const
    {
      exec(drawBackgroundFunction);
    }

    virtual void drawTopbarBackground(uint8_t icon) const
    {
      exec(drawTopbarBackgroundFunction);
    }

#if 0
    virtual void drawAlertBox(const char * title, const char * text, const char * action) const
    {
      exec(drawAlertBoxFunction);
    }
#endif

  protected:
    int loadFunction;
    int drawBackgroundFunction;
    int drawTopbarBackgroundFunction;
    int drawAlertBoxFunction;
};

void luaLoadThemeCallback()
{
  TRACE("luaLoadThemeCallback()");
  const char * name=NULL;
  int themeOptions=0, loadFunction=0, drawBackgroundFunction=0, drawTopbarBackgroundFunction=0;

  luaL_checktype(lsWidgets, -1, LUA_TTABLE);

  for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2); lua_pop(lsWidgets, 1)) {
    const char * key = lua_tostring(lsWidgets, -2);
    if (!strcmp(key, "name")) {
      name = luaL_checkstring(lsWidgets, -1);
    }
    else if (!strcmp(key, "options")) {
      themeOptions = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
    else if (!strcmp(key, "load")) {
      loadFunction = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
    else if (!strcmp(key, "drawBackground")) {
      drawBackgroundFunction = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
    else if (!strcmp(key, "drawTopbarBackground")) {
      drawTopbarBackgroundFunction = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
  }

  if (name) {
    ZoneOption * options = NULL;
    if (themeOptions) {
      options = createOptionsArray(themeOptions, MAX_THEME_OPTIONS);
      if (!options)
        return;
    }
    LuaTheme * theme = new LuaTheme(name, options);
    theme->loadFunction = loadFunction;
    theme->drawBackgroundFunction = drawBackgroundFunction;
    theme->drawTopbarBackgroundFunction = drawTopbarBackgroundFunction;   // NOSONAR
    TRACE("Loaded Lua theme %s", name);
  }
}

class LuaWidget: public Widget
{
  public:
    LuaWidget(const WidgetFactory * factory, const Zone & zone, Widget::PersistentData * persistentData, int widgetData):
      Widget(factory, zone, persistentData),
      widgetData(widgetData),
      errorMessage(0)
    {
    }

    virtual ~LuaWidget()
    {
      luaL_unref(lsWidgets, LUA_REGISTRYINDEX, widgetData);
      if (errorMessage) free(errorMessage);
    }

    virtual void update();

    virtual void refresh();

    virtual void background();

    virtual const char * getErrorMessage() const;

  protected:
    int widgetData;
    char * errorMessage;

    void setErrorMessage(const char * funcName);
};

void l_pushtableint(const char * key, int value)
{
  lua_pushstring(lsWidgets, key);
  lua_pushinteger(lsWidgets, value);
  lua_settable(lsWidgets, -3);
}

class LuaWidgetFactory: public WidgetFactory
{
  friend void luaLoadWidgetCallback();
  friend class LuaWidget;

  public:
    LuaWidgetFactory(const char * name, ZoneOption * widgetOptions, int createFunction):
      WidgetFactory(name, widgetOptions),
      createFunction(createFunction),
      updateFunction(0),
      refreshFunction(0),
      backgroundFunction(0)
    {
    }

    virtual Widget * create(const Zone & zone, Widget::PersistentData * persistentData, bool init=true) const
    {
      if (lsWidgets == 0) return 0;
      if (init) {
        initPersistentData(persistentData);
      }

      luaSetInstructionsLimit(lsWidgets, WIDGET_SCRIPTS_MAX_INSTRUCTIONS);
      lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, createFunction);

      lua_newtable(lsWidgets);
      l_pushtableint("x", zone.x);
      l_pushtableint("y", zone.y);
      l_pushtableint("w", zone.w);
      l_pushtableint("h", zone.h);

      lua_newtable(lsWidgets);
      int i = 0;
      for (const ZoneOption * option = options; option->name; option++, i++) {
        l_pushtableint(option->name, persistentData->options[i].signedValue);
      }

      if (lua_pcall(lsWidgets, 2, 1, 0) != 0) {
        TRACE("Error in widget %s create() function: %s", getName(), lua_tostring(lsWidgets, -1));
      }
      int widgetData = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      Widget * widget = new LuaWidget(this, zone, persistentData, widgetData);
      return widget;
    }

  protected:
    int createFunction;
    int updateFunction;
    int refreshFunction;
    int backgroundFunction;
};

void LuaWidget::update()
{
  if (lsWidgets == 0 || errorMessage) return;

  luaSetInstructionsLimit(lsWidgets, WIDGET_SCRIPTS_MAX_INSTRUCTIONS);
  LuaWidgetFactory * factory = (LuaWidgetFactory *)this->factory;
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, factory->updateFunction);
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, widgetData);

  lua_newtable(lsWidgets);
  int i = 0;
  for (const ZoneOption * option = getOptions(); option->name; option++, i++) {
    l_pushtableint(option->name, persistentData->options[i].signedValue);
  }

  if (lua_pcall(lsWidgets, 2, 0, 0) != 0) {
    setErrorMessage("update()");
  }
}

void LuaWidget::setErrorMessage(const char * funcName)
{
  TRACE("Error in widget %s %s function: %s", factory->getName(), funcName, lua_tostring(lsWidgets, -1));
  TRACE("Widget disabled");
  size_t needed = snprintf(NULL, 0, "%s: %s", funcName, lua_tostring(lsWidgets, -1)) + 1;
  errorMessage = (char *)malloc(needed);
  if (errorMessage) {
    snprintf(errorMessage, needed, "%s: %s", funcName, lua_tostring(lsWidgets, -1));
  }
}

const char * LuaWidget::getErrorMessage() const
{
  return errorMessage;
}

void LuaWidget::refresh()
{
  if (lsWidgets == 0) return;

  if (errorMessage) {
    lcdSetColor(RED);
    lcdDrawText(zone.x, zone.y, "Disabled", SMLSIZE|CUSTOM_COLOR);
    return;
  }

  luaSetInstructionsLimit(lsWidgets, WIDGET_SCRIPTS_MAX_INSTRUCTIONS);
  LuaWidgetFactory * factory = (LuaWidgetFactory *)this->factory;
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, factory->refreshFunction);
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, widgetData);
  if (lua_pcall(lsWidgets, 1, 0, 0) != 0) {
    setErrorMessage("refresh()");
  }
}

void LuaWidget::background()
{
  if (lsWidgets == 0 || errorMessage) return;

  luaSetInstructionsLimit(lsWidgets, WIDGET_SCRIPTS_MAX_INSTRUCTIONS);
  LuaWidgetFactory * factory = (LuaWidgetFactory *)this->factory;
  if (factory->backgroundFunction) {
    lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, factory->backgroundFunction);
    lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, widgetData);
    if (lua_pcall(lsWidgets, 1, 0, 0) != 0) {
      setErrorMessage("background()");
    }
  }
}

void luaLoadWidgetCallback()
{
  TRACE("luaLoadWidgetCallback()");
  const char * name=NULL;
  int widgetOptions=0, createFunction=0, updateFunction=0, refreshFunction=0, backgroundFunction=0;

  luaL_checktype(lsWidgets, -1, LUA_TTABLE);

  for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2); lua_pop(lsWidgets, 1)) {
    const char * key = lua_tostring(lsWidgets, -2);
    if (!strcmp(key, "name")) {
      name = luaL_checkstring(lsWidgets, -1);
    }
    else if (!strcmp(key, "options")) {
      widgetOptions = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
    else if (!strcmp(key, "create")) {
      createFunction = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
    else if (!strcmp(key, "update")) {
      updateFunction = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
    else if (!strcmp(key, "refresh")) {
      refreshFunction = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
    else if (!strcmp(key, "background")) {
      backgroundFunction = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
  }

  if (name && createFunction) {
    ZoneOption * options = createOptionsArray(widgetOptions, MAX_WIDGET_OPTIONS);
    if (options) {
      LuaWidgetFactory * factory = new LuaWidgetFactory(name, options, createFunction);
      factory->updateFunction = updateFunction;
      factory->refreshFunction = refreshFunction;
      factory->backgroundFunction = backgroundFunction;   // NOSONAR
      TRACE("Loaded Lua widget %s", name);
    }
  }
}

void luaLoadFile(const char * filename, void (*callback)())
{
  if (lsWidgets == NULL || callback == NULL)
    return;

  TRACE("luaLoadFile(%s)", filename);

  luaSetInstructionsLimit(lsWidgets, MANUAL_SCRIPTS_MAX_INSTRUCTIONS);

  PROTECT_LUA() {
    if (luaLoadScriptFileToState(lsWidgets, filename, LUA_SCRIPT_LOAD_MODE) == SCRIPT_OK) {
      if (lua_pcall(lsWidgets, 0, 1, 0) == LUA_OK && lua_istable(lsWidgets, -1)) {
        (*callback)();
      }
      else {
        TRACE("luaLoadFile(%s): Error parsing script: %s", filename, lua_tostring(lsWidgets, -1));
      }
    }
  }
  else {
    // error while loading Lua widget/theme,
    // do not disable whole Lua state, just ingnore bad widget/theme
    return;
  }
  UNPROTECT_LUA();
}

void luaLoadFiles(const char * directory, void (*callback)())
{
  char path[LUA_FULLPATH_MAXLEN+1];
  FILINFO fno;
  DIR dir;

  strcpy(path, directory);
  TRACE("luaLoadFiles() %s", path);

  FRESULT res = f_opendir(&dir, path);        /* Open the directory */

  if (res == FR_OK) {
    int pathlen = strlen(path);
    path[pathlen++] = '/';
    for (;;) {
      res = f_readdir(&dir, &fno);                   /* Read a directory item */
      if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
      uint8_t len = strlen(fno.fname);
      if (len > 0 && (unsigned int)(len + pathlen + sizeof(LUA_WIDGET_FILENAME)) <= sizeof(path) &&
          fno.fname[0]!='.' && (fno.fattrib & AM_DIR)) {
        strcpy(&path[pathlen], fno.fname);
        strcat(&path[pathlen], LUA_WIDGET_FILENAME);
        if (isFileAvailable(path)) {
          luaLoadFile(path, callback);
        }
      }
    }
  }
  else {
    TRACE("f_opendir(%s) failed, code=%d", path, res);
  }

  f_closedir(&dir);
}

#if defined(LUA_ALLOCATOR_TRACER)
LuaMemTracer lsWidgetsTrace;
#endif

void luaInitThemesAndWidgets()
{
  TRACE("luaInitThemesAndWidgets");

#if defined(USE_BIN_ALLOCATOR)
  lsWidgets = lua_newstate(bin_l_alloc, NULL);   //we use our own allocator!
#elif defined(LUA_ALLOCATOR_TRACER)
  memset(&lsWidgetsTrace, 0 , sizeof(lsWidgetsTrace));
  lsWidgetsTrace.script = "lua_newstate(widgets)";
  lsWidgets = lua_newstate(tracer_alloc, &lsWidgetsTrace);   //we use tracer allocator
#else
  lsWidgets = lua_newstate(l_alloc, NULL);   //we use Lua default allocator
#endif
  if (lsWidgets) {
    // install our panic handler
    lua_atpanic(lsWidgets, &custom_lua_atpanic);

#if defined(LUA_ALLOCATOR_TRACER)
    lua_sethook(lsWidgets, luaHook, LUA_MASKLINE, 0);
#endif

    // protect libs and constants registration
    PROTECT_LUA() {
      luaRegisterLibraries(lsWidgets);
    }
    else {
      // if we got panic during registration
      // we disable Lua for this session
      // luaDisable();
      luaClose(&lsWidgets);
      lsWidgets = 0;
    }
    UNPROTECT_LUA();
    TRACE("lsWidgets %p", lsWidgets);
    luaLoadFiles(THEMES_PATH, luaLoadThemeCallback);
    luaLoadFiles(WIDGETS_PATH, luaLoadWidgetCallback);
    luaDoGc(lsWidgets, true);
  }
}
