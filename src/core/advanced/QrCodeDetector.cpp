#include "QrCodeDetector.hpp"
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryFile>

namespace ro_screenshot {

QrCodeDetector::QrCodeDetector(QObject *parent) : QObject(parent) {}

bool QrCodeDetector::isAvailable() const {
  return !QStandardPaths::findExecutable(QStringLiteral("zbarimg")).isEmpty();
}

QrScanResult QrCodeDetector::scan(const QImage &image) {
  if (image.isNull()) {
    return {};
  }
  return scanWithZbar(image);
}

QrScanResult QrCodeDetector::scanRegion(const QImage &image,
                                        const QRect &region) {
  if (region.isValid() && !region.isEmpty()) {
    return scan(image.copy(region));
  }
  return scan(image);
}

QVector<QrScanResult> QrCodeDetector::scanAll(const QImage &image) {
  QVector<QrScanResult> results;
  const QrScanResult single = scan(image);
  if (single.found) {
    results.append(single);
  }
  return results;
}

QrScanResult QrCodeDetector::scanWithZbar(const QImage &image) {
  QrScanResult result;
  const QString zbarimg =
      QStandardPaths::findExecutable(QStringLiteral("zbarimg"));
  if (zbarimg.isEmpty()) {
    return result;
  }

  QTemporaryFile tempImg;
  tempImg.setFileTemplate(QDir::tempPath() + "/ro_qr_XXXXXX.png");
  if (!tempImg.open()) {
    return result;
  }
  const QString tempPath = tempImg.fileName();
  tempImg.close();

  if (!image.save(tempPath, "PNG")) {
    QFile::remove(tempPath);
    return result;
  }

  QProcess process;
  process.start(zbarimg, QStringList{QStringLiteral("--raw"),
                                     QStringLiteral("-q"), tempPath});
  if (process.waitForFinished(5000)) {
    if (process.exitCode() == 0) {
      const QString output =
          QString::fromUtf8(process.readAllStandardOutput()).trimmed();
      if (!output.isEmpty()) {
        result.found = true;
        result.text = output;
        result.type = output.startsWith(QStringLiteral("http://")) ||
                              output.startsWith(QStringLiteral("https://"))
                          ? QStringLiteral("URL")
                          : QStringLiteral("QR-Code");
        result.boundingBox = image.rect();
      }
    }
  }

  QFile::remove(tempPath);
  return result;
}

} // namespace ro_screenshot
