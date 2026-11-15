#include "ui/ThemeIconProvider.hpp"
#include <QObject>
#include <QQmlContext>
#include <QQmlEngine>
#include <QtQuickTest/quicktest.h>

class ThemePreferences : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString themeMode MEMBER themeMode NOTIFY changed)
  Q_PROPERTY(qreal animationDurationFactor MEMBER factor NOTIFY changed)

public:
  QString themeMode = QStringLiteral("light");
  qreal factor = 1;

signals:
  void changed();
};

class ThemeSetup : public QObject {
  Q_OBJECT
public slots:
  void qmlEngineAvailable(QQmlEngine *engine) {
    engine->addImageProvider(QStringLiteral("ro-theme-icon"),
                             new ro_screenshot::ThemeIconProvider);
    engine->rootContext()->setContextProperty("UiPreferencesManager",
                                              &preferences);
  }

private:
  ThemePreferences preferences;
};

QUICK_TEST_MAIN_WITH_SETUP(ro_theme, ThemeSetup)
#include "test_theme.moc"
