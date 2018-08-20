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

#ifndef _CALIBRATION_H_
#define _CALIBRATION_H_

#include <QTableWidget>
#include "generaledit.h"

namespace Ui {
  class Calibration;
}

class QLabel;
class AutoLineEdit;
class AutoComboBox;

class CalibrationPanel : public GeneralPanel
{
    Q_OBJECT

  public:
    CalibrationPanel(QWidget *parent, GeneralSettings & generalSettings, Firmware * firmware);
    virtual ~CalibrationPanel() {};

  private slots:
    void onCellChanged(int value);

  private:
    QTableWidget * tableWidget;

    int getCalibrationValue(int row, int column);
    void setCalibrationValue(int row, int column, int value);
};

#endif // _CALIBRATION_H_
