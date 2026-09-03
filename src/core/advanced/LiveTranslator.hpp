#pragma once

#include "core/advanced/OcrEngine.hpp"
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QVector>

namespace ro_screenshot {

struct TranslatedBlock {
  QString originalText;
  QString translatedText;
  QRect boundingBox;
  QString sourceLang;
  QString targetLang;
};

class LiveTranslator : public QObject {
  Q_OBJECT

public:
  explicit LiveTranslator(QObject *parent = nullptr);
  ~LiveTranslator() override = default;

  Q_INVOKABLE QString translateText(const QString &text,
                                    const QString &sourceLang = "auto",
                                    const QString &targetLang = "tr");
  Q_INVOKABLE QVector<TranslatedBlock>
  translateBlocks(const QVector<OcrTextBlock> &blocks,
                  const QString &targetLang = "tr");

  static QStringList supportedLanguages();

signals:
  void translationFinished(const QString &translatedText);
  void translationFailed(const QString &errorMessage);

private:
  QString dictionaryLookup(const QString &phrase,
                           const QString &targetLang) const;
  QMap<QString, QMap<QString, QString>> m_phrasebook;
};

} // namespace ro_screenshot
