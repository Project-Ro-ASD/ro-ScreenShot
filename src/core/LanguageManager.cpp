#include "LanguageManager.hpp"
#include <QDir>
#include <QLocale>
#include <QSettings>

namespace ro_screenshot {

namespace {
struct LanguageEntry {
  const char *code;
  const char *nativeLabel;
  const char *label;
};

constexpr LanguageEntry kSupportedLanguages[] = {{"tr", "Türkçe", "Turkish"},
                                                 {"en", "English", "English"},
                                                 {"de", "Deutsch", "German"},
                                                 {"es", "Español", "Spanish"}};

bool isSupported(const QString &code) {
  for (const auto &entry : kSupportedLanguages) {
    if (QString::fromLatin1(entry.code) == code) {
      return true;
    }
  }
  return false;
}
} // namespace

LanguageManager::LanguageManager(QCoreApplication *app, QQmlEngine *engine,
                                 QTranslator *translator, QObject *parent)
    : QObject(parent), m_app(app), m_engine(engine), m_translator(translator) {
  QSettings settings("ro-asd", "ro-screenshot");
  // Before this key existed, the application persisted the detected system
  // language as if it were a user choice. Prefer native system following for
  // that legacy state; an explicit selection creates an override below.
  m_followsSystemLanguage =
      settings.value("General/FollowSystemLanguage", true).toBool();
  if (m_followsSystemLanguage) {
    applyLanguage(normalizeLanguageCode(systemLanguageCode()));
  } else {
    applyLanguage(normalizeLanguageCode(
        settings.value("General/LanguageOverride", "en").toString()));
  }
}

QString LanguageManager::currentLanguage() const { return m_currentLanguage; }

QString LanguageManager::currentLanguageLabel() const {
  return displayNameForLanguage(m_currentLanguage);
}

bool LanguageManager::followsSystemLanguage() const {
  return m_followsSystemLanguage;
}

QString LanguageManager::systemLanguageLabel() const {
  return displayNameForLanguage(normalizeLanguageCode(systemLanguageCode()));
}

QVariantList LanguageManager::availableLanguages() const {
  QVariantList list;
  for (const auto &entry : kSupportedLanguages) {
    QVariantMap map;
    map["code"] = QString::fromLatin1(entry.code);
    map["nativeLabel"] = QString::fromUtf8(entry.nativeLabel);
    map["label"] = QString::fromUtf8(entry.label);
    list.append(map);
  }
  return list;
}

void LanguageManager::setCurrentLanguage(const QString &languageCode) {
  const QString normalized = normalizeLanguageCode(languageCode);
  const bool changed =
      m_followsSystemLanguage || m_currentLanguage != normalized;
  m_followsSystemLanguage = false;
  applyLanguage(normalized);

  QSettings settings("ro-asd", "ro-screenshot");
  settings.setValue("General/FollowSystemLanguage", false);
  settings.setValue("General/LanguageOverride", m_currentLanguage);
  settings.remove("General/Language");

  if (changed) {
    emit currentLanguageChanged();
  }
}

void LanguageManager::useSystemLanguage() {
  const QString systemLanguage = normalizeLanguageCode(systemLanguageCode());
  const bool changed =
      !m_followsSystemLanguage || m_currentLanguage != systemLanguage;
  m_followsSystemLanguage = true;
  applyLanguage(systemLanguage);

  QSettings settings("ro-asd", "ro-screenshot");
  settings.setValue("General/FollowSystemLanguage", true);
  settings.remove("General/LanguageOverride");
  settings.remove("General/Language");

  if (changed) {
    emit currentLanguageChanged();
  }
}

QString
LanguageManager::displayNameForLanguage(const QString &languageCode) const {
  for (const auto &entry : kSupportedLanguages) {
    if (QString::fromLatin1(entry.code) == languageCode) {
      return QString::fromUtf8(entry.nativeLabel);
    }
  }
  return languageCode;
}

QString LanguageManager::normalizeLanguageCode(const QString &code) const {
  QString lower = code.trimmed().toLower();
  if (lower.contains('_')) {
    lower = lower.section('_', 0, 0);
  } else if (lower.contains('-')) {
    lower = lower.section('-', 0, 0);
  }

  if (isSupported(lower)) {
    return lower;
  }
  return "en";
}

QString LanguageManager::systemLanguageCode() const {
  return QLocale::system().name().section('_', 0, 0).toLower();
}

void LanguageManager::applyLanguage(const QString &code) {
  if (m_currentLanguage == code && m_app && m_translator &&
      !m_translator->isEmpty()) {
    return;
  }
  loadLanguage(code);
  m_currentLanguage = code;
}

bool LanguageManager::loadLanguage(const QString &code) {
  if (!m_app || !m_translator || !m_engine) {
    return false;
  }

  m_app->removeTranslator(m_translator);

  bool loaded = false;
  QString qmPath = QString(":/i18n/ro-screenshot_%1.qm").arg(code);

  loaded = m_translator->load(qmPath);

  if (loaded) {
    m_app->installTranslator(m_translator);
  }

  m_engine->setUiLanguage(code);
  m_engine->retranslate();
  return true;
}

} // namespace ro_screenshot
