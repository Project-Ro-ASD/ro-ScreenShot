#pragma once

#include "IScreenshotProvider.hpp"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QObject>
#include <QTimer>

namespace ro_screenshot {

class XdgPortalProvider : public IScreenshotProvider {
  Q_OBJECT

public:
  explicit XdgPortalProvider(QObject *parent = nullptr);
  ~XdgPortalProvider() override;

  QString name() const override { return QStringLiteral("XdgPortal"); }
  bool isAvailable() const override;
  void capture(CaptureMode mode, const QVariantMap &options = {}) override;
  void cancel() override;

  // Configuration
  void setParentWindowHandle(const QString &handle);
  QString parentWindowHandle() const;
  void setTimeoutDurationMs(int ms);

private slots:
  void handlePortalResponse(uint response, const QVariantMap &results);
  void handleTimeout();

private:
  QString expectedPortalRequestPath(const QString &token) const;
  bool connectPortalResponse(const QString &path);
  void disconnectPortalResponse();
  void cleanupPendingCall();

  bool m_isCapturing{false};
  CaptureMode m_currentMode{CaptureMode::Region};
  QString m_pendingPortalRequestPath;
  QString m_parentWindowHandle;
  QDBusPendingCallWatcher *m_activeWatcher{nullptr};
  QTimer *m_timeoutTimer{nullptr};
  int m_timeoutDurationMs{120000}; // 120s timeout
};

} // namespace ro_screenshot
