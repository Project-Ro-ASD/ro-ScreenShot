#include "core/CaptureEngine.hpp"
#include "core/LibraryManager.hpp"
#include "core/SettingsManager.hpp"
#include "core/platform/SafeImageWriter.hpp"
#include <QClipboard>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace ro_screenshot;

class TestSafeImageIo : public QObject {
  Q_OBJECT

private slots:
  void testAtomicWriteSuccess() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SafeImageWriter writer;
    QImage image(100, 100, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::red);

    const QString destPath = tempDir.path() + "/atomic_test.png";
    const SaveResult res =
        writer.writeImageAtomically(image, destPath, "PNG", -1);

    QVERIFY(res.success);
    QCOMPARE(res.errorCode, CaptureErrorCode::None);
    QVERIFY(QFile::exists(destPath));
    QVERIFY(res.bytesWritten > 0);

    const QImage loaded(destPath);
    QVERIFY(!loaded.isNull());
    QCOMPARE(loaded.size(), QSize(100, 100));
  }

  void testInvalidFormatHandling() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SafeImageWriter writer;
    QImage image(50, 50, QImage::Format_ARGB32);
    image.fill(Qt::blue);

    const QString destPath = tempDir.path() + "/bad_format.xyz";
    const SaveResult res =
        writer.writeImageAtomically(image, destPath, "XYZ_NON_EXISTENT", -1);

    QVERIFY(!res.success);
    QCOMPARE(res.errorCode, CaptureErrorCode::EncoderFailed);
    QVERIFY(!QFile::exists(destPath));
  }

  void testNullImageHandling() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SafeImageWriter writer;
    const QImage nullImage;

    const QString destPath = tempDir.path() + "/null_test.png";
    const SaveResult res =
        writer.writeImageAtomically(nullImage, destPath, "PNG", -1);

    QVERIFY(!res.success);
    QCOMPARE(res.errorCode, CaptureErrorCode::InvalidImage);
    QVERIFY(!QFile::exists(destPath));
  }

  void testNonWritableDirectoryError() {
    SafeImageWriter writer;
    QImage image(50, 50, QImage::Format_ARGB32);
    image.fill(Qt::green);

    // Root directory or non-existent protected path
    const QString destPath = "/root/ro_screenshot_forbidden/image.png";
    const SaveResult res =
        writer.writeImageAtomically(image, destPath, "PNG", -1);

    QVERIFY(!res.success);
    QCOMPARE(res.errorCode, CaptureErrorCode::PermissionDenied);
  }

  void testUniqueFilePathCollisionAvoidance() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SafeImageWriter writer;
    const QString basePath = tempDir.path() + "/screenshot.png";

    // 1st file
    QFile f1(basePath);
    QVERIFY(f1.open(QIODevice::WriteOnly));
    f1.write("test1");
    f1.close();

    const QString unique1 = writer.resolveUniqueFilePath(basePath);
    QCOMPARE(unique1, tempDir.path() + "/screenshot_1.png");

    // 2nd file
    QFile f2(unique1);
    QVERIFY(f2.open(QIODevice::WriteOnly));
    f2.write("test2");
    f2.close();

    const QString unique2 = writer.resolveUniqueFilePath(basePath);
    QCOMPARE(unique2, tempDir.path() + "/screenshot_2.png");
  }

  void testTempFileLifecycleAndCleanup() {
    SafeImageWriter writer;
    const QString tempPath1 = writer.createTempFilePath("test_frame1", ".png");
    const QString tempPath2 = writer.createTempFilePath("test_frame2", ".png");

    QFile f1(tempPath1);
    QVERIFY(f1.open(QIODevice::WriteOnly));
    f1.write("frame1");
    f1.close();

    QFile f2(tempPath2);
    QVERIFY(f2.open(QIODevice::WriteOnly));
    f2.write("frame2");
    f2.close();

    QVERIFY(QFile::exists(tempPath1));
    QVERIFY(QFile::exists(tempPath2));

    writer.cleanupTempFiles();

    QVERIFY(!QFile::exists(tempPath1));
    QVERIFY(!QFile::exists(tempPath2));
  }

  void testSafeMemoryImageCaching() {
    SafeImageWriter writer;
    QVERIFY(writer.cachedImage().isNull());

    QImage img(80, 60, QImage::Format_RGB32);
    img.fill(Qt::yellow);
    writer.setCachedImage(img);

    const QImage retrieved = writer.cachedImage();
    QVERIFY(!retrieved.isNull());
    QCOMPARE(retrieved.size(), QSize(80, 60));

    writer.clearCachedImage();
    QVERIFY(writer.cachedImage().isNull());
  }

  void testCaptureEngineWorkflows() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SettingsManager settings;
    settings.setSaveDirectory(tempDir.path());
    LibraryManager library(&settings);
    CaptureEngine engine(&settings, &library);

    // Test copyImageToClipboard
    const QString testImgPath = tempDir.path() + "/clip_test.png";
    QImage srcImg(40, 40, QImage::Format_ARGB32);
    srcImg.fill(Qt::magenta);
    QVERIFY(srcImg.save(testImgPath));

    QVERIFY(engine.copyImageToClipboard(testImgPath));
    const QImage clipImg = QGuiApplication::clipboard()->image();
    QVERIFY(!clipImg.isNull());
  }
};

QTEST_MAIN(TestSafeImageIo)
#include "test_safe_image_io.moc"
