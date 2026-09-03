#pragma once

#include <QImage>
#include <QObject>
#include <QRect>
#include <QString>
#include <QVector>

namespace ro_screenshot {

struct QrScanResult {
  bool found{false};
  QString text;
  QString type; // e.g. "QR-Code", "EAN-13", "URL"
  QRect boundingBox;
};

class QrCodeDetector : public QObject {
  Q_OBJECT

public:
  explicit QrCodeDetector(QObject *parent = nullptr);
  ~QrCodeDetector() override = default;

  bool isAvailable() const;
  QrScanResult scan(const QImage &image);
  QrScanResult scanRegion(const QImage &image, const QRect &region);
  QVector<QrScanResult> scanAll(const QImage &image);

private:
  QrScanResult scanWithZbar(const QImage &image);
};

} // namespace ro_screenshot
