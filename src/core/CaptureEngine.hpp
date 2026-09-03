#pragma once

#include "LibraryManager.hpp"
#include "SettingsManager.hpp"
#include "Types.hpp"
#include <QDBusObjectPath>
#include <QDateTime>
#include <QImage>
#include <QObject>
#include <QRect>
#include <QString>
#include <QTimer>

namespace ro_screenshot {

class CaptureEngine : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool isCapturing READ isCapturing NOTIFY isCapturingChanged)
  Q_PROPERTY(QString lastCapturedFilePath READ lastCapturedFilePath NOTIFY
                 lastCaptureChanged)
  Q_PROPERTY(
      QString frozenFramePath READ frozenFramePath NOTIFY frozenFrameChanged)
  Q_PROPERTY(bool hasLastRegion READ hasLastRegion NOTIFY lastRegionChanged)

public:
  explicit CaptureEngine(SettingsManager *settings, LibraryManager *library,
                         QObject *parent = nullptr);
  ~CaptureEngine() override = default;

  bool isCapturing() const;
  QString lastCapturedFilePath() const;
  QString frozenFramePath() const;
  bool hasLastRegion() const;

  Q_INVOKABLE void requestRegionCapture(int delaySeconds = 0);
  Q_INVOKABLE void requestFullscreenCapture(int delaySeconds = 0);
  Q_INVOKABLE void requestWindowCapture(int delaySeconds = 0);
  Q_INVOKABLE void requestLastRegionCapture(int delaySeconds = 0);

  Q_INVOKABLE void processRegionSelected(int x, int y, int width, int height,
                                         const QString &action = QString());
  Q_INVOKABLE void cancelCapture();
  Q_INVOKABLE bool copyImageToClipboard(const QString &filePath);
  Q_INVOKABLE bool saveImageAs(const QString &sourcePath,
                               const QString &destinationPath);

signals:
  void isCapturingChanged();
  void lastCaptureChanged();
  void frozenFrameChanged();
  void lastRegionChanged();
  void openSniperOverlay(const QString &framePath, int screenWidth,
                         int screenHeight);
  void closeSniperOverlay();
  void captureSuccess(const QString &filePath, const QString &fileName,
                      bool savedToDisk, bool copiedToClipboard);
  void captureError(const QString &message);
  void captureUiShouldHide();
  void captureUiMayRestore();

private slots:
  void executeRegionCapture();
  void executeFullscreenCapture();
  void executeWindowCapture();
  void handlePortalResponse(uint response, const QVariantMap &results);

private:
  QImage captureCombinedDesktop() const;
  bool shouldUsePortal() const;
  void requestPortalCapture(CaptureMode mode);
  QString expectedPortalRequestPath(const QString &token) const;
  bool connectPortalResponse(const QString &path);
  void disconnectPortalResponse();
  void failCapture(const QString &message);
  bool saveAndProcessResult(const QImage &image, CaptureMode mode,
                            const QRect &sourceRect,
                            const QString &action = QString());

  SettingsManager *m_settings{nullptr};
  LibraryManager *m_library{nullptr};
  bool m_isCapturing{false};
  QString m_lastCapturedFilePath;
  QString m_frozenFramePath;
  QImage m_cachedDesktopFrame;
  CaptureMode m_pendingPortalMode{CaptureMode::Region};
  bool m_pendingLastRegion{false};
  QString m_pendingPortalRequestPath;
};

} // namespace ro_screenshot
