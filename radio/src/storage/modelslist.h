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

#ifndef _MODELSLIST_H_
#define _MODELSLIST_H_

#include <stdint.h>
#include <list>
#include "sdcard.h"

#define MODELCELL_WIDTH                172
#define MODELCELL_HEIGHT               59

// modelXXXXXXX.bin F,FF F,3F,FF\r\n
#define LEN_MODELS_IDX_LINE (LEN_MODEL_FILENAME + sizeof(" F,FF F,3F,FF\r\n")-1)

struct SimpleModuleData
{
  uint8_t type;
  uint8_t rfProtocol;
};

class ModelCell
{
public:
  char modelFilename[LEN_MODEL_FILENAME+1];
  char modelName[LEN_MODEL_NAME+1];
  BitmapBuffer * buffer;

  bool             valid_rfData;
  uint8_t          modelId[NUM_MODULES];
  SimpleModuleData moduleData[NUM_MODULES];

  ModelCell(const char * name);
  ~ModelCell();

  void save(FIL* file);

  void setModelName(char* name);
  void setRfData(ModelData* model);

  void setModelId(uint8_t moduleIdx, uint8_t id);
  void setRfModuleData(uint8_t moduleIdx, ModuleData* modData);

  bool  fetchRfData();
  void  loadBitmap();
  const BitmapBuffer * getBuffer();
  void  resetBuffer();
};

class ModelsCategory: public std::list<ModelCell *>
{
public:
  char name[LEN_MODEL_FILENAME+1];

  ModelsCategory(const char * name);
  ~ModelsCategory();

  ModelCell * addModel(const char * name);
  void removeModel(ModelCell * model);
  void moveModel(ModelCell * model, int8_t step);
  void save(FIL * file);
};

class ModelsList
{
  bool loaded;
  std::list<ModelsCategory *> categories;
  ModelsCategory * currentCategory;
  ModelCell * currentModel;
  unsigned int modelsCount;

  void init();

public:

  ModelsList();
  ~ModelsList();

  bool load();
  void save();
  void clear();

  const std::list<ModelsCategory *>& getCategories() const {
    return categories;
  }
  
  void setCurrentCategorie(ModelsCategory* cat);
  ModelsCategory* getCurrentCategory() const {
    return currentCategory;
  }

  void setCurrentModel(ModelCell* cell);
  ModelCell* getCurrentModel() const {
    return currentModel;
  }

  unsigned int getModelsCount() const {
    return modelsCount;
  }
  
  bool readNextLine(char * line, int maxlen);

  ModelsCategory * createCategory();
  void removeCategory(ModelsCategory * category);

  ModelCell * addModel(ModelsCategory * category, const char * name);
  void removeModel(ModelsCategory * category, ModelCell * model);
  void moveModel(ModelsCategory * category, ModelCell * model, int8_t step);
  void moveModel(ModelCell * model, ModelsCategory * previous_category, ModelsCategory * new_category);

  bool isModelIdUnique(uint8_t moduleIdx, char* warn_buf, size_t warn_buf_len);
  uint8_t findNextUnusedModelId(uint8_t moduleIdx);

  void onNewModelCreated(ModelCell* cell, ModelData* model);

protected:
  FIL file;

};

extern ModelsList modelslist;

#endif // _MODELSLIST_H_
