#pragma once

#include "core/LibraryManager.hpp"
#include "core/SettingsManager.hpp"
#include "core/library/ThumbnailCache.hpp"
#include <QDate>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QTemporaryDir>
#include <QTest>

namespace ro_screenshot {

class TestGalleryBackend : public QObject {
  Q_OBJECT

public:
  static void runAllTests() {
    testScanGenerationsAndCancellation();
    testCorruptAndUnreadableFiles();
    testSubdirectoryDiscovery();
    testThumbnailCacheInvalidationAndEviction();
    testDateFilteringSemantics();
  }

  static void testScanGenerationsAndCancellation() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SettingsManager settings;
    settings.setSaveDirectory(tempDir.path());

    // Create 5 test images
    for (int i = 0; i < 5; ++i) {
      QImage img(400, 300, QImage::Format_RGB32);
      img.fill(Qt::blue);
      img.save(tempDir.path() + QString("/image_%1.png").arg(i), "PNG");
    }

    LibraryManager library(&settings);
    // Rapidly trigger multiple refreshes to test cancellation/supersession
    library.refresh();
    library.refresh();
    library.refresh();

    QVERIFY(library.waitForScan());
    QCOMPARE(library.count(), 5);
    QCOMPARE(library.scanState(), QString("idle"));
  }

  static void testCorruptAndUnreadableFiles() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SettingsManager settings;
    settings.setSaveDirectory(tempDir.path());

    // 1 valid image
    QImage validImg(200, 200, QImage::Format_RGB32);
    validImg.fill(Qt::green);
    QVERIFY(validImg.save(tempDir.path() + "/valid.png", "PNG"));

    // 1 corrupt file disguised as png
    QFile corruptFile(tempDir.path() + "/corrupt.png");
    QVERIFY(corruptFile.open(QIODevice::WriteOnly));
    corruptFile.write("This is not a valid PNG binary file data");
    corruptFile.close();

    LibraryManager library(&settings);
    library.refresh();
    QVERIFY(library.waitForScan());

    // Scan should handle corrupt file without crashing
    QCOMPARE(library.count(), 2);
  }

  static void testSubdirectoryDiscovery() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SettingsManager settings;
    settings.setSaveDirectory(tempDir.path());

    // Create year-month subdirectories
    QDir(tempDir.path()).mkpath("2026-08");
    QDir(tempDir.path()).mkpath("2026-09");

    QImage img1(300, 200, QImage::Format_RGB32);
    img1.fill(Qt::yellow);
    img1.save(tempDir.path() + "/2026-08/shot1.png", "PNG");

    QImage img2(300, 200, QImage::Format_RGB32);
    img2.fill(Qt::cyan);
    img2.save(tempDir.path() + "/2026-09/shot2.png", "PNG");

    LibraryManager library(&settings);
    library.refresh();
    QVERIFY(library.waitForScan());

    QCOMPARE(library.count(), 2);
  }

  static void testThumbnailCacheInvalidationAndEviction() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QTemporaryDir cacheDir;
    QVERIFY(cacheDir.isValid());

    ThumbnailCache cache(cacheDir.path());
    cache.setMaxCacheSizeBytes(1024 * 1024); // 1 MB limit
    cache.setMaxCacheItemCount(3);

    // Create 5 images
    QStringList paths;
    for (int i = 0; i < 5; ++i) {
      QString p = tempDir.path() + QString("/thumb_test_%1.png").arg(i);
      QImage img(400, 400, QImage::Format_RGB32);
      img.fill(QColor(i * 40, 50, 100));
      img.save(p, "PNG");
      paths.append(p);
      cache.getThumbnailPath(p);
    }

    // Cache should be bounded by item count
    QVERIFY(cache.currentCacheItemCount() <= 3);

    // Test invalidation
    QString target = paths[0];
    QFileInfo beforeInfo(target);
    qint64 beforeSize = beforeInfo.size();
    QDateTime beforeTime = beforeInfo.lastModified();
    QString thumb1 = cache.getThumbnailPath(target, beforeSize, beforeTime);
    QVERIFY(!thumb1.isEmpty());

    // Modify target file to change size and content
    QTest::qWait(50);
    QImage modifiedImg(800, 800, QImage::Format_RGB32);
    modifiedImg.fill(Qt::magenta);
    QVERIFY(modifiedImg.save(target, "PNG"));

    QFileInfo afterInfo(target);
    afterInfo.refresh();
    qint64 afterSize = afterInfo.size();
    QDateTime afterTime = afterInfo.lastModified();

    // New thumbnail key should be generated for changed size/mtime
    QString keyBefore = cache.generateKey(target, beforeSize, beforeTime);
    QString keyAfter = cache.generateKey(target, afterSize, afterTime);
    QVERIFY(keyBefore != keyAfter);

    // Test clearCache
    QVERIFY(cache.clearCache());
    QCOMPARE(cache.currentCacheItemCount(), 0);
  }

  static void testDateFilteringSemantics() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SettingsManager settings;
    settings.setSaveDirectory(tempDir.path());

    // Create an image
    QImage img(100, 100, QImage::Format_RGB32);
    img.fill(Qt::white);
    img.save(tempDir.path() + "/today_shot.png", "PNG");

    LibraryManager library(&settings);
    library.refresh();
    QVERIFY(library.waitForScan());

    // Filter: Today (1)
    library.setDateFilter(1);
    QCOMPARE(library.count(), 1);

    // Filter: Yesterday (2) -> should be 0 since it was created today
    library.setDateFilter(2);
    QCOMPARE(library.count(), 0);

    // Filter: This Week (3) -> should be 1
    library.setDateFilter(3);
    QCOMPARE(library.count(), 1);

    // Filter: Last 7 Days (4) -> should be 1
    library.setDateFilter(4);
    QCOMPARE(library.count(), 1);

    // Filter: This Month (5) -> should be 1
    library.setDateFilter(5);
    QCOMPARE(library.count(), 1);

    // Filter: All (0)
    library.setDateFilter(0);
    QCOMPARE(library.count(), 1);
  }
};

} // namespace ro_screenshot
