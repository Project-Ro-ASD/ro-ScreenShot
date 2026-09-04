#include "MetadataSanitizer.hpp"
#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QProcess>
#include <QStandardPaths>

namespace ro_screenshot {

MetadataSanitizer::MetadataSanitizer(QObject *parent) : QObject(parent) {}

bool MetadataSanitizer::sanitizeFile(const QString &sourcePath,
                                     const QString &destPath) {
  QImage img;
  QImageReader reader(sourcePath);
  reader.setAutoTransform(true);
  if (!reader.read(&img) || img.isNull()) {
    return false;
  }

  for (const QString &key : img.textKeys()) {
    img.setText(key, QString());
  }

  QString target = destPath.isEmpty() ? sourcePath : destPath;
  QFileInfo fi(sourcePath);
  QString fmt = fi.suffix().toUpper();
  if (fmt.isEmpty())
    fmt = QStringLiteral("PNG");

  QImageWriter writer(target, fmt.toLatin1());
  writer.setQuality(95);
  return writer.write(img);
}

QByteArray MetadataSanitizer::sanitizeBytes(const QByteArray &rawImageData,
                                            const QString &format) {
  QImage img;
  if (!img.loadFromData(rawImageData, format.toLatin1().constData()) ||
      img.isNull()) {
    return rawImageData;
  }

  for (const QString &key : img.textKeys()) {
    img.setText(key, QString());
  }

  QByteArray outData;
  QBuffer buffer(&outData);
  buffer.open(QIODevice::WriteOnly);
  QImageWriter writer(&buffer, format.toLatin1().constData());
  writer.setQuality(95);
  if (writer.write(img)) {
    return outData;
  }
  return rawImageData;
}

CompressionStats MetadataSanitizer::optimizeLossless(const QString &filePath,
                                                     int /*compressionLevel*/) {
  CompressionStats stats;
  QFileInfo fi(filePath);
  if (!fi.exists()) {
    stats.errorMessage = "File does not exist.";
    return stats;
  }

  stats.originalBytes = fi.size();

  QString suffix = fi.suffix().toLower();
  if (suffix == "png") {
    QString oxipng = QStandardPaths::findExecutable(QStringLiteral("oxipng"));
    QString optipng = QStandardPaths::findExecutable(QStringLiteral("optipng"));
    if (!oxipng.isEmpty()) {
      QProcess proc;
      proc.start(oxipng, {"-o", "4", "--strip", "all", filePath});
      proc.waitForFinished(10000);
    } else if (!optipng.isEmpty()) {
      QProcess proc;
      proc.start(optipng, {"-o7", "-strip", "all", filePath});
      proc.waitForFinished(10000);
    } else {
      sanitizeFile(filePath, filePath);
    }
  } else if (suffix == "webp") {
    QString cwebp = QStandardPaths::findExecutable(QStringLiteral("cwebp"));
    if (!cwebp.isEmpty()) {
      QProcess proc;
      proc.start(cwebp, {"-lossless", "-q", "100", filePath, "-o", filePath});
      proc.waitForFinished(10000);
    } else {
      sanitizeFile(filePath, filePath);
    }
  } else {
    sanitizeFile(filePath, filePath);
  }

  QFileInfo newFi(filePath);
  stats.optimizedBytes = newFi.size();
  stats.success = true;
  if (stats.originalBytes > 0) {
    stats.reductionPercent =
        (static_cast<double>(stats.originalBytes - stats.optimizedBytes) /
         static_cast<double>(stats.originalBytes)) *
        100.0;
  }
  return stats;
}

} // namespace ro_screenshot
