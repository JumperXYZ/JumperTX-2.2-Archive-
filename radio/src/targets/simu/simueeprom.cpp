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

#include "opentx.h"

const char * eepromFile = NULL;
FILE * fp = NULL;

uint32_t eeprom_pointer;
uint8_t * eeprom_buffer_data;
volatile int32_t eeprom_buffer_size;
bool eeprom_read_operation;

bool eeprom_thread_running = false;

#if defined(EEPROM_SIZE)
uint8_t eeprom[EEPROM_SIZE];
#else
uint8_t * eeprom = NULL;
#endif

sem_t * eeprom_write_sem;

void eepromReadBlock (uint8_t * buffer, size_t address, size_t size)
{
  assert(size);

  if (fp) {
    // TRACE("EEPROM read (pos=%d, size=%d)", pointer_eeprom, size);
    if (fseek(fp, address, SEEK_SET) < 0)
      perror("error in fseek");
    if (fread(buffer, size, 1, fp) <= 0)
      perror("error in fread");
  }
  else {
    memcpy(buffer, &eeprom[address], size);
  }
}

void eepromSimuWriteBlock(uint8_t * buffer, size_t address, size_t size)
{
  assert(size);

  if (fp) {
    // TRACE("EEPROM write (pos=%d, size=%d)", pointer_eeprom, size);
    if (fseek(fp, address, SEEK_SET) < 0)
      perror("error in fseek");
    if (fwrite(buffer, size, 1, fp) <= 0)
      perror("error in fwrite");
  }
  else {
    memcpy(&eeprom[address], buffer, size);
  }
}

volatile uint8_t eepromTransferComplete = 1;
void * eeprom_thread_function(void *)
{
  while (!sem_wait(eeprom_write_sem)) {
    if (!eeprom_thread_running)
      return NULL;
    assert(eeprom_buffer_size);
    if (eeprom_read_operation) {
      eepromReadBlock(eeprom_buffer_data, eeprom_pointer, eeprom_buffer_size);
    }
    else {
      eepromSimuWriteBlock(eeprom_buffer_data, eeprom_pointer, eeprom_buffer_size);
    }
    eepromTransferComplete = 1;
  }
  return 0;
}

uint8_t eepromReadStatus()
{
  return 1;
}

uint8_t eepromIsTransferComplete()
{
  return eepromTransferComplete;
}

void eepromTransmitData(uint32_t address, uint8_t * buffer, uint32_t size, bool read)
{
  eeprom_pointer = address;
  eeprom_buffer_data = buffer;
  eeprom_buffer_size = size;
  eeprom_read_operation = read;
  eepromTransferComplete = 0;
  sem_post(eeprom_write_sem);
}

#if defined(EEPROM_BLOCK_SIZE)
void eepromBlockErase(uint32_t address)
{
  static uint8_t erasedBlock[EEPROM_BLOCK_SIZE]; // can't be on the stack!
  memset(erasedBlock, 0xff, sizeof(erasedBlock));
  eepromTransmitData(address, erasedBlock, EEPROM_BLOCK_SIZE, false);
}
#endif

void eepromStartRead(uint8_t * buffer, size_t address, size_t size)
{
  assert(size);
  eepromTransmitData(address, buffer, size, true);
}

void eepromStartWrite(uint8_t * buffer, size_t address, size_t size)
{
  assert(size);
  eepromTransmitData(address, buffer, size, false);
}

void eepromWriteBlock(uint8_t * buffer, size_t address, size_t size)
{
  eepromStartWrite(buffer, address, size);

  while (!eepromIsTransferComplete()) {
#if defined(GTESTS)
    sleep(0/*ms*/);
#else
    sleep(1/*ms*/);
#endif
  }
}

pthread_t eeprom_thread_pid;

void StartEepromThread(const char * filename)
{
  eepromFile = filename;
  if (eepromFile) {
    fp = fopen(eepromFile, "rb+");
    if (!fp)
      fp = fopen(eepromFile, "wb+");
    if (!fp)
      perror("error in fopen");
  }

#ifdef __APPLE__
  eeprom_write_sem = sem_open("eepromsem", O_CREAT, S_IRUSR | S_IWUSR, 0);
#else
  eeprom_write_sem = (sem_t *)malloc(sizeof(sem_t));
  sem_init(eeprom_write_sem, 0, 0);
#endif

  if (!pthread_create(&eeprom_thread_pid, NULL, &eeprom_thread_function, NULL))
    eeprom_thread_running = true;
  else
    perror("Could not create eeprom thread.");
}

void StopEepromThread()
{
  eeprom_thread_running = false;
  sem_post(eeprom_write_sem);
  pthread_join(eeprom_thread_pid, NULL);

#ifdef __APPLE__
  sem_close(eeprom_write_sem);
#else
  sem_destroy(eeprom_write_sem);
  free(eeprom_write_sem);
#endif

  if (fp)
    fclose(fp);
}
