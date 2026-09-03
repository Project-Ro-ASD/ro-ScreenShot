#include "core/CaptureEngine.hpp"
#include "core/LibraryManager.hpp"
#include "core/SettingsManager.hpp"
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace ro_screenshot;

class TestCapture : public QObject {
  Q_OBJECT

  QTemporaryDir m_configDir;

private slots:
  void initTestCase() {
    QVERIFY(m_configDir.isValid());
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope,
                       m_configDir.path());
  }

  void testEngineInitialization() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SettingsManager settings;
    settings.setSaveDirectory(tempDir.path());
    settings.setAutoSaveToDisk(true);
    settings.setAutoCopyToClipboard(false);
    settings.setLastRegion({});

    LibraryManager library(&settings);
    CaptureEngine engine(&settings, &library);

    QCOMPARE(engine.isCapturing(), false);
    QVERIFY(!engine.frozenFramePath().isEmpty());
    QVERIFY(!engine.hasLastRegion());

    settings.setLastRegionGeometry(QRect(10, 10, 320, 180), QSize(1920, 1080));
    QVERIFY(engine.hasLastRegion());
  }

  void testSaveImageAsWritesValidImage() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SettingsManager settings;
    LibraryManager library(&settings);
    CaptureEngine engine(&settings, &library);

    const QString sourcePath = tempDir.path() + "/source.png";
    QImage source(48, 32, QImage::Format_ARGB32_Premultiplied);
    source.fill(Qt::cyan);
    QVERIFY(source.save(sourcePath));

    const QString destinationPath = tempDir.path() + "/nested/export.webp";
    QVERIFY(engine.saveImageAs(sourcePath, destinationPath));

    const QImage exported(destinationPath);
    QVERIFY(!exported.isNull());
    QCOMPARE(exported.size(), source.size());
  }
};

QTEST_MAIN(TestCapture)
#include "test_capture.moc"
