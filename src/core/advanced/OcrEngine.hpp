#pragma once

#include <QImage>
#include <QObject>
#include <QRect>
#include <QString>
#include <QVector>

namespace ro_screenshot {

struct OcrTextBlock {
  QString text;
  QRect boundingBox;
  double confidence{1.0};
};

struct OcrResult {
  bool success{false};
  QString fullText;
  QVector<OcrTextBlock> blocks;
  QString errorMessage;
  QString language;
};

class OcrEngine : public QObject {
  Q_OBJECT

public:
  explicit OcrEngine(QObject *parent = nullptr);
  ~OcrEngine() override = default;

  bool isAvailable() const;
  QString defaultLanguage() const;
  void setDefaultLanguage(const QString &lang);
  QStringList availableLanguages() const;

  OcrResult recognize(const QImage &image, const QString &language = QString());
  OcrResult recognizeRegion(const QImage &image, const QRect &region,
                            const QString &language = QString());

private:
  QString m_language{QStringLiteral("eng+tur")};
};

} // namespace ro_screenshot
