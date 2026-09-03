#pragma once

#include "CaptureEngine.hpp"
#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QObject>
#include <QVariantMap>

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
  Q_NOREPLY void CaptureLastRegion(int delaySeconds = 0);
  Q_NOREPLY void CaptureRegionWithAction(int delaySeconds,
                                         const QString &action);
  Q_NOREPLY void CaptureFullscreenWithAction(int delaySeconds,
                                             const QString &action);
  Q_NOREPLY void CaptureWindowWithAction(int delaySeconds,
                                         const QString &action);
  Q_NOREPLY void CaptureLastRegionWithAction(int delaySeconds,
                                             const QString &action);
  Q_NOREPLY void CaptureWithOptions(const QVariantMap &options);
  Q_NOREPLY void CancelCapture();
  Q_NOREPLY void OpenLibrary();
  Q_NOREPLY void OpenSettings();
  Q_NOREPLY void Quit();

  bool IsCapturing() const;
  QString GetLastCapturedFilePath() const;
  QString GetColorAt(int x, int y, const QString &format = "HEX") const;

signals:
  void CaptureCompleted(const QString &filePath, const QString &fileName,
                        bool savedToDisk, bool copiedToClipboard);
  void CaptureFailed(const QString &errorMessage, int errorCode);
  void CaptureCancelled();
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
  void unregisterService();
  bool isRegistered() const;

signals:
  void openLibraryRequested();
  void openSettingsRequested();
  void quitRequested();

private:
  CaptureEngine *m_engine{nullptr};
  DBusAdaptor *m_adaptor{nullptr};
  bool m_isRegistered{false};
};

} // namespace ro_screenshot
