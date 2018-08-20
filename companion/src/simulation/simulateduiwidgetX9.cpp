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
 
// NOTE: RadioUiAction(NUMBER,...): NUMBER relates to enum EnumKeys in the specific board.h

#include "simulateduiwidget.h"
#include "ui_simulateduiwidgetX9.h"

SimulatedUIWidgetX9::SimulatedUIWidgetX9(SimulatorInterface *simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidgetX9)
{
  RadioUiAction * act;

  ui->setupUi(this);

  // add actions in order of appearance on the help menu

  // button placement, size, spacing
  const int btop = 45, vsp = 17;
  QRect btn(16, btop, 46, 31);

  // left side

  act = new RadioUiAction(0, QList<int>() << Qt::Key_Up << Qt::Key_PageUp, SIMU_STR_HLP_KEYS_GO_UP, SIMU_STR_HLP_ACT_MENU);
  addRadioWidget(ui->leftbuttons->addArea(btn, "Taranis/x9l1.png", act));
  btn.moveTop(btn.top() + btn.height() + vsp);

  act = new RadioUiAction(3, QList<int>() << Qt::Key_Down << Qt::Key_PageDown, SIMU_STR_HLP_KEYS_GO_DN, SIMU_STR_HLP_ACT_PAGE);
  addRadioWidget(ui->leftbuttons->addArea(btn, "Taranis/x9l2.png", act));
  btn.moveTop(btn.top() + btn.height() + vsp);

  act = new RadioUiAction(1, QList<int>() << Qt::Key_Delete << Qt::Key_Escape << Qt::Key_Backspace, SIMU_STR_HLP_KEYS_EXIT, SIMU_STR_HLP_ACT_EXIT);
  addRadioWidget(ui->leftbuttons->addArea(btn, "Taranis/x9l3.png", act));

  m_scrollUpAction = new RadioUiAction(4, QList<int>() << Qt::Key_Plus << Qt::Key_Equal << Qt::Key_Left,
                                      SIMU_STR_HLP_KEY_LFT % "|" % SIMU_STR_HLP_KEY_PLS % "|" % SIMU_STR_HLP_MOUSE_UP, SIMU_STR_HLP_ACT_PLS);
  m_scrollDnAction = new RadioUiAction(5, QList<int>() << Qt::Key_Minus << Qt::Key_Right,
                                      SIMU_STR_HLP_KEY_RGT % "|" % SIMU_STR_HLP_KEY_MIN % "|" % SIMU_STR_HLP_MOUSE_DN, SIMU_STR_HLP_ACT_MIN);

  // right side
  btn.moveTopLeft(QPoint(58, btop));

  addRadioWidget(ui->rightbuttons->addArea(btn, "Taranis/x9r1.png", m_scrollUpAction));
  btn.moveTop(btn.top() + btn.height() + vsp);

  addRadioWidget(ui->rightbuttons->addArea(btn, "Taranis/x9r2.png", m_scrollDnAction));
  btn.moveTop(btn.top() + btn.height() + vsp);

  m_mouseMidClickAction = new RadioUiAction(2, QList<int>() << Qt::Key_Enter << Qt::Key_Return, SIMU_STR_HLP_KEYS_ACTIVATE, SIMU_STR_HLP_ACT_ENT);
  addRadioWidget(ui->rightbuttons->addArea(btn, "Taranis/x9r3.png", m_mouseMidClickAction));

  addRadioWidget(ui->leftbuttons->addArea(QRect(89, 177, 30, 20), "Taranis/x9l4.png", m_screenshotAction));

  m_backlightColors << QColor(47, 123, 227);  // Taranis Blue
  m_backlightColors << QColor(166,247,159);
  m_backlightColors << QColor(247,159,166);
  m_backlightColors << QColor(255,195,151);
  m_backlightColors << QColor(247,242,159);

  setLcd(ui->lcd);
}

SimulatedUIWidgetX9::~SimulatedUIWidgetX9()
{
  delete ui;
}

