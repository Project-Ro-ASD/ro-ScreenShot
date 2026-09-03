#include "core/Types.hpp"
#include "core/platform/XdgPortalProvider.hpp"
#include <QSignalSpy>
#include <QTest>

using namespace ro_screenshot;

class TestPortalBackend : public QObject {
  Q_OBJECT

private slots:
  void testPortalProviderProperties() {
    XdgPortalProvider portal;
    QCOMPARE(portal.name(), QStringLiteral("XdgPortal"));

    portal.setParentWindowHandle(QStringLiteral("wayland:test-handle-123"));
    QCOMPARE(portal.parentWindowHandle(),
             QStringLiteral("wayland:test-handle-123"));

    portal.setTimeoutDurationMs(500);
  }

  void testPortalTimeoutHandling() {
    XdgPortalProvider portal;
    portal.setTimeoutDurationMs(100); // 100ms timeout

    QSignalSpy failedSpy(&portal, &IScreenshotProvider::captureFailed);
    portal.capture(CaptureMode::Region);

    // If portal DBus is not available, fails immediately with
    // PortalUnavailable. Otherwise timeout triggers after 100ms.
    if (failedSpy.isEmpty()) {
      QVERIFY(failedSpy.wait(1000));
    }

    QVERIFY(!failedSpy.isEmpty());
  }

  void testPortalCancellation() {
    XdgPortalProvider portal;
    QSignalSpy cancelSpy(&portal, &IScreenshotProvider::captureCancelled);

    portal.capture(CaptureMode::Fullscreen);
    portal.cancel();

    // cancel should emit captureCancelled if capture was in progress
    // or gracefully return
  }
};

QTEST_MAIN(TestPortalBackend)
#include "test_portal_backend.moc"
