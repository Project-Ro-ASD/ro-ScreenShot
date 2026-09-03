#pragma once

#include "core/Types.hpp"
#include "core/library/ThumbnailCache.hpp"
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QImageReader>
#include <QList>
#include <QMetaObject>
#include <QPointer>
#include <QRunnable>
#include <QSet>
#include <QString>
#include <QStringList>
#include <atomic>
#include <functional>
#include <memory>

namespace ro_screenshot {

struct ScanResult {
  uint64_t generation{0};
  QString rootDirectory;
  QList<ScreenshotItem> items;
  QStringList discoveredDirectories;
  qint64 totalBytes{0};
  bool success{true};
  QString errorMessage;
};

class ScanWorker : public QRunnable {
public:
  ScanWorker(uint64_t generation, const QString &rootDir,
             std::shared_ptr<ThumbnailCache> cache,
             std::shared_ptr<std::atomic<bool>> cancelFlag, QObject *receiver,
             std::function<void(int)> progressCb,
             std::function<void(ScanResult)> finishedCb)
      : m_generation(generation), m_rootDir(rootDir), m_cache(cache),
        m_cancelFlag(cancelFlag), m_receiver(receiver),
        m_progressCb(std::move(progressCb)),
        m_finishedCb(std::move(finishedCb)) {
    setAutoDelete(true);
  }

  void run() override {
    if (isCancelled()) {
      return;
    }

    ScanResult result;
    result.generation = m_generation;
    result.rootDirectory = m_rootDir;

    QDir root(m_rootDir);
    if (!root.exists()) {
      result.success = false;
      result.errorMessage = "Kayıt dizini mevcut değil.";
      dispatchFinished(result);
      return;
    }

    QSet<QString> visitedDirs;
    QString rootCanonical = root.canonicalPath();
    if (rootCanonical.isEmpty()) {
      rootCanonical = root.absolutePath();
    }
    visitedDirs.insert(rootCanonical);
    result.discoveredDirectories.append(root.absolutePath());

    QList<ScreenshotItem> items;
    qint64 total = 0;

    QStringList nameFilters;
    nameFilters << "*.png" << "*.jpg" << "*.jpeg" << "*.webp" << "*.bmp";

    // Recursive search with symlink loop protection
    scanDirectory(root, nameFilters, visitedDirs, result.discoveredDirectories,
                  items, total);

    if (isCancelled()) {
      return;
    }

    // Sort newest-first deterministically
    std::sort(items.begin(), items.end(),
              [](const ScreenshotItem &a, const ScreenshotItem &b) {
                if (a.createdAt != b.createdAt) {
                  return a.createdAt > b.createdAt;
                }
                return a.fileName < b.fileName;
              });

    result.items = std::move(items);
    result.totalBytes = total;
    result.success = true;

    if (!isCancelled()) {
      dispatchFinished(result);
    }
  }

private:
  bool isCancelled() const {
    return m_cancelFlag && m_cancelFlag->load(std::memory_order_relaxed);
  }

  void dispatchProgress(int count) {
    if (m_receiver && m_progressCb) {
      QMetaObject::invokeMethod(
          m_receiver, [cb = m_progressCb, count]() { cb(count); },
          Qt::QueuedConnection);
    }
  }

  void dispatchFinished(const ScanResult &result) {
    if (m_receiver && m_finishedCb) {
      QMetaObject::invokeMethod(
          m_receiver, [cb = m_finishedCb, result]() { cb(result); },
          Qt::QueuedConnection);
    }
  }

  static QString formatSize(qint64 bytes) {
    double dBytes = static_cast<double>(bytes);
    if (bytes < 1024) {
      return QString("%1 B").arg(bytes);
    } else if (bytes < 1024 * 1024) {
      return QString("%1 KB").arg(QString::number(dBytes / 1024.0, 'f', 1));
    } else if (bytes < 1024 * 1024 * 1024) {
      return QString("%1 MB").arg(
          QString::number(dBytes / (1024.0 * 1024.0), 'f', 1));
    } else {
      return QString("%1 GB").arg(
          QString::number(dBytes / (1024.0 * 1024.0 * 1024.0), 'f', 2));
    }
  }

  void scanDirectory(const QDir &currentDir, const QStringList &nameFilters,
                     QSet<QString> &visitedDirs, QStringList &discoveredDirs,
                     QList<ScreenshotItem> &items, qint64 &total) {
    if (isCancelled()) {
      return;
    }

    // Scan files in this directory
    QFileInfoList fileEntries =
        currentDir.entryInfoList(nameFilters, QDir::Files | QDir::Readable);
    for (const auto &info : fileEntries) {
      if (isCancelled()) {
        return;
      }

      ScreenshotItem item;
      item.fileName = info.fileName();
      item.filePath = info.absoluteFilePath();
      item.fileSize = info.size();
      total += item.fileSize;
      item.formattedSize = formatSize(item.fileSize);
      item.createdAt = info.lastModified();
      item.formattedDate = item.createdAt.toString("yyyy-MM-dd hh:mm");

      // Extract image size safely with QImageReader without loading full pixel
      // buffer
      QImageReader reader(item.filePath);
      reader.setAutoTransform(true);
      QSize size = reader.size();
      if (size.isValid()) {
        item.width = size.width();
        item.height = size.height();
        item.resolution =
            QString("%1 × %2").arg(size.width()).arg(size.height());
      } else {
        item.width = 0;
        item.height = 0;
        item.resolution = "Bilinmiyor";
      }

      if (m_cache) {
        item.thumbnailUrl = m_cache->getThumbnailPath(
            item.filePath, item.fileSize, item.createdAt);
      }
      if (item.thumbnailUrl.isEmpty()) {
        item.thumbnailUrl = QUrl::fromLocalFile(item.filePath).toString();
      }

      items.append(item);

      if (items.size() % 25 == 0) {
        dispatchProgress(static_cast<int>(items.size()));
      }
    }

    // Scan subdirectories
    QFileInfoList subDirs = currentDir.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable);
    for (const auto &subDirInfo : subDirs) {
      if (isCancelled()) {
        return;
      }

      QString canonical = subDirInfo.canonicalFilePath();
      if (canonical.isEmpty()) {
        canonical = subDirInfo.absoluteFilePath();
      }

      // Check for symlink loops
      if (visitedDirs.contains(canonical)) {
        continue;
      }
      visitedDirs.insert(canonical);
      discoveredDirs.append(subDirInfo.absoluteFilePath());

      QDir nextDir(subDirInfo.absoluteFilePath());
      scanDirectory(nextDir, nameFilters, visitedDirs, discoveredDirs, items,
                    total);
    }
  }

  uint64_t m_generation{0};
  QString m_rootDir;
  std::shared_ptr<ThumbnailCache> m_cache;
  std::shared_ptr<std::atomic<bool>> m_cancelFlag;
  QPointer<QObject> m_receiver;
  std::function<void(int)> m_progressCb;
  std::function<void(ScanResult)> m_finishedCb;
};

} // namespace ro_screenshot
