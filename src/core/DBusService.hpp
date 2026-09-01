#pragma once

#include "CaptureEngine.hpp"
#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QObject>

namespace ro_screenshot {

class DBusAdaptor : public QDBusAbstractAdaptor {
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.ro_asd.ScreenShot")

public:
  explicit DBusAdaptor(CaptureEngine *engine, QObject *parent = nullptr);
  ~DBusAdaptor() override = default;

public slots:
  Q_NOREPLY void CaptureRegion(int delaySeconds = 0);
  Q_NOREPLY void CaptureFullscreen(int delaySeconds = 0);
  Q_NOREPLY void CaptureWindow(int delaySeconds = 0);
  Q_NOREPLY void OpenLibrary();
  Q_NOREPLY void OpenSettings();
  Q_NOREPLY void Quit();

signals:
  void openLibraryRequested();
  void openSettingsRequested();
  void quitRequested();

private:
  CaptureEngine *m_engine{nullptr};
};

class DBusService : public QObject {
  Q_OBJECT

public:
  explicit DBusService(CaptureEngine *engine, QObject *parent = nullptr);
  ~DBusService() override = default;

  bool registerService();

signals:
  void openLibraryRequested();
  void openSettingsRequested();
  void quitRequested();

private:
  CaptureEngine *m_engine{nullptr};
  DBusAdaptor *m_adaptor{nullptr};
};

} // namespace ro_screenshot
