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
}

void TestUiPreferences::persistAndRestore() {
  {
    UiPreferencesManager preferences;
    QSignalSpy themeSpy(&preferences, &UiPreferencesManager::themeModeChanged);

    preferences.setThemeMode(QStringLiteral("dark"));

    QCOMPARE(themeSpy.count(), 1);
  }

  UiPreferencesManager reloadedPreferences;
  QCOMPARE(reloadedPreferences.themeMode(), QStringLiteral("dark"));
}

void TestUiPreferences::normalizesInvalidMode() {
  UiPreferencesManager preferences;

  preferences.setThemeMode(QStringLiteral("midnight"));
  QVERIFY(preferences.themeMode() == QStringLiteral("light") ||
          preferences.themeMode() == QStringLiteral("dark"));
}

void TestUiPreferences::resetToDefaults() {
  UiPreferencesManager preferences;

  preferences.setThemeMode(QStringLiteral("dark"));
  preferences.resetToDefaults();
  QVERIFY(preferences.themeMode() == QStringLiteral("light") ||
          preferences.themeMode() == QStringLiteral("dark"));
}

QTEST_GUILESS_MAIN(TestUiPreferences)

#include "test_ui_preferences.moc"