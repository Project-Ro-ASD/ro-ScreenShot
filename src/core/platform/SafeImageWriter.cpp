#include "SafeImageWriter.hpp"
#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImageWriter>
#include <QMutexLocker>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QUuid>

namespace ro_screenshot {

SafeImageWriter::SafeImageWriter(QObject *parent) : QObject(parent) {}

SafeImageWriter::~SafeImageWriter() { cleanupTempFiles(); }

bool SafeImageWriter::isFormatSupported(const QString &format) const {
  const QString upper = format.toUpper();
  const QList<QByteArray> supported = QImageWriter::supportedImageFormats();
  for (const QByteArray &fmt : supported) {
    if (QString::fromLatin1(fmt).toUpper() == upper) {
      return true;
    }
  }
  return upper == "PNG" || upper == "JPG" || upper == "JPEG" || upper == "WEBP";
}

bool SafeImageWriter::checkDirectoryWritable(const QString &targetDirectory,
                                             QString *errorMessage) const {
  QDir dir(targetDirectory);
  if (!dir.exists()) {
    if (!dir.mkpath(QStringLiteral("."))) {
      if (errorMessage) {
        *errorMessage =
            tr("Hedef dizin oluşturulamıyor veya yazma izni yok: %1")
                .arg(targetDirectory);
      }
      return false;
    }
  }

  const QFileInfo dirInfo(targetDirectory);
  if (!dirInfo.isWritable()) {
    if (errorMessage) {
      *errorMessage =
          tr("Hedef dizine yazma izni bulunmuyor: %1").arg(targetDirectory);
    }
    return false;
  }
  return true;
}

bool SafeImageWriter::checkDiskSpace(const QString &targetDirectory,
                                     qint64 requiredBytes,
                                     QString *errorMessage) const {
  const QStorageInfo storage(targetDirectory);
  if (storage.isValid() && storage.isReady()) {
    // Keep at least 1MB safety margin
    const qint64 safetyMargin = 1024 * 1024;
    if (storage.bytesAvailable() < (requiredBytes + safetyMargin)) {
      if (errorMessage) {
        *errorMessage =
            tr("Disk dolu veya yetersiz alan. Mevcut: %1 MB, Gerekli: %2 MB")
                .arg(storage.bytesAvailable() / (1024 * 1024))
                .arg((requiredBytes + safetyMargin) / (1024 * 1024));
      }
      return false;
    }
  }
  return true;
}

SaveResult SafeImageWriter::writeImageAtomically(const QImage &image,
                                                 const QString &destinationPath,
                                                 const QString &format,
                                                 int quality) {
  SaveResult result;
  result.filePath = destinationPath;

  if (image.isNull()) {
    result.success = false;
    result.errorCode = CaptureErrorCode::InvalidImage;
    result.errorMessage = tr("Geçersiz veya boş görsel verisi.");
    return result;
  }

  if (destinationPath.isEmpty()) {
    result.success = false;
    result.errorCode = CaptureErrorCode::PermissionDenied;
    result.errorMessage = tr("Kayıt yolu belirtilmedi.");
    return result;
  }

  const QFileInfo destinationInfo(destinationPath);
  const QString targetDir = destinationInfo.absolutePath();

  QString dirError;
  if (!checkDirectoryWritable(targetDir, &dirError)) {
    result.success = false;
    result.errorCode = CaptureErrorCode::PermissionDenied;
    result.errorMessage = dirError;
    return result;
  }

  QString normFormat = format.toUpper();
  if (normFormat == "JPEG") {
    normFormat = "JPG";
  }

  if (!isFormatSupported(normFormat)) {
    result.success = false;
    result.errorCode = CaptureErrorCode::EncoderFailed;
    result.errorMessage = tr("Desteklenmeyen görsel formatı: %1").arg(format);
    return result;
  }

  // Encode image to memory buffer first
  QByteArray encodedImage;
  QBuffer buffer(&encodedImage);
  if (!buffer.open(QIODevice::WriteOnly)) {
    result.success = false;
    result.errorCode = CaptureErrorCode::EncoderFailed;
    result.errorMessage = tr("Bellek tamponu açılamadı.");
    return result;
  }

  QImageWriter writer(&buffer, normFormat.toUtf8());
  if (quality >= 1 && quality <= 100) {
    writer.setQuality(quality);
  }
  if (!writer.write(image)) {
    buffer.close();
    result.success = false;
    result.errorCode = CaptureErrorCode::EncoderFailed;
    result.errorMessage =
        tr("Görsel kodlayıcı hatası: %1").arg(writer.errorString());
    return result;
  }
  buffer.close();

  // Check disk space before saving
  QString diskError;
  if (!checkDiskSpace(targetDir, encodedImage.size(), &diskError)) {
    result.success = false;
    result.errorCode = CaptureErrorCode::DiskFull;
    result.errorMessage = diskError;
    return result;
  }

  // Atomic write via QSaveFile
  QSaveFile outputFile(destinationPath);
  if (!outputFile.open(QIODevice::WriteOnly)) {
    result.success = false;
    result.errorCode = CaptureErrorCode::PermissionDenied;
    result.errorMessage =
        tr("Dosya yazma için açılamadı: %1").arg(outputFile.errorString());
    return result;
  }

  const qint64 written = outputFile.write(encodedImage);
  if (written != encodedImage.size()) {
    outputFile.cancelWriting();
    result.success = false;
    result.errorCode = CaptureErrorCode::DiskFull;
    result.errorMessage = tr("Dosyaya veri tam yazılamadı.");
    return result;
  }

  if (!outputFile.commit()) {
    result.success = false;
    result.errorCode = CaptureErrorCode::PermissionDenied;
    result.errorMessage =
        tr("Atomik dosya işlemi tamamlanamadı (commit hatası).");
    return result;
  }

  result.success = true;
  result.errorCode = CaptureErrorCode::None;
  result.bytesWritten = written;
  return result;
}

QString
SafeImageWriter::resolveUniqueFilePath(const QString &targetPath) const {
  if (!QFileInfo::exists(targetPath)) {
    return targetPath;
  }

  const QFileInfo targetInfo(targetPath);
  const QString dir = targetInfo.absolutePath();
  const QString baseName = targetInfo.completeBaseName();
  const QString suffix = targetInfo.suffix();

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

QString SafeImageWriter::createTempFilePath(const QString &prefix,
                                            const QString &suffix) {
  QString cacheBase =
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
  if (cacheBase.isEmpty()) {
    cacheBase = QDir::homePath() + "/.cache/ro-asd/ro-screenshot";
  }
  QDir().mkpath(cacheBase);

  const QString uniqueId = QUuid::createUuid().toString(QUuid::Id128);
  const QString tempPath = cacheBase + "/" + prefix + "_" + uniqueId + suffix;
  registerTempFile(tempPath);
  return tempPath;
}

void SafeImageWriter::registerTempFile(const QString &filePath) {
  if (!filePath.isEmpty() && !m_activeTempFiles.contains(filePath)) {
    m_activeTempFiles.append(filePath);
  }
}

void SafeImageWriter::cleanupFile(const QString &filePath) {
  if (!filePath.isEmpty() && QFile::exists(filePath)) {
    QFile::remove(filePath);
  }
  m_activeTempFiles.removeAll(filePath);
}

void SafeImageWriter::cleanupTempFiles() {
  for (const QString &path : m_activeTempFiles) {
    if (QFile::exists(path)) {
      QFile::remove(path);
    }
  }
  m_activeTempFiles.clear();
}

void SafeImageWriter::setCachedImage(const QImage &image) {
  QMutexLocker locker(&m_cacheMutex);
  m_cachedImage = image;
}

QImage SafeImageWriter::cachedImage() const {
  QMutexLocker locker(&m_cacheMutex);
  return m_cachedImage;
}

void SafeImageWriter::clearCachedImage() {
  QMutexLocker locker(&m_cacheMutex);
  m_cachedImage = QImage();
}

} // namespace ro_screenshot
