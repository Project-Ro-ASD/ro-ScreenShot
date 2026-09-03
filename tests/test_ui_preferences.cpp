#include <QCoreApplication>
#include <QSettings>
#include <QSignalSpy>
#include <QTest>

#include "core/UiPreferencesManager.hpp"

using namespace ro_screenshot;

class TestUiPreferences : public QObject {
  Q_OBJECT

private slots:
  void init();
  void defaults();
  void persistAndRestore();
  void normalizesInvalidMode();
  void sidebarPreference();
  void resetToDefaults();
};

void TestUiPreferences::init() {
  QCoreApplication::setOrganizationName(
      QStringLiteral("Project-Ro-ASD-TestSuite"));
  QCoreApplication::setApplicationName(
      QStringLiteral("ro-screenshot-ui-prefs"));

  QSettings settings;
  settings.clear();
  settings.sync();
}

void TestUiPreferences::defaults() {
  UiPreferencesManager preferences;

  QVERIFY(preferences.themeMode() == QStringLiteral("light") ||
          preferences.themeMode() == QStringLiteral("dark"));
  QCOMPARE(preferences.availableThemeModes().size(), 2);
  QCOMPARE(preferences.availableThemeModes()
               .first()
               .toMap()
               .value(QStringLiteral("code"))
               .toString(),
           QStringLiteral("light"));
  QCOMPARE(preferences.isSidebarCollapsed(), false);
}

void TestUiPreferences::persistAndRestore() {
  {
    UiPreferencesManager preferences;
    QSignalSpy themeSpy(&preferences, &UiPreferencesManager::themeModeChanged);
    QSignalSpy sidebarSpy(&preferences,
                          &UiPreferencesManager::sidebarCollapsedChanged);

    preferences.setThemeMode(QStringLiteral("dark"));
    preferences.setSidebarCollapsed(true);

    QCOMPARE(themeSpy.count(), 1);
    QCOMPARE(sidebarSpy.count(), 1);
  }

  UiPreferencesManager reloadedPreferences;
  QCOMPARE(reloadedPreferences.themeMode(), QStringLiteral("dark"));
  QCOMPARE(reloadedPreferences.isSidebarCollapsed(), true);
}

void TestUiPreferences::normalizesInvalidMode() {
  UiPreferencesManager preferences;

  preferences.setThemeMode(QStringLiteral("midnight"));
  QVERIFY(preferences.themeMode() == QStringLiteral("light") ||
          preferences.themeMode() == QStringLiteral("dark"));
}

void TestUiPreferences::sidebarPreference() {
  UiPreferencesManager preferences;
  QSignalSpy sidebarSpy(&preferences,
                        &UiPreferencesManager::sidebarCollapsedChanged);

  preferences.toggleSidebar();
  QCOMPARE(preferences.isSidebarCollapsed(), true);
  QCOMPARE(sidebarSpy.count(), 1);

  preferences.toggleSidebar();
  QCOMPARE(preferences.isSidebarCollapsed(), false);
  QCOMPARE(sidebarSpy.count(), 2);
}

void TestUiPreferences::resetToDefaults() {
  UiPreferencesManager preferences;

  preferences.setThemeMode(QStringLiteral("dark"));
  preferences.setSidebarCollapsed(true);
  preferences.resetToDefaults();
  QVERIFY(preferences.themeMode() == QStringLiteral("light") ||
          preferences.themeMode() == QStringLiteral("dark"));
  QCOMPARE(preferences.isSidebarCollapsed(), false);
}

QTEST_GUILESS_MAIN(TestUiPreferences)

#include "test_ui_preferences.moc"