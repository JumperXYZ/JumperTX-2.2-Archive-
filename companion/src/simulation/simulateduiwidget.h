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

#ifndef SIMULATEDUIWIDGET_H
#define SIMULATEDUIWIDGET_H

#include "boards.h"
#include "constants.h"
#include "radiowidget.h"
#include "simulator.h"
#include "simulator_strings.h"

#include <QWidget>
#include <QMouseEvent>

class SimulatorInterface;
class LcdWidget;
class RadioKeyWidget;
class RadioUiAction;

/*
 * This is a base class for the main hardware-specific radio user interface, including LCD screen and navigation buttons/widgets.
 * It is responsible for hanlding all interactions with this part of the simulation (vs. common radio widgets like sticks/switches/knobs).
 * Sub-classes are responsible for building the actual UI form they need for presentation.
 * This base class should not be instantiated directly.
 */
class SimulatedUIWidget : public QWidget
{
  Q_OBJECT

  protected:

    explicit SimulatedUIWidget(SimulatorInterface * simulator, QWidget * parent = NULL);

  public:

    ~SimulatedUIWidget();

    RadioWidget * addRadioWidget(RadioWidget * keyWidget);
    RadioUiAction * addRadioAction(RadioUiAction * act);

    QVector<Simulator::keymapHelp_t> getKeymapHelp() const;

    static QPolygon polyArc(int ctrX, int ctrY, int radius, int startAngle = 0, int endAngle = 360, int step = 10);

  public slots:

    void captureScreenshot();

    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

  signals:

    void controlValueChange(RadioWidget::RadioWidgetType type, int index, int value);
    void customStyleRequest(const QString & style);
    void simulatorWheelEvent(qint8 steps);

  protected slots:

    void setLcd(LcdWidget * lcd);
    void connectScrollActions();
    void onLcdChange(bool backlightEnable);
    virtual void setLightOn(bool enable) { }

  protected:

    SimulatorInterface * m_simulator;
    QWidget * m_parent;
    LcdWidget * m_lcd;
    QVector<QColor> m_backlightColors;
    QList<RadioUiAction *> m_actions;
    QList<RadioWidget *> m_widgets;
    RadioUiAction * m_scrollUpAction;
    RadioUiAction * m_scrollDnAction;
    RadioUiAction * m_mouseMidClickAction;
    RadioUiAction * m_screenshotAction;
    Board::Type m_board;
    unsigned int m_backLight;
    int m_beepShow;
    int m_beepVal;

};


// Each subclass is responsible for its own Ui
namespace Ui {
  class SimulatedUIWidget9X;
  class SimulatedUIWidgetX7;
  class SimulatedUIWidgetX9;
  class SimulatedUIWidgetXLITE;
  class SimulatedUIWidgetX9E;
  class SimulatedUIWidgetX10;
  class SimulatedUIWidgetX12;
}

class SimulatedUIWidget9X: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidget9X(SimulatorInterface * simulator, QWidget * parent = NULL);
    virtual ~SimulatedUIWidget9X();

  protected:
    void setLightOn(bool enable);

  private:
    Ui::SimulatedUIWidget9X * ui;

};

class SimulatedUIWidgetX7: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetX7(SimulatorInterface * simulator, QWidget * parent = NULL);
    virtual ~SimulatedUIWidgetX7();

  private:
    Ui::SimulatedUIWidgetX7 * ui;
};

class SimulatedUIWidgetX9: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetX9(SimulatorInterface * simulator, QWidget * parent = NULL);
    virtual ~SimulatedUIWidgetX9();

  private:
    Ui::SimulatedUIWidgetX9 * ui;
};

class SimulatedUIWidgetXLITE: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetXLITE(SimulatorInterface * simulator, QWidget * parent = NULL);
    virtual ~SimulatedUIWidgetXLITE();

  private:
    Ui::SimulatedUIWidgetXLITE * ui;
};

class SimulatedUIWidgetX9E: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetX9E(SimulatorInterface * simulator, QWidget * parent = NULL);
    virtual ~SimulatedUIWidgetX9E();

  private:
    Ui::SimulatedUIWidgetX9E * ui;
};

class SimulatedUIWidgetX10: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetX10(SimulatorInterface * simulator, QWidget * parent = NULL);
    virtual ~SimulatedUIWidgetX10();

  private:
    Ui::SimulatedUIWidgetX10 * ui;
};

class SimulatedUIWidgetX12: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetX12(SimulatorInterface * simulator, QWidget * parent = NULL);
    virtual ~SimulatedUIWidgetX12();

  private:
    Ui::SimulatedUIWidgetX12 * ui;
};

#endif // SIMULATEDUIWIDGET_H
