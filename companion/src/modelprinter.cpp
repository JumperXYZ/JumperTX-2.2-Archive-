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

#include "helpers.h"
#include "modelprinter.h"
#include "multiprotocols.h"
#include "boards.h"
#include "helpers_html.h"
#include "multiprotocols.h"

#include <QApplication>
#include <QPainter>
#include <QFile>
#include <QUrl>

QString changeColor(const QString & input, const QString & to, const QString & from)
{
  QString result = input;
  return result.replace("color="+from, "color="+to);
}

ModelPrinter::ModelPrinter(Firmware * firmware, const GeneralSettings & generalSettings, const ModelData & model):
  firmware(firmware),
  generalSettings(generalSettings),
  model(model)
{
}

ModelPrinter::~ModelPrinter()
{
}

QString formatTitle(const QString & name)
{
  return QString("<b>" + name + "</b>&nbsp;");
}

void debugHtml(const QString & html)
{
  QFile file("foo.html");
  file.open(QIODevice::Truncate | QIODevice::WriteOnly);
  file.write(html.toUtf8());
  file.close();
}

QString addFont(const QString & input, const QString & color, const QString & size, const QString & face)
{
  QString colorStr;
  if (!color.isEmpty()) {
    colorStr = "color=" + color;
  }
  QString sizeStr;
  if (!size.isEmpty()) {
    sizeStr = "size=" + size;
  }
  QString faceStr;
  if (!face.isEmpty()) {
    faceStr = "face='" + face + "'";
  }
  return "<font " + sizeStr + " " + faceStr + " " + colorStr + ">" + input + "</font>";
}

QString ModelPrinter::printLabelValue(const QString & lbl, const QString & val, const bool sep) {
  return QString("<b>%1:</b> %2%3 ").arg(lbl, val, (sep ? ";" : ""));
}

QString ModelPrinter::printLabelValues(const QString & lbl, const QStringList & vals, const bool sep) {
  QString str;
  if (vals.count() > 1)
    str.append("(");
  for (int i=0;i<vals.count();i++) {
    str.append(vals.at(i));
    if (i<(vals.count()-1))
      str.append(", ");
  }
  if (vals.count() > 1)
    str.append(")");

  return printLabelValue(lbl, str, sep);
}

#define MASK_TIMEVALUE_HRSMINS 1
#define MASK_TIMEVALUE_ZEROHRS 2
#define MASK_TIMEVALUE_PADSIGN 3

QString ModelPrinter::printTimeValue(const int value, const unsigned int mask)
{
  QString result;
  int sign = 1;
  int val = value;
  if (val < 0) {
    val = -val;
    sign = -1;
  }
  result = (sign < 0 ? QString("-") : ((mask & MASK_TIMEVALUE_PADSIGN) ? QString(" ") : QString("")));
  if (mask & MASK_TIMEVALUE_HRSMINS) {
    int hours = val / 3600;
    if (hours > 0 || (mask & MASK_TIMEVALUE_ZEROHRS)) {
      val -= hours * 3600;
      result.append(QString("%1:").arg(hours, 2, 10, QLatin1Char('0')));
    }
  }
  int minutes = val / 60;
  int seconds = val % 60;
  result.append(QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0')));
  return result;
}

#define BOOLEAN_ENABLEDISABLE 1
#define BOOLEAN_TRUEFALSE 2
#define BOOLEAN_YESNO 3
#define BOOLEAN_YN 4
#define BOOLEAN_ONOFF 5

QString ModelPrinter::printBoolean(const bool val, const int typ)
{
  switch (typ) {
    case BOOLEAN_ENABLEDISABLE:
      return (val ? tr("Enable") : tr("Disable"));
    case BOOLEAN_TRUEFALSE:
      return (val ? tr("True") : tr("False"));
    case BOOLEAN_YESNO:
      return (val ? tr("Yes") : tr("No"));
    case BOOLEAN_YN:
      return (val ? tr("Y") : tr("N"));
    case BOOLEAN_ONOFF:
      return (val ? tr("ON") : tr("OFF"));
    default:
      return tr("???");
  }
}

QString ModelPrinter::printEEpromSize()
{
  return QString("%1 ").arg(getCurrentEEpromInterface()->getSize(model)) + tr("bytes");
}

QString ModelPrinter::printChannelName(int idx)
{
  QString str = RawSource(SOURCE_TYPE_CH, idx).toString(&model, &generalSettings);
  if (firmware->getCapability(ChannelsName)) {
    str = str.leftJustified(firmware->getCapability(ChannelsName) + 5, ' ', false);
  }
  str.append(' ');
  return str.toHtmlEscaped();
}

QString ModelPrinter::printTrimIncrementMode()
{
  switch (model.trimInc) {
    case -2:
      return tr("Exponential");
    case -1:
      return tr("Extra Fine");
    case 0:
      return tr("Fine");
    case 1:
      return tr("Medium");
    case 2:
      return tr("Coarse");
    default:
      return tr("Unknown");
  }
}

QString ModelPrinter::printModuleProtocol(unsigned int protocol)
{
  static const char * strings[] = {
    "OFF",
    "PPM",
    "Silverlit A", "Silverlit B", "Silverlit C",
    "CTP1009",
    "LP45", "DSM2", "DSMX",
    "PPM16", "PPMsim",
    "FrSky XJT (D16)", "FrSky XJT (D8)", "FrSky XJT (LR12)", "FrSky DJT",
    "Crossfire",
    "DIY Multiprotocol Module",
    "FrSky R9M Module",
    "SBUS output at VBat"
  };

  return CHECK_IN_ARRAY(strings, protocol);
}

QString ModelPrinter::printMultiRfProtocol(int rfProtocol, bool custom)
{
  static const char * strings[] = {
    "FlySky", "Hubsan", "FrSky", "Hisky", "V2x2", "DSM", "Devo", "YD717", "KN", "SymaX", "SLT", "CX10", "CG023",
    "Bayang", "ESky", "MT99XX", "MJXQ", "Shenqi", "FY326", "SFHSS", "J6 PRO","FQ777","Assan","Hontai","OLRS",
    "FlySky AFHDS2A", "Q2x2", "Walkera", "Q303", "GW008", "DM002", "CABELL", "Esky 150", "H8 3D", "Corona", "CFlie"
  };
  if (custom)
    return QObject::tr("Custom - proto %1)").arg(QString::number(rfProtocol));
  else
    return CHECK_IN_ARRAY(strings, rfProtocol);
}

QString ModelPrinter::printMultiSubType(unsigned rfProtocol, bool custom, unsigned int subType) {
  /* custom protocols */

  if (custom)
    rfProtocol = MM_RF_CUSTOM_SELECTED;

  Multiprotocols::MultiProtocolDefinition pdef = multiProtocols.getProtocol(rfProtocol);

  if (subType < (unsigned int) pdef.subTypeStrings.size())
    return qApp->translate("Multiprotocols", qPrintable(pdef.subTypeStrings[subType]));
  else
    return "???";
}

QString ModelPrinter::printR9MPowerValue(unsigned subType, unsigned val, bool telem)
{
  QStringList strFCC;
  QStringList strLBT;

  if (IS_TARANIS_XLITE(firmware->getBoard())) {
    strFCC = QStringList() << tr("100mW - 16CH");
    strLBT = QStringList() << tr("25mW - 8CH") << tr("25mW - 16CH") << tr("100mW 16CH");
  }
  else {
    strFCC = QStringList() << tr("10mW") << tr("100mW") << tr("500mW") << tr("1W");
    strLBT = QStringList() << tr("25mW - 8CH") << tr("25mW - 16CH") << tr("200mW 16CH") << tr("500mW 16CH");
  }


  if (subType == R9M_FCC && (int)val < strFCC.size())
    return strFCC.at(val);
  else if (subType == R9M_LBT)
    return (telem ? strLBT.at(0) : strLBT.at(1));
  else
    return "???";
}

QString ModelPrinter::printModuleSubType(unsigned protocol, unsigned subType, unsigned rfProtocol, bool custom)
{
  static const char * strings[] = {
    "FCC",
    "LBT(EU)"
  };

  switch (protocol) {
    case PULSES_MULTIMODULE:
      return printMultiSubType(rfProtocol, custom, subType);

    case PULSES_PXX_R9M:
      return CHECK_IN_ARRAY(strings, subType);

    default:
      return "???";
  }
}

QString ModelPrinter::printModule(int idx)
{
  QStringList str;
  QString result;
  ModuleData module = model.moduleData[(idx<0 ? CPN_MAX_MODULES : idx)];
  if (idx < 0) {
    str << printLabelValue(tr("Mode"), printTrainerMode());
    if (IS_HORUS_OR_TARANIS(firmware->getBoard())) {
      if (model.trainerMode == TRAINER_SLAVE_JACK) {
        str << printLabelValue(tr("Channels"), QString("%1-%2").arg(module.channelsStart + 1).arg(module.channelsStart + module.channelsCount));
        str << printLabelValue(tr("Frame length"), QString("%1ms").arg(printPPMFrameLength(module.ppm.frameLength)));
        str << printLabelValue(tr("PPM delay"), QString("%1us").arg(module.ppm.delay));
        str << printLabelValue(tr("Polarity"), module.polarityToString());
      }
    }
    result = str.join(" ");
  }
  else {
    str << printLabelValue(tr("Protocol"), printModuleProtocol(module.protocol));
    if (module.protocol) {
      str << printLabelValue(tr("Channels"), QString("%1-%2").arg(module.channelsStart + 1).arg(module.channelsStart + module.channelsCount));
      if (module.protocol == PULSES_PPM || module.protocol == PULSES_SBUS) {
        str << printLabelValue(tr("Frame length"), QString("%1ms").arg(printPPMFrameLength(module.ppm.frameLength)));
        str << printLabelValue(tr("Polarity"), module.polarityToString());
        if (module.protocol == PULSES_PPM)
          str << printLabelValue(tr("Delay"), QString("%1us").arg(module.ppm.delay));
      }
      else {
        if (!(module.protocol == PULSES_PXX_XJT_D8 || module.protocol == PULSES_CROSSFIRE || module.protocol == PULSES_SBUS)) {
          str << printLabelValue(tr("Receiver"), QString::number(module.modelId));
        }
        if (module.protocol == PULSES_MULTIMODULE) {
          str << printLabelValue(tr("Radio protocol"), printMultiRfProtocol(module.multi.rfProtocol, module.multi.customProto));
          str << printLabelValue(tr("Subtype"), printMultiSubType(module.multi.rfProtocol, module.multi.customProto, module.subType));
          str << printLabelValue(tr("Option value"), QString::number(module.multi.optionValue));
        }
        if (module.protocol == PULSES_PXX_R9M) {
          str << printLabelValue(tr("Sub Type"), printModuleSubType(module.protocol, module.subType));
          str << printLabelValue(tr("RF Output Power"), printR9MPowerValue(module.subType, module.pxx.power, module.pxx.sport_out));
          str << printLabelValue(tr("Telemetry"), printBoolean(module.pxx.sport_out, BOOLEAN_ENABLEDISABLE));
        }
      }
    }
    result = str.join(" ");
    if (((PulsesProtocol)module.protocol == PulsesProtocol::PULSES_PXX_XJT_X16 || (PulsesProtocol)module.protocol == PulsesProtocol::PULSES_PXX_R9M)
       && firmware->getCapability(HasFailsafe))
      result.append("<br/>" + printFailsafe(idx));
  }
  return result;
}

QString ModelPrinter::printTrainerMode()
{
  QString result;
  switch (model.trainerMode) {
    case TRAINER_MASTER_JACK:
      result = tr("Master/Jack");
      break;
    case TRAINER_SLAVE_JACK:
      result = tr("Slave/Jack");
      break;
    case TRAINER_MASTER_SBUS_MODULE:
      result = tr("Master/SBUS Module");
      break;
    case TRAINER_MASTER_CPPM_MODULE:
      result = tr("Master/CPPM Module");
      break;
    case TRAINER_MASTER_SBUS_BATT_COMPARTMENT:
      result = tr("Master/SBUS in battery compartment");
      break;
    default:
      result = tr("????");
  }
  return result;
}

QString ModelPrinter::printHeliSwashType ()
{
  switch (model.swashRingData.type) {
    case HELI_SWASH_TYPE_90:
        return tr("90");
      case HELI_SWASH_TYPE_120:
        return tr("120");
      case HELI_SWASH_TYPE_120X:
        return tr("120X");
      case HELI_SWASH_TYPE_140:
        return tr("140");
      case HELI_SWASH_TYPE_NONE:
        return tr("Off");
      default:
        return "???";
    }
}

QString ModelPrinter::printCenterBeep()
{
  QStringList strl;
  if (model.beepANACenter & 0x01)
    strl << tr("Rudder");
  if (model.beepANACenter & 0x02)
    strl << tr("Elevator");
  if (model.beepANACenter & 0x04)
    strl << tr("Throttle");
  if (model.beepANACenter & 0x08)
    strl << tr("Aileron");
  if (IS_HORUS(firmware->getBoard())) {
    // TODO
    qDebug() << "ModelPrinter::printCenterBeep() TODO";
  }
  else if (IS_TARANIS(firmware->getBoard())) {
    if (model.beepANACenter & 0x10)
      strl << "S1";
    if (model.beepANACenter & 0x20)
      strl << "S2";
    if (model.beepANACenter & 0x40)
      strl << "S3";
    if (model.beepANACenter & 0x80)
      strl << "LS";
    if (model.beepANACenter & 0x100)
      strl << "RS";
  }
  else {
    if (model.beepANACenter & 0x10)
      strl << "P1";
    if (model.beepANACenter & 0x20)
      strl << "P2";
    if (model.beepANACenter & 0x40)
      strl << "P3";
  }
  return (strl.isEmpty() ? tr("None") : strl.join(" "));
}

QString ModelPrinter::printTimer(int idx)
{
  return printTimer(model.timers[idx]);
}

QString ModelPrinter::printTimer(const TimerData & timer)
{
  QStringList result;
  if (firmware->getCapability(TimersName) && timer.name[0])
    result += tr("Name") + QString("(%1)").arg(timer.name);
  result += printTimeValue(timer.val, MASK_TIMEVALUE_HRSMINS | MASK_TIMEVALUE_ZEROHRS);
  result += timer.mode.toString();
  if (timer.countdownBeep)
    result += tr("Countdown") + QString("(%1)").arg(printTimerCountdownBeep(timer.countdownBeep));
  if (timer.minuteBeep)
    result += tr("Minute call");
  if (timer.persistent)
    result += tr("Persistent") + QString("(%1)").arg(printTimerPersistent(timer.persistent));
  return result.join(", ");
}

QString ModelPrinter::printTrim(int flightModeIndex, int stickIndex)
{
  const FlightModeData & fm = model.flightModeData[flightModeIndex];

  if (fm.trimMode[stickIndex] == -1) {
    return tr("OFF");
  }
  else {
    if (fm.trimRef[stickIndex] == flightModeIndex) {
      return QString("%1").arg(fm.trim[stickIndex]);
    }
    else {
      if (fm.trimMode[stickIndex] == 0) {
        return tr("FM%1").arg(fm.trimRef[stickIndex]);
      }
      else {
        if (fm.trim[stickIndex] < 0)
          return tr("FM%1%2").arg(fm.trimRef[stickIndex]).arg(fm.trim[stickIndex]);
        else
          return tr("FM%1+%2").arg(fm.trimRef[stickIndex]).arg(fm.trim[stickIndex]);
      }
    }
  }
}

QString ModelPrinter::printGlobalVar(int flightModeIndex, int gvarIndex)
{
  const FlightModeData & fm = model.flightModeData[flightModeIndex];

  if (fm.gvars[gvarIndex] <= 1024) {
    return QString("%1").arg(fm.gvars[gvarIndex] * model.gvarData[gvarIndex].multiplierGet());
  }
  else {
    int num = fm.gvars[gvarIndex] - 1025;
    if (num >= flightModeIndex) num++;
    return tr("FM%1").arg(num);
  }
}

QString ModelPrinter::printRotaryEncoder(int flightModeIndex, int reIndex)
{
  const FlightModeData & fm = model.flightModeData[flightModeIndex];

  if (fm.rotaryEncoders[reIndex] <= 1024) {
    return QString("%1").arg(fm.rotaryEncoders[reIndex]);
  }
  else {
    int num = fm.rotaryEncoders[reIndex] - 1025;
    if (num >= flightModeIndex) num++;
    return tr("FM%1").arg(num);
  }
}

QString ModelPrinter::printInputName(int idx)
{
  RawSourceType srcType = (firmware->getCapability(VirtualInputs) ? SOURCE_TYPE_VIRTUAL_INPUT : SOURCE_TYPE_STICK);
  return RawSource(srcType, idx).toString(&model, &generalSettings).toHtmlEscaped();
}

QString ModelPrinter::printInputLine(int idx)
{
  return printInputLine(model.expoData[idx]);
}

QString ModelPrinter::printInputLine(const ExpoData & input)
{
  QString str = "&nbsp;";

  switch (input.mode) {
    case (1): str += "&lt;-&nbsp;"; break;
    case (2): str += "-&gt;&nbsp;"; break;
    default:  str += "&nbsp;&nbsp;&nbsp;"; break;
  }

  if (firmware->getCapability(VirtualInputs)) {
    str += input.srcRaw.toString(&model, &generalSettings).toHtmlEscaped();
  }

  str += " " + tr("Weight").toHtmlEscaped() + QString("(%1)").arg(Helpers::getAdjustmentString(input.weight, &model, true).toHtmlEscaped());
  if (input.curve.value)
    str += " " + input.curve.toString(&model).toHtmlEscaped();

  QString flightModesStr = printFlightModes(input.flightModes);
  if (!flightModesStr.isEmpty())
    str += " " + flightModesStr.toHtmlEscaped();

  if (input.swtch.type != SWITCH_TYPE_NONE)
    str += " " + tr("Switch").toHtmlEscaped() + QString("(%1)").arg(input.swtch.toString(getCurrentBoard(), &generalSettings)).toHtmlEscaped();


  if (firmware->getCapability(VirtualInputs)) {
    if (input.carryTrim>0)
      str += " " + tr("NoTrim").toHtmlEscaped();
    else if (input.carryTrim<0)
      str += " " + RawSource(SOURCE_TYPE_TRIM, (-(input.carryTrim)-1)).toString(&model, &generalSettings).toHtmlEscaped();
  }

  if (input.offset)
    str += " " + tr("Offset(%1)").arg(Helpers::getAdjustmentString(input.offset, &model)).toHtmlEscaped();

  if (firmware->getCapability(HasExpoNames) && input.name[0])
    str += QString(" [%1]").arg(input.name).toHtmlEscaped();

  return str;
}

QString ModelPrinter::printMixerLine(const MixData & mix, bool showMultiplex, int highlightedSource)
{
  QString str = "&nbsp;";

  if (showMultiplex) {
    switch(mix.mltpx) {
      case (1): str += "*="; break;
      case (2): str += ":="; break;
      default:  str += "+="; break;
    }
  }
  else {
    str += "&nbsp;&nbsp;";
  }
  // highlight source if needed
  QString source = mix.srcRaw.toString(&model, &generalSettings).toHtmlEscaped();
  if ( (mix.srcRaw.type == SOURCE_TYPE_CH) && (mix.srcRaw.index+1 == (int)highlightedSource) ) {
    source = "<b>" + source + "</b>";
  }
  str += "&nbsp;" + source;

  if (mix.mltpx == MLTPX_MUL && !showMultiplex)
    str += " " + tr("MULT!").toHtmlEscaped();
  else
    str += " " + tr("Weight") + QString("(%1)").arg(Helpers::getAdjustmentString(mix.weight, &model, true)).toHtmlEscaped();

  QString flightModesStr = printFlightModes(mix.flightModes);
  if (!flightModesStr.isEmpty())
    str += " " + flightModesStr.toHtmlEscaped();

  if (mix.swtch.type != SWITCH_TYPE_NONE)
    str += " " + tr("Switch") + QString("(%1)").arg(mix.swtch.toString(getCurrentBoard(), &generalSettings)).toHtmlEscaped();

  if (mix.carryTrim > 0)
    str += " " + tr("NoTrim").toHtmlEscaped();
  else if (mix.carryTrim < 0)
    str += " " + RawSource(SOURCE_TYPE_TRIM, (-(mix.carryTrim)-1)).toString(&model, &generalSettings);

  if (firmware->getCapability(HasNoExpo) && mix.noExpo)
    str += " " + tr("No DR/Expo").toHtmlEscaped();
  if (mix.sOffset)
    str += " " + tr("Offset") + QString("(%1)").arg(Helpers::getAdjustmentString(mix.sOffset, &model)).toHtmlEscaped();
  if (mix.curve.value)
    str += " " + mix.curve.toString(&model).toHtmlEscaped();
  int scale = firmware->getCapability(SlowScale);
  if (scale == 0)
    scale = 1;
  if (mix.delayDown || mix.delayUp)
    str += " " + tr("Delay") + QString("(u%1:d%2)").arg((double)mix.delayUp/scale).arg((double)mix.delayDown/scale).toHtmlEscaped();
  if (mix.speedDown || mix.speedUp)
    str += " " + tr("Slow") + QString("(u%1:d%2)").arg((double)mix.speedUp/scale).arg((double)mix.speedDown/scale).toHtmlEscaped();
  if (mix.mixWarn)
    str += " " + tr("Warn") + QString("(%1)").arg(mix.mixWarn).toHtmlEscaped();
  if (firmware->getCapability(HasMixerNames) && mix.name[0])
    str += QString(" [%1]").arg(mix.name).toHtmlEscaped();
  return str;
}

QString ModelPrinter::printFlightModeSwitch(const RawSwitch & swtch)
{
  return swtch.toString(getCurrentBoard(), &generalSettings);
}

QString ModelPrinter::printFlightModeName(int index)
{
  return model.flightModeData[index].nameToString(index);
}

QString ModelPrinter::printFlightModes(unsigned int flightModes)
{
  int numFlightModes = firmware->getCapability(FlightModes);
  if (numFlightModes && flightModes) {
    if (flightModes == (unsigned int)(1<<numFlightModes) - 1) {
      return tr("Disabled in all flight modes");
    }
    else {
      QStringList list;
      for (int i=0; i<numFlightModes; i++) {
        if (!(flightModes & (1<<i))) {
          list << printFlightModeName(i);
        }
      }
      return (list.size() > 1 ? tr("Flight modes") : tr("Flight mode")) + QString("(%1)").arg(list.join(", "));
    }
  }
  else
    return "";
}

QString ModelPrinter::printInputFlightModes(unsigned int flightModes)
{
  int numFlightModes = firmware->getCapability(FlightModes);
  if (numFlightModes && flightModes) {
    if (flightModes == (unsigned int)(1<<numFlightModes) - 1) {
      return tr("None");
    }
    else {
      QStringList list;
      for (int i=0; i<numFlightModes; i++) {
        if (!(flightModes & (1<<i))) {
          list << printFlightModeName(i);
        }
      }
      return QString("%1").arg(list.join(" "));
    }
  }
  else
    return tr("All");
}

QString ModelPrinter::printLogicalSwitchLine(int idx)
{
  QString result = "";
  const LogicalSwitchData & ls = model.logicalSw[idx];
  const QString sw1Name = RawSwitch(ls.val1).toString(getCurrentBoard(), &generalSettings);
  const QString sw2Name = RawSwitch(ls.val2).toString(getCurrentBoard(), &generalSettings);

  if (ls.isEmpty())
    return result;

  if (ls.andsw!=0) {
    result +="( ";
  }
  switch (ls.getFunctionFamily()) {
    case LS_FAMILY_EDGE:
      result += tr("Edge") + QString("(%1, [%2:%3])").arg(sw1Name).arg(ValToTim(ls.val2)).arg(ls.val3<0 ? tr("instant") : QString("%1").arg(ValToTim(ls.val2+ls.val3)));
      break;
    case LS_FAMILY_STICKY:
      result += tr("Sticky") + QString("(%1, %2)").arg(sw1Name).arg(sw2Name);
      break;
    case LS_FAMILY_TIMER:
      result += tr("Timer") + QString("(%1, %2)").arg(ValToTim(ls.val1)).arg(ValToTim(ls.val2));
      break;
    case LS_FAMILY_VOFS: {
      RawSource source = RawSource(ls.val1);
      RawSourceRange range = source.getRange(&model, generalSettings);
      QString res;
      if (ls.val1)
        res += source.toString(&model, &generalSettings);
      else
        res += "0";
      res.remove(" ");
      if (ls.func == LS_FN_APOS || ls.func == LS_FN_ANEG)
        res = "|" + res + "|";
      else if (ls.func == LS_FN_DAPOS)
        res = "|d(" + res + ")|";
      else if (ls.func == LS_FN_DPOS)
        result = "d(" + res + ")";
      result += res;
      if (ls.func == LS_FN_VEQUAL)
        result += " = ";
      else if (ls.func == LS_FN_APOS || ls.func == LS_FN_VPOS || ls.func == LS_FN_DPOS || ls.func == LS_FN_DAPOS)
        result += " &gt; ";
      else if (ls.func == LS_FN_ANEG || ls.func == LS_FN_VNEG)
        result += " &lt; ";
      else if (ls.func == LS_FN_VALMOSTEQUAL)
        result += " ~ ";
      else
        result += tr(" missing");
      result += QString::number(range.step * (ls.val2 /*TODO+ source.getRawOffset(model)*/) + range.offset);
      break;
    }
    case LS_FAMILY_VBOOL:
      result += sw1Name;
      switch (ls.func) {
        case LS_FN_AND:
          result += " AND ";
          break;
        case LS_FN_OR:
          result += " OR ";
          break;
        case LS_FN_XOR:
          result += " XOR ";
          break;
       default:
          result += " bar ";
          break;
      }
      result += sw2Name;
      break;

    case LS_FAMILY_VCOMP:
      if (ls.val1)
        result += RawSource(ls.val1).toString(&model, &generalSettings);
      else
        result += "0";
      switch (ls.func) {
        case LS_FN_EQUAL:
        case LS_FN_VEQUAL:
          result += " = ";
          break;
        case LS_FN_NEQUAL:
          result += " != ";
          break;
        case LS_FN_GREATER:
          result += " &gt; ";
          break;
        case LS_FN_LESS:
          result += " &lt; ";
          break;
        case LS_FN_EGREATER:
          result += " &gt;= ";
          break;
        case LS_FN_ELESS:
          result += " &lt;= ";
          break;
        default:
          result += " foo ";
          break;
      }
      if (ls.val2)
        result += RawSource(ls.val2).toString(&model, &generalSettings);
      else
        result += "0";
      break;
  }

  if (ls.andsw != 0) {
    result +=" ) AND ";
    result += RawSwitch(ls.andsw).toString(getCurrentBoard(), &generalSettings);
  }

  if (firmware->getCapability(LogicalSwitchesExt)) {
    if (ls.duration)
      result += " " + tr("Duration") + QString("(%1s)").arg(ls.duration/10.0);
    if (ls.delay)
      result += " " + tr("Delay") + QString("(%1s)").arg(ls.delay/10.0);
  }

  return result;
}

QString ModelPrinter::printCustomFunctionLine(int idx)
{
  QString result;
  const CustomFunctionData & cf = model.customFn[idx];
  if (cf.swtch.type == SWITCH_TYPE_NONE)
    return result;

  result += cf.swtch.toString(getCurrentBoard(), &generalSettings) + " - ";
  result += cf.funcToString(&model) + " (";
  result += cf.paramToString(&model) + ")";
  if (!cf.repeatToString().isEmpty())
    result += " " + cf.repeatToString();
  if (!cf.enabledToString().isEmpty())
    result += " " + cf.enabledToString();
  return result;
}

QString ModelPrinter::printCurveName(int idx)
{
  return model.curves[idx].nameToString(idx).toHtmlEscaped();
}

QString ModelPrinter::printCurve(int idx)
{
  QString result;
  const CurveData & curve = model.curves[idx];
  result += (curve.type == CurveData::CURVE_TYPE_CUSTOM) ? tr("Custom") : tr("Standard");
  result += ", [";
  if (curve.type == CurveData::CURVE_TYPE_CUSTOM) {
    for (int j=0; j<curve.count; j++) {
      if (j != 0)
        result += ", ";
      result += QString("(%1, %2)").arg(curve.points[j].x).arg(curve.points[j].y);
    }
  }
  else {
    for (int j=0; j<curve.count; j++) {
      if (j != 0)
        result += ", ";
      result += QString("%1").arg(curve.points[j].y);
    }
  }
  result += "]";
  return result;
}

CurveImage::CurveImage():
  size(200),
  image(size+1, size+1, QImage::Format_RGB32),
  painter(&image)
{
  painter.setBrush(QBrush("#FFFFFF"));
  painter.setPen(QColor(0, 0, 0));
  painter.drawRect(0, 0, size, size);

  painter.setPen(QColor(0, 0, 0));
  painter.drawLine(0, size/2, size, size/2);
  painter.drawLine(size/2, 0, size/2, size);
  for (int i=0; i<21; i++) {
    painter.drawLine(size/2-5, (size*i)/(20), size/2+5, (size*i)/(20));
    painter.drawLine((size*i)/(20), size/2-5, (size*i)/(20), size/2+5);
  }
}

void CurveImage::drawCurve(const CurveData & curve, QColor color)
{
  painter.setPen(QPen(color, 2, Qt::SolidLine));
  for (int j=1; j<curve.count; j++) {
    if (curve.type == CurveData::CURVE_TYPE_CUSTOM)
      painter.drawLine(size/2+(size*curve.points[j-1].x)/200, size/2-(size*curve.points[j-1].y)/200, size/2+(size*curve.points[j].x)/200, size/2-(size*curve.points[j].y)/200);
    else
      painter.drawLine(size*(j-1)/(curve.count-1), size/2-(size*curve.points[j-1].y)/200, size*(j)/(curve.count-1), size/2-(size*curve.points[j].y)/200);
  }
}

QString ModelPrinter::createCurveImage(int idx, QTextDocument * document)
{
  CurveImage image;
  image.drawCurve(model.curves[idx], colors[idx]);
  QString filename = QString("mydata://curve-%1-%2.png").arg((uint64_t)this).arg(idx);
  if (document)
    document->addResource(QTextDocument::ImageResource, QUrl(filename), image.get());
  // qDebug() << "ModelPrinter::createCurveImage()" << idx << filename;
  return filename;
}

QString ModelPrinter::printGlobalVarUnit(int idx)
{
  return model.gvarData[idx].unitToString().toHtmlEscaped();
}

QString ModelPrinter::printGlobalVarPrec(int idx)
{
  return model.gvarData[idx].precToString().toHtmlEscaped();
}

QString ModelPrinter::printGlobalVarMin(int idx)
{
  return QString::number(model.gvarData[idx].getMinPrec());
}

QString ModelPrinter::printGlobalVarMax(int idx)
{
  return QString::number(model.gvarData[idx].getMaxPrec());
}

QString ModelPrinter::printGlobalVarPopup(int idx)
{
  return printBoolean(model.gvarData[idx].popup, BOOLEAN_YN);
}

QString ModelPrinter::printOutputValueGVar(int val)
{
  QString result = "";
  if (abs(val) > 10000) {
    if (val < 0)
      result = "-";
    result.append(RawSource(SOURCE_TYPE_GVAR, abs(val)-10001).toString(&model));
  }
  else {
    if (val >= 0)
      result = "+";
    result.append(QString::number((qreal)val/10, 'f', 1) + "%");
  }
  return result;
}

QString ModelPrinter::printOutputOffset(int idx)
{
  return printOutputValueGVar(model.limitData[idx].offset);
}

QString ModelPrinter::printOutputMin(int idx)
{
  return printOutputValueGVar(model.limitData[idx].min);
}

QString ModelPrinter::printOutputMax(int idx)
{
  return printOutputValueGVar(model.limitData[idx].max);
}

QString ModelPrinter::printOutputRevert(int idx)
{
  return model.limitData[idx].revertToString();
}

QString ModelPrinter::printOutputPpmCenter(int idx)
{
  return QString::number(model.limitData[idx].ppmCenter + 1500);
}

QString ModelPrinter::printOutputCurve(int idx)
{
  return CurveReference(CurveReference::CURVE_REF_CUSTOM, model.limitData[idx].curve.value).toString(&model, false);
}

QString ModelPrinter::printOutputSymetrical(int idx)
{
  return printBoolean(model.limitData[idx].symetrical, BOOLEAN_YN);
}

QString ModelPrinter::printSettingsOther()
{
  QStringList str;
  str << printLabelValue(tr("Extended Limits"), printBoolean(model.extendedLimits, BOOLEAN_YESNO));
  if (firmware->getCapability(HasDisplayText))
    str << printLabelValue(tr("Display Checklist"), printBoolean(model.displayChecklist, BOOLEAN_YESNO));
  if (firmware->getCapability(GlobalFunctions))
    str << printLabelValue(tr("Global Functions"), printBoolean(!model.noGlobalFunctions, BOOLEAN_YESNO));
  return str.join(" ");
}

QString ModelPrinter::printSwitchWarnings()
{
  QStringList str;
  Boards board = firmware->getBoard();
  uint64_t switchStates = model.switchWarningStates;
  uint64_t value;

  for (int idx=0; idx<board.getCapability(Board::Switches); idx++) {
    Board::SwitchInfo switchInfo = Boards::getSwitchInfo(board.getBoardType(), idx);
    switchInfo.config = Board::SwitchType(generalSettings.switchConfig[idx]);
    if (switchInfo.config == Board::SWITCH_NOT_AVAILABLE || switchInfo.config == Board::SWITCH_TOGGLE) {
      continue;
    }
    if (!(model.switchWarningEnable & (1 << idx))) {
      if (IS_HORUS_OR_TARANIS(board.getBoardType())) {
        value = (switchStates >> (2*idx)) & 0x03;
      }
      else {
        value = (idx==0 ? switchStates & 0x3 : switchStates & 0x1);
        switchStates >>= (idx==0 ? 2 : 1);
      }
      str += RawSwitch(SWITCH_TYPE_SWITCH, 1+idx*3+value).toString(board.getBoardType(), &generalSettings, &model);
    }
  }
  return (str.isEmpty() ? tr("None") : str.join(" ")) ;
}

QString ModelPrinter::printPotWarnings()
{
  QStringList str;
  int genAryIdx = 0;
  Boards board = firmware->getBoard();
  if (model.potsWarningMode) {
    for (int i=0; i<board.getCapability(Board::Pots)+board.getCapability(Board::Sliders); i++) {
      RawSource src(SOURCE_TYPE_STICK, CPN_MAX_STICKS + i);
      if ((src.isPot(&genAryIdx) && generalSettings.isPotAvailable(genAryIdx)) || (src.isSlider(&genAryIdx) && generalSettings.isSliderAvailable(genAryIdx))) {
        if (!model.potsWarningEnabled[i])
          str += src.toString(&model, &generalSettings);
      }
    }
  }
  str << printLabelValue(tr("Mode"), printPotsWarningMode());
  return str.join(" ");
}

QString ModelPrinter::printPotsWarningMode()
{
  switch (model.potsWarningMode) {
    case 0:
      return tr("OFF");
    case 1:
      return tr("Manual");
    case 2:
      return tr("Auto");
    default:
      return tr("????");
  }
}

QString ModelPrinter::printFailsafe(int idx)
{
  QStringList strl;
  ModuleData module = model.moduleData[idx];
  strl << printLabelValue(tr("Failsafe Mode"), printFailsafeMode(module.failsafeMode));
  if (module.failsafeMode == FAILSAFE_CUSTOM) {
    for (int i=0; i<module.channelsCount; i++) {
      //strl << QString("%1(%2)").arg(printChannelName(module.channelsStart + i).trimmed()).arg(printFailsafeValue(module.failsafeChannels[i]));
      strl << printLabelValue(printChannelName(module.channelsStart + i).trimmed(), printFailsafeValue(module.failsafeChannels[i]));
    }
  }
  return strl.join(" ");
}

QString ModelPrinter::printFailsafeValue(int val)
{
  switch (val) {
    case 2000:
      return tr("Hold");
    case 2001:
      return tr("No Pulse");
    default:
      return QString("%1%").arg(QString::number(divRoundClosest(val * 1000, 1024) / 10.0));
  }
}

QString ModelPrinter::printFailsafeMode(unsigned int fsmode)
{
  switch (fsmode) {
    case FAILSAFE_NOT_SET:
      return tr("Not set");
    case FAILSAFE_HOLD:
      return tr("Hold");
    case FAILSAFE_CUSTOM:
      return tr("Custom");
    case FAILSAFE_NOPULSES:
      return tr("No pulses");
    case FAILSAFE_RECEIVER:
      return tr("Receiver");
    default:
      return tr("???");
  }
}

QString ModelPrinter::printTimerCountdownBeep(unsigned int countdownBeep)
{
  switch (countdownBeep) {
    case TimerData::COUNTDOWN_SILENT:
      return tr("Silent");
    case TimerData::COUNTDOWN_BEEPS:
      return tr("Beeps");
    case TimerData::COUNTDOWN_VOICE:
      return tr("Voice");
    case TimerData::COUNTDOWN_HAPTIC:
      return tr("Haptic");
    default:
      return tr("???");
  }
}

QString ModelPrinter::printTimerPersistent(unsigned int persistent)
{
  switch (persistent) {
    case 0:
      return tr("OFF");
    case 1:
      return tr("Flight");
    case 2:
      return tr("Manual reset");
    default:
      return tr("???");
  }
}

QString ModelPrinter::printSettingsTrim()
{
  QStringList str;
  str << printLabelValue(tr("Step"), printTrimIncrementMode());
  if (IS_ARM(firmware->getBoard()))
    str << printLabelValue(tr("Display"), printTrimsDisplayMode());
  str << printLabelValue(tr("Extended"), printBoolean(model.extendedTrims, BOOLEAN_YESNO));
  return str.join(" ");
}

QString ModelPrinter::printThrottleSource(int idx)
{
  Boards board = firmware->getBoard();
  int chnstart = board.getCapability(Board::Pots)+board.getCapability(Board::Sliders);
  if (idx == 0)
    return "THR";
  else if (idx < (chnstart+1))
    return firmware->getAnalogInputName(idx+board.getCapability(Board::Sticks)-1);
  else
    return RawSource(SOURCE_TYPE_CH, idx-chnstart-1).toString(&model, &generalSettings);
}

QString ModelPrinter::printTrimsDisplayMode()
{
  switch (model.trimsDisplay) {
    case 0:
      return tr("Never");
    case 1:
      return tr("On Change");
    case 2:
      return tr("Always");
    default:
      return tr("???");
  }
}

QString ModelPrinter::printModuleType(int idx)
{
  if (idx < 0)
    return tr("Trainer Port");
  else if (firmware->getCapability(NumModules) > 1)
    if (IS_HORUS_OR_TARANIS(firmware->getBoard()))
      if (idx == 0)
        return tr("Internal Radio System");
      else
        return tr("External Radio Module");
    else if (idx == 0)
      return tr("Radio System");
    else
      return tr("Extra Radio System");
  else
    return tr("Radio System");
}

QString ModelPrinter::printPxxPower(int power)
{
  static const char *strings[] = {
    "10mW", "100mW", "500mW", "3W"
  };
  return CHECK_IN_ARRAY(strings, power);
}

QString ModelPrinter::printThrottle()
{
  QStringList result;
  result << printLabelValue(tr("Source"), printThrottleSource(model.thrTraceSrc));
  result << printLabelValue(tr("Trim idle only"), printBoolean(model.thrTrim, BOOLEAN_YESNO));
  result << printLabelValue(tr("Warning"), printBoolean(!model.disableThrottleWarning, BOOLEAN_YESNO));
  result << printLabelValue(tr("Reversed"), printBoolean(model.throttleReversed, BOOLEAN_YESNO));
  return result.join(" ");
}

QString ModelPrinter::printPPMFrameLength(int ppmFL)
{
  double result = (((double)ppmFL * 5) + 225) / 10;
  return QString::number(result);
}

QString ModelPrinter::printTimerName(int idx)
{
  QString result;
  result = tr("Tmr") + QString("%1").arg(idx+1);
  if (firmware->getCapability(TimersName) && model.timers[idx].name[0])
    result.append(":" + QString(model.timers[idx].name));

  return result;
}

QString ModelPrinter::printTimerTimeValue(unsigned int val)
{
  return printTimeValue(val, MASK_TIMEVALUE_HRSMINS | MASK_TIMEVALUE_ZEROHRS);
}

QString ModelPrinter::printTimerMinuteBeep(bool mb)
{
  return printBoolean(mb, BOOLEAN_YESNO);
}

QString ModelPrinter::printTelemetryProtocol(unsigned int val)
{
  switch (val) {
    case 0:
      return tr("FrSky S.PORT");
    case 1:
      return tr("FrSky D");
    case 2:
      return tr("FrSky D (cable)");
    default:
      return tr("???");
  }
}

QString ModelPrinter::printRssiAlarmsDisabled(bool mb)
{
  return printBoolean(!mb, BOOLEAN_ENABLEDISABLE);
}

QString ModelPrinter::printTelemetrySource(int val)
{
  QStringList strings = QStringList() << tr("None");

  for (int i=1; i<=CPN_MAX_SENSORS; ++i) {
    strings << QString("%1").arg(model.sensorData[i-1].label);
  }

  return QString("%1%2").arg((val < 0 ? "-" : "")).arg(strings.value(abs(val)));
}

QString ModelPrinter::printVarioSource(unsigned int val)
{
  switch (val) {
    case TELEMETRY_VARIO_SOURCE_ALTI:
      return tr("Alti");
    case TELEMETRY_VARIO_SOURCE_ALTI_PLUS:
      return tr("Alti+");
    case TELEMETRY_VARIO_SOURCE_VSPEED:
      return tr("VSpeed");
    case TELEMETRY_VARIO_SOURCE_A1:
      return tr("A1");
    case TELEMETRY_VARIO_SOURCE_A2:
      return tr("A2");
    default:
      return tr("???");
  }
}

QString ModelPrinter::printVarioCenterSilent(bool mb)
{
  return printBoolean(mb, BOOLEAN_YESNO);
}

QString ModelPrinter::printVoltsSource(unsigned int val)
{
  switch (val) {
    case TELEMETRY_VOLTS_SOURCE_A1:
      return tr("A1");
    case TELEMETRY_VOLTS_SOURCE_A2:
      return tr("A2");
    case TELEMETRY_VOLTS_SOURCE_A3:
      return tr("A3");
    case TELEMETRY_VOLTS_SOURCE_A4:
      return tr("A4");
    case TELEMETRY_VOLTS_SOURCE_FAS:
      return tr("FAS");
    case TELEMETRY_VOLTS_SOURCE_CELLS:
      return tr("Cells");
    default:
      return tr("???");
  }
}

QString ModelPrinter::printCurrentSource(unsigned int val)
{
  switch (val) {
    case TELEMETRY_CURRENT_SOURCE_NONE:
      return tr("None");
    case TELEMETRY_CURRENT_SOURCE_A1:
      return tr("A1");
    case TELEMETRY_CURRENT_SOURCE_A2:
      return tr("A2");
    case TELEMETRY_CURRENT_SOURCE_A3:
      return tr("A3");
    case TELEMETRY_CURRENT_SOURCE_A4:
      return tr("A4");
    case TELEMETRY_CURRENT_SOURCE_FAS:
      return tr("FAS");
    default:
      return tr("???");
  }
}

QString ModelPrinter::printMahPersistent(bool mb)
{
  return printBoolean(mb, BOOLEAN_YESNO);
}

QString ModelPrinter::printIgnoreSensorIds(bool mb)
{
  return printBoolean(mb, BOOLEAN_ENABLEDISABLE);
}

QString ModelPrinter::printSensorType(unsigned int val)
{
  switch (val) {
    case SensorData::TELEM_TYPE_CUSTOM:
      return tr("Custom");
    case SensorData::TELEM_TYPE_CALCULATED:
      return tr("Calculated");
    default:
      return tr("???");
  }
}

QString ModelPrinter::printSensorFormula(unsigned int val)
{
  switch (val) {
    case SensorData::TELEM_FORMULA_ADD:
      return tr("Add");
    case SensorData::TELEM_FORMULA_AVERAGE:
      return tr("Average");
    case SensorData::TELEM_FORMULA_MIN:
      return tr("Min");
    case SensorData::TELEM_FORMULA_MAX:
      return tr("Max");
    case SensorData::TELEM_FORMULA_MULTIPLY:
      return tr("Multiply");
    case SensorData::TELEM_FORMULA_TOTALIZE:
      return tr("Totalise");
    case SensorData::TELEM_FORMULA_CELL:
      return tr("Cell");
    case SensorData::TELEM_FORMULA_CONSUMPTION:
      return tr("Consumption");
    case SensorData::TELEM_FORMULA_DIST:
      return tr("Distance");
    default:
      return tr("???");
  }
}

QString ModelPrinter::printSensorCells(unsigned int val)
{
  QStringList strings;

  strings << tr("Lowest");
  for (int i=1; i<=6; i++)
    strings << tr("Cell %1").arg(i);
  strings << tr("Highest") << tr("Delta");

  return strings.value(val);
}

QString ModelPrinter::printSensorTypeCond(unsigned int idx)
{
  if (!model.sensorData[idx].isAvailable())
    return "";
  else
    return printSensorType(model.sensorData[idx].type);
}

QString ModelPrinter::printSensorDetails(unsigned int idx)
{
  QString str = "";
  SensorData sensor = model.sensorData[idx];

  if (!sensor.isAvailable())
    return str;

  bool isConfigurable = false;
  bool gpsFieldsPrinted = false;
  bool cellsFieldsPrinted = false;
  bool consFieldsPrinted = false;
  bool ratioFieldsPrinted = false;
  bool totalizeFieldsPrinted = false;
  bool sources12FieldsPrinted = false;
  bool sources34FieldsPrinted = false;

  str.append(doTableCell(printSensorTypeCond(idx)));

  QString tc = "";
  if (sensor.type == SensorData::TELEM_TYPE_CALCULATED) {
    isConfigurable = (sensor.formula < SensorData::TELEM_FORMULA_CELL);
    gpsFieldsPrinted = (sensor.formula == SensorData::TELEM_FORMULA_DIST);
    cellsFieldsPrinted = (sensor.formula == SensorData::TELEM_FORMULA_CELL);
    consFieldsPrinted = (sensor.formula == SensorData::TELEM_FORMULA_CONSUMPTION);
    sources12FieldsPrinted = (sensor.formula <= SensorData::TELEM_FORMULA_MULTIPLY);
    sources34FieldsPrinted = (sensor.formula < SensorData::TELEM_FORMULA_MULTIPLY);
    totalizeFieldsPrinted = (sensor.formula == SensorData::TELEM_FORMULA_TOTALIZE);

    tc.append(printLabelValue(tr("Formula"), printSensorFormula(sensor.formula)));
  }
  else {
    isConfigurable = sensor.unit < SensorData::UNIT_FIRST_VIRTUAL;
    ratioFieldsPrinted = (sensor.unit < SensorData::UNIT_FIRST_VIRTUAL);

    tc.append(printLabelValue(tr("Id"), QString::number(sensor.id,16).toUpper()));
    tc.append(printLabelValue(tr("Instance"), QString::number(sensor.instance)));
  }
  if (cellsFieldsPrinted) {
    tc.append(printLabelValue(tr("Sensor"), QString("%1 > %2").arg(printTelemetrySource(sensor.source), false).arg(printSensorCells(sensor.index))));
  }
  if (sources12FieldsPrinted) {
    QStringList srcs;
    for (int i=0;i<4;i++) {
      if (i < 2 || sources34FieldsPrinted) {
        srcs << printTelemetrySource(sensor.sources[i]);
      }
    }
    tc.append(printLabelValues(tr("Sources"), srcs));
  }
  if (consFieldsPrinted || totalizeFieldsPrinted)
    tc.append(printLabelValue(tr("Sensor"), printTelemetrySource(sensor.amps)));
  if (gpsFieldsPrinted) {
    tc.append(printLabelValue(tr("GPS"), printTelemetrySource(sensor.gps)));
    tc.append(printLabelValue(tr("Alt."), printTelemetrySource(sensor.alt)));
  }
  if (ratioFieldsPrinted && sensor.unit == SensorData::UNIT_RPMS) {
      tc.append(printLabelValue(tr("Blades"), QString::number(sensor.ratio)));
      tc.append(printLabelValue(tr("Multi."), QString::number(sensor.offset)));
  }
  str.append(doTableCell(tc));

  tc = sensor.unitString();
  tc = tc.trimmed() == "" ? "-" : tc;
  str.append(doTableCell(tc));

  if (isConfigurable && sensor.unit != SensorData::UNIT_FAHRENHEIT)
    tc = QString::number(sensor.prec);
  else
    tc = "";
  str.append(doTableCell(tc));

  if (!ratioFieldsPrinted) {
    str.append(doTableCell(""));
    str.append(doTableCell(""));
  }
  else if (sensor.unit != SensorData::UNIT_RPMS) {
      int prec = sensor.prec == 0 ? 1 : pow(10, sensor.prec);
      str.append(doTableCell(QString::number((float)sensor.ratio / prec)));
      str.append(doTableCell(QString::number((float)sensor.offset / prec)));
  }

  if (sensor.unit != SensorData::UNIT_RPMS && isConfigurable)
    str.append(doTableCell(printBoolean(sensor.autoOffset, BOOLEAN_YN)));
  else
    str.append(doTableCell(""));

  if (isConfigurable)
    str.append(doTableCell(printBoolean(sensor.filter, BOOLEAN_YN)));
  else
    str.append(doTableCell(""));

  if (sensor.type == SensorData::TELEM_TYPE_CALCULATED)
    str.append(doTableCell(printBoolean(sensor.persistent, BOOLEAN_YN)));
  else
    str.append(doTableCell(""));

  str.append(doTableCell(printBoolean(sensor.onlyPositive, BOOLEAN_YN)));
  str.append(doTableCell(printBoolean(sensor.logs, BOOLEAN_YN), false));
  return str;
}

QString ModelPrinter::printSensorParams(unsigned int idx)
{
  QString str = "";
  SensorData sensor = model.sensorData[idx];

  if (!sensor.isAvailable())
    return str;

  bool isConfigurable = false;
  bool gpsFieldsPrinted = false;
  bool cellsFieldsPrinted = false;
  bool consFieldsPrinted = false;
  bool ratioFieldsPrinted = false;
  bool totalizeFieldsPrinted = false;
  bool sources12FieldsPrinted = false;
  bool sources34FieldsPrinted = false;

  if (sensor.type == SensorData::TELEM_TYPE_CALCULATED) {
    isConfigurable = (sensor.formula < SensorData::TELEM_FORMULA_CELL);
    gpsFieldsPrinted = (sensor.formula == SensorData::TELEM_FORMULA_DIST);
    cellsFieldsPrinted = (sensor.formula == SensorData::TELEM_FORMULA_CELL);
    consFieldsPrinted = (sensor.formula == SensorData::TELEM_FORMULA_CONSUMPTION);
    sources12FieldsPrinted = (sensor.formula <= SensorData::TELEM_FORMULA_MULTIPLY);
    sources34FieldsPrinted = (sensor.formula < SensorData::TELEM_FORMULA_MULTIPLY);
    totalizeFieldsPrinted = (sensor.formula == SensorData::TELEM_FORMULA_TOTALIZE);

    str.append(printLabelValue(tr("F"), printSensorFormula(sensor.formula)));
  }
  else {
    isConfigurable = sensor.unit < SensorData::UNIT_FIRST_VIRTUAL;
    ratioFieldsPrinted = (sensor.unit < SensorData::UNIT_FIRST_VIRTUAL);

    str.append(printLabelValue(tr("Id"), QString::number(sensor.id,16).toUpper()));
    str.append(printLabelValue(tr("Inst"), QString::number(sensor.instance)));
  }
  if (cellsFieldsPrinted) {
    str.append(printLabelValue(tr("Sensor"), QString("%1 %2").arg(printTelemetrySource(sensor.source), false).arg(printSensorCells(sensor.index))));
  }
  if (sources12FieldsPrinted) {
    QStringList srcs;
    for (int i=0;i<4;i++) {
      if (i < 2 || sources34FieldsPrinted) {
        srcs << printTelemetrySource(sensor.sources[i]);
      }
    }
    str.append(printLabelValues(tr("Sources"), srcs));
  }
  if (consFieldsPrinted || totalizeFieldsPrinted)
    str.append(printLabelValue(tr("Sensor"), printTelemetrySource(sensor.amps)));
  if (gpsFieldsPrinted) {
    str.append(printLabelValue(tr("GPS"), printTelemetrySource(sensor.gps)));
    str.append(printLabelValue(tr("Alt"), printTelemetrySource(sensor.alt)));
  }
  QString u = sensor.unitString();
  u = u.trimmed() == "" ? "-" : u;
  str.append(printLabelValue(tr("Unit"), u));
  if (isConfigurable && sensor.unit != SensorData::UNIT_FAHRENHEIT)
    str.append(printLabelValue(tr("Prec"), QString::number(sensor.prec)));
  if (ratioFieldsPrinted) {
    if (sensor.unit != SensorData::UNIT_RPMS) {
      int prec = sensor.prec == 0 ? 1 : pow(10, sensor.prec);
      str.append(printLabelValue(tr("Ratio"), QString::number((float)sensor.ratio / prec)));
      str.append(printLabelValue(tr("Offset"), QString::number((float)sensor.offset / prec)));
    }
    else if (sensor.unit == SensorData::UNIT_RPMS) {
      str.append(printLabelValue(tr("Blades"), QString::number(sensor.ratio)));
      str.append(printLabelValue(tr("Multi"), QString::number(sensor.offset)));
    }
  }
  if (sensor.unit != SensorData::UNIT_RPMS && isConfigurable)
    str.append(printLabelValue(tr("A/Offset"), printBoolean(sensor.autoOffset, BOOLEAN_YN)));
  if (isConfigurable)
    str.append(printLabelValue(tr("Filter"), printBoolean(sensor.filter, BOOLEAN_YN)));
  if (sensor.type == SensorData::TELEM_TYPE_CALCULATED)
    str.append(printLabelValue(tr("Persist"), printBoolean(sensor.persistent, BOOLEAN_YN)));
  str.append(printLabelValue(tr("Positive"), printBoolean(sensor.onlyPositive, BOOLEAN_YN)));
  str.append(printLabelValue(tr("Log"), printBoolean(sensor.logs, BOOLEAN_YN), false));
  return str;
}

QString ModelPrinter::printTelemetryScreenType(unsigned int val)
{
  switch (val) {
    case TelemetryScreenEnum::TELEMETRY_SCREEN_NONE:
      return tr("None");
    case TelemetryScreenEnum::TELEMETRY_SCREEN_NUMBERS:
      return tr("Numbers");
    case TelemetryScreenEnum::TELEMETRY_SCREEN_BARS:
      return tr("Bars");
    case TelemetryScreenEnum::TELEMETRY_SCREEN_SCRIPT:
      return tr("Script");
    default:
      return tr("???");
  }
}

QString ModelPrinter::printTelemetryScreen(unsigned int idx, unsigned int line, unsigned int width)
{
  QStringList strl;
  QStringList hd;
  FrSkyScreenData screen = model.frsky.screens[idx];
  hd << "";  // blank 1st column
  strl << "";
  if (screen.type == TelemetryScreenEnum::TELEMETRY_SCREEN_NUMBERS) {
    if (line == 0) {
      for (int c=0; c<firmware->getCapability(TelemetryCustomScreensFieldsPerLine); c++) {
        hd << tr("Source");
      }
    }
    for (int c=0; c<firmware->getCapability(TelemetryCustomScreensFieldsPerLine); c++) {
      RawSource source = screen.body.lines[line].source[c];
      strl << source.toString(&model, &generalSettings);
    }
  }
  else if (screen.type == TelemetryScreenEnum::TELEMETRY_SCREEN_BARS) {
    if (line == 0) {
      hd << tr("Source") << tr("Min") << tr("Max");
    }
    RawSource source = screen.body.bars[line].source;
    RawSourceRange range = source.getRange(&model, generalSettings);
    strl << source.toString(&model, &generalSettings);
    QString unit;
    QString minstr;
    QString maxstr;
    if (source.isTimeBased()){
      minstr = printTimeValue((float)screen.body.bars[line].barMin, MASK_TIMEVALUE_HRSMINS);
      maxstr = printTimeValue((float)screen.body.bars[line].barMax, MASK_TIMEVALUE_HRSMINS);
    }
    else {
      minstr = QString::number(range.getValue(screen.body.bars[line].barMin));
      maxstr = QString::number(range.getValue(screen.body.bars[line].barMax));
      unit = range.unit;
    }
    strl << QString("%1%2").arg(minstr).arg(unit);
    strl << QString("%1%2").arg(maxstr).arg(unit);
  }
  else if (screen.type == TelemetryScreenEnum::TELEMETRY_SCREEN_SCRIPT && line == 0) {
    hd << tr("Filename");
    strl << QString("%1.lua").arg(screen.body.script.filename);
  }
  return (hd.count() > 1 ? doTableRow(hd, width / hd.count(), "left", "", true) : "" ) + doTableRow(strl, width / strl.count());
}
