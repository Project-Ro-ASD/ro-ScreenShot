#include "OcrEngine.hpp"
#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryFile>

namespace ro_screenshot {

OcrEngine::OcrEngine(QObject *parent) : QObject(parent) {}

bool OcrEngine::isAvailable() const {
  return !QStandardPaths::findExecutable(QStringLiteral("tesseract")).isEmpty();
}

QString OcrEngine::defaultLanguage() const { return m_language; }

void OcrEngine::setDefaultLanguage(const QString &lang) {
  if (!lang.isEmpty()) {
    m_language = lang;
  }
}

QStringList OcrEngine::availableLanguages() const {
  QStringList langs;
  const QString tesseract =
      QStandardPaths::findExecutable(QStringLiteral("tesseract"));
  if (tesseract.isEmpty()) {
    return {QStringLiteral("eng"), QStringLiteral("tur"), QStringLiteral("deu"),
            QStringLiteral("spa")};
  }

  QProcess process;
  process.start(tesseract, QStringList{QStringLiteral("--list-langs")});
  if (process.waitForFinished(3000)) {
    const QString output = QString::fromUtf8(process.readAllStandardOutput());
    const QStringList lines =
        output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (int i = 1; i < lines.size(); ++i) {
      langs.append(lines[i].trimmed());
    }
  }

  if (langs.isEmpty()) {
    langs = {QStringLiteral("eng"), QStringLiteral("tur")};
  }
  return langs;
}

OcrResult OcrEngine::recognize(const QImage &image, const QString &language) {
  OcrResult result;
  if (image.isNull()) {
    result.errorMessage = tr("Tanınacak görsel bulunamadı.");
    return result;
  }

  const QString tesseract =
      QStandardPaths::findExecutable(QStringLiteral("tesseract"));
  if (tesseract.isEmpty()) {
    result.errorMessage = tr("Tesseract OCR sistemi sistemde kurulu değil.");
    return result;
  }

  QTemporaryFile tempImg;
  tempImg.setFileTemplate(QDir::tempPath() + "/ro_ocr_XXXXXX.png");
  if (!tempImg.open()) {
    result.errorMessage = tr("Geçici dosya oluşturulamadı.");
    return result;
  }
  const QString tempPath = tempImg.fileName();
  tempImg.close();

  if (!image.save(tempPath, "PNG")) {
    QFile::remove(tempPath);
    result.errorMessage = tr("Görsel geçici dosyaya yazılamadı.");
    return result;
  }

  const QString lang = language.isEmpty() ? m_language : language;
  result.language = lang;

  QProcess process;
  process.start(tesseract,
                QStringList{tempPath, QStringLiteral("stdout"),
                            QStringLiteral("-l"), lang, QStringLiteral("--psm"),
                            QStringLiteral("3")});

  if (!process.waitForFinished(10000)) {
    process.kill();
    QFile::remove(tempPath);
    result.errorMessage = tr("OCR işlemi zaman aşımına uğradı.");
    return result;
  }

  QFile::remove(tempPath);

  if (process.exitCode() != 0) {
    result.errorMessage =
        QString::fromUtf8(process.readAllStandardError()).trimmed();
    return result;
  }

  result.fullText =
      QString::fromUtf8(process.readAllStandardOutput()).trimmed();
  result.success = true;

  // Split lines into blocks
  const QStringList lines =
      result.fullText.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
  for (const QString &line : lines) {
    OcrTextBlock block;
    block.text = line.trimmed();
    block.confidence = 0.95;
    result.blocks.append(block);
  }

  return result;
}

OcrResult OcrEngine::recognizeRegion(const QImage &image, const QRect &region,
                                     const QString &language) {
  if (region.isValid() && !region.isEmpty()) {
    return recognize(image.copy(region), language);
  }
  return recognize(image, language);
}

} // namespace ro_screenshot
