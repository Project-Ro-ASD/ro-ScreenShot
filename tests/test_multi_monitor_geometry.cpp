#include "core/platform/ScreenGeometryManager.hpp"
#include <QSignalSpy>
#include <QTest>

using namespace ro_screenshot;

class TestMultiMonitorGeometry : public QObject {
  Q_OBJECT

private slots:
  void testCoordinateMappingDpiScales() {
    ScreenGeometryManager geom;

    // 1.0x Scale: 1920x1080 overlay on 1920x1080 image
    const QSize overlay1080(1920, 1080);
    const QSize image1080(1920, 1080);
    const QRect selection(100, 150, 400, 300);

    const QRect mapped1 =
        geom.mapOverlayToImage(selection, overlay1080, image1080);
    QCOMPARE(mapped1, selection);

    const QRect back1 = geom.mapImageToOverlay(mapped1, image1080, overlay1080);
    QCOMPARE(back1, selection);

    // 1.25x Scale (125% DPI): 1536x864 logical overlay, 1920x1080 physical
    // image
    const QSize overlay125(1536, 864);
    const QSize image125(1920, 1080);
    const QRect sel125(100, 100, 200, 200);

    const QRect mapped125 =
        geom.mapOverlayToImage(sel125, overlay125, image125);
    QCOMPARE(mapped125.x(), 125);
    QCOMPARE(mapped125.y(), 125);
    QCOMPARE(mapped125.width(), 250);
    QCOMPARE(mapped125.height(), 250);

    // 1.5x Scale (150% DPI): 1280x720 logical overlay, 1920x1080 physical image
    const QSize overlay150(1280, 720);
    const QSize image150(1920, 1080);
    const QRect sel150(100, 100, 200, 200);

    const QRect mapped150 =
        geom.mapOverlayToImage(sel150, overlay150, image150);
    QCOMPARE(mapped150.x(), 150);
    QCOMPARE(mapped150.y(), 150);
    QCOMPARE(mapped150.width(), 300);
    QCOMPARE(mapped150.height(), 300);

    // 2.0x Scale (200% DPI HiDPI / Retina): 1920x1080 logical overlay,
    // 3840x2160 physical 4K image
    const QSize overlay200(1920, 1080);
    const QSize image4K(3840, 2160);
    const QRect sel200(100, 200, 300, 400);

    const QRect mapped4K = geom.mapOverlayToImage(sel200, overlay200, image4K);
    QCOMPARE(mapped4K.x(), 200);
    QCOMPARE(mapped4K.y(), 400);
    QCOMPARE(mapped4K.width(), 600);
    QCOMPARE(mapped4K.height(), 800);
  }

  void testNegativeCoordinateMapping() {
    ScreenGeometryManager geom;

    // Simulate virtual desktop with negative coordinates
    const QRect bounds(-1920, -500, 4480, 1940);
    // Canvas mapping: Logical screen at (-1920, 0) should map to virtual canvas
    // (0, 500)
    const QPoint screenPos(-1920, 0);
    const QPoint canvasPos(screenPos.x() - bounds.x(),
                           screenPos.y() - bounds.y());

    QCOMPARE(canvasPos.x(), 0);
    QCOMPARE(canvasPos.y(), 500);

    // Reverse canvas mapping
    const QPoint backScreen(canvasPos.x() + bounds.x(),
                            canvasPos.y() + bounds.y());
    QCOMPARE(backScreen, screenPos);
  }

  void testRegionValidationOnLayout() {
    ScreenGeometryManager geom;
    const QSize currentDesktopSize = geom.totalVirtualSize();

    // Valid region within desktop
    const QRect validRegion(10, 10, 200, 200);
    QVERIFY(geom.isRegionValidInCurrentLayout(validRegion, currentDesktopSize));

    // Region from different resolution / topology change
    const QSize oldDesktopSize(currentDesktopSize.width() + 500,
                               currentDesktopSize.height());
    QVERIFY(!geom.isRegionValidInCurrentLayout(validRegion, oldDesktopSize));

    // Out of bounds region
    const QRect outOfBounds(currentDesktopSize.width() + 100, 0, 200, 200);
    QVERIFY(
        !geom.isRegionValidInCurrentLayout(outOfBounds, currentDesktopSize));

    // Sanitization of out-of-bounds region
    const QRect sanitized =
        geom.sanitizeRegion(QRect(10, 10, currentDesktopSize.width() + 500,
                                  currentDesktopSize.height() + 500),
                            currentDesktopSize);
    QCOMPARE(sanitized.width(), currentDesktopSize.width() - 10);
    QCOMPARE(sanitized.height(), currentDesktopSize.height() - 10);
  }
};

QTEST_MAIN(TestMultiMonitorGeometry)
#include "test_multi_monitor_geometry.moc"
