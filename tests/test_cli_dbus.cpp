#include "core/CaptureEngine.hpp"
#include "core/DBusService.hpp"
#include "core/LibraryManager.hpp"
#include "core/SettingsManager.hpp"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace ro_screenshot;

class TestCliDBus : public QObject {
  Q_OBJECT

private slots:
  void testDBusAdaptorAndService() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SettingsManager settings;
    settings.setSaveDirectory(tempDir.path());
    LibraryManager library(&settings);
    CaptureEngine engine(&settings, &library);

    DBusService service(&engine);
    // Even if headless or dbus bus unavailable, service object should be safe
    QCOMPARE(service.isRegistered(), false);

    // Test DBus adaptor helper queries
    DBusAdaptor adaptor(&engine, &service);
    QCOMPARE(adaptor.IsCapturing(), false);
    QVERIFY(adaptor.GetLastCapturedFilePath().isEmpty());

    QSignalSpy libSpy(&adaptor, &DBusAdaptor::openLibraryRequested);
    adaptor.OpenLibrary();
    QCOMPARE(libSpy.count(), 1);

    QSignalSpy setSpy(&adaptor, &DBusAdaptor::openSettingsRequested);
    adaptor.OpenSettings();
    QCOMPARE(setSpy.count(), 1);
  }

  void testCaptureEngineColorExtraction() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SettingsManager settings;
    LibraryManager library(&settings);
    CaptureEngine engine(&settings, &library);

    // Initial with no desktop frame returns empty
    QVERIFY(engine.colorAt(10, 10).isEmpty());
    QVERIFY(engine.colorAtFormat(10, 10, "RGB").isEmpty());
    QVERIFY(engine.colorAtFormat(10, 10, "HSL").isEmpty());
    QVERIFY(!engine.copyColorAt(10, 10));
  }
};

QTEST_MAIN(TestCliDBus)
#include "test_cli_dbus.moc"
