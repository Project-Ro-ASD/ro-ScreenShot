#include "SettingsManager.hpp"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

namespace ro_screenshot {

namespace {
constexpr const char *KEY_SAVE_DIR = "General/SaveDirectory";
constexpr const char *KEY_TEMPLATE = "General/FileNameTemplate";
constexpr const char *KEY_FORMAT = "General/ImageFormat";
constexpr const char *KEY_JPEG_QUALITY = "General/JpegQuality";
constexpr const char *KEY_AUTO_COPY = "Workflow/AutoCopyToClipboard";
constexpr const char *KEY_AUTO_SAVE = "Workflow/AutoSaveToDisk";
constexpr const char *KEY_FLOATING_THUMB = "Workflow/ShowFloatingThumbnail";
constexpr const char *KEY_NOTIFICATION = "Workflow/ShowNotification";
constexpr const char *KEY_SHUTTER_SOUND = "Workflow/PlayShutterSound";
constexpr const char *KEY_SUBFOLDERS = "Workflow/CreateSubfoldersByMonth";
constexpr const char *KEY_CLOSE_OVERLAY = "Sniper/CloseOverlayOnCapture";
constexpr const char *KEY_MAGNIFIER = "Sniper/MagnifierEnabled";
constexpr const char *KEY_MAGNIFIER_ZOOM = "Sniper/MagnifierZoom";
constexpr const char *KEY_LAST_REGION = "Sniper/LastRegion";

QString defaultPicturesDir() {
  QString pictures =
      QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
  if (pictures.isEmpty()) {
    pictures = QDir::homePath() + "/Pictures";
  }
  return pictures + "/Screenshots";
}
} // namespace

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent), m_settings("ro-asd", "ro-screenshot") {
  load();
  ensureSaveDirectoryExists();
}

void SettingsManager::load() {
  m_saveDirectory =
      m_settings.value(KEY_SAVE_DIR, defaultPicturesDir()).toString();
  m_fileNameTemplate =
      m_settings.value(KEY_TEMPLATE, "Ro-Shot_%Y-%m-%d_%H-%M-%S").toString();
  m_imageFormat = m_settings.value(KEY_FORMAT, "png").toString().toLower();
  if (m_imageFormat == "jpeg") {
    m_imageFormat = "jpg";
  }
  m_jpegQuality = m_settings.value(KEY_JPEG_QUALITY, 90).toInt();
  m_autoCopyToClipboard = m_settings.value(KEY_AUTO_COPY, true).toBool();
  m_autoSaveToDisk = m_settings.value(KEY_AUTO_SAVE, true).toBool();
  m_showFloatingThumbnail = m_settings.value(KEY_FLOATING_THUMB, true).toBool();
  m_showNotification = m_settings.value(KEY_NOTIFICATION, true).toBool();
  m_playShutterSound = m_settings.value(KEY_SHUTTER_SOUND, false).toBool();
  m_createSubfoldersByMonth = m_settings.value(KEY_SUBFOLDERS, false).toBool();
  m_closeOverlayOnCapture = m_settings.value(KEY_CLOSE_OVERLAY, true).toBool();
  m_magnifierEnabled = m_settings.value(KEY_MAGNIFIER, true).toBool();
  m_magnifierZoom = m_settings.value(KEY_MAGNIFIER_ZOOM, 8).toInt();
  m_lastRegion = m_settings.value(KEY_LAST_REGION).toRect();
}

void SettingsManager::sync() {
  m_settings.setValue(KEY_SAVE_DIR, m_saveDirectory);
  m_settings.setValue(KEY_TEMPLATE, m_fileNameTemplate);
  m_settings.setValue(KEY_FORMAT, m_imageFormat);
  m_settings.setValue(KEY_JPEG_QUALITY, m_jpegQuality);
  m_settings.setValue(KEY_AUTO_COPY, m_autoCopyToClipboard);
  m_settings.setValue(KEY_AUTO_SAVE, m_autoSaveToDisk);
  m_settings.setValue(KEY_FLOATING_THUMB, m_showFloatingThumbnail);
  m_settings.setValue(KEY_NOTIFICATION, m_showNotification);
  m_settings.setValue(KEY_SHUTTER_SOUND, m_playShutterSound);
  m_settings.setValue(KEY_SUBFOLDERS, m_createSubfoldersByMonth);
  m_settings.setValue(KEY_CLOSE_OVERLAY, m_closeOverlayOnCapture);
  m_settings.setValue(KEY_MAGNIFIER, m_magnifierEnabled);
  m_settings.setValue(KEY_MAGNIFIER_ZOOM, m_magnifierZoom);
  m_settings.setValue(KEY_LAST_REGION, m_lastRegion);
  m_settings.sync();
}

void SettingsManager::resetToDefaults() {
  setSaveDirectory(defaultPicturesDir());
  setFileNameTemplate("Ro-Shot_%Y-%m-%d_%H-%M-%S");
  setImageFormat("png");
  setJpegQuality(90);
  setAutoCopyToClipboard(true);
  setAutoSaveToDisk(true);
  setShowFloatingThumbnail(true);
  setShowNotification(true);
  setPlayShutterSound(false);
  setCreateSubfoldersByMonth(false);
  setCloseOverlayOnCapture(true);
  setMagnifierEnabled(true);
  setMagnifierZoom(8);
  setLastRegion({});
  sync();
}

QString SettingsManager::saveDirectory() const { return m_saveDirectory; }

void SettingsManager::setSaveDirectory(const QString &dir) {
  if (m_saveDirectory != dir && !dir.isEmpty()) {
    m_saveDirectory = dir;
    m_settings.setValue(KEY_SAVE_DIR, m_saveDirectory);
    ensureSaveDirectoryExists();
    emit saveDirectoryChanged();
  }
}

QString SettingsManager::fileNameTemplate() const { return m_fileNameTemplate; }

void SettingsManager::setFileNameTemplate(const QString &tmpl) {
  if (m_fileNameTemplate != tmpl && !tmpl.isEmpty()) {
    m_fileNameTemplate = tmpl;
    m_settings.setValue(KEY_TEMPLATE, m_fileNameTemplate);
    emit fileNameTemplateChanged();
  }
}

QString SettingsManager::imageFormat() const { return m_imageFormat; }

void SettingsManager::setImageFormat(const QString &format) {
  QString fmt = format.toLower();
  if (fmt == "jpeg") {
    fmt = "jpg";
  }
  if (fmt != "png" && fmt != "jpg" && fmt != "webp") {
    fmt = "png";
  }
  if (m_imageFormat != fmt) {
    m_imageFormat = fmt;
    m_settings.setValue(KEY_FORMAT, m_imageFormat);
    emit imageFormatChanged();
  }
}

int SettingsManager::jpegQuality() const { return m_jpegQuality; }

void SettingsManager::setJpegQuality(int quality) {
  int q = std::clamp(quality, 1, 100);
  if (m_jpegQuality != q) {
    m_jpegQuality = q;
    m_settings.setValue(KEY_JPEG_QUALITY, m_jpegQuality);
    emit jpegQualityChanged();
  }
}

bool SettingsManager::autoCopyToClipboard() const {
  return m_autoCopyToClipboard;
}

void SettingsManager::setAutoCopyToClipboard(bool enable) {
  if (m_autoCopyToClipboard != enable) {
    m_autoCopyToClipboard = enable;
    m_settings.setValue(KEY_AUTO_COPY, m_autoCopyToClipboard);
    emit autoCopyToClipboardChanged();
  }
}

bool SettingsManager::autoSaveToDisk() const { return m_autoSaveToDisk; }

void SettingsManager::setAutoSaveToDisk(bool enable) {
  if (m_autoSaveToDisk != enable) {
    m_autoSaveToDisk = enable;
    m_settings.setValue(KEY_AUTO_SAVE, m_autoSaveToDisk);
    emit autoSaveToDiskChanged();
  }
}

bool SettingsManager::showFloatingThumbnail() const {
  return m_showFloatingThumbnail;
}

void SettingsManager::setShowFloatingThumbnail(bool enable) {
  if (m_showFloatingThumbnail != enable) {
    m_showFloatingThumbnail = enable;
    m_settings.setValue(KEY_FLOATING_THUMB, m_showFloatingThumbnail);
    emit showFloatingThumbnailChanged();
  }
}

bool SettingsManager::showNotification() const { return m_showNotification; }

void SettingsManager::setShowNotification(bool enable) {
  if (m_showNotification != enable) {
    m_showNotification = enable;
    m_settings.setValue(KEY_NOTIFICATION, m_showNotification);
    emit showNotificationChanged();
  }
}

bool SettingsManager::playShutterSound() const { return m_playShutterSound; }

void SettingsManager::setPlayShutterSound(bool enable) {
  if (m_playShutterSound != enable) {
    m_playShutterSound = enable;
    m_settings.setValue(KEY_SHUTTER_SOUND, m_playShutterSound);
    emit playShutterSoundChanged();
  }
}

bool SettingsManager::createSubfoldersByMonth() const {
  return m_createSubfoldersByMonth;
}

void SettingsManager::setCreateSubfoldersByMonth(bool enable) {
  if (m_createSubfoldersByMonth != enable) {
    m_createSubfoldersByMonth = enable;
    m_settings.setValue(KEY_SUBFOLDERS, m_createSubfoldersByMonth);
    emit createSubfoldersByMonthChanged();
  }
}

bool SettingsManager::closeOverlayOnCapture() const {
  return m_closeOverlayOnCapture;
}

void SettingsManager::setCloseOverlayOnCapture(bool enable) {
  if (m_closeOverlayOnCapture != enable) {
    m_closeOverlayOnCapture = enable;
    m_settings.setValue(KEY_CLOSE_OVERLAY, m_closeOverlayOnCapture);
    emit closeOverlayOnCaptureChanged();
  }
}

bool SettingsManager::magnifierEnabled() const { return m_magnifierEnabled; }

void SettingsManager::setMagnifierEnabled(bool enable) {
  if (m_magnifierEnabled != enable) {
    m_magnifierEnabled = enable;
    m_settings.setValue(KEY_MAGNIFIER, m_magnifierEnabled);
    emit magnifierEnabledChanged();
  }
}

int SettingsManager::magnifierZoom() const { return m_magnifierZoom; }

void SettingsManager::setMagnifierZoom(int zoom) {
  int z = std::clamp(zoom, 2, 16);
  if (m_magnifierZoom != z) {
    m_magnifierZoom = z;
    m_settings.setValue(KEY_MAGNIFIER_ZOOM, m_magnifierZoom);
    emit magnifierZoomChanged();
  }
}

QRect SettingsManager::lastRegion() const { return m_lastRegion; }

void SettingsManager::setLastRegion(const QRect &region) {
  QRect normalized = region.normalized();
  if (normalized.width() <= 2 || normalized.height() <= 2) {
    normalized = {};
  }
  if (normalized == m_lastRegion) {
    return;
  }
  m_lastRegion = normalized;
  m_settings.setValue(KEY_LAST_REGION, m_lastRegion);
  emit lastRegionChanged();
}

QString SettingsManager::formatFileName(const QDateTime &dt) const {
  QString result = m_fileNameTemplate;
  result.replace("%Y", dt.toString("yyyy"));
  result.replace("%y", dt.toString("yy"));
  result.replace("%m", dt.toString("MM"));
  result.replace("%d", dt.toString("dd"));
  result.replace("%H", dt.toString("hh"));
  result.replace("%M", dt.toString("mm"));
  result.replace("%S", dt.toString("ss"));
  result.replace("%ms", dt.toString("zzz"));

  QString ext = m_imageFormat;
  if (ext == "jpeg") {
    ext = "jpg";
  }
  return result + "." + ext;
}

QString SettingsManager::generateFullPath(const QDateTime &dt) const {
  QString dir = m_saveDirectory;
  if (m_createSubfoldersByMonth) {
    dir += "/" + dt.toString("yyyy-MM");
  }
  QDir().mkpath(dir);
  QString requestedPath = dir + "/" + formatFileName(dt);
  if (!QFileInfo::exists(requestedPath)) {
    return requestedPath;
  }

  const QFileInfo requestedInfo(requestedPath);
  const QString baseName = requestedInfo.completeBaseName();
  const QString suffix = requestedInfo.suffix();
  for (int copyIndex = 1; copyIndex < 10000; ++copyIndex) {
    const QString candidate =
        dir + "/" + baseName + QStringLiteral("_%1").arg(copyIndex) +
        (suffix.isEmpty() ? QString() : QStringLiteral(".") + suffix);
    if (!QFileInfo::exists(candidate)) {
      return candidate;
    }
  }

  return dir + "/" + baseName + "_" +
         QString::number(QDateTime::currentMSecsSinceEpoch()) +
         (suffix.isEmpty() ? QString() : QStringLiteral(".") + suffix);
}

QString SettingsManager::previewFileName() const {
  return formatFileName(QDateTime::currentDateTime());
}

void SettingsManager::ensureSaveDirectoryExists() const {
  if (!m_saveDirectory.isEmpty()) {
    QDir().mkpath(m_saveDirectory);
  }
}

} // namespace ro_screenshot
