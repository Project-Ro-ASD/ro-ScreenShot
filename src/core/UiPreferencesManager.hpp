#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

namespace ro_screenshot {

class UiPreferencesManager : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString themeMode READ themeMode WRITE setThemeMode NOTIFY
                 themeModeChanged)
  Q_PROPERTY(bool sidebarCollapsed READ isSidebarCollapsed WRITE
                 setSidebarCollapsed NOTIFY sidebarCollapsedChanged)
  Q_PROPERTY(QVariantList availableThemeModes READ availableThemeModes CONSTANT)

public:
  explicit UiPreferencesManager(QObject *parent = nullptr);

  QString themeMode() const;
  bool isSidebarCollapsed() const;
  QVariantList availableThemeModes() const;

  Q_INVOKABLE void setThemeMode(const QString &themeMode);
  Q_INVOKABLE void setSidebarCollapsed(bool collapsed);
  Q_INVOKABLE void toggleSidebar();
  Q_INVOKABLE void resetToDefaults();

signals:
  void themeModeChanged();
  void sidebarCollapsedChanged();

private:
  QString normalizeThemeMode(const QString &themeMode) const;
  void persistValue(const QString &key, const QVariant &value) const;
  bool detectSystemDarkMode() const;
  QString systemThemeMode() const;

  QString m_themeMode = QStringLiteral("light");
  bool m_systemDarkMode = false;
  bool m_sidebarCollapsed = false;
};

} // namespace ro_screenshot