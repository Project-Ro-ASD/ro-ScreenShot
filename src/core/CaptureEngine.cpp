#include "CaptureEngine.hpp"
#include "DesktopFeedback.hpp"
#include <QClipboard>
#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QScreen>
#include <QStandardPaths>
#include <algorithm>

namespace ro_screenshot {

CaptureEngine::CaptureEngine(SettingsManager *settings, LibraryManager *library,
                             QObject *parent)
    : QObject(parent), m_settings(settings), m_library(library),
      m_geometryManager(new ScreenGeometryManager(this)),
      m_imageWriter(new SafeImageWriter(this)),
      m_wlrProvider(new WlrScreencopyProvider(this)),
      m_portalProvider(new XdgPortalProvider(this)),
      m_x11Provider(new X11FallbackProvider(m_geometryManager, this)) {

  m_frozenFramePath = m_imageWriter->createTempFilePath(
      QStringLiteral("frozen_frame"), QStringLiteral(".png"));

  if (m_settings) {
    connect(m_settings, &SettingsManager::lastRegionChanged, this,
            &CaptureEngine::lastRegionChanged);
  }

  // Connect Wlr Provider signals
  connect(m_wlrProvider, &WlrScreencopyProvider::captureReady, this,
          &CaptureEngine::handleProviderCaptureReady);
  connect(m_wlrProvider, &WlrScreencopyProvider::captureFailed, this,
          &CaptureEngine::handleProviderCaptureFailed);
  connect(m_wlrProvider, &WlrScreencopyProvider::captureCancelled, this,
          &CaptureEngine::handleProviderCaptureCancelled);

  // Connect Portal Provider signals
  connect(m_portalProvider, &IScreenshotProvider::captureReady, this,
          &CaptureEngine::handleProviderCaptureReady);
  connect(m_portalProvider, &IScreenshotProvider::captureFailed, this,
          &CaptureEngine::handleProviderCaptureFailed);
  connect(m_portalProvider, &IScreenshotProvider::captureCancelled, this,
          &CaptureEngine::handleProviderCaptureCancelled);

  // Connect X11 Provider signals
  connect(m_x11Provider, &IScreenshotProvider::captureReady, this,
          &CaptureEngine::handleProviderCaptureReady);
  connect(m_x11Provider, &IScreenshotProvider::captureFailed, this,
          &CaptureEngine::handleProviderCaptureFailed);
  connect(m_x11Provider, &IScreenshotProvider::captureCancelled, this,
          &CaptureEngine::handleProviderCaptureCancelled);

  // Connect Screen Topology changes
  connect(m_geometryManager, &ScreenGeometryManager::screenTopologyChanged,
          this, &CaptureEngine::handleScreenTopologyChanged);
}

CaptureEngine::~CaptureEngine() {
  if (m_imageWriter) {
    m_imageWriter->cleanupTempFiles();
  }
}

bool CaptureEngine::isCapturing() const { return m_isCapturing; }

QString CaptureEngine::lastCapturedFilePath() const {
  return m_lastCapturedFilePath;
}

QString CaptureEngine::frozenFramePath() const { return m_frozenFramePath; }

QImage CaptureEngine::lastCapturedImage() const {
  return m_imageWriter ? m_imageWriter->cachedImage() : QImage();
}

ScreenGeometryManager *CaptureEngine::geometryManager() const {
  return m_geometryManager;
}

SafeImageWriter *CaptureEngine::imageWriter() const { return m_imageWriter; }

bool CaptureEngine::hasLastRegion() const {
  if (!m_settings || !m_settings->lastRegion().isValid() ||
      !m_settings->lastRegionFrameSize().isValid()) {
    return false;
  }
  return m_geometryManager->isRegionValidInCurrentLayout(
      m_settings->lastRegion(), m_settings->lastRegionFrameSize());
}

void CaptureEngine::handleScreenTopologyChanged() {
  if (m_settings && m_settings->lastRegion().isValid()) {
    if (!m_geometryManager->isRegionValidInCurrentLayout(
            m_settings->lastRegion(), m_settings->lastRegionFrameSize())) {
      m_settings->setLastRegion({});
      emit lastRegionChanged();
    }
  }
}

IScreenshotProvider *CaptureEngine::activeProvider() const {
  const bool isWayland = QGuiApplication::platformName().contains(
      QStringLiteral("wayland"), Qt::CaseInsensitive);

  if (isWayland) {
    if (m_wlrProvider && m_wlrProvider->isAvailable()) {
      return m_wlrProvider;
    }
    if (m_portalProvider && m_portalProvider->isAvailable()) {
      return m_portalProvider;
    }
  }
  return m_x11Provider;
}

void CaptureEngine::requestRegionCapture(int delaySeconds) {
  requestRegionCaptureWithAction(delaySeconds, {});
}

void CaptureEngine::requestRegionCaptureWithAction(int delaySeconds,
                                                   const QString &action) {
  startCaptureWorkflow(CaptureMode::Region, delaySeconds, action, false);
}

void CaptureEngine::requestFullscreenCapture(int delaySeconds) {
  requestFullscreenCaptureWithAction(delaySeconds, {});
}

void CaptureEngine::requestFullscreenCaptureWithAction(int delaySeconds,
                                                       const QString &action) {
  startCaptureWorkflow(CaptureMode::Fullscreen, delaySeconds, action, false);
}

void CaptureEngine::requestWindowCapture(int delaySeconds) {
  requestWindowCaptureWithAction(delaySeconds, {});
}

void CaptureEngine::requestWindowCaptureWithAction(int delaySeconds,
                                                   const QString &action) {
  startCaptureWorkflow(CaptureMode::Window, delaySeconds, action, false);
}

void CaptureEngine::requestLastRegionCapture(int delaySeconds) {
  requestLastRegionCaptureWithAction(delaySeconds, {});
}

void CaptureEngine::requestLastRegionCaptureWithAction(int delaySeconds,
                                                       const QString &action) {
  if (!hasLastRegion()) {
    failCapture(
        tr("Önceki yakalama bölgesi mevcut ekran düzeninde geçerli değil."),
        CaptureErrorCode::InvalidImage);
    return;
  }
  startCaptureWorkflow(CaptureMode::Region, delaySeconds, action, true);
}

void CaptureEngine::requestMonitorCapture(int monitorIndex, int delaySeconds,
                                          const QString &action) {
  auto screens = QGuiApplication::screens();
  if (screens.isEmpty()) {
    failCapture(tr("Bağlı monitör bulunamadı."),
                CaptureErrorCode::InvalidImage);
    return;
  }

  m_pendingMonitorIndex =
      std::clamp(monitorIndex, 0, static_cast<int>(screens.size()) - 1);
  startCaptureWorkflow(CaptureMode::Fullscreen, delaySeconds, action, false);
}

void CaptureEngine::requestScrollingCapture(int delaySeconds) {
  // Region capture first to define scrolling container bounds
  requestRegionCapture(delaySeconds);
}

void CaptureEngine::startCaptureWorkflow(CaptureMode mode, int delaySeconds,
                                         const QString &action,
                                         bool isLastRegion) {
  if (m_isCapturing) {
    cancelCapture();
  }

  m_isCapturing = true;
  m_pendingMode = mode;
  m_pendingAction = action;
  m_pendingLastRegion = isLastRegion;

  emit isCapturingChanged();
  emit captureUiShouldHide();
  emit captureProgress(10, tr("Yakalama başlatılıyor..."));

  const int delayMs = delaySeconds > 0 ? (delaySeconds * 1000) : 150;
  QTimer::singleShot(delayMs, this, [this, mode]() { executeCapture(mode); });
}

void CaptureEngine::executeCapture(CaptureMode mode) {
  IScreenshotProvider *provider = activeProvider();
  if (!provider) {
    failCapture(tr("Kullanılabilir ekran görüntüsü sağlayıcısı bulunamadı."),
                CaptureErrorCode::PortalUnavailable);
    return;
  }

  emit captureProgress(30, tr("Ekran görüntüsü alınıyor..."));
  provider->capture(mode);
}

void CaptureEngine::handleProviderCaptureReady(const QImage &image,
                                               const QRect &sourceRect) {
  Q_UNUSED(sourceRect)

  if (image.isNull()) {
    failCapture(tr("Sağlayıcı geçersiz görsel döndürdü."),
                CaptureErrorCode::InvalidImage);
    return;
  }

  emit captureProgress(70, tr("Görsel işleniyor..."));

  // Check if specific monitor capture was requested
  if (m_pendingMonitorIndex >= 0) {
    auto screens = QGuiApplication::screens();
    if (m_pendingMonitorIndex < screens.size()) {
      QRect screenGeom = screens[m_pendingMonitorIndex]->geometry();
      QRect validGeom =
          m_geometryManager->sanitizeRegion(screenGeom, image.size());
      QImage monitorImg = image.copy(validGeom);
      m_pendingMonitorIndex = -1;
      saveAndProcessResult(monitorImg, CaptureMode::Fullscreen, validGeom,
                           m_pendingAction);
      m_isCapturing = false;
      m_pendingAction.clear();
      emit isCapturingChanged();
      emit captureUiMayRestore();
      return;
    }
    m_pendingMonitorIndex = -1;
  }

  if (m_pendingMode == CaptureMode::Region) {
    if (m_pendingLastRegion) {
      const QRect lastReg = m_settings->lastRegion();
      const QRect validReg =
          m_geometryManager->sanitizeRegion(lastReg, image.size());
      if (validReg.isEmpty()) {
        failCapture(tr("Önceki yakalama bölgesi geçersiz."),
                    CaptureErrorCode::InvalidImage);
        return;
      }
      const QImage cropped = image.copy(validReg);
      saveAndProcessResult(cropped, CaptureMode::Region, validReg,
                           m_pendingAction);
      m_isCapturing = false;
      m_pendingLastRegion = false;
      m_pendingAction.clear();
      emit isCapturingChanged();
      emit captureUiMayRestore();
      return;
    }

    m_cachedDesktopFrame = image;
    m_imageWriter->setCachedImage(image);

    // Save frozen frame to temporary file for QML Overlay
    m_imageWriter->writeImageAtomically(image, m_frozenFramePath,
                                        QStringLiteral("PNG"), -1);
    emit frozenFrameChanged();

    emit openSniperOverlay(m_frozenFramePath, image.width(), image.height());
    return;
  }

  // Fullscreen or Window
  m_cachedDesktopFrame = image;
  m_imageWriter->setCachedImage(image);
  saveAndProcessResult(image, m_pendingMode, image.rect(), m_pendingAction);

  m_isCapturing = false;
  m_pendingAction.clear();
  emit isCapturingChanged();
  emit captureUiMayRestore();
}

void CaptureEngine::handleProviderCaptureFailed(const QString &errorMessage,
                                                CaptureErrorCode errorCode) {
  failCapture(errorMessage, errorCode);
}

void CaptureEngine::handleProviderCaptureCancelled() { cancelCapture(); }

void CaptureEngine::failCapture(const QString &message, CaptureErrorCode code) {
  m_isCapturing = false;
  m_pendingLastRegion = false;
  m_pendingMonitorIndex = -1;
  m_pendingAction.clear();
  emit isCapturingChanged();
  emit captureError(message);
  emit captureErrorCode(message, code);
  emit captureUiMayRestore();
}

void CaptureEngine::processRegionSelected(int x, int y, int width, int height,
                                          const QString &action) {
  if (m_cachedDesktopFrame.isNull()) {
    m_isCapturing = false;
    emit isCapturingChanged();
    return;
  }

  QRect targetRect(x, y, width, height);
  targetRect = m_geometryManager->sanitizeRegion(targetRect,
                                                 m_cachedDesktopFrame.size());

  if (targetRect.width() <= 2 || targetRect.height() <= 2) {
    emit closeSniperOverlay();
    m_isCapturing = false;
    emit isCapturingChanged();
    emit captureUiMayRestore();
    return;
  }

  QImage cropped = m_cachedDesktopFrame.copy(targetRect);
  m_imageWriter->setCachedImage(cropped);

  if (m_settings) {
    m_settings->setLastRegionGeometry(targetRect, m_cachedDesktopFrame.size());
  }

  saveAndProcessResult(cropped, CaptureMode::Region, targetRect,
                       action.isEmpty() ? m_pendingAction : action);

  if (m_settings && m_settings->closeOverlayOnCapture()) {
    emit closeSniperOverlay();
  }

  m_isCapturing = false;
  m_pendingAction.clear();
  emit isCapturingChanged();
  emit captureUiMayRestore();
}

void CaptureEngine::processPolygonSelected(const QVariantList &points,
                                           const QString &action) {
  if (m_cachedDesktopFrame.isNull() || points.size() < 3) {
    cancelCapture();
    return;
  }

  QPolygonF polygon;
  for (const QVariant &ptVal : points) {
    QVariantMap ptMap = ptVal.toMap();
    polygon.append(QPointF(ptMap["x"].toDouble(), ptMap["y"].toDouble()));
  }

  QRectF bRect = polygon.boundingRect();
  QRect boundInt =
      bRect.toAlignedRect().intersected(m_cachedDesktopFrame.rect());

  if (boundInt.width() <= 4 || boundInt.height() <= 4) {
    cancelCapture();
    return;
  }

  QImage cropped(boundInt.size(), QImage::Format_ARGB32_Premultiplied);
  cropped.fill(Qt::transparent);

  QPainter painter(&cropped);
  painter.setRenderHint(QPainter::Antialiasing, true);

  QPainterPath path;
  path.addPolygon(polygon.translated(-boundInt.topLeft()));
  painter.setClipPath(path);
  painter.drawImage(QPoint(0, 0), m_cachedDesktopFrame, boundInt);
  painter.end();

  m_imageWriter->setCachedImage(cropped);
  saveAndProcessResult(cropped, CaptureMode::Region, boundInt,
                       action.isEmpty() ? m_pendingAction : action);

  if (m_settings && m_settings->closeOverlayOnCapture()) {
    emit closeSniperOverlay();
  }

  m_isCapturing = false;
  m_pendingAction.clear();
  emit isCapturingChanged();
  emit captureUiMayRestore();
}

void CaptureEngine::cancelCapture() {
  IScreenshotProvider *provider = activeProvider();
  if (provider) {
    provider->cancel();
  }

  emit closeSniperOverlay();
  m_isCapturing = false;
  m_pendingAction.clear();
  m_pendingMonitorIndex = -1;
  emit isCapturingChanged();
  emit captureCancelled();
  emit captureUiMayRestore();
}

bool CaptureEngine::saveAndProcessResult(const QImage &image,
                                         CaptureMode /*mode*/,
                                         const QRect & /*sourceRect*/,
                                         const QString &action) {
  if (image.isNull() || !m_settings) {
    return false;
  }

  bool copied = false;
  bool saved = false;
  QString finalPath;

  const bool shouldCopy =
      action == QStringLiteral("copy") ||
      (action.isEmpty() && m_settings->autoCopyToClipboard());
  const bool shouldSave = action == QStringLiteral("save") ||
                          (action.isEmpty() && m_settings->autoSaveToDisk());

  if (!shouldCopy && !shouldSave) {
    emit captureError(tr("En az bir çıktı seçilmelidir: Pano veya Disk."));
    emit captureErrorCode(tr("En az bir çıktı seçilmelidir: Pano veya Disk."),
                          CaptureErrorCode::PermissionDenied);
    return false;
  }

  if (shouldCopy) {
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setImage(image);
    copied = true;
  }

  if (shouldSave) {
    finalPath = m_settings->generateFullPath();
    QString fmt = m_settings->imageFormat().toUpper();
    int quality =
        (fmt == "JPEG" || fmt == "JPG") ? m_settings->jpegQuality() : -1;

    const SaveResult saveRes =
        m_imageWriter->writeImageAtomically(image, finalPath, fmt, quality);

    if (saveRes.success) {
      saved = true;
      m_lastCapturedFilePath = finalPath;
      emit lastCaptureChanged();

      if (m_library) {
        m_library->refresh();
      }
    } else {
      emit captureError(saveRes.errorMessage);
      emit captureErrorCode(saveRes.errorMessage, saveRes.errorCode);
    }
  }

  if (!saved && !copied) {
    return false;
  }

  QFileInfo info(finalPath);
  emit captureProgress(100, tr("Tamamlandı"));
  emit captureSuccess(finalPath, info.fileName(), saved, copied);
  return true;
}

bool CaptureEngine::copyImageToClipboard(const QString &filePath) {
  QImage img(filePath);
  if (!img.isNull()) {
    QGuiApplication::clipboard()->setImage(img);
    return true;
  }
  return false;
}

bool CaptureEngine::saveImageAs(const QString &sourcePath,
                                const QString &destinationPath) {
  const QImage image(sourcePath);
  const QString format = QFileInfo(destinationPath).suffix().toUpper();
  const SaveResult res = m_imageWriter->writeImageAtomically(
      image, destinationPath, format.isEmpty() ? QStringLiteral("PNG") : format,
      -1);
  return res.success;
}

QString CaptureEngine::colorAt(int x, int y) const {
  return colorAtFormat(x, y, QStringLiteral("HEX"));
}

QString CaptureEngine::colorAtFormat(int x, int y,
                                     const QString &format) const {
  if (m_cachedDesktopFrame.isNull() ||
      !m_cachedDesktopFrame.rect().contains(x, y)) {
    return {};
  }
  const QColor color = m_cachedDesktopFrame.pixelColor(x, y);
  const QString upperFmt = format.toUpper();

  if (upperFmt == "RGB") {
    return DesktopFeedback::formatColor(color, ColorFormat::Rgb);
  } else if (upperFmt == "HSL") {
    return DesktopFeedback::formatColor(color, ColorFormat::Hsl);
  }
  return DesktopFeedback::formatColor(color, ColorFormat::Hex);
}

bool CaptureEngine::copyColorAt(int x, int y) {
  return copyColorAtFormat(x, y, QStringLiteral("HEX"));
}

bool CaptureEngine::copyColorAtFormat(int x, int y, const QString &format) {
  const QString colorStr = colorAtFormat(x, y, format);
  if (colorStr.isEmpty()) {
    return false;
  }
  QGuiApplication::clipboard()->setText(colorStr);
  emit colorCopied(colorStr);
  return true;
}

} // namespace ro_screenshot
