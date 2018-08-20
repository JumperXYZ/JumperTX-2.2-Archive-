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

#include "heli.h"
#include "ui_heli.h"
#include "helpers.h"

HeliPanel::HeliPanel(QWidget *parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware):
  ModelPanel(parent, model, generalSettings, firmware),
  ui(new Ui::Heli),
  rawSourceItemModel(nullptr)
{
  ui->setupUi(this);

  setDataModels();

  connect(ui->swashType, SIGNAL(currentIndexChanged(int)), this, SLOT(edited()));
  connect(ui->swashRingVal, SIGNAL(editingFinished()), this, SLOT(edited()));
  connect(ui->swashCollectiveSource, SIGNAL(currentIndexChanged(int)), this, SLOT(edited()));
  if (firmware->getCapability(VirtualInputs)) {
    connect(ui->swashAileronSource, SIGNAL(currentIndexChanged(int)), this, SLOT(edited()));
    connect(ui->swashElevatorSource, SIGNAL(currentIndexChanged(int)), this, SLOT(edited()));
    connect(ui->swashAileronWeight, SIGNAL(editingFinished()), this, SLOT(edited()));
    connect(ui->swashElevatorWeight, SIGNAL(editingFinished()), this, SLOT(edited()));
    connect(ui->swashCollectiveWeight, SIGNAL(editingFinished()), this, SLOT(edited()));
    ui->invertLabel->hide();
    ui->swashElevatorInvert->hide();
    ui->swashAileronInvert->hide();
    ui->swashCollectiveInvert->hide();
  }
  else {
    connect(ui->swashElevatorInvert, SIGNAL(stateChanged(int)), this, SLOT(edited()));
    connect(ui->swashAileronInvert, SIGNAL(stateChanged(int)), this, SLOT(edited()));
    connect(ui->swashCollectiveInvert, SIGNAL(stateChanged(int)), this, SLOT(edited()));
    ui->aileronLabel->hide();
    ui->elevatorLabel->hide();
    ui->swashAileronSource->hide();
    ui->swashElevatorSource->hide();
    ui->swashAileronWeight->hide();
    ui->swashElevatorWeight->hide();
    ui->swashCollectiveWeight->hide();
  }

  disableMouseScrolling();
}

HeliPanel::~HeliPanel()
{
  delete ui;
}

void HeliPanel::setDataModels()
{
  if (rawSourceItemModel)
    rawSourceItemModel->deleteLater();

  rawSourceItemModel = Helpers::getRawSourceItemModel(&generalSettings, model, POPULATE_NONE | POPULATE_SOURCES | POPULATE_VIRTUAL_INPUTS | POPULATE_SWITCHES | POPULATE_TRIMS);
  rawSourceItemModel->setParent(this);
}

void HeliPanel::update()
{
  lock = true;
  setDataModels();

  ui->swashType->setCurrentIndex(model->swashRingData.type);
  ui->swashCollectiveSource->setModel(rawSourceItemModel);
  ui->swashCollectiveSource->setCurrentIndex(ui->swashCollectiveSource->findData(model->swashRingData.collectiveSource.toValue()));
  ui->swashRingVal->setValue(model->swashRingData.value);
  if (firmware->getCapability(VirtualInputs)) {
    ui->swashElevatorSource->setModel(rawSourceItemModel);
    ui->swashElevatorSource->setCurrentIndex(ui->swashElevatorSource->findData(model->swashRingData.elevatorSource.toValue()));
    ui->swashAileronSource->setModel(rawSourceItemModel);
    ui->swashAileronSource->setCurrentIndex(ui->swashAileronSource->findData(model->swashRingData.aileronSource.toValue()));
    ui->swashElevatorWeight->setValue(model->swashRingData.elevatorWeight);
    ui->swashAileronWeight->setValue(model->swashRingData.aileronWeight);
    ui->swashCollectiveWeight->setValue(model->swashRingData.collectiveWeight);
  }
  else {
    ui->swashElevatorInvert->setChecked(model->swashRingData.elevatorWeight < 0);
    ui->swashAileronInvert->setChecked(model->swashRingData.aileronWeight < 0);
    ui->swashCollectiveInvert->setChecked(model->swashRingData.collectiveWeight < 0);
  }

  lock = false;
}

void HeliPanel::edited()
{
  if (!lock) {
    model->swashRingData.type  = ui->swashType->currentIndex();
    model->swashRingData.collectiveSource = RawSource(ui->swashCollectiveSource->itemData(ui->swashCollectiveSource->currentIndex()).toInt());
    model->swashRingData.value = ui->swashRingVal->value();
    if (firmware->getCapability(VirtualInputs)) {
      model->swashRingData.elevatorSource = RawSource(ui->swashElevatorSource->itemData(ui->swashElevatorSource->currentIndex()).toInt());
      model->swashRingData.aileronSource = RawSource(ui->swashAileronSource->itemData(ui->swashAileronSource->currentIndex()).toInt());
      model->swashRingData.elevatorWeight = ui->swashElevatorWeight->value();
      model->swashRingData.aileronWeight = ui->swashAileronWeight->value();
      model->swashRingData.collectiveWeight = ui->swashCollectiveWeight->value();
    }
    else {
      model->swashRingData.elevatorWeight = (ui->swashElevatorInvert->isChecked() ? -100 : 100);
      model->swashRingData.aileronWeight = (ui->swashAileronInvert->isChecked() ? -100 : 100);
      model->swashRingData.collectiveWeight = (ui->swashCollectiveInvert->isChecked() ? -100 : 100);
    }
    emit modified();
  }
}
