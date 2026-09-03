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

    settings.setLastRegion(QRect(10, 10, 320, 180));
    QVERIFY(engine.hasLastRegion());
  }
};

QTEST_MAIN(TestCapture)
#include "test_capture.moc"
