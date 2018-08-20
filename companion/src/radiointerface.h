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

#ifndef _RADIOINTERFACE_H_
#define _RADIOINTERFACE_H_

#include <QString>
#include <QStringList>

class ProgressWidget;

QString getRadioInterfaceCmd();

QString findMassstoragePath(const QString &filename, bool onlyPath = false);

QStringList getAvrdudeArgs(const QString &cmd, const QString &filename);
QStringList getSambaArgs(const QString &tcl);
QStringList getDfuArgs(const QString &cmd, const QString &filename);

void readAvrdudeFuses(ProgressWidget *progress);
void resetAvrdudeFuses(bool eepromProtect, ProgressWidget *progress);

QStringList getReadEEpromCmd(const QString &filename);
QStringList getWriteEEpromCmd(const QString &filename);
QStringList getReadFirmwareArgs(const QString &filename);
QStringList getWriteFirmwareArgs(const QString &filename);

bool readFirmware(const QString &filename, ProgressWidget *progress);
bool writeFirmware(const QString &filename, ProgressWidget *progress);
bool readEeprom(const QString &filename, ProgressWidget *progress);
bool writeEeprom(const QString &filename, ProgressWidget *progress);

#endif // _RADIOINTERFACE_H_
