#include "core/SettingsManager.hpp"
#include <QSignalSpy>
#include <QTest>

using namespace ro_screenshot;

class TestSettings : public QObject {
  Q_OBJECT

private slots:
  void initTestCase() {
    QCoreApplication::setOrganizationName("ro-asd-test");
    QCoreApplication::setApplicationName("ro-screenshot-test");
  }

  void testDefaults() {
    SettingsManager settings;
    settings.resetToDefaults();

    QCOMPARE(settings.imageFormat(), QString("png"));
    QCOMPARE(settings.jpegQuality(), 90);
    QCOMPARE(settings.autoCopyToClipboard(), true);
    QCOMPARE(settings.autoSaveToDisk(), true);
    QCOMPARE(settings.showFloatingThumbnail(), true);
    QCOMPARE(settings.magnifierEnabled(), true);
    QCOMPARE(settings.magnifierZoom(), 8);
  }

  void testFileNameFormatting() {
    SettingsManager settings;
    settings.setFileNameTemplate("Screenshot_%Y%m%d_%H%M%S");
    settings.setImageFormat("png");

    QDateTime dt(QDate(2026, 9, 2), QTime(14, 30, 45));
    QString formatted = settings.formatFileName(dt);

    QCOMPARE(formatted, QString("Screenshot_20260902_143045.png"));
  }

  void testJpegFormatAndClamping() {
    SettingsManager settings;
    settings.setImageFormat("jpeg");
    QCOMPARE(settings.imageFormat(), QString("jpg"));

    settings.setJpegQuality(150); // should clamp to 100
    QCOMPARE(settings.jpegQuality(), 100);

    settings.setJpegQuality(-10); // should clamp to 1
    QCOMPARE(settings.jpegQuality(), 1);
  }

  void testSignals() {
    SettingsManager settings;
    QSignalSpy spyDir(&settings, &SettingsManager::saveDirectoryChanged);
    QSignalSpy spyFmt(&settings, &SettingsManager::imageFormatChanged);

    settings.setSaveDirectory("/tmp/test_screenshots");
    QCOMPARE(spyDir.count(), 1);

    settings.setImageFormat("webp");
    QCOMPARE(spyFmt.count(), 1);
  }
};

QTEST_MAIN(TestSettings)
#include "test_settings.moc"
