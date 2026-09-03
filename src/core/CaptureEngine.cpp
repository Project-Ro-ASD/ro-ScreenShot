#include "CaptureEngine.hpp"
#include <QBuffer>
#include <QClipboard>
#include <QColor>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QPainter>
#include <QPixmap>
#include <QSaveFile>
#include <QScreen>
#include <QStandardPaths>
#include <QUrl>
#include <QUuid>
#include <algorithm>

namespace ro_screenshot {

namespace {

bool writeImageAtomically(const QImage &image, const QString &filePath,
                          const QString &format, int quality) {
  if (image.isNull() || filePath.isEmpty()) {
    return false;
  }

  const QFileInfo destinationInfo(filePath);
  if (!QDir().mkpath(destinationInfo.absolutePath())) {
    return false;
  }

  QByteArray encodedImage;
  QBuffer buffer(&encodedImage);
  const bool encoded =
      buffer.open(QIODevice::WriteOnly) &&
      image.save(&buffer, format.toUtf8().constData(), quality);
  buffer.close();
  if (!encoded) {
    return false;
  }

  QSaveFile outputFile(filePath);
  return outputFile.open(QIODevice::WriteOnly) &&
         outputFile.write(encodedImage) == encodedImage.size() &&
         outputFile.commit();
}

} // namespace

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
  if (m_settings) {
    connect(m_settings, &SettingsManager::lastRegionChanged, this,
            &CaptureEngine::lastRegionChanged);
  }
}

bool CaptureEngine::isCapturing() const { return m_isCapturing; }

QString CaptureEngine::lastCapturedFilePath() const {
  return m_lastCapturedFilePath;
}

QString CaptureEngine::frozenFramePath() const { return m_frozenFramePath; }

bool CaptureEngine::hasLastRegion() const {
  return m_settings && m_settings->lastRegion().isValid() &&
         m_settings->lastRegionFrameSize().isValid();
}

void CaptureEngine::requestRegionCapture(int delaySeconds) {
  requestRegionCaptureWithAction(delaySeconds, {});
}

void CaptureEngine::requestRegionCaptureWithAction(int delaySeconds,
                                                   const QString &action) {
  if (m_isCapturing) {
    return;
  }
  m_isCapturing = true;
  m_pendingLastRegion = false;
  m_pendingAction = action;
  emit isCapturingChanged();
  emit captureUiShouldHide();

  if (delaySeconds > 0) {
    QTimer::singleShot(delaySeconds * 1000, this,
                       &CaptureEngine::executeRegionCapture);
  } else {
    // 150ms delay to allow any menus to settle
    QTimer::singleShot(150, this, &CaptureEngine::executeRegionCapture);
  }
}

void CaptureEngine::requestFullscreenCapture(int delaySeconds) {
  requestFullscreenCaptureWithAction(delaySeconds, {});
}

void CaptureEngine::requestFullscreenCaptureWithAction(int delaySeconds,
                                                       const QString &action) {
  if (m_isCapturing) {
    return;
  }
  m_isCapturing = true;
  m_pendingLastRegion = false;
  m_pendingAction = action;
  emit isCapturingChanged();
  emit captureUiShouldHide();

  if (delaySeconds > 0) {
    QTimer::singleShot(delaySeconds * 1000, this,
                       &CaptureEngine::executeFullscreenCapture);
  } else {
    QTimer::singleShot(150, this, &CaptureEngine::executeFullscreenCapture);
  }
}

void CaptureEngine::requestWindowCapture(int delaySeconds) {
  requestWindowCaptureWithAction(delaySeconds, {});
}

void CaptureEngine::requestWindowCaptureWithAction(int delaySeconds,
                                                   const QString &action) {
  if (m_isCapturing) {
    return;
  }
  m_isCapturing = true;
  m_pendingLastRegion = false;
  m_pendingAction = action;
  emit isCapturingChanged();
  emit captureUiShouldHide();

  if (delaySeconds > 0) {
    QTimer::singleShot(delaySeconds * 1000, this,
                       &CaptureEngine::executeWindowCapture);
  } else {
    QTimer::singleShot(150, this, &CaptureEngine::executeWindowCapture);
  }
}

void CaptureEngine::requestLastRegionCapture(int delaySeconds) {
  requestLastRegionCaptureWithAction(delaySeconds, {});
}

void CaptureEngine::requestLastRegionCaptureWithAction(int delaySeconds,
                                                       const QString &action) {
  if (m_isCapturing || !hasLastRegion()) {
    return;
  }
  m_isCapturing = true;
  m_pendingLastRegion = true;
  m_pendingAction = action;
  emit isCapturingChanged();
  emit captureUiShouldHide();
  QTimer::singleShot(std::max(0, delaySeconds) * 1000, this, [this]() {
    if (shouldUsePortal()) {
      requestPortalCapture(CaptureMode::Region);
      return;
    }
    const QImage desktop = captureCombinedDesktop();
    const QRect region = m_settings->lastRegion().intersected(desktop.rect());
    if (desktop.isNull() ||
        m_settings->lastRegionFrameSize() != desktop.size() ||
        region.width() <= 2 || region.height() <= 2) {
      failCapture(tr("The previous capture region is no longer available."));
      return;
    }
    saveAndProcessResult(desktop.copy(region), CaptureMode::Region, region,
                         m_pendingAction);
    m_isCapturing = false;
    m_pendingLastRegion = false;
    m_pendingAction.clear();
    emit isCapturingChanged();
    emit captureUiMayRestore();
  });
}

void CaptureEngine::executeRegionCapture() {
  if (shouldUsePortal()) {
    requestPortalCapture(CaptureMode::Region);
    return;
  }
  m_cachedDesktopFrame = captureCombinedDesktop();
  if (m_cachedDesktopFrame.isNull()) {
    failCapture(tr("Ekran görüntüsü yakalanamadı."));
    return;
  }

  // Save frozen frame to temporary cache for QML overlay
  m_cachedDesktopFrame.save(m_frozenFramePath, "PNG");
  emit frozenFrameChanged();

  emit openSniperOverlay(m_frozenFramePath, m_cachedDesktopFrame.width(),
                         m_cachedDesktopFrame.height());
}

void CaptureEngine::executeFullscreenCapture() {
  if (shouldUsePortal()) {
    requestPortalCapture(CaptureMode::Fullscreen);
    return;
  }
  QImage full = captureCombinedDesktop();
  if (full.isNull()) {
    failCapture(tr("Tam ekran görüntüsü yakalanamadı."));
    return;
  }

  saveAndProcessResult(full, CaptureMode::Fullscreen, full.rect(),
                       m_pendingAction);
  m_isCapturing = false;
  m_pendingAction.clear();
  emit isCapturingChanged();
  emit captureUiMayRestore();
}

void CaptureEngine::executeWindowCapture() {
  if (shouldUsePortal()) {
    requestPortalCapture(CaptureMode::Window);
    return;
  }
  // For window capture in Wayland/X11, fallback to active screen or primary
  // screen
  QScreen *primary = QGuiApplication::primaryScreen();
  if (!primary) {
    failCapture(tr("Aktif ekran bulunamadı."));
    return;
  }

  QPixmap pixmap = primary->grabWindow(0);
  QImage img = pixmap.toImage();
  if (img.isNull()) {
    failCapture(tr("Pencere görüntüsü yakalanamadı."));
    return;
  }

  saveAndProcessResult(img, CaptureMode::Window, img.rect(), m_pendingAction);
  m_isCapturing = false;
  m_pendingAction.clear();
  emit isCapturingChanged();
  emit captureUiMayRestore();
}

bool CaptureEngine::shouldUsePortal() const {
  return QGuiApplication::platformName().contains(QStringLiteral("wayland"),
                                                  Qt::CaseInsensitive);
}

void CaptureEngine::requestPortalCapture(CaptureMode mode) {
  QDBusInterface portal(QStringLiteral("org.freedesktop.portal.Desktop"),
                        QStringLiteral("/org/freedesktop/portal/desktop"),
                        QStringLiteral("org.freedesktop.portal.Screenshot"),
                        QDBusConnection::sessionBus());
  if (!portal.isValid()) {
    failCapture(tr("The Wayland screenshot portal is not available."));
    return;
  }

  m_pendingPortalMode = mode;
  QVariantMap options;
  const QString token = QStringLiteral("ro_screenshot_%1")
                            .arg(QUuid::createUuid().toString(QUuid::Id128));
  options.insert(QStringLiteral("handle_token"), token);

  const uint version = portal.property("version").toUInt();
  const uint availableTargets = portal.property("AvailableTargets").toUInt();
  if (version >= 3U && availableTargets != 0U) {
    constexpr uint screenTarget = 1U;
    constexpr uint windowTarget = 2U;
    constexpr uint activeWindowTarget = 8U;
    uint target = screenTarget;
    if (mode == CaptureMode::Window) {
      target = (availableTargets & activeWindowTarget) != 0U
                   ? activeWindowTarget
                   : windowTarget;
    }
    if ((availableTargets & target) != 0U) {
      options.insert(QStringLiteral("target"), target);
      options.insert(QStringLiteral("interactive"), false);
    } else {
      options.insert(QStringLiteral("interactive"),
                     mode == CaptureMode::Window);
    }
  } else {
    options.insert(QStringLiteral("interactive"), mode == CaptureMode::Window);
  }

  m_pendingPortalRequestPath = expectedPortalRequestPath(token);
  if (!connectPortalResponse(m_pendingPortalRequestPath)) {
    failCapture(tr("Could not monitor the Wayland screenshot request."));
    return;
  }

  auto *watcher = new QDBusPendingCallWatcher(
      portal.asyncCall(QStringLiteral("Screenshot"), QString(), options), this);
  connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher]() {
    QDBusPendingReply<QDBusObjectPath> reply = *watcher;
    watcher->deleteLater();
    if (reply.isError()) {
      disconnectPortalResponse();
      failCapture(tr("The Wayland screenshot request failed: %1")
                      .arg(reply.error().message()));
      return;
    }

    // A very fast backend may emit Response before this async method
    // reply is delivered. In that case the response handler has
    // already completed and disconnected the request path.
    if (!m_isCapturing) {
      return;
    }

    const QString returnedPath = reply.value().path();
    if (returnedPath != m_pendingPortalRequestPath) {
      disconnectPortalResponse();
      m_pendingPortalRequestPath = returnedPath;
      if (!connectPortalResponse(m_pendingPortalRequestPath)) {
        failCapture(tr("Could not monitor the Wayland screenshot request."));
      }
    }
  });
}

QString CaptureEngine::expectedPortalRequestPath(const QString &token) const {
  QString sender = QDBusConnection::sessionBus().baseService();
  sender.remove(QLatin1Char(':'));
  sender.replace(QLatin1Char('.'), QLatin1Char('_'));
  return QStringLiteral("/org/freedesktop/portal/desktop/request/%1/%2")
      .arg(sender, token);
}

bool CaptureEngine::connectPortalResponse(const QString &path) {
  return !path.isEmpty() &&
         QDBusConnection::sessionBus().connect(
             QStringLiteral("org.freedesktop.portal.Desktop"), path,
             QStringLiteral("org.freedesktop.portal.Request"),
             QStringLiteral("Response"), this,
             SLOT(handlePortalResponse(uint, QVariantMap)));
}

void CaptureEngine::disconnectPortalResponse() {
  if (m_pendingPortalRequestPath.isEmpty()) {
    return;
  }
  QDBusConnection::sessionBus().disconnect(
      QStringLiteral("org.freedesktop.portal.Desktop"),
      m_pendingPortalRequestPath,
      QStringLiteral("org.freedesktop.portal.Request"),
      QStringLiteral("Response"), this,
      SLOT(handlePortalResponse(uint, QVariantMap)));
  m_pendingPortalRequestPath.clear();
}

void CaptureEngine::handlePortalResponse(uint response,
                                         const QVariantMap &results) {
  disconnectPortalResponse();

  if (response != 0U) {
    failCapture(response == 1U
                    ? tr("Screenshot capture was cancelled.")
                    : tr("The screenshot portal rejected the request."));
    return;
  }

  const QString uri = results.value(QStringLiteral("uri")).toString();
  const QImage image(QUrl(uri).toLocalFile());
  if (image.isNull()) {
    failCapture(tr("The screenshot portal returned an invalid image."));
    return;
  }

  if (m_pendingPortalMode == CaptureMode::Region) {
    if (m_pendingLastRegion) {
      const QRect region = m_settings->lastRegion().intersected(image.rect());
      if (m_settings->lastRegionFrameSize() != image.size() ||
          region.width() <= 2 || region.height() <= 2) {
        failCapture(tr("The previous capture region is no longer available."));
        return;
      }
      saveAndProcessResult(image.copy(region), CaptureMode::Region, region,
                           m_pendingAction);
      m_pendingLastRegion = false;
      m_isCapturing = false;
      m_pendingAction.clear();
      emit isCapturingChanged();
      emit captureUiMayRestore();
      return;
    }
    m_cachedDesktopFrame = image;
    m_cachedDesktopFrame.save(m_frozenFramePath, "PNG");
    emit frozenFrameChanged();
    emit openSniperOverlay(m_frozenFramePath, image.width(), image.height());
    return;
  }

  saveAndProcessResult(image, m_pendingPortalMode, image.rect(),
                       m_pendingAction);
  m_isCapturing = false;
  m_pendingAction.clear();
  emit isCapturingChanged();
  emit captureUiMayRestore();
}

void CaptureEngine::failCapture(const QString &message) {
  disconnectPortalResponse();
  m_isCapturing = false;
  m_pendingLastRegion = false;
  m_pendingAction.clear();
  emit isCapturingChanged();
  emit captureError(message);
  emit captureUiMayRestore();
}

void CaptureEngine::processRegionSelected(int x, int y, int width, int height,
                                          const QString &action) {
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
  m_settings->setLastRegionGeometry(targetRect, m_cachedDesktopFrame.size());
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

void CaptureEngine::cancelCapture() {
  emit closeSniperOverlay();
  m_isCapturing = false;
  m_pendingAction.clear();
  emit isCapturingChanged();
  emit captureUiMayRestore();
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
                                         const QRect & /*sourceRect*/,
                                         const QString &action) {
  if (image.isNull() || !m_settings) {
    return false;
  }

  bool copied = false;
  bool saved = false;
  QString finalPath;

  // 1. Pano (Clipboard)
  const bool shouldCopy =
      action == QStringLiteral("copy") ||
      (action.isEmpty() && m_settings->autoCopyToClipboard());
  const bool shouldSave = action == QStringLiteral("save") ||
                          (action.isEmpty() && m_settings->autoSaveToDisk());

  if (!shouldCopy && !shouldSave) {
    emit captureError(
        tr("Select at least one capture output: clipboard or disk."));
    return false;
  }

  if (shouldCopy) {
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setImage(image);
    copied = true;
  }

  // 2. Diske Kaydetme (Disk Save)
  if (shouldSave) {
    finalPath = m_settings->generateFullPath();
    QString fmt = m_settings->imageFormat().toUpper();
    int quality =
        (fmt == "JPEG" || fmt == "JPG") ? m_settings->jpegQuality() : -1;

    const bool savedAtomically =
        writeImageAtomically(image, finalPath, fmt, quality);

    if (savedAtomically) {
      saved = true;
      m_lastCapturedFilePath = finalPath;
      emit lastCaptureChanged();

      if (m_library) {
        m_library->refresh();
      }
    } else {
      emit captureError(
          tr("Could not save the screenshot to %1.").arg(finalPath));
    }
  }

  if (!saved && !copied) {
    return false;
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
  const QImage image(sourcePath);
  const QString format = QFileInfo(destinationPath).suffix().toUpper();
  return writeImageAtomically(image, destinationPath,
                              format.isEmpty() ? QStringLiteral("PNG") : format,
                              -1);
}

QString CaptureEngine::colorAt(int x, int y) const {
  if (m_cachedDesktopFrame.isNull() ||
      !m_cachedDesktopFrame.rect().contains(x, y)) {
    return {};
  }
  return m_cachedDesktopFrame.pixelColor(x, y).name(QColor::HexRgb).toUpper();
}

bool CaptureEngine::copyColorAt(int x, int y) {
  const QString color = colorAt(x, y);
  if (color.isEmpty()) {
    return false;
  }
  QGuiApplication::clipboard()->setText(color);
  emit colorCopied(color);
  return true;
}

} // namespace ro_screenshot
