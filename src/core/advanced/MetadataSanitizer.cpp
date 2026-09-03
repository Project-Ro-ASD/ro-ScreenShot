#include "MetadataSanitizer.hpp"
#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QProcess>

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
    fmt = "PNG";

  QImageWriter writer(target, fmt.toLatin1());
  writer.setQuality(100);
  writer.setCompression(9);
  return writer.write(img);
}

QByteArray MetadataSanitizer::sanitizeBytes(const QByteArray &rawImageData,
                                            const QString &format) {
  QImage img;
  if (!img.loadFromData(rawImageData))
    return rawImageData;

  for (const QString &key : img.textKeys()) {
    img.setText(key, QString());
  }

  QByteArray out;
  QBuffer buf(&out);
  buf.open(QIODevice::WriteOnly);
  img.save(&buf, format.toLatin1().constData());
  return out;
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
    if (QProcess::execute("which", {"oxipng"}) == 0) {
      QProcess proc;
      proc.start("oxipng", {"-o", "4", "--strip", "all", filePath});
      proc.waitForFinished(10000);
    } else if (QProcess::execute("which", {"optipng"}) == 0) {
      QProcess proc;
      proc.start("optipng", {"-o7", "-strip", "all", filePath});
      proc.waitForFinished(10000);
    } else {
      sanitizeFile(filePath, filePath);
    }
  } else if (suffix == "webp") {
    if (QProcess::execute("which", {"cwebp"}) == 0) {
      QProcess proc;
      proc.start("cwebp", {"-lossless", "-q", "100", filePath, "-o", filePath});
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
