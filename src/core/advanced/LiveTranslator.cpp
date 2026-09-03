#include "LiveTranslator.hpp"

namespace ro_screenshot {

LiveTranslator::LiveTranslator(QObject *parent) : QObject(parent) {
  // Built-in offline technical phrasebook (English to Turkish, German, Spanish)
  m_phrasebook["error"]["tr"] = "hata";
  m_phrasebook["error"]["de"] = "Fehler";
  m_phrasebook["error"]["es"] = "error";

  m_phrasebook["warning"]["tr"] = "uyarı";
  m_phrasebook["warning"]["de"] = "Warnung";
  m_phrasebook["warning"]["es"] = "advertencia";

  m_phrasebook["success"]["tr"] = "başarılı";
  m_phrasebook["success"]["de"] = "Erfolg";
  m_phrasebook["success"]["es"] = "éxito";

  m_phrasebook["file not found"]["tr"] = "dosya bulunamadı";
  m_phrasebook["permission denied"]["tr"] = "erişim engellendi";
  m_phrasebook["connection refused"]["tr"] = "bağlantı reddedildi";
  m_phrasebook["syntax error"]["tr"] = "sözdizimi hatası";
  m_phrasebook["segmentation fault"]["tr"] =
      "bellek erişim ihlali (segmentation fault)";
}

QStringList LiveTranslator::supportedLanguages() {
  return {QStringLiteral("tr"), QStringLiteral("en"), QStringLiteral("de"),
          QStringLiteral("es"), QStringLiteral("fr")};
}

QString LiveTranslator::dictionaryLookup(const QString &phrase,
                                         const QString &targetLang) const {
  QString lower = phrase.trimmed().toLower();
  if (m_phrasebook.contains(lower) &&
      m_phrasebook[lower].contains(targetLang)) {
    return m_phrasebook[lower][targetLang];
  }
  return {};
}

QString LiveTranslator::translateText(const QString &text,
                                      const QString & /*sourceLang*/,
                                      const QString &targetLang) {
  if (text.isEmpty())
    return {};

  QString lookup = dictionaryLookup(text, targetLang);
  if (!lookup.isEmpty())
    return lookup;

  // Fallback direct word substitutions
  QString result = text;
  for (auto it = m_phrasebook.constBegin(); it != m_phrasebook.constEnd();
       ++it) {
    if (it.value().contains(targetLang)) {
      result.replace(it.key(), it.value()[targetLang], Qt::CaseInsensitive);
    }
  }
  return result;
}

QVector<TranslatedBlock>
LiveTranslator::translateBlocks(const QVector<OcrTextBlock> &blocks,
                                const QString &targetLang) {
  QVector<TranslatedBlock> out;
  out.reserve(blocks.size());

  for (const auto &b : blocks) {
    TranslatedBlock tb;
    tb.originalText = b.text;
    tb.boundingBox = b.boundingBox;
    tb.targetLang = targetLang;
    tb.translatedText = translateText(b.text, "auto", targetLang);
    out.append(tb);
  }
  return out;
}

} // namespace ro_screenshot
