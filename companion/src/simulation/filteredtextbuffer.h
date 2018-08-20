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

#ifndef FILTEREDTEXTBUFFER_H
#define FILTEREDTEXTBUFFER_H

#include <QBuffer>
#include <QRegularExpression>
#include <QReadWriteLock>
#include <QTimer>

/*
 * FIFOBufferDevice implements a thread-safe, asynchronous, buffered, FIFO I/O device based on QBuffer (which is a QIODevice).
 * Data is removed from the beginning after each read operation (read(), readAll(), readLine(), etc).
 * Also unlike QBuffer, it will NOT grow in an unlmited fashion. Even if never read, size is constrained to dataBufferMaxSize.
 * Default maximum buffer size 20KB.
 */
class FIFOBufferDevice : public QBuffer
{
  Q_OBJECT

  public:
    explicit FIFOBufferDevice(QObject * parent = Q_NULLPTR);

    qint64 getDataBufferMaxSize() const;
    void setDataBufferMaxSize(qint64 size);

    // reimplemented from QIODevice for efficiency
    qint64 readLine(char * data, qint64 maxSize);
    QByteArray readLine(qint64 maxSize = 0);

  signals:
    void bufferOverflow(qint64 len);  // len is overflow size, <= 0 means overflow has cleared

  protected:
    qint64 trimData(qint64 len);
    virtual qint64 writeData(const char * data, qint64 len);
    virtual qint64 readData(char * data, qint64 len);

    QReadWriteLock m_dataRWLock;
    qint64 m_dataBufferMaxSize;     // [bytes] output buffer limit (FIFO).
    bool m_hasOverflow;
};


/*
 * FilteredTextBuffer implements a FIFOBufferDevice which can, optionally, have a line filter applied to it.
 * If no filter is set, it acts exactly like its parent class.
 * If a filter is set, incoming data is buffered until a full line (\n terminated) is available. The line is then
 *   compared against the filter, and either added to the normal output buffer or discarded.
 * If a full line is not found after a specified timeout (1500ms by default) then any data in the input buffer is
 *   flushed anyway.  The same is true if the input buffer overflows (max. 5KB by default).
 * Another FIFOBufferDevice is used as the input buffer.
 */
class FilteredTextBuffer : public FIFOBufferDevice
{
  Q_OBJECT

  public:
    explicit FilteredTextBuffer(QObject * parent = Q_NULLPTR);
    ~FilteredTextBuffer();

    qint64 getInputBufferMaxSize() const;
    quint32 getInputBufferTimeout() const;

  public slots:
    // input buffer will be flushed if grows > size bytes.
    void setInputBufferMaxSize(qint64 size);
    // how often to flush the input buffer when less than whole lines are present.
    void setInputBufferTimeout(quint32 ms);
    void setLineFilterExpr(const QRegularExpression & expr);
    void setLineFilterEnabled(bool enable);
    void setLineFilterExclusive(bool exclusive);
    void setLineFilter(bool enable, bool exclusive, const QRegularExpression & expr);

  signals:
    // these are used internally to toggle the input buffer timeout timer (we use signals for thread safety)
    void timerStop();
    void timerStart();

  protected:
    qint64 writeDataSuper(const char * data, qint64 len = -1);
    virtual qint64 writeData(const char * data, qint64 len);
    void flushInputBuffer();
    void closeInputBuffer();
    bool openInputBuffer();
    void processInputBuffer();
    void onInputBufferWrite(qint64);
    void onInputBufferOverflow(const qint64 len);

    FIFOBufferDevice * m_inBuffer;
    QTimer * m_bufferFlushTimer;
    QRegularExpression m_lineFilter;
    qint64 m_inBuffMaxSize;          // [bytes]
    quint32 m_inBuffFlushTimeout;    // [ms]
    bool m_lineFilterEnable;
    bool m_lineFilterExclusive;
};


#endif // FILTEREDTEXTBUFFER_H
