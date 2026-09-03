#include "core/SettingsManager.hpp"
#include <QFile>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace ro_screenshot;

class TestSettings : public QObject {
  Q_OBJECT

  QTemporaryDir m_configDir;

private slots:
  void initTestCase() {
    QVERIFY(m_configDir.isValid());
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope,
                       m_configDir.path());
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
    QVERIFY(!settings.lastRegion().isValid());
  }

  void testGeneratedPathDoesNotOverwriteExistingCapture() {
    QTemporaryDir outputDir;
    QVERIFY(outputDir.isValid());

    SettingsManager settings;
    settings.setSaveDirectory(outputDir.path());
    settings.setFileNameTemplate("collision-test");
    settings.setImageFormat("png");

    const QDateTime timestamp = QDateTime::fromSecsSinceEpoch(1700000000);
    const QString firstPath = settings.generateFullPath(timestamp);
    QFile existing(firstPath);
    QVERIFY(existing.open(QIODevice::WriteOnly));
    existing.close();

    const QString secondPath = settings.generateFullPath(timestamp);
    QVERIFY(secondPath != firstPath);
    QVERIFY(secondPath.endsWith("collision-test_1.png"));
  }

  void testLastRegionPersistence() {
    SettingsManager settings;
    const QRect region(12, 24, 640, 360);
    QSignalSpy spy(&settings, &SettingsManager::lastRegionChanged);
    settings.setLastRegion(region);
    QCOMPARE(settings.lastRegion(), region);
    QCOMPARE(spy.count(), 1);

    SettingsManager reloaded;
    QCOMPARE(reloaded.lastRegion(), region);
  }

  void testLastRegionGeometryPersistence() {
    SettingsManager settings;
    const QRect region(12, 24, 640, 360);
    const QSize frameSize(1920, 1080);
    settings.setLastRegionGeometry(region, frameSize);

    QCOMPARE(settings.lastRegion(), region);
    QCOMPARE(settings.lastRegionFrameSize(), frameSize);

    SettingsManager reloaded;
    QCOMPARE(reloaded.lastRegionFrameSize(), frameSize);

    reloaded.setLastRegion({});
    QVERIFY(!reloaded.lastRegionFrameSize().isValid());
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
