#pragma once

#include "LibraryManager.hpp"
#include "SettingsManager.hpp"
#include "Types.hpp"
#include "platform/IScreenshotProvider.hpp"
#include "platform/SafeImageWriter.hpp"
#include "platform/ScreenGeometryManager.hpp"
#include "platform/X11FallbackProvider.hpp"
#include "platform/XdgPortalProvider.hpp"

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
  ~CaptureEngine() override;

  bool isCapturing() const;
  QString lastCapturedFilePath() const;
  QString frozenFramePath() const;
  bool hasLastRegion() const;
  QImage lastCapturedImage() const;

  // Accessors
  ScreenGeometryManager *geometryManager() const;
  SafeImageWriter *imageWriter() const;

  // Q_INVOKABLE Capture triggers
  Q_INVOKABLE void requestRegionCapture(int delaySeconds = 0);
  Q_INVOKABLE void requestFullscreenCapture(int delaySeconds = 0);
  Q_INVOKABLE void requestWindowCapture(int delaySeconds = 0);
  Q_INVOKABLE void requestLastRegionCapture(int delaySeconds = 0);
  Q_INVOKABLE void requestRegionCaptureWithAction(int delaySeconds,
                                                  const QString &action);
  Q_INVOKABLE void requestFullscreenCaptureWithAction(int delaySeconds,
                                                      const QString &action);
  Q_INVOKABLE void requestWindowCaptureWithAction(int delaySeconds,
                                                  const QString &action);
  Q_INVOKABLE void requestLastRegionCaptureWithAction(int delaySeconds,
                                                      const QString &action);

  // Region selection from QML Sniper Overlay
  Q_INVOKABLE void processRegionSelected(int x, int y, int width, int height,
                                         const QString &action = QString());
  Q_INVOKABLE void cancelCapture();
  Q_INVOKABLE bool copyImageToClipboard(const QString &filePath);
  Q_INVOKABLE bool saveImageAs(const QString &sourcePath,
                               const QString &destinationPath);

  // Color picker methods (Hex, RGB, HSL)
  Q_INVOKABLE QString colorAt(int x, int y) const;
  Q_INVOKABLE QString colorAtFormat(int x, int y, const QString &format) const;
  Q_INVOKABLE bool copyColorAt(int x, int y);
  Q_INVOKABLE bool copyColorAtFormat(int x, int y, const QString &format);

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
  void captureErrorCode(const QString &message, CaptureErrorCode code);
  void captureCancelled();
  void captureProgress(int percent, const QString &statusText);
  void colorCopied(const QString &colorString);
  void captureUiShouldHide();
  void captureUiMayRestore();

private slots:
  void handleProviderCaptureReady(const QImage &image, const QRect &sourceRect);
  void handleProviderCaptureFailed(const QString &errorMessage,
                                   CaptureErrorCode errorCode);
  void handleProviderCaptureCancelled();
  void handleScreenTopologyChanged();

private:
  IScreenshotProvider *activeProvider() const;
  void startCaptureWorkflow(CaptureMode mode, int delaySeconds,
                            const QString &action, bool isLastRegion);
  void executeCapture(CaptureMode mode);
  void failCapture(const QString &message,
                   CaptureErrorCode code = CaptureErrorCode::Unknown);
  bool saveAndProcessResult(const QImage &image, CaptureMode mode,
                            const QRect &sourceRect,
                            const QString &action = QString());

  SettingsManager *m_settings{nullptr};
  LibraryManager *m_library{nullptr};
  ScreenGeometryManager *m_geometryManager{nullptr};
  SafeImageWriter *m_imageWriter{nullptr};
  XdgPortalProvider *m_portalProvider{nullptr};
  X11FallbackProvider *m_x11Provider{nullptr};

  bool m_isCapturing{false};
  QString m_lastCapturedFilePath;
  QString m_frozenFramePath;
  QImage m_cachedDesktopFrame;
  CaptureMode m_pendingMode{CaptureMode::Region};
  QString m_pendingAction;
  bool m_pendingLastRegion{false};
};

} // namespace ro_screenshot
