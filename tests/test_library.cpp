#include "core/LibraryManager.hpp"
#include "core/SettingsManager.hpp"
#include <QDir>
#include <QImage>
#include <QTemporaryDir>
#include <QTest>

using namespace ro_screenshot;

class TestLibrary : public QObject {
  Q_OBJECT

private slots:
  void testScanAndFilter() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    SettingsManager settings;
    settings.setSaveDirectory(tempDir.path());

    // Create 2 test images
    QImage img1(800, 600, QImage::Format_RGB32);
    img1.fill(Qt::red);
    QVERIFY(img1.save(tempDir.path() + "/test_shot_alpha.png", "PNG"));

    QImage img2(1920, 1080, QImage::Format_RGB32);
    img2.fill(Qt::blue);
    QVERIFY(img2.save(tempDir.path() + "/test_shot_beta.png", "PNG"));

    LibraryManager library(&settings);
    library.refresh();

    QCOMPARE(library.count(), 2);

    // Search query filter test
    library.setSearchQuery("alpha");
    QCOMPARE(library.count(), 1);
    QCOMPARE(library.data(library.index(0, 0), LibraryManager::FileNameRole)
                 .toString(),
             QString("test_shot_alpha.png"));

    library.setSearchQuery("");
    QCOMPARE(library.count(), 2);

    // Test deletion
    QVERIFY(library.deleteItem(0));
    QCOMPARE(library.count(), 1);
  }
};

QTEST_MAIN(TestLibrary)
#include "test_library.moc"
