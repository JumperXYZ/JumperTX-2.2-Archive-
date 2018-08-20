/*
 * Author - Bertrand Songis <bsongis@gmail.com>
 *
 * Based on th9x -> http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "opentxsimulator.h"
#include "opentx.h"
#include "simulcd.h"

#include <QDebug>
#include <QElapsedTimer>

#if !defined(MAX_LOGICAL_SWITCHES) && defined(NUM_CSW)
  #define MAX_LOGICAL_SWITCHES    NUM_CSW
#endif

#if defined(CPUARM)
  #define GET_SWITCH_BOOL(sw__)    getSwitch((sw__), 0);
#else
  #define GET_SWITCH_BOOL(sw__)    getSwitch(sw__);
#endif

#define OTXS_DBG    qDebug() << "(" << simuTimerMicros() << "us)"

int16_t g_anas[Analogs::NUM_ANALOGS];
QVector<QIODevice *> OpenTxSimulator::tracebackDevices;

uint16_t anaIn(uint8_t chan)
{
  return g_anas[chan];
}

uint16_t getAnalogValue(uint8_t index)
{
  return anaIn(index);
}

void firmwareTraceCb(const char * text)
{
  foreach (QIODevice * dev, OpenTxSimulator::tracebackDevices) {
    if (dev)
      dev->write(text);
  }
}

OpenTxSimulator::OpenTxSimulator() :
  SimulatorInterface(),
  m_timer10ms(NULL),
  m_resetOutputsData(true),
  m_stopRequested(false)
{
  tracebackDevices.clear();
  traceCallback = firmwareTraceCb;
}

OpenTxSimulator::~OpenTxSimulator()
{
  traceCallback = NULL;
  tracebackDevices.clear();

  if (m_timer10ms)
    delete m_timer10ms;

  if (isRunning()) {
    stop();
    QElapsedTimer tmout;
    tmout.start();
    while (isRunning() && !tmout.hasExpired(1000))
      ;
  }
  //qDebug() << "Deleting OpenTxSimulator";
}

QString OpenTxSimulator::name()
{
  return QString(SIMULATOR_FLAVOUR);
}

bool OpenTxSimulator::isRunning()
{
  QMutexLocker lckr(&m_mtxSimuMain);
  return (bool)main_thread_running;
}

void OpenTxSimulator::init()
{
  if (isRunning())
    return;
  OTXS_DBG;

  if (!m_timer10ms) {
    // make sure we create & control the timer from current thread
    m_timer10ms = new QTimer();
    m_timer10ms->setInterval(10);
    connect(m_timer10ms, &QTimer::timeout, this, &OpenTxSimulator::run);
    connect(this, SIGNAL(started()), m_timer10ms, SLOT(start()));
    connect(this, SIGNAL(stopped()), m_timer10ms, SLOT(stop()));
  }

  m_resetOutputsData = true;
  setStopRequested(false);

  QMutexLocker lckr(&m_mtxSimuMain);
  memset(g_anas, 0, sizeof(g_anas));
  simuInit();
}

void OpenTxSimulator::start(const char * filename, bool tests)
{
  if (isRunning())
    return;
  OTXS_DBG << "file:" << filename << "tests:" << tests;

  QMutexLocker lckr(&m_mtxSimuMain);
  QMutexLocker slckr(&m_mtxSettings);
  StartEepromThread(filename);
  StartAudioThread(volumeGain);
  StartSimu(tests, simuSdDirectory.toLatin1().constData(), simuSettingsDirectory.toLatin1().constData());

  emit started();
  QTimer::singleShot(0, this, SLOT(run()));  // old style for Qt < 5.4
}

void OpenTxSimulator::stop()
{
  if (!isRunning())
    return;
  OTXS_DBG;

  setStopRequested(true);

  QMutexLocker lckr(&m_mtxSimuMain);
  StopSimu();
  StopAudioThread();
  StopEepromThread();

  emit stopped();
}

void OpenTxSimulator::setSdPath(const QString & sdPath, const QString & settingsPath)
{
  QMutexLocker lckr(&m_mtxSettings);
  simuSdDirectory = sdPath;
  simuSettingsDirectory = settingsPath;
}

void OpenTxSimulator::setVolumeGain(const int value)
{
  QMutexLocker lckr(&m_mtxSettings);
  volumeGain = value;
}

void OpenTxSimulator::setRadioData(const QByteArray & data)
{
#if defined(EEPROM_SIZE)
  QMutexLocker lckr(&m_mtxRadioData);
  memcpy(eeprom, data.data(), qMin<int>(EEPROM_SIZE, data.size()));
#endif
}

void OpenTxSimulator::readRadioData(QByteArray & dest)
{
#if defined(EEPROM_SIZE)
  QMutexLocker lckr(&m_mtxRadioData);
  memcpy(dest.data(), eeprom, std::min<int>(EEPROM_SIZE, dest.size()));
#endif
}

uint8_t * OpenTxSimulator::getLcd()
{
  return (uint8_t *)simuLcdBuf;
}

void OpenTxSimulator::setAnalogValue(uint8_t index, int16_t value)
{
  static int dim = DIM(g_anas);
  if (index < dim)
    g_anas[index] = value;
}

void OpenTxSimulator::setSwitch(uint8_t swtch, int8_t state)
{
  simuSetSwitch(swtch, state);
}

void OpenTxSimulator::setKey(uint8_t key, bool state)
{
  simuSetKey(key, state);
}

void OpenTxSimulator::setTrimSwitch(uint8_t trim, bool state)
{
  simuSetTrim(trim, state);
}

void OpenTxSimulator::setTrim(unsigned int idx, int value)
{
  unsigned i = idx;
  if (i < 4)  // swap axes
    i = modn12x3[4 * getStickMode() + idx];
  uint8_t phase = getTrimFlightMode(getFlightMode(), i);

#ifdef CPUARM
  if (!setTrimValue(phase, i, value)) {
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, [=]() {
      emit trimValueChange(idx, 0);
      emit outputValueChange(OUTPUT_SRC_TRIM_VALUE, idx, 0);
      timer->deleteLater();
    });
    timer->start(350);
  }
#else
  setTrimValue(phase, i, value);
#endif
}

void OpenTxSimulator::setTrainerInput(unsigned int inputNumber, int16_t value)
{
  static unsigned dim = DIM(ppmInput);
  //setTrainerTimeout(100);
  if (inputNumber < dim)
    ppmInput[inputNumber] = qMin(qMax((int16_t)-512, value), (int16_t)512);
}

void OpenTxSimulator::setInputValue(int type, uint8_t index, int16_t value)
{
  //qDebug() << type << index << value << this->thread();
  switch (type) {
    case INPUT_SRC_ANALOG :
    case INPUT_SRC_STICK :
      setAnalogValue(index, value);
      break;
    case INPUT_SRC_KNOB :
      setAnalogValue(index + NUM_STICKS, value);
      break;
    case INPUT_SRC_SLIDER :
      setAnalogValue(index + NUM_STICKS + NUM_POTS, value);
      break;
    case INPUT_SRC_TXVIN :
      setAnalogValue(Analogs::TX_VOLTAGE, voltageToAdc(value));
      break;
    case INPUT_SRC_SWITCH :
      setSwitch(index, (int8_t)value);
      break;
    case INPUT_SRC_TRIM_SW :
      setTrimSwitch(index, (bool)value);
      break;
    case INPUT_SRC_TRIM :
      setTrim(index, value);
      break;
    case INPUT_SRC_KEY :
      setKey(index, (bool)value);
      break;
    case INPUT_SRC_TRAINER :
      setTrainerInput(index, value);
      break;
    case INPUT_SRC_ROTENC :  // TODO
    default:
      return;
  }
}

void OpenTxSimulator::rotaryEncoderEvent(int steps)
{
#if defined(ROTARY_ENCODER_NAVIGATION)
  ROTARY_ENCODER_NAVIGATION_VALUE += steps * ROTARY_ENCODER_GRANULARITY;
#else
  // TODO : this should probably be handled in the GUI
  int key;
#if defined(PCBXLITE)
  if (steps > 0)
    key = KEY_DOWN;
  else if (steps < 0)
    key = KEY_UP;
#else
  if (steps > 0)
    key = KEY_MINUS;
  else if (steps < 0)
    key = KEY_PLUS;
#endif
  else
    // Should not happen but Clang complains that key is unset otherwise
    return;

  setKey(key, 1);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
  QTimer::singleShot(10, [this, key]() { setKey(key, 0); });
#else
  QTimer *timer = new QTimer(this);
  timer->setSingleShot(true);
  connect(timer, &QTimer::timeout, [=]() {
    setKey(key, 0);
    timer->deleteLater();
  } );
  timer->start(10);
#endif
#endif  // defined(ROTARY_ENCODER_NAVIGATION)
}

void OpenTxSimulator::setTrainerTimeout(uint16_t ms)
{
  ppmInputValidityTimer = ms;
}

void OpenTxSimulator::sendTelemetry(const QByteArray data)
{
#if defined(TELEMETRY_FRSKY_SPORT)
  //OTXS_DBG << data;
  sportProcessTelemetryPacket((uint8_t *)data.constData());
#else
  Q_UNUSED(data)
#endif
}

uint8_t OpenTxSimulator::getSensorInstance(uint16_t id, uint8_t defaultValue)
{
#if defined(TELEMETRY_FRSKY_SPORT)
  for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
    if (isTelemetryFieldAvailable(i)) {
      TelemetrySensor * sensor = &g_model.telemetrySensors[i];
      if (sensor->id == id) {
        return sensor->instance;
      }
    }
  }
#else
  Q_UNUSED(id)
#endif
  return defaultValue;
}

uint16_t OpenTxSimulator::getSensorRatio(uint16_t id)
{
#if defined(TELEMETRY_FRSKY_SPORT)
  for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
    if (isTelemetryFieldAvailable(i)) {
      TelemetrySensor * sensor = &g_model.telemetrySensors[i];
      if (sensor->id == id) {
        return sensor->custom.ratio;
      }
    }
  }
#else
  Q_UNUSED(id)
#endif
  return 0;
}

const int OpenTxSimulator::getCapability(Capability cap)
{
  int ret = 0;
  switch(cap) {
    case CAP_LUA :
      #ifdef LUA
        ret = 1;
      #endif
      break;

    case CAP_ROTARY_ENC :
      #ifdef ROTARY_ENCODERS
        ret = ROTARY_ENCODERS;
      #endif
      break;

    case CAP_ROTARY_ENC_NAV :
      #ifdef ROTARY_ENCODER_NAVIGATION
        ret = 1;
      #endif
      break;

    case CAP_TELEM_FRSKY_SPORT :
      #ifdef TELEMETRY_FRSKY_SPORT
        ret = 1;
      #endif
      break;
  }
  return ret;
}

void OpenTxSimulator::setLuaStateReloadPermanentScripts()
{
#if defined(LUA)
  luaState = INTERPRETER_RELOAD_PERMANENT_SCRIPTS;
#endif
}

void OpenTxSimulator::addTracebackDevice(QIODevice * device)
{
  QMutexLocker lckr(&m_mtxTbDevices);
  if (device && !tracebackDevices.contains(device))
    tracebackDevices.append(device);
}

void OpenTxSimulator::removeTracebackDevice(QIODevice * device)
{
  if (device) {
    QMutexLocker lckr(&m_mtxTbDevices);
    // no QVector::removeAll() in Qt < 5.4
    int i = 0;
    foreach (QIODevice * d, tracebackDevices) {
      if (d == device)
        tracebackDevices.remove(i);
      ++i;
    }
  }
}


/*** Protected functions ***/

void OpenTxSimulator::run()
{
  static uint32_t loops = 0;
  static QElapsedTimer ts;

  if (!loops)
    ts.start();

  if (isStopRequested()) {
    return;
  }
  if (!isRunning()) {
    QString err(getError());
    emit runtimeError(err);
    emit stopped();
    return;
  }

  ++loops;

  per10ms();

  checkLcdChanged();

  if (!(loops % 5)) {
    checkOutputsChanged();
  }

  if (!(loops % (SIMULATOR_INTERFACE_HEARTBEAT_PERIOD / 10))) {
    emit heartbeat(loops, simuTimerMicros() / 1000);
  }
}

bool OpenTxSimulator::isStopRequested()
{
  QMutexLocker lckr(&m_mtxStopReq);
  return m_stopRequested;
}

void OpenTxSimulator::setStopRequested(bool stop)
{
  QMutexLocker lckr(&m_mtxStopReq);
  m_stopRequested = stop;
}

bool OpenTxSimulator::checkLcdChanged()
{
  if (simuLcdRefresh) {
    simuLcdRefresh = false;
    emit lcdChange(isBacklightEnabled());
    return true;
  }
  return false;
}

void OpenTxSimulator::checkOutputsChanged()
{
  static TxOutputs lastOutputs;
  static size_t chansDim = DIM(channelOutputs);
  const static int16_t limit = 512 * 2;
  qint32 tmpVal;
  uint8_t i, idx;
  const uint8_t phase = getFlightMode();  // opentx.cpp
  const uint8_t mode = getStickMode();

  for (i=0; i < chansDim; i++) {
    if (lastOutputs.chans[i] != channelOutputs[i] || m_resetOutputsData) {
      emit channelOutValueChange(i, channelOutputs[i], (g_model.extendedLimits ? limit * LIMIT_EXT_PERCENT / 100 : limit));
      emit channelMixValueChange(i, ex_chans[i], limit * 2);
      emit outputValueChange(OUTPUT_SRC_CHAN_OUT, i, channelOutputs[i]);
      emit outputValueChange(OUTPUT_SRC_CHAN_MIX, i, ex_chans[i]);
      lastOutputs.chans[i] = channelOutputs[i];
    }
  }

  for (i=0; i < MAX_LOGICAL_SWITCHES; i++) {
    tmpVal = (qint32)GET_SWITCH_BOOL(SWSRC_SW1+i);
    if (lastOutputs.vsw[i] != (bool)tmpVal || m_resetOutputsData) {
      emit virtualSwValueChange(i, tmpVal);
      emit outputValueChange(OUTPUT_SRC_VIRTUAL_SW, i, tmpVal);
      lastOutputs.vsw[i] = tmpVal;
    }
  }

  for (i=0; i < Board::TRIM_AXIS_COUNT; i++) {
    if (i < 4)  // swap axes
      idx = modn12x3[4 * mode + i];
    else
      idx = i;

    tmpVal = getTrimValue(getTrimFlightMode(phase, idx), idx);
    if (lastOutputs.trims[i] != tmpVal || m_resetOutputsData) {
      emit trimValueChange(i, tmpVal);
      emit outputValueChange(OUTPUT_SRC_TRIM_VALUE, i, tmpVal);
      lastOutputs.trims[i] = tmpVal;
    }
  }

  tmpVal = g_model.extendedTrims ? TRIM_EXTENDED_MAX : TRIM_MAX;
  if (lastOutputs.trimRange != tmpVal || m_resetOutputsData) {
    emit trimRangeChange(Board::TRIM_AXIS_COUNT, -tmpVal, tmpVal);
    emit outputValueChange(OUTPUT_SRC_TRIM_RANGE, Board::TRIM_AXIS_COUNT, tmpVal);
    lastOutputs.trimRange = tmpVal;
  }

  if (lastOutputs.phase != phase || m_resetOutputsData) {
    emit phaseChanged(phase, getCurrentPhaseName());
    emit outputValueChange(OUTPUT_SRC_PHASE, 0, qint16(phase));
    lastOutputs.phase = phase;
  }

#if defined(GVAR_VALUE) && defined(GVARS)
  gVarMode_t gvar;
  for (uint8_t gv=0; gv < MAX_GVARS; gv++) {
#if !defined(PCBSTD)
    gvar.prec = g_model.gvars[gv].prec;
    gvar.unit = g_model.gvars[gv].unit;
#endif
    for (uint8_t fm=0; fm < MAX_FLIGHT_MODES; fm++) {
      gvar.mode = fm;
      gvar.value = (int16_t)GVAR_VALUE(gv, getGVarFlightMode(fm, gv));
      tmpVal = gvar;
      if (lastOutputs.gvars[fm][gv] != tmpVal || m_resetOutputsData) {
        lastOutputs.gvars[fm][gv] = tmpVal;
        emit gVarValueChange(gv, tmpVal);
        emit outputValueChange(OUTPUT_SRC_GVAR, gv, tmpVal);
      }
    }
  }
#endif

  m_resetOutputsData = false;
}

uint8_t OpenTxSimulator::getStickMode()
{
  return limit<uint8_t>(0, g_eeGeneral.stickMode, 3);
}

const char * OpenTxSimulator::getPhaseName(unsigned int phase)
{
  static char buff[sizeof(g_model.flightModeData[0].name)+1];
  zchar2str(buff, g_model.flightModeData[phase].name, sizeof(g_model.flightModeData[0].name));
  return buff;
}

const QString OpenTxSimulator::getCurrentPhaseName()
{
  unsigned phase = getFlightMode();
  QString name(getPhaseName(phase));
  if (name.isEmpty())
    name = QString::number(phase);
  return name;
}

const char * OpenTxSimulator::getError()
{
  return main_thread_error;
}

const int OpenTxSimulator::voltageToAdc(const int volts)
{
  int ret = 0;
#if defined(PCBHORUS) || defined(PCBX7)
  ret = (float)volts * 16.2f;
#elif defined(PCBTARANIS) || defined(PCBSKY9X)
  ret = (float)volts * 13.3f;
#elif defined(PCBGRUVIN9X)
  ret = (float)volts * 1.63f;
#else
  ret = (float)volts * 14.15f;
#endif
  return ret;
}


/*
 * OpenTxSimulatorFactory
 */

class OpenTxSimulatorFactory: public SimulatorFactory
{
  public:
    OpenTxSimulatorFactory()
    {
    }

    virtual SimulatorInterface * create()
    {
      return new OpenTxSimulator();
    }

    virtual QString name()
    {
      return QString(SIMULATOR_FLAVOUR);
    }

    virtual Board::Type type()
    {
#if defined(PCBX12S)
      return Board::BOARD_X12S;
#elif defined(PCBX10)
      return Board::BOARD_X10;
#elif defined(PCBX7)
      return Board::BOARD_TARANIS_X7;
#elif defined(PCBTARANIS)
      return Board::BOARD_TARANIS_X9D;
#else
      return Board::BOARD_STOCK;
#endif
    }
};

extern "C" DLLEXPORT SimulatorFactory * registerSimu()
{
  return new OpenTxSimulatorFactory();
}
