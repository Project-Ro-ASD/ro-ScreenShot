#include "DBusService.hpp"
#include <QCoreApplication>
#include <QDBusConnectionInterface>

namespace ro_screenshot {

DBusAdaptor::DBusAdaptor(CaptureEngine *engine, QObject *parent)
    : QDBusAbstractAdaptor(parent), m_engine(engine) {
  setAutoRelaySignals(true);
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

  const QString serviceName = "org.ro_asd.ScreenShot";
  const QString objectPath = "/org/ro_asd/ScreenShot";

  if (!bus.registerService(serviceName)) {
    return false;
  }

  if (!bus.registerObject(objectPath, this)) {
    bus.unregisterService(serviceName);
    return false;
  }

  return true;
}

} // namespace ro_screenshot
