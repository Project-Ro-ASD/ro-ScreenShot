#include "DBusService.hpp"
#include <QCoreApplication>
#include <QDBusConnectionInterface>

namespace ro_screenshot {

DBusAdaptor::DBusAdaptor(CaptureEngine *engine, QObject *parent)
    : QDBusAbstractAdaptor(parent), m_engine(engine) {
  setAutoRelaySignals(true);

  if (m_engine) {
    connect(m_engine, &CaptureEngine::captureSuccess, this,
            [this](const QString &filePath, const QString &fileName,
                   bool savedToDisk, bool copiedToClipboard) {
              emit CaptureCompleted(filePath, fileName, savedToDisk,
                                    copiedToClipboard);
            });
    connect(m_engine, &CaptureEngine::captureErrorCode, this,
            [this](const QString &message, CaptureErrorCode code) {
              emit CaptureFailed(message, static_cast<int>(code));
            });
    connect(m_engine, &CaptureEngine::captureCancelled, this,
            &DBusAdaptor::CaptureCancelled);
  }
}

void DBusAdaptor::CaptureRegion(int delaySeconds) {
  if (m_engine) {
    m_engine->requestRegionCapture(delaySeconds);
  }
}

void DBusAdaptor::CaptureFullscreen(int delaySeconds) {
  if (m_engine) {
    m_engine->requestFullscreenCapture(delaySeconds);
  }
}

void DBusAdaptor::CaptureWindow(int delaySeconds) {
  if (m_engine) {
    m_engine->requestWindowCapture(delaySeconds);
  }
}

void DBusAdaptor::CaptureLastRegion(int delaySeconds) {
  if (m_engine) {
    m_engine->requestLastRegionCapture(delaySeconds);
  }
}

void DBusAdaptor::CaptureRegionWithAction(int delaySeconds,
                                          const QString &action) {
  if (m_engine) {
    m_engine->requestRegionCaptureWithAction(delaySeconds, action);
  }
}

void DBusAdaptor::CaptureFullscreenWithAction(int delaySeconds,
                                              const QString &action) {
  if (m_engine) {
    m_engine->requestFullscreenCaptureWithAction(delaySeconds, action);
  }
}

void DBusAdaptor::CaptureWindowWithAction(int delaySeconds,
                                          const QString &action) {
  if (m_engine) {
    m_engine->requestWindowCaptureWithAction(delaySeconds, action);
  }
}

void DBusAdaptor::CaptureLastRegionWithAction(int delaySeconds,
                                              const QString &action) {
  if (m_engine) {
    m_engine->requestLastRegionCaptureWithAction(delaySeconds, action);
  }
}

void DBusAdaptor::CaptureWithOptions(const QVariantMap &options) {
  if (!m_engine) {
    return;
  }
  const QString modeStr =
      options.value(QStringLiteral("mode"), QStringLiteral("region"))
          .toString()
          .toLower();
  const int delay = options.value(QStringLiteral("delay"), 0).toInt();
  const QString action =
      options.value(QStringLiteral("action"), QString()).toString();

  if (modeStr == "fullscreen") {
    m_engine->requestFullscreenCaptureWithAction(delay, action);
  } else if (modeStr == "window") {
    m_engine->requestWindowCaptureWithAction(delay, action);
  } else if (modeStr == "last_region" || modeStr == "last-region") {
    m_engine->requestLastRegionCaptureWithAction(delay, action);
  } else {
    m_engine->requestRegionCaptureWithAction(delay, action);
  }
}

void DBusAdaptor::CancelCapture() {
  if (m_engine) {
    m_engine->cancelCapture();
  }
}

bool DBusAdaptor::IsCapturing() const {
  return m_engine && m_engine->isCapturing();
}

QString DBusAdaptor::GetLastCapturedFilePath() const {
  return m_engine ? m_engine->lastCapturedFilePath() : QString();
}

QString DBusAdaptor::GetColorAt(int x, int y, const QString &format) const {
  return m_engine ? m_engine->colorAtFormat(x, y, format) : QString();
}

void DBusAdaptor::OpenLibrary() { emit openLibraryRequested(); }

void DBusAdaptor::OpenSettings() { emit openSettingsRequested(); }

void DBusAdaptor::Quit() { emit quitRequested(); }

DBusService::DBusService(CaptureEngine *engine, QObject *parent)
    : QObject(parent), m_engine(engine),
      m_adaptor(new DBusAdaptor(engine, this)) {
  connect(m_adaptor, &DBusAdaptor::openLibraryRequested, this,
          &DBusService::openLibraryRequested);
  connect(m_adaptor, &DBusAdaptor::openSettingsRequested, this,
          &DBusService::openSettingsRequested);
  connect(m_adaptor, &DBusAdaptor::quitRequested, this,
          &DBusService::quitRequested);
}

bool DBusService::registerService() {
  QDBusConnection bus = QDBusConnection::sessionBus();
  if (!bus.isConnected()) {
    return false;
  }

  const QString serviceName = QStringLiteral("org.ro_asd.ScreenShot");
  const QString objectPath = QStringLiteral("/org/ro_asd/ScreenShot");

  if (!bus.registerService(serviceName)) {
    return false;
  }

  if (!bus.registerObject(objectPath, this)) {
    bus.unregisterService(serviceName);
    return false;
  }

  m_isRegistered = true;
  return true;
}

void DBusService::unregisterService() {
  if (m_isRegistered) {
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterObject(QStringLiteral("/org/ro_asd/ScreenShot"));
    bus.unregisterService(QStringLiteral("org.ro_asd.ScreenShot"));
    m_isRegistered = false;
  }
}

bool DBusService::isRegistered() const { return m_isRegistered; }

} // namespace ro_screenshot
