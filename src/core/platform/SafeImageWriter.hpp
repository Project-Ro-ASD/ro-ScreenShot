#pragma once

#include "core/Types.hpp"
#include <QDateTime>
#include <QDir>
#include <QImage>
#include <QMutex>
#include <QObject>
#include <QString>

namespace ro_screenshot {

struct SaveResult {
  bool success{false};
  CaptureErrorCode errorCode{CaptureErrorCode::None};
  QString errorMessage;
  QString filePath;
  qint64 bytesWritten{0};
};

class SafeImageWriter : public QObject {
  Q_OBJECT

public:
  explicit SafeImageWriter(QObject *parent = nullptr);
  ~SafeImageWriter() override;

  // Atomic file writer
  SaveResult writeImageAtomically(const QImage &image,
                                  const QString &destinationPath,
                                  const QString &format = QStringLiteral("PNG"),
                                  int quality = -1);

  // Disk & Permission validation
  bool checkDiskSpace(const QString &targetDirectory, qint64 requiredBytes,
                      QString *errorMessage = nullptr) const;
  bool checkDirectoryWritable(const QString &targetDirectory,
                              QString *errorMessage = nullptr) const;
  bool isFormatSupported(const QString &format) const;

  // Collision-free path generator
  QString resolveUniqueFilePath(const QString &targetPath) const;

  // Temporary file management
  QString createTempFilePath(const QString &prefix = QStringLiteral("ro_frame"),
                             const QString &suffix = QStringLiteral(".png"));
  void registerTempFile(const QString &filePath);
  void cleanupTempFiles();
  void cleanupFile(const QString &filePath);

  // In-memory safe image cache
  void setCachedImage(const QImage &image);
  QImage cachedImage() const;
  void clearCachedImage();

private:
  mutable QMutex m_cacheMutex;
  QImage m_cachedImage;
  QStringList m_activeTempFiles;
};

} // namespace ro_screenshot
