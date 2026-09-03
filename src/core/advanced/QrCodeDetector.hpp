#pragma once

#include <QImage>
#include <QObject>
#include <QRect>
#include <QString>
#include <QVector>

namespace ro_screenshot {

enum class QrActionType {
  OpenUrl,
  ConnectWifi,
  SaveVCard,
  CryptoAddress,
  CopyText
};

struct QrSmartAction {
  QrActionType actionType{QrActionType::CopyText};
  QString label;
  QString targetUrl;
  QString wifiSsid;
  QString wifiPassword;
  QString wifiAuthType;
  QString vcardName;
  QString vcardPhone;
  QString vcardEmail;
  QString cryptoAddress;
};

struct QrScanResult {
  bool found{false};
  QString text;
  QString type; // "URL", "WIFI", "VCARD", "CRYPTO", "QR-Code"
  QRect boundingBox;
  QrSmartAction action;
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

  static QrSmartAction parsePayload(const QString &rawPayload);

private:
  QrScanResult scanWithZbar(const QImage &image);
};

} // namespace ro_screenshot
