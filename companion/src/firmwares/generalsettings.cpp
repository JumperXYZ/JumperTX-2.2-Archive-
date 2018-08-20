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

#include "generalsettings.h"

#include "appdata.h"
#include "eeprominterface.h"
#include "radiodataconversionstate.h"

const uint8_t chout_ar[] = { // First number is 0..23 -> template setup,  Second is relevant channel out
  1,2,3,4 , 1,2,4,3 , 1,3,2,4 , 1,3,4,2 , 1,4,2,3 , 1,4,3,2,
  2,1,3,4 , 2,1,4,3 , 2,3,1,4 , 2,3,4,1 , 2,4,1,3 , 2,4,3,1,
  3,1,2,4 , 3,1,4,2 , 3,2,1,4 , 3,2,4,1 , 3,4,1,2 , 3,4,2,1,
  4,1,2,3 , 4,1,3,2 , 4,2,1,3 , 4,2,3,1 , 4,3,1,2 , 4,3,2,1
};

bool GeneralSettings::switchPositionAllowedTaranis(int index) const
{
  if (index == 0)
    return true;

  div_t qr = div(abs(index)-1, 3);

  if (index < 0 && switchConfig[qr.quot] != Board::SWITCH_3POS)
    return false;
  else if (qr.rem == 1)
    return switchConfig[qr.quot] == Board::SWITCH_3POS;
  else
    return switchConfig[qr.quot] != Board::SWITCH_NOT_AVAILABLE;
}

bool GeneralSettings::switchSourceAllowedTaranis(int index) const
{
  return switchConfig[index] != Board::SWITCH_NOT_AVAILABLE;
}

bool GeneralSettings::isPotAvailable(int index) const
{
  if (index<0 || index>getBoardCapability(getCurrentBoard(), Board::Pots)) return false;
  return potConfig[index] != Board::POT_NONE;
}

bool GeneralSettings::isSliderAvailable(int index) const
{
  if (index<0 || index>getBoardCapability(getCurrentBoard(), Board::Sliders)) return false;
  return sliderConfig[index] != Board::SLIDER_NONE;
}

GeneralSettings::GeneralSettings()
{
  memset(this, 0, sizeof(GeneralSettings));

  contrast  = 25;

  for (int i=0; i < CPN_MAX_ANALOGS; ++i) {
    calibMid[i]     = 0x200;
    calibSpanNeg[i] = 0x180;
    calibSpanPos[i] = 0x180;
  }

  Firmware * firmware = Firmware::getCurrentVariant();
  Board::Type board = firmware->getBoard();

  // vBatWarn is voltage in 100mV, vBatMin is in 100mV but with -9V offset, vBatMax has a -12V offset
  vBatWarn  = 90;
  if (IS_TARANIS_X9E(board) || IS_HORUS_X12S(board)) {
    // NI-MH 9.6V
    vBatWarn = 87;
    vBatMin = -5;   //8,5V
    vBatMax = -5;   //11,5V
  }
  else if (IS_TARANIS_XLITE(board) || IS_HORUS_X10(board)) {
    // Lipo 2S
    vBatWarn = 66;
    vBatMin = -23;  // 6.7V
    vBatMax = -37;  // 8.3V 
  }
  else if (IS_TARANIS(board)) {
    // NI-MH 7.2V, X9D, X9D+ and X7
    vBatWarn = 65;
    vBatMin = -30; //6V
    vBatMax = -40; //8V
  }

  setDefaultControlTypes(board);

  backlightMode = 3; // keys and sticks
  // backlightBright = 0; // 0 = 100%

  if (IS_HORUS(board)) {
    backlightOffBright = 20;
  }

  if (IS_ARM(board)) {
    speakerVolume = 12;
  }

  if (IS_HORUS(board)) {
    strcpy(bluetoothName, "Horus");
  }
  else if (IS_TARANIS_X9E(board) || IS_TARANIS_SMALL(board)) {
    strcpy(bluetoothName, "Taranis");
  }

  templateSetup = g.profile[g.sessionId()].channelOrder();
  stickMode = g.profile[g.sessionId()].defaultMode();

  QString t_calib = g.profile[g.sessionId()].stickPotCalib();
  int potsnum = getBoardCapability(getCurrentBoard(), Board::Pots);
  if (!t_calib.isEmpty()) {
    QString t_trainercalib=g.profile[g.sessionId()].trainerCalib();
    int8_t t_txVoltageCalibration=(int8_t)g.profile[g.sessionId()].txVoltageCalibration();
    int8_t t_txCurrentCalibration=(int8_t)g.profile[g.sessionId()].txCurrentCalibration();
    int8_t t_PPM_Multiplier=(int8_t)g.profile[g.sessionId()].ppmMultiplier();
    uint8_t t_stickMode=(uint8_t)g.profile[g.sessionId()].gsStickMode();
    uint8_t t_vBatWarn=(uint8_t)g.profile[g.sessionId()].vBatWarn();
    QString t_DisplaySet=g.profile[g.sessionId()].display();
    QString t_BeeperSet=g.profile[g.sessionId()].beeper();
    QString t_HapticSet=g.profile[g.sessionId()].haptic();
    QString t_SpeakerSet=g.profile[g.sessionId()].speaker();
    QString t_CountrySet=g.profile[g.sessionId()].countryCode();

    if ((t_calib.length()==(CPN_MAX_STICKS+potsnum)*12) && (t_trainercalib.length()==16)) {
      QString Byte;
      int16_t byte16;
      bool ok;
      for (int i=0; i<(CPN_MAX_STICKS+potsnum); i++) {
        Byte=t_calib.mid(i*12,4);
        byte16=(int16_t)Byte.toInt(&ok,16);
        if (ok)
          calibMid[i]=byte16;
        Byte=t_calib.mid(4+i*12,4);
        byte16=(int16_t)Byte.toInt(&ok,16);
        if (ok)
          calibSpanNeg[i]=byte16;
        Byte=t_calib.mid(8+i*12,4);
        byte16=(int16_t)Byte.toInt(&ok,16);
        if (ok)
          calibSpanPos[i]=byte16;
      }
      for (int i=0; i<4; i++) {
        Byte=t_trainercalib.mid(i*4,4);
        byte16=(int16_t)Byte.toInt(&ok,16);
        if (ok)
          trainer.calib[i]=byte16;
      }
      txCurrentCalibration=t_txCurrentCalibration;
      txVoltageCalibration=t_txVoltageCalibration;
      vBatWarn=t_vBatWarn;
      PPM_Multiplier=t_PPM_Multiplier;
      stickMode = t_stickMode;
    }
    if ((t_DisplaySet.length()==6) && (t_BeeperSet.length()==4) && (t_HapticSet.length()==6) && (t_SpeakerSet.length()==6)) {
      uint8_t byte8u;
      int8_t byte8;
      bool ok;
      byte8=(int8_t)t_DisplaySet.mid(0,2).toInt(&ok,16);
      if (ok)
        optrexDisplay=(byte8==1 ? true : false);
      byte8u=(uint8_t)t_DisplaySet.mid(2,2).toUInt(&ok,16);
      if (ok)
        contrast=byte8u;
      byte8u=(uint8_t)t_DisplaySet.mid(4,2).toUInt(&ok,16);
      if (ok)
        backlightBright=byte8u;
      byte8=(int8_t)t_BeeperSet.mid(0,2).toUInt(&ok,16);
      if (ok)
        beeperMode=(BeeperMode)byte8;
      byte8=(int8_t)t_BeeperSet.mid(2,2).toInt(&ok,16);
      if (ok)
        beeperLength=byte8;
      byte8=(int8_t)t_HapticSet.mid(0,2).toUInt(&ok,16);
      if (ok)
        hapticMode=(BeeperMode)byte8;
      byte8=(int8_t)t_HapticSet.mid(2,2).toInt(&ok,16);
      if (ok)
        hapticStrength=byte8;
      byte8=(int8_t)t_HapticSet.mid(4,2).toInt(&ok,16);
      if (ok)
        hapticLength=byte8;
      byte8u=(uint8_t)t_SpeakerSet.mid(0,2).toUInt(&ok,16);
      if (ok)
        speakerMode=byte8u;
      byte8u=(uint8_t)t_SpeakerSet.mid(2,2).toUInt(&ok,16);
      if (ok)
        speakerPitch=byte8u;
      byte8u=(uint8_t)t_SpeakerSet.mid(4,2).toUInt(&ok,16);
      if (ok)
        speakerVolume=byte8u;
      if (t_CountrySet.length()==6) {
        byte8u=(uint8_t)t_CountrySet.mid(0,2).toUInt(&ok,16);
        if (ok)
          countryCode=byte8u;
        byte8u=(uint8_t)t_CountrySet.mid(2,2).toUInt(&ok,16);
        if (ok)
          imperial=byte8u;
        QString chars = t_CountrySet.mid(4, 2);
        ttsLanguage[0] = chars[0].toLatin1();
        ttsLanguage[1] = chars[1].toLatin1();
      }
    }
  }

  strcpy(themeName, "default");
  ThemeOptionData option1 = { 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0 };
  memcpy(&themeOptionValue[0], option1, sizeof(ThemeOptionData));
  ThemeOptionData option2 = { 0x03, 0xe1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0 };
  memcpy(&themeOptionValue[1], option2, sizeof(ThemeOptionData));
}

void GeneralSettings::setDefaultControlTypes(Board::Type board)
{
  for (int i=0; i<getBoardCapability(board, Board::FactoryInstalledSwitches); i++) {
    switchConfig[i] = Boards::getSwitchInfo(board, i).config;
  }

  // TODO: move to Boards, like with switches
  if (IS_HORUS(board)) {
    potConfig[0] = Board::POT_WITH_DETENT;
    potConfig[1] = Board::POT_MULTIPOS_SWITCH;
    potConfig[2] = Board::POT_WITH_DETENT;
  }
  else if (IS_TARANIS_XLITE(board)) {
    potConfig[0] = Board::POT_WITHOUT_DETENT;
    potConfig[1] = Board::POT_WITHOUT_DETENT;
  }
  else if (IS_TARANIS_X7(board)) {
    potConfig[0] = Board::POT_WITHOUT_DETENT;
    potConfig[1] = Board::POT_WITH_DETENT;
  }
  else if (IS_TARANIS(board)) {
    potConfig[0] = Board::POT_WITH_DETENT;
    potConfig[1] = Board::POT_WITH_DETENT;
  }
  else {
    potConfig[0] = Board::POT_WITHOUT_DETENT;
    potConfig[1] = Board::POT_WITHOUT_DETENT;
    potConfig[2] = Board::POT_WITHOUT_DETENT;
  }

  if (IS_HORUS_X12S(board) || IS_TARANIS_X9E(board)) {
    sliderConfig[0] = Board::SLIDER_WITH_DETENT;
    sliderConfig[1] = Board::SLIDER_WITH_DETENT;
    sliderConfig[2] = Board::SLIDER_WITH_DETENT;
    sliderConfig[3] = Board::SLIDER_WITH_DETENT;
  }
  else if (IS_TARANIS_X9(board) || IS_HORUS_X10(board)) {
    sliderConfig[0] = Board::SLIDER_WITH_DETENT;
    sliderConfig[1] = Board::SLIDER_WITH_DETENT;
  }
}

int GeneralSettings::getDefaultStick(unsigned int channel) const
{
  if (channel >= CPN_MAX_STICKS)
    return -1;
  else
    return chout_ar[4*templateSetup + channel] - 1;
}

RawSource GeneralSettings::getDefaultSource(unsigned int channel) const
{
  int stick = getDefaultStick(channel);
  if (stick >= 0)
    return RawSource(SOURCE_TYPE_STICK, stick);
  else
    return RawSource(SOURCE_TYPE_NONE);
}

int GeneralSettings::getDefaultChannel(unsigned int stick) const
{
  for (int i=0; i<4; i++){
    if (getDefaultStick(i) == (int)stick)
      return i;
  }
  return -1;
}

void GeneralSettings::convert(RadioDataConversionState & cstate)
{
  // Here we can add explicit conversions when moving from one board to another

  cstate.setOrigin(tr("Radio Settings"));

  setDefaultControlTypes(cstate.toType);  // start with default switches/pots/sliders

  // Try to intelligently copy any custom control names

  // SE and SG are skipped on X7 board
  if (IS_TARANIS_X7(cstate.toType)) {
    if (IS_TARANIS_X9(cstate.fromType) || IS_HORUS(cstate.fromType)) {
      strncpy(switchName[4], switchName[5], sizeof(switchName[0]));
      strncpy(switchName[5], switchName[7], sizeof(switchName[0]));
    }
  }
  else if (IS_TARANIS_X7(cstate.fromType)) {
    if (IS_TARANIS_X9(cstate.toType) || IS_HORUS(cstate.toType)) {
      strncpy(switchName[5], switchName[4], sizeof(switchName[0]));
      strncpy(switchName[7], switchName[5], sizeof(switchName[0]));
    }
  }

  // LS and RS sliders are after 2 aux sliders on X12 and X9E
  if ((IS_HORUS_X12S(cstate.toType) || IS_TARANIS_X9E(cstate.toType)) && !IS_HORUS_X12S(cstate.fromType) && !IS_TARANIS_X9E(cstate.fromType)) {
    strncpy(sliderName[0], sliderName[2], sizeof(sliderName[0]));
    strncpy(sliderName[1], sliderName[3], sizeof(sliderName[0]));
  }
  else if (!IS_TARANIS_X9E(cstate.toType) && !IS_HORUS_X12S(cstate.toType) && (IS_HORUS_X12S(cstate.fromType) || IS_TARANIS_X9E(cstate.fromType))) {
    strncpy(sliderName[2], sliderName[0], sizeof(sliderName[0]));
    strncpy(sliderName[3], sliderName[1], sizeof(sliderName[0]));
  }

  if (IS_HORUS(cstate.toType)) {
    // 6P switch is only on Horus (by default)
    if (cstate.fromBoard.getCapability(Board::FactoryInstalledPots) == 2) {
      strncpy(potName[2], potName[1], sizeof(potName[0]));
      potName[1][0] = '\0';
    }
  }

  if (IS_TARANIS(cstate.toType)) {
    // No S3 pot on Taranis boards by default
    if (cstate.fromBoard.getCapability(Board::FactoryInstalledPots) > 2)
      strncpy(potName[1], potName[2], sizeof(potName[0]));

    contrast = qBound<int>(getCurrentFirmware()->getCapability(MinContrast), contrast, getCurrentFirmware()->getCapability(MaxContrast));
  }

  // TODO: Would be nice at this point to have GUI pause and ask the user to set up any custom hardware they have on the destination radio.

  // Convert all global functions (do this after HW adjustments)
  for (int i=0; i<CPN_MAX_SPECIAL_FUNCTIONS; i++) {
    customFn[i].convert(cstate.withComponentIndex(i));
  }

}
