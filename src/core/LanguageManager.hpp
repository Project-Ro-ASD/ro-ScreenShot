#pragma once

#include <QCoreApplication>
#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QTranslator>
#include <QVariantList>

namespace ro_screenshot {

class LanguageManager : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString currentLanguage READ currentLanguage WRITE
                 setCurrentLanguage NOTIFY currentLanguageChanged)
  Q_PROPERTY(QString currentLanguageLabel READ currentLanguageLabel NOTIFY
                 currentLanguageChanged)
  Q_PROPERTY(QVariantList availableLanguages READ availableLanguages CONSTANT)

public:
  explicit LanguageManager(QCoreApplication *app, QQmlEngine *engine,
                           QTranslator *translator, QObject *parent = nullptr);
  ~LanguageManager() override = default;

  QString currentLanguage() const;
  QString currentLanguageLabel() const;
  QVariantList availableLanguages() const;

  Q_INVOKABLE void setCurrentLanguage(const QString &languageCode);
  Q_INVOKABLE QString displayNameForLanguage(const QString &languageCode) const;

signals:
  void currentLanguageChanged();

private:
  QString normalizeLanguageCode(const QString &code) const;
  QString systemLanguageCode() const;
  bool loadLanguage(const QString &code);

  QCoreApplication *m_app{nullptr};
  QQmlEngine *m_engine{nullptr};
  QTranslator *m_translator{nullptr};
  QString m_currentLanguage{"tr"};
};

} // namespace ro_screenshot
