#pragma once

#include <QByteArray>
#include <QImage>
#include <QObject>
#include <QString>

namespace ro_screenshot {

struct CompressionStats {
  qint64 originalBytes{0};
  qint64 optimizedBytes{0};
  double reductionPercent{0.0};
  bool success{false};
  QString errorMessage;
};

class MetadataSanitizer : public QObject {
  Q_OBJECT

public:
  explicit MetadataSanitizer(QObject *parent = nullptr);
  ~MetadataSanitizer() override = default;

  // Sanitizes image by reloading raw pixel buffer and writing clean container
  // without metadata chunks
  static bool sanitizeFile(const QString &sourcePath,
                           const QString &destPath = QString());
  static QByteArray sanitizeBytes(const QByteArray &rawImageData,
                                  const QString &format = "PNG");

  // Lossless PNG / WebP compression optimizer
  static CompressionStats optimizeLossless(const QString &filePath,
                                           int compressionLevel = 9);
};

} // namespace ro_screenshot
