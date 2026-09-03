#include "QrCodeDetector.hpp"
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryFile>

namespace ro_screenshot {

QrCodeDetector::QrCodeDetector(QObject *parent) : QObject(parent) {}

bool QrCodeDetector::isAvailable() const {
  return !QStandardPaths::findExecutable(QStringLiteral("zbarimg")).isEmpty();
}

QrSmartAction QrCodeDetector::parsePayload(const QString &raw) {
  QrSmartAction act;
  QString trimmed = raw.trimmed();

  // 1. URL
  if (trimmed.startsWith("http://", Qt::CaseInsensitive) ||
      trimmed.startsWith("https://", Qt::CaseInsensitive)) {
    act.actionType = QrActionType::OpenUrl;
    act.label = QStringLiteral("Tarayıcıda Aç");
    act.targetUrl = trimmed;
    return act;
  }

  // 2. Wi-Fi (WIFI:S:MySSID;T:WPA;P:MyPassword;;)
  if (trimmed.startsWith("WIFI:", Qt::CaseInsensitive)) {
    act.actionType = QrActionType::ConnectWifi;
    act.label = QStringLiteral("Wi-Fi Ağına Bağlan");

    QRegularExpression ssidRegex("S:([^;]+)");
    auto ssidMatch = ssidRegex.match(trimmed);
    if (ssidMatch.hasMatch())
      act.wifiSsid = ssidMatch.captured(1);

    QRegularExpression passRegex("P:([^;]+)");
    auto passMatch = passRegex.match(trimmed);
    if (passMatch.hasMatch())
      act.wifiPassword = passMatch.captured(1);

    QRegularExpression typeRegex("T:([^;]+)");
    auto typeMatch = typeRegex.match(trimmed);
    if (typeMatch.hasMatch())
      act.wifiAuthType = typeMatch.captured(1);

    return act;
  }

  // 3. vCard
  if (trimmed.contains("BEGIN:VCARD", Qt::CaseInsensitive)) {
    act.actionType = QrActionType::SaveVCard;
    act.label = QStringLiteral("Kişi Kartını Kaydet");

    QRegularExpression fnRegex("FN:(.+?)(?:\r|\n)");
    auto fnMatch = fnRegex.match(trimmed);
    if (fnMatch.hasMatch())
      act.vcardName = fnMatch.captured(1).trimmed();

    QRegularExpression telRegex("TEL[^:]*:(.+?)(?:\r|\n)");
    auto telMatch = telRegex.match(trimmed);
    if (telMatch.hasMatch())
      act.vcardPhone = telMatch.captured(1).trimmed();

    QRegularExpression emailRegex("EMAIL[^:]*:(.+?)(?:\r|\n)");
    auto emailMatch = emailRegex.match(trimmed);
    if (emailMatch.hasMatch())
      act.vcardEmail = emailMatch.captured(1).trimmed();

    return act;
  }

  // 4. Crypto Address (Bitcoin, Ethereum, Solana)
  if (trimmed.startsWith("bitcoin:", Qt::CaseInsensitive) ||
      trimmed.startsWith("ethereum:", Qt::CaseInsensitive) ||
      trimmed.startsWith("0x") || trimmed.startsWith("bc1") ||
      trimmed.startsWith("1") || trimmed.startsWith("3")) {
    if (trimmed.length() >= 26 && trimmed.length() <= 64 &&
        !trimmed.contains(" ")) {
      act.actionType = QrActionType::CryptoAddress;
      act.label = QStringLiteral("Kripto Adresini Kopyala");
      act.cryptoAddress = trimmed;
      return act;
    }
  }

  // Default: Plain Text copy
  act.actionType = QrActionType::CopyText;
  act.label = QStringLiteral("Panoya Kopyala");
  return act;
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
        result.action = parsePayload(output);

        switch (result.action.actionType) {
        case QrActionType::OpenUrl:
          result.type = QStringLiteral("URL");
          break;
        case QrActionType::ConnectWifi:
          result.type = QStringLiteral("WIFI");
          break;
        case QrActionType::SaveVCard:
          result.type = QStringLiteral("VCARD");
          break;
        case QrActionType::CryptoAddress:
          result.type = QStringLiteral("CRYPTO");
          break;
        default:
          result.type = QStringLiteral("QR-Code");
          break;
        }

        result.boundingBox = image.rect();
      }
    }
  }

  QFile::remove(tempPath);
  return result;
}

} // namespace ro_screenshot
