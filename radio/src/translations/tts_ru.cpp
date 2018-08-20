/*
 * Author
 * - Alexander Novikov (mr.pokryshkin@gmail.com)
 *
 * tts_ru is based on tts_de and tts_en
 *
 * opentx is based on code named
 * gruvin9x by Bryan J. Rentoul: http://code.google.com/p/gruvin9x/,
 * er9x by Erez Raviv: http://code.google.com/p/er9x/,
 * and the original (and ongoing) project by
 * Thomas Husterer, th9x: http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "opentx.h"

enum RusPrompts {
  RU_PROMPT_NUMBERS_BASE = 0,
  RU_PROMPT_ZERO = RU_PROMPT_NUMBERS_BASE+0,       //02-99
  RU_PROMPT_HUNDRED = RU_PROMPT_NUMBERS_BASE+100,  //100,200 .. 900

  RU_PROMPT_AND = RU_PROMPT_NUMBERS_BASE+110, // и
  RU_PROMPT_MINUS = RU_PROMPT_NUMBERS_BASE+111, // минус
  RU_PROMPT_UNITS_BASE = RU_PROMPT_NUMBERS_BASE + 113,
  RU_PROMPT_POINT_BASE = RU_PROMPT_NUMBERS_BASE + 165, //.0 - .9
  RU_PROMPT_FEMALE_ONE = RU_PROMPT_NUMBERS_BASE + 180,
  RU_PROMPT_FEMALE_TWO = RU_PROMPT_NUMBERS_BASE + 190,

  RU_PROMPT_THOUSAND1 = RU_PROMPT_NUMBERS_BASE+200, //1000
  RU_PROMPT_THOUSAND2 = RU_PROMPT_NUMBERS_BASE+201, //2000
  RU_PROMPT_THOUSAND5 = RU_PROMPT_NUMBERS_BASE+202, //5000
};
#define MALE     0x00
#define FEMALE     0x01
#define RU_FEMALE_UNIT 0xFF

#if defined(VOICE)

#if defined(CPUARM)
  #define RU_PUSH_UNIT_PROMPT(u, p) ru_pushUnitPrompt((u), (p), id)
#else
  #define RU_PUSH_UNIT_PROMPT(u, p) pushUnitPrompt((u), (p))
#endif

I18N_PLAY_FUNCTION(ru, pushUnitPrompt,  uint8_t unitprompt, int16_t number)
{
#if defined(CPUARM)
    if (number < 0){ // if negative number, we have to use 2 units form (for example value = 1.3)
       PUSH_UNIT_PROMPT(unitprompt, 2);
    }
    else{
      int16_t mod10 = number % 10;
      if (number == 0)
        PUSH_UNIT_PROMPT(unitprompt, 0);
      else if (number == 1)
        PUSH_UNIT_PROMPT(unitprompt, 1);
      else if (number>=2 && number <=4)
        PUSH_UNIT_PROMPT(unitprompt, 2);
      else if (number>=5 && number <=20)
        PUSH_UNIT_PROMPT(unitprompt, 5);
      else if (mod10 == 1)
        PUSH_UNIT_PROMPT(unitprompt, 1);
      else if (mod10 >= 2 && mod10 <=4)
        PUSH_UNIT_PROMPT(unitprompt, 2);
      else
        PUSH_UNIT_PROMPT(unitprompt, 5);
    }
#else

   // TODO: add rules for Russian language
  unitprompt = RU_PROMPT_UNITS_BASE + unitprompt*4;
  if (number == 1)
    PUSH_NUMBER_PROMPT(unitprompt);
  else
    PUSH_NUMBER_PROMPT(unitprompt+1);
#endif
}

I18N_PLAY_FUNCTION(ru, playNumber, getvalue_t number, uint8_t unit, uint8_t att)
{
  if (number < 0) {
    PUSH_NUMBER_PROMPT(RU_PROMPT_MINUS);
    number = -number;
  }

#if !defined(CPUARM)
  if (unit) {
    unit--;
    convertUnit(number, unit);
    if (IS_IMPERIAL_ENABLE()) {
      if (unit == UNIT_DIST) {
        unit = UNIT_FEET;
      }
      if (unit == UNIT_SPEED) {
    	unit = UNIT_KTS;
      }
    }
    unit++;
  }
#endif
  div_t qr = div((int)number, 10);
  int8_t mode = MODE(att);
  if (mode > 0 && att != RU_FEMALE_UNIT) {
#if defined(CPUARM)
    if (mode == 2) {
      number /= 10;
    }
#else
    // we assume that we are PREC1
#endif
    if (qr.rem) {
      PLAY_NUMBER(qr.quot, 0, 0);
      PUSH_NUMBER_PROMPT(RU_PROMPT_POINT_BASE + qr.rem);
      number = -1;
    }
    else {
      number = qr.quot;
    }
  }

  int16_t tmp = number;

  if (number >= 1000) {
    PLAY_NUMBER(number / 1000, RU_FEMALE_UNIT, 0); // female
    uint8_t thousands = number / 1000;
    int16_t mod10 = thousands % 10;
    if (thousands == 1)
      PUSH_NUMBER_PROMPT(RU_PROMPT_THOUSAND1);
    else if (thousands>=2 && thousands <=4)
      PUSH_NUMBER_PROMPT(RU_PROMPT_THOUSAND2);
    else if (thousands>=5 && thousands <=20)
      PUSH_NUMBER_PROMPT(RU_PROMPT_THOUSAND5);
    else if (mod10 == 1)
      PUSH_NUMBER_PROMPT(RU_PROMPT_THOUSAND1);
    else if (mod10 >= 2 && mod10 <=4)
      PUSH_NUMBER_PROMPT(RU_PROMPT_THOUSAND2);
    else
      PUSH_NUMBER_PROMPT(RU_PROMPT_THOUSAND5);
    number %= 1000;
    if (number == 0)
      number = -1;
  }
  if (number >= 100) {
    PUSH_NUMBER_PROMPT(RU_PROMPT_HUNDRED + (number/100)-1);
    number %= 100;
    if (number == 0)
      number = -1;
  }
  if (number >= 0) {
      uint8_t attr = MALE;
      switch(unit) {
        case RU_FEMALE_UNIT:
        case UNIT_MPH:
        case UNIT_FLOZ:
        case UNIT_MINUTES:
        case UNIT_SECONDS:
          attr = FEMALE;
          break;
        default:
          attr = MALE;
          break;
      }
      uint8_t lastDigit = number % 10;
      uint8_t ten=(number - (number % 10))/10;
      if (lastDigit == 1 && number != 11 && attr == FEMALE)
        PUSH_NUMBER_PROMPT(RU_PROMPT_FEMALE_ONE + ten);
      else if (lastDigit == 2 && number !=12 && attr == FEMALE)
        PUSH_NUMBER_PROMPT(RU_PROMPT_FEMALE_TWO + ten);
      else
        PUSH_NUMBER_PROMPT(RU_PROMPT_ZERO + number);
  }

  if (unit) {
    if (mode > 0 && qr.rem) // number with decimal point
      RU_PUSH_UNIT_PROMPT(unit, -1); // force 2 units form, if float value
    else
      RU_PUSH_UNIT_PROMPT(unit, tmp);
  }
}

I18N_PLAY_FUNCTION(ru, playDuration, int seconds PLAY_DURATION_ATT)
{
  if (seconds == 0) {
    PLAY_NUMBER(seconds, 0, 0);
    return;
  }

  if (seconds < 0) {
    PUSH_NUMBER_PROMPT(RU_PROMPT_MINUS);
    seconds = -seconds;
  }

  uint8_t tmp = seconds / 3600;
  seconds %= 3600;
  if (tmp > 0 || IS_PLAY_TIME()) {
    PLAY_NUMBER(tmp, UNIT_HOURS, 0);
  }

  tmp = seconds / 60;
  seconds %= 60;
  if (tmp > 0) {
    PLAY_NUMBER(tmp, UNIT_MINUTES, 0);
    if (seconds > 0)
      PUSH_NUMBER_PROMPT(RU_PROMPT_AND);
  }
  if (seconds > 0) {
    PLAY_NUMBER(seconds, UNIT_SECONDS, 0);
  }
}

LANGUAGE_PACK_DECLARE(ru, "Russian");

#endif
