#pragma once

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QMutex>
#include <QMutexLocker>
#include <QSaveFile>
#include <QStandardPaths>
#include <QString>
#include <QUrl>

namespace ro_screenshot {

class ThumbnailCache {
public:
  explicit ThumbnailCache(const QString &customCacheDir = QString()) {
    if (!customCacheDir.isEmpty()) {
      m_cacheDir = customCacheDir;
    } else {
      QString cacheBase =
          QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
      if (cacheBase.isEmpty()) {
        cacheBase = QDir::homePath() + "/.cache/ro-asd/ro-screenshot";
      }
      m_cacheDir = cacheBase + "/thumbnails";
    }
    QDir().mkpath(m_cacheDir);
  }

  QString cacheDir() const {
    QMutexLocker locker(&m_mutex);
    return m_cacheDir;
  }

  void setMaxCacheSizeBytes(qint64 bytes) {
    QMutexLocker locker(&m_mutex);
    m_maxCacheSizeBytes = bytes;
  }

  qint64 maxCacheSizeBytes() const {
    QMutexLocker locker(&m_mutex);
    return m_maxCacheSizeBytes;
  }

  void setMaxCacheItemCount(int count) {
    QMutexLocker locker(&m_mutex);
    m_maxCacheItemCount = count;
  }

  QString generateKey(const QString &canonicalPath, qint64 fileSize,
                      const QDateTime &mtime) const {
    QString keyData = QString("%1|%2|%3")
                          .arg(canonicalPath)
                          .arg(fileSize)
                          .arg(mtime.toMSecsSinceEpoch());
    QByteArray hash =
        QCryptographicHash::hash(keyData.toUtf8(), QCryptographicHash::Sha256)
            .toHex();
    return QString::fromUtf8(hash);
  }

  QString getThumbnailPath(const QString &imagePath, qint64 fileSize = -1,
                           const QDateTime &mtime = QDateTime()) {
    QMutexLocker locker(&m_mutex);
    QFileInfo info(imagePath);
    QString canonicalPath = info.canonicalFilePath();
    if (canonicalPath.isEmpty()) {
      canonicalPath = info.absoluteFilePath();
    }

    if (fileSize <= 0) {
      fileSize = info.size();
    }
    QDateTime modTime = mtime;
    if (!modTime.isValid()) {
      modTime = info.lastModified();
    }

    QString key = generateKey(canonicalPath, fileSize, modTime);
    QString thumbPath = m_cacheDir + "/" + key + ".png";

    if (QFile::exists(thumbPath)) {
      return QUrl::fromLocalFile(thumbPath).toString();
    }

    // Generate thumbnail safely
    QImageReader reader(canonicalPath);
    reader.setAutoTransform(true);
    QSize origSize = reader.size();
    if (!origSize.isValid()) {
      return QString();
    }

    QSize targetSize = origSize.scaled(320, 240, Qt::KeepAspectRatio);
    reader.setScaledSize(targetSize);
    QImage thumb = reader.read();
    if (thumb.isNull()) {
      return QString();
    }

    // Atomic write using QSaveFile
    QSaveFile saveFile(thumbPath);
    if (saveFile.open(QIODevice::WriteOnly)) {
      if (thumb.save(&saveFile, "PNG")) {
        if (saveFile.commit()) {
          pruneCacheIfNeeded();
          return QUrl::fromLocalFile(thumbPath).toString();
        }
      } else {
        saveFile.cancelWriting();
      }
    }

    return QUrl::fromLocalFile(imagePath).toString();
  }

  bool clearCache() {
    QMutexLocker locker(&m_mutex);
    QDir dir(m_cacheDir);
    if (!dir.exists()) {
      return true;
    }
    QFileInfoList entries =
        dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    bool allSuccess = true;
    for (const auto &entry : entries) {
      if (!QFile::remove(entry.absoluteFilePath())) {
        allSuccess = false;
      }
    }
    return allSuccess;
  }

  void removeThumbnailForFile(const QString &canonicalPath, qint64 fileSize,
                              const QDateTime &mtime) {
    QMutexLocker locker(&m_mutex);
    QString key = generateKey(canonicalPath, fileSize, mtime);
    QString thumbPath = m_cacheDir + "/" + key + ".png";
    if (QFile::exists(thumbPath)) {
      QFile::remove(thumbPath);
    }
  }

  void pruneCacheIfNeeded() {
    // Note: Called under m_mutex
    QDir dir(m_cacheDir);
    if (!dir.exists()) {
      return;
    }

    QFileInfoList entries = dir.entryInfoList(
        QStringList() << "*.png", QDir::Files | QDir::NoDotAndDotDot,
        QDir::Time | QDir::Reversed); // Oldest first

    qint64 totalBytes = 0;
    for (const auto &entry : entries) {
      totalBytes += entry.size();
    }

    int count = static_cast<int>(entries.size());
    for (const auto &entry : entries) {
      if (totalBytes <= m_maxCacheSizeBytes && count <= m_maxCacheItemCount) {
        break;
      }
      qint64 sz = entry.size();
      if (QFile::remove(entry.absoluteFilePath())) {
        totalBytes -= sz;
        count--;
      }
    }
  }

  qint64 currentCacheSize() const {
    QMutexLocker locker(&m_mutex);
    QDir dir(m_cacheDir);
    if (!dir.exists()) {
      return 0;
    }
    qint64 total = 0;
    QFileInfoList entries =
        dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const auto &entry : entries) {
      total += entry.size();
    }
    return total;
  }

  int currentCacheItemCount() const {
    QMutexLocker locker(&m_mutex);
    QDir dir(m_cacheDir);
    if (!dir.exists()) {
      return 0;
    }
    return static_cast<int>(
        dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot).size());
  }

private:
  mutable QMutex m_mutex;
  QString m_cacheDir;
  qint64 m_maxCacheSizeBytes{100 * 1024 * 1024}; // 100 MB default
  int m_maxCacheItemCount{1000};                 // 1000 items max
};

} // namespace ro_screenshot
