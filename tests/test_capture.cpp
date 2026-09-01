#include "core/CaptureEngine.hpp"
#include "core/LibraryManager.hpp"
#include "core/SettingsManager.hpp"
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace ro_screenshot;

class TestCapture : public QObject {
  Q_OBJECT

private slots:
  void testEngineInitialization() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SettingsManager settings;
    settings.setSaveDirectory(tempDir.path());
    settings.setAutoSaveToDisk(true);
    settings.setAutoCopyToClipboard(false);

    LibraryManager library(&settings);
    CaptureEngine engine(&settings, &library);

    QCOMPARE(engine.isCapturing(), false);
    QVERIFY(!engine.frozenFramePath().isEmpty());
  }
};

QTEST_MAIN(TestCapture)
#include "test_capture.moc"
