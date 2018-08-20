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

#ifndef _MDICHILD_H_
#define _MDICHILD_H_

#include "eeprominterface.h"
#include "modelslist.h"

#include <QActionGroup>
#include <QtGui>
#include <QMessageBox>
#include <QProxyStyle>
#include <QWidget>

class QToolBar;

namespace Ui {
class MdiChild;
}

class MdiChild : public QWidget
{
  Q_OBJECT

  public:
    enum Actions {
      ACT_GEN_EDT,  // radio general settings
      ACT_GEN_CPY,
      ACT_GEN_PST,
      ACT_GEN_SIM,
      ACT_ITM_EDT,  // edit model/rename category
      ACT_ITM_DEL,  // delete model or cat
      ACT_CAT_ADD,  // category actions...
      //ACT_CAT_EDT,  // not sure these are needed...
      //ACT_CAT_DEL,  // the ACT_ITM_* actions do the same thing
      ACT_CAT_SEP,  // convenience separator shown/hidden with category actions
      ACT_MDL_ADD,  // model actions...
      ACT_MDL_CPY,
      ACT_MDL_CUT,
      ACT_MDL_PST,
      ACT_MDL_DUP,
      ACT_MDL_INS,
      ACT_MDL_MOV,
      ACT_MDL_RTR,  // ResToRe backup
      ACT_MDL_WIZ,
      ACT_MDL_DFT,  // set as DeFaulT
      ACT_MDL_PRT,  // print
      ACT_MDL_SIM,
      ACT_ENUM_END
    };

    MdiChild(QWidget *parent = Q_NULLPTR, QWidget * parentWin = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~MdiChild();

    QString currentFile() const;
    QString userFriendlyCurrentFile() const;
    QVector<int> getSelectedCategories() const;
    QVector<int> getSelectedModels() const;
    QList<QAction *> getGeneralActions();
    QList<QAction *> getEditActions(bool incCatNew = true);
    QList<QAction *> getModelActions();
    //QList<QAction *> getCategoryActions();
    QAction * getAction(const Actions type);

  public slots:
    void newFile(bool createDefaults = true);
    bool loadFile(const QString & fileName, bool resetCurrentFile=true);
    bool save();
    bool saveAs(bool isNew=false);
    bool saveFile(const QString & fileName, bool setCurrent=true);
    void closeFile(bool force = false);
    void writeEeprom();
    void print(int model=-1, const QString & filename="");
    void onFirmwareChanged();

  signals:
    void modified();
    void newStatusMessage(const QString & msg, const int duration);

  protected:
    virtual void changeEvent(QEvent * event);
    virtual void closeEvent(QCloseEvent * event);
    virtual void resizeEvent(QResizeEvent * event);
    virtual QSize sizeHint() const;

  private slots:
    void setupNavigation();
    void updateNavigation();
    void updateTitle();
    void setModified();
    void retranslateUi();
    void showModelsListContextMenu(const QPoint & pos);
    void showContextMenu(const QPoint & pos);
    void adjustToolbarLayout();

    void initModelsList();
    void refresh();
    void onItemActivated(const QModelIndex index);
    void onItemSelected(const QModelIndex &);
    void onCurrentItemChanged(const QModelIndex &, const QModelIndex &);
    void onDataChanged(const QModelIndex & index);

    void generalEdit();
    void copyGeneralSettings();
    void pasteGeneralSettings();

    void copy();
    void cut();
    void paste();
    void insert();
    void edit();
    void confirmDelete();
    void categoryAdd();
    void modelAdd();
    void modelEdit();
    void wizardEdit();
    void modelDuplicate();
    void onModelMoveToCategory();

    void openModelWizard(int row = -1);
    void openModelEditWindow(int row = -1);

    void setDefault();
    void modelSimulate();
    void radioSimulate();
    bool loadBackup();

    void pasteModelData(const QMimeData * mimeData, const QModelIndex row, bool insert = false, bool move = false);
    void pasteGeneralData(const QMimeData * mimeData);

  private:
    QAction *addAct(Actions actId, const QString & icon, const char * slot = 0, const QKeySequence & shortcut = 0, QObject * slotObj = NULL);

    QModelIndex getCurrentIndex() const;
    int getCurrentModel() const;
    int countSelectedModels() const;
    bool hasSelectedModel();
    bool setSelectedModel(const int modelIndex);
    int getCurrentCategory() const;
    int countSelectedCats() const;
    bool hasSelectedCat();

    bool deleteCategory(int categoryIndex = -1, QString * error = NULL);
    void deleteSelectedCats();

    void checkAndInitModel(int row);
    void findNewDefaultModel(const unsigned startAt = 0);
    bool insertModelRows(int atModelIdx, int count);
    int modelAppend(const ModelData model);
    int newModel(int modelIndex = -1, int categoryIndex = -1);
    unsigned deleteModels(const QVector<int> modelIndices);
    bool deleteModel(const int modelIndex);
    void deleteSelectedModels();
    void moveModelsToCategory(const QVector<int> models, const int toCategoryId);
    void moveSelectedModelsToCat(const int toCategoryId);
    unsigned countUsedModels(const int categoryId = -1);

    void clearCutList();
    void removeModelFromCutList(const int modelIndex);
    bool hasClipboardData(const quint8 type = 0) const;

    bool maybeSave();
    void setCurrentFile(const QString & fileName);
    void forceNewFilename(const QString & suffix = "", const QString & ext = "otx");
    bool convertStorage(Board::Type from, Board::Type to, bool newFile = false);
    void showWarning(const QString & msg);
    int askQuestion(const QString & msg, QMessageBox::StandardButtons buttons = (QMessageBox::Yes | QMessageBox::No), QMessageBox::StandardButton defaultButton = QMessageBox::No);

    Ui::MdiChild * ui;
    TreeModel * modelsListModel;
    QWidget * parentWindow;

    QString curFile;
    QVector<int> cutModels;
    QVector<QAction *> action;
    QToolBar * radioToolbar;
    QToolBar * categoriesToolbar;
    QToolBar * modelsToolbar;

    Firmware * firmware;
    RadioData radioData;

    int lastSelectedModel;
    bool isUntitled;
    bool showCatToolbar;
    bool forceCloseFlag;
    const quint16 stateDataVersion;
};

// This will draw the drop indicator across all columns of a model View (vs. in just one column), and lets us make the indicator more obvious.
class ItemViewProxyStyle: public QProxyStyle
{
  public:
    ItemViewProxyStyle(QStyle * style = 0) : QProxyStyle(style) {}

    void drawPrimitive(PrimitiveElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = 0) const
    {
      if (element == QStyle::PE_IndicatorItemViewItemDrop && !option->rect.isNull()) {
        painter->save();
        // set a wider stroke
        QPen pen(painter->pen());
        pen.setWidthF(2.5f);
        painter->setPen(pen);
        // adjust the rectangle size of the indicator to encompass the whole row
        QStyleOption opt(*option);
        opt.rect.setLeft(0);
        if (widget)
          opt.rect.setRight(widget->width());
        // call default handler and exit
        QProxyStyle::drawPrimitive(element, &opt, painter, widget);
        painter->restore();
      }
      else {
        QProxyStyle::drawPrimitive(element, option, painter, widget);
      }
    }
};

#endif // _MDICHILD_H_
