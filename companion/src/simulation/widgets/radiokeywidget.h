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

#ifndef RADIOKEYWIDGET_H
#define RADIOKEYWIDGET_H

#include "radiouiaction.h"
#include "radiowidget.h"

class RadioKeyWidget : public RadioWidget
{
  Q_OBJECT

  public:

    explicit RadioKeyWidget(const QPolygon & polygon, const QString &image, RadioUiAction * action = NULL, QWidget * parent = NULL, Qt::WindowFlags f = Qt::WindowFlags()):
      RadioWidget(action, parent, f),
      polygon(polygon),
      imgFile(image)
    {
      m_type = RADIO_WIDGET_KEY;
      setValue(0);
      hide();  // we're a "virtual" button for now
      setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    }

    virtual void setAction(RadioUiAction * action)
    {
      if (m_action)
        disconnect(m_action, 0, this, 0);
      RadioWidget::setAction(action);
      if (m_action)
        connect(m_action, &RadioUiAction::toggled, this, &RadioKeyWidget::onActionToggled);
    }

    virtual int getValue() const
    {
      return (m_action && m_action->isActive()) ? 1 : 0;
    }

    void press() {
      if (m_action)
        m_action->trigger(true);
    }

    void release() {
      if (m_action)
        m_action->trigger(false);
    }

    void toggle(bool down)
    {
      if (down)
        press();
      else
        release();
    }

    bool contains(const QPoint & point)
    {
      return polygon.containsPoint(point, Qt::OddEvenFill);
    }

  signals:

    void imageChanged(const QString image);

  protected:

    virtual void onActionToggled(int index, bool active)
    {
      RadioWidget::onActionToggled(index, active);
      emit imageChanged(active ? imgFile : "");
    }

    QPolygon polygon;
    QString imgFile;

};

#endif // RADIOKEYWIDGET_H


