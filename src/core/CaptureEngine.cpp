#include "CaptureEngine.hpp"
#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QPainter>
#include <QPixmap>
#include <QScreen>
#include <QStandardPaths>

namespace ro_screenshot {

CaptureEngine::CaptureEngine(SettingsManager *settings, LibraryManager *library,
                             QObject *parent)
    : QObject(parent), m_settings(settings), m_library(library) {
  QString cacheBase =
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
  if (cacheBase.isEmpty()) {
    cacheBase = QDir::homePath() + "/.cache/ro-asd/ro-screenshot";
  }
  QDir().mkpath(cacheBase);
  m_frozenFramePath = cacheBase + "/frozen_frame.png";
}

bool CaptureEngine::isCapturing() const { return m_isCapturing; }

QString CaptureEngine::lastCapturedFilePath() const {
  return m_lastCapturedFilePath;
}

QString CaptureEngine::frozenFramePath() const { return m_frozenFramePath; }

void CaptureEngine::requestRegionCapture(int delaySeconds) {
  if (m_isCapturing) {
    return;
  }
  m_isCapturing = true;
  emit isCapturingChanged();

  if (delaySeconds > 0) {
    QTimer::singleShot(delaySeconds * 1000, this,
                       &CaptureEngine::executeRegionCapture);
  } else {
    // 150ms delay to allow any menus to settle
    QTimer::singleShot(150, this, &CaptureEngine::executeRegionCapture);
  }
}

void CaptureEngine::requestFullscreenCapture(int delaySeconds) {
  if (m_isCapturing) {
    return;
  }
  m_isCapturing = true;
  emit isCapturingChanged();

  if (delaySeconds > 0) {
    QTimer::singleShot(delaySeconds * 1000, this,
                       &CaptureEngine::executeFullscreenCapture);
  } else {
    QTimer::singleShot(150, this, &CaptureEngine::executeFullscreenCapture);
  }
}

void CaptureEngine::requestWindowCapture(int delaySeconds) {
  if (m_isCapturing) {
    return;
  }
  m_isCapturing = true;
  emit isCapturingChanged();

  if (delaySeconds > 0) {
    QTimer::singleShot(delaySeconds * 1000, this,
                       &CaptureEngine::executeWindowCapture);
  } else {
    QTimer::singleShot(150, this, &CaptureEngine::executeWindowCapture);
  }
}

void CaptureEngine::executeRegionCapture() {
  m_cachedDesktopFrame = captureCombinedDesktop();
  if (m_cachedDesktopFrame.isNull()) {
    m_isCapturing = false;
    emit isCapturingChanged();
    emit captureError(tr("Ekran görüntüsü yakalanamadı."));
    return;
  }

  // Save frozen frame to temporary cache for QML overlay
  m_cachedDesktopFrame.save(m_frozenFramePath, "PNG");
  emit frozenFrameChanged();

  emit openSniperOverlay(m_frozenFramePath, m_cachedDesktopFrame.width(),
                         m_cachedDesktopFrame.height());
}

void CaptureEngine::executeFullscreenCapture() {
  QImage full = captureCombinedDesktop();
  if (full.isNull()) {
    m_isCapturing = false;
    emit isCapturingChanged();
    emit captureError(tr("Tam ekran görüntüsü yakalanamadı."));
    return;
  }

  saveAndProcessResult(full, CaptureMode::Fullscreen, full.rect());
  m_isCapturing = false;
  emit isCapturingChanged();
}

void CaptureEngine::executeWindowCapture() {
  // For window capture in Wayland/X11, fallback to active screen or primary
  // screen
  QScreen *primary = QGuiApplication::primaryScreen();
  if (!primary) {
    m_isCapturing = false;
    emit isCapturingChanged();
    emit captureError(tr("Aktif ekran bulunamadı."));
    return;
  }

  QPixmap pixmap = primary->grabWindow(0);
  QImage img = pixmap.toImage();
  if (img.isNull()) {
    m_isCapturing = false;
    emit isCapturingChanged();
    emit captureError(tr("Pencere görüntüsü yakalanamadı."));
    return;
  }

  saveAndProcessResult(img, CaptureMode::Window, img.rect());
  m_isCapturing = false;
  emit isCapturingChanged();
}

void CaptureEngine::processRegionSelected(int x, int y, int width, int height) {
  if (m_cachedDesktopFrame.isNull()) {
    m_isCapturing = false;
    emit isCapturingChanged();
    return;
  }

  // Normalize rect
  QRect targetRect(x, y, width, height);
  targetRect = targetRect.normalized();
  targetRect = targetRect.intersected(m_cachedDesktopFrame.rect());

  if (targetRect.width() <= 2 || targetRect.height() <= 2) {
    // Too small selection, ignore / cancel
    emit closeSniperOverlay();
    m_isCapturing = false;
    emit isCapturingChanged();
    return;
  }

  QImage cropped = m_cachedDesktopFrame.copy(targetRect);
  saveAndProcessResult(cropped, CaptureMode::Region, targetRect);

  if (m_settings && m_settings->closeOverlayOnCapture()) {
    emit closeSniperOverlay();
  }

  m_isCapturing = false;
  emit isCapturingChanged();
}

void CaptureEngine::cancelCapture() {
  emit closeSniperOverlay();
  m_isCapturing = false;
  emit isCapturingChanged();
}

QImage CaptureEngine::captureCombinedDesktop() const {
  const auto screens = QGuiApplication::screens();
  if (screens.isEmpty()) {
    return {};
  }

  if (screens.size() == 1) {
    return screens.first()->grabWindow(0).toImage();
  }

  // Calculate combined bounding rectangle for multi-monitor setup
  QRect totalGeometry;
  for (QScreen *screen : screens) {
    totalGeometry = totalGeometry.united(screen->geometry());
  }

  QImage combined(totalGeometry.size(), QImage::Format_ARGB32_Premultiplied);
  combined.fill(Qt::black);

  QPainter painter(&combined);
  for (QScreen *screen : screens) {
    QPixmap pix = screen->grabWindow(0);
    QRect geom = screen->geometry();
    painter.drawPixmap(geom.x() - totalGeometry.x(),
                       geom.y() - totalGeometry.y(), pix);
  }
  painter.end();

  return combined;
}

bool CaptureEngine::saveAndProcessResult(const QImage &image,
                                         CaptureMode /*mode*/,
                                         const QRect & /*sourceRect*/) {
  if (image.isNull() || !m_settings) {
    return false;
  }

  bool copied = false;
  bool saved = false;
  QString finalPath;

  // 1. Pano (Clipboard)
  if (m_settings->autoCopyToClipboard()) {
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setImage(image);
    copied = true;
  }

  // 2. Diske Kaydetme (Disk Save)
  if (m_settings->autoSaveToDisk()) {
    finalPath = m_settings->generateFullPath();
    QString fmt = m_settings->imageFormat().toUpper();
    int quality =
        (fmt == "JPEG" || fmt == "JPG") ? m_settings->jpegQuality() : -1;

    if (image.save(finalPath, fmt.toUtf8().constData(), quality)) {
      saved = true;
      m_lastCapturedFilePath = finalPath;
      emit lastCaptureChanged();

      if (m_library) {
        m_library->refresh();
      }
    }
  }

  QFileInfo info(finalPath);
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
  return QFile::copy(sourcePath, destinationPath);
}

} // namespace ro_screenshot
